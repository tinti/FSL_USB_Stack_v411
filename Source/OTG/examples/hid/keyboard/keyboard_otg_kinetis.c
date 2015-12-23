/**HEADER********************************************************************
 *
 * Copyright (c) 2010 Freescale Semiconductor;
 * All Rights Reserved
 *
 ***************************************************************************
 *
 * THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 **************************************************************************
 *
 * Comments:
 *
 *   This file contains the implementation of the CDC OTG demonstration app.
 *
 *END************************************************************************/
#include "user_config.h"
#include "types.h"

#include "usb.h"
#include"host_common.h"
#include "usb_otg_main.h"
#if !HIGH_SPEED_DEVICE
#include "usb_otg_max3353.h"
#else
#include "usb3300.h"
#endif

#include "hidef.h" /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "usb_devapi.h"
#include "usb_dci_kinetis.h"
#include "khci.h"
#include "usb_hid.h"        /* USB-HID class Header File */
#include "keyboard_dev.h"   /* Device Keyboard Application Header File */
#include "keyboard_host.h"
#include "usb_host_hub_sm.h"
#include "usb_bsp.h"
#include "sci.h"
#include "rtc.h"
#include "IIC_kinetis.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* Private variables ********************************************************/

/* Private functions prototypes *********************************************/
static void App_OtgCallback(_usb_otg_handle handle, OTG_EVENT event);
static void App_HandleUserInput(void);
static void App_PrintMenu(void);
static void App_ActiveStackUninit(void);

void IRQ_ISR(void);
void Kbi_ISR(void);
void USB_OTG_ISR(void);
void USBHS_OTG_ISR(void);

/* OTG initialization structure */
static const OTG_INIT_STRUCT otg_init=
{
		TRUE,   						/* Use external circuit */
#if !HIGH_SPEED_DEVICE
		_otg_max3353_enable_disable,
		_otg_max3353_get_status,
		_otg_max3353_get_interrupts,
		_otg_max3353_set_VBUS,
		_otg_max3353_set_pdowns,
#else
		NULL,					// #2
		USB3300_GetStatus,		// #3
		USB3300_GetInterrupts,	// #4
		USB3300_SetVBUS,		// #5
		USB3300_SetPdowns,		// #6
#endif
		App_Host_Init,			// #7
		App_PeripheralInit,		// #8
		App_Host_Shut_Down,		// #9
		App_PeripheralUninit,	// #10
		App_ActiveStackUninit	// #11
};

typedef enum
{
	dev_b = 0,
	dev_a
} dev_type_t;


_usb_otg_handle   otg_handle;
uint_32            host_stack_active = FALSE;  /* TRUE if the host stack is active */   
uint_32           dev_stack_active = FALSE;   /* TRUE if the peripheral stack is active */
dev_type_t        dev_type;           /* dev_type = 0 (device B); dev_type = 1 (device A) */
boolean           sess_vld;           /* TRUE if session is valid */
boolean           vbus_err;           /* VBUS overcurrent */

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : Main
 * Returned Value : none
 * Comments       :
 *
 *
 *END*--------------------------------------------------------------------*/
void main(void)
{
	USB_STATUS      status = USB_OK;
	/* Initialize the current platform. Call for the _bsp_platform_init which is specific to each processor family */
	_bsp_platform_init();

#ifdef MCU_MK70F12
	sci2_init();

	// clear terminal
	printf("\f");
#if HIGH_SPEED_DEVICE
	USB3300_Init();
#endif // HIGH_SPEED_DEVICE
#else // MCU_MK70F12
	sci1_init();
#endif
#if !HIGH_SPEED_DEVICE
	IIC_ModuleInit();
#endif

	TimerInit();
	//TimerQInitialize();

	DisableInterrupts;

#if !HIGH_SPEED_DEVICE
	/* Setup #INT(MAX3353) pin handler */
#if (defined MCU_MK40N512VMD100)||(defined MCU_MK53N512CMD100)
	NVICICPR2 = (1 << 24);                    /* Clear any pending interrupts */
	NVICISER2 = (1 << 24);                    /* Enable interrupts PORTB */
	GPIOB_PDDR &= ~((uint_32)1 << 7);		/* set input PTB 7*/

	/* Configure pin as GPIO PTB 7 for #INT pin of Max3353 */
	PORTB_PCR7=(0|PORT_PCR_MUX(1)|PORT_PCR_PE_MASK|PORT_PCR_PS_MASK|PORT_PCR_IRQC(0x0A));
#elif (defined MCU_MK20D7) || (defined MCU_MK40D7)
	NVICICPR2 = (1 << 23);                    /* Clear any pending interrupts */
	NVICISER2 = (1 << 23);                    /* Enable interrupts PORTA */
	GPIOA_PDDR &= ~((uint_32)1 << 17);		/* Set input PTA 17 */

	/* Configure pin as GPIO PTA 17 for #INT pin of Max3353 */
	PORTA_PCR17=(0|PORT_PCR_MUX(1)|PORT_PCR_PE_MASK|PORT_PCR_PS_MASK|PORT_PCR_IRQC(0x0A));   
#elif defined MCU_MK60N512VMD100
	NVICICPR2 = (1 << 23);                    /* Clear any pending interrupts */
	NVICISER2 = (1 << 23);                    /* Enable interrupts PORTA */
	GPIOA_PDDR &= ~((uint_32)1 << 26);		/* Set input PTA 26 */

	/* configure pin as GPIO PTA 26 for #INT pin of Max3353*/
	PORTA_PCR26=(0|PORT_PCR_MUX(1)|PORT_PCR_PE_MASK|PORT_PCR_PS_MASK|PORT_PCR_IRQC(0x0A));
#elif defined MCU_MK70F12
	NVICICPR2 = (1 << 24);						/* Clear any pending interrupts */
	NVICISER2 = (1 << 24);						/* Enable interrupts PORTB*/

	/* set IRQ_B pin */
	GPIOB_PDDR &= ~((uint_32)1 << 6);		/* set input PTB 6 */

	/* PTB6 */
	/* Enable the corresponding pin as GPIO */
	PORTB_PCR6 = PORT_PCR_MUX(1);

	/* Assure that the Interrupt Status Flag (ISF) is cleared before enabling interrupt */
	PORTB_PCR6 |= PORT_PCR_ISF_MASK;

	/* Configure the interrupt on falling edge */
	PORTB_PCR6 = (0|PORT_PCR_MUX(1)|PORT_PCR_PE_MASK|PORT_PCR_PS_MASK|PORT_PCR_IRQC(0x0B));
#elif defined(MCU_MK20D5)
#if defined (TWR_K20D5)
	/* VBUS detection  */
	NVICICPR1 = (1 << (42%32));    	/* Clear any pending interrupts */
	NVICISER1 = (1 << (42%32));      /* Enable interrupts PORTC */

	PORTC_PCR11=(0 | PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_IRQC(0x0B)); 

	/* ID detection  */
	PORTC_PCR10=(0 | PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_IRQC(0x0B));
#endif
#elif defined MCU_MK21D5
	NVICICPR1 = (1 << 27);                    /* Clear any pending interrupts */
	NVICISER1 = (1 << 27);                    /* Enable interrupts PORTA */
	GPIOA_PDDR &= ~((uint_32)1 << 16);		/* Set input PTA 16 */

	/* configure pin as GPIO PTA 16 for #INT pin of Max3353*/
	PORTA_PCR16=(0|PORT_PCR_MUX(1)|PORT_PCR_PE_MASK|PORT_PCR_PS_MASK|PORT_PCR_IRQC(0x0A));
#elif defined MCU_MKL25Z4
	
	NVIC_ICPR = (1 << 31);                    /* Clear any pending interrupts */
	NVIC_ISER = (1 << 31);                    /* Enable interrupts PORTD */
	GPIOD_PDDR &= ~((uint_32)1 << 1);		/* Set input PTD 1 */
	/* configure pin as GPIO PTD 1 for #INT pin of Max3353*/
	PORTD_PCR1=(0|PORT_PCR_MUX(1)|PORT_PCR_PE_MASK|PORT_PCR_PS_MASK|PORT_PCR_IRQC(0x0A));
	
#else
#error "#INT pin of MAX3353 not handled"
#endif
#endif	// HIGH_SPEED_DEVICE

	status = _usb_otg_init(0, (OTG_INIT_STRUCT*)&otg_init, &otg_handle);
	if(status == USB_OK)
	{
		status = _usb_otg_register_callback(otg_handle, App_OtgCallback);
	}

	printf("\n\rInitialization passed. Plug-in HID device to USB port");
	printf("\n\rPress P to print the menu:");

	EnableInterrupts;
	
	for(;;) 
	{
		_usb_otg_task();

		if(dev_stack_active)
		{	
			App_PeripheralTask();
		}
		if(host_stack_active)
		{
			App_Host_Task();
		}

		App_HandleUserInput();
		__RESET_WATCHDOG(); /* feeds the dog */
	} /* loop forever */
	/* please make sure that you never leave main */
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : App_ActiveStackUninit
 * Returned Value : none
 * Comments       :
 *
 *
 *END*--------------------------------------------------------------------*/
static void App_ActiveStackUninit(void)
{
	if(dev_stack_active)
	{
		App_PeripheralUninit();
	}
	if(host_stack_active)
	{
		App_Host_Shut_Down();
	}
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : App_OtgCallback
 * Returned Value : none
 * Comments       :
 *
 *
 *END*--------------------------------------------------------------------*/
void App_OtgCallback(_usb_otg_handle handle, OTG_EVENT event)
{
	(void)handle; /* not used */		

	if(event & OTG_B_IDLE)
	{
		printf("\n\r>B: OTG state change to B idle");
		dev_type = dev_b;               /* Device type: B */
		sess_vld = FALSE;           /* session not valid */
	}

	if(event & OTG_B_IDLE_SRP_READY)
	{
		printf("\n\r>B: OTG is ready to initialize SRP");
	}

	if(event & OTG_B_SRP_INIT)
	{
		printf("\n\r>B: OTG has initialized SRP");
	}	

	if(event & OTG_B_SRP_FAIL)
	{
		printf("\n\r>B: OTG SRP failed to get a response from the Host");
	}	

	if(event & OTG_B_PERIPHERAL)
	{
		printf("\n\r>B: OTG state change to B peripheral.");
		printf("\n\r>B: USB peripheral stack initialized.");

		if(sess_vld == FALSE)
		{
			sess_vld = TRUE;      /* session valid */
		}	
	}

	if(event & OTG_B_PERIPHERAL_LOAD_ERROR)	
	{
		printf("\n\r>B: OTG state change to B peripheral.");
		printf("\n\r>B: USB peripheral stack initialization failed.");
	}

	if(event & OTG_B_PERIPHERAL_HNP_READY)
	{
		printf("\n\r>B: OTG is ready to initialize HNP. Press SW1 to request the bus.");	
	}

	if(event & OTG_B_PERIPHERAL_HNP_START)
	{
		printf("\n\r>B: OTG has initialized the HNP to request the bus from Host");	
	}

	if(event & OTG_B_PERIPHERAL_HNP_FAIL)
	{
		printf("\n\r>B: HNP failed. OTG is back into peripheral state");
	}

	if(event & OTG_B_HOST)
	{ 	
		printf("\n\r>B: OTG is in the Host state");
		printf("\n\r>B: USB host stack initialized.");
	}

	if(event & OTG_B_HOST_LOAD_ERROR)
	{
		printf("\n\r>B: OTG is in the Host state");
		printf("\n\r>B: USB host stack initialization failed.");
	}

	if(event & OTG_B_A_HNP_REQ)
	{
		if(_usb_otg_bus_release(otg_handle) == USB_OK)
		{
			printf("\n\rBus release");
		}
		else
		{
			printf("\n\rError releasing the bus");
		}
	}

	if(event & OTG_A_WAIT_BCON_TMOUT)
	{
		printf("\n\r>A: OTG_A_WAIT_BCON_TMOUT");
		_usb_otg_set_a_bus_req(otg_handle , FALSE);

	}

	if(event & OTG_A_BIDL_ADIS_TMOUT)
	{
		printf("\n\r>A: OTG_A_BIDL_ADIS_TMOUT");
		_usb_otg_set_a_bus_req(otg_handle, TRUE);	
	}

	if(event & OTG_A_AIDL_BDIS_TMOUT)
	{
		printf("\n\r>A: OTG_A_AIDL_BDIS_TMOUT");
	}


	if(event & OTG_A_ID_TRUE)
	{
		printf("\n\r>A: ID = TRUE ");
	}


	if(event & OTG_A_WAIT_VRISE_TMOUT)
	{
		printf("\n\r>A: VBUS rise failed");
	}

	if(event & OTG_A_B_HNP_REQ)
	{
		printf("\n\r>A: OTG_A_B_HNP_REQ");
		_usb_otg_set_a_bus_req( handle , FALSE);

	}

	if(event & OTG_A_IDLE)
	{
		printf("\n\r>A: OTG state change to A_IDLE");
		host_stack_active  = FALSE;
		dev_stack_active = FALSE;

		dev_type = dev_a;             /* Device type: A */
		sess_vld = FALSE;
	}

	if(event & OTG_A_WAIT_VRISE)
	{
		printf("\n\r>A: OTG state change to A_WAIT_VRISE");
	}

	if(event & OTG_A_WAIT_BCON)
	{
		printf("\n\r>A: OTG state change to A_WAIT_BCON");

		sess_vld = TRUE;
	}

	if(event & OTG_A_HOST)
	{
		printf("\n\r>A: OTG state change to OTG_A_HOST");
		printf("\n\r>A: USB host stack initialized.");

	}
	if(event & OTG_A_HOST_LOAD_ERROR)
	{
		printf("\n\r>A: OTG state change to OTG_A_HOST");
		printf("\n\r>A: USB host stack initialization failed.");
	}

	if(event & OTG_A_SUSPEND)
	{
		printf("\n\r>A: OTG state change to A_SUSPEND");
	}	

	if(event & OTG_A_PERIPHERAL)
	{ 	
		printf("\n\r>A: OTG state change to A_PERIPHERAL	");
		printf("\n\r>A: USB peripheral stack initialized.");
	}	

	if(event & OTG_A_PERIPHERAL_LOAD_ERROR)
	{
		printf("\n\r>A: USB peripheral stack initialization failed.");
		printf("\n\r>A: OTG state change to A_PERIPHERAL	");
	}	

	if(event & OTG_A_WAIT_VFALL)
	{
		printf("\n\r>A: OTG state change to OTG_A_WAIT_VFALL");

		if(vbus_err == TRUE)
		{
			vbus_err = FALSE;
		}
	}

	if(event & OTG_A_VBUS_ERR)
	{
		printf("\n\r>A: VBUS falls below VBUS_Valid treshold");
		printf("\n\r>A: OTG state change to A_VBUS_ERR");

		vbus_err = TRUE;
	}		
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : App_HandleUserInput
 * Returned Value : none
 * Comments       :
 *
 *
 *END*--------------------------------------------------------------------*/
static void App_HandleUserInput(void)
{
	uint_8 character = TERMIO_GetCharNB ();

	switch(character)
	{
	case '1': if(_usb_otg_hnp_enable(0, TRUE) == USB_OK)
	{
		printf("\n\rHNP enabled");
	}
	else
	{
		printf("\n\rError enabling HNP");
	}
	break;

	case '2': if(_usb_otg_hnp_enable(0, FALSE) == USB_OK)
	{
		printf("\n\rHNP disabled");
	}
	else
	{
		printf("\n\rError disabling HNP");
	}
	break;

	case '3': if(_usb_otg_bus_request(otg_handle) == USB_OK)
	{
		printf("\n\rBus request");
	}
	else
	{
		printf("\n\rError requesting the bus");
	}
	break;

	case '4': if(_usb_otg_bus_release(otg_handle) == USB_OK)
	{
		printf("\n\rBus release");
	}
	else
	{
		printf("\n\rError releasing the bus");
	}
	break;


	case '5': if(_usb_otg_session_request(otg_handle) == USB_OK)
	{
		printf("\n\rSRP request");
	}
	else
	{
		printf("\n\rError Requesting SRP");
	}
	break;

	case '6': _usb_otg_set_a_bus_req(otg_handle , TRUE);

	printf("\n\rA BUS REQ");

	break;
	case '7': _usb_otg_set_a_bus_req(otg_handle , FALSE);

	printf("\n\rA BUS RELEASE");
	break;

	case '8': _usb_otg_set_a_bus_drop(otg_handle , TRUE);

	printf("\n\rA BUS DROP TRUE");
	break;
	case '9': _usb_otg_set_a_bus_drop(otg_handle , FALSE);

	printf("\n\rA BUS DROP FALSE");
	break;
	case 'a':
	case 'A':
		_usb_otg_set_a_clear_err(otg_handle);
		break;

	case 'p':
	case 'P': App_PrintMenu();
	break;
	default: break;
	}
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : App_PrintMenu
 * Returned Value : none
 * Comments       :
 *
 *
 *END*--------------------------------------------------------------------*/
static void App_PrintMenu(void)
{
	printf("\n\r  OTG App User Input Menu");
	if(dev_type == dev_a)
	{
		if(vbus_err == FALSE)
		{
			if(sess_vld == TRUE)
			{
				if(dev_stack_active == TRUE)
				{
					printf("\n\r      6. A Bus request (HNP start)");
				}
				if(host_stack_active == TRUE)
				{
					printf("\n\r      7. A Bus release");
				}
				printf("\n\r      8. A Set Bus Drop TRUE (session end)");
			}
			else  /* session not valid */
			{
				printf("\n\r      9. A Set Bus Drop FALSE");
			}
		}
		else    /* no VBUS error */
		{
			printf("\n\r      8. A Set Bus Drop TRUE (session end)");
			printf("\n\r      a. A Clear Error");
		}
	}
	else if(dev_type == dev_b)
	{
		if(sess_vld == TRUE)
		{
			printf("\n\r      1. B Force HNP enable ON");
			printf("\n\r      2. B Force HNP enable OFF");
			if(dev_stack_active == TRUE)
			{
				printf("\n\r      3. B Bus request (HNP start)");
			}

			if(host_stack_active == TRUE)
			{
				printf("\n\r      4. B Bus release");
			}
		}
		else
		{
			printf("\n\r      5. B Session request (SRP start)");
		}
	}
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : IRQ_ISR_PTA
 * Returned Value : none
 * Comments       : interrupt function PTA
 *
 *
 *END*--------------------------------------------------------------------*/
#ifdef USE_IRQ_PTA
void IRQ_ISR_PTA(void)
{
	#if (defined MCU_MK20D7) || (defined MCU_MK40D7)
		if(PORTA_ISFR & (1<<17))
		{
			/* #INT pin interrupt */
			_usb_otg_ext_isr(0);
			PORTA_ISFR = (1 << 17);     /* Clear KBI interrupt PTA 17 */
		}
	#elif defined MCU_MK21D5	
		if(PORTA_ISFR & (1<<16))	
		{
			/* #INT pin interrupt */	
			_usb_otg_ext_isr(0);	
			PORTA_ISFR = (1 << 16);     /* Clear KBI interrupt PTA 16 */
		}
	#elif defined MCU_MKL25Z4	
		if(PORTA_ISFR & (1<<4))	
		{
			if(dev_stack_active)
			{
				App_PeripheralKbiServiceRoutine();		    
			}
			PORTA_ISFR = (1 << 4);     /* clear KBI interrupt PTA 4 */
		}
	#else
		if(PORTA_ISFR & (1<<26))
		{
			/* #INT pin interrupt */
			_usb_otg_ext_isr(0);
			PORTA_ISFR = (1 << 26);     /* clear KBI interrupt PTA 26 */
		}
		
		if(PORTA_ISFR & (1<<19)) 
		{		
			if(dev_stack_active)
			{
				App_PeripheralKbiServiceRoutine();		    
			}
			PORTA_ISFR = (1 << 19);     /* clear KBI interrupt PTA 19 */
		}
	#endif
}
#endif	// USE_IRQ_PTA

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : IRQ_ISR_PTB
 * Returned Value : none
 * Comments       : interrupt function PTB
 *
 *
 *END*--------------------------------------------------------------------*/
#ifdef USE_IRQ_PTB
void IRQ_ISR_PTB(void)
{
#ifdef MCU_MK70F12
	if(PORTB_ISFR & (1<<6))
	{
		_usb_otg_ext_isr(0);
		PORTB_ISFR = (1 << 6);     /* clear KBI interrupt */
	}
#else
	if(PORTB_ISFR & (1<<7))
	{
		_usb_otg_ext_isr(0);
		PORTB_ISFR = (1 << 7);     /* clear KBI interrupt */
	}
#endif
}
#endif // USE_IRQ_PTB

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : IRQ_ISR_PTC
 * Returned Value : none
 * Comments       : interrupt function PTC
 *
 *
 *END*--------------------------------------------------------------------*/
#ifdef USE_IRQ_PTC
#ifdef __ICCARM__
void PORTC_IRQHandler(void)
#else
void IRQ_ISR_PTC(void)
#endif
{
	if(dev_stack_active)
	{
	#if (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
		if(PORTC_ISFR & (1<<1))
        #elif defined MCU_MK21D5	 
                if(PORTC_ISFR & (1<<6))
	#else	
		if(PORTC_ISFR & (1<<5))
	#endif
		{			
			App_PeripheralKbiServiceRoutine();
		#if (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)						
			PORTC_ISFR = (1 << 1);	/* Clear KBI interrupt */
                #elif defined MCU_MK21D5 
                        PORTC_ISFR = (1 << 6);	/* Clear KBI interrupt */
		#else						
			PORTC_ISFR = (1 << 5);    /* Clear KBI interrupt */
		#endif
		}
	}

	if(dev_stack_active)
	{
	#if (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
		if(PORTC_ISFR & (1<<2))
        #elif defined MCU_MK21D5
                if(PORTC_ISFR & (1<<7))
	#else
		if(PORTC_ISFR & (1<<13))
	#endif
		{
			App_PeripheralKbiServiceRoutine();
		#if (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
			PORTC_ISFR = (1 << 2);
                #elif defined MCU_MK21D5 
                        PORTC_ISFR = (1 << 7);
		#else
			PORTC_ISFR = (1 << 13);    /*  clear KBI interrupt */
		#endif
		}
	}

#if (defined MCU_MK20D5) && (defined TWR_K20D5)
	{
#define PTISR_EVENT (0x80)
#define ID_GND_EVENT            0x20
#define ID_FLOAT_EVENT          0x40
#define VBUS_LOW_EVENT          0x10
#define VBUS_HIGH_EVENT         0x01

		uint32_t 	u32Value;
		static uint8_t 	gu8ISR_Flags = 0;

		u32Value = PORTC_ISFR;
		PORTC_ISFR = u32Value;

		gu8ISR_Flags |= PTISR_EVENT;

		if (u32Value & (uint32_t)(1<<10))
		{
			if (GPIOC_PDIR & (uint32_t)(1<<10))
			{	          
				gu8ISR_Flags |= ID_FLOAT_EVENT;
				printf("\n\rID = TRUE");
			}
			else
			{    
				gu8ISR_Flags |= ID_GND_EVENT;
				printf("\n\rID = FALSE");
			}
		}

		if (u32Value & (uint32_t)(1<<11))
		{
			if (GPIOC_PDIR & (uint32_t)(1<<11))
			{	        
				gu8ISR_Flags |= VBUS_HIGH_EVENT;
				printf("\n\rVBUS is Up"); 
			}
			else
			{	        
				gu8ISR_Flags |= VBUS_LOW_EVENT; 
				printf("\n\rVBUS is Down"); 
			}
		}
	}
#endif
}
#endif	// USE_IRQ_PTC

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : IRQ_ISR_PTD
 * Returned Value : none
 * Comments       : interrupt function PTD
 *
 *
 *END*--------------------------------------------------------------------*/
#ifdef USE_IRQ_PTD
void IRQ_ISR_PTD(void)
{	
#if defined MCU_MKL25Z4	
		if(PORTD_ISFR & (1<<1))	
		{
			/* #INT pin interrupt */	
			_usb_otg_ext_isr(0);	
			PORTD_ISFR = (1 << 1);     /* Clear KBI interrupt PTA 16 *///namnt10
		}
#else
	if(PORTD_ISFR & (1<<0)) 
	{		  
		if(dev_stack_active)  
		{
			App_PeripheralKbiServiceRoutine();	        
		}
		PORTD_ISFR = (1 << 0);    /*  clear KBI interrupt PTD 0 */
	}
#endif
}
#endif	// USE_IRQ_PTD

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : IRQ_ISR_PTE
 * Returned Value : none
 * Comments       : interrupt function PTE
 *
 *
 *END*--------------------------------------------------------------------*/
#ifdef USE_IRQ_PTE
void IRQ_ISR_PTE(void)
{	
	if(PORTE_ISFR & (1<<26)) 
	{
		if(dev_stack_active)  
		{
			App_PeripheralKbiServiceRoutine();	        
		}
		PORTE_ISFR = (1 << 26);    /*  clear KBI interrupt PTE 26 */
	}
}
#endif	// USE_IRQ_PTE


/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : USB_OTG_ISR
 * Returned Value : none
 * Comments       :
 *
 *
 *END*--------------------------------------------------------------------*/
#if !HIGH_SPEED_DEVICE
extern void USB_ISR(void);
void USB_OTG_ISR(void)
{
	_usb_otg_isr(0);

	if(dev_stack_active)
	{
		USB_ISR();
	}

	if(host_stack_active)
	{
		USB_ISR_HOST();
		USB0_USBTRC0 = 0;
	}
	if(!dev_stack_active)
	{
		uint_8  int_stat;
		/* Clear pending interrupt */
		int_stat = USB0_ISTAT;
		USB0_ISTAT = int_stat;
	}
}
#endif

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : USBHS_OTG_ISR
 * Returned Value : none
 * Comments       :
 *
 *
 *END*--------------------------------------------------------------------*/
#if HIGH_SPEED_DEVICE
void USBHS_OTG_ISR(void)
{
	_usb_otg_isr(0);
	
	if(!(USBHS_OTGSC & USBHS_OTGSC_W1C))
		return;
	if(USBHS_OTGSC & USBHS_OTGSC_MSS_MASK){
		USBHS_OTGSC |= USBHS_OTGSC_MSS_MASK;
	}
	if(USBHS_OTGSC & USBHS_OTGSC_BSEIS_MASK){
		USBHS_OTGSC |= USBHS_OTGSC_BSEIS_MASK;
		printf("B session End Interrupt Status\n");
	}
	if(USBHS_OTGSC & USBHS_OTGSC_BSVIS_MASK){
		USBHS_OTGSC |= USBHS_OTGSC_BSVIS_MASK;
		printf("B Session Valid Interrupt Status\n");
	}
	if(USBHS_OTGSC & USBHS_OTGSC_ASVIS_MASK){
		USBHS_OTGSC |= USBHS_OTGSC_ASVIS_MASK;
		printf("A Session Valid Interrupt Status\n");
	}
	if(USBHS_OTGSC & USBHS_OTGSC_AVVIS_MASK){
		USBHS_OTGSC |= USBHS_OTGSC_AVVIS_MASK;
		printf("A VBUS Valid Interrupt Status\n");
	}
	if(USBHS_OTGSC & USBHS_OTGSC_IDIS_MASK){
		USBHS_OTGSC |= USBHS_OTGSC_IDIS_MASK;
		printf("USB ID Interrupt Status\n");
	}

#if !HIGH_SPEED_DEVICE
	_usb_otg_isr(0);
#endif

	if(dev_stack_active)
	{
		USBHS_ISR();
	}

	if(host_stack_active)
	{
		USBHS_ISR_HOST();
	}
	if(!dev_stack_active)
	{
		/* Clear pending interrupt */
		USBHS_OTGSC |= USBHS_OTGSC_W1C;
	}
}
#endif

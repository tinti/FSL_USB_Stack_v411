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
#include "types.h"

#include "usb.h"
#include"host_common.h"
#include "usb_otg_main.h"
#include "usb_otg_max3353.h"

#include "hidef.h" /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "usb_devapi.h"
#if (defined MCU_mcf51jf128)
#include "usb_dci_cfv1_plus.h"
#endif
#include "khci.h"
#include "usb_hid.h"        /* USB-HID class Header File */
#include "keyboard_dev.h"   /* Device Keyboard Application Header File */
#include "keyboard_host.h"
#include "usb_host_hub_sm.h"
#include "usb_bsp.h"
#include "sci.h"
#include "rtc.h"
#include "RealTimerCounter.h"
#include "IIC_cfv1_plus.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Private variables ********************************************************/

/* Private functions prototypes *********************************************/
static void App_OtgCallback(_usb_otg_handle handle, OTG_EVENT event);
static void App_HandleUserInput(void);
static void App_PrintMenu(void);
static void App_ActiveStackUninit(void);

void interrupt 64 IRQ_ISR(void);
void Kbi_ISR(void);
void interrupt 73 USB_OTG_ISR(void);

/* OTG initialization structure */
static const OTG_INIT_STRUCT otg_init= 
{
 TRUE,   						/* Use external circuit */
 _otg_max3353_enable_disable,
 _otg_max3353_get_status,
 _otg_max3353_get_interrupts,
 _otg_max3353_set_VBUS,
 _otg_max3353_set_pdowns,
 App_Host_Init,
 App_PeripheralInit,
 App_Host_Shut_Down,
 App_PeripheralUninit,
 App_ActiveStackUninit
};

typedef enum
{
  dev_b = 0,
  dev_a 
} dev_type_t;


_usb_otg_handle   otg_handle;

uint_8            host_stack_active = FALSE;  /* TRUE if the host stack is active */
uint_8            dev_stack_active = FALSE;   /* TRUE if the peripheral stack is active */
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
   sci1_init();
   sci2_init();
   IIC_ModuleInit();
   TimerInit();
   TimerQInitialize(0);
   
   /* Jumper setting */
   printf("\n*************************************************************");
   printf("\n*           MAX3353 & MIC2026 Use for CFV Plus               *");
   printf("\n*************************************************************");
   printf("\n\nTWR-Serial board Jumpers");
   printf("\nJ16 -> 1-2   5-6");
   printf("\nJ11 -> 1-2");
   printf("\n*************************************************************\n");
   DisableInterrupts;   

   /* configure pin as GPIO PTB 7 for #INT pin of Max3353 */
#if (defined MCU_mcf51jf128)
   /* Enable IRQ clock */
   SIM_SCGC4 |= SIM_SCGC4_IRQ_MASK;		/* set input PTB 7*/
   /* Configure PTB0 is IRQ */
   MXC_PTBPF4 &=~MXC_PTBPF4_B0(0xF);
   MXC_PTBPF4 |=MXC_PTBPF4_B0(0x5);
   /* Enable interrupt */
   IRQ_SC = IRQ_SC_IRQIE_MASK | IRQ_SC_IRQPE_MASK;
#endif
 
   status = _usb_otg_init(0, (OTG_INIT_STRUCT*)&otg_init, &otg_handle);
   if(status == USB_OK)
   {   
	   status = _usb_otg_register_callback(otg_handle, App_OtgCallback);
   }	   
 
   EnableInterrupts;

   printf("\n\rInitialization passed. Plug-in HID device to USB port");
   printf("\n\rPress P to print the menu:");
   
   for(;;) {
      
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
             printf("\n\rError rerequesting SRP");
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
* Function Name  : IRQ_ISR_PTB
* Returned Value : none
* Comments       : interrupt function PTB
*     
*
*END*--------------------------------------------------------------------*/
void interrupt 64 IRQ_ISR(void)
{   
     _usb_otg_ext_isr(0);
     IRQ_SC |= IRQ_SC_IRQF_MASK | IRQ_SC_IRQACK_MASK;     /* clear KBI interrupt */ 
}



/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : USB_OTG_ISR
* Returned Value : none
* Comments       :
*     
*
*END*--------------------------------------------------------------------*/
void interrupt 73 USB_OTG_ISR(void)
{   
	  _usb_otg_isr(0);
		  
	  if(dev_stack_active)
	  {    
		USB_ISR();
	  }
	  
	  if(host_stack_active)
	  {
		USB_ISR_HOST(); 

	  }
	  if(!dev_stack_active){

	  } 
}

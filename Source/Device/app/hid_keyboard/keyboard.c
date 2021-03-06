/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 **************************************************************************//*!
 *
 * @file keyboard.c
 *
 * @author
 *
 * @version
 *
 * @date May-28-2009
 *
 * @brief  The file emulates a keyboard
 *         4 buttons are used on the demo board for the emulation
 *         PTG0--------- Page Up (For JS16 PTG0 is inactive)
 *         PTG1--------- Page Down
 *         PTG2--------- Space Bar
 *         PTG3--------- Print Screen
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "types.h"      /* User Defined Data-types */
#include "hidef.h"      /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "usb_hid.h"    /* USB-HID class Header File */
#include "keyboard.h"   /* Keyboard Application Header File */

/* skip the inclusion in dependency statge */
#ifndef __NO_SETJMP
   #include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>		

#if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
#include "exceptions.h"
#endif

/*****************************************************************************
 * Constant and Macro's - None
 *****************************************************************************/
/* Macros to define which mode of power savings you would like in suspend.
 * Only select one!
 */
#define WAIT_IN_SUSPEND 0
#define STOP_IN_SUSPEND 1

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
void TestApp_Init(void);
extern void set_MCG_PEE_Mode(void); // Needed to resume correct clock upon wake
extern void all_led_off(void);
/***************************************************************/
// function prototypes from AN4503
extern void sleep(void);
extern void deepsleep(void);
extern void enter_wait(void);
extern void enter_stop(void);
extern int enter_vlpr(char lpwui_value);
extern void exit_vlpr(void);
extern void enter_vlps(void);
extern void enter_lls(void);
extern void enter_vlls3(void);
extern void enter_vlls2(void);
extern void enter_vlls1(void);
extern void enter_vlls0(unsigned char PORPO_value);
/***************************************************************/
/****************************************************************************
 * Global Variables - None
 ****************************************************************************/
boolean main_App_Should_Suspend = FALSE; // Flag to enter suspend mode

/*****************************************************************************
 * Local Types - None
 *****************************************************************************/

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static void USB_App_Callback(uint_8 controller_ID, uint_8 event_type,
                            void* val);
static uint_8 USB_App_Param_Callback(uint_8 request, uint_16 value, uint_16 wIndex,
                                    uint_8_ptr* data,
                                    USB_PACKET_SIZE* size);
static void KeyBoard_Events_Process(void);

/*****************************************************************************
 * Local Variables
 *****************************************************************************/
static boolean keyboard_init=FALSE;             /* Keyboard App Init Flag */
static uint_8 rpt_buf[KEYBOARD_BUFF_SIZE];      /* Key Press Report Buffer */
static uint_8 null_buf[KEYBOARD_BUFF_SIZE];     /* Key Release Report Buffer */
static uint_8 g_app_request_params[2];          /* for get/set idle and
                                                protocol requests */

/*****************************************************************************
 * Local Functions
 *****************************************************************************/
/******************************************************************************
 * @name:         KeyBoard_Events_Process
 *
 * @brief:        This function gets the input from keyboard, the keyboard
 *                does not include the code to filter the glitch on keys since
 *                it is just for demo
 *
 * @param:        None
 *
 * @return:       None
 *
 *****************************************************************************
 * This function sends the keyboard data depending on which key is pressed on
 * the board
 *****************************************************************************/
static void KeyBoard_Events_Process(void)
{
    static uint_8 Key_Index = 2;
    static uint_8 New_Key_Pressed = 0;

    kbi_stat &= KBI_STAT_MASK;
    if((Key_Index == 2) && (kbi_stat))
    {
        if(kbi_stat & PAGEUP_PRESS)
        {
            kbi_stat &= (uint_8)(~PAGEUP_PRESS);
            rpt_buf[Key_Index ++] = KEY_PAGEUP;
        }
        else if(kbi_stat & PAGEDWON_PRESS)
        {
            kbi_stat &= (uint_8)(~PAGEDWON_PRESS);
            rpt_buf[Key_Index ++] = KEY_PAGEDOWN;
        }
        else if(kbi_stat & SPACE_PRESS)
        {
            kbi_stat &= (uint_8)(~SPACE_PRESS);
            rpt_buf[Key_Index ++] = KEY_SPACEBAR;
        }
        else if(kbi_stat & PRTSCRN_PRESS)
        {
            kbi_stat &= (uint_8)(~PRTSCRN_PRESS);
            rpt_buf[Key_Index ++] = KEY_KEYPAD_NUM_LOCK_AND_CLEAR;
        }
    }

    if(Key_Index == 3)
    {
        (void)USB_Class_HID_Send_Data(CONTROLLER_ID,HID_ENDPOINT,rpt_buf,
                                                        KEYBOARD_BUFF_SIZE);
        New_Key_Pressed = 1;
        Key_Index = 2;
        return;
    }

    if(New_Key_Pressed == 1)
    {
        *((uint_32_ptr)null_buf) = 0;
        New_Key_Pressed = 0;
        (void)USB_Class_HID_Send_Data(CONTROLLER_ID,HID_ENDPOINT,null_buf,
                                                        KEYBOARD_BUFF_SIZE);
    }

    return;
}

/******************************************************************************
 *
 *    @name        USB_App_Callback
 *
 *    @brief       This function handles USB Class callback
 *
 *    @param       controller_ID    : Controller ID
 *    @param       event_type       : value of the event
 *    @param       val              : gives the configuration value
 *
 *    @return      None
 *
 *****************************************************************************
 * This function is called from the class layer whenever reset occurs or enum
 * is complete. After the enum is complete this function sets a variable so
 * that the application can start
 *****************************************************************************/
static void USB_App_Callback (
      uint_8 controller_ID, /* [IN] Controller ID */
      uint_8 event_type,    /* [IN] value of the event */
      void* val             /* [IN] gives the configuration value */
)
{
    UNUSED (controller_ID)
    UNUSED (val)
    if((event_type == USB_APP_BUS_RESET) || (event_type == USB_APP_CONFIG_CHANGED))
    {
        keyboard_init=FALSE;
    }
    else if(event_type == USB_APP_ENUM_COMPLETE)
    {   /* if enumeration is complete set keyboard_init
           so that application can start */
        keyboard_init=TRUE;
    }
    else if(event_type == USB_APP_ERROR)
    {
        /* user may add code here for error handling */
    }
    /* Added for USB Suspend/Resume */
    else if (event_type == USB_APP_BUS_SUSPEND)
    {
    	// This is the app callback, so we shouldn't halt the ISR by doing anything here.
    	main_App_Should_Suspend = keyboard_init;
    }
    else if (event_type == USB_APP_BUS_RESUME)
    {
    	/* For the keyboard application, it is best to let the main app 
    	 * handle all resume processing. Because of the software flow, we know
    	 * that after initiating the STOP mode and waking up, the application
    	 * will just continue, so we can resume there.
    	 */
    }
 	/* End addition */
    
    return;
}

/******************************************************************************
 *
 *    @name        USB_App_Param_Callback
 *
 *    @brief       This function handles USB-HID Class callback
 *
 *    @param       request  :  request type
 *    @param       value    :  give report type and id
 *    @param       data     :  pointer to the data
 *    @param       size     :  size of the transfer
 *
 *    @return      status
 *                 USB_OK   :  Always
 *
 *****************************************************************************
 * This function is called whenever a HID class request is received. This
 * function handles these class requests.
 *****************************************************************************/
static uint_8 USB_App_Param_Callback (
      uint_8 request,        /* [IN] request type */
      uint_16 value,         /* [IN] report type and ID */
  		uint_16 wIndex,		 /* [IN] Interface */
      uint_8_ptr* data,      /* [OUT] pointer to the data */
      USB_PACKET_SIZE* size  /* [OUT] size of the transfer */
)
{
    uint_8 status = USB_OK;
    uint_8 direction =  (uint_8)((request & USB_HID_REQUEST_DIR_MASK) >>3);
    uint_8 index = (uint_8)((request - 2) & USB_HID_REQUEST_TYPE_MASK);
                                         /* index == 0 for get/set idle,
                                            index == 1 for get/set protocol
                                         */
    *size =0;
    /* handle the class request */
    switch(request)
    {
        case USB_HID_GET_REPORT_REQUEST :
            *data = &rpt_buf[0]; /* point to the report to send */
            *size = KEYBOARD_BUFF_SIZE; /* report size */
            break;

        case USB_HID_SET_REPORT_REQUEST :
        	//for(index = 0; index < KEYBOARD_BUFF_SIZE ; index++)
            //{   /* copy the report sent by the host */
            //    rpt_buf[index] = *(*data + index);
            //}

			if(**data & 0x01) {	// turn NumLock led on request from HOST
#if defined(_MC9S08JM60_H)
				PTDD_PTDD2 = 0;
#elif (defined(__MCF52259_H__) || defined(__MCF52221_H__))
				MCF_GPIO_SETTC = MCF_GPIO_SETTC_SETTC0;	// led D10
#elif (defined(_MCF51JM128_H) || defined(_MC9S08JM16_H) || defined(_MC9S08JM60_H))
				PTDD_PTDD2 = 0;
#elif defined(MCU_MK60N512VMD100)
				GPIOA_PCOR = (1<<10);	// led E4
#elif defined(MCU_MK53N512CMD100) || (defined MCU_MK40N512VMD100)
				GPIOC_PCOR = (1<<8);	// led D7
#elif (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
				GPIOC_PCOR = (1<<9);	// Turn on Led D501
#elif defined MCU_MK70F12
				GPIOA_PCOR = (1<<10);	// led D12
#else
				;
#endif
			}	else { // turn NumLock led off request from HOST
#if defined(_MC9S08JM60_H)
				PTDD_PTDD2 = 1;
#elif (defined(__MCF52259_H__) || defined(__MCF52221_H__))
				MCF_GPIO_CLRTC = (vuint8) ~MCF_GPIO_CLRTC_CLRTC0;	// led D10
#elif (defined (_MCF51JM128_H) || defined(_MC9S08JM16_H) || defined(_MC9S08JM60_H))
				PTDD_PTDD2 = 1;
#elif defined(MCU_MK60N512VMD100)
				GPIOA_PSOR = (1<<10);	// led E4
#elif defined(MCU_MK53N512CMD100) || (defined MCU_MK40N512VMD100)
				GPIOC_PSOR = (1<<8);	// led D7
#elif (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
				GPIOC_PSOR = (1<<9);	// Turn off Led D501
#elif defined MCU_MK70F12
				GPIOA_PSOR = (1<<10);	// led D12
#else
				;
#endif
			}
			
			if(**data & 0x02) {	// turn CapsLock led on request from HOST
#if (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
				GPIOC_PCOR = (1<<10);	// Turn on Led D502
#else
				;
#endif
			}	else { // turn CapsLock led off request from HOST
#if (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
				GPIOC_PSOR = (1<<10);	// Turn off Led D502
#else
				;
#endif
			}
            break;

        case USB_HID_GET_IDLE_REQUEST :
            /* point to the current idle rate */
            *data = &g_app_request_params[index];
            *size = REQ_DATA_SIZE;
            break;

        case USB_HID_SET_IDLE_REQUEST :
            /* set the idle rate sent by the host */
            if(index <2)
            {
                g_app_request_params[index] =(uint_8)((value & MSB_MASK) >>
                                                      HIGH_BYTE_SHIFT);
            }
            break;

        case USB_HID_GET_PROTOCOL_REQUEST :
            /* point to the current protocol code
               0 = Boot Protocol
               1 = Report Protocol*/
            *data = &g_app_request_params[index];
            *size = REQ_DATA_SIZE;
            break;

        case USB_HID_SET_PROTOCOL_REQUEST :
            /* set the protocol sent by the host
                 0 = Boot Protocol
                 1 = Report Protocol*/
            if(index < 2)
            {
                g_app_request_params[index] = (uint_8)(value);
            }
            break;
    }

    return status;
}

/******************************************************************************
 *
 *   @name        TestApp_Init
 *
 *   @brief       This function is the entry for Keyboard Application
 *
 *   @param       None
 *
 *   @return      None
 *
 *****************************************************************************
 * This function starts the keyboard application
 *****************************************************************************/

void TestApp_Init(void)
{
    uint_8 error;

    /* initialize status of Key pressed */
    kbi_stat = 0x00;
    DisableInterrupts;
    #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
     usb_int_dis();
    #endif
    /* Initialize the USB interface */
    error = USB_Class_HID_Init(CONTROLLER_ID, USB_App_Callback,
                                NULL, USB_App_Param_Callback);
    if(error != USB_OK)
    {
        /* USB HID Class Initialization Failed */
        return;
    }
    EnableInterrupts;
    #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
     usb_int_en();
    #endif
}

/******************************************************************************
 *
 *   @name        TestApp_Task
 *
 *   @brief       Application task function. It is called from the main loop
 *
 *   @param       None
 *
 *   @return      None
 *
 *****************************************************************************
 * Application task function. It is called from the main loop
 *****************************************************************************/
void TestApp_Task(void)
{
	uint_8 controller_id = CONTROLLER_ID;
	uint_8 state;
        /* call the periodic task function */
        USB_Class_HID_Periodic_Task();

        if(keyboard_init) /*check whether enumeration is
                                        complete or not */
        {
           /* run the button emulation code */
           KeyBoard_Events_Process();
           
           /*TODO Remove MCU_MK20D5 define check and port on other processors */
		#if defined (MCU_MK20D5)
           if ( main_App_Should_Suspend ) {
          	 all_led_off(); /* Disable LEDs in Suspend */

          	 (void) _usb_device_get_status( &controller_id, USB_STATUS_DEVICE, &state );
          		if ( ( state & REMOTE_WAKEUP_STATUS_MASK ) == 0 ) { /* Remote Wakeup Disabled by Host */
          			/* Disable Pin Wakeups - Functionality needs added for other boards */
								#if defined(MCU_MK20D5)
									DisableInterrupts;
									NVICICER1 = (uint32_t)(1 << ((42)%32)); /* Disable interrupt on PORTC */
									NVICICPR1 = (uint32_t)(1 << ((42)%32)); /* Clear any pending interrupt on PORTC */
									PORTC_PCR1 &= ~PORT_PCR_IRQC_MASK; /* Disable Interrupts on PTC1 */
									PORTC_PCR2 &= ~PORT_PCR_IRQC_MASK; /* Disable Interrupts on PTC2 */									
									PORTC_ISFR = (1 << 1) | (1 << 2); /* Clear any pending interrupt on PTC1 & PTC2 */
									EnableInterrupts;
								#endif
          		}
          	 
							#if WAIT_IN_SUSPEND
          				enter_wait();
							#elif STOP_IN_SUSPEND
								#if defined(MCU_MK20D5)
          				PMC_REGSC |= 0x10; // e4481 PMC: STOP mode recovery unstable workaround
								#endif
          				
          			enter_stop();
          			/* The processor will wait for an AWIC interrupt, and then continue on.
          			 * Any lines following this assume we are waking up.
          			 */

								#if defined(MCU_MK20D5)
									PMC_REGSC &= ~(0x10); // e4481 workaround
								#endif

								set_MCG_PEE_Mode(); /* Resume proper clock */
							#endif // STOP_IN_SUSPEND
							USB_Class_Initiate_Resume(CONTROLLER_ID); /* Resume USB Bus */

							/* Reenable Pin Wakeups - Functionality needs added for other boards */
						 	#if defined(MCU_MK20D5)
          	 		PORTC_PCR1 |= PORT_PCR_IRQC(0xA); /* GPIO_INT_EDGE_FALLING */
          	 		PORTC_PCR2 |= PORT_PCR_IRQC(0xA); /* GPIO_INT_EDGE_FALLING */
          	 		/* Enable interrupt */
          	 		PORTC_ISFR = (1 << 1);
          	 		PORTC_ISFR = (1 << 2);
          	 		NVICICPR1 = 1 << ((42)%32); /* Clear any pending interrupt on PORTC */
          	 		NVICISER1 = 1 << ((42)%32); /* Enable interrupts from PORTC module */
						 	#endif
          	 
          	 	main_App_Should_Suspend = FALSE;
           }
		#endif
        }
        
}

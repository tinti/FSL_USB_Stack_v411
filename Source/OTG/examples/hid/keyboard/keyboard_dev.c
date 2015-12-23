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
#include "keyboard_dev.h"   /* Keyboard Application Header File */
#include "usb_dciapi.h" /* USB DCI API Header File */
#include "user_config.h"
#include "RealTimerCounter.h"
#if MAX_TIMER_OBJECTS
extern uint_8 TimerQInitialize(uint_8 ControllerId);
#endif


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

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
void TestApp_Init(void);
/****************************************************************************
 * Global Variables - None
 ****************************************************************************/
#define TIMER_NOT_STARTED      0xFF
volatile uint_8 kbi_stat = 0x00;		/* Status of the Key Pressed */
uint_8 kbi_debounce_tmr = TIMER_NOT_STARTED;
boolean start_kbi_debounce_tmr;
extern uint_8               host_stack_active; /* TRUE if the host stack is active */
extern uint_8               dev_stack_active;  /* TRUE if the peripheral stack is active */
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
static void Kbi_Init(void);
#ifdef TIMER_CALLBACK_ARG
static void KbiDebounceTimerCallback(void * arg);
#else
static void KbiDebounceTimerCallback(void);
#endif

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
        else if(kbi_stat & PAGEDOWN_PRESS)
        {
            kbi_stat &= (uint_8)(~PAGEDOWN_PRESS);
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
            rpt_buf[Key_Index ++] = KEY_PRINTSCREEN;
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
	  uint_16 wIndex,		 /* [IN] interface */
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
            for(index = 0; index < KEYBOARD_BUFF_SIZE ; index++)
            {   /* copy the report sent by the host */
                rpt_buf[index] = *(*data + index);
            }
            *size = 0;
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
 * @name        Kbi_Init
 *
 * @brief       This routine is the initialization function
 *
 * @param       None
 *
 * @return      None
 *
 ******************************************************************************
 * This function initializes the keyboard interrupt module
 *****************************************************************************/
 static void Kbi_Init(void)
{
#if (defined(_MC9S08MM128_H) || defined(_MCF51MM256_H) || (defined _MCF51JE256_H) || defined(_MC9S08JE128_H))
    PTCPE |= 0x40;
    PTCD &= 0xBF;
    KBI2SC_KB2ACK = 1;   
    KBI2SC_KBI2MOD = 0;
    KBI2ES = 0x00;
    KBI2PE = 0x02;
    KBI2SC_KB2IE = 1;
#endif
#ifdef _MCF51JM128_H
    PTGDD &= 0xF0;     /* set PTG0-3 to input */
    PTGPE |= 0x0F;     /* enable PTG0-3 pull-up resistor */
    //KBI1SC_KBIE = 1;
    //KBI1SC_KBACK = 1;
    KBI1ES &= ~(0xC3);
    KBI1PE |= 0xC3;      /* enable KBI0,1,6,7 */
#endif
#if (defined(_MC9S08JM16_H) || defined(_MC9S08JM60_H))	
    PTGDD &= 0xF0;     /* set PTG0-3 to input */
    PTGPE |= 0x0F;     /* enable PTG0-3 pull-up resistor */
    KBISC_KBIE = 1;
    KBISC_KBACK = 1;
    KBIES = 0x00;
    KBIPE = 0xC3;      /* enable KBI0,1,6,7 */
#endif
#ifdef _MC9S08JS16_H
    PTAD |= 0x21;
    PTADD |= 0x21;

    PTAPE = 0xC2;      /*enable PTG0-3 pull-up resistor*/
    PTBPE = 0x08;

    /*
      Clear Interrupt Status Register
      and Interrupt ACK to avoid spurious KBI Interrupt
    */
    KBISC_KBIE = 1;
    KBISC_KBACK = 1;
    KBIES = 0x00;
    KBIPE = 0xC2;      /*enable KBI0,1,6,7*/
#endif
}

/******************************************************************************
 * @name       Kbi_ISR
 *
 * @brief      The routine is the KBI interrupt service routine
 *
 * @param	   None
 *
 * @return     None
 *
 *****************************************************************************
 * This function is hooked on to the KBI interrupt and is called each time
 * KBI interrupt is generated
 *****************************************************************************/
void App_PeripheralKbiServiceRoutine(void)


{  
 /* Start a debounce timer to sample the GPIO after 50ms. 
  * This removes the spikes in detecting a pressed keyboard.
  */
 start_kbi_debounce_tmr = TRUE;  

 /* Acknowledge the interrupt */
}


/******************************************************************************
 * @name       KbiDebounceTimerCallback
 *
 * @brief      The routine samples the GPIO input to determine the pressed key
 *
 * @param	     None
 *
 * @return     None
 *
 *****************************************************************************
 * This function is hooked on to the KBI interrupt and is called 50ms after the.
 * KBI interrupt is generated
 *****************************************************************************/
#ifdef TIMER_CALLBACK_ARG
static void KbiDebounceTimerCallback(void * arg)
#else
static void KbiDebounceTimerCallback(void)
#endif
{
  #ifdef TIMER_CALLBACK_ARG
    UNUSED (arg)
  #endif
 /* Reset the debounce timer */
 kbi_debounce_tmr = TIMER_NOT_STARTED;
 
 /* Sample the GPIO */
 #if (defined(_MCF51MM256_H) || defined(_MC9S08MM128_H) || (defined _MCF51JE256_H) || defined(_MC9S08JE128_H))
    if(!PTCD_PTCD6)
        kbi_stat = 2;
 #endif
 #ifdef _MC9S08JS16_H
    if(!PTAD_PTAD1)
        kbi_stat |= 0x02;

    if(!PTAD_PTAD6)
        kbi_stat |= 0x04;

    if(!PTAD_PTAD7)
        kbi_stat |= 0x08;     	 
 #endif	  
 #if (defined(_MCF51JM128_H) || defined(_MC9S08JM16_H) || defined(_MC9S08JM60_H))    
    kbi_stat = (uint_8)~(PTGD & 0x0F); /* get which buttons were pressed */
 #endif	
 
}


/******************************************************************************
 *
 *   @name        App_PeripheralInit
 *
 *   @brief       This function is the entry for Keyboard Application (Peripheral)
 *
 *   @param       None
 *
 *   @return      None
 *
 *****************************************************************************
 * This function starts the keyboard application
 *****************************************************************************/

uint_32 App_PeripheralInit(void)
{
    uint_32 error;

    host_stack_active  = FALSE;
    dev_stack_active = TRUE;
    /* initialize status of Key pressed */
    kbi_stat=0x00;
    DisableInterrupts;
    #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
     usb_int_dis();
    #endif
    
    
    Kbi_Init();
    #if MAX_TIMER_OBJECTS
    (void)TimerQInitialize(0);
    #endif
    
    
    /* Initialize the USB interface */
    error = (uint_32)USB_Class_HID_Init(CONTROLLER_ID, USB_App_Callback,
                                NULL, USB_App_Param_Callback);

    EnableInterrupts;
    #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
     usb_int_en();
    #endif
    
    return error;
}


/******************************************************************************
 *
 *   @name        App_PeripheralUninit
 *
 *   @brief       This function is the entry for Keyboard Application (Peripheral)
 *
 *   @param       None
 *
 *   @return      None
 *
 *****************************************************************************
 * This function starts the keyboard application
 *****************************************************************************/

void App_PeripheralUninit(void)
{       
    DisableInterrupts;
    #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
     usb_int_dis();
    #endif
    /* Uninitialize the USB interface */
     USB_DCI_Shutdown(0);
     INT_ENB = 0;
     CTL = 0;
     INT_STAT = 0xff;
     USBTRC0 &= ~USBTRC0_USBRESMEN_MASK;
     TOKEN = 0;
     ENDPT0 = 0;
     ENDPT1 = 0;
     ENDPT2 = 0;
     ENDPT3 = 0;
     ENDPT4 = 0;
     ENDPT5 = 0;
     ENDPT6 = 0;
     ENDPT7 = 0;
     ENDPT8 = 0;
     ENDPT9 = 0;
     ENDPT10 = 0;
     ENDPT11 = 0;
     ENDPT12 = 0;
     ENDPT13 = 0;
     ENDPT14 = 0;
     ENDPT15 = 0;
     ADDR = 0;
     
  
     RTCSC_RTIE = 0x00;
     RTCSC_RTIF = 0x01;
     KBI1PE &= ~(0xC3) ;
     CTL = CTL_HOST_MODE_EN_MASK;
     CTL = 0;
    
    EnableInterrupts;
    #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
     usb_int_en();
    #endif
    
   host_stack_active  = FALSE;
   dev_stack_active = FALSE; 
}

/******************************************************************************
 *
 *   @name        App_PeripheralTask
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
void App_PeripheralTask(void)
{


       #if MAX_TIMER_OBJECTS
       if((start_kbi_debounce_tmr) && (kbi_debounce_tmr == TIMER_NOT_STARTED))
       {
         TIMER_OBJECT TimerObject;
  
         TimerObject.msCount = 50; /* 50ms count */
         TimerObject.pfnTimerCallback = KbiDebounceTimerCallback;
         kbi_debounce_tmr = AddTimerQ(&TimerObject);
         
         if(kbi_debounce_tmr == (uint_8)ERR_TIMER_QUEUE_FULL)
         {
           /* Could not add the debounce timer. Call the KBI callback directly */
           #ifdef TIMER_CALLBACK_ARG
            KbiDebounceTimerCallback(0);
           #else
            KbiDebounceTimerCallback();
           #endif
         }
         start_kbi_debounce_tmr = FALSE;
       }
       #else
        #warning "MAX_TIMER_OBJECTS is 0. The KBI is possible to detect false press events (cannot add KBI debounce TMR) "
        #ifdef TIMER_CALLBACK_ARG
          KbiDebounceTimerCallback(0);
        #else
          KbiDebounceTimerCallback();
        #endif
       #endif
       

        /* call the periodic task function */
       USB_Class_HID_Periodic_Task();

       if(keyboard_init) /*check whether enumeration is
                                        complete or not */
        {
           /* run the button emulation code */
           KeyBoard_Events_Process();
        }
        
}

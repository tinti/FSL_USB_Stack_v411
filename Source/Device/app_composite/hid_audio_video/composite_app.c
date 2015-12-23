/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
*
**************************************************************************//*!
*
* @file composite_app.c
*
* @author
*
* @version
*
* @date May-28-2009
*
* @brief  The file emulates a composite device, contains HID, Audio, Video 
*         Feature 
*****************************************************************************/

/******************************************************************************
* Includes
*****************************************************************************/
#include "hidef.h"          /* for EnableInterrupts macro */
#include "derivative.h"     /* include peripheral declarations */
#include "types.h"          /* User Defined Data Types */
#include "usb_composite.h"        /* USB Composite layer Header File */
#include "audio_speaker.h"
#include "virtual_camera.h"
#include "mouse_button.h"
#include "audio_pwm.h"
#include "sci.h"

#if  (defined MCU_mcf51jf128)
#include "audio_ftm_cfv1_plus.h"
#elif  (defined __MCF52xxx_H__)
#include "audio_pit1_cfv2.h"
#elif  (defined __MK_xxx_H__)
#include "audio_pit1_kinetis.h"
#else
#include "audio_cmt.h"
#endif


#include "composite_app.h"   /* Composite Application Header File */
#if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
#include "exceptions.h"
#endif

/* skip the inclusion in dependency state */
#ifndef __NO_SETJMP
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>

/******************************************************************************
* Local variables
*****************************************************************************/
static CLASS_APP_CALLBACK_STRUCT audio_class_callback =
{
    USB_Audio_App_Callback,
    NULL,
    NULL,
    NULL,
};
static CLASS_APP_CALLBACK_STRUCT video_class_callback =
{
    USB_Video_App_Callback,
    NULL,
    NULL,
    NULL,
};
static CLASS_APP_CALLBACK_STRUCT Hid_class_callback =
{
    USB_HID_App_Callback,
    NULL,
    NULL,
    USB_HID_App_Param_Callback ,
};
static COMPOSITE_CALLBACK_STRUCT usb_composite_callback =
{
    COMP_CLASS_UNIT_COUNT,
    {
        &Hid_class_callback,

        &video_class_callback,

        &audio_class_callback,
    }
};

/******************************************************************************
*
*   @name        TestApp_Init
*
*   @brief       This function is the entry for Mouse Application
*
*   @param       None
*
*   @return      None
*
*****************************************************************************
* This function starts the Composite device Application
*****************************************************************************/
void TestApp_Init(void)
{
    DisableInterrupts;
    #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
    usb_int_dis();
    #endif
    #if !(defined _MC9S08JS16_H)
    #if (defined MCU_mcf51jf128)
    sci1_init();
    #else
    /* For M52221 Demo board baudrate is 19200 */
    sci_init();
    #endif
    #endif

    #if  ((defined __MCF52xxx_H__) || (defined __MK_xxx_H__))
    pit1_init();
    #elif (defined _MC9S08JM60_H) || (defined _MC9S08JM16_H) || (defined _MC9S08JS16_H)
    rtc_init();
    #elif (defined MCU_mcf51jf128)
    ftm0_init();
    #else
    cmt_init();
    #endif

    pwm_init();
    (void)USB_Composite_Init(CONTROLLER_ID,&usb_composite_callback);

    EnableInterrupts;
    #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
    usb_int_en();
    #endif
}

/******************************************************************************
*
*    @name        TestApp_Task
*
*    @brief       This function use to send data
*
*    @param       None
*
*    @return      None
*
*****************************************************************************
*
*****************************************************************************/
void TestApp_Task(void)
{
    Test_Audio_App_Task();
    Test_Video_App_Task();
    Test_HID_App_Task();
}

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
* @date 
*
* @brief  The file emulates a virtual com port and a mouse with buttons
*****************************************************************************/

/******************************************************************************
* Includes
*****************************************************************************/
#include "hidef.h"          /* for EnableInterrupts macro */
#include "derivative.h"     /* include peripheral declarations */
#include "types.h"          /* User Defined Data Types */
#include "usb_composite.h"  /* USB Composite layer File */
#include "virtual_com.h"    /* CDC Application Header File */
#include "disk.h"           /* MSD Application Header File */


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
static CLASS_APP_CALLBACK_STRUCT Cdc_class_callback =
{
    USB_Cdc_App_Callback,
    NULL,
    USB_Cdc_Notify_Callback,
    NULL,
};

static CLASS_APP_CALLBACK_STRUCT Msd_class_callback =
{
    USB_Msd_App_Callback,
    NULL,
    USB_Msd_Event_Callback,
    NULL ,
};

static COMPOSITE_CALLBACK_STRUCT usb_composite_callback =
{
    COMP_CLASS_UNIT_COUNT,
    {
        &Msd_class_callback,
        &Cdc_class_callback,
    }
};

/******************************************************************************
*
*   @name        TestApp_Init
*
*   @brief       This function is the entry for CDC and MSD Composite device
*                Application
*
*   @param       None
*
*   @return      None
*
*****************************************************************************
* This function starts the CDC and MSD Composite device Application
*****************************************************************************/
void TestApp_Init(void)
{
  
    Msd_App_Init();
    DisableInterrupts;
    #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
    usb_int_dis();
    #endif

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
    Test_Cdc_App_Task();
    Test_Msd_App_Task();
}
/* EOF */

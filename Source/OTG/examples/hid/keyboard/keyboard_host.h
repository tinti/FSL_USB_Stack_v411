/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 **************************************************************************//*!
 *
 * @file keyboard.h
 *
 * @author 
 *
 * @version 
 *
 * @date May-28-2009
 *
 * @brief The file contains Macro's and functions needed by the keyboard 
 *        application
 *
 *****************************************************************************/


#ifndef _KEYBOARD_HOST_H
#define _KEYBOARD_HOST_H

#include "types.h"
#include "host_common.h"

/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/


#define  CONTROLLER_ID         (0)   /* ID to identify USB Device Controller */ 

/*****************************************************************************
 * Global variables
 *****************************************************************************/


/*****************************************************************************
 * Global Functions
 *****************************************************************************/
extern USB_STATUS App_Host_Init(void) ;
extern void App_Host_Task(void);
extern void App_Host_Shut_Down(void);

#endif 

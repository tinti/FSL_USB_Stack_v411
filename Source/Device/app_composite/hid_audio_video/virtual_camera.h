/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
*
**************************************************************************//*!
*
* @file virtual_camera.h
*
* @author
*
* @version
*
* @date Jul-20-2010
*
* @brief The file contains Macro's and functions required for Virtual Camera
*        Loopback Application
*
*****************************************************************************/

#ifndef _VIRTUAL_CAMERA_H
#define _VIRTUAL_CAMERA_H

#include "types.h"
#include "user_config.h"

/******************************************************************************
* Constants - None
*****************************************************************************/

/******************************************************************************
* Macro's
*****************************************************************************/
#define  CONTROLLER_ID      (0)   /* ID to identify USB CONTROLLER */

#define  KBI_STAT_MASK      (0x0F)

/*****************************************************************************
* Global variables
*****************************************************************************/

/*****************************************************************************
* Global Functions
*****************************************************************************/
#define HEADER_PACKET_SIZE 12

#ifndef MCU_MKL25Z4
     #define VIDEO_PACKET_SIZE 192
#else
     #define VIDEO_PACKET_SIZE 96
#endif

extern void USB_Video_App_Callback(uint_8 controller_ID,
uint_8 event_type, void* val);

extern void Test_Video_App_Task(void);
#endif

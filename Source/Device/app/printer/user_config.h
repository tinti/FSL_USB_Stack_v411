/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
*
**************************************************************************//*!
*
* @file user_config.h
*
* @author
*
* @version
*
* @date May-2012
*
* @brief The file contains User Modifiable Macros for Printer Application
* ****************************************************************************/
#include "derivative.h"

#if defined MCU_MK70F12
#define  HIGH_SPEED_DEVICE                (0)
#elif defined __MCF52277_H__
#define  HIGH_SPEED_DEVICE                (1)
#else
#define  HIGH_SPEED_DEVICE                (0)
#endif

#if (defined MCU_MK20D7) || (defined MCU_MK40D7)
#define MCGOUTCLK_72_MHZ
#endif

#if ((defined __MK_xxx_H__)||(defined MCU_mcf51jf128))
/* 1s between simulated key press events */
#define  KEY_PRESS_SIM_TMR_INTERVAL       (1000)
#else
#ifdef __MCF52277_H__
#define  BUTTON_PRESS_SIMULATION          (1)
/* 2s between simulated key press events */
#define  KEY_PRESS_SIM_TMR_INTERVAL       (2000)        
#endif
#endif

#define LONG_SEND_TRANSACTION       /* support to send large data pkts */
#define LONG_RECEIVE_TRANSACTION    /* support to receive large data pkts */
#ifndef _MC9S08JS16_H
#define USB_OUT_PKT_SIZE 32			/* Define the maximum data length received from the host */
#else
#define USB_OUT_PKT_SIZE 16			/* Define the maximum data length received from the host */
#endif
#define MAX_TIMER_OBJECTS                 1
#define PRINTER_SEND_BUFF_SIZE            64

/* Don't Change this definition*/
#define USB_PACKET_SIZE                   uint_32 

#ifndef _MCF51JM128_H
/* Use double buffered endpoints 5 & 6. To be only used with S08 cores */
#define DOUBLE_BUFFERING_USED
#endif

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
 * @date May-28-2009
 *
 * @brief The file contains User Modifiable Macros for MSD Application
 *
 *****************************************************************************/
#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#include "derivative.h"

#if (defined MCU_MK70F12) || (defined __MCF52277_H__)
	#define  HIGH_SPEED_DEVICE	(0)
#else
	#define  HIGH_SPEED_DEVICE	(0)
#endif

// Macro for alignment to specific byte boundary
#define  USB_MEM32_ALIGN(n)		((n) + (-(n) & 31))
#define  USB_MEM512_ALIGN(n)	((n) + (-(n) & 511))
#define  USB_MEM4096_ALIGN(n)	((n) + (-(n) & 4095))
#ifdef __MCF52277_H__
#define  BUTTON_PRESS_SIMULATION      (1)
#define  KEY_PRESS_SIM_TMR_INTERVAL	  (2000)		/* 2s between simulated key press events */
#endif

//#define USE_SDCARD			(1)
#ifdef USE_SDCARD
  #define RAM_DISK_APP        (0)
  #define SD_CARD_APP         (1)
//#define USE_SDHC_PROTOCOL   1
//#define USE_SPI_PROTOCOL    0
#else
  #define RAM_DISK_APP        (1)
  #define SD_CARD_APP         (0)
#endif
#define LONG_SEND_TRANSACTION       /* support to send large data pkts */
#define LONG_RECEIVE_TRANSACTION    /* support to receive large data pkts */

// #define USB_LOWPOWERMODE /* This MACRO enables STOP3 Mode Transition */  

#define MAX_TIMER_OBJECTS   5

#define MSD_RECEIVE_BUFFER_SIZE       (512)
#define MSD_SEND_BUFFER_SIZE          (512)
/* Don't Change this definition*/
#define USB_PACKET_SIZE  uint_32 /* support 8 bit packet size */ 

#ifndef _MCF51JM128_H
  /* Use double buffered endpoints 5 & 6. To be only used with S08 cores */
  #define DOUBLE_BUFFERING_USED
#endif

#endif // _USER_CONFIG_H_

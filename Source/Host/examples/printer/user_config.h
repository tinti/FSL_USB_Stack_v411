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
 * @date
 *
 * @brief
 *
 *****************************************************************************/
/* INCLUDE *---------------------------------------------------------------*/
#include "derivative.h"

/* MACRO *-----------------------------------------------------------------*/
#if(defined MCU_MK70F12) 
    #define  HIGH_SPEED_DEVICE    (0)
#elif (defined __MCF52277_H__)
    #define  HIGH_SPEED_DEVICE    (1)
#else
    #define  HIGH_SPEED_DEVICE    (0)
#endif

/* Macro for alignment to specific byte boundary */
#define USB_MEM32_ALIGN(n)        ((n) + (-(n) & 31))
#define USB_MEM512_ALIGN(n)       ((n) + (-(n) & 511))
#define USB_MEM4096_ALIGN(n)      ((n) + (-(n) & 4095))

/* EOF */

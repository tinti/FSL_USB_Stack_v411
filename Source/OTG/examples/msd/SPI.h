/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 **************************************************************************//*!
 *
 * @file spi.h
 *
 * @author
 *
 * @version
 *
 * @date May-28-2009
 *
 * @brief  This file is SPI Driver Header File
 *****************************************************************************/
#ifndef __SPI__
#define __SPI__


/* Includes */
#include "derivative.h"


/* Defines */

    // Slave Select
#define SPI_SS    PTBD_PTBD3      
#define _SPI_SS   PTBDD_PTBDD3    
#define ENABLE    0
#define DISABLE   1


/* Prototypes */
void   SPI_Init(void);
uint_8 SPI_Receive_byte(void);
void   SPI_Send_byte(uint_8 data);
void   SPI_High_rate(void);


#endif /* __SPI__ */

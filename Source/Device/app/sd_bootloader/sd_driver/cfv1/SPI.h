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

#if defined(_MCF51MM256_H)
/* Slave Select */
    #define SPI_SS    PTAD_PTAD0      
    #define _SPI_SS   PTADD_PTADD0    
    #define ENABLE    0
    #define DISABLE   1
#elif defined(MCU_mcf51jf128)
/* Slave Select */
    #define SPI_SS     PTF_D      
    #define ENABLE    0x01
#endif



/* Prototypes */
void   SPI_Init(void);
uint_8 SPI_Receive_byte(void);
void   SPI_Send_byte(uint_8 data);
void   SPI_High_rate(void);
void   SPI_clr_SS(void);
void   SPI_set_SS(void);

#endif /* __SPI__ */

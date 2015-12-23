/*****************************************************************************
* IIC Serial Port declarations.
*
* (c) Copyright 2007, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _IIC_h
#define _IIC_h





#ifndef gSystemClock_d
#define gSystemClock_d     24           /* 16 MHz. */
#endif

#if gSystemClock_d == 8
#define gIIC_FrequencyDivider_50000_c     (uint8_t)((0<<6)| 0x1D)
#define gIIC_FrequencyDivider_100000_c    (uint8_t)((0<<6)| 0x14)
#define gIIC_FrequencyDivider_200000_c    (uint8_t)((0<<6)| 0x0B)
#define gIIC_FrequencyDivider_400000_c    (uint8_t)((0<<6)| 0x00)
#endif

#if gSystemClock_d == 12
#define gIIC_FrequencyDivider_50000_c     (uint8_t)((0<<6)| 0x1F)
#define gIIC_FrequencyDivider_100000_c    (uint8_t)((2<<6)| 0x05)
#define gIIC_FrequencyDivider_200000_c    (uint8_t)((1<<6)| 0x05)
#define gIIC_FrequencyDivider_400000_c    (uint8_t)((0<<6)| 0x05)
#endif

#if gSystemClock_d == 16
#define gIIC_FrequencyDivider_50000_c     (uint_8)((0<<6)| 0x25)
#define gIIC_FrequencyDivider_100000_c    (uint_8)((0<<6)| 0x1D)
#define gIIC_FrequencyDivider_200000_c    (uint_8)((0<<6)| 0x14)
#define gIIC_FrequencyDivider_400000_c    (uint_8)((0<<6)| 0x0B)
#endif

#if gSystemClock_d == 16780
#define gIIC_FrequencyDivider_50000_c     (uint8_t)((0<<6)| 0x25) //  52437.5
#define gIIC_FrequencyDivider_100000_c    (uint8_t)((0<<6)| 0x1D) // 104875
#define gIIC_FrequencyDivider_200000_c    (uint8_t)((0<<6)| 0x15) // 190681.81
#define gIIC_FrequencyDivider_400000_c    (uint8_t)((0<<6)| 0x0C) // 381363.63
#endif

#if gSystemClock_d == 24
#define gIIC_FrequencyDivider_50000_c     (uint_8)((1<<6)| 0x22) //  53571
#define gIIC_FrequencyDivider_100000_c    (uint_8)((2<<6)| 0x11) // 107148
#define gIIC_FrequencyDivider_200000_c    (uint_8)((2<<6)| 0x05) // 190681.81
#define gIIC_FrequencyDivider_400000_c    (uint_8)((1<<6)| 0x05) // 400000
#endif

/*list of possible baudrates */
#define  gIIC_BaudRate_50000_c    gIIC_FrequencyDivider_50000_c
#define  gIIC_BaudRate_100000_c   gIIC_FrequencyDivider_100000_c
#define  gIIC_BaudRate_200000_c   gIIC_FrequencyDivider_200000_c
#define  gIIC_BaudRate_400000_c   gIIC_FrequencyDivider_400000_c

/* Default baud rate. */
#ifndef gIIC_DefaultBaudRate_c
  #define gIIC_DefaultBaudRate_c  gIIC_FrequencyDivider_400000_c  
#endif

/* The I2C slave address */
#ifndef gIIC_DefaultSlaveAddress_c
  #define gIIC_DefaultSlaveAddress_c   0x00
#endif

#if((gIIC_DefaultSlaveAddress_c > 0x7f) || (((gIIC_DefaultSlaveAddress_c & 0x78) == 0) && ((gIIC_DefaultSlaveAddress_c & 0x07) != 0)) || ((gIIC_DefaultSlaveAddress_c & 0x78) == 0x78))
  #error Illegal Slave Address!!!
#endif

/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/

/* Initialize the IIC module */
extern void     IIC_ModuleInit(void);
/* Shut down the IIC module */
extern void     IIC_ModuleUninit(void);
/* Set the IIC module Baud Rate  */
boolean IIC_SetBaudRate(uint_8 baudRate);
/* Set the IIC module slave address */
boolean   IIC_SetSlaveAddress(uint_8 slaveAddress);
/* Tries to set free the SDA line if another IIC device keeps it low    */
void IIC_BusRecovery(void);
boolean  IIC_Bus_Busy(void);
boolean IIC_Transmit_Master(uint_8 const *pBuf, uint_32 bufLen, uint_8 destAddress) ;
boolean IIC_Receive_Master(uint_8 *pBuf, uint_32 bufLen, uint_8 destAddress) ;
boolean IIC_Transmit_RS_Receive_Master(uint_8 *p_tx_Buf, uint_32 txLen,uint_8 *p_rx_Buf, uint_32 rxLen, uint_8 destAddress);
/* Checks if Slave  Tx process is still running */
#endif    /* _IIC_h */

/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 ******************************************************************************
 *
 * THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
 * IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 **************************************************************************//*!
 *
 * @file serial_cfv1_plus.h
 *
 * @author
 *
 * @version
 *
 * @date    
 *
 * @brief   This is a header file for Serial Driver
 *****************************************************************************/
#ifndef _SERIAL_CFV1_PLUS_H
#define _SERIAL_CFV1_PLUS_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "types.h"

/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/
#define SYSTEM_CLOCK		48000000

#define SCI_CONNECT                     7
#define SCI_TRANSMIT_COMPLETE           0
#define SCI_RECEIVE_OVERRUN             1
#define SCI_RECEIVE_NOISE_DETECTED      2
#define SCI_RECEIVE_FRAME_ERROR         3
#define SCI_RECEIVE_PARITY_ERROR        4
#define SCI_RECEIVE_COMPLETE            5
#define SCI_TRANSMIT_SPURIOUS_INTERRUPT 6

#define TX_BUFFER   0
#define RX_BUFFER   1
#define MAX_BUFFER  2

#define SCI0_BASE_ADDRESS   0xFFFF8140
#define SCI1_BASE_ADDRESS   0xFFFF8160

/* Serial Driver Return Codes */
#define SERIAL_OK           0
#define SERIAL_TX_BSY       (uint_8)-1
#define SERIAL_RX_BSY       (uint_8)-2
#define SERIAL_INIT_FAILED  (uint_8)-3

#define MAX_SCI_INTERFACES   2

/******************************************************************************
 * Types
 *****************************************************************************/
#pragma pack(1)
typedef struct _BUFFER
{
    uint_8_ptr pBuffer;
    uint_16 CurOffSet;
    uint_16 Length;
    boolean InUse;
} BUFFER, *PBUFFER;

/* callback function pointer structure for Application to handle events */
typedef void(_CODE_PTR_ SCI_CALLBACK)(uint_8, uint_8, PBUFFER);

typedef struct _SCI_STRUCT
{
    PBUFFER buffer[MAX_BUFFER];
    SCI_CALLBACK app_callback;       
} SCI_STRUCT, *PTR_SCI_STRUCT;

/* Serial Initialization Structure */
typedef struct _SERIAL_INIT
{
	uint_8 ControllerId;		/* Serial Controller ID */
	boolean ParityEnable;		/* Enable Parity */
	uint_8 Parity;				/* Parity */
	uint_8 StopBits;			/* Stop Bits */
	uint_8 Mode;				/* 8 bit or 9 bit mode */
	uint_16 BaudRate;			/* Baud Rate */
	uint_16 MaxPktSize;
}SERIAL_INIT, *PSERIAL_INIT;

/* Serial Controller Register Structure */
typedef struct _SCI_REG
{
	volatile uint8_t UART_BDH;   /*!< UART Baud Rate Registers:High, offset: 0x0 */
	volatile uint8_t UART_BDL;   /*!< UART Baud Rate Registers: Low, offset: 0x1 */
	volatile uint8_t UART_C1;    /*!< UART Control Register 1, offset: 0x2 */
	volatile uint8_t UART_C2;    /*!< UART Control Register 2, offset: 0x3 */
	volatile uint8_t UART_S1;    /*!< UART Status Register 1, offset: 0x4 */
	volatile uint8_t UART_S2;    /*!< UART Status Register 2, offset: 0x5 */
	volatile uint8_t UART_C3;    /*!< UART Control Register 3, offset: 0x6 */
	volatile uint8_t UART_D;     /*!< UART Data Register, offset: 0x7 */
	volatile uint8_t UART_MA1;   /*!< UART Match Address Registers 1, offset: 0x8 */
	volatile uint8_t UART_MA2;   /*!< UART Match Address Registers 2, offset: 0x9 */
	volatile uint8_t UART_C4;    /*!< UART Control Register 4, offset: 0xA */	
}SCI_REG, *PSCI_REG;


/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
uint_8 SCI_Init (PSERIAL_INIT pSerialInit, SCI_CALLBACK pfnSciCallback);
uint_8 SCI_Read(uint_8 ControllerID, PBUFFER sBuffer);
uint_8 SCI_Write(uint_8 ControllerID, PBUFFER sBuffer);

#endif  /* _SERIAL_H */

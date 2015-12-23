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
 * @file serial_cfv2.h
 *
 * @author
 *
 * @version
 *
 * @date    June-17-2009
 *
 * @brief   This is a header file for Serial Driver
 *****************************************************************************/
#ifndef __SERIAL_CFV2_H
#define __SERIAL_CFV2_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "types.h"

/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/
#define BUS_CLK		80000

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

#define UART0_BASE_ADDRESS   0x40000200
#define UART1_BASE_ADDRESS   0x40000240
#define UART2_BASE_ADDRESS   0x40000280

/* Serial Driver Return Codes */
#define SERIAL_OK           0
#define SERIAL_TX_BSY       (uint_8)-1
#define SERIAL_RX_BSY       (uint_8)-2
#define SERIAL_INIT_FAILED  (uint_8)-3

#define MAX_SCI_INTERFACES   3

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
typedef union _SCI_REG
{
	struct
	{
	  volatile uint_8 UMR;             // UART mode register
      volatile uint_8 filler1[3];
      volatile uint_8 USR;             // UART status register
      volatile uint_8 filler2[3];
      volatile uint_8 filler4a;             
      volatile uint_8 filler4[3];
      volatile uint_8 URB;             // (Read) UART Receive Buffer 
      volatile uint_8 filler5[3];
      volatile uint_8 UIPCR;           // (Read) UART Input Port Change Register 
      volatile uint_8 filler6[3];
      volatile uint_8 UISR;            // (Read) UART Interrupt Status Register 
      volatile uint_8 filler7[3];
      volatile uint_8 filler9a;            
      volatile uint_8 filler9[3];
      volatile uint_8 filler10a;            
      volatile uint_8 filler10[3];
      volatile uint_8 filler11[0x0234 - 0x021C - 3 - 1];
      volatile uint_8 UIP;             // (Read) UART Input Port Register
      volatile uint_8 filler12[3];
      volatile uint_8 filler13a;            
      volatile uint_8 filler13[3];
      volatile uint_8 filler14a;       // (Write) UART Output Port Bit Reset Command Register
      volatile uint_8 filler14[3];
	}READ;
	struct
	{
	  volatile uint_8 UMR;             // UART mode register
	  volatile uint_8 filler1[3];
	  volatile uint_8 UCSR;             // UART status register
	  volatile uint_8 filler2[3];
	  volatile uint_8 UCR;             // (Write) UART Command Register
	  volatile uint_8 filler4[3];
	  volatile uint_8 UTB;             //  (Write) UART Transmit Buffer
	  volatile uint_8 filler5[3];
	  volatile uint_8 UACR;            //  (Write) UART Auxiliary Control Register
	  volatile uint_8 filler6[3];
	  volatile uint_8 UIMR;            // (Write) UART Interrupt Mask Register
	  volatile uint_8 filler7[3];
	  volatile uint_8 UBG1;            // (Write) UART Baud Rate Generator Register 1
	  volatile uint_8 filler9[3];
	  volatile uint_8 UBG2;            // (Write) UART Baud Rate Generator Register 2
	  volatile uint_8 filler10[3];
	  volatile uint_8 filler11[0x0234 - 0x021C - 3 - 1];
	  volatile uint_8 filler12a;             
	  volatile uint_8 filler12[3];
	  volatile uint_8 UOP1;            // (Write) UART Output Port Bit Set Command Register 0
	  volatile uint_8 filler13[3];
	  volatile uint_8 UIP0;             // (Write) UART Output Port Bit Reset Command Register
	}WRITE;                   
}SCI_REG, *PSCI_REG;

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
uint_8 SCI_Init (PSERIAL_INIT pSerialInit, SCI_CALLBACK pfnSciCallback);
uint_8 SCI_Read(uint_8 ControllerID, PBUFFER sBuffer);
uint_8 SCI_Write(uint_8 ControllerID, PBUFFER sBuffer);
void __declspec(interrupt) UART0_ISR(void);
void __declspec(interrupt) UART1_ISR(void);
void __declspec(interrupt) UART2_ISR(void);

#endif  /* _SERIAL_CFV2_H */
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
 * @file serial_class.h
 *
 * @author
 *
 * @version
 *
 * @date    June-17-2009
 *
 * @brief   This is a header file for Serial Class Driver
 *****************************************************************************/
#ifndef _SERIAL_CLASS_H
#define _SERIAL_CLASS_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "derivative.h"
#include "types.h"
#ifdef LITTLE_ENDIAN
#include "serial_kinetis.h"
#elif (defined __MCF52xxx_H__)
#include "serial_cfv2.h"
#else
#include "serial.h"
#endif
/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/
#define SERIAL_SEND                 (1)
#define SERIAL_RECV                 (0)

#define INVALID_TIMER_INDEX         (0xFF)
#define MAX_XMIT_QUEUE_ELEMENTS     (4)

/* Serial Class Driver Return Codes */
#define SERIAL_CLASS_OK             0
#define SERIAL_CLASS_TX_FAILED      (uint_8)-1
#define SERIAL_CLASS_RX_FAILED      (uint_8)-2
#define SERIAL_ERROR                (uint_8)-3
#define SERIAL_BUSY                 (uint_8)-4    

/* Serial Class Driver Application Events */
#define SERIAL_APP_CONNECTED                   (0)
#define SERIAL_APP_ENUM_COMPLETE               (2)
#define SERIAL_APP_SEND_COMPLETE               (3)
#define SERIAL_APP_DATA_RECEIVED               (4)
#define SERIAL_APP_ERROR                       (5)
#define SERIAL_APP_GET_DATA_BUFF               (6)
#define SERIAL_APP_GET_TRANSFER_SIZE           (7)

/******************************************************************************
 * Types
 *****************************************************************************/
#pragma pack(1)
/* Serial class Transfer Size */
typedef struct _serial_class_xfer_size
{
    uint_8 direction;
    uint_8_ptr in_buff;
    uint_16 in_size;
    uint_16 transfer_size;
}SERIAL_CLASS_XFER_SIZE, *PTR_SERIAL_CLASS_XFER_SIZE;

/* Serial class Data Received */
typedef struct _serial_app_event_data_received
{
    uint_8 qos;             /* Qos of the data received */
    uint_8_ptr buffer_ptr;  /* Pointer to the data received */
    uint_16 size;   /* Size of the data received */
    uint_16 transfer_size;  /* Total transfer Size */
}SERIAL_APP_EVENT_DATA_RECEIVED, *PTR_SERIAL_APP_EVENT_DATA_RECEIVED;

/* Serial class Data Sent */
typedef struct _serial_app_event_send_complete
{
    uint_8 qos;             /* Qos of the data received */
    uint_8_ptr buffer_ptr;  /* Pointer to the data sent */
    uint_16 size;           /* Size of the data sent */
}SERIAL_APP_EVENT_SEND_COMPLETE, *PTR_SERIAL_APP_EVENT_SEND_COMPLETE;

/* callback function pointer structure for the upper layer to handle events */
typedef void(_CODE_PTR_ SERIAL_CLASS_CALLBACK)(uint_8, uint_8, void*);

/* Serial class Receive Buffer  */
typedef struct _SERIAL_CLASS_RX_BUFF
{
    uint_16 in_size;
    uint_8_ptr in_buff;  /* Pointer to input Buffer */
    uint_16 transfer_size;
}SERIAL_CLASS_RX_BUFF, *PTR_SERIAL_CLASS_RX_BUFF;

/* Serial Class Buffer */
typedef struct _SERIAL_CLASS_BUFFER
{
    uint_8_ptr pBuffer;         /* Buffer Pointer */
    uint_16 size;               /* Size for current transaction */
    uint_16 transfer_size;      /* Total Size */
    uint_16 cur_offset;         /* Current Offset */
}SERIAL_CLASS_BUFFER, *PTR_SERIAL_CLASS_BUFFER;

/* Serial Transmit Queue Structure */
typedef struct _serial_xmit_queue
{
    uint_8 producer;
    uint_8 consumer;
    SERIAL_CLASS_BUFFER class_buffer[MAX_XMIT_QUEUE_ELEMENTS];
}SERIAL_XMIT_QUEUE, *PTR_SERIAL_XMIT_QUEUE;

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
/* This function intializes the Serial Class */
uint_8 SCI_Class_Init(
    PSERIAL_INIT pSerialInit,
    SERIAL_CLASS_CALLBACK serial_class_callback);
/* This function sends data over Serial transport */
uint_8 SCI_Class_Write (
    uint_8           controller_ID,
    uint_8_ptr       app_buff,
    uint_16          size);
/* This function reads Serial Protocol data */
uint_8 SCI_Class_Read (
    uint_8           controller_ID,
    uint_8_ptr       app_buff,
    uint_16          size);
    
#endif
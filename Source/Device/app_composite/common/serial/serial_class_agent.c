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
 * @file serial_class_agent.c
 *
 * @author
 *
 * @version
 *
 * @date May-28-2009
 *
 * @brief The file contains Serial Agent Protocol Implementation
 *
 *****************************************************************************/
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "derivative.h"         /* include peripheral declarations */
#include "types.h"              /* User Defined Data Types */
#include "user_config.h"        /* User Defined Configuration Settings */
#include "serial_protocol.h"
#include "serial_class.h"
#ifdef LITTLE_ENDIAN
#include "serial_kinetis.h"
#elif (defined __MCF52xxx_H__)
#include "serial_cfv2.h"
#else
#include "serial.h"
#endif
#include <string.h>


/******************************************************************************
 * Macro's
 *****************************************************************************/
#ifndef MIN
#define MIN(a,b)    (uint_16)(((a) > (b)) ? (b) : (a))
#endif
/******************************************************************************
 * Local Types
 *****************************************************************************/
/* Serial Communication Buffer */
typedef struct _SERIAL_COMM
{
    BUFFER Buff;
}SERIAL_COMM, *PSERIAL_COMM;

/* Serial Device States */
typedef enum 
{
    SERIAL_DISCONNECTED,
    SERIAL_CONNECTED,
    SERIAL_BUFF_NEG,
    SERIAL_READY,
    SERIAL_WAIT_TRANSMIT,
    SERIAL_TRANSMIT,
    SERIAL_WAIT_RECEIVE,
    SERIAL_RECEIVE
}SERIAL_FSM;

/* Serial class structure */
typedef struct _SERIAL_CLASS
{
    uint_16 MaxPktSize;
    SERIAL_CLASS_CALLBACK SerialClassCallback;
    uint_8 Controller_ID;
    SERIAL_FSM  SerialFsm;
    SERIAL_COMM TxComm, RxComm;
}SERIAL_CLASS, *PSERIAL_CLASS;

/******************************************************************************
 * Local Variables
 *****************************************************************************/
static SERIAL_CLASS g_Serial;
static SERIAL_XMIT_QUEUE g_send_queue;
static SERIAL_CLASS_BUFFER g_recv_buffer;
static uint_8 DummyRecvBuffer[MAX_SERIAL_RECV_BUFFER];
static uint_8 DummyTxBuffer[PACKET_HEADER_SIZE];
static uint_8 g_read_enable = FALSE;

/*****************************************************************************
 * Global Functions Prototypes - None
 *****************************************************************************/

/******************************************************************************
 * Local Function Prototypes
 *****************************************************************************/
static uint_16 FindMarker(
    uint_8_ptr pBuffer, 
    uint_16 size
);
static void PrepareRecvPcktHeader(
    uint_16 size
);
static void SendPktHeader(
    uint_8 header_type,
    uint_16 length
);

static void SCI_Class_Callback(
    uint_8 controller_ID, 
    uint_8 event_type, 
    PBUFFER psBuffer);
static void HandleSerialClassDisConnected(
    uint_8 controller_ID, 
    uint_8 event_type, 
    PBUFFER psBuffer
);
static void HandleSerialClassConnected(
    uint_8 controller_ID, 
    uint_8 event_type, 
    PBUFFER psBuffer
);
static void HandleSerialClassWaitRecv(
    uint_8 controller_ID, 
    uint_8 event_type, 
    PBUFFER psBuffer
);
static void HandleSerialClassReceive(
    uint_8 controller_ID, 
    uint_8 event_type, 
    PBUFFER psBuffer
);
static void HandleSerialClassReady(
    uint_8 controller_ID, 
    uint_8 event_type, 
    PBUFFER psBuffer
);
static void HandleSerialClassTransmit(
    uint_8 controller_ID, 
    uint_8 event_type, 
    PBUFFER psBuffer
);
static void HandleSerialClassWaitTransmit(
    uint_8 controller_ID, 
    uint_8 event_type, 
    PBUFFER psBuffer
);

/******************************************************************************
 * Local Function
 *****************************************************************************/
/**************************************************************************//*!
 *
 * @name  SCI_Class_Callback
 *
 * @brief This callback function is called from SCI driver.
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This callback function is invoked from Serial Driver when Data is 
 * transmitted or received or their is any error is reported on Serial Bus
 *****************************************************************************/
static void SCI_Class_Callback(
    uint_8 controller_ID,       /* [IN] Controller ID */
    uint_8 event_type,          /* [IN] Serial Bus Event ID */
    PBUFFER psBuffer            /* [IN] Pointer to Buffer Structure */
 )
{
    /* Call the respective callback function according to the device state */
    switch(g_Serial.SerialFsm)
    {
        case SERIAL_DISCONNECTED:
        {            
            HandleSerialClassDisConnected(controller_ID, event_type, psBuffer);
            break;
        }
        case SERIAL_CONNECTED:
        {
            HandleSerialClassConnected(controller_ID, event_type, psBuffer);
            break;
        }
        case SERIAL_WAIT_RECEIVE:
        {
            HandleSerialClassWaitRecv(controller_ID, event_type, psBuffer);
            break;
        }
        case SERIAL_RECEIVE:
        {
            HandleSerialClassReceive(controller_ID, event_type, psBuffer);
            break;
        }
        case SERIAL_READY:
        {
            HandleSerialClassReady(controller_ID, event_type, psBuffer);
            break;
        }
        case SERIAL_TRANSMIT:
        {
            HandleSerialClassTransmit(controller_ID, event_type, psBuffer);
            break;            
        }
        case SERIAL_WAIT_TRANSMIT:
        {
            HandleSerialClassWaitTransmit(controller_ID, event_type, psBuffer);
            break;            
        }
    }
    return;
}

/**************************************************************************//*!
 *
 * @name  HandleSerialClassWaitTransmit
 *
 * @brief This function handles Serial Event when Agent is in WAIT_TRANSMIT 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Agent is in WAIT_TRANSMIT State
 *****************************************************************************/
static void HandleSerialClassWaitTransmit(
    uint_8 controller_ID,       /* [IN] Controller ID */
    uint_8 event_type,          /* [IN] Serial Bus Event ID */
    PBUFFER psBuffer            /* [IN] Pointer to Buffer Structure */
)
{
    PSERIAL_COMM pTxComm = &g_Serial.TxComm;
    PTR_SERIAL_CLASS_BUFFER pserial_class_buffer;
    uint_8 queue;
    UNUSED(psBuffer)
    
    switch(event_type)
    {
        case SCI_TRANSMIT_COMPLETE:
        {
            /* Agent device has acknowledged the receive call from the Master.
               Start Sending data */
            queue = (uint_8)(g_send_queue.consumer % MAX_XMIT_QUEUE_ELEMENTS);
            pserial_class_buffer = &g_send_queue.class_buffer[queue];
            g_Serial.SerialFsm = SERIAL_TRANSMIT;
            if(pserial_class_buffer->transfer_size == 0)
            {
                SERIAL_CLASS_XFER_SIZE xfer_size;
                xfer_size.direction = SERIAL_SEND;
                xfer_size.in_buff = pserial_class_buffer->pBuffer;
                xfer_size.in_size = pserial_class_buffer->size;
                xfer_size.transfer_size = 0;
                
                /* Get the total size of the Transfer */
                g_Serial.SerialClassCallback(controller_ID, 
                    SERIAL_APP_GET_TRANSFER_SIZE, &xfer_size);
                pserial_class_buffer->transfer_size = xfer_size.transfer_size;
                pserial_class_buffer->cur_offset = 0;
            }
            if(pserial_class_buffer->transfer_size > 
                pserial_class_buffer->cur_offset)
            {
                /* Packets left to be sent */
                uint_16 bytesleft;
                pTxComm->Buff.pBuffer = &pserial_class_buffer->pBuffer
                    [pserial_class_buffer->cur_offset];
                    
                bytesleft = (uint_16)(pserial_class_buffer->transfer_size - 
                    pserial_class_buffer->cur_offset);
                pTxComm->Buff.Length = MIN(bytesleft, g_Serial.MaxPktSize);
                pTxComm->Buff.InUse = FALSE;
                pTxComm->Buff.CurOffSet = 0;
                
                (void)SCI_Write(g_Serial.Controller_ID, &pTxComm->Buff);
            }
        }
        break;        
        case SCI_RECEIVE_COMPLETE:
        {
            break;
        }
        case SCI_RECEIVE_OVERRUN:
        case SCI_RECEIVE_NOISE_DETECTED:
        case SCI_RECEIVE_FRAME_ERROR:
        case SCI_RECEIVE_PARITY_ERROR:
        {
        }
    }
}

/**************************************************************************//*!
 *
 * @name  HandleSerialClassTransmit
 *
 * @brief This function handles Serial Event when Agent is in TRANSMIT 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Agent is in TRANSMIT State
 *****************************************************************************/
static void HandleSerialClassTransmit(
    uint_8 controller_ID,       /* [IN] Controller ID */
    uint_8 event_type,          /* [IN] Serial Bus Event ID */
    PBUFFER psBuffer            /* [IN] Pointer to Buffer Structure */
)
{
    PTR_SERIAL_CLASS_BUFFER pserial_class_buffer;
    uint_8 queue;
    queue = (uint_8)(g_send_queue.consumer % MAX_XMIT_QUEUE_ELEMENTS);
    
    pserial_class_buffer = &g_send_queue.class_buffer[queue];

    switch(event_type)
    {
        case SCI_TRANSMIT_COMPLETE:
        {
            /* Packet Transmission complete */
            pserial_class_buffer->cur_offset += psBuffer->CurOffSet;
            /* Prepare to Receive ACK */
            PrepareRecvPcktHeader(PACKET_HEADER_SIZE);
        }
        break;
        case SCI_RECEIVE_COMPLETE:
        {            
            /* Data Sent acknowledged by the master */
            /* Change State to Ready */
            g_Serial.SerialFsm = SERIAL_READY;

            /* Disable Packet Decoding till we are done with Packet */
            g_read_enable = FALSE;

            /* Prepare to Receive */
            PrepareRecvPcktHeader(PACKET_HEADER_SIZE);

            if(pserial_class_buffer->transfer_size == 
                pserial_class_buffer->cur_offset)
            {
                /* Packet Send Complete */
                SERIAL_APP_EVENT_SEND_COMPLETE tx_buff;
                
                /* Increment Consumer */
                g_send_queue.consumer++;              

                tx_buff.buffer_ptr = pserial_class_buffer->pBuffer;
                tx_buff.size = pserial_class_buffer->transfer_size;
                
                (void)memset((void*)pserial_class_buffer, 0x00, 
                    sizeof(SERIAL_CLASS_BUFFER));

                /* Send Application Callback */
                g_Serial.SerialClassCallback(controller_ID, 
                    SERIAL_APP_SEND_COMPLETE, &tx_buff);

            }

            /* Enable Packet Decoding till we are done with Packet */
            g_read_enable = TRUE;
        }
        break;
    }
}

/**************************************************************************//*!
 *
 * @name  HandleSerialClassReady
 *
 * @brief This function handles Serial Event when Agent is in READY 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Agent is in READY State
 *****************************************************************************/
static void HandleSerialClassReady(
    uint_8 controller_ID,       /* [IN] Controller ID */
    uint_8 event_type,          /* [IN] Serial Bus Event ID */
    PBUFFER psBuffer            /* [IN] Pointer to Buffer Structure */
)
{
    static PBUFFER  psBuffer_tmp;
    psBuffer_tmp = psBuffer;
    switch(event_type)
    {
        case SCI_TRANSMIT_COMPLETE:
        {
            break;
        }
        case SCI_RECEIVE_COMPLETE:
        {
            if(g_read_enable == FALSE)
            {
                /* Disable Packet Decoding until there is a 
                Read call from app */
                PrepareRecvPcktHeader(PACKET_HEADER_SIZE);
                break;
            }
            
            switch(psBuffer->pBuffer[0])
            {
                case MASTER_SEND_HEADER:
                {

                    /* Master send header received */
                    PSERIAL_COMM pRxComm = &g_Serial.RxComm;
                    uint_16 recv_pkt_size = 
                        *(uint_16_ptr)(&psBuffer->pBuffer[1]);
                    
                    /* Change State to Wait Receive */
                    g_Serial.SerialFsm = SERIAL_WAIT_RECEIVE;
                    
                    /* We Prepare to receive data here */
                    pRxComm->Buff.pBuffer = (g_recv_buffer.pBuffer == NULL) ?
                        DummyRecvBuffer : g_recv_buffer.pBuffer;
                    pRxComm->Buff.Length = ieee_htons(recv_pkt_size);
                    pRxComm->Buff.CurOffSet = 0;
                    
                    /* Setup Data receive */
                    (void)SCI_Read(controller_ID, &pRxComm->Buff);

					/* Send wait send response */
                    SendPktHeader(MASTER_SEND_ACK, 0);
                    break;
                }
                case MASTER_RECV_HEADER:
                {
                    /* Master receive header received */
                    uint_8 producer = g_send_queue.producer;
                    uint_8 consumer = g_send_queue.consumer;
                    uint_8 queue;
                    PTR_SERIAL_CLASS_BUFFER pserial_class_buffer;
                    
                    /* Queue not full */                    
                    if(producer != consumer)
                    {
                        uint_16 send_pkt_size;
                        
                        queue = (uint_8) 
                            (g_send_queue.consumer % MAX_XMIT_QUEUE_ELEMENTS);                        
                        pserial_class_buffer = 
                            &g_send_queue.class_buffer[queue];
                        
                        send_pkt_size = 
                            (uint_16)(pserial_class_buffer->transfer_size - 
                            pserial_class_buffer->cur_offset);
                        /* Change State to Transmit */
                        g_Serial.SerialFsm = SERIAL_WAIT_TRANSMIT;
                        /* Send wait recv response */
                        SendPktHeader(MASTER_RECV_ACK, 
                            MIN(send_pkt_size, g_Serial.MaxPktSize));
                    }
                    else
                    {
                        /* Prepare to receive next packet header */
                        PrepareRecvPcktHeader(PACKET_HEADER_SIZE);
                    }
                    break;
                }
                default:
                {
                    /* We have lost sync with Manager here */
                    /* Find our Marker */
                    if(psBuffer->CurOffSet < PACKET_HEADER_SIZE)
                    {
                        /* Prepare to receive header */
                        PrepareRecvPcktHeader(PACKET_HEADER_SIZE);
                    }
                    else
                    {
                        /* Find the error position and correct it */
                        uint_16 pos = FindMarker(psBuffer->pBuffer, 
                            psBuffer->CurOffSet);
                        PrepareRecvPcktHeader(pos);
                    }
                    break;
                }
            }
            break;
        }
        case SCI_RECEIVE_OVERRUN:
        case SCI_RECEIVE_NOISE_DETECTED:
        case SCI_RECEIVE_FRAME_ERROR:
        case SCI_RECEIVE_PARITY_ERROR:
        {
        }
    }
}

/**************************************************************************//*!
 *
 * @name  HandleSerialClassReceive
 *
 * @brief This function handles Serial Event when Agent is in RECEIVE 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Agent is in RECEIVE State
 *****************************************************************************/
static void HandleSerialClassReceive(
    uint_8 controller_ID,       /* [IN] Controller ID */
    uint_8 event_type,          /* [IN] Serial Bus Event ID */
    PBUFFER psBuffer            /* [IN] Pointer to Buffer Structure */
)
{
    UNUSED(psBuffer)
    switch(event_type)
    {
        case SCI_TRANSMIT_COMPLETE:
        {
            /* ACK of the Data received sent */
            PSERIAL_COMM pRxComm = &g_Serial.RxComm;
            BUFFER RecvBuffer;
            PBUFFER pRecvBuffer;
            
            (void)memcpy((void*)&RecvBuffer, &pRxComm->Buff, sizeof(BUFFER));
            pRecvBuffer = &RecvBuffer;

            /* Disable Packet Decoding till we are done */
            g_read_enable = FALSE;
                
            /* Prepare to Receive next Packet */
            PrepareRecvPcktHeader(PACKET_HEADER_SIZE);

            /* Change State to READY */
            g_Serial.SerialFsm = SERIAL_READY;
                       
            if(g_recv_buffer.transfer_size == 0)
            {
                SERIAL_CLASS_XFER_SIZE xfer_size;
                xfer_size.direction = SERIAL_RECV;
                xfer_size.in_buff = pRecvBuffer->pBuffer;
                xfer_size.in_size = pRecvBuffer->CurOffSet;
                xfer_size.transfer_size = 0;
                /* Get the total transfer size */
                g_Serial.SerialClassCallback(controller_ID, 
                    SERIAL_APP_GET_TRANSFER_SIZE, &xfer_size);
                g_recv_buffer.transfer_size = xfer_size.transfer_size;
                g_recv_buffer.cur_offset = 0;
            }
            
            g_recv_buffer.cur_offset += pRecvBuffer->CurOffSet;
            
            if(g_recv_buffer.transfer_size > g_recv_buffer.cur_offset)
            {
                SERIAL_CLASS_RX_BUFF rx_buff;
                
                rx_buff.in_size = pRecvBuffer->CurOffSet;
                rx_buff.in_buff = pRecvBuffer->pBuffer;
                rx_buff.transfer_size = g_recv_buffer.transfer_size;
                /* Send Application Callback */
                g_Serial.SerialClassCallback(controller_ID, 
                    SERIAL_APP_GET_DATA_BUFF, &rx_buff); 
            }
            else
            {
                SERIAL_APP_EVENT_DATA_RECEIVED rx_buff;

                rx_buff.buffer_ptr = pRecvBuffer->pBuffer;
                rx_buff.size = pRecvBuffer->CurOffSet;
                rx_buff.transfer_size = g_recv_buffer.transfer_size;
                (void)memset((void*)&g_recv_buffer, 0x00, sizeof(g_recv_buffer));
                
                /* Send Application Callback */
                g_Serial.SerialClassCallback(controller_ID, 
                    SERIAL_APP_DATA_RECEIVED, &rx_buff); 
            }
            break;
        }
        case SCI_RECEIVE_COMPLETE:
        {
            /* Send ACK */
            SendPktHeader(PACKET_ACK, 0);
            break;
        }
        case SCI_RECEIVE_OVERRUN:
        case SCI_RECEIVE_NOISE_DETECTED:
        case SCI_RECEIVE_FRAME_ERROR:
        case SCI_RECEIVE_PARITY_ERROR:
        {
            g_Serial.SerialClassCallback(controller_ID, 
                    SERIAL_APP_ERROR, (void*)event_type);
            break;
        }
    }
}

/**************************************************************************//*!
 *
 * @name  HandleSerialClassWaitRecv
 *
 * @brief This function handles Serial Event when Agent is in WAIT_RECEIVE 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Agent is in WAIT_RECEIVE State
 *****************************************************************************/
static void HandleSerialClassWaitRecv(
    uint_8 controller_ID,       /* [IN] Controller ID */
    uint_8 event_type,          /* [IN] Serial Bus Event ID */
    PBUFFER psBuffer            /* [IN] Pointer to Buffer Structure */
)
{
    UNUSED(controller_ID);
    UNUSED(psBuffer)
    switch(event_type)
    {
        case SCI_TRANSMIT_COMPLETE:
        {
            /* ACK to master send header sent */
            g_Serial.SerialFsm = SERIAL_RECEIVE;
            /* Preparation to receive data is done in HandleSerialClassReady */
            break;
        }
        case SCI_RECEIVE_COMPLETE:
        {
            break;
        }
        case SCI_RECEIVE_OVERRUN:
        case SCI_RECEIVE_NOISE_DETECTED:
        case SCI_RECEIVE_FRAME_ERROR:
        case SCI_RECEIVE_PARITY_ERROR:
        {
            break;
        }
        
    }
}

/**************************************************************************//*!
 *
 * @name  HandleSerialClassDisConnected
 *
 * @brief This function handles Serial Event when Agent is in DISCONNECTED 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Agent is in DISCONNECTED State
 *****************************************************************************/
static void HandleSerialClassDisConnected(
    uint_8 controller_ID,       /* [IN] Controller ID */
    uint_8 event_type,          /* [IN] Serial Bus Event ID */
    PBUFFER psBuffer            /* [IN] Pointer to Buffer Structure */
)
{
    UNUSED(controller_ID);
    UNUSED(psBuffer);
    if(event_type == SCI_CONNECT)
    {
        g_Serial.SerialFsm = SERIAL_CONNECTED;   
        PrepareRecvPcktHeader(PACKET_HEADER_SIZE);   
    }
    return;
}

/**************************************************************************//*!
 *
 * @name  HandleSerialClassConnected
 *
 * @brief This function handles Serial Event when Agent is in CONNECTED 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Agent is in CONNECTED State
 *****************************************************************************/
static void HandleSerialClassConnected(
    uint_8 controller_ID,       /* [IN] Controller ID */
    uint_8 event_type,          /* [IN] Serial Bus Event ID */
    PBUFFER psBuffer            /* [IN] Pointer to Buffer Structure */
)
{
    switch(event_type)
    {
        case SCI_RECEIVE_COMPLETE:
        {
            if(psBuffer->pBuffer[0] == BUFFER_NEG_HEADER)
            {
                uint_16 pktsize = (*(uint_16*)&psBuffer->pBuffer[1]);
                if(pktsize < g_Serial.MaxPktSize)
                {
                    /* Adjust MAX Packet Size */
                    g_Serial.MaxPktSize = pktsize;
                }
                /* 
                    Serial Slave to send response to 
                    Buffer Negotiation 
                */
                SendPktHeader(BUFFER_NEG_HEADER, g_Serial.MaxPktSize);
            }
            else
            {
                /* Bad Packet Received */
                if(psBuffer->CurOffSet < PACKET_HEADER_SIZE)
                {
                    PrepareRecvPcktHeader(PACKET_HEADER_SIZE);
                }
                else
                {
                    uint_16 pos = FindMarker(psBuffer->pBuffer, 
                        psBuffer->CurOffSet);
                    PrepareRecvPcktHeader(pos);
                }
            }
            break;
        }
        case SCI_TRANSMIT_COMPLETE:
        {
            g_read_enable = TRUE;
            PrepareRecvPcktHeader(PACKET_HEADER_SIZE);
            g_Serial.SerialFsm = SERIAL_READY;
            g_Serial.SerialClassCallback(controller_ID, 
                SERIAL_APP_ENUM_COMPLETE, NULL);
        }
        case SCI_RECEIVE_OVERRUN:
        case SCI_RECEIVE_NOISE_DETECTED:
        case SCI_RECEIVE_FRAME_ERROR:
        case SCI_RECEIVE_PARITY_ERROR:
        {
            /* Error Case Handling */
        }
        break;
    }
    return;
}

/**************************************************************************//*!
 *
 * @name  FindMarker
 *
 * @brief This function finds Serial Protocol Header Markers in a buffer
 *
 * @param pBuffer       :   Buffer Pointer
 * @param size          :   Size of Buffer
 *
 * @return Position of Marker if successful otherwise PACKET_HEADER_SIZE
 ******************************************************************************
 * This function finds Serial Protocol Header Markers in a buffer
 *****************************************************************************/
static uint_16 FindMarker(
    uint_8_ptr pBuffer,     /* [IN] Buffer Pointer */
    uint_16 size            /* [IN] Size of Buffer */
)
{
    uint_16 pos = 0;
    while(pos < size)
    {
        switch(pBuffer[pos])
        {
            case BUFFER_NEG_HEADER:
            case MASTER_SEND_HEADER:
            case MASTER_SEND_ACK:
            case MASTER_RECV_HEADER:
            case MASTER_RECV_ACK:
            case PACKET_ACK:
                return pos;
                break;
            default:
                pos++;
                break;
        }
    }
    /* return the position of the first byte of the header */
    return (uint_8)PACKET_HEADER_SIZE;
}

/**************************************************************************//*!
 *
 * @name  PrepareRecvPcktHeader
 *
 * @brief This function calls Serial Driver to receive Packet Header
 *
 * @param size	:	Number of Bytes to be received
 *
 * @return None
 ******************************************************************************
 * This function calls Serial Driver to receive Packet Header
 *****************************************************************************/
static void PrepareRecvPcktHeader(
    uint_16 size    /* [IN] Number of Bytes to be received */
)
{
    PSERIAL_COMM pRxComm = &g_Serial.RxComm;
    pRxComm->Buff.pBuffer = DummyRecvBuffer;
    pRxComm->Buff.CurOffSet = 0;
    pRxComm->Buff.Length = size;  
    (void)SCI_Read(g_Serial.Controller_ID, &pRxComm->Buff);
}

/**************************************************************************//*!
 *
 * @name  SendPktHeader
 *
 * @brief This function transmits Serial Protocol Packet Header
 *
 * @param header_type	:	Protocol Packet Header
 * @param length	    :	Length of Data to Follow
 *
 * @return None
 ******************************************************************************
 * This function transmits Serial Protocol Packet Header
 *****************************************************************************/
static void SendPktHeader(
    uint_8 header_type,     /* [IN] Protocol Packet Header */
    uint_16 length          /* [IN] Length of Data to Follow */
)
{
    PSERIAL_COMM pTxComm = &g_Serial.TxComm;

    DummyTxBuffer[0] = header_type;
    *(uint_16*)&DummyTxBuffer[1] = ieee_htons(length);

    pTxComm->Buff.pBuffer = DummyTxBuffer;
    pTxComm->Buff.CurOffSet = 0;         
    pTxComm->Buff.Length = 3;
    pTxComm->Buff.InUse = FALSE;

    (void)SCI_Write(g_Serial.Controller_ID, &pTxComm->Buff);
    return;
}

/******************************************************************************
 * Global Function
 *****************************************************************************/
/**************************************************************************//*!
 *
 * @name  SCI_Class_Read
 *
 * @brief This function reads Serial Protocol data
 *
 * @param controller_ID	:	Serial Controller ID
 * @param app_buff	    :	Application Receive Buffer
 * @param size	        :	Size of the packet
 *
 * @return SERIAL_OK    :   if Successful
 *         SERIAL_BUSY  :   Unsuccessful
 ******************************************************************************
 * This function reads Serial Protocol data
 *****************************************************************************/
uint_8 SCI_Class_Read (
    uint_8           controller_ID, /* [IN] Serial Controller ID */
    uint_8_ptr       app_buff,      /* [OUT] Application Receive Buffer */
    uint_16          size)          /* [IN] Size of the packet */
{
    UNUSED(controller_ID);
    if(g_recv_buffer.pBuffer != NULL)
        return SERIAL_BUSY;
    g_recv_buffer.pBuffer = app_buff;
    g_recv_buffer.size = size;
    g_read_enable = TRUE;

    return SERIAL_OK;    
}

/**************************************************************************//*!
 *
 * @name  SCI_Class_Write
 *
 * @brief This function sends data over Serial transport
 *
 * @param controller_ID	:	Serial Controller ID
 * @param app_buff	    :	Application Send Buffer
 * @param size	        :	Size of the packet
 *
 * @return SERIAL_OK    :   if Successful
 *         SERIAL_BUSY  :   Unsuccessful
 ******************************************************************************
 * This function sends data over Serial transport
 *****************************************************************************/
uint_8 SCI_Class_Write (
    uint_8           controller_ID, /* [IN] Serial Controller ID */
    uint_8_ptr       app_buff,      /* [IN] Application Send Buffer */
    uint_16          size)          /* [IN] Size of the packet */
{
    SERIAL_CLASS_XFER_SIZE xfer_size;
    uint_8 producer, consumer, queue;
    PTR_SERIAL_CLASS_BUFFER pserial_class_buffer;
    
    UNUSED(controller_ID);
    
    producer = g_send_queue.producer;
    consumer = g_send_queue.consumer;
    if((producer - consumer) >= MAX_XMIT_QUEUE_ELEMENTS)
    {
        /* Queue Full */
        return SERIAL_BUSY;
    }
    
    queue = (uint_8)(producer % MAX_XMIT_QUEUE_ELEMENTS);
    pserial_class_buffer = &g_send_queue.class_buffer[queue];
    
    /* Increment producer */
    g_send_queue.producer++;

    (void)memset((void*)pserial_class_buffer, 0x00, sizeof(SERIAL_CLASS_BUFFER));
    /* Add entry to queue */
    pserial_class_buffer->pBuffer = app_buff;
    pserial_class_buffer->size = size;

    xfer_size.direction = SERIAL_SEND;
    xfer_size.in_buff = pserial_class_buffer->pBuffer;
    xfer_size.in_size = pserial_class_buffer->size;
    xfer_size.transfer_size = 0;
    /* Get the size of the whole tranfer */
    g_Serial.SerialClassCallback(controller_ID, 
        SERIAL_APP_GET_TRANSFER_SIZE, &xfer_size);
    pserial_class_buffer->transfer_size = xfer_size.transfer_size;
    pserial_class_buffer->cur_offset = 0;
        
    return SERIAL_OK;   
}

/**************************************************************************//*!
 *
 * @name  SCI_Class_Init
 *
 * @brief This function intializes the Serial Class
 *
 * @param pSerialInit	        :	Serial Class Initialization Structure
 * @param serial_class_callback	:	Serial Class Callback function
 *
 * @return size of AVA_Type Data Structure
 ******************************************************************************
 * This function intializes the Serial Class
 *****************************************************************************/
uint_8 SCI_Class_Init(
    PSERIAL_INIT pSerialInit,                   /* [IN] Serial Class 
                                                   Initialization Structure */
    SERIAL_CLASS_CALLBACK serial_class_callback /* [IN] Serial Class Callback 
                                                   function */
)
{
    g_Serial.SerialClassCallback = serial_class_callback;
    g_Serial.Controller_ID = pSerialInit->ControllerId;
    g_Serial.MaxPktSize = pSerialInit->MaxPktSize;
    g_Serial.SerialFsm = SERIAL_DISCONNECTED;
    
    (void)memset((void*)&g_send_queue, 0x00, sizeof(g_send_queue));
    (void)memset((void*)&g_recv_buffer, 0x00, sizeof(g_recv_buffer));
    (void)SCI_Init(pSerialInit, SCI_Class_Callback);
    
    return SERIAL_CLASS_OK;
}

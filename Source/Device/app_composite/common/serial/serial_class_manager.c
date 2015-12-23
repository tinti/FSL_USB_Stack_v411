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
 * @file serial_class_manager.c
 *
 * @author
 *
 * @version
 *
 * @date May-28-2009
 *
 * @brief The file contains Serial Manager Protocol Implementation
 *
 *****************************************************************************/
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "hidef.h"
#include "derivative.h" /* include peripheral declarations */
#include "types.h"      /* User Defined Data Types */
#include <string.h>
#include "user_config.h"
#include "serial_protocol.h"
#include "serial_class.h"
#ifdef LITTLE_ENDIAN
#include "serial_kinetis.h"
#include "RealTimerCounter.h"
#elif defined __MCF52xxx_H__
#include "serial_cfv2.h"
#include "RealTimerCounter_cfv2.h"
#else
#include "serial.h"
#include "RealTimerCounter.h"
#endif

/******************************************************************************
 * Macro's
 *****************************************************************************/
#ifndef MIN
#define MIN(a,b)    (uint_16)(((a) > (b)) ? (b) : (a))
#endif

/******************************************************************************
 * Local Types
 *****************************************************************************/
typedef struct _SERIAL_COMM
{
    uint_16 Timeout;
    uint_8 TimerIndex;
    uint_8 qos;
    BUFFER Buff;
}SERIAL_COMM, *PSERIAL_COMM;

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
uint_8 g_SendTimerIndex, g_RecvTimerIndex;
SERIAL_CLASS g_Serial;
BUFFER g_serial_class_recv;
static SERIAL_CLASS_BUFFER g_send_buffer;
static SERIAL_CLASS_BUFFER g_recv_buffer;
static uint_8 DummyRecvBuffer[MAX_SERIAL_RECV_BUFFER];
static uint_8 DummyTxBuffer[PACKET_HEADER_SIZE];

/*****************************************************************************
 * Global Functions Prototypes - None
 *****************************************************************************/

/******************************************************************************
 * Local Function Prototypes
 *****************************************************************************/
#ifdef TIMER_CALLBACK_ARG 
static void SCI_Send_Timer_Callback(
    void* arg
);
static void SCI_Send_Slave_Recv(
    void* arg
);
#else
static void SCI_Send_Timer_Callback(void);
static void SCI_Send_Slave_Recv(void);
#endif

static void PrepareRecvPcktHeader(void);

static void SendPktHeader(
    uint_8 header_type,
    uint_16 length
);
static void SCI_Class_Callback(
    uint_8 controller_ID, 
    uint_8 event_type, 
    PBUFFER psBuffer
);
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

static void SCI_Class_Send_Slave_Send_Poll(void);

/******************************************************************************
 * Local Function
 *****************************************************************************/

/**************************************************************************//*!
 *
 * @name  SCI_Class_Send_Slave_Send_Poll
 *
 * @brief This function polls the slave device to receive data.
 *
 * @param None
 *
 * @return None
 ******************************************************************************
 * This function polls the slave device to receive data. The slave device 
 * acknowledges this packet whenever it has data to send.
 *****************************************************************************/
static void SCI_Class_Send_Slave_Send_Poll(void)
{
    if((g_Serial.SerialFsm == SERIAL_READY) || 
        (g_Serial.SerialFsm == SERIAL_RECEIVE))
    {
        /* Change Serial Class State as Slave Wait Receive */
        g_Serial.SerialFsm = SERIAL_WAIT_RECEIVE; 
        
        /* Prepare to Receive Acknowledgement Packet from Slave */       
        PrepareRecvPcktHeader();
        
        /* Send Slave Recv Poll Packet */        
        SendPktHeader(MASTER_RECV_HEADER, 0);
    }
}
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
 * @brief This function handles Serial Event when Master is in WAIT_TRANSMIT 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Master is in WAIT_TRANSMIT State
 *****************************************************************************/
static void HandleSerialClassWaitTransmit(
    uint_8 controller_ID,       /* [IN] Controller ID */
    uint_8 event_type,          /* [IN] Serial Bus Event ID */
    PBUFFER psBuffer            /* [IN] Pointer to Buffer Structure */
)
{
    UNUSED(controller_ID);
    switch(event_type)
    {
        case SCI_TRANSMIT_COMPLETE:
        {
            /* Prepare to Receive Acknowledgement Packet from Slave */       
            PrepareRecvPcktHeader();
            break;
        }
        case SCI_RECEIVE_COMPLETE:
        {
            volatile uint_8_ptr temp = psBuffer->pBuffer;
            if(psBuffer->pBuffer[0] == MASTER_SEND_ACK)
            {
                PSERIAL_COMM pTxComm = &g_Serial.TxComm;

                g_Serial.SerialFsm = SERIAL_TRANSMIT;

                /* Initiate Transfer */
                if(g_send_buffer.transfer_size > g_send_buffer.cur_offset)
                {
                    uint_16 send_pkt_size = (uint_16)
                        (g_send_buffer.transfer_size - 
                        g_send_buffer.cur_offset);
                    pTxComm->Buff.pBuffer = &g_send_buffer.
                        pBuffer[g_send_buffer.cur_offset];
                    pTxComm->Buff.Length = MIN(send_pkt_size, 
                        g_Serial.MaxPktSize);
                    pTxComm->Buff.InUse = FALSE;
                    pTxComm->Buff.CurOffSet = 0;

                    SCI_Write(g_Serial.Controller_ID, &pTxComm->Buff);
                }
            }
            else
            {
                /* Prepare for Packet retransmit in case of bad packet */
                PrepareRecvPcktHeader();
            }
            break;
        }
        case SCI_RECEIVE_OVERRUN:
        case SCI_RECEIVE_NOISE_DETECTED:
        case SCI_RECEIVE_FRAME_ERROR:
        case SCI_RECEIVE_PARITY_ERROR:
        {
            /* Error Handling Code Here */
        }
    }
}
/**************************************************************************//*!
 *
 * @name  HandleSerialClassTransmit
 *
 * @brief This function handles Serial Event when Master is in TRANSMIT 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Master is in TRANSMIT State
 *****************************************************************************/
static void HandleSerialClassTransmit(
    uint_8 controller_ID,       /* [IN] Controller ID */
    uint_8 event_type,          /* [IN] Serial Bus Event ID */
    PBUFFER psBuffer            /* [IN] Pointer to Buffer Structure */
)
{
    UNUSED(controller_ID);
    switch(event_type)
    {
        case SCI_TRANSMIT_COMPLETE:
        {
            /* Increment number of bytes sent */
            g_send_buffer.cur_offset += psBuffer->CurOffSet;
            /* Prepare to receive Ack */
            PrepareRecvPcktHeader();
            break;
        }
        case SCI_RECEIVE_COMPLETE:
        {
            /* Send Next Packet */
            PSERIAL_COMM pTxComm = &g_Serial.TxComm;
            if(g_send_buffer.transfer_size > g_send_buffer.cur_offset)
            {
                uint_16 send_pkt_size = (uint_16)(g_send_buffer.transfer_size - 
                    g_send_buffer.cur_offset);

                g_Serial.SerialFsm = SERIAL_WAIT_TRANSMIT;
                SendPktHeader(MASTER_SEND_HEADER, MIN(send_pkt_size, 
                    g_Serial.MaxPktSize));   
            }
            else
            {
                /* Packet Sent Complete */
                SERIAL_APP_EVENT_SEND_COMPLETE tx_buff;
                
                tx_buff.buffer_ptr = g_send_buffer.pBuffer;
                tx_buff.size = g_send_buffer.transfer_size;
                
                memset((void*)&g_send_buffer, 0x00, sizeof(g_send_buffer));
                /* Change State to Ready */
                /* Send Application Callback */
                g_Serial.SerialClassCallback(controller_ID, 
                    SERIAL_APP_SEND_COMPLETE, &tx_buff); 

                /* Packet Transmit Complete */
                g_Serial.SerialFsm = SERIAL_READY;
                /* Initiate Serial Slave Send Poll */
                SCI_Class_Send_Slave_Send_Poll();
            }
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
 * @name  HandleSerialClassReady
 *
 * @brief This function handles Serial Event when Master is in READY 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Master is in READY State
 *****************************************************************************/
static void HandleSerialClassReady(
    uint_8 controller_ID,       /* [IN] Controller ID */
    uint_8 event_type,          /* [IN] Serial Bus Event ID */
    PBUFFER psBuffer            /* [IN] Pointer to Buffer Structure */
)
{
    UNUSED(controller_ID);
    UNUSED(psBuffer);
    switch(event_type)
    {
        case SCI_TRANSMIT_COMPLETE:
        {
            break;
        }
        case SCI_RECEIVE_COMPLETE:
        {
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
 * @brief This function handles Serial Event when Master is in RECEIVE 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Master is in RECEIVE State
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
            /* Received data acknowledged */
            PSERIAL_COMM pRxComm = &g_Serial.RxComm;
            PBUFFER pRecvBuffer = &pRxComm->Buff;
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
                /* Transfer receive complete */
                SERIAL_APP_EVENT_DATA_RECEIVED rx_buff;

                rx_buff.buffer_ptr = pRecvBuffer->pBuffer;
                rx_buff.size = pRecvBuffer->CurOffSet;
                rx_buff.transfer_size = g_recv_buffer.transfer_size; 
                memset((void*)&g_recv_buffer, 0x00, sizeof(g_recv_buffer));
                
                /* Change State to READY */
                g_Serial.SerialFsm = SERIAL_READY;
                
                /* Send Application Callback */
                g_Serial.SerialClassCallback(controller_ID, 
                    SERIAL_APP_DATA_RECEIVED, &rx_buff); 
            }
            break;
        }
        case SCI_RECEIVE_COMPLETE:
        {
            /* Send Receive ACK */
            SendPktHeader(PACKET_ACK, 0x00);
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
 * @brief This function handles Serial Event when Master is in WAIT_RECEIVE 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Master is in WAIT_RECEIVE State
 *****************************************************************************/
static void HandleSerialClassWaitRecv(
    uint_8 controller_ID,       /* [IN] Controller ID */
    uint_8 event_type,          /* [IN] Serial Bus Event ID */
    PBUFFER psBuffer            /* [IN] Pointer to Buffer Structure */
)
{
    UNUSED(controller_ID);
    switch(event_type)
    {
        case SCI_TRANSMIT_COMPLETE:
        {
            /* For Manager start timer to wait for Poll Ack */
            TIMER_OBJECT timer;
            timer.msCount = SERIAL_SLAVE_RECEIVE_POLL_TIMEOUT;
            timer.pfnTimerCallback = SCI_Send_Timer_Callback;
#ifdef TIMER_CALLBACK_ARG
            timer.arg = NULL;
#endif            
            g_SendTimerIndex = AddTimerQ(&timer);
            break;
        }
        case SCI_RECEIVE_COMPLETE:
        {
            volatile uint_8_ptr temp = psBuffer->pBuffer;
            if(psBuffer->pBuffer[0] == MASTER_RECV_ACK)
            {
                uint_16 recv_pkt_size = *(uint_16_ptr)&psBuffer->pBuffer[1];
                /* Manager Code */
                PSERIAL_COMM pRxComm = &g_Serial.RxComm;
                /* Close Timer */
                RemoveTimerQ(g_SendTimerIndex);
                /* Set Manager State as RECEIVE */
                g_Serial.SerialFsm = SERIAL_RECEIVE;
                
                /* Call SCI Read to read Data */
                pRxComm->Buff.pBuffer = (g_recv_buffer.pBuffer == NULL) ?
                    DummyRecvBuffer : g_recv_buffer.pBuffer;
                pRxComm->Buff.CurOffSet = 0;
                pRxComm->Buff.Length = ieee_htons(recv_pkt_size);
                pRxComm->Buff.InUse = FALSE;
                SCI_Read(controller_ID, &pRxComm->Buff);
            }
            else
            {
                
                /* Invalid Packet Received */
                PrepareRecvPcktHeader();
            }
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
 * @brief This function handles Serial Event when Master is in DISCONNECTED 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Master is in DISCONNECTED State
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
        PrepareRecvPcktHeader();   
        SendPktHeader(BUFFER_NEG_HEADER, g_Serial.MaxPktSize);
    }
    return;
}
/**************************************************************************//*!
 *
 * @name  HandleSerialClassConnected
 *
 * @brief This function handles Serial Event when Master is in CONNECTED 
 *        State
 *
 * @param controller_ID :   Serial Controller ID
 * @param event_type    :   Serial Bus Event ID
 * @param psBuffer      :   Pointer to Buffer Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Serial Event when Master is in CONNECTED State
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
                uint_16 pktsize = *(uint_16*)&psBuffer->pBuffer[1];
                if(pktsize < g_Serial.MaxPktSize)
                {
                    /* Adjust MAX Packet Size */
                    g_Serial.MaxPktSize = pktsize;
                }
                /* Remove Timer */
                RemoveTimerQ(g_SendTimerIndex);
                g_SendTimerIndex = INVALID_TIMER_INDEX;
                g_Serial.SerialFsm = SERIAL_READY;
                g_Serial.SerialClassCallback(controller_ID, 
                    SERIAL_APP_ENUM_COMPLETE, NULL);
                
                /* 
                    Manager in Ready State.
                    Start sending Slave Receive Poll Packets
                */
                SCI_Class_Send_Slave_Send_Poll();
            }
            else
            {
                /* Bad Packet Received */
                psBuffer->pBuffer = DummyRecvBuffer;
                psBuffer->CurOffSet = 0;
                psBuffer->Length = 3;  
                SCI_Read(controller_ID, psBuffer);
            }
            break;
        }
        case SCI_TRANSMIT_COMPLETE:
        {
            /* Manager Code */
            /* Start Buffer Negotiation Timer */
            TIMER_OBJECT timer;
            timer.msCount = SERIAL_BUFF_NEGOTIATION_TIMEOUT;
            timer.pfnTimerCallback = SCI_Send_Timer_Callback;
#ifdef TIMER_CALLBACK_ARG
            timer.arg = NULL;
#endif
            g_SendTimerIndex = AddTimerQ(&timer);                    
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
 * @name  PrepareRecvPcktHeader
 *
 * @brief This function calls Serial Driver to receive Packet Header
 *
 * @param None
 *
 * @return None
 ******************************************************************************
 * This function calls Serial Driver to receive Packet Header
 *****************************************************************************/
static void PrepareRecvPcktHeader(void)
{
    PSERIAL_COMM pRxComm = &g_Serial.RxComm;
    pRxComm->Buff.pBuffer = DummyRecvBuffer;
    pRxComm->Buff.CurOffSet = 0;
    pRxComm->Buff.Length = 3;  
    SCI_Read(g_Serial.Controller_ID, &pRxComm->Buff);
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

    SCI_Write(g_Serial.Controller_ID, &pTxComm->Buff);
}

#ifdef TIMER_CALLBACK_ARG
/**************************************************************************//*!
 *
 * @name  SCI_Send_Timer_Callback
 *
 * @brief This function is called whenever the Agent does not respond to the 
 *        buffer negotiation packet or the master receive header.
 *
 * @param arg	    :	Argument passed by Timer ISR (optional)
 *
 * @return None
 ******************************************************************************
 * This function is called whenever the Agent does not respond to the 
 * buffer negotiation packet or the master receive header.
 *****************************************************************************/
static void SCI_Send_Timer_Callback(
	void* arg
)
#else
static void SCI_Send_Timer_Callback(void)
#endif
{
#ifdef TIMER_CALLBACK_ARG
    UNUSED(arg);
#endif
    switch(g_Serial.SerialFsm)
    {
        case SERIAL_CONNECTED:
        {
            /* Start buffer negotiation */
            SendPktHeader(BUFFER_NEG_HEADER, g_Serial.MaxPktSize);
            break;
        }
        case SERIAL_WAIT_RECEIVE:
        {
            g_Serial.SerialFsm = SERIAL_READY;
            if(g_send_buffer.pBuffer == NULL)
            {
                
                /* if no pending write */
                TIMER_OBJECT timer;
                timer.msCount = SERIAL_SLAVE_RECEIVE_POLL_TIMEOUT;
                timer.pfnTimerCallback = SCI_Send_Slave_Recv;
#ifdef TIMER_CALLBACK_ARG
                timer.arg = NULL;
#endif
                g_SendTimerIndex = AddTimerQ(&timer);                    
            }
            else
            {
                /* There is a pending Write */
                uint_16 send_pkt_size = (uint_16)(g_send_buffer.transfer_size -
                    g_send_buffer.cur_offset);
                g_Serial.SerialFsm = SERIAL_WAIT_TRANSMIT;
                SendPktHeader(MASTER_SEND_HEADER, MIN(send_pkt_size, 
                    g_Serial.MaxPktSize));
            }
            break;
        }
    }
}

/**************************************************************************//*!
 *
 * @name  SCI_Send_Slave_Recv
 *
 * @brief This function when called sends master receive header to the agent
 *
 * @param arg	    :	Argument passed by Timer ISR (optional)
 *
 * @return None
 ******************************************************************************
 * This function when called sends master receive header to the agent.
 *****************************************************************************/
#ifdef TIMER_CALLBACK_ARG
static void SCI_Send_Slave_Recv(
	void* arg
)
#else
static void SCI_Send_Slave_Recv(void)
#endif
{
#ifdef TIMER_CALLBACK_ARG
    UNUSED(arg)
#endif    
    SCI_Class_Send_Slave_Send_Poll();
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
    
    /* Initiate Slave Send Polling Sequence */
    SCI_Class_Send_Slave_Send_Poll();
    return 0;    
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
#ifndef MC9S08 
    UNUSED(controller_ID)
#endif    
    
    SERIAL_CLASS_XFER_SIZE xfer_size;
    
    if(g_send_buffer.pBuffer != NULL)
        return SERIAL_BUSY;
    if(g_send_buffer.transfer_size == 0)
    {               
        memset((void*)&g_send_buffer, 0x00, sizeof(g_send_buffer));
        g_send_buffer.pBuffer = app_buff;
        g_send_buffer.size = size;

        xfer_size.direction = SERIAL_SEND;
        xfer_size.in_buff = g_send_buffer.pBuffer;
        xfer_size.in_size = g_send_buffer.size;
        xfer_size.transfer_size = 0;
        /* Get the total transfer size */
        g_Serial.SerialClassCallback(controller_ID, 
            SERIAL_APP_GET_TRANSFER_SIZE, &xfer_size);
        g_send_buffer.transfer_size = xfer_size.transfer_size;
        g_send_buffer.cur_offset = 0;
    }
    else
    {
        g_send_buffer.pBuffer = app_buff;
        g_send_buffer.size = size;               
    }
    if(g_Serial.SerialFsm == SERIAL_READY)
    {
        PSERIAL_COMM pTxComm = &g_Serial.TxComm;
        uint_16 send_pkt_size = (uint_16)(g_send_buffer.transfer_size - 
            g_send_buffer.cur_offset);
        g_Serial.SerialFsm = SERIAL_WAIT_TRANSMIT;
        SendPktHeader(MASTER_SEND_HEADER, MIN(send_pkt_size, 
            g_Serial.MaxPktSize));   
    }
    return 0;   
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
    memset((void*)&g_send_buffer, 0, sizeof(g_send_buffer));
    memset((void*)&g_recv_buffer, 0, sizeof(g_recv_buffer));
    SCI_Init(pSerialInit, SCI_Class_Callback);
    
    return SERIAL_CLASS_OK;
}

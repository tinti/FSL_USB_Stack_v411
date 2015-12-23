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
 * @file serial_protocol.h
 *
 * @author
 *
 * @version
 *
 * @date    June-17-2009
 *
 * @brief   This header file contains macros used by the serial protocol
 *****************************************************************************/
#ifndef _SERIAL_PROTOCOL_H
#define _SERIAL_PROTOCOL_H

/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/
/* Serial Protocol Packet Headers */
#define BUFFER_NEG_HEADER   0x80
#define MASTER_SEND_HEADER  0xA6
#define MASTER_SEND_ACK     0xA9
#define MASTER_RECV_HEADER  0xA5
#define MASTER_RECV_ACK     0xAA
#define PACKET_ACK          0xAC

/* Serial Protocol Packet Header Size */
#define PACKET_HEADER_SIZE  (3)

/* Slave Receive Poll Packet timeout in msec */
#define SERIAL_SLAVE_RECEIVE_POLL_TIMEOUT       10  
#define SERIAL_BUFF_NEGOTIATION_TIMEOUT         1000


#endif /* _SERIAL_PROTOCOL_H */
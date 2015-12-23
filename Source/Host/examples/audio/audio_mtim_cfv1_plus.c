/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
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
 * @file audio_mtim_cfv1_plus.c
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

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <string.h>
#include "derivative.h"     /* include peripheral declarations */
#include "types.h"          /* Contains User Defined Data Types */
#include "audio.h"

/******************************************************************************
 * Global functions
 *****************************************************************************/
void mtim_init(void);
void EnableTimer1Interrupt(void);
void DisableTimer1Interrupt(void);
void interrupt 110 mtim_isr(void);

extern void usb_host_audio_tr_callback(_usb_pipe_handle ,pointer ,uchar_ptr ,uint_32 ,uint_32 );

/******************************************************************************
 * Global variable
 *****************************************************************************/
extern volatile AUDIO_CONTROL_DEVICE_STRUCT audio_control;
extern volatile AUDIO_STREAM_DEVICE_STRUCT audio_stream;

extern uint_8 wav_buff[MAX_ISO_PACKET_SIZE];
extern uint_8 wav_recv_buff[MAX_ISO_PACKET_SIZE];
extern uint_8 wav_recv_buff_tmp[MAX_ISO_PACKET_SIZE];

extern uint_8 device_direction;
extern uint_32 packet_size;
extern volatile unsigned char *duty;
extern uint_8 resolution_size;
extern uint_8 sample_out;

extern USB_EVENT_STRUCT USB_Event;

uint_8 audio_sample = 0;

/******************************************************************************
*   @name        mtim_init
*
*   @brief       This function init MTIM
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
void mtim_init(void)
{
	MTIM0_CLK &= ~MTIM_CLK_CLKS_MASK; /* Select Bus Clock Source MTIM_CLK[CLKS] = 0 */
	MTIM0_CLK |= 0x03;	/* Prescaler = 8: MTIM_CLK[PS]  0x03 */
	MTIM0_MODH = 0x0B;	/* 1/8 ms Interrupt Generation */
	MTIM0_MODL = 0xB8;
	MTIM0_SC |= MTIM_SC_TRST_MASK ;  /* Clear previous MTIM Interrupt */
	MTIM0_SC &= ~MTIM_SC_TSTP_MASK; /* Start timer */
	
}
/******************************************************************************
 *   @name        EnableTimer1Interrupt
 *
 *   @brief       This routine enables Timer Interrupt
 *
 *   @return      None
 *
 ******************************************************************************
 * Enables RTC Timer Interrupt
 *****************************************************************************/
void EnableTimer1Interrupt(void)
{
	/* Enable Timer Interrupt */
	MTIM0_SC |= MTIM_SC_TOIE_MASK;
	return;
}

/******************************************************************************
 *   @name        DisableTimer1Interrupt
 *
 *   @brief       This routine disables Timer Interrupt
 *
 *   @return      None
 *
 ******************************************************************************
 * Disables RTC Timer Interrupt
 *****************************************************************************/
void DisableTimer1Interrupt(void)
{
	/* Disable Timer Interrupt */
	MTIM0_SC &= ~MTIM_SC_TOIE_MASK;
	return;
}

/******************************************************************************
 *   @name        mtim_isr
 *
 *   @brief       This routine services MTIM Interrupt
 *
 *	 @param       None
 *
 *   @return      None
 *
 ******************************************************************************
 * Services Programmable Interrupt Timer 0. If a Timer Object expires, then  
 * removes the object from Timer Queue and Calls the callback function 
 * (if registered)
 *****************************************************************************/
void interrupt 110 mtim_isr(void)
{
	/* Clear RTC Interrupt */
	  (void)MTIM0_SC; 
	  MTIM0_SC &= ~MTIM_SC_TOF_MASK; 
	if (USB_DEVICE_INTERFACED == audio_stream.DEV_STATE)
	{
		/* Check device type */
		if(IN_DEVICE == device_direction)
		{
			/* For speaker */
			/* Send data */
			usb_audio_send_data((CLASS_CALL_STRUCT_PTR)&audio_control.CLASS_INTF,
			(CLASS_CALL_STRUCT_PTR)&audio_stream.CLASS_INTF, usb_host_audio_tr_callback,
			NULL, packet_size, (uchar_ptr)wav_buff);			
		}
		else
		{
			/* For microphone */
			if((packet_size-1) < audio_sample)
			{	
				audio_sample = 0;
				_usb_event_clear(&USB_Event, USB_EVENT_RECEIVED_DATA);
				/* Recv data */
				usb_audio_recv_data((CLASS_CALL_STRUCT_PTR)&audio_control.CLASS_INTF,
				(CLASS_CALL_STRUCT_PTR)&audio_stream.CLASS_INTF, usb_host_audio_tr_callback,
				NULL, packet_size*resolution_size, (uchar_ptr)wav_recv_buff);
			}
			else
			{
				/* Check packet is sent completely */
		    	if (USB_EVENT_SET == _usb_event_wait_ticks(&USB_Event, USB_EVENT_RECEIVED_DATA,0,0))
		    	{
				    /* update duty */
		    		FTM1_C0VH = 0;
					FTM1_C0VL = wav_recv_buff_tmp[audio_sample];
		    	}
			}
			audio_sample+= sample_out;		
		}
	}
}

/* EOF */


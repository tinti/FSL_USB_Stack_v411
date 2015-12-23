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
 * @file PIT0_cfv2.c
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
#include "Int_Ctl_cfv2.h"
#include "audio.h"

/******************************************************************************
 * Global functions
 *****************************************************************************/
void pit0_init(void);
void EnableTimer1Interrupt(void);
void DisableTimer1Interrupt(void);
void __declspec(interrupt) PIT0_ISR(void);
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

int8 audio_sample = 0;

/******************************************************************************
*   @name        pit0_init
*
*   @brief       This function init PIT0
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
void pit0_init(void) 
{ 
	Int_Ctl_int_init(PIT0_INT_CNTL, PIT0_ISR_SRC, 2,2, TRUE);  
    /* Internal Bus Clock is System Clock/2 meaning 40 MHz */
    /* Prescaler 4 (PRE field equals to 2) thus giving an tick of 0.1 usec. */
    MCF_PIT0_PMR = 10000; 
    MCF_PIT0_PCSR = MCF_PIT_PCSR_PRE(2) | 
                    MCF_PIT_PCSR_PIF | 
                    MCF_PIT_PCSR_RLD |
                    MCF_PIT_PCSR_OVW |
                    MCF_PIT_PCSR_EN;
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
	/* Enable TPM Interrupt */
	MCF_PIT0_PCSR |= MCF_PIT_PCSR_PIE; 
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
	/* Disable TPM Interrupt */
	MCF_PIT0_PCSR &= ~MCF_PIT_PCSR_PIE; 
	return;
}

/******************************************************************************
 *   @name        PIT0_ISR
 *
 *   @brief       This routine services RTC Interrupt
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
void __declspec(interrupt) PIT0_ISR(void)
{
	MCF_PIT0_PCSR |= MCF_PIT_PCSR_PIF; /* Write 1 to Clear */
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
	    			 *duty = wav_recv_buff_tmp[audio_sample];
		    	}
			}
			audio_sample+= sample_out;		
		}
	}
}

/* EOF */


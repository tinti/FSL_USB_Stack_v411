/**HEADER********************************************************************
* 
* Copyright (c) 2010 Freescale Semiconductor;
* All Rights Reserved
*
*************************************************************************** 
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
**************************************************************************

*
* Comments:
*
*
*END************************************************************************/
#include "psptypes.h"
#include "derivative.h"
#include "rtc.h"
#include "audio.h"
#include "audio_pwm.h"

#include "cmt.h"

void interrupt VectorNumber_Vcmt cmt_isr(void);

extern volatile AUDIO_CONTROL_DEVICE_STRUCT audio_control;
extern volatile AUDIO_STREAM_DEVICE_STRUCT audio_stream;
extern uint_8 wav_buff[MAX_ISO_PACKET_SIZE];
extern uint_8 wav_recv_buff[MAX_ISO_PACKET_SIZE];
extern uint_8 wav_recv_buff_tmp[MAX_ISO_PACKET_SIZE];
extern uint_8 device_direction;
extern uint_32 packet_size;
extern USB_EVENT_STRUCT USB_Event;
extern uint_8 resolution_size;
extern uint_8 sample_out;

uint_8 audio_sample = 0;

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : TimerInit
* Returned Value   :
* Comments         : Initialize timer module
*    
*
*END*----------------------------------------------------------------------*/
void cmt_init(void)
{
  /* CMTMSC: EOCF=0,CMTDIV1=0,CMTDIV0=0,EXSPC=0,BASE=0,FSK=0,EOCIE=0,MCGEN=0 */
  CMTMSC = 0x00;                
  /* CMTOC: IROL=0,CMTPOL=0,IROPEN=0,??=0,??=0,??=0,??=0,??=0 */
  CMTOC = 0x00;                 
  /* CMTCG1: PH7=0,PH6=0,PH5=0,PH4=0,PH3=0,PH2=0,PH1=0,PH0=0,PL7=0,PL6=0,PL5=0,PL4=0,PL3=0,PL2=0,PL1=0,PL0=0 */
  CMTCG1 = 0x00;               
  /* CMTCG2: SH7=0,SH6=0,SH5=0,SH4=0,SH3=0,SH2=0,SH1=0,SH0=0,SL7=0,SL6=0,SL5=0,SL4=0,SL3=0,SL2=0,SL1=0,SL0=0 */
  CMTCG2 = 0x00;               
  /* CMTCMD12: MB15=0,MB14=0,MB13=0,MB12=0,MB11=0,MB10=0,MB9=0,MB8=1,MB7=0,MB6=1,MB5=1,MB4=1,MB3=0,MB2=1,MB1=1,MB0=0 */
  CMTCMD12 = TIMER_MODULAR;           
  /* CMTCMD34: SB15=0,SB14=0,SB13=0,SB12=0,SB11=0,SB10=0,SB9=0,SB8=0,SB7=0,SB6=0,SB5=0,SB4=0,SB3=0,SB2=0,SB1=0,SB0=0 */
  CMTCMD34 = 0x00;             
  /* CMTMSC: EOCF=0,CMTDIV1=0,CMTDIV0=0,EXSPC=0,BASE=0,FSK=0,EOCIE=1,MCGEN=1 */
  CMTMSC = 0x01;  
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : EnableTimerInterrupt 
* Returned Value   :
* Comments         : Enable timer interrupt
*    
*
*END*----------------------------------------------------------------------*/
void EnableTimer1Interrupt(void)
{
	/* Enable Timer Interrupt */
	  CMTMSC |= CMTMSC_EOCIE_MASK;
	return;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : DisableTimerInterrupt 
* Returned Value   :
* Comments         : Disable timer interrupt.
*    
*
*END*----------------------------------------------------------------------*/
void DisableTimer1Interrupt(void)
{
	/* Disable Timer Interrupt */
	CMTMSC &= ~CMTMSC_EOCIE_MASK;
	return;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : Timer_ISR 
* Returned Value   :
* Comments         : Timer interrupt service routine
*    
*
*END*----------------------------------------------------------------------*/
void interrupt VectorNumber_Vcmt cmt_isr(void)
{
    (void)CMTMSC; 
    (void)CMTCMD12;
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
  				    duty = wav_recv_buff_tmp[audio_sample];
  		    	}
  			}
  			audio_sample+=sample_out;		
  		}
  	}
}




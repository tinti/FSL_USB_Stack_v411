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
 * @file audio_ftm_cfv1_cmt.c
 *
 * @author
 *
 * @version
 *
 * @date    
 *
 * @brief   This file configures FTM for Timer implementation
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <string.h>
#include "derivative.h"     /* include peripheral declarations */
#include "types.h"          /* Contains User Defined Data Types */
#include "audio_ftm_cfv1_plus.h"
#include "usb_class.h"
#include "usb_descriptor.h"

extern uint_8 audio_sample;
extern uint_8 audio_event;
extern uint_8 audio_data_recv[8];
void interrupt 83 ftm0_isr(void);
extern uint_8 g_cur_mute[USB_MAX_SUPPORTED_INTERFACES];

/******************************************************************************
 * @name   ftm0_init    
 *
 * @brief   Init FTM0 counter   
 *
 * @param	   None
 *
 * @return     None
 *
 *****************************************************************************
 * This function initializes the FTM0 counter
 *****************************************************************************/
void ftm0_init(void)
{
	FTM0_CNTH = 0x00;
	FTM0_CNTL = 0x00;
	/* Set Modulo -> timer = 1/8s*/
	FTM0_MODH = 0x02;
	FTM0_MODL = 0xEE;
	
	FTM0_SC &= ~FTM_SC_TOF_MASK;
	/* Start timer */
	FTM0_SC  = 0x4A;
		/* b01001010
		 *  ||||||||_ PS:    Prescale factor selection - Divide by 4
		 *  |||||||__ PS: 
		 *  ||||||___ PS: 
		 *  |||||____ CLKS:  Select source clock is system_clock (48M)
		 *  ||||_____ CLKS:  
		 *  |||______ CPWMS: FTM counter operates in up counting mode
		 *  ||_______ TOIE:  Enable timer overflow flag 
		 *  |________ Timer  Overflow Flag
		 *  
		 * */
}

/******************************************************************************
 * @name  ftm0_isr     
 *
 * @brief  Interrupt service routine of real-time counter 
 *
 * @param	 None
 *
 * @return   None
 *
 *****************************************************************************
 * This function responses to Interrupt service routine of real-time counter
 *****************************************************************************/
void interrupt 83 ftm0_isr(void)
{
  
  if (audio_event == USB_APP_DATA_RECEIVED){
    /* update pulse width of PWM module*/
    FTM1_C0VH = 0;
    if(g_cur_mute[0] == 0)
    {
        FTM1_C0VL = audio_data_recv[audio_sample];
    } 
    else 
    {
        FTM1_C0VL = 0;
    }
    
    audio_sample+=1;
    if (7 < audio_sample){
      audio_sample = 0;
      audio_event = 0;
    }
  }
  (void)FTM0_SC; 
  FTM0_SC &= ~FTM_SC_TOF_MASK;
}


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
 * @file DTIM_cfv2.c
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
#include "DTIM_cfv2.h"
#include "audio.h"
#include "PIT0_cfv2.h"
#include "kbi_cfv2.h"


/******************************************************************************
 * Global functions
 *****************************************************************************/
void __declspec(interrupt) DTIM0_ISR(void);

extern USB_EVENT_STRUCT USB_Event;
extern boolean play;
extern volatile uint_8 mute_flag;
extern volatile AUDIO_CONTROL_DEVICE_STRUCT 	audio_stream;
/******************************************************************************
 * Global variable
 *****************************************************************************/

/******************************************************************************
*   @name        DTIM0_init
*
*   @brief       This function inits DTIM0
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
void DTIM0_init(void) 
{ 
	Int_Ctl_int_init(DTIM0_INT_CNTL, DTIM0_ISR_SRC, 4,4, TRUE);  
    /* Internal Bus Clock is System Clock/2 meaning 40 MHz */
    /* Prescaler 4 (PRE field equals to 2) thus giving an tick of 0.1 usec. */
    MCF_DTIM0_DTMR = 	MCF_DTIM_DTMR_PS(7) | 
    					MCF_DTIM_DTMR_CE_NONE |
    					MCF_DTIM_DTMR_FRR |
    					MCF_DTIM_DTMR_CLK_DIV1 |
    					MCF_DTIM_DTMR_ORRI;

	MCF_DTIM0_DTRR = 10000;
	
	MCF_DTIM0_DTER = 0x03;
	
	/* Enable timer */
	MCF_DTIM0_DTMR |= MCF_DTIM_DTMR_RST;

}

/******************************************************************************
 *   @name        EnableDTIM0Interrupt
 *
 *   @brief       This routine enables Timer Interrupt
 *
 *   @return      None
 *
 ******************************************************************************
 * Enables RTC Timer Interrupt
 *****************************************************************************/
void EnableDTIM0Interrupt(void)
{
	/* Enable TPM Interrupt */
	MCF_DTIM0_DTMR |= MCF_DTIM_DTMR_ORRI;
	return;
}

/******************************************************************************
 *   @name        DisableDTIM0Interrupt
 *
 *   @brief       This routine disables Timer Interrupt
 *
 *   @return      None
 *
 ******************************************************************************
 * Disables RTC Timer Interrupt
 *****************************************************************************/
void DisableDTIM0Interrupt(void)
{
	
	/* Disable TPM Interrupt */
	MCF_DTIM0_DTMR &= ~MCF_DTIM_DTMR_ORRI;
	return;
}

/******************************************************************************
 *   @name        DTIM0_ISR
 *
 *   @brief       This routine services DTIM Interrupt
 *
 *	 @param       None
 *
 *   @return      None
 *
 ******************************************************************************
 * Services Programmable Interrupt Timer 0. 
 *****************************************************************************/
void __declspec(interrupt) DTIM0_ISR(void)
{
	SwitchIntervalPollingTimerCallback();
	MCF_DTIM0_DTER |= 0x03;
}
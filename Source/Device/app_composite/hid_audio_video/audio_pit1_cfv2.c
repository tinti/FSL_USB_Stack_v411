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
* @file TPM1.c
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
#include "audio_pwm.h"
#include "audio_pit1_cfv2.h"
#include "usb_descriptor.h"

void pit1_init(void);
static void EnablePIT1Interrupt(void);
static void DisablePIT1Interrupt(void);
void __declspec(interrupt) PIT1_ISR(void);

extern volatile unsigned char *duty;

extern uint_8 audio_sample;
extern uint_8 audio_event;
extern uint_8 audio_data_recv[8];
extern uint_8 g_cur_mute[USB_MAX_SUPPORTED_INTERFACES];

/******************************************************************************
*   @name        tpm1_init
*
*   @brief       This function init TPM1
*
*   @return      None
*
*   @comment
*
*******************************************************************************/
void pit1_init(void)
{
    Int_Ctl_int_init(PIT1_INT_CNTL, PIT1_ISR_SRC, 3,3, TRUE);
    /* Internal Bus Clock is System Clock/2 meaning 40 MHz */
    /* Prescaler 4 (PRE field equals to 2) thus giving an tick of 0.1 usec. */
    MCF_PIT1_PCSR = MCF_PIT_PCSR_PRE(2) | MCF_PIT_PCSR_PIF; /* Clear previous RTC Interrupt */
    EnablePIT1Interrupt();
    MCF_PIT1_PMR = 0x04E2;                /* 1 ms Interrupt Generation */
    MCF_PIT1_PCSR |= MCF_PIT_PCSR_RLD;    /* Reload the Timer counter */
    MCF_PIT1_PCSR |= MCF_PIT_PCSR_EN;    /* Enable the Programmable Interrupt Timer 0 (PIT0) */
}

/******************************************************************************
*   @name        EnableTPM1Interrupt
*
*   @brief       This routine enables Timer Interrupt
*
*   @return      None
*
******************************************************************************
* Enables RTC Timer Interrupt
*****************************************************************************/
static void EnablePIT1Interrupt(void)
{
    /* Enable TPM Interrupt */
    MCF_PIT1_PCSR |= MCF_PIT_PCSR_PIE;
    return;
}

/******************************************************************************
*   @name        DisableTPM1Interrupt
*
*   @brief       This routine disables Timer Interrupt
*
*   @return      None
*
******************************************************************************
* Disables RTC Timer Interrupt
*****************************************************************************/
static void DisablePIT1Interrupt(void)
{
    /* Disable TPM Interrupt */
    MCF_PIT1_PCSR &= ~MCF_PIT_PCSR_PIE;
    return;
}

/******************************************************************************
*   @name        TPM1_ISR
*
*   @brief       This routine services RTC Interrupt
*
*     @param       None
*
*   @return      None
*
******************************************************************************
* Services Programmable Interrupt Timer 0. If a Timer Object expires, then
* removes the object from Timer Queue and Calls the callback function
* (if registered)
*****************************************************************************/
void __declspec(interrupt) PIT1_ISR(void)
{
    if (audio_event == 4){
        if(g_cur_mute[0] == 0)
        {
            *duty = audio_data_recv[audio_sample];
        }
        else
        {
            *duty = 0;
        }

        audio_sample+=1;
        if (7 < audio_sample){
            audio_sample = 0;
            audio_event = 0;
        }
    }
    MCF_PIT1_PCSR |= MCF_PIT_PCSR_PIF; /* Write 1 to Clear */
}
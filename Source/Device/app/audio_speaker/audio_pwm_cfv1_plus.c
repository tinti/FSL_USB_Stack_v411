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
 * @file audio_pwm_cfv1_plus.c
 *
 * @author
 *
 * @version
 *
 * @date    
 *
 * @brief   This file configures Pulse Width Modulation
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <string.h>
#include "derivative.h"     /* include peripheral declarations */
#include "types.h"          /* Contains User Defined Data Types */
#include "audio_pwm_cfv1_plus.h"

void pwm_init(void);

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : pwm_init
* Returned Value   :
* Comments         : Initialize timer module
*    
*END*------------------------------------------------------------------------*/

void pwm_init(void) 
{
    MXC_PTAPF4 &= ~MXC_PTAPF4_A0(0xF);
    MXC_PTAPF4 |= MXC_PTAPF4_A0(0x4);
    /* Init counter value*/
    FTM1_CNTH = 0x00;
    FTM1_CNTL = 0x00;
    /* Set Modulo -> free run mode*/
    FTM1_MODH = 0x00;
    FTM1_MODL = 0xFF;
    
    /* Disable dual capture mode and combine channels */
    FTM1_COMBINE0 &= ~(FTM_COMBINE_DECAPEN_MASK | FTM_COMBINE_COMBINE_MASK);
    
    /* Select channel 0 to generate PWM signal */
    FTM1_C0SC |= FTM_CnSC_MSB_MASK;
    FTM1_C0SC |= FTM_CnSC_ELSB_MASK;
    FTM1_C0SC &= ~FTM_CnSC_ELSA_MASK;
    
    /* Start timer */
    FTM1_SC  = 0x08;
        /* b00001000
         *  ||||||||_ PS:    Prescale factor selection - Divide by 1
         *  |||||||__ PS: 
         *  ||||||___ PS: 
         *  |||||____ CLKS:  Select source clock is system_clock (48M)
         *  ||||_____ CLKS:  
         *  |||______ CPWMS: FTM counter operates in up counting mode
         *  ||_______ TOIE:  Diable timer overflow flag 
         *  |________ Timer  Overflow Flag
         *  
         * */      
}

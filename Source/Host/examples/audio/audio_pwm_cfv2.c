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
 * @file audio_pwm_cfv2.c$
 *
 * @author
 *
 * @version
 *
 * @date    
 *
 * @brief   This file configures Pulse Width Modulation (RTC) for Timer 
 *          Implementation
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <string.h>
#include "derivative.h"     /* include peripheral declarations */
#include "types.h"          /* Contains User Defined Data Types */
#include "audio_pwm.h"

void pwm_init(void);
volatile unsigned char *duty;

/******************************************************************************
* Function Name    : TimerInit
* Returned Value   :
* Comments         : Initialize timer module
*    
******************************************************************************/
#if (defined __MCF52259_H__)
void pwm_init(void) 
{
	duty = &(MCF_PWM_PWMDTY1);
	
	/* Init IO for PWM */
	MCF_GPIO_PTAPAR |= 0x03;
	
	MCF_PWM_PWME = 0;
	/* PCKA = 1: clock A rate = bus clk / 1 = 40MHz*/
	MCF_PWM_PWMPRCLK = 0x00;
	MCF_PWM_PWMPER1 = 0xFF;
	MCF_PWM_PWMDTY1 = 0x00;
	
	/* High at begin of period */
	MCF_PWM_PWMPOL = 0x2;
	/* chose clock source is A */
	MCF_PWM_PWMCLK = 0x00;
	/* No Concatenates */
	MCF_PWM_PWMCTL = 0x00;
	
	/* Enable PWM */
	MCF_PWM_PWME = 0x02;
}
#endif

#if (defined __MCF52221_H__)
void pwm_init(void) 
{
	duty = &(MCF_PWM_PWMDTY0);
	
	/* Init IO for PWM */
	MCF_GPIO_PTCPAR |= 0x03;
	
	MCF_PWM_PWME = 0;
	/* PCKA = 1: clock A rate = bus clk / 1 = 40MHz*/
	MCF_PWM_PWMPRCLK = 0x00;
	MCF_PWM_PWMPER0 = 0xFF;
	MCF_PWM_PWMDTY0 = 0x00;
	
	/* High at begin of period */
	MCF_PWM_PWMPOL = 0x2;
	/* chose clock source is A */
	MCF_PWM_PWMCLK = 0x00;
	/* No Concatenates */
	MCF_PWM_PWMCTL = 0x00;
	
	/* Enable PWM */
	MCF_PWM_PWME = 0x01;
}
#endif
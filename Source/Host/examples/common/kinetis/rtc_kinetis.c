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
* $FileName:
* $Version :
* $Date    :
*
* Comments:
*
*
*END************************************************************************/
#include "psptypes.h"
#include "usb.h"
#include "derivative.h"
#include "rtc.h"
#include "host_common.h"

static int_16 _bsp_timer_sw_prescaller;
static int_16 _bsp_timer_sw_prescaller_cnt;

void TimerInit(void);
static void EnableTimerInterrupt(void);
static void DisableTimerInterrupt(void);

void  Timer_ISR(void);

uint_32 delay_count;

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : TimerInit
* Returned Value   :
* Comments         : Initialize timer module
*    
*
*END*----------------------------------------------------------------------*/
void TimerInit(void) 
{ 	
    /* Enable LPT Module Clock */
#ifdef MCU_MKL25Z4
	SIM_SCGC5 |= SIM_SCGC5_LPTMR_MASK;
#else
    SIM_SCGC5 |= SIM_SCGC5_LPTIMER_MASK;
#endif
    /* Configure LPT */
    LPTMR0_CMR = LPTMR_CMR_COMPARE(1);  // Set compare value
    LPTMR0_PSR = LPTMR_PSR_PCS(0x1);  //Use internal 1khz clock
    LPTMR0_CSR = LPTMR_CSR_TIE_MASK;  //Enable LPT interrupt

    LPTMR0_CSR |= LPTMR_CSR_TEN_MASK; //Turn on LPT and start counting

  EnableTimerInterrupt();
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : EnableTimerInterrupt 
* Returned Value   :
* Comments         : Enable timer interrupt
*    
*
*END*----------------------------------------------------------------------*/
static void EnableTimerInterrupt(void)
{
	/* Enable Timer Interrupt */
	LPTMR0_CSR |= LPTMR_CSR_TIE_MASK;
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
static void DisableTimerInterrupt(void)
{
	/* Disable Timer Interrupt */
	LPTMR0_CSR &= ~LPTMR_CSR_TIE_MASK;
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
void  Timer_ISR(void)
{
	LPTMR0_CSR |= LPTMR_CSR_TCF_MASK;  //Clear LPT Compare flag
	LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK; //Turn off LPT to avoid more interrupts
	
	if(0 <= delay_count)
		delay_count--;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : time_delay 
* Returned Value   :
* Comments         : Wait until interrupt of timer occur
*    
*
*END*----------------------------------------------------------------------*/
void time_delay(uint_32 delay) 
{
    uint_32 u32Delay;
    uint_32 u32Delay2;
  
    for(u32Delay=0; u32Delay<delay; u32Delay++)
        for(u32Delay2=0; u32Delay2<0x1111; u32Delay2++){};
     
}




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
* $FileName:	PIT1_cfv2.c
* $Version :
* $Date    :
*
* Comments:
*
*
*END************************************************************************/
#include "sci.h"
#include "psptypes.h"
#include "usb.h"
#include "derivative.h"
#include "Int_Ctl_cfv2.h"
#include "rtc.h"
#include "host_common.h"

#define PIT1_RELOAD_VAL 1000			/* 0.1 ms */
#define PIT1_PRE_VAL     2 

void TimerInit(void);
static void EnableTimerInterrupt(void);
static void DisableTimerInterrupt(void);

#ifdef OTG_BUILD
__declspec(interrupt) void PIT1_ISR(void);
#else
__declspec(interrupt) void Timer_ISR(void);
#endif

volatile uint_32 delay_count;
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
    MCF_PIT1_PMR = PIT1_RELOAD_VAL ; 
    MCF_PIT1_PCSR = MCF_PIT_PCSR_PRE(PIT1_PRE_VAL) | 
                    MCF_PIT_PCSR_PIF | 
                    MCF_PIT_PCSR_RLD |
                    MCF_PIT_PCSR_OVW |
                    MCF_PIT_PCSR_EN;
    
 
  Int_Ctl_int_init(PIT1_INT_CNTL, PIT1_ISR_SRC, 3,3, TRUE);
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
	MCF_PIT1_PCSR |= MCF_PIT_PCSR_PIE;
	
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
	MCF_PIT1_PCSR &= (~MCF_PIT_PCSR_PIE);
	
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : Timer_ISR 
* Returned Value   :
* Comments         : Timer interrupt service routine
*    
*
*END*----------------------------------------------------------------------*/
#ifdef OTG_BUILD
__declspec(interrupt) void PIT1_ISR(void)
#else
__declspec(interrupt) void Timer_ISR(void)
#endif
{
    if(MCF_PIT1_PCSR & MCF_PIT_PCSR_PIF)
    {
		/* Clear RTC Interrupt */
		MCF_PIT1_PCSR |= MCF_PIT_PCSR_PIF; /* Write 1 to Clear */
	    if(0 < delay_count)
	    {
	    delay_count--;	
	    }
	    if(delay_count == 0)
	    {
	    DisableTimerInterrupt();		
	    }
	}
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : delay 
* Returned Value   :
* Comments         : Wait until interrupt of timer occur
*    
*
*END*----------------------------------------------------------------------*/
void delay(uint_32 delay) 
{
  MCF_PIT1_PMR = PIT1_RELOAD_VAL;
  delay_count = delay;
  EnableTimerInterrupt();
  while(1){    
    if(0 == delay_count){
      break;
    }
  }
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
  MCF_PIT1_PMR = PIT1_RELOAD_VAL;
  delay_count = 10*delay;
  EnableTimerInterrupt();
  while(1){    
    if(0 == delay_count){
      break;
    }
  }
}


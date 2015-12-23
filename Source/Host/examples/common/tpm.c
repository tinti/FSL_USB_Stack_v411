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

#include "tmp.h"

void TimerInit(void);

static void EnableTimerInterrupt(void);

static void DisableTimerInterrupt(void);
#ifdef OTG_BUILD
void interrupt VectorNumber_Vtpm2ovf TPM2_ISR(void);
#else
void interrupt VectorNumber_Vtpm2ovf Timer_ISR(void);
#endif


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
    /* */
    TPM2SC = 0;
    TPM2MOD = 150;   /* 0.1 ms time base */  
    TPM2SC_PS = 0x04; /* divide by 16 */
    TPM2SC_CLKSx = 0x1;  /* Bus Clock Source 24 MHZ*/
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
	  TPM2SC_TOIE = 0x1;
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
void DisableTimerInterrupt(void)
{
	/* Disable Timer Interrupt */
	TPM2SC_TOIE = 0x0;
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
#ifdef OTG_BUILD
void interrupt VectorNumber_Vtpm2ovf TPM2_ISR(void)
#else
void interrupt VectorNumber_Vtpm2ovf Timer_ISR(void)
#endif
{
    if(TPM2SC & TPM2SC_TOF_MASK)
    {
		/* Clear RTC Interrupt */
		  TPM2SC_TOF = 0x0;
      if(0 <= delay_count)
        delay_count--;
		
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
  delay_count = delay;
  EnableTimerInterrupt();
  TPM2CNT = 0;
  while(1){    
    if(0 >= delay_count){
      DisableTimerInterrupt();
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
  delay_count = delay * 10;
  EnableTimerInterrupt();
  TPM2CNT = 0;
  while(1){    
    if(0 >= delay_count){
      DisableTimerInterrupt();
      break;
    }
  }
}


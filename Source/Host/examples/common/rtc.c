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
#include "sci.h"
#include "psptypes.h"
#include "usb.h"
#include "derivative.h"
#include "rtc.h"
#include "host_common.h"

void TimerInit(void);
static void EnableTimerInterrupt(void);
static void DisableTimerInterrupt(void);

void interrupt VectorNumber_Vrtc Timer_ISR(void);

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
	RTCSC_RTIF = 0x01; /* Clear previous RTC Interrupt */ 	
	RTCMOD = 0x01;	/* 1 ms Interrupt Generation */
	/* Start RTC by Reseting Counter to 0 */ 
	RTCSC = 0xA8;	/* Prescaler = 1000 Clock = 12MHz (external clock ) */       
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
	RTCSC_RTIE = 0x01;
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
	RTCSC_RTIE = 0x00;
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
void interrupt VectorNumber_Vrtc Timer_ISR(void)
{
    if(RTCSC & RTCSC_RTIF_MASK)
    {
		/* Clear RTC Interrupt */
		RTCSC_RTIF = 1; /* Write 1 to Clear */
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
  RTCMOD = 0x01; /* Reset RTCCNT */
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
  delay_count = 10 * delay;
  RTCMOD = 0x01; /* Reset RTCCNT */
  while(1){    
    if(0 == delay_count){
      break;
    }
  }
}



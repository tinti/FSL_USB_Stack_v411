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
 * @file mtim_cfv1_plus.c
 *
 * @author
 *
 * @version
 *
 * @date    
 *
 * @brief   This file configures Real Time Counter (RTC) for Timer 
 *          Implementation
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <string.h>
#include "derivative.h"     /* include peripheral declarations */
#include "types.h"          /* Contains User Defined Data Types */
#include "rtc.h"

void TimerInit(void);
static void EnableTimerInterrupt(void);
static void DisableTimerInterrupt(void);


/****************************************************************************
 * Global Variables
 ****************************************************************************/
volatile uint_32 delay_count;

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
void interrupt 110 Timer_ISR(void);

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

/******************************************************************************
 *   @name        TimerInit
 *
 *   @brief       This is RTC initialization function
 *
 *   @return      None
 *
 ******************************************************************************
 * Intiializes the RTC module registers
 *****************************************************************************/
void TimerInit(void)
{
	MTIM0_CLK &= ~MTIM_CLK_CLKS_MASK; /* Select Bus Clock Source MTIM_CLK[CLKS] = 0 */
	MTIM0_CLK |= 0x05;	/* Prescaler = 32: MTIM_CLK[PS]  0x05 */
	MTIM0_MODH = 0x00;	/* 0.1 ms Interrupt Generation */
	MTIM0_MODL = 0x4B;
	MTIM0_SC |= MTIM_SC_TRST_MASK ;  /* Clear previous MTIM Interrupt */
	MTIM0_SC &= ~MTIM_SC_TSTP_MASK; /* Start timer */
}

/******************************************************************************
 *   @name        EnableTimerInterrupt
 *
 *   @brief       This routine enables Timer Interrupt
 *
 *   @return      None
 *
 ******************************************************************************
 * Enables RTC Timer Interrupt
 *****************************************************************************/
static void EnableTimerInterrupt(void)
{
	/* Enable Timer Interrupt */
	MTIM0_SC |= MTIM_SC_TOIE_MASK;
	return;
}

/******************************************************************************
 *   @name        DisableTimerInterrupt
 *
 *   @brief       This routine disables Timer Interrupt
 *
 *   @return      None
 *
 ******************************************************************************
 * Disables RTC Timer Interrupt
 *****************************************************************************/
static void DisableTimerInterrupt(void)
{
	/* Disable Timer Interrupt */
	MTIM0_SC &= ~MTIM_SC_TOIE_MASK;
	return;
}

/******************************************************************************
 *   @name        Timer_ISR
 *
 *   @brief       This routine services RTC Interrupt
 *
 *	 @param       None
 *
 *   @return      None
 *
 ******************************************************************************
 * Services RTC Interrupt. If a Timer Object expires, then removes the object 
 * from Timer Queue and Calls the callback function (if registered)
 *****************************************************************************/
void interrupt 110 Timer_ISR()
{
    if(MTIM0_SC & MTIM_SC_TOF_MASK)
    {
		/* Clear RTC Interrupt */
		MTIM0_SC &= ~MTIM_SC_TOF_MASK; /* Write 1 to Clear */
		if(0 < delay_count)
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
	MTIM0_MODH = 0x00;	/* 0.1 ms Interrupt Generation */
	MTIM0_MODL = 0x4B;
	EnableTimerInterrupt();

	while(1){    
    if(0 == delay_count){
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
	delay_count = 10 * delay;
	MTIM0_MODH = 0x00;	/* 0.1 ms Interrupt Generation */
	MTIM0_MODL = 0x4B;
	EnableTimerInterrupt();
	while(1){    
		if(0 == delay_count){
			DisableTimerInterrupt();
		  break;
    }
  }
}


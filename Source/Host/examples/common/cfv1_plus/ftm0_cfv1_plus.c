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
 * @file ftm0_cfv1_plus.c
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
uint_32 delay_count;

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
void interrupt 83 Timer_ISR_FTM(void);

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

/******************************************************************************
 *   @name        TimerInit
 *
 *   @brief       This is FTM0 initialization function
 *
 *   @return      None
 *
 ******************************************************************************
 * Intializes the FTM0 module registers
 *****************************************************************************/
volatile uint_8 FTM0_MODH_reg@0xFFFF8443;
volatile uint_8 FTM0_MODL_reg@0xFFFF8444;
void TimerInit(void)
{
	FTM0_CNTH = 0x00;
	FTM0_CNTL = 0x00;
	/* Set Modulo -> timer = 0.1ms*/
	FTM0_MODH_reg = 0x00;
	FTM0_MODL_reg = 0x4B;
	
	FTM0_SC &= ~FTM_SC_TOF_MASK;
	/* Start timer */
	FTM0_SC  = 0x4D;
		/* b01001001
		 *  ||||||||_ PS:    Prescale factor selection - Divide by 32
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
	FTM0_SC |= FTM_SC_TOIE_MASK;
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
	FTM0_SC &= ~FTM_SC_TOIE_MASK;
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
void interrupt 83 Timer_ISR_FTM()
{
    if(FTM0_SC & FTM_SC_TOF_MASK)
    {
		/* Clear RTC Interrupt */
    	(void)FTM0_SC; 
    	FTM0_SC &= ~FTM_SC_TOF_MASK;
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
	/* Set Modulo -> timer = 0.1ms*/
	FTM0_MODH = 0x00;
	FTM0_MODL = 0x4B;
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
	/* Set Modulo -> timer = 0.1ms*/
	FTM0_MODH = 0x00;
	FTM0_MODL = 0x4B;
	EnableTimerInterrupt();
  	while(1){    
    if(0 == delay_count){
    	DisableTimerInterrupt();
      break;
    }
  }
}


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
 * @file RealTimerCounter_cfv2.c
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
#include "RealTimerCounter_cfv2.h"
#include "exceptions_cfv2.h"
#if MAX_TIMER_OBJECTS

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static uint_8 TimerInit(void);
static void EnableTimerInterrupt(void);
static void DisableTimerInterrupt(void);
void __declspec(interrupt) Timer_ISR(void);

/****************************************************************************
 * Global Variables
 ****************************************************************************/
/* Array of Timer Objects */
TIMER_OBJECT g_TimerObjectArray[MAX_TIMER_OBJECTS];
/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
uint_8 TimerQInitialize(uint_8 controller_ID);
uint_8 AddTimerQ(PTIMER_OBJECT pTimerObject);
uint_8 RemoveTimerQ(uint_8 index);

/*****************************************************************************
 * Global Functions
 *****************************************************************************/
/******************************************************************************
 *
 *   @name        TimerQInitialize
 *
 *   @brief       Initializes RTC, Timer Object Queue and System Clock Counter
 *
 *	 @param       controller_ID    : Controller ID
 *
 *   @return      None
 *****************************************************************************
 * This function initializes System Clock Counter, Timer Queue and Initializes
 * System Timer
 *****************************************************************************/
uint_8 TimerQInitialize(uint_8 controller_ID)
{
    UNUSED (controller_ID)
	(void)memset(g_TimerObjectArray, (int)NULL, sizeof(g_TimerObjectArray));
	return TimerInit();
}

/******************************************************************************
 *
 *   @name        AddTimerQ
 *
 *   @brief       Adds Timer Object to Timer Queue
 *
 *	 @param       pTimerObject	: Pointer to Timer Object
 *
 *   @return      None
 *****************************************************************************
 * Adds Timer Object to Timer Queue
 *****************************************************************************/
uint_8 AddTimerQ(PTIMER_OBJECT pTimerObject)
{
	uint_8 index;
	if(pTimerObject == NULL)
		return (uint_8)ERR_INVALID_PARAM;
	if(pTimerObject->msCount == (unsigned int)INVALID_TIME_COUNT)
		return (uint_8)ERR_INVALID_PARAM;
	
	for(index = 0; index < MAX_TIMER_OBJECTS; index++)
	{
	  	/* Disable Timer Interrupts */
    	DisableTimerInterrupt();
		
		if(g_TimerObjectArray[index].pfnTimerCallback == NULL)
		{
			(void)memcpy(&g_TimerObjectArray[index], pTimerObject, sizeof(TIMER_OBJECT)); 
			/* Enable Timer Interrupts */
			EnableTimerInterrupt();
			break;
		}
	  /* Enable Timer Interrupts */
		EnableTimerInterrupt();
	}
	if(index == MAX_TIMER_OBJECTS)
		return (uint_8)ERR_TIMER_QUEUE_FULL;
	return index;
}

/******************************************************************************
 *
 *   @name        RemoveTimerQ
 *
 *   @brief       Removes Timer Object from Timer Queue
 *
 *	 @param       index	: Index of Timer Object
 *
 *   @return      None
 *****************************************************************************
 * Removes Timer Object from Timer Queue
 *****************************************************************************/
uint_8 RemoveTimerQ(uint_8 index)
{
	if(index >= MAX_TIMER_OBJECTS)
		return (uint_8)ERR_INVALID_PARAM;
	/* Disable Timer Interrupts */
	DisableTimerInterrupt();
	(void)memset(&g_TimerObjectArray[index], (int)NULL, sizeof(TIMER_OBJECT));
	/* Enable Timer Interrupts */
	EnableTimerInterrupt();
	return (uint_8)ERR_SUCCESS;
}

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
static uint_8 TimerInit(void)
{   
    /* Internal Bus Clock is System Clock/2 meaning 40 MHz */
    /* Prescaler 4 (PRE field equals to 2) thus giving an tick of 0.1 usec. */
    MCF_PIT0_PCSR = MCF_PIT_PCSR_PRE(2) | MCF_PIT_PCSR_PIF; /* Clear previous RTC Interrupt */
    EnableTimerInterrupt();
	MCF_PIT0_PMR = 0x2710;				/* 1 ms Interrupt Generation */			
	MCF_PIT0_PCSR |= MCF_PIT_PCSR_RLD;	/* Reload the Timer counter */   
    MCF_PIT0_PCSR |= MCF_PIT_PCSR_EN;	/* Enable the Programmable Interrupt Timer 0 (PIT0) */
    
    return ERR_SUCCESS;
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
	MCF_PIT0_PCSR |= MCF_PIT_PCSR_PIE; 
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
	MCF_PIT0_PCSR &= ~MCF_PIT_PCSR_PIE;
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
 * Services Programmable Interrupt Timer 0. If a Timer Object expires, then  
 * removes the object from Timer Queue and Calls the callback function 
 * (if registered)
 *****************************************************************************/
void __declspec(interrupt) Timer_ISR(void)
{
	uint_8 index;
    if(MCF_PIT0_PCSR & MCF_PIT_PCSR_PIF)
    {
		/* Clear RTC Interrupt */
		MCF_PIT0_PCSR |= MCF_PIT_PCSR_PIF; /* Write 1 to Clear */

		/* Call Pending Timer CallBacks */
		for (index = 0; index < MAX_TIMER_OBJECTS; index++)
		{
			PTIMER_OBJECT ptemp = &g_TimerObjectArray[index];
			if(ptemp->pfnTimerCallback == NULL)
			{
				continue;
			}
			ptemp->msCount--;
			if (ptemp->msCount == 0) 
			{
			    PFNTIMER_CALLBACK pfnTimerCallback = ptemp->pfnTimerCallback;
#ifdef TIMER_CALLBACK_ARG
			    void *parg = ptemp->arg;
#endif
			    (void)RemoveTimerQ(index);
#ifdef TIMER_CALLBACK_ARG
				pfnTimerCallback(parg);
#else
				pfnTimerCallback();
#endif
			}
		}
	}
}
#endif
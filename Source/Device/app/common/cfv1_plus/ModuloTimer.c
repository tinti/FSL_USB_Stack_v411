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
 * @file ModuloTimer.c
 *
 * @author
 *
 * @version
 *
 * @date    
 *
 * @brief   This file configures Modulo Timer(MTIM) for Timer 
 *          Implementation
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <string.h>
#include "derivative.h"     /* include peripheral declarations */
#include "types.h"          /* Contains User Defined Data Types */
#include "RealTimerCounter.h"
#if MAX_TIMER_OBJECTS

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static uint_8 TimerInit(void);
static void EnableTimerInterrupt(void);
static void DisableTimerInterrupt(void);


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

void interrupt 110 Timer_ISR(void);

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
	MTIM0_CLK &= ~MTIM_CLK_CLKS_MASK; /* Select Bus Clock Source MTIM_CLK[CLKS] = 0 */
	MTIM0_CLK |= 0x06;	/* Prescaler = 64: MTIM_CLK[PS]  0x06 */
	MTIM0_MODH = 0x01;	/* 1 ms Interrupt Generation */
	MTIM0_MODL = 0x77;
	EnableTimerInterrupt();
	MTIM0_SC |= MTIM_SC_TRST_MASK ;  /* Clear previous MTIM Interrupt */
	MTIM0_SC &= ~MTIM_SC_TSTP_MASK; /* Start timer */
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
	uint_8 index;
    if(MTIM0_SC & MTIM_SC_TOF_MASK)
    {
		/* Clear RTC Interrupt */
		MTIM0_SC &= ~MTIM_SC_TOF_MASK; 

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

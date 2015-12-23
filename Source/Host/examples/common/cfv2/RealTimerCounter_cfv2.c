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
/* INCLUDES ---------------------------------------------------------------*/
#include <string.h>
#include "ieee11073_timer.h"
#include "derivative.h"
#include "exceptions_cfv2.h"
#include "Int_Ctl_cfv2.h"

#if MAX_TIMER_OBJECTS

/* LOCAL FUNCTION PROTOTYPES ------------------------------------------------*/
static uint_8 TimerInit(void);
static void EnableTimerInterrupt(void);
static void DisableTimerInterrupt(void);
void __declspec(interrupt) PIT0_ISR(void);

/* GLOBLA VARIABLES ---------------------------------------------------------*/
/* Array of Timer Objects */
TIMER_OBJECT g_TimerObjectArray[MAX_TIMER_OBJECTS];

/* GLOBAL FUNCTION PROTOTYPES ------------------------------------------------*/
uint_8 TimerQInitialize(uint_8 controller_ID);
uint_8 AddTimerQ(PTIMER_OBJECT pTimerObject, uint_8 *);
uint_8 RemoveTimerQ(uint_8 index);

/* GLOBLA FUNCTION*--------------------------------------------------------
* 
* Function Name  :  TimerQInitialize
* Returned Value :  None
* Comments       :  Initializes RTC, Timer Object Queue and System Clock Counter
*      
*END*--------------------------------------------------------------------*/
uint_8 TimerQInitialize(uint_8 controller_ID)
{
    UNUSED (controller_ID)
    (void)memset(g_TimerObjectArray, (int)NULL, sizeof(g_TimerObjectArray));
    return TimerInit();
}

/* GLOBLA FUNCTION*--------------------------------------------------------
* 
* Function Name  :  AddTimerQ
* Returned Value :  timer index
* Comments       :  Adds Timer Object to Timer Queue
*      
*END*--------------------------------------------------------------------*/
uint_8 AddTimerQ(PTIMER_OBJECT pTimerObject, uint_8 *timer_index)
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
            if (NULL != timer_index)
            {
              *timer_index = index;  
            }
            
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

/* GLOBLA FUNCTION*--------------------------------------------------------
* 
* Function Name  :  RemoveTimerQ
* Returned Value :  ERR_SUCCESS if successful
*                   ERR_INVALID PARAM if failed
* Comments       :  Removes Timer Object from Timer Queue
*      
*END*--------------------------------------------------------------------*/
uint_8 RemoveTimerQ(uint_8 index)
{
    if(index >= MAX_TIMER_OBJECTS)
        return (uint_8)ERR_INVALID_PARAM;
    /* Disable Timer Interrupts */
    DisableTimerInterrupt();
    (void)memset(&g_TimerObjectArray[index], (int)NULL, sizeof(TIMER_OBJECT));
    g_TimerObjectArray[index].pfnTimerCallback = NULL;
    /* Enable Timer Interrupts */
    EnableTimerInterrupt();
    return (uint_8)ERR_SUCCESS;
}

/* STATIC FUNCTION*--------------------------------------------------------
* 
* Function Name  :  TimerInit
* Returned Value :  None
* Comments       :  This is RTC initialization function
*      
*END*--------------------------------------------------------------------*/
static uint_8 TimerInit(void)
{   
    /* Internal Bus Clock is System Clock/2 meaning 40 MHz */
    /* Prescaler 4 (PRE field equals to 2) thus giving an tick of 0.1 usec. */
    MCF_PIT0_PCSR = MCF_PIT_PCSR_PRE(2) | MCF_PIT_PCSR_PIF; /* Clear previous RTC Interrupt */
    //EnableTimerInterrupt();
    MCF_PIT0_PMR = 0x2710;                /* 1 ms Interrupt Generation */            
    MCF_PIT0_PCSR |= MCF_PIT_PCSR_RLD;    /* Reload the Timer counter */   
    MCF_PIT0_PCSR |= MCF_PIT_PCSR_EN;    /* Enable the Programmable Interrupt Timer 0 (PIT0) */
    Int_Ctl_int_init(PIT0_INT_CNTL, PIT0_ISR_SRC, 4,4, TRUE);
    
    return ERR_SUCCESS;
}

/* STATIC FUNCTION*--------------------------------------------------------
* 
* Function Name  :  EnableTimerInterrupt
* Returned Value :  None
* Comments       :  This routine enables Timer Interrupt
*      
*END*--------------------------------------------------------------------*/
static void EnableTimerInterrupt(void)
{
    /* Enable Timer Interrupt */
    MCF_PIT0_PCSR |= MCF_PIT_PCSR_PIE; 
    return;
}

/* STATIC FUNCTION*--------------------------------------------------------
* 
* Function Name  :  DisableTimerInterrupt
* Returned Value :  None
* Comments       :  This routine disables Timer Interrupt
*      
*END*--------------------------------------------------------------------*/
static void DisableTimerInterrupt(void)
{
    /* Disable Timer Interrupt */
    MCF_PIT0_PCSR &= ~MCF_PIT_PCSR_PIE;
    return;
}

/* STATIC FUNCTION*--------------------------------------------------------
* 
* Function Name  :  Timer_ISR
* Returned Value :  None
* Comments       :  Services Programmable Interrupt Timer 0. If a Timer Object expires, then  
*                   removes the object from Timer Queue and Calls the callback function 
*                   (if registered)
*      
*END*--------------------------------------------------------------------*/
void __declspec(interrupt) PIT0_ISR(void)
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
                void *parg = ptemp->arg;
                (void)RemoveTimerQ(index);
                pfnTimerCallback(index,parg);
            }
        }
    }
}
#endif
/* EOF */
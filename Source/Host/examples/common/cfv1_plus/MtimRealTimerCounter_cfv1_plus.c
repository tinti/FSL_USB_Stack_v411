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
 * @file audio_mtim_cfv1_plus.c
 *
 * @author
 *
 * @version
 *
 * @date    
 *
 * @brief   
 *          
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <string.h>
#include "ieee11073_timer.h"
#include "derivative.h"

#if MAX_TIMER_OBJECTS
/* LOCAL FUNCTION PROTOTYPES ------------------------------------------------*/
static uint_8 MTIM_TimerInit(void);
static void EnableTimerInterrupt(void);
static void DisableTimerInterrupt(void);
void interrupt 110 MTIM_ISR(void);

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
    return MTIM_TimerInit();
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
* Function Name  :  Timer_ISR
* Returned Value :  None
* Comments       :  Services Programmable Interrupt Timer 0. If a Timer Object expires, then  
*                   removes the object from Timer Queue and Calls the callback function 
*                   (if registered)
*      
*END*--------------------------------------------------------------------*/
void interrupt 110 MTIM_ISR(void)
{
	uint_8 index;	
	/* Clear RTC Interrupt */
	(void)MTIM0_SC; 
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
            void *parg = ptemp->arg;
            (void)RemoveTimerQ(index);
            pfnTimerCallback(index,parg);
        }
    }
}
/*STATIC FUNCTION*-----------------------------------------------------------
*
* Function Name    : TimerInit
* Returned Value   :
* Comments         : Initialize timer module
*    
*
*END*----------------------------------------------------------------------*/
static uint_8 MTIM_TimerInit(void)
{
	MTIM0_CLK &= ~MTIM_CLK_CLKS_MASK; /* Select Bus Clock Source MTIM_CLK[CLKS] = 0 */
	MTIM0_CLK |= 0x03;	/* Prescaler = 8: MTIM_CLK[PS]  0x03 */
	MTIM0_MODH = 0x0B;	/* 1/8 ms Interrupt Generation */
	MTIM0_MODL = 0xB8;
	MTIM0_SC |= MTIM_SC_TRST_MASK ;  /* Clear previous MTIM Interrupt */
	MTIM0_SC &= ~MTIM_SC_TSTP_MASK; /* Start timer */
	return ERR_SUCCESS;
}

/*STATIC FUNCTION*-----------------------------------------------------------
*
* Function Name    : EnableTimerInterrupt
* Returned Value   :
* Comments         : This routine enables Timer Interrupt
*    
*
*END*----------------------------------------------------------------------*/
static void EnableTimerInterrupt(void)
{
	/* Enable Timer Interrupt */
	MTIM0_SC |= MTIM_SC_TOIE_MASK;
	return;
}

/*STATIC FUNCTION*-----------------------------------------------------------
*
* Function Name    : DisableTimerInterrupt
* Returned Value   :
* Comments         : This routine disables Timer Interrupt
*    
*
*END*----------------------------------------------------------------------*/
static void DisableTimerInterrupt(void)
{
	/* Disable Timer Interrupt */
	MTIM0_SC &= ~MTIM_SC_TOIE_MASK;
	return;
}
#endif
/* EOF */


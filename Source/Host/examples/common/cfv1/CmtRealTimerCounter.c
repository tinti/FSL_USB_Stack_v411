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
#include <string.h>
#include "ieee11073_timer.h"
#include "derivative.h"

#if MAX_TIMER_OBJECTS

/* LOCAL FUNCTION PROTOTYPES ------------------------------------------------*/
static uint_8 CMT_TimerInit(void);
static void EnableTimerInterrupt(void);
static void DisableTimerInterrupt(void);
void interrupt VectorNumber_Vcmt CMT_ISR(void);

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
    return CMT_TimerInit();
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
void interrupt VectorNumber_Vcmt CMT_ISR(void)
{
    uint_8 index;
    (void)CMTMSC; 
    (void)CMTCMD12;

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
static uint_8 CMT_TimerInit(void)
{
  /* CMTMSC: EOCF=0,CMTDIV1=0,CMTDIV0=0,EXSPC=0,BASE=0,FSK=0,EOCIE=0,MCGEN=0 */
  CMTMSC = 0x00;                
  /* CMTOC: IROL=0,CMTPOL=0,IROPEN=0,??=0,??=0,??=0,??=0,??=0 */
  CMTOC = 0x00;                 
  /* CMTCG1: PH7=0,PH6=0,PH5=0,PH4=0,PH3=0,PH2=0,PH1=0,PH0=0,PL7=0,PL6=0,PL5=0,PL4=0,PL3=0,PL2=0,PL1=0,PL0=0 */
  CMTCG1 = 0x00;               
  /* CMTCG2: SH7=0,SH6=0,SH5=0,SH4=0,SH3=0,SH2=0,SH1=0,SH0=0,SL7=0,SL6=0,SL5=0,SL4=0,SL3=0,SL2=0,SL1=0,SL0=0 */
  CMTCG2 = 0x00;               
  /* CMTCMD12: MB15=0,MB14=0,MB13=0,MB12=0,MB11=0,MB10=0,MB9=0,MB8=1,MB7=0,MB6=1,MB5=1,MB4=1,MB3=0,MB2=1,MB1=1,MB0=0 */
  CMTCMD12 = 2992;           
  /* CMTCMD34: SB15=0,SB14=0,SB13=0,SB12=0,SB11=0,SB10=0,SB9=0,SB8=0,SB7=0,SB6=0,SB5=0,SB4=0,SB3=0,SB2=0,SB1=0,SB0=0 */
  CMTCMD34 = 0x00;             
  /* CMTMSC: EOCF=0,CMTDIV1=0,CMTDIV0=0,EXSPC=0,BASE=0,FSK=0,EOCIE=1,MCGEN=1 */
  CMTMSC = 0x01;  
  return ERR_SUCCESS;
}


/*STATIC FUNCTION*-----------------------------------------------------------
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
	CMTMSC |= CMTMSC_EOCIE_MASK;
	return;
}

/*STATIC FUNCTION*-----------------------------------------------------------
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
	CMTMSC &= ~CMTMSC_EOCIE_MASK;
	return;
}
#endif
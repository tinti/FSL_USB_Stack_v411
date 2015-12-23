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
* $FileName:	PIT1_kinetis.c
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
#include "host_common.h"
#include "usb_bsp.h"

#ifdef MCU_MK70F12
#define PIT1_RELOAD_VAL 6000
#else
	#if (defined MCU_MK20D7) || (defined MCU_MK40D7)
		#ifdef MCGOUTCLK_72_MHZ 
			#define PIT1_RELOAD_VAL 3600
		#else
			#define PIT1_RELOAD_VAL 4800
		#endif
	#else
        #ifdef MCU_MKL25Z4
            #define PIT1_RELOAD_VAL 2400 
        #else
		    #define PIT1_RELOAD_VAL 4800
        #endif
    #endif
#endif
#define PIT1_PRE_VAL     2 

void TimerInit(void);
static void EnableTimerInterrupt(void);
static void DisableTimerInterrupt(void);

void PIT1_ISR(void);

volatile int_32 delay_count;

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
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
#ifdef MCU_MK20D5
	NVICICPR0 = (1 << 31);   /* Clear any pending interrupts on PIT1 */
	NVICISER0 = (1 << 31);   /* Enable interrupts from PIT1 module */
#elif (defined MCU_MK21D5)
	NVICICPR1 = (1 << 17);   /* Clear any pending interrupts on PIT1 */
	NVICISER1 = (1 << 17);   /* Enable interrupts from PIT1 module */
#elif (defined MCU_MKL25Z4) || (defined MCU_MKL46Z4)
	NVIC_ICPR = (1 << 22);   /* Clear any pending interrupts on PIT1 */
	NVIC_ISER = (1 << 22);   /* Enable interrupts from PIT1 module */
#else
	NVICICPR2 = (1 << 5);   /* Clear any pending interrupts on PIT1 */
	NVICISER2 = (1 << 5);   /* Enable interrupts from PIT1 module */
#endif
	
	/* Configure PIT1 */
	PIT_MCR = ~(PIT_MCR_FRZ_MASK | PIT_MCR_MDIS_MASK);

	/* Timer 0.1ms */
	PIT_LDVAL1 = PIT1_RELOAD_VAL;
	
	/* Mask PIT interrupt flag */
	PIT_TFLG1 = PIT_TFLG_TIF_MASK;

	/* Enable PIT interrupt */
	PIT_TCTRL1 |= PIT_TCTRL_TIE_MASK;
	PIT_TCTRL1 |= PIT_TCTRL_TEN_MASK;
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
	PIT_TCTRL1 |= PIT_TCTRL_TEN_MASK;
	PIT_TCTRL1 |= PIT_TCTRL_TIE_MASK;	
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
	PIT_TCTRL1 &= ~PIT_TCTRL_TEN_MASK;
	PIT_TCTRL1 &= ~PIT_TCTRL_TIE_MASK;	
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : PIT1_ISR 
* Returned Value   :
* Comments         : Timer interrupt service routine
*    
*
*END*----------------------------------------------------------------------*/
void PIT1_ISR(void)
{
	if(PIT_TFLG1 & PIT_TFLG_TIF_MASK)
	{
		/* Clear PIT Interrupt */
		PIT_TFLG1 |= PIT_TFLG_TIF_MASK;
		
		DisableTimerInterrupt();
#ifdef MCU_MK20D5
	        NVICICPR0 = (1 << 31);   /* Clear any pending interrupts on PIT1 */
	        NVICISER0 = (1 << 31);   /* Enable interrupts from PIT1 module */
#elif(defined MCU_MK21D5)
	        NVICICPR1 = (1 << 17);   /* Clear any pending interrupts on PIT1 */
	        NVICISER1 = (1 << 17);   /* Enable interrupts from PIT1 module */
#elif(defined MCU_MKL25Z4) || (defined MCU_MKL46Z4)
	        NVIC_ICPR = (1 << 22);   /* Clear any pending interrupts on PIT1 */
	        NVIC_ISER = (1 << 22);   /* Enable interrupts from PIT1 module */
#else
            NVICICPR2 = (1 << 5);    /* Clear any pending interrupts on PIT1 */
	        NVICISER2 = (1 << 5);    /* Enable interrupts from PIT1 module */
#endif		
		if(0 < delay_count)
		{
			delay_count--;
			EnableTimerInterrupt();
		}
		if(delay_count <= 0)
		{
			DisableTimerInterrupt();		
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
	PIT_LDVAL1 = PIT1_RELOAD_VAL;
	delay_count = 10*delay;
	EnableTimerInterrupt();
  
	while(1)
	{    
		if(0 >= delay_count)
		{
			break;
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
	PIT_LDVAL1 = PIT1_RELOAD_VAL;
	delay_count = delay;
	EnableTimerInterrupt();
  
	while(1)
	{    
		if(0 == delay_count)
		{
			break;
		}
	}
}

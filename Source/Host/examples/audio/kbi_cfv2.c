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
 * @file kbi_cfv2.c
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
 
#include <string.h>
#include "derivative.h"     /* include peripheral declarations */
#include "types.h"          /* Contains User Defined Data Types */
#include "Int_Ctl_cfv2.h"
#include "audio.h"
#include "PIT0_cfv2.h"
#include "kbi_cfv2.h"

#ifdef __MCF52259_EVB_
#include "DTIM_cfv2.h"
#endif

extern volatile AUDIO_CONTROL_DEVICE_STRUCT 	audio_stream;
void __declspec(interrupt) IRQ_ISR(void);

/***************************************
**
** Global variables
****************************************/
extern boolean play;
extern uint_8 mute_flag;
extern USB_EVENT_STRUCT USB_Event;

/***************************************
**
** Global functions
****************************************/

/******************************************************************************
*   @name        GPIO_Init
*
*   @brief       This function init GPIO
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
void GPIO_Init(void)
{
#ifdef __MCF52259_DEMO_ 
	/* M52259Demo: SW1, SW2 connected to PNQ1, PNQ5 */
	/* Configure PNQ1, PNQ5 as input */
					 	
	Int_Ctl_int_init(IRQ1_INT_CNTL, IRQ1_ISR_SRC, 3,2, TRUE); 
	Int_Ctl_int_init(IRQ5_INT_CNTL, IRQ5_ISR_SRC, 3,3, TRUE);     	   

	MCF_GPIO_PNQPAR &= ~(MCF_GPIO_PNQPAR_PNQPAR1(3) | MCF_GPIO_PNQPAR_PNQPAR5(3));
	MCF_GPIO_PNQPAR |= (MCF_GPIO_PNQPAR_IRQ1_IRQ1 | MCF_GPIO_PNQPAR_IRQ5_IRQ5); 
	
	/* Configure IRQ[1] & IRQ[5] as input */
	MCF_EPORT_EPDDR &= ~(MCF_EPORT_EPDDR_EPDD1 | MCF_EPORT_EPDDR_EPDD5); 
	/* Configure IRQ1 & IRQ5 as falling-edge sense */
	MCF_EPORT_EPPAR = MCF_EPORT_EPPAR_EPPA1_FALLING | MCF_EPORT_EPPAR_EPPA5_FALLING;
	/* Enable Interrupt for IRQ1 & IRQ5 */
	MCF_EPORT_EPIER = MCF_EPORT_EPIER_EPIE1 | MCF_EPORT_EPIER_EPIE5;  
#endif

#ifdef __MCF52221_H__
	/* M52221Demo: SW1, SW2 connected to PNQ1, PNQ7 */
	/* Configure PNQ1, PNQ7 as input */
	Int_Ctl_int_init(IRQ1_INT_CNTL, IRQ1_ISR_SRC, 3,2, TRUE); 
	Int_Ctl_int_init(IRQ7_INT_CNTL, IRQ7_ISR_SRC, 2,2, TRUE);       	   

	MCF_GPIO_PNQPAR &= ~(MCF_GPIO_PNQPAR_PNQPAR1(3) | MCF_GPIO_PNQPAR_PNQPAR7(3));
	MCF_GPIO_PNQPAR |= (MCF_GPIO_PNQPAR_IRQ1_IRQ1 | MCF_GPIO_PNQPAR_IRQ7_IRQ7); 
	
	/* Configure IRQ[1] & IRQ[7] as input */
	MCF_EPORT_EPDDR &= ~(MCF_EPORT_EPDDR_EPDD1 | MCF_EPORT_EPDDR_EPDD7); 
	/* Configure IRQ1 & IRQ5 as falling-edge sense */
	MCF_EPORT_EPPAR = MCF_EPORT_EPPAR_EPPA1_FALLING | MCF_EPORT_EPPAR_EPPA7_FALLING;
	/* Enable Interrupt for IRQ1 & IRQ5 */
	MCF_EPORT_EPIER = MCF_EPORT_EPIER_EPIE1 | MCF_EPORT_EPIER_EPIE7;  
#endif

#ifdef __MCF52259_EVB_
 	/***************************************************************************
 	*						 GPIO Init for KBI
 	*
 	****************************************************************************/    
    /* Configure switch buttons */
	/* M52259EVB: SW1, SW2, SW3, SW4 connected to PDD[5 - 7] */
	/* Configure PDD[5 - 7] as input */
	MCF_GPIO_DDRDD &= ~(MCF_GPIO_DDRDD_DDRDD5 |
					 	MCF_GPIO_DDRDD_DDRDD6 |
					 	MCF_GPIO_DDRDD_DDRDD7);
	/* Pin assignement registers: GPIO */   	
	/* Assure that PDDPAR[5-7] are configured to GPIO regardless of the reset state */
	MCF_GPIO_PDDPAR &= ~(MCF_GPIO_PDDPAR_PDDPAR5 | 
	  	                 MCF_GPIO_PDDPAR_PDDPAR6 |  
	  	                 MCF_GPIO_PDDPAR_PDDPAR7); 
	DTIM0_init();
#endif
}


/******************************************************************************
*   @name        IRQ_ISR
*
*   @brief       Service interrupt routine of IRQ
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
#ifdef __MCF52259_DEMO_
void __declspec(interrupt) IRQ_ISR(void)
{
	 	if(MCF_EPORT_EPFR & MCF_EPORT_EPFR_EPF1)
	 	{
		 	if(USB_DEVICE_INTERFACED == audio_stream.DEV_STATE)
		 	{
				play = (boolean)(1 - play);
				/* play */
		 		if (TRUE == play)
		 		{
		 			printf("Playing ...\r\n");
		 			_usb_event_set(&USB_Event, USB_EVENT_RECEIVED_DATA);
		 			EnableTimer1Interrupt();
		 		}
		 		/* stop */
		 		else
		 		{
		 			printf("\nPaused.\n");
		 			DisableTimer1Interrupt();
		 		}
		 	}
		 	MCF_EPORT_EPFR |= MCF_EPORT_EPFR_EPF1;	/* Clear the bit by writting a 1 to it */	 			
	 	}
	 	
	 	if(MCF_EPORT_EPFR & MCF_EPORT_EPFR_EPF5)
	 	{
	 		mute_flag=1;
	 		MCF_EPORT_EPFR |= MCF_EPORT_EPFR_EPF5;	/* Clear the bit by writting a 1 to it */	 			
	 	}	 	
}
#endif

#ifdef __MCF52221_H__
void __declspec(interrupt) IRQ_ISR(void)
{
	 	if(MCF_EPORT_EPFR & MCF_EPORT_EPFR_EPF1)
	 	{
		 	if(USB_DEVICE_INTERFACED == audio_stream.DEV_STATE)
		 	{
				play = (uint_8)(1 - play);
				/* play */
		 		if (TRUE == play)
		 		{
		 			printf("Playing ...\r\n");
		 			_usb_event_set(&USB_Event, USB_EVENT_RECEIVED_DATA);
		 			EnableTimer1Interrupt();
		 		}
		 		/* stop */
		 		else
		 		{
		 			printf("\nPaused.\n");
		 			DisableTimer1Interrupt();
		 		}
		 	}
		 	MCF_EPORT_EPFR |= MCF_EPORT_EPFR_EPF1;	/* Clear the bit by writting a 1 to it */	 			
	 	}
	 	
	 	if(MCF_EPORT_EPFR & MCF_EPORT_EPFR_EPF7)
	 	{
	 		mute_flag=1;
	 		MCF_EPORT_EPFR |= MCF_EPORT_EPFR_EPF7;	/* Clear the bit by writting a 1 to it */	 			
	 	}	 	
}
#endif

#ifdef __MCF52259_EVB_
volatile uint_8 kbi_gpio_state = MCF_GPIO_SETDD_SETDD5 | MCF_GPIO_SETDD_SETDD6 | MCF_GPIO_SETDD_SETDD7;
volatile uint_8 kbi_stat;
void SwitchIntervalPollingTimerCallback(void)
{

 uint_8 kbi_gpio_new_state = 0;	
 	/* Sample the GPIO */
 	kbi_gpio_new_state = (uint_8)(MCF_GPIO_SETDD & (uint_8)(MCF_GPIO_SETDD_SETDD5 | MCF_GPIO_SETDD_SETDD6 | MCF_GPIO_SETDD_SETDD7));
 	
 	kbi_gpio_state ^= kbi_gpio_new_state; 	 	
 	
 	
 	if((kbi_gpio_state & MCF_GPIO_SETDD_SETDD5) && !(kbi_gpio_new_state & MCF_GPIO_SETDD_SETDD5))
 	{ 		
	 	if(USB_DEVICE_INTERFACED == audio_stream.DEV_STATE)
	 	{
			play = 1 - play;
			/* play */
	 		if (TRUE == play)
	 		{
	 			printf("Playing ...\r\n");
	 			_usb_event_set(&USB_Event, USB_EVENT_RECEIVED_DATA);
	 			EnableTimer1Interrupt();
	 		}
	 		/* stop */
	 		else
	 		{
	 			printf("\nPaused.\n");
	 			DisableTimer1Interrupt();
	 		}
	 	}
 	}	
 
 	if((kbi_gpio_state & MCF_GPIO_SETDD_SETDD6) && !(kbi_gpio_new_state & MCF_GPIO_SETDD_SETDD6))
 	{ 		
	 		mute_flag=1;
 	}
 	 	
	if((kbi_gpio_state & MCF_GPIO_SETDD_SETDD7) && !(kbi_gpio_new_state & MCF_GPIO_SETDD_SETDD7))
	{		
 		kbi_stat |= 0x08;
	} 	 	
 	
 	kbi_gpio_state = kbi_gpio_new_state;

}
#endif

/* EOF */

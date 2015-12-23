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
 * @file kbi_cfv1_plus.c
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
#include "audio.h"
#include "audio_mtim_cfv1_plus.h"

void GPIO_Init(void);
void interrupt 64 IRQ_ISR(void);
extern volatile AUDIO_CONTROL_DEVICE_STRUCT 	audio_stream;

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
	  /* Enable IRQ clock */
	   SIM_SCGC4 |= SIM_SCGC4_IRQ_MASK;		/* set input PTB 7*/
	   /* Configure PTB0 is IRQ */
	   MXC_PTBPF4 &=~MXC_PTBPF4_B0(0xF);
	   MXC_PTBPF4 |=MXC_PTBPF4_B0(0x5);
	   /* Enable interrupt */
	   IRQ_SC = IRQ_SC_IRQIE_MASK | IRQ_SC_IRQPE_MASK;
}
/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : IRQ_ISR
* Returned Value : none
* Comments       : IRQ interrupt service routine
*     
*
*END*--------------------------------------------------------------------*/
void interrupt 64 IRQ_ISR(void)
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
     IRQ_SC |= IRQ_SC_IRQF_MASK | IRQ_SC_IRQACK_MASK;     /* clear KBI interrupt */ 
}




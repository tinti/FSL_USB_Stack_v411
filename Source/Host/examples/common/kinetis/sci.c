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
*
* Comments:  This file includes operation of serial communication interface.
*
*
*END************************************************************************/
#include "sci.h"
#include "psptypes.h"
#include "usb_bsp.h"
#include "derivative.h"

char   buff[200];
uint_32 buff_index;

#ifndef MCU_MK70F12
//#define SYSCLK 96000
#define SYSCLK 48000
#else
#define SYSCLK 60000
#endif
#define BAUDRATE 115200

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : sci1_init
* Returned Value   :
* Comments         : This function initilizes the SCI 1 baud rate.
*    
*
*END*----------------------------------------------------------------------*/
void sci1_init(void) 
{
	register uint_16 ubd;
	int periph_clk_khz;
	
    /* Enable all of the port clocks. These have to be enabled to configure
     * pin muxing options, so most code will need all of these on anyway.
     */
	
	periph_clk_khz = SYSCLK / (((SIM_CLKDIV1 & SIM_CLKDIV1_OUTDIV2_MASK) >> 24)+ 1);
	
    SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK
                  | SIM_SCGC5_PORTB_MASK
                  | SIM_SCGC5_PORTC_MASK
                  | SIM_SCGC5_PORTD_MASK
                  | SIM_SCGC5_PORTE_MASK );
	
	/* Enable the UART3_TXD function on PTC17 */
	PORTC_PCR17 = PORT_PCR_MUX(0x3); /* UART is alt3 function for this pin */

	/* Enable the UART3_RXD function on PTC16 */
	PORTC_PCR16 = PORT_PCR_MUX(0x3); /* UART is alt3 function for this pin */
	
	SIM_SCGC4 |= SIM_SCGC4_UART3_MASK;
	
	UART3_C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK );
	UART3_C1 = 0;
	
	ubd = (uint_16)((periph_clk_khz*1000)/(BAUDRATE * 16));
	
	UART3_BDH |= UART_BDH_SBR(((ubd & 0x1F00) >> 8));
	UART3_BDL = (uint_8)(ubd & UART_BDL_SBR_MASK);
	
	UART3_C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK );
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : sci2_init
* Returned Value   :
* Comments         : This function initilizes the SCI 2 module.
*    
*
*END*----------------------------------------------------------------------*/
void sci2_init(){
#ifdef MCU_MK70F12
	register uint_16 sbr, brfa;
	uint_8 temp;

	/* Enable the UART2_TXD function on PTE16 */
	PORTE_PCR16 = PORT_PCR_MUX(0x3); // UART2 is alt3 function for this pin

	/* Enable the UART2_RXD function on PTE17 */
	PORTE_PCR17 = PORT_PCR_MUX(0x3); // UART2 is alt3 function for this pin

	/* Enable the clock  */ 
	SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;

	/* Make sure that the transmitter and receiver are disabled while we 
	 * change settings.
	 */
	UART_C2_REG(UART2_BASE_PTR) &= ~(UART_C2_TE_MASK
			| UART_C2_RE_MASK );

	/* Configure the UART for 8-bit mode, no parity */
	UART_C1_REG(UART2_BASE_PTR) = 0;	/* We need all default settings, so entire register is cleared */

	/* Calculate baud settings */
	sbr = (uint_16)((SYSCLK*1000)/(BAUDRATE * 16));

	/* Save off the current value of the UARTx_BDH except for the SBR field */
	temp = UART_BDH_REG(UART2_BASE_PTR) & ~(UART_BDH_SBR(0x1F));

	UART_BDH_REG(UART2_BASE_PTR) = temp |  UART_BDH_SBR(((sbr & 0x1F00) >> 8));
	UART_BDL_REG(UART2_BASE_PTR) = (uint_8)(sbr & UART_BDL_SBR_MASK);

	/* Determine if a fractional divider is needed to get closer to the baud rate */
	brfa = (((SYSCLK*32000)/(BAUDRATE * 16)) - (sbr * 32));

	/* Save off the current value of the UARTx_C4 register except for the BRFA field */
	temp = UART_C4_REG(UART2_BASE_PTR) & ~(UART_C4_BRFA(0x1F));

	UART_C4_REG(UART2_BASE_PTR) = temp |  UART_C4_BRFA(brfa);    

	/* Enable receiver and transmitter */
	UART_C2_REG(UART2_BASE_PTR) |= (UART_C2_TE_MASK
			| UART_C2_RE_MASK );
#endif
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : TERMIO_PutChar
* Returned Value   :
* Comments         :
*                     This function sends a char via SCI.
*
*END*----------------------------------------------------------------------*/
#ifdef __CC_ARM
int sendchar(int ch)
#else
void uart_putchar (char ch)
#endif
{
#ifndef MCU_MK70F12
    /* Wait until space is available in the FIFO */
    while(!(UART3_S1 & UART_S1_TDRE_MASK)){};
    
    /* Send the character */
    UART3_D = (uint_8)ch;
#else
    /* Wait until space is available in the FIFO */
    while(!(UART2_S1 & UART_S1_TDRE_MASK)){};

    /* Send the character */
    UART2_D = (uint_8)ch;
#endif
 }

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : TERMIO_GetChar
* Returned Value   : the char get via SCI
* Comments         :
*                     This function gets a char via SCI.
*
*END*----------------------------------------------------------------------*/
#ifdef __CC_ARM
int getkey(void)
#else
char uart_getchar (void)
#endif 
{
#ifdef MCU_MK70F12
	/* Wait until character has been received */
	while (!(UART2_S1 & UART_S1_RDRF_MASK));

	/* Return the 8-bit data from the receiver */
	return UART2_D;
#else
    /* Wait until character has been received */
    while (!(UART3_S1 & UART_S1_RDRF_MASK)){};
    
    /* Return the 8-bit data from the receiver */
    return UART3_D;
#endif
}

/********************************************************************/
void out_char (char ch)
{
#ifndef __CC_ARM
	uart_putchar(ch);
#endif 
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : sci2_PutChar 
* Returned Value   :
* Comments         : send a character through UART2
*    
*
*END*----------------------------------------------------------------------*/
void sci2_PutChar(char send)
{
	UNUSED(send);
}

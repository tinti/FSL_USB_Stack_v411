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
#include "types.h"
#include "derivative.h"

char   buff[200];
uint_32 buff_index;

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : sci1_init
* Returned Value   :
* Comments         : This function initilizes the SCI 1 baud rate.
*    
*
*END*----------------------------------------------------------------------*/
#define SYSCLK 48000
#define BAUDRATE 115200
void sci_init(void) 
{
#if defined MCU_MK21D5
	register uint_16 ubd;
	int system_clk_khz;

	/* Calculate the system clock value expressed in kHz */
	system_clk_khz = SYSCLK / (((SIM_CLKDIV1 & SIM_CLKDIV1_OUTDIV1_MASK) >> 28)+ 1);
	
	/* Enable all of the port clocks. These have to be enabled to configure
     * pin muxing options, so most code will need all of these on anyway.
     */    
    SIM_SCGC5 |= (	SIM_SCGC5_PORTA_MASK
                  | SIM_SCGC5_PORTB_MASK
                  | SIM_SCGC5_PORTC_MASK
                  | SIM_SCGC5_PORTD_MASK
                  | SIM_SCGC5_PORTE_MASK );

    /* Enable the UART2_TXD function on PTE16 */
    PORTE_PCR16 = PORT_PCR_MUX(0x03);

    /* Enable the UART2_RXD function on PTE17 */
    PORTE_PCR17 = PORT_PCR_MUX(0x03);

    SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;

    UART2_C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK );
    UART2_C1 = 0;

    ubd = (uint_16)((system_clk_khz * 1000)/(BAUDRATE * 16));

	UART2_BDH |= UART_BDH_SBR(((ubd & 0x1F00) >> 8));
	UART2_BDL = (uint_8)(ubd & UART_BDL_SBR_MASK);

    UART2_C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK );
#elif(defined MCU_MKL25Z4)
	register uint_16 ubd;
	int system_clk_khz;

	/* Calculate the system clock value expressed in kHz */
	system_clk_khz = SYSCLK / (((SIM_CLKDIV1 & SIM_CLKDIV1_OUTDIV1_MASK) >> 28)+ 1);
	
	SIM_SOPT2 |= SIM_SOPT2_UART0SRC(1);
	
	/* Enable all of the port clocks. These have to be enabled to configure
     * pin muxing options, so most code will need all of these on anyway.
     */    
    SIM_SCGC5 |= (	SIM_SCGC5_PORTA_MASK
                  | SIM_SCGC5_PORTB_MASK
                  | SIM_SCGC5_PORTC_MASK
                  | SIM_SCGC5_PORTD_MASK
                  | SIM_SCGC5_PORTE_MASK );
	
	/* Enable the UART2_TXD function on PTA14 */
	PORTA_PCR14 = PORT_PCR_MUX(0x03); /* UART is alt3 function for this pin */

	/* Enable the UART2_RXD function on PTA15 */
	PORTA_PCR15 = PORT_PCR_MUX(0x03); /* UART is alt3 function for this pin */
	
	
	SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;
	
	UART0_C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK );
	UART0_C1 = 0;
	
	ubd = (uint_16)((system_clk_khz * 1000)/(BAUDRATE * 16));
	
	UART0_BDH = UART_BDH_SBR(((ubd & 0x1F00) >> 8));
	UART0_BDL = (uint_8)(ubd & UART_BDL_SBR_MASK);
	UART0_C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK );
#else
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
	PORTC_PCR17 = PORT_PCR_MUX(0x3); // UART is alt3 function for this pin

	/* Enable the UART3_RXD function on PTC16 */
	PORTC_PCR16 = PORT_PCR_MUX(0x3); // UART is alt3 function for this pin
	
	SIM_SCGC4 |= SIM_SCGC4_UART3_MASK;
	
	UART3_C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK );
	UART3_C1 = 0;
	
	ubd = (uint_16)((periph_clk_khz*1000)/(BAUDRATE * 16));
	
	UART3_BDH |= UART_BDH_SBR(((ubd & 0x1F00) >> 8));
	UART3_BDL = (uint_8)(ubd & UART_BDL_SBR_MASK);
	
	UART3_C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK );
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
void uart_putchar (char ch)
{
#if (defined MCU_MK21D5)
    /* Wait until space is available in the FIFO */
    while(!(UART2_S1 & UART_S1_TDRE_MASK)){};

    /* Send the character */
    UART2_D = (uint_8)ch;	
#elif(defined MCU_MKL25Z4)
    /* Wait until space is available in the FIFO */
    while(!(UART0_S1 & UART_S1_TDRE_MASK)){};

    /* Send the character */
    UART0_D = (uint_8)ch;	
#else   
    /* Wait until space is available in the FIFO */
    while(!(UART3_S1 & UART_S1_TDRE_MASK)){};
    
    /* Send the character */
    UART3_D = (uint_8)ch;
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
char uart_getchar (void)
{
#if (defined MCU_MK21D5)
    while (!(UART2_S1 & UART_S1_RDRF_MASK));
    
    /* Return the 8-bit data from the receiver */
    return UART2_D;	
#elif(defined MCU_MKL25Z4)
    while (!(UART0_S1 & UART_S1_RDRF_MASK));

    /* Return the 8-bit data from the receiver */
    return UART0_D;	
#else
    /* Wait until character has been received */
    while (!(UART3_S1 & UART_S1_RDRF_MASK));
    
    /* Return the 8-bit data from the receiver */
    return UART3_D;
#endif
}

/********************************************************************/
void
out_char (char ch)
{
	uart_putchar(ch);
}

void sci2_init(void) 
{

}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : sci2_PutChar 
* Returned Value   :
* Comments         : send a charactor through UART2
*    
*
*END*----------------------------------------------------------------------*/
void sci2_PutChar(char send)
{

}

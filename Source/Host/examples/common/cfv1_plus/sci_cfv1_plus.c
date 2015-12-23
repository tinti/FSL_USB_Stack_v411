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
 * @file sci_cfv1_plus.c
 *
 * @author
 *
 * @version
 *
 * @date    
 *
 * @brief   This file configures SCI module 
 * 
 *****************************************************************************/#include "types.h"
#include "sci.h"
#include "derivative.h"
#include "sci_cfv1_plus.h"

#include <Uart.h>


char   buff[200];
uint_32 buff_index;

void interrupt 93 UART2RX_ISR(void);


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : sci_init
* Returned Value   :
* Comments         : This function initilizes the SCI 1 baud rate.
*    
*
*END*----------------------------------------------------------------------*/
void sci1_init(void) 
{
	/* Enable clock source for UART0 */
	SIM_SCGC1 |= SIM_SCGC1_UART0_MASK;
	/* Set Pins for UART0 */
	MXC_PTAPF1 &= ~MXC_PTAPF1_A7(0xF);
	MXC_PTAPF1 |= MXC_PTAPF1_A7(0x2);
	MXC_PTDPF1 &= ~MXC_PTDPF1_D6(0xF);
	MXC_PTDPF1 |= MXC_PTDPF1_D6(0x2);
    /* UART Configuration */
    UART0_BDH = 0x00;
    UART0_BDL = UART_BDL_SBR(0xD0);
                
    UART0_C1 = 0x0;
    UART0_C4  = UART_C4_BRFA(12);
        
    /* Enable receiver and transmitter */ 
    UART0_C2  = UART_C2_RE_MASK | UART_C2_TE_MASK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : TERMIO_PutChar
* Returned Value   :
* Comments         :
*                     This function sends a char via SCI.
*
*END*----------------------------------------------------------------------*/
void TERMIO_PutChar(char send) 
{
	char dummy;
	/* Wait until transmitter is idle */
	while(!(UART0_S1 & UART_S1_TC_MASK)){};
	dummy = UART0_S1;
	/* Send the character */
	UART0_D  = send;    
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : TERMIO_GetChar
* Returned Value   : the char get via SCI
* Comments         :
*                     This function gets a char via SCI.
*
*END*----------------------------------------------------------------------*/
char TERMIO_GetChar(void) 
{
	char dummy;
	
	while(!(UART0_S1 & UART_S1_RDRF_MASK)){};

	dummy = UART0_S1;
	return UART0_D; 
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : Initialize UART
* Returned Value   :
* Comments         :
*                    This function initializes the UART for console printf/scanf
*
*END*----------------------------------------------------------------------*/
UARTError InitializeUART(UARTBaudRate baudRate)
{
	uint_32 	baud_divisor;
	uint_32	fractional_fine_adjust;
	/* Calculate baud settings */
	/* UART baud rate = UART module clock / (16 × (SBR[SBR] +BRFD)) */
	fractional_fine_adjust = (uint_32)(SYSTEM_CLOCK/(baudRate/2));
	baud_divisor = (uint_32)(fractional_fine_adjust >>5);
	fractional_fine_adjust -= (baud_divisor <<5);
	if (baud_divisor > 0x1fff) 
	{
		return kUARTUnknownBaudRate;
	}
	/* Set baudrate */
	UART0_BDH |= ((baud_divisor>>8) & UART_BDH_SBR_MASK);
	UART0_BDL = (uint_8)(baud_divisor & 0xFF);
	/* 8 bits data, no parity */
	UART0_C1 = 0x0;
	
	UART0_C4  |= (UART_C4_BRFA_MASK & fractional_fine_adjust);
	
	/* Enable receiver and transmitter */ 
	UART0_C2  |= UART_C2_RE_MASK | UART_C2_TE_MASK;

 return kUARTNoError;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : WriteUARTN
* Returned Value   :
* Comments         :
*                    This function writes N bytes on the SCI1 console output
*
*END*----------------------------------------------------------------------*/
UARTError WriteUARTN(const void* bytes, unsigned long length) 
{
  int i;
  char* src = (char*)bytes;
  
  for(i = 0; i< length; i++) 
  {
   TERMIO_PutChar(*src++);
  }

 return kUARTNoError;  
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : ReadUARTN
* Returned Value   :
* Comments         :
*                    This function reads N bytes on the SCI1 console input
*
*END*----------------------------------------------------------------------*/
UARTError ReadUARTN(void* bytes, unsigned long length)
{
  int i;
  char *dst = (char*)bytes;
  
  for(i = 0; i< length; i++) 
  {
   *dst++ = TERMIO_GetChar();
  }
  
  return kUARTNoError;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : TERMIO_GetCharNB
* Returned Value   : the char get via SCI
* Comments         :
*                     This function gets a char via SCI. This is the non blocking version 
*                     of the TERMIO_GetChar. if the caracter is not available, this function returns 0.     
*
*END*----------------------------------------------------------------------*/
char TERMIO_GetCharNB(void)  
{
  char dummy;
  
  if((UART0_S1 & UART_S1_RDRF_MASK))
  {    
   dummy = (char)UART0_S1;
   return (char)UART0_D;
  }
  else
  {    
   return 0; 
  }
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : sci2_init
* Returned Value   :
* Comments         : This function initializes the SCI2 interrupt.
*    
*
*END*----------------------------------------------------------------------*/
void sci2_init(void) 
{
	/* Enable clock source for UART0 */
	SIM_SCGC1 |= SIM_SCGC1_UART1_MASK;
	
	/* Set Pins for UART0 */
	MXC_PTFPF1 &= ~MXC_PTFPF1_F6(0xF);
	MXC_PTFPF1 |= MXC_PTFPF1_F6(0x2);
	MXC_PTFPF2 &= ~MXC_PTFPF2_F5(0xF);
	MXC_PTFPF2 |= MXC_PTFPF2_F5(0x2);	

	/* Set baudrate */
	UART1_BDH &= ~UART_BDH_SBR_MASK;
	UART1_BDL = 0x1A;
	/* 8 bits data, no parity */
	UART1_C1 = 0x0;
	
	UART1_C4  |= 0x02;
	
	/* Enable and Receiver Full interrupt */
	UART1_C2  |=  UART_C2_RIE_MASK;
	
	/* Enable receiver and transmitter */ 
	UART1_C2  |= UART_C2_RE_MASK | UART_C2_TE_MASK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : SCI2RX_ISR
* Returned Value   :
* Comments         : Service interrupt in SCI2 hardware.
*    
*
*END*----------------------------------------------------------------------*/
void interrupt 93 UART2RX_ISR(void) 
{  
  char tmp, dummy;
  tmp = (char)UART1_D;
  if (UART1_S1 & UART_S1_RDRF_MASK){
    dummy = (char)UART1_S1;
    buff[buff_index] = tmp;
    buff_index ++;
    if(200 == buff_index) {
      buff_index = 0;
      printf("String too length, some data may be lost !");
    }
    
  }
  
  
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : sci2_PutChar
* Returned Value   :
* Comments         :
*                     This function sends a char via SCI.
*
*END*----------------------------------------------------------------------*/
void sci2_PutChar(char send) 
{
	char dummy;
	/* Wait until transmitter is idle */
	while(!(UART1_S1 & UART_S1_TC_MASK)){};
	dummy = UART1_S1;
	/* Send the character */
	UART1_D  = send;    
}




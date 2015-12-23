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
* $FileName:	sci2.c
* $Version :
* $Date    :
*
* Comments:
*
*
*END************************************************************************/
#include "sci.h"
#include "psptypes.h"
#include "derivative.h"
#include "Int_Ctl_cfv2.h"
#include "uart_support.h"




__declspec(interrupt) void UART1_ISR(void);
void sci2_init(void);
void sci2_PutChar(char send);
char TERMIO_GetCharNB(void);
char   buff[BUFF_SIZE];
uint_32 buff_index;



/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : sci2_init
* Returned Value   :
* Comments         : Initialize timer module
*    
*
*END*----------------------------------------------------------------------*/
void sci2_init(void) 
{
  uart_init(1, SYSTEM_CLOCK_KHZ, kBaud115200);
  MCF_UART1_UIMR = MCF_UART_UIMR_FFULL_RXRDY;
  Int_Ctl_int_init(UART1_INT_CNTL , UART1_ISR_SRC , 1 , 1 , TRUE );	
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : UART1_ISR 
* Returned Value   :
* Comments         : Timer interrupt service routine
*    
*
*END*----------------------------------------------------------------------*/
__declspec(interrupt) void UART1_ISR(void)
{
 char tmp;
 
if(MCF_UART_USR(1) & MCF_UART_USR_RXRDY) 
  {
    tmp = (char)MCF_UART_URB(1);  	
    buff[buff_index] = tmp;
    buff_index ++;
    if(BUFF_SIZE == buff_index) 
    {
      buff_index = 0;
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
void sci2_PutChar(char send)
{
uart_putchar (1, send);
}
/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : TERMIO_GetCharNB 
* Returned Value   :
* Comments         : get char
*    
*
*END*----------------------------------------------------------------------*/
char TERMIO_GetCharNB(void)
{
char dummy;

      /* Wait until character has been received */
    if ((MCF_UART_USR(1) & MCF_UART_USR_RXRDY)) 
    {
    	 dummy = (char)MCF_UART_USR(1);
    	 return (char)MCF_UART_URB(1);
    }
	else
  	{    
   		 return 0; 
  	}
}
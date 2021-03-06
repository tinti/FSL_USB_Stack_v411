/**HEADER********************************************************************
* 
* Copyright (c) 2004 - 2010 Freescale Semiconductor;
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
* $FileName: sci_cfv1_plus.h$
* $Version : 
* $Date    : 
*
* Comments:
*
*   
*
*END************************************************************************/
#ifndef __SCI_CFV1_PLUS_H_
#define __SCI_CFV1_PLUS_H_
#define SYSTEM_CLOCK   ((uint_32)48000000)
#define BUS_CLOCK      ((uint_32)(SYSTEM_CLOCK))
#define BAUDRATE		19200

void sci1_init(void) ;
void TERMIO_PutChar(char send);
char TERMIO_GetChar(void);
char TERMIO_GetCharNB(void);
void out_char (char ch);
void sci2_init(void) ;
void sci2_PutChar(char send);
 
#endif

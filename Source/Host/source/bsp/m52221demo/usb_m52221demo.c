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
* $FileName: usb_mcf51JM.c$
* $Version : 3.0.9.0$
* $Date    : Nov-21-2008$
*
* Comments:
*
*   This file contains board-specific initialization functions.
*
*END************************************************************************/
#include "usb_bsp.h"
#include "derivative.h"
#include "Int_Ctl_cfv2.h"
#include "support_common.h"
#include "exceptions_cfv2.h"

void sci1_init(void);
/***************************************************************************
 * Local Functions Prototypes 
 ***************************************************************************/

/***************************************************************************
 * Local Functions 
 ***************************************************************************/
extern __declspec(interrupt) void UART1_ISR(void);
/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_pll_init
* Returned Value   : void
* Comments         :
*   
*
*END*----------------------------------------------------------------------*/
static void _bsp_pll_init(void) 
{
         
    /* Initialize the PLL
    ** Divide 48Mhz reference crystal by 6 and multiply by 10 to achieve a 
    ** system clock of 80 Mhz.
    **   
    ** To set an MFD of ‘x’ and an RFD of ‘y’, you must first program RFD to ‘y+1’,
    ** then program MFD to ‘x’, then let the PLL lock, then program RFD to ‘y’. If 
    ** you program RFD simultaneous to MFD, you may over-clock and damage the part.
    */
    
    MCF_CLOCK_OCLR  = 0xf0;
    MCF_CLOCK_CCHR  = 5;
    MCF_CLOCK_SYNCR = MCF_CLOCK_SYNCR_RFD(0) |
                      MCF_CLOCK_SYNCR_MFD(3) |
                      MCF_CLOCK_SYNCR_PLLMODE|
                      MCF_CLOCK_SYNCR_PLLEN;
    

    /* wait for PLL locks before switching clock source */
    while (!(MCF_CLOCK_SYNSR & MCF_CLOCK_SYNSR_LOCK)) {}

    /* now changing clock source is possible */
    MCF_CLOCK_CCLR  = 0;
    MCF_CLOCK_SYNCR |= MCF_CLOCK_SYNCR_CLKSRC;
      
    /* wait for PLL lock again */
    while (!(MCF_CLOCK_SYNSR & MCF_CLOCK_SYNSR_LOCK)) {}
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : scm_init
* Returned Value   : void
* Comments         :
*   
*
*END*----------------------------------------------------------------------*/

static void _bsp_scm_init(void)
{
	/*
	 * Enable on-chip modules to access internal SRAM
	 */
	MCF_SCM_RAMBAR = (0
		| MCF_SCM_RAMBAR_BA(RAMBAR_ADDRESS)
		| MCF_SCM_RAMBAR_BDE);

}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_usb_io_init
* Returned Value   : void
* Comments         :
*   
*
*END*----------------------------------------------------------------------*/

static void _bsp_usb_io_init
(
    void
)
{
  
  // setup gpio state, func and dirrections
    MCF_GPIO_PUAPAR &= 0x3f;
    MCF_GPIO_PORTUA |= MCF_GPIO_PORTUA_PORTUA3;   // turn Vbus off
	MCF_GPIO_DDRUA |= MCF_GPIO_DDRUA_DDRUA3;      // CTS0 as output
	
	MCF_GPIO_PUAPAR &= 0xcf;
	MCF_GPIO_DDRUA &= ~MCF_GPIO_PORTUA_PORTUA2;     // RTS0 as input
	
    //reg_ptr->GPIO.PORTUA |= MCF5225_GPIO_PORTxP3; // turn Vbus off
	MCF_GPIO_PORTUA &= ~MCF_GPIO_PORTUA_PORTUA3;  // turn Vbus on
	
    MCF_GPIO_PQSPAR |= MCF_GPIO_PQSPAR_PQSPAR5(3) | MCF_GPIO_PQSPAR_PQSPAR6(3);
}



/***************************************************************************
 * Global Functions 
 ***************************************************************************/
 
/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_platform_init
* Returned Value   : 0 for success
* Comments         :
*    This function performs BSP-specific related to the MCF51JM platform
*
*END*----------------------------------------------------------------------*/

void _bsp_platform_init(void)
{
  _bsp_scm_init();
  _bsp_pll_init();  
}  

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_usb_host_init
* Returned Value   : 0 for success
* Comments         :
*    This function performs BSP-specific initialization related to USB host
*
*END*----------------------------------------------------------------------*/
int_32 _bsp_usb_host_init(pointer param)
{
   (void)param;
   _bsp_usb_io_init();
   Int_Ctl_int_init(USB_INT_CNTL, USB_ISR_SRC, 2 , 2 , TRUE);
   return 0;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : __initialize_hardware
* Returned Value   : none
* Comments         :
*   
*
*END*----------------------------------------------------------------------*/

void __initialize_hardware(void)
{
	/*******************************************************
	*	Out of reset, the low-level assembly code calls this 
	*	routine to initialize the MCF52259 modules.
	********************************************************/

	asm 
	{
	    /* Initialize IPSBAR */
	    move.l  #__IPSBAR,d0
	       andi.l  #0xC0000000,d0 // need to mask
	    add.l   #0x1,d0
	    move.l  d0,0x40000000

	    

	    /* Initialize FLASHBAR */
	    move.l  #__FLASHBAR,d0
	       andi.l  #0xFFF80000,d0 // need to mask
	    add.l   #0x61,d0
	    movec   d0,FLASHBAR

	}
   	initialize_exceptions();
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : __initialize_hardware
* Returned Value   : none
* Comments         :
*   
*
*END*----------------------------------------------------------------------*/

void sci1_init(void)
{
	
}



#pragma overload __declspec(interrupt) void UART1_ISR(void);
__declspec(interrupt) void UART1_ISR(void)
{
}

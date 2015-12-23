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
 * @file init_hw_52259.c
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief   This file contains the HW initialization for the MC52259 controller
 *****************************************************************************/
 
#include "types.h"      	/* User Defined Data Types */
#include "derivative.h" 	/* include peripheral declarations */
#include "init_hw.h"        /* own header with public declarations */
#include "exceptions_cfv2.h"

static void pll_init(void);
static void scm_init(void);


/*****************************************************************************
 * @name     GPIO_Init
 *
 * @brief:   Initialization of the GPIO.
 *
 * @param  : None
 *
 * @return : None
 *****************************************************************************
 *
 *
 ****************************************************************************/
void GPIO_Init(void)
{
 	/***************************************************************************
 	*						 GPIO Init for LED
 	*
 	****************************************************************************/		
	/* Configure LED io pins to be outputs.
	 * M52259EVB: LED to port mapping
	 * D10  D11  D12  D13
	 *  |    |    |    |
	 *  ^    ^    ^    ^
	 * PTC0 PTC1 PTC2 PTC3 
	 */
	MCF_GPIO_DDRTC = (MCF_GPIO_DDRTC_DDRTC0 | 
					  MCF_GPIO_DDRTC_DDRTC1 | 
					  MCF_GPIO_DDRTC_DDRTC2 | 
					  MCF_GPIO_DDRTC_DDRTC3);
	/* Assign PORTC[0-3] as GPIO */
	MCF_GPIO_PTCPAR = MCF_GPIO_PTCPAR_PTCPAR0(MCF_GPIO_PTCPAR_DTIN0_GPIO) |
	                  MCF_GPIO_PTCPAR_PTCPAR1(MCF_GPIO_PTCPAR_DTIN1_GPIO) |
	                  MCF_GPIO_PTCPAR_PTCPAR2(MCF_GPIO_PTCPAR_DTIN2_GPIO) | 
	                  MCF_GPIO_PTCPAR_PTCPAR3(MCF_GPIO_PTCPAR_DTIN3_GPIO);
    
 	/***************************************************************************
 	*						 GPIO Init for USB
 	*
 	****************************************************************************/		    
    // setup gpio state, func and dirrections
    MCF_GPIO_PUAPAR &= 0x3f;
    MCF_GPIO_PORTUA |= MCF_GPIO_PORTUA_PORTUA3;   // turn Vbus off
	MCF_GPIO_DDRUA 	|= MCF_GPIO_DDRUA_DDRUA3;      // CTS0 as output
	
	MCF_GPIO_PUAPAR &= 0xcf;
	MCF_GPIO_DDRUA 	&= ~MCF_GPIO_DDRUA_DDRUA2;     // RTS0 as input
	
    //reg_ptr->GPIO.PORTUA |= MCF5225_GPIO_PORTxP3; // turn Vbus off
	MCF_GPIO_PORTUA &= ~MCF_GPIO_PORTUA_PORTUA3;  // turn Vbus on
	
    MCF_GPIO_PQSPAR |= MCF_GPIO_PQSPAR_PQSPAR5(3) | MCF_GPIO_PQSPAR_PQSPAR6(3);	                  
    
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
	  	                 
#ifdef __MCF52259_DEMO_ 
	/* M52259Demo: SW1, SW2 connected to PNQ1, PNQ5 */
	/* Configure PNQ1, PNQ5 as input */   	   

	MCF_GPIO_PNQPAR &= ~(MCF_GPIO_PNQPAR_PNQPAR1(3) | MCF_GPIO_PNQPAR_PNQPAR5(3));
	MCF_GPIO_PNQPAR |= (MCF_GPIO_PNQPAR_IRQ1_IRQ1 | MCF_GPIO_PNQPAR_IRQ5_IRQ5); 
	
	/* Configure IRQ[1] & IRQ[5] as input */
	MCF_EPORT_EPDDR &= ~(MCF_EPORT_EPDDR_EPDD1 | MCF_EPORT_EPDDR_EPDD5); 
	/* Configure IRQ1 & IRQ5 as falling-edge sense */
	MCF_EPORT_EPPAR = MCF_EPORT_EPPAR_EPPA1_FALLING | MCF_EPORT_EPPAR_EPPA5_FALLING;
	/* Enable Interrupt for IRQ1 & IRQ5 */
	MCF_EPORT_EPIER = MCF_EPORT_EPIER_EPIE1 | MCF_EPORT_EPIER_EPIE5;  
#endif
	
	
	/***************************************************************************
 	*						 GPIO Init for QSPI
 	*
 	****************************************************************************/    
	/* Configure the pins for QSPI for specialized purpose */
	/* QSPI_CLK, QSPI_DOUT, QSPI_DIN, QSPI_CS0  */
	MCF_GPIO_PQSPAR &= ~(MCF_GPIO_PQSPAR_PQSPAR0(3) | 
	                	 MCF_GPIO_PQSPAR_PQSPAR1(3) | 
	                	 MCF_GPIO_PQSPAR_PQSPAR2(3) | 
	                	 MCF_GPIO_PQSPAR_PQSPAR3(3));  
	                	 
	MCF_GPIO_PQSPAR |= (MCF_GPIO_PQSPAR_QSPI_DOUT_DOUT |
				        MCF_GPIO_PQSPAR_QSPI_DIN_DIN   |
	  				    MCF_GPIO_PQSPAR_QSPI_CLK_CLK   |
	  				    MCF_GPIO_PQSPAR_QSPI_CS0_CS0); 
}

/*****************************************************************************
 * @name     __initialize_hardware
 *
 * @brief:   Called from startcf_cfv2.
 *
 * @param  : None
 *
 * @return : None
 *****************************************************************************
 *
 *
 ****************************************************************************/
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
    
    pll_init();
	scm_init();	
	
	initialize_exceptions();	
}

/********************************************************************/
void pll_init(void)
{
	  MCF_CLOCK_OCLR = 0xF0;
      MCF_CLOCK_CCHR = 0x05;
      MCF_CLOCK_SYNCR = 
      MCF_CLOCK_SYNCR_RFD(0) |
      MCF_CLOCK_SYNCR_MFD(3) | 
      MCF_CLOCK_SYNCR_PLLMODE | 
      MCF_CLOCK_SYNCR_PLLEN ;
      
    /* wait for PLL locks before switching clock source */
    while (!(MCF_CLOCK_SYNSR & MCF_CLOCK_SYNSR_LOCK)) {}
    
    MCF_CLOCK_CCLR = 0;
    MCF_CLOCK_SYNCR |= MCF_CLOCK_SYNCR_CLKSRC;
    
    /* wait for PLL lock again */
    while (!(MCF_CLOCK_SYNSR & MCF_CLOCK_SYNSR_LOCK)) {}
}
/********************************************************************/
void scm_init(void)
{
	/************************************************
	 * Enable on-chip modules to access internal SRAM
	 ************************************************/
	MCF_SCM_RAMBAR = (0
		| MCF_SCM_RAMBAR_BA(RAMBAR_ADDRESS)
		| MCF_SCM_RAMBAR_BDE);
}

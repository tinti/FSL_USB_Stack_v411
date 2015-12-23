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
* $FileName: P0.c$
* $Version :
* $Date    :
*
* Comments:
*
*   This file contains board-specific initialization functions.
*
*END************************************************************************/
#include "usb_bsp.h"
#include "derivative.h"

#define BSP_CLOCK_SRC                   (8000000ul)       // crystal, oscillator freq
#define BSP_REF_CLOCK_SRC               (2000000ul)       // must be 2-4MHz

#ifdef MCGOUTCLK_72_MHZ
	#define BSP_CORE_DIV                    (1)
	#define BSP_BUS_DIV                     (2)
	#define BSP_FLEXBUS_DIV                 (2)
	#define BSP_FLASH_DIV                   (4)

	// BSP_CLOCK_MUL from interval 24 - 55
	#define BSP_CLOCK_MUL                   (36)    // 72MHz
#else
	#define BSP_CORE_DIV                    (1)
	#define BSP_BUS_DIV                     (1)
	#define BSP_FLEXBUS_DIV                 (1)
	#define BSP_FLASH_DIV                   (2)

	// BSP_CLOCK_MUL from interval 24 - 55
	#define BSP_CLOCK_MUL                   (24)    // 48MHz
#endif

#define BSP_REF_CLOCK_DIV               (BSP_CLOCK_SRC / BSP_REF_CLOCK_SRC)

#define BSP_CLOCK                       (BSP_REF_CLOCK_SRC * BSP_CLOCK_MUL)
#define BSP_CORE_CLOCK                  (BSP_CLOCK / BSP_CORE_DIV)          // CORE CLK, max 100MHz
#define BSP_SYSTEM_CLOCK                (BSP_CORE_CLOCK)                    // SYSTEM CLK, max 100MHz
#define BSP_BUS_CLOCK                   (BSP_CLOCK / BSP_BUS_DIV)       	// max 50MHz
#define BSP_FLEXBUS_CLOCK               (BSP_CLOCK / BSP_FLEXBUS_DIV)
#define BSP_FLASH_CLOCK                 (BSP_CLOCK / BSP_FLASH_DIV)     	// max 25MHz

/* Use the uUSB connector on the board */
#define USE_MICRO_USB					(FALSE)					

/***************************************************************************
 * Local Functions
 ***************************************************************************/

/*****************************************************************************
 * @name     wdog_disable
 *
 * @brief:   Disable watchdog.
 *
 * @param  : None
 *
 * @return : None
 *****************************************************************************
 * It will disable watchdog.
  ****************************************************************************/
static void wdog_disable(void)
{
	/* Write 0xC520 to the unlock register */
	WDOG_UNLOCK = 0xC520;
	
	/* Followed by 0xD928 to complete the unlock */
	WDOG_UNLOCK = 0xD928;
	
	/* Clear the WDOGEN bit to disable the watchdog */
	WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}

/*****************************************************************************
 * @name     pll_init
 *
 * @brief:   Initialization of the MCU.
 *
 * @param  : None
 *
 * @return : None
 *****************************************************************************
 * It will configure the MCU to disable STOP and COP Modules.
 * It also set the MCG configuration and bus clock frequency.
 ****************************************************************************/
static unsigned char pll_init()
{
	/* First move to FBE mode */
	/* Enable external oscillator, RANGE=1, HGO=1, EREFS=1, LP=0, IRCS=0 */
	MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_HGO0_MASK | MCG_C2_EREFS0_MASK | MCG_C2_IRCS_MASK;
	
    /* Select external oscillator and Reference Divider and clear IREFS to start ext osc
	   CLKS=2, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0 */
    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(3);

	/* Wait for oscillator to initialize */
   while (!(MCG_S & MCG_S_OSCINIT0_MASK)){};

   	/* Wait for Reference clock Status bit to clear */
    while (MCG_S & MCG_S_IREFST_MASK){};

    /* Wait for clock status bits to show clock source is ext ref clk */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2){};

    MCG_C5 = MCG_C5_PRDIV0(BSP_REF_CLOCK_DIV - 1) | MCG_C5_PLLCLKEN0_MASK;

    /* Ensure MCG_C6 is at the reset default of 0. LOLIE disabled,
     PLL enabled, clk monitor disabled, PLL VCO divider is clear */
    MCG_C6 = 0;

    /* Set system options dividers */
	#if (defined MCU_MK20D5) || (defined MCU_MK40D7)
		SIM_CLKDIV1 =   SIM_CLKDIV1_OUTDIV1(BSP_CORE_DIV - 1) | 	/* core/system clock */
						SIM_CLKDIV1_OUTDIV2(BSP_BUS_DIV - 1)  | 	/* peripheral clock; */
						SIM_CLKDIV1_OUTDIV4(BSP_FLASH_DIV - 1);     /* flash clock */
	#else  
		SIM_CLKDIV1 =   SIM_CLKDIV1_OUTDIV1(BSP_CORE_DIV - 1) 	| 	/* Core/system clock */
						SIM_CLKDIV1_OUTDIV2(BSP_BUS_DIV - 1)  	| 	/* Peripheral clock; */
						SIM_CLKDIV1_OUTDIV3(BSP_FLEXBUS_DIV - 1)|  	/* FlexBus clock driven to the external pin (FB_CLK)*/
						SIM_CLKDIV1_OUTDIV4(BSP_FLASH_DIV - 1);     /* Flash clock */
	#endif
		
    /* Set the VCO divider and enable the PLL, LOLIE = 0, PLLS = 1, CME = 0, VDIV = */
    MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV0(BSP_CLOCK_MUL - 24); /* 2MHz * BSP_CLOCK_MUL */

    while (!(MCG_S & MCG_S_PLLST_MASK)){}; 	/* Wait for PLL status bit to set */
    while (!(MCG_S & MCG_S_LOCK0_MASK)){}; 	/* Wait for LOCK bit to set */

    /* Transition into PEE by setting CLKS to 0
    CLKS=0, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0 */
    MCG_C1 &= ~MCG_C1_CLKS_MASK;

    /* Wait for clock status bits to update */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3){};
     
    /* Enable the ER clock of oscillators */
    OSC_CR = OSC_CR_ERCLKEN_MASK | OSC_CR_EREFSTEN_MASK;
    
    /* Now running in PEE Mode */
    SIM_SOPT1 |= SIM_SOPT1_USBREGEN_MASK;
    
    return 0;
} //pll_init

/*****************************************************************************
 *
 *    @name     GPIO_Init
 *
 *    @brief    This function Initializes LED GPIO
 *
 *    @param    None
 *
 *    @return   None
 *
 ****************************************************************************
 * Initializes the GPIO
 ***************************************************************************/
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

extern uint_32 ___VECTOR_RAM[];
void _bsp_platform_init(void)
{
	/* Point the VTOR to the new copy of the vector table */
	SCB_VTOR = (uint_32)___VECTOR_RAM;
		
	NVICICPR2 = (1 << 9);                     // Clear any pending interrupts on USB
	NVICISER2 = (1 << 9);                     // Enable interrupts from USB module
	
    // Clear ACKISO
	if (PMC_REGSC &  PMC_REGSC_ACKISO_MASK)
    	PMC_REGSC |= PMC_REGSC_ACKISO_MASK;
	
	/* init pll */
	pll_init();
              
	#ifdef MCGOUTCLK_72_MHZ
		// USB Freq. Divider (Out. clk. = In. clk * (2/3))
		SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(2) | SIM_CLKDIV2_USBFRAC_MASK;		
	#else
    	// USB Freq. Divider (Out. clk. = In. clk * (1/1))
    	SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(0);     
	#endif 
    
    // MCGPLLCLK clock selected as CLK source    	
    SIM_SOPT2 |= SIM_SOPT2_USBSRC_MASK | SIM_SOPT2_PLLFLLSEL_MASK;
	
    // USB Clock Gating 
	SIM_SCGC4 |= (SIM_SCGC4_USBOTG_MASK);    
    
	#if(USE_MICRO_USB == TRUE)
		/** USB 5V enable */
		SIM_SCGC5 |=  SIM_SCGC5_PORTC_MASK;              // Turn on PTC clocks 
		PORTC_PCR9 = (0 | PORT_PCR_MUX(1));              // Configure PTC9 pin as GPIO
		
		GPIOC_PDDR |=(1<<9);                             // Set as output
		GPIOC_PSOR =(1<<9);
	#endif   
    
    // Weak pull-downs
    USB0_USBCTRL = USB_USBCTRL_PDE_MASK;
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
    return 0;
}






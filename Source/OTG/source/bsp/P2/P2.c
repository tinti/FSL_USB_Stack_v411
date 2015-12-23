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

#if defined(MCU_MK60N512VMD100) || defined(MCU_MK53N512CMD100)
#define BSP_CLOCK_SRC                   (50000000ul)       // crystal, oscillator freq
#else
#define BSP_CLOCK_SRC                   (8000000ul)       // crystal, oscillator freq
#endif
#define BSP_REF_CLOCK_SRC               (2000000ul)       // must be 2-4MHz

#define BSP_CORE_DIV                    (1)
#define BSP_BUS_DIV                     (1)
#define BSP_FLEXBUS_DIV                 (1)
#define BSP_FLASH_DIV                   (2)

// BSP_CLOCK_MUL from interval 24 - 55
#define BSP_CLOCK_MUL                   (24)    // 48MHz

#define BSP_REF_CLOCK_DIV               (BSP_CLOCK_SRC / BSP_REF_CLOCK_SRC)

#define BSP_CLOCK                       (BSP_REF_CLOCK_SRC * BSP_CLOCK_MUL)
#define BSP_CORE_CLOCK                  (BSP_CLOCK / BSP_CORE_DIV)          // CORE CLK, max 100MHz
#define BSP_SYSTEM_CLOCK                (BSP_CORE_CLOCK)                    // SYSTEM CLK, max 100MHz
#define BSP_BUS_CLOCK                   (BSP_CLOCK / BSP_BUS_DIV)       // max 50MHz
#define BSP_FLEXBUS_CLOCK               (BSP_CLOCK / BSP_FLEXBUS_DIV)
#define BSP_FLASH_CLOCK                 (BSP_CLOCK / BSP_FLASH_DIV)     // max 25MHz


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
#if defined(MCU_MK60N512VMD100) || defined(MCU_MK53N512CMD100)
	/* Enable external oscillator, RANGE=0, HGO=, EREFS=, LP=, IRCS= */
	MCG_C2 = 0;
#else
	/* Enable external oscillator, RANGE=2, HGO=1, EREFS=1, LP=0, IRCS=0 */
	MCG_C2 = MCG_C2_RANGE(2) | MCG_C2_HGO_MASK | MCG_C2_EREFS_MASK|MCG_C2_IRCS_MASK;
#endif
	
    /* Select external oscillator and Reference Divider and clear IREFS to start ext osc
	   CLKS=2, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0 */
    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(3);

#if !defined(MCU_MK60N512VMD100) && !defined(MCU_MK53N512CMD100)
	/* Wait for oscillator to initialize */
   while (!(MCG_S & MCG_S_OSCINIT_MASK)){};
#endif

   /* Wait for Reference clock Status bit to clear */
    while (MCG_S & MCG_S_IREFST_MASK){};

    /* Wait for clock status bits to show clock source is ext ref clk */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2){};

#if defined(MCU_MK60N512VMD100) || defined MCU_MK53N512CMD100
	MCG_C5 = MCG_C5_PRDIV(BSP_REF_CLOCK_DIV - 1);
#else
    MCG_C5 = MCG_C5_PRDIV(BSP_REF_CLOCK_DIV - 1) | MCG_C5_PLLCLKEN_MASK;
#endif
    /* Ensure MCG_C6 is at the reset default of 0. LOLIE disabled,
     PLL enabled, clock monitor disabled, PLL VCO divider is clear */
    MCG_C6 = 0;

    /* Set system options dividers */

    SIM_CLKDIV1 =   SIM_CLKDIV1_OUTDIV1(BSP_CORE_DIV - 1) | 	/* core/system clock */
                    SIM_CLKDIV1_OUTDIV2(BSP_BUS_DIV - 1)  | 	/* peripheral clock; */
					SIM_CLKDIV1_OUTDIV3(BSP_FLEXBUS_DIV - 1) |  /* FlexBus clock driven to the external pin (FB_CLK)*/
					SIM_CLKDIV1_OUTDIV4(BSP_FLASH_DIV - 1);     /* flash clock */

    /* Set the VCO divider and enable the PLL, LOLIE = 0, PLLS = 1, CME = 0, VDIV = */
    MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(BSP_CLOCK_MUL - 24); /* 2MHz * BSP_CLOCK_MUL */

    while (!(MCG_S & MCG_S_PLLST_MASK)){}; /* wait for PLL status bit to set */
    while (!(MCG_S & MCG_S_LOCK_MASK)){}; /* Wait for LOCK bit to set */

    /* Transition into PEE by setting CLKS to 0
    CLKS=0, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0 */
    MCG_C1 &= ~MCG_C1_CLKS_MASK;

    /* Wait for clock status bits to update */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3){};

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
 * Intializes the GPIO
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
#if (defined(__CWCC__) || (defined __IAR_SYSTEMS_ICC__) || defined(__CC_ARM))
	extern uint_32 ___VECTOR_RAM[];
#elif defined(__GNUC__)
	extern __cs3_interrupt_vector[];
#endif
void _bsp_platform_init(void)
{
	/* Point the VTOR to the new copy of the vector table */
#if (defined(__CWCC__) || (defined __IAR_SYSTEMS_ICC__) || defined(__CC_ARM))
	SCB_VTOR = (uint_32)___VECTOR_RAM;
#elif defined(__GNUC__)
	SCB_VTOR = (uint_32)__cs3_interrupt_vector;
#endif
	
	
	NVICICPR2 = (1 << 9);                     //Clear any pending interrupts on USB
	NVICISER2 = (1 << 9);                     //Enable interrupts from USB module
	
	/* init pll */
	pll_init();
	
    /* SIM Configuration */
	MPU_CESR=0;

    SIM_SCGC4|=(SIM_SCGC4_USBOTG_MASK);             // USB Clock Gating

    // PLL/FLL selected as CLK source
    SIM_SOPT2 |= SIM_SOPT2_USBSRC_MASK | SIM_SOPT2_PLLFLLSEL_MASK;
    SIM_CLKDIV2=0x02;                               // USB Freq Divider

    // weak pulldowns
    USB0_USBCTRL=0x40;
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






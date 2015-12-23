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
* $FileName: P2.c$
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

/***************************************************************************
 * Definitions
 ***************************************************************************/
#define BSP_CLOCK_SRC                   (8000000ul)       /* crystal, oscillator freq */
#define BSP_REF_CLOCK_SRC               (2000000ul)       /* must be 2-4MHz */
#define BSP_CORE_DIV                    (1)
#define BSP_BUS_DIV                     (1)
#define BSP_FLEXBUS_DIV                 (1)
#define BSP_FLASH_DIV                   (2)

/* BSP_CLOCK_MUL from interval 24 - 55 */
#define BSP_CLOCK_MUL                   (24)              /* 48MHz */
#define BSP_REF_CLOCK_DIV               (BSP_CLOCK_SRC / BSP_REF_CLOCK_SRC)
#define BSP_CLOCK                       (BSP_REF_CLOCK_SRC * BSP_CLOCK_MUL)
#define BSP_CORE_CLOCK                  (BSP_CLOCK / BSP_CORE_DIV)          /* CORE CLK, max 100MHz */
#define BSP_SYSTEM_CLOCK                (BSP_CORE_CLOCK)                    /* SYSTEM CLK, max 100MHz */
#define BSP_BUS_CLOCK                   (BSP_CLOCK / BSP_BUS_DIV)           /* max 50MHz */
#define BSP_FLEXBUS_CLOCK               (BSP_CLOCK / BSP_FLEXBUS_DIV)
#define BSP_FLASH_CLOCK                 (BSP_CLOCK / BSP_FLASH_DIV)         /* max 25MHz */


/***************************************************************************
 * Local Functions
 ***************************************************************************/
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
#if (defined MCU_MKL25Z4)
    SIM_CLKDIV1 = (uint32_t)0x10010000UL; /* Update system prescalers */
    
    /* First FEI must transition to FBE mode
    Enable external oscillator, RANGE=02, HGO=, EREFS=, LP=, IRCS= */
    MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_HGO0_MASK | MCG_C2_EREFS0_MASK | MCG_C2_IRCS_MASK;
    
    /* Select external oscillator and Reference Divider and clear IREFS 
     * to start external oscillator
     * CLKS = 2, FRDIV = 3, IREFS = 0, IRCLKEN = 0, IREFSTEN = 0
     */
    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(3);
    
    /* MCG_C4: DMX32=0,DRST_DRS=0 */
    MCG_C4 &= (uint8_t)~(uint8_t)0xE0U;
    
    /* MCG_C5: ??=0,PLLCLKEN=0,PLLSTEN=1,PRDIV=0x3, external clock reference = 8/4 = 2MHz */
    MCG_C5 = (uint8_t)0x23U;
    
    /* MCG_C6: LOLIE=0,PLLS=0,CME=0,VDIV=0x18 */
    MCG_C6 = (uint8_t)0x18U;
    
    while((MCG_S & MCG_S_IREFST_MASK) != 0x00U) { /* Check that the source of the FLL reference clock is the external reference clock. */
    }
    
    while((MCG_S & 0x0CU) != 0x08U) {    /* Wait until external reference clock is selected as MCG output */
    }
    
    /* Switch to PBE Mode */
    /* MCG_C1: CLKS=2,FRDIV=3,IREFS=0,IRCLKEN=0,IREFSTEN=0 */
    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(3);
    
    /* MCG_C2: ??=0,??=0,RANGE=2,HGO=0,EREFS=0,LP=0,IRCS=1 */
    MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_HGO0_MASK | MCG_C2_EREFS0_MASK | MCG_C2_IRCS_MASK;
    
    /* MCG_C5: ??=0,PLLCLKEN=0,PLLSTEN=1,PRDIV=0x03 */
    MCG_C5 = (uint8_t)0x23U;
    
    /* MCG_C6: LOLIE=0,PLLS=1,CME=0,VDIV=0x18 */
    MCG_C6 = (uint8_t)0x58U;
    while((MCG_S & 0x0CU) != 0x08U) {    /* Wait until external reference clock is selected as MCG output */
    }
    while((MCG_S & MCG_S_LOCK0_MASK) == 0x00U) { /* Wait until locked */
    }
    /* Switch to PEE Mode */
    /* MCG_C1: CLKS=0,FRDIV=3,IREFS=0,IRCLKEN=0,IREFSTEN=0 */
    MCG_C1 = (uint8_t)0x18U;
    
    /* MCG_C2: ??=0,??=0,RANGE=2,HGO=0,EREFS=0,LP=0,IRCS=1 */
    MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_HGO0_MASK | MCG_C2_EREFS0_MASK | MCG_C2_IRCS_MASK;
    
    /* MCG_C5: ??=0,PLLCLKEN=0,PLLSTEN=1,PRDIV=0x03 */
    MCG_C5 = (uint8_t)0x23U;
    
    /* MCG_C6: LOLIE=0,PLLS=1,CME=0,VDIV=0x18 */
    MCG_C6 = (uint8_t)0x58U;
    while((MCG_S & 0x0CU) != 0x0CU) {    /* Wait until output of the PLL is selected */
    }
    /* MCG_C6: CME=1 */
    MCG_C6 |= (uint8_t)0x20U;            /* Enable the clock monitor */
    /*** End of PE initialization code after reset ***/
#endif
    return 0;
} /* End of pll_init */

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
#if((defined USE_PIT0)||(defined USE_PIT1))
#ifdef USE_PIT0
extern void Timer_ISR(void);
#endif
#ifdef USE_PIT0
extern void PIT0_ISR(void);
#endif
/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : PIT_ISR
* Returned Value   : 
* Comments         :
*    Services Programmable Interrupt Timer PIT
*
*END*----------------------------------------------------------------------*/
void PIT_ISR(void)
{
#ifdef USE_PIT0
    if(PIT_TFLG0 & PIT_TFLG_TIF_MASK)
        Timer_ISR();
#endif
#ifdef USE_PIT1
    if(PIT_TFLG1 & PIT_TFLG_TIF_MASK)
        PIT1_ISR();
#endif
}
#endif

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_platform_init
* Returned Value   : 0 for success
* Comments         :
*    This function performs BSP-specific related to the MCF51JM platform
*
*END*----------------------------------------------------------------------*/
#if (defined(__CWCC__) || (defined __IAR_SYSTEMS_ICC__) || defined(__CC_ARM)|| defined(__arm__))
extern uint_32 ___VECTOR_RAM[];
#endif
void _bsp_platform_init(void)
{
    /* Point the VTOR to the new copy of the vector table */
#if (defined(__CWCC__) || (defined __IAR_SYSTEMS_ICC__) || defined(__CC_ARM)|| defined(__arm__))
    SCB_VTOR = (uint_32)___VECTOR_RAM;
#endif
    
#if (defined MCU_MKL25Z4)
    NVIC_ICPR = (1 << 24);     /* Clear any pending interrupts on USB */
    NVIC_ISER = (1 << 24);     /* Enable interrupts from USB module */    
#endif
    
    /* Initialize PLL */
    pll_init();
    
#if !(defined MCU_MKL25Z4) 
    /* SIM Configuration */
    MPU_CESR=0x00;
#endif
   
    /* USB Clock Gating */
    SIM_SCGC4 |= (SIM_SCGC4_USBOTG_MASK);       
                            
    /* PLL/FLL selected as CLK source */
    SIM_SOPT2 |= SIM_SOPT2_USBSRC_MASK | SIM_SOPT2_PLLFLLSEL_MASK;
 
#ifndef MCU_MKL25Z4
    /* USB Frequency Divider */
    SIM_CLKDIV2 = 0x02;       
#endif
    
    /* Weak pull downs */
    USB0_USBCTRL = 0x40;
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






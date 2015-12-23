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
* $FileName: usb_mcf51JF.c$
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
#include "hidef.h" /* for EnableInterrupts macro */
/***************************************************************************
 * Local variables
 ***************************************************************************/
int core_clk_khz;
int core_clk_mhz;

enum clk_option
{
  PLL50,
  PLL100,
  PLL96,
  PLL48
};

enum clkout_select
{
  CLK_DIS,
  OSC1,
  OSC2,
  MCG_OUT,
  CPU_CLK,
  BUS_CLK,
  LPO_CLK,
  LPTMR0
};

enum crystal_val
{
  XTAL2,
  XTAL4,
  XTAL6,
  XTAL8,
  XTAL10,
  XTAL12,
  XTAL14,
  XTAL16,
  XTAL18,
  XTAL20,
  XTAL22,
  XTAL24,
  XTAL26,
  XTAL28,
  XTAL30,
  XTAL32
};

/***************************************************************************
 * Static Functions  
 ***************************************************************************/
/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : pll_init
* Returned Value : none
* Comments       :
*     This function initializes pll clock
*
*END*--------------------------------------------------------------------*/
static unsigned char pll_init(unsigned char clk_option, unsigned char crystal_val)
{
  unsigned char mcg_freq;
  /*return 0 if one of the available options is not selected */
  if (clk_option > 3) {return 0;} 
  /* return 1 if one of the available crystal options is not available */
  if (crystal_val > 15) {return 1;} 
 /* This assumes that the MCG is in default FEI mode out of reset. */

/* First move to FBE mode */
#if (defined(CANNED_OSC) || defined(EXT_CLK))
     MCG_C2 = 0;
#else
/* Enable external oscillator, RANGE=1, HGO=1, EREFS=1, LP=0, IRCS=0 */
    MCG_C2 = MCG_C2_RANGE(1) | MCG_C2_HGO_MASK | MCG_C2_EREFS_MASK;
#endif

    /* clear ACKISO before setting up the oscillator */
    PMC_REGSC |= PMC_REGSC_ACKISO_MASK;
    
/* Select external oscillator and Reference Divider and clear IREFS to start ext osc */
/* CLKS=2, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0 */
  MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(3);

  /* if we aren't using an osc input we don't need to wait for the osc to init */
#if (!defined(CANNED_OSC) && !defined(EXT_CLK))
    while (!(MCG_S & MCG_S_OSCINIT_MASK)){};  /* wait for oscillator to initialize */
#endif
  /* wait for Reference clock Status bit to clear */
  while (MCG_S & MCG_S_IREFST_MASK){}; 
  /* Wait for clock status bits to show clock source is ext ref clk */
  while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2){}; 

/* Now in FBE

 Configure PLL Ref Divider, PLLCLKEN=0, PLLSTEN=0, PRDIV=5
 The crystal frequency is used to select the PRDIV value. Only even frequency crystals are supported
 that will produce a 2MHz reference clock to the PLL. */
 /* Set PLL ref divider to match the crystal used */
  MCG_C5 = MCG_C5_PRDIV(crystal_val); 

  /* Ensure MCG_C6 is at the reset default of 0. LOLIE disabled, PLL disabled, clk monitor disabled, PLL VCO divider is clear */
  MCG_C6 = 0x0;
 
 /* Select the PLL VCO divider and system clock dividers depending on clocking option */
  switch (clk_option) {
    case 0:
      /* Set system options dividers */
      /*MCG=PLL, core = MCG */
      SIM_CLKDIV0 =  SIM_CLKDIV0_OUTDIV(0);
      /* Set the VCO divider and enable the PLL for 50MHz, LOLIE=0, PLLS=1, CME=0, VDIV=1 */
	  /* VDIV = 1 (x25) */
      MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(1); 
      mcg_freq = 50;
      break;
   case 1:
      /* Set system options dividers */
      /*MCG=PLL, core = MCG/2 */
	  SIM_CLKDIV0 =  SIM_CLKDIV0_OUTDIV(1);
      /* Set the VCO divider and enable the PLL for 100MHz, LOLIE=0, PLLS=1, CME=0, VDIV=26 */
	  /*VDIV = 26 (x50) */
      MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(26); 
      mcg_freq = 50;
      break;
    case 2:
      /* Set system options dividers */
      /* MCG=PLL, core = MCG/2 */
      SIM_CLKDIV0 =  SIM_CLKDIV0_OUTDIV(1);
      /* Set the VCO divider and enable the PLL for 96MHz, LOLIE=0, PLLS=1, CME=0, VDIV=24 */
	  /* VDIV = 24 (x48) */
      MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(24); 
      mcg_freq = 48;
      break;
   case 3:
      /* Set system options dividers */
      /* MCG=PLL, core = MCG */
	  SIM_CLKDIV0 =  SIM_CLKDIV0_OUTDIV(0);
      /* Set the VCO divider and enable the PLL for 48MHz, LOLIE=0, PLLS=1, CME=0, VDIV=0 */
	  /*VDIV = 0 (x24) */
      MCG_C6 = MCG_C6_PLLS_MASK; 
      mcg_freq = 48;
      break;
  }
  
  /* wait for PLL status bit to set */
  while (!(MCG_S & MCG_S_PLLST_MASK)){}; 
  /* Wait for LOCK bit to set */
  while (!(MCG_S & MCG_S_LOCK_MASK)){}; 

/* Now running PBE Mode */

/* Transition into PEE by setting CLKS to 0 */
/* CLKS=0, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0 */
  MCG_C1 &= ~MCG_C1_CLKS_MASK;

/* Wait for clock status bits to update */
  while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3){};


return mcg_freq;
} //pll_init

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : clkout_init
* Returned Value : none
* Comments       :
*     Initialize clock out
*
*END*--------------------------------------------------------------------*/
static void clkout_init(unsigned char clockout_select, unsigned char clkoutdiv)
{
	/* Set the trace clock to the core clock frequency */
	/* should add test of clkoutdiv to check it is is range 0 to 7 */
	SIM_CLKOUT = SIM_CLKOUT_CLKOUTDIV(clkoutdiv)|SIM_CLKOUT_CS(clockout_select);
        	
	/* Enable CLKOUT on PTA5 */
	MXC_PTAPF2 = MXC_PTAPF2_A5(6);
	/* enable high drive strength to support high toggle rate */
	PCTLA_DS |= 0x20;			
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : fb_clk_init
* Returned Value : none
* Comments       :
*     
*
*END*--------------------------------------------------------------------*/
static void fb_clk_init(void)
{
	/* Enable the clock to the FlexBus module */
    SIM_SCGC5 |= SIM_SCGC5_MFBUS_MASK;

 	/* Enable the FB_CLKOUT function on PTB1 (alt7 function) */
	
    MXC_PTBPF4 = MXC_PTBPF4_B1(7);
	/* enable high drive strength to support high toggle rate enable high drive strength to support high toggle rate */
    PCTLB_DS |= 0x02;
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : init_hw
* Returned Value : none
* Comments       :
*     
*
*END*--------------------------------------------------------------------*/
static void init_hw(void)
{
	EnableInterrupts;
	
	/* Disable Watchdog Timer */
	SIM_COPC = SIM_COPC_COPT(0);


	/* Enable Clocks of all the modules */
	SIM_SCGC1 = 0xFF;
	SIM_SCGC2 = 0xFF;
	SIM_SCGC3 = 0xFF;
	SIM_SCGC4 = 0xFF;
	SIM_SCGC5 = 0xFF;
	SIM_SCGC6 = 0xFF; 	
	/* To avoid Multilink issue */
	PCTLB_DS = 0x10; 
	
	/* select CLKOUT source and division factor */
	clkout_init(MCG_OUT,0); 
	fb_clk_init();
	/* configure the Trim values for internal RC (slow) to get MCGOUT at 50MHz */
	MCG_C3 = 0x11;
	/* configure the DRS=1 */
	MCG_C4 = 0x30;	

	core_clk_mhz = 50;
	core_clk_khz = core_clk_mhz*1000;
	
	SIM_CLKOUT = SIM_CLKOUT_CLKOUTDIV(0)|SIM_CLKOUT_CS(3);
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
	  init_hw();
	pll_init(PLL48,XTAL8);
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
	SIM_SOPT7=0x80;	
	  
	/* PTB7 enables 5v on USB connector */
	MXC_PTBPF1|=0x10;
	PTB_DD|=0x80;
	PTB_D|=0x80;
        
    return 0;
}






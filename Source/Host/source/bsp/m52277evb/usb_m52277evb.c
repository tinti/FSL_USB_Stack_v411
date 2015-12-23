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
#include "ehci_host.h"
#include "usbevent.h"

extern USB_EVENT_STRUCT ehci_event;

extern __declspec(interrupt) void Timer_ISR(void);
extern __declspec(interrupt) void UART1_ISR(void);
void sci1_init(void);
void _usb_khci_task(void);
void TimerInit(void) ;
/***************************************************************************
 * Local Functions Prototypes 
 ***************************************************************************/
static asm void CACHE_Init(void);
static asm void _dcache_set_cacr(uint_32 cacr_val);
static void RAM_Init(void);
static void FLASH_Init(void);
extern void EHCI_Process_ISR(void);
/* linker generated symbols */
extern uint_8 __EXTERNAL_SDRAM_BASE[];
extern uint_8 __EXTERNAL_SDRAM_SIZE[];
extern uint_8 __INTERNAL_SRAM_BASE[];
extern uint_8 __INTERNAL_SRAM_SIZE[];
extern uint_8 __EXTERNAL_SRAM_BASE[];
extern uint_8 __EXTERNAL_SRAM_SIZE[];
extern uint_8 __EXTERNAL_FLASH_BASE[]; 
extern uint_8 __EXTERNAL_FLASH_SIZE[];

/***************************************************************************
 * Local Functions 
 ***************************************************************************/


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
         
  /* Activate the Limp mode while changing the PLL settings */
  MCF_CCM_MISCCR |= MCF_CCM_MISCCR_LIMP;
    
  MCF_CLOCK_PCR= MCF_CLOCK_PCR_PFDR(0x22)     // 34 fvco = 16 * 33MHz = 528MHz
        | MCF_CLOCK_PCR_OUTDIV5(8)            // fusb = fvco / 1 = 
        | MCF_CLOCK_PCR_OUTDIV3(3)            // ffb_clk = fvco / 8 = 66MHz
        | MCF_CLOCK_PCR_OUTDIV2(7)            // fbus = fsys / 2 = fvco / 4 = 132MHz
        | MCF_CLOCK_PCR_OUTDIV1(3);           // fsys = fvco / 2 = 264MHz;
  
  MCF_CCM_MISCCR &= ~MCF_CCM_MISCCR_LIMP;
  
  /* Wait for lock */
  while (!(MCF_CLOCK_PSR & MCF_CLOCK_PSR_LOCK)) {
    	/* nop */  
  };
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : scm_init
* Returned Value   : void
* Comments         :
*   
*
*END*----------------------------------------------------------------------*/
__declspec(register_abi) static asm void CACHE_Init(void)
{
  // invalidate the cache and disable it
  move.l #0x01000000, d0
  movec d0, CACR
  
  // disable ACRs
  moveq.l #0x00000040, d0
  movec d0, ACR0
  movec d0, ACR1     
  
  // initialize RAMBAR - locate it on the data bus
  move.l #0x80000000, d0
  andi.l #0xFFF80000, d0
  add.l #0x221, d0
  movec d0, RAMBAR  
  
  rts
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
    // USB_VBUS_OC
    MCF_PAD_PAR_UART = (uint_16)((MCF_PAD_PAR_UART & ~(3 << 4)) | ((uint_32)1 << 4));
   

}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : ram_init
* Returned Value   : void
* Comments         :
*   Initialize chip selects and SDRAM controller
*
*END*----------------------------------------------------------------------*/
static void RAM_Init(void) 
{

    if (!(MCF_SDRAMC_SDCR & MCF_SDRAMC_SDCR_REF_EN)) {
        MCF_SDRAMC_SDCR &= ~MCF_SDRAMC_SDCR_REF_EN;
    
        /* SDRAM chip select initialization */
        MCF_SDRAMC_SDCS0 = MCF_SDRAMC_SDCS_CSBA((uint_32)(void*)__EXTERNAL_SDRAM_BASE)
            | MCF_SDRAMC_SDCS_CSSZ(MCF_SDRAMC_SDCS_CSSZ_64MBYTE);

        /* 
        ** Basic configuration and initialization 
        */         
        MCF_SDRAMC_SDCFG1 = MCF_SDRAMC_SDCFG1_SRD2RWP(4)
		    | MCF_SDRAMC_SDCFG1_SWT2RWP(3)
		    | MCF_SDRAMC_SDCFG1_RD_LAT(7)
		    | MCF_SDRAMC_SDCFG1_ACT2RW(1)
		    | MCF_SDRAMC_SDCFG1_PRE2ACT(1)
		    | MCF_SDRAMC_SDCFG1_REF2ACT(6)
		    | MCF_SDRAMC_SDCFG1_WT_LAT(3);
           
	    MCF_SDRAMC_SDCFG2 = MCF_SDRAMC_SDCFG2_BRD2RP(5)
		    | MCF_SDRAMC_SDCFG2_BWT2RWP(6)
		    | MCF_SDRAMC_SDCFG2_BRD2W(6)
		    | MCF_SDRAMC_SDCFG2_BL(7);

        /* 
        ** Precharge and enable write to SDMR 
        */
        MCF_SDRAMC_SDCR = MCF_SDRAMC_SDCR_MODE_EN | MCF_SDRAMC_SDCR_CKE
		   | MCF_SDRAMC_SDCR_DDR_MODE | MCF_SDRAMC_SDCR_ADDR_MUX(1)
		   | MCF_SDRAMC_SDCR_REF_CNT(9)
		   | MCF_SDRAMC_SDCR_MEM_PS | MCF_SDRAMC_SDCR_IPALL;

        /* 
        ** Write extended mode register 
        */
	    MCF_SDRAMC_SDMR = MCF_SDRAMC_SDMR_BK(2) | MCF_SDRAMC_SDMR_AD(0x60) | MCF_SDRAMC_SDMR_CMD;

        /* 
        ** Write mode register and reset DLL 
        */
	    MCF_SDRAMC_SDMR = MCF_SDRAMC_SDMR_BK_LMR | MCF_SDRAMC_SDMR_AD(0x33) | MCF_SDRAMC_SDMR_CMD;

        /*
        ** Execute a PALL command 
        */
	    MCF_SDRAMC_SDCR |= MCF_SDRAMC_SDCR_IPALL;

        /* 
        ** Perform two REF cycles 
        */
	    MCF_SDRAMC_SDCR |= MCF_SDRAMC_SDCR_IREF;
	    MCF_SDRAMC_SDCR |= MCF_SDRAMC_SDCR_IREF;

        /* 
        ** Write mode register and clear reset DLL 
        */
	    //MCF_SDRAMC_SDMR = MCF_SDRAMC_SDMR_BK_LMR | MCF_SDRAMC_SDMR_AD(0x63) | MCF_SDRAMC_SDMR_CMD;
      
        /*
        ** Enable auto refresh and lock SDMR 
        */
	    MCF_SDRAMC_SDCR &= ~MCF_SDRAMC_SDCR_MODE_EN;
        MCF_SDRAMC_SDCR |= (MCF_SDRAMC_SDCR_REF_EN | MCF_SDRAMC_SDCR_DQS_OE(0x3));
    }
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : flash_init
* Returned Value   : void
* Comments         :
*   Initialize chip selects for FLASH
*
*END*----------------------------------------------------------------------*/
static void FLASH_Init(void) 
{
    MCF_FBCS_CSAR(0) = MCF_FBCS_CSAR_BA((uint_32)(void*)__EXTERNAL_FLASH_BASE);
    MCF_FBCS_CSCR(0) = MCF_FBCS_CSCR_WS(7) | MCF_FBCS_CSCR_SBM | MCF_FBCS_CSCR_AA | MCF_FBCS_CSCR_PS_16 | MCF_FBCS_CSCR_BEM;
    MCF_FBCS_CSMR(0) = MCF_FBCS_CSMR_BAM_16M | MCF_FBCS_CSMR_V;
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
   MCF_SCM_CWCR = 0;
   CACHE_Init();
   /* All masters are trusted */
   MCF_SCM_MPR = 0x77777777;

   /* Allow supervisor/user, read/write, and trusted/untrusted access to all slaves */
   MCF_SCM_PACRA = 0;
   MCF_SCM_PACRB = 0;
   MCF_SCM_PACRC = 0;
   MCF_SCM_PACRD = 0;
   MCF_SCM_PACRE = 0;
   MCF_SCM_PACRF = 0;
   MCF_SCM_PACRG = 0;
   MCF_SCM_PACRI = 0;
   RAM_Init();
   
   FLASH_Init();

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
   _usb_event_init(&ehci_event);
  
    Int_Ctl_int_init(USB_INT_CNTL, USB_ISR_SRC, 4,4, TRUE);
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

#pragma overload void sci1_init(void);
void sci1_init(void)
{
	
}


void _usb_khci_task(void)
{
 if(ehci_event.VALUE & EHCI_EVENT_ISR)
 {
  _usb_event_clear(&ehci_event, EHCI_EVENT_ISR);
  EHCI_Process_ISR();
  Int_Ctl_int_init(USB_INT_CNTL, USB_ISR_SRC, 4,4, TRUE);
 }
}
#pragma overload void TimerInit(void);
void TimerInit(void) 
{
	
}

#pragma overload __declspec(interrupt) void UART1_ISR(void);
__declspec(interrupt) void UART1_ISR(void)
{
}

#pragma overload __declspec(interrupt) void Timer_ISR(void);
__declspec(interrupt) void Timer_ISR(void)
{
	
}

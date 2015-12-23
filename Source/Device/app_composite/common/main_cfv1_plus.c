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
 * @file main_cfv1_plus.c
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief   This software is the USB driver stack for S08 family
 *****************************************************************************/
#include <hidef.h>
#include "types.h"
#include "derivative.h" /* include peripheral declarations */
#include "user_config.h"
#include "RealTimerCounter.h"
/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/

#if MAX_TIMER_OBJECTS
extern uint_8 TimerQInitialize(uint_8 ControllerId);
#endif
extern void TestApp_Init(void);
extern void TestApp_Task(void);
/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
void Watchdog_Reset(void);
void display_led(uint_8 val);
static void Init_Sys(void);
static void Mcu_Init(void);
static void MCG_Init(void);

static void StartKeyPressSimulationTimer(void);
static void KeyPressSimulationTimerCallback(void);


/****************************************************************************
 * Global Variables
 ****************************************************************************/
uint_8 kbi_stat_sim;
volatile uint_8 kbi_stat;	   /* Status of the Key Pressed */

void Watchdog_Reset(void)
{
	SIM_SRVCOP = 0x55;
	SIM_SRVCOP = 0xAA;
}
/******************************************************************************
 * @name        main
 *
 * @brief       This routine is the starting point of the application
 *
 * @param       None
 *
 * @return      None
 *
 *****************************************************************************
 * This function initializes the system, enables the interrupts and calls the
 * application
 *****************************************************************************/
void main(void)
{
    Init_Sys();        /* initial the system */
#if MAX_TIMER_OBJECTS
    (void)TimerQInitialize(0);
#endif
    (void)TestApp_Init(); /* Initialize the USB Test Application */
#if MAX_TIMER_OBJECTS
    StartKeyPressSimulationTimer();
#endif
    while(TRUE)
    {
       Watchdog_Reset();

       /* Call the application task */
       TestApp_Task();
    }
}
/*****************************************************************************
 * Local Functions
 *****************************************************************************/
/*****************************************************************************
 *
 *    @name     Init_Sys
 *
 *    @brief    This function Initializes the system
 *
 *    @param    None
 *
 *    @return   None
 *
 ****************************************************************************
 * Intializes the MCU, MCG, KBI, RTC modules
 ***************************************************************************/
static void Init_Sys(void)
{
    Mcu_Init(); /* initialize the MCU registers */
    MCG_Init(); /* initialize the MCG to generate 24MHz bus clock */
}

/*****************************************************************************
 * @name     MCU_Init
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

static void Mcu_Init()
{
	/* Enable clock source for UART0 module */
	SIM_SCGC1 |= SIM_SCGC1_UART0_MASK;
	/* Enable clock source for MTIM, FTM0 FTM1 modules */
	SIM_SCGC3 |= (SIM_SCGC3_MTIM_MASK|SIM_SCGC3_FTM1_MASK|SIM_SCGC3_FTM0_MASK);
	/* Enable clock source for USB module */
	SIM_SCGC6 |= SIM_SCGC6_USBOTG_MASK;
	
	/* Enable Port Muxing of Uart0 */
	MXC_PTAPF1 |= MXC_PTAPF1_A7(2);
	MXC_PTDPF1 |= MXC_PTDPF1_D6(2);

}

/*****************************************************************************
 * @name     MCG_Init
 *
 * @brief:   Initialization of the Multiple Clock Generator.
 *
 * @param  : None
 *
 * @return : None
 *****************************************************************************
 * Provides clocking options for the device, including a phase-locked
 * loop(PLL) and frequency-locked loop (FLL) for multiplying slower reference
 * clock sources
 ****************************************************************************/
static void MCG_Init()
{
    /* Select MCGOUT */
    SIM_CLKOUT = SIM_CLKOUT_CLKOUTDIV(0)|SIM_CLKOUT_CS(3);
    
    /* configure the Trim values for internal RC (slow) to get MCGOUT at 50MHz */
    MCG_C3 = 0x30;
    /* configure the DRS=1 */
    MCG_C4 = 0xB0; 
           
    SIM_SOPT3 |= SIM_SOPT3_RWE_MASK; 
    SIM_SOPT1 |= SIM_SOPT1_REGE_MASK;
    SIM_SOPT7 |= 0x80;
    // PLL/FLL selected as CLK source
    SIM_CLKDIV1 = 0x01;
}
#if MAX_TIMER_OBJECTS
/******************************************************************************
 * @name        StartKeyPressSimulationTimer
 *
 * @brief       This routine is used to start the timer for generating key press events
 *              Used on boards without buttons
 *
 * @param       None
 *
 * @return      None
 *
 *****************************************************************************
 *
 *****************************************************************************/
static void StartKeyPressSimulationTimer(void)
{
  TIMER_OBJECT TimerObject;
			
  TimerObject.msCount = KEY_PRESS_SIM_TMR_INTERVAL;
  TimerObject.pfnTimerCallback = (void*)KeyPressSimulationTimerCallback;
  (void)AddTimerQ(&TimerObject);
}

/******************************************************************************
 * @name        KeyPressSimulationTimerCallback
 *
 * @brief       This routine is used to generate simulated key press events
 *              It is called at KeyPressSimulationTimer expire time
 *              Used on boards without buttons
 *
 * @param       None
 *
 * @return      None
 *
 *****************************************************************************
 *
 *****************************************************************************/
static void KeyPressSimulationTimerCallback(void)
{
	 /* generate the button press */
	 kbi_stat_sim = (uint_8)((kbi_stat_sim << 1) & (uint_8)0x0F);
	 
	 if(kbi_stat_sim == 0)
	 {
	  kbi_stat_sim = 1;
	 }
	 
	 kbi_stat = kbi_stat_sim;
	 
	 /* re-trigger the timer */
	 StartKeyPressSimulationTimer();
}
/******************************************************************************
 * @name        display_led
 *
 * @brief       Display LEDs
 *
 * @param       Bit map corresponding with displayed LEDs
 *
 * @return      None
 *
 *****************************************************************************
 *
 *****************************************************************************/
#endif /* End of #if MAX_TIMER_OBJECTS*/
void display_led(uint_8 val)
{
	UNUSED(val)
}

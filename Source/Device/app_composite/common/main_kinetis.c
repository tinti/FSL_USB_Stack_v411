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
 * @file main_kinetis.c
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief   This software is the USB driver stack for S08 family
 *****************************************************************************/
#include "types.h"
#include "derivative.h" /* include peripheral declarations */
#include "user_config.h"
#include "RealTimerCounter.h"
#include "Wdt_kinetis.h"
#include "hidef.h"


extern uint_32 ___VECTOR_RAM[];            //Get vector table that was copied to RAM

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
#if MAX_TIMER_OBJECTS
extern uint_8 TimerQInitialize(uint_8 ControllerId);
#endif
extern void TestApp_Init(void);
extern void TestApp_Task(void);
#if (defined MCU_MK60N512VMD100) || (defined MCU_MK53N512CMD100)
#define BSP_CLOCK_SRC                   (50000000ul)       // crystal, oscillator freq
#else
#define BSP_CLOCK_SRC                   (8000000ul)       // crystal, oscillator freq
#endif
#define BSP_REF_CLOCK_SRC               (2000000ul)       // must be 2-4MHz

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

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static void Init_Sys(void);
static unsigned char pll_init();

/****************************************************************************
 * Global Variables
 ****************************************************************************/
volatile uint_8 kbi_stat;	   /* Status of the Key Pressed */


/*****************************************************************************
 * Global Functions
 *****************************************************************************/
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
    
#if USART_DEBUG
    sci_init();
#endif /* USART_DEBUG */
#if MAX_TIMER_OBJECTS
    (void)TimerQInitialize(0);
#endif
    (void)TestApp_Init(); /* Initialize the USB Test Application */
    
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
void GPIO_Init()
{    
#if (!defined(MCU_MK21D5)&&!defined(MCU_MKL25Z4))
    SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
    PORTC_PCR8|= PORT_PCR_SRE_MASK    /* Slow slew rate */
              |  PORT_PCR_ODE_MASK    /* Open Drain Enable */
              |  PORT_PCR_DSE_MASK    /* High drive strength */
              ;
    PORTC_PCR8 = PORT_PCR_MUX(1);
    PORTC_PCR9|= PORT_PCR_SRE_MASK    /* Slow slew rate */
              |  PORT_PCR_ODE_MASK    /* Open Drain Enable */
              |  PORT_PCR_DSE_MASK    /* High drive strength */
              ;
    PORTC_PCR9 = PORT_PCR_MUX(1);
    GPIOC_PSOR = 1 << 8 | 1 << 9;
    GPIOC_PDDR |= 1 << 8 | 1 << 9;
#endif
    /* setting for port interrupt */
#if (defined MCU_MK40N512VMD100) ||  (defined MCU_MK53N512CMD100)
    SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	/* set in put PORTC5*/
	PORTC_PCR5 =  PORT_PCR_MUX(1);
	GPIOC_PDDR &= ~((uint_32)1 << 5);
	/* pull up*/
	PORTC_PCR5 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
	/* GPIO_INT_EDGE_HIGH */
	PORTC_PCR5 |= PORT_PCR_IRQC(9);	
	/* set in put PORTC13*/
	PORTC_PCR13 =  PORT_PCR_MUX(1);
	GPIOC_PDDR &= ~((uint_32)1 << 13);
	/* pull up*/
	PORTC_PCR13 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
	/* GPIO_INT_EDGE_HIGH */
	PORTC_PCR13 |= PORT_PCR_IRQC(9);
	/* enable interrupt */
	PORTC_ISFR = (1 << 5);
	PORTC_ISFR = (1 << 13);
	NVICICPR2 = 1 << ((89)%32);
	NVICISER2 = 1 << ((89)%32);
	
	PORTC_PCR8 =  PORT_PCR_MUX(1);
	GPIOC_PDDR |= 1<<8;
#endif
	
#if defined(MCU_MK60N512VMD100)  
	/* Enable clock gating to PORTA and PORTE */
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK |SIM_SCGC5_PORTE_MASK;
	
	/* set in put PORTA pin 19 */
	PORTA_PCR19 =  PORT_PCR_MUX(1);
	
	// set led pins
	PORTA_PCR10 =  PORT_PCR_MUX(1);
	PORTA_PCR11 =  PORT_PCR_MUX(1);
	PORTA_PCR28 =  PORT_PCR_MUX(1);
	PORTA_PCR29 =  PORT_PCR_MUX(1);
	
	GPIOA_PDDR |= (1<<10);
	GPIOA_PDDR |= (1<<11);
	GPIOA_PDDR |= (1<<28);
	GPIOA_PDDR |= (1<<29);
		
	/* set in put PORTA pin 19 */
	PORTA_PCR19 =  PORT_PCR_MUX(1);
	GPIOC_PDDR &= ~((uint_32)1 << 19);
	
	/* pull up*/
	PORTA_PCR19 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
	
	/* GPIO_INT_EDGE_HIGH */
	PORTA_PCR19 |= PORT_PCR_IRQC(9);	
	
	/* set in put PORTE pin 26 */
	PORTE_PCR26 =  PORT_PCR_MUX(1);
	GPIOC_PDDR &= ~((uint_32)1 << 26);
	
	/* pull up*/
	PORTE_PCR26 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
	
	/* GPIO_INT_EDGE_HIGH */
	PORTE_PCR26 |= PORT_PCR_IRQC(9);
	
	/* Clear interrupt flag */
	PORTA_ISFR = (1 << 19);
	PORTE_ISFR = (1 << 26);
	
	/* enable interrupt port A */
	NVICICPR2 = 1 << ((87)%32);
	NVICISER2 = 1 << ((87)%32);
	
	/* enable interrupt port E */
	NVICICPR2 = 1 << ((91)%32);
	NVICISER2 = 1 << ((91)%32);
#endif
	
#if defined(MCU_MK21D5)  
    /* Enable clock gating to PORTC and PORTD */
    SIM_SCGC5 |= (SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK);

    /* LEDs settings */
    PORTD_PCR4 =  PORT_PCR_MUX(1);
    PORTD_PCR5 =  PORT_PCR_MUX(1);
    PORTD_PCR6 =  PORT_PCR_MUX(1);
    PORTD_PCR7 =  PORT_PCR_MUX(1);

    GPIOD_PDDR |= (1<<4) | (1<<5) | (1<<6) | (1<<7);

    /* Switch buttons settings */
    /* Set input on PORTC pin 6 */
    PORTC_PCR6 =  PORT_PCR_MUX(1);
    GPIOC_PDDR &= ~((uint_32)1 << 6);
    /* Pull up enabled */
    PORTC_PCR6 |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    /* GPIO_INT_EDGE_HIGH */
    PORTC_PCR6 |= PORT_PCR_IRQC(9);

    /* Set input on PORTC pin 7 */
    PORTC_PCR7 =  PORT_PCR_MUX(1);
    GPIOC_PDDR &= ~((uint_32)1 << 7);
    /* Pull up */
    PORTC_PCR7 |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    /* GPIO_INT_EDGE_HIGH */
    PORTC_PCR7 |= PORT_PCR_IRQC(9);

    /* Clear interrupt flag */
    PORTC_ISFR = (1 << 6);
    PORTC_ISFR = (1 << 7);

    /* Enable interrupt port C */
    NVICICPR1 = 1 << ((61)%32);
    NVICISER1 = 1 << ((61)%32);
#endif
#if defined(MCU_MKL25Z4)  
    /* Enable clock gating to PORTA, PORTB, PORTC, PORTD and PORTE */
    SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK 
              | SIM_SCGC5_PORTB_MASK 
              | SIM_SCGC5_PORTC_MASK 
              | SIM_SCGC5_PORTD_MASK 
              | SIM_SCGC5_PORTE_MASK);

    /* LEDs settings */
    PORTA_PCR5  =  PORT_PCR_MUX(1);
    PORTA_PCR16 =  PORT_PCR_MUX(1);
    PORTA_PCR17 =  PORT_PCR_MUX(1);
    PORTB_PCR8  =  PORT_PCR_MUX(1);

    GPIOA_PDDR |= (1<<5) | (1<<16) | (1<<17);
    GPIOB_PDDR |= (1<<8);

    /* Switch buttons settings */
    /* Set input on PORTC pin 3 */
    PORTC_PCR3 =  PORT_PCR_MUX(1);
    GPIOC_PDDR &= ~((uint_32)1 << 3);
    /* Pull up enabled */
    PORTC_PCR3 |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    /* GPIO_INT_EDGE_HIGH */
    PORTC_PCR3 |= PORT_PCR_IRQC(9);

    /* Set input on PORTA pin 4 */
    PORTA_PCR4 =  PORT_PCR_MUX(1);
    GPIOA_PDDR &= ~((uint_32)1 << 4);
    /* Pull up */
    PORTA_PCR4 |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    /* GPIO_INT_EDGE_HIGH */
    PORTA_PCR4 |= PORT_PCR_IRQC(9);

    /* Clear interrupt flag */
    PORTC_ISFR = (1 << 3);
    PORTA_ISFR = (1 << 4);

    /* Enable interrupt port A */
    NVIC_ICPR = 1 << 30;
    NVIC_ISER = 1 << 30;
#endif
}

/******************************************************************************
 * @name       all_led_off
 *
 * @brief      Switch OFF all LEDs on Board
 *
 * @param	   None
 *
 * @return     None
 *
 *****************************************************************************
 * This function switch OFF all LEDs on Board
 *****************************************************************************/
static void all_led_off(void)
{
	GPIOC_PSOR = 1 << 8 | 1 << 9;
}

/******************************************************************************
 * @name       display_led
 *
 * @brief      Displays 8bit value on Board LEDs
 *
 * @param	   val
 *
 * @return     None
 *
 *****************************************************************************
 * This function displays 8 bit value on Board LED
 *****************************************************************************/
void display_led(uint_8 val)
{
    uint_8 i = 0;
	UNUSED(i);
    all_led_off();

	val &= 0x03;
	if(val & 0x1)
		GPIOC_PCOR = 1 << 8;
	if(val & 0x2)
		GPIOC_PCOR = 1 << 9;
}
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
 * Initializes the MCU, MCG, KBI, RTC modules
 ***************************************************************************/
static void Init_Sys(void)
{
	/* Point the VTOR to the new copy of the vector table */
	SCB_VTOR = (uint_32)___VECTOR_RAM;
	  
#if (defined MCU_MK21D5)
    NVICICPR1 = (1 << 21);    /* Clear any pending interrupts on USB */
    NVICISER1 = (1 << 21);    /* Enable interrupts from USB module */   
#elif (defined MCU_MKL25Z4)
    NVIC_ICPR = (1 << 24);	 /* Clear any pending interrupts on USB */
    NVIC_ISER = (1 << 24);	 /* Enable interrupts from USB module */	
#else
    NVICICPR2 = (1 << 9);	/* Clear any pending interrupts on USB */
    NVICISER2 = (1 << 9);	/* Enable interrupts from USB module */             
#endif  

    /* SIM Configuration */
	GPIO_Init();
	pll_init();
#if !(defined MCU_MK21D5) && !(defined MCU_MKL25Z4) 
    MPU_CESR=0x00;
#endif    
    /************* USB Part **********************/
    /*********************************************/   
#ifndef MCU_MKL25Z4
    /* Configure USBFRAC = 0, USBDIV = 1 => frq(USBout) = 1 / 2 * frq(PLLin) */
    SIM_CLKDIV2 &= SIM_CLKDIV2_USBFRAC_MASK | SIM_CLKDIV2_USBDIV_MASK;
    SIM_CLKDIV2|= SIM_CLKDIV2_USBDIV(0);
#endif    
    
    /* Enable USB-OTG IP clocking */
    SIM_SCGC4|=(SIM_SCGC4_USBOTG_MASK);          
    
    /* Configure USB to be clocked from PLL */
    SIM_SOPT2  |= SIM_SOPT2_USBSRC_MASK | SIM_SOPT2_PLLFLLSEL_MASK;
    
    /* Configure enable USB regulator for device */
    SIM_SOPT1 |= SIM_SOPT1_USBREGEN_MASK;
}

#if(!(defined MCU_MK21D5))
/******************************************************************************
*   @name        IRQ_ISR_PORTA
*
*   @brief       Service interrupt routine of IRQ
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
void IRQ_ISR_PORTA(void)
{
#if defined (MCU_MKL25Z4)
    NVIC_ICPR = 1 << 30;
    NVIC_ISER = 1 << 30;
#else
    NVICICPR2 = 1 << ((87)%32);
    NVICISER2 = 1 << ((87)%32);
#endif
    DisableInterrupts;
#if defined MCU_MKL25Z4
    if(PORTA_ISFR & (1<<4))
    {
        kbi_stat |= 0x02;                 /* Update the kbi state */
        PORTA_ISFR = (1 << 4);            /* Clear the bit by writing a 1 to it */
    }
#else
    if(PORTA_ISFR & (1<<19))
    {
        kbi_stat |= 0x02;                 /* Update the kbi state */
        PORTA_ISFR = (1 << 19);            /* Clear the bit by writing a 1 to it */
    }
#endif
    EnableInterrupts;
}
#endif

#if (!defined MCU_MKL25Z4)
/******************************************************************************
*   @name        IRQ_ISR
*
*   @brief       Service interrupt routine of IRQ
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
void IRQ_ISR_PORTC(void)
{	
#if defined(MCU_MK21D5)
    NVICICPR1 = 1 << ((61)%32);
    NVICISER1 = 1 << ((61)%32);
#else
	NVICICPR2 = (uint32_t)(1 << ((89)%32));		/* Clear any pending interrupt on PORTC */
	NVICISER2 = (uint32_t)(1 << ((89)%32));		/* Set interrupt on PORTC */
#endif
	DisableInterrupts;
#if defined(MCU_MK21D5)
    if(PORTC_ISFR & (1<<6))
#else
	if(PORTC_ISFR & (1<<5))
#endif	
	{
		kbi_stat |= 0x02; 				/* Update the kbi state */
#if defined(MCU_MK21D5)
        PORTC_ISFR = (1 << 6);
#else
	    PORTC_ISFR = (1 << 5);
#endif	
	}
	
#if defined(MCU_MK21D5)
    if(PORTC_ISFR & (1<<7))
#else
	if(PORTC_ISFR & (1<<13))
#endif	
	{
		kbi_stat |= 0x08;				/* Update the kbi state */
#if defined(MCU_MK21D5)
        PORTC_ISFR = (1 << 7);
#else
	    PORTC_ISFR = (1 << 13);
#endif	
	}	
	EnableInterrupts;
}
#endif

#if((!defined MCU_MK21D5)&&(!defined MCU_MKL25Z4))
/******************************************************************************
*   @name        IRQ_ISR_PORTE
*
*   @brief       Service interrupt routine of IRQ
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
void IRQ_ISR_PORTE(void)
{
	NVICICPR2 = 1 << ((91)%32);
	NVICISER2 = 1 << ((91)%32);	
	DisableInterrupts;
	if(PORTE_ISFR & (1<<26))
	{
		kbi_stat |= 0x08;				/* Update the kbi state */
		PORTE_ISFR = (1 << 26);			/* Clear the bit by writing a 1 to it */
	}
	EnableInterrupts;
}
#endif

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
#ifdef MCU_MK21D5
    /* System clock initialization */
    if ( *((uint8_t*) 0x03FFU) != 0xFFU) {
        MCG_C3 = *((uint8_t*) 0x03FFU);
        MCG_C4 = (MCG_C4 & 0xE0U) | ((*((uint8_t*) 0x03FEU)) & 0x1FU);
    }
    /* SIM_CLKDIV1: OUTDIV1=0,OUTDIV2=1,OUTDIV3=0,OUTDIV4=2,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0 */
    SIM_CLKDIV1 = (uint32_t)0x00010000UL; /* Update system prescalers */
    /* OSC_CR: ERCLKEN=0,??=0,EREFSTEN=0,??=0,SC2P=0,SC4P=0,SC8P=0,SC16P=0 */
    OSC_CR = (uint8_t)0x00U;                             
    /* MCG_C7: OSCSEL=0 */
    MCG_C7 &= (uint8_t)~(uint8_t)0x01U;                           
    /* MCG_C2: LOCRE0=0,??=0,RANGE0=2,HGO0=0,EREFS0=1,LP=0,IRCS=1 */
    MCG_C2 = (uint8_t)0x25U;                             
    /* MCG_C1: CLKS=2,FRDIV=3,IREFS=0,IRCLKEN=0,IREFSTEN=0 */
    MCG_C1 = (uint8_t)0x98U;
    /* MCG_C4: DMX32=0,DRST_DRS=0 */
    MCG_C4 &= (uint8_t)~(uint8_t)0xE0U;
    /* MCG_C5: ??=0,PLLCLKEN0=0,PLLSTEN0=0,PRDIV0=3 */
    MCG_C5 = (uint8_t)0x03U;                             
    /* MCG_C6: LOLIE0=0,PLLS=0,CME0=0,VDIV0=0 */
    MCG_C6 = (uint8_t)0x00U;                             
    while((MCG_S & MCG_S_OSCINIT0_MASK) == 0x00U) { /* Check that the oscillator is running */
    }
    while((MCG_S & MCG_S_IREFST_MASK) != 0x00U) { /* Check that the source of the FLL reference clock is the external reference clock. */
    }
    while((MCG_S & 0x0CU) != 0x08U) {    /* Wait until external reference clock is selected as MCG output */
    }
    /* Switch to PBE Mode */
    /* OSC_CR: ERCLKEN=0,??=0,EREFSTEN=0,??=0,SC2P=0,SC4P=0,SC8P=0,SC16P=0 */
    OSC_CR = (uint8_t)0x00U;                             
    /* MCG_C7: OSCSEL=0 */
    MCG_C7 &= (uint8_t)~(uint8_t)0x01U;
    /* MCG_C1: CLKS=2,FRDIV=3,IREFS=0,IRCLKEN=0,IREFSTEN=0 */
    MCG_C1 = (uint8_t)0x98U;                             
    /* MCG_C2: LOCRE0=0,??=0,RANGE0=2,HGO0=0,EREFS0=1,LP=0,IRCS=1 */
    MCG_C2 = (uint8_t)0x25U;                             
    /* MCG_C5: ??=0,PLLCLKEN0=0,PLLSTEN0=0,PRDIV0=3 */
    MCG_C5 = (uint8_t)0x03U;                             
    /* MCG_C6: LOLIE0=0,PLLS=1,CME0=0,VDIV0=0 */
    MCG_C6 = (uint8_t)0x40U;                             
    while((MCG_S & 0x0CU) != 0x08U) {    /* Wait until external reference clock is selected as MCG output */
    }
    while((MCG_S & MCG_S_LOCK0_MASK) == 0x00U) { /* Wait until locked */
    }
    /* Switch to PEE Mode */
    /* OSC_CR: ERCLKEN=0,??=0,EREFSTEN=0,??=0,SC2P=0,SC4P=0,SC8P=0,SC16P=0 */
    OSC_CR = (uint8_t)0x00U;                             
    /* MCG_C7: OSCSEL=0 */
    MCG_C7 &= (uint8_t)~(uint8_t)0x01U;
    /* MCG_C1: CLKS=0,FRDIV=3,IREFS=0,IRCLKEN=0,IREFSTEN=0 */
    MCG_C1 = (uint8_t)0x18U;                             
    /* MCG_C2: LOCRE0=0,??=0,RANGE0=2,HGO0=0,EREFS0=1,LP=0,IRCS=1 */
    MCG_C2 = (uint8_t)0x25U;                             
    /* MCG_C5: ??=0,PLLCLKEN0=0,PLLSTEN0=0,PRDIV0=3 */
    MCG_C5 = (uint8_t)0x03U;                             
    /* MCG_C6: LOLIE0=0,PLLS=1,CME0=0,VDIV0=0 */
    MCG_C6 = (uint8_t)0x40U;                             
    while((MCG_S & 0x0CU) != 0x0CU) {    /* Wait until output of the PLL is selected */
    }
    while((MCG_S & MCG_S_LOCK0_MASK) == 0x00U) { /* Wait until locked */
    }
    /* MCG_C6: CME0=1 */
    MCG_C6 |= (uint8_t)0x20U;            /* Enable the clock monitor */   
#elif (defined MCU_MKL25Z4)
    SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV4(1);
    SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(1);/* Update system prescalers */
    
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
    
#else	
    /* 
     * First move to FBE mode
     * Enable external oscillator, RANGE=0, HGO=, EREFS=, LP=, IRCS=
    */
#if defined(MCU_MK60N512VMD100) || defined(MCU_MK53N512CMD100)
    MCG_C2 = 0;
#else
	/* Enable external oscillator, RANGE=2, HGO=1, EREFS=1, LP=0, IRCS=0 */
	MCG_C2 = MCG_C2_RANGE(2) | MCG_C2_HGO_MASK | MCG_C2_EREFS_MASK|MCG_C2_IRCS_MASK;
#endif

    /* Select external oscillator and Reference Divider and clear IREFS 
     * to start external oscillator
     * CLKS = 2, FRDIV = 3, IREFS = 0, IRCLKEN = 0, IREFSTEN = 0
     */
    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(3);
#ifdef MCU_MK40N512VMD100
	/* wait for oscillator to initialize */
   while (!(MCG_S & MCG_S_OSCINIT_MASK)){};  
#endif
    
    /* Wait for Reference Clock Status bit to clear */
    while (MCG_S & MCG_S_IREFST_MASK) {};
    
    /* Wait for clock status bits to show clock source 
     * is external reference clock */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2) {};
    
    /* Now in FBE
     * Configure PLL Reference Divider, PLLCLKEN = 0, PLLSTEN = 0, PRDIV = 0x18
     * The crystal frequency is used to select the PRDIV value. 
     * Only even frequency crystals are supported
     * that will produce a 2MHz reference clock to the PLL.
     */

#if defined(MCU_MK60N512VMD100) || defined(MCU_MK53N512CMD100)
    MCG_C5 = MCG_C5_PRDIV(BSP_REF_CLOCK_DIV - 1);
#else
    MCG_C5 = MCG_C5_PRDIV(BSP_REF_CLOCK_DIV - 1) | MCG_C5_PLLCLKEN_MASK;
#endif
    /* Ensure MCG_C6 is at the reset default of 0. LOLIE disabled, 
     * PLL disabled, clock monitor disabled, PLL VCO divider is clear
     */
    MCG_C6 = 0;

    
    /* Calculate mask for System Clock Divider Register 1 SIM_CLKDIV1 */
    SIM_CLKDIV1 =  SIM_CLKDIV1_OUTDIV1(BSP_CORE_DIV    - 1) |
                        SIM_CLKDIV1_OUTDIV2(BSP_BUS_DIV     - 1) |
                        SIM_CLKDIV1_OUTDIV3(BSP_FLEXBUS_DIV - 1) |
                        SIM_CLKDIV1_OUTDIV4(BSP_FLASH_DIV   - 1);

   /* Set the VCO divider and enable the PLL, 
     * LOLIE = 0, PLLS = 1, CME = 0, VDIV = 2MHz * BSP_CLOCK_MUL
     */
    MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(BSP_CLOCK_MUL - 24);

    /* wait for PLL status bit to set */
    while (!(MCG_S & MCG_S_PLLST_MASK)) {};
    
    /* Wait for LOCK bit to set */
    while (!(MCG_S & MCG_S_LOCK_MASK)) {};

    /* Now running PBE Mode */

    /* Transition into PEE by setting CLKS to 0
     * CLKS=0, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0
     */
    MCG_C1 &= ~MCG_C1_CLKS_MASK;

    /* Wait for clock status bits to update */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3) {};

    /* Now running PEE Mode */
#endif
    return 0;
}

/* EOF */

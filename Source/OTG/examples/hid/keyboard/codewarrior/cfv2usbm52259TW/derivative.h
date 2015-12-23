/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand
 */

/* Include the derivative-specific header file */
#include <MCF52259.h>
#include "psptypes.h"
#define _Stop asm ( mov3q #4,d0; bclr.b d0,SOPT1; stop #0x2000; )
  /*!< Macro to enter stop modes, STOPE bit in SOPT1 register must be set prior to executing this macro */

#define _Wait asm ( mov3q #4,d0; bset.b d0,SOPT1; nop; stop #0x2000; )
  /*!< Macro to enter wait mode */
#define __RESET_WATCHDOG() (void)(MCF_BWT_WSR = 0x5555, MCF_BWT_WSR = 0xAAAA)

#define DTIM0_ISR_SRC  19
#define USB_ISR_SRC    53
#define PIT0_ISR_SRC   55
#define PIT1_ISR_SRC   56
#define UART1_ISR_SRC  14
#define IRQ1_ISR_SRC   1
#define IRQ3_ISR_SRC   3
#define IRQ5_ISR_SRC   5

#define DTIM0_INT_CNTL	0
#define IRQ1_INT_CNTL   0   /* Coresponding assigned interrupt controller */
#define IRQ3_INT_CNTL   0   /* Coresponding assigned interrupt controller */
#define IRQ5_INT_CNTL   0   /* Coresponding assigned interrupt controller */
#define PIT0_INT_CNTL   0   /* Coresponding assigned interrupt controller */
#define PIT1_INT_CNTL   0   /* Coresponding assigned interrupt controller */
#define USB_INT_CNTL    0   /* Coresponding assigned interrupt controller */
#define UART1_INT_CNTL  0   /* Coresponding assigned interrupt controller */

#define USB_ISR_SRC    53
#define PIT1_ISR_SRC   56


#define PIT1_INT_CNTL   0   /* Coresponding assigned interrupt controller */
#define USB_INT_CNTL    0   /* Coresponding assigned interrupt controller */
	  
#define USE_POLL
#define __MCF52259_TW_
#define IRQ_ISR_USED
#define USED_PIT1
#define __MCF52xxx_H__
#ifdef __MCF52259_TW_
    #define ENABLE_USB_5V   MCF_GPIO_PORTUC |= MCF_GPIO_PORTUC_PORTUC3; /* enable Vhost  */
    #define DISABLE_USB_5V  MCF_GPIO_PORTUC &= ~MCF_GPIO_PORTUC_PORTUC3; /* disable Vhost */
#endif
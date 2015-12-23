/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand
 */

/* Include the derivative-specific header file */
#include <MCF52221.h>
#include "psptypes.h"
#define _Stop asm ( mov3q #4,d0; bclr.b d0,SOPT1; stop #0x2000; )
  /*!< Macro to enter stop modes, STOPE bit in SOPT1 register must be set prior to executing this macro */

#define _Wait asm ( mov3q #4,d0; bset.b d0,SOPT1; nop; stop #0x2000; )
  /*!< Macro to enter wait mode */
#define __RESET_WATCHDOG() (void)(MCF_SCM_CWSR = 0x5555, MCF_SCM_CWSR = 0xAAAA)

#define __MCF52xxx_H__

#define DTIM0_ISR_SRC  19
#define USB_ISR_SRC    53
#define PIT0_ISR_SRC   55
#define PIT1_ISR_SRC   56
#define UART1_ISR_SRC  14
#define IRQ1_ISR_SRC			1
#define IRQ7_ISR_SRC			7


#define DTIM0_INT_CNTL			0
#define IRQ1_INT_CNTL			0
#define IRQ7_INT_CNTL			0
#define PIT0_INT_CNTL   0   /* Coresponding assigned interrupt controller */
#define PIT1_INT_CNTL   0   /* Coresponding assigned interrupt controller */
#define USB_INT_CNTL    0   /* Coresponding assigned interrupt controller */
#define UART1_INT_CNTL  0   /* Coresponding assigned interrupt controller */

#define PIT0_ISR_USED
#define DTIM0_ISR_USED
#define IRQ_ISR_USED

#define TIMER_MODULAR       10000
#define TIMER_DIV           8

#define USE_POLL
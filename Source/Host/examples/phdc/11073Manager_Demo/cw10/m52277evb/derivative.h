/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand
 */

/* Include the derivative-specific header file */
#include <MCF52277.h>
#include "psptypes.h"
#define _Stop asm ( mov3q #4,d0; bclr.b d0,SOPT1; stop #0x2000; )
  /*!< Macro to enter stop modes, STOPE bit in SOPT1 register must be set prior to executing this macro */

#define _Wait asm ( mov3q #4,d0; bset.b d0,SOPT1; nop; stop #0x2000; )
  /*!< Macro to enter wait mode */
#define __RESET_WATCHDOG() (void)(MCF_SCM_CWSR = 0x55, MCF_SCM_CWSR = 0xAA)

#define USB_ISR_SRC		47  /* USB  Interrupt source */
#define PIT1_ISR_SRC    44
#define UART1_ISR_SRC	27  /* USB  Interrupt source */


#define USB_INT_CNTL    1   /* Coresponding assigned interrupt controller */
#define UART1_INT_CNTL  0   /* Coresponding assigned interrupt controller */
#define PIT1_INT_CNTL   1   /* Coresponding assigned interrupt controller */


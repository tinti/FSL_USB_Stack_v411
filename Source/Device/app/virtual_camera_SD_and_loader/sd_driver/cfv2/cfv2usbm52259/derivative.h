/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand
 */

/* Include the derivative-specific header file */
#include "MCF52259.h"
#include "types.h"      /* User Defined Data Types */
#include "Int_Ctl_cfv2.h"

#define __MCF52xxx_H__

#define _Stop asm ( mov3q #4,d0; bclr.b d0,SOPT1; stop #0x2000; )
  /*!< Macro to enter stop modes, STOPE bit in SOPT1 register must be set prior to executing this macro */

#define _Wait asm ( mov3q #4,d0; bset.b d0,SOPT1; nop; stop #0x2000; )
  /*!< Macro to enter wait mode */

/* Interrupt controller */
#define USB_INT_CNTL            0
#define PIT0_INT_CNTL           0
#define UART1_INT_CNTL  		0 

/* Interrupt sources */
#define USB_ISR_SRC             53
#define PIT0_ISR_SRC            55
#define UART1_ISR_SRC  			14

#define BSP_LED1    MCF_GPIO_PORTTC_PORTTC0
#define BSP_LED2    MCF_GPIO_PORTTC_PORTTC1
#define BSP_LED3    MCF_GPIO_PORTTC_PORTTC2
#define BSP_LED4    MCF_GPIO_PORTTC_PORTTC3



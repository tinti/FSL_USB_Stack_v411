/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand
 */

/* Include the derivative-specific header file */
#include <MKL25Z4.h>
#define _MK_xxx_H_

#define USE_POLL

#define USE_PIT1
#define USE_PIT0

#define USE_IRQ_PTA
#define USE_IRQ_PTD

#define LITTLE_ENDIAN

#define printf printf_kinetis
#define sprintf sprintf_kinetis


/* reset watchdog timer */
#ifndef __RESET_WATCHDOG
  #define __RESET_WATCHDOG()  (void)(WDOG_REFRESH = 0xA602, WDOG_REFRESH = 0xB480)
#endif 
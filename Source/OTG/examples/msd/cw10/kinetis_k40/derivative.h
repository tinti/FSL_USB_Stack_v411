/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand
 */

/* Include the derivative-specific header file */
#include <MK40N512VMD100.h>
#include "user_config.h"
#define USE_IRQ_PTB
#define USE_IRQ_PTC
#define USE_POLL
#define USE_PIT1
#define USE_PIT0
#define _MK_xxx_H_
#define printf printf_kinetis
#define sprintf sprintf_kinetis

/* reset watchdog timer */
#ifndef __RESET_WATCHDOG
  #define __RESET_WATCHDOG()  (void)(WDOG_REFRESH = 0xA602, WDOG_REFRESH = 0xB480)
#endif 

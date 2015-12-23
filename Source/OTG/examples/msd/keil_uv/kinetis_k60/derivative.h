/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand
 */

/* Include the derivative-specific header file */
#include <MK60N512VMD100.h>
#include <stdio.h>
#include <stdlib.h>
#include "user_config.h"

#define _MK_xxx_H_
/* reset watchdog timer */
#ifndef __RESET_WATCHDOG
  #define __RESET_WATCHDOG()  (void)(WDOG_REFRESH = 0xA602, WDOG_REFRESH = 0xB480)
#endif 

#define USE_IRQ_PTA
#define IRQ_INDEX_PORTA	87 /* interrupt index PORTA */

#define USE_PIT0
#define USE_PIT1
#define USE_POLL

/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand
 */

/* Include the derivative-specific header file */
#include <MK21D5.h>

#include "user_config.h"
#define USE_IRQ_PTA
#define USE_IRQ_PTC
#define IRQ_INDEX_PORTC 61  /* interrupt index PORTC */ 

#define _MK_xxx_H_
#define USE_POLL
#define USED_PIT0
#define USED_PIT1
#define LITTLE_ENDIAN

#define printf 	printf_kinetis
#define sprintf sprintf_kinetis

/* Reset watchdog timer */
#ifndef __RESET_WATCHDOG
  #define __RESET_WATCHDOG()  (void)(WDOG_REFRESH = 0xA602, WDOG_REFRESH = 0xB480)
#endif 

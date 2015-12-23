/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand
 */

/* Include the derivative-specific header file */
#include <MK20D7.h>
#include <stdio.h>
#include <stdlib.h>
#include "user_config.h"

#define IRQ_INDEX_PORTA	87 	/* interrupt index PORTA */
#define IRQ_INDEX_PORTC	89 	/* Interrupt index PORTC */
#define IRQ_INDEX_PORTE	91 	/* interrupt index PORTE */

#define USE_IRQ_PTA
#define USE_IRQ_PTC

#define _MK_xxx_H_
#define USE_POLL
#define TWR_K20D72M

/* Reset watchdog timer */
#ifndef __RESET_WATCHDOG
  #define __RESET_WATCHDOG()  (void)(WDOG_REFRESH = 0xA602, WDOG_REFRESH = 0xB480)
#endif 

#define _BDT_RESERVED_SECTION_

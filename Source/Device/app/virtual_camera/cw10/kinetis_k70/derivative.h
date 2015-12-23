/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand
 */

/* Include the derivative-specific header file */
#include <MK70F12.h>

#define __MK_xxx_H__
#define ASYNCH_MODE    /* PLL1 is source for MCGCLKOUT and DDR controller */

/* use pit0 interrupt */
#define USED_PIT0

#define printf printf_kinetis
#define sprintf sprintf_kinetis
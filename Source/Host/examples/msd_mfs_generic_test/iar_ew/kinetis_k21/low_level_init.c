/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2010
 *
 *    File name   : low_level_init.c
 *    Description : Low level init procedure
 *
 *    History :
 *    1. Date        : 08, September 2010
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *
 *    $Revision: #2 $
 **************************************************************************/
#include <derivative.h>

/*************************************************************************
 * Function Name: low_level_init
 * Parameters: none
 *
 * Return: none
 *
 * Description: This function is used for low level initialization
 *
 *************************************************************************/
int __low_level_init(void)
{
  unsigned int reg;
  /*Disable WWDT*/
  reg = WDOG_STCTRLH;
  reg &= ~1UL;
  /*Unlock sequence*/
  WDOG_UNLOCK  = 0xC520;
  WDOG_UNLOCK  = 0xD928;
  WDOG_STCTRLH = reg;
  /* perform initialization*/
  return 1;
}


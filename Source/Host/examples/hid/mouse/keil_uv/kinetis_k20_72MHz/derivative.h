/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand
 */

/* Include the derivative-specific header file */
#if (defined(TWR_K40X256))
  #include <MK40N512VMD100.h>
#elif (defined(TWR_K53N512))
  #include <MK53N512CMD100.h>
#elif (defined(TWR_K60N512))
  #include <MK60N512VMD100.h>
#elif (defined(TWR_K20D72M))
  #include <MK20D7.h>
#else
  #error "No valid platform defined"
#endif

#define _MK_xxx_H_
#define USE_POLL
#define USE_PIT1

#define _BDT_RESERVED_SECTION_
/*
** ###################################################################
**     Processor:           MK60N512MD100
**     Compilers:           ARM Compiler
**                          Freescale C/C++ for Embedded ARM
**                          GNU ARM C Compiler
**                          IAR ANSI C/C++ Compiler for ARM
**     Reference manual:    K60P144M100SF2RM, Rev. 3, 4 Nov 2010
**     Version:             rev. 1.6, 2011-01-14
**
**     Abstract:
**         Provides a system configuration function and a global variable that contains the system frequency.
**         It configures the device and initializes the oscillator (PLL) that is part of the microcontroller device.
**
**     Copyright: 2011 Freescale Semiconductor, Inc. All Rights Reserved.
**
**     http:                 www.freescale.com
**     mail:                 support@freescale.com
**
**     Revisions:
**     - rev. 0.1 (2010-09-29)
**         Initial version
**     - rev. 1.0 (2010-10-15)
**         First public version
**     - rev. 1.1 (2010-10-27)
**         Registers updated according to the new reference manual revision - Rev. 2, 15 Oct 2010
**         ADC - Peripheral register PGA bit definition has been fixed, bits PGALP, PGACHP removed.
**         CAN - Peripheral register MCR bit definition has been fixed, bit WAKSRC removed.
**         CRC - Peripheral register layout structure has been extended with 8/16-bit access to shadow registers.
**         CMP - Peripheral base address macro renamed from HSCMPx_BASE to CMPx_BASE.
**         CMP - Peripheral base pointer macro renamed from HSCMPx to CMPx.
**         DMA - Peripheral base address macro renamed from eDMA_BASE to DMA_BASE.
**         DMA - Peripheral base pointer macro renamed from eDMA to DMA.
**         ENET - Statistic event counter register MASK and SHIFT macros removed (#MTWX43372).
**         GPIO - Port Output Enable Register (POER) has been renamed to Port Data Direction Register (PDDR), all POER related macros fixed to PDDR.
**         PDB - Peripheral register layout structure has been extended for Channel n and DAC n register array access (#MTWX44115).
**         RFSYS - System regfile registers have been added (#MTWX43999)
**         RFVBAT - VBAT  regfile registers have been added (#MTWX43999)
**         RNG - Peripheral base address macro renamed from RNGB_BASE to RNG_BASE.
**         RNG - Peripheral base pointer macro renamed from RNGB to RNG.
**         RTC - Peripheral register CR bit definition has been fixed, bit OTE removed.
**         TSI - Peripheral registers STATUS, SCANC bit definition have been fixed, bit groups CAPTRM, DELVOL and AMCLKDIV added.
**         USB - Peripheral base address macro renamed from USBOTG0_BASE to USB0_BASE.
**         USB - Peripheral base pointer macro renamed from USBOTG0 to USB0.
**         VREF - Peripheral register TRM removed.
**     - rev. 1.2 (2010-11-11)
**         Registers updated according to the new reference manual revision - Rev. 3, 4 Nov 2010
**         CAN - Individual Matching Element Update (IMEU) feature has been removed.
**         CAN - Peripheral register layout structure has been fixed, registers IMEUR, LRFR have been removed.
**         CAN - Peripheral register CTRL2 bit definition has been fixed, bits IMEUMASK, LOSTRMMSK, LOSTRLMSK, IMEUEN have been removed.
**         CAN - Peripheral register ESR2 bit definition has been fixed, bits IMEUF, LOSTRMF, LOSTRLF have been removed.
**         NV - Fixed offset address of BACKKEYx, FPROTx registers.
**         TSI - Peripheral register layout structure has been fixed, register WUCNTR has been removed.
**     - rev. 1.3 (2010-11-19)
**         CAN - Support for CAN0_IMEU_IRQn, CAN0_Lost_Rx_IRQn interrupts has been removed.
**         CAN - Support for CAN1_IMEU_IRQn, CAN1_Lost_Rx_IRQn interrupts has been removed.
**     - rev. 1.4 (2010-11-30)
**         EWM - Peripheral base address EWM_BASE definition has been fixed from 0x4005F000u to 0x40061000u (#MTWX44776).
**     - rev. 1.5 (2010-12-17)
**         AIPS0, AIPS1 - Fixed offset of PACRE-PACRP registers (#MTWX45259).
**         CAU - Fixed register definition.
**     - rev. 1.6 (2011-01-14)
**         Added BITBAND_REG() macro to provide access to register bits using bit band region.
**
** ###################################################################
*/

/*! \file MK60N512MD100 */
/*! \version 1.6 */
/*! \date 2011-01-14 */
/*! \brief Device specific configuration file for MK60N512MD100 (implementation file) */
/*! \detailed Provides a system configuration function and a global variable that contains the system frequency.
      It configures the device and initializes the oscillator (PLL) that is part of the microcontroller device. */

#include <stdint.h>
#include "MK60N512VMD100.h"

/* ----------------------------------------------------------------------------
   -- SystemInit()
   ---------------------------------------------------------------------------- */

void SystemInit (void) {
  /* Disable the WDOG module */
  /* WDOG_UNLOCK: WDOGUNLOCK=0xC520 */
  WDOG_UNLOCK = (uint16_t)0xC520u;     /* Key 1 */
  /* WDOG_UNLOCK : WDOGUNLOCK=0xD928 */
  WDOG_UNLOCK  = (uint16_t)0xD928u;    /* Key 2 */
  /* WDOG_STCTRLH: ??=0,DISTESTWDOG=0,BYTESEL=0,TESTSEL=0,TESTWDOG=0,??=0,STNDBYEN=1,WAITEN=1,STOPEN=1,DBGEN=0,ALLOWUPDATE=1,WINEN=0,IRQRSTEN=0,CLKSRC=1,WDOGEN=0 */
  WDOG_STCTRLH = (uint16_t)0x01D2u;
}

/* ----------------------------------------------------------------------------
   -- SystemCoreClockUpdate()
   ---------------------------------------------------------------------------- */

void SystemCoreClockUpdate (void) {
}

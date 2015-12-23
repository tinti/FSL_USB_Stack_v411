/*
 * File:		mcf52277_sysinit.h
 * Purpose:		Power-on Reset configuration of the MCF52277.
 *
 * Notes:
 *
 */

#ifndef __MCF52277_SYSINIT_H__
#define __MCF52277_SYSINIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if ENABLE_UART_SUPPORT==1

/*** 
 * System Bus Clock Info 
 */

#define SYSTEM_CLOCK_KHZ  68000    /* system bus frequency in kHz*/
#define SYSTEM_PERIOD       12    /* system bus period in ns */
/*** 
 * Serial Port Info
 * The baud rate to be : 19200
 * Data bits : 8
 * Parity : None
 * Stop Bits : 1
 * Flow Control : None 
 */
#define TERMINAL_PORT       (0)
#define TERMINAL_BAUD       kBaud19200
#endif  //ENABLE_UART_SUPPORT==1 

/*
 * Ethernet Port Info
 */
#define FEC_PHY0            (0x00)


/*
 *  SDRAM Timing Parameters
 */  
#define SDRAM_BL             	8      /* # of beats in a burst */
#define SDRAM_TWR               2      /* in clocks */
#define SDRAM_CASL          	3      /* CASL in clocks */
#define SDRAM_TRCD              2      /* in clocks */
#define SDRAM_TRP               2      /* in clocks */
#define SDRAM_TRFC              6      /* in clocks */
#define SDRAM_TREFI             7800   /* in ns */


/*** 
 * Board Memory map definitions from linker command files:
 * __SDRAM,__SDRAM_SIZE, __FLASH, __FLASH_SIZE linker 
 * symbols must be defined in the linker command file.
 */
extern __declspec(system) uint8 __FLASH[];
extern __declspec(system) uint8 __SDRAM[];
extern __declspec(system) uint8 __FLASH_SIZE[];
extern __declspec(system) uint8 __SDRAM_SIZE[];


#define FLASH_ADDRESS   (uint32)__FLASH
#define SDRAM_ADDRESS	(uint32)__SDRAM	

#define FLASH_SIZE    (uint32)__FLASH_SIZE
#define SDRAM_SIZE	  (uint32)__SDRAM_SIZE
/********************************************************************/
/* __initialize_hardware Startup code routine
 * 
 * __initialize_hardware is called by the startup code right after reset, 
 * with interrupt disabled and SP pre-set to a valid memory area.
 * Here you should initialize memory and some peripherics;
 * at this point global variables are not initialized yet.
 * The startup code will initialize SP on return of this function.
 */
void __initialize_hardware(void);

/********************************************************************/
/* __initialize_system Startup code routine
 * 
 * __initialize_system is called by the startup code when all languages 
 * specific initialization are done to allow additional hardware setup.
 */ 
void __initialize_system(void);

asm void mcf5xxx_wr_cacr(unsigned long);


#ifdef __cplusplus
}
#endif

#endif /* __MCF52277_SYSINIT_H__ */



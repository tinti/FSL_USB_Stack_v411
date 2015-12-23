/**************************************************
 *
 * Copyright 2012 IAR Systems. All rights reserved.
 *
 * $Revision: #2 $
 *
 **************************************************/

;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:ROOT(2)

        EXTERN  __iar_program_start
        PUBLIC  ___VECTOR_RAM
        PUBLIC  __vector_table
        EXTERN  USB_ISR
        EXTERN  PIT_ISR
        EXTERN  IRQ_ISR_PORTA
        DATA
___VECTOR_RAM
__vector_table
        DCD     sfe(CSTACK)               ; Top of Stack
        DCD     __iar_program_start       ; Reset Handler
        DCD     NMI_Handler               ; NMI Handler
        DCD     HardFault_Handler         ; Hard Fault Handler
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     SVC_Handler               ; SVCall Handler
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     PendSV_Handler            ; PendSV Handler
        DCD     SysTick_Handler           ; SysTick Handler
        ; External Interrupts
        DCD     DMA0_IRQHandler           ; 0:  DMA Channel 0 transfer complete
        DCD     DMA1_IRQHandler           ; 1:  DMA Channel 1 transfer complete
        DCD     DMA2_IRQHandler           ; 2:  DMA Channel 2 transfer complete
        DCD     DMA3_IRQHandler           ; 3:  DMA Channel 3 transfer complete
        DCD     0                         ; Reserved
        DCD     FLASH_RC_IRQHandler       ; 5:  Flash memory read collision
        DCD     VLD_IRQHandler            ; 6:  Low Voltage Detect, Low Voltage Warning
        DCD     LLWU_IRQHandler           ; 7:  Low Leakage Wakeup
        DCD     I2C0_IRQHandler           ; 8:  I2C0 interrupt
        DCD     I2C1_IRQHandler           ; 9:  I2C1 interrupt
        DCD     SPI0_IRQHandler           ;10:  SPI 0 interrupt
        DCD     SPI1_IRQHandler           ;11:  SPI 1 interrupt
        DCD     UART0_ERR_IRQHandler	  ;12:  UART 0 error intertrupt
        DCD     UART1_ERR_IRQHandler	  ;13:  UART 1 error intertrupt
        DCD     UART2_ERR_IRQHandler      ;14:  UART 2 error intertrupt
        DCD     ADC0_IRQHandler 	  ;15:  ADC 0 interrupt
        DCD     CMP0_IRQHandler		  ;16:  CMP 0 High-speed comparator interrupt
        DCD     FTM0_IRQHandler           ;17:  FTM 0 interrupt
        DCD     FTM1_IRQHandler           ;18:  FTM 1 interrupt
        DCD     FTM2_IRQHandler           ;19:  FTM 2 interrupt
        DCD     RTC_IRQHandler            ;20:  RTC intrrupt
        DCD     RTC_SE_IRQHandler         ;21:  RTC second interrupt
        DCD     PIT_ISR            ;22:  PIT interrupt
        DCD     0                         ; Reserved
        DCD     USB_ISR                   ;24:  USB 0 interrupt
        DCD     DAC0_IRQHandler           ;25:  DAC 0 interrupt
        DCD     TSI0_IRQHandler           ;26:  TSI 0 interrupt
        DCD     MCG_IRQHandler         	  ;27:  MCG interrupt
        DCD     LPT_IRQHandler            ;28:  LPT interrupt
        DCD     0                         ; Reserved
        DCD     IRQ_ISR_PORTA          ;30:  PORT A interrupt
        DCD     PORTD_IRQHandler          ;31:  PORT D interrupt
        
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;

      PUBWEAK NMI_Handler
      PUBWEAK HardFault_Handler
      PUBWEAK SVC_Handler
      PUBWEAK PendSV_Handler
      PUBWEAK SysTick_Handler
      PUBWEAK DMA0_IRQHandler
      PUBWEAK DMA1_IRQHandler
      PUBWEAK DMA2_IRQHandler
      PUBWEAK DMA3_IRQHandler
      PUBWEAK FLASH_RC_IRQHandler
      PUBWEAK VLD_IRQHandler
      PUBWEAK LLWU_IRQHandler
      PUBWEAK I2C0_IRQHandler
      PUBWEAK I2C1_IRQHandler
      PUBWEAK SPI0_IRQHandler
      PUBWEAK SPI1_IRQHandler
      PUBWEAK UART0_ERR_IRQHandler
      PUBWEAK UART1_ERR_IRQHandler
      PUBWEAK UART2_ERR_IRQHandler
      PUBWEAK ADC0_IRQHandler
      PUBWEAK CMP0_IRQHandler
      PUBWEAK FTM0_IRQHandler
      PUBWEAK FTM1_IRQHandler
      PUBWEAK FTM2_IRQHandler
      PUBWEAK RTC_IRQHandler
      PUBWEAK RTC_SE_IRQHandler
      PUBWEAK PIT_IRQHandler
      PUBWEAK USB0_IRQHandler
      PUBWEAK DAC0_IRQHandler
      PUBWEAK TSI0_IRQHandler
      PUBWEAK MCG_IRQHandler
      PUBWEAK LPT_IRQHandler
      PUBWEAK PORTA_IRQHandler
      PUBWEAK PORTD_IRQHandler

      SECTION .text:CODE:REORDER(1)
      THUMB
NMI_Handler
HardFault_Handler
SVC_Handler
PendSV_Handler
SysTick_Handler
DMA0_IRQHandler
DMA1_IRQHandler
DMA2_IRQHandler
DMA3_IRQHandler
FLASH_RC_IRQHandler
VLD_IRQHandler
LLWU_IRQHandler
I2C0_IRQHandler
I2C1_IRQHandler
SPI0_IRQHandler
SPI1_IRQHandler
UART0_ERR_IRQHandler
UART1_ERR_IRQHandler
UART2_ERR_IRQHandler
ADC0_IRQHandler
CMP0_IRQHandler
FTM0_IRQHandler
FTM1_IRQHandler
FTM2_IRQHandler
RTC_IRQHandler
RTC_SE_IRQHandler
PIT_IRQHandler
USB0_IRQHandler
DAC0_IRQHandler
TSI0_IRQHandler
MCG_IRQHandler
LPT_IRQHandler
PORTA_IRQHandler
PORTD_IRQHandler
Default_Handler
k
        B Default_Handler
        END

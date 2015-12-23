/**************************************************
 *
 * Copyright 2010 IAR Systems. All rights reserved.
 *
 * $Revision: #1 $
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
        EXTERN  Timer_ISR        
        EXTERN  IRQ_ISR_PORTC		

        DATA
___VECTOR_RAM
__vector_table
        DCD     sfe(CSTACK)               ; Top of Stack
        DCD     __iar_program_start       ; Reset Handler
        DCD     NMI_Handler               ; NMI Handler
        DCD     HardFault_Handler         ; Hard Fault Handler
        DCD     MemManage_Handler         ; MPU Fault Handler
        DCD     BusFault_Handler          ; Bus Fault Handler
        DCD     UsageFault_Handler        ; Usage Fault Handler
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     SVC_Handler               ; SVCall Handler
        DCD     DebugMon_Handler          ; Debug Monitor Handler
        DCD     0                         ; Reserved
        DCD     PendSV_Handler            ; PendSV Handler
        DCD     SysTick_Handler           ; SysTick Handler
        ; External Interrupts
        DCD     DMA0_IRQHandler           ; 0:  DMA Channel 0 transfer complete
        DCD     DMA1_IRQHandler           ; 1:  DMA Channel 1 transfer complete 
        DCD     DMA2_IRQHandler           ; 2:  DMA Channel 2 transfer complete
        DCD     DMA3_IRQHandler           ; 3:  DMA Channel 3 transfer complete
        DCD     DMA4_IRQHandler           ; 4:  DMA Channel 4 transfer complete
        DCD     DMA5_IRQHandler           ; 5:  DMA Channel 5 transfer complete
        DCD     FLASH_CC_IRQHandler       ; 6:  Flash memory command complete
        DCD     FLASH_RC_IRQHandler       ; 7:  Flash memory read collision
        DCD     VLD_IRQHandler            ; 8:  Low Voltage Detect, Low Voltage Warning
        DCD     LLWU_IRQHandler           ; 9:  Low Leakage Wakeup
        DCD     WDOG_IRQHandler           ;10:  WDOG interrupt
        DCD     I2C0_IRQHandler           ;11:  I2C0 interrupt
        DCD     SPI0_IRQHandler           ;12:  SPI 0 interrupt
        DCD     I2S0_IRQHandler           ;13:  I2S transmit interrupt
        DCD     I2S1_IRQHandler           ;14:  I2S receive interrupt
        DCD     UART0_CEA709IRQHandler    ;15:  UART 0 intertrupt
        DCD     UART0_IRQHandler          ;16:  UART 0 intertrupt
        DCD     UART0_ERR_IRQHandler      ;17:  UART 0 error intertrupt        
        DCD     UART1_IRQHandler          ;18:  UART 1 intertrupt
        DCD     UART1_ERR_IRQHandler      ;19:  UART 1 error intertrupt
        DCD     UART2_IRQHandler          ;20:  UART 2 intertrupt
        DCD     UART2_ERR_IRQHandler      ;21:  UART 2 error intertrupt        
        DCD     ADC0_IRQHandler           ;22:  ADC 0 interrupt
        DCD     CMP0_IRQHandler           ;23:  CMP 0 High-speed comparator interrupt
        DCD     CMP1_IRQHandler           ;24:  CMP 1 interrupt
        DCD     FTM0_IRQHandler           ;25:  FTM 0 interrupt
        DCD     FTM1_IRQHandler           ;26:  FTM 1 interrupt
        DCD     CMT_IRQHandler            ;27:  CMT intrrupt
        DCD     RTC_AlarmIRQHandler       ;28:  RTC alarm interrupt
        DCD     RTC_SecondsIRQHandler     ;29:  RTC seconds interrupt
        DCD     PIT0_IRQHandler           ;30:  PIT 0 interrupt
        DCD     PIT1_IRQHandler           ;31:  PIT 1 interrupt
        DCD     PIT2_IRQHandler           ;32:  PIT 2 interrupt
        DCD     PIT3_IRQHandler           ;33:  PIT 3 interrupt
        DCD     PDB_IRQHandler            ;34:  PDB interrupt
        DCD     USB_ISR                   ;35:  USB OTG interrupt
        DCD     USB_CD_IRQHandler         ;36:  USB Charger Detect interrupt
        DCD     TSI_IRQHandler            ;37:  TSI interrupt
        DCD     MCG_IRQHandler            ;38:  MCG interrupt
        DCD     Timer_ISR                 ;39:  LPT interrupt
        DCD     PORTA_IRQHandler          ;40:  PORT A interrupt
        DCD     PORTB_IRQHandler          ;41:  PORT B interrupt
        DCD     IRQ_ISR_PORTC             ;42:  PORT C interrupt
        DCD     PORTD_IRQHandler          ;43:  PORT D interrupt
        DCD     PORTE_IRQHandler          ;44:  PORT E interrupt

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;

      PUBWEAK NMI_Handler
      PUBWEAK HardFault_Handler
      PUBWEAK MemManage_Handler
      PUBWEAK BusFault_Handler
      PUBWEAK UsageFault_Handler
      PUBWEAK SVC_Handler
      PUBWEAK DebugMon_Handler
      PUBWEAK PendSV_Handler
      PUBWEAK SysTick_Handler
      PUBWEAK DMA0_IRQHandler
      PUBWEAK DMA1_IRQHandler
      PUBWEAK DMA2_IRQHandler
      PUBWEAK DMA3_IRQHandler
      PUBWEAK DMA4_IRQHandler
      PUBWEAK DMA5_IRQHandler
      PUBWEAK FLASH_CC_IRQHandler
      PUBWEAK FLASH_RC_IRQHandler
      PUBWEAK VLD_IRQHandler
      PUBWEAK LLWU_IRQHandler
      PUBWEAK WDOG_IRQHandler
      PUBWEAK I2C0_IRQHandler
      PUBWEAK SPI0_IRQHandler
      PUBWEAK I2S0_IRQHandler
      PUBWEAK I2S1_IRQHandler
      PUBWEAK UART0_CEA709IRQHandler
      PUBWEAK UART0_IRQHandler      
      PUBWEAK UART0_ERR_IRQHandler  
      PUBWEAK UART1_IRQHandler      
      PUBWEAK UART1_ERR_IRQHandler  
      PUBWEAK UART2_IRQHandler      
      PUBWEAK UART2_ERR_IRQHandler  
      PUBWEAK ADC0_IRQHandler       
      PUBWEAK CMP0_IRQHandler       
      PUBWEAK CMP1_IRQHandler       
      PUBWEAK FTM0_IRQHandler       
      PUBWEAK FTM1_IRQHandler       
      PUBWEAK CMT_IRQHandler        
      PUBWEAK RTC_AlarmIRQHandler   
      PUBWEAK RTC_SecondsIRQHandler 
      PUBWEAK PIT0_IRQHandler       
      PUBWEAK PIT1_IRQHandler       
      PUBWEAK PIT2_IRQHandler       
      PUBWEAK PIT3_IRQHandler       
      PUBWEAK PDB_IRQHandler        
      PUBWEAK USB_OTG_IRQHandler               
      PUBWEAK USB_CD_IRQHandler     
      PUBWEAK TSI_IRQHandler        
      PUBWEAK MCG_IRQHandler        
      PUBWEAK LPT_IRQHandler             
      PUBWEAK PORTA_IRQHandler         
      PUBWEAK PORTB_IRQHandler      
      PUBWEAK PORTC_IRQHandler         
      PUBWEAK PORTD_IRQHandler      
      PUBWEAK PORTE_IRQHandler     
      
      SECTION .text:CODE:REORDER(1)
      THUMB
NMI_Handler
HardFault_Handler
MemManage_Handler
BusFault_Handler
UsageFault_Handler
SVC_Handler
DebugMon_Handler
PendSV_Handler
SysTick_Handler
DMA0_IRQHandler
DMA1_IRQHandler
DMA2_IRQHandler
DMA3_IRQHandler
DMA4_IRQHandler
DMA5_IRQHandler
FLASH_CC_IRQHandler
FLASH_RC_IRQHandler
VLD_IRQHandler
LLWU_IRQHandler
WDOG_IRQHandler
I2C0_IRQHandler
SPI0_IRQHandler
I2S0_IRQHandler
I2S1_IRQHandler
UART0_CEA709IRQHandler
UART0_IRQHandler      
UART0_ERR_IRQHandler  
UART1_IRQHandler      
UART1_ERR_IRQHandler  
UART2_IRQHandler      
UART2_ERR_IRQHandler  
ADC0_IRQHandler       
CMP0_IRQHandler       
CMP1_IRQHandler       
FTM0_IRQHandler       
FTM1_IRQHandler       
CMT_IRQHandler        
RTC_AlarmIRQHandler   
RTC_SecondsIRQHandler 
PIT0_IRQHandler       
PIT1_IRQHandler       
PIT2_IRQHandler       
PIT3_IRQHandler       
PDB_IRQHandler        
USB_OTG_IRQHandler               
USB_CD_IRQHandler     
TSI_IRQHandler        
MCG_IRQHandler        
LPT_IRQHandler             
PORTA_IRQHandler         
PORTB_IRQHandler      
PORTC_IRQHandler         
PORTD_IRQHandler      
PORTE_IRQHandler     
Default_Handler

        B Default_Handler
        END

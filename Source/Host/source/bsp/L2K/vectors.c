#include "derivative.h" /* include peripheral declarations */

/*--------------------------------------------------------------*/
typedef void (*const tIsrFunc)(void);
typedef struct 
{
    uint32_t*   __ptr;
#if (defined MCU_MKL25Z4)
    tIsrFunc __func[0x2F];
#endif   
} tVectorTable;

extern void USB_ISR();
#ifdef USE_IRQ
    extern void IRQ_ISR_PORTC(void);
    extern void IRQ_ISR_PORTA(void);
    extern void IRQ_ISR_PORTE(void);
#endif 

#if((defined USE_PIT0)||(defined USE_PIT1))
    extern void  PIT_ISR(void);
#endif
  
#ifdef USE_LPT
    extern void Timer_ISR(void);
#endif
    
extern void __thumb_startup( void );
extern uint32_t __SP_INIT[];
  
#pragma define_section vectortable ".vectortable" ".vectortable" ".vectortable" far_abs R

 void Cpu_INT_NMIInterrupt(void)
 {
     
 }

 void Cpu_Interrupt(void)
 {
     printf("\n\rISR is entered on vector number 0x%x\n\r", (*(volatile uint8_t*)(0xE000ED04)));
 }
 
 /*lint -save  -e926 -e927 -e928 -e929 Disable MISRA rule (11.4) checking. Need to explicitly cast pointers to the general ISR for Interrupt vector table */
const tVectorTable __vector_table __attribute__ ((section(".vectortable"))) = {
   /* ISR name                             No. Address      Pri Name                           Description */
   (uint32_t *)__SP_INIT,                  /* 0x00  0x00000000   -   ivINT_Initial_Stack_Pointer    used by PE */
   {
#if (defined MCU_MKL25Z4)
        (tIsrFunc)__thumb_startup,         /* 0x01  0x00000004   -   ivINT_Initial_Program_Counter  used by PE */
        (tIsrFunc)Cpu_INT_NMIInterrupt,    /* 0x02  0x00000008   -2   ivINT_NMI                     used by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x03  0x0000000C   -1   ivINT_Hard_Fault              unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x04  0x00000010   -   ivINT_Reserved4                unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x05  0x00000014   -   ivINT_Reserved5                unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x06  0x00000018   -   ivINT_Reserved6                unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x07  0x0000001C   -   ivINT_Reserved7                unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x08  0x00000020   -   ivINT_Reserved8                unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x09  0x00000024   -   ivINT_Reserved9                unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x0A  0x00000028   -   ivINT_Reserved10               unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x0B  0x0000002C   -   ivINT_SVCall                   unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x0C  0x00000030   -   ivINT_Reserved12               unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x0D  0x00000034   -   ivINT_Reserved13               unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x0E  0x00000038   -   ivINT_PendableSrvReq           unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x0F  0x0000003C   -   ivINT_SysTick                  unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x10  0x00000040   -   ivINT_DMA0                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x11  0x00000044   -   ivINT_DMA1                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x12  0x00000048   -   ivINT_DMA2                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x13  0x0000004C   -   ivINT_DMA3                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x14  0x00000050   -   ivINT_Reserved20               unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x15  0x00000054   -   ivINT_Read_Collision           unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x16  0x00000058   -   ivINT_LVD_LVW                  unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x17  0x0000005C   -   ivINT_LLW                      unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x18  0x00000060   -   ivINT_I2C0                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x19  0x00000064   -   ivINT_I2C1                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x1A  0x00000068   -   ivINT_SPI0                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x1B  0x0000006C   -   ivINT_SPI1                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x1C  0x00000070   -   ivINT_UART0_ERR                unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x1D  0x00000074   -   ivINT_UART1_ERR                unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x1E  0x00000078   -   ivINT_UART2_ERR                unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x1F  0x0000007C   -   ivINT_ADC0                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x20  0x00000080   -   ivINT_HSCMP0                   unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x21  0x00000084   -   ivINT_FTM0                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x22  0x00000088   -   ivINT_FTM1                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x23  0x0000008C   -   ivINT_FTM2                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x24  0x00000090   -   ivINT_RTC                      unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x25  0x00000094   -   ivINT_RTC_SecontInt            unused by PE */
#if((defined USE_PIT0)||(defined USE_PIT1))
        (tIsrFunc)PIT_ISR,                 /* 0x26  0x00000098   -   ivINT_PIT                      unused by PE */
#else 
        (tIsrFunc)Cpu_Interrupt,           /* 0x26  0x00000098   -   ivINT_PIT                      unused by PE */
#endif
        (tIsrFunc)Cpu_Interrupt,           /* 0x27  0x0000009C   -   ivINT_Reserved39               unused by PE */
        (tIsrFunc)USB_ISR,                 /* 0x28  0x000000A0   -   ivINT_USB0                     USB_ISR      */
        (tIsrFunc)Cpu_Interrupt,           /* 0x29  0x000000A4   -   ivINT_DAC0                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x2A  0x000000A8   -   ivINT_TSI0                     unused by PE */
        (tIsrFunc)Cpu_Interrupt,           /* 0x2B  0x000000AC   -   ivINT_MCG                      unused by PE */
#ifdef USE_LPT
        (tIsrFunc)Timer_ISR,               /* 0x2C  0x000000B0   -   ivINT_LPTimer                  unused by PE */
#else
        (tIsrFunc)Cpu_Interrupt,           /* 0x2C  0x000000B0   -   ivINT_LPTimer                  unused by PE */
#endif
        (tIsrFunc)Cpu_Interrupt,           /* 0x2D  0x000000B4   -   ivINT_Reserved44               unused by PE */
#ifdef USE_IRQ
        (tIsrFunc)IRQ_ISR_PORTA,           /* 0x2E  0x000000B8   -   ivINT_PORTA                    unused by PE */
#else
        (tIsrFunc)Cpu_Interrupt,           /* 0x2E  0x000000B8   -   ivINT_PORTA                    unused by PE */
#endif
        (tIsrFunc)Cpu_Interrupt,           /* 0x2F  0x000000BC   -   ivINT_PORTD                    unused by PE */
#endif
   }
 };

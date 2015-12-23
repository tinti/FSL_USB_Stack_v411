/******************************************************************************
*  (c) copyright Freescale Semiconductor China Ltd. 2008
*  ALL RIGHTS RESERVED
*  File Name: SPI.C
*  Description: This file is to handle SPI communication
*  Assembler:  Codewarrior for HC(S)08 V6.1
*  Version: 2.0
*  Author: Patrick Yang
*  Location: Shanghai, P.R.China
*
* UPDATED HISTORY:
*
* REV   YYYY.MM.DD  AUTHOR        DESCRIPTION OF CHANGE
* ---   ----------  ------        --------------------- 
* 1.0   2008.01.02  Patrick Yang  Initial version
* 2.0   2008.08.12  Jose Ruiz     Speed Change (x2)
******************************************************************************/                                                                        
/* Freescale  is  not  obligated  to  provide  any  support, upgrades or new */
/* releases  of  the Software. Freescale may make changes to the Software at */
/* any time, without any obligation to notify or provide updated versions of */
/* the  Software  to you. Freescale expressly disclaims any warranty for the */
/* Software.  The  Software is provided as is, without warranty of any kind, */
/* either  express  or  implied,  including, without limitation, the implied */
/* warranties  of  merchantability,  fitness  for  a  particular purpose, or */
/* non-infringement.  You  assume  the entire risk arising out of the use or */
/* performance of the Software, or any systems you design using the software */
/* (if  any).  Nothing  may  be construed as a warranty or representation by */
/* Freescale  that  the  Software  or  any derivative work developed with or */
/* incorporating  the  Software  will  be  free  from  infringement  of  the */
/* intellectual property rights of third parties. In no event will Freescale */
/* be  liable,  whether in contract, tort, or otherwise, for any incidental, */
/* special,  indirect, consequential or punitive damages, including, but not */
/* limited  to,  damages  for  any loss of use, loss of time, inconvenience, */
/* commercial loss, or lost profits, savings, or revenues to the full extent */
/* such  may be disclaimed by law. The Software is not fault tolerant and is */
/* not  designed,  manufactured  or  intended by Freescale for incorporation */
/* into  products intended for use or resale in on-line control equipment in */
/* hazardous, dangerous to life or potentially life-threatening environments */
/* requiring  fail-safe  performance,  such  as  in the operation of nuclear */
/* facilities,  aircraft  navigation  or  communication systems, air traffic */
/* control,  direct  life  support machines or weapons systems, in which the */
/* failure  of  products  could  lead  directly to death, personal injury or */
/* severe  physical  or  environmental  damage  (High  Risk Activities). You */
/* specifically  represent and warrant that you will not use the Software or */
/* any  derivative  work of the Software for High Risk Activities.           */
/* Freescale  and the Freescale logos are registered trademarks of Freescale */
/* Semiconductor Inc.                                                        */ 
/*****************************************************************************/

/* Includes */
#include "types.h"
#include "SPI_cfv2.h"
#include "wdt_cfv2.h"

#ifdef __MCF52277_H__
    /*  Global variable defining the baud rate; this variable can be touched by 
        SPI_Init() and SPI_HighRate() to change the baud rate from 375KHz (normal 
        baud rate) to 6 MHz (high rate) */
    static uint_32 gSPI_BaudRate;
    static uint_32 gSPI_BeforeTransfDelay, gSPI_AfterTransfDelay, gSPI_InterTransfDelay;
#endif

/*********************************************************
* Name: SPI_Init
* Desc: Initialize SPI2 Module
* Parameter: None
* Return: None             
**********************************************************/
void SPI_Init(void)
{
    /*Body*/
  #if(defined(__MCF52259_H__) || defined(__MCF52221_H__))
    /* QSPI Clock: 375KHz */
    MCF_QSPI_QMR = MCF_QSPI_QMR_BAUD(0x6A);
    /* Number of bits in a transfer operation: field BITS = 8 */
    MCF_QSPI_QMR |= MCF_QSPI_QMR_BITS(8);
    /* Enable master mode (MSTR = 1); CPHA = CPOL = 0 */
    MCF_QSPI_QMR |= MCF_QSPI_QMR_MSTR;
  #endif
  #ifdef __MCF52277_H__
    /*  Enable master mode, disable both transmit and receive FIFO buffers, 
        set the inactive state for PCS2 high, enable the module (MDIS = 0), 
        delay the sample point from the leading edge of the clock and halt 
        any transfer */
    MCF_DSPI_DSPI_MCR = (   MCF_DSPI_DSPI_MCR_MSTR          | 
                            MCF_DSPI_DSPI_MCR_PCSIS0        | 
                            MCF_DSPI_DSPI_MCR_SMPL_PT_2CLK  |
                            MCF_DSPI_DSPI_MCR_HALT); 

    MCF_DSPI_DSPI_MCR |= (MCF_DSPI_DSPI_MCR_DIS_RXF | MCF_DSPI_DSPI_MCR_DIS_TXF);
    /* DSPI clock 375 KHz */
    /* The system clock fsys = 68 MHz */
    /* Value to be passed in SPI_Send_byte() function to DPI_CTAR0 register */
    gSPI_BaudRate = (MCF_DSPI_DSPI_CTAR_PBR_3CLK | MCF_DSPI_DSPI_CTAR_BR(0x06)); 
    /* Configure the rest of the delays */
    gSPI_BeforeTransfDelay = (MCF_DSPI_DSPI_CTAR_CSSCK(1)  | MCF_DSPI_DSPI_CTAR_CSSCK(0x04));
    gSPI_AfterTransfDelay  = (MCF_DSPI_DSPI_CTAR_PASC_3CLK | MCF_DSPI_DSPI_CTAR_ASC(0x04));
    gSPI_InterTransfDelay  = (MCF_DSPI_DSPI_CTAR_PDT_3CLK  | MCF_DSPI_DSPI_CTAR_DT(0x05));  
  #endif
}/*EndBody*/



/*********************************************************
* Name: SPI_Send_byte
* Desc: Send one byte 
* Parameter: The byte to be sent
* Return: None
**********************************************************/
void SPI_Send_byte(uint_8 u8Data)
{
    /*Body*/
    #if(defined(__MCF52259_H__) || defined(__MCF52221_H__))
        /* Check the status flag */
        if(MCF_QSPI_QIR & MCF_QSPI_QIR_SPIF)
        {
            /* Write a 1 to clear it */
            MCF_QSPI_QIR |= MCF_QSPI_QIR_SPIF;
        }/*EndIf*/
        
        /* Point the Address Register to the Command RAM */
        MCF_QSPI_QAR = MCF_QSPI_QAR_ADDR(MCF_QSPI_QAR_CMD);
        /* Write the Command RAM using the Data Register */
        /* CONT = 0, BITSE = 1, DT= 0, DSCK = 0 */
        MCF_QSPI_QDR = MCF_QSPI_QDR_BITSE | (~MCF_QSPI_QDR_QSPI_CS0 & 0x0F00); 

        /* Point the Address Register to the Transmit RAM */
        MCF_QSPI_QAR = MCF_QSPI_QAR_ADDR(MCF_QSPI_QAR_TRANS);
        /* Write the byte in the transmit RAM at the first location */
        MCF_QSPI_QDR = (uint16)MCF_QSPI_QDR_DATA(u8Data);  

        /* Configure the NEWQP and ENDQP both to zero */
        MCF_QSPI_QWR = MCF_QSPI_QWR_NEWQP(0) | MCF_QSPI_QWR_ENDQP(0) | MCF_QSPI_QWR_CSIV; 
        
        /* Initiates the transfer by seting the SPE bit in the QDLYR register */
        MCF_QSPI_QDLYR |= MCF_QSPI_QDLYR_SPE; 

        /* Wait to complete the current transfer */
        while(!(MCF_QSPI_QIR & MCF_QSPI_QIR_SPIF))
        {
            Watchdog_Reset();
        }/*EndWhile*/
        /* Write a 1 to clear it */
        MCF_QSPI_QIR |= MCF_QSPI_QIR_SPIF;
    #endif
    #ifdef __MCF52277_H__
        /* Check the status flag */
        if(MCF_DSPI_DSPI_SR & MCF_DSPI_DSPI_SR_EOQF)
        {
            /* Clear the EOQF by writting a 1 to it */
            MCF_DSPI_DSPI_SR |= MCF_DSPI_DSPI_SR_EOQF;
        }/*EndIf*/

        /* Write the DSPI_PUSHR register */
        MCF_DSPI_DSPI_PUSHR = ( MCF_DSPI_DSPI_PUSHR_CTAS(0) | 
                                MCF_DSPI_DSPI_PUSHR_EOQ     |
                                MCF_DSPI_DSPI_PUSHR_CTCNT   |
                                MCF_DSPI_DSPI_PUSHR_PCS0    |
                                MCF_DSPI_DSPI_PUSHR_TXDATA((uint_32)u8Data)); 
        /* Write the clock and transfer attributes: master clock and frame size (8 bits) */
        MCF_DSPI_DSPI_CTAR0 = ( MCF_DSPI_DSPI_CTAR_FMSZ(7)  | 
                                gSPI_BeforeTransfDelay      |
                                gSPI_AfterTransfDelay       |
                                gSPI_InterTransfDelay       |
                                gSPI_BaudRate);

        /* Start the transfer */
        MCF_DSPI_DSPI_MCR &= ~MCF_DSPI_DSPI_MCR_HALT;
        /* Wait until the transfer has been finished */
        while(!(MCF_DSPI_DSPI_SR & MCF_DSPI_DSPI_SR_EOQF))
        {
            Watchdog_Reset();
        }/*EndWhile*/
        /* Clear the EOQF by writting a 1 to it */
        MCF_DSPI_DSPI_SR |= MCF_DSPI_DSPI_SR_EOQF;
    #endif
}/*EndBody*/



/*********************************************************
* Name: SPI_Receive_byte
* Desc: The byte received by SPI  
* Parameter: None
* Return: Received byte
**********************************************************/
uint_8 SPI_Receive_byte(void)
{
    /*Body*/
    #if( defined(__MCF52259_H__) || defined(__MCF52221_H__))
        uint16 u8Data;
        
        /* Check the status flag */
        if(MCF_QSPI_QIR & MCF_QSPI_QIR_SPIF)
        {
            /* Write a 1 to clear it */
            MCF_QSPI_QIR |= MCF_QSPI_QIR_SPIF;
            }
            
        /* Point the Address Register to the Transmit RAM */
        MCF_QSPI_QAR = MCF_QSPI_QAR_ADDR(MCF_QSPI_QAR_TRANS);
        /* Write the byte in the transmit RAM at the first location */
        MCF_QSPI_QDR = (uint16)MCF_QSPI_QDR_DATA(0xFF);  

        /* Point the Address Register to the Command RAM */
        MCF_QSPI_QAR = MCF_QSPI_QAR_ADDR(MCF_QSPI_QAR_CMD);
        /* Write the Command RAM using the Data Register */
        /* CONT = 0, BITSE = 1, DT= 0, DSCK = 0 */
        MCF_QSPI_QDR = MCF_QSPI_QDR_BITSE | (~MCF_QSPI_QDR_QSPI_CS0 & 0x0F00); 

        /* Configure the NEWQP and ENDQP both to zero */
        MCF_QSPI_QWR = MCF_QSPI_QWR_NEWQP(0) | MCF_QSPI_QWR_ENDQP(0) | MCF_QSPI_QWR_CSIV; 

        /* Initiates the transfer by seting the SPE bit in the QDLYR register */
        MCF_QSPI_QDLYR |= MCF_QSPI_QDLYR_SPE; 

        /* Wait to complete the current transfer */
        while(!(MCF_QSPI_QIR & MCF_QSPI_QIR_SPIF))
        {
            Watchdog_Reset();
        }/*EndWhile*/
        /* Write a 1 to clear it */
        MCF_QSPI_QIR |= MCF_QSPI_QIR_SPIF;
        
        /* Point the Address Register to the Receive RAM */
        MCF_QSPI_QAR = MCF_QSPI_QAR_ADDR(MCF_QSPI_QAR_RECV);
        /* Read the byte in the transmit RAM at the first location */
        u8Data = (uint16)MCF_QSPI_QDR_DATA(MCF_QSPI_QDR); 
        
        return((uint_8)u8Data); 
    #endif

    #ifdef __MCF52277_H__
        uint16 u8Data;
        
        /* Check the status flag */
        if(MCF_DSPI_DSPI_SR & MCF_DSPI_DSPI_SR_EOQF)
        {
            /* Clear the EOQF by writting a 1 to it */
             MCF_DSPI_DSPI_SR |= MCF_DSPI_DSPI_SR_EOQF;
        }/*EndIf*/
        /* Write the DSPI_PUSHR register */
        MCF_DSPI_DSPI_PUSHR = ( MCF_DSPI_DSPI_PUSHR_CTAS(0)     | 
                                MCF_DSPI_DSPI_PUSHR_EOQ         |
                                MCF_DSPI_DSPI_PUSHR_CTCNT       |
                                MCF_DSPI_DSPI_PUSHR_PCS0        |
                                MCF_DSPI_DSPI_PUSHR_TXDATA(0xFF)); 

        /* Write the clock and transfer attributes: master clock and frame size (8 bits) */
        MCF_DSPI_DSPI_CTAR0 = ( MCF_DSPI_DSPI_CTAR_FMSZ(7)   | 
                                gSPI_BeforeTransfDelay        |
                                gSPI_AfterTransfDelay         |
                                gSPI_InterTransfDelay         |
                                gSPI_BaudRate);
        
        /* Start the transfer */
        MCF_DSPI_DSPI_MCR &= ~MCF_DSPI_DSPI_MCR_HALT;   
    
        /* Wait until the transfer has been finished */
        while(!(MCF_DSPI_DSPI_SR & MCF_DSPI_DSPI_SR_EOQF))
        {
            Watchdog_Reset();
        }/*EndWhile*/
        /* Clear the EOQF by writting a 1 to it */
        MCF_DSPI_DSPI_SR |= MCF_DSPI_DSPI_SR_EOQF;

        /* Read the byte form the DSPI_POPR register */ 
        u8Data = (uint_16)MCF_DSPI_DSPI_RXFR_RXDATA(MCF_DSPI_DSPI_POPR);

        return((uint_8)u8Data); 
    #endif  
}/*EndBody*/



/*********************************************************
* Name: SPI_High_rate
* Desc: Change SPI baud rate to high speed
* Parameter: None
* Return: None
**********************************************************/
void SPI_High_rate(void)
{
    /*Body*/
    #if(defined(__MCF52259_H__) || defined(__MCF52221_H__))
        /* QSPI Clock: 6MHz */
        /* Disable QSPI Clock */
        MCF_QSPI_QMR = 0;
        MCF_QSPI_QMR = MCF_QSPI_QMR_BAUD(0x06) | MCF_QSPI_QMR_MSTR | MCF_QSPI_QMR_BITS(8);
    #endif
    #ifdef __MCF52277_H__
        /* DSPI clock 6 MHz */
        /* The system clock fsys = 68 MHz */
        /* Value to be passed in SPI_Send_byte() function to DPI_CTAR0 register */
        gSPI_BaudRate = (MCF_DSPI_DSPI_CTAR_PBR_3CLK | MCF_DSPI_DSPI_CTAR_BR(0x01)); 
        
        /* Configure the rest of the delays */
        gSPI_BeforeTransfDelay = (MCF_DSPI_DSPI_CTAR_CSSCK(1)  | MCF_DSPI_DSPI_CTAR_CSSCK(0x02));
        gSPI_AfterTransfDelay  = (MCF_DSPI_DSPI_CTAR_PASC_3CLK | MCF_DSPI_DSPI_CTAR_ASC(0x02));
        gSPI_InterTransfDelay  = (MCF_DSPI_DSPI_CTAR_PDT_3CLK  | MCF_DSPI_DSPI_CTAR_DT(0x01));  
    #endif   
}/*EndBody*/




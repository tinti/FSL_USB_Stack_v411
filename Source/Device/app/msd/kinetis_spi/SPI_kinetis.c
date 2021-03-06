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
#include "SPI_kinetis.h"

extern void Watchdog_Reset(void);

/* Global variable defining the baud rate; this variable can be touched by
   SPI_Init() and SPI_HighRate() to change the baud rate from 375KHz (normal
   baud rate) to 6 MHz (high rate) */
static uint_32 gSPI_BaudRate;
static uint_32 gSPI_BeforeTransfDelay, gSPI_AfterTransfDelay, gSPI_InterTransfDelay;


/*********************************************************
 * Name: SPI_Init
 * Desc: Initialize SPI2 Module
 * Parameter: None
 * Return: None
 **********************************************************/
void SPI_Init(void)
{
	/*Body*/
#ifdef MCU_MK70F12
	/* set PORTE pin 1 to DSPI1.SOUT*/
	PORTE_PCR1 =  PORT_PCR_MUX(7);
	/* set PORTE pin 2 to DSPI1.SCK*/
	PORTE_PCR2 =  PORT_PCR_MUX(2);
	/* set PORTE pin 3 to DSPI1.SIN*/
	PORTE_PCR3 =  PORT_PCR_MUX(7);
	/* set PORTE pin 4 to DSPI1.CS0*/
	PORTE_PCR4 =  PORT_PCR_MUX(2);    

	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	/* Enable clock gate to DSPI1 module */
	SIM_SCGC6 |= SIM_SCGC6_DSPI1_MASK;
#elif defined MCU_MK20D5
	/* set PORTD pin 1 to SPI0.SOUT*/
	PORTD_PCR2 =  PORT_PCR_MUX(2);
	/* set PORTD pin 1 to SPI0.SCK*/
	PORTD_PCR1 =  PORT_PCR_MUX(2);
	/* set PORTD pin 3 to SPI0.SIN*/
	PORTD_PCR3 =  PORT_PCR_MUX(2);
	/* set PORTD pin 0 to SPI0.CS0*/
	PORTD_PCR0 =  PORT_PCR_MUX(2);    

	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	/* Enable clock gate to DSPI1 module */
	SIM_SCGC6 |= SIM_SCGC6_SPI0_MASK;
#elif defined MCU_MK20D7
	/* set PORTE pin 1 to DSPI1.SOUT*/
	PORTE_PCR1 =  PORT_PCR_MUX(2);
	/* set PORTE pin 2 to DSPI1.SCK*/
	PORTE_PCR2 =  PORT_PCR_MUX(2);
	/* set PORTE pin 3 to DSPI1.SIN*/
	PORTE_PCR3 =  PORT_PCR_MUX(2);
	/* set PORTE pin 4 to DSPI1.CS0*/
	PORTE_PCR4 =  PORT_PCR_MUX(2);

	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	/* Enable clock gate to DSPI1 module */
	SIM_SCGC6 |= SIM_SCGC6_SPI1_MASK;
#elif defined MCU_MK40D7
	/* set PORTE pin 1 to DSPI1.SOUT*/
	PORTE_PCR1 =  PORT_PCR_MUX(2);
	/* set PORTE pin 2 to DSPI1.SCK*/
	PORTE_PCR2 =  PORT_PCR_MUX(2);
	/* set PORTE pin 3 to DSPI1.SIN*/
	PORTE_PCR3 =  PORT_PCR_MUX(2);
	/* set PORTE pin 4 to DSPI1.CS0*/
	PORTE_PCR4 =  PORT_PCR_MUX(2);

	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	/* Enable clock gate to DSPI1 module */
	SIM_SCGC6 |= SIM_SCGC6_SPI1_MASK;
#elif defined MCU_MK40N512VMD100
	/* set PORTE pin 1 to DSPI1.SOUT*/
	PORTE_PCR1 =  PORT_PCR_MUX(2);
	/* set PORTE pin 2 to DSPI1.SCK*/
	PORTE_PCR2 =  PORT_PCR_MUX(2);
	/* set PORTE pin 3 to DSPI1.SIN*/
	PORTE_PCR3 =  PORT_PCR_MUX(2);
	/* set PORTE pin 4 to DSPI1.CS0*/
	PORTE_PCR4 =  PORT_PCR_MUX(2);

	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	/* Enable clock gate to DSPI1 module */
	SIM_SCGC6 |= SIM_SCGC6_SPI1_MASK;
#elif defined MCU_MK53N512CMD100
	/* set PORTE pin 1 to DSPI1.SOUT*/
	PORTE_PCR1 =  PORT_PCR_MUX(2);
	/* set PORTE pin 2 to DSPI1.SCK*/
	PORTE_PCR2 =  PORT_PCR_MUX(2);
	/* set PORTE pin 3 to DSPI1.SIN*/
	PORTE_PCR3 =  PORT_PCR_MUX(2);
	/* set PORTE pin 4 to DSPI1.CS0*/
	PORTE_PCR4 =  PORT_PCR_MUX(2);

	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	/* Enable clock gate to DSPI1 module */
	SIM_SCGC6 |= SIM_SCGC6_SPI1_MASK;
#elif defined MCU_MK60N512VMD100
	/* set PORTE pin 1 to DSPI1.SOUT*/
	PORTE_PCR1 =  PORT_PCR_MUX(2);
	/* set PORTE pin 2 to DSPI1.SCK*/
	PORTE_PCR2 =  PORT_PCR_MUX(2);
	/* set PORTE pin 3 to DSPI1.SIN*/
	PORTE_PCR3 =  PORT_PCR_MUX(2);
	/* set PORTE pin 4 to DSPI1.CS0*/
	PORTE_PCR4 =  PORT_PCR_MUX(2);

	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	/* Enable clock gate to DSPI1 module */
	SIM_SCGC6 |= SIM_SCGC6_SPI1_MASK;
#elif defined MCU_MK21D5
    SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	/* set PORTB pin 16 to DSPI1.SOUT*/
	PORTB_PCR16 =  PORT_PCR_MUX(2);
	/* set PORTB pin 11 to DSPI1.SCK*/
	PORTB_PCR11 =  PORT_PCR_MUX(2);
	/* set PORTB pin 17 to DSPI1.SIN*/
	PORTB_PCR17 =  PORT_PCR_MUX(2);
	/* set PORTB pin 10 to DSPI1.CS0*/
	PORTB_PCR10 =  PORT_PCR_MUX(2);
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	/* Enable clock gate to DSPI1 module */
	SIM_SCGC6 |= SIM_SCGC6_SPI1_MASK | SIM_SCGC6_DMAMUX_MASK;
#elif defined MCU_MKL25Z4
	/* set PORTE pin 1 to DSPI1.SOUT*/
	PORTE_PCR1 =  PORT_PCR_MUX(2);
	/* set PORTE pin 2 to DSPI1.SCK*/
	PORTE_PCR2 =  PORT_PCR_MUX(2);
	/* set PORTE pin 3 to DSPI1.SIN*/
	PORTE_PCR3 =  PORT_PCR_MUX(2);
	/* set PORTE pin 4 to DSPI1.CS0*/
	PORTE_PCR4 =  PORT_PCR_MUX(2);
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	/* Enable clock gate to DSPI1 module */
	SIM_SCGC4 |= SIM_SCGC4_SPI1_MASK;
	SIM_SCGC6 |= SIM_SCGC6_DMAMUX_MASK;
#else
	/* set PORTE pin 1 to DSPI1.SOUT*/
	PORTE_PCR1 =  PORT_PCR_MUX(2);
	/* set PORTE pin 2 to DSPI1.SCK*/
	PORTE_PCR2 =  PORT_PCR_MUX(2);
	/* set PORTE pin 3 to DSPI1.SIN*/
	PORTE_PCR3 =  PORT_PCR_MUX(2);
	/* set PORTE pin 4 to DSPI1.CS0*/
	PORTE_PCR4 =  PORT_PCR_MUX(2);

	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	/* Enable clock gate to DSPI1 module */
	SIM_SCGC6 |= SIM_SCGC6_DMAMUX_SPI1_MASK;
#endif

	/* Enable master mode, disable both transmit and receive FIFO buffers,
   set the inactive state for PCS2 high, enable the module (MDIS = 0),
   delay the sample point from the leading edge of the clock and halt
   any transfer
	 */
#ifdef MCU_MK20D5
	SPI0_MCR = (SPI_MCR_MSTR_MASK   |
			SPI_MCR_PCSIS(1)    |
			SPI_MCR_SMPL_PT(2)  |
			SPI_MCR_HALT_MASK);

	SPI0_MCR |= (SPI_MCR_CLR_RXF_MASK | SPI_MCR_CLR_TXF_MASK);
#elif defined MCU_MKL25Z4
  
	PORTE_PCR4 =  PORT_PCR_MUX(1);
	GPIOE_PDDR |= (1<<4);
	SPI_clr_SS();
	

	SPI1_C2 = SPI_C2_SPISWAI_MASK;     
	SPI1_C1 = SPI_C1_SPE_MASK | SPI_C1_MSTR_MASK | SPI_C1_SSOE_MASK;
	
#else
	SPI1_MCR = (SPI_MCR_MSTR_MASK   |
			SPI_MCR_PCSIS(1)    |
			SPI_MCR_SMPL_PT(2)  |
			SPI_MCR_HALT_MASK);

	SPI1_MCR |= (SPI_MCR_CLR_RXF_MASK | SPI_MCR_CLR_TXF_MASK);
#endif
	/* DSPI clock 375 KHz */
	/* The system clock fsys = 68 MHz */
	// K60: bus clock: 48MHz, DSPI clock: ~107 KHz
	// K70: bus clock: 60MHz, DSPI clock: ~156 KHz
	/* Value to be passed in SPI_Send_byte() function to DPI_CTAR0 register */
#ifdef MCU_MKL25Z4
	/* 375KHz SPI clock */
	SPI1_BR = SPI_BR_SPPR(7) | SPI_BR_SPR(5);             /*  SCK = 10us */
#else
#ifdef MCU_MK70F12
	gSPI_BaudRate = (SPI_CTAR_PBR(1) | SPI_CTAR_BR(0x07));
#else
	gSPI_BaudRate = (SPI_CTAR_PBR(3) | SPI_CTAR_BR(0x06));
#endif
	/* Configure the rest of the delays */
	gSPI_BeforeTransfDelay = (SPI_CTAR_CSSCK(1) | SPI_CTAR_CSSCK(0x04));
	gSPI_AfterTransfDelay  = (SPI_CTAR_PASC(3) | SPI_CTAR_ASC(0x04));
	gSPI_InterTransfDelay  = (SPI_CTAR_PDT(3)  | SPI_CTAR_DT(0x05));
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
#ifdef MCU_MK20D5
	/* Check the status flag */
	if(SPI0_SR & SPI_SR_EOQF_MASK)
	{
		/* Clear the EOQF by writting a 1 to it */
		SPI0_SR |= SPI_SR_EOQF_MASK;
	}/*Endif*/

	/* Write the DSPI_PUSHR register */
	SPI0_PUSHR = (  SPI_PUSHR_CTAS(0)        |
			SPI_PUSHR_EOQ_MASK       |
			SPI_PUSHR_CTCNT_MASK     |
			SPI_PUSHR_PCS(1)         |
			SPI_PUSHR_TXDATA(u8Data));
	/* Write the clock and transfer attributes: master clock and frame size (8 bits) */
	SPI0_CTAR0 = (  SPI_CTAR_SLAVE_FMSZ(7)      |
			gSPI_BeforeTransfDelay      |
			gSPI_AfterTransfDelay       |
			gSPI_InterTransfDelay       |
			gSPI_BaudRate);

	/* Start the transfer */
	SPI0_MCR &= ~SPI_MCR_HALT_MASK;

	/* Wait until the transfer has been finished */
	while(!(SPI0_SR & SPI_SR_EOQF_MASK))
	{
		Watchdog_Reset();
	}/*EndWhile*/
	/* Clear the EOQF by writting a 1 to it */
	SPI0_SR |= SPI_SR_EOQF_MASK;
#elif defined MCU_MKL25Z4

    while(!(SPI1_S & SPI_S_SPTEF_MASK))
    {
    	Watchdog_Reset();
    }
    (void)SPI1_S;
    SPI1_D=u8Data;
    while(!(SPI1_S & SPI_S_SPRF_MASK))
    {
    	Watchdog_Reset();
    }
    (void)SPI1_D;
    
#else
	/* Check the status flag */
	if(SPI1_SR & SPI_SR_EOQF_MASK)
	{
		/* Clear the EOQF by writting a 1 to it */
		SPI1_SR |= SPI_SR_EOQF_MASK;
	}/*Endif*/

	/* Write the DSPI_PUSHR register */
	SPI1_PUSHR = (  SPI_PUSHR_CTAS(0)        |
			SPI_PUSHR_EOQ_MASK       |
			SPI_PUSHR_CTCNT_MASK     |
			SPI_PUSHR_PCS(1)         |
			SPI_PUSHR_TXDATA(u8Data));
	/* Write the clock and transfer attributes: master clock and frame size (8 bits) */
	SPI1_CTAR0 = (  SPI_CTAR_SLAVE_FMSZ(7)      |
			gSPI_BeforeTransfDelay      |
			gSPI_AfterTransfDelay       |
			gSPI_InterTransfDelay       |
			gSPI_BaudRate);

	/* Start the transfer */
	SPI1_MCR &= ~SPI_MCR_HALT_MASK;

	/* Wait until the transfer has been finished */
	while(!(SPI1_SR & SPI_SR_EOQF_MASK))
	{
		Watchdog_Reset();
	}/*EndWhile*/
	/* Clear the EOQF by writting a 1 to it */
	SPI1_SR |= SPI_SR_EOQF_MASK;
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
	uint_16 u8Data;

	/*Body*/
#ifdef MCU_MK20D5
	/* Check the status flag */
	if(SPI0_SR & SPI_SR_EOQF_MASK)
	{
		/* Clear the EOQF by writting a 1 to it */
		SPI0_SR |= SPI_SR_EOQF_MASK;
	}/*EndIf*/

	/* Write the DSPI_PUSHR register */
	SPI0_PUSHR = (  SPI_PUSHR_CTAS(0)       |
			SPI_PUSHR_EOQ_MASK      |
			SPI_PUSHR_CTCNT_MASK    |
			SPI_PUSHR_PCS(1)        |
			SPI_PUSHR_TXDATA(0xFF));
	/* Write the clock and transfer attributes: master clock and frame size (8 bits) */
	SPI0_CTAR0 = (  SPI_CTAR_FMSZ(7)            |
			gSPI_BeforeTransfDelay      |
			gSPI_AfterTransfDelay       |
			gSPI_InterTransfDelay       |
			gSPI_BaudRate);

	/* Start the transfer */
	SPI0_MCR &= ~SPI_MCR_HALT_MASK;
	/* Wait until the transfer has been finished */
	while(!(SPI0_SR & SPI_SR_EOQF_MASK))
	{
		Watchdog_Reset();
	}/*EndWhile*/
	/* Clear the EOQF by writting a 1 to it */
	SPI0_SR |= SPI_SR_EOQF_MASK;
	/* Read the byte form the DSPI_POPR register */
	u8Data = (uint_16)SPI_RXFR0_RXDATA(SPI0_POPR);
#elif defined MCU_MKL25Z4
	
    SPI1_D=0xFF;
    while(!(SPI1_S & SPI_S_SPRF_MASK))
    {
    	Watchdog_Reset();
    }
	return((uint_8)SPI1_D);
		
#else
	/* Check the status flag */
	if(SPI1_SR & SPI_SR_EOQF_MASK)
	{
		/* Clear the EOQF by writting a 1 to it */
		SPI1_SR |= SPI_SR_EOQF_MASK;
	}/*EndIf*/

	/* Write the DSPI_PUSHR register */
	SPI1_PUSHR = (  SPI_PUSHR_CTAS(0)       |
			SPI_PUSHR_EOQ_MASK      |
			SPI_PUSHR_CTCNT_MASK    |
			SPI_PUSHR_PCS(1)        |
			SPI_PUSHR_TXDATA(0xFF));
	/* Write the clock and transfer attributes: master clock and frame size (8 bits) */
	SPI1_CTAR0 = (  SPI_CTAR_FMSZ(7)            |
			gSPI_BeforeTransfDelay      |
			gSPI_AfterTransfDelay       |
			gSPI_InterTransfDelay       |
			gSPI_BaudRate);

	/* Start the transfer */
	SPI1_MCR &= ~SPI_MCR_HALT_MASK;
	/* Wait until the transfer has been finished */
	while(!(SPI1_SR & SPI_SR_EOQF_MASK))
	{
		Watchdog_Reset();
	}/*EndWhile*/
	/* Clear the EOQF by writting a 1 to it */
	SPI1_SR |= SPI_SR_EOQF_MASK;
	/* Read the byte form the DSPI_POPR register */
	u8Data = (uint_16)SPI_RXFR0_RXDATA(SPI1_POPR);
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
	/* DSPI clock 6 MHz */
	/* The system clock fsys = 68 MHz */
	/* Value to be passed in SPI_Send_byte() function to DPI_CTAR0 register */
#ifdef MCU_MKL25Z4

	SPI1_BR = SPI_BR_SPPR(1) | SPI_BR_SPR(1);
#else
	gSPI_BaudRate = (SPI_CTAR_PBR(1) | SPI_CTAR_BR(0x01));

	/* Configure the rest of the delays */
	gSPI_BeforeTransfDelay = (SPI_CTAR_CSSCK(1)  | SPI_CTAR_CSSCK(0x02));
	gSPI_AfterTransfDelay  = (SPI_CTAR_PASC(1) | SPI_CTAR_ASC(0x02));
	gSPI_InterTransfDelay  = (SPI_CTAR_PDT(1)  | SPI_CTAR_DT(0x01));
#endif
}/*EndBody*/



/*********************************************************
 * Name: SPI_CS_assert
 * Desc: Change SPI_CS pin state
 * Parameter: expected state
 * Return: None
 **********************************************************/
void SPI_CS_assert(uint_8 state)
{
	/*Body*/
	GPIOE_PDDR |= GPIO_PDDR_PDD(4);
	GPIOE_PDOR |= GPIO_PDOR_PDO(state);
}/*EndBody*/
#if (defined MCU_MKL25Z4)
/*********************************************************
 * Name: SPI_set_SS
 * Desc: Change SPI_CS pin state to Low
 * Parameter: expected state
 * Return: None
 **********************************************************/
void SPI_set_SS()
{
	GPIOE_PDOR &= ~(1<<4);
}
/*********************************************************
 * Name: SPI_clr_SS
 * Desc: Change SPI_CS pin state to High
 * Parameter: expected state
 * Return: None
 **********************************************************/
void SPI_clr_SS()
{
	GPIOE_PDOR |= (1<<4);
}
#endif
/*EOF*/

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
#include "SPI.h"


/*********************************************************
* Name: SPI_Init
* Desc: Initialize SPI2 Module
* Parameter: None
* Return: None             
**********************************************************/
void SPI_Init(void)
{
  SPI_SS = 1;
  _SPI_SS= 1;
  
  SPI2BR = 0x14;              // 375KHz SPI clock    		
  SPI2C2 = 0x00;     
  SPI2C1 = SPI2C1_SPE_MASK | SPI2C1_MSTR_MASK;
}



/*********************************************************
* Name: SPI_Send_byte
* Desc: Send one byte 
* Parameter: The byte to be sent
* Return: None
**********************************************************/
void SPI_Send_byte(uint_8 u8Data)
{
	while(!SPI2S_SPTEF)
	{
	    __RESET_WATCHDOG();
	}
	(void)SPI2S;
	SPI2DL=u8Data;
	while(!SPI2S_SPRF)
	{
	    __RESET_WATCHDOG();
	}
	
	(void)SPI2DL;
}



/*********************************************************
* Name: SPI_Receive_byte
* Desc: The byte received by SPI  
* Parameter: None
* Return: Received byte
**********************************************************/
uint_8 SPI_Receive_byte(void)
{
	SPI2DL=0xFF;
	while(!SPI2S_SPRF)
	{
	    __RESET_WATCHDOG();
	}
		
	return(SPI2DL);
}



/*********************************************************
* Name: SPI_High_rate
* Desc: Change SPI baud rate to high speed
* Parameter: None
* Return: None
**********************************************************/
void SPI_High_rate(void)
{
  SPI2C1 = 0x00;
  SPI2BR = 0x00;          // 6MHz SPI clock    		
  SPI2C1 = SPI2C1_SPE_MASK | SPI2C1_MSTR_MASK;

}




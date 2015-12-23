/**HEADER********************************************************************
* 
* Copyright (c) 2010 Freescale Semiconductor;
* All Rights Reserved
*
*************************************************************************** 
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* $FileName: IIC_kinetis.c
* $Version : 
* $Date    : 
*
* Comments:  Code for initializing and using I2C
*
*
*END************************************************************************/

#include "derivative.h"     /* include peripheral declarations */
#include "IIC_kinetis.h"


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : IIC_ModuleInit
* Returned Value   : None
* Comments         : I2C Initialization. Set Baud Rate and turn on I2C.
*    
*
*END*----------------------------------------------------------------------*/

void IIC_ModuleInit(void)
{
	#if (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
		SIM_SCGC4 |= SIM_SCGC4_I2C0_MASK;
		
		/* Configure GPIO for I2C function (PTB2-I2C0_SCL, PTB3-I2C0_SDA) */
		PORTB_PCR2 = PORT_PCR_MUX(2);
		PORTB_PCR3 = PORT_PCR_MUX(2);
	
		I2C0_F  = 0x14;       				/* Set MULT and ICR I2C clock = 8M/80=100000 */

		I2C0_C1 = I2C_C1_IICEN_MASK;    	/* Enable IIC */		
	#elif (defined MCU_MK40N512VMD100) || (defined MCU_MK53N512CMD100)
		SIM_SCGC4 |= SIM_SCGC4_I2C1_MASK;
	
		/* Configure GPIO for I2C function (PTB2-I2C0_SCL, PTB3-I2C0_SDA) */
		PORTC_PCR11 = PORT_PCR_MUX(2);
		PORTC_PCR10 = PORT_PCR_MUX(2);
	
		I2C1_F  = 0x14;       /* Set MULT and ICR I2c clock = 8M/80=100000 */
	
		I2C1_C1 = 0x80;       /* Enable IIC */
	#elif defined MCU_MK60N512VMD100
		SIM_SCGC4 |= SIM_SCGC4_I2C0_MASK;
	
		/* configure GPIO for I2C function */
		PORTD_PCR8 = PORT_PCR_MUX(2);
		PORTD_PCR9 = PORT_PCR_MUX(2);
	
		I2C0_F  = 0x14;       /* Set MULT and ICR  */
	
		I2C0_C1 = 0x80;       /* Enable IIC */
	#elif defined MCU_MK21D5
		/* Configure Used I2C1*/
		SIM_SCGC4 |= SIM_SCGC4_I2C1_MASK;

		/* configure GPIO for I2C function */
		PORTC_PCR10 = PORT_PCR_MUX(2);
		PORTC_PCR11 = PORT_PCR_MUX(2);

		I2C1_F  = 0x14;       /* Set MULT and ICR  */

		I2C1_C1 = 0x80;       /* Enable IIC */
#elif defined MCU_MKL25Z4
	/* Configure Used I2C0*/
	SIM_SCGC4 |= SIM_SCGC4_I2C0_MASK;

	/* configure GPIO for I2C function */
	PORTE_PCR25 = PORT_PCR_MUX(5);
	PORTB_PCR0 = PORT_PCR_MUX(2);

	I2C0_F  = 0x14;      /* set MULT and ICR*/ 
	I2C0_C1 = 0x80;       /* Enable IIC */
	
	#elif defined MCU_MK70F12
		SIM_SCGC4 |= SIM_SCGC4_IIC2_MASK;
	
		/* configure GPIO for I2C function */
		PORTE_PCR18 = PORT_PCR_MUX(4);
		PORTE_PCR19 = PORT_PCR_MUX(4);
	
		I2C0_F  = 0x14;       /* Set MULT and ICR  */
	
		I2C0_C1 = 0x80;       /* Enable IIC */
	#endif
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : Pause
* Returned Value   : None
* Comments         : Pause Routine
*    
*
*END*----------------------------------------------------------------------*/
void Pause(void)
{
  int n;
  for(n=1; n<50; n++) 
  {
	#if (defined(__CWCC__) || defined(__IAR_SYSTEMS_ICC__) || defined(__GNUC__))
	  asm("nop");
	#elif defined (__CC_ARM)
	  __nop();
    #endif
  }
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : IIC_StartTransmission
* Returned Value   : None
* Comments         : Start I2C Transmission
*    
*
*END*----------------------------------------------------------------------*/
void IIC_StartTransmission (unsigned char SlaveID, unsigned char Mode)
{
  /* shift ID in right position */
  SlaveID = (unsigned char) SlaveID << 1;

  /* Set R/W bit at end of Slave Address */
  SlaveID |= (unsigned char)Mode;

  /* send start signal */
  i2c_Start();

  /* send ID with W/R bit */
  i2c_write_byte(SlaveID);
}
#if (defined MCU_MK40N512VMD100) || (defined MCU_MK53N512CMD100)||(defined MCU_MK21D5)   
/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : I2C_ReadRegister
* Returned Value   : None
* Comments         : Read a register 
*    
*
*END*----------------------------------------------------------------------*/
unsigned char I2C_ReadRegister(unsigned char u8I2CSlaveAddress,unsigned char u8RegisterAddress)
{
  unsigned char result;
  unsigned int j;
	
  /* Send Slave Address */
  IIC_StartTransmission(u8I2CSlaveAddress,MWSR);
  i2c_Wait();

  /* Write Register Address */
  I2C1_D = u8RegisterAddress;
  i2c_Wait();
	
  /* Do a repeated start */
  I2C1_C1 |= I2C_C1_RSTA_MASK;
	
  /* Send Slave Address */
  I2C1_D = (u8I2CSlaveAddress << 1) | 0x01; //read address
  i2c_Wait();
	
  /* Put in Rx Mode */
  I2C1_C1 &= (~I2C_C1_TX_MASK);
	
  /* Turn off ACK */
  I2C1_C1 |= I2C_C1_TXAK_MASK;
	
  /* Dummy read */
  result = I2C1_D;
  for (j=0; j<5000; j++){};
  i2c_Wait();
	  
  /* Send stop */
  i2c_Stop();
  result = I2C1_D;
  Pause();	
  
  return result;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : I2C_WriteRegister
* Returned Value   : None
* Comments         : Write a byte of Data
*    
*
*END*----------------------------------------------------------------------*/
void I2C_WriteRegister(unsigned char u8I2CSlaveAddress,unsigned char u8RegisterAddress, unsigned char u8Data)
{
  /* send data to slave */
  IIC_StartTransmission(u8I2CSlaveAddress,MWSR);
  i2c_Wait();
  
  /* Send I2C address */
  I2C1_D = u8RegisterAddress;
  i2c_Wait();

  /* Send data */
  I2C1_D = u8Data;
  i2c_Wait(); 

  i2c_Stop();
  Pause();
}
/* EOF */
#endif
#if (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7) || (defined MCU_MK60N512VMD100) || (defined MCU_MK70F12)|| (defined MCU_MKL25Z4)
/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : I2C_ReadRegister
* Returned Value   : None
* Comments         : Read a register 
*    
*
*END*----------------------------------------------------------------------*/
unsigned char I2C_ReadRegister(unsigned char u8I2CSlaveAddress,unsigned char u8RegisterAddress)
{
  unsigned char result;
  unsigned int j;

  /* Send Slave Address */
  IIC_StartTransmission(u8I2CSlaveAddress,MWSR);
  i2c_Wait();

  /* Write Register Address */
  I2C0_D = u8RegisterAddress;
  i2c_Wait();

  /* Do a repeated start */
  I2C0_C1 |= I2C_C1_RSTA_MASK;

  /* Send Slave Address */
  I2C0_D = (u8I2CSlaveAddress << 1) | 0x01; //read address
  i2c_Wait();

  /* Put in Rx Mode */
  I2C0_C1 &= (~I2C_C1_TX_MASK);

  /* Turn off ACK */
  I2C0_C1 |= I2C_C1_TXAK_MASK;

  /* Dummy read */
  result = I2C0_D;
  for (j=0; j<5000; j++){};
  i2c_Wait();

  /* Send stop */
  i2c_Stop();
  result = I2C0_D ;
  Pause();
  return result;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : I2C_WriteRegister
* Returned Value   : None
* Comments         : Write a byte of Data
*    
*
*END*----------------------------------------------------------------------*/
void I2C_WriteRegister(unsigned char u8I2CSlaveAddress,unsigned char u8RegisterAddress, unsigned char u8Data)
{
  /* send data to slave */
  IIC_StartTransmission(u8I2CSlaveAddress,MWSR);
  i2c_Wait();

  /* Send I2C address */
  I2C0_D = u8RegisterAddress;
  i2c_Wait();

  /* Send data */
  I2C0_D = u8Data;
  i2c_Wait();

  i2c_Stop();

  Pause();
}
/* EOF */
#endif

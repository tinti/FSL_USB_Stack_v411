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
* $FileName: IIC_cfv1_plus.c
* $Version : 
* $Date    : 
*
* Comments:  Code for initializing and using I2C
*
*
*END************************************************************************/

#include "derivative.h"     /* include peripheral declarations */
#include "IIC_cfv1_plus.h"
#include "types.h"

void Pause(void);

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
	/*Enable source clock for I2C0*/
	SIM_SCGC1 |= SIM_SCGC1_I2C0_MASK;

    /* configure PINs for I2C function */
	MXC_PTCPF1 = 0x33;

    I2C0_F  = 0x14;       /* set MULT and ICR  */

    I2C0_C1 = 0x80;       /* enable IIC */

}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : Pause
* Returned Value   : None
* Comments         : Pause Routine
*    
*
*END*----------------------------------------------------------------------*/
void Pause(void){
    int n;
    for(n=1;n<50;n++) {
      asm("nop");
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
  result = I2C0_D ;
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


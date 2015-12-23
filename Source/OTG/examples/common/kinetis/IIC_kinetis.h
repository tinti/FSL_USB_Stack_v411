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
* $FileName: IIC_kinetis.h
* $Version : 
* $Date    : 
*
* Comments:  Code for initializing and using I2C
*
*
*END************************************************************************/

/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/
#include "derivative.h"
#ifndef IIC_KINETIS_H
#define IIC_KINETIS_H

#define MMA7660_I2C_ADDRESS                         0x4C
#if (defined MCU_MK40N512VMD100)||(defined MCU_MK53N512CMD100)||(defined MCU_MK21D5)
#define i2c_DisableAck()       I2C1_C1 |= I2C_C1_TXAK_MASK

#define i2c_RepeatedStart()    I2C1_C1     |= 0x04;

#define i2c_Start()            I2C1_C1     |= 0x10;\
                               I2C1_C1     |= I2C_C1_MST_MASK

#define i2c_Stop()             I2C1_C1  &= ~I2C_C1_MST_MASK;\
                               I2C1_C1  &= ~I2C_C1_TX_MASK

#define i2c_EnterRxMode()      I2C1_C1   &= ~I2C_C1_TX_MASK;\
                               I2C1_C1   &= ~I2C_C1_TXAK_MASK

#define i2c_Wait()               while((I2C1_S & I2C_S_IICIF_MASK)==0) {} \
                                  I2C1_S |= I2C_S_IICIF_MASK;

#define i2c_write_byte(data)   I2C1_D = data
#endif
#if (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7) || (defined MCU_MK60N512VMD100) || (defined MCU_MK70F12)|| (defined MCU_MKL25Z4)
#define i2c_DisableAck()       I2C0_C1 |= I2C_C1_TXAK_MASK

#define i2c_RepeatedStart()    I2C0_C1     |= 0x04;

#define i2c_Start()            I2C0_C1     |= 0x10;\
                               I2C0_C1     |= I2C_C1_MST_MASK

#define i2c_Stop()             I2C0_C1  &= ~I2C_C1_MST_MASK;\
                               I2C0_C1  &= ~I2C_C1_TX_MASK

#define i2c_EnterRxMode()      I2C0_C1   &= ~I2C_C1_TX_MASK;\
                               I2C0_C1   &= ~I2C_C1_TXAK_MASK

#define i2c_Wait()               while((I2C0_S & I2C_S_IICIF_MASK)==0) {} \
                                  I2C0_S |= I2C_S_IICIF_MASK;

#define i2c_write_byte(data)   I2C0_D = data
#endif

#define MWSR                   0x00  /* Master write  */
#define MRSW                   0x01  /* Master read */

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/
void IIC_ModuleInit(void);
void IIC_StartTransmission (unsigned char SlaveID, unsigned char Mode);
void I2C_WriteRegister(unsigned char, unsigned char u8RegisterAddress, unsigned char u8Data);
unsigned char I2C_ReadRegister(unsigned char, unsigned char u8RegisterAddress);

#endif

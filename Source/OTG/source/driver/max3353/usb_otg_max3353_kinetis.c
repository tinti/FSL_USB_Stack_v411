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
* Comments:
*
*   This file contains the implementation of the OTG functions using the MAX3353 circuit
*
*END************************************************************************/
#include "types.h"
#include "usb_otg_max3353_kinetis.h"
#include "IIC_kinetis.h"
#include "usb_otg_main.h"
#include "user_config.h"
/* Private functions prototypes *********************************************/

#if !HIGH_SPEED_DEVICE
static boolean max3353_WriteReg(uint_8 regAdd , uint_8 regValue);
static boolean max3353_ReadReg(uint_8 regAdd , uint_8* p_regValue);
static uint_8 max3353_Init(void);

/* Private functions definitions *********************************************/

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : max3353_CheckifPresent
* Returned Value   :
* Comments         : Check if MAX3353 is present
*    
*
*END*----------------------------------------------------------------------*/
uint_32 u32ID=0;
static uint_8 max3353_CheckifPresent(void)
{

	uint_8  u8Counter;

	for(u8Counter=0;u8Counter<4;u8Counter++)
	{
		u32ID=u32ID<<8;
		u32ID|=I2C_ReadRegister(MAX3353_I2C_ADDRESS,u8Counter);
	}
	if(u32ID!=MAX3353_MID)
		return(MAX3353_NOT_PRESENT);

	u32ID=0;
	for(u8Counter=4;u8Counter<8;u8Counter++)
	{
		u32ID=u32ID<<8;
		u32ID|=I2C_ReadRegister(MAX3353_I2C_ADDRESS,u8Counter);
	}
	if(u32ID!=MAX3353_PID)
		return(MAX3353_NOT_PRESENT);

	return(MAX3353_OK);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : max3353_Init
* Returned Value   :
* Comments         : Initialize max3353
*    
*
*END*----------------------------------------------------------------------*/
uint_8 max3353_Init(void)
{
	if(max3353_CheckifPresent())
		return(MAX3353_NOT_PRESENT);

	/* Enable Charge pump for VBUs detection */ 
	max3353_WriteReg(MAX3353_REG_CTRL_2,0x80);

	/* Set Rising edge for VBUS detection */
	max3353_WriteReg(MAX3353_REG_INT_EDGE,VBUS_VALID_ED_MASK);

	/* Activate ID (GND and float) & SESSION Interrupts */
	max3353_WriteReg(MAX3353_REG_INT_MASK,ID_FLOAT_EN_MASK|ID_GND_EN_MASK\
			|VBUS_VALID_EN_MASK|SESSION_SESSEND_EN_MASK);

	return(MAX3353_OK);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : max3353_WriteReg
* Returned Value   :
* Comments         : Write data to max3353 register
*    
*
*END*----------------------------------------------------------------------*/
static boolean max3353_WriteReg(uint_8 regAdd , uint_8 regValue)
{
	I2C_WriteRegister(MAX3353_I2C_ADDRESS,regAdd,regValue);
	return TRUE;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : max3353_ReadReg
* Returned Value   :
* Comments         : Read data from max3353 register
*    
*
*END*----------------------------------------------------------------------*/
static boolean max3353_ReadReg(uint_8 regAdd , uint_8* p_regValue)
{
	*p_regValue = I2C_ReadRegister(MAX3353_I2C_ADDRESS,regAdd);
	return TRUE;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _otg_max3353_enable_disable
* Returned Value   :
* Comments         : Enable/disable MAX3353
*    
*
*END*----------------------------------------------------------------------*/
void _otg_max3353_enable_disable(boolean enable)
{
	volatile uint_8 max3353_data;
	max3353_Init();
	if(enable)
	{
		max3353_data = 0x02;
		while(max3353_WriteReg(MAX3353_REG_CTRL_1, max3353_data) == FALSE){}   
		max3353_data = 0x1F; /* Enable interrupts */
		while(max3353_WriteReg(MAX3353_REG_INT_MASK, max3353_data) == FALSE){}
		max3353_data = 0x00;  /* Enable module. */
	}
	else
	{
		max3353_data = 1;  /* Activate shutdown */
	}


	/* Enable/Disable module */
	while(max3353_WriteReg(MAX3353_REG_CTRL_2, max3353_data) == FALSE){}

	/* Read the latch to clear any pending interrupts */
	(void)_otg_max3353_get_interrupts();  
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _otg_max3353_get_status
* Returned Value   : unsigned char, meaning the status of MAX3353
* Comments         : Get MAX3353 status
*    
*
*END*----------------------------------------------------------------------*/
uint_8 _otg_max3353_get_status(void)
{
	uint_8 status;
	uint_8 edge, new_edge;

	while(max3353_ReadReg(MAX3353_REG_STATUS, &status) == FALSE){}

	/* Handle here the edge detection in SessionValid and VBus valid */

	/* Read the current edge */
	while(max3353_ReadReg(MAX3353_REG_INT_EDGE, &edge) == FALSE){}

	new_edge = (uint_8)((~(status)) & 0x03);

	if(new_edge != edge)
	{	  
		/* Write the new edges */
		while(max3353_WriteReg(MAX3353_REG_INT_EDGE, new_edge) == FALSE){}
	}

	return status;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _otg_max3353_get_interrupts
* Returned Value   : unsigned char meaning interrupts
* Comments         : Read interrupts from MAX3353
*    
*
*END*----------------------------------------------------------------------*/
uint_8 _otg_max3353_get_interrupts(void)
{
	uint_8 data;
	
	while(max3353_ReadReg(MAX3353_REG_INT_LATCH, &data) == FALSE){}	
	
	return data;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _otg_max3353_set_VBUS
* Returned Value   :
* Comments         : Set VBUS for MAX3353
*    
*
*END*----------------------------------------------------------------------*/
void _otg_max3353_set_VBUS(boolean enable)
{
  volatile uint_8 max3353_data;
	
	if(enable)
	{
 	 max3353_data = OTG_CTRL_2_VBUS_DRV;/* connect VBUS to the charge pump */
    }
	else
	{
	 max3353_data = OTG_CTRL_2_VBUS_DISCHG;/* disconnect the charge pump and  activate the 5k pull down resistor */
	}
	
	/* Enable/Disable module */
	while(max3353_WriteReg(MAX3353_REG_CTRL_2, max3353_data) == FALSE){}

}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _otg_max3353_set_pdowns
* Returned Value   :
* Comments         : Set pull-downs for MAX3353
*    
*
*END*----------------------------------------------------------------------*/
void _otg_max3353_set_pdowns(uint_8 bitfield)
{
   volatile uint_8 max3353_data;
       
 	 max3353_data =  OTG_CTRL_1_IRQ_PUSH_PULL;
 	 
 	 if(bitfield & OTG_CTRL_PDOWN_DP)
 	  {
 	   max3353_data |= OTG_CTRL_1_PDOWN_DP;
 	  }
 	  
 	 if(bitfield & OTG_CTRL_PDOWN_DM)
 	  {
 	   max3353_data |= OTG_CTRL_1_PDOWN_DM;
 	  }
 	  
   while(max3353_WriteReg(MAX3353_REG_CTRL_1, max3353_data) == FALSE){}
}
#endif

/* EOF */

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
#include "usb_otg_max3353.h"
#include "IIC_cfv2.h"
#include "usb_otg_main.h"
/* Private functions prototypes *********************************************/

static boolean max3353_WriteReg(uint_8 regAdd , uint_8 regValue);
static boolean max3353_ReadReg(uint_8 regAdd , uint_8* p_regValue);

/* Private functions definitions *********************************************/

/*****************************************************************************
*   max3353_WriteReg 
*
*   Initializes the I2C module 
******************************************************************************/
static boolean max3353_WriteReg(uint_8 regAdd , uint_8 regValue)
{
 uint_8 tx_buff[2];
 tx_buff[0] = regAdd;
 tx_buff[1] = regValue;
 
 while(IIC_Bus_Busy()){}
  
 
 return IIC_Transmit_Master(tx_buff, 2, MAX3353_SLAVE_ADDR);
}


/*****************************************************************************
*   max3353_ReadReg 
*
*   Resets the I2C module.
******************************************************************************/
static boolean max3353_ReadReg(uint_8 regAdd , uint_8* p_regValue)
{
 uint_8 tx_buff;
 tx_buff = regAdd;
 
 while(IIC_Bus_Busy()){}
 
 return IIC_Transmit_RS_Receive_Master(&tx_buff , 1 , p_regValue , 1 , MAX3353_SLAVE_ADDR);
}

/* Public functions *********************************************************/
void _otg_max3353_enable_disable(boolean enable)
{
  volatile uint_8 max3353_data;
	
	if(enable)
	{
 	 max3353_data = 0x02;
   while(max3353_WriteReg(MAX3353_REG_CTRL_1, max3353_data) == FALSE){}
	 //max3353_data = 0x03; /* Edge detection - Rising */
   //while(max3353_WriteReg(MAX3353_REG_INT_EDGE, max3353_data) == FALSE){}
   
   max3353_data = 0x1F; /* Enable interrupts */
   while(max3353_WriteReg(MAX3353_REG_INT_MASK, max3353_data) == FALSE){}
   //max3353_data = 0x10;  /* Enable module. Activate the discharge rezistor on  VBUS */
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

uint_8 _otg_max3353_get_interrupts(void)
{
	uint_8 data;
	
	while(max3353_ReadReg(MAX3353_REG_INT_LATCH, &data) == FALSE){}	
	
	return data;
}
void _otg_max3353_set_VBUS(boolean enable)
{
  volatile uint_8 max3353_data;
	
	if(enable)
	{
 	 max3353_data = OTG_CTRL_2_VBUS_DRV;// connect VBUS to the charge pump
  }
	else
	{
	 max3353_data = OTG_CTRL_2_VBUS_DISCHG;// disconnect the charge pump and  activate the 5k pull down resistor 
	}
	
	/* Enable/Disable module */
	while(max3353_WriteReg(MAX3353_REG_CTRL_2, max3353_data) == FALSE){}

}


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

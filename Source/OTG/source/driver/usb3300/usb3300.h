/**HEADER********************************************************************
* 
* Copyright (c) 2012 Freescale Semiconductor;
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
*   This is the header file for the SMSC USB3300 access functions
*
*END************************************************************************/

#ifndef _USB3300_H_
#define _USB3300_H_

#include "psptypes.h"
#include "user_config.h"

#if HIGH_SPEED_DEVICE

// USB3300 registers
#define USB3300_VENDOR_ID_LOW					0x00
#define USB3300_VENDOR_ID_HIGH					0x01
#define USB3300_PRODUCT_ID_LOW					0x02
#define USB3300_PRODUCT_ID_HIGH					0x03
#define USB3300_FUNCTION_CONTROL				0x04
#define USB3300_INTERFACE_CONTROL				0x07
#define USB3300_OTG_CONTROL						0x0A
#define USB3300_OTG_INTERRUPT_ENABLE_RISING		0x0D
#define USB3300_OTG_INTERRUPT_ENABLE_FALLING	0x10
#define USB3300_INTERRUPT_STATUS				0x13
#define USB3300_INTERRUPT_LATCH					0x14
#define USB3300_DEBUG							0x15
#define USB3300_SCRATCH							0x16

// USB3300 register access offsets
#define USB3300_READ							0x00
#define USB3300_WRITE							0x00
#define USB3300_SET								0x01
#define USB3300_CLEAR							0x02

// USB3300 register masks
#define USB3300_OTG_CONTROL_CHARGE_VBUS_MASK	0x10
#define USB3300_OTG_CONTROL_DISCHARGE_VBUS_MASK	0x08
#define USB3300_OTG_CONTROL_PDOWN_DP_MASK		0x02
#define USB3300_OTG_CONTROL_PDOWN_DM_MASK		0x04
#define USB3300_OTG_CONTROL_DRV_VBUS_MASK		0x20
#define USB3300_OTG_CONTROL_DRV_VBUS_EXT_MASK	0x40

#define OTG_STAT_VBUS_VALID						0x02
#define OTG_STAT_SESS_VALID						0x04
#define OTG_STAT_SESS_END						0x08
#define OTG_STAT_ID_GND							0x10

extern void USB3300_Init();
extern uint_8 USB3300_GetStatus();
extern uint_8 USB3300_GetInterrupts();
extern void USB3300_SetVBUS(boolean enable);
extern void USB3300_SetPdowns(uint_8 bitfield);

#endif // HIGH_SPEED_DEVICE

#endif // _USB3300_H_
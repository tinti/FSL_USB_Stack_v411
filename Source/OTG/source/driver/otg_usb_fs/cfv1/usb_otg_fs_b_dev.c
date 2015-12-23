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
*   This file contains the implementation of the OTG driver functions for the A device 
*
*END************************************************************************/

#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usb.h"
#include "usbevent.h"
#include "usb_otg_main.h"
#include "host_dev_list.h"
#include "usb_otg_private.h"
#include "derivative.h"

/* Constant Definitions*********************************************************/


/* Private functions prototypes *********************************************/

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_init_se0_detect
* Returned Value   :
* Comments         : This function is called to initialize the SE0 detection
*    
*
*END*----------------------------------------------------------------------*/
void _usb_otg_init_se0_detect(_usb_otg_handle otg_handle)
{
 USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;
 
 /* Clear the Line state change interrupt */
 USB_OTG_REG(otg_device_number, OTG_INT_STAT) |= OTG_INT_STAT_LINE_STATE_CHG_MASK; 
 
 /* Remove the D+ pullup */
 USB_OTG_REG(otg_device_number, OTG_CTRL) &= ~OTG_CTRL_DP_HIGH_MASK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_init_se0_detect
* Returned Value   :
* Comments         : This function enables / disables the pull-up on DP line
*    
*
*END*----------------------------------------------------------------------*/
void _usb_otg_dp_pullup_enable(_usb_otg_handle otg_handle, boolean enable)
{
  USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;
  
  if(enable)
  {
    /* Enable the D+ pullup */
    USB_OTG_REG(otg_device_number, OTG_CTRL) |= OTG_CTRL_DP_HIGH_MASK;
  }
  else
  {
    /* Remove the D+ pullup */
    USB_OTG_REG(otg_device_number, OTG_CTRL) &= ~OTG_CTRL_DP_HIGH_MASK;
  }
}
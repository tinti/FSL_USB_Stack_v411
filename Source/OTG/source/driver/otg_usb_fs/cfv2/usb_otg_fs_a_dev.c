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
#include "usb_otg_max3353.h"

/* Constant Definitions*********************************************************/


/* Public functions prototypes *********************************************/

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_init_a_device
* Returned Value   :
* Comments         : This function is called after an ID change transition to the A-idle state
*    
*
*END*----------------------------------------------------------------------*/
void _usb_otg_init_a_device(_usb_otg_handle otg_handle)
{
 USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;
 
 /* Enable the A device pull-down */
 USB_OTG_REG( usb_otg_struct_ptr->deviceNumber , MCF_USB_OTG_OTG_CTRL) |= (MCF_USB_OTG_OTG_CTRL_DM_LOW | MCF_USB_OTG_OTG_CTRL_DP_LOW);
 if(usb_otg_struct_ptr->init_struct->ext_set_pdowns != (otg_ext_set_pdowns)NULL)
  {
   usb_otg_struct_ptr->init_struct->ext_set_pdowns( OTG_CTRL_PDOWN_DP | OTG_CTRL_PDOWN_DM);
  }
 
}

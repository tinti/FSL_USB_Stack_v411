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
 *   This file contains the implementation of the OTG state machine
 *
 *END************************************************************************/

#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usb.h"
#include "usbevent.h"
#include "usb_otg_main.h"
#include "usb_otg_sm.h"
#include "host_dev_list.h"
#include "usb_otg_private.h"
#include "derivative.h"

/* Constant Definitions*********************************************************/


/* Type Definitions*********************************************************/


/* Private memory definitions ***********************************************/

/* Private functions prototypes *********************************************/

/* Public functions *********************************************************/

/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _usb_otg_sm
 * Returned Value   :
 * Comments         : Handles the changes in OTG status
 *    
 *
 *END*----------------------------------------------------------------------*/
void _usb_otg_sm(_usb_otg_handle otg_handle)
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;

	/* Check the device number */
	if(otg_handle != NULL)
	{
		USB_OTG_STATUS *otg_status =  &usb_otg_struct_ptr->otg_status;

		if(usb_otg_struct_ptr->deviceState == USB_OTG_DEVSTATE_A)
		{
			_usb_otg_sm_a(otg_handle);
		} 
		else
		{
			if(usb_otg_struct_ptr->deviceState == USB_OTG_DEVSTATE_B)
			{

				_usb_otg_sm_b(otg_handle);
			}
			else
			{
				/* State is undefined, this point is reached after initialization */
				/* ID change events are handled here */	 
				if(otg_status->id)
				{
					_usb_otg_id_chg_b(otg_handle);
				}
				else
				{
					_usb_otg_id_chg_a(otg_handle);
				}
			}
		}      
	} 
}


/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _usb_otg_id_chg_a
 * Returned Value   :
 * Comments         : Handles the changes in OTG status for the A-device
 *    
 *
 *END*----------------------------------------------------------------------*/
void _usb_otg_id_chg_a(_usb_otg_handle otg_handle)
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;
	USB_OTG_STATUS *otg_status =  &usb_otg_struct_ptr->otg_status;
	/* Initialize the state machine */
	_usb_otg_init_a_device(otg_handle);
	usb_otg_struct_ptr->deviceState = USB_OTG_DEVSTATE_A;
	usb_otg_struct_ptr->subState = USB_OTG_SM_A_IDLE;
	otg_status->a_bus_req = TRUE;
	otg_status->a_srp_det = FALSE;
	otg_status->a_srp_det_state = srp_not_started;
	otg_status->b_timeout_en = FALSE;
	otg_status->b_timeout = 0;
	/* Signal the event to the application */
	_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_IDLE);
}


/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _usb_otg_id_chg_b
 * Returned Value   :
 * Comments         : Handles the changes in OTG status for the B-device
 *    
 *
 *END*----------------------------------------------------------------------*/
void _usb_otg_id_chg_b(_usb_otg_handle otg_handle)
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;
	/* Initialize the state machine */
	usb_otg_struct_ptr->deviceState = USB_OTG_DEVSTATE_B;
	usb_otg_struct_ptr->subState = USB_OTG_SM_B_IDLE_SESS_DETECT;	

	/* Call the driver function to perform the OTG controller specific initializations */
	//_usb_otg_init_b_device(otg_handle);

	/* Signal the event to the application */
	_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_B_IDLE);
}

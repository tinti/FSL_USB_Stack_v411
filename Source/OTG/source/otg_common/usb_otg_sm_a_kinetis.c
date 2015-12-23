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
#include "usb_devapi.h"
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
static void _usb_otg_a_change_state(_usb_otg_handle otg_handle , uint_8 new_state  );

/* Public functions *********************************************************/

/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _usb_otg_sm_a
 * Returned Value   :
 * Comments         : This function handles the substates of the B-state machine
 *    
 *
 *END*----------------------------------------------------------------------*/
void _usb_otg_sm_a(_usb_otg_handle otg_handle)
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;
	USB_OTG_STATUS *otg_status =  &usb_otg_struct_ptr->otg_status;

 	switch(usb_otg_struct_ptr->subState)
	{
	case USB_OTG_SM_A_IDLE:

		if((!otg_status->a_srp_det) &&  otg_status->sess_end)
		{
			// srp detection
			if(otg_status->a_srp_det_state == srp_not_started)
			{
				if(otg_status->live_se0 && otg_status->line_stable)
				{
					// se0 detected
					otg_status->a_srp_det_state = srp_se0;
				}
			}
			else if(otg_status->a_srp_det_state == srp_se0)
			{
				if(otg_status->line_stable)
				{
					if(otg_status->live_jstate)
					{
						//j state detected
						otg_status->a_srp_det_state = srp_dp_puls;
						otg_status->host_req_poll_timer = 0;
					}
					else if(!otg_status->live_se0)
					{
						otg_status->a_srp_det_state = srp_not_started;
					}
				}
			}
			else//srp_dp_puls state
			{
				if(otg_status->line_stable)
				{
					if(otg_status->live_se0)
					{
						otg_status->a_srp_det_state = srp_not_started;
						otg_status->a_srp_pulse_duration =  otg_status->host_req_poll_timer;
						if((TB_DATA_PLS_MIN <= otg_status->a_srp_pulse_duration ) && (otg_status->a_srp_pulse_duration <= TB_DATA_PLS_MAX ))
						{
							// valid srp puls detected
							otg_status->a_srp_det = TRUE;
						}
					}
					else if(!otg_status->live_jstate)
					{
						otg_status->a_srp_det_state = srp_not_started;

					}
				}
			} 
		}
		else//(otg_status->sess_end)
		{
			otg_status->a_srp_det_state = srp_not_started;
		}
		if(otg_status->id )
		{
			_usb_otg_id_chg_b(otg_handle);
			_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_ID_TRUE);
			break;
		}
		if(otg_status->a_bus_drop)
		{
			break;
		}
		if(otg_status->a_bus_req || usb_otg_struct_ptr->powerUp || otg_status->a_srp_det)  
		{
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_VRISE );
			usb_otg_struct_ptr->powerUp = FALSE;

		}
		break;
	case USB_OTG_SM_A_WAIT_VRISE:
		if(otg_status->id || otg_status->a_bus_drop)
		{
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_VFALL );
			break;
		}
		if(otg_status->vbus_valid)
		{
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_BCON );
		}
		else if(otg_status->b_timeout_en == FALSE)
		{
			// TA_VBUS_RISE time expires
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_VFALL );
			_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_WAIT_VRISE_TMOUT);

		}

		break;
	case USB_OTG_SM_A_WAIT_BCON:
		if(otg_status->id || otg_status->a_bus_drop)
		{
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_VFALL );
			break;
		}
		if(!otg_status->vbus_valid)
		{
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_VBUS_ERR);
			_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_VBUS_ERR);
			break;
		}
		if(otg_status->live_jstate && (otg_status->ms_since_line_changed >= otg_status->b_conn_dbnc_time))
		{
			// b device has connected
			otg_status->b_conn = TRUE;
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_HOST );
			_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_HOST);
		}
		else if(otg_status->b_timeout_en == FALSE)
		{
			// TA_WAIT_BCON timeout expires
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_VFALL );
			_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_WAIT_BCON_TMOUT);
		}
		else
		{
			// if the short debouce window expires , long debounce time must be used
			if(otg_status->host_req_poll_timer >= TA_BCON_SDB_WIN)
			{
				otg_status->b_conn_dbnc_time = TA_BCON_LDB;
			}

		}
		break;
	case USB_OTG_SM_A_VBUS_ERR:
		if(otg_status->id || otg_status->a_bus_drop || otg_status->a_clr_err)
		{
			otg_status->a_clr_err = 0;
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_VFALL );
		}
		break;
	case USB_OTG_SM_A_WAIT_VFALL:
		if(otg_status->b_timeout_en == FALSE)
		{
			//TA_VBUS_FALL timeout expires
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_IDLE );
		}
		break;
	case USB_OTG_SM_A_HOST:
		if(!otg_status->vbus_valid)
		{
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_VBUS_ERR);
			_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_VBUS_ERR);
			break;
		}
		if(otg_status->id || otg_status->a_bus_drop)
		{
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_VFALL );
			if(otg_status->id)
			{
				_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_ID_TRUE);
			}

			break;
		}
		if(!otg_status->b_conn)    
		{
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_BCON );
			break;
		}
		if(otg_status->hnp_req)
		{
			//B device has requested HNP
			// when app wants to start HNP it sets a_bus_req to FALSE                 
			if( !otg_status->a_bus_req )    
			{
				_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_SUSPEND );
				break;
			}
			_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_B_HNP_REQ);
		}
		if( usb_otg_struct_ptr->dev_inst_ptr != NULL )
		{
			// A device polls B device for HNP request
			if(otg_status->a_set_b_hnp_en && (otg_status->host_req_poll_timer >= T_HOST_REQ_POLL))
			{
				otg_status->host_req_poll_timer = 0;
				_usb_otg_hnp_poll_req (usb_otg_struct_ptr);  
			}
		}
		break;
	case USB_OTG_SM_A_SUSPEND:
		if(!otg_status->vbus_valid)
		{
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_VBUS_ERR);
			_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_VBUS_ERR);
			break;
		}
		if(otg_status->id || otg_status->a_bus_drop)
		{
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_VFALL );
			break;
		}
		if(otg_status->a_set_b_hnp_en)
		{
			if(otg_status->live_se0 && otg_status->line_stable && (!otg_status->a_bus_req))   
			{
				// B device  detects bus idle and disconnects as part of the HNP .
				// A device must change its state to peripheral.
				// DP pullup must be enabled , DP pulldown disabled and DM pulldown mantained .
				otg_status->b_conn = FALSE;
				_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_PERIPHERAL );

				USB_OTG_REG(usb_otg_struct_ptr->deviceNumber, USB0_OTGCTL) = USB_OTGCTL_DPHIGH_MASK | USB_OTGCTL_DMLOW_MASK | USB_OTGCTL_OTGEN_MASK ;
				if(usb_otg_struct_ptr->init_struct->ext_set_pdowns != (otg_ext_set_pdowns)NULL)
				{
					usb_otg_struct_ptr->init_struct->ext_set_pdowns(OTG_CTRL_PDOWN_DM);
				}
				_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_PERIPHERAL);
				break;
			}
			if(otg_status->b_timeout_en == FALSE)
			{ 
				// TA_AIDL_BDIS timeout expires
				if(otg_status->a_bus_req)
				{
					USB_OTG_REG( usb_otg_struct_ptr->deviceNumber , USB0_CTL) |= USB_CTL_RESUME_MASK;
					_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_SUSPEND_RESUME );
				}
				else
				{
					_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_VFALL );
					_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_AIDL_BDIS_TMOUT);
				}
				break; 
			}
		}
		else//(otg_status->a_set_b_hnp_en)
		{
			if(otg_status->live_se0 && otg_status->line_stable)   
			{
				otg_status->b_conn = FALSE;
				_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_BCON );
				break;
			}
			if(otg_status->a_bus_req)
			{
				USB_OTG_REG( usb_otg_struct_ptr->deviceNumber , USB0_CTL) |= USB_CTL_RESUME_MASK;
				_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_SUSPEND_RESUME );
			}
		}
		break;
	case USB_OTG_SM_A_SUSPEND_RESUME:
		if(!otg_status->vbus_valid)
		{
			USB_OTG_REG( usb_otg_struct_ptr->deviceNumber , USB0_CTL) &= ~USB_CTL_RESUME_MASK;
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_VBUS_ERR);
			_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_VBUS_ERR);
			break;
		}
		if(otg_status->id || otg_status->a_bus_drop)
		{
			USB_OTG_REG( usb_otg_struct_ptr->deviceNumber , USB0_CTL) &= ~USB_CTL_RESUME_MASK;
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_VFALL );
			break;
		}
		if(otg_status->b_timeout_en == FALSE)
		{
			USB_OTG_REG( usb_otg_struct_ptr->deviceNumber , USB0_CTL) &= ~USB_CTL_RESUME_MASK;
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_HOST );
			_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_HOST);
		}
		break;
	case USB_OTG_SM_A_PERIPHERAL:
		if(!otg_status->vbus_valid)
		{
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_VBUS_ERR);
			_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_VBUS_ERR);
			break;
		}
		if(otg_status->id || otg_status->a_bus_drop)
		{
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_VFALL );
			if(otg_status->id)
			{
				_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_ID_TRUE);
			}
			break;
		}
		if(otg_status->live_jstate && (otg_status->ms_since_line_changed >= TA_BIDL_ADIS))
		{
			// the bus is idle for TA_BIDL_ADIS. This is the last stage of HNP
			// A device leaves peripheral state 
			_usb_otg_a_change_state( otg_handle , USB_OTG_SM_A_WAIT_BCON );
			_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_BIDL_ADIS_TMOUT);
		}
		break;
	}
}

/* Private functions *********************************************************/

/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _usb_otg_a_change_state
 * Returned Value   :
 * Comments         : This function handles the actions performed at A substate change
 *    
 *
 *END*----------------------------------------------------------------------*/
static void _usb_otg_a_change_state(_usb_otg_handle otg_handle , uint_8 new_state  )
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;
	USB_OTG_STATUS *otg_status =  &usb_otg_struct_ptr->otg_status;

	if(usb_otg_struct_ptr->subState == USB_OTG_SM_A_HOST)
	{
		usb_otg_struct_ptr->init_struct->unload_usb_host(); 
	}
	if(usb_otg_struct_ptr->subState == USB_OTG_SM_A_PERIPHERAL)
	{

		usb_otg_struct_ptr->init_struct->unload_usb_device(); 
		USB_OTG_REG(usb_otg_struct_ptr->deviceNumber, USB0_OTGCTL) = USB_OTGCTL_DPLOW_MASK | USB_OTGCTL_DMLOW_MASK | USB_OTGCTL_OTGEN_MASK ;  

		if(usb_otg_struct_ptr->init_struct->ext_set_pdowns != (otg_ext_set_pdowns)NULL)
		{
			usb_otg_struct_ptr->init_struct->ext_set_pdowns( OTG_CTRL_PDOWN_DP | OTG_CTRL_PDOWN_DM);
		}
	}

	switch(new_state)
	{
	case USB_OTG_SM_A_IDLE:
		usb_otg_struct_ptr->subState = USB_OTG_SM_A_IDLE;
		otg_status->a_srp_det_state = srp_not_started;
		otg_status->b_timeout_en = FALSE;
		otg_status->b_timeout = 0;
		_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_IDLE);
		break;
	case USB_OTG_SM_A_WAIT_VRISE:

		usb_otg_struct_ptr->subState = USB_OTG_SM_A_WAIT_VRISE;	
		usb_otg_struct_ptr->init_struct->ext_set_VBUS(TRUE);
#if HIGH_SPEED_DEVICE
		// allow the state machine to reinitialize its variables
		_usb_otg_ext_isr(0);
#endif
		otg_status->b_timeout_en = FALSE;
		otg_status->b_timeout = TA_VBUS_RISE;
		otg_status->b_timeout_en = TRUE;  	  
		_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_WAIT_VRISE);
		break;
	case USB_OTG_SM_A_WAIT_BCON:
		if(usb_otg_struct_ptr->subState == USB_OTG_SM_A_WAIT_VRISE)
		{
			otg_status->b_conn_dbnc_time = TA_BCON_LDB; 
		}
		else
		{
			otg_status->b_conn_dbnc_time = TA_BCON_SDB; 
		}
		usb_otg_struct_ptr->subState = USB_OTG_SM_A_WAIT_BCON;
		usb_otg_struct_ptr->dev_inst_ptr = NULL;
		otg_status->a_set_b_hnp_en = 0;
		otg_status->srp_support = 0;
		otg_status->hnp_support = 0;
		otg_status->host_req_poll_timer = 0;
		otg_status->b_timeout_en = FALSE;
		otg_status->b_timeout = TA_WAIT_BCON;
		otg_status->b_timeout_en = TRUE;
		_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_WAIT_BCON);
		break;
	case USB_OTG_SM_A_VBUS_ERR:
		usb_otg_struct_ptr->subState = USB_OTG_SM_A_VBUS_ERR;  
		usb_otg_struct_ptr->init_struct->ext_set_VBUS(FALSE);
		otg_status->b_conn = FALSE;
		otg_status->b_timeout_en = FALSE;
		otg_status->b_timeout = 0;
		_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_VBUS_ERR);
		break;
	case USB_OTG_SM_A_WAIT_VFALL:
		usb_otg_struct_ptr->subState = USB_OTG_SM_A_WAIT_VFALL;  
		usb_otg_struct_ptr->init_struct->ext_set_VBUS(FALSE);
		otg_status->b_conn = FALSE;
		otg_status->a_srp_det = FALSE;
		otg_status->a_srp_det_state = srp_not_started;
		otg_status->b_timeout_en = FALSE;
		otg_status->b_timeout = TA_VBUS_FALL;
		otg_status->b_timeout_en = TRUE;
		_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_WAIT_VFALL);
		break;
	case USB_OTG_SM_A_HOST:
		usb_otg_struct_ptr->subState = USB_OTG_SM_A_HOST; 
		usb_otg_struct_ptr->dev_inst_ptr = NULL;
		otg_status->hnp_req = 0;
		otg_status->srp_support = 0;
		otg_status->hnp_support = 0;
		otg_status->a_set_b_hnp_en = 0;
		// otg_status->a_bus_req = TRUE;
		otg_status->b_timeout_en = FALSE;
		otg_status->b_timeout = 0;
		{
			uint_32 error;
			error = usb_otg_struct_ptr->init_struct->load_usb_host();
			if(error == USB_OK)
			{
				_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_HOST);
			}
			else
			{
				_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_HOST_LOAD_ERROR);
			}
		}
		break;
	case USB_OTG_SM_A_SUSPEND:
		usb_otg_struct_ptr->subState = USB_OTG_SM_A_SUSPEND;
		otg_status->b_timeout_en = FALSE;
		otg_status->b_timeout = 0;
		if(otg_status->a_set_b_hnp_en)
		{
			otg_status->b_timeout = TA_AIDL_BDIS;
			otg_status->b_timeout_en = TRUE;
		}
		_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_SUSPEND);
		break;
	case USB_OTG_SM_A_SUSPEND_RESUME:
		usb_otg_struct_ptr->subState = USB_OTG_SM_A_SUSPEND_RESUME;
		otg_status->b_timeout_en = FALSE;
		otg_status->b_timeout = TA_SUSPEND_RESUME;
		otg_status->b_timeout_en = TRUE;
		break;
	case USB_OTG_SM_A_PERIPHERAL:
		usb_otg_struct_ptr->subState = USB_OTG_SM_A_PERIPHERAL;  
		otg_status->b_timeout_en = FALSE;
		otg_status->b_timeout = 0;
		{
			uint_32 error;
			error = usb_otg_struct_ptr->init_struct->load_usb_device();
			if(error == USB_OK)
			{
				_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_PERIPHERAL);
				_usb_device_set_status(&(usb_otg_struct_ptr->deviceNumber), USB_STATUS_OTG, USB_STATUS_IDLE);
			}
			else
			{
				_usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_A_PERIPHERAL_LOAD_ERROR);
			}
		}
		break;
	}
}






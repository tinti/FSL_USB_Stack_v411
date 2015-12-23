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
*   This file contains the implementation of the OTG driver for USB FS
*
*END************************************************************************/

#include "user_config.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "derivative.h"
#include "usb.h"
#include "usb_devapi.h"
#include "host_dev_list.h"
#include "host_common.h"
#if !HIGH_SPEED_DEVICE
	#include "usbprv_host.h"
#else
	#include "ehci_usbprv_host.h"
#endif
#include "usbevent.h"
#include "usb_otg_main.h"
#if !HIGH_SPEED_DEVICE
#include "usb_otg_max3353.h"
#else
#include "usb3300.h"
#endif
#include "usb_otg_sm.h"
#include "host_dev_list.h"
#include "usb_otg_private.h"


/* Constant Definitions*********************************************************/


/* Type Definitions*********************************************************/


/* Private functions prototypes ***********************************************/

/* Private memory definitions ***********************************************/
static USB_OTG_STRUCT* usb_otg_struct_ptr_array[USB_OTG_DEV_CNT];


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_init
* Returned Value   : initialization message
* Comments         : Initializes OTG stack and OTG hardware
*    
*
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_init(uint_8 controller_ID, OTG_INIT_STRUCT *init_struct, _usb_otg_handle *otg_handle)
{
	USB_OTG_STRUCT *usb_otg_struct_ptr;
	USB_OTG_STATUS *otg_status ;

	/* Check the device number and get_pin function */
	if((controller_ID >= USB_OTG_DEV_CNT) ||
			(init_struct == NULL)
	)
	{
		return USB_INVALID_PARAMETER;	 
	} 

#if !HIGH_SPEED_DEVICE
	/* Check the members of the initialization structure */
	if((init_struct->ext_circuit_use == FALSE) ||   /* only the external circuit is currently supported */
			(init_struct->ext_enable_disable_func == NULL) ||
			(init_struct->ext_get_status_func == NULL)     ||
			(init_struct->ext_set_VBUS == NULL)            ||
			(init_struct->load_usb_host == NULL)           ||
			(init_struct->load_usb_device == NULL)         ||
			(init_struct->unload_usb_host == NULL)         ||
			(init_struct->unload_usb_device == NULL)       
	)
	{
		return USB_INVALID_PARAMETER;	 
	}
#endif
 
	/* Check if this device controller was already initialized */
	if(usb_otg_struct_ptr_array[controller_ID] != NULL)
	{
		return USBERR_INIT_FAILED; 	 
	}

	/* Initialize the USB OTG interface. */   
	usb_otg_struct_ptr = (USB_OTG_STRUCT*)malloc(sizeof(USB_OTG_STRUCT));

	if(usb_otg_struct_ptr == NULL)
	{
		return USB_OUT_OF_MEMORY;
	}
	memset(usb_otg_struct_ptr, 0, sizeof(USB_OTG_STRUCT));
	
	otg_status =  &usb_otg_struct_ptr->otg_status;
    UNUSED(otg_status);
	usb_otg_struct_ptr->deviceNumber = controller_ID;
	/* Save the initialization structure pointer for further usage */
	usb_otg_struct_ptr->init_struct = init_struct;

	/* Write the handle for the application */
	*otg_handle = (void*)usb_otg_struct_ptr;
	/* Save the handler in the global structure pointer array */
	usb_otg_struct_ptr_array[controller_ID] = usb_otg_struct_ptr;

	/* initialize the event used for application signaling */
	_usb_event_init(&(usb_otg_struct_ptr->otg_app_event));

	/* Initialize the OTG controller */
#if !HIGH_SPEED_DEVICE
	USB_OTG_REG(otg_device_number, USB0_ISTAT) = 0xFF;
	USB_OTG_REG(otg_device_number, USB0_OTGISTAT) = 0xFF;
#else

	// reset module(USBHS_USBMODE can be written just once after module reset)
	USBHS_USBCMD |= USBHS_USBCMD_RST_MASK;
	
	while(USBHS_USBCMD & USBHS_USBCMD_RST_MASK);
		
	USBHS_USBMODE = USBHS_USBMODE_CM(3);
	
	// Set HS module to run mode
	USBHS_USBCMD |= USBHS_USBCMD_RS_MASK;

	// only full speed mode
	USBHS_PORTSC1 |= USBHS_PORTSC1_PFSC_MASK;
	
	// Apply port power
	//USBHS_PORTSC1 |= USBHS_PORTSC1_PP_MASK;

	// clear interrupts
	USBHS_USBSTS = USBHS_USBSTS;
	USBHS_OTGSC |= USBHS_OTGSC_W1C;		// w1c bits
#endif

#if !HIGH_SPEED_DEVICE
	/* Enable the OTG mode */
	USB_OTG_REG(otg_device_number, USB0_OTGCTL) = USB_OTGCTL_OTGEN_MASK;
	USB_OTG_REG(otg_device_number, USB0_USBTRC0) = 0;
	
	/* Enable the external OTG circuit */
	usb_otg_struct_ptr->init_struct->ext_enable_disable_func(TRUE);
#endif

	/* Initialize the State machine */
	usb_otg_struct_ptr->deviceState = USB_OTG_DEVSTATE_UNDEFINED;
	usb_otg_struct_ptr->subState = USB_OTG_SM_UNDEFINED;
	usb_otg_struct_ptr->pending_ext_isr = TRUE;
	usb_otg_struct_ptr->powerUp = TRUE;
	//otg_status->line_change = TRUE;

	/* Enable the OTG Interrupts */
#if !HIGH_SPEED_DEVICE
	USB_OTG_REG(otg_device_number, USB0_OTGICR) = 
			USB_OTGICR_ONEMSECEN_MASK | USB_OTGICR_LINESTATEEN_MASK;
#else
	USBHS_OTGSC =
			USBHS_OTGSC_MSE_MASK |		// 1 Milli-Second timer interrupt Enable
			USBHS_OTGSC_BSEIE_MASK | 	// B Session End Interrupt Enable
			USBHS_OTGSC_BSVIE_MASK |	// B Session Valid Interrupt Enable
			USBHS_OTGSC_ASVIE_MASK |	// A Session Valid Interrupt Enable
			USBHS_OTGSC_AVVIE_MASK |	// A VBUS Valid Interrupt Enable
			USBHS_OTGSC_IDIE_MASK; 		// USB ID Interrupt Enable
			
			
#endif
	return USB_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_register_callback
* Returned Value   : register callback message
* Comments         : Registers OTG callback
*    
*
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_register_callback(_usb_otg_handle handle, otg_event_callback callback)
{
	USB_OTG_STRUCT *usb_otg_struct_ptr;

	if(handle == NULL)
	{
		return USB_INVALID_PARAMETER;	 
	} 

	usb_otg_struct_ptr = (USB_OTG_STRUCT *)handle;

	/* Save the provided callback */
	usb_otg_struct_ptr->callback = callback;

	return USB_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_session_request
* Returned Value   : session request message
* Comments         : B-device requests a new session to be started by the A device
*    
*
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_session_request(_usb_otg_handle handle)
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)handle;

	if(handle == NULL)
	{
		return USB_INVALID_PARAMETER;	 
	}

	if(usb_otg_struct_ptr->deviceState != USB_OTG_DEVSTATE_B)
	{
		return USBOTGERR_INVALID_REQUEST;
	}

	usb_otg_struct_ptr->srpRequest = TRUE;

	return USB_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_bus_request
* Returned Value   : bus request message
* Comments         : B-device requests to become Host
*    
*
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_bus_request(_usb_otg_handle handle)
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)handle;
	uint_8 dev_otg_status;

	if(handle == NULL)
	{
		return USB_INVALID_PARAMETER;	 
	}

	if(usb_otg_struct_ptr->deviceState != USB_OTG_DEVSTATE_B)
	{
		return USBOTGERR_INVALID_REQUEST;
	}

	usb_otg_struct_ptr->busRequest = TRUE;	

	_usb_device_get_status(&(usb_otg_struct_ptr->deviceNumber), USB_STATUS_OTG, &dev_otg_status);

	dev_otg_status |= USB_OTG_HOST_REQUEST_FLAG;

	_usb_device_set_status(&(usb_otg_struct_ptr->deviceNumber), USB_STATUS_OTG, dev_otg_status);


	return USB_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_bus_release
* Returned Value   : bus release message
* Comments         : B-device hands over the bus back to the A device
*    
*
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_bus_release(_usb_otg_handle handle)
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)handle;

	if(handle == NULL)
	{
		return USB_INVALID_PARAMETER;	 
	}

	if( (usb_otg_struct_ptr->deviceState != USB_OTG_DEVSTATE_B)  ||
			(usb_otg_struct_ptr->subState != USB_OTG_SM_B_HOST)
	)	    
	{
		return USBOTGERR_INVALID_REQUEST;
	}

	usb_otg_struct_ptr->busRelease = TRUE;

	return USB_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_hnp_enable
* Returned Value   : HNP enable status
* Comments         : This function is intended to be called from the Peripheral USB stack 
*                  : in response to SET/CLEAR Feature requests from the Host for HNP_Enable 
*    
*
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_hnp_enable(uint_8 controller_ID, uint_8 enable)
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = usb_otg_struct_ptr_array[controller_ID];

	if(usb_otg_struct_ptr == NULL)
	{
		return USB_INVALID_PARAMETER;	 
	}

	usb_otg_struct_ptr->hnpEnabled = enable;

	return USB_OK;
}


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_otg_task
*  Returned Value :
*  Comments       : OTG task
*        
*END*------------------------------------------------------------------*/
// todo: check
#if HIGH_SPEED_DEVICE
	boolean state_A = FALSE;
#endif	
void _usb_otg_task(void)
{
	uint_32 i=0;
	
#if USB_OTG_DEV_CNT>1
	/* Cycle through all the USB OTG controllers and handle the events */
	for(i = 0; i < USB_OTG_DEV_CNT; i++)
#endif
	{
		USB_OTG_STRUCT *usb_otg_struct_ptr = usb_otg_struct_ptr_array[i];

		if(usb_otg_struct_ptr != NULL)
		{
			USB_OTG_STATUS *otg_status =  &usb_otg_struct_ptr->otg_status;
			uint_8 otg_device_number = usb_otg_struct_ptr->deviceNumber;
			
            UNUSED(otg_device_number);
	#if HIGH_SPEED_DEVICE
			/* 
			 * check ID pin state
			 * ID = 0, this is an A device
			 * ID = 1, this is a B device
			 */
			if(!(USB3300_Read(USB3300_INTERRUPT_STATUS) & OTG_STAT_ID_GND) && !state_A)
			{
				_usb_otg_ext_isr(0);
				state_A = TRUE;
			}
	#endif

			/* *** Handle the events *** */	
			if(usb_otg_struct_ptr->pending_ext_isr)
			{
				uint_8 int_latch, status;

				/* read the active interrupts from the controller and the current status */
				int_latch = usb_otg_struct_ptr->init_struct->ext_get_interrupts_func();
                UNUSED(int_latch);
				status = usb_otg_struct_ptr->init_struct->ext_get_status_func();         

				/* check the status indications */         
				/* ID status update */
#if !HIGH_SPEED_DEVICE
				otg_status->id = (uint_8)((status & OTG_STAT_ID_FLOAT) ? TRUE : FALSE);
#else
				otg_status->id = //(USBHS_OTGSC & USBHS_OTGSC_ID_MASK) ? TRUE : FALSE;
						USB3300_Read(USB3300_INTERRUPT_STATUS) & OTG_STAT_ID_GND ? TRUE : FALSE;
#endif

				/* V_BUS_VALID status update */
				otg_status->vbus_valid = 
#if !HIGH_SPEED_DEVICE
						(uint_8)((status & OTG_STAT_VBUS_VALID) ? TRUE : FALSE);
#else
						(USBHS_PORTSC1 & USBHS_PORTSC1_PP_MASK) ? TRUE : FALSE;
#endif

				/* SESS_VALID status update */
				otg_status->sess_valid = (uint_8)((status & OTG_STAT_SESS_VALID) ? TRUE : FALSE);

				/* SESS_END status update */
				otg_status->sess_end = (uint_8)((status & OTG_STAT_SESS_END) ? TRUE : FALSE);

				if(usb_otg_struct_ptr->init_struct->ext_get_interrupts_func() == 0)
				{
					usb_otg_struct_ptr->pending_ext_isr = FALSE; 
				}
			}

			/* Update status */    	      	      	      
#if !HIGH_SPEED_DEVICE
			otg_status->line_stable = (uint_8)((USB_OTG_REG(otg_device_number, USB0_OTGSTAT) & USB_OTGSTAT_LINESTATESTABLE_MASK) ? TRUE : FALSE);
#else
			otg_status->line_stable = (USBHS_OTGSC | USBHS_OTGSC_DPS_MASK) ? TRUE : FALSE;
#endif
			if(!otg_status->line_stable)  
			{
				otg_status->ms_since_line_changed = 0;
			}

			if(otg_status->line_stable)
			{
#if !HIGH_SPEED_DEVICE
				otg_status->live_se0 =  (uint_8)((USB_OTG_REG(otg_device_number, USB0_CTL) & USB_CTL_SE0_MASK) ? TRUE : FALSE);				
#else
				otg_status->live_se0 = (USBHS_PORTSC1 & USBHS_PORTSC1_LS_MASK) ? FALSE : TRUE;
#endif

				if(otg_status->live_se0)
				{
					otg_status->live_jstate = 0;
				}
				else
				{
#if !HIGH_SPEED_DEVICE
					otg_status->live_jstate = ((USB_OTG_REG(otg_device_number, USB0_CTL) & USB_CTL_JSTATE_MASK) ? TRUE : FALSE);
#else
					otg_status->live_jstate = (USBHS_PORTSC1 & USBHS_PORTSC1_LS(0x2)) ? TRUE : FALSE;
#endif
				}
				
			}
			
			/* Call the state machine to handle the changes in OTG status */
			_usb_otg_sm(usb_otg_struct_ptr);

			/* Application events */
			if(usb_otg_struct_ptr->otg_app_event.VALUE)		
			{
				/* App indications */
				if(usb_otg_struct_ptr->callback)
				{
					usb_otg_struct_ptr->callback((_usb_otg_handle)usb_otg_struct_ptr, (OTG_EVENT)usb_otg_struct_ptr->otg_app_event.VALUE);				
				}
				_usb_event_clear(&(usb_otg_struct_ptr->otg_app_event), (OTG_EVENT)usb_otg_struct_ptr->otg_app_event.VALUE);
			}
		}
	}
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_otg_ext_isr
*  Returned Value :  
*  Comments       : Service interrupt for the external OTG circuit. 
*                   Needs to be called from the corresponding interrupt handler (KBI, IRQ, etc)
*        
*END*------------------------------------------------------------------*/
void _usb_otg_ext_isr(uint_8 controller_ID)
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = usb_otg_struct_ptr_array[controller_ID];
	
	if(usb_otg_struct_ptr != NULL)
	{  
		usb_otg_struct_ptr->pending_ext_isr = TRUE;
	}
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_otg_ext_isr
*  Returned Value :  
*  Comments       : Service interrupt for the external OTG circuit. 
*                   Needs to be called from the corresponding interrupt handler (KBI, IRQ, etc)
*        
*END*------------------------------------------------------------------*/
void _usb_otg_isr(uint_8 controller_ID)
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = usb_otg_struct_ptr_array[controller_ID];
	if(usb_otg_struct_ptr != NULL)
	{
#if !HIGH_SPEED_DEVICE
		uint_8 otg_int_stat =  (uint_8)(USB_OTG_REG(otg_device_number, USB0_OTGISTAT) & (USB_OTGISTAT_LINE_STATE_CHG_MASK | USB_OTGISTAT_ONEMSEC_MASK));
		uint_8 otg_stat =    (uint_8)(USB_OTG_REG(otg_device_number, USB0_OTGSTAT));
		USB_OTG_STATUS *otg_status =  &usb_otg_struct_ptr->otg_status;
		uint_8 otg_device_number = usb_otg_struct_ptr->deviceNumber;

        UNUSED(otg_device_number);
		/* Clear the OTG relevant interrupts */
		USB_OTG_REG(otg_device_number, USB0_OTGISTAT) = otg_int_stat;	 

		if(otg_int_stat & USB_OTGISTAT_ONEMSEC_MASK)
		{          
			otg_status->tmr_1ms  = TRUE; 
			if( otg_status->ms_since_line_changed < 0xffff )
			{
				otg_status->ms_since_line_changed++;
			}
			if( otg_status->host_req_poll_timer < 0xffff )
			{
				otg_status->host_req_poll_timer++;
			} 

			/* Decrement timeouts */
			if(otg_status->b_timeout_en && otg_status->b_timeout)
			{
				otg_status->b_timeout--;
				if(!otg_status->b_timeout)
				{
					otg_status->b_timeout_en = FALSE;
				}
			}
		}

		if((otg_int_stat & USB_OTGISTAT_LINE_STATE_CHG_MASK) || (!(otg_stat & USB_OTGSTAT_LINESTATESTABLE_MASK)))
		{          
			otg_status->ms_since_line_changed = 0;
		}
#else
		//uint_8 otg_int_stat =  (uint_8)(USB_OTG_REG(otg_device_number, USB0_OTGISTAT) & (USB_OTGISTAT_LINE_STATE_CHG_MASK | USB_OTGISTAT_ONEMSEC_MASK));
		uint_32 otg_int_stat = USBHS_OTGSC & USBHS_OTGSC_MSS_MASK;
		//uint_8 otg_stat =    (uint_8)(USB_OTG_REG(otg_device_number, USB0_OTGSTAT));
		USB_OTG_STATUS *otg_status = &usb_otg_struct_ptr->otg_status;
		uint_8 otg_device_number = usb_otg_struct_ptr->deviceNumber;

		/* Clear the OTG relevant interrupts */
		USBHS_OTGSC |= otg_int_stat;	 

		if(otg_int_stat & USBHS_OTGSC_MSS_MASK)
		{          
			otg_status->tmr_1ms  = TRUE;
			if( otg_status->ms_since_line_changed < 0xffff )
			{
				otg_status->ms_since_line_changed++;
			}
			if( otg_status->host_req_poll_timer < 0xffff )
			{
				otg_status->host_req_poll_timer++;
			} 

			/* Decrement timeouts */
			if(otg_status->b_timeout_en && otg_status->b_timeout)
			{
				otg_status->b_timeout--;
				if(!otg_status->b_timeout)
				{
					otg_status->b_timeout_en = FALSE;
				}
			}
		}
		// todo: check this
/*
		if((otg_int_stat & USB_OTGISTAT_LINE_STATE_CHG_MASK) || (!(otg_stat & USB_OTGSTAT_LINESTATESTABLE_MASK)))
		{
			otg_status->ms_since_line_changed = 0;
		}
		*/
#endif // HIGH_SPEED_DEVICE
	}
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_set_a_bus_req
* Returned Value   : set A bus request status
* Comments         : This function is called from the application to set/clear the 
*                    a_bus_req parameter. This is one of the parameters that determines 
*                    A state machine behavior.If the A device is in peripheral state 
*                    the otg status changes to USB_OTG_HOST_REQUEST_FLAG.  
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_set_a_bus_req(_usb_otg_handle otg_handle , boolean a_bus_req )
{
	uint_8 dev_otg_status;
	USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;
	USB_OTG_STATUS *otg_status =  &usb_otg_struct_ptr->otg_status;
	if(otg_handle == NULL)
	{
		return USB_INVALID_PARAMETER;	 
	}
	if(usb_otg_struct_ptr->deviceState != USB_OTG_DEVSTATE_A)
	{
		return USBOTGERR_INVALID_REQUEST;
	}	
	otg_status->a_bus_req  = a_bus_req; 
	if(usb_otg_struct_ptr->subState == USB_OTG_SM_A_PERIPHERAL)
	{
		_usb_device_get_status(&(usb_otg_struct_ptr->deviceNumber), USB_STATUS_OTG, &dev_otg_status);

		dev_otg_status |= USB_OTG_HOST_REQUEST_FLAG;

		_usb_device_set_status(&(usb_otg_struct_ptr->deviceNumber), USB_STATUS_OTG, dev_otg_status);
	}
	return USB_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_set_a_bus_drop
* Returned Value   : set A bus drop status
* Comments         : This function is called from the application to set/clear the 
*                    a_bus_drop parameter. This is one of the parameters that determines 
*                    A state machine behavior.
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_set_a_bus_drop(_usb_otg_handle otg_handle , boolean a_bus_drop )
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;
	USB_OTG_STATUS *otg_status =  &usb_otg_struct_ptr->otg_status;
	
	if(otg_handle == NULL)
	{
		return USB_INVALID_PARAMETER;	 
	}
	if(usb_otg_struct_ptr->deviceState != USB_OTG_DEVSTATE_A)
	{
		return USBOTGERR_INVALID_REQUEST;
	}

	if(a_bus_drop)
	{
		otg_status->a_bus_drop = TRUE; 
	}
	else
	{
		otg_status->a_bus_drop = FALSE;
	}

	return USB_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_set_a_clear_err
* Returned Value   : set A clear error status
* Comments         : This function is called from the application to set the a_clr_err
*                    parameter which is one way to escape from the a_vbus_err state.
*                    The other two are id = FALSE and a_bus_drop = TRUE.
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_set_a_clear_err( _usb_otg_handle otg_handle )
{
	USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;
	USB_OTG_STATUS *otg_status =  &usb_otg_struct_ptr->otg_status;
	
	if(otg_handle == NULL)
	{
		return USB_INVALID_PARAMETER;	 
	}
	if(usb_otg_struct_ptr->deviceState != USB_OTG_DEVSTATE_A)
	{
		return USBOTGERR_INVALID_REQUEST;
	}

	otg_status->a_clr_err = TRUE; 	


	return USB_OK;
}



/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_a_set_b_hnp_en
* Returned Value   : set B HNP status
* Comments         : This function is called from usb host stack at device enumeration.
*                    More specific it is called from usb_host_cntrl_transaction_done function
*                    (in host_ch9.c) when b_hnp_enable feature was succesfully set in the B OTG device.
*                    The function simply set the a_set_b_hnp_en status parameter. 
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_a_set_b_hnp_en(void*  dev_handle , boolean b_hnp_en)  
{
	DEV_INSTANCE_PTR  dev_inst_ptr; 
	USB_HOST_STATE_STRUCT_PTR host_struct_ptr;
	USB_OTG_STRUCT*   usb_otg_struct_ptr;
	uint_8            deviceNumber;
	USB_OTG_STATUS *  otg_status;

	if(dev_handle == NULL)
	{
		return USB_INVALID_PARAMETER;	 
	}
	dev_inst_ptr = (DEV_INSTANCE_PTR)dev_handle;
	host_struct_ptr = (USB_HOST_STATE_STRUCT_PTR)dev_inst_ptr->host;
	if((void*)host_struct_ptr == NULL)
	{
		return USB_INVALID_PARAMETER;
	}

	deviceNumber = host_struct_ptr->DEV_NUM;
	if( deviceNumber >= USB_OTG_DEV_CNT )
	{
		return USB_INVALID_PARAMETER;  
	}

	usb_otg_struct_ptr = usb_otg_struct_ptr_array[deviceNumber];
	if((void*)usb_otg_struct_ptr == NULL)
	{
		return USB_INVALID_PARAMETER;
	}
	otg_status = &usb_otg_struct_ptr->otg_status;
	otg_status->a_set_b_hnp_en  = b_hnp_en; 
	return USB_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_get_otg_attribute
* Returned Value   : get OTG attribute status
* Comments         : This function is called from usb host stack at device enumeration.
*                    More specific it is called from usb_host_cntrl_transaction_done function
*                    (in host_ch9.c) when configuration descriptor was read and an OTG descriptor     .
*                    was identified in it.
*                    The function simply set srp_support hnp_support status parameters according with
*                    their corresponding values in the OTG descriptor bmAttributes.
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_get_otg_attribute(void*  dev_handle , uint_8 bm_attributes)  
{
	DEV_INSTANCE_PTR  dev_inst_ptr; 
	USB_HOST_STATE_STRUCT_PTR host_struct_ptr;
	USB_OTG_STRUCT*   usb_otg_struct_ptr;
	uint_8            deviceNumber;
	USB_OTG_STATUS *  otg_status;

	if(dev_handle == NULL)
	{
		return USB_INVALID_PARAMETER;	 
	}
	dev_inst_ptr = (DEV_INSTANCE_PTR)dev_handle;
	host_struct_ptr = (USB_HOST_STATE_STRUCT_PTR)dev_inst_ptr->host;
	if((void*)host_struct_ptr == NULL)
	{
		return USB_INVALID_PARAMETER;
	}

	deviceNumber = host_struct_ptr->DEV_NUM;
	if( deviceNumber >= USB_OTG_DEV_CNT )
	{
		return USB_INVALID_PARAMETER;  
	}

	usb_otg_struct_ptr = usb_otg_struct_ptr_array[deviceNumber];
	if((void*)usb_otg_struct_ptr == NULL)
	{
		return USB_INVALID_PARAMETER;
	}
	otg_status = &usb_otg_struct_ptr->otg_status;
	otg_status->srp_support = (bm_attributes & OTG_ATTR_SRP_SUPPORT)?(TRUE):(FALSE);
	otg_status->hnp_support = (bm_attributes & OTG_ATTR_HNP_SUPPORT)?(TRUE):(FALSE);

	return USB_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_set_feature_required
* Returned Value   : set feature status
* Comments         : This function is called from usb host stack at device enumeration.
*                    More specific it is called from usb_host_cntrl_transaction_done function
*                    (in host_ch9.c) when configuration descriptor  was read ,  an OTG descriptor     .
*                    was identified in it and _usb_otg_get_otg_attribute was already called.
*                    The function decides if a_hnp_support and b_hnp_enable features have to be set
*                    in the attached peripheral.
*                    The function returns TRUE if the following two conditions are met: the peripheral  
*                    supports hnp and the host is the A device.
*END*----------------------------------------------------------------------*/
uint_8 _usb_otg_set_feature_required(void*  dev_handle )  
{
	DEV_INSTANCE_PTR  dev_inst_ptr; 
	USB_HOST_STATE_STRUCT_PTR host_struct_ptr;
	USB_OTG_STRUCT*   usb_otg_struct_ptr;
	uint_8            deviceNumber;
	USB_OTG_STATUS *  otg_status;

	if(dev_handle == NULL)
	{
		return FALSE;	 
	}
	dev_inst_ptr = (DEV_INSTANCE_PTR)dev_handle;
	host_struct_ptr = (USB_HOST_STATE_STRUCT_PTR)dev_inst_ptr->host;
	if((void*)host_struct_ptr == NULL)
	{
		return FALSE;
	}

	deviceNumber = host_struct_ptr->DEV_NUM;
	if( deviceNumber >= USB_OTG_DEV_CNT )
	{
		return FALSE;  
	}

	usb_otg_struct_ptr = usb_otg_struct_ptr_array[deviceNumber];
	if((void*)usb_otg_struct_ptr == NULL)
	{
		return FALSE;
	}
	otg_status = &usb_otg_struct_ptr->otg_status;

	if(otg_status->hnp_support && (otg_status->id == FALSE))
	{
		return TRUE;
	}

	return FALSE;
}



/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_on_interface_event
* Returned Value   : on interface event status
* Comments         : This function is called from the host application at interface event.
*                    The function simply set the dev_inst_ptr pointer in the status struct  
*                    to the (DEV_INSTANCE_PTR)dev_handle value after dev_handle value was 
*                    checked and found to be valid.
*                    The dev_inst_ptr value will be used in the OTG state machine to poll 
*                    the peripheral for hnp request.
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_on_interface_event(void*  dev_handle)  
{
	DEV_INSTANCE_PTR  dev_inst_ptr; 
	USB_HOST_STATE_STRUCT_PTR host_struct_ptr;
	USB_OTG_STRUCT*   usb_otg_struct_ptr;
	uint_8            deviceNumber;
	USB_OTG_STATUS *  otg_status;
	if(dev_handle == NULL)
	{
		return USB_INVALID_PARAMETER;	 
	}
	dev_inst_ptr = (DEV_INSTANCE_PTR)dev_handle;
	host_struct_ptr = (USB_HOST_STATE_STRUCT_PTR)dev_inst_ptr->host;
	if((void*)host_struct_ptr == NULL)
	{
		return USB_INVALID_PARAMETER;
	}

	deviceNumber = host_struct_ptr->DEV_NUM;
	if( deviceNumber >= USB_OTG_DEV_CNT )
	{
		return USB_INVALID_PARAMETER;  
	}

	usb_otg_struct_ptr = usb_otg_struct_ptr_array[deviceNumber];
	if((void*)usb_otg_struct_ptr == NULL)
	{
		return USB_INVALID_PARAMETER;
	}
	otg_status = &usb_otg_struct_ptr->otg_status;
	usb_otg_struct_ptr->dev_inst_ptr  = dev_inst_ptr;
	otg_status->host_req_poll_timer = 0;


	return USB_OK;


}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_on_detach_event
* Returned Value   : on detach status
* Comments         : This function is called from the host event function in the host application 
*                    at detach event.
*                    The function resets all peripheral related parameters in the OTG state struct
*                    if the host event function was called due to a detach event.
*                    The function doesn't take any actions if the host event function was called
*                    due to a host stack unload.
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_on_detach_event(void*  dev_handle)  
{
	DEV_INSTANCE_PTR  dev_inst_ptr; 
	USB_HOST_STATE_STRUCT_PTR host_struct_ptr;
	USB_OTG_STRUCT*   usb_otg_struct_ptr;
	uint_8            deviceNumber;
	USB_OTG_STATUS *  otg_status;

	if(dev_handle == NULL)
	{
		return USB_INVALID_PARAMETER;	 
	}
	dev_inst_ptr = (DEV_INSTANCE_PTR)dev_handle;
	host_struct_ptr = (USB_HOST_STATE_STRUCT_PTR)dev_inst_ptr->host;
	if((void*)host_struct_ptr == NULL)
	{
		return USB_INVALID_PARAMETER;
	}

	deviceNumber = host_struct_ptr->DEV_NUM;
	if( deviceNumber >= USB_OTG_DEV_CNT )
	{
		return USB_INVALID_PARAMETER;  
	}

	if((USB_OTG_REG(deviceNumber,USB0_CTL) & (USB_CTL_SE0_MASK | USB_CTL_JSTATE_MASK)) == USB_CTL_JSTATE_MASK)
	{
		return USB_OK;  
	}

	usb_otg_struct_ptr = usb_otg_struct_ptr_array[deviceNumber];
	if((void*)usb_otg_struct_ptr == NULL)
	{
		return USB_INVALID_PARAMETER;
	}
	usb_otg_struct_ptr->dev_inst_ptr = NULL;
	otg_status = &usb_otg_struct_ptr->otg_status;
	otg_status->a_conn = FALSE;
	otg_status->b_conn = 0; 
	otg_status->hnp_req = 0;
	otg_status->srp_support = 0;
	otg_status->hnp_support = 0;
	otg_status->a_set_b_hnp_en = 0;

	return USB_OK;
}
 
 /*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_hnp_poll_req
* Returned Value   : HNP poll request status
* Comments         : This function is called from the otg stack by the host device to check
*                    if the peripheral device wants to become host.   
*                    The function initiates a get status request with the otg status selector in the
*                    wIndex field.
*END*----------------------------------------------------------------------*/
uint_32 _usb_otg_hnp_poll_req (_usb_otg_handle handle)  
{
	DEV_INSTANCE_PTR  dev_inst_ptr; 
	USB_OTG_STRUCT*   usb_otg_struct_ptr;
	USB_OTG_STATUS *  otg_status;
	USB_SETUP           req;
	uint_16             dummy;
	uint_32             status ;

	if(handle == NULL)
	{
		return USB_INVALID_PARAMETER;
	}

	usb_otg_struct_ptr = (USB_OTG_STRUCT*)handle;  
	dev_inst_ptr = usb_otg_struct_ptr->dev_inst_ptr ;
	if((void*)dev_inst_ptr == NULL)
	{
		return USB_INVALID_PARAMETER;
	}
	otg_status = &usb_otg_struct_ptr->otg_status;

	req.BMREQUESTTYPE = OTG_STATUS_BM_REQ_TYPE;
	req.BREQUEST = REQ_GET_STATUS;
	dummy =0;
	htou16(req.WVALUE, dummy);
	dummy = OTG_STATUS_SELECTOR;
	htou16(req.WINDEX, dummy);
	dummy = 1;    
	htou16(req.WLENGTH, dummy);
	status = _usb_hostdev_cntrl_request(dev_inst_ptr, &req, &otg_status->hnp_req, NULL, NULL);
  
	return status;  
}

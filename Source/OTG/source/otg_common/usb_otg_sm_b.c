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
static void _usb_otg_sm_b_substate_change(_usb_otg_handle otg_handle, uint_8 new_state, uint_32 sm_indication);

/* Public functions *********************************************************/

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_sm_b
* Returned Value   :
* Comments         : This function handles the substates of the B-state machine
*    
*
*END*----------------------------------------------------------------------*/
void _usb_otg_sm_b(_usb_otg_handle otg_handle)
{
 USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;
 USB_OTG_STATUS *otg_status =  &usb_otg_struct_ptr->otg_status;

 switch(usb_otg_struct_ptr->subState)
 {
  case USB_OTG_SM_B_IDLE_SESS_DETECT:
      /* the id is changed to 0 */
    if(!otg_status->id)
    {  
        _usb_otg_id_chg_a(otg_handle);
        return;
    }
    usb_otg_struct_ptr->hnpEnabled = FALSE;
    if(otg_status->sess_valid)
    {
        _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_PERI_BUS_SUSP_DETECT, OTG_B_PERIPHERAL);                 
    }
    else
    {
        /* Disable pull-up. The B-Device is now in the B-Idle state */
        _usb_otg_dp_pullup_enable(otg_handle, FALSE);

        if(otg_status->sess_end)
        {              
             usb_otg_struct_ptr->subState = USB_OTG_SM_B_IDLE_SESS_END_DETECT;
             
             otg_status->b_timeout = TB_SESSEND_SRP; /* Program the SRP detect timeout as SESSEND SRP */
             /* Init SE0 detection. remove the D+ pullup */
             _usb_otg_init_se0_detect(otg_handle);
        }
    }
    break;
  
  case USB_OTG_SM_B_IDLE_SESS_END_DETECT:
      /* the id is changed to 0 */
    if(!otg_status->id)
    {  
        _usb_otg_id_chg_a(otg_handle);
        return;
    }
    if(otg_status->sess_valid)
    {
        _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_PERI_BUS_SUSP_DETECT, OTG_B_PERIPHERAL);
    }
    else
    {              
        /* Read the Live SE0 bit */
        if((otg_status->live_se0) && (otg_status->line_stable))
        {                                
            otg_status->b_timeout_en = TRUE;
            usb_otg_struct_ptr->subState = USB_OTG_SM_B_IDLE_SE0_STABLE_WAIT;
        }            
    }
    break;
  
  case USB_OTG_SM_B_IDLE_SE0_STABLE_WAIT:
    /* the id is changed to 0 */
    if(!otg_status->id)
    {  
        _usb_otg_id_chg_a(otg_handle);
        return;
    }
    if(otg_status->sess_valid)
    {
        _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_PERI_BUS_SUSP_DETECT, OTG_B_PERIPHERAL);
    }
    else
    {
        /* Line state change. restart SE0 detection */
        if(!otg_status->line_stable)
        {               
            otg_status->b_timeout_en = TRUE;

            otg_status->b_timeout = TB_SE0_SRP; /* reinitialize the the SE0 detect timer */

            /* Keep the current state */
        }
        else
        {
            if(otg_status->b_timeout == 0)
            {
                /* The timeout expired during stable SE0 */
                _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_IDLE_SRP_START_ARMED, OTG_B_IDLE_SRP_READY);               
            }
        }
    }
     break; 
         
  case USB_OTG_SM_B_IDLE_SRP_START_ARMED:
    /* the id is changed to 0 */
    if(!otg_status->id)
    {  
        _usb_otg_id_chg_a(otg_handle);
        return;
    }
    if(otg_status->sess_valid)
    {
       _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_PERI_BUS_SUSP_DETECT, OTG_B_PERIPHERAL);
    }
    else
    {                
        /* Line state change. restart SE0 detection */
        if(!otg_status->line_stable)
        {
            otg_status->b_timeout = TB_SE0_SRP; /* reinitialize the the SE0 detect timer */              
            usb_otg_struct_ptr->subState = USB_OTG_SM_B_IDLE_SESS_END_DETECT;
        }
        else
        {
            if(usb_otg_struct_ptr->srpRequest || usb_otg_struct_ptr->powerUp)
            {
                usb_otg_struct_ptr->powerUp = FALSE;
                usb_otg_struct_ptr->srpRequest = FALSE;
                /* Start SRP */                                  
                /* Start the D+ pulsing timeout */              
                otg_status->b_timeout = TB_DATA_PLS; /* reinitialize the the SE0 detect timer */                 
                otg_status->b_timeout_en = TRUE;

                /* Enable D+ pullup */
                _usb_otg_dp_pullup_enable(otg_handle, TRUE);

                usb_otg_struct_ptr->subState = USB_OTG_SM_B_SRP_PULSE;                                                   
            }
        }
    }
    break;
         
  case USB_OTG_SM_B_SRP_PULSE:
    if(otg_status->b_timeout == 0)
    {
        /* The timeout expired. Remove the D+ pullup */
        _usb_otg_dp_pullup_enable(otg_handle, FALSE);               

        /* Wait for VBUS */
        otg_status->b_timeout = (TB_SRP_FAIL - TB_DATA_PLS);
        otg_status->b_timeout_en = TRUE;

        usb_otg_struct_ptr->subState = USB_OTG_SM_B_SRP_VBUS_WAIT;

        /* Signal the event to the application */
         _usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_B_SRP_INIT);               
    }
    break;
          
  case USB_OTG_SM_B_SRP_VBUS_WAIT:
    if(otg_status->sess_valid)
    {
         _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_PERI_BUS_SUSP_DETECT, OTG_B_PERIPHERAL);
    }
    else
    {
      if(otg_status->b_timeout == 0)
      {
       /* The timeout expired during VBUS wait */               
                                                  
       /* Inform the application about the failed SRP and return to idle state */               
       /* and wait for SE0 condition on the bus to be able to restart the SRP */
       
       usb_otg_struct_ptr->subState = USB_OTG_SM_B_IDLE_SE0_STABLE_WAIT;
       
       otg_status->b_timeout_en = TRUE;
      
       otg_status->b_timeout = TB_SE0_SRP; /* reinitialize the the SE0 detect timer */
       
       /* Signal the event to the application */
         _usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_B_SRP_FAIL);
      }
    }            
  break;
  
  case USB_OTG_SM_B_PERI_BUS_SUSP_DETECT:
    if((!otg_status->sess_valid)||(!otg_status->id))
    {
         _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_IDLE_SESS_DETECT, OTG_B_IDLE); 
    }
    else
    {              
         /* Enable the D+ pull-up. the B device is in the peripheral state */
         _usb_otg_dp_pullup_enable(otg_handle, TRUE);
         
         
         usb_otg_struct_ptr->powerUp = FALSE;
         
         /* Start monitoring the data lines. 
          * If the bus has inactivity for more than TB_AIDL_BDIS, then the A is considered disconnected and B can start HNP
          */ 
         usb_otg_struct_ptr->subState = USB_OTG_SM_B_PERI_BUS_SUSP_WAIT;
        
         otg_status->b_timeout_en = TRUE;
          
         _usb_device_set_status(&(usb_otg_struct_ptr->deviceNumber), USB_STATUS_OTG, USB_STATUS_IDLE);
         otg_status->b_timeout = TB_AIDL_BDIS; /* reinitialize the IDLE detect timer */             
    }
        
     break;
  
  case USB_OTG_SM_B_PERI_BUS_SUSP_WAIT:
    if((!otg_status->sess_valid)||(!otg_status->id))
    {
        _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_IDLE_SESS_DETECT, OTG_B_IDLE); 
    }
    else
    {              
        if(!otg_status->line_stable)
        {              
            /* Restart detection */

            otg_status->b_timeout_en = TRUE;

            otg_status->b_timeout = TB_AIDL_BDIS; /* reinitialize the IDLE detect timer */
        }
        else
        {
            if((usb_otg_struct_ptr->hnpEnabled) && (otg_status->b_timeout == 0))
            {               
                usb_otg_struct_ptr->subState = USB_OTG_SM_B_PERI_HNP_ARMED;    
                /* Signal the event to the application */
                _usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_B_PERIPHERAL_HNP_READY); 
            }
        }
    }
       
  break;
  
  case USB_OTG_SM_B_PERI_HNP_ARMED:
    if((!otg_status->sess_valid)||(!otg_status->id))
    {
        _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_IDLE_SESS_DETECT, OTG_B_IDLE); 
    }
    else
    {              
        if((!otg_status->line_stable) || (!usb_otg_struct_ptr->hnpEnabled))
        {
            _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_PERI_BUS_SUSP_DETECT, OTG_B_PERIPHERAL_HNP_FAIL);
        }
        else
        {
            if((usb_otg_struct_ptr->busRequest) && (usb_otg_struct_ptr->hnpEnabled))
            {
                usb_otg_struct_ptr->busRequest = FALSE;

                /* Clear the Status at the USB device level */

                _usb_device_set_status(&(usb_otg_struct_ptr->deviceNumber), USB_STATUS_OTG, USB_STATUS_IDLE);


                /* Start HNP. Turn off Pull-Up on D+ for the Host to detect SE0 */
                _usb_otg_dp_pullup_enable(otg_handle, FALSE);
                /* Wait for data line to discharge (25us) */
                otg_status->b_timeout = 1;
                otg_status->b_timeout_en = TRUE;
                usb_otg_struct_ptr->subState = USB_OTG_SM_B_PERI_HNP_START;
                /* Signal the event to the application */
                _usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_B_PERIPHERAL_HNP_START);                    
            }
        }
    }              
    break;
  
  case USB_OTG_SM_B_PERI_HNP_START:          
    if((!otg_status->sess_valid)||(!otg_status->id))
    {
        _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_IDLE_SESS_DETECT, OTG_B_IDLE); 
    }
    else
    {
        if(otg_status->b_timeout == 0)
        {
            /* Line should have discharged by now */
            /* Start the host disconnect detect */

            otg_status->b_timeout = TB_ASE0_BRST;
            otg_status->b_timeout_en = TRUE;              
            usb_otg_struct_ptr->subState = USB_OTG_SM_B_PERI_HNP_ACONN;    
        }
    }
    break;
          
   case USB_OTG_SM_B_PERI_HNP_ACONN: 
    if((!otg_status->sess_valid)||(!otg_status->id))
    {
        _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_IDLE_SESS_DETECT, OTG_B_IDLE); 
    }        
    else
    {
        if(otg_status->b_timeout == 0)
        {
            /* A connect timeout detected. Go back to peripheral state */
            _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_PERI_BUS_SUSP_DETECT, OTG_B_PERIPHERAL_HNP_FAIL);
        }             
        else
        {
            /* Check the Data line state */
            if((!otg_status->live_se0) && (otg_status->live_jstate) && (otg_status->line_stable))
            {
                /* J-STATE. Host has been released */
                /* Enter the B-Host state */
                usb_otg_struct_ptr->hnpEnabled = FALSE;
                otg_status->a_conn = TRUE;
                _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_HOST, OTG_B_HOST);
            }
            else
            {
                if((!otg_status->live_se0) && (!otg_status->live_jstate) && (otg_status->line_stable))
                {
                    /* Host has retained the bus */
                    _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_PERI_BUS_SUSP_DETECT, OTG_B_PERIPHERAL_HNP_FAIL);               
                }               
            }
        }
    }
    break;
   case USB_OTG_SM_B_HOST:
    if(!otg_status->sess_valid)
    {
        otg_status->a_conn = FALSE;
        _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_IDLE_SESS_DETECT, OTG_B_IDLE); 
    }
    else
    {
        //if((otg_status->live_se0 && otg_status->line_stable) ||
        if((otg_status->a_conn == FALSE) ||
        usb_otg_struct_ptr->busRelease
        ) 
            {
                usb_otg_struct_ptr->busRelease = FALSE;
                otg_status->a_conn = FALSE;
                /* A-device disconnected or B has finished using the bus */
                _usb_otg_sm_b_substate_change(otg_handle, USB_OTG_SM_B_PERI_BUS_SUSP_DETECT, OTG_B_PERIPHERAL);
            } 
        else 
        {
            if( usb_otg_struct_ptr->dev_inst_ptr != NULL )
            {
                if(otg_status->hnp_req)
                {
                    _usb_event_set(&(usb_otg_struct_ptr->otg_app_event), OTG_B_A_HNP_REQ);
                }

                if(otg_status->hnp_support && (otg_status->host_req_poll_timer >= T_HOST_REQ_POLL))
                {
                     otg_status->host_req_poll_timer = 0;
                     _usb_otg_hnp_poll_req (usb_otg_struct_ptr);  
                } 
            }
        }
    }
    break;          
  default: break;                          
 }  
}

/* Private functions ********************************************************/

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _usb_otg_sm_b_substate_change
* Returned Value   :
* Comments         : This function handles the actions performed at B substate change
*    
*
*END*------------------------------------------------------------------------*/

static void _usb_otg_sm_b_substate_change(_usb_otg_handle otg_handle, uint_8 new_state, uint_32 sm_indication)
{
   USB_OTG_STRUCT *usb_otg_struct_ptr = (USB_OTG_STRUCT *)otg_handle;
   USB_OTG_STATUS *otg_status =  &usb_otg_struct_ptr->otg_status;
   
   switch(new_state)
   {
    case USB_OTG_SM_B_IDLE_SESS_DETECT:
       _usb_otg_dp_pullup_enable(otg_handle, FALSE);
       /* Unload the active USB stack if any */
       usb_otg_struct_ptr->init_struct->unload_usb_active(); 
       if(usb_otg_struct_ptr->init_struct->ext_set_pdowns != (otg_ext_set_pdowns)NULL)
         {
           usb_otg_struct_ptr->init_struct->ext_set_pdowns(0);
         }

                
       break;
     
    case USB_OTG_SM_B_PERI_BUS_SUSP_DETECT:
       _usb_otg_dp_pullup_enable(otg_handle, TRUE);
       if(sm_indication == OTG_B_PERIPHERAL)
       {
         /* Unload the active USB stack if any. Could be that the Host stack is active */ 
         usb_otg_struct_ptr->init_struct->unload_usb_active(); 
         
         /* Load the Peripheral stack */
         if(usb_otg_struct_ptr->init_struct->ext_set_pdowns != (otg_ext_set_pdowns)NULL)
         {
           usb_otg_struct_ptr->init_struct->ext_set_pdowns(OTG_CTRL_PDOWN_DM);
         }
         if(usb_otg_struct_ptr->init_struct->load_usb_device() != USB_OK)
         {
          sm_indication = OTG_B_PERIPHERAL_LOAD_ERROR;
         }
       }
       break;  
    
    case USB_OTG_SM_B_HOST:
         usb_otg_struct_ptr->dev_inst_ptr = NULL;
         otg_status->hnp_req = 0;
         otg_status->srp_support = 0;
         otg_status->hnp_support = 0;

         /* Unload the active USB stack if any. Most probably the peripheral stack is active */
         usb_otg_struct_ptr->init_struct->unload_usb_active(); 
         if(usb_otg_struct_ptr->init_struct->ext_set_pdowns != (otg_ext_set_pdowns)NULL)
         {
           usb_otg_struct_ptr->init_struct->ext_set_pdowns( OTG_CTRL_PDOWN_DP | OTG_CTRL_PDOWN_DM);
         }
         
         /* Load the Host stack */
         if(usb_otg_struct_ptr->init_struct->load_usb_host() != USB_OK)
         {
          sm_indication = OTG_B_HOST_LOAD_ERROR;
         }
     break;
       
    default :   break;
   }
   usb_otg_struct_ptr->subState = new_state;
   _usb_event_set(&(usb_otg_struct_ptr->otg_app_event), sm_indication);
}

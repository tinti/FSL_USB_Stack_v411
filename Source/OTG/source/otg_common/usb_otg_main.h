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
*   This is the header file for the OTG driver
*
*END************************************************************************/

#ifndef USB_OTG_MAIN_H_
#define USB_OTG_MAIN_H_


#include "user_config.h"

/* Public constants */

/* Number of available USB device controllers */
#define USB_OTG_DEV_CNT			1

#if (USB_OTG_DEV_CNT == 0)
 #error "USB_OTG_DEV_CNT (number of USB controllers) cannot be configured to 0"
#endif

/* B state machine indications */
#define OTG_B_IDLE				        ((uint_32)0x0001)  	  /* B idle state */
#define OTG_B_IDLE_SRP_READY	        ((uint_32)0x0002)     /* B idle, SRP ready to start */
#define OTG_B_SRP_INIT			        ((uint_32)0x0004)     /* B srp init state */
#define OTG_B_SRP_FAIL			        ((uint_32)0x0008)     /* B srp failed to get a response */
#define OTG_B_PERIPHERAL	 	        ((uint_32)0x0010)     /* B peripheral state */
#define OTG_B_PERIPHERAL_LOAD_ERROR     ((uint_32)0x0020)     /* B peripheral state (peripheral stack could not be loaded) */
#define OTG_B_PERIPHERAL_HNP_READY	 	((uint_32)0x0040)     /* B peripheral, HNP ready to be performed */
#define OTG_B_PERIPHERAL_HNP_START	 	((uint_32)0x0080)     /* B peripheral, HNP start */
#define OTG_B_PERIPHERAL_HNP_FAIL 	 	((uint_32)0x0100)     /* B peripheral, HNP failed */
#define OTG_B_HOST				        ((uint_32)0x0200)     /* B host state */
#define OTG_B_HOST_LOAD_ERROR           ((uint_32)0x0400)     /* B host state (host stack could not be loaded) */
#define OTG_B_A_HNP_REQ                 ((uint_32)0x0800)
#if HIGH_SPEED_DEVICE
#define USBHS_OTGSC_W1C					0x7F0000U
#endif

/* A state machine indications */
#define OTG_A_IDLE      	            ((uint_32)0x00010000)
#define OTG_A_WAIT_VRISE                ((uint_32)0x00020000)
#define OTG_A_WAIT_BCON                 ((uint_32)0x00040000)
#define OTG_A_HOST				        ((uint_32)0x00080000)
#define OTG_A_SUSPEND                   ((uint_32)0x00100000)
#define OTG_A_PERIPHERAL		        ((uint_32)0x00200000)
#define OTG_A_WAIT_VFALL                ((uint_32)0x00400000)
#define OTG_A_VBUS_ERR                  ((uint_32)0x00800000)

#define OTG_A_WAIT_VRISE_TMOUT          ((uint_32)0x01000000)
#define OTG_A_WAIT_BCON_TMOUT           ((uint_32)0x02000000)
#define OTG_A_B_HNP_REQ                 ((uint_32)0x04000000)
#define OTG_A_BIDL_ADIS_TMOUT           ((uint_32)0x08000000)

#define OTG_A_ID_TRUE                   ((uint_32)0x10000000)
#define OTG_A_HOST_LOAD_ERROR           ((uint_32)0x20000000)
#define OTG_A_PERIPHERAL_LOAD_ERROR     ((uint_32)0x40000000)
#define OTG_A_AIDL_BDIS_TMOUT           ((uint_32)0x80000000)

#if !HIGH_SPEED_DEVICE
#define OTG_CTRL_PDOWN_DP               ((uint_8)0x01)
#define OTG_CTRL_PDOWN_DM               ((uint_8)0x02)
#else
#define OTG_CTRL_PDOWN_DP               ((uint_8)0x02)
#define OTG_CTRL_PDOWN_DM               ((uint_8)0x04)
#endif

#define OTG_ATTR_SRP_SUPPORT            ((uint_8)0x01)
#define OTG_ATTR_HNP_SUPPORT            ((uint_8)0x02)


/* Public types */
typedef uint_32 OTG_EVENT;
typedef void*   _usb_otg_handle;

/**** Function types for interfacing an external OTG circuit ****/ 

/* Function for enabling / disabling the external circuit */
typedef void (*otg_ext_enable_disable)(boolean enable);

/* Function for enabling / disabling the VBUS generator*/
typedef void (*otg_ext_set_VBUS)(boolean a_device);

/* Function for getting the status from the external circuit */
typedef uint_8 (*otg_ext_get_status)(void);

/* Function for getting the active interrupts from the external circuit */
typedef uint_8 (*otg_ext_get_interrupts)(void);

/* Function for getting the active interrupts from the external circuit */
typedef void (*otg_ext_set_pdowns)(uint_8 bitfield);

/* Function for getting the active interrupts from the external circuit */
typedef uint_32 (*otg_load_usb_stack)(void);
typedef void (*otg_unload_usb_stack)(void);


/* OTG initialization structure type */
#ifdef __CC_ARM
  #pragma push
  #pragma pack(1)
#endif
#ifdef __GNUC__
  #pragma pack(1)
#endif
typedef struct otg_init_struct
{
	boolean					ext_circuit_use;			// #1
	otg_ext_enable_disable	ext_enable_disable_func;	// #2
	otg_ext_get_status		ext_get_status_func;		// #3
	otg_ext_get_interrupts	ext_get_interrupts_func;	// #4
	otg_ext_set_VBUS		ext_set_VBUS;				// #5
	otg_ext_set_pdowns		ext_set_pdowns;				// #6
	otg_load_usb_stack		load_usb_host;				// #7
	otg_load_usb_stack		load_usb_device;			// #8
	otg_unload_usb_stack	unload_usb_host;			// #9
	otg_unload_usb_stack	unload_usb_device;			// #10
	otg_unload_usb_stack	unload_usb_active;			// #11
}OTG_INIT_STRUCT;


typedef void    (*otg_event_callback)(_usb_otg_handle handle, OTG_EVENT event);

/* Public functions */
extern uint_32    _usb_otg_init( uint_8 otg_device_number, OTG_INIT_STRUCT *init_struct, _usb_otg_handle *otg_handle);
extern uint_32    _usb_otg_register_callback(_usb_otg_handle handle, otg_event_callback callback);
extern uint_32    _usb_otg_session_request(_usb_otg_handle handle);
extern uint_32    _usb_otg_bus_request(_usb_otg_handle handle);
extern uint_32    _usb_otg_bus_release(_usb_otg_handle handle);
extern uint_32    _usb_otg_hnp_enable(uint_8 otg_device_number, uint_8 enable);
extern void       _usb_otg_task(void);
extern void       _usb_otg_ext_isr(uint_8 otg_device_number);
extern void       _usb_otg_isr(uint_8 otg_device_number);
extern uint_32 	  _usb_otg_set_a_bus_req(_usb_otg_handle otg_handle , boolean a_bus_req );
extern uint_32    _usb_otg_set_a_bus_drop(_usb_otg_handle otg_handle , boolean a_bus_drop );
extern uint_32    _usb_otg_set_a_clear_err( _usb_otg_handle otg_handle );
extern uint_32    _usb_otg_a_set_b_hnp_en(void* dev_handle , boolean b_hnp_en);
extern uint_32    _usb_otg_on_interface_event(void*  dev_handle) ;
extern uint_32    _usb_otg_hnp_poll_req (_usb_otg_handle handle) ;
extern uint_32    _usb_otg_on_detach_event(void*  dev_handle) ;
extern uint_32    _usb_otg_get_otg_attribute(void*  dev_handle , uint_8 bm_attributes);
extern uint_8     _usb_otg_set_feature_required(void*  dev_handle );

#ifdef __CC_ARM
  #pragma pop
#endif
#ifdef __GNUC__
  #pragma options align=reset
#endif
#endif /* USB_OTG_FS_MAIN_H_ */

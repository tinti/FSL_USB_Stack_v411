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
* $FileName: msd_host.c$
* $Version : 3.4.21.0$
* $Date    : Nov-12-2010$
*
* Comments:
*
*   This file contains device driver for mass storage class. This code tests
*   the UFI set of commands.
*
*END************************************************************************/

/**************************************************************************
Include the OS and BSP dependent files that define IO functions and
basic types. You may like to change these files for your board and RTOS 
**************************************************************************/
/**************************************************************************
Include the USB stack header files.
**************************************************************************/
#include "types.h"
#include "hostapi.h"
#include "derivative.h" /* include peripheral declarations */
#include "host_common.h"
#include "usb.h"
#include "usb_bsp.h"
#include "khci.h"
//#include "usb_host_hid.h"
#include "usbevent.h"
#include "msd_host.h"
#include "sci.h"
#include "rtc.h"
//#include "fio.h"
#include "hidef.h"
#include "usb_host_msd_bo.h"
#include "usb_host_msd_ufi.h"
#include "usb_host_hub_sm.h"
#include "usb_otg_main.h"

#if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
#include "exceptions.h"
#endif


/**************************************************************************
   Global variables
**************************************************************************/

extern uint_32 host_stack_active;      /* TRUE if the host stack is active */           //tung
extern uint_32 dev_stack_active;       /* TRUE if the peripheral stack is active */

volatile DEVICE_STRUCT      mass_device = { 0 };   /* mass storage device struct */
_usb_host_handle            host_handle;
volatile boolean            bCallBack   = FALSE;
volatile USB_STATUS         bStatus     = USB_OK;
volatile uint_32            dBuffer_length      = 0;
boolean                     Attach_Notification = FALSE;

static uint_32 test = 0, ontest;

/* the following is the mass storage class driver object structure. This is
used to send commands down to  the class driver. See the Class API document
for details */

COMMAND_OBJECT_STRUCT pCmd;

/* some handles for communicating with USB stack */
uchar buff_in[BUFF_IN_SIZE], buff_out[0x0F];

/* The following MSD data blocks shall be aligned at multiple of 4 bytes */
#ifdef __CWCC__ 	
	//static uint_8  bLun = 0;
	
	#pragma define_section usb_msd ".usb_msd" RW 
	__declspec(usb_msd) static REQ_SENSE_DATA_FORMAT                      	req_sense;
	__declspec(usb_msd) static CAPACITY_LIST                              	capacity_list;
	__declspec(usb_msd) static MASS_STORAGE_READ_CAPACITY_CMD_STRUCT_INFO 	read_capacity;
	__declspec(usb_msd) static INQUIRY_DATA_FORMAT   						inquiry;	
#elif defined __IAR_SYSTEMS_ICC__
	static INQUIRY_DATA_FORMAT                        inquiry       @ "usb_msd";	
	static MASS_STORAGE_READ_CAPACITY_CMD_STRUCT_INFO read_capacity @ "usb_msd";
	static CAPACITY_LIST                              capacity_list @ "usb_msd";
	static REQ_SENSE_DATA_FORMAT                      req_sense     @ "usb_msd";	
    static uint_8                                     bLun          @ "usb_msd";
#elif (defined __CC_ARM)|| (defined __arm__)
	static INQUIRY_DATA_FORMAT                        inquiry;	
	static MASS_STORAGE_READ_CAPACITY_CMD_STRUCT_INFO read_capacity;
	static CAPACITY_LIST                              capacity_list;
	static REQ_SENSE_DATA_FORMAT                      req_sense;	
    static uint_8                                     bLun;
#endif

void MassStorage_Task(void);

const USB_HOST_DRIVER_INFO DriverInfoTable[] = 
{
   /* Floppy drive */
   {
      {0x00,0x00},                  /* Vendor ID per USB-IF             */
      {0x00,0x00},                  /* Product ID per manufacturer      */
      USB_CLASS_MASS_STORAGE,       /* Class code                       */
      USB_SUBCLASS_MASS_UFI,        /* Sub-Class code                   */
      USB_PROTOCOL_MASS_BULK,       /* Protocol                         */
      0,                            /* Reserved                         */
      usb_host_mass_device_event    /* Application call back function   */
   },

   /* USB 2.0 hard drive */
   {

      {0x49,0x0D},                  /* Vendor ID per USB-IF             */
      {0x00,0x30},                  /* Product ID per manufacturer      */
      USB_CLASS_MASS_STORAGE,       /* Class code                       */
      USB_SUBCLASS_MASS_SCSI,       /* Sub-Class code                   */
      USB_PROTOCOL_MASS_BULK,       /* Protocol                         */
      0,                            /* Reserved                         */
      usb_host_mass_device_event    /* Application call back function   */
   },

   /* USB 1.1 hub */
   {

      {0x00,0x00},                  /* Vendor ID per USB-IF             */
      {0x00,0x00},                  /* Product ID per manufacturer      */
      USB_CLASS_HUB,                /* Class code                       */
      USB_SUBCLASS_HUB_NONE,        /* Sub-Class code                   */
      USB_PROTOCOL_HUB_LS,          /* Protocol                         */
      0,                            /* Reserved                         */
      usb_host_hub_device_event     /* Application call back function   */
   },

   {
      {0x00,0x00},                  /* All-zero entry terminates        */
      {0x00,0x00},                  /*    driver info list.             */
      0,
      0,
      0,
      0,
      NULL
   }
};

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : App_Host_Init(void)
* Returned Value : 
* Comments       : Host initialization
*     
*
*END*--------------------------------------------------------------------*/
USB_STATUS App_Host_Init(void) 
{
	USB_STATUS           status = USB_OK;
	mass_device.dev_state = USB_DEVICE_IDLE;

	host_stack_active  = TRUE;
	dev_stack_active = FALSE;  

	TimerInit();
	DisableInterrupts;

#if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
	usb_int_dis();
#endif  

	status = _usb_host_init
	(HOST_CONTROLLER_NUMBER,    /* Use value in header file */
			MAX_FRAME_SIZE,            /* Frame size per USB spec  */
			&host_handle);             /* Returned pointer */
#ifdef _MCF51JM128_H
	PTCD &= 0xbf;
#endif
	if (status != USB_OK) 
	{
		printf("\n\r   USB Host Initialization failed. STATUS: %x", status);    
		fflush(stdout);
		return status;
	} /* Endif */

	/*
	 ** Since we are going to act as the host driver, register
	 ** the driver information for wanted class/subclass/protocols
	 */
	status = _usb_host_driver_info_register(host_handle, (void*)DriverInfoTable);
	if(status != USB_OK) 
	{
		printf("\n\r   Driver Registration failed. STATUS: %x", status); 
		return status;
	}

	EnableInterrupts;
#if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
	usb_int_en();
#endif

	pCmd.CBW_PTR = (CBW_STRUCT_PTR) malloc(sizeof(CBW_STRUCT));  
	if(pCmd.CBW_PTR == NULL)
	{
		printf ("\n\r   Unable to allocate Command Block Wrapper");
		fflush(stdout);
		return USBERR_ALLOC;     
	}
	memset(pCmd.CBW_PTR, 0, sizeof(CBW_STRUCT));

	pCmd.CSW_PTR = (CSW_STRUCT_PTR) malloc(sizeof(CSW_STRUCT));   
	if(pCmd.CSW_PTR == NULL)
	{
		printf ("\n\r   Unable to allocate Command Status Wrapper");
		USB_mem_free(pCmd.CBW_PTR);
		return USBERR_ALLOC;
	}    
	memset(pCmd.CSW_PTR, 0, sizeof(CSW_STRUCT));
 
	printf("\n\r   USB MSD Command test\n\r   Waiting for USB mass storage to be attached...\n\r");
	fflush(stdout); 

	return USB_OK;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : App_Host_Shut_Down(void)
* Returned Value   :
* Comments         : Unload host
*    
*
*END*----------------------------------------------------------------------*/

void App_Host_Shut_Down(void) 
{
	USB_STATUS status = USB_OK;   

	DisableInterrupts;   
#if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
	usb_int_dis();
#endif  

	USB_mem_free(pCmd.CBW_PTR);
	USB_mem_free(pCmd.CSW_PTR);

	usb_dev_list_detach_device(host_handle, 0, 0); 
	_usb_host_shutdown(host_handle);

	EnableInterrupts;
#if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
	usb_int_en();
#endif  

	host_stack_active = FALSE;
	dev_stack_active  = FALSE;
}


/******************************************************************************
 *
 *   @name        App_HostTask
 *
 *   @brief       Application task function. It is called from the main loop
 *
 *   @param       None
 *
 *   @return      None
 *
 *****************************************************************************
 * Application task function. It is called from the main loop
 *****************************************************************************/
void App_Host_Task(void)
{
	/* call the periodic task function */
#if !HIGH_SPEED_DEVICE
	_usb_khci_task();
#endif
	MassStorage_Task();
}



/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : main
* Returned Value : none
* Comments       : Mass Storage Task
*    
*
*END*--------------------------------------------------------------------*/

void MassStorage_Task(void)
{ /* Body */
	USB_STATUS status = USB_OK;

	/*----------------------------------------------------**
	 ** Infinite loop, waiting for events requiring action **
	 **----------------------------------------------------*/
	switch(mass_device.dev_state) 
	{
		case USB_DEVICE_IDLE:
			break;
			
		case USB_DEVICE_ATTACHED:
			printf("\n\r   Mass Storage Device Attached\n\r");
			mass_device.dev_state = USB_DEVICE_SET_INTERFACE_STARTED;
			status = _usb_hostdev_select_interface(mass_device.dev_handle,
					mass_device.intf_handle, (pointer)&(mass_device.class_intf));
	
			test = 0;
			break;

		case USB_DEVICE_SET_INTERFACE_STARTED:
			break;
			
		case USB_DEVICE_INTERFACED:                        
			usb_host_mass_test_storage();
			//mass_device.dev_state = USB_DEVICE_OTHER;
			break;
	
		case USB_DEVICE_DETACHED:
#ifndef FIXED_DETACH_ERROR
		printf("\n\r   Mass Storage Device Detached\n\r");
		mass_device.dev_state = USB_DEVICE_IDLE;
#endif
			break;
	
		case USB_DEVICE_OTHER:
			break;
			
		default:
			printf("   Unknown Mass Storage Device State = %d\n\r",\
					mass_device.dev_state);
			break;
	} /* Endswitch */
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : usb_host_mass_device_event
* Returned Value : None
* Comments       : Called when mass storage device has been attached, detached, etc.
*     
*END*--------------------------------------------------------------------*/
void usb_host_mass_device_event
   (
      /* [IN] pointer to device instance */
      _usb_device_instance_handle      dev_handle,

      /* [IN] pointer to interface descriptor */
      _usb_interface_descriptor_handle intf_handle,

      /* [IN] code number for event causing callback */
      uint_32           event_code
   )
{ /* Body */
	INTERFACE_DESCRIPTOR_PTR   intf_ptr =
		(INTERFACE_DESCRIPTOR_PTR)intf_handle;

	switch (event_code) 
	{
	case USB_CONFIG_EVENT:
		/* Drop through into attach, same processing */
	case USB_ATTACH_EVENT:
		if(mass_device.dev_state == USB_DEVICE_IDLE) 
		{
			mass_device.dev_handle = dev_handle;
			mass_device.intf_handle = intf_handle;
			mass_device.dev_state = USB_DEVICE_ATTACHED;
		} 
		else 
		{
			printf("   Mass Storage Device Is Already Attached\n\r");
		} /* EndIf */
		break;

	case USB_INTF_EVENT:
		mass_device.dev_state = USB_DEVICE_INTERFACED;
		_usb_otg_on_interface_event(dev_handle);
		break;

	case USB_DETACH_EVENT:
#ifdef _MCF51JM128_H
         PTCD &= 0xbf;    
#endif
         if(mass_device.dev_state != USB_DEVICE_IDLE) 
         {
        	 mass_device.dev_handle = NULL;
        	 mass_device.intf_handle = NULL;
        	 mass_device.dev_state = USB_DEVICE_DETACHED;
#ifdef FIXED_DETACH_ERROR
        	 printf("\n\r   Mass Storage Device Detached\n\r");
        	 mass_device.dev_state = USB_DEVICE_IDLE;
#endif
         } 
         else 
         {
        	 printf("   Mass Storage Device Is Not Attached\n\r");
         } /* EndIf */
         _usb_otg_on_detach_event(dev_handle); 
         break;

	default:
		mass_device.dev_state = USB_DEVICE_IDLE;
		break;
	} /* EndSwitch */
} /* Endbody */


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : usb_host_mass_ctrl_calback
* Returned Value : None
* Comments       : Called on completion of a control-pipe transaction.
*     
*
*END*--------------------------------------------------------------------*/
static void usb_host_mass_ctrl_callback
(
		/* [IN] pointer to pipe */
		_usb_pipe_handle  pipe_handle,

		/* [IN] user-defined parameter */
		pointer           user_parm,

		/* [IN] buffer address */
		uchar_ptr         buffer,

		/* [IN] length of data transferred */
		uint_32           buflen,

		/* [IN] status, hopefully USB_OK or USB_DONE */
		uint_32           status
)
{ /* Body */
	
	UNUSED(pipe_handle)
	UNUSED(user_parm)
	UNUSED(buffer)
	UNUSED(buflen)
	
	bCallBack = TRUE;
	bStatus = status;

} /* Endbody */


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : callback_bulk_pipe
* Returned Value : None
* Comments       : Called on completion of a bulk-pipe transaction.
*     
*END*--------------------------------------------------------------------*/
void callback_bulk_pipe
(
		/* [IN] Status of this command */
		USB_STATUS status,

		/* [IN] pointer to USB_MASS_BULK_ONLY_REQUEST_STRUCT*/
		pointer p1,

		/* [IN] pointer to the command object*/
		pointer  p2,

		/* [IN] Length of data transmitted */
		uint_32 buffer_length
)
{ /* Body */

	UNUSED(p1)
	UNUSED(p2)
	
	dBuffer_length = buffer_length;
	bCallBack = TRUE;
	bStatus = status;

} /* Endbody */



/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : usb_host_mass_test_storage
* Returned Value : None
* Comments       : Calls the UFI command set for testing
*     
*END*--------------------------------------------------------------------*/
static void usb_host_mass_test_storage
(
		void
)
{ /* Body */	
	static USB_STATUS                                 status     = USB_OK;
	static uint_8  									  bLun 		 = 0;
	static uint_32 									  start_lba  = 0;
	static uint_32 									  lba_length = 0;	
	static FORMAT_UNIT_PARAMETER_BLOCK                formatunit = { 0 };
	static CBW_STRUCT_PTR cbw_ptr; 
	static CSW_STRUCT_PTR csw_ptr;	
	
	switch(test)
	{
		case 0:
			cbw_ptr = pCmd.CBW_PTR;
			csw_ptr = pCmd.CSW_PTR;
	
			memset(buff_in, 0, BUFF_IN_SIZE);
			memset(buff_out, 0xA5, 0x0F);
			memset(pCmd.CSW_PTR, 0, sizeof(CSW_STRUCT));
			memset(pCmd.CBW_PTR, 0, sizeof(CBW_STRUCT));
			memset(&pCmd, 0, sizeof(COMMAND_OBJECT_STRUCT));
			
			pCmd.CBW_PTR  = cbw_ptr;
			pCmd.CSW_PTR  = csw_ptr;
			pCmd.LUN      = bLun;
			pCmd.CALL_PTR = (pointer)&mass_device.class_intf;
			pCmd.CALLBACK = callback_bulk_pipe;
	
			printf("\n\r   ================ START OF A NEW SESSION ================");
			test++;
			ontest = 1;
			break;
			
		case 1:
			/* Test the GET MAX LUN command */
			if (1 == ontest)
			{
				printf("\n\r   Testing: GET MAX LUN Command");
				bCallBack = FALSE;
	
				status = usb_class_mass_getmaxlun_bulkonly(
						(pointer)&mass_device.class_intf, (uint_8_ptr)&bLun,
						usb_host_mass_ctrl_callback);
				ontest = 0;
			} 
			else 
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf (".......................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus)
					{
						printf("..........................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 2:
			/* Test the TEST UNIT READY command */
			if (1 == ontest)
			{
				printf("   Testing: TEST UNIT READY Command");
				bCallBack = FALSE;
	
				status =  usb_mass_ufi_test_unit_ready(&pCmd);
				ontest = 0;
			} 
			else
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf ("...................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus) 
					{
						printf("......................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported(bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 3:
			/* Test the REQUEST SENSE command */
			if (1 == ontest)
			{
				printf("   Testing: REQUEST SENSE Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_request_sense(&pCmd, &req_sense,
						sizeof(REQ_SENSE_DATA_FORMAT));
				ontest = 0;
			} 
			else 
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf (".....................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus) 
					{
						printf("........................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 4:
			/* Test the INQUIRY command */
			if (1 == ontest)
			{
				printf("   Testing: INQUIRY Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_inquiry(&pCmd, (uchar_ptr)&inquiry,
						sizeof(INQUIRY_DATA_FORMAT));
				ontest = 0;
			} 
			else 
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf ("...........................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus) 
					{
						printf("..............................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 5:
			/* Test the REQUEST SENSE command */
			if (1 == ontest)
			{
				printf("   Testing: REQUEST SENSE Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_request_sense(&pCmd, &req_sense,
						sizeof(REQ_SENSE_DATA_FORMAT));
				ontest = 0;
			} 
			else
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf (".....................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus)
					{
						printf("........................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;	
					
		case 6:
			/* Test the READ CAPACITY command */
			if (1 == ontest)
			{
				printf("   Testing: READ CAPACITY Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_read_capacity(&pCmd, (uchar_ptr)&read_capacity,
						sizeof(MASS_STORAGE_READ_CAPACITY_CMD_STRUCT_INFO));
				ontest = 0;
			} 
			else 
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf (".....................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus)
					{
						printf("........................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 7:
			/* Test the REQUEST SENSE command */
			if (1 == ontest)
			{
				printf("   Testing: REQUEST SENSE Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_request_sense(&pCmd, &req_sense,
						sizeof(REQ_SENSE_DATA_FORMAT));
				ontest = 0;
			} 
			else 
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf (".....................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus)
					{
						printf("........................OK\n\r");
						test++;
						ontest = 1;
					}
					else
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 8:
			/* Test the READ FORMAT CAPACITY command */
			if (1 == ontest)
			{
				printf("   Testing: READ FORMAT CAPACITIES Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_format_capacity(&pCmd, (uchar_ptr)&capacity_list,
						sizeof(CAPACITY_LIST));
				ontest = 0;
			} 
			else 
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf ("............ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus)
					{
						printf("...............OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 9:
			/* Test the REQUEST SENSE command */
			if (1 == ontest)
			{
				printf("   Testing: REQUEST SENSE Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_request_sense(&pCmd, &req_sense,
						sizeof(REQ_SENSE_DATA_FORMAT));
				ontest = 0;
			} 
			else
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf (".....................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus) 
					{
						printf("........................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;			

		case 10:
			/* Test the MODE SENSE command */
			if (1 == ontest)
			{
				printf("   Testing: MODE SENSE Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_mode_sense(&pCmd,
						2, //PC
						0x3F, //page code
						buff_in,
						(uint_32)0x08);
				ontest = 0;
			}
			else 
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf ("........................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus) 
					{
						printf("...........................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 11:
			/* Test the PREVENT ALLOW command */
			if (1 == ontest)
			{
				printf("   Testing: PREVENT-ALLOW MEDIUM REMOVAL Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_prevent_allow_medium_removal(
						&pCmd,
						1 // prevent
				);
				ontest = 0;
			} 
			else 
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf ("......ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus)
					{
						printf(".........OK\n");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 12:
			/* Test the REQUEST SENSE command */
			if (1 == ontest)
			{
				printf("   Testing: REQUEST SENSE Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_request_sense(&pCmd, &req_sense,
						sizeof(REQ_SENSE_DATA_FORMAT));
				ontest = 0;
			} 
			else
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf (".....................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus)
					{
						printf("........................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 13:
			/* Test the VERIFY command */
			if (1 == ontest)
			{
				printf("   Testing: VERIFY Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_verify(
						&pCmd,
						0x400, // block address
						1 //length to be verified
				);
				ontest = 0;
			} 
			else 
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf ("............................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus)
					{
						printf("...............................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 14:
			/* Test the WRITE(10) command */
			if (1 == ontest)
			{
				printf("   Testing: WRITE(10) Command");
				bCallBack = FALSE;
	
				/* Logical Block Address (LBA) start address */
				/* It was configured at the half of the maximum LBA number */
				start_lba = (HOST_READ_BEOCT_32(read_capacity.BLLBA) + 1) >> 1;
				/* Length of each LBA */
				lba_length = HOST_READ_BEOCT_32(read_capacity.BLENGTH);
				
				status = usb_mass_ufi_write_10(&pCmd, start_lba, buff_out, lba_length, 1);
				ontest = 0;
			} 
			else
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf (".........................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus) 
					{
						printf("............................OK\n\r");
						test++;
						ontest = 1;
					}
					else
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}					
				}
			}
			break;
			
		case 15:
			/* Test the REQUEST SENSE command */
			if (1 == ontest)
			{
				printf("   Testing: REQUEST SENSE Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_request_sense(&pCmd, &req_sense,
						sizeof(REQ_SENSE_DATA_FORMAT));
				ontest = 0;
			}
			else 
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf (".....................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus) 
					{
						printf("........................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 16:
			/* Test the READ(10) command */
			if (1 == ontest)
			{
				printf("   Testing: READ(10) Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_read_10(&pCmd, start_lba, buff_in, lba_length, 1);
				ontest = 0;
			} 
			else 
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf ("..........................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus) 
					{
						printf(".............................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
			
		case 17:
			/* Test the START-STOP UNIT command */
			if (1 == ontest)
			{
				printf("   Testing: START-STOP UNIT Command");
				bCallBack = FALSE;
	
				status = usb_mass_ufi_start_stop(&pCmd, 0, 1);
				ontest = 0;
			} 
			else
			{
				if ((status != USB_OK) && (status != USB_STATUS_TRANSFER_QUEUED))
				{
					printf ("...................ERROR");
					return;
				}
				else
				{
					/* Wait till command comes back */
					if (!bCallBack) {break;}
					if (!bStatus) 
					{
						printf("......................OK\n\r");
						test++;
						ontest = 1;
					}
					else 
					{
						printf("...Unsupported (bStatus=0x%08x)\n\r", bStatus);
						test++;
						ontest = 1;
					}	
				}
			}
			break;
	
		case 18:
			printf("\n\r   Test done!\n\r");
			test++;
			break;
			
		default:
			break;
	} /* End of switch */

} /* Endbody */

/* EOF */

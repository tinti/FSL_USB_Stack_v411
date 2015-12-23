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
* $FileName: printer.c$
* $Version :
* $Date    :

*
* Comments:
*
*   This file is an example of device drivers for the HID class. This example
*   demonstrates the keyboard functionality. Note that a real keyboard driver also
*   needs to distinguish between intentionally repeated and unrepeated key presses.
*   This example simply demonstrates how to receive data from a USB Keyboard.
*   Interpretation of data is upto the application and customers can add the code
*   for it.
*
*END************************************************************************/
/* INCLUDE *---------------------------------------------------------------*/
#include "types.h"
#include "usb_host_printer.h"
#include "usb_host_hub_sm.h"
#include "usbevent.h"
#include "host_common.h"
#include "printer.h"
#include "host_driver.h"
#include "hidef.h" /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "usb.h"
#include "khci.h"
#include "usb_bsp.h"
#include "sci.h"
#include "rtc.h"
#include "string.h"
#include "poll.h"
#include "usbprv.h"
#include "menu_printer.h"
#include "usb_printer_language_common.h"

/* Global variables *------------------------------------------------------*/
/* Device struct */
volatile DEVICE_STRUCT printer_device = { 0 };
/* printer host handle */
_usb_host_handle host_handle;
/* Device ID */
uchar string_id_buffer[USB_PRINTER_DEVICE_ID_MAX_SIZE] = { 0 };
/* Printer language setup */
LANGUAGE_ID language_id;
boolean b_callback = FALSE;
volatile USB_STATUS bStatus       = USB_OK;
boolean b_language = FALSE;
boolean get_string_ID_complete = FALSE;
void Printer_Task(uchar_ptr);
/* Table of driver capabilities this application wants to use */
static const USB_HOST_DRIVER_INFO DriverInfoTable[] = 
{
    {
        {0x00, 0x00},          /* Vendor ID per USB-IF             */
        {0x00, 0x00},          /* Product ID per manufacturer      */
        USB_CLASS_PRINTER,     /* Class code                       */
        USB_SUBCLASS_PRINTER,  /* Sub-Class code                   */
        USB_PROTOCOL_PRT_UNIDIR, /* Protocol                         */
        0,                     /* Reserved                         */
        usb_host_printer_event /* Application call back function   */
    },
    {
        {0x00, 0x00},          /* Vendor ID per USB-IF             */
        {0x00, 0x00},          /* Product ID per manufacturer      */
        USB_CLASS_PRINTER,     /* Class code                       */
        USB_SUBCLASS_PRINTER,  /* Sub-Class code                   */
        USB_PROTOCOL_PRT_BIDIR, /* Protocol                         */
        0,                     /* Reserved                         */
        usb_host_printer_event /* Application call back function   */
    },
    {
        {0x00, 0x00},          /* Vendor ID per USB-IF             */
        {0x00, 0x00},          /* Product ID per manufacturer      */
        USB_CLASS_PRINTER,         /* Class code                       */
        USB_SUBCLASS_PRINTER,  /* Sub-Class code                   */
        USB_PROTOCOL_PRT_1284, /* Protocol                         */
        0,                     /* Reserved                         */
        usb_host_printer_event /* Application call back function   */
    },
    /* USB 1.1 hub */
    {
        {0x00, 0x00},          /* Vendor ID per USB-IF             */
        {0x00, 0x00},          /* Product ID per manufacturer      */
        USB_CLASS_HUB,         /* Class code                       */
        USB_SUBCLASS_HUB_NONE, /* Sub-Class code                   */
        USB_PROTOCOL_HUB_LS,   /* Protocol                         */
        0,                     /* Reserved                         */
        usb_host_hub_device_event /* Application call back function   */
    },
    {
        {0x00, 0x00},          /* All-zero entry terminates        */
        {0x00, 0x00},          /* driver info list.                */
        0,
        0,
        0,
        0,
        NULL
    },
};

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : main
* Returned Value   :
* Comments         :
*
*
*END*----------------------------------------------------------------------*/
#ifdef __GNUC__
int main(void)
#else
void main(void) 
#endif
{
    USB_STATUS status = USB_OK;
    /* Initialize the current platform. Call for the _bsp_platform_init which is specific to each processor family */
    _bsp_platform_init();
    #ifdef MCU_MK70F12
        sci2_init();
    #else
        sci1_init();
    #endif
    TimerInit();
    
    /* Init polling global variable */
    POLL_init();
    DisableInterrupts;
    
    status = _usb_host_init(HOST_CONTROLLER_NUMBER, /* Use value in header file */
                            MAX_FRAME_SIZE,         /* Frame size per USB spec  */
                            &host_handle);          /* Returned pointer */
    if(status != USB_OK) 
    {
        printf("\nUSB Host Initialization failed! STATUS: 0x%x",(unsigned int) status);
        fflush(stdout);
        exit(1);
    }
    
    /*
    ** Since we are going to act as the host driver, register the driver
    ** information for wanted class/subclass/protocols
    */
    status = _usb_host_driver_info_register(host_handle, (void *)DriverInfoTable);
    if(status != USB_OK) 
    {
        printf("\nUSB Initialization driver info failed! STATUS: 0x%x", status);
        fflush(stdout);      
        exit(1);
    }
    EnableInterrupts;

    printf("\nUSB Printer Host Demo\nWaiting for USB printer to be attached...\n");
    fflush(stdout);
    
    for(;;) 
    {
        Poll();
        Printer_Task(NULL);
        __RESET_WATCHDOG(); /* feeds the dog */
    } /* Loop forever */
    /* Please make sure that you never leave main */
#ifdef __GNUC__
return 0;
#endif
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : Keyboard_Task
* Returned Value : none
* Comments       :
*     Execution starts here
*
*END*--------------------------------------------------------------------*/
void Printer_Task(uchar *buffer)
{
    USB_STATUS status = USB_OK;
    switch (printer_device.DEV_STATE) 
    {
        case USB_DEVICE_IDLE:
            break;
        case USB_DEVICE_ATTACHED:
            printer_device.DEV_STATE = USB_DEVICE_SET_INTERFACE_STARTED;
            status = _usb_hostdev_select_interface(printer_device.DEV_HANDLE, printer_device.INTF_HANDLE, (pointer) & printer_device.CLASS_INTF);
            if(status != USB_OK) 
            {
                printf("\nError in _usb_hostdev_select_interface! STATUS: 0x%x",(unsigned int) status);
                fflush(stdout);
                exit(1);
            }
            break;
        case USB_DEVICE_SET_INTERFACE_STARTED:
            break;
        case USB_DEVICE_INTERFACED:
            /* Get string ID */
            status = usb_printer_get_device_ID((pointer)&printer_device.CLASS_INTF,
                                                usb_host_printer_ctrl_callback, 
                                                NULL, 
                                                USB_PRINTER_DEVICE_ID_MAX_SIZE, 
                                                string_id_buffer);
         	if(status != USB_STATUS_TRANSFER_QUEUED) 
         	{
                  printf("\nError in usb_printer_get_device_ID! STATUS: 0x%x", status);
                  fflush(stdout);
         	}
            break;
        case USB_DEVICE_INUSE:
		    if(FALSE == get_string_ID_complete)
		    {
		        get_string_ID_complete = TRUE;
		        usb_host_show_info_device();
		        b_language = usb_host_get_language_printer();
		    }
		    if(TRUE == b_language)
		    {
		    	/* Main Menu */
		        usb_host_menu_main();
		    }
		    else
		    {
		    	printf("\nThe printer device language is not supported ! \n");
		    	printf("Can't print !\n");
		    	printer_device.DEV_STATE = USB_DEVICE_IDLE;
		    }
            break;
        case USB_DEVICE_DETACHED:
        	get_string_ID_complete = FALSE;
            printf("Going to idle state\n");
            printer_device.DEV_STATE = USB_DEVICE_IDLE;
            break;
        default:
            printf ( "Unknown Printer Device State = %d\n",\
                (int)printer_device.DEV_STATE );
            break;
    }
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : usb_host_hid_keyboard_event
* Returned Value : None
* Comments       :
*     Called when PRINTER device has been attached, detached, etc.
*END*--------------------------------------------------------------------*/
void usb_host_printer_event(
    /* [IN] pointer to device instance */
    _usb_device_instance_handle dev_handle,
    /* [IN] pointer to interface descriptor */
    _usb_interface_descriptor_handle intf_handle,
    /* [IN] code number for event causing callback */
    uint_32 event_code
)
{
    INTERFACE_DESCRIPTOR_PTR intf_ptr = (INTERFACE_DESCRIPTOR_PTR) intf_handle;
    fflush(stdout);
    switch (event_code) 
    {
        case USB_ATTACH_EVENT:
            printf("----- Attach Event -----\n");
            /* Drop through config event for the same processing */
        case USB_CONFIG_EVENT:
            printf("  State = %d\n", printer_device.DEV_STATE);
            printf("  Class = %d\n", intf_ptr->bInterfaceClass);
            printf("  SubClass = %d\n", intf_ptr->bInterfaceSubClass);
            printf("  Protocol = %d\n", intf_ptr->bInterfaceProtocol);
            fflush(stdout);
    
            if(printer_device.DEV_STATE == USB_DEVICE_IDLE)
            {
                printer_device.DEV_HANDLE = dev_handle;
                printer_device.INTF_HANDLE = intf_handle;
                printer_device.DEV_STATE = USB_DEVICE_ATTACHED;
            }
            else 
            {
                printf("Printer device already attached\n");
                fflush(stdout);
            }
            break;
        case USB_INTF_EVENT:
            printf("----- Interfaced Event -----\n");
            printer_device.DEV_STATE = USB_DEVICE_INTERFACED;
            break;
        case USB_DETACH_EVENT:
            /* Use only the interface with desired protocol */
            printf("\n----- Detach Event -----\n");
            printf("  State = %d\n", printer_device.DEV_STATE);
            printf("  Class = %d\n", intf_ptr->bInterfaceClass);
            printf("  SubClass = %d\n", intf_ptr->bInterfaceSubClass);
            printf("  Protocol = %d\n", intf_ptr->bInterfaceProtocol);
            fflush(stdout);
            printer_device.DEV_HANDLE = NULL;
            printer_device.INTF_HANDLE = NULL;
            printer_device.DEV_STATE = USB_DEVICE_DETACHED;
            break;
    }
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : usb_host_hid_ctrl_callback
* Returned Value : None
* Comments       :
*     Called when a control pipe command is completed.
*
*END*--------------------------------------------------------------------*/
void usb_host_printer_ctrl_callback(
    /* [IN] pointer to pipe */
    _usb_pipe_handle pipe_handle,
    /* [IN] user-defined parameter */
    pointer user_parm,
    /* [IN] buffer address */
    uchar_ptr buffer,
    /* [IN] length of data transferred */
    uint_32 buflen,
    /* [IN] status, hopefully USB_OK or USB_DONE */
    uint_32 status)
{
    UNUSED(pipe_handle)
    UNUSED(user_parm)
    UNUSED(buffer)
    UNUSED(buflen)
    
    if (printer_device.DEV_STATE == USB_DEVICE_INTERFACED){
        printer_device.DEV_STATE = USB_DEVICE_INUSE;
    }
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : usb_host_hid_recv_callback
* Returned Value : None
* Comments       :
*     Called when a interrupt pipe transfer is completed.
*
*END*--------------------------------------------------------------------*/
void usb_host_printer_recv_callback(
    /* [IN] pointer to pipe */
    _usb_pipe_handle pipe_handle,
    /* [IN] user-defined parameter */
    pointer user_parm,
    /* [IN] buffer address */
    uchar_ptr buffer,
    /* [IN] length of data transferred */
    uint_32 buflen,
    /* [IN] status, hopefully USB_OK or USB_DONE */
    uint_32 status)
{
    UNUSED(pipe_handle)
    UNUSED(user_parm)
    UNUSED(buffer)
    UNUSED(buflen)
    UNUSED(status)
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : usb_host_hid_send_callback
* Returned Value : None
* Comments       :
*     Called when an interrupt pipe transfer is completed.
*
*END*--------------------------------------------------------------------*/
void usb_host_printer_send_callback(
    /* [IN] pointer to pipe */
    _usb_pipe_handle pipe_handle,
    /* [IN] user-defined parameter */
    pointer user_parm,
    /* [IN] buffer address */
    uchar_ptr buffer,
    /* [IN] length of data transferred */
    uint_32 buflen,
    /* [IN] status, hopefully USB_OK or USB_DONE */
    uint_32 status)
{
    UNUSED(pipe_handle)
    UNUSED(user_parm)
    UNUSED(buflen)
    UNUSED(status)
    UNUSED(buffer);
    b_callback = TRUE;
	bStatus = status;
}


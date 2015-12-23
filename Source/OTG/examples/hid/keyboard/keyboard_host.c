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
* $FileName: hidkeyboard.c$
* $Version : 3.4.26.0$
* $Date    : Sep-17-2009$
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
#include "types.h"
#include "keyboard_host.h"
#include "usb_host_hid.h"
#include "usb_host_hub_sm.h"
#include "usbevent.h"
#include "host_common.h"
#include "hidkeyboard.h"
#include "host_driver.h"
#include "hidef.h" /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "usb.h"
#include "khci.h"
#include "usb_bsp.h"
#include "sci.h"
#include "rtc.h"
#include "string.h"
#include "usb_otg_main.h"
#include "mem_util.h"

#if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
#include "exceptions.h"
#endif

/***************************************
**
** Globals
*/


#define USB_EVENT_CTRL          0x01
#define USB_EVENT_DATA          0x02
#define MAX_SUPPORTED_USAGE_ID  45

extern uint_32             host_stack_active; /* TRUE if the host stack is active */                   
extern uint_32               dev_stack_active;  /* TRUE if the peripheral stack is active */

volatile DEVICE_STRUCT hid_device = { 0 };
_usb_host_handle host_handle;
USB_EVENT_STRUCT USB_Event;
HID_COMMAND hid_com;
_usb_pipe_handle pipe;

uchar_ptr main_buffer;

void Keyboard_Task(uchar *buffer);
void process_kbd_buffer(uchar_ptr buffer);


static const uchar HID_table[MAX_SUPPORTED_USAGE_ID][2] = {
    {0, 0},                     /* Usage ID  0 */
    {0, 0},                     /* Usage ID  1 */
    {0, 0},                     /* Usage ID  2 */
    {0, 0},                     /* Usage ID  3 */
    {0x61, 'A'},                /* Usage ID  4 */
    {'b', 'B'},                 /* Usage ID  5 */
    {'c', 'C'},                 /* Usage ID  6 */
    {'d', 'D'},                 /* Usage ID  7 */
    {'e', 'E'},                 /* Usage ID  8 */
    {'f', 'F'},                 /* Usage ID  9 */
    {'g', 'G'},                 /* Usage ID 10 */
    {'h', 'H'},                 /* Usage ID 11 */
    {'i', 'I'},                 /* Usage ID 12 */
    {'j', 'J'},                 /* Usage ID 13 */
    {'k', 'K'},                 /* Usage ID 14 */
    {'l', 'L'},                 /* Usage ID 15 */
    {'m', 'M'},                 /* Usage ID 16 */
    {'n', 'N'},                 /* Usage ID 17 */
    {'o', 'O'},                 /* Usage ID 18 */
    {'p', 'P'},                 /* Usage ID 19 */
    {'q', 'Q'},                 /* Usage ID 20 */
    {'r', 'R'},                 /* Usage ID 21 */
    {'s', 'S'},                 /* Usage ID 22 */
    {'t', 'T'},                 /* Usage ID 23 */
    {'u', 'U'},                 /* Usage ID 24 */
    {'v', 'V'},                 /* Usage ID 25 */
    {'w', 'W'},                 /* Usage ID 26 */
    {'x', 'X'},                 /* Usage ID 27 */
    {'y', 'Y'},                 /* Usage ID 28 */
    {'z', 'Z'},                 /* Usage ID 29 */
    {'1', '!'},                 /* Usage ID 30 */
    {'2', '@'},                 /* Usage ID 31 */
    {'3', '#'},                 /* Usage ID 32 */
    {'4', '$'},                 /* Usage ID 33 */
    {'5', '%'},                 /* Usage ID 34 */
    {'6', '^'},                 /* Usage ID 35 */
    {'7', '&'},                 /* Usage ID 36 */
    {'8', '*'},                 /* Usage ID 37 */
    {'9', '('},                 /* Usage ID 38 */
    {'0', ')'},                 /* Usage ID 39 */
    {'\n', '\n'},               /* Usage ID 40 */
    {0x1B, 0x1B},               /* Usage ID 41 */
    {0x7F, 0x7F},               /* Usage ID 43 */
    {0x9, 0x9},                 /* Usage ID 42 */
    {0x20, 0x20},               /* Usage ID 44 */

};

/* Table of driver capabilities this application wants to use */
static const USB_HOST_DRIVER_INFO DriverInfoTable[] = {
    {
            {0x00, 0x00},          /* Vendor ID per USB-IF             */
            {0x00, 0x00},          /* Product ID per manufacturer      */
            USB_CLASS_HID,         /* Class code                       */
            USB_SUBCLASS_HID_BOOT, /* Sub-Class code                   */
            USB_PROTOCOL_HID_KEYBOARD, /* Protocol                         */
            0,                     /* Reserved                         */
            usb_host_hid_keyboard_event /* Application call back function   */
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
        NULL},
};

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : App_Host_Init(void)
* Returned Value   :
* Comments         :
*
*
*END*----------------------------------------------------------------------*/
USB_STATUS App_Host_Init(void) 
{
  USB_STATUS status = USB_OK;
  host_stack_active  = TRUE;
  dev_stack_active = FALSE;
  hid_device.DEV_STATE = USB_DEVICE_IDLE;

  DisableInterrupts;
  #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
    usb_int_dis();
  #endif

  main_buffer =(uchar_ptr) USB_mem_alloc_uncached(HID_BUFFER_SIZE);
  if(main_buffer == NULL)
  {
     return USBERR_ALLOC;
  }

  status = _usb_host_init(HOST_CONTROLLER_NUMBER, /* Use value in header file */
        MAX_FRAME_SIZE,         /* Frame size per USB spec  */
       &host_handle);           /* Returned pointer */
#ifdef _MCF51JM128_H
  PTCD &= 0xbf;
#endif
  if(status != USB_OK)
  {
      printf("\n\r   USB Host Initialization failed. STATUS: %x", status);
      fflush(stdout);
      return status;
  }

  /*
   ** since we are going to act as the host driver, register the driver
   ** information for wanted class/subclass/protocols
   */
  status = _usb_host_driver_info_register(host_handle, (void *)DriverInfoTable);
  if(status != USB_OK)
  {
      return status;
  }
  _usb_event_init(&USB_Event);
  EnableInterrupts;
  #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
  usb_int_en();
  #endif

  printf("\n\n\r   USB HID Keyboard Demo\n\r   Waiting for USB Keyboard to be attached...\n\r");
  fflush(stdout);

  return USB_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : App_Host_Shut_Down(void)
* Returned Value   :
* Comments         :
*
*
*END*----------------------------------------------------------------------*/

void App_Host_Shut_Down(void) 
{
  DisableInterrupts;
  #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
    usb_int_dis();
  #endif

  USB_mem_free(main_buffer);
  usb_dev_list_detach_device(host_handle, 0, 0);
  _usb_host_shutdown  ( host_handle);

  EnableInterrupts;
  #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
    usb_int_en();
  #endif

  host_stack_active  = FALSE;
  dev_stack_active = FALSE;
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
  Keyboard_Task(main_buffer);
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : Keyboard_Task
* Returned Value : none
* Comments       :
*     Execution starts here
*
*END*--------------------------------------------------------------------*/
void Keyboard_Task(uchar *buffer)
{
    USB_STATUS status = USB_OK;
    static TR_INIT_PARAM_STRUCT tr;

    switch (hid_device.DEV_STATE)
    {

      case USB_DEVICE_IDLE:
              break;

      case USB_DEVICE_ATTACHED:
              hid_device.DEV_STATE = USB_DEVICE_SET_INTERFACE_STARTED;

              status = _usb_hostdev_select_interface(hid_device.DEV_HANDLE, hid_device.INTF_HANDLE, (pointer) &(hid_device.CLASS_INTF));
              if(status != USB_OK)
              {
                  printf("\n\r   Error in _usb_hostdev_select_interface: %x", status);
                  fflush(stdout);
                  exit(1);
              }
              break;

      case USB_DEVICE_SET_INTERFACE_STARTED:
              break;

      case USB_DEVICE_INTERFACED:
              printf("   Keyboard device interfaced, setting protocol...\n\r");
              /* now we will set the USB Hid standard boot protocol */
              hid_device.DEV_STATE = USB_DEVICE_SETTING_PROTOCOL;

              hid_com.CLASS_PTR = (CLASS_CALL_STRUCT_PTR) & hid_device.CLASS_INTF;
              hid_com.CALLBACK_FN = usb_host_hid_ctrl_callback;
              hid_com.CALLBACK_PARAM = 0;

              /* Force the keyboard to behave as in USB Hid class standard boot protocol */
              status = usb_class_hid_set_protocol(&hid_com, USB_PROTOCOL_HID_KEYBOARD);

              if(status != USB_STATUS_TRANSFER_QUEUED)
              {
                printf("\n\r   Error in usb_class_hid_set_protocol: %x", status);
                fflush(stdout);
              }
              break;

      case USB_DEVICE_INITIALIZED:  			
              pipe = _usb_hostdev_find_pipe_handle(hid_device.DEV_HANDLE, hid_device.INTF_HANDLE, USB_INTERRUPT_PIPE, USB_RECV);

              if(pipe)
              {
            	printf("   Keyboard device ready, try to press the keyboard\n\r ");
  			  }
              hid_device.DEV_STATE = USB_DEVICE_START_RECEIVE;
              break;

      case USB_DEVICE_START_RECEIVE:
         	  /******************************************************************
              Initiate a transfer request on the interrupt pipe
              ******************************************************************/
              _usb_event_clear(&USB_Event, USB_EVENT_CTRL | USB_EVENT_DATA);
         	 		usb_hostdev_tr_init(&tr, usb_host_hid_recv_callback, NULL);
         			
              tr.RX_BUFFER = buffer;
              tr.RX_LENGTH = HID_BUFFER_SIZE;	                  	
                  	
              status = _usb_host_recv_data(host_handle, pipe, &tr);
              if(status != USB_STATUS_TRANSFER_QUEUED)
              {
                printf("\n\r   Error in _usb_host_recv_data: %x", status);
                fflush(stdout);
              }
              else
              {
                hid_device.DEV_STATE = USB_DEVICE_WAIT_RECEIVE_EVENT;
              }
              break;

      case USB_DEVICE_WAIT_RECEIVE_EVENT:
              /* Wait until we get the data from keyboard. */
              if(_usb_event_wait_ticks(&USB_Event,USB_EVENT_CTRL | USB_EVENT_DATA, FALSE, 0) == USB_EVENT_SET)
              {
                  /* if not detached in the meanwhile */
                  //_usb_event_clear(&USB_Event, USB_EVENT_CTRL | USB_EVENT_DATA);

                  process_kbd_buffer((uchar *)buffer);
                  //_usb_event_clear(&USB_Event, USB_EVENT_CTRL | USB_EVENT_DATA);
                  hid_device.DEV_STATE = USB_DEVICE_START_RECEIVE;
              }
              break;

      case USB_DEVICE_DETACHED:
          	  printf("   Going to idle state\n\r");

          	  hid_device.DEV_STATE = USB_DEVICE_IDLE;
          break;
      }
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : usb_host_hid_keyboard_event
* Returned Value : None
* Comments       :
*     Called when HID device has been attached, detached, etc.
*END*--------------------------------------------------------------------*/

void usb_host_hid_keyboard_event(
    /* [IN] pointer to device instance */
    _usb_device_instance_handle dev_handle,
    /* [IN] pointer to interface descriptor */
    _usb_interface_descriptor_handle intf_handle,
    /* [IN] code number for event causing callback */
    uint_32 event_code)
{
    INTERFACE_DESCRIPTOR_PTR intf_ptr = (INTERFACE_DESCRIPTOR_PTR) intf_handle;

    fflush(stdout);
    switch (event_code) 
    {

    case USB_ATTACH_EVENT:
        printf("\n\n\r   ----- Attach Event -----\n\r");
        /* Drop through config event for the same processing */
    case USB_CONFIG_EVENT:
        printf("   State = %d", hid_device.DEV_STATE);
        printf("  Class = %d", intf_ptr->bInterfaceClass);
        printf("  SubClass = %d", intf_ptr->bInterfaceSubClass);
        printf("  Protocol = %d\n", intf_ptr->bInterfaceProtocol);
        fflush(stdout);

        if(hid_device.DEV_STATE == USB_DEVICE_IDLE) 
        {
            hid_device.DEV_HANDLE = dev_handle;
            hid_device.INTF_HANDLE = intf_handle;
            hid_device.DEV_STATE = USB_DEVICE_ATTACHED;
        }
        else 
        {
            printf("   HID device already attached\n\r");
            fflush(stdout);
        }
        break;

    case USB_INTF_EVENT:
        printf("\n\r   ----- Interfaced Event -----\n\r");
        hid_device.DEV_STATE = USB_DEVICE_INTERFACED;
        _usb_otg_on_interface_event(dev_handle);
        break;

    case USB_DETACH_EVENT:
       //debug
#ifdef _MCF51JM128_H
  PTCD &= 0xbf;
#endif
        /* Use only the interface with desired protocol */
        printf("\n\r   ----- Detach Event -----\n\r");
        printf("   State = %d", hid_device.DEV_STATE);
        printf("  Class = %d", intf_ptr->bInterfaceClass);
        printf("  SubClass = %d", intf_ptr->bInterfaceSubClass);
        printf("  Protocol = %d\n\r", intf_ptr->bInterfaceProtocol);
        fflush(stdout);

        hid_device.DEV_HANDLE = NULL;
        hid_device.INTF_HANDLE = NULL;
        hid_device.DEV_STATE = USB_DEVICE_DETACHED;
        _usb_otg_on_detach_event(dev_handle);
        break;
    }

    /* notify application that status has changed */
    _usb_event_set(&USB_Event, USB_EVENT_CTRL);
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : usb_host_hid_ctrl_callback
* Returned Value : None
* Comments       :
*     Called when a control pipe command is completed.
*
*END*--------------------------------------------------------------------*/

void usb_host_hid_ctrl_callback(
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
	
    if(status == USBERR_ENDPOINT_STALLED) 
    {
        printf("\n\r   HID Set_Protocol Request BOOT is not supported!\n\r");
        fflush(stdout);
    }
    else if(status) 
    {
        printf("\n\r   HID Set_Protocol Request BOOT failed!: 0x%x ... END!\n\r", status);
        fflush(stdout);
        exit(1);
    }

    if(hid_device.DEV_STATE == USB_DEVICE_SETTING_PROTOCOL)
        hid_device.DEV_STATE = USB_DEVICE_INITIALIZED;

    /* notify application that status has changed */
    _usb_event_set(&USB_Event, USB_EVENT_CTRL);
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : usb_host_hid_recv_callback
* Returned Value : None
* Comments       :
*     Called when a interrupt pipe transfer is completed.
*
*END*--------------------------------------------------------------------*/

void usb_host_hid_recv_callback(
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
	
    /* notify application that data are available */
    _usb_event_set(&USB_Event, USB_EVENT_DATA);
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : process_kbd_buffer
* Returned Value : None
* Comments       :
*      The way keyboard works is that it sends reports of 8 bytes of data
*      every time keys are pressed. However, it reports all the keys
*      that are pressed in a single report. The following code should
*      really be implemented by a user in the way he would like it to be.
*END*--------------------------------------------------------------------*/

void process_kbd_buffer(uchar_ptr buffer)
{
    /* a little array in which we count how long each key is pressed */
    static uchar special_last;

    uint_32 i, shift = 0;
    uchar code;

    /* The first byte in buffer gives special key status.
     ** Process only the keys which are newly pressed. */
    code = (buffer[0] ^ special_last) & buffer[0];
    special_last = buffer[0];

    shift = 0;
    /* Check Modifiers in byte 0 (see HID specification 1.11 page 56) */
    if(code & 0x01) 
    {
        printf("LEFT CTRL ");
    }
    if(code & 0x02) 
    {
        printf("LEFT SHIFT ");
    }
    if(code & 0x04) 
    {
        printf("LEFT ALT ");
    }
    if(code & 0x08) 
    {
        printf("LEFT GUI ");
    }
    if(code & 0x10) 
    {
        printf("RIGHT CTRL ");
    }
    if(code & 0x20) 
    {
        printf("RIGHT SHIFT ");
    }
    if(code & 0x40) 
    {
        printf("RIGHT ALT ");
    }
    if(code & 0x80) 
    {
        printf("RIGHT GUI ");
    }

    /* immediate state of left or right SHIFT */
    if(buffer[0] & 0x22) 
    {
        shift = 1;
    }

    /* Byte 1 is reserved (HID specification 1.11 page 60) */

    /*
     ** Build initial press-map by checking Keybcodes in bytes 2 to 7
     ** (HID spec 1.11 page 60)
     */
    for(i = HID_BUFFER_SIZE - 1; i >= 2; i--) 
    {
        code = buffer[i];

        /* if valid keyboard code was received */
        if(code > 1) 
        {
            if(code <= MAX_SUPPORTED_USAGE_ID && HID_table[code][shift]) 
            {
               printf("%c",HID_table[code][shift]);
            }
            else 
            {
                /* Print hexadecimal code */
                printf("\\x%02x",code);
            }

            /* only print the newest key */
            break;
        }
    }

    fflush(stdout);
}



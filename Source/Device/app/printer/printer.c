/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
*
**************************************************************************//*!
*
* @file printer.c
*
* @author 
*
* @version 
*
* @date May-08-2009
*
* @brief  
*****************************************************************************/

/******************************************************************************
* Includes
*****************************************************************************/
#include "types.h"          /* User Defined Data Types */
#include "hidef.h"          /* for EnableInterrupts macro */
#include "derivative.h"     /* include peripheral declarations */
#include "usb_printer.h"    /* USB Printer Class Header File */
#include "printer.h"        /* Printer Application Header File */
#include "usb_class.h"
#include "usb_dciapi.h"     

#ifdef __MCF52xxx_H__
#include "Wdt_cfv2.h"
#endif

#if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
#include "exceptions.h"
#endif

/* skip the inclusion in dependency stage */
#ifndef __NO_SETJMP
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>    

/*****************************************************************************
* Constant and Macro's - None
*****************************************************************************/
uint_8 printer_buffer[PRINTER_BULK_OUT_ENDP_PACKET_SIZE]; 
/*****************************************************************************
* Global Functions Prototypes
*****************************************************************************/

/****************************************************************************
* Global Variables
****************************************************************************/
uint_8 printer_device_id[PRINTER_DEVICE_ID_STRING_SIZE] = 
{  
    0x00,0x46,      // Lenght of the Device ID string (MSB/LSB) including the
    'M',            // two length bytes
    'A',
    'N',
    'U',
    'F',
    'A',
    'C',
    'T',
    'U',
    'R',
    'E',
    'R',
    ':',
    'F',
    'R',
    'E',
    'E',
    'S',
    'C',
    'A',
    'L',
    'E',
    ';',
    'M',
    'O',
    'D',
    'E',
    'L',
    ':',
    'F',
    'S',
    'L',
    '-',
    'P',
    'R',
    '0',
    '0',
    '1',
    ';',
    'C',
    'O',
    'M',
    'M',
    'A',
    'N',
    'D',
    ' ',
    'S',
    'E',
    'T',
    ':',
    'P',
    'C',
    'L',
    '5',
    ';',
    'C',
    'L',
    'S',
    ':',
    'P',
    'R',
    'I',
    'N',
    'T',
    'E',
    'R',
    ';'
};
uint_8 printer_port_status = (uint_8)(USB_PRINTER_NOT_ERROR|USB_PRINTER_SELECT&(~USB_PRINTER_PAGE_EMPTY));

/*****************************************************************************
* Local Types - None
*****************************************************************************/

/*****************************************************************************
* Local Functions Prototypes
*****************************************************************************/
static void USB_App_Callback(
    uint_8 controller_ID,
    uint_8 event_type,
    void* val);
static uint_8 USB_App_Param_Callback(
    uint_8 request,
    uint_16 value,
    uint_16 wIndex,		 
    uint_8_ptr* data,
    USB_PACKET_SIZE* size);

/*****************************************************************************
* Local Variables 
*****************************************************************************/

/*****************************************************************************
* Local Functions
*****************************************************************************/
/******************************************************************************
*
*    @name        USB_App_Callback
*
*    @brief       This function handles the callback  
*
*    @param       controller_ID : To Identify the controller
*    @param       event_type : value of the event
*    @param       val : gives the configuration value 
*
*    @return      None
*
*****************************************************************************/
static void USB_App_Callback(uint_8 controller_ID, uint_8 event_type, void* val) 
{
    UNUSED (controller_ID)
    UNUSED (val)    
    switch(event_type)
    {
    case USB_APP_SEND_COMPLETE:
        /* Add code here */
        break;
    case USB_APP_ERROR:
        break;
    case USB_APP_DATA_RECEIVED:
        USB_DCI_Recv_Data(0, PRINTER_BULK_OUT_ENDPOINT, 
                          printer_buffer, PRINTER_BULK_OUT_ENDP_PACKET_SIZE); 
        break;   
    case USB_APP_ENUM_COMPLETE:
        USB_DCI_Recv_Data(0, PRINTER_BULK_OUT_ENDPOINT, 
                          printer_buffer, PRINTER_BULK_OUT_ENDP_PACKET_SIZE);
        break;
    default:
        break;
    }
}

/******************************************************************************
 *
 *    @name        USB_App_Param_Callback
 *
 *    @brief       This function handles callbacks for USB Printer Class request
 *
 *    @param       request  : request type
 *    @param       data     : pointer to the data
 *    @param       size     : size of the transfer
 *
 *    @return      status
 *                  USB_OK  :  if successful
 *                  else return error
 *
 *****************************************************************************
 * This function is called whenever a Printer class request is received. This
 * function handles these class requests
 *****************************************************************************/
static uint_8 USB_App_Param_Callback(
    uint_8 request,        /* [IN] request type */
    uint_16 value,         /* [IN] value */
    uint_16 wIndex,	       /* [IN] interface */
    uint_8_ptr* data,      /* [OUT] pointer to the data */
    USB_PACKET_SIZE* size  /* [OUT] size of the transfer */
)
{
    UNUSED (value)
    UNUSED (wIndex)
    /* handle the class request */
    switch(request)
    {
        case USB_PRINTER_GET_DEVICE_ID :
            *data = printer_device_id;
            *size = PRINTER_DEVICE_ID_STRING_SIZE;
            break;
        case USB_PRINTER_GET_PORT_STATUS :
            *data = &printer_port_status;
            *size = 1;
            break;
        case USB_PRINTER_SOFT_RESET :
            break;
        default :
            break;
    }
    return USB_OK;
}

/******************************************************************************
*  
*   @name        TestApp_Init
* 
*   @brief       This function is the entry for mouse (or other usuage)
* 
*   @param       None
* 
*   @return      None
**                
*****************************************************************************/
void TestApp_Init(void)
{
    uint_8   error;
    
    /* Disable all interrupts */
    DisableInterrupts;
#if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
    usb_int_dis();
#endif

    /* Initialize the USB interface */
    error = USB_Class_Printer_Init(CONTROLLER_ID, USB_App_Callback, NULL, USB_App_Param_Callback);

    /* Enable interrupts */
    EnableInterrupts;

    #if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
    usb_int_en();
#endif
} 

/******************************************************************************
*
*   @name        TestApp_Task
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
void TestApp_Task(void)
{
}

/* EOF */

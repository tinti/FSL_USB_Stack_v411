/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
*
**************************************************************************//*!
*
* @file audio_speaker.c
*
* @author
*
* @version
*
* @date
*
* @brief  The file emulates a audio speaker.
*****************************************************************************/

/******************************************************************************
* Includes
*****************************************************************************/
#include "hidef.h"          /* for EnableInterrupts macro */
#include "derivative.h"     /* include peripheral declarations */
#include "types.h"          /* Contains User Defined Data Types */
#include "usb_audio.h"        /* USB AUDIO Class Header File */
#include "audio_speaker.h"    /* AUDIO Speaker Application Header File */
#include "sci.h"

/* skip the inclusion in dependency statge */
#ifndef __NO_SETJMP
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>

/*****************************************************************************
* Constant and Macro's - None
*****************************************************************************/

/*****************************************************************************
* Global Functions Prototypes
*****************************************************************************/
void Test_Audio_App_Task(void);

/****************************************************************************
* Global Variables
****************************************************************************/
uint_8 audio_sample;
uint_8 audio_event;
uint_8 audio_data_recv[8];

/*****************************************************************************
* Local Types - None
*****************************************************************************/

/*****************************************************************************
* Local Functions Prototypes
*****************************************************************************/


/*****************************************************************************
* Local Variables
*****************************************************************************/
#ifdef _MC9S08JS16_H
#pragma DATA_SEG APP_DATA
#endif
/* Audio speaker Application start Init Flag */
static volatile boolean start_app = FALSE;
/* Receive Buffer */
static uint_8 g_curr_recv_buf[DATA_BUFF_SIZE];

/*****************************************************************************
* Local Functions
*****************************************************************************/

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
void Test_Audio_App_Task(void)
{

}

/******************************************************************************
*
*    @name        USB_App_Callback
*
*    @brief       This function handles Class callback
*
*    @param       controller_ID    : Controller ID
*    @param       event_type       : Value of the event
*    @param       val              : gives the configuration value
*
*    @return      None
*
*****************************************************************************
* This function is called from the class layer whenever reset occurs or enum
* is completed. After the enum is completed this function sets a variable so
* that the application can start.
* This function also receives DATA Send and RECEIVED Events
*****************************************************************************/
void USB_Audio_App_Callback (
uint_8 controller_ID,   /* [IN] Controller ID */
uint_8 event_type,      /* [IN] value of the event */
void* val               /* [IN] gives the configuration value */
)
{
    uint_8 i;
    static APP_DATA_STRUCT* data_receive;

    if(event_type == USB_APP_BUS_RESET)
    {
        start_app=FALSE;
    }
    else if(event_type == USB_APP_ENUM_COMPLETE)
    {
        start_app=TRUE;

        #if (!(defined _MC9S08JS16_H))
		#if USART_DEBUG
        (void)printf("Audio Speaker is working ... \r\n");
		#endif /* USART_DEBUG */
        #endif
    }
    else if ((event_type == USB_APP_DATA_RECEIVED) && (TRUE == start_app))
    {
        (void)USB_Class_Audio_Recv_Data(controller_ID, AUDIO_ENDPOINT,
        g_curr_recv_buf, 8);
        audio_event =  USB_APP_DATA_RECEIVED;
        data_receive = (APP_DATA_STRUCT*)val;
        for(i=0;i<data_receive->data_size;i++)
        {
            audio_data_recv[i]=data_receive->data_ptr[i];
        }
    }
    return;
}
/* EOF */

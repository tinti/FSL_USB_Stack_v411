/**HEADER********************************************************************
* 
* Copyright (c) 2008 Freescale Semiconductor;
* All Rights Reserved
*
* Copyright (c) 1989-2008 ARC International;
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
* $FileName: usb_host_printer_pcl5.c
* $Version :
* $Date    :
*
* Comments:
*
*   This file contains  the USB printer PCL5 language
*
*END************************************************************************/
/*INCLUDE*---------------------------------------------------------------*/
#include "usb_host_printer_pcl5.h"

/* Font name array */
const USB_PRINTER_PCL5_FONT_STRUCT PCL5_FONT_NAME[USB_PRINTER_PCL5_MAX_FONT] = 
{
    {AVANT_GARDE_FONT, Ec"(s24607T"},
    {BOOKMAN_FONT, Ec"(s24623T"},
    {COURIER_FONT, Ec"(s3T"},
    {TIMES_FONT, Ec"(s25093T"},
    {PALATINO_FONT, Ec"(s24591T"},
    {NEW_CENTURY_SCHOOLBOOK_FONT, Ec"(s25093T"},
    {HELVETICA_FONT, Ec"(s24580T"},
    {HELVETICA_NARROW_FONT, Ec"(s24580T"}
};
static boolean underline_font;
tr_callback g_printer_plc5_write_callback = NULL;
/* Static function */
static USB_STATUS _usb_host_printer_pcl5_write_command(
    CLASS_CALL_STRUCT_PTR       cc_ptr,
    uchar_ptr                   command
);
static void usb_host_printer_pcl5_write_callback(
    _usb_pipe_handle            pipe_handle,
    pointer                     user_parm,
    uchar_ptr                   buffer,
    uint_32                     buflen,
    uint_32                     status
);

/*Globle functions*------------------------------------------------------*/
/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _usb_host_printer_pcl5_write
* Returned Value : USB_OK or error code
* Comments       :
*
*END*--------------------------------------------------------------------*/
USB_STATUS _usb_host_printer_pcl5_write(
    CLASS_CALL_STRUCT_PTR       cc_ptr,
    tr_callback                 callback,
    USB_PRINTER_COMMAND         command,
    uint_32                     buff_size,
    void*                       buffer_ptr
)
{
    /* Register the pcl5 write callback */
    g_printer_plc5_write_callback = callback;
    switch(command)
    {
        case USB_PRINTER_COMMAND_SET_POSITION:
        {
            if(NULL != buffer_ptr)
            {
                char * buffer = NULL;
                USB_PRINER_POSITION_STRUCT_PTR position_cursor_ptr = NULL;
                /* Prepare set position buffer */
                buffer = (char *)USB_mem_alloc_zero(USB_PRINTER_PCL5_POSITION_MAX_BUFF);
                if(NULL == buffer)
                {
                    return USB_PRINTER_LANGUAGE_ERROR_OUT_OF_MEMORY;
                }
                position_cursor_ptr = (USB_PRINER_POSITION_STRUCT_PTR)buffer_ptr;
                /* Combine set position buffer */
                sprintf(buffer, USB_PRINTER_PCL5_CURSOR_HORIZONTAL_POSITION_H
                    USB_PRINTER_PCL5_CURSOR_VERTICAL_POSITION_V,
                    position_cursor_ptr->position_x,
                    position_cursor_ptr->position_y);
                /* Send set position command */
                return usb_printer_send_data(cc_ptr,
                    usb_host_printer_pcl5_write_callback,
                    NULL,strlen(buffer),(uchar_ptr)buffer);
            }
            break;
        }
        /* Set orientation */
        case USB_PRINTER_COMMAND_ORIENTATION:
        {
            if(NULL != buffer_ptr)
            {        
                char* buffer = NULL;
                buffer = (char*)USB_mem_alloc_zero(USB_PRINTER_PCL5_ORIENTATION_MAX_BUFF);
                if (NULL == buffer)
                {
                    return USB_PRINTER_LANGUAGE_ERROR_OUT_OF_MEMORY;
                }
                /* Page orientation: Landscape */
                if((boolean)(*(unsigned char*)buffer_ptr))
                {
                    sprintf(buffer,USB_PRINTER_PCL5_PAGE_LOGICAL_ORIENTATION,1);
                }
                /* Page orientation: Portrait */
                else
                {
                    sprintf(buffer,USB_PRINTER_PCL5_PAGE_LOGICAL_ORIENTATION,0);
                }
                /* Send page orientation command */
                return usb_printer_send_data(cc_ptr,
                       usb_host_printer_pcl5_write_callback,
                       NULL,strlen(buffer),(uchar_ptr)buffer);
            }
            break;
        }
        /* Set font */
        case USB_PRINTER_COMMAND_FONT:
        {
            if(NULL != buffer_ptr)
            {
                uint_8 font_index = 0;
                USB_PRINTER_FONT_STRUCT_PTR font_setup_ptr = NULL;
                char* buffer = (char*)USB_mem_alloc_zero(USB_PRINTER_PCL5_FONT_MAX_BUFF);
                if(NULL == buffer)
                {
                    return USB_PRINTER_LANGUAGE_ERROR_OUT_OF_MEMORY;
                }
                font_setup_ptr = (USB_PRINTER_FONT_STRUCT_PTR)buffer_ptr;
                /* Search font type */
                for(font_index = 0 ; font_index < USB_PRINTER_PCL5_MAX_FONT ; font_index++)
                {
                    if(font_setup_ptr->font_ID == PCL5_FONT_NAME[font_index].font_id)
                    {
                        strcpy(buffer,PCL5_FONT_NAME[font_index].font_name);
                        break;
                    }
                }
                /* Invalid font type */
                if(USB_PRINTER_PCL5_MAX_FONT == font_index)
                {
                    return USB_PRINTER_LANGUAGE_ERROR_INVALID_FONT_TYPE;
                }
                strcat(buffer,USB_PRINTER_PCL5_FONT_ASCII);
                /* Set font size */
                if(font_setup_ptr->font_size > 0)
                {
                    /* Courier font */
                    if(font_setup_ptr->font_ID == COURIER_FONT)
                    {
                        sprintf(&buffer[strlen(buffer)],USB_PRINTER_PCL5_FONT_SPACING_P,0);
                        strcat(buffer,Ec "(s");
                        sprintf(&buffer[strlen(buffer)],"%d",120/font_setup_ptr->font_size);
                        strcat(buffer,"H");
                    }
                    /* Other fonts */ 
                    else
                    {
                        sprintf(&buffer[strlen(buffer)],USB_PRINTER_PCL5_FONT_SPACING_P,1);
                        strcat(buffer,Ec "(s");
                        sprintf(&buffer[strlen(buffer)],"%d",font_setup_ptr->font_size);
                        strcat(buffer,"V");
                    }
                }
                else
                {
                    return USB_PRINTER_LANGUAGE_ERROR_INVALID_FONT_SIZE;
                }
            
                /* Font style: Italic */
                if(font_setup_ptr->Font_style.u.Italic)
                {
                    sprintf(&buffer[strlen(buffer)],USB_PRINTER_PCL5_FONT_STYLE_P,1);
                }
            
                /* Font style: Bold */
                if(font_setup_ptr->Font_style.u.Bold)
                {
                    sprintf(&buffer[strlen(buffer)],USB_PRINTER_PCL5_FONT_STROKE_WEIGHT_P,3);
                }
                /* Font style: Underline */
                if(font_setup_ptr->Font_style.u.UnderLine)
                {
                    underline_font = TRUE;
                } 
                else 
                {
                    underline_font = FALSE;
                }
                /* Send set font command */
                return usb_printer_send_data(cc_ptr, 
                    usb_host_printer_pcl5_write_callback,
                    NULL,strlen(buffer),(uchar_ptr)buffer);
            }
            break;
        }
        /* Start text */
        case USB_PRINTER_COMMAND_TEXT_START:
        {
            char* buffer = (char*)USB_mem_alloc_zero(10);
            if (NULL == buffer)
            {
                return USB_PRINTER_LANGUAGE_ERROR_OUT_OF_MEMORY;
            }
            /* Font style: Underline */
            if(TRUE == underline_font)
            {
                sprintf(buffer,USB_PRINTER_PCL5_FONT_UNDERLINE USB_PRINTER_PCL5_TEXT_START,2);          
            } else
            {
                 sprintf(buffer,USB_PRINTER_PCL5_TEXT_START);
            }
            
                return usb_printer_send_data(cc_ptr, 
                    usb_host_printer_pcl5_write_callback,
                    NULL,strlen(buffer),(uchar_ptr)buffer);
        }   
        /* Print text*/            
        case USB_PRINTER_COMMAND_PRINT_TEXT:
        {
            if(NULL != buffer_ptr)
            {
                char* buffer = (char*)USB_mem_alloc_zero(buff_size);
                strcpy(buffer,buffer_ptr);
                return usb_printer_send_data(cc_ptr,
                    usb_host_printer_pcl5_write_callback,
                    NULL,strlen(buffer),(uchar_ptr)buffer);
            }
            break;
        }
        /* Stop text */
        case USB_PRINTER_COMMAND_TEXT_STOP:
        /* Eject page */
        case USB_PRINTER_COMMAND_EJECT_PAGE:
        {
            return _usb_host_printer_pcl5_write_command(cc_ptr,
                (uchar_ptr)USB_PRINTER_PCL5_JOB_RESET USB_PRINTER_PCL5_JOB_UEL);
        }
        default:
            break;
    }
    return USB_PRINTER_LANGUAGE_ERROR_INVALID_PARAM;
}

/*Static functions*------------------------------------------------------*/
/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _usb_host_printer_pcl5_command
* Returned Value : USB_OK or error code
* Comments       : 
*
*END*--------------------------------------------------------------------*/
static USB_STATUS _usb_host_printer_pcl5_write_command(
    CLASS_CALL_STRUCT_PTR              cc_ptr,
    uchar_ptr                          command
)
{
    char_ptr buffer = NULL;
    buffer = (char_ptr)USB_mem_alloc_zero(strlen((char *)command));
    if(buffer == NULL)
    {
        return USB_PRINTER_LANGUAGE_ERROR_OUT_OF_MEMORY;
    }
    sprintf(buffer,(char *)command);
    return usb_printer_send_data(cc_ptr,
        usb_host_printer_pcl5_write_callback,
        NULL,strlen(buffer),(uchar_ptr)buffer);
}

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : usb_host_printer_pcl5_write_callback
* Returned Value : None
* Comments       : 
*       Called when an interrupt pipe transfer is completed.
*END*--------------------------------------------------------------------*/
static void usb_host_printer_pcl5_write_callback(
    _usb_pipe_handle pipe_handle,
    pointer             user_parm,
    uchar_ptr           buffer,
    uint_32             buflen,
    uint_32             status
)
{
    if(NULL != g_printer_plc5_write_callback)
    {
        g_printer_plc5_write_callback(pipe_handle,user_parm,buffer,buflen,status);
    }
    if(NULL != buffer)
        USB_mem_free(buffer);
}
/* EOF */

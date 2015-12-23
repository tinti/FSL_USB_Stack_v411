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
* $FileName: usb_host_printer_postscript.c
* $Version : 
* $Date    : 
*
* Comments:
*
*   
*
*END************************************************************************/
/*INCLUDE*---------------------------------------------------------------*/
#include "usb_host_printer_postscript.h"

/* Font name array */
const USB_PRINTER_POSTSCRIPT_FONT_STRUCT POSTSCRIPT_FONT_NAME[USB_PRINTER_POSTSCRIPT_MAX_FONT] = 
{
    {AVANT_GARDE_FONT,            {"/AvantGarde-Book", "/AvantGarde-Demi", "/AvantGarde-Oblique", "/AvantGarde-DemiOblique",NULL,NULL,NULL,NULL}}, 
    {BOOKMAN_FONT,                {"/Bookman-Light", "/Bookman-Demi", "/Bookman-LightItalic", "/Bookman-DemiItalic",NULL,NULL,NULL,NULL}},
    {COURIER_FONT,                {"/Courier", "/Courier-Bold", "/Courier-Oblique", "/Courier-BoldOblique",NULL,NULL,NULL,NULL}},
    {HELVETICA_FONT,              {"/Helvetica", "/Helvetica-Bold", "/Helvetica-Oblique", "/Helvetica-BoldOblique",NULL,NULL,NULL,NULL}},
    {HELVETICA_NARROW_FONT,       {"/Helvetica-Narrow", "/Helvetica-Narrow-Bold", "/Helvetica-Narrow-Oblique", "/Helvetica-Narrow-BoldOblique",NULL,NULL,NULL,NULL}},
    {NEW_CENTURY_SCHOOLBOOK_FONT, {"/NewCenturySchlbk-Roman", "/NewCenturySchlbk-Bold", "/NewCenturySchlbk-Italic", "/NewCenturySchlbk-BoldItalic",NULL,NULL,NULL,NULL}},
    {PALATINO_FONT,               {"/Palatino-Roman", "/Palatino-Bold", "/Palatino-Italic", "/Palatino-BoldItalic",NULL,NULL,NULL,NULL}},
    {TIMES_FONT,                  {"/Times-Roman", "/Times-Bold", "/Times-Italic", "/Times-BoldItalic",NULL,NULL,NULL,NULL}} 
};
/* Pointer to postscript write callback */
tr_callback g_printer_postscript_write_callback = NULL;
/* Static function */
static USB_STATUS _usb_host_printer_postscript_write_command(
    CLASS_CALL_STRUCT_PTR       cc_ptr,
    uchar_ptr                   command
);
static void usb_host_printer_postscript_write_callback(
    _usb_pipe_handle            pipe_handle,
    pointer                     user_parm,
    uchar_ptr                   buffer,
    uint_32                     buflen,
    uint_32                     status
);

/*Globle functions*------------------------------------------------------*/
/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _usb_host_printer_postscript_write
* Returned Value : USB_OK or error code
* Comments       : 
*
*END*--------------------------------------------------------------------*/
USB_STATUS _usb_host_printer_postscript_write(
    CLASS_CALL_STRUCT_PTR       cc_ptr,
    tr_callback                 callback,
    USB_PRINTER_COMMAND         command,
    uint_32                     buff_size,
    void*                       buffer_ptr
)
{
    /* Register the pcl5 write callback */
    g_printer_postscript_write_callback = callback;
    switch(command)
    {
        /* Set position */
        case USB_PRINTER_COMMAND_SET_POSITION:
        {
            if(NULL != buffer_ptr)
            {
                char* buffer = NULL;
                USB_PRINER_POSITION_STRUCT_PTR position_cursor_ptr = NULL;
                buffer = (char *)USB_mem_alloc_zero(USB_PRINTER_POSTSCRIPT_MOVETO_MAX_BUFF);
                if (NULL == buffer)
                {
                    return USB_PRINTER_LANGUAGE_ERROR_OUT_OF_MEMORY;
                }
                /* Get position */
                position_cursor_ptr = (USB_PRINER_POSITION_STRUCT_PTR)buffer_ptr;
                sprintf(buffer,USB_PRINTER_POSTSCRIPT_PATH_NEW 
                    USB_PRINTER_POSTSCRIPT_MOVETO,
                    position_cursor_ptr->position_x, 
                    position_cursor_ptr->position_y);
                /* Send set position command */
                return usb_printer_send_data(cc_ptr,
                    usb_host_printer_postscript_write_callback,
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
                buffer = (char*)USB_mem_alloc_zero(USB_PRINTER_POSTSCRIPT_ORIENTATION_MAX_BUFF);
                if (NULL == buffer)
                {
                    return USB_PRINTER_LANGUAGE_ERROR_OUT_OF_MEMORY;
                }
                if((boolean)(*(unsigned char*)buffer_ptr))
                {
                    /* Page Orientation: Landscape */
                    sprintf(buffer,USB_PRINTER_POSTSCRIPT_ORIENTATION,90);
                }
                else
                {
                    /* Page Orientation: Portrait */
                    sprintf(buffer,USB_PRINTER_POSTSCRIPT_ORIENTATION,0);
                }
            return usb_printer_send_data(cc_ptr,
                   usb_host_printer_postscript_write_callback,
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
                char* buffer = NULL;
                USB_PRINTER_FONT_STRUCT_PTR font_setup_ptr = NULL;
                /* Prepare font buffer */
                buffer = (char*)USB_mem_alloc_zero(USB_PRINTER_POSTSCRIPT_FONT_MAX_BUFF);
                if (NULL == buffer)
                {
                    return USB_PRINTER_LANGUAGE_ERROR_OUT_OF_MEMORY;
                }
                /* Get font setup */
                font_setup_ptr = (USB_PRINTER_FONT_STRUCT_PTR)buffer_ptr;
                /* Set font name and font style */
                for(font_index = 0 ; font_index < USB_PRINTER_POSTSCRIPT_MAX_FONT ; font_index++)
                {
                    /* Set font type and font style */
                    if (font_setup_ptr->font_ID == POSTSCRIPT_FONT_NAME[font_index].font_id)
                    {
                        if(NULL != POSTSCRIPT_FONT_NAME[font_index].font_name[font_setup_ptr->Font_style.value])
                        {
                            sprintf(buffer, USB_PRINTER_POSTSCRIPT_FONT_FIND, 
                                POSTSCRIPT_FONT_NAME[font_index].font_name[font_setup_ptr->Font_style.value]);
                            break;
                        }
                        else
                        {
                            return USB_PRINTER_LANGUAGE_ERROR_INVALID_FONT_STYLE;
                        }
                    }
                }
                /* Invalid font type */
                if(USB_PRINTER_POSTSCRIPT_MAX_FONT == font_index)
                {
                    return USB_PRINTER_LANGUAGE_ERROR_INVALID_FONT_TYPE;
                }
                /* Set font size */
                if(font_setup_ptr->font_size > 0)
                {
                    sprintf(&buffer[strlen(buffer)], 
                        USB_PRINTER_POSTSCRIPT_FONT_SCALE 
                        USB_PRINTER_POSTSCRIPT_FONT_SET, 
                        font_setup_ptr->font_size);
                }
                else
                {
                    return USB_PRINTER_LANGUAGE_ERROR_INVALID_FONT_SIZE;
                }
                return usb_printer_send_data(cc_ptr,
                    usb_host_printer_postscript_write_callback,
                    NULL,strlen(buffer),(uchar_ptr)buffer);
            }
            break;
        }

        case USB_PRINTER_COMMAND_TEXT_START:
        {
            return _usb_host_printer_postscript_write_command(cc_ptr,
                (uchar_ptr)USB_PRINTER_POSTSCRIPT_TEXT_START);
        }
        /* Print text */
        case USB_PRINTER_COMMAND_PRINT_TEXT:
        {
            if(NULL != buffer_ptr)
            {
                char* buffer = NULL;
                buffer = (char*)USB_mem_alloc_zero(buff_size);
                if (NULL == buffer)
                {
                    return USB_PRINTER_LANGUAGE_ERROR_OUT_OF_MEMORY;
                }
                sprintf(buffer,"%s",(uchar_ptr)buffer_ptr);
                return usb_printer_send_data(cc_ptr,
                    usb_host_printer_postscript_write_callback,
                    NULL,strlen(buffer),(uchar_ptr)buffer);
            }
            break;
        }

        case USB_PRINTER_COMMAND_TEXT_STOP:
        {
            return _usb_host_printer_postscript_write_command(cc_ptr,
                (uchar_ptr)USB_PRINTER_POSTSCRIPT_TEXT_STOP);
        }
        /* Clear all setting of printer device */
        case USB_PRINTER_COMMAND_EJECT_PAGE:
        {
            return _usb_host_printer_postscript_write_command(cc_ptr,
                (uchar_ptr)USB_PRINTER_POSTSCRIPT_EJECT_PAGE);
        }
        default:
            break;
    }
    return USB_PRINTER_LANGUAGE_ERROR_INVALID_PARAM;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : usb_host_printer_postscript_write_static_command
*  Returned Value :
*  Comments       :
*
*END*------------------------------------------------------------------*/
static USB_STATUS _usb_host_printer_postscript_write_command(
    CLASS_CALL_STRUCT_PTR       cc_ptr,
    uchar_ptr                   command
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
        usb_host_printer_postscript_write_callback,
        NULL,strlen((void *)buffer),(uchar_ptr)buffer);
}

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : usb_host_printer_postscript_write_callback
* Returned Value : None
* Comments       : 
*       Called when an interrupt pipe transfer is completed.
*END*--------------------------------------------------------------------*/
static void usb_host_printer_postscript_write_callback(
    _usb_pipe_handle            pipe_handle,
    pointer                     user_parm,
    uchar_ptr                   buffer,
    uint_32                     buflen,
    uint_32                     status
)
{
    if(NULL != g_printer_postscript_write_callback)
    {
        g_printer_postscript_write_callback(pipe_handle,user_parm,buffer,buflen,status);
    }
    if(NULL != buffer)
        USB_mem_free(buffer);
}
/* EOF */

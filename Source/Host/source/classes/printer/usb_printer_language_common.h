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
* $FileName: usb_printer_language_common.h$
* $Version : 
* $Date    : 
*
* Comments:
*
*   This file describes common data structure and common function prototytes are used to select printer language.
*
*END************************************************************************/
#ifndef _usb_printer_language_common_h_
#define _usb_printer_language_common_h_
#include "usb_host_printer.h"

/* Define Error */
#define USB_PRINTER_LANGUAGE_ERROR_OUT_OF_MEMORY        (0x01)
#define USB_PRINTER_LANGUAGE_ERROR_INVALID_FONT_TYPE    (0x02)
#define USB_PRINTER_LANGUAGE_ERROR_INVALID_PARAM        (0x03)
#define USB_PRINTER_LANGUAGE_ERROR_INVALID_FONT_SIZE    (0x04)
#define USB_PRINTER_LANGUAGE_ERROR_INVALID_FONT_STYLE   (0x05)


typedef enum
{
    USB_PRINTER_LANGUAGE_PCL5,
    USB_PRINTER_LANGUAGE_POSTSCRIPT,
    USB_PRINTER_LANGUAGE_UNKNOWN,
}LANGUAGE_ID;

typedef enum
{
    USB_PRINTER_COMMAND_SET_POSITION,
    USB_PRINTER_COMMAND_TEXT_START,
    USB_PRINTER_COMMAND_PRINT_TEXT,
    USB_PRINTER_COMMAND_TEXT_STOP,
    USB_PRINTER_COMMAND_ORIENTATION,
    USB_PRINTER_COMMAND_FONT,
    USB_PRINTER_COMMAND_EJECT_PAGE, 
}USB_PRINTER_COMMAND;

/* Font ID */
typedef enum
{
    AVANT_GARDE_FONT, 
    BOOKMAN_FONT,
    COURIER_FONT, 
    HELVETICA_FONT,
    HELVETICA_NARROW_FONT,
    NEW_CENTURY_SCHOOLBOOK_FONT,
    PALATINO_FONT, 
    TIMES_FONT, 
}USB_PRINTER_FONT_ID;

/* Position of cursor */
typedef struct _usb_printer_position_struct
{
    uint_16 position_x;
    uint_16 position_y;
}USB_PRINER_POSITION_STRUCT, _PTR_ USB_PRINER_POSITION_STRUCT_PTR;

/* Font setting */
typedef struct _usb_printer_font_struct
{
    uint_8 font_size;
    USB_PRINTER_FONT_ID font_ID;
    union
    {
        uint_8 value;
        struct
        {
            uint_8 Bold      : 1;
            uint_8 Italic    : 1;
            uint_8 UnderLine : 1;
        }u;
    }Font_style;
}USB_PRINTER_FONT_STRUCT, _PTR_ USB_PRINTER_FONT_STRUCT_PTR;

/* Pointer function */
typedef USB_STATUS (_CODE_PTR_ USB_PRINTER_LANGUAGE_FUNC)(
    CLASS_CALL_STRUCT_PTR              cc_ptr,
    tr_callback                        callback,
    USB_PRINTER_COMMAND                command,
    uint_32                            buff_size,
    void*                              buffer_ptr
);

/* Init language */
USB_PRINTER_LANGUAGE_FUNC _usb_host_printer_language_init(
    LANGUAGE_ID language_id
);
#endif
/* EOF */

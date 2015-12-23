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
* $FileName: usb_host_printer_postscript.h
* $Version : 
* $Date    : 
*
* Comments:
*
*   This file contains  the USB printer postscript language
*
*END************************************************************************/
/*INCLUDE*---------------------------------------------------------------*/
#ifndef _usb_printer_language_postscript_h_
#define _usb_printer_language_postscript_h_
#include "usb_printer_language_common.h"
#include <string.h>

/*DEFINITIONS*-----------------------------------------------------------*/
/* Postscript commands */

/* Path Construction Operators */
/* Initialize current path to be empty */
#define USB_PRINTER_POSTSCRIPT_PATH_NEW                             "newpath\r\n"
/* Connect subpath back to its starting point */
#define USB_PRINTER_POSTSCRIPT_PATH_CLOSE                           "closepath\r"

/* Cursor positions definitions */ 
/* Set current point to (x, y) */
#define USB_PRINTER_POSTSCRIPT_MOVETO                               "%d %d moveto\r\n"
/* Perform relative moveto */
#define USB_PRINTER_POSTSCRIPT_RMOVETO                              "%d %d rmoveto\r"
/* Append straight line to (x, y) */
#define USB_PRINTER_POSTSCRIPT_LINETO                               "%d %d lineto\r"
/* Perform relative lineto */
#define USB_PRINTER_POSTSCRIPT_RLINETO                              "%d %d rlineto\r"

/* Device Setup and Output Operators */
#define USB_PRINTER_POSTSCRIPT_END_PAGE                             "stroke\r\nshowpage\r"
/* Eject page */
#define USB_PRINTER_POSTSCRIPT_EJECT_PAGE                           "showpage " "\033" "%%-12345X"

/* Glyph and Font Operators */
#define USB_PRINTER_POSTSCRIPT_MAX_FONT                             (8)
/* Set font or CIDFont in graphics state */
#define USB_PRINTER_POSTSCRIPT_FONT_SET                             "setfont\r\n"
/* Scale font|cidfont by scale to produce font'|cidfont' */
#define USB_PRINTER_POSTSCRIPT_FONT_SCALE                           "%d scalefont\r"
/* Return Font resource instance identified by key */
#define USB_PRINTER_POSTSCRIPT_FONT_FIND                            "%s findfont\r"

/* Literal Text Strings */
/* Text start */
#define USB_PRINTER_POSTSCRIPT_TEXT_START                           "("
/* Text stop */
#define USB_PRINTER_POSTSCRIPT_TEXT_STOP                            ") show\r"
/* Slash */
#define USB_PRINTER_POSTSCRIPT_SLASH                                "/"

/* Page setup */
#define USB_PRINTER_POSTSCRIPT_ORIENTATION                          "%d rotate "

/* Max Position Buffer */
#define USB_PRINTER_POSTSCRIPT_MOVETO_MAX_BUFF                      (33)

/* Max Page Orientation Buffer */
#define USB_PRINTER_POSTSCRIPT_ORIENTATION_MAX_BUFF                 (9)

/* Max Font Buffer */
#define USB_PRINTER_POSTSCRIPT_FONT_MAX_BUFF                        (55)

/* Struct font name */
typedef struct _postscript_font_struct_{
    USB_PRINTER_FONT_ID font_id;
    char* font_name[8];
} USB_PRINTER_POSTSCRIPT_FONT_STRUCT, _PTR_ USB_PRINTER_POSTSCRIPT_FONT_STRUCT_PTR;

/* Public functions */
USB_STATUS _usb_host_printer_postscript_write(
      CLASS_CALL_STRUCT_PTR              cc_ptr,
      tr_callback                        callback,
      USB_PRINTER_COMMAND                command,
      uint_32                            buff_size,
      void*                              buffer_ptr
);
#endif
/* EOF */

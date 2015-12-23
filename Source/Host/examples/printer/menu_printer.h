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
 * $FileName: menu_printer.h$
 * $Version :
 * $Date    :
 *
 * Comments:
 *
 *   This file describes datas and function prototytes are used to select printing options.
 *
 *END************************************************************************/
#ifndef _MENU_PRINTER_H_
#define _MENU_PRINTER_H_
#include "psptypes.h"
#include <stdlib.h>
#include <string.h>
#include "derivative.h" /* include peripheral declarations */
#include "sci.h"
#include "usb_printer_language_common.h"
#include "printer.h"

#define APP_PRINTER_DEVICE_DETACHED               0
/* Macro use in main menu */
#define APP_SELECT_PRINTER_LANGUAGE_MENU          '1'
#define APP_SELECT_POSITION_MENU                  '2'
#define APP_SELECT_FONT_MENU                      '3'
#define APP_SELECT_PAGE_ORIENTATION_MENU          '4'
#define APP_SELECT_STRING_MENU                    '5'
#define APP_SELECT_PRINT_STRING                   '6'

/* Macro use in position menu */
#define APP_SET_POSITION_X                        '1'
#define APP_SET_POSITION_Y                        '2'
#define APP_SET_POSITION_BACK                     '3'

/* Macro use in font menu */
#define APP_FONT_TYPE                             '1'
#define APP_FONT_STYLE                            '2'
#define APP_FONT_SIZE                             '3'
#define APP_FONT_BACK                             '4'

/* Macro use in font type menu */
#define APP_FONT_TYPE_TIMES                       '1'
#define APP_FONT_TYPE_BOOKMAN                     '2'
#define APP_FONT_TYPE_COURIER                     '3'
#define APP_FONT_TYPE_MENU_BACK                   '4'

/* Macro use in font style menu */
#define APP_FONT_STYLE_BOLD                       '1'
#define APP_FONT_STYLE_ITALIC                     '2'
#define APP_FONT_STYLE_UNDERLINE                  '3'
#define APP_FONT_STYLE_MENU_BACK                  '4'

/* Macro use in page orientation menu */
#define APP_PAGE_LANDSCAPE                        '1'
#define APP_PAGE_PORTRAIT                         '2'
#define APP_PAGE_MENU_BACK                        '3'

/* Macro max value */
#define APP_MAX_SIZE_FONT                          4
#define APP_MAX_SIZE_NUMBER                        5
#define APP_MAX_SIZE_POSITION                      5
#define APP_MAX_SIZE_STRING_PRINT                  100
#define APP_MAX_SIZE_STRING_LANGUAGE               30
#define APP_MAX_NUMBER_LANGUAGE_DEVICE             6
/* Printer setup */
typedef struct _usb_printer_setup{
    /* Cursor */
    USB_PRINER_POSITION_STRUCT      current_position;
    /* Font */
    USB_PRINTER_FONT_STRUCT         font_setup;
    /* Page orientation: True = Landscape, False = Portrait */  
    boolean                         page_orientation;
}USB_PRINTER_HOST_SETUP, _PTR_ PTR_USB_PRINTER_HOST_SETUP;

/* Function to show main menu */
void usb_host_menu_main(void);
/* Function to display information of printer device */
void usb_host_show_info_device(void);
/* Get printer language */
boolean usb_host_get_language_printer(void);
#endif
/* EOF */

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
* $FileName: usb_host_printer_pcl5.h
* $Version : 
* $Date    : 
*
* Comments:
*
*   This file contains  all macro and global function of pcl5 language
*
*END************************************************************************/
/*INCLUDE*---------------------------------------------------------------*/
#ifndef _usb_host_printer_pcl5_h_
#define _usb_host_printer_pcl5_h_

#include "usb_printer_language_common.h"
#include <string.h>
/*PCL5 Commands definitions*---------------------------------------------*/
/* Escape */
#define Ec                                                  "\033"

/* PCL Job control commands */
#define USB_PRINTER_PCL5_JOB_RESET                          Ec "E"
#define USB_PRINTER_PCL5_JOB_UEL                            Ec "%%-12345X"
#define USB_PRINTER_PCL5_JOB_NUMBER_OF_COPY                 Ec "&l%dX"
#define USB_PRINTER_PCL5_JOB_SIMPLEX_DUPLEX                 Ec "&l%dS"
#define USB_PRINTER_PCL5_JOB_LEFT_OFFSET_REG                Ec "&l%dU"
#define USB_PRINTER_PCL5_JOB_TOP_OFFSET_REG                 Ec "&l%dZ"
#define USB_PRINTER_PCL5_JOB_DUPLEX_PAGE_SIDE_SEL           Ec "&a%dG"
#define USB_PRINTER_PCL5_JOB_SEPARATION                     Ec "&l1T"
#define USB_PRINTER_PCL5_JOB_OUTPUT_BIN_SEL                 Ec "&l%dG"
#define USB_PRINTER_PCL5_JOB_UNIT_OF_MEASURE                Ec "&u%dD"

/* PCL Page control commands */
#define USB_PRINTER_PCL5_PAGE_SIDE                          Ec "&l%dA"
#define USB_PRINTER_PCL5_PAGE_SOURCE                        Ec "&l%dH"
#define USB_PRINTER_PCL5_PAGE_LOGICAL_ORIENTATION           Ec "&l%dO"
#define USB_PRINTER_PCL5_PAGE_DIRECTION                     Ec "&a%dP"
#define USB_PRINTER_PCL5_PAGE_LEFT_MARGIN                   Ec "&a%dL"
#define USB_PRINTER_PCL5_PAGE_RIGHT_MARGIN                  Ec "&a%dM"
#define USB_PRINTER_PCL5_PAGE_CLR_HORIZONTAL_MARGIN         Ec "9"
#define USB_PRINTER_PCL5_PAGE_TOP_MARGIN                    Ec "&a%dE"
#define USB_PRINTER_PCL5_PAGE_TEXT_LENGTH                   Ec "&l%dF"
#define USB_PRINTER_PCL5_PAGE_PERFORATION_SKIP              Ec "&l%dL"
#define USB_PRINTER_PCL5_PAGE_HMI                           Ec "&k%dH"
#define USB_PRINTER_PCL5_PAGE_VMI                           Ec "&l%dC"
#define USB_PRINTER_PCL5_PAGE_LINE_SPACING                  Ec "&l%dD"

/* PCL Cursor positioning */
#define USB_PRINTER_PCL5_CURSOR_HORIZONTAL_POSITION_C       Ec "&a%dC"
#define USB_PRINTER_PCL5_CURSOR_HORIZONTAL_POSITION_H       Ec "&a%dH"
#define USB_PRINTER_PCL5_CURSOR_HORIZONTAL_POSITION_X       Ec "&a%dX"
#define USB_PRINTER_PCL5_CURSOR_VERTICAL_POSITION_R         Ec "&a%dR"
#define USB_PRINTER_PCL5_CURSOR_VERTICAL_POSITION_V         Ec "&a%dV"
#define USB_PRINTER_PCL5_CURSOR_VERTICAL_POSITION_Y         Ec "&a%dY"
#define USB_PRINTER_PCL5_CURSOR_HALF_LINE_FEED              Ec "="
#define USB_PRINTER_PCL5_CURSOR_LINE_TERMINATION            Ec "&k%dG"
#define USB_PRINTER_PCL5_CURSOR_PUSH_POP_POSITION           Ec "&f%dS"

/* PCL Font ID */
#define USB_PRINTER_PCL5_FONT_FRENCH                        Ec "(1F"
#define USB_PRINTER_PCL5_FONT_LATIN                         Ec "(0N"
#define USB_PRINTER_PCL5_FONT_ASCII                         Ec "(0U"
#define USB_PRINTER_PCL5_FONT_LEGAL                         Ec "(1U"
#define USB_PRINTER_PCL5_FONT_ROMAN_8                       Ec "(8U"
#define USB_PRINTER_PCL5_FONT_PC_8                          Ec "(10U"
#define USB_PRINTER_PCL5_FONT_BARCODE                       Ec "(0Y"
#define USB_PRINTER_PCL5_FONT_ANSI                          Ec "(19U"

/* PCL Font selection */
#define USB_PRINTER_PCL5_FONT_SPACING_P                     Ec "(s%dP"
#define USB_PRINTER_PCL5_FONT_SPACING_S                     Ec ")s%dP"
#define USB_PRINTER_PCL5_FONT_PITCH_P                       Ec "(s%dH"
#define USB_PRINTER_PCL5_FONT_PITCH_S                       Ec ")s%dH"
#define USB_PRINTER_PCL5_FONT_HEIGHT_P                      Ec "(s%dV"
#define USB_PRINTER_PCL5_FONT_HEIGHT_S                      Ec ")s%dV"
#define USB_PRINTER_PCL5_FONT_STYLE_P                       Ec "(s%dS"
#define USB_PRINTER_PCL5_FONT_STYLE_S                       Ec ")s%dS"
#define USB_PRINTER_PCL5_FONT_STROKE_WEIGHT_P               Ec "(s%dB"
#define USB_PRINTER_PCL5_FONT_STROKE_WEIGHT_S               Ec ")s%dB"
#define USB_PRINTER_PCL5_FONT_TYPEFACE_FAMILY_P             Ec "(s%dT"
#define USB_PRINTER_PCL5_FONT_TYPEFACE_FAMILY_S             Ec ")s%dT"
#define USB_PRINTER_PCL5_FONT_SEL_BYID_P                    Ec "(%dX"
#define USB_PRINTER_PCL5_FONT_SEL_BYID_S                    Ec ")%dX"
#define USB_PRINTER_PCL5_FONT_SEL_DEFAULT_P                 Ec "(3@"
#define USB_PRINTER_PCL5_FONT_SEL_DEFAULT_S                 Ec ")3@"
#define USB_PRINTER_PCL5_FONT_SEL_DEFAULT                   Ec "(3@"
#define USB_PRINTER_PCL5_FONT_TRANSPARENT                   Ec "&p%dX"
#define USB_PRINTER_PCL5_FONT_UNDERLINE                     Ec "&d%dD"

/* PCL Text selection */    
#define USB_PRINTER_PCL5_TEXT_START                         Ec "L"
#define USB_PRINTER_PCL5_TEXT_STOP                          Ec "\003;"

/* Max Font type supported */
#define USB_PRINTER_PCL5_MAX_FONT                           (8)

/* Max Position buffer */
#define USB_PRINTER_PCL5_POSITION_MAX_BUFF                  (24)

/* Max Page Orientation buffer */
#define USB_PRINTER_PCL5_ORIENTATION_MAX_BUFF               (8)

/* Max Font Buffer */
#define USB_PRINTER_PCL5_FONT_MAX_BUFF                      (65)

/* Struct font name */
typedef struct _pcl5_font_struct_{
    USB_PRINTER_FONT_ID font_id;
    char* font_name;
} USB_PRINTER_PCL5_FONT_STRUCT, _PTR_ USB_PRINTER_PCL5_FONT_STRUCT_PTR;

/* Public function */
USB_STATUS _usb_host_printer_pcl5_write(
    CLASS_CALL_STRUCT_PTR              cc_ptr,
    tr_callback                        callback,
    USB_PRINTER_COMMAND                command,
    uint_32                            buff_size,
    void*                              buffer_ptr
);

#endif


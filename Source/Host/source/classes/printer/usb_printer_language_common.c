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
* $FileName: usb_printer_language_common.c$
* $Version : 
* $Date    : 
*
* Comments:
*
*   
*
*END************************************************************************/
#include "usb_host_printer_pcl5.h"
#include "usb_host_printer_postscript.h"

/* Public function */
/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _usb_host_printer_language_init
* Returned Value : USB_OK or error code
* Comments       :
*
*END*--------------------------------------------------------------------*/
USB_PRINTER_LANGUAGE_FUNC _usb_host_printer_language_init(
    LANGUAGE_ID language_id
)
{
    /* If printer language is pcl5 */
    if(language_id == USB_PRINTER_LANGUAGE_PCL5)
    {
        /* Call public pcl5 language function */
        return _usb_host_printer_pcl5_write;
    }
    else if(language_id == USB_PRINTER_LANGUAGE_POSTSCRIPT)
    {
        /* Call public postscript language function */
        return _usb_host_printer_postscript_write;
    }
    /* Invalid */
    else
    {
        return NULL;
    }
}
/* EOF */

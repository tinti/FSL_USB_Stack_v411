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
 * $FileName: menu_printer.c$
 * $Version : 
 * $Date    : 
 *
 * Comments:
 *
 *   This file is a demo menu of using printer application.
 *   The program help user select options for printing text. 
 *   See the code for details.
 *
 *END************************************************************************/
/* INCLUDE *---------------------------------------------------------------*/
#include <menu_printer.h>
#include <psptypes.h>
#include "usbevent.h"

/* Local variables *------------------------------------------------------*/
/* Language device supported */
char* language_printer_device_support[APP_MAX_NUMBER_LANGUAGE_DEVICE];
/* Printer setup: font, page */
USB_PRINTER_HOST_SETUP printer_setup = 
{
    {100,200},
    {20,COURIER_FONT,0},
    {FALSE},
};
/* Add character null */
char string_printer[APP_MAX_SIZE_STRING_PRINT+1];
char* string_to_print = NULL;
/* Demo string array */
const char default_string[] = "FREESCALE USB HOST PRINTER DEMO";

/* Printer language write pointer */
USB_PRINTER_LANGUAGE_FUNC g_printer_language_write = NULL;
char PJL_PCL5[50] = "@PJL ENTER LANGUAGE = PCL \r\n";
char PJL_POSTSCRIPT[50] = "@PJL ENTER LANGUAGE = POSTSCRIPT \r\n";

/* Global variables *------------------------------------------------------*/
/* Printer device struct */
extern DEVICE_STRUCT printer_device;
/* String ID */
extern uchar string_id_buffer[USB_PRINTER_DEVICE_ID_MAX_SIZE];
/* Printer language ID */
extern LANGUAGE_ID language_id;
extern boolean b_callback;
extern volatile USB_STATUS bStatus;
extern USB_EVENT_STRUCT USB_Event;
/* Local function prototypes *----------------------------------------------*/
/* print string */
static void usb_host_string_print(void);
/* Setup page orientation */
static void usb_host_menu_page_orientation(void);
/* Language menu */
static void usb_host_menu_language_printer(void);
/* Position menu */
static void usb_host_menu_position(void);
/* Font menu */
static void usb_host_menu_font(void);
/* Font type menu */
static void usb_host_menu_font_ID(void);
/* Font style menu */
static void usb_host_menu_font_style(void);
/* String decode */
static void usb_host_string_decode(char* string);
/* Get string fields */
static char* usb_host_string_get_field(char* key, char* string, uint_16* length);
/* Get string */
static void usb_host_string_get_str(char* string);
/* Get value number */
static void usb_host_printer_get_number(char* number);
/* Get character */
static uint_8 usb_host_printer_get_char(void);
/* Show info that is selected to print */
static void usb_host_printer_info_to_print(void);
/* Local functions *-------------------------------------------------------*/
/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_string_decode;
 * Returned Value : none
 * Comments       :
 *     This function is called to separate all the printer language supported 
 *     from the printer string ID.
 *
 *END*--------------------------------------------------------------------*/
static void usb_host_string_decode(char* string)
{
    char* p1;
    char* p;
    int_8 id = -1;
    /* Search character ':' */
    p = strchr(string,':');
    p++;
    /* Get all the printer languages that is supported */
    while(1)
    {
        id++;
        p1 = strchr(p,',');
        if(p1 == NULL)
        {
            if(*p == ' ')
            {
                p++;
            }
            language_printer_device_support[id] = p;
            break;
        }
        *p1 = '\0';
        if(*p == ' ')
            {
                p++;
            }
        language_printer_device_support[id] = p;
        p1++;
        p = p1;
    }
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_string_get_field
 * Returned Value : none
 * Comments       :
 *     Get string fields
 *
 *END*--------------------------------------------------------------------*/
static char* usb_host_string_get_field(char* key, char* string, uint_16* length)
{
    uint_8 i = 0;
    char* p = NULL;
    *length = 0;
    p = strstr(string,key);
    if(NULL != p)
    {
        /* Copy field */
        while((p[*length] != ';'))
        {
            (*length)++;
            if(*length  > strlen(p))
            {
                return NULL;
            }
        };
        return p;
    }
    else
    {
        return NULL;
    }
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_string_get_str
 * Returned Value : poiter to a string
 * Comments       :
 *     Get a string from keyboard
 *
 *END*--------------------------------------------------------------------*/
static void usb_host_string_get_str(char* string)
{
    uint_8 i = 0;
    char tmp;  
    do
    {
        /* Get a character from the keyboard */
        tmp = TERMIO_GetCharNB();
        if((tmp > 31) && (tmp <= 127))
        {
            if(tmp != '\r')
            {
                string[i] = tmp;
                /* Show them on terminal */
                TERMIO_PutChar(string[i]);
                i++;
            }
        }
        if(printer_device.DEV_STATE != USB_DEVICE_INUSE)
            return;
        Poll();
      /* If user type character enter */
    } while((tmp != '\r') && (i < APP_MAX_SIZE_STRING_PRINT));
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_font_get_number
 * Returned Value : none
 * Comments       :
 *     Get a number that is typed from the keyboard.
 *
 *END*--------------------------------------------------------------------*/
static void usb_host_printer_get_number(char* number)
{
    uint_8 i = 0;
    char tmp;  
    do
    {
        /* Get a character from the keyboard */
        tmp = TERMIO_GetCharNB();
        if((tmp >= 48) && (tmp <= 57))
        {
            number[i] = tmp;
            /* Show them on terminal */
            TERMIO_PutChar(number[i]);
            i++;
        }
        if(printer_device.DEV_STATE != USB_DEVICE_INUSE)
            return;
        Poll();
      /* If user type character enter */
    } while((tmp != '\r') && (i < APP_MAX_SIZE_NUMBER));
}


/*LOCAL FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_printer_get_char
 * Returned Value : character
 * Comments       :
 *     This function is called to get char from keyboard 
 *
 *END*--------------------------------------------------------------------*/
static uint_8 usb_host_printer_get_char(void)
{
    uint_8 input_char;
    do
    {
        Poll();
        input_char = TERMIO_GetCharNB();
        if(printer_device.DEV_STATE != USB_DEVICE_INUSE)
            return APP_PRINTER_DEVICE_DETACHED;
    }while((input_char <= 31)||(input_char >= 128));
    return input_char;
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_string_print
 * Returned Value : none
 * Comments       :
 *     This function is called to print demo text
 *
 *END*--------------------------------------------------------------------*/
static void usb_host_string_print(void)
{
    /* Init id language */
    g_printer_language_write = _usb_host_printer_language_init(language_id);
    if(NULL != g_printer_language_write)
    {
        if(USB_PRINTER_LANGUAGE_PCL5 == language_id)
        {
            (void)usb_printer_send_data((pointer)&printer_device.CLASS_INTF, 
                                         usb_host_printer_send_callback, 
                                         NULL, 
                                         strlen(PJL_PCL5),
                                         (uchar_ptr)PJL_PCL5);
        }
        else if(USB_PRINTER_LANGUAGE_POSTSCRIPT == language_id)
        {
            (void)usb_printer_send_data((pointer)&printer_device.CLASS_INTF, 
                                         usb_host_printer_send_callback,
                                         NULL, 
                                         strlen(PJL_POSTSCRIPT),
                                         (uchar_ptr)PJL_POSTSCRIPT);
        }
        /* Wait for send complete */
        while(b_callback == FALSE){
            Poll();
        };
        if (bStatus) printf("Init id language ... FAILED \n");
        b_callback = FALSE;
        /* Set font */
        (void)g_printer_language_write((pointer)&printer_device.CLASS_INTF,
                                        usb_host_printer_send_callback,
                                        USB_PRINTER_COMMAND_FONT,
                                        sizeof(USB_PRINTER_FONT_STRUCT),
                                        (void*)&printer_setup.font_setup);
        /* Wait for send complete */
        while(b_callback == FALSE){
            Poll();
        };
        if (bStatus) printf("Set font ... FAILED \n");
        b_callback = FALSE;
        /* Set position */
        (void)g_printer_language_write((pointer)&printer_device.CLASS_INTF,
                                        usb_host_printer_send_callback,
                                        USB_PRINTER_COMMAND_SET_POSITION,
                                        sizeof(USB_PRINER_POSITION_STRUCT),
                                        (void*)&printer_setup.current_position);
        /* Wait for send complete */
        while(b_callback == FALSE){
            Poll();
        };
        if (bStatus) printf("Set position ... FAILED \n");
        b_callback = FALSE;
        /* Set orientation */
        (void)g_printer_language_write((pointer)&printer_device.CLASS_INTF,
                                        usb_host_printer_send_callback,
                                        USB_PRINTER_COMMAND_ORIENTATION,
                                        sizeof(boolean),
                                        (void*)&printer_setup.page_orientation);
        /* Wait for send complete */
        while(b_callback == FALSE){
            Poll();
        };
        if (bStatus) printf("Set orientation ... FAILED \n");
        b_callback = FALSE;
        /* Start text */
        (void)g_printer_language_write((pointer)&printer_device.CLASS_INTF,
                                        usb_host_printer_send_callback,
                                        USB_PRINTER_COMMAND_TEXT_START,
                                        0,
                                        NULL);
        /* Wait for send complete */
        while(b_callback == FALSE){
            Poll();
        };
        if (bStatus) printf("Start text ... FAILED \n");
        b_callback = FALSE;
        /* Print text */
        if(string_printer[0] == 0)
        {
            string_to_print = (char *)default_string;
        } 
        else 
        {
            string_to_print = string_printer;
        }
        /* Send text */
        (void)g_printer_language_write((pointer)&printer_device.CLASS_INTF,
                                     usb_host_printer_send_callback,
                                     USB_PRINTER_COMMAND_PRINT_TEXT,
                                     strlen(string_to_print),
                                     (void*)string_to_print);
     
        /* Wait for send complete */
        while(b_callback == FALSE){
            Poll();
        }
        if (bStatus) printf("Send text ... FAILED \n");
        b_callback = FALSE;
        /* Stop text */
        (void)g_printer_language_write((pointer)&printer_device.CLASS_INTF,
                                        usb_host_printer_send_callback,
                                        USB_PRINTER_COMMAND_TEXT_STOP,
                                        0,
                                        NULL);
        /* Wait for send complete */
        while(b_callback == FALSE){
            Poll();
        };
        if (!bStatus) printf("\nSend COMPLETE! \n\n");
        else printf("\nSend FAILED! \n\n");
        b_callback = FALSE;
        /* Clear all setting */
        (void)g_printer_language_write((pointer)&printer_device.CLASS_INTF,
                                        usb_host_printer_send_callback,
                                        USB_PRINTER_COMMAND_EJECT_PAGE,
                                        0,
                                        NULL);
        /* Wait for send complete */
        while(b_callback == FALSE){
            Poll();
        };
        if (bStatus) printf("Clear all settings ... FAILED \n");
        b_callback = FALSE;
    }
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_menu_font
 * Returned Value : none
 * Comments       : 
 *     This function is called to provide all font options for user selects.
 *
 *END*--------------------------------------------------------------------*/
static void usb_host_menu_font(void)
{
    uint_8 check = 1;
    uint_8 font_size_input_char = 0;
    char* font_size = NULL;
    /* Check while loop */
    while(check)
    {
        if(printer_device.DEV_STATE != USB_DEVICE_INUSE)
            return;
        /* Show font menu */
        printf("\n \n          Font Menu \n");
        printf("_____________________________________________\n");
        printf("_____________________________________________\n \n");
        printf("1.    Font Type\n");
        printf("2.    Font Style\n");
        printf("3.    Font Size\n");
        printf("4. ...Back \n");
        printf("\nEnter your choice (Press 4 to exit this menu): ");
        /* Get value is entered from keyboard */
        font_size_input_char = usb_host_printer_get_char();
        if(0 == font_size_input_char)
            return;
        switch(font_size_input_char)
        {
            /* If select type menu */
            case APP_FONT_TYPE: 
                /* Show value that has just entered */
                TERMIO_PutChar(APP_FONT_TYPE);
                /* Show type menu */
                usb_host_menu_font_ID();  
                break;
            /* If select size menu */
            case APP_FONT_SIZE:
                /* Show value that has just entered */
                TERMIO_PutChar(APP_FONT_SIZE);
                printf("\nEnter font size: ");    
                /* Get size that is entered from keyboard */
                if(NULL == font_size)
                {
                    font_size = (char*)malloc(APP_MAX_SIZE_FONT+1);
                    if(NULL == font_size)
                    {
                        return;
                    }
                }
                memset(font_size,0,APP_MAX_SIZE_FONT+1);
                /* Get font size from the keyboard */
                usb_host_printer_get_number(font_size);
                /* If font_size equal to 0 */
                if(0 == font_size[0])
                {
                    printer_setup.font_setup.font_size = 20;
                }
                else
                {
                    printer_setup.font_setup.font_size = atoi(font_size);  
                }
                if(NULL != font_size)
                {
                    free(font_size);
                    font_size = NULL;
                }
                break;
            /* If select style menu */
            case APP_FONT_STYLE:
                /* Show value that has just entered */
                TERMIO_PutChar(APP_FONT_STYLE);
                /* Show style menu */
                usb_host_menu_font_style();          
                break;
            case APP_FONT_BACK:     
                /* Show value that has just entered */
                TERMIO_PutChar(APP_FONT_BACK);
                /* Break while loop */
                check = 0;
                break;
            default: 
                printf("\nTry again... \n");
                break;
        }
    }
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_get_language_printer
 * Returned Value : 
 * Comments       :
 *     This function is used to get printer language from a device supported.
 *
 *END*--------------------------------------------------------------------*/
boolean usb_host_get_language_printer(void)
{
    char* str = NULL;
    char* sub_string = NULL;
    uint_16 length;
    uint_8 no_support_language = 0;
    uint_8 index = 0;
    const char* cmd_key[2] = {"COMMAND SET","CMD"};
    /* Get command set info of printer device */
    str = usb_host_string_get_field((char*)cmd_key[0], (char*)(string_id_buffer+2), &length);
    if(NULL != str)
    {
        sub_string = (char*)malloc(length + 1);
        if(NULL == sub_string)
        {
            return FALSE;
        }
        strncpy(sub_string, str, length);
        sub_string[length] = '\0';
    }
    else
    {
        str = usb_host_string_get_field((char*)cmd_key[1], (char*)(string_id_buffer+2), &length);
        if(NULL != str)
        {
            sub_string = (char*)malloc(length + 1);
            if(NULL == sub_string)
            {
                return FALSE;
            }
            strncpy(sub_string, str, length);
            sub_string[length] = '\0';
        }
        else
        {
            printf("\nCan't find the printer language!");
            return FALSE;
        }
    }
    /* Get all printer language that printer device supported */
    usb_host_string_decode(sub_string);
    free(sub_string);
    while(language_printer_device_support[index] != NULL)
    {
        if(strstr(language_printer_device_support[index],"PCL") != NULL)
        {
            no_support_language++;
            language_id = USB_PRINTER_LANGUAGE_PCL5;
            return TRUE;
        }
        if(strstr(language_printer_device_support[index],"Post") != NULL)
        {
            no_support_language++;
            language_id = USB_PRINTER_LANGUAGE_POSTSCRIPT;
            return TRUE;
        }
        index++;
    }
    /* If printer language is invalid */
    if(0 == no_support_language)
    {
        return FALSE;
    }
}
/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_menu_language_printer
 * Returned Value : none
 * Comments       :
 *     This function provides the printer language options for user selects
 *
 *END*--------------------------------------------------------------------*/
static void usb_host_menu_language_printer(void)
{
    char* p1 = NULL;
    char* p2 = NULL;
    uint_8 index = 0;
    uint_8 no_language = 0;
    uint_8 language_input_char = 0;
    char* lang_temp = NULL;
    char* printer_lang = NULL;
    /* Malloc the printer language variable in heap */
    printer_lang = (char*)malloc(APP_MAX_SIZE_STRING_LANGUAGE);
    if(NULL == printer_lang)
    {
        return;
    }
    memset(printer_lang,0,APP_MAX_SIZE_STRING_LANGUAGE);
    /* Get language from stringID of printer device */
    usb_host_get_language_printer();
    /* Show language menu */
    printf("\n \n          Language Menu \n");
    printf("_____________________________________________\n");
    printf("_____________________________________________\n \n");
    while(language_printer_device_support[index] != NULL)
    {
        /* If language is pcl or postscript, it will not contain the character "!" */
        if((strstr(language_printer_device_support[index],"PCL")  != NULL)||
        (strstr(language_printer_device_support[index],"Post") != NULL))
        {
            /* Count number of language that both host and device supported */
            no_language++;
            lang_temp = language_printer_device_support[index];
            printf("%d.    %s\n",index+1,language_printer_device_support[index]);
        }
        /* Else - show ! before that language */
        else
        {
            printf("%d.  ! %s\n",index+1,language_printer_device_support[index]);
        }
        index++;
    }
    if(1 == no_language)
    {
        printf("Default language : %s", lang_temp);
    }
    /* If number of language that both host and device supported which is greater 1 */
    else if(no_language > 1)
    {
        printf("%d. ...Back \n",index+1);
        printf("\nEnter your choice: ");
        /* Select printer language */
        while(1)
        {
            /* Get value is entered from keyboard */
            language_input_char = usb_host_printer_get_char();
            if(language_input_char == 0)
                return;
            /* If selection is invalid */
            if((language_input_char < 49) || (language_input_char > (index + 49)))
            {
                printf("\nTry again...\n");
                printf("\nEnter your choice: ");
            }
            /* If select back*/
            else if(language_input_char == (index + 49))
            {
                /* Show value that has just entered */
                TERMIO_PutChar(language_input_char);
                break;
            }
            /* If select printer languages */
            else
            {
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(language_input_char);
                strcpy(printer_lang,language_printer_device_support[language_input_char-49]);
                p1 = strstr(printer_lang,"PCL");
                p2 = strstr(printer_lang,"Post");
                /* If select PCL printer language */
                if(NULL != p1)
                {  /* PCL 5 */
                    language_id = USB_PRINTER_LANGUAGE_PCL5;
                    printf("\nLanguage: %s \n",printer_lang);
                }
                /* If select postscript printer language */
                if(NULL != p2)
                {
                    /* Postscript */
                    language_id = USB_PRINTER_LANGUAGE_POSTSCRIPT;
                    printf("\nLanguage: %s \n",printer_lang);
                }
                /* If language isn't pcl or postscript */
                if((NULL == p1)&&(NULL == p2))
                {
                    printf("\nLanguage: This language is not supported! \n");
                }
                break;
            }
        }
    }
    else
    {
        printf("\nThe printer host does not support any above language !\n");
    }
    free(printer_lang);
}


/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_menu_position
 * Returned Value : none
 * Comments       :
 *     This function is called to set positions
 *
 *END*--------------------------------------------------------------------*/
static void usb_host_menu_position(void)
{
    uint_8 check = 1;
    uint_8 position_input_char = 0;
    char* position_value = NULL;
    while(check)
    {
        if(printer_device.DEV_STATE != USB_DEVICE_INUSE)
            return;
        printf("\n \n           Position Menu \n");
        printf("_____________________________________________\n");
        printf("_____________________________________________\n \n");
        printf("1.    Position X \n");
        printf("2.    Position Y \n");   
        printf("3. ...Back");
        printf("\n\nEnter your choice: ");
        /* Get value is entered from keyboard */
        position_input_char = usb_host_printer_get_char();
        if(APP_PRINTER_DEVICE_DETACHED == position_input_char)
            return;
        switch(position_input_char)
        {
            case APP_SET_POSITION_X:
                /* Show value that has just entered */
                TERMIO_PutChar(APP_SET_POSITION_X);
                /* Get x position that is entered from keyboard */
                printf("\nEnter x position: ");
                if(NULL == position_value)
                {
                    position_value = (char*)malloc(APP_MAX_SIZE_POSITION+1);
                    if(NULL == position_value)
                    {
                        return;
                    }
                }
                memset(position_value,0,APP_MAX_SIZE_POSITION+1);
                usb_host_printer_get_number(position_value);
                /* If position_value equal to 0 */
                if(0 == position_value[0])
                {
                    printer_setup.current_position.position_x = 100;
                }
                else
                {
                    printer_setup.current_position.position_x = atoi(position_value);  
                }
                if(NULL != position_value)
                {
                    free(position_value);
                    position_value = NULL;
                }
                check = 0;
                break;
            case APP_SET_POSITION_Y:
                /* Show value that has just entered */
                TERMIO_PutChar(APP_SET_POSITION_Y);
                /* Get x position that is entered from keyboard */
                printf("\nEnter y position: ");    
                if(NULL == position_value)
                {
                    position_value = (char*)malloc(APP_MAX_SIZE_POSITION+1);    
                    if(NULL == position_value)
                    {
                        return;
                    }
                }
                memset(position_value,0,APP_MAX_SIZE_POSITION+1);
                usb_host_printer_get_number(position_value);
                /* If position_value equal to 0 */
                if(0 == position_value[0])
                {
                    printer_setup.current_position.position_y = 200;
                }
                else
                {
                    printer_setup.current_position.position_y = atoi(position_value);  
                }
                if(NULL != position_value)
                {
                    free(position_value);
                    position_value = NULL;
                }
                check = 0;
                break;
            case APP_SET_POSITION_BACK:
                TERMIO_PutChar(APP_SET_POSITION_BACK);
                check = 0;
                break;
            default:
                printf("\nTry again...\n");
                break;
        }
    }
}
/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_menu_page_orientation
 * Returned Value : none
 * Comments       :
 *     This function provides page orientation options for user selects
 *
 *END*--------------------------------------------------------------------*/
static void usb_host_menu_page_orientation(void)
{
    uint_8 check = 1;
    uint_8 orientation_input_char = 0;
    /* Show page orientation menu */
    printf("\n \n         Page Orientation Menu \n");
    printf("_____________________________________________\n");
    printf("_____________________________________________\n \n");
    printf("1.    Landscape \n");
    printf("2.    Portrait \n");
    printf("3. ...Back \n");
    printf("\nEnter your choice: ");
    while(check)
    {
        /* Get value is entered from keyboard */
        orientation_input_char = usb_host_printer_get_char();
        if(orientation_input_char == 0)
            return;
        switch(orientation_input_char)
        {
            /* If select landscape */
            case APP_PAGE_LANDSCAPE: 
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(APP_PAGE_LANDSCAPE);
                /* Get value page setup */
                printer_setup.page_orientation = TRUE;
                printf("\nPage Orientation: Landscape\n");
                check = 0;
                break;
            /* If select portrait */
            case APP_PAGE_PORTRAIT:
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(APP_PAGE_PORTRAIT);
                /* Get value page setup */
                printer_setup.page_orientation = FALSE;
                printf("\nPage Orientation: Portrait\n");
                check = 0;
                break; 
            case APP_PAGE_MENU_BACK:
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(APP_PAGE_MENU_BACK);
                check = 0;
                break;
            default: 
                printf("\nTry again...\n");
                printf("\nEnter your choice: ");
                break;
        }
    }
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_menu_font_type
 * Returned Value : none
 * Comments       :
 *     This function provides font types options for user selects
 *
 *END*--------------------------------------------------------------------*/
static void usb_host_menu_font_ID(void)
{
    uint_8 check = 1;
    uint_8 font_id_input_char = 0;
    printf("\n \n           Font Type Menu \n");
    printf("_____________________________________________\n");
    printf("_____________________________________________\n \n");
    printf("1.    Times \n");
    printf("2.    Bookman \n");   
    printf("3.    Courier \n");
    printf("4. ...Back \n");
    printf("\nEnter your choice: ");
    while(check)
    {
        /* Get value is entered from keyboard */
        font_id_input_char = usb_host_printer_get_char();
        if(font_id_input_char == 0)
            return;
        switch(font_id_input_char)
        {
            /* If font type is time new roman */
            case APP_FONT_TYPE_TIMES:
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(APP_FONT_TYPE_TIMES);
                /* Get type of font */
                printer_setup.font_setup.font_ID = TIMES_FONT;
                printf("\nFont Type: Times\n");
                check = 0;
                break;
            /* If font type is bookman */
            case APP_FONT_TYPE_BOOKMAN : 
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(APP_FONT_TYPE_BOOKMAN);
                /* Get type of font */
                printer_setup.font_setup.font_ID = BOOKMAN_FONT;
                printf("\nFont Type: Bookman\n");
                check = 0;
                break; 
            /* If font type is courier*/
            case APP_FONT_TYPE_COURIER:
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(APP_FONT_TYPE_COURIER);
                /* Get type of font */
                printer_setup.font_setup.font_ID = COURIER_FONT;
                printf("\nFont Type: Courier\n");
                check = 0;
                break; 
            /* Back to main menu */
            case APP_FONT_TYPE_MENU_BACK: 
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(APP_FONT_TYPE_MENU_BACK);
                /* Break while loop */
                check = 0;
                break;
            /* If value is invalid */
            default: 
                printf("\nTry again...\n");
                printf("\nEnter your choice: ");
                /* Continuous while loop */
                break;
        }
    }
}

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_menu_font_style
 * Returned Value : none
 * Comments       :
 *     This function provides font styles options for user selects
 *
 *END*--------------------------------------------------------------------*/
static void usb_host_menu_font_style(void)
{
    uint_8 font_style;
    uint_8 check = 1;
    uint_8 font_style_input_char = 0;
    while(check)
    {
        if(printer_device.DEV_STATE != USB_DEVICE_INUSE)
            return;
        printf("\n \n          Font Style Menu \n");
        printf("_____________________________________________\n");
        printf("_____________________________________________\n \n");
        if(printer_setup.font_setup.Font_style.u.Bold)
        {
            printf("1.    Bold      - ON\n");
        }
        else
        {
            printf("1.    Bold      - OFF\n");
        }
        if(printer_setup.font_setup.Font_style.u.Italic)
        {
            printf("2.    Italic    - ON\n"); 
        }
        else
        {
            printf("2.    Italic    - OFF\n"); 
        }
        if(printer_setup.font_setup.Font_style.u.UnderLine)
        {
            printf("3.    Underline - ON\n");
        }
        else
        {
            printf("3.    Underline - OFF\n");
        }
        printf("4. ...Back \n");
        printf("\nEnter your choice (Press 4 to exit this menu): ");
        /* Get value is entered from keyboard */
        font_style_input_char = usb_host_printer_get_char();
        if(font_style_input_char == 0)
            return;
        switch(font_style_input_char)
        {
            /* If font style is bold */
            case APP_FONT_STYLE_BOLD:
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(APP_FONT_STYLE_BOLD);
                /* Get style of font from menu*/
                printer_setup.font_setup.Font_style.u.Bold = ~printer_setup.font_setup.Font_style.u.Bold;
                break;
             /* If font style is italic */
            case APP_FONT_STYLE_ITALIC:
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(APP_FONT_STYLE_ITALIC);
                /* Get style of font from menu*/
                printer_setup.font_setup.Font_style.u.Italic = ~printer_setup.font_setup.Font_style.u.Italic;
                break;
            /* If font style is underline */
            case APP_FONT_STYLE_UNDERLINE:
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(APP_FONT_STYLE_UNDERLINE);
                /* Get style of font from menu*/
                printer_setup.font_setup.Font_style.u.UnderLine = ~printer_setup.font_setup.Font_style.u.UnderLine;
                break;
            /* Back to main menu */
            case APP_FONT_STYLE_MENU_BACK: 
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(APP_FONT_STYLE_MENU_BACK);
                /* Break while loop */
                check = 0;
                break;
            default: 
                printf("\nTry again...\n");
                /* Continue while loop */
                break; 
        }
    }
}
/*PUBLIC FUNCTIONs *------------------------------------------------------*/
/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_show_info_device
 * Returned Value : none
 * Comments       :
 *     This function display all information of usb printer device
 *
 *END*--------------------------------------------------------------------*/
void usb_host_show_info_device(void)
{
    /* There are two standards for printer device stringID */
    const char* printer_dev_IDkey[6] = {"MANUFACTURER","COMMAND SET","MODEL","MFG","CMD","MDL"};
    uint_8 i;
    uint_16 length;
    char* str = NULL;
    char* str_info = NULL;
    /* Show printer device information */
    printf("\n________________PRINTER DEVICE INFORMATION________________\n\n"); 
    for(i = 0 ; i < 6 ; i++)
    {          
        str = usb_host_string_get_field((char*)printer_dev_IDkey[i],(char*)(string_id_buffer+2), &length);
        if(NULL != str)
        {
            str_info = (char*)malloc(length + 1);
            if(NULL == str_info)
            {
                return;
            }
            strncpy(str_info, str, length);
            str_info[length] = '\0';
            printf("%s\n", str_info);
            free(str_info);
        }
    }
    printf("__________________________________________________________\n");
}
/*LOCAL FUNCTIONs *------------------------------------------------------*/
/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_printer_info_to_print
 * Returned Value : none
 * Comments       :
 *     This function display all informations to print
 *
 *END*--------------------------------------------------------------------*/
static void usb_host_printer_info_to_print(void)
{
    printf("\n--------------------------------------\n");
    printf("          Information\n");
    /* ----------Language info----------- */
    if(USB_PRINTER_LANGUAGE_PCL5 == language_id)
    {
        printf("Language: PCL5\n");
    }
    else if(USB_PRINTER_LANGUAGE_POSTSCRIPT == language_id)
    {
        printf("Language: Postscript\n");
    }
    else
    {
        printf("Unkown language!\n");
    }
    
    /* -----------Font info-----------*/
    /* Font type */
    if(TIMES_FONT == printer_setup.font_setup.font_ID)
    {
        printf("Font Type: Times\n");
    }
    else if(BOOKMAN_FONT == printer_setup.font_setup.font_ID)
    {
        printf("Font Type: Bookman\n");
    }
    else if(COURIER_FONT == printer_setup.font_setup.font_ID)
    {
        printf("Font Type: Courier\n");
    }
    else
    {
        printf("Unkown font type \n");
    }
    /* Font style */
    printf("Font Style: ");
    if(printer_setup.font_setup.Font_style.u.Bold)
    {
        printf(" Bold ");
    }
    if(printer_setup.font_setup.Font_style.u.Italic)
    {
        printf(" Italic ");
    }
    if(printer_setup.font_setup.Font_style.u.UnderLine)
    {
        printf(" Underline ");
    }
    if(!printer_setup.font_setup.Font_style.value)
    {
        printf(" No style ");
    }
    printf("\n");
    /* Font size */
    printf("Font Size: %d\n",printer_setup.font_setup.font_size);
    
    /* --------------Page orientation-------------*/
    if(printer_setup.page_orientation)
    {
        printf("Page Orientation: Landscape \n");
    }
    else
    {
        printf("Page Orientation: Portrait \n");
    }
    
    /*----------------Position--------------------*/
    printf("Position: X = %d   Y = %d\n",
            printer_setup.current_position.position_x,
            printer_setup.current_position.position_y);
    /*----------------String----------------------*/
    if(string_printer[0] == 0)
    {
        printf("String: %s",default_string);
    }
    else
    {
        printf("String: %s", string_printer);
    }
    printf("\n--------------------------------------\n");
}
/*PUBLIC FUNCTIONs *------------------------------------------------------*/
/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name  : usb_host_menu_main
 * Returned Value : none
 * Comments       :
 *     This function provides menu options for user selects
 *
 *END*--------------------------------------------------------------------*/
void usb_host_menu_main(void)
{
    /* Enter value from keyboard to select menu */
    uint_8 main_input_char = 0; 
    
    /* Show main menu */
    printf("\n \n            Main Menu \n");
    printf("_____________________________________________\n");
    printf("_____________________________________________\n \n");
    printf("1.    Printer Language \n");
    printf("2.    Position \n");
    printf("3.    Font\n");
    printf("4.    Page Orientation \n");
    printf("5.    String\n");
    printf("6.    Print\n");
    /* Select your choice */
    printf("\nEnter your choice: ");
    /* Get value is entered from keyboard */
    main_input_char = usb_host_printer_get_char();
    if(main_input_char == 0)
        return;
    /* Check all value that is selected */
    switch(main_input_char)
    {
        /* If select languge menu */
        case APP_SELECT_PRINTER_LANGUAGE_MENU:
            /* Show value that has just entered */
            TERMIO_PutChar(APP_SELECT_PRINTER_LANGUAGE_MENU);
            /* Show languge menu */
            usb_host_menu_language_printer();
            break;
        case APP_SELECT_POSITION_MENU:
            /* Show value that has just entered */
            TERMIO_PutChar(APP_SELECT_POSITION_MENU);
            /* Show position menu */
            usb_host_menu_position();
            break;
        case APP_SELECT_FONT_MENU:
            /* Show value that has just entered */
            TERMIO_PutChar(APP_SELECT_FONT_MENU);
            /* Show font menu */
            usb_host_menu_font();
            break;
        /* If select page menu */
        case APP_SELECT_PAGE_ORIENTATION_MENU:
            /* Show value that has just entered */
            TERMIO_PutChar(APP_SELECT_PAGE_ORIENTATION_MENU);
            /* Show page setup menu */
            usb_host_menu_page_orientation();
            break;
        /* If select style of font */   
        case APP_SELECT_STRING_MENU:
            /* Show value that has just entered */
            TERMIO_PutChar(APP_SELECT_STRING_MENU);
            /* Type the string */
            printf("\nEnter the string: "); 
            /* Set memory equal to zero */
            memset(string_printer,0,APP_MAX_SIZE_STRING_PRINT+1);
            /* Get string to print */
            usb_host_string_get_str(string_printer);
            break;
        /* If enter 5 from keyboard - save all values of variables and break while loop */
        case APP_SELECT_PRINT_STRING:
            if((printer_setup.font_setup.Font_style.u.UnderLine == 1)&&(language_id == USB_PRINTER_LANGUAGE_POSTSCRIPT))
            {
                printf("\nThe Postscrip language unsupports the font style: UnderLine ");
                printf("\nTry again... \n");
            }
            else
            {
                /* Show on terminal value that has just entered from keyboard */
                TERMIO_PutChar(APP_SELECT_PRINT_STRING);
                /* Show info that have selected */
                usb_host_printer_info_to_print();
                /* Print string */
                usb_host_string_print();
            }
            break;
        default:
            printf("\nTry again... \n");
            break;
    }
}
/* EOF */

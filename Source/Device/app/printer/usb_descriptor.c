/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
*
**************************************************************************//*!
*
* @file usb_descriptor.c
*
* @author 
*
* @version 
*
* @date May-08-2009
*
* @brief The file contains USB descriptors and functions
*
*****************************************************************************/

/******************************************************************************
* Includes
*****************************************************************************/
#include "derivative.h"
#include "usb_descriptor.h"

#if (defined __MCF52xxx_H__)||(defined __MK_xxx_H__)
/* Put CFV2 descriptors in RAM */
#define USB_DESC_CONST
#else
#define USB_DESC_CONST    const
#endif

/*****************************************************************************
* Constant and Macro's
*****************************************************************************/ 
/* structure containing details of all the endpoints used by this device */ 
USB_DESC_CONST USB_ENDPOINTS usb_desc_ep = 
{
    PRINTER_DESC_ENDPOINT_COUNT,
    {  
        {
            PRINTER_BULK_IN_ENDPOINT,
            USB_BULK_PIPE, 
            USB_SEND,
            PRINTER_BULK_IN_ENDP_PACKET_SIZE
        }, 
        {
            PRINTER_BULK_OUT_ENDPOINT,
            USB_BULK_PIPE, 
            USB_RECV,
            PRINTER_BULK_OUT_ENDP_PACKET_SIZE
        }        
    }
};

uint_8 USB_DESC_CONST g_device_descriptor[DEVICE_DESCRIPTOR_SIZE] =  
{
    /* Device Dexcriptor Size */
    DEVICE_DESCRIPTOR_SIZE,               
    /* Devic Type of descriptor */   
    USB_DEVICE_DESCRIPTOR,                
    /* BCD USB version  */  
    USB_uint_16_low(BCD_USB_VERSION), USB_uint_16_high(BCD_USB_VERSION),
    /* Device Class is indicated in the interface descriptors */   
    DEVICE_DESC_DEVICE_CLASS,
    /* Device Subclass is indicated in the interface descriptors  */      
    DEVICE_DESC_DEVICE_SUBCLASS,
    /* Device Protocol  */     
    DEVICE_DESC_DEVICE_PROTOCOL,
    /* Max Packet size */
    CONTROL_MAX_PACKET_SIZE,
    /* Vendor ID */
    0xA2,0x15,
    /* Product ID */
    0x00,0x01,  
    /* BCD Device version */
    0x02,0x00,
    /* Manufacturer string index */
    0x01,     
    /* Product string index */  
    0x02,                        
    /* Serial number string index */
    0x03,                  
    /* Number of configurations */
    DEVICE_DESC_NUM_CONFIG_SUPPOTED                           
};

uint_8 USB_DESC_CONST g_config_descriptor[CONFIG_DESC_SIZE] =         
{   
    /* Configuration Descriptor Size - always 9 bytes*/
    CONFIG_ONLY_DESC_SIZE,
    USB_CONFIG_DESCRIPTOR,                         /* "Configuration" type of descriptor */   
    USB_uint_16_low(CONFIG_DESC_SIZE),
    USB_uint_16_high(CONFIG_DESC_SIZE),            /*  Total length of the Configuration descriptor */ 
    CONFIG_DESC_NUM_INTERFACES_SUPPOTED,           /*  NumInterfaces */   
    1,                                             /*  Configuration Value */      
    0,                                             /* Configuration Description String Index */  
    BUS_POWERED|SELF_POWERED|
    (REMOTE_WAKEUP_SUPPORT
    <<REMOTE_WAKEUP_SHIFT),                        /*  Attributes.support RemoteWakeup and self power */
    CONFIG_DESC_CURRENT_DRAWN,                     /*  Current draw from bus */
    
    /* Interface descriptor */
    IFACE_ONLY_DESC_SIZE,
    USB_IFACE_DESCRIPTOR,
    0x00,                                          /* bInterfaceNumber */
    0x00,                                          /* bAlternateSetting */
    (uint_8)PRINTER_DESC_ENDPOINT_COUNT,           /* bNumEndpoints */
    PRINTER_CLASS,                                 /* Interface Class */    
    PRINTER_SUB_CLASS,                             /* Interface Subclass*/
    BI_DIRECTIONAL_PROTOCOL,                       /* Interface Protocol*/
    0x00,                                          /* Interface Description String Index*/

    /*Endpoint descriptor */
    ENDP_ONLY_DESC_SIZE,
    USB_ENDPOINT_DESCRIPTOR,
    PRINTER_BULK_IN_ENDPOINT|(USB_SEND << 7),
    USB_BULK_PIPE, 
    USB_uint_16_low(PRINTER_BULK_IN_ENDP_PACKET_SIZE),
    USB_uint_16_high(PRINTER_BULK_IN_ENDP_PACKET_SIZE),
    0x00,                                          /* This value is ignored for Bulk ENDPOINT */
    
    /*Endpoint descriptor */
    ENDP_ONLY_DESC_SIZE,
    USB_ENDPOINT_DESCRIPTOR,
    PRINTER_BULK_OUT_ENDPOINT|(USB_RECV << 7),
    USB_BULK_PIPE,
    USB_uint_16_low(PRINTER_BULK_OUT_ENDP_PACKET_SIZE),
    USB_uint_16_high(PRINTER_BULK_OUT_ENDP_PACKET_SIZE),
    0x00                                           /* This value is ignored for Bulk ENDPOINT */
};

uint_8 USB_DESC_CONST USB_STR_0[USB_STR_0_SIZE+USB_STR_DESC_SIZE] =     
{ sizeof(USB_STR_0),    
    USB_STRING_DESCRIPTOR, 
    0x09,
    0x04/* Equivalent to 0x0409*/ 
};

/*  Manufacturer string */                                    
uint_8 USB_DESC_CONST USB_STR_1[USB_STR_1_SIZE+USB_STR_DESC_SIZE] = 
{
    sizeof(USB_STR_1),          
    USB_STRING_DESCRIPTOR,
    'F',0,
    'R',0,
    'E',0,
    'E',0,
    'S',0,
    'C',0,
    'A',0,
    'L',0,
    'E',0,
    ' ',0,
    'S',0,
    'E',0,
    'M',0,
    'I',0,
    'C',0,
    'O',0,
    'N',0,
    'D',0,
    'U',0,
    'C',0,
    'T',0,
    'O',0,
    'R',0,
    ' ',0,
    'I',0,
    'N',0,
    'C',0,
    '.',0                               
};

uint_8 USB_DESC_CONST USB_STR_2[USB_STR_2_SIZE+USB_STR_DESC_SIZE]= 
{
    sizeof(USB_STR_2),
    USB_STRING_DESCRIPTOR,
    ' ',0,
    ' ',0,
#ifdef __MK_xxx_H__
    'M',0,
    'K',0,                               
#elif (defined __MCF52xxx_H__)
    'C',0,
    'F',0, 
 #elif (defined MCU_mcf51jf128)
    'J',0,
    'F',0,                               
#else     
    'J',0,
    'M',0,
#endif                               
    ' ',0,
    'P',0,
    'R',0,
    'I',0,
    'N',0,
    'T',0,
    'E',0,
    'R',0,
    ' ',0,
    'D',0,
    'E',0,
    'M',0,
    'O',0,
    ' ',0,
};

/*  Serial number string */
uint_8 USB_DESC_CONST USB_STR_3[USB_STR_3_SIZE+USB_STR_DESC_SIZE] = 
{  sizeof(USB_STR_3),          
    USB_STRING_DESCRIPTOR,                                
    '0',0,
    '1',0,
    '2',0,
    '3',0,
    '4',0,
    '5',0,
    '6',0,
    '7',0,
    '8',0,
    '9',0,
    'A',0,
    'B',0,
    'C',0,
    'D',0,
    'E',0,
    'F',0
};                 

uint_8 USB_DESC_CONST USB_STR_n[USB_STR_n_SIZE+USB_STR_DESC_SIZE] =
{  sizeof(USB_STR_n),         
    USB_STRING_DESCRIPTOR,                                
    'B',0,
    'A',0,
    'D',0,
    ' ',0,
    'S',0,
    'T',0,
    'R',0,
    'I',0,
    'N',0,
    'G',0,
    ' ',0,
    'I',0,
    'N',0,
    'D',0,
    'E',0,
    'X',0                               
};

uint_32 const g_std_desc_size[USB_MAX_STD_DESCRIPTORS+1] =
{ 0,
    DEVICE_DESCRIPTOR_SIZE,
    CONFIG_DESC_SIZE,
    0, /* string */
    0, /* Interfdace */
    0, /* Endpoint */                                      
    0, /* Device Qualifier */
    0 /* other spped config */
};   

uint_8 const *g_std_descriptors[USB_MAX_STD_DESCRIPTORS+1] = 
{
    NULL,
    g_device_descriptor,
    g_config_descriptor,
    NULL, /* string */
    NULL, /* Interfdace */
    NULL, /* Endpoint */
    NULL, /* Device Qualifier */
    NULL /* other spped config*/
}; 

uint_8 const g_string_desc_size[USB_MAX_STRING_DESCRIPTORS+1] = 
{ sizeof(USB_STR_0),
    sizeof(USB_STR_1),
    sizeof(USB_STR_2),
    sizeof(USB_STR_3),
    sizeof(USB_STR_n)
};   

uint_8 const *g_string_descriptors[USB_MAX_STRING_DESCRIPTORS+1] = 
{
    USB_STR_0,
    USB_STR_1,
    USB_STR_2,
    USB_STR_3,
    USB_STR_n
};    

USB_ALL_LANGUAGES g_languages = 
{ 
    USB_STR_0, 
    sizeof(USB_STR_0), 
    { 
        (uint_16 const)0x0409,
        (const uint_8 **)g_string_descriptors, g_string_desc_size
    }
};

uint_8 const g_valid_config_values[USB_MAX_CONFIG_SUPPORTED+1] = {0,1};

/****************************************************************************
* Global Variables
****************************************************************************/
static uint_8 g_alternate_interface[USB_MAX_SUPPORTED_INTERFACES];

/*****************************************************************************
* Global Functions
*****************************************************************************/
/**************************************************************************//*!
*
* @name  USB_Desc_Get_Descriptor
*
* @brief The function returns the correponding descriptor
*
* @param controller_ID:        To identify the controller     
* @param type:          type of descriptor requested     
* @param str_num:       string index for string descriptor     
* @param index:         string descriptor language Id     
* @param descriptor:    output descriptor pointer
* @param size:          size of descriptor returned
*
* @return USB_OK                              When Successfull
*         USBERR_INVALID_REQ_TYPE             when Error
*****************************************************************************/
uint_8 USB_Desc_Get_Descriptor(
    uint_8 controller_ID, 
    uint_8 type,
    uint_8 str_num, 
    uint_16 index,
    uint_8_ptr *descriptor,
    uint_32 *size
) 
{
    UNUSED (controller_ID)
    /* string descriptors are handled saperately */
    if (type == USB_STRING_DESCRIPTOR)
    {
        if(index == 0) 
        {  
            /* return the string and size of all languages */      
            *descriptor = (uint_8_ptr)g_languages.languages_supported_string;
            *size = g_languages.languages_supported_size;            
        } 
        else 
        {
            uint_8 lang_id=0;
            uint_8 lang_index=USB_MAX_LANGUAGES_SUPPORTED;
            
            for(;lang_id< USB_MAX_LANGUAGES_SUPPORTED;lang_id++) 
            {
                /* check whether we have a string for this language */
                if(index == g_languages.usb_language[lang_id].language_id) 
                {   /* check for max descriptors */
                    if(str_num < USB_MAX_STRING_DESCRIPTORS) 
                    {   /* setup index for the string to be returned */
                        lang_index=str_num;                 
                    }                    
                    break;                    
                }                
            }
            
            /* set return val for descriptor and size */
            *descriptor = (uint_8_ptr)
            g_languages.usb_language[lang_id].lang_desc[lang_index];
            *size = 
            g_languages.usb_language[lang_id].lang_desc_size[lang_index];
        }        
    }
    else if (type < USB_MAX_STD_DESCRIPTORS+1)
    {
        /* set return val for descriptor and size*/
        *descriptor = (uint_8_ptr)g_std_descriptors [type];
        
        /* if there is no descriptor then return error */
        if(*descriptor == NULL) 
        {
            return USBERR_INVALID_REQ_TYPE;
        }        
        *size = g_std_desc_size[type];                
    }
    else /* invalid descriptor */
    {
        return USBERR_INVALID_REQ_TYPE;
    }
    return USB_OK;  
}

/**************************************************************************//*!
*
* @name  USB_Desc_Get_Interface
*
* @brief The function returns the alternate interface
*
* @param controller_ID:        To identify the controller:              
* @param interface:      interface number     
* @param alt_interface:  output alternate interface     
*
* @return USB_OK                              When Successfull
*         USBERR_INVALID_REQ_TYPE             when Error
*****************************************************************************/
uint_8 USB_Desc_Get_Interface(
    uint_8 controller_ID, 
    uint_8 interface, 
    uint_8_ptr alt_interface
)
{   
    UNUSED (controller_ID)
    /* if interface valid */
    if(interface < USB_MAX_SUPPORTED_INTERFACES)
    {
        /* get alternate interface*/
        *alt_interface = g_alternate_interface[interface];
        return USB_OK;  
    }
    return USBERR_INVALID_REQ_TYPE;
}

/**************************************************************************//*!
*
* @name  USB_Desc_Set_Interface
*
* @brief The function sets the alternate interface
*
* @param controller_ID:        To identify the controller:              
* @param interface:      interface number     
* @param alt_interface:  input alternate interface     
*
* @return USB_OK                              When Successfull
*         USBERR_INVALID_REQ_TYPE             when Error
*****************************************************************************/
uint_8 USB_Desc_Set_Interface(
    uint_8 controller_ID, 
    uint_8 interface, 
    uint_8 alt_interface
)
{
    UNUSED (controller_ID)
    /* if interface valid */
    if(interface < USB_MAX_SUPPORTED_INTERFACES)
    {
        /* set alternate interface*/
        g_alternate_interface[interface]=alt_interface;
        return USB_OK;  
    }
    return USBERR_INVALID_REQ_TYPE;
}

/**************************************************************************//*!
*
* @name  USB_Desc_Valid_Configation
*
* @brief The function checks whether the configuration parameter 
*        input is valid or not
*
* @param controller_ID:        To identify the controller              
* @param config_val      configuration value     
*
* @return TRUE           When Valid
*         FALSE          When Error
*****************************************************************************/
boolean USB_Desc_Valid_Configation(
    uint_8 controller_ID,
    uint_16 config_val
)
{
    uint_8 loop_index=0;
    UNUSED (controller_ID)
    /* check with only supported val right now */
    while(loop_index < (USB_MAX_CONFIG_SUPPORTED+1)) 
    {
        if(config_val == g_valid_config_values[loop_index]) 
        {          
            return TRUE;
        }
        loop_index++;
    }    
    return FALSE;    
}

/**************************************************************************//*!
*
* @name  USB_Desc_Remote_Wakeup
*
* @brief The function checks whether the remote wakeup is supported or not
*
* @param controller_ID:        To identify the controller     
*
* @return REMOTE_WAKEUP_SUPPORT (TRUE) - if remote wakeup supported
*****************************************************************************/
boolean USB_Desc_Remote_Wakeup(
    uint_8 controller_ID
) 
{
    UNUSED (controller_ID)
    return REMOTE_WAKEUP_SUPPORT;   
}           

/**************************************************************************//*!
*
* @name  USB_Desc_Get_Endpoints
*
* @brief The function returns with the list of all non control endpoints used
*
* @param controller_ID:        To identify the controller:              
*
* @return pointer to USB_ENDPOINTS
*****************************************************************************/
USB_ENDPOINTS *USB_Desc_Get_Endpoints(
    uint_8 controller_ID
) 
{
    UNUSED (controller_ID) 
    return &usb_desc_ep;
}         

/* EOF */

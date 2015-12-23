/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 **************************************************************************//*!
 *
 * @file usb_descriptor.h
 *
 * @author
 *
 * @version
 *
 * @date May-28-2009
 *
 * @brief The file is a header file for USB Descriptors required for Keyboard
 *        Application
 *
 *****************************************************************************/

#ifndef _USB_DESCRIPTOR_H
#define _USB_DESCRIPTOR_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "types.h"
#include "usb_class.h"

/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/
#define REMOTE_WAKEUP_SHIFT              (5)
#define REMOTE_WAKEUP_SUPPORT            (TRUE)

/* Various descriptor sizes */
#define DEVICE_DESCRIPTOR_SIZE            (18)
#define DEVICE_QUALIFIER_DESCRIPTOR_SIZE  (10)
#define REPORT_DESC_SIZE                  (63)
#define CONFIG_ONLY_DESC_SIZE             (9)
#define IFACE_ONLY_DESC_SIZE              (9)
#define HID_ONLY_DESC_SIZE                (9)
#define ENDP_ONLY_DESC_SIZE               (7)

#ifdef OTG_BUILD
#define OTG_ONLY_DESC_SIZE                (5)
#define CONFIG_DESC_SIZE                  ((34) + OTG_ONLY_DESC_SIZE)
#else
#define CONFIG_DESC_SIZE                  (34)
#endif



/* HID buffer size */
#define HID_BUFFER_SIZE                   (8)
/* Max descriptors provided by the Application */
#define USB_MAX_STD_DESCRIPTORS               (8)
#define USB_MAX_CLASS_SPECIFIC_DESCRIPTORS    (2)
/* Max configuration supported by the Application */
#define USB_MAX_CONFIG_SUPPORTED          (1)

/* Max string descriptors supported by the Application */
#define USB_MAX_STRING_DESCRIPTORS        (3)

/* Max language codes supported by the USB */
#define USB_MAX_LANGUAGES_SUPPORTED       (1)


#define HID_DESC_ENDPOINT_COUNT         (1)
#define HID_ENDPOINT                    (1)
#define HID_ENDPOINT_PACKET_SIZE        (8)

/* OTG descriptor capabilities  */
#ifdef OTG_BUILD
#define USB_OTG_SRP_SUPPORT             (0x01)
#define USB_OTG_HNP_SUPPORT             (0x02)
#define USB_OTG_ADP_SUPPORT             (0x04)
#define USB_OTG_VERSION_2_0             (0x0200) /* BCD encoding for OTG and EH 2.0 */
#endif

/* string descriptors sizes */
#define USB_STR_DESC_SIZE               (2)
#define USB_STR_0_SIZE                  (2)
#define USB_STR_1_SIZE                  (56)
#ifdef OTG_BUILD
#define USB_STR_2_SIZE                  (48)
#else
#define USB_STR_2_SIZE                  (38)
#endif
#define USB_STR_n_SIZE                  (32)

/* descriptors codes */
#define USB_DEVICE_DESCRIPTOR     (1)
#define USB_CONFIG_DESCRIPTOR     (2)
#define USB_STRING_DESCRIPTOR     (3)
#define USB_IFACE_DESCRIPTOR      (4)
#define USB_ENDPOINT_DESCRIPTOR   (5)
#define USB_DEVQUAL_DESCRIPTOR    (6)

#ifdef OTG_BUILD
#define USB_OTG_DESCRIPTOR        (9)
#endif

#define USB_HID_DESCRIPTOR        (0x21)
#define USB_REPORT_DESCRIPTOR     (0x22)

#define USB_MAX_SUPPORTED_INTERFACES     (1)

/******************************************************************************
 * Types
 *****************************************************************************/
typedef const struct _USB_LANGUAGE
{
    uint_16 const language_id;      /* Language ID */
    uint_8 const ** lang_desc;      /* Language Descriptor String */
    uint_8 const * lang_desc_size;  /* Language Descriptor Size */
} USB_LANGUAGE;

typedef const struct _USB_ALL_LANGUAGES
{
    /* Pointer to Supported Language String */
    uint_8 const *languages_supported_string;
    /* Size of Supported Language String */
    uint_8 const languages_supported_size;
    /* Array of Supported Languages */
    USB_LANGUAGE usb_language[USB_MAX_SUPPORTED_INTERFACES];
}USB_ALL_LANGUAGES;

typedef const struct _USB_ENDPOINTS
{
    /* Number of non control Endpoints */
    uint_8 count;
    /* Array of Endpoints Structures */
    USB_EP_STRUCT ep[HID_DESC_ENDPOINT_COUNT];
}USB_ENDPOINTS;

#ifdef OTG_BUILD
typedef struct usb_otg_descriptor_tag
{
   uint_8   bLength;          /* Descriptor size in bytes = 9 */
   uint_8   bDescriptorType;  /* CONFIGURATION type = 2 or 7 */
   uint_8   bmAttributes;     /* OTG characteristics */
   #define  OTG_SRP_SUPPORT   (0x01)  /* Supports SRP */
   #define  OTG_HNP_SUPPORT   (0x02)  /* Supports HNP */
   uint_8   bcdOTG[2];       /* OTG supplement release number */   
} OTG_DESCRIPTOR_T, _PTR_ OTG_DESCRIPTOR_PTR_T;  
#endif

/******************************************************************************
 * Global Functions
 *****************************************************************************/
extern boolean USB_Desc_Remote_Wakeup(uint_8 controller_ID);

extern void* USB_Desc_Get_Endpoints(uint_8 controller_ID);
extern uint_8 USB_Desc_Get_Descriptor(
                                uint_8 controller_ID,
                                uint_8 type,
                                uint_8 str_num,
                                uint_16 index,
                                uint_8_ptr *descriptor,
                                USB_PACKET_SIZE *size);

extern uint_8 USB_Desc_Get_Interface(
                                uint_8 controller_ID,
                                uint_8 interface,
                                uint_8_ptr alt_interface);


extern uint_8 USB_Desc_Set_Interface(
                                uint_8 controller_ID,
                                uint_8 interface,
                                uint_8 alt_interface);

extern boolean USB_Desc_Valid_Configation(
                                uint_8 controller_ID,
                                uint_16 config_val);

extern boolean USB_Desc_Valid_Interface(
                                uint_8 controller_ID,
                                uint_8 interface);


#endif

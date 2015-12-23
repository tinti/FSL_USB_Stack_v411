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
* $FileName: usb_prvhs_cfv2.h$
* $Version : 3.5.2.0$
* $Date    : Jan-8-2010$
*
* Comments:
*
*  This file contains the private defines, externs and
*  data structure definitions required by the VUSB_HS Device
*  driver.
*                                                               
*END*********************************************************************/

#ifndef __usb_prvhs_cfv2_h__
#define __usb_prvhs_cfv2_h__ 1

#define  MAX_EP_TR_DESCRS                    (32) 
#define  MAX_XDS_FOR_TR_CALLS                (32) 
#define  MAX_USB_DEVICES                     (1)
#define  USB_MAX_ENDPOINTS                   (4) 
#define  ZERO_LENGTH                         (0) 

#define  USB_MAX_CTRL_PAYLOAD                (64)
 
/* Macro for aligning the EP queue head to 32 byte boundary */
#define USB_MEM32_ALIGN(n)                   ((n) + (-(n) & 31))

/* Macro for aligning the EP queue head to 1024 byte boundary */
#define USB_MEM1024_ALIGN(n)                 ((n) + (-(n) & 1023))

/* Macro for aligning the EP queue head to 1024 byte boundary */
#define USB_MEM2048_ALIGN(n)                 ((n) + (-(n) & 2047))

#define USB_XD_QADD(head,tail,XD)      \
   if ((head) == NULL) {         \
      (head) = (XD);            \
   } else {                      \
      (tail)->SCRATCH_PTR->PRIVATE = (XD);   \
   } /* Endif */                 \
   (tail) = (XD);               \
   (XD)->SCRATCH_PTR->PRIVATE = NULL
   
#define USB_XD_QGET(head,tail,XD)      \
   (XD) = (head);               \
   if (head) {                   \
      (head) = (XD_STRUCT_PTR)((head)->SCRATCH_PTR->PRIVATE);  \
      if ((head) == NULL) {      \
         (tail) = NULL;          \
      } /* Endif */              \
   } /* Endif */

#define EHCI_DTD_QADD(head,tail,dTD)      \
   if ((head) == NULL) {         \
      (head) = (dTD);            \
   } else {                      \
      (tail)->SCRATCH_PTR->PRIVATE = (void *) (dTD);   \
   } /* Endif */                 \
   (tail) = (dTD);               \
   (dTD)->SCRATCH_PTR->PRIVATE = NULL
   
#define EHCI_DTD_QGET(head,tail,dTD)      \
   (dTD) = (head);               \
   if (head) {                   \
      (head) = (head)->SCRATCH_PTR->PRIVATE;  \
      if ((head) == NULL) {      \
         (tail) = NULL;          \
      } /* Endif */              \
   } /* Endif */

/***************************************
**
** Data structures
**
*/

typedef struct xd_struct 
{
   uint_8         EP_NUM;           /* Endpoint number */
   uint_8         BDIRECTION;       /* Direction : Send/Receive */
   uint_8         EP_TYPE;          /* Type of the endpoint: Ctrl, Isoch, Bulk, 
                                    ** Int 
                                    */
   uint_8         BSTATUS;          /* Current transfer status */
   uint_8_ptr     WSTARTADDRESS;    /* Address of first byte */
   uint_32        WTOTALLENGTH;     /* Number of bytes to send/recv */
   uint_32        WSOFAR;           /* Number of bytes recv'd so far */
   uint_16        WMAXPACKETSIZE;   /* Max Packet size */
   boolean        DONT_ZERO_TERMINATE;
   uint_8         MAX_PKTS_PER_UFRAME;
   SCRATCH_STRUCT_PTR SCRATCH_PTR;
} XD_STRUCT, *XD_STRUCT_PTR;

/* The USB Device State Structure */
typedef struct 
{
   boolean                          BUS_RESETTING;       /* Device is 
                                                         ** being reset 
                                                         */
   boolean                          TRANSFER_PENDING;    /* Transfer pending ? */

   
   XD_STRUCT_PTR                    TEMP_XD_PTR;         /* Temp xd for ep init */
   XD_STRUCT_PTR                    XD_BASE;
   XD_STRUCT_PTR                    XD_HEAD;             /* Head Transaction 
                                                         ** descriptors 
                                                         */
   XD_STRUCT_PTR                    XD_TAIL;             /* Tail Transaction 
                                                         ** descriptors 
                                                         */
   XD_STRUCT_PTR                    PENDING_XD_PTR;      /* pending transfer */
   uint_32                          XD_ENTRIES;
   USBHS_EP_QUEUE_HEAD_STRUCT_PTR  EP_QUEUE_HEAD_PTR;   /* Endpoint Queue 
                                                         ** head 
                                                         */   
   uint_8_ptr                       DRIVER_MEMORY;/* pointer to driver memory*/
   uint_32                          TOTAL_MEMORY; /* total memory occupied 
                                                     by driver */
   USBHS_EP_QUEUE_HEAD_STRUCT_PTR  EP_QUEUE_HEAD_BASE;
   USBHS_EP_TR_STRUCT_PTR          DTD_BASE_PTR;        /* Device transfer 
                                                         ** descriptor pool 
                                                         ** address 
                                                         */
   USBHS_EP_TR_STRUCT_PTR          DTD_ALIGNED_BASE_PTR;/* Aligned transfer 
                                                         ** descriptor pool 
                                                         ** address 
                                                         */
   USBHS_EP_TR_STRUCT_PTR          DTD_HEAD;
   USBHS_EP_TR_STRUCT_PTR          DTD_TAIL;
   USBHS_EP_TR_STRUCT_PTR          EP_DTD_HEADS[USB_MAX_ENDPOINTS * 2];
   USBHS_EP_TR_STRUCT_PTR          EP_DTD_TAILS[USB_MAX_ENDPOINTS * 2];
   
   SCRATCH_STRUCT_PTR               SCRATCH_STRUCT_BASE;
   
   /* These fields are kept only for USB_shutdown() */
   void(_CODE_PTR_                  OLDISR_PTR)(pointer);
   void *                           OLDISR_DATA;
   uint_16                          USB_STATE;
   uint_16                          USB_DEVICE_STATE;
   uint_16                          USB_SOF_COUNT;
   uint_16                          DTD_ENTRIES;
   uint_16                          ERRORS;
   uint_16                          USB_DEV_STATE_B4_SUSPEND;
   uint_8                           DEV_NUM;             /* USB device number 
                                                         ** on the board 
                                                         */
   uint_8                           DEV_VEC;             /* Interrupt vector 
                                                         ** number for USB OTG 
                                                         */
   uint_8                           SPEED;               /* Low Speed, 
                                                         ** High Speed, 
                                                         ** Full Speed 
                                                         */
   uint_8                           MAX_ENDPOINTS;       /* Max endpoints
                                                         ** supported by this
                                                         ** device
                                                         */
                                                         
   uint_16                          USB_CURR_CONFIG;                                                         
   uint_8                           DEVICE_ADDRESS;
} USB_DEV_STATE_STRUCT, * USB_DEV_STATE_STRUCT_PTR;


/*
** Standard cache macros
*/
extern void _dcache_flush(void* addr, uint_32 linecnt, uint_32 linesize);

#define _PSP_BYTES_TO_LINES(b)              (((b)+(PSP_CACHE_LINE_SIZE-1)) / PSP_CACHE_LINE_SIZE)
#define _PSP_MASK_CACHE_ADDR(p)             (void*)(((uint_32)p) & ~(PSP_CACHE_LINE_SIZE - 1))

#define _DCACHE_FLUSH()			    	    /* Data cache is writethrough */
#define _DCACHE_FLUSH_LINE(p)		        /* Data cache is writethrough */
#define _DCACHE_FLUSH_MBYTES(p, m)	        /* Data cache is writethrough */

#if PSP_HAS_DATA_CACHE
    #define _DCACHE_INVALIDATE()                _mcf5227_dcache_invalidate()
    #define _DCACHE_INVALIDATE_LINE(p)          _DCACHE_INVALIDATE_MBYTES(p, 1)
    #define _DCACHE_INVALIDATE_MBYTES(p, m)     _dcache_flush(_PSP_MASK_CACHE_ADDR(p), _PSP_BYTES_TO_LINES(m), PSP_CACHE_LINE_SIZE)
#else
    #define _DCACHE_INVALIDATE()
    #define _DCACHE_INVALIDATE_LINE(p)
    #define _DCACHE_INVALIDATE_MBYTES(p, m)
#endif

#if PSP_HAS_DATA_CACHE

#define USB_dcache_invalidate()        	     _DCACHE_INVALIDATE()
#define USB_dcache_invalidate_line(p)        _DCACHE_INVALIDATE_LINE(p)
/* Based on the targets it should be modified, for coldfire it is MBYTES */
#define USB_dcache_invalidate_mlines(p,n)    _DCACHE_INVALIDATE_MBYTES(p, n)
#define USB_dcache_flush_line(p)             _DCACHE_FLUSH_LINE(p)
/* Based on the targets it should be modified, for coldfire it is MBYTES */
#define USB_dcache_flush_mlines(p,n)         _DCACHE_FLUSH_MBYTES(p, n)

#else

#define USB_dcache_invalidate()
#define USB_dcache_invalidate_line(p)
/* Based on the targets it should be modified, for coldfire it is MBYTES */
#define USB_dcache_invalidate_mlines(p,n)
#define USB_dcache_flush_line(p)
/* Based on the targets it should be modified, for coldfire it is MBYTES */
#define USB_dcache_flush_mlines(p,n)

#endif // PSP_HAS_DATA_CACHE

#endif

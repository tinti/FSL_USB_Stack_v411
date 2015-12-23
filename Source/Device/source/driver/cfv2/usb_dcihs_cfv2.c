/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 ******************************************************************************
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
 **************************************************************************//*!
 *
 * @file usb_dcihs_cfv2.c
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief The file contains CFV2 USB stack controller layer implementation.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "types.h"
#include "hidef.h"
#include "usb_dciapi_cfv2.h" /* USB DCI API Header File */
#include "usb_devapi.h"      /* USB Device API Header File */
#include "exceptions_cfv2.h"
#include "usb_dcihs_cfv2.h"
#include "usb_prvhs_cfv2.h"
#include <stdlib.h>
#include <string.h>

/*****************************************************************************
 * Constant and Macro's - None
 *****************************************************************************/
/****************************************************************************
 * Global Variables
 ****************************************************************************/
USB_DEV_STATE_STRUCT  usb_dev;
USB_DEV_STATE_STRUCT  *usb_dev_ptr;
static uint_8_ptr g_ep_recv_buff_ptr[USB_MAX_ENDPOINTS];
USB_PACKET_SIZE global_transfer_no;
/*****************************************************************************
 * Local Types - None
 *****************************************************************************/

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static void   USB_DCI_Init_Controller(uint_8 controller_ID);

static uint_8 USB_DCI_Add_dTD(uint_8 controller_ID, uint_8 ep_num, uint_8 direction, uint_8* buff_ptr, USB_PACKET_SIZE size);
static void   USB_DCI_Free_dTD(uint_8 controller_ID, void* dTD_ptr);

static void   USB_DCI_Process_Tr_Complete(uint_8 controller_ID);
static void   USB_DCI_Process_Reset(uint_8 controller_ID);
static void   USB_DCI_Process_Suspend(uint_8 controller_ID);
static void   USB_DCI_Process_SOF(uint_8 controller_ID);
static void   USB_DCI_Process_Port_Change(uint_8 controller_ID);
static void   USB_DCI_Process_Error(uint_8 controller_ID);

/*****************************************************************************
 * Local Variables - None
 *****************************************************************************/

/*****************************************************************************
 * Local Functions
 *****************************************************************************/
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_usbhs_add_dTD
*  Returned Value : USB_OK or error code
*  Comments       :
*        Adds a device transfer desriptor(s) to the queue.
*
*END*-----------------------------------------------------------------*/
static uint_8 USB_DCI_Add_dTD
(
    uint_8          controller_ID,  /* [IN] Controller ID */
    uint_8          ep_num,         /* [IN] Endpoint number */
    uint_8          direction,      /* [IN] Direction (send / receive) */
    uint_8*         buff_ptr,       /* [IN/OUT] Application buffer pointer */
    USB_PACKET_SIZE size            /* [IN] Size of the buffer */
)
{   /* Body */
    volatile USBHS_EP_TR_STRUCT_PTR   dTD_ptr, temp_dTD_ptr, first_dTD_ptr = NULL;
    volatile USBHS_EP_QUEUE_HEAD_STRUCT_PTR ep_queue_head_ptr;
    uint_32 curr_pkt_len, remaining_len, curr_offset, bit_pos;
    uint_8 temp;

    /*********************************************************************
     For a optimal implementation, we need to detect the fact that
     we are adding DTD to an empty list. If list is empty, we can
     actually skip several programming steps esp. those for ensuring
     that there is no race condition.The following boolean will be useful
     in skipping some code here.
     *********************************************************************/
    boolean   list_empty = FALSE;
    #ifdef TRIP_WIRE
    boolean   read_safe;   
    #endif
    UNUSED(controller_ID)
     
    remaining_len = size;

    curr_offset = 0;
    temp = (uint_8)(2*ep_num + direction);
    bit_pos = (uint_32)(1 << (16 * direction + ep_num));

    ep_queue_head_ptr = (USBHS_EP_QUEUE_HEAD_STRUCT_PTR)MCF_USB_OTG_EPLISTADDR +
        temp;
    
    #if PSP_HAS_DATA_CACHE
        USB_dcache_flush_mlines((void *)ep_queue_head_ptr,
            sizeof(USBHS_EP_QUEUE_HEAD_STRUCT));
    #endif
   
   /*********************************************************************
    This loops iterates through the length of the transfer and divides
    the data in to DTDs each handling the a max of 0x4000 bytes of data.
    The first DTD in the list is stored in a pointer called first_dTD_ptr.
    This pointer is later linked in to QH for processing by the hardware.
    *********************************************************************/

    do
    {
        /* Check if we need to split the transfer into multiple dTDs */
        if(remaining_len > VUSB_EP_MAX_LENGTH_TRANSFER)
        {
            curr_pkt_len = VUSB_EP_MAX_LENGTH_TRANSFER;
        }
        else
        {
            curr_pkt_len = remaining_len;
        } 

        remaining_len -= curr_pkt_len;

        /* Get a dTD from the queue */
        EHCI_DTD_QGET(usb_dev_ptr->DTD_HEAD, usb_dev_ptr->DTD_TAIL, dTD_ptr);

        if (!dTD_ptr)
        {
            return USBERR_TR_FAILED;
        } 

        usb_dev_ptr->DTD_ENTRIES--;

        if (curr_offset == 0)
        {
            first_dTD_ptr = dTD_ptr;
        } 

        #if PSP_HAS_DATA_CACHE
            /**************************************************************
             USB Memzero does not bypass the cache and hence we must use
             DTD pointer to update the memory and bypass the cache. If
             your DTD are allocated from an uncached regigio, you can
             eliminitate this approach and switch back to USB_mem_zero().
             **************************************************************/
            dTD_ptr->NEXT_TR_ELEM_PTR   = 0;
            dTD_ptr->SIZE_IOC_STS       = 0;
            dTD_ptr->BUFF_PTR0          = 0;
            dTD_ptr->BUFF_PTR1          = 0;
            dTD_ptr->BUFF_PTR2          = 0;
            dTD_ptr->BUFF_PTR3          = 0;
            dTD_ptr->BUFF_PTR4          = 0;
        #else
            /* Zero the dTD. Leave the last 4 bytes as that is the 
               scratch pointer*/
            (void)memset((void *) dTD_ptr, 0x00, (sizeof(USBHS_EP_TR_STRUCT) - 4));
        #endif

        /* Initialize the dTD */
        //dTD_ptr->SCRATCH_PTR->controller_ID = controller_ID;
        //dTD_ptr->SCRATCH_PTR->ep_num = ep_num;
        //dTD_ptr->SCRATCH_PTR->direction = direction;
        dTD_ptr->SCRATCH_PTR->buff_ptr = buff_ptr;
        dTD_ptr->SCRATCH_PTR->size = size;
        dTD_ptr->SCRATCH_PTR->transfer_no=global_transfer_no;

        /* Set the Terminate bit */
        dTD_ptr->NEXT_TR_ELEM_PTR = VUSB_EP_QUEUE_HEAD_NEXT_TERMINATE;

        /*************************************************************
         FIX ME: For hig-speed and high-bandwidth ISO IN endpoints,
         we must initialize the multiplied field so that Host can issues
         multiple IN transactions on the endpoint. See the DTD data
         structure for MultiIO field.

         S Garg 11/06/2003
         *************************************************************/

        /* Set the reserved field to 0 */
        dTD_ptr->SIZE_IOC_STS &= ~USBHS_TD_RESERVED_FIELDS;

        /* 4K apart buffer page pointers */
        dTD_ptr->BUFF_PTR0 = (uint_32)(buff_ptr + curr_offset);
        dTD_ptr->BUFF_PTR1 = (dTD_ptr->BUFF_PTR0 + 4096);
        dTD_ptr->BUFF_PTR2 = (dTD_ptr->BUFF_PTR1 + 4096);
        dTD_ptr->BUFF_PTR3 = (dTD_ptr->BUFF_PTR2 + 4096);
        dTD_ptr->BUFF_PTR4 = (dTD_ptr->BUFF_PTR3 + 4096);

        curr_offset += curr_pkt_len;
        #if PSP_HAS_DATA_CACHE
            USB_dcache_flush_mlines((void *)dTD_ptr,sizeof(USBHS_EP_TR_STRUCT));
        #endif 
        /* Fill in the transfer size */
        if (!remaining_len)
        {
            dTD_ptr->SIZE_IOC_STS = ((curr_pkt_len <<
                USBHS_TD_LENGTH_BIT_POS) | (USBHS_TD_IOC) |
                (USBHS_TD_STATUS_ACTIVE));
        }
        else
        {
            dTD_ptr->SIZE_IOC_STS = ((curr_pkt_len << USBHS_TD_LENGTH_BIT_POS)
                | USBHS_TD_STATUS_ACTIVE);
        } 
        #if PSP_HAS_DATA_CACHE
            USB_dcache_flush_mlines((void *)dTD_ptr,sizeof(USBHS_EP_TR_STRUCT));
        #endif
        /* Maintain the first and last device transfer descriptor per
           endpoint and direction */
        if (!usb_dev_ptr->EP_DTD_HEADS[temp])
        {
            usb_dev_ptr->EP_DTD_HEADS[temp] = dTD_ptr;
            /***********************************************
             If list does not have a head, it means that list
             is empty. An empty condition is detected.
             ***********************************************/
            list_empty = TRUE;
        } 

        /* Check if the transfer is to be queued at the end or beginning */
        temp_dTD_ptr = usb_dev_ptr->EP_DTD_TAILS[temp];

        /* New tail */
        usb_dev_ptr->EP_DTD_TAILS[temp] = dTD_ptr;

        if (temp_dTD_ptr)
        {
            /* Should not do |=. The Terminate bit should be zero */
            temp_dTD_ptr->NEXT_TR_ELEM_PTR = (uint_32)dTD_ptr;
        } 
      
    } while (remaining_len); /* EndWhile */

    global_transfer_no++;

    /**************************************************************
     In the loop above DTD has already been added to the list
     However endpoint has not been primed yet. If list is not empty
     we need safer ways to add DTD to the
     existing list. Else we just skip to adding DTD to QH safely.
     **************************************************************/
    if(list_empty)/* If List is Empty : case 1*/
    {
        /* No other transfers on the queue */
        /* Step 1 of Executing a Transfer Descriptor documentation */
        ep_queue_head_ptr->NEXT_DTD_PTR = (uint_32)first_dTD_ptr;
        /* Step 2 of Executing a Transfer Descriptor documentation */
        ep_queue_head_ptr->SIZE_IOC_INT_STS = 0; 

        /* Prime the Endpoint */
        /* Step 3 of Executing a Transfer Descriptor documentation */
        MCF_USB_OTG_EPPRIME = bit_pos;
    }
    else /* If list is not empty : case 2*/
    {
        #ifdef TRIP_WIRE
            /*********************************************************
             Hardware v3.2+ require the use of semaphore to ensure that
             QH is safely updated.
             *********************************************************/

            /*********************************************************
             Check the prime bit. If set return USB_OK
             *********************************************************/
            if (MCF_USB_OTG_EPPRIME & bit_pos)
            {
                return USB_OK;
            }

            read_safe = FALSE;
            while(!read_safe)
            {
                /*********************************************************
                 start with setting the semaphores
                 *********************************************************/
                MCF_USB_OTG_USBCMD |= EHCI_CMD_ATDTW_TRIPWIRE_SET;

                /*********************************************************
                 Read the endpoint status
                 *********************************************************/
                if(MCF_USB_OTG_USBCMD & EHCI_CMD_ATDTW_TRIPWIRE_SET)
                {
                    read_safe = TRUE;
                }
            }/*end while loop */

            /*********************************************************
             Clear the semaphore
             *********************************************************/
            MCF_USB_OTG_USBCMD &= EHCI_CMD_ATDTW_TRIPWIRE_CLEAR;
        #else   /*workaround old method */
            /* Start CR 1015 */
            /* Prime the Endpoint */
            MCF_USB_OTG_EPPRIME = bit_pos;

            if(!(MCF_USB_OTG_EPSR & bit_pos))
            {
               /* old workaround will be compiled */
               while(MCF_USB_OTG_EPPRIME & bit_pos)
               {
                    /* Wait for the ENDPTPRIME to go to zero */
               } 

               if(MCF_USB_OTG_EPSR & bit_pos)
               {
                    /* The endpoint was not not primed so no other transfers on
                       the queue */
                    return USB_OK;
               }
            }
            else
            {
                return USB_OK;
            } 
          
            #if PSP_HAS_DATA_CACHE
                USB_dcache_invalidate_mlines((void *)first_dTD_ptr,
                    sizeof(USBHS_EP_TR_STRUCT));
            #endif
           
            /* No other transfers on the queue */
            ep_queue_head_ptr->NEXT_DTD_PTR = (uint_32)first_dTD_ptr;
            ep_queue_head_ptr->SIZE_IOC_INT_STS = 0;
         
            #if PSP_HAS_DATA_CACHE
                USB_dcache_flush_mlines((void *)ep_queue_head_ptr,
                    sizeof(USBHS_EP_QUEUE_HEAD_STRUCT));
            #endif

            /* Prime the Endpoint */
            MCF_USB_OTG_EPPRIME = bit_pos;
        #endif
    }
    return USB_OK;
   /* End CR 1015 */
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : USB_DCI_Free_dTD
*  Returned Value : void
*  Comments       :
*        Enqueues a dTD onto the free DTD ring.
*
*END*-----------------------------------------------------------------*/
static void USB_DCI_Free_dTD
(
	uint_8    controller_ID,
    /* [IN] the dTD to enqueue */
    void*  dTD_ptr
)
{   /* Body */

   (void)controller_ID;
    /*
    ** This function can be called from any context, and it needs mutual
    ** exclusion with itself.
    */
    DisableInterrupts;

    /*
    ** Add the dTD to the free dTD queue (linked via PRIVATE) and
    ** increment the tail to the next descriptor
    */
    EHCI_DTD_QADD(usb_dev_ptr->DTD_HEAD, usb_dev_ptr->DTD_TAIL,
        (USBHS_EP_TR_STRUCT_PTR)dTD_ptr);
    usb_dev_ptr->DTD_ENTRIES++;
    
    EnableInterrupts;
} /* Endbody */ 
 
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : USB_DCI_InitController
*  Returned Value : USB_OK or error code
*  Comments       :
*        Initializes the USB device controller.
*
*END*-----------------------------------------------------------------*/
static void USB_DCI_Init_Controller(uint_8 controller_ID)
{   /* Body */
    volatile USBHS_EP_QUEUE_HEAD_STRUCT_PTR   ep_queue_head_ptr;
    volatile USBHS_EP_TR_STRUCT_PTR           dTD_ptr;
    uint_8                                    temp,i;
    SCRATCH_STRUCT_PTR                        temp_scratch_ptr;
    volatile uint_16_ptr                      pUOCSR;

    DisableInterrupts;

    /* Program the controller to be the USB device controller */
	MCF_USB_OTG_USBMODE = ( MCF_USB_OTG_USBMODE_CM_DEVICE | MCF_USB_OTG_USBMODE_ES | 
          MCF_USB_OTG_USBMODE_SLOM);

    temp = (uint_8)(usb_dev_ptr->MAX_ENDPOINTS * 2);

    /* Initialize the internal dTD head and tail to NULL */
    usb_dev_ptr->DTD_HEAD = NULL;
    usb_dev_ptr->DTD_TAIL = NULL;

    /* Make sure the 16 MSBs of this register are 0s */
    MCF_USB_OTG_EPSETUPSR = 0;

    ep_queue_head_ptr = usb_dev_ptr->EP_QUEUE_HEAD_PTR;

    /* Initialize all device queue heads */
    for (i=0;i<temp;i++)
    {
        /* Interrupt on Setup packet */
        (ep_queue_head_ptr + i)->MAX_PKT_LENGTH =
            ((uint_32)USB_MAX_CTRL_PAYLOAD << 
                VUSB_EP_QUEUE_HEAD_MAX_PKT_LEN_POS)
            | VUSB_EP_QUEUE_HEAD_IOS;
        (ep_queue_head_ptr + i)->NEXT_DTD_PTR = 
            VUSB_EP_QUEUE_HEAD_NEXT_TERMINATE;
    } 
   
    #if PSP_HAS_DATA_CACHE
        USB_dcache_flush_mlines((void *)ep_queue_head_ptr,
            ((sizeof(USBHS_EP_QUEUE_HEAD_STRUCT))*temp));
    #endif
       
    /* Configure the Endpoint List Address */
    MCF_USB_OTG_EPLISTADDR = (uint_32)ep_queue_head_ptr;

    dTD_ptr = usb_dev_ptr->DTD_ALIGNED_BASE_PTR;
    usb_dev_ptr->DTD_ENTRIES = 0;

    temp_scratch_ptr = usb_dev_ptr->SCRATCH_STRUCT_BASE;

    /* Enqueue all the dTDs */
    for (i=0;i<MAX_EP_TR_DESCRS;i++)
    {
        dTD_ptr->SCRATCH_PTR = temp_scratch_ptr;        
        /* Set the dTD to be invalid */
        dTD_ptr->NEXT_TR_ELEM_PTR = USBHS_TD_NEXT_TERMINATE;
        /* Set the Reserved fields to 0 */
        dTD_ptr->SIZE_IOC_STS &= ~USBHS_TD_RESERVED_FIELDS;
        dTD_ptr->SCRATCH_PTR->PRIVATE = (void *) usb_dev_ptr;
        USB_DCI_Free_dTD(controller_ID, (void *)dTD_ptr);
        dTD_ptr++;
        temp_scratch_ptr++;
    }

    /*--- Intial Configuration ---*/
    /* Initialize the endpoint 0 properties */
    MCF_USB_OTG_EPCR(0) = (MCF_USB_OTG_EPCR_TXR | MCF_USB_OTG_EPCR_RXR);
    MCF_USB_OTG_EPCR(0) &= ~(MCF_USB_OTG_EPCR_TXS | MCF_USB_OTG_EPCR_RXS);

    /* Enable interrupts */
    /* There is no need to enable SOF Interrupt as its generated automatically
       by hardware irrespective of device attachment status */
    MCF_USB_OTG_USBINTR =
        (MCF_USB_OTG_USBINTR_UE | MCF_USB_OTG_USBINTR_UEE |
        MCF_USB_OTG_USBINTR_PCE | MCF_USB_OTG_USBINTR_URE | 
        MCF_USB_OTG_USBINTR_SLE);

    /* Enable the interrupts */
    EnableInterrupts;

    usb_dev_ptr->USB_STATE = USB_STATE_UNKNOWN;

    /* Set the Run bit in the command register */
    MCF_USB_OTG_USBCMD = MCF_USB_OTG_USBCMD_RS;

    /* Enable a valid B session to allow device to connect to a host*/
    pUOCSR = (uint_16*)&MCF_CCM_UOCSR;
    *pUOCSR |= MCF_CCM_UOCSR_BVLD;  
    
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : USB_DCI_Process_Tr_Complete
*  Returned Value : None
*  Comments       :
*        Services transaction complete interrupt
*
*END*-----------------------------------------------------------------*/
void USB_DCI_Process_Tr_Complete(uint_8 controller_ID)
{   /* Body */
    volatile USBHS_EP_TR_STRUCT_PTR   dTD_ptr;
    volatile USBHS_EP_TR_STRUCT_PTR   temp_dTD_ptr;
    volatile USBHS_EP_QUEUE_HEAD_STRUCT_PTR ep_queue_head_ptr;
    uint_8  temp, i, ep_num, direction;
    uint_32  remaining_length = 0;
    uint_32  actual_transfer_length = 0;
    uint_32  errors = 0;
    
    //volatile XD_STRUCT_PTR  xd_ptr;
    //volatile XD_STRUCT_PTR  temp_xd_ptr = NULL;
    
    uint_8_ptr buff_start_address = NULL;

    USB_DEV_EVENT_STRUCT event;
    uint_32 max_packet_size, bit_pos;

    DisableInterrupts
    
    (void)memset(&event, 0x00, sizeof(USB_DEV_EVENT_STRUCT)); /* Initialize event structure */
    
    event.controller_ID = controller_ID;

    /* We use separate loops for ENDPTSETUPSTAT and ENDPTCOMPLETE because the
       setup packets are to be read ASAP */
    /* Process all Setup packet received interrupts */
    bit_pos = MCF_USB_OTG_EPSETUPSR;

    if (bit_pos)
    {
        for (i=0;i<16;i++)
        {
            if (bit_pos & (1 << i))
            {
                event.ep_num = USB_CONTROL_ENDPOINT;
                event.setup = TRUE;
                event.direction = USB_RECV;
                event.len = USB_SETUP_PKT_SIZE;
                event.buffer_ptr = (uint_8*)malloc(USB_SETUP_PKT_SIZE);
                if(event.buffer_ptr == NULL)
                {
                    return;
                }
                
                (void)USB_DCI_Get_Setup_Data(&controller_ID, USB_CONTROL_ENDPOINT, event.buffer_ptr);                 
                (void)USB_Device_Call_Service(i, &event);
                                
                free(event.buffer_ptr);
             } /* Endif */
         } /* Endfor */
    } /* Endif */

   /* Don't clear the endpoint setup status register here. It is cleared as a
      setup packet is read out of the buffer */

    /* Process non-setup transaction complete interrupts */
    bit_pos = MCF_USB_OTG_EPCOMPLETE;

    /* Clear the bits in the register */
    MCF_USB_OTG_EPCOMPLETE = bit_pos;

    if (bit_pos)
    {
        /* Get the endpoint number and the direction of transfer */
        for (i=0;i<32;i++)
        {
            if (bit_pos & (1 << i))
            {
                if (i > 15)
                {
                    ep_num = (uint_8)(i - 16);
                    direction = 1;
                }
                else
                {
                    ep_num = i;
                    direction = 0;
                } /* Endif */

                temp = (uint_8)(2*ep_num + direction);

                /* Get the first dTD */
                dTD_ptr = usb_dev_ptr->EP_DTD_HEADS[temp];

                ep_queue_head_ptr = (USBHS_EP_QUEUE_HEAD_STRUCT_PTR)MCF_USB_OTG_EPLISTADDR +
                temp;
                
                max_packet_size = (ep_queue_head_ptr->MAX_PKT_LENGTH >> 16) & 0x000007FF;
            
                #if PSP_HAS_DATA_CACHE
                    _DCACHE_INVALIDATE_MBYTES((void *)ep_queue_head_ptr,
                        sizeof(USBHS_EP_QUEUE_HEAD_STRUCT));
                #endif
                
                /* Process all the dTDs for respective transfers */
                while (dTD_ptr)
                { 
                    if (dTD_ptr->SIZE_IOC_STS & USBHS_TD_STATUS_ACTIVE)
                    {
                        /*No more dTDs to process. Next one is owned by VUSB*/
                        break;
                    } /* Endif */

                    /* Get the correct internal transfer descriptor */
                    buff_start_address = dTD_ptr->SCRATCH_PTR->buff_ptr;
                    actual_transfer_length = dTD_ptr->SCRATCH_PTR->size;
                    event.buffer_ptr = dTD_ptr->SCRATCH_PTR->buff_ptr;

                    /* Get the address of the next dTD */
                    temp_dTD_ptr = (USBHS_EP_TR_STRUCT_PTR)(dTD_ptr->NEXT_TR_ELEM_PTR & USBHS_TD_ADDR_MASK);

                    /* Read the errors */
                    errors = (dTD_ptr->SIZE_IOC_STS & USBHS_TD_ERROR_MASK);

                    if (!errors)
                    {
                        /* No errors */
                        /* Get the length of transfer from the current dTD */
                        remaining_length += ((dTD_ptr->SIZE_IOC_STS & 
                            VUSB_EP_TR_PACKET_SIZE) >> 16);
                        actual_transfer_length -= remaining_length;
                    }
                    else
                    {
                        if (errors & USBHS_TD_STATUS_HALTED)
                        {
                            /* Clear the errors and Halt condition */
                            ep_queue_head_ptr->SIZE_IOC_INT_STS &= ~errors;
                        } /* Endif */
                    } /* Endif */

                    /* Retire the processed dTD */
                    (void)USB_DCI_Cancel_Transfer(&controller_ID, ep_num, direction);
                    event.ep_num = ep_num;
                    event.setup = FALSE;
                    event.direction = direction;
                    /* if wtotallength is greater tahn max_packet_size, 
                       then it marks split transaction */  
                    event.len = (USB_PACKET_SIZE)((dTD_ptr->SCRATCH_PTR->size > max_packet_size)?
                        dTD_ptr->SCRATCH_PTR->size : actual_transfer_length);
                    event.len = (USB_PACKET_SIZE)actual_transfer_length;
                    
                    if (temp_dTD_ptr)
                    {
                        //if((uint_32)event.buffer_ptr != (uint_32)temp_dTD_ptr->SCRATCH_PTR->buff_ptr)
                        if(dTD_ptr->SCRATCH_PTR->transfer_no != (uint_32)temp_dTD_ptr->SCRATCH_PTR->transfer_no)
                        {
                            /* Transfer complete. Call the register service 
                               function for the endpoint */
                            (void)USB_Device_Call_Service(ep_num,&event);
                            remaining_length = 0;
                        } /* Endif */
                        /* else -> same buffer (start address) on the next dTD. The transfer is not finished yet! */
                    }
                    else
                    {
                        /* Transfer complete. Call the register service 
                           function for the endpoint */
                        (void)USB_Device_Call_Service(ep_num,&event);
                    } /* Endif */
               
                    dTD_ptr = temp_dTD_ptr;
                    errors = 0;
                } /* Endwhile */
            } /* Endif */
        } /* Endfor */
    }/* Endif */
    EnableInterrupts
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : USB_DCI_Process_Reset
*  Returned Value : None
*  Comments       :
*        Services reset interrupt
*
*END*-----------------------------------------------------------------*/
static void USB_DCI_Process_Reset(uint_8 controller_ID)
{   /* Body */
    uint_32                    temp;
    uint_8                     cnt=0;

    USB_DEV_EVENT_STRUCT           event;

    event.controller_ID = controller_ID;
    event.ep_num = USB_CONTROL_ENDPOINT;
    event.setup = FALSE;
    event.direction = USB_RECV;
    event.buffer_ptr = NULL;
    event.len = ZERO_LENGTH;

    /* De-Init All the End Point.  */
    for (cnt = 0; cnt < USB_MAX_ENDPOINTS; cnt++)
    {
        USB_DCI_Deinit_EndPoint(controller_ID,cnt,USB_RECV);
        USB_DCI_Deinit_EndPoint(controller_ID,cnt,USB_SEND);
    }

    /* The address bits are past bit 25-31. Set the address */
    MCF_USB_OTG_DEVICEADDR &= ~0xFE000000;

    /* Clear all the setup token semaphores */
    temp = MCF_USB_OTG_EPSETUPSR;
    MCF_USB_OTG_EPSETUPSR = temp;

    /* Clear all the endpoint complete status bits */
    temp = MCF_USB_OTG_EPCOMPLETE;
    MCF_USB_OTG_EPCOMPLETE = temp;

    while (MCF_USB_OTG_EPPRIME & 0xFFFFFFFF)
    {
        /* Wait until all ENDPTPRIME bits cleared */
    } /* Endif */

    /* Write 1s to the Flush register */
    MCF_USB_OTG_EPFLUSH = 0xFFFFFFFF;

    if (MCF_USB_OTG_PORTSC1 & MCF_USB_OTG_PORTSC1_PR)
    {
        usb_dev_ptr->BUS_RESETTING = TRUE;
        usb_dev_ptr->USB_STATE = USB_STATE_POWERED;
    }
    else
    {
        /* re-initialize */
        USB_DCI_Init_Controller(controller_ID);
    } /* Endif */

    /* Inform the application so that it can cancel all previously queued transfers */
    USB_Device_Call_Service(USB_SERVICE_BUS_RESET, &event);
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : USB_DCI_Process_Suspend
*  Returned Value : None
*  Comments       :
*        Services suspend interrupt
*
*END*-----------------------------------------------------------------*/
static void USB_DCI_Process_Suspend(uint_8 controller_ID)
{   /* Body */
    USB_DEV_EVENT_STRUCT          event;

    usb_dev_ptr->USB_DEV_STATE_B4_SUSPEND = usb_dev_ptr->USB_STATE;

    usb_dev_ptr->USB_STATE = USB_STATE_SUSPEND;

    /* Initialize the event strucutre to be passed to the upper layer*/
    event.controller_ID = controller_ID;
    event.ep_num = (uint_8)-1;
    event.setup = 0;
    event.direction = 0;
    event.buffer_ptr = (uint_8*)NULL;
    event.len = 0;
    /* Inform the upper layers */
    (void)USB_Device_Call_Service(USB_SERVICE_SLEEP, &event);

} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_usbhs_process_SOF
*  Returned Value : None
*  Comments       :
*        Services SOF interrupt
*
*END*-----------------------------------------------------------------*/
static void USB_DCI_Process_SOF(uint_8 controller_ID)
{   /* Body */
    USB_DEV_EVENT_STRUCT      event;
    uint_32 fr_index = MCF_USB_OTG_FRINDEX;

    /* Initialize the event strucutre to be passed to the upper layer*/
    event.controller_ID = controller_ID;
    event.ep_num = (uint_8)-1;
    event.setup = 0;
    event.direction = 0;
    event.len = sizeof(fr_index);
    event.buffer_ptr = (uint_8*)&fr_index;
        
    /* Inform the upper layer */
    /* event.buffer_ptr is valid only in USB_DCI_Process_SOF function ontext */
    (void)USB_Device_Call_Service(USB_SERVICE_SOF,&event);     
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_usbhs_process_port_change
*  Returned Value : None
*  Comments       :
*        Services port change detect interrupt
*
*END*-----------------------------------------------------------------*/
static void USB_DCI_Process_Port_Change(uint_8 controller_ID)
{   /* Body */
    USB_DEV_EVENT_STRUCT      event;

    /* Initialize the event strucutre to be passed to the upper layer*/
    event.controller_ID = controller_ID;
    event.ep_num = (uint_8)-1;
    event.setup = 0;
    event.direction = 0;

    if (usb_dev_ptr->BUS_RESETTING)
    {
        /* Bus reset operation complete */
        usb_dev_ptr->BUS_RESETTING = FALSE;
    } /* Endif */

    if (!(MCF_USB_OTG_PORTSC1 & MCF_USB_OTG_PORTSC1_PR))
    {
        /* Get the speed */
        if (MCF_USB_OTG_PORTSC1 & MCF_USB_OTG_PORTSC1_HSP)
        {
            usb_dev_ptr->SPEED = USB_SPEED_HIGH;
        } 
        else 
        {
           if (MCF_USB_OTG_PORTSC1 & MCF_USB_OTG_PORTSC1_PSPD_LOW)
           {           	
            usb_dev_ptr->SPEED = USB_SPEED_LOW;
           }
           else
           {
            usb_dev_ptr->SPEED = USB_SPEED_FULL;
           }
        } /* Endif */

        /* Inform the upper layers of the speed of operation */
        (void)USB_Device_Call_Service(USB_SERVICE_SPEED_DETECTION, &event);

    } /* Endif */

    if (MCF_USB_OTG_PORTSC1 & MCF_USB_OTG_PORTSC1_SUSP)
    {
        usb_dev_ptr->USB_DEV_STATE_B4_SUSPEND = usb_dev_ptr->USB_STATE;
        usb_dev_ptr->USB_STATE = USB_STATE_SUSPEND;

        event.len = ZERO_LENGTH;
        event.buffer_ptr = (uint_8*)NULL;
        /* Inform the upper layers */
        (void)USB_Device_Call_Service(USB_SERVICE_SUSPEND, &event);
    } /* Endif */

    if(!(MCF_USB_OTG_PORTSC1 & MCF_USB_OTG_PORTSC1_SUSP) &&
        (usb_dev_ptr->USB_STATE == USB_STATE_SUSPEND))
    {
        usb_dev_ptr->USB_STATE = usb_dev_ptr->USB_DEV_STATE_B4_SUSPEND;

        event.len = ZERO_LENGTH;
        event.buffer_ptr = (uint_8*)NULL;
        /* Inform the upper layers */
        (void)USB_Device_Call_Service(USB_SERVICE_RESUME, &event);

        return;
    } /* Endif */

    usb_dev_ptr->USB_STATE = USB_STATE_DEFAULT;
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_usbhs_process_error
*  Returned Value : None
*  Comments       :
*        Services error interrupt
*
*END*-----------------------------------------------------------------*/
static void USB_DCI_Process_Error(uint_8 controller_ID)
{   /* Body */
   UNUSED(controller_ID)
    /* Increment the error count */
    usb_dev_ptr->ERRORS++;
} /* EndBody */

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

/**************************************************************************//*!
 *
 * @name  USB_DCI_Init
 *
 * @brief The function initializes the Controller layer
 *
 * @param controller_ID : Controller ID
 *
 * @return status
 *         USB_OK    : Always
 ******************************************************************************
 * Initializes the USB controller
 *****************************************************************************/
uint_8 USB_DCI_Init (
    uint_8    controller_ID,   /* [IN] Controller ID */
    uint_8    bVregEn         /* Enables or disables internal regulator */
)
{  
    uint_8                             temp;
    uint_32                            total_memory=0;
    uint_8*                            driver_memory;
    usb_dev_ptr = (USB_DEV_STATE_STRUCT*)&usb_dev;

    (void)controller_ID;
    (void)bVregEn;


    /* Get the maximum number of endpoints supported by this USB controller */
    usb_dev_ptr->MAX_ENDPOINTS = (uint_8)(MCF_USB_OTG_DCCPARAMS & USBHS_MAX_ENDPTS_SUPPORTED);

    temp = (uint_8)(usb_dev_ptr->MAX_ENDPOINTS * 2);

    /****************************************************************
     Consolidated memory allocation
     ****************************************************************/
    total_memory = ((temp * sizeof(USBHS_EP_QUEUE_HEAD_STRUCT)) + 2048) +
                   ((MAX_EP_TR_DESCRS * sizeof(USBHS_EP_TR_STRUCT)) + 32) +
                   (sizeof(SCRATCH_STRUCT)*MAX_EP_TR_DESCRS);

    driver_memory = (uint_8*)malloc(total_memory);


    if (driver_memory == NULL)
    {
        return USBERR_ALLOC;
    }

    /****************************************************************
     Zero out the memory allocated
    ****************************************************************/
    (void)memset((void *)driver_memory, 0x00, total_memory);

    #if PSP_HAS_DATA_CACHE
    /****************************************************************
     Flush the zeroed memory if cache is enabled
     ****************************************************************/
        USB_dcache_flush_mlines((void*)driver_memory,total_memory);
    #endif

    /****************************************************************
     Keep a pointer to driver memory alloctaion
     ****************************************************************/
    usb_dev_ptr->DRIVER_MEMORY = driver_memory;
    usb_dev_ptr->TOTAL_MEMORY = total_memory;
    /****************************************************************
     Assign QH base
     ****************************************************************/
    usb_dev_ptr->EP_QUEUE_HEAD_BASE = (USBHS_EP_QUEUE_HEAD_STRUCT_PTR)\
                                     driver_memory;
    driver_memory += ((temp * sizeof(USBHS_EP_QUEUE_HEAD_STRUCT)) + 2048);

    /* Align the endpoint queue head to 2K boundary */
    usb_dev_ptr->EP_QUEUE_HEAD_PTR = (USBHS_EP_QUEUE_HEAD_STRUCT_PTR)
        USB_MEM2048_ALIGN((uint_32)usb_dev_ptr->EP_QUEUE_HEAD_BASE);
        
    /****************************************************************
     Assign DTD base
     ****************************************************************/
    usb_dev_ptr->DTD_BASE_PTR = (USBHS_EP_TR_STRUCT_PTR) \
                                driver_memory;
    driver_memory += ((MAX_EP_TR_DESCRS * sizeof(USBHS_EP_TR_STRUCT)) + 32);

    /* Align the dTD base to 32 byte boundary */
    usb_dev_ptr->DTD_ALIGNED_BASE_PTR = (USBHS_EP_TR_STRUCT_PTR)
        USB_MEM32_ALIGN((uint_32)usb_dev_ptr->DTD_BASE_PTR);

    /****************************************************************
     Assign SCRATCH Structure base
     ****************************************************************/
    /* Allocate memory for internal scratch structure */
    usb_dev_ptr->SCRATCH_STRUCT_BASE = (SCRATCH_STRUCT_PTR)driver_memory;

    usb_dev_ptr->USB_STATE = USB_STATE_UNKNOWN;

    USB_DCI_Init_Controller(controller_ID);
    
    return USB_OK;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Init_EndPoint
 *
 * @brief The function initializes an endpoint
 *
 * @param controller_ID : Controller ID
 * @param ep_ptr        : Pointer to EndPoint Structures
 * @param flag          : Zero Termination
 *
 * @return status
 *         USB_OK                    : When Successfull
 *         USBERR_EP_INIT_FAILED     : When Error
 ******************************************************************************
 *
 * This function initializes an endpoint and the Buffer Descriptor Table
 * entry associated with it. In case the input parameters are invalid it will
 * return USBERR_EP_INIT_FAILED error.
 *
 *****************************************************************************/
uint_8 USB_DCI_Init_EndPoint (
    uint_8               controller_ID,/* [IN] Controller ID */
    USB_EP_STRUCT_PTR    ep_ptr,       /* [IN] Pointer to Endpoint structure,                                               
                                               (endpoint number, 
                                                endpoint type, 
                                                endpoint direction,
                                                max packet size) */
    boolean              flag          /* [IN] Zero Termination (DON'T ZERO TERMINATE) */
)
{    
    volatile USBHS_EP_QUEUE_HEAD_STRUCT *ep_queue_head_ptr;
    uint_32                              bit_pos;
    uint_8                               max_pkts_per_uframe;   
    
    flag = TRUE;
       
    max_pkts_per_uframe = (uint_8)(((uint_8)flag & USB_MAX_PKTS_PER_UFRAME) >> 1);

    if((ep_ptr->type > USB_INTERRUPT_PIPE) || 
        (ep_ptr->direction > USB_SEND))
    {
        return USBERR_EP_INIT_FAILED;
    }
    
    /* before initializing cancel all transfers on EP as there may be calls
       for endpoint initialization more than once. This will free any allocated
       queue */
    (void)USB_DCI_Cancel_Transfer(&controller_ID, ep_ptr->ep_num,
         ep_ptr->direction);

    /* Get the endpoint queue head address */
    ep_queue_head_ptr = (USBHS_EP_QUEUE_HEAD_STRUCT_PTR)MCF_USB_OTG_EPLISTADDR +
        2*ep_ptr->ep_num + ep_ptr->direction;

    bit_pos = (uint_32)(1 << (16 * ep_ptr->direction + ep_ptr->ep_num));

    /* Check if the Endpoint is Primed */
    if((!(MCF_USB_OTG_EPPRIME & bit_pos)) &&
       (!(MCF_USB_OTG_EPSR & bit_pos)))
    {
        /* Set the max packet length, interrupt on Setup and Mult fields */
        if (ep_ptr->type == USB_ISOCHRONOUS_PIPE)
        {
            /* Mult bit should be set for isochronous endpoints */
            ep_queue_head_ptr->MAX_PKT_LENGTH = (uint_32)((uint_32)((ep_ptr->size & 0x0000FFFF) << 16)|
               ((max_pkts_per_uframe ? max_pkts_per_uframe : 1)
                << VUSB_EP_QUEUE_HEAD_MULT_POS));
            #if PSP_HAS_DATA_CACHE
                USB_dcache_flush_mlines((void *)ep_queue_head_ptr,
                    sizeof(USBHS_EP_QUEUE_HEAD_STRUCT));
            #endif                
        }
        else
        {
            if (ep_ptr->type != USB_CONTROL_PIPE)
            {
                ep_queue_head_ptr->MAX_PKT_LENGTH = (uint_32)
                    ((uint_32)((ep_ptr->size & 0x0000FFFF) << 16) | 
                     (flag ? VUSB_EP_QUEUE_HEAD_ZERO_LEN_TER_SEL : 0));
                if(ep_ptr->direction == USB_RECV)
                {
                    g_ep_recv_buff_ptr[ep_ptr->ep_num] = (uint_8*)malloc(ep_ptr->size);                   
                    
                    if(g_ep_recv_buff_ptr[ep_ptr->ep_num] == NULL)
                    {
                        return USBERR_ALLOC;
                    }
                    
                    (void)memset((void *)g_ep_recv_buff_ptr[ep_ptr->ep_num], 0x00, ep_ptr->size);

                    (void)USB_DCI_Recv_Data(&controller_ID, ep_ptr->ep_num,
                    g_ep_recv_buff_ptr[ep_ptr->ep_num], ep_ptr->size);
                }
            }
            else
            {
                ep_queue_head_ptr->MAX_PKT_LENGTH = (uint_32)
                    ((uint_32)((ep_ptr->size & 0x0000FFFF) << 16)| VUSB_EP_QUEUE_HEAD_IOS);
                #if PSP_HAS_DATA_CACHE
                    USB_dcache_flush_mlines((void *)ep_queue_head_ptr,
                        sizeof(USBHS_EP_QUEUE_HEAD_STRUCT));
                #endif                    
            
            } /* Endif */
        } /* Endif */

        /* Enable the endpoint for Rx and Tx and set the endpoint type */
        MCF_USB_OTG_EPCR(ep_ptr->ep_num) |=
            ((ep_ptr->direction ? (EHCI_EPCTRL_TX_ENABLE |
            EHCI_EPCTRL_TX_DATA_TOGGLE_RST) :
            (EHCI_EPCTRL_RX_ENABLE | EHCI_EPCTRL_RX_DATA_TOGGLE_RST)) |
            (ep_ptr->type << (ep_ptr->direction ?
            EHCI_EPCTRL_TX_EP_TYPE_SHIFT : EHCI_EPCTRL_RX_EP_TYPE_SHIFT)));
    } 
    else
    {
        return USBERR_EP_INIT_FAILED;
    } /* Endif */

    return USB_OK;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Cancel_Transfer
 *
 * @brief The function cancels any pending Transfers which ahve not been sent
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param direction       : Endpoint direction
 *
 * @return status
 *         USBERR_NOT_SUPPORTED : Always
 ******************************************************************************
 * This function just returns Error Code not supported
 *****************************************************************************/
uint_8 USB_DCI_Cancel_Transfer (
    _usb_device_handle    handle,             /* [IN] USB Device handle */
    uint_8                endpoint_number,    /* [IN] Endpoint number for the transfer */
    uint_8                direction           /* [IN] Direction of transfer */
)
{
    volatile USBHS_EP_TR_STRUCT_PTR           dTD_ptr, check_dTD_ptr;
    volatile USBHS_EP_QUEUE_HEAD_STRUCT_PTR   ep_queue_head_ptr;
    uint_32                                   temp, bit_pos;

    bit_pos = (uint_32)(1 << (16 * direction + endpoint_number));
    temp = (uint_32)(2*endpoint_number + direction);

    ep_queue_head_ptr = (USBHS_EP_QUEUE_HEAD_STRUCT_PTR)MCF_USB_OTG_EPLISTADDR + temp;

    #if PSP_HAS_DATA_CACHE
        _DCACHE_INVALIDATE_MBYTES((void *)ep_queue_head_ptr,
            sizeof(USBHS_EP_QUEUE_HEAD_STRUCT));
    #endif    

    /* Unlink the dTD */
    dTD_ptr = usb_dev_ptr->EP_DTD_HEADS[temp];

    if (dTD_ptr)
    {
        check_dTD_ptr = (USBHS_EP_TR_STRUCT_PTR) \
            ((uint_32)dTD_ptr->NEXT_TR_ELEM_PTR &
            USBHS_TD_ADDR_MASK);

        if (dTD_ptr->SIZE_IOC_STS & USBHS_TD_STATUS_ACTIVE)
        {
            /* Flushing will halt the pipe */
            /* Write 1 to the Flush register */
            MCF_USB_OTG_EPFLUSH = bit_pos;
            /* Wait until flushing completed */
            while (MCF_USB_OTG_EPFLUSH & bit_pos)
            {
                /* ENDPTFLUSH bit should be cleared to indicate this
                   operation is complete */
            } 

            while (MCF_USB_OTG_EPSR & bit_pos)
            {
                /* Write 1 to the Flush register */
                MCF_USB_OTG_EPFLUSH = bit_pos;

                /* Wait until flushing completed */
                while (MCF_USB_OTG_EPFLUSH & bit_pos)
                {
                    /* ENDPTFLUSH bit should be cleared to indicate this
                       operation is complete */
                } 
            } 
        }

        /* Retire the current dTD */
        dTD_ptr->SIZE_IOC_STS = 0;
        dTD_ptr->NEXT_TR_ELEM_PTR = USBHS_TD_NEXT_TERMINATE;

        /* The transfer descriptor for this dTD */
        
        dTD_ptr->SCRATCH_PTR->PRIVATE = (void*)usb_dev_ptr;
        /* Free the dTD */
        USB_DCI_Free_dTD((uint_8)handle, (void*)dTD_ptr);

        /* Update the dTD head and tail for specific endpoint/direction */
        if (!check_dTD_ptr)
        {
            usb_dev_ptr->EP_DTD_HEADS[temp] = NULL;
            usb_dev_ptr->EP_DTD_TAILS[temp] = NULL;

            /* No other transfers on the queue */
            ep_queue_head_ptr->NEXT_DTD_PTR =  VUSB_EP_QUEUE_HEAD_NEXT_TERMINATE;
            ep_queue_head_ptr->SIZE_IOC_INT_STS = 0;
        }
        else
        {
            usb_dev_ptr->EP_DTD_HEADS[temp] = check_dTD_ptr;

            if (check_dTD_ptr->SIZE_IOC_STS & USBHS_TD_STATUS_ACTIVE)
            {
                /* Prime the Endpoint */
                MCF_USB_OTG_EPPRIME = bit_pos;

                if (!(MCF_USB_OTG_EPSR & bit_pos))
                {
                    while(MCF_USB_OTG_EPPRIME & bit_pos)
                    {
                        /* Wait for the ENDPTPRIME to go to zero */
                    }

                    if(MCF_USB_OTG_EPSR & bit_pos)
                    {
                        /* The endpoint was not not primed so no other 
                        transfers on the queue */
                        return USB_OK;
                    }
                }
                else
                {
                    return USB_OK;
                }

                /* No other transfers on the queue */
                ep_queue_head_ptr->NEXT_DTD_PTR = (uint_32)check_dTD_ptr;
                ep_queue_head_ptr->SIZE_IOC_INT_STS = 0;

                /* Prime the Endpoint */
                MCF_USB_OTG_EPPRIME = bit_pos;
            }
        }
    }

    return USB_OK;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Deinit_EndPoint
 *
 * @brief The function de initializes an endpoint
 *
 * @param controller_ID : Controller ID
 * @param ep_num        : Endpoint number
 * @param direction     : Endpoint direction
 *
 * @return status
 *         USB_OK                   : When successfull
 *         USBERR_EP_DEINIT_FAILED  : When unsuccessfull
 ******************************************************************************
 *
 * This function un-intializes the endpoint by clearing the corresponding
 * endpoint control register and then clearing the bdt elem.
 *
 *****************************************************************************/
uint_8 USB_DCI_Deinit_EndPoint (
    uint_8    controller_ID,   /* [IN] Controller ID */
    uint_8    ep_num,          /* [IN] Endpoint number */
    uint_8    direction        /* [IN] Endpoint direction */
)
{
    volatile USBHS_EP_QUEUE_HEAD_STRUCT *ep_queue_head_ptr;
    uint_32                              bit_pos;

    /*before de-initializing cancel all transfers on EP */
    (void)USB_DCI_Cancel_Transfer(&controller_ID, ep_num, direction);

    /* Get the endpoint queue head address */
    ep_queue_head_ptr = (USBHS_EP_QUEUE_HEAD_STRUCT_PTR)MCF_USB_OTG_EPLISTADDR +
        (2*ep_num + direction);

    #if PSP_HAS_DATA_CACHE
        _DCACHE_INVALIDATE_MBYTES((void *)ep_queue_head_ptr,
            sizeof(USBHS_EP_QUEUE_HEAD_STRUCT));
    #endif

    bit_pos = (uint_32)(1 << (16 * direction + ep_num));

    /* Check if the Endpoint is Primed */
    if((!(MCF_USB_OTG_EPPRIME & bit_pos)) &&
      (!(MCF_USB_OTG_EPSR & bit_pos)))
    {
        /* Reset the max packet length and the interrupt on Setup */
        ep_queue_head_ptr->MAX_PKT_LENGTH = 0;

        /* Disable the endpoint for Rx or Tx and reset the endpoint type */
        MCF_USB_OTG_EPCR(ep_num) &=
            ((direction ? ~EHCI_EPCTRL_TX_ENABLE : ~EHCI_EPCTRL_RX_ENABLE) |
            (direction ? ~EHCI_EPCTRL_TX_TYPE : ~EHCI_EPCTRL_RX_TYPE));
    }
    else
    {
        return USBERR_EP_DEINIT_FAILED;
    }

    if((ep_num != USB_CONTROL_ENDPOINT) && (direction == USB_RECV))
    {
        free(g_ep_recv_buff_ptr[ep_num]);
    }

    return USB_OK;    
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Stall_EndPoint
 *
 * @brief The function stalls an endpoint
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param direction       : Endpoint direction
 *
 * @return None
 *
 ******************************************************************************
 * This function stalls the endpoint by setting Endpoint BDT
 *****************************************************************************/
void USB_DCI_Stall_EndPoint (
    _usb_device_handle    handle,             /* [IN] USB Device handle */
    uint_8                endpoint_number,    /* [IN] Endpoint number to stall */
    uint_8                direction           /* [IN] Direction to stall */
)
{
    volatile USBHS_EP_QUEUE_HEAD_STRUCT *ep_queue_head_ptr;
    
    UNUSED(handle)

    /* Get the endpoint queue head address */
    ep_queue_head_ptr = (USBHS_EP_QUEUE_HEAD_STRUCT_PTR)MCF_USB_OTG_EPLISTADDR +
        (2*endpoint_number + direction);    
    
    #if PSP_HAS_DATA_CACHE
        USB_dcache_flush_mlines((void *)ep_queue_head_ptr,
            sizeof(USBHS_EP_QUEUE_HEAD_STRUCT));
    #endif

    /* Stall the endpoint for Rx or Tx and set the endpoint type */
    if (ep_queue_head_ptr->MAX_PKT_LENGTH & VUSB_EP_QUEUE_HEAD_IOS)
    {
        /* This is a control endpoint so STALL both directions */
        MCF_USB_OTG_EPCR(endpoint_number) |= (EHCI_EPCTRL_TX_EP_STALL | EHCI_EPCTRL_RX_EP_STALL);
    }
    else
    {
        MCF_USB_OTG_EPCR(endpoint_number) |= (direction ? EHCI_EPCTRL_TX_EP_STALL : EHCI_EPCTRL_RX_EP_STALL);
    }   
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Unstall_EndPoint
 *
 * @brief The function unstalls an endpoint
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param direction       : Endpoint direction
 *
 * @return None
 *
 ******************************************************************************
 * This function unstalls the endpoint by clearing Endpoint Control Register
 * and BDT
 *****************************************************************************/
void USB_DCI_Unstall_EndPoint (
    _usb_device_handle    handle,             /* [IN] USB Device handle */
    uint_8                endpoint_number,    /* [IN] Endpoint number to unstall */
    uint_8                direction           /* [IN] Direction to unstall */
)
{
    UNUSED(handle)
    
    /* Enable the endpoint for Rx or Tx and set the endpoint type */
    MCF_USB_OTG_EPCR(endpoint_number) &=
        (direction ? ~EHCI_EPCTRL_TX_EP_STALL : ~EHCI_EPCTRL_RX_EP_STALL);
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Get_Setup_Data
 *
 * @brief The function copies Setup Packet from USB RAM to application buffer
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param buffer_ptr      : Application buffer pointer
 *
 * @return None
 *
 ******************************************************************************
 * Copies setup packet from USB RAM to Application Buffer
 *****************************************************************************/
void USB_DCI_Get_Setup_Data (
    _usb_device_handle    handle,             /* [IN] USB Device handle */
    uint_8                endpoint_number,    /* [IN] Endpoint number for the transaction */
    uint_8_ptr            buffer_ptr          /* [IN] Pointer to the buffer into which to read data */
)
{
    volatile USBHS_EP_QUEUE_HEAD_STRUCT_PTR  ep_queue_head_ptr;
    #ifdef TRIP_WIRE
    boolean  read_safe;
    #endif
    
    UNUSED(handle)    
    /* Get the endpoint queue head */
    ep_queue_head_ptr = (USBHS_EP_QUEUE_HEAD_STRUCT_PTR)MCF_USB_OTG_EPLISTADDR + 
        2*endpoint_number + USB_RECV;

    /********************************************************************
     CR 1219. Hardware versions 2.3+ have a implementation of tripwire
     semaphore mechanism that requires that we read the contents of
     QH safely by using the semaphore. Read the USBHS document to under
     stand how the code uses the semaphore mechanism. The following are
     the steps in brief

     1. USBCMD Write ‘1’ to Setup Tripwire in register.
     2. Duplicate contents of dQH.StatusBuffer into local software byte
        array.
     3  Read Setup TripWire in register. (if set - continue; if
        cleared goto 1.)
     4. Write '0' to clear Setup Tripwire in register.
     5. Process setup packet using local software byte array copy and
        execute status/handshake phases.
     ********************************************************************/
    
    /*if semaphore mechanism is used the following code is compiled in*/
    #ifdef TRIP_WIRE  
        read_safe = FALSE;
        while(!read_safe)
        {       
         #if PSP_HAS_DATA_CACHE
           _DCACHE_INVALIDATE_MBYTES((void *)ep_queue_head_ptr,sizeof(USBHS_EP_QUEUE_HEAD_STRUCT)); 
           _DCACHE_INVALIDATE_MBYTES((void *)ep_queue_head_ptr->SETUP_BUFFER,USB_SETUP_PKT_SIZE); 
         #endif  
            /*********************************************************
             start with setting the semaphores
             *********************************************************/

            MCF_USB_OTG_USBCMD |= EHCI_CMD_SETUP_TRIPWIRE_SET;
            /*********************************************************
             Duplicate the contents of SETUP buffer to our buffer
             Note:On 5329 the data returned is little endian so it needs
             to be byte swapped.Here while copying it is been takencare.
             If you are using this as reference code care should be taken
             while copying the setup packet on your chip.
             *********************************************************/
            buff_ptr[0]= ep_queue_head_ptr->SETUP_BUFFER[3];
            buff_ptr[1]= ep_queue_head_ptr->SETUP_BUFFER[2];
            buff_ptr[2]= ep_queue_head_ptr->SETUP_BUFFER[1];
            buff_ptr[3]= ep_queue_head_ptr->SETUP_BUFFER[0];
            buff_ptr[4]= ep_queue_head_ptr->SETUP_BUFFER[7];
            buff_ptr[5]= ep_queue_head_ptr->SETUP_BUFFER[6];
            buff_ptr[6]= ep_queue_head_ptr->SETUP_BUFFER[5];
            buff_ptr[7]= ep_queue_head_ptr->SETUP_BUFFER[4];
            
            /*********************************************************
             If setup tripwire semaphore is cleared by hardware it means
             that we have a danger and we need to restart.
             else we can exit out of loop safely.
             *********************************************************/
            if(MCF_USB_OTG_USBCMD & EHCI_CMD_SETUP_TRIPWIRE_SET)
            {
                /* we can proceed exiting out of loop*/
                read_safe = TRUE;
            }
        }

        /*********************************************************
         Clear the semaphore bit now
         *********************************************************/
        MCF_USB_OTG_USBCMD &= EHCI_CMD_SETUP_TRIPWIRE_CLEAR;
    #else   /*when semaphore is not used *
        /*********************************************************
         Duplicate the contents of SETUP buffer to our buffer
         Note:On 5329 the data returned is little endian so it needs
         to be byte swapped.Here while copying it is been takencare.
         If you are using this as reference code care should be taken
         while copying the setup packet on your chip.
         *********************************************************/
        buffer_ptr[0]= ep_queue_head_ptr->SETUP_BUFFER[3];
        buffer_ptr[1]= ep_queue_head_ptr->SETUP_BUFFER[2];
        buffer_ptr[2]= ep_queue_head_ptr->SETUP_BUFFER[1];
        buffer_ptr[3]= ep_queue_head_ptr->SETUP_BUFFER[0];
        buffer_ptr[4]= ep_queue_head_ptr->SETUP_BUFFER[7];
        buffer_ptr[5]= ep_queue_head_ptr->SETUP_BUFFER[6];
        buffer_ptr[6]= ep_queue_head_ptr->SETUP_BUFFER[5];
        buffer_ptr[7]= ep_queue_head_ptr->SETUP_BUFFER[4];
    #endif
    
    /* Clear the bit in the ENDPTSETUPSTAT */
    MCF_USB_OTG_EPSETUPSR = (uint_32)(1 << endpoint_number);
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Get_Transfer_Status
 *
 * @brief The function retrieves the Transfer status of an endpoint
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param direction       : Endpoint direction
 *
 * @return status
 *         USBERR_TR_FAILED                : When unsuccessfull
 *         USB_STATUS_IDLE                 : No transfer on endpoint
 *         USB_STATUS_DISABLED             : endpoint is disabled
 *         USB_STATUS_STALLED              : endpoint is stalled
 *         USB_STATUS_TRANSFER_IN_PROGRESS : When SIE has control of BDT
 ******************************************************************************
 *
 * This function retrieves the transfer status of the endpoint by checking the
 * BDT as well as the endpoint control register
 *
 *****************************************************************************/
uint_8 USB_DCI_Get_Transfer_Status (
    _usb_device_handle    handle,             /* [IN] USB Device handle */
    uint_8                endpoint_number,    /* [IN] Endpoint number */
    uint_8                direction           /* [IN] Endpoint direction */
)
{
    UNUSED(handle)          
    
    volatile USBHS_EP_TR_STRUCT_PTR dTD_ptr;
    
    uint_32 bit_pos = (uint_32)(1 << (16 * direction + endpoint_number));
    
    uint_8  status = USB_STATUS_IDLE;
    
    uint_32 ep_control = MCF_USB_OTG_EPCR(endpoint_number);

    /* Unlink the dTD */
    dTD_ptr = usb_dev_ptr->EP_DTD_HEADS[2*endpoint_number + direction];
    
    /* Check for direction in endpoint control register */
    if(!(ep_control & (MCF_USB_OTG_EPCR_RXE|MCF_USB_OTG_EPCR_TXE)))
    {
            status = USB_STATUS_DISABLED;
    }
    /* Check for stall bit in endpoint control register */
    else if (ep_control &  (MCF_USB_OTG_EPCR_RXS|MCF_USB_OTG_EPCR_TXS))
    {
            status = USB_STATUS_STALLED ;
    }
    /* Check whether SIE has control of BDT */
    else if (dTD_ptr != NULL)
    {
            status = USB_STATUS_TRANSFER_IN_PROGRESS;
    }

    
    return status; 
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Clear_DATA0_Endpoint
 *
 * @brief The function clear the DATA0/1 bit 
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param direction       : Endpoint direction
 *
 * @return None
 *
 ******************************************************************************
 * This function clear the DATA0/1 bit 
 *****************************************************************************/
void  USB_DCI_Clear_DATA0_Endpoint (
    uint_8                endpoint_number,    /* [IN] Endpoint number */
    uint_8                direction           /* [IN] Endpoint direction */
)
{
	UNUSED(endpoint_number);
	UNUSED(direction);
	return;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Recv_Data
 *
 * @brief The function retrieves data received on an RECV endpoint
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param buffer_ptr      : Application buffer pointer
 * @param size            : Size of the buffer
 *
 * @return status
 *         USB_OK                          : When successfull
 *         USBERR_RX_FAILED                : When unsuccessfull
 ******************************************************************************
 * This function retrieves data received data on a RECV endpoint by copying it
 * from USB RAM to application buffer
 *****************************************************************************/

uint_8 embedded_rx_buff[64];

uint_8 USB_DCI_Recv_Data (
    _usb_device_handle    handle,             /* [IN] USB Device handle */
    uint_8                endpoint_number,    /* [IN] Endpoint number */
    uchar_ptr             buffer_ptr,         /* [OUT] Application buffer pointer */
    uint_32               size                /* [IN] Size of the buffer */
)
{
    /* Add and execute the device transfer descriptor */
    #if PSP_HAS_DATA_CACHE
        USB_dcache_flush_mlines((void *)buff_ptr, size);

        USB_dcache_invalidate_mlines((void *)buff_ptr, size);
    #endif
  
    if((endpoint_number != USB_CONTROL_ENDPOINT) && (buffer_ptr == NULL) && (size == 0))
    {
     volatile USBHS_EP_QUEUE_HEAD_STRUCT_PTR ep_queue_head_ptr;
     uint_32 max_packet_size;
     
     /* NULL buffer and 0 size on a non-control endpoint receive */
     /* Use the embedded buffer to maintain the compatibility with previous code written for FS */
     buffer_ptr = &embedded_rx_buff[0];
     size = sizeof(embedded_rx_buff)/sizeof(embedded_rx_buff[0]);
     

     ep_queue_head_ptr = (USBHS_EP_QUEUE_HEAD_STRUCT_PTR)MCF_USB_OTG_EPLISTADDR + (2*endpoint_number + USB_RECV);
     max_packet_size = (ep_queue_head_ptr->MAX_PKT_LENGTH >> 16) & 0x000007FF;
     
     if(size > max_packet_size)
     {
      size = (USB_PACKET_SIZE)max_packet_size;
     }
     
    }

    return(USB_DCI_Add_dTD((uint_8)handle, endpoint_number, USB_RECV, buffer_ptr, (USB_PACKET_SIZE)size)); 
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Send_Data
 *
 * @brief The function configures Controller to send data on an SEND endpoint
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param buffer_ptr      : Application buffer pointer
 * @param size            : Size of the buffer
 *
 * @return status
 *         USB_OK           : When successfull
 *         USBERR_TX_FAILED : When unsuccessfull
 ******************************************************************************
 * This function configures Controller to send data on an SEND endpoint by
 * setting the BDT to send data.
 *****************************************************************************/
uint_8 USB_DCI_Send_Data (
    _usb_device_handle    handle,             /* [IN] USB Device handle */
    uint_8                endpoint_number,    /* [IN] Endpoint number of the transaction */
    uchar_ptr             buffer_ptr,         /* [IN] Pointer to the buffer to send */
    uint_32               size                /* [IN] Number of bytes to send */
)
{
    /* Add and execute the device transfer descriptor */
    #if PSP_HAS_DATA_CACHE
        USB_dcache_flush_mlines((void *)xd_ptr->WSTARTADDRESS,
            xd_ptr->WTOTALLENGTH);
    #endif
    
    return(USB_DCI_Add_dTD((uint_8)handle, endpoint_number, USB_SEND, buffer_ptr, (USB_PACKET_SIZE)size));
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Set_Address
 *
 * @brief The function configures Controller to send data on an SEND endpoint
 *
 * @param handle  : USB Device handle
 * @param address : Controller Address
 *
 * @return None
 *
 ******************************************************************************
 * Assigns the Address to the Controller
 *****************************************************************************/
void  USB_DCI_Set_Address (
    _usb_device_handle    handle,    /* [IN] Controller ID */
    uint_8                address    /* [IN] Controller Address */
)
{
  UNUSED(handle)
   
   /* The address bits are past bit 25-31. Set the address */
   MCF_USB_OTG_DEVICEADDR = ((uint_32)address << USBHS_ADDRESS_BIT_SHIFT);

   usb_dev_ptr->USB_STATE = USB_STATE_ADDRESS;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Shutdown
 *
 * @brief The function shuts down the controller
 *
 * @param handle : USB Device handle
 *
 * @return None
 *
 ******************************************************************************
 * Resets USB Device Controller
 *****************************************************************************/
void USB_DCI_Shutdown (
    _usb_device_handle    handle    /* [IN] USB Device handle */
)
{
	UNUSED(handle) 
    
    /* Disable interrupts */
    MCF_USB_OTG_USBINTR &=
        ~(MCF_USB_OTG_USBINTR_UE | MCF_USB_OTG_USBINTR_UEE |
        MCF_USB_OTG_USBINTR_PCE | MCF_USB_OTG_USBINTR_URE);

    /* Reset the Run the bit in the command register to stop VUSB */
    MCF_USB_OTG_USBCMD &= ~MCF_USB_OTG_USBCMD_RS;

    /* Reset the controller to get default values */
    MCF_USB_OTG_USBCMD = MCF_USB_OTG_USBCMD_RST;

    free((void*)usb_dev_ptr->DRIVER_MEMORY);    
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Assert_Resume
 *
 * @brief The function makes the Controller start USB RESUME signaling
 *
 * @param handle: USB Device handle
 *
 * @return None
 *
 ******************************************************************************
 *
 * This function starts RESUME signalling and then stops it after some delay.
 * In this delay make sure that COP is reset.
 *
 *****************************************************************************/
void USB_DCI_Assert_Resume (
    _usb_device_handle    handle    /* [IN] USB Device handle */
)
{
    uint_32                               temp;
	
	UNUSED(handle) 
    /* Assert the Resume signal */
    temp = MCF_USB_OTG_PORTSC1;
    temp &= ~EHCI_PORTSCX_W1C_BITS;
    temp |= EHCI_PORTSCX_PORT_FORCE_RESUME;
    MCF_USB_OTG_PORTSC1 = temp;  

    /* Port change interrupt will be asserted at the end of resume
       operation */      
}

/**************************************************************************//*!
 *
 * @name  USB_ISR
 *
 * @brief The function handles USB interrupts on the bus.
 *
 * @param None
 *
 * @return None
 *
 ******************************************************************************
 * This function is hooked onto interrupt 69 and handles the USB interrupts.
 * After handling the interrupt it calls the Device Layer to notify it about
 * the event.
 *****************************************************************************/
void __declspec(interrupt) USB_ISR(void)
{
    uint_32 error = MCF_USB_OTG_USBSTS & MCF_USB_OTG_USBINTR;
    
    /* Clear all the interrupts occured */
    MCF_USB_OTG_USBSTS = error;
    
    /* Check for RESET Interrupt */
    if(error & MCF_USB_OTG_USBSTS_URI)
    {
            /* Print so that user can be notified of reset in general */
            USB_DCI_Process_Reset(0);
    }
    
    /* Check for USB Interrupt (transaction complete) */  
    if (error & MCF_USB_OTG_USBSTS_UI)
    {           
            USB_DCI_Process_Tr_Complete(0);
    }
    
    /* Check for PORT CHANGE interrupt */
    if (error & MCF_USB_OTG_USBSTS_PCI)
    {
            USB_DCI_Process_Port_Change(0);
    }
    
    /* Check for ERROR interrupt */
    if (error & MCF_USB_OTG_USBSTS_UEI)
    {
            USB_DCI_Process_Error(0);
    }
    
    /* Check for SOF receive interrupt */
    if (error & MCF_USB_OTG_USBSTS_SRI)
    {
            USB_DCI_Process_SOF(0);
    }
 
    /* Check for SUSPEND interrupt */
    if (error & MCF_USB_OTG_USBSTS_SLI)
    {
            USB_DCI_Process_Suspend(0);
    }
}

/**************************************************************************//*!
 *
 * @name  Clear_Mem
 *
 * @brief The function clears memory starting from start_addr till count bytes
 *
 * @param start_addr : Buffer Start address
 * @param count      : Count of Bytes
 * @param val        : Value to be set
 *
 * @return None
 ******************************************************************************
 * This function is an implementation of memset
 *****************************************************************************/
void Clear_Mem (
    uint_8_ptr start_addr,  /* [OUT] Buffer Start address */
    uint_32 count,          /* [IN] Count of Bytes */
    uint_8 val              /* [IN] Value to be set */
)
{
    (void)memset(start_addr, val, count);
    return;
}


#ifdef USB_LOWPOWERMODE
/**************************************************************************//*!
 *
 * @name  Enter_StopMode
 *
 * @brief The function configures STOP Mode
 *
 * @param stop_mode : STOP MODE to be entered
 *
 * @return None
 ******************************************************************************
 * This function configures different STOP MODES defined by the controller.
 * Used to put controller into low power mode. Only STOP MODE 3 is implemented
 *****************************************************************************/
static void Enter_StopMode(STOP_MODE stop_mode)
{
    _Stop;
}

#endif

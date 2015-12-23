/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 **************************************************************************//*!
 *
 * @file msd_dev_kinetis.c
 *
 * @author 
 *
 * @version 
 *
 * @date 
 *
 * @brief  
 *****************************************************************************/
 
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "types.h"          /* User Defined Data Types */
#include "hidef.h"          /* for EnableInterrupts macro */
#include "derivative.h"     /* include peripheral declarations */
#include "usb_msc.h"		/* USB MSC Class Header File */
#include "msd_dev.h"			/* Disk Application Header File */
#include "usb_class.h"
#include "sd.h"
#include "usb_dciapi.h" /* USB DCI API Header File */
#include "user_config.h"

/* skip the inclusion in dependency statge */
#ifndef __NO_SETJMP
   #include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>	

/*****************************************************************************
 * Constant and Macro's - None
 *****************************************************************************/
 
/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
extern void Watchdog_Reset(void);

/****************************************************************************
 * Global Variables
 ****************************************************************************/ 
 /* Add all the variables needed for disk.c to this structure */
DISK_GLOBAL_VARIABLE_STRUCT g_disk;
extern uint_8               host_stack_active; /* TRUE if the host stack is active */
extern uint_8               dev_stack_active;  /* TRUE if the peripheral stack is active */

/*****************************************************************************
 * Local Types - None
 *****************************************************************************/
 
/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
void USB_App_Callback(uint_8 controller_ID,  uint_8 event_type, void* val);
void MSD_Event_Callback(uint_8 controller_ID, uint_8 event_type, void* val);
void Disk_App(void);  

/*****************************************************************************
 * Local Variables 
 *****************************************************************************/

/*****************************************************************************
 * Local Functions
 *****************************************************************************/
/******************************************************************************
 * 
 *    @name       Disk_App
 *    
 *    @brief      
 *                  
 *    @param      None
 * 
 *    @return     None
 *    
 *****************************************************************************/
void Disk_App(void)
{
  /* User Code */ 
  return;
}

/******************************************************************************
 * 
 *    @name        USB_App_Callback
 *    
 *    @brief       This function handles the callback  
 *                  
 *    @param       controller_ID : To Identify the controller
 *    @param       event_type : value of the event
 *    @param       val : gives the configuration value 
 * 
 *    @return      None
 *    
 *
 *****************************************************************************/
void USB_App_Callback(uint_8 controller_ID, uint_8 event_type, void* val) 
{
    UNUSED (controller_ID)
    UNUSED (val)    
    
    if(event_type == USB_APP_BUS_RESET) 
    {
        g_disk.start_app = FALSE;    
    }
    else if(event_type == USB_APP_ENUM_COMPLETE) 
    {
        g_disk.start_app = TRUE;        
    }
    else if(event_type == USB_APP_ERROR)
    {
    	/* add user code for error handling */
    }
    
    return;
}

/******************************************************************************
 * 
 *    @name        MSD_Event_Callback
 *    
 *    @brief       This function handles the callback  
 *                  
 *    @param       controller_ID : To Identify the controller
 *    @param       event_type : value of the event
 *    @param       val : gives the configuration value 
 * 
 *    @return      None
 *
 *****************************************************************************/
void MSD_Event_Callback(uint_8 controller_ID, 
							   uint_8 event_type, 
							   void* val) 
{
	PTR_LBA_APP_STRUCT lba_data_ptr;
	uint_8_ptr prevent_removal_ptr, load_eject_start_ptr;	
	PTR_DEVICE_LBA_INFO_STRUCT device_lba_info_ptr;
    UNUSED (controller_ID)
    switch(event_type)
    {
    	case USB_APP_DATA_RECEIVED:
    		break;
    	case USB_APP_SEND_COMPLETE:
    		break;
    	case USB_MSC_START_STOP_EJECT_MEDIA:
    		load_eject_start_ptr = (uint_8_ptr)val;
    		/* Code to be added by user for starting, stopping or 
           	   ejecting the disk drive. e.g. starting/stopping the motor in 
           	   case of CD/DVD*/
    		break;
    	case USB_MSC_DEVICE_READ_REQUEST:     		
    		/* copy data from storage device before sending it on USB Bus 
    		   (Called before calling send_data on BULK IN endpoints)*/
    		lba_data_ptr = (PTR_LBA_APP_STRUCT)val;
    		/* read data from mass storage device to driver buffer */
		#if RAM_DISK_APP
    		USB_memcopy(g_disk.storage_disk + lba_data_ptr->offset,
    			lba_data_ptr->buff_ptr, 
    			lba_data_ptr->size);
		#elif SD_CARD_APP        
			SD_Read_Block(lba_data_ptr);
		#endif         
    		break;
    	case USB_MSC_DEVICE_WRITE_REQUEST:
    		/* copy data from USb buffer to Storage device 
   		   	   (Called before after recv_data on BULK OUT endpoints)*/
    		lba_data_ptr = (PTR_LBA_APP_STRUCT)val;
    		/* read data from driver buffer to mass storage device */
		#if RAM_DISK_APP
    		USB_memcopy(lba_data_ptr->buff_ptr,
    			g_disk.storage_disk + lba_data_ptr->offset,
    			lba_data_ptr->size);
		#elif SD_CARD_APP
    		SD_Write_Block(lba_data_ptr);
		#endif          
    		break;
    	case USB_MSC_DEVICE_FORMAT_COMPLETE:
    		break;
    	case USB_MSC_DEVICE_REMOVAL_REQUEST:
    		prevent_removal_ptr = (uint_8_ptr) val;
    		if(SUPPORT_DISK_LOCKING_MECHANISM)
    		{    			 
    			g_disk.disk_lock = *prevent_removal_ptr;
    		}
    		else if((!SUPPORT_DISK_LOCKING_MECHANISM)&&(!(*prevent_removal_ptr)))
    		{
    			/*there is no support for disk locking and removal of medium is enabled*/
				/* code to be added here for this condition, if required */ 
    		}
    		break;
    	case USB_MSC_DEVICE_GET_INFO:
    		device_lba_info_ptr = (PTR_DEVICE_LBA_INFO_STRUCT)val;
		#if RAM_DISK_APP
    		device_lba_info_ptr->total_lba_device_supports = TOTAL_LOGICAL_ADDRESS_BLOCKS;	
    		device_lba_info_ptr->length_of_each_lba_of_device = LENGTH_OF_EACH_LBA; 
		#elif SD_CARD_APP
    		SD_Card_Info(&device_lba_info_ptr->total_lba_device_supports,
    		    &device_lba_info_ptr->length_of_each_lba_of_device);
		#endif
    		device_lba_info_ptr->num_lun_supported = LOGICAL_UNIT_SUPPORTED;
    		break;    		
    	default: 
    		break;
    }
        
    return;
}

 /******************************************************************************
 *  
 *   @name        TestApp_Init
 * 
 *   @brief       This function is the entry for mouse (or other usuage)
 * 
 *   @param       None
 * 
 *   @return      None
 **                
 *****************************************************************************/
 
uint_32 App_PeripheralInit(void)
{       
  uint_32   error;
    
  host_stack_active  = FALSE;
  dev_stack_active = TRUE;  
  
  /* initialize the Global Variable Structure */
	USB_memzero(&g_disk, sizeof(DISK_GLOBAL_VARIABLE_STRUCT));
  g_disk.app_controller_ID = USB_CONTROLLER_ID;

  DisableInterrupts;
	
  #if SD_CARD_APP 
    #if(defined(__MCF52259_H__) || defined(__MCF52221_H__))    	
    	/* PAN0 is configured to be GPIO */
    	MCF_GPIO_PANPAR &= ~(MCF_GPIO_PANPAR_PANPAR0 | 
    	                 	 MCF_GPIO_PANPAR_PANPAR1 | 
    	                 	 MCF_GPIO_PANPAR_PANPAR2);
    	/* PAN0 is input */
    	MCF_GPIO_DDRAN &= ~MCF_GPIO_DDRAN_DDRAN0;
    	
    	_SD_DE; 						/* Card detection */
    	_SD_WR;    						/* Write protection */    	    		
    #else 
    	#ifdef __MCF52277_H__

	    	/* IRQ1 is configured as input */
	    	MCF_PAD_PAR_IRQ = 0; 
	    	
	    	_SD_DE;                     /* Card detection */
		    _SD_WR;                     /* Write protect */      	
    	#else 
        #if (defined _MK_xxx_H_)
          #if USE_SPI_PROTOCOL
            #if (defined MCU_MK40N512VMD100)||(defined MCU_MK53N512CMD100)
              SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK |SIM_SCGC5_PORTE_MASK;
              GPIOB_PDIR |= 1 << 8;
              PORTB_PCR8 |= PORT_PCR_MUX(1);
              GPIOB_PDDR &= ~((uint_32)1 << 8);
              PORTB_PCR8 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
            #elif (defined MCU_MK60N512VMD100)        
              GPIOA_PDIR |= 1 << 27;
              PORTA_PCR27 |= PORT_PCR_MUX(1);
              GPIOA_PDDR &= ~((uint_32)1 << 27);
              PORTA_PCR27 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
            #endif

            GPIOE_PDIR |= 1 << 5;
            PORTE_PCR5 |= PORT_PCR_MUX(1);
            GPIOE_PDDR &= ~((uint_32)1 << 5);
            PORTE_PCR5 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;

          #elif USE_SDHC_PROTOCOL
            #if (defined MCU_MK40N512VMD100)
              SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK;
              GPIOA_PDIR |= 1 << 16;
              PORTA_PCR16 |= PORT_PCR_MUX(1);
              GPIOA_PDDR &= ~((uint_32)1 << 16);
              PORTA_PCR16 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
            #else
              #if (defined MCU_MK53N512CMD100)
                SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK |SIM_SCGC5_PORTE_MASK;
                SIM_SCGC3 |= SIM_SCGC3_SDHC_MASK;
              #endif
              
              GPIOE_PDIR |= 1 << 28;
              PORTE_PCR28 |= PORT_PCR_MUX(1);
              GPIOE_PDDR &= ~((uint_32)1 << 28);
              PORTE_PCR28 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
            #endif
            #if (defined MCU_MK53N512CMD100)
              GPIOC_PDIR |= 1 << 9;
              PORTC_PCR9 |= PORT_PCR_MUX(1);
              GPIOC_PDDR &= ~((uint_32)1 << 9);
              PORTC_PCR9 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
            #else
              GPIOE_PDIR |= 1 << 27;
              PORTE_PCR27 |= PORT_PCR_MUX(1);
              GPIOE_PDDR &= ~((uint_32)1 << 27);
              PORTE_PCR27 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
            #endif
          #endif
    
          _SD_DE;                 /* Card detection */
          _SD_WR;                 /* Write protect */
        #elif defined(MCU_mcf51jf128)
          SIM_SCGC6 |=  SIM_SCGC6_PORTC_MASK;
          PTC_DD &=~0x20; /* Clear PTF4 to input for SD_DE*/
          PCTLC_PUE |=0x20;/* Enable pull up resistor on PTC4 pin*/

          MXC_PTCPF2 &=~MXC_PTCPF2_C4_MASK;
          MXC_PTCPF2 |= MXC_PTCPF2_C4(1);/* Set GPIO funtionality to PTC4 pin for SD card detecting*/
          _SD_DE;
		    #else
		      PTGDD_PTGDD0 = 0;           /* PTG0 is input */
		      PTGPE_PTGPE0 = 1;           /* internal pullup for PTG0 */
		    
		      _SD_DE = 0;                 /* Card detection */
		      _SD_WR = 0;                 /* Write protect */
			  #endif
      #endif 
    #endif

#if (defined _MK_xxx_H_) ||  defined(MCU_mcf51jf128)
   while(SD_DE&kSD_Desert)
   {
      Watchdog_Reset();
   }     /* SD Card inserted */
#else
   while(SD_DE == kSD_Desert)
   {
      Watchdog_Reset();
   }     /* SD Card inserted */
#endif

if(!SD_Init()) return 1; /* Initialize SD_CARD and SPI Interface */


#if (USE_SPI_PROTOCOL && !(defined _MCF51MM256_H) && !defined(MCU_mcf51jf128))
   (void)SD_ReadCSD();
#endif
#endif
  /* Initialize the USB interface */
  error = (uint_32)USB_Class_MSC_Init(g_disk.app_controller_ID,
                      USB_App_Callback,NULL, MSD_Event_Callback);
	
    EnableInterrupts;
   
  return error;
} 



/******************************************************************************
 *
 *   @name        App_PeripheralUninit
 *
 *   @brief       This function is the entry for Keyboard Application (Peripheral)
 *
 *   @param       None
 *
 *   @return      None
 *
 *****************************************************************************
 * This function starts the keyboard application
 *****************************************************************************/     
void App_PeripheralUninit(void)
{       
    DisableInterrupts;
    /* Uninitialize the USB interface */
    USB_DCI_Shutdown(0);
    USB0_INTEN = 0;
    USB0_CTL = 0;
    USB0_ISTAT = 0xff;
    USB0_USBTRC0 &= ~USB_USBTRC0_USBRESMEN_MASK;
    USB0_TOKEN = 0;
    USB0_ENDPT0 = 0;
    USB0_ENDPT1 = 0;
    USB0_ENDPT2 = 0;
    USB0_ENDPT3 = 0;
    USB0_ENDPT4 = 0;
    USB0_ENDPT5 = 0;
    USB0_ENDPT6 = 0;
    USB0_ENDPT7 = 0;
    USB0_ENDPT8 = 0;
    USB0_ENDPT9 = 0;
    USB0_ENDPT10 = 0;
    USB0_ENDPT11 = 0;
    USB0_ENDPT12 = 0;
    USB0_ENDPT13 = 0;
    USB0_ENDPT14 = 0;
    USB0_ENDPT15 = 0;
    USB0_ADDR = 0;
     
    USB0_CTL = USB_CTL_HOSTMODEEN_MASK;
    USB0_CTL = 0;
    
    EnableInterrupts;
   
    host_stack_active  = FALSE;
    dev_stack_active = FALSE; 
}



/******************************************************************************
 *
 *   @name        App_PeripheralTask
 *
 *   @brief       Application task function. It is called from the main loop
 *
 *   @param       None
 *
 *   @return      None
 *
 *****************************************************************************
 * Application task function. It is called from the main loop
 *****************************************************************************/
void App_PeripheralTask(void)
{
  /* call the periodic task function */      
  USB_MSC_Periodic_Task(); 
  
  /* check whether enumeration is complete or not */
  if(g_disk.start_app == TRUE)
  {        
    Disk_App(); 
  }
}

/* EOF */

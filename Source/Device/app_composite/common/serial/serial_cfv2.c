/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
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
 * @file serial_cfv2.c
 *
 * @author
 *
 * @version
 *
 * @date May-28-2009
 *
 * @brief The file contains Serial low level driver Implementation
 *
 *****************************************************************************/
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "derivative.h" /* include peripheral declarations */
#include "types.h"      /* User Defined Data Types */
#include "user_config.h"
#include "serial_cfv2.h"
#include "file_struc.h"

/******************************************************************************
 * Macro's
 *****************************************************************************/
#define CONSOLE_BAUD_RATE   57600
#define CONSOLE_PORT        1
/******************************************************************************
 * Local Variables
 *****************************************************************************/
boolean g_console_init;
const PSCI_REG g_sci_reg[MAX_SCI_INTERFACES] = 
{
    (PSCI_REG)(UART0_BASE_ADDRESS),
    (PSCI_REG)(UART1_BASE_ADDRESS),    
    (PSCI_REG)(UART2_BASE_ADDRESS)
};
SCI_STRUCT g_serial_struct[MAX_SCI_INTERFACES];

/******************************************************************************
 * Local Function Prototypes
 *****************************************************************************/
static void SCI_DisableTx(
    uint_8 ControllerID 
);
static void SCI_EnableTx(
    uint_8 ControllerID     
);
static void SCI_WriteData(
    uint_8 ControllerID,    
    uint_8 val              
);
static uint_8 SCI_ReadData(
    uint_8 ControllerID     
);
static uint_8 SCI_GetStatus(
    uint_8 ControllerID     
);
static void SCI_TransmitISR(
    uint_8 ControllerID     
);
static void SCI_ReceiveISR(
    uint_8 ControllerID     
);
static void SCI_ErrorISR(
    uint_8 ControllerID     
);
/******************************************************************************
 * Local Function
 *****************************************************************************/
/**************************************************************************//*!
 *
 * @name  SCI_DisableTx
 *
 * @brief This function disables the Transmitter Interrupt
 *
 * @param controller_ID :   Serial Controller ID
 *
 * @return None
 ******************************************************************************
 * This function disables the Transmitter Interrupt
 *****************************************************************************/
static void SCI_DisableTx
(
    uint_8 ControllerID     /* [IN] Serial Controller ID */
)
{
    PSCI_REG pSciReg = g_sci_reg[ControllerID];
    pSciReg->WRITE.UCR |= MCF_UART_UCR_TX_DISABLED; 
    pSciReg->WRITE.UIMR = (uint_8)~MCF_UART_UIMR_TXRDY;
    return;  
}

/**************************************************************************//*!
 *
 * @name  SCI_EnableTx
 *
 * @brief This function enables the Transmitter Interrupt
 *
 * @param controller_ID :   Serial Controller ID
 *
 * @return None
 ******************************************************************************
 * This function enables the Transmitter Interrupt
 *****************************************************************************/
static void SCI_EnableTx
(
    uint_8 ControllerID     /* [IN] Serial Controller ID */
)
{
    PSCI_REG pSciReg = g_sci_reg[ControllerID];
    
    /* Enable transmition interrupt */
	pSciReg->WRITE.UIMR |= MCF_UART_UIMR_TXRDY;
    pSciReg->WRITE.UCR |= MCF_UART_UCR_TX_ENABLED; 
    return;  
}

/**************************************************************************//*!
 *
 * @name  SCI_WriteData
 *
 * @brief This function writes the data to be transmitted into the data 
 *        register
 *
 * @param controller_ID :   Serial Controller ID
 * @param val           :   Data to be transmitted
 *
 * @return None
 ******************************************************************************
 * This function writes the data to be transmitted into the data register
 *****************************************************************************/
static void SCI_WriteData
(
    uint_8 ControllerID,    /* [IN] Serial Controller ID */ 
    uint_8 val              /* [IN] Data to be transmitted */
)
{
    
    PSCI_REG pSciReg = g_sci_reg[ControllerID];
    /* Send the character */
    pSciReg->WRITE.UTB = (uint8)val;
}
/**************************************************************************//*!
 *
 * @name  SCI_ReadData
 *
 * @brief This function reads the data received in the serial port data 
 *        register
 *
 * @param controller_ID :   Serial Controller ID
 *
 * @return None
 ******************************************************************************
 * This function reads the data received in the serial port data register
 *****************************************************************************/
static uint_8 SCI_ReadData(
    uint_8 ControllerID     /* [IN] Serial Controller ID */ 
)
{
    PSCI_REG pSciReg = g_sci_reg[ControllerID];
    return (uint_8)(pSciReg->READ.URB);
}
/**************************************************************************//*!
 *
 * @name  SCI_GetStatus
 *
 * @brief This function returns the value in the status register
 *
 * @param controller_ID :   Serial Controller ID
 *
 * @return None
 ******************************************************************************
 * This function returns the value in the status register
 *****************************************************************************/
static uint_8 SCI_GetStatus(
    uint_8 ControllerID     /* [IN] Serial Controller ID */ 
)
{
    PSCI_REG pSciReg = g_sci_reg[ControllerID];
    return pSciReg->READ.USR;
}

/**************************************************************************//*!
 *
 * @name  SCI_TransmitISR
 *
 * @brief This function is called whenever a transmit complete interrupt is 
 *        generated
 *
 * @param controller_ID :   Serial Controller ID
 *
 * @return None
 ******************************************************************************
 * This function is called whenever a transmit complete interrupt is generated
 *****************************************************************************/
static void SCI_TransmitISR
(
    uint_8 ControllerID     /* [IN] Serial Controller ID */ 
)
{
    PBUFFER psBuffer = g_serial_struct[ControllerID].buffer[TX_BUFFER]; 
    
    if(psBuffer == NULL) 
    {
        /* No Data Transmitted. Spurious Interrupt */
        SCI_DisableTx(ControllerID);
        g_serial_struct[ControllerID].app_callback(ControllerID, 
            		   			SCI_TRANSMIT_SPURIOUS_INTERRUPT, psBuffer);
        return;            
    }
    if (psBuffer->CurOffSet < psBuffer->Length)
    {
        static uint_8 val;
        /* Write Next Byte */
        val = psBuffer->pBuffer[psBuffer->CurOffSet++];
        SCI_WriteData(ControllerID, val);
    }
    else if(psBuffer->CurOffSet == psBuffer->Length)
    {
        /* Buffer Send Complete */
        SCI_DisableTx(ControllerID); 
        
        psBuffer->InUse = FALSE;
        g_serial_struct[ControllerID].buffer[TX_BUFFER] = NULL;
        
        /* Send Callback to the upper layer */
        if(g_serial_struct[ControllerID].app_callback)
        {
            g_serial_struct[ControllerID].app_callback(ControllerID, 
            SCI_TRANSMIT_COMPLETE, psBuffer);
        }   
    }
    return;
}

/**************************************************************************//*!
 *
 * @name  SCI_ReceiveISR
 *
 * @brief This function is called whenever a receive complete interrupt is 
 *        generated
 *
 * @param controller_ID :   Serial Controller ID
 *
 * @return None
 ******************************************************************************
 * This function is called whenever a receive complete interrupt is generated
 *****************************************************************************/
static void SCI_ReceiveISR
(
    uint_8 ControllerID     /* [IN] Serial Controller ID */ 
)
{
    PBUFFER psBuffer = g_serial_struct[ControllerID].buffer[RX_BUFFER];
    /* data received. read the SCI1D register for the data */

    if(psBuffer == NULL) 
    {         
        return;            
    }
    if (psBuffer->CurOffSet < psBuffer->Length)
    {
        static uint_8 val;
        val = SCI_ReadData(ControllerID);
       	
        /* Read another Byte */
        psBuffer->pBuffer[psBuffer->CurOffSet] = val;
	    psBuffer->CurOffSet++;
    }
    if(psBuffer->CurOffSet == psBuffer->Length)
    {  
        /* Data Receive Complete */
        psBuffer->InUse = FALSE;
        g_serial_struct[ControllerID].buffer[RX_BUFFER] = NULL;

        /* Inform the Upper layer of the receive complete */            
        if(g_serial_struct[ControllerID].app_callback)
        {
            g_serial_struct[ControllerID].app_callback(ControllerID, 
            SCI_RECEIVE_COMPLETE, psBuffer);
        }
    }    
}

/**************************************************************************//*!
 *
 * @name  SCI_ErrorISR
 *
 * @brief This function is called whenever an error interrupt is generated. 
 *        An error is generated whenever any receive or transmit fails.
 *
 * @param controller_ID :   Serial Controller ID
 *
 * @return None
 ******************************************************************************
 * This function is called whenever an error interrupt is generated. An error 
 * is generated whenever any receive or transmit fails.
 *****************************************************************************/
static void SCI_ErrorISR(
    uint_8 ControllerID     /* [IN] Serial Controller ID */ 
)
{
    uint_8 val;
    uint_8 Status = SCI_GetStatus(ControllerID);
    PBUFFER psBuffer = g_serial_struct[ControllerID].buffer[RX_BUFFER];
    uint_8 event;
    PSCI_REG pSciReg = g_sci_reg[ControllerID];
    /* Read Data Register To Clear Error Flag */
    val = SCI_ReadData(ControllerID);
        
	if(Status & MCF_UART_USR_OE)
	{
	    /* data lost as the buffer was not empty, previous data was not read */
	    event = SCI_RECEIVE_OVERRUN;
	}
	else if(Status & MCF_UART_USR_RB)
	{
	    /* noise detected in the received character */
	    event = SCI_RECEIVE_NOISE_DETECTED;        
	}
	else if(Status & MCF_UART_USR_FE)
	{
	    /* framing error */
	    event = SCI_RECEIVE_FRAME_ERROR;        
	}
	else if(Status & MCF_UART_USR_PE)
	{
	    /* parity error */
	    event = SCI_RECEIVE_PARITY_ERROR;        
	}
	pSciReg->WRITE.UCR = MCF_UART_UCR_RESET_ERROR;
    /* Inform the upper layer of the error occured */
    if(g_serial_struct[ControllerID].app_callback)
        g_serial_struct[ControllerID].app_callback(ControllerID, 
        event, psBuffer);
}
/**************************************************************************//*!
 *
 * @name  SCI_Interrupt
 *
 * @brief This function is called whenever an uart interrupt is generated. 
 *
 * @param controller_ID :   Serial Controller ID
 *
 * @return None
 ******************************************************************************
 * This function is called whenever an uart interrupt is generated.
 *****************************************************************************/
 static void SCI_Interrupt(
 	uint_8 ControllerID     /* [IN] Serial Controller ID */ 
 )
 {
 	uint_8 Status = SCI_GetStatus(ControllerID);
 	
 	/* Transmit Interrupt */
 	if(Status & MCF_UART_USR_TXRDY) 
 	{
 		SCI_TransmitISR(ControllerID);
 	}	
 		
 	/* Receiver Interrupt */
 	if (Status & MCF_UART_USR_RXRDY)
 	{
 		SCI_ReceiveISR(ControllerID);
 	}
 	
 	/* Error Interrupt */
 	if(Status & (MCF_UART_USR_RB | MCF_UART_USR_FE |
                 MCF_UART_USR_PE | MCF_UART_USR_OE))
    	SCI_ErrorISR(ControllerID);
    Status = SCI_GetStatus(ControllerID);
 }	
/**************************************************************************//*!
 *
 * @name  uart_gpio_init
 *
 * @brief This function intializes gpio support for uart
 *
 * @param channel	    :	uart channel support
 * 
 * @return none
 ******************************************************************************
 * This function intializes the gpio support
 *****************************************************************************/
static void uart_gpio_init(int channel)
{
#if(defined(__MCF52259_H__) || defined(__MCF52221_H__))
/* gpio settings for uart pins */
     switch (channel) 
	 {
   	  case 0:
   	     MCF_GPIO_PUAPAR |= (MCF_GPIO_PUAPAR_UTXD0_UTXD0 | MCF_GPIO_PUAPAR_URXD0_URXD0);
   	     break;
   	  case 1:
   	     MCF_GPIO_PUBPAR |= (MCF_GPIO_PUBPAR_UTXD1_UTXD1 | MCF_GPIO_PUBPAR_URXD1_URXD1);
   	     break;
#ifdef __MCF52259_H__
      case 2:
   	     MCF_GPIO_PUCPAR |= (MCF_GPIO_PUCPAR_UTXD2_UTXD2 | MCF_GPIO_PUCPAR_URXD2_URXD2);
         break;
#endif
	 } /* Endswitch */
#endif
#ifdef __MCF52277_H__ 
     switch (channel) {
    	case 0:
		   MCF_PAD_PAR_UART |=            MCF_PAD_PAR_UART_PAR_U0TXD_U0TXD |
		    							  MCF_PAD_PAR_UART_PAR_U0RXD_U0RXD;
		   break;
		case 1:
		   MCF_PAD_PAR_UART |=            MCF_PAD_PAR_UART_PAR_U1TXD_U1TXD |
		    							  MCF_PAD_PAR_UART_PAR_U1RXD_U1RXD ;
		   break;
		case 2:
		   MCF_PAD_PAR_DSPI &=            ~(MCF_PAD_PAR_DSPI_PAR_SOUT_SOUT |
		    				          	  MCF_PAD_PAR_DSPI_PAR_SIN_SIN);
		   MCF_PAD_PAR_DSPI |=            MCF_PAD_PAR_DSPI_PAR_SOUT(2)|
		                                  MCF_PAD_PAR_DSPI_PAR_SIN(2);   				          	  
		   break;	
	}
#endif	
}
/**************************************************************************//*!
 *
 * @name  UART0_ISR
 *
 * @brief The function initialize UART0 interrupts.
 *
 * @param None
 *
 * @return None
 *
 *****************************************************************************/
 
void __declspec(interrupt) UART0_ISR(void)
{
	SCI_Interrupt(0);
}
/**************************************************************************//*!
 *
 * @name  UART1_ISR
 *
 * @brief The function initialize UART1 interrupts.
 *
 * @param None
 *
 * @return None
 *
 *****************************************************************************/
void __declspec(interrupt) UART1_ISR(void)
{
	SCI_Interrupt(1);;
}
/**************************************************************************//*!
 *
 * @name  UART2_ISR
 *
 * @brief The function initialize UART2 interrupts.
 *
 * @param None
 *
 * @return None
 *
 *****************************************************************************/
void __declspec(interrupt) UART2_ISR(void)
{
	SCI_Interrupt(2);
}

/******************************************************************************
 * Global Function
 *****************************************************************************/
/**************************************************************************//*!
 *
 * @name  SCI_Init
 *
 * @brief This function intializes the Serial port
 *
 * @param pSerialInit	    :	Serial Class Initialization Structure
 * @param pfnSciCallback	:	Serial Class Callback function
 *
 * @return size of AVA_Type Data Structure
 ******************************************************************************
 * This function intializes the Serial port
 *****************************************************************************/
uint_8 SCI_Init (
    PSERIAL_INIT pSerialInit,       /* [IN] Serial Class Initialization 
                                        Structure */ 
    SCI_CALLBACK pfnSciCallback     /* [IN] Serial Class Callback function */ 
)
{               
    /* Enable SCI, transmit and receive interrupts */
    PSCI_REG pSciReg;
	static uint_16 divider;
	
	Int_Ctl_int_init(0, 12, 4,4, TRUE);
	Int_Ctl_int_init(0, 13, 3,3, TRUE);
	Int_Ctl_int_init(0, 14, 3,3, TRUE);
	
    /* Register Application Callback */
	g_serial_struct[pSerialInit->ControllerId].app_callback = pfnSciCallback;

    pSciReg = g_sci_reg[pSerialInit->ControllerId];
    
    uart_gpio_init(pSerialInit->ControllerId);
    
    pSciReg->WRITE.UCR = MCF_UART_UCR_RESET_RX;
    pSciReg->WRITE.UCR = MCF_UART_UCR_RESET_TX;
    pSciReg->WRITE.UCR = MCF_UART_UCR_RESET_ERROR;
    pSciReg->WRITE.UCR = MCF_UART_UCR_RESET_BKCHGINT;
    pSciReg->WRITE.UCR = MCF_UART_UCR_RESET_MR;
    
	/* No parity, 8-bits per character */
	pSciReg->WRITE.UMR = (0 | MCF_UART_UMR_PM_NONE | MCF_UART_UMR_BC_8);

	/* No echo or loopback, 1 stop bit */
	pSciReg->WRITE.UMR = (0 | MCF_UART_UMR_CM_NORMAL | MCF_UART_UMR_SB_STOP_BITS_1);  
		
    /* Set Rx and Tx baud by SYSTEM CLOCK */
	pSciReg->WRITE.UCSR = MCF_UART_UCSR_RCS_SYS_CLK | MCF_UART_UCSR_TCS_SYS_CLK;
    
    /* Mask all UART interrupt */                      
    pSciReg->WRITE.UIMR = 0; 
      
    /* Calculate divider */
    divider = (uint_16)((BUS_CLK*1000 +16*pSerialInit->BaudRate) / (32*pSerialInit->BaudRate));
   
    /* Set baud rate */
    pSciReg->WRITE.UBG1 = (uint8)((divider & 0xFF00) >> 8); 
    pSciReg->WRITE.UBG2 = (uint8)(divider & 0x00FF);
    
    /* Enable Tx an Rx */
    pSciReg->WRITE.UCR |= MCF_UART_UCR_RX_ENABLED;
    
    /* Enable FFULL interrupt */
    pSciReg->WRITE.UIMR |= MCF_UART_USR_FFULL;
    
    /* Inform the upper layer */
    if(pfnSciCallback)
    {
        pfnSciCallback(pSerialInit->ControllerId, SCI_CONNECT, NULL);
    }
    return SERIAL_OK;   
}
/**************************************************************************//*!
 *
 * @name  SCI_Class_Read
 *
 * @brief This function reads Serial Protocol data
 *
 * @param controller_ID	:	Serial Controller ID
 * @param sBuffer	    :	Pointer to the Receive Buffer
 *
 * @return SERIAL_OK        :   if Successful
 *         SERIAL_RX_BUSY   :   Unsuccessful
 ******************************************************************************
 * This function reads Serial Protocol data
 *****************************************************************************/
uint_8 SCI_Read(
    uint_8 ControllerID,    /* [IN] Serial Controller ID */ 
    PBUFFER sBuffer         /* [OUT] Pointer to the Receive Buffer */
)
{
    PBUFFER *psBuffer = &g_serial_struct[ControllerID].buffer[RX_BUFFER];
    if(*psBuffer != NULL)
        return SERIAL_RX_BSY;
        
    sBuffer->InUse = TRUE;
    *psBuffer = sBuffer;
    
    return SERIAL_OK;
}

/**************************************************************************//*!
 *
 * @name  SCI_Class_Write
 *
 * @brief This function sends data over Serial transport
 *
 * @param controller_ID	:	Serial Controller ID
 * @param sBuffer	    :	Pointer to the Send Buffer
 *
 * @return SERIAL_OK    :   if Successful
 *         SERIAL_BUSY  :   Unsuccessful
 ******************************************************************************
 * This function sends data over Serial transport
 *****************************************************************************/
uint_8 SCI_Write(
    uint_8 ControllerID,    /* [IN] Serial Controller ID */  
    PBUFFER sBuffer         /* [IN] Pointer to the Send Buffer */
)
{     
    if(g_serial_struct[ControllerID].buffer[TX_BUFFER] != NULL)
        return SERIAL_TX_BSY;
    g_serial_struct[ControllerID].buffer[TX_BUFFER] = sBuffer;
            
    sBuffer->InUse = TRUE;
    SCI_EnableTx(ControllerID);
    return SERIAL_OK;
}

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
 * @file serial.c
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
#include "serial.h"
#ifndef MC9S08
#include <file_struc.h>
#endif
/******************************************************************************
 * Macro's
 *****************************************************************************/
#define CONSOLE_BAUD_RATE   115200
#define CONSOLE_PORT        1
/******************************************************************************
 * Local Variables
 *****************************************************************************/
boolean g_console_init;
const PSCI_REG g_sci_reg[MAX_SCI_INTERFACES] = 
{
    (PSCI_REG)(SCI1_BASE_ADDRESS),
    (PSCI_REG)(SCI2_BASE_ADDRESS)    
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
static void SCI_DisableTx(
    uint_8 ControllerID     /* [IN] Serial Controller ID */
)
{
    PSCI_REG pSciReg = g_sci_reg[ControllerID];
    pSciReg->SCIC2 &= ~(SCI1C2_TCIE_MASK); 
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
static void SCI_EnableTx(
    uint_8 ControllerID     /* [IN] Serial Controller ID */
)
{
    PSCI_REG pSciReg = g_sci_reg[ControllerID];
    pSciReg->SCIC2 |= (SCI1C2_TCIE_MASK); 
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
static void SCI_WriteData(
    uint_8 ControllerID,    /* [IN] Serial Controller ID */ 
    uint_8 val              /* [IN] Data to be transmitted */
)
{
    PSCI_REG pSciReg = g_sci_reg[ControllerID];
    pSciReg->SCID = val;
    return;
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
    return pSciReg->SCID;
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
    return pSciReg->SCIS1;
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
static void SCI_TransmitISR(
    uint_8 ControllerID     /* [IN] Serial Controller ID */ 
)
{
    PBUFFER psBuffer = g_serial_struct[ControllerID].buffer[TX_BUFFER];
    uint_8 Status = SCI_GetStatus(ControllerID);
    
    if( Status & SCI1S1_TC_MASK)
    {
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
            volatile uint_8 val;
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
static void SCI_ReceiveISR(
    uint_8 ControllerID     /* [IN] Serial Controller ID */ 
)
{
    PBUFFER psBuffer = g_serial_struct[ControllerID].buffer[RX_BUFFER];
    uint_8 Status = SCI_GetStatus(ControllerID);

    if(Status & SCI1S1_RDRF_MASK)
    {
        /* data received. read the SCI1D register for the data */
        if(psBuffer == NULL) 
        {         
            return;            
        }
        if (psBuffer->CurOffSet < psBuffer->Length)
        {
            uint_8 val = SCI_ReadData(ControllerID);
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
    /* Read Data Register To Clear Error Flag */
    val = SCI_ReadData(ControllerID);
    
    if(Status & SCI1S1_OR_MASK)
    {
        /* data lost as the buffer was not empty, previous data was not read */
        /* read SCI1D in the callback to clear OR */
        event = SCI_RECEIVE_OVERRUN;
    }
    else if(Status & SCI1S1_NF_MASK)
    {
        /* noise detected in the received character */
        /* read SCI1D in the callback to clear NF */
        event = SCI_RECEIVE_NOISE_DETECTED;        
    }
    else if(Status & SCI1S1_FE_MASK)
    {
        /* framing error */
        /* read SCI1D in the callback to clear FE */
        event = SCI_RECEIVE_FRAME_ERROR;        
    }
    else if(Status & SCI1S1_PF_MASK)
    {
        /* parity error */
        /* read SCI1D in the callback to clear PF */
        event = SCI_RECEIVE_PARITY_ERROR;        
    }
    /* Inform the upper layer of the error occured */
    if(g_serial_struct[ControllerID].app_callback)
        g_serial_struct[ControllerID].app_callback(ControllerID, 
        event, psBuffer);

}

/**************************************************************************//*!
 *
 * @name  SCI1_Tx_ISR
 *
 * @brief This function is hooked onto the transmit complete interrupt of SCI1
 *
 * @param None
 *
 * @return None
 ******************************************************************************
 * This function is hooked onto the transmit complete interrupt of SCI1
 *****************************************************************************/
void interrupt VectorNumber_Vsci1tx SCI1_Tx_ISR(void)
{
    SCI_TransmitISR(0);
}
/**************************************************************************//*!
 *
 * @name  SCI1_Err_ISR
 *
 * @brief This function is hooked onto the error interrupt of SCI1
 *
 * @param None
 *
 * @return None
 ******************************************************************************
 * This function is hooked onto the error interrupt of SCI1
 *****************************************************************************/
void interrupt VectorNumber_Vsci1err SCI1_Err_ISR(void)
{
    SCI_ErrorISR(0);    
}
/**************************************************************************//*!
 *
 * @name  SCI1_Rx_ISR
 *
 * @brief This function is hooked onto the receive complete interrupt of SCI1
 *
 * @param None
 *
 * @return None
 ******************************************************************************
 * This function is hooked onto the receive complete interrupt of SCI1
 *****************************************************************************/
void interrupt VectorNumber_Vsci1rx SCI1_Rx_ISR(void)
{
    SCI_ReceiveISR(0);
}
/**************************************************************************//*!
 *
 * @name  SCI2_Tx_ISR
 *
 * @brief This function is hooked onto the transmit complete interrupt of SCI2
 *
 * @param None
 *
 * @return None
 ******************************************************************************
 * This function is hooked onto the transmit complete interrupt of SCI2
 *****************************************************************************/
void interrupt VectorNumber_Vsci2tx SCI2_Tx_ISR(void)
{
    SCI_TransmitISR(1);
}
/**************************************************************************//*!
 *
 * @name  SCI2_Err_ISR
 *
 * @brief This function is hooked onto the error interrupt of SCI2
 *
 * @param None
 *
 * @return None
 ******************************************************************************
 * This function is hooked onto the error interrupt of SCI2
 *****************************************************************************/
void interrupt VectorNumber_Vsci2err SCI2_Err_ISR(void)
{
    SCI_ErrorISR(1);    
}
/**************************************************************************//*!
 *
 * @name  SCI2_Rx_ISR
 *
 * @brief This function is hooked onto the receive complete interrupt of SCI2
 *
 * @param None
 *
 * @return None
 ******************************************************************************
 * This function is hooked onto the receive complete interrupt of SCI2
 *****************************************************************************/
void interrupt VectorNumber_Vsci2rx SCI2_Rx_ISR(void)
{
    SCI_ReceiveISR(1);
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
	uint_16 baud;
       
    /* Register Application Callback */
	g_serial_struct[pSerialInit->ControllerId].app_callback = pfnSciCallback;

    pSciReg = g_sci_reg[pSerialInit->ControllerId];
    baud = (uint_16)((BUS_CLK >> 4) / pSerialInit->BaudRate);
    pSciReg->SCIBDH = (uint_8)((baud >> 8) & 0x1F); 
    pSciReg->SCIBDL = (uint_8)(baud & 0xFF);

    pSciReg->SCIC1 = 0;
	
	
	/* Configure Mode */
	/* Default Mode is 8 bit mode */
    pSciReg->SCIC1 |= ((pSerialInit->Mode == 9) ? SCI1C1_M_MASK : 0) ;
	
	if (pSerialInit->ParityEnable == TRUE)
	{
		pSciReg->SCIC1 |= SCI1C1_PE_MASK;
		if(pSerialInit->Parity == 1)
		{
			pSciReg->SCIC1 |= SCI1C1_PT_MASK;
		}
	}
	/* Enable Address Mark Wake Up */
    pSciReg->SCIC1 |= SCI1C1_WAKE_MASK;
	/* Clear Status2 Register Write 1 to clear */
	pSciReg->SCIS2 = SCI1S2_LBKDIF_MASK | SCI1S2_RXEDGIF_MASK;
    pSciReg->SCIC2 = SCI1C2_TE_MASK | SCI1C2_RIE_MASK | SCI1C2_RE_MASK | 
        SCI1C2_RWU_MASK;
    pSciReg->SCIC3 = (SCI1C3_PEIE_MASK | SCI1C3_FEIE_MASK | SCI1C3_NEIE_MASK |
        SCI1C3_ORIE_MASK);
    
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

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
 * @file serial_cfv1_plus.c
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
#include "serial_cfv1_plus.h"
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
    (PSCI_REG)(SCI0_BASE_ADDRESS),
    (PSCI_REG)(SCI1_BASE_ADDRESS)    
};
SCI_STRUCT g_serial_struct[MAX_SCI_INTERFACES];

/******************************************************************************
 * Local Function Prototypes
 *****************************************************************************/
void interrupt 87 UART3_RTx_ISR(void);
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
    pSciReg->UART_C2 &= ~(UART_C2_TCIE_MASK); 
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
    pSciReg->UART_C2 |= (UART_C2_TCIE_MASK); 
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
    pSciReg->UART_D = val;
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
    return pSciReg->UART_D;
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
    return pSciReg->UART_S1;
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
    
    if(Status & UART_S1_OR_MASK)
    {
        /* data lost as the buffer was not empty, previous data was not read */
        /* read SCI1D in the callback to clear OR */
        event = SCI_RECEIVE_OVERRUN;
    }
    else if(Status & UART_S1_NF_MASK)
    {
        /* noise detected in the received character */
        /* read SCI1D in the callback to clear NF */
        event = SCI_RECEIVE_NOISE_DETECTED;        
    }
    else if(Status & UART_S1_FE_MASK)
    {
        /* framing error */
        /* read SCI1D in the callback to clear FE */
        event = SCI_RECEIVE_FRAME_ERROR;        
    }
    else if(Status & UART_S1_PF_MASK)
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
void interrupt 87 UART3_RTx_ISR(void)
{
	uint_8 Status = SCI_GetStatus(0);
	if( Status & UART_S1_TC_MASK)
		SCI_TransmitISR(0);
	if(Status & UART_S1_RDRF_MASK)
		SCI_ReceiveISR(0);
	if(Status & (UART_S1_OR_MASK | UART_S1_NF_MASK |
						UART_S1_FE_MASK | UART_S1_PF_MASK))
		SCI_ErrorISR(0);
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
	uint_32 	baud_divisor;
	uint_32	fractional_fine_adjust;

	/* Enable clock source for UART0 module */
	SIM_SCGC1 |= SIM_SCGC1_UART0_MASK;
	
	/* Enable Port Muxing of Uart0 */
	MXC_PTAPF1 &= ~MXC_PTAPF1_A7(0xF);
	MXC_PTAPF1 |= MXC_PTAPF1_A7(0x2);
	MXC_PTDPF1 &=~ MXC_PTDPF1_D6(0xF);
	MXC_PTDPF1 |= MXC_PTDPF1_D6(0x2);

    /* Register Application Callback */
	g_serial_struct[pSerialInit->ControllerId].app_callback = pfnSciCallback;

    pSciReg = g_sci_reg[pSerialInit->ControllerId];
    
    /* Calculate baudrate divisor and fractional fine adjust values */
    fractional_fine_adjust = (uint_32)(SYSTEM_CLOCK/(CONSOLE_BAUD_RATE/2));
	baud_divisor = (uint_32)(fractional_fine_adjust >>5);
	fractional_fine_adjust -= (baud_divisor <<5);
	/* Set baudrate */
    pSciReg->UART_BDH |= ((baud_divisor>>8) & UART_BDH_SBR_MASK);
    pSciReg->UART_BDL = ((uint_8)(baud_divisor & 0xFF));
    pSciReg->UART_C4  |= UART_C4_BRFA(fractional_fine_adjust);
    /* 8 bits data, no parity */
    pSciReg->UART_C1 = 0;

    /* Configure Mode */
	/* Default Mode is 8 bit mode */
    pSciReg->UART_C1 |= ((pSerialInit->Mode == 9) ? UART_C1_M_MASK : 0) ;
	
	if (pSerialInit->ParityEnable == TRUE)
	{
		pSciReg->UART_C1 |= UART_C1_PE_MASK;
		if(pSerialInit->Parity == 1)
		{
			pSciReg->UART_C1 |= UART_C1_PT_MASK;
		}
	}
	/* Enable Address Mark Wake Up */
    pSciReg->UART_C1 |= UART_C1_WAKE_MASK;
	/* Clear Status2 Register Write 1 to clear */
	pSciReg->UART_S2 = UART_S2_LBKDIF_MASK | UART_S2_RXEDGIF_MASK;
    pSciReg->UART_C2 = (UART_C2_TE_MASK | UART_C2_RIE_MASK | UART_C2_RE_MASK | UART_C2_TCIE_MASK |UART_C2_RWU_MASK);
    pSciReg->UART_C3 = (UART_C3_PEIE_MASK | UART_C3_FEIE_MASK | UART_C3_NEIE_MASK |UART_C3_ORIE_MASK);
    
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

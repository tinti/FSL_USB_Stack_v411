/*****************************************************************************
* IIC Serial Port implementation.
*
* (c) Copyright 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/
#include "psptypes.h"
#include "IIC_cfv2.h"
#include "derivative.h"


/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/
 /* Define the IIC ports */

#if defined(_MCF51MM256_H)

#define mIIC_A_c IICA
#define mIIC_F_c IICF
#define mIIC_C_c IICC
#define mIIC_S_c IICS
#define mIIC_D_c IICD
#define mIIC_C_2 IICC2

#endif

#if defined(_MCF51JM128_H)

#define mIIC_A_c IIC2A
#define mIIC_F_c IIC2F
#define mIIC_C_c IIC2C
#define mIIC_S_c IIC2S
#define mIIC_D_c IIC2D
#define mIIC_C_2 IIC2C2
#endif

#if defined(__MCF52259_H__)

#define mIIC_A_c MCF_I2C0_I2ADR
#define mIIC_F_c MCF_I2C0_I2FDR
#define mIIC_C_c MCF_I2C0_I2CR
#define mIIC_S_c MCF_I2C0_I2SR
#define mIIC_D_c MCF_I2C0_I2DR
#endif



/* Bits in IIC Control Register. Read/Write. */
/* IICC at 0x005A. */
#define mIICC_IICEN_c       0x80        /* IIC Enable */
#define mIICC_IICIE_c       0x40        /* IIC Interrupt Enable */
#define mIICC_MST_c         0x20        /* Master Mode Select */
#define mIICC_TX_c          0x10        /* Transmit Mode Select */
#define mIICC_TXAK_c        0x08        /* Transmit Acknowledge Enable */
#define mIICC_RSTA_c        0x04        /* Repeat START */

/* Bits in IIC Status Register. Read/Write. */
/* IIC1S at 0x005B. */
#define mIICS_TCF_c         0x80        /* Transfer Complete Flag */
#define mIICS_IAAS_c        0x40        /* Addressed as a Slave */
#define mIICS_BUSY_c        0x20        /* Bus Busy Flag */
#define mIICS_ARBL_c        0x10        /* Arbitration Lost */
#define mIICS_SRW_c         0x04        /* Slave Read/Write Flag */
#define mIICS_IICIF_c       0x02        /* IIC Interrupt Flag */
#define mIICS_RXAK_c        0x01        /* Receive Acknowledge */




/* Initialize IIC Control Register so that:
*   - IIC is enabled
*   - configured for Slave Mode */
#define mIICC_Init_c        (mIICC_IICEN_c)


#define mIICC_PS_c          (1<<5)

#if (defined(_MCF51MM256_H) || defined(_MCF51JM128_H))
/* Initialize IICxC2 Control Register so that:
*   - General call address is disabled
*   - 7 bit address scheme  */
#define mIICxC2_Init_c        0
#endif

/* Initialize IIC Status Register so that:
*   - Busy flag is cleared
*   - R/W bit = 0
*   - Interrupt flag is cleared
*   - Arbitration Lost flag is cleared */
#define mIICS_Init_c        (mIICS_ARBL_c | mIICS_IICIF_c)

/* Turn everything off. */
#define mIICC_Reset_c     0x00

/* Number of elements in an array. */
#ifndef NumberOfElements
#define NumberOfElements(array)     ((sizeof(array) / (sizeof(array[0]))))
#endif



/*****************************************************************************
******************************************************************************
* Private type definitions
******************************************************************************
*****************************************************************************/
/* The transmit code keeps a circular list of buffers to be sent */
/*****************************************************************************
******************************************************************************
* Public memory definitions
******************************************************************************
*****************************************************************************/


/*****************************************************************************
******************************************************************************
* Private memory definitions
******************************************************************************
*****************************************************************************/

/*****************************************************************************
******************************************************************************
* Private prototypes
******************************************************************************
*****************************************************************************/




/*****************************************************************************
******************************************************************************
* Public functions
******************************************************************************
*****************************************************************************/

/*****************************************************************************
*   IIC_ModuleInit 
*
*   Initializes the I2C module 
******************************************************************************/
void IIC_ModuleInit(void)
{

  /* Configure the I2C hardware peripheral */
 
 MCF_GPIO_PASPAR = (uint_8)((MCF_GPIO_PASPAR  & ~0x0F) | 0x05);
 mIIC_C_c = mIICC_Reset_c;
 mIIC_S_c &= ~mIICS_Init_c;
 mIIC_A_c = (gIIC_DefaultSlaveAddress_c << 1);
 mIIC_F_c = 0x0E;
 mIIC_C_c = mIICC_Init_c; 
}




/*****************************************************************************
*   IIC_ModuleUninit 
*
*   Resets the I2C module.
******************************************************************************/
void IIC_ModuleUninit(void)
{

  mIIC_C_c = mIICC_Reset_c;

}
/*****************************************************************************
*   IIC_SetBaudRate 
*
*   Resets the I2C module.
******************************************************************************/
boolean IIC_SetBaudRate(uint_8 baudRate)
{

   if(mIIC_S_c & mIICS_BUSY_c)
    {
     return FALSE; 
    }
   mIIC_F_c = baudRate;
   return TRUE;
}


/*****************************************************************************
*   IIC_SetSlaveAddress 
*
*   Sets the slave address of the I2C module.
******************************************************************************/
boolean IIC_SetSlaveAddress(uint_8 slaveAddress)
{
  /* Check if the I2C address is valid */
  if((slaveAddress > 0x7f) || 
     (((slaveAddress & 0x78) == 0) && ((slaveAddress & 0x07) != 0)) || 
     ((slaveAddress & 0x78) == 0x78))
  {
    return FALSE;
  }
  {
    mIIC_A_c = (uint_8)(slaveAddress << 1);
    return TRUE;
  }
}


/*****************************************************************************
*   IIC_BusRecovery 
*
*   Resets the I2C module.
******************************************************************************/
void IIC_BusRecovery(void)
{

   uint_8 iicControlReg;
   iicControlReg = mIIC_C_c;
   mIIC_C_c = mIICC_Reset_c;
   mIIC_C_c = mIICC_MST_c;
   mIIC_C_c |= mIICC_IICEN_c;
   mIIC_S_c = mIICS_Init_c;
   mIIC_D_c;
   while((mIIC_S_c & mIICS_IICIF_c) == 0){}
   mIIC_C_c &= ~mIICC_MST_c;
   mIIC_S_c = mIICS_Init_c;
   mIIC_C_c = iicControlReg;
   

}





/*****************************************************************************
*   IIC_Transmit_Master 
*
*   Begin transmitting size bytes of data from *pBuffer.
*   Returns FALSE if there are no more slots in the buffer reference table.
******************************************************************************/
boolean IIC_Transmit_Master(uint_8 const *pBuf, uint_32 bufLen, uint_8 destAddress) 
{
  if(bufLen == 0)
    {
    return TRUE;
    }
  destAddress <<= 1;
  if(destAddress == mIIC_A_c)
    {
    return FALSE;
    }
  if(mIIC_S_c & mIICS_BUSY_c)
    {
     return FALSE; 
    }
    
  mIIC_S_c = mIICS_Init_c;    
  mIIC_C_c |= (mIICC_MST_c | mIICC_TX_c);// start condition
  mIIC_D_c = destAddress; // address the slave for writting
  while(TRUE)
    {
    while(!(mIIC_S_c & mIICS_IICIF_c)){}
    mIIC_S_c &= ~mIICS_IICIF_c;// clear interrupt
  /* Check arbitration  and slave addressing*/
    if(mIIC_S_c & (mIICS_ARBL_c | mIICS_IAAS_c ))
      {
      if(mIIC_S_c & mIICS_IAAS_c)
        {
         /* Disable TX operation.Writing to IICxC register also clears IAAS bit */
         mIIC_C_c &= ~(mIICC_TX_c);
        }
            
      if(mIIC_S_c & mIICS_ARBL_c)
        {
         /* Clear Arbitration lost Flag */
         mIIC_S_c &= ~ mIICS_ARBL_c;    
        }
      return FALSE;   
      } 
    if(mIIC_S_c & mIICS_RXAK_c)// No ack received
      {
       mIIC_C_c &= ~(mIICC_MST_c | mIICC_TX_c);
       return FALSE;   
      }
    else	// Ack received
      {
      if(bufLen)
        {
        mIIC_D_c = *pBuf++ ;
        bufLen--;
        }
      else
        {
        mIIC_C_c &= ~(mIICC_MST_c | mIICC_TX_c); 
        //while(mIIC_S_c & mIICS_BUSY_c);
        return TRUE;
        }
      }
    }
} 

/*****************************************************************************
*   IIC_Receive_Master 
*
*   Begin transmitting size bytes of data from *pBuffer.
*   Returns FALSE if there are no more slots in the buffer reference table.
******************************************************************************/
boolean IIC_Receive_Master(uint_8 *pBuf, uint_32 bufLen, uint_8 destAddress) 
{
 /* Handle empty buffers. */
 if (bufLen == 0) 
   {
    return TRUE;
   }
 if(pBuf == NULL)
   {
   return FALSE;
   }
  
 destAddress <<= 1;
 if(destAddress == mIIC_A_c)
   {
   return FALSE;
   }
 if(mIIC_S_c & mIICS_BUSY_c)
   {
    return FALSE; 
   }    
 mIIC_S_c &= ~mIICS_Init_c;         
 mIIC_C_c |= (mIICC_MST_c | mIICC_TX_c);// start condition
 mIIC_D_c = (uint_8)(destAddress  | 0x1); // address the slave for reading
 while(TRUE)
   {
    while(!(mIIC_S_c & mIICS_IICIF_c)){}
    /* Clear the interrupt request */
    mIIC_S_c &= ~mIICS_IICIF_c;
    /* Check arbitration  and slave addressing*/
    if (mIIC_S_c & (mIICS_ARBL_c | mIICS_IAAS_c ))
      {
      if(mIIC_S_c & mIICS_IAAS_c)
        {
         /* Clear Tx Operation .Writing to IICxC register also clears IAAS bit */
         mIIC_C_c &= ~(mIICC_TX_c);
        }
      if(mIIC_S_c & mIICS_ARBL_c)
        {
        /* Arbitration lost */
        mIIC_S_c &= ~ mIICS_ARBL_c;    
        }
      return FALSE;  
      }
  
    /* Arbitration okay
       Check addressing */
 
    if(mIIC_C_c & mIICC_TX_c)
      {
      if(mIIC_S_c & mIICS_RXAK_c)// No ack received
        {
         mIIC_C_c &= ~(mIICC_MST_c | mIICC_TX_c);
         return FALSE;
        
        }
      else	// Ack received
        {
        mIIC_C_c &= ~(mIICC_TX_c | mIICC_TXAK_c);
        if(bufLen-- == 1)  
          {
          mIIC_C_c |= mIICC_TXAK_c;
          }
        mIIC_D_c;  
                
        }
      }
    else// (mIIC_C_c & mIICC_TX_c) == 0       
      {
       if(bufLen == 0)   
        {
        mIIC_C_c &= ~mIICC_MST_c;   
        
        }
       else if(bufLen == 1)  
        {
         mIIC_C_c |= mIICC_TXAK_c;
        }
       *pBuf++ = mIIC_D_c;
        if(bufLen == 0)   
        {
        return TRUE;
        }
       bufLen--;
      }
   }
} 






/*****************************************************************************
*   IIC_Receive_Master 
*
*   Begin transmitting size bytes of data from *pBuffer.
*   Returns FALSE if there are no more slots in the buffer reference table.
******************************************************************************/
boolean IIC_Transmit_RS_Receive_Master(uint_8 *p_tx_Buf, uint_32 txLen,uint_8 *p_rx_Buf, uint_32 rxLen, uint_8 destAddress) 
{
 /* Handle empty buffers. */
 if ((txLen == 0) || (rxLen == 0)) 
   {
    return FALSE;
   }
 if(p_rx_Buf == NULL)
   {
   return FALSE;
   }
  
 destAddress <<= 1;
 if(destAddress == mIIC_A_c)
   {
   return FALSE;
   }
 if(mIIC_S_c & mIICS_BUSY_c)
   {
    return FALSE; 
   }    
 
  mIIC_S_c &= ~ mIICS_Init_c;    
  mIIC_C_c |= (mIICC_MST_c | mIICC_TX_c);// start condition
  mIIC_D_c = destAddress; // address the slave for writting
  while(TRUE)
    {
     while(!(mIIC_S_c & mIICS_IICIF_c)){}
     mIIC_S_c &= ~ mIICS_IICIF_c;// clear interrupt
     /* Check arbitration  and slave addressing*/
     if(mIIC_S_c & (mIICS_ARBL_c | mIICS_IAAS_c ))
      {
        if(mIIC_S_c & mIICS_IAAS_c)
        {
         /* Disable TX operation.Writing to IICxC register also clears IAAS bit */
         mIIC_C_c &= ~(mIICC_TX_c);
        }
        if(mIIC_S_c & mIICS_ARBL_c)
        {
         /* Clear Arbitration lost Flag */
         mIIC_S_c &= ~ mIICS_ARBL_c;    
        }
       return FALSE;   
      } 
    if(mIIC_S_c & mIICS_RXAK_c)// No ack received
      {
       mIIC_C_c &= ~(mIICC_MST_c | mIICC_TX_c);
       return FALSE;   
      }
    else	// Ack received
      {
      if(txLen)
        {
        mIIC_D_c = *p_tx_Buf++ ;
        txLen--;
        }
      else
        {
        break;
        }
      }
    }
  
 mIIC_C_c |= mIICC_RSTA_c;// repeat start condition
 mIIC_D_c = (uint_8)(destAddress  | 0x1); // address the slave for reading
 while(TRUE)
   {
    while(!(mIIC_S_c & mIICS_IICIF_c)){}
    /* Clear the interrupt request */
    mIIC_S_c &= ~ mIICS_IICIF_c;
    /* Check arbitration  and slave addressing*/
    if (mIIC_S_c & (mIICS_ARBL_c | mIICS_IAAS_c ))
      {
      if(mIIC_S_c & mIICS_IAAS_c)
        {
         /* Clear Tx Operation .Writing to IICxC register also clears IAAS bit */
         mIIC_C_c &= ~(mIICC_TX_c);
        }
      if(mIIC_S_c & mIICS_ARBL_c)
        {
        /* Arbitration lost */
        mIIC_S_c &= ~ mIICS_ARBL_c;    
        }
      return FALSE;  
      }
  
    /* Arbitration okay
       Check addressing */
 
    if(mIIC_C_c & mIICC_TX_c)
      {
      if(mIIC_S_c & mIICS_RXAK_c)// No ack received
        {
         mIIC_C_c &= ~(mIICC_MST_c | mIICC_TX_c);
         return FALSE;
        
        }
      else	// Ack received
        {
        mIIC_C_c &= ~(mIICC_TX_c | mIICC_TXAK_c);
        if(rxLen-- == 1)  
          {
          mIIC_C_c |= mIICC_TXAK_c;
          }
        mIIC_D_c;  
                
        }
      }
    else// (mIIC_C_c & mIICC_TX_c) == 0       
      {
       if(rxLen == 0)   
        {
        mIIC_C_c &= ~mIICC_MST_c;   
        
        }
       else if(rxLen == 1)  
        {
         mIIC_C_c |= mIICC_TXAK_c;
        }
       *p_rx_Buf++ = mIIC_D_c;
        if(rxLen == 0)   
        {
        return TRUE;
        }
       rxLen--;
      }
   }
} 

/*****************************************************************************
*   IIC_Bus_Busy 
*
*   Begin transmitting size bytes of data from *pBuffer.
*   Returns FALSE if there are no more slots in the buffer reference table.
******************************************************************************/
boolean  IIC_Bus_Busy(void)
{
return  (boolean) ((mIIC_S_c & mIICS_BUSY_c) ?TRUE:FALSE);
}



#if 0
/*****************************************************************************
*  IIC_Isr
*
*  I2C Interrupt Service Routine.
******************************************************************************/
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

INTERRUPT_KEYWORD void IIC_Isr(void)
{

#if gIIC_Enabled_d

  /* Clear the interrupt request */
  mIIC_S_c |= mIICS_IICIF_c;
  /* Check arbitration  and slave addressing*/
  if (mIIC_S_c & (mIICS_ARBL_c | mIICS_IAAS_c ))
  {
    if (mIIC_S_c & mIICS_IAAS_c)
      {
      /* Check if I2C module was addressed for read or for write */
      if(mIIC_S_c & mIICS_SRW_c)
        {
         /* Configure I2C module for Tx operation.Writing to IICxC register also clears IAAS bit */
         mIIC_C_c |= mIICC_TX_c;
         /* Send next byte from the current Tx buffer */
         IIC_SendNextByte();
        }
      else
        {
         /* Configure I2C module for Rx operation.Writing to IICxC register also clears IAAS bit */
         mIIC_C_c &= ~(mIICC_TX_c | mIICC_TXAK_c);
         mIIC_D_c;
        }

      }
    if(mIIC_S_c & mIICS_ARBL_c)
      {
      /* Arbitration lost */
       mIIC_S_c |= mIICS_ARBL_c;    
       
       if(mIICMasterOp.iicOpType == mIIC_OpType_Tx_c)
        {
         if(pfIIcMasterTxCallBack)
          {
           TS_SendEvent(gIIcTaskId, gIIC_Event_MasterTxFail_c); 
          }
          
        }
       else
        {
        if(pfIIcMasterRxCallBack)
          {
          TS_SendEvent(gIIcTaskId, gIIC_Event_MasterRxFail_c);    
          }
        
        }
      }
  }
  else
  {
    /* Arbitration okay
       Check addressing */
  if(mIIC_C_c & mIICC_MST_c)/* Master mode */
    {
     if(mIIC_C_c & mIICC_TX_c)
      {
        if(mIIC_S_c & mIICS_RXAK_c)// No ack received
          {
            mIIC_C_c &= ~(mIICC_MST_c | mIICC_TX_c);
            if(mIICMasterOp.iicOpType == mIIC_OpType_Tx_c)	// Tx
              {
              if(pfIIcMasterTxCallBack)
               {
                TS_SendEvent(gIIcTaskId, gIIC_Event_MasterTxFail_c); 
               }
              }
            else	// Rx
              {
              if(pfIIcMasterRxCallBack)
               {
                TS_SendEvent(gIIcTaskId, gIIC_Event_MasterRxFail_c);    
               }
              }
          }
        else	// Ack received
          {
            if(mIICMasterOp.iicOpType == mIIC_OpType_Rx_c)	// Rx
              {
              mIIC_C_c &= ~(mIICC_TX_c | mIICC_TXAK_c);
              if(mIICMasterOp.bufLen-- == 1)  
                {
                 mIIC_C_c |= mIICC_TXAK_c;
                }
              mIIC_D_c;  
              }
            else	// Tx
              {
               if(mIICMasterOp.bufLen)
                {
                 mIIC_D_c =  *mIICMasterOp.pBuf++ ;
                 mIICMasterOp.bufLen--;
                }
               else
                {
                 mIIC_C_c &= ~(mIICC_MST_c | mIICC_TX_c); 
                 if(pfIIcMasterTxCallBack)
                  {
                   TS_SendEvent(gIIcTaskId, gIIC_Event_MasterTxSuccess_c); 
                  }
                 
                }
              }
          }
      }
     else// (mIIC_C_c & mIICC_TX_c) == 0       
      {
       if(mIICMasterOp.bufLen == 0)   
        {
        mIIC_C_c &= ~mIICC_MST_c;   
        if(pfIIcMasterRxCallBack)
          {
           TS_SendEvent(gIIcTaskId, gIIC_Event_MasterRxSuccess_c); 
          }
        }
       else if(mIICMasterOp.bufLen-- == 1)  
        {
         mIIC_C_c |= mIICC_TXAK_c;
        }
       *mIICMasterOp.pBuf++ = mIIC_D_c;
        
      }
     
    }
    
  else   /* slave mode*/
    {
    if (mIIC_C_c & mIICC_TX_c)
      {
        /* IIC has Tx a byte to master. Check if ack was received */
        if (mIIC_S_c & mIICS_RXAK_c)
        {
          /* No ack received. Switch back to receive mode */
          mIIC_C_c &= ~mIICC_TX_c;
          mIIC_D_c;
        }
        else
        {
          /* Ack received. Send next byte */
          IIC_SendNextByte();
        }
      }
    else
      {
        /* Put the received byte in the buffer */
        if(pfIIcSlaveRxCallBack)
          {
          maIIcRxBuf[mIIcRxBufLeadingIndex] = mIIC_D_c;

          if (++mIIcRxBufLeadingIndex >= sizeof(maIIcRxBuf)) 
            {
            mIIcRxBufLeadingIndex = 0;
            }

          if (mIIcRxBufferByteCount < sizeof(maIIcRxBuf)) 
            {
            ++mIIcRxBufferByteCount;
            }
          else
            {
             if (++mIIcRxBufTrailingIndex >= sizeof(maIIcRxBuf)) 
               {
                mIIcRxBufTrailingIndex = 0;
               }
            }
            
        /* Let the application know a byte has been received. */
          TS_SendEvent(gIIcTaskId, gIIC_Event_SlaveRx_c);
            
          }
        else
          {
           mIIC_D_c; 
          }
      }  
    }/* Data transfer.Check if it is a Tx or Rx operation */
      
    
  }
  
#endif  
}
#pragma CODE_SEG DEFAULT           
#endif

/*****************************************************************************
******************************************************************************
* Private functions
******************************************************************************
*****************************************************************************/


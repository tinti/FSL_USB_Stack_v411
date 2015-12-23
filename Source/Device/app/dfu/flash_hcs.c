/********************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
*
**************************************************************************//*!
 *
 * @file flash_hcs.c
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief The file includes flash routines
 *
 *****************************************************************************/

/*********************************** Includes ***********************************/
#include <hidef.h>      /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "flash_hcs.h"  /* include flash driver header file */
/* Define DCDIV registor for MC9S08MM128 and MC9S08JE128 */
#if (defined(_MC9S08MM128_H)||defined(_MC9S08JE128_H))
#define FCDIV_DIVLD FCDIV_FDIVLD
#endif

/*********************************** Macros ************************************/

/*********************************** Constant **********************************/

/*********************************** Variables *********************************/

/*********************************** Prototype *********************************/

/*********************************** Function **********************************/


/*******************************************************************************
 * Function:        Flash_Init
 *
 * Description:     Set the flash clock
 *
 * Returns:         never return
 *
 * Notes:
 *
 *******************************************************************************/
void Flash_Init(unsigned char FlashClockDiv)
{
    /* Clear Error bits */
    FSTAT = 0x30;
    /* Flash clock between 150-200kHz - > 8MHz/(39+1)=200kHz*/
#if (defined(_MC9S08MM128_H)||defined(_MC9S08JE128_H))
    FCDIV = 49;
#else
    FCDIV = FlashClockDiv;
#endif
}
/*******************************************************************************
 * Function:        Flash_SectorErase
 *
 * Description:     erase a sector of the flash
 *
 * Returns:         Error Code
 *
 * Notes:
 *
 *******************************************************************************/
unsigned char Flash_SectorErase(unsigned char *FlashPtr)
{
    unsigned char Return = Flash_OK;

    /* Allocate space on stack to run flash command out of SRAM */
    char localbuf[100];
    int (*RunOnStack)(void) = (int(*)(void))localbuf;
    (void)memcpy(localbuf, (void*)SpSub, (char*)SpSubEnd - (char*)SpSub);

    if (FCDIV_DIVLD ==1) /* flash is init */
    {
        /* wait until FCBEF is set in FSTAT */
        while (FSTAT_FCBEF == 0) {}
        /* check for errors */
        if (FSTAT_FACCERR == 1)
        {
            /* clear error flag */
            FSTAT = 0x10;
        }
        if (FSTAT_FPVIOL == 1)
        {
            /* clear error flag */
            FSTAT = 0x20;
        }
        /* dummy write to flash */
        *FlashPtr = (unsigned char)0xFF;

        /* write command */
        FCMD = FlashCmd_SectorErase;

        (void)RunOnStack();

        /* check for errors */
        if (FSTAT_FACCERR == 1)
        {
            /* clear error flag */
            FSTAT = 0x10;

            /* update return value*/
            Return |= Flash_FACCERR;
        }

        if (FSTAT_FPVIOL == 1)
        {
            /* clear error flag */
            FSTAT = 0x20;

            /* update return value*/
            Return |= Flash_FPVIOL;
        }
    }
    else
    {
        /* flash is not init */
        Return = Flash_NOT_INIT;
    }
    /* function return */
    return  Return;
}
/*******************************************************************************
 * Function:        Flash_ByteProgram
 *
 * Description:     byte program the flash
 *
 * Returns:         Error Code
 *
 * Notes:
 *
 *******************************************************************************/
unsigned char Flash_ByteProgram
(
    unsigned char *FlashStartAdd,
    unsigned char *DataSrcPtr,
    unsigned short NumberOfBytes
)
{
    unsigned char Return = Flash_OK;
    /* Allocate space on stack to run flash command out of SRAM */
    char localbuf[100];
    int (*RunOnStack)(void) = (int(*)(void))localbuf;
    (void)memcpy(localbuf, (void*)SpSub, (char*)SpSubEnd - (char*)SpSub);

    if (FCDIV_DIVLD == 1)
    {
        /* flash is init */

        /* wait until FCBEF is set in FSTAT */
        while (FSTAT_FCBEF == 0) {}

        /* check for errors */
        if (FSTAT_FACCERR == 1)
        {
            /* clear error flag */
            FSTAT = 0x10;
        }
        if (FSTAT_FPVIOL == 1)
        {
            /* clear error flag */
            FSTAT = 0x20;
        }

        while ((NumberOfBytes) && (Return == Flash_OK))
        {
            /* write data to flash and increase pointers by 1 */
            *FlashStartAdd++ = *DataSrcPtr++;

            /* write command */

            FCMD = FlashCmd_Program;

            (void)RunOnStack();

            /* check for errors */
            if (FSTAT_FACCERR == 1)
            {
                /* clear error flag */
                FSTAT = 0x10;

                /* update return value*/
                Return |= Flash_FACCERR;
            }
            if (FSTAT_FPVIOL == 1)
            {
                /* clear error flag */
                FSTAT = 0x20;

                /* update return value*/
                Return |= Flash_FPVIOL;
            }
            /* decrement byte count */
            NumberOfBytes--;
        }
    }
    else
    {
        /* flash is not init */
        Return = Flash_NOT_INIT;
    }

    /* function return */
    return  Return;
}

/*******************************************************************************
 * Function:        SpSub
 *
 * Description:     Execute the Flash write while running out of SRAM
 *
 * Returns:
 *
 * Notes:
 *
 *******************************************************************************/
void SpSub(void)
{
    /* launch command */
    FSTAT_FCBEF = 1;
    /* wait for command completion */
    while (!(FSTAT_FCBEF & FSTAT_FCCF)) {};
}
/* Leave this immediately after SpSub */
void SpSubEnd(void) {}

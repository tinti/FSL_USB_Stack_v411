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
 * @file audio_pwm_cfv1_plus.h
 *
 * @author
 *
 * @version
 *
 * @date    
 *
 * @brief   The file is a header file for PWM
 *****************************************************************************/

#ifndef _AUDIO_PWM__CFV1_HEADER_H
#define _AUDIO_PWM__CFV1_HEADER_H
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "derivative.h"     /* include peripheral declarations */
#include "types.h"          /* Contains User Defined Data Types */
#include "user_config.h"    /* User Defined Configuration Parameters */

/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/
#if (!defined(__MCF52xxx_H__))
#if (defined _MC9S08JS16_H)
#define duty TPMC1V
#else
#define duty TPM1C2V
#endif
#endif
/******************************************************************************
 * Types
 *****************************************************************************/

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
extern void pwm_init(void);

#endif /* _AUDIO_PWM__CFV1_HEADER_H */

/*
 * (C) Copyright 2015-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2015-2020 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  ------------ ----------------------------------------------------
 1.00                Vanessa Chen Init version.
 1.01      16/08/22  Vanessa Chen Support CPU turbo On/Off SW SMI 0xDA
*/

#ifndef _COMPAL_SMM_PLATFORM_H_
#define _COMPAL_SMM_PLATFORM_H_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/GpioLib.h>
#include <Library/PcdLib.h>						
#include <Protocol/SmmGpiDispatch2.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmBase2.h>
#if (EC_PLATFORM_SETTING == 2 )                 // 2:eSPI mode
#include <Protocol/PchEspiSmiDispatch.h>
#include <Register/PchRegsPmc.h>
#include <Library/PchCycleDecodingLib.h>
#endif
#include <Library/PciCf8Lib.h>
#include <CpuAccess.h>
#include <Library/BaseLib.h>
#include <CompalEcCommonCmd.h>
#include <Compalthermallib.h>
#include <CompalEcLib.h>
#include <CompalSwSmi.h>
#include "ProjectDefinition.h"

#define R_PCH_SMI_EN                    0x30
#define B_PCH_SMI_EN_ESPI               BIT28 
#define MSR_MISC_ENABLE                 0x1a0

//**********************************************************
// Compal Common EC SMI event number.
//**********************************************************
#define EC_SMI_KBRST 0x26

typedef struct {
    UINT8   EcSmiFunNo;
    VOID    (*EcSmiFunService)();
} EC_SMI_FUNCTIONS;


VOID
CompalEcSmiKbRst(
  VOID
);

EFI_STATUS
CompalEcSmiEventOEM(
	IN       UINT8								EcSmiEvent
);

EFI_STATUS
EFIAPI
CompalEcSmiService(
	IN		 EFI_HANDLE 						DispatchHandle,
	IN		 CONST EFI_SMM_GPI_REGISTER_CONTEXT *GpiRegisterContext,
	IN OUT	 EFI_SMM_GPI_REGISTER_CONTEXT		*CurrentContext,
	IN OUT	 UINTN								*SizeOfContext
);

VOID
EFIAPI
CompalESPISmiService(
	IN		 EFI_HANDLE 						DispatchHandle
);
#endif

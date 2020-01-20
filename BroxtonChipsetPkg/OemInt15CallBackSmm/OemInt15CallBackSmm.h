/** @file
  Definition for Oem Int 15 SMM Callback.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#ifndef _OEM_INT15_CALLBACK_SMM_H_
#define _OEM_INT15_CALLBACK_SMM_H_

#include <PiSmm.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Protocol/LegacyBios.h>
#include <Protocol/SetupUtility.h>
#include <Protocol/SmmInt15Service.h>
#include <PostCode.h>
#include <SetupConfig.h>

STATIC
VOID
OemInt15CallBack (
  IN  OUT EFI_IA32_REGISTER_SET   *CpuRegs, 
  IN      VOID                    *Context
  );

#endif

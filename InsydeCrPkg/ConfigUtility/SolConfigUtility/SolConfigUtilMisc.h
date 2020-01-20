/** @file
  This file for CR SOL reference

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SOL_CONFIG_UTIL_MISC_H_
#define _SOL_CONFIG_UTIL_MISC_H_

#include <SolConfigUtilSetupConfig.h>

#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>

#include <Protocol/H2ODialog.h>


EFI_STATUS
SolConfigCallback (
  EFI_QUESTION_ID          QuestionId,
  SOL_CONFIGURATION         *SolConfig
  );
  
EFI_STATUS
SolConfigFormInit (
  EFI_HII_HANDLE           HiiHandle,
  SOL_CONFIGURATION         *SolConfig
  );

VOID
SolConfigSetToDefault (
  SOL_CONFIGURATION         *SolConfig
  );

VOID
SolUpdateEntrys (
  SOL_CONFIGURATION         *SolConfig
  );

#endif // _SOL_CONFIG_UTIL_MISC_H_
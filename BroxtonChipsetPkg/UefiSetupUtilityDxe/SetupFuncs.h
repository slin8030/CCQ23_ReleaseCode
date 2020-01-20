/** @file

 Setup function for SetupUtilityDxe Driver

;******************************************************************************
;* Copyright (c) 2012-2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#ifndef _SETUP_FUNCS_H_
#define _SETUP_FUNCS_H_

#include "SetupUtility.h"
#include <SetupConfig.h>
/*
#include "McAccess.h"
#include "OemPnpStrings.h"
*/

EFI_STATUS
LoadCustomOption (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL        *This
  );

EFI_STATUS
SaveCustomOption (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL        *This
  );

EFI_STATUS
DiscardChange (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL        *This
  );

VOID *
GetVariableAndSize (
  IN  CHAR16              *Name,
  IN  EFI_GUID            *VendorGuid,
  OUT UINTN               *VariableSize
  );

EFI_STATUS
CheckLanguage (
  VOID
  );

EFI_STATUS
UpdateAtaString(
  IN      EFI_ATAPI_IDENTIFY_DATA     *IdentifyDriveInfo,
  IN OUT  CHAR16                      **NewString
  );

EFI_STATUS
AsciiToUnicode (
  IN    CHAR8     *AsciiString,
  IN    CHAR16    *UnicodeString
  );

EFI_STATUS
EventTimerControl (
  IN UINT64                     Timeout
  );

CHAR16 *
GetTokenStringByLanguage (
  IN EFI_HII_HANDLE           HiiHandle,
  IN STRING_REF               Token,
  IN CHAR8                    *LanguageString
  );

EFI_STATUS
SaveSetupConfig (
  IN     CHAR16               *VariableName,
  IN     EFI_GUID             *VendorGuid,
  IN     UINT32               Attributes,
  IN     UINTN                DataSize,
  IN     VOID                 *Buffer
  );

EFI_STATUS
AddNewString (
  IN   EFI_HII_HANDLE           InputHiiHandle,
  IN   EFI_HII_HANDLE           OutputHiiHandle,
  IN   STRING_REF               InputToken,
  OUT  STRING_REF               *OutputToken
  );
EFI_STATUS
DetermineCpuSupportVMx (
  IN OUT CHIPSET_CONFIGURATION  *SetupNvData
);
EFI_STATUS
DefaultSetup (
  OUT SYSTEM_CONFIGURATION    *SetupNvData
  );

#endif

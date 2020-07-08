/** @file
  Definition for Dxe Project Services Default Lib.

;******************************************************************************
;* Copyright (c) 2016, Compal Electronics, Inc.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _DXE_PROJECT_SVC_LIB_H_
#define _DXE_PROJECT_SVC_LIB_H_

#include <Uefi.h>
#include "T66CommonDefinition.h"

/**
  Get Bios setting data by Signature(GUID :gEfiBIOSSettingGuid; Variable Name:L"BiosSetting").

  @param[In Out] BiosSetting      Return BiosSetting data point
  @param[In]     Signature        Search by Signature

  @return EFI_INVALID_PARAMETER   BiosSetting is NULL
  @return EFI_OUT_OF_RESOURCES    Allocate memory fail.
  @return EFI_NOT_FOUND           Not found signature data.
  @return EFI_SUCCESS             Function execute successful.
**/
EFI_STATUS
EFIAPI
GetBiosSettingData (
  IN OUT BIOS_SETTING_STRUCT      **BiosSetting,
  IN UINT32                         Signature
  );

#endif

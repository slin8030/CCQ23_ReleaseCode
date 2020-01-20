/** @file
;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/DxeInsydeChipsetLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Guid/SetupDefaultHob.h>

/**
 Retrieve setup (default) variable data HOB. This function is created for DXE driver
 without normal variable service.

 @param[out] SetupVariable      Pointer to the structure of CHIPSET_CONFIGURATION,
                                this pointer must be allocated with sizeof(CHIPSET_CONFIGURATION)
                                before being called
 @param[in]  SetupVariableSize  Setup variable data Size

 @retval EFI_SUCCESS            The Chipset configuration is successfully retrieved
 @retval EFI_NOT_FOUND          Setup (default) variable is not found
 @retval EFI_INVALID_PARAMETER  Parameter error
**/
EFI_STATUS
EFIAPI
GetChipsetSetupVariableDxe (
  IN OUT CHIPSET_CONFIGURATION          *SetupVariable,
  IN     UINTN                          SetupVariableSize
  )
{
  EFI_PEI_HOB_POINTERS                  GuidHob;
  UINTN                                 VariableSize;
  VOID                                  *SetupVariableHob;

  VariableSize = PcdGet32 (PcdSetupConfigSize);
  if ((SetupVariable == NULL) || (SetupVariableSize == 0) || (VariableSize != SetupVariableSize)) {
    return EFI_INVALID_PARAMETER;
  }

  GuidHob.Raw = GetHobList ();
  GuidHob.Raw = GetNextGuidHob (&gSetupDefaultHobGuid, GuidHob.Raw);
  if (GuidHob.Raw != NULL) {
    SetupVariableHob = GET_GUID_HOB_DATA (GuidHob.Guid);
    CopyMem (SetupVariable, SetupVariableHob, VariableSize);
    return EFI_SUCCESS;
  }

  DEBUG ((EFI_D_INFO | EFI_D_ERROR, "!!! Get gSetupDefaultHobGuid HOB fail.\n"));

  GuidHob.Raw = GetHobList ();
  GuidHob.Raw = GetNextGuidHob (&gSystemConfigurationGuid, GuidHob.Raw);
  if (GuidHob.Raw != NULL) {
    SetupVariableHob = GET_GUID_HOB_DATA (GuidHob.Guid);
    CopyMem (SetupVariable, SetupVariableHob, VariableSize);
    return EFI_SUCCESS;
  }

  DEBUG ((EFI_D_INFO | EFI_D_ERROR, "!!! Get gSystemConfigurationGuid HOB fail.\n"));
  ASSERT_EFI_ERROR (EFI_NOT_FOUND);
  return EFI_NOT_FOUND;
}
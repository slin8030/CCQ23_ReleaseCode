/** @file
  This module provides default Setup variable data if Setupvariable is not found
  and also provides a Setup variable cache mechanism in PEI phase

;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <PiPei.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/KernelConfigLib.h>
#include <Library/SetupUtilityLib.h>
#include <Library/PeiOemSvcKernelLib.h>
#include <Library/BaseSetupDefaultLib.h>

#include <Ppi/ReadOnlyVariable2.h>
#include <Guid/PeiDefaultSetupProviderHob.h>
#include <Guid/SetupDefaultHob.h>
#include <Ppi/EmuPei.h>
#include <FastRecoveryData.h>
#include "SetupDataProvider.h"
#include <Library/SeamlessRecoveryLib.h>

CHAR16 mSetupVariableName[]    = SETUP_VARIABLE_NAME;
CHAR16 mSetupOrgVariableName[] = L"SetupOrg";


EFI_PEI_READ_ONLY_VARIABLE2_PPI mSetupDataProviderVariablePpi = {
  NewPeiGetVariable,
  NewPeiNextVariableName
};

EFI_PEI_PPI_DESCRIPTOR     mSetupDataProviderVariablePpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiReadOnlyVariable2PpiGuid,
  &mSetupDataProviderVariablePpi
};

EFI_STATUS
ReadOnlyVariable2HookCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  );
EFI_PEI_NOTIFY_DESCRIPTOR      mVariableNotifyPpiList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiReadOnlyVariable2PpiGuid,
    ReadOnlyVariable2HookCallback
  }
};
/**
  Provide the get next variable functionality of the variable services.

  @param  This              A pointer to this instance of the EFI_PEI_READ_ONLY_VARIABLE2_PPI.

  @param  VariableNameSize  On entry, points to the size of the buffer pointed to by VariableName.
  @param  VariableName      On entry, a pointer to a null-terminated string that is the variable's name.
                            On return, points to the next variable's null-terminated name string.

  @param  VariableGuid      On entry, a pointer to an EFI_GUID that is the variable's GUID.
                            On return, a pointer to the next variable's GUID.

  @retval EFI_SUCCESS       The interface could be successfully installed
  @retval EFI_NOT_FOUND     The variable could not be discovered

**/
EFI_STATUS
EFIAPI
NewPeiNextVariableName (
  IN CONST  EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
  IN OUT UINTN                              *VariableNameSize,
  IN OUT CHAR16                             *VariableName,
  IN OUT EFI_GUID                           *VendorGuid
  )
{
  EFI_STATUS                                Status;
  EFI_PEI_HOB_POINTERS                      GuidHob;
  SETUP_DATA_PROVIDER_CONTEXT               *SetupDataProviderContext;

  GuidHob.Raw = GetHobList ();
  GuidHob.Raw = GetNextGuidHob (&gPeiDefaultSetupProviderGuid, GuidHob.Raw);
  if (GuidHob.Raw == NULL) {
    return EFI_NOT_FOUND;
  }

  SetupDataProviderContext = (SETUP_DATA_PROVIDER_CONTEXT *)GET_GUID_HOB_DATA (GuidHob.Guid);

  Status = SetupDataProviderContext->VariablePpi->NextVariableName (
                                                    This,
                                                    VariableNameSize,
                                                    VariableName,
                                                    VendorGuid
                                                    );

  return Status;
}

/**
  Provide the revised read variable functionality of the variable services.

  @param  This                  A pointer to this instance of the EFI_PEI_READ_ONLY_VARIABLE2_PPI.
  @param  VariableName          A pointer to a null-terminated string that is the variable's name.
  @param  VariableGuid          A pointer to an EFI_GUID that is the variable's GUID. The combination of
                                VariableGuid and VariableName must be unique.
  @param  Attributes            If non-NULL, on return, points to the variable's attributes.
  @param  DataSize              On entry, points to the size in bytes of the Data buffer.
                                On return, points to the size of the data returned in Data.
  @param  Data                  Points to the buffer which will hold the returned variable value.

  @retval EFI_SUCCESS           The interface could be successfully installed
  @retval EFI_NOT_FOUND         The variable could not be discovered
  @retval EFI_BUFFER_TOO_SMALL  The caller buffer is not large enough

**/
EFI_STATUS
EFIAPI
NewPeiGetVariable (
  IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI  *This,
  IN CONST CHAR16                           *VariableName,
  IN CONST EFI_GUID                         *VendorGuid,
  OUT UINT32                                *Attributes OPTIONAL,
  IN OUT UINTN                              *DataSize,
  OUT VOID                                  *Data
  )
{
  EFI_PEI_HOB_POINTERS                      GuidHob;
  SETUP_DATA_PROVIDER_CONTEXT               *SetupDataProviderContext;
  UINTN                                     SetupVariableSize;

  if (VariableName == NULL || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  GuidHob.Raw = GetHobList ();
  GuidHob.Raw = GetNextGuidHob (&gPeiDefaultSetupProviderGuid, GuidHob.Raw);
  if (GuidHob.Raw == NULL) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  SetupDataProviderContext = (SETUP_DATA_PROVIDER_CONTEXT *)GET_GUID_HOB_DATA (GuidHob.Guid);
  if (SetupDataProviderContext == NULL) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  SetupVariableSize = SetupDataProviderContext->SetupDataSize;
  if (CompareGuid ((EFI_GUID *)VendorGuid, &gSystemConfigurationGuid) &&
      (StrCmp (VariableName, mSetupVariableName) == 0)) {

    if (*DataSize < SetupVariableSize) {
      *DataSize = SetupVariableSize;
      return EFI_BUFFER_TOO_SMALL;
    } else if (Data == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    if (Attributes != NULL) {
      *Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
    }
    CopyMem (Data, SetupDataProviderContext->SetupData, SetupVariableSize);
    return EFI_SUCCESS;
  }

  if (CompareGuid ((EFI_GUID *)VendorGuid, &gSystemConfigurationGuid) &&
      (StrCmp (VariableName, mSetupOrgVariableName) == 0)) {
    return SetupDataProviderContext->VariablePpi->GetVariable (
                                                    This,
                                                    mSetupVariableName,
                                                    VendorGuid,
                                                    Attributes,
                                                    DataSize,
                                                    Data
                                                    );

  }

  return SetupDataProviderContext->VariablePpi->GetVariable (
                                                  This,
                                                  VariableName,
                                                  VendorGuid,
                                                  Attributes,
                                                  DataSize,
                                                  Data
                                                  );
}

/**
  Setup setup default data and hook variable PPI in PEI phase.

  @param FileHandle             Handle of the file being invoked.
  @param PeiServices            Describes the list of possible PEI Services.

 @retval EFI_STATUS

**/
EFI_STATUS
ReadOnlyVariable2HookCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  EFI_STATUS                                Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI           *VariablePpi;
  EFI_PEI_PPI_DESCRIPTOR                    *VariablePpiDescriptor;
  SETUP_DATA_PROVIDER_CONTEXT               *SetupDataProviderContext;
  UINTN                                     VariableSize;
  BOOLEAN                                   IsRecovery;
  UINT8                                     *EmuPeiPpi;

  EFI_PEI_HOB_POINTERS                      GuidHob;
  VOID                                      *SetupDefaultHobData;

  IsRecovery = FALSE;

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             &VariablePpiDescriptor,
             (VOID **)&VariablePpi
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Detect if the system is in recovery mode
  //
  Status = OemSvcDetectRecoveryRequest (&IsRecovery);

  if (FeaturePcdGet (PcdUseFastCrisisRecovery)) {
    Status = PeiServicesLocatePpi (
               &gEmuPeiPpiGuid,
               0,
               NULL,
               (VOID **)&EmuPeiPpi
               );
    if (!EFI_ERROR (Status)) {
      IsRecovery = TRUE;
    }
  }
  if ((!IsRecovery) && IsFirmwareFailureRecovery ()) {
    IsRecovery = TRUE;
  }

  if (IsRecovery) {
    Status = PeiServicesSetBootMode (BOOT_IN_RECOVERY_MODE);
  }

  VariableSize = 0;
  Status = VariablePpi->GetVariable (
                          VariablePpi,
                          mSetupVariableName,
                          &gSystemConfigurationGuid,
                          NULL,
                          &VariableSize,
                          NULL
                          );

  if (Status == EFI_BUFFER_TOO_SMALL && !IsRecovery ) {
      return EFI_SUCCESS;
  } else {
    VariableSize = GetSetupVariableSize ();
  }
  GuidHob.Raw = GetHobList ();
  GuidHob.Raw = GetNextGuidHob (&gPeiDefaultSetupProviderGuid, GuidHob.Raw);
  if (GuidHob.Raw != NULL) {
    return EFI_SUCCESS;
  }

  SetupDataProviderContext = BuildGuidHob (
                               &gPeiDefaultSetupProviderGuid,
                               sizeof (SETUP_DATA_PROVIDER_CONTEXT) + VariableSize - 1
                               );
  if (SetupDataProviderContext == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  SetupDataProviderContext->VariablePpi = VariablePpi;
  SetupDataProviderContext->SetupDataSize = VariableSize;

  if (Status == EFI_BUFFER_TOO_SMALL && !IsRecovery) {
    Status = VariablePpi->GetVariable (
                            VariablePpi,
                            mSetupVariableName,
                            &gSystemConfigurationGuid,
                            NULL,
                            &VariableSize,
                            SetupDataProviderContext->SetupData
                            );
  } else {
    SetMem (SetupDataProviderContext->SetupData, VariableSize, 0);
    ExtractSetupDefault (SetupDataProviderContext->SetupData);
    //
    // Create SetupDefaultHob only if Setup variable does not exist or system is in recovery mode
    //
    SetupDefaultHobData = BuildGuidDataHob (
      &gSetupDefaultHobGuid,
      SetupDataProviderContext->SetupData,
      VariableSize
      );

    //
    // Setup variable is not available, set SetupVariableInvalid flag
    //
    PcdSetBool (PcdSetupDataInvalid, TRUE);

    //
    //  SetupDataProviderContext->SetupData should be synchronized with SetupDefaultHobData
    //  to ensure SetupDataInvalid is updated
    //
    CopyMem(SetupDataProviderContext->SetupData, SetupDefaultHobData, VariableSize);
  }

  Status = PeiServicesReInstallPpi (
             VariablePpiDescriptor,
             &mSetupDataProviderVariablePpiList
             );

  return Status;
}

/**
  Setup setup default data and hook variable PPI in PEI phase.

  @param FileHandle             Handle of the file being invoked.
  @param PeiServices            Describes the list of possible PEI Services.

 @retval EFI_STATUS

**/
EFI_STATUS
EFIAPI
SetupDataProviderEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  PeiServicesNotifyPpi (mVariableNotifyPpiList);
  return EFI_SUCCESS;
}

/** @file
  This file is SampleCode for a DXE driver to allow for caching of HSTI
  test results to eliminate the need for testing on every boot.

@copyright
  Copyright (c) 2015 - 2016 Intel Corporation. All rights reserved This
  software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in
  any form or by any means without the express written consent
  of Intel Corporation.
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
**/

#include "HstiResultDxe.h"

DXE_SI_POLICY_PROTOCOL      *mSiPolicyData;
CONFIGURATION_CHANGE        mConfigChanged;

/**
  Handler to Save HSTI results for publishing on subsequent boots

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context

**/
VOID
EFIAPI
SaveConfigurationChange (
  EFI_EVENT                               Event,
  VOID                                    *Context
  )
{
  UINT32                        VarAttributes;
  UINTN                         VarSize;
  CONFIGURATION_CHANGE          ConfigChanged;
  VOID                          *Interface;
  EFI_STATUS                    Status;

  //
  // Check whether we are running for first time or actually when signaled
  //
  Status = gBS->LocateProtocol (&gPlatformConfigChangeProtocolGuid, NULL, &Interface);
  if (EFI_ERROR (Status)) {
    return;
  }

  ConfigChanged.ConfigChangeType = SETUP_OPTION_CHANGE;
  VarSize                        = sizeof(CONFIGURATION_CHANGE);
  VarAttributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;

  gRT->SetVariable (
            PLATFORM_CONFIGURATION_CHANGE,
            &gPlatformConfigChangeGuid,
            VarAttributes,
            VarSize,
            &ConfigChanged
            );
  if (Event != NULL) {
    gBS->CloseEvent (Event);
  }

}

/**
  Handler to Save HSTI results for publishing on subsequent boots

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context
**/
VOID
EFIAPI
SaveResults (
  EFI_EVENT                               Event,
  VOID                                    *Context
  )
{

  UINT32                        VarAttributes;
  UINTN                         VarSize;
  EFI_STATUS                    Status;
  VOID                          *Interface;

  Status = EFI_SUCCESS;

  //
  // Check whether we are running for first time or actually when signaled
  //
  Status = gBS->LocateProtocol (&gHstiPublishCompleteProtocolGuid, NULL, &Interface);
  if (EFI_ERROR (Status)) {
    return;
  }

  if (mSiPolicyData->Hsti == NULL) {
    return;
  }
  if(( mConfigChanged.ConfigChangeType != NO_CONFIG_CHANGE)){

    VarAttributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;
    VarSize       = mSiPolicyData->HstiSize;

    gRT->SetVariable (
                  HSTI_STORAGE_NAME,
                  &gHstiStoredResultsGuid,
                  VarAttributes,
                  VarSize,
                  mSiPolicyData->Hsti
                  );
    //
    // Need to clear out the previous change so we can use cached results again
    //
    mConfigChanged.ConfigChangeType = NO_CONFIG_CHANGE;
    VarSize       = sizeof(CONFIGURATION_CHANGE);
    VarAttributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;

    Status = gRT->SetVariable (
                PLATFORM_CONFIGURATION_CHANGE,
                &gPlatformConfigChangeGuid,
                VarAttributes,
                VarSize,
                &mConfigChanged
                );
  }
  if (Event != NULL) {
    gBS->CloseEvent (Event);
  }
}

/**
  The driver's entry point.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurs when executing this entry point.
**/
EFI_STATUS
DriverEntryPoint (
  IN    EFI_HANDLE                  ImageHandle,
  IN    EFI_SYSTEM_TABLE            *SystemTable
  )
{
  VOID                            *Registration;
  EFI_STATUS                      Status;
  UINT32                          VarAttributes;
  ADAPTER_INFO_PLATFORM_SECURITY  *Hsti;
  UINTN                           VarSize;
  EDKII_VARIABLE_LOCK_PROTOCOL    *VariableLockProtocol;
  //HOB_SAVE_MEMORY_DATA            *MemInfoHob;

  Hsti = NULL;
  ZeroMem(&mConfigChanged,sizeof(CONFIGURATION_CHANGE));

  //
  // Locate DxeSiPolicyProtocolGuid protocol instance and assign it to a global variable
  //
  Status = gBS->LocateProtocol (&gDxeSiPolicyProtocolGuid, NULL, (VOID **) &mSiPolicyData);
  if (EFI_ERROR (Status)) {
    DEBUG((DEBUG_ERROR,"Failed to locate DxeSiPolicyProtocolGuid Protocol\n"));
  }

  //
  // Register protocol notification function for saving results
  //
  EfiCreateProtocolNotifyEvent (
           &gHstiPublishCompleteProtocolGuid,
           TPL_NOTIFY,
           SaveResults,
           NULL,
           &Registration
           );

  //
  // Register protocol notification function for saving configuration change
  //
  EfiCreateProtocolNotifyEvent (
           &gPlatformConfigChangeProtocolGuid,
           TPL_NOTIFY,
           SaveConfigurationChange,
           NULL,
           &Registration
           );

  VarAttributes = 0;
  VarSize       = sizeof(CONFIGURATION_CHANGE);
  Status = gRT->GetVariable (
                    PLATFORM_CONFIGURATION_CHANGE,
                    &gPlatformConfigChangeGuid,
                    &VarAttributes,
                    &VarSize,
                    &mConfigChanged
                    );
  if(EFI_ERROR(Status)){
    mConfigChanged.ConfigChangeType = FIRST_BOOT;
  }

  VarAttributes = 0;
  VarSize       = 0;

  Status = gRT->GetVariable (
                  HSTI_STORAGE_NAME,
                  &gHstiStoredResultsGuid,
                  &VarAttributes,
                  &VarSize,
                  Hsti
                  );
  if(EFI_ERROR(Status)){
    Hsti   = (ADAPTER_INFO_PLATFORM_SECURITY *)   AllocatePool(VarSize + 1);
    if (((UINTN) Hsti & BIT0) == 0){// Why is this check? - isn't this a random pool?
      Hsti = (ADAPTER_INFO_PLATFORM_SECURITY*)((UINT8 *) Hsti + 1);
    }

    Status = gRT->GetVariable (
                    HSTI_STORAGE_NAME,
                    &gHstiStoredResultsGuid,
                    &VarAttributes,
                    &VarSize,
                    Hsti
                    );
  }

  //MemInfoHob = (HOB_SAVE_MEMORY_DATA*)GetFirstGuidHob (&gMemRestoreDataGuid);
  //if (MemInfoHob != NULL) {
  //  DEBUG ((EFI_D_INFO, "Hsti MrcBootMode = %x\n", MemInfoHob->MrcData.Inputs.BootMode));
  //}

  //
  // Need to not use cached results if any setup option changed on previous boot or any major
  // HW change happened that is significant enough to trigger MRC retraining
  //
  if( !EFI_ERROR(Status) &&
      ( mConfigChanged.ConfigChangeType == NO_CONFIG_CHANGE )){// &&
//      ( MemInfoHob->MrcData.Inputs.BootMode != bmCold)){
    mSiPolicyData->Hsti = Hsti;
    mSiPolicyData->HstiSize = VarSize;

    //
    // Need to lock this variable to prevent changing cached results by some malicious software
    //
    Status = gBS->LocateProtocol (&gEdkiiVariableLockProtocolGuid, NULL, (VOID **)&VariableLockProtocol);
    if (!EFI_ERROR (Status)) {
      Status = VariableLockProtocol->RequestToLock (
                                       VariableLockProtocol,
                                       HSTI_STORAGE_NAME,
                                       &gHstiStoredResultsGuid
                                       );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "HSTI cached results were unable to be locked: Status = %r\n", Status));
        return Status;
      }
    }
  }
  return EFI_SUCCESS;
}

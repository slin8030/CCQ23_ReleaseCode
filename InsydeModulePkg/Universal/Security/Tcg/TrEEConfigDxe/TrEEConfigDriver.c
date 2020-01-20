/** @file
  TrEE configuration module.

;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Protocol/TcgService.h>
#include <Protocol/TpmPhysicalPresence.h>
#include <Protocol/TrEEProtocol.h>
#include <Protocol/TrEEPhysicalPresence.h>
#include <Protocol/SetupUtility.h>
#include <Protocol/EndOfBdsBootSelection.h>

#include <Guid/PhysicalPresenceData.h>
#include <Guid/TrEEPhysicalPresenceData.h>
#include <Guid/TpmInstance.h>

#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/VariableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/HobLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/Tcg2PpVendorLib.h>

#include <IndustryStandard/TcgPhysicalPresence.h>

#include <KernelSetupConfig.h>

EFI_EVENT                         mEndOfBdsSelectionEvent;

/**
  Check if physical presence command request needs platform reset.

  @param[in] PPRequest     The physical presence command request.
  @param[in] PPFlags       The original physical presence flags.
  @param[in] NewPPFlags    The new physical presence flags after the command executed.

  @retval  TRUE            Platform reset is required.
  @retval  FALSE           Platform reset is not required.
**/
STATIC
BOOLEAN
IsResetRequired (
  IN  UINT8         PPRequest,
  IN  UINT32        PPFlags,
  IN  UINT32        NewPPFlags
  )
{

  if (((PPFlags & TCG2_BIOS_INFORMATION_FLAG_HIERACHY_CONTROL_STORAGE_DISABLE) != 0) &&
      ((NewPPFlags & TCG2_BIOS_INFORMATION_FLAG_HIERACHY_CONTROL_STORAGE_DISABLE) == 0)) {
    return TRUE;
  }

  if (((PPFlags & TCG2_BIOS_INFORMATION_FLAG_HIERACHY_CONTROL_ENDORSEMENT_DISABLE) != 0) &&
      ((NewPPFlags & TCG2_BIOS_INFORMATION_FLAG_HIERACHY_CONTROL_ENDORSEMENT_DISABLE) == 0)) {
    return TRUE;
  }
  
  switch (PPRequest) {
  
  case TCG2_PHYSICAL_PRESENCE_CLEAR:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR_2:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR_3:
  case TCG2_PHYSICAL_PRESENCE_SET_PCR_BANKS:
  case TCG2_PHYSICAL_PRESENCE_CHANGE_EPS:
  case TCG2_PHYSICAL_PRESENCE_LOG_ALL_DIGESTS:
    return TRUE;
  
  default:
    return FALSE;
  }
}

EFI_STATUS
EFIAPI
TpmSetupSync (
  IN  KERNEL_CONFIGURATION        *SetupRam
)
/*++

  Routine Description:
    Updates setup variables in NV.

  Arguments:
    SetupRam                   - The setup variables from SetupUtility protocol

  Returns:
    EFI_SUCCESS                - Setup variables is successfully updated
    EFI_INVALID_PARAMETER      - Null input used
    Others                     - Variable services failed
    
--*/
{
  EFI_STATUS                      Status;
  VOID                            *SetupNv;
  UINTN                           SetupDataSize;

  if (SetupRam == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = CommonGetVariableDataAndSize (
             SETUP_VARIABLE_NAME,
             &gSystemConfigurationGuid,
             &SetupDataSize,
             (VOID**) &SetupNv
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ((KERNEL_CONFIGURATION *) SetupNv)->TpmDevice       = SetupRam->TpmDevice;
  ((KERNEL_CONFIGURATION *) SetupNv)->TpmDeviceOk     = SetupRam->TpmDeviceOk;
  ((KERNEL_CONFIGURATION *) SetupNv)->Tpm2DeviceOk    = SetupRam->Tpm2DeviceOk;
  ((KERNEL_CONFIGURATION *) SetupNv)->TpmOperation    = SetupRam->TpmOperation;
  ((KERNEL_CONFIGURATION *) SetupNv)->Tpm2Operation   = SetupRam->Tpm2Operation;
  ((KERNEL_CONFIGURATION *) SetupNv)->TpmClear        = SetupRam->TpmClear;
  ((KERNEL_CONFIGURATION *) SetupNv)->Tpm2Enable      = SetupRam->Tpm2Enable;
  ((KERNEL_CONFIGURATION *) SetupNv)->TpmHide         = SetupRam->TpmHide;
  ((KERNEL_CONFIGURATION *) SetupNv)->GrayoutTpmClear = SetupRam->GrayoutTpmClear;

  Status = CommonSetVariable (
             SETUP_VARIABLE_NAME,
             &gSystemConfigurationGuid,
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
             SetupDataSize,
             SetupNv
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->FreePool (SetupNv);
  
  return EFI_SUCCESS;
}

VOID
EFIAPI
TpmPpNotify (
  IN      EFI_EVENT                   Event,
  IN      VOID                        *Context
  )
/*++

  Routine Description:
    The callback function after gEfiTpmPhysicalPresenceProtocolGuid is installed.
    The function issues TPM requests from SCU and update the setup variables.

  Arguments:
    (Standard EFI_EVENT_NOTIFY)

  Returns:
    None
    
--*/
{
  EFI_STATUS                          Status;
  UINT32                              TpmResponse;
  EFI_TCG_PROTOCOL                    *TcgProtocol;
  EFI_TPM_PHYSICAL_PRESENCE_PROTOCOL  *TpmPhysicalPresenceProtocol;
  EFI_SETUP_UTILITY_PROTOCOL          *EfiSetupUtility;
  KERNEL_CONFIGURATION                *SetupRam;
  TCG_EFI_BOOT_SERVICE_CAPABILITY     ProtocolCapability;
  BOOLEAN                             NeedToRestart;

  //
  // Get TPM Physical Presence Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiTpmPhysicalPresenceProtocolGuid,
                  NULL,
                  (VOID **) &TpmPhysicalPresenceProtocol
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  //
  // Get TCG Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiTcgProtocolGuid, 
                  NULL, 
                  (VOID **) &TcgProtocol
                  );
  if (EFI_ERROR (Status)) {
    return;
  }
  
  //
  // Get TPM Status From TCG Protocol
  //
  ProtocolCapability.Size = sizeof (TCG_EFI_BOOT_SERVICE_CAPABILITY);
  Status = TcgProtocol->StatusCheck (
                          TcgProtocol,
                          &ProtocolCapability,
                          NULL,
                          NULL,
                          NULL
                          );
  if (EFI_ERROR (Status)) {
    return;
  }
  
  //
  // Get Setup Variable
  //
  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityProtocolGuid, 
                  NULL, 
                  (VOID **) &EfiSetupUtility
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  SetupRam = (KERNEL_CONFIGURATION *) EfiSetupUtility->SetupNvData;
  //
  // The TPM is initialized if the required protocols exist.
  //
  SetupRam->TpmHide      = 0;
  SetupRam->TpmDeviceOk  = 1;
  SetupRam->Tpm2DeviceOk = 0xFF;
  if (PcdGetBool (PcdTpmAutoDetection)) {
    SetupRam->TpmDevice = TPM_DEVICE_1_2;
  }

  NeedToRestart = FALSE;
  if (SetupRam->TpmClear == 1) {
    if (!(ProtocolCapability.TPMDeactivatedFlag)) {
      //
      // TPM Force Clear
      //
      Status = TpmPhysicalPresenceProtocol->TpmPhysicalPresence (TcgProtocol, 14, &TpmResponse);
      NeedToRestart = TRUE;
      SetupRam->TpmClear = 0;
    }
  }
  
  switch (SetupRam->TpmOperation) {
   
  case TPM_OPERATION_DISABLE_DEACTIVATE:
    Status = TpmPhysicalPresenceProtocol->TpmPhysicalPresence (
                                            TcgProtocol, 
                                            PHYSICAL_PRESENCE_DEACTIVATE_DISABLE, 
                                            &TpmResponse
                                            );
    NeedToRestart = TRUE;
    SetupRam->TpmOperation = 0;
    break;

  case TPM_OPERATION_ENABLE_ACTIVATE:
    Status = TpmPhysicalPresenceProtocol->TpmPhysicalPresence (
                                            TcgProtocol, 
                                            PHYSICAL_PRESENCE_ENABLE_ACTIVATE, 
                                            &TpmResponse
                                            );
    NeedToRestart = TRUE;
    SetupRam->TpmOperation = 3;
    break;

  case TPM_OPERATION_SET_OWNER_INSTALL_TRUE:
    Status = TpmPhysicalPresenceProtocol->TpmPhysicalPresence (
                                            TcgProtocol, 
                                            PHYSICAL_PRESENCE_SET_OWNER_INSTALL_TRUE, 
                                            &TpmResponse
                                            );
    SetupRam->TpmOperation = 0;
    break;
  }

  //
  // Sync Setup in Ram and Setup in NV
  //
  TpmSetupSync (SetupRam);
  //
  // Reset if needed
  //
  if (NeedToRestart) {
    gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
  }
  //
  // Close EndOfBdsSelection Event
  //
  gBS->CloseEvent (mEndOfBdsSelectionEvent);
  gBS->CloseEvent (Event);
  
  return;
}

VOID
EFIAPI
TrEEPpNotify (
  IN      EFI_EVENT                   Event,
  IN      VOID                        *Context
  )
/*++

  Routine Description:
    The callback function after gEfiTrEEPhysicalPresenceProtocolGuid is installed.
    The function issues TPM requests from SCU and update the setup variables.

  Arguments:
    (Standard EFI_EVENT_NOTIFY)

  Returns:
    None
    
--*/
{
  EFI_STATUS                          Status;
  UINT32                              TpmResponse;
  EFI_TCG2_PROTOCOL                   *Tcg2Protocol;
  EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL *TrEEPhysicalPresenceProtocol;
  EFI_SETUP_UTILITY_PROTOCOL          *EfiSetupUtility;
  KERNEL_CONFIGURATION                *SetupRam;
  BOOLEAN                             NeedToRestart;
  UINTN                               FlagsSize;
  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS    *Flags;
  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS    NewFlags;
  EFI_TCG2_BOOT_SERVICE_CAPABILITY    ProtocolCapability;

  //
  // Get TrEE Physical Presence Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiTrEEPhysicalPresenceProtocolGuid, 
                  NULL, 
                  (VOID **) &TrEEPhysicalPresenceProtocol
                  );
  if (EFI_ERROR (Status)) {
    return;
  }
  
  //
  // Get Tcg2 Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiTcg2ProtocolGuid, 
                  NULL, 
                  (VOID **) &Tcg2Protocol
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  ProtocolCapability.Size = sizeof (ProtocolCapability);
  Status = Tcg2Protocol->GetCapability (
                           Tcg2Protocol,
                           &ProtocolCapability
                           );
  if (EFI_ERROR (Status)) {
    return;
  }

  //
  // Get Setup Variable
  //
  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityProtocolGuid, 
                  NULL, 
                  (VOID **) &EfiSetupUtility
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  SetupRam = (KERNEL_CONFIGURATION *) EfiSetupUtility->SetupNvData;
  //
  // The TPM is initialized if the required protocols exist.
  //
  SetupRam->TpmHide      = 0;
  SetupRam->Tpm2DeviceOk = 1;
  SetupRam->TpmDeviceOk  = 0xFF;
  if (PcdGetBool (PcdTpmAutoDetection)) {
    SetupRam->TpmDevice = TPM_DEVICE_2_0;
  }

  FlagsSize = sizeof(EFI_TCG2_PHYSICAL_PRESENCE_FLAGS);
  Status = CommonGetVariableDataAndSize (
             TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
             &gEfiTcg2PhysicalPresenceGuid,
             &FlagsSize,
             (VOID**)&Flags
             );
  ASSERT_EFI_ERROR (Status);

  CopyMem (&NewFlags, Flags, FlagsSize);

  NeedToRestart = FALSE;
  if (SetupRam->Tpm2Operation == 1) {
    //
    // TPM Clear
    //
    Status = TrEEPhysicalPresenceProtocol->ExecutePhysicalPresenceEx (
                                             TrEEPhysicalPresenceProtocol,
                                             TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR_3,
                                             &TpmResponse,
                                             &NewFlags,
                                             0
                                             );
    NeedToRestart = TRUE;
    SetupRam->Tpm2Operation = 0;
  }

  if (SetupRam->TpmOperation != TCG2_PHYSICAL_PRESENCE_NO_ACTION) {
    Status = TrEEPhysicalPresenceProtocol->ExecutePhysicalPresenceEx (
                                             TrEEPhysicalPresenceProtocol,
                                             SetupRam->TpmOperation,
                                             &TpmResponse,
                                             &NewFlags,
                                             (SetupRam->TpmOperation == TCG2_PHYSICAL_PRESENCE_SET_PCR_BANKS) ? 
                                               SetupRam->ActivePcrBanks : 0
                                             );
    if (!NeedToRestart) {
      NeedToRestart = IsResetRequired (SetupRam->TpmOperation, Flags->PPFlags, NewFlags.PPFlags);
    }
    SetupRam->TpmOperation = TCG2_PHYSICAL_PRESENCE_NO_ACTION;
  } else if ((ProtocolCapability.Size == sizeof (ProtocolCapability)) &&
             (SetupRam->ActivePcrBanks != 0) &&
             (SetupRam->ActivePcrBanks != (UINT8)ProtocolCapability.ActivePcrBanks)) {
    //
    // Activate proper PCR banks
    //
    Status = TrEEPhysicalPresenceProtocol->ExecutePhysicalPresenceEx (
                                             TrEEPhysicalPresenceProtocol,
                                             TCG2_PHYSICAL_PRESENCE_SET_PCR_BANKS,
                                             &TpmResponse,
                                             &NewFlags,
                                             SetupRam->ActivePcrBanks
                                             );
    if (!NeedToRestart) {
      NeedToRestart = IsResetRequired (TCG2_PHYSICAL_PRESENCE_SET_PCR_BANKS, Flags->PPFlags, NewFlags.PPFlags);
    }
  }

  if (CompareMem (Flags, &NewFlags, FlagsSize) != 0) {
    Status = CommonSetVariable (
               TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
               &gEfiTcg2PhysicalPresenceGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
               FlagsSize,
               (VOID *)&NewFlags
               );
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Sync Setup in Ram and Setup in NV
  //
  TpmSetupSync (SetupRam);
  //
  // Reset if needed
  //
  if (NeedToRestart) {
    gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
  }
  //
  // Close EndOfBdsSelection Event
  //
  gBS->CloseEvent (mEndOfBdsSelectionEvent);
  gBS->CloseEvent (Event);
  
  return;
}

VOID
EFIAPI
EndOfBdsSelectionNotify (
  IN      EFI_EVENT                   Event,
  IN      VOID                        *Context
  )
/*++

  Routine Description:
    The callback function after gEfiEndOfBdsBootSelectionProtocolGuid is installed.
    The function suppresses SCU TPM options.

  Arguments:
    (Standard EFI_EVENT_NOTIFY)

  Returns:
    None
    
--*/
{
  EFI_STATUS                          Status;
  EFI_SETUP_UTILITY_PROTOCOL          *EfiSetupUtility;
  KERNEL_CONFIGURATION                *SetupRam;
  VOID                                *Interface;

  Status = gBS->LocateProtocol (
                  &gEndOfBdsBootSelectionProtocolGuid, 
                  NULL, 
                  (VOID **) &Interface
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  //
  // Get Setup Variable
  //
  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityProtocolGuid, 
                  NULL, 
                  (VOID **) &EfiSetupUtility
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  SetupRam = (KERNEL_CONFIGURATION *) EfiSetupUtility->SetupNvData;
  
  if (PcdGetBool (PcdTpmHide)) {
    DEBUG((EFI_D_INFO, "TPM is hidden\n"));
    SetupRam->TpmHide = 1;
  } else {
    //
    // TPM is lost, set all TPM variables to default value
    //
    SetupRam->TpmDeviceOk     = 0xFF;
    SetupRam->Tpm2DeviceOk    = 0xFF;
    SetupRam->TpmOperation    = 0;
    SetupRam->Tpm2Operation   = 0;
    SetupRam->TpmClear        = 0;
    SetupRam->Tpm2Enable      = 1;
    SetupRam->TpmHide         = 0;
    SetupRam->GrayoutTpmClear = 0;
    if (PcdGetBool (PcdTpmAutoDetection)) {
      SetupRam->TpmDevice = TPM_DEVICE_NULL;
    }
  }
  
  //
  // Sync SetupRam and SetupNv
  //
  TpmSetupSync (SetupRam);

  gBS->CloseEvent (Event);    
  
  return;
}

EFI_STATUS
EFIAPI
TrEEConfigDriverEntryPoint (
  IN      EFI_HANDLE                ImageHandle,
  IN      EFI_SYSTEM_TABLE          *SystemTable
  )
/*++

  Routine Description:
    Registers event handling function to perform requests from SCU.

  Arguments:
    ImageHandle   - The firmware allocated handle for the EFI image.
    SystemTable   - A pointer to the EFI System Table.

  Returns:
    EFI_SUCCESS   - Handling functions successfully registered.
    Others        - Handling functions register failed.
    
--*/
{
  EFI_STATUS                        Status;
  VOID                              *Registration;

  if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
    //
    // Prevents any changes of TPM state in recovery
    //
    return EFI_UNSUPPORTED;
  }

  //
  // Register the event handling function to suppress SCU TPM options
  // The event will be closed if TPM is working
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  EndOfBdsSelectionNotify,
                  NULL,
                  &mEndOfBdsSelectionEvent
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->RegisterProtocolNotify (
                  &gEndOfBdsBootSelectionProtocolGuid,
                  mEndOfBdsSelectionEvent,
                  (VOID **) &Registration
                  );
  ASSERT_EFI_ERROR (Status);

  if (CompareGuid (PcdGetPtr (PcdTpmInstanceGuid), &gEfiTpmDeviceInstanceTpm12Guid)) {
    //
    // Register event for TPM1.2
    //
    EfiCreateProtocolNotifyEvent (
      &gEfiTpmPhysicalPresenceProtocolGuid, 
      TPL_CALLBACK, 
      TpmPpNotify, 
      NULL, 
      (VOID **) &Registration
      );
  } else {
    //
    // Register event for TPM2.0
    //
    EfiCreateProtocolNotifyEvent (
      &gEfiTrEEPhysicalPresenceProtocolGuid, 
      TPL_CALLBACK, 
      TrEEPpNotify, 
      NULL, 
      (VOID **) &Registration
      );
  }

  return Status;
}


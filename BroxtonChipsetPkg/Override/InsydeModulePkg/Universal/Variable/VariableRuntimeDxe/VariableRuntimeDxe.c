/** @file
  If platform supports SMM mode, this dirver provides variable services in protected mode and SMM mode
  If platform doesn't support SMM mode, this driver provides variables services in protected mode.

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "Variable.h"
#include "VariableLock.h"
#include "VarCheck.h"
#include "AuthService.h"
#include <Protocol/NonVolatileVariable.h>
//[-start-161116-IB07400815-add]//
//#include <Library/IoLib.h>
//#include <Library/CmosLib.h>
//#include <ChipsetPostCode.h>
//#include <ChipsetCmos.h>
//[-end-161116-IB07400815-add]//
//
// Declare ECP_SMM_VARIABLE_PROTOCOL_GUID for backward compatible
//
#define ECP_SMM_VARIABLE_PROTOCOL_GUID \
  { \
    0x5eb95b0a, 0xd085, 0x4a18, 0x9e, 0x41, 0x8e, 0xbd, 0xa7, 0x9f, 0x30, 0xc8 \
  }
EFI_GUID                            mEcpSmmVariableProtocolGuid = ECP_SMM_VARIABLE_PROTOCOL_GUID;

ESAL_VARIABLE_GLOBAL                *mVariableModuleGlobal;
SMM_VARIABLE_INSTANCE               *mSmmVariableGlobal;
EFI_SMM_SYSTEM_TABLE2               *mSmst;
VOID                                *mStorageArea;
VOID                                *mCertDbList;
EDKII_VARIABLE_LOCK_PROTOCOL        mVariableLock = { VariableLockRequestToLock };
EDKII_VAR_CHECK_PROTOCOL            mVarCheck     = { VarCheckRegisterSetVariableCheckHandler,
                                                      VarCheckVariablePropertySet,
                                                      VarCheckVariablePropertyGet };
EDKII_SMM_VAR_CHECK_PROTOCOL        mSmmVarCheck  = { VarCheckRegisterSetVariableCheckHandler,
                                                      VarCheckVariablePropertySet,
                                                      VarCheckVariablePropertyGet };

//[-start-161117-IB07400815-add]//
//VOID
//RumtimeSmiPostCode (
//  UINT32  InputPostCode
//  )
//{ 
//  UINT8  CmosData;
//
//  CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature);
//  if (CmosData & B_CMOS_ACPI_CALLBACK_NOTIFY) {
//    WriteExtCmos32 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosRtPostCodeByte0, InputPostCode); 
//    IoWrite8 (0xB3, 0x01);
//    IoWrite8 (0xB2, 0x90);
//  } 
//}
//[-end-161117-IB07400815-add]//

EFI_STATUS
EFIAPI
RuntimeDxeGetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      * VendorGuid,
  OUT UINT32       *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT VOID         *Data
  )
{
  return VariableServicesGetVariable (
           VariableName,
           VendorGuid,
           Attributes,
           DataSize,
           Data
           );
}

EFI_STATUS
EFIAPI
RuntimeDxeGetNextVariableName (
  IN OUT UINTN     *VariableNameSize,
  IN OUT CHAR16    *VariableName,
  IN OUT EFI_GUID  *VendorGuid
  )
{
  return VariableServicesGetNextVariableName (
           VariableNameSize,
           VariableName,
           VendorGuid
           );
}

EFI_STATUS
EFIAPI
RuntimeDxeSetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  IN UINT32        Attributes,
  IN UINTN         DataSize,
  IN VOID          *Data
  )
{

//[-start-180502-IB07400963-add]//
#if 0 // Example for debug single variable
  if (StrCmp (VariableName, L"CrConfig") == 0) {
    __debugbreak();
  }
#endif
//[-end-180502-IB07400963-add]//

  return VariableServicesSetVariable (
          VariableName,
          VendorGuid,
          Attributes,
          DataSize,
          Data
          );
}

EFI_STATUS
EFIAPI
RuntimeDxeQueryVariableInfo (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  )
{
  return VariableServicesQueryVariableInfo (
           Attributes,
           MaximumVariableStorageSize,
           RemainingVariableStorageSize,
           MaximumVariableSize
           );
}

EFI_STATUS
EFIAPI
SmmGetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      * VendorGuid,
  OUT UINT32       *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT VOID         *Data
  )
{
  return VariableServicesGetVariable (
           VariableName,
           VendorGuid,
           Attributes,
           DataSize,
           Data
           );
}

EFI_STATUS
EFIAPI
SmmGetNextVariableName (
  IN OUT UINTN     *VariableNameSize,
  IN OUT CHAR16    *VariableName,
  IN OUT EFI_GUID  *VendorGuid
  )
{
  return VariableServicesGetNextVariableName (
           VariableNameSize,
           VariableName,
           VendorGuid
           );
}

EFI_STATUS
EFIAPI
SmmSetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  IN UINT32        Attributes,
  IN UINTN         DataSize,
  IN VOID          *Data
  )
{
  EFI_STATUS       Status;

  mEnableLocking = FALSE;
  Status = VariableServicesSetVariable (
             VariableName,
             VendorGuid,
             Attributes,
             DataSize,
             Data
             );
 mEnableLocking = TRUE;
 return Status;
}

EFI_STATUS
EFIAPI
SmmQueryVariableInfo (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  )
{
  return VariableServicesQueryVariableInfo (
           Attributes,
           MaximumVariableStorageSize,
           RemainingVariableStorageSize,
           MaximumVariableSize
           );
}

/**
  A callback invoked by each VARIABLE_STORAGE_PROTOCOL to indicate to the core variable driver that
  SetVariable() is ready for use on that VARIABLE_STORAGE_PROTOCOL

  @retval     EFI_SUCCESS                    Change to WriteServiceIsReady() status was processed successfully.
  @retval     EFI_NOT_READY                  Write service is not ready.
**/
EFI_STATUS
EFIAPI
VariableStorageWriteServiceReadyCallback (
  VOID
  )
{
  UINTN                         Index;
  VARIABLE_STORAGE_PROTOCOL     *VariableStorageProtocol;

  if (mVariableModuleGlobal->WriteServiceReady) {
    return EFI_SUCCESS;
  }

  for ( Index = 0;
        Index < mVariableModuleGlobal->VariableBase.VariableStoresCount;
        Index++) {
    VariableStorageProtocol = mVariableModuleGlobal->VariableBase.VariableStores[Index];
    if (!VariableStorageProtocol->WriteServiceIsReady (VariableStorageProtocol)) {
      return EFI_NOT_READY;
    }
  }
  mVariableModuleGlobal->WriteServiceReady = TRUE;
  return EFI_SUCCESS;
}

/**
  Get variable storage protocol instances and store them in private memory for
  future usage.

  @retval EFI_SUCCESS           Init variable storages succssfully.
  @retval EFI_OUT_OF_RESOURCES  There are not enough memory for SMM variable services.
  @retval EFI_NOT_FOUND         Cannot find any variable storage instance.
  @retval Other                 Other error occurred in this function.
**/
EFI_STATUS
InitVariableStorages (
  VOID
  )
{
  EFI_HANDLE                    *Handles;
  UINTN                         Index;
  EFI_STATUS                    Status;
  VARIABLE_STORAGE_PROTOCOL     *VariableStorageProtocol;

  Status = gBS->LocateProtocol (
                  &gVariableStorageSelectorProtocolGuid,
                  NULL,
                  (VOID **) &mVariableModuleGlobal->VariableBase.VariableStorageSelectorProtocol
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_READY;
  }
  //
  // Get the VARIABLE_STORAGE_PROTOCOL instances
  //
  mVariableModuleGlobal->VariableBase.VariableStoresCount = 0;
  Handles                                                 = NULL;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gVariableStorageProtocolGuid,
                  NULL,
                  &mVariableModuleGlobal->VariableBase.VariableStoresCount,
                  &Handles
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  mVariableModuleGlobal->VariableBase.VariableStores =
    VariableAllocateZeroBuffer (
      sizeof (VARIABLE_STORAGE_PROTOCOL *) * mVariableModuleGlobal->VariableBase.VariableStoresCount,
      TRUE
      );
  ASSERT (mVariableModuleGlobal->VariableBase.VariableStores != NULL);
  if (mVariableModuleGlobal->VariableBase.VariableStores == NULL) {
    EFI_FREE_POOL (Handles);
    return EFI_OUT_OF_RESOURCES;
  }
  for ( Index = 0;
        Index < mVariableModuleGlobal->VariableBase.VariableStoresCount;
        Index++) {
    Status = gBS->OpenProtocol (
                    Handles[Index],
                    &gVariableStorageProtocolGuid,
                    (VOID **) &mVariableModuleGlobal->VariableBase.VariableStores[Index],
                    gImageHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      EFI_FREE_POOL (Handles);
      return Status;
    }
    VariableStorageProtocol = mVariableModuleGlobal->VariableBase.VariableStores[Index];
    VariableStorageProtocol->RegisterWriteServiceReadyCallback (
                              VariableStorageProtocol,
                              VariableStorageWriteServiceReadyCallback
                              );
  }

  EFI_FREE_POOL (Handles);
  return Status;
}


/**
  Callback function for gEfiFaultTolerantWriteLiteProtocolGuid protocol.
  This function will initialize write variable functionality.

  @retval EFI_SUCCESS       RuntimeDxe variable  successfully initialized.
  @return Ohter             Error occurred while doing runtime Dxe variable initialization.
**/
EFI_STATUS
RuntimeDxeVariableInitialize (
  VOID
  )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            NewHandle;
  CRYPTO_SERVICES_PROTOCOL              *CryptoServiceProtocol;
  VOID                                  *Registration;
  EFI_EVENT                             NewEvent;

  //
  // Great!!! Now, this is a runtime driver.
  //
  Status = VariableCommonInitialize ();
//[-start-170210-IB07400839-remove]//
//  ASSERT_EFI_ERROR (Status);
//[-end-170210-IB07400839-remove]//
  if (Status == EFI_UNSUPPORTED ) {
    return Status;
  }
  InitializeSecureBootModesValue ();

  gST->RuntimeServices->GetVariable         = RuntimeDxeGetVariable;
  gST->RuntimeServices->GetNextVariableName = RuntimeDxeGetNextVariableName;
  gST->RuntimeServices->SetVariable         = RuntimeDxeSetVariable;
  gST->RuntimeServices->QueryVariableInfo   = RuntimeDxeQueryVariableInfo;

  //
  // Now install the Variable Runtime Architectural Protocol on a new handle
  //
  NewHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &NewHandle,
                  &gEfiVariableArchProtocolGuid,
                  NULL,
                  &gEfiVariableWriteArchProtocolGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  Status = gBS->LocateProtocol (&gCryptoServicesProtocolGuid, NULL, (VOID **)&CryptoServiceProtocol);
  if (!EFI_ERROR (Status)) {
    CryptoCallback (NULL, NULL);
  } else {
    NewEvent = VariableCreateProtocolNotifyEvent (
               &gCryptoServicesProtocolGuid,
               TPL_CALLBACK,
               CryptoCallback,
               NULL,
               &Registration
               );
  }

  NewEvent = VariableCreateProtocolNotifyEvent (
             &gBootOrderHookEnableGuid,
             TPL_NOTIFY + 1,
             BootOrderHookEnableCallback,
             NULL,
             &Registration
             );
  NewEvent = VariableCreateProtocolNotifyEvent (
             &gBootOrderHookDisableGuid,
             TPL_NOTIFY + 1,
             BootOrderHookDisableCallback,
             NULL,
             &Registration
             );

  //
  // Register the event to convert the pointer for runtime.
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  VariableAddressChangeEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &NewEvent
                  );
  ASSERT_EFI_ERROR (Status);
  //
  // Note!!!
  // This driver is SMM and RuntimeDxe hybrid driver and UefiRuntimeLib creates ExitBootServices and
  // VariableAddressChange event in constructor, so we cannot use this library to prevent from system
  // calls function located in SMM RAM in protected mode when ExitBootServices event is signaled.
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  ExitBootServicesEvent,
                  NULL,
                  &gEfiEventExitBootServicesGuid,
                  &NewEvent
                  );
  ASSERT_EFI_ERROR (Status);

  mVariableModuleGlobal->SecureBootCallbackEnabled = IsAdministerSecureBootSupport () ? TRUE : FALSE;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &NewHandle,
                  &gEdkiiVariableLockProtocolGuid,
                  &mVariableLock,
                  NULL
                  );
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &NewHandle,
                  &gEdkiiVarCheckProtocolGuid,
                  &mVarCheck,
                  NULL
                  );
  return Status;
}


/**
  Get SMM variable storage protocol instances and store them in private memory for
  future usage.

  @retval EFI_SUCCESS           Init SMM variable storages succssfully.
  @retval EFI_OUT_OF_RESOURCES  There are not enough memory for SMM variable services.
  @retval EFI_NOT_FOUND         Cannot find any SMM variable storage instance.
  @retval Other                 Other error occurred in this function.
**/
EFI_STATUS
InitSmmVariableStorages (
  VOID
  )
{
  EFI_HANDLE                    *Handles;
  UINTN                         Index;
  EFI_STATUS                    Status;
  VARIABLE_STORAGE_PROTOCOL     *VariableStorageProtocol;

  Status = mSmst->SmmLocateProtocol (
                    &gSmmVariableStorageSelectorProtocolGuid,
                    NULL,
                    (VOID **) &mVariableModuleGlobal->VariableBase.VariableStorageSelectorProtocol
                    );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_READY;
  }
  ///
  /// Get the VARIABLE_STORAGE_PROTOCOL instances
  ///
  mVariableModuleGlobal->VariableBase.VariableStoresCount = 0;
  Handles                                                 = NULL;

  ///
  /// Determine the number of handles
  ///
  Status = mSmst->SmmLocateHandle (
                    ByProtocol,
                    &gVariableStorageProtocolGuid,
                    NULL,
                    &mVariableModuleGlobal->VariableBase.VariableStoresCount,
                    Handles
                    );
  if (Status != EFI_BUFFER_TOO_SMALL) {
    DEBUG ((DEBUG_INFO, "No SMM VariableStorageProtocol instances exist\n"));
    return EFI_NOT_READY;
  }
  Handles = VariableAllocateZeroBuffer (mVariableModuleGlobal->VariableBase.VariableStoresCount, TRUE);
  if (Handles == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  mVariableModuleGlobal->VariableBase.VariableStores = VariableAllocateZeroBuffer (mVariableModuleGlobal->VariableBase.VariableStoresCount, TRUE);
  if (mVariableModuleGlobal->VariableBase.VariableStores == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  ///
  /// Get the handles
  ///
  Status = mSmst->SmmLocateHandle (
                    ByProtocol,
                    &gVariableStorageProtocolGuid,
                    NULL,
                    &mVariableModuleGlobal->VariableBase.VariableStoresCount,
                    Handles
                    );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    goto Done;
  }
  mVariableModuleGlobal->VariableBase.VariableStoresCount /= sizeof (EFI_HANDLE);

  for ( Index = 0;
        Index < mVariableModuleGlobal->VariableBase.VariableStoresCount;
        Index++) {
    Status = mSmst->SmmHandleProtocol (
                      Handles[Index],
                      &gVariableStorageProtocolGuid,
                      (VOID **) &mVariableModuleGlobal->VariableBase.VariableStores[Index]
                      );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      goto Done;
    }
    VariableStorageProtocol = mVariableModuleGlobal->VariableBase.VariableStores[Index];
    VariableStorageProtocol->RegisterWriteServiceReadyCallback (
                              VariableStorageProtocol,
                              VariableStorageWriteServiceReadyCallback
                              );
  }

Done:
  if (Handles != NULL) {
    EFI_FREE_POOL (Handles);
  }
  if (Status != EFI_SUCCESS && mVariableModuleGlobal->VariableBase.VariableStores != NULL) {
    EFI_FREE_POOL (mVariableModuleGlobal->VariableBase.VariableStores);
  }
  return Status;
}

/**

  This function will do Smm variable initialization and provide
  gEfiSmmVariableProtocolGuid protocol


  @retval EFI_SUCCESS           Install gEfiSmmVariableProtocolGuid protocol successfully.
  @retval EFI_OUT_OF_RESOURCES  There are not enough memory for SMM variable services.
  @retval EFI_NOT_READY         Cannot find related protocols
  @retval EFI_ALREADY_STARTED   gEfiSmmVariableProtocolGuid has been installed.
  @retval Other                 Other error occurred in this function.
**/
EFI_STATUS
SmmVariableInitialize (
  VOID
  )
{
  EFI_STATUS                             Status;
  EFI_SMM_BASE2_PROTOCOL                 *SmmBase;
  EFI_SMM_VARIABLE_PROTOCOL              *SmmVariable;
  VOID                                   *Registration;
  EFI_SMM_SW_DISPATCH2_PROTOCOL          *SwDispatch;
  EFI_SMM_SW_REGISTER_CONTEXT            SwContext;
  EFI_SMM_RUNTIME_PROTOCOL               *SmmRT;
  EFI_EVENT                              Event;
  EFI_HANDLE                             NewHandle;
  CRYPTO_SERVICES_PROTOCOL              *CryptoServiceProtocol;
  ESAL_VARIABLE_GLOBAL                  *NonVolatileVariable;

  Status = mSmst->SmmLocateProtocol (
                    &gEfiSmmVariableProtocolGuid,
                    NULL,
                    (VOID **)&SmmVariable
                    );
  if (Status == EFI_SUCCESS) {
    return EFI_ALREADY_STARTED;
  }

  Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **)&SmmBase
                  );
  if (Status != EFI_SUCCESS) {
    return Status;
  }
  //
  // Check runtime part of variable driver has been loaded to memory.
  // There are some cases we needn't provide SMM variable services in
  // this driver.
  // (1) system want to use EmuVarialbe driver.
  // (2) Any error occurred while loading runtime part variable driver.
  //
  Status = gBS->LocateProtocol (
                  &gEfiNonVolatileVariableProtocolGuid,
                  NULL,
                  (VOID **)&NonVolatileVariable
                  );
  if (Status != EFI_SUCCESS) {
    return Status;
  }
  mVariableModuleGlobal = VariableAllocateZeroBuffer (sizeof (ESAL_VARIABLE_GLOBAL), TRUE);

  if (mVariableModuleGlobal == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  mVariableDataBuffer = VariableAllocateZeroBuffer (MAX_VARIABLE_SIZE, TRUE);
  if (mVariableDataBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = InitSmmVariableStorages ();
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  mSmmVariableGlobal = VariableAllocateZeroBuffer (sizeof (SMM_VARIABLE_INSTANCE), TRUE);

  if (mSmmVariableGlobal == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Reserved MAX_VARIABLE_SIZE runtime buffer for "Append" operation in virtual mode.
  //
  mStorageArea  = VariableAllocateZeroBuffer (APPEND_BUFF_SIZE, TRUE);
  if (mStorageArea == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Reserved MAX_VARIABLE_SIZE runtime buffer for certificated database list (normal time based authenticated variable)
  // operation in virtual mode.
  //
  mCertDbList  = VariableAllocateZeroBuffer (MAX_VARIABLE_SIZE, TRUE);
  if (mCertDbList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Start of relocated code (to avoid uninitialized variable access
  // in the set and get variable functions in callbacks triggered by
  // the installation of the protocol)
  //
  Status = SmmNvsInitialize (SmmBase);
  SmmVariable = VariableAllocateZeroBuffer (sizeof (EFI_SMM_VARIABLE_PROTOCOL), TRUE);

  if (SmmVariable == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SmmVariable->SmmGetVariable          = SmmGetVariable;
  SmmVariable->SmmSetVariable          = SmmSetVariable;
  SmmVariable->SmmGetNextVariableName  = SmmGetNextVariableName;
  SmmVariable->SmmQueryVariableInfo    = SmmQueryVariableInfo;

  mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->SmmSetVariable = VariableServicesSetVariable;

  Status = mSmst->SmmLocateProtocol (
                    &gCryptoServicesProtocolGuid,
                    NULL,
                    (VOID **)&CryptoServiceProtocol
                    );
  if (!EFI_ERROR (Status)) {
    CryptoCallback (NULL, NULL);
  } else {
    Status = mSmst->SmmRegisterProtocolNotify (
                      &gCryptoServicesProtocolGuid,
                      CryptoServicesInstalled,
                      &Registration
                      );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = mSmst->SmmLocateProtocol (
                    &gEfiSmmSwDispatch2ProtocolGuid,
                    NULL,
                    (VOID **)&SwDispatch
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Locate Smm Cpu protocol for Cpu save state manipulation
  //
  Status = mSmst->SmmLocateProtocol (
                    &gEfiSmmCpuProtocolGuid,
                    NULL,
                    (VOID **)&mSmmVariableGlobal->SmmCpu
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SwContext.SwSmiInputValue = SECURE_BOOT_SW_SMI;

  if (IsAdministerSecureBootSupport ()) {
    Status = SwDispatch->Register (
                           SwDispatch,
                           SecureBootCallback,
                           &SwContext,
                           &mSmmVariableGlobal->SwHandle
                           );

  } else {
    Status = SwDispatch->Register (
                           SwDispatch,
                           NonSecureBootCallback,
                           &SwContext,
                           &mSmmVariableGlobal->SwHandle
                           );
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }



  //
  // TODO: If our codebase doesn't have any EDKI base driver, we can remove below gEfiSmmRuntimeProtocolGuid
  //       protocol related code
  // Since we still have some EDK SMM drivers, we still need use gEfiSmmRuntimeProtocolGuid for backward
  // compatible
  //
  Status = gBS->LocateProtocol (&gEfiSmmRuntimeProtocolGuid, NULL, (VOID **)&SmmRT);
  if (!EFI_ERROR(Status)) {
    Status = SmmRT->LocateProtocol (
                      &gEfiSmmRuntimeProtocolGuid,
                      NULL,
                      (VOID **)&mSmmVariableGlobal->SmmRT
                      );
    ASSERT_EFI_ERROR (Status);
    SmmRuntimeCallback (NULL, SmmVariable);
  } else {
    Event = VariableCreateProtocolNotifyEvent (
              &gEfiSmmRuntimeProtocolGuid,
              TPL_CALLBACK,
              SmmRuntimeCallback,
              SmmVariable,
              &Registration
              );
  }
  InitializeSecureBootModesValue ();
  //
  // Since some drivers or libraries may need check gEfiSmmVariableProtocolGuid protocol to determine
  // if they can send SMI to variable driver, Installs gEfiSmmVariableProtocolGuid after all of SMM
  // callback functions are ready.
  //
  NewHandle = NULL;
  Status = mSmst->SmmInstallProtocolInterface (
                    &NewHandle,
                    &gEfiSmmVariableProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    SmmVariable
                    );
  //
  // For compatible with native EDKII variable driver, install gEfiSmmVariableProtocolGuid in gBS.
  //
  NewHandle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &NewHandle,
                  &gEfiSmmVariableProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  SmmVariable
                  );
  return Status;
}


/**
  Variable Driver main entry point. The Variable driver places the 4 EFI
  runtime services in the EFI System Table and installs arch protocols
  for variable read and write services being available. It also registers
  a notification function for an EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       Variable service successfully initialized.

**/
EFI_STATUS
EFIAPI
VariableServiceInitialize (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_SMM_BASE2_PROTOCOL                *SmmBase;
  BOOLEAN                               InSmm;

  Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **)&SmmBase
                  );
  InSmm = FALSE;
  if (!EFI_ERROR (Status)) {
    SmmBase->InSmm (SmmBase, &InSmm);
  }

  if (!InSmm) {

    if (!IsRuntimeDriver (ImageHandle)) {
       Status = RelocateImageToRuntimeDriver (ImageHandle);
//[-start-170210-IB07400839-remove]//
//       ASSERT_EFI_ERROR (Status);
//[-end-170210-IB07400839-remove]//
        //
        // We only want to load runtime services code to memory and don't load boot services code to memory,
        // so just return EFI_ALREADY_STARTED if it isn't a runtime driver.
        //
        return EFI_ALREADY_STARTED;
    }
    Status = RuntimeDxeVariableInitialize ();
    if (Status != EFI_SUCCESS) {
      return Status;
    }
  } else {
    //
    // Get Smm Syatem Table
    //
    Status = SmmBase->GetSmstLocation(
                        SmmBase,
                        &mSmst
                        );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = SmmVariableInitialize ();
    if (Status != EFI_SUCCESS) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

/**
  Callback for SMM runtime

  @param    Event     Event whose notification function is being invoked.
  @param    Context   Pointer to the notification function's context.

  None

**/
VOID
EFIAPI
SmmRuntimeCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS                      Status;
  EFI_SMM_RUNTIME_PROTOCOL        *SmmRT;
  EFI_HANDLE                      Handle;

  gBS->CloseEvent (Event);
  if (mSmmVariableGlobal->SmmRT == NULL) {
    Status = gBS->LocateProtocol (&gEfiSmmRuntimeProtocolGuid, NULL, (VOID **)&SmmRT);
    if (EFI_ERROR (Status)) {
      return;
    }
    Status = SmmRT->LocateProtocol (
                      &gEfiSmmRuntimeProtocolGuid,
                      NULL,
                      (VOID **)&mSmmVariableGlobal->SmmRT
                      );
    if (EFI_ERROR (Status)) {
      return;
    }
  }
  SmmRT = mSmmVariableGlobal->SmmRT;
  //
  // Since MdePkg defined gEfiSmmVariableProtocolGuid but the GUID and we defined in ECP are different,
  // using SmmRT to use mEcpSmmVariableProtocolGuid insatll SmmVarialbe services for ECP driver
  //
  Handle = NULL;
  Status = SmmRT->InstallProtocolInterface (
                    &Handle,
                    &mEcpSmmVariableProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    Context
                    );
  ASSERT_EFI_ERROR (Status);
  //
  // Use gBS to install mEcpSmmVariableProtocolGuid for ECP driver dependency compatible
  //
  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &mEcpSmmVariableProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  return;
}

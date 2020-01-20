/** @file
  This driver checks whether there is pending TPM request. If yes,
  it will display TPM request information and ask for user confirmation.
  The TPM request will be cleared after it is processed.

;******************************************************************************
;* Copyright (c) 2013 - 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <PiDxe.h>

#include <Protocol/GraphicsOutput.h>
#include <Protocol/TrEEPhysicalPresence.h>
#include <Protocol/VariableLock.h>
//
// Work around for OemFormBrowser2
//
#ifdef MDE_CPU_ARM
typedef UINT16 STRING_REF;
#endif
#include <Protocol/H2ODialog.h>
#include <Protocol/EndOfBdsBootSelection.h>

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/HiiLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/OemGraphicsLib.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <Library/Tcg2PpVendorLib.h>

#include <Guid/EventGroup.h>
#include <Guid/Tcg2PhysicalPresenceData.h>
#include <Guid/UsbEnumeration.h>
//
// TrEEOnTcg2ThunkBegin
//
#include <Guid/TrEEPhysicalPresenceData.h>
#include <Guid/H2OTpm20DtpmPublishAcpiTableDone.h>
#include <Guid/H2OBdsCheckPoint.h>
#include <Library/BdsCpLib.h>
//
// TrEEOnTcg2ThunkEnd
//

#include <IndustryStandard/TcgPhysicalPresence.h>

#include <Base.h>
#include <KernelSetupConfig.h>

#define CONFIRM_BUFFER_SIZE         4096

EFI_HII_HANDLE mTcg2PpStringPackHandle = NULL;
BOOLEAN        mLockInEntry = FALSE;
EFI_GUID       mTcg2UpdatedPlatformAuthGuid = {
  0xebab103e, 0x8906, 0x4dc8, 0xbd, 0xd1, 0xa0, 0xbc, 0x26, 0xb1, 0xba, 0x86
};

/**
  Check and execute the physical presence command requested

  @param[in]  Event        Event whose notification function is being invoked
  @param[in]  Context      Pointer to the notification function's context

**/
VOID
EFIAPI
Tcg2ExecuteTpmRequestCallback (
  IN EFI_EVENT                                Event,
  IN VOID                                     *Context
  );
  
/**
  Lock PPI flags varaible.

  @param[in]  Event        Event whose notification function is being invoked
  @param[in]  Context      Pointer to the notification function's context

**/
VOID
EFIAPI
LockVariableCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  );

/**
  BDS Init check point notification handler.
  Replaces the current platformAuth with a value from the RNG.

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context
**/
VOID
EFIAPI
CpProtocolInitCp (
  IN      EFI_EVENT                       Event,
  IN      VOID                            *Context
  );

/**
  Execute physical presence operation.

  @param[in]      This                Pointer to EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL.
  @param[in]      CommandCode         Physical presence operation value.
  @param[in]      LastReturnedCode    Return code.
  @param[in, out] PpiFlags            The physical presence interface flags.
  
  @retval EFI_SUCCESS                         Operation completed successfully.
  @retval EFI_INVALID_PARAMETER               One or more of the parameters are incorrect.
  @retval Others                              Unexpected error.
  
**/
EFI_STATUS
EFIAPI
ExeucteTcg2PhysicalPresence (
  IN      EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL *This,
  IN      UINT32                              CommandCode,
  OUT     UINT32                              *LastReturnedCode,
  IN OUT  EFI_TREE_PHYSICAL_PRESENCE_FLAGS    *PpiFlags OPTIONAL
  );

/**
  Execute physical presence operation with TCG2 definition.

  @param[in]      This                        EFI TrEE Protocol instance.
  @param[in]      CommandCode                 Physical presence operation value.
  @param[out]     LastReturnedCode            Execute result
  @param[in, out] PpiFlags                    Physical presence interface flags.
  @param[in]      CommandParameter            Physical presence operation parameter.

  @retval EFI_SUCCESS                         Operation completed successfully.
  @retval EFI_INVALID_PARAMETER               One or more of the parameters are incorrect.
  @retval Others                              Unexpected error.
  
**/
EFI_STATUS
EFIAPI
ExeucteTcg2PhysicalPresenceEx (
  IN      EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL *This,
  IN      UINT32                              CommandCode,
  OUT     UINT32                              *LastReturnedCode,
  IN OUT  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS    *PpiFlags OPTIONAL,
  IN      UINT32                              CommandParameter
  );

EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL  mTrEEPhysicalPresenceProtocol = {
  ExeucteTcg2PhysicalPresence,
  ExeucteTcg2PhysicalPresenceEx
  };

/**
  Provides lastest platformAuth value.

  @retval PlatformAuth
**/
STATIC
TPM2B_AUTH *
GetAuthKey (
  VOID
  )
{
  EFI_STATUS           Status;
  TPM2B_AUTH           *Auth;

  Status = gBS->LocateProtocol (&mTcg2UpdatedPlatformAuthGuid, NULL, (VOID **) &Auth);
  if (EFI_ERROR (Status)) {
    //
    // Auth value is default before protocol installation
    //
    return NULL;
  }
  return Auth;
}

/**
  Fill Buffer With BootHashAlg.

  @param[in] Buffer               Buffer to be filled.
  @param[in] BufferSize           Size of buffer.
  @param[in] BootHashAlg          BootHashAlg.

**/
STATIC
VOID
Tcg2FillBufferWithBootHashAlg (
  IN UINT16  *Buffer,
  IN UINTN   BufferSize,
  IN UINT32  BootHashAlg
  )
{
  Buffer[0] = 0;
  if ((BootHashAlg & EFI_TCG2_BOOT_HASH_ALG_SHA1) != 0) {
    if (Buffer[0] != 0) {
      StrnCatS (Buffer, BufferSize / sizeof (CHAR16), L", ", (BufferSize / sizeof (CHAR16)) - StrLen (Buffer) - 1);
    }
    StrnCatS (Buffer, BufferSize / sizeof (CHAR16), L"SHA1", (BufferSize / sizeof (CHAR16)) - StrLen (Buffer) - 1);
  }
  if ((BootHashAlg & EFI_TCG2_BOOT_HASH_ALG_SHA256) != 0) {
    if (Buffer[0] != 0) {
      StrnCatS (Buffer, BufferSize / sizeof (CHAR16), L", ", (BufferSize / sizeof (CHAR16)) - StrLen (Buffer) - 1);
    }
    StrnCatS (Buffer, BufferSize / sizeof (CHAR16), L"SHA256", (BufferSize / sizeof (CHAR16)) - StrLen (Buffer) - 1);
  }
  if ((BootHashAlg & EFI_TCG2_BOOT_HASH_ALG_SHA384) != 0) {
    if (Buffer[0] != 0) {
      StrnCatS (Buffer, BufferSize / sizeof (CHAR16), L", ", (BufferSize / sizeof (CHAR16)) - StrLen (Buffer) - 1);
    }
    StrnCatS (Buffer, BufferSize / sizeof (CHAR16), L"SHA384", (BufferSize / sizeof (CHAR16)) - StrLen (Buffer) - 1);
  }
  if ((BootHashAlg & EFI_TCG2_BOOT_HASH_ALG_SHA512) != 0) {
    if (Buffer[0] != 0) {
      StrnCatS (Buffer, BufferSize / sizeof (CHAR16), L", ", (BufferSize / sizeof (CHAR16)) - StrLen (Buffer) - 1);
    }
    StrnCatS (Buffer, BufferSize / sizeof (CHAR16), L"SHA512", (BufferSize / sizeof (CHAR16)) - StrLen (Buffer) - 1);
  }
  if ((BootHashAlg & EFI_TCG2_BOOT_HASH_ALG_SM3_256) != 0) {
    if (Buffer[0] != 0) {
      StrnCatS (Buffer, BufferSize / sizeof (CHAR16), L", ", (BufferSize / sizeof (CHAR16)) - StrLen (Buffer) - 1);
    }
    StrnCatS (Buffer, BufferSize / sizeof (CHAR16), L"SM3_256", (BufferSize / sizeof (CHAR16)) - StrLen (Buffer) - 1);
  }
}

/**
  Disable the specific hierarchy.

  @param [in]  PlatformAuth    Platform auth value. NULL means no platform auth change.
  @param [in]  PpiFlags        Pointer to EFI_TCG2_PHYSICAL_PRESENCE_FLAGS.

  @retval EFI_SUCCESS          Operation completed successfully.
  @retval other                The operation was unsuccessful.
**/
EFI_STATUS
Tpm2HierarchyDisable (
  IN TPM2B_AUTH                          *PlatformAuth,
  IN EFI_TCG2_PHYSICAL_PRESENCE_FLAGS    *PpiFlags
  )
{
  EFI_STATUS                Status;
  TPMS_AUTH_COMMAND         *AuthSession;
  TPMS_AUTH_COMMAND         LocalAuthSession;

  if (PlatformAuth == NULL) {
    AuthSession = NULL;
  } else {
    ZeroMem (&LocalAuthSession, sizeof(LocalAuthSession));
    LocalAuthSession.sessionHandle = TPM_RS_PW;
    LocalAuthSession.hmac.size = PlatformAuth->size;
    CopyMem (LocalAuthSession.hmac.buffer, PlatformAuth->buffer, PlatformAuth->size);
    AuthSession = &LocalAuthSession;
  }

  Status = EFI_SUCCESS;
  if (PpiFlags->PPFlags & TCG2_BIOS_INFORMATION_FLAG_HIERACHY_CONTROL_STORAGE_DISABLE) {
    Status = Tpm2HierarchyControl (TPM_RH_PLATFORM, AuthSession, TPM_RH_OWNER, NO);
    ASSERT_EFI_ERROR (Status);
    DEBUG ((EFI_D_INFO, "[TPM2] Disable Storage Hierarchy %r\n", Status));
  }
  
  if (PpiFlags->PPFlags & TCG2_BIOS_INFORMATION_FLAG_HIERACHY_CONTROL_ENDORSEMENT_DISABLE) {
    Status = Tpm2HierarchyControl (TPM_RH_PLATFORM, AuthSession, TPM_RH_ENDORSEMENT, NO);
    ASSERT_EFI_ERROR (Status);
    DEBUG ((EFI_D_INFO, "[TPM2] Disable Storage Hierarchy %r\n", Status));
  }

  return Status;
}

/**
  Get string by string id from HII Interface.

  @param[in] Id          String ID.

  @retval    CHAR16 *    String from ID.
  @retval    NULL        If error occurs.

**/
CHAR16 *
Tcg2PhysicalPresenceGetStringById (
  IN  EFI_STRING_ID                           Id
  )
{
  return HiiGetString (mTcg2PpStringPackHandle, Id, NULL);
}

/**
  Send ClearControl and Clear command to TPM.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_TIMEOUT           The register can't run into the expected status in time.
  @retval EFI_BUFFER_TOO_SMALL  Response data buffer is too small.
  @retval EFI_DEVICE_ERROR      Unexpected device behavior.

**/
EFI_STATUS
EFIAPI
Tpm2CommandClear (
  IN TPM2B_AUTH             *PlatformAuth OPTIONAL
  )
{
  EFI_STATUS                Status;
  TPMS_AUTH_COMMAND         *AuthSession;
  TPMS_AUTH_COMMAND         LocalAuthSession;

  if (PlatformAuth == NULL) {
    AuthSession = NULL;
  } else {
    AuthSession = &LocalAuthSession;
    ZeroMem (&LocalAuthSession, sizeof(LocalAuthSession));
    LocalAuthSession.sessionHandle = TPM_RS_PW;
    LocalAuthSession.hmac.size = PlatformAuth->size;
    CopyMem (LocalAuthSession.hmac.buffer, PlatformAuth->buffer, PlatformAuth->size);
  }

  DEBUG ((EFI_D_INFO, "Tpm2ClearControl ... \n"));
  Status = Tpm2ClearControl (TPM_RH_PLATFORM, AuthSession, NO);
  DEBUG ((EFI_D_INFO, "Tpm2ClearControl - %r\n", Status));
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  DEBUG ((EFI_D_INFO, "Tpm2Clear ... \n"));
  Status = Tpm2Clear (TPM_RH_PLATFORM, AuthSession);
  DEBUG ((EFI_D_INFO, "Tpm2Clear - %r\n", Status));

Done:
  ZeroMem (&LocalAuthSession.hmac, sizeof(LocalAuthSession.hmac));
  return Status;
}

/**
  Change EPS.

  @param[in]  PlatformAuth      Platform auth value. NULL means no platform auth change.
  
  @retval EFI_SUCCESS Operation completed successfully.
**/
EFI_STATUS
Tpm2CommandChangeEps (
  IN TPM2B_AUTH                *PlatformAuth OPTIONAL
  )
{
  EFI_STATUS                Status;
  TPMS_AUTH_COMMAND         *AuthSession;
  TPMS_AUTH_COMMAND         LocalAuthSession;

  if (PlatformAuth == NULL) {
    AuthSession = NULL;
  } else {
    AuthSession = &LocalAuthSession;
    ZeroMem (&LocalAuthSession, sizeof(LocalAuthSession));
    LocalAuthSession.sessionHandle = TPM_RS_PW;
    LocalAuthSession.hmac.size = PlatformAuth->size;
    CopyMem (LocalAuthSession.hmac.buffer, PlatformAuth->buffer, PlatformAuth->size);
  }

  Status = Tpm2ChangeEPS (TPM_RH_PLATFORM, AuthSession);
  DEBUG ((EFI_D_INFO, "Tpm2ChangeEPS - %r\n", Status));
  return Status;
}

/**
  Alloc PCR data.

  @param[in]  PlatformAuth      platform auth value. NULL means no platform auth change.
  @param[in]  SupportedPCRBanks Supported PCR banks
  @param[in]  PCRBanks          PCR banks
  
  @retval EFI_SUCCESS Operation completed successfully.
**/
STATIC
EFI_STATUS
Tpm2CommandAllocPcr (
  IN TPM2B_AUTH                *PlatformAuth OPTIONAL,
  IN UINT32                    SupportedPCRBanks,
  IN UINT32                    PCRBanks
  )
{
  EFI_STATUS                Status;
  TPMS_AUTH_COMMAND         *AuthSession;
  TPMS_AUTH_COMMAND         LocalAuthSession;
  TPML_PCR_SELECTION        PcrAllocation;
  TPMI_YES_NO               AllocationSuccess;
  UINT32                    MaxPCR;
  UINT32                    SizeNeeded;
  UINT32                    SizeAvailable;

  if (PlatformAuth == NULL) {
    AuthSession = NULL;
  } else {
    AuthSession = &LocalAuthSession;
    ZeroMem (&LocalAuthSession, sizeof(LocalAuthSession));
    LocalAuthSession.sessionHandle = TPM_RS_PW;
    LocalAuthSession.hmac.size = PlatformAuth->size;
    CopyMem (LocalAuthSession.hmac.buffer, PlatformAuth->buffer, PlatformAuth->size);
  }

  //
  // Fill input
  //
  ZeroMem (&PcrAllocation, sizeof(PcrAllocation));
  if ((EFI_TCG2_BOOT_HASH_ALG_SHA1 & SupportedPCRBanks) != 0) {
    PcrAllocation.pcrSelections[PcrAllocation.count].hash = TPM_ALG_SHA1;
    PcrAllocation.pcrSelections[PcrAllocation.count].sizeofSelect = PCR_SELECT_MAX;
    if ((EFI_TCG2_BOOT_HASH_ALG_SHA1 & PCRBanks) != 0) {
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[0] = 0xFF;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[1] = 0xFF;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[2] = 0xFF;
    } else {
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[0] = 0x00;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[1] = 0x00;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[2] = 0x00;
    }
    PcrAllocation.count++;
  }
  if ((EFI_TCG2_BOOT_HASH_ALG_SHA256 & SupportedPCRBanks) != 0) {
    PcrAllocation.pcrSelections[PcrAllocation.count].hash = TPM_ALG_SHA256;
    PcrAllocation.pcrSelections[PcrAllocation.count].sizeofSelect = PCR_SELECT_MAX;
    if ((EFI_TCG2_BOOT_HASH_ALG_SHA256 & PCRBanks) != 0) {
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[0] = 0xFF;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[1] = 0xFF;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[2] = 0xFF;
    } else {
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[0] = 0x00;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[1] = 0x00;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[2] = 0x00;
    }
    PcrAllocation.count++;
  }
  if ((EFI_TCG2_BOOT_HASH_ALG_SHA384 & SupportedPCRBanks) != 0) {
    PcrAllocation.pcrSelections[PcrAllocation.count].hash = TPM_ALG_SHA384;
    PcrAllocation.pcrSelections[PcrAllocation.count].sizeofSelect = PCR_SELECT_MAX;
    if ((EFI_TCG2_BOOT_HASH_ALG_SHA384 & PCRBanks) != 0) {
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[0] = 0xFF;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[1] = 0xFF;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[2] = 0xFF;
    } else {
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[0] = 0x00;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[1] = 0x00;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[2] = 0x00;
    }
    PcrAllocation.count++;
  }
  if ((EFI_TCG2_BOOT_HASH_ALG_SHA512 & SupportedPCRBanks) != 0) {
    PcrAllocation.pcrSelections[PcrAllocation.count].hash = TPM_ALG_SHA512;
    PcrAllocation.pcrSelections[PcrAllocation.count].sizeofSelect = PCR_SELECT_MAX;
    if ((EFI_TCG2_BOOT_HASH_ALG_SHA512 & PCRBanks) != 0) {
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[0] = 0xFF;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[1] = 0xFF;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[2] = 0xFF;
    } else {
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[0] = 0x00;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[1] = 0x00;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[2] = 0x00;
    }
    PcrAllocation.count++;
  }
  if ((EFI_TCG2_BOOT_HASH_ALG_SM3_256 & SupportedPCRBanks) != 0) {
    PcrAllocation.pcrSelections[PcrAllocation.count].hash = TPM_ALG_SM3_256;
    PcrAllocation.pcrSelections[PcrAllocation.count].sizeofSelect = PCR_SELECT_MAX;
    if ((EFI_TCG2_BOOT_HASH_ALG_SM3_256 & PCRBanks) != 0) {
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[0] = 0xFF;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[1] = 0xFF;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[2] = 0xFF;
    } else {
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[0] = 0x00;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[1] = 0x00;
      PcrAllocation.pcrSelections[PcrAllocation.count].pcrSelect[2] = 0x00;
    }
    PcrAllocation.count++;
  }
  Status = Tpm2PcrAllocate (
             TPM_RH_PLATFORM,
             AuthSession,
             &PcrAllocation,
             &AllocationSuccess,
             &MaxPCR,
             &SizeNeeded,
             &SizeAvailable
             );
  DEBUG ((EFI_D_INFO, "Tpm2PcrAllocate - %r\n", Status));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((EFI_D_INFO, "AllocationSuccess - %02x\n", AllocationSuccess));
  DEBUG ((EFI_D_INFO, "MaxPCR            - %08x\n", MaxPCR));
  DEBUG ((EFI_D_INFO, "SizeNeeded        - %08x\n", SizeNeeded));
  DEBUG ((EFI_D_INFO, "SizeAvailable     - %08x\n", SizeAvailable));

  return EFI_SUCCESS;
}

/**
  Execute physical presence operation requested by the OS.

  @param[in]      This                Pointer to EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL.
  @param[in]      CommandCode         Physical presence operation value.
  @param[in]      LastReturnedCode    Return code.
  @param[in, out] PpiFlags            The physical presence interface flags.
  @param[in]      CommandParameter    Physical presence operation parameter.
  
  @retval EFI_SUCCESS                         Operation completed successfully.
  @retval EFI_INVALID_PARAMETER               One or more of the parameters are incorrect.
  @retval Others                              Unexpected error.
  
**/
EFI_STATUS
EFIAPI
Tcg2ExecutePhysicalPresence (
  IN      EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL *This,
  IN      UINT32                              CommandCode,
  OUT     UINT32                              *LastReturnedCode,
  IN OUT  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS    *PpiFlags,
  IN      UINT32                              CommandParameter
  )
{
  EFI_STATUS                                  Status;
  UINT32                                      TpmResult;
  EFI_TCG2_PROTOCOL                           *Tcg2Protocol;
  EFI_TCG2_BOOT_SERVICE_CAPABILITY            ProtocolCapability;

  if (LastReturnedCode == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gEfiTcg2ProtocolGuid, NULL, (VOID **) &Tcg2Protocol);
  ASSERT_EFI_ERROR (Status);

  ProtocolCapability.Size = sizeof(ProtocolCapability);
  Status = Tcg2Protocol->GetCapability (
                           Tcg2Protocol,
                           &ProtocolCapability
                           );
  ASSERT_EFI_ERROR (Status);

  TpmResult = TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;

  switch (CommandCode) {
   
  case TCG2_PHYSICAL_PRESENCE_ENABLE:
    PpiFlags->PPFlags &= ~TCG2_BIOS_INFORMATION_FLAG_HIERACHY_CONTROL_STORAGE_DISABLE;
    PpiFlags->PPFlags &= ~TCG2_BIOS_INFORMATION_FLAG_HIERACHY_CONTROL_ENDORSEMENT_DISABLE;
    TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    break;
    
  case TCG2_PHYSICAL_PRESENCE_DISABLE:
    if (PcdGetBool (PcdH2OTpm2DisableOperationSupported)) {
      PpiFlags->PPFlags |= TCG2_BIOS_INFORMATION_FLAG_HIERACHY_CONTROL_STORAGE_DISABLE;
      PpiFlags->PPFlags |= TCG2_BIOS_INFORMATION_FLAG_HIERACHY_CONTROL_ENDORSEMENT_DISABLE;
      Status = Tpm2HierarchyDisable (GetAuthKey (), PpiFlags);
      if (EFI_ERROR (Status)) {
        TpmResult = TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;
      } else {
        TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
      }
    } else {
      TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    }
    break;
    
  case TCG2_PHYSICAL_PRESENCE_DISABLE_ENDORSEMENT_ENABLE_STORAGE_HIERARCHY:
    if (PcdGetBool (PcdH2OTpm2DisableOperationSupported)) {
      PpiFlags->PPFlags &= ~TCG2_BIOS_INFORMATION_FLAG_HIERACHY_CONTROL_STORAGE_DISABLE;
      PpiFlags->PPFlags |= TCG2_BIOS_INFORMATION_FLAG_HIERACHY_CONTROL_ENDORSEMENT_DISABLE;
      Status = Tpm2HierarchyDisable (GetAuthKey (), PpiFlags);
      if (EFI_ERROR (Status)) {
        TpmResult = TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;
      } else {
        TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
      }
    } else {
      TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    }
    break;
  
  case TCG2_PHYSICAL_PRESENCE_CLEAR:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR_2:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR_3:
    Status = Tpm2CommandClear (GetAuthKey ());    
    if (EFI_ERROR (Status)) {
      TpmResult = TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;
    } else {
      TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    }
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CLEAR_TRUE:
    PpiFlags->PPFlags |= TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CLEAR;
    TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CLEAR_FALSE:
    PpiFlags->PPFlags &= ~TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CLEAR;
    TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_ON_TRUE:
    PpiFlags->PPFlags |= TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_TURN_ON;
    TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_ON_FALSE:
    PpiFlags->PPFlags &= ~TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_TURN_ON;
    TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_OFF_TRUE:
    PpiFlags->PPFlags |= TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_TURN_OFF;
    TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_OFF_FALSE:
    PpiFlags->PPFlags &= ~TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_TURN_OFF;
    TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_EPS_TRUE:
    PpiFlags->PPFlags |= TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CHANGE_EPS;
    TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_EPS_FALSE:
    PpiFlags->PPFlags &= ~TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CHANGE_EPS;
    TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    break;

  case TCG2_PHYSICAL_PRESENCE_CHANGE_EPS:
    Status = Tpm2CommandChangeEps (GetAuthKey ());
    if (EFI_ERROR (Status)) {
      TpmResult = TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;
    } else {
      TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    }
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_PCRS_TRUE:
    PpiFlags->PPFlags |= TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CHANGE_PCRS;
    TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_PCRS_FALSE:
    PpiFlags->PPFlags &= ~TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CHANGE_PCRS;
    TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    break;
  
  case TCG2_PHYSICAL_PRESENCE_SET_PCR_BANKS:
    Status = Tpm2CommandAllocPcr (GetAuthKey (), ProtocolCapability.HashAlgorithmBitmap, CommandParameter);
    if (EFI_ERROR (Status)) {
      TpmResult = TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;
    } else {
      TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    }
    break;

  case TCG2_PHYSICAL_PRESENCE_LOG_ALL_DIGESTS:
    Status = Tpm2CommandAllocPcr (GetAuthKey (), ProtocolCapability.HashAlgorithmBitmap, ProtocolCapability.HashAlgorithmBitmap);
    if (EFI_ERROR (Status)) {
      TpmResult = TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;
    } else {
      TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    }
    break;
    
  default:
    if (CommandCode <= TCG2_PHYSICAL_PRESENCE_NO_ACTION_MAX) {
      TpmResult = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    } else {
      TpmResult = TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;
    }
    break;
  }

  *LastReturnedCode = TpmResult;
  return EFI_SUCCESS;
}

/**
  Display the confirm text and get user confirmation.

  @param[in] TpmPpCommand             The requested TPM physical presence command.
  @param[in] TpmPpCommandParameter    The requested TPM physical presence command parameter.

  @retval  TRUE            The user has confirmed the changes.
  @retval  FALSE           The user doesn't confirm the changes.
**/
BOOLEAN
Tcg2UserConfirm (
  IN      UINT32                              TpmPpCommand,
  IN      UINT32                              TpmPpCommandParameter
  )
{
  CHAR16                                      *ConfirmText;
  CHAR16                                      *TmpStr1;
  CHAR16                                      *TmpStr2;
  UINTN                                       BufSize;
  BOOLEAN                                     CautionKey;
  EFI_HANDLE                                  Handle;
  UINT8                                       *Instance;
  EFI_STATUS                                  Status;
  H2O_DIALOG_PROTOCOL                         *H2ODialog;
  EFI_INPUT_KEY                               Key;
  CHAR16                                      TempBuffer[1024];
  CHAR16                                      TempBuffer2[1024];
  EFI_TCG2_PROTOCOL                           *Tcg2Protocol;
  EFI_TCG2_BOOT_SERVICE_CAPABILITY            ProtocolCapability;
  UINT32                                      CurrentPCRBanks;

  Status = gBS->LocateProtocol (&gEfiTcg2ProtocolGuid, NULL, (VOID **) &Tcg2Protocol);
  ASSERT_EFI_ERROR (Status);

  ProtocolCapability.Size = sizeof(ProtocolCapability);
  Status = Tcg2Protocol->GetCapability (
                           Tcg2Protocol,
                           &ProtocolCapability
                           );
  ASSERT_EFI_ERROR (Status);

  Status = Tcg2Protocol->GetActivePcrBanks (Tcg2Protocol, &CurrentPCRBanks);
  ASSERT_EFI_ERROR (Status);

  TmpStr2     = NULL;
  CautionKey  = FALSE;
  BufSize     = CONFIRM_BUFFER_SIZE;
  ConfirmText = AllocateZeroPool (BufSize);
  ASSERT (ConfirmText != NULL);
  if (ConfirmText == NULL) {
    return FALSE;
  }

  mTcg2PpStringPackHandle = HiiAddPackages (
                              &gEfiTrEEPhysicalPresenceGuid,
                              NULL,
                              Tcg2PhysicalPresenceDxeStrings,
                              NULL
                              );
  ASSERT (mTcg2PpStringPackHandle != NULL);

  switch (TpmPpCommand) {
  
  case TCG2_PHYSICAL_PRESENCE_ENABLE:
    CautionKey = TRUE;
    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_ENABLE));

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_HEAD_STR));
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CAUTION_KEY));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;
  
  case TCG2_PHYSICAL_PRESENCE_DISABLE:
    CautionKey = TRUE;
    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_DISABLE));

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_HEAD_STR));
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_WARNING));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CAUTION_KEY));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case TCG2_PHYSICAL_PRESENCE_DISABLE_ENDORSEMENT_ENABLE_STORAGE_HIERARCHY:
    CautionKey = TRUE;
    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_DISABLE_ENDORSEMENT_ENABLE_STORAGE));

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_HEAD_STR));
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_WARNING_DISABLE_ENDORSEMENT_ENABLE_STORAGE));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CAUTION_KEY));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CHANGE_CONFIG));
    break;

  case TCG2_PHYSICAL_PRESENCE_CLEAR:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR_2:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR_3:
    CautionKey = TRUE;
    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CLEAR));

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_HEAD_STR));
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_WARNING_CLEAR));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    StrnCat (ConfirmText, L" \n\n", (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CAUTION_KEY));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CLEAR_FALSE:
    CautionKey = TRUE;
    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CLEAR));

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_PPI_HEAD_STR));
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NOTE_CLEAR));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_WARNING_CLEAR));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    StrnCat (ConfirmText, L" \n\n", (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CAUTION_KEY));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NO_PPI_INFO));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_ON_FALSE:
    CautionKey = TRUE;
    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NO_PPI_TURN_ON));

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_PPI_HEAD_STR));
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CAUTION_KEY));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NO_PPI_INFO));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NO_PPI_PROVISION));
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_OFF_FALSE:
    CautionKey = TRUE;
    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NO_PPI_TURN_OFF));

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_PPI_HEAD_STR));
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CAUTION_KEY));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NO_PPI_INFO));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NO_PPI_PROVISION));
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_EPS_FALSE:
    CautionKey = TRUE;
    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NO_PPI_MAINTAIN));

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_PPI_HEAD_STR));
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_WARNING_PP_CHANGE_EPS_FALSE_1));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    
    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_WARNING_PP_CHANGE_EPS_FALSE_2));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CAUTION_KEY));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NO_PPI_INFO));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case TCG2_PHYSICAL_PRESENCE_CHANGE_EPS:
    CautionKey = TRUE;
    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CHANGE_EPS));

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_HEAD_STR));
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_WARNING_CHANGE_EPS_1));
    StrnCatS (ConfirmText, BufSize / sizeof (CHAR16), TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);      
    
    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_WARNING_CHANGE_EPS_2));
    StrnCatS (ConfirmText, BufSize / sizeof (CHAR16), TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CAUTION_KEY));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_PCRS_FALSE:
    CautionKey = TRUE;
    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NO_PPI_CHANGE_PCRS));

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_PPI_HEAD_STR));
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CAUTION_KEY));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NO_PPI_INFO));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_NO_PPI_CHANGE_PCRS));
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PCR_BANKS:
    CautionKey = TRUE;
    TmpStr2 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_SET_PCR_BANKS));

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_HEAD_STR));
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_WARNING_SET_PCR_BANKS_1));
    StrnCatS (ConfirmText, BufSize / sizeof (CHAR16), TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);      

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_WARNING_SET_PCR_BANKS_2));
    StrnCatS (ConfirmText, BufSize / sizeof (CHAR16), TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);      

    Tcg2FillBufferWithBootHashAlg (TempBuffer, sizeof(TempBuffer), TpmPpCommandParameter);
    Tcg2FillBufferWithBootHashAlg (TempBuffer2, sizeof(TempBuffer2), CurrentPCRBanks);

    TmpStr1 = AllocateZeroPool (BufSize);
    ASSERT (TmpStr1 != NULL);
    UnicodeSPrint (TmpStr1, BufSize, L"Current PCRBanks is 0x%x. (%s)\nNew PCRBanks is 0x%x. (%s)\n", CurrentPCRBanks, TempBuffer2, TpmPpCommandParameter, TempBuffer);

    StrnCatS (ConfirmText, BufSize / sizeof (CHAR16), TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    StrnCatS (ConfirmText, BufSize / sizeof (CHAR16), L" \n", (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);

    TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_CAUTION_KEY));
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);	
    FreePool (TmpStr1);      
    break;

  default:
    ;
  }

  if (TmpStr2 == NULL) {
    FreePool (ConfirmText);
    return FALSE;
  }

  TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_REJECT_KEY));
  UnicodeSPrint (ConfirmText + StrLen (ConfirmText), BufSize, TmpStr1, TmpStr2);
  FreePool (TmpStr1);
  FreePool (TmpStr2);

  TmpStr1 = Tcg2PhysicalPresenceGetStringById (STRING_TOKEN (TPM_INQUERY));
  StrnCat (ConfirmText, TmpStr1, StrLen (TmpStr1));
  FreePool (TmpStr1);

  //
  // Trigger USB keyboard connection by protocol notify function if need.
  //
  Status = gBS->LocateProtocol (&gEfiUsbEnumerationGuid, NULL, (VOID **) &Instance);
  if (EFI_ERROR (Status)) {
    Handle = 0;
    gBS->InstallProtocolInterface (
           &Handle,
           &gEfiUsbEnumerationGuid,
           EFI_NATIVE_INTERFACE,
           NULL
           );
  }

  Status = gBS->LocateProtocol (&gH2ODialogProtocolGuid, NULL, (VOID **) &H2ODialog);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }
  DisableQuietBoot ();
  gST->ConOut->ClearScreen (gST->ConOut);
  Status = H2ODialog->ConfirmDialog (0, FALSE, 0, NULL, &Key, ConfirmText);
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 0);
  FreePool (ConfirmText);

  if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
    return TRUE;
  }

  return FALSE;
}

/**
  Check if there is a valid physical presence command request. Also updates parameter value 
  to whether the requested physical presence command already confirmed by user
 
   @param[in]  PpData                      EFI TCG2 Physical Presence request data. 
   @param[in]  Flags                       The physical presence interface flags.
   @param[out] RequestConfirmed            If the physical presence operation command required user confirm from UI.
                                             True, it indicates the command doesn't require user confirm, or already confirmed 
                                                   in last boot cycle by user.
                                             False, it indicates the command need user confirm from UI.

   @retval  TRUE        Physical Presence operation command is valid.
   @retval  FALSE       Physical Presence operation command is invalid.

**/
BOOLEAN
Tcg2HaveValidTpmRequest (
  IN      EFI_TCG2_PHYSICAL_PRESENCE       *PpData,
  IN      EFI_TCG2_PHYSICAL_PRESENCE_FLAGS Flags,
  OUT     BOOLEAN                          *RequestConfirmed
  )
{
  BOOLEAN  IsRequestValid;

  *RequestConfirmed = FALSE;

  switch (PpData->PPRequest) {
  
  case TCG2_PHYSICAL_PRESENCE_NO_ACTION:
    *RequestConfirmed = TRUE;
    return TRUE;

  case TCG2_PHYSICAL_PRESENCE_ENABLE:
    if ((Flags.PPFlags & TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_TURN_ON) == 0) {
      *RequestConfirmed = TRUE;
    }
    break;

  case TCG2_PHYSICAL_PRESENCE_DISABLE:
    if (PcdGetBool (PcdH2OTpm2DisableOperationSupported)) {
      if ((Flags.PPFlags & TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_TURN_OFF) == 0) {
        *RequestConfirmed = TRUE;
      }
    } else {
      *RequestConfirmed = TRUE;
    }
    break;

  case TCG2_PHYSICAL_PRESENCE_DISABLE_ENDORSEMENT_ENABLE_STORAGE_HIERARCHY:
    if (PcdGetBool (PcdH2OTpm2DisableOperationSupported)) {
      if ((Flags.PPFlags & (TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_TURN_ON | TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_TURN_OFF)) == 0) {
        *RequestConfirmed = TRUE;
      }
    } else {
      *RequestConfirmed = TRUE;
    }
    break;
     
  case TCG2_PHYSICAL_PRESENCE_CLEAR:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR_2:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR_3:
    if ((Flags.PPFlags & TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CLEAR) == 0) {
      *RequestConfirmed = TRUE;
    }
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CLEAR_TRUE:
    *RequestConfirmed = TRUE;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CLEAR_FALSE:
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_ON_TRUE:
    *RequestConfirmed = TRUE;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_ON_FALSE:
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_OFF_TRUE:
    *RequestConfirmed = TRUE;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_OFF_FALSE:
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_EPS_TRUE:
    *RequestConfirmed = TRUE;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_EPS_FALSE:
    break;

  case TCG2_PHYSICAL_PRESENCE_CHANGE_EPS:
    if ((Flags.PPFlags & TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CHANGE_EPS) == 0) {
      *RequestConfirmed = TRUE;
    }
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_PCRS_TRUE:
    *RequestConfirmed = TRUE;
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_PCRS_FALSE:
    break;

  case TCG2_PHYSICAL_PRESENCE_SET_PCR_BANKS:
    if ((Flags.PPFlags & TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CHANGE_PCRS) == 0) {
      *RequestConfirmed = TRUE;
    }
    break;

  case TCG2_PHYSICAL_PRESENCE_LOG_ALL_DIGESTS:
    *RequestConfirmed = TRUE;
    break;
    
  default:
    if (PpData->PPRequest >= TCG2_PHYSICAL_PRESENCE_VENDOR_SPECIFIC_OPERATION) {
      IsRequestValid = Tcg2PpVendorLibHasValidRequest (PpData->PPRequest, Flags.PPFlags, RequestConfirmed);
      if (!IsRequestValid) {
        return FALSE;
      } else {
        break;
      }
    } else {
      //
      // Wrong Physical Presence command
      //
      return FALSE;
    }
  }

  if ((Flags.PPFlags & TCG2_LIB_PP_FLAG_RESET_TRACK) != 0) {
    //
    // It had been confirmed in last boot, it doesn't need confirm again.
    //
    *RequestConfirmed = TRUE;
  }

  //
  // Physical Presence command is correct
  //
  return TRUE;
}

/**
  Check and execute the requested physical presence command.

  Caution: This function may receive untrusted input.
  TcgPpData variable is external input, so this function will validate
  its data structure to be valid value.

  @param[in, out] TrEEPpData  Point to the physical presence NV variable.
  @param[in]      Flags       The physical presence interface flags.

**/
VOID
Tcg2ExecutePendingTpmRequest (
  IN OUT  EFI_TCG2_PHYSICAL_PRESENCE          *PpData,
  IN      EFI_TCG2_PHYSICAL_PRESENCE_FLAGS    Flags
  )
{
  EFI_STATUS                                  Status;
  UINTN                                       DataSize;
  BOOLEAN                                     RequestConfirmed;
  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS            NewFlags;
  BOOLEAN                                     ResetRequired;
  UINT32                                      NewPPFlags;

  if (PpData->PPRequest == TCG2_PHYSICAL_PRESENCE_NO_ACTION) {
    //
    // No operation request
    //
    return;
  }

  if (!Tcg2HaveValidTpmRequest (PpData, Flags, &RequestConfirmed)) {
    //
    // Invalid operation request.
    //
    if (PpData->PPRequest <= TCG2_PHYSICAL_PRESENCE_NO_ACTION_MAX) {
      PpData->PPResponse = TCG_PP_OPERATION_RESPONSE_SUCCESS;
    } else {
      PpData->PPResponse = TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;
    }
    PpData->LastPPRequest      = PpData->PPRequest;
    PpData->PPRequest          = TCG2_PHYSICAL_PRESENCE_NO_ACTION;
    PpData->PPRequestParameter = 0;
    DataSize = sizeof (EFI_TCG2_PHYSICAL_PRESENCE);
    Status = gRT->SetVariable (
                    TCG2_PHYSICAL_PRESENCE_VARIABLE,
                    &gEfiTcg2PhysicalPresenceGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    DataSize,
                    PpData
                    );
    return;
  }

  ResetRequired = FALSE;
  if (PpData->PPRequest >= TCG2_PHYSICAL_PRESENCE_VENDOR_SPECIFIC_OPERATION) {
    NewFlags = Flags;
    NewPPFlags = NewFlags.PPFlags;
    PpData->PPResponse = Tcg2PpVendorLibExecutePendingRequest (GetAuthKey (), PpData->PPRequest, &NewPPFlags, &ResetRequired);
    NewFlags.PPFlags = NewPPFlags;
  } else {
    if (!RequestConfirmed) {
      //
      // Print confirm text and wait for approval.
      //
      Status = OemSvcTpmUserConfirmDialog (PpData->PPRequest, TPM_DEVICE_2_0, &RequestConfirmed);
      if (EFI_ERROR (Status)) {
        RequestConfirmed = Tcg2UserConfirm (PpData->PPRequest, PpData->PPRequestParameter);
      }
    }

    //
    // Execute requested physical presence command
    //
    PpData->PPResponse = TCG_PP_OPERATION_RESPONSE_USER_ABORT;
    NewFlags = Flags;
    if (RequestConfirmed) {
      Tcg2ExecutePhysicalPresence (&mTrEEPhysicalPresenceProtocol, PpData->PPRequest, &PpData->PPResponse, &NewFlags, PpData->PPRequestParameter);
    }
  }

  //
  // Save the flags if it is updated.
  //
  if (CompareMem (&Flags, &NewFlags, sizeof(EFI_TCG2_PHYSICAL_PRESENCE_FLAGS)) != 0) {
    Status   = gRT->SetVariable (
                      TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                      &gEfiTcg2PhysicalPresenceGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      sizeof (EFI_TCG2_PHYSICAL_PRESENCE_FLAGS),
                      &NewFlags
                      ); 
    if (EFI_ERROR (Status)) {
      return;
    }
  }

  //
  // Clear request
  //
  if ((NewFlags.PPFlags & TCG2_LIB_PP_FLAG_RESET_TRACK) == 0) {
    PpData->LastPPRequest = PpData->PPRequest;
    PpData->PPRequest = TCG2_PHYSICAL_PRESENCE_NO_ACTION;
    PpData->PPRequestParameter = 0;
  }

  //
  // Save changes
  //
  DataSize = sizeof (EFI_TCG2_PHYSICAL_PRESENCE);
  Status = gRT->SetVariable (
                  TCG2_PHYSICAL_PRESENCE_VARIABLE,
                  &gEfiTcg2PhysicalPresenceGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  DataSize,
                  PpData
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  if (!mLockInEntry) {
    //
    // Reset system if variable is not locked
    //
    Print (L"Rebooting system to make TPM settings in effect\n");
    gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
    ASSERT (FALSE);
  }
  
  if (PpData->PPResponse == TCG_PP_OPERATION_RESPONSE_USER_ABORT) {
    return;
  }

  //
  // Reset system to make new TPM settings in effect
  //
  switch (PpData->LastPPRequest) {
  
  case TCG2_PHYSICAL_PRESENCE_CLEAR:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR_2:
  case TCG2_PHYSICAL_PRESENCE_ENABLE_CLEAR_3:
  case TCG2_PHYSICAL_PRESENCE_SET_PCR_BANKS:
  case TCG2_PHYSICAL_PRESENCE_CHANGE_EPS:
  case TCG2_PHYSICAL_PRESENCE_LOG_ALL_DIGESTS:
    break;
  
  default:
    if (PpData->LastPPRequest >= TCG2_PHYSICAL_PRESENCE_VENDOR_SPECIFIC_OPERATION) {
      if (ResetRequired) {
        break;
      } else {
        return ;
      }
    }
    if (PpData->PPRequest != TCG2_PHYSICAL_PRESENCE_NO_ACTION) {
      break;
    }
    return;
  }

  Print (L"Rebooting system to make TPM2 settings in effect\n");
  gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
  ASSERT (FALSE);  
}

/**
  The driver's entry point.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
DriverEntry (
  IN EFI_HANDLE                               ImageHandle,
  IN EFI_SYSTEM_TABLE                         *SystemTable
  )
{
  EFI_STATUS                                  Status;
  UINTN                                       DataSize;
  EFI_TCG2_PHYSICAL_PRESENCE                  PpData;
  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS            PpiFlags;
  VOID                                        *Registration;
  VOID                                        *Interface;
  H2O_BDS_CP_HANDLE                           CpHandle;
  EFI_TREE_PHYSICAL_PRESENCE                  TrEEPpData;

  Status = gBS->LocateProtocol (&gEfiTcg2ProtocolGuid, NULL, (VOID **) &Interface);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Initialize physical presence flags.
  //
  DataSize = sizeof (EFI_TCG2_PHYSICAL_PRESENCE_FLAGS);
  Status = gRT->GetVariable (
                  TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                  &gEfiTcg2PhysicalPresenceGuid,
                  NULL,
                  &DataSize,
                  &PpiFlags
                  );
  if (EFI_ERROR (Status)) {
    if (Status == EFI_NOT_FOUND) {
      PpiFlags.PPFlags = PcdGet32 (PcdH2OTpm2PpFlagsDefault);
      Status   = gRT->SetVariable (
                        TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                        &gEfiTcg2PhysicalPresenceGuid,
                        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                        sizeof (EFI_TCG2_PHYSICAL_PRESENCE_FLAGS),
                        &PpiFlags
                        );
    }
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Initialize physical presence variable.
  //
  DataSize = sizeof (EFI_TCG2_PHYSICAL_PRESENCE);
  Status = gRT->GetVariable (
                  TCG2_PHYSICAL_PRESENCE_VARIABLE,
                  &gEfiTcg2PhysicalPresenceGuid,
                  NULL,
                  &DataSize,
                  &PpData
                  );
  if (EFI_ERROR (Status)) {
    if (Status == EFI_NOT_FOUND) {
      ZeroMem ((VOID*)&PpData, sizeof (PpData));
      DataSize = sizeof (EFI_TCG2_PHYSICAL_PRESENCE);
      Status   = gRT->SetVariable (
                        TCG2_PHYSICAL_PRESENCE_VARIABLE,
                        &gEfiTcg2PhysicalPresenceGuid,
                        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                        DataSize,
                        &PpData
                        );
    }
    ASSERT_EFI_ERROR (Status);
  }
  
  //
  // TrEEOnTcg2ThunkBegin
  //
  if (PcdGetBool (PcdCompatibleTrEEPpSupported)) {
    //
    // Syn TCG2 variable if TREE has request within.
    //
    DataSize = sizeof (EFI_TREE_PHYSICAL_PRESENCE);
    Status = gRT->GetVariable (
                    TREE_PHYSICAL_PRESENCE_VARIABLE,
                    &gEfiTrEEPhysicalPresenceGuid,
                    NULL,
                    &DataSize,
                    &TrEEPpData
                    );
    if (!EFI_ERROR (Status) && (TrEEPpData.PPRequest != TREE_PHYSICAL_PRESENCE_NO_ACTION)) {
      PpData.PPRequest          = TrEEPpData.PPRequest;
      PpData.LastPPRequest      = TrEEPpData.LastPPRequest;
      PpData.PPResponse         = TrEEPpData.PPResponse;
      PpData.PPRequestParameter = 0;
    
      Status   = gRT->SetVariable (
                        TCG2_PHYSICAL_PRESENCE_VARIABLE,
                        &gEfiTcg2PhysicalPresenceGuid,
                        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                        sizeof(EFI_TCG2_PHYSICAL_PRESENCE),
                        &PpData
                        );
      ASSERT_EFI_ERROR (Status);
  
      TrEEPpData.PPRequest = TREE_PHYSICAL_PRESENCE_NO_ACTION;
      Status   = gRT->SetVariable (
                        TREE_PHYSICAL_PRESENCE_VARIABLE,
                        &gEfiTrEEPhysicalPresenceGuid,
                        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                        sizeof(EFI_TREE_PHYSICAL_PRESENCE),
                        &TrEEPpData
                        );
      ASSERT_EFI_ERROR (Status);
    }

    //
    // Sync the result of request execution in TCG2 variable back to TREE variable, and lock TrEEPpiFlags.
    //
    BdsCpRegisterHandler (
      &gH2OBdsCpInitProtocolGuid,
      CpProtocolInitCp,
      H2O_BDS_CP_MEDIUM,
      &CpHandle
      );
  }
  //
  // TrEEOnTcg2ThunkEnd
  //
  
  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiTrEEPhysicalPresenceProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mTrEEPhysicalPresenceProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "[TPM2] Install TrEEPhysicalPresenceProtocol failed, Status=%r\n", Status));
    return Status;
  }
  
  EfiCreateProtocolNotifyEvent (&gEndOfBdsBootSelectionProtocolGuid, TPL_CALLBACK, Tcg2ExecuteTpmRequestCallback, NULL, (VOID **) &Registration);

  if (mLockInEntry) {
    //
    // Request have been done, lock the variable.
    //
    EfiCreateProtocolNotifyEvent (&gEdkiiVariableLockProtocolGuid, TPL_CALLBACK, LockVariableCallback, NULL, (VOID **) &Registration);
  }

  return Status;
}

/**
  Execute physical presence operation requested by the OS.

  @param[in]      This                Pointer to EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL.
  @param[in]      CommandCode         Physical presence operation value.
  @param[in]      LastReturnedCode    Return code.
  @param[in, out] PpiFlags            The physical presence interface flags.
  
  @retval EFI_SUCCESS                         Operation completed successfully.
  @retval EFI_INVALID_PARAMETER               One or more of the parameters are incorrect.
  @retval Others                              Unexpected error.
  
**/
EFI_STATUS
EFIAPI
ExeucteTcg2PhysicalPresence (
  IN      EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL *This,
  IN      UINT32                              CommandCode,
  OUT     UINT32                              *LastReturnedCode,
  IN OUT  EFI_TREE_PHYSICAL_PRESENCE_FLAGS    *PpiFlags OPTIONAL
  )
{
  EFI_STATUS                                  Status;
  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS            TcgPpiFlags;
  
  if (PpiFlags == NULL) {
    TcgPpiFlags.PPFlags = 0;
    Status = Tcg2ExecutePhysicalPresence (
               This, 
               CommandCode, 
               LastReturnedCode, 
               &TcgPpiFlags,
               0
               );
  } else {
    TcgPpiFlags.PPFlags = PpiFlags->PPFlags;
    Status = Tcg2ExecutePhysicalPresence (
               This, 
               CommandCode, 
               LastReturnedCode, 
               &TcgPpiFlags,
               0
               );
    PpiFlags->PPFlags = (UINT8)TcgPpiFlags.PPFlags;
  }
  return Status;
}

/**
  Execute physical presence operation with TCG2 definition.

  @param[in]      This                        EFI TrEE Protocol instance.
  @param[in]      CommandCode                 Physical presence operation value.
  @param[out]     LastReturnedCode            Execute result
  @param[in, out] PpiFlags                    Physical presence interface flags.
  @param[in]      CommandParameter            Physical presence operation parameter.

  @retval EFI_SUCCESS                         Operation completed successfully.
  @retval EFI_INVALID_PARAMETER               One or more of the parameters are incorrect.
  @retval Others                              Unexpected error.
  
**/
EFI_STATUS
EFIAPI
ExeucteTcg2PhysicalPresenceEx (
  IN      EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL *This,
  IN      UINT32                              CommandCode,
  OUT     UINT32                              *LastReturnedCode,
  IN OUT  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS    *PpiFlags OPTIONAL,
  IN      UINT32                              CommandParameter
  )
{
  EFI_STATUS                                  Status;
  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS            TcgPpiFlags;
  
  if (PpiFlags == NULL) {
    TcgPpiFlags.PPFlags = 0;
    Status = Tcg2ExecutePhysicalPresence (
               This, 
               CommandCode, 
               LastReturnedCode, 
               &TcgPpiFlags,
               CommandParameter
               );
  } else {
    Status = Tcg2ExecutePhysicalPresence (
               This, 
               CommandCode, 
               LastReturnedCode, 
               PpiFlags,
               CommandParameter
               );
  }
  return Status;
}

/**
  Lock PPI flags varaible.

  @param[in]  Event        Event whose notification function is being invoked
  @param[in]  Context      Pointer to the notification function's context

**/
VOID
EFIAPI
LockVariableCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS                        Status;
  EDKII_VARIABLE_LOCK_PROTOCOL      *VariableLockProtocol;

  Status = gBS->LocateProtocol (&gEdkiiVariableLockProtocolGuid, NULL, (VOID **) &VariableLockProtocol);
  if (EFI_ERROR (Status)) {
    return;
  }

  //
  // This flags variable controls whether physical presence is required for TPM command. 
  // It should be protected from malicious software. We set it as read-only variable here.
  //
  Status = VariableLockProtocol->RequestToLock (
                                   VariableLockProtocol,
                                   TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                                   &gEfiTcg2PhysicalPresenceGuid
                                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[TPM2] Error when lock variable %s, Status = %r\n", TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE, Status));
    ASSERT_EFI_ERROR (Status);
  }
}

/**
  Check and execute the physical presence command requested

  @param[in]  Event        Event whose notification function is being invoked
  @param[in]  Context      Pointer to the notification function's context

**/
VOID
EFIAPI
Tcg2ExecuteTpmRequestCallback (
  IN EFI_EVENT                                Event,
  IN VOID                                     *Context
  )
{
  EFI_STATUS                                  Status;
  UINTN                                       DataSize;
  BOOLEAN                                     RequestConfirmed;
  EFI_TCG2_PHYSICAL_PRESENCE                  PpData;
  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS            PpiFlags;
  VOID                                        *Interface;
  BOOLEAN                                     IsRequestValid;

  //
  // Check pending request, if not exist, just return.
  //
  DataSize = sizeof (EFI_TCG2_PHYSICAL_PRESENCE);
  Status = gRT->GetVariable (
                  TCG2_PHYSICAL_PRESENCE_VARIABLE,
                  &gEfiTcg2PhysicalPresenceGuid,
                  NULL,
                  &DataSize,
                  &PpData
                  );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_INFO, "[TPM2] PPRequest=%x\n", PpData.PPRequest));

  DataSize = sizeof (EFI_TCG2_PHYSICAL_PRESENCE_FLAGS);
  Status = gRT->GetVariable (
                  TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                  &gEfiTcg2PhysicalPresenceGuid,
                  NULL,
                  &DataSize,
                  &PpiFlags
                  );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_INFO, "[TPM2] Flags=%x\n", PpiFlags.PPFlags));

  if (EFI_ERROR (Status)) {
    return;
  }
  
  IsRequestValid = Tcg2HaveValidTpmRequest (&PpData, PpiFlags, &RequestConfirmed);
  
  if (!IsRequestValid || RequestConfirmed) {
    //
    // No need to wait connect console, so close the event.
    //
    gBS->CloseEvent (Event);
    mLockInEntry = TRUE;
  } else {
    //
    // User confirm dialog is needed, executes the request when connect console is ready.
    //
    Status = gBS->LocateProtocol (&gEndOfBdsBootSelectionProtocolGuid, NULL, (VOID **) &Interface);
    if (EFI_ERROR (Status)) {
      return;
    }
  }

  //
  // Execute pending TPM request.
  //
  Tcg2ExecutePendingTpmRequest (&PpData, PpiFlags);
  DEBUG ((EFI_D_INFO, "[TPM2] PPResponse = %x\n", PpData.PPResponse));

  //
  // Update the flags since they could be changed after executing the request.
  //
  DataSize = sizeof (EFI_TCG2_PHYSICAL_PRESENCE_FLAGS);
  Status = gRT->GetVariable (
                  TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                  &gEfiTcg2PhysicalPresenceGuid,
                  NULL,
                  &DataSize,
                  &PpiFlags
                  );
  ASSERT_EFI_ERROR (Status);
  
  //
  // Perform hierarchy control
  //
  Status = Tpm2HierarchyDisable (GetAuthKey (), &PpiFlags);
  DEBUG ((EFI_D_INFO, "[TPM2] Tpm2HierarchyDisable  %r\n", Status));
  ASSERT_EFI_ERROR (Status);
}

/**
  BDS Init check point notification handler.
  Replaces the current platformAuth with a value from the RNG.

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context
**/
VOID
EFIAPI
CpProtocolInitCp (
  IN      EFI_EVENT                           Event,
  IN      VOID                                *Context
  )
{
  EFI_STATUS                                  Status;
  EDKII_VARIABLE_LOCK_PROTOCOL                *VariableLockProtocol;
  VOID                                        *Interface;
  UINTN                                       DataSize;
  EFI_TCG2_PHYSICAL_PRESENCE                  TcgPpData;
  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS            TcgPpiFlags;
  EFI_TREE_PHYSICAL_PRESENCE                  TrEEPpData;
  EFI_TREE_PHYSICAL_PRESENCE_FLAGS            TrEEPpiFlags;

  if (!mLockInEntry) {
    //
    // The request hasn't been done, there is no need to update variables yet.
    //
    return;
  }

  Status = gBS->LocateProtocol (&gH2OTpm20DtpmPublishAcpiTableDoneGuid, NULL, (VOID**)&Interface);
  if (!EFI_ERROR (Status)) {
    //
    // TCG PPI spec 1.30 is supported for DTPM2 driver that owned by kernel.
    // Thunk does not need.
    //
    gBS->CloseEvent (Event);
    return;
  }

  //
  // TrEEPhysicalPresence(TCG PPI spec 1.20) on Tcg2PhysicalPresence(TCG PPI spec 1.30) thunk.
  //
  DataSize = sizeof (EFI_TCG2_PHYSICAL_PRESENCE);
  Status = gRT->GetVariable (
                  TCG2_PHYSICAL_PRESENCE_VARIABLE,
                  &gEfiTcg2PhysicalPresenceGuid,
                  NULL,
                  &DataSize,
                  &TcgPpData
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  DataSize = sizeof (EFI_TCG2_PHYSICAL_PRESENCE_FLAGS);
  Status = gRT->GetVariable (
                  TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                  &gEfiTcg2PhysicalPresenceGuid,
                  NULL,
                  &DataSize,
                  &TcgPpiFlags
                  );
  if (EFI_ERROR (Status)) {
    return;
  }
  
  TrEEPpData.PPRequest     = TcgPpData.PPRequest;
  TrEEPpData.LastPPRequest = TcgPpData.LastPPRequest;
  TrEEPpData.PPResponse    = TcgPpData.PPResponse;

  Status   = gRT->SetVariable (
                    TREE_PHYSICAL_PRESENCE_VARIABLE,
                    &gEfiTrEEPhysicalPresenceGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    sizeof(EFI_TREE_PHYSICAL_PRESENCE),
                    &TrEEPpData
                    );
  if (EFI_ERROR (Status)) {
    return;
  }
  
  TrEEPpiFlags.PPFlags = 0;
  if ((TcgPpiFlags.PPFlags & TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CLEAR) == 0) {
    TrEEPpiFlags.PPFlags |= TREE_FLAG_NO_PPI_CLEAR;
  }
  
  Status = gRT->SetVariable (
                  TREE_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                  &gEfiTrEEPhysicalPresenceGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  sizeof(EFI_TREE_PHYSICAL_PRESENCE_FLAGS),
                  &TrEEPpiFlags
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  Status = gBS->LocateProtocol (&gEdkiiVariableLockProtocolGuid, NULL, (VOID **) &VariableLockProtocol);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[TPM2] Locate EdkiiVariableLockProtocol, Status = %r\n", Status));
    return;
  }
  
  //
  // This flags variable controls whether physical presence is required for TPM command. 
  // It should be protected from malicious software. We set it as read-only variable here.
  //
  Status = VariableLockProtocol->RequestToLock (
                                   VariableLockProtocol,
                                   TREE_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                                   &gEfiTrEEPhysicalPresenceGuid
                                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[TPM2] Error when lock variable %s, Status = %r\n", TREE_PHYSICAL_PRESENCE_FLAGS_VARIABLE, Status));
    ASSERT_EFI_ERROR (Status);
  }
  
  gBS->CloseEvent (Event);
}


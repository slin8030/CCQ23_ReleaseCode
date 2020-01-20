/** @file
  Runtime DXE driver implementation for the OemSvc Registration

;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include "IhisiRegistration.h"
#include "IhisiServicesSmm.h"
#include "IhisiFbts.h"
#include "IhisiVats.h"
#include "SecureFlash.h"
#include <SmiTable.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/SeamlessRecoveryLib.h>
#include <Library/LockBoxLib.h>

#include <Protocol/SmmCpu.h>
#include <Protocol/SmmAccess2.h>
#include <Protocol/SmmSwDispatch2.h>

#include <Protocol/SmmExitBootServices.h>

UINTN                                   mSmramRangeCount;
EFI_SMM_VARIABLE_PROTOCOL              *mSmmVariable;
EFI_SMRAM_DESCRIPTOR                   *mSmramRanges;
IHISI_CONTEXT                          *mIhisiContext = NULL;
EFI_SMM_FW_BLOCK_SERVICE_PROTOCOL      *mSmmFwBlockService;
EFI_HANDLE                             mIhisiHandle = NULL;
BOOLEAN                                mCapsuleUpdate = FALSE;
BOOLEAN                                mRuntimeDisableIhisi = FALSE;

/**
  The notification of gEfiSmmFwBlockServiceProtocolGuid protocol is installed

  @param[in] Protocol      Points to the protocol's unique identifier.
  @param[in] Interface     Points to the interface instance.
  @param[in] Handle        The handle on which the interface was installed.

  @retval EFI_SUCCESS      Locate gEfiSmmVariableProtocolGuid protocol successful.
  @retval EFI_NOT_FOUND    Cannot find gEfiSmmVariableProtocolGuid instance.
**/
STATIC
EFI_STATUS
SmmFwBlockNotify (
  IN     CONST EFI_GUID                *Protocol,
  IN     VOID                          *Interface,
  IN     EFI_HANDLE                     Handle
  )
{
  EFI_STATUS            Status;

  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmFwBlockServiceProtocolGuid,
                    NULL,
                    (VOID **) &mSmmFwBlockService
                    );

  if (EFI_ERROR(Status)) {
    ASSERT_EFI_ERROR (Status);
  }
  return Status;
}

/**
  The notification of gEfiSmmVariableProtocolGuid protocol is installed

  @param[in] Protocol      Points to the protocol's unique identifier.
  @param[in] Interface     Points to the interface instance.
  @param[in] Handle        The handle on which the interface was installed.

  @retval EFI_SUCCESS      Locate gEfiSmmVariableProtocolGuid protocol successful.
  @retval EFI_NOT_FOUND    Cannot find gEfiSmmVariableProtocolGuid instance.
**/
STATIC
EFI_STATUS
SmmVariableNotify (
  IN     CONST EFI_GUID                *Protocol,
  IN     VOID                          *Interface,
  IN     EFI_HANDLE                     Handle
  )
{
  return gSmst->SmmLocateProtocol (
                  &gEfiSmmVariableProtocolGuid,
                  NULL,
                  (VOID **) &mSmmVariable
                  );
}

UINT32
GetOtherIhisiStatus (
  IN EFI_STATUS                             Status
  )
{
  switch (Status) {
  case EFI_SUCCESS:
    return (UINT32) IHISI_SUCCESS;
    break;

  case EFI_BUFFER_TOO_SMALL:
    return (UINT32) IHISI_OB_LEN_TOO_SMALL;
    break;

  case EFI_INVALID_PARAMETER:
    return (UINT32) IHISI_INVALID_PARAMETER;
    break;

  case EFI_UNSUPPORTED:
  case EFI_NOT_FOUND:
    return (UINT32) IHISI_UNSUPPORTED_FUNCTION;

  default:
    return (UINT32) IHISI_ACCESS_PROHIBITED;
    break;
  }
}

/**
  Get IHISI status.translated from EFI status

  @param[in] Status  EFI_STATUS

  @return UINT32     IHISI status
**/
UINT32
EfiStatusToIhisiStatus (
  UINT32        FuncType,
  EFI_STATUS    Status
  )
{
  UINT32        IhisiStatus;

  IhisiStatus = (UINT32)Status;

  switch (FuncType) {
    case VATSRead:
    case VATSWrite:
    case VATSNext:
      IhisiStatus = GetVatsIhisiStatus(Status);
      break;

    case FBTSRead:
      if (Status == EFI_ACCESS_DENIED) {
        IhisiStatus = IHISI_FBTS_ME_LOCK_READ_FAILED;
        break;
      }

    case FBTSWrite:
      if (Status == EFI_WRITE_PROTECTED) {
        IhisiStatus = IHISI_FBTS_SKIP_THIS_WRITE_BLOCK;
        break;
      }
    case FBTSGetSupportVersion:
    case FBTSGetPlatformInfo:
    case FBTSGetPlatformRomMap:
    case FBTSGetFlashPartInfo:
    case FBTSComplete:
    case FBTSGetRomFileAndPlatformTable:
    case FBTSSecureFlashBIOS:
    case FBTSOemCustomization1:
    case FBTSOemCustomization2:
    case FBTSSkipMcCheckAndBinaryTrans:
    case FBTSGetATpInformation:
    case FBTSPassPlatforminiSettings:
    case FBTSGetWholeBiosRomMap:
    case FBTSApHookPoint:
    case FBTSOEMCapsuleSecureFalsh:
    case FBTSPassImageFromTool:
    case FBTSGetRuntimeBuffer:
    case FBTSPassImagetoBios:
    case FBTSWriteToSPIRom:
    case FBTSCommonCommunication:
    case FBTSCommonWrite:
    case FBTSCommonRead:
      IhisiStatus = GetFbtsIhisiStatus(Status);
      break;

    case FETSWrite:
    case FETSGetEcPartInfo:
    case FETSRead:
      IhisiStatus = GetFbtsIhisiStatus(Status);
      break;

    default :
      IhisiStatus = GetOtherIhisiStatus(Status);
      break;
  }
  return IhisiStatus;
}

/**
  Returned error code in AL.

  @param[in] IhisiStatus  Returned error code in AL.
**/
VOID
IhisiErrorCodeHandler (
  IN  UINT32            IhisiStatus
  )
{
  UINT32            Eax;

  Eax = IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RAX);
  Eax = (UINT32) ((Eax & 0xffffff00) | IhisiStatus);
  IhisiWriteCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RAX, Eax);
  return;
}

VOID
UpdateApRequestFlag (
  UINT8                         ApRequest
  )
{
  UINT32                        Ecx;

  Ecx = (IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX) & 0xffffff00) | ApRequest;
  IhisiWriteCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX, Ecx);
}

VOID
RecoveryUpdateCheck (
  VOID
  )
{
  BOOLEAN                       RecoveryFlag;
  UINTN                         Size;
  EFI_STATUS                    Status;

  RecoveryFlag = FALSE;
  Size = sizeof (RecoveryFlag);
  Status = RestoreLockBox (&gSecureFlashInfoGuid, &RecoveryFlag, &Size);
  if (EFI_ERROR (Status)) {
    return;
  }
  if (RecoveryFlag) {
    UpdateApRequestFlag (0x00);
  }
}

VOID
CapsuleUpdateCheck (
  VOID
  )
{

  if (mCapsuleUpdate) {
    //
    // Force override CL register as 0x00 to avoid system do reset behavior during Windows Capsule Update
    // It will cause ESRT of LastAttemptStatus field not be updated completely.
    //
    UpdateApRequestFlag (0x00);
  }
}

VOID
SpecialCaseFunHook(
  UINT32       Cmd
  )
{
  if (IsFirmwareFailureRecovery ()) {
    //
    // Check the PcdSeamlessRecoverySignature GUID have been existed on FtwSpareBase during CapsuleUpdate
    //
    mCapsuleUpdate = TRUE;
  }

  switch (Cmd) {
    case FBTSComplete:
      if (PcdGetBool (PcdSecureFlashSupported)) {
      //
      // If the firmware update is initiated by capsule update, IHISI FBTS Complete should not
      // reset or shutdown the system, otherwise the ESRT status won't get updated
      //
        CapsuleUpdateCheck ();

      //
      // In Crisis recovery process, IHISI FBTS complete should not reset or shutdown the system
      //
       RecoveryUpdateCheck ();
      }
      break;

    default:
      break;
  }
  return;
}

IHISI_COMMAND_ENTRY *
IhisiFindCommandEntry (
  UINT32         CmdNumber
  )
{
  IHISI_COMMAND_ENTRY          *Node;
  LIST_ENTRY                   *Link;

  Link = GetFirstNode (&mIhisiContext->CommandList);
  while (!IsNull (&mIhisiContext->CommandList, Link)) {
    Node = IHISI_COMMAND_ENTRY_FROM_LINK (Link);
    if (Node->CmdNumber == CmdNumber) {
      return Node;
    }
    Link = GetNextNode (&mIhisiContext->CommandList, Link);
  }
  return NULL;
}

EFI_STATUS
EFIAPI
IhisiRegisterCommand (
  IN UINT32                              CmdNumber,
  IN IHISI_FUNCTION                      IhisiFunction,
  IN UINT8                               Priority
  )
{
  LIST_ENTRY                 *Link;
  IHISI_COMMAND_ENTRY        *CmdNode;
  IHISI_FUNCTION_ENTRY       *FunctionNode;
  IHISI_FUNCTION_ENTRY       *FunctionEntry;

  CmdNode = IhisiFindCommandEntry (CmdNumber);
  if (CmdNode == NULL) {
    CmdNode = AllocatePool (sizeof (IHISI_COMMAND_ENTRY));
    if (CmdNode == NULL) {
      ASSERT (CmdNode != NULL);
      return EFI_OUT_OF_RESOURCES;
    }
    CmdNode->Signature = IHISI_SIGNATURE;
    CmdNode->CmdNumber = CmdNumber;
    InitializeListHead (&CmdNode->FunctionChain);
    InsertTailList (&mIhisiContext->CommandList, &CmdNode->Link);
  }

  FunctionEntry = AllocatePool (sizeof(IHISI_FUNCTION_ENTRY));
  if (FunctionEntry == NULL) {
    ASSERT (FunctionEntry != NULL);
    return EFI_OUT_OF_RESOURCES;
  }
  FunctionEntry->Signature = IHISI_SIGNATURE;
  FunctionEntry->Function = IhisiFunction;
  FunctionEntry->Priority = Priority;

  Link = GetFirstNode (&CmdNode->FunctionChain);
  while (!IsNull (&CmdNode->FunctionChain, Link)) {
    FunctionNode = IHISI_FUNCTION_ENTRY_FROM_LINK (Link);
    if (FunctionNode->Priority == Priority) {
      FreePool (FunctionEntry);
      return EFI_ALREADY_STARTED;
    }
    if (FunctionNode->Priority < Priority) {
      InsertTailList (Link, &FunctionEntry->Link);
      return EFI_SUCCESS;
    }
    Link = GetNextNode (&CmdNode->FunctionChain, Link);
  }
  InsertTailList (&CmdNode->FunctionChain, &FunctionEntry->Link);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
IhisiRemoveFunctions (
  UINT32         CmdNumber,
  UINT8          FromPriority,
  UINT8          ToPriority
  )
{
  LIST_ENTRY                   *Link;
  LIST_ENTRY                   *NextLink;
  IHISI_COMMAND_ENTRY          *CmdNode;
  IHISI_FUNCTION_ENTRY         *FunctionNode;

  CmdNode = IhisiFindCommandEntry (CmdNumber);
  if (CmdNode == NULL) {
    return EFI_NOT_FOUND;
  }
  Link = GetFirstNode (&CmdNode->FunctionChain);
  while (!IsNull (&CmdNode->FunctionChain, Link)) {
    NextLink = GetNextNode (&CmdNode->FunctionChain, Link);
    FunctionNode = IHISI_FUNCTION_ENTRY_FROM_LINK (Link);
    if ( (FunctionNode->Priority >= FromPriority) &&
         (FunctionNode->Priority <= ToPriority)) {
      RemoveEntryList (&FunctionNode->Link);
      FreePool (FunctionNode);
    }
    Link = NextLink;
  }
  if (IsListEmpty (&CmdNode->FunctionChain)) {
    RemoveEntryList (&CmdNode->Link);
    FreePool (CmdNode);
  }
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
IhisiExecuteCommand (
  UINT32         CmdNumber
  )
{
  EFI_STATUS                    Status;
  LIST_ENTRY                   *Link;
  IHISI_COMMAND_ENTRY          *CmdNode;
  IHISI_FUNCTION_ENTRY         *FunctionNode;

  CmdNode = IhisiFindCommandEntry (CmdNumber);
  if (CmdNode == NULL) {
    return EFI_NOT_FOUND;
  }
  Link = GetFirstNode (&CmdNode->FunctionChain);
  while (!IsNull (&CmdNode->FunctionChain, Link)) {
    FunctionNode = IHISI_FUNCTION_ENTRY_FROM_LINK (Link);
    Status = FunctionNode->Function();
    if (Status == IHISI_END_FUNCTION_CHAIN) {
      return EFI_SUCCESS;
    } else if (Status != EFI_SUCCESS) {
      return Status;
    }
    Link = GetNextNode (&CmdNode->FunctionChain, Link);
  }

  return EFI_SUCCESS;
}


UINT32
EFIAPI
IhisiReadCpuReg32 (
  IN  EFI_SMM_SAVE_STATE_REGISTER       RegisterNum
  )
{
  EFI_STATUS      Status;
  UINT32          Value;

  Status = mIhisiContext->SmmCpu->ReadSaveState (
                                    mIhisiContext->SmmCpu,
                                    sizeof (UINT32),
                                    RegisterNum,
                                    mIhisiContext->IhisiCpuIndex,
                                    &Value
                                    );
  ASSERT_EFI_ERROR (Status);
  return Value;
}

EFI_STATUS
EFIAPI
IhisiWriteCpuReg32 (
  IN  EFI_SMM_SAVE_STATE_REGISTER       RegisterNum,
  IN  UINT32                            Value
  )
{
  EFI_STATUS      Status;

  Status = mIhisiContext->SmmCpu->WriteSaveState (
                                    mIhisiContext->SmmCpu,
                                    sizeof (UINT32),
                                    RegisterNum,
                                    mIhisiContext->IhisiCpuIndex,
                                    &Value
                                    );
  ASSERT_EFI_ERROR (Status);
  return Status;
}

/**
  Check the input memory buffer is whether overlap the SMRAM ranges.

  @param[in] Buffer       The pointer to the buffer to be checked.
  @param[in] BufferSize   The size in bytes of the input buffer

  @retval  TURE        The buffer overlaps SMRAM ranges.
  @retval  FALSE       The buffer doesn't overlap SMRAM ranges.
**/
BOOLEAN
EFIAPI
BufferOverlapSmram (
  IN VOID              *Buffer,
  IN UINTN              BufferSize
  )
{
  UINTN                 Index;
  EFI_PHYSICAL_ADDRESS  BufferEnd;
  EFI_PHYSICAL_ADDRESS  BufferStart;

  if (Buffer == NULL || BufferSize == 0) {
    return FALSE;
  }

  BufferStart = (EFI_PHYSICAL_ADDRESS) (UINTN) Buffer;
  BufferEnd =   (EFI_PHYSICAL_ADDRESS) ((UINTN) Buffer + BufferSize - 1);

  //
  // Integer overflow check
  //
  if (BufferEnd < BufferStart) {
    return TRUE;
  }

  for (Index = 0; Index < mSmramRangeCount; Index ++) {
    //
    // The condition for two ranges doesn't overlap is:
    // Buffer End is smaller than the range start or Buffer start is larger than the range end.
    // so the overlap condition is above condition isn't satisfied.
    //
    if (!(BufferEnd < mSmramRanges[Index].CpuStart ||
        BufferStart >= (mSmramRanges[Index].CpuStart + mSmramRanges[Index].PhysicalSize))) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
  IHISI Services entry function

  @param[in]     DispatchHandle               The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]     Context                      Points to an optional handler context which was specified when the
                                              handler was registered.
  @param[in,out] CommBuffer                   A pointer to a collection of data in memory that will
                                              be conveyed from a non-SMM environment into an SMM environment.
  @param[in,out] CommBufferSize               The size of the CommBuffer.

  @retval EFI_SUCCESS                         The interrupt was handled and quiesced. No other handlers
                                              should still be called.
  @retval EFI_WARN_INTERRUPT_SOURCE_QUIESCED  The interrupt has been quiesced but other handlers should
                                              still be called.
  @retval EFI_WARN_INTERRUPT_SOURCE_PENDING   The interrupt is still pending and other handlers should still
                                              be called.
  @retval EFI_INTERRUPT_PENDING               The interrupt could not be quiesced.

**/
EFI_STATUS
IhisiServicesCallback (
  IN EFI_HANDLE                 DispatchHandle,
  IN CONST VOID                *Context         OPTIONAL,
  IN OUT VOID                  *CommBuffer      OPTIONAL,
  IN OUT UINTN                 *CommBufferSize  OPTIONAL
  )
{
  EFI_STATUS                    Status;
  UINT32                        Cmd;
  UINTN                         Index;
  UINT32                        IhisiStatus;
  UINT32                        RegisterValue;
  IHISI_COMMAND_ENTRY          *CommandEntry;

  if (mSmmFwBlockService == NULL) {
    Status = gSmst->SmmLocateProtocol (
                      &gEfiSmmFwBlockServiceProtocolGuid,
                      NULL,
                      (VOID **) &mSmmFwBlockService
                      );
    if (EFI_ERROR (Status)) {
      IhisiErrorCodeHandler ((UINT32)IHISI_ACCESS_PROHIBITED);
      return EFI_NOT_FOUND;
    }
  }

  if (mSmmVariable == NULL) {
    Status = gSmst->SmmLocateProtocol (
                      &gEfiSmmVariableProtocolGuid,
                      NULL,
                      (VOID **) &mSmmVariable
                      );

    if (EFI_ERROR (Status)) {
      IhisiErrorCodeHandler ((UINT32)IHISI_ACCESS_PROHIBITED);
      return EFI_NOT_FOUND;
    }
  }
  //
  // Find out which CPU triggered the S/W SMI
  //
  for (Index = 0; Index < gSmst->NumberOfCpus; Index++) {
    Status = mIhisiContext->SmmCpu->ReadSaveState (
                                      mIhisiContext->SmmCpu,
                                      sizeof (UINT32),
                                      EFI_SMM_SAVE_STATE_REGISTER_RBX,
                                      Index,
                                      &RegisterValue
                                      );
    if ((Status == EFI_SUCCESS) && (RegisterValue == IHISI_EBX_SIGNATURE)) {
      break;
    }
  }

  if (Index == gSmst->NumberOfCpus) {
    IhisiErrorCodeHandler ((UINT32)IHISI_ACCESS_PROHIBITED);
    return EFI_NOT_FOUND;
  }

  if (mRuntimeDisableIhisi == TRUE) {
    IhisiErrorCodeHandler ((UINT32)IHISI_ACCESS_PROHIBITED);
    return EFI_NOT_FOUND;
  }

  mIhisiContext->IhisiCpuIndex = Index;

  Status = mIhisiContext->SmmCpu->ReadSaveState (
                                    mIhisiContext->SmmCpu,
                                    sizeof (UINT32),
                                    EFI_SMM_SAVE_STATE_REGISTER_RAX,
                                    mIhisiContext->IhisiCpuIndex,
                                    &RegisterValue
                                    );
  ASSERT_EFI_ERROR (Status);

  Cmd = (UINTN)((RegisterValue >> 8) & 0xFF);

  CommandEntry = IhisiFindCommandEntry (Cmd);
  if (CommandEntry == NULL) {
    IhisiErrorCodeHandler ((UINT32)IHISI_UNSUPPORTED_FUNCTION);
    return EFI_NOT_FOUND;
  }

  SpecialCaseFunHook(Cmd);
  Status = IhisiExecuteCommand (Cmd);
  if ((Status & IHISI_STATUS_BIT) == 00) {
    IhisiStatus = EfiStatusToIhisiStatus (Cmd, Status);
  } else {
    IhisiStatus = (UINT32)Status;
  }

  IhisiErrorCodeHandler (IhisiStatus);

  return EFI_SUCCESS;
}

/**
  IHISI Protocol installation routine

  @retval EFI_SUCCESS:          IHISI Protocol is successfully installed
  @retval Others                Failed to install IHISI Protocol
**/
EFI_STATUS
InstallIhisiProtocol(
  VOID
  )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            Handle;
  EFI_SMM_SW_REGISTER_CONTEXT           SwContext;
  EFI_SMM_SW_DISPATCH2_PROTOCOL        *SwDispatch;

  //
  // Software SMI for IHISI services callback function
  //
  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmSwDispatch2ProtocolGuid,
                    NULL,
                    (VOID **) &SwDispatch
                    );
  if (EFI_ERROR (Status)) {
     ASSERT_EFI_ERROR (Status);
    return Status;
  }

  SwContext.SwSmiInputValue = IHISI_SW_SMI;
  Status = SwDispatch->Register (
                         SwDispatch,
                         IhisiServicesCallback,
                         &SwContext,
                         &mIhisiHandle
                         );
  if (EFI_ERROR (Status)) {
     ASSERT_EFI_ERROR (Status);
     return Status;
  }

  Handle = NULL;
  Status = gSmst->SmmInstallProtocolInterface (
                    &Handle,
                    &gH2OIhisiProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &mIhisiContext->Ihisi
                    );
  if (EFI_ERROR (Status)) {
    FreePool (mIhisiContext);
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InitIhisi (
VOID
  )
{
  EFI_STATUS                  Status;
  UINTN                       Size;
  EFI_SMM_ACCESS2_PROTOCOL   *SmmAccess;
  VOID                       *FwBlockRegistration;
  VOID                       *VariableRegistration;

  //
  // Locate SMM FwBlock Protocol
  //
  mSmmFwBlockService = NULL;
  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmFwBlockServiceProtocolGuid,
                    NULL,
                    (VOID **) &mSmmFwBlockService
                    );

  if (EFI_ERROR (Status)) {
    Status = gSmst->SmmRegisterProtocolNotify (
                      &gEfiSmmFwBlockServiceProtocolGuid,
                      SmmFwBlockNotify,
                      &FwBlockRegistration
                      );
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Locate SMM Variable Protocol
  //
  mSmmVariable = NULL;
  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmVariableProtocolGuid,
                    NULL,
                    (VOID **) &mSmmVariable
                    );

  if (EFI_ERROR (Status)) {
    Status = gSmst->SmmRegisterProtocolNotify (
                      &gEfiSmmVariableProtocolGuid,
                      SmmVariableNotify,
                      &VariableRegistration
                      );
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Locate SMM Access2 Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiSmmAccess2ProtocolGuid,
                  NULL,
                  (VOID **)&SmmAccess
                  );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  //
  // Get SMRAM range information
  //
  Size = 0;
  Status = SmmAccess->GetCapabilities (SmmAccess, &Size, NULL);
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);
  mSmramRanges = NULL;
  Status = gSmst->SmmAllocatePool (EfiRuntimeServicesData, Size, (VOID **) &mSmramRanges);
  ASSERT (mSmramRanges != NULL);

  Status = SmmAccess->GetCapabilities (SmmAccess, &Size, mSmramRanges);
  ASSERT_EFI_ERROR (Status);

  mSmramRangeCount = Size / sizeof (EFI_SMRAM_DESCRIPTOR);

  //
  //Initialize mIhisiContext
  //
  mIhisiContext = AllocateZeroPool (sizeof (IHISI_CONTEXT));
  if (mIhisiContext == NULL) {
    ASSERT (mIhisiContext != NULL);
    return EFI_OUT_OF_RESOURCES;
  }
  InitializeListHead (&mIhisiContext->CommandList);
  mIhisiContext->Signature                         = IHISI_SIGNATURE;
  mIhisiContext->Ihisi.RegisterCommand             = IhisiRegisterCommand;
  mIhisiContext->Ihisi.RemoveFunctions             = IhisiRemoveFunctions;
  mIhisiContext->Ihisi.ExecuteCommand              = IhisiExecuteCommand;
  mIhisiContext->Ihisi.ReadCpuReg32                = IhisiReadCpuReg32;
  mIhisiContext->Ihisi.WriteCpuReg32               = IhisiWriteCpuReg32;
  mIhisiContext->Ihisi.BufferOverlapSmram          = BufferOverlapSmram;

  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmCpuProtocolGuid,
                    NULL,
                    (VOID **) &mIhisiContext->SmmCpu
                    );
  if (EFI_ERROR (Status)) {
     FreePool (mIhisiContext);
     ASSERT_EFI_ERROR (Status);
    return Status;
  }
  return EFI_SUCCESS;
}

/**
  Unregister the SMI which used by IHISI interface.

  @retval EFI_SUCCESS:          Locate EFI_SMM_SW_DISPATCH2_PROTOCOL success
  @retval Others                EFI_SMM_SW_DISPATCH2_PROTOCOL not installed
**/

STATIC
EFI_STATUS
EFIAPI
RemoveIhisiServicesCallback (
  IN CONST EFI_GUID                       *Protocol,
  IN VOID                                 *Interface,
  IN EFI_HANDLE                           Handle
  )
{
  if (mIhisiHandle != NULL) {
    mRuntimeDisableIhisi = TRUE;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
IhisiRuntimeProtect (
  VOID
  )
{
  VOID                *Registration;

  mRuntimeDisableIhisi = FALSE;
  gSmst->SmmRegisterProtocolNotify (
           &gEdkiiSmmExitBootServicesProtocolGuid,
           RemoveIhisiServicesCallback,
           &Registration
           );

  return EFI_SUCCESS;
}

/**
  Register IHISI sub function if SubFuncTable CmdNumber/AsciiFuncGuid define in PcdIhisiRegisterTable list.

  @param[out] SubFuncTable        Pointer to ihisi register table.
  @param[out] TableCount          SubFuncTable count

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
RegisterIhisiSubFunction (
  IHISI_REGISTER_TABLE          *SubFuncTable,
  UINT16                        TableCount
  )
{
  EFI_STATUS                    Status;
  UINT8                         *PcdTable;
  UINT8                         PcdPriority;
  UINT8                         EndChar;
  UINTN                         Index;
  UINTN                         PcdCount;
  UINTN                         PcdMaxCount;
  UINTN                         SignatureSize;
  BOOLEAN                       PcdFoundRegistered;

  Status             = EFI_SUCCESS;
  EndChar            = 0;
  PcdFoundRegistered = FALSE;

  PcdTable              = (UINT8 *)PcdGetPtr (PcdIhisiRegisterTable);
  PcdMaxCount           = FixedPcdGetPtrSize (PcdIhisiRegisterTable) / sizeof (UINT8);
  if ((FixedPcdGetPtrSize (PcdIhisiRegisterTable) % sizeof (PCD_IHISI_REGISTER_TABLE)) != 0) {
    DEBUG ((EFI_D_ERROR, "PcdIhisiRegisterTable of description not follow PCD_IHISI_REGISTER_TABLE definition, \
                          it may cause some of IHISI function register fail \n"));
  }

  for (Index = 0; Index < TableCount; Index ++) {
    PcdCount   = 0;
    PcdPriority = 0x80;
    PcdFoundRegistered = FALSE;
    SignatureSize = AsciiStrLen (SubFuncTable[Index].FuncSignature);

    //
    // Caculate PCD of address to find 1. CmdNumber 2. FuncSignature 3. Priority
    //
    do {
      if (SubFuncTable[Index].CmdNumber == *(PcdTable + PcdCount)) {
        PcdCount++;
        if (AsciiStrnCmp (SubFuncTable[Index].FuncSignature, (CHAR8 *)(PcdTable + PcdCount), SignatureSize) == 0) {
          if (EndChar == *(PcdTable + PcdCount + SignatureSize)) {
            PcdPriority = *(PcdTable + PcdCount + SignatureSize + 1);
            PcdFoundRegistered = TRUE;
            break;
          }
        }
      }
      PcdCount++;
    } while (PcdCount < PcdMaxCount);

    if (PcdFoundRegistered) {
      Status = IhisiRegisterCommand (SubFuncTable[Index].CmdNumber, SubFuncTable[Index].IhisiFunction, PcdPriority);
      if (EFI_ERROR(Status)) {
        DEBUG ((EFI_D_ERROR, "IHISI command :0x%X, priority : 0x%X, that already has a registered function\n", \
                              SubFuncTable[Index].CmdNumber, PcdPriority));
        ASSERT (FALSE);
      }
    }
  }

  return Status;
}


/** @file

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

/**
  This driver checks whether there is pending TPM request. If yes,
  it will display TPM request information and ask for user confirmation.
  The TPM request will be cleared after it is processed.

Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "PhysicalPresence.h"

EFI_HII_HANDLE mPpStringPackHandle;

EFI_TPM_PHYSICAL_PRESENCE_PROTOCOL  mTpmPhysicalPresenceProtocol = {
  ExecuteTpmPhysicalPresence,
  TcgGetTpmManagementFlags,
  ManipulatePhysicalPresence,
  TpmNvLock
  };

BOOLEAN                             mLockInEntry = FALSE;

/**
  Check and execute the physical presence command requested.

  @param[in]  Event        Event whose notification function is being invoked
  @param[in]  Context      Pointer to the notification function's context

**/
VOID
EFIAPI
ExecuteTpmRequestCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
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
  Lock physical presence.

  @param[in]  Event        Event whose notification function is being invoked
  @param[in]  Context      Pointer to the notification function's context

**/
VOID
EFIAPI
OnReadyToBoot (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  );

/**
  Get TPM physical presence permanent flags.

  @param[in]  TcgProtocol   EFI TCG Protocol instance.
  @param[out] LifetimeLock  physicalPresenceLifetimeLock permanent flag.
  @param[out] CmdEnable     physicalPresenceCMDEnable permanent flag.

  @retval EFI_SUCCESS       Flags were returns successfully.
  @retval other             Failed to locate EFI TCG Protocol.

**/
EFI_STATUS
GetTpmCapability (
  OUT  BOOLEAN                      *LifetimeLock,
  OUT  BOOLEAN                      *CmdEnable
  )
{
  EFI_STATUS                        Status;
  EFI_TCG_PROTOCOL                  *TcgProtocol;
  TPM_RQU_COMMAND_HDR               *TpmRqu;
  TPM_RSP_COMMAND_HDR               *TpmRsp;
  UINT32                            *SendBufPtr;
  UINT8                             SendBuffer[sizeof (*TpmRqu) + sizeof (UINT32) * 3];
  TPM_PERMANENT_FLAGS               *TpmPermanentFlags;
  UINT8                             RecvBuffer[40];

  Status = gBS->LocateProtocol (&gEfiTcgProtocolGuid, NULL, (VOID **) &TcgProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Fill request header
  //
  TpmRsp = (TPM_RSP_COMMAND_HDR*)RecvBuffer;
  TpmRqu = (TPM_RQU_COMMAND_HDR*)SendBuffer;
  
  TpmRqu->tag       = SwapBytes16 (TPM_TAG_RQU_COMMAND);
  TpmRqu->paramSize = SwapBytes32 (sizeof (SendBuffer));
  TpmRqu->ordinal   = SwapBytes32 (TPM_ORD_GetCapability);

  //
  // Set request parameter
  //
  SendBufPtr      = (UINT32*)(TpmRqu + 1);
  WriteUnaligned32 (SendBufPtr++, SwapBytes32 (TPM_CAP_FLAG));
  WriteUnaligned32 (SendBufPtr++, SwapBytes32 (sizeof (TPM_CAP_FLAG_PERMANENT)));
  WriteUnaligned32 (SendBufPtr, SwapBytes32 (TPM_CAP_FLAG_PERMANENT));  
  
  Status = TcgProtocol->PassThroughToTpm (
                          TcgProtocol,
                          sizeof (SendBuffer),
                          (UINT8*)TpmRqu,
                          sizeof (RecvBuffer),
                          (UINT8*)&RecvBuffer
                          );
  ASSERT_EFI_ERROR (Status);
  ASSERT (TpmRsp->tag == SwapBytes16 (TPM_TAG_RSP_COMMAND));
  ASSERT (TpmRsp->returnCode == 0);

  TpmPermanentFlags = (TPM_PERMANENT_FLAGS *)&RecvBuffer[sizeof (TPM_RSP_COMMAND_HDR) + sizeof (UINT32)];

  if (LifetimeLock != NULL) {
    *LifetimeLock = TpmPermanentFlags->physicalPresenceLifetimeLock;
  }

  if (CmdEnable != NULL) {
    *CmdEnable = TpmPermanentFlags->physicalPresenceCMDEnable;
  }

  return Status;
}

/**
  Issue TSC_PhysicalPresence command to TPM.

  @param[in] PhysicalPresence     The state to set the TPM's Physical Presence flags.

  @retval EFI_SUCCESS             TPM executed the command successfully.
  @retval EFI_SECURITY_VIOLATION  TPM returned error when executing the command.
  @retval other                   Failed to locate EFI TCG Protocol.

**/
EFI_STATUS
TpmPhysicalPresence (
  IN      TPM_PHYSICAL_PRESENCE     PhysicalPresence
  )
{
  EFI_STATUS                        Status;
  EFI_TCG_PROTOCOL                  *TcgProtocol;
  TPM_RQU_COMMAND_HDR               *TpmRqu;
  TPM_PHYSICAL_PRESENCE             *TpmPp;
  TPM_RSP_COMMAND_HDR               TpmRsp;
  UINT8                             Buffer[sizeof (*TpmRqu) + sizeof (*TpmPp)];

  Status = gBS->LocateProtocol (&gEfiTcgProtocolGuid, NULL, (VOID **) &TcgProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  TpmRqu = (TPM_RQU_COMMAND_HDR*)Buffer;
  TpmPp = (TPM_PHYSICAL_PRESENCE*)(TpmRqu + 1);

  TpmRqu->tag       = SwapBytes16 (TPM_TAG_RQU_COMMAND);
  TpmRqu->paramSize = SwapBytes32 (sizeof (Buffer));
  TpmRqu->ordinal   = SwapBytes32 (TSC_ORD_PhysicalPresence);
  WriteUnaligned16 (TpmPp, (TPM_PHYSICAL_PRESENCE) SwapBytes16 (PhysicalPresence));  

  Status = TcgProtocol->PassThroughToTpm (
                          TcgProtocol,
                          sizeof (Buffer),
                          (UINT8*)TpmRqu,
                          sizeof (TpmRsp),
                          (UINT8*)&TpmRsp
                          );
  ASSERT_EFI_ERROR (Status);
  ASSERT (TpmRsp.tag == SwapBytes16 (TPM_TAG_RSP_COMMAND));
  if (TpmRsp.returnCode != 0) {
    //
    // If it fails, some requirements may be needed for this command.
    //
    return EFI_SECURITY_VIOLATION;
  }
  return Status;
}

/**
  Issue a TPM command for which no additional output data will be returned.

  @param[in] TcgProtocol              EFI TCG Protocol instance.  
  @param[in] Ordinal                  TPM command code.  
  @param[in] AdditionalParameterSize  Additional parameter size.  
  @param[in] AdditionalParameters     Pointer to the Additional paramaters.  
  
  @retval TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE  Error occurred during sending command to TPM or 
                                                  receiving response from TPM.
  @retval Others                                  Return code from the TPM device after command execution.

**/
UINT32
TpmCommandNoReturnData (
  IN      EFI_TCG_PROTOCOL          *TcgProtocol,
  IN      TPM_COMMAND_CODE          Ordinal,
  IN      UINTN                     AdditionalParameterSize,
  IN      VOID                      *AdditionalParameters
  )
{
  EFI_STATUS                        Status;
  TPM_RQU_COMMAND_HDR               *TpmRqu;
  TPM_RSP_COMMAND_HDR               TpmRsp;
  UINT32                            Size;

  TpmRqu = (TPM_RQU_COMMAND_HDR*) AllocatePool (sizeof (*TpmRqu) + AdditionalParameterSize);
  if (TpmRqu == NULL) {
    return TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;
  }

  TpmRqu->tag       = SwapBytes16 (TPM_TAG_RQU_COMMAND);
  Size              = (UINT32)(sizeof (*TpmRqu) + AdditionalParameterSize);
  TpmRqu->paramSize = SwapBytes32 (Size);
  TpmRqu->ordinal   = SwapBytes32 (Ordinal);
  CopyMem (TpmRqu + 1, AdditionalParameters, AdditionalParameterSize);

  Status = TcgProtocol->PassThroughToTpm (
                          TcgProtocol,
                          Size,
                          (UINT8*)TpmRqu,
                          (UINT32)sizeof (TpmRsp),
                          (UINT8*)&TpmRsp
                          );
  FreePool (TpmRqu);
  if (EFI_ERROR (Status) || (TpmRsp.tag != SwapBytes16 (TPM_TAG_RSP_COMMAND))) {
    return TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;
  }
  return SwapBytes32 (TpmRsp.returnCode);
}

/**
  Execute physical presence operation requested by the OS.

  @param[in]      TcgProtocol         EFI TCG Protocol instance.
  @param[in]      CommandCode         Physical presence operation value.
  @param[in, out] PpiFlags            The physical presence interface flags.
  
  @retval TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE  Unknown physical presence operation.
  @retval TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE  Error occurred during sending command to TPM or 
                                                  receiving response from TPM.
  @retval Others                                  Return code from the TPM device after command execution.

**/
UINT32
ExecutePhysicalPresence (
  IN      EFI_TCG_PROTOCOL            *TcgProtocol,
  IN      UINT32                      CommandCode,
  IN OUT  EFI_PHYSICAL_PRESENCE_FLAGS *PpiFlags
  )
{
  BOOLEAN                           BoolVal;
  UINT32                            TpmResponse;
  UINT32                            InData[5];

  switch (CommandCode) {
  
  case PHYSICAL_PRESENCE_ENABLE:
    return TpmCommandNoReturnData (
             TcgProtocol,
             TPM_ORD_PhysicalEnable,
             0,
             NULL
             );

  case PHYSICAL_PRESENCE_DISABLE:
    return TpmCommandNoReturnData (
             TcgProtocol,
             TPM_ORD_PhysicalDisable,
             0,
             NULL
             );

  case PHYSICAL_PRESENCE_ACTIVATE:
    BoolVal = FALSE;
    return TpmCommandNoReturnData (
             TcgProtocol,
             TPM_ORD_PhysicalSetDeactivated,
             sizeof (BoolVal),
             &BoolVal
             );

  case PHYSICAL_PRESENCE_DEACTIVATE:
    BoolVal = TRUE;
    return TpmCommandNoReturnData (
             TcgProtocol,
             TPM_ORD_PhysicalSetDeactivated,
             sizeof (BoolVal),
             &BoolVal
             );

  case PHYSICAL_PRESENCE_CLEAR:
    return TpmCommandNoReturnData (
             TcgProtocol,
             TPM_ORD_ForceClear,
             0,
             NULL
             );

  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE:
    TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_ENABLE, PpiFlags);
    if (TpmResponse == 0) {
      TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_ACTIVATE, PpiFlags);
    }
    return TpmResponse;

  case PHYSICAL_PRESENCE_DEACTIVATE_DISABLE:
    TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_DEACTIVATE, PpiFlags);
    if (TpmResponse == 0) {
      TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_DISABLE, PpiFlags);
    }
    return TpmResponse;

  case PHYSICAL_PRESENCE_SET_OWNER_INSTALL_TRUE:
    BoolVal = TRUE;
    return TpmCommandNoReturnData (
             TcgProtocol,
             TPM_ORD_SetOwnerInstall,
             sizeof (BoolVal),
             &BoolVal
             );

  case PHYSICAL_PRESENCE_SET_OWNER_INSTALL_FALSE:
    BoolVal = FALSE;
    return TpmCommandNoReturnData (
             TcgProtocol,
             TPM_ORD_SetOwnerInstall,
             sizeof (BoolVal),
             &BoolVal
             );

  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE_OWNER_TRUE:
    //
    // PHYSICAL_PRESENCE_ENABLE_ACTIVATE + PHYSICAL_PRESENCE_SET_OWNER_INSTALL_TRUE
    // PHYSICAL_PRESENCE_SET_OWNER_INSTALL_TRUE will be executed after reboot
    //
    if ((PpiFlags->PPFlags & TCG_VENDOR_LIB_FLAG_RESET_TRACK) == 0) {
      TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_ENABLE_ACTIVATE, PpiFlags);
      PpiFlags->PPFlags |= TCG_VENDOR_LIB_FLAG_RESET_TRACK;
    } else {
      TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_SET_OWNER_INSTALL_TRUE, PpiFlags);
      PpiFlags->PPFlags &= ~TCG_VENDOR_LIB_FLAG_RESET_TRACK;
    }
    return TpmResponse;

  case PHYSICAL_PRESENCE_DEACTIVATE_DISABLE_OWNER_FALSE:
    TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_SET_OWNER_INSTALL_FALSE, PpiFlags);
    if (TpmResponse == 0) {
      TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_DEACTIVATE_DISABLE, PpiFlags);
    }
    return TpmResponse;

  case PHYSICAL_PRESENCE_DEFERRED_PP_UNOWNERED_FIELD_UPGRADE:
    InData[0] = SwapBytes32 (TPM_SET_STCLEAR_DATA);            // CapabilityArea
    InData[1] = SwapBytes32 (sizeof(UINT32));                  // SubCapSize
    InData[2] = SwapBytes32 (TPM_SD_DEFERREDPHYSICALPRESENCE); // SubCap
    InData[3] = SwapBytes32 (sizeof(UINT32));                  // SetValueSize
    InData[4] = SwapBytes32 (1);                               // UnownedFieldUpgrade; bit0
    return TpmCommandNoReturnData (
             TcgProtocol,
             TPM_ORD_SetCapability,
             sizeof (UINT32) * 5,
             InData
             );

  case PHYSICAL_PRESENCE_SET_OPERATOR_AUTH:
    //
    // TPM_SetOperatorAuth
    // This command requires UI to prompt user for Auth data
    // Here it is NOT implemented
    //
    return TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;

  case PHYSICAL_PRESENCE_CLEAR_ENABLE_ACTIVATE:
    TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_CLEAR, PpiFlags);
    if (TpmResponse == 0) {
      TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_ENABLE_ACTIVATE, PpiFlags);
    }
    return TpmResponse;

  case PHYSICAL_PRESENCE_SET_NO_PPI_PROVISION_FALSE:
    PpiFlags->PPFlags &= ~TCG_BIOS_TPM_MANAGEMENT_FLAG_NO_PPI_PROVISION;
    return 0;

  case PHYSICAL_PRESENCE_SET_NO_PPI_PROVISION_TRUE:
    PpiFlags->PPFlags |= TCG_BIOS_TPM_MANAGEMENT_FLAG_NO_PPI_PROVISION;
    return 0;

  case PHYSICAL_PRESENCE_SET_NO_PPI_CLEAR_FALSE:
    PpiFlags->PPFlags &= ~TCG_BIOS_TPM_MANAGEMENT_FLAG_NO_PPI_CLEAR;
    return 0;

  case PHYSICAL_PRESENCE_SET_NO_PPI_CLEAR_TRUE:
    PpiFlags->PPFlags |= TCG_BIOS_TPM_MANAGEMENT_FLAG_NO_PPI_CLEAR;
    return 0;

  case PHYSICAL_PRESENCE_SET_NO_PPI_MAINTENANCE_FALSE:
    PpiFlags->PPFlags &= ~TCG_BIOS_TPM_MANAGEMENT_FLAG_NO_PPI_MAINTENANCE;
    return 0;

  case PHYSICAL_PRESENCE_SET_NO_PPI_MAINTENANCE_TRUE:
    PpiFlags->PPFlags |= TCG_BIOS_TPM_MANAGEMENT_FLAG_NO_PPI_MAINTENANCE;
    return 0;

  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE_CLEAR:
    //
    // PHYSICAL_PRESENCE_ENABLE_ACTIVATE + PHYSICAL_PRESENCE_CLEAR
    // PHYSICAL_PRESENCE_CLEAR will be executed after reboot.
    //
    if ((PpiFlags->PPFlags & TCG_VENDOR_LIB_FLAG_RESET_TRACK) == 0) {
      TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_ENABLE_ACTIVATE, PpiFlags);
      PpiFlags->PPFlags |= TCG_VENDOR_LIB_FLAG_RESET_TRACK;
    } else {
      TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_CLEAR, PpiFlags);
      PpiFlags->PPFlags &= ~TCG_VENDOR_LIB_FLAG_RESET_TRACK;
    }
    return TpmResponse;

  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE_CLEAR_ENABLE_ACTIVATE:
    //
    // PHYSICAL_PRESENCE_ENABLE_ACTIVATE + PHYSICAL_PRESENCE_CLEAR_ENABLE_ACTIVATE
    // PHYSICAL_PRESENCE_CLEAR_ENABLE_ACTIVATE will be executed after reboot.
    //
    if ((PpiFlags->PPFlags & TCG_VENDOR_LIB_FLAG_RESET_TRACK) == 0) {
      TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_ENABLE_ACTIVATE, PpiFlags);
      PpiFlags->PPFlags |= TCG_VENDOR_LIB_FLAG_RESET_TRACK;
    } else {
      TpmResponse = ExecutePhysicalPresence (TcgProtocol, PHYSICAL_PRESENCE_CLEAR_ENABLE_ACTIVATE, PpiFlags);
      PpiFlags->PPFlags &= ~TCG_VENDOR_LIB_FLAG_RESET_TRACK;
    } 
    return TpmResponse;

  default:
    ;
  }
  return TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;
}

/**
  Display the confirm text and get user confirmation.

  @param[in] TpmPpCommand  The requested TPM physical presence command.

  @retval  TRUE            The user has confirmed the changes.
  @retval  FALSE           The user doesn't confirm the changes.
**/
BOOLEAN
UserConfirm (
  IN      UINT32                    TpmPpCommand
  )
{
  CHAR16                            *ConfirmText;
  CHAR16                            *TmpStr1;
  CHAR16                            *TmpStr2;
  UINTN                             BufSize;
  BOOLEAN                           CautionKey;
  EFI_HANDLE                        Handle;
  UINT8                             *Instance;
  EFI_STATUS                        Status;
  H2O_DIALOG_PROTOCOL               *H2ODialog;
  EFI_INPUT_KEY                     Key;

  TmpStr2     = NULL;
  CautionKey  = FALSE;
  BufSize     = CONFIRM_BUFFER_SIZE;
  ConfirmText = AllocateZeroPool (BufSize);
  ASSERT (ConfirmText != NULL);
  if (ConfirmText == NULL) {
    return FALSE;
  }

  mPpStringPackHandle = HiiAddPackages (
                          &gEfiPhysicalPresenceGuid,
                          NULL,
                          PhysicalPresenceDxeStrings,
                          NULL
                          );
  ASSERT (mPpStringPackHandle != NULL);

  switch (TpmPpCommand) {
  
  case PHYSICAL_PRESENCE_ENABLE:
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ENABLE), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ACCEPT_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_DISABLE:
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_DISABLE), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ACCEPT_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_ACTIVATE:
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ACTIVATE), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ACCEPT_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_DEACTIVATE:
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_DEACTIVATE), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ACCEPT_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_CLEAR:
    CautionKey = TRUE;
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_CLEAR), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING_CLEAR), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    StrnCat (ConfirmText, L" \n\n", (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_CAUTION_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE:
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ENABLE_ACTIVATE), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_NOTE_ON), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ACCEPT_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_DEACTIVATE_DISABLE:
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_DEACTIVATE_DISABLE), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_NOTE_OFF), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ACCEPT_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_SET_OWNER_INSTALL_TRUE:
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ALLOW_TAKE_OWNERSHIP), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ACCEPT_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_SET_OWNER_INSTALL_FALSE:
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_DISALLOW_TAKE_OWNERSHIP), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ACCEPT_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE_OWNER_TRUE:
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_TURN_ON), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_NOTE_ON), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ACCEPT_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_DEACTIVATE_DISABLE_OWNER_FALSE:
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_TURN_OFF), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_NOTE_OFF), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ACCEPT_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_DEFERRED_PP_UNOWNERED_FIELD_UPGRADE:
    CautionKey = TRUE;
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_UNOWNED_FIELD_UPGRADE), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_UPGRADE_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING_MAINTAIN), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_CAUTION_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_SET_OPERATOR_AUTH:
    //
    // TPM_SetOperatorAuth
    // This command requires UI to prompt user for Auth data
    // Here it is NOT implemented
    //
    break;

  case PHYSICAL_PRESENCE_CLEAR_ENABLE_ACTIVATE:
    CautionKey = TRUE;
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_CLEAR_TURN_ON), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_NOTE_ON), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING_CLEAR), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING_CLEAR_CONT), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_CAUTION_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_SET_NO_PPI_PROVISION_TRUE:
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_NO_PPI_PROVISION), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_PPI_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ACCEPT_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_NO_PPI_INFO), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_SET_NO_PPI_CLEAR_TRUE:
    CautionKey = TRUE;
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_CLEAR), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_PPI_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_NOTE_CLEAR), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING_CLEAR), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    StrnCat (ConfirmText, L" \n\n", (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_CAUTION_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_NO_PPI_INFO), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_SET_NO_PPI_MAINTENANCE_TRUE:
    CautionKey = TRUE;
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_NO_PPI_MAINTAIN), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_PPI_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING_MAINTAIN), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_CAUTION_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_NO_PPI_INFO), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE_CLEAR:
    CautionKey = TRUE;
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ENABLE_ACTIVATE_CLEAR), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING_CLEAR), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    StrnCat (ConfirmText, L" \n\n", (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_CAUTION_KEY), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);
    break;

  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE_CLEAR_ENABLE_ACTIVATE:
    CautionKey = TRUE;
    TmpStr2 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_ENABLE_ACTIVATE_CLEAR_ENABLE_ACTIVATE), NULL);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_HEAD_STR), NULL);
    UnicodeSPrint (ConfirmText, BufSize, TmpStr1, TmpStr2);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_NOTE_ON), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING_CLEAR), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_WARNING_CLEAR_CONT), NULL);
    StrnCat (ConfirmText, TmpStr1, (BufSize / sizeof (CHAR16)) - StrLen (ConfirmText) - 1);
    FreePool (TmpStr1);

    TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_CAUTION_KEY), NULL);
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

  TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_REJECT_KEY), NULL);
  UnicodeSPrint (ConfirmText + StrLen (ConfirmText), BufSize, TmpStr1, TmpStr2);
  FreePool (TmpStr1);
  FreePool (TmpStr2);

  TmpStr1 = HiiGetString (mPpStringPackHandle, STRING_TOKEN (TPM_INQUERY), NULL);
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
  H2ODialog->ConfirmDialog (0, FALSE, 0, NULL, &Key, ConfirmText);
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
 
   @param[in]  TcgPpData           EFI TCG Physical Presence request data.
   @param[in]  Flags               The physical presence interface flags. 
   @param[out] RequestConfirmed    If the physical presence operation command required user confirm from UI.
                                   True, it indicates the command doesn't require user confirm, or already confirmed 
                                   in last boot cycle by user.
                                   False, it indicates the command need user confirm from UI.

   @retval  TRUE        Physical Presence operation command is valid.
   @retval  FALSE       Physical Presence operation command is invalid.

**/
BOOLEAN
HaveValidTpmRequest  (
  IN      EFI_PHYSICAL_PRESENCE       *TcgPpData,
  IN      EFI_PHYSICAL_PRESENCE_FLAGS Flags,
  OUT     BOOLEAN                     *RequestConfirmed
  )
{
  BOOLEAN  IsRequestValid;

  *RequestConfirmed = FALSE;

  switch (TcgPpData->PPRequest) {
  
  case PHYSICAL_PRESENCE_NO_ACTION:
    *RequestConfirmed = TRUE;
    return TRUE;
  
  case PHYSICAL_PRESENCE_ENABLE:
  case PHYSICAL_PRESENCE_DISABLE:
  case PHYSICAL_PRESENCE_ACTIVATE:
  case PHYSICAL_PRESENCE_DEACTIVATE:
  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE:
  case PHYSICAL_PRESENCE_DEACTIVATE_DISABLE:
  case PHYSICAL_PRESENCE_SET_OWNER_INSTALL_TRUE:
  case PHYSICAL_PRESENCE_SET_OWNER_INSTALL_FALSE:
  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE_OWNER_TRUE:
  case PHYSICAL_PRESENCE_DEACTIVATE_DISABLE_OWNER_FALSE:
  case PHYSICAL_PRESENCE_SET_OPERATOR_AUTH:
    if ((Flags.PPFlags & TCG_BIOS_TPM_MANAGEMENT_FLAG_NO_PPI_PROVISION) != 0) {
      *RequestConfirmed = TRUE;
    }
    break;

  case PHYSICAL_PRESENCE_CLEAR:
  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE_CLEAR:
    if ((Flags.PPFlags & TCG_BIOS_TPM_MANAGEMENT_FLAG_NO_PPI_CLEAR) != 0) {
      *RequestConfirmed = TRUE;
    }
    break;

  case PHYSICAL_PRESENCE_DEFERRED_PP_UNOWNERED_FIELD_UPGRADE:
    if ((Flags.PPFlags & TCG_BIOS_TPM_MANAGEMENT_FLAG_NO_PPI_MAINTENANCE) != 0) {
      *RequestConfirmed = TRUE;
    }
    break;

  case PHYSICAL_PRESENCE_CLEAR_ENABLE_ACTIVATE:
  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE_CLEAR_ENABLE_ACTIVATE:
    if ((Flags.PPFlags & TCG_BIOS_TPM_MANAGEMENT_FLAG_NO_PPI_CLEAR) != 0 && (Flags.PPFlags & TCG_BIOS_TPM_MANAGEMENT_FLAG_NO_PPI_PROVISION) != 0) {
      *RequestConfirmed = TRUE;
    }
    break;

  case PHYSICAL_PRESENCE_SET_NO_PPI_PROVISION_FALSE:
  case PHYSICAL_PRESENCE_SET_NO_PPI_CLEAR_FALSE:
  case PHYSICAL_PRESENCE_SET_NO_PPI_MAINTENANCE_FALSE:
    *RequestConfirmed = TRUE;
    break;

  case PHYSICAL_PRESENCE_SET_NO_PPI_PROVISION_TRUE:
  case PHYSICAL_PRESENCE_SET_NO_PPI_CLEAR_TRUE:
  case PHYSICAL_PRESENCE_SET_NO_PPI_MAINTENANCE_TRUE:
    break;

  default:
    if (TcgPpData->PPRequest >= TCG_PHYSICAL_PRESENCE_VENDOR_SPECIFIC_OPERATION) {
      IsRequestValid = TcgPpVendorLibHasValidRequest (TcgPpData->PPRequest, Flags.PPFlags, RequestConfirmed);
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

  if ((Flags.PPFlags & TCG_VENDOR_LIB_FLAG_RESET_TRACK) != 0) {
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

  @param[in] TcgPpData            Point to the physical presence NV variable.
  @param[in] Flags                The physical presence interface flags.

**/
VOID
ExecutePendingTpmRequest (
  IN OUT  EFI_PHYSICAL_PRESENCE         *TcgPpData,
  IN      EFI_PHYSICAL_PRESENCE_FLAGS   Flags  
  )
{
  EFI_STATUS                        Status;
  EFI_TCG_PROTOCOL                  *TcgProtocol;
  UINTN                             DataSize;
  EFI_PHYSICAL_PRESENCE_FLAGS       NewFlags;
  BOOLEAN                           RequestConfirmed;
  BOOLEAN                           ResetRequired;
  UINT32                            NewPPFlags;

  if (TcgPpData->PPRequest == PHYSICAL_PRESENCE_NO_ACTION) {
    return;
  }

  if (!HaveValidTpmRequest(TcgPpData, Flags, &RequestConfirmed)) {
    //
    // Invalid operation request.
    //
    TcgPpData->PPResponse    = TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE;
    TcgPpData->LastPPRequest = TcgPpData->PPRequest;
    TcgPpData->PPRequest     = PHYSICAL_PRESENCE_NO_ACTION;
    DataSize                 = sizeof (EFI_PHYSICAL_PRESENCE);
    Status = gRT->SetVariable (
                    PHYSICAL_PRESENCE_VARIABLE,
                    &gEfiPhysicalPresenceGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    DataSize,
                    TcgPpData
                    );
    return;
  }

  ResetRequired = FALSE;
  if (TcgPpData->PPRequest >= TCG_PHYSICAL_PRESENCE_VENDOR_SPECIFIC_OPERATION) {
    NewFlags = Flags;
    NewPPFlags = NewFlags.PPFlags;
    TcgPpData->PPResponse = TcgPpVendorLibExecutePendingRequest (TcgPpData->PPRequest, &NewPPFlags, &ResetRequired);
    NewFlags.PPFlags = (UINT8)NewPPFlags;
  } else {
    if (!RequestConfirmed) {
      //
      // Print confirm text and wait for approval.
      //
      Status = OemSvcTpmUserConfirmDialog (TcgPpData->PPRequest, TPM_DEVICE_1_2, &RequestConfirmed);
      if (EFI_ERROR (Status)) {
        RequestConfirmed = UserConfirm (TcgPpData->PPRequest);
      }
    }
    //
    // Execute requested physical presence command
    //
    TcgPpData->PPResponse = TCG_PP_OPERATION_RESPONSE_USER_ABORT;
    NewFlags = Flags;
    if (RequestConfirmed) {
      Status = gBS->LocateProtocol (&gEfiTcgProtocolGuid, NULL, (VOID**) &TcgProtocol);
      ASSERT_EFI_ERROR (Status);
      TcgPpData->PPResponse = ExecutePhysicalPresence (TcgProtocol, TcgPpData->PPRequest, &NewFlags);
    }
  }

  //
  // Save the flags if it is updated.
  //
  if (CompareMem (&Flags, &NewFlags, sizeof(EFI_PHYSICAL_PRESENCE_FLAGS)) != 0) {
    Status   = gRT->SetVariable (
                      PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                      &gEfiPhysicalPresenceGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      sizeof (EFI_PHYSICAL_PRESENCE_FLAGS),
                      &NewFlags
                      ); 
    if (EFI_ERROR (Status)) {
      return;
    }
  }

  //
  // Clear request
  //
  if ((NewFlags.PPFlags & TCG_VENDOR_LIB_FLAG_RESET_TRACK) == 0) {
    TcgPpData->LastPPRequest = TcgPpData->PPRequest;
    TcgPpData->PPRequest = PHYSICAL_PRESENCE_NO_ACTION;    
  }

  //
  // Save changes
  //
  DataSize = sizeof (EFI_PHYSICAL_PRESENCE);
  Status = gRT->SetVariable (
                  PHYSICAL_PRESENCE_VARIABLE,
                  &gEfiPhysicalPresenceGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  DataSize,
                  TcgPpData
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

  if (TcgPpData->PPResponse == TCG_PP_OPERATION_RESPONSE_USER_ABORT) {
    return;
  }

  //
  // Reset system to make new TPM settings in effect
  //
  switch (TcgPpData->LastPPRequest) {
  
  case PHYSICAL_PRESENCE_ACTIVATE:
  case PHYSICAL_PRESENCE_DEACTIVATE:
  case PHYSICAL_PRESENCE_CLEAR:
  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE:
  case PHYSICAL_PRESENCE_DEACTIVATE_DISABLE:
  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE_OWNER_TRUE:
  case PHYSICAL_PRESENCE_DEACTIVATE_DISABLE_OWNER_FALSE:
  case PHYSICAL_PRESENCE_DEFERRED_PP_UNOWNERED_FIELD_UPGRADE:
  case PHYSICAL_PRESENCE_CLEAR_ENABLE_ACTIVATE:
  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE_CLEAR:
  case PHYSICAL_PRESENCE_ENABLE_ACTIVATE_CLEAR_ENABLE_ACTIVATE:      
    break;
  
  default:
    if (TcgPpData->LastPPRequest >= TCG_PHYSICAL_PRESENCE_VENDOR_SPECIFIC_OPERATION) {
      if (ResetRequired) {
        break;
      } else {
        return ;
      }
    }
    if (TcgPpData->PPRequest != PHYSICAL_PRESENCE_NO_ACTION) {
      break;
    }
    return;
  }

  Print (L"Rebooting system to make TPM settings in effect\n");
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
  IN      EFI_HANDLE                ImageHandle,
  IN      EFI_SYSTEM_TABLE          *SystemTable
  )
{
  EFI_STATUS                        Status;
  UINTN                             DataSize;
  EFI_PHYSICAL_PRESENCE             TcgPpData;
  EFI_PHYSICAL_PRESENCE_FLAGS       PpiFlags;
  EFI_EVENT                         Event;  
  VOID                              *Registration;
  VOID                              *Interface;

  Status = gBS->LocateProtocol (&gEfiTcgProtocolGuid, NULL, (VOID **) &Interface);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Initialize physical presence flags.
  //
  DataSize = sizeof (EFI_PHYSICAL_PRESENCE_FLAGS);
  Status = gRT->GetVariable (
                  PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                  &gEfiPhysicalPresenceGuid,
                  NULL,
                  &DataSize,
                  &PpiFlags
                  );
  if (EFI_ERROR (Status)) {
    if (Status == EFI_NOT_FOUND) {
      PpiFlags.PPFlags = PcdGet8 (PcdH2OTpmPpFlagsDefault);
      Status   = gRT->SetVariable (
                        PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                        &gEfiPhysicalPresenceGuid,
                        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      sizeof (EFI_PHYSICAL_PRESENCE_FLAGS),
                        &PpiFlags
                        );
    }
    ASSERT_EFI_ERROR (Status);
  }
  
  //
  // Initialize physical presence variable.
  //
  DataSize = sizeof (EFI_PHYSICAL_PRESENCE);
  Status = gRT->GetVariable (
                  PHYSICAL_PRESENCE_VARIABLE,
                  &gEfiPhysicalPresenceGuid,
                  NULL,
                  &DataSize,
                  &TcgPpData
                  );
  if (EFI_ERROR (Status)) {
    if (Status == EFI_NOT_FOUND) {
      ZeroMem ((VOID*)&TcgPpData, sizeof (TcgPpData));
      DataSize = sizeof (EFI_PHYSICAL_PRESENCE);
      Status   = gRT->SetVariable (
                        PHYSICAL_PRESENCE_VARIABLE,
                        &gEfiPhysicalPresenceGuid,
                        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                        DataSize,
                        &TcgPpData
                        );
    }
    ASSERT_EFI_ERROR (Status);
  }
  
  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiTpmPhysicalPresenceProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mTpmPhysicalPresenceProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[TPM] Install TpmPhysicalPresenceProtocol failed, Status=%r\n", Status));
    return Status;
  }

  EfiCreateProtocolNotifyEvent (&gEndOfBdsBootSelectionProtocolGuid, TPL_CALLBACK, ExecuteTpmRequestCallback, NULL, (VOID **) &Registration);

  if (mLockInEntry) {
    //
    // Request have been done, lock the variable.
    //
    EfiCreateProtocolNotifyEvent (&gEdkiiVariableLockProtocolGuid, TPL_CALLBACK, LockVariableCallback, NULL, (VOID **) &Registration);
  }
  
  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             OnReadyToBoot,
             NULL,
             &Event
             );
  ASSERT_EFI_ERROR (Status);
  
  return Status;
}

/**
  Lock physical presence.

  @param[in]  Event        Event whose notification function is being invoked
  @param[in]  Context      Pointer to the notification function's context

**/
VOID
EFIAPI
OnReadyToBoot (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS                        Status;
  BOOLEAN                           LifetimeLock;
  BOOLEAN                           CmdEnable;
  
  Status = GetTpmCapability (&LifetimeLock, &CmdEnable);
  if (EFI_ERROR (Status)) {
    return;
  }

  if (!CmdEnable) {
    if (LifetimeLock) {
      //
      // physicalPresenceCMDEnable is locked, can't execute physical presence command.
      //
      return;
    }
    Status = TpmPhysicalPresence (TPM_PHYSICAL_PRESENCE_CMD_ENABLE);
    if (EFI_ERROR (Status)) {
      return;
    }
  }

  //
  // Set operator physical presence flags
  //
  TpmPhysicalPresence (TPM_PHYSICAL_PRESENCE_PRESENT);

  //
  // Lock physical presence.
  //
  TpmPhysicalPresence (TPM_PHYSICAL_PRESENCE_NOTPRESENT | TPM_PHYSICAL_PRESENCE_LOCK);

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
                                   PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                                   &gEfiPhysicalPresenceGuid
                                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[TPM] Error when lock variable %s, Status = %r\n", PHYSICAL_PRESENCE_FLAGS_VARIABLE, Status));
    ASSERT_EFI_ERROR (Status);
  }
}

/**
  Check and execute the physical presence command requested.

  @param[in]  Event        Event whose notification function is being invoked
  @param[in]  Context      Pointer to the notification function's context

**/
VOID
EFIAPI
ExecuteTpmRequestCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS                        Status;
  BOOLEAN                           LifetimeLock;
  BOOLEAN                           CmdEnable;
  UINTN                             DataSize;
  EFI_PHYSICAL_PRESENCE             TcgPpData;
  EFI_PHYSICAL_PRESENCE_FLAGS       PpiFlags;
  VOID                              *Interface;
  BOOLEAN                           IsRequestValid;
  BOOLEAN                           RequestConfirmed;

  //
  // Check pending request, if not exist, just return.
  //
  DataSize = sizeof (EFI_PHYSICAL_PRESENCE);
  Status = gRT->GetVariable (
                  PHYSICAL_PRESENCE_VARIABLE,
                  &gEfiPhysicalPresenceGuid,
                  NULL,
                  &DataSize,
                  &TcgPpData
                  );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_INFO, "[TPM] PPRequest=%x\n", TcgPpData.PPRequest));
  
  DataSize = sizeof (EFI_PHYSICAL_PRESENCE_FLAGS);
  Status = gRT->GetVariable (
                  PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                  &gEfiPhysicalPresenceGuid,
                  NULL,
                  &DataSize,
                  &PpiFlags
                  );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_INFO, "[TPM] Flags=%x\n", PpiFlags.PPFlags));

  if (EFI_ERROR (Status)) {
    return;
  }

  IsRequestValid = HaveValidTpmRequest(&TcgPpData, PpiFlags, &RequestConfirmed);
  
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
  
  Status = GetTpmCapability (&LifetimeLock, &CmdEnable);
  if (EFI_ERROR (Status)) {
    return;
  }

  if (!CmdEnable) {
    if (LifetimeLock) {
      //
      // physicalPresenceCMDEnable is locked, can't execute physical presence command.
      //
      return;
    }
    Status = TpmPhysicalPresence (TPM_PHYSICAL_PRESENCE_CMD_ENABLE);
    if (EFI_ERROR (Status)) {
      return;
    }
  }

  //
  // Set operator physical presence flags
  //
  TpmPhysicalPresence (TPM_PHYSICAL_PRESENCE_PRESENT);

  //
  // Execute pending TPM request.
  //
  ExecutePendingTpmRequest (&TcgPpData, PpiFlags);
  DEBUG ((EFI_D_INFO, "[TPM] PPResponse = %x\n", TcgPpData.PPResponse));
}

UINT32
TpmNvDefine (
  IN      EFI_TCG_PROTOCOL          *TcgProtocol,
  IN      TPM_NV_INDEX              NvIndex,
  IN      UINT32                    DataSize
  )
{
  TPM_NV_DEFINE_IN_24PCR                  NvDefineIn;

  ZeroMem (&NvDefineIn, sizeof (NvDefineIn));

  //
  // 1.2 tag
  //
  NvDefineIn.PubInfo.tag = SwapBytes16(TPM_TAG_NV_DATA_PUBLIC);

  //
  // 1.3 nvIndex
  //
  NvDefineIn.PubInfo.nvIndex = SwapBytes32(NvIndex);

  //
  // 1.4 pcrInfoRead
  //
  NvDefineIn.PubInfo.pcrInfoRead.pcrSelection.sizeOfSelect = SwapBytes16(0x03);
  if ((NvIndex != TPM_NV_INDEX_LOCK) && (DataSize != 0)) {
    NvDefineIn.PubInfo.pcrInfoRead.localityAtRelease = 0x01;
  }

  //
  // 1.5 pcrInfoWrite
  //
  NvDefineIn.PubInfo.pcrInfoWrite.pcrSelection.sizeOfSelect = SwapBytes16(0x03);
  if ((NvIndex != TPM_NV_INDEX_LOCK) && (DataSize != 0)) {
    NvDefineIn.PubInfo.pcrInfoWrite.localityAtRelease = 0x01;
  }

  //
  // 1.6 permission
  //
  NvDefineIn.PubInfo.permission.tag = SwapBytes16(TPM_TAG_NV_ATTRIBUTES);

  //
  // 1.7 bReadSTClear
  //
  NvDefineIn.PubInfo.bReadSTClear  = FALSE;

  //
  // 1.8 bWriteSTClear
  //
  NvDefineIn.PubInfo.bWriteSTClear = FALSE;

  //
  // 1.9 bWriteDefine
  //
  NvDefineIn.PubInfo.bWriteDefine  = FALSE;

  //
  // 1.10 dataSize
  //
  NvDefineIn.PubInfo.dataSize = SwapBytes32(DataSize);

  //
  // 2. Prepare EncAuth
  //
  return TpmCommandNoReturnData (
           TcgProtocol,
           TPM_ORD_NV_DefineSpace,
           sizeof(NvDefineIn),
           &NvDefineIn
           );
}

EFI_STATUS
EFIAPI
ExecuteTpmPhysicalPresence (
  IN      EFI_TCG_PROTOCOL          *TcgProtocol,
  IN      UINT32                    CommandCode,
  OUT     UINT32                    *Result
  )
{
  EFI_PHYSICAL_PRESENCE_FLAGS       PpiFlags;

  PpiFlags.PPFlags = FLAG_NO_PPI_PROVISION | FLAG_NO_PPI_CLEAR | FLAG_NO_PPI_MAINTENANCE;

  *Result = ExecutePhysicalPresence (TcgProtocol, CommandCode, &PpiFlags);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
TcgGetTpmManagementFlags (
  IN      EFI_TCG_PROTOCOL               *TcgProtocol,
  OUT     EFI_PHYSICAL_PRESENCE_FLAGS    *TpmMangFlagPtr
  )
{
  EFI_STATUS                             Status;
  UINTN                                  DataSize;
  EFI_PHYSICAL_PRESENCE_FLAGS            PpiFlags;

  if (TpmMangFlagPtr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DataSize = sizeof (EFI_PHYSICAL_PRESENCE_FLAGS);
  Status = gRT->GetVariable (
                  PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                  &gEfiPhysicalPresenceGuid,
                  NULL,
                  &DataSize,
                  &PpiFlags
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *TpmMangFlagPtr = PpiFlags;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ManipulatePhysicalPresence (
  IN      EFI_TCG_PROTOCOL          *TcgProtocol,
  IN      TPM_PHYSICAL_PRESENCE     PhysicalPresence
  )
{
  return TpmPhysicalPresence (PhysicalPresence);
}

UINT32
EFIAPI
TpmNvLock (
  IN      EFI_TCG_PROTOCOL          *TcgProtocol
  )
{
  return TpmNvDefine (TcgProtocol, TPM_NV_INDEX_LOCK, 0);
}

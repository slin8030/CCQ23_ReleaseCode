/** @file
  This file offers some oem service for H2OUVE utility.
;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <Uefi.h>
#include <Base.h>
#include <KernelSetupConfig.h>
#include <ChipsetSetupConfig.h>
#include <IndustryStandard/Pci22.h>
#include <SysPasswordSmm.h>
#include <VariableEditPwd.h>

#include <Library/SmmOemSvcH2oUveLib.h>
#include <Library/PcdLib.h>
#include <Library/SetupUtilityLib.h>
#include <Library/VariableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/SmmServicesTableLib.h>

#include <Protocol/SmmVariable.h>


extern SYS_PASSWORD_PRIVATE             mPasswordInstance;
EFI_SYS_PASSWORD_SERVICE_PROTOCOL       *mSysPwdProtocolPtr2 = & (mPasswordInstance.SysPasswordProtocol);
VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL   mData = { VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL_CONTENT };
UINTN                                   mTryCount = 0;


VOID
SafeFreePool (
  IN      VOID      **Pointer
)
{
  // check if pointer to pointer is null
  if ( (Pointer != NULL) && (*Pointer != NULL)) {
    FreePool (*Pointer); // actually deallocate memory
    *Pointer = NULL; // null terminate
  }
}
#define SafeFreePoolM(p)        SafeFreePool ( (VOID **) & (p))


/**
  Update the global variable(mData) by PCD and Setup variable.

**/
VOID
EFIAPI
UpdateVariableEditBootTypeInfo  (
  VOID
)
{
  EFI_STATUS                            Status = EFI_SUCCESS;
  UINT8                                 *DefaultLegacyBootTypeOrder;
  UINTN                                 Index = 0;
  VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL VariableEditBootTypeInfoData = { VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL_CONTENT };
  KERNEL_CONFIGURATION                  *SetupVariable = NULL;
  UINTN                                 SetupBufferSize = 0;
  UINT32                                SetupAttributes = 0;
  EFI_SMM_VARIABLE_PROTOCOL             *SmmVariable = NULL;

  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmVariableProtocolGuid,
                    NULL,
                    (VOID **)&SmmVariable
                    );

  if (EFI_ERROR (Status) || (SmmVariable == NULL)) {
    DEBUG ( (EFI_D_INFO, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
  } else {
    //
    // Try to get Setup variable.
    //
    SetupVariable      = NULL;
    SetupBufferSize  = 0;
    Status = SmmVariable->SmmGetVariable (
                            SETUP_VARIABLE_NAME,
                            &gSystemConfigurationGuid,
                            &SetupAttributes,
                            &SetupBufferSize,
                            NULL
                            );
    if (Status == EFI_BUFFER_TOO_SMALL) {
      SetupVariable = (KERNEL_CONFIGURATION *) AllocateZeroPool (SetupBufferSize);
      if (SetupVariable == NULL) {
        DEBUG ( (EFI_D_INFO, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
      } else {
        Status = SmmVariable->SmmGetVariable (
                                SETUP_VARIABLE_NAME,
                                &gSystemConfigurationGuid,
                                &SetupAttributes,
                                &SetupBufferSize,
                                SetupVariable
                                );
        if (EFI_ERROR (Status)) {
          FreePool (SetupVariable);
          SetupVariable = NULL;
          DEBUG ( (EFI_D_INFO, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
        }
      }
    } else {
      SetupVariable = NULL;
    }
  }

  //
  // Update BootTypeOrderNewDefaultSequence by Setup variable.
  //
  if (SetupVariable != NULL) {
    for (Index = 0; Index < MAX_BOOT_ORDER_NUMBER; ++Index) {
      VariableEditBootTypeInfoData.BootTypeOrderNewDefaultSequence[Index] = SetupVariable->BootTypeOrder[Index];
    }
  }

  //
  // Update BootTypeOrderDefaultSequence by PcdLegacyBootTypeOrder.
  //
  DefaultLegacyBootTypeOrder = (UINT8 *) PcdGetPtr (PcdLegacyBootTypeOrder);
  if (DefaultLegacyBootTypeOrder != NULL) {
    for (Index = 0; DefaultLegacyBootTypeOrder[Index] != 0 && Index < MAX_BOOT_ORDER_NUMBER; ++Index) {
      VariableEditBootTypeInfoData.BootTypeOrderDefaultSequence[Index] = DefaultLegacyBootTypeOrder[Index];
    }
  }

  CopyMem (&mData , &VariableEditBootTypeInfoData, sizeof (VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL));

}

/**
  Check that whether it is a USB device.

  @param[in] CurrentBbsTable   A BBS table.

  @retval TRUE                 It is a USB device.
  @retval FALSE                It is not a USB device.

**/
STATIC
BOOLEAN
IsUsbDevice (
  IN BBS_TABLE              *CurrentBbsTable
  )
{
  if ((CurrentBbsTable->Class == PCI_CLASS_SERIAL) &&
      (CurrentBbsTable->SubClass == PCI_CLASS_SERIAL_USB)) {
    return TRUE;
  }
  return FALSE;

}

/**
  Callback function for the modifying of Setup variable.

  @param[in] Rax   Value of CPU register.
  @param[in] Rcx   Value of CPU register.
  @param[in] Rsi   Value of CPU register.
  @param[in] Rdi   Value of CPU register.

  @retval EFI_SUCCESS
  @retval EFI_INVALID_PARAMETER          Rsi/Rcx is 0, variable name or GUID is mismatched with Setup variable.

**/
EFI_STATUS
EFIAPI
SetupCallbackSyncCallbackIhisi (
  IN CONST UINT64               Rax,
  IN CONST UINT64               Rcx,
  IN CONST UINT64               Rsi,
  IN CONST UINT64               Rdi
)
{
  UINTN                                 Index = 0;
  CHIPSET_CONFIGURATION                 *SetupNVData  = NULL;
  CALLBACK_LIST                         *CallbackList = NULL;
  CALLBACK_DATA                         *CallbackData = NULL;
  RESULT_ARRAY                          *ResultArray  = NULL;
  UINTN                                 Offset = 0;
  BOOLEAN                               Result = 0;
  EFI_STATUS                            Status = 0;
  CHAR16                                Buffer[0x80] = {0};
  UINTN                                 IndexPwd  = 0;
  VARIABLE_BUFFER_PTR                   *VarBuffer = NULL;
  BOOLEAN                               IsLoadDefault = FALSE;
  H2O_SMI_CALLBACK                      *PwdCallbackData = NULL;
  H2O_CALLBACK_COMMON_HDR               *Hdr = 0;
  H2O_CALLBACK_STATUS                   *PwdCallbackStatus = NULL;
  H2O_CALLBACK_PASSWORD_CHECK           *PwdCheck = NULL;
  H2O_CALLBACK_PASSWORD                 *CallbackPwd = NULL;
  UINTN                                 Size = 0;
  CHAR8                                 *Password = NULL;

  POST_CODE (0x5A);
  //__debugbreak();

  PwdCallbackData = (H2O_SMI_CALLBACK *)(UINTN)Rdi;
  if (PwdCallbackData == NULL) {
    return EFI_ABORTED;
  }

  // Check Zero Vector and Signature
  for (Index = 0; Index < 16; ++Index) {
    if (PwdCallbackData->ZeroVector[Index] != 0) {
      break;
    }
  }

  if ((Index == 16) && (PwdCallbackData->Signature == IHISI_H2O_SMI_CB_SIGNATURE)) {
    // Check Header
    Hdr = (H2O_CALLBACK_COMMON_HDR *)(UINTN)Rsi;
    // TODO: Header type ckeck
    if ( (Hdr->Type != H2O_SMI_CALLBACK_STATUS) &&
         (Hdr->Type != H2O_SMI_CALLBACK_PASSWORD_CHECK) &&
         (Hdr->Type != H2O_SMI_CALLBACK_PASSWORD)) {
      DEBUG ((EFI_D_ERROR, "The header type is out of range.\n"));
      return EFI_ABORTED;
    }

    //
    //Check support status of password
    //
    if (Hdr->Type == H2O_SMI_CALLBACK_STATUS) {
      PwdCallbackStatus = (H2O_CALLBACK_STATUS *) Hdr;
      PwdCallbackStatus->Flag = 0;
      PwdCallbackStatus->Flag |= FLAG_H2O_SMICB_PSW;
      PwdCallbackStatus->Hdr.Status = STS_H2O_SMICB_SUCCESS;
    }

    //
    //Check status of password
    //
    if (Hdr->Type == H2O_SMI_CALLBACK_PASSWORD_CHECK) {

      PwdCheck = (H2O_CALLBACK_PASSWORD_CHECK *) Hdr;
      Status = CommonGetVariableDataAndSize (SETUP_VARIABLE_NAME,
                                         &gSystemConfigurationGuid,
                                         &Size,
                                         (VOID **) &SetupNVData
                                        );

      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "%a:%d Status:%r\n", __FUNCTION__, __LINE__, Status));
        return EFI_ABORTED;
      }

      if ( (SetupNVData->SupervisorFlag == 0) && (SetupNVData->UserFlag == 0)) {
        PwdCheck->Hdr.Status = STS_H2O_SMICB_SUCCESS;
      } else if (mTryCount >= 5) {
        PwdCheck->Hdr.Status  = STS_H2O_SMICB_ACCESS_DENIED;
      } else {
        PwdCheck->Hdr.Status  = STS_H2O_SMICB_PSW_CHECK;
      }

      SafeFreePoolM (SetupNVData);
    }

    //
    //Validate password
    //
    if (Hdr->Type == H2O_SMI_CALLBACK_PASSWORD) {

      CallbackPwd = (H2O_CALLBACK_PASSWORD *) Hdr;
      Password = (CHAR8 *) ((UINTN)CallbackPwd + sizeof (H2O_CALLBACK_PASSWORD));
      SetupNVData = (CHIPSET_CONFIGURATION *)((UINTN)Password + AsciiStrSize (Password));

      if (mTryCount >= 5) {
        CallbackPwd->Hdr.Status = STS_H2O_SMICB_ACCESS_DENIED;
        return EFI_SUCCESS;
      }

      AsciiStrToUnicodeStr (Password, Buffer);

      Status = mSysPwdProtocolPtr2->CheckPassword (mSysPwdProtocolPtr2, Buffer, StrLen (Buffer), SystemSupervisor);
      if (EFI_ERROR (Status)) {
        Status = mSysPwdProtocolPtr2->CheckPassword (mSysPwdProtocolPtr2, Buffer, StrLen (Buffer), SystemUser);
        if (EFI_ERROR (Status)) {
        } else {
          SetupNVData->SetUserPass = 1;
        }
      } else {
        SetupNVData->SetUserPass = 0;
      }
      if (EFI_ERROR (Status)) {
        CallbackPwd->Hdr.Status = STS_H2O_SMICB_PSW_CHK_FAILED;
        mTryCount++;
      } else {
        CallbackPwd->Hdr.Status  = STS_H2O_SMICB_SUCCESS;
        mTryCount = 0;
      }
    }
    return EFI_SUCCESS;
  }


  VarBuffer    = (VARIABLE_BUFFER_PTR *) (UINTN) Rsi;
  if (VarBuffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (CompareGuid (&VarBuffer->Guid, &gSystemConfigurationGuid) != TRUE) {
    return EFI_INVALID_PARAMETER;
  }
  if (StrnCmp (&VarBuffer->Name[0], SETUP_VARIABLE_NAME, VarBuffer->NameLength) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  SetupNVData  = VARIABLE_BUFFER_TO_DATA (VarBuffer);
  CallbackList = (CALLBACK_LIST *) (UINTN) Rdi;
  if (SetupNVData == NULL || CallbackList == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Offset       = (UINTN) (&CallbackList->List);

  ResultArray  = (RESULT_ARRAY *) (UINTN) Rcx;
  if (ResultArray == NULL) {
    return EFI_INVALID_PARAMETER;
  } else {
    ResultArray->ArrayCount = 0;
  }

  POST_CODE (0x5B);
  for (Index = 0; Index < CallbackList->ListCount; Index++) {
    CallbackData = (CALLBACK_DATA *) Offset;
    Result = FALSE;

    switch (CallbackData->CallbackID) {

      //
      // Advance Callback Handle
      //
//[-start-170110-IB07400831-remove]//
//    case KEY_AHCI_OPROM_CONFIG:
//      break;
//
//    case KEY_SERIAL_PORTA:
//    case KEY_SERIAL_PORTA_BASE_IO:
//    case KEY_SERIAL_PORTA_INTERRUPT:
//    case KEY_SERIAL_PORTB:
//    case KEY_SERIAL_PORTB_BASE_IO:
//    case KEY_SERIAL_PORTB_INTERRUPT:
//      break;
//[-end-170110-IB07400831-remove]//

    case KEY_SAVE_EXIT:
    case KEY_SAVE_WITHOUT_EXIT:
      break;

    case KEY_EXIT_DISCARD:
    case KEY_DISCARD_CHANGE:
      break;

    case KEY_SAVE_CUSTOM:
      break;

    case KEY_LOAD_OPTIMAL:
      Result = RESULT_VALUE_BUFFER_INVAID;
      Status = CommonSetVariable (
                 SETUP_VARIABLE_NAME,
                 &gSystemConfigurationGuid,
                 0,
                 0,
                 Buffer
               );
      Status = CommonSetVariable (
                 EFI_TIME_OUT_VARIABLE_NAME,
                 &gEfiGlobalVariableGuid,
                 0,
                 0,
                 Buffer
               );
      IsLoadDefault = TRUE;
      break;

    case KEY_LOAD_CUSTOM:
      Result = TRUE;
      break;

      //
      // Main Callback Routine
      //
    case KEY_SUPERVISOR_PASSWORD:
      AsciiStrToUnicodeStr (CallbackData->StrData, Buffer);
      for (IndexPwd = 0; IndexPwd < ARRAY_SIZE (Buffer); ++IndexPwd) {
        Buffer[IndexPwd] = (Buffer[IndexPwd] == L'\t' ? 0 : Buffer[IndexPwd]);
      }
      IndexPwd = StrLen (Buffer) + 1; // new password start index!

      Status = mSysPwdProtocolPtr2->CheckPassword (mSysPwdProtocolPtr2, Buffer, StrLen (Buffer), SystemSupervisor);
      if (PcdGetBool (PcdSecureSysPasswordSupported)) {
        mSysPwdProtocolPtr2->UnlockPassword (mSysPwdProtocolPtr2, Buffer, StrLen (Buffer));
      }
      if ( (Status == EFI_NOT_FOUND) && (Buffer[0] == 0)) {
        // No Password!
        if (Buffer[IndexPwd] == 0) {
          Status = mSysPwdProtocolPtr2->DisablePassword (mSysPwdProtocolPtr2, SystemSupervisor);
          SetupNVData->SupervisorFlag = 0;
        } else {
          Status = mSysPwdProtocolPtr2->SetPassword (mSysPwdProtocolPtr2, &Buffer[IndexPwd], StrLen (&Buffer[IndexPwd]), SystemSupervisor);
          SetupNVData->SupervisorFlag = 1;
        }
      } else if (!EFI_ERROR (Status)) {
        // Password check ok!
        if (Buffer[IndexPwd] == 0) {
          Status = mSysPwdProtocolPtr2->DisablePassword (mSysPwdProtocolPtr2, SystemSupervisor);
          SetupNVData->SupervisorFlag = 0;
          Status = mSysPwdProtocolPtr2->DisablePassword (mSysPwdProtocolPtr2, SystemUser);
          SetupNVData->UserFlag = 0;
        } else {
          Status = mSysPwdProtocolPtr2->SetPassword (mSysPwdProtocolPtr2, &Buffer[IndexPwd], StrLen (&Buffer[IndexPwd]), SystemSupervisor);
          SetupNVData->SupervisorFlag = 1;
        }
      }
      if (PcdGetBool (PcdSecureSysPasswordSupported)) {
        mSysPwdProtocolPtr2->LockPassword (mSysPwdProtocolPtr2);
      }
      break;

    case KEY_USER_PASSWORD:
      AsciiStrToUnicodeStr (CallbackData->StrData, Buffer);
      for (IndexPwd = 0; IndexPwd < ARRAY_SIZE (Buffer); ++IndexPwd) {
        Buffer[IndexPwd] = (Buffer[IndexPwd] == L'\t' ? 0 : Buffer[IndexPwd]);
      }
      IndexPwd = StrLen (Buffer) + 1; // new password start index!

      Status = mSysPwdProtocolPtr2->CheckPassword (mSysPwdProtocolPtr2, Buffer, StrLen (Buffer), SystemUser);
      if (PcdGetBool (PcdSecureSysPasswordSupported)) {
        mSysPwdProtocolPtr2->UnlockPassword (mSysPwdProtocolPtr2, Buffer, StrLen (Buffer));
      }
      if ( (Status == EFI_NOT_FOUND) && (Buffer[0] == 0)) {
        // No Password!
        if (Buffer[IndexPwd] == 0) {
          Status = mSysPwdProtocolPtr2->DisablePassword (mSysPwdProtocolPtr2, SystemUser);
          SetupNVData->UserFlag = 0;
        } else {
          Status = mSysPwdProtocolPtr2->SetPassword (mSysPwdProtocolPtr2, &Buffer[IndexPwd], StrLen (&Buffer[IndexPwd]), SystemUser);
          SetupNVData->UserFlag = 1;
        }
      } else if (!EFI_ERROR (Status)) {
        // Password check ok!
        if (Buffer[IndexPwd] == 0) {
          Status = mSysPwdProtocolPtr2->DisablePassword (mSysPwdProtocolPtr2, SystemUser);
          SetupNVData->UserFlag = 0;
        } else {
          Status = mSysPwdProtocolPtr2->SetPassword (mSysPwdProtocolPtr2, &Buffer[IndexPwd], StrLen (&Buffer[IndexPwd]), SystemUser);
          SetupNVData->UserFlag = 1;
        }
      }
      if (PcdGetBool (PcdSecureSysPasswordSupported)) {
        mSysPwdProtocolPtr2->LockPassword (mSysPwdProtocolPtr2);
      }
      break;

      //
      // Main Callback Routine
      //
    case KEY_LANGUAGE_UPDATE:
      break;

      //
      // Server Callback Routine
      //
    default:
      break;
    }

    if (ResultArray != NULL) {
      ResultArray->ArrayCount             += 1;
      ResultArray->Array[Index].CallbackID = CallbackData->CallbackID;
      ResultArray->Array[Index].Result     = Result;
    }

    //
    // Move to next callback data
    //
    Offset = Offset + sizeof (CALLBACK_DATA) + AsciiStrLen ( (UINT8 *) CallbackData->StrData);

  }
  if (IsLoadDefault == FALSE) {
    Status = CommonSetVariable (
               EFI_TIME_OUT_VARIABLE_NAME,
               &gEfiGlobalVariableGuid,
               BS_RT_NV_VAR_ATTR,
               sizeof (SetupNVData->Timeout),
               (VOID *) &SetupNVData->Timeout
             );
  }

  return EFI_SUCCESS;
}

/**
  Get device type.

  @param[in] BootTypeOrder   A pointer to boot type order array.
  @param[in] BootTypeCount   arrat count of boot type order.
  @param[in] DevType         Device type, for check with boot type order.

  @retval DevType
  @retval OTHER_DRIVER       It can not found at boot type order array.

**/
UINT8
EFIAPI
UveSmmGetBbsTypeTableFromBootTypeOrder (
  IN UINT8                                  *BootTypeOrder,
  IN UINTN                                  BootTypeCount,
  IN UINT16                                 DevType
)
{
  UINTN                                     Index = 0;
  UINT8                                     FindDevType = OTHER_DRIVER;

  if (BootTypeOrder == NULL) {
    return FindDevType;
  }

  for (Index = 0; Index < BootTypeCount; ++Index) {
    if (BootTypeOrder[Index] == DevType) {
      FindDevType = BootTypeOrder[Index];
      break;
    }
  }

  return FindDevType;
}

/**
  Get the boot type order infomation.

  @param[in] Rax   Value of CPU register.
  @param[in] Rcx   Value of CPU register.
  @param[in] Rsi   Value of CPU register.
  @param[in] Rdi   Value of CPU register.

  @retval EFI_SUCCESS
  @retval EFI_INVALID_PARAMETER          Rdi is 0, Rcx not equal to 01h.
  @retval EFI_NOT_READY                  BootOrder/Setup has not been set.
  @return Other                          Error occurred in this function.

**/
EFI_STATUS
EFIAPI
GetBootTypeOrderInformation (
  IN CONST UINT64                       Rax,
  IN CONST UINT64                       Rcx,
  IN CONST UINT64                       Rsi,
  IN CONST UINT64                       Rdi
)
{
  H2OUVE_BOOT_OPTION_INFORMATION        *Data = NULL;
  CHIPSET_CONFIGURATION                 *SetupVariable = NULL;
  UINTN                                 SetupBufferSize = 0;
  UINT16                                *BootOrder = NULL;
  UINTN                                 BootOrderSize = 0;
  UINTN                                 Count = 0;
  UINTN                                 Index = 0;
  UINT16                                BootString[10] = {0};
  UINT8                                 *BootOption = NULL;
  UINTN                                 BootOptionSize = 0;
  UINT8                                 *WorkingPtr = NULL;
  EFI_DEVICE_PATH_PROTOCOL              *BootOptionDevicePath = NULL;
  UINTN                                 BootOptionDevicePathSize = 0;
  BBS_BBS_DEVICE_PATH                   *BbsDp = NULL;
  UINT8                                 BootTypeOrder[MAX_BOOT_ORDER_NUMBER] = {0};
  EFI_STATUS                            Status = EFI_SUCCESS;
  UINT16                                DevPathLen;

  Data    = (H2OUVE_BOOT_OPTION_INFORMATION *) (UINTN) Rdi;
  if ( (Rcx != _ECX__01H_) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  do {
    Status = CommonGetVariableDataAndSize (EFI_BOOT_ORDER_VARIABLE_NAME,
                                           &gEfiGlobalVariableGuid,
                                           &BootOrderSize,
                                           (VOID **) &BootOrder
                                          );
    if (EFI_ERROR (Status)) {
      // goto memory recycling process.
      break;
    }
    if ( (BootOrder == NULL) || (BootOrderSize == 0)) {
      Status = EFI_NOT_READY;
      // goto memory recycling process.
      break;
    }

    Count = BootOrderSize / sizeof (BootOrder[0]);
    if (Data->BufferSize < (UINT16) (sizeof (H2OUVE_BOOT_OPTION_INFORMATION) + (Count - 1) * sizeof (H2OUVE_BOOT_OPTION_MAPPING))) {
      Data->BufferSize   = (UINT16) (sizeof (H2OUVE_BOOT_OPTION_INFORMATION) + (Count - 1) * sizeof (H2OUVE_BOOT_OPTION_MAPPING));
      Status = EFI_BUFFER_TOO_SMALL;
      // goto memory recycling process.
      break;
    }

    Status = CommonGetVariableDataAndSize (SETUP_VARIABLE_NAME,
                                           &gSystemConfigurationGuid,
                                           &SetupBufferSize,
                                           (VOID **) &SetupVariable
                                          );
    if (EFI_ERROR (Status)) {
      // goto memory recycling process.
      break;
    }
    if ( (SetupVariable == NULL) || (SetupBufferSize == 0)) {
      Status = EFI_NOT_READY;
      // goto memory recycling process.
      break;
    }

// SEG 5.1 need to overwrite it
    Data->BootOrderByType        = (SetupVariable->BootType != EFI_BOOT_TYPE ? TRUE : FALSE);
// SEG 5.1 need to overwrite it
    Data->BootOrderByType       &= (SetupVariable->LegacyNormalMenuType == NORMAL_MENU ? TRUE : FALSE);
    Data->EfiFirst               = (SetupVariable->BootNormalPriority == EFI_FIRST ? TRUE : FALSE);
    Data->Count                  = (UINT16) Count;
    for (Index = 0; Index < MAX_BOOT_ORDER_NUMBER; ++Index) {
      BootTypeOrder[Index] = (UINT8) SetupVariable->BootTypeOrder[Index];
    }
    SafeFreePoolM (SetupVariable);

    for (Index = 0; Index < Count; ++Index) {
      UnicodeSPrint (BootString, sizeof (BootString), L"Boot%04x", BootOrder[Index]);
      Status = CommonGetVariableDataAndSize (BootString,
                                             &gEfiGlobalVariableGuid,
                                             &BootOptionSize,
                                             (VOID **) &BootOption
                                            );
      if (EFI_ERROR (Status)) {
        continue;
      }
      if ( (BootOption == NULL) || (BootOptionSize == 0)) {
        continue;
      }

      //
      // Find device path in Bootxxxx variable
      //
      WorkingPtr               = BootOption;
      WorkingPtr              += sizeof (UINT32);
      DevPathLen               = *(UINT16 *)WorkingPtr;
      WorkingPtr              += sizeof (UINT16);
      WorkingPtr              += (UINTN) StrSize ( (UINT16 *) WorkingPtr);
      BootOptionDevicePath     = (EFI_DEVICE_PATH_PROTOCOL *) WorkingPtr;
      BootOptionDevicePathSize = GetDevicePathSize (BootOptionDevicePath);

      if ( (BBS_DEVICE_PATH == BootOptionDevicePath->Type) && (BBS_BBS_DP == BootOptionDevicePath->SubType)) {
        BbsDp = (VOID *) BootOptionDevicePath;
        WorkingPtr += DevPathLen;

        Data->Map[Index].BootOptionNumber = BootOrder[Index];

        if (IsUsbDevice ((BBS_TABLE *) WorkingPtr)) {
          Data->Map[Index].BootOptionType = BBS_USB;
        } else {
          Data->Map[Index].BootOptionType   = UveSmmGetBbsTypeTableFromBootTypeOrder (
                                                & (BootTypeOrder[0]),
                                                MAX_BOOT_ORDER_NUMBER,
                                                BbsDp->DeviceType
                                              );
        }
      } else {
        Data->Map[Index].BootOptionNumber = BootOrder[Index];
        Data->Map[Index].BootOptionType   = OTHER_DRIVER;
      }
      SafeFreePoolM (BootOption);
    }
  } while (0);

  // memory recycling process.
  SafeFreePoolM (SetupVariable);
  SafeFreePoolM (BootOrder);
  return Status;
}

/**
  Set the boot type order infomation.

  @param[in] Rax   Value of CPU register.
  @param[in] Rcx   Value of CPU register.
  @param[in] Rsi   Value of CPU register.
  @param[in] Rdi   Value of CPU register.

  @retval EFI_SUCCESS
  @retval EFI_INVALID_PARAMETER          Rdi is 0, Rcx not equal to 02h.
  @retval EFI_NOT_READY                  BootOrder has not been set.
  @return Other                          Error occurred in this function.

**/
EFI_STATUS
EFIAPI
SetBootTypeOrderInformation (
  IN CONST UINT64                       Rax,
  IN CONST UINT64                       Rcx,
  IN CONST UINT64                       Rsi,
  IN CONST UINT64                       Rdi
)
{
  H2OUVE_BOOT_OPTION_INFORMATION        *Data = NULL;
  UINTN                                 Index = 0;
  UINT16                                *BootOrder = NULL;
  UINTN                                 BootOrderSize = 0;
  EFI_STATUS                            Status = EFI_SUCCESS;
  UINTN                                 Count = 0;

  Data    = (H2OUVE_BOOT_OPTION_INFORMATION *) (UINTN) Rdi;
  if ( (Rcx != _ECX__02H_) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  do {
    Status = CommonGetVariableDataAndSize (EFI_BOOT_ORDER_VARIABLE_NAME,
                                           &gEfiGlobalVariableGuid,
                                           &BootOrderSize,
                                           (VOID **) &BootOrder
                                          );
    if (EFI_ERROR (Status)) {
      // goto memory recycling process.
      break;
    }
    if ( (BootOrder == NULL) || (BootOrderSize == 0)) {
      Status = EFI_NOT_READY;
      // goto memory recycling process.
      break;
    }

    Count = BootOrderSize / sizeof (BootOrder[0]);
    if (Data->BufferSize < (UINT16) (sizeof (H2OUVE_BOOT_OPTION_INFORMATION) + (Count - 1) * sizeof (H2OUVE_BOOT_OPTION_MAPPING))) {
      Data->BufferSize   = (UINT16) (sizeof (H2OUVE_BOOT_OPTION_INFORMATION) + (Count - 1) * sizeof (H2OUVE_BOOT_OPTION_MAPPING));
      Status = EFI_BUFFER_TOO_SMALL;
      // goto memory recycling process.
      break;
    }

    for (Index = 0; Index < Count; ++Index) {
      BootOrder[Index] = Data->Map[Index].BootOptionNumber;
    }
    Status = CommonSetVariable (
               EFI_BOOT_ORDER_VARIABLE_NAME,
               &gEfiGlobalVariableGuid,
               BS_RT_NV_VAR_ATTR,
               BootOrderSize,
               (VOID *) BootOrder
             );
    if (EFI_ERROR (Status)) {
      // goto memory recycling process.
      break;
    }
  } while (0);

  // memory recycling process.
  SafeFreePoolM (BootOrder);
  return Status;
}

/**
  Get current boot type infomation.

  @param[in] Rax   Value of CPU register.
  @param[in] Rcx   Value of CPU register.
  @param[in] Rsi   Value of CPU register.
  @param[in] Rdi   Value of CPU register.

  @retval EFI_SUCCESS
  @retval EFI_INVALID_PARAMETER          Rdi is 0, Rcx not equal to 03h.
  @retval EFI_NOT_READY                  Setup variable has not been set.
  @retval EFI_BUFFER_TOO_SMALL           Buffer size is smaller than H2OUVE_BOOT_TYPE_INFORMATION.
  @return Other                          Error occurred in this function.

**/
EFI_STATUS
EFIAPI
GetCurrentBootTypeInformation (
  IN CONST UINT64                       Rax,
  IN CONST UINT64                       Rcx,
  IN CONST UINT64                       Rsi,
  IN CONST UINT64                       Rdi
)
{
  H2OUVE_BOOT_TYPE_INFORMATION          *Data = NULL;
  UINTN                                 Index = 0;
  CHIPSET_CONFIGURATION                 *SetupVariable = NULL;
  UINTN                                 SetupBufferSize = 0;
  EFI_STATUS                            Status = EFI_SUCCESS;

  Data    = (H2OUVE_BOOT_TYPE_INFORMATION *) (UINTN) Rdi;
  if ( (Rcx != _ECX__03H_) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Data->BufferSize < (UINT16) sizeof (H2OUVE_BOOT_TYPE_INFORMATION)) {
    Data->BufferSize   = (UINT16) sizeof (H2OUVE_BOOT_TYPE_INFORMATION);
    return EFI_BUFFER_TOO_SMALL;
  }

  Data->Count   =  MAX_BOOT_ORDER_NUMBER;
  StrCpy (& (Data->BootTypeString[0]), & (mData.BootTypeString[0]));

  CopyMem (Data->BootTypeDefaultNumber, mData.BootTypeOrderNewDefaultSequence, MAX_BOOT_ORDER_NUMBER * sizeof (UINT16));

  do {
    Status = CommonGetVariableDataAndSize (SETUP_VARIABLE_NAME,
                                           &gSystemConfigurationGuid,
                                           &SetupBufferSize,
                                           (VOID **) &SetupVariable
                                          );
    if (EFI_ERROR (Status)) {
      // goto memory recycling process.
      break;
    }
    if ( (SetupVariable == NULL) || (SetupBufferSize == 0)) {
      Status = EFI_NOT_READY;
      // goto memory recycling process.
      break;
    }
    for (Index = 0; Index < MAX_BOOT_ORDER_NUMBER; ++Index) {
      Data->BootTypeNumber[Index] = (UINT16) SetupVariable->BootTypeOrder[Index];
    }

  } while (0);

  // memory recycling process.
  SafeFreePoolM (SetupVariable);
  return Status;
}

/**
  Set current boot type infomation.

  @param[in] Rax   Value of CPU register.
  @param[in] Rcx   Value of CPU register.
  @param[in] Rsi   Value of CPU register.
  @param[in] Rdi   Value of CPU register.

  @retval EFI_SUCCESS
  @retval EFI_INVALID_PARAMETER          Rdi is 0, Rcx not equal to 04h.
  @retval EFI_NOT_READY                  Setup variable has not been set.
  @retval EFI_BUFFER_TOO_SMALL           Buffer size is smaller than H2OUVE_BOOT_TYPE_INFORMATION.
  @return Other                          Error occurred in this function.

**/
EFI_STATUS
EFIAPI
SetCurrentBootTypeInformation (
  IN CONST UINT64                       Rax,
  IN CONST UINT64                       Rcx,
  IN CONST UINT64                       Rsi,
  IN CONST UINT64                       Rdi
)
{
  H2OUVE_BOOT_TYPE_INFORMATION          *Data = NULL;
  UINTN                                 Index = 0;
  CHIPSET_CONFIGURATION                 *SetupVariable = NULL;
  UINTN                                 SetupBufferSize = 0;
  EFI_STATUS                            Status = EFI_SUCCESS;

  Data    = (H2OUVE_BOOT_TYPE_INFORMATION *) (UINTN) Rdi;
  if ( (Rcx != _ECX__04H_) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Data->BufferSize < (UINT16) sizeof (H2OUVE_BOOT_TYPE_INFORMATION)) {
    Data->BufferSize   = (UINT16) sizeof (H2OUVE_BOOT_TYPE_INFORMATION);
    return EFI_BUFFER_TOO_SMALL;
  }

  do {
    Status = CommonGetVariableDataAndSize (
               SETUP_VARIABLE_NAME,
               &gSystemConfigurationGuid,
               &SetupBufferSize,
               (VOID **) &SetupVariable
               );
    if (EFI_ERROR (Status)) {
      // goto memory recycling process.
      break;
    }
    if ( (SetupVariable == NULL) || (SetupBufferSize == 0)) {
      Status = EFI_NOT_READY;
      // goto memory recycling process.
      break;
    }
    for (Index = 0; Index < MAX_BOOT_ORDER_NUMBER; ++Index) {
      SetupVariable->BootTypeOrder[Index] = (UINT8) Data->BootTypeNumber[Index];
    }
    Status = CommonSetVariable (SETUP_VARIABLE_NAME,
                                &gSystemConfigurationGuid,
                                BS_RT_NV_VAR_ATTR,
                                SetupBufferSize,
                                (VOID *) SetupVariable
                               );
    if (EFI_ERROR (Status)) {
      // goto memory recycling process.
      break;
    }
  } while (0);

  // memory recycling process.
  SafeFreePoolM (SetupVariable);
  return Status;
}

/**
  Get current boot type name.

  @param[in] Rax   Value of CPU register.
  @param[in] Rcx   Value of CPU register.
  @param[in] Rsi   Value of CPU register.
  @param[in] Rdi   Value of CPU register.

  @retval EFI_SUCCESS
  @retval EFI_INVALID_PARAMETER          Rdi is 0, Rcx not equal to 05h, cannot find the match with BootTypeNumber.
  @retval EFI_BUFFER_TOO_SMALL           Buffer size is smaller than needed.
  @return Other                          Error occurred in this function.

**/
EFI_STATUS
EFIAPI
GetCurrentBootTypeName (
  IN CONST UINT64                       Rax,
  IN CONST UINT64                       Rcx,
  IN CONST UINT64                       Rsi,
  IN CONST UINT64                       Rdi
)
{
  H2OUVE_BOOT_TYPE_NAME                 *Data = NULL;
  UINTN                                 BufferSize = 0;
  UINTN                                 Index = 0;

  Data    = (H2OUVE_BOOT_TYPE_NAME *) (UINTN) Rdi;
  if ( (Rcx != _ECX__05H_) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < mData.BootTypeStringCount; ++Index) {
    if (Data->BootTypeNumber == mData.NameMapping[Index].BootTypeID) {
      BufferSize  = sizeof (H2OUVE_BOOT_TYPE_NAME);
      BufferSize += sizeof (CHAR16) * StrLen (& (mData.NameMapping[Index].BootTypeName[0]));
      break;
    }
  }
  if (Index == mData.BootTypeStringCount) {
    return EFI_INVALID_PARAMETER;
  }
  if (Data->BufferSize < BufferSize) {
    Data->BufferSize   = (UINT32) BufferSize;
    return EFI_BUFFER_TOO_SMALL;
  }
  StrCpy (Data->String, & (mData.NameMapping[Index].BootTypeName[0]));

  return EFI_SUCCESS;
}

/**
  This constructor function will initialize all the function that are consumed by SmmOemSvcH2oUveLib.

  @param[in] ImageHandle The firmware allocated handle for the EFI image.
  @param[in] SystemTable A pointer to the EFI System Table.

  @retval EFI_SUCCESS The constructor always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
SmmOemSvcH2oUveLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                                Status;

  Status = InitializeSysPasswordDriver (ImageHandle, SystemTable);
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_INFO, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return Status;
  }

  UpdateVariableEditBootTypeInfo();

  return EFI_SUCCESS;
}

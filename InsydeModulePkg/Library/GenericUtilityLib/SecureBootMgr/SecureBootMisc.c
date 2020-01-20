/** @file
  The secure boot manager misc function implementation.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "SecureBootMisc.h"

/**
  Wrap original FreePool gBS call in order to decrease code length.
**/
VOID
SecureBootSafeFreePool (
  IN VOID                              **Buffer
  )
{
  if (Buffer != NULL && *Buffer != NULL) {
    gBS->FreePool (*Buffer);
    *Buffer = NULL;
  }
}

/**
  Worker function that prints an EFI_GUID into specified Buffer.

  @param[in]  Guid          Pointer to GUID to print
  @param[out] Buffer        Buffer to print Guid into
  @param[in]  BufferSize    Size of Buffer
**/
VOID
SecureBootGuidToStr (
  IN  EFI_GUID                         *Guid,
  OUT CHAR16                           *Buffer,
  IN  UINTN                            BufferSize
  )
{
  UnicodeSPrint (
    Buffer,
    BufferSize,
    L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    (UINTN)Guid->Data1,
    (UINTN)Guid->Data2,
    (UINTN)Guid->Data3,
    (UINTN)Guid->Data4[0],
    (UINTN)Guid->Data4[1],
    (UINTN)Guid->Data4[2],
    (UINTN)Guid->Data4[3],
    (UINTN)Guid->Data4[4],
    (UINTN)Guid->Data4[5],
    (UINTN)Guid->Data4[6],
    (UINTN)Guid->Data4[7]
    );
}

/**
  Converts a string to GUID value.
  Guid Format is xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx

  @param[in] Str           The registry format GUID string that contains the GUID value.
  @param[out] Guid         A pointer to the converted GUID value.

  @retval EFI_SUCCESS      The GUID string was successfully converted to the GUID value.
  @retval EFI_UNSUPPORTED  The input string is not in registry format.
  @return others           Some error occurred when converting part of GUID value.
**/
EFI_STATUS
SecureBootStrToGuid (
  IN  CHAR16                           *Str,
  OUT EFI_GUID                         *Guid
  )
{
  CHAR16                               *PtrBuffer;
  CHAR16                               *PtrPosition;
  CHAR16                               *Buffer;
  UINTN                                Data;
  UINTN                                Index;
  UINT16                               Digits[3];

  Buffer = AllocateCopyPool (StrSize (Str), Str);
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // The format of GUID string should be L"12345678-1234-1234-1234-1234657890ab".
  //
  PtrBuffer       = Buffer;
  PtrPosition     = PtrBuffer;
  while (*PtrBuffer != L'\0') {
    if (*PtrBuffer == L'-') {
      break;
    }
    PtrBuffer++;
  }
  if (*PtrBuffer == L'\0') {
    FreePool (Buffer);
    return EFI_NOT_FOUND;
  }
  *PtrBuffer      = L'\0';
  Data            = StrHexToUintn (PtrPosition);
  Guid->Data1     = (UINT32)Data;

  //
  // Data2
  //
  PtrBuffer++;
  PtrPosition     = PtrBuffer;
  while (*PtrBuffer != L'\0') {
    if (*PtrBuffer == L'-') {
      break;
    }
    PtrBuffer++;
  }
  if (*PtrBuffer == L'\0') {
    FreePool (Buffer);
    return EFI_NOT_FOUND;
  }
  *PtrBuffer      = L'\0';
  Data            = StrHexToUintn (PtrPosition);
  Guid->Data2     = (UINT16)Data;

  //
  // Data3
  //
  PtrBuffer++;
  PtrPosition     = PtrBuffer;
  while (*PtrBuffer != L'\0') {
    if (*PtrBuffer == L'-') {
      break;
    }
    PtrBuffer++;
  }
  if (*PtrBuffer == L'\0') {
    FreePool (Buffer);
    return EFI_NOT_FOUND;
  }
  *PtrBuffer      = L'\0';
  Data            = StrHexToUintn (PtrPosition);
  Guid->Data3     = (UINT16)Data;

  //
  // Data4[0..1]
  //
  for ( Index = 0 ; Index < 2 ; Index++) {
    PtrBuffer++;
    if ((*PtrBuffer == L'\0') || ( *(PtrBuffer + 1) == L'\0')) {
      FreePool (Buffer);
      return EFI_NOT_FOUND;
    }
    Digits[0]     = *PtrBuffer;
    PtrBuffer++;
    Digits[1]     = *PtrBuffer;
    Digits[2]     = L'\0';
    Data          = StrHexToUintn (Digits);
    Guid->Data4[Index] = (UINT8)Data;
  }

  //
  // skip the '-'
  //
  PtrBuffer++;
  if ((*PtrBuffer != L'-' ) || ( *PtrBuffer == L'\0')) {
    return EFI_NOT_FOUND;
  }

  //
  // Data4[2..7]
  //
  for ( ; Index < 8; Index++) {
    PtrBuffer++;
    if ((*PtrBuffer == L'\0') || ( *(PtrBuffer + 1) == L'\0')) {
      FreePool (Buffer);
      return EFI_NOT_FOUND;
    }
    Digits[0]     = *PtrBuffer;
    PtrBuffer++;
    Digits[1]     = *PtrBuffer;
    Digits[2]     = L'\0';
    Data          = StrHexToUintn (Digits);
    Guid->Data4[Index] = (UINT8)Data;
  }

  FreePool (Buffer);

  return EFI_SUCCESS;
}

/**
  Reclaim string depository by moving the current node pointer to list head.

  @param[in] StringDepository   Pointer to the string repository
**/
VOID
SecureBootReclaimStrDepository (
  IN STRING_DEPOSITORY                 *StrDepository
  )
{
  if (StrDepository != NULL) {
    StrDepository->CurrentNode = StrDepository->ListHead;
  }
}

/**
  Fetch a usable string node from the string depository and return the string token.

  @param[in] CallbackData             Pointer to SECURE_BOOT_MANAGER_CALLBACK_DATA instance
  @param[in] StringDepository         Pointer of the string depository

  @return String token or zero if input parameter is NULL or fail to allocate memory.
**/
EFI_STRING_ID
SecureBootGetStrTokenFromDepository (
  IN SECURE_BOOT_MANAGER_CALLBACK_DATA *CallbackData,
  IN STRING_DEPOSITORY                 *StringDepository
  )
{
  STRING_LIST_NODE                     *CurrentListNode;
  STRING_LIST_NODE                     *NextListNode;

  if (CallbackData == NULL || StringDepository == NULL) {
    return 0;
  }

  CurrentListNode = StringDepository->CurrentNode;

  if ((NULL != CurrentListNode) && (NULL != CurrentListNode->Next)) {
    //
    // Fetch one reclaimed node from the list.
    //
    NextListNode = StringDepository->CurrentNode->Next;
  } else {
    //
    // If there is no usable node in the list, update the list.
    //
    NextListNode = AllocateZeroPool (sizeof (STRING_LIST_NODE));
    if (NextListNode == NULL) {
      return 0;
    }
    NextListNode->StringToken = HiiSetString (
                                  CallbackData->HiiHandle,
                                  NextListNode->StringToken,
                                  L" ",
                                  NULL
                                  );
    ASSERT (NextListNode->StringToken != 0);

    StringDepository->TotalNodeNumber++;

    if (NULL == CurrentListNode) {
      StringDepository->ListHead = NextListNode;
    } else {
      CurrentListNode->Next = NextListNode;
    }
  }

  StringDepository->CurrentNode = NextListNode;

  return StringDepository->CurrentNode->StringToken;
}

/**
  Show H2O confirm dialog with ok button.

  @param[in] StringId       String ID of dialog message

  @retval EFI_SUCCESS      Show H2O confirm dialog successfully.
  @retval EFI_NOT_FOUND    Fail to get string.
**/
EFI_STATUS
SecureBootShowOkConfirmDlg (
  IN EFI_STRING_ID                     StringId
  )
{
  CHAR16                               *String;
  EFI_INPUT_KEY                        Key;

  String = HiiGetString (mSecureBootPrivate.HiiHandle, StringId, NULL);
  if (String == NULL) {
    return EFI_NOT_FOUND;
  }

  mSecureBootPrivate.H2ODialog->ConfirmDialog (
                                  DlgOk,
                                  FALSE,
                                  0,
                                  NULL,
                                  &Key,
                                  String
                                  );
  FreePool (String);
  return EFI_SUCCESS;
}


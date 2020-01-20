/** @file
  Header file for the secure boot manager misc function.

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

#ifndef _SECURE_BOOT_MISC_H_
#define _SECURE_BOOT_MISC_H_

#include "SecureBootMgr.h"
#include "BootMaint.h"

/**
  Wrap original FreePool gBS call in order to decrease code length.
**/
VOID
SecureBootSafeFreePool (
  IN VOID                              **Buffer
  );

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
  );

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
  );

/**
  Reclaim string depository by moving the current node pointer to list head.

  @param[in] StringDepository   Pointer to the string repository
**/
VOID
SecureBootReclaimStrDepository (
  IN STRING_DEPOSITORY                 *StrDepository
  );

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
  );

/**
  Show H2O confirm dialog with ok button.

  @param[in] StringId       String ID of dialog message

  @retval EFI_SUCCESS      Show H2O confirm dialog successfully.
  @retval EFI_NOT_FOUND    Fail to get string.
**/
EFI_STATUS
SecureBootShowOkConfirmDlg (
  IN EFI_STRING_ID                     StringId
  );

#endif

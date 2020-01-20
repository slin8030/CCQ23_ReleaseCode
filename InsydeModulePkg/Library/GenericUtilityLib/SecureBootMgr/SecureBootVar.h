/** @file

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

#ifndef _SECURE_BOOT_VAR_H_
#define _SECURE_BOOT_VAR_H_

#include "SecureBootMgr.h"
#include "SecureBootMisc.h"
#include "EnrollHash.h"
#include <Guid/ImageAuthentication.h>
#include <Protocol/CryptoServices.h>
#include <Library/VariableLib.h>
#include <Library/PrintLib.h>

/**
  Add signature to specific secure boot variable determined by input secure boot manager state.

  @param[in] SecureBootMgrState      Secure boot manager state
  @param[in] SignatureType           Pointer to signature type GUID
  @param[in] SignatureOwnerGuid      Pointer to signature owner GUID
  @param[in] SignatureData           Pointer to signature data buffer
  @param[in] SignatureDataSize       Size in bytes of signature data buffer

  @retval EFI_SUCCESS                Add signature successfully.
  @retval EFI_NOT_FOUND              Target secure boot variable info is not found.
  @retval EFI_ABORTED                Reject to add signature because PK is existing.
  @retval EFI_ABORTED                There is an identical signature in target secure boot variable.
  @retval EFI_OUT_OF_RESOURCES       Fail to allocate memory.
**/
EFI_STATUS
SecureBootVarAddSignature (
  IN SECURE_BOOT_MANAGER_STATE         SecureBootMgrState,
  IN EFI_GUID                          *SignatureType,
  IN EFI_GUID                          *SignatureOwnerGuid,
  IN UINT8                             *SignatureData,
  IN UINTN                             SignatureDataSize
  );

/**
  Delete signature from specific secure boot variable determined by input secure boot manager state.

  @param[in] SecureBootMgrState      Secure boot manager state
  @param[in] SignatureIndex          The index value of signature which is wanted to be deleted

  @retval EFI_SUCCESS                Delete signature successfully.
  @retval EFI_NOT_FOUND              Target secure boot variable info is not found.
  @retval EFI_NOT_FOUND              Signatue is not found because index value is larger than the signature count
                                     in secure boot variable.
**/
EFI_STATUS
SecureBootVarDeleteSignature (
  IN SECURE_BOOT_MANAGER_STATE         SecureBootMgrState,
  IN UINTN                             SignatureIndex
  );

/**
  Update the related forms of specified secure boot variable.

  @param[in] SecureBootMgrState      Secure boot manager state

  @retval EFI_SUCCESS                Update the related forms successfully.
  @retval EFI_NOT_FOUND              Fail to get secure boot vairable info or state is unsupported.
**/
EFI_STATUS
SecureBootVarUpdateForm (
  IN SECURE_BOOT_MANAGER_STATE         SecureBootMgrState
  );

/**
  Update each secure boot variable if it is modified.

  @retval EFI_SUCCESS          Update secure boot variables successfully.
**/
EFI_STATUS
SecureBootVarSaveChange (
  VOID
  );

/**
  Initialize secure boot variable info.

  @retval EFI_SUCCESS          Initialize secure boot variable info successfully.
**/
EFI_STATUS
SecureBootVarInit (
  VOID
  );

extern STRING_DEPOSITORY               *mSecureBootSigOptionStrDepository;
#endif

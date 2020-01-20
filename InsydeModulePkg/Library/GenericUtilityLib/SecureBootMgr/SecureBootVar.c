/** @file
  The secure boot variable function implementation.

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

#include "SecureBootVar.h"

#define TAG_TYPE_VALUE_OID             0x06    /*  6: Object Identifier */
#define TAG_TYPE_VALUE_SEQUENCE        0x30    /* 16: Sequence/sequence of */
#define TAG_TYPE_VALUE_SET             0x31    /* 17: Set/set of */
#define TAG_TYPE_VALUE_PRINTABLESTRING 0x13    /* 19: Printable string (ASCII subset) */

#define LEN_XTND                       0x80    /* Indefinite or long form */
#define LEN_MASK                       0x7F    /* Bits 7 - 1 */

STATIC UINT8                           mSubjectCommonNameOID[] = {0x55, 0x04, 0x03};    /* 2.5.4.3 */

typedef struct {
  EFI_GUID                             *SignatureType;
  EFI_STRING_ID                        StringId;
} SECURE_BOOT_SIGNATURE_TYPE_HII_INFO;

STATIC SECURE_BOOT_SIGNATURE_TYPE_HII_INFO  mSigTypeHiiInfo[] = {
  {&gEfiCertRsa2048Guid   , STRING_TOKEN (STR_SIGNATURE_TYPE_RSA2048_SHA256)},
  {&gEfiCertX509Guid      , STRING_TOKEN (STR_SIGNATURE_TYPE_PCKS7)         },
  {&gEfiCertSha1Guid      , STRING_TOKEN (STR_SIGNATURE_TYPE_SHA1)          },
  {&gEfiCertSha256Guid    , STRING_TOKEN (STR_SIGNATURE_TYPE_SHA256)        },
  {&gEfiCertSha512Guid    , STRING_TOKEN (STR_SIGNATURE_TYPE_SHA512)        },
  {&gEfiCertX509Sha256Guid, STRING_TOKEN (STR_SIGNATURE_TYPE_X509_SHA256)   },
  {&gEfiCertX509Sha384Guid, STRING_TOKEN (STR_SIGNATURE_TYPE_X509_SHA384)   },
  {&gEfiCertX509Sha512Guid, STRING_TOKEN (STR_SIGNATURE_TYPE_X509_SHA512)   }
  };

typedef struct {
  UINT8                *VarData;
  UINTN                VarDataSize;
} SECURE_BOOT_VAR_INFO;

STATIC SECURE_BOOT_VAR_INFO            mPkVarInfo  = {NULL, 0};
STATIC SECURE_BOOT_VAR_INFO            mKekVarInfo = {NULL, 0};
STATIC SECURE_BOOT_VAR_INFO            mDbVarInfo  = {NULL, 0};
STATIC SECURE_BOOT_VAR_INFO            mDbxVarInfo = {NULL, 0};

STRING_DEPOSITORY                      *mSecureBootSigOptionStrDepository;

/**
  Worker function that prints an binary data into specified string buffer.

  @param[in]  BinaryBuffer        Pointer to binary buffer
  @param[in]  BinaryBufferSize    Size in bytes of binary buffer
  @param[out] StrBuffer           Pointer to output string buffer
  @param[in]  StrBufferSize       Size in bytes of output string buffer

  @retval EFI_SUCCESS             The binary data was successfully converted to the string buffer.
  @retval EFI_INVALID_PARAMETER   StrBuffer is NULL or StrBufferSize is zero.
**/
STATIC
EFI_STATUS
BinaryToString (
  IN UINT8                             *BinaryBuffer,
  IN UINTN                             BinaryBufferSize,
  IN CHAR16                            *StrBuffer,
  IN UINTN                             StrBufferSize
  )
{
  UINTN                                Index;
  UINTN                                Count;
  UINTN                                LenPerChar;

  if (StrBuffer == NULL || StrBufferSize == 0) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem (StrBuffer, StrBufferSize);
  if (BinaryBuffer == NULL || BinaryBufferSize == 0) {
    return EFI_SUCCESS;
  }

  LenPerChar = 3;
  Count = MIN(StrBufferSize / sizeof (CHAR16) / LenPerChar, BinaryBufferSize);
  for (Index = 0; Index < Count; Index++) {
    UnicodeSPrint (&StrBuffer[Index * LenPerChar], LenPerChar * sizeof (CHAR16), L"%02x", BinaryBuffer[Index]);
    StrBuffer[(Index + 1) * LenPerChar - 1] = ' ';
  }
  if (Index > 1 && BinaryBufferSize > Count) {
    StrBuffer[Index * LenPerChar - 2] = '.';
    StrBuffer[Index * LenPerChar - 3] = '.';
    StrBuffer[Index * LenPerChar - 4] = '.';
  }
  if (Index > 0) {
    StrBuffer[Index * LenPerChar - 1] = CHAR_NULL;
  }

  return EFI_SUCCESS;
}

/**
  Get the string of signature type GUID.

  @param[in]  HiiHandle           HII handle
  @param[in]  SignatureType       Pointer to signature type GUID

  @return The pointer of signature type string or NULL if fail to get string.
**/
STATIC
CHAR16 *
GetSigTypeStr (
  IN EFI_HII_HANDLE                    HiiHandle,
  IN EFI_GUID                          *SignatureType
  )
{
  UINTN                                Index;

  for (Index = 0; Index < sizeof (mSigTypeHiiInfo) / sizeof (SECURE_BOOT_SIGNATURE_TYPE_HII_INFO); Index++) {
    if (CompareGuid (SignatureType, mSigTypeHiiInfo[Index].SignatureType)) {
      return HiiGetString (HiiHandle, mSigTypeHiiInfo[Index].StringId, NULL);
    }
  }

  return HiiGetString (HiiHandle, STRING_TOKEN (STR_SIGNATURE_TYPE_UNKNOWN), NULL);
}

/**
  Get DER Encoded header info by ASN.1 TLV(Type-Length-Value) encoding from DER Encoded buffer.

  @param[in]  DerData             Pointer to DER buffer
  @param[in]  DerDataSize         Size in bytes of DER buffer
  @param[out] EncodedHeaderSize   Return the size of DER Encoded header
  @param[out] EncodedTag          Return the tag value of DER
  @param[out] EncodedValueSize    Return the size of DER Encoded value

  @retval EFI_SUCCESS             Get DER Encoded header info successfully.
  @retval EFI_INVALID_PARAMETER   Input parameter is invalid.
  @retval EFI_NOT_FOUND           DER Encoded header is incorrect.
**/
STATIC
EFI_STATUS
GetDerEncodedHeaderInfo (
  IN  UINT8                            *DerData,
  IN  UINTN                            DerDataSize,
  OUT UINTN                            *EncodedHeaderSize,
  OUT UINT8                            *EncodedTag,
  OUT UINT32                           *EncodedValueSize
  )
{
  UINTN                                Offset;
  UINT8                                Tag;
  UINT32                               Size;
  UINT32                               Length;
  UINT32                               Index;

  if (DerData == NULL || DerDataSize == 0 || EncodedHeaderSize == NULL || EncodedTag == NULL || EncodedValueSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Offset = 0;
  Tag    = DerData[Offset++];
  Length = DerData[Offset++];
  if ((Length & LEN_XTND) != 0) {
    Length &= LEN_MASK;
    if (Length > 4) {
      return EFI_NOT_FOUND;
    }

    Size = 0;
    for (Index = 0; Index < Length; Index++) {
      Size = (Size << 8) | DerData[Offset++];
    }
  } else {
    Size = Length;
  }

  *EncodedHeaderSize = Offset;
  *EncodedTag        = Tag;
  *EncodedValueSize  = Size;
  return EFI_SUCCESS;
}

/**
  Get CN(Common Name) from subject buffer.

  @param[in]  Subject             Pointer to subject buffer
  @param[in]  SubjectSize         Size in bytes of subject buffer
  @param[out] StrBuffer           Pointer to output string buffer
  @param[in]  StrBufferSize       Size in bytes of output string buffer

  @retval EFI_SUCCESS             Get CN successfully.
  @retval EFI_INVALID_PARAMETER   Subject is NULL or SubjectSize is zero.
  @retval EFI_NOT_FOUND           CN is not found.
  @retval EFI_ABORTED             DER Encoded header is invalid.
**/
STATIC
EFI_STATUS
GetSubjectCommonName (
  IN  UINT8                            *Subject,
  IN  UINTN                            SubjectSize,
  OUT CHAR16                           *StrBuffer,
  IN  UINTN                            StrBufferSize
  )
{
  EFI_STATUS                           Status;
  UINTN                                HeaderSize;
  UINT8                                Tag;
  UINT32                               ValueSize;
  UINT8                                *DataPtr;
  UINTN                                DataSize;
  UINT8                                *ChildData;
  UINTN                                ChildDataSize;
  UINT8                                *GrandChildData;
  UINTN                                GrandChilddDataSize;
  UINT8                                *Oid;
  UINT32                               OidSize;
  CHAR8                                *AsciiStr;
  UINTN                                Index;
  UINTN                                Count;

  if (Subject == NULL || SubjectSize == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Parsing method is referenced from CERT_getSubjectCommonNameAux() in BaseCryptLib.
  //
  // now get the child with the COMMON NAME OID 2.5.4.3 */
  //   Name ::= SEQUENCE of RelativeDistinguishedName
  //   RelativeDistinguishedName = SET of AttributeValueAssertion
  //   AttributeValueAssertion = SEQUENCE { attributeType OID; attributeValue ANY }
  //
  /* Name is a sequence */
  Status = GetDerEncodedHeaderInfo (Subject, SubjectSize, &HeaderSize, &Tag, &ValueSize);
  if (EFI_ERROR (Status) || Tag != TAG_TYPE_VALUE_SEQUENCE) {
    return EFI_ABORTED;
  }

  DataPtr  = Subject + HeaderSize;
  DataSize = ValueSize;
  while (DataSize > 0) {
    /* child should be a SET */
    Status = GetDerEncodedHeaderInfo (DataPtr, DataSize, &HeaderSize, &Tag, &ValueSize);
    if (EFI_ERROR (Status) || Tag != TAG_TYPE_VALUE_SET) {
      return EFI_ABORTED;
    }
    ChildData     = DataPtr + HeaderSize;
    ChildDataSize = ValueSize;
    DataPtr       = DataPtr  + (HeaderSize + ChildDataSize);
    DataSize      = DataSize - (HeaderSize + ValueSize);

    /* GrandChild should be a SEQUENCE */
    Status = GetDerEncodedHeaderInfo (ChildData, ChildDataSize, &HeaderSize, &Tag, &ValueSize);
    if (EFI_ERROR (Status) || Tag != TAG_TYPE_VALUE_SEQUENCE) {
      return EFI_ABORTED;
    }
    GrandChildData      = ChildData + HeaderSize;
    GrandChilddDataSize = ValueSize;

    Status = GetDerEncodedHeaderInfo (GrandChildData, GrandChilddDataSize, &HeaderSize, &Tag, &ValueSize);
    if (EFI_ERROR (Status) || Tag != TAG_TYPE_VALUE_OID) {
      return EFI_ABORTED;
    }
    Oid     = GrandChildData + HeaderSize;
    OidSize = ValueSize;
    if (OidSize != sizeof (mSubjectCommonNameOID) ||
        CompareMem (Oid, mSubjectCommonNameOID, OidSize) != 0) {
      continue;
    }

    GrandChildData      += (HeaderSize + ValueSize);
    GrandChilddDataSize -= (HeaderSize + ValueSize);
    Status = GetDerEncodedHeaderInfo (GrandChildData, GrandChilddDataSize, &HeaderSize, &Tag, &ValueSize);
    if (EFI_ERROR (Status) || Tag != TAG_TYPE_VALUE_PRINTABLESTRING) {
      return EFI_ABORTED;
    }

    AsciiStr = (CHAR8 *) (GrandChildData + HeaderSize);
    Count    = MIN (ValueSize, (StrBufferSize / sizeof (CHAR16) - 1));
    for (Index = 0; Index < Count; Index++) {
      StrBuffer[Index] = (CHAR16) AsciiStr[Index];
    }
    StrBuffer[Index] = CHAR_NULL;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

/**
  Get the display string of PKCS7 signature buffer.
  Display string is determined by subject CN(Common Name).

  @param[in]  SignatureBuffer     Pointer to PKCS7 signature buffer
  @param[in]  SignatureBufferSize Size in bytes of PKCS7 signature buffer
  @param[out] StrBuffer           Pointer to output string buffer
  @param[in]  StrBufferSize       Size in bytes of output string buffer

  @retval EFI_SUCCESS             Get the display string successfully.
  @retval EFI_NOT_FOUND           Subject CN is not found.
  @retval EFI_OUT_OF_RESOURCES    Fail to allocate memory.
  @retval Other                   Fail to get CryptoServicesProtocol instance or get subject CN.
**/
STATIC
EFI_STATUS
GetPkcs7DisplayStr (
  IN  UINT8                            *SignatureBuffer,
  IN  UINTN                            SignatureBufferSize,
  OUT CHAR16                           *StrBuffer,
  IN  UINTN                            StrBufferSize
  )
{
  EFI_STATUS                           Status;
  CRYPTO_SERVICES_PROTOCOL             *CryptoService;
  UINT8                                *Subject;
  UINTN                                SubjectSize;
  BOOLEAN                              Success;

  Status = gBS->LocateProtocol (&gCryptoServicesProtocolGuid, NULL, (VOID **) &CryptoService);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Subject     = NULL;
  SubjectSize = 0;
  Success = CryptoService->X509GetSubjectName (
                             SignatureBuffer,
                             SignatureBufferSize,
                             Subject,
                             &SubjectSize
                             );
  if (Success || SubjectSize == 0) {
    return EFI_NOT_FOUND;
  }

  Subject = AllocatePool (SubjectSize);
  if (Subject == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Success = CryptoService->X509GetSubjectName (
                             SignatureBuffer,
                             SignatureBufferSize,
                             Subject,
                             &SubjectSize
                             );
  if (!Success) {
    FreePool (Subject);
    return EFI_NOT_FOUND;
  }

  Status = GetSubjectCommonName (Subject, SubjectSize, StrBuffer, StrBufferSize);
  FreePool (Subject);
  return Status;
}

/**
  Get the display string of signature buffer.

  @param[in]  SignatureType       Pointer to signature type GUID
  @param[in]  SignatureBuffer     Pointer to signature buffer
  @param[in]  SignatureBufferSize Size in bytes of signature buffer
  @param[out] StrBuffer           Pointer to output string buffer
  @param[in]  StrBufferSize       Size in bytes of output string buffer

  @retval EFI_SUCCESS             Get the display string successfully.
  @retval Other                   Fail to get PKCS7 display string or binary string.
**/
STATIC
EFI_STATUS
GetSignatureDisplayStr (
  IN  EFI_GUID                         *SignatureType,
  IN  UINT8                            *SignatureBuffer,
  IN  UINT32                           SignatureBufferSize,
  OUT CHAR16                           *StrBuffer,
  IN  UINTN                            StrBufferSize
  )
{
  if (CompareGuid (SignatureType, &gEfiCertX509Guid)) {
    return GetPkcs7DisplayStr (SignatureBuffer, SignatureBufferSize, StrBuffer, StrBufferSize);
  } else {
    return BinaryToString (SignatureBuffer, SignatureBufferSize, StrBuffer, StrBufferSize);
  }
}

/**
  Get the pointer of secure boot variable info by input secure boot manager state.

  @param[in] SecureBootMgrState      Secure boot manager state

  @return The pointer of secure boot variable info or NULL if secure boot manager state is invalid.
**/
STATIC
SECURE_BOOT_VAR_INFO *
GetSecureBootVarInfo (
  IN SECURE_BOOT_MANAGER_STATE         SecureBootMgrState
  )
{
  switch (SecureBootMgrState) {

  case PkForm:
    return &mPkVarInfo;

  case KekForm:
    return &mKekVarInfo;

  case HashImageForm:
  case DbForm:
    return &mDbVarInfo;

  case DbxForm:
    return &mDbxVarInfo;

  default:
    return NULL;
  }
}

/**
  Check to see if input secure boot variable is modified or not by comparing secure boot variable in NV storage.

  @param[in] VarName                 Pointer to secure boot variable name
  @param[in] VendorGuid              Pointer to secure boot variable vendor GUID
  @param[in] CurrVarData             Pointer to secure boot variable buffer
  @param[in] CurrVarDataSize         Size in bytes of secure boot variable buffer

  @retval TRUE   The input secure boot variable is modified.
  @retval FALSE  The input secure boot variable is not modified.
**/
STATIC
BOOLEAN
IsSecureBootVarModified (
  IN CHAR16                            *VarName,
  IN EFI_GUID                          *VendorGuid,
  IN UINT8                             *CurrVarData,
  IN UINTN                             CurrVarDataSize
  )
{
  EFI_STATUS                           Status;
  UINTN                                OrgVarDataSize;
  UINT8                                *OrgVarData;
  BOOLEAN                              IsModified;

  Status = CommonGetVariableDataAndSize (VarName, VendorGuid, &OrgVarDataSize, (VOID **) &OrgVarData);
  if (EFI_ERROR (Status)) {
    OrgVarData     = NULL;
    OrgVarDataSize = 0;
  }

  if ((OrgVarDataSize != CurrVarDataSize)         ||
      (OrgVarData == NULL && CurrVarData != NULL) ||
      (OrgVarData != NULL && CurrVarData == NULL) ||
      (CompareMem (OrgVarData, CurrVarData, CurrVarDataSize) != 0)) {
    IsModified = TRUE;
  } else {
    IsModified = FALSE;
  }
  SecureBootSafeFreePool ((VOID **) &OrgVarData);

  return IsModified;
}

/**
  Check to see if input signature is existing in input signature list or not.

  @param[in] SigList                 Pointer to signature list
  @param[in] Sig                     Pointer to signature buffer
  @param[in] SigSize                 Size in bytes of signature buffer

  @retval TRUE   The input signature is existing in input signature list.
  @retval FALSE  The input signature is not existing in input signature list.
**/
STATIC
BOOLEAN
IsSignatureExistence (
  IN EFI_SIGNATURE_LIST                *SigList,
  IN EFI_SIGNATURE_DATA                *Sig,
  IN UINTN                             SigSize
  )
{
  UINT8                                *CurrSig;
  UINTN                                SigCount;
  UINTN                                Index;

  if (SigList == NULL || Sig == NULL || SigSize == 0) {
    return FALSE;
  }
  if (SigSize != SigList->SignatureSize) {
    return FALSE;
  }

  CurrSig  = ((UINT8 *) SigList) + sizeof (EFI_SIGNATURE_LIST) + SigList->SignatureHeaderSize;
  SigCount = (SigList->SignatureListSize - sizeof (EFI_SIGNATURE_LIST) - SigList->SignatureHeaderSize) / SigList->SignatureSize;
  for (Index = 0; Index < SigCount; Index++) {
    if (CompareMem (CurrSig, Sig, SigSize) == 0) {
      return TRUE;
    }
    CurrSig += SigList->SignatureSize;
  }

  return FALSE;
}

/**
  Check to see if input signature is existing in input signature list or not.

  @param[in] SigList                 Pointer to signature list
  @param[in] Sig                     Pointer to signature buffer
  @param[in] SigSize                 Size in bytes of signature buffer

  @retval TRUE   The input signature is existing in input signature list.
  @retval FALSE  The input signature is not existing in input signature list.
**/
STATIC
BOOLEAN
IsShaSignatureType (
  IN EFI_GUID                          *SignatureType
  )
{
  if (CompareGuid (SignatureType, &gEfiCertSha1Guid  ) ||
      CompareGuid (SignatureType, &gEfiCertSha256Guid) ||
      CompareGuid (SignatureType, &gEfiCertSha512Guid)) {
    return TRUE;
  }

  return FALSE;
}

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
  )
{
  SECURE_BOOT_VAR_INFO                 *VarInfo;
  EFI_SIGNATURE_DATA                   *Sig;
  UINTN                                SigSize;
  UINTN                                RemainedSize;
  EFI_SIGNATURE_LIST                   *TargetSigList;
  EFI_SIGNATURE_LIST                   *SigList;
  EFI_SIGNATURE_LIST                   *NewSigList;
  UINT8                                *NewVarData;
  UINT8                                *NewVarDataPtr;
  UINTN                                NewVarDataSize;
  UINTN                                Size;

  if (SignatureType == NULL || SignatureOwnerGuid == NULL || SignatureData == NULL || SignatureDataSize == 0) {
    return EFI_INVALID_PARAMETER;
  }

  VarInfo = GetSecureBootVarInfo (SecureBootMgrState);
  if (VarInfo == NULL) {
    return EFI_NOT_FOUND;
  }

  if (VarInfo == &mPkVarInfo && VarInfo->VarData != NULL) {
    SecureBootShowOkConfirmDlg (STRING_TOKEN (STR_PK_EXIST_ERROR_MESSAGE));
    return EFI_ABORTED;
  }

  //
  // Create signature.
  //
  SigSize = sizeof (EFI_SIGNATURE_DATA) - 1 + SignatureDataSize;
  Sig     = AllocatePool (SigSize);
  if (Sig == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyGuid (&Sig->SignatureOwner, SignatureOwnerGuid);
  CopyMem (Sig->SignatureData, SignatureData, SignatureDataSize);

  RemainedSize  = VarInfo->VarDataSize;
  SigList       = (EFI_SIGNATURE_LIST *) VarInfo->VarData;
  TargetSigList = NULL;
  while ((RemainedSize > 0) && (RemainedSize >= SigList->SignatureListSize)) {
    if (SigList->SignatureListSize < sizeof (EFI_SIGNATURE_LIST)) {
      break;
    }

    if (SigList->SignatureSize == SigSize && CompareGuid (&SigList->SignatureType, SignatureType)) {
      if (IsSignatureExistence (SigList, Sig, SigSize)) {
        SecureBootShowOkConfirmDlg (STRING_TOKEN(STR_IDENTICAL_SIGNATURE_ERROR_MESSAGE));
        FreePool (Sig);
        return EFI_ABORTED;
      }

      if (IsShaSignatureType (SignatureType)) {
        TargetSigList = SigList;
        break;
      }
    }

    RemainedSize -= SigList->SignatureListSize;
    SigList = (EFI_SIGNATURE_LIST *) ((UINT8 *) SigList + SigList->SignatureListSize);
  }

  if (TargetSigList == NULL) {
    //
    // Target signature list doesn't exist. Append new signature list in the end of variable.
    //
    NewVarDataSize = VarInfo->VarDataSize + sizeof (EFI_SIGNATURE_LIST) + SigSize;
    NewVarData     = AllocatePool (NewVarDataSize);
    if (NewVarData == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    CopyMem (NewVarData, VarInfo->VarData, VarInfo->VarDataSize);

    NewSigList = (EFI_SIGNATURE_LIST *) (NewVarData + VarInfo->VarDataSize);
    NewSigList->SignatureListSize   = (UINT32) (sizeof (EFI_SIGNATURE_LIST) + SigSize);
    NewSigList->SignatureHeaderSize = 0;
    NewSigList->SignatureSize       = (UINT32) SigSize;
    CopyGuid (
      &NewSigList->SignatureType,
      SignatureType
      );
    CopyMem (
      ((UINT8 *) NewSigList) + sizeof (EFI_SIGNATURE_LIST),
      Sig,
      SigSize
      );
  } else {
    //
    // Target signature list exist. Append new signature in the end of target signature list.
    //
    NewVarDataSize = VarInfo->VarDataSize + SigSize;
    NewVarData     = AllocatePool (NewVarDataSize);
    if (NewVarData == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    NewVarDataPtr = NewVarData;
    RemainedSize  = VarInfo->VarDataSize;

    Size = ((UINT8 *) TargetSigList) - VarInfo->VarData;
    CopyMem (NewVarDataPtr, VarInfo->VarData, Size);
    NewVarDataPtr += Size;
    RemainedSize  -= Size;

    NewSigList = (EFI_SIGNATURE_LIST *) NewVarDataPtr;
    Size       = TargetSigList->SignatureListSize;
    CopyMem (NewVarDataPtr, TargetSigList, Size);
    NewVarDataPtr += Size;
    RemainedSize  -= Size;
    CopyMem (NewVarDataPtr, Sig, SigSize);
    NewVarDataPtr += SigSize;
    NewSigList->SignatureListSize += (UINT32) SigSize;

    CopyMem (
      NewVarDataPtr,
      ((UINT8 *) TargetSigList) + TargetSigList->SignatureListSize,
      RemainedSize
      );
  }

  SecureBootSafeFreePool ((VOID **) &VarInfo->VarData);
  VarInfo->VarData     = NewVarData;
  VarInfo->VarDataSize = NewVarDataSize;

  SecureBootVarUpdateForm (mSecureBootPrivate.SecureBootMgrState);

  FreePool (Sig);
  return EFI_SUCCESS;
}

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
  )
{
  SECURE_BOOT_VAR_INFO                 *VarInfo;
  UINTN                                Index;
  UINTN                                CurrIndex;
  UINTN                                RemainedSize;
  EFI_SIGNATURE_LIST                   *SigList;
  UINT8                                *Sig;
  UINTN                                SigCount;

  VarInfo = GetSecureBootVarInfo (SecureBootMgrState);
  if (VarInfo == NULL) {
    return EFI_NOT_FOUND;
  }

  CurrIndex    = 0;
  RemainedSize = VarInfo->VarDataSize;
  SigList      = (EFI_SIGNATURE_LIST *) VarInfo->VarData;
  while ((RemainedSize > 0) && (RemainedSize >= SigList->SignatureListSize)) {
    if (SigList->SignatureListSize < sizeof (EFI_SIGNATURE_LIST)) {
      break;
    }

    SigCount = (SigList->SignatureListSize - sizeof (EFI_SIGNATURE_LIST) - SigList->SignatureHeaderSize) / SigList->SignatureSize;
    if (SignatureIndex >= CurrIndex &&
        SignatureIndex  < CurrIndex + SigCount) {
      if (SigCount <= 1) {
        //
        // There is only one signature in the signature list. Delete this signature list.
        //
        VarInfo->VarDataSize -= SigList->SignatureListSize;
        CopyMem (
          SigList,
          ((UINT8 *) SigList) + SigList->SignatureListSize,
          RemainedSize - SigList->SignatureListSize
          );
      } else {
        //
        // There are many signatures in the signature list. Delete this signature in the signature list.
        //
        Sig   = ((UINT8 *) SigList) + sizeof (EFI_SIGNATURE_LIST) + SigList->SignatureHeaderSize;
        Index = SignatureIndex - CurrIndex;
        CopyMem (
          Sig + SigList->SignatureSize * Index,
          Sig + SigList->SignatureSize * (Index + 1),
          SigList->SignatureSize * (SigCount - Index - 1) + (RemainedSize - SigList->SignatureListSize)
          );
        SigList->SignatureListSize -= SigList->SignatureSize;
        VarInfo->VarDataSize       -= SigList->SignatureSize;
      }

      if (VarInfo->VarDataSize == 0) {
        SecureBootSafeFreePool ((VOID **) &VarInfo->VarData);
      }

      SecureBootVarUpdateForm (mSecureBootPrivate.SecureBootMgrState);
      return EFI_SUCCESS;
    }

    CurrIndex += SigCount;
    RemainedSize -= SigList->SignatureListSize;
    SigList = (EFI_SIGNATURE_LIST *) ((UINT8 *) SigList + SigList->SignatureListSize);
  }

  return EFI_NOT_FOUND;
}

/**
  Enumerating all signature from variable data to update signature question list in target VFR form.

  @param[in] VarData                 Pointer to secure boot variable data
  @param[in] VarDataSize             Size in bytes of secure boot variable data
  @param[in] FormId                  Target form ID
  @param[in] LabelStartId            Target start label ID
  @param[in] LabelEndId              Target end label ID
  @param[in] IfrOpcode               Target IFR opcode of signature
  @param[in] QuestionIdBase          Question ID base value if QuestionOpcode is a question opcode.
  @param[in] QuestionIdLen           Available question ID length if QuestionOpcode is a question opcode.

  @retval EFI_SUCCESS                Update signature question list successfully.
  @retval EFI_OUT_OF_RESOURCES       Fail to create opcode or allocate memory.
**/
STATIC
EFI_STATUS
SecureBootVarUpdateQuestionList (
  IN UINT8                             *VarData,
  IN UINTN                             VarDataSize,
  IN EFI_FORM_ID                       FormId,
  IN UINT16                            LabelStartId,
  IN UINT16                            LabelEndId,
  IN UINT8                             IfrOpcode,
  IN EFI_QUESTION_ID                   QuestionIdBase,
  IN EFI_QUESTION_ID                   QuestionIdLen
  )
{
  EFI_STATUS                           Status;
  EFI_HII_HANDLE                       HiiHandle;
  VOID                                 *StartOpCodeHandle;
  VOID                                 *EndOpCodeHandle;
  EFI_IFR_GUID_LABEL                   *StartLabel;
  EFI_IFR_GUID_LABEL                   *EndLabel;
  UINTN                                Index;
  CHAR16                               *SigTypeStr;
  CHAR16                               *SigDataStr;
  UINTN                                SigDataStrSize;
  CHAR16                               *PromptStr;
  UINTN                                PromptStrSize;
  EFI_STRING_ID                        PromptStrId;
  CHAR16                               *HelpStr;
  UINTN                                HelpStrSize;
  EFI_STRING_ID                        HelpStrId;
  CHAR16                               GuidStr[37];
  UINTN                                QuestionIdIndex;
  UINTN                                RemainedSize;
  EFI_SIGNATURE_LIST                   *SigList;
  UINTN                                SigCount;
  EFI_SIGNATURE_DATA                   *Sig;
  CHAR16                               UnknownStr[] = L"Unknown";

  HiiHandle  = mSecureBootPrivate.HiiHandle;
  Status     = EFI_SUCCESS;
  SigDataStr = NULL;
  PromptStr  = NULL;
  HelpStr    = NULL;

  //
  // Initialize the container for dynamic opcodes.
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  EndOpCodeHandle   = HiiAllocateOpCodeHandle ();
  if (StartOpCodeHandle == NULL || EndOpCodeHandle == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel   = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle  , &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  if (StartLabel == NULL || EndLabel == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = LabelStartId;
  EndLabel->ExtendOpCode   = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number         = LabelEndId;

  if (VarData == NULL || VarDataSize == 0) {
    goto Exit;
  }

  SigDataStrSize = 100 * sizeof (CHAR16);
  SigDataStr     = AllocateZeroPool (SigDataStrSize);
  PromptStrSize  = SigDataStrSize + 100 * sizeof (CHAR16);
  PromptStr      = AllocateZeroPool (PromptStrSize);
  HelpStrSize    = 100 * sizeof (CHAR16);
  HelpStr        = AllocateZeroPool (HelpStrSize);
  if (PromptStr == NULL || SigDataStr == NULL || HelpStr == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  QuestionIdIndex = 0;
  RemainedSize    = VarDataSize;
  SigList = (EFI_SIGNATURE_LIST *) VarData;

  while ((RemainedSize > 0) && (RemainedSize >= SigList->SignatureListSize)) {
    if (SigList->SignatureListSize < sizeof (EFI_SIGNATURE_LIST)) {
      break;
    }

    SigTypeStr = GetSigTypeStr (HiiHandle, &SigList->SignatureType);
    if (SigTypeStr == NULL) {
      SigTypeStr = UnknownStr;
    }

    Sig      = (EFI_SIGNATURE_DATA *) (((UINT8 *) SigList) + sizeof (EFI_SIGNATURE_LIST) + SigList->SignatureHeaderSize);
    SigCount = (SigList->SignatureListSize - sizeof (EFI_SIGNATURE_LIST) - SigList->SignatureHeaderSize) / SigList->SignatureSize;
    for (Index = 0; Index < SigCount; Index++, QuestionIdIndex++) {
      Status = GetSignatureDisplayStr (
                 &SigList->SignatureType,
                 Sig->SignatureData,
                 SigList->SignatureSize - OFFSET_OF(EFI_SIGNATURE_DATA, SignatureData),
                 SigDataStr,
                 SigDataStrSize
                 );
      if (EFI_ERROR (Status)) {
        ZeroMem (SigDataStr, SigDataStrSize);
      }

      UnicodeSPrint (PromptStr, PromptStrSize, L"%02d. [%s] %s", QuestionIdIndex + 1, SigTypeStr, SigDataStr);
      PromptStrId = SecureBootGetStrTokenFromDepository (&mSecureBootPrivate, mSecureBootSigOptionStrDepository);
      PromptStrId = HiiSetString (HiiHandle, PromptStrId, PromptStr, NULL);

      SecureBootGuidToStr (&Sig->SignatureOwner, GuidStr, sizeof(GuidStr));
      UnicodeSPrint (HelpStr, HelpStrSize, L"Owner_GUID: %s", GuidStr);
      HelpStrId = SecureBootGetStrTokenFromDepository (&mSecureBootPrivate, mSecureBootSigOptionStrDepository);
      HelpStrId = HiiSetString (HiiHandle, HelpStrId, HelpStr, NULL);

      switch (IfrOpcode) {

      case EFI_IFR_TEXT_OP:
        HiiCreateTextOpCode (
          StartOpCodeHandle,
          PromptStrId,
          HelpStrId,
          0
          );
        break;

      case EFI_IFR_ACTION_OP:
        if (QuestionIdIndex >= QuestionIdLen) {
          break;
        }
        HiiCreateActionOpCode (
          StartOpCodeHandle,
          (EFI_QUESTION_ID) (QuestionIdBase + QuestionIdIndex),
          PromptStrId,
          HelpStrId,
          EFI_IFR_FLAG_CALLBACK,
          0
          );
        break;
      }

      Sig = (EFI_SIGNATURE_DATA *) (((UINT8 *) Sig) + SigList->SignatureSize);
    }

    if (SigTypeStr != UnknownStr) {
      FreePool (SigTypeStr);
    }

    RemainedSize -= SigList->SignatureListSize;
    SigList = (EFI_SIGNATURE_LIST *) ((UINT8 *) SigList + SigList->SignatureListSize);
  }

Exit:
  if (StartOpCodeHandle != NULL && EndOpCodeHandle != NULL) {
    HiiUpdateForm (
      HiiHandle,
      &mSecureBootGuid,
      FormId,
      StartOpCodeHandle,
      EndOpCodeHandle
      );
  }

  if (StartOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle (StartOpCodeHandle);
  }
  if (EndOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle (EndOpCodeHandle);
  }
  SecureBootSafeFreePool ((VOID **) &SigDataStr);
  SecureBootSafeFreePool ((VOID **) &PromptStr);
  SecureBootSafeFreePool ((VOID **) &HelpStr);
  return Status;
}

/**
  Update the related forms of specified secure boot variable.

  @param[in] SecureBootMgrState      Secure boot manager state

  @retval EFI_SUCCESS                Update the related forms successfully.
  @retval EFI_NOT_FOUND              Fail to get secure boot vairable info or state is unsupported.
**/
EFI_STATUS
SecureBootVarUpdateForm (
  IN SECURE_BOOT_MANAGER_STATE         SecureBootMgrState
  )
{
  SECURE_BOOT_VAR_INFO                 *VarInfo;
  EFI_FORM_ID                          SigListFormId;
  EFI_FORM_ID                          DeleteSigFormId;

  VarInfo = GetSecureBootVarInfo (SecureBootMgrState);
  if (VarInfo == NULL) {
    return EFI_NOT_FOUND;
  }

  switch (SecureBootMgrState) {

  case PkForm:
    SigListFormId   = FORM_ID_PK;
    DeleteSigFormId = FORM_ID_PK_DELETE_SIGNATURE;
    break;

  case KekForm:
    SigListFormId   = FORM_ID_KEK;
    DeleteSigFormId = FORM_ID_KEK_DELETE_SIGNATURE;
    break;

  case DbForm:
    SigListFormId   = FORM_ID_DB;
    DeleteSigFormId = FORM_ID_DB_DELETE_SIGNATURE;
    break;

  case DbxForm:
    SigListFormId   = FORM_ID_DBX;
    DeleteSigFormId = FORM_ID_DBX_DELETE_SIGNATURE;
    break;

  default:
    return EFI_NOT_FOUND;
  }

  SecureBootReclaimStrDepository (mSecureBootSigOptionStrDepository);

  if (SigListFormId != 0) {
    SecureBootVarUpdateQuestionList (
      VarInfo->VarData,
      VarInfo->VarDataSize,
      SigListFormId,
      LABEL_ID_SIGNATURE_LIST_START,
      LABEL_ID_SIGNATURE_LIST_END,
      EFI_IFR_TEXT_OP,
      0,
      0
      );
  }

  if (DeleteSigFormId != 0) {
    SecureBootVarUpdateQuestionList (
      VarInfo->VarData,
      VarInfo->VarDataSize,
      DeleteSigFormId,
      LABEL_ID_DELETE_SIGNATURE_LIST_START,
      LABEL_ID_DELETE_SIGNATURE_LIST_END,
      EFI_IFR_ACTION_OP,
      KEY_DELETE_SIGNATURE_QUESTION_BASE,
      KEY_DELETE_SIGNATURE_QUESTION_LENGTH
      );
  }

  return EFI_SUCCESS;
}

/**
  Update each secure boot variable if it is modified.

  @retval EFI_SUCCESS          Update secure boot variables successfully.
**/
EFI_STATUS
SecureBootVarSaveChange (
  VOID
  )
{
  if (IsSecureBootVarModified (EFI_PLATFORM_KEY_NAME, &gEfiGlobalVariableGuid, mPkVarInfo.VarData, mPkVarInfo.VarDataSize)) {
    UpdatePkVariable (mPkVarInfo.VarData, mPkVarInfo.VarDataSize);
  }

  if (IsSecureBootVarModified (EFI_KEY_EXCHANGE_KEY_NAME, &gEfiGlobalVariableGuid, mKekVarInfo.VarData, mKekVarInfo.VarDataSize)) {
    UpdateKekVariable (mKekVarInfo.VarData, mKekVarInfo.VarDataSize, UPDATE_AUTHENTICATED_VARIABLE);
  }

  if (IsSecureBootVarModified (EFI_IMAGE_SECURITY_DATABASE, &gEfiImageSecurityDatabaseGuid, mDbVarInfo.VarData, mDbVarInfo.VarDataSize)) {
    UpdateDbVariable (mDbVarInfo.VarData, mDbVarInfo.VarDataSize, UPDATE_AUTHENTICATED_VARIABLE);
  }

  if (IsSecureBootVarModified (EFI_IMAGE_SECURITY_DATABASE1, &gEfiImageSecurityDatabaseGuid, mDbxVarInfo.VarData, mDbxVarInfo.VarDataSize)) {
    UpdateDbxVariable (mDbxVarInfo.VarData, mDbxVarInfo.VarDataSize, UPDATE_AUTHENTICATED_VARIABLE);
  }

  return EFI_SUCCESS;
}

/**
  Initialize secure boot variable info.

  @retval EFI_SUCCESS          Initialize secure boot variable info successfully.
**/
EFI_STATUS
SecureBootVarInit (
  VOID
  )
{
  CommonGetVariableDataAndSize (EFI_PLATFORM_KEY_NAME       , &gEfiGlobalVariableGuid       , &mPkVarInfo.VarDataSize , (VOID **) &mPkVarInfo.VarData );
  CommonGetVariableDataAndSize (EFI_KEY_EXCHANGE_KEY_NAME   , &gEfiGlobalVariableGuid       , &mKekVarInfo.VarDataSize, (VOID **) &mKekVarInfo.VarData);
  CommonGetVariableDataAndSize (EFI_IMAGE_SECURITY_DATABASE , &gEfiImageSecurityDatabaseGuid, &mDbVarInfo.VarDataSize , (VOID **) &mDbVarInfo.VarData );
  CommonGetVariableDataAndSize (EFI_IMAGE_SECURITY_DATABASE1, &gEfiImageSecurityDatabaseGuid, &mDbxVarInfo.VarDataSize, (VOID **) &mDbxVarInfo.VarData);
  return EFI_SUCCESS;
}

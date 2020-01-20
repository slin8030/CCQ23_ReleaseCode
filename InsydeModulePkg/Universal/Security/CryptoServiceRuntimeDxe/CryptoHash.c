/** @file
  Crypto Hash protocol implement

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "CryptoHash.h"
#include <PiSmm.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Protocol/CryptoServices.h>

#define HASH_BLOCK_SIZE  64
extern EFI_RUNTIME_SERVICES         *gRT;

CRYPTO_SELECTOR
mCryptoSelector [CRYPTO_ALGORITHM_MAX] = {
  { &gEfiHashAlgorithmSha1Guid,        FALSE, sizeof (EFI_SHA1_HASH),   Sha1GetContextSize,        Sha1Init,        Sha1Duplicate,        Sha1Update,        Sha1Final        },  // SHA 1
  { &gEfiHashAlgorithmSha224Guid,      FALSE, sizeof (EFI_SHA224_HASH), Sha224GetContextSize,      Sha224Init,      Sha224Duplicate,      Sha224Update,      Sha224Final      },  // SHA 224
  { &gEfiHashAlgorithmSha256Guid,      FALSE, sizeof (EFI_SHA256_HASH), Sha256GetContextSize,      Sha256Init,      Sha256Duplicate,      Sha256Update,      Sha256Final      },  // SHA 256
  { &gEfiHashAlgorithmSha384Guid,      FALSE, sizeof (EFI_SHA384_HASH), Sha384GetContextSize,      Sha384Init,      Sha384Duplicate,      Sha384Update,      Sha384Final      },  // SHA 384
  { &gEfiHashAlgorithmSha512Guid,      FALSE, sizeof (EFI_SHA512_HASH), Sha512GetContextSize,      Sha512Init,      Sha512Duplicate,      Sha512Update,      Sha512Final      },  // SHA 512
  { &gEfiHashAlgorithmMD5Guid,         FALSE, sizeof (EFI_MD5_HASH),    Md5GetContextSize,         Md5Init,         Md5Duplicate,         Md5Update,         Md5Final         }   // MD5
  };

EFI_SERVICE_BINDING_PROTOCOL
mHashServiceBindingProtocol = {
  CryptoHashCreateChild,
  CryptoHashDestroyChild
};

EFI_SERVICE_BINDING_PROTOCOL
mHash2ServiceBindingProtocol = {
  CryptoHash2CreateChild,
  CryptoHash2DestroyChild
};

EFI_HASH2_PROTOCOL mHash2Protocol = {
  BaseCrypto2GetHashSize,
  BaseCrypto2Hash,
  BaseCrypto2HashInit,
  BaseCrypto2HashUpdate,
  BaseCrypto2HashFinal,
};


EFI_STATUS
EFIAPI
RuntimeCryptoHashCreateChild (
  IN     EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN OUT EFI_HANDLE                    *ChildHandle
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
RuntimeCryptoHashDestroyChild (
  IN EFI_SERVICE_BINDING_PROTOCOL      *This,
  IN EFI_HANDLE                        ChildHandle
  )
{
  return EFI_UNSUPPORTED;
}

/*
  Service Binding protocol, create child function.

  @param This                            Indicate the instance condition.
  @param ChildHandle                     Indicate the handle of the child to create.
                                         If handle is NULL, function will create a new and install protocol.

  @retval EFI_SUCCESS                    Protocol create success.
  @retval EFI_INVALID_PARAMETER          ChildHandle is NULL.
                                         This is not belong this driver.
  @retval EFI_UNSUPPORTED                The protocol was not belong this driver or the protocol is root create.
  @retval other                          Error status from UninstallProtocolInterface.

*/
EFI_STATUS
EFIAPI
CryptoHashCreateChild (
  IN     EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN OUT EFI_HANDLE                    *ChildHandle
  )
{
  CRYPTO_HASH_INSTANCE                 *Instance;

  if ((This->CreateChild != CryptoHashCreateChild) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = RTAllocatePool (sizeof (CRYPTO_HASH_INSTANCE));
  if (Instance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  ZeroMem (Instance, sizeof (CRYPTO_HASH_INSTANCE));

  Instance->Signature = CRYPTO_HASH_SIGNATURE;
  Instance->HashProtocol.Hash        = CryptHash;
  Instance->HashProtocol.GetHashSize = CryptGetHashSize;
  return gBS->InstallProtocolInterface (
                ChildHandle,
                &gEfiHashProtocolGuid,
                EFI_NATIVE_INTERFACE,
                (VOID *) &(Instance->HashProtocol)
                );
}

/*
  Service Binding protocol, Destroy child function.

  @param This                            Indicate the instance condition.
  @param ChildHandle                     The Handle has the Hash protocol and create by CreateChild.

  @retval EFI_SUCCESS                    Protocol destroy success.
  @retval EFI_INVALID_PARAMETER          ChildHandle is NULL.
  @retval EFI_UNSUPPORTED                The protocol was not belong this driver or the protocol is root create.
  @retval other                          Error status from UninstallProtocolInterface.

*/
EFI_STATUS
EFIAPI
CryptoHashDestroyChild (
  IN EFI_SERVICE_BINDING_PROTOCOL      *This,
  IN EFI_HANDLE                        ChildHandle
  )
{
  EFI_STATUS                           Status;
  EFI_HASH_PROTOCOL                    *Hash;
  CRYPTO_HASH_INSTANCE                 *Instance;
  UINTN                                Index;

  if ((This->DestroyChild != CryptoHashDestroyChild) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->HandleProtocol (
                  ChildHandle,
                  &gEfiHashProtocolGuid,
                  (VOID *) &Hash
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Instance = CRYPTO_HASH_INSTANCE_FROM_THIS (Hash);
  if ((Instance->Signature == CRYPTO_HASH_SIGNATURE) && (Instance->IsRoot)) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->UninstallProtocolInterface (
                  ChildHandle,
                  &gEfiHashProtocolGuid,
                  (VOID *) Hash
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < CRYPTO_ALGORITHM_MAX; Index++) {
    if (Instance->Context[Index] != NULL) {
      RTFreePool (Instance->Context[Index]);
    }
  }

  RTFreePool (Instance);
  return EFI_SUCCESS;
}


/*
  Service Binding protocol, create child function.

  @param This                            Indicate the instance condition.
  @param ChildHandle                     Indicate the handle of the child to create.
                                         If handle is NULL, function will create a new and install protocol.

  @retval EFI_SUCCESS                    Protocol create success.
  @retval EFI_INVALID_PARAMETER          ChildHandle is NULL.
                                         This is not belong this driver.
  @retval EFI_UNSUPPORTED                The protocol was not belong this driver or the protocol is root create.
  @retval other                          Error status from UninstallProtocolInterface.

*/
EFI_STATUS
EFIAPI
CryptoHash2CreateChild (
  IN     EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN OUT EFI_HANDLE                    *ChildHandle
  )
{
  EFI_STATUS          Status;
  HASH2_INSTANCE_DATA *Instance;

  if ((This == NULL) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Allocate buffer for the new instance.
  //
  Instance = RTAllocatePool (sizeof (HASH2_INSTANCE_DATA));
  if (Instance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  ZeroMem (Instance, sizeof (HASH2_INSTANCE_DATA));

  //
  // Init the instance data.
  //
  Instance->Signature = HASH2_INSTANCE_DATA_SIGNATURE;
  CopyMem (&Instance->Hash2Protocol, &mHash2Protocol, sizeof (Instance->Hash2Protocol));

  Status = gBS->InstallMultipleProtocolInterfaces (
                  ChildHandle,
                  &gEfiHash2ProtocolGuid,
                  &Instance->Hash2Protocol,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    RTFreePool (Instance);
    return Status;
  }

  Instance->Handle = *ChildHandle;

  return Status;
}

/*
  Service Binding protocol, Destroy child function.

  @param This                            Indicate the instance condition.
  @param ChildHandle                     The Handle has the Hash protocol and create by CreateChild.

  @retval EFI_SUCCESS                    Protocol destroy success.
  @retval EFI_INVALID_PARAMETER          ChildHandle is NULL.
  @retval EFI_UNSUPPORTED                The protocol was not belong this driver or the protocol is root create.
  @retval other                          Error status from UninstallProtocolInterface.

*/
EFI_STATUS
EFIAPI
CryptoHash2DestroyChild (
  IN EFI_SERVICE_BINDING_PROTOCOL      *This,
  IN EFI_HANDLE                        ChildHandle
  )
{
  EFI_STATUS                     Status;
  EFI_HASH2_PROTOCOL             *Hash2Protocol;
  HASH2_INSTANCE_DATA            *Instance;

  if ((This == NULL) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get HashProtocol
  //
  Status = gBS->HandleProtocol (
                  ChildHandle,
                  &gEfiHash2ProtocolGuid,
                  (VOID **)&Hash2Protocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Instance = HASH2_INSTANCE_DATA_FROM_THIS (Hash2Protocol);
  if (Instance->Signature == HASH2_INSTANCE_DATA_SIGNATURE && Instance->IsRoot) {
    return EFI_UNSUPPORTED;
  }
  //
  // Uninstall the Hash protocol.
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  ChildHandle,
                  &gEfiHash2ProtocolGuid,
                  &Instance->Hash2Protocol,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (Instance->HashContext != NULL) {
    RTFreePool (Instance->HashContext);
  }
  RTFreePool (Instance);

  return EFI_SUCCESS;
}

/*
  Change address to virtual memory address
*/
VOID
CryptHashVirtualAddressChange (
  )
{
  UINTN    Index;

  //
  // Modify the ServiceBinding protocol function for fake function, to avoid it was called in runtime.
  //
  mHashServiceBindingProtocol.CreateChild  = RuntimeCryptoHashCreateChild;
  mHashServiceBindingProtocol.DestroyChild = RuntimeCryptoHashDestroyChild;
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(mHashServiceBindingProtocol.CreateChild));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(mHashServiceBindingProtocol.DestroyChild));

  for (Index = 0; Index < CRYPTO_ALGORITHM_MAX; Index++) {
    gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(mCryptoSelector [Index].Index));
    gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(mCryptoSelector [Index].GetContextSize));
    gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(mCryptoSelector [Index].Init));
    gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(mCryptoSelector [Index].Duplicate));
    gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(mCryptoSelector [Index].Update));
    gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(mCryptoSelector [Index].Final));
  }

  mHash2ServiceBindingProtocol.CreateChild  = RuntimeCryptoHashCreateChild;
  mHash2ServiceBindingProtocol.DestroyChild = RuntimeCryptoHashDestroyChild;
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(mHash2ServiceBindingProtocol.CreateChild));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(mHash2ServiceBindingProtocol.DestroyChild));
}

/*
  Compares two GUIDs

  @param Guid1                        guid to compare
  @param Guid2                        guid to compare

  @retval TRUE                        Guid is the same
  @retval FALSE                       Guid if different
*/
BOOLEAN
CryptCompareGuid (
  IN CONST EFI_GUID *Guid1,
  IN CONST EFI_GUID *Guid2
  )
{
  UINTN Index;

  //
  // Compare byte by byte
  //
  for (Index = 0; Index < 16; ++Index) {
    if (*(((UINT8*) Guid1) + Index) != *(((UINT8*) Guid2) + Index)) {
      return FALSE;
    }
  }
  return TRUE;
}

/*
  Search Crypto algorithm by EFI 2.3.1 definaion GUID

  @parma  GuidKey                    The Crypto algorithm chose
  @parma  Index                      The Algorithm array index

  @retval EFI_SUCCESS                Crypto has found
  @retval EFI_NOT_FOUND              Crypto algorithm not found or not support
*/
EFI_STATUS
CryptGetIndexByGuid (
  IN   CONST EFI_GUID    *GuidKey,
  OUT  UINTN             *Index
  )
{
  UINTN    LoopIndex;
  *Index = 0;
  for (LoopIndex = CRYPTO_ALGORITHM_NO_PAD_SHA1; LoopIndex < CRYPTO_ALGORITHM_MAX; LoopIndex++) {
    if (CryptCompareGuid ( mCryptoSelector[LoopIndex].Index, GuidKey)) {
      *Index = LoopIndex;
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}

/*
  Returns the size of the hash which results from a specific algorithm.

  @parma  HashAlgorithm              Points to the EFI_GUID which identifies the algorithm to use.
  @parma  HashSize                   Holds the returned size of the algorithm¡¯s hash.
  @parma  Index                      Index of Algorithm selector

  @retval EFI_SUCCESS                Hash size returned successfully.
  @retval EFI_NOT_FOUND              Algorithm not found
  @retval EFI_INVALID_PARAMETER      HashSize is NULL
  @retval EFI_UNSUPPORTED            The algorithm specified by HashAlgorithm is not supported
                                     by this driver.
--*/
EFI_STATUS
CryptGetHashSizeIndex (
  IN  CONST EFI_GUID              *HashAlgorithm,
  OUT UINTN                       *HashSize,
  OUT UINTN                       *Index  OPTIONAL
  )
{
  EFI_STATUS   Status;
  UINTN        SelectorIndex;

  Status = CryptGetIndexByGuid ( (EFI_GUID *)HashAlgorithm, &SelectorIndex);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  if (mCryptoSelector[SelectorIndex].GetContextSize == NULL) {
    return EFI_UNSUPPORTED;
  }

  if (Index != NULL) {
    *Index = SelectorIndex;
  }
  *HashSize = mCryptoSelector[SelectorIndex].GetContextSize ();

  return EFI_SUCCESS;
}

/*++
  Returns the size of the hash which results from a specific algorithm.

  @parma  This                       Points to this instance of EFI_HASH_PROTOCOL.
  @parma  HashAlgorithm              Points to the EFI_GUID which identifies the algorithm to use.
  @parma  HashSize                   Holds the returned size of the algorithm¡¯s hash.

  @retval EFI_SUCCESS                Hash size returned successfully.
  @retval EFI_NOT_FOUND              Algorithm not found
  @retval EFI_INVALID_PARAMETER      HashSize is NULL
  @retval EFI_UNSUPPORTED            The algorithm specified by HashAlgorithm is not supported
                                     by this driver.
--*/
EFI_STATUS
EFIAPI
CryptGetHashSize (
  IN  CONST EFI_HASH_PROTOCOL     *This,
  IN  CONST EFI_GUID              *HashAlgorithm,
  OUT UINTN                       *HashSize
  )
{
  EFI_STATUS   Status;
  UINTN        AlgoIndex;

  if ((This == NULL) || (HashSize == NULL) || (HashAlgorithm == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = CryptGetIndexByGuid ( (EFI_GUID *) HashAlgorithm, &AlgoIndex);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  *HashSize = mCryptoSelector[AlgoIndex].HashSize;
  return EFI_SUCCESS;
}

/*++
  Returns the size of the hash which results from a specific algorithm.

  @parma  This                       Points to this instance of EFI_HASH_PROTOCOL.
  @parma  HashAlgorithm              Points to the EFI_GUID which identifies the algorithm to use.
  @parma  Extend                     Specifies whether to create a new hash (FALSE) or extend the specified
                                     existing hash (TRUE).
  @parma  Message                    Points to the start of the message.
  @parma  MessageSize                The size of Message, in bytes.
  @parma  Hash                       On input, if Extend is TRUE, then this holds the hash to extend. On
                                     output, holds the resulting hash computed from the message.

  @retval EFI_SUCCESS                Hash returned successfully.
  @retval EFI_INVALID_PARAMETER      Message or Hash is NULL
  @retval EFI_UNSUPPORTED            The algorithm specified by HashAlgorithm is not supported by this
                                     driver. Or extend is TRUE and the algorithm doesn¡¯t support extending the hash.
--*/
EFI_STATUS
EFIAPI
CryptHash (
  IN CONST EFI_HASH_PROTOCOL      *This,
  IN CONST EFI_GUID               *HashAlgorithm,
  IN BOOLEAN                      Extend,
  IN CONST UINT8                  *Message,
  IN UINT64                       MessageSize,
  IN OUT EFI_HASH_OUTPUT          *Hash
  )
{
  EFI_STATUS               Status;
  CRYPTO_HASH_INSTANCE     *Instance;
  UINTN                    CtxSize;
  UINTN                    Index;
  CRYPTO_SELECTOR          *CryptoHash;
  VOID                     *HashCtx;
  VOID                     *ResultHash;


  if (This == NULL || Hash == NULL || Hash->Sha256Hash == NULL || Message == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (HashAlgorithm == NULL) {
    return EFI_UNSUPPORTED;
  }

  Instance = CRYPTO_HASH_INSTANCE_FROM_THIS (This);

  //
  // Get Hash result size and hash context size, and detect the function capability.
  //
  Status = CryptGetHashSizeIndex (HashAlgorithm, &CtxSize, &Index);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  if (mCryptoSelector[Index].NoPad && MessageSize % HASH_BLOCK_SIZE != 0) {
    return EFI_INVALID_PARAMETER;
  }

  ResultHash = RTAllocatePool (CtxSize);
  if (ResultHash == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Check HASH context, if Extend then initialize it.
  //
  CryptoHash = &(mCryptoSelector[Index]);
  HashCtx    = Instance->Context[Index];
  if (HashCtx == NULL) {
    if (Extend) {
      Status = EFI_UNSUPPORTED;
      goto Done;
    }
    HashCtx = RTAllocatePool (CtxSize);
    if (HashCtx == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }
  }

  Status = EFI_OUT_OF_RESOURCES;
  //
  // If Extend is FALSE, initial the Hash context for new HASH.
  //
  if (!Extend) {
    if (!CryptoHash->Init (HashCtx)) {
      goto Done;
    }
  }

  if (!CryptoHash->Update (HashCtx, Message, (UINTN)MessageSize)) {
    goto Done;
  }
  if (!CryptoHash->Duplicate (HashCtx, ResultHash)) {
    goto Done;
  }
  if (!CryptoHash->Final (ResultHash, (UINT8*)(UINTN)Hash->Sha256Hash)) {
    goto Done;
  }
  Instance->Context[Index] = HashCtx;
  Status = EFI_SUCCESS;

Done:
  RTFreePool (ResultHash);
  return Status;
}

/**
  Returns the size of the hash which results from a specific algorithm.

  @param[in]  This                  Points to this instance of EFI_HASH2_PROTOCOL.
  @param[in]  HashAlgorithm         Points to the EFI_GUID which identifies the algorithm to use.
  @param[out] HashSize              Holds the returned size of the algorithm's hash.

  @retval EFI_SUCCESS           Hash size returned successfully.
  @retval EFI_INVALID_PARAMETER This or HashSize is NULL.
  @retval EFI_UNSUPPORTED       The algorithm specified by HashAlgorithm is not supported by this driver
                                or HashAlgorithm is null.

**/
EFI_STATUS
EFIAPI
BaseCrypto2GetHashSize (
  IN  CONST EFI_HASH2_PROTOCOL     *This,
  IN  CONST EFI_GUID              *HashAlgorithm,
  OUT UINTN                       *HashSize
  )
{
  EFI_STATUS   Status;
  UINTN        AlgoIndex;

  if ((This == NULL) || (HashSize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (HashAlgorithm == NULL) {
    return EFI_UNSUPPORTED;
  }

  Status = CryptGetIndexByGuid ( (EFI_GUID *)HashAlgorithm, &AlgoIndex);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  *HashSize = mCryptoSelector[AlgoIndex].HashSize;
  return EFI_SUCCESS;
}

/**
  Creates a hash for the specified message text. The hash is not extendable.
  The output is final with any algorithm-required padding added by the function.

  @param[in]  This          Points to this instance of EFI_HASH2_PROTOCOL.
  @param[in]  HashAlgorithm Points to the EFI_GUID which identifies the algorithm to use.
  @param[in]  Message       Points to the start of the message.
  @param[in]  MessageSize   The size of Message, in bytes.
  @param[in,out]  Hash      On input, points to a caller-allocated buffer of the size
                              returned by GetHashSize() for the specified HashAlgorithm.
                            On output, the buffer holds the resulting hash computed from the message.

  @retval EFI_SUCCESS           Hash returned successfully.
  @retval EFI_INVALID_PARAMETER This or Hash is NULL.
  @retval EFI_UNSUPPORTED       The algorithm specified by HashAlgorithm is not supported by this driver
                                or HashAlgorithm is Null.
  @retval EFI_OUT_OF_RESOURCES  Some resource required by the function is not available
                                or MessageSize is greater than platform maximum.

**/
EFI_STATUS
EFIAPI
BaseCrypto2Hash (
  IN CONST EFI_HASH2_PROTOCOL      *This,
  IN CONST EFI_GUID                *HashAlgorithm,
  IN CONST UINT8                   *Message,
  IN UINTN                         MessageSize,
  IN OUT EFI_HASH2_OUTPUT          *Hash
  )
{
  VOID                     *HashCtx;
  UINTN                    CtxSize;
  EFI_STATUS               Status;
  UINTN                    Index;
  CRYPTO_SELECTOR          *CryptoHash;
  HASH2_INSTANCE_DATA      *Instance;


  if (This == NULL || Hash == NULL || Message == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (HashAlgorithm == NULL) {
    return EFI_UNSUPPORTED;
  }


  Status = CryptGetHashSizeIndex (HashAlgorithm, &CtxSize, &Index);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Instance = HASH2_INSTANCE_DATA_FROM_THIS(This);
  if (Instance->HashContext != NULL) {
    RTFreePool (Instance->HashContext);
  }
  Instance->HashContext = NULL;

  if (mCryptoSelector[Index].NoPad && MessageSize % HASH_BLOCK_SIZE != 0) {
    return EFI_INVALID_PARAMETER;
  }

  HashCtx = RTAllocatePool (CtxSize);
  if (HashCtx == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = EFI_OUT_OF_RESOURCES;
  CryptoHash = &(mCryptoSelector[Index]);
  if (!CryptoHash->Init (HashCtx)) {
    goto Done;
  }
  if (!CryptoHash->Update (HashCtx, Message, (UINTN)MessageSize)) {
    goto Done;
  }
  if (!CryptoHash->Final (HashCtx, (UINT8*)(UINTN)Hash->Sha256Hash)) {
    goto Done;
  }
  Status = EFI_SUCCESS;

Done:
  RTFreePool (HashCtx);
  return Status;
}

/**
  This function must be called to initialize a digest calculation to be subsequently performed using the
  EFI_HASH2_PROTOCOL functions HashUpdate() and HashFinal().

  @param[in]  This          Points to this instance of EFI_HASH2_PROTOCOL.
  @param[in]  HashAlgorithm Points to the EFI_GUID which identifies the algorithm to use.

  @retval EFI_SUCCESS           Initialized successfully.
  @retval EFI_INVALID_PARAMETER This is NULL.
  @retval EFI_UNSUPPORTED       The algorithm specified by HashAlgorithm is not supported by this driver
                                or HashAlgorithm is Null.
  @retval EFI_OUT_OF_RESOURCES  Process failed due to lack of required resource.
  @retval EFI_ALREADY_STARTED   This function is called when the operation in progress is still in processing Hash(),
                                or HashInit() is already called before and not terminated by HashFinal() yet on the same instance.

**/
EFI_STATUS
EFIAPI
BaseCrypto2HashInit (
  IN CONST EFI_HASH2_PROTOCOL      *This,
  IN CONST EFI_GUID                *HashAlgorithm
  )
{
  EFI_STATUS               Status;
  VOID                     *HashCtx;
  UINTN                    CtxSize;
  HASH2_INSTANCE_DATA      *Instance;
  UINTN                    Index;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (HashAlgorithm == NULL) {
    return EFI_UNSUPPORTED;
  }

  Status = CryptGetHashSizeIndex (HashAlgorithm, &CtxSize, &Index);
  if (Status != EFI_SUCCESS) {
    return EFI_UNSUPPORTED;
  }
  //
  // Consistency Check
  //
  Instance = HASH2_INSTANCE_DATA_FROM_THIS(This);
  if (Instance->HashContext != NULL) {
    return EFI_ALREADY_STARTED;
  }

  HashCtx = RTAllocatePool (CtxSize);
  if (HashCtx == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (!mCryptoSelector[Index].Init (HashCtx)) {
    RTFreePool (HashCtx);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Setup the context
  //
  Instance->HashContext = HashCtx;
  Instance->AlgorithmIndex = Index;

  return EFI_SUCCESS;
}

/**
  Updates the hash of a computation in progress by adding a message text.

  @param[in]  This          Points to this instance of EFI_HASH2_PROTOCOL.
  @param[in]  Message       Points to the start of the message.
  @param[in]  MessageSize   The size of Message, in bytes.

  @retval EFI_SUCCESS           Digest in progress updated successfully.
  @retval EFI_INVALID_PARAMETER This or Hash is NULL.
  @retval EFI_OUT_OF_RESOURCES  Some resource required by the function is not available
                                or MessageSize is greater than platform maximum.
  @retval EFI_NOT_READY         This call was not preceded by a valid call to HashInit(),
                                or the operation in progress was terminated by a call to Hash() or HashFinal() on the same instance.

**/
EFI_STATUS
EFIAPI
BaseCrypto2HashUpdate (
  IN CONST EFI_HASH2_PROTOCOL      *This,
  IN CONST UINT8                   *Message,
  IN UINTN                         MessageSize
  )
{
  VOID                     *HashCtx;
  HASH2_INSTANCE_DATA      *Instance;
  UINTN                    Index;

  if (This == NULL || Message == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Consistency Check
  //
  Instance = HASH2_INSTANCE_DATA_FROM_THIS(This);
  if (Instance->HashContext == NULL) {
    return EFI_NOT_READY;
  }
  Index = Instance->AlgorithmIndex;
  HashCtx  = Instance->HashContext;

  if (!mCryptoSelector[Index].Update (HashCtx, Message, MessageSize)) {
    return EFI_OUT_OF_RESOURCES;
  }
  Instance->Updated = TRUE;
  return EFI_SUCCESS;
}

/**
  Finalizes a hash operation in progress and returns calculation result.
  The output is final with any necessary padding added by the function.
  The hash may not be further updated or extended after HashFinal().

  @param[in]  This          Points to this instance of EFI_HASH2_PROTOCOL.
  @param[in,out]  Hash      On input, points to a caller-allocated buffer of the size
                              returned by GetHashSize() for the specified HashAlgorithm specified in preceding HashInit().
                            On output, the buffer holds the resulting hash computed from the message.

  @retval EFI_SUCCESS           Hash returned successfully.
  @retval EFI_INVALID_PARAMETER This or Hash is NULL.
  @retval EFI_NOT_READY         This call was not preceded by a valid call to HashInit() and at least one call to HashUpdate(),
                                or the operation in progress was canceled by a call to Hash() on the same instance.

**/
EFI_STATUS
EFIAPI
BaseCrypto2HashFinal (
  IN CONST EFI_HASH2_PROTOCOL      *This,
  IN OUT EFI_HASH2_OUTPUT          *Hash
  )
{
  VOID                     *HashCtx;
  HASH2_INSTANCE_DATA      *Instance;
  BOOLEAN                  Ret;
  UINTN                    Index;

  if ((This == NULL) || (Hash == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Consistency Check
  //
  Instance = HASH2_INSTANCE_DATA_FROM_THIS(This);
  if (Instance->HashContext == NULL || !Instance->Updated) {
    return EFI_NOT_READY;
  }
  Index = Instance->AlgorithmIndex;
  HashCtx  = Instance->HashContext;

  Ret = mCryptoSelector[Index].Final (HashCtx, (UINT8 *)Hash->Sha1Hash);

  //
  // Cleanup the context
  //
  RTFreePool (HashCtx);
  Instance->HashContext = NULL;
  Instance->Updated     = FALSE;
  if (!Ret) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}
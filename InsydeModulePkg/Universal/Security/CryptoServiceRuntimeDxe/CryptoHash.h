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

#ifndef __EFI_CRYPTO_HASH_H__
#define __EFI_CRYPTO_HASH_H__

#include <Protocol/CryptoServices.h>
#include <Protocol/Hash.h>
#include <Protocol/Hash2.h>
#include <Protocol/ServiceBinding.h>
#include <Library/DebugLib.h>

#define CRYPTO_HASH_SIGNATURE          SIGNATURE_32 ('C', 'R', 'H','A')
#define HASH2_SERVICE_DATA_SIGNATURE   SIGNATURE_32 ('H', 'S', '2', 'S')
#define HASH2_INSTANCE_DATA_SIGNATURE  SIGNATURE_32 ('H', 's', '2', 'I')
typedef
UINTN
(EFIAPI *CRYPTO_GET_CONTEXT_SIZE) (
  VOID
  );

typedef
BOOLEAN
(EFIAPI *CRYPTO_INIT) (
  OUT  VOID  *HashContext
  );

typedef
BOOLEAN
(EFIAPI *CRYPTO_DUPLICATE) (
  IN   CONST VOID  *HashContext,
  OUT  VOID        *NewHashContext
  );

typedef
BOOLEAN
(EFIAPI *CRYPTO_UDPATE) (
  IN OUT  VOID        *HashContext,
  IN      CONST VOID  *Data,
  IN      UINTN       DataSize
  );

typedef
BOOLEAN
(EFIAPI *CRYPTO_FINAL) (
  IN OUT  VOID   *HashContext,
  OUT     UINT8  *HashValue
  );

typedef struct _CRYPTO_SELECTOR {
  EFI_GUID                 *Index;
  BOOLEAN                  NoPad;
  UINTN                    HashSize;
  CRYPTO_GET_CONTEXT_SIZE  GetContextSize;
  CRYPTO_INIT              Init;
  CRYPTO_DUPLICATE         Duplicate;
  CRYPTO_UDPATE            Update;
  CRYPTO_FINAL             Final;
} CRYPTO_SELECTOR;

enum {
  CRYPTO_ALGORITHM_NO_PAD_SHA1,
  CRYPTO_ALGORITHM_NO_PAD_SHA256,
  CRYPTO_ALGORITHM_SHA1,
  CRYPTO_ALGORITHM_SHA224,
  CRYPTO_ALGORITHM_SHA256,
  CRYPTO_ALGORITHM_SHA384,
  CRYPTO_ALGORITHM_SHA512,
  CRYPTO_ALGORITHM_MD5,
  CRYPTO_ALGORITHM_MAX
} CRYPTO_SELETOR_TYPE;


/*
  Returns the size of the hash which results from a specific algorithm.

  @parma This                        Points to this instance of EFI_HASH_PROTOCOL.
  @parma HashAlgorithm               Points to the EFI_GUID which identifies the algorithm to use.
  @parma HashSize                    Holds the returned size of the algorithm¡¯s hash.

  @retval EFI_SUCCESS                Hash size returned successfully.
  @retval EFI_INVALID_PARAMETER      HashSize is NULL
  @retval EFI_UNSUPPORTED            The algorithm specified by HashAlgorithm is not supported
                                     by this driver.
*/
EFI_STATUS
EFIAPI
CryptGetHashSize (
  IN  CONST EFI_HASH_PROTOCOL     *This,
  IN  CONST EFI_GUID              *HashAlgorithm,
  OUT UINTN                       *HashSize
  );

/*
  Returns the size of the hash which results from a specific algorithm.

  @parma This                        Points to this instance of EFI_HASH_PROTOCOL.
  @parma HashAlgorithm               Points to the EFI_GUID which identifies the algorithm to use.
  @parma Extend                      Specifies whether to create a new hash (FALSE) or extend the specified
                                     existing hash (TRUE).
  @parma Message                     Points to the start of the message.
  @parma MessageSize                 The size of Message, in bytes.
  @parma Hash                        On input, if Extend is TRUE, then this holds the hash to extend. On
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
  );

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
  IN  CONST EFI_GUID               *HashAlgorithm,
  OUT UINTN                        *HashSize
  );

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
  );

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
  );

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
  );

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
  );

/*
  Change address to virtual memory address
*/
VOID
CryptHashVirtualAddressChange (
  );

EFI_STATUS
EFIAPI
CryptoHashCreateChild (
  IN     EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN OUT EFI_HANDLE                    *ChildHandle
  );

EFI_STATUS
EFIAPI
CryptoHashDestroyChild (
  IN EFI_SERVICE_BINDING_PROTOCOL      *This,
  IN EFI_HANDLE                        ChildHandle
  );

EFI_STATUS
EFIAPI
CryptoHash2CreateChild (
  IN     EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN OUT EFI_HANDLE                    *ChildHandle
  );

EFI_STATUS
EFIAPI
CryptoHash2DestroyChild (
  IN EFI_SERVICE_BINDING_PROTOCOL      *This,
  IN EFI_HANDLE                        ChildHandle
  );

//
// Hash protocol related definitions
//
typedef struct _CRYPTO_HASH_INSTANCE {
  UINT32                               Signature;
  EFI_HASH_PROTOCOL                    HashProtocol;
  VOID                                 *Context[CRYPTO_ALGORITHM_MAX];
  BOOLEAN                              IsRoot;
} CRYPTO_HASH_INSTANCE;

#define CRYPTO_HASH_INSTANCE_FROM_THIS(a) \
  CR (a, \
      CRYPTO_HASH_INSTANCE, \
      HashProtocol, \
      CRYPTO_HASH_SIGNATURE \
      )

//
// Hash2 protocol related definitions
//
typedef struct {
  UINT32                           Signature;
  EFI_HANDLE                       Handle;
  EFI_HASH2_PROTOCOL               Hash2Protocol;
  VOID                             *HashContext;
  UINTN                            AlgorithmIndex;
  BOOLEAN                          IsRoot;
  BOOLEAN                          Updated;
} HASH2_INSTANCE_DATA;

#define HASH2_INSTANCE_DATA_FROM_THIS(a) \
  CR ( \
  (a), \
  HASH2_INSTANCE_DATA, \
  Hash2Protocol, \
  HASH2_INSTANCE_DATA_SIGNATURE \
  )


extern EFI_SERVICE_BINDING_PROTOCOL mHashServiceBindingProtocol;
extern EFI_SERVICE_BINDING_PROTOCOL mHash2ServiceBindingProtocol;
//
//Allocates memory blocks
//
void *malloc (UINTN size);
//
//Deallocates or frees a memory block
//
void free (void *ptr);

#define RTAllocatePool    malloc
#define RTFreePool        free
#endif
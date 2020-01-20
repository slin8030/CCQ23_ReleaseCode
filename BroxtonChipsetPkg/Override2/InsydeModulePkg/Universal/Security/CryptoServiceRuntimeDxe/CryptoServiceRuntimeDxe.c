/** @file
  CryptoServices protocol and Hash protocol driver instance
  It will implement two phase function, runtime and boot service.

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

#include <PiSmm.h>

#include <Protocol/SmmBase2.h>
#include <Protocol/CryptoServices.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadFile.h>
#include <Protocol/LoadFile2.h>
#include <Protocol/SmmRuntime.h>

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/DebugLib.h>
#include <Library/ImageRelocationLib.h>

#include <Guid/FileInfo.h>
#include <Guid/EventGroup.h>

#include <IndustryStandard/PeImage.h>

#include "CryptoHash.h"
#include "CryptoServiceProtocol.h"
#include "CryptoServiceRuntimeDxe.h"

EFI_SMM_SYSTEM_TABLE2                   *mSmst;
HASH2_INSTANCE_DATA                     mRootHash2Instance;
extern EFI_PKCS7_VERIFY_PROTOCOL        mPkcs7Verify;
/**
  Constructor routine for runtime crypt library instance.

  The constructor function pre-allocates space for runtime cryptographic operation.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS          The construction succeeded.
  @retval EFI_OUT_OF_RESOURCE  Failed to allocate memory.

**/
EFI_STATUS
EFIAPI
RuntimeCryptLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

CRYPTO_SERVICE                         *mCryptoService;
EFI_EVENT                              mVirtualAddrChange;
EFI_HANDLE                             mImageHandle;
EFI_HANDLE                             mNewImageHandle;
EFI_EVENT                              mSmmRuntimeProtocolNotifyEvent;

/*
  Change address to virtual memory address

  This function will change CryptoService protocol function to Runtime Phase.
*/
VOID
EFIAPI
CryptoServicesVirtualAddressChange (
  EFI_EVENT                            Event,
  VOID                                 *Context
  )
{
  CRYPTO_SERVICE                       *ProtocolServices;
  UINTN                                Index;

  ProtocolServices = Context;

  //
  // Change CryptoService protocol function to Runtime phase
  //
  SetupCryptoService (
    ProtocolServices->Handle,
    ProtocolServices,
    CRYPTO_SERVICE_CHANGE_TO_RUNTIME,
    NULL,
    NULL,
    NULL
    );

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->HashInstance.HashProtocol.GetHashSize));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->HashInstance.HashProtocol.Hash));
  for (Index = 0; Index < CRYPTO_ALGORITHM_MAX; Index++) {
    gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->HashInstance.Context[Index]));
  }
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->HashInstance));

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->Hash2Instance.Hash2Protocol.GetHashSize));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->Hash2Instance.Hash2Protocol.Hash));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->Hash2Instance.Hash2Protocol.HashInit));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->Hash2Instance.Hash2Protocol.HashUpdate));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->Hash2Instance.Hash2Protocol.HashFinal));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->Hash2Instance));

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.Sha1GetContextSize));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.Sha1Init));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.Sha1Duplicate));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.Sha1Update));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.Sha1Final));

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.HmacSha1GetContextSize));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.HmacSha1Init));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.HmacSha1Duplicate));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.HmacSha1Update));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.HmacSha1Final));

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.TdesGetContextSize));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.TdesInit));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.TdesEcbEncrypt));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.TdesEcbDecrypt));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.TdesCbcEncrypt));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.TdesCbcDecrypt));

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.AesCbcDecrypt));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.AesCbcEncrypt));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.AesEcbDecrypt));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.AesEcbEncrypt));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.AesGetContextSize));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.AesInit));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.Pkcs7Verify));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.Pkcs7VerifyUsingPubKey));

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.RsaGetPrivateKeyFromPem));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.RsaGetPublicKeyFromX509));

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.X509GetSubjectName));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.X509VerifyCert));

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.RsaFree));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.RsaNew));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.RsaSetKey));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.RsaPkcs1Sign));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.RsaPkcs1Verify));

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.DhNew));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.DhFree));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.DhGenerateParameter));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.DhSetParameter));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.DhGenerateKey));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.DhComputeKey));

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.RandomSeed));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.RandomBytes));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.AuthenticodeVerify));

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.Pkcs7GetSigners));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.Pkcs7FreeSigners));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.RsaPkcs1Decrypt));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.X509GetIssuerName));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.ImageTimestampVerify));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.X509GetTBSCert));

  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.HmacSha256GetContextSize));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.HmacSha256Init));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.HmacSha256Duplicate));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.HmacSha256Update));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.HmacSha256Final));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.Pkcs7GetCertificatesList));
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.Pbkdf2CreateKey));
//[-start-190320-IB07401093-add]//
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol.HkdfExtractAndExpand));
//[-end-190320-IB07401093-add]//
  gRT->ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ProtocolServices->CryptoServiceProtocol));

  CryptHashVirtualAddressChange ();
}


/**
  Install CryptoServices protocol.
  The Runtime boolean is indicate install to Runtime or BootService phase

  In BootService driver, it will install Hash and CryptoSerice with CryptoServiceBootService GUID.
  In Runtime driver, it will locate BootService protocol, and replce to CryptoService protocol.

  @param  Runtime                    Phase indicator

  @retval EFI_SUCCESS                Protocol install success
  @retval others                     Failed from another driver
**/
EFI_STATUS
CryptoServiceInstallProtocol (
  BOOLEAN                              Runtime
  )
{
  EFI_STATUS                           Status;
  EFI_HANDLE                           Handle;
  EFI_HASH_PROTOCOL                    *HashProtocol;
  EFI_HASH2_PROTOCOL                   *Hash2Protocol;
  CRYPTO_SERVICES_PROTOCOL             *CryptoService;

  EFI_GUID  CryptoServiceBootServiceProtocolGuid      = CRYPTO_SERVICE_BOOT_SERVICES_CRYPTO_PROTOCOL_GUID;
  EFI_GUID  CryptoServiceBootServiceHashProtocolGuid  = CRYPTO_SERVICE_BOOT_SERVICES_HASH_PROTOCOL_GUID;
  EFI_GUID  CryptoServiceBootServiceHash2ProtocolGuid = CRYPTO_SERVICE_BOOT_SERVICES_HASH2_PROTOCOL_GUID;

  HashProtocol   = NULL;
  Hash2Protocol  = NULL;
  CryptoService  = NULL;

  Status = gBS->AllocatePool (
                  (Runtime) ? EfiRuntimeServicesData : EfiBootServicesData,
                  sizeof (CRYPTO_SERVICE),
                  (VOID **)&mCryptoService
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ZeroMem (mCryptoService, sizeof (CRYPTO_SERVICE));

  if (Runtime) {
    Status = gBS->LocateProtocol (&CryptoServiceBootServiceHashProtocolGuid, NULL, (VOID **)&HashProtocol);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = gBS->LocateProtocol (&CryptoServiceBootServiceHash2ProtocolGuid, NULL, (VOID **)&Hash2Protocol);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = gBS->LocateProtocol (&CryptoServiceBootServiceProtocolGuid,     NULL, (VOID **)&CryptoService);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  SetupCryptoService (mImageHandle, mCryptoService, Runtime, HashProtocol, Hash2Protocol, CryptoService);

  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  (Runtime) ? &gEfiHashProtocolGuid        : &CryptoServiceBootServiceHashProtocolGuid,
                  (VOID *)  &(mCryptoService->HashInstance.HashProtocol),
                  (Runtime) ? &gEfiHash2ProtocolGuid        : &CryptoServiceBootServiceHash2ProtocolGuid,
                  (VOID *)  &(mCryptoService->Hash2Instance.Hash2Protocol),
                  (Runtime) ? &gCryptoServicesProtocolGuid : &CryptoServiceBootServiceProtocolGuid,
                  (VOID *) &(mCryptoService->CryptoServiceProtocol),
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (mCryptoService);
    ASSERT_EFI_ERROR (Status);
  }
  return Status;
}

/**
  This function called when SmmRT service installed.
  It will install CryptoService Protocol for Rev3.7 SMM interface

  @param                        NONE
  @retval                       NONE

**/
VOID
EFIAPI
CryptoServiceInstallProtocolCallBack (
  EFI_EVENT                            Event,
  VOID                                 *Context
  )
{
  EFI_STATUS                           Status;
  EFI_SMM_RUNTIME_PROTOCOL             *SmmRT;
  EFI_HANDLE                           Handle;

  Handle        = NULL;

  Status = gBS->LocateProtocol (&gEfiSmmRuntimeProtocolGuid, NULL, (VOID **)&SmmRT);
  if (!EFI_ERROR (Status)) {
    Status = SmmRT->InstallProtocolInterface (
                      &Handle,
                      &gCryptoServicesProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      (VOID *) &(mCryptoService->CryptoServiceProtocol)
                      );

    Status = SmmRT->InstallProtocolInterface (
                      Handle,
                      &gEfiHashProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      (VOID *) &(mCryptoService->HashInstance.HashProtocol)
                      );
  }
}

/*
  The entry point for PxeBc driver which install the driver
  binding and component name protocol on its image.

  @param  ImageHandle                The Image handle of the driver
  @param  SystemTable                The system table

  @retval EFI_SUCCESS                Driver initialize in RuntimeServices memory success
  @retval EFI_ALREADY_STARTED        Driver unload from BootServices memory
  @retval EFI_OUT_OF_RESOURCES       Allocate memory failed
  @retval Others                     Failed

*/
EFI_STATUS
EFIAPI
CryptoServiceSmmRuntimeEntry (
  IN EFI_HANDLE                        ImageHandle,
  IN EFI_SYSTEM_TABLE                  *SystemTable
  )
{
  EFI_STATUS                           Status;
  EFI_SMM_BASE2_PROTOCOL               *SmmBase;
  BOOLEAN                              InSmm;
  EFI_HANDLE                           Handle;
  EFI_EVENT                            CryptoServicesVirtualAddressChangeEvent;
  VOID                                 *Registration;

  mImageHandle   = ImageHandle;
  mSmst          = NULL;
  mCryptoService = NULL;

  //
  // SMM check
  //
  Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **)&SmmBase
                  );
  if (!EFI_ERROR (Status)) {
    SmmBase->InSmm (SmmBase, &InSmm);
  } else {
    InSmm = FALSE;
  }

  if (!InSmm) {
    gRT = SystemTable->RuntimeServices;

    if (!IsRuntimeDriver (ImageHandle)) {
      //
      // This section is BootService driver
      // Install CryptoService protocol for BootServices phase
      //
      Status = CryptoServiceInstallProtocol (CRYPTO_SERVICE_BOOT_SERVICES_PROTOCOL);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Status = RelocateImageToRuntimeDriver (ImageHandle);
      ASSERT_EFI_ERROR (Status);
      return EFI_SUCCESS;
    }

    //
    // Image location is Runtime Services
    //

    //
    // Initialize BaseCryptLib Memory services
    //
    Status = RuntimeCryptLibConstructor (ImageHandle, SystemTable);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = CryptoServiceInstallProtocol (CRYPTO_SERVICE_RUNTIME_PROTOCOL);
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
    //
    // Install EFI_HASH_SERVICE_BINDING_PROTOCOL
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &mImageHandle,
                    &gEfiHashServiceBindingProtocolGuid,
                    (VOID *) &mHashServiceBindingProtocol,
                    &gEfiHash2ServiceBindingProtocolGuid,
                    (VOID *) &mHash2ServiceBindingProtocol,
                    &gEfiPkcs7VerifyProtocolGuid,
                    (VOID *) &mPkcs7Verify,
                    NULL
                    );
    ASSERT_EFI_ERROR (Status);
    //
    // Create Event for change Runtime Function Pointer
    //
    CryptoServicesVirtualAddressChangeEvent = NULL;
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    CryptoServicesVirtualAddressChange,
                    (VOID *) mCryptoService,
                    &gEfiEventVirtualAddressChangeGuid,
                    &CryptoServicesVirtualAddressChangeEvent
                    );
  } else {
    //
    // Get Smm Syatem Table
    //
    Status = SmmBase->GetSmstLocation(
                        SmmBase,
                        &mSmst
                        );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = mSmst->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      sizeof (CRYPTO_SERVICE),
                      (VOID **)&mCryptoService
                      );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }

    SetupCryptoService (ImageHandle, mCryptoService, FALSE, NULL, NULL, NULL);

    //
    // SMM Entry
    //
    Handle = NULL;
    Status = mSmst->SmmInstallProtocolInterface (
                      &Handle,
                      &gEfiHashProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      (VOID *) &(mCryptoService->HashInstance.HashProtocol)
                      );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = mSmst->SmmInstallProtocolInterface (
                      &Handle,
                      &gEfiHash2ProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      (VOID *) &(mCryptoService->Hash2Instance.Hash2Protocol)
                      );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = mSmst->SmmInstallProtocolInterface (
                      &Handle,
                      &gCryptoServicesProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      (VOID *) &(mCryptoService->CryptoServiceProtocol)
                      );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // For old SMM interface before SMM_BASE2
    //
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    CryptoServiceInstallProtocolCallBack,
                    (VOID *) mCryptoService,
                    &mSmmRuntimeProtocolNotifyEvent
                    );
    if (!EFI_ERROR (Status)) {
      Status = gBS->RegisterProtocolNotify (
                      &gEfiSmmRuntimeProtocolGuid,
                      mSmmRuntimeProtocolNotifyEvent,
                      &Registration
                      );
    }
    return EFI_SUCCESS;
  }

  return Status;
}

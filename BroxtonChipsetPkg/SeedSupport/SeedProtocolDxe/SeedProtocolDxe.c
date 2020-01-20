/** @file
This file contains the entry code to the Seed Protocol DXE.
@copyright
  INTEL CONFIDENTIAL
  Copyright 2018 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification

**/

#include <Library/BaseCryptLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Guid/GlobalVariable.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/BootloaderSeedProtocol.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
//[-start-190321-IB07401094-add]//
#include <Guid/EventGroup.h>
#include <Protocol/CryptoServices.h>
#include <Protocol/Hash.h>
//[-end-190321-IB07401094-add]//

//[-start-190522-IB16530024-add]//
#include <Library/SteppingLib.h>
//[-end-190522-IB16530024-add]//

//[-start-190322-IB07401094-add]//
extern EFI_GUID gSeedSaltVariableGuid;
extern EFI_GUID gEfiBootloaderSeedProtocolGuid;

CRYPTO_SERVICES_PROTOCOL  *mCryptoService = NULL;
//[-end-190322-IB07401094-add]//

STATIC BUP_BOOTLOADER_SEED_LIST_INFO              BLSInfo = {0};
CONST STATIC UINT8                                TrustyInfo[]   = "trusty";
#define UEFI_SECURE_BOOT_DISABLE                  (0)
#define SEED_SALT_SIZE                            (32)

VOID
DumpHex (
  IN UINT8 *Buffer,
  IN UINTN BufferSize
  )
{
  UINTN  Index;
  UINTN  IndexJ;
#define COL_SIZE  16

  for (Index = 0; Index < BufferSize/COL_SIZE; Index++) {
    DEBUG ((DEBUG_INFO, "      %04x: ", Index * COL_SIZE));
    for (IndexJ = 0; IndexJ < COL_SIZE; IndexJ++) {
      DEBUG ((DEBUG_INFO, "%02x ", *(Buffer + Index * COL_SIZE + IndexJ)));
    }
    DEBUG ((DEBUG_INFO, "\n"));
  }
  if ((BufferSize % COL_SIZE) != 0) {
    DEBUG ((DEBUG_INFO, "      %04x: ", Index * COL_SIZE));
    for (IndexJ = 0; IndexJ < (BufferSize % COL_SIZE); IndexJ++) {
      DEBUG ((DEBUG_INFO, "%02x ", *(Buffer + Index * COL_SIZE + IndexJ)));
    }
    DEBUG ((DEBUG_INFO, "\n"));
  }
}


STATIC
VOID
ApplyUEFISecureBootCheck(
  VOID
  )
{
  UINT8                    *SecureBoot;
  UINT32                   i, j;

  SecureBoot = NULL;
  GetEfiGlobalVariable2 (EFI_SECURE_BOOT_MODE_NAME, (VOID**)&SecureBoot, NULL);

  if ((SecureBoot == NULL) ||
    (*SecureBoot == UEFI_SECURE_BOOT_DISABLE)) {

    for (i=0; i<BLSInfo.NumOfSeeds; i++){
      for (j=0; j < sizeof(BLSInfo.SeedList[i].Seed); j++){
        BLSInfo.SeedList[i].Seed[j] = 0x5A;
      }
    }
  }

  if (SecureBoot != NULL) {
    FreePool (SecureBoot);
  }
}


STATIC
EFI_STATUS
GetSeedSalt(
  UINT8    *SeedSalt,
  UINT32    SeedSaltSize
  )
{
  EFI_STATUS                 Status;
  UINTN                      SaltVarSize = 0;
  UINT8                      *SaltVarData = NULL;
  UINT8                      RandomSalt[SEED_SALT_SIZE] ;

  if ((SeedSalt == NULL) || (SeedSaltSize != SEED_SALT_SIZE))
    return EFI_INVALID_PARAMETER;

  Status = GetVariable2 (L"SeedSalt",
                &gSeedSaltVariableGuid, &SaltVarData, &SaltVarSize);

  if (Status == EFI_NOT_FOUND){

    DEBUG ((DEBUG_INFO, "SeedSalt not found. Generating ...\n"));

//[-start-190323-IB07401094-modify]//
    Status = mCryptoService->RandomBytes(RandomSalt, sizeof(RandomSalt));
//[-end-190323-IB07401094-modify]//
    if (Status != EFI_SUCCESS)
      goto _Exit;

    Status = gRT->SetVariable (
                  L"SeedSalt",
                  &gSeedSaltVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  sizeof (RandomSalt),
                  RandomSalt
                  );
    if (Status == EFI_SUCCESS) {
      CopyMem(SeedSalt, RandomSalt, SEED_SALT_SIZE);
    } else {
      DEBUG ((DEBUG_ERROR, " SetVariable(SeedSalt) failed with 0x%08x \n", Status));
    }

  } else if (Status == EFI_SUCCESS) {
    CopyMem(SeedSalt, SaltVarData, SEED_SALT_SIZE);

    DEBUG ((DEBUG_INFO, " Retrieved SeedSalt successfully from Variable Store \n"));
    DumpHex(SeedSalt, SEED_SALT_SIZE);
  }

  DEBUG ((DEBUG_INFO, " GetSeedSalt return Status 0x%08x \n", Status));

_Exit:
  if (SaltVarData != NULL) {
    FreePool(SaltVarData);
  }
  return Status;
}


/**
  Get the symmetric encryption key for Android Attestation Keybox.
  Kernelflinger will prepare for the data buffer, and the BIOS will copy the data to this buffer.

  @param[in, out]       buf      Buffer to fill with att kb key

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_INVALID_PARAMETER  One or more of the parameters are incorrect.
**/
EFI_STATUS
GetAttKBEncKey (
  IN OUT BOOTLOADER_ATTKB_ENC_KEY *buf
  )
{
  return EFI_DEVICE_ERROR;
}


/**
  Get the seed info list used by Trusty.
  Kernelflinger will prepare for the data buffer, and the BIOS will copy the data to this buffer.

  @param[in, out]       OutBLSList      Buffer to fill with Seed info list

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_INVALID_PARAMETER  One or more of the parameters are incorrect.
**/
EFI_STATUS
GetSeedInfoList(
  IN OUT BOOTLOADER_SEED_INFO_LIST *OutBLSList
  )
{
  UINT8                 SeedSalt[SEED_SALT_SIZE] = {0x0};
  UINT32                i;
  BOOLEAN               Ret;
  EFI_STATUS            Status;

  DEBUG ((DEBUG_INFO, "Get Seed Info List \n"));

  if (OutBLSList==NULL){
    return EFI_INVALID_PARAMETER;
  }

  Status = GetSeedSalt( SeedSalt,  sizeof(SeedSalt));
  if (Status != EFI_SUCCESS){
    return EFI_DEVICE_ERROR;
  }

  OutBLSList->NumOfSeeds=(UINT8)(BLSInfo.NumOfSeeds);
  for (i=0; i<OutBLSList->NumOfSeeds; i++){

    OutBLSList->SeedList[i].cse_svn=BLSInfo.SeedList[i].Svn;
    OutBLSList->SeedList[i].bios_svn=0xFF;
    *(UINT16*)(&(OutBLSList->SeedList[i].Reserved))=0;

//[-start-190321-IB07401094-modify]//
//    Ret = HkdfSha256ExtractAndExpand ( BLSInfo.SeedList[i].Seed, sizeof(BLSInfo.SeedList[i].Seed),
//      SeedSalt, sizeof(SeedSalt), TrustyInfo, sizeof(TrustyInfo), OutBLSList->SeedList[i].seed, sizeof(OutBLSList->SeedList[i].seed));
    Ret = mCryptoService->HkdfExtractAndExpand ( 
            &gEfiHashAlgorithmSha256Guid,
            BLSInfo.SeedList[i].Seed, 
            sizeof(BLSInfo.SeedList[i].Seed),
            SeedSalt, 
            sizeof(SeedSalt), 
            TrustyInfo, 
            sizeof(TrustyInfo), 
            OutBLSList->SeedList[i].seed, 
            sizeof(OutBLSList->SeedList[i].seed
            ));
//[-end-190321-IB07401094-modify]//

    if (Ret != TRUE) {
      return EFI_DEVICE_ERROR;
    }
  }

  return EFI_SUCCESS;
}


/**
  Get the RPMB key. This key is derived from CSE Fixed seed.
  Kernelflinger will prepare for the data buffer, and the BIOS will copy the data to this buffer.

  @param[in, out]       num_keys   Number of RPMB keys
  @param[in, out]       buf        Buffer to fill with Seed info list

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_INVALID_PARAMETER  One or more of the parameters are incorrect.
**/
EFI_STATUS
GetRpmbKey(
  IN OUT UINT8 *num_keys,
  IN OUT BOOTLOADER_RPMB_KEY *buf
  )
{
  return EFI_DEVICE_ERROR;
}


/**
  This API can be invoked by OS loader application after it has consumed seeds.

  @retval EFI_SUCCESS            Operation completed successfully.
**/
EFI_STATUS
EndOfService ()
{
  ZeroMem(&BLSInfo, sizeof(BUP_BOOTLOADER_SEED_LIST_INFO));
  return EFI_SUCCESS;
}


BOOTLOADER_SEED_PROTOCOL mCseProtocol = {
  EndOfService,
  GetSeedInfoList,
  GetRpmbKey,
  GetAttKBEncKey
};

/**
  Unregister status code callback functions only available at boot time from
  report status code router when exiting boot services.

  @param  Event         Event whose notification function is being invoked.
  @param  Context       Pointer to the notification function's context, which is
                        always zero in current implementation.

**/
VOID
EFIAPI
UnregisterSeedProtocolHandlers (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  DEBUG ((DEBUG_INFO, "Unregister Seed Protocol \n"));
  ZeroMem(&BLSInfo, sizeof(BUP_BOOTLOADER_SEED_LIST_INFO));
}

/**
  Seed Protocol Entry
  @retval  status
**/
EFI_STATUS
SeedProtocolEntry (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_HANDLE                    mHandle = NULL;
  EFI_STATUS                    Status;
  EFI_EVENT                     EfiExitBootServicesEvent;
//[-start-190522-IB16530024-add]//
  BXT_STEPPING                    SocStepping;
//[-end-190522-IB16530024-add]//

  DEBUG ((DEBUG_INFO, "SeedProtocolEntry \n"));
//[-start-190522-IB16530024-add]//
  SocStepping = BxtStepping ();
  if (SocStepping <= BxtPA0) {
    DEBUG ((DEBUG_INFO, "Stepping earlier than B0 SeedProtocol unsupported\n"));
    return EFI_UNSUPPORTED;
  }
//[-end-190522-IB16530024-add]//

  Status = HeciGetBootloaderSeedList(&BLSInfo);

  if (Status == EFI_SUCCESS){
    ApplyUEFISecureBootCheck();
  } else {
    DEBUG ((DEBUG_ERROR, "Seed Retrieval from Heci FAIL \n"));
//[-start-190323-IB07401094-add]//
    return Status;
//[-end-190323-IB07401094-add]//
  }

//[-start-190321-IB07401094-add]//
  Status = gBS->LocateProtocol (
                 &gCryptoServicesProtocolGuid,
                 NULL,
                 &mCryptoService
                 );
  if (EFI_ERROR (Status)) {
    return Status;
  }
//[-end-190321-IB07401094-add]//

  DEBUG ((DEBUG_INFO, "Install Protocol to expose CSE_PROTOCOL_GET_SEED_INFO_LIST\n"));
  Status = gBS->InstallMultipleProtocolInterfaces (
    &mHandle,
    &gEfiBootloaderSeedProtocolGuid,
    &mCseProtocol,
    NULL
    );

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  UnregisterSeedProtocolHandlers,
                  NULL,
                  &gEfiEventExitBootServicesGuid,
                  &EfiExitBootServicesEvent
                  );

  return EFI_SUCCESS;
}
/** @file
  Capsule Update Runtime Services

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

#include <Uefi.h>
#include <Protocol/Capsule.h>
#include <Guid/FmpCapsule.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/CapsuleLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#define MAX_CAPSULE_NAME_LENGTH         24

//
// Handle for the installation of Capsule Architecture Protocol.
//
EFI_HANDLE      mNewHandle = NULL;
UINTN           mTimes      = 0;
UINT32          mMaxSizePopulateCapsule     = 0;
UINT32          mMaxSizeNonPopulateCapsule  = 0;
UINT8           mDstBuffer[0x40000];

extern  EFI_GUID gEfiCrashDumpCapsuleGuid;

EFI_STATUS
EFIAPI
TianoCompress (
  IN     UINT8  *SrcBuffer,
  IN     UINT32 SrcSize,
  IN     UINT8  *DstBuffer,
  IN OUT UINT32 *DstSize
  );

EFI_STATUS
EFIAPI
TianoDecompress (
  IN     VOID   *Source,
  IN OUT VOID   *Destination
  );

#pragma pack(1)
typedef struct {
  UINTN                 CapsuleCount;
  EFI_CAPSULE_HEADER    *CapsuleHeaderPtr;
  EFI_CAPSULE_HEADER    CapsuleHeader;
} EFI_CRASH_DUMP_CAPSULE_HEADER;
#pragma pack()

STATIC
VOID
InstallCrashDumpTable (
  )
{
  EFI_STATUS                    Status;
  UINTN                         Index;
  UINTN                         Counter;
  UINTN                         TotalCompressedSize;
  UINTN                         TotalSize;
  UINTN                         DataSize;
  CHAR16                        CapsuleVarName[MAX_CAPSULE_NAME_LENGTH];
  EFI_CRASH_DUMP_CAPSULE_HEADER *CrashCapsuleHeader;
  UINT8                         *Buffer;
  UINT8                         *Ptr;
  UINT8                         *Src;
  UINT8                         *Dst;
  //
  // Count whole crash dump variables
  //
  for (Index = 0, TotalCompressedSize = 0;; Index ++) {
    DataSize = 0;
    UnicodeSPrint (CapsuleVarName, MAX_CAPSULE_NAME_LENGTH * sizeof (CHAR16), L"CrashDumpCapsule%04X", Index);
    Status = EfiGetVariable (
               CapsuleVarName,
               &gEfiCrashDumpCapsuleGuid,
               NULL,
               &DataSize,
               NULL
               );
    if (Status == EFI_BUFFER_TOO_SMALL) {
      TotalCompressedSize += DataSize;
    } else if (Status == EFI_NOT_FOUND) {
      break;
    }
  }
  if (TotalCompressedSize == 0) {
    return;
  }
  Buffer = (UINT8*)AllocatePool(TotalCompressedSize);
  if (Buffer == NULL) {
    return;
  }
  for (Counter = 0, TotalSize = 0, Ptr = Buffer; Counter < Index; Counter ++) {
    DataSize = TotalCompressedSize;
    UnicodeSPrint (CapsuleVarName, MAX_CAPSULE_NAME_LENGTH * sizeof (CHAR16), L"CrashDumpCapsule%04X", Counter);
    Status = EfiGetVariable (
               CapsuleVarName,
               &gEfiCrashDumpCapsuleGuid,
               NULL,
               &DataSize,
               Ptr
               );
    if (!EFI_ERROR (Status)) {
      TotalSize += (UINTN)(Ptr[4] + (Ptr[5] << 8) + (Ptr[6] << 16) + (Ptr[7] << 24));
      Ptr += DataSize;
    }
    //
    // Delete the crash dump variable after get the payload
    //
    Status = EfiSetVariable (
               CapsuleVarName,
               &gEfiCrashDumpCapsuleGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
               0,
               NULL
               );
  }
  CrashCapsuleHeader = (EFI_CRASH_DUMP_CAPSULE_HEADER*)AllocateRuntimePool(TotalSize + sizeof(EFI_CRASH_DUMP_CAPSULE_HEADER));
  if (CrashCapsuleHeader == NULL) {
    return;
  }
  CrashCapsuleHeader->CapsuleCount = 1;
  CrashCapsuleHeader->CapsuleHeaderPtr = &CrashCapsuleHeader->CapsuleHeader;
  for (Counter = 0, Src = Buffer, Dst = (UINT8*)CrashCapsuleHeader->CapsuleHeaderPtr; Counter < Index; Counter ++) {
    Status = TianoDecompress (
               Src,
               Dst
               );
    if (!EFI_ERROR (Status)) {
      Ptr = Src;
      Src += (UINTN)(Ptr[0] + (Ptr[1] << 8) + (Ptr[2] << 16) + (Ptr[3] << 24));
      Dst += (UINTN)(Ptr[4] + (Ptr[5] << 8) + (Ptr[6] << 16) + (Ptr[7] << 24));
    } 
  }
  FreePool (Buffer);
  //
  // Populate the crash dump into configuration table
  //
  gBS->InstallConfigurationTable(
         &gEfiCrashDumpCapsuleGuid,
         (VOID *)(UINTN)CrashCapsuleHeader
         );
}

/**
  Passes capsules to the firmware with both virtual and physical mapping. Depending on the intended
  consumption, the firmware may process the capsule immediately. If the payload should persist
  across a system reset, the reset value returned from EFI_QueryCapsuleCapabilities must
  be passed into ResetSystem() and will cause the capsule to be processed by the firmware as
  part of the reset process.

  @param  CapsuleHeaderArray    Virtual pointer to an array of virtual pointers to the capsules
                                being passed into update capsule.
  @param  CapsuleCount          Number of pointers to EFI_CAPSULE_HEADER in
                                CaspuleHeaderArray.
  @param  ScatterGatherList     Physical pointer to a set of
                                EFI_CAPSULE_BLOCK_DESCRIPTOR that describes the
                                location in physical memory of a set of capsules.

  @retval EFI_SUCCESS           Valid capsule was passed. If
                                CAPSULE_FLAGS_PERSIT_ACROSS_RESET is not set, the
                                capsule has been successfully processed by the firmware.
  @retval EFI_DEVICE_ERROR      The capsule update was started, but failed due to a device error.
  @retval EFI_INVALID_PARAMETER CapsuleSize is NULL, or an incompatible set of flags were
                                set in the capsule header.
  @retval EFI_INVALID_PARAMETER CapsuleCount is Zero.
  @retval EFI_INVALID_PARAMETER For across reset capsule image, ScatterGatherList is NULL.
  @retval EFI_UNSUPPORTED       CapsuleImage is not recognized by the firmware.
  @retval EFI_OUT_OF_RESOURCES  When ExitBootServices() has been previously called this error indicates the capsule 
                                is compatible with this platform but is not capable of being submitted or processed 
                                in runtime. The caller may resubmit the capsule prior to ExitBootServices().
  @retval EFI_OUT_OF_RESOURCES  When ExitBootServices() has not been previously called then this error indicates 
                                the capsule is compatible with this platform but there are insufficient resources to process.

**/
EFI_STATUS
EFIAPI
UpdateCapsule (
  IN EFI_CAPSULE_HEADER      **CapsuleHeaderArray,
  IN UINTN                   CapsuleCount,
  IN EFI_PHYSICAL_ADDRESS    ScatterGatherList OPTIONAL
  )
{
  UINTN                     ArrayNumber;
  EFI_STATUS                Status;
  EFI_CAPSULE_HEADER        *CapsuleHeader;
  BOOLEAN                   NeedReset;
  BOOLEAN                   InitiateReset;
  CHAR16                    CapsuleVarName[MAX_CAPSULE_NAME_LENGTH];
  UINT32                    DstSize;

  //
  // Capsule Count can't be less than one.
  //
  if (CapsuleCount < 1) {
    return EFI_INVALID_PARAMETER;
  }

  Status            = EFI_UNSUPPORTED;
  NeedReset         = FALSE;
  InitiateReset     = FALSE;
  CapsuleHeader     = NULL;

  for (ArrayNumber = 0; ArrayNumber < CapsuleCount; ArrayNumber++) {
    //
    // A capsule which has the CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE flag must have
    // CAPSULE_FLAGS_PERSIST_ACROSS_RESET set in its header as well.
    //
    CapsuleHeader = CapsuleHeaderArray[ArrayNumber];
    if ((CapsuleHeader->Flags & (CAPSULE_FLAGS_PERSIST_ACROSS_RESET | CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE)) == CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) {
      return EFI_INVALID_PARAMETER;
    }
    //
    // A capsule which has the CAPSULE_FLAGS_INITIATE_RESET flag must have
    // CAPSULE_FLAGS_PERSIST_ACROSS_RESET set in its header as well.
    //
    if ((CapsuleHeader->Flags & (CAPSULE_FLAGS_PERSIST_ACROSS_RESET | CAPSULE_FLAGS_INITIATE_RESET)) == CAPSULE_FLAGS_INITIATE_RESET) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // Check FMP capsule flag 
    //
    if (CompareGuid(&CapsuleHeader->CapsuleGuid, &gEfiFmpCapsuleGuid)
     && (CapsuleHeader->Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) != 0 ) {
       return EFI_INVALID_PARAMETER;
    }

    //
    // Check Capsule image without populate flag by firmware support capsule function  
    //
    if ((CapsuleHeader->Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) == 0) {
      Status = SupportCapsuleImage (CapsuleHeader);
      if (EFI_ERROR(Status)) {
        continue;
      }
    }

    //
    // ScatterGatherList is only referenced if the capsules are defined to persist across
    // system reset. 
    //
    if (((CapsuleHeader->Flags & CAPSULE_FLAGS_PERSIST_ACROSS_RESET) == CAPSULE_FLAGS_PERSIST_ACROSS_RESET) &&
         ScatterGatherList == (EFI_PHYSICAL_ADDRESS) (UINTN) NULL) {
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Walk through all capsules, record whether there is a capsule needs reset
  // or initiate reset. And then process capsules which has no reset flag directly.
  //
  for (ArrayNumber = 0; ArrayNumber < CapsuleCount; ArrayNumber++) {
    CapsuleHeader = CapsuleHeaderArray[ArrayNumber];
    //
    // Setup reset related flags before processing image to make the reset
    // able to be processed whether processing image successful or not 
    //
    if ((CapsuleHeader->Flags & CAPSULE_FLAGS_PERSIST_ACROSS_RESET) == CAPSULE_FLAGS_PERSIST_ACROSS_RESET) {
      NeedReset = TRUE;
      if ((CapsuleHeader->Flags & CAPSULE_FLAGS_INITIATE_RESET) != 0) {
        InitiateReset = TRUE;
      }
    }
    if (EfiAtRuntime()) {
      //
      // We only support crash dump capsule during runtime
      //
      if (CompareGuid(&CapsuleHeader->CapsuleGuid, &gEfiCrashDumpCapsuleGuid)) {
        DstSize = 0x40000;
        Status = TianoCompress (
                   (UINT8*)CapsuleHeader,
                   (UINT32)(CapsuleHeader->HeaderSize + CapsuleHeader->CapsuleImageSize),
                   mDstBuffer,
                   &DstSize
                   );
        if (EFI_ERROR (Status)) {
          return EFI_UNSUPPORTED;
        }
        //
        // Preserve crash dump into UEFI variable
        //
        UnicodeSPrint (CapsuleVarName, MAX_CAPSULE_NAME_LENGTH * sizeof (CHAR16), L"CrashDumpCapsule%04X", mTimes);
        Status = EfiSetVariable (
                   CapsuleVarName,
                   &gEfiCrashDumpCapsuleGuid,
                   EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                   DstSize,
                   (VOID*)mDstBuffer
                   );
        if (!EFI_ERROR (Status)) {
          //
          // Variable has been set successfully, increase variable index.
          //
          mTimes++;
        }
      } else {
        return EFI_UNSUPPORTED;
      }
    } else {
      //
      // Always process capsule image whether CAPSULE_FLAGS_PERSIST_ACROSS_RESET set or not 
      //
      Status = ProcessCapsuleImage(CapsuleHeader);
      if (EFI_ERROR(Status)) {
        break;
      }
    }
  }
  //
  // After launching all capsules who has no reset flag, if no more capsules claims
  // for a system reset just return.
  //
  if (!NeedReset) {
    return EFI_SUCCESS;
  }

  //
  // Check if the platform supports update capsule across a system reset
  //
  if (!FeaturePcdGet(PcdSupportUpdateCapsuleReset)) {
    return EFI_UNSUPPORTED;
  }

  if(InitiateReset) {
    //
    // Firmware that encounters a capsule which has the CAPSULE_FLAGS_INITIATE_RESET Flag set in its header
    // will initiate a reset of the platform which is compatible with the passed-in capsule request and will 
    // not return back to the caller.
    //
    EfiResetSystem (EfiResetWarm, EFI_SUCCESS, 0, NULL);
  }
  return Status;
}

/**
  Returns if the capsule can be supported via UpdateCapsule().

  @param  CapsuleHeaderArray    Virtual pointer to an array of virtual pointers to the capsules
                                being passed into update capsule.
  @param  CapsuleCount          Number of pointers to EFI_CAPSULE_HEADER in
                                CaspuleHeaderArray.
  @param  MaxiumCapsuleSize     On output the maximum size that UpdateCapsule() can
                                support as an argument to UpdateCapsule() via
                                CapsuleHeaderArray and ScatterGatherList.
  @param  ResetType             Returns the type of reset required for the capsule update.

  @retval EFI_SUCCESS           Valid answer returned.
  @retval EFI_UNSUPPORTED       The capsule image is not supported on this platform, and
                                MaximumCapsuleSize and ResetType are undefined.
  @retval EFI_INVALID_PARAMETER MaximumCapsuleSize is NULL, or ResetTyep is NULL,
                                Or CapsuleCount is Zero, or CapsuleImage is not valid.

**/
EFI_STATUS
EFIAPI
QueryCapsuleCapabilities (
  IN  EFI_CAPSULE_HEADER   **CapsuleHeaderArray,
  IN  UINTN                CapsuleCount,
  OUT UINT64               *MaxiumCapsuleSize,
  OUT EFI_RESET_TYPE       *ResetType
  )
{
  EFI_STATUS                Status;
  UINTN                     ArrayNumber;
  EFI_CAPSULE_HEADER        *CapsuleHeader;
  BOOLEAN                   NeedReset;

  //
  // Capsule Count can't be less than one.
  //
  if (CapsuleCount < 1) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Check whether input parameter is valid
  //
  if ((MaxiumCapsuleSize == NULL) ||(ResetType == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  CapsuleHeader = NULL;
  NeedReset     = FALSE;

  for (ArrayNumber = 0; ArrayNumber < CapsuleCount; ArrayNumber++) {
    CapsuleHeader = CapsuleHeaderArray[ArrayNumber];
    //
    // A capsule which has the CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE flag must have
    // CAPSULE_FLAGS_PERSIST_ACROSS_RESET set in its header as well.
    //
    if ((CapsuleHeader->Flags & (CAPSULE_FLAGS_PERSIST_ACROSS_RESET | CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE)) == CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) {
      return EFI_INVALID_PARAMETER;
    }
    //
    // A capsule which has the CAPSULE_FLAGS_INITIATE_RESET flag must have
    // CAPSULE_FLAGS_PERSIST_ACROSS_RESET set in its header as well.
    //
    if ((CapsuleHeader->Flags & (CAPSULE_FLAGS_PERSIST_ACROSS_RESET | CAPSULE_FLAGS_INITIATE_RESET)) == CAPSULE_FLAGS_INITIATE_RESET) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // Check FMP capsule flag 
    //
    if (CompareGuid(&CapsuleHeader->CapsuleGuid, &gEfiFmpCapsuleGuid)
     && (CapsuleHeader->Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) != 0 ) {
       return EFI_INVALID_PARAMETER;
    }

    //
    // Check Capsule image without populate flag is supported by firmware
    //
    if ((CapsuleHeader->Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) == 0) {
      Status = SupportCapsuleImage (CapsuleHeader);
      if (EFI_ERROR(Status)) {
        return Status;
      }
    }
  }

  //
  // Find out whether there is any capsule defined to persist across system reset. 
  //
  for (ArrayNumber = 0; ArrayNumber < CapsuleCount ; ArrayNumber++) {
    CapsuleHeader = CapsuleHeaderArray[ArrayNumber];
    if ((CapsuleHeader->Flags & CAPSULE_FLAGS_PERSIST_ACROSS_RESET) != 0) {
      NeedReset = TRUE;
      break;
    }
  }

  if (NeedReset) {
    //
    //Check if the platform supports update capsule across a system reset
    //
    if (!FeaturePcdGet(PcdSupportUpdateCapsuleReset)) {
      return EFI_UNSUPPORTED;
    }
    *ResetType = EfiResetWarm;
    *MaxiumCapsuleSize = (UINT64) mMaxSizePopulateCapsule;
  } else {
    //
    // For non-reset capsule image.
    //
    *ResetType = EfiResetCold;
    *MaxiumCapsuleSize = (UINT64) mMaxSizeNonPopulateCapsule;
  }

  return EFI_SUCCESS;
}


/**

  This code installs UEFI capsule runtime service.

  @param  ImageHandle    The firmware allocated handle for the EFI image.  
  @param  SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS    UEFI Capsule Runtime Services are installed successfully. 

**/
EFI_STATUS
EFIAPI
CapsuleServiceInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;

  mMaxSizePopulateCapsule = PcdGet32(PcdMaxSizePopulateCapsule);
  mMaxSizeNonPopulateCapsule = PcdGet32(PcdMaxSizeNonPopulateCapsule);

  //
  // Install capsule runtime services into UEFI runtime service tables.
  //
  gRT->UpdateCapsule                    = UpdateCapsule;
  gRT->QueryCapsuleCapabilities         = QueryCapsuleCapabilities;

  //
  // Install the Capsule Architectural Protocol on a new handle
  // to signify the capsule runtime services are ready.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mNewHandle,
                  &gEfiCapsuleArchProtocolGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  
  InstallCrashDumpTable ();

  return Status;
}

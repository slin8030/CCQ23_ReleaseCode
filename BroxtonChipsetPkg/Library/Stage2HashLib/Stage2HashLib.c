/** @file
  Common stage 2 hash data operation functions

;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/Stage2HashLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Guid/FirmwareFileSystem2.h>
#include <Guid/Stage2HashFile.h>
#include <ChipsetOemBlockData.h>

#define OEM_BLOCK_OFFSET_IN_MANIFEST    0x58

/**
 This function will return if manifest is available.
 @retval            TRUE                Manifest is available.
 @retval            FALSE               Manifest is not available.
*/
BOOLEAN 
ManifestAvailable(
  VOID
  )
{
  //
  // If Recovery FV is less than 127K, we will use manifest
  //
  if (PcdGet32 (PcdFlashFvRecoveryBase) >= 
     (PcdGet32 (PcdManifestBase) + PcdGet32 (PcdManifestSize))) {
    return TRUE;
  }
  
  return FALSE;
}

/**
 This function will return stage 2 hash data size.
 @param[out]        *HashSize           Return the data size of stage 2 hash.
 @retval            EFI_SUCCESS         Operation successfully completed.
*/
EFI_STATUS
GetStage2HashSize (
  OUT UINTN                                   *HashSize
  )
{
  *HashSize = SHA256_DIGEST_SIZE;
  return EFI_SUCCESS;  
}

/**
 Find Hash File buffer in Recovery FV in memory.
 
 @param[in]         RecoveryFv          Pointer to recovery FV in current memory
 @param[out]        RawFile             Pointer to RawFile pointer
 @retval            EFI_SUCCESS         Find the file
                    EFI_NOT_FOUND       Do not find the file
*/
STATIC
EFI_STATUS
GetHashRawFileInRecoveryFv(
  IN  VOID *                                   RecoveryFv,
  OUT VOID **                                  RawFile
)
{
  EFI_FIRMWARE_VOLUME_HEADER    *VolumeHeader;
  EFI_FFS_FILE_HEADER           *FfsFileHeader;
  EFI_FFS_FILE_HEADER           FfsEndHeader;
  UINTN                         CurrentOffset;
  UINTN                         FvLength;
  UINTN                         FfsOccupiedSize;
  UINT32                        FfsFileSize;

  *RawFile = NULL;
  VolumeHeader = (EFI_FIRMWARE_VOLUME_HEADER *)RecoveryFv;
  if ((VolumeHeader->Signature != EFI_FVH_SIGNATURE) ||
      (VolumeHeader->Revision != EFI_FVH_REVISION) ||
      (!CompareGuid (&VolumeHeader->FileSystemGuid, &gEfiFirmwareFileSystem2Guid))) {
    return EFI_NOT_FOUND;
  }

  FvLength = (UINTN)VolumeHeader->FvLength;
  SetMem (&FfsEndHeader, (UINTN)sizeof(EFI_FFS_FILE_HEADER), (UINT8)(((VolumeHeader->Attributes & EFI_FVB2_ERASE_POLARITY) != 0) ? 0xFF : 0x00));

  //
  // Find Raw file with Hash GUID
  //
  for (FfsFileHeader = (EFI_FFS_FILE_HEADER *)((UINT8 *)VolumeHeader + VolumeHeader->HeaderLength), CurrentOffset = VolumeHeader->HeaderLength;
       (CurrentOffset < FvLength) && (CompareMem(FfsFileHeader, &FfsEndHeader, sizeof(EFI_FFS_FILE_HEADER)) != 0);
       FfsFileHeader = (EFI_FFS_FILE_HEADER *)(((UINT8*)VolumeHeader) + CurrentOffset)) {

    FfsFileSize = (*((UINT32 *)FfsFileHeader->Size)) & 0xffffff; 
    if (FfsFileSize < sizeof(EFI_FFS_FILE_HEADER)) {
      break;
    }

    FfsOccupiedSize = ((FfsFileSize + 7) / 8 * 8);
    if ((FfsFileHeader->Type == EFI_FV_FILETYPE_RAW) && CompareGuid (&FfsFileHeader->Name, &gStage2HashFileGuid))  {
      *RawFile = (FfsFileHeader + 1);
      break;
    }

    CurrentOffset = CurrentOffset + FfsOccupiedSize; 
  }

  if (*RawFile != NULL) {
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}


/**
 This function will return stage 2 physical address.
 @param[in]         RecoveryFv          Pointer to recovery FV in current memory
 @param[out]        *Offset             Return hash offset.
 @retval            EFI_SUCCESS         Operation successfully completed.
 @retval            EFI_NOT_FOUND       Can not find the address.
*/
EFI_STATUS
GetStage2HashPhysicalAddress (
  IN  VOID *                                   RecoveryFv,
  OUT PHYSICAL_ADDRESS                         *Address
  )
{
  EFI_STATUS    Status;
  VOID          *HashFile;
  
  if (ManifestAvailable()) {
    *Address = (PHYSICAL_ADDRESS) (&((CHIPSET_OEM_BLOCK_DATA *)(UINTN)(PcdGet32(PcdManifestBase) + OEM_BLOCK_OFFSET_IN_MANIFEST))->Stage2HashBlock.Stage2Hash);
  } else {
    Status = GetHashRawFileInRecoveryFv(RecoveryFv, &HashFile);
    if (!EFI_ERROR(Status)) {
      *Address = (PHYSICAL_ADDRESS)PcdGet32(PcdFlashFvRecoveryBase) + (PHYSICAL_ADDRESS)((UINTN)HashFile - (UINTN)RecoveryFv); 
    } else {
      *Address = 0;
      return EFI_NOT_FOUND;
    }
  }
  return EFI_SUCCESS;  
}  

/**
 This function will return stage 2 hash data offset in a fd file.
 @param[in]         RecoveryFv          Pointer to recovery FV in current memory
 @param[out]        *Offset             Return hash offset.
 @retval            EFI_SUCCESS         Operation successfully completed.
 @retval            EFI_NOT_FOUND       Can not find the address.
*/
EFI_STATUS
GetStage2HashFdOffset (
  IN  VOID *                                  RecoveryFv,
  OUT UINTN                                   *Offset
  )
{
  PHYSICAL_ADDRESS    Stage2Address;
  EFI_STATUS          Status;
  
  Status = GetStage2HashPhysicalAddress(RecoveryFv, &Stage2Address);
  if (EFI_ERROR(Status)) {
    *Offset = 0;
  } else {
    *Offset = (UINTN)Stage2Address - (UINTN)PcdGet32(PcdFlashAreaBaseAddress);
  }

  return Status;
}

/**
 This function will return physical address of stage 2 hash data.
 @param[out]        *Offset             Return hash offset.
 @retval            EFI_SUCCESS         Operation successfully completed.
*/
EFI_STATUS
GetStage2PhysicalAddress (
  OUT PHYSICAL_ADDRESS                        *Address
  )
{
  *Address = (PHYSICAL_ADDRESS)PcdGet32(PcdFlashFvMainBase); 

  return EFI_SUCCESS;  
}  


/**
 This function will return stage 2 ffset in a fd file.
 @param[out]        *Offset             Return hash offset.
 @retval            EFI_SUCCESS         Operation successfully completed.
*/
EFI_STATUS
GetStage2FdOffset (
  OUT UINTN                                   *Offset
  )
{
  PHYSICAL_ADDRESS        Stage2Start;
  
  GetStage2PhysicalAddress(&Stage2Start);
#ifdef BIOS_2MB_BUILD
  if (FeaturePcdGet(PcdSpi2mSupport)) {
    *Offset = (UINTN)Stage2Start - (UINTN)PcdGet32(PcdFlashAreaBase2Address); 
  } 
#else  
  {
    *Offset = (UINTN)Stage2Start -  (UINTN)PcdGet32(PcdFlashAreaBaseAddress);
  }
#endif
  return EFI_SUCCESS;  
}

/**
 This function will return stage 2 size in bytes.
 @param[out]        *Offset             Return size.
 @retval            EFI_SUCCESS         Operation successfully completed.
*/
EFI_STATUS
GetStage2Size (
  OUT UINTN                                   *Size
  )
{
  PHYSICAL_ADDRESS        Stage2Start;
  
  GetStage2PhysicalAddress(&Stage2Start);
#ifdef BIOS_2MB_BUILD
  if (FeaturePcdGet(PcdSpi2mSupport)) {
    //
    // DXE & PEI FVs in eMMC FD
    //
    *Size = (UINTN)PcdGet32(PcdFlashAreaBase2Address) + (UINTN)PcdGet32(PcdFlashArea2Size) - (UINTN)PcdGet32(PcdFlashFvMainBase);
  }  
#else  
  {
    *Size = (UINTN)PcdGet32(PcdFlashFvRecovery2Base) + (UINTN)PcdGet32(PcdFlashFvRecovery2Size) - (UINTN)Stage2Start;
  }
#endif 
  
  return EFI_SUCCESS;

}
  
/**
 This function will calculate hash data provides an interface to modify OEM Logo and POST String.
 @param[in]         Stage2              Pointer to stage 2 data.
 @param[in]         Stage2Size          Size of stage 2.
 @param[out]        HashData            Caller will allocate the buffer enough to fill hash data.
                                        On exit, this function will return the hash data of stage 2 data.
 @retval            EFI_SUCCESS         Operation successfully completed.
*/
EFI_STATUS
GetStage2HashData (
  IN     VOID                                  *Stage2,
  IN     UINTN                                 Stage2Size,              
  OUT    VOID                                  *HashData
  )
{
  EFI_STATUS Status;
  UINTN      CtxSize;
  VOID       *HashCtx;

  Status  = EFI_ABORTED;
  HashCtx = NULL;
  CtxSize = Sha256GetContextSize ();
  
  ZeroMem (HashData, SHA256_DIGEST_SIZE);
  HashCtx = AllocatePool (CtxSize);

  if (!Sha256Init (HashCtx)) {
    goto GetHashDataFailClean;
  }

  if (!Sha256Update (HashCtx, (VOID*)Stage2, Stage2Size)) {
    goto GetHashDataFailClean;
  }

  if (!Sha256Final (HashCtx, HashData)) {
    goto GetHashDataFailClean;
  }
  
  Status = EFI_SUCCESS;

GetHashDataFailClean:  
  
  if (HashCtx != NULL) {
    FreePool(HashCtx);    
  }
  return Status;
}
  

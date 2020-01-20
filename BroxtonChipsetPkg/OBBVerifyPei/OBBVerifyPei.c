/** @file
  OBB verify PEI module.

@copyright
 Copyright (c) 2016 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.

This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.
**/

#include "OBBVerifyPei.h"
#include <Library/HeciMsgLib.h>
#include <Library/PmcIpcLib.h>
#include <Library/MmPciLib.h>
#include <Guid/PlatformInfo.h>
#include <Library/DebugLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/PeiServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>



extern EFI_GUID gEfiBootMediaHobGuid;
extern EFI_GUID gUfsBootLunIdHobGuid;


#pragma pack(push, 1)
typedef struct {
  UINT64  Address;
  UINT32  Size:24;
  UINT32  Rsvd:8;
  UINT16  Version;
  UINT8   Type:7;
  UINT8   C_V:1;
  UINT8   Checksum;
} FitEntry;

#pragma pack(pop)


#define SECTOR_SIZE_4KB   0x1000      // Common 4kBytes sector size



//
// Get BIOS boot device
//
EFI_STATUS
EFIAPI
PeiGetBootDevice (
  OUT UINT32                   *BootDevice
  )
{
  MBP_CURRENT_BOOT_MEDIA         *BootMediaData;
  EFI_PEI_HOB_POINTERS           HobPtr;

  //
  // Get Boot Media Hob
  //
  SetMem (&HobPtr, sizeof(HobPtr), 0x0);
  HobPtr.Guid   = GetFirstGuidHob (&gEfiBootMediaHobGuid);
  ASSERT (HobPtr.Guid != NULL);
  BootMediaData = (MBP_CURRENT_BOOT_MEDIA*) GET_GUID_HOB_DATA (HobPtr.Guid);

  if (BootMediaData->PhysicalData == 0xFFFFFFFF) {  //Unknown boot device
    return EFI_DEVICE_ERROR;
  } else {
    *BootDevice = BootMediaData->PhysicalData;
  }

  return EFI_SUCCESS;
}


//
// eMMC Device Read/Write operation
//
EFI_STATUS
EFIAPI
EmmcReadBlocksPei (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN UINT8                     SelBpNum,
  IN UINTN                     FlashAddress,
  IN UINTN                     BufferSize,
  OUT VOID                     *Buffer
  )
{
  EFI_STATUS                            Status;
  UINTN                                 Size;
  EFI_PHYSICAL_ADDRESS                  Address;
  UINT8                                 *BufferPointer;
  UINTN                                 BlockIoPpiInstance;
  EFI_PEI_RECOVERY_BLOCK_IO_PPI         *BlockIoPpi;
  EFI_PEI_BLOCK_IO_MEDIA                MediaInfo;
  PEI_MMC_PARTITION_DATA                *Partition;
  UINT8                                 PartitionNum;
  EFI_LBA                               LBA;
  UINTN                                 Remainder;
  UINT8                                 *PreData = NULL;
  UINT8                                 *Data;

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0) {
    return EFI_SUCCESS;
  }

  if ((SelBpNum > 2) || (BufferSize > IFWI_SINGLE_SIZE)) {
    Status = EFI_INVALID_PARAMETER;
    DEBUG((EFI_D_ERROR, "Invalid Parameter. Status = %r\n", Status));
    return Status;
  }

  Status = EFI_ABORTED;

  BufferPointer = Buffer;

  for (BlockIoPpiInstance = 0; BlockIoPpiInstance < MAX_NUMBER_OF_PARTITIONS; BlockIoPpiInstance++) {
    Status = (*PeiServices)->LocatePpi (
                               PeiServices,
                               &gEfiPeiVirtualBlockIoPpiGuid,
                               BlockIoPpiInstance,
                               NULL,
                               (VOID **) &BlockIoPpi
                               );
    if (EFI_ERROR (Status)) {
      //
      // Done with all Block Io Ppis
      //
      continue;
    }

    //
    // Check whether it is a eMMC
    //
    Status = BlockIoPpi->GetBlockDeviceMediaInfo (
                           (EFI_PEI_SERVICES**)PeiServices,
                           BlockIoPpi,
                           0,
                           &MediaInfo
                           );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Fail to get MediaInfo. Status = %r\n", Status));
      goto ErrorExit;
    }
    DEBUG ((EFI_D_INFO, "DeviceType: 0x%x, BlockSize: 0x%x.\n", MediaInfo.DeviceType, MediaInfo.BlockSize));
    if (MediaInfo.DeviceType != eMMCDevice) {
      //
      // It is not a eMMC
      //
      continue;
    }

    Partition    = PEI_CARD_PARTITION_DATA_FROM_THIS (BlockIoPpi);
    PartitionNum = (UINT8)PEI_CARD_DATA_PARTITION_NUM (Partition);

    //
    // Read boot partition 1 and compare
    //
    DEBUG((EFI_D_INFO, "Reading boot partition %d.\n", SelBpNum));
    if (SelBpNum == PartitionNum) {
      if ((BufferSize % MediaInfo.BlockSize) != 0) {
        Status = EFI_BAD_BUFFER_SIZE;
        DEBUG ((EFI_D_ERROR, "Bad buffer size. eMMC block size is: 0x%x\n", MediaInfo.BlockSize));
        goto ErrorExit;
      }

      Remainder = FlashAddress % MediaInfo.BlockSize;
      if (Remainder) {
        DEBUG((EFI_D_ERROR, "Start address is not aligned and process it now.\n"));
        BufferSize += MediaInfo.BlockSize;
      }

      Size = BufferSize;
      Status = (*PeiServices)->AllocatePages (
                                 PeiServices,
                                 EfiBootServicesCode,
                                 (Size - 1) / 0x1000 + 1,
                                 &Address
                                 );
      PreData = (UINT8 *) (UINTN) Address;
      Data = PreData;
      if ((EFI_ERROR(Status)) || (PreData == NULL)) {
        DEBUG((EFI_D_ERROR, "Failed to allocate memory for reading region.\n"));
        Status = EFI_BAD_BUFFER_SIZE;
        goto ErrorExit;
      }

      LBA = DivU64x32((UINT64)FlashAddress, MediaInfo.BlockSize);
      DEBUG((EFI_D_INFO, "Boot Partition = 0x%X, Size = 0x%X, StartAddress = 0x%X, LBA = 0x%X.\n", SelBpNum, BufferSize, FlashAddress, LBA));

      Status = BlockIoPpi->ReadBlocks (
                             (EFI_PEI_SERVICES**)PeiServices,
                             BlockIoPpi,
                             0,
                             LBA,
                             BufferSize,
                             (VOID*)Data
                             );
      if (EFI_ERROR (Status)) {
        DEBUG((EFI_D_ERROR, "Read boot partition %d failed, Status = %r\n", SelBpNum, Status));
        goto ErrorExit;
      } else {
        DEBUG((EFI_D_INFO, "Read boot partition %d successfully, Status = %r\n", SelBpNum, Status));
        if (Remainder) {
          CopyMem(BufferPointer, (UINT8*)(Data + Remainder), BufferSize - MediaInfo.BlockSize);
        } else {
          CopyMem(BufferPointer, Data, BufferSize);
        }
        Status = EFI_SUCCESS;
        break;
      }
    }
  }

  if (BlockIoPpiInstance == MAX_NUMBER_OF_PARTITIONS) {
    DEBUG((EFI_D_ERROR, "FotaRecoveryEntry: Failed to recovery BIOS.\n"));
    Status = EFI_NO_MEDIA;
    goto ErrorExit;
  }

ErrorExit:

  if (PreData != NULL) {
    FreePool (PreData);
  }
  return Status;
}

EFI_STATUS
EFIAPI
EmmcWriteBlocksPei (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN UINT8                     SelBpNum,
  IN UINTN                     FlashAddress,
  IN UINTN                     BufferSize,
  IN VOID                      *Buffer
  )
{
  EFI_STATUS                            Status;
  UINTN                                 BlockIoPpiInstance;
  EFI_PEI_RECOVERY_BLOCK_IO_PPI         *BlockIoPpi;
  EFI_PEI_BLOCK_IO_MEDIA                MediaInfo;
  PEI_MMC_PARTITION_DATA                *Partition;
  UINT8                                 PartitionNum;
  EFI_LBA                               LBA;
  UINTN                                 Remainder;

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0) {
    return EFI_SUCCESS;
  }

  if ((SelBpNum > 2) || (BufferSize > IFWI_SINGLE_SIZE)) {
    Status = EFI_INVALID_PARAMETER;
    DEBUG((EFI_D_ERROR, "Invalid Parameter. Status = %r\n", Status));
    return Status;
  }

  Status = EFI_ABORTED;

  for (BlockIoPpiInstance = 0; BlockIoPpiInstance < MAX_NUMBER_OF_PARTITIONS; BlockIoPpiInstance++) {
    Status = (*PeiServices)->LocatePpi (
                               PeiServices,
                               &gEfiPeiVirtualBlockIoPpiGuid,
                               BlockIoPpiInstance,
                               NULL,
                               (VOID **) &BlockIoPpi
                               );
    if (EFI_ERROR (Status)) {
      //
      // Done with all Block Io Ppis
      //
      continue;
    }

    //
    // Check whether it is a eMMC
    //
    BlockIoPpi->GetBlockDeviceMediaInfo (
                  (EFI_PEI_SERVICES**)PeiServices,
                  BlockIoPpi,
                  0,
                  &MediaInfo
                  );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Fail to get MediaInfo. Status = %r\n", Status));
      goto ErrorExit;
    }
    DEBUG ((EFI_D_INFO, "DeviceType: 0x%x, BlockSize: 0x%x.\n", MediaInfo.DeviceType, MediaInfo.BlockSize));
    if (MediaInfo.DeviceType != eMMCDevice) {
      //
      // It is not a eMMC
      //
      continue;
    }

    Partition    = PEI_CARD_PARTITION_DATA_FROM_THIS (BlockIoPpi);
    PartitionNum = (UINT8)PEI_CARD_DATA_PARTITION_NUM (Partition);

    //
    // Read boot partition 1 and compare
    //
    DEBUG((EFI_D_INFO, "Writting boot partition %d.\n", SelBpNum));
    if (SelBpNum == PartitionNum) {
      if ((BufferSize % MediaInfo.BlockSize) != 0) {
        Status = EFI_BAD_BUFFER_SIZE;
        DEBUG ((EFI_D_ERROR, "Bad buffer size. eMMC block size is: 0x%x\n", MediaInfo.BlockSize));
        goto ErrorExit;
      }

      Remainder = FlashAddress % MediaInfo.BlockSize;
      if (Remainder) {
        Status = EFI_BAD_BUFFER_SIZE;
        DEBUG((EFI_D_ERROR, "Bad start address. Start address is: 0x%x\n", FlashAddress));
        goto ErrorExit;
      }

      LBA = DivU64x32((UINT64)FlashAddress, MediaInfo.BlockSize);
      DEBUG((EFI_D_INFO, "Boot Partition = 0x%X, Size = 0x%X, StartAddress = 0x%X, LBA = 0x%X.\n", SelBpNum, BufferSize, FlashAddress, LBA));

      Status = BlockIoPpi->WriteBlocks (
                             (EFI_PEI_SERVICES**)PeiServices,
                             BlockIoPpi,
                             0,
                             LBA,
                             BufferSize,
                             (VOID*)Buffer
                             );
      if (EFI_ERROR (Status)) {
        DEBUG((EFI_D_ERROR, "Write boot partition %d failed, Status = %r\n", SelBpNum, Status));
        goto ErrorExit;
      } else {
        DEBUG((EFI_D_INFO, "Write boot partition %d successfully, Status = %r\n", SelBpNum, Status));
        Status = EFI_SUCCESS;
        break;
      }
    }
  }

  if (BlockIoPpiInstance == MAX_NUMBER_OF_PARTITIONS) {
    DEBUG((EFI_D_ERROR, "FotaRecoveryEntry: Failed to recovery BIOS.\n"));
    Status = EFI_NO_MEDIA;
    goto ErrorExit;
  }

ErrorExit:

  return Status;
}

//
// UFS Device Read/Write operation
//
EFI_STATUS
EFIAPI
UfsReadBlocksPei (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN UINT8                     SelBpNum,
  IN UINTN                     FlashAddress,
  IN UINTN                     BufferSize,
  OUT VOID                     *Buffer
  )
{

  EFI_STATUS                            Status;
  UINTN                                 Size;
  EFI_PHYSICAL_ADDRESS                  Address;
  UINT8                                 *BufferPointer;
  UINTN                                 BlockIoPpiInstance;
  EFI_PEI_RECOVERY_BLOCK_IO_PPI         *BlockIoPpi;
  EFI_PEI_BLOCK_IO_MEDIA                MediaInfo;
  EFI_LBA                               LBA;
  UINTN                                 Remainder;
  UINT8                                 *PreData = NULL;
  UINT8                                 *Data;

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0) {
    return EFI_SUCCESS;
  }

  if (BufferSize > IFWI_SINGLE_SIZE) {
    Status = EFI_INVALID_PARAMETER;
    DEBUG((EFI_D_ERROR, "Invalid Parameter. Status = %r\n", Status));
    return Status;
  }

  Status = EFI_ABORTED;

  BufferPointer = Buffer;

  for (BlockIoPpiInstance = 0; BlockIoPpiInstance < MAX_NUMBER_OF_PARTITIONS; BlockIoPpiInstance++) {
    Status = (*PeiServices)->LocatePpi (
                               PeiServices,
                               &gEfiPeiVirtualBlockIoPpiGuid,
                               BlockIoPpiInstance,
                               NULL,
                               (VOID **) &BlockIoPpi
                               );
    if (EFI_ERROR (Status)) {
      //
      // Done with all Block Io Ppis
      //
      continue;
    }

    //
    // Check whether it is a UFS
    //
    Status = BlockIoPpi->GetBlockDeviceMediaInfo (
                           (EFI_PEI_SERVICES**)PeiServices,
                           BlockIoPpi,
                           SelBpNum,
                           &MediaInfo
                           );
    if (EFI_ERROR (Status) || !MediaInfo.MediaPresent) {
      DEBUG ((EFI_D_ERROR, "Fail to get MediaInfo. Status = %r\n", Status));
      goto ErrorExit;
    }
    DEBUG ((EFI_D_INFO, "DeviceType: 0x%x, BlockSize: 0x%x.\n", MediaInfo.DeviceType, MediaInfo.BlockSize));

    if (MediaInfo.DeviceType != UfsDevice) {
      //
      // It is not a UFS
      //
      continue;
    }


    //
    // Read boot partition 1 and compare
    //
    DEBUG((EFI_D_INFO, "Reading boot partition %d.\n", SelBpNum+1));

    if ((BufferSize % MediaInfo.BlockSize) != 0) {
      Status = EFI_BAD_BUFFER_SIZE;
      DEBUG ((EFI_D_ERROR, "Bad buffer size. UFS block size is: 0x%x\n", MediaInfo.BlockSize));
      goto ErrorExit;
    }

    Remainder = FlashAddress % MediaInfo.BlockSize;
    if (Remainder) {
      DEBUG((EFI_D_ERROR, "Start address is not aligned and process it now.\n"));
      BufferSize += MediaInfo.BlockSize;
    }

    Size = BufferSize;
    Status = (*PeiServices)->AllocatePages (
                               PeiServices,
                               EfiBootServicesCode,
                               (Size - 1) / 0x1000 + 1,
                               &Address
                               );
    PreData = (UINT8 *) (UINTN) Address;
    Data = PreData;
    if ((EFI_ERROR(Status)) || (PreData == NULL)) {
      DEBUG((EFI_D_ERROR, "Failed to allocate memory for reading region.\n"));
      Status = EFI_BAD_BUFFER_SIZE;
      goto ErrorExit;
    }

    LBA = DivU64x32((UINT64)FlashAddress, MediaInfo.BlockSize);
    DEBUG((EFI_D_INFO, "Boot Partition = 0x%X, Size = 0x%X, StartAddress = 0x%X, LBA = 0x%X.\n", SelBpNum, BufferSize, FlashAddress, LBA));

    Status = BlockIoPpi->ReadBlocks (
                           (EFI_PEI_SERVICES**)PeiServices,
                           BlockIoPpi,
                           SelBpNum,
                           LBA,
                           BufferSize,
                           (VOID*)Data
                           );
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "Read boot partition %d failed, Status = %r\n", SelBpNum, Status));
        goto ErrorExit;
    } else {
      DEBUG((EFI_D_INFO, "Read boot partition %d successfully, Status = %r\n", SelBpNum, Status));
      if (Remainder) {
          CopyMem(BufferPointer, (UINT8*)(Data + Remainder), BufferSize - MediaInfo.BlockSize);
      } else {
          CopyMem(BufferPointer, Data, BufferSize);
      }
      Status = EFI_SUCCESS;
      break;
    }
  }

  if (BlockIoPpiInstance == MAX_NUMBER_OF_PARTITIONS) {
    DEBUG((EFI_D_ERROR, "FotaRecoveryEntry: Failed to recovery BIOS.\n"));
    Status = EFI_NO_MEDIA;
    goto ErrorExit;
  }

ErrorExit:

  if (PreData != NULL) {
    FreePool (PreData);
  }
  return Status;
}

EFI_STATUS
EFIAPI
UfsWriteBlocksPei (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN UINT8                     SelBpNum,
  IN UINTN                     FlashAddress,
  IN UINTN                     BufferSize,
  IN VOID                      *Buffer
  )
{

  EFI_STATUS                            Status;
  UINTN                                 BlockIoPpiInstance;
  EFI_PEI_RECOVERY_BLOCK_IO_PPI         *BlockIoPpi;
  EFI_PEI_BLOCK_IO_MEDIA                MediaInfo;
  EFI_LBA                               LBA;
  UINTN                                 Remainder;

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0) {
    return EFI_SUCCESS;
  }

  if (BufferSize > IFWI_SINGLE_SIZE) {
    Status = EFI_INVALID_PARAMETER;
    DEBUG((EFI_D_ERROR, "Invalid Parameter. Status = %r\n", Status));
    return Status;
  }

  Status = EFI_ABORTED;

  for (BlockIoPpiInstance = 0; BlockIoPpiInstance < MAX_NUMBER_OF_PARTITIONS; BlockIoPpiInstance++) {
    Status = (*PeiServices)->LocatePpi (
                               PeiServices,
                               &gEfiPeiVirtualBlockIoPpiGuid,
                               BlockIoPpiInstance,
                               NULL,
                               (VOID **) &BlockIoPpi
                               );
    if (EFI_ERROR (Status)) {
      //
      // Done with all Block Io Ppis
      //
      continue;
    }

    //
    // Check whether it is a UFS
    //
    Status = BlockIoPpi->GetBlockDeviceMediaInfo (
                           (EFI_PEI_SERVICES**)PeiServices,
                           BlockIoPpi,
                           SelBpNum,
                           &MediaInfo
                           );
    if (EFI_ERROR (Status) || !MediaInfo.MediaPresent) {
      DEBUG ((EFI_D_ERROR, "Fail to get MediaInfo. Status = %r\n", Status));
      goto ErrorExit;
    }
    DEBUG ((EFI_D_INFO, "DeviceType: 0x%x, BlockSize: 0x%x.\n", MediaInfo.DeviceType, MediaInfo.BlockSize));

    if (MediaInfo.DeviceType != UfsDevice) {
      //
      // It is not a UFS
      //
      continue;
    }


    //
    // Read boot partition 1 and compare
    //
    DEBUG((EFI_D_INFO, "Writting boot partition %d.\n", SelBpNum));

    if ((BufferSize % MediaInfo.BlockSize) != 0) {
      Status = EFI_BAD_BUFFER_SIZE;
      DEBUG ((EFI_D_ERROR, "Bad buffer size. UFS block size is: 0x%x\n", MediaInfo.BlockSize));
      goto ErrorExit;
    }

    Remainder = FlashAddress % MediaInfo.BlockSize;
    if (Remainder) {
      Status = EFI_BAD_BUFFER_SIZE;
      DEBUG((EFI_D_ERROR, "Bad start address. Start address is: 0x%x\n", FlashAddress));
      goto ErrorExit;
    }

    LBA = DivU64x32((UINT64)FlashAddress, MediaInfo.BlockSize);
    DEBUG((EFI_D_INFO, "Boot Partition = 0x%X, Size = 0x%X, StartAddress = 0x%X, LBA = 0x%X.\n", SelBpNum, BufferSize, FlashAddress, LBA));

    Status = BlockIoPpi->WriteBlocks (
                           (EFI_PEI_SERVICES**)PeiServices,
                           BlockIoPpi,
                           SelBpNum,
                           LBA,
                           BufferSize,
                           (VOID*)Buffer
                           );
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "Read boot partition %d failed, Status = %r\n", SelBpNum, Status));
        goto ErrorExit;
    } else {
      DEBUG((EFI_D_INFO, "Read boot partition %d successfully, Status = %r\n", SelBpNum, Status));
      Status = EFI_SUCCESS;
      break;
    }
  }

  if (BlockIoPpiInstance == MAX_NUMBER_OF_PARTITIONS) {
    DEBUG((EFI_D_ERROR, "FotaRecoveryEntry: Failed to recovery BIOS.\n"));
    Status = EFI_NO_MEDIA;
    goto ErrorExit;
  }

ErrorExit:

  return Status;
}

/**
  Reads the requested size of data from the SPI boot device.

  The function reads the requested number of blocks from the device. All the
  blocks are read, or an error is returned.

  @param[in]  SpiRegionType       - The SPI Region type for flash cycle which is listed in the Descriptor
  @param[in]  Address             - The starting address to read from on the device
  @param[in]  BufferSize          - The size of the Buffer in bytes.
  @param[out] Buffer              - A pointer to the destination buffer for the data.

  @retval EFI_SUCCESS             - Command succeed.
  @retval EFI_INVALID_PARAMETER   - The parameters specified are not valid.
  @retval EFI_DEVICE_ERROR        - Device error, command aborts abnormally.
**/
EFI_STATUS
SpiReadBlocks (
  IN CONST EFI_PEI_SERVICES	 **PeiServices,
  IN UINT8                     SpiRegionType,
  IN UINTN                     Address,
  IN UINTN                     BufferSize,
  OUT VOID                     *Buffer
  )
{
  EFI_STATUS            Status = EFI_SUCCESS;
  UINT32                SectorSize;
  UINT32                SpiAddress;
  UINTN                 NumBytes = 0;

  SpiAddress = (UINT32)(UINTN)(Address);
  SectorSize = SECTOR_SIZE_4KB;

  //
  // Calculate Boot partition #2 physical address
  // FLA[26:0] <= (Flash_Regionn_Limit) - (FFFF_FFFCh - bios_address)
  //
  SpiAddress = 0xFFFFFFFC - (GetSpiFlashRegionLimit (BIOS) + 0xFFC - Address);
  
  while (NumBytes < BufferSize) {
    CopyMem ((VOID*)((UINT8*)Buffer + NumBytes), (VOID *)SpiAddress, SectorSize);
    SpiAddress += SectorSize;
    NumBytes += SectorSize;

  }
  return Status;
}

/**
  Writes the requested size of data to the SPI boot device.

  The function reads the requested number of blocks from the device. All the
  blocks are written, or an error is returned.

  @param[in]  SpiRegionType       - The SPI Region type for flash cycle which is listed in the Descriptor
  @param[in]  Address             - The starting address to write to on the device
  @param[in]  BufferSize          - The size of the Buffer in bytes.
  @param[in]  Buffer              - A pointer to the destination buffer for the data.

  @retval EFI_SUCCESS             - Command succeed.
  @retval EFI_INVALID_PARAMETER   - The parameters specified are not valid.
  @retval EFI_DEVICE_ERROR        - Device error, command aborts abnormally.
**/
EFI_STATUS
SpiWriteBlocks (
  IN CONST EFI_PEI_SERVICES  **PeiServices,
  IN UINT8                     SpiRegionType,
  IN UINTN                     Address,
  IN UINTN                     BufferSize,
  IN VOID                      *Buffer
  )
{
  
  return EFI_SUCCESS;
}

//
// Create two talbes for eMMC and UFS update
//
FOTA_BLOCK_READ_WRITE_TABLE_PEI FotaEmmcBlockTablePei = {
  EmmcReadBlocksPei,
  EmmcWriteBlocksPei
};

FOTA_BLOCK_READ_WRITE_TABLE_PEI FotaUfsBlockTablePei = {
  UfsReadBlocksPei,
  UfsWriteBlocksPei
};


FOTA_BLOCK_READ_WRITE_TABLE_PEI FotaSpiBlockTablePei = {
  SpiReadBlocks,
  SpiWriteBlocks
};


//
// This routine attempts to get Manifest Hash in BP1 and compare with OBB data in BP2.
//
EFI_STATUS
EFIAPI
VerifyOBBFvInBP(
  IN  CONST EFI_PEI_SERVICES    **PeiServices,
  IN  UINT8                     *Bp1BpdtHdrPtr,
  IN  UINT8                     *Bp2BpdtHdrPtr,
  OUT BOOLEAN                   *OBBVerifyFlag
  )
{
  EFI_STATUS                            Status = EFI_SUCCESS;
  EFI_PHYSICAL_ADDRESS                  Address;
  UINT32                                Size;

  UINT32                                BpIbbOffset;      //For BP1 BPDT IBB
  UINT32                                BpIbbSize;
  UINT8                                 *H1BpIbbPtr = NULL;
  UINT32                                BpBpmHashOffset;  //BPM(Boot Policy Manifest) Hash offset
  UINT32                                BpBpmHashLength;
  UINT32                                BpSbpdtOffset;    //BP2 BPDT SBPDT
  UINT32                                BpSbpdtSize;
  UINT8                                 *H2BpSbptPtr = NULL; //BP2 SBPDT buffer
  UINT32                                BpObbOffset;      //BP1 BPDT OBB offset (including IBBR, OBB, OBBX and NvStorage)
  UINT32                                BpObbSize;
  UINT8                                 *H2BpObbPtr = NULL;
  UINT32                                IBBRFvOffset = 0;
  UINT32                                IBBRFvLength = 0;
  UINT32                                OBBFvOffset = 0;
  UINT32                                OBBFvLength = 0;
  UINT32                                OBBXFvOffset = 0;
  UINT32                                OBBXFvLength = 0;
  UINT32                                OBBYFvOffset = 0;
  UINT32                                OBBYFvLength = 0;
  UINT32                                NVSFvOffset = 0;
  UINT32                                NVSFvLength = 0;
  UINT8                                 *OBBFvPtr = NULL;
  UINTN                                 HashObbBuffer;

  //
  // eMMC/UFS/SPI read/write handler
  //
  UINT32                                BootDevice = 0;
  UINT8                                 BootPartition1 = 0;
  UINT8                                 BootPartition2 = 0;
  UINT32                                BiosFlashStartAddress = 0;
  UINT32                                BP2FlashLinearAddress = 0;
  SPI_REGION_TYPE                       SpiRegionType;  
  FOTA_BLOCK_READ_WRITE_TABLE_PEI       *FotaBlockPtr;

  //
  // UFS boot partition checking
  //
  UINT8                                 Index;
  EFI_PEI_HOB_POINTERS                  HobPtr;
  UFS_CONFIG_DESC                       *Config = NULL;

  SetMem(&HobPtr, sizeof(HobPtr), 0x0);

  //
  // Get BIOS boot device
  //
  Status = PeiGetBootDevice(&BootDevice);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can not get BIOS boot device.\n"));
    return Status;
  }

  if (BootDevice == BOOT_FROM_EMMC) {
    BootPartition1 = EMMC_BOOT_PARTITION_1;
    BootPartition2 = EMMC_BOOT_PARTITION_2;
    FotaBlockPtr = &FotaEmmcBlockTablePei;
    DEBUG ((EFI_D_INFO, "Boot from eMMC device.\n"));
  } else if (BootDevice == BOOT_FROM_UFS) {
    //
    // Get UFS Configuration Descriptor Hob
    // Scan all UFS partition to get Boot LU A and Boot LU B
    //
    HobPtr.Guid   = GetFirstGuidHob (&gUfsBootLunIdHobGuid);
    ASSERT (HobPtr.Guid != NULL);
    Config = (UFS_CONFIG_DESC*) GET_GUID_HOB_DATA (HobPtr.Guid);

    for (Index = 0; Index < 8; Index++) {
      if (Config->UnitDescConfParams[Index].BootLunId == 0x1) {  //Boot LU A
        BootPartition1 = Index;
        DEBUG ((EFI_D_INFO, "UFS Boot Partition 1 is: %x\n", BootPartition1));
      }
      if (Config->UnitDescConfParams[Index].BootLunId == 0x2) {  //Boot LU B
        BootPartition2 = Index;
        DEBUG ((EFI_D_INFO, "UFS Boot Partition 2 is: %x\n", BootPartition2));
      }
    }
    FotaBlockPtr = &FotaUfsBlockTablePei;
    DEBUG ((EFI_D_INFO, "Boot from UFS device.\n"));
  } else if(BootDevice == BOOT_FROM_SPI){
    DEBUG ((EFI_D_INFO, "Boot from SPI device.\n"));
	SpiRegionType = FlashRegionAll;
	BiosFlashStartAddress = GetSpiFlashRegionBase(BIOS);
    BP2FlashLinearAddress = (BiosFlashStartAddress + GetSpiFlashRegionBase (DeviceExpansion1)) / 2;
    DEBUG ((EFI_D_INFO, "BiosFlashStartAddress: 0x%X, BP2FlashLinearAddress: 0x%X\n", BiosFlashStartAddress, BP2FlashLinearAddress));
    FotaBlockPtr = &FotaSpiBlockTablePei;
  }else {
    DEBUG ((EFI_D_ERROR, "Unsupported boot device.\n"));
    return EFI_UNSUPPORTED;
  }

  //
  // Checking OBB checksum and compare with Manifest in BP1 if both of BP1 and BP2 signature are valid.
  //
  *OBBVerifyFlag = TRUE;

  DEBUG((EFI_D_INFO, "Finding Manifest region.\n"));
  
  Size = 0x1000;
  Status = (*PeiServices)->AllocatePages (
                             PeiServices,
                             EfiBootServicesCode,
                             (Size - 1) / 0x1000 + 1,
                             &Address
                             );
  H1BpIbbPtr = (UINT8 *) (UINTN) Address;
  
  if ((EFI_ERROR(Status)) || (H1BpIbbPtr == NULL)) {
    DEBUG((EFI_D_ERROR, "Failed to allocate memory for reading Hash region.\n"));
    Status = EFI_BAD_BUFFER_SIZE;
    goto ErrorExit;
  }

  // From "$CPD" signature
  
  Status = ProcessBpdtHdr(Bp1BpdtHdrPtr, bpIBB, &BpIbbOffset, &BpIbbSize);
  if (EFI_ERROR(Status)) {
    *OBBVerifyFlag = FALSE;
    DEBUG((EFI_D_ERROR, "Read BpIbb region failed, Status = %r\n", Status));
    goto ErrorExit;
  }
  DEBUG((EFI_D_INFO, "BpIbbOffset = 0x%X, BpIbbSize = 0x%X.\n", BpIbbOffset, BpIbbSize));
   
  if (BootDevice != BOOT_FROM_SPI){
    Status = FotaBlockPtr->ProcessReadBlock(PeiServices, BootPartition1, BpIbbOffset, Size, H1BpIbbPtr);
  } else if (BootDevice == BOOT_FROM_SPI){
    Status = FotaBlockPtr->ProcessReadBlock(PeiServices, SpiRegionType, (BpIbbOffset + SPI_FD_SIZE ), Size, H1BpIbbPtr);
  }
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Read H1 BpIbb failed, Status = %r\n", Status));
    goto ErrorExit;
  } else {
    DEBUG((EFI_D_INFO, "Read H1 BpIbb successfully.\n"));
    DEBUG((EFI_D_INFO, "H1 BpIbb content = 0x%x\n",*(UINT32 *)H1BpIbbPtr));
  }
    
  //
  // BPDT (IBB partition type)
  // Get Manifest hash Offset from Boot Partiton 1.
  // IBBL: Offset + 0x0C + 0x8
  // IBBM: Offset + 0x0C + 0x8*2 + 0x20
  // OBB:  Offset + 0x0C + 0x8*3 + 0x20*2
  //
  Status = GetParDirOffset(H1BpIbbPtr, "BPM.met", &BpBpmHashOffset, &BpBpmHashLength);
  DEBUG((EFI_D_INFO, "H1 BPM Offset: 0x%X.\n", BpBpmHashOffset));
  if (EFI_ERROR(Status)) {
    *OBBVerifyFlag = FALSE;
    DEBUG((EFI_D_ERROR, "Can not find Manifest BPM Hash Offset, Status = %r\n", Status));
    goto ErrorExit;
  }
    
  //
  // Analyze BP2 BPDT to get SBPDT, then get OBB offset from SBPDT.
  //
  DEBUG((EFI_D_INFO, "Finding H2 SBPDT to get OBB region.\n"));
  Status = ProcessBpdtHdr(Bp2BpdtHdrPtr, bpSBpdt, &BpSbpdtOffset, &BpSbpdtSize);
  if (EFI_ERROR(Status)) {
    *OBBVerifyFlag = FALSE;
    DEBUG((EFI_D_ERROR, "Read bpSBpdt region failed, Status = %r\n", Status));
    goto ErrorExit;
  }
  DEBUG((EFI_D_INFO, "BpSbpdtOffset = 0x%X, BpSbpdtSize = 0x%X.\n", BpSbpdtOffset, BpSbpdtSize));

  // Analyze SBPDT region and get OBB offset
  // Read SBPDT table date
  Size = 0x1000;
  Status = (*PeiServices)->AllocatePages (
                             PeiServices,
                             EfiBootServicesCode,
                             (Size - 1) / 0x1000 + 1,
                             &Address
                             );
  H2BpSbptPtr = (UINT8 *) (UINTN) Address;
  if ((EFI_ERROR(Status)) || (H2BpSbptPtr == NULL)) {
    DEBUG((EFI_D_ERROR, "Failed to allocate memory for reading H2 BpSbpdt region.\n"));
    Status = EFI_BAD_BUFFER_SIZE;
    goto ErrorExit;
  }
  if (BootDevice != BOOT_FROM_SPI){
    Status = FotaBlockPtr->ProcessReadBlock(PeiServices, BootPartition2, BpSbpdtOffset, Size, H2BpSbptPtr);
  } else if (BootDevice == BOOT_FROM_SPI) {
    Status = FotaBlockPtr->ProcessReadBlock(PeiServices, SpiRegionType, (BP2FlashLinearAddress + BpSbpdtOffset), Size, H2BpSbptPtr);
  }
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Read H2 BpSBpdt failed, Status = %r\n", Status));
    goto ErrorExit;
  } else {
    DEBUG((EFI_D_INFO, "Read H2 BpSBpdt successfully.\n"));
	DEBUG((EFI_D_INFO, "H2 BpSBpdt content = 0x%x\n",*(UINT32 *)H2BpSbptPtr));
  }

  //Get OBB offset from SBPDT Table. (SBPDT big OBB Offset including IBBR, OBB, OBBX and NvStorage regions, we should get OBB FV data)
  Status = ProcessBpdtHdr(H2BpSbptPtr, bpObb, &BpObbOffset, &BpObbSize);
  if (EFI_ERROR(Status)) {
    *OBBVerifyFlag = FALSE;
    DEBUG((EFI_D_ERROR, "Read BpObb region failed, Status = %r\n", Status));
    goto ErrorExit;
  }
  DEBUG((EFI_D_INFO, "BpObbOffset = 0x%X, BpObbSize = 0x%X.\n", BpObbOffset, BpObbSize));

  //
  // Read OBB Partition directory date
  //
  Size = 0x1000;
  Status = (*PeiServices)->AllocatePages (
                             PeiServices,
                             EfiBootServicesCode,
                             (Size - 1) / 0x1000 + 1,
                             &Address
                             );
  H2BpObbPtr = (UINT8 *) (UINTN) Address;
  if ((EFI_ERROR(Status)) || (H2BpObbPtr == NULL)) {
    DEBUG((EFI_D_ERROR, "Failed to allocate memory for reading H2 BpObb region.\n"));
    Status = EFI_BAD_BUFFER_SIZE;
    goto ErrorExit;
  }
  if (BootDevice != BOOT_FROM_SPI){
    Status = FotaBlockPtr->ProcessReadBlock(PeiServices, BootPartition2, BpObbOffset, Size, H2BpObbPtr);
  } else if (BootDevice == BOOT_FROM_SPI) {
    Status = FotaBlockPtr->ProcessReadBlock(PeiServices, SpiRegionType, (BP2FlashLinearAddress + BpObbOffset), Size, H2BpObbPtr);
  }
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Read H2 BpObb failed, Status = %r\n", Status));
    goto ErrorExit;
  } else {
    DEBUG((EFI_D_INFO, "Read H2 BpObb successfully.\n"));
	DEBUG((EFI_D_INFO, "H2 BpObb content = 0x%x\n",*(UINT32 *)H2BpObbPtr));
  }

  // Analyze "OBB" partition and get IBBR FV info
  Status = GetParDirOffset(H2BpObbPtr, "IBBR", &IBBRFvOffset, &IBBRFvLength);
  if (EFI_ERROR(Status)) {
    *OBBVerifyFlag = FALSE;
    DEBUG((EFI_D_ERROR, "Can not find H2 IBBR partition, Status = %r\n", Status));
    goto ErrorExit;
  }
  DEBUG((EFI_D_INFO, "IBBRFvOffset = 0x%X, Length = 0x%X.\n", IBBRFvOffset, IBBRFvLength));

  // Analyze "OBB" partition and get OBB FV info
  Status = GetParDirOffset(H2BpObbPtr, "OBB", &OBBFvOffset, &OBBFvLength);
  if (EFI_ERROR(Status)) {
    *OBBVerifyFlag = FALSE;
    DEBUG((EFI_D_ERROR, "Can not find H2 OBB partition, Status = %r\n", Status));
    goto ErrorExit;
  }
  DEBUG((EFI_D_INFO, "OBBFvOffset = 0x%X, Length = 0x%X.\n", OBBFvOffset, OBBFvLength));

  // Analyze "OBB" partition and get OBBX FV info
  Status = GetParDirOffset(H2BpObbPtr, "OBBX", &OBBXFvOffset, &OBBXFvLength);
  if (EFI_ERROR(Status)) {
    *OBBVerifyFlag = FALSE;
    DEBUG((EFI_D_ERROR, "Can not find H2 OBBX partition, Status = %r\n", Status));
    goto ErrorExit;
  }
  DEBUG((EFI_D_INFO, "OBBXFvOffset = 0x%X, Length = 0x%X.\n", OBBXFvOffset, OBBXFvLength));

  Status = GetParDirOffset(H2BpObbPtr, "OBBY", &OBBYFvOffset, &OBBYFvLength);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can not find H2 OBBY partition-Its optional for BXTM, Status = %r\n", Status));
  }
  DEBUG((EFI_D_INFO, "OBBXFvOffset = 0x%X, Length = 0x%X.\n", OBBYFvOffset, OBBYFvLength));
  Status = GetParDirOffset(H2BpObbPtr, "NvStorage", &NVSFvOffset, &NVSFvLength);
  if (EFI_ERROR(Status)) {
    *OBBVerifyFlag = FALSE;
    DEBUG((EFI_D_ERROR, "Can not find H2 OBBX partition, Status = %r\n", Status));
    goto ErrorExit;
  }
  DEBUG((EFI_D_INFO, "NVSFvOffset = 0x%X, Length = 0x%X.\n", NVSFvOffset, NVSFvLength));
  // Read OBB FV data
  Size = IBBRFvLength + OBBFvLength + OBBXFvLength+OBBYFvLength+NVSFvLength;
  Status = (*PeiServices)->AllocatePages (
                             PeiServices,
                             EfiBootServicesCode,
                             (Size - 1) / 0x1000 + 1,
                             &Address
                             );
  OBBFvPtr = (UINT8 *) (UINTN) Address;
  if ((EFI_ERROR(Status)) || (OBBFvPtr == NULL)) {
    DEBUG((EFI_D_ERROR, "Failed to allocate memory for reading real OBB FV.\n"));
    Status = EFI_BAD_BUFFER_SIZE;
    goto ErrorExit;
  }

  IBBRFvOffset += BpObbOffset;  //IBBR FV offset relative to BPDT_OBB offset

  if (BootDevice != BOOT_FROM_SPI){
    Status = FotaBlockPtr->ProcessReadBlock(PeiServices, BootPartition2, IBBRFvOffset, Size, OBBFvPtr);
  } else if (BootDevice == BOOT_FROM_SPI) {
    Status = FotaBlockPtr->ProcessReadBlock(PeiServices, SpiRegionType, (BP2FlashLinearAddress + IBBRFvOffset), Size, OBBFvPtr);
  }
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Read OBB FV failed, Status = %r\n", Status));
    goto ErrorExit;
  } else {
    DEBUG((EFI_D_INFO, "Read OBB FV successfully.\n"));
    DEBUG((EFI_D_INFO, "OBB FV content = 0x%x\n",*(UINT32 *)OBBFvPtr));
  }

  HashObbBuffer = (UINTN)OBBFvPtr;
  Size = IBBRFvLength + OBBFvLength + OBBXFvLength+OBBYFvLength+NVSFvLength;
  if (!VerifyFvHash(HashObb, HashObbBuffer, Size, (UINTN)(H1BpIbbPtr + BpBpmHashOffset))) {
    //
    // OBB data not matched.
    //
    DEBUG((EFI_D_ERROR, "OBB signed not matched.\n"));
    *OBBVerifyFlag = FALSE;
	Status = EFI_ABORTED;
  } else {
    Status = EFI_SUCCESS;
  }

ErrorExit:
  //
  // Free buffer
  //
  if (H1BpIbbPtr!= NULL) {
    FreePool(H1BpIbbPtr);
  }

  if (H2BpSbptPtr!= NULL) {
    FreePool(H2BpSbptPtr);
  }

  if (H2BpObbPtr!= NULL) {
    FreePool(H2BpObbPtr);
  }

  if (OBBFvPtr!= NULL) {
    FreePool(OBBFvPtr);
  }

  return Status;
}

/**
  Fota Recovery function entry.

  @param  PeiServices    General purpose services available to every PEIM.
  @param  FfsHeader      Points to the FFS file header

  @return FFS File state

**/



EFI_STATUS
EFIAPI
OBBVerifyEntry (
  IN EFI_PEI_FILE_HANDLE       FfsHeader,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  EFI_STATUS                            Status;
  EFI_PHYSICAL_ADDRESS                  Address;
  UINT32                                Size;
  UINT8                                 *BootPartionImageBuffer = NULL;
  UINT8                                 Index;
  UINT8                                 *FileBuffer = NULL;

  //
  //FOTA image/region component
  //
  BPDT_HEADER                           *BpdtHeader;
  UINT8                                 *H1BpdtChkPtr = NULL;
  UINT8                                 *H2BpdtChkPtr = NULL;
  UINT8                                 *TempH1BpdtPtr = NULL;
  UINT8                                 *TempH2BpdtPtr = NULL;


  BOOLEAN                               Bp1UpdateReq = FALSE;
  BOOLEAN                               Bp2UpdateReq = FALSE;
  BOOLEAN                               Bp1HdrValid = FALSE;
  BOOLEAN                               Bp2HdrValid = FALSE;
  BOOLEAN                               OBBVerifyReslut = TRUE;


  //
  // eMMC/UFS/SPI read/write handler
  //
  UINT32                                BootDevice = 0;
  UINT8                                 BootPartition1 = 0;
  UINT8                                 BootPartition2 = 0;
  UINT32                                FlashDescriptorSize;
  UINT32                                BiosFlashStartAddress;
  UINT32                                BP2FlashLinearAddress;
  SPI_REGION_TYPE                       SpiRegionType;
  FOTA_BLOCK_READ_WRITE_TABLE_PEI       *FotaBlockPtr;

  //
  // UFS boot partition checking
  //
  EFI_PEI_HOB_POINTERS                  HobPtr;
  IAFW_DNX_REQ_SET_RESP_DATA            Resp;
  UFS_CONFIG_DESC                       *Config = NULL;
  

  
  DEBUG((EFI_D_INFO, "OOB Verify  Entry Point.\n"));
  ZeroMem(&Resp, sizeof(IAFW_DNX_REQ_SET_RESP_DATA));
  SetMem(&HobPtr, sizeof(HobPtr), 0x0);
  FlashDescriptorSize = 0;
  BP2FlashLinearAddress = 0;
  Status = PeiServicesRegisterForShadow (FfsHeader);
  if (!EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get BIOS boot device
  //
  Status = PeiGetBootDevice(&BootDevice);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can not get BIOS boot device.\n"));
    return Status;
  }

  if (BootDevice == BOOT_FROM_EMMC) {
    BootPartition1 = EMMC_BOOT_PARTITION_1;
    BootPartition2 = EMMC_BOOT_PARTITION_2;
    FotaBlockPtr = &FotaEmmcBlockTablePei;
    DEBUG ((EFI_D_INFO, "Boot from eMMC device.\n"));
  } else if (BootDevice == BOOT_FROM_UFS) {
    //
    // Get UFS Configuration Descriptor Hob
    // Scan all UFS partition to get Boot LU A and Boot LU B
    //
    HobPtr.Guid   = GetFirstGuidHob (&gUfsBootLunIdHobGuid);
    ASSERT (HobPtr.Guid != NULL);
    Config = (UFS_CONFIG_DESC*) GET_GUID_HOB_DATA (HobPtr.Guid);

    for (Index = 0; Index < 8; Index++) {
      if (Config->UnitDescConfParams[Index].BootLunId == 0x1) {  //Boot LU A
        BootPartition1 = Index;
        DEBUG ((EFI_D_INFO, "UFS Boot Partition 1 is: %x\n", BootPartition1));
      }
      if (Config->UnitDescConfParams[Index].BootLunId == 0x2) {  //Boot LU B
        BootPartition2 = Index;
        DEBUG ((EFI_D_INFO, "UFS Boot Partition 2 is: %x\n", BootPartition2));
      }
    }
        DEBUG ((EFI_D_INFO, "Boot from UFS device.\n"));
    } else if(BootDevice == BOOT_FROM_SPI){
        DEBUG ((EFI_D_INFO, "Boot from SPI device.\n"));
		SpiRegionType = FlashRegionAll;
	    BiosFlashStartAddress = GetSpiFlashRegionBase(BIOS);
        BP2FlashLinearAddress = (BiosFlashStartAddress + GetSpiFlashRegionBase (DeviceExpansion1)) / 2;
        DEBUG ((EFI_D_INFO, "BiosFlashStartAddress: 0x%X, BP2FlashLinearAddress: 0x%X\n", BiosFlashStartAddress, BP2FlashLinearAddress));
        FotaBlockPtr = &FotaSpiBlockTablePei;
		FlashDescriptorSize = SPI_FD_SIZE;
    } else {
      DEBUG ((EFI_D_ERROR, "Unsupported boot device.\n"));
      return EFI_UNSUPPORTED;
    }

  //
  // <TBD>
  // Checking whether should update IFWI by CSE status bit
  // 1. If CSE recovery bit is set, should update BP1 and BP2
  // 2. If CSE recovery bit isn't be set, we assert that BP1 partition is intact.
  //    a) Check BP2 BPDT sigbature
  //    b) Verify OBB SHA256 value compare with Manifest.
  //

  //
  // Step1. Check whether boot partition is corrupted or not.
  //

  // Checking H1 BPDT signature

  Size = 0x1000;
  Status = (*PeiServices)->AllocatePages (
                             PeiServices,
                             EfiBootServicesCode,
                             (Size - 1) / 0x1000 + 1,
                             &Address
                             );
  H1BpdtChkPtr = (UINT8 *) (UINTN) Address;
  if ((EFI_ERROR(Status)) || (H1BpdtChkPtr == NULL)) {
    DEBUG((EFI_D_ERROR, "Failed to allocate memory for reading boot partition1 BPDT table.\n"));
    Status = EFI_BAD_BUFFER_SIZE;
    goto ErrorExitB;
  }

  if(BootDevice != BOOT_FROM_SPI){ 
    Status = FotaBlockPtr->ProcessReadBlock(PeiServices, BootPartition1, 0, Size, H1BpdtChkPtr);
  } else if (BootDevice == BOOT_FROM_SPI) {
	Status = FotaBlockPtr->ProcessReadBlock(PeiServices, SpiRegionType, SPI_FD_SIZE, Size, H1BpdtChkPtr);
  } else {
    DEBUG ((EFI_D_ERROR, "Unsupported boot device.\n"));
    Status = EFI_UNSUPPORTED;
  }
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Read Boot Partiton 1 BPDT failed, Status = %r\n", Status));
    goto ErrorExitB;
  } else {
    DEBUG((EFI_D_INFO, "Read Boot Partition 1 BPDT successfully.\n"));
  }
   
  BpdtHeader = (BPDT_HEADER *)H1BpdtChkPtr;
  if (BpdtHeader->Signature != BPDT_SIGNATURE) {
    DEBUG ((EFI_D_ERROR, "Boot Partition 1 is invalid.\n"));
  } else {
    DEBUG((EFI_D_INFO, "Boot Partition 1 is valid.\n"));
    Bp1HdrValid = TRUE;
  }

  // Checking H2 BPDT signature
  Size = 0x1000;
  Status = (*PeiServices)->AllocatePages (
                             PeiServices,
                             EfiBootServicesCode,
                             (Size - 1) / 0x1000 + 1,
                             &Address
                             );
  H2BpdtChkPtr = (UINT8 *) (UINTN) Address;
  if ((EFI_ERROR(Status)) || (H2BpdtChkPtr == NULL)) {
    DEBUG((EFI_D_ERROR, "Failed to allocate memory for reading boot partition BPDT table.\n"));
    Status = EFI_BAD_BUFFER_SIZE;
    goto ErrorExitB;
  }

  if(BootDevice != BOOT_FROM_SPI){ 
    Status = FotaBlockPtr->ProcessReadBlock(PeiServices, BootPartition2, 0, Size, H2BpdtChkPtr);
  } else if (BootDevice == BOOT_FROM_SPI) {
	Status = FotaBlockPtr->ProcessReadBlock(PeiServices, SpiRegionType, BP2FlashLinearAddress, Size, H2BpdtChkPtr);
  } else {
    DEBUG ((EFI_D_ERROR, "Unsupported boot device.\n"));
    Status = EFI_UNSUPPORTED;
  }
  
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Read Boot Partiton 2 BPDT failed, Status = %r\n", Status));
    goto ErrorExitB;
  } else {
    DEBUG((EFI_D_INFO, "Read Boot Partition 2 BPDT successfully.\n"));
  }
 
  BpdtHeader = (BPDT_HEADER *)H2BpdtChkPtr;
  if (BpdtHeader->Signature != BPDT_SIGNATURE) {
    DEBUG ((EFI_D_ERROR, "Boot Partition 2 is invalid.\n"));
  } else {
    DEBUG((EFI_D_INFO, "Boot Partition 2 is valid.\n"));
    Bp2HdrValid = TRUE;
  }
  
  //
  // Checking OBB FV integrity. Assumption is H2 contains OBB 
  //
  if (Bp1HdrValid) {
    Status = VerifyOBBFvInBP(PeiServices, H1BpdtChkPtr, H2BpdtChkPtr, &OBBVerifyReslut);
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Verify OBB FV failed, Status = %r\n", Status));
      Bp2UpdateReq = TRUE;
      goto ErrorExitB;
	} else {
      Bp2UpdateReq = FALSE;
	}
  } 

  if (!Bp1UpdateReq & !Bp2UpdateReq) {
    DEBUG((EFI_D_INFO, "IFWI boot partition are intact.\n"));
  }


ErrorExitB:
  //
  // Free buffer
  //
  if (H1BpdtChkPtr!= NULL) {
    FreePool(H1BpdtChkPtr);
  }

  if (H2BpdtChkPtr!= NULL) {
    FreePool(H2BpdtChkPtr);
  }

  if (FileBuffer != NULL) {
    FreePool(FileBuffer);
  }

  if (BootPartionImageBuffer != NULL) {
    FreePool(BootPartionImageBuffer);
  }

  if (TempH1BpdtPtr != NULL) {
    FreePool(TempH1BpdtPtr);
  }

  if (TempH2BpdtPtr != NULL) {
    FreePool(TempH2BpdtPtr);
  }

  DEBUG((EFI_D_INFO, "OBB Verification, Status = %r\n", Status));

  return Status;
}

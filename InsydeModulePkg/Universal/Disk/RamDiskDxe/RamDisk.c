/** @file

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software must
 *    display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and its
 *    contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include "RamDisk.h"

LIST_ENTRY                  gRamDiskDeviceList;
UINT32                      gRamDiskDeviceLNum;

//
// EFI device path definition
//
STATIC 
H2O_RAM_DISK_DEVICE_PATH gH2ORamDiskDevicePath =
{
  MESSAGING_DEVICE_PATH,
  MSG_VENDOR_DP,
  sizeof (H2O_RAM_DISK_DEVICE_PATH) - END_DEVICE_PATH_LENGTH,
  0,
  H2O_RAM_DISK_GUID,
  0,0,0,0,0,0,0,0,  // ID assigned below
  END_DEVICE_PATH_TYPE,
  END_ENTIRE_DEVICE_PATH_SUBTYPE,
  END_DEVICE_PATH_LENGTH
};


/**
  Lookup table of total sectors vs. cluster size.
  RAM Disk sizes between 0x20D0 (4.1MB) and 0x100000 (512MB) sectors are valid FAT16 drive sizes.
**/
STATIC 
FAT16TABLE fat16tbl[] =
{
  {0x00000800, 1},  // 800 sectors * 1 sec/cluster * 512 bytes = 1 M 
  {0x00001000, 1},  // 1000 sectors * 1 sec/cluster * 512 bytes = 2 M 
  {0x00001800, 1},  // 1800 sectors * 1 sec/cluster * 512 bytes = 3 M 
  {0x00007FA8, 2},
  {0x00040000, 4},
  {0x00080000, 8},
  {0x00100000,16},
  {0xFFFFFFFF, 0}
};

/** 
  RAM pseudo-boot sector.  No code.
  Needs BS_Sig, BPB_SecPerClus, BPB_TotSec32, g_bs.BPB_TotSec16,
  and BPB_FATSz16 filled out properly by FormatRamdisk().
**/
STATIC 
BOOTSEC g_bs =
{
  {0xeb,0x0,0x90},                            // BS_jmpBoot      
  {'E','F','I','R','D','I','S','K'},          // BS_OEMName      
  512,                                        // BPB_BytsPerSec             
  0,                                          // BPB_SecPerClus  
  1,                                          // BPB_RsvdSecCnt  
  2,                                          // BPB_NumFATs     
  512,                                        // BPB_RootEntCnt  
  0,                                          // BPB_TotSec16    
  0xF8,                                       // BPB_Media       
  0,                                          // BPB_FATSz16     
  0,                                          // BPB_SecPerTrk   
  0,                                          // BPB_NumHeads    
  0,                                          // BPB_HiddSec     
  0,                                          // BPB_TotSec32    
  0,                                          // BS_DrvNum       
  0,                                          // BS_Reserved1    
  0x29,                                       // BS_BootSig      
  0,                                          // BS_VolID        
  {'N','O',' ','N','A','M','E',' ',' ',' '},  // BS_VolLab       
  {'F','A','T','1','6',' ',' ',' '}           // BS_FilSysType   
};

H2O_RAM_DISK_SERVICES_PROTOCOL    *mH2ORamDiskServicesProtocol;

/**
  Helper function to compute cluster size vs. total sectors on drive.

  @param[in] ts      total sectors.
  
  @retval value      The number of sectors per cluster.

**/
STATIC 
UINT8 
size2spc (
  IN UINT32 ts
  )
{
  int i = 0;
  
  while (fat16tbl[i].size != 0xFFFFFFFF)
  {
    if(ts <= fat16tbl[i].size) {
      return fat16tbl[i].spc;
    }
    ++i;
  }
  
  return 0;
}

/**
  Check required RAM Disk size, if size equal ZERO, will use default size.
  If required RAM Disk size less then minimal support size, or large then maxmal
  support size, return the minmal/maxmal size.

  @param[in] RamdiskSize      Required RAM Disk size.
  
  @retval value               The created RAM Disk size (MB).

**/
UINT32
GetDiskSize ( 
  IN UINT32   RamDiskSize 
  )
{
  if (RamDiskSize == 0) {
    RamDiskSize = PcdGet32(PcdH2ORamDiskSize);
  }
  
  RamDiskSize = MAX(RamDiskSize, MIN_DISK_SIZE);
  RamDiskSize = MIN(RamDiskSize, MAX_DISK_SIZE);
  
  return (RamDiskSize * 1024 * 1024);
}

/**
  Given a block of memory representing a RAM Disk, build a pseudo-boot sector
  and initialize the drive.
 
  Assumes the global boot sector structure g_bs has been filled out with the
  static information the boot sector requires.  Also assumes the RAM Disk size
  is between 4.1MB and 512MB as appropriate for FAT16 file system.

  @param[in] pStart             Physical start address.
  @param[in] Size               RAM Disk size.
  
  @retval EFI_SUCCESS           Format RAM Disk success.
  @retval EFI_BAD_BUFFER_SIZE   Required RAM Disk size not support.

**/
STATIC 
EFI_STATUS 
FormatRamDisk (
  IN VOID*  pStart,
  IN UINT32 Size
  )
{
  UINT32     TotalSectors,RootDirSectors,FatSz,tmp1,tmp2;
  UINT8      *Fat1,*Fat2;

  // The boot signature needs to be filled out 
  g_bs.BS_Sig = 0xAA55;

  // Compute the total sectors and appropriate cluster size 
  TotalSectors = Size / g_bs.BPB_BytsPerSec;
  g_bs.BPB_SecPerClus = size2spc (TotalSectors);
  if (g_bs.BPB_SecPerClus == 0) {
    DEBUG((EFI_D_ERROR, "RamDiskDxe: Sector per cluster not support\n"));
    return EFI_BAD_BUFFER_SIZE;
  }

  // Compute how many root directory sectors are needed 
  RootDirSectors = (g_bs.BPB_RootEntCnt * 32 + g_bs.BPB_BytsPerSec - 1) / g_bs.BPB_BytsPerSec;

  // Compute how many sectors are required per FAT 
  tmp1 = TotalSectors - (g_bs.BPB_RsvdSecCnt + RootDirSectors);
  tmp2 = 256 * g_bs.BPB_SecPerClus + g_bs.BPB_NumFATs;
  FatSz = (tmp1 + tmp2 - 1) / tmp2;
  if (FatSz > 0xFFFF) {
    DEBUG ((EFI_D_ERROR, "RamDiskDxe: Sectors required per FAT not support\n"));
    return EFI_BAD_BUFFER_SIZE;
  }

  // Store the total sectors and fat size values 
  if (TotalSectors > 0xFFFF) {
    g_bs.BPB_TotSec32 = TotalSectors;
  } else {
    g_bs.BPB_TotSec16 = (UINT16)TotalSectors;
  }

  g_bs.BPB_FATSz16 = (UINT16)FatSz;

  //
  //The FAT table and root directory need to be all zeroes.
  //We'll zero the whole drive.
  //
  ZeroMem (pStart,Size);
  
  // Write the completed boot sector to the RAM Disk
  CopyMem (pStart,&g_bs,512);

  // Compute the starting offsets of the two FATs 
  Fat1 = (UINT8*)pStart + g_bs.BPB_RsvdSecCnt * 512;
  Fat2 = (UINT8*)pStart + (g_bs.BPB_RsvdSecCnt + FatSz) * 512;

  // Initialize FAT1 
  Fat1[0] = g_bs.BPB_Media;
  Fat1[1] = 0xFF;
  Fat1[2] = 0xFF;
  Fat1[3] = 0xFF;

  // Initialize FAT2 
  Fat2[0] = g_bs.BPB_Media;
  Fat2[1] = 0xFF;
  Fat2[2] = 0xFF;
  Fat2[3] = 0xFF;

  return EFI_SUCCESS;
}

/**
  Implementation of block I/O read.

  @param[in]   This       Indicates a pointer to the calling context.
  @param[in]   MediaId    Id of the media, changes every time the media is replaced.
  @param[in]   Lba        The starting Logical Block Address to read from
  @param[in]   BufferSize Size of Buffer, must be a multiple of device block size.
  @param[out]  Buffer     A pointer to the destination buffer for the data. The caller is
                          responsible for either having implicit or explicit ownership of the buffer.

  @retval EFI_SUCCESS           The data was read correctly from the device.
  @retval EFI_DEVICE_ERROR      The device reported an error while performing the read.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_MEDIA_CHANGED     The MediaId does not matched the current device.
  @retval EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
  @retval EFI_INVALID_PARAMETER The read request contains LBAs that are not valid,
                                or the buffer is not on proper alignment.
**/
STATIC 
EFI_STATUS 
RamDiskReadBlocks (
  IN EFI_BLOCK_IO_PROTOCOL *This,
  IN UINT32                MediaId,
  IN EFI_LBA               LBA,
  IN UINTN                 BufferSize,
  OUT VOID                 *Buffer
  )
{
  EFI_BLOCK_IO_MEDIA   *Media;
  RAM_DISK_DEV         *RamDiskDev;
  EFI_PHYSICAL_ADDRESS RamDiskLBA;

  Media = This->Media;

  if (BufferSize % Media->BlockSize != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  if (LBA > Media->LastBlock) {
    return EFI_DEVICE_ERROR;
  }

  if (LBA + BufferSize / Media->BlockSize - 1 > Media->LastBlock) {
    return EFI_DEVICE_ERROR;
  }

  RamDiskDev = RAM_DISK_FROM_THIS(This);
  RamDiskLBA = RamDiskDev->Start + MultU64x32 (LBA, Media->BlockSize);
  CopyMem (Buffer, (VOID*)RamDiskLBA, BufferSize);

  return EFI_SUCCESS;
}

/**
  Implementation of block I/O write
  
  @param[in]   This       Indicates a pointer to the calling context.
  @param[in]   MediaId    The media ID that the write request is for.
  @param[in]   Lba        The starting logical block address to be written. The caller is
                          responsible for writing to only legitimate locations.
  @param[in]   BufferSize Size of Buffer, must be a multiple of device block size.
  @param[in]   Buffer     A pointer to the source buffer for the data.

  @retval EFI_SUCCESS           The data was written correctly to the device.
  @retval EFI_WRITE_PROTECTED   The device can not be written to.
  @retval EFI_DEVICE_ERROR      The device reported an error while performing the write.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_MEDIA_CHNAGED     The MediaId does not matched the current device.
  @retval EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
  @retval EFI_INVALID_PARAMETER The write request contains LBAs that are not valid,
                                or the buffer is not on proper alignment.
**/
STATIC 
EFI_STATUS 
RamDiskWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL *This,
  IN UINT32                MediaId,
  IN EFI_LBA               LBA,
  IN UINTN                 BufferSize,
  IN VOID                  *Buffer
  )
{
  EFI_BLOCK_IO_MEDIA   *Media;
  RAM_DISK_DEV         *RamDiskDev;
  EFI_PHYSICAL_ADDRESS RamDiskLBA;

  Media = This->Media;
  if (Media->ReadOnly) {
    return EFI_WRITE_PROTECTED;
  }

  if (BufferSize % Media->BlockSize != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  if (LBA > Media->LastBlock) {
    return EFI_DEVICE_ERROR;
  }

  if (LBA + BufferSize / Media->BlockSize - 1 > Media->LastBlock) {
    return EFI_DEVICE_ERROR;
  }

  RamDiskDev = RAM_DISK_FROM_THIS(This);
  RamDiskLBA = RamDiskDev->Start + MultU64x32 (LBA, Media->BlockSize);
  CopyMem ((VOID*)RamDiskLBA, Buffer, BufferSize);

  return EFI_SUCCESS;
}

/**
  Implementation of block I/O flush
  
  @param[in] This         Indicates the EFI_BLOCK_IO instance.

**/
STATIC 
EFI_STATUS 
RamDiskFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL *This
  )
{
  return EFI_SUCCESS;
}

/**
  To create a new RAM Disk.

  @param[in]  Size            The size for new RAM Disk.
  @param[in]  Removable       If TRUE, then new created RAM Disk could be remove.
  @param[out] RamDiskId       The DiskId for new RAM Disk.

  @retval EFI_SUCCESS         Create new RAM Disk successfully.
  @retval other               Some error occurred when executing function.

**/
EFI_STATUS
EFIAPI
CreateRamDisk (
  IN UINT32                    Size,
  IN BOOLEAN                   Removable,
  OUT UINT64                   *RamDiskId
  )
{
  EFI_STATUS           Status;
  UINT64               DiskId;
  UINT32               NumPages;
  UINT32               BlockSize;
  UINT32               RamDiskSize;
  RAM_DISK_DEV         *RamDiskDev = NULL;
  
  // 
  //  Allocate storage for RAM Disk device info on the heap.
  //
  RamDiskDev = AllocateZeroPool (sizeof(RAM_DISK_DEV));
  if (RamDiskDev == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  RamDiskSize = GetDiskSize (Size);
  
  BlockSize   = 512;

  //
  //  Compute the number of 4KB pages needed by the RAM Disk and allocate the memory.
  //
  NumPages   = RamDiskSize / EFI_PAGE_SIZE;
  if (NumPages % RamDiskSize) {
    NumPages++;
  }

  Status = gBS->AllocatePages (
                  AllocateAnyPages,
                  EfiBootServicesData,
                  NumPages,
                  &RamDiskDev->Start
                  );
  if (EFI_ERROR(Status)) {
    FreePool (RamDiskDev);
    return Status;
  }

  //
  //  Initialize the RAM Disk's device info.
  //
  Status = gBS->GetNextMonotonicCount (&DiskId);
  CopyMem (&gH2ORamDiskDevicePath.DiskId, &DiskId, sizeof(DiskId));
  CopyMem (RamDiskId, &DiskId, sizeof(DiskId));

  RamDiskDev->Signature              = PBLOCK_DEVICE_SIGNATURE;
  RamDiskDev->BlkIo.Revision         = EFI_BLOCK_IO_INTERFACE_REVISION;
  RamDiskDev->BlkIo.Media            = &RamDiskDev->Media;
  RamDiskDev->Media.RemovableMedia   = (Removable ? TRUE : FALSE);
  RamDiskDev->Media.MediaPresent     = TRUE;
  
  RamDiskDev->Media.LastBlock        = RamDiskSize / BlockSize - 1;
  RamDiskDev->Media.BlockSize        = BlockSize;
  RamDiskDev->Media.LogicalPartition = TRUE;
  RamDiskDev->Media.ReadOnly         = FALSE;
  RamDiskDev->Media.WriteCaching     = TRUE;
  
  RamDiskDev->BlkIo.ReadBlocks       = RamDiskReadBlocks;
  RamDiskDev->BlkIo.WriteBlocks      = RamDiskWriteBlocks;
  RamDiskDev->BlkIo.FlushBlocks      = RamDiskFlushBlocks;
  
  RamDiskDev->DevicePath = DuplicateDevicePath ((EFI_DEVICE_PATH_PROTOCOL*)&gH2ORamDiskDevicePath);
  
  //
  //  Build a FAT16 file system on the RAM Disk.
  //
  Status = FormatRamDisk ((VOID*)RamDiskDev->Start,RamDiskSize);
  if (EFI_ERROR(Status)) {
    if (&RamDiskDev->Start != NULL) {
      FreePool (&RamDiskDev->Start);
    }
    if (RamDiskDev->DevicePath != NULL) {
      FreePool (RamDiskDev->DevicePath);
    }
    FreePool (RamDiskDev);
    return Status;
  }
  
  InsertTailList (&gRamDiskDeviceList, &RamDiskDev->Link);
  gRamDiskDeviceLNum += 1;
  
  //
  //  Install the device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &RamDiskDev->Handle,
                  &gEfiBlockIoProtocolGuid,
                  &RamDiskDev->BlkIo,
                  &gEfiDevicePathProtocolGuid,
                  RamDiskDev->DevicePath,
                  NULL
                  );
  
  return Status;
}

/**
  To remove a specified RAM Disk selected by DiskId.

  @param[in] RamDiskId      The RAM Disk with the same DiskId will be removed.

  @retval EFI_SUCCESS       Remove successfully.
  @retval other             Some error occurred when executing function.

**/
EFI_STATUS
EFIAPI
RemoveRamDisk (
  IN UINT64                    RamDiskId
  )
{
  EFI_STATUS                 Status;
  LIST_ENTRY                 *Link;
  RAM_DISK_DEV               *RamDiskDev = NULL;
  BOOLEAN                    RamDiskFound;

  RamDiskFound  = FALSE;
  Status        = EFI_SUCCESS;

  if (gRamDiskDeviceLNum == 0) {
    return EFI_NO_MEDIA;
  }

  for (Link = gRamDiskDeviceList.ForwardLink; Link != &gRamDiskDeviceList; Link = Link->ForwardLink) {
    RamDiskDev = RAM_DISK_FROM_LINK (Link);
    if (RamDiskDev != NULL) {
      if (CompareMem (&(((H2O_RAM_DISK_DEVICE_PATH*) RamDiskDev->DevicePath)->DiskId), &RamDiskId, sizeof(UINT64)) == 0) {
        RamDiskFound = TRUE;
        break;
      }
    }
  }
  
  if (RamDiskFound) {
    if (RamDiskDev->Media.RemovableMedia) {
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      RamDiskDev->Handle,
                      &gEfiBlockIoProtocolGuid,
                      &RamDiskDev->BlkIo,
                      &gEfiDevicePathProtocolGuid,
                      RamDiskDev->DevicePath,
                      NULL
                      );
      if (!EFI_ERROR (Status)) {
        RemoveEntryList (&RamDiskDev->Link);
        gRamDiskDeviceLNum -= 1;
        if (RamDiskDev->DevicePath != NULL) {
          FreePool (RamDiskDev->DevicePath);
        }
        if (&RamDiskDev->Start != NULL) {
          FreePool (&RamDiskDev->Start);
        }
        if (RamDiskDev != NULL) {
          FreePool (RamDiskDev);
        }
      }
    } else {
      Status = EFI_DEVICE_ERROR;
    }
  } else {
    Status = EFI_NOT_FOUND;
  }
  return Status;
}

/**
  Return current RAM Disk devices counts.

  @retval value      Number of RAM Disk devices.

**/ 
UINT32
EFIAPI
GetRamDiskCount (
  VOID
  )
{
  return gRamDiskDeviceLNum;
}

/**
  Return current RAM Disk device information.

  @param[in]  DeviceNum         Number of RAM Disk device.
  @param[out] DiskSize          Size of the RAM Disk.
  @param[out] DiskId            DiskId of the RAM Disk.
  @param[out] Removable         If TRUE, it is a removable RAM Disk.
  
  @retval EFI_SUCCESS           Get RAM Disk information success.
  @retval other                 Cannot find specific RAM Disk.

**/ 
EFI_STATUS
EFIAPI
GetRamDiskInfo (
  IN  UINT32                    DeviceNum,
  OUT UINT64                    *DiskSize,
  OUT UINT64                    *DiskId,
  OUT BOOLEAN                   *Removable
  )
{
  UINTN                      Index;
  LIST_ENTRY                 *Link;
  RAM_DISK_DEV               *RamDiskDev = NULL;
  BOOLEAN                    RamDiskFound;
  
  if (DeviceNum >= gRamDiskDeviceLNum) {
    return EFI_NOT_FOUND;
  }

  Index = 0;
  RamDiskFound  = FALSE;
  for (Link = gRamDiskDeviceList.ForwardLink; Link != &gRamDiskDeviceList; Link = Link->ForwardLink) {
    RamDiskDev = RAM_DISK_FROM_LINK (Link);
    if (RamDiskDev != NULL) {
      if (Index == DeviceNum) {
        RamDiskFound = TRUE;
        break;
      }
    }
    Index++;
  }

  if (RamDiskFound) {
    *DiskSize = RamDiskDev->Media.BlockSize * (RamDiskDev->Media.LastBlock + 1);
    CopyMem (DiskId, &(((H2O_RAM_DISK_DEVICE_PATH*) RamDiskDev->DevicePath)->DiskId), sizeof(UINT64));
    *Removable = RamDiskDev->Media.RemovableMedia;
    return EFI_SUCCESS;
  } else {
    return EFI_NOT_FOUND;
  }
}

/**
  The Driver Entry Point.

  The function is the driver Entry point which will produce H2ORamDiskServicesProtocol.

  @param[in]  ImageHandle   A handle for the image that is initializing this driver
  @param[in]  SystemTable   A pointer to the EFI system table

  @retval EFI_SUCCESS:              Driver initialized successfully
  @retval EFI_LOAD_ERROR:           Failed to Initialize or has been loaded
  @retval EFI_OUT_OF_RESOURCES      Could not allocate needed resources

**/
EFI_STATUS 
InitializeRamDiskDriver(
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS           Status;
  UINT32               *AutoCreateRamDiskList = NULL;
  UINT64               DiskId = 0;
  UINTN                Index = 0;
  
  gRamDiskDeviceLNum = 0;
  InitializeListHead (&gRamDiskDeviceList);
  
  AutoCreateRamDiskList = (UINT32*)PcdGetPtr (PcdH2ORamDiskAutoCreatedList);
  if (AutoCreateRamDiskList != NULL) {
    for (Index = 0; AutoCreateRamDiskList[Index] != 0; Index++) {
      Status = CreateRamDisk (
                 AutoCreateRamDiskList[Index],
                 FALSE,
                 &DiskId
                 );
      if (EFI_ERROR(Status)) {
        return Status;
      }
    }
  }

  //
  // If PcdH2ORamDiskApiSupported set to FALSE, skip producing H2O_RAM_DISK_SERVICES_PROTOCOL interface
  //
  if (PcdGetBool (PcdH2ORamDiskApiSupported) == FALSE){    
    return EFI_UNSUPPORTED;
  }
  
  // 
  //  Allocate memory for RAM Disk protocol on the heap.
  //
  mH2ORamDiskServicesProtocol = AllocateZeroPool (sizeof(H2O_RAM_DISK_SERVICES_PROTOCOL));
  if(mH2ORamDiskServicesProtocol == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mH2ORamDiskServicesProtocol->Create                    = CreateRamDisk;
  mH2ORamDiskServicesProtocol->Remove                    = RemoveRamDisk;
  mH2ORamDiskServicesProtocol->GetRamDiskCount           = GetRamDiskCount;
  mH2ORamDiskServicesProtocol->GetRamDiskInfo            = GetRamDiskInfo;
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gH2ORamDiskServicesProtocolGuid,
                  mH2ORamDiskServicesProtocol,
                  NULL
                  );
  
  return Status;
}


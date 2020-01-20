/** @file
  This is an implementation for preload IBBR and IBBM.

Copyright (c) 2006 - 2015, Intel Corporation. All rights reserved.<BR>

This program and the accompanying materials
are licensed and made available under the terms and conditions
of the BSD License which accompanies this distribution.  The
full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

==
**/

#include <FrameworkDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <MediaDeviceDriver.h>
#include <Protocol/BlockIo.h>
#include <IBBpreload.h>
#include <IBBpreloadVariable.h>

EFI_STATUS
EFIAPI
GetFvToMemory (
  IN UINT32                FvType,
  IN VOID                  *FvBuffer,
  OUT EFI_PHYSICAL_ADDRESS **FvOnMemoryPoint
  );
/**
  Allocate memory below 4G memory address.

  This function allocates memory below 4G memory address.

  @param  MemoryType   Memory type of memory to allocate.
  @param  Size         Size of memory to allocate.

  @return Allocated address for output.

**/
VOID*
AllocateMemoryBelow4G (
  IN EFI_MEMORY_TYPE    MemoryType,
  IN UINTN              Size
  );

EFI_STATUS
ReadeMMCBlocks(UINT8 SelBPNum, UINTN FlashSize, void **pBuffer);

EFI_STATUS
EFIAPI
IBBpreloadStart (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  UINT8                                         *FlashReadBufferBP1 = NULL;
  EFI_PHYSICAL_ADDRESS                          *FvPoint=NULL;
  IBB_PRELOAD_VARIABLE                          IBBpreloadVariable;
  VOID                                          *VariableData;
  UINTN                                         VarSize;
  EFI_STATUS                                    Status;

  Status = ReadeMMCBlocks(EMMC_BOOT_PARTITION_1, 0x400000, &FlashReadBufferBP1);
  Status = GetFvToMemory(bpIBB ,FlashReadBufferBP1, &FvPoint);
  IBBpreloadVariable.IbbmAddress= (EFI_PHYSICAL_ADDRESS)FvPoint;

  Status = ReadeMMCBlocks(EMMC_BOOT_PARTITION_2, 0x400000, &FlashReadBufferBP1);
  Status = GetFvToMemory(bpObb ,FlashReadBufferBP1, &FvPoint);
  IBBpreloadVariable.IbbrAddress= (EFI_PHYSICAL_ADDRESS)FvPoint;

  VarSize = sizeof(IBB_PRELOAD_VARIABLE);
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  VarSize,
                  (VOID **) &VariableData
                  );
  Status = gRT->GetVariable (
                    IBB_PRELOAD_VARIABLE_NAME,
                    &gIBBpreloadVariableGuid,
                    NULL,
                    &VarSize,
                    VariableData
                    );
  if (EFI_ERROR (Status) || (0 != CompareMem (&IBBpreloadVariable, VariableData, VarSize))) {

    DEBUG((EFI_D_INFO, "Update IBBpreloadVariableGuid.\n"));

    Status = gRT->SetVariable (
                    IBB_PRELOAD_VARIABLE_NAME,
                    &gIBBpreloadVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    sizeof(IBB_PRELOAD_VARIABLE),
                    &IBBpreloadVariable
                   );
  }
  gBS->FreePool(VariableData);
  return Status;
}

EFI_STATUS
EFIAPI
GetFvToMemory (
  IN UINT32                FvType,
  IN VOID                  *FvBuffer,
  OUT EFI_PHYSICAL_ADDRESS **FvOnMemoryPoint
  )
/*++

Routine Description:

  Install Firmware Volume Hob's once there is main memory

Arguments:

  PeiServices       General purpose services available to every PEIM.
  NotifyDescriptor  Notify that this module published.
  Ppi               PPI that was installed.

Returns:

  EFI_SUCCESS     The function completed successfully.

--*/
{
  EFI_STATUS                     Status;
  BPDT_HEADER                    *BpdtHeader;
  BPDT_HEADER                    *sBpdtHeader;
  DIRECTORY_HEADER               *DirectoryHeader;
  EFI_FIRMWARE_VOLUME_HEADER     *FvHeader;
  UINTN                          FvLength;
  UINT32                         Offset;
  UINT8                          Index1;
  UINT8                          Index;
  EFI_PHYSICAL_ADDRESS           *AcpiMemoryBase;
  UINT32                         *FvName;
  Status = EFI_SUCCESS;

  //
  // Read 3M BIOS file from eMMC or UFS,  IAFW is loacted at LUN1 for UFS or boot partition 2 for EMMC.
  //
  BpdtHeader = (BPDT_HEADER *)FvBuffer;
  sBpdtHeader = BpdtHeader;
    while (TRUE) {
    DEBUG ((EFI_D_ERROR, "Signature = %X, DscCount = %X\n",BpdtHeader->Signature, BpdtHeader->DscCount));

    if (BpdtHeader->Signature != BPDT_SIGNATURE){
      DEBUG ((EFI_D_ERROR, "eMMC partition layout invalid\n"));
    } else {
      sBpdtHeader = BpdtHeader;
      for (Index = 0; Index < (UINT8)BpdtHeader->DscCount; Index++) {
        DEBUG ((EFI_D_ERROR, "Type = %X\n",BpdtHeader->BpdtDscriptor[Index].RegionType));
        if (bpSBpdt == BpdtHeader->BpdtDscriptor[Index].RegionType) {
           sBpdtHeader = (BPDT_HEADER *)((UINTN)FvBuffer+ (UINTN)BpdtHeader->BpdtDscriptor[Index].RegOffset);
        }
        if (FvType == BpdtHeader->BpdtDscriptor[Index].RegionType) {
          DEBUG ((EFI_D_ERROR, "Reg Offset  = %x,Reg Length =%x \n",BpdtHeader->BpdtDscriptor[Index].RegOffset,BpdtHeader->BpdtDscriptor[Index].RegSize));
          Offset = BpdtHeader->BpdtDscriptor[Index].RegOffset;
          DirectoryHeader = (DIRECTORY_HEADER  *)((UINTN)FvBuffer + Offset);
          DEBUG ((EFI_D_ERROR, "DirectoryHeader = %x , NumOf Dir Entries  = %x\n",(UINTN)DirectoryHeader,DirectoryHeader->NumOfEntries));

          for (Index1 = 0; Index1 < DirectoryHeader->NumOfEntries; Index1++) {

            Offset= DirectoryHeader->PatDirEntry[Index1].EntryOffset;
            FvHeader =   (EFI_FIRMWARE_VOLUME_HEADER *)((UINTN)DirectoryHeader+ Offset);
            DEBUG ((EFI_D_ERROR, "%a Entry Offset  = %x FvHeader = %x\n",DirectoryHeader->PatDirEntry[Index1].EntryName,Offset, (UINTN)FvHeader));
            FvName = (UINT32 *)DirectoryHeader->PatDirEntry[Index1].EntryName;
            if ((*FvName == 0x00424249)||(*FvName == SIGNATURE_32('I', 'B', 'B', 'R'))){
              FvLength = (UINTN)(UINT64)FvHeader->FvLength;
              // copy data to reserved memory
              AcpiMemoryBase = (EFI_PHYSICAL_ADDRESS *)(UINTN)AllocateMemoryBelow4G (EfiReservedMemoryType, FvLength);
              CopyMem(AcpiMemoryBase , (VOID *) FvHeader , FvLength);
              *FvOnMemoryPoint =  AcpiMemoryBase;
              DEBUG ((EFI_D_ERROR, "FvOnMemoryPoint = %X\n",*FvOnMemoryPoint));
              return Status;
            }
          }
        }
      }
    }
    if (sBpdtHeader != BpdtHeader) {
      BpdtHeader = sBpdtHeader;
    } else {
      break;
    }

}

  return Status;
}


EFI_STATUS
ReadeMMCBlocks(UINT8 SelBPNum, UINTN FlashSize, void **pBuffer)
{
  EFI_STATUS                            Status;
  UINTN                                 Index = 0;
  EFI_HANDLE                            *HandleArray = NULL;
  UINTN                                 HandleArrayCount;

  MMC_PARTITION_DATA                    *Partition;
  UINTN                                 PartitionNum = 0;
  EFI_BLOCK_IO_PROTOCOL                 *pBlockIo;
  EFI_DEVICE_PATH_PROTOCOL              *pDevicePath = NULL;
  PCI_DEVICE_PATH                       *pPciDevicePath;
  UINT32                                ControllerDevNum = 0;
  UINT8                                 *FileBuffer  = NULL;


  //
  //Read data content from Boot Partition.
  //
  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiBlockIoProtocolGuid,
                  NULL,
                  &HandleArrayCount,
                  &HandleArray
                  );
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Cannot Locate BlockIo protocol. Status = %r\n", Status));
    return Status;
  }
  DEBUG((EFI_D_INFO, "BlockIo handle count is: %d\n", HandleArrayCount));

  FileBuffer = AllocateZeroPool(FlashSize);

  if (FileBuffer == NULL) {
    DEBUG((EFI_D_ERROR, "Failed to allocate memory.\n"));
    return EFI_OUT_OF_RESOURCES;
  }


  //
  //Search boot partition
  //
  for (Index = 0; HandleArrayCount>Index; Index++){
    Status = gBS->HandleProtocol(HandleArray[Index], &gEfiBlockIoProtocolGuid, (VOID*)&pBlockIo);
    if (EFI_ERROR(Status)){
      DEBUG((EFI_D_ERROR, "Failed to Handle BlockIo Protocol. Status = %r\n", Status));
      return Status;
    }

    gBS->HandleProtocol(HandleArray[Index],&gEfiDevicePathProtocolGuid,(VOID*)&pDevicePath);
    for (;!IsDevicePathEndType(pDevicePath);pDevicePath = NextDevicePathNode(pDevicePath)) {
      if (DevicePathType(pDevicePath) == HARDWARE_DEVICE_PATH && DevicePathSubType(pDevicePath) == HW_PCI_DP){
        pPciDevicePath = (PCI_DEVICE_PATH *)pDevicePath;
        ControllerDevNum = pPciDevicePath->Device;
        DEBUG((EFI_D_ERROR, "DevNum = 0x%02X\n", (UINT8)ControllerDevNum));
        break;
      }
    }
    if (ControllerDevNum != 0x1C) {
      DEBUG((EFI_D_ERROR, "Not found DevNum\n"));
      continue;
    }


    DEBUG((EFI_D_INFO, "Signature is %d\n", BASE_CR (pBlockIo, MMC_PARTITION_DATA, BlockIo)->Signature));
    Partition = CARD_PARTITION_DATA_FROM_THIS(pBlockIo);
    if (Partition->Signature != CARD_PARTITION_SIGNATURE)
    {
      DEBUG((EFI_D_ERROR, "The signature not matched\n\n"));
      continue;
    }
    PartitionNum = CARD_DATA_PARTITION_NUM(Partition);

    DEBUG((EFI_D_INFO, "Get Partition number is: %d\n", PartitionNum));

    if (SelBPNum == PartitionNum ){
      Status = pBlockIo->ReadBlocks(
                           pBlockIo,
                           pBlockIo->Media->MediaId,
                           0,
                           FlashSize,
                           (VOID*)FileBuffer
                           );
      if (EFI_ERROR(Status)){
        DEBUG((EFI_D_ERROR, "Failed to Write BolckIo. Status = %r\n", Status));
        return Status;
      } else {
        break;
      }
    }
    *pBuffer = FileBuffer;
  }

  if (Index < HandleArrayCount)
  {
    DEBUG((EFI_D_INFO, "BlockIo read successfully.\n"));
    return EFI_SUCCESS;
  }

  return EFI_ABORTED;
}

/**
  Allocate memory below 4G memory address.

  This function allocates memory below 4G memory address.

  @param  MemoryType   Memory type of memory to allocate.
  @param  Size         Size of memory to allocate.

  @return Allocated address for output.

**/
VOID*
AllocateMemoryBelow4G (
  IN EFI_MEMORY_TYPE    MemoryType,
  IN UINTN              Size
  )
{
  UINTN                 Pages;
  EFI_PHYSICAL_ADDRESS  Address;
  EFI_STATUS            Status;
  VOID*                 Buffer;

  Pages = EFI_SIZE_TO_PAGES (Size);
  Address = 0xffffffff;

  Status  = gBS->AllocatePages (
                   AllocateMaxAddress,
                   MemoryType,
                   Pages,
                   &Address
                   );
  ASSERT_EFI_ERROR (Status);

  Buffer = (VOID *) (UINTN) Address;
  ZeroMem (Buffer, Size);

  return Buffer;
}
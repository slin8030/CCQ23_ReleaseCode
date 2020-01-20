/** @file
  Implement Windows Offline Crash Dump feature defined in Windows
  Offline Crash Dump Specification.

@copyright
 Copyright (c) 2013 - 2015 Intel Corporation. All rights reserved
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

@par Specification Reference:
**/

#include "OfflineCrashDumpDxe.h"

GLOBAL_REMOVE_IF_UNREFERENCED EFI_GUID gOfflineCrashdumpVendorGuid = OFFLINE_CRASHDUMP_VENDOR_GUID;
GLOBAL_REMOVE_IF_UNREFERENCED EFI_GUID gOfflineCrashdumpConfigurationTableGuid = OFFLINE_CRASHDUMP_CONFIGURATION_TABLE_GUID;
GLOBAL_REMOVE_IF_UNREFERENCED EFI_GUID gOfflineCrashdumpDedicatedPartitionGuid = OFFLINE_CRASHDUMP_DEDICATED_PARTITION_GUID;

extern EFI_GUID gEfiCrashDumpMemoryMapGuid;
extern EFI_GUID gEfiCrashDumpAddrGuid;

UINT8 TcoTimeoutRebootOccurred = 0;

EFI_STATUS
EFIAPI
CrashDumpResource (
  );

UINTN
GetCpuContextCmosAddr (
  VOID
  )
{
  UINT8            Index;
  UINTN            CpuRsvMemAddr = 0;

  for (Index = 0; Index < 4; Index++) {
    IoWrite8(CmosIo_72, 0x68 + Index);
    CpuRsvMemAddr |= (UINTN)(IoRead8(CmosIo_73) << (8*Index));
  }
  DEBUG((EFI_D_INFO, "CPU Context Registers CMOS Addr: 0x%X\n", CpuRsvMemAddr));
  return CpuRsvMemAddr;
}

/**
  Internal function.

  Get the TargetHandle for save to hard disk action.

  @param MemoryDumpUseCapability   Indicate which addressing method BIOS should use
  @param pTargetHandle             A pointer to TargetHandle

  @retval EFI_SUCCESS  TargetHandle has been found.

**/
EFI_STATUS
GetTargetHandle (
  IN     UINT8 MemoryDumpUseCapability,
  IN OUT EFI_HANDLE *pTargetHandle
  )
{
  EFI_STATUS                            Status;
  UINTN                                 HandleCount;
  EFI_HANDLE                            *HandleBuffer;
  UINTN                                 Index;
  UINTN                                 Index1;
  EFI_BLOCK_IO_PROTOCOL                 *BlockIo;
  EFI_DISK_IO_PROTOCOL                  *DiskIo;
  EFI_PARTITION_TABLE_HEADER            *PrimaryHeader;
  VOID                                  *EntryPtr;
  EFI_PARTITION_ENTRY                   *PartitionEntry;
  EFI_DEVICE_PATH_PROTOCOL              *BlockIoDevicePath;

  BlockIo = NULL;
  DiskIo = NULL;

  //
  // Currently BIOS only support method 1: scan the GPT to find Raw Dump Partition
  //
  if ((MemoryDumpUseCapability & BIT0) != BIT0) {
    DEBUG ((DEBUG_INFO, "MemoryDumpUseCapability: 0x%X\n", MemoryDumpUseCapability));
    //return EFI_UNSUPPORTED;
   }

  Status = gBS->LocateHandleBuffer (
             ByProtocol,
             &gEfiBlockIoProtocolGuid,
             NULL,
             &HandleCount,
             &HandleBuffer
             );

  for (Index = 0; HandleCount > Index; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiBlockIoProtocolGuid, (VOID **) &BlockIo);
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiDiskIoProtocolGuid, (VOID **) &DiskIo);

    PrimaryHeader = NULL;
    PrimaryHeader = (EFI_PARTITION_TABLE_HEADER *) AllocatePool (BlockIo->Media->BlockSize);
    if (PrimaryHeader == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    //
    // Search for GPT
    //
    Status = DiskIo->ReadDisk (
                       DiskIo,
                       BlockIo->Media->MediaId,
                       1 * BlockIo->Media->BlockSize,
                       BlockIo->Media->BlockSize,
                       (UINT8 *) PrimaryHeader
                       );

    if (PrimaryHeader->Header.Signature == SIGNATURE_64 ('E','F','I',' ','P','A','R','T')) {
      DEBUG ((DEBUG_INFO, "Find the GPT table\n"));
    } else {
      FreePool (PrimaryHeader);
      continue;
    }

    EntryPtr = NULL;
    EntryPtr = (UINT8 *) AllocatePool (PrimaryHeader->NumberOfPartitionEntries * PrimaryHeader->SizeOfPartitionEntry);
    if (EntryPtr == NULL) {
      FreePool (PrimaryHeader);
      return EFI_OUT_OF_RESOURCES;
    }

    Status = DiskIo->ReadDisk (
                       DiskIo,
                       BlockIo->Media->MediaId,
                       MultU64x32 (PrimaryHeader->PartitionEntryLBA, BlockIo->Media->BlockSize),
                       PrimaryHeader->NumberOfPartitionEntries * PrimaryHeader->SizeOfPartitionEntry,
                       EntryPtr
                       );

    //
    // Loop through partition entries search for Raw Dump Partition entry
    //
    PartitionEntry = (EFI_PARTITION_ENTRY *)EntryPtr;
    for (Index = 0; Index < PrimaryHeader->NumberOfPartitionEntries; Index++) {
      if (CompareGuid (&PartitionEntry->PartitionTypeGUID, &gOfflineCrashdumpDedicatedPartitionGuid)) {

        //
        // Raw Dump Partition entry is found, compare it to DevicePath node to find the correct Handle.
        //
        for (Index1 = 0; HandleCount > Index1; Index1++) {
          Status = gBS->HandleProtocol (HandleBuffer[Index1], &gEfiDevicePathProtocolGuid, (VOID *)&BlockIoDevicePath);
          while (!IsDevicePathEnd (BlockIoDevicePath)) {
            if (DevicePathType (BlockIoDevicePath) == MEDIA_DEVICE_PATH && DevicePathSubType (BlockIoDevicePath) == MEDIA_HARDDRIVE_DP) {
              if (((HARDDRIVE_DEVICE_PATH *)BlockIoDevicePath)->SignatureType == SIGNATURE_TYPE_GUID &&
                !CompareMem (((HARDDRIVE_DEVICE_PATH *) BlockIoDevicePath)->Signature, &PartitionEntry->UniquePartitionGUID, sizeof (EFI_GUID))) {

                DEBUG ((DEBUG_INFO, "Raw Dump Partition TargetHandle is found\n"));

                FreePool (PrimaryHeader);
                FreePool (EntryPtr);

                *pTargetHandle = HandleBuffer[Index1];
                return EFI_SUCCESS;
              }
            }
            BlockIoDevicePath = NextDevicePathNode (BlockIoDevicePath);
          }
        }
      }
      PartitionEntry++;
    }
    FreePool (PrimaryHeader);
    FreePool (EntryPtr);
  }

  DEBUG ((DEBUG_INFO, "Raw Dump Partition TargetHandle is not found\n"));
  return EFI_NOT_FOUND;
}

/**
  Internal function.

  Prepare dump data and save to hard disk.

  @param TargetHandle             A Handle with the desired BlockIo on it.

  @retval EFI_SUCCESS  Dump data is stored successfully.

**/
EFI_STATUS
SaveDumpData (
  IN EFI_HANDLE TargetHandle
  )
{
  EFI_STATUS                            Status;
  UINTN                                 VariableSize;
  UINT64                                MemoryDumpOsData;
  EFI_BLOCK_IO_PROTOCOL                 *BlockIo;
  EFI_DISK_IO_PROTOCOL                  *DiskIo;
  UINTN                                 MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR                 *MemDescriptorPtr;
  EFI_MEMORY_DESCRIPTOR                 *FirstMemDescriptorPtr;
  UINTN                                 MapKey;
  UINTN                                 DescriptorSize;
  UINT32                                DescriptorVersion;
  UINT8                                 Index;
  UINT64                                SectionOffset;
  UINT64                                PartitionByteOffset;
  UINT32                                DdrSectionNumber;
  UINT32                                CpuContextSectionNumber;
  UINT64                                TotalDumpSize;
  RAW_DUMP_HEADER                       *DumpHeader;
  RAW_DUMP_SECTION_HEADER               *DumpSectionHeader;
  OFFLINE_CPU_CONTEXT_LIST              CpuContextList;
  UINTN                                 CpuContextAddr;
  CRASH_DUMP_MEMORY_MAP_DESCRIPTOR      CrashDumpMemoryMapDesc;
  UINTN                                 VarSize;

  //
  // Layout of dump data format.
  // This data structure will be stored into Raw Dump Partition in hard disk.
  //
  // *********************************************
  // *                                           *
  // *             RAW_DUMP_HEADER               *
  // *                                           *
  // *********************************************
  // *                                           *
  // *        RAW_DUMP_SECTION_HEADER [0] (DDR)  *
  // *        RAW_DUMP_SECTION_HEADER [1] (DDR)  *
  // *        RAW_DUMP_SECTION_HEADER [2] (DDR)  *
  // *                   .                       *
  // *                   .                       *
  // *        RAW_DUMP_SECTION_HEADER [n] (CPU)  *
  // *                                           *
  // *********************************************
  // *                                           *
  // *              DDR_RANGE [0]                *
  // *              DDR_RANGE [1]                *
  // *              DDR_RANGE [2]                *
  // *                   .                       *
  // *                   .                       *
  // *             CPU_CONTEXT [n]               *
  // *                                           *
  // *********************************************
  //

  Status = gBS->HandleProtocol (TargetHandle, &gEfiBlockIoProtocolGuid, (VOID **)&BlockIo);
  Status = gBS->HandleProtocol (TargetHandle, &gEfiDiskIoProtocolGuid, (VOID **)&DiskIo);

  VariableSize = sizeof (MemoryDumpOsData);
  Status = gRT->GetVariable (
                  L"OfflineMemoryDumpOsData",
                  &gOfflineCrashdumpVendorGuid,
                  NULL,
                  &VariableSize,
                  &MemoryDumpOsData
                  );
  if (Status != EFI_SUCCESS) {
    MemoryDumpOsData = 0;
    DEBUG ((DEBUG_ERROR, "OfflineMemoryDumpOsData doesn't exist\n"));
  }

  //
  // Get Memory Map from Variable that saved from last normal boot
  //
  VarSize = sizeof(CRASH_DUMP_MEMORY_MAP_DESCRIPTOR);
  Status = gRT->GetVariable(
                  L"CrashDumpMemoryMapDesc",
                  &gEfiCrashDumpMemoryMapGuid,
                  NULL,
                  &VarSize,
                  &CrashDumpMemoryMapDesc
                  );
  ASSERT_EFI_ERROR (Status);

  MemoryMapSize     = CrashDumpMemoryMapDesc.MemoryMapSize;
  MapKey            = CrashDumpMemoryMapDesc.MapKey;
  DescriptorSize    = CrashDumpMemoryMapDesc.DescriptorSize;
  DescriptorVersion = CrashDumpMemoryMapDesc.DescriptorVersion;

  DEBUG ((EFI_D_INFO, "CrashDumpMemoryMapDesc.MemoryMapSize:     %llX\n", CrashDumpMemoryMapDesc.MemoryMapSize));
  DEBUG ((EFI_D_INFO, "CrashDumpMemoryMapDesc.MapKey:            %llX\n", CrashDumpMemoryMapDesc.MapKey));
  DEBUG ((EFI_D_INFO, "CrashDumpMemoryMapDesc.DescriptorSize:    %llX\n", CrashDumpMemoryMapDesc.DescriptorSize));
  DEBUG ((EFI_D_INFO, "CrashDumpMemoryMapDesc.DescriptorVersion: %llX\n\n", CrashDumpMemoryMapDesc.DescriptorVersion));

  VarSize = MemoryMapSize;
  DEBUG ((EFI_D_INFO, "VarSize: %X, length: 0x%X\n\n", VarSize, CrashDumpMemoryMapDesc.MemoryMapSize / CrashDumpMemoryMapDesc.DescriptorSize));

  FirstMemDescriptorPtr = AllocatePool (VarSize);
  if (FirstMemDescriptorPtr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gRT->GetVariable(
                  L"CrashDumpMemoryMapData",
                  &gEfiCrashDumpMemoryMapGuid,
                  NULL,
                  &VarSize,
                  FirstMemDescriptorPtr
                  );
  ASSERT_EFI_ERROR (Status);

  MemDescriptorPtr = FirstMemDescriptorPtr;
  DEBUG ((EFI_D_INFO, "Index     Type      PhysicalStart     VirtualStart     NumberOfPages       Attribute\n"));
  DEBUG ((EFI_D_INFO, "======  ========  ================  ================  ================  ================\n"));
  for (Index = 0; Index < (CrashDumpMemoryMapDesc.MemoryMapSize / CrashDumpMemoryMapDesc.DescriptorSize); Index++) {
    DEBUG ((EFI_D_INFO, "  %02X    %08lX  %016lX  %016lX  %016lX  %016lX\n", Index, MemDescriptorPtr->Type, MemDescriptorPtr->PhysicalStart, \
    MemDescriptorPtr->VirtualStart, MemDescriptorPtr->NumberOfPages, MemDescriptorPtr->Attribute));

    MemDescriptorPtr = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)MemDescriptorPtr + CrashDumpMemoryMapDesc.DescriptorSize);
  }

  DdrSectionNumber = 0;
  MemDescriptorPtr = FirstMemDescriptorPtr;
  for (Index = 0; Index < (MemoryMapSize / DescriptorSize); Index++) {
    if (MemDescriptorPtr && (MemDescriptorPtr->Type != EfiReservedMemoryType) && (MemDescriptorPtr->Type != EfiMemoryMappedIO)) {
      DdrSectionNumber ++;
    }
    MemDescriptorPtr = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)MemDescriptorPtr + DescriptorSize);
  }
  DEBUG ((DEBUG_INFO, "DdrSectionNumber = %x\n", DdrSectionNumber));

  //
  // Check the CPU Context
  //
  CpuContextAddr = GetCpuContextCmosAddr ();
  CopyMem(&CpuContextList, (VOID *)CpuContextAddr, sizeof(CpuContextList));
  CpuContextSectionNumber = 1;

  //
  // Prepare the RAW_DUMP_SECTION_HEADER : DDR range
  //
  MemDescriptorPtr = FirstMemDescriptorPtr;

  DumpSectionHeader = AllocateZeroPool (sizeof (RAW_DUMP_SECTION_HEADER));
  ASSERT (DumpSectionHeader != NULL);
  if (DumpSectionHeader == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  PartitionByteOffset = sizeof (RAW_DUMP_HEADER);
  TotalDumpSize = sizeof (RAW_DUMP_HEADER);
  SectionOffset = sizeof (RAW_DUMP_HEADER) + (DdrSectionNumber + CpuContextSectionNumber) * sizeof (RAW_DUMP_SECTION_HEADER);

  for (Index = 0; Index < (MemoryMapSize / DescriptorSize); Index++) {
    if (MemDescriptorPtr && (MemDescriptorPtr->Type != EfiReservedMemoryType) && (MemDescriptorPtr->Type != EfiMemoryMappedIO)) {

      DumpSectionHeader->Flags = BIT0;
      DumpSectionHeader->Version = 0x1000;
      DumpSectionHeader->Type = 1;
      DumpSectionHeader->Offset = SectionOffset;
      DumpSectionHeader->Size = MultU64x32 (MemDescriptorPtr->NumberOfPages, EFI_PAGE_SIZE);
      DumpSectionHeader->TypeSpecificInformation.DdrRangeRaw.BaseAddress = MemDescriptorPtr->PhysicalStart;
      DumpSectionHeader->Name[0] = 'D';
      DumpSectionHeader->Name[1] = 'D';
      DumpSectionHeader->Name[2] = 'R';

      Status = DiskIo->WriteDisk (
                         DiskIo,
                         BlockIo->Media->MediaId,
                         PartitionByteOffset,
                         sizeof (RAW_DUMP_SECTION_HEADER),
                         DumpSectionHeader
                         );
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_INFO, "Prepare RAW_DUMP_SECTION_HEADER fail.\n"));
        return EFI_DEVICE_ERROR;
      }

      SectionOffset = SectionOffset + DumpSectionHeader->Size;
      PartitionByteOffset += sizeof (RAW_DUMP_SECTION_HEADER);
      TotalDumpSize += sizeof (RAW_DUMP_SECTION_HEADER);
    }
    MemDescriptorPtr = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)MemDescriptorPtr + DescriptorSize);
  }

  FreePool (DumpSectionHeader);
  //
  // Prepare the RAW_DUMP_SECTION_HEADER : CPU Context
  //
  DumpSectionHeader = AllocateZeroPool (sizeof (RAW_DUMP_SECTION_HEADER));
  ASSERT (DumpSectionHeader != NULL);
  if (DumpSectionHeader == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (CpuContextSectionNumber == 1) {
    DumpSectionHeader->Flags = BIT0;
    DumpSectionHeader->Version = 0x1000;
    DumpSectionHeader->Type = 2;
    DumpSectionHeader->Offset = SectionOffset;
    DumpSectionHeader->Size = CpuContextList.NumOfCpus * sizeof (OFFLINE_CPU_CONTEXT);
    if (CpuContextList.CpuArchitecture == CONTEXT_i386) {
      DumpSectionHeader->TypeSpecificInformation.CpuContextRaw.Architecture = Architecture_X86;
    } else {
      DumpSectionHeader->TypeSpecificInformation.CpuContextRaw.Architecture = Architecture_X64;
    }
    DumpSectionHeader->TypeSpecificInformation.CpuContextRaw.CoreCount = CpuContextList.NumOfCpus;
    DumpSectionHeader->Name[0] = 'C';
    DumpSectionHeader->Name[1] = 'P';
    DumpSectionHeader->Name[2] = 'U';

    Status = DiskIo->WriteDisk (
                       DiskIo,
                       BlockIo->Media->MediaId,
                       PartitionByteOffset,
                       sizeof (RAW_DUMP_SECTION_HEADER),
                       DumpSectionHeader
                       );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "Prepare RAW_DUMP_SECTION_HEADER fail.\n"));
      return EFI_DEVICE_ERROR;
    }

    SectionOffset = SectionOffset + DumpSectionHeader->Size;
    PartitionByteOffset += sizeof (RAW_DUMP_SECTION_HEADER);
    TotalDumpSize += sizeof (RAW_DUMP_SECTION_HEADER);
  }

  FreePool (DumpSectionHeader);

  //
  //  Prepare the DDR_RANGE
  //
  MemDescriptorPtr = FirstMemDescriptorPtr;

  for (Index = 0; Index < (MemoryMapSize/DescriptorSize); Index++){
    if (MemDescriptorPtr && (MemDescriptorPtr->Type != EfiReservedMemoryType) && (MemDescriptorPtr->Type != EfiMemoryMappedIO)) {

      DEBUG ((DEBUG_INFO, "MemDescriptorPtr->PhysicalStart = %llx\n", MemDescriptorPtr->PhysicalStart));
      DEBUG ((DEBUG_INFO, "Size = %llx\n", MultU64x32 (MemDescriptorPtr->NumberOfPages, EFI_PAGE_SIZE)));
      Status = DiskIo->WriteDisk (
                         DiskIo,
                         BlockIo->Media->MediaId,
                         PartitionByteOffset,
                         (UINTN)MultU64x32 (MemDescriptorPtr->NumberOfPages, EFI_PAGE_SIZE),
                         (VOID *)(UINTN)MemDescriptorPtr->PhysicalStart
                         );
      if (EFI_ERROR(Status)) {
        DEBUG ((DEBUG_INFO, "Prepare DDR_RANGE fail.\n"));
        return EFI_DEVICE_ERROR;
      }

      PartitionByteOffset += MultU64x32 (MemDescriptorPtr->NumberOfPages, EFI_PAGE_SIZE);
      TotalDumpSize += MultU64x32 (MemDescriptorPtr->NumberOfPages, EFI_PAGE_SIZE);
    }
    MemDescriptorPtr = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)MemDescriptorPtr + DescriptorSize);
  }

  //
  //  Prepare the CPU_CONTEXT
  //
  if (CpuContextSectionNumber == 1) {
    Status = DiskIo->WriteDisk (
                       DiskIo,
                       BlockIo->Media->MediaId,
                       PartitionByteOffset,
                       CpuContextList.NumOfCpus * sizeof (OFFLINE_CPU_CONTEXT),
                       &(CpuContextList.CpuContext[0])
                       );
    if (EFI_ERROR(Status)) {
      DEBUG ((DEBUG_INFO, "Prepare CPU_CONTEXT fail. %r\n", Status));
      return EFI_DEVICE_ERROR;
    }

    TotalDumpSize += CpuContextList.NumOfCpus * sizeof (OFFLINE_CPU_CONTEXT);
  }


  //
  // Prepare the RAW_DUMP_HEADER
  //
  DumpHeader = AllocateZeroPool (sizeof (RAW_DUMP_HEADER));
  ASSERT(DumpHeader != NULL);
  if (DumpHeader == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DumpHeader->Signature = SIGNATURE_64 ('R','a','w','_','D','m','p','!');
  DumpHeader->Version = 0x1000;
  DumpHeader->Flags = BIT0;
  DumpHeader->OsData = MemoryDumpOsData;
  DumpHeader->CpuContext = 0;
  DumpHeader->ResetTrigger = 1;
  DumpHeader->DumpSize = TotalDumpSize;
  DumpHeader->TotalDumpSizeRequired = 0;
  DumpHeader->SectionsCount = DdrSectionNumber + CpuContextSectionNumber;

  Status = DiskIo->WriteDisk (
                     DiskIo,
                     BlockIo->Media->MediaId,
                     0,
                     sizeof (RAW_DUMP_HEADER),
                     DumpHeader
                     );

  FreePool (DumpHeader);

  DEBUG ((DEBUG_INFO, "Save DumpData successfully. Total DumpSize = %llx\n", TotalDumpSize));

  return EFI_SUCCESS;
}

/**
  Notification function of EVT_GROUP_READY_TO_BOOT event group.

  Save the Crashdump data into hard disk.

  @param  Event                 Event whose notification function is being invoked.
  @param  Context               Pointer to the notification function's context.

**/
VOID
EFIAPI
OfflineCrashDumpReadyToBootCallback (
  EFI_EVENT                    Event,
  VOID                         *Context
  )
{
  EFI_STATUS                            Status;
  UINT8                                 MemoryDumpUseCapability;
  UINTN                                 VariableSize;
  EFI_HANDLE                            TargetHandle;

  DEBUG ((DEBUG_INFO, "OfflineCrashDumpReadyToBootCallback() - Start\n"));

  if (Event != NULL) {
    gBS->CloseEvent (Event);
  }

  TargetHandle = NULL;

  IoWrite8(CmosIo_72, EFI_CMOS_CRASHDUMP_TRIGGERED);
  if (IoRead8(CmosIo_73) != 0x5A) {
    Status = CrashDumpResource ();
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "CrashDumpResource() failed.\n"));
      return;
    }
    return;
  }

  DEBUG ((DEBUG_INFO, "Enter to Crash Dump process...\n"));

  VariableSize = sizeof (MemoryDumpUseCapability);
  Status = gRT->GetVariable (
                  L"OfflineMemoryDumpUseCapability",
                  &gOfflineCrashdumpVendorGuid,
                  NULL,
                  &VariableSize,
                  &MemoryDumpUseCapability
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "OfflineMemoryDumpUseCapability doesn't exist\n"));
  } else {
    DEBUG ((DEBUG_INFO, "OfflineMemoryDumpUseCapability exist with the value = %x\n", MemoryDumpUseCapability));
  }

  Status = GetTargetHandle (MemoryDumpUseCapability, &TargetHandle);
  if (Status != EFI_SUCCESS) {
    goto Exit;
  }

  gST->ConOut->OutputString (gST->ConOut, L"Performing Crash Dump...\r\n");
  Status = SaveDumpData (TargetHandle);

Exit :
  //
  // Clear CPU Context flag
  //
  IoWrite8(CmosIo_72, EFI_CMOS_CRASHDUMP_TRIGGERED);
  IoWrite8(CmosIo_73, 0x0);

  //
  // Reset system
  //
  gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
  CpuDeadLoop();

  return;
}

EFI_STATUS
EFIAPI
CrashDumpResource (
  )
{
  EFI_STATUS                            Status;
  UINTN                                 VarSize;
  UINTN                                 MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR                 *FirstMemDescriptorPtr = NULL;
  EFI_MEMORY_DESCRIPTOR                 *TempMemDescriptorPtr = NULL;
  UINTN                                 MapKey;
  UINTN                                 DescriptorSize;
  UINT32                                DescriptorVersion;
  CRASH_DUMP_MEMORY_MAP_DESCRIPTOR      CrashDumpMemoryMapDesc;
  UINT8                                 Index;
  EFI_HOB_GUID_TYPE                     *GuidHob = NULL;
  EFI_PHYSICAL_ADDRESS                  CrashDumpMemCpuAddr = 0;

  DEBUG ((DEBUG_INFO, "CrashDumpResource() - Start.\n"));

  //
  // Get the MemoryMap and save to EFI variable
  //
  MemoryMapSize = 0;
  FirstMemDescriptorPtr = NULL;
  MapKey = 0;
  DescriptorSize = 0;
  DescriptorVersion = 0;

  Status = gBS->GetMemoryMap (
                  &MemoryMapSize,
                  FirstMemDescriptorPtr,
                  &MapKey,
                  &DescriptorSize,
                  &DescriptorVersion
                  );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    MemoryMapSize += SIZE_1KB;
    FirstMemDescriptorPtr = AllocateZeroPool (MemoryMapSize);
    Status = gBS->GetMemoryMap (
                    &MemoryMapSize,
                    FirstMemDescriptorPtr,
                    &MapKey,
                    &DescriptorSize,
                    &DescriptorVersion
                    );
    if (!EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "Get MemoryMap successfully\n"));
    }
  } else {
    DEBUG ((DEBUG_INFO, "Get MemoryMap failed.\n"));
    return EFI_DEVICE_ERROR;
  }

  CrashDumpMemoryMapDesc.MemoryMapSize     = MemoryMapSize;
  CrashDumpMemoryMapDesc.MapKey            = MapKey;
  CrashDumpMemoryMapDesc.DescriptorSize    = DescriptorSize;
  CrashDumpMemoryMapDesc.DescriptorVersion = DescriptorVersion;

  TempMemDescriptorPtr = FirstMemDescriptorPtr;

  DEBUG ((EFI_D_INFO, "Dump Memory Data:\n"));
  DEBUG ((EFI_D_INFO, "MemoryMapSize:     %X\n", MemoryMapSize));
  DEBUG ((EFI_D_INFO, "MapKey:            %X\n", MapKey));
  DEBUG ((EFI_D_INFO, "DescriptorSize:    %X\n", DescriptorSize));
  DEBUG ((EFI_D_INFO, "DescriptorVersion: %X\n\n", DescriptorVersion));

  DEBUG ((EFI_D_INFO, "Index     Type      PhysicalStart     VirtualStart     NumberOfPages       Attribute\n"));
  DEBUG ((EFI_D_INFO, "======  ========  ================  ================  ================  ================\n"));
  for (Index = 0; Index < (MemoryMapSize / DescriptorSize); Index++) {
    DEBUG ((EFI_D_INFO, "  %02X    %08lX  %016lX  %016lX  %016lX  %016lX\n", Index, TempMemDescriptorPtr->Type, TempMemDescriptorPtr->PhysicalStart, \
    TempMemDescriptorPtr->VirtualStart, TempMemDescriptorPtr->NumberOfPages, TempMemDescriptorPtr->Attribute));

    TempMemDescriptorPtr = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)TempMemDescriptorPtr + DescriptorSize);
  }

  VarSize = sizeof(CRASH_DUMP_MEMORY_MAP_DESCRIPTOR);
  Status = gRT->SetVariable(
                  L"CrashDumpMemoryMapDesc",
                  &gEfiCrashDumpMemoryMapGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  VarSize,
                  &CrashDumpMemoryMapDesc
                  );
  ASSERT_EFI_ERROR (Status);

  VarSize = MemoryMapSize;
  Status = gRT->SetVariable(
                  L"CrashDumpMemoryMapData",
                  &gEfiCrashDumpMemoryMapGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  VarSize,
                  FirstMemDescriptorPtr
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Save the address of memory for save CPU context registers
  //
  GuidHob = GetFirstGuidHob (&gEfiCrashDumpAddrGuid);
  if (GuidHob == NULL) {
    ASSERT_EFI_ERROR (Status);
  }
  CrashDumpMemCpuAddr = *(EFI_PHYSICAL_ADDRESS *)GET_GUID_HOB_DATA (GuidHob);
  DEBUG ((EFI_D_INFO, "CrashDumpMemCpuAddr: %llX\n", CrashDumpMemCpuAddr));

  for (Index = 0; Index < 4; Index++) {
    IoWrite8(CmosIo_72, EFI_CMOS_CRASHDUMP_ADDR_0 + Index);
    IoWrite8(CmosIo_73, (UINT8) RShiftU64 (CrashDumpMemCpuAddr, 8*Index));
  }

  return EFI_SUCCESS;
}

/**
  DXE driver entry. Get the TCO States and establish Crash Dump Configuration Table.

  @param ImageHandle   A handle for the image that is initializing this driver
  @param SystemTable   A pointer to the EFI system table

  @retval EFI_SUCCESS  Offline Crashdump Configuration Table has been created successfully.
**/
EFI_STATUS
EFIAPI
OfflineCrashDumpDxeEntry (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                            Status;
  SYSTEM_CONFIGURATION                  SystemConfiguration;
  UINTN                                 VarSize;
  UINT16                                AcpiBase;
  UINT32                                Data32;
  OFFLINE_CRASHDUMP_CONFIGURATION_TABLE *OfflineCrashdumpConfigurationTable;
  EFI_PHYSICAL_ADDRESS                  Address;
  EFI_EVENT                             Event;

  DEBUG ((EFI_D_INFO, "OfflineCrashDumpDxeEntry - Start\n"));

  VarSize = sizeof(SYSTEM_CONFIGURATION);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (SYSTEM_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
  Status = GetSystemConfigData (&SystemConfiguration, &VarSize);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_INFO, "Get Offline Crash Dump variable failed. %r\n", Status));
    return Status;
  }

  if (SystemConfiguration.CrashDump == 0) {
    return EFI_SUCCESS;
  }

  //
  // Check TCO status register
  //
  AcpiBase = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  Data32 = IoRead32 (AcpiBase + R_TCO_STS);
  DEBUG ((EFI_D_INFO, "ACPI R_TCO_STS = 0x%x\n", Data32));
  if ((Data32 & B_TCO_STS_SECOND_TO) == B_TCO_STS_SECOND_TO) {
    DEBUG ((EFI_D_INFO, "TCO watchdog hard hang.\n"));
    TcoTimeoutRebootOccurred = 1; //Bit0 of AbnormalResetOccured will be set.
  }

  //
  // Perform the offline dump callback.
  //
  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             OfflineCrashDumpReadyToBootCallback,
             NULL,
             &Event
             );

  //
  // Allocate memory to establish Offline Crashdump Configuration Table.
  //
  Address = SIZE_4GB - 1;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiACPIMemoryNVS,
                  EFI_SIZE_TO_PAGES (sizeof (OFFLINE_CRASHDUMP_CONFIGURATION_TABLE)),
                  &Address
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  OfflineCrashdumpConfigurationTable = (OFFLINE_CRASHDUMP_CONFIGURATION_TABLE *)(UINTN)Address;

  //
  // Create Offline Crashdump Configuration Table
  //
  OfflineCrashdumpConfigurationTable->Version = 1;
  OfflineCrashdumpConfigurationTable->AbnormalResetOccurred = TcoTimeoutRebootOccurred;
  OfflineCrashdumpConfigurationTable->OfflineMemoryDumpCapable = BIT0;
  OfflineCrashdumpConfigurationTable->ResetDataAddress = 0;
  OfflineCrashdumpConfigurationTable->ResetDataSize = 0;

  Status = gBS->InstallConfigurationTable (
                  &gOfflineCrashdumpConfigurationTableGuid,
                  OfflineCrashdumpConfigurationTable
                  );
  if (EFI_ERROR(Status)) {
    gBS->FreePages (Address, EFI_SIZE_TO_PAGES (sizeof (OFFLINE_CRASHDUMP_CONFIGURATION_TABLE)));
    DEBUG ((DEBUG_ERROR, "Fail to install the Crashdump Configuration Table \n"));
  }

  return EFI_SUCCESS;
}

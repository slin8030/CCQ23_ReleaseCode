/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

/** @file
**/



#include <Library/DebugLib.h>
//#include <Library/DxeServicesTableLib.h>
//#include <Library/UefiBootServicesTableLib.h>
#include "OsipPrivate.h"
#include "BdsPlatform.h"

#define EFI_MAX_E820_ENTRY  100

typedef enum {
  EfiAcpiAddressRangeMemory   = 1,
  EfiAcpiAddressRangeReserved = 2,
  EfiAcpiAddressRangeACPI     = 3,
  EfiAcpiAddressRangeNVS      = 4
} EFI_ACPI_MEMORY_TYPE;

#pragma pack(1)

//typedef struct {
//  UINT64                BaseAddr;
//  UINT64                Length;
//  EFI_ACPI_MEMORY_TYPE  Type;
//} EFI_E820_ENTRY64;
//
//typedef struct {
//  UINT32                BassAddrLow;
//  UINT32                BaseAddrHigh;
//  UINT32                LengthLow;
//  UINT32                LengthHigh;
//  EFI_ACPI_MEMORY_TYPE  Type;
//} EFI_E820_ENTRY;
typedef struct {
  UINT32                BaseAddr;
  UINT32                Length;
  UINT32                Type;
} EFI_E820_ENTRY;
#pragma pack()

EFI_STATUS
ValidateFvHeader (
  IN    VOID                *Buffer,
  IN    UINTN               Size
  )
{
  EFI_FIRMWARE_VOLUME_HEADER   *FvHdr;

  FvHdr = (EFI_FIRMWARE_VOLUME_HEADER *) Buffer;

  if (FvHdr->Signature != EFI_FVH_SIGNATURE) {
    DEBUG ((EFI_D_ERROR, "Error: Invalid FV signature\n"));
    return EFI_VOLUME_CORRUPTED;
  }

  if (FvHdr->FvLength > Size) {
    DEBUG ((EFI_D_ERROR,
      "Error: FV length (0x%x) is larger than data read (0x%x)\n",
      FvHdr->FvLength,
      Size));
    return EFI_VOLUME_CORRUPTED;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
ValidateOsip(
  IN  OSIP_HEADER   *Osip,
  IN  UINTN         MaxBlocks
  )
{
  UINTN           idx;
  UINTN           nextBlock = OSIP_SIZE_IN_BLOCKS;

  DEBUG((EFI_D_ERROR, "Parsing OSIP Header...\n")) ;
  DEBUG((EFI_D_ERROR, " Signature         : %x\n", Osip->Signature));
  DEBUG((EFI_D_ERROR, " MajorRevision     : %x\n", Osip->MajorRevision));
  DEBUG((EFI_D_ERROR, " MinorRevision     : %x\n", Osip->MinorRevision));
  DEBUG((EFI_D_ERROR, " NumberOfPointers  : %x\n", Osip->NumberOfPointers));
  DEBUG((EFI_D_ERROR, " NumberOfImages    : %x\n", Osip->NumberOfImages));
  DEBUG((EFI_D_ERROR, " Checksum          : %x\n", Osip->Checksum));

  if (Osip->Signature != OSIP_HEADER_SIGNATURE)             return EFI_INVALID_PARAMETER;
  if (Osip->MajorRevision != OSIP_HEADER_MAJOR_REVISION)    return EFI_INVALID_PARAMETER;
  if (Osip->MinorRevision != OSIP_HEADER_MINOR_REVISION)    return EFI_INVALID_PARAMETER;
  if (Osip->NumberOfPointers == 0)                          return EFI_INVALID_PARAMETER;
  if (Osip->NumberOfPointers > MAX_OSII_ENTRIES)            return EFI_INVALID_PARAMETER;
  if (Osip->NumberOfImages == 0)                            return EFI_INVALID_PARAMETER;
  if (Osip->NumberOfImages > Osip->NumberOfPointers)        return EFI_INVALID_PARAMETER;

  //
  // TODO: Validate checksum, not sure what good that does though...
  //

  DEBUG((EFI_D_ERROR, "Parsing OSII Entries...\n"));

  for (idx = 0; idx < Osip->NumberOfPointers; idx++) {
    if(idx < (sizeof(Osip->Osii) / sizeof(Osip->Osii[0]))) {
      DEBUG((EFI_D_ERROR, "Image %d\n", idx+1));
      DEBUG((EFI_D_ERROR, " MajorRevision     : %x\n", Osip->Osii[idx].MajorRevision));
      DEBUG((EFI_D_ERROR, " MinorRevision     : %x\n", Osip->Osii[idx].MinorRevision));
      DEBUG((EFI_D_ERROR, " FirstBlock        : %x\n", Osip->Osii[idx].FirstBlock));
      DEBUG((EFI_D_ERROR, " LoadAddress       : %x\n", Osip->Osii[idx].LoadAddress));
      DEBUG((EFI_D_ERROR, " EntryPoint        : %x\n", Osip->Osii[idx].EntryPoint));
      DEBUG((EFI_D_ERROR, " BlockCount        : %x\n", Osip->Osii[idx].BlockCount));
      DEBUG((EFI_D_ERROR, " Attributes        : %x\n", Osip->Osii[idx].Attributes));

      //
      // Enforce ordering, do not permit empty entries or holes
      //
      if (Osip->Osii[idx].FirstBlock != nextBlock)       return EFI_INVALID_PARAMETER;
      if (Osip->Osii[idx].BlockCount == 0)               return EFI_INVALID_PARAMETER;
      nextBlock += Osip->Osii[idx].BlockCount;

      //
      // TODO: More intensive OSII validation
      //
    }
  }

  //
  // Make sure numBlocks is not pointing past the end of the device
  //
  if (nextBlock > MaxBlocks) return EFI_INVALID_PARAMETER;

  return EFI_SUCCESS;
}

VOID
RebaseImage(
  IN  EFI_PHYSICAL_ADDRESS  DstBuffer,
  IN  EFI_PHYSICAL_ADDRESS  SrcBuffer,
  IN  UINTN                 NumberOfBytes
  )
{
  UINT8   *sptr;
  UINT8   *dptr;
  UINTN   idx;

  sptr      = (UINT8 *) (UINTN) SrcBuffer;
  dptr      = (UINT8 *) (UINTN) DstBuffer;

  for (idx = 0; idx < NumberOfBytes; ++idx, ++dptr, ++sptr) {
      *dptr = *sptr;
  }

  return;
}


UINTN
GetImageSizeByNumber(
  IN  UINTN  ImageNumber
  )
{
  return mOsip->Osii[ImageNumber].BlockCount * OSP_BLOCK_SIZE;
}


EFI_STATUS
LoadImageByNumber(
  IN  UINTN                 ImageNumber,
  OUT EFI_PHYSICAL_ADDRESS  *ImageBase,
  OUT UINTN                 *NumberOfPages
  )
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  physAddr;
  UINTN                 firstBlock;
  UINTN                 numBlocks;
  UINTN                 numPages;

  //
  // Parse OSII entry and find OS Image size in bytes
  //
  firstBlock  = mOsip->Osii[ImageNumber].FirstBlock;
  numBlocks   = mOsip->Osii[ImageNumber].BlockCount;
  numPages    = OSP_BLOCKS_TO_PAGES(numBlocks);

//  //
//  // Allocate image buffer
//  //
  Status = gBS->AllocatePages(
                  AllocateAnyPages,
                  EfiBootServicesData,
                  numPages,
                  &physAddr);

  if (EFI_ERROR(Status)) ASSERT_EFI_ERROR(Status);
  //physAddr = 0x01100000 + 0x00300000;
  
  //
  // Copy OSII[ImageNumber] to buffer
  //
  Status = mBootableBlockIo->ReadBlocks(
                            mBootableBlockIo,
                            mBootableBlockIo->Media->MediaId,
                            firstBlock,
                            numBlocks * OSP_BLOCK_SIZE,
                            (VOID *) (UINTN) physAddr);

  if (EFI_ERROR(Status)) {
    gBS->FreePages(physAddr, numPages);
    return EFI_DEVICE_ERROR;
  }

  *ImageBase      = physAddr;
  *NumberOfPages  = numPages;

  return EFI_SUCCESS;
}



/**
  Convert EFI Memory Type to E820 Memory Type.

  @param  Type  EFI Memory Type

  @return ACPI Memory Type for EFI Memory Type

**/
EFI_ACPI_MEMORY_TYPE
EfiMemoryTypeToE820Type (
  IN  UINT32    Type
  )
{
  switch (Type) {
  case EfiLoaderCode:
  case EfiLoaderData:
  case EfiBootServicesCode:
  case EfiBootServicesData:
  case EfiConventionalMemory:
  case EfiRuntimeServicesCode:
  case EfiRuntimeServicesData:
    return EfiAcpiAddressRangeMemory;

  case EfiACPIReclaimMemory:
    return EfiAcpiAddressRangeACPI;

  case EfiACPIMemoryNVS:
    return EfiAcpiAddressRangeNVS;

  //
  // All other types map to reserved.
  // Adding the code just waists FLASH space.
  //
  //  case  EfiReservedMemoryType:
  //  case  EfiUnusableMemory:
  //  case  EfiMemoryMappedIO:
  //  case  EfiMemoryMappedIOPortSpace:
  //  case  EfiPalCode:
  //
  default:
    return EfiAcpiAddressRangeReserved;
  }
}


/**
  Build the E820 table.

  @param  Private  Legacy BIOS Instance data
  @param  Size     Size of E820 Table

  @retval EFI_SUCCESS  It should always work.

**/
EFI_STATUS
LegacyBiosBuildE820 (
//  IN  LEGACY_BIOS_INSTANCE    *Private,
//  OUT UINTN                   *Size
  )
{
//  EFI_STATUS                  Status;
//  EFI_E820_ENTRY64            *E820Table;
//  EFI_MEMORY_DESCRIPTOR       *EfiMemoryMap;
//  EFI_MEMORY_DESCRIPTOR       *EfiMemoryMapEnd;
//  EFI_MEMORY_DESCRIPTOR       *EfiEntry;
//  EFI_MEMORY_DESCRIPTOR       *NextEfiEntry;
//  EFI_MEMORY_DESCRIPTOR       TempEfiEntry;
//  UINTN                       EfiMemoryMapSize;
//  UINTN                       EfiMapKey;
//  UINTN                       EfiDescriptorSize;
//  UINT32                      EfiDescriptorVersion;
//  UINTN                       Index;
//  EFI_PEI_HOB_POINTERS        Hob;
//  EFI_HOB_RESOURCE_DESCRIPTOR *ResourceHob;
//  UINTN                       TempIndex;
//  UINTN                       IndexSort;
//  UINTN                       TempNextIndex;
//  EFI_E820_ENTRY64            TempE820;
//  EFI_ACPI_MEMORY_TYPE        TempType;
//  BOOLEAN                     ChangedFlag;
//  UINTN                       Above1MIndex;
//  UINT64                      MemoryBlockLength;
//
//  E820Table = (EFI_E820_ENTRY64 *) (UINTN)0x01101940;
//
//  //
//  // Get the EFI memory map.
//  //
//  EfiMemoryMapSize  = 0;
//  EfiMemoryMap      = NULL;
//  Status = gBS->GetMemoryMap (
//                  &EfiMemoryMapSize,
//                  EfiMemoryMap,
//                  &EfiMapKey,
//                  &EfiDescriptorSize,
//                  &EfiDescriptorVersion
//                  );
//  ASSERT (Status == EFI_BUFFER_TOO_SMALL);
//
//  do {
//    //
//    // Use size returned back plus 1 descriptor for the AllocatePool.
//    // We don't just multiply by 2 since the "for" loop below terminates on
//    // EfiMemoryMapEnd which is dependent upon EfiMemoryMapSize. Otherwize
//    // we process bogus entries and create bogus E820 entries.
//    //
//    EfiMemoryMap = (EFI_MEMORY_DESCRIPTOR *) AllocatePool (EfiMemoryMapSize);
//    ASSERT (EfiMemoryMap != NULL);
//    Status = gBS->GetMemoryMap (
//                    &EfiMemoryMapSize,
//                    EfiMemoryMap,
//                    &EfiMapKey,
//                    &EfiDescriptorSize,
//                    &EfiDescriptorVersion
//                    );
//    if (EFI_ERROR (Status)) {
//      FreePool (EfiMemoryMap);
//    }
//  } while (Status == EFI_BUFFER_TOO_SMALL);
//
//  ASSERT_EFI_ERROR (Status);
//
//  //
//  // Punch in the E820 table for memory less than 1 MB.
//  // Assume ZeroMem () has been done on data structure.
//  //
//  //
//  // First entry is 0 to (640k - EBDA)
//  //
//  E820Table[0].BaseAddr  = 0;
//  E820Table[0].Length    = (UINT64) ((*(UINT16 *) (UINTN)0x40E) << 4);
//  E820Table[0].Type      = EfiAcpiAddressRangeMemory;
//
//  //
//  // Second entry is (640k - EBDA) to 640k
//  //
//  E820Table[1].BaseAddr  = E820Table[0].Length;
//  E820Table[1].Length    = (UINT64) ((640 * 1024) - E820Table[0].Length);
//  E820Table[1].Type      = EfiAcpiAddressRangeReserved;
//
//  //
//  // Third Entry is legacy BIOS
//  // DO NOT CLAIM region from 0xA0000-0xDFFFF. OS can use free areas
//  // to page in memory under 1MB.
//  // Omit region from 0xE0000 to start of BIOS, if any. This can be
//  // used for a multiple reasons including OPROMS.
//  //
//
//  //
//  // The CSM binary image size is not the actually size that CSM binary used,
//  // to avoid memory corrupt, we declare the 0E0000 - 0FFFFF is used by CSM binary.
//  //
//  E820Table[2].BaseAddr  = 0xE0000;
//  E820Table[2].Length    = 0x20000;
//  E820Table[2].Type      = EfiAcpiAddressRangeReserved;
//
//  Above1MIndex = 2;
//
//  //
//  // Process the EFI map to produce E820 map;
//  //
//
//  //
//  // Sort memory map from low to high
//  //
//  EfiEntry        = EfiMemoryMap;
//  NextEfiEntry    = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
//  EfiMemoryMapEnd = (EFI_MEMORY_DESCRIPTOR *) ((UINT8 *) EfiMemoryMap + EfiMemoryMapSize);
//  while (EfiEntry < EfiMemoryMapEnd) {
//    while (NextEfiEntry < EfiMemoryMapEnd) {
//      if (EfiEntry->PhysicalStart > NextEfiEntry->PhysicalStart) {
//        CopyMem (&TempEfiEntry, EfiEntry, sizeof (EFI_MEMORY_DESCRIPTOR));
//        CopyMem (EfiEntry, NextEfiEntry, sizeof (EFI_MEMORY_DESCRIPTOR));
//        CopyMem (NextEfiEntry, &TempEfiEntry, sizeof (EFI_MEMORY_DESCRIPTOR));
//      }
//
//      NextEfiEntry = NEXT_MEMORY_DESCRIPTOR (NextEfiEntry, EfiDescriptorSize);
//    }
//
//    EfiEntry      = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
//    NextEfiEntry  = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
//  }
//
//  EfiEntry        = EfiMemoryMap;
//  EfiMemoryMapEnd = (EFI_MEMORY_DESCRIPTOR *) ((UINT8 *) EfiMemoryMap + EfiMemoryMapSize);
//  for (Index = Above1MIndex; (EfiEntry < EfiMemoryMapEnd) && (Index < EFI_MAX_E820_ENTRY - 1); ) {
//    MemoryBlockLength = (UINT64) (LShiftU64 (EfiEntry->NumberOfPages, 12));
//    if ((EfiEntry->PhysicalStart + MemoryBlockLength) < 0x100000) {
//      //
//      // Skip the memory block is under 1MB
//      //
//    } else {
//      if (EfiEntry->PhysicalStart < 0x100000) {
//        //
//        // When the memory block spans below 1MB, ensure the memory block start address is at least 1MB
//        //
//        MemoryBlockLength       -= 0x100000 - EfiEntry->PhysicalStart;
//        EfiEntry->PhysicalStart =  0x100000;
//      }
//
//      //
//      // Convert memory type to E820 type
//      //
//      TempType = EfiMemoryTypeToE820Type (EfiEntry->Type);
//
//      if ((E820Table[Index].Type == TempType) && (EfiEntry->PhysicalStart == (E820Table[Index].BaseAddr + E820Table[Index].Length))) {
//        //
//        // Grow an existing entry
//        //
//        E820Table[Index].Length += MemoryBlockLength;
//      } else {
//        //
//        // Make a new entry
//        //
//        ++Index;
//        E820Table[Index].BaseAddr  = EfiEntry->PhysicalStart;
//        E820Table[Index].Length    = MemoryBlockLength;
//        E820Table[Index].Type      = TempType;
//      }
//    }
//    EfiEntry = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
//  }
//
//  FreePool (EfiMemoryMap);
//
//  //
//  // Process the reserved memory map to produce E820 map ;
//  //
//  for (Hob.Raw = GetHobList (); !END_OF_HOB_LIST (Hob); Hob.Raw = GET_NEXT_HOB (Hob)) {
//    if (Hob.Raw != NULL && GET_HOB_TYPE (Hob) == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR) {
//      ResourceHob = Hob.ResourceDescriptor;
//      if (((ResourceHob->ResourceType == EFI_RESOURCE_MEMORY_MAPPED_IO) ||
//          (ResourceHob->ResourceType == EFI_RESOURCE_FIRMWARE_DEVICE)  ||
//          (ResourceHob->ResourceType == EFI_RESOURCE_MEMORY_RESERVED)    ) &&
//          (ResourceHob->PhysicalStart > 0x100000) &&
//          (Index < EFI_MAX_E820_ENTRY - 1)) {
//        ++Index;
//        E820Table[Index].BaseAddr  = ResourceHob->PhysicalStart;
//        E820Table[Index].Length    = ResourceHob->ResourceLength;
//        E820Table[Index].Type      = EfiAcpiAddressRangeReserved;
//      }
//    }
//  }
//
//  Index ++;
////  Private->IntThunk->EfiToLegacy16InitTable.NumberE820Entries = (UINT32)Index;
////  Private->IntThunk->EfiToLegacy16BootTable.NumberE820Entries = (UINT32)Index;
////  Private->NumberE820Entries = (UINT32)Index;
////  *Size = (UINTN) (Index * sizeof (EFI_E820_ENTRY64));
//
//  //
//  // Sort E820Table from low to high
//  //
//  for (TempIndex = 0; TempIndex < Index; TempIndex++) {
//    ChangedFlag = FALSE;
//    for (TempNextIndex = 1; TempNextIndex < Index - TempIndex; TempNextIndex++) {
//      if (E820Table[TempNextIndex - 1].BaseAddr > E820Table[TempNextIndex].BaseAddr) {
//        ChangedFlag                       = TRUE;
//        TempE820.BaseAddr                 = E820Table[TempNextIndex - 1].BaseAddr;
//        TempE820.Length                   = E820Table[TempNextIndex - 1].Length;
//        TempE820.Type                     = E820Table[TempNextIndex - 1].Type;
//
//        E820Table[TempNextIndex - 1].BaseAddr  = E820Table[TempNextIndex].BaseAddr;
//        E820Table[TempNextIndex - 1].Length    = E820Table[TempNextIndex].Length;
//        E820Table[TempNextIndex - 1].Type      = E820Table[TempNextIndex].Type;
//
//        E820Table[TempNextIndex].BaseAddr      = TempE820.BaseAddr;
//        E820Table[TempNextIndex].Length        = TempE820.Length;
//        E820Table[TempNextIndex].Type          = TempE820.Type;
//      }
//    }
//
//    if (!ChangedFlag) {
//      break;
//    }
//  }
//
//  //
//  // Remove the overlap range
//  //
//  for (TempIndex = 1; TempIndex < Index; TempIndex++) {
//    if (E820Table[TempIndex - 1].BaseAddr <= E820Table[TempIndex].BaseAddr &&
//        ((E820Table[TempIndex - 1].BaseAddr + E820Table[TempIndex - 1].Length) >=
//         (E820Table[TempIndex].BaseAddr +E820Table[TempIndex].Length))) {
//        //
//        //Overlap range is found
//        //
//        ASSERT (E820Table[TempIndex - 1].Type == E820Table[TempIndex].Type);
//
//        if (TempIndex == Index - 1) {
//          E820Table[TempIndex].BaseAddr = 0;
//          E820Table[TempIndex].Length   = 0;
//          E820Table[TempIndex].Type     = (EFI_ACPI_MEMORY_TYPE) 0;
//          Index--;
//          break;
//        } else {
//          for (IndexSort = TempIndex; IndexSort < Index - 1; IndexSort ++) {
//            E820Table[IndexSort].BaseAddr = E820Table[IndexSort + 1].BaseAddr;
//            E820Table[IndexSort].Length   = E820Table[IndexSort + 1].Length;
//            E820Table[IndexSort].Type     = E820Table[IndexSort + 1].Type;
//          }
//          Index--;
//       }
//    }
//  }
//
//
//
////  Private->IntThunk->EfiToLegacy16InitTable.NumberE820Entries = (UINT32)Index;
////  Private->IntThunk->EfiToLegacy16BootTable.NumberE820Entries = (UINT32)Index;
////  Private->NumberE820Entries = (UINT32)Index;
////  *Size = (UINTN) (Index * sizeof (EFI_E820_ENTRY64));
//
////  //
////  // Determine OS usable memory above 1Mb
////  //
////  Private->IntThunk->EfiToLegacy16BootTable.OsMemoryAbove1Mb = 0x0000;
////  for (TempIndex = Above1MIndex; TempIndex < Index; TempIndex++) {
////    if (E820Table[TempIndex].BaseAddr >= 0x100000 && E820Table[TempIndex].BaseAddr < 0x100000000ULL) { // not include above 4G memory
////      //
////      // ACPIReclaimMemory is also usable memory for ACPI OS, after OS dumps all ACPI tables.
////      //
////      if ((E820Table[TempIndex].Type == EfiAcpiAddressRangeMemory) || (E820Table[TempIndex].Type == EfiAcpiAddressRangeACPI)) {
////        Private->IntThunk->EfiToLegacy16BootTable.OsMemoryAbove1Mb += (UINT32) (E820Table[TempIndex].Length);
////      } else {
////        break; // break at first not normal memory, because SMM may use reserved memory.
////      }
////    }
////  }
////
////  Private->IntThunk->EfiToLegacy16InitTable.OsMemoryAbove1Mb = Private->IntThunk->EfiToLegacy16BootTable.OsMemoryAbove1Mb;
//
//  //
//  // Print DEBUG information
//  //
//  for (TempIndex = 0; TempIndex < Index; TempIndex++) {
//    DEBUG((EFI_D_ERROR, "E820[%2d]: 0x%16lx ---- 0x%16lx, Type = 0x%x \n",
//      TempIndex,
//      E820Table[TempIndex].BaseAddr,
//      (E820Table[TempIndex].BaseAddr + E820Table[TempIndex].Length),
//      E820Table[TempIndex].Type
//      ));
//  }
//
//  return EFI_SUCCESS;

//E820[ 0]: 0x               0 ---- 0x           2A4A0, Type = 0x1
//E820[ 1]: 0x           2A4A0 ---- 0x           A0000, Type = 0x2
//E820[ 2]: 0x           E0000 ---- 0x          100000, Type = 0x2
//E820[ 3]: 0x          100000 ---- 0x         1000000, Type = 0x1
//E820[ 4]: 0x         1000000 ---- 0x        10000000, Type = 0x2
//E820[ 5]: 0x        10000000 ---- 0x        20000000, Type = 0x1
//E820[ 6]: 0x        20000000 ---- 0x        20100000, Type = 0x2
//E820[ 7]: 0x        20100000 ---- 0x        76F95000, Type = 0x1
//E820[ 8]: 0x        76F95000 ---- 0x        77595000, Type = 0x2
//E820[ 9]: 0x        77595000 ---- 0x        77695000, Type = 0x4
//E820[10]: 0x        77695000 ---- 0x        776D5000, Type = 0x3
//E820[11]: 0x        776D5000 ---- 0x        78000000, Type = 0x1
//E820[12]: 0x        7A800000 ---- 0x        7AE00000, Type = 0x2
//E820[13]: 0x        7AE00000 ---- 0x        80000000, Type = 0x2
//E820[14]: 0x        E0000000 ---- 0x        E4000000, Type = 0x2
//E820[15]: 0x        FEC00000 ---- 0x        FEC01000, Type = 0x2
//E820[16]: 0x        FED01000 ---- 0x        FED02000, Type = 0x2
//E820[17]: 0x        FED03000 ---- 0x        FED04000, Type = 0x2
//E820[18]: 0x        FED08000 ---- 0x        FED09000, Type = 0x2
//E820[19]: 0x        FED0C000 ---- 0x        FED10000, Type = 0x2
//E820[20]: 0x        FED1C000 ---- 0x        FED1D000, Type = 0x2
//E820[21]: 0x        FEE00000 ---- 0x        FEE01000, Type = 0x2
//E820[22]: 0x        FEF00000 ---- 0x        FF000000, Type = 0x2
//E820[23]: 0x        FFE00000 ---- 0x       100000000, Type = 0x2




//E820[ 0]: 0x               0 ---- 0x           2A4A0, Type = 0x1
//E820[ 1]: 0x           2A4A0 ---- 0x           A0000, Type = 0x2
//E820[ 2]: 0x           E0000 ---- 0x          100000, Type = 0x2
//E820[ 3]: 0x          100000 ---- 0x         1000000, Type = 0x1
        //E820[ 4]: 0x        1000000 ---- 0x        10000000, Type = 0x2
//E820[ 5]: 0x        10000000 ---- 0x        20000000, Type = 0x1
//E820[ 6]: 0x        20000000 ---- 0x        20100000, Type = 0x2
//E820[ 7]: 0x        20100000 ---- 0x        76F95000, Type = 0x1
        //E820[ 8]: 0x        76F95000 ---- 0x        77595000, Type = 0x2
        //E820[ 9]: 0x        77595000 ---- 0x        77695000, Type = 0x4
        //E820[10]: 0x        77695000 ---- 0x        776D5000, Type = 0x3
        //E820[11]: 0x        776D5000 ---- 0x        78000000, Type = 0x1
        //E820[12]: 0x        7A800000 ---- 0x        7AE00000, Type = 0x2
        //E820[13]: 0x        7AE00000 ---- 0x        80000000, Type = 0x2
//E820[14]: 0x        E0000000 ---- 0x        E4000000, Type = 0x2
//E820[15]: 0x        FEC00000 ---- 0x        FEC01000, Type = 0x2
//E820[16]: 0x        FED01000 ---- 0x        FED02000, Type = 0x2
//E820[17]: 0x        FED03000 ---- 0x        FED04000, Type = 0x2
//E820[18]: 0x        FED08000 ---- 0x        FED09000, Type = 0x2
//E820[19]: 0x        FED0C000 ---- 0x        FED10000, Type = 0x2
//E820[20]: 0x        FED1C000 ---- 0x        FED1D000, Type = 0x2
//E820[21]: 0x        FEE00000 ---- 0x        FEE01000, Type = 0x2
//E820[22]: 0x        FEF00000 ---- 0x        FF000000, Type = 0x2
//E820[23]: 0x        FFE00000 ---- 0x       100000000, Type = 0x2
	
  EFI_E820_ENTRY            *E820Table;
  UINT8                     Index;
  
  E820Table = (EFI_E820_ENTRY *) (UINTN)0x01101940;
  
  Index = 0;
  E820Table[Index].BaseAddr  = 0x00000;
  E820Table[Index].Length    = 0x98000;
  E820Table[Index].Type      = 1;
  
  Index++;
  E820Table[Index].BaseAddr  = 0x98000;
  E820Table[Index].Length    = 0xa0000 - 0x98000;
  E820Table[Index].Type      = 2;
  
  Index++;
  E820Table[Index].BaseAddr  = 0xe0000;
  E820Table[Index].Length    = 0x100000 - 0xe0000;
  E820Table[Index].Type      = 2;
  
  Index++;
	E820Table[Index].BaseAddr = 0x100000;
	E820Table[Index].Length =  0x72000000 - 0x100000;
	E820Table[Index].Type =  1;

  Index++;
	E820Table[Index].BaseAddr = 0x72000000;
	E820Table[Index].Length =   0x80000000 - 0x72000000;
	E820Table[Index].Type =  2;
	
  Index++;
	E820Table[Index].BaseAddr = 0xe0000000;
	E820Table[Index].Length =   0xf0000000 - 0xe0000000;
	E820Table[Index].Type =  2;
  
  Index++;
	E820Table[Index].BaseAddr = 0xfec00000;
	E820Table[Index].Length =   0xfec01000 - 0xfec00000;
	E820Table[Index].Type =  2;
	
  Index++;
	E820Table[Index].BaseAddr = 0xfed1c000;
	E820Table[Index].Length =   0xfed20000 - 0xfed1c000;
	E820Table[Index].Type =  2;

  Index++;
	E820Table[Index].BaseAddr = 0xfee00000;
	E820Table[Index].Length =   0xfee01000 - 0xfee00000;
	E820Table[Index].Type =  2;

  Index++;
	E820Table[Index].BaseAddr = 0xFF800000;
	E820Table[Index].Length =   0x800000;
	E820Table[Index].Type =  2;
  
  return EFI_SUCCESS;
}

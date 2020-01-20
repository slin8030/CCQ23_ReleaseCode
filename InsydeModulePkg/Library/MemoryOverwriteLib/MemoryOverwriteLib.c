/** @file
  Memory Overwrite Library

;******************************************************************************
;* Copyright (c) 2015 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <PiDxe.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>

#define MEM_EQU_1MB            0x100000

/**
  Calculate the maximum physical memory addressability of the processor.

  @retval Maximum supported physical memory space.

**/
STATIC
EFI_PHYSICAL_ADDRESS
GetPhysicalMemoryLimit (
  VOID
  )
{
  EFI_HOB_CPU                      *CpuHob;
  UINT8                            PhysicalAddressBits;
#if defined (MDE_CPU_IA32) || defined (MDE_CPU_X64)
  UINT32                           RegEax;
#endif

  //
  // Get physical address bits supported.
  //
  PhysicalAddressBits = 36;

  CpuHob = GetFirstHob (EFI_HOB_TYPE_CPU);
  if (CpuHob != NULL) {
    PhysicalAddressBits = CpuHob->SizeOfMemorySpace;
  } else {
#if defined (MDE_CPU_IA32) || defined (MDE_CPU_X64)
    AsmCpuid (0x80000000, &RegEax, NULL, NULL, NULL);
    if (RegEax >= 0x80000008) {
      AsmCpuid (0x80000008, &RegEax, NULL, NULL, NULL);
      PhysicalAddressBits = (UINT8) RegEax;
    }
#endif
  }
  
  //
  // IA-32e paging translates 48-bit linear addresses to 52-bit physical addresses.
  //
  if (PhysicalAddressBits > 48) {
    PhysicalAddressBits = 48;
  }
  return (EFI_PHYSICAL_ADDRESS)(UINTN)(LShiftU64 (1, PhysicalAddressBits));
}

/**
  Clear the assigned memory region.

  @param[in]   StartAddr           The starting address of memory.
  @param[out]  EndAddr             The ending address of memory that doesn't contain itself.

**/
VOID
ClearMemory (
  IN  EFI_PHYSICAL_ADDRESS     StartAddr,
  IN  EFI_PHYSICAL_ADDRESS     EndAddr
  )
{
  if (EndAddr > StartAddr) {
    SetMem ((VOID *)(UINTN)StartAddr, (UINTN)(EndAddr - StartAddr), 0xAF);
  }
}

/**
  Overwirte memory above 1MB with meaningless data.

  @param[in] MemoryMapSize          The size, in bytes, of the MemoryMap buffer.
  @param[in] MemoryMap              A pointer to the memory map.
  @param[in] MapKey                 The key for the current memory map.
  @param[in] DescriptorSize         The size, in bytes, of an individual EFI_MEMORY_DESCRIPTOR.
  @param[in] DescriptorVersion      The version number associated with EFI_MEMORY_DESCRIPTOR.

  @retval EFI_SUCCESS               Memory overwrite succeeded.
  @retval EFI_INVALID_PARAMETER     Invalid input.
  @retval Others                    Operation failed.

**/
EFI_STATUS
EFIAPI
MemoryOverwriteAbove1M (
  IN  UINTN                        MemoryMapSize,
  IN  EFI_MEMORY_DESCRIPTOR        *MemoryMap,
  IN  UINTN                        MapKey,
  IN  UINTN                        DescriptorSize,
  IN  UINT32                       DescriptorVersion
  )
{
  EFI_MEMORY_DESCRIPTOR            *MemoryMapEnd;
  EFI_MEMORY_DESCRIPTOR            *DescriptorEntry;
  EFI_MEMORY_DESCRIPTOR            *NextDescriptorEntry;
  EFI_MEMORY_DESCRIPTOR            TempDescriptorEntry;
  EFI_MEMORY_DESCRIPTOR            *MemoryMapAbove1M;
  UINT64                           MemoryBlockLength;
  EFI_PHYSICAL_ADDRESS             HobMemStartAddr;
  EFI_PHYSICAL_ADDRESS             HobMemEndAddr;
  UINTN                            HobMemLen;
  EFI_PEI_HOB_POINTERS             Hob;
  EFI_PHYSICAL_ADDRESS             PhysicalMemoryLimit;

  if ((MemoryMap == NULL)||
      (DescriptorVersion != EFI_MEMORY_DESCRIPTOR_VERSION) || 
      (DescriptorSize < sizeof (EFI_MEMORY_DESCRIPTOR))) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Sort memory map from low to high
  //
  DescriptorEntry     = MemoryMap;
  NextDescriptorEntry = NEXT_MEMORY_DESCRIPTOR (DescriptorEntry, DescriptorSize);
  MemoryMapEnd = (EFI_MEMORY_DESCRIPTOR *) ((UINT8 *) MemoryMap + MemoryMapSize);
  while (DescriptorEntry < MemoryMapEnd) {
    while (NextDescriptorEntry < MemoryMapEnd) {
      if (DescriptorEntry->PhysicalStart > NextDescriptorEntry->PhysicalStart) {
        TempDescriptorEntry  = *DescriptorEntry;
        *DescriptorEntry     = *NextDescriptorEntry;
        *NextDescriptorEntry = TempDescriptorEntry;
      }
      NextDescriptorEntry = NEXT_MEMORY_DESCRIPTOR (NextDescriptorEntry, DescriptorSize);
    }

    //
    // According to UEFI spec., EfiConventionalMemory means Free (unallocated) memory, so clear it.
    //
    if (DescriptorEntry->Type == EfiConventionalMemory) {
      MemoryBlockLength = (UINT64) (LShiftU64 (DescriptorEntry->NumberOfPages, 12));
      if ((DescriptorEntry->PhysicalStart + MemoryBlockLength) > MEM_EQU_1MB) {
        if (DescriptorEntry->PhysicalStart < MEM_EQU_1MB) {
          ClearMemory (MEM_EQU_1MB, (DescriptorEntry->PhysicalStart + MemoryBlockLength));
        } else {
          ClearMemory (DescriptorEntry->PhysicalStart, (DescriptorEntry->PhysicalStart + MemoryBlockLength));
        }
      }
    }

    DescriptorEntry     = NEXT_MEMORY_DESCRIPTOR (DescriptorEntry, DescriptorSize);
    NextDescriptorEntry = NEXT_MEMORY_DESCRIPTOR (DescriptorEntry, DescriptorSize);
  }

  //
  // Skip the memory region below 1MB
  //
  DescriptorEntry = MemoryMap;
  MemoryBlockLength = (UINT64) (LShiftU64 (DescriptorEntry->NumberOfPages, 12));
  while ((DescriptorEntry < MemoryMapEnd) &&
         ((DescriptorEntry->PhysicalStart + MemoryBlockLength) <= MEM_EQU_1MB)) {
    DescriptorEntry = NEXT_MEMORY_DESCRIPTOR (DescriptorEntry, DescriptorSize);
    MemoryBlockLength = (UINT64) (LShiftU64 (DescriptorEntry->NumberOfPages, 12));
  }
  if (DescriptorEntry < MemoryMapEnd) {
    MemoryMapAbove1M = DescriptorEntry;
  } else {
    //
    // There is no memory region above 1MB described in memory map
    //
    MemoryMapAbove1M = MemoryMapEnd;
  }

  PhysicalMemoryLimit = GetPhysicalMemoryLimit ();
  
  //
  // According to the memory information described in HOB and MemoryMap to clear memory.
  //
  Hob.Raw = GetHobList ();
  while (!END_OF_HOB_LIST (Hob)) {
    if (Hob.Header->HobType == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR)  {
      if (Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {

        HobMemStartAddr = Hob.ResourceDescriptor->PhysicalStart;
        HobMemLen = (UINTN)(Hob.ResourceDescriptor->ResourceLength);
        HobMemEndAddr = HobMemStartAddr + HobMemLen;
        
        //
        // Continue if StartAddr reach the limit.
        //
        if (HobMemStartAddr >= PhysicalMemoryLimit) {
          Hob.Raw = GET_NEXT_HOB (Hob);
          continue;
        }
        
        if (HobMemEndAddr > PhysicalMemoryLimit) {
          HobMemEndAddr = PhysicalMemoryLimit;
          HobMemLen = (UINTN)(HobMemEndAddr - HobMemStartAddr);
        }
        
        //
        // Clear the unused memory regions above 1MB.
        //
        if (HobMemEndAddr > MEM_EQU_1MB) {
          if (HobMemStartAddr < MEM_EQU_1MB) {
            HobMemStartAddr = MEM_EQU_1MB;
            HobMemLen = (UINTN)(HobMemEndAddr - HobMemStartAddr);
          }

          //
          // Move to the lowest memory descriptor above the Start Address of this memory description HOB
          //
          DescriptorEntry = MemoryMapAbove1M;
          MemoryBlockLength = (UINT64) (LShiftU64 (DescriptorEntry->NumberOfPages, 12));
          while ((DescriptorEntry < MemoryMapEnd) &&
                 ((DescriptorEntry->PhysicalStart + MemoryBlockLength) <= HobMemStartAddr)) {
            DescriptorEntry = NEXT_MEMORY_DESCRIPTOR (DescriptorEntry, DescriptorSize);
            MemoryBlockLength = (UINT64) (LShiftU64 (DescriptorEntry->NumberOfPages, 12));
          }

          if ((DescriptorEntry < MemoryMapEnd) && (DescriptorEntry->PhysicalStart < HobMemEndAddr)) {

            //
            // Clear the memory region below the first memory descriptor contained in this HOB
            //
            if (DescriptorEntry->PhysicalStart > HobMemStartAddr){
              ClearMemory (HobMemStartAddr, DescriptorEntry->PhysicalStart);
            }

            //
            // Clear the memory regions between the memory descriptors
            //
            NextDescriptorEntry = NEXT_MEMORY_DESCRIPTOR (DescriptorEntry, DescriptorSize);
            MemoryBlockLength = (UINT64) (LShiftU64 (DescriptorEntry->NumberOfPages, 12));

            while ((NextDescriptorEntry < MemoryMapEnd) &&
                   (NextDescriptorEntry->PhysicalStart < HobMemEndAddr)) {

              ClearMemory (
                (DescriptorEntry->PhysicalStart + MemoryBlockLength),
                NextDescriptorEntry->PhysicalStart
                );

              DescriptorEntry = NEXT_MEMORY_DESCRIPTOR (DescriptorEntry, DescriptorSize);
              MemoryBlockLength = (UINT64) (LShiftU64 (DescriptorEntry->NumberOfPages, 12));
              NextDescriptorEntry = NEXT_MEMORY_DESCRIPTOR (DescriptorEntry, DescriptorSize);
            }

            //
            // Clear the memory regions above the latest memory descriptor contained in this HOB
            //
            if ((DescriptorEntry->PhysicalStart + MemoryBlockLength) < HobMemEndAddr) {
              ClearMemory ((DescriptorEntry->PhysicalStart + MemoryBlockLength), HobMemEndAddr);
            }
          } else {
            //
            // The memory regions described in memory map are not in this HOB.
            // Clear all of the memory region described in this HOB.
            //
            ClearMemory (HobMemStartAddr, HobMemEndAddr);
          }
        }
      }
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  return EFI_SUCCESS;
}

/**
  Overwirte memory below 1MB with meaningless data.

  @retval EFI_SUCCESS               Memory overwrite succeeded.
  @retval Others                    Operation failed.

**/
EFI_STATUS
EFIAPI
MemoryOverwriteBelow1M (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS         Hob;

  Hob.Raw = GetHobList ();
  while (!END_OF_HOB_LIST (Hob)) {
    if (Hob.Header->HobType == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR) {
      if (Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {
        if ((Hob.ResourceDescriptor->PhysicalStart + Hob.ResourceDescriptor->ResourceLength) <= MEM_EQU_1MB) {
          ClearMemory (
            Hob.ResourceDescriptor->PhysicalStart, 
            (Hob.ResourceDescriptor->PhysicalStart + Hob.ResourceDescriptor->ResourceLength)
            );
        } else if (Hob.ResourceDescriptor->PhysicalStart < MEM_EQU_1MB) {
          ClearMemory (Hob.ResourceDescriptor->PhysicalStart, MEM_EQU_1MB);
        }
      }
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  return EFI_SUCCESS;
}


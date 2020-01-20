/** @file
  UEFI MemoryAttributesTable support

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>

#include <Guid/EventGroup.h>

#include <Guid/MemoryAttributesTable.h>
#include <Guid/PropertiesTable.h>

#include "DxeMain.h"

/**
  This function for GetMemoryMap() with properties table capability.

  It calls original GetMemoryMap() to get the original memory map information. Then
  plus the additional memory map entries for PE Code/Data seperation.

  @param  MemoryMapSize          A pointer to the size, in bytes, of the
                                 MemoryMap buffer. On input, this is the size of
                                 the buffer allocated by the caller.  On output,
                                 it is the size of the buffer returned by the
                                 firmware  if the buffer was large enough, or the
                                 size of the buffer needed  to contain the map if
                                 the buffer was too small.
  @param  MemoryMap              A pointer to the buffer in which firmware places
                                 the current memory map.
  @param  MapKey                 A pointer to the location in which firmware
                                 returns the key for the current memory map.
  @param  DescriptorSize         A pointer to the location in which firmware
                                 returns the size, in bytes, of an individual
                                 EFI_MEMORY_DESCRIPTOR.
  @param  DescriptorVersion      A pointer to the location in which firmware
                                 returns the version number associated with the
                                 EFI_MEMORY_DESCRIPTOR.

  @retval EFI_SUCCESS            The memory map was returned in the MemoryMap
                                 buffer.
  @retval EFI_BUFFER_TOO_SMALL   The MemoryMap buffer was too small. The current
                                 buffer size needed to hold the memory map is
                                 returned in MemoryMapSize.
  @retval EFI_INVALID_PARAMETER  One of the parameters has an invalid value.

**/
EFI_STATUS
EFIAPI
CoreGetMemoryMapPropertiesTable (
  IN OUT UINTN                  *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  OUT UINTN                     *MapKey,
  OUT UINTN                     *DescriptorSize,
  OUT UINT32                    *DescriptorVersion
  );

extern EFI_PROPERTIES_TABLE  mPropertiesTable;

/**
  Internal function to get memory descriptor entry from according to input memory
  address.

  @param[in] PhysicalStart   Input memory address.
  @param[in] MemoryMap       A pointer to memory map buffer.
  @param[in] MemoryMapSize   The buffer size of MemoryMap.
  @param[in] DescriptorSize  The size, in bytes, of an individual EFI_MEMORY_DESCRIPTOR.

  @return    Memory descriptor entry which the input memory address is located or NULL if not found.
**/
STATIC
EFI_MEMORY_DESCRIPTOR  *
GetPhysicalAddressMemoryEntry (
  IN   EFI_PHYSICAL_ADDRESS    PhysicalStart,
  IN   EFI_MEMORY_DESCRIPTOR   *MemoryMap,
  IN   UINTN                   MemoryMapSize,
  IN   UINTN                   DescriptorSize
  )
{
  UINTN           Index;

  for (Index = 0; Index < MemoryMapSize / DescriptorSize; Index++) {
    if (PhysicalStart >= MemoryMap->PhysicalStart &&
        PhysicalStart < MemoryMap->PhysicalStart + LShiftU64 (MemoryMap->NumberOfPages, EFI_PAGE_SHIFT)) {
       return MemoryMap;
    }
    MemoryMap = NEXT_MEMORY_DESCRIPTOR(MemoryMap, DescriptorSize);
  }
  return NULL;
}

/**
  Internal function to check whether the input memory descriptor entry crosses
  different memory entry in memory map.

  @param[in] Entry           A pointer to EFI_MEMORY_DESCRIPTOR instancce.
  @param[in] MemoryMap       A pointer to memory map buffer.
  @param[in] MemoryMapSize   The buffer size of MemoryMap.
  @param[in] DescriptorSize  The size, in bytes, of an individual EFI_MEMORY_DESCRIPTOR.

  @retval TRUE               The input memory descriptor ccrosses different memory entry
                             in memory map.
  @retval FALSE              The input memory descriptor doesn't ccross different memory
                             entry in memory map.
**/
STATIC
BOOLEAN
CrossDifferentRegion (
  IN   EFI_MEMORY_DESCRIPTOR   *Entry,
  IN   EFI_MEMORY_DESCRIPTOR   *MemoryMap,
  IN   UINTN                   MemoryMapSize,
  IN   UINTN                   DescriptorSize
  )
{
  UINTN           Index;

  for (Index = 0; Index < MemoryMapSize / DescriptorSize; Index++) {
    if (Entry->PhysicalStart >= MemoryMap->PhysicalStart &&
        Entry->PhysicalStart < MemoryMap->PhysicalStart + LShiftU64 (MemoryMap->NumberOfPages, EFI_PAGE_SHIFT)) {
      if (Entry->PhysicalStart + LShiftU64 (Entry->NumberOfPages, EFI_PAGE_SHIFT) <=
          MemoryMap->PhysicalStart + LShiftU64 (MemoryMap->NumberOfPages, EFI_PAGE_SHIFT)) {
        return FALSE;
      }
      return TRUE;
    }
    MemoryMap = NEXT_MEMORY_DESCRIPTOR(MemoryMap, DescriptorSize);
  }
  return FALSE;
}

/**
  Internal function to update memory type in memory attribute table
  according to memory map.

  @param[in]  MemoryAttributesTable   Double pointer to EFI_MEMORY_ATTRIBUTES_TABLE instance.

  @return Updated memory attribute table.
**/
STATIC
VOID
SyncMemoryTypeWithMeoryMap (
  IN OUT EFI_MEMORY_ATTRIBUTES_TABLE    **MemoryAttributesTable
  )
{
  UINTN                          MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR          *MemoryMap;
  UINTN                          MapKey;
  UINTN                          DescriptorSize;
  UINT32                         DescriptorVersion;
  UINT32                         RuntimeEntryCount;
  EFI_MEMORY_DESCRIPTOR          *MemoryEntry;
  EFI_MEMORY_DESCRIPTOR          *OrgMemoryEntry;
  EFI_MEMORY_DESCRIPTOR          *MemoryMapEntry;
  UINTN                          Index;
  EFI_STATUS                     Status;
  EFI_MEMORY_ATTRIBUTES_TABLE    *AttributesTable;

  if (MemoryAttributesTable == NULL || *MemoryAttributesTable == NULL) {
    return;
  }
  AttributesTable = *MemoryAttributesTable;

  MemoryMapSize = 0;
  MemoryMap = NULL;
  Status = CoreGetMemoryMap (
             &MemoryMapSize,
             MemoryMap,
             &MapKey,
             &DescriptorSize,
             &DescriptorVersion
             );
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);

  do {
    MemoryMap = AllocatePool (MemoryMapSize);
    ASSERT (MemoryMap != NULL);

    Status = CoreGetMemoryMap (
               &MemoryMapSize,
               MemoryMap,
               &MapKey,
               &DescriptorSize,
               &DescriptorVersion
               );
    if (EFI_ERROR (Status)) {
      FreePool (MemoryMap);
    }
  } while (Status == EFI_BUFFER_TOO_SMALL);

  //
  // Recalculate the runtime memory entrys first
  //
  MemoryEntry = (EFI_MEMORY_DESCRIPTOR *)(AttributesTable + 1);
  RuntimeEntryCount = 0;
  for (Index = 0; Index < AttributesTable->NumberOfEntries; Index++) {
    if (CrossDifferentRegion (MemoryEntry, MemoryMap, MemoryMapSize, DescriptorSize)) {
      RuntimeEntryCount += 2;
    } else {
      RuntimeEntryCount++;
    }
    MemoryEntry = NEXT_MEMORY_DESCRIPTOR(MemoryEntry, DescriptorSize);
  }
  if (RuntimeEntryCount > AttributesTable->NumberOfEntries) {
    AttributesTable = ReallocatePool (
                        sizeof(EFI_MEMORY_ATTRIBUTES_TABLE) + DescriptorSize * AttributesTable->NumberOfEntries,
                        sizeof(EFI_MEMORY_ATTRIBUTES_TABLE) + DescriptorSize * RuntimeEntryCount,
                        AttributesTable
                        );
  }
  if (AttributesTable == NULL) {
    return;
  }
  //
  // 1. Update memory type according to memory map.
  // 2. Split to different entry if the memory entry in memory attributes table
  //    crosses two different memory entry in memory map.
  //
  MemoryEntry = (EFI_MEMORY_DESCRIPTOR *)(AttributesTable + 1);
  for (Index = 0; Index < AttributesTable->NumberOfEntries; Index++) {
    MemoryMapEntry = GetPhysicalAddressMemoryEntry (
                       MemoryEntry->PhysicalStart,
                       MemoryMap,
                       MemoryMapSize,
                       DescriptorSize
                       );
    ASSERT (MemoryMapEntry != NULL);
    if (MemoryMapEntry == NULL) {
      return;
    }

    MemoryEntry->Type = MemoryMapEntry->Type;
    if (CrossDifferentRegion (MemoryEntry, MemoryMap, MemoryMapSize, DescriptorSize)) {
      CopyMem (
        (UINT8 *) MemoryEntry + (DescriptorSize * 2),
        (UINT8 *) MemoryEntry + DescriptorSize,
        (AttributesTable->NumberOfEntries - Index - 1) * DescriptorSize
        );
      CopyMem ((UINT8 *) MemoryEntry + DescriptorSize, MemoryEntry, DescriptorSize);
      MemoryEntry->NumberOfPages = (UINTN) (MemoryMapEntry->PhysicalStart + LShiftU64 (MemoryMapEntry->NumberOfPages, EFI_PAGE_SHIFT) -  MemoryEntry->PhysicalStart) / EFI_PAGE_SIZE;
      OrgMemoryEntry = MemoryEntry;
      MemoryEntry = NEXT_MEMORY_DESCRIPTOR(MemoryEntry, DescriptorSize);
      MemoryMapEntry = GetPhysicalAddressMemoryEntry (
                         MemoryEntry->PhysicalStart + LShiftU64 (MemoryEntry->NumberOfPages, EFI_PAGE_SHIFT) - 1,
                         MemoryMap,
                         MemoryMapSize,
                         DescriptorSize
                         );
      ASSERT (MemoryMapEntry != NULL);
      if (MemoryMapEntry == NULL) {
        return;
      }
      MemoryEntry->PhysicalStart  = MemoryMapEntry->PhysicalStart;
      MemoryEntry->NumberOfPages -= OrgMemoryEntry->NumberOfPages;
      MemoryEntry->Type           =  MemoryMapEntry->Type;
    }
    MemoryEntry = NEXT_MEMORY_DESCRIPTOR(MemoryEntry, DescriptorSize);
  }
  AttributesTable->NumberOfEntries = RuntimeEntryCount;
  *MemoryAttributesTable = AttributesTable;
  FreePool (MemoryMap);
}

/**
  Install MemoryAttributesTable.

  @param[in]  Event     The Event this notify function registered to.
  @param[in]  Context   Pointer to the context data registered to the Event.
**/
VOID
EFIAPI
InstallMemoryAttributesTable (
  EFI_EVENT                               Event,
  VOID                                    *Context
  )
{
  UINTN                          MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR          *MemoryMap;
  EFI_MEMORY_DESCRIPTOR          *MemoryMapStart;
  UINTN                          MapKey;
  UINTN                          DescriptorSize;
  UINT32                         DescriptorVersion;
  UINTN                          Index;
  EFI_STATUS                     Status;
  UINT32                         RuntimeEntryCount;
  EFI_MEMORY_ATTRIBUTES_TABLE    *MemoryAttributesTable;
  EFI_MEMORY_DESCRIPTOR          *MemoryAttributesEntry;

  if ((mPropertiesTable.MemoryProtectionAttribute & EFI_PROPERTIES_RUNTIME_MEMORY_PROTECTION_NON_EXECUTABLE_PE_DATA) == 0) {
    DEBUG ((EFI_D_VERBOSE, "MemoryProtectionAttribute NON_EXECUTABLE_PE_DATA is not set, "));
    DEBUG ((EFI_D_VERBOSE, "because Runtime Driver Section Alignment is not %dK.\n", EFI_ACPI_RUNTIME_PAGE_ALLOCATION_ALIGNMENT >> 10));
    return ;
  }

  MemoryMapSize = 0;
  MemoryMap = NULL;
  Status = CoreGetMemoryMapPropertiesTable (
             &MemoryMapSize,
             MemoryMap,
             &MapKey,
             &DescriptorSize,
             &DescriptorVersion
             );
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);

  do {
    MemoryMap = AllocatePool (MemoryMapSize);
    ASSERT (MemoryMap != NULL);

    Status = CoreGetMemoryMapPropertiesTable (
               &MemoryMapSize,
               MemoryMap,
               &MapKey,
               &DescriptorSize,
               &DescriptorVersion
               );
    if (EFI_ERROR (Status)) {
      FreePool (MemoryMap);
    }
  } while (Status == EFI_BUFFER_TOO_SMALL);

  MemoryMapStart = MemoryMap;
  RuntimeEntryCount = 0;
  for (Index = 0; Index < MemoryMapSize/DescriptorSize; Index++) {
    switch (MemoryMap->Type) {
    case EfiRuntimeServicesCode:
    case EfiRuntimeServicesData:
      RuntimeEntryCount ++;
      break;
    }
    MemoryMap = NEXT_MEMORY_DESCRIPTOR(MemoryMap, DescriptorSize);
  }

  //
  // Allocate MemoryAttributesTable
  //
  MemoryAttributesTable = AllocatePool (sizeof(EFI_MEMORY_ATTRIBUTES_TABLE) + DescriptorSize * RuntimeEntryCount);
  ASSERT (MemoryAttributesTable != NULL);
  MemoryAttributesTable->Version         = EFI_MEMORY_ATTRIBUTES_TABLE_VERSION;
  MemoryAttributesTable->NumberOfEntries = RuntimeEntryCount;
  MemoryAttributesTable->DescriptorSize  = (UINT32)DescriptorSize;
  MemoryAttributesTable->Reserved        = 0;
  DEBUG ((EFI_D_VERBOSE, "MemoryAttributesTable:\n"));
  DEBUG ((EFI_D_VERBOSE, "  Version              - 0x%08x\n", MemoryAttributesTable->Version));
  DEBUG ((EFI_D_VERBOSE, "  NumberOfEntries      - 0x%08x\n", MemoryAttributesTable->NumberOfEntries));
  DEBUG ((EFI_D_VERBOSE, "  DescriptorSize       - 0x%08x\n", MemoryAttributesTable->DescriptorSize));
  MemoryAttributesEntry = (EFI_MEMORY_DESCRIPTOR *)(MemoryAttributesTable + 1);
  MemoryMap = MemoryMapStart;
  for (Index = 0; Index < MemoryMapSize/DescriptorSize; Index++) {
    switch (MemoryMap->Type) {
    case EfiRuntimeServicesCode:
    case EfiRuntimeServicesData:
      CopyMem (MemoryAttributesEntry, MemoryMap, DescriptorSize);
      MemoryAttributesEntry->Attribute &= (EFI_MEMORY_RO|EFI_MEMORY_XP|EFI_MEMORY_RUNTIME);
      DEBUG ((EFI_D_VERBOSE, "Entry (0x%x)\n", MemoryAttributesEntry));
      DEBUG ((EFI_D_VERBOSE, "  Type              - 0x%x\n", MemoryAttributesEntry->Type));
      DEBUG ((EFI_D_VERBOSE, "  PhysicalStart     - 0x%016lx\n", MemoryAttributesEntry->PhysicalStart));
      DEBUG ((EFI_D_VERBOSE, "  VirtualStart      - 0x%016lx\n", MemoryAttributesEntry->VirtualStart));
      DEBUG ((EFI_D_VERBOSE, "  NumberOfPages     - 0x%016lx\n", MemoryAttributesEntry->NumberOfPages));
      DEBUG ((EFI_D_VERBOSE, "  Attribute         - 0x%016lx\n", MemoryAttributesEntry->Attribute));
      MemoryAttributesEntry = NEXT_MEMORY_DESCRIPTOR(MemoryAttributesEntry, DescriptorSize);
      break;
    }
    MemoryMap = NEXT_MEMORY_DESCRIPTOR(MemoryMap, DescriptorSize);
  }
  SyncMemoryTypeWithMeoryMap (&MemoryAttributesTable);
  Status = gBS->InstallConfigurationTable (&gEfiMemoryAttributesTableGuid, MemoryAttributesTable);
  ASSERT_EFI_ERROR (Status);
}

/**
  Initialize MemoryAttrubutesTable support.
**/
VOID
EFIAPI
CoreInitializeMemoryAttributesTable (
  VOID
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   ReadyToBootEvent;

  //
  // Construct the table at ReadyToBoot, because this should be
  // last point to allocate RuntimeCode/RuntimeData.
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  InstallMemoryAttributesTable,
                  NULL,
                  &gEfiEventReadyToBootGuid,
                  &ReadyToBootEvent
                  );
  ASSERT_EFI_ERROR (Status);
  return ;
}

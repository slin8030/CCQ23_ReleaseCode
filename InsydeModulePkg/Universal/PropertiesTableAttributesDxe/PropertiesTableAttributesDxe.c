/** @file
  This module sets default policy for attributes of EfiACPIMemoryNVS and EfiReservedMemoryType.

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

/**
  This module sets default policy for attributes of EfiACPIMemoryNVS and EfiReservedMemoryType.

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

/**
  This module sets default policy for attributes of EfiACPIMemoryNVS and EfiReservedMemoryType.

  This module sets EFI_MEMORY_XP for attributes of EfiACPIMemoryNVS and EfiReservedMemoryType
  in UEFI memory map, if and only of PropertiesTable is published and has BIT0 set.

Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BdsCpLib.h>
#include <Library/PcdLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesLib.h>
#include <Protocol/H2OBdsServices.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Guid/EventGroup.h>
#include <Guid/H2OBdsCheckPoint.h>
#include <Guid/PropertiesTable.h>

#define RUNTIME_MEMORY_PROTECTION_DISABLED     0x00
#define RUNTIME_MEMORY_PROTECTION_ENABLED      0x01
#define RUNTIME_MEMORY_PROTECTION_AUTO         0x02

#define MICROSOFT_OS_VERSION_WINDOWS_10        0x000A0000
#define VS_FIXEDFILEINFO_SIGNATURE             0xFEEF04BD
#define RESOURCE_SECTION_RESOURCE_TYPE_VERSION 16

typedef struct {
  UINT32             Signature;
  UINT32             StrucVersion;
  UINT32             FileVersionMS;
  UINT32             FileVersionLS;
  UINT32             ProductVersionMS;
  UINT32             ProductVersionLS;
  UINT32             FileFlagsMask;
  UINT32             FileFlags;
  UINT32             FileOS;
  UINT32             FileType;
  UINT32             FileSubtype;
  UINT32             FileDateMS;
  UINT32             FileDateLS;
} VS_FIXEDFILEINFO;


STATIC EFI_IMAGE_START          mOrgStartImage;
STATIC BOOLEAN                  mPropertiesTableEnable;
STATIC BOOLEAN                  mBootingImage;


/**
  Checkpoint to let other function know system will boot image.

  @param[in]  Event     The Event this notify function registered to.
  @param[in]  Handle    The handle associated with a previously registered checkpoint handler.
**/
STATIC
VOID
EFIAPI
BootBeforeCpHandler (
  IN EFI_EVENT         Event,
  IN H2O_BDS_CP_HANDLE Handle
  )
{
  mBootingImage = TRUE;
  BdsCpUnregisterHandler (Handle);
}

/**
  Converts a number of EFI_PAGEs to a size in bytes.

  NOTE: Do not use EFI_PAGES_TO_SIZE because it handles UINTN only.

  @param[in]  Pages     The number of EFI_PAGES.

  @return     The number of bytes associated with the number of EFI_PAGEs specified
              by Pages.
**/
STATIC
UINT64
EfiPagesToSize (
  IN UINT64 Pages
  )
{
  return LShiftU64 (Pages, EFI_PAGE_SHIFT);
}

/**
  Set memory attributes according to default policy.

  @param[in]  MemoryMap        A pointer to the buffer in which firmware places the current memory map.
  @param[in]  MemoryMapSize    Size, in bytes, of the MemoryMap buffer.
  @param[in]  DescriptorSize   size, in bytes, of an individual EFI_MEMORY_DESCRIPTOR.
  @param[in]  ExecuteProtect   Enable execute protection or not.
**/
STATIC
VOID
SetMemorySpaceAttributesDefault (
  IN EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN BOOLEAN                ExecuteProtect
  )
{
  EFI_MEMORY_DESCRIPTOR       *MemoryMapEntry;
  EFI_MEMORY_DESCRIPTOR       *MemoryMapEnd;
  EFI_STATUS                  Status;

  DEBUG ((EFI_D_INFO, "SetMemorySpaceAttributesDefault\n"));

  MemoryMapEntry = MemoryMap;
  MemoryMapEnd   = (EFI_MEMORY_DESCRIPTOR *) ((UINT8 *) MemoryMap + MemoryMapSize);
  while ((UINTN)MemoryMapEntry < (UINTN)MemoryMapEnd) {
    if (MemoryMapEntry->PhysicalStart < BASE_1MB) {
      //
      // Do not touch memory space below 1MB
      //
      MemoryMapEntry = NEXT_MEMORY_DESCRIPTOR (MemoryMapEntry, DescriptorSize);
      continue;
    }
    switch (MemoryMapEntry->Type) {
    case EfiRuntimeServicesCode:
    case EfiRuntimeServicesData:
      //
      // should be handled later;
      //
      break;
    case EfiReservedMemoryType:
      //
      // Handle EfiReservedMemoryType, because there might be firmware executable there.
      //
      DEBUG ((EFI_D_INFO, "SetMemorySpaceAttributes - %016lx - %016lx (%016lx) ...\n",
        MemoryMapEntry->PhysicalStart,
        MemoryMapEntry->PhysicalStart + EfiPagesToSize (MemoryMapEntry->NumberOfPages),
        MemoryMapEntry->Attribute
        ));
      Status = gDS->SetMemorySpaceCapabilities (
                      MemoryMapEntry->PhysicalStart,
                      EfiPagesToSize (MemoryMapEntry->NumberOfPages),
                      ExecuteProtect ? (MemoryMapEntry->Attribute | EFI_MEMORY_XP) : (MemoryMapEntry->Attribute & ~EFI_MEMORY_XP)
                      );
      DEBUG ((EFI_D_INFO, "SetMemorySpaceCapabilities - %r\n", Status));
      break;
    case EfiACPIMemoryNVS:
      //
      // Platform defualt policy doesn't protect EfiACPIMemoryNVS
      // (Set this type memory to EFI_MEMORY_XP may cause some system cannot shutdown.)
      //
      break;
    }

    MemoryMapEntry = NEXT_MEMORY_DESCRIPTOR (MemoryMapEntry, DescriptorSize);
  }

  return;
}

/**
  This function uses to check the allocated memory size of runtime types memory is whether over than
  pre-allocated memory size of runtime memory types.

  @param[out] MemoryMapSize   A pointer to the size, in bytes, of the MemoryMap buffer.
  @param[out] DescriptorSize  A pointer to the location in which firmware returns the size, in
                              bytes, of an individual EFI_MEMORY_DESCRIPTOR.

  @return  A pointer to the current memory map is returned.
           NULL is returned if space for the memory map could not be allocated from pool.
           It is up to the caller to free the memory if they are no longer needed.
**/
STATIC
EFI_MEMORY_DESCRIPTOR *
GetMemoryMap (
  OUT UINTN                  *MemoryMapSize,
  OUT UINTN                  *DescriptorSize
  )
{
  EFI_STATUS                Status;
  EFI_MEMORY_DESCRIPTOR     *EfiMemoryMap;
  UINTN                     EfiMemoryMapSize;
  UINTN                     EfiMapKey;
  UINTN                     EfiDescriptorSize;
  UINT32                    EfiDescriptorVersion;

  EfiMemoryMapSize = 0;
  EfiMemoryMap     = NULL;
  Status = gBS->GetMemoryMap (
                  &EfiMemoryMapSize,
                  EfiMemoryMap,
                  &EfiMapKey,
                  &EfiDescriptorSize,
                  &EfiDescriptorVersion
                  );
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return NULL;
  }

  EfiMemoryMapSize += SIZE_1KB;
  EfiMemoryMap = AllocatePool (EfiMemoryMapSize);
  if (EfiMemoryMap == NULL) {
    return NULL;
  }

  Status = gBS->GetMemoryMap (
                  &EfiMemoryMapSize,
                  EfiMemoryMap,
                  &EfiMapKey,
                  &EfiDescriptorSize,
                  &EfiDescriptorVersion
                  );
  if (Status != EFI_SUCCESS) {
    FreePool (EfiMemoryMap);
    return NULL;
  }

  *MemoryMapSize  = EfiMemoryMapSize;
  *DescriptorSize = EfiDescriptorSize;
  return EfiMemoryMap;
}


/**
  Internal function to get image buffer from input image handle.

  @param[in]   ImageHandle    Input image handle.
  @param[out]  BufferSize     pointer Image buffer size, in bytes.
  @param[out]  ImageBuffer    Pointer to save the address of image buffer.

  @retval  EFI_SUCCESS        Get image buffer and image size from image successfully.
  @retval  EFI_NOT_FOUND      Cannot find gEfiLoadedImageProtocolGuid in image handle.
**/
STATIC
EFI_STATUS
GetImageBuffer (
  IN   EFI_HANDLE  ImageHandle,
  OUT  UINTN       *BufferSize,
  OUT  VOID        **ImageBuffer
  )
{
  EFI_STATUS                 Status;
  EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage;

  Status = gBS->HandleProtocol (
                  ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **)&LoadedImage
                  );
  if (Status != EFI_SUCCESS) {
    return Status;
  }
  *BufferSize  = (UINTN) LoadedImage->ImageSize;
  *ImageBuffer = LoadedImage->ImageBase;
  return EFI_SUCCESS;
}

/**
  Internal function to get resource data from image buffer.

  @param[in]  ImageBuffer     The start address of input buffer.
  @param[in]  BufferSize      Image buffer size, in bytes.

  @return  The start address of resource data or NULL if cannot find resource data
           in image buffer.
**/
STATIC
VOID *
GetResourceData (
  IN  CONST VOID                   *ImageBuffer,
  IN        UINTN                  BufferSize
  )
{
  EFI_IMAGE_DOS_HEADER                  *DosHeader;
  EFI_IMAGE_OPTIONAL_HEADER_UNION       *Hdr;
  UINT16                                Magic;
  EFI_IMAGE_DATA_DIRECTORY              *DataDir;

  if (ImageBuffer == NULL || BufferSize == 0) {
    return NULL;
  }
  //
  // Check Dos header
  //
  DosHeader = (EFI_IMAGE_DOS_HEADER *) ImageBuffer;
  if (BufferSize < sizeof (EFI_IMAGE_DOS_HEADER) || DosHeader->e_magic != EFI_IMAGE_DOS_SIGNATURE) {
    return NULL;
  }
  //
  // Check optional header
  //
  Hdr = (EFI_IMAGE_OPTIONAL_HEADER_UNION *) ((UINT8 *) ImageBuffer + DosHeader->e_lfanew);
  if (BufferSize - DosHeader->e_magic < sizeof (EFI_IMAGE_OPTIONAL_HEADER_UNION) ||
      Hdr->Pe32.Signature != EFI_IMAGE_NT_SIGNATURE) {
    return NULL;
  }
  //
  // Check PE32 or PE32+ magic
  //
  if (Hdr->Pe32.OptionalHeader.Magic != EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
      Hdr->Pe32.OptionalHeader.Magic != EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
    return NULL;
  }
  //
  // Check resource section
  //
  if (Hdr->Pe32.FileHeader.Machine == EFI_IMAGE_MACHINE_IA64 &&
      Hdr->Pe32.OptionalHeader.Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    Magic = EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC;
  } else {
    Magic = Hdr->Pe32.OptionalHeader.Magic;
  }
  if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    DataDir = (EFI_IMAGE_DATA_DIRECTORY *) &Hdr->Pe32.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_RESOURCE];
  } else {
    DataDir = (EFI_IMAGE_DATA_DIRECTORY *) &Hdr->Pe32Plus.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_RESOURCE];
  }
  if (DataDir->Size == 0 || DataDir->VirtualAddress > BufferSize) {
    return NULL;
  }

 return (VOID *) ((UINT8 *) ImageBuffer + DataDir->VirtualAddress);
}

/**
  Get version type resource directory

  @param[in]  ResourceData       The pointer of resource data
  @param[out] VersionDir         The double pointer of version type resource directory

  @retval EFI_SUCCESS            Successfully get version type resource directory
  @retval EFI_INVALID_PARAMETER  Input pointer is NULL
  @retval EFI_NOT_FOUND          There is no version type resource directory
**/
STATIC
EFI_STATUS
GetVersionResourceDir (
  IN  CONST UINT8                                 *ResourceData,
  OUT       EFI_IMAGE_RESOURCE_DIRECTORY          **VersionDir
  )
{
  EFI_IMAGE_RESOURCE_DIRECTORY              *Dir;
  EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY        *DirEntry;
  UINT16                                    Index;
  UINT16                                    Count;

  if (ResourceData == NULL || VersionDir == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Dir      = (EFI_IMAGE_RESOURCE_DIRECTORY *) ResourceData;
  DirEntry = (EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY *) ((UINT8 *) Dir + sizeof (EFI_IMAGE_RESOURCE_DIRECTORY));

  Count = Dir->NumberOfNamedEntries + Dir->NumberOfIdEntries;
  for (Index = 0; Index < Count; Index++) {
    if (DirEntry[Index].u1.Id == RESOURCE_SECTION_RESOURCE_TYPE_VERSION &&
        DirEntry[Index].u2.s.DataIsDirectory) {
      *VersionDir      = (EFI_IMAGE_RESOURCE_DIRECTORY *) (ResourceData + DirEntry[Index].u2.s.OffsetToDirectory);
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Get Microsoft OS most significant version from version directory.

  @param[in]  ResourceData       The pointer of resource data.
  @param[in]  ResourceDataOffset The offset in bytes of Image buffer to resource data.
  @param[in]  VersionDir         pointer of version type resource directory.
  @param[out] VersionMS          The most significant 32 bits of the binary version number of Windows OS.

  @retval EFI_SUCCESS            Successfully get version type resource directory
  @retval EFI_INVALID_PARAMETER  Input pointer is NULL
  @retval EFI_NOT_FOUND          There is no version type resource directory
**/
STATIC
EFI_STATUS
GetWinOsVersionFromVersionDir (
  IN  CONST UINT8                                 *ResourceData,
  IN        UINTN                                 ResourceDataOffset,
  IN  CONST EFI_IMAGE_RESOURCE_DIRECTORY          *VersionDir,
  OUT       UINT32                                *VersionMS
  )
{

  EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY    *VersionDirEntry;
  UINT16                                Index;
  UINT16                                Count;
  EFI_IMAGE_RESOURCE_DIRECTORY          *Dir;
  EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY    *DirEntry;
  EFI_IMAGE_RESOURCE_DATA_ENTRY         *DataEntry;
  UINT8                                 *Ptr;
  UINT8                                 *VersionInfo;
  CHAR16                                *KeyStr;
  UINT16                                MaxPaddingCount;
  VS_FIXEDFILEINFO                      *FixedFileInfo;

  if (ResourceData == NULL || VersionDir == NULL || VersionMS == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Get Microsoft OS version from resource directory
  //
  VersionDirEntry = (EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY *) (VersionDir + 1);
  Count           = VersionDir->NumberOfNamedEntries + VersionDir->NumberOfIdEntries;
  for (Index = 0; Index < Count; Index++) {
    if (!VersionDirEntry[Index].u2.s.DataIsDirectory) {
      continue;
    }
    Dir      = (EFI_IMAGE_RESOURCE_DIRECTORY *) (ResourceData + VersionDirEntry[Index].u2.s.OffsetToDirectory);
    DirEntry = (EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY *) ((UINT8 *) Dir + sizeof (EFI_IMAGE_RESOURCE_DIRECTORY));
    if (DirEntry->u2.s.DataIsDirectory) {
      continue;
    }
    DataEntry = (EFI_IMAGE_RESOURCE_DATA_ENTRY *) (ResourceData + DirEntry->u2.OffsetToData);
    if (DataEntry->OffsetToData < ResourceDataOffset) {
      continue;
    }
    //
    // Get version info data entry VS_VERSIONINFO by comparing key string.
    // Key string is located after VersionInfoSize (UINT16), ValueSize (UINT16) and Type (UINT16).
    //
    VersionInfo = (UINT8 *) ResourceData + (DataEntry->OffsetToData - ResourceDataOffset);
    KeyStr      = (CHAR16 *) (VersionInfo + sizeof (UINT16) + sizeof (UINT16) + sizeof (UINT16));
    if (StrCmp (KeyStr, L"VS_VERSION_INFO") != 0) {
      continue;
    }
    Ptr  = (UINT8 *) KeyStr;
    Ptr += StrSize (KeyStr);

    MaxPaddingCount = 3;
    while (*Ptr == 0 && MaxPaddingCount > 0) {
      Ptr++;
      MaxPaddingCount--;
    }
    if (MaxPaddingCount == 0) {
      continue;
    }

    FixedFileInfo = (VS_FIXEDFILEINFO *) Ptr;
    if (FixedFileInfo->Signature != VS_FIXEDFILEINFO_SIGNATURE) {
      continue;
    }

    *VersionMS = FixedFileInfo->ProductVersionMS;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}
/**
  Get the binary version number of Windows OS

  @param[in]  ImageBuffer        The pointer of image buffer.
  @param[in]  BufferSize         Input buffer size.
  @param[out] VersionMS          The most significant 32 bits of the binary version number of Windows OS

  @retval EFI_SUCCESS            Successfully get binary version number
  @retval EFI_INVALID_PARAMETER  Input pointer is NULL or data size is zero
  @retval EFI_NOT_FOUND          There is no version number information in image
**/
EFI_STATUS
GetWindowsOsVer (
  IN  CONST VOID                    *ImageBuffer,
  IN        UINTN                   BufferSize,
  OUT       UINT32                  *VersionMS
  )
{
  UINT8                                 *ResourceData;
  UINTN                                 ResoruceDataOffset;
  EFI_STATUS                            Status;
  EFI_IMAGE_RESOURCE_DIRECTORY          *VersionDir;


  if (ImageBuffer == NULL || BufferSize == 0 || VersionMS == 0) {
    return EFI_INVALID_PARAMETER;
  }

  ResourceData = GetResourceData (ImageBuffer, BufferSize);
  if (ResourceData == NULL) {
    return EFI_NOT_FOUND;
  }
  VersionDir = NULL;
  Status = GetVersionResourceDir (ResourceData, &VersionDir);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }
  ResoruceDataOffset = (ResourceData - (UINT8 *) ImageBuffer);
  return GetWinOsVersionFromVersionDir (ResourceData, ResoruceDataOffset, VersionDir, VersionMS);
}

/**
  Internal function to check booting OS whether supports runtime memory protection feature.

  @param[in]  ImageHandle        Handle of image to be started.

  @retval TRUE     Booting OS supports runtime memory protection feature.
  @retval FALSE    Booting OS doesn't support runtime memory protection feature.
**/
STATIC
BOOLEAN
SupportRtMemoryProtect (
  IN  EFI_HANDLE                  ImageHandle
  )
{
  EFI_STATUS      Status;
  UINTN           BufferSize;
  UINT8           *ImageBuffer;
  UINT32          VersionMS;

  Status = GetImageBuffer (ImageHandle, &BufferSize, (VOID **) &ImageBuffer);
  if (Status != EFI_SUCCESS) {
    return FALSE;
  }
  Status = GetWindowsOsVer (ImageBuffer, BufferSize, &VersionMS);
  if (Status != EFI_SUCCESS) {
    return FALSE;
  }

  return VersionMS >= MICROSOFT_OS_VERSION_WINDOWS_10;
}


/**
  Internal function to set proterties table value

  @param[in] RtMemoryProtection  BOOLEAN value to enable or disable
                                 EFI_PROPERTIES_RUNTIME_MEMORY_PROTECTION_NON_EXECUTABLE_PE_DATA bit.

  @retval EFI_SUCCES             Set properties table value successfully.
  @retval EFI_NOT_FOUND          Cannot find properties table
**/
STATIC
EFI_STATUS
SetPropertiesTable (
  IN  BOOLEAN    RtMemoryProtection
  )
{
  EFI_PROPERTIES_TABLE        *PropertiesTable;
  EFI_STATUS                  Status;

  PropertiesTable = NULL;
  Status = EfiGetSystemConfigurationTable (&gEfiPropertiesTableGuid, (VOID **) &PropertiesTable);
  if (EFI_ERROR (Status) || PropertiesTable == NULL) {
    return EFI_NOT_FOUND;
  }

  if (RtMemoryProtection) {
    PropertiesTable->MemoryProtectionAttribute |= EFI_PROPERTIES_RUNTIME_MEMORY_PROTECTION_NON_EXECUTABLE_PE_DATA;
  } else {
    PropertiesTable->MemoryProtectionAttribute &= ~((UINT64) EFI_PROPERTIES_RUNTIME_MEMORY_PROTECTION_NON_EXECUTABLE_PE_DATA);
  }

  return EFI_SUCCESS;
}

/**
  Internal function to auto adjust runtime memory protection feature.

  @param[in]  ImageHandle        Handle of image to be started.
**/
STATIC
VOID
AutoAdjustRutimeMemoryProtection (
  IN  EFI_HANDLE                  ImageHandle
  )
{
  EFI_MEMORY_DESCRIPTOR         *MemoryMap;
  UINTN                         MemoryMapSize;
  UINTN                         DescriptorSize;

  MemoryMap = GetMemoryMap (&MemoryMapSize, &DescriptorSize);
  if (MemoryMap == NULL) {
    return;
  }

  if (PcdGet8 (PcdRuntimeMemoryProtection) == RUNTIME_MEMORY_PROTECTION_DISABLED ||
      (PcdGet8 (PcdRuntimeMemoryProtection) == RUNTIME_MEMORY_PROTECTION_AUTO && !SupportRtMemoryProtect (ImageHandle))) {
    SetPropertiesTable (FALSE);
    SetMemorySpaceAttributesDefault (MemoryMap, MemoryMapSize, DescriptorSize, FALSE);
  } else {
    SetPropertiesTable (TRUE);
    SetMemorySpaceAttributesDefault (MemoryMap, MemoryMapSize, DescriptorSize, TRUE);
  }
  FreePool (MemoryMap);
}


/**
  Transfers control to a loaded image's entry point.

  @param[in]  ImageHandle        Handle of image to be started.
  @param[in]  ExitDataSize       The pointer to the size, in bytes, of ExitData.
  @param[in]  ExitData           The pointer to a pointer to a data buffer that includes a Null-terminated
                                 string, optionally followed by additional binary data.

  @retval EFI_INVALID_PARAMETER  ImageHandle is either an invalid image handle or the image
                                 has already been initialized with StartImage.
  @retval EFI_SECURITY_VIOLATION The current platform policy specifies that the image should not be started.
  @return Exit code from image
**/
STATIC
EFI_STATUS
EFIAPI
StartImageCheckImage (
  IN  EFI_HANDLE                  ImageHandle,
  OUT UINTN                       *ExitDataSize,
  OUT CHAR16                      **ExitData    OPTIONAL
  )
{


  if (mPropertiesTableEnable && mBootingImage) {
    AutoAdjustRutimeMemoryProtection (ImageHandle);
  }
  return mOrgStartImage (ImageHandle, ExitDataSize, ExitData);
}

/**
  Internal function to hook original StartImage function.
**/
STATIC
VOID
HookStartImage (
  VOID
  )
{
  mOrgStartImage = gBS->StartImage;
  gBS->StartImage = StartImageCheckImage;
  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 ((UINT8 *)gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
}


/**
  Update memory attributes according to default policy.

  @param[in]  Event     The Event this notify function registered to.
  @param[in]  Context   Pointer to the context data registered to the Event.
**/
VOID
EFIAPI
UpdateMemoryAttributesDefault (
  EFI_EVENT                               Event,
  VOID                                    *Context
  )
{
  EFI_STATUS                  Status;
  EFI_MEMORY_DESCRIPTOR       *MemoryMap;
  UINTN                       MemoryMapSize;
  UINTN                       DescriptorSize;
  EFI_PROPERTIES_TABLE        *PropertiesTable;

  gBS->CloseEvent (Event);
  HookStartImage ();
  DEBUG ((EFI_D_INFO, "UpdateMemoryAttributesDefault\n"));
  PropertiesTable = NULL;
  Status = EfiGetSystemConfigurationTable (&gEfiPropertiesTableGuid, (VOID **) &PropertiesTable);
  if (EFI_ERROR (Status) || PropertiesTable == NULL) {
    return;
  }

  DEBUG ((EFI_D_INFO, "MemoryProtectionAttribute - 0x%016lx\n", PropertiesTable->MemoryProtectionAttribute));
  if ((PropertiesTable->MemoryProtectionAttribute & EFI_PROPERTIES_RUNTIME_MEMORY_PROTECTION_NON_EXECUTABLE_PE_DATA) == 0) {
    return;
  }
  //
  // Save properties tabble feature status.
  //
  mPropertiesTableEnable = TRUE;
  //
  // Set memory space attribute by default policy
  //
  MemoryMap = GetMemoryMap (&MemoryMapSize, &DescriptorSize);
  if (MemoryMap == NULL) {
    return;
  }
  SetMemorySpaceAttributesDefault (MemoryMap, MemoryMapSize, DescriptorSize, TRUE);
  FreePool (MemoryMap);
}

/**
  The entrypoint of properties table attribute driver.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       It always returns EFI_SUCCESS.
**/
EFI_STATUS
EFIAPI
InitializePropertiesTableAttributesDxe (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS          Status;
  EFI_EVENT           ReadyToBootEvent;
  H2O_BDS_CP_HANDLE   CpHandle;

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  UpdateMemoryAttributesDefault,
                  NULL,
                  &gEfiEventReadyToBootGuid,
                  &ReadyToBootEvent
                  );
  ASSERT_EFI_ERROR (Status);
  BdsCpRegisterHandler (
    &gH2OBdsCpBootBeforeProtocolGuid,
    BootBeforeCpHandler,
    H2O_BDS_CP_MEDIUM,
    &CpHandle
    );

  return EFI_SUCCESS;
}

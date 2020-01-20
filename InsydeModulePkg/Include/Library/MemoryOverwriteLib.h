/** @file
  Memory Overwrite Library
  
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

#ifndef _MEMORY_OVERWRITE_LIB_H_
#define _MEMORY_OVERWRITE_LIB_H_

#include <Uefi.h>

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
  );

/**
  Overwirte memory below 1MB with meaningless data.

  @retval EFI_SUCCESS               Memory overwrite succeeded.
  @retval Others                    Operation failed.

**/
EFI_STATUS
EFIAPI
MemoryOverwriteBelow1M (
  VOID
  );

#endif //_MEMORY_OVERWRITE_LIB_H_
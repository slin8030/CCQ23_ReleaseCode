/** @file
  PlatformBdsLib

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/**
  Platform BDS library definition. A platform can implement
  instances to support platform-specific behavior.

Copyright (c) 2008 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under
the terms and conditions of the BSD License that accompanies this distribution.
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __PLATFORM_BDS_LIB_H_
#define __PLATFORM_BDS_LIB_H_

#include <Protocol/GenericMemoryTest.h>
#include <Library/GenericBdsLib.h>

//
// The size of a 3 character ISO639 language code.
//
#define ISO_639_2_ENTRY_SIZE   3

/**
  Perform the memory test base on the memory test intensive level,
  and update the memory resource.

  @param  Level         The memory test intensive level.

  @retval EFI_STATUS    Successfully test all the system memory, and update
                        the memory resource

**/
typedef
EFI_STATUS
(EFIAPI *BASEM_MEMORY_TEST)(
  IN EXTENDMEM_COVERAGE_LEVEL Level,
  IN UINTN                    BaseLine
  );

/**
  This routine is called to see if there are any capsules we need to process.
  If the boot mode is not UPDATE, then we do nothing. Otherwise, find the
  capsule HOBS and produce firmware volumes for them via the DXE service.
  Then call the dispatcher to dispatch drivers from them. Finally, check
  the status of the updates.

  This function should be called by BDS in case we need to do some
  sort of processing even if there is no capsule to process. We
  need to do this if an earlier update went away and we need to
  clear the capsule variable so on the next reset PEI does not see it and
  think there is a capsule available.

  @param BootMode                 The current boot mode

  @retval EFI_INVALID_PARAMETER   The boot mode is not correct for an update.
  @retval EFI_SUCCESS             There is no error when processing a capsule.

**/
typedef
EFI_STATUS
(EFIAPI *PROCESS_CAPSULES)(
  IN EFI_BOOT_MODE BootMode
  );

/**
  Platform Bds initialization. Includes the platform firmware vendor, revision
  and so crc check.

**/
VOID
EFIAPI
PlatformBdsInit (
  VOID
  );

/**
  The function will excute with as the platform policy, current policy
  is driven by boot mode. IBV/OEM can customize this code for their specific
  policy action.

  @param  DriverOptionList        The header of the driver option link list
  @param  BootOptionList          The header of the boot option link list
  @param  ProcessCapsules         A pointer to ProcessCapsules()
  @param  BaseMemoryTest          A pointer to BaseMemoryTest()

**/
VOID
EFIAPI
PlatformBdsPolicyBehavior (
  IN LIST_ENTRY                      *DriverOptionList,
  IN LIST_ENTRY                      *BootOptionList,
  IN PROCESS_CAPSULES                ProcessCapsules,
  IN BASEM_MEMORY_TEST               BaseMemoryTest
  );


/**
  This function locks platform flash that is not allowed to be updated during normal boot path.
  The flash layout is platform specific.

  **/
VOID
EFIAPI
PlatformBdsLockNonUpdatableFlash (
  VOID
  );

EFI_STATUS
InitializeFrontPage (
  IN BOOLEAN                         InitializeHiiData
  );

VOID
PlatformBdsEnterFrontPage (
  IN UINT16                       TimeoutDefault,
  IN BOOLEAN                      ConnectAllHappened
  );
#endif

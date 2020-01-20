/** @file
  A Pei Timer Library implementation which uses the Time Stamp Counter in the processor.

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

  Copyright (c) 2009 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiPei.h>
#include <Library/HobLib.h>
#include <Guid/TscFrequency.h>
#include "TscTimerLibInternal.h"

EFI_STATUS
EFIAPI
PeiTscTimerLibConstructor (
  IN EFI_PEI_FILE_HANDLE        FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_HOB_GUID_TYPE       *GuidHob;
  UINT64                  TscFrequency;

  //
  // In PeiCore library constructor, hob not ready
  //
  if (FileHandle == NULL) {
    return EFI_SUCCESS;
  }

  //
  // Get TSC frequency from TSC frequency GUID HOB.
  //
  GuidHob = GetFirstGuidHob (&gEfiTscFrequencyGuid);
  if (GuidHob != NULL) {
    return EFI_SUCCESS;
  }

  //
  // TSC frequency GUID HOB is not found, build it.
  //
  TscFrequency = InternalCalculateTscFrequency ();

  //
  // TscFrequency is now equal to the number of TSC counts per second, build GUID HOB for it.
  //
  GuidHob = BuildGuidDataHob (
              &gEfiTscFrequencyGuid,
              &TscFrequency,
              sizeof (UINT64)
              );
  if (GuidHob == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

/**  Get TSC frequency from TSC frequency GUID HOB, if the HOB is not found, build it.

  @return The number of TSC counts per second.

**/
UINT64
InternalGetTscFrequency (
  VOID
  )
{
  EFI_HOB_GUID_TYPE       *GuidHob;
  UINT64                  TscFrequency;

  //
  // Get TSC frequency from TSC frequency GUID HOB.
  //
  GuidHob = GetFirstGuidHob (&gEfiTscFrequencyGuid);
  if (GuidHob != NULL) {
    TscFrequency = *(UINT64 *) GET_GUID_HOB_DATA (GuidHob);
    return TscFrequency;
  }

  //
  // TSC frequency GUID HOB is not found, build it.
  //
  TscFrequency = InternalCalculateTscFrequency ();

  //
  // TscFrequency is now equal to the number of TSC counts per second, build GUID HOB for it.
  //
  BuildGuidDataHob (
    &gEfiTscFrequencyGuid,
    &TscFrequency,
    sizeof (UINT64)
    );
  return TscFrequency;
}


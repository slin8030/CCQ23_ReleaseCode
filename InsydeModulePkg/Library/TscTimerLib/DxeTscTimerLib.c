/** @file
  A Dxe Timer Library implementation which uses the Time Stamp Counter in the processor.

;******************************************************************************
;* Copyright (c) 2015 - 2016, Insyde Software Corp. All Rights Reserved.
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

#include <PiDxe.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Guid/TscFrequency.h>
#include "TscTimerLibInternal.h"

UINT64 mTscFrequency;

/** The constructor function determines the actual TSC frequency.

  First, Get TSC frequency from system configuration table with TSC frequency GUID,
  if the table is not found, install it.
  This function will always return EFI_SUCCESS.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
DxeTscTimerLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_HOB_GUID_TYPE       *GuidHob;
  EFI_STATUS              Status;
  UINT64                  *TscFrequency;

  TscFrequency = NULL;

  //
  // Get TSC frequency from system configuration table with TSC frequency GUID.
  //
  Status = EfiGetSystemConfigurationTable (&gEfiTscFrequencyGuid, (VOID **) &TscFrequency);
  if (Status == EFI_SUCCESS && TscFrequency != NULL) {
    mTscFrequency = *TscFrequency;
    return EFI_SUCCESS;
  }

  //
  // TSC frequency GUID system configuration table is not found, install it.
  //
  Status = gBS->AllocatePool (EfiBootServicesData, sizeof (UINT64), (VOID **) &TscFrequency);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get TSC frequency
  //
  GuidHob = GetFirstGuidHob (&gEfiTscFrequencyGuid);
  if (GuidHob != NULL) {
    *TscFrequency = *(UINT64 *) GET_GUID_HOB_DATA (GuidHob);
  } else {
    *TscFrequency = InternalCalculateTscFrequency ();
  }

  //
  // TscFrequency now points to the number of TSC counts per second, install system configuration table for it.
  //
  Status = gBS->InstallConfigurationTable (&gEfiTscFrequencyGuid, TscFrequency);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mTscFrequency = *TscFrequency;
  return EFI_SUCCESS;
}

/**  Get TSC frequency.

  @return The number of TSC counts per second.

**/
UINT64
InternalGetTscFrequency (
  VOID
  )
{
  return mTscFrequency;
}


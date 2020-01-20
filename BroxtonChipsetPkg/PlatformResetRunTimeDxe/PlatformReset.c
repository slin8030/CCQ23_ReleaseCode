/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c)  2011 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

@file:

  PlatformReset.c

@brief:

  Provide the ResetSystem AP.

**/
#include <Uefi.h>
//#include <TianoApi.h>
#include <Guid/EventGroup.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeOemSvcChipsetLib.h>

EFI_RESET_SYSTEM mOrgResetPtr;

/**
  Reset the system

  @param[in] ResetType            Warm or cold
  @param[in] ResetStatus          Possible cause of reset
  @param[in] DataSize             Size of ResetData in bytes
  @param[in] ResetData            Optional Unicode string

  @retval Does not return if the reset takes place.
**/
VOID
EFIAPI
PlatformResetSystem (
  IN EFI_RESET_TYPE                   ResetType,
  IN EFI_STATUS                       ResetStatus,
  IN UINTN                            DataSize,
  IN CHAR16                           *ResetData
  )
{
  EFI_STATUS                          Status;

  Status = OemSvcHookPlatformReset (
             ResetType,
             ResetStatus
             );
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "DxeOemSvcChipsetLib OemSvcHookPlatformReset, Status : %r\n", Status ) );
  if (Status == EFI_SUCCESS) {
    return;
  }

  mOrgResetPtr (ResetType, ResetStatus, DataSize, ResetData);
}

VOID
PchResetVirtualAddressChangeEvent (
  IN EFI_EVENT                    Event,
  IN VOID                         *Context
  )
{
  gRT->ConvertPointer (0x0, (VOID **) &mOrgResetPtr);
}

/**
  Entry point of Platform Reset driver.

  @param[in] ImageHandle          Standard entry point parameter
  @param[in] SystemTable          Standard entry point parameter

  @retval EFI_SUCCESS             Reset RT protocol installed
  @retval All other error conditions encountered result in an ASSERT
**/
EFI_STATUS
EFIAPI
InitializePlatformReset (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                      Status;
  EFI_EVENT                       AddressChangeEvent;
  
  mOrgResetPtr = SystemTable->RuntimeServices->ResetSystem;

  ///
  /// Hook the runtime service table
  ///
  SystemTable->RuntimeServices->ResetSystem = PlatformResetSystem;

  Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    PchResetVirtualAddressChangeEvent,
                    NULL,
                    &gEfiEventVirtualAddressChangeGuid,
                    &AddressChangeEvent
                    );

  return Status;
}


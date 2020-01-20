/** @file
  DXE CSE Variable Storage Selector Library

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#include "CseVariableStorageSelectorLibInternal.h"

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
//[-start-160812-IB07220128-add]//
#include <Guid/EventGroup.h>
//[-end-160812-IB07220128-add]//
//[-start-161020-IB07400800-add]//
#include <Library/PcdLib.h>
//[-end-161020-IB07400800-add]//


/**
  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It converts pointers to a new virtual address.

  @param  Event        Event whose notification function is being invoked.
  @param  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
CseVariableStorageSelectorAddressChangeEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  gRT->ConvertPointer (0, (VOID **) &mHeci2Protocol);
}


VOID
EFIAPI
CseVariableStorageSelectorEndOfDxeNotification (
  IN EFI_EVENT    Event,
  IN VOID*        Context
  )
{
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "CseVariableStorageSelectorLib: End of DXE event.\n"));

  Status = gBS->LocateProtocol (
                  &gEfiHeciSmmRuntimeProtocolGuid,
                  NULL,
                  &mHeci2Protocol
                  );
//[-start-170210-IB07400839-remove]//
//  ASSERT_EFI_ERROR (Status);
//[-end-170210-IB07400839-remove]//

  gBS->CloseEvent (Event);
}

/**
  The library constructor.

  @param[in]  ImageHandle       The firmware allocated handle for the UEFI image.
  @param[in]  SystemTable       A pointer to the EFI system table.

  @retval     EFI_SUCCESS       The function always return EFI_SUCCESS for now.
                                It will ASSERT on error for debug version.
  @retval     EFI_ERROR         Please reference LocateProtocol for error code details.
**/
EFI_STATUS
EFIAPI
CseVariableStorageLibInit (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_EVENT   Event;
  EFI_STATUS  Status;

//[-start-161020-IB07400800-add]//
  if (PcdGetBool (PcdFDOState) == 1) {
    return EFI_SUCCESS;
  }
//[-end-161020-IB07400800-add]//

  //
  // Create the End of DXE event
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  CseVariableStorageSelectorEndOfDxeNotification,
                  NULL,
                  &gEfiEndOfDxeEventGroupGuid,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Register the virtual address change event
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  CseVariableStorageSelectorAddressChangeEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
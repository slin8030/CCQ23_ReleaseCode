/** @file
  Implements CSE Variable Storage Services and installs
  an instance of the VariableStorage Runtime DXE protocol.

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

#include "CseVariableStorage.h"

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
//[-start-160812-IB07220128-add]//
#include <Guid/EventGroup.h>
//[-end-160812-IB07220128-add]//

//
// Module globals
//
static EFI_EVENT      mVirtualAddressChangeEvent = NULL;

/**
  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It converts pointers to a new virtual address.

  @param  Event        Event whose notification function is being invoked.
  @param  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
CseVariableStorageAddressChangeEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  CSE_VARIABLE_FILE_TYPE  Type;

//[-start-161003-IB07220139-add]//
  EfiConvertPointer (0x0, (VOID **) &mCseVariableStorageProtocol.GetAuthenticatedSupport);
  EfiConvertPointer (0x0, (VOID **) &mCseVariableStorageProtocol.GetAuthenticatedVariable);
  EfiConvertPointer (0x0, (VOID **) &mCseVariableStorageProtocol.GetId);
  EfiConvertPointer (0x0, (VOID **) &mCseVariableStorageProtocol.GetNextVariableName);
  EfiConvertPointer (0x0, (VOID **) &mCseVariableStorageProtocol.GetStorageUsage);
  EfiConvertPointer (0x0, (VOID **) &mCseVariableStorageProtocol.GetVariable);
  EfiConvertPointer (0x0, (VOID **) &mCseVariableStorageProtocol.RegisterWriteServiceReadyCallback);
  EfiConvertPointer (0x0, (VOID **) &mCseVariableStorageProtocol.SetVariable);
  EfiConvertPointer (0x0, (VOID **) &mCseVariableStorageProtocol.WriteServiceIsReady);
  EfiConvertPointer (0x0, (VOID **) &mCseVariableStorageProtocol);
//[-end-161003-IB07220139-add]//
  
  for (Type = (CSE_VARIABLE_FILE_TYPE) 0; Type < CseVariableFileTypeMax; Type++) {
    EfiConvertPointer (0x0, (VOID **) &mCseVariableFileInfo[Type]->FileName);
    EfiConvertPointer (0x0, (VOID **) &mCseVariableFileInfo[Type]->HeaderRegionBase);
    EfiConvertPointer (0x0, (VOID **) &mCseVariableFileInfo[Type]);
  }

//[-start-160816-IB07220129-add]//
  EfiConvertPointer (0x0, (VOID **) &mSmmTrustedKey);
//[-end-160816-IB07220129-add]//
}

/**
  Provide DXE functionality of CSE NVM variable storage services.

  @param  ImageHandle  The image handle.
  @param  SystemTable  The system table.

  @retval EFI_SUCCESS  The protocol was installed successfully.
  @retval Others       Protocol could not be installed.
**/
EFI_STATUS
EFIAPI
CseVariableStorageDxeRuntimeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS          Status;
  EFI_HANDLE          Handle                 = NULL;
  BOOLEAN             ForceVolatileVariables = FALSE;

  if (!PcdGetBool (PcdEnableCseVariableStorage)) {
    DEBUG ((EFI_D_INFO, "CSE Variable Storage Protocol is disabled.\n"));
    return EFI_SUCCESS;
  }

  DEBUG ((EFI_D_INFO, "\n\n======\nCSE Variable Storage DXE Protocol Initialization\n======\n\n"));

  ForceVolatileVariables = PcdGetBool (PcdForceVolatileVariable);

  if (!ForceVolatileVariables) {
    Status = CseVariableStorageCommonInitialize ();

    if (EFI_ERROR (Status)) {
      if (Status != EFI_DEVICE_ERROR) {
        ASSERT_EFI_ERROR (Status);
      }
      return Status;
    }

    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    CseVariableStorageAddressChangeEvent,
                    NULL,
                    &gEfiEventVirtualAddressChangeGuid,
                    &mVirtualAddressChangeEvent
                    );
    ASSERT_EFI_ERROR (Status);
  }
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gVariableStorageProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mCseVariableStorageProtocol
                  );

  return Status;
}
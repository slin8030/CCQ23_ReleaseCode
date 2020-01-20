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
#include <Library/SmmServicesTableLib.h>

/**
  Provide SMM functionality of CSE NVM variable storage services.

  @param  ImageHandle  The image handle.
  @param  SystemTable  The system table.

  @retval EFI_SUCCESS  The protocol was installed successfully.
  @retval Others       Protocol could not be installed.
**/
EFI_STATUS
EFIAPI
CseVariableStorageSmmInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS    Status;
  EFI_HANDLE    Handle                 = NULL;
  BOOLEAN       ForceVolatileVariables = FALSE;

  if (!PcdGetBool (PcdEnableCseVariableStorage)) {
    DEBUG ((EFI_D_INFO, "CSE Variable Storage Protocol is disabled.\n"));
    return EFI_SUCCESS;
  }

  ForceVolatileVariables = PcdGetBool (PcdForceVolatileVariable);

  if (!ForceVolatileVariables) {
    Status = CseVariableStorageCommonInitialize ();

    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
  }

  Status = gSmst->SmmInstallProtocolInterface (
                    &Handle,
                    &gVariableStorageProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &mCseVariableStorageProtocol
                    );

  return Status;
}

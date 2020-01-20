/** @file
  SMM CSE Variable Storage Selector Library

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
#include <Library/SmmServicesTableLib.h>
//[-start-160812-IB07220128-add]//
#include <Protocol/SmmEndOfDxe.h>
//[-end-160812-IB07220128-add]//

/**
  SmmEndOfDxeCallback

  @param[in]  Protocol
  @param[in]  Interface
  @param[in]  Handle

  @retval     EFI_STATUS
**/
EFI_STATUS
EFIAPI
CseVariableStorageSelectorSmmEndOfDxeNotification (
  IN CONST EFI_GUID                       *Protocol,
  IN VOID                                 *Interface,
  IN EFI_HANDLE                           Handle
  )
{
  EFI_STATUS  Status;

  Status = gSmst->SmmLocateProtocol (&gEfiHeciSmmProtocolGuid, NULL, (VOID**) &mHeci2Protocol);

//[-start-170210-IB07400839-remove]//
//  ASSERT_EFI_ERROR (Status);
//[-end-170210-IB07400839-remove]//
  DEBUG ((DEBUG_INFO, "HECI2 protocol found at 0x%x.\n", mHeci2Protocol));

  return Status;
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
SmmCseVariableStorageLibInit (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;
  VOID        *SmmEndOfDxeRegistration;

  Status = gSmst->SmmRegisterProtocolNotify (
                    &gEfiSmmEndOfDxeProtocolGuid,
                    CseVariableStorageSelectorSmmEndOfDxeNotification,
                    &SmmEndOfDxeRegistration
                    );
 ASSERT_EFI_ERROR (Status);

  return Status;
}
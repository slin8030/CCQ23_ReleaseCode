/** @file
  Implementation file for Heci Message functionality

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

@par Specification Reference

**/

//
// Allocate non-SMM buffer to support CSE DMA requirement when accessing NVM via HECI1 from SMM.
// BXT / GLK Workaround: Since the address range provided to CSE can't be in SMRAM, must use a non-SMM buffer
//                       when reading/writing NVM file(s). The pre-allocated 64KB buffer initialized in this file
//                       is passed to HeciSetReadWriteCache() to use as a non-SMM buffer.
//
// This library is not needed outside of SMM. It should not be used by PEI / DXE drivers.
//

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
//[-start-160812-IB07220128-add]//
#include <Protocol/SmmEndOfDxe.h>
//[-end-160812-IB07220128-add]//

UINT8       *mNonSmmData = NULL;

/**
  Set Read Write Temp memory.

  @param[in] Address   Temp memory.
**/
VOID
HeciSetReadWriteCache (
  VOID*  Address
  );


/**
  SmmEndOfDxeCallback

  @param[in]  Protocol
  @param[in]  Interface
  @param[in]  Handle

  @retval     EFI_STATUS
**/
EFI_STATUS
EFIAPI
HeciMsgLibSmmEndOfDxeNotification (
  IN CONST EFI_GUID                       *Protocol,
  IN VOID                                 *Interface,
  IN EFI_HANDLE                           Handle
  )
{
  //
  // This pool should be freed. It must be guaranteed
  // to happen after HECI2 has been used.
  //
  //gBS->FreePool (mNonSmmData);

  //HeciSetReadWriteCache (NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HeciMsgLibSmmConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  VOID        *SmmEndOfDxeRegistration;

  Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    EFI_SIZE_TO_PAGES (SIZE_64KB),
                    (EFI_PHYSICAL_ADDRESS *)(UINTN) &mNonSmmData
                    );
  ASSERT_EFI_ERROR (Status);

  if (mNonSmmData == NULL) {
    ASSERT (mNonSmmData != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  HeciSetReadWriteCache (mNonSmmData);

  Status = gSmst->SmmRegisterProtocolNotify (
                    &gEfiSmmEndOfDxeProtocolGuid,
                    HeciMsgLibSmmEndOfDxeNotification,
                    &SmmEndOfDxeRegistration
                    );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
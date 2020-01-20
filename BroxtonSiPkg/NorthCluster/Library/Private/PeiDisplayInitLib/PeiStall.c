/** @file
  PEIM to initialize Early Display.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

#include <Library/DisplayInitLib.h>
#include <Library/TimerLib.h>


#define PEI_STALL_RESOLUTION   1

EFI_PEI_STALL_PPI          mStallPpi = {
  PEI_STALL_RESOLUTION,
  Stall
};

EFI_PEI_PPI_DESCRIPTOR     mPeiPlatformPpis = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiStallPpiGuid,
  &mStallPpi
};


/**
  This function provides a blocking stall for reset at least the given number of microseconds
  stipulated in the final argument.

  @param[in]  PeiServices General purpose services available to every PEIM.
  @param[in]  this Pointer to the local data for the interface.
  @param[in]  Microseconds number of microseconds for which to stall.

  @retval  EFI_SUCCESS the function provided at least the required stall.
**/
EFI_STATUS
EFIAPI
Stall (
  IN CONST EFI_PEI_SERVICES   **PeiServices,
  IN CONST EFI_PEI_STALL_PPI  *This,
  IN UINTN                    Microseconds
  )
{
  MicroSecondDelay (Microseconds);
  return EFI_SUCCESS;
}

/**
  Install stall Ppi.

  @param[in]  None

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
InstallStallPpi(
  )
{
  EFI_STATUS   Status = EFI_SUCCESS;
  DEBUG ((DEBUG_INFO, "Installing Stall Ppi \n"));
  Status = PeiServicesInstallPpi (&mPeiPlatformPpis);
  ASSERT_EFI_ERROR (Status);
  return Status;
}

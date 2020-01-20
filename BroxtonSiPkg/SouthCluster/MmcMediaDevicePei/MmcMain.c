/** @file
  UEFI Driver Entry and Binding support.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

@par Specification
**/
#ifndef __GNUC__
#include "SdHostDriver.h"
#include <MediaDeviceDriverPei.h>
#include <Ppi/SeCUma.h>
#endif

#ifdef FSP_FLAG
#include <Library/PeiMmcMainLib.h>
#endif

UINT32 mSdBaseAddress;

/**

  Entry point for EFI drivers.

  @param[in]  FileHandle  - EFI_PEI_FILE_HANDLE
  @param[in]  PeiServices - EFI_PEI_SERVICES

  @retval  EFI_SUCCESS       - Success
  @retval  EFI_DEVICE_ERROR  - Fail

**/
#ifndef __GNUC__
EFI_STATUS
EFIAPI
MmcMainEntryPoint(
                 IN EFI_PEI_FILE_HANDLE FileHandle,
                 IN CONST EFI_PEI_SERVICES **PeiServices)
{
  EFI_STATUS  Status;
  SEC_UMA_PPI *SeCUma;

  DEBUG((EFI_D_ERROR, "MmcMainEntryPoint Entry \n"));

#ifndef FSP_FLAG
  Status = (*PeiServices)->RegisterForShadow (FileHandle);
  if (Status == EFI_ALREADY_STARTED) {
#endif
    Status = (*PeiServices)->LocatePpi (PeiServices, &gSeCUmaPpiGuid, 0, NULL, (void **)&SeCUma);
    SeCUma->SeCTakeOwnerShip ();
    DEBUG((DEBUG_ERROR, "MmcMainEntryPoint Take Owner Ship \n"));
    mSdBaseAddress = (UINT32)PcdGet32 (PcdEmmcBaseAddress0);
    Status = SdHostDriverEntryPoint (FileHandle, PeiServices);
    if (EFI_ERROR(Status)){
      DEBUG((EFI_D_ERROR, "Fail to Init SD Host controller \n"));
      return Status;
    }
    Status = MediaDeviceDriverEntryPoint (FileHandle, PeiServices);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Fail to Init eMMC Card \n"));
      return Status;
    }
#ifndef FSP_FLAG
  } else if (Status == EFI_NOT_FOUND) {
    ASSERT_EFI_ERROR (Status);
  }
#endif

  DEBUG((EFI_D_ERROR, "MmcMainEntryPoint Exit \n"));

  return EFI_SUCCESS;
}
#else
EFI_STATUS
EFIAPI
MmcMainEntryPoint(
                 IN EFI_PEI_FILE_HANDLE FileHandle,
                 IN CONST EFI_PEI_SERVICES **PeiServices)
{
  return EFI_SUCCESS;
}
#endif //__GNUC__

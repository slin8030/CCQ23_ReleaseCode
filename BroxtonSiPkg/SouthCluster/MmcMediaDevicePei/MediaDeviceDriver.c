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

#include <MediaDeviceDriverPei.h>

/**
  Entry point for EFI drivers.

  @param[in]  EFI_PEI_FILE_HANDLE - FileHandle
  @param[in]  EFI_PEI_SERVICES    - PeiServices

  @retval  EFI_SUCCESS         - Success
  @retval  EFI_DEVICE_ERROR    - Fail

**/
EFI_STATUS
EFIAPI
MediaDeviceDriverEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                Status;
  PEI_CARD_DATA             *PeiCardData;
  EFI_PEI_PCI_CFG2_PPI      *PciCfgPpi;
  PEI_SD_CONTROLLER_PPI     *SdControllerPpi;
  INTN                      Loop;

  Status = PeiServicesLocatePpi (
            &gPeiSdhcPpiGuid,
            0,
            NULL,
            (VOID **) &SdControllerPpi
            );

  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  PciCfgPpi = (**PeiServices).PciCfg;

  Status = SdControllerPpi->DetectCardAndInitHost ( SdControllerPpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Fail to DetectCardAndInitHost \n"));
    return Status;
  }

  PeiCardData = (PEI_CARD_DATA*)AllocateZeroPool (sizeof (PEI_CARD_DATA));
  if (PeiCardData == NULL) {
    Status =  EFI_OUT_OF_RESOURCES;
    DEBUG ((EFI_D_ERROR, "Fail to AllocateZeroPool(CARD_DATA) \n"));
    return Status;
  }

  ASSERT (SdControllerPpi->HostCapability.BoundarySize >= 4 * 1024);
  PeiCardData->RawBufferPointer = (UINT8*)AllocatePages ((2 * SdControllerPpi->HostCapability.BoundarySize)/4096);
  if (PeiCardData->RawBufferPointer == NULL) {
    DEBUG ((EFI_D_ERROR, "MediaDeviceDriverBindingStart: Fail to AllocateZeroPool(2*x) \n"));
    Status =  EFI_OUT_OF_RESOURCES;
    return Status;
  }
  PeiCardData->AlignedBuffer = PeiCardData->RawBufferPointer - ((UINTN)(PeiCardData->RawBufferPointer) & (SdControllerPpi->HostCapability.BoundarySize - 1)) + SdControllerPpi->HostCapability.BoundarySize;

  PeiCardData->Signature = PEI_CARD_DATA_SIGNATURE;
  PeiCardData->SdControllerPpi  = SdControllerPpi;
  for (Loop = 0; Loop < MAX_NUMBER_OF_PARTITIONS; Loop++) {
    PeiCardData->Partitions[Loop].Signature = PEI_CARD_PARTITION_SIGNATURE;
  }
  Status = MMCSDCardInit ( PeiCardData);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Fail to MMCSDCardInit \n"));
    return Status;
  }

  DEBUG ((EFI_D_INFO, "MMC SD card\n"));
  Status = MMCSDBlockIoInit (PeiCardData);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Card BlockIo init failed\n"));
    return Status;
  }

#ifndef FSP_FLAG
  for (Loop = 0; Loop < MAX_NUMBER_OF_PARTITIONS; Loop++) {
    Status = (**PeiServices).InstallPpi (
                               PeiServices,
                               &PeiCardData->Partitions[Loop].BlkIoPpiList
                               );
  }
#else
  //
  // Install the Boot Partition first
  //
  for (Loop = 2; Loop >= 0; Loop--) {
    if (PeiCardData->Partitions[Loop].Present) {
      Status = (**PeiServices).InstallPpi (
                                 PeiServices,
                                 &PeiCardData->Partitions[Loop].BlkIoPpiList
                                 );
    }
  }

  for (Loop = MAX_NUMBER_OF_PARTITIONS - 1; Loop > 2; Loop--) {
    if (PeiCardData->Partitions[Loop].Present) {
      Status = (**PeiServices).InstallPpi (
                                 PeiServices,
                                 &PeiCardData->Partitions[Loop].BlkIoPpiList
                                 );
    }
  }
#endif


  return Status;
}







/** @file
  This file is PeiSiPolicyLib library creates default settings of RC
  Policy and installs RC Policy PPI.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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
#include "PeiSiPolicyLibrary.h"

/**
  SiCreatePolicyDefaults creates the default setting of Silicon Policy.
  It allocates and zero out buffer, and fills in the Intel default settings.

  @param[out] SiPolicyPpi         The pointer to get Silicon Policy PPI instance

  @retval EFI_SUCCESS             The policy default is initialized.
  @retval EFI_OUT_OF_RESOURCES    Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
SiCreatePolicyDefaults (
  OUT  SI_POLICY_PPI **SiPolicyPpi
  )
{
  SI_POLICY_PPI *SiPolicy;

  SiPolicy = (SI_POLICY_PPI *) AllocateZeroPool (sizeof (SI_POLICY_PPI));
  if (SiPolicy == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // General intialization
  //
  SiPolicy->Revision        = SI_POLICY_REVISION;

  //
  // PlatformData configuration
  //
  //
  // FviSmbiosType is the SMBIOS OEM type (0x80 to 0xFF) defined in SMBIOS Type 14 - Group
  // Associations structure - item type. FVI structure uses it as SMBIOS OEM type to provide
  // version information. The default value is type 221.
  //
  SiPolicy->FviSmbiosType   = 0xDD;

//  SiPolicy->EcPresent       = 0;
  //
  // Temporary Bus range for silicon initialization.
  //
  SiPolicy->TempPciBusMin   = 2;
  SiPolicy->TempPciBusMax   = 6;
  //
  // Temporary Memory Base Address for PCI devices to be used to initialize MMIO registers.
  // Minimum size is 2MB bytes.
  //
  SiPolicy->TempMemBaseAddr = TEMP_MEM_BASE_ADDRESS;
  SiPolicy->TempMemSize     = TEMP_MEM_SIZE;
  //
  // Temporary IO Base Address for PCI devices to be used to initialize IO registers.
  // And size of temporary IO space.
  //
  SiPolicy->TempIoBaseAddr  = TEMP_IO_BASE_ADDRESS;
  SiPolicy->TempIoSize      = TEMP_IO_SIZE;

  *SiPolicyPpi = SiPolicy;
  return EFI_SUCCESS;
}

/**
  SiInstallPolicyPpi installs SiPolicyPpi.
  While installed, RC assumes the Policy is ready and finalized. So please update and override
  any setting before calling this function.

  @param[in] SiPolicyPpi         The pointer to Silicon Policy PPI instance

  @retval EFI_SUCCESS            The policy is installed.
  @retval EFI_OUT_OF_RESOURCES   Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
SiInstallPolicyPpi (
  IN  SI_POLICY_PPI *SiPolicyPpi
  )
{
  EFI_STATUS             Status;
  EFI_PEI_PPI_DESCRIPTOR *SiPolicyPpiDesc;
  EFI_PEI_PPI_DESCRIPTOR *OldSiPolicyPpiDesc;

  SiPolicyPpiDesc = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
  if (SiPolicyPpiDesc == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }

  SiPolicyPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  SiPolicyPpiDesc->Guid  = &gSiPolicyPpiGuid;
  SiPolicyPpiDesc->Ppi   = SiPolicyPpi;

  //
  // Print whole SI_POLICY_PPI and serial out.
  //
  SiPrintPolicyPpi (SiPolicyPpi);

  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             &OldSiPolicyPpiDesc,
             NULL
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Locate Old gSiPolicyPpiGuid fail in Post-Memory\n"));
    Status = PeiServicesInstallPpi (SiPolicyPpiDesc);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Install gSiPolicyPpiGuid fail in Post-Memory\n"));
    }
    ASSERT_EFI_ERROR (Status);
  } else {
    //
    // ReInstall Sc Spi PPI
    //
    DEBUG ((DEBUG_INFO, "Re-Install gSiPolicyPpiGuid in Post-Memory, OldSiPolicyPpiDesc is %x, New is %x\n", OldSiPolicyPpiDesc, SiPolicyPpiDesc));
    Status = PeiServicesReInstallPpi (OldSiPolicyPpiDesc, SiPolicyPpiDesc);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Re-Install gSiPolicyPpiGuid fail in Post-Memory\n"));
    }
    ASSERT_EFI_ERROR (Status);
  }

  ASSERT_EFI_ERROR (Status);
  return Status;
}

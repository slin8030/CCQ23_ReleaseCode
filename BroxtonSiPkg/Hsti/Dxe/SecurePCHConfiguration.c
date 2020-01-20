/** @file
  This file contains the tests for the SecuePCHConfiguration bit

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
#include "HstiSiliconDxe.h"

/**
  Run tests for SecurePCHConfiguration bit
**/
VOID
CheckSecurePchConfiguration (
  VOID
  )
{
  EFI_STATUS      Status;
  BOOLEAN         Result;
  UINT32          PmcBase;
  UINT32          SmiEn;
  UINT8           GenPmCon1;
  UINT16          Tco1Cnt;
  CHAR16          *HstiErrorString;
  UINT16          AcpiBaseAddr;

  if ((mFeatureImplemented[1] & HSTI_BYTE1_SECURE_PCH_CONFIGURATION) == 0) {
    return;
  }

  Result = TRUE;
  //PmcBase = PciRead32(PCI_LIB_ADDRESS(0, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_PMC_BASE)) & B_PCH_LPC_PMC_BASE_BAR;
  PmcBase = PMC_BASE_ADDRESS;
  ///
  /// Read ACPI Base Address
  ///
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  DEBUG((DEBUG_INFO, "  Table 3-10. PCH Security Configuration\n"));

  DEBUG((DEBUG_INFO, "    1. SMI Enable\n"));

  SmiEn = IoRead32(AcpiBaseAddr + R_SMI_EN);
  if ((SmiEn & B_SMI_EN_GBL_SMI) == 0x0) {

    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_D, HSTI_PCH_SECURITY_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_D);
    Status = HstiLibAppendErrorString(
      PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
      NULL,
      HstiErrorString
      );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  //TCO SMI is not used in CHT, so need not check this but lock needs to be checked
  /*DEBUG ((DEBUG_INFO, "    2. TCO SMI Enable\n"));
  DEBUG ((DEBUG_INFO, "    SmiEn = %x\n", SmiEn));
  if ((SmiEn & B_PCH_SMI_EN_TCO) == 0x0) {

  HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_E ,HSTI_PCH_SECURITY_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_E);
  Status = HstiLibAppendErrorString(
  PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
  NULL,
  HstiErrorString
  );
  ASSERT_EFI_ERROR (Status);
  Result = FALSE;
  FreePool(HstiErrorString);
  }*/

  DEBUG((DEBUG_INFO, "    3. SMI Lock\n"));

  GenPmCon1 = MmioRead8(PmcBase + R_PMC_GEN_PMCON_2);

  if ((GenPmCon1 & B_PMC_GEN_PMCON_SMI_LOCK) == 0x0) {

    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_D, HSTI_PCH_SECURITY_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_D);
    Status = HstiLibAppendErrorString(
      PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
      NULL,
      HstiErrorString
      );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  DEBUG((DEBUG_INFO, "    4. TCO SMI Lock\n"));

  Tco1Cnt = IoRead16(AcpiBaseAddr + R_TCO1_CNT);
  DEBUG((DEBUG_INFO, "    AcpiBaseAddr + R_TCO1_CNT = %x\n", (AcpiBaseAddr + R_TCO1_CNT)));
  DEBUG((DEBUG_INFO, "    Tco1Cnt = %x\n", Tco1Cnt));

  if ((Tco1Cnt & B_TCO1_CNT_LOCK) == 0x0) {

    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_E, HSTI_PCH_SECURITY_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_E);
    Status = HstiLibAppendErrorString(
      PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
      NULL,
      HstiErrorString
      );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }


  //
  // ALL PASS
  //
  if (Result) {
    Status = HstiLibSetFeaturesVerified (
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               1,
               HSTI_BYTE1_SECURE_PCH_CONFIGURATION
               );
    ASSERT_EFI_ERROR (Status);
  }
  return;
}

/** @file
  This file contains the tests for the SecureCPUConfiguration bit

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
  Run tests for SecureCPUConfiguration bit
**/
VOID
CheckSecureCpuConfiguration (
  VOID
  )
{
  EFI_STATUS                      Status;
  BOOLEAN                         Result;
  UINTN                           CpuNumber;
  UINTN                           CpuIndex;
  UINT32                          RegEcx;
  UINT32                          TsegMB;
  UINT32                          Bgsm;
  UINTN                           SmrrBase;
  UINTN                           SmrrMask;
  UINT64                          MasterSmrrBase;
  UINT64                          MasterSmrrMask;
  CHAR16                          *HstiErrorString;

  if ((mFeatureImplemented[0] & HSTI_BYTE0_SECURE_CPU_CONFIGURATION) == 0) {
    return;
  }

  Result = TRUE;

  DEBUG ((DEBUG_INFO, "  Table 3-1. CPU Security Configuration\n"));

  CpuNumber = GetCpuNumber ();
  for (CpuIndex = 0; CpuIndex < CpuNumber; CpuIndex++) {
    DEBUG ((DEBUG_INFO, "  [CPU - 0x%x]\n", CpuIndex));

    DEBUG ((DEBUG_INFO, "    1. Microcode Update Revision\n"));
    if ((ProcessorReadMsr64 (CpuIndex, MSR_IA32_BIOS_SIGN_ID) & 0xFFFFFFFF00000000) == 0) {

      HstiErrorString = BuildHstiErrorString (HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_CODE_1 ,HSTI_CPU_SECURITY_CONFIGURATION, HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_STRING_1);
      Status = HstiLibAppendErrorString (
                 PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
                 NULL,
                 HstiErrorString
                 );
      ASSERT_EFI_ERROR (Status);
      Result = FALSE;
      FreePool (HstiErrorString);
    }

    DEBUG ((DEBUG_INFO, "    2. Sample Part \n"));

    if ((ProcessorReadMsr64 (CpuIndex, MSR_PLATFORM_INFO) & BIT27) != 0) {
      DEBUG ((DEBUG_INFO, "Fail: This is a sample part\n"));

      HstiErrorString = BuildHstiErrorString (HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_CODE_2 ,HSTI_CPU_SECURITY_CONFIGURATION, HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_STRING_2);
      Status = HstiLibAppendErrorString (
                 PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
                 NULL,
                 HstiErrorString
                 );
      ASSERT_EFI_ERROR (Status);
      Result = FALSE;
      FreePool (HstiErrorString);
    }

    DEBUG ((DEBUG_INFO, "    3. IA32_FEATURE_CONTROL MSR Lock\n"));

    if ((ProcessorReadMsr64 (CpuIndex, MSR_IA32_FEATURE_CONTROL) & B_MSR_IA32_FEATURE_CONTROL_LOCK) == 0) {
      DEBUG ((DEBUG_INFO, "Fail: Feature control MSR is not locked\n"));

      HstiErrorString = BuildHstiErrorString (HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_CODE_3 ,HSTI_CPU_SECURITY_CONFIGURATION, HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_STRING_3);
      Status = HstiLibAppendErrorString (
                 PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
                 NULL,
                 HstiErrorString
                 );
      ASSERT_EFI_ERROR (Status);
      Result = FALSE;
      FreePool (HstiErrorString);
    }

    DEBUG ((DEBUG_INFO, "    5. FEATURE_CONFIG MSR  Lock\n"));

    ProcessorCpuid (CpuIndex, CPUID_VERSION_INFO, NULL, NULL, &RegEcx, NULL);
    if ((RegEcx & B_CPUID_VERSION_INFO_ECX_AES) != 0) {
      if ((ProcessorReadMsr64 (CpuIndex, MSR_IA32_FEATURE_CONFIG) & B_IA32_FEATURE_CONFIG_LOCK) == 0) {
        DEBUG ((DEBUG_INFO, "Fail: Feature control msr not locked\n"));

        HstiErrorString = BuildHstiErrorString (HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_CODE_5 ,HSTI_CPU_SECURITY_CONFIGURATION, HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_STRING_5);
        Status = HstiLibAppendErrorString (
                   PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
                   NULL,
                   HstiErrorString
                   );
        ASSERT_EFI_ERROR (Status);
        Result = FALSE;
        FreePool (HstiErrorString);
      }
    }
  }

  DEBUG ((DEBUG_INFO, "  Table 7-4. SMM Security Configuration\n"));

  MasterSmrrBase = 0;
  MasterSmrrMask = 0;
  for (CpuIndex = 0; CpuIndex < CpuNumber; CpuIndex++) {
    DEBUG ((DEBUG_INFO, "  [CPU - 0x%x]\n", CpuIndex));

    DEBUG((DEBUG_INFO, "    1. SMM FEATURE CONTROL MSR IS ENABLED AND LOCKED\n"));

    if ((ProcessorReadMsr64(CpuIndex, MSR_SMM_FEATURE_CONTROL) & (B_SMM_CODE_CHK_EN | B_SMM_FEATURE_CONTROL_LOCK)) == 0) {
      HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_CODE_1, HSTI_CPU_SECURITY_CONFIGURATION, HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_STRING_1);
      Status = HstiLibAppendErrorString(
                 PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
                 NULL,
                 HstiErrorString
                 );
      ASSERT_EFI_ERROR(Status);
      Result = FALSE;
      FreePool(HstiErrorString);
    }

    DEBUG ((DEBUG_INFO, "    2. SMRR1 are supported\n"));

    if ((ProcessorReadMsr64 (CpuIndex, IA32_MTRR_CAP) & B_IA32_MTRR_CAP_SMRR_SUPPORT) == 0) {
      DEBUG ((DEBUG_INFO, "Fail: SMRR1 not supported\n"));

      HstiErrorString = BuildHstiErrorString (HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_CODE_A ,HSTI_CPU_SECURITY_CONFIGURATION, HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_STRING_A);
      Status = HstiLibAppendErrorString (
                 PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
                 NULL,
                 HstiErrorString
                 );
      ASSERT_EFI_ERROR (Status);
      Result = FALSE;
      FreePool (HstiErrorString);
    }

    if ((ProcessorReadMsr64 (CpuIndex, IA32_MTRR_CAP) & B_IA32_MTRR_CAP_SMRR_SUPPORT) != 0) {
      DEBUG ((DEBUG_INFO, "    3. SMRR1 programmed consistently on all cores\n"));

      SmrrBase = (UINTN)ProcessorReadMsr64 (CpuIndex, SMRR_PHYS_BASE);
      SmrrMask = (UINTN)ProcessorReadMsr64 (CpuIndex, SMRR_PHYS_MASK);

      if (CpuIndex == 0) {
        MasterSmrrBase = SmrrBase;
        MasterSmrrMask = SmrrMask;
      } else {
        if ((SmrrBase != MasterSmrrBase) || (SmrrMask != MasterSmrrMask)) {
          DEBUG ((DEBUG_INFO, "Fail: SMRR1 not programmed consistently across all cores\n"));

          HstiErrorString = BuildHstiErrorString (HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_CODE_A ,HSTI_CPU_SECURITY_CONFIGURATION, HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_STRING_A);
          Status = HstiLibAppendErrorString (
                     PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
                     NULL,
                     HstiErrorString
                     );
          ASSERT_EFI_ERROR (Status);
          Result = FALSE;
          FreePool (HstiErrorString);
        }
      }

      DEBUG ((DEBUG_INFO, "    4. SMRR1 enabled/correct\n"));

      if (((SmrrMask & BIT11) == 0x0) ||
          ((SmrrMask & BIT9) != 0x0) ||
          ((SmrrBase & 0x7) != 0x6)) {
        DEBUG ((DEBUG_INFO, "Fail: SMRR1 not enabled/correct\n"));

        HstiErrorString = BuildHstiErrorString (HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_CODE_A ,HSTI_CPU_SECURITY_CONFIGURATION, HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_STRING_A);
        Status = HstiLibAppendErrorString (
                   PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
                   NULL,
                   HstiErrorString
                   );
        ASSERT_EFI_ERROR (Status);
        Result = FALSE;
        FreePool (HstiErrorString);
      }

      DEBUG ((DEBUG_INFO, "    5. SMRR1 and TSEGMB match\n"));
      TsegMB = MmioRead32(MmPciBase(SA_MC_BUS, SA_MC_DEV, SA_MC_FUN) + TSEGMB_0_0_0_PCI_CUNIT_REG);

      DEBUG ((DEBUG_INFO, "SMRR:   0x%08x - 0x%08x\n", SmrrBase & (SmrrMask & 0xFFFFF000), (UINT32) (~(SmrrMask & 0xFFFFF000) + 1)));
      DEBUG((DEBUG_INFO, "TSEGMB: 0x%08x\n", TsegMB & B_SA_TOLUD_TOLUD_MASK));

      if ((SmrrBase & (SmrrMask & 0xFFFFF000)) != (TsegMB & B_SA_TOLUD_TOLUD_MASK)) {
        DEBUG ((DEBUG_INFO, "Fail: SMRR1 != TSEGMB\n"));

        HstiErrorString = BuildHstiErrorString (HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_CODE_A ,HSTI_CPU_SECURITY_CONFIGURATION, HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_STRING_A);
        Status = HstiLibAppendErrorString (
                   PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
                   NULL,
                   HstiErrorString
                   );
        ASSERT_EFI_ERROR (Status);
        Result = FALSE;
        FreePool (HstiErrorString);
      }

      DEBUG ((DEBUG_INFO, "    6. SMRR1 size\n"));
      Bgsm   = MmioRead32(MmPciBase(SA_MC_BUS, SA_MC_DEV, SA_MC_FUN) + R_SA_BGSM);
      if ((UINT32) (~(SmrrMask & 0xFFFFF000) + 1) !=
          ((Bgsm & B_SA_BGSM_BGSM_MASK) - (TsegMB & B_SA_TOLUD_TOLUD_MASK))) {
        DEBUG ((DEBUG_INFO, "Fail: SMRR1 size != BGSM-TSEGMB\n"));

        HstiErrorString = BuildHstiErrorString (HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_CODE_A ,HSTI_CPU_SECURITY_CONFIGURATION, HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_STRING_A);
        Status = HstiLibAppendErrorString (
                   PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
                   NULL,
                   HstiErrorString
                   );
        ASSERT_EFI_ERROR (Status);
        Result = FALSE;
        FreePool (HstiErrorString);
      }

      DEBUG ((DEBUG_INFO, "    7. SMRR1 work\n"));

      if (MmioRead32(SmrrBase & SmrrMask) != 0xFFFFFFFF) {
        DEBUG ((DEBUG_INFO, "Fail: SMRR1 not working, read succeeded\n"));


        HstiErrorString = BuildHstiErrorString (HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_CODE_A ,HSTI_CPU_SECURITY_CONFIGURATION, HSTI_BYTE0_SECURE_CPU_CONFIGURATION_ERROR_STRING_A);
        Status = HstiLibAppendErrorString (
                   PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
                   NULL,
                   HstiErrorString
                   );
        ASSERT_EFI_ERROR (Status);
        Result = FALSE;
        FreePool (HstiErrorString);
      }
    }
  }

  //
  // ALL PASS
  //
  if (Result) {
    Status = HstiLibSetFeaturesVerified (
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               0,
               HSTI_BYTE0_SECURE_CPU_CONFIGURATION
               );
    ASSERT_EFI_ERROR (Status);
  }
  return;
}

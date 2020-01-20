/** @file
  BIOS Guard EarlyPost initializations.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/RngLib.h>
#include <Private/Library/MpServiceLib.h>
#include <Private/Library/BiosGuardInit.h>
#include "CpuAccess.h"
#include <Library/CpuPolicyLib.h>

//
// Private GUID for BIOS Guard initializes
//
extern EFI_GUID gBiosGuardHobGuid;

/**
  Perform BIOS Guard initialization.

  @param[in] SiCpuPolicyPpi  - Policy PPI
**/
VOID
BiosGuardInit (
  IN SI_CPU_POLICY_PPI     *SiCpuPolicyPpi
  )
{
  EFI_STATUS               Status;
  UINT64                   MsrValue;
  BOOLEAN                  ResetRequired;
  BIOSGUARD_HOB            *BiosGuardHobPtr;
  UINT8                    i;
  BIOS_GUARD_CONFIG        *BiosGuardConfig;
  volatile UINT32          EphemeralAuthValue;
  UINT8                    EcData;
  CPU_CONFIG_PREMEM        *CpuConfigPreMem;
  CPU_CONFIG               *CpuConfig;
  PLATFORM_SEND_EC_COMMAND SendEcCmd;
  BOOLEAN                  RngSuccess;

  DEBUG ((DEBUG_INFO, "BiosGuardInit\n"));

  ResetRequired = FALSE;
  RngSuccess = FALSE;
  MsrValue = AsmReadMsr64 (MSR_PLATFORM_INFO);
  if (!(MsrValue & B_MSR_PLATFORM_INFO_BIOSGUARD_AVAIL)) {
    DEBUG ((DEBUG_INFO, "BIOS Guard Feature is not supported\n"));
    return;
  }

  Status = GetConfigBlock ((CONFIG_BLOCK_TABLE_HEADER *)SiCpuPolicyPpi, &gBiosGuardConfigGuid , (VOID *)&BiosGuardConfig);

  ///
  /// Check if BiosGuardConfig is a NULL pointer.
  ///
  if (BiosGuardConfig == NULL) {
    DEBUG ((DEBUG_ERROR, "Policy pointer of BIOS Guard is NULL.\n"));
    return;
  }
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((CONFIG_BLOCK_TABLE_HEADER *)SiCpuPolicyPpi, &gCpuConfigPreMemGuid , (VOID *)&CpuConfigPreMem);

  Status = GetConfigBlock ((CONFIG_BLOCK_TABLE_HEADER *)SiCpuPolicyPpi, &gCpuConfigGuid , (VOID *)&CpuConfig);

  ///
  /// Check if CpuConfig is a NULL pointer.
  ///
  if (CpuConfigPreMem == NULL) {
    DEBUG ((DEBUG_ERROR, "Policy pointer of CPU Config is NULL.\n"));
    return;
  }
  ASSERT_EFI_ERROR (Status);

  if (CpuConfigPreMem->BiosGuard == TRUE) {
    DEBUG ((DEBUG_INFO, "BIOS Guard Module is Enabled\n"));
    SendEcCmd = (PLATFORM_SEND_EC_COMMAND) (UINTN) CpuConfig->SendEcCmd;

    ///
    /// Read BIOS Guard Control Register
    ///
    MsrValue = AsmReadMsr64 (MSR_PLAT_FRMW_PROT_CTRL);
    if (MsrValue & B_MSR_PLAT_FRMW_PROT_CTRL_LK) {
      if (!(MsrValue & B_MSR_PLAT_FRMW_PROT_CTRL_EN)) {
        ///
        /// Reset required as the BIOS Guard CTRL MSR is locked and needs to be toggled
        ///
        ResetRequired = TRUE;
      }
    } else {
      if ((BiosGuardConfig->Bgpdt.PlatAttr & EC_PRESENT) && (BiosGuardConfig->Bgpdt.PlatAttr & EC_BIOSGUARD_PROTECTED)) {
        DEBUG ((DEBUG_INFO, "EC is Present and EC FW supports BIOS Guard\n"));
        ///
        /// Check EC command functions if NULL pointer.
        ///
        if (CpuConfig->SendEcCmd == NULL) {
          DEBUG ((DEBUG_ERROR, "EC is not present or EC command function is not implemented.\n"));
          return;
        }
        Status = CpuConfig->SendEcCmd(SendEcCmds, BiosGuardConfig->EcCmdProvisionEav, 0 , NULL);
        if (Status == EFI_SUCCESS) {
          RngSuccess = GetRandomNumber32 ((UINT32 *)&EphemeralAuthValue);
          if (!RngSuccess) {
            DEBUG ((DEBUG_ERROR, "Fail to get a 32-bits random number!!!\n"));
          } else {
            DEBUG ((DEBUG_INFO, "BIOSGUARD:EphemeralAuthValue = %x\n", EphemeralAuthValue));
          }
          for (i = 0; (i < (sizeof (EphemeralAuthValue))); i++) {
            Status = CpuConfig->SendEcCmd(SendEcValue, 0, (UINT8)((EphemeralAuthValue >> (i * 8)) & 0xFF), NULL);
          }
          Status = CpuConfig->SendEcCmd(ReceiveEcValue, 0, 0, &EcData);
          if (EcData != 0) {
            ResetRequired = TRUE;
          }
          AsmWriteMsr32 (MSR_PLAT_FRMW_PROT_PASSWD, EphemeralAuthValue);
          EphemeralAuthValue = 0;
          Status = CpuConfig->SendEcCmd(SendEcCmds, BiosGuardConfig->EcCmdLock, 0, NULL);
          Status = CpuConfig->SendEcCmd(ReceiveEcValue, 0, 0, &EcData);
          if (EcData != 0) {
            ResetRequired = TRUE;
          }
        } else {
          DEBUG ((DEBUG_ERROR, "EC is not detected or BIOS Guard doesn't protect EC.\n"));
        }
      }
      AsmWriteMsr64 (MSR_PLAT_FRMW_PROT_HASH_0, BiosGuardConfig->BgpdtHash[0]);
      AsmWriteMsr64 (MSR_PLAT_FRMW_PROT_HASH_1, BiosGuardConfig->BgpdtHash[1]);
      AsmWriteMsr64 (MSR_PLAT_FRMW_PROT_HASH_2, BiosGuardConfig->BgpdtHash[2]);
      AsmWriteMsr64 (MSR_PLAT_FRMW_PROT_HASH_3, BiosGuardConfig->BgpdtHash[3]);
      MsrValue |= (UINT64) (B_MSR_PLAT_FRMW_PROT_CTRL_LK | B_MSR_PLAT_FRMW_PROT_CTRL_EN);
      AsmWriteMsr64 (MSR_PLAT_FRMW_PROT_CTRL, MsrValue);
    }
    ///
    /// Create BIOS Guard HOB
    ///
    if (!ResetRequired) {
      Status = PeiServicesCreateHob (EFI_HOB_TYPE_GUID_EXTENSION, sizeof (BIOSGUARD_HOB), (VOID **) &BiosGuardHobPtr);
      BiosGuardHobPtr->EfiHobGuidType.Name = gBiosGuardHobGuid;
      CopyMem (&BiosGuardHobPtr->Bgpdt, &BiosGuardConfig->Bgpdt, BiosGuardConfig->Bgpdt.BgpdtSize);
      CopyMem (&BiosGuardHobPtr->BgupHeader, &BiosGuardConfig->BgupHeader, sizeof (BGUP_HEADER));
      CopyMem (&BiosGuardHobPtr->BiosGuardLog, &BiosGuardConfig->BiosGuardLog, sizeof (BIOSGUARD_LOG));
      BiosGuardHobPtr->TotalFlashSize = CpuConfigPreMem->TotalFlashSize;
      BiosGuardHobPtr->BiosSize = CpuConfigPreMem->BiosSize;
      BiosGuardHobPtr->BiosGuardModulePtr = BiosGuardConfig->BiosGuardModulePtr;
    }
  } else {
    DEBUG ((DEBUG_INFO, "BIOS Guard Module is Disabled\n"));
    MsrValue = AsmReadMsr64 (MSR_PLAT_FRMW_PROT_CTRL);
    if (MsrValue & B_MSR_PLAT_FRMW_PROT_CTRL_LK) {
      if (MsrValue & B_MSR_PLAT_FRMW_PROT_CTRL_EN) {
        ///
        /// Reset required as the BIOS Guard CTRL MSR is locked and needs to be toggled
        ///
        ResetRequired = TRUE;
      }
    } else {
      MsrValue &= (UINT64) ~B_MSR_PLAT_FRMW_PROT_CTRL_EN;
      MsrValue |= B_MSR_PLAT_FRMW_PROT_CTRL_LK;
      AsmWriteMsr64 (MSR_PLAT_FRMW_PROT_CTRL, MsrValue);
    }
  }

  if (ResetRequired) {
    ///
    /// Perform Cold Reset
    ///
    DEBUG ((DEBUG_INFO, "Reset Required. Performing Cold Reset to unlock BIOS Guard CONTROL MSR\n"));
    PerformWarmOrColdReset (COLD_RESET);
  }

  return;
}

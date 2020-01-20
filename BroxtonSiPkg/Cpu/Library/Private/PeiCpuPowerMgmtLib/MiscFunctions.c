/** @file
  This file contains Processor Power Management ACPI related functions for
  processors.

  <b>Acronyms:</b>
     - PPM: Processor Power Management
     - TM:  Thermal Monitor
     - IST: Intel(R) Speedstep technology
     - HT:  Hyper-Threading Technology

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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

#include "PowerMgmtCommon.h"
#include <Library/SteppingLib.h>

/**
  This will perform Miscellaneous Power Management related programming.
**/
VOID
InitMiscFeatures (
  )
{
  PostCode (0xC6D);
  ///
  /// Configure Package Turbo Power Limits
  ///
  ConfigureTdpPowerLimits ();

  ///
  /// Configure PL3 and PL4 limits
  ///
  ConfigurePl3AndPl4PowerLimits();

  ///
  /// Configure VR Current Config of MSR 601
  ///
  ConfigureVrCurrentConfig();

  ///
  /// Configure DDR RAPL PowerLimits
  ///
  ConfigureDdrPowerLimits();

  ///
  /// Configure Platform Power Limits
  ///
  ConfigurePlatformPowerLimits();
}


/**
  Locks down all settings.

  @param[in] PowerMgmtConfig  Pointer to PPM Policy structure.
**/
VOID
PpmLockDown (
  VOID
  )
{
  MSR_REGISTER TempMsr;

  ///
  /// Program PMG_CST_CONFIG MSR [15] (CFG lock bit)
  ///
  RunOnAllLogicalProcessors (ApSafeLockDown, gPowerMgmtConfig);
  ///
  /// Lock Package power limit MSR
  ///
  TempMsr.Qword = AsmReadMsr64 (MSR_PACKAGE_POWER_LIMIT);
  TempMsr.Dwords.High &= ~(B_POWER_LIMIT_LOCK);
  if (gPowerMgmtConfig->TurboPowerLimitLock) {
    TempMsr.Dwords.High |= B_POWER_LIMIT_LOCK;
  }
  AsmWriteMsr64 (MSR_PACKAGE_POWER_LIMIT, TempMsr.Qword);
  ///
  /// Program the PROCHOT_Lock
  ///
  TempMsr.Qword = AsmReadMsr64 (MSR_POWER_CTL);
  TempMsr.Dwords.Low &= ~(B_MSR_POWER_CTL_PROCHOT_LOCK);
  if (gPowerMgmtConfig->ProcHotLock) {
    TempMsr.Dwords.Low |= B_MSR_POWER_CTL_PROCHOT_LOCK;
  }
  AsmWriteMsr64 (MSR_POWER_CTL, TempMsr.Qword);
  ///
  /// Program Ddr RAPL LIMIT Lock
  ///
  TempMsr.Qword = AsmReadMsr64 (MSR_DDR_RAPL_LIMIT);
  TempMsr.Dwords.High &= ~(B_POWER_LIMIT_LOCK);
  if (gPowerMgmtConfig->TurboPowerLimitLock) {
    TempMsr.Dwords.High |= B_POWER_LIMIT_LOCK;
  }
  AsmWriteMsr64 (MSR_DDR_RAPL_LIMIT, TempMsr.Qword);
  PostCode (0xC70);
  return;
}

/**
  Lock MSR_PMG_CST_CONFIG.
  This function must be MP safe.

  @param[in] Buffer    Not used (needed for API compatibility)

  @retval EFI_SUCCESS  Processor C-State locked successfully.
**/
VOID
EFIAPI
ApSafeLockDown (
  IN OUT VOID *Buffer
  )
{
  MSR_REGISTER      PmCfgCtrl;
  POWER_MGMT_CONFIG *PowerMgmtConfig;
  UINT8             CfgLock;

  PowerMgmtConfig = (POWER_MGMT_CONFIG *) Buffer;
  if (PowerMgmtConfig == NULL) {
    CfgLock = PPM_ENABLE;
  } else {
    CfgLock = (UINT8) gPowerMgmtConfig->PmgCstCfgCtrlLock;
  }
  PmCfgCtrl.Qword = AsmReadMsr64 (MSR_PMG_CST_CONFIG);
  PmCfgCtrl.Dwords.Low &= ~B_CST_CONTROL_LOCK;

    if (CfgLock == PPM_ENABLE) {
      PmCfgCtrl.Dwords.Low |= B_CST_CONTROL_LOCK;
    }

  AsmWriteMsr64 (MSR_PMG_CST_CONFIG, PmCfgCtrl.Qword);

  return;
}

/**
  Runs the specified procedure on all logical processors, passing in the
  parameter buffer to the procedure.

  @param[in] Procedure  The function to be run.
  @param[in] Buffer     Pointer to a parameter buffer.

  @retval EFI_SUCCESS
**/
EFI_STATUS
RunOnAllLogicalProcessors (
  IN OUT EFI_AP_PROCEDURE Procedure,
  IN OUT VOID             *Buffer
  )
{
  EFI_STATUS Status;

  ///
  /// Run the procedure on all logical processors.
  ///
  (*Procedure)(Buffer);
  Status = StartupAllAps ((EFI_AP_PROCEDURE) Procedure, Buffer);

  return Status;
}

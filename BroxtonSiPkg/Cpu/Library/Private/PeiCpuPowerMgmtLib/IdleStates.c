/** @file
  This file contains power management C State configuration functions for
  processors.

  Acronyms:
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

/**
  Initializes C States Power management features
**/
VOID
InitCState (
  VOID
  )
{
  UINT16     mAcpiBaseAddr;

  mAcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  PostCode (0xC6B);
  ///
  /// Initialize C states, some are general, some are processor specific.
  /// Dynamic loading of CST SSDT tables occurs at InitPpmPost.
  ///
  EnableCStates (mAcpiBaseAddr + PM_CST_LVL2);
}

/**
  Enables C-State support as specified by the input flags on all logical
  processors and sets associated timing requirements in the chipset.

  @param[in] C3IoAddress  IO address to generate C3 states (PM base + 014 usually)
**/
VOID
EnableCStates (
  IN UINT16 C3IoAddress
  )
{
  MSR_REGISTER PowerCtl;
  UINT16       EnableCStateParameters;

  ///
  /// Load the C-State parameters to pass to the core function.
  ///
  EnableCStateParameters = C3IoAddress;
  ///
  /// Enable C-States on all logical processors.
  ///
  RunOnAllLogicalProcessors (ApSafeEnableCStates, &EnableCStateParameters);
  ///
  /// If C-states are disabled or not supported, Disable C1e  and return
  ///
  if ((gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_C_STATES) == 0) {
    PowerCtl.Qword = AsmReadMsr64 (MSR_POWER_CTL);
    PowerCtl.Dwords.Low &= ~B_MSR_POWER_CTL_C1E;
    AsmWriteMsr64 (MSR_POWER_CTL, PowerCtl.Qword);
    DEBUG (
            (DEBUG_INFO,
             "Setup C state disabled.Disable C1e. MSR(1FC) : %X %X\n",
             PowerCtl.Dwords.High,
             PowerCtl.Dwords.Low)
            );
    return;
  }
  ///
  /// Configure supported enhanced C-states
  ///
  /// Read Power Ctl MSR
  ///
  PowerCtl.Qword = AsmReadMsr64 (MSR_POWER_CTL);
  DEBUG ((DEBUG_INFO, "MSR(1FC) before configuring C1E: %X %X\n", PowerCtl.Dwords.High, PowerCtl.Dwords.Low));
  ///
  /// Enable supported states
  ///
  if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_C1E) {
    PowerCtl.Dwords.Low |= B_MSR_POWER_CTL_C1E;
  } else {
    PowerCtl.Dwords.Low &= ~B_MSR_POWER_CTL_C1E;
  }
  ///
  /// Update Power Control MSR
  ///
  AsmWriteMsr64 (MSR_POWER_CTL, PowerCtl.Qword);
  DEBUG ((DEBUG_INFO, "MSR(1FC) after configuring C1E: %X %X\n", PowerCtl.Dwords.High, PowerCtl.Dwords.Low));
  return;
}

/**
  Enable C-State support as specified by the input flags on a logical processor.
    Configure BIOS C1 Coordination (SMI coordination)
    Enable IO redirection coordination
    Choose proper coordination method
    Configure extended C-States

  This function must be MP safe.

  @param[in] Buffer    Pointer to a ENABLE_CSTATE_PARAMS containing the necessary
                       information to enable C-States

  @retval EFI_SUCCESS  Processor C-State support configured successfully.
**/
VOID
EFIAPI
ApSafeEnableCStates (
  IN OUT VOID *Buffer
  )
{
  MSR_REGISTER PmCfgCtrl;
  MSR_REGISTER IoCaptAddr;

  ///
  /// If C-states are disabled in setup, disable C-states
  ///
  if (!(gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_C_STATES)) {
    PmCfgCtrl.Qword = AsmReadMsr64 (MSR_PMG_CST_CONFIG);
    PmCfgCtrl.Dwords.Low &= ~B_PACKAGE_C_STATE_LIMIT;
    PmCfgCtrl.Dwords.Low |= V_PKG_CSTATE_LIMIT_C0;
    PmCfgCtrl.Dwords.Low &= ~V_CSTATE_LIMIT_MASK;
    PmCfgCtrl.Dwords.Low |= V_CSTATE_LIMIT_UNLIMIT;
    AsmWriteMsr64 (MSR_PMG_CST_CONFIG, PmCfgCtrl.Qword);
    return;
  }
  ///
  /// Set C-state package limit to the highest C-state enabled
  ///

  PmCfgCtrl.Qword = AsmReadMsr64 (MSR_PMG_CST_CONFIG);
  if (gPowerMgmtConfig->PkgCStateLimit != PkgCpuDefault) {
    PmCfgCtrl.Dwords.Low &= ~B_PACKAGE_C_STATE_LIMIT;
    if (gPowerMgmtConfig->PkgCStateLimit ==2) {
      PmCfgCtrl.Dwords.Low |= V_PKG_CSTATE_LIMIT_C2;
    } else if (gPowerMgmtConfig->PkgCStateLimit ==1) {
      PmCfgCtrl.Dwords.Low |= V_PKG_CSTATE_LIMIT_C1;
    } else if (gPowerMgmtConfig->PkgCStateLimit ==0) {
      PmCfgCtrl.Dwords.Low |= V_PKG_CSTATE_LIMIT_C0;
    }

    if (!(gPowerMgmtConfig->EnableCCx)) {
      PmCfgCtrl.Dwords.Low &= ~V_CSTATE_LIMIT_MASK;
      if (gPowerMgmtConfig->EnableC10) {
        PmCfgCtrl.Dwords.Low |= V_CSTATE_LIMIT_C10;
      } else if (gPowerMgmtConfig->EnableC9) {
        PmCfgCtrl.Dwords.Low |= V_CSTATE_LIMIT_C9;
      } else if (gPowerMgmtConfig->EnableC8) {
        PmCfgCtrl.Dwords.Low |= V_CSTATE_LIMIT_C8;
      } else if (gPowerMgmtConfig->EnableC7) {
        PmCfgCtrl.Dwords.Low |= V_CSTATE_LIMIT_C7;
      } else if (gPowerMgmtConfig->EnableC6) {
        PmCfgCtrl.Dwords.Low |= V_CSTATE_LIMIT_C6;
      } else if (gPowerMgmtConfig->EnableC3) {
        PmCfgCtrl.Dwords.Low |= V_CSTATE_LIMIT_C3;
      } else if (gPowerMgmtConfig->EnableC1) {
        PmCfgCtrl.Dwords.Low |= V_CSTATE_LIMIT_C1;
      } else if (gPowerMgmtConfig->UnlimitedCstate) {
        PmCfgCtrl.Dwords.Low |= V_CSTATE_LIMIT_UNLIMIT;
      }
    }
  }
  ///
  /// Configure C State IO redirection
  ///
  if (gPowerMgmtConfig->PmgCstCfgCtrIoMwaitRedirection) {
    PmCfgCtrl.Dwords.Low |= B_IO_MWAIT_REDIRECTION_ENABLE;
  }

  //
  // Enable TimedMwait
  //
  if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_TIMED_MWAIT) {
    PmCfgCtrl.Dwords.Low &= (~B_TIMED_MWAIT_ENABLE);
    PmCfgCtrl.Dwords.Low |= B_TIMED_MWAIT_ENABLE;
  }

  ///
  /// Configure C-state auto-demotion
  ///
  PmCfgCtrl.Dwords.Low &= ~(B_C1_AUTO_DEMOTION_ENABLE | B_C3_AUTO_DEMOTION_ENABLE);
  if (gPowerMgmtConfig->C3AutoDemotion) {
    ///
    /// Enable C6/C7 Auto-demotion to C3
    ///
    PmCfgCtrl.Dwords.Low |= B_C3_AUTO_DEMOTION_ENABLE;
  }
  if (gPowerMgmtConfig->C1AutoDemotion) {
    ///
    /// Enable C3/C6/C7 Auto-demotion to C1
    ///
    PmCfgCtrl.Dwords.Low |= B_C1_AUTO_DEMOTION_ENABLE;
  }
  ///
  /// Configure C-state un-demotion
  ///
  PmCfgCtrl.Dwords.Low &= ~(B_C1_AUTO_UNDEMOTION_ENABLE | B_C3_AUTO_UNDEMOTION_ENABLE);
  if (gPowerMgmtConfig->C3UnDemotion) {
    ///
    /// Enable un-demotion from demoted C3
    ///
    PmCfgCtrl.Dwords.Low |= B_C3_AUTO_UNDEMOTION_ENABLE;
  }
  if (gPowerMgmtConfig->C1UnDemotion) {
    ///
    /// Enable un-demotion from demoted C1
    ///
    PmCfgCtrl.Dwords.Low |= B_C1_AUTO_UNDEMOTION_ENABLE;
  }

  ///
  /// Configure Package C-state Demotion / un-demotion
  ///
  PmCfgCtrl.Dwords.Low &= ~(B_PKG_CSTATE_DEMOTION_ENABLE | B_PKG_CSTATE_UNDEMOTION_ENABLE);
  if (gPowerMgmtConfig->PkgCStateDemotion) {
    ///
    /// Enable Package C-state Demotion
    ///
    PmCfgCtrl.Dwords.Low |= B_PKG_CSTATE_DEMOTION_ENABLE;
  }
  if (gPowerMgmtConfig->PkgCStateUnDemotion) {
    ///
    /// Enable Package C-state un-demotion
    ///
    PmCfgCtrl.Dwords.Low |= B_PKG_CSTATE_UNDEMOTION_ENABLE;
  }
  AsmWriteMsr64 (MSR_PMG_CST_CONFIG, PmCfgCtrl.Qword);

  if (gPowerMgmtConfig->PmgCstCfgCtrIoMwaitRedirection) {
    IoCaptAddr.Qword = AsmReadMsr64 (MSR_PMG_IO_CAPTURE_BASE);

    IoCaptAddr.Dwords.Low &= ~(V_IO_CAPT_LVL2_BASE_ADDR_MASK);

    /// C3 IO Address is 0x414
    IoCaptAddr.Dwords.Low = V_IO_CAPT_LVL2_BASE_ADDR_C3;

    ///
    /// Mask off CST range and set the CST range
    ///
    IoCaptAddr.Dwords.Low &= ~B_MSR_PMG_CST_RANGE;
    if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_C10) {
      IoCaptAddr.Dwords.Low |= V_IO_CAPT_LVL7;
    } else if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_C9) {
      IoCaptAddr.Dwords.Low |= V_IO_CAPT_LVL6;
    } else if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_C8) {
      IoCaptAddr.Dwords.Low |= V_IO_CAPT_LVL5;
    } else if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_C7) {
      IoCaptAddr.Dwords.Low |= V_IO_CAPT_LVL4;
    } else if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_C6) {
      IoCaptAddr.Dwords.Low |= V_IO_CAPT_LVL3;
    } else if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_C3) {
      IoCaptAddr.Dwords.Low |= V_IO_CAPT_LVL2;
    }

    AsmWriteMsr64 (MSR_PMG_IO_CAPTURE_BASE, IoCaptAddr.Qword);
  }

  gCpuGlobalNvsAreaConfig->Area->MonitorMwaitEnable = (UINT8) mCpuConfig->MonitorMwaitEnable;

  return;
}

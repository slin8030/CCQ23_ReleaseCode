/** @file
  This file contains P States and Turbo Power Management configuration functions for
  processors.

  <b>Acronyms:</b>
     - PPM:  Processor Power Management
     - TM:   Thermal Monitor
     - IST:  Intel(R) Speedstep technology
     - HT:   Hyper-Threading Technology

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
#include <CpuDataStruct.h>
#include <Library/PreSiliconLib.h>

extern UINT16 mCpuConfigTdpBootRatio;

/**
  Initializes P States and Turbo Power management features
**/
VOID
InitPStates (
  VOID
  )
{
  MSR_REGISTER          Ia32MiscEnableMsr;
  PostCode (0xC6A);
  ///
  /// InitTurboRatioLimits has to be called before InitEist as InitEist uses the Turbo Ratio Limit programmed.
  ///
  InitTurboRatioLimits ();
  ///
  /// Initialize P states
  ///
  if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_EIST) {
    if (mMaxBusRatio != mMinBusRatio) {
      InitEist (mFvidPointer);
      mNumberOfStates = mFvidPointer[0].FvidHeader.EistStates;
    } else {
      ///
      /// Initialize the FVID tables. FVID table created anyway. Required for ACPI table update on DXE stage.
      ///
      InitFvidTable (mFvidPointer, FVID_MAX_STATES, FVID_MIN_STEP_SIZE, FALSE);
      ASSERT (mFvidPointer->FvidHeader.EistStates != 0);
      DEBUG ((DEBUG_WARN, "CPU MaxBusRatio is equal to MinBusRatio, InitEist() is skipped!\n"));
    }
  } else {
    ///
    /// Clear EIST bit in IA32 Misc Enable MSR that was intially set in PEI
    ///
    Ia32MiscEnableMsr.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
    Ia32MiscEnableMsr.Qword &= ~B_MSR_IA32_MISC_ENABLE_EIST;

    ///
    /// Disable Turbo if EIST is disabled
    ///
    if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_TURBO) {
      Ia32MiscEnableMsr.Qword |= B_MSR_IA32_MISC_DISABLE_TURBO;
    }
    AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnableMsr.Qword);
  }
}

/**
  Initializes Turbo Ratio limits in the processor.
**/
VOID
InitTurboRatioLimits (
  VOID
  )
{
  MSR_REGISTER Ia32MiscEnableMsr;
  MSR_REGISTER TurboRatioLimit;
  MSR_REGISTER CoreThreadCount;
  UINT8        CoreCount;
  UINT8        OneCoreRatioLimit;
  EFI_CPUID_REGISTER Cpuid = { 0, 0, 0, 0 };

  ///
  /// Test for Turbo Mode supported and initialize if true.
  ///
  AsmCpuid (CPUID_POWER_MANAGEMENT_PARAMS, &Cpuid.RegEax, &Cpuid.RegEbx, &Cpuid.RegEcx, &Cpuid.RegEdx);
  Ia32MiscEnableMsr.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_TURBO) {
    ///
    /// Clear Turbo Mode disable bit in IA32 Misc Enable MSR
    ///
    Ia32MiscEnableMsr.Qword &= ~B_MSR_IA32_MISC_DISABLE_TURBO;
    AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnableMsr.Qword);
  } else if (((Ia32MiscEnableMsr.Qword & B_MSR_IA32_MISC_DISABLE_TURBO) == 0) &&
             ((Cpuid.RegEax & B_CPUID_POWER_MANAGEMENT_EAX_TURBO) == B_CPUID_POWER_MANAGEMENT_EAX_TURBO)) {
    ///
    /// If Turbo mode is supported but required to be disabled (by Policy setting)
    /// Set Turbo Mode disable bit in IA32 Misc Enable MSR since it might be temporarily enabled earlier.
    ///
    Ia32MiscEnableMsr.Qword |= B_MSR_IA32_MISC_DISABLE_TURBO;
    AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnableMsr.Qword);
  }

  ///
  /// Check if processor turbo-ratio can be overriden
  ///
  if ((!mRatioLimitProgrammble) || !(gPowerMgmtConfig->RatioLimitProgramable)){
    DEBUG ((DEBUG_WARN, "Turbo Ratio Limit is NOT programmable. Platform Info MSR (0xCE) [28] is not set \n"));
    return;
  }
  TurboRatioLimit.Qword = AsmReadMsr64 (MSR_TURBO_RATIO_LIMIT);
  OneCoreRatioLimit     = (UINT8) (TurboRatioLimit.Dwords.Low & B_MSR_TURBO_RATIO_LIMIT_1C);
  ///
  /// Max Turbo ratio or P0 = Fused 1C Turbo Ratio Limit
  ///
  mTurboBusRatio = OneCoreRatioLimit;
  ///
  /// Initialize turbo ratio limit MSR.
  /// Find the number of active cores and initialize the ratio limits only if they are available.
  ///
  CoreThreadCount.Qword = AsmReadMsr64 (MSR_CORE_THREAD_COUNT);
  if (PLATFORM_ID != VALUE_REAL_PLATFORM) { // pre-silicon start
    if (CoreThreadCount.Qword == 0) {  //msr 35h is 0 force set to one core one thread;
      CoreThreadCount.Qword = 0x10001;
    }
    else {
      CoreThreadCount.Qword |=  LShiftU64(CoreThreadCount.Qword, N_CORE_COUNT_OFFSET);
    }
  }
  CoreCount             = (UINT8) RShiftU64 (CoreThreadCount.Dwords.Low, N_CORE_COUNT_OFFSET);
  if (gPowerMgmtConfig->RatioLimit[0] >= gPowerMgmtConfig->RatioLimit[1] &&
      gPowerMgmtConfig->RatioLimit[0] >= gPowerMgmtConfig->RatioLimit[2] &&
      gPowerMgmtConfig->RatioLimit[0] >= gPowerMgmtConfig->RatioLimit[3] &&
      gPowerMgmtConfig->RatioLimit[1] >= mMaxBusRatio &&
      gPowerMgmtConfig->RatioLimit[2] >= mMaxBusRatio &&
      gPowerMgmtConfig->RatioLimit[3] >= mMaxBusRatio
      ) {
    if (CoreCount >= 1) {
      TurboRatioLimit.Dwords.Low &= ~B_MSR_TURBO_RATIO_LIMIT_1C;
      TurboRatioLimit.Dwords.Low |= (UINT32) gPowerMgmtConfig->RatioLimit[0];
    }
    if (CoreCount >= 2) {
      TurboRatioLimit.Dwords.Low &= ~B_MSR_TURBO_RATIO_LIMIT_2C;
      TurboRatioLimit.Dwords.Low |= LShiftU64 (gPowerMgmtConfig->RatioLimit[1], 8);
    }
    if (CoreCount >= 3) {
      TurboRatioLimit.Dwords.Low &= ~B_MSR_TURBO_RATIO_LIMIT_3C;
      TurboRatioLimit.Dwords.Low |= LShiftU64 (gPowerMgmtConfig->RatioLimit[2], 16);
    }
    if (CoreCount >= 4) {
      TurboRatioLimit.Dwords.Low &= ~B_MSR_TURBO_RATIO_LIMIT_4C;
      TurboRatioLimit.Dwords.Low |= LShiftU64 (gPowerMgmtConfig->RatioLimit[3], 24);
    }
    AsmWriteMsr64 (MSR_TURBO_RATIO_LIMIT, TurboRatioLimit.Qword);
  }
  return;
}

/**
  Initializes required structures for P-State table creation and enables EIST
  support in the processor.

  @param[in out] FvidPointer  Table to update, must be initialized.
**/
VOID
InitEist (
  IN OUT FVID_TABLE        *FvidPointer
  )
{
  ///
  /// Initialize the FVID tables.
  ///
  InitFvidTable (FvidPointer, FVID_MAX_STATES, FVID_MIN_STEP_SIZE, FALSE);
  ASSERT (FvidPointer->FvidHeader.EistStates != 0);

  ///
  /// Enable EIST on all logical processors.
  ///
  RunOnAllLogicalProcessors (ApSafeEnableEist, NULL);

  return;
}

/**
  Enables EIST support in a logical processor.

  This function must be MP safe.

  @param[in out] Buffer  Pointer to arguments - not used
**/
VOID
EFIAPI
ApSafeEnableEist (
  IN OUT VOID *Buffer
  )
{
  MSR_REGISTER Ia32MiscEnable;
  MSR_REGISTER MiscPwrMgmt;

  ///
  /// Enable EIST in the CPU MSR.
  ///
  Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  Ia32MiscEnable.Qword |= B_MSR_IA32_MISC_ENABLE_EIST;
  AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);

  ///
  /// If CMP is disabled, disable hardware coordination.
  ///
  if (!(gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_CMP)) {
    MiscPwrMgmt.Qword = AsmReadMsr64 (MSR_MISC_PWR_MGMT);
    MiscPwrMgmt.Qword |= B_MISC_PWR_MGMT_SINGLE_PCTL_EN;
    AsmWriteMsr64 (MSR_MISC_PWR_MGMT, MiscPwrMgmt.Qword);
  }

  return;
}

/**
  This function updates the table provided with the FVID data for the processor.
  If CreateDefaultTable is TRUE, a minimam FVID table will be provided.
  The maximum number of states must be greater then or equal to two.
  The table should be initialized in such a way as for the caller to determine if the
  table was updated successfully.  This function should be deprecated in the future when
  Release 8 is integrated in favor of the EIST protocol calculating FVID information.

  @param[in out] FvidPointer         Pointer to a table to be updated
  @param[in] MaxNumberOfStates   Number of entries in the table pointed to by FvidPointer
  @param[in] MinStepSize         Minimum step size for generating the FVID table
  @param[in] CreateDefaultTable  Create default FVID table rather then full state support
**/
VOID
InitFvidTable (
  IN OUT FVID_TABLE *FvidPointer,
  IN UINT16         MaxNumberOfStates,
  IN UINT16         MinStepSize,
  IN BOOLEAN        CreateDefaultTable
  )
{
  EFI_STATUS Status;

  ///
  /// Return the function, if the FVID tables have already been created.
  ///
  if (FvidPointer[0].FvidHeader.EistStates != 0) {
    return;
  }
  ///
  /// Create FVID table
  ///
  if (CreateDefaultTable) {
    CreateDefaultFvidTable (FvidPointer);
    gCpuGlobalNvsAreaConfig->Area->PpmFlags &= ~PPM_TURBO;
  } else {
    Status = CreateFvidTable (FvidPointer, MaxNumberOfStates);
    if (EFI_ERROR (Status)) {
      CreateDefaultFvidTable (FvidPointer);
      gCpuGlobalNvsAreaConfig->Area->PpmFlags &= ~PPM_TURBO;
    }
  }
  return;
}

/**
  Create default FVID table with max and min states only.

  @param[in out] FvidPointer  Pointer to a table to be updated
**/
VOID
CreateDefaultFvidTable (
  IN OUT FVID_TABLE *FvidPointer
  )
{
  UINT64 wPower1;
  UINT64 wPower2;

  ///
  /// Fill in the FVid table header.
  ///
  FvidPointer[0].FvidHeader.Stepping    = gCpuGlobalNvsAreaConfig->Area->Cpuid;
  FvidPointer[0].FvidHeader.MaxBusRatio = mMaxBusRatio;
  FvidPointer[0].FvidHeader.EistStates  = 2;
  ///
  /// First entry is state 0, highest state.
  ///
  FvidPointer[1].FvidState.State    = 0;
  FvidPointer[1].FvidState.BusRatio = mMaxBusRatio;
  ///
  /// Power is calculated in milliwatts
  ///
  FvidPointer[1].FvidState.Power = (mPackageTdpWatt * 1000);
  ///
  /// Second entry is state 1, lowest state.
  ///
  FvidPointer[2].FvidState.State    = 1;
  FvidPointer[2].FvidState.BusRatio = (UINT16) mMinBusRatio;
  ///
  /// Calculate Relative Power
  ///
  wPower1 = (mMaxBusRatio - FvidPointer[2].FvidState.BusRatio) * 625;
  wPower1 = (110000 - wPower1);
  wPower1 = DivU64x32 (wPower1, 11);
  wPower1 = DivU64x32 (MultU64x64 (wPower1, wPower1), 1000);
  //
  // Power is calculated in milliwatts
  //
  wPower2 = (((FvidPointer[2].FvidState.BusRatio * 100000) / mMaxBusRatio) / 100);
  wPower2 = DivU64x32 (MultU64x32 (MultU64x64 (wPower2, DivU64x32 (wPower1, 100)), mPackageTdpWatt), 1000);
  FvidPointer[2].FvidState.Power = (UINT16) wPower2;
}

/**
  Calculate the ratio for the requested p state

  @param[in] MaxRatio           Maximum Supported Ratio (HFM)
  @param[in] MinRatio           Minimum Supported Ratio (LFM)
  @param[in] MaxNumberOfStates  Number of entries in the table pointed to by FvidPointer
  @param[in] PStateNumber       Desired P State from range 0..MaxNumberOfStates

  @retval Ratio for the requested Pstate
**/
UINT16
ComputePstateRatio (
  IN UINT16 MaxRatio,
  IN UINT16 MinRatio,
  IN UINT16 MaxNumberOfStates,
  IN UINT16 PStateNumber
  )
{
  UINT16 RatioRange;
  UINT16 NumGaps;
  UINT16 PStateRatio;

  RatioRange  = MaxRatio - MinRatio;
  NumGaps     = MaxNumberOfStates - 1;
  PStateRatio = MaxRatio - (((PStateNumber * RatioRange) + (NumGaps / 2)) / NumGaps);

  return PStateRatio;
}

/**
  Create FVID table

  @param[in out] FvidPointer         Pointer to a table to be updated
  @param[in] MaxNumberOfStates   Number of entries in the table pointed to by FvidPointer

  @retval EFI_SUCCESS            FVID table created successfully.
  @retval EFI_INVALID_PARAMETER  The bus ratio range don't permit FVID table calculation;
                                 a default FVID table should be constructed.
**/
EFI_STATUS
CreateFvidTable (
  IN OUT FVID_TABLE *FvidPointer,
  IN UINT16         MaxNumberOfStates
  )
{
  UINT16 BusRatioRange;
  UINT16 NumberOfStates;
  UINT16 Turbo;
  UINT16 index;
  UINT16 StepSize;
  UINT16 MaxRatio;
  UINT64 wPower1;
  UINT64 wPower2;

  DEBUG((EFI_D_INFO,"\n\n== == CreateFvidTable == ==\n\n"));

  ///
  /// Determine whether turbo mode is supported or not
  ///
  Turbo = 0;
  Turbo = ((gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_TURBO) ? 1 : 0);
  DEBUG((EFI_D_INFO, "Turbo Mode:[%04x]\n", Turbo));

  ///
  /// Determine the bus ratio range
  ///
  if (Turbo) {
    BusRatioRange = mTurboBusRatio - mMinBusRatio;
    MaxRatio = mTurboBusRatio;
  } else {
    BusRatioRange = mMaxBusRatio - mMinBusRatio;
    MaxRatio = mMaxBusRatio;
  }
  if (((INT16) BusRatioRange < 0) || (MaxNumberOfStates == 0)) {
    return EFI_INVALID_PARAMETER;
  }
  DEBUG((EFI_D_INFO, "BusRatioRange:[%04x]\n", BusRatioRange));


  ///
  ///  Determine the number of states as cpu supported range or Maximum _PSS limit
  ///
  NumberOfStates = ((BusRatioRange + 1) < MaxNumberOfStates ? (BusRatioRange + 1) : MaxNumberOfStates);
  DEBUG((EFI_D_INFO, "NumberOfStates:[%04x]\n", NumberOfStates));
  ///
  /// Ensure we have at least two states
  ///
  if ((NumberOfStates + Turbo) < 2) {
    ///
    /// In case HFM = LFM and no Turbo, at least have two states with same ratio values
    ///
    NumberOfStates = 2;
  }
  ///
  /// Fill in the table header
  ///
  FvidPointer[0].FvidHeader.Stepping    = gCpuGlobalNvsAreaConfig->Area->Cpuid;
  FvidPointer[0].FvidHeader.MaxBusRatio = (Turbo ? mTurboBusRatio : mMaxBusRatio);
  FvidPointer[0].FvidHeader.EistStates  = (UINT16) (NumberOfStates < MaxNumberOfStates ? NumberOfStates : MaxNumberOfStates);

  DEBUG ((EFI_D_INFO, "FVID[00].FvidHeader.Stepping = %x\n", FvidPointer[0].FvidHeader.Stepping));
  DEBUG ((EFI_D_INFO, "FVID[00].FvidHeader.MaxBusRatio = %x\n", FvidPointer[0].FvidHeader.MaxBusRatio));
  DEBUG ((EFI_D_INFO, "FVID[00].FvidHeader.EistStates = %x\n", FvidPointer[0].FvidHeader.EistStates));

  ///
  /// Add Turbo as P0 if Turbo Mode supported and initialize.
  ///
  if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_TURBO) {
    FvidPointer[1].FvidState.BusRatio = mTurboBusRatio;
    FvidPointer[1].FvidState.Power = (mPackageTdpWatt * 1000); // power is calculated in milliwatts
  } else {
    FvidPointer[1].FvidState.BusRatio = mMaxBusRatio;
    FvidPointer[1].FvidState.Power = (mPackageTdpWatt * 1000);
  }
  DEBUG ((EFI_D_INFO, "FVID[1].FvidState.State = %x\n", FvidPointer[1].FvidState.State));
  DEBUG ((EFI_D_INFO, "FVID[1].FvidState.BusRatio = %x\n", FvidPointer[1].FvidState.BusRatio));
  DEBUG ((EFI_D_INFO, "FVID[1].FvidState.Power = %x\n", FvidPointer[1].FvidState.Power));

  ///
  /// Fill in the table starting at the last entry
  /// The algorithm is available in the processor BIOS writer's guide.
  ///
  for (index = 1, StepSize = 1; index < NumberOfStates; index++) {
    FvidPointer[index + 1].FvidState.State    = index;
    FvidPointer[index + 1].FvidState.BusRatio = FvidPointer[index].FvidState.BusRatio - StepSize;
    ///
    /// Calculate Relative Power
    ///
    wPower1 = (MaxRatio - FvidPointer[index + 1].FvidState.BusRatio) * 625;
    wPower1 = (110000 - wPower1);
    wPower1 = DivU64x32 (wPower1, 11);
    wPower1 = MultU64x64 (wPower1, wPower1);
    //
    // Power is calculated in milliwatts
    //
    wPower2 = (((FvidPointer[index + 1].FvidState.BusRatio * 100) / MaxRatio));
    wPower2 = DivU64x32 (MultU64x32 (MultU64x64 (wPower2, wPower1), mPackageTdpWatt), 10000000);
    FvidPointer[index + 1 + Turbo].FvidState.Power = (UINT32) wPower2;
    DEBUG ((EFI_D_INFO, "FVID[%02d].State = %x\n", (1 + index), FvidPointer[1 + index].FvidState.State));
    DEBUG ((EFI_D_INFO, "FVID[%02d].BusRatio = %x\n", (1 + index), FvidPointer[1 + index].FvidState.BusRatio));
    DEBUG ((EFI_D_INFO, "FVID[%02d].Power = %x\n", (1 + index), FvidPointer[1 + index].FvidState.Power));
  }

  return EFI_SUCCESS;
}

/**
  Set processor P state to HFM or LFM.

  @exception EFI_UNSUPPORTED  EIST not supported.
**/
VOID
SetBootPState (
  VOID
  )
{
  MSR_REGISTER Ia32MiscEnable;
  MSR_REGISTER Ia32PlatformInfo;
  BOOLEAN      EistEnabled;

  ///
  /// This function will be executed even when EIST is disabled so processor can be switched to HFM
  /// Only skip this when EIST is not capable.
  ///
  if ((mCpuid01.RegEcx & B_CPUID_VERSION_INFO_ECX_EIST) == 0) {
    return;
  }
  ///
  /// Read EIST.
  ///
  Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  EistEnabled = (BOOLEAN) RShiftU64 (
                  (Ia32MiscEnable.Qword & B_MSR_IA32_MISC_ENABLE_EIST),
                  N_MSR_IA32_MISC_ENABLE_EIST_OFFSET
                  );
  ///
  /// If EIST is disabled, temporarily enable it
  ///
  if (EistEnabled == 0) {
    Ia32MiscEnable.Qword |= B_MSR_IA32_MISC_ENABLE_EIST;
    AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);
  }

  Ia32PlatformInfo.Qword = AsmReadMsr64 (MSR_PLATFORM_INFO);
  if (gPowerMgmtConfig->BootPState == 0) { //HFM
    Ia32PlatformInfo.Qword &= B_PLATFORM_INFO_MAX_NON_TURBO_LIM_RATIO;
    mBspBootRatio = (UINT16) RShiftU64 (Ia32PlatformInfo.Qword, N_PLATFORM_INFO_MAX_NON_TURBO_LIM_RATIO);
  } else { //LFM
    Ia32PlatformInfo.Qword &= B_PLATFORM_INFO_MAX_EFFICIENCY_RATIO;
    mBspBootRatio = (UINT16) RShiftU64 (Ia32PlatformInfo.Qword, N_PLATFORM_INFO_MAX_EFFICIENCY_RATIO);
  }
  DEBUG ((EFI_D_INFO, "mBspBootRatio = %x\n", mBspBootRatio));

  ///
  /// Set P-state on all cores
  ///
  RunOnAllLogicalProcessors (ApSafeSetBootPState, NULL);
  ///
  /// Disable EIST if we enabled it previously
  ///
  if (EistEnabled == 0) {
    Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
    Ia32MiscEnable.Qword &= ~B_MSR_IA32_MISC_ENABLE_EIST;
    AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);
  }
  PostCode (0xC6F);
  return;
}

/**
  Set processor P state based on Boot ConfigTdp level.

  @param[in out] Buffer    Unused
**/
VOID
EFIAPI
ApSafeSetBootPState (
  IN OUT VOID *Buffer
  )
{
  MSR_REGISTER Ia32PerfCtl;
  UINT16       BootRatio;

  Ia32PerfCtl.Qword = AsmReadMsr64 (MSR_IA32_PERF_CTRL);
  Ia32PerfCtl.Qword &= ~B_IA32_PERF_CTRLP_STATE_TARGET;
  if (mCpuConfigTdpBootRatio != 0) {
    ///
    /// For ConfigTDP enabled SKU use (ConfigTDP boot ratio - 1 / TAR Ratio) as max non-turbo ratio
    ///
    BootRatio = mCpuConfigTdpBootRatio-1;
    //
    // If EIST is disabled use boot ratio ConfigTDP boot ratio / TAR+1.
    //
    if((gCpuGlobalNvsAreaConfig->Area->PpmFlags & PPM_EIST)== 0) {
     BootRatio = mCpuConfigTdpBootRatio;
    }
  } else {
    ///
    /// For Non-ConfigTDP enabled SKU set BSP ratio on all threads.
    ///
    BootRatio = mBspBootRatio;
  }
  Ia32PerfCtl.Qword |= LShiftU64 (BootRatio, N_IA32_PERF_CTRLP_STATE_TARGET);
  AsmWriteMsr64 (MSR_IA32_PERF_CTRL, Ia32PerfCtl.Qword);

  return;
}

/**
  Completes processor power management initialization
    (1) Initializes the TSC update variables.
    (2) Initializes the GV state for processors.
**/
VOID
InitPpmPost (
  VOID
  )
{
  PostCode (0xC6E);
  ///
  /// Set Boot P-state based on Policy.
  ///
//[-start-180727-IB07400990-modify]//
  //
  // Doc#570618, Rev1.4, Section 9.1, Workaround and Sample Code to Set HFM 
  //
//  if ((gPowerMgmtConfig->SkipSetBootPState != CPU_FEATURE_ENABLE) && (gPowerMgmtConfig->Eist == CPU_FEATURE_ENABLE)){
  if ((gPowerMgmtConfig->SkipSetBootPState != CPU_FEATURE_ENABLE)){
    SetBootPState ();
  }
//[-end-180727-IB07400990-modify]//
  ///
  /// Lock down all settings
  ///
  PpmLockDown ();
  PostCode (0xC74);
}

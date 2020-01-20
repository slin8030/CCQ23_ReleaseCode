/** @file
  Processor Power Management initialization code. This code determines current
  user configuration and modifies and loads ASL as well as initializing chipset
  and processor features to enable the proper power management.

  Acronyms:
    PPM - Processor Power Management
    TM  - Thermal Monitor
    IST - Intel(R) Speedstep technology
    HT  - Hyper-Threading Technology

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

#include "PowerMgmtInitPeim.h"
#include <Library/MmPciLib.h>
#include <Library/PreSiliconLib.h>
#include <Library/CpuPolicyLib.h>
#ifdef FSP_FLAG
#include <Library/FspCommonLib.h>
#include <FspsUpd.h>
#endif
extern EFI_GUID gCpuInitDataHobGuid;

//
// Global variables
//

///
/// Power Managment policy configurations
///
SI_CPU_POLICY_PPI           *gSiCpuPolicy        = NULL;
POWER_MGMT_CONFIG           *gPowerMgmtConfig    = NULL;
EFI_CPUID_REGISTER mCpuid01                      = { 0, 0, 0, 0 };            // CPUID 01 values
//
// Values for FVID table calculate.
//
UINT16 mTurboBusRatio             = 0;
UINT16 mMaxBusRatio               = 0;
UINT16 mMinBusRatio               = 0;
UINT16 mProcessorFlavor           = 0;
UINT16 mBspBootRatio              = 0;
UINT16 mPackageTdp                = 0;                        ///< Processor TDP value in MSR_PACKAGE_POWER_SKU.
UINT16 mPackageTdpWatt            = 0;                        ///< Processor TDP value in Watts.
UINT16 mCpuConfigTdpBootRatio     = 0;                        ///< Config TDP Boot settings
UINT16 mCustomPowerUnit           = 1;
UINT16 mCpuCacheSize              = 0;                        ///< Cache Size in KB

///
/// Fractional part of Processor Power Unit in Watts. (i.e. Unit is 1/mProcessorPowerUnit)
///
UINT8 mProcessorPowerUnit        = 0;
///
/// Maximum allowed power limit value in TURBO_POWER_LIMIT_MSR and PRIMARY_PLANE_POWER_LIMIT_MSR
/// in units specified by PACKAGE_POWER_SKU_UNIT_MSR
///
UINT16 mPackageMaxPower           = 0;
///
/// Minimum allowed power limit value in TURBO_POWER_LIMIT_MSR and PRIMARY_PLANE_POWER_LIMIT_MSR
/// in units specified by PACKAGE_POWER_SKU_UNIT_MSR
///
UINT16 mPackageMinPower           = 0;

UINT8                        mRatioLimitProgrammble     = 0;     ///< Programmable Ratio Limit
UINT8                        mTdpLimitProgrammble       = 0;     ///< Porgrammable TDP Limit
CPU_GLOBAL_NVS_AREA_CONFIG   mCpuGlobalNvsAreaConfig    = {0};
CPU_GLOBAL_NVS_AREA_CONFIG   *gCpuGlobalNvsAreaConfig   = NULL;

//
// FVID Table Information
// Default FVID table
// One header field plus states
//
UINT16     mNumberOfStates            = 0;
FVID_TABLE *mFvidPointer              = NULL;

/**
  Power Management init after memory PEI module

  @retval EFI_SUCCESS     The driver installes/initialized correctly.
  @retval Driver will ASSERT in debug builds on error.  PPM functionality is considered critical for mobile systems.
**/
EFI_STATUS
CpuPowerMgmtInit (
  VOID
  )
{
  EFI_STATUS                 Status;
  UINTN                      Size;
  EFI_PHYSICAL_ADDRESS       Area;
  EFI_BOOT_MODE              BootMode;
  VOID                       *Hob;
  CPU_INIT_DATA_HOB          *CpuInitDataHob;
#ifdef FSP_FLAG
  FSPS_UPD                   *FspsUpd;
#endif

#ifdef FSP_FLAG
  FspsUpd = (FSPS_UPD *)GetFspSiliconInitUpdDataPointer ();
#endif

  Status = PeiServicesGetBootMode (&BootMode);
  if ((Status == EFI_SUCCESS) && (BootMode == BOOT_ON_S3_RESUME)) {
#ifdef FSP_FLAG
    if (!(FspsUpd->FspsConfig.InitS3Cpu)) {
#endif
    return EFI_SUCCESS;
#ifdef FSP_FLAG
    }
#endif
  }

  DEBUG ((DEBUG_INFO, "PeimInitializePowerMgmt Start\n"));
  PostCode (0xC60);

  ///
  /// Locate platform configuration information
  ///
  Status = PeiServicesLocatePpi (
                         &gSiCpuPolicyPpiGuid,
                         0,
                         NULL,
                         (VOID **) &gSiCpuPolicy);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Initialize the Global pointer for Power Managment Policy
  ///
  Status = GetConfigBlock ((CONFIG_BLOCK_TABLE_HEADER *)gSiCpuPolicy, &gPowerMgmtConfigGuid , (VOID *)&gPowerMgmtConfig);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Get CPU Init Data Hob
  ///
  CpuInitDataHob = NULL;
  Hob = GetFirstGuidHob (&gCpuInitDataHobGuid);
  if (Hob == NULL) {
    DEBUG ((DEBUG_ERROR, "CPU Data HOB not available\n"));
    return EFI_NOT_FOUND;
  }
  CpuInitDataHob = (CPU_INIT_DATA_HOB *) ((UINTN) Hob + sizeof (EFI_HOB_GUID_TYPE));

  ///
  /// Allocated ACPI NVS type memory for CPU Global NVS Configuration.
  ///
  Size = sizeof (CPU_GLOBAL_NVS);
  Status = PeiServicesAllocatePages (EfiACPIMemoryNVS, EFI_SIZE_TO_PAGES (Size), &Area);
  ASSERT_EFI_ERROR (Status);
  gCpuGlobalNvsAreaConfig = &mCpuGlobalNvsAreaConfig;
  mCpuGlobalNvsAreaConfig.Area = (CPU_GLOBAL_NVS *) (UINTN) Area;
  ZeroMem ((VOID *) mCpuGlobalNvsAreaConfig.Area, sizeof (CPU_GLOBAL_NVS));

  ///
  /// Saved CPU Global NVS pointer to CpuInitDataHob
  ///
  CpuInitDataHob->CpuGnvsPointer = (EFI_PHYSICAL_ADDRESS) (UINTN) Area;

  mCpuGlobalNvsAreaConfig.Area->Revision = CPU_GLOBAL_NVS_AREA_REVISION_1;

  ///
  /// Initialize FVID table pointer
  ///
  mFvidPointer = (FVID_TABLE *)(UINTN)CpuInitDataHob->FvidTable;

  ///
  /// Initialize Power management Global variables
  ///
  InitPowerManagementGlobalVariables ();

  ///
  /// Initialize CPU Power management code (determine HW and configured state, configure hardware and software accordingly)
  ///
  Status = InitPpm ();
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, " PeimInitializePowerMgmt End\n"));
  PostCode (0xC7F);
  return EFI_SUCCESS;
}

/**
  Initializes the platform power management global variable.
  This must be called prior to any of the functions being used.
**/
VOID
InitPowerManagementGlobalVariables (
  VOID
  )
{
  MSR_REGISTER        TempMsr;
  MSR_REGISTER        PackagePowerSKUUnitMsr;
  MSR_REGISTER        PlatformInfoMsr;
  UINT16              Associativity;
  UINT16              CachePartitions;
  UINT16              CacheLineSize;
  UINT16              CacheNumberofSets;
  EFI_CPUID_REGISTER  Cpuid04;

  PostCode (0xC61);
  //
  // Read the CPUID information
  //
  AsmCpuid (CPUID_VERSION_INFO, &mCpuid01.RegEax, &mCpuid01.RegEbx, &mCpuid01.RegEcx, &mCpuid01.RegEdx);
  mCpuGlobalNvsAreaConfig.Area->Cpuid = GetCpuFamily() | GetCpuStepping();
  //
  // Get Platform ID
  //
  TempMsr.Qword = AsmReadMsr64 (MSR_IA32_PLATFORM_ID);
  mProcessorFlavor = (UINT8) (RShiftU64(TempMsr.Qword, 50) & B_PLATFORM_ID_MASK);
  //
  // Read the CPUID4 information for LLC size
  //
  AsmCpuidEx (CPUID_FUNCTION_4, 0x3, &Cpuid04.RegEax, &Cpuid04.RegEbx, &Cpuid04.RegEcx, &Cpuid04.RegEdx);
  //
  // Determine Cache Size in Kilo Bytes
  // This Cache Size in Bytes = (Associativity) * (Partitions + 1) * (Line_Size + 1) * (Sets + 1)
  // = (EBX[31:22] + 1) * (EBX[21:12] + 1) * (EBX[11:0] + 1) * (ECX + 1)
  //
  Associativity     = (UINT16) (((Cpuid04.RegEbx >> 22) & CPU_CACHE_ASSOCIATIVITY_MASK) + 1);
  CachePartitions   = (UINT16) (((Cpuid04.RegEbx >> 12) & CPU_CACHE_PARTITION_MASK) + 1);
  CacheLineSize     = (UINT16) (((UINT16) Cpuid04.RegEbx & CPU_CACHE_LINE_SIZE_MASK) + 1);
  CacheNumberofSets = (UINT16) (Cpuid04.RegEcx + 1);
  mCpuCacheSize = (UINT16) ((Associativity * CachePartitions * CacheLineSize * CacheNumberofSets) / 1024);
  ///
  /// Get Maximum Non-Turbo bus ratio (HFM) from Platform Info MSR Bits[15:8]
  ///
  PlatformInfoMsr.Qword = AsmReadMsr64 (MSR_PLATFORM_INFO);
  mMaxBusRatio          = PlatformInfoMsr.Bytes.SecondByte;
  ///
  /// Get Maximum Efficiency bus ratio (LFM) from Platform Info MSR Bits[47:40]
  ///
  mMinBusRatio = PlatformInfoMsr.Bytes.SixthByte;
  ///
  /// Get Max Turbo Ratio from Turbo Ratio Limit MSR Bits [7:0]
  ///
  TempMsr.Qword   = AsmReadMsr64 (MSR_TURBO_RATIO_LIMIT);
  mTurboBusRatio  = (UINT16) (TempMsr.Dwords.Low & B_MSR_TURBO_RATIO_LIMIT_1C);
  ///
  /// Check if Turbo Ratio Limit is programmable
  ///  Platform Info MSR (0xCE) [28]
  ///
  mRatioLimitProgrammble = (UINT8) RShiftU64 ((PlatformInfoMsr.Qword & B_PLATFORM_INFO_RATIO_LIMIT), 28);
  ///
  /// Check if TDP Limit is programmable
  ///  Platform Info MSR (0xCE) [29]
  ///
  mTdpLimitProgrammble = (UINT8) RShiftU64 ((PlatformInfoMsr.Qword & B_PLATFORM_INFO_TDC_TDP_LIMIT), 29);

  ///
  /// Get Processor TDP
  /// Get Maximum Power from Turbo Power Limit MSR Bits[14:0]
  /// and convert it to units specified by Package Power SKU
  /// Unit MSR [3:0]
  ///
  TempMsr.Qword                 = AsmReadMsr64 (MSR_PACKAGE_POWER_SKU);
  PackagePowerSKUUnitMsr.Qword  = AsmReadMsr64 (MSR_PACKAGE_POWER_SKU_UNIT);
  mProcessorPowerUnit           = (PackagePowerSKUUnitMsr.Bytes.FirstByte & PACKAGE_POWER_UNIT_MASK);
  if (mProcessorPowerUnit == 0) {
    mProcessorPowerUnit = 1;
  } else {
    mProcessorPowerUnit = (UINT8) LShiftU64 (2, (mProcessorPowerUnit - 1));
  }
  //mPackageTdp = (UINT16)(TempMsr.Dwords.Low & PACKAGE_TDP_POWER_MASK);
  //mPackageTdpWatt = (UINT16) DivU64x32 (mPackageTdp , mProcessorPowerUnit);
  mPackageMaxPower  = (UINT16) (TempMsr.Dwords.High & PACKAGE_MAX_POWER_MASK);
  mPackageMinPower  = (UINT16) RShiftU64 ((TempMsr.Dwords.Low & PACKAGE_MIN_POWER_MASK), 16);

  ///
  /// Set mCustomPowerUnit to user selected Power unit
  ///
  mCustomPowerUnit = 1;
  if (gPowerMgmtConfig->CustomPowerUnit == PowerUnit125MilliWatts) {
    //
    // Unit is 125 milli watt
    //
    mCustomPowerUnit = 8;
  }

  ///
  /// If specified, create a custom the FVID table.
  /// (The settings populating the FVID table may not be correct for the
  /// specific processor, and it is up to the user to specify settings
  /// applicable to the processor being used.)
  ///
  if (gPowerMgmtConfig->NumberOfEntries >= 2) {
    CreateCustomFvidTable (mFvidPointer);
  }

  ///
  /// Initialize flags based on processor capablities
  ///
  SetPpmFlags ();

  ///
  /// Determine current user configuration
  ///
  SetUserConfigurationPpmFlags ();
  PostCode (0xC68);
  return;
}

/**
  Create a custom FVID table based on setup options.
  Caller is responsible for providing a large enough table.

  @param[in] FvidPointer   Table to update, must be initialized.
**/
VOID
CreateCustomFvidTable (
  IN OUT FVID_TABLE *FvidPointer
  )
{
  UINT16 Index;
  PostCode (0xC62);
  ///
  /// BWG Section 14.6.2 Determining Number of Operating Points
  /// It is recommended that system BIOS limit the number of P states to 16
  ///
  if (gPowerMgmtConfig->NumberOfEntries > FVID_MAX_STATES) {
    DEBUG (
            (DEBUG_WARN,
             "VidNumber(%d) is greater than maximum(%d) supported.",
             gPowerMgmtConfig->NumberOfEntries,
             FVID_MAX_STATES)
            );
    gPowerMgmtConfig->NumberOfEntries = FVID_MAX_STATES;
  }
  ///
  /// Fill in the table header
  ///
  FvidPointer[0].FvidHeader.Stepping    = gPowerMgmtConfig->Cpuid;
  FvidPointer[0].FvidHeader.MaxBusRatio = gPowerMgmtConfig->MaxRatio;
  FvidPointer[0].FvidHeader.EistStates  = gPowerMgmtConfig->NumberOfEntries;
  ///
  /// Fill in the state data
  ///
  for (Index = 0; Index < gPowerMgmtConfig->NumberOfEntries; Index++) {
    FvidPointer[Index + 1].FvidState.State    = Index;
    FvidPointer[Index + 1].FvidState.BusRatio = gPowerMgmtConfig->StateRatio[Index];
  }
}

/**
  Set the PPM flags
**/
VOID
SetPpmFlags (
  VOID
  )
{
  MSR_REGISTER       Ia32MiscEnable;
  EFI_CPUID_REGISTER Cpuid01;
  EFI_CPUID_REGISTER Cpuid05;
  EFI_CPUID_REGISTER Cpuid06;
  UINTN              States;
  BOOLEAN            CpuidLimitingEnabled;
  UINT32             PpmFlags;

  ZeroMem (&Cpuid01, sizeof (Cpuid01));
  ZeroMem (&Cpuid05, sizeof (Cpuid05));
  ZeroMem (&Cpuid06, sizeof (Cpuid06));
  PpmFlags = 0;

  PostCode (0xC63);
  ///
  /// Set the CMP Flags
  ///
  PpmFlags |= PPM_CMP;
  ///
  /// Check TM capable and update the flag
  ///
  if (mCpuid01.RegEdx & B_CPUID_VERSION_INFO_EDX_TM1) {
    PpmFlags |= PPM_TM;
  }
  ///
  /// Check EIST capable. If EIST capable, also set the boot P-state to HFM flag.
  ///
  if (mCpuid01.RegEcx & B_CPUID_VERSION_INFO_ECX_EIST) {
    PpmFlags |= (PPM_EIST);
    DEBUG ((DEBUG_INFO, "EIST capable\n"));
  }
  ///
  /// Disable CPUID limiting (and save current setting) if enabled
  /// and enable MONITOR/MWAIT support
  ///
  Ia32MiscEnable.Qword  = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  CpuidLimitingEnabled  = (BOOLEAN) (Ia32MiscEnable.Qword & B_MSR_IA32_MISC_ENABLE_CPUID_MAX);
  if (CpuidLimitingEnabled) {
    Ia32MiscEnable.Qword &= ~B_MSR_IA32_MISC_ENABLE_CPUID_MAX;
  }
  AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);
  ///
  /// Read the CPUID values we care about.  We cannot use the stored
  /// values because they may have changes since we disabled limiting
  /// and enabled MONITOR/MWAIT
  ///
  AsmCpuid (1, &Cpuid01.RegEax, &Cpuid01.RegEbx, &Cpuid01.RegEcx, &Cpuid01.RegEdx);
  AsmCpuid (5, &Cpuid05.RegEax, &Cpuid05.RegEbx, &Cpuid05.RegEcx, &Cpuid05.RegEdx);
  AsmCpuid (6, &Cpuid06.RegEax, &Cpuid06.RegEbx, &Cpuid06.RegEcx, &Cpuid06.RegEdx);
  ///
  /// Determine if the MONITOR/MWAIT instructions are supported.
  ///
  if ((Cpuid01.RegEcx & B_CPUID_VERSION_INFO_ECX_MWAIT && Cpuid05.RegEcx & B_CPUID_MONITOR_MWAIT_ECX_EXTENSIONS)) {
    PpmFlags |= PPM_MWAIT_EXT;
  }
  ///
  /// Determine the C-State and Enhanced C-State support present.
  /// Monitor/MWAIT parameters function describes the numbers supported.
  ///
  States = RShiftU64 (Cpuid05.RegEdx, 4) & 0xF;
  if (States >= ENHANCED_CSTATE_SUPPORTED) {
    PpmFlags |= PPM_C1 + PPM_C1E;
  } else if (States == CSTATE_SUPPORTED) {
    PpmFlags |= PPM_C1;
  }
  States = RShiftU64 (Cpuid05.RegEdx, 8) & 0xF;
  if ((States >= CSTATE_SUPPORTED) && (PpmFlags & PPM_C1)) {
    PpmFlags |= PPM_C3;
  }
  States = RShiftU64 (Cpuid05.RegEdx, 12) & 0xF;
  ///
  /// Both Long and Short Latency C6 supported
  ///
  if (States >= C6_C7_LONG_LATENCY_SUPPORTED) {
    PpmFlags |= (PPM_C6 | C6_LONG_LATENCY_ENABLE);
  } else if (States >= C6_C7_SHORT_LATENCY_SUPPORTED) { // Only Short Latency C6 supported.
    PpmFlags |= PPM_C6;
  }

  States = RShiftU64 (Cpuid05.RegEdx, 16) & 0xF;
  switch (States) {
    case C7s_LONG_LATENCY_SUPPORTED:
      //
      // C7 & C7s Long and Short supported
      //
      PpmFlags |= (PPM_C7S | C7s_LONG_LATENCY_ENABLE | PPM_C7 | C7_LONG_LATENCY_ENABLE);
      break;
    case C7s_SHORT_LATENCY_SUPPORTED:
      //
      // C7s Long Latency is not supported.
      //
      PpmFlags |= (PPM_C7S | PPM_C7 | C7_LONG_LATENCY_ENABLE);
      break;
    case C6_C7_LONG_LATENCY_SUPPORTED:
      //
      // C7 Long and Short supported
      //
      PpmFlags |= (PPM_C7 | C7_LONG_LATENCY_ENABLE);
      break;
    case C6_C7_SHORT_LATENCY_SUPPORTED:
      //
      // C7 Long Latency is not supported.
      //
      PpmFlags |= PPM_C7;
      break;
    default:
      break;
  }

  States = RShiftU64 (Cpuid05.RegEdx, 20) & 0xF;
  if (States >= CSTATE_SUPPORTED) {
    PpmFlags |= PPM_C8;
  }
  States = RShiftU64 (Cpuid05.RegEdx, 24) & 0xF;
  if (States >= CSTATE_SUPPORTED) {
    PpmFlags |= PPM_C9;
  }
  States = RShiftU64 (Cpuid05.RegEdx, 28) & 0xF;
  if (States >= CSTATE_SUPPORTED) {
    PpmFlags |= PPM_C10;
  }
  ///
  /// Check TimedMwait is supported and update the flag
  ///
  if (AsmReadMsr64 (MSR_PLATFORM_INFO) & B_PLATFORM_INFO_TIMED_MWAIT_SUPPORTED) {
    PpmFlags |= PPM_TIMED_MWAIT;
  }
  if (PpmFlags & (PPM_C8 |PPM_C9 | PPM_C10)) {
    PpmFlags |= PPM_CD;
  }
  ///
  /// Check if turbo mode is supported and update the flag
  ///
  Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  if (((Cpuid06.RegEax & B_CPUID_POWER_MANAGEMENT_EAX_TURBO) == 0) &&
      ((Ia32MiscEnable.Qword & B_MSR_IA32_MISC_DISABLE_TURBO) == 0)
      ) {
    ///
    /// Turbo Mode is not available in this physical processor package.
    /// BIOS should not attempt to enable Turbo Mode via IA32_MISC_ENABLE MSR.
    /// BIOS should show Turbo Mode as Disabled and Not Configurable.
    ///
  } else if ((Cpuid06.RegEax & B_CPUID_POWER_MANAGEMENT_EAX_TURBO) == 0) {
    ///
    /// Turbo Mode is available but globally disabled for the all logical
    /// processors in this processor package.
    /// BIOS can enable Turbo Mode by IA32_MISC_ENABLE MSR 1A0h bit [38] = 0.
    ///
    PpmFlags |= PPM_TURBO;
  } else if ((Cpuid06.RegEax & B_CPUID_POWER_MANAGEMENT_EAX_TURBO) == B_CPUID_POWER_MANAGEMENT_EAX_TURBO) {
    ///
    /// Turbo Mode is factory-configured as available and enabled for all logical processors in this processor package.
    /// This case handles the cases where turbo mode is enabled before PPM gets chance to enable it
    ///
    PpmFlags |= PPM_TURBO;
  }
  ///
  /// Restore the CPUID limit setting.
  ///
  if (CpuidLimitingEnabled) {
    Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
    Ia32MiscEnable.Qword |= B_MSR_IA32_MISC_ENABLE_CPUID_MAX;
    AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);
  }
  ///
  /// Set the T-states flag
  ///
  PpmFlags |= PPM_TSTATES;
  ///
  /// Determine if Fine grained clock modulation contol is supported
  ///
  if (Cpuid06.RegEax & B_CPUID_POWER_MANAGEMENT_EAX_FINE_GRAINED_CLOCK_MODULATION) {
    PpmFlags |= PPM_TSTATE_FINE_GRAINED;
  }
  ///
  /// Energy Efficient P-state feature is supported
  ///
  PpmFlags |= PPM_EEPST;
  ///
  /// Update the PPM NVS area PPM flags
  ///
  mCpuGlobalNvsAreaConfig.Area->PpmFlags = PpmFlags;
  return;
}

/**
  Set the PPM flags based on current user configuration
**/
VOID
SetUserConfigurationPpmFlags (
  VOID
  )
{
  UINT32 UserPpmFlag;
  PostCode (0xC64);
  //
  // In advance to clear following PPM flags which are related with policies that user can enabled/disabled.
  //
  UserPpmFlag = (UINT32)~(PPM_EIST | PPM_C1 | PPM_C1E | PPM_TM | PPM_TURBO | PPM_TSTATES |
                          PPM_TSTATE_FINE_GRAINED | PPM_EEPST | PPM_TIMED_MWAIT);
  ///
  /// Configure flag based on user selections
  ///
  if (gPowerMgmtConfig->Eist) {
    UserPpmFlag |= PPM_EIST;
  }
  if (gPowerMgmtConfig->Cx) {
    UserPpmFlag |= PPM_C1;
    if (gPowerMgmtConfig->C1e) {
      UserPpmFlag |= PPM_C1E;
    }
  } else {
    UserPpmFlag &= ~(PPM_C3 | PPM_C6 | C6_LONG_LATENCY_ENABLE |
                     PPM_C7S | PPM_C7 | C7_LONG_LATENCY_ENABLE | C7s_LONG_LATENCY_ENABLE | PPM_CD |
                     PPM_C8 | PPM_C9 | PPM_C10);
  }
  if (gPowerMgmtConfig->ThermalMonitor) {
    UserPpmFlag |= PPM_TM;
  }
  if (gPowerMgmtConfig->TurboMode) {
    UserPpmFlag |= PPM_TURBO;
  }
  if (gPowerMgmtConfig->TStates) {
    UserPpmFlag |= (PPM_TSTATES | PPM_TSTATE_FINE_GRAINED);
  }
  if (gPowerMgmtConfig->TimedMwait) {
    UserPpmFlag |= PPM_TIMED_MWAIT;
  }
  ///
  /// Modify PpmFlags based on user selections
  ///
  mCpuGlobalNvsAreaConfig.Area->PpmFlags &= UserPpmFlag;
}

/**
  Initialize the processor power management based on hardware capabilities
  and user configuration settings.

  @retval EFI_SUCCESS - on success
  @retval Appropiate failure code on error
**/
EFI_STATUS
InitPpm (
  VOID
  )
{
  EFI_STATUS Status;
  Status      = EFI_SUCCESS;
  DEBUG((DEBUG_INFO, "InitPpm Start\n"));
  PostCode (0xC69);

  DEBUG((DEBUG_INFO, "Initialize P states Start\n"));
  ///
  /// Initialize P states
  ///
  InitPStates();

  DEBUG((DEBUG_INFO, "Initialize C State Start\n"));
  ///
  /// Initialize C State(IdleStates)
  ///
  InitCState();

  DEBUG((DEBUG_INFO, "Initialize thermal features Start\n"));
  ///
  /// Initialize thermal features
  ///
  InitThermal ();

  DEBUG((DEBUG_INFO, "Initialise Miscellaneous features Start\n"));
  ///
  /// Initialise Miscellaneous features
  ///
  InitMiscFeatures();

  DEBUG((DEBUG_INFO, "Complete with Ppmpost initialization Start\n"));
  ///
  /// Complete with Ppmpost initialization
  ///
  InitPpmPost ();

  DEBUG((DEBUG_INFO, "InitPpm End\n"));
  PostCode (0xC78);
  return Status;
}

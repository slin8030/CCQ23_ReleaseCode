/** @file
  This file contains power management configuration functions for processors.

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
#include <Private/Library/CpuCommonLib.h>

/**
  Configures following fields of MCHBAR 0x71F0
  Configures power limit 3 and 4 power level and time window
**/
VOID
ConfigurePl3AndPl4PowerLimits (
  VOID
  )
{
  UINTN        PciD0F0RegBase;
  UINTN        MchBar;

  ///
  /// Control Power Limit 3 (PL3) and Power Limit 4 (PL4) using this register
  /// Set PL3_CONTROL.POWER_LIMIT(bit 14-0) is 30w and PL3_CONTROL.PL4_PMAX(bit 46-32) is 50w
  /// PL3_CONTROL.TIME_WINDOW(bit 23-17) is 0.04s and PL3_CONTROL.DUTY_CYCLE(bit 30-24) is 10%
  ///
  PciD0F0RegBase  = MmPciBase (0, 0, 0);
  MchBar          = MmioRead32 (PciD0F0RegBase + 0x48) &~BIT0;
  MmioOr64 (MchBar + MMIO_PL3_CONTROL, V_PL3_POWER_LIMIT + V_PL3_TIME_WINDOW + V_PL3_DUTY_CYCLE + V_PL4_PMAX);

}

/**
  Configure VR Current Config of MSR 601
**/
VOID
ConfigureVrCurrentConfig (
  VOID
  )
{
  MSR_REGISTER PowerLimit4Msr;
  UINT8       Psi3_Threshold;
  UINT8       Psi2_Threshold;

  ///
  /// Return if No user overrides selected.
  ///
  if (gPowerMgmtConfig->VrConfig == POWER_MGMT_VR_DISABLE){
    return;
  }
  DEBUG ((DEBUG_INFO,"PowerMgmt VrConfig %x\n", gPowerMgmtConfig->VrConfig));

  Psi3_Threshold = 0x8;
  Psi2_Threshold = 0x10;
  PowerLimit4Msr.Qword = AsmReadMsr64 (MSR_VR_CURRENT_CONFIG);
  DEBUG ((DEBUG_INFO,"MSR_VR_CURRENT_CONFIG MSR 601 Before Writing %x\n", PowerLimit4Msr.Qword));
//
//   0x00000001 : ps4_enable (62:62)
//   0x00000008 : psi3_threshold (61:52)
//   0x00000010 : psi2_threshold (51:42)
//
  PowerLimit4Msr.Dwords.High |= (UINT32) LShiftU64 (Psi3_Threshold, 20);
  PowerLimit4Msr.Dwords.High |= (UINT32) LShiftU64 (Psi2_Threshold, 10);
  PowerLimit4Msr.Qword |= (B_MSR_VR_CURRENT_CONFIG_PS4_ENABLE);

  AsmWriteMsr64 (MSR_VR_CURRENT_CONFIG, PowerLimit4Msr.Qword);
  DEBUG ((DEBUG_INFO,"MSR_VR_CURRENT_CONFIG MSR 601 After Writing %x\n", PowerLimit4Msr.Qword));
}

/**
  Configures following fields of MSR 0x610
    Configures Long duration Turbo Mode (power limit 1) power level and time window
    Configures Short duration turbo mode (power limit 2)
**/
VOID
ConfigureTdpPowerLimits (
  VOID
  )
{
  MSR_REGISTER PakagePowerLimitMsr;
  MSR_REGISTER PakagePowerSKUMsr;
  UINT16       ConvertedPowerLimit1;
  UINT16       ConvertedPowerLimit2;
  UINT8        ConvertedPowerLimit1Time;
  UINT8        ConvertedPowerLimit2Time;
  UINT16       PowerUnitWatts;
  UINT16       FuseSocTdp;
  UINTN        PciD0F0RegBase;
  UINTN        MchBar;

  ConvertedPowerLimit1Time = 0;
  ConvertedPowerLimit2Time = 0;
  FuseSocTdp               = 0;

  ///
  /// By default, for Mobile & Desktop Processors: Package TDP is 1 W / Power2PWR_UNIT.
  /// The default value of 1000b corresponds to 1/256 W.
  ///
  PowerUnitWatts = 256;

  ///
  ///  Initialize the Power Limit 1/2 and Power Limit 2 enable bit in MSR
  ///  Power Limit 1: Turbo Power Limit MSR  [14:0] and Power Limit 2: Turbo Power Limit MSR  [46:32]
  ///  Set MSR value for Power Limit 1/2 to Max Package Power Value or Maximum Supported Value
  ///
  PakagePowerLimitMsr.Qword = AsmReadMsr64 (MSR_PACKAGE_POWER_LIMIT);
  PakagePowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_MASK;
  PakagePowerLimitMsr.Dwords.High &= ~POWER_LIMIT_MASK;

  ///
  /// Set Power Limit 1: Turbo Power Limit MSR  [14:0] is FUSE_SOC_TDP
  ///
  PakagePowerSKUMsr.Qword = AsmReadMsr64 (MSR_PACKAGE_POWER_SKU);
  FuseSocTdp = (UINT16) PakagePowerSKUMsr.Dwords.Low;
  DEBUG ((DEBUG_INFO,"FuseSocTdp %x\n",FuseSocTdp));

  ///
  /// Setting PL1 from Read SOC TDP FUSE
  ///
  ConvertedPowerLimit1 = FuseSocTdp;

  if((FuseSocTdp == 0) || (FuseSocTdp == DESKTOP_SKU_10)) {
    if(FuseSocTdp == 0){
      DEBUG ((DEBUG_INFO,"FuseSocTdpFuse is 0\n"));
      ///
      /// Temp HC PL1 to 10W(Desktop SKU), Since FUSE_SOC_TDP returns all "0"
      ///
      ConvertedPowerLimit1 = PowerUnitWatts * DT_POWER_LIMIT1;
    }
    ///
    /// Set Power Limit 2: Turbo Power Limit MSR  [46:32]. For Desktop, default value is 25w
    ///
    ConvertedPowerLimit2 = PowerUnitWatts * DT_POWER_LIMIT2;
    ///
    /// Set Desktop Power Limit 1 time
    ///
    gPowerMgmtConfig->PowerLimit1Time = DT_POWER_LIMIT1_TIME_DEFAULT;
  } else {
    ///
    /// Mobile SKU
    ///
    DEBUG ((DEBUG_INFO,"Mobile SKU\n"));
    ///
    /// Set 15W for Mobile Sku
    ///
    ConvertedPowerLimit2 = PowerUnitWatts * MB_POWER_LIMIT2;
    ///
    /// Set Mobile Power Limit 1 time
    ///
    gPowerMgmtConfig->PowerLimit1Time = MB_POWER_LIMIT1_TIME_DEFAULT;
  }

  ///
  /// Override with Setup Option Value
  ///
  if (gPowerMgmtConfig->CustomPowerLimit1 != 0) {
    ConvertedPowerLimit1 = PowerUnitWatts * gPowerMgmtConfig->CustomPowerLimit1;
  }
  if (gPowerMgmtConfig->CustomPowerLimit1Time != 0) {
    gPowerMgmtConfig->PowerLimit1Time = gPowerMgmtConfig->CustomPowerLimit1Time;
  }

  ConvertedPowerLimit1Time = GetConvertedTime (gPowerMgmtConfig->PowerLimit1Time, PL1TimeWindowConvert);
  ConvertedPowerLimit2Time = 0;

  ///
  /// Get the MCH space base address
  ///
  PciD0F0RegBase  = MmPciBase (0, 0, 0);
  MchBar          = MmioRead32 (PciD0F0RegBase + 0x48) &~BIT0;

  ///
  ///  Configure PACKAGE_RAPL_LIMIT MSR
  ///
  PakagePowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_1_TIME_MASK;
  if (gPowerMgmtConfig->PowerLimit1Enable == TRUE) {
    PakagePowerLimitMsr.Dwords.Low |= B_POWER_LIMIT_ENABLE;
  } else {
    PakagePowerLimitMsr.Dwords.Low &= ~B_POWER_LIMIT_ENABLE;
  }
  if (gPowerMgmtConfig->PowerLimit1ClampEnable == TRUE) {
    PakagePowerLimitMsr.Dwords.Low |= B_CRITICAL_POWER_CLAMP_ENABLE;
  } else {
    PakagePowerLimitMsr.Dwords.Low &= ~B_CRITICAL_POWER_CLAMP_ENABLE;
  }
  PakagePowerLimitMsr.Dwords.Low |= (UINT32) LShiftU64 (ConvertedPowerLimit1Time, 17);
  PakagePowerLimitMsr.Dwords.Low |= (UINT32) (ConvertedPowerLimit1);

  PakagePowerLimitMsr.Dwords.High &= ~POWER_LIMIT_1_TIME_MASK;
  PakagePowerLimitMsr.Dwords.High |= B_POWER_LIMIT_ENABLE;
  PakagePowerLimitMsr.Dwords.High |= (UINT32) LShiftU64 (ConvertedPowerLimit2Time, 17);
  PakagePowerLimitMsr.Dwords.High |= (UINT32) (ConvertedPowerLimit2);
  MmioWrite64(MchBar + MMIO_RAPL_LIMIT, PakagePowerLimitMsr.Qword);
  AsmWriteMsr64 (MSR_PACKAGE_POWER_LIMIT, PakagePowerLimitMsr.Qword);
}

/**
  Configures following fields of MSR 0x618 based on corresponding MMIO register (MCHBAR+0x58E0):
    Configures Long duration Turbo Mode (power limit 1) power level and time window for DDR domain
    Configures Short duration Turbo Mode (power limit 2) power level and time window for DDR domain
**/
VOID
ConfigureDdrPowerLimits (
  VOID
  )
{
  MSR_REGISTER  DdrPowerLimitMsr;
  UINTN         PciD0F0RegBase;
  UINTN         MchBar;

  if (gPowerMgmtConfig->DdrPowerLimit == AUTO) {
    return;
  }

  PciD0F0RegBase  = MmPciBase (0, 0, 0);
  MchBar          = MmioRead32 (PciD0F0RegBase + 0x48) &~BIT0;

  DdrPowerLimitMsr.Qword = 0;
  DdrPowerLimitMsr.Qword = MmioRead64 (MchBar + MMIO_DDR_RAPL_LIMIT);

  DEBUG (
    (DEBUG_INFO,
    "DDR Power Limit 1 = %d\n",
    DdrPowerLimitMsr.Dwords.Low & POWER_LIMIT_MASK)
    );
  DEBUG (
    (DEBUG_INFO,
    "DDR Power Limit 2 = %d\n",
    DdrPowerLimitMsr.Dwords.High & POWER_LIMIT_MASK)
    );

  AsmWriteMsr64 (MSR_DDR_RAPL_LIMIT, DdrPowerLimitMsr.Qword);
}

/**
  Configures MSR 0x65C platform power limits (PSys)
    -Configures Platform Power Limit 1 Enable, power and time window
    -Configures Platform Power Limit 2 Enable, power
    -Platform power limits are limited by the Package Max and Min power
**/
VOID
ConfigurePlatformPowerLimits (
  VOID
  )
{
  MSR_REGISTER PlatformPowerLimitMsr;
  UINT16       ConvertedPowerLimit1;
  UINT8        ConvertedPowerLimit1Time;
  UINT16       ConvertedPowerLimit2;
  UINT16       CpuConvertedPowerLimit1MaxLimit;
  UINT16       CpuConvertedPowerLimit2MaxLimit;
  UINT16       Multiplier;

  CpuConvertedPowerLimit1MaxLimit = 0;
  CpuConvertedPowerLimit2MaxLimit = 0;
  ConvertedPowerLimit1Time        = 0;

  if (gPowerMgmtConfig->PlatformPowerLimit1Power == AUTO) {
    return;
  }
  ///
  /// By default, for Mobile & Desktop Processors: Platform Power Limit 2 = 1.25 * Package TDP
  ///
  Multiplier = 125;
  ///
  ///  Check if Psys power limits are programmable
  ///   -SPI strap 300 bit 31
  ///
  PlatformPowerLimitMsr.Qword = AsmReadMsr64 (MSR_PLATFORM_POWER_LIMIT);
  ///
  /// Initialize the Power Limit 1 and Power Limit 1 enable bit
  ///  - Power Limit 1: Platform Power Limit MSR  [14:0]
  ///  - Power Limit 1 Enable: Platform Power Limit MSR  [15]
  ///
  ///
  /// By default, program Power Limit 1 to Package TDP limit
  ///
  ConvertedPowerLimit1 = mPackageTdp;
  ///
  /// gPowerMgmtConfig->PlatformPowerLimit1Power is in mW or watts. We need to
  /// convert it to CPU Power unit, specified in PACKAGE_POWER_SKU_UNIT_MSR[3:0].
  /// Since we are converting from Watts to CPU power units, multiply by
  /// PACKAGE_POWER_SKU_UNIT_MSR[3:0].
  ///
  ConvertedPowerLimit1 = (UINT16) ((gPowerMgmtConfig->PlatformPowerLimit1Power * mProcessorPowerUnit) / mCustomPowerUnit);
  if (mPackageMaxPower == 0 && ConvertedPowerLimit1 >= mPackageMinPower) {
    ///
    ///  If PACKAGE_POWER_SKU_MSR [46:32] = 0 means there is no upper limit ( since this field is 15 bits, the max value is 2^15 - 1 )
    ///
    CpuConvertedPowerLimit1MaxLimit = (UINT16) (LShiftU64 (2, 15) - 1);
    if (ConvertedPowerLimit1 > CpuConvertedPowerLimit1MaxLimit) {
      ///
      /// If new Power Limit 1 is > CpuConvertedPowerLimit1MaxLimit, program Power Limit 1 to CpuConvertedPowerLimit1MaxLimit
      ///
      ConvertedPowerLimit1 = CpuConvertedPowerLimit1MaxLimit;
    }
  } else if (mPackageMinPower == 0 && ConvertedPowerLimit1 > 0 && ConvertedPowerLimit1 <= mPackageMaxPower) {
    ///
    ///  If PACKAGE_POWER_SKU_MSR [30:16] = 0 means there is no lower limit
    ///
    ConvertedPowerLimit1 = (UINT16) ((gPowerMgmtConfig->PowerLimit1 * mProcessorPowerUnit) / mCustomPowerUnit);
  } else {
    ///
    /// Power Limit 1 needs to be between mPackageMinPower and mPackageMaxPower
    ///
    CpuConvertedPowerLimit1MaxLimit = mPackageMaxPower;
    if (ConvertedPowerLimit1 < mPackageMinPower) {
      ///
      /// If new Power Limit 1 is < mPackageMinPower, program Power Limit 1 to mPackageMinPower
      ///
      ConvertedPowerLimit1 = mPackageMinPower;
    } else if (ConvertedPowerLimit1 > CpuConvertedPowerLimit1MaxLimit) {
      ///
      /// If new Power Limit 1 is > mPackageMaxPower, program Power Limit 1 to mPackageMaxPower
      ///
      ConvertedPowerLimit1 = CpuConvertedPowerLimit1MaxLimit;
    }
  }
  PlatformPowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_MASK;
  PlatformPowerLimitMsr.Dwords.Low |= (UINT32) (ConvertedPowerLimit1);
  DEBUG (
          (DEBUG_INFO,
          "New Platform Power Limit 1  %d watt (%d in CPU power unit)\n",
           gPowerMgmtConfig->PlatformPowerLimit1Power,
           ConvertedPowerLimit1)
          );

  ///
  /// Update Platform Power Limit 1 enable bit
  ///
  if (gPowerMgmtConfig->PlatformPowerLimit1) {
    PlatformPowerLimitMsr.Dwords.Low |= B_POWER_LIMIT_ENABLE;
  } else {
    PlatformPowerLimitMsr.Dwords.Low &= (~B_POWER_LIMIT_ENABLE);
  }
  ///
  /// Program Platform Power Limit 1 Time Window
  ///  If PlatformPowerLimit1Time is AUTO OR If PlatformPowerLimit1Time
  ///  is > MAX_POWER_LIMIT_1_TIME_IN_SECONDS program default values
  ///
  if ((gPowerMgmtConfig->PlatformPowerLimit1Time == AUTO) ||
      (gPowerMgmtConfig->PlatformPowerLimit1Time > MAX_POWER_LIMIT_1_TIME_IN_SECONDS)
      ) {
    ///
    /// For Desktop, default value is 1 second
    ///
    gPowerMgmtConfig->PlatformPowerLimit1Time = DT_POWER_LIMIT1_TIME_DEFAULT;
  }
  ConvertedPowerLimit1Time = GetConvertedTime (gPowerMgmtConfig->PlatformPowerLimit1Time, PL1TimeWindowConvert);
  ///
  ///  Configure Platform Power Limit 1 time windows: Platform Power Limit MSR [23:17]
  ///
  PlatformPowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_1_TIME_MASK;
  PlatformPowerLimitMsr.Dwords.Low |= (UINT32) LShiftU64 (ConvertedPowerLimit1Time, 17);
  ///
  ///  Initialize Short Duration Power limit and enable bit
  ///    Platform Power Limit 2: Platform Power Limit MSR (0x450h) [46:32]
  ///    Platform Power Limit 2 Enable: Platform Power Limit MSR (0x450h) [47]
  ///
  /// gPowerMgmtConfig->PlatformPowerLimit2Power value is in mW or watts. We need to convert it to
  /// CPU Power unit, specified in PACKAGE_POWER_SKU_UNIT_MSR[3:0].Since we are converting
  /// from Watts to CPU power units, multiply by PACKAGE_POWER_SKU_UNIT_MSR[3:0]
  ///
  ConvertedPowerLimit2 = (UINT16) ((gPowerMgmtConfig->PlatformPowerLimit2Power * mProcessorPowerUnit) / mCustomPowerUnit);
  PlatformPowerLimitMsr.Dwords.High &= ~(POWER_LIMIT_MASK | B_POWER_LIMIT_ENABLE);
  ///
  /// If PlatformPowerLimit2 is AUTO OR if PlatformPowerLimit2 is > mPackageMaxPower
  /// OR if Platform PowerLimit2 < mPackageMinPower program defaul values.
  ///
  CpuConvertedPowerLimit2MaxLimit = mPackageMaxPower;
  if (CpuConvertedPowerLimit2MaxLimit == 0) {
    CpuConvertedPowerLimit2MaxLimit = (UINT16) (LShiftU64 (2, 15) - 1);
  }
  if (gPowerMgmtConfig->PlatformPowerLimit2Power == AUTO) {
    ConvertedPowerLimit2 = EFI_IDIV_ROUND ((Multiplier * mPackageTdp), 100);
  }
  if (ConvertedPowerLimit2 > CpuConvertedPowerLimit2MaxLimit) {
    ConvertedPowerLimit2 = CpuConvertedPowerLimit2MaxLimit;
  }
  if (ConvertedPowerLimit2 < mPackageMinPower) {
    ConvertedPowerLimit2 = mPackageMinPower;
  }
  PlatformPowerLimitMsr.Dwords.High |= (UINT32) (ConvertedPowerLimit2);

  if (gPowerMgmtConfig->PlatformPowerLimit2 == PPM_ENABLE) {
    PlatformPowerLimitMsr.Dwords.High |= B_POWER_LIMIT_ENABLE;
  } else {
    PlatformPowerLimitMsr.Dwords.High &= (~B_POWER_LIMIT_ENABLE);
  }

  DEBUG (
          (DEBUG_INFO,
           "Platform Power Limit 2 Power = %d Watts (%d in CPU power unit)\n",
           gPowerMgmtConfig->PlatformPowerLimit2Power,
           ConvertedPowerLimit2)
          );

    DEBUG ((DEBUG_INFO,"MSR(65Ch)=%08X%08X\n",PlatformPowerLimitMsr.Dwords.High,PlatformPowerLimitMsr.Dwords.Low));

  AsmWriteMsr64 (MSR_PLATFORM_POWER_LIMIT, PlatformPowerLimitMsr.Qword);
}

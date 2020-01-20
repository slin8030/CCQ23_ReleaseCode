/** @file
  This library contains power management configuration functions for processors.

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
#include <Library/SteppingLib.h>
//[-start-161022-IB07400803-add]//
#include <Library/MultiPlatformBaseLib.h>
//[-end-161022-IB07400803-add]//

/**
  This will perform general thermal initialization other then TM1, TM2, or
  PROCHOT# on all logical processors.

  @retval EFI_SUCCESS  General thermal initialization completed successfully
**/
VOID
InitThermal (
  VOID
  )
{
  MSR_REGISTER TempMsr;
  UINT8        MaxRefTemp;
  UINT8        TccActivationOffsetMask;
  UINTN        PciD0F0RegBase;
  UINTN        MchBar;
  PostCode (0xC6C);
  if (GetBxtSeries () == BxtP) {
    ///
    /// Run thermal code on all logical processors.
    ///
    RunOnAllLogicalProcessors (ApSafeInitThermal, gPowerMgmtConfig);
    ///
    /// Automatic Thermal Reporting for Thermal Management
    ///
//[-start-161027-IB07400806-modify]//
//    if (gPowerMgmtConfig->AutoThermalReporting) {
    if (TRUE) {
//[-end-161027-IB07400806-modify]//
      ///
      /// Thermal Reporting for Critical trip
      /// MSR 1A2 bits 23:16 define the  temperature that this specific processor can
      /// function upto. It is recommended that this value + 5  be used as default Critical trip point
      /// _CRT.
      ///
      TempMsr.Qword = AsmReadMsr64 (MSR_TEMPERATURE_TARGET);
      TempMsr.Qword &= B_MSR_TEMPERATURE_TARGET_TCC_ACTIVATION_TEMPERATURE_MASK;
      MaxRefTemp  = (UINT8) RShiftU64 (TempMsr.Qword, N_MSR_TEMPERATURE_TARGET_TCC_ACTIVATION_TEMPERATURE_OFFSET);
      gCpuGlobalNvsAreaConfig->Area->AutoCriticalTripPoint = MaxRefTemp + 5;
      ///
      /// Thermal Reporting for Active Thermal Management
      /// It is recommended that the processor specific value in MSR 1A2 bits 15:8
      /// be used as the highest Active trip point i.e. _AC0.
      ///
      TempMsr.Qword = AsmReadMsr64 (MSR_TEMPERATURE_TARGET);
      TempMsr.Qword &= B_MSR_TEMPERATURE_TARGET_FAN_TEMP_TARGET_OFFSET;
      gCpuGlobalNvsAreaConfig->Area->AutoActiveTripPoint = MaxRefTemp - (UINT8) RShiftU64 (
                      TempMsr.Qword,
                      N_MSR_TEMPERATURE_TARGET_FAN_TEMP_TARGET_OFFSET
                      );

      ///
      /// Tcc activation offset in temperature target MSR bits [30:24]
      ///
      TccActivationOffsetMask = 0x7F;

      ///
      /// Thermal Reporting for Passive Thermal Management
      /// On all turbo enabled systems, it is recommended that the ACPI _PSV point be
      /// set to a temperature above the Active cooling temperature and Tcc activation
      /// temperature.
      /// If platform embedded controller will issue PECI commands to reduce power as a
      /// passive thermal action, then it is recommended to use the package's max temperature
      /// for passive thermal control.
      ///
      TempMsr.Qword = AsmReadMsr64 (MSR_TEMPERATURE_TARGET);
      TempMsr.Qword &= (TccActivationOffsetMask << N_MSR_TEMPERATURE_TARGET_TCC_OFFSET_LIMIT);
      gCpuGlobalNvsAreaConfig->Area->AutoPassiveTripPoint = gCpuGlobalNvsAreaConfig->Area->AutoCriticalTripPoint+3;
    }

    ///
    /// Thermal Limit Control for run-time control of the package level virtual thermal sensor
    /// Set THERMAL_LIMIT_CONTROL.ENABLE(bit 8)
    /// THERMAL_LIMIT_CONTROL.THERMAL_LIMIT_TEMP(bit 7-bit0) is 105c
    ///
    PciD0F0RegBase  = MmPciBase (0, 0, 0);
    MchBar          = MmioRead32 (PciD0F0RegBase + 0x48) &~BIT0;
//[-start-161022-IB07400802-modify]//
//[-start-161022-IB07400803-modify]//
    if (IsIOTGBoardIds()) {
      MmioOr32 (MchBar + MMIO_PACKAGE_THERMAL_LIMIT_CONTROL, B_THERMAL_LIMIT_TEMP_ENABLE + V_THERMAL_LIMIT_TEMP_I);
    } else {
      MmioOr32 (MchBar + MMIO_PACKAGE_THERMAL_LIMIT_CONTROL, B_THERMAL_LIMIT_TEMP_ENABLE + V_THERMAL_LIMIT_TEMP_MD);
    }
//[-end-161022-IB07400803-modify]//
//[-end-161022-IB07400802-modify]//
    DEBUG ((DEBUG_INFO, "Thermal Limit Control \n"));

    ///
    /// Manage Processor Thermal Interrupts, including the management of filtering on the virtual thermal sensor control signal
    /// Set PKG_THERM_INTERRUPT.THRESHOLD1_INT_ENABLE(bit 0) and PKG_THERM_INTERRUPT.THRESHOLD2_INT_ENABLE(bit 1)
    /// PKG_THERM_INTERRUPT.THRESHOLD1_TEMP is 90c(bit 15:8) and PKG_THERM_INTERRUPT.THRESHOLD2_TEMP is 85c(bit 23:16)
    /// Time Window = 0.976ms * (float)((1+xx/4) * (2^yyyyy)); 01,01100b = 5s;
    ///
    PciD0F0RegBase  = MmPciBase (0, 0, 0);
    MchBar          = MmioRead32 (PciD0F0RegBase + 0x48) &~BIT0;
    MmioOr32 (MchBar + MMIO_PACKAGE_THERMAL_INTERRUPT, B_THRESHOLD1_INT_ENABLE + B_THRESHOLD2_INT_ENABLE + V_THRESHOLD1_TEMP + V_THRESHOLD2_TEMP + V_TIME_WINDOW);
    DEBUG ((DEBUG_INFO, "Manage Processor Thermal Interrupts \n"));
  }

  EnableProcHot ();
  return;
}

/**
  This will perform enable thermal initialization. TM1, TM2 and adaptive thermal
  throttling are enabled/disabled together.

  This function must be MP safe.

  @param[in] Buffer    Pointer to the function parameters passed in.

  @retval EFI_SUCCESS  General thermal initialization completed successfully
**/
VOID
EFIAPI
ApSafeInitThermal (
  IN OUT VOID *Buffer
  )
{
  MSR_REGISTER      TempMsr;

  ///
  /// Configure Adaptive thermal monitor. IA32_MISC_ENABLE[3]
  /// (1A0h)IA32_MISC_ENABLE - Bit3:Intel Adaptive Thermal Monitor Enable
  ///  System BIOS must always set this bit to be operating within spec.
  ///
  TempMsr.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  TempMsr.Dwords.Low |= B_MSR_IA32_MISC_ENABLE_TME;
  if (gPowerMgmtConfig->ThermalMonitor == 0) {
    TempMsr.Dwords.Low &= ~B_MSR_IA32_MISC_ENABLE_TME;
  }
  AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, TempMsr.Qword);
  ///
  /// Set the Lock TM interrupt bit so that thermal interrupts are routed to all the cores
  ///
  TempMsr.Qword = AsmReadMsr64 (MSR_MISC_PWR_MGMT);
  TempMsr.Qword |= B_MISC_PWR_MGMT_LOCK_TERM_INT;
  AsmWriteMsr64 (MSR_MISC_PWR_MGMT, TempMsr.Qword);

  ///
  /// Enable Critical Temperature Interrupt
  ///
  TempMsr.Qword = AsmReadMsr64 (IA32_THERM_INTERRUPT);
  TempMsr.Qword |= B_IA32_THERM_INTERRUPT_VIE;
  AsmWriteMsr64 (IA32_THERM_INTERRUPT, TempMsr.Qword);

  return;
}

/**
  Enables the bi-directional PROCHOT# signal.

  @retval EFI_SUCCESS  PROCHOT# configured successfully
**/
EFI_STATUS
EnableProcHot (
  VOID
  )
{
  MSR_REGISTER PowerCtl;

  ///
  /// Enable PROCHOT# in the CPU MSR if TM is enabled,
  ///  else disable it.
  ///
  PowerCtl.Qword = AsmReadMsr64 (MSR_POWER_CTL);
  if (gCpuGlobalNvsAreaConfig->Area->PpmFlags & (PPM_TM)) {
    PowerCtl.Qword &= ~B_MSR_POWER_CTL_DISABLE_VR_THERMAL_ALERT;
    ///
    /// Check PROCHOT Lock,skip programming the below as it will lock bits 0, 21, 22
    ///
    if (!(PowerCtl.Qword & B_MSR_POWER_CTL_PROCHOT_LOCK)) {
      PowerCtl.Qword &= ~B_MSR_POWER_CTL_BI_PROCHOT;
      if (gPowerMgmtConfig->BiProcHot) {
        PowerCtl.Qword |= B_MSR_POWER_CTL_BI_PROCHOT;
        ///
        /// Initialize PROCHOT# OUT basing on Bi-directional PROCHOT# setting
        /// If Bi-directional PROCHOT# is enabled, PROCHOT# OUT can be disabled selectively
        ///
        PowerCtl.Qword &= ~B_MSR_POWER_CTL_DISABLE_PROCHOT_OUT;
        if (gPowerMgmtConfig->DisableProcHotOut == 1) {
          DEBUG ((DEBUG_INFO, "PROCHOT# OUT is disabled\n"));
          PowerCtl.Qword |= B_MSR_POWER_CTL_DISABLE_PROCHOT_OUT;
        }
      }
      PowerCtl.Qword &= ~B_MSR_POWER_CTL_PROCHOT_RESPONSE;
      if (gPowerMgmtConfig->ProcHotResponse == 1) {
        DEBUG ((DEBUG_INFO, "PROCHOT# Response is enabled\n"));
        PowerCtl.Qword |= B_MSR_POWER_CTL_PROCHOT_RESPONSE;
      }
    }
    AsmWriteMsr64 (MSR_POWER_CTL, PowerCtl.Qword);
  }

  return EFI_SUCCESS;
}

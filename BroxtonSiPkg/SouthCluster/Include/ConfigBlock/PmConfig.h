/** @file
  Power Management policy

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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
#ifndef _PM_CONFIG_H_
#define _PM_CONFIG_H_

#define PM_CONFIG_REVISION 2
extern EFI_GUID gPmConfigGuid;

#pragma pack (push,1)

/**
  This structure allows to customize wake up capability from wake events.
**/
typedef struct {
  /**
    Corresponds to the PME_B0_S5_DIS bit in the General PM Configuration B (GEN_PMCON_B) register.
    When set to 1, this bit blocks wake events from PME_B0_STS in S5, regardless of the state of PME_B0_EN.
    When cleared (default), wake events from PME_B0_STS are allowed in S5 if PME_B0_EN = 1. <b>0: Disable</b>; 1: Enable.
  **/
  UINT32  PmeB0S5Dis         :  1;
  UINT32  Rsvdbits           : 31;
} SC_WAKE_CONFIG;

/**
  The SC_PM_CONFIG block describes expected miscellaneous power management settings.
  The PowerResetStatusClear field would clear the Power/Reset status bits, please
  set the bits if you want PCH Init driver to clear it, if you want to check the
  status later then clear the bits.
**/
typedef struct {
  /**
    Revision 1: Init version
    Revision 2: Add power button configurations: PwrBtnOverridePeriod, DisableNativePowerButton, PowerButterDebounceMode
  **/
  CONFIG_BLOCK_HEADER Header;       ///< Config Block Header
  SC_WAKE_CONFIG      WakeConfig;   ///< Specify Wake Policy
  /**
    This member describes whether or not the PCI ClockRun feature of SC should
    be enabled. <b>0: Disable</b>; 1: Enable
  **/
  UINT32              PciClockRun          :  1;
  /**
    Enable/Disable 8254 timer static clock gating. 0: Disable; <b>1: Enable</b>.
    When set, the 8254 timer is disabled statically. This policy is set by BIOS if the 8254 feature is not needed
    in the system or the system that supports C11.
    Normal operation of 8254 requires this policy to be Disabled.
    **/
  UINT32              Timer8254ClkGateEn   :  1;
  /**
    Power button override period.
    This field determines, while the power button remains asserted, how long the PMC will wait before initiating a global reset.
    000b-4s, 001b-6s, 010b-8s, 011b-10s, 100b-12s, 101b-14s
    <b>Default is 0: 4s</b>
  **/
  UINT32              PwrBtnOverridePeriod :  3;
  /**
    Power button native mode disable.
    While FALSE, the PMC's power button logic will act upon the input value from the GPIO unit, as normal.
    While TRUE, this will result in the PMC logic constantly seeing the power button as de-asserted.
    <b>Default is FALSE.</b>
  **/
  UINT32              DisableNativePowerButton : 1;
  /**
    Power button debounce Mode.
    This bit controls when interrupts (SMI#, SCI) are generated in response to assertion of the PWRBTN# pin.
    This bit's values cause the following behavior:
    0: The 16 ms debounce period applies to all usages of the PWRBTN# pin (legacy behavior).
    1: When a falling edge occurs on the PWRBTN# pin, an interrupt is generated and the 16 ms debounce timer starts. Subsequent interrupts are masked while the debounce timer is running.
    Note: Power button override logic always samples the post-debounce version of the pin.
    <b>Default is 0</b>
  **/
  UINT32              PowerButterDebounceMode  : 1;
  UINT32              Rsvdbits                 :25;
} SC_PM_CONFIG;

#pragma pack (pop)

#endif // _PM_CONFIG_H_

/** @file
  CPU policy PPI produced by a platform driver specifying various
  expected CPU settings. This PPI is consumed by CPU PEI modules.

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

@par Specification
**/
#ifndef _CPU_OVERCLOCKING_CONFIG_H_
#define _CPU_OVERCLOCKING_CONFIG_H_


//
// Extern the GUID for protocol users.
//
extern EFI_GUID gCpuOverclockingConfigGuid;

#pragma pack(push, 1)

#define CPU_OVERCLOCKING_CONFIG_REVISION 1

/**
  Overclocking Configuration controls which use the CPU overclocking mailbox interface are defined in this structure.
  Platform code can pass in data to the mailbox through this structure.
**/
typedef struct {
  CONFIG_BLOCK_HEADER Header;          ///< Offset 0 GUID number for main entry of config block
  UINT32 CoreVoltageMode      :  1;     ///< Core voltage mode; <b>0: Adaptive</b>; 1: Override.
  UINT32 RingVoltageMode      :  1;     ///< CLR voltage mode; <b>0: Adaptive</b>; 1:Override
  UINT32 OcSupport            :  1;     ///< Over clocking support; <b>0: Disable</b>; 1: Enable.
  UINT32 RsvdBits             : 29;     ///< Bits reserved for DWORD alignment.
  INT16  CoreVoltageOffset;             ///< The voltage offset applied to the core while operating in turbo mode.
  UINT16 CoreVoltageOverride;           ///< The core voltage override which is applied to the entire range of cpu core frequencies.
  UINT16 CoreExtraTurboVoltage;         ///< Extra Turbo voltage applied to the cpu core when the cpu is operating in turbo mode.
  UINT16 CoreMaxOcTurboRatio;           ///< Maximum core turbo ratio override allows to increase CPU core frequency beyond the fused max turbo ratio limit.
  INT16  RingVoltageOffset;             ///< The voltage offset applied to CLR while operating in turbo mode.
  UINT16 RingVoltageOverride;           ///< The clr voltage override which is applied to the entire range of cpu frequencies.
  UINT16 RingExtraTurboVoltage;         ///< Extra Turbo voltage applied to clr.
  UINT16 RingMaxOcTurboRatio;           ///< Maximum clr turbo ratio override allows to increase CPU clr frequency beyond the fused max turbo ratio limit.
} CPU_OVERCLOCKING_CONFIG;

#pragma pack(pop)
#endif

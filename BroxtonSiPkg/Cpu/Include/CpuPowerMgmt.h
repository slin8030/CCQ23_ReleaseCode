/** @file
  This file contains define definitions specific to processor

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

@par Specification Reference:
**/
#ifndef _POWER_MGMT_DEFINITIONS_H_
#define _POWER_MGMT_DEFINITIONS_H_

#define PPM_ENABLE  1
#define PPM_DISABLE 0

#define POWER_MGMT_VR_DISABLE     0
#define CSTATE_SUPPORTED          0x1
#define ENHANCED_CSTATE_SUPPORTED 0x2
#define C6_C7_SHORT_LATENCY_SUPPORTED 0x01
#define C6_C7_LONG_LATENCY_SUPPORTED  0x02
#define C7s_SHORT_LATENCY_SUPPORTED   0x03
#define C7s_LONG_LATENCY_SUPPORTED    0x04
//
// Voltage offset definitions
//
#define OC_LIB_OFFSET_ADAPTIVE  0
#define OC_LIB_OFFSET_OVERRIDE  1
//
// Platform Power Management Flags Bit Definitions:
//   These defines are also used in CPU0CST.ASL to check platform configuration
//   and build C-state table accordingly.
//
#define PPM_EIST                0x1        ///< BIT 0 : Enhanced Intel Speed Step Technology.
#define PPM_C1                  0x2        ///< BIT 1 : C1 enabled, supported.
#define PPM_C1E                 0x4        ///< BIT 2 : C1E enabled.
#define PPM_C3                  0x8        ///< BIT 3 : C3 enabled, supported.
#define PPM_C6                  0x10       ///< BIT 4 : C6 enabled, supported.
#define PPM_C7                  0x20       ///< BIT 5 : C7 enabled, supported.
#define PPM_C7S                 0x40       ///< BIT 6 : C7S enabled, supported
#define PPM_TM                  0x80       ///< BIT 7 : Adaptive Thermal Monitor.
#define PPM_TURBO               0x100      ///< BIT 8 : Long duration turbo mode
#define PPM_CMP                 0x200      ///< BIT 9 : CMP.
#define PPM_TSTATES             0x400      ///< BIT 10: CPU throttling states
#define PPM_MWAIT_EXT           0x800      ///< BIT 11: MONITIOR/MWAIT Extensions supported.
#define PPM_EEPST               0x1000     ///< BIT 12: Energy efficient P-State Feature enabled
#define PPM_TSTATE_FINE_GRAINED 0x2000     ///< BIT 13: Fine grained CPU Throttling states
#define PPM_CD                  0x4000     ///< BIT 14: Deep Cstate - C8/C9/C10
#define PPM_TIMED_MWAIT         0x8000     ///< BIT 15: Timed Mwait support
#define C6_LONG_LATENCY_ENABLE    0x10000  ///< BIT 16: 1=C6 Long and Short,0=C6 Short only
#define C7_LONG_LATENCY_ENABLE    0x20000  ///< BIT 17: 1=C7 Long and Short,0=C7 Short only
#define C7s_LONG_LATENCY_ENABLE   0x40000  ///< BIT 18: 1=C7s Long and Short,0=C7s Short only
#define PPM_C8                    0x80000  ///< Bit 19: 1= C8 enabled/supported
#define PPM_C9                    0x100000 ///< Bit 20: 1= C9 enabled/supported
#define PPM_C10                   0x200000 ///< Bit 21: 1= C10 enabled/supported
#define PPM_HWP                   0x400000 ///< Bit 22: 1= HWP enabled/supported
#define PPM_HWP_LVT               0x800000 ///< Bit 23: 1= HWP LVT enabled/supported

#define PPM_C_STATES            0x7A    ///< PPM_C1 + PPM_C3 + PPM_C6 + PPM_C7 + PPM_C7S
#define C3_LATENCY              0x42
#define C6_C7_SHORT_LATENCY     0x73
#define C6_C7_LONG_LATENCY      0x91
#define C8_LATENCY              0xE4
#define C9_LATENCY              0x145
#define C10_LATENCY             0x1EF

#define CPUID_FUNCTION_6        0x00000006
//
// The following definitions are based on assumed location for the  ACPI
// Base Address.  Modify as necessary base on platform-specific requirements.
//
#define PCH_ACPI_PBLK 0x1810
#define PCH_ACPI_LV2  0x1814
#define PCH_ACPI_LV3  0x1815
#define PCH_ACPI_LV4  0x1816
#define PCH_ACPI_LV6  0x1818
#define PCH_ACPI_LV5  0x1817
#define PCH_ACPI_LV7  0x1819

//
// C-State Latency (us) and Power (mW) for C1
//
#define C1_LATENCY                        1
#define C1_POWER                          0x3E8
#define C3_POWER                          0x1F4
#define C6_POWER                          0x15E
#define C7_POWER                          0xC8
#define C8_POWER                          0xC8
#define C9_POWER                          0xC8
#define C10_POWER                         0xC8

#define MAX_POWER_LIMIT_1_TIME_IN_SECONDS 32767
#define AUTO                              0
#define END_OF_TABLE                      0xFF

#define CONFIG_TDP_DOWN                   1
#define CONFIG_TDP_UP                     2
#define CONFIG_TDP_DEACTIVATE             0xFF
//
// MMIO definitions
//
#define MMIO_DDR_RAPL_LIMIT                 0x58e0
#define MMIO_TURBO_POWER_LIMIT              0x59A0
#define MMIO_PACKAGE_THERMAL_LIMIT_CONTROL  0x7104
#define B_THERMAL_LIMIT_TEMP_ENABLE          BIT8
//[-start-161022-IB07400802-modify]//
//#if BXTI_PF_ENABLE
#define V_THERMAL_LIMIT_TEMP_I                0x6E              //APL-I sku Tjmax spec is 110C
//#else
#define V_THERMAL_LIMIT_TEMP_MD               0x69
//#endif
//[-end-161022-IB07400802-modify]//

#define MMIO_PACKAGE_THERMAL_INTERRUPT      0x7010
#define B_THRESHOLD1_INT_ENABLE              BIT0
#define B_THRESHOLD2_INT_ENABLE              BIT1
#define V_THRESHOLD1_TEMP                    0x5A00
#define V_THRESHOLD2_TEMP                    0x550000
#define V_TIME_WINDOW                        0x2C000000

#define MMIO_PL3_CONTROL                    0x71F0
#define V_PL3_POWER_LIMIT                    0x1E00            // 30W for all SKU's
#define B_PL3_ENABLE                         BIT15
#define V_PL3_TIME_WINDOW                    0x500000
#define V_PL3_DUTY_CYCLE                     0x0A000000
#define V_PL4_PMAX                           0x1E0000000000    // 30W for all SKU's
#define B_PL4_ENABLE                         BIT47

#define MMIO_RAPL_LIMIT                      0x70A8

///
/// For Mobile, default PL1 time window value is 28 seconds
///
#define MB_POWER_LIMIT1_TIME_DEFAULT  28
///
/// For Desktop, default PL1 time window value is 8 second
///
#define DT_POWER_LIMIT1_TIME_DEFAULT  8
///
/// Mobile Sku in Watts
///
#define MOBILE_SKU_6W                 0x600
#define MOBILE_SKU_4W                 0x400
#define DESKTOP_SKU_10                0xA00

///
/// PL1, PL2 limits
///
#define DT_POWER_LIMIT1               10

#define MB_POWER_LIMIT2               15
#define DT_POWER_LIMIT2               25

#define PROCESSOR_FLAVOR_MOBILE       0x04
#define PROCESSOR_FLAVOR_DESKTOP      0x00
#define PROCESSOR_FLAVOR_MASK         (BIT3 | BIT2)

//
// Power definitions (Based on EMTS V1.0 for standard voltage 2.4-2.6 GHz dual-core parts.)
//
#define FVID_MAX_POWER      35000
#define FVID_TURBO_POWER    35000
#define FVID_SUPERLFM_POWER 12000
//
// Power definitions for LFM and Turbo mode TBD.
//
#define FVID_MIN_POWER  15000
///
/// S3- MSR restore SW SMI
///
#ifndef SW_SMI_S3_RESTORE_MSR
//[-start-160330-IB07220057-modify]//
// #define SW_SMI_S3_RESTORE_MSR 0x48 // conflict with kernel
#define SW_SMI_S3_RESTORE_MSR 0x62
//[-end-160330-IB07220057-modify]//
#endif

///
/// Limit the number of P-states to 16. Up to Windows 7, the OS allocates 1KB buffer for the PSS package.
/// So the maximum number of P-state OS can handle is 19. This is not an OS issue. Having too many P-states
/// is not good for the system performance.
///
#define FVID_MAX_STATES                 20

///
/// VR Commands
///
#define WRITE_VR_CURRENT_CONFIG_CMD     0x80000129

#endif

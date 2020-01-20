/** @file
  BXT Ax HSIO C File

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

@par Specification Reference:
**/

#include <Uefi/UefiBaseType.h>
#include <ScAccess.h>
#include <Private/ScHsio.h>

//
// SSIC
//
//
// NOTE:
//  SSIC_PORT_READ_OPCODE  = 0x06 -> Private Config Register Read
//  SSIC_PORT_WRITE_OPCODE = 0x07 -> Private Config Register Write
//  These are the default opcodes used by SideBandLib.
//
SC_SIDEBAND_TABLE_STRUCT ScSsicHsioTable_RateA[] = {
  ///
  ///clane.cmnreg1.hfpll_enable = 0; CFG_UPDATE()
  ///
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000000},
  CFG_UPDATE,
  ///
  ///clane.cmnreg1.lfpll_enable = 0; CFG_UPDATE()
  ///
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000000},
  CFG_UPDATE,
  ///
  ///CFG_UPDATE(); clane.cmnreg9.pllrefsel = 0x0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG9, 0xfeffffff, 0x00000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg21.hfpll_refcal = 0x0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfffff8ff, 0x00000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg2.hfpll_divisors_msb =0x1
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffff00ff, 0x00000100},
  ///
  ///CFG_UPDATE(); clane.cmnreg2.hfpll_divisors_lsb =0x04
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffffff00, 0x00000004 },
  ///
  ///CFG_UPDATE(); clane.cmnreg1.lfpll_ndiv = 0x17
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0x00ffffff, 0x17000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg1.maxpwmgearsel = 0x0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0xfff8ffff, 0x00000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg21.hfpll_hsrate = 0x1   # Rate A
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfeffffff, 0x01000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg5.bitwidth = 0x2   # 20-bit width
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG5, 0xfffffffc, 0x00000002},
  ///
  ///CFG_UPDATE(); clane.cmnreg7.maxhsgearsel = 0x0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG7, 0xfffffcff, 0x00000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg18.hfpll_cfg3 = 0x1
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG18, 0xe0ffffff, 0x01000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg17.hfpll_cfg2_byte3 = 0x45
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0x00ffffff, 0x45000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg17.hfpll_cfg2_byte2 = 0x1
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xff00ffff, 0x00010000},
  ///
  ///CFG_UPDATE(); clane.cmnreg17.hfpll_cfg2_byte1 = 0xac
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffff00ff, 0x0000ac00},
  ///
  ///CFG_UPDATE(); clane.cmnreg17.hfpll_cfg2_byte0 = 0xd
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffffff00, 0x0000000d},
  ///
  ///CFG_UPDATE(); clane.cmnreg6.hfpll_cfg1_byte3 = 0xc1
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0x00ffffff, 0xc1000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg6.hfpll_cfg1_byte2 = 0x0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xff00ffff, 0x00000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg6.hfpll_cfg1_byte1 = 0x3
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffff00ff, 0x00000300},
  ///
  ///CFG_UPDATE(); clane.cmnreg6.hfpll_cfg1_byte0 = 0x16
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffffff00, 0x00000016},
  ///
  ///CFG_UPDATE(); clane.cmnreg3.hfpll_afcen = 0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG3, 0xfffffffe, 0x00000000},
  ///
  /// Enable LF-PLL and HF-PLL
  ///
  ///clane.cmnreg1.lfpll_enable = 1; CFG_UPDATE()
  ///
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000001},
  CFG_UPDATE,
  ///
  ///clane.cmnreg1.hfpll_enable = 1; CFG_UPDATE()
  ///
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000100},
  CFG_UPDATE,

  ///Enable Clock Gating
  ///clane.cmnreg4.[5:0]= b100011; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG4, 0xffffffc0, 0x0023 },
  CFG_UPDATE,
  /// Enable CLANE Power Gating feature per MPHY Integration Guide
  ///  clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffffa, 0x00000003},
  CFG_UPDATE,
  /// Enable Slow Squelch
  ///  clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffeff, 0x00000100},
  CFG_UPDATE,
  ///
  /// if not init_phy == 0:
  /// if not port1 == 0:
  /// def CFG_UPDATE():
  /// dlane0.impreg22.cnfgupd = 0x4
  /// CFG_UPDATE(); dlane0.impreg21.rx_squelch_enable_timer = 0x20
  ///
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG21, 0xffffff00, 0x00000020},
  ///
  /// Minden specific settings
  /// CFG_UPDATE(); dlane0.impreg25.tx_tob_extn_override_en = 1
  ///
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffffeff, 0x00000100},
  ///
  /// CFG_UPDATE(); dlane0.impreg25.tx_pwm_tob_extension = 0x20
  ///
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xffffff00, 0x00000020},
  ///
  /// CFG_UPDATE(); dlane0.impreg25.force_tx_lcc_disable = 1
  ///
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffeffff, 0x00010000},
  ///Enable Dlane Power gating
  ///CFG_UPDATE(); dlane2.impreg8.[25:24] = b'11
//  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
//  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },

  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
};
SC_SIDEBAND_TABLE_STRUCT ScSsicHsioTable_RateA_Dlane_PG[] = {
  ///
  ///clane.cmnreg1.hfpll_enable = 0; CFG_UPDATE()
  ///
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000000},
  CFG_UPDATE,
  ///
  ///clane.cmnreg1.lfpll_enable = 0; CFG_UPDATE()
  ///
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000000},
  CFG_UPDATE,
  ///
  ///CFG_UPDATE(); clane.cmnreg9.pllrefsel = 0x0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG9, 0xfeffffff, 0x00000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg21.hfpll_refcal = 0x0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfffff8ff, 0x00000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg2.hfpll_divisors_msb =0x1
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffff00ff, 0x00000100},
  ///
  ///CFG_UPDATE(); clane.cmnreg2.hfpll_divisors_lsb =0x04
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffffff00, 0x00000004 },
  ///
  ///CFG_UPDATE(); clane.cmnreg1.lfpll_ndiv = 0x17
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0x00ffffff, 0x17000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg1.maxpwmgearsel = 0x0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0xfff8ffff, 0x00000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg21.hfpll_hsrate = 0x1   # Rate A
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfeffffff, 0x01000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg5.bitwidth = 0x2   # 20-bit width
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG5, 0xfffffffc, 0x00000002},
  ///
  ///CFG_UPDATE(); clane.cmnreg7.maxhsgearsel = 0x0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG7, 0xfffffcff, 0x00000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg18.hfpll_cfg3 = 0x1
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG18, 0xe0ffffff, 0x01000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg17.hfpll_cfg2_byte3 = 0x45
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0x00ffffff, 0x45000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg17.hfpll_cfg2_byte2 = 0x1
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xff00ffff, 0x00010000},
  ///
  ///CFG_UPDATE(); clane.cmnreg17.hfpll_cfg2_byte1 = 0xac
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffff00ff, 0x0000ac00},
  ///
  ///CFG_UPDATE(); clane.cmnreg17.hfpll_cfg2_byte0 = 0xd
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffffff00, 0x0000000d},
  ///
  ///CFG_UPDATE(); clane.cmnreg6.hfpll_cfg1_byte3 = 0xc1
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0x00ffffff, 0xc1000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg6.hfpll_cfg1_byte2 = 0x0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xff00ffff, 0x00000000},
  ///
  ///CFG_UPDATE(); clane.cmnreg6.hfpll_cfg1_byte1 = 0x3
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffff00ff, 0x00000300},
  ///
  ///CFG_UPDATE(); clane.cmnreg6.hfpll_cfg1_byte0 = 0x16
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffffff00, 0x00000016},
  ///
  ///CFG_UPDATE(); clane.cmnreg3.hfpll_afcen = 0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG3, 0xfffffffe, 0x00000000},
  ///
  /// Enable LF-PLL and HF-PLL
  ///
  ///clane.cmnreg1.lfpll_enable = 1; CFG_UPDATE()
  ///
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000001},
  CFG_UPDATE,
  ///
  ///clane.cmnreg1.hfpll_enable = 1; CFG_UPDATE()
  ///
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000100},
  CFG_UPDATE,

  ///Enable Clock Gating
  ///clane.cmnreg4.[5:0]= b100011; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG4, 0xffffffc0, 0x0023 },
  CFG_UPDATE,
  /// Enable CLANE Power Gating feature per MPHY Integration Guide
  ///  clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffffa, 0x00000003},
  CFG_UPDATE,
  /// Enable Slow Squelch
  ///  clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffeff, 0x00000100},
  CFG_UPDATE,
  ///
  /// if not init_phy == 0:
  /// if not port1 == 0:
  /// def CFG_UPDATE():
  /// dlane0.impreg22.cnfgupd = 0x4
  /// CFG_UPDATE(); dlane0.impreg21.rx_squelch_enable_timer = 0x20
  ///
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG21, 0xffffff00, 0x00000020},
  ///
  /// Minden specific settings
  /// CFG_UPDATE(); dlane0.impreg25.tx_tob_extn_override_en = 1
  ///
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffffeff, 0x00000100},
  ///
  /// CFG_UPDATE(); dlane0.impreg25.tx_pwm_tob_extension = 0x20
  ///
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xffffff00, 0x00000020},
  ///
  /// CFG_UPDATE(); dlane0.impreg25.force_tx_lcc_disable = 1
  ///
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffeffff, 0x00010000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  ///Enable Dlane Power gating
  ///CFG_UPDATE(); impreg8.[25:24] = b'11

  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_1_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },
  { R_SC_PCR_MMP0_LANE_1_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
};
SC_SIDEBAND_TABLE_STRUCT ScSsicHsioTable_RateB[] = {
  ///clane.cmnreg1.hfpll_enable = 0; cfg_updt()
 { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000000 },
 CFG_UPDATE,

  ///clane.cmnreg1.lfpll_enable = 0; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000000 },
  CFG_UPDATE,

  ///cfg_updt(); clane.cmnreg9.pllrefsel =         0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG9, 0xfeffffff, 0x00000000 },

  ///cfg_updt(); clane.cmnreg21.hfpll_refcal =     0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfffff8ff, 0x00000000 },

  ///cfg_updt(); clane.cmnreg2.hfpll_divisors_msb =0x1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffff00ff, 0x00000100 },

  ///cfg_updt(); clane.cmnreg2.hfpll_divisors_lsb =0x30
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffffff00, 0x00000030 },

  ///cfg_updt(); clane.cmnreg1.lfpll_ndiv =        0x1B # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0x00ffffff, 0x1B000000 },

  ///cfg_updt(); clane.cmnreg1.maxpwmgearsel =     0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0xfff8ffff, 0x00000000 },

  ///cfg_updt(); clane.cmnreg21.hfpll_hsrate =     0x0   # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfeffffff, 0x00000000 },

  ///cfg_updt(); clane.cmnreg5.bitwidth =          0x2   # 20-bit width
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG5, 0xfffffffc, 0x00000002 },

  ///cfg_updt(); clane.cmnreg7.maxhsgearsel =      0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG7, 0xfffffcff, 0x00000000 },

  ///cfg_updt(); clane.cmnreg18.hfpll_cfg3 =       0x1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG18, 0xe0ffffff, 0x01000000 },

  ///cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte3 = 0x7C # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0x00ffffff, 0x7C000000 },

  ///cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte2 = 0x1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xff00ffff, 0x00010000 },

  ///cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte1 = 0xac
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffff00ff, 0x0000ac00 },

  ///cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte0 = 0xd
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffffff00, 0x0000000d },

  ///cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte3 =  0xc1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0x00ffffff, 0xc1000000 },

  ///cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte2 =  0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xff00ffff, 0x00000000 },

  ///cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte1 =  0x4 # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffff00ff, 0x00000400 },

  ///cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte0 =  0x16
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffffff00, 0x00000016 },
  ///
  ///CFG_UPDATE(); clane.cmnreg3.hfpll_afcen = 0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG3, 0xfffffffe, 0x00000000},

   ///
   /// Enable LF-PLL and HF-PLL
   ///
   ///clane.cmnreg1.lfpll_enable = 1; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000001 },
  CFG_UPDATE,

  ///clane.cmnreg1.hfpll_enable = 1; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000100 },
  CFG_UPDATE,

  ///Enable Clock Gating
  ///clane.cmnreg4.[5:0]= b100011; cfg_updt()
  {R_SC_PCR_MMP0_CMNREG4, 0xffffffc0, 0x0023 },
  CFG_UPDATE,

  /// Enable CLANE Power Gating feature per MPHY Integration Guide
  ///  clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffffa, 0x00000003},
  CFG_UPDATE,
  /// Enable Slow Squelch
  ///  clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffeff, 0x00000100},
  CFG_UPDATE,
  ///if not init_phy == 0:
  ///if not port1 == 0:
  ///def cfg_updt():
  ///dlane0.impreg22.cnfgupd = 0x4
  ///cfg_updt(); dlane0.impreg21.rx_squelch_enable_timer = 0x20

  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG21, 0xffffff00, 0x00000020 },

  /// Minden specific settings
  //cfg_updt(); dlane0.impreg25.tx_tob_extn_override_en = 1
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffffeff, 0x00000100 },

  ///cfg_updt(); dlane0.impreg25.tx_pwm_tob_extension = 0x20
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xffffff00, 0x00000020 },

  ///cfg_updt(); dlane0.impreg25.force_tx_lcc_disable = 1
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffeffff, 0x00010000 },

  ///Enable Dlane Power gating
  ///CFG_UPDATE(); dlane2.impreg8.[25:24] = b'11
//  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
//  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },
  ///cfg_updt();
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },

};
///
///  The following settings are temperay for enable/disable dlane power gating
///
SC_SIDEBAND_TABLE_STRUCT ScSsicHsioTable_RateB_Dlane_PG[] = {
  ///Enable Dlane Power gating
  ///CFG_UPDATE(); dlane2.impreg8.[25:24] = b'11
  ///clane.cmnreg1.hfpll_enable = 0; cfg_updt()
 { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000000 },
 CFG_UPDATE,

  ///clane.cmnreg1.lfpll_enable = 0; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000000 },
  CFG_UPDATE,

  ///cfg_updt(); clane.cmnreg9.pllrefsel =         0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG9, 0xfeffffff, 0x00000000 },

  ///cfg_updt(); clane.cmnreg21.hfpll_refcal =     0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfffff8ff, 0x00000000 },

  ///cfg_updt(); clane.cmnreg2.hfpll_divisors_msb =0x1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffff00ff, 0x00000100 },

  ///cfg_updt(); clane.cmnreg2.hfpll_divisors_lsb =0x30
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffffff00, 0x00000030 },

  ///cfg_updt(); clane.cmnreg1.lfpll_ndiv =        0x1B # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0x00ffffff, 0x1B000000 },

  ///cfg_updt(); clane.cmnreg1.maxpwmgearsel =     0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0xfff8ffff, 0x00000000 },

  ///cfg_updt(); clane.cmnreg21.hfpll_hsrate =     0x0   # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfeffffff, 0x00000000 },

  ///cfg_updt(); clane.cmnreg5.bitwidth =          0x2   # 20-bit width
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG5, 0xfffffffc, 0x00000002 },

  ///cfg_updt(); clane.cmnreg7.maxhsgearsel =      0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG7, 0xfffffcff, 0x00000000 },

  ///cfg_updt(); clane.cmnreg18.hfpll_cfg3 =       0x1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG18, 0xe0ffffff, 0x01000000 },

  ///cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte3 = 0x7C # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0x00ffffff, 0x7C000000 },

  ///cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte2 = 0x1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xff00ffff, 0x00010000 },

  ///cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte1 = 0xac
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffff00ff, 0x0000ac00 },

  ///cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte0 = 0xd
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffffff00, 0x0000000d },

  ///cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte3 =  0xc1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0x00ffffff, 0xc1000000 },

  ///cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte2 =  0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xff00ffff, 0x00000000 },

  ///cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte1 =  0x4 # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffff00ff, 0x00000400 },

  ///cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte0 =  0x16
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffffff00, 0x00000016 },
  ///
  ///CFG_UPDATE(); clane.cmnreg3.hfpll_afcen = 0
  ///
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG3, 0xfffffffe, 0x00000000},

   ///
   /// Enable LF-PLL and HF-PLL
   ///
   ///clane.cmnreg1.lfpll_enable = 1; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000001 },
  CFG_UPDATE,

  ///clane.cmnreg1.hfpll_enable = 1; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000100 },
  CFG_UPDATE,

  ///Enable Clock Gating
  ///clane.cmnreg4.[5:0]= b100011; cfg_updt()
  {R_SC_PCR_MMP0_CMNREG4, 0xffffffc0, 0x0023 },
  CFG_UPDATE,

  /// Enable CLANE Power Gating feature per MPHY Integration Guide
  ///  clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffffa, 0x00000003},
  CFG_UPDATE,
  /// Enable Slow Squelch
  ///  clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffeff, 0x00000100},
  CFG_UPDATE,
  ///if not init_phy == 0:
  ///if not port1 == 0:
  ///def cfg_updt():
  ///dlane0.impreg22.cnfgupd = 0x4
  ///cfg_updt(); dlane0.impreg21.rx_squelch_enable_timer = 0x20

  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG21, 0xffffff00, 0x00000020 },

  /// Minden specific settings
  //cfg_updt(); dlane0.impreg25.tx_tob_extn_override_en = 1
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffffeff, 0x00000100 },

  ///cfg_updt(); dlane0.impreg25.tx_pwm_tob_extension = 0x20
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xffffff00, 0x00000020 },

  ///cfg_updt(); dlane0.impreg25.force_tx_lcc_disable = 1
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffeffff, 0x00010000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  ///Enable Dlane Power gating
  ///CFG_UPDATE(); impreg8.[25:24] = b'11

  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_1_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },
  { R_SC_PCR_MMP0_LANE_1_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
};



SC_SIDEBAND_TABLE_STRUCT ScBxtUsb2PhyCommonTable_A0[] = {
  {R_PCR_USB2_GLOBAL_PORT,      0xFCFFCFF7, 0x03003000},
  {R_PCR_USB2_GLB_ADP_VBUS_REG, 0xFFBFFFFF, 0x00400000},
  {R_PCR_USB2_GLOBAL_PORT_2,    0xF7BFFC7F, 0x00400380},
  {R_PCR_USB2_CFG_COMPBG,       0xFFFF007F, 0x00008480},
  {R_PCR_USB2_CONFIG_3,         0x00000000, 0x59E94D0C},
  {R_PCR_USB2_DFT_1,            0xFFFF0FFF, 0x0000F000},
  {R_PCR_USB2_SFRCONFIG_0,      0xFFFFFCFF, 0x00000200}
};

SC_SIDEBAND_TABLE_STRUCT ScBxtUsb2PhyPerPortTable_A0[] = {
  {R_PCR_USB2_PPORT,   0x03F180EF, 0xFC042810},
  {R_PCR_UTMI_MISC_PR, 0xFFFFC17F, 0x00001A80},
  {R_PCR_USB2_PPORT2,  0xFC7FFFFC, 0x03800003}
};

SC_SIDEBAND_TABLE_STRUCT ScUsb3HsioTable[] = {
  ///-------------------------------------kg_registers_A0_RC04.xlsm-------------------------------------
  ///
  /// Begin: PCS Section
  /// soc.usb.modphy.clane0.dig.cmn_dig_dword8.o_pll1_cfg_fastlock_disable_h=1;0x8020 [23:16]=1
  ///
  {0x8020, 0xdfffffff, 0x0001 << 29},
  ///
  /// cri_rxeb_hiwater_usb3[4:0]
  ///
  {0x2024, 0xffffffe0, 0x000f},
  {0x2224, 0xffffffe0, 0x000f},
  ///
  /// cri_rxeb_lowater_usb3[4:0]
  ///
  {0x2024, 0xffffe0ff, 0x000c << 8},
  {0x2224, 0xffffe0ff, 0x000c << 8},
  ///
  /// cri_rxeb_ptr_init_usb3[4:0]
  ///
  {0x2024, 0xffe0ffff, 0x000c << 16},
  {0x2224, 0xffe0ffff, 0x000c << 16},
  ///
  ///reg_cfg_data_dynclkgate_mode[1:0]
  ///
  {0x2028, 0xffffff9f, 0x0003 << 5},
  {0x2228, 0xffffff9f, 0x0003 << 5},
  /// PCS_DWORD10  [13]=1b'0  0: LFPS Raw mode disabled for Port0
  { 0x2028, 0xffffdfff, 0x0 },
  /// PCS_DWORD10  [13]=1b'1  1 (default): LFPS Raw mode enabled in low power states.
  { 0x2228, 0xffffdfff, 0x0001 << 13 },
  ///
  /// cri_alignmode [default]
  /// cri_kalign_lock_cnt[3:0]
  ///
  {0x2020, 0xfffffff0, 0x000d},
  {0x2220, 0xfffffff0, 0x000d},
  ///
  /// cri_kalign_unlock_relock_cnt[2:0] [default]
  /// cri_kalignmode [default]
  /// cri_kalignmode_ovren [default]
  /// reg_rcomp_rxvgarc_scale=0xb0
  ///{0x2030, 0xffff00ff, 0xb0 << 8},
  ///{0x2230, 0xffff00ff, 0xb0 << 8},
  ///
  /// reg_lfps_rawmodeen=0
  ///
  {0x2028, 0xffffdfff, 0x0},
  {0x2228, 0xffffdfff, 0x0},
  ///
  /// reg_ldo_cw_cfg_sus[7:2]
  ///
  {0x203C, 0xff03ffff, 0x001f << 18},
  {0x223C, 0xff03ffff, 0x001f << 18},
  ///
  /// End: PCS Section
  ///
  /// Begin: CMN_DIG Section
  /// soc.usb.modphy.clane0.dig.cmn_dig_dword13.o_ldo_sw_cfg_sus_7_0=0x40 ;8034 [7:0]=40
  ///
  {0x8034, 0xffffff00, 0x40},
  ///
  /// End: CMN_DIG section
  ///
  /// Begin: TX section
  /// ow2tapdeemph6p0 [default]
  /// o_crireg_frcrcvdtcted [default]
  /// ow2tapgen2deemph3p5 [default]
  ///
  {0x2090, 0xffff00ff, 0x3a << 8},
  {0x2290, 0xffff00ff, 0x3a << 8},
  ///
  /// soc.usb.modphy.dlane1.tx.tx_dword11.o_crireg_rcvdetect_first_en_sus=1; 0x22AC[7] = 1
  ///
  {0x20AC, 0xffffff7f, 0x01 << 7},
  {0x22AC, 0xffffff7f, 0x01 << 7},
  ///
  /// End: TX section
  ///
  /// Begin: RX Section
  /// soc.usb.modphy.dlane1.rx.rx_dword16.icfgpfcfg_fullrate_7_0=0x10
  ///
  {0x2140, 0xffffff00, 0x0010},
  {0x2340, 0xffffff00, 0x0010},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword16.icfgpfcfg_fullrate_15_8=0xc5
  ///
  {0x2140, 0xffff00ff, 0x00c5 << 8},
  {0x2340, 0xffff00ff, 0x00c5 << 8},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword5.olfpscurrentadjust_sus=0x01 ;[30:27] =0x01
  ///
  {0x2114, 0x87ffffff, 0x0001 << 27},
  {0x2314, 0x87ffffff, 0x0001 << 27},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword51.olfpscfgpullupdwnres_sus_2_0=0x00
  ///
  { 0x21CC, 0xfffffff8, 0x0000       },
  { 0x23CC, 0xfffffff8, 0x0000       },
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword26.forcevgafull=0x01 ; [24] =1
  ///
  {0x2168, 0xfeffffff, 0x0001 << 24},
  {0x2368, 0xfeffffff, 0x0001 << 24},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword27.opctler1cfg_5_0=0x28 ;[21:16] = 0x28
  ///
  {0x216C, 0xffc0ffff, 0x0028 << 16},
  {0x236C, 0xffc0ffff, 0x0028 << 16},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword27.opctlec2cfg_5_0=0x01 ;[13:8] = 0x01
  ///
  {0x216C, 0xffffc0ff, 0x0001 << 8},
  {0x236C, 0xffffc0ff, 0x0001 << 8},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword27.opctler2cfg_5_0=0x08 ; [29:24] = 0x08
  ///
  {0x216c, 0xc0ffffff, 0x0008 << 24},
  {0x236c, 0xc0ffffff, 0x0008 << 24},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword30.vgavargmctrl_procslow_2_0=0x06 ;[7:5] = 0x06
  ///
  {0x2178, 0xffffff1f, 0x0006 << 5},
  {0x2378, 0xffffff1f, 0x0006 << 5},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword30.vgabiasctrl_procslow_2_0=0x06 ; [2:0] = 0x06
  ///
  {0x2178, 0xfffffff8, 0x0006},
  {0x2378, 0xfffffff8, 0x0006},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword22.dtleedgeeq_offset_5_0=0x00 ; [29:24] = 0x3a
  ///
  { 0x2158, 0xc0ffffff, 0x3a << 24      },
  { 0x2358, 0xc0ffffff, 0x3a << 24      },
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword51.ocfgbonus3_4_1=0x0f ;[28:25]
  ///
  {0x21CC, 0xe1ffffff, 0x000f << 25},
  {0x23CC, 0xe1ffffff, 0x000f << 25},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword43.cfgtimer_smp_vgacnt_7_0=0x6d ; [  15:8] = 0x6d
  ///
  {0x21AC, 0xffff00ff, 0x006d << 8},
  {0x23AC, 0xffff00ff, 0x006d << 8},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword43.cfgtimer_err_vgarst_7_0=0x10 ; [7:0] = 0x10
  ///
  {0x21AC, 0xffffff00, 0x0010},
  {0x23AC, 0xffffff00, 0x0010},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword31.errsamp_vref_trdone_6_0=0x0C; [22:16] = 0x0C
  ///
  {0x217C, 0xff80ffff, 0x000c << 16},
  {0x237C, 0xff80ffff, 0x000c << 16},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword1.oifppmtrg_en=0x1 ; [24] = 0x1
  ///
  {0x2104, 0xfeffffff, 0x0001 << 24},
  {0x2304, 0xfeffffff, 0x0001 << 24},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword1.oiftrgvalue_15_8=0x12 ;[15:8] = 0x12
  ///
  {0x2104, 0xffff00ff, 0x0012 << 8},
  {0x2304, 0xffff00ff, 0x0012 << 8},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword1.oiftrgvalue_7_0=0x50 ; [7:0] = 0x50
  ///
  {0x2104, 0xffffff00, 0x0050},
  {0x2304, 0xffffff00, 0x0050},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword2.pi0autooffset_dis=0x1; [5] = 0x1
  ///
  {0x2108, 0xffffffdf, 0x0001 << 5},
  {0x2308, 0xffffffdf, 0x0001 << 5},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword35.cfg_gaincalvref_max_6_0=0x26 ;[30:24] = 0x26
  ///
  { 0x218C, 0x80ffffff, 0x0025 << 24 },
  { 0x238C, 0x80ffffff, 0x0025 << 24 },
  ///
  /// soc.usb.modphy.soc.dlane1.rx.rx_dword36.cfg_gaincalvref_min_6_0=0x22
  ///
  {0x2190, 0xffffff80, 0x0022},
  {0x2390, 0xffffff80, 0x0022},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword33.vgavargmctrl_proctyp_2_0=0x06 ;[19:17] = 0x06
  ///
  {0x2184, 0xfff1ffff, 0x0006 << 17},
  {0x2384, 0xfff1ffff, 0x0006 << 17},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword33.vgavargmctrl_procfast_2_0 = 0x06 ;[22:20] = 0x06
  ///
  {0x2184, 0xff8fffff, 0x0006 << 20},
  {0x2384, 0xff8fffff, 0x0006 << 20},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword26.dtle_acq_timer = 0xa
  ///
  {0x2168, 0xfffffff0, 0x000a},
  {0x2368, 0xfffffff0, 0x000a},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword26.dtle_update_rate=0x3 ;[27:26] = 0x3
  ///
  {0x2168, 0xf3ffffff, 0x0003 << 26},
  {0x2368, 0xf3ffffff, 0x0003 << 26},
  ///
  /// soc.usb.modphy.dlane1.rx.rx_dword28.vga_update_rate = 0x1 ; [2:1] = 0x1
  ///
  {0x2170, 0xfffffff9, 0x0001 << 1},
  {0x2370, 0xfffffff9, 0x0001 << 1},
  ///
  /// soc.usb.modphy.dlane1.pcs.pcs_dword1.reg_bonus_pcs_sus = 0x01 ;[15:8] = 0x1
  ///
  {0x2004, 0xffff00ff, 0x0001 << 8},
  {0x2204, 0xffff00ff, 0x0001 << 8},
  ///
  /// reg_rx_stagger_mult[2:0] [default]
  /// reg_pclk_request
  ///
  {0x2004, 0xffffffbf, 0x0000 << 6},
  {0x2204, 0xffffffbf, 0x0000 << 6},
  ///
  /// soc.usb.modphy.clane0.dig.cmn_dig_dword15.o_cfg_cmn_dynclkgate_mode_1_0=0 ;0x803c [2:1]= 3
  ///
  {0x803C, 0xfffffff9, 0x3 << 1},
  ///
  /// reg_enable_pwr_gating
  ///
  //{ 0x2024, 0xdfffffff, 0x0001 << 29 },
  //{ 0x2224, 0xdfffffff, 0x0001 << 29 },
  ///
  /// soc.usb.modphy.clane0.dig.cmn_dig_dword12.o_cfg_pwr_gating_ctrl=0; 0x8030 [0]=1
  ///
  {0x8030, 0xfffffffe, 0x1},
  ///
  /// soc.usb.modphy.clane0.dig.cmn_dig_dword13.o_ldo_sw_cfg_sus_15_8=0x40; 8034 [15:8]=40
  ///
  {0x8034, 0xffff00ff, 0x40 << 8},
  ///
  /// soc.usb.modphy.clane0.dig.cmn_dig_dword12.o_cfg_criclk_ungate_h=0x0 ;8030 [23]=0
  ///
  {0x8030, 0xff7fffff, 0x0 << 23},
  ///
  /// End: RX Section
  ///
/// ---------------------------------end of kg_registers_A0_RC04.xlsm---------------------------------
};

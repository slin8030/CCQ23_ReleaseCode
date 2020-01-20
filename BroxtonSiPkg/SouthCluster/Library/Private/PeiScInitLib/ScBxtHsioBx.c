/** @file
  BXT Bx/E0 HSIO C File

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

UINT16 ScBxtBxUsb3HsioAddress[2] = {
  0x2000,
  0x2200
};

UINT16 ScBxtBxPcieHsioAddress[2] = {
  0x0,
  0x2200
};

SC_SBI_HSIO_TABLE_STRUCT ScBxtHsioTable_Bx[] = {
  // Sequence ID: 3
  {0xA5, 0xdfffffff, 0x20000000, 0x00008020, SC_LANE_OWN_COMMON        },  // Disable fast lock on the PLL
  // Sequence ID: 4
  {0xA5, 0xffffffe0, 0x0000000f, 0x00000024, V_SC_PCR_FIA_LANE_OWN_USB3},  // USB3 EB High Watermark
  {0xA5, 0xffffe0ff, 0x00000c00, 0x00000024, V_SC_PCR_FIA_LANE_OWN_USB3},  // USB3 EB Low Watermark
  {0xA5, 0xffe0ffff, 0x000c0000, 0x00000024, V_SC_PCR_FIA_LANE_OWN_USB3},  // USB3 EB Initial Separation
  {0xA5, 0xfffffff0, 0x00000004, 0x00000020, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Symbol Align lock Count
  {0xA5, 0xfffffff0, 0x0000000d, 0x00000020, V_SC_PCR_FIA_LANE_OWN_USB3},  // Symbol Align lock Count
  {0xA5, 0xfff8ffff, 0x00020000, 0x00000018, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Symbol Align unlock/relock Count
  {0xA5, 0xfdffffff, 0x02000000, 0x00000004, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Disable dynamic (single COM) locking
  {0xA5, 0xfffffeff, 0x00000100, 0x00000010, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Override kalignmode input, use CRI register
  {0xA5, 0xffffff8f, 0x00000000, 0x00000020, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Rx stagger multiplier (3: mul = 4; 4: mul = 8)
  {0xA5, 0x1fffffff, 0x00000000, 0x0000001c, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Tx stagger multiplier
  {0xA5, 0x1fffffff, 0x00000000, 0x0000001c, V_SC_PCR_FIA_LANE_OWN_USB3},  // Tx stagger multiplier
  {0xA5, 0xffffdfff, 0x00000000, 0x00000028, V_SC_PCR_FIA_LANE_OWN_USB3},  // Disabling Raw LFPS mode for USB3
  {0xA5, 0xff03ffff, 0x007c0000, 0x0000003c, V_SC_PCR_FIA_LANE_OWN_PCIE},  // LDO control register
  {0xA5, 0xff03ffff, 0x007c0000, 0x0000003c, V_SC_PCR_FIA_LANE_OWN_USB3},  // LDO control register
  {0xA5, 0xffffff00, 0x00000040, 0x00008034, SC_LANE_OWN_COMMON        },  // Drowsy mode enable (can be ignored for PG families)
  {0xA5, 0xffff00ff, 0x00005100, 0x00000090, V_SC_PCR_FIA_LANE_OWN_PCIE},  // TX 3p5db deemp setting
  {0xA5, 0xffff00ff, 0x00005100, 0x00000090, V_SC_PCR_FIA_LANE_OWN_USB3},  // TX 3p5db deemp setting
  {0xA5, 0xff00ffff, 0x003f0000, 0x00000090, V_SC_PCR_FIA_LANE_OWN_PCIE},  // TX 6p0db deemp setting
  {0xA5, 0xff00ffff, 0x003f0000, 0x00000090, V_SC_PCR_FIA_LANE_OWN_USB3},  // TX 6p0db deemp setting
  {0xA5, 0xffffff7f, 0x00000080, 0x000000ac, V_SC_PCR_FIA_LANE_OWN_USB3},  // Forcing Rx detect to use Vss to Vcm band
  {0xA5, 0xffffff00, 0x00000010, 0x00000140, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Full rate proportional settings (kp1)
  {0xA5, 0xffffff00, 0x00000010, 0x00000140, V_SC_PCR_FIA_LANE_OWN_USB3},  // Full rate proportional settings (kp1)
  {0xA5, 0xffff00ff, 0x0000c700, 0x00000140, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Full rate proportional settings (kp2)
  {0xA5, 0xffff00ff, 0x0000c500, 0x00000140, V_SC_PCR_FIA_LANE_OWN_USB3},  // Full rate proportional settings (kp2)
  {0xA5, 0x87ffffff, 0x08000000, 0x00000114, V_SC_PCR_FIA_LANE_OWN_USB3},  // LFPS main bias blk current settings
  {0xA5, 0xfffffff8, 0x00000000, 0x000001cc, V_SC_PCR_FIA_LANE_OWN_USB3},  // LFPS input offset control
  {0xA5, 0xfeffffff, 0x01000000, 0x00000168, V_SC_PCR_FIA_LANE_OWN_USB3},  // VGA gain
  {0xA5, 0xffc0ffff, 0x00280000, 0x0000016c, V_SC_PCR_FIA_LANE_OWN_USB3},  // CTLE R1
  {0xA5, 0xffffc0ff, 0x00000100, 0x0000016c, V_SC_PCR_FIA_LANE_OWN_PCIE},  // CTLE C1
  {0xA5, 0xffffc0ff, 0x00000100, 0x0000016c, V_SC_PCR_FIA_LANE_OWN_USB3},  // CTLE C1
  {0xA5, 0xc0ffffff, 0x10000000, 0x0000016c, V_SC_PCR_FIA_LANE_OWN_PCIE},  // CTLE R2
  {0xA5, 0xc0ffffff, 0x08000000, 0x0000016c, V_SC_PCR_FIA_LANE_OWN_USB3},  // CTLE R2
  {0xA5, 0xffffff1f, 0x000000c0, 0x00000178, V_SC_PCR_FIA_LANE_OWN_USB3},  // Variable leg on VGA
  {0xA5, 0xfffffff8, 0x00000006, 0x00000178, V_SC_PCR_FIA_LANE_OWN_USB3},  // VGA bias
  {0xA5, 0xfffffff0, 0x00000007, 0x00000110, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Reference for Squelch high
  {0xA5, 0xf0ffffff, 0x07000000, 0x0000010c, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Reference for Squelch high with procmon
  {0xA5, 0xfffff0ff, 0x00000600, 0x00000110, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Squelch BIAS
  {0xA5, 0xe1ffffff, 0x1e000000, 0x000001cc, V_SC_PCR_FIA_LANE_OWN_PCIE},  // source follower configuration
  {0xA5, 0xe1ffffff, 0x1e000000, 0x000001cc, V_SC_PCR_FIA_LANE_OWN_USB3},  // source follower configuration
  {0xA5, 0xffff00ff, 0x00006d00, 0x000001ac, V_SC_PCR_FIA_LANE_OWN_USB3},  // Sampled Error Timer
  {0xA5, 0xffffff00, 0x00000010, 0x000001ac, V_SC_PCR_FIA_LANE_OWN_USB3},  // Vref Change Delay
  {0xA5, 0xff80ffff, 0x000c0000, 0x0000017c, V_SC_PCR_FIA_LANE_OWN_USB3},  // Error sampler train done Vref
  {0xA5, 0xc0ffffff, 0x3a000000, 0x00000158, V_SC_PCR_FIA_LANE_OWN_USB3},  // Edge DTLE for SATA Only
  {0xA5, 0xfeffffff, 0x01000000, 0x00000104, V_SC_PCR_FIA_LANE_OWN_USB3},  // IF reset trigger
  {0xA5, 0xc0ffffff, 0x27000000, 0x00000150, V_SC_PCR_FIA_LANE_OWN_PCIE},  // DATA EQ for PCIE only
  {0xA5, 0xffc0ffff, 0x00200000, 0x00000154, V_SC_PCR_FIA_LANE_OWN_PCIE},  // EDGE EQ for PCIE only
  {0xA5, 0xffdfffff, 0x00200000, 0x00000178, V_SC_PCR_FIA_LANE_OWN_PCIE},  // VGA load resistor select
  {0xA5, 0xffffc0ff, 0x00003f00, 0x00000178, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Load Trim
  {0xA5, 0xffff00ff, 0x00001200, 0x00000104, V_SC_PCR_FIA_LANE_OWN_USB3},  // PPM trigger timer to reset Integral Filter - LSByte
  {0xA5, 0xffffff00, 0x00000050, 0x00000104, V_SC_PCR_FIA_LANE_OWN_USB3},  // PPM trigger timer to reset Integral Filter - MSByte
  {0xA5, 0xffffffdf, 0x00000020, 0x00000108, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Prevent PI reload during periodic cal
  {0xA5, 0xffffffdf, 0x00000020, 0x00000108, V_SC_PCR_FIA_LANE_OWN_USB3},  // Prevent PI reload during periodic cal
  {0xA5, 0x80ffffff, 0x25000000, 0x0000018c, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Training Values for VGA
  {0xA5, 0x80ffffff, 0x25000000, 0x0000018c, V_SC_PCR_FIA_LANE_OWN_USB3},  // Training Values for VGA
  {0xA5, 0xffffff80, 0x00000022, 0x00000190, V_SC_PCR_FIA_LANE_OWN_PCIE},  // Training Values for VGA
  {0xA5, 0xffffff80, 0x00000022, 0x00000190, V_SC_PCR_FIA_LANE_OWN_USB3},  // Training Values for VGA
  {0xA5, 0xfff1ffff, 0x000c0000, 0x00000184, V_SC_PCR_FIA_LANE_OWN_USB3},  // VGA GM --typ
  {0xA5, 0xff8fffff, 0x00600000, 0x00000184, V_SC_PCR_FIA_LANE_OWN_USB3},  // VGA GM - fast
  {0xA5, 0xfffffff0, 0x0000000f, 0x00000168, V_SC_PCR_FIA_LANE_OWN_USB3},  // DTLE Acqusition timer
  {0xA5, 0xf3ffffff, 0x0c000000, 0x00000168, V_SC_PCR_FIA_LANE_OWN_USB3},  // DTLE update rate
  {0xA5, 0xfffffff9, 0x00000002, 0x00000170, V_SC_PCR_FIA_LANE_OWN_USB3},  // VGA update rate
  {0xA5, 0xffff00ff, 0x00003300, 0x00000004, V_SC_PCR_FIA_LANE_OWN_PCIE},  // BIOS version work week
  {0xA5, 0xffff00ff, 0x00003300, 0x00000004, V_SC_PCR_FIA_LANE_OWN_USB3},  // BIOS version work week
  {0xA5, 0xffffff00, 0x0000006f, 0x00008008, SC_LANE_OWN_COMMON        },  // PMC version ID (CMNFUSE2) year
  // Sequence ID: 5
  {0xA5, 0xffffffbf, 0x00000000, 0x00000004, V_SC_PCR_FIA_LANE_OWN_PCIE},  // PCLK request behavior
  {0xA5, 0xffffffbf, 0x00000000, 0x00000004, V_SC_PCR_FIA_LANE_OWN_USB3},  // PCLK request behavior
  // Sequence ID: 6
  {0xA5, 0xffff00ff, 0x00004000, 0x00008034, SC_LANE_OWN_COMMON        },  // enable drowsy mode
};  // Total # of items = 68


SC_SIDEBAND_TABLE_STRUCT ScBxtUsb2PhyCommonTable_E0[] = {
  {R_PCR_USB2_GLOBAL_PORT,      0xFCFFCFF7, 0x03003000},
  {R_PCR_USB2_GLB_ADP_VBUS_REG, 0xFFBFFFFF, 0x00400000},
  {R_PCR_USB2_GLOBAL_PORT_2,    0xF7BFFC7F, 0x00400380},
  {R_PCR_USB2_CFG_COMPBG,       0xFFFF007F, 0x00008480},
  {R_PCR_USB2_CONFIG_3,         0x00000000, 0x59E94D0C},
  {R_PCR_USB2_DFT_1,            0xFFFF0FFF, 0x0000F000},
  {R_PCR_USB2_SFRCONFIG_0,      0xFFFFFCFF, 0x00000200}
};

SC_SIDEBAND_TABLE_STRUCT ScBxtUsb2PhyPerPortTable_E0[] = {
  {R_PCR_USB2_PPORT,   0x03F180EF, 0xFC042810},
  {R_PCR_UTMI_MISC_PR, 0xFFFFC17F, 0x00001A80},
  {R_PCR_USB2_PPORT2,  0xFC7FFFFC, 0x03800003}
};
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

SC_SIDEBAND_TABLE_STRUCT ScSsicHsioTable_Bx_RateA_Dlane_PG[] = {
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
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG21, 0xffffff00, 0x00000020},
  ///
  /// Minden specific settings
  /// CFG_UPDATE(); dlane0.impreg25.tx_tob_extn_override_en = 1
  ///
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffffeff, 0x00000100},
  ///
  /// CFG_UPDATE(); dlane0.impreg25.tx_pwm_tob_extension = 0x20
  ///
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xffffff00, 0x00000020},
  ///
  /// CFG_UPDATE(); dlane0.impreg25.force_tx_lcc_disable = 1
  ///
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffeffff, 0x00010000},
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  ///Enable Dlane Power gating
  ///CFG_UPDATE(); impreg8.[25:24] = b'11
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
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
SC_SIDEBAND_TABLE_STRUCT ScBxtpSsicModphy_RateA_Dlane_PG[] = {
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

SC_SIDEBAND_TABLE_STRUCT ScSsicHsioTable_Bx_RateB_Dlane_PG[] = {
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

  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG21, 0xffffff00, 0x00000020 },

  /// Minden specific settings
  //cfg_updt(); dlane0.impreg25.tx_tob_extn_override_en = 1
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffffeff, 0x00000100 },

  ///cfg_updt(); dlane0.impreg25.tx_pwm_tob_extension = 0x20
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xffffff00, 0x00000020 },

  ///cfg_updt(); dlane0.impreg25.force_tx_lcc_disable = 1
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffeffff, 0x00010000 },
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  ///Enable Dlane Power gating
  ///CFG_UPDATE(); impreg8.[25:24] = b'11
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
};

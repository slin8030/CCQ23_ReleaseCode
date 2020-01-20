/** @file
  BXT-P Bx HSIO C File

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

#include <Uefi/UefiBaseType.h>
#include <ScAccess.h>
#include <Private/ScHsio.h>


UINT16 ScBxtpUsb3HsioAddress[6] = {
  0x2000,
  0x2200,
  0x2400,
  0x2600,
  0xE00,
  0xA00
};

UINT16 ScBxtpPcieHsioAddress[6] = {
  0x800,
  0x200,
  0x0,
  0xE00,
  0x2600,
  0x2400
};

UINT16 ScBxtpSataHsioAddress[2] = {
  0xC00,
  0xA00,
};

SC_SBI_HSIO_TABLE_STRUCT ScBxtpHsioTable_Bx[] = {
   {0xA5, 0xDFFFFFFF, 0x20000000, 0x8020, SC_LANE_OWN_COMMON},         // LANE
   {0xB1, 0xDFFFFFFF, 0x20000000, 0x8020, SC_LANE_OWN_COMMON},         // LANE
   {0xA5, 0xFFFFFF00,       0x40, 0x8034, SC_LANE_OWN_COMMON},         // LANE
   {0xB1, 0xFFFFFF00,       0x40, 0x8034, SC_LANE_OWN_COMMON},         // LANE
   {0xA5, 0xFFFFFF00,       0x1E, 0x8008, SC_LANE_OWN_COMMON},         // LANE
   {0xB1, 0xFFFFFF00,       0x1E, 0x8008, SC_LANE_OWN_COMMON},         // LANE
   {0xA5, 0xFFE0E0E0,    0xC0C0F,   0x24, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFFFFDFFF,        0x0,   0x28, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFDFF00FF,  0x2001E00,   0x04, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xFFFF00FF,     0x1E00,   0x04, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFBFF00FF,  0x4001E00,   0x04, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFFFFF80,        0x4,   0x20, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xFFFFFFF0,        0xD,   0x20, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFFFFFFF0,        0xD,   0x20, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFF8FFFF,    0x20000,   0x18, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xFFFFFEFF,      0x100,   0x10, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0x1FFFFFFF,        0x0,   0x1C, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0x1FFFFFFF,        0x0,   0x1C, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0x1FFFFFFF,        0x0,   0x1C, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0x000000FF, 0xE0163500,   0x64, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFF03FFFF,   0x7C0000,   0x3C, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xFF03FFFF,   0x7C0000,   0x3C, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFF03FFFF,   0x7C0000,   0x3C, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFF0000FF,   0x3F5100,   0x90, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xFF0000FF,   0x3F5100,   0x90, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFF000000,   0x3F406F,   0x90, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFFFFDFF,      0x200,   0xB8, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFFFFF7F,       0x80,   0xAC, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xBFFFFFFF, 0x40000000,   0x94, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFF00FFFF,   0x690000,   0x8C, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFFF0000,     0xC710,  0x140, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xFF000000,   0x12C510,  0x140, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFF000000,   0x12C50A,  0x140, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFF000000,    0x8CB0A,  0x144, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFF000000,    0x4CB0C,  0x148, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0x87FFFFFF,  0x8000000,  0x114, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xE1FFFFF8, 0x1E000007,  0x1CC, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xE1FF80F8, 0x1E001A03,  0x1CC, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xE1FF80F8, 0x1E001A07,  0x1CC, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xF2FFFFF0,  0xD00000F,  0x168, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFEFFF7FF,  0x1000800,  0x168, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xC0FFC0FF, 0x10000100,  0x16C, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xC0C0C0FF,  0x8283F00,  0x16C, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xC0C0C0FF, 0x10000700,  0x16C, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFDFC0FF,   0x203F00,  0x178, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xFFDFC018,   0x2039C6,  0x178, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFFDFC018,   0x2037C6,  0x178, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFFFF0F0,      0x607,  0x110, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0x00FFFFFF, 0x84000000,  0x110, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFFFFF000,      0x6C7,  0x110, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xF0FFFFFF,  0x7000000,  0x10C, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xFFFF0000,     0xB7C7,  0x118, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFE3FFFF,    0x80000,  0x124, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFFF0000,     0x6D10,  0x1AC, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFFFF0000,     0x4109,  0x1AC, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFF80FFFF,    0x70000,  0x17C, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFF80FFFF,    0x40000,  0x17C, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xC0C0FFFF, 0x3C020000,  0x158, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xC0C0FFFF, 0x3C040000,  0x158, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFEFF0000,  0x1001250,  0x104, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFE000000,  0x14A1610,  0x104, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFFFFFDF,       0x20,  0x13C, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xC0FFFFFF, 0x27000000,  0x150, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xFFC0FFFF,   0x200000,  0x154, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xC0FFFFC0, 0x20000020,  0x154, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFFFFFDF,       0x20,  0x108, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xFFFFFFDF,       0x20,  0x108, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFFFFFFDF,       0x20,  0x108, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0x80FFFFFF, 0x25000000,  0x18C, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0x80FFFFFF, 0x25000000,  0x18C, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0x80FFFFFF, 0x25000000,  0x18C, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFFFFF80,       0x22,  0x190, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xFFFFFF80,       0x22,  0x190, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFFFFFF80,       0x22,  0x190, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFF81FFFF,   0x6C0000,  0x184, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFFFFFFF9,        0x2,  0x170, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFFFFFFBF,        0x0,   0x04, V_SC_PCR_FIA_LANE_OWN_PCIE}, // UNICAST
   {0xA5, 0xFFFFFFBF,        0x0,   0x04, V_SC_PCR_FIA_LANE_OWN_USB3}, // UNICAST
   {0xA5, 0xFFFFFFBF,        0x0,   0x04, V_SC_PCR_FIA_LANE_OWN_SATA}, // UNICAST
   {0xA5, 0xFFFF00FF,     0x4000, 0x8034, SC_LANE_OWN_COMMON},         // LANE
   {0xB1, 0xFFFF00FF,     0x4000, 0x8034, SC_LANE_OWN_COMMON},         // LANE
};

SC_SIDEBAND_TABLE_STRUCT ScBxtpUsb2PhyCommonTable_Bx[] = {
  {R_PCR_USB2_GLOBAL_PORT,      0xFCFFCFF7, 0x03003000},
  {R_PCR_USB2_GLB_ADP_VBUS_REG, 0xFFBFFFFF, 0x00400000},
  {R_PCR_USB2_GLOBAL_PORT_2,    0xF7BFFC7F, 0x00400380},
  {R_PCR_USB2_CFG_COMPBG,       0xFFFF007F, 0x00008680},
  {R_PCR_USB2_CONFIG_3,         0x00000000, 0x59E94D0C},
  {R_PCR_USB2_DFT_1,            0xFFFF0FFF, 0x0000F000},
  {R_PCR_USB2_SFRCONFIG_0,      0xFFFFFCFF, 0x00000200}
};

SC_SIDEBAND_TABLE_STRUCT ScBxtpUsb2PhyPerPortTable_Bx[] = {
  {R_PCR_USB2_PPORT,   0x03F180EF, 0xFC043810},
  {R_PCR_UTMI_MISC_PR, 0xFFFFC17F, 0x00001A80},
  {R_PCR_USB2_PPORT2,  0xFC7FFFFC, 0x01800000}
};

SC_SIDEBAND_TABLE_STRUCT ScBxtpSsicModphy_Bx_RateA[] = {
  // clane.cmnreg1.hfpll_enable = 0; CFG_UPDATE()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000000},
  CFG_UPDATE,
  // clane.cmnreg1.lfpll_enable = 0; CFG_UPDATE()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000000},
  CFG_UPDATE,
  // CFG_UPDATE(); clane.cmnreg9.pllrefsel = 0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG9, 0xfeffffff, 0x00000000},
  // CFG_UPDATE(); clane.cmnreg21.hfpll_refcal = 0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfffff8ff, 0x00000000},
  // CFG_UPDATE(); clane.cmnreg2.hfpll_divisors_msb =0x1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffff00ff, 0x00000100},
  // CFG_UPDATE(); clane.cmnreg2.hfpll_divisors_lsb =0x04
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffffff00, 0x00000004 },
  // CFG_UPDATE(); clane.cmnreg1.lfpll_ndiv = 0x17
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0x00ffffff, 0x17000000},
  // CFG_UPDATE(); clane.cmnreg1.maxpwmgearsel = 0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0xfff8ffff, 0x00000000},
  // CFG_UPDATE(); clane.cmnreg21.hfpll_hsrate = 0x1   # Rate A
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfeffffff, 0x01000000},
  // CFG_UPDATE(); clane.cmnreg5.bitwidth = 0x2   # 20-bit width
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG5, 0xfffffffc, 0x00000002},
  // CFG_UPDATE(); clane.cmnreg7.maxhsgearsel = 0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG7, 0xfffffcff, 0x00000000},
  // CFG_UPDATE(); clane.cmnreg18.hfpll_cfg3 = 0x1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG18, 0xe0ffffff, 0x01000000},
  // CFG_UPDATE(); clane.cmnreg17.hfpll_cfg2_byte3 = 0x45
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0x00ffffff, 0x45000000},
  // CFG_UPDATE(); clane.cmnreg17.hfpll_cfg2_byte2 = 0x1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xff00ffff, 0x00010000},
  // CFG_UPDATE(); clane.cmnreg17.hfpll_cfg2_byte1 = 0xac
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffff00ff, 0x0000AC00},
  // CFG_UPDATE(); clane.cmnreg17.hfpll_cfg2_byte0 = 0xd
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffffff00, 0x0000000D},
  // CFG_UPDATE(); clane.cmnreg6.hfpll_cfg1_byte3 = 0xc1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0x00ffffff, 0xc1000000},
  // CFG_UPDATE(); clane.cmnreg6.hfpll_cfg1_byte2 = 0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xff00ffff, 0x00000000},
  // CFG_UPDATE(); clane.cmnreg6.hfpll_cfg1_byte1 = 0x3
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffff00ff, 0x00000300},
  // CFG_UPDATE(); clane.cmnreg6.hfpll_cfg1_byte0 = 0x16
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffffff00, 0x00000016},
  // CFG_UPDATE(); clane.cmnreg3.hfpll_afcen = 0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG3, 0xfffffffe, 0x00000000},
  // Enable LF-PLL and HF-PLL
  // clane.cmnreg1.lfpll_enable = 1; CFG_UPDATE()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000001},
  CFG_UPDATE,
  // clane.cmnreg1.hfpll_enable = 1; CFG_UPDATE()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000100},
  CFG_UPDATE,
  // Enable Clock Gating
  // clane.cmnreg4.[5:0]= b100011; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG4, 0xffffffc0, 0x0023 },
  CFG_UPDATE,
  // Enable CLANE Power Gating feature per MPHY Integration Guide
  //  clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffffa, 0x00000003},
  CFG_UPDATE,
  // Enable Slow Squelch
  //  clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffeff, 0x00000100},
  CFG_UPDATE,
  //
  // if not init_phy == 0:
  // if not port1 == 0:
  // def CFG_UPDATE():
  // dlane0.impreg22.cnfgupd = 0x4
  // CFG_UPDATE(); dlane0.impreg21.rx_squelch_enable_timer = 0x20
  //
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG21, 0xffffff00, 0x00000020},
  //
  // Minden specific settings
  // CFG_UPDATE(); dlane0.impreg25.tx_tob_extn_override_en = 1
  //
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffffeff, 0x00000100},
  //
  // CFG_UPDATE(); dlane0.impreg25.tx_pwm_tob_extension = 0x20
  //
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xffffff00, 0x00000020},
  //
  // CFG_UPDATE(); dlane0.impreg25.force_tx_lcc_disable = 1
  //
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000},
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffeffff, 0x00010000},
  // Enable Dlane Power gating
  // CFG_UPDATE(); dlane2.impreg8.[25:24] = b'11
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
};

SC_SIDEBAND_TABLE_STRUCT ScBxtpSsicModphy_Bx_RateB[] = {
  // clane.cmnreg1.hfpll_enable = 0; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000000 },
  CFG_UPDATE,
  // clane.cmnreg1.lfpll_enable = 0; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000000 },
  CFG_UPDATE,
  // cfg_updt(); clane.cmnreg9.pllrefsel =         0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG9, 0xfeffffff, 0x00000000 },
  // cfg_updt(); clane.cmnreg21.hfpll_refcal =     0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfffff8ff, 0x00000000 },
  // cfg_updt(); clane.cmnreg2.hfpll_divisors_msb =0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffff00ff, 0x00000000 },
  // cfg_updt(); clane.cmnreg2.hfpll_divisors_lsb =0xB6
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffffff00, 0x000000B6 },
  // cfg_updt(); clane.cmnreg1.lfpll_ndiv =        0x1A # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0x00ffffff, 0x1A000000 },
  // cfg_updt(); clane.cmnreg1.maxpwmgearsel =     0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0xfff8ffff, 0x00000000 },
  // cfg_updt(); clane.cmnreg21.hfpll_hsrate =     0x0   # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfeffffff, 0x00000000 },
  // cfg_updt(); clane.cmnreg5.bitwidth =          0x2   # 20-bit width
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG5, 0xfffffffc, 0x00000002 },
  // cfg_updt(); clane.cmnreg7.maxhsgearsel =      0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG7, 0xfffffcff, 0x00000000 },
  // cfg_updt(); clane.cmnreg18.hfpll_cfg3 =       0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG18, 0xe0ffffff, 0x00000000 },
  // cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte3 = 0xE3 # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0x00ffffff, 0xE3000000 },
  // cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte2 = 0x81
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xff00ffff, 0x00810000 },
  // cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte1 = 0xac
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffff00ff, 0x0000AC00 },
  // cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte0 = 0xd
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffffff00, 0x0000000D },
  // cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte3 =  0xc1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0x00ffffff, 0xc1000000 },
  // cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte2 =  0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xff00ffff, 0x00000000 },
  // cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte1 =  0x3 # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffff00ff, 0x00000300 },
  // cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte0 =  0x16
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffffff00, 0x00000016 },
  //
  // CFG_UPDATE(); clane.cmnreg3.hfpll_afcen = 0
  //
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG3, 0xfffffffe, 0x00000000},
  //
  // Enable LF-PLL and HF-PLL
  // clane.cmnreg1.lfpll_enable = 1; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000001 },
  CFG_UPDATE,
  // clane.cmnreg1.hfpll_enable = 1; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000100 },
  CFG_UPDATE,
  // Enable Clock Gating
  // clane.cmnreg4.[5:0]= b100011; cfg_updt()
  {R_SC_PCR_MMP0_CMNREG4, 0xffffffc0, 0x0023 },
  CFG_UPDATE,
  // Enable CLANE Power Gating feature per MPHY Integration Guide
  // clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffffa, 0x00000003},
  CFG_UPDATE,
  // Enable Slow Squelch
  // clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffeff, 0x00000100},
  CFG_UPDATE,
  // if not init_phy == 0:
  // if not port1 == 0:
  // def cfg_updt():
  // dlane0.impreg22.cnfgupd = 0x4
  // cfg_updt(); dlane0.impreg21.rx_squelch_enable_timer = 0x20
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG21, 0xffffff00, 0x00000020 },
  // Minden specific settings
  //cfg_updt(); dlane0.impreg25.tx_tob_extn_override_en = 1
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffffeff, 0x00000100 },
  // cfg_updt(); dlane0.impreg25.tx_pwm_tob_extension = 0x20
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xffffff00, 0x00000020 },
  ///cfg_updt(); dlane0.impreg25.force_tx_lcc_disable = 1
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffeffff, 0x00010000 },
  // Enable Dlane Power gating
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
};

SC_SIDEBAND_TABLE_STRUCT ScBxtpSsicModphy_RateB_Dlane_PG[] = {
  //Enable Dlane Power gating
  //CFG_UPDATE(); dlane2.impreg8.[25:24] = b'11
  // clane.cmnreg1.hfpll_enable = 0; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000000 },
  CFG_UPDATE,
  // clane.cmnreg1.lfpll_enable = 0; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000000 },
  CFG_UPDATE,
  // cfg_updt(); clane.cmnreg9.pllrefsel =         0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG9, 0xfeffffff, 0x00000000 },
  // cfg_updt(); clane.cmnreg21.hfpll_refcal =     0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfffff8ff, 0x00000000 },
  // cfg_updt(); clane.cmnreg2.hfpll_divisors_msb =0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffff00ff, 0x00000000 },
  // cfg_updt(); clane.cmnreg2.hfpll_divisors_lsb =0xB6
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG2, 0xffffff00, 0x000000B6 },
  // cfg_updt(); clane.cmnreg1.lfpll_ndiv =        0x1A # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0x00ffffff, 0x1A000000 },
  // cfg_updt(); clane.cmnreg1.maxpwmgearsel =     0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG1, 0xfff8ffff, 0x00000000 },
  // cfg_updt(); clane.cmnreg21.hfpll_hsrate =     0x0   # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG21, 0xfeffffff, 0x00000000 },
  // cfg_updt(); clane.cmnreg5.bitwidth =          0x2   # 20-bit width
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG5, 0xfffffffc, 0x00000002 },
  // cfg_updt(); clane.cmnreg7.maxhsgearsel =      0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG7, 0xfffffcff, 0x00000000 },
  // cfg_updt(); clane.cmnreg18.hfpll_cfg3 =       0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG18, 0xe0ffffff, 0x00000000 },
  // cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte3 = 0xE3 # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0x00ffffff, 0xE3000000 },
  // cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte2 = 0x81
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xff00ffff, 0x00810000 },
  // cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte1 = 0xac
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffff00ff, 0x0000AC00 },
  // cfg_updt(); clane.cmnreg17.hfpll_cfg2_byte0 = 0xd
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG17, 0xffffff00, 0x0000000D },
  // cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte3 =  0xc1
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0x00ffffff, 0xc1000000 },
  // cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte2 =  0x0
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xff00ffff, 0x00000000 },
  // cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte1 =  0x3 # Rate B
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffff00ff, 0x00000300 },
  // cfg_updt(); clane.cmnreg6.hfpll_cfg1_byte0 =  0x16
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG6, 0xffffff00, 0x00000016 },
  //
  // CFG_UPDATE(); clane.cmnreg3.hfpll_afcen = 0
  //
  CFG_UPDATE,
  { R_SC_PCR_MMP0_CMNREG3, 0xfffffffe, 0x00000000},
  //
  // Enable LF-PLL and HF-PLL
  // clane.cmnreg1.lfpll_enable = 1; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffffe, 0x00000001 },
  CFG_UPDATE,
  // clane.cmnreg1.hfpll_enable = 1; cfg_updt()
  { R_SC_PCR_MMP0_CMNREG1, 0xfffffeff, 0x00000100 },
  CFG_UPDATE,
  // Enable Clock Gating
  // clane.cmnreg4.[5:0]= b100011; cfg_updt()
  {R_SC_PCR_MMP0_CMNREG4, 0xffffffc0, 0x0023 },
  CFG_UPDATE,
  // Enable CLANE Power Gating feature per MPHY Integration Guide
  // clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffffa, 0x00000003},
  CFG_UPDATE,
  // Enable Slow Squelch
  // clane.cmnreg20.power_gate_en = 3; clane.cmnreg16.cfg_update = 1
  { R_SC_PCR_MMP0_CMNREG20, 0xfffffeff, 0x00000100},
  CFG_UPDATE,
  // if not init_phy == 0:
  // if not port1 == 0:
  // def cfg_updt():
  // dlane0.impreg22.cnfgupd = 0x4
  // cfg_updt(); dlane0.impreg21.rx_squelch_enable_timer = 0x20
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG21, 0xffffff00, 0x00000020 },
  // Minden specific settings
  //cfg_updt(); dlane0.impreg25.tx_tob_extn_override_en = 1
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffffeff, 0x00000100 },
  // cfg_updt(); dlane0.impreg25.tx_pwm_tob_extension = 0x20
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xffffff00, 0x00000020 },
  //cfg_updt(); dlane0.impreg25.force_tx_lcc_disable = 1
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG25, 0xfffeffff, 0x00010000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  //Enable Dlane Power gating
  //CFG_UPDATE(); impreg8.[25:24] = b'11
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },
  { R_SC_PCR_MMP0_LANE_0_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_1_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },
  { R_SC_PCR_MMP0_LANE_1_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG8,  0xfaffffff, 0x03000000 },
  { R_SC_PCR_MMP0_LANE_2_OFFSET + R_SC_PCR_MMP0_IMPREG22, 0xf8ffffff, 0x04000000 },
};

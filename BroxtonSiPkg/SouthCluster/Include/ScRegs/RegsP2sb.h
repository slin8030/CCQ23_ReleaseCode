/** @file
  Register names for SC P2SB device

  Conventions:

  - Prefixes:
    Definitions beginning with "R_" are registers
    Definitions beginning with "B_" are bits within registers
    Definitions beginning with "V_" are meaningful values of bits within the registers
    Definitions beginning with "S_" are register sizes
    Definitions beginning with "N_" are the bit position
  - In general, SC registers are denoted by "_SC_" in register names
  - Registers / bits that are different between SC generations are denoted by
    "_SC_<generation_name>_" in register/bit names.
  - Registers / bits that are different between SKUs are denoted by "_<SKU_name>"
    at the end of the register/bit names
  - Registers / bits of new devices introduced in a SC generation will be just named
    as "_SC_" without <generation_name> inserted.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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
#ifndef _PCH_REGS_P2SB_H_
#define _PCH_REGS_P2SB_H_

#include <PlatformBaseAddresses.h>

// PCI to P2SB Bridge Registers (D13:F0)
#define PCI_DEVICE_NUMBER_P2SB           13
#define PCI_FUNCTION_NUMBER_P2SB         0
#define P2SB_MMIO_ADDR(base, port, reg)  (base | (port & 0xFF) <<16 | (reg & 0xFFFC))

#define V_PCH_P2SB_VENDOR_ID                       V_INTEL_VENDOR_ID
#define R_PCH_P2SB_SBREG_BAR                       0x10
#define B_PCH_P2SB_SBREG_RBA                       0xFF000000
#define R_PCH_P2SB_SBREG_BARH                      0x14
#define B_PCH_P2SB_SBREG_RBAH                      0xFFFFFFFF
#define R_PCH_P2SB_VBDF                            0x50
#define B_PCH_P2SB_VBDF_BUF                        0xFF00
#define B_PCH_P2SB_VBDF_DEV                        0x00F8
#define B_PCH_P2SB_VBDF_FUNC                       0x0007
#define R_PCH_P2SB_ESMBDF                          0x52
#define B_PCH_P2SB_ESMBDF_BUF                      0xFF00
#define B_PCH_P2SB_ESMBDF_DEV                      0x00F8
#define B_PCH_P2SB_ESMBDF_FUNC                     0x0007
#define R_PCH_P2SB_RCFG                            0x54
#define B_PCH_P2SB_RCFG_RPRID                      0x0000FF00
#define B_PCH_P2SB_RCFG_RSE                        BIT0       ///< RTC Shadow Enable
#define R_PCH_P2SB_HPTC                            0x60       ///< HPET Configuration
#define B_PCH_P2SB_HPTC_AE                         BIT7       ///< Address Enable
#define B_PCH_P2SB_HPTC_AS                         0x0003
#define N_PCH_HPET_ADDR_ASEL                       12
#define V_PCH_HPET_BASE0                           0xFED00000
#define V_PCH_HPET_BASE1                           0xFED01000
#define V_PCH_HPET_BASE2                           0xFED02000
#define V_PCH_HPET_BASE3                           0xFED03000
#define R_PCH_P2SB_IOAC                            0x64       ///< IOxAPIC Configuration
#define B_PCH_P2SB_IOAC_AE                         BIT8       ///< Address Enable
#define B_PCH_P2SB_IOAC_ASEL                       0x00FF
#define N_PCH_IO_APIC_ASEL                         12
#define R_PCH_IO_APIC_INDEX                        0xFEC00000
#define R_PCH_IO_APIC_DATA                         0xFEC00010
#define R_PCH_IO_APIC_EOI                          0xFEC00040
#define R_PCH_P2SB_IBDF                            0x6C       ///< IOAPIC BDF register
#define B_PCH_P2SB_IBDF_BUF                        0xFF00
#define B_PCH_P2SB_IBDF_DEV                        0x00F8
#define B_PCH_P2SB_IBDF_FUNC                       0x0007
#define R_PCH_P2SB_HBDF                            0x70       ///< HPET BDF register
#define B_PCH_P2SB_HBDF_BUF                        0xFF00
#define B_PCH_P2SB_HBDF_DEV                        0x00F8
#define B_PCH_P2SB_HBDF_FUNC                       0x0007
#define R_PCH_P2SB_80                              0x80
#define R_PCH_P2SB_84                              0x84
#define R_PCH_P2SB_88                              0x88
#define R_PCH_P2SB_8C                              0x8C
#define R_PCH_P2SB_90                              0x90
#define R_PCH_P2SB_94                              0x94
#define R_PCH_P2SB_98                              0x98
#define R_PCH_P2SB_9C                              0x9C
#define R_PCH_P2SB_DISPBDF                         0xA0    ///< Display BDF register (RAVDMs upstream/downstream ID)
#define B_PCH_P2SB_DISPBDF_DTBLK                   0x00070000
#define B_PCH_P2SB_DISPBDF_BUF                     0x0000FF00
#define B_PCH_P2SB_DISPBDF_DEV                     0x000000F8
#define B_PCH_P2SB_DISPBDF_FUNC                    0x00000007
#define R_PCH_P2SB_ICCOS                           0xA4
#define B_PCH_P2SB_ICCOS_MODBASE                   0xFF00
#define B_PCH_P2SB_ICCOS_BUFBASE                   0x00FF
#define R_PCH_P2SB_EPMASK0                         0xB0
#define R_PCH_P2SB_EPMASK1                         0xB4
#define R_PCH_P2SB_EPMASK2                         0xB8
#define R_PCH_P2SB_EPMASK3                         0xBC
#define R_PCH_P2SB_EPMASK4                         0xC0
#define R_PCH_P2SB_EPMASK5                         0xC4
#define R_PCH_P2SB_EPMASK6                         0xC8
#define R_PCH_P2SB_EPMASK7                         0xCC

// Definition for SBI
#define R_PCH_P2SB_SBIADDR                         0xD0
#define B_PCH_P2SB_SBIADDR_DESTID                  0xFF000000
#define B_PCH_P2SB_SBIADDR_RS                      0x000F0000
#define B_PCH_P2SB_SBIADDR_OFFSET                  0x0000FFFF
#define R_PCH_P2SB_SBIDATA                         0xD4
#define B_PCH_P2SB_SBIDATA_DATA                    0xFFFFFFFF
#define R_PCH_P2SB_SBISTAT                         0xD8
#define B_PCH_P2SB_SBISTAT_OPCODE                  0xFF00
#define B_PCH_P2SB_SBISTAT_POSTED                  BIT7
#define B_PCH_P2SB_SBISTAT_RESPONSE                0x0006
#define N_PCH_P2SB_SBISTAT_RESPONSE                1
#define B_PCH_P2SB_SBISTAT_INITRDY                 BIT0
#define R_PCH_P2SB_SBIRID                          0xDA
#define B_PCH_P2SB_SBIRID_FBE                      0xF000
#define B_PCH_P2SB_SBIRID_BAR                      0x0700
#define B_PCH_P2SB_SBIRID_FID                      0x00FF
#define R_PCH_P2SB_SBIEXTADDR                      0xDC
#define B_PCH_P2SB_SBIEXTADDR_ADDR                 0xFFFFFFFF

#define R_P2SB_P2SBC                               0xE0   ///< P2SB Control
#define B_P2SB_P2SBC_HIDE                          BIT8   ///< Hide Device
#define B_P2SB_P2SBC_PGCBCGE                       BIT16  ///< PGCB Clock Gating Enable
#define R_P2SB_PCE                                 0xE4   ///< Power Control Enable
#define B_P2SB_PCE_PMCRE                           BIT0   ///< PMC Request Enable
#define B_P2SB_PCE_SE                              BIT3   ///< Sleep Enable
#define B_P2SB_PCE_HAE                             BIT5   ///< Hardware Autonomous Enable
#define R_P2SB_PDOMAIN                             0xE8   ///< Primary Clock Domain Controls
#define B_P2SB_PDOMAIN_CGD                         BIT0   ///< Primary Clock Gating Disable
#define B_P2SB_PDOMAIN_CRD                         BIT1   ///< Primary Clock Request Disabled

#endif

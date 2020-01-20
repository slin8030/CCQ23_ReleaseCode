/** @file
  Register names for Low Power Audio device.

  Conventions:

  - Prefixes:
    Definitions beginning with "R_" are registers
    Definitions beginning with "B_" are bits within registers
    Definitions beginning with "V_" are meaningful values of bits within the registers
    Definitions beginning with "S_" are register sizes
    Definitions beginning with "N_" are the bit position
  - In general, SC registers are denoted by "_PCH_" in register names
  - Registers / bits that are different between SC generations are denoted by
    "_PCH_<generation_name>_" in register/bit names. e.g., "_PCH_CHV_"
  - Registers / bits that are different between SKUs are denoted by "_<SKU_name>"
    at the end of the register/bit names
  - Registers / bits of new devices introduced in a SC generation will be just named
    as "_PCH_" without <generation_name> inserted.

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
#ifndef _REGS_LPE_H_
#define _REGS_LPE_H_

///
/// LPE Config Registers (D21:F0)
///
#define PCI_DEVICE_NUMBER_LPE          14
#define PCI_FUNCTION_NUMBER_LPE        0

#define R_LPE_DEVVENID                 0x00  ///< Device / Vendor ID
#define B_LPE_DEVVENID_DEVICE_ID       0xFFFF0000 ///< Device ID
#define B_LPE_DEVVENID_VENDOR_ID       0x0000FFFF ///< Vendor ID
#define V_LPE_DEVVENID_VENDOR_ID       V_INTEL_VENDOR_ID ///< Intel Vendor ID
#define V_LPE_DEVICE_ID_0              0x0F28

#define R_LPE_STSCMD                   0x04  ///< Status Command
#define B_LPE_STSCMD_RMA               BIT29 ///< Received Master Abort
#define B_LPE_STSCMD_RCA               BIT28 ///< RCA
#define B_LPE_STSCMD_CAP_LST           BIT20 ///< Capabilities List
#define B_LPE_STSCMD_INTR_STS          BIT19 ///< Interrupt Status
#define B_LPE_STSCMD_INTR_DIS          BIT10 ///< Interrupt Disable
#define B_LPE_STSCMD_SERR_EN           BIT8  ///< SERR Enable
#define B_LPE_STSCMD_BME               BIT2  ///< Bus Master Enable
#define B_LPE_STSCMD_MSE               BIT1  ///< Memory Space Enable

#define R_LPE_RID_CC                   0x08  ///< Revision ID and Class Code
#define B_LPE_RID_CC_BCC               0xFF000000 ///< Base Class Code
#define B_LPE_RID_CC_SCC               0x00FF0000 ///< Sub Class Code
#define B_LPE_RID_CC_PI                0x0000FF00 ///< Programming Interface
#define B_LPE_RID_CC_RID               0x000000FF ///< Revision Identification

#define R_LPE_BAR0                     0x10  ///< BAR 0
#define B_LPE_BAR0_BA                  0xFFFFC000 ///< Base Address
#define V_LPE_BAR0_SIZE                0x4000
#define N_LPE_BAR0_ALIGNMENT           14
#define B_LPE_BAR0_PREF                BIT3  ///< Prefetchable
#define B_LPE_BAR0_ADDRNG              (BIT2 | BIT1) ///< Address Range
#define B_LPE_BAR0_SPTYP               BIT0  ///< Space Type (Memory)

#define R_LPE_BAR1                     0x18  ///< BAR 1
#define B_LPE_BAR1_BA                  0xFFFFF000 ///< Base Address
#define B_LPE_BAR1_PREF                BIT3  ///< Prefetchable
#define B_LPE_BAR1_ADDRNG              (BIT2 | BIT1) ///< Address Range
#define B_LPE_BAR1_SPTYP               BIT0  ///< Space Type (Memory)
#define V_LPE_BAR1_SIZE                (1 << 12)

#define R_LPE_SSID                     0x2C  ///< Sub System ID
#define B_LPE_SSID_SID                 0xFFFF0000 ///< Sub System ID
#define B_LPE_SSID_SVID                0x0000FFFF ///< Sub System Vendor ID

#define R_LPE_ERBAR                    0x30  ///< Expansion ROM BAR
#define B_LPE_ERBAR_BA                 0xFFFFFFFF ///< Expansion ROM Base Address

#define R_LPE_CAPPTR                   0x34  ///< Capability Pointer
#define B_LPE_CAPPTR_CPPWR             0xFF  ///< Capability Pointer Power

#define R_LPE_INTR                     0x3C  ///< Interrupt
#define B_LPE_INTR_ML                  0xFF000000 ///< Max Latency
#define B_LPE_INTR_MG                  0x00FF0000
#define B_LPE_INTR_IP                  0x00000F00 ///< Interrupt Pin
#define B_LPE_INTR_IL                  0x000000FF ///< Interrupt Line

#define R_LPE_PGC                      0x44
#define B_LPE_PGC_ADSPPGD              BIT2

#define R_LPE_PID                      0x50
#define B_LPE_PID_CAPPTR               0x00FF
#define N_LPE_PID_CAPPTR               8

#define R_LPE_PCS                      0x54  ///< PME Control Status
#define B_LPE_PCS_PMESTS               BIT15 ///< PME Status
#define B_LPE_PCS_PMEEN                BIT8  ///< PME Enable
#define B_LPE_PCS_NSS                  BIT3  ///< No Soft Reset
#define B_LPE_PCS_PS                   (BIT1 | BIT0) ///< Power State

#define B_LPE_VSID                     0x80

///
/// LPE Private Space
///
#define LPE_PORT_ID                    0x92  ///< LPE Private Space PortID
#define LPE_PRIVATE_READ_OPCODE        0x06  ///< CUnit to LPE Private Space Read Opcode
#define LPE_PRIVATE_WRITE_OPCODE       0x07  ///< CUnit to LPE Private Space Write Opcode

#define R_LPE_IOSFCTL                  0x0   ///< IOSF and Fabric Configuration Control

#define R_LPE_TTCCFG                   (R_LPE_IOSFCTL + 0xE4) ///< Transfer Traffic Class Configuration
#define R_LPE_TTCCFG_MMT               BIT4  ///< CSME Memory Transfers (MMT)
#define R_LPE_TTCCFG_HMT               BIT2  ///< Host Memory Transfers (HMT)

#define R_LPE_PCICFGCTL                0x500 ///< PCI and Codec Configuration Control
#define B_LPE_PCICFGCTL_SPCBAD         BIT7  ///< Shadowed PCI Configuration Base Address Disable
#define B_LPE_PCICFGCTL_ACPIIE         BIT1  ///< ACPI Interrupt Enable
#define B_LPE_PCICFGCTL_PCICD          BIT0  ///< PCI Configuration Space Disable

#define R_LPE_FNCFG                    (R_LPE_PCICFGCTL + 0x30) ///< Function Configuration
#define B_LPE_FNCFG_MASK               (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)
#define B_LPE_FNCFG_PGD                BIT5  ///< Power Gating Disable
#define B_LPE_FNCFG_BCLD               BIT4  ///< BIOS Configuration Lock Down
#define B_LPE_FNCFG_CGD                BIT3  ///< Clock Gating Disable
#define B_LPE_FNCFG_ADSPD              BIT2  ///< Audio DSP Disable
#define B_LPE_FNCFG_HDASPCID           BIT1  ///< HD Audio Sub System as PCI Device
#define B_LPE_FNCFG_HDASD              BIT0  ///< HD Audio Sub System Disable

#define R_LPE_PMCTL                    0x600 ///< Power Management and EBB Configuration Control

#define R_LPE_IOBCTL                   (R_LPE_PMCTL + 0x1C) ///< I/O Buffer Control
#define B_LPE_IOBCTL_OSEL              (BIT9 | BIT8)
#define V_LPE_IOBCTL_OSEL_HDALINK       0
#define V_LPE_IOBCTL_OSEL_HDALINK_I2S   1
#define V_LPE_IOBCTL_OSEL_I2S           3
#define N_LPE_IOBCTL_OSEL               8
#endif

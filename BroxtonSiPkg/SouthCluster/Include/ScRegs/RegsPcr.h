/** @file
  Register names for SC private chipset register

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
#ifndef _SC_REGS_PCR_H_
#define _SC_REGS_PCR_H_

///
/// Definition for PCR base address (defined in ScReservedResources.h)
///
//#define SC_PCR_BASE_ADDRESS            0xD0000000
//#define SC_PCR_MMIO_SIZE               0x01000000
/**
  Definition for PCR address
  The PCR address is used to the PCR MMIO programming
**/
#define SC_PCR_ADDRESS(Pid, Offset)    (SC_PCR_BASE_ADDRESS | ((UINT8)(Pid) << 16) | (UINT16)(Offset))

/**
  Definition for SBI PID
  The SC_SBI_PID defines the PID for PCR MMIO programming and PCH SBI programming as well.
**/
typedef enum {
  //Rsv                                 = 0xFE-0xFE,
  //Rsv                                 = 0xDC,
  PID_PSF4                              = 0xD5,
  PID_P2SB                              = 0xD4,
  PID_LPC                               = 0xD2,
  PID_RTC                               = 0xD1,
  PID_ITSS                              = 0xD0,
  PID_SMB                               = 0xCD,
  PID_SERIALIO                          = 0xCB,
  PID_GPIOW                             = 0xC7,
  PID_PSF3                              = 0xC6,
  PID_GPION                             = 0xC5,
  PID_GPIONW                            = 0xC4,
  //Rsv                                 = 0xC3,
  PID_GPIOS                             = 0xC2,
  //Rsv                                 = 0xC1,
  PID_GPIOSW                            = 0xC0,
  PID_FIA                               = 0xB0,
  PID_MODPHY1                           = 0xB1,  // SATA
  PID_PCIE0                             = 0xB3,
  PID_PCIE1                             = 0xB4,
  PID_SATA                              = 0xB5,
  PID_PCLKD                             = 0xB6,
  PID_MMP0_BXTP                         = 0xAF,
  PID_MMP0_BXT                          = 0xAB,
  PID_PSF1                              = 0xA9,
  PID_DCI                               = 0xA8,
  PID_MODPHY_USB2                       = 0xA7,
  PID_MODPHY0                           = 0xA5,  // USB3/PCIe
  PID_OTG                               = 0xA4,
  PID_XHCI                              = 0xA2,
  PID_PSF2                              = 0xA0,
  //Rsv                                 = 0x9A,
  PID_ISHBR                             = 0x98,
  PID_SPI                               = 0x93,
  PID_ISH                               = 0x94,
  PID_NPK                               = 0x88,
  //Rsv                                 = 0x86-85,
  PID_PMC                               = 0x82,
  PID_DRNG                              = 0x4E,
  PID_SVID                              = 0x44,
  PID_PSF0                              = 0x24,
  //Rsv                                 = 0x16-0x15,
  //Rsv                                 = 0x0C,
  //Rsv                                 = 0x00
} SC_SBI_PID;
#endif

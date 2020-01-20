/** @file
  This file include defination for ICH register.

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


#ifndef _ICHREGS_H_
#define _ICHREGS_H_

///
/// Definitions beginning with "R_" are registers
/// Definitions beginning with "B_" are bits within registers
/// Definitions beginning with "V_" are meaningful values of bits within the
///   registers
///

#define ICH_HPET_BASE_ADDRESS                   0xFED00000          ///< Program by McEnableHpet and restore by McDisableHPET
#define R_PCH_PCH_HPET_GCFG                     0x10
#define B_PCH_PCH_HPET_GCFG_LRE                 BIT1
#define B_PCH_PCH_HPET_GCFG_EN                  BIT0

#define PCI_BUS_NUMBER_ICH                      0x00

//
// PMC device (D13:F1)
//
#define PCI_DEVICE_NUMBER_PMC                   13
#define PCI_FUNCTION_NUMBER_PMC                 1
#define PMC_BAR_BASE_ADDRESS                    0x10
#define PMC_GCR_GEN_PMCON1                      0x20
#define PMC_GCR_GEN_PMCON1_DRAM_INIT_BIT_STS    BIT23
#define PMC_GCR_GEN_PMCON1_MEM_SR_BIT_STS       BIT21
#define PMC_GCR_GEN_PMCON1_GLOBAL_RESET_STS     BIT24
#define PMC_GCR_GEN_PMCON1_WARM_RESET_STS       BIT25
#define PMC_GCR_GEN_PMCON1_COLD_RESET_STS       BIT26
#define PMC_GCR_GEN_PMCON1_COLD_BOOT_STS        BIT27
#define PMC_GCR_BASE_ADDRESS                    0x1000

#define SC_ADDR_BASE                            0xFF040000
//#define GCR_BASE_ADDRESS                      (SC_ADDR_BASE+0x00003000) //(SC_ADDR_BASE+0x00027000)
#define IPC1_BASE_ADDRESS                       PcdGet32(PcdPmcIpc1BaseAddress0)
//#define GCR_BASE_ADDRESS                      (IPC1_BASE_ADDRESS + 0x1000)
#define ACPI_BASE_ADDRESS                       0x0400
#define PMC_BASEADDRESS                         PcdGet32(PcdPmcIpc1BaseAddress0)         // PMC Memory Base Address
// restore this later once the merge gets un-screwed-up
//#define ACPI_BASE_ADDRESS                       0x0400
//#define SC_ADDR_BASE                            0xFF040000
//#define GCR_BASE_ADDRESS                        PcdGet32(PcdPmcGcrBaseAddress)
//#define PMC_BASEADDRESS                         PcdGet32(PcdPmcGcrBaseAddress)

//
// LPC Bridge Registers(D31:F0)
//
#define PCI_DEVICE_NUMBER_ICH_LPC               31
#define PCI_FUNCTION_NUMBER_ICH_LPC             0

#define R_ICH_LPC_GEN_PMCON_2                   0xA2
#define B_ICH_LPC_GEN_PMCON_MIN_SLP_S4          0x04
#define B_ICH_LPC_GEN_PMCON_DRAM_INIT           0x80
#define R_ICH_LPC_GEN_PMCON_3                   0xA4
#define R_ICH_LPC_ETR3                          0xAC
#define B_ICH_LPC_ETR3_CF9GR                    (BIT20)
#define R_ICH_LPC_RCBA                          0xF0

#define ICH_RCRB_HPTC                           0x3404            ///< High Performance Timer Configuration
#define ICH_RCRB_HPTC_AS                        (BIT1+BIT0)       ///< Address selection
#define ICH_RCRB_HPTC_AE                        (BIT7)            ///< Address enable

#endif

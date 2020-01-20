;; @file
#   Chipset constants and macros
; @copyright
;  INTEL CONFIDENTIAL
;  Copyright 2012 - 2016 Intel Corporation.
;
;  The source code contained or described herein and all documents related to the
;  source code ("Material") are owned by Intel Corporation or its suppliers or
;  licensors. Title to the Material remains with Intel Corporation or its suppliers
;  and licensors. The Material may contain trade secrets and proprietary and
;  confidential information of Intel Corporation and its suppliers and licensors,
;  and is protected by worldwide copyright and trade secret laws and treaty
;  provisions. No part of the Material may be used, copied, reproduced, modified,
;  published, uploaded, posted, transmitted, distributed, or disclosed in any way
;  without Intel's prior express written permission.
;
;  No license under any patent, copyright, trade secret or other intellectual
;  property right is granted to or conferred upon you by disclosure or delivery
;  of the Materials, either expressly, by implication, inducement, estoppel or
;  otherwise. Any license under such intellectual property rights must be
;  express and approved by Intel in writing.
;
;  Unless otherwise agreed by Intel in writing, you may not remove or alter
;  this notice or any other notice embedded in Materials by Intel or
;  Intel's suppliers or licensors in any way.
;
;  This file contains a 'Sample Driver' and is licensed as such under the terms
;  of your license agreement with Intel or your vendor. This file may be modified
;  by the user, subject to the additional terms of the license agreement.
;
; @par Specification
;;

#
#------------------------------------------------------------------------------


#
# APIC register
#
.equ APICID,                        0xFEE00020

#
# Base addresses and register definitions.
#

.equ SC_ADDR_BASE,                  0xFF040000
.equ IPC1_BASE_ADDRESS,             0xFE042000
.equ GCR_BASE_ADDRESS,              (IPC1_BASE_ADDRESS + 0x1000)
.equ SSRAM_BASE_ADDRESS,            0xFE044000

.equ ACPI_BASE_ADDRESS,             0x0400
.equ   B_ACPI_BASE_EN,              BIT1
.equ   R_ACPI_PM1_CNT,              0x04
.equ   V_ACPI_PM1_CNT_S5,           0x1C00
.equ   R_ACPI_PM1_TMR,              0x008
.equ   R_TCO_STS,                   0x64
.equ   B_TCO_STS_SECOND_TO,         BIT17
.equ   R_TCO_CNT,                   0x68
.equ   B_TCO_CNT_TMR_HLT,           BIT11
.equ SMBUS_BASE_ADDRESS,            0xEFA0
.equ   B_SMBUS_PCICMD_IOSE,         BIT0
.equ   R_SMBUS_AUXC,                0x0D
.equ SPI_BASE_ADDRESS,              0xFED01000
#.equ   B_LPC_SPI_BASE_EN,           BIT1
.equ   R_SPI_BCR,                   0xDC
.equ PMC_BASE_ADDRESS,              GCR_BASE_ADDRESS
.equ   R_PMC_GEN_PMCON_1,           0x20
.equ   R_PMC_PMIR,                  0x48
.equ   B_PMC_PMIR_CF9GR,            BIT20
.equ PUNIT_BASE_ADDRESS,            0xFED06000
.equ   B_LPC_PUNIT_BASE_EN,         BIT1
.equ ILB_BASE_ADDRESS,              0xFED08000
.equ   B_LPC_ILB_BASE_EN,           BIT1
.equ RCBA_BASE_ADDRESS,             0xFED1C000
.equ   B_LPC_RCBA_EN,               BIT0
.equ IO_BASE_ADDRESS,               0xFED80000
.equ   B_LPC_IO_BASE_EN,            BIT1
.equ MPHY_BASE_ADDRESS,             0xFEA00000
.equ   B_LPC_MPHY_BASE_EN,          BIT1
.equ MCH_BASE_ADDRESS,              0xFED10000
.equ   B_MCH_BASE_ADDRESS_EN,       BIT0

#
# B_Unit Registers
#
.equ BUNIT_BMISC,                   0x6800
.equ   B_BMISC_RESDRAM,             0x01   # Bit 0 - When this bit is set, reads targeting E-segment are routed to DRAM.
.equ   B_BMISC_RFSDRAM,             0x02   # Bit 1 - When this bit is set, reads targeting F-segment are routed to DRAM.

#
# HPET compare register
#
.equ HPET_COMP_1,                   0xFED00108
.equ HPET_COMP_2,                   0xFED0010C
.equ HPET_COMP_3,                   0xFED00128
.equ HPET_COMP_4,                   0xFED00148
.equ HPTC_AE,                       BIT7

#
# MCH PCIe base address
#
.equ CPU_HEC_BASE,                  0xE0000000      # Must be X0000000
.equ CPU_HEC_SIZE,                  0x00000000      # 256M
.equ CPU_HEC_EN,                    0x00000001      # Enable
.equ CPU0_HEC_PCI_ADDR,             0x80FF0150
.equ CPU1_HEC_PCI_ADDR,             0x80FE0150

#
# PCI registers
#
.equ R_MCH_BASE,                    ((0x00 * 8 + 0x00) * 0x1000 + 0x0048 + CPU_HEC_BASE)
.equ R_LPC_ACPI_BASE,               ((0x1F * 8 + 0x00) * 0x1000 + 0x0040 + CPU_HEC_BASE)
.equ R_LPC_PMC_BASE,                ((0x1F * 8 + 0x00) * 0x1000 + 0x0044 + CPU_HEC_BASE)
.equ R_LPC_IO_BASE,                 ((0x1F * 8 + 0x00) * 0x1000 + 0x004C + CPU_HEC_BASE)
.equ R_LPC_ILB_BASE,                ((0x1F * 8 + 0x00) * 0x1000 + 0x0050 + CPU_HEC_BASE)
.equ R_LPC_SPI_BASE,                ((0x1F * 8 + 0x00) * 0x1000 + 0x0054 + CPU_HEC_BASE)
.equ R_LPC_MPHY_BASE,               ((0x1F * 8 + 0x00) * 0x1000 + 0x0058 + CPU_HEC_BASE)
.equ R_LPC_PUNIT_BASE,              ((0x1F * 8 + 0x00) * 0x1000 + 0x005C + CPU_HEC_BASE)
.equ R_LPC_RCBA,                    ((0x1F * 8 + 0x00) * 0x1000 + 0x00F0 + CPU_HEC_BASE)
.equ R_SMBUS_CMD,                   ((0x1F * 8 + 0x03) * 0x1000 + 0x0004 + CPU_HEC_BASE)
.equ R_SMBUS_BASE,                  ((0x1F * 8 + 0x03) * 0x1000 + 0x0020 + CPU_HEC_BASE)
.equ R_P2SB_HPTC,                   ((0x0D * 8 + 0x00) * 0x1000 + 0x0060 + CPU_HEC_BASE)  #HPTC config register. B0:D13:F0 0x60x
.equ R_PMC_PCI_CMD,                 ((0x0D * 8 + 0x01) * 0x1000 + 0x0004 + CPU_HEC_BASE)  
.equ R_PMC_ACPI_BASE,               ((0x0D * 8 + 0x01) * 0x1000 + 0x0020 + CPU_HEC_BASE)  #BAR2 IO config register. B0:D13:F1 0x20

.equ R_PMC_MMIO_BAR0,               ((0x0D * 8 + 0x01) * 0x1000 + 0x0010 + CPU_HEC_BASE)  #BAR0 IO config register. B0:D13:F1 0x10
.equ R_PMC_MMIO_BAR1,               ((0x0D * 8 + 0x01) * 0x1000 + 0x0018 + CPU_HEC_BASE)  #BAR1 IO config register. B0:D13:F1 0x18
.equ SYRE_CPURST,                   14

#
# PCIEXBAR constants for enable in bit [0]
#
.equ ENABLE,                        1

#
# PCIEXBAR constants for size in bit [2:1]
#
.equ PCIEXBAR_64MB,                 0x02
.equ PCIEXBAR_128MB,                0x01
.equ PCIEXBAR_256MB,                0x00

.equ MMCFG_BASE,                    CPU_HEC_BASE    # 4GB-128MB

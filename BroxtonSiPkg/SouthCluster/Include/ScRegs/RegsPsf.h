/** @file
  Register definition for PSF component

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
#ifndef _PCH_REGS_PSF_H_
#define _PCH_REGS_PSF_H_

//
// Private chipset register (Memory space) offset definition
// The PCR register defines is used for PCR MMIO programming and PCH SBI programming as well.
//

#define SB_PORTID_PSF0    0x24
#define SB_PORTID_PSF1    0xA9
#define SB_PORTID_PSF2    0xA0
#define SB_PORTID_PSF3    0xC6
#define SB_PORTID_PSF4    0xD5

//
// PSFx segment registers
//
#define R_PCH_PCR_PSF_GLOBAL_CONFIG                     0x4000  ///< PSF Global Configuration Register
#define B_PCH_PCR_PSF_GLOBAL_CONFIG_ENLCT               BIT3
#define R_PCH_PCR_PSF_ROOTSPACE_CONFIG_RS0              0x4010  ///< PSF Segment Rootspace Configuration Register
#define B_PCH_PCR_PSF_ROOTSPACE_CONFIG_RS0_ENADDRP2P    BIT1
#define B_PCH_PCR_PSF_ROOTSPACE_CONFIG_RS0_VTDEN        BIT0
#define R_PSF_CLKREQ_HYS                                0x4004  ///< Clkreq hysteresis count reload Register

//
// PSFX PCRs definitions
//
#define R_PCH_PCR_PSFX_T0_SHDW_BAR0                     0x0000                  ///< PCI BAR0
#define R_PCH_PCR_PSFX_T0_SHDW_BAR1                     0x0004                  ///< PCI BAR1
#define R_PCH_PCR_PSFX_T0_SHDW_BAR2                     0x0008                  ///< PCI BAR2
#define R_PCH_PCR_PSFX_T0_SHDW_BAR3                     0x000C                  ///< PCI BAR3
#define R_PCH_PCR_PSFX_T0_SHDW_BAR4                     0x0010                  ///< PCI BAR4

#define R_PCH_PCR_PSFX_T0_SHDW_PCIEN                    0x001C                  ///< PCI configuration space enable bits
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_ROMBARLOCK         BIT30                   ///< ROM bar lock, it's ReadOnly
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BARLOCK            0x3F000000              ///< Bar lock [29:34], it's ReadOnly
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_ROMBARDIS          BIT22                   ///< ROM bar disable
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BARDIS             0x003F0000              ///< Bar disable [21:16]
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR0DIS            BIT16                   ///< Disable BAR0
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR1DIS            BIT17                   ///< Disable BAR1
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR2DIS            BIT18                   ///< Disable BAR2
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR3DIS            BIT19                   ///< Disable BAR3
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR4DIS            BIT20                   ///< Disable BAR4
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR5DIS            BIT21                   ///< Disable BAR5
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_FUNDIS             BIT8                    ///< Function disable
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_MEMEN              BIT1                    ///< Memory decoding enable
#define B_PCH_PCR_PSFX_T0_SHDW_PCIEN_IOEN               BIT0                   ///< IO decoding enable

#define R_PCH_PCR_PSFX_T0_SHDW_PMCSR                    0x0020                  ///< PCI power management configuration
#define B_PCH_PCR_PSFX_T0_SHDW_PMCSR_PWRST              (BIT1 | BIT0)           ///< Power status
#define R_PCH_PCR_PSFX_T0_SHDW_CFG_DIS                  0x0038                  ///< PCI configuration disable
#define B_PCH_PCR_PSFX_T0_SHDW_CFG_DIS_CFGDIS           BIT0                    ///< config disable

#define R_PCH_PCR_PSFX_T1_SHDW_PCIEN                    0x003C                  ///< PCI configuration space enable bits
#define B_PCH_PCR_PSFX_T1_SHDW_PCIEN_ROMBARLOCK         BIT30                   ///< ROM bar lock, it's ReadOnly
#define B_PCH_PCR_PSFX_T1_SHDW_PCIEN_BARLOCK            0x3F000000              ///< Bar lock [29:34], it's ReadOnly
#define B_PCH_PCR_PSFX_T1_SHDW_PCIEN_ROMBARDIS          BIT22                   ///< ROM bar disable
#define B_PCH_PCR_PSFX_T1_SHDW_PCIEN_BARDIS             0x003F0000              ///< Bar disable [21:16]
#define B_PCH_PCR_PSFX_T1_SHDW_PCIEN_FUNDIS             BIT8                    ///< Function disable
#define B_PCH_PCR_PSFX_T1_SHDW_PCIEN_MEMEN              BIT1                    ///< Memory decoding enable
#define B_PCH_PCR_PSFX_T1_SHDW_PCIEN_IOEN               BIT0                    ///< IO decoding enable

#define B_PCH_PCR_PSFX_TX_AGENT_FUNCTION_CONFIG_DEVICE   0x01F0                 ///< device number
#define N_PCH_PCR_PSFX_TX_AGENT_FUNCTION_CONFIG_DEVICE   4
#define B_PCH_PCR_PSFX_TX_AGENT_FUNCTION_CONFIG_FUNCTION (BIT3 | BIT2 | BIT1)   ///< function number
#define N_PCH_PCR_PSFX_TX_AGENT_FUNCTION_CONFIG_FUNCTION 1

//
// PSF_1 PCRs (PID:PSF1)
//
#define R_SC_PCR_PSF1_T0_SHDW_NPK_ACPI_REG_BASE         0x0200                  ///< D0F5 PSF base address(NPK)
#define PSF_1_AGNT_T0_SHDW_CFG_DIS_NPK_RS0_D0_F5        0x38


#define R_SC_PCR_PSF1_T1_SHDW_PCIEN                     0x3C                    ///< PCI configuration space enable bits
#define B_SC_PCR_PSF1_T1_SHDW_PCIEN_FUNDIS              BIT8                    ///< Function disable

#define R_SC_PCR_PSF1_T0_SHDW_USB_XHCI_REG_BASE         0x0600                  ///< D21F0 PSF base address (XHCI)
#define R_SC_PCR_PSF1_T0_SHDW_USB_XDCI_REG_BASE         0x0700                  ///< D21F1 PSF base address (xDCI)
#define R_SC_PCR_PSF1_T0_SHDW_SATA_REG_BASE             0x0800                  ///< D18F0 PSF base address (SATA)

#define R_SC_PCR_PSF1_T1_SHDW_PCIE06_REG_BASE           0x2300                  ///< D19F3 PSF base address (PCIE PORT 06)
#define R_SC_PCR_PSF1_T1_SHDW_PCIE05_REG_BASE           0x2200                  ///< D19F2 PSF base address (PCIE PORT 05)
#define R_SC_PCR_PSF1_T1_SHDW_PCIE04_REG_BASE           0x2100                  ///< D19F1 PSF base address (PCIE PORT 04)
#define R_SC_PCR_PSF1_T1_SHDW_PCIE03_REG_BASE           0x2000                  ///< D19F0 PSF base address (PCIE PORT 03)
#define R_SC_BXTP_PCR_PSF1_T1_SHDW_PCIE02_REG_BASE      0x2500                  ///< D20F1 PSF base address (PCIE PORT 02)
#define R_SC_BXTP_PCR_PSF1_T1_SHDW_PCIE01_REG_BASE      0x2400                  ///< D20F0 PSF base address (PCIE PORT 01)
#define R_SC_BXT_PCR_PSF1_T1_SHDW_PCIE02_REG_BASE       0x2000                  ///< D20F1 PSF base address (PCIE PORT 02)
#define R_SC_BXT_PCR_PSF1_T1_SHDW_PCIE01_REG_BASE       0x2100                  ///< D20F0 PSF base address (PCIE PORT 01)

#define R_SC_PCR_PSF1_PORT_CONFIG_PG0_PORT0             0x4018  ///< PSF Segment Port Configuration Register

#define R_PCH_PCR_PSF1_T0_SHDW_GMM_PCIEN                0x041C                  ///< PCI configuration space enable bits (GMM)
#define B_PCH_PCR_PSF1_T0_SHDW_GMM_PCIEN_FUNDIS         BIT8                    ///< Function Disable

#define R_PCH_PCR_PSF1_T0_SHDW_PCIEN_CUNIT_RS0_D0_F1_CSE_BXTP     0x051C        ///< PCI configuration space enable bits (GMM)
#define B_PCH_PCR_PSF1_T0_SHDW_PCIEN_CUNIT_RS0_D0_F1_FUNDIS       BIT8          ///< Function Disable

//
// controls the PCI configuration header of a PCI function
//
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_VR_RS0_D23_F0    0x4158         ///< VR
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_KVM_RS0_D18_F0   0x415C         ///< KVM
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_PMT_RS0_D18_F2   0x4160         ///< PMT
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_CLINK_RS0_D18_F1 0x4164         ///< CLINK
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_PTIO_RS0_D22_F2  0x4168         ///< PTIO
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_PTIO_RS0_D22_F3  0x416C         ///< PTIO
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_CSE_RS0_D22_F0   0x4170         ///< CSE
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_CSE_RS0_D18_F3   0x4174         ///< CSE
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_CSE_RS0_D22_F1   0x4178         ///< CSE
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_CSE_RS0_D22_F4   0x417C         ///< CSE
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_CSE_RS0_D22_F2   0x4180         ///< CSE
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_CSE_RS0_D22_F7   0x4184         ///< CSE
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_CSE_RS0_D18_F4   0x4188         ///< CSE
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_CAM_RS0_D20_F3   0x418C         ///< CAM
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_GBE_RS0_D31_F6   0x4190         ///< GBE
#define R_PCH_PCR_PSF1_T0_AGENT_FUNCTION_CONFIG_GBE_RS1_D27_F0   0x4194         ///< GBE
#define R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D20_F0   0x40D8         ///< SPA
#define R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D20_F1   0x40D4         ///< SPA
#define R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D19_F0   0x40E8         ///< SPB
#define R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D19_F1   0x40E4         ///< SPB
#define R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D19_F2   0x40E0         ///< SPB
#define R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D19_F3   0x40DC         ///< SPB
#define R_PCH_PCR_PSF1_RS_IFR                                    0x41C8         ///< This register can be used to reset all functions in a particular Root Space simultaneously


//
// PSF_3 PCRs (PID:PSF3), Ref psf_bxt_rdl_1.05.pdf
//
#define R_PCH_PCR_PSF3_T0_SHDW_PMC_PMC_REG_BASE         0x0300                  ///< D13F1 PSF base address (PMC: PMC)
#define R_PCH_PCR_PSF3_T0_SHDW_PMC_PWM_REG_BASE         0x0400                  ///< D26F0 PSF base address (PMC: PWM)
#define R_PCH_PCR_PSF3_T0_SHDW_PMC_SSRAM_REG_BASE       0x0500                  ///< D13F3 PSF base address (PMC: SSRAM)

#define R_PCH_PCR_PSF3_T0_SHDW_SCS_SDIO_REG_BASE        0x0600                  ///< D30F0 PSF base address (SCS: SDIO)
#define R_PCH_PCR_PSF3_T0_SHDW_SCS_UFS_REG_BASE         0x0700                  ///< D29F0 PSF base address (SCS: UFS)
#define R_PCH_PCR_PSF3_T0_SHDW_SCS_EMMC_REG_BASE        0x0800                  ///< D28F0 PSF base address (SCS: EMMC)
#define R_PCH_PCR_PSF3_T0_SHDW_SCS_SDCARD_REG_BASE      0x0900                  ///< D27F0 PSF base address (SCS: SDCARD)

#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_D22F6_REG_BASE      0x0A00                  ///< D22F6 PSF base address (LPSS: D22F6)

#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_SPI2_REG_BASE       0x0B00                  ///< D25F2 PSF base address (LPSS: SPI2)
#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_SPI1_REG_BASE       0x0C00                  ///< D25F2 PSF base address (LPSS: SPI1)
#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_SPI0_REG_BASE       0x0D00                  ///< D25F2 PSF base address (LPSS: SPI0)

#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_UART3_REG_BASE      0x0E00                  ///< D24F2 PSF base address (LPSS: UART3)
#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_UART2_REG_BASE      0x0F00                  ///< D24F2 PSF base address (LPSS: UART2)
#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_UART1_REG_BASE      0x1000                  ///< D24F1 PSF base address (LPSS: UART1)
#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_UART0_REG_BASE      0x1100                  ///< D24F0 PSF base address (LPSS: UART0)

#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C7_REG_BASE       0x1200                  ///< D23F3 PSF base address (LPSS: I2C7)
#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C6_REG_BASE       0x1300                  ///< D23F2 PSF base address (LPSS: I2C6)
#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C5_REG_BASE       0x1400                  ///< D23F1 PSF base address (LPSS: I2C5)
#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C4_REG_BASE       0x1500                  ///< D23F0 PSF base address (LPSS: I2C4)

#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C3_REG_BASE       0x1600                  ///< D22F3 PSF base address (LPSS: I2C3)
#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C2_REG_BASE       0x1700                  ///< D22F2 PSF base address (LPSS: I2C2)
#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C1_REG_BASE       0x1800                  ///< D22F1 PSF base address (LPSS: I2C1)
#define R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C0_REG_BASE       0x1900                  ///< D22F0 PSF base address (LPSS: I2C0)

#define R_PCH_PCR_PSF3_T0_SHDW_CSE_HOFFL_REG_BASE       0x1A00                  ///< D16F0 PSF base address (CSE: HOFFL)

#define R_PCH_PCR_PSF3_T0_SHDW_CSE_HECI3_REG_BASE       0x1C00                  ///< D15F2 PSF base address (CSE: HECI3)
#define R_PCH_PCR_PSF3_T0_SHDW_CSE_HECI2_REG_BASE       0x1D00                  ///< D16F1 PSF base address (CSE: HECI2)
#define R_PCH_PCR_PSF3_T0_SHDW_CSE_D15F1_FUN_DIS        0x1D38
#define R_PCH_PCR_PSF3_T0_SHDW_CSE_D15F2_FUN_DIS        0x1c38

#define R_PCH_PCR_PSF3_T0_SHDW_CSE_HECI1_REG_BASE       0x1E00                  ///< D16F0 PSF base address (CSE: HECI1)

#define R_PCH_PCR_PSF3_T0_SHDW_AUDIO_PCIEN              0x201C                  ///< PCI configuration space enable bits (Audio)
#define B_PCH_PCR_PSF3_T0_SHDW_AUDIO_PCIEN_FUNDIS       BIT8                    ///< Function Disable

#define R_PCH_PCR_PSF3_T0_SHDW_ISH_PCIEN                0x211C                  ///< PCI configuration space enable bits (ISH)
#define B_PCH_PCR_PSF3_T0_SHDW_ISH_PCIEN_FUNDIS         BIT8                    ///< Function Disable

#define R_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_SPI_RS0_D13_F2  0x2338             ///< D13F2 PCI Configuration Disable (SPI)

#define R_PCH_PCR_PSF_3_AGNT_T0_SHDW_PCIEN_LPSS_RS0_D24_F2         0x0F1C       ///< PCI fabric decode enable bits (UART2)
#define B_PCH_PCR_PSF_3_AGNT_T0_SHDW_PCIEN_LPSS_RS0_D24_F2_FUNDIS    BIT8       ///< Function Disable

#define R_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2       0x0F38       ///< PCI configuration space enable bits (UART2)
#define B_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2_CFGDIS  BIT0       ///< Configuration Space Disable
#endif

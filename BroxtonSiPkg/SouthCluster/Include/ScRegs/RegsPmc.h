/** @file
  @brief
  Register names for PMC device.

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
  Copyright 2011 - 2016 Intel Corporation.

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
#ifndef _REGS_PMC_H_
#define _REGS_PMC_H_

///
/// PMC Registers
///
#define PMC_BASE_ADDRESS                    PcdGet32(PcdPmcIpc1BaseAddress0)
#define GCR_BASE_ADDRESS                    PcdGet32(PcdPmcGcrBaseAddress)

#define PCI_DEVICE_NUMBER_PMC               13
#define PCI_FUNCTION_NUMBER_PMC_SSRAM       3

///
///  PMC Controller Registers (D13:F1)
///
#define PCI_FUNCTION_NUMBER_PMC             1
#define R_PMC_BASE                          0x10  ///< BAR0
#define R_PMC_ACPI_BASE                     0x20  ///< BAR2
///
/// PMC Memory Space Registers (PBASE)
///
#define V_PMC_GCR_OFFSET                    0x1000

#define R_PMC_PRSTS                         0x1000  ///< Power and Reset Status
#define B_PMC_PRSTS_PRODID                  0xFF000000 ///< Power Management Controller Product ID
#define B_PMC_PRSTS_REVID                   0x00FF0000 ///< Power Management Controller Revision ID
#define B_PMC_PRSTS_PM_WD_TMR               BIT15 ///< PMC Watchdog Timer Status
#define B_PMC_PRSTS_SEC_GBLRST_STS          BIT7  ///< SEC Global Reset Status
#define B_PMC_PRSTS_SEC_WD_TMR_STS          BIT6  ///< SEC Watchdog Timer Status

#define R_PMC_PM_CFG                        0x1008 ///< Power Management Configuration
#define B_PMC_PM_CFG_SPS                    BIT5   ///< Shutdown Policy Select
#define B_PMC_PM_CFG_NO_REBOOT              BIT4   ///< No Reboot Strap
#define B_PMC_PM_CFG_SX_ENT_TO_EN           BIT3   ///< S1 / 3 / 4 / 5 Entry Timeout Enable

#define R_PMC_CFG2                          0x100C ///< Power Management Configuration 2
#define B_PMC_CFG2_PBOP                     (BIT31 | BIT30 | BIT29) ///< Power Button Override Period (PBOP)
#define N_PMC_CFG2_PBOP                     29     ///< Power Button Override Period (PBOP)
#define B_PMC_CFG2_PB_DIS                   BIT28  ///< Power Button Native Mode Disable (PB_DIS)
#define B_PMC_CFG2_PB_PWRBTN_DB_MODE        BIT10  ///< Power Button Debounce Mode

#define R_PMC_GEN_PMCON_1                   0x1020  ///< General PM Configuration 1
#define B_PMC_GEN_PMCON_COLD_BOOT_STS       BIT27   ///< Cold boot Status
#define B_PMC_GEN_PMCON_COLD_RST_STS        BIT26   ///< Cold Reset Status
#define B_PMC_GEN_PMCON_WARM_RST_STS        BIT25   ///< Warm Reset Status
#define B_PMC_GEN_PMCON_GLOBAL_RST_STS      BIT24   ///< Global Reset Status
#define B_PMC_GEN_PMCON_DRAM_INIT           BIT23   ///< DRAM Initialization Scratchpad Bit
#define B_PMC_GEN_PMCON_MEM_SR              BIT21   ///< Memory Placed in Self-Refresh
#define B_PMC_GEN_PMCON_SRS                 BIT20   ///< System Reset Status
#define B_PMC_GEN_PMCON_CTS                 BIT19   ///< CPU Thermal Trip Status
#define B_PMC_GEN_PMCON_MIN_SLP_S4          BIT18   ///< Minimum SLP_S4# Assertion Width Violation Status
#define B_PMC_GEN_PMCON_PWROK_FLR           BIT16   ///< PWROK Failure
#define B_PMC_GEN_PMCON_PME_B0_S5_DIS       BIT15   ///< PME B0 S5 Disable
#define B_PMC_GEN_PMCON_SUS_PWR_FLR         BIT14   ///< SUS Well Power Failure
#define B_PMC_GEN_PMCON_GEN_RST_STS         BIT9    ///< General Reset Status
#define B_PMC_GEN_PMCON_SWSMI_RTSL          (BIT7 | BIT6)  ///< SWSMI Rate Select
#define V_PMC_GEN_PMCON_SWSMI_RTSL_64MS     0xC0    ///< 64ms +/- 4ms
#define V_PMC_GEN_PMCON_SWSMI_RTSL_32MS     0x80    ///< 32ms +/- 4ms
#define V_PMC_GEN_PMCON_SWSMI_RTSL_16MS     0x40    ///< 16ms +/- 4ms
#define V_PMC_GEN_PMCON_SWSMI_RTSL_1_5MS    0x00    ///< 1.5ms +/- 0.6ms
#define B_PMC_GEN_PMCON_RTC_PWR_STS         BIT2    ///< RTC Power Status
#define B_PMC_GEN_PMCON_AFTERG3_EN          BIT0    ///< After G3 State Enable

#define R_PMC_GEN_PMCON_2                   0x1024  ///< General PM Configuration 2
#define B_PMC_GEN_PMCON_BIOS_PCI_EXP_EN     BIT10   ///< BIOS PCI Express Enable
#define B_PMC_GEN_PMCON_PWRBTN_LVL          BIT9    ///< Power Button Level
#define B_PMC_GEN_PMCON_SMI_LOCK            BIT4    ///< SMI Lock
#define B_PMC_GEN_PMCON_PER_SMI_SEL         (BIT1 | BIT0) ///< Period SMI Select
#define V_PMC_GEN_PMCON_PER_SMI_64S         0x0000  ///< 64 seconds
#define V_PMC_GEN_PMCON_PER_SMI_32S         0x0001  ///< 32 seconds
#define V_PMC_GEN_PMCON_PER_SMI_16S         0x0002  ///< 16 seconds
#define V_PMC_GEN_PMCON_PER_SMI_8S          0x0003  ///<  8 seconds

#define R_PMC_CRID                          0x1030  ///< Configured Revision ID
#define B_PMC_CRID_RID_SEL                  (BIT1 | BIT0) ///< Revision ID Select

#define R_PMC_FUNC_DIS                      0x1034 ///< Function Disable Register (FUNC_DIS_31_0)
#define B_PMC_FUNC_DIS_GMM                  BIT31  ///<31 GMM (GMM) Set by BIOS to inform PMC GMM is disabled.
#define B_PMC_FUNC_DIS_SPI                  BIT30  ///<30 SPI (SPI) Set by BIOS to inform PMC SPI is disabled.
                                                   ///<29 SSRAM (SSRAM) Set by BIOS to inform PMC SSRAM is disabled.
#define B_PMC_FUNC_DIS_AVS                  BIT28  ///<28 cAVS (cAVS) Set by BIOS to inform PMC cAVS is disabled.
                                                   ///<27 CSE_HECI1 (CSE_HECI1) Set by BIOS to inform PMC CSE_HECI1 is disabled.
                                                   ///<26 CSE_HECI2 (CSE_HECI2) Set by BIOS to inform PMC CSE_HECI2 is disabled.
                                                   ///<25 CSE_HECI3 (CSE_HECI3) Set by BIOS to inform PMC CSE_HECI3 is disabled.
#define B_PMC_FUNC_DIS_ISH                  BIT24  ///<24 ISH (ISH) Set by BIOS to inform PMC ISH is disabled.
#define B_PMC_FUNC_DIS_PCIE0_P0             BIT23  ///<23 Set by BIOS to inform PMC PCIE0 P0 is disabled.
#define B_PMC_FUNC_DIS_PCIE0_P1             BIT22  ///<22 Set by BIOS to inform PMC PCIE0 P1 is disabled.
#define B_PMC_FUNC_DIS_LPSS_I2C0            BIT21  ///<21 LPSS_I2C0 (LPSS_I2C0) Set by BIOS to inform PMC LPSS_I2C0 is disabled.
#define B_PMC_FUNC_DIS_LPSS_I2C1            BIT20  ///<20 LPSS_I2C1 (LPSS_I2C1) Set by BIOS to inform PMC LPSS_I2C1 is disabled.
#define B_PMC_FUNC_DIS_LPSS_I2C2            BIT19  ///<19 LPSS_I2C2 (LPSS_I2C2) Set by BIOS to inform PMC LPSS_I2C2 is disabled.
#define B_PMC_FUNC_DIS_LPSS_I2C3            BIT18  ///<18 LPSS_I2C3 (LPSS_I2C3) Set by BIOS to inform PMC LPSS_I2C3 is disabled.
#define B_PMC_FUNC_DIS_LPSS_I2C4            BIT17  ///<17 LPSS_I2C4 (LPSS_I2C4) Set by BIOS to inform PMC LPSS_I2C4 is disabled.
#define B_PMC_FUNC_DIS_LPSS_I2C5            BIT16  ///<16 LPSS_I2C5 (LPSS_I2C5) Set by BIOS to inform PMC LPSS_I2C5 is disabled.
#define B_PMC_FUNC_DIS_LPSS_I2C6            BIT15  ///<15 LPSS_I2C6 (LPSS_I2C6) Set by BIOS to inform PMC LPSS_I2C6 is disabled.
#define B_PMC_FUNC_DIS_LPSS_I2C7            BIT14  ///<14 LPSS_I2C7 (LPSS_I2C7) Set by BIOS to inform PMC LPSS_I2C7 is disabled.
#define B_PMC_FUNC_DIS_LPSS_UART0           BIT13  ///<13 LPSS_UART0 (LPSS_UART0) Set by BIOS to inform PMC LPSS_UART0 is disabled.
#define B_PMC_FUNC_DIS_LPSS_UART1           BIT12  ///<12 LPSS_UART1 (LPSS_UART1) Set by BIOS to inform PMC LPSS_UART1 is disabled.
#define B_PMC_FUNC_DIS_LPSS_UART2           BIT11  ///<11 LPSS_UART2 (LPSS_UART2) Set by BIOS to inform PMC LPSS_UART2 is disabled.
#define B_PMC_FUNC_DIS_LPSS_UART3           BIT10  ///<10 LPSS_UART3 (LPSS_UART3) Set by BIOS to inform PMC LPSS_UART3 is disabled.
#define B_PMC_FUNC_DIS_LPSS_SPI0            BIT9   ///< 9 LPSS_SPI0 (LPSS_SPI0) Set by BIOS to inform PMC LPSS_SPI0 is disabled.
#define B_PMC_FUNC_DIS_LPSS_SPI1            BIT8   ///< 8 LPSS_SPI1 (LPSS_SPI1) Set by BIOS to inform PMC LPSS_SPI1 is disabled.
#define B_PMC_FUNC_DIS_LPSS_SPI2            BIT7   ///< 7 LPSS_SPI2 (LPSS_SPI2) Set by BIOS to inform PMC LPSS_SPI2 is disabled.
#define B_PMC_FUNC_DIS_LPSS_PWM             BIT6   ///< 6 PWM (PWM) Set by BIOS to inform PMC PWM is disabled.
#define B_PMC_FUNC_DIS_SCC_SDCARD           BIT5   ///< 5 SCC_SDCARD (SCC_SDCARD) Set by BIOS to inform PMC SCC_SDCARD is disabled.
#define B_PMC_FUNC_DIS_SCC_SDIO             BIT4   ///< 4 SCC_SDIO (SCC_SDIO) Set by BIOS to inform PMC SCC_SDIO is disabled.
#define B_PMC_FUNC_DIS_SCC_EMMC             BIT3   ///< 3 SCC_eMMC (SCC_eMMC) Set by BIOS to inform PMC SCC_eMMC is disabled.
#define B_PMC_FUNC_DIS_SCC_UFS              BIT2   ///< 2 UFS (UFS) Set by BIOS to inform PMC UFS is disabled.
#define B_PMC_FUNC_DIS_USB_XHCI             BIT1   ///< 1 xHCI (xHCI) Set by BIOS to inform PMC xHCI is disabled.
#define B_PMC_FUNC_DIS_USB_XDCI             BIT0   ///< 0 xDCI (xDCI) Set by BIOS to inform PMC xDCI is disabled.
#define R_PMC_FUNC_DIS_1                    0x1038 ///< Function Disable Register 1 (FUNC_DIS_63_32)
#define B_PMC_FUNC_DIS_1_SATA               BIT9   ///< Set by BIOS to inform PMC SATA is Disable
#define B_PMC_FUNC_DIS_1_PCIE1_P3           BIT6   ///< Set by BIOS to inform PMC PCIE1 P3 is disabled.
#define B_PMC_FUNC_DIS_1_PCIE1_P2           BIT5   ///< Set by BIOS to inform PMC PCIE1 P2 is disabled.
#define B_PMC_FUNC_DIS_1_PCIE1_P1           BIT4   ///< Set by BIOS to inform PMC PCIE1 P1 is disabled.
#define B_PMC_FUNC_DIS_1_PCIE1_P0           BIT3   ///< Set by BIOS to inform PMC PCIE1 P0 is disabled.
#define B_PMC_FUNC_DIS_1_USF2               BIT2   ///< Set by BIOS to inform PMC USF 3rd lane Disable

#define R_PMC_ETR                           0x1048 ///< Extended Test Mode Register (ETR)
#define B_PMC_ETR_CF9LOCK                   BIT31  ///< CF9h Lockdown
#define B_PMC_ETR_CF9GR                     BIT20  ///< CF9h Global Reset
#define B_PMC_ETR_MAX_S0IX                  (BIT17 | BIT16) ///< Max S0ix State SoC Can Go

#define R_PMC_GPIO_GPE_CFG                  0x1050  ///GPIO GPE Config register

#define R_PMC_IRQ_SEL_0                     0x1064 ///< IRQ Select 0
#define N_PMC_IRQ_SEL_0_DIR_IRQ_UART3       24
#define N_PMC_IRQ_SEL_0_DIR_IRQ_UART2       16
#define N_PMC_IRQ_SEL_0_DIR_IRQ_UART1       8
#define N_PMC_IRQ_SEL_0_DIR_IRQ_UART0       0
#define R_PMC_IRQ_SEL_1                       0x1068 ///< IRQ Select 1
#define N_PMC_IRQ_SEL_1_DIR_IRQ_XHCI          24
#define N_PMC_IRQ_SEL_1_DIR_IRQ_XDCI          16
#define N_PMC_IRQ_SEL_1_DIR_IRQ_SDCARD_CD     8
#define N_PMC_IRQ_SEL_1_DIR_IRQ_SDCARD_D1     0
#define R_PMC_IRQ_SEL_2                       0x106C ///< IRQ Select 2
#define N_PMC_IRQ_SEL_2_AX_SCIS               29
#define N_PMC_IRQ_SEL_2_SCIS                  24
#define N_PMC_IRQ_SEL_2_AX_DIR_IRQ_SEL_PMIC   21
#define N_PMC_IRQ_SEL_2_DIR_IRQ_SEL_PMIC      16
#define V_PMC_IRQ_SEL_2_AX_SCI_IRQ_9           0                    ///< IRQ9 (A Stepping)
#define V_PMC_IRQ_SEL_2_SCI_IRQ_9              9                    ///< IRQ9
#define V_PMC_IRQ_SEL_2_SCI_IRQ_10             BIT0                 ///< IRQ10
#define V_PMC_IRQ_SEL_2_SCI_IRQ_11             BIT1                 ///< IRQ11
#define V_PMC_IRQ_SEL_2_SCI_IRQ_DIS            (BIT1 | BIT0)        ///< Routing Disabled
#define V_PMC_IRQ_SEL_2_SCI_IRQ_20             BIT2                 ///< IRQ20 (Only if APIC enabled)
#define V_PMC_IRQ_SEL_2_SCI_IRQ_21             (BIT2 | BIT0)        ///< IRQ21 (Only if APIC enabled)
#define V_PMC_IRQ_SEL_2_SCI_IRQ_22             (BIT2 | BIT1)        ///< IRQ22 (Only if APIC enabled)
#define V_PMC_IRQ_SEL_2_SCI_IRQ_23             (BIT2 | BIT1 | BIT0) ///< IRQ23 (Only if APIC enabled)

#define R_PMC_FUNC_ACPI_ENUM_0                0x1070
#define R_PMC_FUNC_ACPI_ENUM_1                0x1074
#define R_PMC_OBFF_DSPLAY_HPD_CTL             0x1094                ///< Display Hot Plug Control
#define R_PMC_OBFF_CTRL_STS                   0x10C8                ///< OBFF Control and Status
#define R_PMC_LOCK                            0x10CC                ///< Lock Register
#define B_PMC_LOCK_OBFF                       BIT12                 ///< OBFF Lock
#define B_PMC_LOCK_PWRBTN                     BIT11                 ///< Power Button Lock
#define B_PMC_LOCK_IRQ_SEL                    BIT8                  ///< IRQ Select Lock
#define B_PMC_LOCK_CRID                       BIT7                  ///< CRID Lock
#define B_PMC_LOCK_SX_WAKE                    BIT6                  ///< Sx Wake Lock
#define B_PMC_LOCK_PER_SMI                    BIT5                  ///< Periodic SMI Lock
#define B_PMC_LOCK_FUNC_DIS                   BIT3                  ///< Func Dis Lock
#define B_PMC_LOCK_S0IX                       BIT2                  ///< S0ix Lock

///
/// PMC PWM Modules
/// PCI Config Space Registers
///
#define PCI_DEVICE_NUMBER_PMC_PWM           26
#define PCI_FUNCTION_NUMBER_PMC_PWM         0

#define R_PMC_PWM_PMECTRLSTATUS             0x84  ///< PME Control Status
#define B_PMC_PWM_PMECTRLSTATUS_POWERSTATE  (BIT1 | BIT0) ///< Power State
#define R_PMC_PWM_D0I3MAXDEVPG              0xA0  ///< D0i3 Max Power On Latency and Device PG config

///
/// Memory Space Registers
///
#define R_PMC_PWM_MEM_RESETS                0x804 ///< Software Reset
#define B_PMC_PWM_MEM_RESETS_FUNC           BIT1  ///< Function Clock Domain Reset
#define B_PMC_PWM_MEM_RESETS_APB            BIT0  ///< APB Domain Reset

///
///  PMC IOSF2OCP Registers
///
#define R_PMC_IOSF2OCP_PCICFGCTRL1          0x200
#define R_PMC_IOSF2OCP_PCICFGCTRL2          0x204
#define R_PMC_IOSF2OCP_PCICFGCTRL3          0x208
#define B_PMC_IOSF2OCP_PCICFGCTRL3_BAR1_DISABLE3  BIT7

///
///ACPI and Legacy I/O Registers (ABASE)
///
#define R_ACPI_PM1_STS                        0x00  ///< Power Management 1 Status
#define S_ACPI_PM1_STS                        2
#define B_ACPI_PM1_STS_WAK                    BIT15 ///< Wake Status
#define B_ACPI_PM1_STS_WAK_PCIE0              BIT14 ///< PCI Express 0 Wake Status
#define B_ACPI_PM1_STS_USB_CLKLESS            BIT13 ///< USB Clockless Status
#define B_ACPI_PM1_STS_PRBTNOR                BIT11 ///< Power Button Override Status
#define B_ACPI_PM1_STS_RTC                    BIT10 ///< RTC Status
#define B_ACPI_PM1_STS_PWRBTN                 BIT8  ///< Power Button Status
#define B_ACPI_PM1_STS_GBL                    BIT5  ///< Global Status
#define B_ACPI_PM1_STS_WAK_PCIE3              BIT4  ///< PCI Express 3 Wake Status
#define B_ACPI_PM1_STS_WAK_PCIE2              BIT3  ///< PCI Express 2 Wake Status
#define B_ACPI_PM1_STS_WAK_PCIE1              BIT2  ///< PCI Express 1 Wake Status
#define B_ACPI_PM1_STS_TMROF                  BIT0  ///< Timer Overflow Status
#define N_ACPI_PM1_STS_WAK                    15
#define N_ACPI_PM1_STS_PRBTNOR                11
#define N_ACPI_PM1_STS_RTC                    10
#define N_ACPI_PM1_STS_PWRBTN                 8
#define N_ACPI_PM1_STS_GBL                    5
#define N_ACPI_PM1_STS_TMROF                  0

#define R_ACPI_PM1_EN                         0x02  ///< Power Management 1 Enables
#define S_ACPI_PM1_EN                         2
#define B_ACPI_PM1_WAK_DIS_PCIE0              BIT14 ///< PCI Express 0 Disable
#define B_ACPI_PM1_EN_USB_CLKLESS             BIT13 ///< USB Clockless Enable Bit
#define B_ACPI_PM1_EN_RTC                     BIT10 ///< RTC Alarm Enable Bit
#define B_ACPI_PM1_EN_PWRBTN                  BIT8  ///< Power Button Enable Bit
#define B_ACPI_PM1_EN_GBL                     BIT5  ///< Global Enable Bit
#define B_ACPI_PM1_WAK_DIS_PCIE3              BIT4  ///< PCI Express 3 Disable
#define B_ACPI_PM1_WAK_DIS_PCIE2              BIT3  ///< PCI Express 2 Disable
#define B_ACPI_PM1_WAK_DIS_PCIE1              BIT2  ///< PCI Express 1 Disable
#define B_ACPI_PM1_EN_TMROF                   BIT0  ///< Timer Overflow Interrupt Enable Bit
#define N_ACPI_PM1_EN_RTC                     10
#define N_ACPI_PM1_EN_PWRBTN                  8
#define N_ACPI_PM1_EN_GBL                     5
#define N_ACPI_PM1_EN_TMROF                   0

#define R_ACPI_PM1_CNT                        0x04  ///< Power Management 1 Control
#define S_ACPI_PM1_CNT                        4
#define B_ACPI_PM1_CNT_SLP_EN                 BIT13 ///< Sleep enable
#define B_ACPI_PM1_CNT_SLP_TYP                (BIT12 | BIT11 | BIT10) ///< Sleep Type
#define V_ACPI_PM1_CNT_S0                     0x00000000 ///< ON (S0)
#define V_ACPI_PM1_CNT_S1                     0x00000400 ///< Puts CPU in S1 state (S1)
#define V_ACPI_PM1_CNT_S3                     0x00001400 ///< Suspend-to-RAM (S3)
#define V_ACPI_PM1_CNT_S4                     0x00001800 ///< Suspend-to-Disk (S4)
#define V_ACPI_PM1_CNT_S5                     0x00001C00 ///< Soft Off (S5)
#define B_ACPI_PM1_CNT_GBL_RLS                BIT2
#define B_ACPI_PM1_CNT_BM_RLD                 BIT1  ///< Treated as Scratchpad Bit
#define B_ACPI_PM1_CNT_SCI_EN                 BIT0  ///< SCI Enable

#define R_ACPI_PM1_TMR                        0x08  ///< Power Management 1 Timer
#define B_ACPI_PM1_TMR_VAL                    0xFFFFFF ///< The timer value mask
#define V_ACPI_PM1_TMR_MAX_VAL                0x1000000 ///< The timer is 24 bit overflow
#define V_ACPI_PM1_TMR_FREQUENCY              3579545 ///< Timer Frequency
#define V_ACPI_PM1_TMR_NUM_BITS               24    ///< Programmed to 24 not 32
#define V_ACPI_PM1_TMR_MAX_BITS               32

#define R_ACPI_GPE0a_STS                      0x20  ///< General Purpose Event 0a Status
#define S_ACPI_GPE0a_STS                      4
#define B_ACPI_GPE0a_STS_CORE_GPIO            0xFF000000 ///< CORE GPIO Status
#define B_ACPI_GPE0a_STS_SUS_GPIO             0x00FF0000 ///< SUS GPIO Status
#define B_ACPI_GPE0a_STS_PME_B0               BIT13 ///< Power Management Event Bus 0 Status
#define B_ACPI_GPE0a_STS_BATLOW               BIT10 ///< Battery Low Status
#define B_ACPI_GPE0a_STS_PCI_EXP              BIT9  ///< PCI Express Status
#define B_ACPI_GPE0a_STS_GUNIT_SCI            BIT5  ///< GUNIT SCI Status
#define B_ACPI_GPE0a_STS_PUNIT_SCI            BIT4  ///< PUNIT SCI Status
#define B_ACPI_GPE0a_STS_SWGPE                BIT2  ///< Software GPE Status
#define B_ACPI_GPE0a_STS_HOT_PLUG             BIT1  ///< Hot Plug Status
#define N_ACPI_GPE0a_STS_PME_B0               13
#define N_ACPI_GPE0a_STS_BATLOW               10
#define N_ACPI_GPE0a_STS_PCI_EXP              9
#define N_ACPI_GPE0a_STS_GUNIT_SCI            5
#define N_ACPI_GPE0a_STS_PUNIT_SCI            4
#define N_ACPI_GPE0a_STS_SWGPE                2
#define N_ACPI_GPE0a_STS_HOT_PLUG             1

#define B_ACPI_GPE0b_EN_GPIO_TIER1_SCI_EN     BIT12
#define B_ACPI_GPEb_EN_WAKE                   BIT12 ///< GPIO12 Wake

#if (ENBDT_PF_ENABLE == 1)
#define R_ACPI_GPE0a_EN                       0x30  ///< General Purpose Event 0a Enables
#define R_ACPI_GPE0b_EN                       0x34  ///< General Purpose Event 0b Enables
#else
#define R_ACPI_GPE0a_EN                       0x28  ///< General Purpose Event 0a Enables
#endif
#define S_ACPI_GPE0a_EN                       4
#define B_ACPI_GPE0a_EN_CORE_GPIO             0xFF000000 ///< CORE GPIO Enable
#define B_ACPI_GPE0a_EN_SUS_GPIO              0x00FF0000 ///< SUS GPIO Enable
#define B_ACPI_GPE0a_EN_SATA_PME_EN           BIT17 ///< SATA PME Enable
#define B_ACPI_GPE0a_EN_SMB_WAK_EN            BIT16 ///< GPIO SMBUS WAKE Enable
#define B_ACPI_GPE0a_EN_GPIO_TIER1_SCI_EN     BIT15 ///< GPIO Tier1 SCI Enable
#define B_ACPI_GPE0a_EN_AVS_PME_EN            BIT14 ///< AVS PME Enable
#define B_ACPI_GPE0a_EN_XHCI_PME_EN           BIT13 ///< USB xHCI PME Wire Enable
#define B_ACPI_GPE0a_EN_XDCI_PME_EN           BIT12 ///< USB xDCI PME Wire Enable
#define B_ACPI_GPE0a_EN_CSE_PME_EN            BIT11 ///< CSE PME Message Enable
#define B_ACPI_GPE0a_EN_BATLOW_EN             BIT10 ///< Low Battery Enable
#define B_ACPI_GPE0a_EN_PCIE_GPE_EN           BIT9  ///< PCIE GPE Enable
#define B_ACPI_GPE0a_EN_PCIE_WAKE3_EN         BIT8  ///< PCI Express Wake3 Enable
#define B_ACPI_GPE0a_EN_PCIE_WAKE2_EN         BIT7  ///< PCI Express Wake2 Enable
#define B_ACPI_GPE0a_EN_PCIE_WAKE1_EN         BIT6  ///< PCI Express Wake1 Enable
#define B_ACPI_GPE0a_EN_PUNIT_SCI_EN          BIT4  ///< PUnit SCI Enable
#define B_ACPI_GPE0a_EN_PCIE_WAKE0_EN         BIT3  ///< PCI Express Wake0 Enable
#define B_ACPI_GPE0a_EN_SWGPE_EN              BIT2  ///< Software GPE Enable
#define B_ACPI_GPE0a_EN_PCIE_SCI_EN           BIT0  ///< PCIE SCI Message Enable
#define B_SMI_EN_MCSMI                        BIT12
#define B_SMI_STS_MCSMI                       BIT12

#define N_ACPI_GPE0a_EN_BATLOW                10
#define N_ACPI_GPE0a_EN_PCIE_PME              9
#define N_ACPI_GPE0a_EN_SWGPE                 2
#define N_ACPI_GPE0a_EN_HOT_PLUG              1
#define N_SMI_EN_MCSMI                        12
#define N_SMI_STS_MCSMI                       12

#if (ENBDT_PF_ENABLE == 1)
  #define R_SMI_EN                              0x40  ///< SMI Control and Enable
#else
  #define R_SMI_EN                              0x30  ///< SMI Control and Enable
#endif

#define S_SMI_EN                              4
#define B_SMI_EN_SPI_SSMI                     BIT25 ///< SPI Sync SMI Enable
#define B_SMI_EN_LEGACY_USB3                  BIT17 ///< Legacy USB 3 Enable
#define B_SMI_EN_PERIODIC                     BIT14 ///< Periodic Enable
#define B_SMI_EN_TCO                          BIT13 ///< TCO Enable
#define B_SMI_EN_GPIO_UNLOCK                  BIT11 ///< GPIO Unlock SMI EN
#define B_SMI_EN_GPIO                         BIT10 ///< GPIO Enable
#define B_SMI_EN_BIOS_RLS                     BIT7  ///< BIOS RLS
#define B_SMI_EN_SWSMI_TMR                    BIT6  ///< Software SMI Timer Enable
#define B_SMI_EN_APMC                         BIT5  ///< APMC Enable
#define B_SMI_EN_ON_SLP_EN                    BIT4  ///< SMI On Sleep Enable
#define B_SMI_EN_BIOS                         BIT2  ///< BIOS Enable
#define B_SMI_EN_EOS                          BIT1  ///< End of SMI
#define B_SMI_EN_GBL_SMI                      BIT0  ///< Global SMI Enable
#define N_SMI_EN_SPI_SSMI                     25
#define N_SMI_EN_LEGACY_USB3                  17
#define N_SMI_EN_PERIODIC                     14
#define N_SMI_EN_TCO                          13
#define N_SMI_EN_GPIO_UNLOCK                  11
#define N_SMI_EN_GPIO                         10
#define N_SMI_EN_BIOS_RLS                     7
#define N_SMI_EN_SWSMI_TMR                    6
#define N_SMI_EN_APMC                         5
#define N_SMI_EN_ON_SLP_EN                    4
#define N_SMI_EN_LEGACY_USB                   3
#define N_SMI_EN_BIOS                         2
#define N_SMI_EN_EOS                          1
#define N_SMI_EN_GBL_SMI                      0

#if (ENBDT_PF_ENABLE == 1)
#define R_SMI_STS                             0x44  ///< SMI Status Register
#else
#define R_SMI_STS                             0x34  ///< SMI Status Register
#endif
#define S_SMI_STS                             4
#define B_SMI_STS_SPI                         BIT26 ///< SPI SMI Status
#define B_SMI_STS_SPI_SSMI                    BIT25 ///< SPI Sync SMI Status
#define B_SMI_STS_PCI_EXP                     BIT20 ///< PCI Express SMI Status
#define B_SMI_STS_LEGACY_USB3                 BIT17 ///< Legacy USB 3 Status
#define B_SMI_STS_SERIRQ                      BIT15 ///< SERIRQ SMI Status
#define B_SMI_STS_PERIODIC                    BIT14 ///< Periodic Status
#define B_SMI_STS_TCO                         BIT13 ///< TCO Status
#define B_SMI_STS_GPIO_UNLOCK                 BIT11 ///< GPIO Unlock SMI EN
#define B_SMI_STS_GPIO                        BIT10 ///< GPIO Enable
#define B_SMI_STS_GPE0                        BIT9  ///< GPE0 Status
#define B_SMI_STS_PM1_STS_REG                 BIT8  ///< PM1 Status Register
#define B_SMI_STS_SWSMI_TMR                   BIT6  ///< Software SMI Timer Status
#define B_SMI_STS_APM                         BIT5  ///< APM Status
#define B_SMI_STS_ON_SLP_EN                   BIT4  ///< SMI On Sleep Enable Status
#define B_SMI_STS_BIOS                        BIT2  ///< BIOS Status
#define N_SMI_STS_SPI                         26
#define N_SMI_STS_SPI_SSMI                    25
#define N_SMI_STS_PCI_EXP                     20
#define N_SMI_STS_LEGACY_USB3                 17
#define N_SMI_STS_SERIRQ                      15
#define N_SMI_STS_PERIODIC                    14
#define N_SMI_STS_TCO                         13
#define N_SMI_STS_GPIO_UNLOCK                 11
#define N_SMI_STS_GPIO                        10
#define N_SMI_STS_GPE0                        9
#define N_SMI_STS_PM1_STS_REG                 8
#define N_SMI_STS_SWSMI_TMR                   6
#define N_SMI_STS_APM                         5
#define N_SMI_STS_ON_SLP_EN                   4
#define N_SMI_STS_LEGACY_USB                  3
#define N_SMI_STS_BIOS                        2

#define R_ALT_GP_SMI_EN                       0x38  ///< Alternate GPI SMI Enable
#define S_ALT_GP_SMI_EN                       2
#define B_ALT_GP_SMI_EN_CORE_GPIO             0xFF00 ///< SUS GPIO SMI Enable
#define B_ALT_GP_SMI_EN_SUS_GPIO              0x00FF ///< CORE GPIO SMI Enable

#define R_ALT_GP_SMI_STS                      0x3A  ///< Alternate GPI SMI Status
#define S_ALT_GP_SMI_STS                      2
#define B_ALT_GP_SMI_STS_CORE_GPIO            0xFF00 ///< SUS GPIO SMI Status
#define B_ALT_GP_SMI_STS_SUS_GPIO             0x00FF ///< CORE GPIO SMI Status

#if (ENBDT_PF_ENABLE == 1)
#define R_ACPI_GPE_CNTL                       0x50  ///< General Purpose Event Control
#else
#define R_ACPI_GPE_CNTL                       0x40  ///< General Purpose Event Control
#endif
#define B_ACPI_GPE_CNTL_SWGPE_CTRL            BIT17 ///< Software GPE Control
#define B_ACPI_GPE_CNTL_PCIE3_SCI_EN          BIT3
#define B_ACPI_GPE_CNTL_PCIE2_SCI_EN          BIT2
#define B_ACPI_GPE_CNTL_PCIE1_SCI_EN          BIT1
#define B_ACPI_GPE_CNTL_PCIE0_SCI_EN          BIT0

#define R_ACPI_PM2_CNT                        0x50  ///< PM2a Control Block
#define B_ACPI_PM2_CNT_ARB_DIS                BIT0  ///< Scratchpad Bit

///
///  PMC ACPI watchdog timer registers and Bits definitions, offset from ACPI IO base
///
#define R_TCO_RLD                             0x60  ///< TCO Reload
#define B_TCO_RLD_VAL_MASK                    0x3FF ///< TCO Timer Count Value
#define R_TCO_STS                             0x64  ///< TCO Timer Status
#define S_TCO_STS                             4
#define B_TCO_STS_SECOND_TO                   BIT17 ///< Second Timeout Status
#define B_TCO_STS_TIMEOUT                     BIT3  ///< Timeout
#define N_TCO_STS_TIMEOUT                     3
#define R_TCO1_CNT                            0x68  ///< TCO Control
#define S_TCO1_CNT                            2
#define B_TCO1_CNT_OS_POLICY                  (BIT21 | BIT20) ///< OS Policy
#define B_TCO1_CNT_LOCK                       BIT12 ///< TCO Enable Lock
#define B_TCO1_CNT_TMR_HLT                    BIT11 ///< TCO Timer Halt
#define R_TCO_TMR                             0x70  ///< TCO Timer
#define B_TCO_TMR_TCO_TRLD                    0x3FF0000

#define R_DIRECT_IRQ_EN                       0x7C  ///< Direct IRQ Enables
#define B_DIRECT_IRQ_EN_PMIC_EN               BIT9
#define B_DIRECT_IRQ_EN_XHCI_EN               BIT8
#define B_DIRECT_IRQ_EN_XDCI_EN               BIT7
#define B_DIRECT_IRQ_EN_SDIO_D1_EN            BIT6
#define B_DIRECT_IRQ_EN_SDCARD_CD_EN          BIT5
#define B_DIRECT_IRQ_EN_SDCARD_D1_EN          BIT4
#define B_DIRECT_IRQ_EN_UART3_EN              BIT3
#define B_DIRECT_IRQ_EN_UART2_EN              BIT2
#define B_DIRECT_IRQ_EN_UART1_EN              BIT1
#define B_DIRECT_IRQ_EN_UART0_EN              BIT0

#endif

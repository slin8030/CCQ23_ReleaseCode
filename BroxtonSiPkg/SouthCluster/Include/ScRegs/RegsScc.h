/** @file
  Register names for SCC module.

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
#ifndef _REGS_SCC_H_
#define _REGS_SCC_H_


///
/// SCC Modules Registers
///

///
/// SCC SDIO Modules
/// PCI Config Space Registers
///
#define PCI_DEVICE_NUMBER_SCC_SDCARD         27
#define PCI_DEVICE_NUMBER_SCC_EMMC           28
#define PCI_DEVICE_NUMBER_SCC_UFS            29
#define PCI_DEVICE_NUMBER_SCC_SDIO           30
#define PCI_FUNCTION_NUMBER_SCC_FUNC0         0

#define R_SCC_DEVVENDID                 0x00  ///< Device ID & Vendor ID
#define B_SCC_DEVVENDID_DID             0xFFFF0000 ///< Device ID
#define B_SCC_DEVVENDID_VID             0x0000FFFF ///< Vendor ID

#define V_SCC_EMMC_DEVICE_ID_0              0x0F50 ///< RVP
#define V_SCC_EMMC_DEVICE_ID_1              0x0ACC ///< BXT-A
#define V_SCC_EMMC_DEVICE_ID_2              0x5ACC ///< BXTP
#define V_SCC_EMMC_DEVICE_ID_3              0x1ACC ///< BXT-B

#define R_SCC_STSCMD                    0x04  ///< Status & Command
#define B_SCC_STSCMD_RMA                BIT29 ///< RMA
#define B_SCC_STSCMD_RCA                BIT28 ///< RCA
#define B_SCC_STSCMD_CAPLIST            BIT20 ///< Capability List
#define B_SCC_STSCMD_INTRSTS            BIT19 ///< Interrupt Status
#define B_SCC_STSCMD_INTRDIS            BIT10 ///< Interrupt Disable
#define B_SCC_STSCMD_SERREN             BIT8  ///< SERR# Enable
#define B_SCC_STSCMD_BME                BIT2  ///< Bus Master Enable
#define B_SCC_STSCMD_MSE                BIT1  ///< Memory Space Enable

#define R_SCC_REVCC                     0x08  ///< Revision ID & Class Code
#define B_SCC_REVCC_CC                  0xFFFFFF00 ///< Class Code
#define B_SCC_REVCC_RID                 0x000000FF ///< Revision ID

#define R_SCC_CLHB                      0x0C
#define B_SCC_CLHB_MULFNDEV             BIT23 ///< Multi Function Device
#define B_SCC_CLHB_HT                   0x007F0000 ///< Header Type
#define B_SCC_CLHB_LT                   0x0000FF00 ///< Latency Timer
#define B_SCC_CLHB_CLS                  0x000000FF ///< Cache Line Size

#define R_SCC_BAR                       0x10  ///< BAR
#define B_SCC_BAR_BA                    0xFFFFF000 ///< Base Address
#define V_SCC_BAR_SIZE                  0x1000
#define N_SCC_BAR_ALIGNMENT             12
#define B_SCC_BAR_SI                    0x00000FF0 ///< Size Indicator
#define B_SCC_BAR_PF                    BIT3  ///< Prefetchable
#define B_SCC_BAR_TYPE                  (BIT2 | BIT1) ///< Type
#define B_SCC_BAR_MS                    BIT0  ///< Message Space

#define R_SCC_BAR_HIGH                  0x14  ///< BAR High
#define B_SCC_BAR_HIGH_BA               0xFFFFFFFF ///< Base Address

#define R_SCC_BAR1                      0x18  ///< BAR 1
#define B_SCC_BAR1_BA                   0xFFFFF000 ///< Base Address
#define V_SCC_BAR1_SIZE                 0x1000
#define B_SCC_BAR1_SI                   0x00000FF0 ///< Size Indicator
#define B_SCC_BAR1_PF                   BIT3  ///< Prefetchable
#define B_SCC_BAR1_TYPE                 (BIT2 | BIT1) ///< Type
#define B_SCC_BAR1_MS                   BIT0  ///< Message Space

#define R_SCC_BAR1_HIGH                 0x1C  ///< BAR 1 High
#define B_SCC_BAR1_HIGH_BA              0xFFFFFFFF ///< Base Address

#define R_SCC_SSID                      0x2C  ///< Sub System ID
#define B_SCC_SSID_SID                  0xFFFF0000 ///< Sub System ID
#define B_SCC_SSID_SVID                 0x0000FFFF ///< Sub System Vendor ID

#define R_SCC_ERBAR                     0x30  ///< Expansion ROM BAR
#define B_SCC_ERBAR_BA                  0xFFFFFFFF ///< Expansion ROM Base Address

#define R_SCC_CAPPTR                    0x34  ///< Capability Pointer
#define B_SCC_CAPPTR_CPPWR              0xFF  ///< Capability Pointer Power

#define R_SCC_INTR                      0x3C  ///< Interrupt
#define B_SCC_INTR_ML                   0xFF000000 ///< Max Latency
#define B_SCC_INTR_MG                   0x00FF0000
#define B_SCC_INTR_IP                   0x00000F00 ///< Interrupt Pin
#define B_SCC_INTR_IL                   0x000000FF ///< Interrupt Line

#define R_SCC_PCAPID                    0x80  ///< Power Capability ID
#define B_SCC_PCAPID_PS                 0xF8000000 ///< PME Support
#define B_SCC_PCAPID_VS                 0x00070000 ///< Version
#define B_SCC_PCAPID_NC                 0x0000FF00 ///< Next Capability
#define B_SCC_PCAPID_PC                 0x000000FF ///< Power Capability

#define R_SCC_PCS                       0x84  ///< PME Control Status
#define B_SCC_PCS_PMESTS                BIT15 ///< PME Status
#define B_SCC_PCS_PMEEN                 BIT8  ///< PME Enable
#define B_SCC_PCS_NSS                   BIT3  ///< No Soft Reset
#define B_SCC_PCS_PS                    (BIT1 | BIT0) ///< Power State

#define R_SCC_MANID                     0xF8  ///< Manufacturer ID
#define B_SCC_MANID_MANID               0xFFFFFFFF ///< Manufacturer ID

#define R_SCC_D0I3MAXDEVPG              0x0A0 ///< D0i3 Max Power On Latency and Device PG config


///
/// SCC Devices MMIO Space Register
///
#define R_SCC_MEM_DMAADR                                    0x00
#define R_SCC_MEM_BLKSZ                                     0x04
#define R_SCC_MEM_BLKCNT                                    0x06
#define R_SCC_MEM_CMDARG                                    0x08
#define R_SCC_MEM_XFRMODE                                   0x0C
#define B_SCC_MEM_XFRMODE_DMA_EN                            BIT0
#define B_SCC_MEM_XFRMODE_BLKCNT_EN                         BIT1
#define B_SCC_MEM_XFRMODE_AUTOCMD_EN_MASK                   (BIT2 | BIT3)
#define V_SCC_MEM_XFRMODE_AUTOCMD12_EN                      1
#define B_SCC_MEM_XFRMODE_DATA_TRANS_DIR                    BIT4               ///< 1: Read (Card to Host), 0: Write (Host to Card)
#define B_SCC_MEM_XFRMODE_MULTI_SINGLE_BLK                  BIT5               ///< 1: Multiple Block, 0: Single Block
#define R_SCC_MEM_SDCMD                                     0x0E
#define B_SCC_MEM_SDCMD_RESP_TYPE_SEL_MASK                  (BIT0 | BIT1)
#define V_SCC_MEM_SDCMD_RESP_TYPE_SEL_NO_RESP               0
#define V_SCC_MEM_SDCMD_RESP_TYPE_SEL_RESP136               1
#define V_SCC_MEM_SDCMD_RESP_TYPE_SEL_RESP48                2
#define V_SCC_MEM_SDCMD_RESP_TYPE_SEL_RESP48_CHK            3
#define B_SCC_MEM_SDCMD_CMD_CRC_CHECK_EN                    BIT3
#define B_SCC_MEM_SDCMD_CMD_INDEX_CHECK_EN                  BIT4
#define B_SCC_MEM_SDCMD_DATA_PRESENT_SEL                    BIT5
#define R_SCC_MEM_RESP                                      0x10
#define R_SCC_MEM_BUFDATAPORT                               0x20
#define R_SCC_MEM_PSTATE                                    0x24
#define B_SCC_MEM_PSTATE_DAT0                               BIT20
#define R_SCC_MEM_PWRCTL                                    0x29
#define R_SCC_MEM_CLKCTL                                    0x2C
#define R_SCC_MEM_TIMEOUT_CTL                               0x2E               ///< Timeout Control
#define B_SCC_MEM_TIMEOUT_CTL_DTCV                          0x0F               ///< Data Timeout Counter Value
#define R_SCC_MEM_SWRST                                     0x2F
#define B_SCC_MEM_SWRST_CMDLINE                             BIT1
#define B_SCC_MEM_SWRST_DATALINE                            BIT2
#define R_SCC_MEM_NINTSTS                                   0x30
#define B_SCC_MEM_NINTSTS_MASK                              0xFFFF
#define B_SCC_MEM_NINTSTS_CLEAR_MASK                        0x60FF
#define B_SCC_MEM_NINTSTS_CMD_COMPLETE                      BIT0
#define B_SCC_MEM_NINTSTS_TRANSFER_COMPLETE                 BIT1
#define B_SCC_MEM_NINTSTS_DMA_INTERRUPT                     BIT3
#define B_SCC_MEM_NINTSTS_BUF_READ_READY_INTR               BIT5
#define R_SCC_MEM_ERINTSTS                                  0x32
#define B_SCC_MEM_ERINTSTS_MASK                             0x13FF
#define B_SCC_MEM_ERINTSTS_CLEAR_MASK                       0x13FF
#define R_SCC_MEM_NINTEN                                    0x34
#define B_SCC_MEM_NINTEN_MASK                               0x7FFF
#define R_SCC_MEM_ERINTEN                                   0x36
#define B_SCC_MEM_ERINTEN_MASK                              0x13FF
#define R_SCC_MEM_NINTSIGNEN                                0x38
#define B_SCC_MEM_NINTSIGNEN_MASK                           0x7FFF
#define R_SCC_MEM_ERINTSIGNEN                               0x3A
#define B_SCC_MEM_ERINTSIGNEN_MASK                          0x13FF
#define R_SCC_MEM_HOST_CTL2                                 0x3E
#define B_SCC_MEM_HOST_CTL2_MODE_MASK                       (BIT0 | BIT1 | BIT2)
#define V_SCC_MEM_HOST_CTL2_MODE_HS400                      5
#define V_SCC_MEM_HOST_CTL2_MODE_SDR104                     3
#define R_SCC_MEM_CESHC2                                    0x3C              ///< Auto CMD12 Error Status Register & Host Control 2
#define B_SCC_MEM_CESHC2_ASYNC_INT                          BIT30             ///< Asynchronous Interrupt Enable
#define R_SCC_MEM_CAP1                                      0x40
#define R_SCC_MEM_CAP2                                      0x44
#define B_SCC_MEM_CAP2_HS400_SUPPORT                        BIT31
#define B_SCC_MEM_CAP2_SDR104_SUPPORT                       BIT1

#define R_SCC_MEM_SW_LTR_VALUE                              0x804  ///< Software LTR Register
#define R_SCC_MEM_AUTO_LTR_VALUE                            0x808  ///< Auto LTR Value
#define R_SCC_MEM_CAP_BYPASS_CNTL                           0x810  ///< Capabilities Bypass Control
#define V_SCC_MEM_CAP_BYPASS_CNTL_EN                        0x5A
#define R_SCC_MEM_CAP_BYPASS_REG1                           0x814  ///< Capabilities Bypass Register 1
#define V_SCC_MEM_CAP_BYPASS_REG1_DEFAULTS                  0x3040EB1E
#define B_SCC_MEM_CAP_BYPASS_REG1_TIMEOUT_CLK_COUNT         (BIT27 | BIT26 | BIT25 | BIT24 | BIT23 | BIT22)
#define N_SCC_MEM_CAP_BYPASS_REG1_TIMEOUT_CLK_COUNT         22
#define V_SCC_MEM_CAP_BYPASS_REG1_TIMEOUT_CLK_COUNT         1
#define B_SCC_MEM_CAP_BYPASS_REG1_TIMER_COUNT               (BIT20 | BIT19 | BIT18 | BIT17)
#define N_SCC_MEM_CAP_BYPASS_REG1_TIMER_COUNT               17
#define V_SCC_MEM_CAP_BYPASS_REG1_TIMER_COUNT               0x8
#define B_SCC_MEM_CAP_BYPASS_REG1_3P3V_SUPPORT              BIT6
#define B_SCC_MEM_CAP_BYPASS_REG1_ASYNC_INTERRUPT           BIT10
#define B_SCC_MEM_CAP_BYPASS_REG1_SDR104                    BIT14
#define B_SCC_MEM_CAP_BYPASS_REG1_HS400                     BIT29

#define R_SCC_MEM_CAP_BYPASS_REG2                           0x818  ///< Capabilities Bypass Register 2
#define V_SCC_MEM_CAP_BYPASS_REG2_DEFAULTS                  0x040040C8
#define B_SCC_MEM_CAP_BYPASS_REG2_8BIT_SUPPORT              BIT14
#define R_SCC_MEM_IDLE_CTRL                                 0x81C  ///< DevIdle Control per SCC slice
#define R_SCC_MEM_TX_CMD_DLL_CNTL                           0x820  ///< Tx CMD Path Ctrl
#define R_SCC_MEM_TX_DATA_DLL_CNTL1                         0x824  ///< Tx Data Path Ctrl 1
#define R_SCC_MEM_TX_DATA_DLL_CNTL2                         0x828  ///< Tx Data Path Ctrl 2
#define R_SCC_MEM_RX_CMD_DATA_DLL_CNTL1                     0x82C  ///< Rx CMD&Data Path Ctrl 1
#define R_SCC_MEM_RX_STROBE_DLL_CNTL                        0x830  ///< Rx Strobe Ctrl Path
#define R_SCC_MEM_RX_CMD_DATA_DLL_CNTL2                     0x834  ///< Rx CMD&Data Path Ctrl 2
#define N_SCC_MEM_RX_CMD_DATA_DLL_CNTL2_CLKSRC_RX           16
#define V_SCC_MEM_RX_CMD_DATA_DLL_CNTL2_CLKSRC_RX_CLK_AUTO  0x2
#define R_SCC_MEM_MASTER_DLL_SW_CNTL                        0x838  ///< Master DLL Software Ctrl

///
/// SCC UFS Host Controller Regiser
///
#define R_SCC_UFS_HC_AHIT                   0x18 ///< Auto-Hibernate Idle Timer
#define B_SCC_UFS_HC_TIMER_SCALE           (BIT12 | BIT11 | BIT10)
#define V_SCC_UFS_HC_TIMER_SCALE_MS         0x00000C00
#define B_SCC_UFS_HC_AH8ITV                 0x000003FF
#define V_SCC_UFS_HC_AH8ITV_15              0xF
#define R_SCC_UFS_HC_STATUS                 0x0030  ///< UFS Host Controller Status
#define B_SCC_UFS_HC_HCS_DP                 BIT0
#define R_SCC_UFS_HC_ENABLE                 0x0034  ///< Host Controller Enable
#define B_SCC_UFS_HC_HCE_EN                 BIT0

///
///SCC UFS MPHY Register
///
#define R_SCC_UFS_MPHY_CMNREG4             0xF00C ///< Clock Gating Ctrl
#define B_SCC_UFS_MPHY_MAXHSCLK_CG_EN      BIT0   ///< MaxHSClk High speed trunk clock gate enable
#define B_SCC_UFS_MPHY_MAXPWMCLK_CG_EN     BIT1   ///< MaxPWMClk trunk clock gate enable
#define B_SCC_UFS_MPHY_PLL_REF_CG_EN       BIT2   ///< pll_ref_clk clock gate enable
#define B_SCC_UFS_MPHY_RMNI_REF_CG_EN      BIT3   ///< rmmi_ref_clk clock gate enable
#define B_SCC_UFS_MPHY_PLL_REF_T_CG_EN     BIT4   ///< pll_ref_clk trunk clock gate enable
#define B_SCC_UFS_MPHY_RMNI_REF_T_CG_EN    BIT5   ///< rmmi_cfg_clk trunk clock gate enable

#define R_SCC_UFS_MPHY_CMNREG20            0xF080 ///< Common lane Power Gating Ctrl
#define B_SCC_UFS_MPHY_COMMON_CORE_PG_EN   BIT0   ///< Common lane core power gating enable
#define B_SCC_UFS_MPHY_COMMON_SUS_PG_EN    BIT1   ///< Common lane sus power gating enable
#define R_SCC_UFS_MPHY_IMPREG8_L0          0x101C ///< Data lane 0 Power Gating Ctrl
#define R_SCC_UFS_MPHY_IMPREG8_L1          0x301C ///< Data lane 1 Power Gating Ctrl
#define B_SCC_UFS_MPHY_DATA_CORE_PG_EN     BIT24   ///< Common lane core power gating enable
#define B_SCC_UFS_MPHY_DATA_SUS_PG_EN      BIT25   ///< Common lane sus power gating enable
#define R_SCC_UFS_MPHY_CMNREG16            0xF03C ///< Common bank config update
#define R_SCC_UFS_MPHY_IMPREG22_Lane_0     0x1054 ///< Data lane 0 config update
#define R_SCC_UFS_MPHY_IMPREG22_Lane_1     0x3054 ///< Data lane 1 config update

///
/// SCC UFS Vendor Specific Register MMIO Space Register
///
#define R_SCC_UFS_MEM_BUSTHRTL              0xC0  ///< Bus Throttling
#define B_SCC_UFS_LP_AH8_PGE                BIT17
#define B_SCC_UFS_LP_PGE                    BIT16
#define R_SCC_UFS_MEM_OOCPR                 0xC4  ///< Out standing OCP Requests
#define R_SCC_UFS_MEM_CDACFG                0xD0  ///< C-Port Direct Access Configuration Register
#define R_SCC_UFS_MEM_CDATX1                0xD4  ///< C-Port Direct Access Transmit 1 Register
#define R_SCC_UFS_MEM_CDATX2                0xD8  ///< C-Port Direct Access Transmit 2 Register
#define R_SCC_UFS_MEM_CDARX1                0xDC  ///< C-Port Direct Access Receive 1 Register
#define R_SCC_UFS_MEM_CDARX2                0xE0  ///< C-Port Direct Access Receive 2 Register
#define R_SCC_UFS_MEM_CDASTA                0xE4  ///< C-Port Direct Access Status Register
#define R_SCC_UFS_MEM_LBMCFG                0xF0  ///< UPIU Loopback Configuration Register
#define R_SCC_UFS_MEM_LBMSTA                0xF4  ///< UPIU Loopback Status
#define R_SCC_UFS_MEM_HCLKDIV               0xFC  ///< HClk Divider

#define R_SCC_MEM_CUR_XFSM                  0x858  ///< Internal Clock Unit XFSM


///
/// SCC Private Sideband Registers
///

#define SCC_EP_PORT_ID                       0xD6  ///< SCC EP Private Space PortID
#define SCC_EP_READ_OPCODE                   0x06  ///< CUnit to SCC EP Private Space Read Opcode
#define SCC_EP_WRITE_OPCODE                  0x07  ///< CUnit to SCC EP Private Space Write Opcode

#define R_SCC_EP_IOSFCTL                     0x00  ///< IOSF Control
#define B_SCC_EP_IOSFCTL_NSNPDIS             BIT7  ///< Non-Snoop Disable

#define R_SCC_SB_PMCTL                       0x1D0 ///< PM Control Register for bridge
#define B_IOSFPRIMCLK_GATE_EN                BIT0
#define B_OCPCLK_GATE_EN                     BIT1
#define B_OCPCLK_TRUNK_GATE_EN               BIT2
#define B_IOSFSBCLK_GATE_EN                  BIT3
#define B_IOSFPRIM_TRUNK_GATE_EN             BIT4
#define B_IOSFSB_TRUNK_GATE_EN               BIT5

#define R_SCC_SB_PCICFGCTRL_EMMC             0x200
#define R_SCC_SB_PCICFGCTRL_SDIO             0x204
#define R_SCC_SB_PCICFGCTRL_SDCARD           0x208
#define R_SCC_SB_PCICFGCTRL_UFS              0x20C

#define B_SCC_PCICFGCTRL_PCI_CFG_DIS         BIT0
#define B_SCC_ACPI_INTR_EN                   BIT1
#define B_SCC_PCICFGCTRL_BAR1_DIS            BIT7

#define R_SCC_EP_PCICFGCTR1                  0x500 ///< PCI Configuration Control 1 - eMMC
#define B_SCC_EP_PCICFGCTR1_IPIN1            (BIT11 | BIT10 | BIT9 | BIT8) ///< Interrupt Pin
#define B_SCC_EP_PCICFGCTR1_B1D1             BIT7  ///< BAR 1 Disable
#define B_SCC_EP_PCICFGCTR1_PS               0x7C  ///< PME Support
#define B_SCC_EP_PCICFGCTR1_ACPI_INT_EN1     BIT1  ///< ACPI Interrupt Enable
#define B_SCC_EP_PCICFGCTR1_PCI_CFG_DIS1     BIT0  ///< PCI Configuration Space Disable

#define R_SCC_EP_PCICFGCTR2                  0x504 ///< PCI Configuration Control 2 - SD Card
#define B_SCC_EP_PCICFGCTR2_IPIN1            (BIT11 | BIT10 | BIT9 | BIT8) ///< Interrupt Pin
#define B_SCC_EP_PCICFGCTR2_B1D1             BIT7  ///< BAR 1 Disable
#define B_SCC_EP_PCICFGCTR2_PS               0x7C  ///< PME Support
#define B_SCC_EP_PCICFGCTR2_ACPI_INT_EN1     BIT1  ///< ACPI Interrupt Enable
#define B_SCC_EP_PCICFGCTR2_PCI_CFG_DIS1     BIT0  ///< PCI Configuration Space Disable

#define R_SCC_EP_PCICFGCTR3                  0x508 ///< PCI Configuration Control 3 - SDIO
#define B_SCC_EP_PCICFGCTR3_IPIN1            (BIT11 | BIT10 | BIT9 | BIT8) ///< Interrupt Pin
#define B_SCC_EP_PCICFGCTR3_B1D1             BIT7  ///< BAR 1 Disable
#define B_SCC_EP_PCICFGCTR3_PS               0x7C  ///< PME Support
#define B_SCC_EP_PCICFGCTR3_ACPI_INT_EN1     BIT1  ///< ACPI Interrupt Enable
#define B_SCC_EP_PCICFGCTR3_PCI_CFG_DIS1     BIT0  ///< PCI Configuration Space Disable

#define R_SCC_SB_GPPRVRW1                    0x600 ///<Dynamic Clock Gating Control (SR #10)
#define R_SCC_SB_GPPRVRW2                    0x604 ///Host Controller Disable(SR #11)
#define B_SCC_HOST_FUNC_DIS_UFS              BIT0  ///UFS Host Controller Disable
#define B_SCC_HOST_FUNC_DIS_EMMC             BIT1  ///EMMC Host Controller Disable
#define B_SCC_HOST_FUNC_DIS_SDIO             BIT2  ///SDIO Host Controller Disable
#define B_SCC_HOST_FUNC_DIS_SDCARD           BIT3  ///SDCARD Host Controller Disable



#define R_SCC_SB_GPPRVRW6                    0x614 ///<PLLCtrl Register

#endif

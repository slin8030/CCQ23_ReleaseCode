/** @file
  Register names for SC PCI-E root port devices

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
  Copyright 1999 - 2017 Intel Corporation.

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
#ifndef _PCH_REGS_PCIE_H_
#define _PCH_REGS_PCIE_H_

#define PCIE_MAX_ROOT_PORTS                            6

///
/// PCI Express Message Bus
///
#define PCIE_PHY_PORT_ID                               0xA6  ///< PCIe PHY Port ID
#define PCIE_PHY_MMIO_READ_OPCODE                      0x00  ///< CUnit to PCIe PHY MMIO Read Opcode
#define PCIE_PHY_MMIO_WRITE_OPCODE                     0x01  ///< CUnit to PCIe PHY MMIO Write Opcode

///
/// PCI Express Root Ports (PCIe 0: D20:F0~F1, PCIe1: D19:F0~F3)
///
#define PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1       20
#define PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_1     0
#define PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_2     1
#define PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2       19
#define PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_3     0
#define PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_4     1
#define PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_5     2
#define PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_6     3


#define V_PCH_PCIE_VENDOR_ID                      V_INTEL_VENDOR_ID

#define V_BXT_P_PCIE_DEVICE_ID_PORT1              0x5AD6  ///< PCI Express Root Port #1, BXT-P SC
#define V_BXT_P_PCIE_DEVICE_ID_PORT2              0x5AD7  ///< PCI Express Root Port #2, BXT-P SC
#define V_BXT_P_PCIE_DEVICE_ID_PORT3              0x5AD8  ///< PCI Express Root Port #3, BXT-P SC
#define V_BXT_P_PCIE_DEVICE_ID_PORT4              0x5AD9  ///< PCI Express Root Port #4, BXT-P SC
#define V_BXT_P_PCIE_DEVICE_ID_PORT5              0x5ADA  ///< PCI Express Root Port #5, BXT-P SC
#define V_BXT_P_PCIE_DEVICE_ID_PORT6              0x5ADB  ///< PCI Express Root Port #6, BXT-P SC
#define V_BXT_PCIE_DEVICE_ID_PORT1                0x1AD6  ///< PCI Express Root Port #1, BXT B0 SC
#define V_BXT_PCIE_DEVICE_ID_PORT2                0x1AD7  ///< PCI Express Root Port #2, BXT B0 SC
#define R_PCH_PCIE_CLIST                          0x40
#define R_PCH_PCIE_XCAP                           (R_PCH_PCIE_CLIST + R_PCIE_XCAP_OFFSET)
#define R_PCH_PCIE_DCAP                           (R_PCH_PCIE_CLIST + R_PCIE_DCAP_OFFSET)
#define R_PCH_PCIE_DCTL                           (R_PCH_PCIE_CLIST + R_PCIE_DCTL_OFFSET)
#define R_PCH_PCIE_LCAP                           (R_PCH_PCIE_CLIST + R_PCIE_LCAP_OFFSET)
#define R_PCH_PCIE_LCTL                           (R_PCH_PCIE_CLIST + R_PCIE_LCTL_OFFSET)
#define R_PCH_PCIE_LSTS                           (R_PCH_PCIE_CLIST + R_PCIE_LSTS_OFFSET)
#define R_PCH_PCIE_SLCAP                          (R_PCH_PCIE_CLIST + R_PCIE_SLCAP_OFFSET)
#define R_PCH_PCIE_SLCTL                          (R_PCH_PCIE_CLIST + R_PCIE_SLCTL_OFFSET)
#define R_PCH_PCIE_SLSTS                          (R_PCH_PCIE_CLIST + R_PCIE_SLSTS_OFFSET)
#define R_PCH_PCIE_RCTL                           (R_PCH_PCIE_CLIST + R_PCIE_RCTL_OFFSET)
#define R_PCH_PCIE_RSTS                           (R_PCH_PCIE_CLIST + R_PCIE_RSTS_OFFSET)
#define R_PCH_PCIE_DCAP2                          (R_PCH_PCIE_CLIST + R_PCIE_DCAP2_OFFSET)
#define R_PCH_PCIE_DCTL2                          (R_PCH_PCIE_CLIST + R_PCIE_DCTL2_OFFSET)
#define R_PCH_PCIE_LCTL2                          (R_PCH_PCIE_CLIST + R_PCIE_LCTL2_OFFSET)


#define R_PCIE_LCTL_LSTS                               0x50  ///< Link Control; Link Status
#define B_PCIE_LCTL_LSTS_LABS                          BIT31 ///< Link Autonomous Bandwidth Status
#define B_PCIE_LCTL_LSTS_LBMS                          BIT30 ///< Link Bandwidth Management Status
#define B_PCIE_LCTL_LSTS_DLLA                          BIT29 ///< Link Active
#define B_PCIE_LCTL_LSTS_SCC                           BIT28 ///< Slot Clock Configuration
#define B_PCIE_LCTL_LSTS_LT                            BIT27 ///< Link Training
#define B_PCIE_LCTL_LSTS_LTE                           BIT26 ///< Reserved, previously was Link Training Error
#define B_PCIE_LCTL_LSTS_NLW                           0x03F00000 ///< Negotiated Link Width
#define V_PCIE_LCTL_LSTS_NLW_1                         0x00100000
#define V_PCIE_LCTL_LSTS_NLW_2                         0x00200000
#define V_PCIE_LCTL_LSTS_NLW_4                         0x00400000
#define B_PCIE_LCTL_LSTS_LS                            0x000F0000 ///< Current Link Speed
#define B_PCIE_LCTL_LSTS_LABIE                         BIT11 ///< Link Autonomous Bandwidth Interrupt Enable
#define B_PCIE_LCTL_LSTS_LBMIE                         BIT10 ///< Link Bandwidth Management Interrupt Enable
#define B_PCIE_LCTL_LSTS_HAWD                          BIT9  ///< Hardware Autonomous Width Disable
#define B_PCIE_LCTL_LSTS_ES                            BIT7  ///< Extended Synch
#define B_PCIE_LCTL_LSTS_CCC                           BIT6  ///< Common Clock Configuration
#define B_PCIE_LCTL_LSTS_RL                            BIT5  ///< Retrain Link
#define B_PCIE_LCTL_LSTS_LD                            BIT4  ///< Link Disable
#define B_PCIE_LCTL_LSTS_RCBC                          BIT3  ///< Read Completion Boundary
#define B_PCIE_LCTL_LSTS_ASPM                          (BIT1 | BIT0) ///< Active State Link PM Control
#define V_PCIE_LCTL_LSTS_ASPM_L0S                      1     ///< L0s Entry Enabled
#define V_PCIE_LCTL_LSTS_ASPM_L1                       2     ///< L1 Entry Enable
#define V_PCIE_LCTL_LSTS_ASPM_L0S_L1                   3     ///< L0s and L1 Entry Enabled


#define R_PCIE_LCTL2_LSTS2                             0x70  ///< Link Control 2; Link Status 2
#define B_PCIE_LCTL2_LSTS2_CDL                         BIT16 ///< Current De-emphasis Level
#define B_PCIE_LCTL2_LSTS2_CD                          BIT12 ///< Compliance De-emphasis
#define B_PCIE_LCTL2_LSTS2_CSOS                        BIT11 ///< Compliance SOS
#define B_PCIE_LCTL2_LSTS2_EMC                         BIT10 ///< Enter Modified Compliance
#define B_PCIE_LCTL2_LSTS2_TM                          (BIT9 | BIT8 | BIT7) ///< Transmit Margin
#define B_PCIE_LCTL2_LSTS2_SD                          BIT6  ///< Selectable De-emphasis
#define B_PCIE_LCTL2_LSTS2_HASD                        BIT5  ///< Reserved. Hardware Autonomous Speed Disable
#define B_PCIE_LCTL2_LSTS2_EC                          BIT4  ///< Enter Compliance
#define B_PCIE_LCTL2_LSTS2_TLS                         (BIT3 | BIT2 | BIT1 | BIT0) ///< Target Link Speed


#define R_PCH_PCIE_MID                            0x80
#define S_PCH_PCIE_MID                            2
#define R_PCH_PCIE_MC                             0x82
#define S_PCH_PCIE_MC                             2
#define R_PCH_PCIE_MA                             0x84
#define S_PCH_PCIE_MA                             4
#define R_PCH_PCIE_MD                             0x88
#define S_PCH_PCIE_MD                             2

#define R_PCH_PCIE_SVCAP                          0x90
#define S_PCH_PCIE_SVCAP                          2
#define R_PCH_PCIE_SVID                           0x94
#define S_PCH_PCIE_SVID                           4

#define R_PCIE_SVID                                    0x94  ///< Subsystem Vendor IDs
#define S_PCIE_SVID                                    4
#define B_PCIE_SVID_SID                                0xFFFF0000 ///< Subsystem Identifier
#define B_PCIE_SVID_SVID                               0x0000FFFF ///< Subsystem Vendor Identifier

#define R_PCH_PCIE_PMCAP                          0xA0
#define R_PCH_PCIE_PMCS                           (R_PCH_PCIE_PMCAP + R_PCIE_PMCS_OFFST)

#define R_PCH_PCIE_CCFG                           0xD0
#define B_PCH_PCIE_CCFG_UMRPD                     BIT26
#define B_PCH_PCIE_CCFG_UPMWPD                    BIT25
#define B_PCH_PCIE_CCFG_UPSD                      BIT24
#define B_PCH_PCIE_CCFG_UNSD                      BIT23
#define B_PCH_PCIE_CCFG_DCGEISMA                  BIT15
#define B_PCH_PCIE_CCFG_UNRD                      (BIT13 | BIT12)
#define N_PCH_PCIE_CCFG_UNRD                      12
#define B_PCH_PCIE_CCFG_UNRS                      (BIT6 | BIT5 | BIT4)
#define B_PCH_PCIE_CCFG_UPRS                      (BIT2 | BIT1 | BIT0)

#define R_PCH_PCIE_MPC2                           0xD4
#define S_PCH_PCIE_MPC2                           4
#define B_PCH_PCIE_MPC2_PTNFAE                    BIT12
#define B_PCH_PCIE_MPC2_TLPF                      BIT9
#define B_PCH_PCIE_MPC2_LSTP                      BIT6
#define B_PCH_PCIE_MPC2_IEIME                     BIT5
#define B_PCH_PCIE_MPC2_ASPMCOEN                  BIT4
#define B_PCH_PCIE_MPC2_ASPMCO                    (BIT3 | BIT2)
#define V_PCH_PCIE_MPC2_ASPMCO_DISABLED           0
#define V_PCH_PCIE_MPC2_ASPMCO_L0S                1 << 2
#define V_PCH_PCIE_MPC2_ASPMCO_L1                 2 << 2
#define V_PCH_PCIE_MPC2_ASPMCO_L0S_L1             3 << 2
#define B_PCH_PCIE_MPC2_EOIFD                     BIT1

#define R_PCH_PCIE_MPC                            0xD8
#define S_PCH_PCIE_MPC                            4
#define B_PCH_PCIE_MPC_PMCE                       BIT31
#define B_PCH_PCIE_MPC_HPCE                       BIT30
#define B_PCH_PCIE_MPC_MMBNCE                     BIT27
#define B_PCH_PCIE_MPC_P8XDE                      BIT26
#define B_PCH_PCIE_MPC_IRRCE                      BIT25
#define B_PCH_PCIE_MPC_SRL                        BIT23
#define N_PCH_PCIE_MPC_UCEL                       18
#define B_PCH_PCIE_MPC_UCEL                       (BIT20 | BIT19 | BIT18)
#define N_PCH_PCIE_MPC_CCEL                       15
#define B_PCH_PCIE_MPC_CCEL                       (BIT17 | BIT16 | BIT15)
//[-start-170622-IB07400878-add]//
#define B_PCH_PCIE_MPC_PCIESD                     (BIT14 | BIT13)
#define N_PCH_PCIE_MPC_PCIESD                     13
#define V_PCH_PCIE_MPC_PCIESD_GEN1                1
#define V_PCH_PCIE_MPC_PCIESD_GEN2                2
//[-end-170622-IB07400878-add]//
#define B_PCH_PCIE_MPC_MCTPSE                     BIT3
#define B_PCH_PCIE_MPC_HPME                       BIT1
#define N_PCH_PCIE_MPC_HPME                       1
#define B_PCH_PCIE_MPC_PMME                       BIT0

#define R_PCIE_MPC2                                    0xD4  ///< Miscellaneous Port Configuration 2
#define S_PCIE_MPC2                                    4
#define B_PCIE_MPC2_PTNFAE                             BIT12 ///< Poisoned TLP Non-Fatal Advisory Error Enable
#define B_PCIE_MPC2_IPF                                BIT11 ///< IOSF Packet Fast Transmit Mode
#define B_PCIE_MPC2_ITCUM                              BIT10 ///< IOSF Transaction Credit Update Mode
#define B_PCIE_MPC2_TLPF                               BIT9  ///< Transaction Layer Packet Fast Transmit Mode
#define B_PCIE_MPC2_CAM                                BIT8  ///< PCIe Credit Allocated Update Mode
#define B_PCIE_MPC2_CDRM                               BIT7  ///< COM De-Skew Recovery Mechanism
#define B_PCIE_MPC2_LSTP                               BIT6  ///< Link Speed Training Policy
#define B_PCIE_MPC2_IEIME                              BIT5  ///< Infer Electrical Idle Mechanism Enable
#define B_PCIE_MPC2_ASPMCOEN                           BIT4  ///< ASPM Control Override Enable
#define B_PCIE_MPC2_ASPMCO                             (BIT3 | BIT2) ///< APSM Control OverrideEOI Forwarding Disable
#define V_PCIE_MPC2_ASPMCO_DISABLED                    0     ///< Disabled
#define V_PCIE_MPC2_ASPMCO_L0S                         (1 << 2) ///< L0s Entry Enabled
#define V_PCIE_MPC2_ASPMCO_L1                          (2 << 2) ///< L1 Entry Enabled
#define V_PCIE_MPC2_ASPMCO_L0S_L1                      (3 << 2) ///< L0s and L1 Entry Enabled
#define B_PCIE_MPC2_EOIFD                              BIT1  ///< EOI Forwarding Disable
#define B_PCIE_MPC2_L1CTM                              BIT0  ///< L1 Completion Timeout Mode

#define R_PCIE_MPC                                     0xD8  ///< Miscellaneous Port Configuration
#define S_PCIE_MPC                                     4
#define B_PCIE_MPC_PMCE                                BIT31 ///< Power Management SCI Enable
#define B_PCIE_MPC_HPCE                                BIT30 ///< Hot Plug SCI Enable
#define B_PCIE_MPC_LHO                                 BIT29 ///< Link Hold Off
#define B_PCIE_MPC_ATE                                 BIT28 ///< Address Translator Enable
#define B_PCIE_MPC_MMBNCE                              BIT27 ///< MCTP MEssage Bus Number Check Enable
#define B_PCIE_MPC_IRBNCE                              BIT26 ///< Invalid Received Bus Number Check Enable
#define B_PCIE_MPC_IRRCE                               BIT25 ///< Invalid Received Range Check Enable
#define B_PCIE_MPC_BMERCE                              BIT24 ///< BME Received Check Enable
#define B_PCIE_MPC_SRL                                 BIT23 ///< Secured Register Lock
#define B_PCIE_MPC_FORCEDET                            BIT22 ///< Detect Override
#define B_PCIE_MPC_FCDL1E                              BIT21 ///< Flow Control During L1 Entry
#define B_PCIE_MPC_UCEL                                (BIT20 | BIT19 | BIT18) ///< Unique Clock Exit Latency
#define B_PCIE_MPC_CCEL                                (BIT17 | BIT16 | BIT15) ///< Common Clock Exit Latency
#define N_PCIE_MPC_CCEL                                15
#define B_PCIE_MPC_PCIESD                              (BIT14 | BIT13)
#define N_PCIE_MPC_PCIESD                              13
#define V_PCIE_MPC_PCIESD_GEN1                         1
#define V_PCIE_MPC_PCIESD_GEN2                         2
#define B_PCIE_MPC_RATS                                BIT12 ///< Reject ATS
#define B_PCIE_MPC_AT                                  (BIT11 | BIT10 | BIT9 | BIT8) ///< Address Translator
#define B_PCIE_MPC_PAE                                 BIT7  ///< Port I/OxApic Enable
#define B_PCIE_MPC_FCP                                 (BIT6 | BIT5 | BIT4) ///< Flow Control Update Policy
#define B_PCIE_MPC_MCTPSE                              BIT3  ///< MCTP Support Enable
#define B_PCIE_MPC_BT                                  BIT2  ///< Bridge Type
#define B_PCIE_MPC_HPME                                BIT1  ///< Hot Plug SMI Enable
#define N_PCIE_MPC_HPME                                1
#define B_PCIE_MPC_PMME                                BIT0  ///< Power Management SMI Enable

#define R_PCIE_SMSCS                                   0xDC  ///< SMI / SCI Status
#define S_PCIE_SMSCS                                   4
#define B_PCIE_SMSCS_PMCS                              BIT31 ///< Power Management SCI Status
#define B_PCIE_SMSCS_HPCS                              BIT30 ///< Hot Plug SCI Status
#define B_PCIE_SMSCS_HPLAS                             BIT4  ///< Hot Plug Link Active State Changed SMI Status
#define N_PCIE_SMSCS_HPLAS                             4
#define B_PCIE_SMSCS_HPCCM                             BIT3  ///< Hot Plug Command Completed SMI Status
#define B_PCIE_SMSCS_HPABM                             BIT2  ///< Hot Plug Attention Button SMI Status
#define B_PCIE_SMSCS_HPPDM                             BIT1  ///< Hot Plug Presence Detect SMI Status
#define N_PCIE_SMSCS_HPPDM                             1
#define B_PCIE_SMSCS_PMMS                              BIT0  ///< Power Management SMI Status


#define R_PCH_PCIE_SMSCS                          0xDC
#define S_PCH_PCIE_SMSCS                          4
#define N_PCH_PCIE_SMSCS_LERSMIS                  5
#define N_PCH_PCIE_SMSCS_HPLAS                    4
#define N_PCH_PCIE_SMSCS_HPPDM                    1

#define R_PCH_PCIE_RPDCGEN                        0xE1
#define S_PCH_PCIE_RPDCGEN                        1
#define B_PCH_PCIE_RPDCGEN_RPSCGEN                BIT7
#define B_PCH_PCIE_RPDCGEN_PTOCGE                 BIT6
#define B_PCH_PCIE_RPDCGEN_LCLKREQEN              BIT5
#define B_PCH_PCIE_RPDCGEN_BBCLKREQEN             BIT4
#define B_PCH_PCIE_RPDCGEN_SRDLCGEN               BIT3
#define B_PCH_PCIE_RPDCGEN_SRDBCGEN               BIT2
#define B_PCH_PCIE_RPDCGEN_RPDLCGEN               BIT1
#define B_PCH_PCIE_RPDCGEN_RPDBCGEN               BIT0

#define R_PCH_PCIE_RPPGEN                         0xE2
#define B_PCH_PCIE_RPPGEN_PTOTOP                  BIT6
#define B_PCH_PCIE_RPPGEN_SEOSCGE                 BIT4

#define R_PCH_PCIE_PWRCTL                         0xE8
#define B_PCH_PCIE_PWRCTL_WPDMPGEP                BIT17
#define B_PCH_PCIE_PWRCTL_DLP                     BIT16
#define B_PCH_PCIE_PWRCTL_DBUPI                   BIT15
#define B_PCH_PCIE_PWRCTL_TXSWING                 BIT13
#define B_PCH_PCIE_PWRCTL_RPL1SQPOL               BIT1
#define B_PCH_PCIE_PWRCTL_RPDTSQPOL               BIT0

#define R_PCH_PCIE_DC                             0xEC
#define B_PCH_PCIE_DC_PCIBEM                      BIT2


#define R_PCH_PCIE_IPCS                           0xF0
#define B_PCH_PCIE_IPCS_IMPS                      (BIT10 | BIT9 | BIT8)
#define N_PCH_PCIE_IPCS_IMPS                      8
#define B_PCH_PCIE_IPCS_IMRS                      (BIT6 | BIT5 | BIT4)
#define N_PCH_PCIE_IPCS_IMRS                      4
#define V_PCIE_IMPS_64B_MAX_PAYLOAD               7

#define R_PCH_PCIE_F4                             0xF4

#define R_PCH_PCIE_PHYCTL2                        0xF5
#define B_PCH_PCIE_PHYCTL2_TDFT                   (BIT7 | BIT6)
#define B_PCH_PCIE_PHYCTL2_TXCFGCHGWAIT           (BIT5 | BIT4)
#define B_PCH_PCIE_PHYCTL2_PXPPLLWAIT             (BIT3 | BIT2 | BIT1)
#define V_PCH_PCIE_PHYCTL2_PXPPLLWAIT_5uS         (~BIT3 | ~BIT2 | BIT1)
#define B_PCH_PCIE_PHYCTL2_BLKPLLEN               BIT2
#define B_PCH_PCIE_PHYCTL2_PXPG3PLLOFFEN          BIT1
#define B_PCH_PCIE_PHYCTL2_PXPG2PLLOFFEN          BIT0

#define R_PCH_PCIE_IOSFSBCS                       0xF7
#define B_PCH_PCIE_IOSFSBCS_SCPTCGE               BIT6
#define B_PCH_PCIE_IOSFSBCS_SIID                  (BIT3 | BIT2)

#define R_PCH_PCIE_STRPFUSECFG                    0xFC
#define B_PCH_PCIE_STRPFUSECFG_PXIP               (BIT27 | BIT26 | BIT25 | BIT24)
#define N_PCH_PCIE_STRPFUSECFG_PXIP               24
#define B_PCH_PCIE_STRPFUSECFG_RPC                (BIT15 | BIT14)
#define V_PCH_PCIE_STRPFUSECFG_RPC_1_1_1_1        0
#define V_PCH_PCIE_STRPFUSECFG_RPC_2_1_1          1
#define V_PCH_PCIE_STRPFUSECFG_RPC_2_2            2
#define V_PCH_PCIE_STRPFUSECFG_RPC_4              3
#define N_PCH_PCIE_STRPFUSECFG_RPC                14
#define B_PCH_PCIE_STRPFUSECFG_mPHYIOPMDIS        (BIT9)
#define B_PCH_PCIE_STRPFUSECFG_PLLSHTDWNDIS       (BIT8)
#define B_PCH_PCIE_STRPFUSECFG_STPGATEDIS         (BIT7)
#define B_PCH_PCIE_STRPFUSECFG_ASPMDIS            (BIT6)
#define B_PCH_PCIE_STRPFUSECFG_LDCGDIS            (BIT5)
#define B_PCH_PCIE_STRPFUSECFG_LTCGDIS            (BIT4)
#define B_PCH_PCIE_STRPFUSECFG_CDCGDIS            (BIT3)
#define B_PCH_PCIE_STRPFUSECFG_DESKTOPMOB         (BIT2)

//
//PCI Express Extended Capability Registers
//

#define R_PCH_PCIE_EXCAP_OFFSET                   0x100

#define R_PCH_PCIE_EX_AECH                        0x100 ///< Advanced Error Reporting Capability Header
#define V_PCH_PCIE_EX_AEC_CV                      0x1
#define R_PCH_PCIE_EX_UEM                         (R_PCH_PCIE_EX_AECH + R_PCIE_EX_UEM_OFFSEST)

#define R_PCH_PCIE_EX_ACSECH                      0x140 ///< ACS Extended Capability Header
#define V_PCH_PCIE_EX_ACS_CV                      0x1
#define R_PCH_PCIE_EX_ACSCAPR                     (R_PCH_PCIE_EX_ACSECH + R_PCIE_EX_ACSCAPR_OFFSET)

#define R_PCH_PCIE_EX_PTMECH                      0x150 ///< PTM Extended Capability Header
#define V_PCH_PCIE_EX_PTM_CV                      0x1
#define R_PCH_PCIE_EX_PTMCAPR                     (R_PCH_PCIE_EX_PTMECH + R_PCIE_EX_PTMCAPR_OFFSET)

#define R_PCH_PCIE_EX_L1SECH                      0x200 ///< L1 Sub-States Extended Capability Header
#define V_PCH_PCIE_EX_L1S_CV                      0x1
#define R_PCH_PCIE_EX_L1SCAP                      (R_PCH_PCIE_EX_L1SECH + R_PCIE_EX_L1SCAP_OFFSET)
#define R_PCH_PCIE_EX_L1SCTL1                      (R_PCH_PCIE_EX_L1SECH + R_PCIE_EX_L1SCTL1_OFFSET)
#define R_PCH_PCIE_EX_L1SCTL2                     (R_PCH_PCIE_EX_L1SECH + R_PCIE_EX_L1SCTL2_OFFSET)

#define R_PCH_PCIE_EX_SPEECH                      0x220 ///< Secondary PCI Express Extended Capability Header
#define V_PCH_PCIE_EX_SPEECH_CV                   0x1
#define R_PCH_PCIE_EX_LCTL3                       (R_PCH_PCIE_EX_SPEECH + R_PCIE_EX_LCTL3_OFFSET)
#define R_PCH_PCIE_EX_L01EC                       (R_PCH_PCIE_EX_SPEECH + R_PCIE_EX_L01EC_OFFSET)
#define B_PCH_PCIE_EX_L01EC_UPL1RPH               (BIT30 | BIT29 | BIT28)
#define V_PCH_PCIE_EX_L01EC_UPL1TP                0x01000000
#define B_PCH_PCIE_EX_L01EC_DPL1RPH               (BIT22 | BIT21 | BIT20)
#define V_PCH_PCIE_EX_L01EC_DPL1TP                0x00020000
#define B_PCH_PCIE_EX_L01EC_UPL0RPH               (BIT14 | BIT13 | BIT12)
#define V_PCH_PCIE_EX_L01EC_UPL0TP                0x00000000
#define B_PCH_PCIE_EX_L01EC_DPL0RPH               (BIT6 | BIT5 | BIT4)
#define V_PCH_PCIE_EX_L01EC_DPL0TP                0x00000001

#define R_PCH_PCIE_EX_L23EC                       (R_PCH_PCIE_EX_SPEECH + R_PCIE_EX_L23EC_OFFSET)
#define B_PCH_PCIE_EX_L23EC_UPL3RPH               (BIT30 | BIT29 | BIT28)
#define V_PCH_PCIE_EX_L23EC_UPL3TP                0x02000000
#define B_PCH_PCIE_EX_L23EC_DPL3RPH               (BIT22 | BIT21 | BIT20)
#define V_PCH_PCIE_EX_L23EC_DPL3TP                0x00010000
#define B_PCH_PCIE_EX_L23EC_UPL2RPH               (BIT14 | BIT13 | BIT12)
#define V_PCH_PCIE_EX_L23EC_UPL2TP                0x00000300
#define B_PCH_PCIE_EX_L23EC_DPL2RPH               (BIT6 | BIT5 | BIT4)
#define V_PCH_PCIE_EX_L23EC_DPL2TP                0x00000003

#define R_PCH_PCIE_PCIERTP1                       0x300
#define R_PCH_PCIE_PCIERTP2                       0x304
#define R_PCH_PCIE_PCIENFTS                       0x314

#define R_PCH_PCIE_PCIEL0SC                       0x318
#define B_PCH_PCIE_PCIEL0SC_G2ASL0SPL             0xFF000000
#define B_PCH_PCIE_PCIEL0SC_G1ASL0SPL             0x00FF0000

#define R_PCH_PCIE_PCIECFG2                       0x320
#define B_PCH_PCIE_PCIECFG2_LBWSSTE               BIT30
#define B_PCH_PCIE_PCIECFG2_CROAOV                BIT24
#define B_PCH_PCIE_PCIECFG2_CROAOE                BIT23
#define B_PCH_PCIE_PCIECFG2_CRSREN                BIT22
#define B_PCH_PCIE_PCIECFG2_PMET                  (BIT21 | BIT20)
#define V_PCH_PCIE_PCIECFG2_PMET                  1
#define N_PCH_PCIE_PCIECFG2_PMET                  20

#define R_PCH_PCIE_PCIEDBG                        0x324
#define B_PCH_PCIE_PCIEDBG_LGCLKSQEXITDBTIMERS    (BIT25 | BIT24)
#define B_PCH_PCIE_PCIEDBG_CTONFAE                BIT14
#define B_PCH_PCIE_PCIEDBG_SQOL0                  BIT7
#define B_PCH_PCIE_PCIEDBG_SPCE                   BIT5

#define R_PCH_PCIE_PCIESTS1                          0x328
#define B_PCH_PCIE_PCIESTS1_LTSMSTATE                0xFF000000
#define N_PCH_PCIE_PCIESTS1_LTSMSTATE                24
#define V_PCH_PCIE_PCIESTS1_LTSMSTATE_DETRDY         0x01
#define V_PCH_PCIE_PCIESTS1_LTSMSTATE_DETRDYECINP1CG 0x0E
#define B_PCH_PCIE_PCIESTS1_LNKSTAT                  (BIT22 | BIT21 | BIT20 | BIT19)
#define N_PCH_PCIE_PCIESTS1_LNKSTAT                  19
#define V_PCH_PCIE_PCIESTS1_LNKSTAT_L0               0x7

#define R_PCH_PCIE_PCIEALC                        0x338  ///< PCI Express Additional Link Control
#define B_PCH_PCIE_PCIEALC_BLKDQDA                BIT26  ///< Block Detect

#define R_PCH_PCIE_PTMPSDC1                       0x39C
#define N_PCH_PCIE_PTMPSDC1_G1X2RPSD              24
#define V_PCH_PCIE_PTMPSDC1_G1X2RPSD              0x20
#define N_PCH_PCIE_PTMPSDC1_G1X2TPSD              16
#define V_PCH_PCIE_PTMPSDC1_G1X2TPSD              0x01
#define N_PCH_PCIE_PTMPSDC1_G1X1RPSD              8
#define V_PCH_PCIE_PTMPSDC1_G1X1RPSD              0x20
#define N_PCH_PCIE_PTMPSDC1_G1X1TPSD              0
#define V_PCH_PCIE_PTMPSDC1_G1X1TPSD              0x01

#define R_PCH_PCIE_PTMPSDC2                       0x3A0
#define R_PCH_PCIE_PTMPSDC3                       0x3A4
#define R_PCH_PCIE_PTMECFG                        0x3B0

#define R_PCH_PCIE_LTROVR                         0x400
#define R_PCH_PCIE_LTROVR2                        0x404
#define R_PCH_PCIE_LTROVR2_LTRNSOVREN             BIT1
#define R_PCH_PCIE_LTROVR2_LTRSOVREN              BIT0

#define R_PCH_PCIE_PHYCTL4                        0x408
#define B_PCH_PCIE_PHYCTL4_SQDIS                  BIT27
#define R_PCH_PCIE_PCIEPMECTL                     0x420
#define B_PCH_PCIE_PCIEPMECTL_FDPPGE              BIT31
#define B_PCH_PCIE_PCIEPMECTL_DLSULPPGE           BIT30
#define B_PCH_PCIE_PCIEPMECTL_DLSULDLSD           BIT29
#define B_PCH_PCIE_PCIEPMECTL_L1LE                BIT17
#define B_PCH_PCIE_PCIEPMECTL_L1LTRTLV            (BIT13 | BIT12 | BIT11 | BIT10 | BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4)
#define N_PCH_PCIE_PCIEPMECTL_L1LTRTLV            4
#define V_PCH_PCIE_PCIEPMECTL_L1LTRTLV            0x32
#define B_PCH_PCIE_PCIEPMECTL_L1FSOE              BIT0

#define R_PCH_PCIE_PCIEPMECTL2                    0x424
#define B_PCH_PCIE_PCIEPMECTL2_PHYCLPGE           BIT11
#define B_PCH_PCIE_PCIEPMECTL2_FDCPGE             BIT8
#define B_PCH_PCIE_PCIEPMECTL2_DETSCPGE           BIT7
#define B_PCH_PCIE_PCIEPMECTL2_L23RDYSCPGE        BIT6
#define B_PCH_PCIE_PCIEPMECTL2_DISSCPGE           BIT5
#define B_PCH_PCIE_PCIEPMECTL2_L1SCPGE            BIT4

#define R_PCH_PCIE_PCE                            0x428
#define B_PCH_PCIE_PCE_HAE                        BIT5
#define B_PCH_PCIE_PCE_PMCRE                      BIT0

#define R_PCH_PCIE_EQCFG1                         0x450
#define S_PCH_PCIE_EQCFG1                         4
#define N_PCH_PCIE_EQCFG1_LERSMIE                 21
#define B_PCH_PCIE_EQCFG1_RTLEPCEB                BIT16
#define B_PCH_PCIE_EQCFG1_RTPCOE                  BIT15
#define B_PCH_PCIE_EQCFG1_HAED                    BIT12
#define B_PCH_PCIE_EQCFG1_EQTS2IRRC               BIT7
#define B_PCH_PCIE_EQCFG1_TUPP                    BIT1

#define R_PCH_PCIE_RTPCL1                         0x454
#define B_PCH_PCIE_RTPCL1_PCM                     BIT31
#define B_PCH_PCIE_RTPCL1_RTPRECL2PL4             0x3F000000
#define B_PCH_PCIE_RTPCL1_RTPOSTCL1PL3            0xFC0000
#define B_PCH_PCIE_RTPCL1_RTPRECL1PL2             0x3F000
#define B_PCH_PCIE_RTPCL1_RTPOSTCL0PL1            0xFC0
#define B_PCH_PCIE_RTPCL1_RTPRECL0PL0             0x3F

#define R_PCH_PCIE_RTPCL2                         0x458
#define B_PCH_PCIE_RTPCL2_RTPOSTCL3PL             0x3F000
#define B_PCH_PCIE_RTPCL2_RTPRECL3PL6             0xFC0
#define B_PCH_PCIE_RTPCL2_RTPOSTCL2PL5            0x3F

#define R_PCH_PCIE_RTPCL3                         0x45C
#define B_PCH_PCIE_RTPCL3_RTPRECL7                0x3F000000
#define B_PCH_PCIE_RTPCL3_RTPOSTCL6               0xFC0000
#define B_PCH_PCIE_RTPCL3_RTPRECL6                0x3F000
#define B_PCH_PCIE_RTPCL3_RTPOSTCL5               0xFC0
#define B_PCH_PCIE_RTPCL3_RTPRECL5PL10            0x3F

#define R_PCH_PCIE_RTPCL4                         0x460
#define B_PCH_PCIE_RTPCL4_RTPOSTCL9               0x3F000000
#define B_PCH_PCIE_RTPCL4_RTPRECL9                0xFC0000
#define B_PCH_PCIE_RTPCL4_RTPOSTCL8               0x3F000
#define B_PCH_PCIE_RTPCL4_RTPRECL8                0xFC0
#define B_PCH_PCIE_RTPCL4_RTPOSTCL7               0x3F

#define R_PCH_PCIE_HAEQ                           0x468
#define B_PCH_PCIE_HAEQ_HAPCCPI                   (BIT31 | BIT30 | BIT29 | BIT28)
#define V_PCH_PCIE_HAEQ_HAPCCPI_2PCPI             2
#define N_PCH_PCIE_HAEQ_HAPCCPI_2PCPI             28


#define R_PCH_PCIE_LTCO1                          0x470
#define B_PCH_PCIE_LTCO1_L1TCOE                   BIT25
#define B_PCH_PCIE_LTCO1_L0TCOE                   BIT24
#define B_PCH_PCIE_LTCO1_L1TPOSTCO                0xFC0000
#define B_PCH_PCIE_LTCO1_L1TPRECO                 0x3F000
#define B_PCH_PCIE_LTCO1_L0TPOSTCO                0xFC0
#define B_PCH_PCIE_LTCO1_L0TPRECO                 0x3F

#define R_PCH_PCIE_LTCO2                          0x474
#define B_PCH_PCIE_LTCO1_L3TCOE                   BIT25
#define B_PCH_PCIE_LTCO1_L2TCOE                   BIT24
#define B_PCH_PCIE_LTCO1_L3TPOSTCO                0xFC0000
#define B_PCH_PCIE_LTCO1_L3TPRECO                 0x3F000
#define B_PCH_PCIE_LTCO1_L2TPOSTCO                0xFC0
#define B_PCH_PCIE_LTCO1_L2TPRECO                 0x3F

#define R_PCH_PCIE_G3L0SCTL                       0x478
#define B_PCH_PCIE_G3L0SCTL_G3UCNFTS              0x0000FF00
#define B_PCH_PCIE_G3L0SCTL_G3CCNFTS              0x000000FF

#define R_PCH_PCIE_EQCFG2                         0x47C
#define B_PCH_PCIE_EQCFG2_NTIC                    0xFF000000
#define B_PCH_PCIE_EQCFG2_EMD                     BIT23
#define B_PCH_PCIE_EQCFG2_NTSS                    (BIT22 | BIT21 | BIT20)
#define B_PCH_PCIE_EQCFG2_PCET                    (BIT19 | BIT18 | BIT17 | BIT16)
#define B_PCH_PCIE_EQCFG2_HAPCSB                  (BIT15 | BIT14 | BIT13 | BIT12)
#define B_PCH_PCIE_EQCFG2_NTEME                   BIT11
#define B_PCH_PCIE_EQCFG2_MPEME                   BIT10
#define B_PCH_PCIE_EQCFG2_REWMETM                 (BIT9 | BIT8)
#define B_PCH_PCIE_EQCFG2_REWMET                  0xFF

//
//PCI Express Extended End Point Capability Registers
//

#define R_PCH_PCIE_LTRECH_OFFSET                  0x00
#define R_PCH_PCIE_LTRECH_CID                     0x0018
#define R_PCH_PCIE_LTRECH_MSLR_OFFSET             0x04
#define R_PCH_PCIE_LTRECH_MNSLR_OFFSET            0x06


//
// PCIE PCRs (PID:SPA SPB SPC SPD SPE)
//
#define R_PCH_PCR_SPX_PCD                         0x0000                  ///< Port configuration and disable
#define B_PCH_PCR_SPX_PCD_RP1FN                   (BIT2 | BIT1 | BIT0)    ///< Port 1 Function Number
#define B_PCH_PCR_SPX_PCD_RP1CH                   BIT3                    ///< Port 1 config hide
#define B_PCH_PCR_SPX_PCD_RP2FN                   (BIT6 | BIT5 | BIT4)    ///< Port 2 Function Number
#define B_PCH_PCR_SPX_PCD_RP2CH                   BIT7                    ///< Port 2 config hide
#define B_PCH_PCR_SPX_PCD_RP3FN                   (BIT10 | BIT9 | BIT8)   ///< Port 3 Function Number
#define B_PCH_PCR_SPX_PCD_RP3CH                   BIT11                   ///< Port 3 config hide
#define B_PCH_PCR_SPX_PCD_RP4FN                   (BIT14 | BIT13 | BIT12) ///< Port 4 Function Number
#define B_PCH_PCR_SPX_PCD_RP4CH                   BIT15                   ///< Port 4 config hide
#define S_PCH_PCR_SPX_PCD_RP_FIELD                4                       ///< 4 bits for each RP FN
#define B_PCH_PCR_SPX_PCD_P1D                     BIT16                   ///< Port 1 disable
#define B_PCH_PCR_SPX_PCD_P2D                     BIT17                   ///< Port 2 disable
#define B_PCH_PCR_SPX_PCD_P3D                     BIT18                   ///< Port 3 disable
#define B_PCH_PCR_SPX_PCD_P4D                     BIT19                   ///< Port 4 disable
#define B_PCH_PCR_SPX_PCD_SRL                     BIT31                   ///< Secured Register Lock

#define R_PCH_PCR_SPX_PCIEHBP                     0x0004                  ///< PCI Express high-speed bypass
#define B_PCH_PCR_SPX_PCIEHBP_PCIEHBPME           BIT0                    ///< PCIe HBP mode enable
#define B_PCH_PCR_SPX_PCIEHBP_PCIEGMO             (BIT2 | BIT1)           ///< PCIe gen mode override
#define B_PCH_PCR_SPX_PCIEHBP_PCIETIL0O           BIT3                    ///< PCIe transmitter-in-L0 override
#define B_PCH_PCR_SPX_PCIEHBP_PCIERIL0O           BIT4                    ///< PCIe receiver-in-L0 override
#define B_PCH_PCR_SPX_PCIEHBP_PCIELRO             BIT5                    ///< PCIe link recovery override
#define B_PCH_PCR_SPX_PCIEHBP_PCIELDO             BIT6                    ///< PCIe link down override
#define B_PCH_PCR_SPX_PCIEHBP_PCIESSM             BIT7                    ///< PCIe SKP suppression mode
#define B_PCH_PCR_SPX_PCIEHBP_PCIESST             BIT8                    ///< PCIe suppress SKP transmission
#define B_PCH_PCR_SPX_PCIEHBP_PCIEHBPPS           (BIT13 | BIT12)         ///< PCIe HBP port select
#define B_PCH_PCR_SPX_PCIEHBP_CRCSEL              (BIT15 | BIT14)         ///< CRC select
#define B_PCH_PCR_SPX_PCIEHBP_PCIEHBPCRC          0xFFFF0000              ///< PCIe HBP CRC


//
// PCIe CLK Reference Driver PCR (PID: PCLKD)
//
#define R_PCR_PCLKD_L1TREF_CFG            0x1010
#define S_PCR_PCLKD_L1TREF_CFG_TCLKREQSRC 4

#endif

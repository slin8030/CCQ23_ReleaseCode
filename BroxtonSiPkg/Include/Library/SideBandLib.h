/** @file
  Prototype of the SideBandLib library.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

#ifndef _SB_LIB_H_
#define _SB_LIB_H_

/**
  General SideBandLib Information

  - Note: IOSF-SideBand prior to v2.0 is not supported.

    All future versions will be supported with backward support to v2.0. The latest
    version supported is specified in VERSION_IOSF_SIDEBAND_SUPPORT.

  - Note: IOSF-SideBand access via MMIO is not valid prior to SBREG_BAR (P2SB BAR)
          being set. By calling the generic SideBandRead() and SideBandWrite() library
          functions, the appropriate method will be used based on SBREG_BAR availability.

  - In the case SBREG_BAR is not set, the SBMI (SideBand Message Interface) functions
    will be used which read/write to SideBand using private PCI CFG registers in P2SB.
  @{  @}
*/

#define VERSION_IOSF_SIDEBAND_SUPPORT 2.0  ///< Guaranteed IOSF-SideBand Supported Version

//
// IOSF-SideBand Endpoint Port IDs
//
#define SB_AUDIO_PORT           0x92  ///< Audio, Video, Speech Subsystem
#define SB_CSE_GHOST_PORT       0x97  ///< Converged Security Engine Ghost Port
#define SB_CSE_HECI2_PORT       0x97  ///< Converged Security Engine HECI Port 2
#define SB_CSE_HECI3_PORT       0x97  ///< Converged Security Engine HECI Port 3
#define SB_PUNIT_PORT           0x46  ///< P-Unit Port
#define SB_GEN_PORT             0x30  ///< Display Engine
#define SB_GMM_PORT             0xAC  ///< Gaussian Mixture Model - Speech
#define SB_ISH_PHY_PORT         0x94  ///< Integrated Services Hub PHY
#define SB_ISH_BRIDGE_PORT      0x98  ///< Integrated Services Hub Bridge
#define SB_ITSS_PORT            0xD0  ///< Interrupt and Timer Subsystem
#define SB_IUNIT_PORT           0x32  ///< Camera Imaging Unit
#define SB_LPSS_PORT            0x90  ///< Low Power Subsystem
#define SB_MEX_PORT             0xAA  ///< Mobile Express
#define SB_NPK_PORT             0x88  ///< Northpeak
#define SB_P2SB_PORT            0xD4  ///< Primary to SideBand Bridge (P2SB)
#define SB_PMC_PORT             0x82  ///< Power Management Controller
#define SB_PMC_IOSF2OCP_PORT    0x95  ///< Power Management Controller IOSF2OCP Bridge
#define SB_RTC_PORT             0xD1  ///< Real-Time Clock
#define SB_SCC_PORT             0xD6  ///< Storage & Communication Cluster
#define SB_SPI_PORT             0x93  ///< SPI/eSPI Controller
#define SB_UFS_PORT             0xDC  ///< UFS MMP
#define SB_USB_DEVICE_PORT      0xA4  ///< USB Device Controller (xDCI)
#define SB_USB_HOST_PORT        0xA2  ///< USB Host Controller (xHCI)
#define SB_EXI_PORT             0xA8  ///< Embedded DFx Interface (ExI)
#define SB_DFX_AGG_PORT         0x84  ///< DFx Aggregator IP

//
// Internal Library Defines
//
#define P2SB_SBI_ADDR           0xD0
#define P2SB_SBI_DATA           0xD4
#define P2SB_SBI_STAT           0xD8
#define P2SB_SBI_RID            0xDA
#define P2SB_SBI_EXTADDR        0xDC

#define B_ADDR_DEST_PORT_MASK  (BIT31 | BIT30 | BIT29 | BIT28 | BIT27 | BIT26 |  \
                                BIT25 | BIT24)
#define B_ADDR_OFFSET_MASK     (BIT15 | BIT14 | BIT13 | BIT12 | BIT11 | BIT10 |  \
                                BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | \
                                BIT2 | BIT1 | BIT0)
#define B_ADDR_RSVD_MASK       (BIT23 | BIT22 | BIT21 | BIT20)
#define B_ADDR_ROOT_MASK       (BIT19 | BIT18 | BIT17 | BIT16)
#define B_STAT_INITIATE_MASK    BIT0
#define B_STAT_INITRDY_MASK     BIT0
#define B_STAT_OPCODE_MASK     (BIT15 | BIT14 | BIT13 | BIT12 | BIT11 | BIT10 |  \
                                BIT9 | BIT8)
#define B_STAT_POSTED_MASK      BIT7
#define B_STAT_RSVD_RESP       (BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1)
#define V_INITIATE              0x01
#define V_INITRDY               0x01
#define V_OPCODE_CFGRD          0x04
#define V_OPCODE_CFGWR          0x05
#define V_OPCODE_CRRD           0x06
#define V_OPCODE_CRWR           0x07
#define V_OPCODE_IORD           0x02
#define V_OPCODE_IOWR           0x03
#define V_OPCODE_MRD            0x00
#define V_OPCODE_MWR            0x01
#define V_POSTED                0x01
#define V_RID_FBE               0xF000
#define V_RSVD_RESP             0x00
#define V_RID_BAR_DBC           0x05

//
// Main SideBand Interface Functions
//

/**
  Determines whether to call a SideBand read via MMIO or SBMI (MMCFG) based on whether
  SBREG_REG is set up.

  This is the standard Read function that should be used for SideBand transactions.

  For MMIO Access:
    - In a single MMIO transaction, the "SBREG_BAR" is used
      to form a system address to access chipset registers.
    - Transactions using this method will read using Private Control Register opcode CRRd(06h).
    - All accesses using this method must be DWORD aligned DWORDs.

  For SBMI Access: Refer to SideBandSbmiRead32()

  @param[in]  TargetPortId    Target port to read.
  @param[in]  TargetRegister  Target register to read.

  @retval UINT32              The value read from Target Register.
**/
UINT32
EFIAPI
SideBandRead32(
  IN UINT8 TargetPortId,
  IN UINT16 TargetRegister
);

/**
  Determines whether to call a SideBand write via MMIO or SBMI (MMCFG) based on whether
  SBREG_REG is set up.

  This is the standard Write function that should be used for SideBand transactions.

  For MMIO Access:
    - In a single MMIO transaction, the "SBREG_BAR" is used
      to form a system address to access chipset registers.
    - Transactions using this method will write using Private Control Register opcode CRWr(07h).
    - All accesses using this method must be DWORD aligned DWORDs.

  For SBMI Access:  Refer to SideBandSbmiWrite32()

  @param[in]  TargetPortId    Target port to write.
  @param[in]  TargetRegister  Target register to write.
  @param[in]  Value           The value to write.

  @retval None.
**/
VOID
EFIAPI
SideBandWrite32(
  IN UINT8 TargetPortId,
  IN UINT16 TargetRegister,
  IN UINT32 Value
);

/**
  Determines whether to call SideBand operations via MMIO or SBMI (MMCFG) based on whether
  SBREG_REG is set up.

  This is the standard AndThenOr function that should be used for SideBand transactions.

  Reads the value in the Target Register of the Target Port Id, performs a bitwise AND followed
  by a bitwise OR between the read result and the value specified by AndData, and writes the
  32-bit value to the configuration register specified by Target Register.

  @param[in]  TargetPortId    Target port to read/write.
  @param[in]  TargetRegister  Target register to read/write.
  @param[in]  AndData         The value to AND with Target Register data.
  @param[in]  OrData          The value to OR with Target Register data.

  @retval None.
**/
VOID
EFIAPI
SideBandAndThenOr32(
  IN UINT8 TargetPortId,
  IN UINT16 TargetRegister,
  IN UINT32 AndData,
  IN UINT32 OrData
);

//
// Special Case SideBand Interface Functions
//

/**
  This function is reserved for special case sideband access:
    - Non-private control register opcode

  In multiple MMIO transactions, this access method uses a mechanism provided in PCI
  Configuration space to send IOSF-SB messages. This method should be used prior to
  full PCI enumeration. Since multiple messages are sent this method is not thread-safe
  but this is not an issue for single-threaded IAFW.

  Transactions using this method will read using the OpCode parameter.

  All accesses using this method must be DWORD aligned DWORDs.

  @param[in]  TargetPortId    Target port to read.
  @param[in]  TargetRegister  Target register to read.
  @param[in]  OpCode          Opcode to use.
  @param[in]  Bar             Target register access BAR.

  @retval The value read from Target Register.
**/
UINT32
EFIAPI
SideBandReadReserved32(
  IN UINT8  TargetPortId,
  IN UINT16 TargetRegister,
  IN UINT8  OpCode,
  IN UINT8  Bar
);

/**
  This function is reserved for special case sideband access:
    - Non-private control register opcode

  In multiple MMIO transactions, this access method uses a mechanism provided in PCI
  Configuration space to send IOSF-SB messages. This method should be used prior to
  full PCI enumeration. Since multiple messages are sent this method is not thread-safe
  but this is not an issue for single-threaded IAFW.

  Transactions using this method will write using the OpCode parameter.

  All accesses using this method must be DWORD aligned DWORDs.

  @param[in]  TargetPortId    Target port to write.
  @param[in]  TargetRegister  Target register to write.
  @param[in]  OpCode          Opcode to use.
  @param[in]  Bar             Target register access BAR.
  @param[in]  Value           The value to write.

  @retval None.
**/
VOID
EFIAPI
SideBandWriteReserved32(
  IN UINT8  TargetPortId,
  IN UINT16 TargetRegister,
  IN UINT8  OpCode,
  IN UINT8  Bar,
  IN UINT32 Value
);

/**
  This function is reserved for special case sideband access:
    - Non-private control register opcode

  Reads the value in the Target Register of the Target Port Id, performs a bitwise AND followed
  by a bitwise OR between the read result and the value specified by AndData, and writes the
  32-bit value to the configuration register specified by Target Register.

  @param[in]  TargetPortId    Target port to read/write.
  @param[in]  TargetRegister  Target register to read/write.
  @param[in]  ReadOpCode      Opcode to use for read.
  @param[in]  WriteOpCode     Opcode to use for write.
  @param[in]  Bar             Target register access BAR.
  @param[in]  AndData         The value to AND with Target Register data.
  @param[in]  OrData          The value to OR with Target Register data.

  @retval None.
**/
VOID
EFIAPI
SideBandAndThenOrReserved32(
  IN UINT8  TargetPortId,
  IN UINT16 TargetRegister,
  IN UINT8  ReadOpCode,
  IN UINT8  WriteOpCode,
  IN UINT8  Bar,
  IN UINT32 AndData,
  IN UINT32 OrData
);

//
// Helper Functions
//

/**
  Returns if SideBand access using MMIO is valid.

  @param  None.

  @retval BOOLEAN Returns true if SideBand access via P2SB MMIO is valid else returns false
**/
BOOLEAN
EFIAPI
SideBandMmioValid(
  VOID
);

/**
  Library utility function.

  Returns the MMIO Address used to access the register over P2SB.

  This function doesn't check if SB MMIO is enabled or not. Caller has to make sure
  SB MMIO is enabled, this is exposed to meet timing constraints of SDQ protocol in less than 10us

  @param[in]  TargetPortId    Target port.
  @param[in]  TargetRegister  Target register

  @retval UINT32              The MMIO address to access TargetRegister in TargetPortId with P2SB.
**/
UINT32
EFIAPI
GetSideBandMmioAddress(
  IN UINT8 TargetPortId,
  IN UINT16 TargetRegister
  );

#endif // _SB_LIB_H_

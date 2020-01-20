/** @file
  This file contains the implementation of SideBandLib library.

  The library exposes an abstract interface to the SoC SideBand network.

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

@par Specification
**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/SideBandLib.h>
#include <Library/ScPlatformLib.h>

//
// SocRefCode Dependencies
//
#include <SaAccess.h>
#include <ScAccess.h>

#define P2SB_PCI_MM_BASE (UINT32)MmPciAddress(0, \
                                              0, \
                                              PCI_DEVICE_NUMBER_P2SB, \
                                              PCI_FUNCTION_NUMBER_P2SB, \
                                              0)

/**
  The constructor function initializes module variables that contain base addresses
 used by library functions.

  @retval EFI_SUCCESS   Always return, since there's no error codes here.
**/
EFI_STATUS
EFIAPI
SideBandLibConstructor(
  VOID
  )
{
  return EFI_SUCCESS;
}

/**
  Internal library utility function.

  Returns the MMIO Address used to access the register over P2SB.

  @param[in]  TargetPortId    Target port.
  @param[in]  TargetRegister  Target register

  @retval UINT32              The MMIO address to access TargetRegister in TargetPortId with P2SB.
**/
UINT32
EFIAPI
GetSideBandMmioAddress(
  IN UINT8 TargetPortId,
  IN UINT16 TargetRegister
  )
{
  UINT32 Temp = MmioRead32(P2SB_PCI_MM_BASE + R_PCH_P2SB_SBREG_BAR) & B_PCH_P2SB_SBREG_RBA;
  Temp |= TargetPortId << 16;
  Temp |= TargetRegister;

  return Temp;
}

/**
  Reads from IOSF-SB via MMIO.

  In a single MMIO transaction, the "SBREG_BAR" is used
  to form a system address to access chipset registers.

  Transactions using this method will read using Private Control Register opcode CRRd(06h).

  All accesses using this method must be DWORD aligned DWORDs.

  @param[in]  TargetPortId     Target port to read.
  @param[in]  TargetRegister   Target register to read.

  @retval UINT32               The value read from the Target Register.
**/
UINT32
EFIAPI
SideBandMmioRead32(
  IN UINT8 TargetPortId,
  IN UINT16 TargetRegister
  )
{
  return MmioRead32(GetSideBandMmioAddress(TargetPortId, TargetRegister));
}

/**
  Reads from IOSF-SB via SideBand Message Interface Access (MMCFG).

  In multiple MMIO transactions, this access method uses a mechanism provided in PCI
  Configuration space to send IOSF-SB messages. This method should be used prior to
  full PCI enumeration. Since multiple messages are sent this method is not thread-safe
  but this is not an issue for single-threaded IAFW.

  Transactions using this method will read using Private Control Register opcode CRRd(06h)

  All accesses using this method must be DWORD aligned DWORDs.

  @param[in]  TargetPortId           Target port to read.
  @param[in]  TargetRegister         Target register to read.
  @param[in]  OpCode                 Opcode to use.
  @param[in]  Bar                    Target register access BAR.

  @retval  UINT32                The value read from Target Register.
**/
UINT32
EFIAPI
SideBandSbmiRead32(
  IN UINT8  TargetPortId,
  IN UINT16 TargetRegister,
  IN UINT8  OpCode,
  IN UINT8  Bar
  )
{
  MmioWrite32((P2SB_PCI_MM_BASE | P2SB_SBI_EXTADDR), (UINT32)(0x0));
  MmioWrite32((P2SB_PCI_MM_BASE | P2SB_SBI_DATA), (UINT32)(0x0));
  MmioWrite32((P2SB_PCI_MM_BASE | P2SB_SBI_ADDR), (UINT32)((TargetPortId << 24) | (TargetRegister)));
  MmioWrite16((P2SB_PCI_MM_BASE | P2SB_SBI_RID), (UINT16)((Bar << 8) | (V_RID_FBE)));
  MmioWrite16((P2SB_PCI_MM_BASE | P2SB_SBI_STAT), (UINT16)((OpCode << 8) | (V_INITIATE)));

  return MmioRead32(P2SB_PCI_MM_BASE | P2SB_SBI_DATA);
}

/**
  SideBand Message Interface Access (MMCFG) Functions.

  In multiple MMIO transactions, this access method uses a mechanism provided in PCI
  Configuration space to send IOSF-SB messages. This method should be used prior to
  full PCI enumeration. Since multiple messages are sent this method is not thread-safe
  but this is not an issue for single-threaded IAFW.

  All accesses using this method must be DWORD aligned DWORDs.

  @param[in]  TargetPortId        Target port to write.
  @param[in]  TargetRegister      Target register to write.
  @param[in]  OpCode              Opcode to use.
  @param[in]  Value               The value to write.
  @param[in]  Bar                 Target register access BAR.

  @retval None.
**/
VOID
EFIAPI
SideBandSbmiWrite32(
  IN UINT8  TargetPortId,
  IN UINT16 TargetRegister,
  IN UINT8  OpCode,
  IN UINT8  Bar,
  IN UINT32 Value
  )
{
  MmioWrite32((P2SB_PCI_MM_BASE | P2SB_SBI_ADDR), (UINT32)((TargetPortId << 24) | (TargetRegister)));
  MmioWrite16((P2SB_PCI_MM_BASE | P2SB_SBI_RID), (UINT16)((Bar << 8) | (V_RID_FBE)));
  MmioWrite32((P2SB_PCI_MM_BASE | P2SB_SBI_EXTADDR), (UINT32)(0x0));
  MmioWrite32((P2SB_PCI_MM_BASE | P2SB_SBI_DATA), (UINT32)(Value));
  MmioWrite16((P2SB_PCI_MM_BASE | P2SB_SBI_STAT), (UINT16)((OpCode << 8) | (V_INITIATE)));
}

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

  @param[in]  TargetPortId       Target port to read.
  @param[in]  TargetRegister     Target register to read.

  @retval UINT32                 The value read from Target Register.
**/
UINT32
EFIAPI
SideBandRead32(
  IN UINT8 TargetPortId,
  IN UINT16 TargetRegister
  )
{
  UINT32   ReturnValue;
  UINTN    P2sbBase;
  BOOLEAN  P2sbOrgStatus;

  ReturnValue = 0;
  P2sbBase = P2SB_PCI_MM_BASE;
  P2sbOrgStatus = FALSE;
  PchRevealP2sb (P2sbBase, &P2sbOrgStatus);

  // MMIO access requires MMIO to be valid (P2SB BAR set and Memory Decode enabled) and the address to be 32-bit aligned
  if (SideBandMmioValid() && (GetSideBandMmioAddress(TargetPortId, TargetRegister) & 3) == 0) {
    ReturnValue = MmioRead32(GetSideBandMmioAddress(TargetPortId, TargetRegister));
  } else {
    ReturnValue = SideBandSbmiRead32(TargetPortId, TargetRegister, V_OPCODE_CRRD, 0);
  }

  if (!P2sbOrgStatus) {
    PchHideP2sb (P2sbBase);
  }

  return ReturnValue;
}

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

  @param[in]  TargetPortId         Target port to write.
  @param[in]  TargetRegister       Target register to write.
  @param[in]  Value                The value to write.

  @retval None.
**/
VOID
EFIAPI
SideBandWrite32(
  IN UINT8 TargetPortId,
  IN UINT16 TargetRegister,
  IN UINT32 Value
  )
{
  UINTN    P2sbBase;
  BOOLEAN  P2sbOrgStatus;

  P2sbBase = P2SB_PCI_MM_BASE;
  P2sbOrgStatus = FALSE;
  PchRevealP2sb (P2sbBase, &P2sbOrgStatus);

  // MMIO access requires MMIO to be valid (P2SB BAR set and Memory Decode enabled) and the address to be 32-bit aligned
  if (SideBandMmioValid() == TRUE && (GetSideBandMmioAddress(TargetPortId, TargetRegister) & 3) == 0) {
    MmioWrite32(GetSideBandMmioAddress(TargetPortId, TargetRegister), Value);
  } else {
    SideBandSbmiWrite32(TargetPortId, TargetRegister, V_OPCODE_CRWR, 0, Value);
  }

  if (!P2sbOrgStatus) {
    PchHideP2sb (P2sbBase);
  }
}

/**
  Determines whether to call SideBand operations via MMIO or SBMI (MMCFG) based on whether
  SBREG_REG is set up.

  This is the standard AndThenOr function that should be used for SideBand transactions.

  Reads the value in the Target Register of the Target Port Id, performs a bitwise AND followed
  by a bitwise OR between the read result and the value specified by AndData, and writes the
  32-bit value to the configuration register specified by Target Register.

  @param[in]  TargetPortId         Target port to read/write.
  @param[in]  TargetRegister       Target register to read/write.
  @param[in]  AndData              The value to AND with Target Register data.
  @param[in]  OrData               The value to OR with Target Register data.

  @retval None.
**/
VOID
EFIAPI
SideBandAndThenOr32(
  IN UINT8 TargetPortId,
  IN UINT16 TargetRegister,
  IN UINT32 AndData,
  IN UINT32 OrData
  )
{
  UINT32 Temp = SideBandRead32(TargetPortId, TargetRegister);
  Temp = (Temp & AndData) | OrData;
  SideBandWrite32(TargetPortId, TargetRegister, Temp);
}

/**
  This function is reserved for special case sideband access:
    - Non-private control register opcode

  In multiple MMIO transactions, this access method uses a mechanism provided in PCI
  Configuration space to send IOSF-SB messages. This method should be used prior to
  full PCI enumeration. Since multiple messages are sent this method is not thread-safe
  but this is not an issue for single-threaded IAFW.

  Transactions using this method will read using the OpCode parameter.

  All accesses using this method must be DWORD aligned DWORDs.

  @param[IN]  TargetPortId          Target port to read.
  @param[IN]  TargetRegister        Target register to read.
  @param[IN]  OpCode                Opcode to use.
  @param[in]  Bar                   Target register access BAR.

  @retval UINT32                    The value read from Target Register.
**/
UINT32
EFIAPI
SideBandReadReserved32(
  IN UINT8  TargetPortId,
  IN UINT16 TargetRegister,
  IN UINT8  OpCode,
  IN UINT8  Bar
  )
{
  return SideBandSbmiRead32(TargetPortId, TargetRegister, OpCode, Bar);
}

/**
  This function is reserved for special case sideband access:
    - Non-private control register opcode

  In multiple MMIO transactions, this access method uses a mechanism provided in PCI
  Configuration space to send IOSF-SB messages. This method should be used prior to
  full PCI enumeration. Since multiple messages are sent this method is not thread-safe
  but this is not an issue for single-threaded IAFW.

  Transactions using this method will write using the OpCode parameter.

  All accesses using this method must be DWORD aligned DWORDs.

  @param[in]  TargetPortId         Target port to write.
  @param[in]  TargetRegister       Target register to write.
  @param[in]  OpCode               Opcode to use.
  @param[in]  Bar                  Target register access BAR.
  @param[in]  Value                The value to write.

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
  )
{
  SideBandSbmiWrite32(TargetPortId, TargetRegister, OpCode, Bar, Value);
}

/**
  This function is reserved for special case sideband access:
    - Non-private control register opcode

  Reads the value in the Target Register of the Target Port Id, performs a bitwise AND followed
  by a bitwise OR between the read result and the value specified by AndData, and writes the
  32-bit value to the configuration register specified by Target Register.

  @param[in]  TargetPortId         Target port to read/write.
  @param[in]  TargetRegister       Target register to read/write.
  @param[in]  ReadOpCode           Opcode to use for read.
  @param[in]  WriteOpCode          Opcode to use for write.
  @param[in]  Bar                  Target register access BAR.
  @param[in]  AndData              The value to AND with Target Register data.
  @param[in]  OrData               The value to OR with Target Register data.

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
  )
{
  UINT32 Temp = SideBandSbmiRead32(TargetPortId, TargetRegister, ReadOpCode, Bar);

  Temp = (Temp & AndData) | OrData;
  SideBandSbmiWrite32(TargetPortId, TargetRegister, WriteOpCode, Bar, Temp);
}

/**
  Returns if SideBand access using P2SB MMIO is valid.

  @param  None.

  @retval BOOLEAN                  Returns true if SideBand access via P2SB MMIO is valid else returns false
**/
BOOLEAN
EFIAPI
SideBandMmioValid(
  VOID
)
{
  UINT32 P2sbMmioBar = 0;
  UINT16 P2sbCmd = 0;
  UINT32 P2sbPciMmBase = P2SB_PCI_MM_BASE;

  P2sbMmioBar = MmioRead32(P2sbPciMmBase + R_PCH_P2SB_SBREG_BAR);
  P2sbMmioBar &= B_PCH_P2SB_SBREG_RBA;  // Mask off the last byte read from BAR

  P2sbCmd = MmioRead16(P2sbPciMmBase + PCI_COMMAND_OFFSET);

  if ((P2sbMmioBar != 0) && (P2sbMmioBar != B_PCH_P2SB_SBREG_RBA) && (P2sbCmd & EFI_PCI_COMMAND_MEMORY_SPACE) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}
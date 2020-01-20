/** @file
  RegAccess.h
  This file handles register accesses.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2005 - 2016 Intel Corporation.

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
#include "RegAccess.h"
#include "MmrcProjectDefinitions.h"
#include "MmrcLibraries.h"
#ifndef SUSSW
#include <Library/BaseLib.h>
#endif

#if !defined SIM && !defined JTAG
//
// Memory Mapped IO
//
/**
  Mmio32Read

  @param[in]       RegisterAddress

  @return     Value read
**/
UINT32
Mmio32Read (
  IN        UINT32      RegisterAddress
)
{
  VOLATILE UINT32 *addr = (UINT32 *) RegisterAddress;
  return *addr;
}
/**
  Mmio16Read

  @param[in]       RegisterAddress

  @return     Value read
**/
UINT16
Mmio16Read (
  IN        UINT32      RegisterAddress
)
{
  VOLATILE UINT16 *addr = (UINT16 *) RegisterAddress;
  return *addr;
}
/**
  Mmio8Read

  @param[in]       RegisterAddress

  @return     Value read
**/
UINT8
Mmio8Read (
  IN        UINT32      RegisterAddress
)
{
  VOLATILE UINT8 *addr = (UINT8 *) RegisterAddress;
  return *addr;
}

/**
  Mmio32Write

  @param[in]       RegisterAddress
  @param[in]       Value

  @return     None
**/
VOID
Mmio32Write (
  IN        UINT32      RegisterAddress,
  IN        UINT32      Value
)
{
  VOLATILE UINT32 *addr = (UINT32 *) RegisterAddress;
  *addr = Value;
}

/**
  Mmio16Write

  @param[in]       RegisterAddress
  @param[in]       Value

  @return     None
**/
VOID
Mmio16Write (
  IN        UINT32      RegisterAddress,
  IN        UINT16      Value
)
{
  VOLATILE UINT16 *addr = (UINT16 *) RegisterAddress;
  *addr = Value;
}

/**
  Mmio8Write

  @param[in]       RegisterAddress
  @param[in]       Value

  @return     None
**/
VOID
Mmio8Write (
  IN        UINT32      RegisterAddress,
  IN        UINT8       Value
)
{
  VOLATILE UINT8 *addr = (UINT8 *) RegisterAddress;
  *addr = Value;
}

#endif
/**
  GetRegisterAccessInfo

  @param[in]       BoxType
  @param[in]       Channel
  @param[in]       Instance
  @param[in,out]   Command
  @param[in,out]   PortId
  @param[in,out]   Offset
  @param[in,out]   Bus
  @param[in,out]   Device
  @param[in,out]   Func
  @param[in]       Mode

  @return     MMRC_STATUS
**/
UINT32
GetRegisterAccessInfo (
  IN        UINT8       BoxType,
  IN        UINT8       Channel,
  IN        UINT8       Instance,
  IN  OUT   UINT32      *Command,
  IN  OUT   UINT8       *PortId,
  IN  OUT   UINT32      *Offset,
  IN  OUT   UINT8       *Bus,
  IN  OUT   UINT8       *Device,
  IN  OUT   UINT8       *Func,
  IN        UINT32      Mode
)
{
  UINT32 Error;
  MMRC_DATA *MrcData;
  Error = NoError;

  MrcData = GetMrcHostStructureAddress ();
#if SIM || JTAG
  if (!InstancePortMap[BoxType].SimulationSupported) {
    return BoxTypeNotInSimulation;
  }
#endif
  //
  // Provided into this function is the instance number within the channel, getRegAccess requires the total instance index.
  // This translates the channel to the instance number in the INSTANCE_PORT_OFFSET structure.
  //
  if (ChannelToInstanceMap[BoxType][Channel] == 0xFF) {
    return InvalidInstance;
  } else {
    Instance += ChannelToInstanceMap[BoxType][Channel];
  }
  //
  // If the Type is a PCI access, get the B/D/F.
  //
  if (InstancePortMap[BoxType].AccessMethod == ePCI) {
    *Bus    = (InstancePortOffset[InstancePortMap[BoxType].InstanceStartIndex + Instance].Port >> 8) & 0xff;
    *Device = (InstancePortOffset[InstancePortMap[BoxType].InstanceStartIndex + Instance].Port >> 3) & 0x1f;
    *Func   = InstancePortOffset[InstancePortMap[BoxType].InstanceStartIndex + Instance].Port & 0x07;
    *Offset = 0;
  } else if (InstancePortMap[BoxType].AccessMethod == eSB) {
    //
    // Look at the type, which is either a sideband port or an address offset for this box type.
    // If it's a port, assign it here. If it's an address offset, do the appropriate math to
    // target the Instance passed in.
    //
    *PortId = (UINT8) InstancePortOffset[InstancePortMap[BoxType].InstanceStartIndex + Instance].Port;
    *Offset = InstancePortOffset[InstancePortMap[BoxType].InstanceStartIndex + Instance].Offset;
  } else if (InstancePortMap[BoxType].AccessMethod == eBAR) {
    //
    // For BAR accesses, compile a full 32 bit offset from Port (upper 16 bits) and Offset (lower 16 bits).
    // We reuse the definition of Port for the upper 16 bits to save on code space in InstancePortOffset.
    //
    *Offset = (InstancePortOffset[InstancePortMap[BoxType].InstanceStartIndex + Instance].Port << 16) + InstancePortOffset[InstancePortMap[BoxType].InstanceStartIndex + Instance].Offset;
  }
  //
  // Get the sideband command to read/write this particular box type.
  // If the command is not read or write, do not attempt to change it.
  //
  if (Mode == ModeRead) {
  *Command = InstancePortMap[BoxType].ReadOperation  << 24;
  } else if (Mode == ModeWrite) {
  *Command = InstancePortMap[BoxType].WriteOperation << 24;
  } else {
    *Command = *Command << 24;
  }
  //
  // Handle all errors.
  //
  if (Error != NoError) {
    if (Mode == ModeRead) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "RegRead ERROR: "));
    } else if (Mode == ModeWrite) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "RegWrite ERROR: "));
    }
  }
  switch (Error) {
  case NoError:
    break;
  case InvalidInstance:
    MmrcDebugPrint ((MMRC_DBG_MIN, "Invalid Instance 0x%x for Box 0x%d (offset = 0x%x)\n", Instance, BoxType, *Offset));
    break;
  default:
    break;
  }
  return Error;
}
/**
  Reads registers from a specified Unit

  @param[in]       BoxType      Unit to select
  @param[in]       Channel
  @param[in]       Instance     Channel under test
  @param[in,out]   Offset       Offset of register to read.

  @return     Value read
**/
UINTX
MemRegRead (
  IN        UINT8           BoxType,
  IN        UINT8           Channel,
  IN        UINT8           Instance,
  IN        REGISTER_ACCESS Register
)
{
  UINT32          Command;
  UINT8           PortId;
  UINT32          Data;
  UINT32          BaseOffset;
  UINT8           Bus;
  UINT8           Device;
  UINT8           Func;
  UINT32         *MemOffset;
  MMRC_DATA      *MrcData;
  UINT8           OffsetShift;
  UINT8           ShiftLeftAmount;
  UINT8           ShiftRightAmount;
  UINTX           TotalValue;
  REGISTER_ACCESS TempRegister;

  OffsetShift       = 0;
  ShiftLeftAmount   = 0;
  ShiftRightAmount  = 0;
  TotalValue        = 0;
  Data              = 0;
  MrcData           = GetMrcHostStructureAddress();

  if (GetRegisterAccessInfo (BoxType, Channel, Instance, &Command, &PortId, &BaseOffset, &Bus, &Device, &Func, ModeRead) == NoError) {
    //
    // If we are dealing with a PCI or MEM type register, the offset could
    // not be DWORD aligned. In this case, we may need to do multiple reads to
    // piece together the entire register since reads to PCI or MEM should be
    // DWORD aligned.
    //
//#ifndef JTAG
    if (InstancePortMap[BoxType].AccessMethod == ePCI || InstancePortMap[BoxType].AccessMethod == eBAR) {
      OffsetShift       = (Register.Offset % 4);
      ShiftLeftAmount   = OffsetShift * 8;
      ShiftRightAmount  = 32 - ShiftLeftAmount;
      //
      // Align the offset to the next DWORD down (i.e. offset 0x33 will be offset 0x30 after this).
      //
      Register.Offset  -= OffsetShift;
      //
      // If the offset needed to be shifted (ie. it's not DWORD aligned) or the
      // mask is more than 32 bits, we need to recurse to read another DWORD.
      //
      if (OffsetShift != 0 || Register.Mask > 0xFFFFFFFF) {
        //
        // Create a temporary register mask in case we need to recurse with a new mask.
        //
#ifdef SUSSW
        TempRegister.Mask     = Register.Mask >> ShiftRightAmount;
#else
        TempRegister.Mask     = RShiftU64(Register.Mask, ShiftRightAmount);
#endif
        TempRegister.Offset   = Register.Offset + 4;
        if (TempRegister.Mask != 0) {
          TotalValue = MemRegRead (BoxType, Channel, Instance, TempRegister);
        }
      }
      //
      // We've completed recursion, building up all upper DWORDs of the register. Now we
      // need to read the final DWORD. Since we've subtracted OffsetShift from the original
      // offset, we need to align the mask with the new DWORD aligned offset.
      //
#ifdef SUSSW
      Register.Mask <<= ShiftLeftAmount;
#else
      Register.Mask  = LShiftU64(Register.Mask, ShiftLeftAmount);
#endif
    }
//#endif
    //
    // If the action is to be taking place and the transaction is a PCI, then just do the PCI Access.
    //
    if (InstancePortMap[BoxType].AccessMethod == ePCI) {
      //
      // PCI access
      //
#if defined SIM || defined SIM_BUILD || defined KTI_SW_SIMULATION || defined JTAG
      Data = 0;
//
// EP/EX segment simulation hooks.
//
/*      Data = CpuSimRegAccess (MrcData, MrcData->var.mem.currentSocket,
        (UINT32)(MrcData->var.common.mmCfgBase +
        (UINT32)(Bus << 20) +
        (UINT32)(Device << 15) +
        (UINT32)(Func << 12) +
                 Register.Offset + BaseOffset), 4, 0, READ_ACCESS);*/
#else
        Data = PCI_CFG_32B_READ (MrcData->EcBase, Bus, Device, Func, Register.Offset + BaseOffset);
#endif
        MmrcDebugPrint ((MMRC_DBG_REG_ACCESS, "PCI read %02X/%02X/%01X, Offset=0x%X, Mask=0x%08X, Value=0x%08X\n",
           Bus, Device, Func, Register.Offset + BaseOffset, (UINT32) Register.Mask, (UINT32) Data));
    } else if (InstancePortMap[BoxType].AccessMethod == eBAR) {
      //
      // Memory BAR access
      //
      MemOffset = (UINT32 *)(UINT32)(MrcData->DynamicVars[Channel][InstancePortMap[BoxType].AccessBase] + Register.Offset + BaseOffset);
#if SIM || JTAG
      Data = MySimBar32ReadFast ((UINT32) MemOffset);
#else
      Data = (UINT32)*MemOffset;
#endif
      MmrcDebugPrint ((MMRC_DBG_REG_ACCESS, "MEM read to offset=0x%08X; data=0x%08X\n", MemOffset, (UINT32) Data));
    } else {
      //
      // Sideband access.
      //
      //
      // If the read and write opcodes are the same, it means reads are not supported.
      //
      if (InstancePortMap[BoxType].ReadOperation == InstancePortMap[BoxType].WriteOperation) {
        return 0;
      }

#if SIM || JTAG
      Data = MySimMmio32ReadFast (PortId, Register.Offset + BaseOffset, 0xF);
#else
      Mmio32Write (MrcData->EcBase + 0xD8, (Register.Offset + BaseOffset) & 0xFFFFFF00);
      Mmio32Write (MrcData->EcBase + 0xD0, (((Command) | (PortId << 16) | ((Register.Offset & 0x000000FF) << 8)) + 0xF0) );
      Data = Mmio32Read (MrcData->EcBase + 0xD4);
#endif
      MmrcDebugPrint ((MMRC_DBG_REG_ACCESS, "SB read to port=0x%02X; opcode=0x%02X; offset=0x%08X; data=0x%08X\n",
        PortId, InstancePortMap[BoxType].ReadOperation, Register.Offset + BaseOffset, (UINT32) Data));
    }
#ifdef SUSSW
    TotalValue = (TotalValue << ShiftRightAmount) | ((Data & (UINT32) Register.Mask) >> ShiftLeftAmount);
#else
    TotalValue = (LShiftU64(TotalValue, ShiftRightAmount)) | (RShiftU64((Data & (UINT32) Register.Mask), ShiftLeftAmount));
#endif
    return TotalValue;
  }
#if USE_64_BIT_VARIABLES
  return 0xFFFFFFFFFFFFFFFF;
#else
  return 0xFFFFFFFF;
#endif
}
/**
  Writes registers to a specified Unit

  @param[in]       BoxType      Unit to select
  @param[in]       Channel
  @param[in]       Instance     Channel under test
  @param[in]       Offset       Offset of register to write.
  @param[in]       Data         Data to be written
  @param[in]       Be

  @return     None
**/
VOID
MemRegWrite (
  IN        UINT8           BoxType,
  IN        UINT8           Channel,
  IN        UINT8           Instance,
  IN        REGISTER_ACCESS Register,
  IN        UINTX           Data,
  IN        UINT8           Be
)
{
  UINT32          Command;
  UINT8           PortId;
  UINT32          BaseOffset;
  UINT8           Bus;
  UINT8           Device;
  UINT8           Func;
  UINT32         *MemOffset;
  MMRC_DATA      *MrcData;
  UINT8           OffsetShift;
  UINT8           ShiftLeftAmount;
  UINT8           ShiftRightAmount;
  UINTX           TotalValue;
  REGISTER_ACCESS TempRegister;
  UINTX           TempData;
  UINT8           Temp;

  Temp = 0;
  OffsetShift       = 0;
  ShiftLeftAmount   = 0;
  ShiftRightAmount  = 0;
  TotalValue        = 0;
  MrcData           = GetMrcHostStructureAddress();

  if (GetRegisterAccessInfo (BoxType, Channel, Instance, &Command, &PortId, &BaseOffset, &Bus, &Device, &Func, ModeWrite) == NoError) {
    //
    // If we are dealing with a PCI or MEM type register, the offset could
    // not be DWORD aligned. In this case, we may need to do multiple writes to
    // piece together the entire register since reads to PCI or MEM should be
    // DWORD aligned.
    //
//#ifndef JTAG
    if (InstancePortMap[BoxType].AccessMethod == ePCI || InstancePortMap[BoxType].AccessMethod == eBAR) {
      OffsetShift       = (Register.Offset % 4);
      ShiftLeftAmount   = OffsetShift * 8;
      ShiftRightAmount  = 32 - ShiftLeftAmount;
      //
      // Align the offset to the next DWORD down (i.e. offset 0x33 will be offset 0x30 after this).
      //
      Register.Offset  -= OffsetShift;
      //
      // If the offset needed to be shifted (ie. it's not DWORD aligned) or the
      // mask is more than 32 bits, we need to recurse to read another DWORD.
      //
      if (OffsetShift != 0 || Register.Mask > 0xFFFFFFFF) {
        //
        // Write the next 32 bits (0xFFFFFFFF), but leave any bits above that
        // left over so if we need to recurse again, we can.
        //
#ifdef SUSSW
        TempRegister.Mask     = Register.Mask >> ShiftRightAmount;
#else
        TempRegister.Mask     = RShiftU64(Register.Mask, ShiftRightAmount);
#endif
        TempRegister.Offset   = Register.Offset + 4;
        TempRegister.ShiftBit = Register.ShiftBit;
#ifdef SUSSW
        TempData              = Data >> ShiftRightAmount;
#else
        TempData              = RShiftU64(Data, ShiftRightAmount);
#endif
        if (TempRegister.Mask != 0) {
          MemRegWrite (BoxType, Channel, Instance, TempRegister, TempData, 0xF);
        }
      }
      //
      // We've completed recursion, writing all upper DWORDs of the register. Now we
      // need to write the final DWORD. Since we've subtracted OffsetShift from the original
      // offset, we need to align the mask with the new DWORD aligned offset.
      //
#ifdef SUSSW
      Register.Mask <<= ShiftLeftAmount;
#else
      Register.Mask = LShiftU64(Register.Mask, ShiftLeftAmount);
#endif
      //
      // If the register is DWORD aligned and we're writing all 32 bits, skip the read.
      // Else, need to read/modify/write register.
      //
      if (OffsetShift != 0 || (UINT32) Register.Mask != 0xFFFFFFFF) {
        //
        // Since we're doing a 32 bit read/modify/write, only use the lower 32 bits of
        // the mask.
        //
        TempRegister.Mask = (UINT32) ~Register.Mask;
        TempRegister.Offset = Register.Offset;
        TotalValue = MemRegRead (BoxType, Channel, Instance, TempRegister);
      }
      //
      // Now that we have the fields read from the register and masked, OR in the new data.
      //
      ////TotalValue |= (Data << Register.ShiftBit);
#ifdef SUSSW
      Data        = TotalValue | (Data << (ShiftLeftAmount+Register.ShiftBit));
#else
      Data        = TotalValue | LShiftU64(Data, (ShiftLeftAmount+Register.ShiftBit));
#endif
      //TotalValue |= (Data << ShiftLeftAmount);
      //Data        = TotalValue << Register.ShiftBit;

    }
//#endif
    //
    // If the action is to be taking place and the transaction is a PCI, then just do the PCI Access.
    //
    if (InstancePortMap[BoxType].AccessMethod == ePCI) {
      MmrcDebugPrint ((MMRC_DBG_REG_ACCESS, "PCI write to %02X/%02X/%01X, Offset=0x%X, Mask=0x%08X, Data=0x%08X\n",
        Bus, Device, Func, Register.Offset + BaseOffset, (UINT32) Register.Mask, (UINT32) Data));
      //
      // PCI access
      //
#if defined SIM || defined SIM_BUILD || defined KTI_SW_SIMULATION || defined JTAG
//
// EP/EX segment simulation hooks.
//
/*      CpuSimRegAccess (MrcData, MrcData->var.mem.currentSocket,
        (UINT32)(MrcData->var.common.mmCfgBase +
        (UINT32)(Bus << 20) +
        (UINT32)(Device << 15) +
        (UINT32)(Func << 12) +
                 Register.Offset + BaseOffset), 4, (UINT32)Data, WRITE_ACCESS);*/
#else
      PCI_CFG_32B_WRITE (MrcData->EcBase, Bus, Device, Func, Register.Offset + BaseOffset, (UINT32) Data);
#endif
    } else if (InstancePortMap[BoxType].AccessMethod == eBAR) {
      //
      // Memory BAR access
      //
      MemOffset = (UINT32 *)(UINT32)(MrcData->DynamicVars[Channel][InstancePortMap[BoxType].AccessBase] + Register.Offset + BaseOffset);
      MmrcDebugPrint ((MMRC_DBG_REG_ACCESS, "MEM write to offset=0x%08X; data=0x%08X\n", MemOffset, (UINT32) Data));

#if SIM || JTAG
      MySimBar32WriteFast(Command, (UINT32) MemOffset, (UINT32) Data, Be);
#else
      *MemOffset = (UINT32)Data;
#endif
    } else {
      //
      // Sideband access.
      //
      //
      // If we're writing all bits for each byte enable, skip the read.
      // Else, need to read/modify/write register.
      //
#ifndef SIM
#if USE_64_BIT_VARIABLES
      if (ByteEnableToUintX (Be, 0xFFFFFFFFFFFFFFFF, &Temp) != Register.Mask) {
#else
      if (ByteEnableToUintX (Be, 0xFFFFFFFF, &Temp) != Register.Mask) {
#endif
#else
     //TB does not handle ByteEnables so force Read Modify Write
     if (1){
#endif
        TempRegister.Mask   = (UINT32) ~Register.Mask;
        TempRegister.Offset = Register.Offset;
        TotalValue          = MemRegRead (BoxType, Channel, Instance, TempRegister);
#ifdef SUSSW
        Data                = TotalValue | (Data << Register.ShiftBit);
#else
        Data                = TotalValue | LShiftU64(Data, Register.ShiftBit);
#endif
      }
      MmrcDebugPrint ((MMRC_DBG_REG_ACCESS, "SB write to port=0x%02X; opcode=0x%02X; offset=0x%08X; data=0x%08X; be=0x%01X\n",
        PortId, InstancePortMap[BoxType].WriteOperation, Register.Offset + BaseOffset, (UINT32) Data, Be));
#if SIM || JTAG
      MySimMmio32WriteFast (PortId, Command, Register.Offset + BaseOffset, (UINT32)Data, Be);
#else
      Mmio32Write (MrcData->EcBase + 0xD8, (Register.Offset + BaseOffset) & 0xFFFFFF00);
      Mmio32Write (MrcData->EcBase + 0xD4, (UINT32) Data);
      Mmio32Write (MrcData->EcBase + 0xD0, (((Command) | (PortId << 16) | ((Register.Offset & 0x000000FF) << 8)) + (Be << 4)) );
#endif
    }
  }
}
/**
  MemFieldRead

  @param[in]       BoxType      Unit to select
  @param[in]       Channel
  @param[in]       Instance     Channel under test
  @param[in]       Register

  @return     Value read
**/
UINTX
MemFieldRead (
              IN        UINT8           BoxType,
              IN        UINT8           Channel,
              IN        UINT8           Instance,
              IN        REGISTER_ACCESS Register
              )
{
  UINTX Value;
  //
  // Read the entire register specified.
  //
  Value = MemRegRead (BoxType, Channel, Instance, Register);
  //
  // Shift it over so the field starting at ShiftBit is returned.
  //
#ifdef SUSSW
  Value >>= Register.ShiftBit;
#else
  Value = RShiftU64(Value, Register.ShiftBit);
#endif
  //
  // Return the Value.
  //
  return Value;
}
/**
  MemFieldWrite

  @param[in]       BoxType      Unit to select
  @param[in]       Channel
  @param[in]       Instance     Channel under test
  @param[in]       Register
  @param[in]       Value
  @param[in]       Be

  @return     Value read
**/
VOID
MemFieldWrite (
               IN        UINT8           BoxType,
               IN        UINT8           Channel,
               IN        UINT8           Instance,
               IN        REGISTER_ACCESS Register,
               IN        UINTX           Value,
               IN        UINT8           Be
               )
{
  UINTX TempValue;    // Value passed in is the Value that goes to the Bits, this contains the entire register Value.

  if (Register.Offset == 0xffff) {
    return;
  }
  //
  // Read the 32-bit register.
  //
  TempValue = MemRegRead (BoxType, Channel, Instance, Register);
  //
  // Shift the inputted Value to the correct start location
  //
#ifdef SUSSW
  TempValue |= (TempValue << Register.ShiftBit);
#else
  TempValue |= LShiftU64(TempValue, Register.ShiftBit);
#endif
  //
  // Write the register back.
  //
  MemRegWrite (BoxType, Channel, Instance, Register, TempValue, Be);

  return;
}



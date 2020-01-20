/** @file
  Base IPC library implementation.
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

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include "InternalIpcLib.h"
//[-start-160218-IB07400702-add]//
#include <Library/TimerLib.h>
#include <ChipsetPostCode.h>
//[-end-160218-IB07400702-add]//

/**
  Sends an IPC without waiting.

  @param[in]  BaseAddress  The base address of the IPC register bank
  @param[in]  Command      The message identifier to send in the IPC packet.
  @param[in out]  Buffer       Pointer to buffer associated with MessageId.  This is an optional
                       patrameter that may be NULL for some MessageId values.
                       Note: This buffer can be used for both Input/Output buffer at the same time
                       Whenever used as Output buffer to receive return value, it must be 1 or more DWs.
  @param[in out]  BufferSize   The size, in bytes, of Buffer.  Ignored if Buffer is NULL.

  @retval None
**/
VOID
InternalIpcLibSendCommandWithoutWaiting (
  IN  UINTN      BaseAddress,
  IN  UINT32     Command,
  IN OUT  VOID   *Buffer,      OPTIONAL
  IN OUT  UINTN  BufferSize
  )
{
  MmioWrite32 (BaseAddress + R_IPC_CMD_OFFSET, Command);

//  MmioRead32 (BaseAddress + R_IPC_SPTR_OFFSET),
//  MmioRead32 (BaseAddress + R_IPC_DPTR_OFFSET),
//  MmioRead32 (BaseAddress + R_IPC_WBUF_OFFSET),
//  MmioRead32 (BaseAddress + R_IPC_CMD_OFFSET)));
}

/**
  Sends an IPC without waiting.

  @param[in]  BaseAddress     The base address of the IPC register bank
  @param[in]  Command         The Command identifier to send in the IPC packet.
  @param[in]  CommandType     The Command type to send in the IPC packet.
  @param[in]  ReturnDataSize  Return Data size.
  @param[in out]  Buffer          Pointer to buffer associated with MessageId.  This is an optional
                          patrameter that may be NULL for some MessageId values.
                          Note: This buffer can be used for both Input/Output buffer at the same time
                          Whenever used as Output buffer to receive return value, it must be 1 or more DWs.
  @param[in out]  BufferSize      The size, in bytes, of Buffer.  Ignored if Buffer is NULL.

  @retval IPC_STATUS.Uint32
**/
UINT32
InternalIpcLibSendCommandAndWait (
  IN  UINTN           BaseAddress,
  IN  UINT32          Command,
  IN  IPC_CMD_TYPE    CommandType,
  IN  UINT32          ReturnDataSize,
  IN  OUT  VOID       *Buffer,      OPTIONAL
  IN  OUT  UINTN      BufferSize
  )
{
  UINTN        Index;
  IPC_STATUS   IpcStatus;
//[-start-160218-IB07400702-add]//
  UINT32       IpcTimeOutCount = 1000; // 10 sec timeout
//[-end-160218-IB07400702-add]//
  MmioWrite32 (BaseAddress + R_IPC_CMD_OFFSET, Command);

//[-start-160218-IB07400702-modify]//
  do {
    IpcStatus.Uint32 = MmioRead32 (BaseAddress + R_IPC_STS_OFFSET);
    DEBUG ((EFI_D_ERROR, "IpcStatus = %x\n", IpcStatus.Uint32));
	  MicroSecondDelay(10000); // 0.01 s
    IpcTimeOutCount--;
  } while ((IpcStatus.Bits.Busy == 1) && (IpcTimeOutCount != 0));

  if (IpcTimeOutCount == 0) {
    // IPC TimeOut, Do the reboot
    IpcStatus.Bits.Error = 1;
    CHIPSET_POST_CODE (PEI_FRC_SC_INIT_IPC_CMD_BUSY_TIMEOUT);
    DEBUG ((EFI_D_ERROR, "IPC Timeout, Do the reboot\n"));
    IoWrite8 (0xCf9, 0x0E);
  }
//[-end-160218-IB07400702-modify]//
  
  if ((IpcStatus.Bits.Error == 0)&& (CommandType ==IPC_CMD_READ)) {
    if(Buffer != NULL && BufferSize != 0) {
      for (Index = 0; Index < ReturnDataSize; Index++ ){
        *((UINT8*)Buffer + Index) = MmioRead8(BaseAddress + R_IPC_RBUF_OFFSET + Index);
      }
    }
  }
  return IpcStatus.Uint32;
}

/**
  Checks the IPC channel is available for a new request

  @param[in]  BaseAddress  The base address of the IPC register bank

  @retval  RETURN_SUCCESS    Ready for a new IPC
  @retval  RETURN_NOT_READY  Not ready for a new IPC
**/
RETURN_STATUS
InternalReadyForNewIpc (
  IN      UINTN   BaseAddress
  )
{
  IPC_STATUS   IpcStatus;

  IpcStatus.Uint32 = MmioRead32 (BaseAddress + R_IPC_STS_OFFSET);
  if (IpcStatus.Bits.Busy == 1) {
    return RETURN_NOT_READY;
  } else {
    return RETURN_SUCCESS;
  }
}

/**
  Sends an IPC from the x86 CPU to the PMC and waits for the PMC to process the
  specified opcode.

  @param[in]  BaseAddress  The base address of the IPC register bank
  @param[in]  Command      The Command identifier to send in the IPC packet.
  @param[in]  SubCommand   The SubCommand identifier to send in the IPC packet.
  @param[in, out]  Buffer  Pointer to buffer associated with MessageId.  This is an optional
                           patrameter that may be NULL for some MessageId values.
                           Note: This buffer can be used for both Input/Output buffer at the same time
                           Whenever used as Output buffer to receive return value, it must be 1 or more DWs.
  @param[in,out]  BufferSize   The size, in bytes, of Buffer.  Ignored if Buffer is NULL.

  @retval IPC_STATUS.Uint32
**/
UINT32
InternalIpcSendCommandEx (
  IN      UINTN   BaseAddress,
  IN      UINT32  Command,
  IN      UINT8   SubCommand,
  IN OUT  VOID    *Buffer,  OPTIONAL
  IN OUT  UINTN   BufferSize
  )
{
  UINT8             IpcCommand;
  UINT32            CommandSize;
  UINT32            ReturnDataSize;
  UINT32            BufferData;
  UINT32            tempBufferData;
  UINTN             Index;
  UINTN             OffsetIndex;
  IPC_STATUS        IpcStatus;
  IPC_CMD_TYPE      CmdType;
  UINT32            tempCommand =0;
  BOOLEAN  State;

  IpcStatus.Uint32   = 0;
  IpcCommand = (UINT8)Command;
  ReturnDataSize = 0;
  //
  // Make sure interrupts are disabled
  //
  State = SaveAndDisableInterrupts ();


  switch (IpcCommand) {
    case IPC_CMD_ID_USB3P3:
    case IPC_CMD_ID_NPK:
    case IPC_CMD_ID_BLACKLIST_SEL:
    case IPC_CMD_ID_SOUTH_IP_UNGATE:
    case IPC_CMD_ID_CAPSULE_UPDATE_RESET:
      CmdType = IPC_CMD_NONE;
      CommandSize = 0;
      ReturnDataSize = 0;
      break;

    case IPC_CMD_ID_FW_MSG_CTRL:
      switch(SubCommand) {
        case IPC_SUBCMD_ID_FW_MSG_CTRL_READ:
          CmdType = IPC_CMD_READ;
          ReturnDataSize = sizeof(UINT8);
          CommandSize = 0;
          break;

        case IPC_SUBCMD_ID_FW_MSG_CTRL_WRITE:
          CmdType = IPC_CMD_WRITE;
          CommandSize = 1;
          break;

        default:
          return INVALID_IPC_CMD;
          break;
      }
      break;

    case IPC_CMD_ID_PMC_VER:
      switch(SubCommand) {
        case IPC_SUBCMD_ID_PMC_VER_READ:
          CmdType = IPC_CMD_READ;
          ReturnDataSize = sizeof(UINT8) * 8;
          CommandSize = 0;
          break;

        default:
          return INVALID_IPC_CMD;
          break;
      }
      break;

    case IPC_CMD_ID_PHY_CFG:
      CmdType = IPC_CMD_WRITE;
      CommandSize = 1;
      break;

    case IPC_CMD_ID_PM_DEBUG:
      switch (SubCommand) {
        case IPC_SUBCMD_ID_LPSS_DEBUG_UART_ENABLE:
          CmdType = IPC_CMD_WRITE;
          CommandSize = 0;
          break;

        case IPC_SUBCMD_ID_LPSS_DEBUG_UART_DISABLE:
          CmdType = IPC_CMD_WRITE;
          CommandSize = 0;
          break;

        case IPC_SUBCMD_ID_PM_DEBUG_LTR_IGNORE_REPORT:
          CmdType = IPC_CMD_READ;
          CommandSize = 1;
          ReturnDataSize = sizeof(UINT32);
          break;

        case IPC_SUBCMD_ID_PM_DEBUG_LTR_IGNORE_READ:
           CmdType = IPC_CMD_READ;
           CommandSize = 0;
           ReturnDataSize = sizeof(UINT16);
          break;

        case IPC_SUBCMD_ID_PM_DEBUG_LTR_IGNORE_WRITE:
          CmdType = IPC_CMD_WRITE;
          CommandSize = 2;
          break;
        default:
          return INVALID_IPC_CMD;
          break;
      }
      break;

    case IPC_CMD_ID_PMIC:
       //
       // PMIC will read data is byte size.
       //
      switch(SubCommand) {
        case IPC_SUBCMD_ID_PMIC_READ:
          CmdType = IPC_CMD_READ;
          CommandSize = 2;
          ReturnDataSize = (CommandSize /2 );
          break;

        case IPC_SUBCMD_ID_PMIC_WRITE:
          CmdType = IPC_CMD_WRITE;
          CommandSize = 3;
          ReturnDataSize = (CommandSize /3 );
          break;

        case IPC_SUBCMD_ID_PMIC_READ_MOD_WRITE:
          CmdType = IPC_CMD_READ;
          CommandSize = 4;
          ReturnDataSize = (CommandSize /4);

          break;
        default:
          return INVALID_IPC_CMD;
          break;
      }
      break;

    case IPC_CMD_ID_TELEMETRY:
      switch(SubCommand) {
        case 0:
          CmdType = IPC_CMD_READ;
          ReturnDataSize = sizeof(UINT16);
          CommandSize = 1;
          break;

        case 2:
        case 3:
        case 5:
        case 7:
          CmdType = IPC_CMD_READ;
          ReturnDataSize = sizeof(UINT32);
          CommandSize = 0;
          break;

        case 1:
          CommandSize = 1;
          CmdType = IPC_CMD_WRITE;
          break;

        case 4:
        case 6:
        case 8:
          CommandSize = 4;
          CmdType = IPC_CMD_WRITE;
          break;

        default:
          return INVALID_IPC_CMD;
          break;
      }
      break;

    // EMI/RFI support
    // Note that this command is not valid on BXT-Ax
    case IPC_CMD_ID_EMI_RFI_SUPPORT:
      CmdType        = IPC_CMD_WRITE;
      switch(SubCommand) {
        case IPC_SUBCMD_ID_LCPLL_APPLY_NOW:
        case IPC_SUBCMD_ID_LCPLL_DO_NOT_APPLY_NOW:
          CommandSize    = 8;
          break;

        case IPC_SUBCMD_ID_SSC_APPLY_NOW:
        case IPC_SUBCMD_ID_SSC_DO_NOT_APPLY_NOW:
        default:
          CommandSize    = 16;
          break;
      }

      ReturnDataSize = 0;
      break;

    default:
      return INVALID_IPC_CMD;
      break;
  }

  if(CmdType == IPC_CMD_READ) {
    if((ReturnDataSize > BufferSize)||(NULL == Buffer)){  //cmd 2, 3, 5, 7 Read 4 bytes data.
      return INVALID_IPC_CMD;
    }
  }

  tempCommand |= (SubCommand << 12);
  tempCommand |= (CommandSize << 16);
  tempCommand |= Command;

  //
  // Send data to R_IPC_WBUF_OFFSET.
  //
  if (CommandSize > 0){
    BufferData = 0;

    for (Index = 0; Index < CommandSize; Index++) {
      tempBufferData = *(((UINT8*)Buffer + Index));
      OffsetIndex = Index % 4;


      BufferData |= (tempBufferData << (OffsetIndex * 8));

      if (OffsetIndex == 3){
        MmioWrite32(BaseAddress + R_IPC_WBUF_OFFSET + (Index / 4 * 4), BufferData);
        BufferData = 0;
      } else {
        if (Index == (CommandSize - 1)){
          MmioWrite32(BaseAddress + R_IPC_WBUF_OFFSET + (Index / 4 * 4), BufferData);
        }
      }
    }
  }

  IpcStatus.Uint32 = InternalIpcLibSendCommandAndWait(BaseAddress, tempCommand, CmdType, ReturnDataSize, Buffer, BufferSize);    // Returned 4 bytes (max) is stored in Buffer


  if (IpcStatus.Bits.Error != 0) {
  } else {
    if (CmdType == IPC_CMD_READ) {
      for (Index = 0; Index < ReturnDataSize; Index++){
      }
    }
  }
  //
  // Restore the original interrupt state
  //
  SetInterruptState (State);
  return IpcStatus.Uint32;
}


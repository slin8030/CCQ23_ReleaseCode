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
#include <SaCommonDefinitions.h>
#include <PlatformBaseAddresses.h>
#include <ScRegs/RegsPmc.h>
#include "InternalIpcLib.h"




/**
  Checks the IPC channel is available for a new request

  @retval  EFI_SUCCESS    Ready for a new IPC
  @retval  EFI_NOT_READY  Not ready for a new IPC
**/
EFI_STATUS
EFIAPI
ReadyForNewIpc (
  VOID
  )
{
  UINT32 PciCfgReg;
  UINT32  Value=0;
  UINT32 Addr32;
  UINTN PmcIpc1RegBaseAddr = 0;

  PciCfgReg = (UINT32) MmPciAddress(0, 0, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC, 0x10);  //Bar0 Reg
  Addr32 = MmioRead32 (PciCfgReg) & 0xFFFFFFF0;
  PciCfgReg = (UINT32) MmPciAddress(0, 0, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC, 0x4);  //Sts Reg
  Value = MmioRead32 (PciCfgReg);

  if( Addr32 != 0  &&  Addr32 != 0xFFFFFFF0 && ((Value & 0x06) == 0x06)){
    PmcIpc1RegBaseAddr = Addr32;
  } else {
//    DEBUG ((DEBUG_ERROR, "IPC1 MMIO address %x, status =%x \n", Addr32, Value));
    return EFI_NOT_READY;
  }

  return InternalReadyForNewIpc (PmcIpc1RegBaseAddr);
}

/**
  Sends an IPC from the x86 CPU to the SCU and waits for the SCU to process the
  specified opcode.

  @param[in]  Command          The Command identifier to send in the IPC packet.
  @param[in]  SubCommand       The SubCommand identifier to send in the IPC packet.
  @param[in, out]  Buffer      Pointer to buffer associated with MessageId.  This is an optional
                               patrametert that may be NULL for some MessageId values.
  @param[in, out]  BufferSize  The size, in bytes, of Buffer.  Ignored if Buffer is NULL.

  @retval IPC_STATUS.Uint32
**/
EFI_STATUS
IpcSendCommandEx (
  IN      UINT32  Command,
  IN      UINT8   SubCommand,
  IN OUT  VOID    *Buffer,    OPTIONAL
  IN OUT  UINTN   BufferSize
  )
{

  UINT32 PciCfgReg;
  UINT32  Value=0;
  UINT32 Addr32;
  UINTN PmcIpc1RegBaseAddr = 0;
//[-start-160419-IB07400718-add]//
  IPC_STATUS  IpcStatus;
//[-end-160419-IB07400718-add]//

  PciCfgReg = (UINT32) MmPciAddress(0, 0, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC, 0x10);  //Bar0 Reg
  Addr32 = MmioRead32 (PciCfgReg) & 0xFFFFFFF0;
  PciCfgReg = (UINT32) MmPciAddress(0, 0, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC, 0x4);  //Sts Reg
  Value = MmioRead32 (PciCfgReg);

  if( Addr32 != 0  &&  Addr32 != 0xFFFFFFF0 && ((Value & 0x06) == 0x06)){
    PmcIpc1RegBaseAddr = Addr32;
  } else {
    DEBUG ((DEBUG_ERROR, "Error IPC1 command failed MMIO address %x, status =%x \n", Addr32, Value));
    return EFI_NOT_READY;
  }
  
//[-start-160419-IB07400718-modify]//
  IpcStatus.Uint32 = InternalIpcSendCommandEx (PmcIpc1RegBaseAddr, Command, SubCommand, Buffer, BufferSize);
  if ((IpcStatus.Bits.Error) || (IpcStatus.Uint32 == INVALID_IPC_CMD)) {
    DEBUG ((EFI_D_ERROR, "Error: IpcStatus = %x\n", IpcStatus.Uint32));
    return EFI_DEVICE_ERROR;
  } else {
    return EFI_SUCCESS;
  }
//[-end-160419-IB07400718-modify]//
}


/**
  Sends an IPC from the x86 CPU to the PMC and waits for the PMC to process the
  specified opcode.

  !! Use it only when you are sure that IPC PCI config space is no longer valid !!

  @param[in] PciBar0                    PCI config BAR0 for IPC.
  @param[in] Command                    The Command identifier to send in the IPC packet.
  @param[in] SubCommand                 The SubCommand identifier to send in the IPC packet.
  @param[in, out] Buffer                Pointer to buffer associated with MessageId.  This is an optional
                                        patrametert that may be NULL for some MessageId values.
  @param[in, out] BufferSize            The size, in bytes, of Buffer.  Ignored if Buffer is NULL.
**/
RETURN_STATUS
EFIAPI
IpcSendCommandBar0Ex (
  IN      UINT32  PciBar0,
  IN      UINT32  Command,
  IN      UINT8   SubCommand,
  IN OUT  VOID    *Buffer,    OPTIONAL
  IN OUT  UINTN   BufferSize
  )
{
  if(PciBar0 != 0 && PciBar0 != 0xFFFFFFF0){
    return InternalIpcSendCommandEx (PciBar0, Command, SubCommand, Buffer, BufferSize);
  }
  DEBUG ((DEBUG_ERROR, "IpcSendCommandBar0Ex Error IPC1 command failed, not valid address %x\n", PciBar0));
  return EFI_NOT_READY;

}

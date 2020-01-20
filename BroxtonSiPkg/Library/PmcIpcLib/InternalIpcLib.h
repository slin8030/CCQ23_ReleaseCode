/**@file

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

@par Specification
**/




#ifndef __INTERNAL_IPC_LIB_H__
#define __INTERNAL_IPC_LIB_H__

//
// Consumed library classes
//
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/I2CLib.h>

//
// Produced library class
//
#include <Library/PmcIpcLib.h>

//
// PMC IPC1 Register Base Address
// This base address will be dynamically changed during PCI enum
//
extern UINTN PmcIpc1RegBaseAddr;


/**
  Checks the IPC channel is available for a new request

  @param  BaseAddress  The base address of the IPC register bank

  @retval  EFI_SUCCESS    Ready for a new IPC
  @retval  EFI_NOT_READY  Not ready for a new IPC

**/
RETURN_STATUS
InternalReadyForNewIpc (
  IN      UINTN   BaseAddress
  );

/**
  Sends an IPC from the x86 CPU to the PMC and waits for the PMC to process the
  specified opcode.

  @param  BaseAddress  The base address of the IPC register bank
  @param  MessageId    The message identifier to send in the IPC packet.
  @param  Buffer       Pointer to buffer associated with MessageId.  This is an optional
                       patrametert that may be NULL for some MessageId values.
  @param  BufferSize   The size, in bytes, of Buffer.  Ignored if Buffer is NULL.

  @retval IPC_STATUS.Uint32

**/
UINT32
InternalIpcSendCommandEx (
  IN      UINTN   BaseAddress,
  IN      UINT32  MessageId,
  IN      UINT8   CommandId,
  IN OUT  VOID    *Buffer,    OPTIONAL
  IN OUT  UINTN   BufferSize
  );

#endif

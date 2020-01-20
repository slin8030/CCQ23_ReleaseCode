/** @file
 EventLogDxe Lib functions which relate with connect the device

;******************************************************************************
;* Copyright (c) 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

*/


#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

#include <Guid/DebugMessageVariable.h>


EFI_STATUS
GetDebugMessageBufferInfo (
  OUT  UINT32     *AdmgStartAddr,
  OUT  UINT32     *AdmgEndAddr,
  OUT  UINT32     *AdmgSize,  OPTIONAL
  OUT  UINT32     *TotalSize  OPTIONAL
  )
{
  DEBUG_MESSAGE_HEAD_STRUCTURE        *DebugMessageBuffer;
//[-start-180518-IB08400595-add]//
  DEBUG_MESSAGE_MEMORY_ADDR           *DebugMessageMemAddr;
//[-end-180518-IB08400595-add]//

  if (AdmgStartAddr == NULL || AdmgEndAddr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

//[-start-180518-IB08400595-modify]//
  DebugMessageMemAddr = (DEBUG_MESSAGE_MEMORY_ADDR*)(UINTN)PcdGet32(PcdH2OStoreDebugMsgMemAddressPtr);
  if (DebugMessageMemAddr->DumpDebugMessageHeadAddr == 0xFFFFFFFF) {
    return EFI_NOT_FOUND;
  }
  DebugMessageBuffer = (DEBUG_MESSAGE_HEAD_STRUCTURE *)(UINTN)(DebugMessageMemAddr->DumpDebugMessageHeadAddr);

  *AdmgStartAddr = DebugMessageBuffer->AdmgStartAddr;
  *AdmgEndAddr   = DebugMessageBuffer->AdmgEndAddr;
  if (TotalSize != NULL) {
    *TotalSize = DebugMessageBuffer->TotalSize;
  }
  if (AdmgSize != NULL) {
    *AdmgSize = DebugMessageBuffer->AdmgSize;
  }
//[-end-180518-IB08400595-modify]//

  return EFI_SUCCESS;
}


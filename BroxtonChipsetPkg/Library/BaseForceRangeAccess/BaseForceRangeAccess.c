/** @file
  Library classes for force range to read/write access

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>


EFI_STATUS
EFIAPI
BaseForceRangeAccessInit (
  VOID
  )
{
  UINT32              Address;
  VOID                *Buffer;

  
  Address = PcdGet32(PcdAccessNVMFileBase);
  
  if (Address == 0) {
    Buffer = AllocatePages(EFI_SIZE_TO_PAGES(PcdGet32(PcdCseNvmDataFileMaximumSize)));
    if (Buffer != NULL) {
      PcdSet32(PcdAccessNVMFileBase, (UINT32)(UINTN)Buffer);
    }
  
  }

  return EFI_SUCCESS;
}

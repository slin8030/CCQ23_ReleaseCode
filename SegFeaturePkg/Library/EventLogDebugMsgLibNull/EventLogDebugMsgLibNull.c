/** @file
EventLogDebugMsgLibNull

;******************************************************************************
;* Copyright (c) 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/


#include <Uefi.h>

EFI_STATUS
GetDebugMessageBufferInfo (
  OUT  UINT32     *AdmgStartAddr,
  OUT  UINT32     *AdmgSize,
  OUT  UINT32     *TotalSize, OPTIONAL
  OUT  UINT32     *AdmgEndAddr  OPTIONAL
  )
{
  return EFI_UNSUPPORTED;
}


/** @file
  Dummy implementation for SMM related functions

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


UINT8
EFIAPI
SmmFvbAccessCall (
  IN     UINT8            *InPutBuff,
  IN     UINTN            DataSize,
  IN     UINT8            SubFunNum,
  IN     UINT16           SmiPort
  )
{
  return 0;
}

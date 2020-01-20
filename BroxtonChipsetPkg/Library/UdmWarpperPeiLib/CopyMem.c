/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Base.h>
#include <Library/BaseMemoryLib.h>

/* Copies bytes between buffers */
void * memcpy (void *dest, const void *src, unsigned int count)
{
  return CopyMem (dest, src, (UINTN)count);
}

/* Copies bytes between buffers */
void * memcpy_s (void *dest, unsigned int countA, const void *src, unsigned int count)
{
  return CopyMem (dest, src, (UINTN)count);
}

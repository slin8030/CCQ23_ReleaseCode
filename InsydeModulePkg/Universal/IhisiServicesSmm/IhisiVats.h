/** @file
  Type definition for the IhisiSmm module

;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _IHISI_VATS_H_
#define _IHISI_VATS_H_

#include "IhisiRegistration.h"

UINT32
GetVatsIhisiStatus (
  IN EFI_STATUS                             Status
  );

/**
  AH=00h, Read the specific variable into the specified buffer.

  @retval EFI_SUCCESS        AL 00h = Function succeeded.
  @return Others             AL Returned error code.
**/
EFI_STATUS
KernelVatsRead (
  VOID
  );

/**
  AH=01h, Write the specified buffer to the specific variable.

  @retval EFI_SUCCESS        AL	00h = Function succeeded.
  @return Others             AL	Returned error code.
**/
EFI_STATUS
KernelVatsWrite (
  VOID
  );

/**
  AH=05h, Get next specific variable name and GUID

  @retval EFI_SUCCESS        AL	00h = Function succeeded.
  @return Others             AL	Returned error code.
**/
EFI_STATUS
KernelVatsNext (
  VOID
  );

#endif

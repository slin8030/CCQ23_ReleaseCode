/** @file
  This driver provides IHISI interface in SMM mode

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _IHISI_FETS_H_
#define _IHISI_FETS_H_

#include "IhisiRegistration.h"
/**
  AH=20h,  FetsWrite before process, hook SmmCsSvcIhisiFetsDoBeforeFlashing.

  @retval EFI_SUCCESS    Flash EC successful.
  @return Other          Flash EC failed.
**/
EFI_STATUS
EFIAPI
OemFetsDoBeforeWrite (
  VOID
  );

/**
  AH=20h,  FetsWrite after process, hook SmmCsSvcIhisiFetsDoAfterFlashing.

  @retval EFI_SUCCESS    Flash EC successful.
  @return Other          Flash EC failed.
**/
EFI_STATUS
EFIAPI
OemFetsDoAfterWrite (
  VOID
  );

/**
  AH=20h,  FetsWrite Dos shutown, hook SmmCsSvcIhisiFetsShutdown/SmmCsSvcIhisiFetsReboot.

  @retval EFI_SUCCESS    Flash EC successful.
  @return Other          Flash EC failed.
**/
EFI_STATUS
EFIAPI
ChipsetFetsWriteShutdownMode (
  VOID
  );

/**
  AH=20h,  FetsWrite Ec idle true, hook SmmCsSvcIhisiFetsEcIdle.

  @retval EFI_SUCCESS    Flash EC successful.
  @return Other          Flash EC failed.
**/
EFI_STATUS
EFIAPI
OemFetsEcIdleTrue (
  VOID
  );

/**
  AH=20h,  FetsWrite Ec idle false, hook SmmCsSvcIhisiFetsEcIdle.

  @retval EFI_SUCCESS    Flash EC successful.
  @return Other          Flash EC failed.
**/
EFI_STATUS
EFIAPI
OemFetsEcIdleFalse (
  VOID
  );

/**
  AH=20h, FetsWrite, Flash EC.

  @retval EFI_SUCCESS    Flash EC successful.
  @return Other          Flash EC failed.
**/
EFI_STATUS
EFIAPI
OemFetsWrite (
  VOID
  );

/**
  AH=21h, Get EC part information.hook SmmCsSvcIhisiFetsGetEcPartInfo.

  @retval EFI_SUCCESS     Get EC part information successful.
**/
EFI_STATUS
EFIAPI
OemGetEcPartInfo (
  VOID
  );

#endif

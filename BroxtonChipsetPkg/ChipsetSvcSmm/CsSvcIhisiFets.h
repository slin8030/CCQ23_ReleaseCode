/** @file
 Implement the Chipset Servcie IHISI FETS subfunction for this driver.

***************************************************************************
* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/
#ifndef _CHIPSET_SERVICES_IHISI_FETS_H_
#define _CHIPSET_SERVICES_IHISI_FETS_H_

#include <CsSvcIhisiFbts.h>

#define RST_CNT                      0xCF9
#define V_RST_CNT_FULLRESET          0x0E

/**
  Fets reset system function.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFetsReboot (
  VOID
  );

/**
  Fets shutdown function.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFetsShutdown (
  VOID
  );

/**
  Fets EC idle function

  @param[in] Idle     Ec idle mode.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFetsEcIdle (
  IN BOOLEAN              Idle
  );

/**
  AH=21h(GetEcPartInfo), Get EC part information.

  @param[in, out] EcPartSize        EC part size

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFetsGetEcPartInfo (
  IN OUT UINT32     *EcPartSize
  );

#endif

/** @file
 SMM Chipset Services Library.

 This file contains Ihisi Fets Chipset service Lib function.

***************************************************************************
* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

#include <Uefi.h>
#include <H2OIhisi.h>
#include <Library/SmmChipsetSvcLib.h>
#include <Library/DebugLib.h>
#include <Protocol/H2OSmmChipsetServices.h>

extern H2O_SMM_CHIPSET_SERVICES_PROTOCOL *mSmmChipsetSvc;

/**
  Fets reset system function.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsReboot (
  VOID
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFetsReboot) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFetsReboot == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFetsReboot() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFetsReboot ();
}

/**
  Fets shutdown function.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsShutdown (
  VOID
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFetsShutdown) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFetsShutdown == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFetsShutdown() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFetsShutdown ();
}


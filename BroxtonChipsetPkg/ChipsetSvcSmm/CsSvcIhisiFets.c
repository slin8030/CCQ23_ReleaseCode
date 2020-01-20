/** @file
 Implement the Chipset Servcie IHISI FETS subfunction for this driver.

***************************************************************************
* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
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
#include <Library/DebugLib.h>
#include <Library/SmmOemSvcChipsetLib.h>
#include <Library/SmmServicesTableLib.h>
#include <CsSvcIhisiFets.h>


/**
  Fets reset system function.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFetsReboot (
  VOID
  )
{
  UINT8        Buffer;

  Buffer = 0;
  Buffer = V_RST_CNT_FULLRESET;
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, RST_CNT, 1, &Buffer );
  return EFI_SUCCESS;
}

/**
  Fets shutdown function.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFetsShutdown (
  VOID
  )
{
  IhisiFbtsShutDown ();
  return EFI_SUCCESS;
}


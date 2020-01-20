/*++

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  ExI.c

Abstract:

  ExI configuration based on setup option


--*/


#include "PlatformDxe.h"

#define PchLpcPciCfg32(Register)  MmioRead32 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_LPC, 0, Register))

//----------------------------------------------------------------------------
// Procedure: GetPmcBase
//
// Description: This function read content of B:D:F 0:31:0, offset 44h (for
// PmcBase)
//
// Input: None
//
// Output:  32 bit PmcBase
//
//----------------------------------------------------------------------------
UINT32
GetPmcBase (
  VOID
  )
{
  return (PchLpcPciCfg32 (R_LPC_PMC_BASE) & B_LPC_PMC_BASE_BAR);
}

/**
  Configure ExI.

  @param ImageHandle   - Pointer to the loaded image protocol for this driver
  @param SystemTable   - Pointer to the EFI System Table

  @retval EFI_SUCCESS   The driver initializes correctly.
**/
VOID
InitExI (
  )
{

  if (mSystemConfiguration.ExISupport == 1) {
	  MmioOr32 ((UINTN) (GetPmcBase() + R_PCH_PMC_MTPMC1), (UINT32) BIT0+BIT1+BIT2);
  } else if (mSystemConfiguration.ExISupport == 0) {
    MmioAnd32 ((UINTN) (GetPmcBase() + R_PCH_PMC_MTPMC1), ~((UINT32) BIT0+BIT1+BIT2)); //clear bit 0,1,2
  }
}

/** @file
  This driver provides IHISI interface in SMM mode

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

#ifndef _IHISI_FBTS_H_
#define _IHISI_FBTS_H_

#include "IhisiSmm.h"
#include <H2OIhisi.h>
#include <ChipsetSetupConfig.h>
#include <Library/IoLib.h>
#include <Library/FdSupportLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/H2OIhisi.h>
#include <Protocol/SmmFwBlockService.h>

//[-start-160810-IB11270161-add]//
typedef enum {
  APL_DESC_REGION              = 0,
  APL_BIOS_REGION,
  APL_ME_REGION,
  APL_GBE_REGION,
  APL_PLATFORM_DATA_REGION,
  APL_DEVICE_EXPANSION,
  APL_MAX_FLASH_REGION,
  APL_ME_SUB_REGION_IGNITION   = 0x20,
  APL_REGION_TYPE_OF_EOS       = 0xff,
} APL_FLASH_REGION_TYPE;  
//[-end-160810-IB11270161-add]//
/**
  Passing information to flash program on whether
  if current BIOS (AT-p capable) can be flashed with
  other BIOS ROM file
  AT-p: (Anti-Theft PC Protection).

  @retval EFI_SUCCESS   Success returns.
**/
EFI_STATUS
FbtsGetATpInformation (
  VOID
  );

/**
  Get platform and Rom file flash descriptor region.

  @retval EFI_SUCCESS   Success returns.
**/
EFI_STATUS
GetRomFileAndPlatformTable (
  VOID
  );

/**
  Initialize Fbts relative services

  @retval EFI_SUCCESS        Initialize Fbts services successful.
  @return Others             Any error occurred.
**/
EFI_STATUS
FbtsInit (
  VOID
  );

EFI_STATUS
FbtsApHookForBios (
  VOID
  );
#endif

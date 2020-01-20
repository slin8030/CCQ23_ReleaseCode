/** @file

;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _DXE_INSYDE_CHIPSET_LIBRARY_H_
#define _DXE_INSYDE_CHIPSET_LIBRARY_H_

#include <ChipsetSetupConfig.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>

EFI_STATUS
EFIAPI
GetChipsetSetupVariableDxe (
  IN OUT CHIPSET_CONFIGURATION          *SetupVariable,
  IN     UINTN                          SetupVariableSize
  );

#endif

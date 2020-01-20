/*++

Copyright (c)  1999 - 2014 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:
  PlatformInfoPei.c

Abstract:
  Platform Info Driver.

--*/

#include "PlatformInit.h"

#define LEN_64M       0x4000000
//
// Default PCI32 resource size
//
#define RES_MEM32_MIN_LEN   0x38000000

#define RES_IO_BASE   0x0D00
#define RES_IO_LIMIT  0xFFFF

#define MemoryCeilingVariable   L"MemCeil."

EFI_STATUS
PlatformInfoUpdate (
  IN CONST EFI_PEI_SERVICES          **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB *PlatformInfoHob,
  IN SYSTEM_CONFIGURATION      *SystemConfiguration
  )
{
  EFI_STATUS                   Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI   *Variable;
  UINTN                        VariableSize;
  UINT32                       MemoryCeiling;

  //
  // Checking PCI32 resource from previous boot to determine the memory ceiling
  //
  Status = (*PeiServices)->LocatePpi (
                             PeiServices,
                             &gEfiPeiReadOnlyVariable2PpiGuid,
                             0,
                             NULL,
                             (VOID **) &Variable
                             );
  if (!EFI_ERROR(Status)) {
    //
    // Get the memory ceiling
    //
    VariableSize = sizeof(MemoryCeiling);
    Status = Variable->GetVariable (Variable,
                                       MemoryCeilingVariable,
                                       &gEfiGlobalVariableGuid,
                                       NULL,
                                       &VariableSize,
                                       &MemoryCeiling);
    if(!EFI_ERROR(Status)) {
      //
      // Set the new PCI32 resource Base if the variable available
      //
      PlatformInfoHob->PciData.PciResourceMem32Base = MemoryCeiling;
      PlatformInfoHob->MemData.MemMaxTolm = MemoryCeiling;
      PlatformInfoHob->MemData.MemTolm = MemoryCeiling;
      //
      // Platform PCI MMIO Size in unit of 1MB
      //
      PlatformInfoHob->MemData.MmioSize = 0x1000 - (UINT16)(PlatformInfoHob->MemData.MemMaxTolm >> 20);
    }
  }

  //
  // Clear TPM Command CMOS area -need to check later -joosong
  //
  //IoWrite8(R_ICH_RTC_EXT_INDEX, TCG_CMOS_AREA_OFFSET);
  //IoWrite8(R_ICH_RTC_EXT_TARGET, 0);

  return EFI_SUCCESS;
}

EFI_STATUS
InitializePlatform (
  IN CONST EFI_PEI_SERVICES       **PeiServices,
  IN EFI_PLATFORM_INFO_HOB        *PlatformInfoHob,
  IN SYSTEM_CONFIGURATION         *SystemConfiguration
)
/*++
Routine Description:
  Initialize the platform related info hob according to the
  pre-determine value or setup option
Arguments:

Returns:
  EFI_SUCCESS  -  Memory initialization completed successfully.
  Others       -  All other error conditions encountered result in an ASSERT.
--*/
{
// -- cchew10 need to update here.

  return EFI_SUCCESS;
}


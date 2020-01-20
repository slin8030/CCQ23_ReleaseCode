/*++

Copyright (c)  1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:
  PlatformCpuInfoDxe.c

Abstract:
  Platform Cpu Info driver to public platform related HOB data

--*/

#include "PlatformCpuInfoDxe.h"
CHAR16    EfiPlatformCpuInfoVariable[] = L"PlatformCpuInfo";

EFI_STATUS
EFIAPI
PlatformCpuInfoInit (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
/*++

Routine Description:
  Entry point for the driver.

  This routine get the platform HOB data from PEI and publish
  as Platform Info Protocol.

Arguments:

  ImageHandle  -  Image Handle.
  SystemTable  -  EFI System Table.

Returns:

  EFI_SUCCESS  -  Function has completed successfully.

--*/
{
  EFI_STATUS                  Status;
  EFI_PLATFORM_CPU_INFO       *PlatformCpuInfoPtr;
  EFI_PEI_HOB_POINTERS        GuidHob;
  //
  // Get Platform Cpu Info HOB
  //
  GuidHob.Raw = GetHobList ();
  while ((GuidHob.Raw = GetNextGuidHob (&gEfiPlatformCpuInfoGuid, GuidHob.Raw)) != NULL) {
    PlatformCpuInfoPtr = GET_GUID_HOB_DATA (GuidHob.Guid);
    GuidHob.Raw = GET_NEXT_HOB (GuidHob);
      //
      // Write the Platform CPU Info to volatile memory for runtime purposes.
      // This must be done in its own driver because SetVariable protocol is dependent on chipset,
      // which is dependent on CpuIo, PlatformInfo, and Metronome.
      //
      Status = gRT->SetVariable(
                      EfiPlatformCpuInfoVariable,
                      &gEfiBxtVariableGuid,
                      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      sizeof(EFI_PLATFORM_CPU_INFO),
                      PlatformCpuInfoPtr
                      );
      if (EFI_ERROR(Status)) {
        return Status;
      }
  }

   return EFI_SUCCESS;
}


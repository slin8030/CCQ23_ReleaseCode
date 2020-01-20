/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:
  PlatformInfoDxe.c
  
Abstract:
  Platform Info driver to public platform related HOB data

--*/

#include "PlatformInfoDxe.h"
#include <Guid/BxtVariable.h>


EFI_STATUS
EFIAPI
PlatformInfoInit (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
/*++

Routine Description:
  Entry point for the driver.

  This routine get the platform HOB data from PEI and publish
  as Platform Info variable that can be accessed during boot service and
  runtime.

Arguments:

  ImageHandle  -  Image Handle.
  SystemTable  -  EFI System Table.
  
Returns:

  Status  -  Function execution status.
  
--*/
{
  EFI_STATUS                  Status;
  EFI_PLATFORM_INFO_HOB       *PlatformInfoHobPtr;
  EFI_PEI_HOB_POINTERS        GuidHob;

  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw == NULL) {
  	return EFI_NOT_FOUND;
  }
  
  if ((GuidHob.Raw = GetNextGuidHob (&gEfiPlatformInfoGuid, GuidHob.Raw)) != NULL) {
    PlatformInfoHobPtr = GET_GUID_HOB_DATA (GuidHob.Guid);
      //
      // Write the Platform Info to volatile memory
      //
      Status = gRT->SetVariable(
                      L"PlatformInfo",
                      &gEfiBxtVariableGuid,
                      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      sizeof(EFI_PLATFORM_INFO_HOB),
                      PlatformInfoHobPtr
                  );
      if (EFI_ERROR(Status)) {
        return Status;
      }
    }

  return EFI_SUCCESS;
}


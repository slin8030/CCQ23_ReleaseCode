/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/
#include "BdsPlatform.h"
#include <Guid/Acpi.h>


//#include <EfiFastBoot.h>

EFI_STATUS
InstallLegacyAcpi (
  ) 
{
  	  
  VOID                              *AcpiTable;
  VOID                              *AcpiPtr;
  EFI_STATUS                         Status;
  UINTN                             CopySize;
//  EFI_GUID gEfiAcpi20TableGuid            = { 0x8868E871, 0xE4F1, 0x11D3, { 0xBC, 0x22, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81 }}
//  EFI_GUID gEfiAcpi10TableGuid            = { 0xEB9D2D30, 0x2D88, 0x11D3, { 0x9A, 0x16, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D }}
  
  AcpiTable = NULL;
  Status = EfiGetSystemConfigurationTable (
             &gEfiAcpi20TableGuid,
             &AcpiTable
             );
  if (EFI_ERROR (Status)) {
    Status = EfiGetSystemConfigurationTable (
               &gEfiAcpi10TableGuid,
               &AcpiTable
               );
  }
  
  if (AcpiTable == NULL) {
    DEBUG ((EFI_D_ERROR, "ACPI table is not found!\n"));
    return EFI_NOT_FOUND;
  }
  
  AcpiPtr = AcpiTable;
  if (*((UINT8 *) AcpiPtr + 15) == 0) { // ACPI 1.0
    CopySize = 20;
  } else {
    AcpiPtr   = ((UINT8 *) AcpiPtr + 20);
    CopySize  = (*(UINT32 *) AcpiPtr);
  }

  CopyMem (
    (VOID *)(UINTN) 0xE8000, // Hardcode.
    AcpiTable,
    CopySize
    );
  
  return EFI_SUCCESS;
}
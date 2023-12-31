/*++

Copyright (c)  1999 - 2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  Osfr.h

Abstract:

  This file describes the contents of the ACPI OSFR Table.

--*/

#ifndef _OSFR_H
#define _OSFR_H

//
// Statements that include other files
//

#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>

#pragma pack (1)

#define EFI_ACPI_OSFR_TABLE_REVISION            0x1
#define EFI_ACPI_OSFR_TABLE_SIGNATURE           SIGNATURE_32('O', 'S', 'F', 'R')  //'RFSO'

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER          Header;
  UINT32                               ObjectCount;
  UINT32                               TableDWORDs [64];
} EFI_ACPI_OSFR_TABLE;

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER          Header;
  UINT32                               ObjectCount;
} EFI_ACPI_OSFR_TABLE_FIXED_PORTION;

typedef struct {
  EFI_GUID  ObjectUUID;
  UINT32    Reserved1;
  UINT32    ManufacturerNameStringOffset;
  UINT32    ModelNameStringOffset;
  UINT32    Reserved2;
  UINT32    MicrosoftReferenceOffset;
} EFI_ACPI_OSFR_OCUR_OBJECT;

#pragma pack ()

#endif

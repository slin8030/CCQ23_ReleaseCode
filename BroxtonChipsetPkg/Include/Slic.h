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

  Slic.h

Abstract:

  This file describes the contents of the ACPI SLIC Table.

--*/

#ifndef _SLIC_H
#define _SLIC_H

//
// Statements that include other files
//

#include <IndustryStandard/Acpi20.h>

#pragma pack (1)

#define EFI_ACPI_SLIC_TABLE_SIGNATURE           0x43494C53  //"SLIC"
#define EFI_ACPI_SLIC_TABLE_REVISION            0x1

typedef struct {
  UINT32              StructureType;
  UINT32              LicensingDataLength;
  UINT8               bType;
  UINT8               bVersion;
  UINT16              Reserved;
  UINT32              aiKeyAlg;
  UINT32              Magic;
  UINT32              BitLength;
  UINT32              PublicKeyExponent;
  UINT8               Modulus [128];
} EFI_ACPI_SLIC_OEM_PUBLIC_KEY;

typedef struct {
  UINT32              StructureType;
  UINT32              LicensingDataLength;
  UINT32              dwVersion;
  UINT8               sOEMID [6];
  UINT64              sOEMTABLEID;
  UINT8               sWindowsFlag [8];
  UINT32              Reserved0;
  UINT8               Reserved1 [8];
  UINT8               Reserved2 [8];
  UINT8               Signature [128];
} EFI_ACPI_SLIC_SLP_MARKER;

typedef struct {
  EFI_ACPI_SLIC_OEM_PUBLIC_KEY  PublicKeyFileBlob;
  UINT8                         Signature [128];
} EFI_ACPI_SLIC_SIGNED_OEM_PUBLIC_KEY;

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER          Header;
  EFI_ACPI_SLIC_OEM_PUBLIC_KEY         OemPublicKey;
  EFI_ACPI_SLIC_SLP_MARKER             SlpMarker;
} EFI_ACPI_SLIC_TABLE;

#pragma pack ()

#endif

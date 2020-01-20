/** @file
  Energe Management Table extra.

@copyright
 Copyright (c) 1996 - 2016 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains an 'Intel Peripheral Driver' and is uniquely
 identified as "Intel Reference Module" and is licensed for Intel
 CPUs and chipsets under the terms of your license agreement with
 Intel or your vendor. This file may be modified by the user, subject
 to additional terms of the license agreement.

@par Specification
**/

#ifndef _EMT1_H
#define _EMT1_H

//
// Statements that include other files
//
#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include <IndustryStandard/Acpi30.h>

#define EFI_ACPI_EMT1_REVISION                          0x00000000
#define EFI_ACPI_OEM1_REVISION                          0x00000001

#define EFI_ACPI_EMT1_SIGNATURE                         SIGNATURE_32('E', 'M', '_', '1')
#define EFI_ACPI_OEM1_SIGNATURE                         SIGNATURE_32('O', 'E', 'M', '1')

#define EFI_ACPI_EMT1_TABLE_ID     SIGNATURE_64('O','E','M','1',' ',' ',' ',' ') ///< OEM table id 8 bytes long
#define EFI_ACPI_OEM1_TABLE_ID     SIGNATURE_64('E','N','R','G','Y','M','G','T') ///< OEM table id 8 bytes long

#pragma pack(1)

typedef struct {
    EFI_ACPI_DESCRIPTION_HEADER Header;
    UINT16   IaAppsRun;
    UINT8    IaAppsCap;
    UINT8    CapOrVoltFlag;
    UINT8    BootOnInvalidBatt;
} EFI_ACPI_ENERGY_MANAGEMENT_1_TABLE;

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT8                       FixedOption0;
  UINT8                       FixedOption1;
  UINT8                       DBIInGpioNumber;
  UINT8                       DBIOutGpioNumber;
  UINT8                       BatChpType;
  UINT16                      IaAppsRun;
  UINT8                       BatIdDBIBase;
  UINT8                       BatIdAnlgBase;
  UINT8                       IaAppsCap;
  UINT16                      VBattFreqLmt;
  UINT8                       CapFreqIdx;
  UINT8                       Rsvd1;
  UINT8                       BatIdx;
  UINT8                       IaAppsToUse;
  UINT8                       TurboChrg;
  UINT8                       Rsvd2[11];
} EFI_ACPI_EM_OEM_1_TABLE;

#pragma pack()

#endif

/** @file

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

#ifndef _PIDV_H_
#define _PIDV_H_

//
// Statements that include other files
//
#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include <IndustryStandard/Acpi30.h>

//
// Definitions
//
#define EFI_ACPI_OEM_PIDV_REVISION                    0x00000002
#define EFI_ACPI_PLATFORM_ID_V_SIGNATURE              SIGNATURE_32('P', 'I', 'D', 'V')
#define PIDV_PART_NUMBER_LEN_MAX                      32
#define PIDV_EXT_ID_LEN_MAX                           32
#define PIDV_SYSTEM_UUID_LEN_MAX                      16
#define EFI_ACPI_CREATOR_ID                           SIGNATURE_32 ('B', 'R', 'X', 'T')
#define EFI_ACPI_CREATOR_REVISION                     0x0100000D

#pragma pack(1)

typedef struct {
  UINT8 Fru1:4;
  UINT8 Fru2:4;
  UINT8 Fru3:4;
  UINT8 Fru4:4;
  UINT8 Fru5:4;
  UINT8 Fru6:4;
  UINT8 Fru7:4;
  UINT8 Fru8:4;
  UINT8 Fru9:4;
  UINT8 Fru10:4;
  UINT8 Fru11:4;
  UINT8 Fru12:4;
  UINT8 Fru13:4;
  UINT8 Fru14:4;
  UINT8 Fru15:4;
  UINT8 Fru16:4;
  UINT8 Fru17:4;
  UINT8 Fru18:4;
  UINT8 Fru19:4;
  UINT8 Fru20:4;
} FIELD_REPLACEABLE_UNITS;

//
// SPID - 32 bytes
//
typedef struct {
  UINT16                   CustomerID;
  UINT16                   VendorID;
  UINT16                   DeviceManufacturerID;
  UINT16                   PlatformFamilyID;
  UINT16                   ProductLineID;
  UINT16                   HardwareID;
  FIELD_REPLACEABLE_UNITS  Fru;
  UINT16                   Reserved[5];
} SOFTWARE_PLATFORM_ID;
//
// PIDV structure
//
//
// Ensure proper structure formats
//
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT8                       PartNumber[32];
  SOFTWARE_PLATFORM_ID        ExtID1;
  UINT8                       ExtID2[32];
  UINT8                       SystemUuid[16];
  UINT8                       IaFwBuildId[32];  /* unused on Revision#2 */
  UINT32                      IaSvn;            /* unused on Revision#2 */
  UINT32                      SecSvn;           /* unused on Revision#2 */
  UINT32                      PdrSvn;           /* unused on Revision#2 */
  UINT16                      IaFwRevValues[4]; /* unused on Revision#2 */
  UINT16                      SecRevValues[4];  /* unused on Revision#2 */
  UINT16                      PdrRevValues[4];  /* unused on Revision#2 */
  UINT32                      OEM_TAG;          /* unused on Revision#2 */
} EFI_ACPI_PLATFORM_ID_V_TABLE;
#pragma pack()

#endif

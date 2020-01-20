/** @file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2005 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef _BDAT_H_
#define _BDAT_H_

//
// BDAT definitions
//
#define BDAT_PRIMARY_VER    0x0004
#define BDAT_SECONDARY_VER  0x0000

#define MAX_SCHEMA_LIST_LENGTH 4

#pragma pack(1)

typedef struct {
    UINT8   BiosDataSignature[8]; // "BDATHEAD"
    UINT32  BiosDataStructSize;   // sizeof BDAT_STRUCTURE
    UINT16  Crc16;                // 16-bit CRC of BDAT_STRUCTURE (calculated with 0 in this field)
    UINT16  Reserved;
    UINT16  PrimaryVersion;       // Primary version
    UINT16  SecondaryVersion;     // Secondary version
    UINT32  OemOffset;            // Optional offset to OEM-defined structure
    UINT32  Reserved1;
    UINT32  Reserved2;
} BDAT_HEADER_STRUCTURE;

typedef struct bdatSchemaList {
    UINT16  SchemaListLength;
    UINT16  Reserved0;
    UINT16  Year;
    UINT8   Month;
    UINT8   Day;
    UINT8   Hour;
    UINT8   Minute;
    UINT8   Second;
    UINT8   Reserved1;
    UINT32  Schemas[MAX_SCHEMA_LIST_LENGTH];
} BDAT_SCHEMA_LIST_STRUCTURE;

typedef struct bdatStruct {
    BDAT_HEADER_STRUCTURE        bdatHeader;
    BDAT_SCHEMA_LIST_STRUCTURE   BdatSchemas;
} BDAT_STRUCTURE;

typedef struct {
  EFI_GUID                     SchemaId;
  UINT32                       DataSize;
  UINT16                       Crc16;
} BDAT_SCHEMA_HEADER_STRUCTURE;

#pragma pack()

#endif // _BDAT_H_

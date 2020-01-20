/** @file
  Header file for SMBIOS related functions.

@copyright
 Copyright (c) 2013 - 2016 Intel Corporation. All rights reserved
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

@par Specification Reference:
  System Management BIOS (SMBIOS) Reference Specification v2.8.0
  dated 2013-Mar-28 (DSP0134)
  http://www.dmtf.org/sites/default/files/standards/documents/DSP0134_2.8.0.pdf
**/

#ifndef _SMBIOS_CPU_H_
#define _SMBIOS_CPU_H_

#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>
#include <Protocol/Smbios.h>
#include <Protocol/CpuInfo.h>
#include <IndustryStandard/SmBios.h>
#include <CpuAccess.h>
#include <Private/CpuInitDataHob.h>
#include <Library/PcdLib.h>

///
/// Non-static SMBIOS table data to be filled later with a dynamically generated value
///
#define TO_BE_FILLED  0
#define TO_BE_FILLED_STRING  " "        ///< Initial value should not be NULL

///
/// String references in SMBIOS tables. This eliminates the need for pointers. See spec for details.
///
#define NO_STRING_AVAILABLE  0
#define STRING_1  1
#define STRING_2  2
#define STRING_3  3
#define STRING_4  4
#define STRING_5  5
#define STRING_6  6
#define STRING_7  7

#define BRAND_STRING_UNSUPPORTED  "CPU Brand String Not Supported"
#define INTEL_CORPORATION_STRING  "Intel(R) Corporation"

///
/// This constant defines the maximum length of the CPU brand string. According to the
/// IA manual, the brand string is in EAX through EDX (thus 16 bytes) after executing
/// the CPUID instructions with EAX as 80000002, 80000003, 80000004.
///
#define MAXIMUM_CPU_BRAND_STRING_LENGTH 48

///
/// SMBIOS Table values with special meaning
///
#define SMBIOS_TYPE4_64BIT_CAPABLE            BIT2
#define SMBIOS_TYPE4_MULTI_CORE               BIT3
#define SMBIOS_TYPE4_HARDWARE_THREAD          BIT4
#define SMBIOS_TYPE4_EXECUTE_PROTECTION       BIT5
#define SMBIOS_TYPE4_ENHANCED_VIRTUALIZATION  BIT6
#define SMBIOS_TYPE4_POWER_PERF_CONTROL       BIT7

#define SMBIOS_TYPE7_USE_64K_GRANULARITY     0x8000

#pragma pack(1)
typedef struct {
  CHAR8 *Socket;
  CHAR8 *ProcessorManufacture;
  CHAR8 *ProcessorVersion;
  CHAR8 *SerialNumber;
  CHAR8 *AssetTag;
  CHAR8 *PartNumber;
} SMBIOS_TYPE4_STRING_ARRAY;
#define SMBIOS_TYPE4_NUMBER_OF_STRINGS  6

typedef struct {
  CHAR8 *SocketDesignation;
} SMBIOS_TYPE7_STRING_ARRAY;
#define SMBIOS_TYPE7_NUMBER_OF_STRINGS  1
#pragma pack()

typedef enum {
  CpuStatusUnknown        = 0,
  CpuStatusEnabled        = 1,
  CpuStatusDisabledByUser = 2,
  CpuStatusDisabledbyBios = 3,
  CpuStatusIdle           = 4,
  CpuStatusOther          = 7
} SMBIOS_TYPE4_CPU_STATUS;

typedef struct {
  UINT8      CpuStatus       :3;
  UINT8      Reserved1       :3;
  UINT8      SocketPopulated :1;
  UINT8      Reserved2       :1;
} PROCESSOR_STATUS_DATA;

typedef struct {
  UINT16  Level           :3;
  UINT16  Socketed        :1;
  UINT16  Reserved1       :1;
  UINT16  Location        :2;
  UINT16  Enable          :1;
  UINT16  OperationalMode :2;
  UINT16  Reserved2       :6;
} SMBIOS_TYPE7_CACHE_CONFIGURATION_DATA;

extern EFI_SMBIOS_PROTOCOL       *mSmbios;
extern EFI_SMBIOS_HANDLE         mSmbiosL1CacheHandle;
extern EFI_SMBIOS_HANDLE         mSmbiosL2CacheHandle;
extern EFI_SMBIOS_HANDLE         mSmbiosL3CacheHandle;

/**
  Add an SMBIOS table entry using EFI_SMBIOS_PROTOCOL.
  Create the full table record using the formatted section plus each non-null string, plus the terminating (double) null.

  @param[in]  Entry                 The data for the fixed portion of the SMBIOS entry.
                                    The format of the data is determined by EFI_SMBIOS_TABLE_HEADER.
                                    Type. The size of the formatted area is defined by 
                                    EFI_SMBIOS_TABLE_HEADER. Length and either followed by a 
                                    double-null (0x0000) or a set of null terminated strings and a null.
  @param[in]  TableStrings          Set of string pointers to append onto the full record. 
                                    If TableStrings is null, no strings are appended. Null strings
                                    are skipped.
  @param[in]  NumberOfStrings       Number of TableStrings to append, null strings are skipped.
  @param[out] SmbiosHandle          A unique handle will be assigned to the SMBIOS record.

  @retval     EFI_SUCCESS           Table was added.
  @retval     EFI_OUT_OF_RESOURCES  Table was not added due to lack of system resources.
**/
EFI_STATUS
AddSmbiosTableEntry (
  IN  EFI_SMBIOS_TABLE_HEADER *Entry,
  IN  CHAR8                   **TableStrings,
  IN  UINT8                   NumberOfStrings,
  OUT EFI_SMBIOS_HANDLE       *SmbiosHandle
  );

/**
  This function adds a SMBIOS type 4 (processor) entry.

  @retval EFI_SUCCESS           - if the data is successfully reported.
  @retval EFI_OUT_OF_RESOURCES  - if not able to get resources.
  @retval EFI_INVALID_PARAMETER - if a required parameter in a subfunction is NULL.
**/
EFI_STATUS
InstallSmbiosType4 (
  VOID
  );

/**
  This function adds a SMBIOS type 7 (cache) entry.

  @retval EFI_SUCCESS           - if the data is successfully reported.
  @retval EFI_OUT_OF_RESOURCES  - if not able to get resources.
  @retval EFI_INVALID_PARAMETER - if a required parameter in a subfunction is NULL.
**/
EFI_STATUS
InstallSmbiosType7 (
  VOID
  );

#endif

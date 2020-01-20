/** @file
  Common defines and definitions for a dxe driver.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef _OFFLINE_CRASH_DUMP_DXE_H_
#define _OFFLINE_CRASH_DUMP_DXE_H_

#include <PiDxe.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo.h>
#include <Library/DevicePathLib.h>
#include <Guid/SetupVariable.h>
#include "CpuContext.h"
//[-start-160817-IB03090432-modify]//
#include <Scregs/RegsPmc.h>
//[-end-160817-IB03090432-modify]//
#include <CMOSMap.h>
#include <Guid/BxtVariable.h>
#include <Library/HobLib.h>

//
// GUID and structure that defined in Windows Offline Crash Dump Specification
// v3.21 section 4.1
//
#define OFFLINE_CRASHDUMP_CONFIGURATION_TABLE_GUID \
  { \
    0x3804CF02, 0x8538, 0x11E2, 0x88, 0x47, 0x8D, 0xF1, 0x60, 0x88, 0x70, 0x9B \
  }

typedef struct _OFFLINE_CRASHDUMP_CONFIGURATION_TABLE {
  UINT32                             Version;
  UINT32                             AbnormalResetOccurred;
  UINT32                             OfflineMemoryDumpCapable;
  UINT64                             ResetDataAddress;
  UINT32                             ResetDataSize;
} OFFLINE_CRASHDUMP_CONFIGURATION_TABLE;

#define OFFLINE_CRASHDUMP_VENDOR_GUID \
  { \
    0x77fa9abd, 0x0359, 0x4d32, 0xbd, 0x60, 0x28, 0xf4, 0xe7, 0x8f, 0x78, 0x4b \
  }

#define OFFLINE_CRASHDUMP_DEDICATED_PARTITION_GUID \
  { \
    0x66C9B323, 0xF7FC, 0x48B6, 0xBF, 0x96, 0x6F, 0x32, 0xE3, 0x35, 0xA4, 0x28 \
  }

#pragma pack(1)

typedef struct _RAW_DUMP_HEADER {
  UINT64 Signature;    // Raw_Dmp!
  UINT32 Version;      // 0x1000
  UINT32 Flags;        // 0 : dump valid; 1 : dump incomplete
  UINT64 OsData;
  UINT64 CpuContext;   // Not supported. will be set to 0
  UINT32 ResetTrigger; // sv proprietary data. should be set to the same as abnormalesetoccurred value.
  UINT64 DumpSize;     // Total size of the dump including header and data
  UINT64 TotalDumpSizeRequired;
  UINT32 SectionsCount;
} RAW_DUMP_HEADER;

typedef struct _CPU_CONTEXT {
  UINT16 Architecture;
  UINT32 CoreCount;
} CPU_CONTEXT;

typedef struct _RAW_DUMP_DDR_RANGE {
  UINT64 BaseAddress;
} RAW_DUMP_DDR_RANGE;

typedef struct _RAW_DUMP_CPU_CONTEXT {
  UINT16 Architecture;
  UINT32 CoreCount;
} RAW_DUMP_CPU_CONTEXT;

typedef struct _RAW_DUMP_SV_SPECIFIC {
  UINT8  SVSpecific[16];
} RAW_DUMP_SV_SPECIFIC;

typedef struct _RAW_DUMP_SECTION_HEADER {
  UINT32 Flags;       // 0 : dump valid; 2 : Insufficient
  UINT32 Version;     // 0x1000
  UINT32 Type;        // 0 : reserved; 1 : DDR range; 2 : CPU context; 3 : SV Specific; 4... Reserved.
  UINT64 Offset;      // Byte offset of the dump data from the start of this section header corresponding to it.
  UINT64 Size;        // Size of the section in bytes.
  union {
    RAW_DUMP_DDR_RANGE   DdrRangeRaw;
    RAW_DUMP_CPU_CONTEXT CpuContextRaw;
    RAW_DUMP_SV_SPECIFIC SvSpecificRaw;
  } TypeSpecificInformation;
  UINT8  Name[20];    // 20 byte name field.
} RAW_DUMP_SECTION_HEADER;

typedef struct _CRASH_DUMP_MEMORY_MAP_DESCRIPTOR {
  UINTN                       MemoryMapSize;
  UINTN                       MapKey;
  UINTN                       DescriptorSize;
  UINT32                      DescriptorVersion;
} CRASH_DUMP_MEMORY_MAP_DESCRIPTOR;

#pragma pack()

#define Architecture_X86    0
#define Architecture_X64    9

#endif

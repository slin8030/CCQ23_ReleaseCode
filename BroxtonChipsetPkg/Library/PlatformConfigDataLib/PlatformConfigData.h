/** @file
  IPC based PlatformFvbLib library instance

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#ifndef __PLATFORM_CONFIG_DATA_H__
#define __PLATFORM_CONFIG_DATA_H__
#include <UEFI.h>
#include <Base.h>
#include <Pi/PiMultiPhase.h>
#include <Library/PlatformConfigDataLib.h>
#include <Library/HobLib.h>
#include <Guid/PlatformConfigDataGuid.h>
//#include <Guid/SetupVariable.h>
#include <ChipsetSetupConfig.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/DebugLib.h>
//[-start-160216-IB03090424-remove]//
//#include <Library/PcdLib.h>
//[-end-160216-IB03090424-remove]//
//[-start-160807-IB07400769-add]//
#include <Private/Library/HeciInitLib.h>
//[-end-160807-IB07400769-add]//

#define  PLATFORM_CONFIG_STORE_FILE_NAME  "critical/mrc_trg_data"

#pragma pack(1)
typedef struct {
  EFI_GUID          ConfigDataGuid;
  UINTN             Offset;
}PLATFORM_CONFIG_DATA_MAPPING_TABLE;

typedef struct {
  EFI_GUID          ConfigDataGuid;
  UINT32            ConfigDataSize;
  UINT32            CheckSum;
  UINT32            Reserved[2];
  UINT8             Data[16];
}PLATFORM_CONFIG_DATA_TABLE;
#pragma pack()

#define PLATFORM_CONFIG_DATA_ALIGNMENT  16
#define PLATFORM_CONFIG_DATA_TABLE_SIZE(a)  (((UINTN) ((sizeof(PLATFORM_CONFIG_DATA_TABLE) + a - 16)) + PLATFORM_CONFIG_DATA_ALIGNMENT - 1) & (~(PLATFORM_CONFIG_DATA_ALIGNMENT - 1)))

#endif



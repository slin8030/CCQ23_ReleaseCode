/** @file
  Hsti Results driver definitions

@copyright
  Copyright (c) 2015 - 2016 Intel Corporation. All rights reserved.
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor. This file may be modified by the user, subject to
  the additional terms of the license agreement.
**/

#ifndef _HSTI_RESULT_DXE_H_
#define _HSTI_RESULT_DXE_H_

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/SiPolicyProtocol.h>
#include <HstiFeatureBit.h>
#include <PlatformConfigurationChangeVariable.h>
#include <Protocol/VariableLock.h>
#include <Guid/EventGroup.h>
//#include <MemInfoHob.h>
#include <ScRegs/RegsPcu.h>
#include <ScRegs/RegsPmc.h>
#include <Library/HobLib.h>

#pragma pack(1)
typedef struct {
  UINT32  Version;
  UINT32  Role;
  CHAR16  ImplementationID[256];
  UINT32  SecurityFeaturesSize;
  UINT8   SecurityFeaturesRequired[HSTI_SECURITY_FEATURE_SIZE];
  UINT8   SecurityFeaturesImplemented[HSTI_SECURITY_FEATURE_SIZE];
  UINT8   SecurityFeaturesVerified[HSTI_SECURITY_FEATURE_SIZE];
  CHAR16  End;
} ADAPTER_INFO_PLATFORM_SECURITY_STRUCT;
#pragma pack()

#define HSTI_STORAGE_NAME  L"HSTI_RESULTS"
#define HSTI_STORED_RESULTS_GUID \
  { \
    0x8732b833, 0x5367, 0x422c, 0xa7, 0x7d, 0x99, 0xe5, 0xb5, 0x10, 0x39, 0xa8 \
  }

EFI_GUID  gHstiStoredResultsGuid = HSTI_STORED_RESULTS_GUID;
extern EFI_GUID gPlatformConfigChangeProtocolGuid;

#endif

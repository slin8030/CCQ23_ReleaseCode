/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/**@file
  Multiplatform initialization header file.

  This file includes package header files, library classes.

  Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
   This software and associated documentation (if any) is furnished
   under a license and may only be used or copied in accordance
   with the terms of the license. Except as permitted by such
   license, no part of this software or documentation may be
   reproduced, stored in a retrieval system, or transmitted in any
   form or by any means without the express written consent of
   Intel Corporation.
**/

#ifndef _MULTIPLATFORM_LIB_H_
#define _MULTIPLATFORM_LIB_H_

//////////////////////////////////////////////////////////////////////
#define LEN_64M       0x4000000
//
// Default PCI32 resource size
//
#define RES_MEM32_MIN_LEN   0x38000000

#define RES_IO_BASE   0x0D00
#define RES_IO_LIMIT  0xFFFF
//////////////////////////////////////////////////////////////////////
#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <FrameworkPei.h>

#include "PlatformBaseAddresses.h"
#include "ScAccess.h"
#include "PlatformBootMode.h"
#include "CpuRegs.h"
#include "Platform.h"

#include <Ppi/Stall.h>
#include <ChipsetSetupConfig.h>
#include <Ppi/AtaController.h>
#include <Ppi/FindFv.h>
#include <Ppi/BootInRecoveryMode.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/Capsule.h>
#include <Guid/EfiVpdData.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/BaseLib.h>
#include <IndustryStandard/Pci22.h>
#include <Ppi/Speaker.h>
#include <Guid/FirmwareFileSystem.h>
#include <Guid/MemoryTypeInformation.h>
#include <Ppi/Cache.h>
#include <Ppi/Reset.h>
#include <Ppi/EndOfPeiPhase.h>
#include <Ppi/MemoryDiscovered.h>
#include <Guid/GlobalVariable.h>
#include <Ppi/RecoveryModule.h>
#include <Ppi/DevicerecoveryModule.h>
#include <Guid/Capsule.h>
#include <Guid/RecoveryDevice.h>
#include <Ppi/MasterBootMode.h>
#include <Guid/PlatformInfo.h>

#include <BoardOemIds/BoardOemIds.h>
#include <BoardSsidSvid/BoardSsidSvid.h>


EFI_STATUS
GetPlatformInfoHob (
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  OUT EFI_PLATFORM_INFO_HOB     **PlatformInfoHob
  );

#endif

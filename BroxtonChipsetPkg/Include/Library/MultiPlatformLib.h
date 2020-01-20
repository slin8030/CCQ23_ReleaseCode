/**@file
  Multiplatform initialization header file.

@copyright
Copyright (c) 2010 - 2016 Intel Corporation. All rights reserved.
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

@par Specification Reference:
**/

#ifndef _MULTIPLATFORM_LIB_H_
#define _MULTIPLATFORM_LIB_H_

#define LEN_64M             0x4000000

//
// Default PCI32 resource size
//
#define RES_MEM32_MIN_LEN   0x38000000

#define RES_IO_BASE   0x0D00
#define RES_IO_LIMIT  0xFFFF

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <FrameworkPei.h>
#include <Library/PreSiliconLib.h>

#include "PlatformBaseAddresses.h"
#include "ScAccess.h"
//#include "SetupMode.h"
#include "PlatformBootMode.h"
#include "CpuRegs.h"
#include "Platform.h"
#include "CMOSMap.h"
#include <Ppi/Stall.h>
//#include <Guid/SetupVariable.h>
#include <ChipsetSetupConfig.h>
#include <Ppi/AtaController.h>
#include <Ppi/BootInRecoveryMode.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/Capsule.h>
#include <Guid/EfiVpdData.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/BaseLib.h>
#include <Library/SteppingLib.h>
#include <Library/PeiServicesLib.h>
#include <IndustryStandard/Pci22.h>
#include <Ppi/Speaker.h>
#include <Guid/FirmwareFileSystem.h>
#include <Guid/MemoryTypeInformation.h>
#include <Ppi/Reset.h>
#include <Ppi/EndOfPeiPhase.h>
#include <Ppi/MemoryDiscovered.h>
#include <Guid/GlobalVariable.h>
#include <Ppi/RecoveryModule.h>
#include <Ppi/DeviceRecoveryModule.h>
#include <Guid/Capsule.h>
#include <Guid/RecoveryDevice.h>
#include <Ppi/MasterBootMode.h>
#include <Guid/PlatformInfo_Apl.h>

#if (ENBDT_PF_ENABLE == 1)
#include <Library/EcMiscLib.h>

EFI_STATUS
GetBoardIdFabId (
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  );
#endif

EFI_STATUS
GetPlatformInfoHob (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT EFI_PLATFORM_INFO_HOB     **PlatformInfoHob
  );

EFI_STATUS
MultiPlatformGpioTableInit (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN EFI_PLATFORM_INFO_HOB      *PlatformInfoHob
  );

EFI_STATUS
MultiPlatformGpioProgram (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN EFI_PLATFORM_INFO_HOB      *PlatformInfoHob
  );

/**
  Updates Setup values from PlatformInfoHob and platform policies.

  @param  VOID

  @retval EFI_SUCCESS           The Setup data was updated successfully.
**/
EFI_STATUS
UpdateSetupDataValues (
  CHIPSET_CONFIGURATION     *CachedSetupData
  );
  
#endif

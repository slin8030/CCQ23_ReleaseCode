/** @file
  Header file for Platform eMMC Info Dxe Driver

@copyright
 Copyright (c) 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
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
#ifndef _PLATFORM_EMMC_INFO_DRIVER_H_
#define _PLATFORM_EMMC_INFO_DRIVER_H_

//#include <Guid/SetupVariable.h>
#include <ChipsetSetupConfig.h>
#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#include <Guid/PlatformInfo.h>
#include <Guid/PlatformEmmcHs400Info.h>
#include <Protocol/ScEmmcTuning.h>
#include "MediaDeviceDriver.h" 
#include <Protocol/EmmcCardInfoProtocol.h> 

#define B_EMMC_HS400_12V          BIT7  ///< HS400 Dual Data Rate eMMC @ 200MHz - 1.2V I/O
#define B_EMMC_HS400_18V          BIT6  ///< HS400 Dual Data Rate eMMC @ 200MHz - 1.8V I/O
#define V_EMMC_HS200_BUS_WIDTH_4  1     ///< BUS_WIDTH [183] 4 bit data bus (SDR)
#define V_EMMC_HS200_BUS_WIDTH_8  2     ///< BUS_WIDTH [183] 8 bit data bus (SDR)

//extern EFI_GUID gEfiNormalSetupGuid;
#endif

/** @file

@copyright
Copyright (c) 2012 - 2016 Intel Corporation. All rights reserved
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
#ifndef _DXE_SA_POLICY_UPDATE_H_
#define _DXE_SA_POLICY_UPDATE_H_

#include <PiDxe.h>
#include <CpuRegs.h>
#include <Guid/SaDataHob.h>
#include <Guid/PlatformInfo.h>
#include <ChipsetSetupConfig.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/SaPolicy.h>
#include <Library/MmPciLib.h>
#include <Protocol/PlatformGopPolicy.h>
#include <Library/ConfigBlockLib.h>
//[-start-160701-IB07400750-add]//
#include <Library/UefiLib.h>
//[-end-160701-IB07400750-add]//


#endif

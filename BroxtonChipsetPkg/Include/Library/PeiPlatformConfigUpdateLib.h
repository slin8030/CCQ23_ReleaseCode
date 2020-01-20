/** @file
  Header file for the Platform Configuration Update library.

@copyright
 Copyright (c) 2016 Intel Corporation. All rights reserved
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

#ifndef _PEI_PLATFORM_CONFIG_UPDATE_LIB_H_
#define _PEI_PLATFORM_CONFIG_UPDATE_LIB_H_

#include <PiPei.h>
//[-start-160624-IB03090428-modify]//
#include <ChipsetSetupConfig.h>
//[-end-160624-IB03090428-modify]//

/**
  Updates Setup values from PlatformInfoHob and platform policies.

  @param  PreDefaultSetupData   A pointer to the setup data prior to being
                                placed in the default data HOB.

  @retval EFI_SUCCESS           The Setup data was updated successfully.
**/
EFI_STATUS
UpdateSetupDataValues (
//[-start-160624-IB03090428-modify]//
  CHIPSET_CONFIGURATION     *CachedSetupData
//[-end-160624-IB03090428-modify]//
  );
  
#endif


/** @file

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/** @file

@copyright
  Copyright (c) 1999 - 2015 Intel Corporation. All rights reserved.
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
#include "PlatformDxe.h"
#include <Protocol/SaPolicy.h>
#include <Protocol/GlobalNvsArea.h>
#include <Library/PreSiliconLib.h>
#include <Library/DxeSaPolicyLib.h>
#include <Library/DxeSaPolicyUpdateLib.h>
#include <Library/ConfigBlockLib.h>
//[-start-151220-IB1127138-add]//
#include <Library/DxeOemSvcChipsetLib.h>
//[-end-151220-IB1127138-add]//

VOID
InitPlatformSaPolicy (
  IN CHIPSET_CONFIGURATION* SystemConfiguration
  )
{
    EFI_STATUS               Status;
    SA_POLICY_PROTOCOL       *SaPolicy;

    //
    // Call CreateSaDxeConfigBlocks to initialize SA DXE policy structure
    // and get all Intel default policy settings.
    //
    Status = CreateSaDxeConfigBlocks(&SaPolicy);
    DEBUG((DEBUG_INFO, "SaPolicy->TableHeader.NumberOfBlocks = 0x%x\n ", SaPolicy->TableHeader.NumberOfBlocks));
    ASSERT_EFI_ERROR(Status);

    UpdateDxeSaPolicy(SaPolicy, SystemConfiguration);
//[-start-151220-IB1127138-add]//
  //
  // OemServices
  //
  Status = OemSvcUpdateDxePlatformSaPolicy (SaPolicy);
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "DexOemSvcChipsetLib OemSvcUpdateDxePlatformSaPolicy, Status : %r\n", Status));
//[-end-151220-IB1127138-add]//
    Status = SaInstallPolicyProtocol(SaPolicy);
    ASSERT_EFI_ERROR(Status);
}

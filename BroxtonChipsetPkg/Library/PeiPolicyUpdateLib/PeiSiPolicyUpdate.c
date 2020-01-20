/** @file
  This file is SampleCode of the library for Intel Silicon PEI
  Platform Policy initialzation.

@copyright
  Copyright (c) 2014 - 2016 Intel Corporation. All rights reserved
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

#include "PeiSiPolicyUpdate.h"
//[-start-160803-IB07220122-add]//
#include <Library/PeiServicesLib.h>
#include <Ppi/ReadOnlyVariable2.h>
//[-end-160803-IB07220122-add]//
/**
  This function performs Silicon PEI Policy initialzation.

  @param[in] SiPolicy  The Silicon Policy PPI instance

  @retval EFI_SUCCESS  The function completed successfully
**/
EFI_STATUS
EFIAPI
UpdatePeiSiPolicy (
  IN OUT SI_POLICY_PPI *SiPolicy
  )
{
//[-start-160216-IB03090424-modify]//
  UINTN                 VariableSize;
  CHIPSET_CONFIGURATION SystemConfiguration;
//[-start-160803-IB07220122-add]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariableServices;
//[-end-160803-IB07220122-add]//
  EFI_STATUS            Status;

  //
  // Update Silicon Policy Config
  //
  //
  // Retrieve Setup variable
  //
//[-start-160803-IB07220122-add]//
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
//[-end-160803-IB07220122-add]//

  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData(&SystemConfiguration, &VariableSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );
//[-end-160803-IB07220122-modify]//

  //
  // OS Selection 0: WOS 1:AOS 2:Win7
  //
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "UpdatePeiSiPolicy (), Setup Variable is ready!\n"));
    SiPolicy->OsSelection = SystemConfiguration.OsSelection;
	
    // Enable PWMEnabled for other OS except Windows OS
    if (SiPolicy->OsSelection != SiWindows)
	    SiPolicy->PWMEnabled = 1;

//[-start-161008-IB07400794-remove]//
    //
    // Default is Windows
    //
//#if BXTI_PF_ENABLE
//  } else {
//    //
//    // Setting default OsSelection to IntelLinux
//    //
//    SiPolicy->OsSelection = 3;
//#endif
//[-end-161008-IB07400794-remove]//
  }

  return EFI_SUCCESS;
//[-end-160216-IB03090424-modify]//
}


/** @file
  This file is SampleCode for Intel SA PEI Policy initialization.

@copyright
 Copyright (c) 1999 - 2014 Intel Corporation. All rights reserved
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

#include "PeiSaPolicyInit.h"
//[-start-151217-IB11270137-add]//
#include <Library/PeiOemSvcChipsetLib.h>
//[-end-151217-IB11270137-add]//
/**
  This PEIM performs SA PEI Policy initialzation.

  @param[in]  FirmwareConfiguration  It uses to skip specific policy init that depends
                                     on the 'FirmwareConfiguration' varaible.

  @retval     EFI_SUCCESS            The PPI is installed and initialized.
  @retval     EFI_OUT_OF_RESOURCES   Do not have enough resources to initialize the PPI.
  @retval     EFI ERRORS             The PPI is not successfully installed.
**/
EFI_STATUS
PeiSaPolicyInit (
  IN UINT8                     FirmwareConfiguration
  )
{

  EFI_STATUS               Status;
  SI_SA_POLICY_PPI         *SiSaPolicyPpi;

  //
  // Call SaCreatePolicyDefaults to initialize platform policy structure
  // and get all Intel default policy settings.
  //
  Status = CreateConfigBlocks (&SiSaPolicyPpi);
  DEBUG ((DEBUG_INFO, "SiSaPolicyPpi->TableHeader.NumberOfBlocks = 0x%x\n ", SiSaPolicyPpi->TableHeader.NumberOfBlocks));
  ASSERT_EFI_ERROR (Status);

  UpdatePeiSaPolicy (SiSaPolicyPpi);
//[-start-151216-IB11270137-add]//
  //
  // OemServices
  //
  Status = OemSvcUpdateSaPlatformPolicy (SiSaPolicyPpi);
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "PeiOemSvcChipsetLib OemSvcUpdateSaPlatformPolicy, Status : %r\n", Status));  
  ASSERT ( ( Status == EFI_SUCCESS ) || ( Status == EFI_UNSUPPORTED ) || ( Status == EFI_MEDIA_CHANGED ) );
  if (Status == EFI_SUCCESS) {
    return Status;
  }
//[-end-151216-IB11270137-add]//
  //
  // Install SiSaPolicyPpi.
  // While installed, RC assumes the Policy is ready and finalized. So please
  // update and override any setting before calling this function.
  //
  Status = SiSaInstallPolicyPpi (SiSaPolicyPpi);
  ASSERT_EFI_ERROR (Status);
  
  return Status;
}

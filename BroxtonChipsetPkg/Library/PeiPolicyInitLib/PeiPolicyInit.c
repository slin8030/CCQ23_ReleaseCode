/** @file
  This file is SampleCode for Intel PEI Platform Policy initialzation.

@copyright
 Copyright (c) 2013 - 2015 Intel Corporation. All rights reserved
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

#include "PeiPolicyInit.h"

/**
  Initialize Intel PEI Platform Policy

  @param[in] PeiServices            General purpose services available to every PEIM.
  @param[in] FirmwareConfiguration  It uses to skip specific policy init that depends
                                    on the 'FirmwareConfiguration' varaible.
**/
//[-start-151228-IB03090424-modify]//
VOID
EFIAPI
PeiPolicyInit (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN CHIPSET_CONFIGURATION     *SystemConfiguration
  )
{
  EFI_STATUS                   Status;

  DEBUG ((DEBUG_INFO, "PeiPolicyInit - Start\n"));
  //
  // CPU PEI Policy Initialization
  //
  Status = PeiCpuPolicyInit (SystemConfiguration);
  DEBUG ((DEBUG_INFO, "CPU PEI Policy Initialization Done in Post-Memory\n"));
  ASSERT_EFI_ERROR (Status);

  //
  // SC PEI Policy Initialization
  //
  //Status = PeiScPolicyInit ();
  //DEBUG ((DEBUG_INFO, "SC PEI Policy Initialization Done in Post-Memory\n"));
  //ASSERT_EFI_ERROR (Status);
  DEBUG ((DEBUG_INFO, "PeiPolicyInit - End\n"));
}
//[-end-151228-IB03090424-modify]//

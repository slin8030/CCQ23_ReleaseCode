/** @file
  This file is SampleCode for Intel PEI Platform Policy initialization.

@copyright
 Copyright (c) 2013 - 2016 Intel Corporation. All rights reserved
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

//[-start-151228-IB03090424-modify]//
/**
  Initialize Intel PEI Platform Policy
**/
EFI_STATUS
EFIAPI
PeiPolicyInitPreMem (
  VOID
  )
{
  EFI_STATUS    Status;
  
  //
  // CPU PEI Policy Initialization
  //
  Status = PeiCpuPolicyInitPreMem ();
  DEBUG ((DEBUG_INFO, "CPU PEI Policy Initialization Done in Pre-Memory\n"));
  ASSERT_EFI_ERROR (Status);

  return Status;
}
//[-end-151228-IB03090424-modify]//

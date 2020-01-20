/** @file
  Header file for the PeiPolicyInit Library.

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
#ifndef _POLICY_INIT_PEI_LIB_H_
#define _POLICY_INIT_PEI_LIB_H_

//[-start-151228-IB03090424-modify]//
/**
  Initialize Pre-Mem Intel PEI Platform Policy
**/
EFI_STATUS
EFIAPI
PeiPolicyInitPreMem (
  );

/**
  Initialize Intel PEI Platform Policy

  @param[in] PeiServices            General purpose services available to every PEIM.
  @param[in] SystemConfiguration    The pointer to get System Setup
**/
VOID
EFIAPI
PeiPolicyInit (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN CHIPSET_CONFIGURATION     *SystemConfiguration
  );
//[-end-151228-IB03090424-modify]//
#endif

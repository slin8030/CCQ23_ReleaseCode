/** @file
  Header file for the PeiCpuPolicyInit.

@copyright
  Copyright (c) 2009 - 2015 Intel Corporation. All rights reserved
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

#ifndef _PEI_CPU_POLICY_INIT_H_
#define _PEI_CPU_POLICY_INIT_H_

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/CpuPolicyLib.h>
#include <Library/CpuPlatformLib.h>
//[-start-151228-IB03090424-modify]//
#include <ChipsetSetupConfig.h>
#include <Library/PeiCpuPolicyUpdateLib.h>
//[-end-151228-IB03090424-modify]//

//[-start-151228-IB03090424-modify]//
//
// Function prototypes
//
/**
  This function performs CPU PEI Policy initialization in Pre-Memory.

  @retval     EFI_SUCCESS           The PPI is installed and initialized.
  @retval     EFI ERRORS            The PPI is not successfully installed.
  @retval     EFI_OUT_OF_RESOURCES  No enough resoruces (such as out of memory).
**/
EFI_STATUS
EFIAPI
PeiCpuPolicyInitPreMem (
  );

/**
  This function performs CPU PEI Policy initialization in Post-Memory.

  @param[in] SystemConfiguration		The pointer to get System Setup

  @retval     EFI_SUCCESS           The PPI is installed and initialized.
  @retval     EFI ERRORS            The PPI is not successfully installed.
  @retval     EFI_OUT_OF_RESOURCES  No enough resoruces (such as out of memory).
**/
EFI_STATUS
EFIAPI
PeiCpuPolicyInit (
  IN CHIPSET_CONFIGURATION        *SystemConfiguration
  );
//[-end-151228-IB03090424-modify]//
#endif

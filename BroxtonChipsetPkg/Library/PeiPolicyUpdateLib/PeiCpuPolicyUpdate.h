/** @file
  Header file for PEI CpuPolicyUpdate.

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

#ifndef _PEI_CPU_POLICY_UPDATE_H_
#define _PEI_CPU_POLICY_UPDATE_H_

#include <PiPei.h>

//[-start-151228-IB03090424-modify]//
#include <Guid/PlatformInfo.h>
#include <ChipsetSetupConfig.h>
#include <CpuAccess.h>
#include <Ppi/CpuPolicy.h>

#include <Ppi/ReadOnlyVariable2.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/CpuPlatformLib.h>
#include <Library/MmPciLib.h>


/**
  This function performs CPU PEI Policy initialization.

  @param[in] SiCpuPolicyPpi        The Cpu Policy PPI instance

  @retval EFI_SUCCESS              The PPI is installed and initialized.
  @retval EFI ERRORS               The PPI is not successfully installed.
  @retval EFI_OUT_OF_RESOURCES     Do not have enough resources to initialize the driver
**/
EFI_STATUS
EFIAPI
UpdatePeiCpuPolicy (
  IN OUT  SI_CPU_POLICY_PPI      *SiCpuPolicyPpi,
  IN CHIPSET_CONFIGURATION       *SystemConfiguration
  );
//[-end-151228-IB03090424-modify]//
#endif

/** @file
  Prototype of the PeiSaPolicy library.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef _PEI_SA_POLICY_LIB_H_
#define _PEI_SA_POLICY_LIB_H_

#include <Ppi/SaPolicy.h>
#include <Library/ConfigBlockLib.h>
#include <Library/BaseMemoryLib.h>
#include <SaRegs.h>


/**
Creates the Pre Mem Config Blocks for SA Policy.
It allocates and zero out buffer, and fills in the Intel default settings.

@param[in, out] SiSaPolicyPpi         - The pointer to get SI/SA Policy PPI instance

@retval EFI_SUCCESS                   - The policy default is initialized.
@retval EFI_OUT_OF_RESOURCES          - Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
SaCreatePreMemConfigBlocks(
IN OUT  SI_SA_POLICY_PPI          **SiSaPolicyPpi
);

/**
  Creates the Config Blocks for SA Policy.
  It allocates and zero out buffer, and fills in the Intel default settings.

  @param[in, out] SiSaPolicyPpi         - The pointer to get SI/SA Policy PPI instance

  @retval EFI_SUCCESS                   - The policy default is initialized.
  @retval EFI_OUT_OF_RESOURCES          - Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
CreateConfigBlocks (
  IN OUT  SI_SA_POLICY_PPI          **SiSaPolicyPpi
  );

/**
  Install PPI SiSaPolicyPpi.
  While installed, RC assumes the Policy is ready and finalized. So please update and override
  any setting before calling this function.

  @param[in] SiSaPolicyPpi              - pointer of policy structure

  @retval EFI_SUCCESS                   - The policy is installed.
  @retval EFI_OUT_OF_RESOURCES          - Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
SiSaInstallPolicyPpi (
  IN  SI_SA_POLICY_PPI              *SiSaPolicyPpi
  );

/**
  SaInstallPreMemPolicyPpi installs SaPolicyPpi.
  While installed, RC assumes the Policy is ready and finalized. So please update and override
  any setting before calling this function.

  @param[in] SaPreMemPolicyPpi         The pointer to SA PREMEM Policy PPI instance

  @retval EFI_SUCCESS                   The policy is installed.
  @retval EFI_OUT_OF_RESOURCES          Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
SaInstallPreMemPolicyPpi (
  IN  SI_SA_POLICY_PPI             *SaPolicyPpi
  );

#endif // _PEI_SA_POLICY_LIBRARY_H_

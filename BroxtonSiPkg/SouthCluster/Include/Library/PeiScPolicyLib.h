/** @file
  Prototype of the PeiScPolicy library.

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

#ifndef _PEI_SC_POLICY_LIBRARY_H_
#define _PEI_SC_POLICY_LIBRARY_H_

#include <Ppi/ScPolicy.h>
#include <Ppi/ScPolicyPreMem.h>

/**
  Print whole SC_PREMEM_POLICY_PPI and serial out.

  @param[in] ScPreMemPolicyPpi The RC Policy PPI instance
**/
VOID
EFIAPI
ScPreMemPrintPolicyPpi (
  IN  SC_PREMEM_POLICY_PPI          *ScPreMemPolicyPpi
  );

/**
  Print whole SC_POLICY_PPI and serial out.

  @param[in] ScPolicy    The SC Policy Ppi instance
**/
VOID
EFIAPI
ScPrintPolicyPpi (
  IN  SC_POLICY_PPI           *ScPolicy
  );

/**
  CreatePreMemConfigBlocks generates the config blocks of SC Policy.
  It allocates and zero out buffer, and fills in the Intel default settings.

  @param[out] ScPreMemPolicyPpi        The pointer to get SC PREMEM Policy PPI instance

  @retval EFI_SUCCESS                   The policy default is initialized.
  @retval EFI_OUT_OF_RESOURCES          Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
ScCreatePreMemConfigBlocks (
  OUT  SC_PREMEM_POLICY_PPI            **ScPreMemPolicyPpi
  );

/**
  ScInstallPreMemPolicyPpi installs ScPolicyPpi.
  While installed, RC assumes the Policy is ready and finalized. So please update and override
  any setting before calling this function.

  @param[in] ScPreMemPolicyPpi         The pointer to SC PREMEM Policy PPI instance

  @retval EFI_SUCCESS                   The policy is installed.
  @retval EFI_OUT_OF_RESOURCES          Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
ScInstallPreMemPolicyPpi (
  IN  SC_PREMEM_POLICY_PPI             *ScPreMemPolicyPpi
  );

/**
  Get SC config block table total size.

  @retval Size of SC config block table
**/
UINT32
EFIAPI
ScGetConfigBlockTotalSize (
  VOID
  );

/**
  ScCreateConfigBlocks generates the config blocks of SC Policy.
  It allocates and zero out buffer, and fills in the Intel default settings.

  @param[out] ScPolicyPpi               The pointer to get SC Policy PPI instance

  @retval EFI_SUCCESS                   The policy default is initialized.
  @retval EFI_OUT_OF_RESOURCES          Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
ScCreateConfigBlocks (
  OUT  SC_POLICY_PPI            **ScPolicyPpi
  );

/**
  ScInstallPolicyPpi installs ScPolicyPpi.
  While installed, RC assumes the Policy is ready and finalized. So please update and override
  any setting before calling this function.

  @param[in] ScPolicyPpi                The pointer to SC Policy PPI instance

  @retval EFI_SUCCESS                   The policy is installed.
  @retval EFI_OUT_OF_RESOURCES          Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
ScInstallPolicyPpi (
  IN  SC_POLICY_PPI             *ScPolicyPpi
  );

#endif // _PEI_SC_POLICY_LIBRARY_H_

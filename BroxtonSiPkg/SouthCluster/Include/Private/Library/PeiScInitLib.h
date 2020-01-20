/** @file
  Header file for SC Init Lib Pei Phase

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
#ifndef _SC_INIT_LIB_H_
#define _SC_INIT_LIB_H_

#include <Ppi/SiPolicyPpi.h>
#include <Ppi/ScPolicy.h>
#include <Ppi/ScPolicyPreMem.h>

/**
  This function performs PCIe power sequence to bring up the PCIe related device and initiate link training.

  @param[in]  None

  @retval  None
**/
VOID
ScConfigurePciePowerSequence (
  IN  SC_PREMEM_POLICY_PPI   *ScPreMemPolicyPpi
  );

/**
  SC Init Pre Policy Entry Point
  Performing SC init in before Policy init

  @param[in]  None

  @retval  None
**/
VOID
EFIAPI
ScInitPrePolicy (
  VOID
  );


/**
  This function performs basic initialization for SC in PEI phase after Policy produced.
  If any of the base address arguments is zero, this function will disable the corresponding
  decoding, otherwise this function will enable the decoding.
  This function locks down the AcpiBase.

  @param[in] SiPolicyPpi  The Silicon Policy PPI instance
  @param[in] ScPolicyPpi  The SC Policy PPI instance

  @retval  None
**/
VOID
EFIAPI
ScOnPolicyInstalled (
  IN  SI_POLICY_PPI   *SiPolicyPpi,
  IN  SC_POLICY_PPI   *ScPolicyPpi
  );

/**
  Sc init after memory PEI module

  @param[in] SiPolicyPpi     The Silicon Policy PPI instance
  @param[in] ScPolicyPpi     The SC Policy PPI instance

  @retval None
**/
VOID
EFIAPI
ScInit (
  IN  SI_POLICY_PPI  *SiPolicyPpi,
  IN  SC_POLICY_PPI  *ScPolicyPpi
  );
/**
  Sc End of PEI callback function. This is the last change before entering DXE and OS when S3 resume.

  @param[in] SiPolicy     None

  @retval None
**/
VOID
ScOnEndOfPei (
  VOID
  );

#endif // _SC_INIT_LIB_H_

/** @file
  Prototype of the PeiSataInitLib library.

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

@par Specification
**/
#ifndef _PEI_PCIEXPRESS_INIT_LIBRARY_H_
#define _PEI_PCIEXPRESS_LIBRARY_H_

#include <Ppi/ScPolicy.h>

/**
  Perform Initialization of the Downstream Root Ports.

  @param[in] ScPolicyPpi             The SC Policy protocol
  @param[in] TempPciBusMin           The temporary minimum Bus number for root port initialization
  @param[in] TempPciBusMax           The temporary maximum Bus number for root port initialization
  @param[in] OriginalFuncDisableReg  The PMC FUNC_DIS_0 register from PMC
  @param[in] OriginalFuncDisable1Reg The PMC FUNC_DIS_1 register from PMC
  @param[in, out] FuncDisableReg     The PMC FUNC_DIS_0 register to be updated
  @param[in, out] FuncDisable1Reg    The PMC FUNC_DIS_1 register to be updated

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ScInitRootPorts (
  IN SC_POLICY_PPI        *ScPolicyPpi,
  IN UINT8                TempPciBusMin,
  IN UINT8                TempPciBusMax,
  IN UINT32               *OriginalFuncDisableReg,
  IN UINT32               *OriginalFuncDisable1Reg,
  IN OUT UINT32           *FuncDisableReg,
  IN OUT UINT32           *FuncDisable1Reg
  );

#endif ///< _PEI_PCIEXPRESS_LIBRARY_H_

/** @file
  This file is PeiScPolicyLib library for printing PREMEM Policy settings.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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
#include "PeiScPolicyLibrary.h"

VOID
PrintPciePreMemConfig (
  IN CONST SC_PCIE_PREMEM_CONFIG   *PciePreMemConfig
  )
{
  UINT8 i;

  DEBUG ((DEBUG_INFO, "--- PCIe Config ---\n"));
  for (i = 0; i < GetScMaxPciePortNum (); i++) {
    DEBUG ((DEBUG_INFO, " RootPort[%d] PERST = %x\n", i, PciePreMemConfig->RootPort[i].Perst));
  }
  DEBUG ((DEBUG_INFO, " StartTimerTickerOfPerstAssert = %x\n", i, PciePreMemConfig->StartTimerTickerOfPfetAssert));
}

/**
  Print SC_LPC_PREMEM_CONFIG and serial out.

  @param[in] LpcConfig  Pointer to a SC_LPC_CONFIG that provides the platform setting
**/
VOID
PrintLpcPreMemConfig (
  IN CONST SC_LPC_PREMEM_CONFIG  *LpcConfig
  )
{
  DEBUG ((DEBUG_INFO, "--- LPC Config ---\n"));
  DEBUG ((DEBUG_INFO, " EnhancePort8xhDecoding = %x\n", LpcConfig->EnhancePort8xhDecoding));
}

/**
  Print whole SC_PREMEM_POLICY_PPI and serial out.

  @param[in] ScPreMemPolicyPpi The RC Policy PPI instance
**/
VOID
EFIAPI
ScPreMemPrintPolicyPpi (
  IN  SC_PREMEM_POLICY_PPI *ScPreMemPolicyPpi
  )
{
  SC_PCIE_PREMEM_CONFIG  *PciePreMemConfig;
  SC_LPC_PREMEM_CONFIG   *LpcPreMemConfig;
  EFI_STATUS             Status;

  Status = GetConfigBlock ((VOID *) ScPreMemPolicyPpi, &gPcieRpPreMemConfigGuid, (VOID *) &PciePreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPreMemPolicyPpi, &gLpcPreMemConfigGuid, (VOID *) &LpcPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "--- SC Print PreMem Policy Start ---\n"));
  PrintPciePreMemConfig (PciePreMemConfig);
  PrintLpcPreMemConfig (LpcPreMemConfig);
  DEBUG ((DEBUG_INFO, "--- SC Print PreMem Policy End ---\n"));
}


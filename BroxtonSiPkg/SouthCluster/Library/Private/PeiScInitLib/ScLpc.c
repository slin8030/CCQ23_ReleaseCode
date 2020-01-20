/** @file
  Initializes SC LPC at PEI phase

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

#include "ScInitPei.h"

/**
  Configure LPC device on early PEI.
**/
VOID
ConfigureLpcOnEarlyPei (
  VOID
  )
{
  DEBUG ((DEBUG_INFO, "ConfigureLpcOnEarlyPei() \n"));

  //
  // Enhance port 8xh LPC decoding.
  // The can be disable by policy EnhancePort8xhDecoding.
  //
  PchLpcGenIoRangeSet (0x80, 0x10);
}

/**
  Configure LPC device on Policy callback.

  @param[in] ScPreMemPolicyPpi  The SC PreMEM Policy PPI instance
**/
VOID
ConfigureLpcOnPolicy (
  IN  SC_PREMEM_POLICY_PPI  *ScPreMemPolicyPpi
  )
{
  PCH_LPC_GEN_IO_RANGE_LIST LpcGenIoRangeList;
  SC_LPC_PREMEM_CONFIG      *LpcPreMemConfig;
  EFI_STATUS                Status;
  UINTN                     LpcBase;
  UINTN                     Index;

  DEBUG ((DEBUG_INFO, "ConfigureLpcOnPolicy()\n"));
  Status = GetConfigBlock ((VOID *) ScPreMemPolicyPpi, &gLpcPreMemConfigGuid, (VOID *) &LpcPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  //
  // If EnhancePort8xhDecoding is disabled, clear LPC general IO range register.
  //
  if (LpcPreMemConfig->EnhancePort8xhDecoding == 0) {
    DEBUG ((DEBUG_INFO, "Disable EnhancePort8xhDecoding\n"));
    SetMem (&LpcGenIoRangeList, sizeof (PCH_LPC_GEN_IO_RANGE_LIST), 0);
    PchLpcGenIoRangeGet (&LpcGenIoRangeList);
    for (Index = 0; Index < PCH_LPC_GEN_IO_RANGE_MAX; Index++) {
      if ((LpcGenIoRangeList.Range[Index].BaseAddr == 0x80) &&
          (LpcGenIoRangeList.Range[Index].Length   == 0x10))
      {
        //
        // Clear the LPC general IO range register.
        //
        LpcBase = MmPciBase (
                    DEFAULT_PCI_BUS_NUMBER_SC,
                    PCI_DEVICE_NUMBER_PCH_LPC,
                    PCI_FUNCTION_NUMBER_PCH_LPC
                    );
        MmioWrite32 (
          LpcBase + R_PCH_LPC_GEN1_DEC + Index * 4,
          0
          );
        break;
      }
    }
  }
}

/**
  The function performs LPC specific programming.

  @param[in]  None

  @retval EFI_SUCCESS             Succeeds.
**/
EFI_STATUS
ScLpcInit (
  VOID
  )
{
  DEBUG ((DEBUG_INFO, "ScLpcInit()\n"));
  //
  // BIOS need to set LPC PCR 0x341C[11, 8, 3:0] to all 1's and [23:12, 10:9, 7:4] = 0's
  //
  PchPcrAndThenOr32 (
    PID_LPC,
    R_PCH_PCR_LPC_PRC,
    0,
    (UINT32) (BIT13 | BIT11 | BIT8 | BIT3 | BIT2 | BIT1 | BIT0)
    );

  return EFI_SUCCESS;
}

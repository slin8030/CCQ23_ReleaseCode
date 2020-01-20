/** @file
  Initializes P2SB

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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
  Initialize P2SB according to P2SB policy of the SC
  Platform Policy PPI

  @param[in] P2sbBase             P2SB MMIO address

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_DEVICE_ERROR        Device error, aborts abnormally.
**/
EFI_STATUS
ScP2sbInit (
  IN UINT32 P2sbBase
)
{
  UINT32 Data32;
  UINT32 P2sbMmioBar;
  UINT32 P2sbPciMmBase;

  Data32        = 0;
  P2sbMmioBar   = 0;
  P2sbPciMmBase = 0;

  P2sbPciMmBase = MmPciBase (
                    DEFAULT_PCI_BUS_NUMBER_SC,
                    PCI_DEVICE_NUMBER_P2SB,
                    PCI_FUNCTION_NUMBER_P2SB
                    );

  DEBUG((DEBUG_INFO, "ScP2sbInit() - Start\n"));
  //
  // Read back P2SB MMIO BAR base address
  //
  P2sbMmioBar = MmioRead32 (P2sbPciMmBase + R_PCH_P2SB_SBREG_BAR);
  P2sbMmioBar &= B_PCH_P2SB_SBREG_RBA;  // Mask off the last byte read from BAR
  //
  // Initialize P2SB BAR if necessary
  //
  Data32 = MmioRead32 (P2sbPciMmBase + PCI_COMMAND_OFFSET);
  if ((P2sbMmioBar == 0) || (P2sbMmioBar == B_PCH_P2SB_SBREG_RBA) || \
    ((Data32 & EFI_PCI_COMMAND_MEMORY_SPACE) != EFI_PCI_COMMAND_MEMORY_SPACE)) {
    //
    // Disable P2SB BME and MSE
    //
    MmioAnd32 ((UINTN)(P2sbPciMmBase + PCI_COMMAND_OFFSET), (UINT32)~(EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER));
    //
    // Set P2SB BAR
    //
    MmioWrite32 ((UINTN)(P2sbPciMmBase + R_PCH_P2SB_SBREG_BAR), (UINT32)(P2sbBase & B_PCH_P2SB_SBREG_RBA));
    //
    // Set P2SB BME and MSE
    //
    MmioOr32 ((UINTN)(P2sbPciMmBase + PCI_COMMAND_OFFSET), (UINT32)(EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER));
    //
    // Other devices depend on SB with P2SB using MMIO as part of initialization
    //
    ASSERT ((MmioRead32 (P2sbPciMmBase + R_PCH_P2SB_SBREG_BAR) & B_PCH_P2SB_SBREG_RBA) == P2sbBase);
  }

  DEBUG((DEBUG_INFO, "ScP2sbInit() - End\n"));

  return EFI_SUCCESS;
}



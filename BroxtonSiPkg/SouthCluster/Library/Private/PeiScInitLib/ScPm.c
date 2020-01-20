/** @file
  Initializes SC power management controller.

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
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SideBandLib.h>
#include <ScAccess.h>
#include <Library/ScPlatformLib.h>
#include <Library/MmPciLib.h>
#include <Ppi/ScPolicy.h>
#include "ScInitPei.h"

/**
  Configure SPI/eSPI Power Management
**/
VOID ConfigureSpiPm (
  VOID
  )
{
  UINT32     DataAnd32;
  UINT32     DataOr32;
  BXT_SERIES BxtSeries;

  DEBUG ((DEBUG_INFO, "ConfigureSpiPm() Start\n"));
  //
  //  Set SPI PCR 0xC004
  //    [10:8] 3'b111
  //     [5:0] 6'b111011 for all Broxton series
  //     [5:0] 6'b111111 for other platform
  // Set SPI PCR 0xC008[5] = 1'b1
  BxtSeries = GetBxtSeries ();
  if ((BxtSeries == Bxt1) || (BxtSeries == BxtP)) {
    DataOr32  = (UINT32) (BIT10 | BIT9 | BIT8 | BIT5 | BIT4 | BIT3 | BIT1 | BIT0);
    DataAnd32 = (UINT32) ~(BIT27| BIT23| BIT19 | BIT2);
  } else {
    DataOr32  = (UINT32) (BIT10 | BIT9 | BIT8 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
    DataAnd32 = ~0u;
  }
  SideBandAndThenOr32(SB_SPI_PORT, R_SPI_SB_CLK_PWR_GATE_CNTRL, DataAnd32, DataOr32);
  SideBandAndThenOr32(SB_SPI_PORT, R_SPI_SB_PWRCNTRL_EN, ~0u, BIT5);

  DEBUG ((DEBUG_INFO, "ConfigureSpiPm() End\n"));
}

/**
  Configure LPC power management.

  @param[in] ScPolicy                  The SC Policy instance
**/
VOID
ConfigureLpcPm (
  IN  SC_POLICY_PPI  *ScPolicy
  )
{
  UINTN        PciLpcRegBase;
  EFI_STATUS   Status;
  SC_PM_CONFIG *PmConfig;

  DEBUG ((DEBUG_INFO, "ConfigureLpcPm () Start\n"));
  PciLpcRegBase   = MmPciBase (
                      DEFAULT_PCI_BUS_NUMBER_SC,
                      PCI_DEVICE_NUMBER_PCH_LPC,
                      PCI_FUNCTION_NUMBER_PCH_LPC
                      );

  ///
  /// System BIOS is also required to set following bit.
  /// PCI CLKRUN# Enable" bit (LPC PCI offset E0h[0]) = 1b
  ///
  Status = GetConfigBlock ((VOID *) ScPolicy, &gPmConfigGuid, (VOID *) &PmConfig);
  ASSERT_EFI_ERROR (Status);
  if (PmConfig->PciClockRun == FALSE) {
    MmioAnd16 ((UINTN) (PciLpcRegBase + R_PCH_LPC_PCC), (UINT16) (~B_PCH_LPC_PCC_CLKRUN_EN));
  } else {
    MmioOr16 ((UINTN) (PciLpcRegBase + R_PCH_LPC_PCC), (UINT16) (B_PCH_LPC_PCC_CLKRUN_EN));
  }
  DEBUG ((DEBUG_INFO, "ConfigureLpcPm () End\n"));
}

/**
  Perform power management initialization

  @param[in] SchPolicy                  The SC Policy PPI instance

  @retval EFI_SUCCESS                   Succeeds.
**/
EFI_STATUS
ScPmInit (
  IN  SC_POLICY_PPI                    *ScPolicy
  )
{
  ConfigureSpiPm();

  ConfigureLpcPm (ScPolicy);

  return EFI_SUCCESS;
}

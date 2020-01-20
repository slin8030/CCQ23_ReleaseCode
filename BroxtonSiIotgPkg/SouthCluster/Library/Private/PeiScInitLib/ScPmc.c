/** @file
  Initializes SC PMC Devices

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2018 Intel Corporation.

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
#include "ScInitPei.h"

/**
  Configure the PWM Device

  @retval EFI_STATUS
**/
EFI_STATUS
EFIAPI
ConfigurePwm (
  IN SI_POLICY_PPI                 *SiPolicy
  )
{
  UINTN                 PwmPciMmBase;

  ///
  /// Initialize Variables
  ///
  PwmPciMmBase = 0;

  DEBUG((DEBUG_INFO, "ConfigurePwm() Start\n"));

  PwmPciMmBase = MmPciBase(
                   DEFAULT_PCI_BUS_NUMBER_SC,
                   PCI_DEVICE_NUMBER_PMC_PWM,
                   PCI_FUNCTION_NUMBER_PMC_PWM
                   );

  if (MmioRead32 (PwmPciMmBase) == 0xFFFFFFFF) {
    DEBUG((DEBUG_INFO, "PWM Device NOT Present.\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  DEBUG((DEBUG_INFO, "PWM Device Present. PCI Device Address: 0x%x..\n", PwmPciMmBase));

    //
    // Program PWM Device PG Configuration
    //
    MmioOr32 (PwmPciMmBase + R_PMC_PWM_D0I3MAXDEVPG, BIT18 | BIT17);

  ///
  /// Place PWM in the D3 State (for S0ix)
  /// Set PWM PMECTRLSTATUS, bits 1:0 (POWERSTATE) to 2'b11
  ///
  MmioOr32 (PwmPciMmBase + R_PMC_PWM_PMECTRLSTATUS, B_PMC_PWM_PMECTRLSTATUS_POWERSTATE);

  MmioRead32 (PwmPciMmBase + R_PMC_PWM_PMECTRLSTATUS);

  //
  //  Hide PCI config header for PWM when PWMEnabled disabled and for Windows OS.
  //

  if (!SiPolicy->PWMEnabled || SiPolicy->OsSelection == SiWindows) {
    ///
    /// Disable device's PciCfg in PSF
    ///
    SideBandAndThenOr32 (
      SB_PORTID_PSF3,
      R_PCH_PCR_PSF3_T0_SHDW_PMC_PWM_REG_BASE + R_PCH_PCR_PSFX_T0_SHDW_CFG_DIS,
      0xFFFFFFFF,
      B_PCH_PCR_PSFX_T0_SHDW_CFG_DIS_CFGDIS
      );
  }
  DEBUG((DEBUG_INFO, "ConfigurePwm() End\n"));

  return EFI_SUCCESS;
}


/**
  Configure Pmc device

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigurePmc (
  IN SI_POLICY_PPI                 *SiPolicy
  )
{
#ifdef SC_PM_ENABLE
  UINTN       PmcPciMmBase = 0;
#endif
  DEBUG ((DEBUG_INFO, "ConfigurePmc() Start\n"));

#ifdef SC_PM_ENABLE
  DEBUG ((DEBUG_INFO, "Start SC_PM_ENABLE\n"));
  PmcPciMmBase = MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC, 0xA0);
  MmioOr32(PmcPciMmBase, BIT18| BIT17);

  SideBandAndThenOr32(
    SB_PMC_IOSF2OCP_PORT,
    0x1D0,
    0xFFFFFFFF,
    (BIT5| BIT4| BIT3| BIT2| BIT1| BIT0)
    );
  DEBUG ((DEBUG_INFO, "End SC_PM_ENABLE\n"));

#endif

  //
  // Perform PWM configuration
  //
  ConfigurePwm (SiPolicy);

  DEBUG ((DEBUG_INFO, "ConfigurePmc() End\n"));

  return EFI_SUCCESS;
}

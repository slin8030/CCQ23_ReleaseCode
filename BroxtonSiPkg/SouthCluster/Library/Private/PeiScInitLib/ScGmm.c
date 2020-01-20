/** @file
  Initializes SC Gaussian Mixture Models

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

@par Specification
**/
#include "ScInitPei.h"
#include <Library/PreSiliconLib.h>

/**
  Configure GMM

  @param[in] SiPolicyPpi          The Silicon Policy PPI instance
  @param[in] ScPolicyPpi          The SC Policy Ppi instance
  @param[in, out] FuncDisableReg  The value of Function disable register

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureGmm (
  IN  SI_POLICY_PPI               *SiPolicyPpi,
  IN  SC_POLICY_PPI               *ScPolicyPpi,
  IN OUT UINT32                   *FuncDisableReg
  )
{
  EFI_STATUS            Status;
  UINTN                 GmmPciMmBase;
  EFI_PHYSICAL_ADDRESS  GmmMmioBase0;
  SC_GMM_CONFIG         *GmmConfig;

  DEBUG ((DEBUG_INFO, "ConfigureGmm() Start\n"));

  Status       = EFI_SUCCESS;

  GmmPciMmBase = MmPciBase (
                   DEFAULT_PCI_BUS_NUMBER_SC,
                   PCI_DEVICE_NUMBER_GMM,
                   PCI_FUNCTION_NUMBER_GMM
                   );
  GmmMmioBase0 = (EFI_PHYSICAL_ADDRESS) SiPolicyPpi->TempMemBaseAddr;
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gGmmConfigGuid, (VOID *) &GmmConfig);
  ASSERT_EFI_ERROR (Status);

  ///
  /// If GMM is disabled by fuse or soft-strap,
  /// set the function disable bit and then return.
  ///
  if ((MmioRead32 ((UINTN) GmmPciMmBase) == 0xFFFFFFFF) || (PLATFORM_ID != VALUE_REAL_PLATFORM)) {
    DEBUG ((DEBUG_INFO, "GMM not present, skipping.\n"));
    GmmConfig->Enable = FALSE;
    *FuncDisableReg |= B_PMC_FUNC_DIS_GMM;
    return EFI_SUCCESS;
  }

  ///
  /// Disable Bus Master Enable & Memory Space Enable
  ///
  MmioAnd32 ((UINTN) (GmmPciMmBase + R_GMM_STSCMD), (UINT32) ~(B_GMM_STSCMD_BME | B_GMM_STSCMD_MSE));

  ///
  /// Program Lower BAR
  ///
  ASSERT (((GmmMmioBase0 & B_GMM_BAR0_BA) == GmmMmioBase0) && (GmmMmioBase0 != 0));
  MmioWrite32 ((UINTN) (GmmPciMmBase + R_GMM_BAR0_LO), (UINT32) (GmmMmioBase0 & B_GMM_BAR0_BA));

  ///
  /// Program Upper BAR
  ///
  MmioWrite32 ((UINTN) (GmmPciMmBase + R_GMM_BAR0_HI), (UINT32) (0x00));

  ///
  /// Bus Master Enable & Memory Space Enable
  ///
  MmioOr32 ((UINTN) (GmmPciMmBase + R_GMM_STSCMD), (UINT32) (B_GMM_STSCMD_BME | B_GMM_STSCMD_MSE));
  ASSERT (MmioRead32 ((UINTN) GmmMmioBase0) != 0xFFFFFFFF);

  ///
  /// BXT BIOS Spec, Section 31.2 Disabling Intel* GMM
  ///
  if (GmmConfig->Enable == FALSE) {
    ///
    /// Step 1
    /// Enable power gating
    /// Set PCI[GMM] + B2h [5] = 1b
    ///
    DEBUG ((DEBUG_INFO, "GMM: Enabling the power gating.\n"));
    MmioAndThenOr16 ((UINTN) (GmmPciMmBase + R_GMM_D0I3_PWRCTRLEN), (UINT16) B_GMM_D0I3_PWRCTRLEN_PG_MASK, (UINT16) B_GMM_D0I3_PWRCTRLEN_HW_AUTO);
    ///
    /// Step 2
    /// Put device into D0i3 state
    /// Set PCR[GMM] + A8h [2] = 1b
    ///
    DEBUG ((DEBUG_INFO, "GMM: Putting GMM into D0i3 state.\n"));
    MmioOr8 ((UINTN) (GmmMmioBase0 + R_GMM_D0I3C), (UINT8) B_GMM_D0I3C_D0I3);
    ///
    /// Step 3
    /// Put device into D3 hot state
    /// Set PCI[GMM] + E0h [1:0] = 3b
    ///
    DEBUG ((DEBUG_INFO, "GMM: Putting GMM into D3 Hot State.\n"));
    MmioOr16 ((UINTN) (GmmPciMmBase + R_GMM_PCS), (UINT16) B_GMM_PCS_PS);
    ///
    /// Step 4
    /// Disable PCI function
    /// Set PCR[PSF1] + PSF_1_AGNT_T0_SHDW_PCIEN_GMM_RS0_D0_F3[8] = 1b
    ///
    DEBUG ((DEBUG_INFO, "GMM: Disable PCI function.\n"));
    SideBandAndThenOr32(
      SB_PORTID_PSF1,
      R_PCH_PCR_PSF1_T0_SHDW_GMM_PCIEN,
      0xFFFFFFFF,
      (UINT32)B_PCH_PCR_PSF1_T0_SHDW_GMM_PCIEN_FUNDIS
      );
    ///
    /// Step 5
    /// Set function disable register
    /// Set PMC + 34h [31] = 1b
    DEBUG ((DEBUG_INFO, "GMM: Set function disable bit.\n"));
    *FuncDisableReg |= B_PMC_FUNC_DIS_GMM;
  } else {
    ///
    /// Do GMM MMIO programming here if any.
    ///
  }

  ///
  /// Disable Bus Master Enable & Memory Space Enable
  ///
  MmioAnd32 ((UINTN) (GmmPciMmBase + R_GMM_STSCMD), (UINT32) ~(B_GMM_STSCMD_BME | B_GMM_STSCMD_MSE));

  ///
  /// Clear BAR0
  ///
  MmioWrite32 ((UINTN) (GmmPciMmBase + R_GMM_BAR0_LO), (UINT32) (0x00));


  DEBUG ((DEBUG_INFO, "ConfigureGmm() End\n"));

  return EFI_SUCCESS;
}

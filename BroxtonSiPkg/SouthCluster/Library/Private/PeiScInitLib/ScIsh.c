/** @file
  Initializes SC ISH Devices

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

/**
  Configure ISH devices.

  @param[in]  ScPolicy
  @param[in]  FuncDisableReg

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureIsh (
  IN  SC_POLICY_PPI  *ScPolicy,
  IN OUT UINT32      *FuncDisableReg
  )
{
  EFI_STATUS     Status;
  UINTN          IshPciBase;
  SC_ISH_CONFIG  *IshConfig;

  DEBUG ((DEBUG_INFO, "ConfigureIsh() Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicy, &gIshConfigGuid, (VOID *) &IshConfig);
  ASSERT_EFI_ERROR (Status);

  IshPciBase = MmPciBase (
                 DEFAULT_PCI_BUS_NUMBER_SC,
                 PCI_DEVICE_NUMBER_ISH,
                 PCI_FUNCTION_NUMBER_ISH
                );

  DEBUG ((DEBUG_INFO, "ISH IshPciBase: 0x%08X, Value: 0x%08X\n", IshPciBase, MmioRead32 (IshPciBase)));

  ///
  /// Skip ISH initialization and return if
  /// ISH is disabled by fuse or soft strap
  ///
  if (MmioRead16 (IshPciBase) == 0xFFFF) {
    DEBUG ((DEBUG_INFO, "ISH not detected, skipped!\n"));
    if (IshConfig->Enable == FALSE) {
      *FuncDisableReg |= B_PMC_FUNC_DIS_ISH;
    }
    return EFI_SUCCESS;
  }
  DEBUG ((DEBUG_INFO, "IshConfig->Enable: %X\n", IshConfig->Enable ));

  ///
  /// ISH Device Configuration
  ///
  if (IshConfig->Enable == FALSE) {
    ///
    /// Put device in D3hot state.
    ///
    DEBUG ((DEBUG_INFO, "IshPciBase + R_ISH_PCS: 0x%08X\n", MmioRead32 ((UINTN) (IshPciBase + R_ISH_PCS))));
    DEBUG ((DEBUG_INFO, "Putting ISH into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (IshPciBase + R_ISH_PCS), B_ISH_PCS_PS);
    DEBUG ((DEBUG_INFO, "IshPciBase + R_ISH_PCS: 0x%08X\n", MmioRead32 ((UINTN) (IshPciBase + R_ISH_PCS))));

    //
    //[HSDES][1206524017]IOSF2OCP oob_bme and oob_in_d3 indications are not reliable - ISH, need to read back before write.
    //
    MmioRead32((UINTN)(IshPciBase + R_ISH_PCS));

    ///
    /// Disable PCI function by PSF FunDis bit.
    /// Set PCR[PSF3] + PSF_3_AGNT_T0_SHDW_PCIEN_ISH_RS0_D17_F0[8] = 1b
    ///
    DEBUG ((DEBUG_INFO, "ISH PCIEN register is: 0x%08X\n", SideBandRead32(SB_PORTID_PSF3, R_PCH_PCR_PSF3_T0_SHDW_ISH_PCIEN)));
    DEBUG ((DEBUG_INFO, "Disable ISH PCI function.\n"));
    SideBandAndThenOr32(
      SB_PORTID_PSF3,
      R_PCH_PCR_PSF3_T0_SHDW_ISH_PCIEN,
      0xFFFFFFFF,
      (UINT32) B_PCH_PCR_PSF3_T0_SHDW_ISH_PCIEN_FUNDIS
      );
    DEBUG ((DEBUG_INFO, "ISH PCIEN register is: 0x%08X\n", SideBandRead32(SB_PORTID_PSF3, R_PCH_PCR_PSF3_T0_SHDW_ISH_PCIEN)));

    ///
    /// Set function disable register
    /// Set PMC + 34h [24] = 1b
    *FuncDisableReg |= B_PMC_FUNC_DIS_ISH;
    DEBUG ((DEBUG_INFO, "FuncDisableReg: 0x%08X\n", *FuncDisableReg));
  } else {
    //
    // Configure in PCI mode - disable BAR1 (Shadowed PCI)
    // PCICFGCTR.BAR1_DISABLE, offset (200h) [7] = 1b
    //
    SideBandAndThenOr32 (
      SB_ISH_PHY_PORT,
      R_ISH_SB_PCICFGCTRL_ISH,
      0xFFFFFFFF,
      B_ISH_SB_PCICFGCTRL_BAR1DIS
      );

    DEBUG ((DEBUG_INFO, "PCICFGCTR.BAR1_DISABLE, PCICFGCTR: 0x%08X\n", SideBandRead32 (SB_ISH_PHY_PORT,R_ISH_SB_PCICFGCTRL_ISH) ));

    //
    // Set PCR[PSF3] + "AGNT_T0_SHDW_PCIEN"[ISH][19:18] = 11b
    //
    SideBandAndThenOr32 (
      SB_PORTID_PSF3,
      R_PCH_PCR_PSF3_T0_SHDW_ISH_PCIEN,
      0xFFFFFFFF,
      (B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR2DIS | B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR3DIS)
      );

    DEBUG ((DEBUG_INFO, "Set PCR[PSF3] + AGNT_T0_SHDW_PCIEN [ISH][19:18] = 11b, AGNT_T0_SHDW_PCIEN: 0x%08X\n", SideBandRead32 (SB_PORTID_PSF3,R_PCH_PCR_PSF3_T0_SHDW_ISH_PCIEN) ));
#ifdef SC_PM_ENABLE
  if ((PLATFORM_ID == VALUE_REAL_PLATFORM) ||
      (PLATFORM_ID == VALUE_SLE_UPF)) {
    DEBUG ((DEBUG_INFO, "Start SC_PM_ENABLE\n"));
    SideBandAndThenOr32(
      SB_ISH_PHY_PORT,
      R_ISH_SB_PMCTL,
      0xFFFFFFFF,
      (B_IOSFPRIMCLK_GATE_EN |
       B_OCPCLK_GATE_EN |
       B_OCPCLK_TRUNK_GATE_EN |
       B_IOSFSBCLK_GATE_EN |
       B_IOSFPRIM_TRUNK_GATE_EN |
       B_IOSFSB_TRUNK_GATE_EN)
    );
    DEBUG ((DEBUG_INFO, "End SC_PM_ENABLE\n"));
  }
#endif

    //
    // Set BME bit
    //
    MmioOr32 (IshPciBase + R_ISH_STSCMD, B_ISH_STSCMD_BME);
    DEBUG ((EFI_D_INFO, "Set BME bit, CMD Reg: 0x%04X\n", MmioRead16 (IshPciBase + R_ISH_STSCMD)));
  }
  DEBUG ((DEBUG_INFO, "ConfigureIsh() End\n"));

  return EFI_SUCCESS;
}



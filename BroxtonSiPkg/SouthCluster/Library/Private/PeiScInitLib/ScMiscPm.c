/** @file
  Initializes power management features.

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
  Perform Clock Gating programming
  Enables clock gating in various SC interfaces and the registers

  @param[in] ScPolicy     The SC Policy Ppi instance

  @retval EFI_SUCCESS     The function completed successfully
**/
EFI_STATUS
ConfigureClockGating (
  IN  SC_POLICY_PPI *ScPolicy
  )
{
  UINT8             DevIndex;
  UINT32            LpssRegData32;
  UINTN             GmmBase;
  SC_GMM_CONFIG     *GmmConfig;
  SC_LPSS_CONFIG    *LpssConfig;
  UINT32            Data32Or;
  EFI_STATUS        Status;

  DEBUG ((DEBUG_INFO, "ConfigureClockGating() Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicy, &gGmmConfigGuid, (VOID *) &GmmConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicy, &gLpssConfigGuid, (VOID *) &LpssConfig);
  ASSERT_EFI_ERROR (Status);

  LpssRegData32 = 0;
  Data32Or      = 0;
  ///
  /// 4
  /// USB 1.1 / USB 2.0 / USB 3.0
  ///
  ConfigureUsbClockGating ();

  ///
  /// GMM Clock Gating Configuration
  ///
  DEBUG ((DEBUG_INFO, "Configure GMM Clock Gating\n"));
  GmmBase = MmPciBase (
              DEFAULT_PCI_BUS_NUMBER_SC,
              PCI_DEVICE_NUMBER_GMM,
              PCI_FUNCTION_NUMBER_GMM
              );
  if (GmmConfig->ClkGatingPgcbClkTrunk == TRUE) {
    Data32Or |= B_GMM_OVRCFGCTL_PGCBCGEN;
    DEBUG ((DEBUG_INFO, "GMM: Enable clock gate - PGCB Clock Trunk.\n"));
  }
  if (GmmConfig->ClkGatingSb == TRUE) {
    Data32Or |= B_GMM_OVRCFGCTL_SBDCGEN;
    DEBUG ((DEBUG_INFO, "GMM: Enable clock gate - Sideband.\n"));
  }
  if (GmmConfig->ClkGatingSbClkTrunk == TRUE) {
    Data32Or |= B_GMM_OVRCFGCTL_SBTCGEN;
    DEBUG ((DEBUG_INFO, "GMM: Enable clock gate - Sideband Trunk.\n"));
  }
  if (GmmConfig->ClkGatingSbClkPartition == TRUE) {
    Data32Or |= B_GMM_OVRCFGCTL_SBPCGEN;
    DEBUG ((DEBUG_INFO, "GMM: Enable clock gate - Sideband Partition.\n"));
  }
  if (GmmConfig->ClkGatingCore == TRUE) {
    Data32Or |= B_GMM_OVRCFGCTL_C_DCGEN;
    DEBUG ((DEBUG_INFO, "GMM: Enable clock gate - Core.\n"));
  }
  if (GmmConfig->ClkGatingDma == TRUE) {
    Data32Or |= B_GMM_OVRCFGCTL_DMA_DCGEN;
    DEBUG ((DEBUG_INFO, "GMM: Enable clock gate - DMA.\n"));
  }
  if (GmmConfig->ClkGatingRegAccess == TRUE) {
    Data32Or |= B_GMM_OVRCFGCTL_RA_DCGEN;
    DEBUG ((DEBUG_INFO, "GMM: Enable clock gate - Register Access.\n"));
  }
  if (GmmConfig->ClkGatingHost == TRUE) {
    Data32Or |= B_GMM_OVRCFGCTL_HOST_DCGEN;
    DEBUG ((DEBUG_INFO, "GMM: Enable clock gate - Host Interface.\n"));
  }
  if (GmmConfig->ClkGatingPartition == TRUE) {
    Data32Or |= B_GMM_OVRCFGCTL_PCGEN;
    DEBUG ((DEBUG_INFO, "GMM: Enable clock gate - Partition.\n"));
  }
  if (GmmConfig->ClkGatingTrunk == TRUE) {
    Data32Or |= B_GMM_OVRCFGCTL_TCGEN;
    DEBUG ((DEBUG_INFO, "GMM: Enable clock gate - Trunk.\n"));
  }
  MmioOr32 ((UINTN) (GmmBase + R_GMM_OVRCFGCTL), Data32Or);

  ///
  /// LPSS Clock Gating Configuration
  ///

  ///
  /// Configure I2C Clock Gating
  ///
  DEBUG ((DEBUG_INFO, "Configure LPSS I2C Clock Gating\n"));
  for (DevIndex = 0; DevIndex < LPSS_I2C_DEVICE_NUM; DevIndex++) {
    if (LpssConfig->I2cClkGateCfg[DevIndex] == FALSE) {
      DEBUG ((DEBUG_INFO, "Clock gating of I2C device(%d) is disabled\n",DevIndex));
      LpssRegData32 |= (BIT0 << DevIndex);
    }
  }
  SideBandAndThenOr32(
    LPSS_EP_PORT_ID,
    R_LPSS_SB_GPPRVRW4,
    (UINT32) ~B_LPSS_I2C_CG,
    (UINT32) LpssRegData32
    );

  ///
  /// Configure UART Clock Gating
  ///
  LpssRegData32 = 0;
  DEBUG ((DEBUG_INFO, "Configure LPSS UART Clock Gating\n"));
  for (DevIndex = 0; DevIndex < LPSS_HSUART_DEVICE_NUM; DevIndex++) {
    if (LpssConfig->HsuartClkGateCfg[DevIndex] == FALSE) {
      DEBUG ((DEBUG_INFO, "Clock gating of UART device(%d) is disabled\n",DevIndex));
      LpssRegData32 |= (BIT0 << DevIndex);
    }
  }
  SideBandAndThenOr32(
    LPSS_EP_PORT_ID,
    R_LPSS_SB_GPPRVRW5,
    (UINT32)~B_LPSS_UART_CG,
    (UINT32)LpssRegData32
    );

  ///
  /// Configure SPI Clock Gating
  ///
  LpssRegData32 = 0;
  DEBUG ((DEBUG_INFO, "Configure LPSS SPI Clock Gating\n"));
  for (DevIndex = 0; DevIndex < LPSS_SPI_DEVICE_NUM; DevIndex++) {
    if (LpssConfig->SpiClkGateCfg[DevIndex] == FALSE) {
      DEBUG ((DEBUG_INFO, "Clock gating of SPI device(%d) is disabled\n",DevIndex));
      LpssRegData32 |= (BIT0 << DevIndex);
    }
  }
  SideBandAndThenOr32(
    LPSS_EP_PORT_ID,
    R_LPSS_SB_GPPRVRW6,
    (UINT32)~B_LPSS_SPI_CG,
    (UINT32)LpssRegData32
    );

  DEBUG ((DEBUG_INFO, "ConfigureClockGating() End\n"));

  return EFI_SUCCESS;
}

/**
  Perform Power Gating programming
  Enables power gating in various SC interfaces and the registers must be restored during S3 resume.

  @param[in] ScPolicyPpi     The SC Policy Ppi instance

  @retval EFI_SUCCESS        The function completed successfully
**/
EFI_STATUS
ConfigurePowerGating (
  IN  SC_POLICY_PPI *ScPolicyPpi
  )
{
  UINTN             GmmBase;
  SC_GMM_CONFIG     *GmmConfig;
  UINT16            Data16And;
  UINT16            Data16Or;
  EFI_STATUS        Status;

  DEBUG ((DEBUG_INFO, "ConfigurePowerGating() Start\n"));

  ///
  /// GMM Power Gating Configuration
  ///
  Data16And = 0;
  Data16Or  = 0;
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gGmmConfigGuid, (VOID *) &GmmConfig);
  ASSERT_EFI_ERROR (Status);
  GmmBase = MmPciBase (
              DEFAULT_PCI_BUS_NUMBER_SC,
              PCI_DEVICE_NUMBER_GMM,
              PCI_FUNCTION_NUMBER_GMM
              );
  if (GmmConfig->SvPwrGatingHwAutoEnable == FALSE) {
    Data16And = (UINT16) ~B_GMM_D0I3_PWRCTRLEN_HW_AUTO;
    DEBUG ((DEBUG_INFO, "GMM: Disable power gate - HW Autonomous.\n"));
  }
  if (GmmConfig->SvPwrGatingD3HotEnable) {
    Data16Or |= B_GMM_D0I3_PWRCTRLEN_D3_HOT;
    DEBUG ((DEBUG_INFO, "GMM: Enable power gate - D3 hot.\n"));
  }
  if (GmmConfig->SvPwrGatingI3Enable) {
    Data16Or |= B_GMM_D0I3_PWRCTRLEN_I3;
    DEBUG ((DEBUG_INFO, "GMM: Enable power gate - I3.\n"));
  }
  if (GmmConfig->SvPwrGatingPmcReqEnable) {
    Data16Or |= B_GMM_D0I3_PWRCTRLEN_PMCR;
    DEBUG ((DEBUG_INFO, "GMM: Enable power gate - PMC Request.\n"));
  }
  MmioAndThenOr16 ((UINTN) (GmmBase + R_GMM_D0I3_PWRCTRLEN), Data16And, Data16Or);

  DEBUG ((DEBUG_INFO, "ConfigurePowerGating() End\n"));

  return EFI_SUCCESS;
}

/**
  Configure miscellaneous power management settings

  @param[in] ScPolicy     The SC Platform Policy Ppi instance
  @param[in] PmcBase              PMC base address of this SC device
  @param[in] GpioBase             GPIO base address of this SC device

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureMiscPm (
  IN  SC_POLICY_PPI *ScPolicy,
  IN  UINT32        PmcBase
  )
{
  UINT32            RegData32;
  UINT16            RegData16;
  EFI_STATUS        Status;
  SC_GENERAL_CONFIG *GeneralConfig;
  SC_PM_CONFIG      *PmConfig;

  DEBUG ((DEBUG_INFO, "ConfigureMiscPm() Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicy, &gScGeneralConfigGuid, (VOID *) &GeneralConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicy, &gPmConfigGuid, (VOID *) &PmConfig);
  ASSERT_EFI_ERROR (Status);

  // Set CRID
  RegData32 = 0;
  if (GeneralConfig->Crid != 0) {
    RegData32 = (MmioRead32 (PmcBase + R_PMC_CRID) & (~B_PMC_CRID_RID_SEL)) | (UINT32) GeneralConfig->Crid;
    MmioWrite32 (PmcBase + R_PMC_CRID, RegData32);
  }
  // Handle wake policy
  RegData16 = MmioRead16 (PmcBase + R_PMC_GEN_PMCON_1) & (UINT16) (~B_PMC_GEN_PMCON_PME_B0_S5_DIS);
  if (PmConfig->WakeConfig.PmeB0S5Dis) {
    RegData16 |= B_PMC_GEN_PMCON_PME_B0_S5_DIS;
  }
  MmioWrite16 (PmcBase + R_PMC_GEN_PMCON_1, RegData16);

  //
  // Program PM_CFG2 for power button settings
  //
  RegData32 = 0;
  ///
  /// Set power button native mode disable
  ///
  if (PmConfig->DisableNativePowerButton) {
    RegData32 |= B_PMC_CFG2_PB_DIS;
  }
  ///
  /// Set power button debounce mode
  ///
  if (PmConfig->PowerButterDebounceMode) {
    RegData32 |= B_PMC_CFG2_PB_PWRBTN_DB_MODE;
  }
  MmioOr32 (PmcBase + R_PMC_CFG2, RegData32);
  ///
  /// Set power button override period
  ///
  MmioAndThenOr32(PmcBase + R_PMC_CFG2, ~B_PMC_CFG2_PBOP, (PmConfig->PwrBtnOverridePeriod) << N_PMC_CFG2_PBOP);

  DEBUG ((DEBUG_INFO, "ConfigureMiscPm() End\n"));

  return EFI_SUCCESS;
}


/**
  Configure S0ix Settings

  @param[in] PmcBase   PMC Base Address value of this SC device

  @retval EFI_SUCCESS  Successfully completed.
**/
EFI_STATUS
ConfigureS0ix (
  IN  UINT32  PmcBase
  )
{

  DEBUG ((DEBUG_INFO, "ConfigureS0ix() Start \n"));

  //
  // Enable S0ix in the registers, Program max_s0ix
  //
  MmioOr32 (PmcBase + R_PMC_ETR, B_PMC_ETR_MAX_S0IX);

  DEBUG ((DEBUG_INFO, "ConfigureS0ix() End \n"));

  return EFI_SUCCESS;
}

/**
  Perform miscellany SC initialization

  @param[in] PmcBase  PmcBase value of this SC device

  @retval EFI_SUCCESS The function completed successfully
**/
EFI_STATUS
ConfigureMiscItems (
  IN UINT32 PmcBase
  )
{
  DEBUG ((DEBUG_INFO, "ConfigureMiscItems() Start\n"));
  ///
  /// Set the RTC_PWR_STS bit (PBASE + 0x20h [2]) when the RTCRST# pin goes low.
  /// The System BIOS shouldn't rely on the RTC RAM contents when the RTC_PWR_STS bit is set.
  /// BIOS should clear this bit by writing a 0 to this bit position.
  /// This bit isn't cleared by any reset function.
  ///

  //
  // B_PMC_GEN_PMCON_RTC_PWR_STS handled already , don't clear it.
  //

  //MmioAnd8 ((UINTN) (PmcBase + R_PMC_GEN_PMCON_1), (UINT8) (~(B_PMC_GEN_PMCON_RTC_PWR_STS)));

  DEBUG ((DEBUG_INFO, "ConfigureMiscItems() End\n"));

  return EFI_SUCCESS;
}

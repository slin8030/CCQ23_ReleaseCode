/** @file

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/** @file
  The SC Init After Memory PEI module.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2017 Intel Corporation.

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
#include <Library/SteppingLib.h>
#include <Library/HeciMsgLib.h>
#include <Private/ScPmcFunctionDisableResetHob.h>

//[-start-151224-IB07220029-add]//
#include <Ppi/EmuPei.h>
//[-end-151224-IB07220029-add]//
//[-start-160216-IB07400702-add]//
#include <ChipsetPostCode.h>
//[-end-160216-IB07400702-add]//
/**
  Sc End of PEI callback function. This is the last event before entering DXE and OS in S3 resume.

  @param[in]  None

  @retval  None
**/
VOID
ScOnEndOfPei (
  VOID
  )
{
  EFI_STATUS      Status;
  SC_POLICY_PPI   *ScPolicy;
  SI_POLICY_PPI   *SiPolicy;
  UINTN           P2sbBase;
  EFI_BOOT_MODE   BootMode;
  SC_LPSS_CONFIG  *LpssConfig;

  PostCode (0xB40); // PEI_SC_ON_END_OF_PEI_START
  DEBUG ((DEBUG_INFO, "ScOnEndOfPei after memory PEI module - Start\n"));
  //
  // Get Policy settings through the SiPolicy PPI
  //
  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPolicy
             );
  if (Status != EFI_SUCCESS) {
    //
    // SI_POLICY_PPI must be installed at this point
    //
    ASSERT (FALSE);
    return;
  }

  //
  // Get Policy settings through the ScPolicy PPI
  //
  Status = PeiServicesLocatePpi (
             &gScPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &ScPolicy
             );
  if (Status != EFI_SUCCESS) {
    //
    // SC_POLICY_PPI must be installed at this point
    //
    ASSERT (FALSE);
    return;
  }

  P2sbBase = MmPciBase (
               DEFAULT_PCI_BUS_NUMBER_SC,
               PCI_DEVICE_NUMBER_P2SB,
               PCI_FUNCTION_NUMBER_P2SB
               );

  Status = PeiServicesGetBootMode (&BootMode);
  ASSERT_EFI_ERROR (Status);

  //
  // For S3 path:
  //   Hide P2SB
  //   Hide UART2 if Kernel Debug Enabled
  //
  if (BootMode == BOOT_ON_S3_RESUME) {
#ifndef FSP_FLAG
    MmioWrite8 (P2sbBase + R_P2SB_P2SBC + 1, BIT0);
#endif
    // Hide the UART2 device to prevent an OS driver from loading against it
    Status = GetConfigBlock ((VOID *) ScPolicy, &gLpssConfigGuid, (VOID *) &LpssConfig);
    ASSERT_EFI_ERROR (Status);
    if (LpssConfig->OsDbgEnable) {
      SideBandAndThenOr32(
        SB_PORTID_PSF3,
        R_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2,
        0xFFFFFFFF,
        B_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2_CFGDIS
        );
    }
  }
  DEBUG ((DEBUG_INFO, "ScOnEndOfPei after memory PEI module - End\n"));
  PostCode (0xB4F); // PEI_SC_ON_END_OF_PEI_END
}

/**
  Perform DCI configuration.

  @param[in] ScPolicy    The SC Policy PPI instance
**/
VOID
DciConfiguration (
  IN  SC_POLICY_PPI  *ScPolicyPpi
  )
{
  EFI_STATUS    Status;
  SC_DCI_CONFIG *DciConfig;

  ///
  /// Set PCR[DCI] + 30h bit[5] to 1, and clear 30h bit[0] = 0
  ///
  SideBandAndThenOr32 (
    PID_DCI,
    R_PCR_DCI_PCE,
    (UINT32)~B_PCR_DCI_PCE_PMCRE,
    B_PCR_DCI_PCE_HAE
    );

  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gDciConfigGuid, (VOID *) &DciConfig);
  ASSERT_EFI_ERROR (Status);

  if (DciConfig->DciEn) {
    //
    // Set PCR[DCI] + 4h bit[4] to 1 if DCI is enabled.
    //
    SideBandAndThenOr32 (PID_DCI, R_PCR_DCI_ECTRL, ~0u, B_PCR_DCI_ECTRL_HDCIEN);
  } else {
    //
    // Set PCR[DCI] + 4h bit[4] to 0 if DCI is disabled.
    //
    SideBandAndThenOr32 (PID_DCI, R_PCR_DCI_ECTRL, (UINT32) ~(B_PCR_DCI_ECTRL_HDCIEN), 0);
  }
  //
  // Set PCR[DCI] + 4h bit[0] to 1
  //
  SideBandAndThenOr32 (PID_DCI, R_PCR_DCI_ECTRL, ~0u, B_PCR_DCI_ECTRL_HDCIEN_LOCK);
}

/**
  Sc init after memory PEI module

  @param[in] SiPolicy    The Silicon Policy PPI instance
  @param[in] ScPolicy    The SC Policy PPI instance

  @retval None
**/
VOID
EFIAPI
ScInit (
  IN  SI_POLICY_PPI  *SiPolicy,
  IN  SC_POLICY_PPI  *ScPolicyPpi
  )
{
  EFI_STATUS                        Status;
  SC_PCIE_DEVICE_OVERRIDE           *PcieDeviceTable;
  VOID                              *HobPtr;
  UINTN                             Count;
  UINTN                             LpcBaseAddress;
  UINTN                             SpiBaseAddress;
  UINT32                            PmcBase;
  UINT32                            FuncDisableReg;
  UINT32                            FuncDisable1Reg;
  UINT32                            OriginalFuncDisableReg;
  UINT32                            OriginalFuncDisable1Reg;
  UINT32                            NewFuncDisableReg;
  UINT32                            NewFuncDisable1Reg;
  UINT32                            FuncDisableChangeStatus;
  UINT32                            FuncDisable1ChangeStatus;
  BXT_STEPPING                      Stepping;
  SC_LOCK_DOWN_CONFIG               *LockDownConfig;
  SC_GENERAL_CONFIG                 *GeneralConfig;
  UINT8                             IpcWbuffer[4] = {0, 0, 0, 0};
  UINT32                            Data32And;
  SC_PMC_FUNCTION_DISABLE_RESET_HOB FuncDisHob;
//[-start-151224-IB07220029-add]//
  VOID                    *EmuPeiPpi;
//[-end-151224-IB07220029-add]//

  PostCode (0xB00); // SC PostMem Entry // PEI_FRC_SC_INIT_START
  DEBUG ((DEBUG_INFO, "ScInit - Start\n"));

  LpcBaseAddress = MmPciBase (
                     DEFAULT_PCI_BUS_NUMBER_SC,
                     PCI_DEVICE_NUMBER_PCH_LPC,
                     PCI_FUNCTION_NUMBER_PCH_LPC
                     );
  SpiBaseAddress = MmPciBase (
                     DEFAULT_PCI_BUS_NUMBER_SC,
                     PCI_DEVICE_NUMBER_SPI,
                     PCI_FUNCTION_NUMBER_SPI
                     );

  //
  // Locate and PCIe device override table and add to HOB area.
  //
  Status = PeiServicesLocatePpi (
             &gScPcieDeviceTablePpiGuid,
             0,
             NULL,
             (VOID **)&PcieDeviceTable
             );

  if (Status == EFI_SUCCESS) {
    for (Count = 0; PcieDeviceTable[Count].DeviceId != 0; Count++) {
    }
    DEBUG ((DEBUG_INFO, "SC Installing PcieDeviceTable HOB (%d entries)\n", Count));
    HobPtr = BuildGuidDataHob (
               &gScDeviceTableHobGuid,
               PcieDeviceTable,
               Count * sizeof (SC_PCIE_DEVICE_OVERRIDE)
               );
    ASSERT (HobPtr != 0);
  }
  HobPtr = NULL;
  HobPtr = BuildGuidDataHob (&gScPolicyHobGuid, ScPolicyPpi, ScGetConfigBlockTotalSize ());
  ASSERT (HobPtr != 0);

  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gScGeneralConfigGuid, (VOID *) &GeneralConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gLockDownConfigGuid, (VOID *) &LockDownConfig);
  ASSERT_EFI_ERROR (Status);

  PmcBase = GeneralConfig->PmcBase;
  OriginalFuncDisableReg  = MmioRead32 (PmcBase + R_PMC_FUNC_DIS);
  OriginalFuncDisable1Reg = MmioRead32 (PmcBase + R_PMC_FUNC_DIS_1);

  FuncDisableReg  = 0;
  FuncDisable1Reg = 0;
  DEBUG ((DEBUG_INFO, "FuncDisableReg: 0x%08X\n", OriginalFuncDisableReg));
  DEBUG ((DEBUG_INFO, "FuncDisable1Reg: 0x%08X\n", OriginalFuncDisable1Reg));

  //
  // Set and enable P2SB base
  //
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SC_INIT_ENABLE_P2SB_BASE);
//[-end-160217-IB07400702-add]//
  ScP2sbInit(GeneralConfig->P2sbBase);
  //
  // Program HSIO
  //
  PostCode (0xB01); // PEI_FRC_SC_INIT_PROGRAM_HSIO
  ScHsioBiosProg (SiPolicy, ScPolicyPpi);

  //
  // Init RTC
  //
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SC_INIT_INIT_RTC);
//[-end-160217-IB07400702-add]//
  ConfigureRtc ();

  //
  // Misc SC config init
  //
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SC_INIT_MISC_SC_CONFIG_INIT);
//[-end-160217-IB07400702-add]//
  ScMiscInit (ScPolicyPpi);

  //
  // Init IoApic
  //
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SC_INIT_MISC_SC_CONFIG_INIT);
//[-end-160217-IB07400702-add]//
  ScIoApicInit (ScPolicyPpi);

  //
  // Init BXTP specific IPs
  //
  if (GetBxtSeries () == BxtP) {
    //
    // Init Smbus
    //
    PostCode (0xB02); // PEI_FRC_SC_INIT_INIT_SMBUS
    ScSmbusConfigure(ScPolicyPpi);
    //
    // Init LPC
    //
    PostCode (0xB03); // PEI_FRC_SC_INIT_INIT_LPC
    ScLpcInit ();
    //
    // Init Sata
    //
    PostCode (0xB04); // PEI_FRC_SC_INIT_INIT_SATA
    ScSataInit (ScPolicyPpi, SiPolicy->TempMemBaseAddr, &FuncDisable1Reg);
  }

  //
  // Init PciExpress
  //
  Stepping = BxtStepping ();
  if ((Stepping >= BxtB0)) {
    PostCode (0xB05); // PEI_FRC_SC_INIT_INIT_PCIE
    ScInitRootPorts (ScPolicyPpi, SiPolicy->TempPciBusMin, SiPolicy->TempPciBusMax, &OriginalFuncDisableReg, &OriginalFuncDisable1Reg, &FuncDisableReg, &FuncDisable1Reg);
  }

  //
  // Program SVID and SID of SC devices.
  // Program SVID and SID before most SC device init since some device might be locked after init.
  //
//[-start-151220-IB11270138-remove]//
//[-start-160115-IB11270145-add]//
//[-start-160216-IB07400702-modify]//
  //
  // BugBug: H2ODDT hang here??
  //
  if (!FeaturePcdGet (PcdH2ODdtSupported)) {
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_FRC_SC_INIT_PROGRAM_SVID_SID);
//[-end-160217-IB07400702-add]//
    Status = ProgramSvidSid (ScPolicyPpi);
    ASSERT_EFI_ERROR (Status);
  }
//[-end-160216-IB07400702-modify]//
//[-end-160115-IB11270145-add]//
//[-end-151220-IB11270138-remove]//

  ///
  /// Flash Security Recommendation
  /// BIOS needs to enable the "Enable in SMM.STS" (EISS) feature of the PCH by setting
  /// SPI PCI offset DCh[5] = 1b for SPI or setting eSPI PCI offset DCh[5] = 1b for eSPI.
  /// When this bit is set, the BIOS region is not writeable until SMM sets the InSMM.STS bit,
  /// to ensure BIOS can only be modified from SMM. Please refer to CPU BWG for more details
  /// on InSMM.STS bit.
  ///
  if (LockDownConfig->SpiEiss == TRUE) {
    //
    // Set SPI EISS (SPI PCI offset DCh[5])
    // Set LPC/eSPI EISS (LPC/eSPI PCI offset DCh[5])
    //
    MmioOr8 (SpiBaseAddress + R_SPI_BCR, B_SPI_BCR_EISS);
    MmioOr8 (LpcBaseAddress + R_PCH_LPC_BC, B_PCH_LPC_BC_EISS);
  } else {
    //
    // Clear SMM_EISS (SPI PCI offset DCh[5])
    // Clear LPC/eSPI EISS (LPC/eSPI PCI offset DCh[5])
    // Since the HW default is 1, need to clear it when disabled in policy
    //
    MmioAnd8 (SpiBaseAddress + R_SPI_BCR, (UINT8)~B_SPI_BCR_EISS);
    MmioAnd8 (LpcBaseAddress + R_PCH_LPC_BC, (UINT8)~B_PCH_LPC_BC_EISS);
  }

  if (GetBxtSeries () == BxtP) {
    //
    // Configure Serial IRQ
    //
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_FRC_SC_INIT_CONFIG_SIRQ);
//[-end-160217-IB07400702-add]//
    ScConfigureSerialIrq (ScPolicyPpi);
  }

  //
  // Configure PM settings
  //
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SC_INIT_SC_PM_INIT);
//[-end-160217-IB07400702-add]//
  Status = ScPmInit (ScPolicyPpi);

  ///
  ///
  /// Miscellaneous power management handling
  ///
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SC_INIT_CONFIG_MISC_PM);
//[-end-160217-IB07400702-add]//
  Status = ConfigureMiscPm (ScPolicyPpi, PmcBase);

  ///
  /// S0ix Enabling
  ///
  if (GeneralConfig->S0ixSupport == TRUE) {
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_FRC_SC_INIT_CONFIG_S0IX);
//[-end-160217-IB07400702-add]//
    Status = ConfigureS0ix (PmcBase);
  }
  ///
  /// Perform SC initialization sequence
  ///
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SC_INIT_CONFIG_MISC_ITEMS);
//[-end-160217-IB07400702-add]//
  Status = ConfigureMiscItems (PmcBase);

  ///
  /// Configure USB
  ///
  PostCode (0xB06); // PEI_FRC_SC_INIT_SC_USB_INIT
  ScUsbInit (SiPolicy, ScPolicyPpi, &FuncDisableReg);
  //
  // Configure DCI
  //
  DciConfiguration (ScPolicyPpi);
  //
  // Enable ModPHY power gating
  //
  IpcWbuffer[0] |= BIT4;
  IpcSendCommandEx (IPC_CMD_ID_PHY_CFG,IPC_SUBCMD_ID_PHY_CFG_COMPLETE, &IpcWbuffer, 2);

  ////
  ///  Configure OTG Device
  ///
  PostCode (0xB07); // PEI_FRC_SC_INIT_CONFIG_OTG
  Status = ConfigureXdci (SiPolicy, ScPolicyPpi, &FuncDisableReg);
  ///
  /// Detect and initialize the type of codec present in the system
  ///
  PostCode (0xB08); // PEI_FRC_SC_INIT_CONFIG_HDA
  Status = ConfigureHda (SiPolicy, ScPolicyPpi, &FuncDisableReg);

  ///
  /// Configure GMM
  ///
  PostCode (0xB09); // PEI_FRC_SC_INIT_CONFIG_GMM
  Status = ConfigureGmm (SiPolicy, ScPolicyPpi, &FuncDisableReg);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Configure LPSS Devices
  ///
  PostCode (0xB0A); // PEI_FRC_SC_INIT_CONFIG_LPSS
  Status = ConfigureLpss (SiPolicy, ScPolicyPpi, &FuncDisableReg);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Configure SCS Devices
  ///
  PostCode (0xB0B); // PEI_FRC_SC_INIT_CONFIG_SCC
  Status = ConfigureScs (SiPolicy, ScPolicyPpi, &FuncDisableReg, &FuncDisable1Reg);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Configure ISH Devices
  ///
  PostCode (0xB0C); // PEI_FRC_SC_INIT_CONFIG_ISH
  Status = ConfigureIsh (ScPolicyPpi, &FuncDisableReg);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Configure IOSF Devices
  ///
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SC_INIT_CONFIG_IOSF);
//[-end-160217-IB07400702-add]//
  Status = ConfigureIosf ();
  ASSERT_EFI_ERROR (Status);

  ///
  /// Configure ITSS Devices
  ///
  PostCode (0xB0D); // PEI_FRC_SC_INIT_CONFIG_ITSS
  Status = ConfigureItss (ScPolicyPpi);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Configure PMC Devices
  ///
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SC_INIT_CONFIG_PMC);
//[-end-160217-IB07400702-add]//
  Status = ConfigurePmc (SiPolicy);
  ASSERT_EFI_ERROR (Status);

//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SC_INIT_VTD_PEI_INIT);
//[-end-160217-IB07400702-add]//
  VtdInit(ScPolicyPpi);

  //
  // Configure interrupts.
  //
  PostCode (0xB0E); // PEI_FRC_SC_INIT_CONFIG_INTERRUPTS
  Status = ScConfigureInterrupts (ScPolicyPpi);
  ASSERT_EFI_ERROR (Status);

  ///
  ///
  /// Perform clock gating register settings
  /// No clock gating configuration is required for now until there is fix needed by BIOS.
  ///
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SC_INIT_CONFIG_CLOCK_GATING);
//[-end-160217-IB07400702-add]//
  Status = ConfigureClockGating (ScPolicyPpi);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Perform power gating register settings
  ///
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SC_INIT_CONFIG_POWER_GATING);
//[-end-160217-IB07400702-add]//
  Status = ConfigurePowerGating (ScPolicyPpi);
  ASSERT_EFI_ERROR (Status);
  ///
  /// Set the PMC Function Disable Registers
  ///
  NewFuncDisableReg  = OriginalFuncDisableReg | FuncDisableReg;    // Update Original FUNC_DIS with new values
  NewFuncDisable1Reg = OriginalFuncDisable1Reg | FuncDisable1Reg;  // Update Original FUNC_DIS_1 with new values
  MmioAndThenOr32 ((UINTN) (PmcBase + R_PMC_FUNC_DIS), ~0u, NewFuncDisableReg);
  MmioAndThenOr32 ((UINTN) (PmcBase + R_PMC_FUNC_DIS_1), ~0u, NewFuncDisable1Reg);

  ///
  /// Reads back for posted write to take effect
  ///
  DEBUG ((DEBUG_INFO, "FuncDisableReg: 0x%08X\n", MmioRead32 ((UINTN) (PmcBase + R_PMC_FUNC_DIS))));
  DEBUG ((DEBUG_INFO, "FuncDisable1Reg: 0x%08X\n", MmioRead32 ((UINTN) (PmcBase + R_PMC_FUNC_DIS_1))));

  FuncDisableChangeStatus  = FuncDisableReg ^ OriginalFuncDisableReg;
  FuncDisable1ChangeStatus = FuncDisable1Reg ^ OriginalFuncDisable1Reg;
  FuncDisHob.ResetType = SC_PMC_FUNCTION_DISABLE_NO_RESET;
  if (FuncDisableChangeStatus & B_PMC_FUNC_DIS_AVS) {
    if (FuncDisableReg & B_PMC_FUNC_DIS_AVS) {
      FuncDisHob.ResetType = SC_PMC_FUNCTION_DISABLE_COLD_RESET;   // Changing from Enabled to Disabled
    } else {
      MmioAnd32 ((UINTN) (PmcBase + R_PMC_FUNC_DIS), (UINT32)~B_PMC_FUNC_DIS_AVS);
      FuncDisHob.ResetType = SC_PMC_FUNCTION_DISABLE_GLOBAL_RESET; // Changing from Disabled to Enabled
    }
  }
  if (FuncDisableChangeStatus & B_PMC_FUNC_DIS_ISH) {
    if (FuncDisableReg & B_PMC_FUNC_DIS_ISH) {
      FuncDisHob.ResetType = SC_PMC_FUNCTION_DISABLE_COLD_RESET;   // Changing from Enabled to Disabled
    } else {
      MmioAnd32 ((UINTN) (PmcBase + R_PMC_FUNC_DIS), (UINT32)~B_PMC_FUNC_DIS_ISH);
      FuncDisHob.ResetType = SC_PMC_FUNCTION_DISABLE_GLOBAL_RESET; // Changing from Disabled to Enabled
    }
  }

  if (FuncDisable1ChangeStatus & B_PMC_FUNC_DIS_1_SATA) {
    if ((FuncDisable1Reg & B_PMC_FUNC_DIS_1_SATA) != B_PMC_FUNC_DIS_1_SATA) {
      MmioAnd32 ((UINTN) (PmcBase + R_PMC_FUNC_DIS_1), (UINT32)~B_PMC_FUNC_DIS_1_SATA);
      FuncDisHob.ResetType = SC_PMC_FUNCTION_DISABLE_GLOBAL_RESET; // Changing from Disabled to Enabled
    } else {
      FuncDisHob.ResetType = SC_PMC_FUNCTION_DISABLE_COLD_RESET; // Changing from Enabled to Disabled
    }
  }

  if (BxtStepping() >= BxtB0) {
    if (FuncDisableChangeStatus & (B_PMC_FUNC_DIS_PCIE0_P0 | B_PMC_FUNC_DIS_PCIE0_P1)) {
      if ((B_PMC_FUNC_DIS_PCIE0_P0 & OriginalFuncDisableReg) &&
          (B_PMC_FUNC_DIS_PCIE0_P1 & OriginalFuncDisableReg)) {
        Data32And = ~(~FuncDisableReg & (B_PMC_FUNC_DIS_PCIE0_P0 | B_PMC_FUNC_DIS_PCIE0_P1));
        MmioAnd32 ((UINTN) (PmcBase + R_PMC_FUNC_DIS), Data32And);
        FuncDisHob.ResetType = SC_PMC_FUNCTION_DISABLE_GLOBAL_RESET; // Changing from Disabled to Enabled
      }
      if ((FuncDisableReg & B_PMC_FUNC_DIS_PCIE0_P0) &&
          (FuncDisableReg & B_PMC_FUNC_DIS_PCIE0_P1)) {
        FuncDisHob.ResetType = SC_PMC_FUNCTION_DISABLE_COLD_RESET;   // Changing from Enabled to Disabled
      }
    }
  }
  if (GetBxtSeries() == BxtP) {
    if (FuncDisable1ChangeStatus & (B_PMC_FUNC_DIS_1_PCIE1_P0 |
                                    B_PMC_FUNC_DIS_1_PCIE1_P1 |
                                    B_PMC_FUNC_DIS_1_PCIE1_P2 |
                                    B_PMC_FUNC_DIS_1_PCIE1_P3)) {
     if ((B_PMC_FUNC_DIS_1_PCIE1_P0 & OriginalFuncDisable1Reg) &&
          (B_PMC_FUNC_DIS_1_PCIE1_P1 & OriginalFuncDisable1Reg) &&
          (B_PMC_FUNC_DIS_1_PCIE1_P2 & OriginalFuncDisable1Reg) &&
          (B_PMC_FUNC_DIS_1_PCIE1_P3 & OriginalFuncDisable1Reg)) {
        Data32And = ~(~FuncDisable1Reg & (B_PMC_FUNC_DIS_1_PCIE1_P0 | B_PMC_FUNC_DIS_1_PCIE1_P1 | B_PMC_FUNC_DIS_1_PCIE1_P2 | B_PMC_FUNC_DIS_1_PCIE1_P3));
        MmioAnd32 ((UINTN) (PmcBase + R_PMC_FUNC_DIS_1), Data32And);
        FuncDisHob.ResetType = SC_PMC_FUNCTION_DISABLE_GLOBAL_RESET; // Changing from Disabled to Enabled
      }
      if ((FuncDisable1Reg & B_PMC_FUNC_DIS_1_PCIE1_P0) &&
          (FuncDisable1Reg & B_PMC_FUNC_DIS_1_PCIE1_P1) &&
          (FuncDisable1Reg & B_PMC_FUNC_DIS_1_PCIE1_P2) &&
          (FuncDisable1Reg & B_PMC_FUNC_DIS_1_PCIE1_P3)) {
        FuncDisHob.ResetType = SC_PMC_FUNCTION_DISABLE_COLD_RESET;   // Changing from Enabled to Disabled
      }
    }
  }
//[-start-151224-IB07220029-modify]//
  Status = PeiServicesLocatePpi (
                  &gEmuPeiPpiGuid,
                  0,
                  NULL,
                  (VOID **) &EmuPeiPpi
                  );
  if (!EFI_ERROR(Status)) {
    FuncDisHob.ResetType = SC_PMC_FUNCTION_DISABLE_NO_RESET;
  }
//[-end-151224-IB07220029-modify]//
  HobPtr = NULL;
  HobPtr = BuildGuidDataHob (&gScPmcFunctionDisableResetHobGuid, (VOID *) &FuncDisHob, sizeof (SC_PMC_FUNCTION_DISABLE_RESET_HOB));
  ASSERT (HobPtr != 0);

  ///
  /// Ensure PME_EN Bits are Cleared to Prevent SMI Interrupt Storm
  ///
  ///  Will be set by the OS driver during OSPM (D3 flow)
  ///  Enables setting of the *_PME_STS bit to generate a wake event and/or an SCI or SMI#
  ///
  IoAnd32 (
    GeneralConfig->AcpiBase + R_ACPI_GPE0a_EN,
    (UINT32)(~(B_ACPI_GPE0a_EN_XHCI_PME_EN |
               B_ACPI_GPE0a_EN_PCIE_GPE_EN |
               B_ACPI_GPE0a_EN_AVS_PME_EN  |
               B_ACPI_GPE0a_EN_XDCI_PME_EN |
               B_ACPI_GPE0a_EN_CSE_PME_EN))
    );

  PostCode (0xB7F);  // SC PostMem Exit // PEI_FRC_SC_INIT_END 
  DEBUG ((DEBUG_INFO, "ScInit - End\n"));
}

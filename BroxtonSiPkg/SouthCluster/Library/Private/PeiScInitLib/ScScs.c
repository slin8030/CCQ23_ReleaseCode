/** @file
  Initializes SC SCC Devices

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
#include <ScRegs/RegsPsf.h>
#include "SaAccess.h"
#include <Library/SteppingLib.h>

typedef struct {
  UINT32 PciDeviceNum;
  UINT32 PciFuncNum;
  UINT16 ScsPciCfgCtrl;
  UINT16 Psf3BaseAddress;
  UINT32 ScsFuncDisableBit;
  UINT32 ScsHostFuncDisableBit;
} SCS_CONTROLLER_DESCRIPTOR;

#define SCS_DEVICE_NUM  4

//
// Note: the devices order need to align with the order defined in SC_SCS_CONFIG
//
SCS_CONTROLLER_DESCRIPTOR mScsIoDevices[SCS_DEVICE_NUM] =
{
  //
  // SdCard
  //
  {PCI_DEVICE_NUMBER_SCC_SDCARD, PCI_FUNCTION_NUMBER_SCC_FUNC0, R_SCC_SB_PCICFGCTRL_SDCARD, R_PCH_PCR_PSF3_T0_SHDW_SCS_SDCARD_REG_BASE, B_PMC_FUNC_DIS_SCC_SDCARD, B_SCC_HOST_FUNC_DIS_SDCARD},
  //
  // eMMC
  //
  {PCI_DEVICE_NUMBER_SCC_EMMC, PCI_FUNCTION_NUMBER_SCC_FUNC0, R_SCC_SB_PCICFGCTRL_EMMC, R_PCH_PCR_PSF3_T0_SHDW_SCS_EMMC_REG_BASE, B_PMC_FUNC_DIS_SCC_EMMC, B_SCC_HOST_FUNC_DIS_EMMC},
  //
  // UFS
  //
  {PCI_DEVICE_NUMBER_SCC_UFS, PCI_FUNCTION_NUMBER_SCC_FUNC0, R_SCC_SB_PCICFGCTRL_UFS, R_PCH_PCR_PSF3_T0_SHDW_SCS_UFS_REG_BASE, B_PMC_FUNC_DIS_SCC_UFS,B_SCC_HOST_FUNC_DIS_UFS},
  //
  // SDIO
  //
  {PCI_DEVICE_NUMBER_SCC_SDIO, PCI_FUNCTION_NUMBER_SCC_FUNC0, R_SCC_SB_PCICFGCTRL_SDIO, R_PCH_PCR_PSF3_T0_SHDW_SCS_SDIO_REG_BASE, B_PMC_FUNC_DIS_SCC_SDIO, B_SCC_HOST_FUNC_DIS_SDIO},
};


/**
  Configure Storage and Communication Subsystems Controller SDIO
  default DLL registers for Ax stepping silicon.

  @param[in] TempMemBaseAddr          Temporary Memory Base Address for PCI
                                      devices to be used to initialize MMIO
                                      registers.

  @retval None
**/
VOID
ConfigureSdioDll (
  IN SC_SCS_CONFIG            *ScsConfig,
  IN UINT32                   TempMemBaseAddr
  )
{
  UINTN         SdioMmioBaseAddress;
  SdioMmioBaseAddress = TempMemBaseAddr;

  //
  // Configure DLL settings
  //
  MmioWrite32 (SdioMmioBaseAddress + R_SCC_MEM_TX_CMD_DLL_CNTL, ScsConfig->SdioRegDllConfig.TxCmdCntl);
  MmioWrite32 (SdioMmioBaseAddress + R_SCC_MEM_TX_DATA_DLL_CNTL1, ScsConfig->SdioRegDllConfig.TxDataCntl1);
  MmioWrite32 (SdioMmioBaseAddress + R_SCC_MEM_TX_DATA_DLL_CNTL2, ScsConfig->SdioRegDllConfig.TxDataCntl2);
  MmioWrite32 (SdioMmioBaseAddress + R_SCC_MEM_RX_CMD_DATA_DLL_CNTL1, ScsConfig->SdioRegDllConfig.RxCmdDataCntl1);
  MmioWrite32 (SdioMmioBaseAddress + R_SCC_MEM_RX_CMD_DATA_DLL_CNTL2, ScsConfig->SdioRegDllConfig.RxCmdDataCntl2);

  return;
}

VOID
ConfigureSdcardCapability(
  IN UINT32  TempMemBaseAddr
  )
{

  UINTN SdcardMmioBaseAddress;

  SdcardMmioBaseAddress = TempMemBaseAddr;

  DEBUG ((DEBUG_INFO, "SDCard Capabilities Register [31:0]  (40h) = 0x%08x\n", MmioRead32(SdcardMmioBaseAddress + R_SCC_MEM_CAP1)));
  DEBUG ((DEBUG_INFO, "SDCard Capabilities Register [63:32] (44h) = 0x%08x\n", MmioRead32(SdcardMmioBaseAddress + R_SCC_MEM_CAP2)));

  //
  // Enable Capabilities Bypass
  //
  MmioWrite32 (SdcardMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_CNTL, (UINT32)V_SCC_MEM_CAP_BYPASS_CNTL_EN);

  //
  // Set Capability Bypass Values
  //
  MmioWrite32 (SdcardMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_REG1, 0x1051EF5C);
  MmioWrite32 (SdcardMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_REG2, 0x040000C8);

  DEBUG ((DEBUG_INFO, "SDCard Capabilities Bypass Control    (810h) = 0x%08x\n", MmioRead32(SdcardMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_CNTL)));
  DEBUG ((DEBUG_INFO, "SDCard Capabilities Bypass Register 1 (814h) = 0x%08x\n", MmioRead32(SdcardMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_REG1)));
  DEBUG ((DEBUG_INFO, "SDCard Capabilities Bypass Register 2 (818h) = 0x%08x\n", MmioRead32(SdcardMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_REG2)));
  DEBUG ((DEBUG_INFO, "SDCard Capabilities Register [31:0]  (40h) = 0x%08x\n", MmioRead32(SdcardMmioBaseAddress + R_SCC_MEM_CAP1)));
  DEBUG ((DEBUG_INFO, "SDCard Capabilities Register [63:32] (44h) = 0x%08x\n", MmioRead32(SdcardMmioBaseAddress + R_SCC_MEM_CAP2)));
}

VOID
ConfigureSdioCapability(
  IN UINT32                   TempMemBaseAddr
  )
{

  UINTN         SdioMmioBaseAddress;
  UINT32        Data32;

  SdioMmioBaseAddress = TempMemBaseAddr;


  DEBUG ((DEBUG_INFO, "SDIO Capabilities Register [31:0]  (40h) = 0x%08x\n", MmioRead32(SdioMmioBaseAddress + R_SCC_MEM_CAP1)));
  DEBUG ((DEBUG_INFO, "SDIO Capabilities Register [63:32] (44h) = 0x%08x\n", MmioRead32(SdioMmioBaseAddress + R_SCC_MEM_CAP2)));
  //
  // Enable Capabilities Bypass
  //
  MmioWrite32 (SdioMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_CNTL, (UINT32)V_SCC_MEM_CAP_BYPASS_CNTL_EN);
  //
  // CAP_BYPASS_REGx_DEFAULTS value keeps HW/Reset defaults of Capabilities Register (SCC MMIO, offset 40h)
  //
  Data32 = V_SCC_MEM_CAP_BYPASS_REG1_DEFAULTS | B_SCC_MEM_CAP_BYPASS_REG1_ASYNC_INTERRUPT;
  Data32 &= ~(B_SCC_MEM_CAP_BYPASS_REG1_HS400);

  //
  // Set Timer Count for Re-Tuning [20:17] to 1000b for SDIO controller
  //
  Data32 &= ~(B_SCC_MEM_CAP_BYPASS_REG1_TIMER_COUNT);
  Data32 |= (V_SCC_MEM_CAP_BYPASS_REG1_TIMER_COUNT << N_SCC_MEM_CAP_BYPASS_REG1_TIMER_COUNT);

  //
  // Set 3.3 V Support for SDIO controller
  //
  Data32 |= B_SCC_MEM_CAP_BYPASS_REG1_3P3V_SUPPORT;
  MmioWrite32 (SdioMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_REG1, Data32);

  Data32 = V_SCC_MEM_CAP_BYPASS_REG2_DEFAULTS;
  Data32 &= ~(B_SCC_MEM_CAP_BYPASS_REG2_8BIT_SUPPORT);
  MmioWrite32 (SdioMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_REG2, Data32);

  DEBUG ((DEBUG_INFO, "SDIO Capabilities Bypass Control    (810h) = 0x%08x\n", MmioRead32(SdioMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_CNTL)));
  DEBUG ((DEBUG_INFO, "SDIO Capabilities Bypass Register 1 (814h) = 0x%08x\n", MmioRead32(SdioMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_REG1)));
  DEBUG ((DEBUG_INFO, "SDIO Capabilities Bypass Register 2 (818h) = 0x%08x\n", MmioRead32(SdioMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_REG2)));
  DEBUG ((DEBUG_INFO, "SDIO Capabilities Register [31:0]  (40h) = 0x%08x\n", MmioRead32(SdioMmioBaseAddress + R_SCC_MEM_CAP1)));
  DEBUG ((DEBUG_INFO, "SDIO Capabilities Register [63:32] (44h) = 0x%08x\n", MmioRead32(SdioMmioBaseAddress + R_SCC_MEM_CAP2)));
  return;

}


/**
  Configure Storage and Communication Subsystems Controller SDCARD
  default DLL registers for Ax stepping silicon.

  @param[in] TempMemBaseAddr          Temporary Memory Base Address for PCI
                                      devices to be used to initialize MMIO
                                      registers.

  @retval None
**/
VOID
ConfigureSdcardDll (
  IN SC_SCS_CONFIG            *ScsConfig,
  IN UINT32                   TempMemBaseAddr
  )
{

  UINTN         SdcardMmioBaseAddress;
  SdcardMmioBaseAddress = TempMemBaseAddr;

  //
  // Configure DLL settings
  //
  MmioWrite32 (SdcardMmioBaseAddress + R_SCC_MEM_TX_CMD_DLL_CNTL, ScsConfig->SdcardRegDllConfig.TxCmdCntl);
  MmioWrite32 (SdcardMmioBaseAddress + R_SCC_MEM_TX_DATA_DLL_CNTL1, ScsConfig->SdcardRegDllConfig.TxDataCntl1);
  MmioWrite32 (SdcardMmioBaseAddress + R_SCC_MEM_TX_DATA_DLL_CNTL2, ScsConfig->SdcardRegDllConfig.TxDataCntl2);
  MmioWrite32 (SdcardMmioBaseAddress + R_SCC_MEM_RX_CMD_DATA_DLL_CNTL1, ScsConfig->SdcardRegDllConfig.RxCmdDataCntl1);
  MmioWrite32 (SdcardMmioBaseAddress + R_SCC_MEM_RX_STROBE_DLL_CNTL, ScsConfig->SdcardRegDllConfig.RxStrobeCntl);
  MmioWrite32 (SdcardMmioBaseAddress + R_SCC_MEM_RX_CMD_DATA_DLL_CNTL2, ScsConfig->SdcardRegDllConfig.RxCmdDataCntl2);

  return;
}

/**
  Configure Storage and Communication Subsystems Controller eMMC
  default DLL registers for Ax stepping silicon.

  @param[in] ScsConfig                The SCS Config Block instance
  @param[in] TempMemBaseAddr          Temporary Memory Base Address for PCI
                                      devices to be used to initialize MMIO
                                      registers.

  @retval None
**/
VOID
ConfigureEmmcDll (
  IN SC_SCS_CONFIG            *ScsConfig,
  IN UINT32                   TempMemBaseAddr
  )
{
  UINTN                     EmmcMmioBaseAddress;

  EmmcMmioBaseAddress = TempMemBaseAddr;

  // Configure DLL settings
  MmioWrite32 (EmmcMmioBaseAddress + R_SCC_MEM_TX_CMD_DLL_CNTL, ScsConfig->EmmcRegDllConfig.TxCmdCntl);
  MmioWrite32 (EmmcMmioBaseAddress + R_SCC_MEM_TX_DATA_DLL_CNTL1, ScsConfig->EmmcRegDllConfig.TxDataCntl1);
  MmioWrite32 (EmmcMmioBaseAddress + R_SCC_MEM_TX_DATA_DLL_CNTL2, ScsConfig->EmmcRegDllConfig.TxDataCntl2);
  MmioWrite32 (EmmcMmioBaseAddress + R_SCC_MEM_RX_CMD_DATA_DLL_CNTL1, ScsConfig->EmmcRegDllConfig.RxCmdDataCntl1);
  MmioWrite32 (EmmcMmioBaseAddress + R_SCC_MEM_RX_STROBE_DLL_CNTL, ScsConfig->EmmcRegDllConfig.RxStrobeCntl);
  MmioWrite32 (EmmcMmioBaseAddress + R_SCC_MEM_RX_CMD_DATA_DLL_CNTL2, ScsConfig->EmmcRegDllConfig.RxCmdDataCntl2);
  MmioWrite32 (EmmcMmioBaseAddress + R_SCC_MEM_MASTER_DLL_SW_CNTL, ScsConfig->EmmcRegDllConfig.MasterSwCntl);

  return;

}
/**
  Configure SKL PCH eMMC Clock Source for Rx Path

  @param[in] TempMemBaseAddr          Temporary Memory Base Address for PCI
                                      devices to be used to initialize MMIO
                                      registers.

  @retval None
**/
VOID
ConfigureEmmcRxClkSrc (
  IN UINT32                   TempMemBaseAddr
  )
{
  return;
}
/**
  ConfigureEmmcCapability
  @param[in] ScsConfig            The SCS Config Block instance
  @param[in] TempMemBaseAddr      Temporary Memory Base Address for PCI
                                  devices to be used to initialize MMIO
                                  registers.
  @retval None
**/
VOID
ConfigureEmmcCapability(
  IN SC_SCS_CONFIG *ScsConfig,
  IN UINT32        TempMemBaseAddr
  )
{

  UINTN         EmmcMmioBaseAddress;
  UINT32        Data32;

  EmmcMmioBaseAddress = TempMemBaseAddr;


  DEBUG ((DEBUG_INFO, "Emmc Capabilities Register [31:0]  (40h) = 0x%08x\n", MmioRead32(EmmcMmioBaseAddress + R_SCC_MEM_CAP1)));
  DEBUG ((DEBUG_INFO, "Emmc Capabilities Register [63:32] (44h) = 0x%08x\n", MmioRead32(EmmcMmioBaseAddress + R_SCC_MEM_CAP2)));
  //
  // Enable Capabilities Bypass
  //
  MmioWrite32 (EmmcMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_CNTL, (UINT32)V_SCC_MEM_CAP_BYPASS_CNTL_EN);
  //
  // CAP_BYPASS_REGx_DEFAULTS value keeps HW/Reset defaults of Capabilities Register (SCC MMIO, offset 40h)
  //
  Data32 = V_SCC_MEM_CAP_BYPASS_REG1_DEFAULTS;

  //
  // Set Timer Count for Re-Tuning [20:17] to 1000b for eMMC controller
  //
  Data32 &= ~(B_SCC_MEM_CAP_BYPASS_REG1_TIMER_COUNT);
  Data32 |= (V_SCC_MEM_CAP_BYPASS_REG1_TIMER_COUNT << N_SCC_MEM_CAP_BYPASS_REG1_TIMER_COUNT);

  //
  // Set timeoutclkfreq to 1MHz
  //
  Data32 &= ~(B_SCC_MEM_CAP_BYPASS_REG1_TIMEOUT_CLK_COUNT);
  Data32 |= (V_SCC_MEM_CAP_BYPASS_REG1_TIMEOUT_CLK_COUNT << N_SCC_MEM_CAP_BYPASS_REG1_TIMEOUT_CLK_COUNT);

  //
  // Set EMMC Mode according to Policy
  //
  if (ScsConfig->EmmcHostMaxSpeed == SccEmmcHs400) {
    Data32 |= (B_SCC_MEM_CAP_BYPASS_REG1_HS400 + B_SCC_MEM_CAP_BYPASS_REG1_SDR104);
  } else if (ScsConfig->EmmcHostMaxSpeed == SccEmmcHs200) {
    Data32 &= ~(B_SCC_MEM_CAP_BYPASS_REG1_HS400);
    Data32 |= B_SCC_MEM_CAP_BYPASS_REG1_SDR104;
  } else if (ScsConfig->EmmcHostMaxSpeed == SccEmmcDdr50) {
    Data32 &= ~(B_SCC_MEM_CAP_BYPASS_REG1_HS400 + B_SCC_MEM_CAP_BYPASS_REG1_SDR104);
  }

  MmioWrite32 (EmmcMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_REG1, Data32);
  MmioWrite32 (EmmcMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_REG2, V_SCC_MEM_CAP_BYPASS_REG2_DEFAULTS);

  DEBUG ((DEBUG_INFO, "Emmc Capabilities Bypass Control    (810h) = 0x%08x\n", MmioRead32(EmmcMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_CNTL)));
  DEBUG ((DEBUG_INFO, "Emmc Capabilities Bypass Register 1 (814h) = 0x%08x\n", MmioRead32(EmmcMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_REG1)));
  DEBUG ((DEBUG_INFO, "Emmc Capabilities Bypass Register 2 (818h) = 0x%08x\n", MmioRead32(EmmcMmioBaseAddress + R_SCC_MEM_CAP_BYPASS_REG2)));
  DEBUG ((DEBUG_INFO, "Emmc Capabilities Register [31:0]  (40h) = 0x%08x\n", MmioRead32(EmmcMmioBaseAddress + R_SCC_MEM_CAP1)));
  DEBUG ((DEBUG_INFO, "Emmc Capabilities Register [63:32] (44h) = 0x%08x\n", MmioRead32(EmmcMmioBaseAddress + R_SCC_MEM_CAP2)));
  return;

}


/**
  Configure LPSS IO devices per DevIndex
  @param[in] SiPolicy             The Silicon Policy PPI instance
  @param[in] ScsConfig            The SCS Config Block instance
  @param[in, out] FuncDisableReg  The value of Function disable register
  @param[in, out] FuncDisableReg1 The value of Function disable 1 register
  @param[in] DevIndex             The index to access all devices under LPSS

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureScsIoDevices (
  IN SI_POLICY_PPI *SiPolicy,
  IN SC_SCS_CONFIG *ScsConfig,
  IN OUT UINT32    *FuncDisableReg,
  IN OUT UINT32    *FuncDisableReg1,
  IN UINT32        DevIndex
  )
{
  UINTN                 SccPciMmBase;
  EFI_PHYSICAL_ADDRESS  SccMmioBase0;
  UINT32                Value;
  SC_DEV_MODE           *ScDevMode;

  SccPciMmBase = 0;
  SccMmioBase0 = (EFI_PHYSICAL_ADDRESS) SiPolicy->TempMemBaseAddr;;
  Value        = 0;

  DEBUG ((DEBUG_INFO, "ConfigureSccIoDevices SccMmioBase = 0x%x.\n", SccMmioBase0));
  //
  // Pointer to the first item of SCC config list
  //
  ScDevMode = (SC_DEV_MODE*)((UINT8*)ScsConfig + sizeof(CONFIG_BLOCK_HEADER));
  SccPciMmBase = MmPciBase (
                    DEFAULT_PCI_BUS_NUMBER_SC,
                    mScsIoDevices[DevIndex].PciDeviceNum,
                    mScsIoDevices[DevIndex].PciFuncNum
                    );
  //
  // Check if device present
  //
  if (MmioRead32 (SccPciMmBase) == 0xFFFFFFFF) {
    DEBUG ((DEBUG_ERROR, "SCC IO device (%d) not present, skipping.\n", DevIndex));

    ScDevMode[DevIndex] = ScDisabled;
    *FuncDisableReg |= mScsIoDevices[DevIndex].ScsFuncDisableBit;
    if (mScsIoDevices[DevIndex].PciDeviceNum == PCI_DEVICE_NUMBER_SCC_UFS) {
      *FuncDisableReg1 |= B_PMC_FUNC_DIS_1_USF2;
    }
  } else {
    //
    // Enable PCI Power Gating bits for every SCC PCI devices.
    // Enable Power Gating of SCC devices, bit17 & bit18 set to 1 for all PCI slices
    //
    MmioOr32 ((UINTN) (SccPciMmBase + R_SCC_D0I3MAXDEVPG), BIT17| BIT18);

    //
    // Disable Bus Master Enable & Memory Space Enable
    //
    MmioAnd32 ((UINTN) (SccPciMmBase + R_SCC_STSCMD), (UINT32) ~(B_SCC_STSCMD_BME | B_SCC_STSCMD_MSE));
    //
    // Program BAR 0
    //
    ASSERT (((SccMmioBase0 & B_SCC_BAR_BA) == SccMmioBase0) && (SccMmioBase0 != 0));
    MmioWrite32 ((UINTN) (SccPciMmBase + R_SCC_BAR), (UINT32) (SccMmioBase0 & B_SCC_BAR_BA));
    //
    // Bus Master Enable & Memory Space Enable
    //
    MmioOr32 ((UINTN) (SccPciMmBase + R_SCC_STSCMD), (UINT32) (B_SCC_STSCMD_BME | B_SCC_STSCMD_MSE));

    //
    // Special handling for SCC Devices
    //
    if (mScsIoDevices[DevIndex].PciDeviceNum == PCI_DEVICE_NUMBER_SCC_UFS) {
      //
      // Additional Bit21 for UFS only
      //
      MmioOr32 ((UINTN) (SccPciMmBase + R_SCC_D0I3MAXDEVPG), BIT21);
      MmioAnd32 ((UINTN) (SccMmioBase0 + R_SCC_MEM_CUR_XFSM), 0xfffff000);
      DEBUG ((DEBUG_INFO, "UFS R_SCC_D0I3MAXDEVPG.\n", MmioRead32 ((UINTN) (SccPciMmBase + R_SCC_D0I3MAXDEVPG))));
    }
    /// >> Handle MMIO programming using temp BAR0
    //
    // Disable BAR1 when device is working in PCI mode.
    //
    if (ScDevMode[DevIndex] == ScDisabled) {
      //
      // Put device in D3Hot state before disabling it.
      //
      DEBUG ((DEBUG_INFO, "Putting SCC IO device (%d) into D3 Hot State.\n", DevIndex));
      //
      // Do HEC reset for UFS for D3 working
      //
      if (mScsIoDevices[DevIndex].PciDeviceNum == PCI_DEVICE_NUMBER_SCC_UFS) {
        //
        //if UFS HCS.DP not set
        //
        if ((MmioRead32 ((UINTN)(SccMmioBase0 + R_SCC_UFS_HC_STATUS)) & B_SCC_UFS_HC_HCS_DP) == 0) {
          DEBUG ((DEBUG_INFO, "UFS R_SCC_UFS_HC_STATUS Value = 0x%x.\n", MmioRead32 ((UINTN) (SccMmioBase0 + R_SCC_UFS_HC_STATUS))));
          //
          // Write a 1 to the HCE register to enable the UFS host controller.
          //
          MmioWrite32 ((UINTN)(SccMmioBase0 + R_SCC_UFS_HC_ENABLE), B_SCC_UFS_HC_HCE_EN);
          MicroSecondDelay (100);
        }

        //
        //Set the Auto-Hibernate Idle Timer to 15ms
        //
        MmioOr32 ((UINTN) (SccMmioBase0 + R_SCC_UFS_HC_AHIT), V_SCC_UFS_HC_TIMER_SCALE_MS | V_SCC_UFS_HC_AH8ITV_15);
        DEBUG ((DEBUG_INFO, "UFS R_SCC_UFS_HC_AHIT Value = 0x%x.\n", MmioRead32 ((UINTN) (SccMmioBase0 + R_SCC_UFS_HC_AHIT))));

        //
        //
        MmioOr32 ((UINTN) (SccMmioBase0 + R_SCC_UFS_MEM_BUSTHRTL), B_SCC_UFS_LP_AH8_PGE | B_SCC_UFS_LP_PGE);
      }
      ///
      /// 1)  The Restore Required (bit 3 of the Convergence Layer Offset DevIdle_Control) must be cleared by BIOs by writing a '1' to it.
      ///
      MmioOr32 ((UINTN) (SccMmioBase0 + R_SCC_MEM_IDLE_CTRL), BIT3);
      ///
      /// 2)  The respective Slice DevIdle register bit (Convergence Layer Offset DevIdle_Control) must be set (DevIdleC, bit 2 = '1')
      ///
      MmioOr32 ((UINTN) (SccMmioBase0 + R_SCC_MEM_IDLE_CTRL), BIT2);

      ///
      /// 4)  Place Device into the D3HOT State by setting 0x084 PMECTRLSTATUS, bits 1:0 (POWERSTATE) to 2'b11.
      ///
      MmioOr32 ((UINTN) (SccPciMmBase + R_SCC_PCS), B_SCC_PCS_PS);

      //
      //[HSDES][1206524110]IOSF2OCP oob_bme and oob_in_d3 indications are not reliable - SCS, need to read back before write.
      //
      MmioRead32((UINTN)(SccPciMmBase + R_SCC_PCS));

    } else if (ScDevMode[DevIndex] == ScPciMode) {
       //
       // Disable Bar1
       // Disable Bar1 in PSF
       //
       SideBandAndThenOr32(
         SCC_EP_PORT_ID,
         mScsIoDevices[DevIndex].ScsPciCfgCtrl,
         0xFFFFFFFF,
         B_SCC_PCICFGCTRL_BAR1_DIS
         );


       Value = SideBandRead32(SCC_EP_PORT_ID, mScsIoDevices[DevIndex].ScsPciCfgCtrl);
       DEBUG ((DEBUG_INFO, "Read back ScsPciCfgCtrl Value = %x, Index = %d\n", Value, DevIndex));

       SideBandAndThenOr32(
         SB_PORTID_PSF3,
         mScsIoDevices[DevIndex].Psf3BaseAddress + R_PCH_PCR_PSFX_T0_SHDW_PCIEN,
         0xFFFFFFFF,
         B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR1DIS
         );

       Value = SideBandRead32(
         SB_PORTID_PSF3,
         mScsIoDevices[DevIndex].Psf3BaseAddress + R_PCH_PCR_PSFX_T0_SHDW_PCIEN
         );
       DEBUG ((DEBUG_INFO, "Read back Psf3BaseAddress Value = %x, Index = %d\n", Value, DevIndex));

       ///
       /// Note: From HAS, BAR0 need to program into Remap register as well (REMAP_ADDRESS_LOW/HIGH (+SPT))
       /// This need to be done after PCI scan, all BARs are allocated after the scan.
       ///
    }

    if ((mScsIoDevices[DevIndex].PciDeviceNum == PCI_DEVICE_NUMBER_SCC_EMMC) && ((ScDevMode[DevIndex] != ScDisabled))) {
      ConfigureEmmcCapability (ScsConfig, (UINT32)(UINTN)SccMmioBase0);
      ConfigureEmmcDll (ScsConfig, (UINT32)(UINTN)SccMmioBase0);
      ConfigureEmmcRxClkSrc ((UINT32)(UINTN)SccMmioBase0);
    }

    if((mScsIoDevices[DevIndex].PciDeviceNum == PCI_DEVICE_NUMBER_SCC_SDCARD) && ((ScDevMode[DevIndex] != ScDisabled))) {
      ConfigureSdcardCapability ((UINT32)(UINTN)SccMmioBase0);
      ConfigureSdcardDll (ScsConfig, (UINT32)(UINTN)SccMmioBase0);
    }

    if((mScsIoDevices[DevIndex].PciDeviceNum == PCI_DEVICE_NUMBER_SCC_SDIO) && ((ScDevMode[DevIndex] != ScDisabled))){
      ConfigureSdioCapability ((UINT32)(UINTN)SccMmioBase0);
      ConfigureSdioDll (ScsConfig, (UINT32)(UINTN)SccMmioBase0);
    }

    //
    // Handle MMIO programming using temp BAR0
    //

    //
    // Disable Bus Master Enable & Memory Space Enable
    //
    MmioAnd32 ((UINTN) (SccPciMmBase + R_SCC_STSCMD), (UINT32) ~(B_SCC_STSCMD_BME | B_SCC_STSCMD_MSE));
    //
    // Clear BAR0
    //
    MmioWrite32 ((UINTN) (SccPciMmBase + R_SCC_BAR), (UINT32) (0x00));
  }
  //
  // Since the setting below will disable PCI device, so need to do after PCI config space access operation
  //
  if (ScDevMode[DevIndex] == ScDisabled) {
    ///
    /// 5) Disable device's PciCfg
    ///
    SideBandAndThenOr32(
      SCC_EP_PORT_ID,
      mScsIoDevices[DevIndex].ScsPciCfgCtrl,
      0xFFFFFFFF,
      B_SCC_PCICFGCTRL_PCI_CFG_DIS
      );
    ///
    /// 6) Disable device's PciCfg in PSF
    ///
    SideBandAndThenOr32(
      SB_PORTID_PSF3,
      mScsIoDevices[DevIndex].Psf3BaseAddress + R_PCH_PCR_PSFX_T0_SHDW_CFG_DIS,
      0xFFFFFFFF,
      B_PCH_PCR_PSFX_T0_SHDW_CFG_DIS_CFGDIS
      );
    ///
    /// 7) Function Disable in PSF
    ///
    SideBandAndThenOr32(
      SB_PORTID_PSF3,
      mScsIoDevices[DevIndex].Psf3BaseAddress + R_PCH_PCR_PSFX_T0_SHDW_PCIEN,
      0xFFFFFFFF,
      B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR1DIS | B_PCH_PCR_PSFX_T0_SHDW_PCIEN_FUNDIS
      );
    ///
    /// 8) Disable the host controller
    ///
   SideBandAndThenOr32(
     SCC_EP_PORT_ID,
     R_SCC_SB_GPPRVRW2,
     0xFFFFFFFF,
     mScsIoDevices[DevIndex].ScsHostFuncDisableBit
     );

    *FuncDisableReg |= mScsIoDevices[DevIndex].ScsFuncDisableBit;
  }

  return EFI_SUCCESS;
}

/**
  Configure SCS devices.

  @param[in] SiPolicy             The Silicon Policy PPI instance
  @param[in] ScPolicy             The SC Policy PPI instance
  @param[in, out] FuncDisableReg  The value of Function disable register
  @param[in, out] FuncDisableReg1 The value of Function disable 1 register

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureScs (
  IN SI_POLICY_PPI *SiPolicy,
  IN SC_POLICY_PPI *ScPolicy,
  IN OUT UINT32    *FuncDisableReg,
  IN OUT UINT32    *FuncDisableReg1
  )
{
  EFI_STATUS    Status;
  UINT32        DevIndex;
  SC_SCS_CONFIG *ScsConfig;

  DEBUG ((DEBUG_INFO, "ConfigureScs() Start\n"));
  DevIndex = 0;
  Status = GetConfigBlock ((VOID *) ScPolicy, &gScsConfigGuid, (VOID *) &ScsConfig);
  ASSERT_EFI_ERROR (Status);

#ifdef SC_PM_ENABLE
if ((PLATFORM_ID == VALUE_REAL_PLATFORM) ||
    (PLATFORM_ID == VALUE_SLE_UPF)

    ) {
  DEBUG ((DEBUG_INFO, "Start SC_PM_ENABLE\n"));
  ///
  /// Step 1: Write to General Purpose register at offset 0x600 with data 0xdffef
  ///
  SideBandAndThenOr32(
    SCC_EP_PORT_ID,
    R_SCC_SB_GPPRVRW1,
    0xFFFFFFFF,
    0x4DFFEF
    );
  ///
  /// Step 2: Program SCS PMCTL
  ///
  SideBandAndThenOr32(
    SCC_EP_PORT_ID,
    R_SCC_SB_PMCTL,
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
  ///
  /// Step 3: Enable SCS Power Gating in BIOS
  ///  Need writes to 0x1D0 which is PMCTL register with data: 0x3f
  ///  Need write to General Purpose register at offset 0x600 with data 0xdffef
  ///  D0I3_MAX_POW_LAT_PG_CONFIG@0xA2 with 0x2f  ' or in other words; bit 19:16 and bit 21 of offset 0xA0. (Done in ConfigureSccIoDevices())
  ///
  for(DevIndex = 0; DevIndex < SCS_DEVICE_NUM; DevIndex++)
  {
    DEBUG ((DEBUG_INFO, "Call ConfigureScsIoDevices() with Index = %d\n", DevIndex));
    Status = ConfigureScsIoDevices(SiPolicy, ScsConfig, FuncDisableReg, FuncDisableReg1, DevIndex);
  }
  DEBUG ((DEBUG_INFO, "ConfigureScs() End\n"));

  return EFI_SUCCESS;
}

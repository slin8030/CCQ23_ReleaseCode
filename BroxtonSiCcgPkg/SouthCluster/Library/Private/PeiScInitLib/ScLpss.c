/** @file
  Initializes SC LPSS Devices

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
#include <Library/PciLib.h>
#include <Library/SteppingLib.h>
#include <Library/HeciMsgLib.h>

typedef struct {
  UINT32 PciDeviceNum;
  UINT32 PciFuncNum;
  UINT16 LpssPciCfgCtrl;
  UINT16 Psf3BaseAddress;
  UINT32 LpssFuncDisableBit;
} SERIAL_IO_CONTROLLER_DESCRIPTOR;

#define LPSS_IO_DEVICE_NUM  15

//
// Note: the devices order need to align with the order defined in SC_LPSS_CONFIG
//
GLOBAL_REMOVE_IF_UNREFERENCED SERIAL_IO_CONTROLLER_DESCRIPTOR mLpssIoDevices[LPSS_IO_DEVICE_NUM] =
{
  //
  // I2C 0~7
  //
  {PCI_DEVICE_NUMBER_LPSS_I2C0, PCI_FUNCTION_NUMBER_LPSS_I2C0, R_LPSS_SB_PCICFGCTRL_I2C0, R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C0_REG_BASE, B_PMC_FUNC_DIS_LPSS_I2C0},
  {PCI_DEVICE_NUMBER_LPSS_I2C0, PCI_FUNCTION_NUMBER_LPSS_I2C1, R_LPSS_SB_PCICFGCTRL_I2C1, R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C1_REG_BASE, B_PMC_FUNC_DIS_LPSS_I2C1},
  {PCI_DEVICE_NUMBER_LPSS_I2C0, PCI_FUNCTION_NUMBER_LPSS_I2C2, R_LPSS_SB_PCICFGCTRL_I2C2, R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C2_REG_BASE, B_PMC_FUNC_DIS_LPSS_I2C2},
  {PCI_DEVICE_NUMBER_LPSS_I2C0, PCI_FUNCTION_NUMBER_LPSS_I2C3, R_LPSS_SB_PCICFGCTRL_I2C3, R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C3_REG_BASE, B_PMC_FUNC_DIS_LPSS_I2C3},
  {PCI_DEVICE_NUMBER_LPSS_I2C1, PCI_FUNCTION_NUMBER_LPSS_I2C4, R_LPSS_SB_PCICFGCTRL_I2C4, R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C4_REG_BASE, B_PMC_FUNC_DIS_LPSS_I2C4},
  {PCI_DEVICE_NUMBER_LPSS_I2C1, PCI_FUNCTION_NUMBER_LPSS_I2C5, R_LPSS_SB_PCICFGCTRL_I2C5, R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C5_REG_BASE, B_PMC_FUNC_DIS_LPSS_I2C5},
  {PCI_DEVICE_NUMBER_LPSS_I2C1, PCI_FUNCTION_NUMBER_LPSS_I2C6, R_LPSS_SB_PCICFGCTRL_I2C6, R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C6_REG_BASE, B_PMC_FUNC_DIS_LPSS_I2C6},
  {PCI_DEVICE_NUMBER_LPSS_I2C1, PCI_FUNCTION_NUMBER_LPSS_I2C7, R_LPSS_SB_PCICFGCTRL_I2C7, R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C7_REG_BASE, B_PMC_FUNC_DIS_LPSS_I2C7},
  //
  // UART 0~3
  //
  {PCI_DEVICE_NUMBER_LPSS_HSUART, PCI_FUNCTION_NUMBER_LPSS_HSUART0, R_LPSS_SB_PCICFGCTRL_UART0, R_PCH_PCR_PSF3_T0_SHDW_LPSS_UART0_REG_BASE, B_PMC_FUNC_DIS_LPSS_UART0},
  {PCI_DEVICE_NUMBER_LPSS_HSUART, PCI_FUNCTION_NUMBER_LPSS_HSUART1, R_LPSS_SB_PCICFGCTRL_UART1, R_PCH_PCR_PSF3_T0_SHDW_LPSS_UART1_REG_BASE, B_PMC_FUNC_DIS_LPSS_UART1},
  {PCI_DEVICE_NUMBER_LPSS_HSUART, PCI_FUNCTION_NUMBER_LPSS_HSUART2, R_LPSS_SB_PCICFGCTRL_UART2, R_PCH_PCR_PSF3_T0_SHDW_LPSS_UART2_REG_BASE, B_PMC_FUNC_DIS_LPSS_UART2},
  {PCI_DEVICE_NUMBER_LPSS_HSUART, PCI_FUNCTION_NUMBER_LPSS_HSUART3, R_LPSS_SB_PCICFGCTRL_UART3, R_PCH_PCR_PSF3_T0_SHDW_LPSS_UART3_REG_BASE, B_PMC_FUNC_DIS_LPSS_UART3},
  //
  // SPI 0~2
  //
  {PCI_DEVICE_NUMBER_LPSS_SPI, PCI_FUNCTION_NUMBER_LPSS_SPI0, R_LPSS_SB_PCICFGCTRL_SPI0, R_PCH_PCR_PSF3_T0_SHDW_LPSS_SPI0_REG_BASE, B_PMC_FUNC_DIS_LPSS_SPI0},
  {PCI_DEVICE_NUMBER_LPSS_SPI, PCI_FUNCTION_NUMBER_LPSS_SPI1, R_LPSS_SB_PCICFGCTRL_SPI1, R_PCH_PCR_PSF3_T0_SHDW_LPSS_SPI1_REG_BASE, B_PMC_FUNC_DIS_LPSS_SPI1},
  {PCI_DEVICE_NUMBER_LPSS_SPI, PCI_FUNCTION_NUMBER_LPSS_SPI2, R_LPSS_SB_PCICFGCTRL_SPI2, R_PCH_PCR_PSF3_T0_SHDW_LPSS_SPI2_REG_BASE, B_PMC_FUNC_DIS_LPSS_SPI2}
};


/**
  Configure UART2 For Kernel Debug Usage

  @param[in] LpssConfig           The LPSS Config Block instance
  @param[in] Uart2PciMmBase       The UART2 PCI Base Address
  @param[in] Uart2MmioBase0       The UART2 MMIO Base Address

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureUart2KernelDebug (
  IN SC_LPSS_CONFIG        *LpssConfig,
  IN UINTN                 Uart2PciMmBase,
  IN EFI_PHYSICAL_ADDRESS  Uart2MmioBase0
  )
{
  EFI_BOOT_MODE            BootMode;
  EFI_STATUS               Status;

  DEBUG((EFI_D_INFO, "Kernel Debug Enabled - Set 8-bit Legacy Mode for UART2\n"));

  // Set UART Byte Address Control - Control bit for 16550 8-Bit Addressing Mode.
  SideBandWrite32(SB_LPSS_PORT, R_LPSS_SB_GPPRVRW7, B_LPSS_UART2_BYTE_ADDR_EN);

  // An MMIO Read Must Immediately Be Issued to UART2 BAR0 + 0xF8 for 8-bit Legacy Mode to Activate
  MmioRead32((UINTN) Uart2MmioBase0 + R_LPSS_IO_MANID);

  // Send PMC IPC1 CMD to Enable UART Debug (enable S0ix when UART debug is enabled)
  IpcSendCommandEx(IPC_CMD_ID_PM_DEBUG, IPC_SUBCMD_ID_LPSS_DEBUG_UART_ENABLE, NULL, 0);

  Status = PeiServicesGetBootMode (&BootMode);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Restore UART2 BAR in S3 Resume
  if (BootMode == BOOT_ON_S3_RESUME) {
    Uart2MmioBase0 = LpssConfig->Uart2KernelDebugBaseAddress;
    DEBUG ((EFI_D_INFO, "UART2 BAR Restored To: 0x%x\n", Uart2MmioBase0));

    MmioAnd32 ((UINTN)(Uart2PciMmBase + R_LPSS_IO_STSCMD), (UINT32)~(B_LPSS_IO_STSCMD_BME | B_LPSS_IO_STSCMD_MSE));

    ASSERT (((Uart2MmioBase0 & B_LPSS_IO_BAR_BA) == Uart2MmioBase0) && (Uart2MmioBase0 != 0));
    MmioWrite32 ((UINTN) (Uart2PciMmBase + R_LPSS_IO_BAR), (UINT32) (Uart2MmioBase0 & B_LPSS_IO_BAR_BA));

    MmioOr32 ((UINTN) (Uart2PciMmBase + R_LPSS_IO_STSCMD), (UINT32) (B_LPSS_IO_STSCMD_BME | B_LPSS_IO_STSCMD_MSE));
  }

  return EFI_SUCCESS;

  // Note: The UART2 Device Will Be Hidden in an ExitBootServices Callback (Needs PCI Base Address Before Hide)
}


/**
  Configure LPSS IO devices per DevIndex

  @param[in] SiPolicy             The Silicon Policy PPI instance
  @param[in] LpssConfig           The LPSS Config Block instance
  @param[in, out] FuncDisableReg  The value of Function disable register
  @param[in] DevIndex             The index to access all devices under LPSS

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureLpssIoDevices (
  IN SI_POLICY_PPI  *SiPolicy,
  IN SC_LPSS_CONFIG *LpssConfig,
  IN OUT UINT32     *FuncDisableReg,
  IN UINT32         DevIndex
  )
{
  UINTN                LpssPciMmBase;
  EFI_PHYSICAL_ADDRESS LpssMmioBase0;
  UINT32               Value;
  SC_DEV_MODE          *ScDevMode;
  EFI_STATUS           Status;
  BOOLEAN              DeviceConfigured;
  MEFWCAPS_SKU         CurrentFeatures;

  LpssPciMmBase    = 0;
  LpssMmioBase0    = (EFI_PHYSICAL_ADDRESS) SiPolicy->TempMemBaseAddr;
  Value            = 0;
  Status           = EFI_NOT_FOUND;
  DeviceConfigured = FALSE;

  //
  // Pointer to the first item of LPSS config list
  //
  ScDevMode = (SC_DEV_MODE*)((UINT8*)LpssConfig + sizeof(CONFIG_BLOCK_HEADER));
  LpssPciMmBase = MmPciBase (
                  DEFAULT_PCI_BUS_NUMBER_SC,
                  mLpssIoDevices[DevIndex].PciDeviceNum,
                  mLpssIoDevices[DevIndex].PciFuncNum
                  );
  //
  // Check if device present
  //
  if (MmioRead32 (LpssPciMmBase) == 0xFFFFFFFF) {
    DEBUG ((EFI_D_INFO, "LPSS IO device (%d) not present, skipping.\n", DevIndex));
    ScDevMode[DevIndex] = ScDisabled;
    *FuncDisableReg |= mLpssIoDevices[DevIndex].LpssFuncDisableBit;
  } else {
    //
    // Enable PCI Power Gating bits for every LPSS PCI devices.
    //
    MmioOr32 ((UINTN) (LpssPciMmBase + R_LPSS_IO_D0I3MAXDEVPG), BIT17| BIT18);

    if (((MmioRead32 ((UINTN) (LpssPciMmBase + R_LPSS_IO_STSCMD)) & B_LPSS_IO_STSCMD_MSE) != 0) && \
        ((MmioRead32 ((UINTN) (LpssPciMmBase + R_LPSS_IO_BAR)) & 0xFFFFFFF0) != 0)) {
      DeviceConfigured = TRUE;
    } else {
      ///
      /// Disable Bus Master Enable & Memory Space Enable
      ///
      MmioAnd32 ((UINTN) (LpssPciMmBase + R_LPSS_IO_STSCMD), (UINT32) ~(B_LPSS_IO_STSCMD_BME | B_LPSS_IO_STSCMD_MSE));
      ///
      /// Program BAR 0
      ///
      ASSERT (((LpssMmioBase0 & B_LPSS_IO_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
      MmioWrite32 ((UINTN) (LpssPciMmBase + R_LPSS_IO_BAR), (UINT32) (LpssMmioBase0 & B_LPSS_IO_BAR_BA));
      ///
      /// Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 ((UINTN) (LpssPciMmBase + R_LPSS_IO_STSCMD), (UINT32) (B_LPSS_IO_STSCMD_BME | B_LPSS_IO_STSCMD_MSE));
    }
  }

  //
  // Handle MMIO programming using temp BAR0
  //
  {
    ///
    /// Disable BAR1 when device is working in PCI mode.
    ///
    if (ScDevMode[DevIndex] == ScDisabled) {
      ///
      /// Put device in D3Hot state before disabling it.
      ///
      DEBUG ((EFI_D_INFO, "Putting LPSS IO device (%d) into D3 Hot State.\n", DevIndex));
      ///
      /// 1)  The Restore Required (bit 3 of the Convergence Layer Offset 0x24c DevIdle_Control) must be cleared by BIOs by writing a '1' to it.
      ///
      MmioOr32 ((UINTN) (LpssMmioBase0 + R_LPSS_IO_DEV_IDLE_CTRL), BIT3);
      ///
      /// 2)  The respective Slice DevIdle register bit (Convergence Layer Offset 0x24c DevIdle_Control) must be set (DevIdleC, bit 2 = '1')
      ///
      MmioOr32 ((UINTN) (LpssMmioBase0 + R_LPSS_IO_DEV_IDLE_CTRL), BIT2);
      ///
      /// 3)  Enable Power Gating of I2C2, I2C3, I2C4, I2C5 by seting the Section 51.23.19, 0x 0A0 D0I3MAXDEVPG, bits 16 (PMCRE), 17 (IE3), (18 (PGE) to 1.
      ///
      ///   MmioOr32 ((UINTN) (LpssPciMmBase + R_LPSS_IO_D0I3MAXDEVPG), BIT16| BIT17| BIT18);
      ///   S3BootScriptSaveMemWrite (EfiBootScriptWidthUint32,
      ///  (UINTN) (LpssPciMmBase + R_LPSS_IO_D0I3MAXDEVPG), 1,
      ///  (VOID *) (UINTN) (LpssPciMmBase + R_LPSS_IO_D0I3MAXDEVPG));
      ///
      /// 4)  Place I2C2, I2C3, I2C4, I2C5 in the D3 State, by setting the section 51.23.14, PCI Configuration, 0x 084 PMECTRLSTATUS, bits 1:0 (POWERSTATE) to 2'b11.
      ////
      MmioOr32 ((UINTN) (LpssPciMmBase + R_LPSS_IO_PCS), B_LPSS_IO_PCS_PS);
    } else if (ScDevMode[DevIndex] == ScPciMode) {
      ///
      /// Disable BAR1 in PSF
      ///
      SideBandAndThenOr32(LPSS_EP_PORT_ID, mLpssIoDevices[DevIndex].LpssPciCfgCtrl, 0xFFFFFFFF, B_LPSS_PCICFGCTRL_BAR1_DIS);

      Value = SideBandRead32(LPSS_EP_PORT_ID, mLpssIoDevices[DevIndex].LpssPciCfgCtrl);
      DEBUG ((EFI_D_INFO, "Read back LpssPciCfgCtrl Value = %x, Index = %d\n", Value, DevIndex));

      SideBandAndThenOr32(SB_PORTID_PSF3, mLpssIoDevices[DevIndex].Psf3BaseAddress + R_PCH_PCR_PSFX_T0_SHDW_PCIEN, 0xFFFFFFFF, B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR1DIS);

      Value = SideBandRead32(SB_PORTID_PSF3, mLpssIoDevices[DevIndex].Psf3BaseAddress + R_PCH_PCR_PSFX_T0_SHDW_PCIEN);
      DEBUG ((EFI_D_INFO, "Read back Psf3BaseAddress Value = %x, Index = %d\n", Value, DevIndex));
      ///
      /// Note: From HAS, BAR0 need to program into Remap register as well (REMAP_ADDRESS_LOW/HIGH (+SPT))
      /// This need to be done after PCI scan, all BARs are allocated after the scan.
      ///

      ///
      /// Release Resets
      ///
      MmioWrite32 ((UINTN) (LpssMmioBase0 + R_LPSS_IO_MEM_RESETS), (UINT32) (B_LPSS_IO_MEM_HC_RESET_REL | B_LPSS_IO_MEM_iDMA_RESET_REL));

#ifdef SC_PM_ENABLE
      if ((PLATFORM_ID == VALUE_REAL_PLATFORM) || (PLATFORM_ID == VALUE_SLE_UPF)) {
        DEBUG ((EFI_D_INFO, "Start SC_PM_ENABLE\n"));
        //
        // PM LTR Enabling
        //
        MmioWrite32 ((UINTN) (LpssMmioBase0 + R_LPSS_IO_ACTIVELTR), (UINT32) (B_LPSS_IO_ACTIVELTR_LATENCY));
        Value = MmioRead32((UINTN) (LpssMmioBase0 + R_LPSS_IO_ACTIVELTR));
        DEBUG ((EFI_D_INFO, "Read back LPSS Active LTR Value = %x, Index = %d\n", Value, DevIndex));

        MmioWrite32 ((UINTN) (LpssMmioBase0 + R_LPSS_IO_IDLELTR), (UINT32) (B_LPSS_IO_IDLELTR_LATENCY));
        Value = MmioRead32((UINTN) (LpssMmioBase0 + R_LPSS_IO_IDLELTR));
        DEBUG ((EFI_D_INFO, "Read back LPSS Idle LTR Value = %x, Index = %d\n", Value, DevIndex));
        DEBUG ((EFI_D_INFO, "End SC_PM_ENABLE\n"));
      }
#endif
    } else {
      //
      // ScAcpiMode
      //

      ///
      /// ACPI mode is not POR'd
      ///
      /// S/W sequence for ACPI Mode
      /// 1. PCI Enumeration ->  BAR0  is programmed
      /// 2. PCI Enumeration-> BAR1 is programmed
      /// WARNING: BAR 1 MUST always be enumerated in order for proper operation of LPSS
      ///
      /// 3. Requirement that MSE=1 and Device is in D0 state
      /// 4. Program Bridge in ACPI mode -> Turn OFF PCI config space
      /// 5. Program PSF in ACPI mode -> Turn OFF PCI config space
      /// 6. Write Remap register with BAR value
      ///
      DEBUG ((DEBUG_ERROR, "ACPI Mode is not PORed in BXT\n", Value, DevIndex));
    }

    if (mLpssIoDevices[DevIndex].PciDeviceNum == PCI_DEVICE_NUMBER_LPSS_HSUART) {
      //
      // Special handling for UART Devices
      //

      ///
      /// Program Divider & Activate Clock
      /// N = 15625 = 0x3D09
      /// M = 288 = 0x120
      /// M/N = 0.0184320
      /// Divisor = 1 (High Byte = 0, Low Byte = 1)
      /// Functional Clock = 100 MHz * 0.018372 = 1.84320 MHz
      /// Baudrate = (Functional Clock) / (16 * Divisor) = 1.84320MHz / 16 = 115200 Hz
      ///
      MmioWrite32 ((UINTN) (LpssMmioBase0 + R_LPSS_IO_MEM_RESETS), 0);
      MmioWrite32 ((UINTN) (LpssMmioBase0 + R_LPSS_IO_MEM_RESETS), (UINT32) (B_LPSS_IO_MEM_HC_RESET_REL | B_LPSS_IO_MEM_iDMA_RESET_REL));

      Value = (V_LPSS_IO_PPR_CLK_N_DIV << 16) | (V_LPSS_IO_PPR_CLK_M_DIV << 1) | B_LPSS_IO_MEM_PCP_CLK_EN;
      MmioWrite32 ((UINTN)LpssMmioBase0 + R_LPSS_IO_MEM_PCP, Value);

      MmioWrite32 ((UINTN)LpssMmioBase0 + R_LPSS_IO_MEM_PCP, B_LPSS_IO_MEM_PCP_CLK_UPDATE | Value);
      MmioRead32 ((UINTN)LpssMmioBase0 + R_LPSS_IO_MEM_PCP);
      //
      // Program Divisor to DLL/DLH registers.
      //
      Value = MmioRead32((UINTN)LpssMmioBase0 + R_LPSS_UART_MEM_LCR);
      Value |= B_LPSS_UART_MEM_LCR_DLAB;  // Set DLAB = 1
      MmioWrite32 ((UINTN)LpssMmioBase0 + R_LPSS_UART_MEM_LCR, Value);

      MmioWrite32 ((UINTN)LpssMmioBase0 + R_LPSS_UART_MEM_DLL, 1);
      MmioWrite32 ((UINTN)LpssMmioBase0 + R_LPSS_UART_MEM_DLH, 0);

      Value &= ~B_LPSS_UART_MEM_LCR_DLAB; // Clear DLAB = 0
      MmioWrite32 ((UINTN)LpssMmioBase0 + R_LPSS_UART_MEM_LCR, Value);

      ///
      /// UART2 is selected as the OS debug port
      ///
      if (mLpssIoDevices[DevIndex].PciFuncNum == PCI_FUNCTION_NUMBER_LPSS_HSUART2
          && LpssConfig->OsDbgEnable) {
        Status = ConfigureUart2KernelDebug (LpssConfig, LpssPciMmBase, LpssMmioBase0);
        ASSERT_EFI_ERROR (Status);
      }
    //
    // Special handling for SPI Devices
    //
    } else if (mLpssIoDevices[DevIndex].PciDeviceNum == PCI_DEVICE_NUMBER_LPSS_SPI) {
       ///
       /// Program Divider & Activate Clock
       /// N = 2
       /// M = 1
       /// M/N = 0.5
       /// Functional Clock = 100 MHz * 0.5 = 50 MHz
       ///
       Value = B_LPSS_IO_MEM_PCP_CLK_UPDATE | (0x0002 << 16) | (0x0001 << 1) | B_LPSS_IO_MEM_PCP_CLK_EN;
       MmioWrite32 ((UINTN)LpssMmioBase0 + R_LPSS_IO_MEM_PCP, Value);
       MmioRead32 ((UINTN)LpssMmioBase0 + R_LPSS_IO_MEM_PCP);
     }
  }
  ///
  /// Handle MMIO programming using temp BAR0
  ///
  if (!DeviceConfigured) {
    ///
    /// Disable Bus Master Enable & Memory Space Enable
    ///
    MmioAnd32 ((UINTN) (LpssPciMmBase + R_LPSS_IO_STSCMD), (UINT32) ~(B_LPSS_IO_STSCMD_BME | B_LPSS_IO_STSCMD_MSE));

    ///
    /// Clear BAR0
    ///
    MmioWrite32 ((UINTN) (LpssPciMmBase + R_LPSS_IO_BAR), (UINT32) (0x00));
  }
  //
  // Since the setting below will disable PCI device, so need to do after PCI config space access operation
  //
  if (ScDevMode[DevIndex] == ScDisabled) {
    if (GetBxtSeries() != BxtP) {
      if(DevIndex == 2) {
        Status = HeciGetFwFeatureStateMsgII (&CurrentFeatures);
        if (EFI_ERROR (Status)) {
          ASSERT_EFI_ERROR (Status);
          return Status;
        }
        if (CurrentFeatures.Fields.NFC == 1) {
          //
          // if CSE NFC Strap is enabled, CSE owns the I2C2, so don't disable the I2C2.
          //
          return Status;
        }
      }
    }

    ///
    /// 5) Disable device's PciCfg
    ///
    SideBandAndThenOr32(
      LPSS_EP_PORT_ID,
      mLpssIoDevices[DevIndex].LpssPciCfgCtrl,
      0xFFFFFFFF,
      B_LPSS_PCICFGCTRL_PCI_CFG_DIS
      );
    ///
    /// 6) Disable device's PciCfg in PSF
    ///
    SideBandAndThenOr32(
      SB_PORTID_PSF3,
      mLpssIoDevices[DevIndex].Psf3BaseAddress + R_PCH_PCR_PSFX_T0_SHDW_CFG_DIS,
      0xFFFFFFFF,
      B_PCH_PCR_PSFX_T0_SHDW_CFG_DIS_CFGDIS
      );
    ///
    /// 7) Function Disable in PSF
    ///
    SideBandAndThenOr32(
      SB_PORTID_PSF3,
      mLpssIoDevices[DevIndex].Psf3BaseAddress + R_PCH_PCR_PSFX_T0_SHDW_PCIEN,
      0xFFFFFFFF,
      B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR1DIS | B_PCH_PCR_PSFX_T0_SHDW_PCIEN_FUNDIS
      );
    *FuncDisableReg |= mLpssIoDevices[DevIndex].LpssFuncDisableBit;
  }

  return Status;
}


/**
  Configure LPSS devices.

  @param[in] SiPolicy             The Silicon Policy PPI instance
  @param[in] ScPolicy             The SC Policy Ppi instance
  @param[in, out] FuncDisableReg  The value of Function disable register

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureLpss (
  IN SI_POLICY_PPI                 *SiPolicy,
  IN SC_POLICY_PPI                 *ScPolicy,
  IN OUT UINT32                    *FuncDisableReg
  )
{
  EFI_STATUS     Status;
  UINT32         DevIndex;
  UINT32         Value;
  SC_LPSS_CONFIG *LpssConfig;

  Value = 0;

  DEBUG ((EFI_D_INFO, "ConfigureLpss() Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicy, &gLpssConfigGuid, (VOID *) &LpssConfig);
  ASSERT_EFI_ERROR (Status);

#ifdef SC_PM_ENABLE
  if ((PLATFORM_ID == VALUE_REAL_PLATFORM) || (PLATFORM_ID == VALUE_SLE_UPF)) {
    DEBUG ((EFI_D_INFO, "Start SC_PM_ENABLE\n"));
    ///
    /// Step 1: Program GPPRVRW2 (PGCB_CDC_CFG0 - PGCB and CDC Configuration 0)
    /// Note: LPSS implementation inverts bits 0 and 2; thus a programmed value of 0h is interpreted as 5h (default); to use any other value, program a value with the corresponding bits 0 and 2 inverted.
    ///
    SideBandAndThenOr32(
      LPSS_EP_PORT_ID,
      R_LPSS_SB_GPPRVRW2,
      0xFFFFFFFF,
      (B_LPSS_PGCB_FRC_CLK_CP_EN | B_LPSS_CDC_SIDE_CFG_CLKREQ_CTL_EN | BIT2)
    );

    Value = SideBandRead32(LPSS_EP_PORT_ID, R_LPSS_SB_GPPRVRW2);
    DEBUG ((EFI_D_INFO, "Read back R_LPSS_SB_GPPRVRW2 Value = %x\n", Value));
    ///
    /// Step 2: Program PMCTL
    ///
    if (LpssConfig->S0ixEnable == FALSE) {
      SideBandAndThenOr32(LPSS_EP_PORT_ID,
        R_LPSS_SB_PMCTL,
        0xFFFFFFFF,
        (B_IOSFPRIMCLK_GATE_EN |
        B_OCPCLK_GATE_EN |
        B_OCPCLK_TRUNK_GATE_EN |
        B_IOSFSBCLK_GATE_EN)
        );
    } else {
      SideBandAndThenOr32(LPSS_EP_PORT_ID,
        R_LPSS_SB_PMCTL,
        0xFFFFFFFF,
        (B_IOSFPRIMCLK_GATE_EN |
        B_OCPCLK_GATE_EN |
        B_OCPCLK_TRUNK_GATE_EN |
        B_IOSFSBCLK_GATE_EN |
        B_IOSFPRIM_TRUNK_GATE_EN |
        B_IOSFSB_TRUNK_GATE_EN)
        );
    }
    Value = SideBandRead32(LPSS_EP_PORT_ID, R_LPSS_SB_PMCTL);
    DEBUG ((EFI_D_INFO, "Read back R_LPSS_SB_PMCTL Value = %x\n", Value));
    DEBUG ((EFI_D_INFO, "End SC_PM_ENABLE\n"));
  }
#endif
  ///
  /// Step 3: Program D0I3MAXDEVPG
  ///
  for(DevIndex = 0; DevIndex < LPSS_IO_DEVICE_NUM; DevIndex++) {
    Status = ConfigureLpssIoDevices(SiPolicy, LpssConfig, FuncDisableReg, DevIndex);
  }

  DEBUG ((EFI_D_INFO, "ConfigureLpss() End\n"));

  return EFI_SUCCESS;
}

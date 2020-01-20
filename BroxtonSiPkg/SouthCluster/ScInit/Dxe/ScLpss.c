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

#include <IndustryStandard/Acpi.h>
#include <Library/GpioLib.h>
#include <Library/PmcIpcLib.h>
#include <Library/SteppingLib.h>
#include <ScRegs/RegsPsf.h>
#include <SaAccess.h>
#include "ScInit.h"

typedef struct {
  UINT32 PciDeviceNum;
  UINT32 PciFuncNum;
  UINT16 LpssPciCfgCtrl;
  UINT16 Psf3BaseAddress;
  UINT32 LpssFuncDisableBit;
} SERIAL_IO_CONTROLLER_DESCRIPTOR;

#define LPSS_IO_DEVICE_NUM 15

//
// Note: the devices order need to align with the order defined in SC_LPSS_CONFIG
//
SERIAL_IO_CONTROLLER_DESCRIPTOR mLpssIoDevices[LPSS_IO_DEVICE_NUM] =
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

  // SPI 0~2
  {PCI_DEVICE_NUMBER_LPSS_SPI, PCI_FUNCTION_NUMBER_LPSS_SPI0, R_LPSS_SB_PCICFGCTRL_SPI0, R_PCH_PCR_PSF3_T0_SHDW_LPSS_SPI0_REG_BASE, B_PMC_FUNC_DIS_LPSS_SPI0},
  {PCI_DEVICE_NUMBER_LPSS_SPI, PCI_FUNCTION_NUMBER_LPSS_SPI1, R_LPSS_SB_PCICFGCTRL_SPI1, R_PCH_PCR_PSF3_T0_SHDW_LPSS_SPI1_REG_BASE, B_PMC_FUNC_DIS_LPSS_SPI1},
  {PCI_DEVICE_NUMBER_LPSS_SPI, PCI_FUNCTION_NUMBER_LPSS_SPI2, R_LPSS_SB_PCICFGCTRL_SPI2, R_PCH_PCR_PSF3_T0_SHDW_LPSS_SPI2_REG_BASE, B_PMC_FUNC_DIS_LPSS_SPI2}
};

/**
  Hide PCI config space of LPSS devices and do any final initialization.

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
  @retval EFI_NOT_FOUND                 GNVS Protocol interface not found
**/
EFI_STATUS
ConfigureLpssAtBoot (
  IN SC_POLICY_HOB              *ScPolicy
  )
{
  UINTN                         LpssPciMmBase;
  UINT32                        LpssMmioBase0;
  UINT32                        LpssMmioBase0_High;
  UINT32                        P2sbMmioBar;
  UINT32                        Buffer32;
  EFI_STATUS                    AcpiTablePresent;
  EFI_STATUS                    Status;
  UINT64                        Data64;
  UINTN                         AcpiTableKey;
  UINT8                         Index;
  SC_LPSS_CONFIG                *LpssConfig;

#ifndef FSP_FLAG
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;
#endif
  UINTN                         P2sbPciMmBase;

  DEBUG ((DEBUG_INFO, "ConfigureLpssAtBoot() Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicy, &gLpssConfigGuid, (VOID *) &LpssConfig);
  ASSERT_EFI_ERROR (Status);

  LpssPciMmBase            = 0;
  LpssMmioBase0            = 0;
  LpssMmioBase0_High       = 0;
  P2sbMmioBar              = 0;
  Buffer32                 = 0;
  AcpiTablePresent         = EFI_NOT_FOUND;
  AcpiTableKey             = 0;
  Data64                   = 0;
  Index                    = 0;
  Status                   = EFI_SUCCESS;

#ifndef FSP_FLAG
  //
  // Locate ACPI table
  //
  AcpiTablePresent = InitializeScAslUpdateLib ();
#endif


  //
  // 1. Program LPSS REMAP register. At boot time, the BAR is allocated during PCI scan.
  // 2. Hide UART2 device if kernel debug mode is enabled
  //
  for (Index = 0; Index < LPSS_IO_DEVICE_NUM; Index++) {
    LpssPciMmBase = MmPciBase(
      DEFAULT_PCI_BUS_NUMBER_SC,
      mLpssIoDevices[Index].PciDeviceNum,
      mLpssIoDevices[Index].PciFuncNum
      );

    LpssMmioBase0 = MmioRead32((UINTN)(LpssPciMmBase + R_LPSS_IO_BAR));
    LpssMmioBase0 &= 0xFFFFFFF0;  // Mask off the last byte read from BAR

    LpssMmioBase0_High = MmioRead32((UINTN)(LpssPciMmBase + R_LPSS_IO_BAR_HIGH));

    //
    // Assume the MMIO address for BAR0 is always 32 bits (high DW is 0).
    //
    MmioWrite32((UINTN)(LpssMmioBase0 + R_LPSS_IO_REMAP_ADDRESS_LOW), LpssMmioBase0);
    MmioWrite32((UINTN)(LpssMmioBase0 + R_LPSS_IO_REMAP_ADDRESS_HI), LpssMmioBase0_High);

    DEBUG((DEBUG_INFO, "Read back LPSS REMAP Register, High DW = 0x%x, Low DW = 0x%x, Index = %d\n",
      MmioRead32((UINTN)(LpssMmioBase0 + R_LPSS_IO_REMAP_ADDRESS_HI)),
      MmioRead32((UINTN)(LpssMmioBase0 + R_LPSS_IO_REMAP_ADDRESS_LOW)),
      Index));

    //
    // Hide the UART2 device if kernel debugger mode is enabled
    // BXT UART2 device is hidden in OnReadyToBoot (BIOS and OS use different debug COM port)
    //

      if (mLpssIoDevices[Index].PciDeviceNum == PCI_DEVICE_NUMBER_LPSS_HSUART
        && mLpssIoDevices[Index].PciFuncNum == PCI_FUNCTION_NUMBER_LPSS_HSUART2
        && GetBxtSeries() == BxtP
        && LpssConfig->OsDbgEnable
        && LpssMmioBase0 != 0xFFFFFFFF) {

        DEBUG((DEBUG_INFO, "Kernel Debug is Enabled\n"));
        DEBUG((DEBUG_INFO, "---------------------------------------------------------------------------------\n"));
        DEBUG((DEBUG_INFO, "This concludes BIOS serial debug messages, open the kernel debugger (WinDBG) now.\n\n"));
        DEBUG((DEBUG_INFO, "To enable continued serial and console redirection:\n"));
        DEBUG((DEBUG_INFO, "    1) Open the BIOS setup menu\n"));
        DEBUG((DEBUG_INFO, "    2) Navigate to Device Manager->System Setup->Debug Configuration->Kernel Debugger Enable\n"));
        DEBUG((DEBUG_INFO, "    3) Select the \"Disabled\" option and save\n\n"));

        // Hide the UART2 device to prevent an OS driver from loading against it
        SideBandAndThenOr32(
          SB_PORTID_PSF3,
          R_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2,
          0xFFFFFFFF,
          B_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2_CFGDIS
          );

        DEBUG((DEBUG_INFO, "Kernel Debugger Mode Enabled - LPSS D24 F2 CFG_DIS: 0x%08X\n", SideBandRead32(SB_PORTID_PSF3, R_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2)));
      }
  }

#ifndef FSP_FLAG
  //
  // Update LPSS devices ACPI variables
  //
  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &GlobalNvsArea
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
#endif
  P2sbPciMmBase = MmPciBase (
                    DEFAULT_PCI_BUS_NUMBER_SC,
                    PCI_DEVICE_NUMBER_P2SB,
                    PCI_FUNCTION_NUMBER_P2SB
                    );
  P2sbMmioBar = MmioRead32 (P2sbPciMmBase + R_PCH_P2SB_SBREG_BAR);
  if (P2sbMmioBar == 0xFFFFFFFF) {
    // P2SB has been hidden, read it from Pcd
    P2sbMmioBar = PcdGet32 (PcdP2SBBaseAddress);
  } else {
    P2sbMmioBar &= B_PCH_P2SB_SBREG_RBA;
  }

#ifndef FSP_FLAG
  GlobalNvsArea->Area->GPIO0Addr = P2SB_MMIO_ADDR (P2sbMmioBar, NORTH, 0);
  GlobalNvsArea->Area->GPIO0Len = GPIO_NORTH_COMMUNITY_LENGTH;

  GlobalNvsArea->Area->GPIO1Addr = P2SB_MMIO_ADDR (P2sbMmioBar, NORTHWEST, 0);
  GlobalNvsArea->Area->GPIO1Len = GPIO_NORTHWEST_COMMUNITY_LENGTH;

  GlobalNvsArea->Area->GPIO2Addr = P2SB_MMIO_ADDR (P2sbMmioBar, WEST, 0);
  GlobalNvsArea->Area->GPIO2Len = GPIO_WEST_COMMUNITY_LENGTH;

  GlobalNvsArea->Area->GPIO3Addr = P2SB_MMIO_ADDR (P2sbMmioBar, SOUTHWEST, 0);
  GlobalNvsArea->Area->GPIO3Len = GPIO_SOUTHWEST_COMMUNITY_LENGTH;

#if (TABLET_PF_ENABLE == 1)
  GlobalNvsArea->Area->GPIO4Addr = P2SB_MMIO_ADDR (P2sbMmioBar, SOUTH, 0);
  GlobalNvsArea->Area->GPIO4Len = GPIO_SOUTH_COMMUNITY_LENGTH;
#endif

  DEBUG ((DEBUG_INFO, "GPIOxAddr: North: 0x%X, Northwest: 0x%X, West: 0x%X, Southwest: 0x%X, South: 0x%X\n", \
    GlobalNvsArea->Area->GPIO0Addr,
    GlobalNvsArea->Area->GPIO1Addr,
    GlobalNvsArea->Area->GPIO2Addr,
    GlobalNvsArea->Area->GPIO3Addr,
    GlobalNvsArea->Area->GPIO4Addr
    ));
#endif
#ifdef SC_PM_ENABLE
if ((PLATFORM_ID == VALUE_REAL_PLATFORM) ||
    (PLATFORM_ID == VALUE_SLE_UPF)) {
  DEBUG ((DEBUG_INFO, "Start SC_PM_ENABLE for GPIO\n"));
  GpioWrite (NORTH,     R_GPIO_MISCCFG, GpioRead(NORTH    , R_GPIO_MISCCFG) | B_GPIP_MISCCFG);
  GpioWrite (NORTHWEST, R_GPIO_MISCCFG, GpioRead(NORTHWEST, R_GPIO_MISCCFG) | B_GPIP_MISCCFG);
  GpioWrite (WEST,      R_GPIO_MISCCFG, GpioRead(WEST     , R_GPIO_MISCCFG) | B_GPIP_MISCCFG);
  GpioWrite (SOUTHWEST, R_GPIO_MISCCFG, GpioRead(SOUTHWEST, R_GPIO_MISCCFG) | B_GPIP_MISCCFG);
#if (TABLET_PF_ENABLE == 1)
  GpioWrite (SOUTH,     R_GPIO_MISCCFG, GpioRead(SOUTH    , R_GPIO_MISCCFG) | B_GPIP_MISCCFG);
#endif
  DEBUG ((DEBUG_INFO, "End SC_PM_ENABLE for GPIO\n"));
}
#endif

  DEBUG ((DEBUG_INFO, "ConfigureLpssAtBoot() End\n"));
  return EFI_SUCCESS;
}

/**
  Stop Lpss devices[I2C and SPI] if any is running.

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
StopLpssAtBoot (
  VOID
  )
{
  UINTN                         LpssPciMmBase;
  UINT32                        LpssMmioBase0;
  UINT8                         Index;
  UINT32                        D32 = 0;

  DEBUG ((DEBUG_INFO, "Stop LPSS controllers if any controller is running(I2C and SPI)...\n"));
  for (Index = 0; Index < LPSS_IO_DEVICE_NUM; Index++) {
    LpssPciMmBase = MmPciBase (
                      DEFAULT_PCI_BUS_NUMBER_SC,
                      mLpssIoDevices[Index].PciDeviceNum,
                      mLpssIoDevices[Index].PciFuncNum
                      );

    LpssMmioBase0 = MmioRead32 ((UINTN) (LpssPciMmBase + R_LPSS_IO_BAR));
    LpssMmioBase0 &= 0xFFFFFFF0;  // Mask off the last byte read from BAR

    if ((mLpssIoDevices[Index].PciDeviceNum  == PCI_DEVICE_NUMBER_LPSS_I2C0) || \
        (mLpssIoDevices[Index].PciDeviceNum  == PCI_DEVICE_NUMBER_LPSS_I2C1)) {
      D32 = MmioRead32 ((UINTN) (LpssMmioBase0 + R_LPSS_I2C_IC_ENABLE));
      if (D32 & B_LPSS_I2C_IC_ENABLE_ENABLE) {
        DEBUG ((DEBUG_INFO, "Stop I2C controller D:%02X, F: %02X...\n", mLpssIoDevices[Index].PciDeviceNum, mLpssIoDevices[Index].PciFuncNum));
        MmioWrite32 ((UINTN)LpssMmioBase0 + R_LPSS_I2C_IC_ENABLE, \
        MmioRead32 ((UINTN) (LpssMmioBase0 + R_LPSS_I2C_IC_ENABLE)) & (~(UINT32)B_LPSS_I2C_IC_ENABLE_ENABLE));
      }
    } else if (mLpssIoDevices[Index].PciDeviceNum  == PCI_DEVICE_NUMBER_LPSS_SPI) {
      D32 = MmioRead32 ((UINTN) (LpssMmioBase0 + R_LPSS_SPI_MEM_SSP_CONTROL0));
      if (D32 & B_LPSS_SPI_MEM_SSP_CONTROL0_SSE) {
        DEBUG ((DEBUG_INFO, "Stop SPI controller D:%02X, F: %02X...\n", mLpssIoDevices[Index].PciDeviceNum, mLpssIoDevices[Index].PciFuncNum));
        MmioWrite32 ((UINTN)LpssMmioBase0 + R_LPSS_SPI_MEM_SSP_CONTROL0, \
        MmioRead32 ((UINTN) (LpssMmioBase0 + R_LPSS_SPI_MEM_SSP_CONTROL0)) & (~(UINT32)B_LPSS_SPI_MEM_SSP_CONTROL0_SSE));
      }
    }
  }
  return EFI_SUCCESS;
}

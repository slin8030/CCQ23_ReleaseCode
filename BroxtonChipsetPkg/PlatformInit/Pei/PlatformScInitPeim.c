/** @file
 Early SC platform initialization.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

#include <Library/PcdLib.h>
#include <Library/PciExpressLib.h>
#include <Library/PeiScPolicyLib.h>
#include <Library/PreSiliconLib.h>
#include <Library/SideBandLib.h>
#include <Library/PeiScPolicyUpdateLib.h>
#include <Library/SteppingLib.h>
//[-start-160216-IB03090424-add]//
#include <Library/ScPlatformLib.h>
//[-end-160216-IB03090424-add]//

#include "PlatformInit.h"
#include <Ppi/ScPolicy.h>
#include <ScAccess.h>
#include <Guid/PlatformEmmcHs400Info.h>
#include <PlatformBaseAddresses.h>
#include <Library/PlatformConfigDataLib.h>
#include <Ppi/ScPcieDeviceTable.h>

//[-start-151229-IB03090424-add]//
#include <Ppi/SiPolicyPpi.h>
#include <Library/MmPciLib.h>
//[-end-151229-IB03090424-add]//
//[-start-151216-IB11270137-add]//
#include <Library/PeiOemSvcChipsetLib.h>
//[-end-151216-IB11270137-add]//
#include <ConfigBlock/ScInterruptAssign.h>
//[-start-160914-IB07400784-add]//
#include <Library/MultiPlatformBaseLib.h>
//[-end-160914-IB07400784-add]//

#define P_CR_THERMAL_DEVICE_IRQ_0_0_0_MCHBAR_PUNIT_REG     0x0000700C
#define V_PUINT_INTLN                                      0x18
#define PUINT_INTPIN_LOCK                                  BIT31

//[-start-160525-IB06720411-remove]//
//typedef struct {
//  UINT8   Port;
//  UINT16  PciCfgOffset;
//  UINT8   PciIrqNumber;
//  UINT8   IrqPin;
//} PRIVATE_PCICFGCTRL;
//[-end-160525-IB06720411-remove]//

//[-start-160813-IB07400770-modify]//
#ifdef LEGACY_IRQ_SUPPORT
PRIVATE_PCICFGCTRL directIrqTable[] = {
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C0,  27, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C1,  28, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C2,  29, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C3,  30, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C4,  31, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C5,  32, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C6,  33, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C7,  34, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART0, 34, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART1, 35, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART2,  6, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART3, 37, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI0,  35, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI1,  36, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI2,  37, V_PCICFG_CTRL_INTC },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_SDCARD, 33, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_EMMC  , 39, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_UFS   , 38, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_SDIO  , 42, V_PCICFG_CTRL_INTA },
  { SB_ISH_BRIDGE_PORT,  R_ISH_SB_PCICFGCTRL_ISH   , 26, V_PCICFG_CTRL_INTA },
  { SB_USB_DEVICE_PORT,  R_USB_SB_PCICFGCTRL_XDCI  , 13, V_PCICFG_CTRL_INTB },
};
#else
PRIVATE_PCICFGCTRL directIrqTable[] = {
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C0,  27, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C1,  28, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C2,  29, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C3,  30, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C4,  31, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C5,  32, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C6,  33, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C7,  34, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART0,  4, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART1,  5, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART2,  6, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART3,  7, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI0,  35, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI1,  36, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI2,  37, V_PCICFG_CTRL_INTC },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_SDCARD,  3, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_EMMC  , 39, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_UFS   , 38, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_SDIO  , 42, V_PCICFG_CTRL_INTA },
  { SB_ISH_BRIDGE_PORT,  R_ISH_SB_PCICFGCTRL_ISH   , 26, V_PCICFG_CTRL_INTA },
  { SB_USB_DEVICE_PORT,  R_USB_SB_PCICFGCTRL_XDCI  , 13, V_PCICFG_CTRL_INTB },
};
#endif
//[-end-160813-IB07400770-modify]//

#define PCI_CLASS_NETWORK             0x02
#define PCI_CLASS_NETWORK_ETHERNET    0x00
#define PCI_CLASS_NETWORK_OTHER       0x80

GLOBAL_REMOVE_IF_UNREFERENCED SC_PCIE_DEVICE_OVERRIDE mPcieDeviceTable[] = {
  //
  // Intel PRO/Wireless
  //
  { 0x8086, 0x422b, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x422c, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x4238, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x4239, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel WiMAX/WiFi Link
  //
  { 0x8086, 0x0082, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0085, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0083, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0084, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0086, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0087, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0088, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0089, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x008F, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0090, 0xff, 0xff, 0xff, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel Crane Peak WLAN NIC
  //
  { 0x8086, 0x08AE, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x08AF, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel Crane Peak w/BT WLAN NIC
  //
  { 0x8086, 0x0896, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0897, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel Kelsey Peak WiFi, WiMax
  //
  { 0x8086, 0x0885, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0886, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel Centrino Wireless-N 105
  //
  { 0x8086, 0x0894, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0895, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel Centrino Wireless-N 135
  //
  { 0x8086, 0x0892, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0893, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel Centrino Wireless-N 2200
  //
  { 0x8086, 0x0890, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0891, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel Centrino Wireless-N 2230
  //
  { 0x8086, 0x0887, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x0888, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel Centrino Wireless-N 6235
  //
  { 0x8086, 0x088E, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x088F, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel CampPeak 2 Wifi
  //
  { 0x8086, 0x08B5, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x08B6, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel WilkinsPeak 1 Wifi
  //
  { 0x8086, 0x08B3, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2AndL1SubstatesOverride, 0x0158, 0x0000000F, 0, 0, 0, 0, 0 },
  { 0x8086, 0x08B4, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2AndL1SubstatesOverride, 0x0158, 0x0000000F, 0, 0, 0, 0, 0 },
  //
  // Intel Wilkins Peak 2 Wifi
  //
  { 0x8086, 0x08B1, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2AndL1SubstatesOverride, 0x0158, 0x0000000F, 0, 0, 0, 0, 0 },
  { 0x8086, 0x08B2, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2AndL1SubstatesOverride, 0x0158, 0x0000000F, 0, 0, 0, 0, 0 },
  //
  // Intel Wilkins Peak PF Wifi
  //
  { 0x8086, 0x08B0, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel StonePeak Wifi
  //
  { 0x8086, 0x095A, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x095B, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // Intel StonePeak Wifi 1x1
  //
  { 0x8086, 0x3165, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  { 0x8086, 0x3166, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, ScPcieAspmL1, ScPcieL1L2Override, 0, 0, 0, 0, 0, 0, 0 },
  //
  // End of Table
  //
  { 0 }
};

/**
  Install PCIE Device Table.

  @param[in]  DeviceTable            The pointer to the SC_PCIE_DEVICE_OVERRIDE

  @retval     EFI_SUCCESS            The PPI is installed and initialized.
  @retval     EFI ERRORS             The PPI is not successfully installed.
  @retval     EFI_OUT_OF_RESOURCES   Do not have enough resources to initialize the driver
**/
EFI_STATUS
EFIAPI
InternalInstallPcieDeviceTable (
  IN SC_PCIE_DEVICE_OVERRIDE         *DeviceTable
  )
{
  EFI_PEI_PPI_DESCRIPTOR  *DeviceTablePpiDesc;
  EFI_STATUS               Status;
DEBUG((EFI_D_INFO, "InternalInstallPcieDeviceTable () - Start\n"));
  DeviceTablePpiDesc = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
  ASSERT (DeviceTablePpiDesc != NULL);
  if (DeviceTablePpiDesc == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DeviceTablePpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  DeviceTablePpiDesc->Guid  = &gScPcieDeviceTablePpiGuid;
  DeviceTablePpiDesc->Ppi   = DeviceTable;

  Status = PeiServicesInstallPpi (DeviceTablePpiDesc);
  ASSERT_EFI_ERROR (Status);

  return Status;
}

VOID
ScPolicySetupInit (
  IN CONST EFI_PEI_SERVICES **PeiServices
  );

//[-start-160818-IB03090432-modify]//
VOID
ScInitInterrupt (
  IN CHIPSET_CONFIGURATION  *SystemConfiguration
  );
//[-end-160818-IB03090432-modify]//

#ifndef __GNUC__
#pragma warning (push)
#pragma warning (disable : 4245)
#pragma warning (pop)
#endif

UINT8
ReadCmosBank1Byte (
  IN UINT8                      Address
  )
{
  UINT8                           Data;

  IoWrite8(R_RTC_EXT_INDEX, Address);
  Data = IoRead8 (R_RTC_EXT_TARGET);
  return Data;
}

VOID
WriteCmosBank1Byte (
  IN UINT8                     Address,
  IN UINT8                     Data
  )
{
  IoWrite8(R_RTC_EXT_INDEX, Address);
  IoWrite8(R_RTC_EXT_TARGET, Data);
}

EFI_STATUS
InstallPeiScUsbPolicy (
  IN CONST EFI_PEI_SERVICES **PeiServices
  );

VOID
CheckPowerOffNow (
  VOID
  )
/*++

Routine Description:

  Turn off system if needed.

Arguments:

  PeiServices Pointer to PEI Services
  CpuIo       Pointer to CPU I/O Protocol

Returns:

  None.

--*/
{
  UINT16  Pm1Sts;
  UINT16  AcpiBaseAddr;

  ///
  /// Read ACPI Base Address
  ///
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  //
  // Read and check the ACPI registers
  //
  Pm1Sts = IoRead16 (AcpiBaseAddr + R_ACPI_PM1_STS);
  if ((Pm1Sts & B_ACPI_PM1_STS_PWRBTN) == B_ACPI_PM1_STS_PWRBTN) {
    IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_STS, B_ACPI_PM1_STS_PWRBTN);
    IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_CNT, V_ACPI_PM1_CNT_S5);
    IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_CNT, V_ACPI_PM1_CNT_S5 + B_ACPI_PM1_CNT_SLP_EN);
  }
}

VOID
ClearPowerState (
  IN CHIPSET_CONFIGURATION        *SystemConfiguration
  )
{
  UINT16  Data16;
  UINT32  Data32;
  UINT16  AcpiBaseAddr;

  ///
  /// Read ACPI Base Address
  ///
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  //
  // Check for PowerState option for AC power loss and program the chipset
  //

//[-start-160720-IB03090429-remove]//
//  //
//  // Clear PWROK (Set to Clear)
//  //
//  MmioOr32 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1, B_PMC_GEN_PMCON_PWROK_FLR);
//[-end-160720-IB03090429-remove]//

  //
  // Clear Power Failure Bit (Set to Clear)
  //
  // TODO: Check if it is OK to clear here
  //MmioOr32 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1, B_PMC_GEN_PMCON_SUS_PWR_FLR);

  //
  // Clear the GPE and PM enable
  //
  IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_EN, (UINT16) 0x00);
  IoWrite32 (AcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32) 0x00);

//[-start-160817-IB03090432-modify]//
  //
  // Halt the TCO timer
  //
  Data16 = IoRead16 (AcpiBaseAddr + R_TCO1_CNT);
  Data16 |= B_TCO1_CNT_TMR_HLT;
  IoWrite16 (AcpiBaseAddr + R_TCO1_CNT, Data16);
//[-end-160817-IB03090432-modify]//

  //
  // Before we clear the TO status bit here we need to save the results in a CMOS bit for later use.
  //
  Data32 = IoRead16 (AcpiBaseAddr + R_TCO_STS);
  if ((Data32 & B_TCO_STS_SECOND_TO) == B_TCO_STS_SECOND_TO)
  {
#if (defined(HW_WATCHDOG_TIMER_SUPPORT) && (HW_WATCHDOG_TIMER_SUPPORT != 0))
    WriteCmosBank1Byte (
      EFI_CMOS_PERFORMANCE_FLAGS,
      ReadCmosBank1Byte (EFI_CMOS_PERFORMANCE_FLAGS) | B_CMOS_TCO_WDT_RESET
      );
#endif
  }
  //
  // Now clear the TO status bit (Write '1' to clear)
  //
//[-start-151229-IB03090424-modify]//
#if 0
//[-end-151229-IB03090424-modify]//
  IoWrite32 (AcpiBaseAddr + R_TCO_STS, (UINT32) (Data32 | B_TCO_STS_SECOND_TO));
#endif
}

VOID
ClearSmiAndWake (
  VOID
  )
/*++

Routine Description:

  Clear any SMI status or wake status left over from boot.

Arguments:

Returns:

  None.

--*/
{
  UINT16  Pm1Sts;
  UINT32  Gpe0Sts;
  UINT32  SmiSts;
  UINT16  AcpiBaseAddr;

  ///
  /// Read ACPI Base Address
  ///
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  //
  // Read the ACPI registers
  //
  Pm1Sts  = IoRead16 (AcpiBaseAddr + R_ACPI_PM1_STS);
  Gpe0Sts = IoRead32 (AcpiBaseAddr + R_ACPI_GPE0a_STS);
  SmiSts  = IoRead32 (AcpiBaseAddr + R_SMI_STS);

  //
  // Register Wake up reason for S4.  This information is used to notify
  // WinXp of wake up reason because S4 wake up path doesn't keep SCI.
  // This is important for Viiv(Quick resume) platform.
  //

  //
  // First Clear CMOS S4 Wake up flag.
  //
  WriteCmosBank1Byte(EFI_CMOS_S4_WAKEUP_FLAG_ADDRESS, 0);

  //
  // Check wake up reason and set CMOS accordingly.  Currently checks
  // Power button, USB, PS/2.
  // Note : PS/2 wake up is using GPI13 (IO_PME).  This must be changed depending
  // on board design.
  //
  if ((Pm1Sts & B_ACPI_PM1_STS_PWRBTN) || (Gpe0Sts & (B_ACPI_GPE0a_STS_CORE_GPIO | B_ACPI_GPE0a_STS_SUS_GPIO))) {
    WriteCmosBank1Byte(EFI_CMOS_S4_WAKEUP_FLAG_ADDRESS, 1);
  }

  //
  // Clear any SMI or wake state from the boot
  //
  Pm1Sts = (B_ACPI_PM1_STS_PRBTNOR | B_ACPI_PM1_STS_PWRBTN);

  Gpe0Sts |=
    (
      B_ACPI_GPE0a_STS_CORE_GPIO |
      B_ACPI_GPE0a_STS_SUS_GPIO |
      B_ACPI_GPE0a_STS_PME_B0 |
      B_ACPI_GPE0a_STS_BATLOW |
      B_ACPI_GPE0a_STS_PCI_EXP |
      B_ACPI_GPE0a_STS_GUNIT_SCI |
      B_ACPI_GPE0a_STS_PUNIT_SCI |
      B_ACPI_GPE0a_STS_SWGPE |
      B_ACPI_GPE0a_STS_HOT_PLUG
    );

  SmiSts |=
    (
      B_SMI_STS_PERIODIC |
      B_SMI_STS_TCO |
      B_SMI_STS_SWSMI_TMR |
      B_SMI_STS_APM |
      B_SMI_STS_ON_SLP_EN |
      B_SMI_STS_BIOS
    );

  //
  // Write them back
  //
  IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_STS, Pm1Sts);
  IoWrite32 (AcpiBaseAddr + R_ACPI_GPE0a_STS, Gpe0Sts);
  IoWrite32 (AcpiBaseAddr + R_SMI_STS, SmiSts);
}

//[-start-160429-IB03090426-remove]//
//EFI_STATUS
//PcieSecondaryBusReset (
//  IN CONST EFI_PEI_SERVICES  **PeiServices,
//  IN UINT8             Bus,
//  IN UINT8             Dev,
//  IN UINT8             Fun
//  )
///*++
//
//Routine Description:
//
//  Issue PCI-E Secondary Bus Reset
//
//Arguments:
//
//  Bus - Bus number of the bridge
//  Dev - Devices number of the bridge
//  Fun - Function number of the bridge
//
//
//Returns:
//
//  EFI_SUCCESS
//
//--*/
//{
//  EFI_PEI_STALL_PPI   *PeiStall;
//  EFI_STATUS          Status;
//
//  Status = (**PeiServices).LocatePpi (
//                             PeiServices,
//                             &gEfiPeiStallPpiGuid,
//                             0,
//                             NULL,
//                             (VOID **) &PeiStall
//                             );
//  ASSERT_EFI_ERROR (Status);
//
//  //
//  // Issue secondary bus reset
//  //
//  MmPci16Or(0, Bus, Dev, Fun, PCI_BRIDGE_CONTROL_REGISTER_OFFSET, EFI_PCI_BRIDGE_CONTROL_RESET_SECONDARY_BUS);
//
//  //
//  // Wait 1ms
//  //
//  PeiStall->Stall (PeiServices, PeiStall, 1000);
//
//  //
//  // Clear the reset bit
//  // Note: The PCIe spec suggests 100ms delay between clearing this bit and accessing
//  // the device's config space. Since we will not access the config space until we enter DXE
//  // we don't put delay expressly here.
//  //
//  MmPci16And(0, Bus, Dev, Fun, PCI_BRIDGE_CONTROL_REGISTER_OFFSET, ~(EFI_PCI_BRIDGE_CONTROL_RESET_SECONDARY_BUS));
//
//  return EFI_SUCCESS;
//}
//[-end-160429-IB03090426-remove]//

EFI_STATUS
EFIAPI
IchReset (
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Provide hard reset PPI service.
  To generate full hard reset, write 0x0E to ICH RESET_GENERATOR_PORT (0xCF9).

Arguments:

  PeiServices       General purpose services available to every PEIM.

Returns:

  Not return        System reset occured.
  EFI_DEVICE_ERROR  Device error, could not reset the system.

--*/
{
  IoWrite8 (
    R_RST_CNT,
    V_RST_CNT_HARDSTARTSTATE
    );

  IoWrite8 (
    R_RST_CNT,
    V_RST_CNT_HARDRESET
    );

  //
  // System reset occurred, should never reach at this line.
  //
  ASSERT_EFI_ERROR (EFI_DEVICE_ERROR);

  return EFI_DEVICE_ERROR;
}

VOID
ScPlatformIntInit (
  IN  CONST EFI_PEI_SERVICES    **PeiServices,
  IN CHIPSET_CONFIGURATION       *SystemConfiguration
  )
{
  EFI_BOOT_MODE BootMode;
  UINT8         Data8;

  (*PeiServices)->GetBootMode(PeiServices, &BootMode);

  if ((BootMode != BOOT_ON_S3_RESUME)) {
    //
    // Clear all pending SMI. On S3 clear power button enable so it will not generate an SMI
    //
    ClearSmiAndWake ();
  }

  ClearPowerState (SystemConfiguration);

  //
  // Disable SERR NMI and IOCHK# NMI in port 61
  //
  Data8 = IoRead8 (R_NMI_SC);
  IoWrite8(R_NMI_SC, (UINT8) (Data8 | B_NMI_SC_PCI_SERR_EN | B_NMI_SC_IOCHK_NMI_EN));

}

VOID
IchRcrbInit (
  IN CONST EFI_PEI_SERVICES      **PeiServices,
  IN CHIPSET_CONFIGURATION        *SystemConfiguration
  )
{

  EFI_BOOT_MODE                   BootMode;

  (*PeiServices)->GetBootMode(PeiServices, &BootMode);

  //
  // Disable the Watchdog timer expiration from causing a system reset
  //
  MmioOr8(PMC_BASE_ADDRESS + R_PMC_PM_CFG, B_PMC_PM_CFG_NO_REBOOT);

  //
  // HPET is enabled in ScInitPeim.c
  //
}

//
// Returns a pointer to the memory-mapped capability
//
UINTN
MmPciCapability (
  IN UINT16           Segment,
  IN UINT16           Bus,
  IN UINT16           Device,
  IN UINT16           Function,
  IN UINT8            CapabilityId
  )
{
  UINT8 Capability;
  UINTN PciDeviceBase = PCI_EXPRESS_LIB_ADDRESS(Bus, Device, Function, 0x0);

  // Check that capabilities are flagged as enabled in the PCI status register
  ASSERT(EFI_PCI_STATUS_CAPABILITY ==
    (EFI_PCI_STATUS_CAPABILITY & PciExpressRead8(PciDeviceBase + PCI_PRIMARY_STATUS_OFFSET)));

  // Get the config space-relative offset of the capabilities list from the PCI capabilities pointer
  Capability = PciExpressRead8(PciDeviceBase + 0x34);  // Pointer to the first element of the capability linked list

  // Walk the linked list of capabilities
  while (Capability) {  // List is terminated by a next ptr == 0x0
    if (CapabilityId == PciExpressRead8(PciDeviceBase + Capability + 0x0)) { // ID is at offset 0
      return (PciDeviceBase + Capability);
    }
    Capability = PciExpressRead8(PciDeviceBase + Capability + 0x1); // Next pointer is at offset 1
  }

  return EFI_NOT_FOUND;
}

//[-start-160818-IB03090432-remove]//
///*++
//
//  Routine Description:
//
//  Setup PCICFGCTRL registers at each device's respective sideband port and
//  set the interrupt polarity for each corresponding IRQ's respective bit in
//  ITSS (ITSS.IPC0, ITSS.IPC1, etc.)
//
//  @param  CHIPSET_CONFIGURATION   System Configuration Table
//
//  @retval None.
//
//--*/
//VOID
//ScInitInterrupt(
//  IN CHIPSET_CONFIGURATION  *SystemConfiguration
//)
//{
//  UINT32  Index = 0;
//  UINTN   P2SBPciMmBase = 0;
//  UINTN   XhciPciMmBase = 0;
//  UINTN   PunitPciMemBase = 0;
//  UINTN   PciD0F0RegBase = 0;
//  UINTN   MchBar = 0;
//
//  //
//  // General Interrupt Requirements for BXT South Cluster Devices
//  //
//  // 1.) Define device sideband endpoint port, PCICFGCTRL reg offset,
//  //     and IRQ and interrupt pin in directIrqTable.
//  //
//  // Reference BXT Interrupt HAS for device interrupt pin requirements.
//  //
//  // 2.) Set device IRQ in PciTree.asl (IntPin should match the value in directIrqTable).
//  //
//  // 3.) Reference BXT Interrupt HAS Excel sheet for IRQ polarity and ensure value is correct
//  //     in ITSS.IPC0, ITSS.IPC1, etc. register programming below.
//  //
//  // 4.) Add BDF with IRQ for the device in IrqInitTable in ScInit.c.
//  //     This value is written to the interrupt line register (offset 0x3C) in the device's
//  //     PCI CFG space.
//  //
//  // 5.) Write the Master Message Enable bit of ITSS to enable the interrupt transaction
//  //     to IOSF
//  //
//  // Note: MSIs are set by OS device drivers and typically require no IAFW configuration.
//  //
//
//  DEBUG ((EFI_D_INFO, "ScInitInterrupt () - Start\n"));
//
//  P2SBPciMmBase = MmPciAddress (
//                    0,
//                    DEFAULT_PCI_BUS_NUMBER_SC,
//                    PCI_DEVICE_NUMBER_P2SB,
//                    PCI_FUNCTION_NUMBER_P2SB,
//                    0
//                    );
//
//  XhciPciMmBase = MmPciAddress(
//                    0,
//                    DEFAULT_PCI_BUS_NUMBER_SC,
//                    PCI_DEVICE_NUMBER_XHCI,
//                    PCI_FUNCTION_NUMBER_XHCI,
//                    0
//                    );
//
//  //
//  // Writing the Master Message Enable (bit 0) of ITSS Register 0x3334
//  //
//  SideBandAndThenOr32 (SB_ITSS_PORT, R_ITSS_SB_MMC, 0xFFFFFFFF, BIT0);
//
//  //
//  // Ensure HPET and IOAPIC are enabled in P2SB PCI CFG space
//  // 1) HPET   -> Register 0x60 bit 7
//  // 2) IOAPIC -> Register 0x64 bit 8
//  //
//  MmioOr16 ((UINTN)MmioAddress (P2SBPciMmBase, R_P2SB_HPET), (UINT16)(BIT7));
//  MmioOr16 ((UINTN)MmioAddress (P2SBPciMmBase, R_P2SB_IOAC), (UINT16)(BIT8));
//
//  // Set Max Writes Pending (Max Number of Writes Allowed on IOSF-SB to SBREG_BAR)
//  MmioWrite32 ((UINTN)MmioAddress (P2SBPciMmBase, R_P2SB_P2SBC), (UINT32)0x07);
//
//  // Set MemSpaceEn and BusInitiate in P2SB Command Register
//  MmioWrite32 ((UINTN)MmioAddress (P2SBPciMmBase, R_P2SB_STSCMD), (UINT32)0x06);
//
//  //
//  // Set PCICFGCTRL register in corresponding SB port for each device in directIrqTable
//  //
//  for (Index = 0; Index < sizeof(directIrqTable) / sizeof(directIrqTable[0]); Index++) {
//    SideBandWrite32 (
//      directIrqTable[Index].Port,
//      directIrqTable[Index].PciCfgOffset,
//      (directIrqTable[Index].PciIrqNumber << N_PCICFGCTRL_PCI_IRQ) +
//      (directIrqTable[Index].IrqPin << N_PCICFGCTRL_INT_PIN)
//      );
//  }
//
//  //
//  // Set xHCI Interrupt Pin
//  // Inaccessible after XHCC1.ACCTRL is set (lock down bit)
//  //
//  MmioWrite8 ((UINTN)(XhciPciMmBase + R_XHCI_INT_PN), (UINT8)V_XHCI_INT_PN);
//
//  //
//  // Set ACPIIN / ACPIIE For HD Audio Device
//  //
//  DEBUG((EFI_D_INFO, "Disable shadowed PCI config space.\n"));
//  SideBandAndThenOr32 (
//    SB_AUDIO_PORT,
//    R_HDA_PCICFGCTL,
//    0xFFFFFFFF,
//    (UINT32)((V_HDA_INTLN << 8) | B_HDA_PCICFGCTL_SPCBAD | B_HDA_PCICFGCTL_ACPIIE)
//    );
//
//    PunitPciMemBase = MmPciAddress (
//                        0,
//                        DEFAULT_PCI_BUS_NUMBER_SC,
//                        0,
//                        1,
//                        0
//                        );
//   //
//   // Set PUNIT Interrupt Pin
//   // Inaccessible after PUINT_INTPIN_LOCK is set (lock down bit)
//   //
//   MmioWrite8 ((UINTN)(PunitPciMemBase + 0x3D), (UINT8)BIT1);
//   DEBUG((EFI_D_INFO, "PunitPciMemBase = 0x%x.\n",PunitPciMemBase));
//   DEBUG((EFI_D_INFO, "PUNIT Interrupt Pin = 0x%x.\n",MmioRead8((UINTN)(PunitPciMemBase + 0x3D))));
//
//   //
//   // Set PUINT IRQ to 24 and INTPIN LOCK
//   //
//   PciD0F0RegBase  = MmPciAddress (0,0,0,0,0);
//   MchBar          = MmioRead32 (PciD0F0RegBase + 0x48) &~BIT0;
//   MmioWrite32 (MchBar + P_CR_THERMAL_DEVICE_IRQ_0_0_0_MCHBAR_PUNIT_REG, V_PUINT_INTLN + PUINT_INTPIN_LOCK);
//
//   DEBUG((EFI_D_INFO, "PciD0F0RegBase = 0x%x.\n", PciD0F0RegBase));
//   DEBUG((EFI_D_INFO, "MchBar = 0x%x.\n", MchBar));
//   DEBUG((EFI_D_INFO, "P_CR_THERMAL_DEVICE_IRQ_0_0_0_MCHBAR_PUNIT_REG = 0x%x.\n", MmioRead32 (MchBar + P_CR_THERMAL_DEVICE_IRQ_0_0_0_MCHBAR_PUNIT_REG)));
//
//  //
//  // Program Legacy Interrupt Routing Registers
//  // ITSS private registers are only accessible via DWORD aligned IOSF-SB CrWr messages
//  //
//  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PARC, (UINT32)
//    (((V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ3) << 0) +  // R_ITSS_SB_PARC    PIRQA->IRQx Routing Control
//    ((V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ4) << 8) +   // R_ITSS_SB_PBRC    PIRQB->IRQx Routing Control
//    ((V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ5) << 16) +  // R_ITSS_SB_PCRC    PIRQC->IRQx Routing Control
//    ((V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ6) << 24))   // R_ITSS_SB_PDRC    PIRQD->IRQx Routing Control
//    );
//  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PERC, (UINT32)
//    (((V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ7) << 0) +  // R_ITSS_SB_PERC    PIRQE->IRQx Routing Control
//    ((V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ9) << 8) +   // R_ITSS_SB_PFRC    PIRQF->IRQx Routing Control
//    ((V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ10) << 16) + // R_ITSS_SB_PGRC    PIRQG->IRQx Routing Control
//    ((V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ11) << 24))  // R_ITSS_SB_PHRC    PIRQH->IRQx Routing Control
//    );
//
//  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PIR0, (UINT32)
//    (((V_ITSS_SB_IAR_PIRQA + V_ITSS_SB_IBR_PIRQA + V_ITSS_SB_ICR_PIRQA + V_ITSS_SB_IDR_PIRQA) << 0) +  //  R_ITSS_SB_PIR0   Device 31 Pin->PIRQx Routing
//    ((V_ITSS_SB_IAR_PIRQH + V_ITSS_SB_IBR_PIRQH + V_ITSS_SB_ICR_PIRQH + V_ITSS_SB_IDR_PIRQH) << 16))   //  R_ITSS_SB_PIR1   Device 29 Pin->PIRQx Routing
//    );
//  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PIR2, (UINT32)
//    (((V_ITSS_SB_IAR_PIRQD + V_ITSS_SB_IBR_PIRQD + V_ITSS_SB_ICR_PIRQD + V_ITSS_SB_IDR_PIRQD) << 0) +  //  R_ITSS_SB_PIR2   Device 28 Pin->PIRQx Routing
//    ((V_ITSS_SB_IAR_PIRQF + V_ITSS_SB_IBR_PIRQF + V_ITSS_SB_ICR_PIRQF + V_ITSS_SB_IDR_PIRQF) << 16))   //  R_ITSS_SB_PIR3   Device 23 Pin->PIRQx Routing
//    );
//  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PIR4, (UINT32)
//    (((V_ITSS_SB_IAR_PIRQA + V_ITSS_SB_IBR_PIRQA + V_ITSS_SB_ICR_PIRQA + V_ITSS_SB_IDR_PIRQA) << 0) +  //  R_ITSS_SB_PIR4   Device 22 Pin->PIRQx Routing
//    ((V_ITSS_SB_IAR_PIRQE + V_ITSS_SB_IBR_PIRQE + V_ITSS_SB_ICR_PIRQE + V_ITSS_SB_IDR_PIRQE) << 16))   //  R_ITSS_SB_PIR5   Device 20 Pin->PIRQx Routing
//    );
//  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PIR6, (UINT32)
//    (((V_ITSS_SB_IAR_PIRQG + V_ITSS_SB_IBR_PIRQG + V_ITSS_SB_ICR_PIRQG + V_ITSS_SB_IDR_PIRQG) << 0) +  //  R_ITSS_SB_PIR6   Device    Pin->PIRQx Routing
//    ((V_ITSS_SB_IAR_PIRQB + V_ITSS_SB_IBR_PIRQB + V_ITSS_SB_ICR_PIRQB + V_ITSS_SB_IDR_PIRQB) << 16))   //  R_ITSS_SB_PIR7   Device    Pin->PIRQx Routing
//    );
//  SideBandWrite32(SB_ITSS_PORT, R_ITSS_SB_PIR8, (UINT32)
//    (((V_ITSS_SB_IAR_PIRQC + V_ITSS_SB_IBR_PIRQC + V_ITSS_SB_ICR_PIRQC + V_ITSS_SB_IDR_PIRQC) << 0) +  //  R_ITSS_SB_PIR8   Pin->PIRQx Routing
//    ((V_ITSS_SB_IAR_PIRQD + V_ITSS_SB_IBR_PIRQD + V_ITSS_SB_ICR_PIRQD + V_ITSS_SB_IDR_PIRQD) << 16))   //  R_ITSS_SB_PIR9   Pin->PIRQx Routing
//    );
//  SideBandWrite32(SB_ITSS_PORT, R_ITSS_SB_PIR10, (UINT32)
//    (((V_ITSS_SB_IAR_PIRQE + V_ITSS_SB_IBR_PIRQE + V_ITSS_SB_ICR_PIRQE + V_ITSS_SB_IDR_PIRQE) << 0) +  //  R_ITSS_SB_PIR10  Pin->PIRQx Routing
//    (0))                                                                                               //  R_ITSS_SB_PIR11  Pin->PIRQx Routing
//    );
//
//  //
//  // xDCI uses IRQn in BXTM - This write is not needed in BXTM but left as reference for BXTP.
//  //
//  /*
//  SideBandWrite32(SB_ITSS_PORT, R_ITSS_SB_PIR8, (UINT32)
//    (((V_ITSS_SB_IAR_PIRQC + V_ITSS_SB_IBR_PIRQC + V_ITSS_SB_ICR_PIRQC + V_ITSS_SB_IDR_PIRQC) << 0) +  //  R_ITSS_SB_PIR8   Device    Pin->PIRQx Routing
//    (0))                                                                                               //  R_ITSS_SB_PIR9   NULL    Pin->PIRQx Routing
//    );
//  */
//
//  //
//  // Configure Interrupt Polarity
//  //
//  // Reference "BXT Interrupt Mapping" HAS for polarity definitions.
//  //
//  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_IPC0, (UINT32)
//    ((V_ITSS_SB_IPC_ACTIVE_LOW << 31) + // IRQ 31
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 30) +  // IRQ 30
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 29) +  // IRQ 29
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 28) +  // IRQ 28
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 27) +  // IRQ 27
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 26) +  // IRQ 26
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 25) +  // IRQ 25
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 24) +  // IRQ 24
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 23) +  // IRQ 23
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 22) +  // IRQ 22
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 21) +  // IRQ 21
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 20) +  // IRQ 20
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 19) +  // IRQ 19
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 18) +  // IRQ 18
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 17) +  // IRQ 17
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 16) +  // IRQ 16
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 15) +  // IRQ 15
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 14) +  // IRQ 14
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 13) +  // IRQ 13
//    (V_ITSS_SB_IPC_ACTIVE_HIGH << 12) + // IRQ 12
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 11) +  // IRQ 11
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 10) +  // IRQ 10
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 9) +   // IRQ  9
//    (V_ITSS_SB_IPC_ACTIVE_HIGH << 8) +  // IRQ  8
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 7) +   // IRQ  7
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 6) +   // IRQ  6
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 5) +   // IRQ  5
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 4) +   // IRQ  4
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 3) +   // IRQ  3
//    (V_ITSS_SB_IPC_ACTIVE_HIGH << 2) +  // IRQ  2
//    (V_ITSS_SB_IPC_ACTIVE_HIGH << 1) +  // IRQ  1
//    (V_ITSS_SB_IPC_ACTIVE_HIGH << 0))   // IRQ  0
//    );
//  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_IPC1, (UINT32)
//    ((V_ITSS_SB_IPC_ACTIVE_LOW << 31) + // IRQ 63
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 30) +  // IRQ 62
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 29) +  // IRQ 61
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 28) +  // IRQ 60
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 27) +  // IRQ 59
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 26) +  // IRQ 58
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 25) +  // IRQ 57
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 24) +  // IRQ 56
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 23) +  // IRQ 55
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 22) +  // IRQ 54
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 21) +  // IRQ 53
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 20) +  // IRQ 52
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 19) +  // IRQ 51
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 18) +  // IRQ 50
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 17) +  // IRQ 49
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 16) +  // IRQ 48
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 15) +  // IRQ 47
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 14) +  // IRQ 46
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 13) +  // IRQ 45
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 12) +  // IRQ 44
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 11) +  // IRQ 43
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 10) +  // IRQ 42
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 9) +   // IRQ 41
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 8) +   // IRQ 40
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 7) +   // IRQ 39
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 6) +   // IRQ 38
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 5) +   // IRQ 37
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 4) +   // IRQ 36
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 3) +   // IRQ 35
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 2) +   // IRQ 34
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 1) +   // IRQ 33
//    (V_ITSS_SB_IPC_ACTIVE_LOW << 0))    // IRQ 32
//    );
//
//  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_IPC2, 0xFFFFFFFF); //  95-64 are active low (PCI)
//  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_IPC3, 0xFFFFFFFF); // 119-96 are active low (PCI)
//
//  DEBUG ((EFI_D_INFO, "ScInitInterrupt () - End\n"));
//}
//[-end-160818-IB03090432-remove]//

//[-start-151229-IB03090424-modify]//
//[-start-160406-IB07400715-remove]//
//#if (BXTI_PF_ENABLE == 1)
//[-end-160406-IB07400715-remove]//
EFI_STATUS
ScPlatformSccDllOverride (
  VOID
)
{
  UINTN          SccPciMmBase;
  UINT32         SccMmioBase0;
  SI_POLICY_PPI  *SiPolicy;
  EFI_STATUS     Status;

  //
  // Obtain Platform Info from HOB.
  //
  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPolicy
             );
  ASSERT_EFI_ERROR (Status);

  SccMmioBase0 = (UINT32) SiPolicy->TempMemBaseAddr;

  //
  // Override DLL settings
  //
  SccPciMmBase = MmPciBase (
                   DEFAULT_PCI_BUS_NUMBER_SC,
                   PCI_DEVICE_NUMBER_SCC_EMMC,
                   PCI_FUNCTION_NUMBER_SCC_FUNC0
                   );
  MmioWrite32 ((UINTN) (SccPciMmBase + R_SCC_BAR), (UINT32) (SccMmioBase0 & B_SCC_BAR_BA));
  MmioOr32    ((UINTN) (SccPciMmBase + R_SCC_STSCMD), (UINT32) (B_SCC_STSCMD_BME | B_SCC_STSCMD_MSE));
  MmioWrite32 (SccMmioBase0 + R_SCC_MEM_TX_DATA_DLL_CNTL1,     0xC13);
  MmioWrite32 (SccMmioBase0 + R_SCC_MEM_TX_DATA_DLL_CNTL2,     0x1c2a2929);
  MmioWrite32 (SccMmioBase0 + R_SCC_MEM_RX_CMD_DATA_DLL_CNTL1, 0x1d7777);
  MmioWrite32 (SccMmioBase0 + R_SCC_MEM_RX_STROBE_DLL_CNTL,    0x0c0c);
  MmioAnd32   ((UINTN) (SccPciMmBase + R_SCC_STSCMD), (UINT32) ~(B_SCC_STSCMD_BME | B_SCC_STSCMD_MSE));
  MmioWrite32 ((UINTN) (SccPciMmBase + R_SCC_BAR), (UINT32) (0x00));

  return EFI_SUCCESS;
}

//[-start-160517-IB03090427-add]//
VOID
ScUsb2PhyOverride (
  VOID
)
{
  UINT32   Data;

//[-start-160802-IB03090430-modify]//
  Data = SideBandRead32(USB2_PHY_PORT_ID, 0x402C); //USB2_GLOBAL_PORT_2 register
  Data = Data & 0xFFFFFC7F; //set SENSECOMP_DISABLE (bit 7), IDDIGCOMP_DISABLE (bit 8), VBUSCOMP_DISABLE (bit 9) to zero
  SideBandWrite32(USB2_PHY_PORT_ID, 0x402C, Data);
  Data = SideBandRead32(USB2_PHY_PORT_ID, 0x402C);
  DEBUG((EFI_D_INFO, "USB2_GLOBAL_PORT_2 register value after ScUsb2PhyOverride = 0x%x.\n", Data));
//[-end-160802-IB03090430-modify]//
}
//[-end-160517-IB03090427-add]//
//[-start-161018-IB06740518-add]//
//
// Doc#570618 Rev 1.4, Section 2.5, Sample Code to Enable RT Mode in BIOS  
//
VOID
ScInitRT (
//[-start-161022-IB07400802-modify]//
  IN CHIPSET_CONFIGURATION  *SystemConfiguration
//[-end-161022-IB07400802-modify]//
)
{
  UINTN   PciD0F0RegBase = 0;
  UINTN   MchBar = 0;
  UINT32   Data;
  UINT16  StallCount;

  DEBUG((EFI_D_INFO, "ScInitRT() - Start\n"));

  PciD0F0RegBase  = MmPciAddress (0,0,0,0,0);
  MchBar          = MmioRead32 (PciD0F0RegBase + 0x48) &~BIT0;

  if (SystemConfiguration->RTEn != 0) { //RT is enabled
    //ISM disable
    Data = SideBandRead32(SB_PORTID_PSF0, R_PCH_PCR_PSF_GLOBAL_CONFIG); //PSF_0_PSF_GLOBAL_CONFIG
//[-start-180515-IB07400966-modify]//
    //
    // Workaround for S3 resume issue (IPS case:00238352)
    //
    Data = (Data & 0xFFFFFF7B) | 0x80; //7:7 -enEEI (EEI Enable)=1, 2:2 -IdleNAK (Agent Idle NAK)=1
//[-end-180515-IB07400966-modify]//
    SideBandWrite32(SB_PORTID_PSF0, R_PCH_PCR_PSF_GLOBAL_CONFIG, Data);
    Data = SideBandRead32(SB_PORTID_PSF0, R_PCH_PCR_PSF_GLOBAL_CONFIG);
    DEBUG((EFI_D_INFO, "PSF_0_PSF_GLOBAL_CONFIG = 0x%x.\n", Data));

    //Realtime settings
    Data = SideBandRead32(SB_PORTID_PSF1, 0x405C); //PSF_1_CHANMAP_SC_PG0_PORT0_CHAN1_RS0_CHMAP2
    Data = Data & 0xFFFF0FFF; //12:15 -SC_PG1_P1_CH0,1,2,3 (Source Channel)
    SideBandWrite32(SB_PORTID_PSF1, 0x405C, Data);
    Data = SideBandRead32(SB_PORTID_PSF1, 0x405C);
    DEBUG((EFI_D_INFO, "PSF_1_CHANMAP_SC_PG0_PORT0_CHAN1_RS0_CHMAP2 = 0x%x.\n", Data));

    //IDI Real-Time Feature Configuration Bits settings
    Data = MmioRead32(MchBar+0x6D78);
    if (SystemConfiguration->RTEn == 2) { //AGENT_DID1
      DEBUG((EFI_D_INFO, "Set AGENT DID1 \n\r"));
      Data = (Data & 0xFFFCFFFE) | 0x20001; //RT_IDI_AGENT (bit 17:16) = 1'b10,  RT_ENABLE (bit 0) = 1
    }
    else if (SystemConfiguration->RTEn == 1) { //AGENT_DID0
      DEBUG((EFI_D_INFO, "Set AGENT DID0 \n\r"));
      Data = (Data & 0xFFFCFFFE) | 0x10001; //RT_IDI_AGENT (bit 17:16) = 1'b01,  RT_ENABLE (bit 0) = 1
    }
    else { //SystemConfiguration->RTEn == 3 AGENT_DIDX
      DEBUG((EFI_D_INFO, "Set AGENT Disabled \n\r"));
      Data = (Data & 0xFFFCFFFE) | 0x00001; //RT_IDI_AGENT (bit 17:16) = 1'b00,  RT_ENABLE (bit 0) = 1
    }
    MmioWrite32 (MchBar+0x6D78, Data);
    Data = MmioRead32(MchBar+0x6D78);
    DEBUG((EFI_D_INFO, "B_CR_RT_EN_0_0_0_MCHBAR = 0x%x\n\r", Data));

//[-start-161101-IB03090435-remove]//
//    //isoc settings
//    Data = SideBandRead32(0x4C, 0x4C4C); //B_CR_BCOSCAT
//    Data = (Data & 0xFFFFFF3F) | 0x40; //7:6 -COS_CAT_AGENT3 (COS Category for Agent3)=1
//    SideBandWrite32(0x4C, 0x4C4C, Data);
//    Data = SideBandRead32(0x4C, 0x4C4C);
//    DEBUG((EFI_D_INFO, "B_CR_BCOSCAT = 0x%x\n\r", Data));
//
//    Data = SideBandRead32(0x4C, 0x4C3C); //B_CR_SCH_AC_MAPPING1
//    Data = (Data & 0xFFFFFFF0) | 0x1; //3:0 -CHANNEL0_AC (Channel 0 Access Class)=1
//    SideBandWrite32(0x4C, 0x4C3C, Data);
//    Data = SideBandRead32(0x4C, 0x4C3C);
//    DEBUG((EFI_D_INFO, "B_CR_SCH_AC_MAPPING1 = 0x%x\n\r", Data));
//[-end-161101-IB03090435-remove]//

    //punit to stop all memory subsystem power management
    StallCount = 0;
    while (StallCount < 1000) {
      Data = MmioRead32 (MchBar + 0x7084); //read P_CR_BIOS_MAILBOX_INTERFACE_0_0_0_MCHBAR
      if ((Data & BIT31) == BIT31) {  //check RUN_BUSY bit
        MicroSecondDelay (1);         //add 1 microsecond delay
      } else {
        break;
      }
      StallCount++;
    }
    MmioWrite32 ( (MchBar + 0x7080), 0x80008000);  //P_CR_BIOS_MAILBOX_DATA_0_0_0_MCHBAR
//[-start-161101-IB03090435-modify]//
    MmioWrite32 ( (MchBar + 0x7084), (BIT31 | 0x21));  //P_CR_BIOS_MAILBOX_INTERFACE_0_0_0_MCHBAR - Set the Run_Busy bit to signal mailbox data is ready to process
//[-end-161101-IB03090435-modify]//
    Data = MmioRead32 (MchBar + 0x7084); DEBUG((EFI_D_INFO, "P_CR_BIOS_MAILBOX_INTERFACE_0_0_0_MCHBAR = 0x%x\n\r", Data));

    //punit to set soc.sa.bunit.b_cr_bcoscat.cos_cat_agent3 and soc.sa.bunit.b_cr_sch_ac_mapping1.channel0_ac
    StallCount = 0;
    while (StallCount < 1000) {
      Data = MmioRead32 (MchBar + 0x7084); //read P_CR_BIOS_MAILBOX_INTERFACE_0_0_0_MCHBAR
      if ((Data & BIT31) == BIT31) {  //check RUN_BUSY bit
        MicroSecondDelay (1);         //add 1 microsecond delay
      } else {
        break;
      }
      StallCount++;
    }
    MmioWrite32 ( (MchBar + 0x7080), 0x00000001);  //P_CR_BIOS_MAILBOX_DATA_0_0_0_MCHBAR
    MmioWrite32 ( (MchBar + 0x7084), (BIT31 | 0x38));  //P_CR_BIOS_MAILBOX_INTERFACE_0_0_0_MCHBAR

  }

  DEBUG((EFI_D_INFO, "ScInitRT() - End\n"));
}
//[-end-161018-IB06740518-add]//
//[-start-160406-IB07400715-remove]//
//#endif
//[-end-160406-IB07400715-remove]//
//[-end-151229-IB03090424-modify]//

EFI_STATUS
PlatformScInit (
  IN CHIPSET_CONFIGURATION        *SystemConfiguration,
  IN CONST EFI_PEI_SERVICES      **PeiServices,
  IN UINT16                      PlatformType
  )
{
//[-start-160216-IB03090424-modify]//
  EFI_STATUS Status;
  BXT_SERIES BxtSeries;
#ifndef FSP_WRAPPER_FLAG
  IchRcrbInit (PeiServices, SystemConfiguration);
#endif
  //
  // SC Policy Initialization based on Setup variable.
  //
  ScPolicySetupInit (PeiServices);

  //
  // Install PCIe device override table
  //
  BxtSeries = GetBxtSeries ();
  if (BxtSeries == BxtP || (BxtSeries == Bxt1)) {
    Status = InternalInstallPcieDeviceTable (mPcieDeviceTable);
    ASSERT_EFI_ERROR (Status);
  }
  if (BxtSeries == BxtP) {
    //
    // Set TCO Base Address
    //
#if (VP_BIOS_ENABLE == 0)
    SetTcoBase (PcdGet16 (PcdTcoBaseAddress));
#endif
  }
#ifndef FSP_WRAPPER_FLAG
  ScPlatformIntInit (PeiServices, SystemConfiguration);
#endif

//[-start-160517-IB03090427-add]//
  PmicVhostControl(TRUE);
//[-end-160517-IB03090427-add]//

//[-start-161022-IB07400803-modify]//
//#if (BXTI_PF_ENABLE == 1)
  if (IsIOTGBoardIds()) {
    ScInitRT(SystemConfiguration);  //Real Time
  }  
//#endif
//[-end-161022-IB07400803-modify]//

  return EFI_SUCCESS;
//[-end-160216-IB03090424-modify]//
}

VOID
ScPolicySetupInit (
  IN CONST EFI_PEI_SERVICES   **PeiServices
  )
{
  EFI_STATUS                  Status;
  SC_POLICY_PPI               *ScPolicyPpi;

  //
  // Install SC Policy PPI. As we depend on SC Init PPI so we are executed after
  // ScInit PEIM. Thus we can insure SC Initialization is performed when we install the SC Policy PPI,
  // as ScInit PEIM registered a notification function on our policy PPI.
  //
  // For better code structure / modularity, we should use a notification function on SC Init PPI to perform
  // actions that depend on ScInit PEIM's initialization.
  //
  DEBUG ((EFI_D_INFO, "ScPolicySetupInit() - Start\n"));
//[-start-160216-IB03090424-modify]//
  ScCreateConfigBlocks (&ScPolicyPpi);
//[-end-160216-IB03090424-modify]//

//[-start-160317-IB03090425-remove]//
//#if (TABLET_PF_ENABLE == 0)
//  //
//  // Install Verb Table
//  //
//  InstallPlatformVerbTablesEx ();
//#endif
//[-end-160317-IB03090425-remove]//

  UpdatePeiScPolicy(ScPolicyPpi);
//[-start-151216-IB11270137-add]//
  //
  // OemServices
  //
  Status = OemSvcUpdateScPlatformPolicy (ScPolicyPpi);
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "PeiOemSvcChipsetLib OemSvcUpdateScPlatformPolicy, Status : %r\n", Status));
  ASSERT ( ( Status == EFI_SUCCESS ) || ( Status == EFI_UNSUPPORTED ) || ( Status == EFI_MEDIA_CHANGED ) );
//[-end-151216-IB11270137-add]//
  //
  // Install SC Policy PPI
  //
  Status = ScInstallPolicyPpi (ScPolicyPpi);
  ASSERT_EFI_ERROR (Status);

  DEBUG((EFI_D_INFO, "ScPolicySetupInit() - End\n"));
}


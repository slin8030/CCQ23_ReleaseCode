/** @file
  Initializes SC PMC Devices

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
#include "ScInit.h"
#ifndef FSP_FLAG
#include <Protocol/PciEnumerationComplete.h>
#endif

#ifdef FSP_FLAG
/**
  Pmc devices final initialization.

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
ConfigurePmcAtBoot (
  IN SC_POLICY_HOB                      *ScPolicy
  )
{
  UINTN     PmcBaseAddr;
  UINT16    AcpiBaseAddr;
  UINT32    Data32;

  DEBUG ((DEBUG_INFO, "ConfigurePmcAtBoot() Start\n"));

  DEBUG ((DEBUG_INFO, "Override Broxton PMC I/O BAR (bus 0, device 13, function 1) bar2(offset 0x20) to PcdScAcpiIoPortBaseAddress(0x400)\n"));
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  PmcBaseAddr  = MmPciBase (
                   DEFAULT_PCI_BUS_NUMBER_SC,
                   PCI_DEVICE_NUMBER_PMC,
                   PCI_FUNCTION_NUMBER_PMC
                   );
  MmioWrite32 (PmcBaseAddr + R_PMC_ACPI_BASE, AcpiBaseAddr);

  DEBUG ((DEBUG_INFO, "Set PMC IO enable bit (Command register (BIT2 | BIT1 | BIT0)) after PCI enumeration\n"));
  MmioOr32 (PmcBaseAddr + PCI_COMMAND_OFFSET, (EFI_PCI_COMMAND_BUS_MASTER | EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_IO_SPACE));

  SideBandAndThenOr32(
    SB_PORTID_PSF3,
    R_PCH_PCR_PSF3_T0_SHDW_PMC_PMC_REG_BASE + R_PCH_PCR_PSFX_T0_SHDW_PCIEN,
    0xFFFFFFFF,
    (B_PCH_PCR_PSFX_T0_SHDW_PCIEN_MEMEN | B_PCH_PCR_PSFX_T0_SHDW_PCIEN_IOEN)
    );

  //
  // Switch the PMC IPC1 to ACPI mode.
  //
  SideBandAndThenOr32(
    SB_PORTID_PSF3,
    R_PCH_PCR_PSF3_T0_SHDW_PMC_PMC_REG_BASE + R_PCH_PCR_PSFX_T0_SHDW_CFG_DIS,
    0xFFFFFFFF,
    B_PCH_PCR_PSFX_T0_SHDW_CFG_DIS_CFGDIS
    );

  //
  // Switch the PMC SSRAM to ACPI mode.
  //
  SideBandAndThenOr32(
    SB_PMC_IOSF2OCP_PORT,
    R_PMC_IOSF2OCP_PCICFGCTRL3,
    0xFFFFFFFF,
    B_PCH_PCR_PSFX_T0_SHDW_CFG_DIS_CFGDIS | B_PMC_IOSF2OCP_PCICFGCTRL3_BAR1_DISABLE3
    );

  //
  // Program PMC ACPI IRQ.
  //
  Data32 = SideBandRead32(SB_PMC_IOSF2OCP_PORT, R_PMC_IOSF2OCP_PCICFGCTRL2);
  Data32 |= BIT1;
  Data32 |= 0x00028000;
  SideBandWrite32(SB_PMC_IOSF2OCP_PORT, R_PMC_IOSF2OCP_PCICFGCTRL2, Data32);

  Data32 = SideBandRead32(SB_PMC_IOSF2OCP_PORT, R_PMC_IOSF2OCP_PCICFGCTRL2);
  DEBUG((DEBUG_INFO, "  PMC Interrupt value= %x \n ", Data32));


  DEBUG ((DEBUG_INFO, "ConfigurePmcAtBoot() End\n"));

  return EFI_SUCCESS;
}
#else
/**
  Set PMC IO enable bit (Command register (BIT2 | BIT1 | BIT0)) after PCI enumeration

  @param[in] Event                      The Event that is being processed
  @param[in] Context                    Event Context
**/
VOID
EFIAPI
PmcPciEnumeratedCompleteNotify (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  )
{
  UINTN     PmcBaseAddr;
  UINT16    AcpiBaseAddr;
  UINT32    IpcBaseAddr;

  DEBUG ((DEBUG_INFO, "Override Broxton PMC(bus 0, device 13, function 1) bar2(offset 0x20) to PcdScAcpiIoPortBaseAddress(0x400)\n"));
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  PmcBaseAddr  = MmPciBase (
                   DEFAULT_PCI_BUS_NUMBER_SC,
                   PCI_DEVICE_NUMBER_PMC,
                   PCI_FUNCTION_NUMBER_PMC
                   );
  MmioWrite32 (PmcBaseAddr + R_PMC_ACPI_BASE, AcpiBaseAddr);

  DEBUG ((DEBUG_INFO, "Set PMC IO enable bit (Command register (BIT2 | BIT1 | BIT0)) after PCI enumeration\n"));
  MmioOr32 (PmcBaseAddr + PCI_COMMAND_OFFSET, (EFI_PCI_COMMAND_BUS_MASTER | EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_IO_SPACE));

  IpcBaseAddr = MmioRead32(PmcBaseAddr + R_PMC_BASE);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (PmcBaseAddr + R_PMC_BASE),
    1,
    &IpcBaseAddr
    );

  DEBUG((DEBUG_INFO, "PmcPciEnumeratedCompleteNotify IpcBaseAddr = 0x%x.\n", IpcBaseAddr));

  gBS->CloseEvent (Event);
}


/**
  Register notification function for PM configuration after PCI enumeration

  @param[in]  ScPolicy                  The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
ConfigurePmcAfterPciEnum (
  IN     SC_POLICY_HOB                  *ScPolicy
  )
{
  EFI_EVENT         PCIEnumerationCompleteEvent = NULL;
  VOID              *Registration;
  EFI_STATUS        Status;

  DEBUG ((DEBUG_INFO, "ConfigurePmcAfterPciEnum () Start\n"));

  //
  // Register InternalIpcLibPciEnumeratedNotifyEvent() notify function
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  PmcPciEnumeratedCompleteNotify,
                  NULL,
                  NULL,
                  &PCIEnumerationCompleteEvent
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Register for protocol notifications on this event
  //
  Status = gBS->RegisterProtocolNotify (
                  &gEfiPciEnumerationCompleteProtocolGuid,
                  PCIEnumerationCompleteEvent,
                  &Registration
                  );

  ASSERT_EFI_ERROR (Status);
  DEBUG ((DEBUG_INFO, "ConfigurePmcAfterPciEnum () End\n"));

  return EFI_SUCCESS;
}
#endif

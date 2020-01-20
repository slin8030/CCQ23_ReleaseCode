/** @file
  Initializes SC USB xDCI Device

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
#include <Library/PcdLib.h>
#include <ScRegs/RegsPsf.h>
/**
  Do OTG final initialization.

  @param[in] ScPolicy             The SC Policy Hob instance

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureOtgAtBoot (
  IN SC_POLICY_HOB              *ScPolicy
  )
{
  UINTN          OtgPciMmBase;
  UINT32         OtgMmioBase0;
  UINT32         OtgMmioBase1;
  EFI_STATUS     Status;
  SC_USB_CONFIG  *UsbConfig;

  DEBUG ((DEBUG_INFO, "ConfigureOtgAtBoot() Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicy, &gUsbConfigGuid, (VOID *) &UsbConfig);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Initialize Variables
  ///
  OtgPciMmBase     = 0;
  OtgMmioBase0     = 0;
  OtgMmioBase1     = 0;


  //
  // Update OTG device ACPI variables
  //
  if (UsbConfig->XdciConfig.Enable == ScAcpiMode) {
    DEBUG ((DEBUG_INFO, "Switching USB3 OTG into ACPI Mode.\n"));
    OtgPciMmBase = MmPciBase (
                     DEFAULT_PCI_BUS_NUMBER_SC,
                     PCI_DEVICE_NUMBER_OTG,
                     PCI_FUNCTION_NUMBER_OTG
                     );
    OtgMmioBase0 = MmioRead32 ((UINTN) (OtgPciMmBase + R_OTG_BAR0)) & B_OTG_BAR0_BA;
    ASSERT ((OtgMmioBase0 != 0) && (OtgMmioBase0 != B_OTG_BAR0_BA));
    OtgMmioBase1 = MmioRead32 ((UINTN) (OtgPciMmBase + R_OTG_BAR1)) & B_OTG_BAR1_BA;
    ASSERT ((OtgMmioBase1 != 0) && (OtgMmioBase1 != B_OTG_BAR1_BA));
    ///
    /// Broxton BIOS Spec, Section 29.5.1 ACPI Mode for Intel OTG
    /// Step 3: Set INTR_DISABLE bit, D21:F1:Reg 0x04 [10] = 1b.
    /// Step 4: Set BME bit, D21:F1:Reg 0x04 [2] = 1b.
    /// Step 5: Set MSE bit, D21:F1:Reg 0x04 [1] = 1b.
    ///
    MmioOr32 (
      (UINTN) (OtgPciMmBase + R_OTG_STSCMD),
      (UINT32) (B_OTG_STSCMD_INTR_DIS | B_OTG_STSCMD_BME | B_OTG_STSCMD_MSE)
    );
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (OtgPciMmBase + R_OTG_STSCMD),
      1,
      (VOID *) (UINTN) (OtgPciMmBase + R_OTG_STSCMD)
      );
    ///
    /// Step 6: Switch to ACPI interrupt
    ///   Set PCICFGCTR1.ACPI_INTR_EN1 bit in OTG private space, offset 0x500 [1] = 1b.
    /// Step 7: Hide Intel* OTG PCI configuration space
    ///   Set PCICFGCTR1.PCI_CFG_DIS1 bit in OTG private space, offset 0x500 [0] = 1b.
    ///
    SideBandAndThenOr32(
      OTG_PORT_ID,
      R_OTG_PCICFGCTR1,
      0xFFFFFFFF,
      (B_OTG_PCICFGCTR1_ACPI_INT_EN1 | B_OTG_PCICFGCTR1_PCI_CFG_DIS1)
    );
  }

  DEBUG ((DEBUG_INFO, "ConfigureOtgAtBoot() End\n"));

  return EFI_SUCCESS;
}

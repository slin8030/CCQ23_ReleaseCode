/** @file
  Perform related functions for SC Sata in DXE phase

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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
#include <ScInit.h>

/**
  Perform the remaining configuration on SC SATA to perform device detection
  at end of Dxe, then set the SATA SPD and PxE corresponding, and set the Register Lock

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
ConfigureSataDxe (
  IN SC_POLICY_HOB     *ScPolicy
  )
{
  EFI_STATUS     Status;
  SC_SATA_CONFIG *SataConfig;
  UINTN          PciSataRegBase;
  UINT16         SataPortsEnabled;
  UINT32         DwordReg;
  UINTN          Index;

  DEBUG ((DEBUG_INFO, "ConfigureSataDxe() Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicy, &gSataConfigGuid, (VOID *) &SataConfig);
  ASSERT_EFI_ERROR (Status);
  ///
  /// SATA PCS: Enable the port in any of below condition:
  /// i.)   Hot plug is enabled
  /// ii.)  A device is attached
  /// iii.) Test mode is enabled
  /// iv.)  Configured as eSATA port
  ///
  PciSataRegBase = MmPciBase (
                     DEFAULT_PCI_BUS_NUMBER_SC,
                     PCI_DEVICE_NUMBER_SATA,
                     PCI_FUNCTION_NUMBER_SATA
                     );
  SataPortsEnabled = 0;
  DwordReg = MmioRead32 (PciSataRegBase + R_SATA_PCS);
  DEBUG ((DEBUG_INFO, "PCS = 0x%x\n", DwordReg));
  for (Index = 0; Index < SC_MAX_SATA_PORTS; Index++) {
    if ((SataConfig->PortSettings[Index].HotPlug == TRUE) ||
        (DwordReg & (B_SATA_PCS_P0P << Index)) ||
        (SataConfig->TestMode == TRUE) ||
        (SataConfig->PortSettings[Index].External == TRUE)) {
      SataPortsEnabled |= (SataConfig->PortSettings[Index].Enable << Index);
    }
  }
  DEBUG ((DEBUG_INFO, "SataPortsEnabled = 0x%x\n", SataPortsEnabled));

  //
  // MAP - Port Mapping Register
  // PCI Offset:   90h - 93h
  // Value [23:16] Port Disabled
  //       [7:0]   Port Clock Disabled
  //
  MmioOr32 (PciSataRegBase + R_SATA_MAP, (UINT32)((~SataPortsEnabled << N_SATA_MAP_SPD) & B_SATA_MAP_SPD));
  DEBUG ((DEBUG_INFO, "MAP = 0x%x\n", MmioRead32(PciSataRegBase + R_SATA_MAP)));
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (PciSataRegBase + R_SATA_MAP),
    1,
    (VOID *) (UINTN) (PciSataRegBase + R_SATA_MAP)
    );
  ///
  /// Program PCS "Port X Enabled", SATA PCI offset 94h[7:0] = Port 0~7 Enabled bit as per SataPortsEnabled value.
  ///
  MmioOr16 (PciSataRegBase + R_SATA_PCS, SataPortsEnabled);
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint16,
    (UINTN) (PciSataRegBase + R_SATA_PCS),
    1,
    (VOID *) (UINTN) (PciSataRegBase + R_SATA_PCS)
    );
  DEBUG ((DEBUG_INFO, "PCS = 0x%x\n", MmioRead32(PciSataRegBase + R_SATA_PCS)));
  ///
  /// Step 14
  /// Program SATA PCI offset 9Ch [31] to 1b
  ///
  MmioOr32 ((UINTN) (PciSataRegBase + R_SATA_SATAGC), BIT31);
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (PciSataRegBase + R_SATA_SATAGC),
    1,
    (VOID *) (UINTN) (PciSataRegBase + R_SATA_SATAGC)
    );
  DEBUG ((DEBUG_INFO, "SATAGC = 0x%x\n", MmioRead32(PciSataRegBase + R_SATA_SATAGC)));
  DEBUG ((DEBUG_INFO, "ConfigureSataDxe() End\n"));
  return EFI_SUCCESS;
}

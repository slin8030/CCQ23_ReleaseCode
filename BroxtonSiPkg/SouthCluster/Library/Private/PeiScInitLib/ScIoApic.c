/** @file
  Initializes IoApic

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

/**
  Initialize IOAPIC according to IoApicConfig policy of the SC
  Platform Policy PPI

  @param[in] ScPolicyPpi  The SC Platform Policy PPI instance

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_DEVICE_ERROR        Device error, aborts abnormally.
**/
EFI_STATUS
ScIoApicInit (
  IN  SC_POLICY_PPI     *ScPolicyPpi
  )
{
  SC_IOAPIC_CONFIG  *IoApicConfig;
  UINT32            IoApicId;
  UINTN             P2sbBase;
  UINT32            RegData32;
  UINT16            RegData16;
  EFI_STATUS        Status;

  DEBUG ((DEBUG_INFO, "ScIoApicInit() - Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gIoApicConfigGuid, (VOID *) &IoApicConfig);
  ASSERT_EFI_ERROR (Status);

  P2sbBase = MmPciBase (
               DEFAULT_PCI_BUS_NUMBER_SC,
               PCI_DEVICE_NUMBER_P2SB,
               PCI_FUNCTION_NUMBER_P2SB
               );

  MmioOr16 (P2sbBase + R_PCH_P2SB_IOAC, (UINT16) (B_PCH_P2SB_IOAC_AE));
  ///
  /// Reads back for posted write to take effect and make sure it is set properly.
  ///
  if ((MmioRead16 (P2sbBase + R_PCH_P2SB_IOAC) & (UINT16) B_PCH_P2SB_IOAC_AE) == (UINT16) 0x00)
    return EFI_DEVICE_ERROR;

  ///
  /// Get current IO APIC ID
  ///
  MmioWrite8 ((UINTN) R_IO_APIC_INDEX, R_IO_APIC_ID);
  IoApicId = MmioRead32 ((UINTN) R_IO_APIC_WINDOW) >> 24;
  ///
  /// IO APIC ID is at APIC Identification Register [27:24]
  ///
  if ((IoApicConfig->IoApicId != IoApicId) && (IoApicConfig->IoApicId < 0x10)) {
    ///
    /// Program APIC ID
    ///
    MmioWrite8 ((UINTN) R_IO_APIC_INDEX, R_IO_APIC_ID);
    MmioWrite32 ((UINTN) R_IO_APIC_WINDOW, (UINT32) (IoApicConfig->IoApicId << 24));
  }

  ///
  /// P2SB PCI CFG Offset 0x6C = IBDF (IOxAPIC Bus:Device:Function)
  ///
  if (IoApicConfig->BdfValid) {
    RegData16 = ((UINT16) (IoApicConfig->BusNumber) << 8) & B_PCH_P2SB_IBDF_BUF;
    RegData16 |= ((UINT16) (IoApicConfig->DeviceNumber) << 3) & B_PCH_P2SB_IBDF_DEV;
    RegData16 |= (UINT16) (IoApicConfig->FunctionNumber) & B_PCH_P2SB_IBDF_FUNC;
    MmioWrite16 ((UINTN) (P2sbBase + R_PCH_P2SB_IBDF), RegData16);
  }

  ///
  /// CHV BIOS Spec Rev 0.5.0, Section 12.4.1.1
  /// 1. Set the AEN bit, IBASE + 0x60 [8], to 1'b1. Read back the value written.
  /// Done in ScInitPeim.c ScIoApicInit()
  ///
  /// 2. Build the MP table and/or ACPI APIC table for the OS
  /// This will be done in ACPI code.
  ///
  /// 3. The Maximum Redirection Entries (MRE) in APIC Version Register (VER), offset 01h,
  ///    [23:16] has to be written once for Microsoft Windows OS.
  ///
  if ((MmioRead16 (P2sbBase + R_PCH_P2SB_IOAC) & (UINT16) B_PCH_P2SB_IOAC_AE) == B_PCH_P2SB_IOAC_AE) {
    MmioWrite8 ((UINTN) R_IO_APIC_INDEX, R_IO_APIC_VS);
    RegData32 = MmioRead32 ((UINTN) R_IO_APIC_WINDOW);
    DEBUG ((DEBUG_INFO, "P2SB IOAPIC Version Register = 0x%x\n", RegData32));
    MmioWrite32 ((UINTN) R_IO_APIC_WINDOW, RegData32);
  }

  DEBUG ((DEBUG_INFO, "ScIoApicInit() - End\n"));

  return EFI_SUCCESS;
}



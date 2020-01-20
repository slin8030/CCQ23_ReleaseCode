/** @file
  Header file for the SwitchableGraphics Dxe driver.
  This driver loads SwitchableGraphics ACPI tables.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

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
#ifndef _HYBRID_GRAPHICS_DXE_H_
#define _HYBRID_GRAPHICS_DXE_H_

#include <IndustryStandard/Acpi.h>
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Library/UefiLib.h>
#include <Library/PciLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/PciIo.h>
#include <Guid/HobList.h>
#include <Guid/EventGroup.h>
#include <Protocol/FirmwareVolume2.h>
#include <Library/MmPciLib.h>
#include <SaAccess.h>
#include <Guid/SaDataHob.h>
#include <Protocol/GlobalNvsArea.h>
#include <CpuRegs.h>
#include <PcieRegs.h>

///
/// HG ACPI table data storage GUID
///
extern EFI_GUID gHgAcpiTableStorageGuid;

///
/// PEG Capability Equates
///
#define PEG_CAP_ID  0x10
#define PEG_CAP_VER 0x2

/**
  Initialize the Hybrid Graphics support.

  @param[in] ImageHandle - Handle for the image of this driver

  @retval EFI_SUCCESS          - SwitchableGraphics initialization complete
  @retval EFI_OUT_OF_RESOURCES - Unable to allocated memory
**/
EFI_STATUS
HybridGraphicsInit (
  VOID
  );

/**
  Initialize the runtime SwitchableGraphics support data for ACPI tables in GlobalNvs.

  @retval EFI_SUCCESS - The data updated successfully.
**/
EFI_STATUS
UpdateGlobalNvsData (
  VOID
  );


/**
  Find the Offset to a given Capabilities ID
    CAPID list:
      0x01 = PCI Power Management Interface
      0x04 = Slot Identification
      0x05 = MSI Capability
      0x10 = PCI Express Capability

    @param[in] Bus       -   Pci Bus Number
    @param[in] Device    -   Pci Device Number
    @param[in] Function  -   Pci Function Number
    @param[in] CapId     -   CAPID to search for

    @retval 0       - CAPID not found
    @retval Other   - CAPID found, Offset of desired CAPID
**/
UINT32
PcieFindCapId (
  IN UINT8 Bus,
  IN UINT8 Device,
  IN UINT8 Function,
  IN UINT8 CapId
  );

/**
  Load Intel HG SSDT Tables

  @retval EFI_SUCCESS - HG SSDT Table load successful.
**/
EFI_STATUS
LoadAcpiTables (
  VOID
  );


/**
  This function gets registered as a Ready to boot callback to perform HG initialization

  @param[in] Event     - A pointer to the Event that triggered the callback.
  @param[in] Context   - A pointer to private data registered with the callback function.
**/
VOID
EFIAPI
HgReadyToBootCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  );

/**
  Find the Pcie Capability ID.

  @param[in] PciePortDev       - PCIe Port Device Number
  @param[in] PciePortFun       - PCIe Port Function Number

  @retval EFI_SUCCESS - Found  out the Cap Id.
**/
EFI_STATUS
FindPcieEpCapId (
  IN UINT8 PciePortDev,
  IN UINT8 PciePortFun
  );
#endif

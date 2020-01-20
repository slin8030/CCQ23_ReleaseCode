/** @file
  Hybrid Graphics Graphics Dxe Policy only for APLK..

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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
#include "HybridGraphicsInit.h"


GLOBAL_REMOVE_IF_UNREFERENCED SA_DATA_HOB           *SaDataHob;
GLOBAL_REMOVE_IF_UNREFERENCED UINT8                 PciePortBus;
GLOBAL_REMOVE_IF_UNREFERENCED UINT8                 RootPortDev;
GLOBAL_REMOVE_IF_UNREFERENCED UINT8                 RootPortFun;

EFI_EVENT             mReadyToBootEvent;

/**
  Initialize the Hybrid Graphics support (DXE).

  @retval EFI_SUCCESS          - Hybrid Graphics initialization complete
  @retval EFI_OUT_OF_RESOURCES - Unable to allocated memory
  @retval EFI_NOT_FOUND        - SA DataHob not found
  @retval EFI_DEVICE_ERROR     - Error Accessing SG GPIO
**/
EFI_STATUS
HybridGraphicsInit (
  VOID
  )
{
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "HG::Hybrid Graphics DXE Initialization Starts\n"));
  ///
  /// Get HG GPIO info from SA HOB.
  ///
  Status = EfiGetSystemConfigurationTable (&gEfiHobListGuid, (VOID **) &SaDataHob);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SaDataHob = GetNextGuidHob (&gSaDataHobGuid, SaDataHob);
  if (SaDataHob == NULL) {
    return EFI_NOT_FOUND;
  }

  RootPortDev = SaDataHob->HgInfo.RootPortDev;
  RootPortFun = SaDataHob->HgInfo.RootPortFun;
  DEBUG ((DEBUG_INFO, "dGPU Rootport info[B/D/F] : [0x00/0x%x/0x%x]\n", RootPortDev, RootPortFun));

  ///
  /// Create the notification and register callback function
  ///
  Status = EfiCreateEventReadyToBootEx (
            TPL_CALLBACK,
            HgReadyToBootCallback,
            NULL,
            &mReadyToBootEvent
            );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "HG::Hybrid Graphics DXE Initialization Ends\n"));

  return Status;
}

/**
  Initialize the runtime Hybrid Graphics support data for ACPI tables in GlobalNvs.

  @retval EFI_SUCCESS    - The data updated successfully.
**/
EFI_STATUS
UpdateGlobalNvsData (
  VOID
  )
{
  EFI_GLOBAL_NVS_AREA_PROTOCOL    *GlobalNvsArea;
  EFI_STATUS                      Status;

  ///
  ///  Locate the Global NVS Protocol.
  ///
  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **)&GlobalNvsArea
                  );
  ASSERT_EFI_ERROR(Status);

  DEBUG ((DEBUG_INFO, "HG::Updting the Global NVS for Hybrid Graphics\n"));

  ///
  /// SG Mode for ASL usage
  ///
  GlobalNvsArea->Area->HgEnabled            |= SaDataHob->HgInfo.HgEnabled;
  GlobalNvsArea->Area->XPcieCfgBaseAddress   = (UINT32) (MmPciAddress (0, 0, 0, 0, 0));
  GlobalNvsArea->Area->DelayAfterPwrEn       = SaDataHob->HgInfo.HgDelayAfterPwrEn;
  GlobalNvsArea->Area->DelayAfterHoldReset   = SaDataHob->HgInfo.HgDelayAfterHoldReset;

  DEBUG ((DEBUG_INFO, "HG:: Hybrid Graphics Enabled : 0x%x\n", SaDataHob->HgInfo.HgEnabled));

  if (SaDataHob->HgInfo.HgEnabled == 1) {
    ///
    /// GPIO Assignment for ASL usage
    ///
    GlobalNvsArea->Area->HgHoldRstCommOffset     = SaDataHob->HgInfo.HgDgpuHoldRst.CommunityOffset;
    GlobalNvsArea->Area->HgHoldRstPinOffset      = SaDataHob->HgInfo.HgDgpuHoldRst.PinOffset;
    GlobalNvsArea->Area->HgHoldRstActiveInfo     = SaDataHob->HgInfo.HgDgpuHoldRst.Active;
    GlobalNvsArea->Area->HgPwrEnableCommOffset   = SaDataHob->HgInfo.HgDgpuPwrEnable.CommunityOffset;
    GlobalNvsArea->Area->HgPwrEnablePinOffset    = SaDataHob->HgInfo.HgDgpuPwrEnable.PinOffset;
    GlobalNvsArea->Area->HgPwrEnableActiveInfo   = SaDataHob->HgInfo.HgDgpuPwrEnable.Active;
    ///
    /// Find capability ID for Endpoint on PEG Port 0
    ///
    Status = FindPcieEpCapId (RootPortDev, RootPortFun);
  }

  return Status;
}

/**
  Find the Pcie Capability ID.

  @param[in] PciePortDev       - Pcie Port Device Number
  @param[in] PciePortFun       - Pcie Port Function Number

  @retval EFI_SUCCESS - Found  out the Cap Id.
**/
EFI_STATUS
FindPcieEpCapId (
  IN UINT8 PciePortDev,
  IN UINT8 PciePortFun
  )
{
  EFI_GLOBAL_NVS_AREA_PROTOCOL           *GlobalNvsArea;
  UINTN                                  PciePortAddress;
  UINT8                                  PcieCapOffset;
  EFI_STATUS                             Status;

  ///
  ///  Locate the SA Global NVS Protocol.
  ///
  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &GlobalNvsArea
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// Get the PCIe Port Base Address to find Secondary bus number & sub-ordinate bus number
  ///
  PciePortAddress = MmPciBase (0, PciePortDev, PciePortFun);

  ///
  /// Check if Pcie device is present or not.
  ///
  if (MmioRead16 (PciePortAddress + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
    return EFI_UNSUPPORTED;
  }
  ///
  /// Get the Endpoint PCIe Port Bus number by reading secondary bus offset.
  ///
  PciePortBus = MmioRead8 (PciePortAddress + PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET);

  if (PciePortBus == 0xFF) {
    return EFI_UNSUPPORTED;
  }

  ///
  /// Check if End Point is prensent or not on Pcie device..
  ///
  if (MmioRead16 (MmPciBase (PciePortBus, 0, 0) + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
    return EFI_UNSUPPORTED;
  }

  PcieCapOffset = (UINT8) PcieFindCapId (PciePortBus, 0, 0, PEG_CAP_ID);

  GlobalNvsArea->Area->RootPortBaseAddress = (UINT32) PciePortAddress;
  GlobalNvsArea->Area->PcieEpSecBusNum     = PciePortBus;
  GlobalNvsArea->Area->PcieEpCapOffset     = PcieCapOffset;

  return Status;
}

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
  IN UINT8   Bus,
  IN UINT8   Device,
  IN UINT8   Function,
  IN UINT8   CapId
  )
{
  UINTN DeviceBaseAddress;
  UINT8 CapHeader;

  ///
  /// Always start at Offset 0x34
  ///
  DeviceBaseAddress = MmPciAddress (0, Bus, Device, Function,0);
  CapHeader         = MmioRead8 (DeviceBaseAddress + PCI_CAPP);
  if (CapHeader == 0xFF) {
    return 0;
  }

  while (CapHeader != 0) {
    ///
    /// Bottom 2 bits of the pointers are reserved per PCI Local Bus Spec 2.2
    ///
    CapHeader &= ~(BIT1 + BIT0);
    ///
    /// Search for desired CapID
    ///
    if (MmioRead8 (DeviceBaseAddress + CapHeader) == CapId) {
      return CapHeader;
    }

    CapHeader = MmioRead8 (DeviceBaseAddress + CapHeader + 1);
  }

  return 0;
}

/**
  Load Intel HG SSDT Tables

  @retval EFI_SUCCESS - SG SSDT Table load successful.
**/
EFI_STATUS
LoadAcpiTables (
  VOID
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  BOOLEAN                       LoadTable;
  UINTN                         NumberOfHandles;
  UINTN                         Index;
  INTN                          Instance;
  UINTN                         Size;
  UINT32                        FvStatus;
  UINTN                         TableHandle;
  EFI_FV_FILETYPE               FileType;
  EFI_FV_FILE_ATTRIBUTES        Attributes;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *FwVol;
  EFI_ACPI_TABLE_PROTOCOL       *AcpiTable;
  EFI_ACPI_DESCRIPTION_HEADER   *TableHeader;
  EFI_ACPI_COMMON_HEADER        *Table;

  FwVol         = NULL;
  Table         = NULL;

  DEBUG ((DEBUG_INFO, "HG:: Loading ACPI Tables...\n"));

  ///
  /// Locate FV protocol.
  ///
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolume2ProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);

  ///
  /// Look for FV with ACPI storage file
  ///
  for (Index = 0; Index < NumberOfHandles; Index++) {
    ///
    /// Get the protocol on this handle
    /// This should not fail because of LocateHandleBuffer
    ///
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiFirmwareVolume2ProtocolGuid,
                    (VOID **) &FwVol
                    );
    ASSERT_EFI_ERROR (Status);
    if (FwVol == NULL) {
      return EFI_NOT_FOUND;
    }
    ///
    /// See if it has the ACPI storage file
    ///
    Size      = 0;
    FvStatus  = 0;
    Status = FwVol->ReadFile (
                      FwVol,
                      &gHgAcpiTableStorageGuid,
                      NULL,
                      &Size,
                      &FileType,
                      &Attributes,
                      &FvStatus
                      );

    ///
    /// If we found it, then we are done
    ///
    if (!EFI_ERROR (Status)) {
      break;
    }
  }
  ///
  /// Our exit status is determined by the success of the previous operations
  /// If the protocol was found, Instance already points to it.
  ///
  ///
  /// Free any allocated buffers
  ///
  FreePool (HandleBuffer);

  ///
  /// Sanity check that we found our data file
  ///
  ASSERT (FwVol);

  ///
  /// Locate ACPI tables
  ///
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTable);
  ASSERT_EFI_ERROR(Status);

  ///
  /// Read tables from the storage file.
  ///
  if (FwVol == NULL) {
    ASSERT_EFI_ERROR (EFI_NOT_FOUND);
    return EFI_NOT_FOUND;
  }
  Instance = 0;

  while (Status == EFI_SUCCESS) {
    ///
    /// Read the ACPI tables
    ///
    Status = FwVol->ReadSection (
                      FwVol,
                      &gHgAcpiTableStorageGuid,
                      EFI_SECTION_RAW,
                      Instance,
                      (VOID **) &Table,
                      &Size,
                      &FvStatus
                      );
    if (!EFI_ERROR (Status)) {
      ///
      /// check and load Hybrid Graphics SSDT table
      ///
      LoadTable   = FALSE;
      TableHeader = (EFI_ACPI_DESCRIPTION_HEADER *) Table;

      if (((EFI_ACPI_DESCRIPTION_HEADER *) TableHeader)->OemTableId == SIGNATURE_64 (
          'H',
          'g',
          'P',
          'c',
          'h',
          0,
          0,
          0
          )
          ) {
        ///
        /// This is SG SSDT [dGPU is present on PCH RootPort]
        ///
        DEBUG ((DEBUG_INFO, "HG:: ---- HG SSDT ----\n"));
        DEBUG ((DEBUG_INFO, "HG:: Found out SSDT:HgPch [HgSsdt.asl]. dGPU is present on PCH RootPort.\n"));
        LoadTable = TRUE;
      }

      ///
      /// Add the table
      ///
      if (LoadTable) {
        TableHandle = 0;
        Status = AcpiTable->InstallAcpiTable (
                                  AcpiTable,
                                  TableHeader,
                                  TableHeader->Length,
                                  &TableHandle
                                  );
      }
      ///
      /// Increment the instance
      ///
      Instance++;
      Table = NULL;
    }
  }

  return EFI_SUCCESS;
}


/**
  This function gets registered as a EndOfDxe callback to perform SG initialization

  @param[in] Event     - A pointer to the Event that triggered the callback.
  @param[in] Context   - A pointer to private data registered with the callback function.
**/
VOID
EFIAPI
HgReadyToBootCallback (
  IN EFI_EVENT Event,
  IN VOID      *Context
  )
{
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "HG:: Ready To Boot Callback\n"));

  ///
  /// Update GlobalNvs data for runtime usage
  ///
  Status = UpdateGlobalNvsData ();
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
  }

  if (SaDataHob->HgInfo.HgEnabled == 1) {
    ///
    /// Load Intel HG SSDT tables
    ///
    Status = LoadAcpiTables ();
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
    }

  }
  gBS->CloseEvent(Event);
}

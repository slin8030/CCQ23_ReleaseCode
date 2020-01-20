/** @file
  This driver is for Switchable Graphics Feature DXE initialize.

;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <SwitchableGraphicsDxe.h>

/**

  Switchable Graphics feature driver entry point.
  This driver will handle secondary VBIOS and create feature own SSDT.

  @param[in] ImageHandle   Image handle of this driver.
  @param[in] SystemTable   Pointer to standard EFI system table.

  @retval EFI_SUCCESS    Switchable Graphics feature DXE initialized successfully.
  @retval !EFI_SUCCESS   Switchable Graphics feature doesn't be supported.

**/
EFI_STATUS
SwitchableGraphicsDxeInitEntry (
  IN EFI_HANDLE                               ImageHandle,
  IN EFI_SYSTEM_TABLE                         *SystemTable
  )
{
  VOID                                        *Registration;

  if (FeaturePcdGet (PcdSwitchableGraphicsSupported)) {
    EfiCreateProtocolNotifyEvent (
      &gH2OSwitchableGraphicsEventProtocolGuid,
      TPL_CALLBACK,
      SwitchableGraphicsBdsCallback,
      NULL,
      &Registration
      );
  }

  return EFI_SUCCESS;
}

/**

  After BDS platform driver connect Root Bridge will install SG protocol
  to trigger event to run this Switchable Graphics feature callback,
  this specific timing discrete graphics already exist then we can
  handle and initialize it.

  @param[in] Event    Event whose notification function is being invoked.
  @param[in] Context  Pointer to the notification function's context.

  @retval None.

**/
VOID
EFIAPI
SwitchableGraphicsBdsCallback (
  IN EFI_EVENT                                Event,
  IN VOID                                     *Context
  )
{
  EFI_STATUS                                  Status;
  SG_DXE_INFORMATION_DATA                     *SgDxeInfoData;
  SG_INFORMATION_DATA_HOB                     *SgInfoDataHob;

  //
  // Get SG information data HOB, and create SG DXE infromation Data.
  //
  Status = SwitchableGraphicsDxeInitialize (&SgDxeInfoData, &SgInfoDataHob);
  if (EFI_ERROR (Status)) {
    return;
  }

  //
  // Close Switchable Graphics BDS event.
  //
  gBS->CloseEvent (Event);

  //
  // Diable dGPU Hotplug SCI/SMI, set SSID/SVID and close dGPU HD Audio device.
  //
  Status = DgpuBdsInitialize (SgDxeInfoData, SgInfoDataHob);
  if (EFI_ERROR (Status)) {
    FreeSgDxeInfoData (&SgDxeInfoData);
    return;
  }

  //
  // Search discrete graphics VBIOS location.
  //
  Status = SearchDiscreteGraphicsVbios (SgDxeInfoData);
  if (EFI_ERROR (Status)) {
    FreeSgDxeInfoData (&SgDxeInfoData);
    return;
  }

  //
  // Set SG SSDT and initialize SG own operation region.
  //
  Status = SetSwitchableGraphicsSsdt (SgDxeInfoData, SgInfoDataHob);
  if (EFI_ERROR (Status)) {
    FreeSgDxeInfoData (&SgDxeInfoData);
    return;
  }
}

/**

  Switchable Graphics feature DXE driver initialize function.
  This function will get SG information data HOB, and create SG DXE infromation Data.

  @param[in, out] SgDxeInfoData   A double pointer of SG DXE information data structure,
                                  this driver or SG Operation Region will use these data.
  @param[in, out] SgInfoDataHob   A double pointer of SG information data HOB,
                                  SG PEI Module created this HOB, and passes the data from PEI phase.

  @retval EFI_SUCCESS    This path initialized data successfully, and need handle SG SSDT and VBIOS.
  @retval !EFI_SUCCESS   This path initialized data failed and doesn't handle SG SSDT and VBIOS.

**/
STATIC
EFI_STATUS
SwitchableGraphicsDxeInitialize (
  IN OUT SG_DXE_INFORMATION_DATA              **SgDxeInfoData,
  IN OUT SG_INFORMATION_DATA_HOB              **SgInfoDataHob
  )
{
  EFI_STATUS                                  Status;
  UINT8                                       DgpuBus;
  UINT16                                      DgpuVendorId;
  UINTN                                       PciAddress;

  (*SgDxeInfoData) = NULL;
  //
  // Get SG related information data HOB.
  //
  Status = EfiGetSystemConfigurationTable (&gEfiHobListGuid, (VOID **)SgInfoDataHob);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  (*SgInfoDataHob) = GetNextGuidHob (&gH2OSgInformationDataHobGuid, (*SgInfoDataHob));
  if ((*SgInfoDataHob) == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // Read Master discrete GPU Bus and Venodr ID.
  //
  PciAddress = PCI_EXPRESS_LIB_ADDRESS (
                 (**SgInfoDataHob).DgpuBridgeBus,
                 (**SgInfoDataHob).DgpuBridgeDevice,
                 (**SgInfoDataHob).DgpuBridgeFunction,
                 PCI_SBUS
                 );
  DgpuBus = PciExpressRead8 (PciAddress);
  PciAddress = PCI_EXPRESS_LIB_ADDRESS (
                 DgpuBus,
                 DGPU_DEVICE_NUM,
                 DGPU_FUNCTION_NUM,
                 PCI_VID
                 );
  DgpuVendorId = PciExpressRead16 (PciAddress);

  if ((DgpuBus == 0x00) || (DgpuBus == 0xFF) || (DgpuVendorId == 0xFFFF) ||
      ((**SgInfoDataHob).SgMode == SgModeDisabled)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate and initialize Switchable Graphics DXE driver information data.
  //
  (*SgDxeInfoData) = AllocateZeroPool (sizeof (SG_DXE_INFORMATION_DATA));
  if ((*SgDxeInfoData) == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  (**SgDxeInfoData).AmdSecondaryGrcphicsCommandRegister = (**SgInfoDataHob).AmdSecondaryGrcphicsCommandRegister;
  (**SgDxeInfoData).NvidiaSecondaryGrcphicsCommandRegister = (**SgInfoDataHob).NvidiaSecondaryGrcphicsCommandRegister;
  (**SgDxeInfoData).GpioBaseAddress = IO_BASE_ADDRESS;
  (**SgDxeInfoData).DgpuBus = DgpuBus;
  (**SgDxeInfoData).DgpuVendorId = DgpuVendorId;

  return EFI_SUCCESS;
}

/**

  dGPU BDS initialize function.
  This function will diable dGPU Hotplug SCI/SMI, set SSID/SVID and close dGPU HD Audio device.
  Create Exit Boot Service event and Legacy Boot event to set the Secondary Grcphics Command Register.

  @param[in, out] SgDxeInfoData   A pointer of SG DXE information data structure,
                                  this driver or SG Operation Region will use these data.
  @param[in]      SgInfoDataHob   A pointer of SG information data HOB,
                                  SG PEI Module created this HOB, and passes the data from PEI phase.

  @retval EFI_SUCCESS    Update global NVS area variable and set SG variable both successfully.
  @retval !EFI_SUCCESS   Update global NVS area variable or set SG variable failed.

**/
STATIC
EFI_STATUS
DgpuBdsInitialize (
  IN OUT SG_DXE_INFORMATION_DATA              *SgDxeInfoData,
  IN SG_INFORMATION_DATA_HOB                  *SgInfoDataHob
  )
{
  EFI_EVENT                                   ExitBootServicesEvent;
  EFI_EVENT                                   LegacyBootEvent;
  EFI_EVENT                                   ReadyToBootEvent;
  EFI_STATUS                                  Status;

  Status = EFI_SUCCESS;
  if (SgInfoDataHob->SgMode == SgModeMuxless) {
    //
    // Diable dGPU Hotplug SCI/SMI and set SSID/SVID.
    //
    DisableDgpuBridgeHotplugSmi (SgInfoDataHob);
    SetDgpuSsidSvid (SgDxeInfoData);
    //
    // Set Secondary Graphics Command Register.
    //
    if (SgInfoDataHob->BootType == EfiBootType) {
      Status = gBS->CreateEvent (
                      EVT_SIGNAL_EXIT_BOOT_SERVICES,
                      TPL_NOTIFY,
                      SetSecondaryGrcphicsCommandRegister,
                      SgDxeInfoData,
                      &ExitBootServicesEvent
                      );
    } else {
      Status = EfiCreateEventLegacyBootEx (
                 TPL_NOTIFY,
                 SetSecondaryGrcphicsCommandRegister,
                 SgDxeInfoData,
                 &LegacyBootEvent
                 );
    }
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // Close dGPU HD Audio device.
    //
    Status = EfiCreateEventReadyToBootEx (
               TPL_CALLBACK,
               CloseDiscreteHdAudio,
               SgDxeInfoData,
               &ReadyToBootEvent
               );
  }

  return Status;
}

/**

  Search discrete graphics VBIOS location and save into SG information data.

  @param[in, out] SgDxeInfoData   A pointer of SG DXE information data structure,
                                  this driver or SG Operation Region will use these data.

  @retval EFI_SUCCESS    Search discrete graphics VBIOS successfully.
  @retval !EFI_SUCCESS   Search discrete graphics VBIOS failed.

**/
STATIC
EFI_STATUS
SearchDiscreteGraphicsVbios (
  IN OUT SG_DXE_INFORMATION_DATA              *SgDxeInfoData
  )
{
  EFI_HANDLE                                  *HandleBuffer;
  EFI_PCI_IO_PROTOCOL                         *PciIo;
  EFI_STATUS                                  Status;
  PCI_3_0_DATA_STRUCTURE                      *PcirBlockPtr;
  PCI_EXPANSION_ROM_HEADER                    *VBiosRomImage;
  UINTN                                       HandleCount;
  UINTN                                       Index;

  //
  // Get all PCI IO protocols
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Find the video BIOS by checking each PCI IO handle for DGPU video
  // BIOS OPROM.
  //
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (EFI_ERROR (Status) || (PciIo->RomImage == NULL)) {
      //
      // If this PCI device doesn't have a ROM image, skip to the next device.
      //
      continue;
    }
    VBiosRomImage = PciIo->RomImage;

    //
    // Get pointer to PCIR structure
    //
    PcirBlockPtr = (PCI_3_0_DATA_STRUCTURE *)((UINTN)VBiosRomImage + VBiosRomImage->PcirOffset);

    //
    // Check if we have an video BIOS OPROM for DGPU.
    //
    if ((VBiosRomImage->Signature == PCI_EXPANSION_ROM_HEADER_SIGNATURE) &&
        (SgDxeInfoData->DgpuVendorId == PcirBlockPtr->VendorId) &&
        (PcirBlockPtr->ClassCode[2] == PCI_CLASS_DISPLAY)) {
      SgDxeInfoData->Vbios.Address = (UINTN)PciIo->RomImage;
      SgDxeInfoData->Vbios.Size = (UINT32)PciIo->RomSize;
      if (SgDxeInfoData->Vbios.Size > sizeof (OPERATION_REGION_VBIOS)) {
        DEBUG((DEBUG_ERROR, "\nASSERT! Size too small:")); 
        ASSERT(FALSE);
      }
      FreePool (HandleBuffer);
      return EFI_SUCCESS;
    }
  }
  FreePool (HandleBuffer);

  return EFI_NOT_FOUND;
}

/**

  Load Switchable Graphics own SSDT (Secondary System Description Table) and initialize
  Switchable Graphics own operation region for ASL (ACPI Source Language) code usage.

  @param[in] SgDxeInfoData   A pointer of SG DXE information data structure,
                             this driver or SG Operation Region will use these data.
  @param[in] SgInfoDataHob   A pointer of SG information data HOB,
                             SG PEI Module created this HOB, and passes the data from PEI phase.

  @retval EFI_SUCCESS    Load and set Switchable Graphics SSDT successfully.
  @retval !EFI_SUCCESS   Load or set Switchable Graphics SSDT failed.

**/
STATIC
EFI_STATUS
SetSwitchableGraphicsSsdt (
  IN SG_DXE_INFORMATION_DATA                  *SgDxeInfoData,
  IN SG_INFORMATION_DATA_HOB                  *SgInfoDataHob
  )
{
  EFI_ACPI_COMMON_HEADER                      *CurrentTable;
  EFI_ACPI_DESCRIPTION_HEADER                 *TempTable;
  EFI_ACPI_TABLE_PROTOCOL                     *AcpiTable;
  EFI_FIRMWARE_VOLUME2_PROTOCOL               *FirmwareVolume;
  EFI_FV_FILE_ATTRIBUTES                      Attributes;
  EFI_FV_FILETYPE                             FileType;
  EFI_GUID                                    *SsdtFileGuid;
  EFI_HANDLE                                  *HandleBuffer;
  EFI_STATUS                                  Status;
  UINT32                                      FvStatus;
  UINTN                                       TableKey;
  UINTN                                       Index;
  UINTN                                       NumberOfHandles;
  UINTN                                       Size;

  CurrentTable   = NULL;
  FirmwareVolume = NULL;
  SsdtFileGuid   = NULL;
  TempTable      = NULL;

  //
  // Choose the SSDT table base on Vendor ID and SG mode.
  //
  if (SgInfoDataHob->SgMode == SgModeMuxless) {
    if ((FeaturePcdGet (PcdNvidiaOptimusSupported)) && (SgDxeInfoData->DgpuVendorId == NVIDIA_VID)) {
      SsdtFileGuid = &(SgInfoDataHob->NvidiaOptimusSsdtGuid);
    }
    if ((FeaturePcdGet (PcdAmdPowerXpressSupported)) && (SgDxeInfoData->DgpuVendorId == AMD_VID)) {
      SsdtFileGuid = &(SgInfoDataHob->AmdPowerXpressSsdtGuid);
    }
  }
  if (SsdtFileGuid == NULL) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **)&AcpiTable);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolume2ProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status) || (NumberOfHandles == 0)) {
    return Status;
  }
  //
  // Looking for FV with ACPI storage file
  //
  for (Index = 0; Index < NumberOfHandles; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiFirmwareVolume2ProtocolGuid, (VOID **)&FirmwareVolume);
    if (EFI_ERROR (Status)) {
      continue;
    }
    FvStatus = 0;
    Size     = 0;
    Status = FirmwareVolume->ReadFile (
                               FirmwareVolume,
                               SsdtFileGuid,
                               NULL,
                               &Size,
                               &FileType,
                               &Attributes,
                               &FvStatus
                               );
    if (Status == EFI_SUCCESS) {
      break;
    }
  }
  FreePool (HandleBuffer);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  //
  // Read tables from the storage file.
  //
  TableKey = 0;
  Status = FirmwareVolume->ReadSection (
                             FirmwareVolume,
                             SsdtFileGuid,
                             EFI_SECTION_RAW,
                             0,
                             (VOID **)&CurrentTable,
                             &Size,
                             &FvStatus
                             );
  if (!EFI_ERROR (Status)) {
    TempTable = AllocateZeroPool (CurrentTable->Length);
    ASSERT (TempTable != NULL);
    if (TempTable != NULL) {
      CopyMem (TempTable, CurrentTable, CurrentTable->Length);
      Status = InitializeOpRegion ((EFI_ACPI_DESCRIPTION_HEADER*)TempTable, SgDxeInfoData, SgInfoDataHob);
      if (!EFI_ERROR (Status)) {
        Status = AcpiTable->InstallAcpiTable (AcpiTable, TempTable, CurrentTable->Length, &TableKey);
      }
      FreePool (CurrentTable);
      FreePool (TempTable);
    }
  }

  return Status;
}

/**

  Initialize Switchable Graphics own operation region for ASL (ACPI Source Language) code usage.

  @param[in] NvStoreTable    A pointer of temporary SG SSDT that prepares modified and installed.
  @param[in] SgDxeInfoData   A pointer of SG DXE information data structure,
                             this driver or SG Operation Region will use these data.
  @param[in] SgInfoDataHob   A pointer of SG information data HOB,
                             SG PEI Module created this HOB, and passes the data from PEI phase.

  @retval EFI_SUCCESS    Load Switchable Graphics SSDT successfully.
  @retval !EFI_SUCCESS   Load Switchable Graphics SSDT failed.

**/
STATIC
EFI_STATUS
InitializeOpRegion (
  IN EFI_ACPI_DESCRIPTION_HEADER              *NvStoreTable,
  IN SG_DXE_INFORMATION_DATA                  *SgDxeInfoData,
  IN SG_INFORMATION_DATA_HOB                  *SgInfoDataHob
  )
{
  EFI_ACPI_DESCRIPTION_HEADER                 *Table;
  EFI_STATUS                                  Status;
  OPERATION_REGION_AMD                        *AmdOpRegion;
  OPERATION_REGION_NVIDIA                     *NvidiaOpRegion;
  OPERATION_REGION_SG                         *SgOpRegion;
  OPERATION_REGION_VBIOS                      *VbiosOpRegion;
  UINT8                                       *SsdtPointer;
  UINT32                                      *Signature;

  Table = NvStoreTable;
  for (SsdtPointer =  (UINT8 *)((UINTN)Table + sizeof (EFI_ACPI_DESCRIPTION_HEADER)); \
       SsdtPointer <= (UINT8 *)((UINTN)Table + (UINTN)(Table->Length)); SsdtPointer++) {
    Signature = (UINT32*)SsdtPointer;
    switch (*Signature) {
      case (SIGNATURE_32 ('S', 'G', 'O', 'P')): // SG operation region initialize
        if (IsAmlOpRegionObject (SsdtPointer)) {
          Status = gBS->AllocatePool (EfiACPIMemoryNVS, sizeof (OPERATION_REGION_SG), (VOID **)&SgOpRegion);
          if (!EFI_ERROR (Status)) {
            ZeroMem (SgOpRegion, sizeof (OPERATION_REGION_SG));
            UpdateSgOpRegion (SgOpRegion, SgDxeInfoData, SgInfoDataHob);
            SetOpRegion (SsdtPointer, SgOpRegion, sizeof (OPERATION_REGION_SG));
          }
        }
        break;
      case (SIGNATURE_32 ('V', 'B', 'O', 'R')): // VBIOS operation region initialize
        if (IsAmlOpRegionObject (SsdtPointer)) {
          Status = gBS->AllocatePool (EfiACPIMemoryNVS, sizeof (OPERATION_REGION_VBIOS), (VOID **)&VbiosOpRegion);
          if (!EFI_ERROR (Status)) {
            ZeroMem (VbiosOpRegion, sizeof (OPERATION_REGION_VBIOS));
            VbiosOpRegion->RVBS = (UINT32)SgDxeInfoData->Vbios.Size;
            CopyMem (
              (VOID *)(VbiosOpRegion->VBOIS),
              (VOID *)(SgDxeInfoData->Vbios.Address),
              SgDxeInfoData->Vbios.Size
              );
            SgDxeInfoData->Vbios.Address = (UINTN)(VbiosOpRegion->VBOIS);
            SetOpRegion (SsdtPointer, VbiosOpRegion, sizeof (OPERATION_REGION_VBIOS));
          }
        }
        break;
      case (SIGNATURE_32 ('A', 'O', 'P', 'R')): // AMD operation region initialize
        if ((FeaturePcdGet (PcdAmdPowerXpressSupported)) && IsAmlOpRegionObject (SsdtPointer)) {
          Status = gBS->AllocatePool (EfiACPIMemoryNVS, sizeof (OPERATION_REGION_AMD), (VOID **)&AmdOpRegion);
          if (!EFI_ERROR (Status)) {
            ZeroMem (AmdOpRegion, sizeof (OPERATION_REGION_AMD));
            UpdateAmdOpRegion (AmdOpRegion, SgDxeInfoData, SgInfoDataHob);
            SetOpRegion (SsdtPointer, AmdOpRegion, sizeof (OPERATION_REGION_AMD));
          }
        }
        SsdtPointer = (UINT8 *)((UINTN)Table + (UINTN)(Table->Length));
        break;
      case (SIGNATURE_32 ('N', 'O', 'P', 'R')): // nVIDIA operation region initialize
        if ((FeaturePcdGet (PcdNvidiaOptimusSupported)) && IsAmlOpRegionObject (SsdtPointer)) {
          Status = gBS->AllocatePool (EfiACPIMemoryNVS, sizeof (OPERATION_REGION_NVIDIA), (VOID **)&NvidiaOpRegion);
          if (!EFI_ERROR (Status)) {
            ZeroMem (NvidiaOpRegion, sizeof (OPERATION_REGION_NVIDIA));
            UpdateNvidiaOpRegion (NvidiaOpRegion, SgDxeInfoData, SgInfoDataHob);
            SetOpRegion (SsdtPointer, NvidiaOpRegion, sizeof (OPERATION_REGION_NVIDIA));
          }
        }
        SsdtPointer = (UINT8 *)((UINTN)Table + (UINTN)(Table->Length));
        break;
    }
    AcpiChecksum (Table, Table->Length, ((UINTN)(&(((EFI_ACPI_DESCRIPTION_HEADER *)0)->Checksum))));
  }

  return EFI_SUCCESS;
}

/**

  Update Switchable Graphics own operation region for ASL (ACPI Source Language) code usage.

  @param[in] SgOpRegion      A pointer of SG own operation region.
  @param[in] SgDxeInfoData   A pointer of SG DXE information data structure,
                             this driver or SG Operation Region will use these data.
  @param[in] SgInfoDataHob   A pointer of SG information data HOB,
                             SG PEI Module created this HOB, and passes the data from PEI phase.

  @retval None.

**/
STATIC
VOID
UpdateSgOpRegion (
  IN OUT OPERATION_REGION_SG                  *SgOpRegion,
  IN SG_DXE_INFORMATION_DATA                  *SgDxeInfoData,
  IN SG_INFORMATION_DATA_HOB                  *SgInfoDataHob
  )
{
  UINT8                                       CapOffset;
  UINTN                                       PciAddress;

  //
  // SG Mode for ASL usage
  //
  SgOpRegion->GpioBaseAddress      = SgDxeInfoData->GpioBaseAddress;
  PciAddress = PCI_EXPRESS_LIB_ADDRESS (
                 SgInfoDataHob->DgpuBridgeBus,
                 SgInfoDataHob->DgpuBridgeDevice,
                 SgInfoDataHob->DgpuBridgeFunction,
                 0x0
                 );
  SgOpRegion->DgpuBridgePcieBaseAddress = (UINT32)(PcdGet64 (PcdPciExpressBaseAddress) + PciAddress);
  SgDxeInfoData->SgOpRegionAddress = SgOpRegion;

  if (SgInfoDataHob->SgMode == SgModeMuxless) {
    if (SgInfoDataHob->SgGpioSupport)  {
      //
      // GPIO Assignment for ASL usage
      //
      SgOpRegion->SgGPIOSupport   = SgInfoDataHob->SgGpioSupport;
      SgOpRegion->ActiveDgpuPwrEnableDelayTime = SgInfoDataHob->ActiveDgpuPwrEnableDelay;
      SgOpRegion->ActiveDgpuHoldRstDelayTime   = SgInfoDataHob->ActiveDgpuHoldRstDelay;
      SgOpRegion->InctiveDgpuHoldRstDelayTime  = SgInfoDataHob->InactiveDgpuHoldRstDelay;
      
      //
      // GPIO Pin Setting
      //
      SgOpRegion->HoldRstGPIOCommunityOffset      = SgInfoDataHob->SgDgpuHoldRstGpioCommunityOffset;
      SgOpRegion->HoldRstGPIOPinOffset            = SgInfoDataHob->SgDgpuHoldRstGpioPinOffset;
      SgOpRegion->HoldRstGPIOActiveInfo           = SgInfoDataHob->SgDgpuHoldRstGpioPinActiveInformation;
      SgOpRegion->PowerEnableGPIOCommunityOffset  = SgInfoDataHob->SgDgpuPwrEnableGpioCommunityOffset;
      SgOpRegion->PowerEnableGPIOPinOffset        = SgInfoDataHob->SgDgpuPwrEnableGpioPinOffset;
      SgOpRegion->PowerEnableGPIOActiveInfo       = SgInfoDataHob->SgDgpuPwrEnableGpioPinActiveInformation;
      SgOpRegion->SgDgpuEnable                    = SgInfoDataHob->SgDgpuEnable;
      SgOpRegion->SgDgpuDisable                   = SgInfoDataHob->SgDgpuDisable;
      SgOpRegion->SgDgpuPEnable                   = SgInfoDataHob->SgDgpuPEnable;
      SgOpRegion->SgDgpuPDisable                  = SgInfoDataHob->SgDgpuPDisable;
    }

    //
    // PEG Endpoint Base Addresses and Capability Structure Offsets for ASL usage
    //
    PciAddress = PCI_EXPRESS_LIB_ADDRESS (
                   SgDxeInfoData->DgpuBus,
                   DGPU_DEVICE_NUM,
                   DGPU_FUNCTION_NUM,
                   0x0
                   );
    SgOpRegion->DgpuDevicePcieBaseAddress = (UINT32)(PcdGet64 (PcdPciExpressBaseAddress) + PciAddress);
    SgOpRegion->PcieBaseAddress = (UINT32)(PcdGet64 (PcdPciExpressBaseAddress));
    PciFindCapId (SgDxeInfoData->DgpuBus, DGPU_DEVICE_NUM, DGPU_FUNCTION_NUM, PEG_CAP_ID, &CapOffset);
    SgOpRegion->EndpointPcieCapBaseAddress = CapOffset;
  }
}

/**

  Update AMD own operation region for ASL (ACPI Source Language) code usage.

  @param[in] AmdOpRegion     A pointer of AMD own operation region.
  @param[in] SgDxeInfoData   A pointer of SG DXE information data structure,
                             this driver or SG Operation Region will use these data.
  @param[in] SgInfoDataHob   A pointer of SG information data HOB,
                             SG PEI Module created this HOB, and passes the data from PEI phase.

  @retval None.

**/
STATIC
VOID
UpdateAmdOpRegion (
  IN OUT OPERATION_REGION_AMD                 *AmdOpRegion,
  IN SG_DXE_INFORMATION_DATA                  *SgDxeInfoData,
  IN SG_INFORMATION_DATA_HOB                  *SgInfoDataHob
  )
{
  if (SgInfoDataHob->SgMode == SgModeMuxless) {
    AmdOpRegion->Reserved = 0;
  }
}

/**

  Update nVIDIA own operation region for ASL (ACPI Source Language) code usage.

  @param[in] NvidiaOpRegion   A pointer of nVIDIA own operation region.
  @param[in] SgDxeInfoData    A pointer of SG DXE information data structure,
                              this driver or SG Operation Region will use these data.
  @param[in] SgInfoDataHob    A pointer of SG information data HOB,
                              SG PEI Module created this HOB, and passes the data from PEI phase.

  @retval None.

**/
STATIC
VOID
UpdateNvidiaOpRegion (
  IN OUT OPERATION_REGION_NVIDIA              *NvidiaOpRegion,
  IN SG_DXE_INFORMATION_DATA                  *SgDxeInfoData,
  IN SG_INFORMATION_DATA_HOB                  *SgInfoDataHob
  )
{
  if (SgInfoDataHob->SgMode == SgModeMuxless) {
    NvidiaOpRegion->DgpuHotPlugSupport      = SgInfoDataHob->OptimusDgpuHotPlugSupport;
    NvidiaOpRegion->DgpuPowerControlSupport = SgInfoDataHob->OptimusDgpuPowerControlSupport;
    NvidiaOpRegion->GpsFeatureSupport       = SgInfoDataHob->GpsFeatureSupport;
    NvidiaOpRegion->OptimusGc6Support       = SgInfoDataHob->OptimusGc6FeatureSupport;
    NvidiaOpRegion->MxmBinarySize = (UINT32)SgDxeInfoData->MxmBinFile.Size; // MXM bin file size (bits)
    //
    // Copy MXM bin file to OpRegion
    //
    CopyMem (
      (VOID *)(NvidiaOpRegion->MxmBinaryBuffer),
      (VOID *)(SgDxeInfoData->MxmBinFile.Address),
      SgDxeInfoData->MxmBinFile.Size
      );
  }
}

/**

  Set Secondary Graphics Command Register for avoid I/O resource crash.

  @param[in]  Event     The Event this notify function registered to.
  @param[in]  Context   Pointer to the context data registerd to the Event.

  @retval EFI_SUCCESS   Set secondary Graphics command register successfully.

**/
VOID
EFIAPI
SetSecondaryGrcphicsCommandRegister (
  IN EFI_EVENT                                Event,
  IN VOID                                     *Context
  )
{
  SG_DXE_INFORMATION_DATA                     *SgDxeInfoData;
  UINT8                                       Data;
  UINTN                                       PciAddress;

  SgDxeInfoData = (SG_DXE_INFORMATION_DATA *)Context;
  if (SgDxeInfoData == NULL) {
    return;
  }
  Data = 0x07;
  //
  // If dGPU and iGPU exist at the same time, close dGPU IO port.
  // Different vendor requests to write command register with different value.
  //
  if ((SgDxeInfoData->DgpuVendorId) != 0xFFFF) {
    PciAddress = PCI_EXPRESS_LIB_ADDRESS (
                   SgDxeInfoData->DgpuBus,
                   DGPU_DEVICE_NUM,
                   DGPU_FUNCTION_NUM,
                   PCI_CMD
                   );
    if (SgDxeInfoData->DgpuVendorId == NVIDIA_VID) {
      Data = SgDxeInfoData->NvidiaSecondaryGrcphicsCommandRegister;
    }
    if (SgDxeInfoData->DgpuVendorId == AMD_VID) {
      Data = SgDxeInfoData->AmdSecondaryGrcphicsCommandRegister;
    }
    PciExpressAndThenOr8 (PciAddress, Data, Data);
  }

  SetDgpuSsidSvid (SgDxeInfoData);
  PciAddress = PCI_EXPRESS_LIB_ADDRESS (
                 SgDxeInfoData->DgpuBus,
                 DGPU_DEVICE_NUM,
                 DGPU_FUNCTION_NUM,
                 PCI_SVID
                 );
  ((OPERATION_REGION_SG *)(SgDxeInfoData->SgOpRegionAddress))->DgpuSsidSvid = PciExpressRead32 (PciAddress);

  FreeSgDxeInfoData (&SgDxeInfoData);
  gBS->CloseEvent (Event);

  return;
}

/**

  Close Discrete GPU HD Audio device.
  Close nVIDIA slave HD Audio device.
    Discrete GPU memory mapping register 0x488 Bit 25 is HD audio device power enable bit.
  Close AMD slave HD Audio device.
    1. io_wr offset=0x0, data=0x541C
    2. rd_data = io_rd  offset=0x4
    3. io_wr offset=0x4, data=rd_data & ~0x80 (for masking off STRAP_BIF_AUDIO_EN)
    For reference, here's the register composition:
    BIF_PINSTRAP0 <BIFDEC:0x541C> 32
    {
      STRAP_BIF_GEN2_EN_A             0     NUM DEF=1;  
      STRAP_BIF_CLK_PM_EN             1     NUM DEF=0;
      STRAP_BIF_BIOS_ROM_EN           2     NUM DEF=0;
      STRAP_BIF_RX_PLL_CALIB_BYPASS   3     NUM DEF=0;
      STRAP_BIF_MEM_AP_SIZE_PIN       6:4   NUM DEF=0;
      STRAP_BIF_AUDIO_EN_PIN          7     NUM DEF=0;
      STRAP_BIF_VGA_DIS_PIN           8     NUM DEF=0;
      STRAP_TX_DEEMPH_EN              9     NUM DEF=1;
      STRAP_TX_PWRS_ENB               10    NUM DEF=1;
    }   

  @param[in]  Event     The Event this notify function registered to.
  @param[in]  Context   Pointer to the context data registerd to the Event.

  @retval EFI_SUCCESS   Set secondary Graphics command register successfully.

**/
VOID
EFIAPI
CloseDiscreteHdAudio (
  IN EFI_EVENT                                Event,
  IN VOID                                     *Context
  )
{
  SG_DXE_INFORMATION_DATA                     *SgDxeInfoData;
  UINT16                                      SlavePegIoAddress;
  UINTN                                       PciAddress;

  SgDxeInfoData = (SG_DXE_INFORMATION_DATA *)Context;
  if (SgDxeInfoData == NULL) {
    return;
  }

  if (SgDxeInfoData->DgpuVendorId == NVIDIA_VID) {
    PciAddress = PCI_EXPRESS_LIB_ADDRESS (
                   SgDxeInfoData->DgpuBus,
                   DGPU_DEVICE_NUM,
                   DGPU_FUNCTION_NUM,
                   NVIDIA_DGPU_HDA_REGISTER
                   );
    PciExpressAnd32 (PciAddress, (UINT32)~(BIT25));
  } else if (SgDxeInfoData->DgpuVendorId == AMD_VID) {
    PciAddress = PCI_EXPRESS_LIB_ADDRESS (
                   SgDxeInfoData->DgpuBus,
                   DGPU_DEVICE_NUM,
                   DGPU_FUNCTION_NUM,
                   PCI_BAR4
                   );
    SlavePegIoAddress = PciExpressRead16 (PciAddress);
    SlavePegIoAddress &= ~(BIT0);
    IoWrite32 (SlavePegIoAddress, 0x541C);
    IoWrite32 ((SlavePegIoAddress + 4), (IoRead32 (SlavePegIoAddress + 4) & ~(BIT7)));
  }

  return;
}

/**

  The function is for disable dGPU bridge hot plug SMI/SCI.

  @param[in] SgInfoDataHob   A pointer of SG information data HOB,
                             SG PEI Module created this HOB, and passes the data from PEI phase.

  @retval None.

**/
STATIC
VOID
DisableDgpuBridgeHotplugSmi (
  IN  SG_INFORMATION_DATA_HOB                 *SgInfoDataHob
  )
{
  UINT32                                      Data32;
  UINTN                                       PciAddress;

  //
  // Program Misc Port Config (MPC) register at PCI config space offset
  // D8h as follows:
  // Hot Plug SMI Enable (HPME, bit1) = 0b : Disable
  // Hot Plug SMI Enable (HPME, bit1) = 1b : Enable
  //
  PciAddress = PCI_EXPRESS_LIB_ADDRESS (
                 SgInfoDataHob->DgpuBridgeBus,
                 SgInfoDataHob->DgpuBridgeDevice,
                 SgInfoDataHob->DgpuBridgeFunction,
                 R_PCH_PCIE_MPC
                 );

  Data32 = PciExpressRead32 (PciAddress);
  Data32 &= (~(B_PCH_PCIE_MPC_PMCE | B_PCH_PCIE_MPC_HPCE | B_PCH_PCIE_MPC_HPME)); // Disable Hotplug SCI/SMI
  PciExpressWrite32(PciAddress, Data32);

}

/**

  The function is for set dGPU SSID/SVID.

  @param[in] SgDxeInfoData   A pointer of SG DXE information data structure,
                             this driver or SG Operation Region will use these data.

  @retval None.

**/
STATIC
VOID
SetDgpuSsidSvid (
  IN  SG_DXE_INFORMATION_DATA                 *SgDxeInfoData
  )
{
  UINT8                                       SsidSvidOffset;
  UINT32                                      SsidSvid;
  UINT64                                      PciAddress;

  SsidSvid = PcdGet32 (PcdDefaultSsidSvid);
  SsidSvidOffset = PCI_SVID;
  if (SgDxeInfoData->DgpuVendorId == NVIDIA_VID) {
    SsidSvidOffset = NVIDIA_DGPU_SSID_SVID_OFFSET;
    SsidSvid = PcdGet32 (PcdSgNvidiaSsdiSvid);
  }
  if (SgDxeInfoData->DgpuVendorId == AMD_VID) {
    SsidSvidOffset = AMD_DGPU_SSID_SVID_OFFSET;
    SsidSvid = PcdGet32 (PcdSgAmdSsdiSvid);
  }
  PciAddress = PCI_EXPRESS_LIB_ADDRESS (
                 SgDxeInfoData->DgpuBus,
                 DGPU_DEVICE_NUM,
                 DGPU_FUNCTION_NUM, 
                 SsidSvidOffset
                 );
  PciExpressWrite32 ((UINTN)PciAddress, SsidSvid);

  PciAddress = S3_BOOT_SCRIPT_LIB_PCI_ADDRESS (
                 SgDxeInfoData->DgpuBus,
                 DGPU_DEVICE_NUM,
                 DGPU_FUNCTION_NUM, 
                 SsidSvidOffset
                 );
  S3BootScriptSavePciCfgWrite (S3BootScriptWidthUint32, PciAddress, 1, &SsidSvid);

  return;
}

BOOLEAN
IsAmlOpRegionObject (
  IN UINT8                                    *DsdtPointer
  )
{
  UINT16                                      *Operation = NULL;
  
  Operation = (UINT16*)(DsdtPointer - 2);
  if (*Operation == AML_OPREGION_OP ) {
    
    return TRUE;
  } 
  
  return FALSE;   	
}
 
/**

 Update the OperationRegion of a system memory's offset and size

 @param [in]   DsdtPointer         A pointer to the address where the Operation's Name.
                                   Assume it is four letters in ASL, such as 'GNVS'
 @param [in]   RegionStartAddress  The address where the Operation Region allocated in system memory (AcpiNvsArea).
                                   It is assume to be a DWORD.
 @param [in]   RegionSize

 @retval TRUE                   If the object is Name(ABCD, Package(){}), the *PkgLength, *NumElement will be updated

**/
EFI_STATUS
SetOpRegion (
  IN UINT8                                    *DsdtPointer,
  IN VOID*                                    RegionStartAddress,
  IN UINT32                                   RegionSize
  )
{
  UINT8                                       AddressPrefix;
  UINT32                                      *Address;
  UINT32                                      *DwordSize;
  UINT8                                       SizePrefix;
  UINT16                                      *WordSize;
  UINT8                                       *ByteSize;

  AddressPrefix = *(DsdtPointer + 5);
  ASSERT (AddressPrefix == AML_DWORD_PREFIX);
  Address = (UINT32*) (DsdtPointer + 6);
  ASSERT ((((UINTN)RegionStartAddress) & 0xFFFFFFFF) == (UINTN)RegionStartAddress);
  *Address = (UINT32)(UINTN)RegionStartAddress;
  SizePrefix = *(DsdtPointer + 10);
  if (SizePrefix == AML_DWORD_PREFIX) {
    DwordSize = (UINT32*) (DsdtPointer + 11);
    *DwordSize = RegionSize;
  } else if (SizePrefix == AML_WORD_PREFIX) {
    ASSERT (RegionSize < 0x10000);
    RegionSize &= 0xFFFF;
    WordSize = (UINT16*) (DsdtPointer + 11);
    *WordSize = (UINT16) RegionSize;
  } else if (SizePrefix == AML_BYTE_PREFIX) {
    ASSERT (RegionSize < 0x100);
    RegionSize &= 0xFF;
    ByteSize = (UINT8*) (DsdtPointer + 11);
    *ByteSize = (UINT8) RegionSize;
  } else {
    CpuDeadLoop ();  // Wrong assignment could corrupt the DSDT
  }

  return EFI_SUCCESS;
}

/**
 This function calculates and updates an UINT8 checksum.

 @param [in]   Buffer           Pointer to buffer to checksum
 @param [in]   Size             Number of bytes to checksum
 @param [in]   ChecksumOffset   Offset to place the checksum result in

 @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
AcpiChecksum (
  IN VOID                                     *Buffer,
  IN UINTN                                    Size,
  IN UINTN                                    ChecksumOffset
  )
{
  UINT8                                       Sum;
  UINT8                                       *Ptr;
 
  Sum = 0;
  //
  // Initialize pointer
  //
  Ptr = Buffer;
 
  //
  // set checksum to 0 first
  //
  Ptr[ChecksumOffset] = 0;
 
  //
  // add all content of buffer
  //
  while (Size--) {
    Sum = (UINT8) (Sum + (*Ptr++));
  }
  //
  // set checksum
  //
  Ptr                 = Buffer;
  Ptr[ChecksumOffset] = (UINT8) (0xff - Sum + 1);
 
  return EFI_SUCCESS;
}

/**

  The function is for free SgDxeInfoData pointer.

  @param[in, out] SgDxeInfoData   A double pointer of SG DXE information data structure.

  @retval None.

**/
VOID
EFIAPI
FreeSgDxeInfoData (
  IN OUT SG_DXE_INFORMATION_DATA              **SgDxeInfoData
  )
{
  if ((*SgDxeInfoData) != NULL) {
    FreePool (*SgDxeInfoData);
    (*SgDxeInfoData) = NULL;
  }
}

/** @file
  AhciPciHcPei driver is used to provide platform-dependent info, mainly AHCI host controller
  MMIO base, to upper layer AHCI drivers.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "PchAhci.h"

PEI_AHCI_CONTROLLER_PPI mAhciControllerPpi = { GetAhciMemBase };

EFI_PEI_PPI_DESCRIPTOR mPpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiAhciControllerPpiGuid,
  &mAhciControllerPpi
};

/**
  Register notify ppi to reset the AHCI.

  @param[in] PeiServices       An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation.
  @param[in] NotifyDescriptor  Address of the notification descriptor data structure.
  @param[in] Ppi               Address of the PPI that was installed.

  @retval EFI_SUCCESS          Procedure complete.
  @return Others               Error occurs.

**/
EFI_STATUS
EFIAPI
AhciEndOfPeiPpiNotifyCallback (
  IN EFI_PEI_SERVICES              **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR     *NotifyDescriptor,
  IN VOID                          *Ppi
  )
{
  EFI_STATUS                Status;
  AHCI_HC_PEI_PRIVATE_DATA  *AhcDev;
  EFI_PEI_STALL_PPI         *StallPpi;
  UINT8                     Index;
  UINT8                     *BaseAddress;
  UINT32                    Command;

  AhcDev = AHCI_HC_PEI_PRIVATE_DATA_FROM_NOTIFY_DESC (NotifyDescriptor);

  Status = PeiServicesLocatePpi (&gEfiPeiStallPpiGuid, 0, NULL, (VOID **) &StallPpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to allocate gEfiPeiStallPpiGuid for HC reset Notify! \n"));
    return EFI_UNSUPPORTED;
  }

  for (Index = 0; Index < AhcDev->TotalAhciHcs; Index++) {
    BaseAddress = (UINT8*) AhcDev->AhciHcPciAddr[Index];
    BaseAddress = BaseAddress + (*(UINT32*)(UINTN)BaseAddress & 0xff);
    //
    // Halt HC first
    //
    Command = *(UINT32*)(UINTN)BaseAddress;
    Command &= ~0x01;
    *(UINT32*)(UINTN)BaseAddress = Command;
    //
    // Stall 1 millisecond
    //
    Status = StallPpi->Stall (PeiServices, StallPpi, 1000*1000);
    //
    // HCReset
    //
    Command = *(UINT32*)(UINTN)BaseAddress;
    Command |= 0x02;
    *(UINT32*)(UINTN)BaseAddress = Command;
  }

  return EFI_SUCCESS;
}

/**
  Get the MMIO base address of AHCI host controller.

  @param[in]  PeiServices        The pointer to the PEI Services Table.
  @param[in]  This               The pointer to this instance of the PEI_AHCI_CONTROLLER_PPI.
  @param[in]  AhciControllerId   The ID of the AHCI host controller.
  @param[out] AhciMemBaseAddr    Pointer to the AHCI host controller MMIO base address.

  @retval EFI_SUCCESS            The operation succeeds.
  @retval EFI_INVALID_PARAMETER  The parameters are invalid.

**/
EFI_STATUS
EFIAPI
GetAhciMemBase (
  IN  EFI_PEI_SERVICES         **PeiServices,
  IN  PEI_AHCI_CONTROLLER_PPI  *This,
  IN  UINT8                    AhciControllerId,
  OUT EFI_PHYSICAL_ADDRESS     *AhciMemBaseAddr
  )
{
  AHCI_HC_PEI_PRIVATE_DATA     *AhcDev;

  if ((This == NULL) || (AhciMemBaseAddr == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  AhcDev = AHCI_HC_PEI_PRIVATE_DATA_FROM_THIS (This);

  if (AhciControllerId >= AhcDev->TotalAhciHcs) {
    return EFI_INVALID_PARAMETER;
  }

  *AhciMemBaseAddr = AhcDev->AhciHcPciAddr[AhciControllerId];

  return EFI_SUCCESS;
}

/**
  @param FileHandle     Handle of the file being invoked.
  @param PeiServices    Describes the list of possible PEI Services.

  @retval EFI_SUCCESS   PPI successfully installed.

**/
EFI_STATUS
EFIAPI
InitializePchAhci (
  IN EFI_PEI_FILE_HANDLE    FileHandle,
  IN CONST EFI_PEI_SERVICES **PeiServices
  )
{
  EFI_BOOT_MODE             BootMode;
  EFI_STATUS                Status;
  UINT16                    Bus;
  UINT16                    Device;
  UINT16                    Function;
  UINT32                    Size;
  UINT8                     ProgIF;
  UINT8                     SubClass;
  UINT8                     BaseClass;
  AHCI_HC_PEI_PRIVATE_DATA  *AhcDev;

  //
  // Shadow this PEIM to run from memory.
  //
  if (!EFI_ERROR (PeiServicesRegisterForShadow (FileHandle))) {
    return EFI_SUCCESS;
  }

  Status = PeiServicesGetBootMode (&BootMode);
  //
  // We do not export this in S3 boot path, because it is only for recovery.
  //
  if (BootMode == BOOT_ON_S3_RESUME) {
    return EFI_SUCCESS;
  }

  AhcDev = (AHCI_HC_PEI_PRIVATE_DATA *) AllocateZeroPool (sizeof (AHCI_HC_PEI_PRIVATE_DATA));
  if (AhcDev == NULL) {
    DEBUG ((EFI_D_ERROR, "Failed to allocate memory for AHCI_HC_PEI_PRIVATE_DATA! \n"));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize Private Data Buffer
  //
  AhcDev->Signature         = AHCI_HC_PEI_SIGNATURE;
  AhcDev->AhciControllerPpi = mAhciControllerPpi;
  AhcDev->PpiDescriptor     = mPpiList;
  AhcDev->PpiDescriptor.Ppi = &AhcDev->AhciControllerPpi;

  for (Bus = 0; Bus < 256; Bus++) {
    for (Device = 0; Device < 32; Device++) {
      for (Function = 0; Function < 8; Function++) {
        ProgIF    = PciRead8 (PCI_LIB_ADDRESS (Bus, Device, Function, 0x09));
        SubClass  = PciRead8 (PCI_LIB_ADDRESS (Bus, Device, Function, 0x0A));
        BaseClass = PciRead8 (PCI_LIB_ADDRESS (Bus, Device, Function, 0x0B));

        if ((ProgIF == PCI_IF_MASS_STORAGE_AHCI) && (SubClass == PCI_CLASS_MASS_STORAGE_SATADPA) && (BaseClass == PCI_CLASS_MASS_STORAGE)) {
          //
          // Get the Ahci Pci host controller's MMIO region size.
          //
          PciAnd16 (PCI_LIB_ADDRESS (Bus, Device, Function, PCI_COMMAND_OFFSET), (UINT16)~(EFI_PCI_COMMAND_BUS_MASTER | EFI_PCI_COMMAND_MEMORY_SPACE));
          PciWrite32 (PCI_LIB_ADDRESS (Bus, Device, Function, PCI_BASE_ADDRESSREG_OFFSET+0x14), 0xFFFFFFFF);
          Size = ~(PciRead32 (PCI_LIB_ADDRESS (Bus, Device, Function, PCI_BASE_ADDRESSREG_OFFSET+0x14)) & 0xFFFFFFF0) + 1;
          //
          // Assign resource to the Ahci Pci host controller's MMIO BAR.
          // Enable the Ahci Pci host controller by setting BME and MSE bits of PCI_CMD register.
          //
          PciWrite32 (PCI_LIB_ADDRESS (Bus, Device, Function, PCI_BASE_ADDRESSREG_OFFSET+0x14), (UINT32)(AHCI_PCI_HOST_CONTROLLER_MMIO_BASE + Size * AhcDev->TotalAhciHcs));
          PciOr16 (PCI_LIB_ADDRESS (Bus, Device, Function, PCI_COMMAND_OFFSET), (EFI_PCI_COMMAND_BUS_MASTER | EFI_PCI_COMMAND_MEMORY_SPACE));
          //
          // Record the allocated Mmio base address.
          //
          AhcDev->AhciHcPciAddr[AhcDev->TotalAhciHcs] = AHCI_PCI_HOST_CONTROLLER_MMIO_BASE + Size * AhcDev->TotalAhciHcs;
          AhcDev->TotalAhciHcs++;
          ASSERT (AhcDev->TotalAhciHcs < MAX_AHCI_HCS);
        }
      }
    }
  }

  //
  // Install Ahci Controller Ppi
  //
  Status = PeiServicesInstallPpi (&AhcDev->PpiDescriptor);
  ASSERT_EFI_ERROR (Status);

  //
  // Install Notification to reset the AhciHc if needed
  //
  AhcDev->NotifyDescriptor.Flags  = (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);
  AhcDev->NotifyDescriptor.Guid   = &gEfiEndOfPeiSignalPpiGuid;
  AhcDev->NotifyDescriptor.Notify = AhciEndOfPeiPpiNotifyCallback;

  Status = PeiServicesNotifyPpi (&AhcDev->NotifyDescriptor);
  ASSERT_EFI_ERROR (Status);

  return Status;
}

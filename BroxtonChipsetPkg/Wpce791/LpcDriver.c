/*++

Copyright (c)  1999 - 2016 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    LpcDriver.c

Abstract:

    EFI Lpc Driver for a Generic PC Platform


--*/

#include "LpcDriver.h"
#include "IndustryStandard/Pci22.h"
//
// This driver is for ACPI(PNP0A03,0)/PCI(0x1f,0)
//

//
//  Lpc Driver Global Variables
//
//[-start-160216-IB03090424-modify]//

EFI_DRIVER_BINDING_PROTOCOL gLpcDriver = {
  LpcDriverSupported,
  LpcDriverStart,
  LpcDriverStop,
  0x10,
  NULL,
  NULL
};

LPC_DEV mLpc = {
  LPC_DEV_SIGNATURE,
  NULL,
  {
    IsaDeviceEnumerate,
    IsaDeviceSetPower,
    IsaGetCurrentResource,
    IsaGetPossibleResource,
    IsaSetResource,
    IsaEnableDevice,
    IsaInitDevice,
    LpcInterfaceInit
  },
  NULL
};


/**
  Super I/O controller initialization.

  @retval     EFI_SUCCESS       The super I/O controller is found and initialized.
  @retval     EFI_UNSUPPORTED   The super I/O controller is not found.
**/
EFI_STATUS
SioInit (
  VOID
  )
{
  LPCWPCE791SetDefault ();
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
LpcDriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

  Routine Description:
    the entry point of the Lpc driver

  Arguments:

  Returns:

--*/
{
  if (EFI_ERROR (SioInit())) {
    return EFI_UNSUPPORTED;
  } else {
    return EfiLibInstallDriverBinding (ImageHandle, SystemTable, &gLpcDriver, ImageHandle);
  }
}


EFI_STATUS
EFIAPI
LpcDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )

/*++

Routine Description:

  ControllerDriver Protocol Method

Arguments:

Returns:

--*/

{
  EFI_STATUS                Status;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  EFI_DEVICE_PATH_PROTOCOL  *IsaBridgeDevicePath;

  ACPI_HID_DEVICE_PATH      *AcpiNode;
  PCI_DEVICE_PATH           *PciNode;
  PCI_TYPE00                Pci;

  //
  // Get the ISA bridge's Device Path and test it
  // the following code is specific
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **)&IsaBridgeDevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EFI_SUCCESS;
  AcpiNode =  (ACPI_HID_DEVICE_PATH *)IsaBridgeDevicePath;
  if (AcpiNode->Header.Type != ACPI_DEVICE_PATH ||
      AcpiNode->Header.SubType != ACPI_DP ||
      DevicePathNodeLength (&AcpiNode->Header) != sizeof(ACPI_HID_DEVICE_PATH) ||
      AcpiNode -> HID != EISA_PNP_ID(0x0A03) ||
      AcpiNode -> UID != 0 ) {
    Status = EFI_UNSUPPORTED;
  } else {
    //
    // Get the next node
    //
    IsaBridgeDevicePath = NextDevicePathNode (IsaBridgeDevicePath);
    PciNode  = (PCI_DEVICE_PATH *)IsaBridgeDevicePath;
    if (PciNode->Header.Type != HARDWARE_DEVICE_PATH ||
        PciNode->Header.SubType != HW_PCI_DP ||
        DevicePathNodeLength (&PciNode->Header) != sizeof (PCI_DEVICE_PATH) ||
        PciNode -> Function != 0x00 ||
        PciNode -> Device != 0x1f ) {
      Status = EFI_UNSUPPORTED;
    }
  }

  gBS->CloseProtocol (
         Controller,
         &gEfiDevicePathProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Get PciIo protocol instance
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **)&PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        sizeof(Pci) / sizeof(UINT32),
                        &Pci
                        );

  if (!EFI_ERROR (Status)) {
    Status = EFI_SUCCESS; //TODO: force return success as temp solution EFI_UNSUPPORTED;
    if ((Pci.Hdr.Command & 0x03) == 0x03) {
      if (Pci.Hdr.ClassCode[2] == PCI_CLASS_BRIDGE) {
        //
        // See if this is a standard PCI to ISA Bridge from the Base Code
        // and Class Code
        //
        if (Pci.Hdr.ClassCode[1] == PCI_CLASS_BRIDGE_ISA) {
          Status = EFI_SUCCESS;
        } else {
        }

        //
        // See if this is an Intel PCI to ISA bridge in Positive Decode Mode
        //
        if (Pci.Hdr.ClassCode[1] == PCI_CLASS_BRIDGE_ISA_PDECODE &&
            Pci.Hdr.VendorId == 0x8086 &&
            Pci.Hdr.DeviceId == 0x7110) {
          Status = EFI_SUCCESS;
        } else {
        }
      } else {
      }
    } else {
    }
  }

  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );
  return Status;
}


EFI_STATUS
EFIAPI
LpcDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )

/*++

Routine Description:
  Install EFI_ISA_ACPI_PROTOCOL

Arguments:

Returns:

--*/

{
  EFI_STATUS             Status;
  EFI_PCI_IO_PROTOCOL    *PciIo;
  LPC_DEV                *LpcDev;


  LpcDev = NULL;

  //
  // Get Pci IO
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **)&PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    return Status;
  }

  mLpc.PciIo = PciIo;

  //
  // Install IsaAcpi interface, the Sio interface is not installed!
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiIsaAcpiProtocolGuid,
                  &mLpc.IsaAcpi,
                  NULL
                  );
  return Status;
}


EFI_STATUS
EFIAPI
LpcDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )

/*++

  Routine Description:

  Arguments:

  Returns:

--*/

{
  EFI_STATUS             Status;
  EFI_ISA_ACPI_PROTOCOL  *IsaAcpi;
  LPC_DEV                *LpcDev;

  //
  // Get EFI_ISA_ACPI_PROTOCOL interface
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiIsaAcpiProtocolGuid,
                  (VOID **)&IsaAcpi,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  LpcDev = LPC_ISA_ACPI_FROM_THIS (IsaAcpi);

  //
  // Uninstall protocol interface: EFI_ISA_ACPI_PROTOCOL
  //
  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiIsaAcpiProtocolGuid,
                  &LpcDev->IsaAcpi
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
                   Controller,
                   &gEfiPciIoProtocolGuid,
                   This->DriverBindingHandle,
                   Controller
                   );

  return EFI_SUCCESS;
}
//[-end-160216-IB03090424-modify]//

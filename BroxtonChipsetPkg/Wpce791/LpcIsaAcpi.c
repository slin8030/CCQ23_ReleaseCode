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

    LpcIsaAcpi.c

Abstract: IsaAcpi implementation



--*/

#include "LpcDriver.h"

//
// PS/2 Keyboard Controller
//
static EFI_ISA_ACPI_RESOURCE  mLpcWpce791Ps2KeyboardDeviceResources[] = {
  {EfiIsaAcpiResourceIo,        0, 0x60, 0x64},
  {EfiIsaAcpiResourceInterrupt, 0, 1,     0},
  {EfiIsaAcpiResourceEndOfList, 0, 0,     0}
};

//
// PS/2 Mouse Controller
//
static EFI_ISA_ACPI_RESOURCE  mLpcWpce791Ps2MouseDeviceResources[] = {
  {EfiIsaAcpiResourceIo,        0, 0x60, 0x64},
  {EfiIsaAcpiResourceInterrupt, 0, 12,     0},
  {EfiIsaAcpiResourceEndOfList, 0, 0,     0}
};

//
// COM
//
static EFI_ISA_ACPI_RESOURCE  mLpcWpce791ComDeviceResources[] = {
  {EfiIsaAcpiResourceIo,        0, 0x3f8, 0x3ff},
  {EfiIsaAcpiResourceInterrupt, 0, 4,     0},
  {EfiIsaAcpiResourceEndOfList, 0, 0,     0}
};

//
// Table of ISA Controllers
//
EFI_ISA_ACPI_RESOURCE_LIST mLpcWpce791DeviceList[] = {
  {{EISA_PNP_ID(0x303), 0}, mLpcWpce791Ps2KeyboardDeviceResources }, // PS/2 Keyboard Controller
  {{EISA_PNP_ID(0xF03), 0}, mLpcWpce791Ps2MouseDeviceResources    }, // PS/2 Mouse Controller
  {{EISA_PNP_ID(0x501), 0}, mLpcWpce791ComDeviceResources         }, // COM
  {{0,                  0}, NULL                                  }  // End
};

static ICH_DMA_INIT  mIchDmaInitTable [] = {
//Register OFFSET,           Value

            0x0D8,           0x000,   // Reset DMA Controller 2
            0x0D0,           0x000,   // Enable DMA controller 2
            0x00C,           0x000,   // Reset DMA Controller 1
            0x008,           0x000,   // Enable DMA controller 1

            // Channel 4
            0x0D6,           0x0c0,   // DMA contr. 2 Cascade mode, addr. increment, disable auto init.
            0x0D2,           0x000,   // Clear write request register
            0x0d4,           0x000,   // Enable DREQs for channel

            // Channel 0
            0x00B,           0x040,   // DMA contr. 1 single mode, addr. increment, disable auto init.
            0x009,           0x000,   // Clear write request register
            0x00A,           0x000,   // Enable DREQs for channel

            // Channel 1
            0x00B,           0x041,   // DMA contr. 1 single mode, addr. increment, disable auto init.
            0x009,           0x001,   // Clear write request register
            0x00A,           0x001,   // Enable DREQs for channel

            // Channel 2
            0x00B,           0x042,   // DMA contr. 1 single mode, addr. increment, disable auto init.
            0x009,           0x002,   // Clear write request register
            0x00A,           0x002,   // Enable DREQs for channel

            // Channel 3
            0x00B,           0x043,   // DMA contr. 1 single mode, addr. increment, disable auto init.
            0x009,           0x003,   // Clear write request register
            0x00A,           0x003,   // Enable DREQs for channel

            // Channel 5
            0x0D6,           0x041,   // DMA contr. 2 single mode, addr. increment, disable auto init.
            0x0D2,           0x001,   // Clear write request register
            0x0D4,           0x001,   // Enable DREQs for channel

            // Channel 6
            0x0D6,           0x042,   // DMA contr. 2 single mode, addr. increment, disable auto init.
            0x0D2,           0x002,   // Clear write request register
            0x0D4,           0x002,   // Enable DREQs for channel

            // Channel 7
            0x0D6,           0x043,   // DMA contr. 2 single mode, addr. increment, disable auto init.
            0x0D2,           0x003,   // Clear write request register
            0x0D4,           0x003    // Enable DREQs for channel

};

//
// ISA ACPI Protocol Functions
//
VOID
IsaDeviceLookup (
  IN  EFI_ISA_ACPI_DEVICE_ID      *Device,
  OUT EFI_ISA_ACPI_RESOURCE_LIST  **IsaAcpiDevice,
  OUT EFI_ISA_ACPI_RESOURCE_LIST  **NextIsaAcpiDevice
  )
/*++

Routine Description:
  Enumerate the ISA devices on the ISA bus

Arguments:

Returns:

--*/
{
  UINTN  Index;

  *IsaAcpiDevice = NULL;
  if (NextIsaAcpiDevice != NULL) {
    *NextIsaAcpiDevice = NULL;
  }
  if (Device == NULL) {
    Index = 0;
  } else {
    for(Index = 0; mLpcWpce791DeviceList[Index].Device.HID != 0; Index++) {
      if (Device->HID == mLpcWpce791DeviceList[Index].Device.HID &&
          Device->UID == mLpcWpce791DeviceList[Index].Device.UID    ) {
        break;
      }
    }
    if (mLpcWpce791DeviceList[Index].Device.HID == 0) {
      return;
    }
    *IsaAcpiDevice = &(mLpcWpce791DeviceList[Index]);
    Index++;
  }
  if (NextIsaAcpiDevice != NULL && mLpcWpce791DeviceList[Index].Device.HID != 0){
    *NextIsaAcpiDevice = &(mLpcWpce791DeviceList[Index]);
  }
}

EFI_STATUS
EFIAPI
IsaDeviceEnumerate (
  IN     EFI_ISA_ACPI_PROTOCOL       *This,
  OUT    EFI_ISA_ACPI_DEVICE_ID      **Device
  )

/*++

Routine Description:
  Enumerate the ISA devices on the ISA bus
  It is hard code now and future it will get from ACPI table

Arguments:

Returns:

--*/

{
  EFI_ISA_ACPI_RESOURCE_LIST  *IsaAcpiDevice;
  EFI_ISA_ACPI_RESOURCE_LIST  *NextIsaAcpiDevice;

  IsaDeviceLookup (*Device, &IsaAcpiDevice, &NextIsaAcpiDevice);
  if (NextIsaAcpiDevice == NULL) {
    return EFI_NOT_FOUND;
  }
  *Device = &(NextIsaAcpiDevice->Device);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
IsaDeviceSetPower (
  IN     EFI_ISA_ACPI_PROTOCOL       *This,
  IN     EFI_ISA_ACPI_DEVICE_ID      *Device,
  IN     BOOLEAN                     OnOff
  )

/*++

Routine Description:
  Set ISA device power use sio

Arguments:

Returns:

--*/

{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
IsaGetCurrentResource (
  IN     EFI_ISA_ACPI_PROTOCOL        *This,
  IN     EFI_ISA_ACPI_DEVICE_ID       *Device,
  OUT    EFI_ISA_ACPI_RESOURCE_LIST   **ResourceList
  )

/*++

Routine Description:
  Get current Resource of the specific ISA device
  It is hardcode now and future will get from ACPI table

Arguments:

Returns:

--*/

{
  IsaDeviceLookup (Device, ResourceList, NULL);
  if (*ResourceList == NULL || (*ResourceList)->ResourceItem == NULL) {
    return EFI_NOT_FOUND;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
IsaGetPossibleResource (
  IN     EFI_ISA_ACPI_PROTOCOL       *This,
  IN     EFI_ISA_ACPI_DEVICE_ID      *Device,
  OUT    EFI_ISA_ACPI_RESOURCE_LIST  **ResourceList
  )

/*++

Routine Description:

Arguments:

Returns:

--*/

{
  //
  // Not supported yet
  //
  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
IsaSetResource (
  IN     EFI_ISA_ACPI_PROTOCOL       *This,
  IN     EFI_ISA_ACPI_DEVICE_ID      *Device,
  IN     EFI_ISA_ACPI_RESOURCE_LIST  *ResourceList
  )

/*++

Routine Description:

Arguments:

Returns:

--*/

{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
IsaEnableDevice (
  IN    EFI_ISA_ACPI_PROTOCOL        *This,
  IN    EFI_ISA_ACPI_DEVICE_ID       *Device,
  IN    BOOLEAN                      Enable
  )

/*++

Routine Description:

Arguments:

Returns:

--*/

{

  return EFI_UNSUPPORTED;
}

VOID
EmptyResourceList (
  IN  UINT32      DeviceHid
  )
/*++

Routine Description:
  Clear out Resource List if device is set to disable by platform policy

Arguments:

Returns:

--*/
{
  UINT8     Index;
  for (Index = 0; mLpcWpce791DeviceList[Index].Device.HID != 0; Index++) {
    if (DeviceHid == mLpcWpce791DeviceList[Index].Device.HID) {
      mLpcWpce791DeviceList[Index].ResourceItem = NULL;
    }
  }
  return;
}

VOID
EmptyResourceListHidUid (
  IN  UINT32      DeviceHid,
  IN  UINT32      DeviceUid
  )
/*++

Routine Description:
  Clear out Resource List if device is set to disable by platform policy

Arguments:

Returns:

--*/
{
  UINT8     Index;
  for (Index = 0; mLpcWpce791DeviceList[Index].Device.HID != 0; Index++) {
    if ((DeviceHid == mLpcWpce791DeviceList[Index].Device.HID) &&
        (DeviceUid == mLpcWpce791DeviceList[Index].Device.UID)) {
      mLpcWpce791DeviceList[Index].ResourceItem = NULL;
    }
  }
  return;
}

EFI_STATUS
EFIAPI
IsaInitDevice (
  IN    EFI_ISA_ACPI_PROTOCOL        *This,
  IN    EFI_ISA_ACPI_DEVICE_ID       *Device
  )

/*++

Routine Description:

Arguments:

Returns:

--*/

{
  EFI_WPCE791_POLICY_PROTOCOL      *LpcWpce791Policy;
  EFI_STATUS                      Status;

  //
  // Disable configuration according to platform protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiLpcWpce791PolicyProtocolGuid,
                  NULL,
                  (VOID **) &LpcWpce791Policy
                  );
  if (!EFI_ERROR(Status)) {
    if (LpcWpce791Policy->DeviceEnables.Ps2Keyboard == EFI_WPCE791_PS2_KEYBOARD_DISABLE) {
      EmptyResourceList(EISA_PNP_ID(0x303));
      DisableLogicalDevice (SIO_KEYBOARD);
      EmptyResourceList(EISA_PNP_ID(0xF03));
      DisableLogicalDevice (SIO_KEYBOARD);
    }
    if (LpcWpce791Policy->DeviceEnables.Ps2Mouse == EFI_WPCE791_PS2_MOUSE_DISABLE) {
      EmptyResourceList(EISA_PNP_ID(0xF03));
      DisableLogicalDevice (SIO_MOUSE);
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
LpcInterfaceInit (
  IN    EFI_ISA_ACPI_PROTOCOL        *This
)

/*++

Routine Description:

Arguments:

Returns:

--*/

{
  EFI_PCI_IO_PROTOCOL             *PciIo;
  UINTN                           Index;

  PciIo = (LPC_ISA_ACPI_FROM_THIS (This))->PciIo;

  //
  // DMA controller initialize
  //
  for (Index=0; Index < (sizeof(mIchDmaInitTable)/sizeof(ICH_DMA_INIT)); Index++) {
    PciIo->Io.Write (
                PciIo,
                EfiPciIoWidthUint8,
                EFI_PCI_IO_PASS_THROUGH_BAR,
                mIchDmaInitTable[Index].Register,
                1,
                &mIchDmaInitTable[Index].Value
                );
  }

  return EFI_SUCCESS;
}


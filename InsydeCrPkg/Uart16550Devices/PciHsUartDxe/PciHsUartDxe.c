/** @file
  This driver will install CRPolicy protocol for reference.

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "PciHsUartDxe.h"

#define PCI_HSU_DEVICE_NAME_SIZE                    50
#define PCI_HSU_DEVICE_NAME                         L"PCI_HS_UART"
#define PCI_HSU_REGISTER_BYTE_WIDTH                 sizeof(UINT32)
#define PCI_HSU_FIFO_DEPTH                          16
#define PCI_HS_UART_DEVICE_DATA_SIGNATURE           SIGNATURE_32 ('P','I','U','R')
#define PCI_HS_UART_DEVICE_DATA_FROM_THIS(This)     CR (This, PCI_HS_UART_DEVICE_DATA, U16550Access, PCI_HS_UART_DEVICE_DATA_SIGNATURE)

//
// data type definitions
//
typedef struct {
  UINT32                            Signature;
  EFI_PCI_IO_PROTOCOL               *PciIo;
  UART_16550_DEVICE_INFO            DeviceInfo;
  H2O_UART_16550_ACCESS_PROTOCOL    U16550Access;
} PCI_HS_UART_DEVICE_DATA;

EFI_DRIVER_BINDING_PROTOCOL gHsUartControllerDriver = {
  PciHsUartControllerDriverSupport,
  PciHsUartControllerDriverStart,
  PciHsUartControllerDriverStop,
  0x0A,
  NULL,
  NULL
};

extern EFI_COMPONENT_NAME_PROTOCOL  gPciHsUartComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL gPciHsUartComponentName2;

EFI_STATUS
PciHsUartControllerDriverSupport (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS              Status;
  EFI_PCI_IO_PROTOCOL     *PciIo;
  PCI_TYPE00              PciHeader;
  PCI_HSUART_LIST         *DevList;
  UINTN                   Seg;
  UINTN                   Bus;
  UINTN                   Dev;
  UINTN                   Fun;

  //
  // Test the PciIo Protocol is not be open by driver.
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = PciIo->GetLocation (PciIo, &Seg, &Bus, &Dev, &Fun);
  if (EFI_ERROR(Status)) {
    goto Exit;
  }

  //
  // Read Pci configuration to Identication the high speed UART
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        0,
                        sizeof (PciHeader),
                        &PciHeader
                        );

  if (EFI_ERROR(Status)) {
    goto Exit;
  }

  //
  // Check Device ID & PFA support
  //
  DevList = FixedPcdGetPtr (PcdH2OPciHsUartDeviceList);

  Status = EFI_UNSUPPORTED;
  while (DevList->VenderID != 0xFFFF) {

    if (DevList->Bus != 0 || DevList->Device != 0 || DevList->Function != 0) {
      //
      // Check PFA
      //
      if (DevList->Bus == Bus && DevList->Device == Dev && DevList->Function == Fun) {
        Status = EFI_SUCCESS;
      } else {
        DevList++;
        continue;
      }
    }

    if (DevList->DeviceID != 0 || DevList->VenderID != 0) {
      //
      // Check Device ID and Vender ID
      //
      if ((DevList->DeviceID == PciHeader.Hdr.DeviceId) && (DevList->VenderID == PciHeader.Hdr.VendorId)) {
        Status = EFI_SUCCESS;
      } else {
        Status = EFI_UNSUPPORTED;
      }
    }

    if (Status == EFI_SUCCESS) {
      break;
    }

    DevList++;
  };

Exit:

  gBS->CloseProtocol (
          Controller,
          &gEfiPciIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );

  return Status;
}

EFI_STATUS
InitPciHsUartDeviceData (
  PCI_HS_UART_DEVICE_DATA     *HsUartDev,
  EFI_PCI_IO_PROTOCOL         *PciIo
  )
{
  EFI_STATUS                Status;
  UART_16550_DEVICE_INFO    *DevInfo;
  PCI_TYPE00                PciHeader;
  UINTN                     Segment;
  UINTN                     Bus;
  UINTN                     Device;
  UINTN                     Function;
  UINTN                     NameSize;
  CHAR16                    *Name;
  UINT16                    UID;
  UINTN                     BaseAddress;

  //
  // Read pci configuration space for setting some field.
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        0,
                        sizeof(PciHeader),
                        &PciHeader
                        );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if ((PciHeader.Device.Bar[0] & BIT0) == 1) {
    //
    // IO port address
    //
    return EFI_UNSUPPORTED;
  } else {
    //
    // 32bit Memory mapping IO address
    //
    BaseAddress = PciHeader.Device.Bar[0] & 0xFFFFFFF0;
  }
  //
  // Convert PCI location to UID
  //
  Status = PciIo->GetLocation (
                    PciIo,
                    &Segment,
                    &Bus,
                    &Device,
                    &Function
                    );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  UID = (UINT16)CREATE_UID (Bus, Device, Function);

  //
  // Set Device Name
  //
  NameSize = PCI_HSU_DEVICE_NAME_SIZE;
  Name     = AllocateZeroPool (NameSize);

  UnicodeSPrint (
    Name,
    NameSize,
    L"%s (0x%x,0x%x)",
    PCI_HSU_DEVICE_NAME,
    Device,
    Function
    );
  //
  // Initialize PCI_HS_UART_DEVICE_DATA structure
  //
  HsUartDev->Signature = PCI_HS_UART_DEVICE_DATA_SIGNATURE;
  HsUartDev->PciIo     = PciIo;

  DevInfo = &HsUartDev->DeviceInfo;
  DevInfo->DeviceName        = Name;
  DevInfo->DeviceType        = PCI_HS_SERIAL_DEVICE;
  DevInfo->UID               = UID;
  DevInfo->RegisterByteWidth = PCI_HSU_REGISTER_BYTE_WIDTH;
  DevInfo->BaseAddressType   = UBAT_MEMORY;
  DevInfo->BaseAddress       = BaseAddress;
  DevInfo->FifoSize          = PCI_HSU_FIFO_DEPTH;
  DevInfo->SerialClockFreq   = (UINTN)FixedPcdGet32(PcdH2OHsUartSerialClock);
  DevInfo->SampleRate        = FixedPcdGet16(PcdH2OHsUartSampleRate);
  DevInfo->LegacySupport     = TRUE;

  HsUartDev->U16550Access.RegRead    = PciHsUartRegRead;
  HsUartDev->U16550Access.RegWrite   = PciHsUartRegWrite;
  HsUartDev->U16550Access.DeviceInfo = DevInfo;

  return EFI_SUCCESS;
}

EFI_STATUS
PciHsUartControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                Status;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  PCI_HS_UART_DEVICE_DATA   *HsUartDev;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Allocate memory for device Private data
  //
  HsUartDev = AllocateZeroPool(sizeof(PCI_HS_UART_DEVICE_DATA));
  if (HsUartDev == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  SetMem (HsUartDev, 0, sizeof (PCI_HS_UART_DEVICE_DATA));

  //
  // Initialize the device Private data
  //
  Status = InitPciHsUartDeviceData (HsUartDev, PciIo);
  if (EFI_ERROR(Status)) {
    goto Error;
  }

  //
  // Install Uart16550Access protocol on the contorller handle
  //
  Status = gBS->InstallProtocolInterface (
                  &Controller,
                  &gH2OUart16550AccessProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &HsUartDev->U16550Access
                  );
  if (EFI_ERROR(Status)) {
    goto Error;
  }

  return EFI_SUCCESS;

Error:

  if (HsUartDev != NULL) {
    FreePool (HsUartDev);
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
PciHsUartControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL        *This,
  IN  EFI_HANDLE                        Controller,
  IN  UINTN                             NumberOfChildren,
  IN  EFI_HANDLE                        *ChildHandleBuffer
  )
{
  EFI_STATUS                  Status;
  VOID                        *Protocol;
  PCI_HS_UART_DEVICE_DATA     *UartDevData;

  Status = gBS->OpenProtocol (
                Controller,
                &gH2OUart16550AccessProtocolGuid,
                (VOID**)&Protocol,
                This->DriverBindingHandle,
                Controller,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  UartDevData = PCI_HS_UART_DEVICE_DATA_FROM_THIS (Protocol);
  Status = gBS->UninstallProtocolInterface (
                Controller,
                &gH2OUart16550AccessProtocolGuid,
                &UartDevData->U16550Access
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

  gBS->FreePool (UartDevData);

  return EFI_SUCCESS;
}

/**
 Install Driver to produce UART_16550_ACCESS protocol.

 @param    ImageHandle
 @param    SystemTable

 @retval EFI_SUCCESS            CrPolicy protocol installed
 @return Other                       No protocol installed.

**/
EFI_STATUS
PciHsUartRegRead (
  H2O_UART_16550_ACCESS_PROTOCOL    *This,
  UINT16                            Index,
  UINT8                             *Data
  )
{
  PCI_HS_UART_DEVICE_DATA   *DevData;
  UINT32                    *Register;
  UINT8                     Width;

  Width   = This->DeviceInfo->RegisterByteWidth;
  DevData = PCI_HS_UART_DEVICE_DATA_FROM_THIS (This);

  Register = (UINT32 *)(UINTN)(DevData->DeviceInfo.BaseAddress + Index * Width);

  *Data = (UINT8) *Register;

  return EFI_SUCCESS;
}

/**
 Install Driver to produce UART_16550_ACCESS protocol.

 @param    ImageHandle
 @param    SystemTable

 @retval EFI_SUCCESS            CrPolicy protocol installed
 @return Other                       No protocol installed.

**/
EFI_STATUS
PciHsUartRegWrite (
  H2O_UART_16550_ACCESS_PROTOCOL    *This,
  UINT16                            Index,
  UINT8                             Data
  )
{
  PCI_HS_UART_DEVICE_DATA   *DevData;
  UINT32                    *Register;
  UINT8                     Width;

  Width   = This->DeviceInfo->RegisterByteWidth;
  DevData = PCI_HS_UART_DEVICE_DATA_FROM_THIS (This);

  Register = (UINT32 *)(UINTN)(DevData->DeviceInfo.BaseAddress + Index * Width);

  *Register = Data;

  return EFI_SUCCESS;
}

/**
 Install Driver to produce UART_16550_ACCESS protocol.

 @param    ImageHandle
 @param    SystemTable

 @retval EFI_SUCCESS            CrPolicy protocol installed
 @return Other                       No protocol installed.

**/
EFI_STATUS
EFIAPI
PciHsUartEntryPoint (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS      Status;

  Status = EfiLibInstallDriverBindingComponentName2 (
               ImageHandle,
               SystemTable,
               &gHsUartControllerDriver,
               ImageHandle,
               &gPciHsUartComponentName,
               &gPciHsUartComponentName2
               );

  ASSERT_EFI_ERROR (Status);

  return Status;

}



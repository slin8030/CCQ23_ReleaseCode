/** @file
    Serial driver for standard UARTS on an ISA bus.

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

#include <IsaUartDxe.h>

#define ISA_DEVICE_NAME_SIZE                        50
#define ISA_DEVICE_NAME                             L"ISA_UART"
#define ISA_UART_REGISTER_BYTE_WIDTH                sizeof(UINT8)
#define ISA_UART_FIFO_DEPTH                         16
#define ISA_UART_SERIAL_CLOCK_FREQUENCY             (1 * 115200 * 16)
#define ISA_UART_SIMPLE_RATE                        16

#define ISA_16550_DEV_SIGNATURE    SIGNATURE_32 ('I', 'S', 'A', 'U')
#define ISA_16550_DEV_FROM_THIS(a) CR (a, ISA_UART_DEVICE_DATA, U16550Access, ISA_16550_DEV_SIGNATURE)

//
// data type definitions
//
typedef struct {
  UINT32                            Signature;
  EFI_ISA_IO_PROTOCOL               *IsaIo;
  UART_16550_DEVICE_INFO            DeviceInfo;
  H2O_UART_16550_ACCESS_PROTOCOL    U16550Access;
} ISA_UART_DEVICE_DATA;

//
// ISA Serial Driver Global Variables
//
EFI_DRIVER_BINDING_PROTOCOL gIsaUartControllerDriver = {
  Isa16550ControllerDriverSupported,
  Isa16550ControllerDriverStart,
  Isa16550ControllerDriverStop,
  0x0A,
  NULL,
  NULL
};

extern EFI_COMPONENT_NAME_PROTOCOL  gIsaUartComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL gIsaUartComponentName2;

/**
  Use IsaIo protocol to read serial port.

  @param IsaIo         Pointer to EFI_ISA_IO_PROTOCOL instance
  @param BaseAddress   Serial port register group base address
  @param Offset        Offset in register group

  @return Data read from serial port

**/
STATIC
UINT8
IsaSerialReadPort (
  IN EFI_ISA_IO_PROTOCOL                   *IsaIo,
  IN UINT16                                BaseAddress,
  IN UINT32                                Offset
  )
{
  UINT8 Data;

  //
  // Use IsaIo to access IO
  //
  IsaIo->Io.Read (
             IsaIo,
             EfiIsaIoWidthUint8,
             BaseAddress + Offset,
             1,
             &Data
             );
  return Data;
}

/**
  Use IsaIo protocol to write serial port.

  @param  IsaIo         Pointer to EFI_ISA_IO_PROTOCOL instance
  @param  BaseAddress   Serial port register group base address
  @param  Offset        Offset in register group
  @param  Data          data which is to be written to some serial port register

**/
STATIC
VOID
IsaSerialWritePort (
  IN EFI_ISA_IO_PROTOCOL                 *IsaIo,
  IN UINT16                              BaseAddress,
  IN UINT32                              Offset,
  IN UINT8                               Data
  )
{
  //
  // Use IsaIo to access IO
  //
  IsaIo->Io.Write (
             IsaIo,
             EfiIsaIoWidthUint8,
             BaseAddress + Offset,
             1,
             &Data
             );
}

/**
  The user Entry Point for module IsaSerial. The user code starts with this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
IsaUartEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gIsaUartControllerDriver,
             ImageHandle,
             &gIsaUartComponentName,
             &gIsaUartComponentName2
             );
  ASSERT_EFI_ERROR (Status);


  return Status;
}

/**
  Check to see if this driver supports the given controller

  @param  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param  Controller           The handle of the controller to test.
  @param  RemainingDevicePath  A pointer to the remaining portion of a device path.

  @return EFI_SUCCESS          This driver can support the given controller

**/
EFI_STATUS
EFIAPI
Isa16550ControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL      *This,
  IN EFI_HANDLE                       Controller,
  IN EFI_DEVICE_PATH_PROTOCOL         *RemainingDevicePath
  )

{
  EFI_STATUS                           Status;
  EFI_ISA_IO_PROTOCOL                  *IsaIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiIsaIoProtocolGuid,
                  (VOID **) &IsaIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Use the ISA I/O Protocol to see if Controller is standard ISA UART that
  // can be managed by this driver.
  //
  Status = EFI_SUCCESS;
  if (IsaIo->ResourceList->Device.HID != EISA_PNP_ID (0x501)) {
    Status = EFI_UNSUPPORTED;
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         Controller,
         &gEfiIsaIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  return Status;
}

EFI_STATUS
InitIsa16550DeviceData (
  ISA_UART_DEVICE_DATA     *Isa16550Device,
  EFI_ISA_IO_PROTOCOL        *IsaIo
  )
{
  UART_16550_DEVICE_INFO    *DevInfo;
  UINT16                    UID;
  CHAR16                    *Name;
  UINT8                     IsaIRQ;
  UINTN                     Index;
  UINTN                     NameSize;
  UINTN                     BaseAddress;

  IsaIRQ      = 0;
  BaseAddress = 0;

  for (Index = 0; IsaIo->ResourceList->ResourceItem[Index].Type != EfiIsaAcpiResourceEndOfList; Index++) {
    if (IsaIo->ResourceList->ResourceItem[Index].Type == EfiIsaAcpiResourceIo) {
      BaseAddress = (UINTN)IsaIo->ResourceList->ResourceItem[Index].StartRange;
    } else if (IsaIo->ResourceList->ResourceItem[Index].Type == EfiIsaAcpiResourceInterrupt) {
      IsaIRQ      = (UINT8)IsaIo->ResourceList->ResourceItem[Index].StartRange;
    }
  }

  UID = (UINT16)IsaIo->ResourceList->Device.UID;

  //
  // Set Device Name
  //
  NameSize = ISA_DEVICE_NAME_SIZE;
  Name     = AllocateZeroPool (NameSize);

  UnicodeSPrint (
    Name,
    NameSize,
    L"%s (COM%c)",
    ISA_DEVICE_NAME,
    (0x41 + UID)
    );

  //
  // Initialize PCI_HS_UART_DEVICE_DATA structure
  //
  Isa16550Device->Signature = ISA_16550_DEV_SIGNATURE;
  Isa16550Device->IsaIo     = IsaIo;

  DevInfo = &Isa16550Device->DeviceInfo;
  DevInfo->DeviceName        = Name;
  DevInfo->DeviceType        = ISA_SERIAL_DEVICE;
  DevInfo->UID               = UID;
  DevInfo->RegisterByteWidth = ISA_UART_REGISTER_BYTE_WIDTH;
  DevInfo->BaseAddressType   = UBAT_IO;
  DevInfo->BaseAddress       = BaseAddress;
  DevInfo->DevIRQ            = IsaIRQ;
  DevInfo->FifoSize          = ISA_UART_FIFO_DEPTH;
  DevInfo->SerialClockFreq   = ISA_UART_SERIAL_CLOCK_FREQUENCY;
  DevInfo->SampleRate        = ISA_UART_SIMPLE_RATE;
  DevInfo->LegacySupport     = TRUE;
  DevInfo->UartUid           = 0; // Set at uart16550serial driver.

  Isa16550Device->U16550Access.RegRead    = Isa16550DeviceRegRead;
  Isa16550Device->U16550Access.RegWrite   = Isa16550DeviceRegWrite;
  Isa16550Device->U16550Access.DeviceInfo = DevInfo;

  return EFI_SUCCESS;
}


/**
  Start to management the controller passed in

  @param  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param  Controller           The handle of the controller to test.
  @param  RemainingDevicePath  A pointer to the remaining portion of a device path.

  @return EFI_SUCCESS   Driver is started successfully

**/
EFI_STATUS
EFIAPI
Isa16550ControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )

{
  EFI_STATUS                          Status;
  EFI_ISA_IO_PROTOCOL                 *IsaIo;
  ISA_UART_DEVICE_DATA                *Isa16550Device;

  Isa16550Device = NULL;

  //
  // Grab the IO abstraction we need to get any work done
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiIsaIoProtocolGuid,
                  (VOID **) &IsaIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Initialize the serial device instance
  //
  Isa16550Device = AllocateZeroPool (sizeof (ISA_UART_DEVICE_DATA));
  if (Isa16550Device == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }

  //
  // Initialize the device Private data
  //
  Status = InitIsa16550DeviceData (Isa16550Device, IsaIo);
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
                  &Isa16550Device->U16550Access
                  );
  if (EFI_ERROR(Status)) {
    goto Error;
  }

  return EFI_SUCCESS;

Error:

  if (Isa16550Device != NULL) {
    FreePool (Isa16550Device);
  }

  gBS->CloseProtocol (
         Controller,
         &gEfiIsaIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  return Status;
}

/**
  Disconnect this driver with the controller, uninstall related protocol instance

  @param  This                  A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param  Controller            The handle of the controller to test.
  @param  NumberOfChildren      Number of child device.
  @param  ChildHandleBuffer     A pointer to the remaining portion of a device path.

  @retval EFI_SUCCESS           Operation successfully
  @retval EFI_DEVICE_ERROR      Cannot stop the driver successfully

**/
EFI_STATUS
EFIAPI
Isa16550ControllerDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )

{
  EFI_STATUS                          Status;
  VOID                                *Protocol;
  ISA_UART_DEVICE_DATA               *Isa16550Device;

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

  Isa16550Device = ISA_16550_DEV_FROM_THIS (This);

  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gH2OUart16550AccessProtocolGuid,
                  &Isa16550Device->U16550Access
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
         Controller,
         &gEfiIsaIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  gBS->FreePool (Isa16550Device);

  return EFI_SUCCESS;
}

EFI_STATUS
Isa16550DeviceRegRead (
  H2O_UART_16550_ACCESS_PROTOCOL    *This,
  UINT16                            Index,
  UINT8                             *Data
  )
{
  ISA_UART_DEVICE_DATA     *Isa16550Device;

  Isa16550Device = ISA_16550_DEV_FROM_THIS (This);

  *Data = IsaSerialReadPort (Isa16550Device->IsaIo, (UINT16)Isa16550Device->DeviceInfo.BaseAddress, (UINT16)Index);

  return EFI_SUCCESS;
}

EFI_STATUS
Isa16550DeviceRegWrite (
  H2O_UART_16550_ACCESS_PROTOCOL    *This,
  UINT16                            Index,
  UINT8                             Data
  )
{
  ISA_UART_DEVICE_DATA     *Isa16550Device;

  Isa16550Device = ISA_16550_DEV_FROM_THIS (This);

  IsaSerialWritePort (Isa16550Device->IsaIo, (UINT16)Isa16550Device->DeviceInfo.BaseAddress, (UINT16)Index, Data);

  return EFI_SUCCESS;
}


/** @file
  PCI Serial driver for standard UARTS on an PCI bus.

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#include <PciUartDxe.h>

#define PCI_DEVICE_NAME_SIZE              50
#define PCI_DEVICE_NAME                   L"PCI_UART"
#define PCI_UART_REGISTER_BYTE_WIDTH      sizeof(UINT8)
#define PCI_UART_FIFO_DEPTH               16
#define PCI_UART_NON_FIFO_DEPTH           1
#define PCI_UART_SERIAL_CLOCK_FREQUENCY   (1 * 115200 * 16)
#define PCI_UART_SIMPLE_RATE              16

#define PCI_CLASS_SCC                     0x07
#define PCI_SUBCLASS_SERIAL               0x00
#define PCI_IF_16550                      0x02

#define PCI_16550_DEV_SIGNATURE           SIGNATURE_32('P','C','I','U')
#define PCI_16550_DEV_FROM_THIS(a) CR (a, PCI_UART_DEVICE_DATA, U16550Access, PCI_16550_DEV_SIGNATURE)

PCI_DEVICE_PFA  mPciNonFifoList[] = {0x0, 0x0, 0x0};

//
// data type definitions
//
typedef struct {
  UINT32                            Signature;
  EFI_PCI_IO_PROTOCOL               *PCIIo;
  UART_16550_DEVICE_INFO            DeviceInfo;
  H2O_UART_16550_ACCESS_PROTOCOL    U16550Access;
} PCI_UART_DEVICE_DATA;

EFI_DRIVER_BINDING_PROTOCOL gPciUartControllerDriver = {
  Pci16550ControllerDriverSupported,
  Pci16550ControllerDriverStart,
  Pci16550ControllerDriverStop,
  0x10,
  NULL,
  NULL
};

extern EFI_COMPONENT_NAME_PROTOCOL      gPciUartComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL     gPciUartComponentName2;


/**

  Providing PciIo read for serial register I/O macros

  @param[in]  PciIo       A pointer to the EFI_PCI_IO_PROTOCOL instance.
  @param[in]  BarIndex    The BAR index of the standard PCI Configuration header to use as the
                          base address for the memory or I/O operation to perform.
  @param[in]  Offset      The offset within the selected BAR to start the memory or I/O operation.

  @retval    VOID

**/
STATIC
UINT8
PciSerialReadPort (
  IN EFI_PCI_IO_PROTOCOL   *PciIo,
  IN UINT16                BarIndex,
  IN UINT16                Offset
  )
{
  UINT8        Data;

  //
  // Use PciIo to access IO
  //
  PciIo->Io.Read (
              PciIo,
              EfiPciIoWidthUint8,
              (UINT8) BarIndex,
              (UINT16) Offset,
              (UINTN) 1,
              &Data
              );
  return Data;
}

/**

  Providing PciIo write for serial register I/O macros

  @param[in]  PciIo       A pointer to the EFI_PCI_IO_PROTOCOL instance.
  @param[in]  BarIndex    The BAR index of the standard PCI Configuration header to use as the
                          base address for the memory or I/O operation to perform.
  @param[in]  Offset      The offset within the selected BAR to start the memory or I/O operation.
  @param[in]  Buffer      For read operations, the destination buffer to store the results. For write
                          operations, the source buffer to write data from.

  @retval    VOID

**/
STATIC
VOID
PciSerialWritePort (
  IN EFI_PCI_IO_PROTOCOL *PciIo,
  IN UINT16              BarIndex,
  IN UINT16              Offset,
  IN UINT8               Data
  )
{
  //
  // Use PciIo to access IO
  //
  PciIo->Io.Write (
              PciIo,
              EfiPciIoWidthUint8,
              (UINT8) BarIndex,
              (UINT16) Offset,
              (UINTN) 1,
              &Data
              );
}

/**
  The module Entry Point for module PciSerial.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
PciUartEntryPoint(
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
             &gPciUartControllerDriver,
             ImageHandle,
             &gPciUartComponentName,
             &gPciUartComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Test controller is a PciSerial Controller.

  @param This                 Pointer of EFI_DRIVER_BINDING_PROTOCOL
  @param Controller           driver's controller
  @param RemainingDevicePath  children device path

  @retval EFI_UNSUPPORTED This driver does not support this device
  @retval EFI_SUCCESS     This driver supports this device
**/
EFI_STATUS
EFIAPI
Pci16550ControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                Status;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  union {
    UINT8                     ByteBuf[4];
    UINT16                    WordBuf[2];
  }                         Buffer;
  UINT16                    Temp;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **)&PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );


  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Use the PCI I/O Protocol to see if Controller is standard ISA UART that
  // can be managed by this driver.
  //
  Status = EFI_SUCCESS;

  //
  // Looks for a PCI CLASS / SUBCLASS / INTERFACE of 0x07 / 0x00 / 0x02
  // To allow supportting all PCI Devices that are 16550 compatible UARTS.
  //
  // Also if want general PCI Serial com devices to work as well
  // can duplicate this driver one for Tekoa and AMT and the other
  // for general serial devices.
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        (UINT32) PCI_REVISION_ID_OFFSET,
                        (UINTN) 1,
                        (VOID *) &Buffer
                        );
  //
  // Check Pci Serial Device Class code
  //
  if ((Buffer.ByteBuf[3] != PCI_CLASS_SCC) ||
      (Buffer.ByteBuf[2] != PCI_SUBCLASS_SERIAL) ||
      (Buffer.ByteBuf[1] != PCI_IF_16550)) {
    Status = EFI_UNSUPPORTED;
    goto Exit;
  }

  //
  // Make sure the PCI io space is enabled
  //
  Temp = 0x0003;
  PciIo->Pci.Write (
               PciIo,
               EfiPciIoWidthUint16,
               0x04,
               0x01,
               &Temp
               );

Exit:
  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );
  return Status;
}

UINT32
GetUartDeviceSupportFIFOSize (
  UINTN Bus,
  UINTN Dev,
  UINTN Func
  )
{
  UINTN Count;
  UINTN Index;

  Count = sizeof (mPciNonFifoList) / sizeof (PCI_DEVICE_PFA);

  for (Index = 0; Index < Count; Index++) {
    if ( Bus  == mPciNonFifoList[Index].Bus &&
         Dev  == mPciNonFifoList[Index].Dev &&
         Func == mPciNonFifoList[Index].Func ) {
      return PCI_UART_NON_FIFO_DEPTH;
    }
  }
  return PCI_UART_FIFO_DEPTH;
}

EFI_STATUS
InitPci16550DeviceData (
  PCI_UART_DEVICE_DATA                 *Pci16550Device,
  EFI_PCI_IO_PROTOCOL                  *PciIo
  )
{
  EFI_STATUS                           Status;
  UART_16550_DEVICE_INFO               *DevInfo;
  UINTN                                Segment;
  UINTN                                Bus;
  UINTN                                Device;
  UINTN                                Function;
  UINTN                                NameSize;
  UINTN                                Index;
  UINT16                               UID;
  UINT16                               BarIndex;
  CHAR16                               *Name;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR    *Resources;

  BarIndex  = 0;
  Status    = EFI_UNSUPPORTED;

  for (Index = 0; Index < PCI_MAX_BAR; Index++) {
    Status = PciIo->GetBarAttributes (
                      PciIo,
                      (UINT8) Index,
                      NULL,
                      (VOID**)&Resources
                      );

    if (Resources->ResType == ACPI_ADDRESS_SPACE_TYPE_IO) {
      BarIndex = (UINT16) Index;
      Status = EFI_SUCCESS;
      break;
    }
  }

  if (Status != EFI_SUCCESS) {
    return Status;
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

  UID  = (UINT16)(Bus<<8 | Device<<3 | Function);

  //
  // Set Device Name
  //
  NameSize = PCI_DEVICE_NAME_SIZE;
  Name     = AllocateZeroPool (NameSize);

  UnicodeSPrint (
    Name,
    NameSize,
    L"%s (0x%x:0x%x:0x%x)",
    PCI_DEVICE_NAME,
    Bus,
    Device,
    Function
    );

  //
  // Initialize PCI_HS_UART_DEVICE_DATA structure
  //
  Pci16550Device->Signature = PCI_16550_DEV_SIGNATURE;
  Pci16550Device->PCIIo     = PciIo;

  DevInfo = &Pci16550Device->DeviceInfo;
  DevInfo->DeviceName        = Name;
  DevInfo->DeviceType        = PCI_SERIAL_DEVICE;
  DevInfo->UID               = UID;
  DevInfo->RegisterByteWidth = PCI_UART_REGISTER_BYTE_WIDTH;
  DevInfo->BaseAddressType   = UBAT_BAR_INDEX;
  DevInfo->BaseAddress       = (UINTN)BarIndex;
  DevInfo->FifoSize          = GetUartDeviceSupportFIFOSize (Bus, Device, Function);
  DevInfo->SerialClockFreq   = PCI_UART_SERIAL_CLOCK_FREQUENCY;
  DevInfo->SampleRate        = PCI_UART_SIMPLE_RATE;
  DevInfo->LegacySupport     = TRUE;

  Pci16550Device->U16550Access.RegRead    = Pci16550DeviceRegRead;
  Pci16550Device->U16550Access.RegWrite   = Pci16550DeviceRegWrite;
  Pci16550Device->U16550Access.DeviceInfo = DevInfo;

  return EFI_SUCCESS;
}


/**
  Start this driver on ControllerHandle by opening a PciIo protocol, creating
  SERIAL_DEV device and install gEfiSerialIoProtocolGuid
  finally.

  @param  This                 Protocol instance pointer.
  @param  ControllerHandle     Handle of device to bind driver to
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @retval EFI_SUCCESS          This driver is added to ControllerHandle
  @retval EFI_ALREADY_STARTED  This driver is already running on ControllerHandle
  @retval other                This driver does not support this device

**/
EFI_STATUS
EFIAPI
Pci16550ControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                           Status;
  EFI_PCI_IO_PROTOCOL                  *PciIo;
  PCI_UART_DEVICE_DATA                *Pci16550Device;

  Pci16550Device = NULL;

  //
  // Grab the IO abstraction we need to get any work done
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **)&PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // Initialize the serial device instance
  //
  Pci16550Device = AllocateZeroPool (sizeof (PCI_UART_DEVICE_DATA));
  if (Pci16550Device == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }

  //
  // Initialize the device Private data
  //
  Status = InitPci16550DeviceData (Pci16550Device, PciIo);
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
                &Pci16550Device->U16550Access
                );
  if (EFI_ERROR(Status)) {
    goto Error;
  }

  return EFI_SUCCESS;

Error:

  if (Pci16550Device != NULL) {
    FreePool (Pci16550Device);
  }

  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  return Status;
}

/**
  Stop this driver on ControllerHandle. Support stoping any child handles
  created by this driver.

  @param  This              Protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
Pci16550ControllerDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
{
  EFI_STATUS                         Status;
  VOID                               *Protocol;
  PCI_UART_DEVICE_DATA              *Pci16550Device;

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

  Pci16550Device = PCI_16550_DEV_FROM_THIS (This);

  Status = gBS->UninstallProtocolInterface (
                Controller,
                &gH2OUart16550AccessProtocolGuid,
                &Pci16550Device->U16550Access
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

  gBS->FreePool (Pci16550Device);

  return EFI_SUCCESS;
}

EFI_STATUS
Pci16550DeviceRegRead (
  H2O_UART_16550_ACCESS_PROTOCOL    *This,
  UINT16                            Index,
  UINT8                             *Data
  )
{
  PCI_UART_DEVICE_DATA     *Pci16550Device;

  Pci16550Device = PCI_16550_DEV_FROM_THIS (This);

  *Data = PciSerialReadPort (Pci16550Device->PCIIo, (UINT16)Pci16550Device->DeviceInfo.BaseAddress, (UINT16)Index);

  return EFI_SUCCESS;
}

EFI_STATUS
Pci16550DeviceRegWrite (
  H2O_UART_16550_ACCESS_PROTOCOL    *This,
  UINT16                            Index,
  UINT8                             Data
  )
{
  PCI_UART_DEVICE_DATA     *Pci16550Device;

  Pci16550Device = PCI_16550_DEV_FROM_THIS (This);

  PciSerialWritePort (Pci16550Device->PCIIo, (UINT16)Pci16550Device->DeviceInfo.BaseAddress, (UINT16)Index, Data);

  return EFI_SUCCESS;
}


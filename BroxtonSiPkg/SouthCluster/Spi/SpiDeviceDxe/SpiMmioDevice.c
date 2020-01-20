/** @file
  SPI Mmio device driver to enumerate MMIOS devices.

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

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/DevicePath.h>
#include <Protocol/MmioDevice.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/SpiAcpi.h>
#include <IndustryStandard/Pci22.h>
#include "SpiMmioConfig.h"
#include <Protocol/PciEnumerationComplete.h>
#include <Protocol/PciIo.h>
#include <ScRegs/RegsLpss.h>

#include <Guid/EventGroup.h>

EFI_EVENT  mSPIMmioNotiryEvent = NULL;

#define PCI_CFG_ADDRESS(bus, dev, func, reg)  ( \
      (UINT64) ((((UINTN) bus) << 24) + (((UINTN) dev) << 16) + (((UINTN) func) << 8) + ((UINTN) reg)) \
    ) & 0x00000000ffffffff


STATIC EFI_DEVICE_PATH_PROTOCOL mEndOfPath = {
  END_DEVICE_PATH_TYPE,
  END_ENTIRE_DEVICE_PATH_SUBTYPE,
  {
    END_DEVICE_PATH_LENGTH,
    0
  }
};

ACPI_HID_DEVICE_PATH mAcpi_INTC33B0_SPI0 = {  //  SPI controller 0
  {
  ACPI_DEVICE_PATH,
  ACPI_DP,
  (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)),
  (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8)
  },
  EISA_PNP_ID(0x501),
  0
};

ACPI_HID_DEVICE_PATH mAcpi_INTC33B0_SPI1 = {  //  SPI controller 0
  {
  ACPI_DEVICE_PATH,
  ACPI_DP,
  (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)),
  (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8)
  },
  EISA_PNP_ID(0x501),
  0
};

ACPI_HID_DEVICE_PATH mAcpi_INTC33B0_SPI2 = {  //  SPI controller 0
  {
  ACPI_DEVICE_PATH,
  ACPI_DP,
  (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)),
  (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8)
  },
  EISA_PNP_ID(0x501),
  0
};

typedef struct {
  UINT8 device_number;
  UINT8 function_number;
} PCI_SPI_CONTROLLERS;

PCI_SPI_CONTROLLERS SPI_PCI_MAPPING[] = {
  {
    PCI_DEVICE_NUMBER_LPSS_SPI,
    PCI_FUNCTION_NUMBER_LPSS_SPI0
  }
  ,
  {
    PCI_DEVICE_NUMBER_LPSS_SPI,
    PCI_FUNCTION_NUMBER_LPSS_SPI1
  }
  ,
  {
    PCI_DEVICE_NUMBER_LPSS_SPI,
    PCI_FUNCTION_NUMBER_LPSS_SPI2
  }
};

EFI_MMIO_DEVICE_PROTOCOL_ITEM mSpi0Protocols [ ] = {
  { &gEfiSpiAcpiProtocolGuid,  &mSpi0BusEnum.AcpiApi }
};

EFI_MMIO_DEVICE_PROTOCOL_ITEM mSpi1Protocols [ ] = {
  { &gEfiSpiAcpiProtocolGuid,  &mSpi1BusEnum.AcpiApi }
};

EFI_MMIO_DEVICE_PROTOCOL_ITEM mSpi2Protocols [ ] = {
  { &gEfiSpiAcpiProtocolGuid,  &mSpi2BusEnum.AcpiApi }
};


 EFI_MMIO_DEVICE_PROTOCOL gMmioDeviceList [ ] = {
  //
  //  SPI-0
  //
  {
    (ACPI_HID_DEVICE_PATH *)&mAcpi_INTC33B0_SPI0,
    1,
    0x00010000,
    (VOID *)&mSpi0ControllerConfig,
    DIM(mSpi0Protocols),
    mSpi0Protocols
  },
  //
  //  SPI-1
  //
  {
    (ACPI_HID_DEVICE_PATH *)&mAcpi_INTC33B0_SPI1,
    1,
    0x00010000,
    (VOID *)&mSpi1ControllerConfig,
    DIM(mSpi1Protocols),
    mSpi1Protocols
  },
  //
  //  SPI-2
  //
  {
    (ACPI_HID_DEVICE_PATH *)&mAcpi_INTC33B0_SPI2,
    1,
    0x00010000,
    (VOID *)&mSpi2ControllerConfig,
    DIM(mSpi2Protocols),
    mSpi2Protocols
  }
};

UINTN gMmioDeviceCount = DIM ( gMmioDeviceList );

/**
  Test to see if this driver supports Controller Handle.

  @param[in]  This                Protocol instance pointer.
  @param[in]  Controller          Handle of device to test
  @param[in]  RemainingDevicePath Optional parameter use to pick a specific child
                                  device to start.

  @retval     EFI_SUCCESS         This driver supports this device
  @retval     EFI_ALREADY_STARTED This driver is already running on this device
  @retval     Others              This driver does not support this device
**/
EFI_STATUS
EFIAPI
SpiDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *SpiBridgeDevicePath;

  ACPI_HID_DEVICE_PATH      *AcpiNode;
  PCI_DEVICE_PATH           *PciNode;

  //
  // Get the SPI bridge's Device Path and test it
  // the following code is specific
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **)&SpiBridgeDevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EFI_SUCCESS;
  AcpiNode =  (ACPI_HID_DEVICE_PATH *)SpiBridgeDevicePath;
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
    SpiBridgeDevicePath = NextDevicePathNode (SpiBridgeDevicePath);
    PciNode  = (PCI_DEVICE_PATH *)SpiBridgeDevicePath;
    if (PciNode->Header.Type != HARDWARE_DEVICE_PATH ||
        PciNode->Header.SubType != HW_PCI_DP ||
        DevicePathNodeLength (&PciNode->Header) != sizeof (PCI_DEVICE_PATH) ||
//        PciNode->Function != 0x01 ||
        PciNode->Device != 0x19 ) {
      Status = EFI_UNSUPPORTED;
    }else{
      Status = EFI_SUCCESS;
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

  return Status;
}

/**
  Start this driver on ControllerHandle.

  @param[in]  This                 Protocol instance pointer.
  @param[in]  Controller           Handle of device to bind driver to
  @param[in]  RemainingDevicePath  Optional parameter use to pick a specific child
                                   device to start.

  @retval     EFI_SUCCESS          This driver is added to ControllerHandle
  @retval     EFI_ALREADY_STARTED  This driver is already running on ControllerHandle
  @retval     Others               This driver does not support this device
**/
EFI_STATUS
EFIAPI
SpiDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_HANDLE Device;
  UINT8 * Buffer;
  ACPI_HID_DEVICE_PATH * DevicePath;
  UINTN LengthInBytes;
  EFI_MMIO_DEVICE_PROTOCOL * MmioDevice;
  EFI_MMIO_DEVICE_PROTOCOL_ITEM * Protocol;
  EFI_MMIO_DEVICE_PROTOCOL_ITEM * ProtocolEnd;
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_DEVICE_PATH_PROTOCOL  *SpiBridgeDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  UINT8 DeviceNum = 0;
  EFI_DEV_PATH  Node;
  PCI_DEVICE_PATH           *PciNode;
  EFI_PCI_IO_PROTOCOL       *PciIo;

  UINT32 spi_mmioaddr = 0;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **)&SpiBridgeDevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  TempDevicePath = SpiBridgeDevicePath;

  SpiBridgeDevicePath = NextDevicePathNode (SpiBridgeDevicePath);
  PciNode  = (PCI_DEVICE_PATH *)SpiBridgeDevicePath;
  DeviceNum = PciNode->Function;

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
                    PCI_BASE_ADDRESSREG_OFFSET,
                    sizeof(spi_mmioaddr) / sizeof(UINT32),
                    &spi_mmioaddr);

  if(EFI_ERROR(Status) || (spi_mmioaddr == 0) ){
    DEBUG((EFI_D_INFO,"SPI MMIO Addr read failed: %r\r\n", Status));
    return  Status;
  }else{

    DEBUG((DEBUG_INFO, "SPI%02d MMIO Addr: 0x%x\r\n", DeviceNum, spi_mmioaddr));

  }

  DEBUG (( DEBUG_INFO, "MMIO device enumeration starting\r\n" ));
  //
  //  Assume success
  //
  Status = EFI_SUCCESS;

  //
  //  Create the necessary devices
  //

  //
  //  Walk the list of devices
  //
  MmioDevice = &gMmioDeviceList [DeviceNum];
  //
  //  Validate the device entry
  //
  if ( NULL == MmioDevice->AcpiPath ) {
    DEBUG (( DEBUG_ERROR,
              "ERROR - AcpiPath is NULL in gMmioDeviceList [ %d ]\r\n",
              MmioDevice - &gMmioDeviceList [ 0 ]));
    Status = EFI_INVALID_PARAMETER;
  }

  LengthInBytes = DevicePathNodeLength ( MmioDevice->AcpiPath );
  if ( sizeof ( ACPI_HID_DEVICE_PATH ) > LengthInBytes ) {
    DEBUG (( DEBUG_ERROR,
              "ERROR - invalid AcpiPath length in gMmioDeviceList [ %d ]\r\n",
              MmioDevice - &gMmioDeviceList [ 0 ]));
    Status = EFI_INVALID_PARAMETER;
  }

  if ( NULL == MmioDevice->DriverResources ) {
    DEBUG (( DEBUG_ERROR,
              "ERROR - DriverResources is NULL in gMmioDeviceList [ %d ]\r\n",
              MmioDevice - &gMmioDeviceList [ 0 ]));
    Status = EFI_INVALID_PARAMETER;
  }
  if ( EFI_ERROR ( Status )) {
    return  Status;
  }

  ((SPI_PLATFORM_CONTEXT *)(MmioDevice->DriverResources))->BaseAddress = (EFI_PHYSICAL_ADDRESS)spi_mmioaddr;

  //
  //  Allocate the device path buffer
  //
  Buffer = AllocatePool ( LengthInBytes + sizeof ( mEndOfPath ));
  if ( NULL == Buffer ) {
    DEBUG (( DEBUG_ERROR,
              "ERROR - Failed to allocate device path\r\n" ));
    Status = EFI_OUT_OF_RESOURCES;
  }
  if ( EFI_ERROR ( Status )) {
    return  Status;
  }

  //
  //  Build the device path for the device
  //
  DevicePath = (ACPI_HID_DEVICE_PATH *)Buffer;
  CopyMem ( DevicePath,
            MmioDevice->AcpiPath,
            LengthInBytes );
  if ( 0 != MmioDevice->UnitIdentification) {
  }

  Node.DevPath.Type     = ACPI_DEVICE_PATH;
  Node.DevPath.SubType  = ACPI_DP;
  SetDevicePathNodeLength (&Node.DevPath, sizeof (ACPI_HID_DEVICE_PATH));
  Node.Acpi.HID = DevicePath->HID;
  Node.Acpi.UID = DevicePath->UID;

  DevicePath = (ACPI_HID_DEVICE_PATH*)AppendDevicePathNode (
                TempDevicePath,
                (EFI_DEVICE_PATH_PROTOCOL *)DevicePath
                );

  //
  //  Create a handle for the device and install the
  //  protocols
  //
  Device = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                    &Device,
                    &gEfiMmioDeviceProtocolGuid,
                    MmioDevice,
                    &gEfiDevicePathProtocolGuid,
                    DevicePath,
                    NULL );
  if ( EFI_ERROR ( Status )) {
    DEBUG (( DEBUG_ERROR,
              "ERROR - Failed to install protocol, Status: %r\r\n",
              Status ));
  }

  //
  //  Install the additional protocols
  //
  Protocol = MmioDevice->ProtocolArray;
  ProtocolEnd = &Protocol [ MmioDevice->ProtocolCount ];
  while ( ProtocolEnd > Protocol ) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                      &Device,
                      Protocol->Guid,
                      Protocol->Context,
                      NULL );
    if ( EFI_ERROR ( Status )) {
      DEBUG (( DEBUG_ERROR,
                "ERROR - Failed to additional install protocol GUID, Status: %r\r\n",
                Status ));
    }
    Protocol += 1;
  }
  //
  //  Display exit
  //
  DEBUG (( DEBUG_INFO, "MMIO device enumeration done, Status: %r\r\n", Status ));
  //
  //  Return the enumeration status
  //
  return  EFI_SUCCESS;
}

/**
  Stop this driver on ControllerHandle.

  @param[in]  This              Protocol instance pointer.
  @param[in]  Controller        Handle of device to stop driver on
  @param[in]  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                                children is zero stop the entire bus driver.
  @param[in]  ChildHandleBuffer List of Child Handles to Stop.

  @retval     EFI_SUCCESS       This driver is removed ControllerHandle
  @retval     Others            This driver was not removed from this device
**/
EFI_STATUS
EFIAPI
SpiDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
{
  EFI_STATUS             Status;
  EFI_MMIO_DEVICE_PROTOCOL * MmioDevice;
  EFI_MMIO_DEVICE_PROTOCOL_ITEM * Protocol;
  EFI_MMIO_DEVICE_PROTOCOL_ITEM * ProtocolEnd;
  EFI_DEVICE_PATH_PROTOCOL  *SpiBridgeDevicePath;

  //
  // Get gEfiMmioDeviceProtocolGuid interface
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiMmioDeviceProtocolGuid,
                  (VOID **)&MmioDevice,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
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


  gBS->CloseProtocol (
                   Controller,
                   &gEfiMmioDeviceProtocolGuid,
                   This->DriverBindingHandle,
                   Controller
                   );
  //
  // Uninstall protocol interface: gEfiMmioDeviceProtocolGuid
  //
  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiMmioDeviceProtocolGuid,
                  &MmioDevice
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

    Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &SpiBridgeDevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

        Status = gBS->UninstallMultipleProtocolInterfaces (
                      Controller,
                      &gEfiDevicePathProtocolGuid,
                      SpiBridgeDevicePath,
                      &MmioDevice,
                      &MmioDevice,
                      NULL
                      );

  gBS->CloseProtocol (
                   Controller,
                   &gEfiDevicePathProtocolGuid,
                   This->DriverBindingHandle,
                   Controller
                   );

      //
      //  Uninstall the additional protocols
      //
      Protocol = MmioDevice->ProtocolArray;
      ProtocolEnd = &Protocol [ MmioDevice->ProtocolCount ];
      while ( ProtocolEnd > Protocol ) {
        Status = gBS->UninstallMultipleProtocolInterfaces (
                          Controller,
                          Protocol->Guid,
                          Protocol->Context,
                          NULL );
        if ( EFI_ERROR ( Status )) {
          DEBUG (( DEBUG_ERROR,
                    "ERROR - Failed to uninstall protocol GUID, Status: %r\r\n",
                    Status ));
        }
        Protocol += 1;
      }

  return EFI_SUCCESS;
}

EFI_DRIVER_BINDING_PROTOCOL mSpiDriverBinding = {
  SpiDriverSupported,
  SpiDriverStart,
  SpiDriverStop,
  0x10,
  NULL,
  NULL
};

/**
  The Entry Point for Spi Device.

  @param[in]  ImageHandle    The firmware allocated handle for the EFI image.
  @param[in]  SystemTable    A pointer to the EFI System Table.

  @retval     EFI_SUCCESS    The entry point is executed successfully.
  @retval     Others         Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
SPI_MmioEntryPoint (
  IN EFI_HANDLE              ImageHandle,
  IN EFI_SYSTEM_TABLE        *pSystemTable
  )
{
  EFI_STATUS Status = EFI_SUCCESS;

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             pSystemTable,
             &mSpiDriverBinding,
             ImageHandle,
             NULL,
             NULL
             );
  if ( !EFI_ERROR ( Status )) {
    DEBUG (( DEBUG_POOL | DEBUG_INIT | DEBUG_INFO,
              "Installed: gEfiDriverBindingProtocolGuid on   0x%016lx\r\n",
              (UINT64)((UINTN)ImageHandle )));
  }
  return Status;
}

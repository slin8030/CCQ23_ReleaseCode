/** @file
  Implement the SPI driver binding protocol

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
#include <Protocol/DriverBinding.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SpiAcpi.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "SpiBus.h"

static EFI_GUID SpiBusDriverGuid = { 0x74e5c409, 0xfd5c, 0x4a2c, { 0xa5, 0xeb, 0x85, 0x98, 0xdc, 0x60, 0x9, 0x8b } };
/**
  Verify the controller type
  This routine determines if an SPI controller is available.
  This routine is called by the UEFI driver framework during connect
  processing.

  @param [in] DriverBinding             Protocol instance pointer.
  @param [in] Controller                Handle of device to test.
  @param [in] RemainingDevicePath       Not used.

  @retval EFI_SUCCESS                   This driver supports this device.
  @retval Others                        This driver does not support this device.
**/
EFI_STATUS
EFIAPI
SpiBusDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL        *DriverBinding,
  IN EFI_HANDLE                         Controller,
  IN EFI_DEVICE_PATH_PROTOCOL           *RemainingDevicePath
  )
{
  VOID *Protocol;
  EFI_STATUS Status;

  //DEBUG (( DEBUG_LOAD, "SpiBusDriverSupported entered\r\n" ));
  Status = gBS->OpenProtocol ( Controller,
                               &gEfiSpiHostProtocolGuid,
                               &Protocol,
                               DriverBinding->DriverBindingHandle,
                               Controller,
                               EFI_OPEN_PROTOCOL_BY_DRIVER );
  if ( !EFI_ERROR ( Status )) {
    //DEBUG ((  DEBUG_INFO, "SPI host controller found\r\n" ));
    Status = gBS->OpenProtocol ( Controller,
                                 &gEfiSpiAcpiProtocolGuid,
                                 &Protocol,
                                 DriverBinding->DriverBindingHandle,
                                 Controller,
                                 EFI_OPEN_PROTOCOL_BY_DRIVER );
    if ( !EFI_ERROR ( Status )) {
      //DEBUG ((  DEBUG_INFO, "SPI bus enumeration data found\r\n" ));
      gBS->CloseProtocol ( Controller,
                           &gEfiSpiAcpiProtocolGuid,
                           DriverBinding->DriverBindingHandle,
                           Controller );
    }
    gBS->CloseProtocol ( Controller,
                         &gEfiSpiHostProtocolGuid,
                         DriverBinding->DriverBindingHandle,
                         Controller );
  }
  //DEBUG (( DEBUG_LOAD, "SpiBusDriverSupported exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Connect to the SPI controller
  This routine initializes an instance of the SPI driver for this
  controller.
  This routine is called by the UEFI driver framework during connect
  processing if the controller passes the tests in SPIBusDriverSupported.

  @param [in] DriverBinding             Protocol instance pointer.
  @param [in] Controller                Handle of device to work with.
  @param [in] RemainingDevicePath       Not used, always produce all possible children.

  @retval EFI_SUCCESS                   This driver is added to Controller.
  @retval Others                        This driver does not support this device.
**/
EFI_STATUS
EFIAPI
SpiBusDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL        *DriverBinding,
  IN EFI_HANDLE                         Controller,
  IN EFI_DEVICE_PATH_PROTOCOL           *RemainingDevicePath
  )
{
  EFI_SPI_ACPI_PROTOCOL *SpiAcpi;
  EFI_SPI_HOST_PROTOCOL *SpiHost;
  SPI_BUS_CONTEXT *SpiBus;
  EFI_STATUS Status;
  //DEBUG (( DEBUG_LOAD, "SpiBusDriverStart entered\r\n" ));

  SpiBus = AllocateZeroPool ( sizeof ( *SpiBus ));
  if ( NULL == SpiBus ) {
    //DEBUG (( DEBUG_ERROR, "ERROR - No memory for SPI bus driver\r\n" ));
    Status = EFI_OUT_OF_RESOURCES;
  }
  else {
    Status = gBS->OpenProtocol ( Controller,
                                 &gEfiSpiHostProtocolGuid,
                                 (VOID**)&SpiHost,
                                 DriverBinding->DriverBindingHandle,
                                 Controller,
                                 EFI_OPEN_PROTOCOL_BY_DRIVER );
    if ( !EFI_ERROR ( Status )) {
      //DEBUG (( DEBUG_INFO, "0x%016Lx: SPI host found\r\n", SpiHost ));
      Status = gBS->OpenProtocol ( Controller,
                                   &gEfiSpiAcpiProtocolGuid,
                                   (VOID**)&SpiAcpi,
                                   DriverBinding->DriverBindingHandle,
                                   Controller,
                                   EFI_OPEN_PROTOCOL_BY_DRIVER );
      if ( !EFI_ERROR ( Status )) {
        //DEBUG (( DEBUG_INFO, "0x%016Lx: SPI host found\r\n", SpiHost ));
        SpiBus->Signature = SPI_BUS_SIGNATURE;
        SpiBus->SpiHost = SpiHost;
        SpiBus->SpiAcpi = SpiAcpi;
        Status = SpiBusApiStart ( SpiBus, Controller );
        if ( !EFI_ERROR ( Status )) {
          Status = gBS->InstallMultipleProtocolInterfaces (
                          &Controller,
                          &SpiBusDriverGuid,
                          SpiBus,
                          NULL
                          );
          if ( !EFI_ERROR ( Status )) {
            //DEBUG (( DEBUG_INIT, "0x%016Lx: SpiBus started\r\n",  (UINT64)(UINTN)SpiBus ));
          }
          else {
            SpiBusApiStop ( SpiBus );
          }
        }
        if ( EFI_ERROR ( Status )) {
          gBS->CloseProtocol ( Controller,
                               &gEfiSpiAcpiProtocolGuid,
                               DriverBinding->DriverBindingHandle,
                               Controller );
        }
      }
      if ( EFI_ERROR ( Status )) {
        gBS->CloseProtocol ( Controller,
                             &gEfiSpiHostProtocolGuid,
                             DriverBinding->DriverBindingHandle,
                             Controller );
      }
    }
    if ( EFI_ERROR ( Status )) {
      //DEBUG (( DEBUG_ERROR, "ERROR - Failed to start SPI bus driver, Status: %r\r\n", Status ));
      //DEBUG (( DEBUG_POOL | DEBUG_INFO,  "0x%016Lx: SpiBus released\r\n",  (UINT64)(UINTN)SpiBus ));
      FreePool ( SpiBus );
    }
  }

  //DEBUG (( DEBUG_LOAD, "SpiBusDriverStart exiting, Status: %r\r\n", Status ));
  return Status;
}

/**
  Disconnect from the SPI host controller.
  This routine disconnects from the SPI controller.
  This routine is called by DriverUnload when the SPI bus driver
  is being unloaded.

  @param [in] DriverBinding             Protocol instance pointer.
  @param [in] Controller                Handle of device to stop driver on.
  @param [in] NumberOfChildren          How many children need to be stopped.
  @param [in] ChildHandleBuffer         Not used.

  @retval EFI_SUCCESS                   This driver is removed Controller.
  @retval EFI_DEVICE_ERROR              The device could not be stopped due to a device error.
  @retval Others                        This driver was not removed from this device.
**/
EFI_STATUS
EFIAPI
SpiBusDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL       *DriverBinding,
  IN  EFI_HANDLE                        Controller,
  IN  UINTN                             NumberOfChildren,
  IN  EFI_HANDLE                        *ChildHandleBuffer
  )
{
  VOID *DriverProtocol;
  SPI_BUS_CONTEXT *SpiBus;
  EFI_STATUS Status;

  //DEBUG (( DEBUG_LOAD | DEBUG_INFO, "SpiBusDriverStop entered\r\n" ));
  Status = gBS->OpenProtocol (
                  Controller,
                  &SpiBusDriverGuid,
                  &DriverProtocol,
                  DriverBinding->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE );
  if ( !EFI_ERROR ( Status )) {
    SpiBus = SPI_BUS_CONTEXT_FROM_PROTOCOL ( DriverProtocol );
    Status = gBS->CloseProtocol ( Controller,
                                  &SpiBusDriverGuid,
                                  DriverBinding->DriverBindingHandle,
                                  Controller );
    if ( !EFI_ERROR ( Status )) {
      //DEBUG (( DEBUG_INIT, "0x%016Lx: SpiBus stopped\r\n", (UINT64)(UINTN)SpiBus ));
      SpiBusApiStop ( SpiBus );
      if (( NULL != SpiBus->DeviceListHead )
        || ( NULL != SpiBus->DeviceListTail )) {
        Status = EFI_NOT_READY;
      }
      else {
        Status = gBS->UninstallMultipleProtocolInterfaces (
                      Controller,
                      &SpiBusDriverGuid,
                      DriverProtocol,
                      NULL );
        if ( !EFI_ERROR ( Status )) {
          Status = gBS->CloseProtocol ( Controller,
                                        &gEfiSpiAcpiProtocolGuid,
                                        DriverBinding->DriverBindingHandle,
                                        Controller );
          //DEBUG (( DEBUG_POOL | DEBUG_INFO, "0x%016Lx: SPI ACPI Protocol closed: %r\r\n",  (UINT64)(UINTN)SpiBus, Status ));

          Status = gBS->CloseProtocol ( Controller,
                                        &gEfiSpiHostProtocolGuid,
                                        DriverBinding->DriverBindingHandle,
                                        Controller );
          //DEBUG (( DEBUG_POOL | DEBUG_INFO, "0x%016Lx: SPI Host Protocol closed: %r\r\n", (UINT64)(UINTN)SpiBus, Status ));

          //DEBUG (( DEBUG_POOL | DEBUG_INFO, "0x%016Lx: SPI Bus released\r\n", (UINT64)(UINTN)SpiBus ));
          FreePool ( SpiBus );
        }
        else {
          //DEBUG (( DEBUG_ERROR, "ERROR - Failed to uninstall driver protocol, Status: %r\r\n", Status ));
        }
      }
    }
    else {
      //DEBUG (( DEBUG_ERROR, "ERROR - Failed to close driver protocol, Status: %r\r\n", Status ));
    }
  }
  //DEBUG (( DEBUG_LOAD | DEBUG_INFO, "SpiBusDriverStop exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Driver binding protocol support
**/
EFI_DRIVER_BINDING_PROTOCOL mSpiBusDriverBinding = {
  SpiBusDriverSupported,
  SpiBusDriverStart,
  SpiBusDriverStop,
  0x10,
  NULL,
  NULL
};

/**
  SPI driver entry point

  @param [in] ImageHandle       Handle for the image
  @param [in] pSystemTable      Address of the system table.

  @retval EFI_SUCCESS           Image successfully loaded.
  @retval EFI_OUT_OF_RESOURCES  Memory insufficient to install driver binding protocol.
**/
EFI_STATUS
EFIAPI
SpiBusDriverEntryPoint (
  IN EFI_HANDLE                 ImageHandle,
  IN EFI_SYSTEM_TABLE           *pSystemTable
  )
{
//  EFI_LOADED_IMAGE_PROTOCOL * pLoadedImage;
  EFI_STATUS Status = EFI_SUCCESS;
#if 0
  Status = gBS->HandleProtocol (
                  gImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID *)&pLoadedImage
                  );
#endif
  if (!EFI_ERROR (Status)) {
//    pLoadedImage->Unload = SpiDriverUnload;
    Status = EfiLibInstallDriverBindingComponentName2 (
               ImageHandle,
               pSystemTable,
               &mSpiBusDriverBinding,
               ImageHandle,
               NULL,
               NULL
               );
    if ( !EFI_ERROR ( Status )) {
      //DEBUG (( DEBUG_POOL | DEBUG_INIT | DEBUG_INFO, "Installed: gEfiDriverBindingProtocolGuid on   0x%016lx\r\n", (UINT64)((UINTN)ImageHandle )));
    }
  }
  return Status;
}





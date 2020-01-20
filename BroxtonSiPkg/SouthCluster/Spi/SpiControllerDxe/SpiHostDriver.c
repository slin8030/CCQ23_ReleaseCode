/** @file
  Implement the SPI controller driver binding protocol.

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
#include "Uefi.h"
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/MmioDevice.h>
#include <Protocol/SpiHost.h>

#include "SpiHost.h"

UINT32 mHid = EISA_PNP_ID(0x501);

/**
  Verify the controller type
  This routine determines if an SPI controller is available.
  This routine is called by the UEFI driver framework during connect
  processing.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to test.
  @param [in] RemainingDevicePath  Not used.

  @retval EFI_SUCCESS              This driver supports this device.
  @retval Others                   This driver does not support this device.
**/
EFI_STATUS
EFIAPI
SpiHostDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL   *DriverBinding,
  IN EFI_HANDLE                    Controller,
  IN EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  )
{
  ACPI_HID_DEVICE_PATH *AcpiPath;
  EFI_MMIO_DEVICE_PROTOCOL *MmioDevice;
  VOID *Protocol;
  EFI_STATUS Status;

  //DEBUG (( DEBUG_LOAD, "SpiHostDriverSupported entered\r\n" ));
  Status = gBS->OpenProtocol ( Controller,
                               &gEfiMmioDeviceProtocolGuid,
                               (VOID **)&MmioDevice,
                               DriverBinding->DriverBindingHandle,
                               Controller,
                               EFI_OPEN_PROTOCOL_GET_PROTOCOL );
  if ( !EFI_ERROR ( Status )) {
    AcpiPath = (ACPI_HID_DEVICE_PATH *)MmioDevice->AcpiPath;

    if ( AcpiPath->HID != mHid  ){
      Status = EFI_UNSUPPORTED;
    }
    else {
      DEBUG (( DEBUG_LOAD, "SPI controller found\r\n" ));
      Status = gBS->OpenProtocol ( Controller,
                                   &gEfiSpiHostProtocolGuid,
                                   &Protocol,
                                   DriverBinding->DriverBindingHandle,
                                   Controller,
                                   EFI_OPEN_PROTOCOL_GET_PROTOCOL );
      if ( !EFI_ERROR ( Status )) {
        Status = EFI_ALREADY_STARTED;
      }
      else {
        Status = EFI_SUCCESS;
      }
    }
  }
  //DEBUG (( DEBUG_LOAD, "SpiHostDriverSupported exiting, Status: %r\r\n", Status ));
  return Status;
}

/**
  Connect to the SPI controller

  This routine initializes an instance of the SPI host driver
  for this SPI controller.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to work with.
  @param [in] RemainingDevicePath  Not used, always produce all possible children.

  @retval EFI_SUCCESS              This driver is added to Controller.
  @retval Others                   This driver does not support this device.
**/
EFI_STATUS
EFIAPI
SpiHostDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL   *DriverBinding,
  IN EFI_HANDLE                    Controller,
  IN EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  )
{
  SPI_HOST_CONTEXT *SpiHost;
  EFI_MMIO_DEVICE_PROTOCOL *MmioDevice;
  EFI_STATUS Status;
  //DEBUG (( DEBUG_INFO, "SpiHostDriverStart entered\r\n" ));
  Status = gBS->OpenProtocol ( Controller,
                               &gEfiMmioDeviceProtocolGuid,
                               (VOID **)&MmioDevice,
                               DriverBinding->DriverBindingHandle,
                               Controller,
                               EFI_OPEN_PROTOCOL_BY_DRIVER );
  if ( !EFI_ERROR ( Status )) {
    SpiHost = AllocateZeroPool ( sizeof ( *SpiHost ));
    if ( NULL == SpiHost ) {
      //DEBUG (( DEBUG_ERROR, "ERROR - No memory for SPI host driver\r\n" ));
      Status = EFI_OUT_OF_RESOURCES;
    }
    else {
      //DEBUG (( DEBUG_POOL | DEBUG_INFO, "0x%016lx: SpiHost allocated\r\n", (UINT64)((UINTN)SpiHost )));
      SpiHost->Signature = SPI_HOST_SIGNATURE;
      SpiHost->MmioDevice = MmioDevice;
      //
      //  Start the driver
      //
      Status = SpiHostApiStart ( SpiHost );
      if ( !EFI_ERROR ( Status )) {
        //
        //  Install the driver protocol
        //
        Status = gBS->InstallMultipleProtocolInterfaces (
                        &Controller,
                        &gEfiSpiHostProtocolGuid,
                        &SpiHost->HostApi,
                        NULL
                        );
        if ( !EFI_ERROR ( Status )) {
          //DEBUG (( DEBUG_INIT, "0x%016lx: SpiHost started\r\n", (UINT64)(UINTN)SpiHost ));
        }
        else {
          SpiHostApiStop ( SpiHost );
        }
      }

      if ( EFI_ERROR ( Status )) {
        //DEBUG (( DEBUG_WARN, "WARNING - Failed to start SPI host, Status: %r\r\n", Status ));
        //DEBUG (( DEBUG_POOL | DEBUG_INFO, "0x%016lx: SpiHost released\r\n", (UINT64)(UINTN)SpiHost ));
        FreePool ( SpiHost );
      }
    }
  }
  //DEBUG (( DEBUG_LOAD, "SpiHostDriverStart exiting, Status: %r\r\n", Status ));
  return Status;
}

/**
  Disconnect from the SPI host controller.

  This routine disconnects from the SPI controller.
  This routine is called by DriverUnload when the SI host driver
  is being unloaded.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to stop driver on.
  @param [in] NumberOfChildren     How many children need to be stopped.
  @param [in] ChildHandleBuffer    Not used.

  @retval EFI_SUCCESS              This driver is removed Controller.
  @retval EFI_DEVICE_ERROR         The device could not be stopped due to a device error.
  @retval Others                   This driver was not removed from this device.
**/
EFI_STATUS
EFIAPI
SpiHostDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
{
  SPI_HOST_CONTEXT *SpiHost;
  EFI_SPI_HOST_PROTOCOL *SpiHostProtocol;
  EFI_STATUS Status;

  //DEBUG (( DEBUG_LOAD, "SpiHostDriverStop entered\r\n" ));


  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSpiHostProtocolGuid,
                  (VOID**)&SpiHostProtocol,
                  DriverBinding->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE );
  if ( !EFI_ERROR ( Status )) {
    SpiHost = SPI_HOST_CONTEXT_FROM_PROTOCOL ( SpiHostProtocol );
    Status = gBS->CloseProtocol ( Controller,
                                  &gEfiSpiHostProtocolGuid,
                                  DriverBinding->DriverBindingHandle,
                                  Controller );
    if ( !EFI_ERROR ( Status )) {
      Status = gBS->UninstallMultipleProtocolInterfaces (
                    Controller,
                    &gEfiSpiHostProtocolGuid,
                    SpiHostProtocol,
                    NULL );
      if ( !EFI_ERROR ( Status )) {
        //DEBUG (( DEBUG_INIT, "0x%016lx: SpiHost stopped\r\n", (UINT64)(UINTN)SpiHost ));
        SpiHostApiStop ( SpiHost );
        //DEBUG (( DEBUG_POOL | DEBUG_INFO, "0x%016lx: SpiHost released\r\n", (UINT64)(UINTN)SpiHost ));
        FreePool ( SpiHost );
      }
      else {
        //DEBUG (( DEBUG_ERROR, "ERROR - Failed to uninstall SPI host protocol, Status: %r\r\n", Status ));
      }
    }
    else {
      //DEBUG (( DEBUG_ERROR, "ERROR - Failed to close SPI host protocol, Status: %r\r\n", Status ));
    }
  }

  //DEBUG (( DEBUG_LOAD, "SpiHostDriverStop exiting, Status: %r\r\n", Status ));
  return Status;
}

/**
  Driver binding protocol support
**/
EFI_DRIVER_BINDING_PROTOCOL mSpiHostDriverBinding = {
  SpiHostDriverSupported,
  SpiHostDriverStart,
  SpiHostDriverStop,
  0x10,
  NULL,
  NULL
};

/**
  SPI Host driver entry point

  @param [in] ImageHandle       Handle for the image
  @param [in] pSystemTable      Address of the system table.

  @retval EFI_SUCCESS           Image successfully loaded.
  @retval EFI_OUT_OF_RESOURCES  Memory insufficient to install driver binding protocol.
**/
EFI_STATUS
EFIAPI
SpiHostDriverEntryPoint (
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
               &mSpiHostDriverBinding,
               ImageHandle,
               NULL,
               NULL
               );
    if ( !EFI_ERROR ( Status )) {
      DEBUG (( DEBUG_POOL | DEBUG_INIT | DEBUG_INFO,
                "Installed: gEfiDriverBindingProtocolGuid on   0x%016lx\r\n",
                (UINT64)((UINTN)ImageHandle )));
    }
  }
  return Status;
}


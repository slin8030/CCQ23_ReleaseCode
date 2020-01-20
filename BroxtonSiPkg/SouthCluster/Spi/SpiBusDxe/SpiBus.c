/** @file
  Implement SPI bus protocol.

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
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SpiAcpi.h>
#include "SpiBus.h"
#include <Library/SerialPortLib.h>



/**
  Enumerate the SPI bus

  This routine walks the platform specific data describing the
  SPI bus to create the SPI devices where driver GUIDs were
  specified.

  @param[in] SpiBus                Address of an SPI_BUS_CONTEXT structure
  @param[in] Controller            Handle to the controller

  @retval EFI_SUCCESS              The bus is successfully configured
  @retval EFI_OUT_OF_RESOURCES     Insufficient memory for SPI operation
  @retval Others                   Error occurs while enumerating SPI devices
**/
EFI_STATUS
SpiBusEnumerate (
  IN SPI_BUS_CONTEXT               *SpiBus,
  IN EFI_HANDLE                    Controller
  )
{
  EFI_SPI_DEVICE *Device;
  EFI_HANDLE Handle;
  SPI_DEVICE_CONTEXT *SpiDevice;
  SPI_DEVICE_CONTEXT *SpiDevicePrevious;
  EFI_STATUS Status;
  DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiBusEnumerate entered\r\n" ));
  Status = EFI_SUCCESS;
  Device = NULL;
  do {
    //
    //  Get the next SPI device
    //
    Status = SpiBus->SpiAcpi->Enumerate ( SpiBus->SpiAcpi,
                                          &Device );
    if ( EFI_ERROR ( Status )) {
      if ( EFI_NO_MAPPING != Status ) {
        break;
      }
      Device = NULL;
      Status = EFI_SUCCESS;
    }
    if ( NULL != Device ) {
      //
      //  Another device is on the SPI bus
      //  Determine if the device info is valid
      //
      if ( NULL != Device->DevicePath )

      {
        SpiDevice = AllocateZeroPool ( sizeof ( *SpiDevice ));
        if ( NULL == SpiDevice ) {
          DEBUG (( DEBUG_ERROR, "ERROR - No memory for SPI device structure!\r\n" ));
          Status = EFI_OUT_OF_RESOURCES;
          break;
        }

        SpiDevice->Signature = SPI_DEVICE_SIGNATURE;
        SpiDevice->SpiBus = SpiBus;
        SpiDevice->Device = Device;

        SpiDevice->BusApi.StartRequest = &SpiBusStartRequest;
        SpiDevice->BusApi.SPIRead = &SerialPortRead;
        SpiDevice->BusApi.SPIWrite = &SerialPortWrite;
        SpiDevice->BusApi.MaximumReceiveBytes = SpiBus->SpiHost->MaximumReceiveBytes;
        SpiDevice->BusApi.MaximumTransmitBytes = SpiBus->SpiHost->MaximumTransmitBytes;
        SpiDevice->BusApi.MaximumTotalBytes = SpiBus->SpiHost->MaximumTotalBytes;

        Status = SpiBusDevicePathAppend ( SpiDevice, Controller );
        if ( EFI_ERROR ( Status )) {
          //
          //  Out of resources
          //
          break;
        }

        //
        //  Install the protocol
        //
        Handle = NULL;
        Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiSpiBusProtocolGuid,
                  &SpiDevice->BusApi,
                  &gEfiDevicePathProtocolGuid,
                  SpiDevice->DevPath,
                  NULL );
        if ( EFI_ERROR ( Status )) {
          DEBUG (( DEBUG_ERROR,
                    "ERROR - Failed to install the device protocol, Status: %r\r\n",
                    Status ));
          break;
        }
        else {
          DEBUG((DEBUG_INFO, "Add device to SPI bus device list\r\n"));
          SpiDevicePrevious = SpiBus->DeviceListTail;
          if ( NULL == SpiDevicePrevious ) {
            SpiBus->DeviceListHead = SpiDevice;
          }
          else {
            SpiDevicePrevious->Next = SpiDevice;
          }
          SpiBus->DeviceListTail = SpiDevice;
        }
      }
    }
  } while ( NULL != Device );

  DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiBusEnumerate exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Perform an SPI operation on the device
  This routine queues an operation to the SPI controller for execution
  on the SPI bus.

  As an upper layer driver writer, the following need to be provided
  to the platform vendor:

  1.  ACPI CID value or string - this is used to connect the upper layer
      driver to the device.

  @param[in]  This              Address of an EFI_SPI_BUS_PROTOCOL
                                structure
  @param[in]  Event             Event to set for asynchronous operations,
                                NULL for synchronous operations
  @param[in]  RequestPacket     Address of an EFI_SPI_REQUEST_PACKET
                                structure describing the SPI operation
  @param[out] SpiStatus         Optional buffer to receive the SPI operation
                                completion status

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ABORTED           The request did not complete because the driver
                                was shutdown.
  @retval EFI_BAD_BUFFER_SIZE   The WriteBytes or ReadBytes buffer size is too large.
  @retval EFI_DEVICE_ERROR      There was an SPI error (NACK) during the operation.
                                This could indicate the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_NO_RESPONSE       The SPI device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller can not distinguish
                                when the NACK occurred.
  @retval EFI_NOT_FOUND         SPI slave address exceeds maximum address
  @retval EFI_NOT_READY         SPI bus is busy or operation pending, wait for
                                the event and then read status pointed to by
                                the request packet.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for SPI operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.
**/
EFI_STATUS
EFIAPI
SpiBusStartRequest (
  IN EFI_SPI_BUS_PROTOCOL       *This,
  IN EFI_EVENT                  Event OPTIONAL,
  IN EFI_SPI_REQUEST_PACKET     *RequestPacket,
  OUT EFI_STATUS                *SpiStatus OPTIONAL
  )
{
  EFI_SPI_DEVICE *Device;
  SPI_BUS_CONTEXT *SpiBus;
  EFI_SPI_HOST_PROTOCOL *SpiHost;
  SPI_DEVICE_CONTEXT *SpiDevice;
  EFI_STATUS Status;
  DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiBusStartRequest entered\r\n" ));
  Status = EFI_ACCESS_DENIED;

  SpiDevice = SPI_DEVICE_CONTEXT_FROM_PROTOCOL ( This );

  SpiBus = SpiDevice->SpiBus;
  SpiHost = SpiBus->SpiHost;
  Device = SpiDevice->Device;
  DEBUG (( DEBUG_SPI_OPERATION,
           "SpiBus: WriteBytes: %d\r\nWriteBuffer: 0x%016Lx\r\nReadBytes: %d\r\nReadBuffer: 0x%016Lx\r\nTimeout: %d\r\n",
           RequestPacket->WriteBytes,
           (UINT64)(UINTN)RequestPacket->WriteBuffer,
           RequestPacket->ReadBytes,
           (UINT64)(UINTN)RequestPacket->ReadBuffer,
           RequestPacket->Timeout ));
    //
    //  Start the SPI operation
    //
  Status = SpiHost->StartRequest (
    SpiHost,
    Device,
    Event,
    RequestPacket,
    SpiStatus );
  DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiBusStartRequest exiting, Status: %r\r\n", Status ));
  return Status;
}

/**
  Start the SPI driver

  This routine allocates the necessary resources for the driver.
  This routine is called by SpiBusDriverStart to complete the driver
  initialization.

  @param [in] SpiBus           Address of an SPI_BUS_CONTEXT structure
  @param [in] Controller       Handle to the controller

  @retval EFI_SUCCESS          Driver API properly initialized
  @retval Others               Error occurs during Spi bus enumeration
**/
EFI_STATUS
SpiBusApiStart (
  IN SPI_BUS_CONTEXT           *SpiBus,
  IN EFI_HANDLE                Controller
  )
{
  EFI_STATUS Status;

  DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiBusApiStart entered\r\n" ));
  Status = SpiBusEnumerate ( SpiBus, Controller );
  DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiBusApiStart exiting, Status: %r\r\n", Status ));
  return Status;
}

/**
  Stop the SPI driver

  This routine releases the resources allocated by SpiApiStart.
  This routine is called by SpiBusDriverStop to initiate the driver
  shutdown.

  @param [in] SpiBus          Address of an SPI_BUS_CONTEXT structure
**/
VOID
SpiBusApiStop (
  IN SPI_BUS_CONTEXT          *SpiBus
  )
{
  EFI_HANDLE *Handle;
  EFI_HANDLE *HandleArray;
  EFI_HANDLE *HandleArrayEnd;
  UINTN HandleCount;
  SPI_DEVICE_CONTEXT *SpiDevice;
  SPI_DEVICE_CONTEXT *SpiDevicePrevious;
  EFI_SPI_BUS_PROTOCOL *SpiBusProtocol;
  EFI_STATUS Status;
  DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiBusApiStop entered\r\n" ));

  //
  //  Locate the SPI devices
  //
  Status = gBS->LocateHandleBuffer ( ByProtocol,
                                     &gEfiSpiBusProtocolGuid,
                                     NULL,
                                     &HandleCount,
                                     &HandleArray );
  if ( !EFI_ERROR ( Status ))  {
    Handle = HandleArray;
    HandleArrayEnd = &Handle [ HandleCount ];
    while ( HandleArrayEnd > Handle ) {
      Status = gBS->OpenProtocol ( *Handle,
                                   &gEfiSpiBusProtocolGuid,
                                   (VOID **)&SpiBusProtocol,
                                   mSpiBusDriverBinding.DriverBindingHandle,
                                   *Handle,
                                   EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE );
      if ( EFI_ERROR ( Status )) {
        DEBUG (( DEBUG_ERROR,
                  "ERROR - Failed to remove SPI driver stack, Status: %r\r\n",
                  Status ));
      }
      else {
        SpiDevice = SPI_DEVICE_CONTEXT_FROM_PROTOCOL ( SpiBusProtocol );
        gBS->CloseProtocol ( *Handle,
                             &gEfiSpiBusProtocolGuid,
                             mSpiBusDriverBinding.DriverBindingHandle,
                             *Handle );
        Status = gBS->UninstallMultipleProtocolInterfaces ( *Handle,
                                                            &gEfiSpiBusProtocolGuid,
                                                            &SpiDevice->BusApi,
                                                            &gEfiDevicePathProtocolGuid,
                                                            SpiDevice->DevPath,
                                                            NULL );
        if ( EFI_ERROR ( Status )) {
          DEBUG (( DEBUG_ERROR,
                    "ERROR - Failed to uninstall gEfiSpiBusProtocol, Status: %r\r\n",
                    Status ));
        }
        else {
#if 0
          //
          //  Synchronize with the other threads
          //
          TplPrevious = gBS->RaiseTPL ( TPL_SPI_SYNC );
#endif
          SpiDevicePrevious = SpiBus->DeviceListHead;
          if ( SpiDevice != SpiDevicePrevious ) {
            //
            //  The device is at the head of the list
            //
            SpiBus->DeviceListHead = SpiDevice->Next;
            SpiDevicePrevious = NULL;
          }
          else {
            //
            //  Locate the device in the middle of the list
            //
            while ( SpiDevice != SpiDevicePrevious->Next ) {
              SpiDevicePrevious = SpiDevicePrevious->Next;
            }
            SpiDevicePrevious->Next = SpiDevice->Next;
          }
          if ( SpiBus->DeviceListTail == SpiDevice ) {
            SpiBus->DeviceListTail = SpiDevicePrevious;
          }
#if 0
          //
          //  Release the thread synchronization
          //
          gBS->RestoreTPL ( TplPrevious );
#endif
          DEBUG (( DEBUG_SPI_DEVICE,
                    "0x%016Lx: Bus freeing SPI Device\r\n",
                    SpiDevice));
          FreePool ( SpiDevice );
        }
      }
      Handle += 1;
    }
    FreePool ( HandleArray );
  }
  DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiBusApiStop exiting\r\n" ));
}

/** @file
  SPI Bus Driver Declarations

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

#ifndef _SPI_BUS_LOCAL_H_
#define _SPI_BUS_LOCAL_H_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/SpiAcpi.h>
#include <Protocol/SpiHost.h>
#include <Protocol/SpiBus.h>

#define DEBUG_SPI_OPERATION           0x40000000   ///< Debug SPI operation

#define DEBUG_SPI_ROUTINE_ENTRY_EXIT  0x20000000   ///< Debug routine entry and exit

#define DEBUG_SPI_DEVICE              0x10000000   ///< Debug SPI bus configuration

#define SPI_BUS_SIGNATURE             0x62495053   ///< "SPIb"

#define SPI_DEVICE_SIGNATURE          0x64495053   ///< "SPId"


extern EFI_DRIVER_BINDING_PROTOCOL mSpiBusDriverBinding;


typedef struct _SPI_BUS_CONTEXT SPI_BUS_CONTEXT;
typedef struct _SPI_DEVICE_CONTEXT SPI_DEVICE_CONTEXT;

///
/// SPI device context
///
/// Each SPI device instance uses an SPI_DEVICE_CONTEXT structure
/// to manage access to the SPI device
///
struct _SPI_DEVICE_CONTEXT{
  ///
  /// Structure identification
  ///
  UINTN Signature;

  ///
  /// Next device in the list
  ///
  SPI_DEVICE_CONTEXT *Next;

  ///
  /// Upper level API to support the SPI device I/O
  ///
  EFI_SPI_BUS_PROTOCOL BusApi;
  ///
  /// Device path for this device
  ///
  EFI_DEVICE_PATH_PROTOCOL *DevPath;

  ///
  /// Platform specific data for this device
  ///
  EFI_SPI_DEVICE *Device;

  ///
  /// Context for the common I/O support including the
  /// lower level API to the host controller.
  ///
  SPI_BUS_CONTEXT *SpiBus;
};

#define SPI_DEVICE_CONTEXT_FROM_PROTOCOL(a) CR (a, SPI_DEVICE_CONTEXT, BusApi, SPI_DEVICE_SIGNATURE)  ///< Locate SPI_DEVICE_CONTEXT from protocol

/**
  SPI bus context

  Each SPI bus instance uses an SPI_BUS_CONTEXT structure
  to manage the SPI host.
**/
struct _SPI_BUS_CONTEXT{
  ///
  /// Structure identification
  ///
  UINTN Signature;

  ///
  /// Head of the device list
  ///
  SPI_DEVICE_CONTEXT *DeviceListHead;

  ///
  /// Tail of the device list
  ///
  SPI_DEVICE_CONTEXT *DeviceListTail;

  ///
  /// Platform API to enumerate the SPI devices
  ///
  EFI_SPI_ACPI_PROTOCOL *SpiAcpi;

  ///
  /// Lower level API to the host controller
  ///
  EFI_SPI_HOST_PROTOCOL *SpiHost;
};

///
/// Locate SPI_BUS_CONTEXT from protocol
///
#define SPI_BUS_CONTEXT_FROM_PROTOCOL(a)  CR (a, SPI_BUS_CONTEXT, Signature, SPI_BUS_SIGNATURE)


/**
  Start the SPI driver

  This routine allocates the necessary resources for the driver.
  This routine is called by SpiBusDriverStart to complete the driver
  initialization.

  @param[in] SpiBus           Address of an SPI_BUS_CONTEXT structure
  @param[in] Controller       Handle to the controller

  @retval EFI_SUCCESS         Driver API properly initialized
  @retval Others              Error occurs during Spi bus enumeration
**/
EFI_STATUS
SpiBusApiStart (
  IN SPI_BUS_CONTEXT          *SpiBus,
  IN EFI_HANDLE               Controller
  );

/**
  Stop the SPI driver

  This routine releases the resources allocated by SpiApiStart.
  This routine is called by SpiBusDriverStop to initiate the driver
  shutdown.

  @param[in] SpiBus           Address of an SPI_BUS_CONTEXT structure
**/
VOID
SpiBusApiStop (
  IN SPI_BUS_CONTEXT          *SpiBus
  );

/**
  Create a path for the SPI device
  Append the SPI slave path to the SPI master controller path.

  @param [in] SpiDevice             Address of an SPI_DEVICE_CONTEXT structure.
  @param [in] Controller            Handle to the controller

  @retval EFI_SUCCESS               The function completes successfully
  @retval EFI_OUT_OF_RESOURCES      No available address to assign
**/
EFI_STATUS
SpiBusDevicePathAppend (
  IN SPI_DEVICE_CONTEXT             *SpiDevice,
  IN EFI_HANDLE                     Controller
  );

/**
  Perform an SPI operation on the device

  This routine queues an operation to the SPI controller for execution
  on the SPI bus.

  As an upper layer driver writer, the following need to be provided
  to the platform vendor:

  1.  ACPI CID value or string - this is used to connect the upper layer
      driver to the device.

  @param[in] This               Address of an EFI_SPI_BUS_PROTOCOL
                                structure
  @param[in] Event              Event to set for asynchronous operations,
                                NULL for synchronous operations
  @param[in] RequestPacket      Address of an EFI_SPI_REQUEST_PACKET
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
  OUT EFI_STATUS                *SpiStatus
  );

#endif

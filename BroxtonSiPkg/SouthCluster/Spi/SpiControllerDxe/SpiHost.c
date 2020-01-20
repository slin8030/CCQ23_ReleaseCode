/** @file
  Implement the SPI host protocol.

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
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SpiAcpi.h>
#include <Protocol/SpiHost.h>
#include "SpiHost.h"
#include "SpiController.h"

/**
  Queue an SPI operation for execution on the SPI controller.

  @param[in] This               Address of an EFI_SPI_HOST_PROTOCOL instance.
  @param[in] SpiDevice          A pointer to EFI_SPI_DEVICE instance.
  @param[in] Event              Event to set for asynchronous operations,
                                NULL for synchronous operations
  @param[in] RequestPacket      Address of an EFI_SPI_REQUEST_PACKET
  @param[out] SpiStatus         Optional buffer to receive the SPI operation
                                completion status

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER
  @retval EFI_ABORTED           The request did not complete because the driver
                                was shutdown.
  @retval EFI_BAD_BUFFER_SIZE   The WriteBytes or ReadBytes buffer size is too large.
  @retval EFI_DEVICE_ERROR      There was an SPI error during the operation.
                                This could indicate the slave device is not present.
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
SpiHostStartRequest (
  IN EFI_SPI_HOST_PROTOCOL      *This,
  IN EFI_SPI_DEVICE             *SpiDevice,
  IN EFI_EVENT                  Event OPTIONAL,
  IN EFI_SPI_REQUEST_PACKET     *RequestPacket,
  OUT EFI_STATUS                *SpiStatus
  )
{
  SPI_HOST_CONTEXT *SpiHost;
  EFI_STATUS Status;
  SPI_TARGET_SETTINGS *pTargetSettings;
  //DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiHostStartRequest entered\r\n" ));
  //
  //Assuming success
  //
  Status = EFI_SUCCESS;
  pTargetSettings = (SPI_TARGET_SETTINGS *)SpiDevice->targetSettings;
  if ( NULL == RequestPacket || NULL ==  pTargetSettings) {
    //DEBUG (( DEBUG_WARN, "WARNING - RequestPacket:0x%016x pTargetSettings:0x%016x is NULL\r\n", (UINT16)(UINTN)RequestPacket, (UINT16)(UINTN)pTargetSettings ));
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    SpiHost = SPI_HOST_CONTEXT_FROM_PROTOCOL ( This );
    Status = SpiTransfer(
      (void *)SpiHost->MmioDevice->DriverResources,
      pTargetSettings,
      RequestPacket,
      SpiStatus
      );
  }
  //DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiHostRequestQueue exiting, Status: %r\r\n", Status ));
  return Status;
}

/**
  Start the SPI driver

  This routine allocates the necessary resources for the driver.
  This routine is called by SpiHostDriverStart to complete the driver
  initialization.

  @param [in] SpiHost         Address of an SPI_HOST_CONTEXT structure

  @retval EFI_SUCCESS         Driver API properly initialized
**/
EFI_STATUS
SpiHostApiStart (
  IN SPI_HOST_CONTEXT         *SpiHost
  )
{
  EFI_STATUS Status;
  //DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiHostApiStart entered\r\n" ));
  Status = EFI_SUCCESS;

  SpiReset ( (void *)SpiHost->MmioDevice->DriverResources);
  SpiHost->HostApi.StartRequest = SpiHostStartRequest;
  SpiHost->HostApi.MaximumReceiveBytes = SpiMaxReadBytes;
  SpiHost->HostApi.MaximumTransmitBytes = SpiMaxWriteBytes;
  SpiHost->HostApi.MaximumTotalBytes = SpiMaxTransmitBytes;

  //DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiHostApiStart exiting, Status: %r\r\n", Status ));
  return Status;
}

/**
  Stop the SPI driver

  This routine releases the resources allocated by SpiHostApiStart.
  This routine is called by SpiHostDriverStop to initiate the driver
  shutdown.

  @param [in] SpiHost         Address of an SPI_HOST_CONTEXT structure
**/
VOID
SpiHostApiStop (
  IN SPI_HOST_CONTEXT         *SpiHost
  )
{
  //DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiHostApiStop entered\r\n" ));
  //DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiHostApiStop exiting\r\n" ));
}

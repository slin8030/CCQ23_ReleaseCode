/** @file
  MMIO device protocol as defined in the UEFI 2.x.x specification.

  The MMIO device protocol defines a memory mapped I/O device
  for use by the system.

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

@par Specification
**/

#ifndef __SPI_HOST_H__
#define __SPI_HOST_H__

//#include <Library/SpiTargetSettings.h>
//#include <Protocol/DevicePath.h>
#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/MmioDevice.h>
//#include <Protocol/SpiHost.h>
#include <Protocol/SpiAcpi.h>
//#include "SpiRegisters.h"
//#include "SpiController.h"



/////
/////  EFI SPI Device
/////
//typedef struct {
//  UINT8 Index;
//  EFI_DEVICE_PATH_PROTOCOL DevicePath;
//  SPI_TARGET_SETTINGS targetSettings;
//} EFI_SPI_DEVICE;

//extern EFI_GUID gEfiMmioDeviceProtocolGuid;

/// SPI device operation
///
/// This structure provides the information necessary for an operation
/// on an SPI device
///
typedef struct {
  ///
  /// Number of bytes to send to the SPI device
  ///
  UINT32 WriteBytes;

  ///
  /// Number of bytes to read, set to zero for write only operations
  ///
  UINT32 ReadBytes;

  ///
  /// Address of the buffer containing the data to send to the SPI device.
  /// The WriteBuffer must be at least WriteBytes in length.
  ///
  UINT8 *WriteBuffer;

  ///
  /// Address of the buffer to receive data from the SPI device. Use NULL
  /// for write only operations.  The ReadBuffer must be at least ReadBytes
  /// in length.
  ///
  UINT8 *ReadBuffer;

  UINT32 BytesWritten;

  UINT32 BytesRead;

  ///
  /// Timeout for the SPI operation in 100 ns units
  ///
  UINT32 Timeout;
} EFI_SPI_REQUEST_PACKET;


typedef struct _EFI_SPI_ACPI_PROTOCOL EFI_SPI_ACPI_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_SPI_ENUMERATE) (
  IN EFI_SPI_ACPI_PROTOCOL *This,
  IN OUT EFI_SPI_DEVICE **Device
  );

struct _EFI_SPI_ACPI_PROTOCOL{
  ///
  /// Start an I2C operation on the bus
  ///
  EFI_SPI_ENUMERATE Enumerate;
};


typedef struct _EFI_SPI_HOST_PROTOCOL EFI_SPI_HOST_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI* SPI_HOST_STARTREQUEST) (
  IN EFI_SPI_HOST_PROTOCOL *This,
  IN EFI_SPI_DEVICE * SpiDevice,
  IN EFI_EVENT Event OPTIONAL,
  IN EFI_SPI_REQUEST_PACKET *RequestPacket,
  OUT EFI_STATUS *SpiStatus
  );

struct _EFI_SPI_HOST_PROTOCOL{
  SPI_HOST_STARTREQUEST StartRequest;
  UINT32 MaximumReceiveBytes;
  UINT32 MaximumTransmitBytes;
  UINT32 MaximumTotalBytes;
};


extern EFI_GUID gEfiSpiHostProtocolGuid;


//extern EFI_GUID gEfiMmioDeviceProtocolGuid;

#endif  //  __SPI_HOST_H__

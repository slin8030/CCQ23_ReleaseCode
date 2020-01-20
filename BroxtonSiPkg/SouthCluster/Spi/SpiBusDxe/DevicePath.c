/** @file
  Implement The Device Path Extension

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

#include "SpiBus.h"

/**
  Driver path template
**/
CONST STATIC EFI_DEVICE_PATH_PROTOCOL mEndOfPath = {
  END_DEVICE_PATH_TYPE,
  END_ENTIRE_DEVICE_PATH_SUBTYPE,
  {
    END_DEVICE_PATH_LENGTH,
    0
  }
};


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
  )
{
  UINT8 *Buffer;
  EFI_DEVICE_PATH_PROTOCOL *DevPath;
  UINTN LengthInBytes;
  EFI_STATUS Status;

  DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiBusDevicePathAppend entered\r\n" ));

  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID**) &DevPath
                  );
  if ( !EFI_ERROR (Status)) {
    //
    //  Allocate a buffer
    //
    LengthInBytes = DevicePathNodeLength ( SpiDevice->Device->DevicePath );
    Buffer = AllocateZeroPool ( LengthInBytes + sizeof ( mEndOfPath ));
    if ( NULL == Buffer ) {
      DEBUG (( DEBUG_ERROR,
                "ERROR - Failed to allocate device path buffer!\r\n" ));
      Status = EFI_OUT_OF_RESOURCES;
    }
    else {
      //
      //  Build the new device path
      //
      CopyMem ( Buffer,
                SpiDevice->Device->DevicePath,
                LengthInBytes );
      CopyMem ( &Buffer [ LengthInBytes ],
                &mEndOfPath,
                sizeof ( mEndOfPath ));
      SpiDevice->DevPath = AppendDevicePath ( DevPath,
                                              (EFI_DEVICE_PATH_PROTOCOL *)Buffer );
      if ( NULL == SpiDevice->DevPath ) {
        Status = EFI_OUT_OF_RESOURCES;
      }

      //
      //  Free the buffer
      //
      FreePool ( Buffer );
    }
  }

  DEBUG (( DEBUG_SPI_ROUTINE_ENTRY_EXIT, "SpiBusDevicePathAppend exiting, Status: %r\r\n", Status ));
  return Status;
}

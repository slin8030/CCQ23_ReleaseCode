/** @file
  Implement SPI device enumeration.

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
#include <Protocol/SpiAcpi.h>
#include "SpiEnumLib.h"

/**
  Enumerate the SPI devices
  This routine must be called at or below TPL_NOTIFY.
  This function walks the platform specific data to enumerates the
  SPI devices on an SPI bus.

  @param[in]  This              Address of an EFI_SPI_ACPI_PROTOCOL
                                structure.
  @param[in]  Device            Buffer containing the address of an
                                EFI_SPI_DEVICE structure.  Enumeration
                                is started by setting the initial
                                EFI_SPI_DEVICE structure address to NULL.
                                The buffer receives an EFI_SPI_DEVICE
                                structure address for the next SPI device.

  @retval EFI_SUCCESS           The platform data for the next device
                                on the SPI bus was returned successfully.
  @retval EFI_INVALID_PARAMETER Device was NULL
  @retval EFI_NO_MAPPING        Device does not point to a valid
                                EFI_SPI_DEVICE structure.
**/
EFI_STATUS
EFIAPI
SpiEnumerate (
  IN EFI_SPI_ACPI_PROTOCOL      *This,
  IN OUT EFI_SPI_DEVICE         **Device
  )
{
  SPI_BUS_CONFIGURATION *SpiBusConfiguration;
  EFI_SPI_DEVICE *NextDevice;
  EFI_SPI_DEVICE *NextDeviceEnd;
  BOOLEAN Found;
  SPI_ENUM_CONTEXT *SpiEnumContext;
  CONST EFI_SPI_DEVICE *PreviousDevice;
  EFI_STATUS Status;

  //
  //  Assume the device is not found
  //
  Status = EFI_NO_MAPPING;

  //
  //  Validate the return device
  //
  if ( NULL == Device ) {
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Walk the list of SPI bus configurations
    //
    SpiEnumContext = SPI_ENUM_CONTEXT_FROM_PROTOCOL ( This );
    SpiBusConfiguration = SpiEnumContext->SpiBusConfiguration;
    if (SpiBusConfiguration == NULL){
        return Status;
    }
    Found = FALSE;
    NextDevice = NULL;
    PreviousDevice = *Device;
    //
    //  Walk the list of devices
    //
    NextDevice = SpiBusConfiguration->DeviceList;
    if (NextDevice == NULL){
        return Status;
    }
    NextDeviceEnd = &NextDevice [ SpiBusConfiguration->DeviceCount ];
    while ( NextDeviceEnd > NextDevice ) {
      if ( PreviousDevice == NextDevice ) {
        Found = TRUE;
      }
      else {
        if (( NULL == PreviousDevice ) || Found ) {
          break;
        }
      }
      NextDevice += 1;
    }
    if ( NextDeviceEnd == NextDevice ) {
       NextDevice = NULL;
    }else{
       Status = EFI_SUCCESS;
    }
    *Device = NextDevice;

  }
  return Status;
}

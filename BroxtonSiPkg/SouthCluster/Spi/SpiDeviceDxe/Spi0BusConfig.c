/** @file
  Header file for SPI bus configuration

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
#include <Protocol/SpiAcpi.h>
#include <Library/SpiTargetSettings.h>
#include "SpiMmioConfig.h"
#include "SpiEnumLib.h"

///----------------------------------------------------------------------
///  ACPI paths
///----------------------------------------------------------------------

UART_DEVICE_PATH mSpiUart_Acpi = {
  {
    MESSAGING_DEVICE_PATH,
    MSG_UART_DP,
  (UINT8) (sizeof (UART_DEVICE_PATH)),
  (UINT8) ((sizeof (UART_DEVICE_PATH)) >> 8)
  },
  0,
  115200,
  8,
  1,
  1
  };

SPI_TARGET_SETTINGS mE2PROM_target = {
 0,         ///< WireMode
 1,         ///< ChipSelectLine
 0,         ///< ChipSelectPolarity
 0,         ///< SerialClockPolarity
 0,         ///< SerialClockPhase
 8,         ///< DataFrameSize, supported are 8, 16, 32
 10000000,  ///< clock speed
 0,         ///< loopback mode
 1,         ///< bytes per entry
 10
};

EFI_SPI_DEVICE mSpi0DeviceList [ ] = {
   {0,NULL,NULL}
};

EFI_SPI_DEVICE mSpi1DeviceList [ ] = {
  {1,  (UART_DEVICE_PATH *)&mSpiUart_Acpi,  &mE2PROM_target}
};

EFI_SPI_DEVICE mSpi2DeviceList [ ] = {
   {0,NULL,NULL}
};

//----------------------------------------------------------------------
//  Bus configurations
//----------------------------------------------------------------------

SPI_BUS_CONFIGURATION mSpi0BusConfiguration  = {
  0,                      ///< Number of devices in the bus configuration
  &mSpi0DeviceList [ 0 ]  ///< List of devices in the bus configuration
};

SPI_BUS_CONFIGURATION mSpi1BusConfiguration  = {
  DIM ( mSpi1DeviceList ),///< Number of devices in the bus configuration
  &mSpi1DeviceList [ 0 ]  ///< List of devices in the bus configuration
};

SPI_BUS_CONFIGURATION mSpi2BusConfiguration  = {
  0,                      ///< Number of devices in the bus configuration
  &mSpi2DeviceList [ 0 ]  ///< List of devices in the bus configuration
};


SPI_ENUM_CONTEXT mSpi0BusEnum = {
  SPI_ENUM_SIGNATURE,
  &SpiEnumerate,
  NULL
};

SPI_ENUM_CONTEXT mSpi1BusEnum = {
  SPI_ENUM_SIGNATURE,
  &SpiEnumerate,
  &mSpi1BusConfiguration
};

SPI_ENUM_CONTEXT mSpi2BusEnum = {
  SPI_ENUM_SIGNATURE,
  &SpiEnumerate,
  NULL
};

SPI_PLATFORM_CONTEXT mSpi0ControllerConfig = {
  0x0ULL,          // Base address
  100000000ULL
};

SPI_PLATFORM_CONTEXT mSpi1ControllerConfig = {
  0x0ULL,          // Base address
  100000000ULL
};

SPI_PLATFORM_CONTEXT mSpi2ControllerConfig = {
  0x0ULL,          // Base address
  100000000ULL
};


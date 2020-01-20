/** @file
  Header file for PCH Serial IO Lib implementation.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

#ifndef _PCH_SERIAL_IO_LIB_H_
#define _PCH_SERIAL_IO_LIB_H_

typedef enum {
  PchSerialIoIndexI2C0,
  PchSerialIoIndexI2C1,
  PchSerialIoIndexI2C2,
  PchSerialIoIndexI2C3,
  PchSerialIoIndexI2C4,
  PchSerialIoIndexI2C5,
  PchSerialIoIndexI2C6,
  PchSerialIoIndexI2C7,

  PchSerialIoIndexUart0,
  PchSerialIoIndexUart1,
  PchSerialIoIndexUart2,
  PchSerialIoIndexUart3,
  PchSerialIoIndexSpi0,
  PchSerialIoIndexSpi1,
  PchSerialIoIndexSpi2,
  PchSerialIoIndexMax
} PCH_SERIAL_IO_CONTROLLER;

typedef enum {
  PchSerialIoDisabled,
  PchSerialIoAcpi,
  PchSerialIoPci,
  PchSerialIoAcpiHidden,
  PchSerialIoLegacyUart
} PCH_SERIAL_IO_MODE;

enum PCH_LP_SERIAL_IO_VOLTAGE_SEL {
  PchSerialIoIs33V = 0,
  PchSerialIoIs18V
};
enum PCH_LP_SERIAL_IO_CS_POLARITY {
  PchSerialIoCsActiveLow = 0,
  PchSerialIoCsActiveHigh = 1
};
enum PCH_LP_SERIAL_IO_HW_FLOW_CTRL {
  PchSerialIoHwFlowCtrlDisabled = 0,
  PchSerialIoHwFlowControlEnabled = 1
};

#define SERIALIO_HID_LENGTH 8 // including null terminator
#define SERIALIO_UID_LENGTH 1
#define SERIALIO_CID_LENGTH 1
#define SERIALIO_TOTAL_ID_LENGTH SERIALIO_HID_LENGTH+SERIALIO_UID_LENGTH+SERIALIO_CID_LENGTH

/**
  Returns index of the last i2c controller

  @retval Value           Index of I2C controller
**/
PCH_SERIAL_IO_CONTROLLER
GetMaxI2cNumber (
  VOID
  );

/**
  Configures Serial IO Controller

  @param[in] Controller                 0=I2C0, ..., 11=UART2
  @param[in] DeviceMode                 Different type of serial io mode defined in PCH_SERIAL_IO_MODE
  @param[in] SerialIoSafeRegister       D0i3 Max Power On Latency and Device PG config
**/
VOID
ConfigureSerialIoController (
  IN PCH_SERIAL_IO_CONTROLLER Controller,
  IN PCH_SERIAL_IO_MODE       DeviceMode
#ifdef PCH_PO_FLAG
  , IN UINT32                 SerialIoSafeRegister
#endif
  );

#if 0
/**
  Initializes GPIO pins used by SerialIo I2C devices

  @param[in] Controller                 0=I2C0, ..., 11=UART2
  @param[in] DeviceMode                 Different type of serial io mode defined in PCH_SERIAL_IO_MODE
  @param[in] I2cVoltage                 Select I2C voltage, 1.8V or 3.3V
**/
VOID
SerialIoI2cGpioInit (
  IN PCH_SERIAL_IO_CONTROLLER Controller,
  IN PCH_SERIAL_IO_MODE       DeviceMode,
  IN UINT32                   I2cVoltage
  );

/**
  Initializes GPIO pins used by SerialIo SPI devices

  @param[in] Controller                 0=I2C0, ..., 11=UART2
  @param[in] DeviceMode                 Different type of serial io mode defined in PCH_SERIAL_IO_MODE
  @param[in] SpiCsPolarity              SPI CS polarity
**/
VOID
SerialIoSpiGpioInit (
  IN PCH_SERIAL_IO_CONTROLLER Controller,
  IN PCH_SERIAL_IO_MODE       DeviceMode,
  IN UINT32                   SpiCsPolarity
  );

/**
  Initializes GPIO pins used by SerialIo devices

  @param[in] Controller                 0=I2C0, ..., 11=UART2
  @param[in] DeviceMode                 Different type of serial io mode defined in PCH_SERIAL_IO_MODE
  @param[in] HardwareFlowControl        Hardware flow control method
**/
VOID
SerialIoUartGpioInit (
  IN PCH_SERIAL_IO_CONTROLLER Controller,
  IN PCH_SERIAL_IO_MODE       DeviceMode,
  IN BOOLEAN                  HardwareFlowControl
  );
#endif

/**
  Finds PCI Device Number of SerialIo devices.
  SerialIo devices' BDF is configurable

  @param[in] SerialIoDevice             0=I2C0, ..., 11=UART2

  @retval UINT8                         SerialIo device number
**/
UINT8
GetSerialIoDeviceNumber (
  IN PCH_SERIAL_IO_CONTROLLER  SerialIoNumber
  );

/**
  Finds PCI Function Number of SerialIo devices.
  SerialIo devices' BDF is configurable

  @param[in] SerialIoDevice             0=I2C0, ..., 11=UART2

  @retval UINT8                         SerialIo funciton number
**/
UINT8
GetSerialIoFunctionNumber (
  IN PCH_SERIAL_IO_CONTROLLER  SerialIoNumber
  );

/**
  Finds BAR values of SerialIo devices.
  SerialIo devices can be configured to not appear on PCI so traditional method of reading BAR might not work.
  If the SerialIo device is in PCI mode, a request for BAR1 will return its PCI CFG space instead

  @param[in] SerialIoDevice             0=I2C0, ..., 11=UART2
  @param[in] BarNumber                  0=BAR0, 1=BAR1

  @retval UINTN                         SerialIo Bar value
**/
UINTN
FindSerialIoBar (
  IN PCH_SERIAL_IO_CONTROLLER           SerialIoDevice,
  IN UINT8                              BarNumber
  );

//[-start-160629-IB07400750-add]//
VOID
SaveRestoreSerialIoPci (
  IN PCH_SERIAL_IO_CONTROLLER SerialIoDevice,
  IN BOOLEAN                  SaveRestore   // TRUE: SAVE, FALSE: RESTORE
  );
//[-end-160629-IB07400750-add]//

#endif // _PEI_DXE_SMM_PCH_SERIAL_IO_LIB_H_

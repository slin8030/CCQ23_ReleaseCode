/** @file
  Header file for PCH Serial IO UART Lib implementation.

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

@par Specification
**/
#ifndef _PCH_SERIAL_IO_UART_LIB_H_
#define _PCH_SERIAL_IO_UART_LIB_H_

/**
  Initialize selected SerialIo UART.
  This init function MUST be used prior any SerialIo UART functions to init serial io controller if platform is going use serialio UART as debug output.

  @param[in]  UartNumber           Selects Serial IO UART device (0-2)
  @param[in]  FifoEnable           When TRUE, enables 64-byte FIFOs.
  @param[in]  BaudRate             Baud rate.
  @param[in]  LineControl          Data length, parity, stop bits.
  @param[in]  HardwareFlowControl  Automated hardware flow control. If TRUE, hardware automatically checks CTS when sending data, and sets RTS when receiving data.
**/
VOID
EFIAPI
PchSerialIoUartInit (
  IN UINT8      UartNumber,
  IN BOOLEAN    FifoEnable,
  IN UINT32     BaudRate,
  IN UINT8      LineControl,
  IN BOOLEAN    HardwareFlowControl
  );


/**
  Write data to serial device.

  If the buffer is NULL, then return 0;
  if NumberOfBytes is zero, then return 0.

  @param[in]  UartNumber       Selects Serial IO UART device (0-2)
  @param[in]  Buffer           Point of data buffer which need to be writed.
  @param[in]  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval UINTN                Actual number of bytes writed to serial device.
**/
UINTN
EFIAPI
PchSerialIoUartOut (
  IN UINT8  UartNumber,
  IN UINT8  *Buffer,
  IN UINTN  NumberOfBytes
  );

/**
  Read data from serial device and save the datas in buffer.

  If the buffer is NULL, then return 0;
  if NumberOfBytes is zero, then return 0.

  @param[in]  UartNumber           Selects Serial IO UART device (0-2)
  @param[out] Buffer               Point of data buffer which need to be writed.
  @param[in]  NumberOfBytes        Number of output bytes which are cached in Buffer.
  @param[in]  WaitUntilBufferFull  When TRUE, function waits until whole buffer is filled. When FALSE,
                                   function returns as soon as no new characters are available.

  @retval UINTN                    Actual number of bytes raed to serial device.
**/
UINTN
EFIAPI
PchSerialIoUartIn (
  IN  UINT8     UartNumber,
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes,
  IN  BOOLEAN   WaitUntilBufferFull
  );

/**
  Polls a serial device to see if there is any data waiting to be read.

  If there is data waiting to be read from the serial device, then TRUE is returned.
  If there is no data waiting to be read from the serial device, then FALSE is returned.

  @param[in]  UartNumber       Selects Serial IO UART device (0-2)

  @retval TRUE                 Data is waiting to be read from the serial device.
  @retval FALSE                There is no data waiting to be read from the serial device.

**/
BOOLEAN
EFIAPI
PchSerialIoUartPoll (
  IN  UINT8     UartNumber
  );

#endif // _PEI_DXE_SMM_PCH_SERIAL_IO_UART_LIB_H_

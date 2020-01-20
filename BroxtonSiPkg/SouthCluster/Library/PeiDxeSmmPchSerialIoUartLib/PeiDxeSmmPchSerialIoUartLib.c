/** @file
  PCH Serial IO UART Lib implementation.
  All function in this library is available for PEI, DXE, and SMM,
  But do not support UEFI RUNTIME environment call.

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

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>
#include <Library/BaseLib.h>
#include <Library/TimerLib.h>
#include <ScAccess.h>
#include <Library/ScPcrLib.h>
#include <Library/ScSerialIoLib.h>
//[-start-160808-IB07400769-add]//
#include <Library/MmPciLib.h>
//[-end-160808-IB07400769-add]//

#define MAX_BAUD_RATE     115200

#define R_PCH_SERIAL_IO_8BIT_UART_RXBUF      0x00
#define R_PCH_SERIAL_IO_8BIT_UART_TXBUF      0x00
#define R_PCH_SERIAL_IO_8BIT_UART_BAUD_LOW   0x00
#define R_PCH_SERIAL_IO_8BIT_UART_BAUD_HIGH  0x01
#define R_PCH_SERIAL_IO_8BIT_UART_FCR        0x02
#define R_PCH_SERIAL_IO_8BIT_UART_IIR        0x02
#define R_PCH_SERIAL_IO_8BIT_UART_LCR        0x03
#define R_PCH_SERIAL_IO_8BIT_UART_MCR        0x04
#define R_PCH_SERIAL_IO_8BIT_UART_LSR        0x05
#define R_PCH_SERIAL_IO_8BIT_UART_USR        0x1F

#define R_PCH_SERIAL_IO_NATIVE_UART_RXBUF      0x00
#define R_PCH_SERIAL_IO_NATIVE_UART_TXBUF      0x00
#define R_PCH_SERIAL_IO_NATIVE_UART_BAUD_LOW   0x00
#define R_PCH_SERIAL_IO_NATIVE_UART_BAUD_HIGH  0x04
#define R_PCH_SERIAL_IO_NATIVE_UART_FCR        0x08
#define R_PCH_SERIAL_IO_NATIVE_UART_IIR        0x08
#define R_PCH_SERIAL_IO_NATIVE_UART_LCR        0x0C
#define R_PCH_SERIAL_IO_NATIVE_UART_MCR        0x10
#define R_PCH_SERIAL_IO_NATIVE_UART_LSR        0x14
#define R_PCH_SERIAL_IO_NATIVE_UART_USR        0x7C

#define B_PCH_SERIAL_IO_UART_IIR_FIFOSE   BIT7|BIT6
#define B_PCH_SERIAL_IO_UART_LSR_TXRDY    BIT5
#define B_PCH_SERIAL_IO_UART_LSR_RXDA     BIT0
#define B_PCH_SERIAL_IO_UART_LCR_DLAB     BIT7
#define B_PCH_SERIAL_IO_UART_FCR_FCR      BIT0
#define B_PCH_SERIAL_IO_UART_MCR_RTS      BIT1
#define B_PCH_SERIAL_IO_UART_MCR_AFCE     BIT5
#define B_PCH_SERIAL_IO_UART_USR_TFNF     BIT1

/**
  Initialize selected SerialIo UART.

  @param[in]  UartNumber           Selects Serial IO UART device (0-2)
  @param[in]  FifoEnable           When TRUE, enables 64-byte FIFOs.
  @param[in]  BaudRate             Baud rate.
  @param[in]  LineControl          Data length, parity, stop bits.
  @param[in]  HardwareFlowControl  Automated hardware flow control. If TRUE, hardware automatically checks CTS when sending data, and sets RTS when receiving data.
**/
VOID
SerialIo16550Init (
  IN UINTN      Base,
  IN BOOLEAN    FifoEnable,
  IN UINT32     BaudRate,
  IN UINT8      LineControl,
  IN BOOLEAN    HardwareFlowControl,
  IN UINT8      ShiftOffset
  )
{
  UINTN          Divisor;

  Divisor = MAX_BAUD_RATE / BaudRate;
  //
  // Configure baud rate
  //
  MmioWrite8 (Base + (R_PCH_SERIAL_IO_NATIVE_UART_LCR >> ShiftOffset), B_PCH_SERIAL_IO_UART_LCR_DLAB);
  MmioWrite8 (Base + (R_PCH_SERIAL_IO_NATIVE_UART_BAUD_HIGH >> ShiftOffset), (UINT8) (Divisor >> 8));
  MmioWrite8 (Base + (R_PCH_SERIAL_IO_NATIVE_UART_BAUD_LOW >> ShiftOffset), (UINT8) (Divisor & 0xff));
  //
  //  Configure Line control and switch back to bank 0
  //
  MmioWrite8 (Base + (R_PCH_SERIAL_IO_NATIVE_UART_LCR >> ShiftOffset), LineControl & 0x1F);
  //
  // Enable and reset FIFOs
  //
  MmioWrite8 (Base + (R_PCH_SERIAL_IO_NATIVE_UART_FCR >> ShiftOffset), FifoEnable ? B_PCH_SERIAL_IO_UART_FCR_FCR : 0);
  //
  // Put Modem Control Register(MCR) into its reset state of 0x00.
  //
  MmioWrite8 (Base + (R_PCH_SERIAL_IO_NATIVE_UART_MCR >> ShiftOffset), B_PCH_SERIAL_IO_UART_MCR_RTS | (HardwareFlowControl ? B_PCH_SERIAL_IO_UART_MCR_AFCE : 0) );

  return;
}


/**
  Initialize selected SerialIo UART.
  This init function MUST be used prior any SerialIo UART functions to init serial io controller
  if platform is going use serialio UART as debug output.

  @param[in]  UartNumber           Selects Serial IO UART device (0-2)
  @param[in]  FifoEnable           When TRUE, enables 64-byte FIFOs.
  @param[in]  BaudRate             Baud rate.
  @param[in]  LineControl          Data length, parity, stop bits.
  @param[in]  HardwareFlowControl  Automated hardware flow control. If TRUE, hardware automatically checks CTS when sending data,
                                   and sets RTS when receiving data.
**/
VOID
EFIAPI
PchSerialIoUartInit (
  IN UINT8      UartNumber,
  IN BOOLEAN    FifoEnable,
  IN UINT32     BaudRate,
  IN UINT8      LineControl,
  IN BOOLEAN    HardwareFlowControl
  )
{
  UINT32 UartMode    = 0;
  UINTN  Bar         = 0;
  UINT8  ShiftOffset = 0;

  if (UartNumber > 3) {
    return;  // In case of invalid UART device
  }

#ifdef PCH_PO_FLAG
  ConfigureSerialIoController(UartNumber + PchSerialIoIndexUart0, PchSerialIoPci, 0);
#else
  ConfigureSerialIoController(UartNumber + PchSerialIoIndexUart0, PchSerialIoPci);
#endif

  // Find UART Mode (Checking for 16550 Mode)
  PchPcrRead32(LPSS_EP_PORT_ID, R_LPSS_SB_GPPRVRW7, &UartMode);

  if (UartMode == B_LPSS_UART2_BYTE_ADDR_EN) {
    ShiftOffset = 2;
  }

  Bar = FindSerialIoBar(UartNumber + PchSerialIoIndexUart0, 0);

  if (Bar != 0xFFFFFFFF) {
    SerialIo16550Init(Bar, FifoEnable, BaudRate, LineControl, HardwareFlowControl, ShiftOffset);
  }
}

//[-start-160808-IB07400769-add]//
VOID
CheckSerialIoD3State (
  IN PCH_SERIAL_IO_CONTROLLER SerialIoDevice
  )
{
  UINTN  PcieBase;
  UINT32 VenId;
  UINT8  PowerState;

  PcieBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, GetSerialIoDeviceNumber (SerialIoDevice), GetSerialIoFunctionNumber (SerialIoDevice));
  VenId = MmioRead32 (PcieBase + PCI_VENDOR_ID_OFFSET) & 0xFFFF;
  PowerState = MmioRead32 (PcieBase + R_LPSS_IO_PCS) & B_LPSS_IO_PCS_PS;
    
  if ((VenId == V_INTEL_VENDOR_ID) && (PowerState)) {
    //
    // Put to D0 state
    //
    MmioAnd8 (PcieBase + R_LPSS_IO_PCS, (UINT8)~B_LPSS_IO_PCS_PS); 
    //
    // Re-init UART interface
    //
    PchSerialIoUartInit ((UINT8)(SerialIoDevice - PchSerialIoIndexUart0),TRUE,115200,3,FALSE);
  }
}
//[-end-160808-IB07400769-add]//


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
  )
{
  UINTN BytesLeft;
  volatile UINTN Base;
  UINT32 UartMode = 0;
  UINT8  ShiftOffset = 0;
  UINT32 TxAttemptCount = 0;

  if (UartNumber > 3) {
    return 0;  // In case of invalid UART device
  }
//[-start-160629-IB07400750-add]//
//[-start-160819-IB07400772-modify]//
#ifndef H2ODDT_HSUART_SUPPORT
//[-end-160819-IB07400772-modify]//
  SaveRestoreSerialIoPci (UartNumber + PchSerialIoIndexUart0, TRUE);
#endif
//[-end-160629-IB07400750-add]//
//[-start-160808-IB07400769-add]//
  CheckSerialIoD3State (UartNumber + PchSerialIoIndexUart0);
//[-end-160808-IB07400769-add]//

  Base = FindSerialIoBar (UartNumber + PchSerialIoIndexUart0, 0);

  //
  // Sanity checks to avoid infinite loop when trying to print through uninitialized UART
  //
  // If BAR is unavailable, write 0 bytes to the device
  //
  if (Base == 0xFFFFFFFF ||
      (Base & 0xFFFFFF00) == 0x0 ||
      MmioRead8 (Base + (R_PCH_SERIAL_IO_NATIVE_UART_USR >> ShiftOffset)) == 0xFF ||
      Buffer == NULL) {
    return 0;
  }

  PchPcrRead32(LPSS_EP_PORT_ID, R_LPSS_SB_GPPRVRW7, &UartMode);

  if (UartMode == B_LPSS_UART2_BYTE_ADDR_EN)
  {
    ShiftOffset = 2;
  }

  BytesLeft = NumberOfBytes;

  while (BytesLeft != 0 && TxAttemptCount < 200) {
    //
    // Write data while there's room in TXFIFO. If HW Flow Control was enabled, it happens automatically on hardware level.
    //
    while ((MmioRead8(Base + (R_PCH_SERIAL_IO_NATIVE_UART_IIR >> ShiftOffset)) & 0xF) == 0x10);

    if (MmioRead8 (Base + (R_PCH_SERIAL_IO_NATIVE_UART_USR >> ShiftOffset)) & B_PCH_SERIAL_IO_UART_USR_TFNF) {
      MmioWrite8 (Base + R_PCH_SERIAL_IO_NATIVE_UART_TXBUF, *Buffer);

      TxAttemptCount = 0;
      Buffer++;
      BytesLeft--;
    }
    TxAttemptCount++;
  }
  
//[-start-160629-IB07400750-add]//
//[-start-160819-IB07400772-modify]//
#ifndef H2ODDT_HSUART_SUPPORT
//[-end-160819-IB07400772-modify]//
  SaveRestoreSerialIoPci (UartNumber + PchSerialIoIndexUart0, FALSE);
#endif
//[-end-160629-IB07400750-add]//

  return NumberOfBytes;
}

/**
  Read data from serial device and save the datas in buffer.

  If the buffer is NULL, then return 0;
  if NumberOfBytes is zero, then return 0.

  @param[in]  UartNumber           Selects Serial IO UART device (0-2)
  @param[out] Buffer               Point of data buffer which need to be writed.
  @param[in]  NumberOfBytes        Number of output bytes which are cached in Buffer.
  @param[in]  WaitUntilBufferFull  When TRUE, function waits until whole buffer is filled. When FALSE,
                                   function returns as soon as no new characters are available.

  @retval UINTN                    Actual number of bytes read to the serial device.
**/
UINTN
EFIAPI
PchSerialIoUartIn (
  IN  UINT8     UartNumber,
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes,
  IN  BOOLEAN   WaitUntilBufferFull
  )
{
  UINTN BytesReceived;
  UINTN Base;
  UINT32 UartMode = 0;
  UINT8  ShiftOffset = 0;

  if (UartNumber > 3) {
    return 0;  // In case of invalid UART device
  }

  Base = FindSerialIoBar (UartNumber + PchSerialIoIndexUart0, 0);

  // If BAR is unavailable, receive 0 bytes
  if (Base == 0xFFFFFFFF) {
    return 0;
  }

  PchPcrRead32(LPSS_EP_PORT_ID, R_LPSS_SB_GPPRVRW7, &UartMode);
  if(UartMode == 4)
  {
    ShiftOffset = 2;
  }

  if (NULL == Buffer) {
    return 0;
  }

  BytesReceived = 0;

  while (BytesReceived != NumberOfBytes) {
    //
    // check if there's data in RX buffer
    //
    if (MmioRead8 (Base + (R_PCH_SERIAL_IO_NATIVE_UART_LSR >> ShiftOffset)) & B_PCH_SERIAL_IO_UART_LSR_RXDA) {
      //
      // Receive data
      //
      *Buffer = MmioRead8 (Base + R_PCH_SERIAL_IO_NATIVE_UART_RXBUF);
      Buffer++;
      BytesReceived++;
    } else {
      if (!WaitUntilBufferFull && ((MmioRead8(Base + (R_PCH_SERIAL_IO_NATIVE_UART_LSR >> ShiftOffset)) & BIT0) == 0)) {
        //
        // If there's no data and function shouldn't wait, exit early
        //
        return BytesReceived;
      }
    }
  }
  return BytesReceived;
}

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
  )
{
  UINTN Base;

  if (UartNumber > 3) {
    return FALSE;  // In case of invalid UART device
  }

  Base = FindSerialIoBar (UartNumber + PchSerialIoIndexUart0, 0);
  //
  // Read the serial port status
  //
  if ((MmioRead8 (Base + R_PCH_SERIAL_IO_NATIVE_UART_LSR) & B_PCH_SERIAL_IO_UART_LSR_RXDA) != 0) {
    return TRUE;
  }
  return FALSE;
}

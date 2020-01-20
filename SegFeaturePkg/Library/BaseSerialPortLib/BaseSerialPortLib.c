/** @file
  Serial I/O Port library functions with no library constructor/
  destructor

;******************************************************************************
;* Copyright (c) 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
@copyright
  Copyright (c) 2012 - 2015 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:

**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/SerialPortLib.h>
#include <Library/PlatformHookLib.h>

//
// UART register offsets and bitfields
//
#define RXBUF_OFFSET      0x00
#define TXBUF_OFFSET      0x00
#define BAUD_LOW_OFFSET   0x00
#define BAUD_HIGH_OFFSET  0x01
#define IER_OFFSET        0x01
#define LCR_SHADOW_OFFSET 0x01
#define FCR_SHADOW_OFFSET 0x02
#define IR_CONTROL_OFFSET 0x02
#define FCR_OFFSET        0x02
#define IIR_OFFSET        0x02
#define EIR_OFFSET        0x02
#define BSR_OFFSET        0x03
#define LCR_OFFSET        0x03
#define MCR_OFFSET        0x04
#define LSR_OFFSET        0x05
#define MSR_OFFSET        0x06

//
// UART Register Bit Defines
//
#define FCR_FIFOE    BIT0
#define FCR_FIFO64   BIT5
#define IIR_FIFO64   BIT5
#define IIR_FIFOE_L  BIT6
#define IIR_FIFOE_H  BIT7
#define LSR_TXRDY    0x20
#define LSR_RXDA     0x01
#define DLAB         BIT7
#define MCR_RTS      BIT1
#define MSR_CTS      BIT4
#define MSR_DSR      BIT5

#define MAX_BAUD_RATE     115200

//
// PcdH2OSerialPortConfig structure
//
#pragma pack (1)
typedef struct {
  UINT64  RegisterBase;
  BOOLEAN UseIoSpace;
  UINT32  BaudRate;
  UINT8   LineControl;
  UINT8   FifoControl;
  BOOLEAN DetectCable;
  BOOLEAN UseHardwareFlowControl;
} SERIAL_PORT_CONFIGURATION;
#pragma pack ()

/**
  Read one byte from a Serial Port Register.

  @param[in] SerialConfig       The serial port information.
  @param[in] Offset             The offset to read.

  @retval                       The value read.

**/
UINT8
SerialPortReadRegister (
  IN SERIAL_PORT_CONFIGURATION *SerialConfig,
  IN UINTN                     Offset
  )
{
  if (SerialConfig->UseIoSpace) {
    return IoRead8 ((UINTN)SerialConfig->RegisterBase + Offset);
  } else {
    return MmioRead8 ((UINTN)SerialConfig->RegisterBase + Offset);
  }
}

/**
  Write one byte to a Serial Port Register.

  @param[in] SerialConfig       The serial port information.
  @param[in] Offset             The offset to write.
  @param[in] Value              The value to write.

  @retval                       The value written.

**/
UINT8
SerialPortWriteRegister (
  IN SERIAL_PORT_CONFIGURATION *SerialConfig,
  IN UINTN                     Offset,
  IN UINT8                     Value
  )
{
  if (SerialConfig->UseIoSpace) {
    return IoWrite8 ((UINTN)SerialConfig->RegisterBase + Offset, Value);
  } else {
    return MmioWrite8 ((UINTN)SerialConfig->RegisterBase + Offset, Value);
  } 
}

/**
  Initialize a specific Serial Port.

  If no initialization is required, then return RETURN_SUCCESS.
  If the serial device was successfully initialized, then return RETURN_SUCCESS.

  @param[in] SerialConfig       The serial port information.

  @retval RETURN_SUCCESS        The serial device was initialized.

**/
RETURN_STATUS
EFIAPI
UARTInitialize (
  IN SERIAL_PORT_CONFIGURATION *SerialConfig
  )
{
  UINTN          Divisor;
  UINTN          TempDivisor;
  BOOLEAN        Initialized;
  UINT8          OldLcrData;

  Divisor = MAX_BAUD_RATE / SerialConfig->BaudRate;

  //
  // See if the serial port is already initialized
  //
  Initialized = TRUE;

  if ((SerialConfig->FifoControl & FCR_FIFOE) != 0) {
    if ((SerialPortReadRegister (SerialConfig, IIR_OFFSET) & (IIR_FIFOE_H | IIR_FIFOE_L)) !=
        (IIR_FIFOE_H | IIR_FIFOE_L)) {
      Initialized = FALSE;
    }  
  } else {
    if ((SerialPortReadRegister (SerialConfig, IIR_OFFSET) & (IIR_FIFOE_H | IIR_FIFOE_L)) == 
        (IIR_FIFOE_H | IIR_FIFOE_L)) {
      Initialized = FALSE;
    }
  }
  if ((SerialPortReadRegister (SerialConfig, IIR_OFFSET) & IIR_FIFO64) != 
      (SerialConfig->FifoControl & FCR_FIFO64)) {
    Initialized = FALSE;
  }  
  if ((SerialPortReadRegister (SerialConfig, LCR_OFFSET) & 0x3F) != 
      (SerialConfig->LineControl & 0x3F)) {
    Initialized = FALSE;
  }
  OldLcrData = SerialPortReadRegister (SerialConfig, LCR_OFFSET);
  SerialPortWriteRegister (SerialConfig, LCR_OFFSET, (UINT8)(OldLcrData | DLAB));
  TempDivisor = (UINTN)SerialPortReadRegister (SerialConfig, BAUD_HIGH_OFFSET);
  TempDivisor = TempDivisor << 8;
  TempDivisor |= (UINTN)SerialPortReadRegister (SerialConfig, BAUD_LOW_OFFSET);
  SerialPortWriteRegister (SerialConfig, LCR_OFFSET, (UINT8)(OldLcrData & ~DLAB));
  if (TempDivisor != Divisor) {
    Initialized = FALSE;
  }
  if (Initialized) {
    return RETURN_SUCCESS;
  }

  //
  // Set communications format
  //
  SerialPortWriteRegister (SerialConfig, LCR_OFFSET, DLAB);

  //
  // Configure baud rate
  //
  SerialPortWriteRegister (SerialConfig, BAUD_HIGH_OFFSET, (UINT8) (Divisor >> 8));
  SerialPortWriteRegister (SerialConfig, BAUD_LOW_OFFSET, (UINT8) (Divisor & 0xff));

  //
  // Switch back to bank 0
  //
  SerialPortWriteRegister (SerialConfig, LCR_OFFSET, SerialConfig->LineControl & 0x3F);

  //
  // Enable and reset FIFOs
  // Strip reserved bits from PcdSerialFifoControl
  //
  SerialPortWriteRegister (SerialConfig, FCR_OFFSET, SerialConfig->FifoControl & 0x27);

  //
  // Put Modem Control Register(MCR) into its reset state of 0x00.
  //
  SerialPortWriteRegister (SerialConfig, MCR_OFFSET, 0x00);
  
  return RETURN_SUCCESS;
}

/**
  Initialize for BaseSerialLib library instance.

  @retval RETURN_SUCCESS        The BaseSerialLib was initialized.

**/
RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{
  RETURN_STATUS             Status;
  SERIAL_PORT_CONFIGURATION *PcdSerialConfig;

  PcdSerialConfig = NULL;

  //
  // Some init is done by the platform status code initialization.
  //
  Status = PlatformHookSerialPortInitialize();
  if (RETURN_ERROR (Status)) {
    return Status;
  }

  //
  // Initialize all serial ports that are specified by PcdH2OSerialPortConfig
  //
  PcdSerialConfig = (SERIAL_PORT_CONFIGURATION*) PcdGetPtr (PcdH2OSerialPortConfig);
  while ((PcdSerialConfig != NULL) && (PcdSerialConfig->RegisterBase != 0)) {
    UARTInitialize (PcdSerialConfig);
    PcdSerialConfig++;
  }
    
  return RETURN_SUCCESS;
}

/**
  Write data to a specific serial port.

  If the buffer is NULL, then return 0;
  if NumberOfBytes is zero, then return 0.

  @param[in] SerialConfig       The serial port information.
  @param[in] Buffer             Point of data buffer which need to be writed.
  @param[in] NumberOfBytes      Number of output bytes which are cached in Buffer.

  @retval 0                     Write data failed.
  @retval !0                    Actual number of bytes writed to serial port.

**/
UINTN
EFIAPI
UARTDbgOut (
  IN SERIAL_PORT_CONFIGURATION *SerialConfig,
  IN UINT8                     *Buffer,
  IN UINTN                     NumberOfBytes
)
{
  UINTN Result;
  UINTN FifoSize;
  UINTN Index;

  if (NULL == Buffer) {
    return 0;
  }

  //
  // Check the serial port settings
  //
  UARTInitialize (SerialConfig);
  
  //
  // Compute the maximum size of the Tx FIFO
  //
  FifoSize = 1;
  if ((SerialConfig->FifoControl & FCR_FIFOE) != 0) {
    if ((SerialConfig->FifoControl & FCR_FIFO64) == 0) {
      FifoSize = 16;
    } else {
      FifoSize = 64;
    }
  }

  Result = NumberOfBytes;

  while (NumberOfBytes != 0) {
    //
    // Wait for the serial port to be ready, to make sure both the transmit FIFO
    // and shift register empty.
    //
    while ((SerialPortReadRegister (SerialConfig, LSR_OFFSET) & LSR_TXRDY) == 0);

    //
    // Fill then entire Tx FIFO
    //
    for (Index = 0; Index < FifoSize && NumberOfBytes != 0; Index++, NumberOfBytes--, Buffer++) {
      if (SerialConfig->UseHardwareFlowControl) {
        if (SerialConfig->DetectCable) {
          //
          // Wait for both DSR and CTS to be set
          //   DSR is set if a cable is connected.
          //   CTS is set if it is ok to transmit data
          //
          //   DSR  CTS  Description                               Action
          //   ===  ===  ========================================  ========
          //    0    0   No cable connected.                       Wait
          //    0    1   No cable connected.                       Wait
          //    1    0   Cable connected, but not clear to send.   Wait
          //    1    1   Cable connected, and clear to send.       Transmit
          //
          while ((SerialPortReadRegister (SerialConfig, MSR_OFFSET) & (MSR_DSR | MSR_CTS)) != (MSR_DSR | MSR_CTS));
        } else {
          //
          // Wait for both DSR and CTS to be set OR for DSR to be clear.
          //   DSR is set if a cable is connected.
          //   CTS is set if it is ok to transmit data
          //
          //   DSR  CTS  Description                               Action
          //   ===  ===  ========================================  ========
          //    0    0   No cable connected.                       Transmit
          //    0    1   No cable connected.                       Transmit
          //    1    0   Cable connected, but not clear to send.   Wait
          //    1    1   Cable connected, and clar to send.        Transmit
          //
          while ((SerialPortReadRegister (SerialConfig, MSR_OFFSET) & (MSR_DSR | MSR_CTS)) == (MSR_DSR));
        }
      }

      //
      // Write byte to the transmit buffer.
      //
      SerialPortWriteRegister (SerialConfig, TXBUF_OFFSET, *Buffer);
    }
  }

  return Result;
}

/**
  Write data from buffer to multiple serial ports.

  @param[in] Buffer           Point of data buffer which need to be writed.
  @param[in] NumberOfBytes    Number of output bytes which are cached in Buffer.

**/
UINTN
EFIAPI
SerialPortWrite (
  IN UINT8     *Buffer,
  IN UINTN     NumberOfBytes
)
{
  SERIAL_PORT_CONFIGURATION *PcdSerialConfig;

  PcdSerialConfig = NULL;

  //
  // Write data to all serial ports that are specified by PcdH2OSerialPortConfig
  //
  PcdSerialConfig = (SERIAL_PORT_CONFIGURATION*) PcdGetPtr (PcdH2OSerialPortConfig);
  while ((PcdSerialConfig != NULL) && (PcdSerialConfig->RegisterBase != 0)) {
    UARTDbgOut (PcdSerialConfig, Buffer, NumberOfBytes);
    PcdSerialConfig++;
  }

  return RETURN_SUCCESS;
}

/*
  Read data from a specific serial port and save the datas in buffer.

  If the buffer is NULL, then return 0;
  if NumberOfBytes is zero, then return 0.

  @param[in]  SerialConfig       The serial port information.
  @param[out] Buffer             Point of data buffer which need to be writed.
  @param[in]  NumberOfBytes      Number of output bytes which are cached in Buffer.

  @retval 0                      Read data failed.
  @retval !0                     Actual number of bytes raed to serial port.

**/
UINTN
EFIAPI
UARTDbgIn (
  IN SERIAL_PORT_CONFIGURATION *SerialConfig,  
  OUT UINT8                    *Buffer,
  IN  UINTN                    NumberOfBytes
)
{
  UINTN Result;
  UINT8 Mcr;

  if (NULL == Buffer) {
    return 0;
  }

  Result = NumberOfBytes;

  Mcr = (UINT8)(SerialPortReadRegister (SerialConfig, MCR_OFFSET) & ~MCR_RTS);

  for (Result = 0; NumberOfBytes-- != 0; Result++, Buffer++) {
    //
    // Wait for the serial port to have some data.
    //
    while ((SerialPortReadRegister (SerialConfig, LSR_OFFSET) & LSR_RXDA) == 0) {
      if (SerialConfig->UseHardwareFlowControl) {
        //
        // Set RTS to let the peer send some data
        //
        SerialPortWriteRegister (SerialConfig, MCR_OFFSET, (UINT8)(Mcr | MCR_RTS));
      }
    }

    if (SerialConfig->UseHardwareFlowControl) {
      //
      // Clear RTS to prevent peer from sending data
      //
      SerialPortWriteRegister (SerialConfig, MCR_OFFSET, Mcr);
    }

    //
    // Read byte from the receive buffer.
    //
    *Buffer = SerialPortReadRegister (SerialConfig, RXBUF_OFFSET);
  }

  return Result;
}

/*
  Read data from the first serial port into a buffer.
  
  @param[out] Buffer           Point of data buffer which need to be writed.
  @param[in]  NumberOfBytes    Number of output bytes which are cached in Buffer.

**/
UINTN
EFIAPI
SerialPortRead (
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes
)
{
  SERIAL_PORT_CONFIGURATION *PcdSerialConfig;

  PcdSerialConfig = NULL;

  //
  // Read data from the first serial port that are specified by PcdH2OSerialPortConfig
  //
  PcdSerialConfig = (SERIAL_PORT_CONFIGURATION*) PcdGetPtr (PcdH2OSerialPortConfig);
  if ((PcdSerialConfig != NULL) && (PcdSerialConfig->RegisterBase != 0)) {
    UARTDbgIn (PcdSerialConfig, Buffer, NumberOfBytes);
  }
    
  return RETURN_SUCCESS;
}

/**
  Poll a specific serial port to see if there is any data waiting to be read.

  Poll a specific serial port to see if there is any data waiting to be read.
  If there is data waiting to be read from the serial port, then TRUE is returned.
  If there is no data waiting to be read from the serial port, then FALSE is returned.

  @param[in] SerialConfig       The serial port information.

  @retval TRUE                  Data is waiting to be read from the serial port.
  @retval FALSE                 There is no data waiting to be read from the serial port.

**/
BOOLEAN
EFIAPI
UARTDbgPoll (
  IN SERIAL_PORT_CONFIGURATION *SerialConfig
  )
{
  //
  // Read the serial port status
  //
  if ((SerialPortReadRegister (SerialConfig, LSR_OFFSET) & LSR_RXDA) != 0) {
    if (SerialConfig->UseHardwareFlowControl) {
      //
      // Clear RTS to prevent peer from sending data
      //
      SerialPortWriteRegister (SerialConfig, MCR_OFFSET, (UINT8)(SerialPortReadRegister (SerialConfig, MCR_OFFSET) & ~MCR_RTS));
    }
    return TRUE;
  }

  if (SerialConfig->UseHardwareFlowControl) {
    //
    // Set RTS to let the peer send some data
    //
    SerialPortWriteRegister (SerialConfig, MCR_OFFSET, (UINT8)(SerialPortReadRegister (SerialConfig, MCR_OFFSET) | MCR_RTS));
  }
  return FALSE;
}

/**
  Poll the first serial port to see if there is any data waiting to be read.

  Poll the first serial port to see if there is any data waiting to be read.
  If there is data waiting to be read from the serial port, then TRUE is returned.
  If there is no data waiting to be read from the serial port, then FALSE is returned.

  @retval TRUE             Data is waiting to be read from the serial port.
  @retval FALSE            There is no data waiting to be read from the serial port.

**/
BOOLEAN
EFIAPI
SerialPortPoll (
  VOID
  )
{
  BOOLEAN                   Status;
  SERIAL_PORT_CONFIGURATION *PcdSerialConfig;

  PcdSerialConfig = NULL;

  Status = FALSE;

  //
  // Poll the first serial port that are specified by PcdH2OSerialPortConfig
  // 
  PcdSerialConfig = (SERIAL_PORT_CONFIGURATION*) PcdGetPtr (PcdH2OSerialPortConfig);
  if ((PcdSerialConfig != NULL) && (PcdSerialConfig->RegisterBase != 0)) {
    Status |= UARTDbgPoll (PcdSerialConfig);
  }

  return Status;
}


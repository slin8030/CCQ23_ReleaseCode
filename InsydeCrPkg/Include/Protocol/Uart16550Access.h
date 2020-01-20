/** @file
  Protocol used for Terminal Policy definition.

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _UART_16550_ACCESS_PROTOCOL_H_
#define _UART_16550_ACCESS_PROTOCOL_H_

#include <Uefi.h>
#include <CrDeviceType.h>

//
// 16550 Compatible Device Path Node. This node is for non-pci controller device only.
//
#define UART_16550_COMPATIBLE_HW_GUID \
  { \
    0xe76fd4e9, 0x0a30, 0x4ca9, {0x95, 0x40, 0xd7, 0x99, 0x53, 0x4c, 0xc4, 0xff} \
  }

extern EFI_GUID gUart16550CompatibleHwGuid;
//
// 16550 UART register offsets and bitfields
//
#define UART_REG_RBR        0
#define UART_REG_THR        0
#define UART_REG_IER        1
#define UART_REG_IIR        2
#define UART_REG_FCR        2
#define UART_REG_LCR        3
#define UART_REG_MCR        4
#define UART_REG_LSR        5
#define UART_REG_MSR        6
#define UART_REG_SCR        7
#define UART_REG_DLL        0
#define UART_REG_DLH        1

#define UART_LSR_DR       BIT0
#define UART_LSR_THRE     BIT5

#define UART_MCR_DTR      BIT0
#define UART_MCR_RTS      BIT1

#define UART_MSR_CTS      BIT4

#define CREATE_UID(B,D,F)  ((UINT16)(B<<8 | D<<3 | F))
#define GET_BUS(U)         ((UINT8)(U>>8))
#define GET_DEV(U)         ((UINT8)((U & 0x0FF) >> 3))
#define GET_FUN(U)         ((UINT8)(U & 0x07))

#define H2O_UART_1600A_ACCESS_PROTOCOL_GUID \
  { \
    0x76bce257, 0xb552, 0x415a, 0xa3, 0x2e, 0xb0, 0xe7, 0x1c, 0xf8, 0x59, 0xd0 \
  }

typedef struct _H2O_UART_16550_ACCESS_PROTOCOL    H2O_UART_16550_ACCESS_PROTOCOL;

enum UART_BASE_ADDRESS_TYPE {
  UBAT_IO,
  UBAT_MEMORY,
  UBAT_PCI_CFG,
  UBAT_BAR_INDEX
};

typedef struct {
  UINT8 Bus;
  UINT8 Dev;
  UINT8 Func;
} PCI_DEVICE_PFA;

typedef struct _UART_16550_DEV_INFO {
  CHAR16          *DeviceName;
  UINT8           DeviceType;
  UINT16          UID;
  UINT8           RegisterByteWidth;
  UINT8           BaseAddressType;
  UINTN           BaseAddress;
  UINT8           DevIRQ;
  UINT32          FifoSize;
  UINTN           SerialClockFreq;
  UINT16          SampleRate;
  BOOLEAN         LegacySupport;
  UINT8           UartUid;
} UART_16550_DEVICE_INFO;

typedef
EFI_STATUS
(EFIAPI *H2O_REGISTER_READ) (
  H2O_UART_16550_ACCESS_PROTOCOL  *This,
  UINT16                          Index,
  UINT8                           *Data
  );

typedef
EFI_STATUS
(EFIAPI *H2O_REGISTER_WRITE) (
  H2O_UART_16550_ACCESS_PROTOCOL  *This,
  UINT16                          Index,
  UINT8                           Data
  );

struct _H2O_UART_16550_ACCESS_PROTOCOL {
  H2O_REGISTER_READ       RegRead;
  H2O_REGISTER_WRITE      RegWrite;
  UART_16550_DEVICE_INFO  *DeviceInfo;
};

extern EFI_GUID gH2OUart16550AccessProtocolGuid;

#endif // _UART_16550_ACCESS_PROTOCOL_H_


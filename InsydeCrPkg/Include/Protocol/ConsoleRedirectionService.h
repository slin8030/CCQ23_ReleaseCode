/** @file
  ConsoleRedirectionService Protocol Definitions

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CONSOLE_REDIRECTION_SERVICE_H_
#define _CONSOLE_REDIRECTION_SERVICE_H_

#include <CrDeviceType.h>
#include <Protocol/Uart16550Access.h>

#define CONSOLE_REDIRECTION_SERVICE_PROTOCOL_GUID \
  { \
    0x60f904c4, 0x6ff, 0x4004, 0x8e, 0x2b, 0x95, 0x86, 0x90, 0xe3, 0xcc, 0x97 \
  }

#define MAX_CR_DEVICE_VARIABLE  10

#pragma pack(1)
typedef struct {
  UINT8                     PortEnable;
  UINT8                     UseGlobalSetting;
  UINT8                     TerminalType;
  UINT8                     BaudRate;
  UINT8                     Parity;
  UINT8                     DataBits;
  UINT8                     StopBits;
  UINT8                     FlowControl;
} CR_DEVICE_ATTRIBUTES;

typedef struct {
  CR_DEVICE_TYPE                  Type;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  UINT8                           BaudRateDivisor;
  H2O_UART_16550_ACCESS_PROTOCOL  *Uart16550protocol;
  INT32                           TextMode;
} EFI_CONSOLE_REDIRECTION_DEVICE;

typedef struct _EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL;

typedef struct _EFI_CONSOLE_REDIRECTION_INFO {
    BOOLEAN                           Headless;
    UINT8                             DeviceCount;
    EFI_CONSOLE_REDIRECTION_DEVICE    CRDevice[0];
} EFI_CONSOLE_REDIRECTION_INFO;

#pragma pack()

typedef
EFI_STATUS
(EFIAPI *QUERY_VGA_SCREEN_BUFFER) (
  IN      EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL    *This,
  IN OUT  UINTN                                       *ScreenBuffer,
  IN OUT  UINTN                                       *ScreenAttributes
  );

typedef
EFI_STATUS
(EFIAPI *CONNECT_CRS_TERMINAL) (
  IN EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL  *This,
  IN UINTN                                     TerminalNumber
  );

typedef
EFI_STATUS
(EFIAPI *DISCONNECT_CRS_TERMINAL) (
  IN EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL  *This,
  IN UINTN                                     TerminalNumber
  );

struct _EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL{
  EFI_CONSOLE_REDIRECTION_INFO  *CRInfo;
  QUERY_VGA_SCREEN_BUFFER       QueryVgaScreenBuffer;
  DISCONNECT_CRS_TERMINAL       DisconnectCrsTerminal;
  CONNECT_CRS_TERMINAL          ConnectCrsTerminal;
};



extern EFI_GUID gConsoleRedirectionServiceProtocolGuid;

#endif

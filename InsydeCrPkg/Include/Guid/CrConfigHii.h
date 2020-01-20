/** @file

   The definition of Cr Config HII.

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CONSOLE_REDIRECTION_CONFIG_MANAGER_HII_H_
#define _CONSOLE_REDIRECTION_CONFIG_MANAGER_HII_H_

#define CR_CONFIGURATION_GUID  \
  { \
    0x7EC07B9F, 0x66E3, 0x43d4, {0x9B, 0x52, 0x38, 0xFB, 0xB4, 0x63, 0x90, 0xCC} \
  }

#define CONSOLE_REDIRECTION_FORMSET_GUID \
  { \
    0x121fa146, 0xcdc1, 0x422f, {0xb9, 0xd9, 0x8c, 0xac, 0xb9, 0x78, 0xd5, 0xcf} \
  }

//
// class guid just the same with Advance formset guid
//
#define CONSOLE_REDIRECTION_FORMSET_CLASS_GUID \
  { \
    0x63a6fd2f, 0xa251, 0x443c, {0xa6, 0x8d, 0xdf, 0xd4, 0x39, 0x63, 0xf5, 0xa5} \
  }

#define CONSOLE_REDIRECTION_VARSTORE_NAME           L"CrConfig"
#define CONSOLE_REDIRECTION_VARSTORE_DEFAULT_NAME   L"CrConfigDefault"

#pragma pack(1)

typedef struct {
  UINT8         PortEnable;
  UINT8         UseGlobalSetting;
  UINT8         TerminalType;
  UINT8         BaudRate;
  UINT8         Parity;
  UINT8         DataBits;
  UINT8         StopBits;
  UINT8         FlowControl;
} CR_SERIAL_PORT_CONFIG;

typedef struct {
  UINT8                  CREnable;
  UINT8                  CRInfoWaitTime;
  UINT8                  CRAfterPost;
  UINT8                  CRTextModeResolution;

  UINT8                  GlobalTerminalType;
  UINT8                  GlobalBaudRate;
  UINT8                  GlobalParity;
  UINT8                  GlobalDataBits;
  UINT8                  GlobalStopBits;
  UINT8                  GlobalFlowControl;
  UINT8                  AutoRefresh;
  UINT8                  FailSaveBaudRate;
  UINT8                  TerminalResize;
  CR_SERIAL_PORT_CONFIG  CrPortConfig;
} CR_CONFIGURATION;

#pragma pack()

extern EFI_GUID gCrConfigurationGuid;
extern EFI_GUID gConsoleRedirectionFormsetGuid;

#endif

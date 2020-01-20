/** @file
  CrLegacySupport Header

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

#ifndef _CR_LEGACY_SUPPORT_H_
#define _CR_LEGACY_SUPPORT_H_

#include "CrServiceDxe.h"
#include <CrDeviceType.h>

#define UART_THR_OFFSET           0
#define UART_MCR_OFFSET           4
#define PCI_CONFIG_COMMAND_WORD   4
#define UART_IER_OFFSET           0x01
#define PCI_CONFIG_BAR0           0x10

#define UART_MCR_OUT2_BIT               (1 << 3)
#define PCI_CONFIG_DISABLE_INTERRUPT    (1 << 10)
#define PCI_CONFIG_BUS_MASTER           (1 << 2)

#define SWITCH_TEXT_MODE                (TRUE)
#define SWITCH_ORIGINAL_MODE            (FALSE)

//
// Int 10/AH=00h   - VIDEO - SET VIDEO MODE
// Int 10/AX=4F02h - VESA SuperVGA BIOS - SET SuperVGA VIDEO MODE
// Int 10/AX=4F03h - VESA SuperVGA BIOS - GET CURRENT VIDEO MODE
//
#define VESA_GET_MODE             0x4F03
#define VESA_SET_MODE             0x4F02
#define VGA_SET_MODE              0x00
#define VGA_TEXT_MODE             0x03
#define VESA_SUPPORTED            0x4F
#define INT_10                    0x10
#define CR_ROM_HEADER_SIGNATURE   SIGNATURE_32 ('$', 'C', 'R', 'S')
#define IS_CRROM(Addr)(((OPROM_HEADER*)(*Addr))->CRSSignature == CR_ROM_HEADER_SIGNATURE)

//UART
#define UART_DATA_BITS_8      (BIT0 | BIT1)
#define UART_DATA_BITS_7      (BIT1)
#define UART_STOP_BITS_1      0
#define UART_STOP_BITS_2      (BIT2)
#define UART_PARITY_NONE      0
#define UART_PARITY_EVEN      (BIT3 | BIT4)
#define UART_PARITY_ODD       (BIT3)

//
// VGA buffer for text mode
//
#define SEGMENT_SHIFT        4
#define TEXT_PAGE_0_START    0xb8000
#define TEXT_PAGE_SIZE       0x1000
#define CHAR_SPACE           0x20
#define COLOR_BG_BLACK       0
#define COLOR_FG_WHITE       0x07

//
// CR INFO : pass CR information to CR option ROM
//
#define CR_MEM_MANAGER      0xA4
#define CR_CRINFO           0xA6

#pragma pack (1)

typedef struct {
  UINT8                      TerminalType : 4;
  UINT8                      Reserve      : 1;
  UINT8                      DataType     : 3;
  UINT8                      Data;
  UINT16                     OffsetOfString;
} CR_EFI_ESC_SEQUENCE_CODE;

typedef struct {
  UINT16                     Count;
  CR_EFI_ESC_SEQUENCE_CODE   CrEscSequenceCode[1];
} CR_EFI_ESC_SEQUENCE_CODE_TABLE;

typedef struct {
  UINT16                     CommandStrOffset;
  UINT8                      CommandType;
  UINT16                     Command;
} CR_EFI_SPECIAL_COMMAND;

typedef struct {
  UINT16                     Count;
  CR_EFI_SPECIAL_COMMAND     CrSpecialCommand[1];
} CR_EFI_SPECIAL_COMMAND_TABLE;

typedef struct {
  UINT8                      Type;
  UINT8                      UartUid;     //for flow control
  UINT8                      FlowControl;
  CR_LEGACY_SUPPORT_DEVICE   Device;
  UINT8                      BaudRateDivisor;
  UINT32                     BaudRate;
} CR_DEVICE;

#define CR_INFO_REVISION    0x07

typedef struct {
  UINT16      Signature;
  UINT8       Revision;
  UINT8       HLength;
  UINT16      BaudRateDivisor;
  UINT8       Protocol;
  UINT8       FIFOLength;
  UINT8       TerminalType;
  UINT16      FeatureFlag;
  UINT8       ShowHelp;
  UINT8       InfoWaitTime;
  UINT8       CRAfterPost;
  UINT16      TerminalEscCodeOffset;
  UINT16      CrSpecialCommandTableOffset;
  UINT8       Headless;
  UINT8       CRHeadlessVBuffer;              // Which buffer be chose when headless
  UINT8       DeviceCount;
  UINT8       CRWriteCharInterval;
//   UINT16      CrEfiLegacyCommonAreaOffset;
  UINT16      SmiPort;
  UINT8       CrsSmi;
} CR_EFI_INFO;

typedef struct {
  UINT8   LoByte;
  UINT8   HiByte;
} LOHI;

typedef struct {
  UINT16  RomSignature;
  UINT8   RomSizeofSector;
  UINT8   JmpOpcode[2];
  UINT32  CRSSignature;
  LOHI    Version;
  UINT8   Reserverd[13];
  UINT16  PcirOffset;
} OPROM_HEADER;

#pragma pack ()

EFI_STATUS
CrOpRomSupport (
  VOID
  );

#endif


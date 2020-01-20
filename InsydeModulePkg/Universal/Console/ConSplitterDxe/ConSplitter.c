/** @file

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/**
  Console Splitter Driver. Any Handle that attatched console I/O protocols
  (Console In device, Console Out device, Console Error device, Simple Pointer
  protocol, Absolute Pointer protocol) can be bound by this driver.

  So far it works like any other driver by opening a SimpleTextIn and/or
  SimpleTextOut protocol with EFI_OPEN_PROTOCOL_BY_DRIVER attributes. The big
  difference is this driver does not layer a protocol on the passed in
  handle, or construct a child handle like a standard device or bus driver.
  This driver produces three virtual handles as children, one for console input
  splitter, one for console output splitter and one for error output splitter.
  These 3 virtual handles would be installed on gST.

  Each virtual handle, that supports the Console I/O protocol, will be produced
  in the driver entry point. The virtual handle are added on driver entry and
  never removed. Such design ensures sytem function well during none console
  device situation.

Copyright (c) 2006 - 2012, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "ConSplitter.h"

#define PAUSE_TIMER_INTERVAL                  1000

#define BDA(a)                    (*(UINT16*)((UINTN)0x400 +(a)))
#define EBDA(a)                   (*(UINT16*)(UINTN)(((*(UINT16*)(UINTN)0x40e) << 4) + (a)))
#define EXT_DATA_SEG_OFFSET                   0x0E
#define BDA_MEMORY_SIZE_OFFSET                0x13
#define EBDA_KEYBORD_SCAN_CODE                0x164
#define EBDA_DEFAULT_SIZE                     0x400

BOOLEAN                    mPauseActive     = FALSE;
EFI_EVENT                  mPauseEvent      = NULL;
BOOLEAN                    mSaveKbcScanCode = FALSE;
BOOLEAN                    mInNtEmulator    = FALSE;
EFI_GUID                   gEfiWinNtThunkProtocolGuid = { 0x58C518B1, 0x76F3, 0x11D4, { 0xBC, 0xEA, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81 }};
EFI_BDS_ENTRY              mOriginalBdsEntry = NULL;
//
// Efi key to IBM Key Codemapping table
// Format:<efi scan code>, <unicode without shift>, <unicode with shift>
//
STATIC
UINT8 KeyCodeConvertionTable[][7] = {//       (========IBM PC Scan Code========)
//EFI Scancode <EFIUniCode> <EFIUniCode(Shift)> Base     Shift    Alt      Ctrl
    SCAN_NULL,      'a',      'A',              0x1E,    0x1E,    0x1E,    0x1E,// 0x04
    SCAN_NULL,      'b',      'B',              0x30,    0x30,    0x30,    0x30,// 0x05
    SCAN_NULL,      'c',      'C',              0x2E,    0x2E,    0x2E,    0x2E,// 0x06
    SCAN_NULL,      'd',      'D',              0x20,    0x20,    0x20,    0x20,// 0x07
    SCAN_NULL,      'e',      'E',              0x12,    0x12,    0x12,    0x12,// 0x08
    SCAN_NULL,      'f',      'F',              0x21,    0x21,    0x21,    0x21,// 0x09
    SCAN_NULL,      'g',      'G',              0x22,    0x22,    0x22,    0x22,// 0x0A
    SCAN_NULL,      'h',      'H',              0x23,    0x23,    0x23,    0x23,// 0x0B
    SCAN_NULL,      'i',      'I',              0x17,    0x17,    0x17,    0x17,// 0x0C
    SCAN_NULL,      'j',      'J',              0x24,    0x24,    0x24,    0x24,// 0x0D
    SCAN_NULL,      'k',      'K',              0x25,    0x25,    0x25,    0x25,// 0x0E
    SCAN_NULL,      'l',      'L',              0x26,    0x26,    0x26,    0x26,// 0x0F
    SCAN_NULL,      'm',      'M',              0x32,    0x32,    0x32,    0x32,// 0x10
    SCAN_NULL,      'n',      'N',              0x31,    0x31,    0x31,    0x31,// 0x11
    SCAN_NULL,      'o',      'O',              0x18,    0x18,    0x18,    0x18,// 0x12
    SCAN_NULL,      'p',      'P',              0x19,    0x19,    0x19,    0x19,// 0x13
    SCAN_NULL,      'q',      'Q',              0x10,    0x10,    0x10,    0x10,// 0x14
    SCAN_NULL,      'r',      'R',              0x13,    0x13,    0x13,    0x13,// 0x15
    SCAN_NULL,      's',      'S',              0x1F,    0x1F,    0x1F,    0x1F,// 0x16
    SCAN_NULL,      't',      'T',              0x14,    0x14,    0x14,    0x14,// 0x17
    SCAN_NULL,      'u',      'U',              0x16,    0x16,    0x16,    0x16,// 0x18
    SCAN_NULL,      'v',      'V',              0x2F,    0x2F,    0x2F,    0x2F,// 0x19
    SCAN_NULL,      'w',      'W',              0x11,    0x11,    0x11,    0x11,// 0x1A
    SCAN_NULL,      'x',      'X',              0x2D,    0x2D,    0x2D,    0x2D,// 0x1B
    SCAN_NULL,      'y',      'Y',              0x15,    0x15,    0x15,    0x15,// 0x1C
    SCAN_NULL,      'z',      'Z',              0x2C,    0x2C,    0x2C,    0x2C,// 0x1D
    SCAN_NULL,      '1',      '!',              0x02,    0x02,    0x78,    0x02,// 0x1E
    SCAN_NULL,      '2',      '@',              0x03,    0x03,    0x79,    0x03,// 0x1F
    SCAN_NULL,      '3',      '#',              0x04,    0x04,    0x7A,    0x04,// 0x20
    SCAN_NULL,      '4',      '$',              0x05,    0x05,    0x7B,    0x05,// 0x21
    SCAN_NULL,      '5',      '%',              0x06,    0x06,    0x7C,    0x06,// 0x22
    SCAN_NULL,      '6',      '^',              0x07,    0x07,    0x7D,    0x07,// 0x23
    SCAN_NULL,      '7',      '&',              0x08,    0x08,    0x7E,    0x08,// 0x24
    SCAN_NULL,      '8',      '*',              0x09,    0x09,    0x7F,    0x09,// 0x25
    SCAN_NULL,      '9',      '(',              0x0A,    0x0A,    0x81,    0x0A,// 0x26
    SCAN_NULL,      '0',      ')',              0x0B,    0x0B,    0x82,    0x0B,// 0x27
    SCAN_NULL,      0x0d,     0x0d,             0x1C,    0x1C,    0x1C,    0x1C,// 0x28   Enter
    SCAN_ESC,       0x00,     0x00,             0x01,    0x01,    0x01,    0x01,// 0x29   Esc
    SCAN_NULL,      0x08,     0x08,             0x0E,    0x0E,    0x0E,    0x0E,// 0x2A   Backspace
    SCAN_NULL,      0x09,     0x09,             0x0F,    0x0F,    0xA5,    0x94,// 0x2B   Tab
    SCAN_NULL,      ' ',      ' ',              0x39,    0x39,    0x39,    0x39,// 0x2C   Spacebar
    SCAN_NULL,      '-',      '_',              0x0C,    0x0C,    0x82,    0x0C,// 0x2D
    SCAN_NULL,      '=',      '+',              0x0D,    0x0D,    0x83,    0x00,// 0x2E
    SCAN_NULL,      '[',      '{',              0x1A,    0x1A,    0x1A,    0x1A,// 0x2F
    SCAN_NULL,      ']',      '}',              0x1B,    0x1B,    0x1B,    0x1B,// 0x30
    SCAN_NULL,      '\\',     '|',              0x2B,    0x2B,    0x2B,    0x2B,// 0x31
    SCAN_NULL,      '\\',     '|',              0x56,    0x56,    0x56,    0x56,// 0x32  Keyboard US \ and |
    SCAN_NULL,      ';',      ':',              0x27,    0x27,    0x27,    0x00,// 0x33
    SCAN_NULL,      '\'',     '"',              0x28,    0x28,    0x28,    0x00,// 0x34
    SCAN_NULL,      '`',      '~',              0x29,    0x29,    0x29,    0x00,// 0x35  Keyboard Grave Accent and Tlide
    SCAN_NULL,      ',',      '<',              0x33,    0x33,    0x33,    0x00,// 0x36
    SCAN_NULL,      '.',      '>',              0x34,    0x34,    0x34,    0x00,// 0x37
    SCAN_NULL,      '/',      '?',              0x35,    0x35,    0x35,    0x00,// 0x38
    SCAN_NULL,      0x00,     0x00,             0x3A,    0x3A,    0x3A,    0x3A,// 0x39   CapsLock
    SCAN_F1,        0x00,     0x00,             0x3B,    0x54,    0x68,    0x5E,// 0x3A
    SCAN_F2,        0x00,     0x00,             0x3C,    0x55,    0x69,    0x5F,// 0x3B
    SCAN_F3,        0x00,     0x00,             0x3D,    0x56,    0x6A,    0x60,// 0x3C
    SCAN_F4,        0x00,     0x00,             0x3E,    0x57,    0x6B,    0x61,// 0x3D
    SCAN_F5,        0x00,     0x00,             0x3F,    0x58,    0x6C,    0x62,// 0x3E
    SCAN_F6,        0x00,     0x00,             0x40,    0x59,    0x6D,    0x63,// 0x3F
    SCAN_F7,        0x00,     0x00,             0x41,    0x5A,    0x6E,    0x64,// 0x40
    SCAN_F8,        0x00,     0x00,             0x42,    0x5B,    0x6F,    0x65,// 0x41
    SCAN_F9,        0x00,     0x00,             0x43,    0x5C,    0x70,    0x66,// 0x42
    SCAN_F10,       0x00,     0x00,             0x44,    0x5D,    0x71,    0x67,// 0x43
    SCAN_F11,       0x00,     0x00,             0x85,    0x87,    0x8B,    0x89,// 0x44   F11
    SCAN_F12,       0x00,     0x00,             0x86,    0x88,    0x8C,    0x8A,// 0x45   F12
    SCAN_NULL,      0x00,     0x00,             0x80,    0x80,    0x80,    0x72,// 0x46   PrintScreen
    SCAN_NULL,      0x00,     0x00,             0x46,    0x46,    0x46,    0x46,// 0x47   Scroll Lock
    SCAN_PAUSE,     0x00,     0x00,             0x81,    0x81,    0x81,    0x81,// 0x48   Pause
    SCAN_NULL,      0x00,     0x00,             0x45,    0x45,    0x45,    0x45,// 0x53   NumLock
    SCAN_NULL,      '/',      '/',              0xB5,    0xB5,    0xA4,    0x95,// 0x54
    SCAN_NULL,      '*',      '*',              0x37,    0x37,    0x37,    0x96,// 0x55
    SCAN_NULL,      '-',      '-',              0x4A,    0x4A,    0x4A,    0x8E,// 0x56
    SCAN_NULL,      '+',      '+',              0x4E,    0x4E,    0x4E,    0x90,// 0x57
    SCAN_NULL,      0x0d,     0x0d,             0x9C,    0x9C,    0xA6,    0x9C,// 0x58
    SCAN_END,       '1',      '1',              0x4F,    0x4F,    0x9F,    0x75,// 0x59
    SCAN_DOWN,      '2',      '2',              0x50,    0x50,    0xA0,    0x91,// 0x5A
    SCAN_PAGE_DOWN, '3',      '3',              0x51,    0x51,    0xA1,    0x76,// 0x5B
    SCAN_LEFT,      '4',      '4',              0x4B,    0x4B,    0x9B,    0x73,// 0x5C
    SCAN_NULL,      '5',      '5',              0x4C,    0x4C,    0x00,    0x8F,// 0x5D
    SCAN_RIGHT,     '6',      '6',              0x4D,    0x4D,    0x9D,    0x74,// 0x5E
    SCAN_HOME,      '7',      '7',              0x47,    0x47,    0x97,    0x77,// 0x5F
    SCAN_UP,        '8',      '8',              0x48,    0x48,    0x98,    0x8D,// 0x60
    SCAN_PAGE_UP,   '9',      '9',              0x49,    0x49,    0x99,    0x84,// 0x61
    SCAN_INSERT,    '0',      '0',              0x52,    0x52,    0xA2,    0x92,// 0x62
    SCAN_DELETE,    '.',      '.',              0x53,    0x53,    0xA3,    0x93,// 0x63
    SCAN_NULL,      '\\',     '|',              0x2B,    0x2B,    0x2B,    0x2B,// 0x64 Keyboard Non-US \ and |
    SCAN_NULL,      0x00,     0x00,             0xDD,    0xDD,    0xDD,    0xDD,// 0x65 Keyboard Application
    0xFF,           0x00,     0x00,             0x00,    0x00,    0x00,    0x00 // End of table
};


//
// Text In Splitter Private Data template
//
GLOBAL_REMOVE_IF_UNREFERENCED TEXT_IN_SPLITTER_PRIVATE_DATA  mConIn = {
  TEXT_IN_SPLITTER_PRIVATE_DATA_SIGNATURE,
  (EFI_HANDLE) NULL,

  {
    ConSplitterTextInReset,
    ConSplitterTextInReadKeyStroke,
    (EFI_EVENT) NULL
  },
  0,
  (EFI_SIMPLE_TEXT_INPUT_PROTOCOL **) NULL,
  0,

  {
    ConSplitterTextInResetEx,
    ConSplitterTextInReadKeyStrokeEx,
    (EFI_EVENT) NULL,
    ConSplitterTextInSetState,
    ConSplitterTextInRegisterKeyNotify,
    ConSplitterTextInUnregisterKeyNotify
  },
  0,
  (EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL **) NULL,
  0,
  {
    (LIST_ENTRY *) NULL,
    (LIST_ENTRY *) NULL
  },
  0,         // KeyToggleState

  {
    ConSplitterSimplePointerReset,
    ConSplitterSimplePointerGetState,
    (EFI_EVENT) NULL,
    (EFI_SIMPLE_POINTER_MODE *) NULL
  },
  {
    0x10000,
    0x10000,
    0x10000,
    TRUE,
    TRUE
  },
  0,
  (EFI_SIMPLE_POINTER_PROTOCOL **) NULL,
  0,

  {
    ConSplitterAbsolutePointerReset,
    ConSplitterAbsolutePointerGetState,
    (EFI_EVENT) NULL,
    (EFI_ABSOLUTE_POINTER_MODE *) NULL
  },
  {
    0,       // AbsoluteMinX
    0,       // AbsoluteMinY
    0,       // AbsoluteMinZ
    0x10000, // AbsoluteMaxX
    0x10000, // AbsoluteMaxY
    0x10000, // AbsoluteMaxZ
    0        // Attributes
  },
  0,
  (EFI_ABSOLUTE_POINTER_PROTOCOL **) NULL,
  0,
  FALSE,

  FALSE,
  FALSE
};


//
// Uga Draw Protocol Private Data template
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_UGA_DRAW_PROTOCOL mUgaDrawProtocolTemplate = {
  ConSplitterUgaDrawGetMode,
  ConSplitterUgaDrawSetMode,
  ConSplitterUgaDrawBlt
};

//
// Graphics Output Protocol Private Data template
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_GRAPHICS_OUTPUT_PROTOCOL mGraphicsOutputProtocolTemplate = {
  ConSplitterGraphicsOutputQueryMode,
  ConSplitterGraphicsOutputSetMode,
  ConSplitterGraphicsOutputBlt,
  NULL
};


//
// Text Out Splitter Private Data template
//
GLOBAL_REMOVE_IF_UNREFERENCED TEXT_OUT_SPLITTER_PRIVATE_DATA mConOut = {
  TEXT_OUT_SPLITTER_PRIVATE_DATA_SIGNATURE,
  (EFI_HANDLE) NULL,
  {
    ConSplitterTextOutReset,
    ConSplitterTextOutOutputString,
    ConSplitterTextOutTestString,
    ConSplitterTextOutQueryMode,
    ConSplitterTextOutSetMode,
    ConSplitterTextOutSetAttribute,
    ConSplitterTextOutClearScreen,
    ConSplitterTextOutSetCursorPosition,
    ConSplitterTextOutEnableCursor,
    (EFI_SIMPLE_TEXT_OUTPUT_MODE *) NULL
  },
  {
    1,
    0,
    0,
    0,
    0,
    FALSE,
  },

  {
    NULL,
    NULL,
    NULL
  },
  0,
  0,
  0,
  0,

  {
    NULL,
    NULL,
    NULL,
    NULL
  },
  (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *) NULL,
  0,
  0,

  0,
  (TEXT_OUT_AND_GOP_DATA *) NULL,
  0,
  (TEXT_OUT_SPLITTER_QUERY_DATA *) NULL,
  0,
  (INT32 *) NULL,
  0,
  0,
  (CHAR16 *) NULL,
  (INT32 *) NULL,
  FALSE,
  TRUE
};

//
// Standard Error Text Out Splitter Data Template
//
GLOBAL_REMOVE_IF_UNREFERENCED TEXT_OUT_SPLITTER_PRIVATE_DATA mStdErr = {
  TEXT_OUT_SPLITTER_PRIVATE_DATA_SIGNATURE,
  (EFI_HANDLE) NULL,
  {
    ConSplitterTextOutReset,
    ConSplitterTextOutOutputString,
    ConSplitterTextOutTestString,
    ConSplitterTextOutQueryMode,
    ConSplitterTextOutSetMode,
    ConSplitterTextOutSetAttribute,
    ConSplitterTextOutClearScreen,
    ConSplitterTextOutSetCursorPosition,
    ConSplitterTextOutEnableCursor,
    (EFI_SIMPLE_TEXT_OUTPUT_MODE *) NULL
  },
  {
    1,
    0,
    0,
    0,
    0,
    FALSE,
  },

  {
    NULL,
    NULL,
    NULL
  },
  0,
  0,
  0,
  0,

  {
    NULL,
    NULL,
    NULL,
    NULL
  },
  (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *) NULL,
  0,
  0,

  0,
  (TEXT_OUT_AND_GOP_DATA *) NULL,
  0,
  (TEXT_OUT_SPLITTER_QUERY_DATA *) NULL,
  0,
  (INT32 *) NULL,
  0,
  0,
  (CHAR16 *) NULL,
  (INT32 *) NULL,
  FALSE,
  FALSE
};

//
// Driver binding instance for Console Input Device
//
EFI_DRIVER_BINDING_PROTOCOL           gConSplitterConInDriverBinding = {
  ConSplitterConInDriverBindingSupported,
  ConSplitterConInDriverBindingStart,
  ConSplitterConInDriverBindingStop,
  0xa,
  NULL,
  NULL
};

//
// Driver binding instance for Console Out device
//
EFI_DRIVER_BINDING_PROTOCOL           gConSplitterConOutDriverBinding = {
  ConSplitterConOutDriverBindingSupported,
  ConSplitterConOutDriverBindingStart,
  ConSplitterConOutDriverBindingStop,
  0xa,
  NULL,
  NULL
};

//
// Driver binding instance for Standard Error device
//
EFI_DRIVER_BINDING_PROTOCOL           gConSplitterStdErrDriverBinding = {
  ConSplitterStdErrDriverBindingSupported,
  ConSplitterStdErrDriverBindingStart,
  ConSplitterStdErrDriverBindingStop,
  0xa,
  NULL,
  NULL
};

//
// Driver binding instance for Simple Pointer protocol
//
EFI_DRIVER_BINDING_PROTOCOL           gConSplitterSimplePointerDriverBinding = {
  ConSplitterSimplePointerDriverBindingSupported,
  ConSplitterSimplePointerDriverBindingStart,
  ConSplitterSimplePointerDriverBindingStop,
  0xa,
  NULL,
  NULL
};

//
// Driver binding instance for Absolute Pointer protocol
//
EFI_DRIVER_BINDING_PROTOCOL           gConSplitterAbsolutePointerDriverBinding = {
  ConSplitterAbsolutePointerDriverBindingSupported,
  ConSplitterAbsolutePointerDriverBindingStart,
  ConSplitterAbsolutePointerDriverBindingStop,
  0xa,
  NULL,
  NULL
};

/**
  BdsEntry hook function. This function uses to construct virtual ConOut and standard error
  device and then enter original BdsEntry ().

  @param[in]  This             The EFI_BDS_ARCH_PROTOCOL instance.
**/
VOID
EFIAPI
BdsEntry (
  IN EFI_BDS_ARCH_PROTOCOL  *This
  )
{
  EFI_STATUS       Status;
  //
  // Create virtual device handle for ConOut Splitter
  //
  Status = ConSplitterTextOutConstructor (&mConOut);
  if (!EFI_ERROR (Status)) {
    if (!FeaturePcdGet (PcdConOutGopSupport)) {
      //
      // If Graphics Outpurt protocol not supported, UGA Draw protocol is installed
      // on the virtual handle.
      //
      Status = gBS->InstallMultipleProtocolInterfaces (
                      &mConOut.VirtualHandle,
                      &gEfiSimpleTextOutProtocolGuid,
                      &mConOut.TextOut,
                      &gEfiUgaDrawProtocolGuid,
                      &mConOut.UgaDraw,
                      NULL
                      );
    } else if (!FeaturePcdGet (PcdConOutUgaSupport)) {
      //
      // If UGA Draw protocol not supported, Graphics Output Protocol is installed
      // on virtual handle.
      //
      Status = gBS->InstallMultipleProtocolInterfaces (
                      &mConOut.VirtualHandle,
                      &gEfiSimpleTextOutProtocolGuid,
                      &mConOut.TextOut,
                      &gEfiGraphicsOutputProtocolGuid,
                      &mConOut.GraphicsOutput,
                      NULL
                      );
    } else {
      //
      // Boot Graphics Output protocol and UGA Draw protocol are supported,
      // both they will be installed on virtual handle.
      //
      Status = gBS->InstallMultipleProtocolInterfaces (
                      &mConOut.VirtualHandle,
                      &gEfiSimpleTextOutProtocolGuid,
                      &mConOut.TextOut,
                      &gEfiGraphicsOutputProtocolGuid,
                      &mConOut.GraphicsOutput,
                      &gEfiUgaDrawProtocolGuid,
                      &mConOut.UgaDraw,
                      NULL
                      );
    }

    if (!EFI_ERROR (Status)) {
      //
      // Update the EFI System Table with new virtual console
      // and Update the pointer to Text Output protocol.
      //
      gST->ConsoleOutHandle = mConOut.VirtualHandle;
      gST->ConOut           = &mConOut.TextOut;
    }

  }

  //
  // Create virtual device handle for StdErr Splitter
  //
  Status = ConSplitterTextOutConstructor (&mStdErr);
  if (!EFI_ERROR (Status)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &mStdErr.VirtualHandle,
                    &gEfiSimpleTextOutProtocolGuid,
                    &mStdErr.TextOut,
                    NULL
                    );
    if (!EFI_ERROR (Status)) {
      //
      // Update the EFI System Table with new virtual console
      // and update the pointer to Text Output protocol.
      //
      gST->StandardErrorHandle  = mStdErr.VirtualHandle;
      gST->StdErr               = &mStdErr.TextOut;
    }
  }

  //
  // Update the CRC32 in the EFI System Table header
  //
  gST->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (
        (UINT8 *) &gST->Hdr,
        gST->Hdr.HeaderSize,
        &gST->Hdr.CRC32
        );
  mOriginalBdsEntry (This);
}

/**
  This function uses to hook original BdsEntry ().

  @param Event    Event whose notification function is being invoked.
  @param Context  Pointer to the notification function's context.

**/
VOID
EFIAPI
BdsCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS                        Status;
  EFI_BDS_ARCH_PROTOCOL             *Bds;

  Status = gBS->LocateProtocol (
                  &gEfiBdsArchProtocolGuid,
                  NULL,
                  (VOID **)&Bds
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return;
  }
  mOriginalBdsEntry = Bds->Entry;
  Bds->Entry = BdsEntry;
  return;
}

/**
  This function uses to initialize code to check system is weather enter BDS phase.

  @param[in]  SystemTable   Pointer to the EFI System Table.
**/
STATIC
VOID
InitializeBdsPhaseCheck (
  VOID
  )
{
  EFI_STATUS                        Status;
  EFI_BDS_ARCH_PROTOCOL             *Bds;
  EFI_EVENT                         Event;
  VOID                              *Registration;

  Status = gBS->LocateProtocol (
                  &gEfiBdsArchProtocolGuid,
                  NULL,
                  (VOID **)&Bds
                  );
  if (!EFI_ERROR (Status)) {
    mOriginalBdsEntry = Bds->Entry;
    Bds->Entry = BdsEntry;
  } else {
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    BdsCallback,
                    NULL,
                    &Event
                    );
    ASSERT_EFI_ERROR (Status);
    //
    // Register for protocol notifications on this event
    //
    Status = gBS->RegisterProtocolNotify (
                    &gEfiBdsArchProtocolGuid,
                    Event,
                    &Registration
                    );
    ASSERT_EFI_ERROR (Status);
  }
  return;
}

/**
  The Entry Point for module ConSplitter. The user code starts with this function.

  Installs driver module protocols and. Creates virtual device handles for ConIn,
  ConOut, and StdErr. Installs Simple Text In protocol, Simple Text In Ex protocol,
  Simple Pointer protocol, Absolute Pointer protocol on those virtual handlers.
  Installs Graphics Output protocol and/or UGA Draw protocol if needed.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
ConSplitterDriverEntry(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;
  VOID                    *WinNtProtocol;

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gConSplitterConInDriverBinding,
             ImageHandle,
             &gConSplitterConInComponentName,
             &gConSplitterConInComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gConSplitterSimplePointerDriverBinding,
             NULL,
             &gConSplitterSimplePointerComponentName,
             &gConSplitterSimplePointerComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gConSplitterAbsolutePointerDriverBinding,
             NULL,
             &gConSplitterAbsolutePointerComponentName,
             &gConSplitterAbsolutePointerComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gConSplitterConOutDriverBinding,
             NULL,
             &gConSplitterConOutComponentName,
             &gConSplitterConOutComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gConSplitterStdErrDriverBinding,
             NULL,
             &gConSplitterStdErrComponentName,
             &gConSplitterStdErrComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  //
  // Either Graphics Output protocol or UGA Draw protocol must be supported.
  //
  ASSERT (FeaturePcdGet (PcdConOutGopSupport) ||
          FeaturePcdGet (PcdConOutUgaSupport));

  //
  // The driver creates virtual handles for ConIn, ConOut, StdErr.
  // The virtual handles will always exist even if no console exist in the
  // system. This is need to support hotplug devices like USB.
  //
  //
  // Create virtual device handle for ConIn Splitter
  //
  Status = ConSplitterTextInConstructor (&mConIn);
  if (!EFI_ERROR (Status)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &mConIn.VirtualHandle,
                    &gEfiSimpleTextInProtocolGuid,
                    &mConIn.TextIn,
                    &gEfiSimpleTextInputExProtocolGuid,
                    &mConIn.TextInEx,
                    &gEfiSimplePointerProtocolGuid,
                    &mConIn.SimplePointer,
                    &gEfiAbsolutePointerProtocolGuid,
                    &mConIn.AbsolutePointer,
                    NULL
                    );
    if (!EFI_ERROR (Status)) {
      //
      // Update the EFI System Table with new virtual console
      // and update the pointer to Simple Text Input protocol.
      //
      gST->ConsoleInHandle  = mConIn.VirtualHandle;
      gST->ConIn            = &mConIn.TextIn;
    }
  }
  //
  // OemSvcLogoResolution () may locate other protocol (ex: gEfiSetupUtilityProtocolGuid) to get logo resolution.
  // To prevent from dependency issue, we creating virtual standard output and standard error virtual device
  // before entering BDS phase.
  //
  InitializeBdsPhaseCheck ();
  //
  // Update the CRC32 in the EFI System Table header
  //
  gST->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (
        (UINT8 *) &gST->Hdr,
        gST->Hdr.HeaderSize,
        &gST->Hdr.CRC32
        );

  //
  // Check whether is in NtEmulator
  //
  Status = gBS->LocateProtocol (
                  &gEfiWinNtThunkProtocolGuid,
                  NULL,
                  (VOID **) &WinNtProtocol
                  );
  if (!EFI_ERROR (Status)) {
    mInNtEmulator = TRUE;
  }

  return EFI_SUCCESS;

}

/**
  Construct console input devices' private data.

  @param  ConInPrivate             A pointer to the TEXT_IN_SPLITTER_PRIVATE_DATA
                                   structure.

  @retval EFI_OUT_OF_RESOURCES     Out of resources.
  @retval EFI_SUCCESS              Text Input Devcie's private data has been constructed.
  @retval other                    Failed to construct private data.

**/
EFI_STATUS
ConSplitterTextInConstructor (
  TEXT_IN_SPLITTER_PRIVATE_DATA       *ConInPrivate
  )
{
  EFI_STATUS  Status;

  //
  // Allocate buffer for Simple Text Input device
  //
  Status = ConSplitterGrowBuffer (
            sizeof (EFI_SIMPLE_TEXT_INPUT_PROTOCOL *),
            &ConInPrivate->TextInListCount,
            (VOID **) &ConInPrivate->TextInList
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Create Event to wait for a key
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  ConSplitterTextInWaitForKey,
                  ConInPrivate,
                  &ConInPrivate->TextIn.WaitForKey
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Allocate buffer for Simple Text Input Ex device
  //
  Status = ConSplitterGrowBuffer (
             sizeof (EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *),
             &ConInPrivate->TextInExListCount,
             (VOID **) &ConInPrivate->TextInExList
             );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Create Event to wait for a key Ex
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  ConSplitterTextInWaitForKey,
                  ConInPrivate,
                  &ConInPrivate->TextInEx.WaitForKeyEx
                  );
  ASSERT_EFI_ERROR (Status);

  InitializeListHead (&ConInPrivate->NotifyList);

  ConInPrivate->AbsolutePointer.Mode = &ConInPrivate->AbsolutePointerMode;
  //
  // Allocate buffer for Absolute Pointer device
  //
  Status = ConSplitterGrowBuffer (
            sizeof (EFI_ABSOLUTE_POINTER_PROTOCOL *),
            &ConInPrivate->AbsolutePointerListCount,
            (VOID **) &ConInPrivate->AbsolutePointerList
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Create Event to wait for device input for Absolute pointer device
  //
  Status = gBS->CreateEvent (
            EVT_NOTIFY_WAIT,
            TPL_NOTIFY,
            ConSplitterAbsolutePointerWaitForInput,
            ConInPrivate,
            &ConInPrivate->AbsolutePointer.WaitForInput
        );
  ASSERT_EFI_ERROR (Status);

  ConInPrivate->SimplePointer.Mode = &ConInPrivate->SimplePointerMode;
  //
  // Allocate buffer for Simple Pointer device
  //
  Status = ConSplitterGrowBuffer (
            sizeof (EFI_SIMPLE_POINTER_PROTOCOL *),
            &ConInPrivate->PointerListCount,
            (VOID **) &ConInPrivate->PointerList
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Create Event to wait for device input for Simple pointer device
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  ConSplitterSimplePointerWaitForInput,
                  ConInPrivate,
                  &ConInPrivate->SimplePointer.WaitForInput
                  );

  return Status;
}

/**
  Construct console output devices' private data.

  @param  ConOutPrivate            A pointer to the TEXT_OUT_SPLITTER_PRIVATE_DATA
                                   structure.

  @retval EFI_OUT_OF_RESOURCES     Out of resources.
  @retval EFI_SUCCESS              Text Input Devcie's private data has been constructed.

**/
EFI_STATUS
ConSplitterTextOutConstructor (
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *ConOutPrivate
  )
{
  EFI_STATUS  Status;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *Info;
  OEM_LOGO_RESOLUTION_DEFINITION        *OemLogoResolutionPtr;

  //
  // Init local
  //
  Info = NULL;
  OemLogoResolutionPtr = NULL;

  //
  // Copy protocols template
  //
  if (FeaturePcdGet (PcdConOutUgaSupport)) {
    CopyMem (&ConOutPrivate->UgaDraw, &mUgaDrawProtocolTemplate, sizeof (EFI_UGA_DRAW_PROTOCOL));
  }
  if (FeaturePcdGet (PcdConOutGopSupport)) {
    CopyMem (&ConOutPrivate->GraphicsOutput, &mGraphicsOutputProtocolTemplate, sizeof (EFI_GRAPHICS_OUTPUT_PROTOCOL));
  }

  //
  // Initilize console output splitter's private data.
  //
  ConOutPrivate->TextOut.Mode = &ConOutPrivate->TextOutMode;

  //
  // When new console device is added, the new mode will be set later,
  // so put current mode back to init state.
  //
  ConOutPrivate->TextOutMode.Mode = 0xFF;
  //
  // Allocate buffer for Console Out device
  //
  Status = ConSplitterGrowBuffer (
            sizeof (TEXT_OUT_AND_GOP_DATA),
            &ConOutPrivate->TextOutListCount,
            (VOID **) &ConOutPrivate->TextOutList
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Allocate buffer for Text Out query data
  //
  Status = ConSplitterGrowBuffer (
            sizeof (TEXT_OUT_SPLITTER_QUERY_DATA),
            &ConOutPrivate->TextOutQueryDataCount,
            (VOID **) &ConOutPrivate->TextOutQueryData
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Setup the default console to SCU text dimension and mode to 2
  //
  ConOutPrivate->TextOutQueryData[0].Columns  = 80;
  ConOutPrivate->TextOutQueryData[0].Rows     = 25;

  ConOutPrivate->TextOutQueryData[1].Columns  = 80;
  ConOutPrivate->TextOutQueryData[1].Rows     = 50;

  OemSvcLogoResolution (&OemLogoResolutionPtr);
  if (OemLogoResolutionPtr == NULL) {
    ConOutPrivate->TextOutQueryData[2].Columns = 1024 / EFI_GLYPH_WIDTH;
    ConOutPrivate->TextOutQueryData[2].Rows    = 768 / EFI_GLYPH_HEIGHT;
  } else {
    ConOutPrivate->TextOutQueryData[2].Columns = (OemLogoResolutionPtr->ScuResolutionX) / EFI_GLYPH_WIDTH;
    ConOutPrivate->TextOutQueryData[2].Rows    = (OemLogoResolutionPtr->ScuResolutionY) / EFI_GLYPH_HEIGHT;
  }
  ConOutPrivate->TextOutMode.MaxMode = 3;

  DevNullTextOutSetMode (ConOutPrivate, 2);
  ConOutPrivate->TextOutMode.Attribute = EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK);

  //
  // set GOP to full screen text mode
  //
  if (FeaturePcdGet (PcdConOutUgaSupport)) {
    //
    // Setup the UgaDraw to 800 x 600 x 32 bits per pixel, 60Hz.
    //
    ConSplitterUgaDrawSetMode (&ConOutPrivate->UgaDraw, 1024, 768, 32, 60);
  }
  if (FeaturePcdGet (PcdConOutGopSupport)) {
    //
    // Setup resource for mode information in Graphics Output Protocol interface
    //
    if ((ConOutPrivate->GraphicsOutput.Mode = AllocateZeroPool (sizeof (EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE))) == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    if ((ConOutPrivate->GraphicsOutput.Mode->Info = AllocateZeroPool (sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION))) == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // Setup the DevNullGraphicsOutput to 800 x 600 x 32 bits per pixel
    // DevNull will be updated to user-defined mode after driver has started.
    //
    if ((ConOutPrivate->GraphicsOutputModeBuffer = AllocateZeroPool (sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION))) == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    Info = &ConOutPrivate->GraphicsOutputModeBuffer[0];
    Info->Version = 0;
    Info->HorizontalResolution = 1024;
    Info->VerticalResolution = 768;
    Info->PixelFormat = PixelBltOnly;
    Info->PixelsPerScanLine = 1024;
    CopyMem (ConOutPrivate->GraphicsOutput.Mode->Info, Info, sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
    ConOutPrivate->GraphicsOutput.Mode->SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

    //
    // Initialize the following items, theset items remain unchanged in GraphicsOutput->SetMode()
    // GraphicsOutputMode->FrameBufferBase, GraphicsOutputMode->FrameBufferSize
    //
    ConOutPrivate->GraphicsOutput.Mode->FrameBufferBase = (EFI_PHYSICAL_ADDRESS) (UINTN) NULL;
    ConOutPrivate->GraphicsOutput.Mode->FrameBufferSize = 0;

    ConOutPrivate->GraphicsOutput.Mode->MaxMode = 1;
    //
    // Initial current mode to unknown state, and then set to mode 0
    //
    ConOutPrivate->GraphicsOutput.Mode->Mode = 0xffff;
    ConOutPrivate->GraphicsOutput.SetMode (&ConOutPrivate->GraphicsOutput, 0);
  }

  return EFI_SUCCESS;
}


/**
  Test to see if the specified protocol could be supported on the specified device.

  @param  This                Driver Binding protocol pointer.
  @param  ControllerHandle    Handle of device to test.
  @param  Guid                The specified protocol.

  @retval EFI_SUCCESS         The specified protocol is supported on this device.
  @retval EFI_UNSUPPORTED     The specified protocol attempts to be installed on virtul handle.
  @retval other               Failed to open specified protocol on this device.

**/
EFI_STATUS
ConSplitterSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_GUID                        *Guid
  )
{
  EFI_STATUS  Status;
  VOID        *Instance;

  //
  // Make sure the Console Splitter does not attempt to attach to itself
  //
  if (ControllerHandle == mConIn.VirtualHandle  ||
      ControllerHandle == mConOut.VirtualHandle ||
      ControllerHandle == mStdErr.VirtualHandle
      ) {
    return EFI_UNSUPPORTED;
  }

  //
  // Check to see whether the specific protocol could be opened BY_DRIVER
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  Guid,
                  &Instance,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
        ControllerHandle,
        Guid,
        This->DriverBindingHandle,
        ControllerHandle
        );

  return EFI_SUCCESS;
}

/**
  Test to see if Console In Device could be supported on the Controller.

  @param  This                Driver Binding protocol instance pointer.
  @param  ControllerHandle    Handle of device to test.
  @param  RemainingDevicePath Optional parameter use to pick a specific child
                              device to start.

  @retval EFI_SUCCESS         This driver supports this device.
  @retval other               This driver does not support this device.

**/
EFI_STATUS
EFIAPI
ConSplitterConInDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
{
  return ConSplitterSupported (
          This,
          ControllerHandle,
          &gEfiConsoleInDeviceGuid
          );
}

/**
  Test to see if Simple Pointer protocol could be supported on the Controller.

  @param  This                Driver Binding protocol instance pointer.
  @param  ControllerHandle    Handle of device to test.
  @param  RemainingDevicePath Optional parameter use to pick a specific child
                              device to start.

  @retval EFI_SUCCESS         This driver supports this device.
  @retval other               This driver does not support this device.

**/
EFI_STATUS
EFIAPI
ConSplitterSimplePointerDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
{
  return ConSplitterSupported (
          This,
          ControllerHandle,
          &gEfiSimplePointerProtocolGuid
          );
}

/**
  Test to see if Absolute Pointer protocol could be supported on the Controller.

  @param  This                Driver Binding protocol instance pointer.
  @param  ControllerHandle    Handle of device to test.
  @param  RemainingDevicePath Optional parameter use to pick a specific child
                              device to start.

  @retval EFI_SUCCESS         This driver supports this device.
  @retval other               This driver does not support this device.

**/
EFI_STATUS
EFIAPI
ConSplitterAbsolutePointerDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
{
  return ConSplitterSupported (
          This,
          ControllerHandle,
          &gEfiAbsolutePointerProtocolGuid
          );
}


/**
  Test to see if Console Out Device could be supported on the Controller.

  @param  This                Driver Binding protocol instance pointer.
  @param  ControllerHandle    Handle of device to test.
  @param  RemainingDevicePath Optional parameter use to pick a specific child
                              device to start.

  @retval EFI_SUCCESS         This driver supports this device.
  @retval other               This driver does not support this device.

**/
EFI_STATUS
EFIAPI
ConSplitterConOutDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
{
  return ConSplitterSupported (
          This,
          ControllerHandle,
          &gEfiConsoleOutDeviceGuid
          );
}

/**
  Test to see if Standard Error Device could be supported on the Controller.

  @param  This                Driver Binding protocol instance pointer.
  @param  ControllerHandle    Handle of device to test.
  @param  RemainingDevicePath Optional parameter use to pick a specific child
                              device to start.

  @retval EFI_SUCCESS         This driver supports this device.
  @retval other               This driver does not support this device.

**/
EFI_STATUS
EFIAPI
ConSplitterStdErrDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
{
  return ConSplitterSupported (
          This,
          ControllerHandle,
          &gEfiStandardErrorDeviceGuid
          );
}


/**
  Start ConSplitter on devcie handle by opening Console Device Guid on device handle
  and the console virtual handle. And Get the console interface on controller handle.

  @param  This                      Driver Binding protocol instance pointer.
  @param  ControllerHandle          Handle of device.
  @param  ConSplitterVirtualHandle  Console virtual Handle.
  @param  DeviceGuid                The specified Console Device, such as ConInDev,
                                    ConOutDev.
  @param  InterfaceGuid             The specified protocol to be opened.
  @param  Interface                 Protocol interface returned.

  @retval EFI_SUCCESS               This driver supports this device.
  @retval other                     Failed to open the specified Console Device Guid
                                    or specified protocol.

**/
EFI_STATUS
ConSplitterStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_HANDLE                      ConSplitterVirtualHandle,
  IN  EFI_GUID                        *DeviceGuid,
  IN  EFI_GUID                        *InterfaceGuid,
  OUT VOID                            **Interface
  )
{
  EFI_STATUS  Status;
  VOID        *Instance;

  //
  // Check to see whether the ControllerHandle has the DeviceGuid on it.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  DeviceGuid,
                  &Instance,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Open the Parent Handle for the child.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  DeviceGuid,
                  &Instance,
                  This->DriverBindingHandle,
                  ConSplitterVirtualHandle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    goto Err;
  }

  //
  // Open InterfaceGuid on the virtul handle.
  //
  Status =  gBS->OpenProtocol (
                ControllerHandle,
                InterfaceGuid,
                Interface,
                This->DriverBindingHandle,
                ConSplitterVirtualHandle,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );

  if (!EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  //
  // close the DeviceGuid on ConSplitter VirtualHandle.
  //
  gBS->CloseProtocol (
        ControllerHandle,
        DeviceGuid,
        This->DriverBindingHandle,
        ConSplitterVirtualHandle
        );

Err:
  //
  // close the DeviceGuid on ControllerHandle.
  //
  gBS->CloseProtocol (
        ControllerHandle,
        DeviceGuid,
        This->DriverBindingHandle,
        ControllerHandle
        );

  return Status;
}

/**
  Update EBDA region state

  @param [in]       Private             Text In Splitter pointer.
**/
VOID
EBDARegionState (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA         *Private
  )
{
  if (mInNtEmulator) {
    return ;
  }

  //
  // check bda valid
  //
  if ((BDA(BDA_MEMORY_SIZE_OFFSET) == 0xFFFF) || (BDA(BDA_MEMORY_SIZE_OFFSET) == 0)) {
    //
    // Class 3 Bios
    //
    return ;
  }

  //
  // Check whether BDA is valid (BDA offset 0x13(40:13) * 0x400 is equal EBDA address)
  //
  if ((BOOLEAN)((BDA(BDA_MEMORY_SIZE_OFFSET) * 0x400) == (BDA(EXT_DATA_SEG_OFFSET) << 4))) {
    mSaveKbcScanCode = TRUE;
  }
}


/**
  Put the KBC scan code into EBDA offset 0x164.

  @param [in]       KeyData             A pointer to a buffer that is filled in with the keystroke
                                        state data for the key that was pressed.
**/
VOID
SaveKBCScanCode (
  IN EFI_KEY_DATA               *KeyData
  )
{
  UINTN                         ConvertIndex = 0;
  UINT8                         ConvertValue = 0;

  do {
    if (KeyData->Key.ScanCode) {
      if (KeyData->Key.ScanCode == KeyCodeConvertionTable[ConvertIndex][0]) {
        if (KeyData->KeyState.KeyShiftState & (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED)) {
          ConvertValue = KeyCodeConvertionTable[ConvertIndex][4];
          break;
        } else if (KeyData->KeyState.KeyShiftState & (EFI_LEFT_CONTROL_PRESSED | EFI_RIGHT_CONTROL_PRESSED)) {
          ConvertValue = KeyCodeConvertionTable[ConvertIndex][6];
          break;
        } else if (KeyData->KeyState.KeyShiftState & (EFI_LEFT_ALT_PRESSED | EFI_RIGHT_ALT_PRESSED)) {
          ConvertValue = KeyCodeConvertionTable[ConvertIndex][5];
          break;
        } else {
          ConvertValue = KeyCodeConvertionTable[ConvertIndex][3];
          break;
        }
      }
    } else {
      if (KeyData->Key.UnicodeChar == KeyCodeConvertionTable[ConvertIndex][1]) {
        if (KeyData->KeyState.KeyShiftState & (EFI_LEFT_CONTROL_PRESSED | EFI_RIGHT_CONTROL_PRESSED)) {
          ConvertValue = KeyCodeConvertionTable[ConvertIndex][6];
          break;
        } else if (KeyData->KeyState.KeyShiftState & (EFI_LEFT_ALT_PRESSED | EFI_RIGHT_ALT_PRESSED)) {
          ConvertValue = KeyCodeConvertionTable[ConvertIndex][5];
          break;
        } else {
          ConvertValue = KeyCodeConvertionTable[ConvertIndex][3];
          break;
        }
      } else if (KeyData->Key.UnicodeChar == KeyCodeConvertionTable[ConvertIndex][2]) {
        ConvertValue = KeyCodeConvertionTable[ConvertIndex][4];
        break;
      }
    }
    ConvertIndex ++;
  } while (KeyCodeConvertionTable[ConvertIndex][0] != 0xFF);

  //
  // Put the KBC scan code into EBDA + 0x164
  //
  *(UINT16*)&(EBDA(EBDA_KEYBORD_SCAN_CODE)) = ConvertValue;
}




/**
  Start Console In Consplitter on device handle.

  @param  This                 Driver Binding protocol instance pointer.
  @param  ControllerHandle     Handle of device to bind driver to.
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @retval EFI_SUCCESS          Console In Consplitter is added to ControllerHandle.
  @retval other                Console In Consplitter does not support this device.

**/
EFI_STATUS
EFIAPI
ConSplitterConInDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
{
  EFI_STATUS                          Status;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL      *TextIn;
  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL   *TextInEx;

  //
  // Start ConSplitter on ControllerHandle, and create the virtual
  // agrogated console device on first call Start for a SimpleTextIn handle.
  //
  Status = ConSplitterStart (
            This,
            ControllerHandle,
            mConIn.VirtualHandle,
            &gEfiConsoleInDeviceGuid,
            &gEfiSimpleTextInProtocolGuid,
            (VOID **) &TextIn
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Add this device into Text In devices list.
  //
  Status = ConSplitterTextInAddDevice (&mConIn, TextIn);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiSimpleTextInputExProtocolGuid,
                  (VOID **) &TextInEx,
                  This->DriverBindingHandle,
                  mConIn.VirtualHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (!EFI_ERROR (Status)) {
    //
    // If Simple Text Input Ex protocol exists,
    // add this device into Text In Ex devices list.
    //
    EBDARegionState (&mConIn);
    Status = ConSplitterTextInExAddDevice (&mConIn, TextInEx);
  }

  return Status;
}


/**
  Start Simple Pointer Consplitter on device handle.

  @param  This                 Driver Binding protocol instance pointer.
  @param  ControllerHandle     Handle of device to bind driver to.
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @retval EFI_SUCCESS          Simple Pointer Consplitter is added to ControllerHandle.
  @retval other                Simple Pointer Consplitter does not support this device.

**/
EFI_STATUS
EFIAPI
ConSplitterSimplePointerDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
{
  EFI_STATUS                  Status;
  EFI_SIMPLE_POINTER_PROTOCOL *SimplePointer;

  //
  // Start ConSplitter on ControllerHandle, and create the virtual
  // agrogated console device on first call Start for a SimplePointer handle.
  //
  Status = ConSplitterStart (
            This,
            ControllerHandle,
            mConIn.VirtualHandle,
            &gEfiSimplePointerProtocolGuid,
            &gEfiSimplePointerProtocolGuid,
            (VOID **) &SimplePointer
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Add this devcie into Simple Pointer devices list.
  //
  return ConSplitterSimplePointerAddDevice (&mConIn, SimplePointer);
}


/**
  Start Absolute Pointer Consplitter on device handle.

  @param  This                 Driver Binding protocol instance pointer.
  @param  ControllerHandle     Handle of device to bind driver to.
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @retval EFI_SUCCESS          Absolute Pointer Consplitter is added to ControllerHandle.
  @retval other                Absolute Pointer Consplitter does not support this device.

**/
EFI_STATUS
EFIAPI
ConSplitterAbsolutePointerDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
{
  EFI_STATUS                        Status;
  EFI_ABSOLUTE_POINTER_PROTOCOL     *AbsolutePointer;

  //
  // Start ConSplitter on ControllerHandle, and create the virtual
  // agrogated console device on first call Start for a AbsolutePointer handle.
  //
  Status = ConSplitterStart (
             This,
             ControllerHandle,
             mConIn.VirtualHandle,
             &gEfiAbsolutePointerProtocolGuid,
             &gEfiAbsolutePointerProtocolGuid,
             (VOID **) &AbsolutePointer
             );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Add this devcie into Absolute Pointer devices list.
  //
  return ConSplitterAbsolutePointerAddDevice (&mConIn, AbsolutePointer);
}


/**
  Start Console Out Consplitter on device handle.

  @param  This                 Driver Binding protocol instance pointer.
  @param  ControllerHandle     Handle of device to bind driver to.
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @retval EFI_SUCCESS          Console Out Consplitter is added to ControllerHandle.
  @retval other                Console Out Consplitter does not support this device.

**/
EFI_STATUS
EFIAPI
ConSplitterConOutDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
{
  EFI_STATUS                           Status;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL      *TextOut;
  EFI_GRAPHICS_OUTPUT_PROTOCOL         *GraphicsOutput;
  EFI_UGA_DRAW_PROTOCOL                *UgaDraw;
  UINTN                                SizeOfInfo;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;

  //
  // Start ConSplitter on ControllerHandle, and create the virtual
  // agrogated console device on first call Start for a ConsoleOut handle.
  //
  Status = ConSplitterStart (
            This,
            ControllerHandle,
            mConOut.VirtualHandle,
            &gEfiConsoleOutDeviceGuid,
            &gEfiSimpleTextOutProtocolGuid,
            (VOID **) &TextOut
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  GraphicsOutput = NULL;
  UgaDraw        = NULL;
  //
  // Try to Open Graphics Output protocol
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **) &GraphicsOutput,
                  This->DriverBindingHandle,
                  mConOut.VirtualHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status) && FeaturePcdGet (PcdUgaConsumeSupport)) {
    //
    // Open UGA DRAW protocol
    //
    gBS->OpenProtocol (
           ControllerHandle,
           &gEfiUgaDrawProtocolGuid,
           (VOID **) &UgaDraw,
           This->DriverBindingHandle,
           mConOut.VirtualHandle,
           EFI_OPEN_PROTOCOL_GET_PROTOCOL
           );
  }

  //
  // When new console device is added, the new mode will be set later,
  // so put current mode back to init state.
  //
  mConOut.TextOutMode.Mode = 0xFF;

  //
  // If both ConOut and StdErr incorporate the same Text Out device,
  // their MaxMode and QueryData should be the intersection of both.
  //
  Status = ConSplitterTextOutAddDevice (&mConOut, TextOut, GraphicsOutput, UgaDraw);

  if (FeaturePcdGet (PcdConOutUgaSupport)) {
    //
    // Get the UGA mode data of ConOut from the current mode
    //
    if (GraphicsOutput != NULL) {
      Status = GraphicsOutput->QueryMode (GraphicsOutput, GraphicsOutput->Mode->Mode, &SizeOfInfo, &Info);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      ASSERT ( SizeOfInfo <= sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));

      mConOut.UgaHorizontalResolution = Info->HorizontalResolution;
      mConOut.UgaVerticalResolution   = Info->VerticalResolution;
      mConOut.UgaColorDepth           = 32;
      mConOut.UgaRefreshRate          = 60;

      FreePool (Info);

    } else if (UgaDraw != NULL  && FeaturePcdGet (PcdUgaConsumeSupport)) {
      Status = UgaDraw->GetMode (
                 UgaDraw,
                 &mConOut.UgaHorizontalResolution,
                 &mConOut.UgaVerticalResolution,
                 &mConOut.UgaColorDepth,
                 &mConOut.UgaRefreshRate
                 );
    }
  }

  return Status;
}


/**
  Start Standard Error Consplitter on device handle.

  @param  This                 Driver Binding protocol instance pointer.
  @param  ControllerHandle     Handle of device to bind driver to.
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @retval EFI_SUCCESS          Standard Error Consplitter is added to ControllerHandle.
  @retval other                Standard Error Consplitter does not support this device.

**/
EFI_STATUS
EFIAPI
ConSplitterStdErrDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
{
  EFI_STATUS                       Status;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *TextOut;

  //
  // Start ConSplitter on ControllerHandle, and create the virtual
  // agrogated console device on first call Start for a StandardError handle.
  //
  Status = ConSplitterStart (
            This,
            ControllerHandle,
            mStdErr.VirtualHandle,
            &gEfiStandardErrorDeviceGuid,
            &gEfiSimpleTextOutProtocolGuid,
            (VOID **) &TextOut
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // When new console device is added, the new mode will be set later,
  // so put current mode back to init state.
  //
  mStdErr.TextOutMode.Mode = 0xFF;

  //
  // If both ConOut and StdErr incorporate the same Text Out device,
  // their MaxMode and QueryData should be the intersection of both.
  //
  Status = ConSplitterTextOutAddDevice (&mStdErr, TextOut, NULL, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}


/**
  Stop ConSplitter on device handle by closing Console Device Guid on device handle
  and the console virtual handle.

  @param  This                      Protocol instance pointer.
  @param  ControllerHandle          Handle of device.
  @param  ConSplitterVirtualHandle  Console virtual Handle.
  @param  DeviceGuid                The specified Console Device, such as ConInDev,
                                    ConOutDev.
  @param  InterfaceGuid             The specified protocol to be opened.
  @param  Interface                 Protocol interface returned.

  @retval EFI_SUCCESS               Stop ConSplitter on ControllerHandle successfully.
  @retval other                     Failed to Stop ConSplitter on ControllerHandle.

**/
EFI_STATUS
ConSplitterStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_HANDLE                      ConSplitterVirtualHandle,
  IN  EFI_GUID                        *DeviceGuid,
  IN  EFI_GUID                        *InterfaceGuid,
  IN  VOID                            **Interface
  )
{
  EFI_STATUS  Status;

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  InterfaceGuid,
                  Interface,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // close the protocol refered.
  //
  gBS->CloseProtocol (
        ControllerHandle,
        DeviceGuid,
        This->DriverBindingHandle,
        ConSplitterVirtualHandle
        );

  gBS->CloseProtocol (
        ControllerHandle,
        DeviceGuid,
        This->DriverBindingHandle,
        ControllerHandle
        );

  return EFI_SUCCESS;
}


/**
  Stop Console In ConSplitter on ControllerHandle by closing Console In Devcice GUID.

  @param  This              Driver Binding protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
ConSplitterConInDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
{
  EFI_STATUS                        Status;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL    *TextIn;
  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *TextInEx;

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiSimpleTextInputExProtocolGuid,
                  (VOID **) &TextInEx,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (!EFI_ERROR (Status)) {
    //
    // If Simple Text Input Ex protocol exists,
    // remove device from Text Input Ex devices list.
    //
    Status = ConSplitterTextInExDeleteDevice (&mConIn, TextInEx);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Close Simple Text In protocol on controller handle and virtual handle.
  //
  Status = ConSplitterStop (
            This,
            ControllerHandle,
            mConIn.VirtualHandle,
            &gEfiConsoleInDeviceGuid,
            &gEfiSimpleTextInProtocolGuid,
            (VOID **) &TextIn
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Remove device from Text Input devices list.
  //
  return ConSplitterTextInDeleteDevice (&mConIn, TextIn);
}


/**
  Stop Simple Pointer protocol ConSplitter on ControllerHandle by closing
  Simple Pointer protocol.

  @param  This              Driver Binding protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
ConSplitterSimplePointerDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
{
  EFI_STATUS                  Status;
  EFI_SIMPLE_POINTER_PROTOCOL *SimplePointer;

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  //
  // Close Simple Pointer protocol on controller handle and virtual handle.
  //
  Status = ConSplitterStop (
            This,
            ControllerHandle,
            mConIn.VirtualHandle,
            &gEfiSimplePointerProtocolGuid,
            &gEfiSimplePointerProtocolGuid,
            (VOID **) &SimplePointer
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Remove this device from Simple Pointer device list.
  //
  return ConSplitterSimplePointerDeleteDevice (&mConIn, SimplePointer);
}


/**
  Stop Absolute Pointer protocol ConSplitter on ControllerHandle by closing
  Absolute Pointer protocol.

  @param  This              Driver Binding protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
ConSplitterAbsolutePointerDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
{
  EFI_STATUS                        Status;
  EFI_ABSOLUTE_POINTER_PROTOCOL     *AbsolutePointer;

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  //
  // Close Absolute Pointer protocol on controller handle and virtual handle.
  //
  Status = ConSplitterStop (
             This,
             ControllerHandle,
             mConIn.VirtualHandle,
             &gEfiAbsolutePointerProtocolGuid,
             &gEfiAbsolutePointerProtocolGuid,
             (VOID **) &AbsolutePointer
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Remove this device from Absolute Pointer device list.
  //
  return ConSplitterAbsolutePointerDeleteDevice (&mConIn, AbsolutePointer);
}


/**
  Stop Console Out ConSplitter on device handle by closing Console Out Devcice GUID.

  @param  This              Driver Binding protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
ConSplitterConOutDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
{
  EFI_STATUS                       Status;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *TextOut;

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  //
  // Close Absolute Pointer protocol on controller handle and virtual handle.
  //
  Status = ConSplitterStop (
            This,
            ControllerHandle,
            mConOut.VirtualHandle,
            &gEfiConsoleOutDeviceGuid,
            &gEfiSimpleTextOutProtocolGuid,
            (VOID **) &TextOut
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Remove this device from Text Out device list.
  //
  return ConSplitterTextOutDeleteDevice (&mConOut, TextOut);
}


/**
  Stop Standard Error ConSplitter on ControllerHandle by closing Standard Error GUID.

  @param  This              Driver Binding protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
ConSplitterStdErrDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
{
  EFI_STATUS                       Status;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *TextOut;

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  //
  // Close Standard Error Device on controller handle and virtual handle.
  //
  Status = ConSplitterStop (
            This,
            ControllerHandle,
            mStdErr.VirtualHandle,
            &gEfiStandardErrorDeviceGuid,
            &gEfiSimpleTextOutProtocolGuid,
            (VOID **) &TextOut
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Delete this console error out device's data structures.
  //
  return ConSplitterTextOutDeleteDevice (&mStdErr, TextOut);
}


/**
  Take the passed in Buffer of size SizeOfCount and grow the buffer
  by MAX (CONSOLE_SPLITTER_CONSOLES_ALLOC_UNIT, MaxGrow) * SizeOfCount
  bytes. Copy the current data in Buffer to the new version of Buffer
  and free the old version of buffer.

  @param  SizeOfCount              Size of element in array.
  @param  Count                    Current number of elements in array.
  @param  Buffer                   Bigger version of passed in Buffer with all the
                                   data.

  @retval EFI_SUCCESS              Buffer size has grown.
  @retval EFI_OUT_OF_RESOURCES     Could not grow the buffer size.

**/
EFI_STATUS
ConSplitterGrowBuffer (
  IN      UINTN                       SizeOfCount,
  IN OUT  UINTN                       *Count,
  IN OUT  VOID                        **Buffer
  )
{
  VOID  *Ptr;

  //
  // grow the buffer to new buffer size,
  // copy the old buffer's content to the new-size buffer,
  // then free the old buffer.
  //
  *Count += CONSOLE_SPLITTER_CONSOLES_ALLOC_UNIT;
  Ptr = ReallocatePool (
          SizeOfCount * ((*Count) - CONSOLE_SPLITTER_CONSOLES_ALLOC_UNIT),
          SizeOfCount * (*Count),
          *Buffer
          );
  if (Ptr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  *Buffer = Ptr;
  return EFI_SUCCESS;
}


/**
  Add Text Input Device in Consplitter Text Input list.

  @param  Private                  Text In Splitter pointer.
  @param  TextIn                   Simple Text Input protocol pointer.

  @retval EFI_SUCCESS              Text Input Device added successfully.
  @retval EFI_OUT_OF_RESOURCES     Could not grow the buffer size.

**/
EFI_STATUS
ConSplitterTextInAddDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *TextIn
  )
{
  EFI_STATUS  Status;

  //
  // If the Text In List is full, enlarge it by calling ConSplitterGrowBuffer().
  //
  if (Private->CurrentNumberOfConsoles >= Private->TextInListCount) {
    Status = ConSplitterGrowBuffer (
              sizeof (EFI_SIMPLE_TEXT_INPUT_PROTOCOL *),
              &Private->TextInListCount,
              (VOID **) &Private->TextInList
              );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  //
  // Add the new text-in device data structure into the Text In List.
  //
  Private->TextInList[Private->CurrentNumberOfConsoles] = TextIn;
  Private->CurrentNumberOfConsoles++;

  //
  // Extra CheckEvent added to reduce the double CheckEvent().
  //
  gBS->CheckEvent (TextIn->WaitForKey);

  return EFI_SUCCESS;
}


/**
  Remove Text Input Device from Consplitter Text Input list.

  @param  Private                  Text In Splitter pointer.
  @param  TextIn                   Simple Text protocol pointer.

  @retval EFI_SUCCESS              Simple Text Device removed successfully.
  @retval EFI_NOT_FOUND            No Simple Text Device found.

**/
EFI_STATUS
ConSplitterTextInDeleteDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *TextIn
  )
{
  UINTN Index;
  //
  // Remove the specified text-in device data structure from the Text In List,
  // and rearrange the remaining data structures in the Text In List.
  //
  for (Index = 0; Index < Private->CurrentNumberOfConsoles; Index++) {
    if (Private->TextInList[Index] == TextIn) {
      for (; Index < Private->CurrentNumberOfConsoles - 1; Index++) {
        Private->TextInList[Index] = Private->TextInList[Index + 1];
      }

      Private->CurrentNumberOfConsoles--;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Add Text Input Ex Device in Consplitter Text Input Ex list.

  @param  Private                  Text In Splitter pointer.
  @param  TextInEx                 Simple Text Input Ex Input protocol pointer.

  @retval EFI_SUCCESS              Text Input Ex Device added successfully.
  @retval EFI_OUT_OF_RESOURCES     Could not grow the buffer size.

**/
EFI_STATUS
ConSplitterTextInExAddDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA         *Private,
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL     *TextInEx
  )
{
  EFI_STATUS                  Status;
  LIST_ENTRY                  *Link;
  TEXT_IN_EX_SPLITTER_NOTIFY  *CurrentNotify;
  UINTN                       TextInExListCount;

  //
  // If the Text Input Ex List is full, enlarge it by calling ConSplitterGrowBuffer().
  //
  if (Private->CurrentNumberOfExConsoles >= Private->TextInExListCount) {
    for (Link = Private->NotifyList.ForwardLink; Link != &Private->NotifyList; Link = Link->ForwardLink) {
      CurrentNotify     = TEXT_IN_EX_SPLITTER_NOTIFY_FROM_THIS (Link);
      TextInExListCount = Private->TextInExListCount;

      Status = ConSplitterGrowBuffer (
                 sizeof (EFI_HANDLE),
                 &TextInExListCount,
                 (VOID **) &CurrentNotify->NotifyHandleList
                 );
      if (EFI_ERROR (Status)) {
        return EFI_OUT_OF_RESOURCES;
      }
    }

    Status = ConSplitterGrowBuffer (
              sizeof (EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *),
              &Private->TextInExListCount,
              (VOID **) &Private->TextInExList
              );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  // Register the key notify in the new text-in device
  //
  for (Link = Private->NotifyList.ForwardLink; Link != &Private->NotifyList; Link = Link->ForwardLink) {
    CurrentNotify = TEXT_IN_EX_SPLITTER_NOTIFY_FROM_THIS (Link);
    Status = TextInEx->RegisterKeyNotify (
                         TextInEx,
                         &CurrentNotify->KeyData,
                         CurrentNotify->KeyNotificationFn,
                         &CurrentNotify->NotifyHandleList[Private->CurrentNumberOfExConsoles]
                         );
  }

  //
  // Add the new text-in device data structure into the Text Input Ex List.
  //
  Private->TextInExList[Private->CurrentNumberOfExConsoles] = TextInEx;
  Private->CurrentNumberOfExConsoles++;


  //
  // Synchronize toggle state for current input device.
  //
  if (Private->KeyToggleState) {
    SyncKeyToggleState (Private);
  }

  //
  // Extra CheckEvent added to reduce the double CheckEvent().
  //
  gBS->CheckEvent (TextInEx->WaitForKeyEx);

  return EFI_SUCCESS;
}

/**
  Remove Text Ex Device from Consplitter Text Input Ex list.

  @param  Private                  Text In Splitter pointer.
  @param  TextInEx                 Simple Text Ex protocol pointer.

  @retval EFI_SUCCESS              Simple Text Input Ex Device removed successfully.
  @retval EFI_NOT_FOUND            No Simple Text Input Ex Device found.

**/
EFI_STATUS
ConSplitterTextInExDeleteDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA         *Private,
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL     *TextInEx
  )
{
  UINTN Index;
  //
  // Remove the specified text-in device data structure from the Text Input Ex List,
  // and rearrange the remaining data structures in the Text In List.
  //
  for (Index = 0; Index < Private->CurrentNumberOfExConsoles; Index++) {
    if (Private->TextInExList[Index] == TextInEx) {
      for (; Index < Private->CurrentNumberOfExConsoles - 1; Index++) {
        Private->TextInExList[Index] = Private->TextInExList[Index + 1];
      }

      Private->CurrentNumberOfExConsoles--;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}



/**
  Sync toggle state for current input device.


  @param  [in]      Private             A pointer to the TEXT_IN_SPLITTER_PRIVATE_DATA structure.

  @retval EFI_SUCCESS                   The device state was set successfully.
  @retval EFI_DEVICE_ERROR              The device is not functioning correctly and could
                                        not have the setting adjusted.
  @retval EFI_UNSUPPORTED               The device does not have the ability to set its state.
  @retval EFI_INVALID_PARAMETER         KeyToggleState is NULL.

--*/
EFI_STATUS
EFIAPI
SyncKeyToggleState (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA         *Private
  )
{
  EFI_KEY_TOGGLE_STATE          KeyToggleState;
  EFI_STATUS                    Status;
  UINTN                         Index;

  //
  // Current SimpleTextInDevice index.
  //
  Index          = Private->CurrentNumberOfExConsoles - 1;
  KeyToggleState = Private->KeyToggleState;

  if (KeyToggleState) {
    //
    // Sync toggle state for current input device.
    //
    Status = Private->TextInExList[Index]->SetState (
                                             Private->TextInExList[Index],
                                             &KeyToggleState
                                             );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  return EFI_SUCCESS;
}

VOID
EFIAPI
PauseKeyProc (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  EFI_STATUS                         Status;
  EFI_INPUT_KEY                      Key;

  Key.ScanCode    = 0;
  Key.UnicodeChar = 0;
  Status = EFI_NOT_READY;
  while (Status != EFI_SUCCESS) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    if (Key.ScanCode == SCAN_PAUSE) {
      Status = EFI_NOT_READY;
    }
  }
  mPauseActive = FALSE;
  if (mPauseEvent != NULL) {
    gBS->CloseEvent(mPauseEvent);
    mPauseEvent = NULL;
  }
  return;
}

VOID
EFIAPI
EfiPauseKeyCallback (
  VOID
  )
{
  EFI_STATUS       Status;

  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  PauseKeyProc,
                  NULL,
                  &mPauseEvent
                  );
  if (EFI_ERROR (Status)) {
    return;
  }
  Status = gBS->SetTimer (
                  mPauseEvent,
                  TimerRelative,
                  PAUSE_TIMER_INTERVAL
                  );
  mPauseActive = TRUE;

  return;
}


/**
  Add Simple Pointer Device in Consplitter Simple Pointer list.

  @param  Private                  Text In Splitter pointer.
  @param  SimplePointer            Simple Pointer protocol pointer.

  @retval EFI_SUCCESS              Simple Pointer Device added successfully.
  @retval EFI_OUT_OF_RESOURCES     Could not grow the buffer size.

**/
EFI_STATUS
ConSplitterSimplePointerAddDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_POINTER_PROTOCOL     *SimplePointer
  )
{
  EFI_STATUS  Status;

  //
  // If the Simple Pointer List is full, enlarge it by calling ConSplitterGrowBuffer().
  //
  if (Private->CurrentNumberOfPointers >= Private->PointerListCount) {
    Status = ConSplitterGrowBuffer (
              sizeof (EFI_SIMPLE_POINTER_PROTOCOL *),
              &Private->PointerListCount,
              (VOID **) &Private->PointerList
              );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  //
  // Add the new text-in device data structure into the Simple Pointer List.
  //
  Private->PointerList[Private->CurrentNumberOfPointers] = SimplePointer;
  Private->CurrentNumberOfPointers++;

  return EFI_SUCCESS;
}


/**
  Remove Simple Pointer Device from Consplitter Simple Pointer list.

  @param  Private                  Text In Splitter pointer.
  @param  SimplePointer            Simple Pointer protocol pointer.

  @retval EFI_SUCCESS              Simple Pointer Device removed successfully.
  @retval EFI_NOT_FOUND            No Simple Pointer Device found.

**/
EFI_STATUS
ConSplitterSimplePointerDeleteDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_POINTER_PROTOCOL     *SimplePointer
  )
{
  UINTN Index;
  //
  // Remove the specified text-in device data structure from the Simple Pointer List,
  // and rearrange the remaining data structures in the Text In List.
  //
  for (Index = 0; Index < Private->CurrentNumberOfPointers; Index++) {
    if (Private->PointerList[Index] == SimplePointer) {
      for (; Index < Private->CurrentNumberOfPointers - 1; Index++) {
        Private->PointerList[Index] = Private->PointerList[Index + 1];
      }

      Private->CurrentNumberOfPointers--;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}


/**
  Add Absolute Pointer Device in Consplitter Absolute Pointer list.

  @param  Private                  Text In Splitter pointer.
  @param  AbsolutePointer          Absolute Pointer protocol pointer.

  @retval EFI_SUCCESS              Absolute Pointer Device added successfully.
  @retval EFI_OUT_OF_RESOURCES     Could not grow the buffer size.

**/
EFI_STATUS
ConSplitterAbsolutePointerAddDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_ABSOLUTE_POINTER_PROTOCOL     *AbsolutePointer
  )
{
  EFI_STATUS  Status;

  //
  // If the Absolute Pointer List is full, enlarge it by calling ConSplitterGrowBuffer().
  //
  if (Private->CurrentNumberOfAbsolutePointers >= Private->AbsolutePointerListCount) {
    Status = ConSplitterGrowBuffer (
              sizeof (EFI_ABSOLUTE_POINTER_PROTOCOL *),
              &Private->AbsolutePointerListCount,
              (VOID **) &Private->AbsolutePointerList
              );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  //
  // Add the new text-in device data structure into the Absolute Pointer List.
  //
  Private->AbsolutePointerList[Private->CurrentNumberOfAbsolutePointers] = AbsolutePointer;
  Private->CurrentNumberOfAbsolutePointers++;

  return EFI_SUCCESS;
}


/**
  Remove Absolute Pointer Device from Consplitter Absolute Pointer list.

  @param  Private                  Text In Splitter pointer.
  @param  AbsolutePointer          Absolute Pointer protocol pointer.

  @retval EFI_SUCCESS              Absolute Pointer Device removed successfully.
  @retval EFI_NOT_FOUND            No Absolute Pointer Device found.

**/
EFI_STATUS
ConSplitterAbsolutePointerDeleteDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_ABSOLUTE_POINTER_PROTOCOL     *AbsolutePointer
  )
{
  UINTN Index;
  //
  // Remove the specified text-in device data structure from the Absolute Pointer List,
  // and rearrange the remaining data structures from the Absolute Pointer List.
  //
  for (Index = 0; Index < Private->CurrentNumberOfAbsolutePointers; Index++) {
    if (Private->AbsolutePointerList[Index] == AbsolutePointer) {
      for (; Index < Private->CurrentNumberOfAbsolutePointers - 1; Index++) {
        Private->AbsolutePointerList[Index] = Private->AbsolutePointerList[Index + 1];
      }

      Private->CurrentNumberOfAbsolutePointers--;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Reallocate Text Out mode map.

  Allocate new buffer and copy original buffer into the new buffer.

  @param  Private                  Consplitter Text Out pointer.

  @retval EFI_SUCCESS              Buffer size has grown
  @retval EFI_OUT_OF_RESOURCES     Could not grow the buffer size.

**/
EFI_STATUS
ConSplitterGrowMapTable (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private
  )
{
  UINTN Size;
  UINTN NewSize;
  UINTN TotalSize;
  INT32 *TextOutModeMap;
  INT32 *OldTextOutModeMap;
  INT32 *SrcAddress;
  INT32 Index;

  NewSize           = Private->TextOutListCount * sizeof (INT32);
  OldTextOutModeMap = Private->TextOutModeMap;
  TotalSize         = NewSize * (Private->TextOutQueryDataCount);

  //
  // Allocate new buffer for Text Out List.
  //
  TextOutModeMap    = AllocatePool (TotalSize);
  if (TextOutModeMap == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetMem (TextOutModeMap, TotalSize, 0xFF);
  Private->TextOutModeMap = TextOutModeMap;

  //
  // If TextOutList has been enlarged, need to realloc the mode map table
  // The mode map table is regarded as a two dimension array.
  //
  //                         Old                    New
  //  0   ---------> TextOutListCount ----> TextOutListCount
  //  |   -------------------------------------------
  //  |  |                    |                      |
  //  |  |                    |                      |
  //  |  |                    |                      |
  //  |  |                    |                      |
  //  |  |                    |                      |
  // \/  |                    |                      |
  //      -------------------------------------------
  // QueryDataCount
  //
  if (OldTextOutModeMap != NULL) {

    Size        = Private->CurrentNumberOfConsoles * sizeof (INT32);
    Index       = 0;
    SrcAddress  = OldTextOutModeMap;

    //
    // Copy the old data to the new one
    //
    while (Index < Private->TextOutMode.MaxMode) {
      CopyMem (TextOutModeMap, SrcAddress, Size);
      TextOutModeMap += NewSize;
      SrcAddress += Size;
      Index++;
    }
    //
    // Free the old buffer
    //
    FreePool (OldTextOutModeMap);
  }

  return EFI_SUCCESS;
}


/**
  Add new device's output mode to console splitter's mode list.

  @param  Private               Text Out Splitter pointer
  @param  TextOut               Simple Text Output protocol pointer.

  @retval EFI_SUCCESS           Device added successfully.
  @retval EFI_OUT_OF_RESOURCES  Could not grow the buffer size.

**/
EFI_STATUS
ConSplitterAddOutputMode (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *TextOut
  )
{
  EFI_STATUS  Status;
  INT32       MaxMode;
  INT32       Mode;
  UINTN       Index;

  MaxMode                       = TextOut->Mode->MaxMode;
  Private->TextOutMode.MaxMode  = MaxMode;

  //
  // Grow the buffer if query data buffer is not large enough to
  // hold all the mode supported by the first console.
  //
  while (MaxMode > (INT32) Private->TextOutQueryDataCount) {
    Status = ConSplitterGrowBuffer (
              sizeof (TEXT_OUT_SPLITTER_QUERY_DATA),
              &Private->TextOutQueryDataCount,
              (VOID **) &Private->TextOutQueryData
              );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  //
  // Allocate buffer for the output mode map
  //
  Status = ConSplitterGrowMapTable (Private);
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // As the first textout device, directly add the mode in to QueryData
  // and at the same time record the mapping between QueryData and TextOut.
  //
  Mode  = 0;
  Index = 0;
  while (Mode < MaxMode) {
    Status = TextOut->QueryMode (
                  TextOut,
                  Mode,
                  &Private->TextOutQueryData[Mode].Columns,
                  &Private->TextOutQueryData[Mode].Rows
                  );
    //
    // If mode 1 (80x50) is not supported, make sure mode 1 in TextOutQueryData
    // is clear to 0x0.
    //
    if ((EFI_ERROR(Status)) && (Mode == 1)) {
      Private->TextOutQueryData[Mode].Columns = 0;
      Private->TextOutQueryData[Mode].Rows = 0;
    }
    Private->TextOutModeMap[Index] = Mode;
    Mode++;
    Index += Private->TextOutListCount;
  }

  return EFI_SUCCESS;
}

/**
  Reconstruct TextOutModeMap to get intersection of modes.

  This routine reconstruct TextOutModeMap to get the intersection
  of modes for all console out devices. Because EFI/UEFI spec require
  mode 0 is 80x25, mode 1 is 80x50, this routine will not check the
  intersection for mode 0 and mode 1.

  @param TextOutModeMap  Current text out mode map, begin with the mode 80x25
  @param NewlyAddedMap   New text out mode map, begin with the mode 80x25
  @param MapStepSize     Mode step size for one console device
  @param NewMapStepSize  New Mode step size for one console device
  @param MaxMode         IN: Current max text mode, OUT: Updated max text mode.
  @param CurrentMode     IN: Current text mode,     OUT: Updated current text mode.

**/
VOID
ConSplitterGetIntersection (
  IN     INT32                        *TextOutModeMap,
  IN     INT32                        *NewlyAddedMap,
  IN     UINTN                        MapStepSize,
  IN     UINTN                        NewMapStepSize,
  IN OUT INT32                        *MaxMode,
  IN OUT INT32                        *CurrentMode
  )
{
  INT32 Index;
  INT32 *CurrentMapEntry;
  INT32 *NextMapEntry;
  INT32 *NewMapEntry;
  INT32 CurrentMaxMode;
  INT32 Mode;

  //
  // According to EFI/UEFI spec, mode 0 and mode 1 have been reserved
  // for 80x25 and 80x50 in Simple Text Out protocol, so don't make intersection
  // for mode 0 and mode 1, mode number starts from 2.
  //
  Index           = 2;
  CurrentMapEntry = &TextOutModeMap[MapStepSize * 2];
  NextMapEntry    = CurrentMapEntry;
  NewMapEntry     = &NewlyAddedMap[NewMapStepSize * 2];

  CurrentMaxMode  = *MaxMode;
  Mode            = *CurrentMode;

  while (Index < CurrentMaxMode) {
    if (*NewMapEntry == -1) {
      //
      // This mode is not supported any more. Remove it. Special care
      // must be taken as this remove will also affect current mode;
      //
      if (Index == *CurrentMode) {
        Mode = -1;
      } else if (Index < *CurrentMode) {
        Mode--;
      }
      (*MaxMode)--;
    } else {
      if (CurrentMapEntry != NextMapEntry) {
        CopyMem (NextMapEntry, CurrentMapEntry, MapStepSize * sizeof (INT32));
      }

      NextMapEntry += MapStepSize;
    }

    CurrentMapEntry += MapStepSize;
    NewMapEntry     += NewMapStepSize;
    Index++;
  }

  *CurrentMode = Mode;

  return ;
}

/**
  Sync the device's output mode to console splitter's mode list.

  @param  Private               Text Out Splitter pointer.
  @param  TextOut               Simple Text Output protocol pointer.

**/
VOID
ConSplitterSyncOutputMode (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *TextOut
  )
{
  INT32                         CurrentMaxMode;
  INT32                         Mode;
  INT32                         Index;
  INT32                         *TextOutModeMap;
  INT32                         *MapTable;
  INT32                         QueryMode;
  TEXT_OUT_SPLITTER_QUERY_DATA  *TextOutQueryData;
  UINTN                         Rows;
  UINTN                         Columns;
  UINTN                         StepSize;
  EFI_STATUS                    Status;

  //
  // Must make sure that current mode won't change even if mode number changes
  //
  CurrentMaxMode    = Private->TextOutMode.MaxMode;
  TextOutModeMap    = Private->TextOutModeMap;
  StepSize          = Private->TextOutListCount;
  TextOutQueryData  = Private->TextOutQueryData;

  //
  // Query all the mode that the newly added TextOut supports
  //
  Mode      = 0;
  MapTable  = TextOutModeMap + Private->CurrentNumberOfConsoles;
  while (Mode < TextOut->Mode->MaxMode) {
    Status = TextOut->QueryMode (TextOut, Mode, &Columns, &Rows);

    if (EFI_ERROR(Status)) {
      if (Mode == 1) {
        //
        // If mode 1 (80x50) is not supported, make sure mode 1 in TextOutQueryData
        // is clear to 0x0.
        //
        MapTable[StepSize] = Mode;
        TextOutQueryData[Mode].Columns = 0;
        TextOutQueryData[Mode].Rows = 0;
      }
      Mode++;
      continue;
    }
    //
    // Search the intersection map and QueryData database to see if they intersects
    //
    Index = 0;
    while (Index < CurrentMaxMode) {
      QueryMode = *(TextOutModeMap + Index * StepSize);
      if ((TextOutQueryData[QueryMode].Rows == Rows) && (TextOutQueryData[QueryMode].Columns == Columns)) {
        MapTable[Index * StepSize] = Mode;
        break;
      }
      Index++;
    }
    Mode++;
  }
  //
  // Now search the TextOutModeMap table to find the intersection of supported
  // mode between ConSplitter and the newly added device.
  //
  ConSplitterGetIntersection (
    TextOutModeMap,
    MapTable,
    StepSize,
    StepSize,
    &Private->TextOutMode.MaxMode,
    &Private->TextOutMode.Mode
    );

  return ;
}


/**
  Sync output device between ConOut and StdErr output.

  @retval EFI_SUCCESS              Sync implemented successfully.
  @retval EFI_OUT_OF_RESOURCES     Could not grow the buffer size.

**/
EFI_STATUS
ConSplitterGetIntersectionBetweenConOutAndStrErr (
  VOID
  )
{
  UINTN                         ConOutNumOfConsoles;
  UINTN                         StdErrNumOfConsoles;
  TEXT_OUT_AND_GOP_DATA         *ConOutTextOutList;
  TEXT_OUT_AND_GOP_DATA         *StdErrTextOutList;
  UINTN                         Indexi;
  UINTN                         Indexj;
  UINTN                         ConOutRows;
  UINTN                         ConOutColumns;
  UINTN                         StdErrRows;
  UINTN                         StdErrColumns;
  INT32                         ConOutMaxMode;
  INT32                         StdErrMaxMode;
  INT32                         ConOutMode;
  INT32                         StdErrMode;
  INT32                         Mode;
  INT32                         Index;
  INT32                         *ConOutModeMap;
  INT32                         *StdErrModeMap;
  INT32                         *ConOutMapTable;
  INT32                         *StdErrMapTable;
  TEXT_OUT_SPLITTER_QUERY_DATA  *ConOutQueryData;
  TEXT_OUT_SPLITTER_QUERY_DATA  *StdErrQueryData;
  UINTN                         ConOutStepSize;
  UINTN                         StdErrStepSize;
  BOOLEAN                       FoundTheSameTextOut;
  UINTN                         ConOutMapTableSize;
  UINTN                         StdErrMapTableSize;

  ConOutNumOfConsoles = mConOut.CurrentNumberOfConsoles;
  StdErrNumOfConsoles = mStdErr.CurrentNumberOfConsoles;
  ConOutTextOutList   = mConOut.TextOutList;
  StdErrTextOutList   = mStdErr.TextOutList;

  Indexi              = 0;
  FoundTheSameTextOut = FALSE;
  while ((Indexi < ConOutNumOfConsoles) && (!FoundTheSameTextOut)) {
    Indexj = 0;
    while (Indexj < StdErrNumOfConsoles) {
      if (ConOutTextOutList->TextOut == StdErrTextOutList->TextOut) {
        FoundTheSameTextOut = TRUE;
        break;
      }

      Indexj++;
      StdErrTextOutList++;
    }

    Indexi++;
    ConOutTextOutList++;
  }

  if (!FoundTheSameTextOut) {
    return EFI_SUCCESS;
  }
  //
  // Must make sure that current mode won't change even if mode number changes
  //
  ConOutMaxMode     = mConOut.TextOutMode.MaxMode;
  ConOutModeMap     = mConOut.TextOutModeMap;
  ConOutStepSize    = mConOut.TextOutListCount;
  ConOutQueryData   = mConOut.TextOutQueryData;

  StdErrMaxMode     = mStdErr.TextOutMode.MaxMode;
  StdErrModeMap     = mStdErr.TextOutModeMap;
  StdErrStepSize    = mStdErr.TextOutListCount;
  StdErrQueryData   = mStdErr.TextOutQueryData;

  //
  // Allocate the map table and set the map table's index to -1.
  //
  ConOutMapTableSize  = ConOutMaxMode * sizeof (INT32);
  ConOutMapTable      = AllocateZeroPool (ConOutMapTableSize);
  if (ConOutMapTable == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetMem (ConOutMapTable, ConOutMapTableSize, 0xFF);

  StdErrMapTableSize  = StdErrMaxMode * sizeof (INT32);
  StdErrMapTable      = AllocateZeroPool (StdErrMapTableSize);
  if (StdErrMapTable == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetMem (StdErrMapTable, StdErrMapTableSize, 0xFF);

  //
  // Find the intersection of the two set of modes. If they actually intersect, the
  // correponding entry in the map table is set to 1.
  //
  Mode = 0;
  while (Mode < ConOutMaxMode) {
    //
    // Search the intersection map and QueryData database to see if they intersect
    //
    Index = 0;
    ConOutMode    = *(ConOutModeMap + Mode * ConOutStepSize);
    ConOutRows    = ConOutQueryData[ConOutMode].Rows;
    ConOutColumns = ConOutQueryData[ConOutMode].Columns;
    while (Index < StdErrMaxMode) {
      StdErrMode    = *(StdErrModeMap + Index * StdErrStepSize);
      StdErrRows    = StdErrQueryData[StdErrMode].Rows;
      StdErrColumns = StdErrQueryData[StdErrMode].Columns;
      if ((StdErrRows == ConOutRows) && (StdErrColumns == ConOutColumns)) {
        ConOutMapTable[Mode]  = 1;
        StdErrMapTable[Index] = 1;
        break;
      }

      Index++;
    }

    Mode++;
  }
  //
  // Now search the TextOutModeMap table to find the intersection of supported
  // mode between ConSplitter and the newly added device.
  //
  ConSplitterGetIntersection (
    ConOutModeMap,
    ConOutMapTable,
    mConOut.TextOutListCount,
    1,
    &(mConOut.TextOutMode.MaxMode),
    &(mConOut.TextOutMode.Mode)
    );

  if (mConOut.TextOutMode.Mode < 0) {
    mConOut.TextOut.SetMode (&(mConOut.TextOut), 0);
  }

  ConSplitterGetIntersection (
    StdErrModeMap,
    StdErrMapTable,
    mStdErr.TextOutListCount,
    1,
    &(mStdErr.TextOutMode.MaxMode),
    &(mStdErr.TextOutMode.Mode)
    );

  if (mStdErr.TextOutMode.Mode < 0) {
    mStdErr.TextOut.SetMode (&(mStdErr.TextOut), 0);
  }

  FreePool (ConOutMapTable);
  FreePool (StdErrMapTable);

  return EFI_SUCCESS;
}


/**
  Add Grahpics Output modes into Consplitter Text Out list.

  @param  Private               Text Out Splitter pointer.
  @param  GraphicsOutput        Graphics Output protocol pointer.
  @param  UgaDraw               UGA Draw protocol pointer.

  @retval EFI_SUCCESS           Output mode added successfully.
  @retval other                 Failed to add output mode.

**/
EFI_STATUS
ConSplitterAddGraphicsOutputMode (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput,
  IN  EFI_UGA_DRAW_PROTOCOL           *UgaDraw
  )
{
  EFI_STATUS                           Status;
  UINTN                                Index;
  UINTN                                CurrentIndex;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Mode;
  UINTN                                SizeOfInfo;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE    *CurrentGraphicsOutputMode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeBuffer;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *MatchedMode;
  UINTN                                NumberIndex;
  BOOLEAN                              Match;
  BOOLEAN                              AlreadyExist;
  UINT32                               UgaHorizontalResolution;
  UINT32                               UgaVerticalResolution;
  UINT32                               UgaColorDepth;
  UINT32                               UgaRefreshRate;

  ASSERT (GraphicsOutput != NULL || UgaDraw != NULL);

  CurrentGraphicsOutputMode = Private->GraphicsOutput.Mode;

  Index        = 0;
  CurrentIndex = 0;
  Status       = EFI_SUCCESS;

  if (Private->CurrentNumberOfUgaDraw != 0) {
    //
    // If any UGA device has already been added, then there is no need to
    // calculate intersection of display mode of different GOP/UGA device,
    // since only one display mode will be exported (i.e. user-defined mode)
    //
    goto Done;
  }

  if (GraphicsOutput != NULL) {
    if (Private->CurrentNumberOfGraphicsOutput == 0) {
        //
        // This is the first Graphics Output device added
        //
        CurrentGraphicsOutputMode->MaxMode = GraphicsOutput->Mode->MaxMode;
        CurrentGraphicsOutputMode->Mode = GraphicsOutput->Mode->Mode;
        CopyMem (CurrentGraphicsOutputMode->Info, GraphicsOutput->Mode->Info, GraphicsOutput->Mode->SizeOfInfo);
        CurrentGraphicsOutputMode->SizeOfInfo = GraphicsOutput->Mode->SizeOfInfo;
        CurrentGraphicsOutputMode->FrameBufferBase = GraphicsOutput->Mode->FrameBufferBase;
        CurrentGraphicsOutputMode->FrameBufferSize = GraphicsOutput->Mode->FrameBufferSize;

        //
        // Allocate resource for the private mode buffer
        //
        ModeBuffer = AllocatePool (sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION) * GraphicsOutput->Mode->MaxMode);
        if (ModeBuffer == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }
        FreePool (Private->GraphicsOutputModeBuffer);
        Private->GraphicsOutputModeBuffer = ModeBuffer;

        //
        // Store all supported display modes to the private mode buffer
        //
        Mode = ModeBuffer;
        for (Index = 0; Index < GraphicsOutput->Mode->MaxMode; Index++) {
          //
          // The Info buffer would be allocated by callee
          //
          Status = GraphicsOutput->QueryMode (GraphicsOutput, (UINT32) Index, &SizeOfInfo, &Info);
          if (EFI_ERROR (Status)) {
            return Status;
          }
          ASSERT ( SizeOfInfo <= sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
          CopyMem (Mode, Info, SizeOfInfo);
          Mode++;
          FreePool (Info);
        }
    } else {
      //
      // Check intersection of display mode
      //
      ModeBuffer = AllocatePool (sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION) * CurrentGraphicsOutputMode->MaxMode);
      if (ModeBuffer == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      MatchedMode = ModeBuffer;
      Mode = &Private->GraphicsOutputModeBuffer[0];
      for (Index = 0; Index < CurrentGraphicsOutputMode->MaxMode; Index++) {
        Match = FALSE;

        for (NumberIndex = 0; NumberIndex < GraphicsOutput->Mode->MaxMode; NumberIndex++) {
          //
          // The Info buffer would be allocated by callee
          //
          Status = GraphicsOutput->QueryMode (GraphicsOutput, (UINT32) NumberIndex, &SizeOfInfo, &Info);
          if (EFI_ERROR (Status)) {
            return Status;
          }
          if ((Info->HorizontalResolution == Mode->HorizontalResolution) &&
              (Info->VerticalResolution == Mode->VerticalResolution)) {
            //
            // If GOP device supports one mode in current mode buffer,
            // it will be added into matched mode buffer
            //
            Match = TRUE;
            FreePool (Info);
            break;
          }
          FreePool (Info);
        }

        if (Match) {
          AlreadyExist = FALSE;

          //
          // Check if GOP mode has been in the mode buffer, ModeBuffer = MatchedMode at begin.
          //
          for (Info = ModeBuffer; Info < MatchedMode; Info++) {
            if ((Info->HorizontalResolution == Mode->HorizontalResolution) &&
                (Info->VerticalResolution == Mode->VerticalResolution)) {
              AlreadyExist = TRUE;
              break;
            }
          }

          if (!AlreadyExist) {
            CopyMem (MatchedMode, Mode, sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));

            //
            // Physical frame buffer is no longer available when there are more than one physical GOP devices
            //
            MatchedMode->Version = 0;
            MatchedMode->PixelFormat = PixelBltOnly;
            ZeroMem (&MatchedMode->PixelInformation, sizeof (EFI_PIXEL_BITMASK));

            MatchedMode++;
          }
        }

        Mode++;
      }

      //
      // Drop the old mode buffer, assign it to a new one
      //
      FreePool (Private->GraphicsOutputModeBuffer);
      Private->GraphicsOutputModeBuffer = ModeBuffer;

      //
      // Physical frame buffer is no longer available when there are more than one physical GOP devices
      //
      CurrentGraphicsOutputMode->MaxMode = (UINT32) (((UINTN) MatchedMode - (UINTN) ModeBuffer) / sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
      CurrentGraphicsOutputMode->Info->PixelFormat = PixelBltOnly;
      ZeroMem (&CurrentGraphicsOutputMode->Info->PixelInformation, sizeof (EFI_PIXEL_BITMASK));
      CurrentGraphicsOutputMode->SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
      CurrentGraphicsOutputMode->FrameBufferBase = (EFI_PHYSICAL_ADDRESS) (UINTN) NULL;
      CurrentGraphicsOutputMode->FrameBufferSize = 0;
    }

  }

  if (UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport)) {
    //
    // Graphics console driver can ensure the same mode for all GOP devices
    // so we can get the current mode from this video device
    //
    UgaDraw->GetMode (
               UgaDraw,
               &UgaHorizontalResolution,
               &UgaVerticalResolution,
               &UgaColorDepth,
               &UgaRefreshRate
               );

    CurrentGraphicsOutputMode->MaxMode = 1;
    Info = CurrentGraphicsOutputMode->Info;
    Info->Version = 0;
    Info->HorizontalResolution                 = UgaHorizontalResolution;
    Info->VerticalResolution                   = UgaVerticalResolution;
    Info->PixelFormat                          = PixelBltOnly;
    Info->PixelsPerScanLine                    = UgaHorizontalResolution;
    CurrentGraphicsOutputMode->SizeOfInfo      = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    CurrentGraphicsOutputMode->FrameBufferBase = (EFI_PHYSICAL_ADDRESS) (UINTN) NULL;
    CurrentGraphicsOutputMode->FrameBufferSize = 0;

    //
    // Update the private mode buffer
    //
    CopyMem (&Private->GraphicsOutputModeBuffer[0], Info, sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));

    //
    // Only mode 0 is available to be set
    //
    CurrentIndex = 0;
  }

Done:

  if (GraphicsOutput != NULL) {
    Private->CurrentNumberOfGraphicsOutput++;
  }
  if (UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport)) {
    Private->CurrentNumberOfUgaDraw++;
  }

  //
  // Current mode number may need update now, so set it to an invalid mode number
  //
  CurrentGraphicsOutputMode->Mode = 0xffff;

  return Status;
}

/**
  Add Text Output Device in Consplitter Text Output list.

  @param  Private                  Text Out Splitter pointer.
  @param  TextOut                  Simple Text Output protocol pointer.
  @param  GraphicsOutput           Graphics Output protocol pointer.
  @param  UgaDraw                  UGA Draw protocol pointer.

  @retval EFI_SUCCESS              Text Output Device added successfully.
  @retval EFI_OUT_OF_RESOURCES     Could not grow the buffer size.

**/
EFI_STATUS
ConSplitterTextOutAddDevice (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *TextOut,
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL       *GraphicsOutput,
  IN  EFI_UGA_DRAW_PROTOCOL              *UgaDraw
  )
{
  EFI_STATUS                           Status;
  UINTN                                CurrentNumOfConsoles;
  INT32                                MaxMode;
  UINT32                               UgaHorizontalResolution;
  UINT32                               UgaVerticalResolution;
  UINT32                               UgaColorDepth;
  UINT32                               UgaRefreshRate;
  TEXT_OUT_AND_GOP_DATA                *TextAndGop;
  UINTN                                SizeOfInfo;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  EFI_STATUS                           DeviceStatus;
  UINTN                                OptimalTxtModeNum;
  BOOLEAN                              IsNewConsoleSupportCurTxtDimension;
  UINTN                                Columns;
  UINTN                                Rows;
  BOOLEAN                              SetNewMode;

  Status                = EFI_SUCCESS;
  CurrentNumOfConsoles  = Private->CurrentNumberOfConsoles;
  IsNewConsoleSupportCurTxtDimension = FALSE;
  Columns = 0;
  Rows = 0;
  SetNewMode = TRUE;

  //
  // If the Text Out List is full, enlarge it by calling ConSplitterGrowBuffer().
  //
  while (CurrentNumOfConsoles >= Private->TextOutListCount) {
    Status = ConSplitterGrowBuffer (
              sizeof (TEXT_OUT_AND_GOP_DATA),
              &Private->TextOutListCount,
              (VOID **) &Private->TextOutList
              );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // Also need to reallocate the TextOutModeMap table
    //
    Status = ConSplitterGrowMapTable (Private);
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  TextAndGop          = &Private->TextOutList[CurrentNumOfConsoles];

  TextAndGop->TextOut        = TextOut;
  TextAndGop->GraphicsOutput = GraphicsOutput;
  TextAndGop->UgaDraw        = UgaDraw;

  if (CurrentNumOfConsoles == 0) {
    //
    // Add the first device's output mode to console splitter's mode list
    //
    Status = ConSplitterAddOutputMode (Private, TextOut);
  } else {
    ConSplitterSyncOutputMode (Private, TextOut);
  }

  Private->CurrentNumberOfConsoles++;

  //
  // Scan both TextOutList, for the intersection TextOut device
  // maybe both ConOut and StdErr incorporate the same Text Out
  // device in them, thus the output of both should be synced.
  //
  ConSplitterGetIntersectionBetweenConOutAndStrErr ();

  MaxMode     = Private->TextOutMode.MaxMode;
  ASSERT (MaxMode >= 1);

  DeviceStatus = EFI_DEVICE_ERROR;
  if (FeaturePcdGet (PcdConOutGopSupport)) {
    //
    // If GOP is produced by Consplitter, this device display mode will be added into Graphics Ouput modes.
    //
    if ((GraphicsOutput != NULL) || (UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport))) {
      DeviceStatus = ConSplitterAddGraphicsOutputMode (Private, GraphicsOutput, UgaDraw);
    }
  }

  if (FeaturePcdGet (PcdConOutUgaSupport)) {
    //
    // If UGA is produced by Consplitter
    //
    if (GraphicsOutput != NULL) {
      Status = GraphicsOutput->QueryMode (GraphicsOutput, GraphicsOutput->Mode->Mode, &SizeOfInfo, &Info);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      ASSERT ( SizeOfInfo <= sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));

      UgaHorizontalResolution = Info->HorizontalResolution;
      UgaVerticalResolution   = Info->VerticalResolution;

      FreePool (Info);

    } else if (UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport)) {
      Status = UgaDraw->GetMode (
                    UgaDraw,
                    &UgaHorizontalResolution,
                    &UgaVerticalResolution,
                    &UgaColorDepth,
                    &UgaRefreshRate
                    );
      if (!EFI_ERROR (Status) && EFI_ERROR (DeviceStatus)) {
        //
        // if GetMode is successfully and UGA device hasn't been set, set it
        //
        Status = ConSplitterUgaDrawSetMode (
                    &Private->UgaDraw,
                    UgaHorizontalResolution,
                    UgaVerticalResolution,
                    UgaColorDepth,
                    UgaRefreshRate
                    );
      }
      //
      // If GetMode/SetMode is failed, set to 800x600 mode
      //
      if(EFI_ERROR (Status)) {
        Status = ConSplitterUgaDrawSetMode (
                    &Private->UgaDraw,
                    800,
                    600,
                    32,
                    60
                    );
      }
    }
  }

  //
  // After adding new console device, all existing console devices should be
  // synced to the current highest text dimension.
  //
  Status = FindOptimalTextMode (Private, &OptimalTxtModeNum);
  if (EFI_ERROR (Status)) {
    OptimalTxtModeNum = 0;
    Status = EFI_SUCCESS;
  }
  //
  // check if new highest text dimension from new intersections is able to support current highest text dimension.
  //
  Private->TextOut.QueryMode (&Private->TextOut, OptimalTxtModeNum, &Columns, &Rows);
  if ((Private->DevNullColumns == Columns) && (Private->DevNullRows == Rows)) {
    SyncNewConsole (Private, TextOut);
    OutputTextContentsToNewConsoleDev (Private, TextOut);
    //
    // after caclulating new intersections, need to sync virtual console.
    //
    SyncVirtualConsole (Private);
  } else {
    Status = Private->TextOut.SetMode (&Private->TextOut, OptimalTxtModeNum);
    ConSplitterTextOutSetAttribute (&Private->TextOut, EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK));
  }

  return Status;
}


/**
  Remove Text Out Device in Consplitter Text Out list.

  @param  Private                  Text Out Splitter pointer.
  @param  TextOut                  Simple Text Output Pointer protocol pointer.

  @retval EFI_SUCCESS              Text Out Device removed successfully.
  @retval EFI_NOT_FOUND            No Text Out Device found.

**/
EFI_STATUS
ConSplitterTextOutDeleteDevice (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *TextOut
  )
{
  INT32                 Index;
  UINTN                 CurrentNumOfConsoles;
  TEXT_OUT_AND_GOP_DATA *TextOutList;
  EFI_STATUS            Status;
  VOID                  *DummyInterface;
  UINTN                 NumberOfHandles;
  EFI_HANDLE            *HandleBuffer;
  EFI_HANDLE            SignalHandle;

  //
  // init locals
  //
  DummyInterface = NULL;
  NumberOfHandles = 0;
  HandleBuffer = NULL;
  SignalHandle = NULL;

  //
  // Remove the specified text-out device data structure from the Text out List,
  // and rearrange the remaining data structures in the Text out List.
  //
  CurrentNumOfConsoles  = Private->CurrentNumberOfConsoles;
  Index                 = (INT32) CurrentNumOfConsoles - 1;
  TextOutList           = Private->TextOutList;
  while (Index >= 0) {
    if (TextOutList->TextOut == TextOut) {
      if (TextOutList->UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport)) {
        Private->CurrentNumberOfUgaDraw--;
      }
      if (TextOutList->GraphicsOutput != NULL) {
        Private->CurrentNumberOfGraphicsOutput--;
      }
      CopyMem (TextOutList, TextOutList + 1, sizeof (TEXT_OUT_AND_GOP_DATA) * Index);
      CurrentNumOfConsoles--;
      break;
    }

    Index--;
    TextOutList++;
  }
  //
  // The specified TextOut is not managed by the ConSplitter driver
  //
  if (Index < 0) {
    return EFI_NOT_FOUND;
  }

  if (CurrentNumOfConsoles == 0) {
    Private->CurrentNumberOfConsoles = 0;
    Private->CurrentNumberOfGraphicsOutput = 0;
    return EFI_SUCCESS;
  }
  //
  // Max Mode is realy an intersection of the QueryMode command to all
  // devices. So we must copy the QueryMode of the first device to
  // QueryData.
  //
  ZeroMem (
    Private->TextOutQueryData,
    Private->TextOutQueryDataCount * sizeof (TEXT_OUT_SPLITTER_QUERY_DATA)
    );

  FreePool (Private->TextOutModeMap);
  Private->TextOutModeMap = NULL;
  TextOutList             = Private->TextOutList;

  //
  // Add the first TextOut to the QueryData array and ModeMap table
  //
  Private->CurrentNumberOfConsoles = 0;
  Status = ConSplitterAddOutputMode (Private, TextOutList->TextOut);
  Private->CurrentNumberOfGraphicsOutput = 0;
  if ((TextOutList->GraphicsOutput != NULL) || (TextOutList->UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport))) {
    ConSplitterAddGraphicsOutputMode (Private, TextOutList->GraphicsOutput,  TextOutList->UgaDraw);
  }

  //
  // Now add one by one
  //
  Index = 1;
  Private->CurrentNumberOfConsoles = 1;
  TextOutList++;
  while ((UINTN) Index < CurrentNumOfConsoles) {
    ConSplitterSyncOutputMode (Private, TextOutList->TextOut);
    if ((TextOutList->GraphicsOutput != NULL) || (TextOutList->UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport))) {
      ConSplitterAddGraphicsOutputMode (Private, TextOutList->GraphicsOutput,  TextOutList->UgaDraw);
    }
    Index++;
    Private->CurrentNumberOfConsoles++;
    TextOutList++;
  }
  //
  // after caclulating new intersections, need to sync virtual console.
  //
  SyncVirtualConsole (Private);

  ConSplitterGetIntersectionBetweenConOutAndStrErr ();

  return Status;
}


/**
  Reset the input device and optionaly run diagnostics

  @param  This                     Protocol instance pointer.
  @param  ExtendedVerification     Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS              The device was reset.
  @retval EFI_DEVICE_ERROR         The device is not functioning properly and could
                                   not be reset.

**/
EFI_STATUS
EFIAPI
ConSplitterTextInReset (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  IN  BOOLEAN                         ExtendedVerification
  )
{
  EFI_STATUS                    Status;
  EFI_STATUS                    ReturnStatus;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private                       = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  Private->KeyEventSignalState  = FALSE;

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = Private->TextInList[Index]->Reset (
                                          Private->TextInList[Index],
                                          ExtendedVerification
                                          );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  return ReturnStatus;
}


/**
  Reads the next keystroke from the input device. The WaitForKey Event can
  be used to test for existance of a keystroke via WaitForEvent () call.

  @param  Private                  Protocol instance pointer.
  @param  Key                      Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS              The keystroke information was returned.
  @retval EFI_NOT_READY            There was no keystroke data availiable.
  @retval EFI_DEVICE_ERROR         The keydtroke information was not returned due
                                   to hardware errors.

**/
EFI_STATUS
EFIAPI
ConSplitterTextInPrivateReadKeyStroke (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  OUT EFI_INPUT_KEY                   *Key
  )
{
  EFI_STATUS    Status;
  UINTN         Index;
  EFI_INPUT_KEY CurrentKey;

  Key->UnicodeChar  = 0;
  Key->ScanCode     = SCAN_NULL;

  //
  // if no physical console input device exists, return EFI_NOT_READY;
  // if any physical console input device has key input,
  // return the key and EFI_SUCCESS.
  //
  for (Index = 0; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = Private->TextInList[Index]->ReadKeyStroke (
                                          Private->TextInList[Index],
                                          &CurrentKey
                                          );
    if (!EFI_ERROR (Status)) {
      *Key = CurrentKey;
      if (FeaturePcdGet (PcdUefiPauseKeyFunctionSupport)) {
        if (Key->ScanCode == SCAN_PAUSE && Key->UnicodeChar  == CHAR_NULL && !mPauseActive) {
          EfiPauseKeyCallback ();
          Key->UnicodeChar = CHAR_NULL;
          Key->ScanCode    = SCAN_NULL;
          return EFI_NOT_READY;
        }
      }
      return Status;
    }
  }

  return EFI_NOT_READY;
}


/**
  Reads the next keystroke from the input device. The WaitForKey Event can
  be used to test for existance of a keystroke via WaitForEvent () call.

  @param  This                     Protocol instance pointer.
  @param  Key                      Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS              The keystroke information was returned.
  @retval EFI_NOT_READY            There was no keystroke data availiable.
  @retval EFI_DEVICE_ERROR         The keydtroke information was not returned due
                                   to hardware errors.

**/
EFI_STATUS
EFIAPI
ConSplitterTextInReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                   *Key
  )
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;

  Private = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  Private->KeyEventSignalState = FALSE;

  return ConSplitterTextInPrivateReadKeyStroke (Private, Key);
}


/**
  This event aggregates all the events of the ConIn devices in the spliter.

  If any events of physical ConIn devices are signaled, signal the ConIn
  spliter event. This will cause the calling code to call
  ConSplitterTextInReadKeyStroke ().

  @param  Event                    The Event assoicated with callback.
  @param  Context                  Context registered when Event was created.

**/
VOID
EFIAPI
ConSplitterTextInWaitForKey (
  IN  EFI_EVENT                       Event,
  IN  VOID                            *Context
  )
{
  EFI_STATUS                    Status;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private = (TEXT_IN_SPLITTER_PRIVATE_DATA *) Context;

  if (Private->KeyEventSignalState) {
    //
    // If KeyEventSignalState is flagged before, and not cleared by Reset() or ReadKeyStroke()
    //
    gBS->SignalEvent (Event);
    return ;
  }

  //
  // If any physical console input device has key input, signal the event.
  //
  for (Index = 0; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = gBS->CheckEvent (Private->TextInList[Index]->WaitForKey);
    if (!EFI_ERROR (Status)) {
      gBS->SignalEvent (Event);
      Private->KeyEventSignalState = TRUE;
    }
  }
}



/**
  Test if the key has been registered on input device.

  @param  RegsiteredData           A pointer to a buffer that is filled in with the
                                   keystroke state data for the key that was
                                   registered.
  @param  InputData                A pointer to a buffer that is filled in with the
                                   keystroke state data for the key that was
                                   pressed.

  @retval TRUE                     Key be pressed matches a registered key.
  @retval FLASE                    Match failed.

**/
BOOLEAN
IsKeyRegistered (
  IN EFI_KEY_DATA  *RegsiteredData,
  IN EFI_KEY_DATA  *InputData
  )
{
  ASSERT (RegsiteredData != NULL && InputData != NULL);

  if ((RegsiteredData->Key.ScanCode    != InputData->Key.ScanCode) ||
      (RegsiteredData->Key.UnicodeChar != InputData->Key.UnicodeChar)) {
    return FALSE;
  }

  //
  // Assume KeyShiftState/KeyToggleState = 0 in Registered key data means these state could be ignored.
  //
  if (RegsiteredData->KeyState.KeyShiftState != 0 &&
      RegsiteredData->KeyState.KeyShiftState != InputData->KeyState.KeyShiftState) {
    return FALSE;
  }
  if (RegsiteredData->KeyState.KeyToggleState != 0 &&
      RegsiteredData->KeyState.KeyToggleState != InputData->KeyState.KeyToggleState) {
    return FALSE;
  }

  return TRUE;

}


/**
  Reset the input device and optionaly run diagnostics

  @param  This                     Protocol instance pointer.
  @param  ExtendedVerification     Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS              The device was reset.
  @retval EFI_DEVICE_ERROR         The device is not functioning properly and could
                                   not be reset.

**/
EFI_STATUS
EFIAPI
ConSplitterTextInResetEx (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN BOOLEAN                            ExtendedVerification
  )
{
  EFI_STATUS                    Status;
  EFI_STATUS                    ReturnStatus;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private                       = TEXT_IN_EX_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  Private->KeyEventSignalState  = FALSE;

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfExConsoles; Index++) {
    Status = Private->TextInExList[Index]->Reset (
                                             Private->TextInExList[Index],
                                             ExtendedVerification
                                             );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  return ReturnStatus;

}


/**
  Reads the next keystroke from the input device. The WaitForKey Event can
  be used to test for existance of a keystroke via WaitForEvent () call.

  @param  This                     Protocol instance pointer.
  @param  KeyData                  A pointer to a buffer that is filled in with the
                                   keystroke state data for the key that was
                                   pressed.

  @retval EFI_SUCCESS              The keystroke information was returned.
  @retval EFI_NOT_READY            There was no keystroke data availiable.
  @retval EFI_DEVICE_ERROR         The keystroke information was not returned due
                                   to hardware errors.
  @retval EFI_INVALID_PARAMETER    KeyData is NULL.

**/
EFI_STATUS
EFIAPI
ConSplitterTextInReadKeyStrokeEx (
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  OUT EFI_KEY_DATA                      *KeyData
  )
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  EFI_STATUS                    Status;
  UINTN                         Index;
  EFI_KEY_DATA                  CurrentKeyData;


  if (KeyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = TEXT_IN_EX_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  Private->KeyEventSignalState = FALSE;

  KeyData->Key.UnicodeChar  = 0;
  KeyData->Key.ScanCode     = SCAN_NULL;

  //
  // if no physical console input device exists, return EFI_NOT_READY;
  // if any physical console input device has key input,
  // return the key and EFI_SUCCESS.
  //
  for (Index = 0; Index < Private->CurrentNumberOfExConsoles; Index++) {
    Status = Private->TextInExList[Index]->ReadKeyStrokeEx (
                                          Private->TextInExList[Index],
                                          &CurrentKeyData
                                          );
    if (!EFI_ERROR (Status)) {
      CopyMem (KeyData, &CurrentKeyData, sizeof (CurrentKeyData));
      if (mSaveKbcScanCode) {
        SaveKBCScanCode (KeyData);
      }
      if (FeaturePcdGet (PcdUefiPauseKeyFunctionSupport)) {
        if (KeyData->Key.ScanCode == SCAN_PAUSE &&
            KeyData->Key.UnicodeChar == CHAR_NULL &&
            KeyData->KeyState.KeyShiftState == EFI_SHIFT_STATE_VALID &&
            !mPauseActive) {
          EfiPauseKeyCallback ();
          KeyData->Key.UnicodeChar = CHAR_NULL;
          KeyData->Key.ScanCode    = SCAN_NULL;
          return EFI_NOT_READY;
        }
      }
      return Status;
    }
  }

  return EFI_NOT_READY;
}


/**
  Set certain state for the input device.

  @param  This                     Protocol instance pointer.
  @param  KeyToggleState           A pointer to the EFI_KEY_TOGGLE_STATE to set the
                                   state for the input device.

  @retval EFI_SUCCESS              The device state was set successfully.
  @retval EFI_DEVICE_ERROR         The device is not functioning correctly and
                                   could not have the setting adjusted.
  @retval EFI_UNSUPPORTED          The device does not have the ability to set its
                                   state.
  @retval EFI_INVALID_PARAMETER    KeyToggleState is NULL.

**/
EFI_STATUS
EFIAPI
ConSplitterTextInSetState (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_KEY_TOGGLE_STATE               *KeyToggleState
  )
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  EFI_STATUS                    Status;
  UINTN                         Index;

  if (KeyToggleState == NULL || (UINTN)This == (UINTN)KeyToggleState) {
    return EFI_INVALID_PARAMETER;
  }

  Private = TEXT_IN_EX_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // if no physical console input device exists, return EFI_SUCCESS;
  // otherwise return the status of setting state of physical console input device
  //
  for (Index = 0; Index < Private->CurrentNumberOfExConsoles; Index++) {
    Status = Private->TextInExList[Index]->SetState (
                                             Private->TextInExList[Index],
                                             KeyToggleState
                                             );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if ((*KeyToggleState & EFI_TOGGLE_STATE_VALID) == EFI_TOGGLE_STATE_VALID) {
    Private->KeyToggleState = *KeyToggleState;
  }

  return EFI_SUCCESS;

}


/**
  Register a notification function for a particular keystroke for the input device.

  @param  This                     Protocol instance pointer.
  @param  KeyData                  A pointer to a buffer that is filled in with the
                                   keystroke information data for the key that was
                                   pressed.
  @param  KeyNotificationFunction  Points to the function to be called when the key
                                   sequence is typed specified by KeyData.
  @param  NotifyHandle             Points to the unique handle assigned to the
                                   registered notification.

  @retval EFI_SUCCESS              The notification function was registered
                                   successfully.
  @retval EFI_OUT_OF_RESOURCES     Unable to allocate resources for necesssary data
                                   structures.
  @retval EFI_INVALID_PARAMETER    KeyData or KeyNotificationFunction or NotifyHandle is NULL.

**/
EFI_STATUS
EFIAPI
ConSplitterTextInRegisterKeyNotify (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_KEY_DATA                       *KeyData,
  IN EFI_KEY_NOTIFY_FUNCTION            KeyNotificationFunction,
  OUT EFI_HANDLE                        *NotifyHandle
  )
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  EFI_STATUS                    Status;
  UINTN                         Index;
  TEXT_IN_EX_SPLITTER_NOTIFY    *NewNotify;
  LIST_ENTRY                    *Link;
  TEXT_IN_EX_SPLITTER_NOTIFY    *CurrentNotify;


  if (KeyData == NULL || NotifyHandle == NULL || KeyNotificationFunction == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = TEXT_IN_EX_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // Return EFI_SUCCESS if the (KeyData, NotificationFunction) is already registered.
  //
  for (Link = Private->NotifyList.ForwardLink; Link != &Private->NotifyList; Link = Link->ForwardLink) {
    CurrentNotify = TEXT_IN_EX_SPLITTER_NOTIFY_FROM_THIS (Link);
    if (IsKeyRegistered (&CurrentNotify->KeyData, KeyData)) {
      if (CurrentNotify->KeyNotificationFn == KeyNotificationFunction) {
        *NotifyHandle = CurrentNotify->NotifyHandle;
        return EFI_SUCCESS;
      }
    }
  }

  //
  // Allocate resource to save the notification function
  //
  NewNotify = (TEXT_IN_EX_SPLITTER_NOTIFY *) AllocateZeroPool (sizeof (TEXT_IN_EX_SPLITTER_NOTIFY));
  if (NewNotify == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  NewNotify->NotifyHandleList = (EFI_HANDLE *) AllocateZeroPool (sizeof (EFI_HANDLE) * Private->TextInExListCount);
  if (NewNotify->NotifyHandleList == NULL) {
    gBS->FreePool (NewNotify);
    return EFI_OUT_OF_RESOURCES;
  }
  NewNotify->Signature         = TEXT_IN_EX_SPLITTER_NOTIFY_SIGNATURE;
  NewNotify->KeyNotificationFn = KeyNotificationFunction;
  NewNotify->NotifyHandle      = (EFI_HANDLE) NewNotify;
  CopyMem (&NewNotify->KeyData, KeyData, sizeof (EFI_KEY_DATA));

  //
  // Return the wrong status of registering key notify of
  // physical console input device if meet problems
  //
  for (Index = 0; Index < Private->CurrentNumberOfExConsoles; Index++) {
    Status = Private->TextInExList[Index]->RegisterKeyNotify (
                                             Private->TextInExList[Index],
                                             KeyData,
                                             KeyNotificationFunction,
                                             &NewNotify->NotifyHandleList[Index]
                                             );
  }

  InsertTailList (&mConIn.NotifyList, &NewNotify->NotifyEntry);

  *NotifyHandle                = NewNotify->NotifyHandle;

  return EFI_SUCCESS;

}


/**
  Remove a registered notification function from a particular keystroke.

  @param  This                     Protocol instance pointer.
  @param  NotificationHandle       The handle of the notification function being
                                   unregistered.

  @retval EFI_SUCCESS              The notification function was unregistered
                                   successfully.
  @retval EFI_INVALID_PARAMETER    The NotificationHandle is invalid.

**/
EFI_STATUS
EFIAPI
ConSplitterTextInUnregisterKeyNotify (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_HANDLE                         NotificationHandle
  )
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  EFI_STATUS                    Status;
  UINTN                         Index;
  TEXT_IN_EX_SPLITTER_NOTIFY    *CurrentNotify;
  LIST_ENTRY                    *Link;

  if (NotificationHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (((TEXT_IN_EX_SPLITTER_NOTIFY *) NotificationHandle)->Signature != TEXT_IN_EX_SPLITTER_NOTIFY_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  Private = TEXT_IN_EX_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  for (Link = Private->NotifyList.ForwardLink; Link != &Private->NotifyList; Link = Link->ForwardLink) {
    CurrentNotify = TEXT_IN_EX_SPLITTER_NOTIFY_FROM_THIS (Link);
    if (CurrentNotify->NotifyHandle == NotificationHandle) {
      for (Index = 0; Index < Private->CurrentNumberOfExConsoles; Index++) {
        Status = Private->TextInExList[Index]->UnregisterKeyNotify (
                                                 Private->TextInExList[Index],
                                                 CurrentNotify->NotifyHandleList[Index]
                                                 );
      }
      RemoveEntryList (&CurrentNotify->NotifyEntry);

      if (CurrentNotify->NotifyHandleList != NULL) {
        gBS->FreePool (CurrentNotify->NotifyHandleList);
      }
      gBS->FreePool (CurrentNotify);
      return EFI_SUCCESS;
    }
  }

  //
  // NotificationHandle is not found in database
  //
  return EFI_INVALID_PARAMETER;
}


/**
  Reset the input device and optionaly run diagnostics

  @param  This                     Protocol instance pointer.
  @param  ExtendedVerification     Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS              The device was reset.
  @retval EFI_DEVICE_ERROR         The device is not functioning properly and could
                                   not be reset.

**/
EFI_STATUS
EFIAPI
ConSplitterSimplePointerReset (
  IN  EFI_SIMPLE_POINTER_PROTOCOL     *This,
  IN  BOOLEAN                         ExtendedVerification
  )
{
  EFI_STATUS                    Status;
  EFI_STATUS                    ReturnStatus;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private                         = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_SIMPLE_POINTER_THIS (This);

  Private->InputEventSignalState  = FALSE;

  if (Private->CurrentNumberOfPointers == 0) {
    return EFI_SUCCESS;
  }
  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfPointers; Index++) {
    Status = Private->PointerList[Index]->Reset (
                                            Private->PointerList[Index],
                                            ExtendedVerification
                                            );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  return ReturnStatus;
}


/**
  Reads the next keystroke from the input device. The WaitForKey Event can
  be used to test for existance of a keystroke via WaitForEvent () call.

  @param  Private                  Protocol instance pointer.
  @param  State                    The state information of simple pointer device.

  @retval EFI_SUCCESS              The keystroke information was returned.
  @retval EFI_NOT_READY            There was no keystroke data availiable.
  @retval EFI_DEVICE_ERROR         The keydtroke information was not returned due
                                   to hardware errors.

**/
EFI_STATUS
EFIAPI
ConSplitterSimplePointerPrivateGetState (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN OUT EFI_SIMPLE_POINTER_STATE     *State
  )
{
  EFI_STATUS                Status;
  EFI_STATUS                ReturnStatus;
  UINTN                     Index;
  EFI_SIMPLE_POINTER_STATE  CurrentState;

  State->RelativeMovementX  = 0;
  State->RelativeMovementY  = 0;
  State->RelativeMovementZ  = 0;
  State->LeftButton         = FALSE;
  State->RightButton        = FALSE;

  //
  // if no physical console input device exists, return EFI_NOT_READY;
  // if any physical console input device has key input,
  // return the key and EFI_SUCCESS.
  //
  ReturnStatus = EFI_NOT_READY;
  for (Index = 0; Index < Private->CurrentNumberOfPointers; Index++) {

    Status = Private->PointerList[Index]->GetState (
                                            Private->PointerList[Index],
                                            &CurrentState
                                            );
    if (!EFI_ERROR (Status)) {
      if (ReturnStatus == EFI_NOT_READY) {
        ReturnStatus = EFI_SUCCESS;
      }

      if (CurrentState.LeftButton) {
        State->LeftButton = TRUE;
      }

      if (CurrentState.RightButton) {
        State->RightButton = TRUE;
      }

      if (CurrentState.RelativeMovementX != 0 && Private->PointerList[Index]->Mode->ResolutionX != 0) {
        State->RelativeMovementX += (CurrentState.RelativeMovementX * (INT32) Private->SimplePointerMode.ResolutionX) / (INT32) Private->PointerList[Index]->Mode->ResolutionX;
      }

      if (CurrentState.RelativeMovementY != 0 && Private->PointerList[Index]->Mode->ResolutionY != 0) {
        State->RelativeMovementY += (CurrentState.RelativeMovementY * (INT32) Private->SimplePointerMode.ResolutionY) / (INT32) Private->PointerList[Index]->Mode->ResolutionY;
      }

      if (CurrentState.RelativeMovementZ != 0 && Private->PointerList[Index]->Mode->ResolutionZ != 0) {
        State->RelativeMovementZ += (CurrentState.RelativeMovementZ * (INT32) Private->SimplePointerMode.ResolutionZ) / (INT32) Private->PointerList[Index]->Mode->ResolutionZ;
      }
    } else if (Status == EFI_DEVICE_ERROR) {
      ReturnStatus = EFI_DEVICE_ERROR;
    }
  }

  return ReturnStatus;
}


/**
  Reads the next keystroke from the input device. The WaitForKey Event can
  be used to test for existance of a keystroke via WaitForEvent () call.

  @param  This                     A pointer to protocol instance.
  @param  State                    A pointer to state information on the pointer device

  @retval EFI_SUCCESS              The keystroke information was returned in State.
  @retval EFI_NOT_READY            There was no keystroke data availiable.
  @retval EFI_DEVICE_ERROR         The keydtroke information was not returned due
                                   to hardware errors.

**/
EFI_STATUS
EFIAPI
ConSplitterSimplePointerGetState (
  IN  EFI_SIMPLE_POINTER_PROTOCOL     *This,
  IN OUT EFI_SIMPLE_POINTER_STATE     *State
  )
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;

  Private = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_SIMPLE_POINTER_THIS (This);

  Private->InputEventSignalState = FALSE;

  return ConSplitterSimplePointerPrivateGetState (Private, State);
}


/**
  This event agregates all the events of the ConIn devices in the spliter.
  If any events of physical ConIn devices are signaled, signal the ConIn
  spliter event. This will cause the calling code to call
  ConSplitterTextInReadKeyStroke ().

  @param  Event                    The Event assoicated with callback.
  @param  Context                  Context registered when Event was created.

**/
VOID
EFIAPI
ConSplitterSimplePointerWaitForInput (
  IN  EFI_EVENT                       Event,
  IN  VOID                            *Context
  )
{
  EFI_STATUS                    Status;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private = (TEXT_IN_SPLITTER_PRIVATE_DATA *) Context;

  //
  // if InputEventSignalState is flagged before, and not cleared by Reset() or ReadKeyStroke()
  //
  if (Private->InputEventSignalState) {
    gBS->SignalEvent (Event);
    return ;
  }
  //
  // if any physical console input device has key input, signal the event.
  //
  for (Index = 0; Index < Private->CurrentNumberOfPointers; Index++) {
    Status = gBS->CheckEvent (Private->PointerList[Index]->WaitForInput);
    if (!EFI_ERROR (Status)) {
      gBS->SignalEvent (Event);
      Private->InputEventSignalState = TRUE;
    }
  }
}

/**
  Resets the pointer device hardware.

  @param  This                     Protocol instance pointer.
  @param  ExtendedVerification     Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS              The device was reset.
  @retval EFI_DEVICE_ERROR         The device is not functioning correctly and
                                   could not be reset.

**/
EFI_STATUS
EFIAPI
ConSplitterAbsolutePointerReset (
  IN EFI_ABSOLUTE_POINTER_PROTOCOL   *This,
  IN BOOLEAN                         ExtendedVerification
  )
{
  EFI_STATUS                    Status;
  EFI_STATUS                    ReturnStatus;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_ABSOLUTE_POINTER_THIS (This);

  Private->AbsoluteInputEventSignalState = FALSE;

  if (Private->CurrentNumberOfAbsolutePointers == 0) {
    return EFI_SUCCESS;
  }
  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfAbsolutePointers; Index++) {
    Status = Private->AbsolutePointerList[Index]->Reset (
                                                    Private->AbsolutePointerList[Index],
                                                    ExtendedVerification
                                                    );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  return ReturnStatus;
}


/**
  Retrieves the current state of a pointer device.

  @param  This                     Protocol instance pointer.
  @param  State                    A pointer to the state information on the
                                   pointer device.

  @retval EFI_SUCCESS              The state of the pointer device was returned in
                                   State..
  @retval EFI_NOT_READY            The state of the pointer device has not changed
                                   since the last call to GetState().
  @retval EFI_DEVICE_ERROR         A device error occurred while attempting to
                                   retrieve the pointer device's current state.

**/
EFI_STATUS
EFIAPI
ConSplitterAbsolutePointerGetState (
  IN EFI_ABSOLUTE_POINTER_PROTOCOL   *This,
  IN OUT EFI_ABSOLUTE_POINTER_STATE  *State
  )
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  EFI_STATUS                    Status;
  EFI_STATUS                    ReturnStatus;
  UINTN                         Index;
  EFI_ABSOLUTE_POINTER_STATE    CurrentState;
  UINT64                        Factor;


  Private = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_ABSOLUTE_POINTER_THIS (This);

  Private->AbsoluteInputEventSignalState = FALSE;

  State->CurrentX                        = 0;
  State->CurrentY                        = 0;
  State->CurrentZ                        = 0;
  State->ActiveButtons                   = 0;

  //
  // if no physical pointer device exists, return EFI_NOT_READY;
  // if any physical pointer device has changed state,
  // return the state and EFI_SUCCESS.
  //
  ReturnStatus = EFI_NOT_READY;
  for (Index = 0; Index < Private->CurrentNumberOfAbsolutePointers; Index++) {

    Status = Private->AbsolutePointerList[Index]->GetState (
                                                    Private->AbsolutePointerList[Index],
                                                    &CurrentState
                                                    );
    if (!EFI_ERROR (Status)) {
      if (ReturnStatus == EFI_NOT_READY) {
        ReturnStatus = EFI_SUCCESS;
      }

      State->ActiveButtons = CurrentState.ActiveButtons;

      if (!(Private->AbsolutePointerMode.AbsoluteMinX == 0 && Private->AbsolutePointerMode.AbsoluteMaxX == 0)) {
        if (!(Private->AbsolutePointerList[Index]->Mode->AbsoluteMinX == 0 && Private->AbsolutePointerList[Index]->Mode->AbsoluteMaxX == 0)) {
          Factor = DivU64x32 (LShiftU64 ((UINT64)Private->AbsolutePointerMode.AbsoluteMaxX, 32), (UINT32)(Private->AbsolutePointerList[Index]->Mode->AbsoluteMaxX - Private->AbsolutePointerList[Index]->Mode->AbsoluteMinX));
          State->CurrentX = (UINTN)(RShiftU64 (MultU64x32 (Factor, (UINT32)(CurrentState.CurrentX - Private->AbsolutePointerList[Index]->Mode->AbsoluteMinX)), 32));
        }
      }
      if (!(Private->AbsolutePointerMode.AbsoluteMinY == 0 && Private->AbsolutePointerMode.AbsoluteMaxY == 0)) {
        if (!(Private->AbsolutePointerList[Index]->Mode->AbsoluteMinY == 0 && Private->AbsolutePointerList[Index]->Mode->AbsoluteMaxY == 0)) {
          Factor = DivU64x32 (LShiftU64 ((UINT64)Private->AbsolutePointerMode.AbsoluteMaxY, 32), (UINT32)(Private->AbsolutePointerList[Index]->Mode->AbsoluteMaxY - Private->AbsolutePointerList[Index]->Mode->AbsoluteMinY));
          State->CurrentY = (UINTN)(RShiftU64 (MultU64x32 (Factor, (UINT32)(CurrentState.CurrentY - Private->AbsolutePointerList[Index]->Mode->AbsoluteMinY)), 32));
        }
      }
      if (!(Private->AbsolutePointerMode.AbsoluteMinZ == 0 && Private->AbsolutePointerMode.AbsoluteMaxZ == 0)) {
        if (!(Private->AbsolutePointerList[Index]->Mode->AbsoluteMinZ == 0 && Private->AbsolutePointerList[Index]->Mode->AbsoluteMaxZ == 0)) {
          Factor = DivU64x32 (LShiftU64 ((UINT64)Private->AbsolutePointerMode.AbsoluteMaxZ, 32), (UINT32)(Private->AbsolutePointerList[Index]->Mode->AbsoluteMaxZ - Private->AbsolutePointerList[Index]->Mode->AbsoluteMinZ));
          State->CurrentZ = (UINTN)(RShiftU64 (MultU64x32 (Factor, (UINT32)(CurrentState.CurrentZ - Private->AbsolutePointerList[Index]->Mode->AbsoluteMinZ)), 32));
        }
      }
    } else if (Status == EFI_DEVICE_ERROR) {
      ReturnStatus = EFI_DEVICE_ERROR;
    }
  }

  return ReturnStatus;
}


/**
  This event agregates all the events of the pointer devices in the splitter.
  If any events of physical pointer devices are signaled, signal the pointer
  splitter event. This will cause the calling code to call
  ConSplitterAbsolutePointerGetState ().

  @param  Event                    The Event assoicated with callback.
  @param  Context                  Context registered when Event was created.

**/
VOID
EFIAPI
ConSplitterAbsolutePointerWaitForInput (
  IN  EFI_EVENT                       Event,
  IN  VOID                            *Context
  )
{
  EFI_STATUS                    Status;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private = (TEXT_IN_SPLITTER_PRIVATE_DATA *) Context;

  //
  // if AbsoluteInputEventSignalState is flagged before,
  // and not cleared by Reset() or GetState(), signal it
  //
  if (Private->AbsoluteInputEventSignalState) {
    gBS->SignalEvent (Event);
    return ;
  }
  //
  // if any physical console input device has key input, signal the event.
  //
  for (Index = 0; Index < Private->CurrentNumberOfAbsolutePointers; Index++) {
    Status = gBS->CheckEvent (Private->AbsolutePointerList[Index]->WaitForInput);
    if (!EFI_ERROR (Status)) {
      gBS->SignalEvent (Event);
      Private->AbsoluteInputEventSignalState = TRUE;
    }
  }
}


/**
  Reset the text output device hardware and optionaly run diagnostics

  @param  This                     Protocol instance pointer.
  @param  ExtendedVerification     Driver may perform more exhaustive verfication
                                   operation of the device during reset.

  @retval EFI_SUCCESS              The text output device was reset.
  @retval EFI_DEVICE_ERROR         The text output device is not functioning
                                   correctly and could not be reset.

**/
EFI_STATUS
EFIAPI
ConSplitterTextOutReset (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  BOOLEAN                            ExtendedVerification
  )
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = Private->TextOutList[Index].TextOut->Reset (
                                                    Private->TextOutList[Index].TextOut,
                                                    ExtendedVerification
                                                    );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  This->SetAttribute (This, EFI_TEXT_ATTR (This->Mode->Attribute & 0x0F, EFI_BLACK));

  //
  // reset all mode parameters
  //
  Status = DevNullTextOutSetMode (Private, 0);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  return ReturnStatus;
}


/**
  Write a Unicode string to the output device.

  @param  This                     Protocol instance pointer.
  @param  WString                  The NULL-terminated Unicode string to be
                                   displayed on the output device(s). All output
                                   devices must also support the Unicode drawing
                                   defined in this file.

  @retval EFI_SUCCESS              The string was output to the device.
  @retval EFI_DEVICE_ERROR         The device reported an error while attempting to
                                   output the text.
  @retval EFI_UNSUPPORTED          The output device's mode is not currently in a
                                   defined text mode.
  @retval EFI_WARN_UNKNOWN_GLYPH   This warning code indicates that some of the
                                   characters in the Unicode string could not be
                                   rendered and were skipped.

**/
EFI_STATUS
EFIAPI
ConSplitterTextOutOutputString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  CHAR16                             *WString
  )
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;
  UINTN                           BackSpaceCount;
  CHAR16                          *TargetString;

  This->SetAttribute (This, This->Mode->Attribute);

  Private         = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // In case that character's attribute is EFI_WIDE_ATTRIBUTE,
  // we need one more extra backspace to erase character completely.
  //
  BackSpaceCount  = 0;
  for (TargetString = WString; *TargetString; TargetString++) {
    if (*TargetString == CHAR_BACKSPACE) {
      BackSpaceCount++;
    }
  }
  if (BackSpaceCount == 0) {
    TargetString = WString;
  } else {
    TargetString = AllocatePool (sizeof (CHAR16) * (StrLen (WString) + BackSpaceCount + 1));
    if (TargetString == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    StrCpy (TargetString, WString);
  }

  if (Private->CurrentNumberOfConsoles > 0) {
    Private->TextOutMode.CursorColumn = Private->TextOutList[0].TextOut->Mode->CursorColumn;
    Private->TextOutMode.CursorRow = Private->TextOutList[0].TextOut->Mode->CursorRow;
  }

  Status = DevNullTextOutOutputString (Private, TargetString);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = Private->TextOutList[Index].TextOut->OutputString (
                                                    Private->TextOutList[Index].TextOut,
                                                    WString
                                                    );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  if (Private->CurrentNumberOfConsoles > 0) {
    Private->TextOutMode.CursorColumn = Private->TextOutList[0].TextOut->Mode->CursorColumn;
    Private->TextOutMode.CursorRow = Private->TextOutList[0].TextOut->Mode->CursorRow;
  }

  if (BackSpaceCount) {
    FreePool (TargetString);
  }
  return ReturnStatus;
}


/**
  Verifies that all characters in a Unicode string can be output to the
  target device.

  @param  This                     Protocol instance pointer.
  @param  WString                  The NULL-terminated Unicode string to be
                                   examined for the output device(s).

  @retval EFI_SUCCESS              The device(s) are capable of rendering the
                                   output string.
  @retval EFI_UNSUPPORTED          Some of the characters in the Unicode string
                                   cannot be rendered by one or more of the output
                                   devices mapped by the EFI handle.

**/
EFI_STATUS
EFIAPI
ConSplitterTextOutTestString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  CHAR16                             *WString
  )
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = Private->TextOutList[Index].TextOut->TestString (
                                                    Private->TextOutList[Index].TextOut,
                                                    WString
                                                    );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }
  //
  // There is no DevNullTextOutTestString () since a Unicode buffer would
  // always return EFI_SUCCESS.
  // ReturnStatus will be EFI_SUCCESS if no consoles are present
  //
  return ReturnStatus;
}


/**
  Returns information for an available text mode that the output device(s)
  supports.

  @param  This                     Protocol instance pointer.
  @param  ModeNumber               The mode number to return information on.
  @param  Columns                  Returns the columns of the text output device
                                   for the requested ModeNumber.
  @param  Rows                     Returns the rows of the text output device
                                   for the requested ModeNumber.

  @retval EFI_SUCCESS              The requested mode information was returned.
  @retval EFI_DEVICE_ERROR         The device had an error and could not complete
                                   the request.
  @retval EFI_UNSUPPORTED          The mode number was not valid.

**/
EFI_STATUS
EFIAPI
ConSplitterTextOutQueryMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                              ModeNumber,
  OUT UINTN                              *Columns,
  OUT UINTN                              *Rows
  )
{
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           CurrentMode;
  INT32                           *TextOutModeMap;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // Check whether param ModeNumber is valid.
  // ModeNumber should be within range 0 ~ MaxMode - 1.
  //
  if ( (ModeNumber > (UINTN)(((UINT32)-1)>>1)) ) {
    return EFI_UNSUPPORTED;
  }

  if ((INT32) ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  //
  // We get the available mode from mode intersection map if it's available
  //
  if (Private->TextOutModeMap != NULL) {
    TextOutModeMap = Private->TextOutModeMap + Private->TextOutListCount * ModeNumber;
    CurrentMode    = (UINTN)(*TextOutModeMap);
    *Columns       = Private->TextOutQueryData[CurrentMode].Columns;
    *Rows          = Private->TextOutQueryData[CurrentMode].Rows;
  } else {
    *Columns  = Private->TextOutQueryData[ModeNumber].Columns;
    *Rows     = Private->TextOutQueryData[ModeNumber].Rows;
  }

  if (*Columns <= 0 && *Rows <= 0) {
    return EFI_UNSUPPORTED;

  }

  return EFI_SUCCESS;
}


/**
  Sets the output device(s) to a specified mode.

  @param  This                     Protocol instance pointer.
  @param  ModeNumber               The mode number to set.

  @retval EFI_SUCCESS              The requested text mode was set.
  @retval EFI_DEVICE_ERROR         The device had an error and could not complete
                                   the request.
  @retval EFI_UNSUPPORTED          The mode number was not valid.

**/
EFI_STATUS
EFIAPI
ConSplitterTextOutSetMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                              ModeNumber
  )
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  INT32                           *TextOutModeMap;
  EFI_STATUS                      ReturnStatus;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Mode;
  UINTN                                SizeOfInfo;
  EFI_GRAPHICS_OUTPUT_PROTOCOL         *GraphicsOutput;
  UINTN                                NumberIndex;
  UINT32                               GopModeNumber;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // Check whether param ModeNumber is valid.
  // ModeNumber should be within range 0 ~ MaxMode - 1.
  //
  if ( (ModeNumber > (UINTN)(((UINT32)-1)>>1)) ) {
    return EFI_UNSUPPORTED;
  }

  if ((INT32) ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }
  //
  // If the mode is being set to the curent mode, then just clear the screen and return.
  //
  if (Private->TextOutMode.Mode == (INT32) ModeNumber) {
    return ConSplitterTextOutClearScreen (This);
  }
  //
  // return the worst status met
  //
  GopModeNumber  = mConOut.GraphicsOutput.Mode->MaxMode;
  TextOutModeMap = Private->TextOutModeMap + Private->TextOutListCount * ModeNumber;
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = Private->TextOutList[Index].TextOut->SetMode (
                                                    Private->TextOutList[Index].TextOut,
                                                    TextOutModeMap[Index]
                                                    );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    } else if (Private->TextOutList[Index].GraphicsOutput != NULL) {
      Mode           = Private->TextOutList[Index].GraphicsOutput->Mode->Info;
      GraphicsOutput = &mConOut.GraphicsOutput;

      for (NumberIndex = 0; NumberIndex < GraphicsOutput->Mode->MaxMode; NumberIndex ++) {
        Status = GraphicsOutput->QueryMode (GraphicsOutput, (UINT32) NumberIndex, &SizeOfInfo, &Info);
        if (EFI_ERROR (Status)) {
          break;
        }
        if ((Info->HorizontalResolution == Mode->HorizontalResolution) &&
            (Info->VerticalResolution == Mode->VerticalResolution)) {
          GopModeNumber = (UINT32)NumberIndex;
          FreePool (Info);
          break;
        }
        FreePool (Info);
      }
    }
  }

  //
  // The DevNull Console will support any possible mode as it allocates memory
  //
  Status = DevNullTextOutSetMode (Private, ModeNumber);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  //
  // BugBug:
  // Now, we doens't consider there are two gop mode in the same text mode.
  //
  if (!EFI_ERROR (ReturnStatus) && GopModeNumber != Private->GraphicsOutput.Mode->MaxMode &&
      GopModeNumber != Private->GraphicsOutput.Mode->Mode) {
    Private->GraphicsOutput.SetMode (&mConOut.GraphicsOutput, GopModeNumber);
  }

  return ReturnStatus;
}


/**
  Sets the background and foreground colors for the OutputString () and
  ClearScreen () functions.

  @param  This                     Protocol instance pointer.
  @param  Attribute                The attribute to set. Bits 0..3 are the
                                   foreground color, and bits 4..6 are the
                                   background color. All other bits are undefined
                                   and must be zero. The valid Attributes are
                                   defined in this file.

  @retval EFI_SUCCESS              The attribute was set.
  @retval EFI_DEVICE_ERROR         The device had an error and could not complete
                                   the request.
  @retval EFI_UNSUPPORTED          The attribute requested is not defined.

**/
EFI_STATUS
EFIAPI
ConSplitterTextOutSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                              Attribute
  )
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // Check whether param Attribute is valid.
  //
  if ((Attribute | 0x7F) != 0x7F) {
    return EFI_UNSUPPORTED;
  }

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = Private->TextOutList[Index].TextOut->SetAttribute (
                                                    Private->TextOutList[Index].TextOut,
                                                    Attribute
                                                    );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  Private->TextOutMode.Attribute = (INT32) Attribute;

  return ReturnStatus;
}


/**
  Clears the output device(s) display to the currently selected background
  color.

  @param  This                     Protocol instance pointer.

  @retval EFI_SUCCESS              The operation completed successfully.
  @retval EFI_DEVICE_ERROR         The device had an error and could not complete
                                   the request.
  @retval EFI_UNSUPPORTED          The output device is not in a valid text mode.

**/
EFI_STATUS
EFIAPI
ConSplitterTextOutClearScreen (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This
  )
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = Private->TextOutList[Index].TextOut->ClearScreen (Private->TextOutList[Index].TextOut);
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  Status = DevNullTextOutClearScreen (Private);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  return ReturnStatus;
}


/**
  Sets the current coordinates of the cursor position

  @param  This                     Protocol instance pointer.
  @param  Column                   The column position to set the cursor to. Must be
                                   greater than or equal to zero and less than the
                                   number of columns by QueryMode ().
  @param  Row                      The row position to set the cursor to. Must be
                                   greater than or equal to zero and less than the
                                   number of rows by QueryMode ().

  @retval EFI_SUCCESS              The operation completed successfully.
  @retval EFI_DEVICE_ERROR         The device had an error and could not complete
                                   the request.
  @retval EFI_UNSUPPORTED          The output device is not in a valid text mode,
                                   or the cursor position is invalid for the
                                   current mode.

**/
EFI_STATUS
EFIAPI
ConSplitterTextOutSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                              Column,
  IN  UINTN                              Row
  )
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;
  UINTN                           MaxColumn;
  UINTN                           MaxRow;
  INT32                           *TextOutModeMap;
  INT32                           ModeNumber;
  INT32                           CurrentMode;

  Private   = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);
  TextOutModeMap  = NULL;
  ModeNumber      = Private->TextOutMode.Mode;

  //
  // Get current MaxColumn and MaxRow from intersection map
  //
  if (Private->TextOutModeMap != NULL) {
    TextOutModeMap = Private->TextOutModeMap + Private->TextOutListCount * ModeNumber;
    CurrentMode    = *TextOutModeMap;
  } else {
    CurrentMode = ModeNumber;
  }

  MaxColumn = Private->TextOutQueryData[CurrentMode].Columns;
  MaxRow    = Private->TextOutQueryData[CurrentMode].Rows;

  if (Column >= MaxColumn || Row >= MaxRow) {
    return EFI_UNSUPPORTED;
  }
  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = Private->TextOutList[Index].TextOut->SetCursorPosition (
                                                    Private->TextOutList[Index].TextOut,
                                                    Column,
                                                    Row
                                                    );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  DevNullTextOutSetCursorPosition (Private, Column, Row);

  return ReturnStatus;
}


/**
  Makes the cursor visible or invisible

  @param  This                     Protocol instance pointer.
  @param  Visible                  If TRUE, the cursor is set to be visible. If
                                   FALSE, the cursor is set to be invisible.

  @retval EFI_SUCCESS              The operation completed successfully.
  @retval EFI_DEVICE_ERROR         The device had an error and could not complete
                                   the request, or the device does not support
                                   changing the cursor mode.
  @retval EFI_UNSUPPORTED          The output device is not in a valid text mode.

**/
EFI_STATUS
EFIAPI
ConSplitterTextOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  BOOLEAN                            Visible
  )
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = Private->TextOutList[Index].TextOut->EnableCursor (
                                                    Private->TextOutList[Index].TextOut,
                                                    Visible
                                                    );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  DevNullTextOutEnableCursor (Private, Visible);

  return ReturnStatus;
}

EFI_STATUS
EFIAPI
FindOptimalTextMode (
  IN TEXT_OUT_SPLITTER_PRIVATE_DATA        *Private,
  OUT UINTN                                *OptimalTxtModeNum
  )
{
  UINTN                                 ModeNum;
  UINTN                                 Columns;
  UINTN                                 Rows;
  UINTN                                 OptimalTxtMode;
  OEM_LOGO_RESOLUTION_DEFINITION        *OemLogoResolutionPtr;
  EFI_HANDLE                            SinglePhyGopHd;
  EFI_EDID_DISCOVERED_PROTOCOL          *EdidDiscovered;
  UINT32                                NativeResolutionX;
  UINT32                                NativeResolutionY;
  OEM_LOGO_RESOLUTION_DEFINITION        LogoResolution;
  UINT32                                ModeNumber;
  EFI_STATUS                            Status;
  UINTN                                 SizeOfInfo;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *Info;
  UINTN                                 BestResolution;
  UINT32                                SizeOfX;
  UINT32                                SizeOfY;

  //
  // init local vars
  //
  OptimalTxtMode = 0;
  *OptimalTxtModeNum = (UINTN)Private->TextOut.Mode->MaxMode;
  OemLogoResolutionPtr = NULL;
  SinglePhyGopHd = NULL;
  Status = EFI_UNSUPPORTED;
  SizeOfInfo = 0;
  Info = NULL;
  BestResolution = 0;
  SizeOfX = 0;
  SizeOfY = 0;

  if (FoundTextBasedConsole (Private)) {
    //
    //if any Console deveice is text-based display,
    //based on text mode intersection, find optimal text mode
    //
    for (ModeNum = 0; ModeNum < (UINTN)(Private->TextOut.Mode->MaxMode); ModeNum++) {
      Status =  ConSplitterTextOutQueryMode (&Private->TextOut, ModeNum, &Columns, &Rows);
      if (!EFI_ERROR (Status)) {
        if ((Columns * Rows) > OptimalTxtMode) {
          OptimalTxtMode = Columns * Rows;
          *OptimalTxtModeNum = (UINTN)ModeNum;
        }
      }
    }
    goto Done;
  }

  OemLogoResolutionPtr = (OEM_LOGO_RESOLUTION_DEFINITION *)PcdGetPtr (PcdDefaultLogoResolution);
  if (Private->CurrentNumberOfConsoles == 1) {
      GetSinglePhyGopHandle (Private, &SinglePhyGopHd);
      if (SinglePhyGopHd == NULL) {
        goto Done;
      }
      Status = gBS->HandleProtocol (
                      SinglePhyGopHd,
                      &gEfiEdidDiscoveredProtocolGuid,
                      (VOID **) &EdidDiscovered
                      );
      if  (!EFI_ERROR (Status)) {
        Status = GetResolutionByEdid (EdidDiscovered, &NativeResolutionX, &NativeResolutionY);
        if (!EFI_ERROR (Status)) {
          LogoResolution.LogoResolutionX = NativeResolutionX;
          LogoResolution.LogoResolutionY = NativeResolutionY;
          Status = CheckModeSupported (
                     Private->TextOutList->GraphicsOutput,
                     NativeResolutionX,
                     NativeResolutionY,
                     &ModeNumber
                     );
          if (!EFI_ERROR(Status)) {
            OemLogoResolutionPtr = &LogoResolution;
          }
        }
      } else {
        //
        // find optimal resoltion from gop modes to set for possible combination of two video output devices
        // attached to a gop device
        //
        OemLogoResolutionPtr = &LogoResolution;
        GetComboVideoOptimalResolution (Private->TextOutList->GraphicsOutput, &SizeOfX, &SizeOfY);
        LogoResolution.LogoResolutionX = SizeOfX;
        LogoResolution.LogoResolutionY = SizeOfY;
        OemLogoResolutionPtr = &LogoResolution;
      }

      OemSvcLogoResolution (&OemLogoResolutionPtr);

      for (ModeNum = 0; ModeNum < (UINTN)(Private->TextOut.Mode->MaxMode); ModeNum++) {
        Status = ConSplitterTextOutQueryMode (&Private->TextOut, ModeNum, &Columns, &Rows);
        if (!EFI_ERROR (Status)) {
          if ((Columns == (OemLogoResolutionPtr->LogoResolutionX / EFI_GLYPH_WIDTH)) &&
              (Rows == (OemLogoResolutionPtr->LogoResolutionY / EFI_GLYPH_HEIGHT)) ) {
            *OptimalTxtModeNum = (UINTN)ModeNum;
          }
        }
      }

  } else {
    for (ModeNum = 0;ModeNum< Private->GraphicsOutput.Mode->MaxMode; ModeNum++) {
      //
      //find best resolution from virtual gop
      //
      Private->GraphicsOutput.QueryMode (&Private->GraphicsOutput, (UINT32)ModeNum, &SizeOfInfo, &Info);
      if (((Info->HorizontalResolution) * (Info->VerticalResolution)) > BestResolution) {
        BestResolution =  (Info->HorizontalResolution) * (Info->VerticalResolution);
        LogoResolution.LogoResolutionX = Info->HorizontalResolution;
        LogoResolution.LogoResolutionY = Info->VerticalResolution;
        OemLogoResolutionPtr = &LogoResolution;
      }
      gBS->FreePool (Info);
    }

    OemSvcLogoResolution (&OemLogoResolutionPtr);

    for (ModeNum = 0; ModeNum < (UINTN)(Private->TextOut.Mode->MaxMode); ModeNum++) {
      Status = ConSplitterTextOutQueryMode (&Private->TextOut, ModeNum, &Columns, &Rows);
      if (!EFI_ERROR (Status)) {
        if ((Columns == (OemLogoResolutionPtr->LogoResolutionX / EFI_GLYPH_WIDTH)) &&
            (Rows == (OemLogoResolutionPtr->LogoResolutionY / EFI_GLYPH_HEIGHT)) ) {
          *OptimalTxtModeNum = (UINTN)ModeNum;
        }
      }
    }

  }

Done:
  if (*OptimalTxtModeNum < (UINTN)(Private->TextOut.Mode->MaxMode)) {
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_UNSUPPORTED;
  }

  return Status;
}

VOID
GetSinglePhyGopHandle (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA        *Private,
  OUT EFI_HANDLE                            *SinglePhyGop
  )
{
  EFI_STATUS                             Status;
  UINTN                                  HandleCount;
  EFI_HANDLE                             *HandleBuffer;
  UINTN                                  Index;
  EFI_GRAPHICS_OUTPUT_PROTOCOL           *GraphicsOutput;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return ;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiGraphicsOutputProtocolGuid,
                    (VOID*)&GraphicsOutput
                    );

    if (!EFI_ERROR (Status)) {
      if (GraphicsOutput == Private->TextOutList->GraphicsOutput) {
        *SinglePhyGop = HandleBuffer[Index];
        break;
      }
    }
  }
  gBS->FreePool (HandleBuffer);

}

BOOLEAN
FoundTextBasedConsole (
  IN TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private
  )
{
  BOOLEAN  FoundIt;
  UINTN    Index;

  //
  //init local
  //
  FoundIt = FALSE;

  for (Index = 0; Index < Private->CurrentNumberOfConsoles; Index++) {
    if (Private->TextOutList[Index].GraphicsOutput == NULL) {
      FoundIt = TRUE;
    }
  }

  return FoundIt;
}

VOID
GetComboVideoOptimalResolution (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput,
  OUT UINT32                        *XResolution,
  OUT UINT32                        *YResoulution
  )
{
  UINT32                               ModeNumber;
  EFI_STATUS                           Status;
  UINTN                                SizeOfInfo;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  UINT32                               MaxMode;
  UINTN                                MaxResolution;
  UINTN                                TempResolution;

  Status  = EFI_SUCCESS;
  MaxMode = GraphicsOutput->Mode->MaxMode;
  MaxResolution = 0;
  TempResolution = 0;

  for (ModeNumber = 0; ModeNumber < MaxMode; ModeNumber++) {
    Status = GraphicsOutput->QueryMode (
                       GraphicsOutput,
                       ModeNumber,
                       &SizeOfInfo,
                       &Info
                       );
    if (!EFI_ERROR (Status)) {
      TempResolution = (Info->HorizontalResolution) * (Info->VerticalResolution);
      if (TempResolution > MaxResolution) {
        MaxResolution = TempResolution;
        *XResolution = Info->HorizontalResolution;
        *YResoulution = Info->VerticalResolution;
      }
      gBS->FreePool (Info);
    }
  }
}

VOID
SyncNewConsole (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *NewConsole
  )
{
  UINTN    Columns;
  UINTN    Rows;
  UINT8    ModeNum;

  for (ModeNum = 0; ModeNum < NewConsole->Mode->MaxMode; ModeNum++) {
    NewConsole->QueryMode (NewConsole, ModeNum, &Columns, &Rows);
    if ((Columns == Private->DevNullColumns) && (Rows == Private->DevNullRows)) {
      //
      //init new console's text mode
      //
      NewConsole->SetMode (NewConsole, ModeNum);
      NewConsole->SetCursorPosition (NewConsole, 0, 0);
      NewConsole->EnableCursor (NewConsole, TRUE);
      break;
    }
  }
}

VOID
OutputTextContentsToNewConsoleDev (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *NewConsole
  )
{
  UINTN   Row;
  UINTN   Column;
  CHAR16  *TextScreen;
  INT32   *Attributes;
  INT32   CurAttribute;
  CHAR16  PrintChar [2] = {0};
  UINTN   CharIndex;
  UINTN   AttriIndex;

  //
  //init local
  //
  CharIndex = 0;
  AttriIndex = 0;
  TextScreen = Private->DevNullScreen;
  Attributes = Private->DevNullAttributes;
  CurAttribute =  EFI_TEXT_ATTR(EFI_YELLOW, EFI_BACKGROUND_BLACK);

  if (Private->IsDevNullBlankScreen == TRUE) {
    return ;
  }

  for (Row = 0; Row < Private->DevNullRows; Row++) {
    for (Column = 0; Column < Private->DevNullColumns; Column++, TextScreen++, Attributes++) {
      //
      //last row and column is reserved for cursor, so skip it
      //
      if ((Row == (Private->DevNullRows - 1)) && (Column == (Private->DevNullColumns - 1))) {
        break;
      }

      if (CurAttribute != *Attributes) {
        CurAttribute = *Attributes;
        NewConsole->SetAttribute (NewConsole, CurAttribute);
      }
      PrintChar [0] = *TextScreen ;
      NewConsole->OutputString (NewConsole, PrintChar);
    }
    //
    // Each line of the screen has a NULL on the end so we must skip over it
    //
    TextScreen++;
  }

  NewConsole->SetCursorPosition (NewConsole, Private->TextOutMode.CursorColumn, Private->TextOutMode.CursorRow);

}

VOID
SyncVirtualConsole (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA     *Private
  )
{
  UINTN                         ModeNum;
  UINTN                         Columns;
  UINTN                         Rows;
  UINT32                        YResolution;
  UINT32                        XResolution;
  UINT32                        CurXResolution;
  UINT32                        CurYResolution;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;
  UINT8                         Index;

  //
  //init local
  //
  GraphicsOutput = NULL;

  for (ModeNum = 0; ModeNum < (UINTN)Private->TextOutMode.MaxMode; ModeNum++) {
    Private->TextOut.QueryMode (&Private->TextOut, ModeNum, &Columns, &Rows);
    if ((Columns == Private->DevNullColumns) && (Rows == Private->DevNullRows)) {
      Private->TextOutMode.Mode = (INT32)ModeNum;
      break;
    }
  }

  for (Index = 0; Index < Private->CurrentNumberOfConsoles; Index++) {
    if (Private->TextOutList [Index].GraphicsOutput != NULL) {
      GraphicsOutput = Private->TextOutList [Index].GraphicsOutput;
      break;
    }
  }

  if (GraphicsOutput != NULL) {
    CurXResolution = GraphicsOutput->Mode->Info->HorizontalResolution;
    CurYResolution = GraphicsOutput->Mode->Info->VerticalResolution;
    for (ModeNum = 0; ModeNum < Private->GraphicsOutput.Mode->MaxMode; ModeNum++) {
      XResolution = Private->GraphicsOutputModeBuffer [ModeNum].HorizontalResolution;
      YResolution = Private->GraphicsOutputModeBuffer [ModeNum].VerticalResolution;
      if ((XResolution == CurXResolution) && (YResolution == CurYResolution)) {
        Private->GraphicsOutput.Mode->Mode = (INT32)ModeNum;
        break;
      }
    }

    CopyMem (
      Private->GraphicsOutput.Mode->Info,
      &Private->GraphicsOutputModeBuffer[ModeNum],
      Private->GraphicsOutput.Mode->SizeOfInfo
      );
    //
    // Update mode information only when there is one GOP
    //
    if (Private->CurrentNumberOfConsoles == 1 &&
        Private->TextOutList[0].GraphicsOutput != NULL) {
      GraphicsOutput = Private->TextOutList[0].GraphicsOutput;
      Private->GraphicsOutput.Mode->FrameBufferBase = GraphicsOutput->Mode->FrameBufferBase;
      Private->GraphicsOutput.Mode->FrameBufferSize = GraphicsOutput->Mode->FrameBufferSize;
    }
  }

}


/** @file

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

/**
  ConsoleOut Routines that speak VGA.

Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>

This program and the accompanying materials
are licensed and made available under the terms and conditions
of the BSD License which accompanies this distribution.  The
full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "BiosKeyboard.h"

STATIC BOOLEAN             mPS2KeyboardCheck = TRUE;
//
// EFI Driver Binding Protocol Instance
//
EFI_DRIVER_BINDING_PROTOCOL gBiosKeyboardDriverBinding = {
  BiosKeyboardDriverBindingSupported,
  BiosKeyboardDriverBindingStart,
  BiosKeyboardDriverBindingStop,
  0x3,
  NULL,
  NULL
};

BOOLEAN
DetectKeyboard (
  IN  BIOS_KEYBOARD_DEV     *BiosKeyboardPrivate
  );

EFI_STATUS
EFIAPI
CheckKeyboardStatus (
  IN  BIOS_KEYBOARD_DEV     *BiosKeyboardPrivate
  );

//
// Private tables
//
#define TABLE_END 0x0

struct {
  UINT8   ScanCode;
  CHAR16  UnicodeChar;
  CHAR16  ShiftUnicodeChar;
}
mAltConvertTable[] = {
  {
    0x78,
    '1',
    '!'
  },
  {
    0x79,
    '2',
    '@'
  },
  {
    0x7a,
    '3',
    '#'
  },
  {
    0x7b,
    '4',
    '$'
  },
  {
    0x7c,
    '5',
    '%'
  },
  {
    0x7d,
    '6',
    '^'
  },
  {
    0x7e,
    '7',
    '&'
  },
  {
    0x7f,
    '8',
    '*'
  },
  {
    0x80,
    '9',
    '('
  },
  {
    0x81,
    '0',
    ')'
  },
  {
    0x82,
    '-',
    '_'
  },
  {
    0x83,
    '=',
    '+'
  },
  {
    0xa5, //  Tab
    0x09,
    0x09
  },
  {
    0x10,
    'q',
    'Q'
  },
  {
    0x11,
    'w',
    'W'
  },
  {
    0x12,
    'e',
    'E'
  },
  {
    0x13,
    'r',
    'R'
  },
  {
    0x14,
    't',
    'T'
  },
  {
    0x15,
    'y',
    'Y'
  },
  {
    0x16,
    'u',
    'U'
  },
  {
    0x17,
    'i',
    'I'
  },
  {
    0x18,
    'o',
    'O'
  },
  {
    0x19,
    'p',
    'P'
  },
  {
    0x1a,
    '[',
    '{'
  },
  {
    0x1b,
    ']',
    '}'
  },
  {
    0x1e,
    'a',
    'A'
  },
  {
    0x1f,
    's',
    'S'
  },
  {
    0x20,
    'd',
    'D'
  },
  {
    0x21,
    'f',
    'F'
  },
  {
    0x22,
    'g',
    'G'
  },
  {
    0x23,
    'h',
    'H'
  },
  {
    0x24,
    'j',
    'J'
  },
  {
    0x25,
    'k',
    'K'
  },
  {
    0x26,
    'l',
    'L'
  },
  {
    0x27,
    ';',
    ':'
  },
  {
    0x28,
    '\'',
    '"'
  },
  {
    0x29,
    '`',
    '~'
  },
  {
    0x2b,
    '\\',
    '|'
  },
  {
    0x2c,
    'z',
    'Z'
  },
  {
    0x2d,
    'x',
    'X'
  },
  {
    0x2e,
    'c',
    'C'
  },
  {
    0x2f,
    'v',
    'V'
  },
  {
    0x30,
    'b',
    'B'
  },
  {
    0x31,
    'n',
    'N'
  },
  {
    0x32,
    'm',
    'M'
  },
  {
    0x33,
    ',',
    '<'
  },
  {
    0x34,
    '.',
    '>'
  },
  {
    0x35,
    '/',
    '?'
  },
  {
    0x37, // Numeric Keypad *
    '*',
    '*'
  },
  {
    0x39,
    ' ',
    ' '
  },
  {
    0x4a,
    '-',
    '-'
  },
  {
    0x4e,
    '+',
    '+'
  },
  {
    TABLE_END,
    CHAR_NULL
  },
};

STATIC
UINT8 KbcCodeTable[][5] = {
// EfiScanCode      Base       Shift      Alt       Ctrl
    SCAN_HOME,      0x47,      0x47,      0x97,     0x77,           // 'Home'
    SCAN_UP,        0x48,      0x48,      0x98,     0x8D,           // 'Up Arrow'
    SCAN_PAGE_UP,   0x49,      0x49,      0x99,     0x84,           // 'PageUp'
    SCAN_LEFT,      0x4B,      0x4B,      0x9B,     0x73,           // 'Left Arrow'
    SCAN_RIGHT,     0x4D,      0x4D,      0x9D,     0x74,           // 'Right Arrow'
    SCAN_END,       0x4F,      0x4F,      0x9F,     0x75,           // 'End'
    SCAN_DOWN,      0x50,      0x50,      0xA0,     0x91,           // 'Down Arrow'
    SCAN_PAGE_DOWN, 0x51,      0x51,      0xA1,     0x76,           // 'PageDown'
    SCAN_INSERT,    0x52,      0x52,      0xA2,     0x92,           // 'Insert'
    SCAN_DELETE,    0x53,      0x53,      0xA3,     0x93,           // 'Delete Forward'
    SCAN_F1,        0x3B,      0x54,      0x68,     0x5E,           // 'F1'
    SCAN_F2,        0x3C,      0x55,      0x69,     0x5F,           // 'F2'
    SCAN_F3,        0x3D,      0x56,      0x6A,     0x60,           // 'F3'
    SCAN_F4,        0x3E,      0x57,      0x6B,     0x61,           // 'F4'
    SCAN_F5,        0x3F,      0x58,      0x6C,     0x62,           // 'F5'
    SCAN_F6,        0x40,      0x59,      0x6D,     0x63,           // 'F6'
    SCAN_F7,        0x41,      0x5A,      0x6E,     0x64,           // 'F7'
    SCAN_F8,        0x42,      0x5B,      0x6F,     0x65,           // 'F8'
    SCAN_F9,        0x43,      0x5C,      0x70,     0x66,           // 'F9'
    SCAN_F10,       0x44,      0x5D,      0x71,     0x67,           // 'F10'
    SCAN_F11,       0x85,      0x87,      0x8B,     0x89,           // 'F11'
    SCAN_F12,       0x86,      0x88,      0x8C,     0x8A,           // 'F12'
    TABLE_END, TABLE_END, TABLE_END, TABLE_END, TABLE_END
};

/**
  Enqueue the key.

  @param  Queue                 The queue to be enqueued.
  @param  KeyData               The key data to be enqueued.

  @retval EFI_NOT_READY         The queue is full.
  @retval EFI_SUCCESS           Successfully enqueued the key data.

**/
EFI_STATUS
Enqueue (
  IN SIMPLE_QUEUE         *Queue,
  IN EFI_KEY_DATA         *KeyData
  )
{
  if ((Queue->Rear + 1) % QUEUE_MAX_COUNT == Queue->Front) {
    return EFI_NOT_READY;
  }

  CopyMem (&Queue->Buffer[Queue->Rear], KeyData, sizeof (EFI_KEY_DATA));
  Queue->Rear = (Queue->Rear + 1) % QUEUE_MAX_COUNT;

  return EFI_SUCCESS;
}


/**
  Dequeue the key.

  @param  Queue                 The queue to be dequeued.
  @param  KeyData               The key data to be dequeued.

  @retval EFI_NOT_READY         The queue is empty.
  @retval EFI_SUCCESS           Successfully dequeued the key data.

**/
EFI_STATUS
Dequeue (
  IN SIMPLE_QUEUE         *Queue,
  IN EFI_KEY_DATA         *KeyData
  )
{
  if (Queue->Front == Queue->Rear) {
    return EFI_NOT_READY;
  }

  CopyMem (KeyData, &Queue->Buffer[Queue->Front], sizeof (EFI_KEY_DATA));
  Queue->Front  = (Queue->Front + 1) % QUEUE_MAX_COUNT;

  return EFI_SUCCESS;
}


/**
  Check whether the queue is empty.

  @param  Queue                 The queue to be checked.

  @retval EFI_NOT_READY         The queue is empty.
  @retval EFI_SUCCESS           The queue is not empty.

**/
EFI_STATUS
CheckQueue (
  IN SIMPLE_QUEUE         *Queue
  )
{
  if (Queue->Front == Queue->Rear) {
    return EFI_NOT_READY;
  }

  return EFI_SUCCESS;
}

//
// EFI Driver Binding Protocol Functions
//

/**
  Check whether the driver supports this device.

  @param  This                   The Udriver binding protocol.
  @param  Controller             The controller handle to check.
  @param  RemainingDevicePath    The remaining device path.

  @retval EFI_SUCCESS            The driver supports this controller.
  @retval other                  This device isn't supported.

**/
EFI_STATUS
EFIAPI
BiosKeyboardDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_STATUS                                Status;
  EFI_LEGACY_BIOS_PROTOCOL                  *LegacyBios;
  EFI_ISA_IO_PROTOCOL                       *IsaIo;
  UINT8                                     Data;

  Data = 0;

  //
  // See if the Legacy BIOS Protocol is available
  //
  Status = gBS->LocateProtocol (
                  &gEfiLegacyBiosProtocolGuid,
                  NULL,
                  (VOID **) &LegacyBios
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiIsaIoProtocolGuid,
                  (VOID **) &IsaIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Use the ISA I/O Protocol to see if Controller is the Keyboard controller
  //
  if (IsaIo->ResourceList->Device.HID != EISA_PNP_ID (0x303) || IsaIo->ResourceList->Device.UID != 0) {
    Status = EFI_UNSUPPORTED;
  } else {
    Status = IsaIo->Io.Read (IsaIo, EfiIsaIoWidthUint8, KEYBOARD_8042_STATUS_REGISTER, 1,&Data);

    if ((EFI_ERROR (Status)) || (Data == 0xFF)) {
      Status =  EFI_UNSUPPORTED;    //if status register is FF then KBC is missing
    }
  }

  gBS->CloseProtocol (
         Controller,
         &gEfiIsaIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  return Status;
}

/**
  Starts the device with this driver.

  @param  This                   The driver binding instance.
  @param  Controller             Handle of device to bind driver to.
  @param  RemainingDevicePath    Optional parameter use to pick a specific child
                                 device to start.

  @retval EFI_SUCCESS            The controller is controlled by the driver.
  @retval Other                  This controller cannot be started.

**/
EFI_STATUS
EFIAPI
BiosKeyboardDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_STATUS                                Status;
  EFI_LEGACY_BIOS_PROTOCOL                  *LegacyBios;
  EFI_ISA_IO_PROTOCOL                       *IsaIo;
  BIOS_KEYBOARD_DEV                         *BiosKeyboardPrivate;
  EFI_IA32_REGISTER_SET                     Regs;
  BOOLEAN                                   CarryFlag;
  EFI_PS2_POLICY_PROTOCOL                   *Ps2Policy;
  UINT8                                     Command;
  EFI_STATUS_CODE_VALUE                     StatusCode;
  EFI_TPL                                   OldTpl;
  BOOLEAN                                   DoKeyboardReset;
  VOID                                      *HobList;
  UINT8                                     *HobBuffer;
  UINT8                                     Values[2];
  UINT8                                     Value;

  BiosKeyboardPrivate = NULL;
  IsaIo               = NULL;
  StatusCode          = 0;
  HobList             = NULL;
  HobBuffer           = NULL;

  //
  // Get Ps2 policy to set. Will be use if present.
  //
  gBS->LocateProtocol (
        &gEfiPs2PolicyProtocolGuid,
        NULL,
        (VOID **) &Ps2Policy
        );

  //
  // See if the Legacy BIOS Protocol is available
  //
  Status = gBS->LocateProtocol (
                  &gEfiLegacyBiosProtocolGuid,
                  NULL,
                  (VOID **) &LegacyBios
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Open the IO Abstraction(s) needed
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiIsaIoProtocolGuid,
                  (VOID **) &IsaIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Allocate the private device structure
  //
    BiosKeyboardPrivate = (BIOS_KEYBOARD_DEV *) AllocateZeroPool (sizeof (BIOS_KEYBOARD_DEV));
  if (NULL == BiosKeyboardPrivate) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  //
  // Initialize the private device structure
  //
  BiosKeyboardPrivate->Signature                  = BIOS_KEYBOARD_DEV_SIGNATURE;
  BiosKeyboardPrivate->Handle                     = Controller;
  BiosKeyboardPrivate->LegacyBios                 = LegacyBios;
  BiosKeyboardPrivate->IsaIo                      = IsaIo;

  BiosKeyboardPrivate->SimpleTextIn.Reset         = BiosKeyboardReset;
  BiosKeyboardPrivate->SimpleTextIn.ReadKeyStroke = BiosKeyboardReadKeyStroke;

  BiosKeyboardPrivate->DataRegisterAddress        = KEYBOARD_8042_DATA_REGISTER;
  BiosKeyboardPrivate->StatusRegisterAddress      = KEYBOARD_8042_STATUS_REGISTER;
  BiosKeyboardPrivate->CommandRegisterAddress     = KEYBOARD_8042_COMMAND_REGISTER;
  BiosKeyboardPrivate->ExtendedKeyboard           = TRUE;

  BiosKeyboardPrivate->Queue.Front                = 0;
  BiosKeyboardPrivate->Queue.Rear                 = 0;
  BiosKeyboardPrivate->SimpleTextInputEx.Reset               = BiosKeyboardResetEx;
  BiosKeyboardPrivate->SimpleTextInputEx.ReadKeyStrokeEx     = BiosKeyboardReadKeyStrokeEx;
  BiosKeyboardPrivate->SimpleTextInputEx.SetState            = BiosKeyboardSetState;
  BiosKeyboardPrivate->SimpleTextInputEx.RegisterKeyNotify   = BiosKeyboardRegisterKeyNotify;
  BiosKeyboardPrivate->SimpleTextInputEx.UnregisterKeyNotify = BiosKeyboardUnregisterKeyNotify;
  InitializeListHead (&BiosKeyboardPrivate->NotifyList);

  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  DoKeyboardReset = FeaturePcdGet (PcdPs2KbdExtendedVerification);
  if (mPS2KeyboardCheck) {
    mPS2KeyboardCheck = FALSE;
    HobList = GetHobList ();
    HobBuffer = GetNextGuidHob ((CONST EFI_GUID*)&gKbcInitializedGuid, HobList);

    if (HobBuffer != NULL) {
      Status = CheckKeyboardStatus (BiosKeyboardPrivate);
      if (!EFI_ERROR (Status)) {
        DoKeyboardReset = FALSE;
      }
    }
  }

  gBS->RestoreTPL (OldTpl);

  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &BiosKeyboardPrivate->DevicePath
                  );

  //
  // Report that the keyboard is being enabled
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_ENABLE,
    BiosKeyboardPrivate->DevicePath
    );

  //
  // Setup the WaitForKey event
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  BiosKeyboardWaitForKey,
                  &(BiosKeyboardPrivate->SimpleTextIn),
                  &((BiosKeyboardPrivate->SimpleTextIn).WaitForKey)
                  );
  if (EFI_ERROR (Status)) {
    (BiosKeyboardPrivate->SimpleTextIn).WaitForKey = NULL;
    goto Done;
  }
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  BiosKeyboardWaitForKeyEx,
                  &(BiosKeyboardPrivate->SimpleTextInputEx),
                  &(BiosKeyboardPrivate->SimpleTextInputEx.WaitForKeyEx)
                  );
  if (EFI_ERROR (Status)) {
    BiosKeyboardPrivate->SimpleTextInputEx.WaitForKeyEx = NULL;
    goto Done;
  }

  //
  // Report a Progress Code for an attempt to detect the precense of the keyboard device in the system
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_PRESENCE_DETECT,
    BiosKeyboardPrivate->DevicePath
    );

  //
  // Reset the keyboard device
  //
  Status = BiosKeyboardPrivate->SimpleTextInputEx.Reset (
                                                    &BiosKeyboardPrivate->SimpleTextInputEx,
                                                    DoKeyboardReset
                                                    );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[KBD]Reset Failed. Status - %r\n", Status));
    StatusCode = EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_NOT_DETECTED;
    goto Done;
  }
  //
  // Do platform specific policy like port swapping and keyboard light default
  //
  if (Ps2Policy != NULL) {

    Ps2Policy->Ps2InitHardware (Controller);

    Command = 0;
    if ((Ps2Policy->KeyboardLight & EFI_KEYBOARD_CAPSLOCK) == EFI_KEYBOARD_CAPSLOCK) {
      Command |= 4;
    }

    if ((Ps2Policy->KeyboardLight & EFI_KEYBOARD_NUMLOCK) == EFI_KEYBOARD_NUMLOCK) {
      Command |= 2;
    }

    if ((Ps2Policy->KeyboardLight & EFI_KEYBOARD_SCROLLLOCK) == EFI_KEYBOARD_SCROLLLOCK) {
      Command |= 1;
    }

    Status =  KeyboardWrite (BiosKeyboardPrivate, 0xed);
    if (EFI_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
      goto Done;
    }

    Value     = 0;
    Values[0] = KBC_CMDECHO_ACK;
    Values[1] = KBC_CMDECHO_FAILED_ACK;
    Status = KeyboardWaitForValues (BiosKeyboardPrivate, Values, sizeof (Values), &Value, KEYBOARD_WAITFORVALUE_TIMEOUT);
    if (EFI_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
      goto Done;
    }

    if (Value == KBC_CMDECHO_ACK) {
      Status = KeyboardWrite (BiosKeyboardPrivate, Command);
      if (EFI_ERROR (Status)) {
        Status = EFI_DEVICE_ERROR;
        goto Done;
      }

      Status = KeyboardWaitForValues (BiosKeyboardPrivate, Values, sizeof (Values), &Value, KEYBOARD_WAITFORVALUE_TIMEOUT);
      if (EFI_ERROR (Status)) {
        Status = EFI_DEVICE_ERROR;
        goto Done;
      }
    }
    //
    // Call Legacy BIOS Protocol to set whatever is necessary
    //
    LegacyBios->UpdateKeyboardLedStatus (LegacyBios, Command);
  }
  //
  // Get Configuration
  //
  Regs.H.AH = 0xc0;
  CarryFlag = BiosKeyboardPrivate->LegacyBios->Int86 (
                                                 BiosKeyboardPrivate->LegacyBios,
                                                 0x15,
                                                 &Regs
                                                 );

  if (!CarryFlag) {
    //
    // Check bit 6 of Feature Byte 2.
    // If it is set, then Int 16 Func 09 is supported
    //
    if (*(UINT8 *)(UINTN) ((Regs.X.ES << 4) + Regs.X.BX + 0x06) & 0x40) {
      //
      // Get Keyboard Functionality
      //
      Regs.H.AH = 0x09;
      CarryFlag = BiosKeyboardPrivate->LegacyBios->Int86 (
                                                     BiosKeyboardPrivate->LegacyBios,
                                                     0x16,
                                                     &Regs
                                                     );

      if (!CarryFlag) {
        //
        // Check bit 5 of AH.
        // If it is set, then INT 16 Finc 10-12 are supported.
        //
        if ((Regs.H.AL & 0x40) != 0) {
          //
          // Set the flag to use INT 16 Func 10-12
          //
          BiosKeyboardPrivate->ExtendedKeyboard = TRUE;
        }
      }
    }
  }
  DEBUG ((EFI_D_INFO, "[KBD]Extended keystrokes supported by CSM16 - %02x\n", (UINTN)BiosKeyboardPrivate->ExtendedKeyboard));
  //
  // Setup a periodic timer, used for reading keystrokes at a fixed interval
  //
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  BiosKeyboardTimerHandler,
                  BiosKeyboardPrivate,
                  &BiosKeyboardPrivate->TimerEvent
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
    StatusCode  = EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_CONTROLLER_ERROR;
    goto Done;
  }

  Status = gBS->SetTimer (
                  BiosKeyboardPrivate->TimerEvent,
                  TimerPeriodic,
                  KEYBOARD_TIMER_INTERVAL
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
    StatusCode  = EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_CONTROLLER_ERROR;
    goto Done;
  }
  //
  // Install protocol interfaces for the keyboard device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  &BiosKeyboardPrivate->SimpleTextIn,
                  &gEfiSimpleTextInputExProtocolGuid,
                  &BiosKeyboardPrivate->SimpleTextInputEx,
                  NULL
                  );

Done:
  if (StatusCode != 0) {
    //
    // Report an Error Code for failing to start the keyboard device
    //
    REPORT_STATUS_CODE_WITH_DEVICE_PATH (
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      StatusCode,
      BiosKeyboardPrivate->DevicePath
      );
  }

  if (EFI_ERROR (Status)) {

    if (BiosKeyboardPrivate != NULL) {
      if ((BiosKeyboardPrivate->SimpleTextIn).WaitForKey != NULL) {
        gBS->CloseEvent ((BiosKeyboardPrivate->SimpleTextIn).WaitForKey);
      }

      if ((BiosKeyboardPrivate->SimpleTextInputEx).WaitForKeyEx != NULL) {
        gBS->CloseEvent ((BiosKeyboardPrivate->SimpleTextInputEx).WaitForKeyEx);
      }
      BiosKeyboardFreeNotifyList (&BiosKeyboardPrivate->NotifyList);

      if (BiosKeyboardPrivate->TimerEvent != NULL) {
        gBS->CloseEvent (BiosKeyboardPrivate->TimerEvent);
      }

      FreePool (BiosKeyboardPrivate);
    }

    if (IsaIo != NULL) {
      gBS->CloseProtocol (
             Controller,
             &gEfiIsaIoProtocolGuid,
             This->DriverBindingHandle,
             Controller
             );
    }
  }

  return Status;
}

/**
  Stop the device handled by this driver.

  @param  This                   The driver binding protocol.
  @param  Controller             The controller to release.
  @param  NumberOfChildren       The number of handles in ChildHandleBuffer.
  @param  ChildHandleBuffer      The array of child handle.

  @retval EFI_SUCCESS            The device was stopped.
  @retval EFI_DEVICE_ERROR       The device could not be stopped due to a device error.
  @retval Others                 Fail to uninstall protocols attached on the device.

**/
EFI_STATUS
EFIAPI
BiosKeyboardDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
{
  EFI_STATUS                     Status;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *SimpleTextIn;
  BIOS_KEYBOARD_DEV              *BiosKeyboardPrivate;

  //
  // Disable Keyboard
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  (VOID **) &SimpleTextIn,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleTextInputExProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  BiosKeyboardPrivate = BIOS_KEYBOARD_DEV_FROM_THIS (SimpleTextIn);

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  &BiosKeyboardPrivate->SimpleTextIn,
                  &gEfiSimpleTextInputExProtocolGuid,
                  &BiosKeyboardPrivate->SimpleTextInputEx,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Release the IsaIo protocol on the controller handle
  //
  gBS->CloseProtocol (
         Controller,
         &gEfiIsaIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  //
  // Free other resources
  //
  gBS->CloseEvent ((BiosKeyboardPrivate->SimpleTextIn).WaitForKey);
  gBS->CloseEvent (BiosKeyboardPrivate->TimerEvent);
  gBS->CloseEvent (BiosKeyboardPrivate->SimpleTextInputEx.WaitForKeyEx);
  BiosKeyboardFreeNotifyList (&BiosKeyboardPrivate->NotifyList);

  FreePool (BiosKeyboardPrivate);

  return EFI_SUCCESS;
}

/**
  Read data byte from output buffer of Keyboard Controller without delay and waiting for buffer-empty state.

  @param   BiosKeyboardPrivate  Keyboard instance pointer.

  @return  The data byte read from output buffer of Keyboard Controller from data port which often is port 60H.

**/
UINT8
KeyReadDataRegister (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate
  )
{
  UINT8 Data;

  //
  // Use IsaIo protocol to perform IO operations
  //
  BiosKeyboardPrivate->IsaIo->Io.Read (
                                   BiosKeyboardPrivate->IsaIo,
                                   EfiIsaIoWidthUint8,
                                   BiosKeyboardPrivate->DataRegisterAddress,
                                   1,
                                   &Data
                                   );

  return Data;
}

/**
  Read status byte from status register of Keyboard Controller without delay and waiting for buffer-empty state.

  @param   BiosKeyboardPrivate  Keyboard instance pointer.

  @return  The status byte read from status register of Keyboard Controller from command port which often is port 64H.

**/
UINT8
KeyReadStatusRegister (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate
  )
{
  UINT8 Data;

  //
  // Use IsaIo protocol to perform IO operations
  //
  BiosKeyboardPrivate->IsaIo->Io.Read (
                                   BiosKeyboardPrivate->IsaIo,
                                   EfiIsaIoWidthUint8,
                                   BiosKeyboardPrivate->StatusRegisterAddress,
                                   1,
                                   &Data
                                   );

  return Data;
}

/**
  Write command byte to control register of Keyboard Controller without delay and waiting for buffer-empty state.

  @param   BiosKeyboardPrivate  Keyboard instance pointer.
  @param   Data                 Data byte to write.

**/
VOID
KeyWriteCommandRegister (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  IN UINT8              Data
  )
{
  //
  // Use IsaIo protocol to perform IO operations
  //
  BiosKeyboardPrivate->IsaIo->Io.Write (
                                   BiosKeyboardPrivate->IsaIo,
                                   EfiIsaIoWidthUint8,
                                   BiosKeyboardPrivate->CommandRegisterAddress,
                                   1,
                                   &Data
                                   );
}

/**
  Write data byte to input buffer or input/output ports of Keyboard Controller without delay and waiting for buffer-empty state.

  @param   BiosKeyboardPrivate  Keyboard instance pointer.
  @param   Data                 Data byte to write.

**/
VOID
KeyWriteDataRegister (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  IN UINT8              Data
  )
{
  //
  // Use IsaIo protocol to perform IO operations
  //
  BiosKeyboardPrivate->IsaIo->Io.Write (
                                   BiosKeyboardPrivate->IsaIo,
                                   EfiIsaIoWidthUint8,
                                   BiosKeyboardPrivate->DataRegisterAddress,
                                   1,
                                   &Data
                                   );
}

/**
  Read data byte from output buffer of Keyboard Controller with delay and waiting for buffer-empty state.

  @param   BiosKeyboardPrivate  Keyboard instance pointer.
  @param   Data                 The pointer for data that being read out.

  @retval  EFI_SUCCESS          The data byte read out successfully.
  @retval  EFI_TIMEOUT          Timeout occurred during reading out data byte.

**/
EFI_STATUS
KeyboardRead (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  OUT UINT8             *Data
  )
{
  UINT32  TimeOut;
  UINT32  RegFilled;

  TimeOut   = 0;
  RegFilled = 0;

  //
  // wait till output buffer full then perform the read
  //
  for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += 30) {
    if ((KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_OUTB) != 0) {
      RegFilled = 1;
      *Data     = KeyReadDataRegister (BiosKeyboardPrivate);
      break;
    }

    gBS->Stall (30);
  }

  if (RegFilled == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Write data byte to input buffer or input/output ports of Keyboard Controller with delay and waiting for buffer-empty state.

  @param   BiosKeyboardPrivate  Keyboard instance pointer.
  @param   Data                 Data byte to write.

  @retval  EFI_SUCCESS          The data byte is written successfully.
  @retval  EFI_TIMEOUT          Timeout occurred during writing.

**/
EFI_STATUS
KeyboardWrite (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  IN UINT8              Data
  )
{
  UINT32  TimeOut;
  UINT32  RegEmptied;

  TimeOut     = 0;
  RegEmptied  = 0;

  //
  // wait for input buffer empty
  //
  for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += 30) {
    if ((KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_INPB) == 0) {
      RegEmptied = 1;
      break;
    }

    gBS->Stall (30);
  }

  if (RegEmptied == 0) {
    return EFI_TIMEOUT;
  }
  //
  // Write it
  //
  KeyWriteDataRegister (BiosKeyboardPrivate, Data);

  return EFI_SUCCESS;
}

/**
  Write command byte to control register of Keyboard Controller with delay and waiting for buffer-empty state.

  @param   BiosKeyboardPrivate  Keyboard instance pointer.
  @param   Data                 Command byte to write.

  @retval  EFI_SUCCESS          The command byte is written successfully.
  @retval  EFI_TIMEOUT          Timeout occurred during writing.

**/
EFI_STATUS
KeyboardCommand (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  IN UINT8              Data
  )
{
  UINT32  TimeOut;
  UINT32  RegEmptied;

  TimeOut     = 0;
  RegEmptied  = 0;

  //
  // Wait For Input Buffer Empty
  //
  for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += 30) {
    if ((KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_INPB) == 0) {
      RegEmptied = 1;
      break;
    }

    gBS->Stall (30);
  }

  if (RegEmptied == 0) {
    return EFI_TIMEOUT;
  }
  //
  // issue the command
  //
  KeyWriteCommandRegister (BiosKeyboardPrivate, Data);

  //
  // Wait For Input Buffer Empty again
  //
  RegEmptied = 0;
  for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += 30) {
    if ((KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_INPB) == 0) {
      RegEmptied = 1;
      break;
    }

    gBS->Stall (30);
  }

  if (RegEmptied == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Wait for a specific value to be presented in
  Data register of Keyboard Controller by keyboard and then read it,
  used in keyboard commands ack

  @param   BiosKeyboardPrivate  Keyboard instance pointer.
  @param   Value                The value to be waited for
  @param   WaitForValueTimeOut  The limit of microseconds for timeout

  @retval  EFI_SUCCESS          The command byte is written successfully.
  @retval  EFI_TIMEOUT          Timeout occurred during writing.

**/
EFI_STATUS
KeyboardWaitForValue (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  IN UINT8              Value,
  IN UINTN              WaitForValueTimeOut
  )
{
  UINT8   Data;
  UINT32  TimeOut;
  UINT32  SumTimeOut;
  UINT32  GotIt;

  GotIt       = 0;
  TimeOut     = 0;
  SumTimeOut  = 0;

  //
  // Make sure the initial value of 'Data' is different from 'Value'
  //
  Data = 0;
  if (Data == Value) {
    Data = 1;
  }
  //
  // Read from 8042 (multiple times if needed)
  // until the expected value appears
  // use SumTimeOut to control the iteration
  //
  while (1) {
    //
    // Perform a read
    //
    for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += 30) {
      if ((KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_OUTB) != 0) {
        Data = KeyReadDataRegister (BiosKeyboardPrivate);
        break;
      }

      gBS->Stall (30);
    }

    SumTimeOut += TimeOut;

    if (Data == Value) {
      GotIt = 1;
      break;
    }

    if (SumTimeOut >= WaitForValueTimeOut) {
      break;
    }
  }
  //
  // Check results
  //
  if (GotIt != 0) {
    return EFI_SUCCESS;
  } else {
    return EFI_TIMEOUT;
  }

}

/**
  Wait for a specific value to be presented in
  Data register of Keyboard Controller by keyboard and then read it,
  used in keyboard commands ack or failed ack

  @param[in]  BiosKeyboardPrivate  The BIOS_KEYBOARD_DEV instance pointer
  @param[in]  Values               The any of the values to be waited for.
  @param[in]  NumOfValues          Number of values to be waited for.
  @param[out] Value                Output buffer to save the value which receive from data port and the value
                                   is one of the "Values".
  @param[in]  WaitForValueTimeOut  The limit of microseconds for timeout.

  @retval  EFI_SUCCESS             Receive one of the Values from data port successfully.
  @retval  EFI_TIMEOUT             Cannot receive one of the Values during period of time.
**/
EFI_STATUS
KeyboardWaitForValues (
  IN  BIOS_KEYBOARD_DEV       *BiosKeyboardPrivate,
  IN  UINT8                   *Values,
  IN  UINTN                   NumOfValues,
  OUT UINT8                   *Value,
  IN  UINTN                   WaitForValueTimeOut
  )
{
  UINT8   Data;
  UINT32  TimeOut;
  UINT32  SumTimeOut;
  BOOLEAN ReceiveData;
  UINTN   Index;

  SumTimeOut  = 0;
  Data        = 0;
  ReceiveData = FALSE;
  //
  // Read from 8042 (multiple times if needed)
  // until the expected value appears
  // use SumTimeOut to control the iteration
  //
  while (TRUE) {
    //
    // Perform a read
    //
    for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += 30) {
      if (KeyReadStatusRegister (BiosKeyboardPrivate) & 0x01) {
        Data = KeyReadDataRegister (BiosKeyboardPrivate);
        ReceiveData = TRUE;
        break;
      }

      gBS->Stall (30);
    }

    SumTimeOut += TimeOut;

    if (ReceiveData) {
      for (Index = 0; Index < NumOfValues; Index++) {
        if(Values[Index] == Data) {
          *Value = Data;
          return EFI_SUCCESS;
        }
      }
      ReceiveData = FALSE;
    }

    if (SumTimeOut >= WaitForValueTimeOut) {
      break;
    }
  }

  return EFI_TIMEOUT;
}

/**
  Reads the next keystroke from the input device. The WaitForKey Event can
  be used to test for existance of a keystroke via WaitForEvent () call.

  @param  BiosKeyboardPrivate   Bioskeyboard driver private structure.
  @param  KeyData               A pointer to a buffer that is filled in with the keystroke
                                state data for the key that was pressed.

  @retval EFI_SUCCESS           The keystroke information was returned.
  @retval EFI_NOT_READY         There was no keystroke data availiable.
  @retval EFI_DEVICE_ERROR      The keystroke information was not returned due to
                                hardware errors.
  @retval EFI_INVALID_PARAMETER KeyData is NULL.

**/
EFI_STATUS
KeyboardReadKeyStrokeWorker (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  OUT EFI_KEY_DATA      *KeyData
  )
{
  EFI_STATUS                            Status;
  EFI_TPL                               OldTpl;

  if (KeyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  Status = CheckQueue (&BiosKeyboardPrivate->Queue);
  if (EFI_ERROR (Status)) {
    gBS->RestoreTPL (OldTpl);
    return EFI_NOT_READY;
  }

  Status = Dequeue (&BiosKeyboardPrivate->Queue, KeyData);

  gBS->RestoreTPL (OldTpl);

  return EFI_SUCCESS;
}

//
// EFI Simple Text In Protocol Functions
//
/**
  Reset the Keyboard and do BAT test for it, if (ExtendedVerification == TRUE) then do some extra keyboard validations.

  @param  This                  Pointer of simple text Protocol.
  @param  ExtendedVerification  Whether perform the extra validation of keyboard. True: perform; FALSE: skip.

  @retval EFI_SUCCESS           The command byte is written successfully.
  @retval EFI_DEVICE_ERROR      Errors occurred during reseting keyboard.

**/
EFI_STATUS
EFIAPI
BiosKeyboardReset (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  IN  BOOLEAN                         ExtendedVerification
  )
{
  BIOS_KEYBOARD_DEV *BiosKeyboardPrivate;
  EFI_STATUS        Status;
  EFI_TPL           OldTpl;
  UINT8             CommandByte;
  BOOLEAN           MouseEnable;
  EFI_KEY_DATA      KeyData;

  MouseEnable         = FALSE;
  BiosKeyboardPrivate = BIOS_KEYBOARD_DEV_FROM_THIS (This);

  //
  // 1
  // Report reset progress code
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_RESET,
    BiosKeyboardPrivate->DevicePath
    );

  //
  // Report a Progress Code for clearing the keyboard buffer
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_KEYBOARD | EFI_P_KEYBOARD_PC_CLEAR_BUFFER,
    BiosKeyboardPrivate->DevicePath
    );

  //
  // 2
  // Raise TPL to avoid mouse operation impact
  //
  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  //
  // Disable keyboard itself (via KBC) by writing CMD AD via 64H
  //
  Status = KeyboardCommand (
             BiosKeyboardPrivate,
             KBC_CMDREG_VIA64_KB_DISABLE
             );

  if (EFI_ERROR (Status)) {
    Status    = EFI_DEVICE_ERROR;
    goto Exit;
  }

  //
  //
  // Exhaust output buffer data
  //
  if (KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_OUTB) {
    do {
      BiosKeyboardTimerHandler (NULL, BiosKeyboardPrivate);
      Status = Dequeue (&BiosKeyboardPrivate->Queue, &KeyData);
      if (EFI_ERROR (Status)) {
        break;
      }
    } while (!EFI_ERROR (Status));
  }

  //
  // enable keyboard itself (via KBC) by writing CMD AE via 64H
  //
  Status = KeyboardCommand (
             BiosKeyboardPrivate,
             KBC_CMDREG_VIA64_KB_ENABLE
             );

  if (EFI_ERROR (Status)) {
    Status    = EFI_DEVICE_ERROR;
    goto Exit;
  }

  //
  // 3
  // check for KBC itself firstly for setted-up already or not by reading SYSF (bit2) of status register via 64H
  // if not skip step 4&5 and jump to step 6 to selftest KBC and report this
  // else   go step 4
  //
  if (!PcdGetBool (PcdFastPS2Detection)) {
    if ((KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_SYSF) != 0) {
      //
      // 4
      // CheckMouseStatus to decide enable it later or not
      //
      //
      // Read the command byte of KBC
      //
      Status = KeyboardCommand (
                 BiosKeyboardPrivate,
                 KBC_CMDREG_VIA64_CMDBYTE_R
                 );

      if (EFI_ERROR (Status)) {
        Status    = EFI_DEVICE_ERROR;
        goto Exit;
      }

      Status = KeyboardRead (
                 BiosKeyboardPrivate,
                 &CommandByte
                 );

      if (EFI_ERROR (Status)) {
        Status    = EFI_DEVICE_ERROR;
        goto Exit;
      }
      //
      // Check mouse enabled or not before
      //
      if ((CommandByte & KB_CMMBYTE_DISABLE_AUX) != 0) {
        MouseEnable = FALSE;
      } else {
        MouseEnable = TRUE;
      }
      //
      // 5
      // disable mouse (via KBC) and Keyborad device
      //
      Status = KeyboardCommand (
                 BiosKeyboardPrivate,
                 KBC_CMDREG_VIA64_AUX_DISABLE
                 );

      if (EFI_ERROR (Status)) {
        Status    = EFI_DEVICE_ERROR;
        goto Exit;
      }

      Status = KeyboardCommand (
                 BiosKeyboardPrivate,
                 KBC_CMDREG_VIA64_KB_DISABLE
                 );

      if (EFI_ERROR (Status)) {
        Status    = EFI_DEVICE_ERROR;
        goto Exit;
      }

    } else {
      //
      // 6
      // KBC Self Test
      //
      //
      // Report a Progress Code for performing a self test on the keyboard controller
      //
      REPORT_STATUS_CODE_WITH_DEVICE_PATH (
        EFI_PROGRESS_CODE,
        EFI_PERIPHERAL_KEYBOARD | EFI_P_KEYBOARD_PC_SELF_TEST,
        BiosKeyboardPrivate->DevicePath
        );

      Status = KeyboardCommand (
                 BiosKeyboardPrivate,
                 KBC_CMDREG_VIA64_KBC_SLFTEST
                 );
      if (EFI_ERROR (Status)) {
        Status    = EFI_DEVICE_ERROR;
        goto Exit;
      }

      Status = KeyboardWaitForValue (
                 BiosKeyboardPrivate,
                 KBC_CMDECHO_KBCSLFTEST_OK,
                 KEYBOARD_WAITFORVALUE_TIMEOUT
                 );
      if (EFI_ERROR (Status)) {
        Status    = EFI_DEVICE_ERROR;
        goto Exit;
      }
    }
  }
  //
  // 7
  // Disable  Mouse interface, enable  Keyboard interface and declare selftest success
  //
  // Mouse device will block keyboard interface before it be configured, so we should disable mouse first.
  //
  Status = KeyboardCommand (
             BiosKeyboardPrivate,
             KBC_CMDREG_VIA64_CMDBYTE_W
             );

  if (EFI_ERROR (Status)) {
    Status    = EFI_DEVICE_ERROR;
    goto Exit;
  }

  //
  // Write 8042 Command Byte, set System Flag
  // While at the same time:
  //  1. disable mouse interface,
  //  2. enable kbd interface,
  //  3. enable PC/XT kbd translation mode
  //  4. enable mouse and kbd interrupts
  //
  //Command Byte bits:
  //  7: Reserved
  //  6: PC/XT translation mode
  //  5: Disable Auxiliary device interface
  //  4: Disable keyboard interface
  //  3: Reserved
  //  2: System Flag
  //  1: Enable Auxiliary device interrupt
  //  0: Enable Keyboard interrupt
  //
  CommandByte = 0;
  Status = KeyboardWrite (
             BiosKeyboardPrivate,
             (UINT8) ((CommandByte &
              (~KB_CMMBYTE_DISABLE_KB)) |
              KB_CMMBYTE_KSCAN2UNI_COV |
              KB_CMMBYTE_ENABLE_AUXINT |
              KB_CMMBYTE_ENABLE_KBINT  |
              KB_CMMBYTE_SLFTEST_SUCC  |
              KB_CMMBYTE_DISABLE_AUX)
             );

  //
  // For reseting keyboard is not mandatory before booting OS and sometimes keyboard responses very slow,
  // so we only do the real reseting for keyboard when user asks, and normally during booting an OS, it's skipped.
  // Call CheckKeyboardConnect() to check whether keyboard is connected, if it is not connected,
  // Real reset will not do.
  //
  if (ExtendedVerification && CheckKeyboardConnect (BiosKeyboardPrivate)) {
    //
    // 8
    // Send keyboard reset command then read ACK
    //
    Status = KeyboardWrite (
               BiosKeyboardPrivate,
               KBC_INPBUF_VIA60_KBRESET
               );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Status = KeyboardWaitForValue (
               BiosKeyboardPrivate,
               KBC_CMDECHO_ACK,
               KEYBOARD_WAITFORVALUE_TIMEOUT
               );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }
    //
    // 9
    // Wait for keyboard return test OK.
    //
    Status = KeyboardWaitForValue (
               BiosKeyboardPrivate,
               KBC_CMDECHO_BATTEST_OK,
               KEYBOARD_WAITFORVALUE_TIMEOUT
               );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }
    //
    // 10
    // set keyboard scan code set = 02 (standard configuration)
    //
    Status = KeyboardWrite (
               BiosKeyboardPrivate,
               KBC_INPBUF_VIA60_KBSCODE
               );
    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Status = KeyboardWaitForValue (
               BiosKeyboardPrivate,
               KBC_CMDECHO_ACK,
               KEYBOARD_WAITFORVALUE_TIMEOUT
               );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Status = KeyboardWrite (
               BiosKeyboardPrivate,
               KBC_INPBUF_VIA60_SCODESET2
               );
    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Status = KeyboardWaitForValue (
               BiosKeyboardPrivate,
               KBC_CMDECHO_ACK,
               KEYBOARD_WAITFORVALUE_TIMEOUT
               );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }
    //
    // 11
    // enable keyboard itself (not via KBC) by writing CMD F4 via 60H
    //
    Status = KeyboardWrite (
               BiosKeyboardPrivate,
               KBC_INPBUF_VIA60_KBEN
               );
    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Status = KeyboardWaitForValue (
               BiosKeyboardPrivate,
               KBC_CMDECHO_ACK,
               KEYBOARD_WAITFORVALUE_TIMEOUT
               );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }
    //
    // 12
    // Additional validation, do it as follow:
    // 1). check for status register of PARE && TIM via 64H
    // 2). perform KB checking by writing ABh via 64H
    //
    if ((KeyReadStatusRegister (BiosKeyboardPrivate) & (KBC_STSREG_VIA64_PARE | KBC_STSREG_VIA64_TIM)) != 0) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Status = KeyboardCommand (
               BiosKeyboardPrivate,
               KBC_CMDREG_VIA64_KB_CKECK
               );
    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Status = KeyboardWaitForValue (
               BiosKeyboardPrivate,
               KBC_CMDECHO_KBCHECK_OK,
               KEYBOARD_WAITFORVALUE_TIMEOUT
               );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }
  }
  //
  // 13
  // Done for validating keyboard. Enable keyboard (via KBC)
  // and recover the command byte to proper value
  //
  if (!PcdGetBool (PcdFastPS2Detection)) {
    Status = KeyboardCommand (
               BiosKeyboardPrivate,
               KBC_CMDREG_VIA64_KB_ENABLE
               );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      goto Exit;
    }
  }

  //
  // 14
  // conditionally enable mouse (via KBC)
  //
  if (MouseEnable) {
    Status = KeyboardCommand (
               BiosKeyboardPrivate,
               KBC_CMDREG_VIA64_AUX_ENABLE
               );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;

    }
  }

Exit:
  //
  // 15
  // resume priority of task level
  //
  gBS->RestoreTPL (OldTpl);

  return Status;

}

/**
  Read out the scan code of the key that has just been stroked.

  @param  This        Pointer of simple text Protocol.
  @param  Key         Pointer for store the key that read out.

  @retval EFI_SUCCESS The key is read out successfully.
  @retval other       The key reading failed.

**/
EFI_STATUS
EFIAPI
BiosKeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                   *Key
  )
{
  BIOS_KEYBOARD_DEV     *BiosKeyboardPrivate;
  EFI_STATUS            Status;
  EFI_KEY_DATA          KeyData;

  BiosKeyboardPrivate = BIOS_KEYBOARD_DEV_FROM_THIS (This);

  Status = KeyboardReadKeyStrokeWorker (BiosKeyboardPrivate, &KeyData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (KeyData.Key.ScanCode == SCAN_NULL && KeyData.Key.UnicodeChar == 0x00 ) {
    return EFI_NOT_READY;
  }
  //
  // Translate the CTRL-Alpha characters to their corresponding control value
  // (ctrl-a = 0x0001 through ctrl-Z = 0x001A)
  //
  if ((KeyData.KeyState.KeyShiftState & (EFI_LEFT_CONTROL_PRESSED | EFI_RIGHT_CONTROL_PRESSED)) != 0) {
    if (KeyData.Key.UnicodeChar >= L'a' && KeyData.Key.UnicodeChar <= L'z') {
      KeyData.Key.UnicodeChar = (CHAR16) (KeyData.Key.UnicodeChar - L'a' + 1);
    } else if (KeyData.Key.UnicodeChar >= L'A' && KeyData.Key.UnicodeChar <= L'Z') {
      KeyData.Key.UnicodeChar = (CHAR16) (KeyData.Key.UnicodeChar - L'A' + 1);
    } else if (KeyData.Key.UnicodeChar == '[' || KeyData.Key.UnicodeChar == '{') {
      KeyData.Key.ScanCode    = SCAN_ESC;
      KeyData.Key.UnicodeChar = CHAR_NULL;
    } else if (KeyData.Key.UnicodeChar == '\\' || KeyData.Key.UnicodeChar == '|') {
      KeyData.Key.UnicodeChar = 0x1C;
    } else if (KeyData.Key.UnicodeChar == ']' || KeyData.Key.UnicodeChar == '}') {
      KeyData.Key.UnicodeChar = 0x1D;
    } else if (KeyData.Key.UnicodeChar == '^' || KeyData.Key.UnicodeChar == '6') {
      KeyData.Key.UnicodeChar = 0x1E;
    } else if (KeyData.Key.UnicodeChar == '_' || KeyData.Key.UnicodeChar == '-') {
      KeyData.Key.UnicodeChar = 0x1F;
    }
  }

  CopyMem (Key, &KeyData.Key, sizeof (EFI_INPUT_KEY));

  return EFI_SUCCESS;
}

/**
  Waiting on the keyboard event, if there's any key pressed by the user, signal the event

  @param  Event       The event that be siganlled when any key has been stroked.
  @param  Context     Pointer of the protocol EFI_SIMPLE_TEXT_INPUT_PROTOCOL.

**/
VOID
EFIAPI
BiosKeyboardWaitForKey (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
{
  if (!EFI_ERROR (BiosKeyboardCheckForKey (Context))) {
    gBS->SignalEvent (Event);
  }
}

/**
  Check key buffer to get the key stroke status.

  @param  This         Pointer of the protocol EFI_SIMPLE_TEXT_IN_PROTOCOL.

  @retval EFI_SUCCESS  A key is being pressed now.
  @retval Other        No key is now pressed.

**/
EFI_STATUS
EFIAPI
BiosKeyboardCheckForKey (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This
  )
{
  BIOS_KEYBOARD_DEV     *BiosKeyboardPrivate;

  BiosKeyboardPrivate = BIOS_KEYBOARD_DEV_FROM_THIS (This);

  return CheckQueue (&BiosKeyboardPrivate->Queue);
}

/**
  Convert unicode combined with scan code of key to the counterpart of EFIScancode of it.

  @param  KeyChar      Unicode of key.
  @param  ScanCode     Scan code of key.

  @return The value of EFI Scancode for the key.
  @retval SCAN_NULL   No corresponding value in the EFI convert table is found for the key.

**/
UINT16
ConvertToEFIScanCode (
  IN  EFI_KEY_DATA      *KeyData
  )
{
  CHAR16  KeyChar;
  UINT16  ScanCode;
  UINT16  EfiScanCode;
  UINT16  Index;

  KeyChar  = KeyData->Key.UnicodeChar;
  ScanCode = KeyData->Key.ScanCode;
  if (KeyChar == CHAR_ESC) {
    EfiScanCode = SCAN_ESC;
  } else if (KeyChar == 0x00 || KeyChar == 0xe0) {
    for (Index = 0; KbcCodeTable[Index][0] != TABLE_END; Index += 1) {
      if (KeyData->KeyState.KeyShiftState & (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED)) {
        if (ScanCode == KbcCodeTable[Index][2]) {
          return KbcCodeTable[Index][0];
        }
      } else if (KeyData->KeyState.KeyShiftState & (EFI_LEFT_ALT_PRESSED | EFI_RIGHT_ALT_PRESSED)) {
        if (ScanCode == KbcCodeTable[Index][3]) {
          return KbcCodeTable[Index][0];
        }
      } else if (KeyData->KeyState.KeyShiftState & (EFI_LEFT_CONTROL_PRESSED | EFI_RIGHT_CONTROL_PRESSED)) {
        if (ScanCode == KbcCodeTable[Index][4]) {
          return KbcCodeTable[Index][0];
        }
      }
      if (ScanCode == KbcCodeTable[Index][1]) {
        return KbcCodeTable[Index][0];
      }
    }
    //
    // Reach Table end, return default value
    //
    return SCAN_NULL;
  } else {
    return SCAN_NULL;
  }

  return EfiScanCode;
}

/**
  Check whether there is Ps/2 Keyboard device in system by 0xF4 Keyboard Command
  If Keyboard receives 0xF4, it will respond with 'ACK'. If it doesn't respond, the device
  should not be in system.

  @param  BiosKeyboardPrivate  Keyboard Private Data Struture

  @retval TRUE  Keyboard in System.
  @retval FALSE Keyboard not in System.

**/
BOOLEAN
CheckKeyboardConnect (
  IN  BIOS_KEYBOARD_DEV     *BiosKeyboardPrivate
  )
{
  EFI_STATUS     Status;
  UINT8          Values[2];
  UINT8          Value;

  Status         = EFI_SUCCESS;
  //
  // clear previous data
  //
  while (KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_OUTB) {
    KeyReadDataRegister (BiosKeyboardPrivate);
    gBS->Stall (15);
  }
  //
  // enable keyboard itself and wait for its ack
  // If can't receive ack, Keyboard should not be connected.
  //
  if (!PcdGetBool (PcdFastPS2Detection)) {
    Status = KeyboardWrite (
               BiosKeyboardPrivate,
               KBC_INPBUF_VIA60_KBEN
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "[KBD]CheckKeyboardConnect - Keyboard enable failed!\n"));
      REPORT_STATUS_CODE_WITH_DEVICE_PATH (
        EFI_ERROR_CODE | EFI_ERROR_MINOR,
        EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_CONTROLLER_ERROR,
        BiosKeyboardPrivate->DevicePath
        );
      return FALSE;
    }
    //
    // Also try to check failed ACK value to reduce waiting time.
    //
    Value = 0;
    Values[0] = KBC_CMDECHO_ACK;
    Values[1] = KBC_CMDECHO_FAILED_ACK;
    Status = KeyboardWaitForValues (
               BiosKeyboardPrivate,
               Values,
               sizeof (Values),
               &Value,
               KEYBOARD_WAITFORVALUE_TIMEOUT
               );

    if (EFI_ERROR (Status) || Value == KBC_CMDECHO_FAILED_ACK ) {
      DEBUG ((EFI_D_ERROR, "[KBD]CheckKeyboardConnect - Timeout!\n"));
      REPORT_STATUS_CODE_WITH_DEVICE_PATH (
        EFI_ERROR_CODE | EFI_ERROR_MINOR,
        EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_CONTROLLER_ERROR,
        BiosKeyboardPrivate->DevicePath
        );
      return FALSE;
    }

    return TRUE;
  } else {
    return TRUE;
  }
}

/**
  Timer event handler: read a series of key stroke from 8042
  and put them into memory key buffer.
  It is registered as running under TPL_NOTIFY

  @param  Event   The timer event
  @param  Context A BIOS_KEYBOARD_DEV pointer

**/
VOID
EFIAPI
BiosKeyboardTimerHandler (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  EFI_TPL                            OldTpl;
  BIOS_KEYBOARD_DEV                  *BiosKeyboardPrivate;
  EFI_IA32_REGISTER_SET              Regs;
  UINT8                              KbFlag1;  // 0040h:0017h - KEYBOARD - STATUS FLAGS 1
  UINT8                              KbFlag2;  // 0040h:0018h - KEYBOARD - STATUS FLAGS 2
  EFI_KEY_DATA                       KeyData;
  LIST_ENTRY                         *Link;
  BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY *CurrentNotify;
  UINTN                              Index;

  BiosKeyboardPrivate = Context;

  //
  // Enter critical section
  //
  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  //
  // if there is no key present, just return
  //
  if (BiosKeyboardPrivate->ExtendedKeyboard) {
    Regs.H.AH = 0x11;
  } else {
    Regs.H.AH = 0x01;
  }

  BiosKeyboardPrivate->LegacyBios->Int86 (
                                     BiosKeyboardPrivate->LegacyBios,
                                     0x16,
                                     &Regs
                                     );
  if (Regs.X.Flags.ZF != 0) {
    gBS->RestoreTPL (OldTpl);
    return;
  }
  //
  // Read the key
  //
  if (BiosKeyboardPrivate->ExtendedKeyboard) {
    Regs.H.AH = 0x10;
  } else {
    Regs.H.AH = 0x00;
  }

  BiosKeyboardPrivate->LegacyBios->Int86 (
                                     BiosKeyboardPrivate->LegacyBios,
                                     0x16,
                                     &Regs
                                     );

  KeyData.Key.ScanCode            = (UINT16) Regs.H.AH;
  KeyData.Key.UnicodeChar         = (UINT16) Regs.H.AL;
  DEBUG ((
    EFI_D_INFO,
    "[KBD]INT16 returns EFI_INPUT_KEY.ScanCode - %x, EFI_INPUT_KEY.UnicodeChar - %x\n",
    KeyData.Key.ScanCode,
    KeyData.Key.UnicodeChar
    ));

  KeyData.KeyState.KeyShiftState  = EFI_SHIFT_STATE_VALID;
  KeyData.KeyState.KeyToggleState = EFI_TOGGLE_STATE_VALID;
  if (IS_EBDA_VALID) EBDA16(EBDA_KEYBORD_SCAN_CODE) = KeyData.Key.ScanCode ;
  //
  // Leagcy Bios use Int 9 which is IRQ1 interrupt handler to get keystroke scancode to KB  buffer in BDA (BIOS DATE AREA),  then
  // Int 16 depend  KB buffer and some key bits in BDA to translate the scancode to ASCII code, and  return both the scancode and ASCII
  // code to Int 16 caller. This translation process works well if the Int 9  could response user input in time. But in Tiano enviorment,  the Int 9
  // will be disabled after the thunk call finish, which means if user crazy input during int 9 being disabled, some keystrokes will be lost when
  // KB device own hardware buffer overflows. And if the lost keystroke code is CTRL or ALT or SHIFT release code, these function key flags bit
  // in BDA will not be updated. So the Int 16 will believe the CTRL or ALT or SHIFT is still pressed, and Int 16 will translate later scancode
  // to wrong ASCII code. We can increase the Thunk frequence to let Int 9 response in time, but this way will much hurt other dirvers
  // performance, like USB.
  //
  // 1. If CTRL or ALT release code is missed,  all later input keys will be translated to wrong ASCII codes which the Tiano cannot support. In
  //     this case, the KB input seems fail to work, and user input is blocked. To solve the problem, we can help to clear the CTRL or ALT flag in BDA
  //    after every Int 16 finish. Thus persist to press CTRL or ALT has same effection as only press one time. It is Ok, since user not often use the
  //    CTRL and ALT.
  //
  // 2. If SHIFT release code is missed, all later lowercase input will become capital. This is ugly, but not block user input. If user press the lost
  //     SHIFT again,  the lowercase will come back to normal. Since user often use the SHIFT, it is not reasonable to help to clear the SHIFT flag in BDA,
  //     which will let persist to press SHIFT has same effection as only press one time.
  //
  //0040h:0017h - KEYBOARD - STATUS FLAGS 1
  //   7 INSert active
  //   6 Caps Lock active
  //   5 Num Lock active
  //   4 Scroll Lock active
  //   3 either Alt pressed
  //   2 either Ctrl pressed
  //   1 Left Shift pressed
  //   0 Right Shift pressed


  //
  // Clear the CTRL and ALT BDA flag
  //
  KbFlag1 = *((UINT8 *) (UINTN) 0x417);  // read the STATUS FLAGS 1
  KbFlag2 = *((UINT8 *) (UINTN) 0x418); // read STATUS FLAGS 2

  DEBUG_CODE (
    {
      if ((KbFlag1 & KB_CAPS_LOCK_BIT) == KB_CAPS_LOCK_BIT) {
        DEBUG ((EFI_D_INFO, "[KBD]Caps Lock Key is pressed.\n"));
      }
      if ((KbFlag1 & KB_NUM_LOCK_BIT) == KB_NUM_LOCK_BIT) {
        DEBUG ((EFI_D_INFO, "[KBD]Num Lock Key is pressed.\n"));
      }
      if ((KbFlag1 & KB_SCROLL_LOCK_BIT) == KB_SCROLL_LOCK_BIT) {
        DEBUG ((EFI_D_INFO, "[KBD]Scroll Lock Key is pressed.\n"));
      }
      if ((KbFlag1 & KB_ALT_PRESSED) == KB_ALT_PRESSED) {
        if ((KbFlag2 & KB_LEFT_ALT_PRESSED) == KB_LEFT_ALT_PRESSED) {
          DEBUG ((EFI_D_INFO, "[KBD]Left Alt Key is pressed.\n"));
        } else {
          DEBUG ((EFI_D_INFO, "[KBD]Right Alt Key is pressed.\n"));
        }
      }
      if ((KbFlag1 & KB_CTRL_PRESSED) == KB_CTRL_PRESSED) {
        if ((KbFlag2 & KB_LEFT_CTRL_PRESSED) == KB_LEFT_CTRL_PRESSED) {
          DEBUG ((EFI_D_INFO, "[KBD]Left Ctrl Key is pressed.\n"));
        } else {
          DEBUG ((EFI_D_INFO, "[KBD]Right Ctrl Key is pressed.\n"));
        }
      }
      if ((KbFlag1 & KB_LEFT_SHIFT_PRESSED) == KB_LEFT_SHIFT_PRESSED) {
        DEBUG ((EFI_D_INFO, "[KBD]Left Shift Key is pressed.\n"));
      }
      if ((KbFlag1 & KB_RIGHT_SHIFT_PRESSED) == KB_RIGHT_SHIFT_PRESSED) {
        DEBUG ((EFI_D_INFO, "[KBD]Right Shift Key is pressed.\n"));
      }
    }
  );

  //
  // Record toggle state
  //
  if ((KbFlag1 & KB_CAPS_LOCK_BIT) == KB_CAPS_LOCK_BIT) {
    KeyData.KeyState.KeyToggleState |= EFI_CAPS_LOCK_ACTIVE;
  }
  if ((KbFlag1 & KB_NUM_LOCK_BIT) == KB_NUM_LOCK_BIT) {
    KeyData.KeyState.KeyToggleState |= EFI_NUM_LOCK_ACTIVE;
  }
  if ((KbFlag1 & KB_SCROLL_LOCK_BIT) == KB_SCROLL_LOCK_BIT) {
    KeyData.KeyState.KeyToggleState |= EFI_SCROLL_LOCK_ACTIVE;
  }
  //
  // Record shift state
  // BUGBUG: Need add Menu key and Left/Right Logo key state in the future
  //
  if ((KbFlag1 & KB_ALT_PRESSED) == KB_ALT_PRESSED) {
    KeyData.KeyState.KeyShiftState  |= ((KbFlag2 & KB_LEFT_ALT_PRESSED) == KB_LEFT_ALT_PRESSED) ? EFI_LEFT_ALT_PRESSED : EFI_RIGHT_ALT_PRESSED;
  }
  if ((KbFlag1 & KB_CTRL_PRESSED) == KB_CTRL_PRESSED) {
    KeyData.KeyState.KeyShiftState  |= ((KbFlag2 & KB_LEFT_CTRL_PRESSED) == KB_LEFT_CTRL_PRESSED) ? EFI_LEFT_CONTROL_PRESSED : EFI_RIGHT_CONTROL_PRESSED;
  }
  if ((KbFlag1 & KB_LEFT_SHIFT_PRESSED) == KB_LEFT_SHIFT_PRESSED) {
    KeyData.KeyState.KeyShiftState  |= EFI_LEFT_SHIFT_PRESSED;
  }
  if ((KbFlag1 & KB_RIGHT_SHIFT_PRESSED) == KB_RIGHT_SHIFT_PRESSED) {
    KeyData.KeyState.KeyShiftState  |= EFI_RIGHT_SHIFT_PRESSED;
  }

  //
  // Output EFI input key and shift/toggle state
  //
  if (KeyData.Key.UnicodeChar == CHAR_NULL || KeyData.Key.UnicodeChar == CHAR_SCANCODE || KeyData.Key.UnicodeChar == CHAR_ESC) {
    KeyData.Key.ScanCode     = ConvertToEFIScanCode (&KeyData);
    KeyData.Key.UnicodeChar  = CHAR_NULL;
  } else {
    KeyData.Key.ScanCode     = SCAN_NULL;
  }

  //
  // CSM16 has converted the Ctrl+[a-z] to [1-26], converted it back.
  //
  if ((KeyData.KeyState.KeyShiftState & (EFI_LEFT_CONTROL_PRESSED | EFI_RIGHT_CONTROL_PRESSED)) != 0) {
    if (KeyData.Key.UnicodeChar >= 1 && KeyData.Key.UnicodeChar <= 26) {
      if (((KeyData.KeyState.KeyShiftState & (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED)) != 0) ==
          ((KeyData.KeyState.KeyToggleState & EFI_CAPS_LOCK_ACTIVE) != 0)
          ) {
        KeyData.Key.UnicodeChar = (UINT16) (KeyData.Key.UnicodeChar + L'a' - 1);
      } else {
        KeyData.Key.UnicodeChar = (UINT16) (KeyData.Key.UnicodeChar + L'A' - 1);
      }
    } else if (KeyData.Key.ScanCode == SCAN_ESC &&
               KeyData.Key.UnicodeChar == CHAR_NULL) {
      KeyData.Key.UnicodeChar = (KeyData.KeyState.KeyShiftState & (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED)) ? '{' : '[';
    } else if (KeyData.Key.UnicodeChar == 0x1C) {
      KeyData.Key.UnicodeChar = (KeyData.KeyState.KeyShiftState & (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED)) ? '|' : '\\';
    } else if (KeyData.Key.UnicodeChar == 0x1D) {
      KeyData.Key.UnicodeChar = (KeyData.KeyState.KeyShiftState & (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED)) ? '}' : ']';
    } else if (KeyData.Key.UnicodeChar == 0x1E) {
      KeyData.Key.UnicodeChar = (KeyData.KeyState.KeyShiftState & (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED)) ? '^' : '6';
    } else if (KeyData.Key.UnicodeChar == 0x1F) {
      KeyData.Key.UnicodeChar = (KeyData.KeyState.KeyShiftState & (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED)) ? '_' : '-';
    }
  }
  //
  // Convert Alt-alpha key back to UnicodeChar
  //
  if ((KeyData.KeyState.KeyShiftState & (EFI_LEFT_ALT_PRESSED | EFI_RIGHT_ALT_PRESSED)) != 0 &&
       KeyData.Key.UnicodeChar == CHAR_NULL &&
       KeyData.Key.ScanCode == SCAN_NULL &&
       Regs.H.AH != 0) {
    for (Index = 0; mAltConvertTable[Index].ScanCode != TABLE_END; Index ++) {
      if (mAltConvertTable[Index].ScanCode == Regs.H.AH) {
        if (((KeyData.KeyState.KeyShiftState & (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED)) != 0) ==
            ((KeyData.KeyState.KeyToggleState & EFI_CAPS_LOCK_ACTIVE) != 0)) {
          KeyData.Key.UnicodeChar = mAltConvertTable[Index].UnicodeChar;
        } else {
          KeyData.Key.UnicodeChar = mAltConvertTable[Index].ShiftUnicodeChar;
        }
        break;
      }
    }
  }

  DEBUG ((
    EFI_D_INFO,
    "[KBD]Convert to EFI Scan Code, EFI_INPUT_KEY.ScanCode - %x, EFI_INPUT_KEY.UnicodeChar - %x\n",
    KeyData.Key.ScanCode,
    KeyData.Key.UnicodeChar
    ));

  //
  // Need not return associated shift state if a class of printable characters that
  // are normally adjusted by shift modifiers.
  // e.g. Shift Key + 'f' key = 'F'; Shift Key + 'F' key = 'f'.
  //
  if ((KeyData.Key.UnicodeChar >= L'A' && KeyData.Key.UnicodeChar <= L'Z') ||
      (KeyData.Key.UnicodeChar >= L'a' && KeyData.Key.UnicodeChar <= L'z')
     ) {
    DEBUG ((EFI_D_INFO, "[KBD]Shift key with a~z are pressed, remove shift state in EFI_KEY_STATE.\n"));
    KeyData.KeyState.KeyShiftState &= ~(EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED);
  }

  //
  // Invoke notification functions if exist
  //
  for (Link = BiosKeyboardPrivate->NotifyList.ForwardLink; Link != &BiosKeyboardPrivate->NotifyList; Link = Link->ForwardLink) {
    CurrentNotify = CR (
                      Link,
                      BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY,
                      NotifyEntry,
                      BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY_SIGNATURE
                      );
    if (IsKeyRegistered (&CurrentNotify->KeyData, &KeyData)) {
      CurrentNotify->KeyNotificationFn (&KeyData);
    }
  }

  Enqueue (&BiosKeyboardPrivate->Queue, &KeyData);
  //
  // Leave critical section and return
  //
  gBS->RestoreTPL (OldTpl);

  return ;
}

/**
  Free keyboard notify list.

  @param  ListHead   The list head

  @retval EFI_SUCCESS           Free the notify list successfully
  @retval EFI_INVALID_PARAMETER ListHead is invalid.

**/
EFI_STATUS
BiosKeyboardFreeNotifyList (
  IN OUT LIST_ENTRY           *ListHead
  )
{
  BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY *NotifyNode;

  if (ListHead == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  while (!IsListEmpty (ListHead)) {
    NotifyNode = CR (
                   ListHead->ForwardLink,
                   BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY,
                   NotifyEntry,
                   BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY_SIGNATURE
                   );
    RemoveEntryList (ListHead->ForwardLink);
    gBS->FreePool (NotifyNode);
  }

  return EFI_SUCCESS;
}

/**
  Check if key is registered.

  @param  RegsiteredData    A pointer to a buffer that is filled in with the keystroke
                            state data for the key that was registered.
  @param  InputData         A pointer to a buffer that is filled in with the keystroke
                            state data for the key that was pressed.

  @retval TRUE              Key be pressed matches a registered key.
  @retval FLASE             Match failed.

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
  Waiting on the keyboard event, if there's any key pressed by the user, signal the event

  @param  Event    The event that be siganlled when any key has been stroked.
  @param  Context  Pointer of the protocol EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL.

**/
VOID
EFIAPI
BiosKeyboardWaitForKeyEx (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
{
  BIOS_KEYBOARD_DEV                     *BiosKeyboardPrivate;

  BiosKeyboardPrivate = TEXT_INPUT_EX_BIOS_KEYBOARD_DEV_FROM_THIS (Context);
  BiosKeyboardWaitForKey (Event, &BiosKeyboardPrivate->SimpleTextIn);

}

/**
  Reset the input device and optionaly run diagnostics

  @param  This                  Protocol instance pointer.
  @param  ExtendedVerification  Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS           The device was reset.
  @retval EFI_DEVICE_ERROR      The device is not functioning properly and could
                                not be reset.

**/
EFI_STATUS
EFIAPI
BiosKeyboardResetEx (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN BOOLEAN                            ExtendedVerification
  )
{
  BIOS_KEYBOARD_DEV                     *BiosKeyboardPrivate;
  EFI_STATUS                            Status;
  EFI_TPL                               OldTpl;

  BiosKeyboardPrivate = TEXT_INPUT_EX_BIOS_KEYBOARD_DEV_FROM_THIS (This);

  Status = BiosKeyboardPrivate->SimpleTextIn.Reset (
                                               &BiosKeyboardPrivate->SimpleTextIn,
                                               ExtendedVerification
                                               );
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  gBS->RestoreTPL (OldTpl);

  return EFI_SUCCESS;

}

/**
  Reads the next keystroke from the input device. The WaitForKey Event can
  be used to test for existance of a keystroke via WaitForEvent () call.

  @param  This         Protocol instance pointer.
  @param  KeyData      A pointer to a buffer that is filled in with the keystroke
                       state data for the key that was pressed.

  @retval  EFI_SUCCESS           The keystroke information was returned.
  @retval  EFI_NOT_READY         There was no keystroke data availiable.
  @retval  EFI_DEVICE_ERROR      The keystroke information was not returned due to
                                 hardware errors.
  @retval  EFI_INVALID_PARAMETER KeyData is NULL.

**/
EFI_STATUS
EFIAPI
BiosKeyboardReadKeyStrokeEx (
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  OUT EFI_KEY_DATA                      *KeyData
  )
{
  BIOS_KEYBOARD_DEV                     *BiosKeyboardPrivate;
  EFI_KEY_DATA                          BiosKey;
  EFI_STATUS                            Status;

  if (KeyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  BiosKeyboardPrivate = TEXT_INPUT_EX_BIOS_KEYBOARD_DEV_FROM_THIS (This);

  Status = KeyboardReadKeyStrokeWorker (BiosKeyboardPrivate, &BiosKey);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (BiosKey.Key.ScanCode == SCAN_NULL && BiosKey.Key.UnicodeChar == 0x00 ) {
    return EFI_NOT_READY;
  }

  CopyMem (KeyData, &BiosKey, sizeof (EFI_KEY_DATA));

  return EFI_SUCCESS;

}

/**
  Set certain state for the input device.

  @param  This              Protocol instance pointer.
  @param  KeyToggleState    A pointer to the EFI_KEY_TOGGLE_STATE to set the
                            state for the input device.

  @retval EFI_SUCCESS           The device state was set successfully.
  @retval EFI_DEVICE_ERROR      The device is not functioning correctly and could
                                not have the setting adjusted.
  @retval EFI_UNSUPPORTED       The device does not have the ability to set its state.
  @retval EFI_INVALID_PARAMETER KeyToggleState is NULL.

**/
EFI_STATUS
EFIAPI
BiosKeyboardSetState (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_KEY_TOGGLE_STATE               *KeyToggleState
  )
{
  EFI_STATUS                            Status;
  BIOS_KEYBOARD_DEV                     *BiosKeyboardPrivate;
  EFI_TPL                               OldTpl;
  EFI_LEGACY_BIOS_PROTOCOL              *LegacyBios;
  UINT8                                 Command;

  if (KeyToggleState == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*KeyToggleState & EFI_TOGGLE_STATE_VALID) != EFI_TOGGLE_STATE_VALID) {
    return EFI_UNSUPPORTED;
  }

  BiosKeyboardPrivate = TEXT_INPUT_EX_BIOS_KEYBOARD_DEV_FROM_THIS (This);
  //
  // See if the Legacy BIOS Protocol is available
  //
  Status = gBS->LocateProtocol (
                  &gEfiLegacyBiosProtocolGuid,
                  NULL,
                  (VOID **) &LegacyBios
                  );

  ASSERT_EFI_ERROR (Status);
  //
  // Enter critical section
  //
  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  Command = 0;
  if ((*KeyToggleState & EFI_CAPS_LOCK_ACTIVE) == EFI_CAPS_LOCK_ACTIVE) {
    Command |= 4;
  }
  if ((*KeyToggleState & EFI_NUM_LOCK_ACTIVE) == EFI_NUM_LOCK_ACTIVE) {
    Command |= 2;
  }
  if ((*KeyToggleState & EFI_SCROLL_LOCK_ACTIVE) == EFI_SCROLL_LOCK_ACTIVE) {
    Command |= 1;
  }

  Status = KeyboardWrite (BiosKeyboardPrivate, 0xed);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }
  Status = KeyboardWaitForValue (BiosKeyboardPrivate, 0xfa, KEYBOARD_WAITFORVALUE_TIMEOUT);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }
  Status = KeyboardWrite (BiosKeyboardPrivate, Command);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Call Legacy BIOS Protocol to set whatever is necessary
  //
  LegacyBios->UpdateKeyboardLedStatus (LegacyBios, Command);

  Status = EFI_SUCCESS;

  //
  // Leave critical section and return
  //
  gBS->RestoreTPL (OldTpl);

  return Status;

}

/**
  Register a notification function for a particular keystroke for the input device.

  @param  This                    Protocol instance pointer.
  @param  KeyData                 A pointer to a buffer that is filled in with the keystroke
                                  information data for the key that was pressed.
  @param  KeyNotificationFunction Points to the function to be called when the key
                                  sequence is typed specified by KeyData.
  @param  NotifyHandle            Points to the unique handle assigned to the registered notification.


  @retval EFI_SUCCESS             The notification function was registered successfully.
  @retval EFI_OUT_OF_RESOURCES    Unable to allocate resources for necesssary data structures.
  @retval EFI_INVALID_PARAMETER   KeyData or NotifyHandle is NULL.

**/
EFI_STATUS
EFIAPI
BiosKeyboardRegisterKeyNotify (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_KEY_DATA                       *KeyData,
  IN EFI_KEY_NOTIFY_FUNCTION            KeyNotificationFunction,
  OUT EFI_HANDLE                        *NotifyHandle
  )
{
  EFI_STATUS                            Status;
  BIOS_KEYBOARD_DEV                     *BiosKeyboardPrivate;
  EFI_TPL                               OldTpl;
  BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY    *NewNotify;
  LIST_ENTRY                            *Link;
  BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY    *CurrentNotify;

  if (KeyData == NULL || NotifyHandle == NULL || KeyNotificationFunction == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  BiosKeyboardPrivate = TEXT_INPUT_EX_BIOS_KEYBOARD_DEV_FROM_THIS (This);

  //
  // Enter critical section
  //
  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  //
  // Return EFI_SUCCESS if the (KeyData, NotificationFunction) is already registered.
  //
  for (Link = BiosKeyboardPrivate->NotifyList.ForwardLink; Link != &BiosKeyboardPrivate->NotifyList; Link = Link->ForwardLink) {
    CurrentNotify = CR (
                      Link,
                      BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY,
                      NotifyEntry,
                      BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY_SIGNATURE
                      );
    if (IsKeyRegistered (&CurrentNotify->KeyData, KeyData)) {
      if (CurrentNotify->KeyNotificationFn == KeyNotificationFunction) {
        *NotifyHandle = CurrentNotify->NotifyHandle;
        Status = EFI_SUCCESS;
        goto Exit;
      }
    }
  }

  //
  // Allocate resource to save the notification function
  //

  NewNotify = (BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY *) AllocateZeroPool (sizeof (BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY));
  if (NewNotify == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  NewNotify->Signature         = BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY_SIGNATURE;
  NewNotify->KeyNotificationFn = KeyNotificationFunction;
  NewNotify->NotifyHandle      = (EFI_HANDLE) NewNotify;
  CopyMem (&NewNotify->KeyData, KeyData, sizeof (EFI_KEY_DATA));
  InsertTailList (&BiosKeyboardPrivate->NotifyList, &NewNotify->NotifyEntry);

  *NotifyHandle                = NewNotify->NotifyHandle;
  Status                       = EFI_SUCCESS;

Exit:
  //
  // Leave critical section and return
  //
  gBS->RestoreTPL (OldTpl);
  return Status;
}

/**
  Remove a registered notification function from a particular keystroke.

  @param  This                 Protocol instance pointer.
  @param  NotificationHandle   The handle of the notification function being unregistered.

  @retval EFI_SUCCESS             The notification function was unregistered successfully.
  @retval EFI_INVALID_PARAMETER   The NotificationHandle is invalid.

**/
EFI_STATUS
EFIAPI
BiosKeyboardUnregisterKeyNotify (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_HANDLE                         NotificationHandle
  )
{
  EFI_STATUS                            Status;
  BIOS_KEYBOARD_DEV                     *BiosKeyboardPrivate;
  EFI_TPL                               OldTpl;
  LIST_ENTRY                            *Link;
  BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY    *CurrentNotify;

  //
  // Check incoming notification handle
  //
  if (NotificationHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (((BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY *) NotificationHandle)->Signature != BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  BiosKeyboardPrivate = TEXT_INPUT_EX_BIOS_KEYBOARD_DEV_FROM_THIS (This);

  //
  // Enter critical section
  //
  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  for (Link = BiosKeyboardPrivate->NotifyList.ForwardLink; Link != &BiosKeyboardPrivate->NotifyList; Link = Link->ForwardLink) {
    CurrentNotify = CR (
                      Link,
                      BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY,
                      NotifyEntry,
                      BIOS_KEYBOARD_CONSOLE_IN_EX_NOTIFY_SIGNATURE
                      );
    if (CurrentNotify->NotifyHandle == NotificationHandle) {
      //
      // Remove the notification function from NotifyList and free resources
      //
      RemoveEntryList (&CurrentNotify->NotifyEntry);

      Status = EFI_SUCCESS;
      goto Exit;
    }
  }

  //
  // Can not find the specified Notification Handle
  //
  Status = EFI_INVALID_PARAMETER;

Exit:
  //
  // Leave critical section and return
  //
  gBS->RestoreTPL (OldTpl);
  return Status;
}


EFI_STATUS
EFIAPI
CheckKeyboardStatus (
  IN  BIOS_KEYBOARD_DEV     *BiosKeyboardPrivate
  )
{
  EFI_STATUS                                Status;
  UINT8                                     Values[2];
  UINT8                                     Value;

  Status = EFI_DEVICE_ERROR;
  //
  // First do keyboard reset sequence, if reset cmd success during PEI phase.
  //
  if (KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_OUTB) {

     //
    // KBC should return 0xAA to indicate BAT test successful. Some KBC will return
    // KBC self test OK to indicate test successful and doesn't plug-in keyboard.
    //
    Value     = 0;
    Values[0] = KBC_CMDECHO_BATTEST_OK;
    Values[1] = KBC_CMDECHO_KBCSLFTEST_OK;
    Status = KeyboardWaitForValues (BiosKeyboardPrivate, Values, sizeof (Values), &Value, KEYBOARD_WAITFORVALUE_TIMEOUT);
    if (!EFI_ERROR (Status) && Value == KBC_CMDECHO_BATTEST_OK) {
      //
      // Set keyboard scan code set = 02 (standard configuration)
      //
      Status = KeyboardWrite (BiosKeyboardPrivate, KBC_INPBUF_VIA60_KBSCODE);
      //
      // Some KBC doesn't support this command and may return failed ACK data.
      //
      Values[0] = KBC_CMDECHO_ACK;
      Values[1] = KBC_CMDECHO_FAILED_ACK;
      Status = KeyboardWaitForValues (BiosKeyboardPrivate, Values, sizeof (Values), &Value, KEYBOARD_WAITFORVALUE_TIMEOUT);
      if (EFI_ERROR (Status)) {
        Status    = EFI_DEVICE_ERROR;
        goto Exit;
      }
      if (Value == KBC_CMDECHO_ACK) {
        Status = KeyboardWrite (BiosKeyboardPrivate, KBC_INPBUF_VIA60_SCODESET2);
        Status = KeyboardWaitForValue (BiosKeyboardPrivate, KBC_CMDECHO_ACK, KEYBOARD_WAITFORVALUE_TIMEOUT);
        if (EFI_ERROR (Status)) {
          Status    = EFI_DEVICE_ERROR;
          goto Exit;
        }
      }
      //
      // Enable keyboard itself (not via KBC) by writing CMD F4 via 60H
      //
      Status = KeyboardWrite (BiosKeyboardPrivate, KBC_INPBUF_VIA60_KBEN);
      Value = 0;
      Status = KeyboardWaitForValues (BiosKeyboardPrivate, Values, sizeof (Values), &Value, KEYBOARD_WAITFORVALUE_TIMEOUT);
      if (EFI_ERROR (Status)) {
        Status    = EFI_DEVICE_ERROR;
        goto Exit;
      }
      //
      // keyboard isn't connected, return EFI_SUCCESS directly
      //
      if (Value == KBC_CMDECHO_FAILED_ACK) {
        goto Exit;
      }
      //
      //
      // Perform KB checking by writing ABh via 64H
      //
      Status = KeyboardCommand (BiosKeyboardPrivate, KBC_CMDREG_VIA64_KB_CKECK);
      Status = KeyboardWaitForValue (BiosKeyboardPrivate, KBC_CMDECHO_KBCHECK_OK, KEYBOARD_WAITFORVALUE_TIMEOUT);
      if (EFI_ERROR (Status)) {
        Status    = EFI_DEVICE_ERROR;
        goto Exit;
      }

      return EFI_SUCCESS;
    }
  }
Exit:

  return Status;
}

/**
  The user Entry Point for module BiosKeyboard. The user code starts with this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializeBiosKeyboard(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gBiosKeyboardDriverBinding,
             ImageHandle,
             &gBiosKeyboardComponentName,
             &gBiosKeyboardComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

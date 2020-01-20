/** @file
  Function prototype for USB Keyboard Driver

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

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

BOOLEAN
IsUsbKeyboard (
  IN  EFI_USB_IO_PROTOCOL       *UsbIo
  );

EFI_STATUS
InitUsbKeyboard (
  IN USB_KB_DEV   *UsbKeyboardDevice
  );

/**
  Find Key Descriptor in Key Convertion Table given its USB keycode.

  @param[in]  UsbKeyboardDevice   The USB_KB_DEV instance.
  @param[in]  KeyCode             USB Keycode.

  @return The Key Descriptor in Key Convertion Table.
          NULL means not found.

**/
EFI_KEY_DESCRIPTOR *
GetKeyDescriptor (
  IN USB_KB_DEV        *UsbKeyboardDevice,
  IN UINT8             KeyCode
  );

/**
  Find Non-Spacing key for given Key descriptor.

  @param[in]  UsbKeyboardDevice    The USB_KB_DEV instance.
  @param[in]  KeyDescriptor        Key descriptor.

  @return The Non-Spacing key corresponding to KeyDescriptor
          NULL means not found.

**/
USB_NS_KEY *
FindUsbNsKey (
  IN USB_KB_DEV          *UsbKeyboardDevice,
  IN EFI_KEY_DESCRIPTOR  *KeyDescriptor
  );

/**
  Find physical key definition for a given key descriptor.

  For a specified non-spacing key, there are a list of physical
  keys following it. This function traverses the list of
  physical keys and tries to find the physical key matching
  the KeyDescriptor.

  @param[in]  UsbNsKey          The non-spacing key information.
  @param[in]  KeyDescriptor     The key descriptor.

  @return The physical key definition.
          If no physical key is found, parameter KeyDescriptor is returned.

**/
EFI_KEY_DESCRIPTOR *
FindPhysicalKey (
  IN USB_NS_KEY          *UsbNsKey,
  IN EFI_KEY_DESCRIPTOR  *KeyDescriptor
  );

/**
  Initialize Key Convention Table by using default keyboard layout.

  @param[in, out]  UsbKeyboardDevice    The USB_KB_DEV instance.

  @retval EFI_SUCCESS          The default keyboard layout was installed successfully
  @retval Others               Failure to install default keyboard layout.
**/
EFI_STATUS
InstallDefaultKeyboardLayout (
   IN OUT USB_KB_DEV           *UsbKeyboardDevice
  );

/**
  Get current keyboard layout from HII database.

  @return Pointer to HII Keyboard Layout.
          NULL means failure occurred while trying to get keyboard layout.

**/
EFI_HII_KEYBOARD_LAYOUT *
GetCurrentKeyboardLayout (
  VOID
  );

/**
  The notification function for EFI_HII_SET_KEYBOARD_LAYOUT_EVENT_GUID.

  This function is registered to event of EFI_HII_SET_KEYBOARD_LAYOUT_EVENT_GUID
  group type, which will be triggered by EFI_HII_DATABASE_PROTOCOL.SetKeyboardLayout().
  It tries to get curent keyboard layout from HII database.

  @param[in]  Event        Event being signaled.
  @param[in]  Context      Points to USB_KB_DEV instance.

**/
VOID
EFIAPI
SetKeyboardLayoutEvent (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  );

/**
  Destroy resources for keyboard layout.

  @param[in, out]  UsbKeyboardDevice    The USB_KB_DEV instance.

**/
VOID
ReleaseKeyboardLayoutResources (
  IN OUT USB_KB_DEV              *UsbKeyboardDevice
  );

/**
  Initialize USB keyboard layout.

  This function initializes Key Convertion Table for the USB keyboard device.
  It first tries to retrieve layout from HII database. If failed and default
  layout is enabled, then it just uses the default layout.

  @param[out]  UsbKeyboardDevice      The USB_KB_DEV instance.

  @retval EFI_SUCCESS            Initialization succeeded.
  @retval EFI_NOT_READY          Keyboard layout cannot be retrieve from HII
                                 database, and default layout is disabled.
  @retval Other                  Fail to register event to EFI_HII_SET_KEYBOARD_LAYOUT_EVENT_GUID group.

**/
EFI_STATUS
InitKeyboardLayout (
  OUT USB_KB_DEV   *UsbKeyboardDevice
  );

EFI_STATUS
EFIAPI
KeyboardHandler (
  IN  VOID          *Data,
  IN  UINTN         DataLength,
  IN  VOID          *Context,
  IN  UINT32        Result
  );

VOID
EFIAPI
UsbKeyboardRecoveryHandler (
  IN    UINTN        Event,
  IN    VOID         *Context
  );

EFI_STATUS
UsbParseKey (
  IN OUT  USB_KB_DEV  *UsbKeyboardDevice,
  OUT     UINT8       *KeyChar
  );

EFI_STATUS
UsbKeyCodeToEFIScanCode (
  IN  USB_KB_DEV      *UsbKeyboardDevice,
  IN  UINT8           KeyChar,
  OUT EFI_INPUT_KEY   *Key
  );

EFI_STATUS
InitUsbKeyBuffer (
  IN OUT  USB_KB_BUFFER   *KeyboardBuffer
  );

BOOLEAN
IsUsbKeyboardBufferEmpty (
  IN  USB_KB_BUFFER   *KeyboardBuffer
  );

BOOLEAN
IsUsbKeyboardBufferFull (
  IN  USB_KB_BUFFER   *KeyboardBuffer
  );

EFI_STATUS
InsertKeyCode (
  IN      USB_KB_DEV    *UsbKeyboardDevice,
  IN      UINT8         Key,
  IN      UINT8         Down
  );

EFI_STATUS
RemoveKeyCode (
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  OUT     USB_KEY       *UsbKey
  );

VOID
EFIAPI
UsbKeyboardRepeatHandler (
  IN    UINTN        Event,
  IN    VOID         *Context
  );

EFI_STATUS
SetKeyLED (
  IN  USB_KB_DEV    *UsbKeyboardDevice
  );

VOID
EFIAPI
UsbKeyboardCheckLEDHandler (
  IN    UINTN        Event,
  IN    VOID         *Context
  );
#endif

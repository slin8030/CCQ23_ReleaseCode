/** @file
  Helper functions for USB Keyboard Driver

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

#include "Efikey.h"
#include "Keyboard.h"

USB_KEYBOARD_LAYOUT_PACK_BIN  mUsbKeyboardLayoutBin = {
  sizeof (USB_KEYBOARD_LAYOUT_PACK_BIN),   // Binary size

  //
  // EFI_HII_PACKAGE_HEADER
  //
  {
    sizeof (USB_KEYBOARD_LAYOUT_PACK_BIN) - sizeof (UINT32),
    EFI_HII_PACKAGE_KEYBOARD_LAYOUT
  },
  2,  // LayoutCount
  //
  // US Layout
  //
  sizeof (USB_KEYBOARD_US_LAYOUT_PACK), // LayoutLength
  H2O_USB_KEYBOARD_US_LAYOUT_KEY_GUID,  // KeyGuid
  sizeof (UINT16) + sizeof (EFI_GUID) + sizeof (UINT32) + sizeof (UINT8) + (USB_KEYBOARD_US_KEY_COUNT * sizeof (EFI_KEY_DESCRIPTOR)), // LayoutDescriptorStringOffset
  USB_KEYBOARD_US_KEY_COUNT, // DescriptorCount
  {
    //
    // EFI_KEY_DESCRIPTOR (total number is USB_KEYBOARD_US_KEY_COUNT)
    //
    {EfiKeyC1,         'a',      'A',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB5,         'b',      'B',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB3,         'c',      'C',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC3,         'd',      'D',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD3,         'e',      'E',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC4,         'f',      'F',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC5,         'g',      'G',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC6,         'h',      'H',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD8,         'i',      'I',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC7,         'j',      'J',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC8,         'k',      'K',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC9,         'l',      'L',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB7,         'm',      'M',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB6,         'n',      'N',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD9,         'o',      'O',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD10,        'p',      'P',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD1,         'q',      'Q',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD4,         'r',      'R',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC2,         's',      'S',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD5,         't',      'T',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD7,         'u',      'U',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB4,         'v',      'V',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD2,         'w',      'W',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB2,         'x',      'X',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD6,         'y',      'Y',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB1,         'z',      'Z',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyE1,         '1',      '!',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE2,         '2',      '@',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE3,         '3',      '#',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE4,         '4',      '$',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE5,         '5',      '%',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE6,         '6',      '^',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE7,         '7',      '&',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE8,         '8',      '*',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE9,         '9',      '(',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE10,        '0',      ')',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyEnter,      0x0d,     0x0d, 0, 0,  EFI_NULL_MODIFIER,   0},
    {EfiKeyEsc,        0x1b,     0x1b, 0, 0,  EFI_NULL_MODIFIER,   0},
    {EfiKeyBackSpace,  0x08,     0x08, 0, 0,  EFI_NULL_MODIFIER,   0},
    {EfiKeyTab,        0x09,     0x09, 0, 0,  EFI_NULL_MODIFIER,   0},
    {EfiKeySpaceBar,   ' ',      ' ',  0, 0,  EFI_NULL_MODIFIER,   0},
    {EfiKeyE11,        '-',      '_',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE12,        '=',      '+',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyD11,        '[',      '{',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyD12,        ']',      '}',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyD13,        '\\',     '|',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyC12,        '\\',     '|',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyC10,        ';',      ':',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyC11,        '\'',     '"',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE0,         '`',      '~',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyB8,         ',',      '<',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyB9,         '.',      '>',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyB10,        '/',      '?',  0, 0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyCapsLock,   0x00,     0x00, 0, 0,  EFI_CAPS_LOCK_MODIFIER,            0},
    {EfiKeyF1,         0x00,     0x00, 0, 0,  EFI_FUNCTION_KEY_ONE_MODIFIER,     0},
    {EfiKeyF2,         0x00,     0x00, 0, 0,  EFI_FUNCTION_KEY_TWO_MODIFIER,     0},
    {EfiKeyF3,         0x00,     0x00, 0, 0,  EFI_FUNCTION_KEY_THREE_MODIFIER,   0},
    {EfiKeyF4,         0x00,     0x00, 0, 0,  EFI_FUNCTION_KEY_FOUR_MODIFIER,    0},
    {EfiKeyF5,         0x00,     0x00, 0, 0,  EFI_FUNCTION_KEY_FIVE_MODIFIER,    0},
    {EfiKeyF6,         0x00,     0x00, 0, 0,  EFI_FUNCTION_KEY_SIX_MODIFIER,     0},
    {EfiKeyF7,         0x00,     0x00, 0, 0,  EFI_FUNCTION_KEY_SEVEN_MODIFIER,   0},
    {EfiKeyF8,         0x00,     0x00, 0, 0,  EFI_FUNCTION_KEY_EIGHT_MODIFIER,   0},
    {EfiKeyF9,         0x00,     0x00, 0, 0,  EFI_FUNCTION_KEY_NINE_MODIFIER,    0},
    {EfiKeyF10,        0x00,     0x00, 0, 0,  EFI_FUNCTION_KEY_TEN_MODIFIER,     0},
    {EfiKeyF11,        0x00,     0x00, 0, 0,  EFI_FUNCTION_KEY_ELEVEN_MODIFIER,  0},
    {EfiKeyF12,        0x00,     0x00, 0, 0,  EFI_FUNCTION_KEY_TWELVE_MODIFIER,  0},
    {EfiKeyPrint,      0x00,     0x00, 0, 0,  EFI_PRINT_MODIFIER,                0},
    {EfiKeySLck,       0x00,     0x00, 0, 0,  EFI_SCROLL_LOCK_MODIFIER,          0},
    {EfiKeyPause,      0x00,     0x00, 0, 0,  EFI_PAUSE_MODIFIER,                0},
    {EfiKeyIns,        0x00,     0x00, 0, 0,  EFI_INSERT_MODIFIER,               0},
    {EfiKeyHome,       0x00,     0x00, 0, 0,  EFI_HOME_MODIFIER,                 0},
    {EfiKeyPgUp,       0x00,     0x00, 0, 0,  EFI_PAGE_UP_MODIFIER,              0},
    {EfiKeyDel,        0x00,     0x00, 0, 0,  EFI_DELETE_MODIFIER,               0},
    {EfiKeyEnd,        0x00,     0x00, 0, 0,  EFI_END_MODIFIER,                  0},
    {EfiKeyPgDn,       0x00,     0x00, 0, 0,  EFI_PAGE_DOWN_MODIFIER,            0},
    {EfiKeyRightArrow, 0x00,     0x00, 0, 0,  EFI_RIGHT_ARROW_MODIFIER,          0},
    {EfiKeyLeftArrow,  0x00,     0x00, 0, 0,  EFI_LEFT_ARROW_MODIFIER,           0},
    {EfiKeyDownArrow,  0x00,     0x00, 0, 0,  EFI_DOWN_ARROW_MODIFIER,           0},
    {EfiKeyUpArrow,    0x00,     0x00, 0, 0,  EFI_UP_ARROW_MODIFIER,             0},
    {EfiKeyNLck,       0x00,     0x00, 0, 0,  EFI_NUM_LOCK_MODIFIER,             0},
    {EfiKeySlash,      '/',      '/',  0, 0,  EFI_NULL_MODIFIER,                 0},
    {EfiKeyAsterisk,   '*',      '*',  0, 0,  EFI_NULL_MODIFIER,                 0},
    {EfiKeyMinus,      '-',      '-',  0, 0,  EFI_NULL_MODIFIER,                 0},
    {EfiKeyPlus,       '+',      '+',  0, 0,  EFI_NULL_MODIFIER,                 0},
    {EfiKeyEnter,      0x0d,     0x0d, 0, 0,  EFI_NULL_MODIFIER,                 0},
    {EfiKeyOne,        '1',      '1',  0, 0,  EFI_END_MODIFIER,         EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyTwo,        '2',      '2',  0, 0,  EFI_DOWN_ARROW_MODIFIER,  EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyThree,      '3',      '3',  0, 0,  EFI_PAGE_DOWN_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyFour,       '4',      '4',  0, 0,  EFI_LEFT_ARROW_MODIFIER,  EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyFive,       '5',      '5',  0, 0,  EFI_NULL_MODIFIER,        EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeySix,        '6',      '6',  0, 0,  EFI_RIGHT_ARROW_MODIFIER, EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeySeven,      '7',      '7',  0, 0,  EFI_HOME_MODIFIER,        EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyEight,      '8',      '8',  0, 0,  EFI_UP_ARROW_MODIFIER,    EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyNine,       '9',      '9',  0, 0,  EFI_PAGE_UP_MODIFIER,     EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyZero,       '0',      '0',  0, 0,  EFI_INSERT_MODIFIER,      EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyPeriod,     '.',      '.',  0, 0,  EFI_DELETE_MODIFIER,      EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyA4,         0x00,     0x00, 0, 0,  EFI_MENU_MODIFIER,            0},
    {EfiKeyLCtrl,      0,        0,    0, 0,  EFI_LEFT_CONTROL_MODIFIER,    0},
    {EfiKeyLShift,     0,        0,    0, 0,  EFI_LEFT_SHIFT_MODIFIER,      0},
    {EfiKeyLAlt,       0,        0,    0, 0,  EFI_LEFT_ALT_MODIFIER,        0},
    {EfiKeyA0,         0,        0,    0, 0,  EFI_LEFT_LOGO_MODIFIER,       0},
    {EfiKeyRCtrl,      0,        0,    0, 0,  EFI_RIGHT_CONTROL_MODIFIER,   0},
    {EfiKeyRShift,     0,        0,    0, 0,  EFI_RIGHT_SHIFT_MODIFIER,     0},
    {EfiKeyA2,         0,        0,    0, 0,  EFI_RIGHT_ALT_MODIFIER,       0},
    {EfiKeyA3,         0,        0,    0, 0,  EFI_RIGHT_LOGO_MODIFIER,      0},
  },
  1,                          // DescriptionCount
  {'e', 'n', '-', 'U', 'S'},  // RFC4646 language code
  ' ',                        // Space
  {'E', 'n', 'g', 'l', 'i', 's', 'h', ' ', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd', '\0'}, // DescriptionString[]
  //
  // French Layout
  //
  sizeof (USB_KEYBOARD_FRENCH_LAYOUT_PACK), // LayoutLength
  H2O_USB_KEYBOARD_FRENCH_LAYOUT_KEY_GUID,  // KeyGuid
  sizeof (UINT16) + sizeof (EFI_GUID) + sizeof (UINT32) + sizeof (UINT8) + (USB_KEYBOARD_FRENCH_KEY_COUNT * sizeof (EFI_KEY_DESCRIPTOR)), // LayoutDescriptorStringOffset
  USB_KEYBOARD_FRENCH_KEY_COUNT, // DescriptorCount
  {
    //
    // EFI_KEY_DESCRIPTOR (total number is USB_KEYBOARD_FRENCH_KEY_COUNT)
    //
    {EfiKeyD1,         'a',      'A',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB5,         'b',      'B',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB3,         'c',      'C',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC3,         'd',      'D',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD3,         'e',      'E',  0x20AC,   0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC4,         'f',      'F',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC5,         'g',      'G',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC6,         'h',      'H',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD8,         'i',      'I',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC7,         'j',      'J',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC8,         'k',      'K',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC9,         'l',      'L',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC10,        'm',      'M',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB6,         'n',      'N',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD9,         'o',      'O',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD10,        'p',      'P',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC1,         'q',      'Q',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD4,         'r',      'R',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyC2,         's',      'S',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD5,         't',      'T',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD7,         'u',      'U',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB4,         'v',      'V',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB1,         'w',      'W',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyB2,         'x',      'X',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD6,         'y',      'Y',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyD2,         'z',      'Z',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK},
    {EfiKeyE1,         '&',      '1',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE2,         0xE9,     '2',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE3,         '"',      '3',  '#',      0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE4,         '\'',     '4',  '{',      0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE5,         '(',      '5',  '[',      0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE6,         '-',      '6',  '|',      0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE7,         0xE8,     '7',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE8,         '_',      '8',  '\\',     0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE9,         0xE7,     '9',  '^',      0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE10,        0xE0,     '0',  '@',      0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyEnter,      0x0d,     0x0d, 0,        0,  EFI_NULL_MODIFIER,   0},
    {EfiKeyEsc,        0x1b,     0x1b, 0,        0,  EFI_NULL_MODIFIER,   0},
    {EfiKeyBackSpace,  0x08,     0x08, 0,        0,  EFI_NULL_MODIFIER,   0},
    {EfiKeyTab,        0x09,     0x09, 0,        0,  EFI_NULL_MODIFIER,   0},
    {EfiKeySpaceBar,   ' ',      ' ',  0,        0,  EFI_NULL_MODIFIER,   0},
    {EfiKeyE11,        ')',      0xB0, ']',      0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE12,        '=',      '+',  '}',      0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyD11,        0x00,     0x00, 0,        0,  EFI_NULL_MODIFIER,          0},
    {EfiKeyD12,        '$',      0xA3, 0xA4,     0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyC12,        '*' ,     0xB5, 0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyC11,        0xF9,     '%',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyE0,         0xB2,     0x00, 0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyB0,         '<',      '>',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyB7,         ',',      '?',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyB8,         ';',      '.',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyB9,         ':',      '/',  0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyB10,        '!',      0xA7, 0,        0,  EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT},
    {EfiKeyCapsLock,   0x00,     0x00, 0,        0,  EFI_CAPS_LOCK_MODIFIER,            0},
    {EfiKeyF1,         0x00,     0x00, 0,        0,  EFI_FUNCTION_KEY_ONE_MODIFIER,     0},
    {EfiKeyF2,         0x00,     0x00, 0,        0,  EFI_FUNCTION_KEY_TWO_MODIFIER,     0},
    {EfiKeyF3,         0x00,     0x00, 0,        0,  EFI_FUNCTION_KEY_THREE_MODIFIER,   0},
    {EfiKeyF4,         0x00,     0x00, 0,        0,  EFI_FUNCTION_KEY_FOUR_MODIFIER,    0},
    {EfiKeyF5,         0x00,     0x00, 0,        0,  EFI_FUNCTION_KEY_FIVE_MODIFIER,    0},
    {EfiKeyF6,         0x00,     0x00, 0,        0,  EFI_FUNCTION_KEY_SIX_MODIFIER,     0},
    {EfiKeyF7,         0x00,     0x00, 0,        0,  EFI_FUNCTION_KEY_SEVEN_MODIFIER,   0},
    {EfiKeyF8,         0x00,     0x00, 0,        0,  EFI_FUNCTION_KEY_EIGHT_MODIFIER,   0},
    {EfiKeyF9,         0x00,     0x00, 0,        0,  EFI_FUNCTION_KEY_NINE_MODIFIER,    0},
    {EfiKeyF10,        0x00,     0x00, 0,        0,  EFI_FUNCTION_KEY_TEN_MODIFIER,     0},
    {EfiKeyF11,        0x00,     0x00, 0,        0,  EFI_FUNCTION_KEY_ELEVEN_MODIFIER,  0},
    {EfiKeyF12,        0x00,     0x00, 0,        0,  EFI_FUNCTION_KEY_TWELVE_MODIFIER,  0},
    {EfiKeyPrint,      0x00,     0x00, 0,        0,  EFI_PRINT_MODIFIER,                0},
    {EfiKeySLck,       0x00,     0x00, 0,        0,  EFI_SCROLL_LOCK_MODIFIER,          0},
    {EfiKeyPause,      0x00,     0x00, 0,        0,  EFI_PAUSE_MODIFIER,                0},
    {EfiKeyIns,        0x00,     0x00, 0,        0,  EFI_INSERT_MODIFIER,               0},
    {EfiKeyHome,       0x00,     0x00, 0,        0,  EFI_HOME_MODIFIER,                 0},
    {EfiKeyPgUp,       0x00,     0x00, 0,        0,  EFI_PAGE_UP_MODIFIER,              0},
    {EfiKeyDel,        0x00,     0x00, 0,        0,  EFI_DELETE_MODIFIER,               0},
    {EfiKeyEnd,        0x00,     0x00, 0,        0,  EFI_END_MODIFIER,                  0},
    {EfiKeyPgDn,       0x00,     0x00, 0,        0,  EFI_PAGE_DOWN_MODIFIER,            0},
    {EfiKeyRightArrow, 0x00,     0x00, 0,        0,  EFI_RIGHT_ARROW_MODIFIER,          0},
    {EfiKeyLeftArrow,  0x00,     0x00, 0,        0,  EFI_LEFT_ARROW_MODIFIER,           0},
    {EfiKeyDownArrow,  0x00,     0x00, 0,        0,  EFI_DOWN_ARROW_MODIFIER,           0},
    {EfiKeyUpArrow,    0x00,     0x00, 0,        0,  EFI_UP_ARROW_MODIFIER,             0},
    {EfiKeyNLck,       0x00,     0x00, 0,        0,  EFI_NUM_LOCK_MODIFIER,             0},
    {EfiKeySlash,      '/',      '/',  0,        0,  EFI_NULL_MODIFIER,                 0},
    {EfiKeyAsterisk,   '*',      '*',  0,        0,  EFI_NULL_MODIFIER,                 0},
    {EfiKeyMinus,      '-',      '-',  0,        0,  EFI_NULL_MODIFIER,                 0},
    {EfiKeyPlus,       '+',      '+',  0,        0,  EFI_NULL_MODIFIER,                 0},
    {EfiKeyEnter,      0x0d,     0x0d, 0,        0,  EFI_NULL_MODIFIER,                 0},
    {EfiKeyOne,        '1',      '1',  0,        0,  EFI_END_MODIFIER,         EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyTwo,        '2',      '2',  0,        0,  EFI_DOWN_ARROW_MODIFIER,  EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyThree,      '3',      '3',  0,        0,  EFI_PAGE_DOWN_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyFour,       '4',      '4',  0,        0,  EFI_LEFT_ARROW_MODIFIER,  EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyFive,       '5',      '5',  0,        0,  EFI_NULL_MODIFIER,        EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeySix,        '6',      '6',  0,        0,  EFI_RIGHT_ARROW_MODIFIER, EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeySeven,      '7',      '7',  0,        0,  EFI_HOME_MODIFIER,        EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyEight,      '8',      '8',  0,        0,  EFI_UP_ARROW_MODIFIER,    EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyNine,       '9',      '9',  0,        0,  EFI_PAGE_UP_MODIFIER,     EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyZero,       '0',      '0',  0,        0,  EFI_INSERT_MODIFIER,      EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyPeriod,     '.',      '.',  0,        0,  EFI_DELETE_MODIFIER,      EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK},
    {EfiKeyA4,         0x00,     0x00, 0,        0,  EFI_MENU_MODIFIER,            0},
    {EfiKeyLCtrl,      0,        0,    0,        0,  EFI_LEFT_CONTROL_MODIFIER,    0},
    {EfiKeyLShift,     0,        0,    0,        0,  EFI_LEFT_SHIFT_MODIFIER,      0},
    {EfiKeyLAlt,       0,        0,    0,        0,  EFI_LEFT_ALT_MODIFIER,        0},
    {EfiKeyA0,         0,        0,    0,        0,  EFI_LEFT_LOGO_MODIFIER,       0},
    {EfiKeyRCtrl,      0,        0,    0,        0,  EFI_RIGHT_CONTROL_MODIFIER,   0},
    {EfiKeyRShift,     0,        0,    0,        0,  EFI_RIGHT_SHIFT_MODIFIER,     0},
    {EfiKeyA2,         0,        0,    0,        0,  EFI_ALT_GR_MODIFIER,          0},
    {EfiKeyA3,         0,        0,    0,        0,  EFI_RIGHT_LOGO_MODIFIER,      0},
  },
  1,                          // DescriptionCount
  {'f', 'r', '-', 'F', 'R'},  // RFC4646 language code
  ' ',                        // Space
  {'F', 'r', 'e', 'n', 'c', 'h', ' ', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd', '\0'}, // DescriptionString[]
};

//
// EFI_KEY to USB Keycode conversion table
// EFI_KEY is defined in UEFI spec.
// USB Keycode is defined in USB HID Firmware spec.
//
UINT8 EfiKeyToUsbKeyCodeConvertionTable[] = {
  0xe0,  //  EfiKeyLCtrl
  0xe3,  //  EfiKeyA0
  0xe2,  //  EfiKeyLAlt
  0x2c,  //  EfiKeySpaceBar
  0xe6,  //  EfiKeyA2
  0xe7,  //  EfiKeyA3
  0x65,  //  EfiKeyA4
  0xe4,  //  EfiKeyRCtrl
  0x50,  //  EfiKeyLeftArrow
  0x51,  //  EfiKeyDownArrow
  0x4F,  //  EfiKeyRightArrow
  0x62,  //  EfiKeyZero
  0x63,  //  EfiKeyPeriod
  0x28,  //  EfiKeyEnter
  0xe1,  //  EfiKeyLShift
  0x64,  //  EfiKeyB0
  0x1D,  //  EfiKeyB1
  0x1B,  //  EfiKeyB2
  0x06,  //  EfiKeyB3
  0x19,  //  EfiKeyB4
  0x05,  //  EfiKeyB5
  0x11,  //  EfiKeyB6
  0x10,  //  EfiKeyB7
  0x36,  //  EfiKeyB8
  0x37,  //  EfiKeyB9
  0x38,  //  EfiKeyB10
  0xe5,  //  EfiKeyRShift
  0x52,  //  EfiKeyUpArrow
  0x59,  //  EfiKeyOne
  0x5A,  //  EfiKeyTwo
  0x5B,  //  EfiKeyThree
  0x39,  //  EfiKeyCapsLock
  0x04,  //  EfiKeyC1
  0x16,  //  EfiKeyC2
  0x07,  //  EfiKeyC3
  0x09,  //  EfiKeyC4
  0x0A,  //  EfiKeyC5
  0x0B,  //  EfiKeyC6
  0x0D,  //  EfiKeyC7
  0x0E,  //  EfiKeyC8
  0x0F,  //  EfiKeyC9
  0x33,  //  EfiKeyC10
  0x34,  //  EfiKeyC11
  0x32,  //  EfiKeyC12
  0x5C,  //  EfiKeyFour
  0x5D,  //  EfiKeyFive
  0x5E,  //  EfiKeySix
  0x57,  //  EfiKeyPlus
  0x2B,  //  EfiKeyTab
  0x14,  //  EfiKeyD1
  0x1A,  //  EfiKeyD2
  0x08,  //  EfiKeyD3
  0x15,  //  EfiKeyD4
  0x17,  //  EfiKeyD5
  0x1C,  //  EfiKeyD6
  0x18,  //  EfiKeyD7
  0x0C,  //  EfiKeyD8
  0x12,  //  EfiKeyD9
  0x13,  //  EfiKeyD10
  0x2F,  //  EfiKeyD11
  0x30,  //  EfiKeyD12
  0x31,  //  EfiKeyD13
  0x4C,  //  EfiKeyDel
  0x4D,  //  EfiKeyEnd
  0x4E,  //  EfiKeyPgDn
  0x5F,  //  EfiKeySeven
  0x60,  //  EfiKeyEight
  0x61,  //  EfiKeyNine
  0x35,  //  EfiKeyE0
  0x1E,  //  EfiKeyE1
  0x1F,  //  EfiKeyE2
  0x20,  //  EfiKeyE3
  0x21,  //  EfiKeyE4
  0x22,  //  EfiKeyE5
  0x23,  //  EfiKeyE6
  0x24,  //  EfiKeyE7
  0x25,  //  EfiKeyE8
  0x26,  //  EfiKeyE9
  0x27,  //  EfiKeyE10
  0x2D,  //  EfiKeyE11
  0x2E,  //  EfiKeyE12
  0x2A,  //  EfiKeyBackSpace
  0x49,  //  EfiKeyIns
  0x4A,  //  EfiKeyHome
  0x4B,  //  EfiKeyPgUp
  0x53,  //  EfiKeyNLck
  0x54,  //  EfiKeySlash
  0x55,  //  EfiKeyAsterisk
  0x56,  //  EfiKeyMinus
  0x29,  //  EfiKeyEsc
  0x3A,  //  EfiKeyF1
  0x3B,  //  EfiKeyF2
  0x3C,  //  EfiKeyF3
  0x3D,  //  EfiKeyF4
  0x3E,  //  EfiKeyF5
  0x3F,  //  EfiKeyF6
  0x40,  //  EfiKeyF7
  0x41,  //  EfiKeyF8
  0x42,  //  EfiKeyF9
  0x43,  //  EfiKeyF10
  0x44,  //  EfiKeyF11
  0x45,  //  EfiKeyF12
  0x46,  //  EfiKeyPrint
  0x47,  //  EfiKeySLck
  0x48   //  EfiKeyPause
};

//
// Keyboard modifier value to EFI Scan Code convertion table
// EFI Scan Code and the modifier values are defined in UEFI spec.
//
UINT8 ModifierValueToEfiScanCodeConvertionTable[] = {
  SCAN_NULL,       // EFI_NULL_MODIFIER
  SCAN_NULL,       // EFI_LEFT_CONTROL_MODIFIER
  SCAN_NULL,       // EFI_RIGHT_CONTROL_MODIFIER
  SCAN_NULL,       // EFI_LEFT_ALT_MODIFIER
  SCAN_NULL,       // EFI_RIGHT_ALT_MODIFIER
  SCAN_NULL,       // EFI_ALT_GR_MODIFIER
  SCAN_INSERT,     // EFI_INSERT_MODIFIER
  SCAN_DELETE,     // EFI_DELETE_MODIFIER
  SCAN_PAGE_DOWN,  // EFI_PAGE_DOWN_MODIFIER
  SCAN_PAGE_UP,    // EFI_PAGE_UP_MODIFIER
  SCAN_HOME,       // EFI_HOME_MODIFIER
  SCAN_END,        // EFI_END_MODIFIER
  SCAN_NULL,       // EFI_LEFT_SHIFT_MODIFIER
  SCAN_NULL,       // EFI_RIGHT_SHIFT_MODIFIER
  SCAN_NULL,       // EFI_CAPS_LOCK_MODIFIER
  SCAN_NULL,       // EFI_NUM_LOCK_MODIFIER
  SCAN_LEFT,       // EFI_LEFT_ARROW_MODIFIER
  SCAN_RIGHT,      // EFI_RIGHT_ARROW_MODIFIER
  SCAN_DOWN,       // EFI_DOWN_ARROW_MODIFIER
  SCAN_UP,         // EFI_UP_ARROW_MODIFIER
  SCAN_NULL,       // EFI_NS_KEY_MODIFIER
  SCAN_NULL,       // EFI_NS_KEY_DEPENDENCY_MODIFIER
  SCAN_F1,         // EFI_FUNCTION_KEY_ONE_MODIFIER
  SCAN_F2,         // EFI_FUNCTION_KEY_TWO_MODIFIER
  SCAN_F3,         // EFI_FUNCTION_KEY_THREE_MODIFIER
  SCAN_F4,         // EFI_FUNCTION_KEY_FOUR_MODIFIER
  SCAN_F5,         // EFI_FUNCTION_KEY_FIVE_MODIFIER
  SCAN_F6,         // EFI_FUNCTION_KEY_SIX_MODIFIER
  SCAN_F7,         // EFI_FUNCTION_KEY_SEVEN_MODIFIER
  SCAN_F8,         // EFI_FUNCTION_KEY_EIGHT_MODIFIER
  SCAN_F9,         // EFI_FUNCTION_KEY_NINE_MODIFIER
  SCAN_F10,        // EFI_FUNCTION_KEY_TEN_MODIFIER
  SCAN_F11,        // EFI_FUNCTION_KEY_ELEVEN_MODIFIER
  SCAN_F12,        // EFI_FUNCTION_KEY_TWELVE_MODIFIER
  //
  // For Partial Keystroke support
  //
  SCAN_NULL,       // EFI_PRINT_MODIFIER
  SCAN_NULL,       // EFI_SYS_REQUEST_MODIFIER
  SCAN_NULL,       // EFI_SCROLL_LOCK_MODIFIER
  SCAN_PAUSE,      // EFI_PAUSE_MODIFIER
  SCAN_NULL,       // EFI_BREAK_MODIFIER
  SCAN_NULL,       // EFI_LEFT_LOGO_MODIFIER
  SCAN_NULL,       // EFI_RIGHT_LOGO_MODIFER
  SCAN_NULL,       // EFI_MENU_MODIFER
};

STATIC KB_MODIFIER  KB_Mod[8] = {
  { MOD_CONTROL_L,  0xe0 }, // 11100000 
  { MOD_CONTROL_R,  0xe4 }, // 11100100 
  { MOD_SHIFT_L,    0xe1 }, // 11100001 
  { MOD_SHIFT_R,    0xe5 }, // 11100101 
  { MOD_ALT_L,      0xe2 }, // 11100010 
  { MOD_ALT_R,      0xe6 }, // 11100110 
  { MOD_WIN_L,      0xe3 }, // 11100011 
  { MOD_WIN_R,      0xe7 }, // 11100111 
};

STATIC
UINT8 KbcCodeTable[][4] = {
//  Original Shift    Alt      Ctrl
    0x1E,    0x1E,    0x1E,    0x1E,           // 0x04  'A'
    0x30,    0x30,    0x30,    0x30,           // 0x05  'B'
    0x2E,    0x2E,    0x2E,    0x2E,           // 0x06  'C'
    0x20,    0x20,    0x20,    0x20,           // 0x07  'D'
    0x12,    0x12,    0x12,    0x12,           // 0x08  'E'
    0x21,    0x21,    0x21,    0x21,           // 0x09  'F'
    0x22,    0x22,    0x22,    0x22,           // 0x0A  'G'
    0x23,    0x23,    0x23,    0x23,           // 0x0B  'H'
    0x17,    0x17,    0x17,    0x17,           // 0x0C  'I'
    0x24,    0x24,    0x24,    0x24,           // 0x0D  'J'
    0x25,    0x25,    0x25,    0x25,           // 0x0E  'K'
    0x26,    0x26,    0x26,    0x26,           // 0x0F  'L'
    0x32,    0x32,    0x32,    0x32,           // 0x10  'M'
    0x31,    0x31,    0x31,    0x31,           // 0x11  'N'
    0x18,    0x18,    0x18,    0x18,           // 0x12  'O'
    0x19,    0x19,    0x19,    0x19,           // 0x13  'P'
    0x10,    0x10,    0x10,    0x10,           // 0x14  'Q'
    0x13,    0x13,    0x13,    0x13,           // 0x15  'R'
    0x1F,    0x1F,    0x1F,    0x1F,           // 0x16  'S'
    0x14,    0x14,    0x14,    0x14,           // 0x17  'T'
    0x16,    0x16,    0x16,    0x16,           // 0x18  'U'
    0x2F,    0x2F,    0x2F,    0x2F,           // 0x19  'V'
    0x11,    0x11,    0x11,    0x11,           // 0x1A  'W'
    0x2D,    0x2D,    0x2D,    0x2D,           // 0x1B  'X'
    0x15,    0x15,    0x15,    0x15,           // 0x1C  'Y'
    0x2C,    0x2C,    0x2C,    0x2C,           // 0x1D  'Z'
    0x02,    0x02,    0x78,    0x00,           // 0x1E  '1 !'
    0x03,    0x03,    0x79,    0x03,           // 0x1F  '2 @'
    0x04,    0x04,    0x7A,    0x00,           // 0x20  '3 #'
    0x05,    0x05,    0x7B,    0x00,           // 0x21  '4 $'
    0x06,    0x06,    0x7C,    0x00,           // 0x22  '5 %'
    0x07,    0x07,    0x7D,    0x07,           // 0x23  '6 ^'
    0x08,    0x08,    0x7E,    0x00,           // 0x24  '7 &'
    0x09,    0x09,    0x7F,    0x00,           // 0x25  '8 *'
    0x0A,    0x0A,    0x81,    0x00,           // 0x26  '9 ('
    0x0B,    0x0B,    0x82,    0x00,           // 0x27  '0 )'
    0x1C,    0x1C,    0x1C,    0x1C,           // 0x28  'Enter'
    0x01,    0x01,    0x01,    0x01,           // 0x29  'Esc'
    0x0E,    0x0E,    0x0E,    0x0E,           // 0x2A  'Backspace'
    0x0F,    0x0F,    0xA5,    0x94,           // 0x2B  'Tab'
    0x39,    0x39,    0x39,    0x39,           // 0x2C  'Space bar'
    0x0C,    0x0C,    0x82,    0x0C,           // 0x2D  '- _'
    0x0D,    0x0D,    0x83,    0x00,           // 0x2E  '= +'
    0x1A,    0x1A,    0x1A,    0x1A,           // 0x2F  '[ {'
    0x1B,    0x1B,    0x1B,    0x1B,           // 0x30  '] }'
    0x2B,    0x2B,    0x2B,    0x2B,           // 0x31  '\ |'
    0x2B,    0x2B,    0x2B,    0x2B,           // 0x32  Non-US '# ~' ******
    0x27,    0x27,    0x27,    0x00,           // 0x33  '; :'
    0x28,    0x28,    0x28,    0x00,           // 0x34  '' "'
    0x29,    0x29,    0x29,    0x00,           // 0x35  ;` ~'
    0x33,    0x33,    0x33,    0x00,           // 0x36  ', <'
    0x34,    0x34,    0x34,    0x00,           // 0x37  '. >'
    0x35,    0x35,    0x35,    0x00,           // 0x38  '/ ?'
    0x3A,    0x3A,    0x3A,    0x3A,           // 0x39  'CapsLock'
    0x3B,    0x54,    0x68,    0x5E,           // 0x3A  'F1'
    0x3C,    0x55,    0x69,    0x5F,           // 0x3B  'F2'
    0x3D,    0x56,    0x6A,    0x60,           // 0x3C  'F3'
    0x3E,    0x57,    0x6B,    0x61,           // 0x3D  'F4'
    0x3F,    0x58,    0x6C,    0x62,           // 0x3E  'F5'
    0x40,    0x59,    0x6D,    0x63,           // 0x3F  'F6'
    0x41,    0x5A,    0x6E,    0x64,           // 0x40  'F7'
    0x42,    0x5B,    0x6F,    0x65,           // 0x41  'F8'
    0x43,    0x5C,    0x70,    0x66,           // 0x42  'F9'
    0x44,    0x5D,    0x71,    0x67,           // 0x43  'F10'
    0x57,    0x87,    0x8B,    0x89,           // 0x44  'F11'
    0x58,    0x88,    0x8C,    0x8A,           // 0x45  'F12'
    0x80,    0x80,    0x80,    0x72,           // 0x46  'PrntScrn'         E0,2A,E0,37     E0,B7,E0,AA
    0x46,    0x46,    0x46,    0x46,           // 0x47  'ScrollLock'
    0x81,    0x81,    0x81,    0x81,           // 0x48  'Pause'            E1,1D,45        E1,9D,C5
    0xD2,    0xD2,    0xA2,    0x92,           // 0x49  'Insert'           E0,52           E0,D2
    0xC7,    0xC7,    0x97,    0x77,           // 0x4A  'Home'             E0,47           E0,C7
    0xC9,    0xC9,    0x99,    0x84,           // 0x4B  'PageUp'           E0,49           E0,C9
    0xD3,    0xD3,    0xA3,    0x93,           // 0x4C  'Delete Forward'   E0,53           E0,D3
    0xCF,    0xCF,    0x9F,    0x75,           // 0x4D  'End'              E0,4F           E0,CF
    0xD1,    0xD1,    0xA1,    0x76,           // 0x4E  'PageDown'         E0,51           E0,D1
    0xCD,    0xCD,    0x9D,    0x74,           // 0x4F  'Right Arrow'      E0,4D           E0,CD
    0xCB,    0xCB,    0x9B,    0x73,           // 0x50  'Left Arrow'       E0,4B           E0,CB
    0xD0,    0xD0,    0xA0,    0x91,           // 0x51  'Down Arrow'       E0,50           E0,D0
    0xC8,    0xC8,    0x98,    0x8D,           // 0x52  'Up Arrow'         E0,48           E0,C8
    0x45,    0x45,    0x45,    0x45,           // 0x53  'NumLock'
    0xB5,    0xB5,    0xA4,    0x95,           // 0x54  'keypad /'         E0,35           E0,B5
    0x37,    0x37,    0x37,    0x96,           // 0x55  'keypad *'
    0x4A,    0x4A,    0x4A,    0x8E,           // 0x56  'keypad -'
    0x4E,    0x4E,    0x4E,    0x90,           // 0x57  'keypad +'
    0x9C,    0x9C,    0xA6,    0x9C,           // 0x58  'keypad Enter'     E0,1C           E0,9C
    0x4F,    0x4F,    0x9F,    0x75,           // 0x59  'keypad 1 End'
    0x50,    0x50,    0xA0,    0x91,           // 0x5A  'keypad 2 DownArrow'
    0x51,    0x51,    0xA1,    0x76,           // 0x5B  'keypad 3 PageDn'
    0x4B,    0x4B,    0x9B,    0x73,           // 0x5C  'keypad 4 LeftArrow'
    0x4C,    0x4C,    0x00,    0x8F,           // 0x5D  'keypad 5'
    0x4D,    0x4D,    0x9D,    0x74,           // 0x5E  'keypad 6 RightArrow'
    0x47,    0x47,    0x97,    0x77,           // 0x5F  'keypad 7 Home'
    0x48,    0x48,    0x98,    0x8D,           // 0x60  'keypad 8 UpArrow'
    0x49,    0x49,    0x99,    0x84,           // 0x61  'keypad 9 PageUp'
    0x52,    0x52,    0xA2,    0x92,           // 0x62  'keypad 0 Insert'
    0x53,    0x53,    0xA3,    0x93,           // 0x63  'keypad . Del'
    0x56,    0x56,    0x56,    0x56,           // 0x64  '\ |'
    0xDD,    0xDD,    0xDD,    0xDD            // 0x65  'Application'      E0,5D           E0,DD
};

STATIC
UINT8 KbcModCodeTable[] = {
    0x1D,           // 0x00  'Left Ctrl'
    0x2A,           // 0x01  'Left Shift'
    0x38,           // 0x02  'Left Alt'
    0xDB,           // 0x03  'Left Windows'     E0,5B           E0,DB
    0x9D,           // 0x04  'Right Ctrl'       E0,1D           E0,9D
    0x36,           // 0x05  'Right Shift'
    0xB8,           // 0x06  'Right Alt'        E0,38           E0,B8
    0xDC            // 0x07  'Right Windows'    E0,5C           E0,DC
};

STATIC
UINT8 CtrlIgnoreKey[] = {
    0x1E,           // '1','!'
    0x20,           // '3','#'
    0x21,           // '4','$'
    0x22,           // '5','%'
    0x24,           // '7','&'
    0x25,           // '8','*'
    0x26,           // '9','('
    0x27,           // '0',')'
    0x2E,           // '=','+'
    0x33,           // ';',':'
    0x34,           // '\'','"'
    0x35,           // '`','~'
    0x36,           // ',','<'
    0x37,           // '.','>'
    0x38            // '/','?'
};

STATIC
UINT8 AltIgnoreKey[] = {
    0x4C,           // 0x4C  'Delete Forward'   E0,53   
    0x52,           // 0x52  'Up Arrow'         E0,48           E0,C8
    0x51,           // 0x51  'Down Arrow'       E0,50           E0,D0
    0x50,           // 0x50  'Left Arrow'       E0,4B           E0,CB
    0x4F,           // 0x4F  'Right Arrow'      E0,4D           E0,CD  
    0x62            // 0x62  'keypad 0 Insert'
};

STATIC
UINT8 KbcCodeTableJp109[] = {
    0x73,           // 0x87  '\  _'        Key Number #56
    0x70,           // 0x88  'KANA/HIRA'   Key Number #133
    0x7D,           // 0x89  '\  |'        Key Number #14
    0x79,           // 0x8A  'HENKAN'      Key Number #132
    0x7B            // 0x8B  'MU-HENKAN'   Key Number #131
};

/**

  Get Next Item

  @param  StartPos              Start Position
  @param  EndPos                End Position
  @param  HidItem               HidItem to return

  @retval Position

**/
STATIC
UINT8 *
GetNextItem (
  IN  UINT8    *StartPos,
  IN  UINT8    *EndPos,
  OUT HID_ITEM *HidItem
  )
{
  UINT8 Temp;

  if ((EndPos - StartPos) <= 0) {
    return NULL;
  }

  Temp = *StartPos;
  StartPos++;
  //
  // bit 2,3
  //
  HidItem->Type = (UINT8) ((Temp >> 2) & 0x03);
  //
  // bit 4-7
  //
  HidItem->Tag = (UINT8) ((Temp >> 4) & 0x0F);

  if (HidItem->Tag == HID_ITEM_TAG_LONG) {
    //
    // Long Items are not supported by HID rev1.0,
    // although we try to parse it.
    //
    HidItem->Format = HID_ITEM_FORMAT_LONG;

    if ((EndPos - StartPos) >= 2) {
      HidItem->Size = *StartPos++;
      HidItem->Tag  = *StartPos++;

      if ((EndPos - StartPos) >= HidItem->Size) {
        HidItem->Data.LongData = StartPos;
        StartPos += HidItem->Size;
        return StartPos;
      }
    }
  } else {
    HidItem->Format = HID_ITEM_FORMAT_SHORT;
    //
    // bit 0, 1
    //
    HidItem->Size   = (UINT8) (Temp & 0x03);
    switch (HidItem->Size) {

    case 0:
      //
      // No data
      //
      return StartPos;

    case 1:
      //
      // One byte data
      //
      if ((EndPos - StartPos) >= 1) {
        HidItem->Data.U8 = *StartPos++;
        return StartPos;
      }

    case 2:
      //
      // Two byte data
      //
      if ((EndPos - StartPos) >= 2) {
        HidItem->Data.U16 = *(UINT16*)StartPos;
        StartPos += 2;
        return StartPos;
      }

    case 3:
      //
      // 4 byte data, adjust size
      //
      HidItem->Size++;
      if ((EndPos - StartPos) >= 4) {
        HidItem->Data.U32 = *(UINT32*)StartPos;
        StartPos += 4;
        return StartPos;
      }
    }
  }

  return NULL;
}

/**

  Get Item Data

  @param  HidItem               HID_ITEM

  @retval HidItem Data
  
**/
STATIC
UINT32
GetItemData (
  IN  HID_ITEM *HidItem
  )
{
  //
  // Get Data from HID_ITEM structure
  //
  switch (HidItem->Size) {

  case 1:
    return HidItem->Data.U8;

  case 2:
    return HidItem->Data.U16;

  case 4:
    return HidItem->Data.U32;
  }

  return 0;
}

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
  )
{
  UINT8  Index;

  //
  // Make sure KeyCode is in the range of [0x4, 0x65] or [0xe0, 0xe7]
  //
  if ((!USBKBD_VALID_KEYCODE (KeyCode)) || ((KeyCode > 0x65) && (KeyCode < 0xe0)) || (KeyCode > 0xe7)) {
    return NULL;
  }

  //
  // Calculate the index of Key Descriptor in Key Convertion Table
  //
  if (KeyCode <= 0x65) {
    Index = (UINT8) (KeyCode - 4);
  } else {
    Index = (UINT8) (KeyCode - 0xe0 + NUMBER_OF_VALID_NON_MODIFIER_USB_KEYCODE);
  }

  return &UsbKeyboardDevice->KeyConvertionTable[Index];
}

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
  )
{
  LIST_ENTRY      *Link;
  LIST_ENTRY      *NsKeyList;
  USB_NS_KEY      *UsbNsKey;

  NsKeyList = &UsbKeyboardDevice->NsKeyList;
  Link = GetFirstNode (NsKeyList);
  while (!IsNull (NsKeyList, Link)) {
    UsbNsKey = USB_NS_KEY_FORM_FROM_LINK (Link);

    if (UsbNsKey->NsKey[0].Key == KeyDescriptor->Key) {
      return UsbNsKey;
    }

    Link = GetNextNode (NsKeyList, Link);
  }

  return NULL;
}

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
  )
{
  UINTN               Index;
  EFI_KEY_DESCRIPTOR  *PhysicalKey;

  PhysicalKey = &UsbNsKey->NsKey[1];
  for (Index = 0; Index < UsbNsKey->KeyCount; Index++) {
    if (KeyDescriptor->Key == PhysicalKey->Key) {
      return PhysicalKey;
    }

    PhysicalKey++;
  }

  //
  // No children definition matched, return original key
  //
  return KeyDescriptor;
}

/**

  Parse Report Descriptor to Finding LED support

  @param  ReportDescriptor      Report descriptor to parse
  @param  ReportSize            Report descriptor size

**/
STATIC
BOOLEAN
ParseReportToFindingLed (
  IN  UINT8                     *ReportDescriptor,
  IN  UINTN                     ReportSize
  )
{
  UINT8     *DescriptorEnd;
  UINT8     *ptr;
  HID_ITEM  HidItem;
  UINT8     CurrentUsagePage = 0;
  UINT8     CurrentUsage = 0;
  
  DescriptorEnd = ReportDescriptor + ReportSize;
  ptr           = GetNextItem (ReportDescriptor, DescriptorEnd, &HidItem);

  while (ptr != NULL) {
    if (HidItem.Format != HID_ITEM_FORMAT_SHORT) {
      //
      // Long Format Item is not supported at current HID revision
      //
      return 0;
    }
    switch (HidItem.Type) {

    case HID_ITEM_TYPE_GLOBAL:
      //
      // For global Item, parse global item
      //
      if (HidItem.Tag == HID_GLOBAL_ITEM_TAG_USAGE_PAGE) {
        CurrentUsagePage = (UINT8) GetItemData (&HidItem);
      }
      break;

    case HID_ITEM_TYPE_LOCAL:
      //
      // For Local Item, parse local item
      //
      if (HidItem.Tag == HID_LOCAL_ITEM_TAG_USAGE) {
        CurrentUsage = (UINT8) GetItemData (&HidItem);
      }
      //
      // Check Touch device by Usage Page is DIGITIZERS and Usage is Touch Screen,
      // or Usage Page is DIGITIZERS and Usage is PEN
      //
      if (CurrentUsagePage == HID_UP_LED) {
        return TRUE;
      }
      break;

    }
    ptr = GetNextItem (ptr, DescriptorEnd, &HidItem);
  }
  return FALSE;
}

/**

  Check is it support LED in this interface

  @param  UsbKeyboardDevice     The USB_KB_DEV instance.
  
**/  
STATIC
BOOLEAN
CheckLedExist (
  IN USB_KB_DEV   *UsbKeyboardDevice
  )
{
  EFI_STATUS                    Status;
  EFI_USB_IO_PROTOCOL           *UsbIo;
  EFI_USB_CORE_PROTOCOL         *UsbCore;
  EFI_USB_HID_DESCRIPTOR        KeyboardHidDesc;
  UINT8                         *ReportDesc;
  BOOLEAN                       LedSupported = FALSE;
  //
  // Assuming Interface 0 is major interface and support LED
  //
  if (UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber == 0) return TRUE;
  //
  // Examining the LED page for LED support
  //
  UsbCore = UsbKeyboardDevice->UsbCore;
  UsbIo   = UsbKeyboardDevice->UsbIo;
  Status = UsbCore->UsbGetHidDescriptor (
                      UsbIo,
                      UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
                      &KeyboardHidDesc
                      );
  if (EFI_ERROR (Status) || KeyboardHidDesc.HidClassDesc[0].DescriptorType != 0x22) {
    return FALSE;
  }
  Status = UsbCore->AllocateBuffer (
                      KeyboardHidDesc.HidClassDesc[0].DescriptorLength,
                      ALIGNMENT_32,
                      (VOID **)&ReportDesc
                      );
  if (EFI_ERROR(Status)) {                    
    return FALSE;
  }
  Status = UsbCore->UsbGetReportDescriptor (
                      UsbIo,
                      UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
                      KeyboardHidDesc.HidClassDesc[0].DescriptorLength,
                      ReportDesc
                      );
  if (!EFI_ERROR (Status)) {
    LedSupported = ParseReportToFindingLed (
                     ReportDesc,
                     KeyboardHidDesc.HidClassDesc[0].DescriptorLength
                     );
  }
  UsbCore->FreeBuffer (
             KeyboardHidDesc.HidClassDesc[0].DescriptorLength,
             ReportDesc
             );
  return LedSupported;
}

/**

  Uses USB I/O to check whether the device is a USB Keyboard device.

  @param  UsbIo                 Points to a USB I/O protocol instance.

**/ 
BOOLEAN
IsUsbKeyboard (
  IN  EFI_USB_IO_PROTOCOL       *UsbIo
  )
{
  EFI_STATUS                    Status;
  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDescriptor;

  //
  // Get the Default interface descriptor, currently we
  // assume it is interface 1
  //
  Status = UsbIo->UsbGetInterfaceDescriptor (
                    UsbIo,
                    &InterfaceDescriptor
                    );

  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if (InterfaceDescriptor.InterfaceClass == CLASS_HID &&
      InterfaceDescriptor.InterfaceSubClass == SUBCLASS_BOOT &&
      InterfaceDescriptor.InterfaceProtocol == PROTOCOL_KEYBOARD
      ) {

    return TRUE;
  }

  return FALSE;
}

/**
  Initialize Key Convention Table by using default keyboard layout.

  @param[in, out]  UsbKeyboardDevice    The USB_KB_DEV instance.

  @retval EFI_SUCCESS          The default keyboard layout was installed successfully
  @retval Others               Failure to install default keyboard layout.
**/
EFI_STATUS
InstallDefaultKeyboardLayout (
   IN OUT USB_KB_DEV           *UsbKeyboardDevice
  )
{
  EFI_STATUS                   Status;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HII_HANDLE               HiiHandle;
  EFI_GUID                     *DefaultKeyboardLayout;

  //
  // Locate Hii database protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **) &HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Install Keyboard Layout package to HII database
  //
  HiiHandle = HiiAddPackages (
                &gH2OUsbKeyboardLayoutPackageGuid,
                UsbKeyboardDevice->ControllerHandle,
                &mUsbKeyboardLayoutBin,
                NULL
                );
  if (HiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  DefaultKeyboardLayout = (EFI_GUID *)PcdGetPtr(PcdH2ODefautKeyboardLayout);

  //
  // Set current keyboard layout
  //
  Status = HiiDatabase->SetKeyboardLayout (HiiDatabase, DefaultKeyboardLayout);

  return Status;
}

/**
  Get current keyboard layout from HII database.

  @return Pointer to HII Keyboard Layout.
          NULL means failure occurred while trying to get keyboard layout.

**/
EFI_HII_KEYBOARD_LAYOUT *
GetCurrentKeyboardLayout (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_HII_DATABASE_PROTOCOL *HiiDatabase;
  EFI_HII_KEYBOARD_LAYOUT   *KeyboardLayout;
  UINT16                    Length;

  //
  // Locate HII Database Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **) &HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // Get current keyboard layout from HII database
  //
  Length         = 0;
  KeyboardLayout = NULL;
  Status = HiiDatabase->GetKeyboardLayout (
                          HiiDatabase,
                          NULL,
                          &Length,
                          KeyboardLayout
                          );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    KeyboardLayout = AllocatePool (Length);
    ASSERT (KeyboardLayout != NULL);

    Status = HiiDatabase->GetKeyboardLayout (
                            HiiDatabase,
                            NULL,
                            &Length,
                            KeyboardLayout
                            );
    if (EFI_ERROR (Status)) {
      FreePool (KeyboardLayout);
      KeyboardLayout = NULL;
    }
  }

  return KeyboardLayout;
}

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
  )
{
  USB_KB_DEV                *UsbKeyboardDevice;
  EFI_HII_KEYBOARD_LAYOUT   *KeyboardLayout;
  EFI_KEY_DESCRIPTOR        TempKey;
  EFI_KEY_DESCRIPTOR        *KeyDescriptor;
  EFI_KEY_DESCRIPTOR        *TableEntry;
  EFI_KEY_DESCRIPTOR        *NsKey;
  USB_NS_KEY                *UsbNsKey;
  UINTN                     Index;
  UINTN                     Index2;
  UINTN                     KeyCount;
  UINT8                     KeyCode;

  UsbKeyboardDevice = (USB_KB_DEV *) Context;
  if (UsbKeyboardDevice->Signature != USB_KB_DEV_SIGNATURE) {
    return;
  }

  //
  // Try to get current keyboard layout from HII database
  //
  KeyboardLayout = GetCurrentKeyboardLayout ();
  if (KeyboardLayout == NULL) {
    return;
  }

  //
  // Re-allocate resource for KeyConvertionTable
  //
  ReleaseKeyboardLayoutResources (UsbKeyboardDevice);
  UsbKeyboardDevice->KeyConvertionTable = AllocateZeroPool ((NUMBER_OF_VALID_USB_KEYCODE) * sizeof (EFI_KEY_DESCRIPTOR));
  ASSERT (UsbKeyboardDevice->KeyConvertionTable != NULL);

  //
  // Traverse the list of key descriptors following the header of EFI_HII_KEYBOARD_LAYOUT
  //
  KeyDescriptor = (EFI_KEY_DESCRIPTOR *) (((UINT8 *) KeyboardLayout) + sizeof (EFI_HII_KEYBOARD_LAYOUT));
  for (Index = 0; Index < KeyboardLayout->DescriptorCount; Index++) {
    //
    // Copy from HII keyboard layout package binary for alignment
    //
    CopyMem (&TempKey, KeyDescriptor, sizeof (EFI_KEY_DESCRIPTOR));

    //
    // Fill the key into KeyConvertionTable, whose index is calculated from USB keycode.
    //
    KeyCode = EfiKeyToUsbKeyCodeConvertionTable [(UINT8) (TempKey.Key)];
    TableEntry = GetKeyDescriptor (UsbKeyboardDevice, KeyCode);
    if (TableEntry == NULL) {
      ReleaseKeyboardLayoutResources (UsbKeyboardDevice);
      FreePool (KeyboardLayout);
      return;
    }
    CopyMem (TableEntry, KeyDescriptor, sizeof (EFI_KEY_DESCRIPTOR));

    //
    // For non-spacing key, create the list with a non-spacing key followed by physical keys.
    //
    if (TempKey.Modifier == EFI_NS_KEY_MODIFIER) {
      UsbNsKey = AllocateZeroPool (sizeof (USB_NS_KEY));
      if (UsbNsKey == NULL) return;
      //
      // Search for sequential children physical key definitions
      //
      KeyCount = 0;
      NsKey = KeyDescriptor + 1;
      for (Index2 = (UINT8) Index + 1; Index2 < KeyboardLayout->DescriptorCount; Index2++) {
        CopyMem (&TempKey, NsKey, sizeof (EFI_KEY_DESCRIPTOR));
        if (TempKey.Modifier == EFI_NS_KEY_DEPENDENCY_MODIFIER) {
          KeyCount++;
        } else {
          break;
        }
        NsKey++;
      }

      UsbNsKey->Signature = USB_NS_KEY_SIGNATURE;
      UsbNsKey->KeyCount = KeyCount;
      UsbNsKey->NsKey = AllocateCopyPool (
                          (KeyCount + 1) * sizeof (EFI_KEY_DESCRIPTOR),
                          KeyDescriptor
                          );
      InsertTailList (&UsbKeyboardDevice->NsKeyList, &UsbNsKey->Link);

      //
      // Skip over the child physical keys
      //
      Index += KeyCount;
      KeyDescriptor += KeyCount;
    }

    KeyDescriptor++;
  }

  //
  // There are two EfiKeyEnter, duplicate its key descriptor
  //
  TableEntry = GetKeyDescriptor (UsbKeyboardDevice, 0x58);
  KeyDescriptor = GetKeyDescriptor (UsbKeyboardDevice, 0x28);
  CopyMem (TableEntry, KeyDescriptor, sizeof (EFI_KEY_DESCRIPTOR));
  CopyMem (&UsbKeyboardDevice->CurrentLayoutGuid, ((UINT8 *) KeyboardLayout) + sizeof (UINT16), sizeof (EFI_GUID));

  FreePool (KeyboardLayout);
}

/**
  Destroy resources for keyboard layout.

  @param[in, out]  UsbKeyboardDevice    The USB_KB_DEV instance.

**/
VOID
ReleaseKeyboardLayoutResources (
  IN OUT USB_KB_DEV              *UsbKeyboardDevice
  )
{
  USB_NS_KEY      *UsbNsKey;
  LIST_ENTRY      *Link;

  if (UsbKeyboardDevice->KeyConvertionTable != NULL) {
    FreePool (UsbKeyboardDevice->KeyConvertionTable);
  }
  UsbKeyboardDevice->KeyConvertionTable = NULL;

  while (!IsListEmpty (&UsbKeyboardDevice->NsKeyList)) {
    Link = GetFirstNode (&UsbKeyboardDevice->NsKeyList);
    UsbNsKey = USB_NS_KEY_FORM_FROM_LINK (Link);
    RemoveEntryList (&UsbNsKey->Link);

    FreePool (UsbNsKey->NsKey);
    FreePool (UsbNsKey);
  }
}

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
  )
{
  EFI_HII_KEYBOARD_LAYOUT   *KeyboardLayout;
  EFI_STATUS                Status;

  UsbKeyboardDevice->KeyConvertionTable = AllocateZeroPool ((NUMBER_OF_VALID_USB_KEYCODE) * sizeof (EFI_KEY_DESCRIPTOR));
  ASSERT (UsbKeyboardDevice->KeyConvertionTable != NULL);

  InitializeListHead (&UsbKeyboardDevice->NsKeyList);
  UsbKeyboardDevice->CurrentNsKey        = NULL;
  UsbKeyboardDevice->KeyboardLayoutEvent = NULL;

  //
  // Register event to EFI_HII_SET_KEYBOARD_LAYOUT_EVENT_GUID group,
  // which will be triggered by EFI_HII_DATABASE_PROTOCOL.SetKeyboardLayout().
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  SetKeyboardLayoutEvent,
                  UsbKeyboardDevice,
                  &gEfiHiiKeyBoardLayoutGuid,
                  &UsbKeyboardDevice->KeyboardLayoutEvent
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  KeyboardLayout = GetCurrentKeyboardLayout ();
  if (KeyboardLayout != NULL) {
    //
    // If current keyboard layout is successfully retrieved from HII database,
    // force to initialize the keyboard layout.
    //
    gBS->SignalEvent (UsbKeyboardDevice->KeyboardLayoutEvent);
  } else {
    //
    // If no keyboard layout can be retrieved from HII database, and default layout
    // is enabled, then load the default keyboard layout.
    //
    InstallDefaultKeyboardLayout (UsbKeyboardDevice);
  }

  return EFI_SUCCESS;
}

/*++

  Initialize USB Keyboard device and all private data structures.
  
  @param  UsbKeyboardDevice     The USB_KB_DEV instance.
  
  @retval EFI_SUCCESS           Success
  @retval EFI_DEVICE_ERROR      Hardware Error

--*/
EFI_STATUS
InitUsbKeyboard (
  IN USB_KB_DEV   *UsbKeyboardDevice
  )
{
  UINT8                 Protocol;
  EFI_USB_IO_PROTOCOL   *UsbIo;
  EFI_USB_CORE_PROTOCOL *UsbCore;

  UsbIo   = UsbKeyboardDevice->UsbIo;
  UsbCore = UsbKeyboardDevice->UsbCore;

  KbdReportStatusCode (
    UsbKeyboardDevice,
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_KEYBOARD_PC_SELF_TEST)
    );

  InitUsbKeyBuffer (&(UsbKeyboardDevice->KeyboardBuffer));
  //
  // Get current protocol
  //
  UsbCore->UsbGetProtocolRequest (
             UsbKeyboardDevice->UsbIo,
             UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
             &Protocol
             );
  //
  // Stall 2ms after GetProtocol command to workaround slow devices failed on
  // accepting subsequently command issue
  //
  UsbCore->Stall (2 * 1000);
  //
  // Sets boot protocol for the USB Keyboard.
  //
  if (Protocol != BOOT_PROTOCOL) {
    UsbCore->UsbSetProtocolRequest (
               UsbKeyboardDevice->UsbIo,
               UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
               BOOT_PROTOCOL
               );
  }
  //
  // Idle forever until there is a key pressed and released.
  //
  UsbCore->UsbSetIdleRequest (
             UsbKeyboardDevice->UsbIo,
             UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
             0,
             0
             );

  UsbKeyboardDevice->CtrlOn     = 0;
  UsbKeyboardDevice->AltOn      = 0;
  UsbKeyboardDevice->ShiftOn    = 0;
  if (UsbKeyboardDevice->UsbCore->IsCsmEnabled() == EFI_SUCCESS) {
    UsbKeyboardDevice->NumLockOn  = 0;
    UsbKeyboardDevice->CapsOn     = 0;
    UsbKeyboardDevice->ScrollOn   = 0;
  }
  UsbKeyboardDevice->LeftCtrlOn   = 0;
  UsbKeyboardDevice->LeftAltOn    = 0;
  UsbKeyboardDevice->LeftShiftOn  = 0;  
  UsbKeyboardDevice->LeftLogoOn   = 0;  
  UsbKeyboardDevice->RightCtrlOn  = 0;
  UsbKeyboardDevice->RightAltOn   = 0;
  UsbKeyboardDevice->RightShiftOn = 0;  
  UsbKeyboardDevice->RightLogoOn  = 0;  
  UsbKeyboardDevice->MenuKeyOn    = 0;
  UsbKeyboardDevice->SysReqOn     = 0;  
  UsbKeyboardDevice->IsSupportPartialKey = 0;
  UsbKeyboardDevice->AltGrOn                      = 0;
  UsbKeyboardDevice->CircumflexAccentOn           = 0;
  UsbKeyboardDevice->CircumflexAccentShiftedOn    = 0;
  UsbKeyboardDevice->GraveAccentOn                = 0;
  UsbKeyboardDevice->TildeOn                      = 0;
  UsbKeyboardDevice->CurrentNsKey                 = NULL;
  if (UsbKeyboardDevice->RepeatPollingHandle) {
    UsbCore->RemovePeriodicTimer (
               UsbKeyboardDevice->RepeatPollingHandle
               );
    UsbKeyboardDevice->RepeatPollingHandle = NULL;
    UsbKeyboardDevice->RepeatKey           = 0;
  }
  //
  // Check is it support LED
  //
  UsbKeyboardDevice->LedSupported = CheckLedExist (UsbKeyboardDevice);
  if (UsbKeyboardDevice->UsbCore->IsCsmEnabled() == EFI_SUCCESS) {
    //
    // Sync the initial state of LED with BDAs qualifier stat 
    //
    UsbKeyboardCheckLEDHandler(1, UsbKeyboardDevice);
  } else {
    SetKeyLED(UsbKeyboardDevice);
  }
  ZeroMem (UsbKeyboardDevice->LastKeyCodeArray, sizeof (UINT8) * 8);

  return EFI_SUCCESS;
}

/**

  Handler function for USB Keyboard's asynchronous interrupt transfer.
  
  @param  Data                  A pointer to a buffer that is filled with key data which is
                                retrieved via asynchronous interrupt transfer.
  @param  DataLength            Indicates the size of the data buffer.
  @param  Context               Pointing to USB_KB_DEV instance.
  @param  Result                Indicates the result of the asynchronous interrupt transfer.
  
  @retval EFI_SUCCESS           Success
  @retval EFI_DEVICE_ERROR      Hardware Error

**/  
EFI_STATUS
EFIAPI
KeyboardHandler (
  IN  VOID          *Data,
  IN  UINTN         DataLength,
  IN  VOID          *Context,
  IN  UINT32        Result
  )
{
  USB_KB_DEV            *UsbKeyboardDevice;
  EFI_USB_IO_PROTOCOL   *UsbIo;
  EFI_USB_CORE_PROTOCOL *UsbCore;
  UINT8                 *CurKeyCodeBuffer;
  UINT8                 *OldKeyCodeBuffer;
  UINT8                 CurModifierMap;
  UINT8                 OldModifierMap;
  UINT8                 Index;
  UINT8                 Index2;
  BOOLEAN               Down;
  EFI_STATUS            Status;
  BOOLEAN               KeyRelease;
  BOOLEAN               KeyPress;
  UINT8                 SavedTail;
  USB_KEY               UsbKey;
  UINT8                 NewRepeatKey;
  UINT32                UsbStatus;
  UINTN                 Mode;
  EFI_KEY_DESCRIPTOR    *KeyDescriptor;
  UINT8                 CtrlOn;
  UINT8                 AltOn;

  ASSERT (Context);

  NewRepeatKey      = 0;
  UsbKeyboardDevice = (USB_KB_DEV *) Context;
  UsbIo             = UsbKeyboardDevice->UsbIo;
  UsbCore           = UsbKeyboardDevice->UsbCore;
  //
  // Get current mode
  //
  UsbCore->GetMode(&Mode);
  //
  // Analyzes the Result and performs corresponding action.
  //
  if (Result != EFI_USB_NOERROR) {
    //
    // Some errors happen during the process
    //
    KbdReportStatusCode (
      UsbKeyboardDevice,
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      (EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_INPUT_ERROR)
      );
    //
    // stop the repeat key generation if any
    //
    UsbKeyboardDevice->RepeatKey = 0;
    
    if (UsbKeyboardDevice->RepeatPollingHandle) {
      UsbCore->RemovePeriodicTimer (
                 UsbKeyboardDevice->RepeatPollingHandle
                 );
      UsbKeyboardDevice->RepeatPollingHandle = NULL;
    }
    if ((Result & EFI_USB_ERR_STALL) == EFI_USB_ERR_STALL) {
      UsbCore->UsbClearEndpointHalt (
                 UsbIo,
                 UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
                 &UsbStatus
                 );
    }
    
    //
    // Delete & Submit this interrupt again
    //
    
    Status = UsbIo->UsbAsyncInterruptTransfer (
                      UsbIo,
                      UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
                      FALSE,
                      0,
                      0,
                      NULL,
                      NULL
                      );
    //
    // Check is the device be detached
    //
    if (UsbCore->CheckDeviceDetached(UsbIo) == EFI_SUCCESS) {
      return EFI_DEVICE_ERROR;
    }
    UsbCore->InsertPeriodicTimer(
               USB_CORE_ONCE_TIMER,
               UsbKeyboardRecoveryHandler,
               UsbKeyboardDevice,
               EFI_USB_INTERRUPT_DELAY / 10000,
               &UsbKeyboardDevice->RecoveryPollingHandle
               );

    return EFI_DEVICE_ERROR;
  }

  if (DataLength == 0 || Data == NULL) {
    return EFI_SUCCESS;
  }

  CurKeyCodeBuffer  = (UINT8 *) Data;
  OldKeyCodeBuffer  = UsbKeyboardDevice->LastKeyCodeArray;

  //
  // checks for new key stroke.
  // if no new key got, return immediately.
  //
  for (Index = 0; Index < 8; Index++) {
    if (OldKeyCodeBuffer[Index] != CurKeyCodeBuffer[Index]) {
      break;
    }
  }

  if (Index == 8) {
    return EFI_SUCCESS;
  }
  
  //
  // Parse the modifier key
  //
  CurModifierMap  = CurKeyCodeBuffer[0];
  OldModifierMap  = OldKeyCodeBuffer[0];

  //
  // handle modifier key's pressing or releasing situation.
  //
  for (Index = 0; Index < 8; Index++) {

    if ((CurModifierMap & KB_Mod[Index].Mask) != (OldModifierMap & KB_Mod[Index].Mask)) {
      //
      // if current modifier key is up, then
      // CurModifierMap & KB_Mod[Index].Mask = 0;
      // otherwize it is a non-zero value.
      // Inserts the pressed modifier key into key buffer.
      //
      Down = (UINT8) (CurModifierMap & KB_Mod[Index].Mask);
      InsertKeyCode (UsbKeyboardDevice, KB_Mod[Index].Key, Down);
    }
  }
  
  //
  // handle normal key's releasing situation
  //
  KeyRelease = FALSE;
  for (Index = 2; Index < 8; Index++) {

    if (!USBKBD_VALID_KEYCODE (OldKeyCodeBuffer[Index])) {
      continue;
    }

    KeyRelease = TRUE;
    for (Index2 = 2; Index2 < 8; Index2++) {

      if (!USBKBD_VALID_KEYCODE (CurKeyCodeBuffer[Index2])) {
        continue;
      }

      if (OldKeyCodeBuffer[Index] == CurKeyCodeBuffer[Index2]) {
        KeyRelease = FALSE;
        break;
      }
    }

    if (KeyRelease) {
      InsertKeyCode (
        UsbKeyboardDevice,
        OldKeyCodeBuffer[Index],
        0
        );
      //
      // the original reapeat key is released.
      //
      if (OldKeyCodeBuffer[Index] == UsbKeyboardDevice->RepeatKey) {
        UsbKeyboardDevice->RepeatKey = 0;
      }
    }
  }
    
  //
  // original repeat key is released, cancel the repeat timer
  //
  if (UsbKeyboardDevice->RepeatKey == 0 && UsbKeyboardDevice->RepeatPollingHandle != NULL) {
    UsbCore->RemovePeriodicTimer (
               UsbKeyboardDevice->RepeatPollingHandle
               );
    UsbKeyboardDevice->RepeatPollingHandle = NULL;
  }

  //
  // handle normal key's pressing situation
  //
  KeyPress = FALSE;
  for (Index = 2; Index < 8; Index++) {

    if (!USBKBD_VALID_KEYCODE (CurKeyCodeBuffer[Index])) {
      continue;
    }

    KeyPress = TRUE;
    for (Index2 = 2; Index2 < 8; Index2++) {

      if (!USBKBD_VALID_KEYCODE (OldKeyCodeBuffer[Index2])) {
        continue;
      }

      if (CurKeyCodeBuffer[Index] == OldKeyCodeBuffer[Index2]) {
        KeyPress = FALSE;
        break;
      }
    }

    if (KeyPress) {
      InsertKeyCode (UsbKeyboardDevice, CurKeyCodeBuffer[Index], 1);
      
      KeyDescriptor = GetKeyDescriptor (UsbKeyboardDevice, CurKeyCodeBuffer[Index]);
      //
      // NumLock/CapsLock/ScrollLock pressed
      //
      if (KeyDescriptor != NULL &&
          (KeyDescriptor->Modifier != EFI_NUM_LOCK_MODIFIER) && 
          (KeyDescriptor->Modifier != EFI_CAPS_LOCK_MODIFIER)  &&
          (KeyDescriptor->Modifier != EFI_SCROLL_LOCK_MODIFIER)) {
        NewRepeatKey = CurKeyCodeBuffer[Index];
      }
      //
      // do not repeat the original repeated key
      //
      UsbKeyboardDevice->RepeatKey = 0;
    }
  }
  
  //
  // Update LastKeycodeArray[] buffer in the
  // Usb Keyboard Device data structure.
  //
  for (Index = 0; Index < 8; Index++) {
    UsbKeyboardDevice->LastKeyCodeArray[Index] = CurKeyCodeBuffer[Index];
  }
  
  UsbCore->GetMode(&Mode);
  if (Mode != USB_CORE_RUNTIME_MODE) {
    //
    // pre-process KeyboardBuffer, pop out the ctrl,alt,del key in sequence
    // and judge whether it will invoke reset event.
    //
    SavedTail = UsbKeyboardDevice->KeyboardBuffer.bTail;
    Index     = UsbKeyboardDevice->KeyboardBuffer.bHead;
    CtrlOn    = 0;
    AltOn     = 0;
    while (Index != SavedTail) {
      UsbKey.KeyCode = UsbKeyboardDevice->KeyboardBuffer.buffer[Index].KeyCode;
      UsbKey.Down    = UsbKeyboardDevice->KeyboardBuffer.buffer[Index].Down;
      KeyDescriptor = GetKeyDescriptor (UsbKeyboardDevice, UsbKey.KeyCode);
      if (KeyDescriptor != NULL) {
        switch (KeyDescriptor->Modifier) {
        case EFI_LEFT_CONTROL_MODIFIER:
        case EFI_RIGHT_CONTROL_MODIFIER:
          if (UsbKey.Down) {
            CtrlOn = 1;
          } else {
            CtrlOn = 0;
          }
          break;
      
        case EFI_LEFT_ALT_MODIFIER:
        case EFI_RIGHT_ALT_MODIFIER:
          if (UsbKey.Down) {
            AltOn = 1;
          } else {
            AltOn = 0;
          }
          break;
      
        //
        // Del Key Code
        //
        case EFI_DELETE_MODIFIER:
          if (UsbKey.Down) {
            if (CtrlOn && AltOn) {
              gRT->ResetSystem (EfiResetWarm, EFI_SUCCESS, 0, KEYBOARD_RESET_SIGNATURE);
            }
          }
          break;
        }
      }
      Index = (UINT8) ((Index + 1) % (MAX_KEY_ALLOWED + 1));
    }
  }
  //
  // If have new key pressed, update the RepeatKey value, and set the
  // timer to repeate delay timer
  //
  if (NewRepeatKey != 0) {
    if (UsbKeyboardDevice->RepeatPollingHandle) {
      UsbCore->RemovePeriodicTimer(
                 UsbKeyboardDevice->RepeatPollingHandle
                 );
      UsbKeyboardDevice->RepeatPollingHandle = NULL;
    }
    //
    // sets trigger time to "Repeat Delay Time",
    // to trigger the repeat timer when the key is hold long
    // enough time.
    //
    UsbCore->InsertPeriodicTimer(
               USB_CORE_ONCE_TIMER,
               UsbKeyboardRepeatHandler,
               UsbKeyboardDevice,
               USBKBD_REPEAT_DELAY,
               &UsbKeyboardDevice->RepeatPollingHandle
               );
    UsbKeyboardDevice->RepeatKey = NewRepeatKey;
  }

  return EFI_SUCCESS;
}

/**

  Convert USB key code to KBC scan code
  
  @param  UsbKeyboardDevice     The USB_KB_DEV instance.
  @param  Key                   USB key code
  
  @retval KBC scan code

**/  
STATIC
UINT16
ConvertKbcScanCode (        
  IN USB_KB_DEV         *UsbKeyboardDevice,
  IN UINT8              Key
  )
{
  UINT16 KbcScanCode;

  if (Key < 4 || Key >= (sizeof (KbcCodeTable) / 4) + 4) return 0;
  if (UsbKeyboardDevice->AltOn) {
    KbcScanCode = KbcCodeTable[Key - 4][2];
  } else if (UsbKeyboardDevice->CtrlOn) {
    KbcScanCode = KbcCodeTable[Key - 4][3];
  } else if (UsbKeyboardDevice->ShiftOn) {
    KbcScanCode = KbcCodeTable[Key - 4][1];
  } else {
    KbcScanCode = KbcCodeTable[Key - 4][0];
    if (KbcScanCode == 0x57 || KbcScanCode == 0x58) {
      //
      // Convert F11/F12 to KBC converted code 
      //
      KbcScanCode = 0x85 + (KbcScanCode - 0x57);
    } else if (KbcScanCode >= 0x82) {
      //
      // Double code key
      //
      KbcScanCode &= ~0x80;
      KbcScanCode <<= 8;
      KbcScanCode |= 0xE0;
    }
  }
  return KbcScanCode;
}  

/**

  Retrieves a key character after parsing the raw data in keyboard buffer.
  
  @param  UsbKeyboardDevice     The USB_KB_DEV instance.
  @param  KeyChar               Points to the Key character after key parsing.
  
  @retval EFI_SUCCESS           Success
  @retval EFI_NOT_READY         Device is not ready

**/  
EFI_STATUS
UsbParseKey (
  IN OUT  USB_KB_DEV  *UsbKeyboardDevice,
  OUT     UINT8       *KeyChar
  )
{
  USB_KEY UsbKey;
  EFI_KEY_DESCRIPTOR  *KeyDescriptor = NULL;

  *KeyChar = 0;

  while (!IsUsbKeyboardBufferEmpty (&UsbKeyboardDevice->KeyboardBuffer)) {
    //
    // pops one raw data off.
    //
    RemoveKeyCode (&(UsbKeyboardDevice->KeyboardBuffer), &UsbKey);
    
    KeyDescriptor = GetKeyDescriptor (UsbKeyboardDevice, UsbKey.KeyCode);
    if (KeyDescriptor == NULL) {
      return EFI_NOT_READY;
    }

    if (!UsbKey.Down) {

      switch (KeyDescriptor->Modifier) {

      //
      // Ctrl release
      //
      case EFI_LEFT_CONTROL_MODIFIER:
        UsbKeyboardDevice->LeftCtrlOn = 0;
        UsbKeyboardDevice->CtrlOn = 0;
        break;
      case EFI_RIGHT_CONTROL_MODIFIER:
        UsbKeyboardDevice->RightCtrlOn = 0;
        UsbKeyboardDevice->CtrlOn = 0;
        break;

      //
      // Shift release
      //
      case EFI_LEFT_SHIFT_MODIFIER:
        UsbKeyboardDevice->LeftShiftOn = 0;
        UsbKeyboardDevice->ShiftOn = 0;
        break;
      case EFI_RIGHT_SHIFT_MODIFIER:
        UsbKeyboardDevice->RightShiftOn = 0;
        UsbKeyboardDevice->ShiftOn = 0;
        break;

      //
      // Alt release
      //
      case EFI_LEFT_ALT_MODIFIER:
        UsbKeyboardDevice->LeftAltOn = 0;
        UsbKeyboardDevice->AltOn = 0;
        break;
      case EFI_RIGHT_ALT_MODIFIER:
        UsbKeyboardDevice->RightAltOn = 0;
        UsbKeyboardDevice->AltOn = 0;
        break;

      //
      // Left Logo release
      //
      case EFI_LEFT_LOGO_MODIFIER:
        UsbKeyboardDevice->LeftLogoOn = 0;
        break;

      //
      // Right Logo release
      //
      case EFI_RIGHT_LOGO_MODIFIER:
        UsbKeyboardDevice->RightLogoOn = 0;
        break;

      //
      // Menu key release
      //
      case EFI_MENU_MODIFIER:
        UsbKeyboardDevice->MenuKeyOn = 0;
        break;

      //
      // SysReq release
      //
      case EFI_PRINT_MODIFIER:
      case EFI_SYS_REQUEST_MODIFIER:
        UsbKeyboardDevice->SysReqOn = 0;
        break;

      //
      // AltGr release
      //
      case EFI_ALT_GR_MODIFIER:
        UsbKeyboardDevice->AltGrOn = 0;
        break;
               
      default:
        break;
      }
      continue;
    }
    
    //
    // Analyzes key pressing situation
    //
    switch (KeyDescriptor->Modifier) {

    //
    // Ctrl press
    //
    case EFI_LEFT_CONTROL_MODIFIER:
      UsbKeyboardDevice->LeftCtrlOn = 1;
      UsbKeyboardDevice->CtrlOn = 1;
      break;
    case EFI_RIGHT_CONTROL_MODIFIER:
      UsbKeyboardDevice->RightCtrlOn = 1;
      UsbKeyboardDevice->CtrlOn = 1;
      break;

    //
    // Shift press
    //
    case EFI_LEFT_SHIFT_MODIFIER:
      UsbKeyboardDevice->LeftShiftOn = 1;
      UsbKeyboardDevice->ShiftOn = 1;
      break;
    case EFI_RIGHT_SHIFT_MODIFIER:
      UsbKeyboardDevice->RightShiftOn = 1;
      UsbKeyboardDevice->ShiftOn = 1;
      break;

    //
    // Alt press
    //
    case EFI_LEFT_ALT_MODIFIER:
      UsbKeyboardDevice->LeftAltOn = 1;
      UsbKeyboardDevice->AltOn = 1;
      break;
    case EFI_RIGHT_ALT_MODIFIER:
      UsbKeyboardDevice->RightAltOn = 1;
      UsbKeyboardDevice->AltOn = 1;
      break;

    //
    // Left Logo press
    //
    case EFI_LEFT_LOGO_MODIFIER:
      UsbKeyboardDevice->LeftLogoOn = 1;
      break;

    //
    // Right Logo press
    //
    case EFI_RIGHT_LOGO_MODIFIER:
      UsbKeyboardDevice->RightLogoOn = 1;
      break;

    //
    // Menu key press
    //
    case EFI_MENU_MODIFIER:
      UsbKeyboardDevice->MenuKeyOn = 1;
      break;

    //
    // SysReq press
    //
    case EFI_PRINT_MODIFIER:
    case EFI_SYS_REQUEST_MODIFIER:
      UsbKeyboardDevice->SysReqOn = 1;
      break;

    //
    // AltGr press
    //
    case EFI_ALT_GR_MODIFIER:
      UsbKeyboardDevice->AltGrOn = 1;
      break;

    case EFI_NUM_LOCK_MODIFIER:
      if (UsbKeyboardDevice->UsbCore->IsCsmEnabled() == EFI_SUCCESS) {
        if (!PS2_LED_UPDATING) {
          USB_LED_UPDATE;
          UsbKeyboardDevice->NumLockOn ^= 1;
          SetKeyLED (UsbKeyboardDevice);
          if (UsbKeyboardDevice->UsbCore->IsKbcExist() == EFI_NOT_FOUND) {
            UsbKeyboardDevice->UsbCore->SyncKbdLed();
          }
          USB_LED_UPDATED;
        }
      } else {
        UsbKeyboardDevice->NumLockOn ^= 1;
        UsbKeyboardCheckLEDHandler(0, UsbKeyboardDevice);
      }
      break;

    case EFI_CAPS_LOCK_MODIFIER:
      if (UsbKeyboardDevice->UsbCore->IsCsmEnabled() == EFI_SUCCESS) {
        if (!PS2_LED_UPDATING) {
      	  USB_LED_UPDATE;
          UsbKeyboardDevice->CapsOn ^= 1;
          SetKeyLED (UsbKeyboardDevice);
          if (UsbKeyboardDevice->UsbCore->IsKbcExist() == EFI_NOT_FOUND) {
            UsbKeyboardDevice->UsbCore->SyncKbdLed();
          }
          USB_LED_UPDATED;
        }
      } else {
        UsbKeyboardDevice->CapsOn ^= 1;
        UsbKeyboardCheckLEDHandler(0, UsbKeyboardDevice);
      }
      break;

    case EFI_SCROLL_LOCK_MODIFIER:
      if (UsbKeyboardDevice->UsbCore->IsCsmEnabled() == EFI_SUCCESS) {
        if (!PS2_LED_UPDATING) {
      	  USB_LED_UPDATE;
          UsbKeyboardDevice->ScrollOn ^= 1;
          SetKeyLED (UsbKeyboardDevice);
          if (UsbKeyboardDevice->UsbCore->IsKbcExist() == EFI_NOT_FOUND) {
            UsbKeyboardDevice->UsbCore->SyncKbdLed();
          }
          USB_LED_UPDATED;
        }
      } else {
        UsbKeyboardDevice->ScrollOn ^= 1;
        UsbKeyboardCheckLEDHandler(0, UsbKeyboardDevice);
      }
      break;

    case EFI_NULL_MODIFIER:
      //
      // Special IME for French Layout keyboard
      //
      if (CompareGuid (&UsbKeyboardDevice->CurrentLayoutGuid, &gH2OUsbKeyboardFrenchLayoutKeyGuid)) {
        if (KeyDescriptor->Key == EfiKeyD11) {
          if (UsbKeyboardDevice->ShiftOn) {
            if (UsbKeyboardDevice->CircumflexAccentShiftedOn == 0) {
              UsbKeyboardDevice->CircumflexAccentShiftedOn = 1;
              continue;
            }
          } else {
            if (UsbKeyboardDevice->CircumflexAccentOn == 0) {
              UsbKeyboardDevice->CircumflexAccentOn = 1;
              continue;
            }
          }
        }
        if ((KeyDescriptor->Key == EfiKeyE2) &&
            (UsbKeyboardDevice->AltGrOn)) {
          if (UsbKeyboardDevice->TildeOn == 0) {
            UsbKeyboardDevice->TildeOn = 1;
            continue;
          }
        }
        if ((KeyDescriptor->Key == EfiKeyE7) &&
            (UsbKeyboardDevice->AltGrOn)) {
          if (UsbKeyboardDevice->GraveAccentOn == 0) {
            UsbKeyboardDevice->GraveAccentOn = 1;
            continue;
          }
        }
      }
      break;
      
    default:
      break;
    }
    
    //
    // When encountered Del Key...
    //
    if (UsbKey.KeyCode == 0x4c || UsbKey.KeyCode == 0x63) {
      if (UsbKeyboardDevice->CtrlOn && UsbKeyboardDevice->AltOn) {
        gRT->ResetSystem (EfiResetWarm, EFI_SUCCESS, 0, KEYBOARD_RESET_SIGNATURE);
        //
        // Return EFI_NOT_READY in case some OEM feature ignore the reset in purposely through hook the ResetSystem function
        //
        return EFI_NOT_READY;
      }
    }

    *KeyChar = UsbKey.KeyCode;
    if ((UsbKeyboardDevice->UsbCore->IsCsmEnabled() == EFI_SUCCESS) && (UsbKey.Down) && (UsbKey.KeyCode < 0xe0) && (UsbKey.KeyCode != 0x46) && (UsbKey.KeyCode != 0x48)) {
      //
      // Put the KBC scan code into EBDA+0x164. The PrntScrn and Pause will be filter out to sync with PS2 path
      //
      *(UINT16*)&(EBDA(EBDA_KEYBORD_SCAN_CODE)) = ConvertKbcScanCode (UsbKeyboardDevice, UsbKey.KeyCode);
    }
    return EFI_SUCCESS;
  }

  return EFI_NOT_READY;

}

/**

  Converts USB Keyboard code to EFI Scan Code.
  
  @param  UsbKeyboardDevice     The USB_KB_DEV instance.
  @param  KeyChar               Indicates the key code that will be interpreted.    
  @param  Key                   A pointer to a buffer that is filled in with 
                                the keystroke information for the key that 
                                was pressed.

  @retval EFI_NOT_READY         Device is not ready
  @retval EFI_SUCCESS           Success

**/
EFI_STATUS
UsbKeyCodeToEFIScanCode (
  IN  USB_KB_DEV      *UsbKeyboardDevice,
  IN  UINT8           KeyChar,
  OUT EFI_INPUT_KEY   *Key
  )
{
  UINT8 Index;
  EFI_KEY_DESCRIPTOR    *KeyDescriptor = NULL;

  if (!USBKBD_VALID_KEYCODE (KeyChar)) {
    return EFI_NOT_READY;
  }
  
  KeyDescriptor = GetKeyDescriptor (UsbKeyboardDevice, KeyChar);
  if (KeyDescriptor == NULL) {
    return EFI_NOT_READY;
  }

  if (KeyDescriptor->Modifier == EFI_NS_KEY_MODIFIER) {
    //
    // If this is a dead key with EFI_NS_KEY_MODIFIER, then record it and return.
    //
    UsbKeyboardDevice->CurrentNsKey = FindUsbNsKey (UsbKeyboardDevice, KeyDescriptor);
    return EFI_NOT_READY;
  }

  if (UsbKeyboardDevice->CurrentNsKey != NULL) {
    //
    // If this keystroke follows a non-spacing key, then find the descriptor for corresponding
    // physical key.
    //
    KeyDescriptor = FindPhysicalKey (UsbKeyboardDevice->CurrentNsKey, KeyDescriptor);
    UsbKeyboardDevice->CurrentNsKey = NULL;
  }

  //
  // Make sure modifier of Key Descriptor is in the valid range according to UEFI spec.
  //
  if (KeyDescriptor->Modifier >= (sizeof (ModifierValueToEfiScanCodeConvertionTable) / sizeof (UINT8))) {
    return EFI_DEVICE_ERROR;
  }

  if ((KeyChar >= 0xe0) && (KeyChar <= 0xe7)) {
    //
    // For Partial Keystroke support
    //
    Key->ScanCode    = SCAN_NULL;
    Key->UnicodeChar = CHAR_NULL;
  } else {
  
    Key->ScanCode    = ModifierValueToEfiScanCodeConvertionTable[KeyDescriptor->Modifier];
    Key->UnicodeChar = KeyDescriptor->Unicode;
  
    if ((KeyDescriptor->AffectedAttribute & EFI_AFFECTED_BY_STANDARD_SHIFT)!= 0) {
      if (UsbKeyboardDevice->ShiftOn) {
        Key->UnicodeChar = KeyDescriptor->ShiftedUnicode;
  
        //
        // Need not return associated shift state if a class of printable characters that
        // are normally adjusted by shift modifiers. e.g. Shift Key + 'f' key = 'F'
        //
        if ((KeyDescriptor->AffectedAttribute & EFI_AFFECTED_BY_CAPS_LOCK) != 0) {
          UsbKeyboardDevice->LeftShiftOn = FALSE;
          UsbKeyboardDevice->RightShiftOn = FALSE;
        }
        if (UsbKeyboardDevice->AltGrOn) {
          Key->UnicodeChar = KeyDescriptor->ShiftedAltGrUnicode;
        }
      } else {
        //
        // Shift off
        //
        Key->UnicodeChar = KeyDescriptor->Unicode;
        
        if (UsbKeyboardDevice->AltGrOn) {
          Key->UnicodeChar = KeyDescriptor->AltGrUnicode;
        }
      }
    }

    if ((KeyDescriptor->AffectedAttribute & EFI_AFFECTED_BY_CAPS_LOCK) != 0) {
      if (UsbKeyboardDevice->CapsOn) {
        if (Key->UnicodeChar == KeyDescriptor->Unicode) {
          Key->UnicodeChar = KeyDescriptor->ShiftedUnicode;
        } else if (Key->UnicodeChar == KeyDescriptor->ShiftedUnicode) {
          Key->UnicodeChar = KeyDescriptor->Unicode;
        }
      }
    }
  
    //
    // Translate the CTRL-Alpha characters to their corresponding control value  
    // (ctrl-a = 0x0001 through ctrl-Z = 0x001A, ctrl-[, ctrl-\, ctrl-], ctrl-^ and ctrl-_)
    //
    if (UsbKeyboardDevice->CtrlOn) {
      for (Index = 0; Index < sizeof (CtrlIgnoreKey); Index++) {
        if (KeyChar == CtrlIgnoreKey[Index]) {
          return EFI_NOT_READY;
        }
      }
      
      if (Key->UnicodeChar >= 'a' && Key->UnicodeChar <= 'z') {
        Key->UnicodeChar = Key->UnicodeChar - 'a' + 1;
      } else if (Key->UnicodeChar >= 'A' && Key->UnicodeChar <= 'Z') {
        Key->UnicodeChar = Key->UnicodeChar - 'A' + 1;
      } else if (Key->UnicodeChar == '[' || Key->UnicodeChar == '{') {
        Key->ScanCode    = SCAN_ESC;
        Key->UnicodeChar = CHAR_NULL;
      } else if (Key->UnicodeChar == '\\' || Key->UnicodeChar == '|') {
        Key->UnicodeChar = 0x1C;
      } else if (Key->UnicodeChar == ']' || Key->UnicodeChar == '}') {
        Key->UnicodeChar = 0x1D;
      } else if (Key->UnicodeChar == '^' || Key->UnicodeChar == '6') {
        Key->UnicodeChar = 0x1E;
      } else if (Key->UnicodeChar == '_' || Key->UnicodeChar == '-') {
        Key->UnicodeChar = 0x1F;
      }
    }
    
    if (UsbKeyboardDevice->CircumflexAccentShiftedOn) {
      UsbKeyboardDevice->CircumflexAccentShiftedOn += 1;
      if (Key->UnicodeChar == 'a') {
        Key->UnicodeChar = 0xE4;
      } else if (Key->UnicodeChar == 'e') {
        Key->UnicodeChar = 0xEB;
      } else if (Key->UnicodeChar == 'i') {
        Key->UnicodeChar = 0xEF;
      } else if (Key->UnicodeChar == 'o') {
        Key->UnicodeChar = 0xF6;
      } else if (Key->UnicodeChar == 'u') {
        Key->UnicodeChar = 0xFC;
      } else if (Key->UnicodeChar == 'y') {
        Key->UnicodeChar = 0xFF;
      } else if (Key->UnicodeChar == 'A') {
        Key->UnicodeChar = 0xC4;
      } else if (Key->UnicodeChar == 'E') {
        Key->UnicodeChar = 0xCB;
      } else if (Key->UnicodeChar == 'I') {
        Key->UnicodeChar = 0xCF;
      } else if (Key->UnicodeChar == 'O') {
        Key->UnicodeChar = 0xD6;
      } else if (Key->UnicodeChar == 'U') {
        Key->UnicodeChar = 0xDC;
      } else {
        Key->UnicodeChar = 0xA8;
      }
      if (UsbKeyboardDevice->CircumflexAccentShiftedOn > 3)
        UsbKeyboardDevice->CircumflexAccentShiftedOn = 0;
    }
    
    if (UsbKeyboardDevice->CircumflexAccentOn) {
      UsbKeyboardDevice->CircumflexAccentOn += 1;
      if (Key->UnicodeChar == 'a') {
        Key->UnicodeChar = 0xE2;
      } else if (Key->UnicodeChar == 'e') {
        Key->UnicodeChar = 0xEA;
      } else if (Key->UnicodeChar == 'i') {
        Key->UnicodeChar = 0xEE;
      } else if (Key->UnicodeChar == 'o') {
        Key->UnicodeChar = 0xF4;
      } else if (Key->UnicodeChar == 'u') {
        Key->UnicodeChar = 0xFB;
      } else if (Key->UnicodeChar == 'A') {
        Key->UnicodeChar = 0xC2;
      } else if (Key->UnicodeChar == 'E') {
        Key->UnicodeChar = 0xCA;
      } else if (Key->UnicodeChar == 'I') {
        Key->UnicodeChar = 0xCE;
      } else if (Key->UnicodeChar == 'O') {
        Key->UnicodeChar = 0xD4;
      } else if (Key->UnicodeChar == 'U') {
        Key->UnicodeChar = 0xDB;
      } else {
        Key->UnicodeChar = '^';
      }
      if (UsbKeyboardDevice->CircumflexAccentOn > 3)
        UsbKeyboardDevice->CircumflexAccentOn = 0;
    }
    
    if (UsbKeyboardDevice->TildeOn) {
      UsbKeyboardDevice->TildeOn += 1;
      if (Key->UnicodeChar == 'a') {
        Key->UnicodeChar = 0xE3;
      } else if (Key->UnicodeChar == 'A') {
        Key->UnicodeChar = 0xC3;
      } else if (Key->UnicodeChar == 'n') {
        Key->UnicodeChar = 0xF1;
      } else if (Key->UnicodeChar == 'N') {
        Key->UnicodeChar = 0xD1;
      } else if (Key->UnicodeChar == 'o') {
        Key->UnicodeChar = 0xF5;
      } else if (Key->UnicodeChar == 'O') {
        Key->UnicodeChar = 0xD5;
      } else {
        Key->UnicodeChar = '~';
      }
      if (UsbKeyboardDevice->TildeOn > 3)
        UsbKeyboardDevice->TildeOn = 0;
    }
    
    if (UsbKeyboardDevice->GraveAccentOn) {
      UsbKeyboardDevice->GraveAccentOn += 1;
      if (Key->UnicodeChar == 'a') {
        Key->UnicodeChar = 0xE0;
      } else if (Key->UnicodeChar == 'e') {
        Key->UnicodeChar = 0xE8;
      } else if (Key->UnicodeChar == 'i') {
        Key->UnicodeChar = 0xEC;
      } else if (Key->UnicodeChar == 'o') {
        Key->UnicodeChar = 0xF2;
      } else if (Key->UnicodeChar == 'u') {
        Key->UnicodeChar = 0xF9;
      } else if (Key->UnicodeChar == 'A') {
        Key->UnicodeChar = 0xC0;
      } else if (Key->UnicodeChar == 'E') {
        Key->UnicodeChar = 0xC8;
      } else if (Key->UnicodeChar == 'I') {
        Key->UnicodeChar = 0xCC;
      } else if (Key->UnicodeChar == 'O') {
        Key->UnicodeChar = 0xD2;
      } else if (Key->UnicodeChar == 'U') {
        Key->UnicodeChar = 0xD9;
      } else {
        Key->UnicodeChar = 0x60;
      }
      if (UsbKeyboardDevice->GraveAccentOn> 3)
        UsbKeyboardDevice->GraveAccentOn = 0;
    }
  
    if ((KeyDescriptor->AffectedAttribute & EFI_AFFECTED_BY_NUM_LOCK) != 0) {
      //
      // For key affected by NumLock, if NumLock is on and Shift is not pressed, then it means
      // normal key, instead of original control key. So the ScanCode should be cleaned.
      // Otherwise, it means control key, so preserve the EFI Scan Code and clear the unicode keycode.
      //
      if ((UsbKeyboardDevice->NumLockOn) && (!(UsbKeyboardDevice->ShiftOn))) {
        Key->ScanCode = SCAN_NULL;
      } else {
        Key->UnicodeChar = CHAR_NULL;
      }
    }
    
    //
    // Translate Unicode 0x1B (ESC) to EFI Scan Code
    //
    if (Key->UnicodeChar == 0x1B && Key->ScanCode == SCAN_NULL) {
      Key->ScanCode    = SCAN_ESC;
      Key->UnicodeChar = CHAR_NULL;
    }

    if (UsbKeyboardDevice->AltOn) {
      for (Index = 0; Index < sizeof (AltIgnoreKey); Index++) {
        if (KeyChar == AltIgnoreKey[Index]) {
          return EFI_NOT_READY;
        }
      }

      //
      // Alt + numpad 1~9
      //
      if (KeyChar >= 0x59 && KeyChar <= 0x61) {
        Key->UnicodeChar = KeyDescriptor->Unicode;

        if (Key->UnicodeChar >= '1' && Key->UnicodeChar <= '9') {
          Key->UnicodeChar = Key->UnicodeChar - '1' + 1;
        }
      }  

      //
      // Alt + (Up, Down, Left, Right)
      //
      if (Key->ScanCode >= SCAN_UP && Key->ScanCode <= SCAN_PAGE_DOWN) {
        Key->ScanCode = SCAN_NULL;       
      }
    }

  }

  if (Key->UnicodeChar == 0 && Key->ScanCode == SCAN_NULL) {
    if (!UsbKeyboardDevice->IsSupportPartialKey) {
      return EFI_NOT_READY;
    }
  }
  //
  // Save Shift/Toggle state
  //
  UsbKeyboardDevice->KeyState.KeyShiftState  = EFI_SHIFT_STATE_VALID;
  UsbKeyboardDevice->KeyState.KeyToggleState = EFI_TOGGLE_STATE_VALID;
  if (UsbKeyboardDevice->LeftCtrlOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_LEFT_CONTROL_PRESSED;
  }
  if (UsbKeyboardDevice->RightCtrlOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_RIGHT_CONTROL_PRESSED;
  }
  if (UsbKeyboardDevice->LeftAltOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_LEFT_ALT_PRESSED;
  }
  if (UsbKeyboardDevice->RightAltOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_RIGHT_ALT_PRESSED;
  }
  if (UsbKeyboardDevice->LeftShiftOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_LEFT_SHIFT_PRESSED;
  }
  if (UsbKeyboardDevice->RightShiftOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_RIGHT_SHIFT_PRESSED;
  }
  if (UsbKeyboardDevice->LeftLogoOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_LEFT_LOGO_PRESSED;
  }
  if (UsbKeyboardDevice->RightLogoOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_RIGHT_LOGO_PRESSED;
  }
  if (UsbKeyboardDevice->MenuKeyOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_MENU_KEY_PRESSED;
  }
  if (UsbKeyboardDevice->SysReqOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_SYS_REQ_PRESSED;
  }  

  if (UsbKeyboardDevice->ScrollOn == 1) {
    UsbKeyboardDevice->KeyState.KeyToggleState |= EFI_SCROLL_LOCK_ACTIVE;
  }
  if (UsbKeyboardDevice->NumLockOn == 1) {
    UsbKeyboardDevice->KeyState.KeyToggleState |= EFI_NUM_LOCK_ACTIVE;
  }
  if (UsbKeyboardDevice->CapsOn == 1) {
    UsbKeyboardDevice->KeyState.KeyToggleState |= EFI_CAPS_LOCK_ACTIVE;
  }

  if (UsbKeyboardDevice->IsSupportPartialKey) {
    UsbKeyboardDevice->KeyState.KeyToggleState |= EFI_KEY_STATE_EXPOSED;
  }
  return EFI_SUCCESS;
}

/**

  Resets USB Keyboard Buffer.
  
  @param  KeyboardBuffer        Points to the USB Keyboard Buffer.
  
  @retval EFI_SUCCESS           Success

**/  
EFI_STATUS
InitUsbKeyBuffer (
  IN OUT  USB_KB_BUFFER   *KeyboardBuffer
  )
{
  ZeroMem (KeyboardBuffer, sizeof (USB_KB_BUFFER));

  KeyboardBuffer->bHead = KeyboardBuffer->bTail;

  return EFI_SUCCESS;
}

/**

  Check whether USB Keyboard buffer is empty.
  
  @param  KeyboardBuffer        USB Keyboard Buffer.
  
**/
BOOLEAN
IsUsbKeyboardBufferEmpty (
  IN  USB_KB_BUFFER   *KeyboardBuffer
  )
{
  //
  // meet FIFO empty condition
  //
  return (BOOLEAN) (KeyboardBuffer->bHead == KeyboardBuffer->bTail);
}

/**

  Check whether USB Keyboard buffer is full.
  
  @param  KeyboardBuffer        USB Keyboard Buffer.

**/
BOOLEAN
IsUsbKeyboardBufferFull (
  IN  USB_KB_BUFFER   *KeyboardBuffer
  )
{
  return (BOOLEAN)(((KeyboardBuffer->bTail + 1) % (MAX_KEY_ALLOWED + 1)) == 
                                                        KeyboardBuffer->bHead);
}

/**

  Sets monitor keys for SCU (SETUP_HOT_KEY / BOOT_MANAGER_HOT_KEY...)
  
  @param  KeyboardBuffer        Points to the USB Keyboard Buffer.
  @param  Key                   Key code
  
**/
VOID
SetMonitorKey (
  IN      USB_KB_DEV    *UsbKeyboardDevice,
  IN      UINT8         Key
  )
{
  UINT8  *FilterKeyList;
  UINT8  ModKey;
  UINTN  Index;
  UINTN  BitIndex;
  UINT16 KbcScanCode;
  //
  // Skip if no MonitorKey installed
  //
  if (EBDA(EBDA_MONITOR_KEY_TABLE) != 0x55 || Key >= 0xe0) return;
  //
  // Convert USB key code to KBC key code
  //
  KbcScanCode = ConvertKbcScanCode (UsbKeyboardDevice, Key);
  if ((KbcScanCode & 0xff) == 0xe0) {
    //
    // To synchronize with CSM16, covert the scan code of "Enter KP" (0xe0, 0x1c) or "/ KP" (0xe0, 0x35)
    // to invalid code (0xe0) and needn't covert other double code to single code.
    //
    KbcScanCode >>= 8;
    if ((UINT8) KbcScanCode == 0x1c || (UINT8) KbcScanCode == 0x35) {
      KbcScanCode = 0xe0;
    }
  }
  //
  // Get FilterKeyList pointer
  //
  FilterKeyList = (UINT8*)(UINTN)((*(UINT16*)&(EBDA(EBDA_MONITOR_KEY_TABLE + 3)) << 4) + *(UINT16*)&(EBDA(EBDA_MONITOR_KEY_TABLE + 1)));
  for (Index = 0, BitIndex = 0; FilterKeyList[Index] != 0; Index += 3, BitIndex ++) {
    if (FilterKeyList[Index] == (UINT8)KbcScanCode) {
      //
      // Also checks Shift/Alt/Ctrl keys
      //
      if ((ModKey = FilterKeyList[Index + 1]) != 0) {
        if (((ModKey & 0x01) && !UsbKeyboardDevice->ShiftOn) ||
            ((ModKey & 0x02) && !UsbKeyboardDevice->AltOn) ||
            ((ModKey & 0x04) && !UsbKeyboardDevice->CtrlOn)) {
          continue;
        }
      }
      *(UINT32*)&(EBDA(EBDA_MONITOR_KEY_TABLE + 5)) |= (1 << BitIndex);
      break;
    }
  }
}

/**

  Sync the control key status to CSM environment

  @param  KeyboardBuffer        Points to the USB Keyboard Buffer.
  @param  Key                   Key code
  @param  Down                  Special key

  @retval EFI_SUCCESS           Success  

**/
EFI_STATUS
SyncEfiKeyToCsmkey (
  IN      USB_KB_DEV    *UsbKeyboardDevice,
  IN      UINT8         KeyCode,
  IN      UINT8         Down
  )
{
  QUALIFIER_STAT *QualifierStat;
  
  QualifierStat = (QUALIFIER_STAT*)(UINTN)(BDA_QUALIFIER_STAT);
  
  switch (KeyCode) {
  case 0xe1:
    if (Down) {
      QualifierStat->LeftShift = 1;
    } else {
      QualifierStat->LeftShift = 0;
    }
    break;
  case 0xe5:
    if (Down) {
      QualifierStat->RightShift = 1;
    } else {
      QualifierStat->RightShift = 0;
    }
    break;
  
  case 0xe0:
  case 0xe4:
    if (Down) {
      QualifierStat->Ctrl = 1;
    } else {
      QualifierStat->Ctrl = 0;
    }
    break;

  case 0xe2:
  case 0xe6:
    if (Down) {
      QualifierStat->Alt = 1;
    } else {
      QualifierStat->Alt = 0;
    }
    break;
    
  default:
        break;
  }
      
  return  EFI_SUCCESS;
}

/**

  Sync the control key status to EFI environment

  @param  KeyboardBuffer        Points to the USB Keyboard Buffer.
  @param  Key                   Key code
  @param  Down                  Special key

  @retval EFI_SUCCESS           Success  

**/
EFI_STATUS
SyncCsmKeyToEfikey (
  IN      USB_KB_DEV    *UsbKeyboardDevice,
  IN      UINT8         KeyCode,
  IN      UINT8         Down
  )
{
  USB_KB_BUFFER         *KeyboardBuffer;
  USB_KEY               UsbKey;
  
  KeyboardBuffer = &UsbKeyboardDevice->KeyboardBuffer;
  //
  // if keyboard buffer is full, throw the
  // first key out of the keyboard buffer.
  //
  if (IsUsbKeyboardBufferFull (KeyboardBuffer)) {
    RemoveKeyCode (&UsbKeyboardDevice->KeyboardBuffer, &UsbKey);
  }
  KeyboardBuffer->buffer[KeyboardBuffer->bTail].KeyCode = KeyCode;
  KeyboardBuffer->buffer[KeyboardBuffer->bTail].Down    = Down;

  //
  // adjust the tail pointer of the FIFO keyboard buffer.
  //
  KeyboardBuffer->bTail = (UINT8) ((KeyboardBuffer->bTail + 1) % (MAX_KEY_ALLOWED + 1));
  return  EFI_SUCCESS;
}

/**

  Inserts a key code into keyboard buffer.
  
  @param  KeyboardBuffer        Points to the USB Keyboard Buffer.
  @param  Key                   Key code
  @param  Down                  Special key

  @retval EFI_SUCCESS           Success

**/
EFI_STATUS
InsertKeyCode (
  IN      USB_KB_DEV    *UsbKeyboardDevice,
  IN      UINT8         Key,
  IN      UINT8         Down
  )
{
  USB_KB_BUFFER         *KeyboardBuffer;
  USB_KEY               UsbKey;
  UINT8                 KbcKeyCode[4];
  EFI_USB_CORE_PROTOCOL *UsbCore;
  UINTN                 Mode;
  EFI_STATUS            IsCsmEnabled;
  
  if (!USBKBD_VALID_KEYCODE (Key)) return EFI_NOT_FOUND;
  UsbCore = UsbKeyboardDevice->UsbCore;
  UsbCore->GetMode(&Mode);
  IsCsmEnabled = UsbCore->IsCsmEnabled();
  //
  // Ignore LED update if it is updating by Int9
  //
  if (IsCsmEnabled == EFI_SUCCESS && PS2_LED_UPDATING && (Key == 0x53 || Key == 0x39 || Key == 0x47)) {
    return EFI_SUCCESS;
  }
  if ((IsCsmEnabled != EFI_SUCCESS) || (Mode != USB_CORE_RUNTIME_MODE && EBDA(EBDA_OPROM_PROCESSING_FLAG) == 0 && !PAUSE_IN_PROGRESS && (UsbCore->IsKbcExist() == EFI_NOT_FOUND || Key != 0x48))) {
    KeyboardBuffer = &UsbKeyboardDevice->KeyboardBuffer;
    //
    // if keyboard buffer is full, throw the
    // first key out of the keyboard buffer.
    //
    if (IsUsbKeyboardBufferFull (KeyboardBuffer)) {
      RemoveKeyCode (&UsbKeyboardDevice->KeyboardBuffer, &UsbKey);
    }

    KeyboardBuffer->buffer[KeyboardBuffer->bTail].KeyCode = Key;
    KeyboardBuffer->buffer[KeyboardBuffer->bTail].Down    = Down;

    //
    // adjust the tail pointer of the FIFO keyboard buffer.
    //
    KeyboardBuffer->bTail = (UINT8) ((KeyboardBuffer->bTail + 1) % (MAX_KEY_ALLOWED + 1));
    //
    // Sets monitor keys for SCU (SETUP_HOT_KEY / BOOT_MANAGER_HOT_KEY...)
    //
    if (UsbCore->IsCsmEnabled() == EFI_SUCCESS) {
      SetMonitorKey(UsbKeyboardDevice, Key);
      if (Key >= 0xe0) {
        SyncEfiKeyToCsmkey (UsbKeyboardDevice, Key, Down);
      }
    }
  } else {
    //
    // Insert KBC code for legacy mode
    //
    if (Key >= 0xe0) {
      if (Mode != USB_CORE_RUNTIME_MODE) {
        SyncCsmKeyToEfikey (UsbKeyboardDevice, Key, Down);
      }
      KbcKeyCode[0] = KbcModCodeTable[Key - 0xe0];
    } else if (Key >= 4 && Key < (sizeof (KbcCodeTable) / 4) + 4) {
      KbcKeyCode[0] = KbcCodeTable[Key - 4][0];
    } else if (Key >= 0x87 && Key < 0x8C) {
      KbcKeyCode[0] = KbcCodeTableJp109[Key - 0x87];
    }
    if (!(KbcKeyCode[0] & 0x80)) {
      //
      // Single code key
      //
      if (!Down) KbcKeyCode[0] |= 0x80;
      UsbCore->InsertKbcKeyCode(USB_CORE_KEYBOARD_CODE, KbcKeyCode, 1);
    } else if (KbcKeyCode[0] == 0x80) {
      //
      // PrntScrn
      //
      *(UINT32*)KbcKeyCode = (Down) ? 0x37E02AE0 : 0xAAE0B7E0;
      UsbCore->InsertKbcKeyCode(USB_CORE_KEYBOARD_CODE, KbcKeyCode, 4);
    } else if (KbcKeyCode[0] == 0x81) {
      //
      // Pause
      //
      *(UINT32*)KbcKeyCode = (Down) ? 0x00451DE1 : 0x00C59DE1;
      UsbCore->InsertKbcKeyCode(USB_CORE_KEYBOARD_CODE, KbcKeyCode, 3);
    } else {
      //
      // Double code key
      //
      if (Down) KbcKeyCode[0] &= ~0x80;
      KbcKeyCode[1] = KbcKeyCode[0];
      KbcKeyCode[0] = 0xE0;
      UsbCore->InsertKbcKeyCode(USB_CORE_KEYBOARD_CODE, KbcKeyCode, 2);
    }
  }
  return EFI_SUCCESS;
}

/**

  Pops a key code off from keyboard buffer.
  
  @param  KeyboardBuffer        Points to the USB Keyboard Buffer.
  @param  UsbKey                Points to the buffer that contains a usb key code.

  @retval EFI_SUCCESS           Success
  @retval EFI_DEVICE_ERROR      Hardware Error

**/  
EFI_STATUS
RemoveKeyCode (
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  OUT     USB_KEY       *UsbKey
  )
{
  if (IsUsbKeyboardBufferEmpty (KeyboardBuffer)) {
    return EFI_DEVICE_ERROR;
  }

  UsbKey->KeyCode = KeyboardBuffer->buffer[KeyboardBuffer->bHead].KeyCode;
  UsbKey->Down    = KeyboardBuffer->buffer[KeyboardBuffer->bHead].Down;

  //
  // adjust the head pointer of the FIFO keyboard buffer.
  //
  KeyboardBuffer->bHead = (UINT8) ((KeyboardBuffer->bHead + 1) % (MAX_KEY_ALLOWED + 1));

  return EFI_SUCCESS;
}

/**

  Sets USB Keyboard LED state.
  
  @param  UsbKeyboardDevice     The USB_KB_DEV instance.

  @retval EFI_SUCCESS           Success

**/
EFI_STATUS
SetKeyLED (
  IN  USB_KB_DEV    *UsbKeyboardDevice
  )
{
  LED_MAP        Led;
  QUALIFIER_STAT *QualifierStat;

  //
  // Set each field in Led map.
  //
  Led.NumLock    = (UINT8) UsbKeyboardDevice->NumLockOn;
  Led.CapsLock   = (UINT8) UsbKeyboardDevice->CapsOn;
  Led.ScrollLock = (UINT8) UsbKeyboardDevice->ScrollOn;
  Led.Resrvd     = 0;
  //
  // Make synchronize with BDA qualifier stat
  //
  if (UsbKeyboardDevice->UsbCore->IsCsmEnabled() == EFI_SUCCESS) {
    QualifierStat = (QUALIFIER_STAT*)(UINTN)(BDA_QUALIFIER_STAT);
    QualifierStat->ScrollLock = Led.ScrollLock;
    QualifierStat->NumLock    = Led.NumLock;
    QualifierStat->CapsLock   = Led.CapsLock;
  }
  //
  // Only lighten LED if supported
  //
  if (UsbKeyboardDevice->LedSupported) {
    //
    // call Set Report Request to lighten the LED.
    //
    UsbKeyboardDevice->UsbCore->UsbSetReportRequest (
                                  UsbKeyboardDevice->UsbIo,
                                  UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
                                  0,
                                  HID_OUTPUT_REPORT,
                                  1,
                                  (UINT8 *) &Led
                                  );
  }
  return EFI_SUCCESS;
}

/**

  Timer handler for Repeat Key timer.
  
  @param  Context               Points to the USB_KB_DEV instance.
  
**/  
VOID
EFIAPI
UsbKeyboardRepeatHandler (
  IN    UINTN        Event,
  IN    VOID         *Context
  )
{
  USB_KB_DEV  *UsbKeyboardDevice;

  UsbKeyboardDevice = (USB_KB_DEV *) Context;
  //
  // Do nothing when there is no repeat key.
  //
  if (UsbKeyboardDevice->RepeatKey != 0) {
    //
    // Inserts one Repeat key into keyboard buffer,
    //
    InsertKeyCode (
      UsbKeyboardDevice,
      UsbKeyboardDevice->RepeatKey,
      1
      );
    //
    // set repeate rate for repeat key generation.
    //
    UsbKeyboardDevice->UsbCore->InsertPeriodicTimer(
                         USB_CORE_ONCE_TIMER,
                         UsbKeyboardRepeatHandler,
                         UsbKeyboardDevice,
                         USBKBD_REPEAT_RATE,
                         &UsbKeyboardDevice->RepeatPollingHandle
                         );
  }
}

/**

  Timer handler for Delayed Recovery timer.
  
  @param  Context               Points to the USB_KB_DEV instance.
  
**/
VOID
EFIAPI
UsbKeyboardRecoveryHandler (
  IN    UINTN        Event,
  IN    VOID         *Context
  )
{

  USB_KB_DEV          *UsbKeyboardDevice;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINT8               PacketSize;

  UsbKeyboardDevice = (USB_KB_DEV *) Context;

  UsbIo             = UsbKeyboardDevice->UsbIo;

  PacketSize        = (UINT8) (UsbKeyboardDevice->IntEndpointDescriptor.MaxPacketSize);

  UsbIo->UsbAsyncInterruptTransfer (
           UsbIo,
           UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
           TRUE,
           UsbKeyboardDevice->IntEndpointDescriptor.Interval,
           PacketSize,
           KeyboardHandler,
           UsbKeyboardDevice
           );
}

/**

  Timer handler for Repeat Key timer.
  
  @param  Context               Points to the USB_KB_DEV instance.
  
**/
VOID
EFIAPI
UsbKeyboardCheckLEDHandler (
  IN    UINTN        ForceSet,
  IN    VOID         *Context
  )
{
  USB_KB_DEV     *UsbKeyboardDevice;
  QUALIFIER_STAT QualifierStat;
  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL   *ConSplitterEx;
  EFI_KEY_TOGGLE_STATE                KeyToggleState;
  EFI_STATUS                          Status;

  UsbKeyboardDevice = (USB_KB_DEV *) Context;
  KeyToggleState = UsbKeyboardDevice->IsSupportPartialKey ? EFI_TOGGLE_STATE_VALID | EFI_KEY_STATE_EXPOSED : EFI_TOGGLE_STATE_VALID;
  if (UsbKeyboardDevice->UsbCore->IsCsmEnabled() == EFI_SUCCESS) {
    //
    // Get Qualifier Stat in BDA
    //
    QualifierStat = *(QUALIFIER_STAT*)(UINTN)(BDA_QUALIFIER_STAT);
    //
    // Make it synchronization
    //
    if (ForceSet || 
        QualifierStat.ScrollLock != UsbKeyboardDevice->ScrollOn ||
        QualifierStat.NumLock != UsbKeyboardDevice->NumLockOn ||
        QualifierStat.CapsLock != UsbKeyboardDevice->CapsOn) {
      UsbKeyboardDevice->ScrollOn = QualifierStat.ScrollLock;
      UsbKeyboardDevice->NumLockOn = QualifierStat.NumLock;
      UsbKeyboardDevice->CapsOn = QualifierStat.CapsLock;
      SetKeyLED(UsbKeyboardDevice);
    }
  } else {
    if (UsbKeyboardDevice->ScrollOn == 1) {
      KeyToggleState |= EFI_SCROLL_LOCK_ACTIVE;
    }
    if (UsbKeyboardDevice->NumLockOn == 1) {
      KeyToggleState |= EFI_NUM_LOCK_ACTIVE;
    }
    if (UsbKeyboardDevice->CapsOn == 1) {
      KeyToggleState |= EFI_CAPS_LOCK_ACTIVE;
    }

    if (gST->ConsoleInHandle != NULL) {
      Status = gBS->HandleProtocol (
                      gST->ConsoleInHandle,
                      &gEfiSimpleTextInputExProtocolGuid,
                      (VOID **)&ConSplitterEx
                      );
      if (!EFI_ERROR (Status)) {
        //
        // Update all of ConInEx device State.
        //      
        Status = ConSplitterEx->SetState (ConSplitterEx, &KeyToggleState);
        if (Status == EFI_SUCCESS) return;
      }
    }
    //
    // Update USB Keyboard State
    //
    UsbKeyboardDevice->SimpleInputEx.SetState (&UsbKeyboardDevice->SimpleInputEx, &KeyToggleState);
  }
}

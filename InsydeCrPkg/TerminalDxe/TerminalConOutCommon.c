/** @file
  Implementation for common function.

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

#include "Terminal.h"

//
// This list is used to define the valid extend chars.
// It also provides a mapping from Unicode to PCANSI or
// ASCII. The ASCII mapping we just made up.
//
//
UNICODE_TO_CHAR  mUnicodeToPcAnsiOrAscii[] = {
    // EFI UNICODE,                     PcAnsi,   ASCii,    LineDraw Character
    BOXDRAW_HORIZONTAL,                 0xc4,     L'-',     0x71,
    BOXDRAW_VERTICAL,                   0xb3,     L'|',     0x78,
    BOXDRAW_DOWN_RIGHT,                 0xda,     L'/',     0x6c,
    BOXDRAW_DOWN_LEFT,                  0xbf,     L'\\',    0x6b,
    BOXDRAW_UP_RIGHT,                   0xc0,     L'\\',    0x6d,
    BOXDRAW_UP_LEFT,                    0xd9,     L'/',     0x6a,
    BOXDRAW_VERTICAL_RIGHT,             0xc3,     L'|',     0x74,
    BOXDRAW_VERTICAL_LEFT,              0xb4,     L'|',     0x75,
    BOXDRAW_DOWN_HORIZONTAL,            0xc2,     L'+',     0x77,
    BOXDRAW_UP_HORIZONTAL,              0xc1,     L'+',     0x76,
    BOXDRAW_VERTICAL_HORIZONTAL,        0xc5,     L'+',     0x6e,
    BOXDRAW_DOUBLE_HORIZONTAL,          0xcd,     L'-',     0x71,
    BOXDRAW_DOUBLE_VERTICAL,            0xba,     L'|',     0x78,
    BOXDRAW_DOWN_RIGHT_DOUBLE,          0xd5,     L'/',     0x6c,
    BOXDRAW_DOWN_DOUBLE_RIGHT,          0xd6,     L'/',     0x6c,
    BOXDRAW_DOUBLE_DOWN_RIGHT,          0xc9,     L'/',     0x6c,
    BOXDRAW_DOWN_LEFT_DOUBLE,           0xb8,     L'\\',    0x6b,
    BOXDRAW_DOWN_DOUBLE_LEFT,           0xb7,     L'\\',    0x6b,
    BOXDRAW_DOUBLE_DOWN_LEFT,           0xbb,     L'\\',    0x6b,
    BOXDRAW_UP_RIGHT_DOUBLE,            0xd4,     L'\\',    0x6d,
    BOXDRAW_UP_DOUBLE_RIGHT,            0xd3,     L'\\',    0x6d,
    BOXDRAW_DOUBLE_UP_RIGHT,            0xc8,     L'\\',    0x6d,
    BOXDRAW_UP_LEFT_DOUBLE,             0xbe,     L'/',     0x6a,
    BOXDRAW_UP_DOUBLE_LEFT,             0xbd,     L'/',     0x6a,
    BOXDRAW_DOUBLE_UP_LEFT,             0xbc,     L'/',     0x6a,
    BOXDRAW_VERTICAL_RIGHT_DOUBLE,      0xc6,     L'|',     0x74,
    BOXDRAW_VERTICAL_DOUBLE_RIGHT,      0xc7,     L'|',     0x74,
    BOXDRAW_DOUBLE_VERTICAL_RIGHT,      0xcc,     L'|',     0x74,
    BOXDRAW_VERTICAL_LEFT_DOUBLE,       0xb5,     L'|',     0x75,
    BOXDRAW_VERTICAL_DOUBLE_LEFT,       0xb6,     L'|',     0x75,
    BOXDRAW_DOUBLE_VERTICAL_LEFT,       0xb9,     L'|',     0x75,
    BOXDRAW_DOWN_HORIZONTAL_DOUBLE,     0xd1,     L'+',     0x77,
    BOXDRAW_DOWN_DOUBLE_HORIZONTAL,     0xd2,     L'+',     0x77,
    BOXDRAW_DOUBLE_DOWN_HORIZONTAL,     0xcb,     L'+',     0x77,
    BOXDRAW_UP_HORIZONTAL_DOUBLE,       0xcf,     L'+',     0x76,
    BOXDRAW_UP_DOUBLE_HORIZONTAL,       0xd0,     L'+',     0x76,
    BOXDRAW_DOUBLE_UP_HORIZONTAL,       0xca,     L'+',     0x76,
    BOXDRAW_VERTICAL_HORIZONTAL_DOUBLE, 0xd8,     L'+',     0x6e,
    BOXDRAW_VERTICAL_DOUBLE_HORIZONTAL, 0xd7,     L'+',     0x6e,
    BOXDRAW_DOUBLE_VERTICAL_HORIZONTAL, 0xce,     L'+',     0x6e,

    BLOCKELEMENT_FULL_BLOCK,            0xdb,     L'*',     0x61,
    BLOCKELEMENT_LIGHT_SHADE,           0xb0,     L'+',     0x61,

    GEOMETRICSHAPE_UP_TRIANGLE,         0x5E,     L'^',     0x5e,
    GEOMETRICSHAPE_RIGHT_TRIANGLE,      0x3E,     L'>',     0x3e,
    GEOMETRICSHAPE_DOWN_TRIANGLE,       0x76,     L'v',     0x00,     // Not in LineDraw Table
    GEOMETRICSHAPE_LEFT_TRIANGLE,       0x3C,     L'<',     0x3c,

    ARROW_LEFT,                         0x3c,     L'<',     0x3c,
    ARROW_UP,                           0x5E,     L'^',     0x5e,
    ARROW_RIGHT,                        0x3E,     L'>',     0x3e,
    ARROW_DOWN,                         0x76,     L'v',     0x00,     // Not in LineDraw Table
    0x0000, 0x00
};

//
// define the ESC sequence code map EFI scan code.
// It can map to scan code or a function.
// Map to function has not implement.
//
ESC_SEQUENCE_CODE  gEscSequenceCode[] = {
  // Reset function
  TP_ALL_TYPE, 0, ESC_CODE_FUNC,      FUNC_RESET,     L"R\x1Br\x1BR",
  TP_ALL_TYPE, 0, ESC_CODE_FUNC,      FUNC_VIDEO,     L"VID",
  TP_ALL_TYPE, 0, ESC_CODE_FUNC,      FUNC_SENSE,     L"SEN",
  TP_ALL_TYPE, 0, ESC_CODE_FUNC,      FUNC_MANUAL_REFRESH,   L"r",
  TP_ALL_TYPE, 0, ESC_CODE_FUNC,      FUNC_MANUAL_REFRESH,   L"R",
  TP_ALL_TYPE, 0, ESC_CODE_FUNC,      FUNC_AUTO_REFRESH,     L"[0n",

  TP_ALL_TYPE, 0, ESC_CODE_FUNC,      FUNC_AUTO_REFRESH,     L"[c",

  TP_ALL_TYPE, 0, ESC_CODE_FUNC,      FUNC_CHARSET,          L"CS",
  TP_PCANSI,   0, ESC_CODE_FUNC,      FUNC_NONVT100_ALTKEY,  L"^A",
  TP_ALL_TYPE, 0, ESC_CODE_FUNC,      FUNC_CR_SRV_MANAGER,   L"SV",
  //
  // 1 byte ESC sequence code
  //

  // VT100,PC-ANSI  Function key F1 ~ F12
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F1,        L"1",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F2,        L"2",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F3,        L"3",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F4,        L"4",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F5,        L"5",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F6,        L"6",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F7,        L"7",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F8,        L"8",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F9,        L"9",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F10,       L"0",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F11,       L"!",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F12,       L"@",

  // VT100,PC-ANSI keypad key
  TP_ANSI_100, 0, ESC_CODE_EXTENTION, SCAN_HOME,      L"h",
  TP_ANSI_100, 0, ESC_CODE_EXTENTION, SCAN_END,       L"k",
  TP_ANSI_100, 0, ESC_CODE_EXTENTION, SCAN_INSERT,    L"+",
  TP_ANSI_100, 0, ESC_CODE_EXTENTION, SCAN_DELETE,    L"-",
  TP_ANSI_100, 0, ESC_CODE_EXTENTION, SCAN_PAGE_UP,   L"?",
  TP_ANSI_100, 0, ESC_CODE_EXTENTION, SCAN_PAGE_DOWN, L"/",

  // Ctrl + key  , These keys is conflict with ASCII. So, define a sequence code for it.
  TP_ALL_TYPE, 0, ESC_CODE_CONTROL,   0x32,            L"\x0d",   // Ctrl + M
  TP_ALL_TYPE, 0, ESC_CODE_CONTROL,   0x23,            L"\x08",   // Ctrl + H
  TP_ALL_TYPE, 0, ESC_CODE_CONTROL,   0x17,            L"\x09",   // Ctrl + I
  TP_ALL_TYPE, 0, ESC_CODE_CONTROL,   0x24,            L"\x0a",   // Ctrl + J

  //
  // 2 byte ESC sequence code
  //

  // Arrow key
  TP_ALL_TYPE, 0, ESC_CODE_EXTENTION, SCAN_UP,        L"[A",
  TP_ALL_TYPE, 0, ESC_CODE_EXTENTION, SCAN_DOWN,      L"[B",
  TP_ALL_TYPE, 0, ESC_CODE_EXTENTION, SCAN_RIGHT,     L"[C",
  TP_ALL_TYPE, 0, ESC_CODE_EXTENTION, SCAN_LEFT,      L"[D",

  // VT100,PC-ANSI  Function key F1 ~ F12
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F1,        L"OP",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F2,        L"OQ",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F3,        L"OR",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F4,        L"OS",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F5,        L"OT",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F6,        L"OU",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F7,        L"OV",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F8,        L"OW",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F9,        L"OX",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F10,       L"OY",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F11,       L"OZ",
  TP_ANSI_100, 0, ESC_CODE_SCANCODE,  SCAN_F12,       L"OA",

  // PC-ANSI  Function Key F1 ~ F12 (keys for SCO and Unix ANSI emulators)
  TP_PCANSI,   0, ESC_CODE_SCANCODE,  SCAN_F1,        L"[M",
  TP_PCANSI,   0, ESC_CODE_SCANCODE,  SCAN_F2,        L"[N",
  TP_PCANSI,   0, ESC_CODE_SCANCODE,  SCAN_F3,        L"[O",
  TP_PCANSI,   0, ESC_CODE_SCANCODE,  SCAN_F4,        L"[P",
  TP_PCANSI,   0, ESC_CODE_SCANCODE,  SCAN_F5,        L"[Q",
  TP_PCANSI,   0, ESC_CODE_SCANCODE,  SCAN_F6,        L"[R",
  TP_PCANSI,   0, ESC_CODE_SCANCODE,  SCAN_F7,        L"[S",
  TP_PCANSI,   0, ESC_CODE_SCANCODE,  SCAN_F8,        L"[T",
  TP_PCANSI,   0, ESC_CODE_SCANCODE,  SCAN_F9,        L"[U",
  TP_PCANSI,   0, ESC_CODE_SCANCODE,  SCAN_F10,       L"[V",
  TP_PCANSI,   0, ESC_CODE_SCANCODE,  SCAN_F11,       L"[W",
  TP_PCANSI,   0, ESC_CODE_SCANCODE,  SCAN_F12,       L"[X",

  TP_PCANSI,   0, ESC_CODE_EXTENTION, SCAN_HOME,      L"[H",
  TP_PCANSI,   0, ESC_CODE_EXTENTION, SCAN_END,       L"[K",
  TP_PCANSI,   0, ESC_CODE_EXTENTION, SCAN_INSERT,    L"[2",
  TP_PCANSI,   0, ESC_CODE_EXTENTION, SCAN_DELETE,    L"[3",
  TP_PCANSI,   0, ESC_CODE_EXTENTION, SCAN_PAGE_UP,   L"[5",
  TP_PCANSI,   0, ESC_CODE_EXTENTION, SCAN_PAGE_DOWN, L"[6",

  //
  // 3 byte ESC sequence code
  //

  // Shit+TAB
  TP_ALL_TYPE, 0, ESC_CODE_EXTENTION, SCAN_HOME,      L"[0Z",
  // Keypad home end ....
  TP_ALL_TYPE, 0, ESC_CODE_EXTENTION, SCAN_HOME,      L"[1~",
  TP_ALL_TYPE, 0, ESC_CODE_EXTENTION, SCAN_END,       L"[4~",
  TP_ALL_TYPE, 0, ESC_CODE_EXTENTION, SCAN_INSERT,    L"[2~",
  TP_ALL_TYPE, 0, ESC_CODE_EXTENTION, SCAN_DELETE,    L"[3~",
  TP_ALL_TYPE, 0, ESC_CODE_EXTENTION, SCAN_PAGE_UP,   L"[5~",
  TP_ALL_TYPE, 0, ESC_CODE_EXTENTION, SCAN_PAGE_DOWN, L"[6~",

  //
  // 4 byte ESC sequence code
  //

  // VT100+ Function key  F1 ~ F12
  TP_100P_UTF8,0, ESC_CODE_SCANCODE,  SCAN_F1,        L"[11~",
  TP_100P_UTF8,0, ESC_CODE_SCANCODE,  SCAN_F2,        L"[12~",
  TP_100P_UTF8,0, ESC_CODE_SCANCODE,  SCAN_F3,        L"[13~",
  TP_100P_UTF8,0, ESC_CODE_SCANCODE,  SCAN_F4,        L"[14~",
  TP_100P_UTF8,0, ESC_CODE_SCANCODE,  SCAN_F5,        L"[15~",
  TP_100P_UTF8,0, ESC_CODE_SCANCODE,  SCAN_F6,        L"[17~",
  TP_100P_UTF8,0, ESC_CODE_SCANCODE,  SCAN_F7,        L"[18~",
  TP_100P_UTF8,0, ESC_CODE_SCANCODE,  SCAN_F8,        L"[19~",
  TP_100P_UTF8,0, ESC_CODE_SCANCODE,  SCAN_F9,        L"[20~",
  TP_100P_UTF8,0, ESC_CODE_SCANCODE,  SCAN_F10,       L"[21~",
  TP_100P_UTF8,0, ESC_CODE_SCANCODE,  SCAN_F11,       L"[23~",
  TP_100P_UTF8,0, ESC_CODE_SCANCODE,  SCAN_F12,       L"[24~",
             0,0,                 0,         0,          NULL
};
UINTN gEscSequenceCodeSize = sizeof(gEscSequenceCode);

EFI_TO_KB_SCANCODE_MAP gEfiToKbScanCodeMap[] = {
  SCAN_UP,          0x48,
  SCAN_DOWN,        0x50,
  SCAN_RIGHT,       0x4D,
  SCAN_LEFT,        0x4B,
  SCAN_HOME,        0x47,
  SCAN_END,         0x4F,
  SCAN_INSERT,      0x52,
  SCAN_DELETE,      0x53,
  SCAN_PAGE_UP,     0x49,
  SCAN_PAGE_DOWN,   0x51,
  SCAN_F1,          0x3B,
  SCAN_F2,          0x3C,
  SCAN_F3,          0x3D,
  SCAN_F4,          0x3E,
  SCAN_F5,          0x3F,
  SCAN_F6,          0x40,
  SCAN_F7,          0x41,
  SCAN_F8,          0x42,
  SCAN_F9,          0x43,
  SCAN_F10,         0x44,
  SCAN_F11,         0x57,
  SCAN_F12,         0x58,
  SCAN_ESC,         0x01,
  0,                0x00
};

UINT16 gUniCodeToKbScanCodeMap[][3] = {
  // Unicode                 // KbScan
  {'0',    '0',       0x0B,},
  {'1',    '1',       0x02,},
  {'2',    '2',       0x03,},
  {'3',    '3',       0x04,},
  {'4',    '4',       0x05,},
  {'5',    '5',       0x06,},
  {'6',    '6',       0x07,},
  {'7',    '7',       0x08,},
  {'8',    '8',       0x09,},
  {'9',    '9',       0x0A,},
  {'a',    'A',       0x1E,},
  {'b',    'B',       0x30,},
  {'c',    'C',       0x2E,},
  {'d',    'D',       0x20,},
  {'e',    'E',       0x12,},
  {'f',    'F',       0x21,},
  {'g',    'G',       0x22,},
  {'h',    'H',       0x23,},
  {'i',    'I',       0x17,},
  {'j',    'J',       0x24,},
  {'k',    'K',       0x25,},
  {'l',    'L',       0x26,},
  {'m',    'M',       0x32,},
  {'n',    'N',       0x31,},
  {'o',    'O',       0x18,},
  {'p',    'P',       0x19,},
  {'q',    'Q',       0x10,},
  {'r',    'R',       0x13,},
  {'s',    'S',       0x1F,},
  {'t',    'T',       0x14,},
  {'u',    'U',       0x16,},
  {'v',    'V',       0x2F,},
  {'w',    'W',       0x11,},
  {'x',    'X',       0x2D,},
  {'y',    'Y',       0x15,},
  {'z',    'Z',       0x2C,},
  {0}
};

CR_SPECIAL_COMMAND gCrSpecialCommand[] = {
//{L"\x1f",        TYPE_FUNCTION_ID,     FUNC_RESET},         // Press Ctrl+Shift+ '-'  do a System Reset
  {      0,                      0,              0}
};
UINTN gCrSpecialCommandSize = sizeof(gCrSpecialCommand);

BAUDRATE_DATA_LIMIT_TABLE     gBaudRateDataAmountTable[] = {
                                1200,   50,
                                2400,   50,
                                4800,   50,
                                9600,   100,
                                19200,  100,
                                38400,  250,
                                57600,  500,
                                115200, 1000,

                                0,      0
                                };

CHAR16 gSetModeString[]            = { ESC, '[', '8', ';', '0', '0', '0', ';', '0', '0', '0', 't', 0 };
CHAR16 gSetAttributeString[]       = { ESC, '[', '0', 'm', ESC, '[', '4', '0', 'm', ESC, '[', '4', '0', 'm', 0 };
CHAR16 gClearScreenString[]        = { ESC, '[', '2', 'J', 0 };
CHAR16 gSetCursorPositionString[]  = { ESC, '[', '0', '0', '0', ';', '0', '0', '0', 'H', 0 };
CHAR16 gSetCharSetString[]         = { ESC, '(', 'B', 0};
CHAR16 gEraseLine[]                = { ESC, '[', '2', 'K', 0 };
CHAR16 gResetDeviceString[]        = { ESC, 'c', 0};
CHAR16 gDiableAutoWrapString[]     = { ESC, '[', '?', '7', 'l', 0};
CHAR16 gGetTermStatusString[]      = { ESC, '[', '5', 'n', 0};
CHAR16 gSetCursorToLastRowStr[]    = { ESC, '[', '8', '0', ';', '0', '0', 'H', 0 };
CHAR16 gScrollDownStr[]            = { ESC, 'D', 0 };

extern CR_POLICY_VARIABLE  *mCrPolicyVar;
extern UINT8               mTerminalMode;
extern BOOLEAN             gEscProcess = FALSE;
//
// Body of the ConOut functions
//
/**
  Detects if a Unicode char is for Box Drawing text graphics.

  @param Graphic               Unicode char to test.
  @param PcAnsi                Optional pointer to return PCANSI equivalent of Graphic.
  @param Ascii                   Optional pointer to return ASCII equivalent of Graphic.
  @param LineDrawChar      Optional pointer to return LineDraw Character Set equivalent of Graphic.
                                       if value is 0x00 means not matched character

  @retval TRUE                  if Graphic is a supported Unicode Box Drawing character.

**/
BOOLEAN
TerminalIsValidTextGraphics (
  IN  CHAR16  Graphic,
  OUT CHAR8   *PcAnsi,      OPTIONAL
  OUT CHAR8   *Ascii,       OPTIONAL
  OUT CHAR8   *LineDrawChar OPTIONAL
  )
{
  UNICODE_TO_CHAR  *Table;

  if ((((Graphic & 0xff00) != 0x2500) && ((Graphic & 0xff00) != 0x2100))) {
    //
    // Unicode drawing code charts are all in the 0x25xx range,
    //  arrows are 0x21xx
    //
    return FALSE;
  }

  for (Table = mUnicodeToPcAnsiOrAscii; Table->Unicode != 0x0000; Table++) {
    if (Graphic == Table->Unicode) {
      if (PcAnsi != NULL) {
        *PcAnsi = Table->PcAnsi;
      }

      if (Ascii != NULL) {
        *Ascii = Table->Ascii;
      }

      if (LineDrawChar != NULL) {
        *LineDrawChar = Table->LineDrawChar;
      }

      return TRUE;
    }
  }

  return FALSE;
}


/**
  Confirm input ascii is valid for terminal

  @param Ascii              Unicode char to test.

  @retval TRUE              Input ascii is valid.

**/
BOOLEAN
TerminalIsValidAscii (
  IN  CHAR16  Ascii
  )
{
  //
  // valid ascii code lies in the extent of 0x20 ~ 0x7f
  //
  if ((Ascii >= 0x20) && (Ascii <= 0x7f)) {
    return TRUE;
  }

  return FALSE;
}


/**
  Confirm input control character is valid for terminal

  @param CharC             Control character.

  @retval TRUE              Input character is valid.

**/
BOOLEAN
TerminalIsValidEfiCntlChar (
  IN  CHAR16  CharC
  )
{
  //
  // only support four control characters.
  //
  if (CharC == CHAR_NULL ||
      CharC == CHAR_BACKSPACE ||
      CharC == CHAR_LINEFEED ||
      CharC == CHAR_CARRIAGE_RETURN ||
      CharC == CHAR_TAB
      ) {
    return TRUE;
  }

  return FALSE;
}

/**
  Output ESC sequence string without RefreshScreen event interrupt.

  @param This             Pointer to EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
  @param EscString      Pointer to output EscString buffer

  @retval EFI_SUCCESS                           The string is output successfully.
  @retval EFI_DEVICE_ERROR                  The serial port fails to send the string out.
  @retval EFI_WARN_UNKNOWN_GLYPH    Indicates that some of the characters in the Unicode
                                                           string could not be rendered and are skipped.
**/
EFI_STATUS
OutputEscSquenceString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  CHAR16                           *EscString
  )
{
  TERMINAL_DEV             *TerminalDevice;
  EFI_STATUS               Status;
  EFI_TEXT_STRING          TerminalOutputString;

  //
  //  get Terminal device data structure pointer.
  //
  TerminalDevice = TERMINAL_CON_OUT_DEV_FROM_THIS (This);
  if (mTerminalMode == ASYNC) {
    TerminalOutputString = &PrivateOutputString;
  } else {
    TerminalOutputString = This->OutputString;
  }

  //
  // Avoid send ESC command be interrupt
  //
  gEscProcess = TRUE;

  TerminalDevice->OutputEscChar = TRUE;
  Status                        = TerminalOutputString (This, EscString);
  TerminalDevice->OutputEscChar = FALSE;

  gEscProcess = FALSE;

  return Status;
}


/**

  Set the terminal to a specified display mode the same with screen mode.

  @param This          Indicates the calling context.
  @param ModeNumber    The text mode to set.

  @retval EFI_SUCCESS       The requested text mode is set.
  @retval EFI_DEVICE_ERROR  The requested text mode cannot be set
                            because of serial device error.
  @retval EFI_UNSUPPORTED   The text mode number is not valid.

**/
EFI_STATUS
AdjustTerminalResolution (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  UINTN                            ModeNumber
  )
{
  EFI_STATUS    Status;
  TERMINAL_DEV  *TerminalDevice;
  UINTN         Columns;
  UINTN         Rows;

  if (mCrPolicyVar->Feature.Bit.CRTerminalResize == FALSE) {
    return EFI_SUCCESS;
  }
  //
  //  get Terminal device data structure pointer.
  //
  TerminalDevice = TERMINAL_CON_OUT_DEV_FROM_THIS (This);

  Status = This->QueryMode (This, ModeNumber, &Columns, &Rows);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // control sequence to move the cursor
  //
  gSetModeString[MODE_ROW_OFFSET + 0]    = (CHAR16) ('0' + (Rows / 100));
  Rows %= 100;
  gSetModeString[MODE_ROW_OFFSET + 1]    = (CHAR16) ('0' + (Rows / 10));
  Rows %= 10;
  gSetModeString[MODE_ROW_OFFSET + 2]    = (CHAR16) ('0' + Rows);

  Columns;
  gSetModeString[MODE_COLUMN_OFFSET + 0] = (CHAR16) ('0' + (Columns / 100));
  Columns %= 100;
  gSetModeString[MODE_COLUMN_OFFSET + 1] = (CHAR16) ('0' + (Columns / 10));
  Columns %= 10;
  gSetModeString[MODE_COLUMN_OFFSET + 2] = (CHAR16) ('0' + Columns);

  return OutputEscSquenceString (This, gSetModeString);
}

/**
  Switch terminal character set.

  @param This             Pointer to EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
  @param CharSet        The expected character set to switch

  @retval EFI_SUCCESS            The requested character set is update.
  @retval EFI_DEVICE_ERROR   The requested character set cannot be set due to serial port error.
  @retval EFI_UNSUPPORTED    The character set requested is not defined by EFI spec.

**/
EFI_STATUS
Vt100PlusSwitchCharSet (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL      *This,
  IN  VT100_CHARACTER_SET                  CharSet
  )
{
  EFI_STATUS                        Status;
  TERMINAL_DEV                      *TerminalDevice;


  TerminalDevice = TERMINAL_CON_OUT_DEV_FROM_THIS (This);


  //
  // Normal ASCii table
  //
  Status = EFI_SUCCESS;
  if (CharSet == VT100_ASCII_CHAR_SET) {

    if (TerminalDevice->Vt100CurrentCharSet != VT100_ASCII_CHAR_SET) {
      TerminalDevice->Vt100CurrentCharSet = VT100_ASCII_CHAR_SET;
      gSetCharSetString[2] = L'B';

      Status = OutputEscSquenceString (
                 This,
                 gSetCharSetString
                 );
    }
  }

  //
  // Special drawing character set
  //
  else if (CharSet == VT100_GRAPH_CHAR_SET) {
    if (TerminalDevice->Vt100CurrentCharSet != VT100_GRAPH_CHAR_SET) {
      TerminalDevice->Vt100CurrentCharSet = VT100_GRAPH_CHAR_SET;
      gSetCharSetString[2] = L'0';
      Status = OutputEscSquenceString (
                 This,
                 gSetCharSetString
                 );
    }
  } else {
    Status = EFI_UNSUPPORTED;
  }

  return Status;
}

/**
  Is graphic character support in Line Draw char set.

  @param  LineDrawChar The graphic character in Line Draw Table.

  @retval TRUE         LineDrawChar support.
  @retval FALSE        LineDrawChar not support.

**/
BOOLEAN
InLineDrawTable (
  CHAR8  LineDrawChar
  )
{
  if (LineDrawChar == 0) {
    return FALSE;
  } else {
    return TRUE;
  }
}


/**
  Implements EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.Reset().

  If ExtendeVerification is TRUE, then perform dependent serial device reset,
  and set display mode to mode 0.
  If ExtendedVerification is FALSE, only set display mode to mode 0.

  @param  This                  Indicates the calling context.
  @param  ExtendedVerification  Indicates that the driver may perform a more
                                exhaustive verification operation of the device
                                during reset.

  @retval EFI_SUCCESS           The reset operation succeeds.
  @retval EFI_DEVICE_ERROR      The terminal is not functioning correctly or the serial port reset fails.

**/
EFI_STATUS
EFIAPI
CommonConOutReset (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  BOOLEAN                          ExtendedVerification
  )
{
  if (mTerminalMode == SYNC) {
    return SyncTerminalConOutReset (This,ExtendedVerification);
  } else {
    return AsyncTerminalConOutReset (This,ExtendedVerification);
  }
}


/**
  Implements EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString().

  The Unicode string will be converted to terminal expressible data stream
  and send to terminal via serial port.

  @param  This                    Indicates the calling context.
  @param  WString                 The Null-terminated Unicode string to be displayed
                                  on the terminal screen.

  @retval EFI_SUCCESS             The string is output successfully.
  @retval EFI_DEVICE_ERROR        The serial port fails to send the string out.
  @retval EFI_WARN_UNKNOWN_GLYPH  Indicates that some of the characters in the Unicode string could not
                                  be rendered and are skipped.
  @retval EFI_UNSUPPORTED         If current display mode is out of range.

**/
EFI_STATUS
EFIAPI
CommonConOutOutputString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  CHAR16                           *WString
  )
{
  if (mTerminalMode == SYNC) {
    return SyncTerminalConOutOutputString (This, WString);
  } else {
    return AsyncTerminalConOutOutputString (This, WString);
  }
}


/**
  Implements EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.TestString().

  If one of the characters in the *Wstring is
  neither valid Unicode drawing characters,
  not ASCII code, then this function will return
  EFI_UNSUPPORTED.

  @param  This              Indicates the calling context.
  @param  WString           The Null-terminated Unicode string to be tested.

  @retval EFI_SUCCESS       The terminal is capable of rendering the output string.
  @retval EFI_UNSUPPORTED   Some of the characters in the Unicode string cannot be rendered.

**/
EFI_STATUS
EFIAPI
CommonConOutTestString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  CHAR16                           *WString
  )
{
  if (mTerminalMode == SYNC) {
    return SyncTerminalConOutTestString (This, WString);
  } else {
    return AsyncTerminalConOutTestString (This, WString);
  }
}


/**
  Implements EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.QueryMode().

  It returns information for an available text mode
  that the terminal supports.

  @param This        Indicates the calling context.
  @param ModeNumber  The mode number to return information on.
  @param Columns     The returned columns of the requested mode.
  @param Rows        The returned rows of the requested mode.

  @retval EFI_SUCCESS       The requested mode information is returned.
  @retval EFI_UNSUPPORTED   The mode number is not valid.

**/
EFI_STATUS
EFIAPI
CommonConOutQueryMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  UINTN                            ModeNumber,
  OUT UINTN                            *Columns,
  OUT UINTN                            *Rows
  )
{
  if (mTerminalMode == SYNC) {
    return SyncTerminalConOutQueryMode (This, ModeNumber, Columns, Rows);
  } else {
    return AsyncTerminalConOutQueryMode (This, ModeNumber, Columns, Rows);
  }
}


/**
  Implements EFI_SIMPLE_TEXT_OUT.SetMode().

  Set the terminal to a specified display mode.
  In this driver, we only support mode 0.

  @param This          Indicates the calling context.
  @param ModeNumber    The text mode to set.

  @retval EFI_SUCCESS       The requested text mode is set.
  @retval EFI_DEVICE_ERROR  The requested text mode cannot be set
                            because of serial device error.
  @retval EFI_UNSUPPORTED   The text mode number is not valid.

**/
EFI_STATUS
EFIAPI
CommonConOutSetMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  UINTN                            ModeNumber
  )
{
  if (mTerminalMode == SYNC) {
    return SyncTerminalConOutSetMode (This, ModeNumber);
  } else {
    return AsyncTerminalConOutSetMode (This, ModeNumber);
  }
}


/**
  Implements EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.SetAttribute().

  @param This        Indicates the calling context.
  @param Attribute   The attribute to set. Only bit0..6 are valid, all other bits
                     are undefined and must be zero.

  @retval EFI_SUCCESS        The requested attribute is set.
  @retval EFI_DEVICE_ERROR   The requested attribute cannot be set due to serial port error.
  @retval EFI_UNSUPPORTED    The attribute requested is not defined by EFI spec.

**/
EFI_STATUS
EFIAPI
CommonConOutSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  UINTN                            Attribute
  )
{
  if (mTerminalMode == SYNC) {
    return SyncTerminalConOutSetAttribute (This, Attribute);
  } else {
    return AsyncTerminalConOutSetAttribute (This, Attribute);
  }
}


/**
  Implements EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.ClearScreen().
  It clears the ANSI terminal's display to the
  currently selected background color.

  @param This     Indicates the calling context.

  @retval EFI_SUCCESS       The operation completed successfully.
  @retval EFI_DEVICE_ERROR  The terminal screen cannot be cleared due to serial port error.
  @retval EFI_UNSUPPORTED   The terminal is not in a valid display mode.

**/
EFI_STATUS
EFIAPI
CommonConOutClearScreen (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This
  )
{
  if (mTerminalMode == SYNC) {
    return SyncTerminalConOutClearScreen (This);
  } else {
    return AsyncTerminalConOutClearScreen (This);
  }
}


/**
  Implements EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.SetCursorPosition().

  @param This      Indicates the calling context.
  @param Column    The row to set cursor to.
  @param Row       The column to set cursor to.

  @retval EFI_SUCCESS       The operation completed successfully.
  @retval EFI_DEVICE_ERROR  The request fails due to serial port error.
  @retval EFI_UNSUPPORTED   The terminal is not in a valid text mode, or the cursor position
                            is invalid for current mode.

**/
EFI_STATUS
EFIAPI
CommonConOutSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  UINTN                            Column,
  IN  UINTN                            Row
  )
{
  if (mTerminalMode == SYNC) {
    return SyncTerminalConOutSetCursorPosition (This, Column, Row);
  } else {
    return AsyncTerminalConOutSetCursorPosition (This, Column, Row);
  }
}


/**
  Implements SIMPLE_TEXT_OUTPUT.EnableCursor().

  In this driver, the cursor cannot be hidden.

  @param This      Indicates the calling context.
  @param Visible   If TRUE, the cursor is set to be visible,
                   If FALSE, the cursor is set to be invisible.

  @retval EFI_SUCCESS      The request is valid.
  @retval EFI_UNSUPPORTED  The terminal does not support cursor hidden.

**/
EFI_STATUS
EFIAPI
CommonConOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  BOOLEAN                          Visible
  )
{
  if (mTerminalMode == SYNC) {
    return SyncTerminalConOutEnableCursor (This, Visible);
  } else {
    return AsyncTerminalConOutEnableCursor (This, Visible);
  }
}


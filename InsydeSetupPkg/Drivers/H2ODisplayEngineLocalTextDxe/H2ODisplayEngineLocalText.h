/** @file
 Function definition for H2O local text display engine

;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/

#ifndef _H2O_DISPLAY_ENGINE_LOCAL_TEXT_H_
#define _H2O_DISPLAY_ENGINE_LOCAL_TEXT_H_

#include <Uefi.h>
#include <Protocol/DevicePath.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/H2OFormBrowser.h>
#include <Protocol/H2ODisplayEngine.h>
#include <Protocol/SetupMouse.h>
#include <Guid/H2ODisplayEngineType.h>
#include <Guid/BdsHii.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>
#include <Library/RectLib.h>
#include <Library/ConsoleLib.h>
#include <Library/LayoutLib.h>
#include <Library/H2ODisplayEngineLib.h>
#include <Include/Guid/MdeModuleHii.h>
#include <KernelSetupConfig.h>

#define CHAR_SPACE            0x0020
#define CHAR_ADD              0x002B
#define CHAR_SUB              0x002D

#define NARROW_ATTRIBUTE      0x7F

#define NARROW_TEXT_WIDTH     8
#define WIDE_TEXT_WIDTH       16
#define TEXT_HEIGHT           19

#define BUTTON_VALUE_NO             0
#define BUTTON_VALUE_YES            1
#define BUTTON_VALUE_CANCEL         2

//
// Display definitions
//
#define REF_OP_DELIMITER          GEOMETRICSHAPE_RIGHT_TRIANGLE

#define LEFT_ONEOF_DELIMITER      L'<'
#define RIGHT_ONEOF_DELIMITER     L'>'

#define LEFT_NUMERIC_DELIMITER    L'['
#define RIGHT_NUMERIC_DELIMITER   L']'

#define LEFT_CHECKBOX_DELIMITER   L'['
#define RIGHT_CHECKBOX_DELIMITER  L']'

#define CHECK_ON                  L'X'
#define CHECK_OFF                 L' '

#define TIME_SEPARATOR            L':'
#define DATE_SEPARATOR            L'/'

#define PROMPT_VALUE_SEPARATOR_WIDTH    1

#define HEX_NUMBER_PREFIX_STRING      L"0x"

typedef enum {
  LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT = 0,
  LTDE_STRING_ALIGNMENT_ACTION_FLUSH_RIGHT,
  LTDE_STRING_ALIGNMENT_ACTION_CENTERED,
  LTDE_STRING_ALIGNMENT_ACTION_MAX
} LTDE_STRING_ALIGNMENT_ACTION;

#define H2O_DISPLAY_ENGINE_SIGNATURE SIGNATURE_32 ('H', 'D', 'E', 'S')
typedef struct _H2O_DISPLAY_ENGINE_PRIVATE_DATA {
  UINT32                                       Signature;
  EFI_HANDLE                                   ImageHandle;
  H2O_DISPLAY_ENGINE_PROTOCOL                  DisplayEngine;
  H2O_FORM_BROWSER_PROTOCOL                    *FBProtocol;
  LIST_ENTRY                                   ConsoleDevListHead;

  UINT8                                        DEStatus;
  H2O_LAYOUT_INFO                              *Layout;
  LIST_ENTRY                                   PanelListHead;
} H2O_DISPLAY_ENGINE_PRIVATE_DATA;

#define H2O_DISPLAY_ENGINE_PRIVATE_DATA_FROM_PROTOCOL(a) CR (a, H2O_DISPLAY_ENGINE_PRIVATE_DATA, DisplayEngine, H2O_DISPLAY_ENGINE_SIGNATURE)

typedef enum {
  CONSOLE_DEVICE_STATUS_INIT = 0,
  CONSOLE_DEVICE_STATUS_READY
} CONSOLE_DEVICE_STATUS;

#define H2O_FORM_BROWSER_CONSOLE_DEV_NODE_SIGNATURE SIGNATURE_32 ('F', 'B', 'C', 'D')

typedef struct _H2O_FORM_BROWSER_CONSOLE_DEV_NODE {
  UINT32                                       Signature;
  H2O_FORM_BROWSER_CONSOLE_DEV                 *ConsoleDev;
  LIST_ENTRY                                   Link;
  EFI_HANDLE                                   Handle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL              *SimpleTextOut;
  EFI_GRAPHICS_OUTPUT_PROTOCOL                 *GraphicsOut;
  UINT8                                        ConDevStatus;
} H2O_FORM_BROWSER_CONSOLE_DEV_NODE;


#define H2O_FORM_BROWSER_CONSOLE_DEV_NODE_FROM_LINK(a) CR (a, H2O_FORM_BROWSER_CONSOLE_DEV_NODE, Link, H2O_FORM_BROWSER_CONSOLE_DEV_NODE_SIGNATURE)

typedef struct _H2O_DISPLAY_ENGINE_USER_INPUT_DATA {
  BOOLEAN                            IsKeyboard;
  EFI_KEY_DATA                       KeyData;
  UINT32                             CursorX;
  UINT32                             CursorY;
} H2O_DISPLAY_ENGINE_USER_INPUT_DATA;

typedef enum {
  DISPLAY_ENGINE_STATUS_AT_MENU,
  DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG
} DISPLAY_ENGINE_STATUS;

EFI_STATUS
CheckFBHotKey (
  IN  H2O_DISPLAY_ENGINE_USER_INPUT_DATA      *UserInputData,
  OUT HOT_KEY_INFO                            *SelectedHotKey
  );

EFI_STATUS
SendEvtByHotKey (
  IN HOT_KEY_INFO                             *HotKey
  );

EFI_STATUS
DEEventCallback (
  IN       H2O_DISPLAY_ENGINE_PROTOCOL        *This,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify
  );

EFI_STATUS
TransferToTextModePosition (
  IN  INT32                                  GopX,
  IN  INT32                                  GopY,
  OUT UINT32                                 *Column,
  OUT UINT32                                 *Row
  );

EFI_STATUS
DisplayLayout (
  VOID
  );

EFI_STATUS
DEReplaceFakeConOutDev (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA       *Private,
  IN EFI_HANDLE                            ControllerHandle
  );

EFI_STATUS
DERemoveFakeConOutDev (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA       *Private
  );

EFI_STATUS
DisplayString (
  IN UINT32                        StartX,
  IN UINT32                        StartY,
  IN CHAR16                        *String
  );

EFI_STATUS
DEExit (
  VOID
  );

EFI_STATUS
ProcessEvtInQuestionFunc (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData
  );

EFI_STATUS
CompareHiiValue (
  IN  EFI_HII_VALUE                        *Value1,
  IN  EFI_HII_VALUE                        *Value2,
  OUT INTN                                 *Result
  );

EFI_STATUS
CreateValueAsString (
  IN EFI_HII_VALUE  *TargetHiiValue,
  IN UINT16          BufferLen,
  IN UINT8          *Buffer
  );

EFI_STATUS
CreateValueAsUint64 (
  IN EFI_HII_VALUE  *TargetHiiValue,
  IN UINT64         ValueUint64
  );

EFI_STATUS
GetAllBorderLineField (
  OUT RECT                                       **BorderLineFieldList,
  OUT UINT32                                     *BorderLineFieldCount
  );

extern H2O_DISPLAY_ENGINE_PRIVATE_DATA        *mDEPrivate;
extern UINT32                                 mScreenMaxY;
extern UINT32                                 mScreenMaxX;

#endif


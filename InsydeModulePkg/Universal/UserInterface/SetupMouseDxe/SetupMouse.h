/** @file
 Common Utility Header for SetupMouse

;******************************************************************************
;* Copyright (c) 2012 - 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/

#ifndef _SETUP_MOUSE_H
#define _SETUP_MOUSE_H

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Uefi/UefiSpec.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HiiLib.h>
#include <Library/DebugLib.h>
#include <Library/RectLib.h>
#include <Library/BitBltLib.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/PciIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/AbsolutePointer.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/SetupMouse.h>
#include <Protocol/ConsoleControl.h>
#include <Protocol/HiiImage.h>

#include <Guid/MdeModuleHii.h>
#include <InternalFormRepresentation.h>


#define MOUSE_RESOLUTION        0x1400

#define TICKS_PER_MS            10000U
#define TICKS_PER_SECOND        10000000U

//
// Too small value (<= 10ms) will cause I2C bus master timer callback cannot be trigger.
// Please keep MOUSE_TIMER is larger than 10 ms
//
#define MOUSE_TIMER             (20 * TICKS_PER_MS)

#define SETUP_MOUSE_SIGNATURE   SIGNATURE_32('S','t','p','m')


#define STATE_BUFFER_SIZE       32

typedef struct _POINTER_PROTOCOL_INFO_DATA {
  UINTN   Attributes;
  VOID    *PointerProtocol;
} POINTER_PROTOCOL_INFO_DATA;

#define ATTRIBUTE_BIT_SIMPLE_OR_ABSOLUTE  (1 << 0)  // Bit 0
#define ATTRIBUTE_VALUE_SIMPLE            (0 << 0)  // Bit 0 clear
#define ATTRIBUTE_VALUE_ABSOLUTE          (1 << 0)  // Bit 0 set

typedef struct _POINTER_PROTOCOL_INFO {
  UINTN                       Count;
  POINTER_PROTOCOL_INFO_DATA  Data[1];
} POINTER_PROTOCOL_INFO;

typedef struct {
  BOOLEAN                               Visible;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *Image;
  RECT                                  ImageRc;
} IMAGE_INFO;

typedef struct {
  UINT32                                  Signature;
  LIST_ENTRY                              Link;
  EFI_GRAPHICS_OUTPUT_PROTOCOL            *GraphicsOutput;
  UINT32                                  BytesPerScanLine;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT        OrgBlt;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT        OriginalBlt;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE   OriginalSetMode;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           *BlendBuffer;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           *CheckBuffer;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           *FillLine;
  IMAGE_INFO                              Screen;
  RECT                                    InvalidateRc;
} GOP_ENTRY;


#define GOP_ENTRY_SIGNATURE     SIGNATURE_32 ('G', 'O', 'P', 'E')
#define GOP_ENTRY_FROM_THIS(a)  CR (a, GOP_ENTRY, Link, GOP_ENTRY_SIGNATURE);


typedef struct _SETUP_MOUSE_KEYBOARD SETUP_MOUSE_KEYBOARD;

typedef struct {
  UINT32   CurrentX;
  UINT32   CurrentY;
  UINT32   CurrentZ;
  BOOLEAN  LButton;
  BOOLEAN  RButton;
} SETUP_MOUSE_STATE;

typedef struct {
  UINT32                        Signature;
  EFI_HANDLE                    Handle;
  EFI_SETUP_MOUSE_PROTOCOL      SetupMouse;
  POINTER_PROTOCOL_INFO         *PointerProtocolInfo;
  LIST_ENTRY                    GopList;

  EFI_LOCK                      SetupMouseLock;


  EFI_EVENT                     MouseHotplugEvent;
  EFI_EVENT                     TouchHotplugEvent;
  EFI_EVENT                     GopHotplugEvent;

  //
  // SetupMouse data
  //
  BOOLEAN                       IsStart;
  BOOLEAN                       HaveRawData;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *CheckBuffer;
  BOOLEAN                       IsCsmEnabled;

  BOOLEAN                       NeedSyncFrameBuffer;
  EFI_IMAGE_START               OrgStartImage;


  //
  // Cursor data
  //
  MOUSE_CURSOR_RANGE            MouseRange;
  BOOLEAN                       LButton, RButton;
  IMAGE_INFO                    Cursor;
  BOOLEAN                       HideCursorWhenTouch;
  BOOLEAN                       HideCursor;
  UINT32                        SaveCursorX;
  UINT32                        SaveCursorY;

  //
  // Keyboard data
  //
  SETUP_MOUSE_KEYBOARD          *KeyboardData;
  IMAGE_INFO                    Keyboard;
  KEYBOARD_ATTRIBUTES           SaveKeyboardAttributes;

  //
  // Setup Mouse state queue buffer
  //
  SETUP_MOUSE_STATE             State[STATE_BUFFER_SIZE];
  UINTN                         BufferIn;
  UINTN                         BufferOut;


  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL   *SimpleTextInputEx;
  EFI_INPUT_READ_KEY_EX               OrgReadKeyStrokeEx;

  EFI_EVENT                           CheckReadKeyEvent;
  EFI_EVENT                           DisableCheckReadKeyEvent;

  UINTN                               NoReadKeyCount;

  BOOLEAN                             FirstIn;
  BOOLEAN                             FirstDisplayKB;
  BOOLEAN                             ReadKeyFlag;
  BOOLEAN                             InStartImage;
//BOOLEAN                             SetupMouseIsStart;

  EFI_GRAPHICS_OUTPUT_PROTOCOL        *GraphicsOutput;
  EFI_SIMPLE_POINTER_PROTOCOL         *SimplePointer;
  EFI_ABSOLUTE_POINTER_PROTOCOL       *AbsolutePointer;

} PRIVATE_MOUSE_DATA;


extern PRIVATE_MOUSE_DATA                *mPrivate;

#define SETUP_MOUSE_DEV_FROM_THIS(a) \
  CR(a, PRIVATE_MOUSE_DATA, SetupMouse, SETUP_MOUSE_SIGNATURE)

//
// Keyboard State in BDA
//
#define BDA_QUALIFIER_STAT      0x417

typedef struct {
  UINT8 RightShift : 1;
  UINT8 LeftShift : 1;
  UINT8 Ctrl : 1;
  UINT8 Alt : 1;
  UINT8 ScrollLock : 1;
  UINT8 NumLock : 1;
  UINT8 CapsLock : 1;
  UINT8 Insert : 1;
} QUALIFIER_STAT;

//
// Setup Mouse services
//
EFI_STATUS
EFIAPI
SetupMouseStart (
  IN  EFI_SETUP_MOUSE_PROTOCOL          *SetupMouse
  );

EFI_STATUS
EFIAPI
SetupMouseClose (
  IN  EFI_SETUP_MOUSE_PROTOCOL          *SetupMouse
  );

EFI_STATUS
EFIAPI
QueryState (
  IN  EFI_SETUP_MOUSE_PROTOCOL          *SetupMouse,
  OUT UINTN                             *X,
  OUT UINTN                             *Y,
  OUT BOOLEAN                           *LeftButton,
  OUT BOOLEAN                           *RightButton
  );

EFI_STATUS
EFIAPI
StartKeyboard (
  IN  EFI_SETUP_MOUSE_PROTOCOL          *SetupMouse,
  IN  UINTN                             X,
  IN  UINTN                             Y
  );

EFI_STATUS
EFIAPI
InternalStartKeyboard (
  IN  EFI_SETUP_MOUSE_PROTOCOL          *SetupMouse,
  IN  UINTN                             X,
  IN  UINTN                             Y
  );

EFI_STATUS
EFIAPI
CloseKeyboard (
  IN  EFI_SETUP_MOUSE_PROTOCOL          *SetupMouse
  );


EFI_STATUS
InternalCloseKeyboard (
  IN  EFI_SETUP_MOUSE_PROTOCOL          *SetupMouse
  );


EFI_STATUS
EFIAPI
SetupMouseSetMode (
  IN EFI_SETUP_MOUSE_PROTOCOL          *SetupMouse,
  IN EFI_SETUP_MOUSE_SCREEN_MODE       Mode
  );


EFI_STATUS
EFIAPI
SetupMouseSetKeyboardAttributes (
  IN  EFI_SETUP_MOUSE_PROTOCOL         *SetupMouse,
  IN  KEYBOARD_ATTRIBUTES              *KeyboardState
  );

EFI_STATUS
EFIAPI
SetupMouseGetKeyboardAttributes (
  IN  EFI_SETUP_MOUSE_PROTOCOL         *SetupMouse,
  IN  KEYBOARD_ATTRIBUTES              *KeyboardState
  );

EFI_STATUS
InternalSetupMouseSetKeyboardAttributes (
  IN  EFI_SETUP_MOUSE_PROTOCOL         *SetupMouse,
  IN  KEYBOARD_ATTRIBUTES              *KeyboardState
  );

EFI_STATUS
InternalSetupMouseGetKeyboardAttributes (
  IN  EFI_SETUP_MOUSE_PROTOCOL         *SetupMouse,
  IN  KEYBOARD_ATTRIBUTES              *KeyboardState
  );




//
// Setup Mouse event function
//
VOID
EFIAPI
ProcessMouse (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
);

EFI_STATUS
UpdateKeyboardStateByBDA (
  VOID
  );

EFI_STATUS
EFIAPI
SetupMouseScreenBlt (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL            *This,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           *BltBuffer, OPTIONAL
  IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION       BltOperation,
  IN  UINTN                                   SourceX,
  IN  UINTN                                   SourceY,
  IN  UINTN                                   DestinationX,
  IN  UINTN                                   DestinationY,
  IN  UINTN                                   Width,
  IN  UINTN                                   Height,
  IN  UINTN                                   Delta         OPTIONAL
  );


//
// SetupMouse Misc function
//
EFI_STATUS
GetStartOffset (
  IN      EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput,
  OUT     UINTN                         *OffsetX,
  OUT     UINTN                         *OffsetY
  );

EFI_STATUS
InitializeCursor (
  IN PRIVATE_MOUSE_DATA                    *Private
  );

VOID
DestroyCursor (
  IN PRIVATE_MOUSE_DATA                    *Private
  );

EFI_STATUS
ProcessKeyboard (
  VOID
  );

EFI_STATUS
RenderImage (
  IN PRIVATE_MOUSE_DATA   *Private,
  IN GOP_ENTRY            *GopEntry,
  IN BOOLEAN              NeedSyncScreen
  );

VOID
HideImage (
  IN  IMAGE_INFO                           *ImageInfo
  );

VOID
MoveImage (
  IN  IMAGE_INFO                           *ImageInfo,
  IN  UINTN                                X,
  IN  UINTN                                Y
  );

VOID
RenderImageForAllGop (
  IN PRIVATE_MOUSE_DATA   *Private
  );

VOID
DestroyImage (
  IN IMAGE_INFO *ImageInfo
  );


UINTN
GetStringWidth (
  IN CHAR16            *String
  );


EFI_STATUS
SetupMouseDrawText (
  IN IMAGE_INFO                    *ImageInfo,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *TextColor,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BackgroundColor,
  IN EFI_STRING                    String,
  IN RECT                          *TextRc,
  IN UINT16                        FontSize
  );

VOID
SyncScreenImage (
  IN  PRIVATE_MOUSE_DATA  *Private,
  IN  GOP_ENTRY           *GopEntry,
  IN  RECT                *Rc,
  OUT BOOLEAN             *ImageIsSame
  );

VOID
InvalidateImage (
  IN PRIVATE_MOUSE_DATA                    *Private,
  IN IMAGE_INFO                           *ImageInfo
  );

VOID
InvalidateRect (
  IN PRIVATE_MOUSE_DATA         *Private,
  IN RECT                       *Rect
  );

VOID
ShowImage (
  IN  IMAGE_INFO                           *ImageInfo
  );


EFI_STATUS
InternalSetCursorPos (
  IN  UINTN                                 X,
  IN  UINTN                                 Y
  );


EFI_STATUS
SetupMouseShowBitmap(
  IN RECT                          *DstRc,
  IN RECT                          *Scale9Grid,
  IN UINT16                        ImageId
  );

VOID
AcquireSetupMouseLock (
  PRIVATE_MOUSE_DATA            *Private
  );

VOID
ReleaseSetupMouseLock (
  PRIVATE_MOUSE_DATA            *Private
  );

EFI_STATUS
EFIAPI
SetCursorPos (
  IN  UINTN                                 X,
  IN  UINTN                                 Y
  );



#endif

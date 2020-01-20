/** @file
  Functions for H2O display engine driver.

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

#include "H2ODisplayEngineLocalText.h"

EFI_SETUP_MOUSE_PROTOCOL                    *mSetupMouse = NULL;
STATIC BOOLEAN                              mVirtualKbVisibled = FALSE;

EFI_STATUS
DEConOutOutputString (
  IN CHAR16                                 *String
  )
{
  EFI_STATUS                                Status;
  H2O_FORM_BROWSER_CONSOLE_DEV_NODE         *ConDevNode;
  LIST_ENTRY                                *Link;

  if (IsListEmpty (&mDEPrivate->ConsoleDevListHead)) {
    return EFI_NOT_FOUND;
  }

  Link = GetFirstNode (&mDEPrivate->ConsoleDevListHead);
  while (!IsNull (&mDEPrivate->ConsoleDevListHead, Link)) {
    ConDevNode = H2O_FORM_BROWSER_CONSOLE_DEV_NODE_FROM_LINK (Link);

    Status = ConDevNode->SimpleTextOut->OutputString (ConDevNode->SimpleTextOut, String);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Link = GetNextNode (&mDEPrivate->ConsoleDevListHead, Link);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DEConOutQueryModeWithoutModeNumer (
  OUT UINT32                                *Columns,
  OUT UINT32                                *Rows
  )
{
  EFI_STATUS                                Status;
  H2O_FORM_BROWSER_CONSOLE_DEV_NODE         *ConDevNode;
  LIST_ENTRY                                *Link;
  UINTN                                     ColumnValue;
  UINTN                                     RowValue;

  if (IsListEmpty (&mDEPrivate->ConsoleDevListHead)) {
    return EFI_NOT_FOUND;
  }

  ColumnValue = 0;
  RowValue    = 0;
  Link        = GetFirstNode (&mDEPrivate->ConsoleDevListHead);
  while (!IsNull (&mDEPrivate->ConsoleDevListHead, Link)) {
    ConDevNode = H2O_FORM_BROWSER_CONSOLE_DEV_NODE_FROM_LINK (Link);

    Status = ConDevNode->SimpleTextOut->QueryMode (
                                          ConDevNode->SimpleTextOut,
                                          ConDevNode->SimpleTextOut->Mode->Mode,
                                          &ColumnValue,
                                          &RowValue
                                          );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Link = GetNextNode (&mDEPrivate->ConsoleDevListHead, Link);
  }

  *Columns = (UINT32) ColumnValue;
  *Rows    = (UINT32) RowValue;

  return EFI_SUCCESS;
}

EFI_STATUS
DEConOutSetAttribute (
  IN UINTN                                  Attribute
  )
{
  EFI_STATUS                                Status;
  H2O_FORM_BROWSER_CONSOLE_DEV_NODE         *ConDevNode;
  LIST_ENTRY                                *Link;

  if (IsListEmpty (&mDEPrivate->ConsoleDevListHead)) {
    return EFI_NOT_FOUND;
  }

  Link = GetFirstNode (&mDEPrivate->ConsoleDevListHead);
  while (!IsNull (&mDEPrivate->ConsoleDevListHead, Link)) {
    ConDevNode = H2O_FORM_BROWSER_CONSOLE_DEV_NODE_FROM_LINK (Link);

    Status = ConDevNode->SimpleTextOut->SetAttribute (ConDevNode->SimpleTextOut, Attribute);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Link = GetNextNode (&mDEPrivate->ConsoleDevListHead, Link);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DEConOutSetNarrowAttribute (
  VOID
  )
{
  EFI_STATUS                                Status;
  H2O_FORM_BROWSER_CONSOLE_DEV_NODE         *ConDevNode;
  LIST_ENTRY                                *Link;

  if (IsListEmpty (&mDEPrivate->ConsoleDevListHead)) {
    return EFI_NOT_FOUND;
  }

  Link = GetFirstNode (&mDEPrivate->ConsoleDevListHead);
  while (!IsNull (&mDEPrivate->ConsoleDevListHead, Link)) {
    ConDevNode = H2O_FORM_BROWSER_CONSOLE_DEV_NODE_FROM_LINK (Link);

    ConDevNode->SimpleTextOut->Mode->Attribute = ConDevNode->SimpleTextOut->Mode->Attribute & NARROW_ATTRIBUTE;
    Status = ConDevNode->SimpleTextOut->SetAttribute (ConDevNode->SimpleTextOut, ConDevNode->SimpleTextOut->Mode->Attribute);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Link = GetNextNode (&mDEPrivate->ConsoleDevListHead, Link);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DEConOutSetCursorPosition (
  IN UINTN                                  Column,
  IN UINTN                                  Row
  )
{
  EFI_STATUS                                Status;
  H2O_FORM_BROWSER_CONSOLE_DEV_NODE         *ConDevNode;
  LIST_ENTRY                                *Link;

  if (IsListEmpty (&mDEPrivate->ConsoleDevListHead)) {
    return EFI_NOT_FOUND;
  }

  Link = GetFirstNode (&mDEPrivate->ConsoleDevListHead);
  while (!IsNull (&mDEPrivate->ConsoleDevListHead, Link)) {
    ConDevNode = H2O_FORM_BROWSER_CONSOLE_DEV_NODE_FROM_LINK (Link);

    Status = ConDevNode->SimpleTextOut->SetCursorPosition (ConDevNode->SimpleTextOut, Column, Row);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Link = GetNextNode (&mDEPrivate->ConsoleDevListHead, Link);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DEConOutEnableCursor (
  IN BOOLEAN                                Visible
  )
{
  EFI_STATUS                                Status;
  H2O_FORM_BROWSER_CONSOLE_DEV_NODE         *ConDevNode;
  LIST_ENTRY                                *Link;

  if (IsListEmpty (&mDEPrivate->ConsoleDevListHead)) {
    return EFI_NOT_FOUND;
  }

  Link = GetFirstNode (&mDEPrivate->ConsoleDevListHead);
  while (!IsNull (&mDEPrivate->ConsoleDevListHead, Link)) {
    ConDevNode = H2O_FORM_BROWSER_CONSOLE_DEV_NODE_FROM_LINK (Link);

    Status = ConDevNode->SimpleTextOut->EnableCursor (ConDevNode->SimpleTextOut, Visible);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Link = GetNextNode (&mDEPrivate->ConsoleDevListHead, Link);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
InitializeVirtualKb (
  RECT                                        *ControlAbsField
  )
{
  EFI_STATUS                                  Status;
  KEYBOARD_ATTRIBUTES                         KeyboardAttributes;
  H2O_FORM_BROWSER_CONSOLE_DEV_NODE           *ConDevNode;
  LIST_ENTRY                                  *Link;
  UINT32                                      VerticalResolution;
  UINT32                                      KeyboardHeight;
  UINT32                                      ControlTop;
  UINT32                                      ControlBottom;
  UINT32                                      OffsetY;
  UINTN                                       MaxCol;
  UINTN                                       MaxRow;
  UINTN                                       X;
  UINTN                                       Y;

  if (IsListEmpty (&mDEPrivate->ConsoleDevListHead)) {
    return EFI_NOT_FOUND;
  }

  if (mSetupMouse == NULL) {
    Status = gBS->LocateProtocol (&gSetupMouseProtocolGuid, NULL, (VOID **) &mSetupMouse);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = mSetupMouse->GetKeyboardAttributes (mSetupMouse, &KeyboardAttributes);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  KeyboardAttributes.WidthPercentage  = 100;
  KeyboardAttributes.HeightPercentage = 40;
  mSetupMouse->SetKeyboardAttributes (mSetupMouse, &KeyboardAttributes);


  Link = GetFirstNode (&mDEPrivate->ConsoleDevListHead);
  ConDevNode = H2O_FORM_BROWSER_CONSOLE_DEV_NODE_FROM_LINK (Link);
  if (ConDevNode->GraphicsOut == NULL) {
    return EFI_NOT_FOUND;
  }

  VerticalResolution = ConDevNode->GraphicsOut->Mode->Info->VerticalResolution;
  KeyboardHeight     = (VerticalResolution * KeyboardAttributes.HeightPercentage) / 100;

  ControlTop    = ControlAbsField->top * EFI_GLYPH_HEIGHT;
  ControlBottom = (ControlAbsField->bottom + 1) * EFI_GLYPH_HEIGHT;

  OffsetY = 0;
  Status = ConDevNode->SimpleTextOut->QueryMode (
                                        ConDevNode->SimpleTextOut,
                                        ConDevNode->SimpleTextOut->Mode->Mode,
                                        &MaxCol,
                                        &MaxRow
                                        );
  if (!EFI_ERROR(Status)) {
    OffsetY = (VerticalResolution - (UINT32)MaxRow * EFI_GLYPH_HEIGHT) / 2;
  }

  Status = mSetupMouse->GetKeyboardAttributes (mSetupMouse, &KeyboardAttributes);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((ControlBottom + KeyboardHeight) < VerticalResolution) {
    X = 0;
    Y = VerticalResolution - KeyboardHeight;
  } else {
    X = 0;
    Y = OffsetY + (ControlTop - 1) - KeyboardHeight;
  }

  if (!KeyboardAttributes.IsStart || (UINTN)KeyboardAttributes.X != X || (UINTN)KeyboardAttributes.Y != Y) {
    mSetupMouse->StartKeyboard (mSetupMouse, X, Y);
  }

  mVirtualKbVisibled = TRUE;

  return EFI_SUCCESS;
}

EFI_STATUS
ShutdownVirtualKb (
  VOID
  )
{
  EFI_STATUS                                  Status;
  KEYBOARD_ATTRIBUTES                         KeyboardAttributes;

  if (mSetupMouse == NULL) {
    Status = gBS->LocateProtocol (&gSetupMouseProtocolGuid, NULL, (VOID **) &mSetupMouse);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = mSetupMouse->GetKeyboardAttributes (mSetupMouse, &KeyboardAttributes);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mVirtualKbVisibled = FALSE;

  if (!KeyboardAttributes.IsStart) {
    return EFI_SUCCESS;
  }

  return mSetupMouse->CloseKeyboard (mSetupMouse);
}

BOOLEAN
IsVirtualKbVisibled (
  )
{
  return mVirtualKbVisibled;
}

EFI_STATUS
TransferToTextModePosition (
  IN  INT32                                  GopX,
  IN  INT32                                  GopY,
  OUT UINT32                                 *Column,
  OUT UINT32                                 *Row
  )
{
  EFI_STATUS                                 Status;
  UINT32                                     MaxCol;
  UINT32                                     MaxRow;

  Status = DEConOutQueryModeWithoutModeNumer (&MaxCol, &MaxRow);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (GopX >= (INT32) (MaxCol - 1) * NARROW_TEXT_WIDTH) {
    *Column = MaxCol;
  } else {
    *Column = (GopX > 0) ? (UINT32) (GopX / NARROW_TEXT_WIDTH) : 0;
  }

  if (GopY >= (INT32) MaxRow * TEXT_HEIGHT) {
    *Row = MaxRow;
  } else {
    *Row = (GopY > 0) ? (UINT32) (GopY / TEXT_HEIGHT) : 0;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
GetScreenField (
  OUT RECT                                    *ScreenField
  )
{
  EFI_STATUS                                  Status;
  UINT32                                      Columns;
  UINT32                                      Rows;

  if (ScreenField == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = DEConOutQueryModeWithoutModeNumer (&Columns, &Rows);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ScreenField->top    = 0;
  ScreenField->left   = 0;
  ScreenField->right  = (INT32) (Columns - 1);
  ScreenField->bottom = (INT32) (Rows - 1);

  return EFI_SUCCESS;
}

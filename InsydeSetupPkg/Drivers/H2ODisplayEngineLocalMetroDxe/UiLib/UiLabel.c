/** @file
  UI Common Controls

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

#include "UiControls.h"

STATIC UI_LABEL_CLASS            *mLabelClass = NULL;
#define CURRENT_CLASS            mLabelClass

SIZE
EFIAPI
UiLabelEstimateSize (
  UI_CONTROL *Control,
  SIZE       AvailableSize
  )
{

  RECT                          TextRc;
  UI_LABEL                      *This;
  SIZE                          Size;
  INT32                         TextWidth;
  UINT32                        TextStyle;


  This = (UI_LABEL *) Control;

  if (This->Text[0] == '\0') {
    switch ((INTN) Control->FixedSize.cx) {

    case (INTN) MATCH_PARENT:
      Size.cx = AvailableSize.cx - (Control->Padding.left + Control->Padding.right);
      break;

    case (INTN) WRAP_CONTENT:
      Size.cx = 1;
      break;

    case 0:
    default:
      Size.cx = Control->FixedSize.cx;
      break;
    }

    switch ((INTN) Control->FixedSize.cy) {

    case (INTN) MATCH_PARENT:
      Size.cy = AvailableSize.cy - (Control->Padding.top + Control->Padding.bottom);
      break;

    case (INTN) WRAP_CONTENT:
      Size.cy = This->FontSize;
      break;

    case 0:
    default:
      Size.cy= Control->FixedSize.cy;
      break;
    }

    return Size;
  }

  switch ((INTN) Control->FixedSize.cy) {

  case (INTN) WRAP_CONTENT:
    switch ((INTN) Control->FixedSize.cx) {

    case (INTN) MATCH_PARENT:
      TextWidth = AvailableSize.cx - (Control->Padding.left + Control->Padding.right);
      break;

    case (INTN) WRAP_CONTENT:
      //
      // BUGBUG: not yet implement
      //
      ASSERT (FALSE);
      TextWidth = AvailableSize.cx;
      break;

    case 0:
      Size.cx = Control->FixedSize.cx;
      Size.cy = This->FontSize;
      return Size;

    default:
      TextWidth = Control->FixedSize.cx - (Control->Padding.left + Control->Padding.right);
      break;
    }
    ASSERT (TextWidth >= 0);

    SetRect (&TextRc, 0, 0, TextWidth, 9999);
    TextStyle = This->TextStyle;
    if ((TextStyle & DT_SINGLELINE) == 0) {
      TextStyle |= DT_WORDBREAK;
    }
    UiPaintText (
      Control->Manager->PaintDC,
      This->FontSize,
      &TextRc,
      This->Text,
      This->TextColor,
      INVALID_COLOR,
      TextStyle | DT_CALCRECT
      );
    Size.cx = Control->FixedSize.cx;
    Size.cy = MAX (TextRc.bottom - TextRc.top + Control->Padding.top  + Control->Padding.bottom, Control->MinSize.cy);
    Size.cy = MAX (Size.cy, 0);
    break;

  case (INTN) MATCH_PARENT:
    Size    = Control->FixedSize;
    Size.cy = AvailableSize.cy;

    switch ((INTN) Control->FixedSize.cx) {

    case (INTN) MATCH_PARENT:
      Size.cx = AvailableSize.cx;
      break;
    }
    break;

  case 0:
  default:
    Size = Control->FixedSize;

    switch ((INTN) Control->FixedSize.cx) {

    case (INTN) MATCH_PARENT:
      Size.cx = AvailableSize.cx;
      break;

    case (INTN) WRAP_CONTENT:
      SetRect (&TextRc, 0, 0, 99999, 99999);
      TextStyle = This->TextStyle;
      UiPaintText (
        Control->Manager->PaintDC,
        This->FontSize,
        &TextRc,
        This->Text,
        This->TextColor,
        INVALID_COLOR,
        TextStyle | DT_CALCRECT
        );
      Size.cx = MAX (TextRc.right - TextRc.left + Control->Padding.left  + Control->Padding.right, Control->MinSize.cx);
      Size.cx = MAX (Size.cx, 0);
      break;
    }
    break;
  }

  Size.cx = MAX (Size.cx, Control->MinSize.cx);
  if (Control->MaxSize.cx > 0) {
    Size.cx = MIN (Size.cx, Control->MaxSize.cx);
  }
  return Size;

}


BOOLEAN
EFIAPI
UiLabelSetAttribute (
  UI_CONTROL *Control,
  CHAR16     *Name,
  CHAR16     *Value
  )
{
  UI_LABEL                      *This;
  EFI_STATUS                    Status;
  CHAR16                        *Delim;

  This = (UI_LABEL *) Control;

  if (StrCmp (Name, L"text") == 0) {
    FreePool (This->Text);
    This->Text = StrDuplicate (Value);
  } else if (StrCmp (Name, L"font-size") == 0) {
    This->FontSize = (INT32)StrToUInt (Value, 10, &Status);
  } else if (StrCmp (Name, L"textcolor") == 0) {
    This->TextColor = GetColorValue (Value);
  } else if (StrCmp (Name, L"text-align") == 0) {

    do {
      if (StrnCmp (Value, L"bottom", 6) == 0) {
        This->TextStyle &= ~(DT_TOP | DT_VCENTER | DT_LEFT | DT_RIGHT | DT_CENTER);
        This->TextStyle |= DT_CENTER | DT_BOTTOM | DT_SINGLELINE;
      } else if (StrnCmp (Value, L"center", 6) == 0) {
        This->TextStyle &= ~(DT_LEFT | DT_RIGHT);
        This->TextStyle |= DT_CENTER;
      } else if (StrnCmp (Value, L"singleline", 10) == 0) {
        This->TextStyle |= DT_SINGLELINE;
      } else if (StrnCmp (Value, L"right", 5) == 0) {
        This->TextStyle |= DT_RIGHT;
      } else if (StrnCmp (Value, L"left", 4) == 0) {
        This->TextStyle |= DT_LEFT;
      } else {
        DEBUG ((EFI_D_ERROR, "Unsuppoerted align type:%s", Value));
        ASSERT (!L"Unsupported align type");
      }
      Delim = StrStr (Value, L"|");
      if (Delim == NULL) {
        break;
      } else {
        Value = Delim + 1;
      }
    } while (*Value != L'\0');
  } else if (StrCmp (Name, L"text-overflow") == 0) {
    if (StrnCmp (Value, L"ellipsis", 8) == 0) {
      This->TextStyle |= DT_END_ELLIPSIS;
    } else if (StrCmp (Value, L"") == 0) {
      This->TextStyle &= ~(DT_END_ELLIPSIS);
    } else {
      DEBUG ((EFI_D_ERROR, "Unsuppoerted text-overflow type:%s", Value));
      ASSERT (!L"Unsupported align type");
    }
  } else {
    return PARENT_CLASS_SET_ATTRIBUTE (CURRENT_CLASS, Control, Name, Value);
  }
  CONTROL_CLASS_INVALIDATE (This);

  return TRUE;
}


LRESULT
EFIAPI
UiLabelProc (
  HWND   Hwnd,
  UINT32 Msg,
  WPARAM WParam,
  LPARAM LParam
  )
{
  UI_CONTROL   *Control;
  UI_LABEL     *This;
  UINT32       TextColor;
  RECT   Rect;
  HDC    Hdc;
  UI_MANAGER   *Manager;
  UINT32       TextStyle;

  Control = (UI_CONTROL *) GetWindowLongPtr (Hwnd, 0);
  if (Control == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }
  This    = (UI_LABEL *) Control;

  Manager = NULL;
  if (This != NULL) {
    Manager = This->Control.Manager;
  }

  switch (Msg) {

  case WM_CREATE:
    This = (UI_LABEL *) AllocateZeroPool (sizeof (UI_LABEL));
    if (This != NULL) {
      CONTROL_CLASS (This) = (UI_CONTROL_CLASS *) GetClassLongPtr (Hwnd, 0);
      SetWindowLongPtr (Hwnd, 0, (INTN)This);
      SendMessage (Hwnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, UI_NOTIFY_CREATE, WParam, LParam);

    This->TextColor   = 0;
    This->TextStyle   = DT_VCENTER;
    This->Text        = StrDuplicate (L"");
    break;

  case UI_NOTIFY_PAINT:
    if (WParam == PAINT_TEXT) {
      Hdc = Manager->PaintDC;
      Manager->GetControlRect (Manager, (UI_CONTROL *)This, &Rect);

      TextColor = This->TextColor;

      if (TextColor == 0) {
        TextColor = 0xFF000000;
      }

      Rect.left   += Control->Padding.left;
      Rect.top    += Control->Padding.top;
      Rect.right  -= Control->Padding.right;
      Rect.bottom -= Control->Padding.bottom;

      if (This->FontSize == 0) {
        This->FontSize = Control->Manager->GetDefaultFontSize (Control->Manager);
      }
      Control->Manager->PaintDC->font->FontSize = This->FontSize;

      TextStyle = This->TextStyle;
      if ((TextStyle & DT_SINGLELINE) == 0) {
        TextStyle |= DT_WORDBREAK;
      }

      UiPaintText (
        Control->Manager->PaintDC,
        This->FontSize,
        &Rect,
        This->Text,
        TextColor,
        INVALID_COLOR,
        TextStyle
        );
    } else {
      PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);
    }
    break;

  case WM_NCHITTEST:
    return HTTRANSPARENT;

  case WM_DESTROY:
    if (This->Text != NULL) {
      FreePool (This->Text);
    }
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);
    break;

  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);
  }

  return 0;
}

UI_LABEL_CLASS *
EFIAPI
GetLabelClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), L"Label", (UI_CONTROL_CLASS *)GetControlClass());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc      = UiLabelProc;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetAttribute = UiLabelSetAttribute;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->EstimateSize = UiLabelEstimateSize;


  return CURRENT_CLASS;
}


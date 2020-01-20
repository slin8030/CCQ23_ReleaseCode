/** @file
  UI Common Controls

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

#include "UiControls.h"

STATIC UI_EDIT_CLASS           *mEditClass = NULL;
#define CURRENT_CLASS           mEditClass

VOID
UiEditCreate (
  UI_EDIT *This
  )
{
  UI_CONTROL *Control;
  SIZE       Size;

  Control = (UI_CONTROL *) This;

  GetTextExtentPoint32 (Control->Manager->PaintDC, L"X", 1, &Size);
  This->CharHeight  = Size.cy;
  This->EditPos     = 0;
  This->MinValue    = 0;
  This->MaxValue    = 0;
  This->Step        = 0;
  This->ValueType   = NOT_VALUE;
}

VOID
UiEditSetPasswordMode (
  UI_EDIT *This,
  BOOLEAN PasswordMode
  )
{
  if (This->IsPasswordMode != PasswordMode) {
    This->IsPasswordMode = PasswordMode;
    CONTROL_CLASS(This)->Invalidate ((UI_CONTROL *)This);
  }
}

VOID
UiEditSetReadOnly (
  UI_EDIT *This,
  BOOLEAN ReadOnly
  )
{
  if (This->IsReadOnly != ReadOnly) {
    This->IsReadOnly = ReadOnly;
    CONTROL_CLASS(This)->Invalidate ((UI_CONTROL *)This);
  }
}


SIZE
EFIAPI
UiEditEstimateSize (
  UI_CONTROL *Control,
  SIZE       AvailableSize
  )
{

  RECT        TextRc;
  UI_EDIT     *This;
  SIZE        Size;
  UI_LABEL    *Label;

  This = (UI_EDIT *) Control;
  Label = (UI_LABEL *) Control;

  if (Control->FixedSize.cx != 0) {
   AvailableSize.cx = MIN (AvailableSize.cx, Control->FixedSize.cx);
  }

  if (Label->FontSize == 0) {
    Label->FontSize = Control->Manager->GetDefaultFontSize (Control->Manager);
  }

  if (Label->Text == '\0') {
    Size    = Control->FixedSize;
    Size.cy = Label->FontSize;
    return Size;
  }

  SetRect (&TextRc, 0, 0, AvailableSize.cx, 9999);
//TextRc.left  += This->TextPadding.left;
//TextRc.right -= This->TextPadding.right;
  if (Label->FontSize == 0) {
    Label->FontSize = Control->Manager->GetDefaultFontSize (Control->Manager);
  }

  UiPaintText (
    Control->Manager->PaintDC,
    Label->FontSize,
    &TextRc,
    Label->Text,
    INVALID_COLOR,
    INVALID_COLOR,
    DT_WORDBREAK | DT_CALCRECT
    );

  Size.cx = TextRc.right - TextRc.left; // + This->TextPadding.left + This->TextPadding.right;
  Size.cy = TextRc.bottom - TextRc.top; // + This->TextPadding.top + This->TextPadding.bottom;

  if (Control->FixedSize.cy != 0) {
    Size.cy = Control->FixedSize.cy;
  }

  if (Control->FixedSize.cx != 0) {
    Size.cx = Control->FixedSize.cx;
  }

  return Size;
}



BOOLEAN
EFIAPI
UiEditSetAttribute (
  UI_CONTROL  *Control,
  CHAR16      *Name,
  CHAR16      *Value
  )
{
  UI_EDIT     *This;
  EFI_STATUS  Status;

  This = (UI_EDIT *) Control;

  if (StrCmp (Name, L"valuetype") == 0) {
    if (StrCmp (Value, L"dec") == 0) {
      This->ValueType = DEC_VALUE;
    } else if (StrCmp (Value, L"hex") == 0) {
      This->ValueType = HEX_VALUE;
    } else {
      This->ValueType = NOT_VALUE;
    }
  } else if (StrCmp (Name, L"password") == 0) {
    UiEditSetPasswordMode (This, (StrCmp (Value, L"true") == 0));
  } else if (StrCmp (Name, L"readonly") == 0) {
    UiEditSetReadOnly (This, (StrCmp (Value, L"true") == 0));
  } else if (StrCmp (Name, L"maxlength") == 0) {
    This->MaxLength = (UINT32) StrToUInt (Value, 10, &Status);
  } else if (StrCmp (Name, L"maxvalue") == 0) {
    This->MaxValue = StrToUInt (Value, 10, &Status);
  } else if (StrCmp (Name, L"minvalue") == 0) {
    This->MinValue = StrToUInt (Value, 10, &Status);
  } else {
    return PARENT_CLASS_SET_ATTRIBUTE (CURRENT_CLASS, Control, Name, Value);
  }

  return TRUE;
}

VOID
UiEditKeyDown (
  HWND Hwnd,
  WPARAM WParam,
  LPARAM LParam
  )
{

  UI_CONTROL *Control;
  UI_EDIT    *Edit;
  BOOLEAN    ReDraw;
  UINTN      LastEditPos;
  SIZE       Size;
  UI_MANAGER *Manager;
  UI_LABEL   *Label;

  Control = GetUiControl (Hwnd);
  Edit    = (UI_EDIT *) Control;
  Label   = (UI_LABEL *) Control;
  LastEditPos = Edit->EditPos;
  Manager = Control->Manager;


  ReDraw  = FALSE;

  switch (WParam) {

  case VK_LEFT:
    if (Edit->EditPos > 0) {
      Edit->EditPos--;
    }

    break;

  case VK_RIGHT:
    if (Edit->EditPos < StrLen (Label->Text)) {
      Edit->EditPos++;
    }
    break;

  case VK_HOME:
    Edit->EditPos = 0;
    break;

  case VK_END:
    Edit->EditPos = (UINT32) StrLen (Label->Text);
    break;


  case VK_BACK:
    if (Edit->EditPos > 0) {
      Edit->EditPos--;
      memmove (Label->Text + Edit->EditPos,
         Label->Text + Edit->EditPos + 1,
         (StrLen (Label->Text) - Edit->EditPos + 1) * sizeof (CHAR16));
      ReDraw = TRUE;
    }
    break;

  case VK_DELETE:
    if (Edit->EditPos < StrLen (Label->Text)) {
      memmove (Label->Text + Edit->EditPos,
         Label->Text + Edit->EditPos + 1,
         (StrLen (Label->Text) - Edit->EditPos + 1) * sizeof (CHAR16));
      ReDraw = TRUE;
    }
    break;
  }

  if (LastEditPos != Edit->EditPos || ReDraw) {
    if (Edit->EditPos != 0) {
      GetTextExtentPoint32 (Manager->PaintDC, Label->Text, (UINT32)Edit->EditPos, &Size);
    }
    CONTROL_CLASS(Edit)->Invalidate (Control);
  }
}

VOID
UiEditCharPress (
  HWND     Hwnd,
  WPARAM   WParam,
  LPARAM   LParam
  )
{
  CHAR16     Char;
  CHAR16     *OldStr;
  CHAR16     *NewStr;
  UI_EDIT    *This;
  SIZE       Size;
  UINT64     EditValue;
  UINT64     TempValue;
 UI_CONTROL *Control;
  UI_LABEL   *Label;
  UINTN      MaxLength;
  CHAR16     LastChar;
  UINTN      LabelTextLength;


  This    = (UI_EDIT *) GetUiControl (Hwnd);
  Control = (UI_CONTROL *)This;
  Label   = (UI_LABEL *) This;

  ASSERT (Label->Text != NULL);
  if (Label->Text == NULL) {
    return ;
  }
  if (This->IsReadOnly) {
    return ;
  }

  Char = LOWORD (WParam);

  switch (Char) {
    case 0x00:  /* NULL */
    case 0x07:  /* BEL */
    case 0x08:  /* BS */
    case 0x09:  /* HT */
    case 0x0A:  /* LF */
    case 0x0B:  /* VT */
    case 0x0C:  /* FF */
    case 0x1B:  /* Escape */
      return ;
    case 0x0D:  /* enter */
      if (mSetupMouse != NULL) {
        mSetupMouse->CloseKeyboard (mSetupMouse);
      }
      return ;
  }

  if (This->ValueType == DEC_VALUE) {
    if (!IsDecChar (Char)) {
      return ;
    }
  }

  if (This->ValueType == HEX_VALUE) {
    if (!IsHexChar (Char)) {
      return ;
    }
  }

  LabelTextLength = StrLen (Label->Text);
  if (This->MaxLength != 0 && LabelTextLength >= This->MaxLength) {
    return;
  }

  if (This->ValueType == DEC_VALUE || This->ValueType == HEX_VALUE) {
    if (This->ValueType == HEX_VALUE) {
      MaxLength = 16;
    } else {
      MaxLength = 20;
    }
    if (LabelTextLength >= MaxLength) {
      return ;
    }
  }

  //
  // insert data
  //
  OldStr = Label->Text;
  NewStr = AllocatePool (StrSize (OldStr) + sizeof (CHAR16));
  if (NewStr == NULL) {
    return;
  }
  if (This->EditPos != 0) {
    CopyMem (NewStr, OldStr, This->EditPos * sizeof (CHAR16));
  }

  NewStr[This->EditPos] = Char;

  CopyMem (NewStr + This->EditPos + 1,
        OldStr + This->EditPos,
        (StrLen (OldStr) - This->EditPos + 1) * sizeof (CHAR16));


  if (This->ValueType == DEC_VALUE || This->ValueType == HEX_VALUE) {

    if (This->ValueType == DEC_VALUE) {

      if (StrLen (NewStr) == 20) {
        //
        // Because 20 digits value maybe make UINT64 overlap
        // use 19 digits to check the value whether overlap
        //
        LastChar = NewStr[19];
        NewStr[19] = '\0';
        TempValue = StrDecimalToUint64 (NewStr);
        NewStr[19] = LastChar;

        //
        // UINT64 maxmium value is 18446744073709551615.
        // If 19 digits is larger than 1844674407370955161ull,
        // use can not input any key.
        //
        if (TempValue > 1844674407370955161ull) {
          FreePool (NewStr);
          return ;
        }

        //
        // UINT64 maxmium value is 18446744073709551615.
        // If 19 digits is equal to 1844674407370955161ull,
        // use can not input larger than '5' key
        //
        if ((TempValue == 1844674407370955161ull) && (LastChar > '5')) {
          FreePool (NewStr);
          return ;
        }
      }

      EditValue = StrDecimalToUint64 (NewStr);
    } else {
      EditValue = StrHexToUint64 (NewStr);
    }

    if (!IsEditValueValid (EditValue, This->MinValue, This->MaxValue, This->ValueType == HEX_VALUE)) {
      FreePool (NewStr);
      return ;
    }
  }

  FreePool (OldStr);
  Label->Text = NewStr;
  This->EditPos++;

  GetTextExtentPoint32 (Control->Manager->PaintDC, Label->Text, This->EditPos, &Size);
//SetCaretPos (Size.cx, 1);
  CONTROL_CLASS(This)->Invalidate (Control);
  return ;
}



LRESULT
EFIAPI
UiEditProc (
  HWND   Hwnd,
  UINT32 Msg,
  WPARAM WParam,
  LPARAM LParam
  )
{
  UI_EDIT      *This;
  UI_CONTROL   *Control;
  UI_MANAGER   *Manager;
//POINT        Point;
  RECT         Rc;
  SIZE         Size;
  LRESULT      Ret;
  CHAR16       *StrBuf;
  CHAR16       *TmpPtr;
  UINTN        Len;
  UINTN        Index;
  KEYBOARD_ATTRIBUTES           KeyboardAttributes;
  UI_LABEL                      *Label;


  Control   = (UI_CONTROL *) GetWindowLongPtr (Hwnd, 0);
  if (Control == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }
  This    = (UI_EDIT *) Control;
  Label    = (UI_LABEL *) Control;
  Manager = NULL;
  if (Control != NULL) {
    Manager = Control->Manager;
  }

  switch (Msg) {

  case WM_CREATE:
    This = (UI_EDIT *) AllocateZeroPool (sizeof (UI_EDIT));
    if (This != NULL) {
      SetWindowLongPtr (Hwnd, 0, (INTN)This);
      SendMessage (Hwnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, UI_NOTIFY_CREATE, WParam, LParam);
    UiEditCreate (This);
    Control->Class              = (UI_CONTROL_CLASS *) GetClassLongPtr (Hwnd, 0);
    Control->BkColor     = 0xFFD0D0D0;
    Control->BorderColor = 0xFF808080;
    SetWindowLongPtr (
      Control->Wnd,
      GWL_EXSTYLE, GetWindowLongPtr (Control->Wnd, GWL_EXSTYLE) & ~WS_EX_NOACTIVATE
      );
    break;

  case WM_SETFOCUS:
    if ((CONTROL_CLASS_GET_STATE (Control) & UISTATE_CAPTURED) != 0) {
      return 0;
    }
    CONTROL_CLASS_SET_STATE (Control, UISTATE_CAPTURED, 0);

    if (This->IsReadOnly) {
      return 0;
    }

//  CreateCaret(Hwnd, NULL, 1, This->CharHeight);
    //This->EditPos = 0;
    This->EditPos = (UINT32) StrLen (Label->Text);
    GetTextExtentPoint32 (Control->Manager->PaintDC, Label->Text, This->EditPos, &Size);
//  SetCaretPos (Size.cx, 1);
//  ShowCaret (Hwnd);
    SendMessage (Manager->MainWnd, UI_NOTIFY_EDITFOCUS, (WPARAM)Control, 0);
    Control->BorderColor = 0xFF800000;
//  mSetupMouse->DrawKeyboard (mSetupMouse, 0, 0);
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);

  case WM_KILLFOCUS:
    CONTROL_CLASS_SET_STATE (Control, 0, UISTATE_CAPTURED);
//  HideCaret(Hwnd);
//  DestroyCaret();
    SendMessage (Manager->MainWnd, UI_NOTIFY_BLUR, (WPARAM)Control, 0);
    Control->BorderColor = 0xFF808080;
    if (mSetupMouse != NULL) {
      mSetupMouse->CloseKeyboard (mSetupMouse);
    }
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);


  case WM_CHAR:
    UiEditCharPress (Hwnd, WParam, LParam);
    break;

  case WM_KEYDOWN:
    if (WParam == VK_RETURN) {
      SendMessage (Manager->MainWnd, UI_NOTIFY_CARRIAGE_RETURN, (WPARAM) Control, 0);
    } else {
      UiEditKeyDown (Hwnd, WParam, LParam);
    }
    break;

  case WM_LBUTTONDOWN:
//  if (GetCapture () != Hwnd) {
//    SetCapture (Hwnd);
//    SetFocus (Hwnd);
//    return 0;
//  }
//
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);
    GetWindowRect (Hwnd, &Rc);
    if ((GetWindowLongPtr (Hwnd, GWL_STYLE) & WS_DISABLED) == 0) {
      if (mSetupMouse != NULL) {
        mSetupMouse->GetKeyboardAttributes (mSetupMouse, &KeyboardAttributes);
        if (!KeyboardAttributes.IsStart) {
          mSetupMouse->StartKeyboard (mSetupMouse, Rc.left, Rc.bottom + 20);
        }
      }
    }
//  ReleaseCapture ();
//  if (
//
//  ReleaseCapture ();
//  This->StateFlags &= ~UISTATE_CAPTURED;
    break;

  case WM_DESTROY:
    if (mSetupMouse != NULL) {
      mSetupMouse->GetKeyboardAttributes (mSetupMouse, &KeyboardAttributes);
      if (KeyboardAttributes.IsStart) {
        mSetupMouse->CloseKeyboard (mSetupMouse);
      }
    }
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);

  case WM_NCHITTEST:
    return HTCLIENT;
    break;

  case UI_NOTIFY_PAINT:
    if (WParam == PAINT_TEXT && This->IsPasswordMode) {
      Len = StrLen (Label->Text);
      if (Len != 0) {
        TmpPtr = Label->Text;
        StrBuf = AllocateZeroPool ((Len + 1) * sizeof (CHAR16));
        if (StrBuf == NULL) {
          return 0;
        }
        for (Index = 0; Index < Len; Index++) {
          StrBuf[Index] = L'*';
        }
        Label->Text = StrBuf;
        Ret =  PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);
        Label->Text = TmpPtr;
        FreePool (StrBuf);
        return Ret;
      }
    }
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);


  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);
  }

  return 0;
}

UI_EDIT_CLASS *
EFIAPI
GetEditClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), L"UiEdit", (UI_CONTROL_CLASS *)GetLabelClass());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc      = UiEditProc;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetAttribute = UiEditSetAttribute;
  //((UI_CONTROL_CLASS *)CURRENT_CLASS)->EstimateSize = UiEditEstimateSize;

  return CURRENT_CLASS;
}


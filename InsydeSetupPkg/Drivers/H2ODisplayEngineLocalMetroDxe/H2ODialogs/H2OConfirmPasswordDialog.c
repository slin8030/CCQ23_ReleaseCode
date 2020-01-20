/** @file
  UI ordered list control

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "H2ODisplayEngineLocalMetro.h"
#include "UiControls.h"
#include "MetroUi.h"
#include "MetroDialog.h"
#include <Library/ConsoleLib.h>

CHAR16 *mConfirmPasswordDialogChilds = L""
  L"<VerticalLayout name='DialogWithoutSendForm'>"
    L"<Control/>"
    L"<Control background-color='@menucolor' name='parent' height='wrap_content'>"
      L"<VerticalLayout padding='20,30,20,30' background-color='@menucolor' height='wrap_content'>"
        L"<VerticalLayout name='TitleLayout' height='wrap_content'>"
          L"<Label name='DialogTitle' text-align='center' height='40' visibility='false' font-size='19' textcolor='0xFFFFFFFF'/>"
          L"<Label width='match_parent' textcolor='0xFFFFFFFF' font-size='19' name='DialogText' height='wrap_content'/>"
          L"<Control height='15'/>"
          L"<Control height='10' background-image='@DialogSeparator' background-color='0x0' background-image-style='center'/>"
          L"<Control height='15'/>"
        L"</VerticalLayout>"
        L"<VerticalLayout padding='0,0,10,0' min-height='51' name='DialogPasswordInput' height='wrap_content'>"
          L"<Label name='ConfirmNewPasswordLabel' height='wrap_content' padding='0,0,2,0' width='match_parent' font-size='18' textcolor='0xFFFFFFFF'/>"
          L"<Control padding='2,2,2,2' min-height='32' background-color='0xFFCCCCCC' height='wrap_content'>"
            L"<UiEdit name='ConfirmPasswordInput' focusbkcolor='0xFFFFFFFF' tabstop='true' height='27' padding='7,3,0,3' taborder='1' background-color='@menulightcolor' password='true'/>"
          L"</Control>"
        L"</VerticalLayout>"
      L"</VerticalLayout>"
      L"<Texture name='FormHalo' float='true' height='-1' width='-1' background-image='@FormHalo' scale9grid='23,26,22,31'/>"
    L"</Control>"
    L"<Control/>"
  L"</VerticalLayout>";


EFI_STATUS
SendConfirmPassword (
  IN  UI_DIALOG                 *Dialog
  )
{
  UI_CONTROL                    *Control;
  CHAR16                        *PasswordStr;
  EFI_HII_VALUE                 HiiValue;

  Control = UiFindChildByName (Dialog, L"ConfirmPasswordInput");
  PasswordStr = ((UI_LABEL *) Control)->Text;

  HiiValue.BufferLen = (UINT16) StrSize (PasswordStr);
  HiiValue.Buffer    = AllocatePool (HiiValue.BufferLen);
  ASSERT (HiiValue.Buffer != NULL);
  CopyMem (HiiValue.Buffer, PasswordStr, HiiValue.BufferLen);
  SendChangeQNotify (0, 0, &HiiValue);

  return EFI_SUCCESS;
}


LRESULT
H2OConfirmPasswordProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  UI_DIALOG                     *Dialog;
  UI_CONTROL                    *Control;
  CHAR16                        Str[20];

  Dialog  = (UI_DIALOG *) GetWindowLongPtr (Wnd, 0);

  switch (Msg) {

  case UI_NOTIFY_WINDOWINIT:
    if (gFB->CurrentP == NULL) {
      Control = UiFindChildByName (Dialog, L"TitleLayout");
      UiSetAttribute (Control, L"visibility", L"false");
    }

    if (mFbDialog->TitleString != NULL) {
      Control = UiFindChildByName (Dialog, L"DialogText");
      UiSetAttribute (Control, L"text", mFbDialog->TitleString);
      Control = UiFindChildByName (Dialog, L"ConfirmNewPasswordLabel");
      UiSetAttribute (Control, L"text", mFbDialog->TitleString);
    }

    if (mFbDialog->BodyInputCount != 0) {
      UnicodeSPrint (Str, sizeof (Str), L"%d", (mFbDialog->ConfirmHiiValue.BufferLen / sizeof (CHAR16) - 1));
      Control = UiFindChildByName (Dialog, L"ConfirmPasswordInput");
      UiSetAttribute (Control, L"maxlength", Str);
    }

    if (gFB->CurrentP != NULL) {
      mTitleVisible = TRUE;
      SetTimer (Wnd, 0, 1, DialogCallback);
    }

    Control = UiFindChildByName (Dialog, L"ConfirmPasswordInput");
    SetFocus (Control->Wnd);
    break;

  case UI_NOTIFY_CLICK:
  case UI_NOTIFY_CARRIAGE_RETURN:
    //
    // click ok button or ConfirmPasswordInput passowrd input
    //
    SendConfirmPassword (Dialog);
    return 0;
    break;

  case WM_DESTROY:
    if (gFB->CurrentP != NULL) {
      KillTimer (Wnd, 0);
    }
    return 0;

  default:
    return 0;
  }

  return 1;
}

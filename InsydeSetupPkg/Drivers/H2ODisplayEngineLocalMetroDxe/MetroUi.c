/** @file
  Entry point and initial functions for H2O local Metro display engine driver

;******************************************************************************
;* Copyright (c) 2013 - 2016, Insyde Software Corp. All Rights Reserved.
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
#include "H2OControls.h"
#include "H2OPanels.h"
#include "MetroUi.h"
#include <Guid/ZeroGuid.h>

extern H2O_DISPLAY_ENGINE_METRO_PRIVATE_DATA      *mMetroPrivate;
extern HWND                                       gLastFocus;

EFI_INPUT_KEY                                     mPrivateHotKeyList[] = {{SCAN_NULL , CHAR_CARRIAGE_RETURN},
                                                                          {SCAN_UP   , CHAR_NULL},
                                                                          {SCAN_DOWN , CHAR_NULL},
                                                                          {SCAN_RIGHT, CHAR_NULL},
                                                                          {SCAN_LEFT , CHAR_NULL},
                                                                          {SCAN_ESC  , CHAR_NULL},
                                                                          };

typedef struct _DISPLAY_ENGINE_FRAME DISPLAY_ENGINE_FRAME;

struct _DISPLAY_ENGINE_FRAME {
  UI_FRAME                      Frame;

  UI_CONTROL                    *H2OSetupMenuPanel;
  UI_CONTROL                    *H2OSetupPagePanel;
  UI_CONTROL                    *H2OFormTitlePanel;
  UI_CONTROL                    *H2OHelpTextPanel;
  UI_CONTROL                    *H2OHotkeyPanel;
  UI_CONTROL                    *H2OOwnerDrawPanel;
};

typedef struct _PANEL_VFCF_DATA {
  CHAR16                        *PanelName;
  UINT32                        PanelType;
} PANEL_VFCF_DATA;

PANEL_VFCF_DATA                 mPanelVfcfData[] = {
                                  {L"H2OOwnerDrawPanel", H2O_PANEL_TYPE_OWNER_DRAW},
                                  {L"H2OSetupMenuPanel", H2O_PANEL_TYPE_SETUP_MENU},
                                  {L"H2OFormTitlePanel", H2O_PANEL_TYPE_FORM_TITLE},
                                  {L"H2OSetupPagePanel", H2O_PANEL_TYPE_SETUP_PAGE},
                                  {L"H2OHelpTextPanel" , H2O_PANEL_TYPE_HELP_TEXT},
                                  {L"H2OHotkeyPanel"   , H2O_PANEL_TYPE_HOTKEY},
                                  {NULL, 0},
                                  };

typedef struct _DISPLAY_ENGINE_FRAME_CLASS {
  UI_FRAME_CLASS                ParentClass;
} DISPLAY_ENGINE_FRAME_CLASS;

STATIC DISPLAY_ENGINE_FRAME_CLASS    *mDisplayEngineFrameClass = NULL;
#define CURRENT_CLASS                mDisplayEngineFrameClass

DISPLAY_ENGINE_FRAME_CLASS *
EFIAPI
GetDisplayEngineFrameClass (
  VOID
  );

CHAR16 *mDialogPanelChilds = L""
  L"<VerticalLayout name='DialogPanel' background-color='0xFF000000'/>";

CHAR16 *mFrontPageChilds = L""
  L"<Control name='background' background-color='0xFFE8E8E8'>"
    L"<H2OOwnerDrawPanel RefreshInterval='400' name='H2OOwnerDrawPanel'/>"
    L"<H2OSetupMenuPanel name='H2OSetupMenuPanel' background-color='0xFF000000'/>"
    L"<H2OFormTitlePanel border-color='0xff123456' name='H2OFormTitlePanel'/>"
    L"<H2OSetupPagePanel name='H2OSetupPagePanel'/>"
    L"<H2OHelpTextPanel border-color='0xff123456' name='H2OHelpTextPanel'/>"
    L"<H2OHotkeyPanel border-color='0xff123456' name='H2OHotkeyPanel'/>"
    L"<Texture background-image='@FormHalo' float='true' name='FormHalo' scale9grid='23,26,22,31'/>"
    L"<Texture name='overlay' float='true' visibility='false' height='-1' width='-1' background-color='0x80C8C8C8'/>"
  L"</Control>";

EFI_STATUS
GetInformationField (
  OUT RECT                                    *InfoField
  )
{
  EFI_STATUS                                  Status;
  RECT                                        SetupPagePanelField;
  RECT                                        HelpTextPanelField;

  Status = GetRectByName (gWnd, L"H2OSetupPagePanel", &SetupPagePanelField);
  if (EFI_ERROR (Status)) {
    SetRectEmpty (&SetupPagePanelField);
  }

  Status = GetRectByName (gWnd, L"H2OHelpTextPanel", &HelpTextPanelField);
  if (EFI_ERROR (Status)) {
    SetRectEmpty (&HelpTextPanelField);
  }

  if (SetupPagePanelField.top    == HelpTextPanelField.top &&
      SetupPagePanelField.bottom == HelpTextPanelField.bottom) {
    if (SetupPagePanelField.right == HelpTextPanelField.left ||
        SetupPagePanelField.left  == HelpTextPanelField.right) {
      UnionRect (InfoField, &SetupPagePanelField, &HelpTextPanelField);
      return EFI_SUCCESS;
    }
  } else if (SetupPagePanelField.left  == HelpTextPanelField.left &&
             SetupPagePanelField.right == HelpTextPanelField.right) {
    if (SetupPagePanelField.bottom == HelpTextPanelField.top ||
        SetupPagePanelField.top    == HelpTextPanelField.bottom) {
      UnionRect (InfoField, &SetupPagePanelField, &HelpTextPanelField);
      return EFI_SUCCESS;
    }
  }

  CopyRect (InfoField, &SetupPagePanelField);
  return EFI_SUCCESS;
}

EFI_STATUS
DisableHalo (
  IN DISPLAY_ENGINE_FRAME                    *This
  )
{
  UI_MANAGER                               *Manager;
  UI_CONTROL                               *Control;

  Manager = ((UI_CONTROL *) This)->Manager;
  Control = Manager->FindControlByName (Manager, L"FormHalo");
  if (Control == NULL) {
    return EFI_NOT_FOUND;
  }
  UiSetAttribute (Control, L"visibility", L"false");

  return EFI_SUCCESS;
}

EFI_STATUS
SetPanelPosFromVfcf (
  IN DISPLAY_ENGINE_FRAME       *This
  )
{
  EFI_STATUS                    Status;
  H2O_LAYOUT_INFO               *LayoutInfo;
  RECT                          ScreenField;
  RECT                          PanelField;
  UI_MANAGER                    *Manager;
  UI_CONTROL                    *Control;
  CHAR16                        String[20];
  PANEL_VFCF_DATA               *PanelVfcfDataPtr;
  H2O_PANEL_INFO                *PanelInfo;
  BOOLEAN                       IsVisible;
  UINT32                        LayoutId;
  EFI_GUID                      ScuFormSetGuid = {0x9f85453e, 0x2f03, 0x4989, 0xad, 0x3b, 0x4a, 0x84, 0x07, 0x91, 0xaf, 0x3a};
  H2O_FORM_BROWSER_SM           *SetupMenuData;
  EFI_GUID                      FormSetGuid;

  Manager = ((UI_CONTROL *) This)->Manager;

  Status = gFB->GetSMInfo (gFB, &SetupMenuData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CopyGuid (&FormSetGuid, &SetupMenuData->FormSetGuid);
  FreeSetupMenuData (SetupMenuData);

  LayoutInfo = NULL;

  LayoutId = 0;
  Status = GetLayoutIdByGuid (&FormSetGuid, &LayoutId);
  if (EFI_ERROR (Status)) {
    if (NeedShowSetupMenu ()) {
      Status = GetLayoutIdByGuid (&ScuFormSetGuid, &LayoutId);
    }

    if (EFI_ERROR (Status)) {
      Status = GetLayoutIdByGuid (&gZeroGuid, &LayoutId);
    }

    ASSERT_EFI_ERROR (Status);
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = GetLayoutById (LayoutId, &mMetroPrivate->DisplayEngine.Id, &LayoutInfo);
  if (EFI_ERROR (Status)) {
    Status = GetLayoutIdByGuid (&gZeroGuid, &LayoutId);
    ASSERT_EFI_ERROR (Status);
    Status = GetLayoutById (LayoutId, &mMetroPrivate->DisplayEngine.Id, &LayoutInfo);
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (IsFrontPage ()) {
    DisableHalo (This);
  }

  ScreenField.left   = 0;
  ScreenField.top    = 0;
  ScreenField.right  = (INT32) GetSystemMetrics (SM_CXSCREEN);
  ScreenField.bottom = (INT32) GetSystemMetrics (SM_CYSCREEN);

  PanelVfcfDataPtr = mPanelVfcfData;
  while (PanelVfcfDataPtr->PanelName != NULL) {
    IsVisible = FALSE;
    SetRectEmpty (&PanelField);

    PanelInfo = GetPanelInfo (LayoutInfo, PanelVfcfDataPtr->PanelType);
    if (PanelInfo != NULL) {
      GetPanelField (PanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL, &ScreenField, &PanelField);
      IsVisible = IsVisibility (PanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL);
    }

    Control = Manager->FindControlByName (Manager, PanelVfcfDataPtr->PanelName);
    UnicodeSPrint (String, sizeof (String), L"%d,%d,%d,%d", PanelField.left, PanelField.top, PanelField.right, PanelField.bottom);
    UiSetAttribute (Control, L"pos", String);
    UiSetAttribute (Control, L"visibility", IsVisible ? L"true" : L"false");

    PanelVfcfDataPtr++;
  }

  return Status;
}

VOID
LocalMetroSetComponent (
  DISPLAY_ENGINE_FRAME            *This
  )
{
  UI_MANAGER  *Manager;

  Manager = ((UI_CONTROL *) This)->Manager;

  This->H2OSetupMenuPanel = Manager->FindControlByName (Manager, L"H2OSetupMenuPanel");
  This->H2OSetupPagePanel = Manager->FindControlByName (Manager, L"H2OSetupPagePanel");
  This->H2OFormTitlePanel = Manager->FindControlByName (Manager, L"H2OFormTitlePanel");
  This->H2OHelpTextPanel  = Manager->FindControlByName (Manager, L"H2OHelpTextPanel");
  This->H2OHotkeyPanel    = Manager->FindControlByName (Manager, L"H2OHotkeyPanel");
  This->H2OOwnerDrawPanel = Manager->FindControlByName (Manager, L"H2OOwnerDrawPanel");
}

VOID
LocalMetroProcessUiNotify (
  DISPLAY_ENGINE_FRAME            *This,
  UINT                          Msg,
  UI_CONTROL                    *Sender
  )
{
  H2O_STATEMENT_ID              StatementId;
  H2O_FORM_BROWSER_S            *Statement;
  HOT_KEY_INFO                  *HotkeyInfo;
  EFI_STATUS                    Status;
  EFI_HII_VALUE                 HiiValue;

  switch (Msg) {

  case UI_NOTIFY_CLICK:
  case UI_NOTIFY_CARRIAGE_RETURN:
    if (StrnCmp (Sender->Name, L"Hotkey", StrLen (L"Hotkey")) == 0) {
      HotkeyInfo = (HOT_KEY_INFO *) GetWindowLongPtr (Sender->Wnd, GWLP_USERDATA);
      HotKeyFunc (HotkeyInfo);
      break;
    }

    if (StrCmp (Sender->Name, L"CheckBox") == 0) {
      StatementId = (H2O_PAGE_ID) GetWindowLongPtr (Sender->Wnd, GWLP_USERDATA);
      Status = gFB->GetSInfo (gFB, gFB->CurrentP->PageId, StatementId, &Statement);
      ASSERT_EFI_ERROR (Status);
      if (EFI_ERROR (Status)) {
        break;
      }
      SendSelectQNotify (Statement->PageId, Statement->QuestionId, Statement->IfrOpCode);
      CopyMem (&HiiValue, &Statement->HiiValue, sizeof (EFI_HII_VALUE));
      HiiValue.Value.b = !HiiValue.Value.b;
      SendChangeQNotify (Statement->PageId, Statement->QuestionId, &HiiValue);
      break;
    }
    DEBUG ((EFI_D_INFO, "Unsuppoert item click: %s", Sender->Name));
    ASSERT (FALSE);
    break;
  }
}

STATIC
VOID
UpdateHaloColor (
  IN DISPLAY_ENGINE_FRAME            *This
  )
{
  UI_CONTROL                    *Control;
  HSV_VALUE                     MenuHsv;
  HSV_VALUE                     HaloHsv;
  INT16                         HueDiff;
  INT8                          SaturationDiff;
  INT8                          ValueDiff;
  CHAR16                        Str[20];

  ASSERT (This != NULL);
  GetCurrentHaloHsv (&HaloHsv);
  GetCurrentMenuHsv (&MenuHsv);
  if (MenuHsv.Hue == 0 && MenuHsv.Saturation == 0 && MenuHsv.Value == 0) {
    return;
  }
  HueDiff        = (INT16) (MenuHsv.Hue - HaloHsv.Hue);
  SaturationDiff = (INT8)  (MenuHsv.Saturation - HaloHsv.Saturation);
  ValueDiff      = (INT8)  (MenuHsv.Value - HaloHsv.Value);
  UnicodeSPrint (Str, sizeof (Str), L"%d,%d,%d",  HueDiff, SaturationDiff, ValueDiff);
  Control = UiFindChildByName (This, L"FormHalo");
  UiSetAttribute (Control, L"hsvadjust", Str);
}

EFI_STATUS
UpdateHaloPos (
  IN DISPLAY_ENGINE_FRAME                    *This
  )
{
  EFI_STATUS                               Status;
  RECT                                     Field;
  UI_MANAGER                               *Manager;
  UI_CONTROL                               *Control;
  CHAR16                                   String[20];

  Status = GetInformationField (&Field);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Manager = ((UI_CONTROL *) This)->Manager;

  Control = Manager->FindControlByName (Manager, L"FormHalo");
  if (Control == NULL) {
    return EFI_NOT_FOUND;
  }

  UnicodeSPrint (String, sizeof (String), L"%d,%d,%d,%d", Field.left, Field.top, Field.right, Field.bottom);
  UiSetAttribute (Control, L"pos", String);

  return EFI_SUCCESS;
}

BOOLEAN
IsMetroPrivateHotKey (
  IN EFI_KEY_DATA               *KeyData
  )
{
  UINTN                         Index;

  if (KeyData == NULL) {
    return FALSE;
  }

  for (Index = 0; Index < sizeof (mPrivateHotKeyList) / sizeof (EFI_INPUT_KEY); Index++) {
    if (KeyData->Key.ScanCode    == mPrivateHotKeyList[Index].ScanCode &&
        KeyData->Key.UnicodeChar == mPrivateHotKeyList[Index].UnicodeChar &&
        KeyData->KeyState.KeyShiftState == EFI_SHIFT_STATE_VALID) {
      return TRUE;
    }
  }

  return FALSE;
}

VOID
LocalMetroRegisterHotkey (
  HWND                          Wnd
  )
{
  INT32                         Id;

  Id = 1;
  while (UnregisterHotKey (Wnd, Id)) {
    Id++;
  }

  Id = 1;
  MwRegisterHotKey (Wnd, Id++,  0, VK_ESCAPE);
}

LRESULT
CALLBACK
DisplayEngineProc (
  HWND Wnd,
  UINT Msg,
  WPARAM WParam,
  LPARAM LParam
  )
{
  DISPLAY_ENGINE_FRAME            *This;
  UI_MANAGER                    *Manager;
  UI_CONTROL                    *Control;
  UINTN                         VKCode;
  HOT_KEY_INFO                  HotKeyInfo;

  This    = (DISPLAY_ENGINE_FRAME *) GetWindowLongPtr (Wnd, 0);
  if (This == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }
  Control = (UI_CONTROL *) This;

  switch ( Msg ) {

  case WM_CREATE:
    This = (DISPLAY_ENGINE_FRAME *) AllocateZeroPool (sizeof (DISPLAY_ENGINE_FRAME));
    if (This != NULL) {
      CONTROL_CLASS(This) = (UI_CONTROL_CLASS *) GetClassLongPtr (Wnd, 0);
      SetWindowLongPtr (Wnd, 0, (INTN)This);
      SendMessage (Wnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, Msg, WParam,LParam);
    //
    // create UiContainer as root
    //
    Manager = Control->Manager;
    SetWindowLongPtr (Wnd, GWL_EXSTYLE, GetWindowLongPtr (Control->Wnd, GWL_EXSTYLE) & (~WS_EX_NOACTIVATE));
    if (gFB->CurrentP != NULL) {
      XmlCreateControl (mFrontPageChilds, Control);
      LocalMetroSetComponent (This);
      SetPanelPosFromVfcf (This);
    } else {
      XmlCreateControl (mDialogPanelChilds, Control);
    }
    break;

  case WM_SETFOCUS:
    if (gFB->CurrentP == NULL) {
      break;
    }
    if (gLastFocus != NULL) {
      if (gLastFocus == This->H2OSetupMenuPanel->Wnd) {
        SetFocus (gLastFocus);
        break;
      } else if (gLastFocus == This->H2OSetupPagePanel->Wnd) {
        SetFocus (gLastFocus);
        break;
      }
    }
    if (IsWindowVisible(This->H2OSetupMenuPanel->Wnd)) {
      SetFocus (This->H2OSetupMenuPanel->Wnd);
    } else {
      SetFocus (This->H2OSetupPagePanel->Wnd);
    }
    break;

  case WM_HOTKEY:
    if (GetWindowLongPtr (Wnd, GWL_STYLE) & WS_DISABLED) {
      break;
    }

    VKCode = (UINTN) HIWORD(LParam);

    switch (VKCode) {

    case VK_ESCAPE:
      ZeroMem (&HotKeyInfo, sizeof (HOT_KEY_INFO));
      HotKeyInfo.HotKeyAction = HotKeyDiscardAndExit;
      HotKeyFunc (&HotKeyInfo);
      break;
    }
    break;

  case FB_NOTIFY_SELECT_Q:
    if (gFB->CurrentP != NULL) {
      This->H2OSetupMenuPanel->Class->WndProc (This->H2OSetupMenuPanel->Wnd, Msg, WParam, LParam);
      This->H2OSetupPagePanel->Class->WndProc (This->H2OSetupPagePanel->Wnd, Msg, WParam, LParam);
      This->H2OFormTitlePanel->Class->WndProc (This->H2OFormTitlePanel->Wnd, Msg, WParam, LParam);
      This->H2OHelpTextPanel->Class->WndProc (This->H2OHelpTextPanel->Wnd, Msg, WParam, LParam);
      This->H2OHotkeyPanel->Class->WndProc (This->H2OHotkeyPanel->Wnd, Msg, WParam, LParam);
      This->H2OOwnerDrawPanel->Class->WndProc (This->H2OOwnerDrawPanel->Wnd, Msg, WParam, LParam);
    }
    break;

  case FB_NOTIFY_REPAINT:
    if (gFB->CurrentP != NULL) {
      This->H2OSetupMenuPanel->Class->WndProc (This->H2OSetupMenuPanel->Wnd, Msg, WParam, LParam);
      This->H2OSetupPagePanel->Class->WndProc (This->H2OSetupPagePanel->Wnd, Msg, WParam, LParam);
      This->H2OFormTitlePanel->Class->WndProc (This->H2OFormTitlePanel->Wnd, Msg, WParam, LParam);
      This->H2OHelpTextPanel->Class->WndProc (This->H2OHelpTextPanel->Wnd, Msg, WParam, LParam);
      This->H2OHotkeyPanel->Class->WndProc (This->H2OHotkeyPanel->Wnd, Msg, WParam, LParam);
      This->H2OOwnerDrawPanel->Class->WndProc (This->H2OOwnerDrawPanel->Wnd, Msg, WParam, LParam);
      UpdateHaloPos (This);
      UpdateHaloColor (This);
      LocalMetroRegisterHotkey (Wnd);
      SetFocus (Wnd);
    }
    break;

  case FB_NOTIFY_REFRESH_Q:
    if (gFB->CurrentP != NULL) {
      This->H2OSetupPagePanel->Class->WndProc (This->H2OSetupPagePanel->Wnd, Msg, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CLICK:
  case UI_NOTIFY_CARRIAGE_RETURN:
    LocalMetroProcessUiNotify (This, Msg, (UI_CONTROL *)WParam);
    break;

  case WM_DESTROY:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, Msg, WParam,LParam);
    break;

  case WM_CLOSE:
    DestroyWindow (Wnd);
    break;

  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, Msg, WParam,LParam);
  }
  return 0;
}

DISPLAY_ENGINE_FRAME_CLASS *
GetDisplayEngineFrameClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), DISPLAY_ENGINE_CLASS_NAME, (UI_CONTROL_CLASS *)GetFrameClass());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc = DisplayEngineProc;

  return CURRENT_CLASS;
}



UI_GET_CLASS mGetClassTable[] = {
               (UI_GET_CLASS) GetControlClass          ,
               (UI_GET_CLASS) GetTextureClass          ,

               (UI_GET_CLASS) GetLabelClass            ,
               (UI_GET_CLASS) GetButtonClass           ,
               (UI_GET_CLASS) GetSwitchClass           ,

               (UI_GET_CLASS) GetEditClass             ,
               (UI_GET_CLASS) GetScrollBarClass        ,
               (UI_GET_CLASS) GetVerticalLayoutClass   ,
               (UI_GET_CLASS) GetHorizontalLayoutClass ,

               (UI_GET_CLASS) GetListViewClass,
               (UI_GET_CLASS) GetNumberPickerClass     ,

               (UI_GET_CLASS) GetDialogClass           ,
               (UI_GET_CLASS) GetFrameClass            ,


               (UI_GET_CLASS) GetFrontPageItemClass    ,
               (UI_GET_CLASS) GetSetupPageItemClass    ,
               (UI_GET_CLASS) GetSetupMenuItemClass    ,
               (UI_GET_CLASS) GetDateItemClass         ,
               (UI_GET_CLASS) GetTimeItemClass         ,
               (UI_GET_CLASS) GetHotkeyItemClass       ,

               (UI_GET_CLASS) GetDisplayEngineFrameClass,

               (UI_GET_CLASS) GetH2OFormTitlePanelClass,
               (UI_GET_CLASS) GetH2OHotkeyPanelClass,
               (UI_GET_CLASS) GetH2OHelpTextPanelClass,
               (UI_GET_CLASS) GetH2OOwnerDrawPanelClass,
               (UI_GET_CLASS) GetH2OSetupMenuPanelClass,
               (UI_GET_CLASS) GetH2OSetupPagePanelClass,

               (UI_GET_CLASS) GetScrollViewClass,

               NULL,
            };


EFI_STATUS
InitializeGUI (
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput
  )
{
  EFI_STATUS                    Status;

  Status = gBS->LocateProtocol (
                  &gSetupMouseProtocolGuid,
                  NULL,
                  (VOID **) &mSetupMouse
                  );
  if (EFI_ERROR (Status)) {
    mSetupMouse = NULL;
  }

  AddHiiImagePackage ();

  GdAddGopDevice (GraphicsOutput);

  if (MwOpen() < 0) {
    return EFI_INVALID_PARAMETER;
  }

  RegisterClassTable (mGetClassTable);

  return EFI_SUCCESS;
}

EFI_STATUS
InitializeWindows (
  VOID
  )
{
  MSG                                      Msg;

  if (gWnd != NULL) {
    return EFI_ALREADY_STARTED;
  }

  //
  // CreateWindow
  //
  gWnd = CreateWindowEx (
           0,
           DISPLAY_ENGINE_CLASS_NAME,
           L"Insyde H2O",
           WS_OVERLAPPED | WS_VISIBLE,
           0, 0, GetSystemMetrics (SM_CXSCREEN), GetSystemMetrics (SM_CYSCREEN),
           NULL,
           NULL,
           NULL,
           NULL
           );
  //
  // Process Message
  //
  while (PeekMessage (&Msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage (&Msg);
    DispatchMessage (&Msg);
  }

  return EFI_SUCCESS;
}



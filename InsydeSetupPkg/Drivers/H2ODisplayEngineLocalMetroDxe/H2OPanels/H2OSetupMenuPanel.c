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
#include "H2ODisplayEngineLocalMetro.h"
#include "UiControls.h"
#include "H2OPanels.h"
#include "MetroUi.h"

STATIC H2O_SETUP_MENU_PANEL_CLASS        *mH2OSetupMenuPanelClass = NULL;
#define CURRENT_CLASS                    mH2OSetupMenuPanelClass

extern HWND                     gLastFocus;

VOID
SetupMenuItemSelected (
  UI_LIST_VIEW                  *This,
  UI_CONTROL                    *Child,
  UINT32                        Index
  )
{
  EFI_STATUS                    Status;
  H2O_FORM_BROWSER_SM           *SetupMenuData;
  SETUP_MENU_INFO               CurrentSetupMenuInfo;

  ASSERT (gFB->CurrentP != NULL);


  Status = gFB->GetSMInfo (gFB, &SetupMenuData);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR(Status)) {
    return ;
  }

  ASSERT (Index < SetupMenuData->NumberOfSetupMenus);
  if (Index >= SetupMenuData->NumberOfSetupMenus) {
    goto Done;
  }

  Status = GetSetupMenuInfoByPage (gFB->CurrentP, &CurrentSetupMenuInfo);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if (SetupMenuData->SetupMenuInfoList[Index].PageId == CurrentSetupMenuInfo.PageId) {
    goto Done;
  }

  SendSelectPNotify (SetupMenuData->SetupMenuInfoList[Index].PageId);

Done:
  FreeSetupMenuData (SetupMenuData);
}

VOID
SetupMenuItemClick (
  UI_LIST_VIEW                  *This,
  UI_CONTROL                    *Item,
  UINT32                        Index
  )
{
  EFI_STATUS                    Status;
  H2O_FORM_BROWSER_SM           *SetupMenuData;
  SETUP_MENU_INFO               CurrentSetupMenuInfo;

  ASSERT (gFB->CurrentP != NULL);


  Status = gFB->GetSMInfo (gFB, &SetupMenuData);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR(Status)) {
    return ;
  }

  ASSERT (Index < SetupMenuData->NumberOfSetupMenus);
  if (Index >= SetupMenuData->NumberOfSetupMenus) {
    goto Done;
  }

  Status = GetSetupMenuInfoByPage (gFB->CurrentP, &CurrentSetupMenuInfo);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if (SetupMenuData->SetupMenuInfoList[Index].PageId == CurrentSetupMenuInfo.PageId) {
    goto Done;
  }

  SendSelectPNotify (SetupMenuData->SetupMenuInfoList[Index].PageId);

Done:
  FreeSetupMenuData (SetupMenuData);
}

EFI_STATUS
RefreshSetupMenuList (
  H2O_SETUP_MENU_PANEL          *This
  )
{
  UINTN                         Index;
  EFI_STATUS                    Status;
  H2O_FORM_BROWSER_SM           *SetupMenuData;
  SETUP_MENU_INFO               *SetupMenuInfo;
  SETUP_MENU_INFO               CurrentSetupMenuInfo;
  UI_CONTROL                    *MenuControl;
  CHAR16                        Str[20];


  if (!NeedShowSetupMenu ()) {
    UiSetAttribute (This, L"visibility", L"false");
    return EFI_SUCCESS;
  } else {
    UiSetAttribute (This, L"visibility", L"true");
  }

  //
  // remove old items
  //
  CONTROL_CLASS (This)->RemoveAllChild ((UI_CONTROL *)This);

  ((UI_LIST_VIEW *)This)->OnItemClick = SetupMenuItemClick;
  ((UI_LIST_VIEW *)This)->OnItemSelected = SetupMenuItemSelected;


  Status = gFB->GetSMInfo (gFB, &SetupMenuData);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (SetupMenuData->NumberOfSetupMenus == 0) {
    FreePool (SetupMenuData);
    return EFI_UNSUPPORTED;
  }

  Status = GetSetupMenuInfoByPage (gFB->CurrentP, &CurrentSetupMenuInfo);
  if (EFI_ERROR (Status)) {
    ZeroMem (&CurrentSetupMenuInfo, sizeof (SETUP_MENU_INFO));
  }

  for (Index = 0; Index < SetupMenuData->NumberOfSetupMenus; Index++) {

    SetupMenuInfo = &SetupMenuData->SetupMenuInfoList[Index];

    MenuControl = CreateControl (L"SetupMenuItem", (UI_CONTROL*) This);
    UnicodeSPrint (Str, sizeof (Str), L"0x%p", SetupMenuInfo);
    UiSetAttribute (MenuControl, L"setupmenuinfo", Str);

    CONTROL_CLASS(This)->AddChild ((UI_CONTROL *)This, MenuControl);

    //
    // performItemClick to select
    //
    if (SetupMenuData->SetupMenuInfoList[Index].PageId == CurrentSetupMenuInfo.PageId) {
      LIST_VIEW_CLASS (This)->SetSelection (
                                (UI_LIST_VIEW *)This,
                                (INT32)Index,
                                TRUE
                                );
    }
  }

  FreeSetupMenuData (SetupMenuData);
  return EFI_SUCCESS;
}

LRESULT
EFIAPI
H2OSetupMenuPanelProc (
  HWND   Wnd,
  UINT32 Msg,
  WPARAM WParam,
  LPARAM LParam
  )
{
  H2O_SETUP_MENU_PANEL          *This;
  UI_CONTROL                    *Control;


  This = (H2O_SETUP_MENU_PANEL *) GetWindowLongPtr (Wnd, 0);
  if (This == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }
  Control = (UI_CONTROL *)This;

  switch (Msg) {

  case WM_CREATE:
    This = (H2O_SETUP_MENU_PANEL *) AllocateZeroPool (sizeof (H2O_SETUP_MENU_PANEL));
    if (This != NULL) {
      CONTROL_CLASS (This) = (UI_CONTROL_CLASS *) GetClassLongPtr (Wnd, 0);
      SetWindowLongPtr (Wnd, 0, (INTN)This);
      SendMessage (Wnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, UI_NOTIFY_CREATE, WParam, LParam);
    break;

  case FB_NOTIFY_REPAINT:
    RefreshSetupMenuList (This);
    break;

  case WM_SETFOCUS:
    gLastFocus = Wnd;
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, Msg, WParam, LParam);
    break;

  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, Msg, WParam, LParam);

  }
  return 0;

}


H2O_SETUP_MENU_PANEL_CLASS *
EFIAPI
GetH2OSetupMenuPanelClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), L"H2OSetupMenuPanel", (UI_CONTROL_CLASS *) GetListViewClass());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc = H2OSetupMenuPanelProc;

  return CURRENT_CLASS;
}


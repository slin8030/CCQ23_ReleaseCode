/** @file

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

#include "H2ODisplayEngineLocalText.h"
#include "LTDEControl.h"
#include "LTDEPanels.h"
#include "LTDEQuestions.h"
#include "LTDEPrint.h"
#include "LTDEMisc.h"
#include <Guid/ZeroGuid.h>


EFI_STATUS
CheckFBHotKey (
  IN  H2O_DISPLAY_ENGINE_USER_INPUT_DATA      *UserInputData,
  OUT HOT_KEY_INFO                            *SelectedHotKey
  )
{
  if (UserInputData == NULL || SelectedHotKey == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (mDEPrivate->FBProtocol->CurrentP->HotKeyInfo == NULL) {
    return EFI_NOT_FOUND;
  }

  return GetSelectedHotKeyInfoByKeyData (&UserInputData->KeyData, mDEPrivate->FBProtocol->CurrentP->HotKeyInfo, SelectedHotKey);
}

EFI_STATUS
CheckDEHotKeyInVfcf (
  IN EFI_KEY_DATA                             *KeyData
  )
{
  EFI_STATUS                                  Status;
  UINT32                                      HotKeyCount;
  HOT_KEY_INFO                                *HotKey;
  H2O_FORM_BROWSER_SM                         *SetupMenuData;
  EFI_GUID                                    FormsetGuid;
  EFI_FORM_ID                                 FormId;
  EFI_QUESTION_ID                             QuestionId;
  HOT_KEY_INFO                                SelectedHotKeyInfo;

  if (KeyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (mDEPrivate->Layout == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // Based on current (Formset, Form, QuestionId) to get the hot key list of display engine.
  //
  Status = mDEPrivate->FBProtocol->GetSMInfo (mDEPrivate->FBProtocol, &SetupMenuData);
  if (!EFI_ERROR (Status)) {
    CopyGuid (&FormsetGuid, &SetupMenuData->FormSetGuid);
    FreeSetupMenuData (SetupMenuData);
  } else {
    ZeroMem (&FormsetGuid, sizeof (EFI_GUID));
  }
  FormId     = (mDEPrivate->FBProtocol->CurrentP != NULL) ? (EFI_FORM_ID) (mDEPrivate->FBProtocol->CurrentP->PageId & 0xFFFF) : 0;
  QuestionId = (mDEPrivate->FBProtocol->CurrentQ != NULL) ? mDEPrivate->FBProtocol->CurrentQ->QuestionId : 0;

  Status = GetHotKeyListByLayoutInfo (
             mDEPrivate->Layout,
             &FormsetGuid,
             FormId,
             QuestionId,
             &HotKeyCount,
             &HotKey
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = GetSelectedHotKeyInfoByKeyData (KeyData, HotKey, &SelectedHotKeyInfo);
  if (!EFI_ERROR (Status)) {
    SendEvtByHotKey (&SelectedHotKeyInfo);
  }

  FreePool (HotKey);
  return Status;
}

STATIC
EFI_STATUS
CheckDEHotKey (
  IN H2O_DISPLAY_ENGINE_USER_INPUT_DATA       *UserInputData
  )
{
  EFI_STATUS                                  Status;

  if (UserInputData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = CheckDEHotKeyInVfcf (&UserInputData->KeyData);
  if (!EFI_ERROR (Status)) {
    return Status;
  }

  Status = SetupPagePanelProcessHotKey (&UserInputData->KeyData.Key);
  if (!EFI_ERROR (Status)) {
    return Status;
  }

  Status = HelpTextPanelProcessUserInput (&UserInputData->KeyData.Key);
  if (!EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_UNSUPPORTED;
}

EFI_STATUS
SendEvtByHotKey (
  IN HOT_KEY_INFO                             *HotKey
  )
{
  H2O_LTDE_PANEL                              *SetupMenuPanel;
  H2O_LTDE_PANEL                              *SetupPagePanel;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_CONTROL                            *Control;

  if (HotKey == 0) {
    return EFI_INVALID_PARAMETER;
  }

  switch (HotKey->HotKeyAction) {

  case HotKeySelectPreviousItem:
  case HotKeySelectNextItem:
    SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
    if (SetupPagePanel != NULL) {
      SetupPagePanelProcessSelectQHotKey (SetupPagePanel, (BOOLEAN) (HotKey->HotKeyAction == HotKeySelectNextItem));
    }
    break;

  case HotKeyEnter:
    SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
    if (SetupPagePanel == NULL) {
      break;
    }

    if (HotKey->HotKeyTargetQuestionId == 0) {
      Control = SetupPagePanel->SelectedControl;
    } else {
      Control = NULL;
      ContentItem = GetPanelItem (SetupPagePanel, LTDE_PANEL_ITEM_ID_CONTENT);
      if (ContentItem != NULL) {
        Control = GetControlByQuestionId (
                    ContentItem->ControlList,
                    ContentItem->ControlCount,
                    HotKey->HotKeyTargetQuestionId,
                    NULL
                    );
      }
      if (Control != NULL &&
          (SetupPagePanel->SelectedControl == NULL ||
           SetupPagePanel->SelectedControl->QuestionId != Control->QuestionId)) {
        SendSelectQNotify (Control->PageId, Control->QuestionId, Control->IfrOpCode);
      }
    }
    if (Control != NULL) {
      SetupPagePanelProcessOpenQHotKey (SetupPagePanel, Control);
    }
    break;

  case HotKeyModifyPreviousValue:
  case HotKeyModifyNextValue:
    SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
    if (SetupPagePanel != NULL) {
      SetupPagePanelProcessChangeQHotKey (SetupPagePanel, (BOOLEAN) (HotKey->HotKeyAction == HotKeyModifyNextValue));
    }
    break;

  case HotKeySelectPreviousMenu:
  case HotKeySelectNextMenu:
    SetupMenuPanel = GetPanel (H2O_PANEL_TYPE_SETUP_MENU);
    if (SetupMenuPanel != NULL) {
      SetupMenuPanelProcessSelectQHotKey (SetupMenuPanel, (BOOLEAN) (HotKey->HotKeyAction == HotKeySelectNextMenu));
    }
    break;

  case HotKeySaveAndExit:
    SendSubmitExitNotify ();
    break;

  case HotKeyDiscardAndExit:
    SendDiscardExitNotify ();
    break;

  case HotKeyShowHelp:
    SendShowHelpNotify ();
    break;

  case HotKeySave:
  case HotKeyLoadDefault:
  case HotKeyDiscard:
  case HotKeyGoTo:
  case HotKeyCallback:
  case HotKeySetQuestionValue:
    SendHotKeyNotify (HotKey);
    break;

  default:
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetLayout (
  H2O_LAYOUT_INFO                                **Layout
  )
{
  UINT32                                         LayoutId;
  EFI_STATUS                                     Status;
  H2O_FORM_BROWSER_SM                            *SetupMenuData;
  EFI_GUID                                       FormSetGuid;

  Status = mDEPrivate->FBProtocol->GetSMInfo (mDEPrivate->FBProtocol, &SetupMenuData);
  if (!EFI_ERROR (Status)) {
    CopyGuid (&FormSetGuid, &SetupMenuData->FormSetGuid);
    FreeSetupMenuData (SetupMenuData);
  } else {
    CopyGuid (&FormSetGuid, &gZeroGuid);
  }


  LayoutId = 0;
  Status = GetLayoutIdByGuid (&FormSetGuid, &LayoutId);
  if (EFI_ERROR (Status)) {
    Status = GetLayoutIdByGuid (&gZeroGuid, &LayoutId);
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = GetLayoutById (
             LayoutId,
             &mDEPrivate->DisplayEngine.Id,
             Layout
             );
  if (EFI_ERROR (Status)) {
    Status = GetLayoutIdByGuid (&gZeroGuid, &LayoutId);
    ASSERT_EFI_ERROR (Status);

    Status = GetLayoutById (
               LayoutId,
               &mDEPrivate->DisplayEngine.Id,
               Layout
               );
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

STATIC
EFI_STATUS
UpdatePanelFromVfcf (
  IN     H2O_PANEL_INFO                       *VfcfPanelInfo,
  IN     RECT                                 *ScreenField,
  IN OUT H2O_LTDE_PANEL                       *Panel
  )
{
  UINT32                                      OrderFlag;

  if (VfcfPanelInfo == NULL || ScreenField == NULL || Panel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  OrderFlag = GetResolution (VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL);

  Panel->VfcfPanelInfo   = VfcfPanelInfo;
  Panel->Vertical        = ((OrderFlag & H2O_PANEL_ORDER_FLAG_HORIZONTAL_FIRST) != 0) ? FALSE : TRUE;
  Panel->Visible         = IsVisibility (VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL);
  Panel->BorderLineWidth = GetBorderWidth (VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL);
  GetPanelField (VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL, ScreenField, &Panel->PanelField);
  GetPanelColorAttribute (VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL, &Panel->ColorAttribute);

  return EFI_SUCCESS;
}

/**
 Determine if user is editing question at menu or not

 @retval TRUE                User is editing question at menu.
 @retval FALSE               User is not editing question at menu.
*/
BOOLEAN
IsUserEditingAtMenu (
  VOID
  )
{
  H2O_LTDE_PANEL             *SetupPagePanel;
  BOOLEAN                    Result;

  Result = FALSE;

  if (mDEPrivate->DEStatus != DISPLAY_ENGINE_STATUS_AT_MENU) {
    return Result;
  }

  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel == NULL || SetupPagePanel->SelectedControl == NULL) {
    return Result;
  }

  switch (SetupPagePanel->SelectedControl->Operand) {

  case EFI_IFR_DATE_OP:
  case EFI_IFR_TIME_OP:
    Result = IsDateTimeOpCodeBeingModified (SetupPagePanel->SelectedControl);
    break;
  }

  return Result;
}

/**
 Initialize layout definition into PanelList of private data

 @retval EFI_SUCCESS                Initialize layout definition successfully
 @retval EFI_NOT_FOUND              There is no panel information for layout
 @retval Other                      Fail to get screen field or layout data
**/
STATIC
EFI_STATUS
InitLayout (
  VOID
  )
{
  EFI_STATUS                                     Status;
  RECT                                           ScreenField;
  H2O_LAYOUT_INFO                                *Layout;
  LIST_ENTRY                                     *Link;
  H2O_PANEL_INFO                                 *VfcfPanelInfo;
  H2O_LTDE_PANEL                                 *Panel;

  Status = GetScreenField (&ScreenField);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = GetLayout (&Layout);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (IsListEmpty (&Layout->PanelListHead)) {
    return EFI_NOT_FOUND;
  }

  Link = GetFirstNode (&Layout->PanelListHead);
  while (!IsNull (&Layout->PanelListHead, Link)) {
    VfcfPanelInfo = H2O_PANEL_INFO_NODE_FROM_LINK (Link);
    Link          = GetNextNode (&Layout->PanelListHead, Link);

    Panel = GetPanel (VfcfPanelInfo->PanelType);
    if (Panel == NULL) {
      Panel = CreatePanel ();
    }
    if (Panel != NULL) {
      UpdatePanelFromVfcf (VfcfPanelInfo, &ScreenField, Panel);
    }
  }

  mDEPrivate->Layout = Layout;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DERefreshQuestion (
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  )
{
  EFI_STATUS                                  Status;
  BOOLEAN                                     IsHighlight;
  H2O_LTDE_PANEL                              *SetupPagePanel;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_CONTROL                            *Control;
  H2O_FORM_BROWSER_S                          *Statement;

  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel == NULL) {
    return EFI_NOT_FOUND;
  }

  ContentItem = GetPanelItem (SetupPagePanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem == NULL) {
    return EFI_NOT_FOUND;
  }

  IsHighlight = IsCurrentHighlight (PageId, QuestionId, IfrOpCode);
  if (IsHighlight && IsDateTimeOpCodeBeingModified (SetupPagePanel->SelectedControl)) {
    return EFI_ABORTED;
  }

  Control = GetControlByQuestionId (
              ContentItem->ControlList,
              ContentItem->ControlCount,
              QuestionId,
              IfrOpCode
              );
  if (Control == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = mDEPrivate->FBProtocol->GetSInfo (mDEPrivate->FBProtocol, PageId, Control->StatementId, &Statement);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FreeControlInfo (Control);
  UpdateSetupPageControl (SetupPagePanel, ContentItem, Statement, Control);
  SafeFreePool ((VOID **) &Statement);

  DisplaySetupPagePanelContentItemControl (SetupPagePanel, ContentItem, Control);
  if (!IsHighlight) {
    return EFI_SUCCESS;
  }

  Status = InitHelpTextPanel ();
  if (!EFI_ERROR (Status)) {
    DisplayHelpTextPanel ();
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DERefreshSetupPageAndHelp (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA          *Private
  )
{
  EFI_STATUS                                  Status;

  Private->DEStatus = DISPLAY_ENGINE_STATUS_AT_MENU;

  Status = InitSetupPagePanel ();
  if (!EFI_ERROR (Status)) {
    DisplaySetupPagePanel ();
  }

  Status = InitHelpTextPanel ();
  if (!EFI_ERROR (Status)) {
    DisplayHelpTextPanel ();
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DEOpenLayout (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA         *Private
  )
{
  EFI_STATUS                                 Status;

  Status = InitLayout ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = DisplayLayout ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DEOpenPage (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA         *Private
  )
{
  EFI_STATUS                                 Status;

  Status = InitTitlePanel ();
  if (!EFI_ERROR (Status)) {
    DisplayTitlePanel ();
  }

  Status = InitSetupMenuPanel ();
  if (!EFI_ERROR (Status)) {
    DisplaySetupMenuPanel ();
  }

  Status = InitHotKeyPanel ();
  if (!EFI_ERROR (Status)) {
    DisplayHotKeyPanel ();
  }

  //
  // Display Current Page Menu
  //
  Status = DERefreshSetupPageAndHelp (Private);

  return Status;
}

STATIC
EFI_STATUS
DEShutPage (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA             *Private
  )
{
  LIST_ENTRY                                     *PanelLink;
  H2O_LTDE_PANEL                                 *Panel;

  if (IsListEmpty (&Private->PanelListHead)) {
    return EFI_NOT_FOUND;
  }

  PanelLink = GetFirstNode (&Private->PanelListHead);
  while (!IsNull (&Private->PanelListHead, PanelLink)) {
    Panel     = H2O_LTDE_PANEL_FROM_LINK (PanelLink);
    PanelLink = GetNextNode (&Private->PanelListHead, PanelLink);

    switch (Panel->VfcfPanelInfo->PanelType) {

    case H2O_PANEL_TYPE_HELP_TEXT:
    case H2O_PANEL_TYPE_QUESTION:
//      FreePanel (Panel);
      break;
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DESelectQuestion (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA          *Private,
  IN H2O_DISPLAY_ENGINE_EVT_SELECT_Q          *SelectQ
  )
{
  EFI_STATUS                                  Status;
  H2O_LTDE_PANEL                              *SetupPagePanel;

  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel == NULL) {
    return EFI_SUCCESS;
  }

  Status = SetupPagePanelProcessSelectQEvt (SetupPagePanel, SelectQ);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = InitHelpTextPanel ();
  if (!EFI_ERROR (Status)) {
    DisplayHelpTextPanel ();
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DEShutQuestion (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify
  )
{
  EFI_STATUS                                  Status;

  Status = ProcessEvtInQuestionFunc ((H2O_DISPLAY_ENGINE_EVT *) Notify, NULL);
  mDEPrivate->DEStatus = DISPLAY_ENGINE_STATUS_AT_MENU;

  return Status;
}

EFI_STATUS
DEChangingQuestion (
  IN       H2O_FORM_BROWSER_PROTOCOL          *FBProtocol,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify
  )
{
  EFI_STATUS                                  Status;

  Status = ProcessEvtInQuestionFunc (Notify, NULL);
  if (Status == EFI_NOT_FOUND) {
    Status = EFI_SUCCESS;
  }

  return Status;
}

STATIC
EFI_STATUS
DEOpenDialog (
  IN     H2O_FORM_BROWSER_PROTOCOL            *FBProtocol,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify
  )
{
  EFI_STATUS                                  Status;
  H2O_LTDE_PANEL                              *QuestionPanel;

  if (IsListEmpty (&mDEPrivate->PanelListHead)) {
    Status = InitLayout ();
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  QuestionPanel = GetPanel (H2O_PANEL_TYPE_QUESTION);
  if (QuestionPanel == NULL) {
    return EFI_NOT_FOUND;
  }

  QuestionPanelProcessOpenDEvt (&((H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify)->Dialog, QuestionPanel);

  mDEPrivate->DEStatus = DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG;
  Status = ProcessEvtInQuestionFunc (Notify, NULL);
  if (EFI_ERROR (Status)) {
    mDEPrivate->DEStatus = DISPLAY_ENGINE_STATUS_AT_MENU;
  }
  if (Status == EFI_NOT_FOUND) {
    Status = EFI_SUCCESS;
  }

  return Status;
}

STATIC
EFI_STATUS
DEShutDialog (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA   *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT            *Notify
  )
{
  EFI_STATUS                                 Status;

  Status = ProcessEvtInQuestionFunc (Notify, NULL);
  Private->DEStatus = DISPLAY_ENGINE_STATUS_AT_MENU;
  if (Status == EFI_NOT_FOUND) {
    Status = EFI_SUCCESS;
  }

  if (Private->FBProtocol->CurrentP != NULL) {
    DisplayLayout ();
    DEOpenPage (Private);
  }
  DEConOutSetAttribute (EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);

  return Status;
}

EFI_STATUS
DEExit (
  VOID
  )
{
  LIST_ENTRY                                  *Link;
  H2O_LTDE_PANEL                              *Panel;

  if (IsListEmpty (&mDEPrivate->PanelListHead)) {
    return EFI_NOT_FOUND;
  }

  Link = GetFirstNode (&mDEPrivate->PanelListHead);
  while (!IsNull (&mDEPrivate->PanelListHead, Link)) {
    Panel = H2O_LTDE_PANEL_FROM_LINK (Link);
    Link  = GetNextNode (&mDEPrivate->PanelListHead, Link);
    FreePanel (Panel);
    //
    // BUGBUG: verify free panel is safe.
    //
    RemoveEntryList (&Panel->Link);
    FreePool (Panel);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetUserInputData (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  OUT      H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData
  )
{
  H2O_DISPLAY_ENGINE_USER_INPUT_DATA          InputData;
  H2O_DISPLAY_ENGINE_EVT_REL_PTR_MOVE         *RelPtrMoveNotify;
  H2O_DISPLAY_ENGINE_EVT_ABS_PTR_MOVE         *AbsPtrMoveNotify;
  STATIC UINT32                               PreviousActiveButtons = 0;
  STATIC INT32                                RealMouseX = 0;
  STATIC INT32                                RealMouseY = 0;

  if (Notify == NULL || UserInputData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Read user input from input event
  //
  ZeroMem (&InputData, sizeof (H2O_DISPLAY_ENGINE_USER_INPUT_DATA));
  InputData.KeyData.KeyState.KeyShiftState  = EFI_SHIFT_STATE_VALID;
  InputData.KeyData.KeyState.KeyToggleState = EFI_TOGGLE_STATE_VALID;

  switch (Notify->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS:
    //
    // Keyboard
    //
    InputData.IsKeyboard = TRUE;
    CopyMem (&InputData.KeyData, &((H2O_DISPLAY_ENGINE_EVT_KEYPRESS *) Notify)->KeyData, sizeof (EFI_KEY_DATA));
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE:
    RelPtrMoveNotify = (H2O_DISPLAY_ENGINE_EVT_REL_PTR_MOVE *) Notify;
    //
    // Rel Mouse
    //
    if (RelPtrMoveNotify->State.LeftButton) {
      InputData.KeyData.Key.ScanCode    = SCAN_NULL;
      InputData.KeyData.Key.UnicodeChar = CHAR_CARRIAGE_RETURN;
    }
    if (RelPtrMoveNotify->State.RightButton) {
      InputData.KeyData.Key.ScanCode    = SCAN_ESC;
      InputData.KeyData.Key.UnicodeChar = CHAR_NULL;
    }
    RealMouseX += RelPtrMoveNotify->State.RelativeMovementX;
    RealMouseY += RelPtrMoveNotify->State.RelativeMovementY;
    TransferToTextModePosition (RealMouseX, RealMouseY, &InputData.CursorX, &InputData.CursorY);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE:
    AbsPtrMoveNotify = (H2O_DISPLAY_ENGINE_EVT_ABS_PTR_MOVE *) Notify;
    //
    // Abs Mouse
    //
    if ((AbsPtrMoveNotify->AbsPtrState.ActiveButtons & EFI_ABSP_TouchActive) == EFI_ABSP_TouchActive &&
        (AbsPtrMoveNotify->AbsPtrState.ActiveButtons & EFI_ABSP_TouchActive) != (PreviousActiveButtons & EFI_ABSP_TouchActive)) {
      //
      // Left Button
      //
      InputData.KeyData.Key.ScanCode    = SCAN_NULL;
      InputData.KeyData.Key.UnicodeChar = CHAR_CARRIAGE_RETURN;
    }
    if ((AbsPtrMoveNotify->AbsPtrState.ActiveButtons & EFI_ABS_AltActive) == EFI_ABS_AltActive &&
        (AbsPtrMoveNotify->AbsPtrState.ActiveButtons & EFI_ABS_AltActive) != (PreviousActiveButtons & EFI_ABS_AltActive)) {
      //
      // Right Button
      //
      InputData.KeyData.Key.ScanCode    = SCAN_ESC;
      InputData.KeyData.Key.UnicodeChar = CHAR_NULL;
    }
    PreviousActiveButtons = AbsPtrMoveNotify->AbsPtrState.ActiveButtons;
    RealMouseX = (INT32)AbsPtrMoveNotify->AbsPtrState.CurrentX;
    RealMouseY = (INT32)AbsPtrMoveNotify->AbsPtrState.CurrentY;
    TransferToTextModePosition (RealMouseX, RealMouseY, &InputData.CursorX, &InputData.CursorY);
    break;

  default:
    return EFI_UNSUPPORTED;
  }

  CopyMem (UserInputData, &InputData, sizeof (H2O_DISPLAY_ENGINE_USER_INPUT_DATA));

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ProcessMouseClick (
  IN H2O_DISPLAY_ENGINE_USER_INPUT_DATA       *UserInputData
  )
{
  EFI_STATUS                                  Status;
  LIST_ENTRY                                  *Link;
  H2O_LTDE_PANEL                              *SelectedPanel;
  H2O_LTDE_PANEL_ITEM                         *SelectedPanelItem;
  H2O_LTDE_CONTROL                            *SelectedControl;

  if (UserInputData == NULL ||
      UserInputData->IsKeyboard ||
      UserInputData->KeyData.Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsListEmpty (&mDEPrivate->PanelListHead)) {
    return EFI_NOT_FOUND;
  }

  SelectedPanel     = NULL;
  SelectedPanelItem = NULL;
  SelectedControl   = NULL;
  Link              = GetFirstNode (&mDEPrivate->PanelListHead);
  while (!IsNull (&mDEPrivate->PanelListHead, Link)) {
    SelectedPanel = H2O_LTDE_PANEL_FROM_LINK (Link);
    Link          = GetNextNode (&mDEPrivate->PanelListHead, Link);
    if (!IsPointOnField (&SelectedPanel->PanelField, UserInputData->CursorX, UserInputData->CursorY)) {
      continue;
    }

    Status = GetControlByMouse (SelectedPanel, UserInputData->CursorX, UserInputData->CursorY, &SelectedPanelItem, &SelectedControl);
    if (!EFI_ERROR (Status)) {
      break;
    }
  }
  if (SelectedControl == NULL) {
    return EFI_NOT_FOUND;
  }

  switch (SelectedPanel->VfcfPanelInfo->PanelType) {

  case H2O_PANEL_TYPE_SETUP_PAGE:
    return SetupPagePanelProcessMouseClick (SelectedPanel, SelectedPanelItem, SelectedControl, UserInputData);

  case H2O_PANEL_TYPE_SETUP_MENU:
    return SetupMenuPanelProcessMouseClick (SelectedPanel, SelectedPanelItem, SelectedControl);

  case H2O_PANEL_TYPE_HELP_TEXT:
    return HelpTextPanelProcessMouseClick (SelectedPanelItem, SelectedControl);

  case H2O_PANEL_TYPE_HOTKEY:
    return HotKeyPanelProcessMouseClick (SelectedPanel, SelectedPanelItem, SelectedControl, UserInputData);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ProcessInputEvtAtMenu (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData
  )
{
  EFI_STATUS                                  Status;
  HOT_KEY_INFO                                HotKey;

  Status = ProcessEvtInQuestionFunc (Notify, UserInputData);
  if (!EFI_ERROR (Status)) {
    return Status;
  }

  if (!UserInputData->IsKeyboard && UserInputData->KeyData.Key.ScanCode == SCAN_ESC) {
    UserInputData->IsKeyboard = TRUE;
  }

  if (UserInputData->IsKeyboard) {
    Status = CheckFBHotKey (UserInputData, &HotKey);
    if (!EFI_ERROR (Status)) {
      return SendEvtByHotKey (&HotKey);
    }

    CheckDEHotKey (UserInputData);
  } else {
    if (UserInputData->KeyData.Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      return ProcessMouseClick (UserInputData);
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ProcessInputEvtAtPopUpDialog (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData
  )
{
  EFI_STATUS                                  Status;

  Status = ProcessEvtInQuestionFunc (Notify, UserInputData);
  if (!EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  if (UserInputData->KeyData.Key.ScanCode == SCAN_ESC) {
    return SendShutDNotify ();
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ProcessInputEvt (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify
  )
{
  EFI_STATUS                                  Status;
  H2O_DISPLAY_ENGINE_USER_INPUT_DATA          UserInputData;


  Status = GetUserInputData (Notify, &UserInputData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  switch (Private->DEStatus) {

  case DISPLAY_ENGINE_STATUS_AT_MENU:
    Status = ProcessInputEvtAtMenu (Private, Notify, &UserInputData);
    break;

  case DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG:
    Status = ProcessInputEvtAtPopUpDialog (Private, Notify, &UserInputData);
    break;

  default:
    return EFI_UNSUPPORTED;
  }

  return Status;
}

EFI_STATUS
DEEventCallback (
  IN       H2O_DISPLAY_ENGINE_PROTOCOL        *This,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify
  )
{
  H2O_DISPLAY_ENGINE_PRIVATE_DATA             *Private;
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_REFRESH_Q            *RefreshQNotify;

  Private = H2O_DISPLAY_ENGINE_PRIVATE_DATA_FROM_PROTOCOL (This);

  if (IsListEmpty (&Private->ConsoleDevListHead)) {
    //
    // Do nothing
    //
    return EFI_SUCCESS;
  }

  FBProtocol = Private->FBProtocol;
  Status     = EFI_UNSUPPORTED;

  switch (Notify->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE:
    Status = ProcessInputEvt (Private, Notify);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_L:
    Status = DEOpenLayout (Private);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_P:
    if (IsUserEditingAtMenu ()) {
      break;
    }
    Status = DEShutPage (Private);
    Status = DEOpenPage (Private);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_D:
    Status = DEOpenDialog (FBProtocol, Notify);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_D:
    Status = DEShutDialog (Private, Notify);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_REFRESH:
    if (IsUserEditingAtMenu ()) {
      break;
    }
    Status = DERefreshSetupPageAndHelp (Private);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_REFRESH_Q:
    if (Private->DEStatus == DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG) {
      break;
    }

    RefreshQNotify = (H2O_DISPLAY_ENGINE_EVT_REFRESH_Q *) Notify;
    Status = DERefreshQuestion (RefreshQNotify->PageId, RefreshQNotify->QuestionId, RefreshQNotify->IfrOpCode);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_Q:
    Status = DESelectQuestion (Private, (H2O_DISPLAY_ENGINE_EVT_SELECT_Q *) Notify);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_Q:
    Status = DEShutQuestion (Notify);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q:
    Status = DEChangingQuestion (FBProtocol, Notify);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_EXIT:
    Status = DEExit ();
    break;

  default:
    ASSERT (FALSE);
    break;
  }

  return Status;
}


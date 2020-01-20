/** @file
  Initialization and display related functions for setup menu panel.

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "LTDEPanels.h"
#include "LTDEMisc.h"

#define LTDE_SETUP_MENU_PANEL_START_X            1

/**
 Set current position of content item if selected control is not in display range.

 @param[in]      SetupMenuPanel     Pointer to setup menu panel
 @param[in, out] ContentItem        Pointer to content item

 @retval EFI_SUCCESS                Set current position of content item successful.
 @retval EFI_INVALID_PARAMETER      SetupMenuPanel or ContentItem is NULL

**/
STATIC
EFI_STATUS
SetupMenuPanelSetCurrentPos (
  IN     H2O_LTDE_PANEL                       *SetupMenuPanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  )
{
  INT32                                       DisplayStart;
  INT32                                       DisplayEnd;

  if (SetupMenuPanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (SetupMenuPanel->SelectedControl == NULL) {
    ContentItem->CurrentPos = 0;
    return EFI_SUCCESS;
  }

  DisplayStart = ContentItem->CurrentPos;
  DisplayEnd   = ContentItem->CurrentPos + H2O_LTDE_FIELD_WIDTH (&ContentItem->ItemField) - 1;
  if (IN_RANGE (SetupMenuPanel->SelectedControl->ControlField.left , DisplayStart, DisplayEnd) &&
      IN_RANGE (SetupMenuPanel->SelectedControl->ControlField.right, DisplayStart, DisplayEnd)) {
    return EFI_SUCCESS;
  }

  //
  // BUGBUG: Temporarily set current position at start of current control.
  //
  ContentItem->CurrentPos = (ContentItem->Vertical) ? SetupMenuPanel->SelectedControl->ControlField.top :
                                                      SetupMenuPanel->SelectedControl->ControlField.left;

  return EFI_SUCCESS;
}

/**
 Initialize control list in content item

 @param[in]      SetupMenuPanel     Pointer to setup menu panel
 @param[in, out] ContentItem        Pointer to content item

 @retval EFI_SUCCESS                Set current position of content item successful.
 @retval EFI_INVALID_PARAMETER      SetupMenuPanel or ContentItem is NULL
 @retval EFI_UNSUPPORTED            Setup menu panel is vertical
 @retval EFI_OUT_OF_RESOURCES       Allocate pool fail

**/
STATIC
EFI_STATUS
InitSetupMenuPanelContentItemControlList (
  IN     H2O_LTDE_PANEL                       *SetupMenuPanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_SM                         *SetupMenuData;
  SETUP_MENU_INFO                             *SetupMenuInfo;
  UINT32                                      Index;
  UINT32                                      ControlCount;
  H2O_LTDE_CONTROL                            *ControlList;
  H2O_LTDE_CONTROL                            *Control;
  CHAR16                                      *ControlStr;
  INT32                                       DisplayWidth;
  INT32                                       StartX;
  BOOLEAN                                     CurrentPageIsRoot;
  SETUP_MENU_INFO                             CurrentSetupMenu;

  if (SetupMenuPanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem->ControlCount = 0;
  ContentItem->ControlList  = NULL;

  if (SetupMenuPanel->Vertical) {
    return EFI_UNSUPPORTED;
  }

  Status = mDEPrivate->FBProtocol->GetSMInfo (mDEPrivate->FBProtocol, &SetupMenuData);
  if (EFI_ERROR (Status) || SetupMenuData->NumberOfSetupMenus == 0) {
    return EFI_SUCCESS;
  }

  ControlCount = SetupMenuData->NumberOfSetupMenus;
  ControlList  = AllocateZeroPool (ControlCount * sizeof (H2O_LTDE_CONTROL));
  if (ControlList == NULL) {
    FreeSetupMenuData (SetupMenuData);
    return EFI_OUT_OF_RESOURCES;
  }

  Status = GetSetupMenuInfoByPage (mDEPrivate->FBProtocol->CurrentP, &CurrentSetupMenu);
  if (EFI_ERROR (Status)) {
    ZeroMem (&CurrentSetupMenu, sizeof(SETUP_MENU_INFO));
  }

  CurrentPageIsRoot = IsRootPage ();
  StartX            = LTDE_SETUP_MENU_PANEL_START_X;
  for (Index = 0; Index < ControlCount; Index++) {
    SetupMenuInfo = &SetupMenuData->SetupMenuInfoList[Index];

    Control                           = &ControlList[Index];
    Control->PageId                   = SetupMenuInfo->PageId;
    Control->Sequence                 = (UINT8) Index;
    Control->ControlStyle.PseudoClass = H2O_STYLE_PSEUDO_CLASS_SELECTABLE;

    ControlStr   = CatSPrint (NULL, L" %s ", (SetupMenuInfo->PageTitle != NULL) ? SetupMenuInfo->PageTitle : L"");
    DisplayWidth = (INT32) GetStringDisplayWidth (ControlStr);
    if (CurrentPageIsRoot || Control->PageId == CurrentSetupMenu.PageId) {
      Control->Selectable  = TRUE;
      Control->Text.String = ControlStr;
    } else {
      Control->Selectable  = FALSE;
      Control->Text.String = CreateString ((UINT32) DisplayWidth, ' ');
      FreePool (ControlStr);
    }

    if (Control->PageId == CurrentSetupMenu.PageId) {
      SetupMenuPanel->SelectedControl = Control;
    }

    ASSERT (GetStringHeight (Control->Text.String, (UINT32) -1) == 1);
    SetRect (&Control->ControlField, StartX, 0, StartX + DisplayWidth - 1, 0);

    StartX += DisplayWidth;
  }

  ContentItem->MaxPos       = StartX - 1;
  ContentItem->Vertical     = SetupMenuPanel->Vertical;
  ContentItem->ControlCount = ControlCount;
  ContentItem->ControlList  = ControlList;

  FreeSetupMenuData (SetupMenuData);

  return EFI_SUCCESS;
}

/**
 Initialize content item

 @param[in]      SetupMenuPanel     Pointer to setup menu panel
 @param[in, out] ContentItem        Pointer to content item

 @retval EFI_SUCCESS                Set current position of content item successful.
 @retval EFI_INVALID_PARAMETER      SetupMenuPanel or ContentItem is NULL

**/
STATIC
EFI_STATUS
InitSetupMenuPanelContentItem (
  IN     H2O_LTDE_PANEL                       *SetupMenuPanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  )
{
  if (SetupMenuPanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem->ItemId = LTDE_PANEL_ITEM_ID_CONTENT;
  ContentItem->Hidden = FALSE;

  CopyRect (&ContentItem->ItemField, &SetupMenuPanel->PanelField);
  InflateRect (&ContentItem->ItemField, -SetupMenuPanel->BorderLineWidth, -SetupMenuPanel->BorderLineWidth);

  SetupMenuPanel->SelectedControl = NULL;
  InitSetupMenuPanelContentItemControlList (SetupMenuPanel, ContentItem);
  SetupMenuPanelSetCurrentPos (SetupMenuPanel, ContentItem);

  return EFI_SUCCESS;
}

/**
 Initialize setup menu panel

 @retval EFI_SUCCESS                Set current position of content item successful.
 @retval EFI_NOT_FOUND              Setup menu panel is not found
 @retval EFI_OUT_OF_RESOURCES       Allocate pool fail

**/
EFI_STATUS
InitSetupMenuPanel (
  VOID
  )
{
  H2O_LTDE_PANEL                              *SetupMenuPanel;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  EFI_STATUS                                  Status;

  SetupMenuPanel = GetPanel (H2O_PANEL_TYPE_SETUP_MENU);
  if (SetupMenuPanel == NULL) {
    return EFI_NOT_FOUND;
  }

  ContentItem = AllocateZeroPool (sizeof (H2O_LTDE_PANEL_ITEM));
  if (ContentItem == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = InitSetupMenuPanelContentItem (SetupMenuPanel, ContentItem);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FreePanelItemList (SetupMenuPanel->ItemList, SetupMenuPanel->ItemCount);
  SetupMenuPanel->ItemCount = 1;
  SetupMenuPanel->ItemList  = ContentItem;

  return Status;
}

/**
 Display setup menu panel

 @retval EFI_SUCCESS                Display setup menu panel successfully
**/
EFI_STATUS
DisplaySetupMenuPanel (
  VOID
  )
{
  H2O_LTDE_PANEL                              *SetupMenuPanel;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;

  SetupMenuPanel = GetPanel (H2O_PANEL_TYPE_SETUP_MENU);
  if (SetupMenuPanel == NULL || !SetupMenuPanel->Visible) {
    return EFI_SUCCESS;
  }

  ContentItem = GetPanelItem (SetupMenuPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem != NULL) {
    DisplayPanelItem (SetupMenuPanel, ContentItem);
  }

  return EFI_SUCCESS;
}

/**
  Process select question hot key in setup menu panel.

  @param[in] SetupMenuPanel      Pointer to setup menu panel
  @param[in] GoNext              TRUE if go to next control

  @retval EFI_SUCCESS            Process select question hot key successful.
  @retval EFI_INVALID_PARAMETER  SetupMenuPanel is NULL.
  @retval EFI_ABORTED            Currently it is not in root page.

**/
EFI_STATUS
SetupMenuPanelProcessSelectQHotKey (
  IN H2O_LTDE_PANEL                           *SetupMenuPanel,
  IN BOOLEAN                                  GoNext
  )
{
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_CONTROL                            *SelectedControl;
  BOOLEAN                                     Loop;

  if (SetupMenuPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsRootPage ()) {
    return EFI_ABORTED;
  }

  ContentItem = GetPanelItem (SetupMenuPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem == NULL) {
    return EFI_SUCCESS;
  }

  Loop = TRUE;
  if (GoNext) {
    SelectedControl = GetNextSelectableControl (ContentItem, SetupMenuPanel->SelectedControl, Loop);
  } else {
    SelectedControl = GetPreviousSelectableControl (ContentItem, SetupMenuPanel->SelectedControl, Loop);
  }

  if (SelectedControl != NULL && SelectedControl != SetupMenuPanel->SelectedControl) {
    SetupMenuPanel->SelectedControl = SelectedControl;
    SendSelectPNotify (SelectedControl->PageId);
  }

  return EFI_SUCCESS;
}

/**
  Process mouse click in setup menu panel.

  @param[in] SetupMenuPanel      Pointer to setup menu panel
  @param[in] SelectedPanelItem   Pointer to selected panel item
  @param[in] SelectedControl     Pointer to selected control

  @retval EFI_SUCCESS            Process mouse click successful.
  @retval EFI_INVALID_PARAMETER  Input parameter is NULL.
  @retval EFI_ABORTED            Currently it is not in root page or selected control is not selectable.

**/
EFI_STATUS
SetupMenuPanelProcessMouseClick (
  IN H2O_LTDE_PANEL                           *SetupMenuPanel,
  IN H2O_LTDE_PANEL_ITEM                      *SelectedPanelItem,
  IN H2O_LTDE_CONTROL                         *SelectedControl
  )
{
  if (SetupMenuPanel == NULL || SelectedPanelItem == NULL || SelectedControl == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsRootPage () ||
      !SelectedControl->Selectable) {
    return EFI_ABORTED;
  }

  if (SetupMenuPanel->SelectedControl != NULL &&
      SetupMenuPanel->SelectedControl->PageId == SelectedControl->PageId) {
    return EFI_SUCCESS;
  }

  SetupMenuPanel->SelectedControl = SelectedControl;

  return SendSelectPNotify (SelectedControl->PageId);
}


/** @file
  Initialization and display related functions for help text panel.

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
#include "LTDEPrint.h"
#include "LTDEMisc.h"

typedef enum {
  LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION_PAGE_UP = 0,
  LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION_PAGE_DOWN,
  LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION_MAX,
} LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION;

typedef struct {
  UINT32                                      Action;
  EFI_INPUT_KEY                               InputKey;
  CHAR16                                      *String;
} LTDE_HELP_TEXT_PANEL_HOT_KEY;

LTDE_HELP_TEXT_PANEL_HOT_KEY                  mHelpTextPanelHotKeyList[] = {
  {LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION_PAGE_UP  , {SCAN_NULL, 'u'}, L"More (U/u)"},
  {LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION_PAGE_DOWN, {SCAN_NULL, 'd'}, L"More (D/d)"},
  };

/**
  Get help text panel string which is determined by current highlight control in setup page panel.

  @return The help text panel string pointer or NULL if not found.
**/
STATIC
CHAR16 *
GetHelpTextPanelStr (
  VOID
  )
{
  EFI_STATUS                                  Status;
  H2O_LTDE_PANEL                              *SetupPagePanel;
  H2O_FORM_BROWSER_S                          *Statement;
  CHAR16                                      *HelpTextPanelStr;

  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel == NULL || SetupPagePanel->SelectedControl == NULL) {
    return NULL;
  }

  Status = mDEPrivate->FBProtocol->GetSInfo (
                                     mDEPrivate->FBProtocol,
                                     SetupPagePanel->SelectedControl->PageId,
                                     SetupPagePanel->SelectedControl->StatementId,
                                     &Statement
                                     );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  HelpTextPanelStr = (Statement->Help != NULL) ? AllocateCopyPool (StrSize (Statement->Help), Statement->Help) : NULL;
  SafeFreePool ((VOID **) &Statement);

  return HelpTextPanelStr;
}

/**
  Based on current position, update fields of help text panel items (content, page up and page down).

  @param[in] HelpTextPanel       A pointer to the help text panel instance.

  @retval EFI_SUCCESS            Update fields successful.
  @retval EFI_INVALID_PARAMETER  HelpTextPanel is NULL.
  @retval EFI_NOT_FOUND          Content panel item is not found.
  @retval EFI_ABORTED            Field height of Content panel item is less than 3
**/
STATIC
EFI_STATUS
UpdateHelpTextPanelItemField (
  IN H2O_LTDE_PANEL                           *HelpTextPanel
  )
{
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_PANEL_ITEM                         *PageUpItem;
  H2O_LTDE_PANEL_ITEM                         *PageDownItem;

  if (HelpTextPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem = GetPanelItem (HelpTextPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem == NULL) {
    return EFI_NOT_FOUND;
  }

  if (H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) < 3) {
    return EFI_ABORTED;
  }

  PageUpItem = GetPanelItem (HelpTextPanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP);
  if (ContentItem->CurrentPos > 0) {
    if (PageUpItem != NULL && PageUpItem->Hidden) {
      PageUpItem->Hidden = FALSE;
      CopyRect (&PageUpItem->ItemField, &ContentItem->ItemField);
      PageUpItem->ItemField.bottom = PageUpItem->ItemField.top;
      ContentItem->ItemField.top++;
    }
  } else {
    if (PageUpItem != NULL && !PageUpItem->Hidden) {
      PageUpItem->Hidden = TRUE;
      ContentItem->ItemField.top--;
    }
  }

  PageDownItem = GetPanelItem (HelpTextPanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN);
  if (ContentItem->MaxPos >= ContentItem->CurrentPos + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField)) {
    if (PageDownItem != NULL && PageDownItem->Hidden) {
      PageDownItem->Hidden = FALSE;
      CopyRect (&PageDownItem->ItemField, &ContentItem->ItemField);
      PageDownItem->ItemField.top = PageDownItem->ItemField.bottom;
      ContentItem->ItemField.bottom--;
    }
  } else {
    if (PageDownItem != NULL && !PageDownItem->Hidden) {
      PageDownItem->Hidden = TRUE;
      ContentItem->ItemField.bottom++;
    }
  }

  return EFI_SUCCESS;
}

/**
  Initialize the control list in content item of help text panel.

  @param[in]      HelpTextPanel  A pointer to the help text panel instance.
  @param[in, out] ContentItem    A pointer to the content item instance.

  @retval EFI_SUCCESS            Initialize the control list successful.
  @retval EFI_INVALID_PARAMETER  HelpTextPanel or ContentItem is NULL.
  @retval EFI_OUT_OF_RESOURCES   Allocate pool fail.
**/
STATIC
EFI_STATUS
InitHelpTextPanelContentItemControlList (
  IN     H2O_LTDE_PANEL                       *HelpTextPanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  )
{
  H2O_LTDE_CONTROL                            *Control;

  if (HelpTextPanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Control = AllocateZeroPool (sizeof (H2O_LTDE_CONTROL));
  if (Control == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Control->Text.String = GetHelpTextPanelStr ();

  ContentItem->MaxPos       = GetStringHeight (Control->Text.String, H2O_LTDE_FIELD_WIDTH (&ContentItem->ItemField));
  ContentItem->MaxPos      -= ((ContentItem->MaxPos > 0) ? 1 : 0);
  ContentItem->ControlCount = 1;
  ContentItem->ControlList  = Control;

  return EFI_SUCCESS;
}

/**
  Initialize the content item of help text panel.

  @param[in]      HelpTextPanel  A pointer to the help text panel instance.
  @param[in, out] ContentItem    A pointer to the content item instance.

  @retval EFI_SUCCESS            Initialize the content item successful.
  @retval EFI_INVALID_PARAMETER  HelpTextPanel or ContentItem is NULL.
**/
STATIC
EFI_STATUS
InitHelpTextPanelContentItem (
  IN     H2O_LTDE_PANEL                       *HelpTextPanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  )
{
  if (HelpTextPanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem->ItemId = LTDE_PANEL_ITEM_ID_CONTENT;

  CopyRect (&ContentItem->ItemField, &HelpTextPanel->PanelField);
  InflateRect (&ContentItem->ItemField, -HelpTextPanel->BorderLineWidth, -HelpTextPanel->BorderLineWidth);

  InitHelpTextPanelContentItemControlList (HelpTextPanel, ContentItem);

  return EFI_SUCCESS;
}

/**
  Initialize the control list in content page up or content page down item of help text panel.

  @param[in]      HelpTextPanel   A pointer to the help text panel instance.
  @param[in]      IsContentPageUp TRUE if current panel item is content page up item.
  @param[in, out] PanelItem       A pointer to the content page up or content page down item instance.

  @retval EFI_SUCCESS             Initialize the control list successful.
  @retval EFI_INVALID_PARAMETER   HelpTextPanel or PanelItem is NULL.
  @retval EFI_OUT_OF_RESOURCES    Allocate pool fail.
**/
STATIC
EFI_STATUS
InitHelpTextPanelContentPageUpPageDownItemControlList (
  IN     H2O_LTDE_PANEL                       *HelpTextPanel,
  IN     BOOLEAN                              IsContentPageUp,
  IN OUT H2O_LTDE_PANEL_ITEM                  *PanelItem
  )
{
  UINT32                                      Action;
  UINT32                                      Index;
  CHAR16                                      *ControlStr;
  H2O_LTDE_CONTROL                            *Control;

  if (HelpTextPanel == NULL || PanelItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Action     = IsContentPageUp ? LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION_PAGE_UP : LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION_PAGE_DOWN;
  ControlStr = NULL;
  for (Index = 0; Index < sizeof (mHelpTextPanelHotKeyList) / sizeof (LTDE_HELP_TEXT_PANEL_HOT_KEY); Index++) {
    if (mHelpTextPanelHotKeyList[Index].Action == Action) {
      ControlStr = mHelpTextPanelHotKeyList[Index].String;
      break;
    }
  }

  Control = AllocateZeroPool (sizeof (H2O_LTDE_CONTROL));
  if (Control == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Control->Selectable  = TRUE;
  Control->Text.String = (ControlStr != NULL) ? AllocateCopyPool (StrSize (ControlStr), ControlStr) : NULL;
  CopyRect (&Control->ControlField, &PanelItem->ItemField);
  OffsetRect (&Control->ControlField, -Control->ControlField.left, -Control->ControlField.top);

  PanelItem->ControlCount = 1;
  PanelItem->ControlList  = Control;

  return EFI_SUCCESS;
}

/**
  Initialize the content page up or content page down item of help text panel.

  @param[in]      HelpTextPanel   A pointer to the help text panel instance.
  @param[in]      IsContentPageUp TRUE if current panel item is content page up item.
  @param[in, out] PageUpItem      A pointer to the content page up item instance.

  @retval EFI_SUCCESS             Initialize the content page up or content page down item successful.
  @retval EFI_INVALID_PARAMETER   HelpTextPanel or PanelItem is NULL.
**/
STATIC
EFI_STATUS
InitHelpTextPanelContentPageUpPageDownItem (
  IN     H2O_LTDE_PANEL                       *HelpTextPanel,
  IN     BOOLEAN                              IsContentPageUp,
  IN OUT H2O_LTDE_PANEL_ITEM                  *PanelItem
  )
{
  if (HelpTextPanel == NULL || PanelItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PanelItem->ItemId = IsContentPageUp ? LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP : LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN;
  PanelItem->Hidden = TRUE;

  CopyRect (&PanelItem->ItemField, &HelpTextPanel->PanelField);
  InflateRect (&PanelItem->ItemField, -HelpTextPanel->BorderLineWidth, -HelpTextPanel->BorderLineWidth);
  if (IsContentPageUp) {
    PanelItem->ItemField.bottom = PanelItem->ItemField.top;
  } else {
    PanelItem->ItemField.top = PanelItem->ItemField.bottom;
  }

  InitHelpTextPanelContentPageUpPageDownItemControlList (HelpTextPanel, IsContentPageUp, PanelItem);

  return EFI_SUCCESS;
}

/**
  Initialize help text panel.

  @retval EFI_SUCCESS            Initialize help text panel successful.
  @retval EFI_NOT_FOUND          Help text panel is not found.
  @retval EFI_OUT_OF_RESOURCES   Allocate pool fail.
**/
EFI_STATUS
InitHelpTextPanel (
  VOID
  )
{
  H2O_LTDE_PANEL                              *HelpTextPanel;
  UINT32                                      ItemCount;
  H2O_LTDE_PANEL_ITEM                         *ItemList;

  HelpTextPanel = GetPanel (H2O_PANEL_TYPE_HELP_TEXT);
  if (HelpTextPanel == NULL) {
    return EFI_NOT_FOUND;
  }

  ItemCount = 3;
  ItemList  = AllocateZeroPool (ItemCount * sizeof (H2O_LTDE_PANEL_ITEM));
  if (ItemList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  InitHelpTextPanelContentItem (HelpTextPanel, &ItemList[0]);
  InitHelpTextPanelContentPageUpPageDownItem (HelpTextPanel, TRUE , &ItemList[1]);
  InitHelpTextPanelContentPageUpPageDownItem (HelpTextPanel, FALSE, &ItemList[2]);

  FreePanelItemList (HelpTextPanel->ItemList, HelpTextPanel->ItemCount);
  HelpTextPanel->ItemCount = ItemCount;
  HelpTextPanel->ItemList  = ItemList;

  UpdateHelpTextPanelItemField (HelpTextPanel);

  return EFI_SUCCESS;
}

/**
  Display help text panel.

  @retval EFI_SUCCESS            Display help text panel successful.
  @retval EFI_OUT_OF_RESOURCES   Allocate pool fail.
**/
EFI_STATUS
DisplayHelpTextPanel (
  VOID
  )
{
  EFI_STATUS                                  Status;
  H2O_LTDE_PANEL                              *HelpTextPanel;
  H2O_LTDE_PANEL_ITEM                         *PageUpItem;
  H2O_LTDE_PANEL_ITEM                         *PageDownItem;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  UINT32                                      ContentItemWidth;
  UINT32                                      ContentItemHeight;
  UINT32                                      HighlightAttribute;
  UINT32                                      Index;
  UINT32                                      StringArrayNum;
  CHAR16                                      **StringArray;
  CHAR16                                      *LineString;
  CHAR16                                      *SpaceString;

  HelpTextPanel = GetPanel (H2O_PANEL_TYPE_HELP_TEXT);
  if (HelpTextPanel == NULL || !HelpTextPanel->Visible) {
    return EFI_SUCCESS;
  }

  Status = GetPanelColorAttribute (HelpTextPanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_HIGHLIGHT, &HighlightAttribute);
  if (EFI_ERROR (Status)) {
    HighlightAttribute = EFI_WHITE | EFI_BACKGROUND_LIGHTGRAY;
  }

  //
  // Sequentially display content page up, content, content page down items.
  //
  PageUpItem = GetPanelItem (HelpTextPanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP);
  if (PageUpItem != NULL && !PageUpItem->Hidden) {
    LineString = GetAlignmentString (PageUpItem->ControlList->Text.String, H2O_LTDE_FIELD_WIDTH (&PageUpItem->ItemField), LTDE_STRING_ALIGNMENT_ACTION_FLUSH_RIGHT);
    if (LineString != NULL) {
      DEConOutSetAttribute (HighlightAttribute);
      DisplayString (PageUpItem->ItemField.left, PageUpItem->ItemField.top, LineString);
      FreePool (LineString);
    }
  }

  ContentItem = GetPanelItem (HelpTextPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem == NULL) {
    return EFI_SUCCESS;
  }

  if (ContentItem->ControlCount == 0 || ContentItem->ControlList->Text.String == NULL) {
    ClearField (HelpTextPanel->ColorAttribute, &ContentItem->ItemField);
    return EFI_SUCCESS;
  }

  ContentItemWidth  = H2O_LTDE_FIELD_WIDTH (&ContentItem->ItemField);
  ContentItemHeight = H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField);

  Status = GetStringArrayByWidth (ContentItem->ControlList->Text.String, ContentItemWidth, &StringArrayNum, &StringArray);
  if (EFI_ERROR (Status) || StringArray == NULL) {
    ClearField (HelpTextPanel->ColorAttribute, &ContentItem->ItemField);
    return EFI_SUCCESS;
  }

  SpaceString = CreateString (ContentItemWidth, ' ');
  if (SpaceString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DEConOutSetAttribute (HelpTextPanel->ColorAttribute);
  for (Index = 0; Index < ContentItemHeight; Index++) {
    if (ContentItem->CurrentPos + Index < StringArrayNum) {
      LineString = GetAlignmentString (StringArray[ContentItem->CurrentPos + Index], ContentItemWidth, LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT);
      if (LineString != NULL) {
        DisplayString (ContentItem->ItemField.left, ContentItem->ItemField.top + Index, LineString);
        FreePool (LineString);
        continue;
      }
    }

    DisplayString (ContentItem->ItemField.left, ContentItem->ItemField.top + Index, SpaceString);
  }

  PageDownItem = GetPanelItem (HelpTextPanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN);
  if (PageDownItem != NULL && !PageDownItem->Hidden) {
    LineString = GetAlignmentString (PageDownItem->ControlList->Text.String, H2O_LTDE_FIELD_WIDTH (&PageDownItem->ItemField), LTDE_STRING_ALIGNMENT_ACTION_FLUSH_RIGHT);
    if (LineString != NULL) {
      DEConOutSetAttribute (HighlightAttribute);
      DisplayString (PageDownItem->ItemField.left, PageDownItem->ItemField.top, LineString);
      FreePool (LineString);
    }
  }

  for (Index = 0; Index < StringArrayNum; Index++) {
    SafeFreePool ((VOID **) &StringArray[Index]);
  }
  SafeFreePool ((VOID **) &StringArray);
  SafeFreePool ((VOID **) &SpaceString);

  return EFI_SUCCESS;
}

/**
  Process user input in help text panel.

  @param[in] UserInputKey        A pointer to user input key data.

  @retval EFI_SUCCESS            Process user input successful and user input key is one of hot key in help text panel.
  @retval EFI_INVALID_PARAMETER  UserInputKey is NULL.
  @retval EFI_NOT_FOUND          Help text panel or content item is not found.
  @retval EFI_UNSUPPORTED        USer input key is not supported in help text panel.
**/
EFI_STATUS
HelpTextPanelProcessUserInput (
  IN EFI_INPUT_KEY                            *UserInputKey
  )
{
  H2O_LTDE_PANEL                              *HelpTextPanel;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_PANEL_ITEM                         *PageUpItem;
  H2O_LTDE_PANEL_ITEM                         *PageDownItem;
  UINT32                                      Index;
  UINT32                                      Count;
  EFI_INPUT_KEY                               InputKey;
  UINT32                                      HotKeyAction;
  INT32                                       ShiftHeight;

  if (UserInputKey == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HelpTextPanel = GetPanel (H2O_PANEL_TYPE_HELP_TEXT);
  if (HelpTextPanel == NULL || !HelpTextPanel->Visible) {
    return EFI_NOT_FOUND;
  }
  ContentItem = GetPanelItem (HelpTextPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // Check if user input is one of hot key in help text panel or not.
  //
  CopyMem (&InputKey, UserInputKey, sizeof (EFI_INPUT_KEY));
  InputKey.UnicodeChar = TO_LOWER_UNICODE_CHAR (InputKey.UnicodeChar);

  HotKeyAction = LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION_MAX;
  Count        = sizeof (mHelpTextPanelHotKeyList) / sizeof (LTDE_HELP_TEXT_PANEL_HOT_KEY);
  for (Index = 0; Index < Count; Index++) {
    if (mHelpTextPanelHotKeyList[Index].InputKey.ScanCode    == InputKey.ScanCode &&
        mHelpTextPanelHotKeyList[Index].InputKey.UnicodeChar == InputKey.UnicodeChar) {
      HotKeyAction = mHelpTextPanelHotKeyList[Index].Action;
      break;
    }
  }

  switch (HotKeyAction) {

  case LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION_PAGE_UP:
    if (ContentItem->CurrentPos == 0) {
      break;
    }

    ShiftHeight  = H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField);
    PageDownItem = GetPanelItem (HelpTextPanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN);
    if (PageDownItem != NULL && PageDownItem->Hidden) {
      ShiftHeight -= H2O_LTDE_FIELD_HEIGHT (&PageDownItem->ItemField);
    }

    if (ContentItem->CurrentPos > ShiftHeight) {
      ContentItem->CurrentPos -= ShiftHeight;
      PageUpItem = GetPanelItem (HelpTextPanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP);
      if (PageUpItem != NULL && ContentItem->CurrentPos <= H2O_LTDE_FIELD_HEIGHT (&PageUpItem->ItemField)) {
        ContentItem->CurrentPos = 0;
      }
    } else {
      ContentItem->CurrentPos = 0;
    }
    UpdateHelpTextPanelItemField (HelpTextPanel);
    DisplayHelpTextPanel ();
    break;

  case LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION_PAGE_DOWN:
    ShiftHeight = H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField);
    if (ContentItem->MaxPos <= ContentItem->CurrentPos + ShiftHeight - 1) {
      break;
    }

    ContentItem->CurrentPos += ShiftHeight;
    UpdateHelpTextPanelItemField (HelpTextPanel);
    DisplayHelpTextPanel ();
    break;

  default:
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Process mouse click selected control in help text panel.

  @param[in] SelectedPanelItem   A pointer to the mouse selected panel item.
  @param[in] SelectedControl     A pointer to the mouse selected control.

  @retval EFI_SUCCESS            Process user input successful and user input key is one of hot key in help text panel.
  @retval EFI_INVALID_PARAMETER  SelectedPanelItem or SelectedControl is NULL.
  @retval EFI_UNSUPPORTED        Mouse selected control is not supported in help text panel.
**/
EFI_STATUS
HelpTextPanelProcessMouseClick (
  IN H2O_LTDE_PANEL_ITEM                      *SelectedPanelItem,
  IN H2O_LTDE_CONTROL                         *SelectedControl
  )
{
  UINT32                                      Index;
  UINT32                                      Action;

  if (SelectedPanelItem == NULL || SelectedControl == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  switch (SelectedPanelItem->ItemId) {

  case LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP:
  case LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN:
    Action = (SelectedPanelItem->ItemId == LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP) ? LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION_PAGE_UP :
                                                                                 LTDE_HELP_TEXT_PANEL_HOT_KEY_ACTION_PAGE_DOWN;
    for (Index = 0; Index < sizeof (mHelpTextPanelHotKeyList) / sizeof (LTDE_HELP_TEXT_PANEL_HOT_KEY); Index++) {
      if (mHelpTextPanelHotKeyList[Index].Action == Action) {
        HelpTextPanelProcessUserInput (&mHelpTextPanelHotKeyList[Index].InputKey);
        break;
      }
    }
    break;

  default:
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}


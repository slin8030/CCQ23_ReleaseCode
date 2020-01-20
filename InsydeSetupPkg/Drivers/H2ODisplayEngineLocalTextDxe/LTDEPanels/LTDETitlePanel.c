/** @file
  Initialization and display related functions for title panel.

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

/**
 Initialize control list in content item

 @param[in]      TitlePanel         Pointer to title panel
 @param[in, out] ContentItem        Pointer to content item

 @retval EFI_SUCCESS                Initialize control list successful.
 @retval EFI_INVALID_PARAMETER      TitlePanel or ContentItem is NULL
 @retval EFI_OUT_OF_RESOURCES       Allocate pool fail

**/
STATIC
EFI_STATUS
InitTitlePanelContentItemControlList (
  IN     H2O_LTDE_PANEL                       *TitlePanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_SM                         *SetupMenuData;
  UINT32                                      ControlCount;
  H2O_LTDE_CONTROL                            *ControlList;
  H2O_LTDE_CONTROL                            *Control;
  INT32                                       ItemFieldWidth;
  INT32                                       DisplayWidth;
  INT32                                       StartX;

  if (TitlePanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem->ControlCount = 0;
  ContentItem->ControlList  = NULL;

  Status = mDEPrivate->FBProtocol->GetSMInfo (mDEPrivate->FBProtocol, &SetupMenuData);
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  ControlCount = 0;
  ControlCount += (SetupMenuData->TitleString       != NULL) ? 1 : 0;
  ControlCount += (SetupMenuData->CoreVersionString != NULL) ? 1 : 0;
  if (ControlCount == 0) {
    FreeSetupMenuData (SetupMenuData);
    return EFI_SUCCESS;
  }

  ControlList = AllocateZeroPool (ControlCount * sizeof (H2O_LTDE_CONTROL));
  if (ControlList == NULL) {
    FreeSetupMenuData (SetupMenuData);
    return EFI_OUT_OF_RESOURCES;
  }

  ItemFieldWidth = H2O_LTDE_FIELD_WIDTH (&ContentItem->ItemField);
  ControlCount   = 0;
  if (SetupMenuData->TitleString != NULL) {
    Control                           = &ControlList[ControlCount];
    Control->Sequence                 = (UINT8) ControlCount;
    Control->Text.String              = AllocateCopyPool (StrSize (SetupMenuData->TitleString), SetupMenuData->TitleString);
    Control->ControlStyle.PseudoClass = H2O_STYLE_PSEUDO_CLASS_NORMAL;

    DisplayWidth = (INT32) GetStringDisplayWidth (SetupMenuData->TitleString);
    StartX       = (ItemFieldWidth - DisplayWidth) / 2;
    SetRect (&Control->ControlField, StartX, 0, StartX + DisplayWidth - 1, 0);
    ControlCount++;
  }
  if (SetupMenuData->CoreVersionString != NULL) {
    Control                           = &ControlList[ControlCount];
    Control->Sequence                 = (UINT8) ControlCount;
    Control->Text.String              = AllocateCopyPool (StrSize (SetupMenuData->CoreVersionString), SetupMenuData->CoreVersionString);
    Control->ControlStyle.PseudoClass = H2O_STYLE_PSEUDO_CLASS_NORMAL;

    DisplayWidth = (INT32) GetStringDisplayWidth (SetupMenuData->CoreVersionString);
    StartX       = ItemFieldWidth - DisplayWidth;
    SetRect (&Control->ControlField, StartX, 0, StartX + DisplayWidth - 1, 0);
    ControlCount++;
  }

  ContentItem->Vertical     = FALSE;
  ContentItem->ControlCount = ControlCount;
  ContentItem->ControlList  = ControlList;

  FreeSetupMenuData (SetupMenuData);

  return EFI_SUCCESS;
}

/**
 Initialize content item

 @param[in]      TitlePanel         Pointer to title panel
 @param[in, out] ContentItem        Pointer to content item

 @retval EFI_SUCCESS                Initialize content item successful.
 @retval EFI_INVALID_PARAMETER      TitlePanel or ContentItem is NULL

**/
STATIC
EFI_STATUS
InitTitlePanelContentItem (
  IN     H2O_LTDE_PANEL                       *TitlePanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  )
{
  if (TitlePanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem->ItemId = LTDE_PANEL_ITEM_ID_CONTENT;
  ContentItem->Hidden = FALSE;

  CopyRect (&ContentItem->ItemField, &TitlePanel->PanelField);
  InflateRect (&ContentItem->ItemField, -TitlePanel->BorderLineWidth, -TitlePanel->BorderLineWidth);

  InitTitlePanelContentItemControlList (TitlePanel, ContentItem);

  return EFI_SUCCESS;
}

/**
 Initialize title panel

 @retval EFI_SUCCESS                Initialize title panel successful.
 @retval EFI_NOT_FOUND              Title panel is not found
 @retval EFI_OUT_OF_RESOURCES       Allocate pool fail

**/
EFI_STATUS
InitTitlePanel (
  VOID
  )
{
  H2O_LTDE_PANEL                              *TitlePanel;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;

  TitlePanel = GetPanel (H2O_PANEL_TYPE_FORM_TITLE);
  if (TitlePanel == NULL) {
    return EFI_NOT_FOUND;
  }

  ContentItem = AllocateZeroPool (sizeof (H2O_LTDE_PANEL_ITEM));
  if (ContentItem == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  InitTitlePanelContentItem (TitlePanel, ContentItem);

  FreePanelItemList (TitlePanel->ItemList, TitlePanel->ItemCount);
  TitlePanel->ItemCount = 1;
  TitlePanel->ItemList  = ContentItem;

  return EFI_SUCCESS;
}

/**
  Display title panel

  @retval EFI_SUCCESS                Display title panel successfully
**/
EFI_STATUS
DisplayTitlePanel (
  VOID
  )
{
  H2O_LTDE_PANEL                              *TitlePanel;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;

  TitlePanel = GetPanel (H2O_PANEL_TYPE_FORM_TITLE);
  if (TitlePanel == NULL || !TitlePanel->Visible) {
    return EFI_SUCCESS;
  }

  ContentItem = GetPanelItem (TitlePanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem != NULL) {
    DisplayPanelItem (TitlePanel, ContentItem);
  }

  return EFI_SUCCESS;
}


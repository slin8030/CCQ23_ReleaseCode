/** @file
  Initialization and display related functions for question panel.

;******************************************************************************
;* Copyright (c) 2015 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "LTDEPanels.h"
#include "LTDEQuestions.h"
#include "LTDEPrint.h"
#include "LTDESupport.h"
#include "LTDEMisc.h"

#define DIALOG_MIN_WIDTH                               10
#define DIALOG_SEPARATION_WIDTH                        1
#define PANEL_COLOR_ATTRIBUTE_QUESTION                 (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE)
#define PANEL_COLOR_ATTRIBUTE_QUESTION_BODY_INPUT      (EFI_BLUE      | EFI_BACKGROUND_LIGHTGRAY)

/**
 Check if two fields are adjacent.

 @param[in] Field1        Pointer to first field
 @param[in] Field2        Pointer to second field

 @retval TRUE             Two fields are adjacent
 @retval FALSE            Two fields are not adjacent
**/
STATIC
BOOLEAN
IsFieldAdjacent (
  IN RECT                                     *Field1,
  IN RECT                                     *Field2
  )
{
  if (((Field1->top  == Field2->top  && Field1->bottom == Field2->bottom) && (Field1->right == Field2->left   || Field1->left   == Field2->right)) ||
      ((Field1->left == Field2->left && Field1->right  == Field2->right ) && (Field1->top   == Field2->bottom || Field1->bottom == Field2->top  ))) {
    return TRUE;
  }

  return FALSE;
}

/**
 Get the number of control which is specific control ID in panel item.

 @param[in] PanelItem     A Pointer to panel item instance
 @param[in] ControlId     Request control ID

 @retval The number of control which is specific control ID in panel item.

**/
STATIC
UINT32
GetControlCountById (
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem,
  IN UINT32                                   ControlId
  )
{
  UINT32                                      Index;
  UINT32                                      Count;

  if (PanelItem == NULL || PanelItem->ControlList == NULL) {
    return 0;
  }

  for (Index = 0, Count = 0; Index < PanelItem->ControlCount; Index++) {
    if (PanelItem->ControlList[Index].ControlId == ControlId) {
      Count++;
    }
  }

  return Count;
}

/**
 Get pseudo class by dialog type.

 @param[in] DialogType      Dialog type

 @retval The pseudo class value.

**/
STATIC
UINT32
GetPseudoClass (
  IN UINT32                                   DialogType
  )
{
  if ((DialogType & H2O_FORM_BROWSER_D_TYPE_SHOW_HELP        ) != 0 ||
      (DialogType & H2O_FORM_BROWSER_D_TYPE_SHOW_CONFIRM_PAGE) != 0) {
    return H2O_STYLE_PSEUDO_CLASS_HELP;
  }

  return H2O_STYLE_PSEUDO_CLASS_NORMAL;
}

/**
 Get default dialog field.

 @param[out] DefaultDialogField  A pointer to default dialog field

 @retval EFI_SUCCESS             Get default dialog field successful.
 @retval EFI_INVALID_PARAMETER   DefaultDialogField is NULL.

**/
STATIC
EFI_STATUS
GetDefaultDialogField (
  OUT RECT                                    *DefaultDialogField
  )
{
  H2O_LTDE_PANEL                              *SetupPagePanel;
  H2O_LTDE_PANEL                              *HelpTextPanel;
  INT32                                       ScreenX;
  INT32                                       ScreenY;

  if (DefaultDialogField == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel != NULL && SetupPagePanel->Visible) {
    CopyRect (DefaultDialogField, &SetupPagePanel->PanelField);

    HelpTextPanel = GetPanel (H2O_PANEL_TYPE_HELP_TEXT);
    if (HelpTextPanel != NULL && HelpTextPanel->Visible && IsFieldAdjacent (&SetupPagePanel->PanelField, &HelpTextPanel->PanelField)) {
      UnionRect (DefaultDialogField, DefaultDialogField, &HelpTextPanel->PanelField);
    }
  } else {
    DEConOutQueryModeWithoutModeNumer ((UINT32 *) &ScreenX, (UINT32 *) &ScreenY);
    SetRect (DefaultDialogField, 0, 0, ScreenX - 1, ScreenY - 1);
  }

  return EFI_SUCCESS;
}

/**
 Get next selectable control in question panel.

 @param[in] QuestionPanel    A Pointer to question panel instance
 @param[in] CurrentControl   A Pointer to current control in question panel
 @param[in] IsLoop           TRUE if get next selectable control in loop

 @return The pointer of next selectable control or NULL if not found or QuestionPanel is NULL.

**/
STATIC
H2O_LTDE_CONTROL *
GetNextSelectableControlInQuestionPanel (
  IN H2O_LTDE_PANEL                           *QuestionPanel,
  IN H2O_LTDE_CONTROL                         *CurrentControl,
  IN BOOLEAN                                  IsLoop
  )
{
  H2O_LTDE_CONTROL                            *NextControl;
  H2O_LTDE_PANEL_ITEM                         *CurrentPanelItem;
  H2O_LTDE_PANEL_ITEM                         *PanelItem;

  if (QuestionPanel == NULL) {
    return NULL;
  }

  NextControl      = NULL;
  CurrentPanelItem = GetPanelItemByControl (QuestionPanel, CurrentControl);

  //
  // Get next selectable control in current panel item if control ID of current control is not dialog body.
  //
  if (CurrentPanelItem != NULL && CurrentControl != NULL && CurrentControl->ControlId != LTDE_CONTROL_ID_DIALOG_BODY) {
    NextControl = GetNextSelectableControl (CurrentPanelItem, CurrentControl, FALSE);
    if (NextControl != NULL) {
      return NextControl;
    }
  }

  //
  // Get next selectable control from next panel item.
  //
  PanelItem = CurrentPanelItem;
  while (TRUE) {
    PanelItem = GetNextSelectablePanelItem (QuestionPanel, PanelItem, IsLoop);
    if (PanelItem == NULL) {
      break;
    }

    NextControl = GetNextSelectableControl (PanelItem, NULL, FALSE);
    if (NextControl != NULL) {
      break;
    }
  }

  if (NextControl == NULL && IsLoop && CurrentControl != NULL && CurrentControl->ControlId != LTDE_CONTROL_ID_DIALOG_BODY) {
    //
    // Get first selectable control in current panel item.
    //
    NextControl = GetNextSelectableControl (CurrentPanelItem, NULL, FALSE);
  }

  if (NextControl == NULL || NextControl == CurrentControl) {
    return NULL;
  }

  return NextControl;
}

/**
 Get required display width of input numeric value.

 @param[in] Value            Numeric value
 @param[in] IsHex            TRUE if numeric value is display in hex

 @return The required display width of input numeric value.

**/
STATIC
UINT32
GetRequiredWidthOfNumericValue (
  IN UINT64                                   Value,
  IN BOOLEAN                                  IsHex
  )
{
  UINT32                                      Base;
  UINT32                                      RequiredWidth;
  UINT64                                      LastValue;

  Base          = IsHex ? 16 : 10;
  RequiredWidth = 0;
  LastValue     = Value;

  while (Value > 0) {
    RequiredWidth++;
    LastValue = Value;
    Value     = DivU64x32 (Value, Base);
  }
  if (ModU64x32(LastValue, Base) == 0) {
    RequiredWidth++;
  }

  if (IsHex) {
    RequiredWidth += (sizeof (HEX_NUMBER_PREFIX_STRING) / sizeof (CHAR16) - 1);
  }

  return RequiredWidth;
}

/**
 Get required display size of title item by calculating all controls in title item.

 @param[in]  TitleItem           A pointer to title item
 @param[in]  LineWidth           Diaplay line width
 @param[out] Width               A pointer to required width
 @param[out] Height              A pointer to required height

 @retval EFI_SUCCESS             Get required display size of title item successful.
 @retval EFI_INVALID_PARAMETER   Input parameter is NULL.

**/
STATIC
EFI_STATUS
GetRequiredSizeOfTitleItem (
  IN  H2O_LTDE_PANEL_ITEM                     *TitleItem,
  IN  UINT32                                  LineWidth,
  OUT UINT32                                  *Width,
  OUT UINT32                                  *Height
  )
{
  UINT32                                      Index;
  UINT32                                      RequiedWidth;
  UINT32                                      RequiedHeight;
  UINT32                                      ControlWidth;
  UINT32                                      ControlHeight;
  EFI_STATUS                                  Status;

  if (TitleItem == NULL || Width == NULL || Height == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  RequiedWidth  = 0;
  RequiedHeight = 0;
  for (Index = 0; Index < TitleItem->ControlCount; Index++) {
    Status = CalculateRequireSize (TitleItem->ControlList[Index].Text.String, LineWidth, &ControlWidth, &ControlHeight);
    if (EFI_ERROR (Status)) {
      continue;
    }
    RequiedWidth   = MAX(RequiedWidth, ControlWidth);
    RequiedHeight += ControlHeight;
  }

  *Width  = RequiedWidth;
  *Height = RequiedHeight;

  return EFI_SUCCESS;
}

/**
 Get required display size of content item by calculating all controls in content item.

 @param[in]  ContentItem            A pointer to content item
 @param[in]  LineWidth              Diaplay line width
 @param[out] Width                  A pointer to required item width
 @param[out] Height                 A pointer to required item height
 @param[out] BodyControlWidth       A pointer to required body control width
 @param[out] BodyInputControlWidth  A pointer to required body input control width

 @retval EFI_SUCCESS                Get required display size of content item successful.
 @retval EFI_INVALID_PARAMETER      Input parameter is NULL.

**/
STATIC
EFI_STATUS
GetRequiredSizeOfContentItem (
  IN  H2O_LTDE_PANEL_ITEM                     *ContentItem,
  IN  UINT32                                  LineWidth,
  OUT UINT32                                  *Width,
  OUT UINT32                                  *Height,
  OUT UINT32                                  *BodyControlWidth,
  OUT UINT32                                  *BodyInputControlWidth
  )
{
  H2O_LTDE_CONTROL                            *Control;
  UINT32                                      Index;
  UINT32                                      Count;
  UINT32                                      BodyWidth;
  UINT32                                      BodyHeight;
  UINT32                                      BodyInputWidth;
  UINT32                                      BodyInputHeight;
  UINT32                                      MaxBodyWidth;
  UINT32                                      MaxBodyInputWidth;
  UINT32                                      RequiredHeight;
  UINT32                                      RequiredWidth;

  if (ContentItem == NULL || Width == NULL || Height == NULL || BodyControlWidth == NULL || BodyInputControlWidth == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Assume controls in dialog body and controls in dialog body input are vertically arranged
  // Assume body and body input of dialog are horizontally arranged in panel content item
  //
  MaxBodyWidth      = 0;
  MaxBodyInputWidth = 0;
  RequiredHeight    = 0;
  Count             = GetControlCountById (ContentItem, LTDE_CONTROL_ID_DIALOG_BODY);
  Count             = MAX (Count, GetControlCountById (ContentItem, LTDE_CONTROL_ID_DIALOG_BODY_INPUT));

  for (Index = 0; Index < Count; Index++) {
    Control = FindControlByControlId (ContentItem->ControlList, ContentItem->ControlCount, LTDE_CONTROL_ID_DIALOG_BODY, Index);
    if (Control != NULL) {
      CalculateRequireSize (Control->Text.String, LineWidth, &BodyWidth, &BodyHeight);
      MaxBodyWidth = MAX (MaxBodyWidth, BodyWidth);
    } else {
      BodyHeight = 0;
    }

    Control = FindControlByControlId (ContentItem->ControlList, ContentItem->ControlCount, LTDE_CONTROL_ID_DIALOG_BODY_INPUT, Index);
    if (Control != NULL) {
      if (Control->Operand == EFI_IFR_NUMERIC_OP) {
        RequiredWidth = GetRequiredWidthOfNumericValue (Control->Maximum, IS_DISPLAYED_IN_HEX (Control));
      } else {
        RequiredWidth = (UINT32) Control->Maximum;
      }

      if (RequiredWidth == 0) {
        BodyInputWidth  = 0;
        BodyInputHeight = 0;
      } else {
        BodyInputWidth   = (RequiredWidth <= LineWidth) ? RequiredWidth : LineWidth;
        BodyInputHeight  = RequiredWidth / BodyInputWidth;
        BodyInputHeight += (RequiredWidth % BodyInputWidth != 0) ? 1 : 0;
      }

      MaxBodyInputWidth = MAX (MaxBodyInputWidth, BodyInputWidth);
    } else {
      BodyInputHeight = 0;
    }

    RequiredHeight += MAX (BodyHeight, BodyInputHeight);
  }

  *Height = RequiredHeight;
  *Width  = MaxBodyWidth + MaxBodyInputWidth + ((MaxBodyWidth != 0 && MaxBodyInputWidth != 0) ? DIALOG_SEPARATION_WIDTH : 0);
  *BodyControlWidth      = MaxBodyWidth;
  *BodyInputControlWidth = MaxBodyInputWidth;

  return EFI_SUCCESS;
}

/**
 Get required display size of button item by calculating all controls in content item.

 @param[in]  ButtonItem             A pointer to button item
 @param[out] Width                  A pointer to required item width
 @param[out] Height                 A pointer to required item height

 @retval EFI_SUCCESS                Get required display size of button item successful.
 @retval EFI_INVALID_PARAMETER      Input parameter is NULL.

**/
STATIC
EFI_STATUS
GetRequiredSizeOfButtonItem (
  IN  H2O_LTDE_PANEL_ITEM                     *ButtonItem,
  OUT UINT32                                  *Width,
  OUT UINT32                                  *Height
  )
{
  UINT32                                      Index;
  UINTN                                       DialogButtonWidth;

  if (ButtonItem == NULL || Width == NULL || Height == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Assume controls of dialog buttons are horizontally arranged in panel button item
  //
  DialogButtonWidth = 0;
  for (Index = 0; Index < ButtonItem->ControlCount; Index++) {
    DialogButtonWidth += GetStringDisplayWidth (ButtonItem->ControlList[Index].Text.String);
  }
  if (ButtonItem->ControlCount > 1) {
    DialogButtonWidth += ((ButtonItem->ControlCount - 1) * DIALOG_SEPARATION_WIDTH);
  }

  *Width  = (UINT32) DialogButtonWidth;
  *Height = 1;

  return EFI_SUCCESS;
}

/**
 Set field of title item and fields of all controls in title item.

 @param[in] TitleItem               A pointer to title item
 @param[in] Left                    Left value of title item
 @param[in] Top                     Top value of title item
 @param[in] Right                   Right value of title item
 @param[in] Bottom                  Bottom value of title item

 @retval EFI_SUCCESS                Set field successful.
 @retval EFI_INVALID_PARAMETER      TitleItem is NULL.

**/
EFI_STATUS
SetFieldOfTitleItem (
  IN H2O_LTDE_PANEL_ITEM                      *TitleItem,
  IN INT32                                    Left,
  IN INT32                                    Top,
  IN INT32                                    Right,
  IN INT32                                    Bottom
  )
{
  UINT32                                      Index;
  INT32                                       StartX;
  INT32                                       StartY;
  INT32                                       EndX;
  UINT32                                      Height;
  UINT32                                      TitleItemWidth;
  H2O_LTDE_CONTROL                            *Control;

  if (TitleItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SetRect (&TitleItem->ItemField, Left, Top, Right, Bottom);

  //
  // Update field of all controls in title item.
  //
  TitleItemWidth = H2O_LTDE_FIELD_WIDTH(&TitleItem->ItemField);
  StartY         = 0;
  StartX         = 0;
  EndX           = StartX + TitleItemWidth - 1;
  for (Index = 0; Index < TitleItem->ControlCount; Index++) {
    Control = &TitleItem->ControlList[Index];
    Height  = GetStringHeight (Control->Text.String, TitleItemWidth);
    SetRect (&Control->ControlField, StartX, StartY, EndX, StartY + Height - 1);
    StartY += Height;
  }
  TitleItem->MaxPos = StartY - 1;

  return EFI_SUCCESS;
}

/**
 Set field of content item and fields of all controls in content item.

 @param[in] ContentItem             A pointer to content item
 @param[in] Left                    Left value of content item
 @param[in] Top                     Top value of content item
 @param[in] Right                   Right value of content item
 @param[in] Bottom                  Bottom value of content item
 @param[in] BodyControlWidth        Body control Width of content item
 @param[in] BodyInputControlWidth   Body input control Width of content item

 @retval EFI_SUCCESS                Set field successful.
 @retval EFI_INVALID_PARAMETER      ContentItem is NULL.

**/
EFI_STATUS
SetFieldOfContentItem (
  IN H2O_LTDE_PANEL_ITEM                      *ContentItem,
  IN INT32                                    Left,
  IN INT32                                    Top,
  IN INT32                                    Right,
  IN INT32                                    Bottom,
  IN UINT32                                   BodyControlWidth,
  IN UINT32                                   BodyInputControlWidth
  )
{
  UINT32                                      Index;
  UINT32                                      Count;
  UINT32                                      BodyCount;
  UINT32                                      BodyWidth;
  UINT32                                      BodyHeight;
  UINT32                                      BodyInputCount;
  UINT32                                      BodyInputWidth;
  UINT32                                      BodyInputHeight;
  UINT32                                      AvailableWidth;
  INT32                                       BodyX;
  INT32                                       BodyInputX;
  INT32                                       StartY;
  UINT32                                      Width;
  UINT32                                      Height;
  H2O_LTDE_CONTROL                            *BodyControl;
  H2O_LTDE_CONTROL                            *BodyInputControl;

  if (ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SetRect (&ContentItem->ItemField, Left, Top, Right, Bottom);
  if (ContentItem->ControlList == NULL || ContentItem->ControlCount == 0) {
    return EFI_SUCCESS;
  }

  //
  // Determine width of body and body input controls
  //
  BodyCount      = GetControlCountById (ContentItem, LTDE_CONTROL_ID_DIALOG_BODY);
  BodyInputCount = GetControlCountById (ContentItem, LTDE_CONTROL_ID_DIALOG_BODY_INPUT);
  BodyWidth      = BodyControlWidth;
  BodyInputWidth = BodyInputControlWidth;
  if (BodyCount != 0 && BodyInputCount != 0) {
    AvailableWidth = H2O_LTDE_FIELD_WIDTH(&ContentItem->ItemField) - DIALOG_SEPARATION_WIDTH;
    if (BodyWidth + BodyInputWidth > AvailableWidth) {
      if (BodyWidth      > AvailableWidth / 2 &&
          BodyInputWidth > AvailableWidth / 2) {
        BodyWidth      = AvailableWidth / 2;
        BodyInputWidth = AvailableWidth / 2;
      } else if (BodyWidth > AvailableWidth / 2) {
        BodyWidth = AvailableWidth - BodyInputWidth;
      } else {
        BodyInputWidth = AvailableWidth - BodyWidth;
      }
    } else if (BodyWidth + BodyInputWidth < AvailableWidth) {
      BodyInputWidth = AvailableWidth - BodyWidth;
    }
  } else if (BodyCount != 0) {
    BodyWidth = H2O_LTDE_FIELD_WIDTH(&ContentItem->ItemField);
  } else if (BodyInputCount != 0) {
    BodyInputWidth = H2O_LTDE_FIELD_WIDTH(&ContentItem->ItemField);
  } else {
    return EFI_ABORTED;
  }

  //
  // Set field of body and body input controls
  //
  BodyX      = 0;
  BodyInputX = (BodyCount == 0) ? 0 : BodyX + BodyWidth + DIALOG_SEPARATION_WIDTH;
  StartY     = 0;
  Count      = MAX (BodyCount, BodyInputCount);
  for (Index = 0; Index < Count; Index++) {
    BodyInputHeight  = 0;

    BodyControl = FindControlByControlId (ContentItem->ControlList, ContentItem->ControlCount, LTDE_CONTROL_ID_DIALOG_BODY, Index);
    if (BodyControl != NULL) {
      BodyHeight = GetStringHeight (BodyControl->Text.String, BodyWidth);
    } else {
      BodyHeight = 0;
    }

    BodyInputControl = FindControlByControlId (ContentItem->ControlList, ContentItem->ControlCount, LTDE_CONTROL_ID_DIALOG_BODY_INPUT, Index);
    if (BodyInputControl != NULL) {
      if (BodyInputControl->Operand == EFI_IFR_NUMERIC_OP) {
        Width = GetRequiredWidthOfNumericValue (BodyInputControl->Maximum, IS_DISPLAYED_IN_HEX (BodyInputControl));
      } else {
        Width = (UINT32) BodyInputControl->Maximum;
      }
      BodyInputHeight  = Width / BodyInputWidth;
      BodyInputHeight += (Width % BodyInputWidth != 0) ? 1 : 0;
    }

    Height = MAX (BodyHeight, BodyInputHeight);
    if (Height == 0) {
      continue;
    }

    if (BodyControl != NULL) {
      SetRect (&BodyControl->ControlField, BodyX, StartY, BodyX + BodyWidth - 1, StartY + Height - 1);
    }
    if (BodyInputControl != NULL) {
      SetRect (&BodyInputControl->ControlField, BodyInputX, StartY, BodyInputX + BodyInputWidth - 1, StartY + Height - 1);
    }

    StartY += ((INT32) Height);
  }

  ContentItem->MaxPos = (StartY > 0) ? StartY - 1 : 0;

  return EFI_SUCCESS;
}

/**
 Set field of button item and fields of all controls in button item.

 @param[in] ButtonItem                A pointer to button item
 @param[in] Left                      Left value of button item
 @param[in] Top                       Top value of button item
 @param[in] Right                     Right value of button item
 @param[in] Bottom                    Bottom value of button item
 @param[in] ButtonItemRequiredWidth   Required Width of button item

 @retval EFI_SUCCESS                  Set field successful.
 @retval EFI_INVALID_PARAMETER        ButtonItem is NULL.

**/
EFI_STATUS
SetFieldOfButtonItem (
  IN H2O_LTDE_PANEL_ITEM                      *ButtonItem,
  IN INT32                                    Left,
  IN INT32                                    Top,
  IN INT32                                    Right,
  IN INT32                                    Bottom,
  IN UINT32                                   ButtonItemRequiredWidth
  )
{
  UINT32                                      ButtonItemWidth;
  UINT32                                      Index;
  UINT32                                      DisplayWidth;
  INT32                                       StartX;
  INT32                                       StartY;
  H2O_LTDE_CONTROL                            *Control;

  if (ButtonItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SetRect (&ButtonItem->ItemField, Left, Top, Right, Bottom);
  if (ButtonItem->ControlList == NULL || ButtonItem->ControlCount == 0) {
    return EFI_NOT_FOUND;
  }

  ButtonItemWidth = H2O_LTDE_FIELD_WIDTH(&ButtonItem->ItemField);
  StartY          = 0;
  StartX          = (ButtonItemWidth > ButtonItemRequiredWidth) ? (ButtonItemWidth - ButtonItemRequiredWidth) / 2 : 0;
  for (Index = 0; Index < ButtonItem->ControlCount; Index++) {
    Control      = &ButtonItem->ControlList[Index];
    DisplayWidth = (UINT32) GetStringDisplayWidth (Control->Text.String);
    SetRect (&Control->ControlField, StartX, StartY, StartX + DisplayWidth - 1, StartY);

    StartX += (DisplayWidth + DIALOG_SEPARATION_WIDTH);
  }
  ButtonItem->MaxPos = StartX - 1;

  return EFI_SUCCESS;
}

/**
 Update question panel field

 @param[in] Dialog                    A pointer to dialog
 @param[in] QuestionPanel             A pointer to question panel instance

 @retval EFI_SUCCESS                  Set field successful.
 @retval EFI_INVALID_PARAMETER        Dialog or QuestionPanel is NULL.
 @retval EFI_ABORTED                  Dialog field is empty.

**/
EFI_STATUS
QuestionPanelUpdateField (
  IN H2O_FORM_BROWSER_D                       *Dialog,
  IN H2O_LTDE_PANEL                           *QuestionPanel
  )
{
  RECT                                        DefaultDialogField;
  UINT32                                      DefaultDialogWidth;
  UINT32                                      DefaultDialogHeight;
  RECT                                        DialogField;
  H2O_LTDE_PANEL_ITEM                         *TitleItem;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_PANEL_ITEM                         *ButtonItem;
  H2O_LTDE_PANEL_ITEM                         *PageUpItem;
  H2O_LTDE_PANEL_ITEM                         *PageDownItem;
  UINT32                                      TitleItemWidth;
  UINT32                                      TitleItemHeight;
  UINT32                                      ContentItemWidth;
  UINT32                                      ContentItemHeight;
  UINT32                                      ButtonItemWidth;
  UINT32                                      ButtonItemHeight;
  UINT32                                      BodyControlWidth;
  UINT32                                      BodyInputControlWidth;
  UINT32                                      RequiredWidth;
  UINT32                                      RequiredHeight;
  INT32                                       StartX;
  INT32                                       StartY;
  INT32                                       EndX;
  INT32                                       EndY;
  INT32                                       Offset;

  if (Dialog == NULL || QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  GetDefaultDialogField (&DefaultDialogField);
  DefaultDialogWidth  = (UINT32) H2O_LTDE_FIELD_WIDTH(&DefaultDialogField);
  DefaultDialogHeight = (UINT32) H2O_LTDE_FIELD_HEIGHT(&DefaultDialogField);

  //
  // Get required sizes of panel controls
  //
  TitleItem       = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_TITLE);
  TitleItemWidth  = 0;
  TitleItemHeight = 0;
  if (TitleItem != NULL && !TitleItem->Hidden) {
    GetRequiredSizeOfTitleItem (TitleItem, DefaultDialogWidth, &TitleItemWidth, &TitleItemHeight);
  }

  ContentItem           = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  ContentItemWidth      = 0;
  ContentItemHeight     = 0;
  BodyControlWidth      = 0;
  BodyInputControlWidth = 0;
  if (ContentItem != NULL && !ContentItem->Hidden) {
    GetRequiredSizeOfContentItem (ContentItem, DefaultDialogWidth, &ContentItemWidth, &ContentItemHeight, &BodyControlWidth, &BodyInputControlWidth);
  }

  ButtonItem       = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_BUTTON);
  ButtonItemWidth  = 0;
  ButtonItemHeight = 0;
  if (ButtonItem != NULL && !ButtonItem->Hidden) {
    GetRequiredSizeOfButtonItem (ButtonItem, &ButtonItemWidth, &ButtonItemHeight);
  }

  //
  // Calculate dialog field
  // Assume title, body and button controls are vertically arranged in question panel
  //
  if (GetPseudoClass (Dialog->DialogType) == H2O_STYLE_PSEUDO_CLASS_HELP) {
    CopyRect (&DialogField, &DefaultDialogField);
  } else {
    RequiredWidth  = TitleItemWidth;
    RequiredWidth  = MAX (RequiredWidth, ContentItemWidth);
    RequiredWidth  = MAX (RequiredWidth, ButtonItemWidth);
    RequiredWidth += (QuestionPanel->BorderLineWidth * 2);
    if (RequiredWidth < DIALOG_MIN_WIDTH) {
      RequiredWidth = DIALOG_MIN_WIDTH;
    }

    RequiredHeight  = TitleItemHeight;
    RequiredHeight += ContentItemHeight;
    RequiredHeight += ButtonItemHeight;
    RequiredHeight += (ContentItemHeight != 0 && TitleItemHeight  != 0) ? 1 : 0;
    RequiredHeight += (ContentItemHeight != 0 && ButtonItemHeight != 0) ? 1 : 0;
    RequiredHeight += (ContentItemHeight == 0 && TitleItemHeight  != 0 && ButtonItemHeight != 0) ? 1 : 0;
    RequiredHeight += (QuestionPanel->BorderLineWidth * 2);

    CopyRect (&DialogField, &DefaultDialogField);
    if (RequiredWidth < DefaultDialogWidth) {
      DialogField.left  += ((DefaultDialogWidth - RequiredWidth) / 2);
      DialogField.right  = DialogField.left + RequiredWidth - 1;
    }
    if (RequiredHeight < DefaultDialogHeight) {
      DialogField.top   += ((DefaultDialogHeight - RequiredHeight) / 2);
      DialogField.bottom = DialogField.top + RequiredHeight - 1;
    }
  }

  if (IsRectEmpty (&DialogField)) {
    return EFI_ABORTED;
  }

  CopyRect (&QuestionPanel->PanelField, &DialogField);

  //
  // Set fields of panel controls
  //
  StartX = DialogField.left   + QuestionPanel->BorderLineWidth;
  EndX   = DialogField.right  - QuestionPanel->BorderLineWidth;
  StartY = DialogField.top    + QuestionPanel->BorderLineWidth;
  EndY   = DialogField.bottom - QuestionPanel->BorderLineWidth;

  if (TitleItemHeight != 0) {
    SetFieldOfTitleItem (TitleItem, StartX, StartY, EndX, StartY + TitleItemHeight - 1);
    StartY += (TitleItemHeight + 1); // 1 for separation line
  }

  if (ButtonItemHeight != 0) {
    SetFieldOfButtonItem (ButtonItem, StartX, EndY - ButtonItemHeight + 1, EndX, EndY, ButtonItemWidth);
    EndY -= (ButtonItemHeight + 1); // 1 for separation line
  }

  if (ContentItem != NULL && ContentItemHeight != 0) {
    PageUpItem = GetPanelItem (QuestionPanel, ContentItem->Selectable ? LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_UP : LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP);
    if (PageUpItem != NULL && PageUpItem->ControlCount == 1) {
      RequiredWidth = (UINT32) GetStringDisplayWidth (PageUpItem->ControlList[0].Text.String);
      Offset        = (EndX - StartX + 1 > (INT32) RequiredWidth) ? (EndX - StartX + 1 - RequiredWidth) / 2 : 0;
      if (StartY - 1 < DialogField.top) {
        StartY = DialogField.top;
      } else {
        StartY--;
      }
      SetRect (&PageUpItem->ItemField, StartX + Offset, StartY, StartX + Offset + RequiredWidth - 1, StartY);
      StartY++;

      if (ContentItem->CurrentPos > 0) {
        PageUpItem->Hidden = FALSE;
      }
    }

    SetRect (&ContentItem->ItemField, StartX, StartY, EndX, EndY);
    SetFieldOfContentItem (ContentItem, StartX, StartY, EndX, EndY, BodyControlWidth, BodyInputControlWidth);

    if (ContentItem->MaxPos > ContentItem->CurrentPos + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - 1) {
      PageDownItem = GetPanelItem (QuestionPanel, ContentItem->Selectable ? LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_DOWN : LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN);
      if (PageDownItem != NULL && PageDownItem->ControlCount == 1) {
        RequiredWidth = (UINT32) GetStringDisplayWidth (PageDownItem->ControlList[0].Text.String);
        Offset        = (EndX - StartX + 1 > (INT32) RequiredWidth) ? (EndX - StartX + 1 - RequiredWidth) / 2 : 0;
        if (EndY + 1 > DialogField.bottom) {
          EndY = DialogField.bottom;
          SetRect (&ContentItem->ItemField, StartX, StartY, EndX, EndY - 1);
          SetFieldOfContentItem (ContentItem, StartX, StartY, EndX, EndY, BodyControlWidth, BodyInputControlWidth);
        } else {
          EndY++;
        }
        SetRect (&PageDownItem->ItemField, StartX + Offset, EndY, StartX + Offset + RequiredWidth - 1, EndY);
        EndY--;
        PageDownItem->Hidden = FALSE;
      }
    }
  }

  return EFI_SUCCESS;
}

/**
 Initialize title item in question panel

 @param[in]      Dialog               A pointer to dialog
 @param[in, out] TitleItem            A pointer to title item

 @retval EFI_SUCCESS                  Initialize title item successful.
 @retval EFI_INVALID_PARAMETER        Dialog or TitleItem is NULL.
 @retval EFI_OUT_OF_RESOURCES         Allocate pool fail.

**/
EFI_STATUS
QuestionPanelInitTitleItem (
  IN     H2O_FORM_BROWSER_D                   *Dialog,
  IN OUT H2O_LTDE_PANEL_ITEM                  *TitleItem
  )
{
  H2O_LTDE_CONTROL                            *Control;

  if (Dialog == NULL || TitleItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TitleItem->ItemId = LTDE_PANEL_ITEM_ID_TITLE;
  TitleItem->Hidden = TRUE;

  if (Dialog->TitleString == NULL) {
    return EFI_SUCCESS;
  }

  Control = AllocateZeroPool (sizeof (H2O_LTDE_CONTROL));
  if (Control == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Control->ControlId                = LTDE_CONTROL_ID_DIALOG_TITLE;
  Control->Text.String              = AllocateCopyPool (StrSize (Dialog->TitleString), Dialog->TitleString);
  Control->ControlStyle.PseudoClass = GetPseudoClass (Dialog->DialogType);

  TitleItem->Hidden       = FALSE;
  TitleItem->Vertical     = TRUE;
  TitleItem->ControlCount = 1;
  TitleItem->ControlList  = Control;

  return EFI_SUCCESS;
}

/**
 Initialize content item in question panel

 @param[in]      Dialog               A pointer to dialog
 @param[in, out] ContentItem          A pointer to content item

 @retval EFI_SUCCESS                  Initialize content item successful.
 @retval EFI_INVALID_PARAMETER        Dialog or ContentItem is NULL.
 @retval EFI_OUT_OF_RESOURCES         Allocate pool fail.

**/
EFI_STATUS
QuestionPanelInitContentItem (
  IN     H2O_FORM_BROWSER_D                   *Dialog,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  )
{
  UINT32                                      Index;
  UINT32                                      Count;
  H2O_LTDE_CONTROL                            *ControlList;
  H2O_LTDE_CONTROL                            *Control;
  UINT8                                       Operand;
  UINT32                                      PseudoClass;

  if (Dialog == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem->ItemId = LTDE_PANEL_ITEM_ID_CONTENT;
  ContentItem->Hidden = TRUE;

  //
  // Initialize the child control array of panel body control from body and body input of dialog event
  //
  Count = Dialog->BodyStringCount + Dialog->BodyInputCount;
  if (Count == 0) {
    return EFI_SUCCESS;
  }

  ControlList = AllocateZeroPool (Count * sizeof (H2O_LTDE_CONTROL));
  if (ControlList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ContentItem->Hidden       = FALSE;
  ContentItem->Vertical     = TRUE;
  ContentItem->ControlCount = Count;
  ContentItem->ControlList  = ControlList;

  Operand     = GetOpCodeByDialogType (Dialog->DialogType);
  PseudoClass = GetPseudoClass (Dialog->DialogType);
  Count       = 0;
  for (Index = 0; Index < Dialog->BodyStringCount; Index++) {
    Control                           = &ControlList[Count];
    Control->ControlId                = LTDE_CONTROL_ID_DIALOG_BODY;
    Control->Operand                  = Operand;
    Control->Sequence                 = (UINT8) Index;
    Control->Text.String              = AllocateCopyPool (StrSize (Dialog->BodyStringArray[Index]), Dialog->BodyStringArray[Index]);
    Control->ControlStyle.PseudoClass = PseudoClass;

    if (Dialog->BodyHiiValueArray != NULL) {
      Control->Selectable = TRUE;
      CopyHiiValue (&Control->HiiValue, &Dialog->BodyHiiValueArray[Index]);
      ContentItem->Selectable = TRUE;
    } else {
      Control->Selectable = FALSE;
    }
    Count++;
  }

  for (Index = 0; Index < Dialog->BodyInputCount; Index++) {
    Control                           = &ControlList[Count];
    Control->ControlId                = LTDE_CONTROL_ID_DIALOG_BODY_INPUT;
    Control->Operand                  = Operand;
    Control->Sequence                 = (UINT8) Index;
    Control->Selectable               = TRUE;
    Control->ControlStyle.PseudoClass = H2O_STYLE_PSEUDO_CLASS_DISABLED;
    if (Dialog->H2OStatement != NULL) {
      Control->Maximum = Dialog->H2OStatement->Maximum;
      Control->Minimum = Dialog->H2OStatement->Minimum;
      Control->Step    = Dialog->H2OStatement->Step;
      Control->Flags   = Dialog->H2OStatement->Flags;
    }

    switch (Operand) {

    case EFI_IFR_NUMERIC_OP:
      if ((Control->Flags & EFI_IFR_DISPLAY_UINT_HEX) != 0) {
        Control->Text.String = AllocateCopyPool (sizeof (HEX_NUMBER_PREFIX_STRING), HEX_NUMBER_PREFIX_STRING);
      } else {
        Control->Text.String = AllocateZeroPool (sizeof (CHAR16));
      }
      CopyHiiValue (&Control->HiiValue, &Dialog->ConfirmHiiValue);
      break;

    case EFI_IFR_PASSWORD_OP:
      Control->Text.String = AllocateZeroPool (StrSize (Dialog->BodyInputStringArray[Index]));
      CreateValueAsString (&Control->HiiValue, Dialog->ConfirmHiiValue.BufferLen, AllocateZeroPool (Dialog->ConfirmHiiValue.BufferLen));
      if (Control->Maximum == 0) {
        Control->Maximum = Dialog->ConfirmHiiValue.BufferLen / sizeof (CHAR16) - 1;
      }
      break;

    default:
      Control->Text.String = AllocateCopyPool (StrSize (Dialog->BodyInputStringArray[Index]), Dialog->BodyInputStringArray[Index]);
      CreateValueAsString (&Control->HiiValue, Dialog->ConfirmHiiValue.BufferLen, AllocateCopyPool (StrSize (Control->Text.String), Control->Text.String));
      break;
    }

    ContentItem->Selectable = TRUE;
    Count++;
  }

  return EFI_SUCCESS;
}

/**
 Initialize hot key item of content item in question panel

 @param[in]      Dialog               A pointer to dialog
 @param[in]      PanelItemId          Hot key item ID
 @param[in, out] PanelItem            A pointer to hot key item

 @retval EFI_SUCCESS                  Initialize hot key item successful.
 @retval EFI_INVALID_PARAMETER        Dialog or PanelItem is NULL.
 @retval EFI_OUT_OF_RESOURCES         Allocate pool fail.

**/
EFI_STATUS
QuestionPanelInitContentHotKeyItem (
  IN     H2O_FORM_BROWSER_D                   *Dialog,
  IN     UINT32                               PanelItemId,
  IN OUT H2O_LTDE_PANEL_ITEM                  *PanelItem
  )
{
  H2O_LTDE_CONTROL                            *Control;
  UINT32                                      ControlId;
  CHAR16                                      String[2];

  if (Dialog == NULL || PanelItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  switch (PanelItemId) {

  case LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP:
    String[0] = GEOMETRICSHAPE_UP_TRIANGLE;
    ControlId = LTDE_CONTROL_ID_DIALOG_HOT_KEY_PAGE_UP;
    break;

  case LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN:
    String[0] = GEOMETRICSHAPE_DOWN_TRIANGLE;
    ControlId = LTDE_CONTROL_ID_DIALOG_HOT_KEY_PAGE_DOWN;
    break;

  case LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_UP:
    String[0] = GEOMETRICSHAPE_UP_TRIANGLE;
    ControlId = LTDE_CONTROL_ID_DIALOG_HOT_KEY_SCROLL_UP;
    break;

  case LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_DOWN:
    String[0] = GEOMETRICSHAPE_DOWN_TRIANGLE;
    ControlId = LTDE_CONTROL_ID_DIALOG_HOT_KEY_SCROLL_DOWN;
    break;

  default:
    return EFI_UNSUPPORTED;
  }
  String[1] = CHAR_NULL;

  Control = AllocateZeroPool (sizeof (H2O_LTDE_CONTROL));
  if (Control == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Control->ControlId                = ControlId;
  Control->Operand                  = GetOpCodeByDialogType (Dialog->DialogType);
  Control->Selectable               = TRUE;
  Control->Text.String              = AllocateCopyPool (sizeof(String), String);
  Control->ControlStyle.PseudoClass = GetPseudoClass (Dialog->DialogType);

  PanelItem->ItemId       = PanelItemId;
  PanelItem->Hidden       = TRUE;
  PanelItem->Selectable   = TRUE;
  PanelItem->ControlCount = 1;
  PanelItem->ControlList  = Control;

  return EFI_SUCCESS;
}

/**
 Initialize button item in question panel

 @param[in]      Dialog               A pointer to dialog
 @param[in, out] ButtonItem            A pointer to content item

 @retval EFI_SUCCESS                  Initialize hot key item successful.
 @retval EFI_INVALID_PARAMETER        Dialog or ButtonItem is NULL.
 @retval EFI_OUT_OF_RESOURCES         Allocate pool fail.

**/
EFI_STATUS
QuestionPanelInitButtonItem (
  IN     H2O_FORM_BROWSER_D                   *Dialog,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ButtonItem
  )
{
  H2O_LTDE_CONTROL                            *ControlList;
  H2O_LTDE_CONTROL                            *Control;
  UINT32                                      Index;
  UINT8                                       Operand;
  UINT32                                      PseudoClass;

  if (Dialog == NULL || ButtonItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ButtonItem->ItemId = LTDE_PANEL_ITEM_ID_BUTTON;
  ButtonItem->Hidden = TRUE;

  //
  // Initialize the child control array of panel button control from button of dialog event
  //
  if (Dialog->ButtonCount == 0) {
    return EFI_SUCCESS;
  }

  ControlList = AllocateZeroPool (Dialog->ButtonCount * sizeof (H2O_LTDE_CONTROL));
  if (ControlList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ButtonItem->Hidden       = FALSE;
  ButtonItem->Selectable   = TRUE;
  ButtonItem->ControlCount = Dialog->ButtonCount;
  ButtonItem->ControlList  = ControlList;
  ButtonItem->Vertical     = FALSE;

  Operand     = GetOpCodeByDialogType (Dialog->DialogType);
  PseudoClass = GetPseudoClass (Dialog->DialogType);
  for (Index = 0; Index < Dialog->ButtonCount; Index++) {
    Control                           = &ControlList[Index];
    Control->ControlId                = LTDE_CONTROL_ID_DIALOG_BUTTON;
    Control->Operand                  = Operand;
    Control->Sequence                 = (UINT8) Index;
    Control->Text.String              = CatSPrint (NULL, L"[%s]", Dialog->ButtonStringArray[Index]);
    Control->Selectable               = TRUE;
    Control->ControlStyle.PseudoClass = PseudoClass;

    if (Dialog->ButtonHiiValueArray != NULL) {
      CopyHiiValue (&Control->HiiValue, &Dialog->ButtonHiiValueArray[Index]);
    } else {
      CreateValueAsUint64 (&Control->HiiValue, (UINT64) Index);
    }
  }

  return EFI_SUCCESS;
}

/**
 Display content item in question panel

 @param[in]      QuestionPanel        A pointer to question panel instance
 @param[in, out] ContentItem          A pointer to content item

 @retval EFI_SUCCESS                  Display content item successful.
 @retval EFI_INVALID_PARAMETER        QuestionPanel or ContentItem is NULL.

**/
EFI_STATUS
DisplayContentItem (
  IN H2O_LTDE_PANEL                           *QuestionPanel,
  IN H2O_LTDE_PANEL_ITEM                      *ContentItem
  )
{
  UINT32                                      Index;
  UINT32                                      DlgBodyCount;
  UINT32                                      DlgBodyInputCount;
  INT32                                       DisplayStartY;
  INT32                                       DisplayEndY;
  UINT32                                      FieldWidth;
  UINT32                                      FieldHeight;
  UINT32                                      LineIndex;
  UINT32                                      LineCount;
  INT32                                       Y;
  RECT                                        Field;
  H2O_LTDE_CONTROL                            *DlgBodyControl;
  H2O_LTDE_CONTROL                            *DlgBodyInputControl;
  UINT32                                      StringArrayNum;
  CHAR16                                      **StringArray;
  CHAR16                                      *String;
  EFI_STATUS                                  Status;

  if (ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ContentItem->ControlList == NULL || ContentItem->ControlCount == 0) {
    ClearField (QuestionPanel->ColorAttribute, &ContentItem->ItemField);
    return EFI_SUCCESS;
  }

  DlgBodyCount      = GetControlCountById (ContentItem, LTDE_CONTROL_ID_DIALOG_BODY);
  DlgBodyInputCount = GetControlCountById (ContentItem, LTDE_CONTROL_ID_DIALOG_BODY_INPUT);
  DisplayStartY     = ContentItem->CurrentPos;
  DisplayEndY       = DisplayStartY + H2O_LTDE_FIELD_HEIGHT(&ContentItem->ItemField) - 1;

  if (DlgBodyCount != 0) {
    for (Index = 0; Index < DlgBodyCount; Index++) {
      DlgBodyControl = FindControlByControlId (ContentItem->ControlList, ContentItem->ControlCount, LTDE_CONTROL_ID_DIALOG_BODY, Index);
      if (DlgBodyControl == NULL ||
          !IS_OVERLAP(DlgBodyControl->ControlField.top, DlgBodyControl->ControlField.bottom, DisplayStartY, DisplayEndY)) {
        continue;
      }

      FieldWidth = H2O_LTDE_FIELD_WIDTH(&DlgBodyControl->ControlField);
      Status     = GetStringArrayByWidth (DlgBodyControl->Text.String, FieldWidth, &StringArrayNum, &StringArray);
      if (EFI_ERROR (Status)) {
        continue;
      }

      FieldHeight = H2O_LTDE_FIELD_HEIGHT(&DlgBodyControl->ControlField);
      LineCount   = MAX (FieldHeight, StringArrayNum);
      for (LineIndex = 0; LineIndex < LineCount; LineIndex++) {
        Y = DlgBodyControl->ControlField.top + LineIndex;
        if (!IN_RANGE (Y, DisplayStartY, DisplayEndY)) {
          continue;
        }
        ASSERT(Y == DisplayStartY);
        DisplayStartY++;

        String = GetAlignmentString (
                   (LineIndex < StringArrayNum) ? StringArray[LineIndex] : L"",
                   (DlgBodyInputCount == 0) ? FieldWidth : FieldWidth + DIALOG_SEPARATION_WIDTH,
                   LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT
                   );
        if (String == NULL) {
          continue;
        }

        if (DlgBodyControl == QuestionPanel->SelectedControl) {
          DisplayQuestionPanelControl (QuestionPanel, DlgBodyControl, TRUE);
        } else {
          DEConOutSetAttribute (QuestionPanel->ColorAttribute);
          DisplayString (
            ContentItem->ItemField.left + DlgBodyControl->ControlField.left,
            ContentItem->ItemField.top  + Y - ContentItem->CurrentPos,
            String
            );
        }
        FreePool (String);
      }

      for (LineIndex = 0; LineIndex < StringArrayNum; LineIndex++) {
        FreePool (StringArray[LineIndex]);
      }
      FreePool (StringArray);
    }
  }

  if (DlgBodyInputCount != 0) {
    DisplayStartY = ContentItem->CurrentPos;
    DisplayEndY   = DisplayStartY + H2O_LTDE_FIELD_HEIGHT(&ContentItem->ItemField) - 1;

    for (Index = 0; Index < DlgBodyInputCount; Index++) {
      DlgBodyInputControl = FindControlByControlId (ContentItem->ControlList, ContentItem->ControlCount, LTDE_CONTROL_ID_DIALOG_BODY_INPUT, Index);
      if (DlgBodyInputControl == NULL ||
          !IS_OVERLAP(DlgBodyInputControl->ControlField.top, DlgBodyInputControl->ControlField.bottom, DisplayStartY, DisplayEndY)) {
        continue;
      }

      FieldWidth = H2O_LTDE_FIELD_WIDTH(&DlgBodyInputControl->ControlField);
      Status     = GetStringArrayByWidth (DlgBodyInputControl->Text.String, FieldWidth, &StringArrayNum, &StringArray);
      if (EFI_ERROR (Status)) {
        StringArrayNum = 0;
        StringArray    = NULL;
      }

      FieldHeight = H2O_LTDE_FIELD_HEIGHT(&DlgBodyInputControl->ControlField);
      LineCount   = MAX (FieldHeight, StringArrayNum);
      for (LineIndex = 0; LineIndex < LineCount; LineIndex++) {
        Y = DlgBodyInputControl->ControlField.top + LineIndex;
        if (!IN_RANGE (Y, DisplayStartY, DisplayEndY)) {
          continue;
        }
        ASSERT(Y == DisplayStartY);
        DisplayStartY++;

        String = GetAlignmentString (
                   (LineIndex < StringArrayNum) ? StringArray[LineIndex] : L"",
                   FieldWidth,
                   LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT
                   );
        if (String == NULL) {
          continue;
        }

        if (DlgBodyInputControl == QuestionPanel->SelectedControl) {
          DisplayQuestionPanelControl (QuestionPanel, DlgBodyInputControl, TRUE);
        } else {
          DEConOutSetAttribute (PANEL_COLOR_ATTRIBUTE_QUESTION_BODY_INPUT);
          DisplayString (
            ContentItem->ItemField.left + DlgBodyInputControl->ControlField.left,
            ContentItem->ItemField.top  + Y - ContentItem->CurrentPos,
            String
            );
        }
        FreePool (String);
      }

      if (StringArray != NULL) {
        for (LineIndex = 0; LineIndex < StringArrayNum; LineIndex++) {
          FreePool (StringArray[LineIndex]);
        }
        FreePool (StringArray);
      }
    }
  }

  if (DisplayStartY <= DisplayEndY) {
    Field.left   = ContentItem->ItemField.left;
    Field.right  = ContentItem->ItemField.right;
    Field.top    = ContentItem->ItemField.top + DisplayStartY - ContentItem->CurrentPos;
    Field.bottom = ContentItem->ItemField.top + DisplayEndY - ContentItem->CurrentPos;
    ClearField (QuestionPanel->ColorAttribute, &Field);
  }

  return EFI_SUCCESS;
}

/**
 Display panel item in vertical directtion

 @param[in] Panel                A pointer to panel
 @param[in] PanelItem            A pointer to panel item

 @retval EFI_SUCCESS             Display panel item successful.
 @retval EFI_INVALID_PARAMETER   Panel or PanelItem is NULL.

**/
STATIC
EFI_STATUS
DisplayPanelItemInVerticalDirection (
  IN H2O_LTDE_PANEL                           *Panel,
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem
  )
{
  UINT32                                      StartX;
  UINT32                                      StartY;
  UINT32                                      EndY;
  UINT32                                      Index;
  UINT32                                      StringIndex;
  UINT32                                      StringArrayNum;
  CHAR16                                      **StringArray;
  CHAR16                                      *String;
  EFI_STATUS                                  Status;
  UINT32                                      ControlWidth;
  H2O_LTDE_CONTROL                            *Control;

  if (Panel == NULL || PanelItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (PanelItem->ControlList == NULL || PanelItem->ControlCount == 0) {
    ClearField (Panel->ColorAttribute, &PanelItem->ItemField);
    return EFI_SUCCESS;
  }

  StartX = PanelItem->ItemField.left;
  StartY = PanelItem->ItemField.top;
  EndY   = PanelItem->ItemField.bottom;

  for (Index = 0; Index < PanelItem->ControlCount && StartY <= EndY; Index++) {
    Control      = &PanelItem->ControlList[Index];
    ControlWidth = H2O_LTDE_FIELD_WIDTH(&Control->ControlField);

    Status = GetStringArrayByWidth (Control->Text.String, ControlWidth, &StringArrayNum, &StringArray);
    if (EFI_ERROR (Status)) {
      continue;
    }

    for (StringIndex = 0; StringIndex < StringArrayNum && StartY <= EndY; StringIndex++) {
      String = GetAlignmentString (StringArray[StringIndex], ControlWidth, LTDE_STRING_ALIGNMENT_ACTION_CENTERED);
      if (String == NULL) {
        continue;
      }

      DisplayString (StartX, StartY++, String);
      FreePool (String);
    }

    for (StringIndex = 0; StringIndex < StringArrayNum; StringIndex++) {
      SafeFreePool ((VOID **) &StringArray[StringIndex]);
    }
    SafeFreePool ((VOID **) &StringArray);
  }

  return EFI_SUCCESS;
}

/**
 Display panel item in horizontal directtion

 @param[in] Panel                A pointer to panel
 @param[in] PanelItem            A pointer to panel item

 @retval EFI_SUCCESS             Display panel item successful.
 @retval EFI_INVALID_PARAMETER   Panel or PanelItem is NULL.

**/
STATIC
EFI_STATUS
DisplayPanelItemInHorizonalDirection (
  IN H2O_LTDE_PANEL                           *Panel,
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem
  )
{
  UINT32                                      PanelItemWidth;
  UINT32                                      Index;
  INT32                                       Length;
  INT32                                       AbsoluteX;
  INT32                                       AbsoluteY;
  INT32                                       RelativeX;
  H2O_LTDE_CONTROL                            *Control;
  CHAR16                                      *SpaceString;
  CHAR16                                      *DisplayStr;
  UINTN                                       DisplayStrSize;

  if (Panel == NULL || PanelItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PanelItemWidth = H2O_LTDE_FIELD_WIDTH(&PanelItem->ItemField);
  SpaceString    = CreateString (PanelItemWidth, ' ');
  if (SpaceString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DisplayStrSize = (PanelItemWidth + 1) * sizeof(CHAR16);
  DisplayStr     = AllocateZeroPool (DisplayStrSize);
  if (DisplayStr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  AbsoluteY = PanelItem->ItemField.top;
  AbsoluteX = PanelItem->ItemField.left;
  RelativeX = 0;
  for (Index = 0; Index < PanelItem->ControlCount; Index++) {
    Control = &PanelItem->ControlList[Index];

    if (Control->ControlField.left > RelativeX) {
      Length              = Control->ControlField.left - RelativeX;
      SpaceString[Length] = CHAR_NULL;
      StrCat (DisplayStr, SpaceString);
      SpaceString[Length] = L' ';
    }

    RelativeX = Control->ControlField.right + 1;

    if (Control == Panel->SelectedControl) {
      DEConOutSetAttribute (Panel->ColorAttribute);
      DisplayString (AbsoluteX, AbsoluteY, DisplayStr);
      ZeroMem (DisplayStr, DisplayStrSize);
      AbsoluteX += RelativeX;
      DisplayQuestionPanelControl (Panel, Control, TRUE);
    } else {
      StrCat (DisplayStr, Control->Text.String);
    }
  }

  if (RelativeX < (INT32) PanelItemWidth - 1) {
    Length              = (INT32) PanelItemWidth - RelativeX;
    SpaceString[Length] = CHAR_NULL;
    StrCat (DisplayStr, SpaceString);
    SpaceString[Length] = L' ';
  }

  DEConOutSetAttribute (Panel->ColorAttribute);
  DisplayString (AbsoluteX, AbsoluteY, DisplayStr);

  FreePool (DisplayStr);
  FreePool (SpaceString);
  return EFI_SUCCESS;
}

/**
 Display panel item

 @param[in] Panel                A pointer to panel
 @param[in] PanelItem            A pointer to panel item

 @retval EFI_SUCCESS             Display panel item successful.
 @retval EFI_INVALID_PARAMETER   Panel or PanelItem is NULL.

**/
EFI_STATUS
DisplayPanelItem (
  IN H2O_LTDE_PANEL                           *Panel,
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem
  )
{
  if (PanelItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (PanelItem->Hidden) {
    return EFI_ABORTED;
  }

  if (PanelItem->ControlList == NULL || PanelItem->ControlCount == 0) {
    ClearField (Panel->ColorAttribute, &PanelItem->ItemField);
    return EFI_SUCCESS;
  }

  if (PanelItem->Vertical) {
    return DisplayPanelItemInVerticalDirection (Panel, PanelItem);
  } else {
    return DisplayPanelItemInHorizonalDirection (Panel, PanelItem);
  }
}

/**
 Display question panel control

 @param[in] QuestionPanel        A pointer to question panel
 @param[in] Control              A pointer to question panel control
 @param[in] IsHighlight          TRUE if control is highlight

 @retval EFI_SUCCESS             Display question panel control successful.
 @retval EFI_INVALID_PARAMETER   QuestionPanel or Control is NULL.
 @retval EFI_OUT_OF_RESOURCES    Create string fail.
 @retval EFI_NOT_FOUND           Can not find panel item.

**/
EFI_STATUS
DisplayQuestionPanelControl (
  IN H2O_LTDE_PANEL                           *QuestionPanel,
  IN H2O_LTDE_CONTROL                         *Control,
  IN BOOLEAN                                  IsHighlight
  )
{
  H2O_LTDE_PANEL_ITEM                         *PanelItem;
  UINT32                                      Attribute;
  UINT32                                      PseudoClass;
  EFI_STATUS                                  Status;
  UINT32                                      Index;
  UINT32                                      ControlFieldWidth;
  UINT32                                      FieldHeight;
  UINT32                                      StringArrayNum;
  CHAR16                                      **StringArray;
  INT32                                       AbsoluteX;
  INT32                                       RelativeY;
  INT32                                       DisplayStartY;
  INT32                                       DisplayEndY;
  CHAR16                                      *LineString;
  CHAR16                                      *String;
  UINT32                                      Action;

  if (QuestionPanel == NULL || Control == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PseudoClass = IsHighlight ? H2O_STYLE_PSEUDO_CLASS_FOCUS : Control->ControlStyle.PseudoClass;
  GetPanelColorAttribute (QuestionPanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, PseudoClass, &Attribute);
  DEConOutSetAttribute (Attribute);

  ControlFieldWidth = H2O_LTDE_FIELD_WIDTH (&Control->ControlField);
  LineString        = CreateString (ControlFieldWidth, L' ');
  if (LineString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = GetStringArrayByWidth (Control->Text.String, ControlFieldWidth, &StringArrayNum, &StringArray);
  if (EFI_ERROR (Status)) {
    StringArrayNum = 0;
    StringArray    = NULL;
  }

  PanelItem = GetPanelItemByControl (QuestionPanel, Control);
  if (PanelItem == NULL) {
    return EFI_NOT_FOUND;
  }

  Action = (PanelItem->ItemId == LTDE_PANEL_ITEM_ID_CONTENT) ? LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT :
                                                               LTDE_STRING_ALIGNMENT_ACTION_CENTERED;
  if (PanelItem->Vertical) {
    DisplayStartY = PanelItem->CurrentPos;
    DisplayEndY   = DisplayStartY + H2O_LTDE_FIELD_HEIGHT (&PanelItem->ItemField) - 1;

    RelativeY = Control->ControlField.top;
    AbsoluteX = PanelItem->ItemField.left + Control->ControlField.left;

    FieldHeight = H2O_LTDE_FIELD_HEIGHT (&Control->ControlField);
    for (Index = 0; Index < FieldHeight; Index++, RelativeY++) {
      if (!IN_RANGE (RelativeY, DisplayStartY, DisplayEndY)) {
        continue;
      }


      if (Index < StringArrayNum) {
        String = GetAlignmentString (StringArray[Index], ControlFieldWidth, Action);
        if (String == NULL) {
          continue;
        }
        DisplayString (AbsoluteX, PanelItem->ItemField.top + RelativeY - DisplayStartY, String);
        FreePool (String);
      } else {
        DisplayString (AbsoluteX, PanelItem->ItemField.top + RelativeY - DisplayStartY, LineString);
      }
    }
  } else {
    DisplayString (
      PanelItem->ItemField.left + Control->ControlField.left,
      PanelItem->ItemField.top  + Control->ControlField.top,
      Control->Text.String
      );
  }

  FreePool (LineString);
  for (Index = 0; Index < StringArrayNum; Index++) {
    SafeFreePool ((VOID **) &StringArray[Index]);
  }
  SafeFreePool ((VOID **) &StringArray);

  return EFI_SUCCESS;
}

/**
 Display question panel

 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Display question panel control successful.
 @retval EFI_INVALID_PARAMETER   QuestionPanel or Control is NULL.
 @retval EFI_OUT_OF_RESOURCES    Create string fail.

**/
EFI_STATUS
DisplayQuestionPanel (
  IN H2O_LTDE_PANEL                           *QuestionPanel
  )
{
  H2O_LTDE_PANEL_ITEM                         *TitleItem;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_PANEL_ITEM                         *ButtonItem;
  H2O_LTDE_PANEL_ITEM                         *PanelItem;
  UINT32                                      SeparationStrLen;
  CHAR16                                      *SeparationStr;

  if (QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DEConOutSetAttribute (QuestionPanel->ColorAttribute);
  if (QuestionPanel->BorderLineWidth > 0) {
    PrintBorderLineFunc (&QuestionPanel->PanelField, NULL);
  }

  SeparationStrLen = H2O_LTDE_FIELD_WIDTH(&QuestionPanel->PanelField);
  SeparationStr    = CreateString (SeparationStrLen, BOXDRAW_HORIZONTAL);
  if (SeparationStr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  if (QuestionPanel->BorderLineWidth > 0) {
    SeparationStr[0]                    = BOXDRAW_VERTICAL_RIGHT;
    SeparationStr[SeparationStrLen - 1] = BOXDRAW_VERTICAL_LEFT;
  }

  TitleItem = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_TITLE);
  if (TitleItem != NULL && !TitleItem->Hidden) {
    DisplayPanelItem (QuestionPanel, TitleItem);
  }

  ContentItem = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem != NULL && !ContentItem->Hidden) {
    if (TitleItem != NULL && !TitleItem->Hidden) {
      DEConOutSetAttribute (QuestionPanel->ColorAttribute);
      DisplayString (QuestionPanel->PanelField.left, TitleItem->ItemField.bottom + 1, SeparationStr);
    }

    DisplayContentItem (QuestionPanel, ContentItem);
  }

  ButtonItem = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_BUTTON);
  if (ButtonItem != NULL && !ButtonItem->Hidden) {
    if (ContentItem   != NULL && !ContentItem->Hidden) {
      DEConOutSetAttribute (QuestionPanel->ColorAttribute);
      DisplayString (QuestionPanel->PanelField.left, ContentItem->ItemField.bottom + 1, SeparationStr);
    }

    DisplayPanelItem (QuestionPanel, ButtonItem);
  }

  DEConOutSetAttribute (QuestionPanel->ColorAttribute);
  PanelItem = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP);
  if (PanelItem != NULL) {
    DisplayPanelItem (QuestionPanel, PanelItem);
  }

  PanelItem = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN);
  if (PanelItem != NULL) {
    DisplayPanelItem (QuestionPanel, PanelItem);
  }

  PanelItem = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_UP);
  if (PanelItem != NULL) {
    DisplayPanelItem (QuestionPanel, PanelItem);
  }

  PanelItem = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_DOWN);
  if (PanelItem != NULL) {
    DisplayPanelItem (QuestionPanel, PanelItem);
  }

  FreePool (SeparationStr);
  return EFI_SUCCESS;
}

/**
 Shutdown question panel

 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Shutdown question panel successful.

**/
EFI_STATUS
ShutdownQuestionPanel (
  IN H2O_LTDE_PANEL                           *QuestionPanel
  )
{
  if (QuestionPanel != NULL) {
    FreePanelItemList (QuestionPanel->ItemList, QuestionPanel->ItemCount);
    QuestionPanel->ItemList  = NULL;
    QuestionPanel->ItemCount = 0;
  }

  return EFI_SUCCESS;
}

/**
 Process page down for content item

 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Process page down successful.
 @retval EFI_INVALID_PARAMETER   QuestionPanel is NULL.
 @retval EFI_ABORTED             Can not find panel item.

**/
STATIC
EFI_STATUS
ContentItemPageDown (
  IN H2O_LTDE_PANEL                           *QuestionPanel
  )
{
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_PANEL_ITEM                         *PageUpItem;
  H2O_LTDE_PANEL_ITEM                         *PageDownItem;
  H2O_LTDE_PANEL_ITEM                         *SelectedPanelItem;
  INT32                                       ContentItemHeight;

  if (QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem  = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  PageUpItem   = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP);
  PageDownItem = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN);
  if (ContentItem == NULL || PageUpItem == NULL || PageDownItem == NULL) {
    return EFI_ABORTED;
  }

  ContentItemHeight = H2O_LTDE_FIELD_HEIGHT(&ContentItem->ItemField);
  if (ContentItem->CurrentPos + ContentItemHeight > ContentItem->MaxPos) {
    return EFI_SUCCESS;
  }

  PageUpItem->Hidden       = FALSE;
  ContentItem->CurrentPos += ContentItemHeight;

  if (ContentItem->CurrentPos + ContentItemHeight > ContentItem->MaxPos) {
    PageDownItem->Hidden = TRUE;
    SelectedPanelItem = GetPanelItemByControl (QuestionPanel, QuestionPanel->SelectedControl);
    if (QuestionPanel->SelectedControl != NULL &&
        SelectedPanelItem != NULL &&
        SelectedPanelItem->ItemId == LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN) {
      QuestionPanel->SelectedControl = GetNextSelectableControl (PageUpItem, NULL, FALSE);
    }
  }

  DisplayQuestionPanel (QuestionPanel);

  return EFI_SUCCESS;
}

/**
 Process page up for content item

 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Process page up successful.
 @retval EFI_INVALID_PARAMETER   QuestionPanel is NULL.
 @retval EFI_ABORTED             Can not find panel item.

**/
EFI_STATUS
ContentItemPageUp (
  IN H2O_LTDE_PANEL                           *QuestionPanel
  )
{
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_PANEL_ITEM                         *PageUpItem;
  H2O_LTDE_PANEL_ITEM                         *PageDownItem;
  H2O_LTDE_PANEL_ITEM                         *SelectedPanelItem;
  INT32                                       ContentItemHeight;

  if (QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem  = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  PageUpItem   = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP);
  PageDownItem = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN);
  if (ContentItem == NULL || PageUpItem == NULL || PageDownItem == NULL) {
    return EFI_ABORTED;
  }

  if (ContentItem->CurrentPos == 0) {
    return EFI_SUCCESS;
  }

  PageDownItem->Hidden = FALSE;

  ContentItemHeight = H2O_LTDE_FIELD_HEIGHT(&ContentItem->ItemField);
  if (ContentItem->CurrentPos < ContentItemHeight) {
    ContentItem->CurrentPos = 0;
  } else {
    ContentItem->CurrentPos -= ContentItemHeight;
  }

  if (ContentItem->CurrentPos == 0) {
    PageUpItem->Hidden = TRUE;
    SelectedPanelItem = GetPanelItemByControl (QuestionPanel, QuestionPanel->SelectedControl);
    if (QuestionPanel->SelectedControl != NULL &&
        SelectedPanelItem != NULL &&
        SelectedPanelItem->ItemId == LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP) {
      QuestionPanel->SelectedControl = GetNextSelectableControl (PageDownItem, NULL, FALSE);
    }
  }

  DisplayQuestionPanel (QuestionPanel);

  return EFI_SUCCESS;
}

/**
 Get default highlight control in question panel

 @param[in] QuestionPanel        A pointer to question panel

 @return The default highlight control or NULL if not found ot QuestionPanel is NULL.

**/
H2O_LTDE_CONTROL *
QuestionPanelGetDefaultHighlight (
  IN H2O_LTDE_PANEL                           *QuestionPanel
  )
{
  H2O_LTDE_CONTROL                            *HighlightControl;
  H2O_LTDE_CONTROL                            *Control;
  H2O_LTDE_PANEL_ITEM                         *PanelItem;

  if (QuestionPanel == NULL) {
    return NULL;
  }

  HighlightControl = NULL;
  Control          = NULL;
  while (TRUE) {
    Control   = GetNextSelectableControlInQuestionPanel (QuestionPanel, Control, FALSE);
    PanelItem = GetPanelItemByControl (QuestionPanel, Control);
    if (Control == NULL || PanelItem == NULL) {
      break;
    }

    if (HighlightControl == NULL) {
      HighlightControl = Control;
    }

    if (PanelItem->ItemId == LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP   ||
        PanelItem->ItemId == LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN ||
        PanelItem->ItemId == LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_UP ||
        PanelItem->ItemId == LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_DOWN) {
      continue;
    }

    HighlightControl = Control;
    break;
  }

  return HighlightControl;
}

/**
 Process open dialog event in question panel

 @param[in] Dialog               A pointer to dialog data
 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Process open dialog event successful.
 @retval EFI_INVALID_PARAMETER   Dialog or QuestionPanel is NULL.

**/
EFI_STATUS
QuestionPanelProcessOpenDEvt (
  IN H2O_FORM_BROWSER_D                       *Dialog,
  IN H2O_LTDE_PANEL                           *QuestionPanel
  )
{
  EFI_STATUS                                  Status;

  if (Dialog == NULL || QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPanelColorAttribute (QuestionPanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, GetPseudoClass (Dialog->DialogType), &QuestionPanel->ColorAttribute);
  if (EFI_ERROR (Status)) {
    QuestionPanel->ColorAttribute = PANEL_COLOR_ATTRIBUTE_QUESTION;
  }

  return EFI_SUCCESS;
}

/**
 Process user input in question panel

 @param[in]  QuestionPanel        A pointer to question panel
 @param[in]  UserInputData        A pointer to user input data
 @param[out] SelectedControl      A pointer to user selected control
 @param[out] InputString          A pointer to user input string
 @param[out] IsShutdownDialog     A pointer to flag which determine to shutdown dialog

 @retval EFI_SUCCESS              Process user input successful.
 @retval EFI_INVALID_PARAMETER    Input parameter is NULL.
 @retval EFI_ABORTED              Current highlight control is NULL.

**/
EFI_STATUS
QuestionPanelProcessUserInput (
  IN  H2O_LTDE_PANEL                          *QuestionPanel,
  IN  H2O_DISPLAY_ENGINE_USER_INPUT_DATA      *UserInputData,
  OUT H2O_LTDE_CONTROL                        **SelectedControl,
  OUT CHAR16                                  **InputString,
  OUT BOOLEAN                                 *IsShutdownDialog
  )
{
  UINTN                                       StringLength;
  EFI_INPUT_KEY                               *InputKey;
  H2O_LTDE_CONTROL                            *CurrentControl;
  H2O_LTDE_CONTROL                            *NextControl;
  H2O_LTDE_PANEL_ITEM                         *PanelItem;
  EFI_STATUS                                  Status;
  CHAR16                                      *NewInputString;
  RECT                                        ControlAbsField;


  if (UserInputData == NULL || QuestionPanel == NULL || SelectedControl == NULL || InputString == NULL || IsShutdownDialog == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  InputKey       = &UserInputData->KeyData.Key;
  NextControl    = NULL;
  NewInputString = NULL;

  if ((InputKey->ScanCode    == SCAN_ESC) ||
      (InputKey->UnicodeChar == CHAR_CARRIAGE_RETURN && QuestionPanel->SelectedControl == NULL)) {
    *SelectedControl  = NextControl;
    *InputString      = NewInputString;
    *IsShutdownDialog = TRUE;
    return EFI_SUCCESS;
  }

  if (UserInputData->IsKeyboard) {
    CurrentControl = QuestionPanel->SelectedControl;
    if (CurrentControl == NULL) {
      return EFI_ABORTED;
    }

    switch (InputKey->UnicodeChar) {

    case CHAR_TAB:
      NextControl = CurrentControl;
      while (TRUE) {
        NextControl = GetNextSelectableControlInQuestionPanel (QuestionPanel, NextControl, TRUE);
        if (NextControl == NULL ||
            NextControl == CurrentControl ||
            (NextControl->ControlId != LTDE_CONTROL_ID_DIALOG_HOT_KEY_PAGE_UP   &&
             NextControl->ControlId != LTDE_CONTROL_ID_DIALOG_HOT_KEY_PAGE_DOWN &&
             NextControl->ControlId != LTDE_CONTROL_ID_DIALOG_HOT_KEY_SCROLL_UP &&
             NextControl->ControlId != LTDE_CONTROL_ID_DIALOG_HOT_KEY_SCROLL_DOWN)) {
          break;
        }
      }
      break;

    case CHAR_BACKSPACE:
      if (CurrentControl->ControlId == LTDE_CONTROL_ID_DIALOG_BODY_INPUT) {
        StringLength = StrLen (CurrentControl->Text.String);
        if (StringLength > 0) {
          NewInputString = AllocateZeroPool (StringLength * sizeof (CHAR16));
          CopyMem (NewInputString, CurrentControl->Text.String, (StringLength - 1) * sizeof (CHAR16));
        }
      }
      break;

    case CHAR_CARRIAGE_RETURN:
      switch (CurrentControl->ControlId) {

      case LTDE_CONTROL_ID_DIALOG_BODY_INPUT:
        NextControl = GetNextSelectableControlInQuestionPanel (QuestionPanel, CurrentControl, FALSE);
        if (NextControl == NULL) {
          NextControl = CurrentControl;
        }
        break;

      case LTDE_CONTROL_ID_DIALOG_BODY:
      case LTDE_CONTROL_ID_DIALOG_BUTTON:
        NextControl = CurrentControl;
        break;

      case LTDE_CONTROL_ID_DIALOG_HOT_KEY_PAGE_UP:
        ContentItemPageUp (QuestionPanel);
        break;

      case LTDE_CONTROL_ID_DIALOG_HOT_KEY_PAGE_DOWN:
        ContentItemPageDown (QuestionPanel);
        break;
      }
      break;

    case CHAR_NULL:
      switch (InputKey->ScanCode) {

      case SCAN_PAGE_UP:
        ContentItemPageUp (QuestionPanel);
        break;

      case SCAN_PAGE_DOWN:
        ContentItemPageDown (QuestionPanel);
        break;

      case SCAN_RIGHT:
      case SCAN_LEFT:
        PanelItem = GetPanelItemByControl (QuestionPanel, CurrentControl);
        if (PanelItem == NULL || PanelItem->Vertical) {
          return EFI_ABORTED;
        }

        if (InputKey->ScanCode == SCAN_RIGHT) {
          NextControl = GetNextSelectableControl (PanelItem, CurrentControl, FALSE);
        } else {
          NextControl = GetPreviousSelectableControl (PanelItem, CurrentControl, FALSE);
        }
        break;

      case SCAN_UP:
      case SCAN_DOWN:
        PanelItem = GetPanelItemByControl (QuestionPanel, CurrentControl);
        if (PanelItem == NULL ||
            !PanelItem->Vertical ||
            CurrentControl->ControlId == LTDE_CONTROL_ID_DIALOG_BODY_INPUT) {
          return EFI_ABORTED;
        }

        if (InputKey->ScanCode == SCAN_UP) {
          NextControl = GetPreviousSelectableControl (PanelItem, CurrentControl, FALSE);
        } else {
          NextControl = GetNextSelectableControl (PanelItem, CurrentControl, FALSE);
        }
        break;
      }
      break;

    default:
      if (CurrentControl->ControlId == LTDE_CONTROL_ID_DIALOG_BODY_INPUT) {
        if (IsVisibleChar (InputKey->UnicodeChar)) {
          NewInputString = CatSPrint (NULL, L"%s%c", CurrentControl->Text.String, InputKey->UnicodeChar);
          break;
        }
      }
      return EFI_ABORTED;
    }
  } else {
    //
    // Process mouse event
    //
    if (InputKey->UnicodeChar != CHAR_CARRIAGE_RETURN) {
      return EFI_NOT_FOUND;
    }

    Status = GetControlByMouse (QuestionPanel, UserInputData->CursorX, UserInputData->CursorY, &PanelItem, &NextControl);
    if (EFI_ERROR (Status) || !NextControl->Selectable) {
      return EFI_NOT_FOUND;
    }

    if (NextControl == QuestionPanel->SelectedControl) {
      switch (NextControl->ControlId) {

      case LTDE_CONTROL_ID_DIALOG_HOT_KEY_PAGE_UP:
        ContentItemPageUp (QuestionPanel);
        NextControl = NULL;
        break;

      case LTDE_CONTROL_ID_DIALOG_HOT_KEY_PAGE_DOWN:
        ContentItemPageDown (QuestionPanel);
        NextControl = NULL;
        break;

      case LTDE_CONTROL_ID_DIALOG_BODY_INPUT:
        CopyRect (&ControlAbsField, &NextControl->ControlField);
        OffsetRect (&ControlAbsField, PanelItem->ItemField.left, PanelItem->ItemField.top - PanelItem->CurrentPos);
        InitializeVirtualKb (&ControlAbsField);
        NextControl = NULL;
        break;
      }
    }
  }

  *SelectedControl   = NextControl;
  *InputString       = NewInputString;
  *IsShutdownDialog  = FALSE;

  return EFI_SUCCESS;
}

/**
 Process open dialog event for simple dialog

 @param[in] Dialog               A pointer to dialog data
 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Process open dialog event successful.
 @retval EFI_INVALID_PARAMETER   Dialog or QuestionPanel is NULL.
 @retval EFI_OUT_OF_RESOURCES    Allocate pool fail.

**/
STATIC
EFI_STATUS
SimpleDialogProcessOpenDEvt (
  IN H2O_FORM_BROWSER_D                       *Dialog,
  IN H2O_LTDE_PANEL                           *QuestionPanel
  )
{
  UINT32                                      ItemCount;
  H2O_LTDE_PANEL_ITEM                         *ItemList;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  EFI_STATUS                                  Status;
  BOOLEAN                                     IsHelpDialog;

  if (Dialog == NULL || QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  IsHelpDialog = (GetPseudoClass (Dialog->DialogType) == H2O_STYLE_PSEUDO_CLASS_HELP);

  ItemCount = IsHelpDialog ? 5 : 4;
  ItemList  = (H2O_LTDE_PANEL_ITEM *) AllocateZeroPool (sizeof (H2O_LTDE_PANEL_ITEM) * ItemCount);
  if (ItemList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ItemCount = 0;
  if (IsHelpDialog) {
    QuestionPanelInitTitleItem (Dialog, &ItemList[ItemCount++]);
  }
  ContentItem = &ItemList[ItemCount];
  QuestionPanelInitContentItem (Dialog, &ItemList[ItemCount++]);
  QuestionPanelInitButtonItem (Dialog, &ItemList[ItemCount++]);
  if (ContentItem->Selectable) {
    QuestionPanelInitContentHotKeyItem (Dialog, LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_UP  , &ItemList[ItemCount++]);
    QuestionPanelInitContentHotKeyItem (Dialog, LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_DOWN, &ItemList[ItemCount++]);
  } else {
    QuestionPanelInitContentHotKeyItem (Dialog, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP  , &ItemList[ItemCount++]);
    QuestionPanelInitContentHotKeyItem (Dialog, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN, &ItemList[ItemCount++]);
  }

  FreePanelItemList (QuestionPanel->ItemList, QuestionPanel->ItemCount);
  QuestionPanel->ItemCount = ItemCount;
  QuestionPanel->ItemList  = ItemList;

  Status = QuestionPanelUpdateField (Dialog, QuestionPanel);
  QuestionPanel->SelectedControl = QuestionPanelGetDefaultHighlight (QuestionPanel);

  return DisplayQuestionPanel (QuestionPanel);
}

/**
 Process event for simple dialog

 @param[in] Notify               A pointer to notify event
 @param[in] UserInputData        A pointer to user input data
 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Process open dialog event successful.
 @retval EFI_UNSUPPORTED         Event type is not supported.
 @retval Other                   Process event fail.

**/
STATIC
EFI_STATUS
SimpleDialogProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  )
{
  EFI_STATUS                                  Status;
  H2O_LTDE_CONTROL                            *SelectedControl;
  CHAR16                                      *UpdatedString;
  BOOLEAN                                     IsShutdownDialog;

  Status = EFI_UNSUPPORTED;

  switch (Notify->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_D:
    return SimpleDialogProcessOpenDEvt (&((H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify)->Dialog, QuestionPanel);

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_D:
    return ShutdownQuestionPanel (QuestionPanel);

  case H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE:
    if (UserInputData == NULL || mDEPrivate->DEStatus != DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG) {
      return Status;
    }

    Status = QuestionPanelProcessUserInput (
               QuestionPanel,
               UserInputData,
               &SelectedControl,
               &UpdatedString,
               &IsShutdownDialog
               );
    if (EFI_ERROR (Status)) {
      break;
    }

    if (SelectedControl != NULL) {
      if (SelectedControl == QuestionPanel->SelectedControl) {
        Status = SendChangeQNotify (0, 0, &QuestionPanel->SelectedControl->HiiValue);
      } else {
        switch (SelectedControl->ControlId) {

        case LTDE_CONTROL_ID_DIALOG_HOT_KEY_PAGE_UP:
          if (!UserInputData->IsKeyboard) {
            ContentItemPageUp (QuestionPanel);
          }
          break;

        case LTDE_CONTROL_ID_DIALOG_HOT_KEY_PAGE_DOWN:
          if (!UserInputData->IsKeyboard) {
            ContentItemPageDown (QuestionPanel);
          }
          break;

        default:
          DisplayQuestionPanelControl (QuestionPanel, QuestionPanel->SelectedControl, FALSE);
          QuestionPanel->SelectedControl = SelectedControl;
          DisplayQuestionPanelControl (QuestionPanel, QuestionPanel->SelectedControl, TRUE);
          break;
        }
      }
    }

    if (IsShutdownDialog) {
      Status = SendShutDNotify ();
    }
    break;

  default:
    break;
  }

  return Status;
}

/**
 Get dialog information

 @param[in]  QuestionPanel       A pointer to question panel
 @param[in]  Notify              A pointer to notify event
 @param[out] IsQuestion          A pointer to flag to determine it is question dialog or not
 @param[out] QuestionOpCode      A pointer to question opcode

 @retval EFI_SUCCESS             Get dialog information successful.
 @retval EFI_INVALID_PARAMETER   Input parameter is NULL.

**/
STATIC
EFI_STATUS
GetDialogInfo (
  IN       H2O_LTDE_PANEL                     *QuestionPanel,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  OUT      BOOLEAN                            *IsQuestion,
  OUT      UINT8                              *QuestionOpCode
  )
{
  H2O_DISPLAY_ENGINE_EVT_OPEN_D               *OpenDNotify;

  if (QuestionPanel == NULL || Notify == NULL || IsQuestion == NULL || QuestionOpCode == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get dialog info from OPEN_D event because ControlArray of Question Panel is not initialized.
  //
  if (Notify->Type == H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_D) {
    OpenDNotify = (H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify;

    *IsQuestion     = ((OpenDNotify->Dialog.DialogType & H2O_FORM_BROWSER_D_TYPE_QUESTIONS) == 0) ? FALSE : TRUE;
    *QuestionOpCode = *IsQuestion ? GetOpCodeByDialogType (OpenDNotify->Dialog.DialogType) : 0;
    return EFI_SUCCESS;
  }

  //
  // Get dialog info from ControlArray of Question Panel.
  //
  if (QuestionPanel->SelectedControl != NULL) {
    *IsQuestion     = (QuestionPanel->SelectedControl->Operand != 0) ? TRUE : FALSE;
    *QuestionOpCode = QuestionPanel->SelectedControl->Operand;
  } else {
    *IsQuestion     = FALSE;
    *QuestionOpCode = 0;
  }

  return EFI_SUCCESS;
}

/**
 Process event in question function

 @param[in] Notify               A pointer to notify event
 @param[in] UserInputData        A pointer to user input data

 @retval EFI_SUCCESS             Process event successful.
 @retval Other                   Process event fail.

**/
EFI_STATUS
ProcessEvtInQuestionFunc (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_Q                          *Question;
  H2O_DISPLAY_ENGINE_EVT_SHUT_Q               *ShutQNotify;
  H2O_LTDE_PANEL                              *QuestionPanel;
  H2O_LTDE_PANEL                              *SetupPagePanel;
  BOOLEAN                                     IsQuestion;
  UINT8                                       QuestionOpCode;

  QuestionPanel  = GetPanel (H2O_PANEL_TYPE_QUESTION);
  QuestionOpCode = 0;

  if (mDEPrivate->DEStatus == DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG) {
    Status = GetDialogInfo (QuestionPanel, Notify, &IsQuestion, &QuestionOpCode);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (!IsQuestion) {
      return SimpleDialogProcessEvt (Notify, UserInputData, QuestionPanel);
    }
  } else {
    SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
    if (SetupPagePanel != NULL && SetupPagePanel->SelectedControl != NULL) {
      QuestionOpCode = SetupPagePanel->SelectedControl->Operand;
    }

    if (Notify->Type == H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_Q) {
      ShutQNotify = (H2O_DISPLAY_ENGINE_EVT_SHUT_Q *) Notify;
      //
      // SHUT_Q status must get another specific question
      //
      Status = mDEPrivate->FBProtocol->GetQInfo (mDEPrivate->FBProtocol, ShutQNotify->PageId, ShutQNotify->QuestionId, &Question);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      QuestionOpCode = Question->Operand;
      SafeFreePool ((VOID **) &Question);
    }
  }

  switch (QuestionOpCode) {

  case EFI_IFR_CHECKBOX_OP:
    Status = CheckboxOpCodeProcessEvt (Notify, UserInputData, QuestionPanel);
    break;

  case EFI_IFR_NUMERIC_OP:
    Status = NumericOpCodeProcessEvt (Notify, UserInputData, QuestionPanel);
    break;

  case EFI_IFR_ONE_OF_OP:
    Status = OneOfQuestionProcessEvt (Notify, UserInputData, QuestionPanel);
    break;

  case EFI_IFR_ORDERED_LIST_OP:
    Status = OrderListOpCodeProcessEvt (Notify, UserInputData, QuestionPanel);
    break;

  case EFI_IFR_STRING_OP:
    Status = StringOpCodeProcessEvt (Notify, UserInputData, QuestionPanel);
    break;

  case EFI_IFR_PASSWORD_OP:
    Status = PasswordOpCodeProcessEvt (Notify, UserInputData, QuestionPanel);
    break;

  case EFI_IFR_DATE_OP:
  case EFI_IFR_TIME_OP:
    Status = DateTimeOpCodeProcessEvt (Notify, UserInputData, QuestionPanel);
    break;

  case EFI_IFR_REF_OP:
  case EFI_IFR_ACTION_OP:
  case EFI_IFR_RESET_BUTTON_OP:
  default:
    Status = EFI_NOT_FOUND;
    break;
  }

  return Status;
}



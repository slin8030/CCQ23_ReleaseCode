/** @file
  Ordered list question related functions.

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

#include "LTDEQuestions.h"
#include "LTDEPanels.h"
#include "LTDEPrint.h"
#include "LTDEMisc.h"

STATIC
UINT8
GetHiiBufferValueWidth (
  IN UINT8                                    Type
  )
{
  switch (Type) {

  case EFI_IFR_TYPE_NUM_SIZE_8:
    return 1;

  case EFI_IFR_TYPE_NUM_SIZE_16:
    return 2;

  case EFI_IFR_TYPE_NUM_SIZE_32:
    return 4;

  case EFI_IFR_TYPE_NUM_SIZE_64:
    return 8;

  default:
    break;
  }

  return 1;
}

CHAR16 *
OrderListOpCodeCreatePromptStr (
  IN H2O_FORM_BROWSER_Q                       *Question
  )
{
  UINT32                                      OptionIndex;
  UINT32                                      ContainerIndex;
  H2O_FORM_BROWSER_O                          *Option;
  CHAR16                                      *TempStr;
  CHAR16                                      *ResultStr;
  UINT64                                      Value;
  UINT8                                       ValueType;

  if (Question == NULL || Question->Operand != EFI_IFR_ORDERED_LIST_OP) {
    return NULL;
  }

  if (Question->NumberOfOptions == 0 || Question->Options == NULL) {
    return NULL;
  }

  ValueType = Question->Options[0].HiiValue.Type;
  ResultStr = AllocateZeroPool (sizeof (CHAR16));
  if (ResultStr == NULL) {
    return NULL;
  }

  for (ContainerIndex = 0; ContainerIndex < Question->ContainerCount; ContainerIndex++) {
    Value = GetHiiBufferValue (Question->HiiValue.Buffer, ValueType, ContainerIndex);
    if (Value == 0) {
      break;
    }

    for (OptionIndex = 0; OptionIndex < Question->NumberOfOptions; OptionIndex++) {
      Option = &(Question->Options[OptionIndex]);
      if (!Option->Visibility || Option->HiiValue.Value.u64 != Value) {
        continue;
      }

      TempStr = ResultStr;
      if (*ResultStr == CHAR_NULL) {
        ResultStr = AllocateCopyPool (StrSize (Option->Text), Option->Text);
      } else {
        ResultStr = CatSPrint (NULL, L"%s%s%s", ResultStr, L"\n", Option->Text);
      }
      FreePool (TempStr);
      if (ResultStr == NULL) {
        return NULL;
      }
      break;
    }
  }

  return ResultStr;
}

EFI_STATUS
OrderListOpCodeDisplayValueStr (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_PANEL_ITEM                      *ContentItem,
  IN H2O_LTDE_CONTROL                         *Control
  )
{
  EFI_STATUS                                  Status;
  UINT32                                      Index;
  UINT32                                      HighlightStartIndex;
  UINT32                                      HighlightEndIndex;
  UINT32                                      PromptWidth;
  UINT32                                      PromptHeight;
  UINT32                                      PromptStringNum;
  CHAR16                                      **PromptStringArray;
  CHAR16                                      *PromptStr;
  CHAR16                                      *OptionStr;
  CHAR16                                      *NextOptionStr;
  CHAR16                                      *String;
  UINT8                                       SequenceIndex;
  UINT32                                      NormalAttribute;
  UINT32                                      HighlightAttribute;
  BOOLEAN                                     IsHighlight;
  INT32                                       StartY;
  INT32                                       EndY;

  if (SetupPagePanel == NULL || ContentItem == NULL || Control == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  StartY = ContentItem->CurrentPos;
  EndY   = StartY + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - 1;
  if (!IS_OVERLAP (Control->ControlField.top, Control->ControlField.bottom, StartY, EndY)) {
    return EFI_SUCCESS;
  }

  if (!EqualRect (&Control->ControlField, &Control->Text.StringField)) {
    //
    // BUGBUG: clean empty region
    //
  }

  IsHighlight = IsCurrentHighlight (Control->PageId, Control->QuestionId, Control->IfrOpCode);
  GetPanelColorAttribute (SetupPagePanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, Control->ControlStyle.PseudoClass, &NormalAttribute);
  GetPanelColorAttribute (SetupPagePanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_HIGHLIGHT , &HighlightAttribute);

  PromptWidth  = H2O_LTDE_FIELD_WIDTH (&Control->Text.StringField);
  PromptHeight = H2O_LTDE_FIELD_HEIGHT (&Control->Text.StringField);

  HighlightStartIndex = 0;
  HighlightEndIndex   = 0;
  if (IsHighlight) {
    //
    // Find highlight range which are HighlightStartIndex and HighlightEndIndex.
    //
    PromptStr = AllocateCopyPool (StrSize (Control->Text.String), Control->Text.String);
    if (PromptStr == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    SequenceIndex       = 0;
    Index               = 0;
    OptionStr           = PromptStr;
    while (TRUE) {
      NextOptionStr = StrStr (OptionStr, L"\n");
      if (NextOptionStr != NULL) {
        *NextOptionStr = CHAR_NULL;
      }

      PromptStringNum = GetStringHeight (OptionStr, PromptWidth);
      if (SequenceIndex == Control->Sequence) {
        HighlightStartIndex = Index;
        HighlightEndIndex   = Index + PromptStringNum - 1;
        break;
      }

      if (NextOptionStr == NULL) {
        break;
      }

      SequenceIndex++;
      Index    += PromptStringNum;
      OptionStr = NextOptionStr + 1;
    }
    FreePool (PromptStr);
  }

  Status = GetStringArrayByWidth (Control->Text.String, PromptWidth, &PromptStringNum, &PromptStringArray);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < PromptHeight; Index++) {
    if (!IN_RANGE ((INT32) (Control->ControlField.top + Index), StartY, EndY)) {
      continue;
    }

    DEConOutSetAttribute ((IsHighlight && Index >= HighlightStartIndex && Index <= HighlightEndIndex) ? HighlightAttribute : NormalAttribute);

    if (Index < PromptStringNum) {
      String = GetAlignmentString (PromptStringArray[Index], PromptWidth, LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT);
    } else {
      String = CreateString (PromptWidth, ' ');
    }

    if (String != NULL) {
      DisplayString (
        ContentItem->ItemField.left,
        ContentItem->ItemField.top + (Control->ControlField.top - ContentItem->CurrentPos) + Index,
        String
        );

      FreePool (String);
    }
  }

  if (PromptStringArray != NULL) {
    for (Index = 0; Index < PromptStringNum; Index++) {
      FreePool (PromptStringArray[Index]);
    }
    FreePool (PromptStringArray);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
OrderListOpCodeShiftOrder (
  IN H2O_LTDE_CONTROL                         *Control,
  IN BOOLEAN                                  ShiftNext
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_Q                          *Question;
  UINT64                                      CurrentValue;
  UINT64                                      NextValue;
  UINT8                                       Type;

  if (Control == NULL || Control->Operand != EFI_IFR_ORDERED_LIST_OP) {
    return EFI_INVALID_PARAMETER;
  }

  Status = mDEPrivate->FBProtocol->GetQInfo (mDEPrivate->FBProtocol, Control->PageId, Control->QuestionId, &Question);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  switch (Question->HiiValue.BufferLen / Question->ContainerCount) {

  case 1:
    Type = EFI_IFR_TYPE_NUM_SIZE_8;
    break;

  case 2:
    Type = EFI_IFR_TYPE_NUM_SIZE_16;
    break;

  case 4:
    Type = EFI_IFR_TYPE_NUM_SIZE_32;
    break;
  case 8:
    Type = EFI_IFR_TYPE_NUM_SIZE_64;
    break;

  default:
    return EFI_ABORTED;
  }

  if (ShiftNext) {
    if (Control->Sequence == Question->NumberOfOptions - 1) {
      FreePool ((VOID *) Question);
      return EFI_SUCCESS;
    }

    CurrentValue = GetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence);
    NextValue    = GetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence + 1);

    SetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence    , NextValue);
    SetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence + 1, CurrentValue);
    Control->Sequence++;
  } else {
    if (Control->Sequence == 0) {
      FreePool ((VOID *) Question);
      return EFI_SUCCESS;
    }

    CurrentValue = GetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence);
    NextValue    = GetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence - 1);

    SetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence    , NextValue);
    SetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence - 1, CurrentValue);
    Control->Sequence--;
  }

  FreePool ((VOID *) Question);

  return SendChangeQNotify (Control->PageId, Control->QuestionId, &Control->HiiValue);
}

STATIC
EFI_STATUS
OrderListOpCodeShiftOrderInDlg (
  IN H2O_LTDE_PANEL                           *QuestionPanel,
  IN BOOLEAN                                  ShiftNext
  )
{
  UINT32                                      Index;
  UINT64                                      CurrentOptionValue;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_CONTROL                            *Control;
  H2O_LTDE_CONTROL                            *Control1;
  H2O_LTDE_CONTROL                            *Control2;
  H2O_LTDE_CONTROL                            *OrgControl1;
  H2O_LTDE_CONTROL                            *OrgControl2;

  if (QuestionPanel == NULL || QuestionPanel->SelectedControl == NULL || QuestionPanel->SelectedControl->ControlId != LTDE_CONTROL_ID_DIALOG_BODY) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem == NULL) {
    return EFI_ABORTED;
  }

  Control1           = NULL;
  Control2           = NULL;
  CurrentOptionValue = QuestionPanel->SelectedControl->HiiValue.Value.u64;

  for (Index = 0; Index < ContentItem->ControlCount; Index++) {
    Control = &ContentItem->ControlList[Index];
    if (Control->ControlId != LTDE_CONTROL_ID_DIALOG_BODY) {
      continue;
    }

    if (ShiftNext && Control1 != NULL) {
      Control2 = Control;
      break;
    }

    if (Control->HiiValue.Value.u64 == CurrentOptionValue) {
      Control1 = Control;
      if (!ShiftNext) {
        break;
      }
    }

    if (!ShiftNext) {
      Control2 = Control;
    }
  }

  if (Control1 == NULL || Control2 == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // Exchange controls
  //
  OrgControl1 = AllocateCopyPool (sizeof (H2O_LTDE_CONTROL), Control1);
  OrgControl2 = AllocateCopyPool (sizeof (H2O_LTDE_CONTROL), Control2);
  if (OrgControl1 == NULL || OrgControl2 == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (Control1, OrgControl2, sizeof (H2O_LTDE_CONTROL));
  CopyMem (Control2, OrgControl1, sizeof (H2O_LTDE_CONTROL));

  Control1->ControlField.top    = OrgControl1->ControlField.top;
  Control1->ControlField.bottom = OrgControl1->ControlField.top + (OrgControl2->ControlField.bottom - OrgControl2->ControlField.top);

  Control2->ControlField.top    = OrgControl2->ControlField.top;
  Control2->ControlField.bottom = OrgControl2->ControlField.top + (OrgControl1->ControlField.bottom - OrgControl1->ControlField.top);

  FreePool (OrgControl1);
  FreePool (OrgControl2);

  QuestionPanel->SelectedControl = Control2;
  if (UpdatePanelContentItemPos (QuestionPanel)) {
    DisplayQuestionPanel (QuestionPanel);
  } else {
    QuestionPanel->SelectedControl = Control1;
    DisplayQuestionPanelControl (QuestionPanel, QuestionPanel->SelectedControl, FALSE);
    QuestionPanel->SelectedControl = Control2;
    DisplayQuestionPanelControl (QuestionPanel, QuestionPanel->SelectedControl, TRUE);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
OrderListOpCodeGetHiiValue (
  IN  H2O_LTDE_PANEL                          *QuestionPanel,
  OUT EFI_HII_VALUE                           *HiiValue
  )
{
  UINT32                                      Index;
  UINT8                                       HiiValueType;
  UINT16                                      OptionCount;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;

  if (QuestionPanel == NULL || HiiValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem == NULL) {
    return EFI_ABORTED;
  }

  HiiValueType = 0;
  OptionCount  = 0;

  for (Index = 0; Index < ContentItem->ControlCount; Index++) {
    if (ContentItem->ControlList[Index].ControlId == LTDE_CONTROL_ID_DIALOG_BODY) {
      HiiValueType = ContentItem->ControlList[Index].HiiValue.Type;
      OptionCount++;
    }
  }

  if (OptionCount == 0) {
    return EFI_NOT_FOUND;
  }

  ZeroMem (HiiValue, sizeof (EFI_HII_VALUE));
  HiiValue->Type      = EFI_IFR_TYPE_BUFFER;
  HiiValue->BufferLen = OptionCount * GetHiiBufferValueWidth (HiiValueType);
  HiiValue->Buffer    = AllocateZeroPool (HiiValue->BufferLen);
  if (HiiValue->Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  OptionCount = 0;
  for (Index = 0; Index < ContentItem->ControlCount; Index++) {
    if (ContentItem->ControlList[Index].ControlId == LTDE_CONTROL_ID_DIALOG_BODY) {
      SetHiiBufferValue (HiiValue->Buffer, HiiValueType, OptionCount++, ContentItem->ControlList[Index].HiiValue.Value.u64);
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
OrderListOpCodeUpdateCurrentPos (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_PANEL_ITEM                      *ContentItem
  )
{
  INT32                                       OptionStartY;
  INT32                                       OptionEndY;
  INT32                                       ControlStartY;
  INT32                                       ControlEndY;
  CHAR16                                      *PromptStr;
  CHAR16                                      *PromptStrPtr;
  CHAR16                                      *OptionStr;
  UINT32                                      ControlFieldWidth;
  UINT8                                       Sequence;

  if (SetupPagePanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (SetupPagePanel->SelectedControl == NULL || SetupPagePanel->SelectedControl->Operand != EFI_IFR_ORDERED_LIST_OP) {
    return EFI_UNSUPPORTED;
  }

  PromptStr = AllocateCopyPool (StrSize (SetupPagePanel->SelectedControl->Text.String), SetupPagePanel->SelectedControl->Text.String);
  if (PromptStr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ControlFieldWidth = H2O_LTDE_FIELD_WIDTH (&SetupPagePanel->SelectedControl->ControlField);
  Sequence          = 0;
  OptionStartY      = SetupPagePanel->SelectedControl->ControlField.top;
  OptionEndY        = OptionStartY;
  OptionStr         = PromptStr;
  while (TRUE) {
    PromptStrPtr = StrStr (OptionStr, L"\n");
    if (PromptStrPtr != NULL) {
      *PromptStrPtr = CHAR_NULL;
    }

    OptionEndY = OptionStartY + GetStringHeight (OptionStr, ControlFieldWidth) - 1;
    if (Sequence == SetupPagePanel->SelectedControl->Sequence) {
      break;
    }

    if (PromptStrPtr == NULL) {
      break;
    }

    Sequence++;
    OptionStartY = OptionEndY + 1;
    OptionStr    = PromptStrPtr + 1;
  }

  if (Sequence == SetupPagePanel->SelectedControl->Sequence) {
    ControlStartY = ContentItem->CurrentPos;
    ControlEndY   = ContentItem->CurrentPos + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - 1;

    if (!IN_RANGE (OptionStartY, ControlStartY, ControlEndY)) {
      ContentItem->CurrentPos = (OptionStartY < ContentItem->CurrentPos) ? OptionStartY : (OptionEndY - H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) + 1);
    } else if (!IN_RANGE (OptionEndY, ControlStartY, ControlEndY)) {
      ContentItem->CurrentPos = (OptionEndY - H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) + 1);
    }
  }

  FreePool (PromptStr);
  return EFI_SUCCESS;
}

EFI_STATUS
OrderListOpCodeProcessSelectQHotKey (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_PANEL_ITEM                      *ContentItem,
  IN BOOLEAN                                  GoNext
  )
{
  if (SetupPagePanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (SetupPagePanel->SelectedControl == NULL || SetupPagePanel->SelectedControl->Operand != EFI_IFR_ORDERED_LIST_OP) {
    return EFI_UNSUPPORTED;
  }

  if (GoNext) {
    if (SetupPagePanel->SelectedControl->Sequence + 1 < (UINT8) mDEPrivate->FBProtocol->CurrentQ->NumberOfOptions) {
      SetupPagePanel->SelectedControl->Sequence++;
      if (H2O_LTDE_FIELD_HEIGHT (&SetupPagePanel->SelectedControl->ControlField) > H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField)) {
        OrderListOpCodeUpdateCurrentPos (SetupPagePanel, ContentItem);
      }
      return DisplaySetupPagePanelContentItemControl (SetupPagePanel, ContentItem, SetupPagePanel->SelectedControl);
    }
  } else {
    if (SetupPagePanel->SelectedControl->Sequence > 0) {
      SetupPagePanel->SelectedControl->Sequence--;
      if (H2O_LTDE_FIELD_HEIGHT (&SetupPagePanel->SelectedControl->ControlField) > H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField)) {
        OrderListOpCodeUpdateCurrentPos (SetupPagePanel, ContentItem);
      }
      return DisplaySetupPagePanelContentItemControl (SetupPagePanel, ContentItem, SetupPagePanel->SelectedControl);
    }
  }

  return EFI_UNSUPPORTED;
}

EFI_STATUS
OrderListOpCodeProcessOpenDEvt (
  IN H2O_FORM_BROWSER_D                       *Dialog,
  IN H2O_LTDE_PANEL                           *QuestionPanel
  )
{
  UINT32                                      ItemCount;
  H2O_LTDE_PANEL_ITEM                         *ItemList;

  if (Dialog == NULL || QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize panel items in question panel
  //
  ItemCount = 3;
  ItemList  = AllocateZeroPool (sizeof (H2O_LTDE_PANEL_ITEM) * ItemCount);
  if (ItemList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ItemCount = 0;
  QuestionPanelInitContentItem (Dialog, &ItemList[ItemCount++]);
  QuestionPanelInitContentHotKeyItem (Dialog, LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_UP  , &ItemList[ItemCount++]);
  QuestionPanelInitContentHotKeyItem (Dialog, LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_DOWN, &ItemList[ItemCount++]);

  FreePanelItemList (QuestionPanel->ItemList, QuestionPanel->ItemCount);
  QuestionPanel->ItemCount = ItemCount;
  QuestionPanel->ItemList  = ItemList;

  //
  // Set the question panel position and update the default highlight control
  //
  QuestionPanelUpdateField (Dialog, QuestionPanel);
  QuestionPanel->SelectedControl = QuestionPanelGetDefaultHighlight (QuestionPanel);

  return DisplayQuestionPanel (QuestionPanel);
}

EFI_STATUS
OrderListOpCodeProcessMouseClick (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_PANEL_ITEM                      *SelectedPanelItem,
  IN H2O_LTDE_CONTROL                         *SelectedControl,
  IN H2O_DISPLAY_ENGINE_USER_INPUT_DATA       *UserInputData
  )
{
  RECT                                        ControlField;
  CHAR16                                      *PromptStr;
  CHAR16                                      *PromptStrPtr;
  CHAR16                                      *OptionStr;
  UINT32                                      ControlFieldWidth;
  UINT8                                       Sequence;
  INT32                                       StartY;
  INT32                                       EndY;

  if (SetupPagePanel == NULL || SelectedPanelItem == NULL || SelectedControl == NULL || UserInputData == NULL || UserInputData->IsKeyboard) {
    return EFI_INVALID_PARAMETER;
  }

  CopyRect (&ControlField, &SelectedControl->ControlField);
  OffsetRect (&ControlField, SelectedPanelItem->ItemField.left, SelectedPanelItem->ItemField.top);
  ControlField.top    -= SelectedPanelItem->CurrentPos;
  ControlField.bottom -= SelectedPanelItem->CurrentPos;

  if (!IsPointOnField (&ControlField, UserInputData->CursorX, UserInputData->CursorY)) {
    return EFI_NOT_FOUND;
  }

  PromptStr = AllocateCopyPool (StrSize (SelectedControl->Text.String), SelectedControl->Text.String);
  if (PromptStr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ControlFieldWidth = H2O_LTDE_FIELD_WIDTH (&ControlField);
  Sequence          = 0;
  StartY            = ControlField.top;
  OptionStr         = PromptStr;
  while (TRUE) {
    PromptStrPtr = StrStr (OptionStr, L"\n");
    if (PromptStrPtr != NULL) {
      *PromptStrPtr = CHAR_NULL;
    }

    EndY = StartY + GetStringHeight (OptionStr, ControlFieldWidth) - 1;
    if (IN_RANGE ((INT32) UserInputData->CursorY, StartY, EndY)) {
      SelectedControl->Sequence = Sequence;
      DisplaySetupPagePanelContentItemControl (SetupPagePanel, SelectedPanelItem, SelectedControl);
      break;
    }

    if (PromptStrPtr == NULL) {
      break;
    }

    Sequence++;
    StartY    = EndY + 1;
    OptionStr = PromptStrPtr + 1;
  }

  FreePool (PromptStr);

  return EFI_SUCCESS;
}

EFI_STATUS
OrderListOpCodeProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  )
{
  EFI_STATUS                                  Status;
  EFI_HII_VALUE                               HiiValue;
  H2O_LTDE_PANEL                              *SetupPagePanel;
  H2O_LTDE_PANEL_ITEM                         *PanelItem;
  H2O_LTDE_CONTROL                            *SelectedControl;
  H2O_LTDE_CONTROL                            *OrgdControl;
  CHAR16                                      *UpdatedString;
  BOOLEAN                                     IsShutdownDialog;

  if (Notify == NULL || QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status         = EFI_UNSUPPORTED;
  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);

  switch (Notify->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_D:
    return OrderListOpCodeProcessOpenDEvt (&((H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify)->Dialog, QuestionPanel);

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_D:
    Status = ShutdownQuestionPanel (QuestionPanel);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE:
    if (UserInputData == NULL) {
      return Status;
    }

    if (mDEPrivate->DEStatus == DISPLAY_ENGINE_STATUS_AT_MENU) {
      if (SetupPagePanel == NULL || SetupPagePanel->SelectedControl == NULL) {
        break;
      }

      if (UserInputData->KeyData.Key.UnicodeChar == CHAR_SUB ||
          UserInputData->KeyData.Key.UnicodeChar == CHAR_ADD) {
        OrderListOpCodeShiftOrder (SetupPagePanel->SelectedControl, UserInputData->KeyData.Key.UnicodeChar == CHAR_SUB);
        return EFI_SUCCESS;
      }
      break;
    }

    //
    // In dialog process
    //
    if (UserInputData->KeyData.Key.UnicodeChar == CHAR_SUB ||
        UserInputData->KeyData.Key.UnicodeChar == CHAR_ADD) {
      Status = OrderListOpCodeShiftOrderInDlg (QuestionPanel, (BOOLEAN) (UserInputData->KeyData.Key.UnicodeChar == CHAR_SUB));
      break;
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
        Status = OrderListOpCodeGetHiiValue (QuestionPanel, &HiiValue);
          if (EFI_ERROR (Status)) {
            break;
          }

        Status = SendChangeQNotify (0, 0, &HiiValue);
        if (SetupPagePanel != NULL && SetupPagePanel->SelectedControl != NULL) {
          SetupPagePanel->SelectedControl->Sequence = 0;
        }
      } else {
        OrgdControl = QuestionPanel->SelectedControl;

        if (SelectedControl->ControlId == LTDE_CONTROL_ID_DIALOG_HOT_KEY_SCROLL_UP) {
          if (UserInputData->IsKeyboard) {
            SelectedControl = NULL;
          } else {
            PanelItem       = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT);
            SelectedControl = GetPreviousSelectableControl (PanelItem, QuestionPanel->SelectedControl, FALSE);
          }
        } else if (SelectedControl->ControlId == LTDE_CONTROL_ID_DIALOG_HOT_KEY_SCROLL_DOWN) {
          if (UserInputData->IsKeyboard) {
            SelectedControl = NULL;
          } else {
            PanelItem       = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT);
            SelectedControl = GetNextSelectableControl (PanelItem, QuestionPanel->SelectedControl, FALSE);
          }
        }

        if (SelectedControl != NULL) {
          QuestionPanel->SelectedControl = SelectedControl;
          if (UpdatePanelContentItemPos (QuestionPanel)) {
            DisplayQuestionPanel (QuestionPanel);
          } else {
            DisplayQuestionPanelControl (QuestionPanel, OrgdControl, FALSE);
            DisplayQuestionPanelControl (QuestionPanel, QuestionPanel->SelectedControl, TRUE);
          }
          Status = SendChangingQNotify (&SelectedControl->HiiValue, NULL);
        }
      }
      break;
    }

    if (IsShutdownDialog) {
      Status = SendShutDNotify ();
      break;
    }
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q:
    //
    // BUGBUG: Temporarily not support ChangingQ event from other display engine
    //
    break;

  default:
    break;
  }

  return Status;
}


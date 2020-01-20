/** @file
  Date and Time questions related functions.

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

#include "LTDEQuestions.h"
#include "LTDEPanels.h"
#include "LTDEPrint.h"
#include "LTDEMisc.h"

extern EFI_SETUP_MOUSE_PROTOCOL               *mSetupMouse;

/**
 Check if query control or question is date or time opcode.

 @param[in] Control              A pointer to control
 @param[in] Question             A pointer to question

 @retval TRUE                    It is date or time opcode.
 @retval FALSE                   It is not date or time opcode.

**/
STATIC
BOOLEAN
IsDateTimeOpCode (
  IN H2O_LTDE_CONTROL                         *Control  OPTIONAL,
  IN H2O_FORM_BROWSER_Q                       *Question OPTIONAL
  )
{
  return (BOOLEAN) ((Control  != NULL && (Control->Operand  == EFI_IFR_DATE_OP || Control->Operand  == EFI_IFR_TIME_OP)) ||
                    (Question != NULL && (Question->Operand == EFI_IFR_DATE_OP || Question->Operand == EFI_IFR_TIME_OP)));
}

/**
 Check if query date or time control is being modifying.

 @param[in] Control              A pointer to control

 @retval TRUE                    Control is being modifying.
 @retval FALSE                   Control is not being modifying.

**/
BOOLEAN
IsDateTimeOpCodeBeingModified (
  IN H2O_LTDE_CONTROL                         *Control
  )
{
  return (BOOLEAN) (Control != NULL &&
                    IsDateTimeOpCode (Control, NULL) &&
                    Control->ValueStrInfo.String != NULL &&
                    StrStr (Control->ValueStrInfo.String, L" ") != NULL);
}

STATIC
EFI_STATUS
DateTimeOpCodeRestoreValueStr (
  IN H2O_LTDE_CONTROL                         *Control
  )
{
  H2O_LTDE_PANEL                              *SetupPagePanel;
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_S                          *Statement;

  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel == NULL) {
    return EFI_UNSUPPORTED;
  }

  Status = mDEPrivate->FBProtocol->GetSInfo (mDEPrivate->FBProtocol, Control->PageId, Control->StatementId, &Statement);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SafeFreePool ((VOID **) &Control->ValueStrInfo.String);
  Control->ValueStrInfo.String = DateTimeOpCodeCreateValueStr (Statement);
  SafeFreePool ((VOID **) &Statement);

  DateTimeOpCodeDisplayControl (SetupPagePanel, Control);

  return EFI_SUCCESS;
}

H2O_DATE_TIME_ITEM
DateTimeOpCodeGetItemValue (
  IN H2O_LTDE_CONTROL                         *Control
  )
{
  if (Control == NULL || !IsDateTimeOpCode (Control, NULL)) {
    return UnknownItem;
  }

  //
  // Date display format: L"[12/31/2014]"
  // Time display format: L"[23:59:59]"
  //
  switch (Control->Sequence) {

  case 0:
    return (Control->Operand  == EFI_IFR_DATE_OP) ? MonthItem : HourItem;

  case 1:
    return (Control->Operand  == EFI_IFR_DATE_OP) ? DayItem   : MinuteItem;

  case 2:
    return (Control->Operand  == EFI_IFR_DATE_OP) ? YearItem  : SecondItem;

  default:
    break;
  }

  return UnknownItem;
}

CHAR16 *
DateTimeOpCodeCreateValueStr (
  IN H2O_FORM_BROWSER_Q                       *Question
  )
{
  if (Question == NULL || !IsDateTimeOpCode (NULL, Question)) {
    return NULL;
  }

  if (Question->Operand == EFI_IFR_DATE_OP) {
    return CatSPrint (
             NULL,
             L"%c%02d%c%02d%c%04d%c",
             LEFT_NUMERIC_DELIMITER,
             Question->HiiValue.Value.date.Month,
             DATE_SEPARATOR,
             Question->HiiValue.Value.date.Day,
             DATE_SEPARATOR,
             Question->HiiValue.Value.date.Year,
             RIGHT_NUMERIC_DELIMITER
             );
  } else {
    return CatSPrint (
             NULL,
             L"%c%02d%c%02d%c%02d%c",
             LEFT_NUMERIC_DELIMITER,
             Question->HiiValue.Value.time.Hour,
             TIME_SEPARATOR,
             Question->HiiValue.Value.time.Minute,
             TIME_SEPARATOR,
             Question->HiiValue.Value.time.Second,
             RIGHT_NUMERIC_DELIMITER
             );
  }
}

EFI_STATUS
DateTimeOpCodeDisplayControl (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_CONTROL                         *Control
  )
{
  H2O_LTDE_PANEL_ITEM                         *PanelItem;
  INT32                                       StartY;
  INT32                                       EndY;
  INT32                                       X;
  INT32                                       Y;
  UINT32                                      NormalAttribute;
  UINT32                                      HighlightAttribute;
  BOOLEAN                                     IsHighlight;
  UINTN                                       ValueStrSize;
  CHAR16                                      *StrPtr;
  CHAR16                                      CharStr[2] = {0};
  BOOLEAN                                     IsDate;

  if (Control == NULL || !IsDateTimeOpCode (Control, NULL) || Control->ValueStrInfo.String == NULL || (H2O_LTDE_FIELD_HEIGHT (&Control->ControlField) != 1)) {
    return EFI_INVALID_PARAMETER;
  }

  PanelItem = GetPanelItemByControl (SetupPagePanel, Control);
  if (PanelItem == NULL) {
    return EFI_NOT_FOUND;
  }

  StartY = PanelItem->CurrentPos;
  EndY   = StartY + H2O_LTDE_FIELD_HEIGHT (&PanelItem->ItemField) - 1;
  if (!IS_OVERLAP (Control->ControlField.top, Control->ControlField.bottom, StartY, EndY)) {
    return EFI_SUCCESS;
  }

  IsDate       = (Control->Operand == EFI_IFR_DATE_OP) ? TRUE : FALSE;
  ValueStrSize = StrSize (Control->ValueStrInfo.String);
  if ((IsDate  && ValueStrSize != sizeof (L"[12/31/2014]")) ||
      (!IsDate && ValueStrSize != sizeof (L"[23:59:59]"))) {
    return EFI_INVALID_PARAMETER;
  }

  IsHighlight = IsCurrentHighlight (Control->PageId, Control->QuestionId, Control->IfrOpCode);
  GetPanelColorAttribute (SetupPagePanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, Control->ControlStyle.PseudoClass, &NormalAttribute);
  GetPanelColorAttribute (SetupPagePanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_HIGHLIGHT , &HighlightAttribute);

  //
  // Display prompt string
  //
  DEConOutSetAttribute (IsHighlight ? HighlightAttribute : NormalAttribute);
  StrPtr = GetAlignmentString (
             Control->Text.String,
             H2O_LTDE_FIELD_WIDTH (&Control->Text.StringField) + PROMPT_VALUE_SEPARATOR_WIDTH,
             LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT
             );
  DisplayString (
    PanelItem->ItemField.left,
    PanelItem->ItemField.top + (Control->ControlField.top - PanelItem->CurrentPos),
    StrPtr
    );
  FreePool (StrPtr);

  //
  // Display value string
  //
  StrPtr = GetAlignmentString (
               Control->ValueStrInfo.String,
               H2O_LTDE_FIELD_WIDTH (&Control->ValueStrInfo.StringField),
               LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT
               );
  if (StrPtr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  X = PanelItem->ItemField.left + Control->ValueStrInfo.StringField.left;
  Y = PanelItem->ItemField.top + (Control->ControlField.top - PanelItem->CurrentPos);
  DEConOutSetAttribute (NormalAttribute);
  if (IsHighlight) {
    CharStr[0]  = LEFT_NUMERIC_DELIMITER;
    DisplayString (X, Y, CharStr);
    StrPtr[0] = CHAR_NULL;

    CharStr[0]  = IsDate ? DATE_SEPARATOR : TIME_SEPARATOR;
    DisplayString (X + 3, Y, CharStr);
    DisplayString (X + 6, Y, CharStr);
    StrPtr[3] = CHAR_NULL;
    StrPtr[6] = CHAR_NULL;

    CharStr[0]   = RIGHT_NUMERIC_DELIMITER;
    if (IsDate) {
      DisplayString (X + 11, Y, &StrPtr[11]);
      StrPtr[11] = CHAR_NULL;
    } else {
      CharStr[0]   = RIGHT_NUMERIC_DELIMITER;
      DisplayString (X + 9, Y, &StrPtr[9]);
      StrPtr[9] = CHAR_NULL;
    }

    DEConOutSetAttribute ((Control->Sequence == 0) ? HighlightAttribute : NormalAttribute);
    DisplayString (X + 1, Y, &StrPtr[1]);

    DEConOutSetAttribute ((Control->Sequence == 1) ? HighlightAttribute : NormalAttribute);
    DisplayString (X + 4, Y, &StrPtr[4]);

    DEConOutSetAttribute ((Control->Sequence == 2) ? HighlightAttribute : NormalAttribute);
    DisplayString (X + 7, Y, &StrPtr[7]);
  } else {
    DisplayString (X, Y, StrPtr);
  }

  FreePool (StrPtr);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DateTimeOpCodeGetSequenceValue (
  IN  BOOLEAN                                 IsDate,
  IN  RECT                                    *ValueStrAbsField,
  IN  UINT32                                  StartX,
  IN  UINT32                                  StartY,
  OUT UINT8                                   *Sequence
  )
{
  INT32                                       Offset;

  if (ValueStrAbsField == NULL || Sequence == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsPointOnField (ValueStrAbsField, (INT32) StartX, (INT32) StartY)) {
    return EFI_NOT_FOUND;
  }

  Offset = (INT32) StartX - ValueStrAbsField->left;

  if (Offset >= 1 && Offset <= 2) {
    *Sequence = 0;
  } else if (Offset >= 4 && Offset <= 5) {
    *Sequence = 1;
  } else if ((IsDate  && Offset >= 7 && Offset <= 10) ||
             (!IsDate && Offset >= 7 && Offset <= 8)) {
    *Sequence = 2;
  } else {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DateTimeOpCodeGetHiiValueFromValueStr (
  IN  H2O_LTDE_CONTROL                        *Control,
  OUT EFI_HII_VALUE                           *HiiValue
  )
{
  CHAR16                                      *ValueStr;
  EFI_HII_VALUE                               StrHiiValue;
  BOOLEAN                                     IsDate;
  EFI_TIME                                    StrEfiTime;

  if (Control == NULL || !IsDateTimeOpCode (Control, NULL) || Control->ValueStrInfo.String == NULL) {
    return EFI_UNSUPPORTED;
  }

  CopyMem (&StrHiiValue, &Control->HiiValue, sizeof (EFI_HII_VALUE));
  ValueStr = AllocateCopyPool (StrSize (Control->ValueStrInfo.String), Control->ValueStrInfo.String);
  if (ValueStr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Date display format: L"[12/31/2014]"
  // Time display format: L"[23:59:59]"
  //
  IsDate = (Control->Operand == EFI_IFR_DATE_OP) ? TRUE : FALSE;
  if (IsDate) {
    ValueStr[3]  = CHAR_NULL;
    ValueStr[6]  = CHAR_NULL;
    ValueStr[11] = CHAR_NULL;
    StrHiiValue.Value.date.Month = (UINT8)  StrDecimalToUint64 (&ValueStr[1]);
    StrHiiValue.Value.date.Day   = (UINT8)  StrDecimalToUint64 (&ValueStr[4]);
    StrHiiValue.Value.date.Year  = (UINT16) StrDecimalToUint64 (&ValueStr[7]);
  } else {
    ValueStr[3] = CHAR_NULL;
    ValueStr[6] = CHAR_NULL;
    ValueStr[9] = CHAR_NULL;
    StrHiiValue.Value.time.Hour   = (UINT8) StrDecimalToUint64 (&ValueStr[1]);
    StrHiiValue.Value.time.Minute = (UINT8) StrDecimalToUint64 (&ValueStr[4]);
    StrHiiValue.Value.time.Second = (UINT8) StrDecimalToUint64 (&ValueStr[7]);
  }

  FreePool ((VOID *) ValueStr);

  TransferHiiValueToEfiTime (&StrHiiValue, &StrEfiTime);
  if ((IsDate  && !IsDayValid (&StrEfiTime)) ||
      (!IsDate && !IsTimeValid (&StrEfiTime))) {
    return EFI_UNSUPPORTED;
  }

  CopyMem (HiiValue, &StrHiiValue, sizeof (StrHiiValue));

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DateTimeOpCodeInputOnTheFly (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_CONTROL                         *Control,
  IN EFI_INPUT_KEY                            *Key
  )
{
  CHAR16                                      *StrPtr;
  EFI_STATUS                                  Status;
  EFI_HII_VALUE                               HiiValue;

  if (SetupPagePanel == NULL || Control == NULL || Key == NULL || !IsDecChar (Key->UnicodeChar)) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsDateTimeOpCodeBeingModified (Control)) {
    StrPtr = StrStr (Control->ValueStrInfo.String, L" ");
    if (StrPtr == NULL) {
      return EFI_ABORTED;
    }
    *StrPtr = Key->UnicodeChar;

    if (IsDateTimeOpCodeBeingModified (Control)) {
      DateTimeOpCodeDisplayControl (SetupPagePanel, Control);
    } else {
      Status = DateTimeOpCodeGetHiiValueFromValueStr (Control, &HiiValue);
      if (!EFI_ERROR (Status)) {
        SendChangeQNotify (Control->PageId, Control->QuestionId, &HiiValue);
      } else {
        DateTimeOpCodeRestoreValueStr (Control);
      }
    }
  } else {
    //
    // Date display format: L"[12/31/2014]"
    //
    switch (Control->Sequence) {

    case 0:
      CopyMem (&Control->ValueStrInfo.String[1], L"  ", sizeof (L"  ") - sizeof (CHAR16));
      break;
    case 1:
      CopyMem (&Control->ValueStrInfo.String[4], L"  ", sizeof (L"  ") - sizeof (CHAR16));
      break;
    case 2:
      if (Control->Operand == EFI_IFR_DATE_OP) {
        CopyMem (&Control->ValueStrInfo.String[7], L"    ", sizeof (L"    ") - sizeof (CHAR16));
      } else {
        CopyMem (&Control->ValueStrInfo.String[7], L"  "  , sizeof (L"  ") - sizeof (CHAR16));
      }
      break;
    default:
      ASSERT(FALSE);
      return EFI_ABORTED;
    }

    StrPtr = StrStr (Control->ValueStrInfo.String, L" ");
    if (StrPtr == NULL) {
      return EFI_ABORTED;
    }
    *StrPtr = Key->UnicodeChar;
    DateTimeOpCodeDisplayControl (SetupPagePanel, Control);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DateTimeOpCodeProcessInputEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData
  )
{
  EFI_STATUS                                  Status;
  EFI_INPUT_KEY                               *Key;
  EFI_HII_VALUE                               HiiValue;
  EFI_TIME                                    EfiTime;
  RECT                                        ControlAbsField;
  UINT8                                       Sequence;
  BOOLEAN                                     IsInputingOnTheFly;
  H2O_LTDE_PANEL                              *SetupPagePanel;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_PANEL_ITEM                         *SelectedPanelItem;
  H2O_LTDE_CONTROL                            *SelectedControl;
  KEYBOARD_ATTRIBUTES                         KeyboardAttributes;

  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel == NULL || SetupPagePanel->SelectedControl == NULL) {
    return EFI_UNSUPPORTED;
  }


  Status             = EFI_UNSUPPORTED;
  SelectedControl    = SetupPagePanel->SelectedControl;
  IsInputingOnTheFly = IsDateTimeOpCodeBeingModified (SelectedControl);

  if (!UserInputData->IsKeyboard) {

    ContentItem = GetPanelItem (SetupPagePanel, LTDE_PANEL_ITEM_ID_CONTENT);
    if (ContentItem == NULL) {
      return EFI_UNSUPPORTED;
    }

    CopyRect (&ControlAbsField, &SelectedControl->ControlField);
    OffsetRect (&ControlAbsField, ContentItem->ItemField.left, ContentItem->ItemField.top - ContentItem->CurrentPos);

    if (IsInputingOnTheFly) {
      if (IsVirtualKbVisibled() && mSetupMouse != NULL && UserInputData->KeyData.Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
        Status = mSetupMouse->GetKeyboardAttributes (mSetupMouse, &KeyboardAttributes);
        if (!EFI_ERROR (Status) && !KeyboardAttributes.IsStart) {
          InitializeVirtualKb (&ControlAbsField);
        }
      }
      return EFI_SUCCESS;
    }

    if (UserInputData->KeyData.Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
      return Status;
    }


    if (!IsPointOnField (&ControlAbsField, (INT32) UserInputData->CursorX, (INT32) UserInputData->CursorY)) {

      if (!IsPointOnField (&SetupPagePanel->PanelField, UserInputData->CursorX, UserInputData->CursorY)) {
        ShutdownVirtualKb ();
      }

      SelectedPanelItem = NULL;
      Status = GetControlByMouse (SetupPagePanel, UserInputData->CursorX, UserInputData->CursorY, &SelectedPanelItem, &SelectedControl);
      if (EFI_ERROR (Status)) {
        ShutdownVirtualKb ();
      }

      if (SelectedControl == NULL || SelectedControl != SetupPagePanel->SelectedControl) {
        ShutdownVirtualKb ();
      }

      return EFI_UNSUPPORTED;
    }

    CopyRect (&ControlAbsField, &SelectedControl->ValueStrInfo.StringField);
    OffsetRect (&ControlAbsField, ContentItem->ItemField.left, ContentItem->ItemField.top - ContentItem->CurrentPos);

    Status = DateTimeOpCodeGetSequenceValue ((SelectedControl->Operand == EFI_IFR_DATE_OP), &ControlAbsField, (INT32) UserInputData->CursorX, (INT32) UserInputData->CursorY, &Sequence);
    if (!EFI_ERROR (Status)) {
      if (Sequence != SelectedControl->Sequence) {
        SelectedControl->Sequence = Sequence;
        DateTimeOpCodeDisplayControl (SetupPagePanel, SelectedControl);
      } else {
        InitializeVirtualKb (&ControlAbsField);
      }
    }


    return EFI_SUCCESS;
  }

  Key = &UserInputData->KeyData.Key;

  switch (Key->ScanCode) {

  case SCAN_ESC:
    if (IsInputingOnTheFly) {
      DateTimeOpCodeRestoreValueStr (SelectedControl);
      Status = EFI_SUCCESS;
    }
    return Status;

  default:
    break;
  }

  switch (Key->UnicodeChar) {

  case CHAR_CARRIAGE_RETURN:
  case CHAR_TAB:
    if (IsInputingOnTheFly) {
      Status = DateTimeOpCodeGetHiiValueFromValueStr (SelectedControl, &HiiValue);
      if (!EFI_ERROR (Status)) {
        SendChangeQNotify (SelectedControl->PageId, SelectedControl->QuestionId, &HiiValue);
      } else {
        DateTimeOpCodeRestoreValueStr (SelectedControl);
      }
    } else {
      SelectedControl->Sequence = (SelectedControl->Sequence == 2) ? 0 : SelectedControl->Sequence + 1;
      DateTimeOpCodeDisplayControl (SetupPagePanel, SelectedControl);
    }
    return EFI_SUCCESS;

  case CHAR_ADD:
  case CHAR_SUB:
    TransferHiiValueToEfiTime (&SelectedControl->HiiValue, &EfiTime);
    Status = GetNextDateTimeValue (
               DateTimeOpCodeGetItemValue (SelectedControl),
               (Key->UnicodeChar == CHAR_ADD),
               &EfiTime
               );
    if (!EFI_ERROR (Status)) {
      TransferEfiTimeToHiiValue (SelectedControl->Operand == EFI_IFR_DATE_OP, &EfiTime, &SelectedControl->HiiValue);
      SendChangeQNotify (
        SelectedControl->PageId,
        SelectedControl->QuestionId,
        &SelectedControl->HiiValue
        );
    }
    return EFI_SUCCESS;

  case CHAR_BACKSPACE:
    if (IsInputingOnTheFly) {
      DateTimeOpCodeRestoreValueStr (SelectedControl);
      Status = EFI_SUCCESS;
    }
    return Status;

  default:
    if (IsInputingOnTheFly) {
      if (!IsDecChar (Key->UnicodeChar)) {
        DateTimeOpCodeRestoreValueStr (SelectedControl);
        return EFI_SUCCESS;
      }

      DateTimeOpCodeInputOnTheFly (SetupPagePanel, SelectedControl, Key);
    } else {
      if (!IsVisibleChar (Key->UnicodeChar)) {
        ShutdownVirtualKb ();
      }
      if (!IsDecChar (Key->UnicodeChar)) {
        return EFI_UNSUPPORTED;
      }
      DateTimeOpCodeInputOnTheFly (SetupPagePanel, SelectedControl, Key);
    }
    break;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DateTimeOpCodeProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  )
{
  EFI_STATUS                                  Status;

  if (Notify == NULL || QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;

  switch (Notify->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE:
    if (UserInputData == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    if (mDEPrivate->DEStatus == DISPLAY_ENGINE_STATUS_AT_MENU) {
      Status = DateTimeOpCodeProcessInputEvt (Notify, UserInputData);
    }
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_D:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_D:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q:
  default:
    break;
  }

  return Status;
}


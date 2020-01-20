/** @file
  Numeric question related functions.

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

/**
 Process open dialog event in numeric opcode

 @param[in] Dialog               A pointer to dialog data
 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Process event successful.
 @retval EFI_INVALID_PARAMETER   Dialog or QuestionPanel is NULL.
 @retval EFI_OUT_OF_RESOURCES    Allocate pool fail

**/
EFI_STATUS
NumericQuestionProcessOpenDEvt (
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
  QuestionPanelInitTitleItem (Dialog, &ItemList[ItemCount++]);
  QuestionPanelInitContentItem (Dialog, &ItemList[ItemCount++]);
  QuestionPanelInitButtonItem (Dialog, &ItemList[ItemCount++]);

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

/**
 Process event in numeric opcode

 @param[in] Notify               A pointer to notify event
 @param[in] UserInputData        A pointer to user input data
 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Process event successful.
 @retval EFI_INVALID_PARAMETER   Notify or QuestionPanel is NULL.

**/
EFI_STATUS
NumericOpCodeProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  )
{
  EFI_STATUS                                  Status;
  EFI_HII_VALUE                               HiiValue;
  H2O_LTDE_CONTROL                            *Control;
  H2O_LTDE_CONTROL                            *SelectedControl;
  CHAR16                                      *UpdatedString;
  CHAR16                                      *ValueString;
  CHAR16                                      *EvtValueStr;
  UINT64                                      Value;
  BOOLEAN                                     IsShutdownDialog;
  BOOLEAN                                     IsHex;
  H2O_DISPLAY_ENGINE_EVT_CHANGING_Q           *ChangingQ;

  if (Notify == NULL || QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;

  switch (Notify->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_D:
    return NumericQuestionProcessOpenDEvt (&((H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify)->Dialog, QuestionPanel);

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_D:
    ShutdownVirtualKb ();
    Status = ShutdownQuestionPanel (QuestionPanel);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE:
    if (UserInputData == NULL) {
      return Status;
    }

    if (mDEPrivate->DEStatus == DISPLAY_ENGINE_STATUS_AT_MENU) {
      if (UserInputData->KeyData.Key.UnicodeChar == CHAR_SUB ||
          UserInputData->KeyData.Key.UnicodeChar == CHAR_ADD) {
        Status = GetNextQuestionValue (mDEPrivate->FBProtocol->CurrentQ, (UserInputData->KeyData.Key.UnicodeChar == CHAR_ADD), &HiiValue);
        if (EFI_ERROR (Status)) {
          return Status;
        }

        Status = SendChangeQNotify (0, 0, &HiiValue);
      }
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
        if (SelectedControl->ControlId == LTDE_CONTROL_ID_DIALOG_BUTTON) {
          if (SelectedControl->HiiValue.Value.b == BUTTON_VALUE_NO) {
            Status = SendShutDNotify ();
            break;
          } else if (SelectedControl->HiiValue.Value.b == BUTTON_VALUE_CANCEL) {
            break;
          }
        }

        Control = GetControlById (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT, LTDE_CONTROL_ID_DIALOG_BODY_INPUT);
        if (Control == NULL || !IN_RANGE (Control->HiiValue.Value.u64, Control->Minimum, Control->Maximum)) {
          Status = SendShutDNotify ();
          break;
        }

        IsHex = IS_DISPLAYED_IN_HEX (Control);
        if (*Control->Text.String == CHAR_NULL ||
            IsHex && StrSize (Control->Text.String) <= sizeof (HEX_NUMBER_PREFIX_STRING)) {
          Status = SendShutDNotify ();
          break;
        }

        Status = SendChangeQNotify (0, 0, &Control->HiiValue);
      } else {
        DisplayQuestionPanelControl (QuestionPanel, QuestionPanel->SelectedControl, FALSE);
        QuestionPanel->SelectedControl = SelectedControl;
        DisplayQuestionPanelControl (QuestionPanel, QuestionPanel->SelectedControl, TRUE);
      }
    } else if (UpdatedString != NULL) {
      Control = QuestionPanel->SelectedControl;

      if (Control == NULL || Control->ControlId != LTDE_CONTROL_ID_DIALOG_BODY_INPUT) {
        FreePool (UpdatedString);
        break;
      }

      IsHex       = IS_DISPLAYED_IN_HEX (Control);
      ValueString = IsHex ? &UpdatedString[sizeof (HEX_NUMBER_PREFIX_STRING) / sizeof (CHAR16) - 1] : UpdatedString;
      if ((!IsValidValueStr (ValueString, IsHex)) ||
          (StrSize (Control->Text.String) != (IsHex ? sizeof (HEX_NUMBER_PREFIX_STRING) : sizeof (CHAR16)) && IsHexChar (UserInputData->KeyData.Key.UnicodeChar) && (Control->HiiValue.Value.u64 > (IsHex ? (((UINT64) -1) >> 4) : (((UINT64) -1) / 10)))) ||
          IsHex && StrSize (UpdatedString) < sizeof (HEX_NUMBER_PREFIX_STRING)) {
        FreePool (UpdatedString);
        break;
      }

      //
      // UINT64 maximum value is 18446744073709551615, to avoid overlap
      //
      if (!IsHex && (Control->HiiValue.Value.u64 == 1844674407370955161ull) &&
          UserInputData->KeyData.Key.UnicodeChar > '5') {
        FreePool (UpdatedString);
        break;
      }

      Value = IsHex ? StrHexToUint64 (ValueString) : StrDecimalToUint64 (UpdatedString);
      if (!IsEditValueValid (Value, Control->Minimum, Control->Maximum, IsHex)) {
        FreePool (UpdatedString);
        break;
      }

      EvtValueStr = CatSPrint (NULL, L"%s%s", IsHex ? L"0x" : L"", ValueString);
      FreePool (UpdatedString);
      if (EvtValueStr == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      ZeroMem (&HiiValue, sizeof (EFI_HII_VALUE));
      CreateValueAsString (&HiiValue, (UINT16) StrSize (EvtValueStr), (UINT8 *) EvtValueStr);
      Status = SendChangingQNotify (&HiiValue, NULL);
      break;
    } else if (IsShutdownDialog) {
      Status = SendShutDNotify ();
    }
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q:
    Control = QuestionPanel->SelectedControl;
    if (Control == NULL || Control->ControlId != LTDE_CONTROL_ID_DIALOG_BODY_INPUT) {
      break;
    }

    ChangingQ = (H2O_DISPLAY_ENGINE_EVT_CHANGING_Q *) Notify;
    if (ChangingQ->BodyHiiValue.Type != EFI_IFR_TYPE_STRING ||
        ChangingQ->BodyHiiValue.Buffer == NULL) {
      break;
    }

    EvtValueStr = (CHAR16 *) ChangingQ->BodyHiiValue.Buffer;
    IsHex       = IsHexString (EvtValueStr);
    Value       = IsHex ? StrHexToUint64 (EvtValueStr) : StrDecimalToUint64 (EvtValueStr);
    if (!IsEditValueValid (Value, Control->Minimum, Control->Maximum, IsHex)) {
      break;
    }

    if (IS_DISPLAYED_IN_HEX (Control)) {
      UpdatedString = CatSPrint (NULL, L"%s%s", HEX_NUMBER_PREFIX_STRING, &EvtValueStr[sizeof (L"0x") / sizeof (CHAR16) - 1]);
    } else {
      UpdatedString = AllocateCopyPool (StrSize (EvtValueStr), EvtValueStr);
    }
    if (UpdatedString == NULL) {
      break;
    }

    SafeFreePool ((VOID **) &Control->Text.String);
    Control->Text.String        = UpdatedString;
    Control->HiiValue.Value.u64 = Value;
    Status = DisplayQuestionPanelControl (QuestionPanel, Control, (BOOLEAN) (QuestionPanel->SelectedControl == Control));
    break;

  default:
    break;
  }

  return Status;
}


/** @file
  Password question related functions.

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

/**
 Get password HII value from content item.

 @param[in]  ContentItem         A pointer to content item
 @param[out] HiiValue            A pointer to HII value

 @retval EFI_SUCCESS             Get password HII value successful
 @retval EFI_INVALID_PARAMETER   Input parameter is NULL
 @retval EFI_NOT_FOUND           Password value is not found
 @retval EFI_OUT_OF_RESOURCES    Allocate pool fail

**/
STATIC
EFI_STATUS
GetPasswordHiiValue (
  IN  H2O_LTDE_PANEL_ITEM                     *ContentItem,
  OUT EFI_HII_VALUE                           *HiiValue
  )
{
  UINT32                                      Index;
  UINTN                                       TotalStrSize;
  UINTN                                       StringSize;
  UINTN                                       PasswordCount;
  UINT8                                       *StringBuffer;
  H2O_LTDE_CONTROL                            *Control;

  if (ContentItem == NULL || HiiValue == NULL || ContentItem->ControlList == NULL || ContentItem->ControlCount == 0) {
    return EFI_INVALID_PARAMETER;
  }

  TotalStrSize  = 0;
  PasswordCount = 0;

  for (Index = 0; Index < ContentItem->ControlCount; Index++) {
    Control = FindControlByControlId (ContentItem->ControlList, ContentItem->ControlCount, LTDE_CONTROL_ID_DIALOG_BODY_INPUT, Index);
    if (Control == NULL) {
      continue;
    }

    TotalStrSize += StrSize (Control->Text.String);
    PasswordCount++;
  }

  if (PasswordCount == 0 || TotalStrSize == 0) {
    return EFI_NOT_FOUND;
  }

  StringBuffer = AllocateZeroPool (TotalStrSize);
  if (StringBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  HiiValue->Type      = EFI_IFR_TYPE_STRING;
  HiiValue->BufferLen = (UINT16) TotalStrSize;
  HiiValue->Buffer    = StringBuffer;

  for (Index = 0; Index < ContentItem->ControlCount; Index++) {
    Control = FindControlByControlId (ContentItem->ControlList, ContentItem->ControlCount, LTDE_CONTROL_ID_DIALOG_BODY_INPUT, Index);
    if (Control == NULL) {
      continue;
    }

    StringSize = StrSize (Control->Text.String);
    CopyMem (StringBuffer, Control->HiiValue.Buffer, StringSize);
    StringBuffer += StringSize;
  }

  return EFI_SUCCESS;
}

/**
 Process open dialog event in password opcode

 @param[in] Dialog               A pointer to dialog data
 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Process event successful.
 @retval EFI_INVALID_PARAMETER   Dialog or QuestionPanel is NULL.
 @retval EFI_OUT_OF_RESOURCES    Allocate pool fail

**/
EFI_STATUS
PasswordQuestionProcessOpenDEvt (
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
  ItemCount = 2;
  ItemList  = AllocateZeroPool (sizeof (H2O_LTDE_PANEL_ITEM) * ItemCount);
  if (ItemList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ItemCount = 0;
  QuestionPanelInitTitleItem (Dialog, &ItemList[ItemCount++]);
  QuestionPanelInitContentItem (Dialog, &ItemList[ItemCount++]);

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
 Process event in password opcode

 @param[in] Notify               A pointer to notify event
 @param[in] UserInputData        A pointer to user input data
 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Process event successful.
 @retval EFI_INVALID_PARAMETER   Notify or QuestionPanel is NULL.

**/
EFI_STATUS
PasswordOpCodeProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  )
{
  EFI_STATUS                                  Status;
  EFI_HII_VALUE                               HiiValue;
  H2O_LTDE_CONTROL                            *SelectedControl;
  CHAR16                                      *UpdatedString;
  BOOLEAN                                     IsShutdownDialog;
  H2O_LTDE_CONTROL                            *Control;
  UINTN                                       StringSize;
  H2O_DISPLAY_ENGINE_EVT_CHANGING_Q           *ChangingQ;

  if (Notify == NULL || QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_NOT_FOUND;

  switch (Notify->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_D:
    return PasswordQuestionProcessOpenDEvt (&((H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify)->Dialog, QuestionPanel);

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_D:
    QuestionPanel = GetPanel (H2O_PANEL_TYPE_QUESTION);
    if (QuestionPanel == NULL) {
      break;
    }
    ShutdownVirtualKb ();
    Status = ShutdownQuestionPanel (QuestionPanel);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE:
    if (UserInputData == NULL) {
      return EFI_INVALID_PARAMETER;
    }
    if (mDEPrivate->DEStatus != DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG) {
      break;
    }
    if (mDEPrivate->FBProtocol->CurrentP == NULL &&
        UserInputData->IsKeyboard &&
        UserInputData->KeyData.Key.ScanCode == SCAN_ESC) {
      return EFI_SUCCESS;
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
        if (Control == NULL) {
          Status = SendShutDNotify ();
          break;
        }

        Status = GetPasswordHiiValue (GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT), &HiiValue);
        if (EFI_ERROR(Status)) {
          return Status;
        }

        Status = SendChangeQNotify (0, 0, &HiiValue);
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

      StringSize = StrSize (UpdatedString);
      if (StringSize > Control->HiiValue.BufferLen) {
        FreePool ((VOID *) UpdatedString);
        break;
      }

      if (StringSize > StrSize ((CHAR16 *) Control->HiiValue.Buffer)) {
        CopyMem (UpdatedString, Control->HiiValue.Buffer, StrLen ((CHAR16 *) Control->HiiValue.Buffer) * sizeof (CHAR16));
      } else {
        CopyMem (UpdatedString, Control->HiiValue.Buffer, StringSize - sizeof (CHAR16));
      }

      HiiValue.Type      = EFI_IFR_TYPE_STRING;
      HiiValue.BufferLen = (UINT16) StringSize;
      HiiValue.Buffer    = (UINT8 *) UpdatedString;

      Status = SendChangingQNotify (&HiiValue, NULL);
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

    UpdatedString = (CHAR16 *) ChangingQ->BodyHiiValue.Buffer;
    StringSize    = StrSize (UpdatedString);
    if (StringSize > Control->HiiValue.BufferLen) {
      FreePool ((VOID *) UpdatedString);
      break;
    }

    SafeFreePool ((VOID **) &Control->Text.String);
    Control->Text.String = CreateString ((UINT32) (StringSize / 2 - 1), '*');
    CopyMem (Control->HiiValue.Buffer, UpdatedString, StringSize);
    Status = DisplayQuestionPanelControl (QuestionPanel, Control, TRUE);
    break;

  default:
    break;
  }

  return Status;
}


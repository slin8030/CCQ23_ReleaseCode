/** @file
  OneOf question related functions.

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

/**
 Get control by HII value from question panel.

 @param[in] QuestionPanel        A pointer to question panel
 @param[in] HiiValue             A pointer to HII value

 @return The poiter of control or NULL if it is not found or input parameter is NULL.

**/
STATIC
H2O_LTDE_CONTROL *
GetControlByHiiValue (
  IN H2O_LTDE_PANEL                           *QuestionPanel,
  IN EFI_HII_VALUE                            *HiiValue
  )
{
  H2O_LTDE_PANEL_ITEM                         *ContentPanelItem;
  UINT32                                      Index;
  INTN                                        Result;
  EFI_STATUS                                  Status;

  if (QuestionPanel == NULL || HiiValue == NULL) {
    return NULL;
  }

  ContentPanelItem = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentPanelItem == NULL || ContentPanelItem->ControlList == NULL) {
    return NULL;
  }

  for (Index = 0; Index < ContentPanelItem->ControlCount; Index++) {
    Status = CompareHiiValue (&ContentPanelItem->ControlList[Index].HiiValue, HiiValue, &Result);
    if (!EFI_ERROR (Status) && Result == 0) {
      return &ContentPanelItem->ControlList[Index];
    }
  }

  return NULL;
}

/**
 Process open dialog event in one of opcode

 @param[in] Dialog               A pointer to dialog data
 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Process event successful.
 @retval EFI_INVALID_PARAMETER   Dialog or QuestionPanel is NULL.
 @retval EFI_OUT_OF_RESOURCES    Allocate pool fail

**/
EFI_STATUS
OneOfQuestionProcessOpenDEvt (
  IN H2O_FORM_BROWSER_D                       *Dialog,
  IN H2O_LTDE_PANEL                           *QuestionPanel
  )
{
  UINT32                                      ItemCount;
  H2O_LTDE_PANEL_ITEM                         *ItemList;

  if (Dialog == NULL || QuestionPanel == NULL || Dialog->BodyStringCount == 0 || Dialog->BodyHiiValueArray == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize panel items in question panel
  //
  ItemCount = 4;
  ItemList  = AllocateZeroPool (sizeof (H2O_LTDE_PANEL_ITEM) * ItemCount);
  if (ItemList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ItemCount = 0;
  QuestionPanelInitTitleItem (Dialog, &ItemList[ItemCount++]);
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

  QuestionPanel->SelectedControl = GetControlByHiiValue (QuestionPanel, &Dialog->ConfirmHiiValue);
  if (QuestionPanel->SelectedControl == NULL) {
    QuestionPanel->SelectedControl = QuestionPanelGetDefaultHighlight (QuestionPanel);
  }
  UpdatePanelContentItemPos (QuestionPanel);

  return DisplayQuestionPanel (QuestionPanel);
}

/**
 Process event in one of opcode

 @param[in] Notify               A pointer to notify event
 @param[in] UserInputData        A pointer to user input data
 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Process event successful.
 @retval EFI_INVALID_PARAMETER   Notify or QuestionPanel is NULL.

**/
EFI_STATUS
OneOfQuestionProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  )
{
  EFI_STATUS                                  Status;
  H2O_LTDE_CONTROL                            *SelectedControl;
  CHAR16                                      *UpdatedString;
  BOOLEAN                                     IsShutdownDialog;
  H2O_LTDE_CONTROL                            *BackupSelectedControl;
  H2O_LTDE_PANEL_ITEM                         *PanelItem;
  H2O_LTDE_CONTROL                            *ExpectedSelectedControl;

  if (Notify == NULL || QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;

  switch (Notify->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_D:
    return OneOfQuestionProcessOpenDEvt (&((H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify)->Dialog, QuestionPanel);

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_D:
    Status = ShutdownQuestionPanel (QuestionPanel);
    break;

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
        Status = SendChangeQNotify (0, 0, &SelectedControl->HiiValue);
      } else {
        if (SelectedControl->ControlId == LTDE_CONTROL_ID_DIALOG_HOT_KEY_SCROLL_UP) {
          if (!UserInputData->IsKeyboard) {
            PanelItem       = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT);
            SelectedControl = GetPreviousSelectableControl (PanelItem, QuestionPanel->SelectedControl, FALSE);
          }
        } else if (SelectedControl->ControlId == LTDE_CONTROL_ID_DIALOG_HOT_KEY_SCROLL_DOWN) {
          if (!UserInputData->IsKeyboard) {
            PanelItem       = GetPanelItem (QuestionPanel, LTDE_PANEL_ITEM_ID_CONTENT);
            SelectedControl = GetNextSelectableControl (PanelItem, QuestionPanel->SelectedControl, FALSE);
          }
        }

        if (SelectedControl != NULL) {
          ExpectedSelectedControl = GetControlByHiiValue (QuestionPanel, &SelectedControl->HiiValue);
          if (QuestionPanel->SelectedControl != NULL &&
              QuestionPanel->SelectedControl != SelectedControl &&
              SelectedControl != ExpectedSelectedControl &&
              QuestionPanel->SelectedControl->ControlId == LTDE_CONTROL_ID_DIALOG_BODY &&
              SelectedControl->ControlId == LTDE_CONTROL_ID_DIALOG_BODY) {
            //
            // When moving to the duplicate option, move and refresh without sending ChangingQ
            //
            if (SelectedControl->ControlId == LTDE_CONTROL_ID_DIALOG_BODY) {
              BackupSelectedControl = QuestionPanel->SelectedControl;
              QuestionPanel->SelectedControl = SelectedControl;

              if (UpdatePanelContentItemPos (QuestionPanel)) {
                Status = DisplayQuestionPanel (QuestionPanel);
                break;
              }

              QuestionPanel->SelectedControl = BackupSelectedControl;
            }

            DisplayQuestionPanelControl (QuestionPanel, QuestionPanel->SelectedControl, FALSE);
            QuestionPanel->SelectedControl = SelectedControl;
            DisplayQuestionPanelControl (QuestionPanel, QuestionPanel->SelectedControl, TRUE);
          } else {
            Status = SendChangingQNotify (&SelectedControl->HiiValue, NULL);
          }
        }
      }
    } else if (IsShutdownDialog) {
      Status = SendShutDNotify ();
    }
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q:
    SelectedControl = GetControlByHiiValue (QuestionPanel, &((H2O_DISPLAY_ENGINE_EVT_CHANGING_Q *) Notify)->BodyHiiValue);
    if (SelectedControl == NULL || SelectedControl == QuestionPanel->SelectedControl) {
      break;
    }

    Status = EFI_SUCCESS;
    if (SelectedControl->ControlId == LTDE_CONTROL_ID_DIALOG_BODY) {
      BackupSelectedControl = QuestionPanel->SelectedControl;
      QuestionPanel->SelectedControl = SelectedControl;

      if (UpdatePanelContentItemPos (QuestionPanel)) {
        Status = DisplayQuestionPanel (QuestionPanel);
        break;
      }

      QuestionPanel->SelectedControl = BackupSelectedControl;
    }

    DisplayQuestionPanelControl (QuestionPanel, QuestionPanel->SelectedControl, FALSE);
    QuestionPanel->SelectedControl = SelectedControl;
    DisplayQuestionPanelControl (QuestionPanel, QuestionPanel->SelectedControl, TRUE);
    break;

  default:
    break;
  }

  return Status;
}


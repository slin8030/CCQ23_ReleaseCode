/** @file
  Checkbox question related functions.

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

/**
 Process event in checkbox opcode

 @param[in] Notify               A pointer to notify event
 @param[in] UserInputData        A pointer to user input data
 @param[in] QuestionPanel        A pointer to question panel

 @retval EFI_SUCCESS             Process event successful.
 @retval EFI_INVALID_PARAMETER   Notify or QuestionPanel is NULL.
 @retval EFI_UNSUPPORTED         Event is not processed by checkbox opcode.

**/
EFI_STATUS
CheckboxOpCodeProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  )
{
  EFI_STATUS                                  Status;
  H2O_LTDE_PANEL                              *SetupPagePanel;
  EFI_HII_VALUE                               HiiValue;

  if (Notify == NULL || QuestionPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status         = EFI_UNSUPPORTED;
  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);

  switch (Notify->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE:
    if (UserInputData == NULL || SetupPagePanel == NULL || SetupPagePanel->SelectedControl == NULL) {
      break;
    }

    if (mDEPrivate->DEStatus == DISPLAY_ENGINE_STATUS_AT_MENU) {
      if (UserInputData->KeyData.Key.UnicodeChar == CHAR_SPACE) {
        ZeroMem (&HiiValue, sizeof (EFI_HII_VALUE));
        HiiValue.Type    = EFI_IFR_TYPE_BOOLEAN;
        HiiValue.Value.b = (BOOLEAN) (SetupPagePanel->SelectedControl->HiiValue.Value.b ? FALSE : TRUE);
        return SendChangeQNotify (SetupPagePanel->SelectedControl->PageId, SetupPagePanel->SelectedControl->QuestionId, &HiiValue);
      }
    }
    break;

  default:
    break;
  }

  return Status;
}

/** @file
  Header file for question related definitions and functions.

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

#ifndef _LTDE_QUESTIONS_H_
#define _LTDE_QUESTIONS_H_

#include "H2ODisplayEngineLocalText.h"
#include "LTDEControl.h"

//
// Checkbox
//
EFI_STATUS
CheckboxOpCodeProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  );

//
// Date and Time
//
BOOLEAN
IsDateTimeOpCodeBeingModified (
  IN H2O_LTDE_CONTROL                         *Control
  );

H2O_DATE_TIME_ITEM
DateTimeOpCodeGetItemValue (
  IN H2O_LTDE_CONTROL                         *Control
  );

CHAR16 *
DateTimeOpCodeCreateValueStr (
  IN H2O_FORM_BROWSER_Q                       *Question
  );

EFI_STATUS
DateTimeOpCodeDisplayControl (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_CONTROL                         *Control
  );

EFI_STATUS
DateTimeOpCodeProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  );

//
// Numeric
//
EFI_STATUS
NumericOpCodeProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  );

//
// One Of
//
EFI_STATUS
OneOfQuestionProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  );

//
// Ordered List
//
CHAR16 *
OrderListOpCodeCreatePromptStr (
  IN H2O_FORM_BROWSER_Q                       *Question
  );

EFI_STATUS
OrderListOpCodeDisplayValueStr (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_PANEL_ITEM                      *ContentItem,
  IN H2O_LTDE_CONTROL                         *Control
  );

EFI_STATUS
OrderListOpCodeShiftOrder (
  IN H2O_LTDE_CONTROL                         *Control,
  IN BOOLEAN                                  ShiftNext
  );

EFI_STATUS
OrderListOpCodeProcessSelectQHotKey (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_PANEL_ITEM                      *ContentItem,
  IN BOOLEAN                                  GoNext
  );

EFI_STATUS
OrderListOpCodeProcessMouseClick (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_PANEL_ITEM                      *SelectedPanelItem,
  IN H2O_LTDE_CONTROL                         *SelectedControl,
  IN H2O_DISPLAY_ENGINE_USER_INPUT_DATA       *UserInputData
  );

EFI_STATUS
OrderListOpCodeProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  );

//
// Password
//
EFI_STATUS
PasswordOpCodeProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  );

//
// String
//
EFI_STATUS
StringOpCodeProcessEvt (
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_LTDE_PANEL                     *QuestionPanel
  );

#endif

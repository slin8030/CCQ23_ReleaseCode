/** @file
  Header file for panel related definitions and functions.

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

#ifndef _LTDE_PANELS_H_
#define _LTDE_PANELS_H_

#include "H2ODisplayEngineLocalText.h"
#include "LTDEControl.h"

//
// Title Panel
//
EFI_STATUS
InitTitlePanel (
  VOID
  );

EFI_STATUS
DisplayTitlePanel (
  VOID
  );

//
// Setup Menu Panel
//
EFI_STATUS
InitSetupMenuPanel (
  VOID
  );

EFI_STATUS
DisplaySetupMenuPanel (
  VOID
  );

EFI_STATUS
SetupMenuPanelProcessSelectQHotKey (
  IN H2O_LTDE_PANEL                           *SetupMenuPanel,
  IN BOOLEAN                                  GoNext
  );

EFI_STATUS
SetupMenuPanelProcessMouseClick (
  IN H2O_LTDE_PANEL                           *SetupMenuPanel,
  IN H2O_LTDE_PANEL_ITEM                      *SelectedPanelItem,
  IN H2O_LTDE_CONTROL                         *SelectedControl
  );

//
// Hot Key Panel
//
EFI_STATUS
InitHotKeyPanel (
  VOID
  );

EFI_STATUS
DisplayHotKeyPanel (
  VOID
  );

EFI_STATUS
HotKeyPanelProcessMouseClick (
  IN H2O_LTDE_PANEL                           *HotKeyPanel,
  IN H2O_LTDE_PANEL_ITEM                      *SelectedPanelItem,
  IN H2O_LTDE_CONTROL                         *SelectedControl,
  IN H2O_DISPLAY_ENGINE_USER_INPUT_DATA       *UserInputData
  );

//
// Help Text Panel
//
EFI_STATUS
InitHelpTextPanel (
  VOID
  );

EFI_STATUS
DisplayHelpTextPanel (
  VOID
  );

EFI_STATUS
HelpTextPanelProcessUserInput (
  IN EFI_INPUT_KEY                            *UserInputKey
  );

EFI_STATUS
HelpTextPanelProcessMouseClick (
  IN H2O_LTDE_PANEL_ITEM                      *SelectedPanelItem,
  IN H2O_LTDE_CONTROL                         *SelectedControl
  );

//
// Setup Page Panel
//
BOOLEAN
IsCurrentHighlight (
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  );

EFI_STATUS
UpdateSetupPageControl (
  IN  H2O_LTDE_PANEL                          *SetupPagePanel,
  IN  H2O_LTDE_PANEL_ITEM                     *ContentItem,
  IN  H2O_FORM_BROWSER_S                      *Statement,
  OUT H2O_LTDE_CONTROL                        *Control
  );

EFI_STATUS
InitSetupPagePanel (
  VOID
  );

EFI_STATUS
DisplaySetupPagePanel (
  VOID
  );

EFI_STATUS
DisplaySetupPagePanelContentItemControl (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_PANEL_ITEM                      *ContentItem,
  IN H2O_LTDE_CONTROL                         *Control
  );

EFI_STATUS
SetupPagePanelProcessSelectQEvt (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_DISPLAY_ENGINE_EVT_SELECT_Q          *SelectQ
  );

EFI_STATUS
SetupPagePanelProcessSelectQHotKey (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN BOOLEAN                                  GoNext
  );

EFI_STATUS
SetupPagePanelProcessOpenQHotKey (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_CONTROL                         *SelectedControl
  );

EFI_STATUS
SetupPagePanelProcessChangeQHotKey (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN BOOLEAN                                  Increase
  );

EFI_STATUS
SetupPagePanelProcessHotKey (
  IN EFI_INPUT_KEY                            *UserInputKey
  );

EFI_STATUS
SetupPagePanelProcessMouseClick (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_PANEL_ITEM                      *SelectedPanelItem,
  IN H2O_LTDE_CONTROL                         *SelectedControl,
  IN H2O_DISPLAY_ENGINE_USER_INPUT_DATA       *UserInputData
  );

//
// Question Panel
//
EFI_STATUS
DisplayQuestionPanelControl (
  IN H2O_LTDE_PANEL                           *QuestionPanel,
  IN H2O_LTDE_CONTROL                         *Control,
  IN BOOLEAN                                  IsHighlight
  );

EFI_STATUS
DisplayQuestionPanel (
  IN H2O_LTDE_PANEL                           *QuestionPanel
  );

EFI_STATUS
ShutdownQuestionPanel (
  IN H2O_LTDE_PANEL                           *QuestionPanel
  );

EFI_STATUS
QuestionPanelInitTitleItem (
  IN     H2O_FORM_BROWSER_D                   *Dialog,
  IN OUT H2O_LTDE_PANEL_ITEM                  *TitleItem
  );

EFI_STATUS
QuestionPanelInitContentItem (
  IN     H2O_FORM_BROWSER_D                   *Dialog,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  );

EFI_STATUS
QuestionPanelInitContentHotKeyItem (
  IN     H2O_FORM_BROWSER_D                   *Dialog,
  IN     UINT32                               PanelItemId,
  IN OUT H2O_LTDE_PANEL_ITEM                  *PanelItem
  );

EFI_STATUS
QuestionPanelInitButtonItem (
  IN     H2O_FORM_BROWSER_D                   *Dialog,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ButtonItem
  );

H2O_LTDE_CONTROL *
QuestionPanelGetDefaultHighlight (
  IN H2O_LTDE_PANEL                           *QuestionPanel
  );

EFI_STATUS
QuestionPanelUpdateField (
  IN H2O_FORM_BROWSER_D                       *Dialog,
  IN H2O_LTDE_PANEL                           *QuestionPanel
  );

EFI_STATUS
QuestionPanelProcessOpenDEvt (
  IN H2O_FORM_BROWSER_D                       *Dialog,
  IN H2O_LTDE_PANEL                           *QuestionPanel
  );

EFI_STATUS
QuestionPanelProcessUserInput (
  IN  H2O_LTDE_PANEL                          *QuestionPanel,
  IN  H2O_DISPLAY_ENGINE_USER_INPUT_DATA      *UserInputData,
  OUT H2O_LTDE_CONTROL                        **SelectedControl,
  OUT CHAR16                                  **InputString,
  OUT BOOLEAN                                 *IsShutdownDialog
  );

EFI_STATUS
DisplayPanelItem (
  IN H2O_LTDE_PANEL                           *Panel,
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem
  );

#endif

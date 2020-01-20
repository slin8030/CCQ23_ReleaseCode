/** @file
Utility functions for UI presentation.
;******************************************************************************
;* Copyright (c) 2013 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

Copyright (c) 2004 - 2013, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "InternalH2OFormBrowser.h"
//It descipbe HII package whether or not updates
BOOLEAN            mHiiPackageListUpdated;
//It describe curren selection
UI_MENU_SELECTION  *gCurrentSelection;
//It describe current HII handle
EFI_HII_HANDLE     mCurrentHiiHandle = NULL;
//It describe surrent FormSet
EFI_GUID           mCurrentFormSetGuid = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
//It describe current Form id
UINT16             mCurrentFormId = 0;
EFI_EVENT          mValueChangedEvent = NULL;
LIST_ENTRY         mRefreshEventList = INITIALIZE_LIST_HEAD_VARIABLE (mRefreshEventList);
UINT16             mCurFakeQestId;
FORM_DISPLAY_ENGINE_FORM gDisplayFormData;
BOOLEAN            mFinishRetrieveCall = FALSE;
BOOLEAN            mUpdateStatementStatus = FALSE;

extern EFI_GUID    mScuFormSetGuid;
extern USER_INPUT  *gUserInput;

/**
  Evaluate all expressions in a Form.

  @param  [in] FormSet   FormSet this Form belongs to.
  @param  [in] Form      The Form.

  @retval EFI_SUCCESS    The expression evaluated successfuly

**/
EFI_STATUS
EvaluateFormExpressions (
  IN FORM_BROWSER_FORMSET  *FormSet,
  IN FORM_BROWSER_FORM     *Form
  )
{
  EFI_STATUS       Status;
  LIST_ENTRY       *Link;
  FORM_EXPRESSION  *Expression;

  Link = GetFirstNode (&Form->ExpressionListHead);
  while (!IsNull (&Form->ExpressionListHead, Link)) {
    Expression = FORM_EXPRESSION_FROM_LINK (Link);
    Link = GetNextNode (&Form->ExpressionListHead, Link);

    if (Expression->Type == EFI_HII_EXPRESSION_INCONSISTENT_IF ||
        Expression->Type == EFI_HII_EXPRESSION_NO_SUBMIT_IF ||
        Expression->Type == EFI_HII_EXPRESSION_WARNING_IF ||
        Expression->Type == EFI_HII_EXPRESSION_WRITE ||
        (Expression->Type == EFI_HII_EXPRESSION_READ && Form->FormType != STANDARD_MAP_FORM_TYPE)) {
      //
      // Postpone Form validation to Question editing or Form submitting or Question Write or Question Read for nonstandard form.
      //
      continue;
    }

    Status = EvaluateExpression (FormSet, Form, Expression);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

/**
  Add empty function for event process function.

  @param Event    The Event need to be process
  @param Context  The context of the event.

**/
VOID
EFIAPI
SetupBrowserEmptyFunction (
  IN  EFI_EVENT    Event,
  IN  VOID         *Context
  )
{
}

/**
  Check whether this statement value is changed. If yes, update the statement value and return TRUE;
  else return FALSE.

  @param Statement           The statement need to check.

**/
VOID
UpdateStatement (
  IN OUT FORM_BROWSER_STATEMENT        *Statement
  )
{
  GetQuestionValue (gCurrentSelection->FormSet, gCurrentSelection->Form, Statement, GetSetValueWithHiiDriver);

  //
  // Reset FormPackage update flag
  //
  mHiiPackageListUpdated = FALSE;

  //
  // Question value may be changed, need invoke its Callback()
  //
  ProcessCallBackFunction (gCurrentSelection, gCurrentSelection->FormSet, gCurrentSelection->Form, Statement, EFI_BROWSER_ACTION_RETRIEVE, FALSE);

  if (mHiiPackageListUpdated) {
    //
    // Package list is updated, force to reparse IFR binary of target Formset
    //
    mHiiPackageListUpdated = FALSE;
    gCurrentSelection->Action = UI_ACTION_REFRESH_FORMSET;
  }
}


/**

  Enum all statement in current form, find all the statement can be display and
  add to the display form.

**/
VOID
AddStatementToDisplayForm (
  VOID
  )
{
  STATEMENT_REFRESH_ENTRY                *RefreshEntry;
  FORM_BROWSER_STATEMENT                 *Statement;
  FORM_BROWSER_STATEMENT                 *FirstStatement;
  FORM_BROWSER_STATEMENT                 *FirstSelectableStatement;
  LIST_ENTRY                             *Link;
  FORM_BROWSER_FORM                      *Form;
  H2O_FORM_BROWSER_PRIVATE_DATA          *Private;

  Private = &mFBPrivate;
  Private->FB.CurrentQ = NULL;
  Private->FB.CurrentP = NULL;

  //
  // Process the statement in this form.
  //
  ASSERT (gCurrentSelection->Form != NULL);

  FirstStatement = NULL;
  FirstSelectableStatement = NULL;

  Form = gCurrentSelection->Form;
  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Statement = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
    Link = GetNextNode (&gCurrentSelection->Form->StatementListHead, Link);

    //
    // This statement can't be show, skip it.
    //
    if (EvaluateExpressionList(Statement->Expression, FALSE, NULL, NULL) > ExpressGrayOut) {
      continue;
    }

    if (FirstStatement == NULL) {
      FirstStatement = Statement;
    }

    if (FirstSelectableStatement == NULL && IsSelectable (Statement)) {
      FirstSelectableStatement = Statement;
    }

    if (((gCurrentSelection->QuestionId != 0) && (Statement->QuestionId == gCurrentSelection->QuestionId)) ||
        ((mCurFakeQestId != 0) && (Statement->FakeQuestionId == mCurFakeQestId))) {
      Private->FB.CurrentQ = &Statement->Statement;
    }

    //
    // For RTC type of date/time, set default refresh interval to be 1 second.
    //
    if ((Statement->Operand == EFI_IFR_DATE_OP || Statement->Operand == EFI_IFR_TIME_OP) && Statement->Storage == NULL) {
      Statement->RefreshInterval = 1;
    }

    if (Statement->RefreshInterval != 0) {
      RefreshEntry = AllocateZeroPool (sizeof (STATEMENT_REFRESH_ENTRY));
      if (RefreshEntry == NULL) {
        return;
      }
      RefreshEntry->Statement = Statement;
      InsertTailList (&Private->RefreshList, &RefreshEntry->Link);
    }
  }

  if (Private->FB.CurrentQ == NULL) {
    if (FirstSelectableStatement != NULL) {
      Private->FB.CurrentQ = &FirstSelectableStatement->Statement;
    } else if (FirstStatement != NULL) {
      Private->FB.CurrentQ = &FirstStatement->Statement;
    }
  }

  //
  // Init Form (current is equal page id)
  //
  FBInitPageInfo (Private);
  Private->FB.CurrentP = &gCurrentSelection->Form->PageInfo;
}

/**

  Initialize the SettingChangedFlag variable in the display form.

**/
VOID
UpdateDataChangedFlag (
  VOID
  )
{
  LIST_ENTRY           *Link;
  FORM_BROWSER_FORMSET *LocalFormSet;

  gDisplayFormData.SettingChangedFlag   = FALSE;

  if (IsNvUpdateRequiredForForm (gCurrentSelection->Form)) {
    gDisplayFormData.SettingChangedFlag = TRUE;
    return;
  }

  //
  // Base on the system level to check whether need to show the NV flag.
  //
  switch (gBrowserSettingScope) {
  case SystemLevel:
    //
    // Check the maintain list to see whether there is any change.
    //
    Link = GetFirstNode (&gBrowserFormSetList);
    while (!IsNull (&gBrowserFormSetList, Link)) {
      LocalFormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
      if (IsNvUpdateRequiredForFormSet(LocalFormSet)) {
        gDisplayFormData.SettingChangedFlag = TRUE;
        return;
      }
      Link = GetNextNode (&gBrowserFormSetList, Link);
    }
    break;

  case FormSetLevel:
    if (IsNvUpdateRequiredForFormSet(gCurrentSelection->FormSet)) {
      gDisplayFormData.SettingChangedFlag = TRUE;
      return;
    }
    break;

  default:
    break;
  }
}

/**

  Initialize the Display form structure data.

**/
VOID
InitializeDisplayFormData (
  VOID
  )
{

  gDisplayFormData.Signature   = FORM_DISPLAY_ENGINE_FORM_SIGNATURE;
  gDisplayFormData.Version     = FORM_DISPLAY_ENGINE_VERSION_1;
  gDisplayFormData.ImageId     = 0;
  gDisplayFormData.AnimationId = 0;

  InitializeListHead (&gDisplayFormData.StatementListHead);
  InitializeListHead (&gDisplayFormData.StatementListOSF);
  InitializeListHead (&gDisplayFormData.HotKeyListHead);

}

/**

  Update the Display form structure data.

**/
VOID
UpdateDisplayFormData (
  VOID
  )
{
  gDisplayFormData.FormTitle        = gCurrentSelection->Form->FormTitle;
  gDisplayFormData.FormId           = gCurrentSelection->FormId;
  gDisplayFormData.HiiHandle        = gCurrentSelection->Handle;
  CopyGuid (&gDisplayFormData.FormSetGuid, &gCurrentSelection->FormSetGuid);

  gDisplayFormData.Attribute        = 0;
  gDisplayFormData.Attribute       |= gCurrentSelection->Form->ModalForm ? HII_DISPLAY_MODAL : 0;
  gDisplayFormData.Attribute       |= gCurrentSelection->Form->Locked    ? HII_DISPLAY_LOCK  : 0;

  gDisplayFormData.FormRefreshEvent     = NULL;
  gDisplayFormData.HighLightedStatement = NULL;

  UpdateDataChangedFlag ();

  AddStatementToDisplayForm ();
}

/**

  Free the Display form structure data.

**/
VOID
FreeDisplayFormData (
  VOID
  )
{
  LIST_ENTRY                             *Link;
  TIMERINFO                              *TimerInfo;
  STATEMENT_REFRESH_ENTRY                *RefreshEntry;
  H2O_FORM_BROWSER_PRIVATE_DATA          *Private;

  Private = &mFBPrivate;

  //
  // kill all timer events
  //
  while (!IsListEmpty (&Private->TimerList)) {
    Link = GetFirstNode(&Private->TimerList);
    TimerInfo = TIMERINFO_FROM_LINK (Link);
    RemoveEntryList (&TimerInfo->Link);
    FreePool (TimerInfo->NotifyEvent);
    FreePool (TimerInfo);
  }

  //
  // empty refresh list
  //
  while (!IsListEmpty (&Private->RefreshList)) {
    RefreshEntry = (STATEMENT_REFRESH_ENTRY *)GetFirstNode (&Private->RefreshList);
    RemoveEntryList (&RefreshEntry->Link);
    FreePool (RefreshEntry);
  }
}

/**

  Get FORM_BROWSER_STATEMENT from FORM_DISPLAY_ENGINE_STATEMENT based on the OpCode info.

  @param DisplayStatement        The input FORM_DISPLAY_ENGINE_STATEMENT.

  @retval FORM_BROWSER_STATEMENT  The return FORM_BROWSER_STATEMENT info.

**/
FORM_BROWSER_STATEMENT *
GetBrowserStatement (
  IN FORM_DISPLAY_ENGINE_STATEMENT *DisplayStatement
  )
{
  FORM_BROWSER_STATEMENT *Statement;
  LIST_ENTRY             *Link;

  Link = GetFirstNode (&gCurrentSelection->Form->StatementListHead);
  while (!IsNull (&gCurrentSelection->Form->StatementListHead, Link)) {
    Statement = FORM_BROWSER_STATEMENT_FROM_LINK (Link);

    if (Statement->OpCode == DisplayStatement->OpCode) {
      return Statement;
    }

    Link = GetNextNode (&gCurrentSelection->Form->StatementListHead, Link);
  }

  return NULL;
}

/**
  Update the ValueChanged status for questions in this form.

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.

**/
VOID
UpdateStatementStatusForForm (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form
  )
{
  LIST_ENTRY                  *Link;
  FORM_BROWSER_STATEMENT      *Question;

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
    Link = GetNextNode (&Form->StatementListHead, Link);

    IsQuestionValueChanged(FormSet, Form, Question, GetSetValueWithBuffer);
  }
}

/**
  Update the ValueChanged status for questions in this formset.

  @param  FormSet                FormSet data structure.

**/
VOID
UpdateStatementStatusForFormSet (
  IN FORM_BROWSER_FORMSET                *FormSet
  )
{
  LIST_ENTRY                  *Link;
  FORM_BROWSER_FORM           *Form;

  Link = GetFirstNode (&FormSet->FormListHead);
  while (!IsNull (&FormSet->FormListHead, Link)) {
    Form = FORM_BROWSER_FORM_FROM_LINK (Link);
    Link = GetNextNode (&FormSet->FormListHead, Link);

    UpdateStatementStatusForForm (FormSet, Form);
  }
}

/**
  Update the ValueChanged status for questions.

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  SettingScope           Setting Scope for Default action.

**/
VOID
UpdateStatementStatus (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN BROWSER_SETTING_SCOPE            SettingScope
  )
{
  LIST_ENTRY                  *Link;
  FORM_BROWSER_FORMSET        *LocalFormSet;

  switch (SettingScope) {
  case SystemLevel:
    Link = GetFirstNode (&gBrowserFormSetList);
    while (!IsNull (&gBrowserFormSetList, Link)) {
      LocalFormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
      Link = GetNextNode (&gBrowserFormSetList, Link);
      if (!ValidateFormSet(LocalFormSet)) {
        continue;
      }

      UpdateStatementStatusForFormSet (LocalFormSet);
    }
    break;

  case FormSetLevel:
    UpdateStatementStatusForFormSet (FormSet);
    break;

  case FormLevel:
    UpdateStatementStatusForForm (FormSet, Form);
    break;

  default:
    break;
  }
}

/**

  Process the action request in user input.

  @param Action                  The user input action request info.
  @param DefaultId               The user input default Id info.

  @retval EFI_SUCESSS            This function always return successfully for now.

**/
EFI_STATUS
ProcessAction (
  IN UINT32        Action,
  IN UINT16        DefaultId
  )
{
  EFI_STATUS    Status;

  //
  // This is caused by use press ESC, and it should not combine with other action type.
  //
  if ((Action & BROWSER_ACTION_FORM_EXIT) == BROWSER_ACTION_FORM_EXIT) {
    FindNextMenu (gCurrentSelection, FormLevel);
    return EFI_SUCCESS;
  }

  //
  // Below is normal hotkey trigged action, these action maybe combine with each other.
  //
  if ((Action & BROWSER_ACTION_DISCARD) == BROWSER_ACTION_DISCARD) {
    DiscardForm (gCurrentSelection->FormSet, gCurrentSelection->Form, gBrowserSettingScope);
  }

  if ((Action & BROWSER_ACTION_DEFAULT) == BROWSER_ACTION_DEFAULT) {
    ExtractDefault (gCurrentSelection->FormSet, gCurrentSelection->Form, DefaultId, gBrowserSettingScope, GetDefaultForAll, NULL, FALSE);
    UpdateStatementStatus (gCurrentSelection->FormSet, gCurrentSelection->Form, gBrowserSettingScope);
  }

  if ((Action & BROWSER_ACTION_SUBMIT) == BROWSER_ACTION_SUBMIT) {
    Status = SubmitForm (gCurrentSelection->FormSet, gCurrentSelection->Form, gBrowserSettingScope);
    if (EFI_ERROR (Status)) {
      PopupErrorMessage(BROWSER_SUBMIT_FAIL, NULL, NULL);
    }
  }

  if ((Action & BROWSER_ACTION_RESET) == BROWSER_ACTION_RESET) {
    gResetRequired = TRUE;
  }

  if ((Action & BROWSER_ACTION_EXIT) == BROWSER_ACTION_EXIT) {
    //
    // Form Exit without saving, Similar to ESC Key.
    // FormSet Exit without saving, Exit SendForm.
    // System Exit without saving, CallExitHandler and Exit SendForm.
    //
    DiscardForm (gCurrentSelection->FormSet, gCurrentSelection->Form, gBrowserSettingScope);
    if (gBrowserSettingScope == FormLevel || gBrowserSettingScope == FormSetLevel) {
      FindNextMenu (gCurrentSelection, gBrowserSettingScope);
    } else if (gBrowserSettingScope == SystemLevel) {
      if (ExitHandlerFunction != NULL) {
        ExitHandlerFunction ();
      }
      gCurrentSelection->Action = UI_ACTION_EXIT;
    }
  }

  return EFI_SUCCESS;
}

/**
  Check whether the formset guid is in this Hii package list.

  @param  HiiHandle              The HiiHandle for this HII package list.
  @param  FormSetGuid            The formset guid for the request formset.

  @retval TRUE                   Find the formset guid.
  @retval FALSE                  Not found the formset guid.

**/
BOOLEAN
GetFormsetGuidFromHiiHandle (
  IN EFI_HII_HANDLE       HiiHandle,
  IN EFI_GUID             *FormSetGuid
  )
{
  EFI_HII_PACKAGE_LIST_HEADER  *HiiPackageList;
  UINTN                        BufferSize;
  UINT32                       Offset;
  UINT32                       Offset2;
  UINT32                       PackageListLength;
  EFI_HII_PACKAGE_HEADER       PackageHeader;
  UINT8                        *Package;
  UINT8                        *OpCodeData;
  EFI_STATUS                   Status;
  BOOLEAN                      FindGuid;

  BufferSize     = 0;
  HiiPackageList = NULL;
  FindGuid       = FALSE;

  Status = gHiiDatabase->ExportPackageLists (gHiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    HiiPackageList = AllocatePool (BufferSize);
    ASSERT (HiiPackageList != NULL);

    Status = gHiiDatabase->ExportPackageLists (gHiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  }
  if (EFI_ERROR (Status) || HiiPackageList == NULL) {
    return FALSE;
  }

  //
  // Get Form package from this HII package List
  //
  Offset = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  Offset2 = 0;
  CopyMem (&PackageListLength, &HiiPackageList->PackageLength, sizeof (UINT32));

  while (Offset < PackageListLength) {
    Package = ((UINT8 *) HiiPackageList) + Offset;
    CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));
    Offset += PackageHeader.Length;

    if (PackageHeader.Type == EFI_HII_PACKAGE_FORMS) {
      //
      // Search FormSet in this Form Package
      //
      Offset2 = sizeof (EFI_HII_PACKAGE_HEADER);
      while (Offset2 < PackageHeader.Length) {
        OpCodeData = Package + Offset2;

        if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_SET_OP) {
          if (CompareGuid (FormSetGuid, (EFI_GUID *)(OpCodeData + sizeof (EFI_IFR_OP_HEADER)))){
            FindGuid = TRUE;
            break;
          }
        }

        Offset2 += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;
      }
    }
    if (FindGuid) {
      break;
    }
  }

  FreePool (HiiPackageList);

  return FindGuid;
}

/**
  Find HII Handle in the HII database associated with given Device Path.

  If DevicePath is NULL, then ASSERT.

  @param  DevicePath             Device Path associated with the HII package list
                                 handle.
  @param  FormsetGuid            The formset guid for this formset.

  @retval Handle                 HII package list Handle associated with the Device
                                        Path.
  @retval NULL                   Hii Package list handle is not found.

**/
EFI_HII_HANDLE
DevicePathToHiiHandle (
  IN EFI_DEVICE_PATH_PROTOCOL   *DevicePath,
  IN EFI_GUID                   *FormsetGuid
  )
{
  EFI_STATUS                  Status;
  EFI_DEVICE_PATH_PROTOCOL    *TmpDevicePath;
  UINTN                       Index;
  EFI_HANDLE                  Handle;
  EFI_HANDLE                  DriverHandle;
  EFI_HII_HANDLE              *HiiHandles;
  EFI_HII_HANDLE              HiiHandle;

  ASSERT (DevicePath != NULL);

  TmpDevicePath = DevicePath;
  //
  // Locate Device Path Protocol handle buffer
  //
  Status = gBS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid,
                  &TmpDevicePath,
                  &DriverHandle
                  );
  if (EFI_ERROR (Status) || !IsDevicePathEnd (TmpDevicePath)) {
    return NULL;
  }

  //
  // Retrieve all HII Handles from HII database
  //
  HiiHandles = HiiGetHiiHandles (NULL);
  if (HiiHandles == NULL) {
    return NULL;
  }

  //
  // Search Hii Handle by Driver Handle
  //
  HiiHandle = NULL;
  for (Index = 0; HiiHandles[Index] != NULL; Index++) {
    Status = gHiiDatabase->GetPackageListHandle (
                             gHiiDatabase,
                             HiiHandles[Index],
                             &Handle
                             );
    if (!EFI_ERROR (Status) && (Handle == DriverHandle)) {
      if (GetFormsetGuidFromHiiHandle(HiiHandles[Index], FormsetGuid)) {
        HiiHandle = HiiHandles[Index];
        break;
      }

      if (HiiHandle != NULL) {
        break;
      }
    }
  }

  FreePool (HiiHandles);
  return HiiHandle;
}

/**
  Find HII Handle in the HII database associated with given form set guid.

  If FormSetGuid is NULL, then ASSERT.

  @param  ComparingGuid          FormSet Guid associated with the HII package list
                                 handle.

  @retval Handle                 HII package list Handle associated with the Device
                                        Path.
  @retval NULL                   Hii Package list handle is not found.

**/
EFI_HII_HANDLE
FormSetGuidToHiiHandle (
  EFI_GUID     *ComparingGuid
  )
{
  EFI_HII_HANDLE               *HiiHandles;
  EFI_HII_HANDLE               HiiHandle;
  UINTN                        Index;

  ASSERT (ComparingGuid != NULL);

  HiiHandle  = NULL;
  //
  // Get all the Hii handles
  //
  HiiHandles = HiiGetHiiHandles (NULL);
  ASSERT (HiiHandles != NULL);
  if (HiiHandles == NULL) {
    return NULL;
  }

  //
  // Search for formset of each class type
  //
  for (Index = 0; HiiHandles[Index] != NULL; Index++) {
    if (GetFormsetGuidFromHiiHandle(HiiHandles[Index], ComparingGuid)) {
      HiiHandle = HiiHandles[Index];
      break;
    }

    if (HiiHandle != NULL) {
      break;
    }
  }

  FreePool (HiiHandles);

  return HiiHandle;
}

/**
  check how to process the changed data in current form or form set.

  @param Selection       On input, Selection tell setup browser the information
                         about the Selection, form and formset to be displayed.
                         On output, Selection return the screen item that is selected
                         by user.

  @param Scope           Data save or discard scope, form or formset.

  @retval                TRUE   Success process the changed data, will return to the parent form.
  @retval                FALSE  Reject to process the changed data, will stay at  current form.
**/
BOOLEAN
ProcessChangedData (
  IN OUT UI_MENU_SELECTION       *Selection,
  IN     BROWSER_SETTING_SCOPE   Scope
  )
{
  BOOLEAN  RetValue;

  RetValue = TRUE;
  switch (ConfirmDataChange()) {
    case BROWSER_ACTION_DISCARD:
      DiscardForm (Selection->FormSet, Selection->Form, Scope);
      break;

    case BROWSER_ACTION_SUBMIT:
      SubmitForm (Selection->FormSet, Selection->Form, Scope);
      break;

    case BROWSER_ACTION_NONE:
      RetValue = FALSE;
      break;

    default:
      //
      // if Invalid value return, process same as BROWSER_ACTION_NONE.
      //
      RetValue = FALSE;
      break;
  }

  return RetValue;
}

/**
  Find parent formset menu(the first menu which has different formset) for current menu.
  If not find, just return to the first menu.

  @param Selection    The selection info.

**/
VOID
FindParentFormSet (
  IN OUT   UI_MENU_SELECTION           *Selection
  )
{
  FORM_ENTRY_INFO            *CurrentMenu;
  FORM_ENTRY_INFO            *ParentMenu;

  CurrentMenu = Selection->CurrentMenu;
  ParentMenu  = UiFindParentMenu(CurrentMenu);

  //
  // Find a menu which has different formset guid with current.
  //
  while (ParentMenu != NULL && CompareGuid (&CurrentMenu->FormSetGuid, &ParentMenu->FormSetGuid)) {
    CurrentMenu = ParentMenu;
    ParentMenu  = UiFindParentMenu(CurrentMenu);
  }

  if (ParentMenu != NULL) {
    CopyMem (&Selection->FormSetGuid, &ParentMenu->FormSetGuid, sizeof (EFI_GUID));
    Selection->Handle = ParentMenu->HiiHandle;
    Selection->FormId     = ParentMenu->FormId;
    Selection->QuestionId = ParentMenu->QuestionId;
  } else {
    Selection->FormId     = CurrentMenu->FormId;
    Selection->QuestionId = CurrentMenu->QuestionId;
  }

  Selection->Statement  = NULL;
}

/**
  Process the goto op code, update the info in the selection structure.

  @param Statement    The statement belong to goto op code.
  @param Selection    The selection info.

  @retval EFI_SUCCESS    The menu process successfully.
  @return Other value if the process failed.
**/
EFI_STATUS
ProcessGotoOpCode (
  IN OUT   FORM_BROWSER_STATEMENT      *Statement,
  IN OUT   UI_MENU_SELECTION           *Selection
  )
{
  CHAR16                          *StringPtr;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  FORM_BROWSER_FORM               *RefForm;
  EFI_STATUS                      Status;
  EFI_HII_HANDLE                  HiiHandle;

  Status    = EFI_SUCCESS;
  StringPtr = NULL;
  HiiHandle = NULL;

  //
  // Prepare the device path check, get the device path info first.
  //
  if (Statement->HiiValue.Value.ref.DevicePath != 0) {
    StringPtr = GetString (Statement->HiiValue.Value.ref.DevicePath, Selection->FormSet->HiiHandle);
  }

  //
  // Check whether the device path string is a valid string.
  //
  if (Statement->HiiValue.Value.ref.DevicePath != 0 && StringPtr != NULL && StringPtr[0] != L'\0') {
    if (Selection->Form->ModalForm) {
      return Status;
    }

    //
    // Goto another Hii Package list
    //
    if (mPathFromText != NULL) {
      DevicePath = mPathFromText->ConvertTextToDevicePath(StringPtr);
      if (DevicePath != NULL) {
        HiiHandle = DevicePathToHiiHandle (DevicePath, &Statement->HiiValue.Value.ref.FormSetGuid);
        FreePool (DevicePath);
      }
      FreePool (StringPtr);
    } else {
      //
      // Not found the EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL protocol.
      //
      PopupErrorMessage(BROWSER_PROTOCOL_NOT_FOUND, NULL, NULL);
      FreePool (StringPtr);
      return Status;
    }

    if (HiiHandle != Selection->Handle) {
      //
      // Goto another Formset, check for uncommitted data
      //
      if ((gBrowserSettingScope == FormLevel || gBrowserSettingScope == FormSetLevel) &&
          IsNvUpdateRequiredForFormSet(Selection->FormSet)) {
        if (!ProcessChangedData(Selection, FormSetLevel)) {
          return EFI_SUCCESS;
        }
      }
    }

    Selection->Action = UI_ACTION_REFRESH_FORMSET;
    Selection->Handle = (HiiHandle == NULL) ? FormSetGuidToHiiHandle(&Statement->HiiValue.Value.ref.FormSetGuid) : HiiHandle;
    if (Selection->Handle == NULL) {
      //
      // If target Hii Handle not found, exit current formset.
      //
      FindParentFormSet(Selection);
      return EFI_SUCCESS;
    }

    CopyMem (&Selection->FormSetGuid,&Statement->HiiValue.Value.ref.FormSetGuid, sizeof (EFI_GUID));
    Selection->FormId = Statement->HiiValue.Value.ref.FormId;
    Selection->QuestionId = Statement->HiiValue.Value.ref.QuestionId;
  } else if (!CompareGuid (&Statement->HiiValue.Value.ref.FormSetGuid, &gZeroGuid)) {
    if (Selection->Form->ModalForm) {
      return Status;
    }
    if (!CompareGuid (&Statement->HiiValue.Value.ref.FormSetGuid, &Selection->FormSetGuid)) {
      //
      // Goto another Formset, check for uncommitted data
      //
      if ((gBrowserSettingScope == FormLevel || gBrowserSettingScope == FormSetLevel) &&
         IsNvUpdateRequiredForFormSet(Selection->FormSet)) {
        if (!ProcessChangedData(Selection, FormSetLevel)) {
          return EFI_SUCCESS;
        }
      }
    }

    Selection->Action = UI_ACTION_REFRESH_FORMSET;
    Selection->Handle = FormSetGuidToHiiHandle(&Statement->HiiValue.Value.ref.FormSetGuid);
    if (Selection->Handle == NULL) {
      //
      // If target Hii Handle not found, exit current formset.
      //
      FindParentFormSet(Selection);
      return EFI_SUCCESS;
    }

    CopyMem (&Selection->FormSetGuid, &Statement->HiiValue.Value.ref.FormSetGuid, sizeof (EFI_GUID));
    Selection->FormId = Statement->HiiValue.Value.ref.FormId;
    Selection->QuestionId = Statement->HiiValue.Value.ref.QuestionId;
  } else if (Statement->HiiValue.Value.ref.FormId != 0) {
    //
    // Goto another Form, check for uncommitted data
    //
    if (Statement->HiiValue.Value.ref.FormId != Selection->FormId) {
      if ((gBrowserSettingScope == FormLevel && IsNvUpdateRequiredForForm(Selection->Form))) {
        if (!ProcessChangedData (Selection, FormLevel)) {
          return EFI_SUCCESS;
        }
      }
    }

    RefForm = IdToForm (Selection->FormSet, Statement->HiiValue.Value.ref.FormId);
    if ((RefForm != NULL) && (RefForm->SuppressExpression != NULL)) {
      if (EvaluateExpressionList(RefForm->SuppressExpression, TRUE, Selection->FormSet, RefForm) != ExpressFalse) {
        //
        // Form is suppressed.
        //
        PopupErrorMessage(BROWSER_FORM_SUPPRESS, NULL, NULL);
        return EFI_SUCCESS;
      }
    }

    Selection->FormId = Statement->HiiValue.Value.ref.FormId;
    Selection->QuestionId = Statement->HiiValue.Value.ref.QuestionId;
  } else if (Statement->HiiValue.Value.ref.QuestionId != 0) {
    Selection->QuestionId = Statement->HiiValue.Value.ref.QuestionId;
  }

  return Status;
}

/**
  Check whether config header string is in <ConfigHdr> format (i.e. GUID=...&NAME=...&PATH=...) or not.

  @param[in] ConfigHdrStr        Config header string pointer

  @retval TRUE                   Config header string is in <ConfigHdr> format.
  @retval FALSE                  Config header string is not in <ConfigHdr> format.
**/
BOOLEAN
IsValidConfigHdr (
  IN CONST CHAR16             *ConfigHdrStr
  )
{
  CHAR16                      *StringPtr;

  if (ConfigHdrStr == NULL) {
    return FALSE;
  }

  StringPtr = (CHAR16 *) ConfigHdrStr;

  StringPtr = StrStr (StringPtr, L"GUID=");
  if (StringPtr == NULL) {
    return FALSE;
  }
  StringPtr += StrLen (L"GUID=");

  StringPtr = StrStr (StringPtr, L"&NAME=");
  if (StringPtr == NULL) {
    return FALSE;
  }
  StringPtr += StrLen (L"&NAME=");

  StringPtr = StrStr (StringPtr, L"&PATH=");
  if (StringPtr == NULL) {
    return FALSE;
  }

  return TRUE;
}

/**
  Process Question Config.

  @param  Selection              The UI menu selection.
  @param  Question               The Question to be peocessed.

  @retval EFI_SUCCESS            Question Config process success.
  @retval Other                  Question Config process fail.

**/
EFI_STATUS
ProcessQuestionConfig (
  IN  UI_MENU_SELECTION       *Selection,
  IN  FORM_BROWSER_STATEMENT  *Question
  )
{
  EFI_STATUS                      Status;
  CHAR16                          *ConfigResp;
  CHAR16                          *Progress;

  if (Question->QuestionConfig == 0) {
    return EFI_SUCCESS;
  }

  //
  // Get <ConfigResp>
  //
  ConfigResp = GetString (Question->QuestionConfig, Selection->FormSet->HiiHandle);
  if (ConfigResp == NULL) {
    return EFI_NOT_FOUND;
  }

  if (!IsValidConfigHdr (ConfigResp)) {
    FreePool (ConfigResp);
    return EFI_SUCCESS;
  }

  //
  // Send config to Configuration Driver
  //
  Status = gHiiConfigRouting->RouteConfig (
                                gHiiConfigRouting,
                                ConfigResp,
                                &Progress
                                );

  return Status;
}

/**

  Process the user input data.

  @param UserInput               The user input data.
  @param ChangeHighlight         Whether need to change the highlight statement.

  @retval EFI_SUCESSS            This function always return successfully for now.

**/
EFI_STATUS
ProcessUserInput (
  IN USER_INPUT               *UserInput,
  IN BOOLEAN                  ChangeHighlight
  )
{
  EFI_STATUS                    Status;
  FORM_BROWSER_STATEMENT        *Statement;

  Status = EFI_SUCCESS;

  //
  // When Exit from FormDisplay function, one of the below two cases must be true.
  //
  ASSERT (UserInput->Action != 0 || UserInput->SelectedStatement != NULL);

  //
  // Remove the last highligh question id, this id will update when show next form.
  //
  if (UserInput->Action != BROWSER_ACTION_GOTO) {
    gCurrentSelection->QuestionId = 0;
  }

  //
  // First process the Action field in USER_INPUT.
  //
  if (UserInput->Action != 0) {
    Status = ProcessAction (UserInput->Action, UserInput->DefaultId);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Clear the highlight info.
    //
    gCurrentSelection->Statement = NULL;

    if (UserInput->SelectedStatement != NULL) {
      Statement = GetBrowserStatement(UserInput->SelectedStatement);
      ASSERT (Statement != NULL);
      if (Statement == NULL) {
        return EFI_NOT_FOUND;
      }
      //
      // Save the current highlight menu in the menu history data.
      // which will be used when later browse back to this form.
      //
      gCurrentSelection->CurrentMenu->QuestionId = Statement->QuestionId;
      //
      // For statement like text, actio, it not has question id.
      // So use FakeQuestionId to save the question.
      //
      if (gCurrentSelection->CurrentMenu->QuestionId == 0) {
        mCurFakeQestId = Statement->FakeQuestionId;
      } else {
        mCurFakeQestId = 0;
      }
    }
  } else {
    Statement = GetBrowserStatement(UserInput->SelectedStatement);
    ASSERT (Statement != NULL);
    if (Statement == NULL) {
      return EFI_NOT_FOUND;
    }

    gCurrentSelection->Statement = Statement;

    if (ChangeHighlight) {
      //
      // This question is the current user select one,record it and later
      // show it as the highlight question.
      //
      gCurrentSelection->CurrentMenu->QuestionId = Statement->QuestionId;
      //
      // For statement like text, actio, it not has question id.
      // So use FakeQuestionId to save the question.
      //
      if (gCurrentSelection->CurrentMenu->QuestionId == 0) {
        mCurFakeQestId = Statement->FakeQuestionId;
      } else {
        mCurFakeQestId = 0;
      }
    }

    switch (Statement->Operand) {
    case EFI_IFR_REF_OP:
      Status = ProcessGotoOpCode(Statement, gCurrentSelection);
      break;

    case EFI_IFR_ACTION_OP:
      //
      // Process the Config string <ConfigResp>
      //
      Status = ProcessQuestionConfig (gCurrentSelection, Statement);
      break;

    case EFI_IFR_RESET_BUTTON_OP:
      //
      // Reset Question to default value specified by DefaultId
      //
      Status = ExtractDefault (gCurrentSelection->FormSet, NULL, Statement->DefaultId, FormSetLevel, GetDefaultForAll, NULL, FALSE);
      UpdateStatementStatus (gCurrentSelection->FormSet, NULL, FormSetLevel);
      break;

    default:
      switch (Statement->Operand) {
      case EFI_IFR_STRING_OP:
        DeleteString(Statement->HiiValue.Value.string, gCurrentSelection->FormSet->HiiHandle);
        Statement->HiiValue.Value.string = UserInput->InputValue.Value.string;
        CopyMem (Statement->BufferValue, UserInput->InputValue.Buffer, (UINTN) UserInput->InputValue.BufferLen);
        FreePool (UserInput->InputValue.Buffer);
        break;

      case EFI_IFR_PASSWORD_OP:
        if (UserInput->InputValue.Buffer == NULL) {
          //
          // User not input new password, just return.
          //
          break;
        }

        DeleteString(Statement->HiiValue.Value.string, gCurrentSelection->FormSet->HiiHandle);
        Statement->HiiValue.Value.string = UserInput->InputValue.Value.string;
        CopyMem (Statement->BufferValue, UserInput->InputValue.Buffer, (UINTN) UserInput->InputValue.BufferLen);
        FreePool (UserInput->InputValue.Buffer);
        //
        // Two password match, send it to Configuration Driver
        //
        if ((Statement->QuestionFlags & EFI_IFR_FLAG_CALLBACK) != 0) {
          Status = PasswordCheck (NULL, UserInput->SelectedStatement, (CHAR16 *) Statement->BufferValue);
          ShowPwdStatusMessage (0, Status);
          Status = EFI_SUCCESS;
          //
          // Clean the value after saved it.
          //
          ZeroMem (Statement->BufferValue, (UINTN) UserInput->InputValue.BufferLen);
          HiiSetString (gCurrentSelection->FormSet->HiiHandle, Statement->HiiValue.Value.string, (CHAR16*)Statement->BufferValue, NULL);
        } else {
          SetQuestionValue (gCurrentSelection->FormSet, gCurrentSelection->Form, Statement, GetSetValueWithHiiDriver);
        }
        break;

      case EFI_IFR_ORDERED_LIST_OP:
        CopyMem (Statement->BufferValue, UserInput->InputValue.Buffer, UserInput->InputValue.BufferLen);
        break;

      default:
        CopyMem (&Statement->HiiValue, &UserInput->InputValue, sizeof (EFI_HII_VALUE));
        break;
      }
      break;
    }
  }

  return Status;
}

/**

  Display form and wait for user to select one menu option, then return it.

  @retval EFI_SUCESSS            This function always return successfully for now.

**/
EFI_STATUS
DisplayForm (
  VOID
  )
{
  EFI_STATUS               Status;
  USER_INPUT               UserInput;
  FORM_ENTRY_INFO          *CurrentMenu;
  BOOLEAN                  ChangeHighlight;

  ZeroMem (&UserInput, sizeof (USER_INPUT));

  //
  // Update the menu history data.
  //
  CurrentMenu = UiFindMenuList (gCurrentSelection->Handle, &gCurrentSelection->FormSetGuid, gCurrentSelection->FormId);
  if (CurrentMenu == NULL) {
    //
    // Current menu not found, add it to the menu tree
    //
    CurrentMenu = UiAddMenuList (gCurrentSelection->Handle, &gCurrentSelection->FormSetGuid,
                                 gCurrentSelection->FormId, gCurrentSelection->QuestionId);
    ASSERT (CurrentMenu != NULL);
    if (CurrentMenu == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  gCurrentSelection->CurrentMenu = CurrentMenu;

  //
  // Find currrent highlight statement.
  //
  if (gCurrentSelection->QuestionId == 0) {
    //
    // Highlight not specified, fetch it from cached menu
    //
    gCurrentSelection->QuestionId = CurrentMenu->QuestionId;
  }

  //
  // Evaluate all the Expressions in this Form
  //
  Status = EvaluateFormExpressions (gCurrentSelection->FormSet, gCurrentSelection->Form);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  UpdateDisplayFormData ();

  //
  // Three possible status maybe return.
  //
  // EFI_INVALID_PARAMETER: The input dimension info is not valid.
  // EFI_NOT_FOUND:         The input value for oneof/orderedlist opcode is not valid
  //                        and an valid value has return.
  // EFI_SUCCESS:           Success shows form and get user input in UserInput paramenter.
  //
  Status = FormDisplay (&gDisplayFormData, &UserInput);
  if (EFI_ERROR (Status) && Status != EFI_NOT_FOUND) {
    FreeDisplayFormData();
    return Status;
  }

  //
  // If status is EFI_SUCCESS, means user has change the highlight menu and new user input return.
  //                           in this case, browser need to change the highlight menu.
  // If status is EFI_NOT_FOUND, means the input DisplayFormData has error for oneof/orderedlist
  //                          opcode and new valid value has return, browser core need to adjust
  //                          value for this opcode and shows this form again.
  //
  ChangeHighlight = (Status == EFI_SUCCESS ? TRUE :FALSE);

  Status = ProcessUserInput (&UserInput, ChangeHighlight);

  FreeDisplayFormData();

  return Status;
}
/**
  the FB uses this function, which is to update the internal data structure
  when a Callback() member function of the CONFIG_ACCESS protocol
  updates an existing Form or Form Set.
  Functions which are registered to receive notification of
  database events have this prototype. The actual event is encoded
  in NotifyType. The following table describes how PackageType,
  PackageGuid, Handle, and Package are used for each of the
  notification types.

  @param [in] PackageType  Package type of the notification.

  @param [in] PackageGuid  If PackageType is
                           EFI_HII_PACKAGE_TYPE_GUID, then this is
                           the pointer to the GUID from the Guid
                           field of EFI_HII_PACKAGE_GUID_HEADER.
                           Otherwise, it must be NULL.

  @param [in] Package      Points to the package referred to by the
                           notification Handle The handle of the package
                           list which contains the specified package.

  @param [in] Handle       The HII handle.

  @param [in] NotifyType   The type of change concerning the
                           database. See
                           EFI_HII_DATABASE_NOTIFY_TYPE.

**/
EFI_STATUS
EFIAPI
FormUpdateNotify (
  IN UINT8                              PackageType,
  IN CONST EFI_GUID                     *PackageGuid,
  IN CONST EFI_HII_PACKAGE_HEADER       *Package,
  IN EFI_HII_HANDLE                     Handle,
  IN EFI_HII_DATABASE_NOTIFY_TYPE       NotifyType
  )
{
  mHiiPackageListUpdated = TRUE;

  if (gCurrentSelection != NULL && gCurrentSelection->Handle == Handle) {
    mRefreshFormSet = TRUE;
  }

  return EFI_SUCCESS;
}

/**
  Update the NV flag info for this form set.

  @param  FormSet                FormSet data structure.

**/
BOOLEAN
IsNvUpdateRequiredForFormSet (
  IN FORM_BROWSER_FORMSET  *FormSet
  )
{
  LIST_ENTRY              *Link;
  FORM_BROWSER_FORM       *Form;
  BOOLEAN                 RetVal;

  //
  // Not finished question initialization, return FALSE.
  //
  if (!FormSet->QuestionInited) {
    return FALSE;
  }

  RetVal = FALSE;

  Link = GetFirstNode (&FormSet->FormListHead);
  while (!IsNull (&FormSet->FormListHead, Link)) {
    Form = FORM_BROWSER_FORM_FROM_LINK (Link);

    RetVal = IsNvUpdateRequiredForForm(Form);
    if (RetVal) {
      break;
    }

    Link = GetNextNode (&FormSet->FormListHead, Link);
  }

  return RetVal;
}

/**
  Update the NvUpdateRequired flag for a form.

  @param  Form                Form data structure.

**/
BOOLEAN
IsNvUpdateRequiredForForm (
  IN FORM_BROWSER_FORM    *Form
  )
{
  LIST_ENTRY              *Link;
  FORM_BROWSER_STATEMENT  *Statement;

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Statement = FORM_BROWSER_STATEMENT_FROM_LINK (Link);

    if (Statement->ValueChanged) {
      return TRUE;
    }

    Link = GetNextNode (&Form->StatementListHead, Link);
  }

  return FALSE;
}

/**
  Find menu which will show next time.

  @param Selection       On input, Selection tell setup browser the information
                         about the Selection, form and formset to be displayed.
                         On output, Selection return the screen item that is selected
                         by user.
  @param SettingLevel    Input Settting level, if it is FormLevel, just exit current form.
                         else, we need to exit current formset.

  @retval TRUE           Exit current form.
  @retval FALSE          User press ESC and keep in current form.
**/
BOOLEAN
FindNextMenu (
  IN OUT UI_MENU_SELECTION        *Selection,
  IN       BROWSER_SETTING_SCOPE  SettingLevel
  )
{
  FORM_ENTRY_INFO            *CurrentMenu;
  FORM_ENTRY_INFO            *ParentMenu;
  BROWSER_SETTING_SCOPE      Scope;

  CurrentMenu = Selection->CurrentMenu;
  ParentMenu  = NULL;
  Scope       = FormSetLevel;

  if (CurrentMenu != NULL && (ParentMenu = UiFindParentMenu(CurrentMenu)) != NULL) {
    //
    // we have a parent, so go to the parent menu
    //
    if (CompareGuid (&CurrentMenu->FormSetGuid, &ParentMenu->FormSetGuid)) {
      if (SettingLevel == FormSetLevel) {
        //
        // Find a menu which has different formset guid with current.
        //
        while (CompareGuid (&CurrentMenu->FormSetGuid, &ParentMenu->FormSetGuid)) {
          CurrentMenu = ParentMenu;
          if ((ParentMenu = UiFindParentMenu(CurrentMenu)) == NULL) {
            break;
          }
        }

        if (ParentMenu != NULL) {
          Scope = FormSetLevel;
        }
      } else {
        Scope = FormLevel;
      }
    } else {
      Scope = FormSetLevel;
    }
  }

  //
  // Prompt confirm dialog when user press ESC key for SCU / SecureBootMgr which don't care NvUpdateRequired flag
  //
  if ((gUserInput->Action == BROWSER_ACTION_FORM_EXIT) &&
      (gBrowserSettingScope == FormSetLevel || gBrowserSettingScope == SystemLevel) &&
      (Scope == FormSetLevel) &&
      ((ParentMenu == NULL && (GetScuFormset () != NULL || IsBrowserDataModified ())) ||
       CompareGuid (Selection->FormSet->ClassGuid, &mSecureBootMgrFormSetGuid))) {
    if (!ProcessChangedData(Selection, gBrowserSettingScope)) {
      return FALSE;
    }
  }

  //
  // Form Level Check whether the data is changed.
  //
  if ((gBrowserSettingScope == FormLevel && IsNvUpdateRequiredForForm (Selection->Form)) ||
      (gBrowserSettingScope == FormSetLevel && IsNvUpdateRequiredForFormSet(Selection->FormSet) && Scope == FormSetLevel)) {
    if (!ProcessChangedData(Selection, Scope)) {
      return FALSE;
    }
  }

  if (ParentMenu != NULL) {
    //
    // ParentMenu is found. Then, go to it.
    //
    if (Scope == FormLevel) {
      Selection->Action = UI_ACTION_REFRESH_FORM;
    } else {
      Selection->Action = UI_ACTION_REFRESH_FORMSET;
      CopyMem (&Selection->FormSetGuid, &ParentMenu->FormSetGuid, sizeof (EFI_GUID));
      Selection->Handle = ParentMenu->HiiHandle;
    }

    Selection->Statement = NULL;

    Selection->FormId = ParentMenu->FormId;
    Selection->QuestionId = ParentMenu->QuestionId;

    //
    // Clear highlight record for this menu
    //
    CurrentMenu->QuestionId = 0;
    return FALSE;
  }

  //
  // Current in root page, exit the SendForm
  //
  Selection->Action = UI_ACTION_EXIT;

  return TRUE;
}

/**
  Call the call back function for the single question and process the return action.
  This function is called by the forms browser in response to a user action on a question which has the
  EFI_IFR_FLAG_CALLBACK  bit set in the EFI_IFR_QUESTION_HEADER. The user action is
  specified by Action. Depending on the action, the browser may also pass the question value using
  Type and Value.  Upon return, the callback function may specify the desired browser action.
  Callback functions should return EFI_UNSUPPORTED for all values of Action that they do not
  support.

  @param [in,out]Selection          On input, Selection tell setup browser the information
                                    about the Selection, Form and FormSet to be displayed.
                                    On output, Selection return the screen item that is selected
                                    by user.
  @param FormSet                    The formset this question belong to.
  @param Form                       The form this question belong to.
  @param [in] Question              The Question which need to call.
  @param [in] Action                The action request.
  @param SkipSaveOrDiscard     Whether skip save or discard action.

  @retval EFI_SUCCESS               The call back function excutes successfully.
  @return Other value if the call back function failed to excute.
**/
EFI_STATUS
ProcessCallBackFunction (
  IN OUT UI_MENU_SELECTION               *Selection,
  IN     FORM_BROWSER_FORMSET            *FormSet,
  IN     FORM_BROWSER_FORM               *Form,
  IN     FORM_BROWSER_STATEMENT          *Question,
  IN     EFI_BROWSER_ACTION              Action,
  IN     BOOLEAN                         SkipSaveOrDiscard
  )
{
  EFI_STATUS                      Status;
  EFI_BROWSER_ACTION_REQUEST      ActionRequest;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;
  EFI_HII_VALUE                   *HiiValue;
  EFI_IFR_TYPE_VALUE              *TypeValue;
  FORM_BROWSER_STATEMENT          *Statement;
  BOOLEAN                         SubmitFormIsRequired;
  BOOLEAN                         DiscardFormIsRequired;
  BOOLEAN                         NeedExit;
  LIST_ENTRY                      *Link;
  BROWSER_SETTING_SCOPE           SettingLevel;
  EFI_IFR_TYPE_VALUE              BackUpValue;
  UINT8                           *BackUpBuffer;
  CHAR16                          *NewString;
  EFI_QUESTION_ID                 QuestionId;
  EFI_HII_HANDLE                  OrgHiiHandle;

  ConfigAccess = FormSet->ConfigAccess;
  SubmitFormIsRequired  = FALSE;
  SettingLevel          = (GetScuFormset () != NULL) ? gBrowserSettingScope : FormSetLevel;
  DiscardFormIsRequired = FALSE;
  NeedExit              = FALSE;
  Status                = EFI_SUCCESS;
  ActionRequest         = EFI_BROWSER_ACTION_REQUEST_NONE;
  BackUpBuffer          = NULL;

  if (ConfigAccess == NULL) {
    return EFI_SUCCESS;
  }

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Statement = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
    Link = GetNextNode (&Form->StatementListHead, Link);

    //
    // if Question != NULL, only process the question. Else, process all question in this form.
    //
    if ((Question != NULL) && (Statement != Question)) {
      continue;
    }

    if ((Statement->QuestionFlags & EFI_IFR_FLAG_CALLBACK) != EFI_IFR_FLAG_CALLBACK) {
      continue;
    }

    //
    // Check whether Statement is disabled.
    //
    if (Statement->Expression != NULL) {
      if (EvaluateExpressionList(Statement->Expression, TRUE, FormSet, Form) == ExpressDisable) {
        continue;
      }
    }

    HiiValue = &Statement->HiiValue;
    TypeValue = &HiiValue->Value;
    if (HiiValue->Type == EFI_IFR_TYPE_BUFFER) {
      //
      // For OrderedList, passing in the value buffer to Callback()
      //
      TypeValue = (EFI_IFR_TYPE_VALUE *) Statement->BufferValue;
    }

    //
    // If EFI_BROWSER_ACTION_CHANGING type, back up the new question value.
    //
    if (Action == EFI_BROWSER_ACTION_CHANGING) {
      if (HiiValue->Type == EFI_IFR_TYPE_BUFFER) {
        BackUpBuffer = AllocateCopyPool(Statement->StorageWidth + sizeof(CHAR16), Statement->BufferValue);
      } else {
        CopyMem (&BackUpValue, &HiiValue->Value, sizeof (EFI_IFR_TYPE_VALUE));
      }
    }

    QuestionId   = Statement->QuestionId;
    OrgHiiHandle = mSystemLevelFormSet->HiiHandle;
    if (IS_IMPORT_STATEMENT (Statement)) {
      ConfigAccess = Statement->ImportInfo->SrcFormSet->ConfigAccess;
      QuestionId   = Statement->ImportInfo->SrcStatement->QuestionId;
      if (HiiValue->Type == EFI_IFR_TYPE_STRING) {
        TypeValue = &Statement->ImportInfo->SrcStatement->HiiValue.Value;
        HiiCopyStringByStringId (
          FormSet->HiiHandle,
          Statement->HiiValue.Value.string,
          Statement->ImportInfo->SrcFormSet->HiiHandle,
          &TypeValue->string
          );
      }
      mSystemLevelFormSet->HiiHandle = Statement->ImportInfo->SrcFormSet->HiiHandle;
    }

    ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
    Status = ConfigAccess->Callback (
                             ConfigAccess,
                             Action,
                             QuestionId,
                             HiiValue->Type,
                             TypeValue,
                             &ActionRequest
                             );
    mSystemLevelFormSet->HiiHandle = OrgHiiHandle;
    if (!EFI_ERROR (Status)) {
      //
      // Need to sync the value between Statement->HiiValue->Value and Statement->BufferValue
      //
      if (HiiValue->Type == EFI_IFR_TYPE_STRING) {
        if (IS_IMPORT_STATEMENT (Statement)) {
          HiiCopyStringByStringId (
            Statement->ImportInfo->SrcFormSet->HiiHandle,
            TypeValue->string,
            FormSet->HiiHandle,
            &Statement->HiiValue.Value.string
            );
        }

        NewString = GetString (Statement->HiiValue.Value.string, FormSet->HiiHandle);
        ASSERT (NewString != NULL);
        if (NewString == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }

        ASSERT (StrLen (NewString) * sizeof (CHAR16) <= Statement->StorageWidth);
        if (StrLen (NewString) * sizeof (CHAR16) <= Statement->StorageWidth) {
          CopyMem (Statement->BufferValue, NewString, StrSize (NewString));
        } else {
          CopyMem (Statement->BufferValue, NewString, Statement->StorageWidth);
        }
        FreePool (NewString);
      }

      //
      // Only for EFI_BROWSER_ACTION_CHANGED need to handle this ActionRequest.
      //
      switch (Action) {
      case EFI_BROWSER_ACTION_CHANGED:
        switch (ActionRequest) {
        case EFI_BROWSER_ACTION_REQUEST_RESET:
          DiscardFormIsRequired = TRUE;
          gResetRequired        = TRUE;
          NeedExit              = TRUE;
          break;

        case EFI_BROWSER_ACTION_REQUEST_SUBMIT:
          SubmitFormIsRequired = TRUE;
          NeedExit              = TRUE;
          break;

        case EFI_BROWSER_ACTION_REQUEST_EXIT:
          DiscardFormIsRequired = TRUE;
          NeedExit              = TRUE;
          break;

        case EFI_BROWSER_ACTION_REQUEST_FORM_SUBMIT_EXIT:
          SubmitFormIsRequired  = TRUE;
          SettingLevel          = FormLevel;
          NeedExit              = TRUE;
          break;

        case EFI_BROWSER_ACTION_REQUEST_FORM_DISCARD_EXIT:
          DiscardFormIsRequired = TRUE;
          SettingLevel          = FormLevel;
          NeedExit              = TRUE;
          break;

        case EFI_BROWSER_ACTION_REQUEST_FORM_APPLY:
          SubmitFormIsRequired  = TRUE;
          SettingLevel          = FormLevel;
          break;

        case EFI_BROWSER_ACTION_REQUEST_FORM_DISCARD:
          DiscardFormIsRequired = TRUE;
          SettingLevel          = FormLevel;
          break;

        default:
          break;
        }
        break;

      case EFI_BROWSER_ACTION_CHANGING:
        //
        // Do the question validation.
        //
        Status = ValueChangedValidation (gCurrentSelection->FormSet, gCurrentSelection->Form, Statement);
        if (!EFI_ERROR (Status)) {
          //
          // According the spec, return value from call back of "changing" and
          // "retrieve" should update to the question's temp buffer.
          //
          SetQuestionValue(FormSet, Form, Statement, GetSetValueWithEditBuffer);
        }
        break;

      case EFI_BROWSER_ACTION_RETRIEVE:
        //
        // According the spec, return value from call back of "changing" and
        // "retrieve" should update to the question's temp buffer.
        //
        SetQuestionValue(FormSet, Form, Statement, GetSetValueWithEditBuffer);
        break;

      default:
        break;
      }
    } else {
      //
      // If the callback returns EFI_UNSUPPORTED for EFI_BROWSER_ACTION_CHANGING,
      // then the browser will use the value passed to Callback() and ignore the
      // value returned by Callback().
      //
      if (Action  == EFI_BROWSER_ACTION_CHANGING && Status == EFI_UNSUPPORTED) {
        if (HiiValue->Type == EFI_IFR_TYPE_BUFFER) {
          CopyMem (Statement->BufferValue, BackUpBuffer, Statement->StorageWidth + sizeof(CHAR16));
        } else {
          CopyMem (&HiiValue->Value, &BackUpValue, sizeof (EFI_IFR_TYPE_VALUE));
        }

        //
        // Do the question validation.
        //
        Status = ValueChangedValidation (gCurrentSelection->FormSet, gCurrentSelection->Form, Statement);
        if (!EFI_ERROR (Status)) {
          SetQuestionValue(FormSet, Form, Statement, GetSetValueWithEditBuffer);
        }
      }

      //
      // According the spec, return fail from call back of "changing" and
      // "retrieve", should restore the question's value.
      //
      if ((Action == EFI_BROWSER_ACTION_CHANGING && Status != EFI_UNSUPPORTED) ||
           Action == EFI_BROWSER_ACTION_RETRIEVE) {
        GetQuestionValue(FormSet, Form, Statement, GetSetValueWithEditBuffer);
      }

      if (Status == EFI_UNSUPPORTED) {
        //
        // If return EFI_UNSUPPORTED, also consider Hii driver suceess deal with it.
        //
        Status = EFI_SUCCESS;
      }
    }

    if (BackUpBuffer != NULL) {
      FreePool (BackUpBuffer);
    }

    if ((Question != NULL) && (Statement == Question)) {
      break;
    }
  }

  if (SubmitFormIsRequired && !SkipSaveOrDiscard) {
    SubmitForm (FormSet, Form, SettingLevel);
  }

  if (DiscardFormIsRequired && !SkipSaveOrDiscard) {
    DiscardForm (FormSet, Form, SettingLevel);
  }

  if (NeedExit) {
    FindNextMenu (Selection, SettingLevel);
  }

  return Status;
}

/**
  Call the RETRIEVE type callback function for one question to get the initial question value.

  This function only used when in the initialization stage, because in this stage, the
  Selection->Form is not ready. For other case, use the ProcessCallBackFunction instead.

  @param [in] ConfigAccess     The config access protocol produced by the HII driver.
  @param [in] Statement        The Question which need to call.

  @retval EFI_SUCCESS          The call back function excutes successfully.
  @return Other value if the call back function failed to excute.
**/
EFI_STATUS
ProcessRetrieveForQuestion (
  IN     EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess,
  IN     FORM_BROWSER_STATEMENT          *Statement,
  IN     FORM_BROWSER_FORMSET            *FormSet
  )
{
  EFI_STATUS                      Status;
  EFI_BROWSER_ACTION_REQUEST      ActionRequest;
  EFI_HII_VALUE                   *HiiValue;
  EFI_IFR_TYPE_VALUE              *TypeValue;
  CHAR16                          *NewString;
  EFI_QUESTION_ID                 QuestionId;
  EFI_HII_HANDLE                  OrgHiiHandle;

  Status                = EFI_SUCCESS;
  ActionRequest         = EFI_BROWSER_ACTION_REQUEST_NONE;

  if (((Statement->QuestionFlags & EFI_IFR_FLAG_CALLBACK) != EFI_IFR_FLAG_CALLBACK) || ConfigAccess == NULL) {
    return EFI_UNSUPPORTED;
  }

  HiiValue  = &Statement->HiiValue;
  TypeValue = &HiiValue->Value;
  if (HiiValue->Type == EFI_IFR_TYPE_BUFFER) {
    //
    // For OrderedList, passing in the value buffer to Callback()
    //
    TypeValue = (EFI_IFR_TYPE_VALUE *) Statement->BufferValue;
  }

  QuestionId   = Statement->QuestionId;
  OrgHiiHandle = mSystemLevelFormSet->HiiHandle;
  if (IS_IMPORT_STATEMENT (Statement)) {
    ConfigAccess = Statement->ImportInfo->SrcFormSet->ConfigAccess;
    QuestionId   = Statement->ImportInfo->SrcStatement->QuestionId;
    if (HiiValue->Type == EFI_IFR_TYPE_STRING) {
      TypeValue = &Statement->ImportInfo->SrcStatement->HiiValue.Value;
      HiiCopyStringByStringId (
        FormSet->HiiHandle,
        Statement->HiiValue.Value.string,
        Statement->ImportInfo->SrcFormSet->HiiHandle,
        &TypeValue->string
        );
    }
    mSystemLevelFormSet->HiiHandle = Statement->ImportInfo->SrcFormSet->HiiHandle;
  }

  ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
  Status = ConfigAccess->Callback (
                           ConfigAccess,
                           EFI_BROWSER_ACTION_RETRIEVE,
                           QuestionId,
                           HiiValue->Type,
                           TypeValue,
                           &ActionRequest
                           );
  mSystemLevelFormSet->HiiHandle = OrgHiiHandle;
  if (!EFI_ERROR (Status) && HiiValue->Type == EFI_IFR_TYPE_STRING) {
    if (IS_IMPORT_STATEMENT (Statement)) {
      HiiCopyStringByStringId (
        Statement->ImportInfo->SrcFormSet->HiiHandle,
        TypeValue->string,
        FormSet->HiiHandle,
        &Statement->HiiValue.Value.string
        );
    }

    NewString = GetString (Statement->HiiValue.Value.string, FormSet->HiiHandle);
    ASSERT (NewString != NULL);
    if (NewString == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    ASSERT (StrLen (NewString) * sizeof (CHAR16) <= Statement->StorageWidth);
    if (StrLen (NewString) * sizeof (CHAR16) <= Statement->StorageWidth) {
      CopyMem (Statement->BufferValue, NewString, StrSize (NewString));
    } else {
      CopyMem (Statement->BufferValue, NewString, Statement->StorageWidth);
    }
    FreePool (NewString);
  }

  return Status;
}

/**
  The worker function that send the displays to the screen. On output,
  the selection made by user is returned.

  @param Selection       On input, Selection tell setup browser the information
                         about the Selection, form and formset to be displayed.
                         On output, Selection return the screen item that is selected
                         by user.

  @retval EFI_SUCCESS    The page is displayed successfully.
  @return Other value if the page failed to be diplayed.

**/
EFI_STATUS
SetupBrowser (
  IN OUT UI_MENU_SELECTION    *Selection
  )
{
  EFI_STATUS                      Status;
  LIST_ENTRY                      *Link;
  EFI_HANDLE                      NotifyHandle;
  FORM_BROWSER_STATEMENT          *Statement;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;
  H2O_FORM_BROWSER_PRIVATE_DATA   *Private;
  EFI_BROWSER_ACTION_REQUEST      DummyActionRequest;

  Private = &mFBPrivate;
  ConfigAccess = Selection->FormSet->ConfigAccess;

  //
  // Register notify for Form package update
  //
  Status = gHiiDatabase->RegisterPackageNotify (
                           gHiiDatabase,
                           EFI_HII_PACKAGE_FORMS,
                           NULL,
                           FormUpdateNotify,
                           EFI_HII_DATABASE_NOTIFY_REMOVE_PACK,
                           &NotifyHandle
                           );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Initialize current settings of Questions in this FormSet
  //
  InitCurrentSetting (Selection->FormSet);

  //
  // Initilize Action field.
  //
  Selection->Action = UI_ACTION_REFRESH_FORM;

  //
  // Clean the mCurFakeQestId value is formset refreshed.
  //
  mCurFakeQestId = 0;


  Private->Repaint = TRUE;

  if (mUpdateStatementStatus) {
    mUpdateStatementStatus = FALSE;
    UpdateStatementStatus (Selection->FormSet, NULL, FormSetLevel);
  }

  do {
    //
    // IFR is updated, force to reparse the IFR binary
    //
    if (mHiiPackageListUpdated) {
      Selection->Action = UI_ACTION_REFRESH_FORMSET;
      mHiiPackageListUpdated = FALSE;
      break;
    }

    ASSERT (Selection->FormSet != NULL);
    if (Selection->FormSet == NULL) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }

    //
    // Initialize Selection->Form
    //
    if (Selection->FormId == 0) {
      //
      // Zero FormId indicates display the first Form in a FormSet
      //
      Link = GetFirstNode (&Selection->FormSet->FormListHead);

      Selection->Form = FORM_BROWSER_FORM_FROM_LINK (Link);
      Selection->FormId = Selection->Form->FormId;
    } else {
      Selection->Form = IdToForm (Selection->FormSet, Selection->FormId);
    }

    if (Selection->Form == NULL) {
      //
      // No Form to display
      //
      Status = EFI_NOT_FOUND;
      goto Done;
    }

    //
    // Check Form is suppressed.
    //
    if (Selection->Form->SuppressExpression != NULL) {
      if (EvaluateExpressionList(Selection->Form->SuppressExpression, TRUE, Selection->FormSet, Selection->Form) == ExpressSuppress) {
        //
        // Form is suppressed.
        //
        PopupErrorMessage(BROWSER_FORM_SUPPRESS, NULL, NULL);
        Status = EFI_NOT_FOUND;
        goto Done;
      }
    }

    //
    // Before display new form, invoke ConfigAccess.Callback() with EFI_BROWSER_ACTION_FORM_OPEN
    // for each question with callback flag.
    // New form may be the first form, or the different form after another form close.
    //
    if (((Selection->Handle != mCurrentHiiHandle) ||
        (!CompareGuid (&Selection->FormSetGuid, &mCurrentFormSetGuid)) ||
        (Selection->FormId != mCurrentFormId))) {
      //
      // Update Retrieve flag.
      //
      mFinishRetrieveCall = FALSE;

      //
      // Keep current form information
      //
      mCurrentHiiHandle   = Selection->Handle;
      CopyGuid (&mCurrentFormSetGuid, &Selection->FormSetGuid);
      mCurrentFormId      = Selection->FormId;

      if (ConfigAccess != NULL) {
        Status = ProcessCallBackFunction (Selection, Selection->FormSet, Selection->Form, NULL, EFI_BROWSER_ACTION_FORM_OPEN, FALSE);
        if (EFI_ERROR (Status)) {
          goto Done;
        }

        //
        // retrieve SetupUtility local data
        // Future, we will remove it, and use InitCurrentSetting -> LoadStorage -> ExtractConfig
        //
        ConfigAccess->Callback(ConfigAccess, EFI_BROWSER_ACTION_FORM_OPEN, 0, 0, NULL, &DummyActionRequest);

        //
        // IFR is updated during callback of open form, force to reparse the IFR binary
        //
        if (mHiiPackageListUpdated) {
          Selection->Action = UI_ACTION_REFRESH_FORMSET;
          mHiiPackageListUpdated = FALSE;
          break;
        }
      }
    }

    //
    // Load Questions' Value for display
    //
    Status = LoadFormSetConfig (Selection, Selection->FormSet);
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    if (!mFinishRetrieveCall) {
      //
      // Finish call RETRIEVE callback for this form.
      //
      mFinishRetrieveCall = TRUE;

      if (ConfigAccess != NULL) {
        Status = ProcessCallBackFunction (Selection, Selection->FormSet, Selection->Form, NULL, EFI_BROWSER_ACTION_RETRIEVE, FALSE);
        if (EFI_ERROR (Status)) {
          goto Done;
        }

        //
        // IFR is updated during callback of open form, force to reparse the IFR binary
        //
        if (mHiiPackageListUpdated) {
          Selection->Action = UI_ACTION_REFRESH_FORMSET;
          mHiiPackageListUpdated = FALSE;
          break;
        }
      }
    }

    //
    // Display form
    //
    Status = DisplayForm ();
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    //
    // Check Selected Statement (if press ESC, Selection->Statement will be NULL)
    //
    Statement = Selection->Statement;
    if (Statement != NULL) {
      if ((ConfigAccess != NULL) &&
          ((Statement->QuestionFlags & EFI_IFR_FLAG_CALLBACK) == EFI_IFR_FLAG_CALLBACK) &&
          (Statement->Operand != EFI_IFR_PASSWORD_OP)) {
        Status = ProcessCallBackFunction(Selection, Selection->FormSet, Selection->Form, Statement, EFI_BROWSER_ACTION_CHANGING, FALSE);
        if (Statement->Operand == EFI_IFR_REF_OP) {
          //
          // Process dynamic update ref opcode.
          //
          if (!EFI_ERROR (Status)) {
            Status = ProcessGotoOpCode(Statement, Selection);
          }

          //
          // Callback return error status or status return from process goto opcode.
          //
          if (EFI_ERROR (Status)) {
            //
            // Cross reference will not be taken
            //
            Selection->FormId = Selection->Form->FormId;
            Selection->QuestionId = 0;
          }
        }

        //
        // Verify whether question value has checked, update the ValueChanged flag in Question.
        //
        IsQuestionValueChanged(gCurrentSelection->FormSet, gCurrentSelection->Form, Statement, GetSetValueWithBuffer);

        if (!EFI_ERROR (Status) && Statement->Operand != EFI_IFR_REF_OP) {
          ProcessCallBackFunction(Selection, Selection->FormSet, Selection->Form, Statement, EFI_BROWSER_ACTION_CHANGED, FALSE);
        }
      } else {
        //
        // Do the question validation.
        //
        Status = ValueChangedValidation (gCurrentSelection->FormSet, gCurrentSelection->Form, Statement);
        if (!EFI_ERROR (Status) && (Statement->Operand != EFI_IFR_PASSWORD_OP)) {
          SetQuestionValue (gCurrentSelection->FormSet, gCurrentSelection->Form, Statement, GetSetValueWithEditBuffer);
          //
          // Verify whether question value has checked, update the ValueChanged flag in Question.
          //
          IsQuestionValueChanged(gCurrentSelection->FormSet, gCurrentSelection->Form, Statement, GetSetValueWithBuffer);
        }
      }

      //
      // If question has EFI_IFR_FLAG_RESET_REQUIRED flag and without storage and process question success till here,
      // trig the gResetFlag.
      //
      if ((Status == EFI_SUCCESS) &&
          (Statement->Storage == NULL) &&
          ((Statement->QuestionFlags & EFI_IFR_FLAG_RESET_REQUIRED) != 0)) {
        gResetRequired = TRUE;
      }
    }

    //
    // Check whether Exit flag is TRUE.
    //
    if (gExitRequired) {
      switch (gBrowserSettingScope) {
      case SystemLevel:
        Selection->Action = UI_ACTION_EXIT;
        break;

      case FormSetLevel:
      case FormLevel:
        FindNextMenu (Selection, gBrowserSettingScope);
        break;

      default:
        break;
      }

      gExitRequired = FALSE;
    }

    //
    // restore changed SetupUtility setup data;
    // Future we will use EFI_BROWSER_ACTION_FORM_CLOSE to save local SCU data
    //
    if (Selection->FormSet != NULL) {
      if (ConfigAccess != NULL) {
        ConfigAccess->Callback(ConfigAccess, EFI_BROWSER_ACTION_FORM_CLOSE, 0, 0, NULL, &DummyActionRequest);
      }
    }

    //
    // Before exit the form, invoke ConfigAccess.Callback() with EFI_BROWSER_ACTION_FORM_CLOSE
    // for each question with callback flag.
    //
    if ((ConfigAccess != NULL) &&
        ((Selection->Action == UI_ACTION_EXIT) ||
         (Selection->Handle != mCurrentHiiHandle) ||
         (!CompareGuid (&Selection->FormSetGuid, &mCurrentFormSetGuid)) ||
         (Selection->FormId != mCurrentFormId))) {

      Status = ProcessCallBackFunction (Selection, Selection->FormSet, Selection->Form, NULL, EFI_BROWSER_ACTION_FORM_CLOSE, FALSE);
      if (EFI_ERROR (Status)) {
        goto Done;
      }
    }
  } while (Selection->Action == UI_ACTION_REFRESH_FORM);

Done:
  //
  // Reset current form information to the initial setting when error happens or form exit.
  //
  if (EFI_ERROR (Status) || Selection->Action == UI_ACTION_EXIT) {
    mCurrentHiiHandle = NULL;
    CopyGuid (&mCurrentFormSetGuid, &gZeroGuid);
    mCurrentFormId = 0;
  }

  //
  // Unregister notify for Form package update
  //
  gHiiDatabase->UnregisterPackageNotify (
                   gHiiDatabase,
                   NotifyHandle
                   );
  return Status;
}

/** @file
Function which process FormSet and Form.

;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

Copyright (c) 2007 - 2012, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "InternalH2OFormBrowser.h"

EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL *mPathFromText;
BOOLEAN                            mMultiFormSet = FALSE;
extern H2O_FORM_BROWSER_PRIVATE_DATA mFBPrivate;


UINTN           gBrowserContextCount = 0;
LIST_ENTRY      gBrowserContextList = INITIALIZE_LIST_HEAD_VARIABLE (gBrowserContextList);
LIST_ENTRY      gBrowserFormSetList = INITIALIZE_LIST_HEAD_VARIABLE (gBrowserFormSetList);
LIST_ENTRY      gBrowserHotKeyList  = INITIALIZE_LIST_HEAD_VARIABLE (gBrowserHotKeyList);
LIST_ENTRY      gBrowserStorageList  = INITIALIZE_LIST_HEAD_VARIABLE (gBrowserStorageList);


UINT32                mClassOfVfr;           //Judge FormSet what class belong to(front page or others)
UINT32                mFunctionKeySetting;
BOOLEAN               gResetRequired;
BOOLEAN               gExitRequired;
EFI_HII_HANDLE        mHiiHandle;
EFI_SCREEN_DESCRIPTOR mScreenDimensions;
BROWSER_SETTING_SCOPE gBrowserSettingScope = FormSetLevel;
BOOLEAN               mBrowserScopeFirstSet = TRUE;
EXIT_HANDLER          ExitHandlerFunction = NULL;
FORM_BROWSER_FORMSET  *mSystemLevelFormSet;

//
// Browser Global Strings
//

CHAR16            *gPressEnter;
CHAR16            *gEmptyString;
CHAR16            *gAreYouSure;
CHAR16            *gYesResponse;
CHAR16            *gNoResponse;
CHAR16            *gSaveChanges;
CHAR16            *gSpaceString;
CHAR16            *gSaveFailed;
CHAR16            *gFormNotFound;
CHAR16            *gFormSuppress;
CHAR16            *gProtocolNotFound;
CHAR16            *gBrwoserError;

CHAR16            *mUnknownString = L"!";

EFI_GUID  gZeroGuid = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};

extern EFI_GUID        mScuFormSetGuid;
extern EFI_GUID        mCurrentFormSetGuid;
extern EFI_HII_HANDLE  mCurrentHiiHandle;
extern UINT16          mCurrentFormId;
extern FORM_DISPLAY_ENGINE_FORM gDisplayFormData;

/**
  Create a menu with specified formset GUID and form ID, and add it as a child
  of the given parent menu.

  @param  HiiHandle              Hii handle related to this formset.
  @param  FormSetGuid            The Formset Guid of menu to be added.
  @param  FormId                 The Form ID of menu to be added.
  @param  QuestionId             The question id of this menu to be added.

  @return A pointer to the newly added menu or NULL if memory is insufficient.

**/
FORM_ENTRY_INFO *
UiAddMenuList (
  IN EFI_HII_HANDLE       HiiHandle,
  IN EFI_GUID             *FormSetGuid,
  IN UINT16               FormId,
  IN UINT16               QuestionId
  )
{
  FORM_ENTRY_INFO  *MenuList;

  MenuList = AllocateZeroPool (sizeof (FORM_ENTRY_INFO));
  if (MenuList == NULL) {
    return NULL;
  }

  MenuList->Signature = FORM_ENTRY_INFO_SIGNATURE;

  MenuList->HiiHandle  = HiiHandle;
  CopyMem (&MenuList->FormSetGuid, FormSetGuid, sizeof (EFI_GUID));
  MenuList->FormId     = FormId;
  MenuList->QuestionId = QuestionId;

  //
  // If parent is not specified, it is the root Form of a Formset
  //
  InsertTailList (&mFBPrivate.FormBrowserEx2.FormViewHistoryHead, &MenuList->Link);

  return MenuList;
}

/**
  Return the form id for the input hiihandle and formset.

  @param  HiiHandle              HiiHandle for FormSet.
  @param  FormSetGuid            The Formset GUID of the menu to search.

  @return First form's id for this form set.

**/
EFI_FORM_ID
GetFirstFormId (
  IN EFI_HII_HANDLE       HiiHandle,
  IN EFI_GUID             *FormSetGuid
  )
{
  LIST_ENTRY         *Link;
  FORM_BROWSER_FORM  *Form;

  Link = GetFirstNode (&gCurrentSelection->FormSet->FormListHead);
  Form = FORM_BROWSER_FORM_FROM_LINK (Link);

  return Form->FormId;
}

/**
  Search Menu with given FormSetGuid and FormId in all cached menu list.

  @param  HiiHandle              HiiHandle for FormSet.
  @param  FormSetGuid            The Formset GUID of the menu to search.
  @param  FormId                 The Form ID of menu to search.

  @return A pointer to menu found or NULL if not found.

**/
FORM_ENTRY_INFO *
UiFindMenuList (
  IN EFI_HII_HANDLE       HiiHandle,
  IN EFI_GUID             *FormSetGuid,
  IN UINT16               FormId
  )
{
  LIST_ENTRY         *Link;
  FORM_ENTRY_INFO    *MenuList;
  FORM_ENTRY_INFO    *RetMenu;
  EFI_FORM_ID        FirstFormId;

  RetMenu = NULL;

  Link = GetFirstNode (&mFBPrivate.FormBrowserEx2.FormViewHistoryHead);
  while (!IsNull (&mFBPrivate.FormBrowserEx2.FormViewHistoryHead, Link)) {
    MenuList = FORM_ENTRY_INFO_FROM_LINK (Link);
    Link = GetNextNode (&mFBPrivate.FormBrowserEx2.FormViewHistoryHead, Link);

    //
    // If already find the menu, free the menus behind it.
    //
    if (RetMenu != NULL) {
      RemoveEntryList (&MenuList->Link);
      FreePool (MenuList);
      continue;
    }

    //
    // Find the same FromSet.
    //
    if (MenuList->HiiHandle == HiiHandle) {
      if (CompareGuid (&MenuList->FormSetGuid, &gZeroGuid)) {
        //
        // FormSetGuid is not specified.
        //
        RetMenu = MenuList;
      } else if (CompareGuid (&MenuList->FormSetGuid, FormSetGuid)) {
        if (MenuList->FormId == FormId) {
          RetMenu = MenuList;
        } else if (FormId == 0 || MenuList->FormId == 0 ) {
          FirstFormId = GetFirstFormId (HiiHandle, FormSetGuid);
          if ((FormId == 0 && FirstFormId == MenuList->FormId) || (MenuList->FormId ==0 && FirstFormId == FormId)) {
            RetMenu = MenuList;
          }
        }
      }
    }
  }

  return RetMenu;
}

/**
  Find parent menu for current menu.

  @param  CurrentMenu    Current Menu

  @retval   The parent menu for current menu.
**/
FORM_ENTRY_INFO *
UiFindParentMenu (
  IN FORM_ENTRY_INFO  *CurrentMenu
  )
{
  FORM_ENTRY_INFO    *ParentMenu;

  ParentMenu = NULL;
  if (CurrentMenu->Link.BackLink != &mFBPrivate.FormBrowserEx2.FormViewHistoryHead) {
    ParentMenu = FORM_ENTRY_INFO_FROM_LINK (CurrentMenu->Link.BackLink);
  }

  return ParentMenu;
}

/**
  Free Menu list linked list.

  @param  MenuListHead    One Menu list point in the menu list.

**/
VOID
UiFreeMenuList (
  LIST_ENTRY   *MenuListHead
  )
{
  FORM_ENTRY_INFO    *MenuList;

  while (!IsListEmpty (MenuListHead)) {
    MenuList = FORM_ENTRY_INFO_FROM_LINK (MenuListHead->ForwardLink);
    RemoveEntryList (&MenuList->Link);

    FreePool (MenuList);
  }
}

/**
  Load all hii formset to the browser.

**/
VOID
LoadAllHiiFormset (
  VOID
  )
{
  FORM_BROWSER_FORMSET    *LocalFormSet;
  EFI_HII_HANDLE          *HiiHandles;
  UINTN                   Index;
  EFI_GUID                ZeroGuid;
  EFI_STATUS              Status;
  FORM_BROWSER_FORMSET    *OldFormset;

  OldFormset = mSystemLevelFormSet;

  //
  // Get all the Hii handles
  //
  HiiHandles = HiiGetHiiHandles (NULL);
  ASSERT (HiiHandles != NULL);
  if (HiiHandles == NULL) {
    return;
  }

  //
  // Search for formset of each class type
  //
  for (Index = 0; HiiHandles[Index] != NULL; Index++) {
    //
    // Check HiiHandles[Index] does exist in global maintain list.
    //
    if (GetFormSetFromHiiHandle (HiiHandles[Index]) != NULL) {
      continue;
    }

    //
    // Initilize FormSet Setting
    //
    LocalFormSet = AllocateZeroPool (sizeof (FORM_BROWSER_FORMSET));
    ASSERT (LocalFormSet != NULL);
    if (LocalFormSet == NULL) {
      return;
    }
    mSystemLevelFormSet = LocalFormSet;

    ZeroMem (&ZeroGuid, sizeof (ZeroGuid));
    Status = InitializeFormSet (HiiHandles[Index], &ZeroGuid, LocalFormSet);
    if (EFI_ERROR (Status) || IsListEmpty (&LocalFormSet->FormListHead)) {
      DestroyFormSet (LocalFormSet);
      continue;
    }
    InitCurrentSetting (LocalFormSet);

    //
    // Initilize Questions' Value
    //
    Status = LoadFormSetConfig (NULL, LocalFormSet);
    if (EFI_ERROR (Status)) {
      DestroyFormSet (LocalFormSet);
      continue;
    }
  }

  //
  // Free resources, and restore gOldFormSet and gClassOfVfr
  //
  FreePool (HiiHandles);

  mSystemLevelFormSet = OldFormset;
}

/**
  Pop up the error info.

  @param      BrowserStatus    The input browser status.
  @param      OpCode           The opcode use to get the erro info and timeout value.
  @param      ErrorString      Error string used by BROWSER_NO_SUBMIT_IF.

**/
VOID
PopupErrorMessage (
  IN UINT32                BrowserStatus,
  IN EFI_IFR_OP_HEADER     *OpCode, OPTIONAL
  IN CHAR16                *ErrorString
  )
{
  FORM_DISPLAY_ENGINE_STATEMENT *Statement;

  Statement = NULL;

  if (OpCode != NULL) {
    Statement = AllocateZeroPool (sizeof(FORM_DISPLAY_ENGINE_STATEMENT));
    ASSERT (Statement != NULL);
    if (Statement == NULL) {
      return;
    }
    Statement->OpCode = OpCode;
    gDisplayFormData.HighLightedStatement = Statement;
  }

  //
  // Used to compatible with old display engine.
  // New display engine not use this field.
  //
  gDisplayFormData.ErrorString   = ErrorString;
  gDisplayFormData.BrowserStatus = BrowserStatus;

  FormDisplay (&gDisplayFormData, NULL);

  gDisplayFormData.BrowserStatus = BROWSER_SUCCESS;
  gDisplayFormData.ErrorString   = NULL;

  if (OpCode != NULL) {
    FreePool (Statement);
  }
}

/**
  This is the routine which an external caller uses to direct the browser
  where to obtain it's information.


  @param[in]  This                The Form Browser protocol instanse.
  @param[in]  Handles             A pointer to an array of Handles.  If HandleCount > 1 we
                                  display a list of the FormSets for the handles specified.
  @param[in]  HandleCount         The number of Handles specified in Handle.
  @param[in]  FormSetGuid         This field points to the EFI_GUID which must match the Guid
                                  field in the EFI_IFR_FORM_SET op-code for the specified
                                  forms-based package. If FormSetGuid is NULL, then this
                                  function will display the first found forms package.
  @param[in]  FormId              This field specifies which EFI_IFR_FORM to render as the first
                                  displayable page. If this field has a value of 0x0000, then
                                  the forms browser will render the specified forms in their encoded order.
  @param[in]  ScreenDimensions    Points to recommended Form dimensions, including any non-content area, in characters.
  @param[out] ActionRequest       Points to the action recommended by the Form.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_OUT_OF_RESOURCES    ConsoleDevList is NULL
  @retval  EFI_INVALID_PARAMETER  One of the parameters has an invalid value.
  @retval  EFI_NOT_FOUND          No valid forms could be found to display.
**/
EFI_STATUS
EFIAPI
SendForm (
  IN  CONST EFI_FORM_BROWSER2_PROTOCOL *This,
  IN  EFI_HII_HANDLE                   *Handles,
  IN  UINTN                            HandleCount,
  IN  EFI_GUID                         *FormSetGuid OPTIONAL,
  IN  UINT16                           FormId OPTIONAL,
  IN  CONST EFI_SCREEN_DESCRIPTOR      *ScreenDimensions OPTIONAL,
  OUT EFI_BROWSER_ACTION_REQUEST       *ActionRequest  OPTIONAL
  )
{
  EFI_STATUS                           Status;
  H2O_FORM_BROWSER_PRIVATE_DATA        *Private;
  UI_MENU_SELECTION                    *Selection;
  FORM_BROWSER_FORMSET                 *FormSet;
  FORM_ENTRY_INFO                      *MenuList;
  BROWSER_SETTING_SCOPE                OrgBrowserSettingScope;
  UINT32                               HiiHandleIndex;
  HII_HANDLE_ENTRY                     *HiiHandleEntry;


  Private = FORM_BROWSER2_DATA_FROM_PROTOCOL (This);

  Status = FBInitConsoles (Private);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // BUGBUG: Need move to refresh formset, avoid change language incorrect.
  //
  InitializeBrowserStrings ();

  //
  // Save globals used by SendForm()
  //
  SaveBrowserContext ();

  //
  // process handle list
  //
  Private->HiiHandleCount = (UINT32) HandleCount;
  Private->HiiHandleList  = AllocateCopyPool (Private->HiiHandleCount * sizeof (EFI_HII_HANDLE), Handles);
  if (Private->HiiHandleList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  OrgBrowserSettingScope = gBrowserSettingScope;

  InitializeListHead (&mFBPrivate.NeedAddHiiHandleList);
  for (HiiHandleIndex = 0; HiiHandleIndex < HandleCount; HiiHandleIndex ++) {
    InsertNeedAddHiiHandle (Handles[HiiHandleIndex]);
  }

  while (!IsListEmpty (&mFBPrivate.NeedAddHiiHandleList)) {
    HiiHandleEntry = (HII_HANDLE_ENTRY *)GetFirstNode (&mFBPrivate.NeedAddHiiHandleList);
    LoadHiiFormset (&HiiHandleEntry->HiiHandle, 1);
    RemoveEntryList (&HiiHandleEntry->Link);
    FreePool (HiiHandleEntry);
  }

  if (Private->HiiHandleCount >= 2) {
    mMultiFormSet        = TRUE;
    gBrowserSettingScope = SystemLevel;
  }

  gResetRequired = FALSE;
  gExitRequired  = FALSE;
  Status         = EFI_SUCCESS;

  FBInitSMInfo (Private, Handles, HandleCount);
  Private->HiiHandleIndex = Private->SetupMenuInfo[0].PageId >> 16;

  {
    Selection = AllocateZeroPool (sizeof (UI_MENU_SELECTION));
    ASSERT (Selection != NULL);
    if (Selection == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    Selection->Handle = Handles[Private->HiiHandleIndex];
    if (FormSetGuid != NULL) {
      CopyMem (&Selection->FormSetGuid, FormSetGuid, sizeof (EFI_GUID));
      Selection->FormId = FormId;
    } else {
      CopyMem (&Selection->FormSetGuid, &gEfiHiiPlatformSetupFormsetGuid, sizeof (EFI_GUID));
    }

    do {
      FormSet = AllocateZeroPool (sizeof (FORM_BROWSER_FORMSET));
      ASSERT (FormSet != NULL);
      if (FormSet == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      //
      // Initialize internal data structures of FormSet
      //
      Status = InitializeFormSet (Selection->Handle, &Selection->FormSetGuid, FormSet);
      if (EFI_ERROR (Status) || IsListEmpty (&FormSet->FormListHead)) {
        DestroyFormSet (FormSet);
        break;
      }
      FBProcessVfcfImport (FormSet);
      FBProcessVfcfProperty (FormSet);

      Selection->FormSet = FormSet;
      mSystemLevelFormSet = FormSet;

      //
      // Display this formset
      //
      gCurrentSelection = Selection;

      Status = SetupBrowser (Selection);
      if (mMultiFormSet && Selection->Action == UI_ACTION_REFRESH_FORMSET) {
        SyncCurrentStorageToAllHiiHandles ();
      }
      gCurrentSelection = NULL;
      mSystemLevelFormSet = NULL;

      //
      // If no data is changed, don't need to save current FormSet into the maintain list.
      //
    if (!IsNvUpdateRequiredForFormSet (FormSet) && !mMultiFormSet) {
        CleanBrowserStorage(FormSet);
        RemoveEntryList (&FormSet->Link);
        DestroyFormSet (FormSet);
      }

      if (EFI_ERROR (Status)) {
        break;
      }
    } while (Selection->Action == UI_ACTION_REFRESH_FORMSET);

    FreePool (Selection);
  }

  if (ActionRequest != NULL) {
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
    if (gResetRequired) {
      *ActionRequest = EFI_BROWSER_ACTION_REQUEST_RESET;
    }
  }

  if (mMultiFormSet) {
    DestroyLoadHiiFormset ();
    gBrowserSettingScope = OrgBrowserSettingScope;
    mMultiFormSet        = FALSE;
  }

  FBDetachConsoles (Private);

  DestroySetupMenuInfo (Private);
  DestroyFormSetList (Private);
  DestroyPageInfo (Private);
  DestroyEventQueue (Private);
  DestroyImportFormSet ();
  DeleteInvalidBrowserStorage ();

  Private->FB.CurrentP = NULL;
  Private->FB.CurrentQ = NULL;
  FreePool (Private->HiiHandleList);

  FreeBrowserStrings ();
  //
  // Clear the menu history data.
  //
  while (!IsListEmpty (&mFBPrivate.FormBrowserEx2.FormViewHistoryHead)) {
    MenuList = FORM_ENTRY_INFO_FROM_LINK (mFBPrivate.FormBrowserEx2.FormViewHistoryHead.ForwardLink);
    RemoveEntryList (&MenuList->Link);
    FreePool (MenuList);
  }

  //
  // Restore globals used by SendForm()
  //
  RestoreBrowserContext ();

  return Status;
}

/**
  Get or set data to the storage.

  @param  ResultsDataSize        The size of the buffer associatedwith ResultsData.
  @param  ResultsData            A string returned from an IFR browser or
                                 equivalent. The results string will have no
                                 routing information in them.
  @param  RetrieveData           A BOOLEAN field which allows an agent to retrieve
                                 (if RetrieveData = TRUE) data from the uncommitted
                                 browser state information or set (if RetrieveData
                                 = FALSE) data in the uncommitted browser state
                                 information.
  @param  Storage                The pointer to the storage.

  @retval EFI_SUCCESS            The results have been distributed or are awaiting
                                 distribution.

**/
EFI_STATUS
ProcessStorage (
  IN OUT UINTN                         *ResultsDataSize,
  IN OUT EFI_STRING                    *ResultsData,
  IN BOOLEAN                           RetrieveData,
  IN BROWSER_STORAGE                   *Storage
  )
{
  CHAR16                *ConfigResp;
  EFI_STATUS            Status;
  CHAR16                *StrPtr;
  UINTN                 BufferSize;
  UINTN                 TmpSize;

  if (RetrieveData) {
    //
    // Skip if there is no RequestElement
    //
    if (Storage->ConfigRequest == NULL) {
      return EFI_SUCCESS;
    }

    //
    // Generate <ConfigResp>
    //
    Status = StorageToConfigResp (Storage, &ConfigResp, Storage->ConfigRequest, TRUE);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Skip <ConfigHdr> and '&' to point to <ConfigBody> when first copy the configbody.
    // Also need to consider add "\0" at first time.
    //
    StrPtr     = ConfigResp + StrLen (Storage->ConfigHdr) + 1;
    BufferSize = StrSize (StrPtr);


    //
    // Copy the data if the input buffer is bigger enough.
    //
    if (*ResultsDataSize >= BufferSize) {
      StrCpy (*ResultsData, StrPtr);
    }

    *ResultsDataSize = BufferSize;
    FreePool (ConfigResp);
  } else {
    //
    // Prepare <ConfigResp>
    //
    TmpSize = StrLen (*ResultsData);
    BufferSize = (TmpSize + StrLen (Storage->ConfigHdr) + 2) * sizeof (CHAR16);
    ConfigResp = AllocateZeroPool (BufferSize);
    ASSERT (ConfigResp != NULL);
    if (ConfigResp == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    StrCpy (ConfigResp, Storage->ConfigHdr);
    StrCat (ConfigResp, L"&");
    StrCat (ConfigResp, *ResultsData);

    //
    // Update Browser uncommited data
    //
    Status = ConfigRespToStorage (Storage, ConfigResp);
    FreePool (ConfigResp);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

/**
  This routine called this service in the browser to retrieve or set certain uncommitted
  state information that resides in the open formsets.

  @param  [in] This                 A pointer to the EFI_FORM_BROWSER2_PROTOCOL
                                    instance.
  @param  [in,out] ResultsDataSize  A pointer to the size of the buffer associated
                                    with ResultsData.
  @param  [in,out] ResultsData      A string returned from an IFR browser or
                                    equivalent. The results string will have no
                                    routing information in them.
  @param  [in] RetrieveData         A BOOLEAN field which allows an agent to retrieve
                                    (if RetrieveData = TRUE) data from the uncommitted
                                    browser state information or set (if RetrieveData
                                    = FALSE) data in the uncommitted browser state
                                    information.
  @param  [in] VariableGuid         An optional field to indicate the target variable
                                    GUID name to use.
  @param  [in] VariableName         An optional field to indicate the target
                                    human-readable variable name.

  @retval EFI_SUCCESS               The results have been distributed or are awaiting
                                    distribution.
  @retval EFI_BUFFER_TOO_SMALL      The ResultsDataSize specified was too small to
                                    contain the results data.

**/
EFI_STATUS
EFIAPI
BrowserCallback (
  IN CONST EFI_FORM_BROWSER2_PROTOCOL  *This,
  IN OUT UINTN                         *ResultsDataSize,
  IN OUT EFI_STRING                    ResultsData,
  IN BOOLEAN                           RetrieveData,
  IN CONST EFI_GUID                    *VariableGuid OPTIONAL,
  IN CONST CHAR16                      *VariableName  OPTIONAL
  )
{
  EFI_STATUS            Status;
  LIST_ENTRY            *Link;
  BROWSER_STORAGE       *Storage;
  FORMSET_STORAGE       *FormsetStorage;
  UINTN                 TotalSize;
  BOOLEAN               Found;

  if (ResultsDataSize == NULL || ResultsData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TotalSize = *ResultsDataSize;
  Storage   = NULL;
  Found     = FALSE;
  Status    = EFI_SUCCESS;

  if (VariableGuid != NULL) {
    //
    // Try to find target storage in the current formset.
    //
    Link = GetFirstNode (&gBrowserStorageList);
    while (!IsNull (&gBrowserStorageList, Link)) {
      Storage = BROWSER_STORAGE_FROM_LINK (Link);
      Link = GetNextNode (&gBrowserStorageList, Link);
      //
      // Check the current storage.
      //
      if (!CompareGuid (&Storage->Guid, (EFI_GUID *) VariableGuid)) {
        continue;
      }

      if (Storage->Type == EFI_HII_VARSTORE_BUFFER ||
          Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
        //
        // Buffer storage require both GUID and Name
        //
        if (VariableName == NULL) {
          return EFI_NOT_FOUND;
        }

        if (StrCmp (Storage->Name, (CHAR16 *) VariableName) != 0) {
          continue;
        }
      }

      if (Storage->Type == EFI_HII_VARSTORE_NAME_VALUE ||
          Storage->Type == EFI_HII_VARSTORE_BUFFER) {
        if (mSystemLevelFormSet == NULL || mSystemLevelFormSet->HiiHandle == NULL) {
          return EFI_NOT_FOUND;
        }

        if (Storage->HiiHandle != mSystemLevelFormSet->HiiHandle) {
          continue;
        }
      }

      Status = ProcessStorage (&TotalSize, &ResultsData, RetrieveData, Storage);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
        ConfigRequestAdjust (Storage, ResultsData, TRUE);
      }

      //
      // Different formsets may have same varstore, so here just set the flag
      // not exit the circle.
      //
      Found = TRUE;
      break;
    }

    if (!Found) {
      return EFI_NOT_FOUND;
    }
  } else {
    //
    // GUID/Name is not specified, take the first storage in FormSet
    //
    if (mSystemLevelFormSet == NULL) {
      return EFI_NOT_READY;
    }

    //
    // Generate <ConfigResp>
    //
    Link = GetFirstNode (&mSystemLevelFormSet->StorageListHead);
    if (IsNull (&mSystemLevelFormSet->StorageListHead, Link)) {
      return EFI_UNSUPPORTED;
    }

    FormsetStorage = FORMSET_STORAGE_FROM_LINK (Link);

    Status = ProcessStorage (&TotalSize, &ResultsData, RetrieveData, FormsetStorage->BrowserStorage);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (RetrieveData) {
    Status = TotalSize <= *ResultsDataSize ? EFI_SUCCESS : EFI_BUFFER_TOO_SMALL;
    *ResultsDataSize = TotalSize;
  } else {
    //
    // For performance concern, only update statement status for non-SCU formset.
    //
    if (mSystemLevelFormSet != NULL && !CompareGuid (mSystemLevelFormSet->ClassGuid, &mScuFormSetGuid)) {
      UpdateStatementStatus (mSystemLevelFormSet, NULL, FormSetLevel);
      mUpdateStatementStatus = (mHiiPackageListUpdated) ? TRUE : FALSE;
    }
  }

  return Status;

}

/**
  Create a new string in HII Package List.

  @param  [in] String                 The String to be added
  @param  [in] HiiHandle              The package list in the HII database to insert the
                                 specified string.

  @return The output string.

**/
EFI_STRING_ID
NewString (
  IN  CHAR16                   *String,
  IN  EFI_HII_HANDLE           HiiHandle
  )
{
  EFI_STRING_ID  StringId;

  StringId = HiiSetString (HiiHandle, 0, String, NULL);
  ASSERT (StringId != 0);

  return StringId;
}


/**
  Delete a string from HII Package List.

  @param  [in] StringId               Id of the string in HII database.
  @param  [in] HiiHandle              The HII package list handle.

  @retval EFI_SUCCESS                 The string was deleted successfully.

**/
EFI_STATUS
DeleteString (
  IN  EFI_STRING_ID            StringId,
  IN  EFI_HII_HANDLE           HiiHandle
  )
{
  CHAR16  NullChar;

  NullChar = CHAR_NULL;
  HiiSetString (HiiHandle, StringId, &NullChar, NULL);
  return EFI_SUCCESS;
}

/**
  Allocate new memory and then copy the Unicode string Source to Destination.

  @param  [in.out] Dest          Location to copy string
  @param  [in] Src               String to copy

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_OUT_OF_RESOURCES   The pool requested could not be allocated.

**/
EFI_STATUS
NewStringCpy (
  IN OUT CHAR16       **Dest,
  IN CHAR16           *Src
  )
{
  if (Dest == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (*Dest != NULL) {
    FreePool (*Dest);
  }
  *Dest = AllocateCopyPool (StrSize (Src), Src);
  ASSERT (*Dest != NULL);
  if (*Dest == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  return EFI_SUCCESS;
}


/**
  Allocate new memory and concatinate Source on the end of Destination.

  @param  [in.out] Dest               String to added to the end of.
  @param  [in] Src                    String to concatinate.

  @retval EFI_SUCCESS                 Concatinate string successfully.
  @retval Other                       Concatinate string fail.
**/
STATIC
EFI_STATUS
NewStringCat (
  IN OUT CHAR16       **Dest,
  IN CHAR16           *Src
  )
{
  CHAR16       *NewString;
  UINTN        TmpSize;
  EFI_STATUS   Status;

  Status = EFI_SUCCESS;
  if (*Dest == NULL) {
    Status = NewStringCpy (Dest, Src);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    return EFI_SUCCESS;
  }

  TmpSize = StrSize (*Dest);
  NewString = AllocateZeroPool (TmpSize + StrSize (Src) - 1);
  ASSERT (NewString != NULL);
  if (NewString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  StrCpy (NewString, *Dest);
  StrCat (NewString, Src);

  FreePool (*Dest);
  *Dest = NewString;
  return EFI_SUCCESS;
}

/**
  Get Value for given Name from a NameValue Storage.

  @param  [in]  Storage           The NameValue Storage.
  @param  [in]  Name              The Name.
  @param  [out] Value             The retured Value.
  @param  [in]  GetValueFrom      Where to get source value, from CurrentValue or OriginalValue.

  @retval EFI_SUCCESS             Value found for given Name.
  @retval EFI_NOT_FOUND           No such Name found in NameValue storage.

**/
EFI_STATUS
GetValueFromNameValueStorage (
  IN  BROWSER_STORAGE             *Storage,
  IN  CHAR16                      *Name,
  OUT  CHAR16                      **Value,
  IN  GET_SET_QUESTION_VALUE_WITH GetValueFrom
  )
{
  LIST_ENTRY              *Link;
  NAME_VALUE_NODE         *Node;
  EFI_STATUS              Status;
  Status = EFI_SUCCESS;
  if (GetValueFrom != GetSetValueWithEditBuffer && GetValueFrom != GetSetValueWithBuffer) {
    return EFI_INVALID_PARAMETER;
  }

  *Value = NULL;

  Link = GetFirstNode (&Storage->NameValueListHead);
  while (!IsNull (&Storage->NameValueListHead, Link)) {
    Node = NAME_VALUE_NODE_FROM_LINK (Link);

    if (StrCmp (Name, Node->Name) == 0) {
      if (GetValueFrom == GetSetValueWithEditBuffer) {
        Status = NewStringCpy (Value, Node->EditValue);
        if (EFI_ERROR (Status)) {
          return Status;
        }
      } else {
        Status = NewStringCpy (Value, Node->Value);
        if (EFI_ERROR (Status)) {
          return Status;
        }
      }
      return EFI_SUCCESS;
    }

    Link = GetNextNode (&Storage->NameValueListHead, Link);
  }

  return EFI_NOT_FOUND;
}


/**
  Set Value of given Name in a NameValue Storage.

  @param  [in] Storage                The NameValue Storage.
  @param  [in] Name                   The Name.
  @param  [in] Value                  The Value to set.
  @param  [in] SetValueTo             Whether update CurrentValue or Originalvalue.
  @param  [out] ReturnNode            The node use the input name.
                                      Node: name node
  @retval EFI_SUCCESS                 Value found for given Name.
  @retval EFI_NOT_FOUND               No such Name found in NameValue storage.

**/
EFI_STATUS
SetValueByName (
  IN BROWSER_STORAGE              *Storage,
  IN CONST CHAR16                 *Name,
  IN CONST CHAR16                 *Value,
  IN GET_SET_QUESTION_VALUE_WITH  SetValueTo,
  OUT NAME_VALUE_NODE             **ReturnNode
  )
{
  LIST_ENTRY              *Link;
  NAME_VALUE_NODE         *Node;
  CHAR16                  *Buffer;

  if (SetValueTo != GetSetValueWithEditBuffer && SetValueTo != GetSetValueWithBuffer) {
    return EFI_INVALID_PARAMETER;
  }

  Link = GetFirstNode (&Storage->NameValueListHead);
  while (!IsNull (&Storage->NameValueListHead, Link)) {
    Node = NAME_VALUE_NODE_FROM_LINK (Link);

    if (StrCmp (Name, Node->Name) == 0) {
      if (SetValueTo == GetSetValueWithEditBuffer) {
        Buffer = Node->EditValue;
      } else {
        Buffer = Node->Value;
      }
      if (Buffer != NULL) {
        FreePool (Buffer);
      }
      Buffer = AllocateCopyPool (StrSize (Value), Value);
      ASSERT (Buffer != NULL);
      if (Buffer == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      if (SetValueTo == GetSetValueWithEditBuffer) {
        Node->EditValue = Buffer;
      } else {
        Node->Value = Buffer;
      }

      if (ReturnNode != NULL) {
        *ReturnNode = Node;
      }

      return EFI_SUCCESS;
    }

    Link = GetNextNode (&Storage->NameValueListHead, Link);
  }

  return EFI_NOT_FOUND;
}


/**
  Convert setting of Buffer Storage or NameValue Storage to <ConfigResp>.

  @param  [in] Storage                The Storage to be conveted.
  @param  [in] ConfigResp             The returned <ConfigResp>.
  @param  [in] ConfigRequest          The ConfigRequest string.
  @param  [in] GetEditBuf             Get the data from editbuffer or buffer.

  @retval EFI_SUCCESS            Convert success.
  @retval EFI_INVALID_PARAMETER  Incorrect storage type.

**/
EFI_STATUS
StorageToConfigResp (
  IN BROWSER_STORAGE         *Storage,
  IN CHAR16                  **ConfigResp,
  IN CHAR16                  *ConfigRequest,
  IN BOOLEAN                 GetEditBuf
  )
{
  EFI_STATUS              Status;
  EFI_STRING              Progress;
  LIST_ENTRY              *Link;
  NAME_VALUE_NODE         *Node;
  UINT8                   *SourceBuf;

  Status = EFI_SUCCESS;

  switch (Storage->Type) {
  case EFI_HII_VARSTORE_BUFFER:
  case EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER:
    SourceBuf = GetEditBuf ? Storage->EditBuffer : Storage->Buffer;
    Status = gHiiConfigRouting->BlockToConfig (
                                  gHiiConfigRouting,
                                  ConfigRequest,
                                  SourceBuf,
                                  Storage->Size,
                                  ConfigResp,
                                  &Progress
                                  );
    break;

  case EFI_HII_VARSTORE_NAME_VALUE:
    *ConfigResp = NULL;
    Status = NewStringCat (ConfigResp, Storage->ConfigHdr);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    Link = GetFirstNode (&Storage->NameValueListHead);
    while (!IsNull (&Storage->NameValueListHead, Link)) {
      Node = NAME_VALUE_NODE_FROM_LINK (Link);

      if (StrStr (ConfigRequest, Node->Name) != NULL) {
        Status = NewStringCat (ConfigResp, L"&");
        if (EFI_ERROR(Status)) {
          return Status;
        }
        Status = NewStringCat (ConfigResp, Node->Name);
        if (EFI_ERROR(Status)) {
          return Status;
        }
        Status = NewStringCat (ConfigResp, L"=");
        if (EFI_ERROR(Status)) {
          return Status;
        }
        if (GetEditBuf) {
          Status = NewStringCat (ConfigResp, Node->EditValue);
        } else {
          Status = NewStringCat (ConfigResp, Node->Value);
        }
        if (EFI_ERROR(Status)) {
          return Status;
        }
      }
      Link = GetNextNode (&Storage->NameValueListHead, Link);
    }
    break;

  case EFI_HII_VARSTORE_EFI_VARIABLE:
  default:
    Status = EFI_INVALID_PARAMETER;
    break;
  }

  return Status;
}


/**
  Convert <ConfigResp> to settings in Buffer Storage or NameValue Storage.

  @param  [in] Storage           The Storage to receive the settings.
  @param  [in] ConfigResp        The <ConfigResp> to be converted.

  @retval EFI_SUCCESS            Convert success.
  @retval EFI_INVALID_PARAMETER  Incorrect storage type.

**/
EFI_STATUS
ConfigRespToStorage (
  IN BROWSER_STORAGE         *Storage,
  IN CHAR16                  *ConfigResp
  )
{
  EFI_STATUS  Status;
  EFI_STRING  Progress;
  UINTN       BufferSize;
  CHAR16      *StrPtr;
  CHAR16      *Name;
  CHAR16      *Value;

  Status = EFI_SUCCESS;

  switch (Storage->Type) {
  case EFI_HII_VARSTORE_BUFFER:
  case EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER:
    BufferSize = Storage->Size;
    Status = gHiiConfigRouting->ConfigToBlock (
                                  gHiiConfigRouting,
                                  ConfigResp,
                                  Storage->EditBuffer,
                                  &BufferSize,
                                  &Progress
                                  );
    break;

  case EFI_HII_VARSTORE_NAME_VALUE:
    StrPtr = StrStr (ConfigResp, L"PATH");
    if (StrPtr == NULL) {
      break;
    }
    StrPtr = StrStr (ConfigResp, L"&");
    while (StrPtr != NULL) {
      //
      // Skip '&'
      //
      StrPtr = StrPtr + 1;
      Name = StrPtr;
      StrPtr = StrStr (StrPtr, L"=");
      if (StrPtr == NULL) {
        break;
      }
      *StrPtr = 0;

      //
      // Skip '='
      //
      StrPtr = StrPtr + 1;
      Value = StrPtr;
      StrPtr = StrStr (StrPtr, L"&");
      if (StrPtr != NULL) {
        *StrPtr = 0;
      }
      SetValueByName (Storage, Name, Value, GetSetValueWithEditBuffer, NULL);
    }
    break;

  case EFI_HII_VARSTORE_EFI_VARIABLE:
  default:
    Status = EFI_INVALID_PARAMETER;
    break;
  }

  return Status;
}


/**
  Get Question's current Value.

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  Question               Question to be initialized.
  @param  GetValueFrom           Where to get value, may from editbuffer, buffer or hii driver.

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
GetQuestionValue (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN OUT FORM_BROWSER_STATEMENT       *Question,
  IN GET_SET_QUESTION_VALUE_WITH      GetValueFrom
  )
{
  EFI_STATUS          Status;
  BOOLEAN             Enabled;
  BOOLEAN             Pending;
  UINT8               *Dst;
  UINTN               StorageWidth;
  EFI_TIME            EfiTime;
  BROWSER_STORAGE     *Storage;
  EFI_IFR_TYPE_VALUE  *QuestionValue;
  CHAR16              *ConfigRequest;
  CHAR16              *Progress;
  CHAR16              *Result;
  CHAR16              *Value;
  CHAR16              *StringPtr;
  UINTN               Length;
  UINTN               Index;
  UINTN               LengthStr;
  BOOLEAN             IsBufferStorage;
  BOOLEAN             IsString;
  CHAR16              TemStr[5];
  UINT8               DigitUint8;

  Status = EFI_SUCCESS;
  Value  = NULL;
  Result = NULL;

  if (GetValueFrom >= GetSetValueWithMax) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Question value is provided by an Expression, evaluate it
  //
  if (Question->ValueExpression != NULL) {
    Status = EvaluateExpression (FormSet, Form, Question->ValueExpression);
    if (!EFI_ERROR (Status)) {
      if (Question->ValueExpression->Result.Type == EFI_IFR_TYPE_BUFFER) {
        ASSERT (Question->HiiValue.Type == EFI_IFR_TYPE_BUFFER && Question->HiiValue.Buffer != NULL);
        if (Question->StorageWidth > Question->ValueExpression->Result.BufferLen) {
          CopyMem (Question->HiiValue.Buffer, Question->ValueExpression->Result.Buffer, Question->ValueExpression->Result.BufferLen);
          Question->HiiValue.BufferLen = Question->ValueExpression->Result.BufferLen;
        } else {
          CopyMem (Question->HiiValue.Buffer, Question->ValueExpression->Result.Buffer, Question->StorageWidth);
          Question->HiiValue.BufferLen = Question->StorageWidth;
        }
        FreePool (Question->ValueExpression->Result.Buffer);
      }
      Question->HiiValue.Type = Question->ValueExpression->Result.Type;
      CopyMem (&Question->HiiValue.Value, &Question->ValueExpression->Result.Value, sizeof (EFI_IFR_TYPE_VALUE));
    }
    return Status;
  }

  //
  // Get question value by read expression.
  //
  if (Question->ReadExpression != NULL && Form->FormType == STANDARD_MAP_FORM_TYPE) {
    Status = EvaluateExpression (FormSet, Form, Question->ReadExpression);
    if (!EFI_ERROR (Status) &&
      ((Question->ReadExpression->Result.Type < EFI_IFR_TYPE_OTHER) || (Question->ReadExpression->Result.Type == EFI_IFR_TYPE_BUFFER))) {
      //
      // Only update question value to the valid result.
      //
      if (Question->ReadExpression->Result.Type == EFI_IFR_TYPE_BUFFER) {
        ASSERT (Question->HiiValue.Type == EFI_IFR_TYPE_BUFFER && Question->HiiValue.Buffer != NULL);
        if (Question->StorageWidth > Question->ReadExpression->Result.BufferLen) {
          CopyMem (Question->HiiValue.Buffer, Question->ReadExpression->Result.Buffer, Question->ReadExpression->Result.BufferLen);
          Question->HiiValue.BufferLen = Question->ReadExpression->Result.BufferLen;
        } else {
          CopyMem (Question->HiiValue.Buffer, Question->ReadExpression->Result.Buffer, Question->StorageWidth);
          Question->HiiValue.BufferLen = Question->StorageWidth;
        }
        FreePool (Question->ReadExpression->Result.Buffer);
      }
      Question->HiiValue.Type = Question->ReadExpression->Result.Type;
      CopyMem (&Question->HiiValue.Value, &Question->ReadExpression->Result.Value, sizeof (EFI_IFR_TYPE_VALUE));
      return EFI_SUCCESS;
    }
  }

  //
  // Question value is provided by RTC
  //
  Storage = Question->Storage;
  QuestionValue = &Question->HiiValue.Value;
  if (Storage == NULL) {
    //
    // It's a Question without storage, or RTC date/time
    //
    if (Question->Operand == EFI_IFR_DATE_OP || Question->Operand == EFI_IFR_TIME_OP) {
      //
      // Date and time define the same Flags bit
      //
      switch (Question->Flags & EFI_QF_DATE_STORAGE) {
      case QF_DATE_STORAGE_TIME:
        Status = gRT->GetTime (&EfiTime, NULL);
        break;

      case QF_DATE_STORAGE_WAKEUP:
        Status = gRT->GetWakeupTime (&Enabled, &Pending, &EfiTime);
        break;

      case QF_DATE_STORAGE_NORMAL:
      default:
        //
        // For date/time without storage
        //
        return EFI_SUCCESS;
      }

      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (Question->Operand == EFI_IFR_DATE_OP) {
        QuestionValue->date.Year  = EfiTime.Year;
        QuestionValue->date.Month = EfiTime.Month;
        QuestionValue->date.Day   = EfiTime.Day;
      } else {
        QuestionValue->time.Hour   = EfiTime.Hour;
        QuestionValue->time.Minute = EfiTime.Minute;
        QuestionValue->time.Second = EfiTime.Second;
      }
    }

    return EFI_SUCCESS;
  }

  //
  // Question value is provided by EFI variable
  //
  StorageWidth = Question->StorageWidth;
  if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
    if (Question->BufferValue != NULL) {
      Dst = Question->BufferValue;
    } else {
      Dst = (UINT8 *) QuestionValue;
    }

    Status = gRT->GetVariable (
                     Question->VariableName,
                     &Storage->Guid,
                     NULL,
                     &StorageWidth,
                     Dst
                     );
    //
    // Always return success, even this EFI variable doesn't exist
    //
    return EFI_SUCCESS;
  }

  //
  // Question Value is provided by Buffer Storage or NameValue Storage
  //
  if (Question->BufferValue != NULL) {
    //
    // This Question is password or orderedlist
    //
    Dst = Question->BufferValue;
  } else {
    //
    // Other type of Questions
    //
    Dst = (UINT8 *) &Question->HiiValue.Value;
  }

  if (Storage->Type == EFI_HII_VARSTORE_BUFFER ||
      Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
    IsBufferStorage = TRUE;
  } else {
    IsBufferStorage = FALSE;
  }
  IsString = (BOOLEAN) ((Question->HiiValue.Type == EFI_IFR_TYPE_STRING) ?  TRUE : FALSE);
  if (GetValueFrom == GetSetValueWithEditBuffer || GetValueFrom == GetSetValueWithBuffer ) {
    if (IsBufferStorage) {
      if (GetValueFrom == GetSetValueWithEditBuffer) {
        //
        // Copy from storage Edit buffer
        //
        CopyMem (Dst, Storage->EditBuffer + Question->VarStoreInfo.VarOffset, StorageWidth);
      } else {
        //
        // Copy from storage Edit buffer
        //
        CopyMem (Dst, Storage->Buffer + Question->VarStoreInfo.VarOffset, StorageWidth);
      }
    } else {
      Value = NULL;
      Status = GetValueFromNameValueStorage (Storage, Question->VariableName, &Value, GetValueFrom);
      ASSERT (Value != NULL);
      if (Value == NULL) {
        return Status;
      }

      LengthStr = StrLen (Value);
      Status    = EFI_SUCCESS;
      if (IsString) {
        //
        // Convert Config String to Unicode String, e.g "0041004200430044" => "ABCD"
        // Add string tail char L'\0' into Length
        //
        Length    = StorageWidth + sizeof (CHAR16);
        if (Length < ((LengthStr / 4 + 1) * 2)) {
          Status = EFI_BUFFER_TOO_SMALL;
        } else {
          StringPtr = (CHAR16 *) Dst;
          ZeroMem (TemStr, sizeof (TemStr));
          for (Index = 0; Index < LengthStr; Index += 4) {
            StrnCpy (TemStr, Value + Index, 4);
            StringPtr[Index/4] = (CHAR16) StrHexToUint64 (TemStr);
          }
          //
          // Add tailing L'\0' character
          //
          StringPtr[Index/4] = L'\0';
        }
      } else {
        if (StorageWidth < ((LengthStr + 1) / 2)) {
          Status = EFI_BUFFER_TOO_SMALL;
        } else {
          ZeroMem (TemStr, sizeof (TemStr));
          for (Index = 0; Index < LengthStr; Index ++) {
            TemStr[0] = Value[LengthStr - Index - 1];
            DigitUint8 = (UINT8) StrHexToUint64 (TemStr);
            if ((Index & 1) == 0) {
              Dst [Index/2] = DigitUint8;
            } else {
              Dst [Index/2] = (UINT8) ((DigitUint8 << 4) + Dst [Index/2]);
            }
          }
        }
      }

      FBFreePool ((VOID **) &Value);
    }
  } else {
    //
    // <ConfigRequest> ::= <ConfigHdr> + <BlockName> ||
    //                   <ConfigHdr> + "&" + <VariableName>
    //
    if (IsBufferStorage) {
      Length = StrLen (Storage->ConfigHdr);
      Length += StrLen (Question->BlockName);
    } else {
      Length = StrLen (Storage->ConfigHdr);
      Length += StrLen (Question->VariableName) + 1;
    }
    ConfigRequest = AllocateZeroPool ((Length + 1) * sizeof (CHAR16));
    ASSERT (ConfigRequest != NULL);
    if (ConfigRequest == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    StrCpy (ConfigRequest, Storage->ConfigHdr);
    if (IsBufferStorage) {
      StrCat (ConfigRequest, Question->BlockName);
    } else {
      StrCat (ConfigRequest, L"&");
      StrCat (ConfigRequest, Question->VariableName);
    }

    //
    // Request current settings from Configuration Driver
    //
    Status = gHiiConfigRouting->ExtractConfig (
                                      gHiiConfigRouting,
                                      ConfigRequest,
                                      &Progress,
                                      &Result
                                      );
    FreePool (ConfigRequest);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Skip <ConfigRequest>
    //
    if (IsBufferStorage) {
      Value = StrStr (Result, L"&VALUE");
      if (Value == NULL) {
        FreePool (Result);
        return EFI_NOT_FOUND;
      }
      //
      // Skip "&VALUE"
      //
      Value = Value + 6;
    } else {
      Value = Result + Length;
    }
    if (Value == NULL || *Value != '=') {
      FreePool (Result);
      return EFI_NOT_FOUND;
    }
    //
    // Skip '=', point to value
    //
    Value = Value + 1;

    //
    // Suppress <AltResp> if any
    //
    StringPtr = Value;
    while (*StringPtr != L'\0' && *StringPtr != L'&') {
      StringPtr++;
    }
    *StringPtr = L'\0';

    LengthStr = StrLen (Value);
    Status    = EFI_SUCCESS;
    if (!IsBufferStorage && IsString) {
      //
      // Convert Config String to Unicode String, e.g "0041004200430044" => "ABCD"
      // Add string tail char L'\0' into Length
      //
      Length    = StorageWidth + sizeof (CHAR16);
      if (Length < ((LengthStr / 4 + 1) * 2)) {
        Status = EFI_BUFFER_TOO_SMALL;
      } else {
        StringPtr = (CHAR16 *) Dst;
        ZeroMem (TemStr, sizeof (TemStr));
        for (Index = 0; Index < LengthStr; Index += 4) {
          StrnCpy (TemStr, Value + Index, 4);
          StringPtr[Index/4] = (CHAR16) StrHexToUint64 (TemStr);
        }
        //
        // Add tailing L'\0' character
        //
        StringPtr[Index/4] = L'\0';
      }
    } else {
      if (StorageWidth < ((LengthStr + 1) / 2)) {
        Status = EFI_BUFFER_TOO_SMALL;
      } else {
        ZeroMem (TemStr, sizeof (TemStr));
        for (Index = 0; Index < LengthStr; Index ++) {
          TemStr[0] = Value[LengthStr - Index - 1];
          DigitUint8 = (UINT8) StrHexToUint64 (TemStr);
          if ((Index & 1) == 0) {
            Dst [Index/2] = DigitUint8;
          } else {
            Dst [Index/2] = (UINT8) ((DigitUint8 << 4) + Dst [Index/2]);
          }
        }
      }
    }

    if (EFI_ERROR (Status)) {
      FreePool (Result);
      return Status;
    }

    //
    // Synchronize Edit Buffer
    //
    if (IsBufferStorage) {
      CopyMem (Storage->EditBuffer + Question->VarStoreInfo.VarOffset, Dst, StorageWidth);
    } else {
      SetValueByName (Storage, Question->VariableName, Value, GetSetValueWithEditBuffer, NULL);
    }

    if (Result != NULL) {
      FreePool (Result);
    }
  }

  return Status;
}


/**
  Save Question Value to edit copy(cached) or Storage(uncached).

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  Question               Pointer to the Question.
  @param  SetValueTo             Update the question value to editbuffer , buffer or hii driver.

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
SetQuestionValue (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN OUT FORM_BROWSER_STATEMENT       *Question,
  IN GET_SET_QUESTION_VALUE_WITH      SetValueTo
  )
{
  EFI_STATUS          Status;
  BOOLEAN             Enabled;
  BOOLEAN             Pending;
  UINT8               *Src;
  EFI_TIME            EfiTime;
  UINTN               BufferLen;
  UINTN               StorageWidth;
  BROWSER_STORAGE     *Storage;
  EFI_IFR_TYPE_VALUE  *QuestionValue;
  CHAR16              *ConfigResp;
  CHAR16              *Progress;
  CHAR16              *Value;
  UINTN               Length;
  BOOLEAN             IsBufferStorage;
  BOOLEAN             IsString;
  UINT8               *TemBuffer;
  CHAR16              *TemName;
  CHAR16              *TemString;
  UINTN               Index;
  NAME_VALUE_NODE     *Node;
  SETUP_DATE_CHANGE   SetupDateChange;

  Status = EFI_SUCCESS;
  Node   = NULL;

  if (SetValueTo >= GetSetValueWithMax) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If Question value is provided by an Expression, then it is read only
  //
  if (Question->ValueExpression != NULL) {
    return Status;
  }

  //
  // Before set question value, evaluate its write expression.
  //
  if (Question->WriteExpression != NULL && Form->FormType == STANDARD_MAP_FORM_TYPE) {
    Status = EvaluateExpression (FormSet, Form, Question->WriteExpression);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Question value is provided by RTC
  //
  Storage = Question->Storage;
  QuestionValue = &Question->HiiValue.Value;
  if (Storage == NULL) {
    //
    // It's a Question without storage, or RTC date/time
    //
    if (Question->Operand == EFI_IFR_DATE_OP || Question->Operand == EFI_IFR_TIME_OP) {
      //
      // Date and time define the same Flags bit
      //
      switch (Question->Flags & EFI_QF_DATE_STORAGE) {
      case QF_DATE_STORAGE_TIME:
        Status = gRT->GetTime (&EfiTime, NULL);
        break;

      case QF_DATE_STORAGE_WAKEUP:
        Status = gRT->GetWakeupTime (&Enabled, &Pending, &EfiTime);
        break;

      case QF_DATE_STORAGE_NORMAL:
      default:
        //
        // For date/time without storage
        //
        return EFI_SUCCESS;
      }

      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (FeaturePcdGet(PcdH2OSetupChangeDisplaySupported)) {
        SetupDateChange.SetTime = TRUE;
        SetupDateChange.Prompt = Question->Statement.Prompt;
        SetupDateChange.OrgDate.Year = EfiTime.Year;
        SetupDateChange.OrgDate.Month= EfiTime.Month;
        SetupDateChange.OrgDate.Day  = EfiTime.Day;
        SetupDateChange.ModDate.Year = QuestionValue->date.Year;
        SetupDateChange.ModDate.Month= QuestionValue->date.Month;
        SetupDateChange.ModDate.Day  = QuestionValue->date.Day;
      }

      if (Question->Operand == EFI_IFR_DATE_OP) {
        EfiTime.Year  = QuestionValue->date.Year;
        EfiTime.Month = QuestionValue->date.Month;
        EfiTime.Day   = QuestionValue->date.Day;
      } else {
        EfiTime.Hour   = QuestionValue->time.Hour;
        EfiTime.Minute = QuestionValue->time.Minute;
        EfiTime.Second = QuestionValue->time.Second;
      }

      if ((Question->Flags & EFI_QF_DATE_STORAGE) == QF_DATE_STORAGE_TIME) {
        Status = gRT->SetTime (&EfiTime);
        if (FeaturePcdGet(PcdH2OSetupChangeDisplaySupported) && !EFI_ERROR (Status) && (Question->Operand == EFI_IFR_DATE_OP)) {
          if (CompareMem (&(SetupDateChange.OrgDate), &(SetupDateChange.ModDate), sizeof(EFI_HII_DATE)) != 0) {
            REPORT_STATUS_CODE_EX (
              EFI_PROGRESS_CODE,
              EFI_SOFTWARE_DXE_RT_DRIVER | EFI_SW_RS_PC_SET_TIME,
              0,
              NULL,
              &gH2OSetupChangeStatusCodeGuid,
              (VOID *) &SetupDateChange,
              sizeof (SETUP_DATE_CHANGE)
              );
          }
        }
      } else {
        Status = gRT->SetWakeupTime (TRUE, &EfiTime);
      }
    }

    return Status;
  }

  //
  // Question value is provided by EFI variable
  //
  StorageWidth = Question->StorageWidth;
  if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
    if (Question->BufferValue != NULL) {
      Src = Question->BufferValue;
    } else {
      Src = (UINT8 *) QuestionValue;
    }

    Status = gRT->SetVariable (
                     Question->VariableName,
                     &Storage->Guid,
                     Storage->Attributes,
                     StorageWidth,
                     Src
                     );
    return Status;
  }

  //
  // Question Value is provided by Buffer Storage or NameValue Storage
  //
  if (Question->BufferValue != NULL) {
    Src = Question->BufferValue;
  } else {
    Src = (UINT8 *) &Question->HiiValue.Value;
  }

  if (Storage->Type == EFI_HII_VARSTORE_BUFFER ||
      Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
    IsBufferStorage = TRUE;
  } else {
    IsBufferStorage = FALSE;
  }
  IsString = (BOOLEAN) ((Question->HiiValue.Type == EFI_IFR_TYPE_STRING) ?  TRUE : FALSE);

  if (SetValueTo == GetSetValueWithEditBuffer || SetValueTo == GetSetValueWithBuffer) {
    if (IsBufferStorage) {
      if (SetValueTo == GetSetValueWithEditBuffer) {
        //
        // Copy to storage edit buffer
        //
        CopyMem (Storage->EditBuffer + Question->VarStoreInfo.VarOffset, Src, StorageWidth);
      } else if (SetValueTo == GetSetValueWithBuffer) {
        //
        // Copy to storage edit buffer
        //
        CopyMem (Storage->Buffer + Question->VarStoreInfo.VarOffset, Src, StorageWidth);
      }
    } else {
      if (IsString) {
        //
        // Allocate enough string buffer.
        //
        Value = NULL;
        BufferLen = ((StrLen ((CHAR16 *) Src) * 4) + 1) * sizeof (CHAR16);
        Value = AllocateZeroPool (BufferLen);
        ASSERT (Value != NULL);
        //
        // Convert Unicode String to Config String, e.g. "ABCD" => "0041004200430044"
        //
        TemName = (CHAR16 *) Src;
        TemString = Value;
        for (; *TemName != L'\0'; TemName++) {
          TemString += UnicodeValueToString (TemString, PREFIX_ZERO | RADIX_HEX, *TemName, 4);
        }
      } else {
        BufferLen = StorageWidth * 2 + 1;
        Value = AllocateZeroPool (BufferLen * sizeof (CHAR16));
        ASSERT (Value != NULL);
        //
        // Convert Buffer to Hex String
        //
        TemBuffer = Src + StorageWidth - 1;
        TemString = Value;
        for (Index = 0; Index < StorageWidth; Index ++, TemBuffer --) {
          TemString += UnicodeValueToString (TemString, PREFIX_ZERO | RADIX_HEX, *TemBuffer, 2);
        }
      }

      Status = SetValueByName (Storage, Question->VariableName, Value, SetValueTo, &Node);
      FreePool (Value);
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }
  } else if (SetValueTo == GetSetValueWithHiiDriver) {
    //
    // <ConfigResp> ::= <ConfigHdr> + <BlockName> + "&VALUE=" + "<HexCh>StorageWidth * 2" ||
    //                <ConfigHdr> + "&" + <VariableName> + "=" + "<string>"
    //
    if (IsBufferStorage) {
      Length = StrLen (Question->BlockName) + 7;
    } else {
      Length = StrLen (Question->VariableName) + 2;
    }
    if (!IsBufferStorage && IsString) {
      Length += (StrLen ((CHAR16 *) Src) * 4);
    } else {
      Length += (StorageWidth * 2);
    }
    ConfigResp = AllocateZeroPool ((StrLen (Storage->ConfigHdr) + Length + 1) * sizeof (CHAR16));
    ASSERT (ConfigResp != NULL);
    if (ConfigResp == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    StrCpy (ConfigResp, Storage->ConfigHdr);
    if (IsBufferStorage) {
      StrCat (ConfigResp, Question->BlockName);
      StrCat (ConfigResp, L"&VALUE=");
    } else {
      StrCat (ConfigResp, L"&");
      StrCat (ConfigResp, Question->VariableName);
      StrCat (ConfigResp, L"=");
    }

    Value = ConfigResp + StrLen (ConfigResp);

    if (!IsBufferStorage && IsString) {
      //
      // Convert Unicode String to Config String, e.g. "ABCD" => "0041004200430044"
      //
      TemName = (CHAR16 *) Src;
      TemString = Value;
      for (; *TemName != L'\0'; TemName++) {
        TemString += UnicodeValueToString (TemString, PREFIX_ZERO | RADIX_HEX, *TemName, 4);
      }
    } else {
      //
      // Convert Buffer to Hex String
      //
      TemBuffer = Src + StorageWidth - 1;
      TemString = Value;
      for (Index = 0; Index < StorageWidth; Index ++, TemBuffer --) {
        TemString += UnicodeValueToString (TemString, PREFIX_ZERO | RADIX_HEX, *TemBuffer, 2);
      }
    }

    //
    // Convert to lower char.
    //
    for (TemString = Value; *Value != L'\0'; Value++) {
      if (*Value >= L'A' && *Value <= L'Z') {
        *Value = (CHAR16) (*Value - L'A' + L'a');
      }
    }

    //
    // Submit Question Value to Configuration Driver
    //
    Status = gHiiConfigRouting->RouteConfig (
                                      gHiiConfigRouting,
                                      ConfigResp,
                                      &Progress
                                      );
    if (EFI_ERROR (Status)) {
      FreePool (ConfigResp);
      return Status;
    }
    FreePool (ConfigResp);

    //
    // Sync storage, from editbuffer to buffer.
    //
    CopyMem (Storage->Buffer + Question->VarStoreInfo.VarOffset, Src, StorageWidth);
  }

  return Status;
}


/**
  Perform nosubmitif check for a Form.

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  Question               The Question to be validated.
  @param  Type                   Validation type: NoSubmit

  @retval EFI_SUCCESS            Form validation pass.
  @retval other                  Form validation failed.

**/
EFI_STATUS
ValidateQuestion (
  IN  FORM_BROWSER_FORMSET            *FormSet,
  IN  FORM_BROWSER_FORM               *Form,
  IN  FORM_BROWSER_STATEMENT          *Question,
  IN  UINT32                           Type
  )
{
  EFI_STATUS              Status;
  LIST_ENTRY              *Link;
  LIST_ENTRY              *ListHead;
  FORM_EXPRESSION         *Expression;
  UINT32                  BrowserStatus;
  CHAR16                  *ErrorStr;

  BrowserStatus = BROWSER_SUCCESS;
  ErrorStr      = NULL;

  switch (Type) {
  case EFI_HII_EXPRESSION_INCONSISTENT_IF:
    ListHead = &Question->InconsistentListHead;
    break;

  case EFI_HII_EXPRESSION_WARNING_IF:
    ListHead = &Question->WarningListHead;
    break;

  case EFI_HII_EXPRESSION_NO_SUBMIT_IF:
    ListHead = &Question->NoSubmitListHead;
    break;

  default:
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  Link = GetFirstNode (ListHead);
  while (!IsNull (ListHead, Link)) {
    Expression = FORM_EXPRESSION_FROM_LINK (Link);

    //
    // Evaluate the expression
    //
    Status = EvaluateExpression (FormSet, Form, Expression);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (IsTrue (&Expression->Result)) {
      switch (Type) {
      case EFI_HII_EXPRESSION_INCONSISTENT_IF:
        BrowserStatus = BROWSER_INCONSISTENT_IF;
        break;

      case EFI_HII_EXPRESSION_WARNING_IF:
        BrowserStatus = BROWSER_WARNING_IF;
        break;

      case EFI_HII_EXPRESSION_NO_SUBMIT_IF:
        BrowserStatus = BROWSER_NO_SUBMIT_IF;
        //
        // This code only used to compatible with old display engine,
        // New display engine will not use this field.
        //
        if (Expression->Error != 0) {
          ErrorStr = GetString (Expression->Error, FormSet->HiiHandle);
        }
        break;

      default:
        ASSERT (FALSE);
        break;
      }

      PopupErrorMessage(BrowserStatus, Expression->OpCode, ErrorStr);

      if (ErrorStr != NULL) {
        FreePool (ErrorStr);
      }

      if (Type == EFI_HII_EXPRESSION_WARNING_IF) {
        return EFI_SUCCESS;
      } else {
        return EFI_NOT_READY;
      }
    }

    Link = GetNextNode (ListHead, Link);
  }

  return EFI_SUCCESS;
}

/**
  Perform question check.

  If one question has more than one check, process form high priority to low.
  Only one error info will be popup.

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  Question               The Question to be validated.

  @retval EFI_SUCCESS            Form validation pass.
  @retval other                  Form validation failed.

**/
EFI_STATUS
ValueChangedValidation (
  IN  FORM_BROWSER_FORMSET            *FormSet,
  IN  FORM_BROWSER_FORM               *Form,
  IN  FORM_BROWSER_STATEMENT          *Question
  )
{
  EFI_STATUS   Status;

  Status = EFI_SUCCESS;

  //
  // Do the inconsistentif check.
  //
  if (!IsListEmpty (&Question->InconsistentListHead)) {
    Status = ValidateQuestion (FormSet, Form, Question, EFI_HII_EXPRESSION_INCONSISTENT_IF);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Do the warningif check.
  //
  if (!IsListEmpty (&Question->WarningListHead)) {
    Status = ValidateQuestion (FormSet, Form, Question, EFI_HII_EXPRESSION_WARNING_IF);
  }

  return Status;
}

/**
  Perform NoSubmit check for each Form in FormSet.

  @param  [in] FormSet           FormSet data structure.
  @param  [in] CurrentForm       Current input Form data structure.

  @retval EFI_SUCCESS            Form validation pass.
  @retval other                  Form validation failed.

**/
STATIC
EFI_STATUS
NoSubmitCheck (
  IN  FORM_BROWSER_FORMSET            *FormSet,
  IN  FORM_BROWSER_FORM               *CurrentForm
  )
{
  EFI_STATUS              Status;
  LIST_ENTRY              *Link;
  FORM_BROWSER_STATEMENT  *Question;
  FORM_BROWSER_FORM       *Form;
  LIST_ENTRY              *LinkForm;

  LinkForm = GetFirstNode (&FormSet->FormListHead);
  while (!IsNull (&FormSet->FormListHead, LinkForm)) {
    Form = FORM_BROWSER_FORM_FROM_LINK (LinkForm);
    LinkForm = GetNextNode (&FormSet->FormListHead, LinkForm);

    if (CurrentForm != NULL && CurrentForm != Form) {
      continue;
    }

    Link = GetFirstNode (&Form->StatementListHead);
    while (!IsNull (&Form->StatementListHead, Link)) {
      Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);

      Status = ValidateQuestion (FormSet, Form, Question, EFI_HII_EXPRESSION_NO_SUBMIT_IF);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Link = GetNextNode (&Form->StatementListHead, Link);
    }
  }

  return EFI_SUCCESS;
}

/**
  Fill storage's edit copy with settings requested from Configuration Driver.

  @param  FormSet                FormSet data structure.
  @param  Storage                The storage which need to sync.
  @param  ConfigRequest          The config request string which used to sync storage.
  @param  SyncOrRestore          Sync the buffer to editbuffer or Restore  the
                                 editbuffer to buffer
                                 if TRUE, copy the editbuffer to the buffer.
                                 if FALSE, copy the buffer to the editbuffer.

  @retval EFI_SUCCESS            The function completed successfully.

**/
STATIC
EFI_STATUS
SynchronizeStorage (
  IN  FORM_BROWSER_FORMSET        *FormSet,
  OUT BROWSER_STORAGE             *Storage,
  IN  CHAR16                      *ConfigRequest,
  IN  BOOLEAN                     SyncOrRestore
  )
{
  EFI_STATUS              Status;
  EFI_STRING              Progress;
  EFI_STRING              Result;
  UINTN                   BufferSize;
  LIST_ENTRY              *Link;
  NAME_VALUE_NODE         *Node;
  UINT8                   *Src;
  UINT8                   *Dst;

  Status = EFI_SUCCESS;
  Result = NULL;

  if (Storage->Type == EFI_HII_VARSTORE_BUFFER ||
      (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER)) {
    BufferSize = Storage->Size;

    if (SyncOrRestore) {
      Src = Storage->EditBuffer;
      Dst = Storage->Buffer;
    } else {
      Src = Storage->Buffer;
      Dst = Storage->EditBuffer;
    }

    if (ConfigRequest != NULL) {
      Status = gHiiConfigRouting->BlockToConfig(
                                    gHiiConfigRouting,
                                    ConfigRequest,
                                    Src,
                                    BufferSize,
                                    &Result,
                                    &Progress
                                    );
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Status = gHiiConfigRouting->ConfigToBlock (
                                    gHiiConfigRouting,
                                    Result,
                                    Dst,
                                    &BufferSize,
                                    &Progress
                                    );
      if (Result != NULL) {
        FreePool (Result);
      }
    } else {
      CopyMem (Dst, Src, BufferSize);
    }
  } else if (Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
    Link = GetFirstNode (&Storage->NameValueListHead);
    while (!IsNull (&Storage->NameValueListHead, Link)) {
      Node = NAME_VALUE_NODE_FROM_LINK (Link);

      if ((ConfigRequest != NULL && StrStr (ConfigRequest, Node->Name) != NULL) ||
          (ConfigRequest == NULL)) {
        if (SyncOrRestore) {
          NewStringCpy (&Node->Value, Node->EditValue);
        } else {
          NewStringCpy (&Node->EditValue, Node->Value);
        }
      }

      Link = GetNextNode (&Storage->NameValueListHead, Link);
    }
  }

  return Status;
}

/**
  When discarding the question value, call the callback function with Changed type
  to inform the callback function associated with a question.

  @param  [in] FormSet                FormSet data structure.
  @param  [in] Form                   Form data structure.

**/
STATIC
VOID
SendDiscardInfoToDriver (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form
  )
{
  LIST_ENTRY                  *Link;
  FORM_BROWSER_STATEMENT      *Question;
  EFI_IFR_TYPE_VALUE          *TypeValue;
  EFI_BROWSER_ACTION_REQUEST  ActionRequest;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;
  EFI_QUESTION_ID                 QuestionId;
  EFI_HII_HANDLE                  OrgHiiHandle;

  if (FormSet->ConfigAccess == NULL) {
    return;
  }

  OrgHiiHandle = mSystemLevelFormSet->HiiHandle;
  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
    Link = GetNextNode (&Form->StatementListHead, Link);

    if (Question->Storage == NULL || Question->Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
      continue;
    }

    if ((Question->QuestionFlags & EFI_IFR_FLAG_CALLBACK) != EFI_IFR_FLAG_CALLBACK) {
      continue;
    }

    if (Question->Operand == EFI_IFR_PASSWORD_OP) {
      continue;
    }

    if (!Question->ValueChanged) {
      continue;
    }

    if (Question->HiiValue.Type == EFI_IFR_TYPE_BUFFER) {
      TypeValue = (EFI_IFR_TYPE_VALUE *) Question->BufferValue;
    } else {
      TypeValue = &Question->HiiValue.Value;
    }

    if (IS_IMPORT_STATEMENT (Question)) {
      ConfigAccess = Question->ImportInfo->SrcFormSet->ConfigAccess;
      QuestionId   = Question->ImportInfo->SrcStatement->QuestionId;
      if (Question->HiiValue.Type == EFI_IFR_TYPE_STRING) {
        TypeValue = &Question->ImportInfo->SrcStatement->HiiValue.Value;
        HiiCopyStringByStringId (
          FormSet->HiiHandle,
          Question->HiiValue.Value.string,
          Question->ImportInfo->SrcFormSet->HiiHandle,
          &TypeValue->string
          );
      }
      mSystemLevelFormSet->HiiHandle = Question->ImportInfo->SrcFormSet->HiiHandle;
    } else {
      ConfigAccess = FormSet->ConfigAccess;
      QuestionId   = Question->QuestionId;
    }

    ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
    ConfigAccess->Callback (
                    ConfigAccess,
                    EFI_BROWSER_ACTION_CHANGED,
                    QuestionId,
                    Question->HiiValue.Type,
                    TypeValue,
                    &ActionRequest
                    );
    mSystemLevelFormSet->HiiHandle = OrgHiiHandle;
    if (IS_IMPORT_STATEMENT (Question)) {
      if (Question->HiiValue.Type == EFI_IFR_TYPE_STRING) {
        HiiCopyStringByStringId (
          Question->ImportInfo->SrcFormSet->HiiHandle,
          TypeValue->string,
          FormSet->HiiHandle,
          &Question->HiiValue.Value.string
          );
      }
    }
  }
}

/**
  Validate the FormSet. If the formset is not validate, remove it from the list.

  @param  FormSet                The input FormSet which need to validate.

  @retval TRUE                   The handle is validate.
  @retval FALSE                  The handle is invalidate.

**/
BOOLEAN
ValidateFormSet (
  FORM_BROWSER_FORMSET    *FormSet
  )
{
  EFI_HII_HANDLE          *HiiHandles;
  UINTN                   Index;
  BOOLEAN                 Find;

  ASSERT (FormSet != NULL);
  if (FormSet == NULL) {
    return FALSE;
  }
  Find = FALSE;
  //
  // Get all the Hii handles
  //
  HiiHandles = HiiGetHiiHandles (NULL);
  ASSERT (HiiHandles != NULL);
  if (HiiHandles == NULL) {
    return FALSE;
  }

  //
  // Search for formset of each class type
  //
  for (Index = 0; HiiHandles[Index] != NULL; Index++) {
    if (HiiHandles[Index] == FormSet->HiiHandle) {
      Find = TRUE;
      break;
    }
  }

  if (!Find) {
    CleanBrowserStorage(FormSet);
    RemoveEntryList (&FormSet->Link);
    DestroyFormSet (FormSet);
  }

  FreePool (HiiHandles);

  return Find;
}

/**
  Check whether need to enable the reset flag in form level.
  Also clean all ValueChanged flag in question.

  @param  SetFlag                Whether need to set the Reset Flag.
  @param  Form                   Form data structure.

**/
VOID
UpdateFlagForForm (
  IN BOOLEAN                          SetFlag,
  IN FORM_BROWSER_FORM                *Form
  )
{
  LIST_ENTRY              *Link;
  FORM_BROWSER_STATEMENT  *Question;
  BOOLEAN                 FindOne;

  FindOne = FALSE;
  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);

    if (SetFlag && Question->ValueChanged && ((Question->QuestionFlags & EFI_IFR_FLAG_RESET_REQUIRED) != 0)) {
      gResetRequired = TRUE;
    }

    if (Question->ValueChanged) {
      Question->ValueChanged = FALSE;
    }

    Link = GetNextNode (&Form->StatementListHead, Link);
  }
}

/**
  Check whether need to enable the reset flag.
  Also clean ValueChanged flag for all statements.

  Form level or formset level, only one.

  @param  SetFlag                Whether need to set the Reset Flag.
  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.

**/
VOID
ValueChangeResetFlagUpdate (
  IN BOOLEAN                          SetFlag,
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form
  )
{
  FORM_BROWSER_FORM       *CurrentForm;
  LIST_ENTRY              *Link;

  //
  // Form != NULL means only check form level.
  //
  if (Form != NULL) {
    UpdateFlagForForm(SetFlag, Form);
    return;
  }

  Link = GetFirstNode (&FormSet->FormListHead);
  while (!IsNull (&FormSet->FormListHead, Link)) {
    CurrentForm = FORM_BROWSER_FORM_FROM_LINK (Link);
    Link = GetNextNode (&FormSet->FormListHead, Link);

    UpdateFlagForForm(SetFlag, CurrentForm);
  }
}

/**
  Discard data based on the input setting scope (Form, FormSet or System).

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  SettingScope           Setting Scope for Discard action.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_UNSUPPORTED        Unsupport SettingScope.

**/
EFI_STATUS
DiscardForm (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN BROWSER_SETTING_SCOPE            SettingScope
  )
{
  LIST_ENTRY                   *Link;
  FORMSET_STORAGE              *Storage;
  FORM_BROWSER_CONFIG_REQUEST  *ConfigInfo;
  FORM_BROWSER_FORMSET         *LocalFormSet;
  FORM_BROWSER_FORMSET         *OldFormSet;

  //
  // Check the supported setting level.
  //
  if (SettingScope >= MaxLevel) {
    return EFI_UNSUPPORTED;
  }

  if (SettingScope == FormLevel && IsNvUpdateRequiredForForm (Form)) {
    ConfigInfo = NULL;
    Link = GetFirstNode (&Form->ConfigRequestHead);
    while (!IsNull (&Form->ConfigRequestHead, Link)) {
      ConfigInfo = FORM_BROWSER_CONFIG_REQUEST_FROM_LINK (Link);
      Link = GetNextNode (&Form->ConfigRequestHead, Link);

      if (ConfigInfo->Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
        continue;
      }

      //
      // Skip if there is no RequestElement
      //
      if (ConfigInfo->ElementCount == 0) {
        continue;
      }

      //
      // Prepare <ConfigResp>
      //
      SynchronizeStorage(FormSet, ConfigInfo->Storage, ConfigInfo->ConfigRequest, FALSE);

      //
      // Call callback with Changed type to inform the driver.
      //
      SendDiscardInfoToDriver (FormSet, Form);
    }

    ValueChangeResetFlagUpdate (FALSE, NULL, Form);
  } else if (SettingScope == FormSetLevel && IsNvUpdateRequiredForFormSet (FormSet)) {

    //
    // Discard Buffer storage or Name/Value storage
    //
    Link = GetFirstNode (&FormSet->StorageListHead);
    while (!IsNull (&FormSet->StorageListHead, Link)) {
      Storage = FORMSET_STORAGE_FROM_LINK (Link);
      Link = GetNextNode (&FormSet->StorageListHead, Link);

      if (Storage->BrowserStorage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
        continue;
      }

      //
      // Skip if there is no RequestElement
      //
      if (Storage->ElementCount == 0) {
        continue;
      }

      SynchronizeStorage(FormSet, Storage->BrowserStorage, Storage->ConfigRequest, FALSE);
    }

    Link = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, Link)) {
      Form = FORM_BROWSER_FORM_FROM_LINK (Link);
      Link = GetNextNode (&FormSet->FormListHead, Link);

      //
      // Call callback with Changed type to inform the driver.
      //
      SendDiscardInfoToDriver (FormSet, Form);
    }

    ValueChangeResetFlagUpdate(FALSE, FormSet, NULL);
  } else if (SettingScope == SystemLevel) {
    //
    // System Level Discard.
    //
    OldFormSet = mSystemLevelFormSet;

    //
    // Discard changed value for each FormSet in the maintain list.
    //
    Link = GetFirstNode (&gBrowserFormSetList);
    while (!IsNull (&gBrowserFormSetList, Link)) {
      LocalFormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
      Link = GetNextNode (&gBrowserFormSetList, Link);
      if (!ValidateFormSet(LocalFormSet)) {
        continue;
      }
      if (CompareGuid (LocalFormSet->ClassGuid, &mScuFormSetGuid)) {
        continue;
      }

      mSystemLevelFormSet = LocalFormSet;

      DiscardForm (LocalFormSet, NULL, FormSetLevel);
      if (!IsHiiHandleInBrowserContext (LocalFormSet->HiiHandle) && !mMultiFormSet) {
        //
        // Remove maintain backup list after discard except for the current using FormSet.
        //
        CleanBrowserStorage(LocalFormSet);
        RemoveEntryList (&LocalFormSet->Link);
        DestroyFormSet (LocalFormSet);
      }
    }

    mSystemLevelFormSet = OldFormSet;
  }

  return EFI_SUCCESS;
}

/**
  Submit data based on the input Setting level (Form, FormSet or System).

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  SettingScope           Setting Scope for Submit action.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_UNSUPPORTED        Unsupport SettingScope.

**/
EFI_STATUS
SubmitForm (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN BROWSER_SETTING_SCOPE            SettingScope
  )
{
  EFI_STATUS              Status;
  LIST_ENTRY              *Link;
  EFI_STRING              ConfigResp;
  EFI_STRING              Progress;
  BROWSER_STORAGE         *Storage;
  FORMSET_STORAGE         *FormSetStorage;
  FORM_BROWSER_FORMSET    *LocalFormSet;
  FORM_BROWSER_CONFIG_REQUEST  *ConfigInfo;

  //
  // Check the supported setting level.
  //
  if (SettingScope >= MaxLevel) {
    return EFI_UNSUPPORTED;
  }

  //
  // Validate the Form by NoSubmit check
  //
  Status = EFI_SUCCESS;
  if (SettingScope == FormLevel) {
    Status = NoSubmitCheck (FormSet, Form);
  } else if (SettingScope == FormSetLevel) {
    Status = NoSubmitCheck (FormSet, NULL);
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (SettingScope == FormLevel && IsNvUpdateRequiredForForm (Form)) {
    ConfigInfo = NULL;
    Link = GetFirstNode (&Form->ConfigRequestHead);
    while (!IsNull (&Form->ConfigRequestHead, Link)) {
      ConfigInfo = FORM_BROWSER_CONFIG_REQUEST_FROM_LINK (Link);
      Link = GetNextNode (&Form->ConfigRequestHead, Link);

      Storage = ConfigInfo->Storage;
      if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
        continue;
      }

      //
      // Skip if there is no RequestElement
      //
      if (ConfigInfo->ElementCount == 0) {
        continue;
      }

      //
      // 1. Prepare <ConfigResp>
      //
      Status = StorageToConfigResp (ConfigInfo->Storage, &ConfigResp, ConfigInfo->ConfigRequest, TRUE);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      //
      // 2. Set value to hii config routine protocol.
      //
      Status = gHiiConfigRouting->RouteConfig (
                                        gHiiConfigRouting,
                                        ConfigResp,
                                        &Progress
                                        );
      if (EFI_ERROR (Status)) {
        FreePool (ConfigResp);
        return Status;
      }

      FreePool (ConfigResp);
      //
      // 3. Config success, update storage shadow Buffer, only update the data belong to this form.
      //
      SynchronizeStorage (FormSet, ConfigInfo->Storage, ConfigInfo->ConfigRequest, TRUE);
    }

    //
    // 4. Update the NV flag.
    //
    ValueChangeResetFlagUpdate(TRUE, NULL, Form);
  } else if (SettingScope == FormSetLevel &&
             (IsNvUpdateRequiredForFormSet (FormSet) ||
              CompareGuid (FormSet->ClassGuid, &mScuFormSetGuid) ||
              CompareGuid (FormSet->ClassGuid, &mSecureBootMgrFormSetGuid))) {
    //
    // Submit Buffer storage or Name/Value storage
    //
    Link = GetFirstNode (&FormSet->StorageListHead);
    while (!IsNull (&FormSet->StorageListHead, Link)) {
      FormSetStorage = (FORMSET_STORAGE_FROM_LINK (Link));
      Storage        = FormSetStorage->BrowserStorage;
      Link = GetNextNode (&FormSet->StorageListHead, Link);

      if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
        continue;
      }

      //
      // Skip if there is no RequestElement
      //
      if (FormSetStorage->ElementCount == 0) {
        continue;
      }

      //
      // 1. Prepare <ConfigResp>
      //
      Status = StorageToConfigResp (Storage, &ConfigResp, FormSetStorage->ConfigRequest, TRUE);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      //
      // 2. Send <ConfigResp> to Routine config Protocol.
      //
      Status = gHiiConfigRouting->RouteConfig (
                                        gHiiConfigRouting,
                                        ConfigResp,
                                        &Progress
                                        );
      if (EFI_ERROR (Status)) {
        FreePool (ConfigResp);
        return Status;
      }

      FreePool (ConfigResp);
      //
      // 3. Config success, update storage shadow Buffer
      //
      SynchronizeStorage (FormSet, Storage, FormSetStorage->ConfigRequest, TRUE);
    }

    //
    // 4. Update the NV flag.
    //
    ValueChangeResetFlagUpdate(TRUE, FormSet, NULL);
  } else if (SettingScope == SystemLevel) {
    //
    // System Level Save.
    //

    //
    // Save changed value for each FormSet in the maintain list.
    //
    Link = GetFirstNode (&gBrowserFormSetList);
    while (!IsNull (&gBrowserFormSetList, Link)) {
      LocalFormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
      Link = GetNextNode (&gBrowserFormSetList, Link);
      if (!ValidateFormSet(LocalFormSet)) {
        continue;
      }
      if (CompareGuid (LocalFormSet->ClassGuid, &mScuFormSetGuid)) {
        continue;
      }

      SubmitForm (LocalFormSet, NULL, FormSetLevel);
      if (!IsHiiHandleInBrowserContext (LocalFormSet->HiiHandle) && !mMultiFormSet) {
        //
        // Remove maintain backup list after save except for the current using FormSet.
        //
        CleanBrowserStorage(LocalFormSet);
        RemoveEntryList (&LocalFormSet->Link);
        DestroyFormSet (LocalFormSet);
      }
    }

    LocalFormSet = GetScuFormset ();
    if (LocalFormSet != NULL) {
      SubmitForm (LocalFormSet, NULL, FormSetLevel);
    }
  }

  return EFI_SUCCESS;
}

/**
  Get Question default value from AltCfg string.

  @param  [in] FormSet           The Form Set.
  @param  [in,out] Question      The question.
  @param  [in] DefaultId         The default Id.

  @retval EFI_SUCCESS            Question is reset to default value.
  @retval EFI_NOT_FOUND          Storage type is EFI_HII_VARSTORE_EFI_VARIABLE or
                                 EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER.
                                 Value is not equal '='.

**/
STATIC
EFI_STATUS
GetDefaultValueFromAltCfg (
  IN     FORM_BROWSER_FORMSET             *FormSet,
  IN OUT FORM_BROWSER_STATEMENT           *Question,
  IN     UINT16                           DefaultId
  )
{
  BOOLEAN             IsBufferStorage;
  BOOLEAN             IsString;
  UINTN               Length;
  BROWSER_STORAGE     *Storage;
  CHAR16              *ConfigRequest;
  CHAR16              *Progress;
  CHAR16              *Result;
  CHAR16              *ConfigResp;
  CHAR16              *Value;
  CHAR16              *StringPtr;
  UINTN               LengthStr;
  UINT8               *Dst;
  CHAR16              TemStr[5];
  UINTN               Index;
  UINT8               DigitUint8;
  EFI_STATUS          Status;

  Status        = EFI_NOT_FOUND;
  Length        = 0;
  Dst           = NULL;
  ConfigRequest = NULL;
  Result        = NULL;
  ConfigResp    = NULL;
  Value         = NULL;
  Storage       = Question->Storage;

  if ((Storage == NULL) || (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE)) {
    return Status;
  }

  //
  // Question Value is provided by Buffer Storage or NameValue Storage
  //
  if (Question->BufferValue != NULL) {
    //
    // This Question is password or orderedlist
    //
    Dst = Question->BufferValue;
  } else {
    //
    // Other type of Questions
    //
    Dst = (UINT8 *) &Question->HiiValue.Value;
  }

  if (Storage->Type == EFI_HII_VARSTORE_BUFFER || Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
    IsBufferStorage = TRUE;
  } else {
    IsBufferStorage = FALSE;
  }
  IsString = (BOOLEAN) ((Question->HiiValue.Type == EFI_IFR_TYPE_STRING) ?  TRUE : FALSE);

  //
  // <ConfigRequest> ::= <ConfigHdr> + <BlockName> ||
  //                   <ConfigHdr> + "&" + <VariableName>
  //
  if (IsBufferStorage) {
    Length  = StrLen (Storage->ConfigHdr);
    Length += StrLen (Question->BlockName);
  } else {
    Length  = StrLen (Storage->ConfigHdr);
    Length += StrLen (Question->VariableName) + 1;
  }
  ConfigRequest = AllocateZeroPool ((Length + 1) * sizeof (CHAR16));
  ASSERT (ConfigRequest != NULL);
  if (ConfigRequest == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  StrCpy (ConfigRequest, Storage->ConfigHdr);
  if (IsBufferStorage) {
    StrCat (ConfigRequest, Question->BlockName);
  } else {
    StrCat (ConfigRequest, L"&");
    StrCat (ConfigRequest, Question->VariableName);
  }

  Status = gHiiConfigRouting->ExtractConfig (
                                    gHiiConfigRouting,
                                    ConfigRequest,
                                    &Progress,
                                    &Result
                                    );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Call ConfigRouting GetAltCfg(ConfigRoute, <ConfigResponse>, Guid, Name, DevicePath, AltCfgId, AltCfgResp)
  //    Get the default configuration string according to the default ID.
  //
  Status = gHiiConfigRouting->GetAltConfig (
                                gHiiConfigRouting,
                                Result,
                                &Storage->Guid,
                                Storage->Name,
                                NULL,
                                &DefaultId,  // it can be NULL to get the current setting.
                                &ConfigResp
                              );

  //
  // The required setting can't be found. So, it is not required to be validated and set.
  //
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if (ConfigResp == NULL) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  //
  // Skip <ConfigRequest>
  //
  if (IsBufferStorage) {
    Value = StrStr (ConfigResp, L"&VALUE");
    if (Value == NULL) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
    ASSERT (Value != NULL);
    //
    // Skip "&VALUE"
    //
    Value = Value + 6;
  } else {
    Value = StrStr (ConfigResp, Question->VariableName);
    ASSERT (Value != NULL);

    Value = Value + StrLen (Question->VariableName);
  }
  if (*Value != '=') {
    Status = EFI_NOT_FOUND;
    goto Done;
  }
  //
  // Skip '=', point to value
  //
  Value = Value + 1;

  //
  // Suppress <AltResp> if any
  //
  StringPtr = Value;
  while (*StringPtr != L'\0' && *StringPtr != L'&') {
    StringPtr++;
  }
  *StringPtr = L'\0';

  LengthStr = StrLen (Value);
  if (!IsBufferStorage && IsString) {
    StringPtr = (CHAR16 *) Dst;
    ZeroMem (TemStr, sizeof (TemStr));
    for (Index = 0; Index < LengthStr; Index += 4) {
      StrnCpy (TemStr, Value + Index, 4);
      StringPtr[Index / 4] = (CHAR16) StrHexToUint64 (TemStr);
    }
    //
    // Add tailing L'\0' character
    //
    StringPtr[Index / 4] = L'\0';
  } else {
    ZeroMem (TemStr, sizeof (TemStr));
    for (Index = 0; Index < LengthStr; Index ++) {
      TemStr[0] = Value[LengthStr - Index - 1];
      DigitUint8 = (UINT8) StrHexToUint64 (TemStr);
      if ((Index & 1) == 0) {
        Dst [Index / 2] = DigitUint8;
      } else {
        Dst [Index / 2] = (UINT8) ((DigitUint8 << 4) + Dst [Index / 2]);
      }
    }
  }

Done:
  if (ConfigRequest != NULL){
    FreePool (ConfigRequest);
  }

  if (ConfigResp != NULL) {
    FreePool (ConfigResp);
  }

  if (Result != NULL) {
    FreePool (Result);
  }

  return Status;
}

/**
  Get default Id value used for browser.

  @param  [in] DefaultId              The default id value used by HII.

  @retval Browser used default value.

**/
STATIC
INTN
GetDefaultIdForCallBack (
  UINTN DefaultId
  )
{
  if (DefaultId == EFI_HII_DEFAULT_CLASS_STANDARD) {
    return EFI_BROWSER_ACTION_DEFAULT_STANDARD;
  } else if (DefaultId == EFI_HII_DEFAULT_CLASS_MANUFACTURING) {
    return EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING;
  } else if (DefaultId == EFI_HII_DEFAULT_CLASS_SAFE) {
    return EFI_BROWSER_ACTION_DEFAULT_SAFE;
  } else if (DefaultId >= EFI_HII_DEFAULT_CLASS_PLATFORM_BEGIN && DefaultId < EFI_HII_DEFAULT_CLASS_PLATFORM_BEGIN + 0x1000) {
    return EFI_BROWSER_ACTION_DEFAULT_PLATFORM + DefaultId - EFI_HII_DEFAULT_CLASS_PLATFORM_BEGIN;
  } else if (DefaultId >= EFI_HII_DEFAULT_CLASS_HARDWARE_BEGIN && DefaultId < EFI_HII_DEFAULT_CLASS_HARDWARE_BEGIN + 0x1000) {
    return EFI_BROWSER_ACTION_DEFAULT_HARDWARE + DefaultId - EFI_HII_DEFAULT_CLASS_HARDWARE_BEGIN;
  } else if (DefaultId >= EFI_HII_DEFAULT_CLASS_FIRMWARE_BEGIN && DefaultId < EFI_HII_DEFAULT_CLASS_FIRMWARE_BEGIN + 0x1000) {
    return EFI_BROWSER_ACTION_DEFAULT_FIRMWARE + DefaultId - EFI_HII_DEFAULT_CLASS_FIRMWARE_BEGIN;
  } else {
    return -1;
  }
}



/**
  Return data element in an Array by its Index.

  @param  Array                  The data array.
  @param  Type                   Type of the data in this array.
  @param  Index                  Zero based index for data in this array.

  @retval Value                  The data to be returned

**/
UINT64
GetArrayData (
  IN VOID                     *Array,
  IN UINT8                    Type,
  IN UINTN                    Index
  )
{
  UINT64 Data;

  ASSERT (Array != NULL);

  Data = 0;
  switch (Type) {
  case EFI_IFR_TYPE_NUM_SIZE_8:
    Data = (UINT64) *(((UINT8 *) Array) + Index);
    break;

  case EFI_IFR_TYPE_NUM_SIZE_16:
    Data = (UINT64) *(((UINT16 *) Array) + Index);
    break;

  case EFI_IFR_TYPE_NUM_SIZE_32:
    Data = (UINT64) *(((UINT32 *) Array) + Index);
    break;

  case EFI_IFR_TYPE_NUM_SIZE_64:
    Data = (UINT64) *(((UINT64 *) Array) + Index);
    break;

  default:
    break;
  }

  return Data;
}


/**
  Set value of a data element in an Array by its Index.

  @param  Array                  The data array.
  @param  Type                   Type of the data in this array.
  @param  Index                  Zero based index for data in this array.
  @param  Value                  The value to be set.

**/
VOID
SetArrayData (
  IN VOID                     *Array,
  IN UINT8                    Type,
  IN UINTN                    Index,
  IN UINT64                   Value
  )
{

  ASSERT (Array != NULL);

  switch (Type) {
  case EFI_IFR_TYPE_NUM_SIZE_8:
    *(((UINT8 *) Array) + Index) = (UINT8) Value;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_16:
    *(((UINT16 *) Array) + Index) = (UINT16) Value;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_32:
    *(((UINT32 *) Array) + Index) = (UINT32) Value;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_64:
    *(((UINT64 *) Array) + Index) = (UINT64) Value;
    break;

  default:
    break;
  }
}

/**
  Check whether this value already in the array, if yes, return the index.

  @param  Array                  The data array.
  @param  Type                   Type of the data in this array.
  @param  Value                  The value to be find.
  @param  Index                  The index in the array which has same value with Value.

  @retval   TRUE Found the value in the array.
  @retval   FALSE Not found the value.

**/
BOOLEAN
FindArrayData (
  IN VOID                     *Array,
  IN UINT8                    Type,
  IN UINT64                   Value,
  OUT UINTN                   *Index OPTIONAL
  )
{
  UINTN  Count;
  UINT64 TmpValue;
  UINT64 ValueComp;

  ASSERT (Array != NULL);

  Count    = 0;
  TmpValue = 0;

  switch (Type) {
  case EFI_IFR_TYPE_NUM_SIZE_8:
    ValueComp = (UINT8) Value;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_16:
    ValueComp = (UINT16) Value;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_32:
    ValueComp = (UINT32) Value;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_64:
    ValueComp = (UINT64) Value;
    break;

  default:
    ValueComp = 0;
    break;
  }

  while ((TmpValue = GetArrayData (Array, Type, Count)) != 0) {
    if (ValueComp == TmpValue) {
      if (Index != NULL) {
        *Index = Count;
      }
      return TRUE;
    }

    Count ++;
  }

  return FALSE;
}

/**
  Search an Option of a Question by its value.

  @param  Question               The Question
  @param  OptionValue            Value for Option to be searched.

  @retval Pointer                Pointer to the found Option.
  @retval NULL                   Option not found.

**/
QUESTION_OPTION *
ValueToOption (
  IN FORM_BROWSER_STATEMENT   *Question,
  IN EFI_HII_VALUE            *OptionValue
  )
{
  LIST_ENTRY       *Link;
  QUESTION_OPTION  *Option;
  INTN             Result;

  Link = GetFirstNode (&Question->OptionListHead);
  while (!IsNull (&Question->OptionListHead, Link)) {
    Option = QUESTION_OPTION_FROM_LINK (Link);

    if ((CompareValue (&Option->Value, OptionValue, &Result, NULL) == EFI_SUCCESS) && (Result == 0)) {
      //
      // Check the suppressif condition, only a valid option can be return.
      //
      if ((Option->SuppressExpression == NULL) ||
          ((EvaluateExpressionList(Option->SuppressExpression, FALSE, NULL, NULL) == ExpressFalse))) {
        return Option;
      }
    }

    Link = GetNextNode (&Question->OptionListHead, Link);
  }

  return NULL;
}


/**
  Reset Question to its default value.

  @param  [in] FormSet           The Form Set.
  @param  [in] Form              The Form.
  @param  [in] Question          The question.
  @param  [in] DefaultId         The Class of the default.

  @retval EFI_SUCCESS            Question is reset to default value.

**/
EFI_STATUS
GetQuestionDefault (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN FORM_BROWSER_STATEMENT           *Question,
  IN UINT16                           DefaultId
  )
{
  EFI_STATUS              Status;
  LIST_ENTRY              *Link;
  QUESTION_DEFAULT        *Default;
  QUESTION_OPTION         *Option;
  EFI_HII_VALUE           *HiiValue;
  EFI_STRING              StrValue;
  UINT32                  Index;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;
  EFI_BROWSER_ACTION_REQUEST      ActionRequest;
  INTN                            Action;
  CHAR16                          *NewString;
  EFI_QUESTION_ID                 QuestionId;
  EFI_HII_HANDLE                  OrgHiiHandle;

  Status   = EFI_NOT_FOUND;
  StrValue = NULL;

  //
  // Statement don't have storage, skip them
  //
  if (Question->QuestionId == 0) {
    return Status;
  }

  //
  // There are Five ways to specify default value for a Question:
  //  1, use call back function (highest priority)
  //  2, use ExtractConfig function
  //  3, use nested EFI_IFR_DEFAULT
  //  4, set flags of EFI_ONE_OF_OPTION (provide Standard and Manufacturing default)
  //  5, set flags of EFI_IFR_CHECKBOX (provide Standard and Manufacturing default)
  //     (lowest priority)
  //
  HiiValue = &Question->HiiValue;

  //
  // Get Question default value from call back function.
  //
  ConfigAccess = FormSet->ConfigAccess;
  Action = GetDefaultIdForCallBack (DefaultId);
  if ((Action > 0) && ((Question->QuestionFlags & EFI_IFR_FLAG_CALLBACK) != 0) && (ConfigAccess != NULL)) {
    QuestionId   = Question->QuestionId;
    OrgHiiHandle = mSystemLevelFormSet->HiiHandle;
    if (IS_IMPORT_STATEMENT (Question)) {
      ConfigAccess = Question->ImportInfo->SrcFormSet->ConfigAccess;
      QuestionId   = Question->ImportInfo->SrcStatement->QuestionId;
      if (Question->HiiValue.Type == EFI_IFR_TYPE_STRING) {
        HiiValue = &Question->ImportInfo->SrcStatement->HiiValue;
        HiiCopyStringByStringId (
          FormSet->HiiHandle,
          Question->HiiValue.Value.string,
          Question->ImportInfo->SrcFormSet->HiiHandle,
          &HiiValue->Value.string
          );
      }
      mSystemLevelFormSet->HiiHandle = Question->ImportInfo->SrcFormSet->HiiHandle;
    }
    ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
    Status = ConfigAccess->Callback (
                             ConfigAccess,
                             Action,
                             QuestionId,
                             HiiValue->Type,
                             &HiiValue->Value,
                             &ActionRequest
                             );
    mSystemLevelFormSet->HiiHandle = OrgHiiHandle;
    if (!EFI_ERROR (Status)) {
      if (HiiValue->Type == EFI_IFR_TYPE_STRING) {
        if (IS_IMPORT_STATEMENT (Question)) {
          HiiCopyStringByStringId (
            Question->ImportInfo->SrcFormSet->HiiHandle,
            HiiValue->Value.string,
            FormSet->HiiHandle,
            &Question->HiiValue.Value.string
            );
        }

        NewString = GetString (Question->HiiValue.Value.string, FormSet->HiiHandle);
        ASSERT (NewString != NULL);
        if (NewString == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }

        ASSERT (StrLen (NewString) * sizeof (CHAR16) <= Question->StorageWidth);
        if (StrLen (NewString) * sizeof (CHAR16) <= Question->StorageWidth) {
          CopyMem (Question->BufferValue, NewString, StrSize (NewString));
        } else {
          CopyMem (Question->BufferValue, NewString, Question->StorageWidth);
        }

        FreePool (NewString);
      }
      return Status;
    }
  }

  //
  // Get default value from altcfg string.
  //
  if (ConfigAccess != NULL) {
    Status = GetDefaultValueFromAltCfg (FormSet, Question, DefaultId);
    if (!EFI_ERROR (Status)) {
        return Status;
    }
  }

  //
  // EFI_IFR_DEFAULT has highest priority
  //
  if (!IsListEmpty (&Question->DefaultListHead)) {
    Link = GetFirstNode (&Question->DefaultListHead);
    while (!IsNull (&Question->DefaultListHead, Link)) {
      Default = QUESTION_DEFAULT_FROM_LINK (Link);

      if (Default->DefaultId == DefaultId) {
        if (Default->ValueExpression != NULL) {
          //
          // Default is provided by an Expression, evaluate it
          //
          Status = EvaluateExpression (FormSet, Form, Default->ValueExpression);
          if (EFI_ERROR (Status)) {
            return Status;
          }

          if (Default->ValueExpression->Result.Type == EFI_IFR_TYPE_BUFFER) {
            ASSERT (HiiValue->Type == EFI_IFR_TYPE_BUFFER && Question->BufferValue != NULL);
            if (Question->StorageWidth > Default->ValueExpression->Result.BufferLen) {
              CopyMem (Question->HiiValue.Buffer, Default->ValueExpression->Result.Buffer, Default->ValueExpression->Result.BufferLen);
              Question->HiiValue.BufferLen = Default->ValueExpression->Result.BufferLen;
            } else {
              CopyMem (Question->HiiValue.Buffer, Default->ValueExpression->Result.Buffer, Question->StorageWidth);
              Question->HiiValue.BufferLen = Question->StorageWidth;
            }
            FreePool (Default->ValueExpression->Result.Buffer);
          }
          HiiValue->Type = Default->ValueExpression->Result.Type;
          CopyMem (&HiiValue->Value, &Default->ValueExpression->Result.Value, sizeof (EFI_IFR_TYPE_VALUE));
        } else {
          //
          // Default value is embedded in EFI_IFR_DEFAULT
          //
          CopyMem (HiiValue, &Default->Value, sizeof (EFI_HII_VALUE));
        }

        if (HiiValue->Type == EFI_IFR_TYPE_STRING) {
          StrValue = HiiGetString (FormSet->HiiHandle, HiiValue->Value.string, NULL);
          if (StrValue == NULL) {
            return EFI_NOT_FOUND;
          }
          if (Question->StorageWidth > StrSize (StrValue)) {
            CopyMem (Question->BufferValue, StrValue, StrSize (StrValue));
          } else {
            CopyMem (Question->BufferValue, StrValue, Question->StorageWidth);
          }
        }

        return EFI_SUCCESS;
      }

      Link = GetNextNode (&Question->DefaultListHead, Link);
    }
  }

  //
  // EFI_ONE_OF_OPTION
  //
  if ((Question->Operand == EFI_IFR_ONE_OF_OP) && !IsListEmpty (&Question->OptionListHead)) {
    if (DefaultId <= EFI_HII_DEFAULT_CLASS_MANUFACTURING)  {
      //
      // OneOfOption could only provide Standard and Manufacturing default
      //
      Link = GetFirstNode (&Question->OptionListHead);
      while (!IsNull (&Question->OptionListHead, Link)) {
        Option = QUESTION_OPTION_FROM_LINK (Link);
        Link = GetNextNode (&Question->OptionListHead, Link);

        if ((Option->SuppressExpression != NULL) &&
            EvaluateExpressionList(Option->SuppressExpression, FALSE, NULL, NULL) != ExpressFalse) {
          continue;
        }

        if (((DefaultId == EFI_HII_DEFAULT_CLASS_STANDARD) && ((Option->Flags & EFI_IFR_OPTION_DEFAULT) != 0)) ||
            ((DefaultId == EFI_HII_DEFAULT_CLASS_MANUFACTURING) && ((Option->Flags & EFI_IFR_OPTION_DEFAULT_MFG) != 0))
           ) {
          CopyMem (HiiValue, &Option->Value, sizeof (EFI_HII_VALUE));

          return EFI_SUCCESS;
        }
      }
    }
  }

  //
  // EFI_IFR_CHECKBOX - lowest priority
  //
  if (Question->Operand == EFI_IFR_CHECKBOX_OP) {
    if (DefaultId <= EFI_HII_DEFAULT_CLASS_MANUFACTURING)  {
      //
      // Checkbox could only provide Standard and Manufacturing default
      //
      if (((DefaultId == EFI_HII_DEFAULT_CLASS_STANDARD) && ((Question->Flags & EFI_IFR_CHECKBOX_DEFAULT) != 0)) ||
          ((DefaultId == EFI_HII_DEFAULT_CLASS_MANUFACTURING) && ((Question->Flags & EFI_IFR_CHECKBOX_DEFAULT_MFG) != 0))
         ) {

        SetValueBool(HiiValue, TRUE);
      } else {
        SetValueBool(HiiValue, FALSE);
      }

      return EFI_SUCCESS;
    }
  }

  //
  // For Questions without default
  //
  Status = EFI_NOT_FOUND;
  switch (Question->Operand) {
  case EFI_IFR_NUMERIC_OP:
    //
    // Take minimum value as numeric default value
    //
    if ((HiiValue->Value.u64 < Question->Minimum) || (HiiValue->Value.u64 > Question->Maximum)) {
      HiiValue->Value.u64 = Question->Minimum;
      Status = EFI_SUCCESS;
    }
    break;

  case EFI_IFR_ONE_OF_OP:
    //
    // Take first oneof option as oneof's default value
    //
    if (ValueToOption (Question, HiiValue) == NULL) {
      Link = GetFirstNode (&Question->OptionListHead);
      while (!IsNull (&Question->OptionListHead, Link)) {
        Option = QUESTION_OPTION_FROM_LINK (Link);
        Link = GetNextNode (&Question->OptionListHead, Link);

        if ((Option->SuppressExpression != NULL) &&
            EvaluateExpressionList(Option->SuppressExpression, FALSE, NULL, NULL) != ExpressFalse) {
          continue;
        }

        CopyMem (HiiValue, &Option->Value, sizeof (EFI_HII_VALUE));
        Status = EFI_SUCCESS;
        break;
      }
    }
    break;

  case EFI_IFR_ORDERED_LIST_OP:
    //
    // Take option sequence in IFR as ordered list's default value
    //
    Index = 0;
    Link = GetFirstNode (&Question->OptionListHead);
    while (!IsNull (&Question->OptionListHead, Link)) {
      Status = EFI_SUCCESS;
      Option = QUESTION_OPTION_FROM_LINK (Link);
      Link = GetNextNode (&Question->OptionListHead, Link);

      if ((Option->SuppressExpression != NULL) &&
          EvaluateExpressionList(Option->SuppressExpression, FALSE, NULL, NULL) != ExpressFalse) {
        continue;
      }

      SetArrayData (Question->BufferValue, Question->ValueType, Index, Option->Value.Value.u64);

      Index++;
      if (Index >= Question->MaxContainers) {
        break;
      }
    }
    break;

  default:
    break;
  }

  return Status;
}

/**
  Check if the form has a question which default value is from default value expression.

  @param[in] Form         The Form.
  @param[in] DefaultId    The Class of the default.

  @retval TRUE            The form has a question which default value is from default value expression.
  @retval FALSE           The form has not a question which default value is from default value expression.
**/
BOOLEAN
HaveQuestionDefaultFromExpression (
  IN FORM_BROWSER_FORM                        *Form,
  IN UINT16                                   DefaultId
  )
{
  LIST_ENTRY                                  *Link;
  LIST_ENTRY                                  *DefaultLink;
  FORM_BROWSER_STATEMENT                      *Question;
  QUESTION_DEFAULT                            *Default;

  if (Form == NULL) {
    return FALSE;
  }

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
    Link     = GetNextNode (&Form->StatementListHead, Link);
    if (Question->QuestionId == 0 || IsListEmpty (&Question->DefaultListHead)) {
      continue;
    }

    DefaultLink = GetFirstNode (&Question->DefaultListHead);
    while (!IsNull (&Question->DefaultListHead, DefaultLink)) {
      Default     = QUESTION_DEFAULT_FROM_LINK (DefaultLink);
      DefaultLink = GetNextNode (&Question->DefaultListHead, DefaultLink);
      if (Default->DefaultId == DefaultId && Default->ValueExpression != NULL) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

EFI_STATUS
ExtractScuImportQuestionDefault (
  IN UINT16                                   DefaultId,
  IN BROWSER_GET_DEFAULT_VALUE                GetDefaultValueScope,
  IN BROWSER_STORAGE                          *Storage OPTIONAL,
  IN BOOLEAN                                  RetrieveValueFirst
  )
{
  FORM_BROWSER_FORMSET                        *FormSet;
  LIST_ENTRY                                  *Link;
  LIST_ENTRY                                  *FormLink;
  FORM_BROWSER_FORM                           *Form;

  Link = GetFirstNode (&gBrowserFormSetList);
  while (!IsNull (&gBrowserFormSetList, Link)) {
    FormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
    Link = GetNextNode (&gBrowserFormSetList, Link);
    if (!CompareGuid (FormSet->ClassGuid, &mScuFormSetGuid)) {
      continue;
    }

    FormLink = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, FormLink)) {
      Form     = FORM_BROWSER_FORM_FROM_LINK (FormLink);
      FormLink = GetNextNode (&FormSet->FormListHead, FormLink);

      if (HaveImportQuestion (Form)) {
        ExtractDefault (FormSet, Form, DefaultId, FormLevel, GetDefaultValueScope, Storage, RetrieveValueFirst);
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  Extract question default Value from default value expression for the questions in SCU FormSets of the maintain list.

  @param[in] DefaultId              The Class of the default.
  @param[in] GetDefaultValueScope   Get default value scope.
  @param[in] Storage                Get default value only for this storage.
  @param[in] RetrieveValueFirst     Whether call the retrieve call back to get the initial value before get default value.

**/
VOID
ExtractScuQuestionDefaultFromExpression (
  IN UINT16                                   DefaultId,
  IN BROWSER_GET_DEFAULT_VALUE                GetDefaultValueScope,
  IN BROWSER_STORAGE                          *Storage OPTIONAL,
  IN BOOLEAN                                  RetrieveValueFirst
  )
{
  LIST_ENTRY                                  *Link;
  LIST_ENTRY                                  *FormLink;
  FORM_BROWSER_FORMSET                        *OldFormSet;
  FORM_BROWSER_FORMSET                        *FormSet;
  FORM_BROWSER_FORM                           *Form;
  EFI_BROWSER_ACTION_REQUEST                  ActionRequest;
  BOOLEAN                                     SyncToScuDriver;

  OldFormSet = mSystemLevelFormSet;

  Link = GetFirstNode (&gBrowserFormSetList);
  while (!IsNull (&gBrowserFormSetList, Link)) {
    FormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
    Link    = GetNextNode (&gBrowserFormSetList, Link);
    if (!ValidateFormSet(FormSet) ||
        !CompareGuid (FormSet->ClassGuid, &mScuFormSetGuid)) {
      continue;
    }

    mSystemLevelFormSet = FormSet;
    SyncToScuDriver     = FALSE;

    FormLink = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, FormLink)) {
      Form     = FORM_BROWSER_FORM_FROM_LINK (FormLink);
      FormLink = GetNextNode (&FormSet->FormListHead, FormLink);
      if (HaveQuestionDefaultFromExpression (Form, DefaultId)) {
        ExtractDefault (FormSet, Form, DefaultId, FormLevel, GetDefaultValueScope, Storage, RetrieveValueFirst);
        SyncToScuDriver = TRUE;
      }
    }

    if (SyncToScuDriver && FormSet->ConfigAccess != NULL) {
      FormSet->ConfigAccess->Callback (FormSet->ConfigAccess, EFI_BROWSER_ACTION_FORM_CLOSE, 0, 0, NULL, &ActionRequest);
    }
  }

  mSystemLevelFormSet = OldFormSet;
}

/**
  Reset Questions to their initial value or default value in a Form, Formset or System.

  GetDefaultValueScope parameter decides which questions will reset
  to its default value.

  @param  [in] FormSet                FormSet data structure.
  @param  [in] Form                   Form data structure.
  @param  [in] DefaultId              The Class of the default.
  @param  [in] SettingScope           Setting Scope for Default action.
  @param  [in] GetDefaultValueScope   Get default value scope.
  @param  [in] Storage                Get default value only for this storage.
  @param  [in] RetrieveValueFirst     Whether call the retrieve call back to
                                      get the initial value before get default
                                      value.

  @retval EFI_SUCCESS                 The function completed successfully.
  @retval EFI_INVALID_PARAMETER       Unsupport SettingScope.

**/
EFI_STATUS
ExtractDefault (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN UINT16                           DefaultId,
  IN BROWSER_SETTING_SCOPE            SettingScope,
  IN BROWSER_GET_DEFAULT_VALUE        GetDefaultValueScope,
  IN BROWSER_STORAGE                  *Storage OPTIONAL,
  IN BOOLEAN                          RetrieveValueFirst
  )
{
  EFI_STATUS              Status;
  LIST_ENTRY              *FormLink;
  LIST_ENTRY              *Link;
  FORM_BROWSER_STATEMENT  *Question;
  FORM_BROWSER_FORMSET    *LocalFormSet;
  FORM_BROWSER_FORMSET    *OldFormSet;

  Status = EFI_SUCCESS;

  //
  // Check the supported setting level.
  //
  if (SettingScope >= MaxLevel || GetDefaultValueScope >= GetDefaultForMax) {
    return EFI_INVALID_PARAMETER;
  }

  if (GetDefaultValueScope == GetDefaultForStorage && Storage == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (SettingScope == FormLevel) {
    //
    // Extract Form default
    //
    Link = GetFirstNode (&Form->StatementListHead);
    while (!IsNull (&Form->StatementListHead, Link)) {
      Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
      Link = GetNextNode (&Form->StatementListHead, Link);

      //
      // If get default value only for this storage, check the storage first.
      //
      if ((GetDefaultValueScope == GetDefaultForStorage) && (Question->Storage != Storage)) {
        continue;
      }

      //
      // If get default value only for no storage question, just skip the question which has storage.
      //
      if ((GetDefaultValueScope == GetDefaultForNoStorage) && (Question->Storage != NULL)) {
        continue;
      }

      //
      // If Question is disabled, don't reset it to default
      //
      if (Question->Expression != NULL) {
        if (EvaluateExpressionList(Question->Expression, TRUE, FormSet, Form) == ExpressDisable) {
          continue;
        }
      }

      if (RetrieveValueFirst) {
        //
        // Call the Retrieve call back to get the initial question value.
        //
        Status = ProcessRetrieveForQuestion(FormSet->ConfigAccess, Question, FormSet);
      }

      //
      // If not request to get the initial value or get initial value fail, then get default value.
      //
      if (!RetrieveValueFirst || EFI_ERROR (Status)) {
        Status = GetQuestionDefault (FormSet, Form, Question, DefaultId);
        if (EFI_ERROR (Status)) {
          continue;
        }
      }

      //
      // Synchronize Buffer storage's Edit buffer
      //
      if ((Question->Storage != NULL) &&
          (Question->Storage->Type != EFI_HII_VARSTORE_EFI_VARIABLE)) {
        SetQuestionValue (FormSet, Form, Question, GetSetValueWithEditBuffer);
      }
    }
  } else if (SettingScope == FormSetLevel) {
    if (CompareGuid (FormSet->ClassGuid, &mScuFormSetGuid) &&
        GetDefaultValueScope == GetDefaultForAll &&
        DefaultId == EFI_HII_DEFAULT_CLASS_STANDARD) {
      ExtractScuImportQuestionDefault (DefaultId, GetDefaultValueScope, Storage, RetrieveValueFirst);
      ExtractScuQuestionDefaultFromExpression (DefaultId, GetDefaultValueScope, Storage, RetrieveValueFirst);
      Form = GetScuLoadDefaultForm (FormSet);
      Status = ExtractDefault (
                 FormSet,
                 Form,
                 EFI_HII_DEFAULT_CLASS_MANUFACTURING,
                 FormLevel,
                 GetDefaultForAll,
                 NULL,
                 FALSE
                 );
    } else {
      FormLink = GetFirstNode (&FormSet->FormListHead);
      while (!IsNull (&FormSet->FormListHead, FormLink)) {
        Form = FORM_BROWSER_FORM_FROM_LINK (FormLink);
        ExtractDefault (FormSet, Form, DefaultId, FormLevel, GetDefaultValueScope, Storage, RetrieveValueFirst);
        FormLink = GetNextNode (&FormSet->FormListHead, FormLink);
      }
    }
  } else if (SettingScope == SystemLevel) {
    //
    // Preload all Hii formset.
    //
    if (!mBrowserScopeFirstSet) {
      LoadAllHiiFormset();
    }

    LocalFormSet = GetScuFormset ();
    if (LocalFormSet != NULL) {
      ExtractDefault (LocalFormSet, NULL, DefaultId, FormSetLevel, GetDefaultValueScope, Storage, RetrieveValueFirst);
    }

    OldFormSet = mSystemLevelFormSet;

    //
    // Set Default Value for each FormSet in the maintain list.
    //
    Link = GetFirstNode (&gBrowserFormSetList);
    while (!IsNull (&gBrowserFormSetList, Link)) {
      LocalFormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
      Link = GetNextNode (&gBrowserFormSetList, Link);
      if (!ValidateFormSet(LocalFormSet)) {
        continue;
      }
      if (CompareGuid (LocalFormSet->ClassGuid, &mScuFormSetGuid)) {
        continue;
      }

      mSystemLevelFormSet = LocalFormSet;

      ExtractDefault (LocalFormSet, NULL, DefaultId, FormSetLevel, GetDefaultValueScope, Storage, RetrieveValueFirst);
    }

    mSystemLevelFormSet = OldFormSet;
  }

  return EFI_SUCCESS;
}


/**
  Validate whether this question's value has changed.

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  Question               Question to be initialized.
  @param  GetValueFrom           Where to get value, may from editbuffer, buffer or hii driver.

  @retval TRUE                   Question's value has changed.
  @retval FALSE                  Question's value has not changed

**/
BOOLEAN
IsQuestionValueChanged (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN OUT FORM_BROWSER_STATEMENT       *Question,
  IN GET_SET_QUESTION_VALUE_WITH      GetValueFrom
  )
{
  EFI_HII_VALUE    BackUpValue;
  CHAR8            *BackUpBuffer;
  EFI_STATUS       Status;
  BOOLEAN          ValueChanged;
  UINTN            BufferWidth;

  //
  // For quetion without storage, always mark it as data not changed.
  //
  if (Question->Storage == NULL && Question->Operand != EFI_IFR_TIME_OP && Question->Operand != EFI_IFR_DATE_OP) {
    return FALSE;
  }

  BackUpBuffer = NULL;
  ValueChanged = FALSE;

  switch (Question->Operand) {
    case EFI_IFR_ORDERED_LIST_OP:
      BufferWidth  = Question->StorageWidth;
      BackUpBuffer = AllocateCopyPool (BufferWidth, Question->BufferValue);
      ASSERT (BackUpBuffer != NULL);
      break;

    case EFI_IFR_STRING_OP:
    case EFI_IFR_PASSWORD_OP:
      BufferWidth  = (UINTN) Question->Maximum * sizeof (CHAR16);
      BackUpBuffer = AllocateCopyPool (BufferWidth, Question->BufferValue);
      ASSERT (BackUpBuffer != NULL);
      break;

    default:
      BufferWidth = 0;
      break;
  }
  CopyMem (&BackUpValue, &Question->HiiValue, sizeof (EFI_HII_VALUE));

  Status = GetQuestionValue (FormSet, Form, Question, GetValueFrom);
  ASSERT_EFI_ERROR(Status);

  if (CompareMem (&BackUpValue, &Question->HiiValue, sizeof (EFI_HII_VALUE)) != 0 ||
      CompareMem (BackUpBuffer, Question->BufferValue, BufferWidth) != 0) {
    ValueChanged = TRUE;
  }

  CopyMem (&Question->HiiValue, &BackUpValue, sizeof (EFI_HII_VALUE));
  CopyMem (Question->BufferValue, BackUpBuffer, BufferWidth);

  if (BackUpBuffer != NULL) {
    FreePool (BackUpBuffer);
  }

  Question->ValueChanged = ValueChanged;

  return ValueChanged;
}

/**
  Initialize Question's Edit copy from Storage.

  @param  [in] Selection         Selection contains the information about
                                 the Selection, Form and FormSet to be displayed.
                                 Selection action may be updated in retrieve callback.
                                 If Selection is NULL, only initialize Question value.
  @param  [in] FormSet           FormSet data structure.
  @param  [in] Form              Form data structure.

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
LoadFormConfig (
  IN OUT UI_MENU_SELECTION    *Selection,
  IN FORM_BROWSER_FORMSET     *FormSet,
  IN FORM_BROWSER_FORM        *Form
  )
{
  EFI_STATUS                  Status;
  LIST_ENTRY                  *Link;
  FORM_BROWSER_STATEMENT      *Question;

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);

    //
    // Initialize local copy of Value for each Question
    //
    if (Question->Operand == EFI_IFR_PASSWORD_OP && (Question->QuestionFlags & EFI_IFR_FLAG_CALLBACK)== 0) {
      Status = GetQuestionValue (FormSet, Form, Question, GetSetValueWithHiiDriver);
    } else {
      Status = GetQuestionValue (FormSet, Form, Question, GetSetValueWithEditBuffer);
    }
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if ((Question->Operand == EFI_IFR_STRING_OP) || (Question->Operand == EFI_IFR_PASSWORD_OP)) {
      HiiSetString (FormSet->HiiHandle, Question->HiiValue.Value.string, (CHAR16*)Question->BufferValue, NULL);
    }

    Link = GetNextNode (&Form->StatementListHead, Link);
  }

  return EFI_SUCCESS;
}

/**
  Initialize Question's Edit copy from Storage for the whole FormSet.

  @param  [in] Selection         Selection contains the information about
                                 the Selection, Form and FormSet to be displayed.
                                 Selection action may be updated in retrieve callback.
                                 If Selection is NULL, only initialize Question value.
  @param  [in] FormSet           FormSet data structure.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_UNSUPPORTED        ProcessCallBackFunction()  action not unsupported

**/

EFI_STATUS
LoadFormSetConfig (
  IN OUT UI_MENU_SELECTION    *Selection,
  IN     FORM_BROWSER_FORMSET *FormSet
  )
{
  EFI_STATUS            Status;
  LIST_ENTRY            *Link;
  FORM_BROWSER_FORM     *Form;

  Link = GetFirstNode (&FormSet->FormListHead);
  while (!IsNull (&FormSet->FormListHead, Link)) {
    Form = FORM_BROWSER_FORM_FROM_LINK (Link);

    //
    // Initialize local copy of Value for each Form
    //
    Status = LoadFormConfig (Selection, FormSet, Form);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "LoadFormConfig() is fail\n"));
      return Status;
    }

    Link = GetNextNode (&FormSet->FormListHead, Link);
  }

  //
  // Finished question initialization.
  //
  FormSet->QuestionInited = TRUE;

  return EFI_SUCCESS;
}

/**
  Remove the Request element from the Config Request.

  @param  Storage                Pointer to the browser storage.
  @param  RequestElement         The pointer to the Request element.

**/
VOID
RemoveElement (
  IN OUT BROWSER_STORAGE      *Storage,
  IN     CHAR16               *RequestElement
  )
{
  CHAR16   *NewStr;
  CHAR16   *DestStr;

  ASSERT (Storage->ConfigRequest != NULL && RequestElement != NULL);

  NewStr = StrStr (Storage->ConfigRequest, RequestElement);

  if (NewStr == NULL) {
    return;
  }

  //
  // Remove this element from this ConfigRequest.
  //
  DestStr = NewStr;
  NewStr += StrLen (RequestElement);
  CopyMem (DestStr, NewStr, StrSize (NewStr));

  Storage->SpareStrLen += StrLen (RequestElement);
}

/**
  Adjust config request in storage, remove the request elements existed in the input ConfigRequest.

  @param  Storage                Pointer to the browser storage.
  @param  ConfigRequest          The pointer to the Request element.

**/
VOID
RemoveConfigRequest (
  BROWSER_STORAGE   *Storage,
  CHAR16            *ConfigRequest
  )
{
  CHAR16       *RequestElement;
  CHAR16       *NextRequestElement;
  CHAR16       *SearchKey;

  //
  // No request element in it, just return.
  //
  if (ConfigRequest == NULL) {
    return;
  }

  if (Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
    //
    // "&Name1&Name2" section for EFI_HII_VARSTORE_NAME_VALUE storage
    //
    SearchKey = L"&";
  } else {
    //
    // "&OFFSET=####&WIDTH=####" section for EFI_HII_VARSTORE_BUFFER storage
    //
    SearchKey = L"&OFFSET";
  }

  //
  // Find SearchKey storage
  //
  if (Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
    RequestElement = StrStr (ConfigRequest, L"PATH");
    ASSERT (RequestElement != NULL);
    if (RequestElement == NULL) {
      return;
    }
    RequestElement = StrStr (RequestElement, SearchKey);
  } else {
    RequestElement = StrStr (ConfigRequest, SearchKey);
  }

  while (RequestElement != NULL) {
    //
    // +1 to avoid find header itself.
    //
    NextRequestElement = StrStr (RequestElement + 1, SearchKey);

    //
    // The last Request element in configRequest string.
    //
    if (NextRequestElement != NULL) {
      //
      // Replace "&" with '\0'.
      //
      *NextRequestElement = L'\0';
    }

    RemoveElement (Storage, RequestElement);

    if (NextRequestElement != NULL) {
      //
      // Restore '&' with '\0' for later used.
      //
      *NextRequestElement = L'&';
    }

    RequestElement = NextRequestElement;
  }

  //
  // If no request element remain, just remove the ConfigRequest string.
  //
  if (StrCmp (Storage->ConfigRequest, Storage->ConfigHdr) == 0) {
    FreePool (Storage->ConfigRequest);
    Storage->ConfigRequest = NULL;
    Storage->SpareStrLen   = 0;
  }
}

/**
  Base on the current formset info, clean the ConfigRequest string in browser storage.

  @param  FormSet                Pointer of the FormSet

**/
VOID
CleanBrowserStorage (
  IN OUT FORM_BROWSER_FORMSET  *FormSet
  )
{
  LIST_ENTRY            *Link;
  FORMSET_STORAGE       *Storage;

  Link = GetFirstNode (&FormSet->StorageListHead);
  while (!IsNull (&FormSet->StorageListHead, Link)) {
    Storage = FORMSET_STORAGE_FROM_LINK (Link);
    Link = GetNextNode (&FormSet->StorageListHead, Link);

    if (Storage->BrowserStorage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
      if (Storage->ConfigRequest == NULL || Storage->BrowserStorage->ConfigRequest == NULL) {
        continue;
      }

      RemoveConfigRequest (Storage->BrowserStorage, Storage->ConfigRequest);
    } else if (Storage->BrowserStorage->Type == EFI_HII_VARSTORE_BUFFER ||
               Storage->BrowserStorage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
      if (Storage->BrowserStorage->ConfigRequest != NULL) {
        FreePool (Storage->BrowserStorage->ConfigRequest);
        Storage->BrowserStorage->ConfigRequest = NULL;
        Storage->BrowserStorage->SpareStrLen = 0;
      }
      Storage->BrowserStorage->Initialized = FALSE;
    }
  }
}

/**
  Check whether current element in the ConfigReqeust string.

  @param  BrowserStorage                Storage which includes ConfigReqeust.
  @param  RequestElement                New element need to check.

  @retval TRUE        The Element is in the ConfigReqeust string.
  @retval FALSE       The Element not in the configReqeust String.

**/
BOOLEAN
ElementValidation (
  BROWSER_STORAGE   *BrowserStorage,
  CHAR16            *RequestElement
  )
{
  return StrStr (BrowserStorage->ConfigRequest, RequestElement) != NULL ? TRUE : FALSE;
}

/**
  Append the Request element to the Config Request.

  @param  ConfigRequest          Current ConfigRequest info.
  @param  SpareStrLen            Current remain free buffer for config reqeust.
  @param  RequestElement         New Request element.

**/
VOID
AppendConfigRequest (
  IN OUT CHAR16               **ConfigRequest,
  IN OUT UINTN                *SpareStrLen,
  IN     CHAR16               *RequestElement
  )
{
  CHAR16   *NewStr;
  UINTN    StringSize;
  UINTN    StrLength;

  StrLength = StrLen (RequestElement);

  //
  // Append <RequestElement> to <ConfigRequest>
  //
  if (StrLength > *SpareStrLen) {
    //
    // Old String buffer is not sufficient for RequestElement, allocate a new one
    //
    StringSize = (*ConfigRequest != NULL) ? StrSize (*ConfigRequest) : sizeof (CHAR16);
    NewStr = AllocateZeroPool (StringSize + CONFIG_REQUEST_STRING_INCREMENTAL * sizeof (CHAR16));
    ASSERT (NewStr != NULL);
    if (NewStr == NULL) {
      return;
    }

    if (*ConfigRequest != NULL) {
      CopyMem (NewStr, *ConfigRequest, StringSize);
      FreePool (*ConfigRequest);
    }
    *ConfigRequest = NewStr;
    *SpareStrLen   = CONFIG_REQUEST_STRING_INCREMENTAL;
  }

  StrCat (*ConfigRequest, RequestElement);
  *SpareStrLen -= StrLength;
}

/**
  Adjust the config request info, remove the request elements which already in AllConfigRequest string.

  @param  Storage                Form set Storage.

  @retval TRUE                   Has element not covered by current used elements, need to continue to call ExtractConfig
  @retval FALSE                  All elements covered by current used elements.

**/
BOOLEAN
ConfigRequestAdjust (
  IN  BROWSER_STORAGE         *Storage,
  IN  CHAR16                  *Request,
  IN  BOOLEAN                 RespString
  )
{
  CHAR16       *RequestElement;
  CHAR16       *NextRequestElement;
  CHAR16       *NextElementBakup;
  UINTN        SpareBufLen;
  CHAR16       *SearchKey;
  CHAR16       *ValueKey;
  BOOLEAN      RetVal;
  CHAR16       *ConfigRequest;

  SpareBufLen    = 0;
  RetVal         = FALSE;
  NextElementBakup = NULL;
  ValueKey         = NULL;

  if (Request != NULL) {
    ConfigRequest = Request;
  } else {
    ConfigRequest = Storage->ConfigRequest;
  }

  if (Storage->ConfigRequest == NULL) {
    Storage->ConfigRequest = AllocateCopyPool (StrSize (ConfigRequest), ConfigRequest);
    return TRUE;
  }

  if (Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
    //
    // "&Name1&Name2" section for EFI_HII_VARSTORE_NAME_VALUE storage
    //
    SearchKey = L"&";
  } else {
    //
    // "&OFFSET=####&WIDTH=####" section for EFI_HII_VARSTORE_BUFFER storage
    //
    SearchKey = L"&OFFSET";
    ValueKey  = L"&VALUE";
  }

  //
  // Find SearchKey storage
  //
  if (Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
    RequestElement = StrStr (ConfigRequest, L"PATH");
    ASSERT (RequestElement != NULL);
    if (RequestElement == NULL) {
      return RetVal;
    }
    RequestElement = StrStr (RequestElement, SearchKey);
  } else {
    RequestElement = StrStr (ConfigRequest, SearchKey);
  }

  while (RequestElement != NULL) {

    //
    // +1 to avoid find header itself.
    //
    NextRequestElement = StrStr (RequestElement + 1, SearchKey);

    //
    // The last Request element in configRequest string.
    //
    if (NextRequestElement != NULL) {
      if (RespString && (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER)) {
        NextElementBakup = NextRequestElement;
        NextRequestElement = StrStr (RequestElement, ValueKey);
        ASSERT (NextRequestElement != NULL);
        if (NextRequestElement == NULL) {
          return RetVal;
        }
      }
      //
      // Replace "&" with '\0'.
      //
      *NextRequestElement = L'\0';
    } else {
      if (RespString && (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER)) {
        NextElementBakup = NextRequestElement;
        NextRequestElement = StrStr (RequestElement, ValueKey);
        ASSERT (NextRequestElement != NULL);
        if (NextRequestElement == NULL) {
          return RetVal;
        }
        //
        // Replace "&" with '\0'.
        //
        *NextRequestElement = L'\0';
      }
    }

    if (!ElementValidation (Storage, RequestElement)) {
      //
      // Add this element to the Storage->BrowserStorage->AllRequestElement.
      //
      AppendConfigRequest(&Storage->ConfigRequest, &Storage->SpareStrLen, RequestElement);
      RetVal = TRUE;
    }

    if (NextRequestElement != NULL) {
      //
      // Restore '&' with '\0' for later used.
      //
      *NextRequestElement = L'&';
    }

    if (RespString && (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER)) {
      RequestElement = NextElementBakup;
    } else {
      RequestElement = NextRequestElement;
    }
  }

  return RetVal;
}

/**

  Base on ConfigRequest info to get default value for current formset.

  ConfigRequest info include the info about which questions in current formset need to
  get default value. This function only get these questions default value.

  @param  FormSet                FormSet data structure.
  @param  Storage                Storage need to update value.
  @param  ConfigRequest          The config request string.

**/
VOID
GetDefaultForFormset (
  IN FORM_BROWSER_FORMSET    *FormSet,
  IN BROWSER_STORAGE         *Storage,
  IN CHAR16                  *ConfigRequest
  )
{
  UINT8             *BackUpBuf;
  UINTN             BufferSize;
  LIST_ENTRY        BackUpList;
  NAME_VALUE_NODE   *Node;
  LIST_ENTRY        *Link;
  LIST_ENTRY        *NodeLink;
  NAME_VALUE_NODE   *TmpNode;
  EFI_STATUS        Status;
  EFI_STRING        Progress;
  EFI_STRING        Result;

  BackUpBuf = NULL;
  InitializeListHead(&BackUpList);

  //
  // Back update the edit buffer.
  //
  if (Storage->Type == EFI_HII_VARSTORE_BUFFER ||
      (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER)) {
    BackUpBuf = AllocateCopyPool (Storage->Size, Storage->EditBuffer);
    ASSERT (BackUpBuf != NULL);
  } else if (Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
    Link = GetFirstNode (&Storage->NameValueListHead);
    while (!IsNull (&Storage->NameValueListHead, Link)) {
      Node = NAME_VALUE_NODE_FROM_LINK (Link);
      Link = GetNextNode (&Storage->NameValueListHead, Link);

      //
      // Only back Node belong to this formset.
      //
      if (StrStr (Storage->ConfigRequest, Node->Name) == NULL) {
        continue;
      }

      TmpNode = AllocateCopyPool (sizeof (NAME_VALUE_NODE), Node);
      TmpNode->Name = AllocateCopyPool (StrSize(Node->Name) * sizeof (CHAR16), Node->Name);
      TmpNode->EditValue = AllocateCopyPool (StrSize(Node->EditValue) * sizeof (CHAR16), Node->EditValue);

      InsertTailList(&BackUpList, &TmpNode->Link);
    }
  }

  //
  // Get default value.
  //
  ExtractDefault (FormSet, NULL, EFI_HII_DEFAULT_CLASS_STANDARD, FormSetLevel, GetDefaultForStorage, Storage, TRUE);

  //
  // Update the question value based on the input ConfigRequest.
  //
  if (Storage->Type == EFI_HII_VARSTORE_BUFFER ||
      (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER)) {
    ASSERT (BackUpBuf != NULL);
    BufferSize = Storage->Size;
    Status = gHiiConfigRouting->BlockToConfig(
                                  gHiiConfigRouting,
                                  ConfigRequest,
                                  Storage->EditBuffer,
                                  BufferSize,
                                  &Result,
                                  &Progress
                                  );
    ASSERT_EFI_ERROR (Status);

    Status = gHiiConfigRouting->ConfigToBlock (
                                  gHiiConfigRouting,
                                  Result,
                                  BackUpBuf,
                                  &BufferSize,
                                  &Progress
                                  );
    ASSERT_EFI_ERROR (Status);

    if (Result != NULL) {
      FreePool (Result);
    }

    CopyMem (Storage->EditBuffer, BackUpBuf, Storage->Size);
    FreePool (BackUpBuf);
  } else if (Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
    //
    // Update question value, only element in ConfigReqeust will be update.
    //
    Link = GetFirstNode (&BackUpList);
    while (!IsNull (&BackUpList, Link)) {
      Node = NAME_VALUE_NODE_FROM_LINK (Link);
      Link = GetNextNode (&BackUpList, Link);

      if (StrStr (ConfigRequest, Node->Name) != NULL) {
        continue;
      }

      NodeLink = GetFirstNode (&Storage->NameValueListHead);
      while (!IsNull (&Storage->NameValueListHead, NodeLink)) {
        TmpNode  = NAME_VALUE_NODE_FROM_LINK (NodeLink);
        NodeLink = GetNextNode (&Storage->NameValueListHead, NodeLink);

        if (StrCmp (Node->Name, TmpNode->Name) != 0) {
          continue;
        }

        FreePool (TmpNode->EditValue);
        TmpNode->EditValue = AllocateCopyPool (StrSize(Node->EditValue) * sizeof (CHAR16), Node->EditValue);

        RemoveEntryList (&Node->Link);
        FreePool (Node->EditValue);
        FreePool (Node->Name);
        FreePool (Node);
      }
    }

    //
    // Restore the Name/Value node.
    //
    Link = GetFirstNode (&BackUpList);
    while (!IsNull (&BackUpList, Link)) {
      Node = NAME_VALUE_NODE_FROM_LINK (Link);
      Link = GetNextNode (&BackUpList, Link);

      //
      // Free this node.
      //
      RemoveEntryList (&Node->Link);
      FreePool (Node->EditValue);
      FreePool (Node->Name);
      FreePool (Node);
    }
  }
}

/**
  Fill storage's edit copy with settings requested from Configuration Driver.

  @param  [in] FormSet           FormSet data structure.
  @param  [in] Storage           Buffer Storage.

**/
VOID
LoadStorage (
  IN FORM_BROWSER_FORMSET    *FormSet,
  IN FORMSET_STORAGE         *Storage
  )
{
  EFI_STATUS  Status;
  EFI_STRING  Progress;
  EFI_STRING  Result;
  CHAR16      *StrPtr;
  EFI_STRING  ConfigRequest;
  UINTN       StrLen;

  ConfigRequest = NULL;

  switch (Storage->BrowserStorage->Type) {
    case EFI_HII_VARSTORE_EFI_VARIABLE:
      return;

    case EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER:
      if (Storage->BrowserStorage->ConfigRequest != NULL) {
        ConfigRequestAdjust(Storage->BrowserStorage, Storage->ConfigRequest, FALSE);
        return;
      }
      break;

    case EFI_HII_VARSTORE_BUFFER:
    case EFI_HII_VARSTORE_NAME_VALUE:
      //
      // Skip if there is no RequestElement.
      //
      if (Storage->ElementCount == 0) {
        return;
      }

      //
      // Just update the ConfigRequest, if storage already initialized.
      //
      if (Storage->BrowserStorage->Initialized) {
        ConfigRequestAdjust(Storage->BrowserStorage, Storage->ConfigRequest, FALSE);
        return;
      }

      Storage->BrowserStorage->Initialized = TRUE;
      break;

    default:
      return;
  }

  if (Storage->BrowserStorage->Type != EFI_HII_VARSTORE_NAME_VALUE) {
    //
    // Create the config request string to get all fields for this storage.
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWW"followed by a Null-terminator
    //
    StrLen = StrSize (Storage->BrowserStorage->ConfigHdr) + 20 * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (StrLen);
    ASSERT (ConfigRequest != NULL);
    UnicodeSPrint (
               ConfigRequest,
               StrLen,
               L"%s&OFFSET=0&WIDTH=%04x",
               Storage->BrowserStorage->ConfigHdr,
               Storage->BrowserStorage->Size);
  } else {
    ConfigRequest = Storage->ConfigRequest;
  }

  //
  // Request current settings from Configuration Driver
  //
  Status = gHiiConfigRouting->ExtractConfig (
                                    gHiiConfigRouting,
                                    ConfigRequest,
                                    &Progress,
                                    &Result
                                    );

  //
  // If get value fail, extract default from IFR binary
  //
  if (EFI_ERROR (Status)) {
    ExtractDefault (FormSet, NULL, EFI_HII_DEFAULT_CLASS_STANDARD, FormSetLevel, GetDefaultForStorage, Storage->BrowserStorage, TRUE);
  } else {
    //
    // Convert Result from <ConfigAltResp> to <ConfigResp>
    //
    StrPtr = StrStr (Result, L"&GUID=");
    if (StrPtr != NULL) {
      *StrPtr = L'\0';
    }

    Status = ConfigRespToStorage (Storage->BrowserStorage, Result);
    FreePool (Result);
  }

  Storage->BrowserStorage->ConfigRequest = AllocateCopyPool (StrSize (Storage->ConfigRequest), Storage->ConfigRequest);

  //
  // Input NULL for ConfigRequest field means sync all fields from editbuffer to buffer.
  //
  SynchronizeStorage(FormSet, Storage->BrowserStorage, NULL, TRUE);

  if (Storage->BrowserStorage->Type != EFI_HII_VARSTORE_NAME_VALUE) {
    if (ConfigRequest != NULL) {
      FreePool (ConfigRequest);
    }
  }
}

/**
  Get Value changed status from old question.

  @param  NewFormSet                FormSet data structure.
  @param  OldQuestion               Old question which has value changed.

**/
VOID
SyncStatusForQuestion (
  IN OUT FORM_BROWSER_FORMSET             *NewFormSet,
  IN     FORM_BROWSER_STATEMENT           *OldQuestion
  )
{
  LIST_ENTRY                  *Link;
  LIST_ENTRY                  *QuestionLink;
  FORM_BROWSER_FORM           *Form;
  FORM_BROWSER_STATEMENT      *Question;

  //
  // For each form in one formset.
  //
  Link = GetFirstNode (&NewFormSet->FormListHead);
  while (!IsNull (&NewFormSet->FormListHead, Link)) {
    Form = FORM_BROWSER_FORM_FROM_LINK (Link);
    Link = GetNextNode (&NewFormSet->FormListHead, Link);

    //
    // for each question in one form.
    //
    QuestionLink = GetFirstNode (&Form->StatementListHead);
    while (!IsNull (&Form->StatementListHead, QuestionLink)) {
      Question = FORM_BROWSER_STATEMENT_FROM_LINK (QuestionLink);
      QuestionLink = GetNextNode (&Form->StatementListHead, QuestionLink);

      if (Question->QuestionId == OldQuestion->QuestionId) {
        Question->ValueChanged = TRUE;
        return;
      }
    }
  }
}

/**
  Get Value changed status from old formset.

  @param  NewFormSet                FormSet data structure.
  @param  OldFormSet                FormSet data structure.

**/
VOID
SyncStatusForFormSet (
  IN OUT FORM_BROWSER_FORMSET             *NewFormSet,
  IN     FORM_BROWSER_FORMSET             *OldFormSet
  )
{
  LIST_ENTRY                  *Link;
  LIST_ENTRY                  *QuestionLink;
  FORM_BROWSER_FORM           *Form;
  FORM_BROWSER_STATEMENT      *Question;

  //
  // For each form in one formset.
  //
  Link = GetFirstNode (&OldFormSet->FormListHead);
  while (!IsNull (&OldFormSet->FormListHead, Link)) {
    Form = FORM_BROWSER_FORM_FROM_LINK (Link);
    Link = GetNextNode (&OldFormSet->FormListHead, Link);

    //
    // for each question in one form.
    //
    QuestionLink = GetFirstNode (&Form->StatementListHead);
    while (!IsNull (&Form->StatementListHead, QuestionLink)) {
      Question = FORM_BROWSER_STATEMENT_FROM_LINK (QuestionLink);
      QuestionLink = GetNextNode (&Form->StatementListHead, QuestionLink);

      if (!Question->ValueChanged) {
        continue;
      }

      //
      // Find the same question in new formset and update the value changed flag.
      //
      SyncStatusForQuestion (NewFormSet, Question);
    }
  }
}

/**
  Get current setting of Questions.

  @param  [in,out] FormSet       FormSet data structure.

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
InitCurrentSetting (
  IN OUT FORM_BROWSER_FORMSET             *FormSet
  )
{
  LIST_ENTRY              *Link;
  FORMSET_STORAGE         *Storage;
  FORM_BROWSER_FORMSET    *OldFormSet;

  //
  // Try to find pre FormSet in the maintain backup list.
  // If old formset != NULL, destroy this formset. Add new formset to gBrowserFormSetList.
  //
  OldFormSet = GetFormSetFromHiiHandle (FormSet->HiiHandle);
  if (OldFormSet != NULL) {
    SyncStatusForFormSet (FormSet, OldFormSet);
    RemoveEntryList (&OldFormSet->Link);
    DestroyFormSet (OldFormSet);
  }
  InsertTailList (&gBrowserFormSetList, &FormSet->Link);

  //
  // Extract default from IFR binary for no storage questions.
  //
  ExtractDefault (FormSet, NULL, EFI_HII_DEFAULT_CLASS_STANDARD, FormSetLevel, GetDefaultForNoStorage, NULL, TRUE);

  //
  // Request current settings from Configuration Driver
  //
  Link = GetFirstNode (&FormSet->StorageListHead);
  while (!IsNull (&FormSet->StorageListHead, Link)) {
    Storage = FORMSET_STORAGE_FROM_LINK (Link);

    LoadStorage (FormSet, Storage);

    Link = GetNextNode (&FormSet->StorageListHead, Link);
  }

  return EFI_SUCCESS;
}


/**
  Fetch the IFR binary data of a FormSet.

  @param  [in] Handle            PackageList Handle
  @param  [in,out] FormSetGuid   On input, GUID or class GUID of a FormSet. If not
                                 specified (NULL or zero GUID), take the first
                                 FormSet with class GUID EFI_HII_PLATFORM_SETUP_FORMSET_GUID
                                 found in package list.
                                 On output, GUID of the FormSet found(if not NULL).
  @param  [out] BinaryLength     The length of the FormSet IFR binary.
  @param  [out] BinaryData       The buffer designed to receive the FormSet.

  @retval EFI_SUCCESS            Buffer filled with the requested FormSet.
                                 BufferLength was updated.
  @retval EFI_INVALID_PARAMETER  The handle is unknown.
  @retval EFI_NOT_FOUND          A Form or FormSet on the requested handle cannot
                                 be found with the requested FormId.

**/
EFI_STATUS
GetIfrBinaryData (
  IN  EFI_HII_HANDLE   Handle,
  IN OUT EFI_GUID      *FormSetGuid,
  OUT UINTN            *BinaryLength,
  OUT UINT8            **BinaryData
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *HiiPackageList;
  UINTN                        BufferSize;
  UINT8                        *Package;
  UINT8                        *OpCodeData;
  UINT32                       Offset;
  UINT32                       Offset2;
  UINT32                       PackageListLength;
  EFI_HII_PACKAGE_HEADER       PackageHeader;
  UINT8                        Index;
  UINT8                        NumberOfClassGuid;
  BOOLEAN                      ClassGuidMatch;
  EFI_GUID                     *ClassGuid;
  EFI_GUID                     *ComparingGuid;

  OpCodeData = NULL;
  Package = NULL;
  ZeroMem (&PackageHeader, sizeof (EFI_HII_PACKAGE_HEADER));

  //
  // if FormSetGuid is NULL or zero GUID, return first Setup FormSet in the package list
  //
  if (FormSetGuid == NULL) {
    ComparingGuid = &gZeroGuid;
  } else {
    ComparingGuid = FormSetGuid;
  }

  //
  // Get HII PackageList
  //
  BufferSize = 0;
  HiiPackageList = NULL;
  Status = gHiiDatabase->ExportPackageLists (gHiiDatabase, Handle, &BufferSize, HiiPackageList);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    HiiPackageList = AllocatePool (BufferSize);
    ASSERT (HiiPackageList != NULL);
    if (HiiPackageList == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    Status = gHiiDatabase->ExportPackageLists (gHiiDatabase, Handle, &BufferSize, HiiPackageList);
    ASSERT (HiiPackageList != NULL);
  }
  if (HiiPackageList == NULL) {
    return Status;
  }


  //
  // Get Form package from this HII package List
  //
  Offset = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  Offset2 = 0;
  CopyMem (&PackageListLength, &HiiPackageList->PackageLength, sizeof (UINT32));

  ClassGuidMatch = FALSE;
  while (Offset < PackageListLength) {
    Package = ((UINT8 *) HiiPackageList) + Offset;
    CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));

    if (PackageHeader.Type == EFI_HII_PACKAGE_FORMS) {
      //
      // Search FormSet in this Form Package
      //
      Offset2 = sizeof (EFI_HII_PACKAGE_HEADER);
      while (Offset2 < PackageHeader.Length) {
        OpCodeData = Package + Offset2;

        if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_SET_OP) {
          //
          // Try to compare against FormSet GUID
          //
          if ((FormSetGuid != NULL && CompareGuid (FormSetGuid, &gZeroGuid)) ||
              CompareGuid (ComparingGuid, (EFI_GUID *)(OpCodeData + sizeof (EFI_IFR_OP_HEADER)))) {
            break;
          }

          if (((EFI_IFR_OP_HEADER *) OpCodeData)->Length > OFFSET_OF (EFI_IFR_FORM_SET, Flags)) {
            //
            // Try to compare against FormSet class GUID
            //
            NumberOfClassGuid = (UINT8) (((EFI_IFR_FORM_SET *) OpCodeData)->Flags & 0x3);
            ClassGuid         = (EFI_GUID *) (OpCodeData + sizeof (EFI_IFR_FORM_SET));
            for (Index = 0; Index < NumberOfClassGuid; Index++) {
              if (CompareGuid (ComparingGuid, ClassGuid + Index)) {
                ClassGuidMatch = TRUE;
                break;
              }
            }
            if (ClassGuidMatch) {
              break;
            }
          } else if (ComparingGuid == &gEfiHiiPlatformSetupFormsetGuid) {
            ClassGuidMatch = TRUE;
            break;
          }
        }

        Offset2 += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;
      }

      if (Offset2 < PackageHeader.Length) {
        //
        // Target FormSet found
        //
        break;
      }
    }

    Offset += PackageHeader.Length;
  }

  if (Offset >= PackageListLength) {
    //
    // Form package not found in this Package List
    //
    FreePool (HiiPackageList);
    return EFI_NOT_FOUND;
  }

  if (FormSetGuid != NULL) {
    //
    // Return the FormSet GUID
    //
    CopyMem (FormSetGuid, &((EFI_IFR_FORM_SET *) OpCodeData)->Guid, sizeof (EFI_GUID));
  }

  //
  // To determine the length of a whole FormSet IFR binary, one have to parse all the Opcodes
  // in this FormSet; So, here just simply copy the data from start of a FormSet to the end
  // of the Form Package.
  //
  *BinaryLength = PackageHeader.Length - Offset2;
  *BinaryData = AllocateCopyPool (*BinaryLength, OpCodeData);

  FreePool (HiiPackageList);

  if (*BinaryData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}


/**
  Initialize the internal data structure of a FormSet.

  @param  [in] Handle            PackageList Handle
  @param  [in] FormSetGuid       On input, GUID or class GUID of a FormSet. If not
                                 specified (NULL or zero GUID), take the first
                                 FormSet with class GUID EFI_HII_PLATFORM_SETUP_FORMSET_GUID
                                 found in package list.
                                 On output, GUID of the FormSet found(if not NULL).
  @param  [in] FormSet           FormSet data structure.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_NOT_FOUND          The specified FormSet could not be found.

**/
EFI_STATUS
InitializeFormSet (
  IN  EFI_HII_HANDLE                   Handle,
  IN OUT EFI_GUID                      *FormSetGuid,
  OUT FORM_BROWSER_FORMSET             *FormSet
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                DriverHandle;

  Status = GetIfrBinaryData (Handle, FormSetGuid, &FormSet->IfrBinaryLength, &FormSet->IfrBinaryData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FormSet->Signature = FORM_BROWSER_FORMSET_SIGNATURE;
  FormSet->HiiHandle = Handle;

  CopyGuid (&FormSet->Guid, FormSetGuid);
  FormSet->QuestionInited = FALSE;

  //
  // Retrieve ConfigAccess Protocol associated with this HiiPackageList
  //
  Status = gHiiDatabase->GetPackageListHandle (gHiiDatabase, Handle, &DriverHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  FormSet->DriverHandle = DriverHandle;
  Status = gBS->HandleProtocol (
                  DriverHandle,
                  &gEfiHiiConfigAccessProtocolGuid,
                  (VOID **) &FormSet->ConfigAccess
                  );
  if (EFI_ERROR (Status)) {
    //
    // Configuration Driver don't attach ConfigAccess protocol to its HII package
    // list, then there will be no configuration action required
    //
    FormSet->ConfigAccess = NULL;
  }

  //
  // Parse the IFR binary OpCodes
  //
  Status = ParseOpCodes (FormSet);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  InsertTailList (&mFBPrivate.FormSetList, &FormSet->DisplayLink);
  if (!ChkInHiiHandleList (Handle)) {
    InsertHiiHandle (Handle);
  }

  return EFI_SUCCESS;
}


/**
  Save globals used by previous call to SendForm(). SendForm() may be called from
  HiiConfigAccess.Callback(), this will cause SendForm() be reentried.
  So, save globals of previous call to SendForm() and restore them upon exit.

**/
VOID
SaveBrowserContext (
  VOID
  )
{
  BROWSER_CONTEXT  *Context;
  FORM_ENTRY_INFO     *MenuList;

  gBrowserContextCount++;
  if (gBrowserContextCount == 1) {
    //
    // This is not reentry of SendForm(), no context to save
    //
    return;
  }

  Context = AllocatePool (sizeof (BROWSER_CONTEXT));
  ASSERT (Context != NULL);
  if (Context == NULL) {
    return;
  }

  Context->Signature = BROWSER_CONTEXT_SIGNATURE;

  //
  // Save FormBrowser context
  //
  Context->Selection            = gCurrentSelection;
  Context->ResetRequired        = gResetRequired;
  Context->ExitRequired         = gExitRequired;
  Context->HiiHandle            = mCurrentHiiHandle;
  Context->FormId               = mCurrentFormId;
  CopyGuid (&Context->FormSetGuid, &mCurrentFormSetGuid);


  //
  // Save the menu history data.
  //
  InitializeListHead(&Context->FormHistoryList);
  while (!IsListEmpty (&mFBPrivate.FormBrowserEx2.FormViewHistoryHead)) {
    MenuList = FORM_ENTRY_INFO_FROM_LINK (mFBPrivate.FormBrowserEx2.FormViewHistoryHead.ForwardLink);
    RemoveEntryList (&MenuList->Link);

    InsertTailList(&Context->FormHistoryList, &MenuList->Link);
  }

  //
  // Insert to FormBrowser context list
  //
  InsertHeadList (&gBrowserContextList, &Context->Link);
}


/**
  Restore globals used by previous call to SendForm().

**/
VOID
RestoreBrowserContext (
  VOID
  )
{
  LIST_ENTRY       *Link;
  BROWSER_CONTEXT  *Context;
  FORM_ENTRY_INFO     *MenuList;

  ASSERT (gBrowserContextCount != 0);
  gBrowserContextCount--;
  if (gBrowserContextCount == 0) {
    //
    // This is not reentry of SendForm(), no context to restore
    //
    return;
  }

  ASSERT (!IsListEmpty (&gBrowserContextList));

  Link = GetFirstNode (&gBrowserContextList);
  Context = BROWSER_CONTEXT_FROM_LINK (Link);

  //
  // Restore FormBrowser context
  //
  gCurrentSelection     = Context->Selection;
  gResetRequired        = Context->ResetRequired;
  gExitRequired         = Context->ExitRequired;
  mCurrentHiiHandle     = Context->HiiHandle;
  mCurrentFormId        = Context->FormId;
  CopyGuid (&mCurrentFormSetGuid, &Context->FormSetGuid);

  //
  // Restore the menu history data.
  //
  while (!IsListEmpty (&Context->FormHistoryList)) {
    MenuList = FORM_ENTRY_INFO_FROM_LINK (Context->FormHistoryList.ForwardLink);
    RemoveEntryList (&MenuList->Link);

    InsertTailList(&mFBPrivate.FormBrowserEx2.FormViewHistoryHead, &MenuList->Link);
  }

  //
  // Remove from FormBrowser context list
  //
  RemoveEntryList (&Context->Link);
  gBS->FreePool (Context);
}

/**
  Find the matched FormSet context in the backup maintain list based on HiiHandle.

  @param [in] Handle  The HII Handle.

  @return the found FormSet context. If no found, NULL will return.

**/
FORM_BROWSER_FORMSET *
GetFormSetFromHiiHandle (
  IN EFI_HII_HANDLE Handle
  )
{
  LIST_ENTRY           *Link;
  FORM_BROWSER_FORMSET *FormSet;

  Link = GetFirstNode (&gBrowserFormSetList);
  while (!IsNull (&gBrowserFormSetList, Link)) {
    FormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
    Link = GetNextNode (&gBrowserFormSetList, Link);
    if (!ValidateFormSet(FormSet)) {
      continue;
    }
    if (FormSet->HiiHandle == Handle) {
      return FormSet;
    }
  }

  return NULL;
}

/**
  Check whether the input HII handle is the FormSet that is being used.

  @param [in] Handle  The HII Handle.

  @retval TRUE        HII handle is being used.
  @retval FALSE       HII handle is not being used.

**/
BOOLEAN
IsHiiHandleInBrowserContext (
  IN EFI_HII_HANDLE Handle
  )
{
  LIST_ENTRY       *Link;
  BROWSER_CONTEXT  *Context;

  //
  // HiiHandle is Current FormSet.
  //
  if (mCurrentHiiHandle == Handle) {
    return TRUE;
  }

  //
  // Check whether HiiHandle is in BrowserContext.
  //
  Link = GetFirstNode (&gBrowserContextList);
  while (!IsNull (&gBrowserContextList, Link)) {
    Context = BROWSER_CONTEXT_FROM_LINK (Link);
    if (Context->HiiHandle == Handle) {
      //
      // HiiHandle is in BrowserContext
      //
      return TRUE;
    }
    Link = GetNextNode (&gBrowserContextList, Link);
  }

  return FALSE;
}

/**
  Perform Password check.
  Passwork may be encrypted by driver that requires the specific check.

  @param  Form             Form where Password Statement is in.
  @param  Statement        Password statement
  @param  PasswordString   Password string to be checked. It may be NULL.
                           NULL means to restore password.
                           "" string can be used to checked whether old password does exist.

  @return Status     Status of Password check.
**/
EFI_STATUS
EFIAPI
PasswordCheck (
  IN FORM_DISPLAY_ENGINE_FORM      *Form,
  IN FORM_DISPLAY_ENGINE_STATEMENT *Statement,
  IN EFI_STRING                    PasswordString  OPTIONAL
  )
{
  EFI_STATUS                      Status;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;
  EFI_BROWSER_ACTION_REQUEST      ActionRequest;
  EFI_IFR_TYPE_VALUE              IfrTypeValue;
  FORM_BROWSER_STATEMENT          *Question;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_QUESTION_ID                 QuestionId;
  EFI_HII_HANDLE                  OrgHiiHandle;

  ConfigAccess = gCurrentSelection->FormSet->ConfigAccess;
  Question = GetBrowserStatement(Statement);
  ASSERT (Question != NULL);
  if (Question == NULL) {
    return EFI_NOT_FOUND;
  }

  if ((Question->QuestionFlags & EFI_IFR_FLAG_CALLBACK) == EFI_IFR_FLAG_CALLBACK) {
    if (ConfigAccess == NULL) {
      return EFI_UNSUPPORTED;
    }
  } else {
    if (PasswordString == NULL) {
      return EFI_SUCCESS;
    }

    //
    // Check whether has preexisted password.
    //
    if (PasswordString[0] == 0) {
      if (*((CHAR16 *) Question->BufferValue) == 0) {
        return EFI_SUCCESS;
      } else {
        return EFI_NOT_READY;
      }
    }

    //
    // Check whether the input password is same as preexisted password.
    //
    if (StrnCmp (PasswordString, (CHAR16 *) Question->BufferValue, Question->StorageWidth/sizeof (CHAR16)) == 0) {
      return EFI_SUCCESS;
    } else {
      return EFI_NOT_READY;
    }
  }

  OrgHiiHandle = mSystemLevelFormSet->HiiHandle;
  if (IS_IMPORT_STATEMENT (Question)) {
    HiiHandle    = Question->ImportInfo->SrcFormSet->HiiHandle;
    ConfigAccess = Question->ImportInfo->SrcFormSet->ConfigAccess;
    QuestionId   = Question->ImportInfo->SrcStatement->QuestionId;
    mSystemLevelFormSet->HiiHandle = Question->ImportInfo->SrcFormSet->HiiHandle;
  } else {
    HiiHandle    = gCurrentSelection->FormSet->HiiHandle;
    QuestionId   = Question->QuestionId;
  }

  //
  // Prepare password string in HII database
  //
  if (PasswordString != NULL) {
    IfrTypeValue.string = NewString (PasswordString, HiiHandle);
  } else {
    IfrTypeValue.string = 0;
  }

  //
  // Send password to Configuration Driver for validation
  //
  Status = ConfigAccess->Callback (
                           ConfigAccess,
                           EFI_BROWSER_ACTION_CHANGING,
                           QuestionId,
                           Question->HiiValue.Type,
                           &IfrTypeValue,
                           &ActionRequest
                           );
  if (!EFI_ERROR (Status)) {
    //
    // SCU use EFI_BROWSER_ACTION_CHANGED to process setting.
    //
    Status = ConfigAccess->Callback (
                             ConfigAccess,
                             EFI_BROWSER_ACTION_CHANGED,
                             QuestionId,
                             Question->HiiValue.Type,
                             &IfrTypeValue,
                             &ActionRequest
                             );
  }
  mSystemLevelFormSet->HiiHandle = OrgHiiHandle;

  //
  // Remove password string from HII database
  //
  if (PasswordString != NULL) {
    DeleteString (IfrTypeValue.string, HiiHandle);
  }

  return Status;
}

/**
  Find the registered HotKey based on KeyData.

  @param[in] KeyData     A pointer to a buffer that describes the keystroke
                         information for the hot key.

  @return The registered HotKey context. If no found, NULL will return.
**/
BROWSER_HOT_KEY *
GetHotKeyFromRegisterList (
  IN EFI_INPUT_KEY *KeyData
  )
{
  LIST_ENTRY       *Link;
  BROWSER_HOT_KEY  *HotKey;

  Link = GetFirstNode (&gBrowserHotKeyList);
  while (!IsNull (&gBrowserHotKeyList, Link)) {
    HotKey = BROWSER_HOT_KEY_FROM_LINK (Link);
    if (HotKey->KeyData->ScanCode == KeyData->ScanCode) {
      return HotKey;
    }
    Link = GetNextNode (&gBrowserHotKeyList, Link);
  }

  return NULL;
}

/**
  Configure what scope the hot key will impact.
  All hot keys have the same scope. The mixed hot keys with the different level are not supported.
  If no scope is set, the default scope will be FormSet level.
  After all registered hot keys are removed, previous Scope can reset to another level.

  @param[in] Scope               Scope level to be set.

  @retval EFI_SUCCESS            Scope is set correctly.
  @retval EFI_INVALID_PARAMETER  Scope is not the valid value specified in BROWSER_SETTING_SCOPE.
  @retval EFI_UNSPPORTED         Scope level is different from current one that the registered hot keys have.

**/
EFI_STATUS
EFIAPI
SetScope (
  IN BROWSER_SETTING_SCOPE Scope
  )
{
  if (Scope >= MaxLevel) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // When no hot key registered in system or on the first setting,
  // Scope can be set.
  //
  if (mBrowserScopeFirstSet || IsListEmpty (&gBrowserHotKeyList)) {
    gBrowserSettingScope  = Scope;
    mBrowserScopeFirstSet = FALSE;
  } else if (Scope != gBrowserSettingScope) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Register the hot key with its browser action, or unregistered the hot key.
  Only support hot key that is not printable character (control key, function key, etc.).
  If the action value is zero, the hot key will be unregistered if it has been registered.
  If the same hot key has been registered, the new action and help string will override the previous ones.

  @param[in] KeyData     A pointer to a buffer that describes the keystroke
                         information for the hot key. Its type is EFI_INPUT_KEY to
                         be supported by all ConsoleIn devices.
  @param[in] Action      Action value that describes what action will be trigged when the hot key is pressed.
  @param[in] DefaultId   Specifies the type of defaults to retrieve, which is only for DEFAULT action.
  @param[in] HelpString  Help string that describes the hot key information.
                         Its value may be NULL for the unregistered hot key.

  @retval EFI_SUCCESS            Hot key is registered or unregistered.
  @retval EFI_INVALID_PARAMETER  KeyData is NULL or HelpString is NULL on register.
  @retval EFI_NOT_FOUND          KeyData is not found to be unregistered.
  @retval EFI_UNSUPPORTED        Key represents a printable character. It is conflicted with Browser.
**/
EFI_STATUS
EFIAPI
RegisterHotKey (
  IN EFI_INPUT_KEY *KeyData,
  IN UINT32        Action,
  IN UINT16        DefaultId,
  IN EFI_STRING    HelpString OPTIONAL
  )
{
  BROWSER_HOT_KEY  *HotKey;

  //
  // Check input parameters.
  //
  if (KeyData == NULL || KeyData->UnicodeChar != CHAR_NULL ||
     (Action != BROWSER_ACTION_UNREGISTER && HelpString == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check whether the input KeyData is in BrowserHotKeyList.
  //
  HotKey = GetHotKeyFromRegisterList (KeyData);

  //
  // Unregister HotKey
  //
  if (Action == BROWSER_ACTION_UNREGISTER) {
    if (HotKey != NULL) {
      //
      // The registered HotKey is found.
      // Remove it from List, and free its resource.
      //
      RemoveEntryList (&HotKey->Link);
      FreePool (HotKey->KeyData);
      FreePool (HotKey->HelpString);
      return EFI_SUCCESS;
    } else {
      //
      // The registered HotKey is not found.
      //
      return EFI_NOT_FOUND;
    }
  }

  //
  // Register HotKey into List.
  //
  if (HotKey == NULL) {
    //
    // Create new Key, and add it into List.
    //
    HotKey = AllocateZeroPool (sizeof (BROWSER_HOT_KEY));
    ASSERT (HotKey != NULL);
    if (HotKey == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    HotKey->Signature = BROWSER_HOT_KEY_SIGNATURE;
    HotKey->KeyData   = AllocateCopyPool (sizeof (EFI_INPUT_KEY), KeyData);
    InsertTailList (&gBrowserHotKeyList, &HotKey->Link);
  }

  //
  // Fill HotKey information.
  //
  HotKey->Action     = Action;
  HotKey->DefaultId  = DefaultId;
  if (HotKey->HelpString != NULL) {
    FreePool (HotKey->HelpString);
  }
  HotKey->HelpString = AllocateCopyPool (StrSize (HelpString), HelpString);

  return EFI_SUCCESS;
}

/**
  Register Exit handler function.
  When more than one handler function is registered, the latter one will override the previous one.
  When NULL handler is specified, the previous Exit handler will be unregistered.

  @param[in] Handler      Pointer to handler function.

**/
VOID
EFIAPI
RegiserExitHandler (
  IN EXIT_HANDLER Handler
  )
{
  ExitHandlerFunction = Handler;
  return;
}

/**
  Check whether the browser data has been modified.

  @retval TRUE        Browser data is modified.
  @retval FALSE       No browser data is modified.

**/
BOOLEAN
EFIAPI
IsBrowserDataModified (
  VOID
  )
{
  LIST_ENTRY              *Link;
  FORM_BROWSER_FORMSET    *FormSet;

  switch (gBrowserSettingScope) {
    case FormLevel:
      if (gCurrentSelection == NULL) {
        return FALSE;
      }
      return IsNvUpdateRequiredForForm (gCurrentSelection->Form);

    case FormSetLevel:
      if (gCurrentSelection == NULL) {
        return FALSE;
      }
      return IsNvUpdateRequiredForFormSet (gCurrentSelection->FormSet);

    case SystemLevel:
      Link = GetFirstNode (&gBrowserFormSetList);
      while (!IsNull (&gBrowserFormSetList, Link)) {
        FormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
        if (!ValidateFormSet(FormSet)) {
          continue;
        }

        if (IsNvUpdateRequiredForFormSet (FormSet)) {
          return TRUE;
        }
        Link = GetNextNode (&gBrowserFormSetList, Link);
      }
      return FALSE;

    default:
      return FALSE;
  }
}

/**
  Execute the action requested by the Action parameter.

  @param[in] Action     Execute the request action.
  @param[in] DefaultId  The default Id info when need to load default value. Only used when Action is BROWSER_ACTION_DEFAULT.

  @retval EFI_SUCCESS              Execute the request action succss.
  @retval EFI_INVALID_PARAMETER    The input action value is invalid.

**/
EFI_STATUS
EFIAPI
ExecuteAction (
  IN UINT32        Action,
  IN UINT16        DefaultId
  )
{
  EFI_STATUS              Status;
  FORM_BROWSER_FORMSET    *FormSet;
  FORM_BROWSER_FORM       *Form;

  if (gBrowserSettingScope < SystemLevel && gCurrentSelection == NULL) {
    return EFI_NOT_READY;
  }

  Status  = EFI_SUCCESS;
  FormSet = NULL;
  Form    = NULL;
  if (gBrowserSettingScope < SystemLevel) {
    FormSet = gCurrentSelection->FormSet;
    Form    = gCurrentSelection->Form;
  }

  //
  // Executet the discard action.
  //
  if ((Action & BROWSER_ACTION_DISCARD) != 0) {
    Status = DiscardForm (FormSet, Form, gBrowserSettingScope);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Executet the difault action.
  //
  if ((Action & BROWSER_ACTION_DEFAULT) != 0) {
    Status = ExtractDefault (FormSet, Form, DefaultId, gBrowserSettingScope, GetDefaultForAll, NULL, FALSE);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    UpdateStatementStatus (FormSet, Form, gBrowserSettingScope);
  }

  //
  // Executet the submit action.
  //
  if ((Action & BROWSER_ACTION_SUBMIT) != 0) {
    Status = SubmitForm (FormSet, Form, gBrowserSettingScope);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Executet the reset action.
  //
  if ((Action & BROWSER_ACTION_RESET) != 0) {
    gResetRequired = TRUE;
  }

  //
  // Executet the exit action.
  //
  if ((Action & BROWSER_ACTION_EXIT) != 0) {
    DiscardForm (FormSet, Form, gBrowserSettingScope);
    if (gBrowserSettingScope == SystemLevel) {
      if (ExitHandlerFunction != NULL) {
        ExitHandlerFunction ();
      }
    }

    gExitRequired = TRUE;
  }

  return Status;
}

/**
  Create reminder to let user to choose save or discard the changed browser data.
  Caller can use it to actively check the changed browser data.

  @retval BROWSER_NO_CHANGES       No browser data is changed.
  @retval BROWSER_SAVE_CHANGES     The changed browser data is saved.
  @retval BROWSER_DISCARD_CHANGES  The changed browser data is discard.
  @retval BROWSER_KEEP_CURRENT     Browser keep current changes.

**/
UINT32
EFIAPI
SaveReminder (
  VOID
  )
{
  LIST_ENTRY              *Link;
  FORM_BROWSER_FORMSET    *FormSet;
  BOOLEAN                 IsDataChanged;
  UINT32                  DataSavedAction;
  UINT32                  ConfirmRet;

  DataSavedAction  = BROWSER_NO_CHANGES;
  IsDataChanged    = FALSE;
  Link = GetFirstNode (&gBrowserFormSetList);
  while (!IsNull (&gBrowserFormSetList, Link)) {
    FormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
    Link = GetNextNode (&gBrowserFormSetList, Link);
    if (!ValidateFormSet(FormSet)) {
      continue;
    }
    if (IsNvUpdateRequiredForFormSet (FormSet)) {
      IsDataChanged = TRUE;
      break;
    }
  }

  //
  // No data is changed. No save is required.
  //
  if (!IsDataChanged) {
    return DataSavedAction;
  }

  //
  // If data is changed, prompt user to save or discard it.
  //
  do {
    ConfirmRet = (UINT32) ConfirmDataChange();

    if (ConfirmRet == BROWSER_ACTION_SUBMIT) {
      SubmitForm (NULL, NULL, SystemLevel);
      DataSavedAction = BROWSER_SAVE_CHANGES;
      break;
    } else if (ConfirmRet == BROWSER_ACTION_DISCARD) {
      DiscardForm (NULL, NULL, SystemLevel);
      DataSavedAction = BROWSER_DISCARD_CHANGES;
      break;
    } else if (ConfirmRet == BROWSER_ACTION_NONE) {
      DataSavedAction = BROWSER_KEEP_CURRENT;
      break;
    }
  } while (1);

  return DataSavedAction;
}

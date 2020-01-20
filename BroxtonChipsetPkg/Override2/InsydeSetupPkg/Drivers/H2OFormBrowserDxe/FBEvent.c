/** @file
Event for formbrowser
;******************************************************************************
;* Copyright (c) 2013 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "InternalH2OFormBrowser.h"
#include <Protocol/SetupMouse.h>
#include <Protocol/Cpu.h>
#include <Protocol/H2OSubmitSvc.h>
//[-start-170316-IB07400848-add]//
//[-start-170629-IB07400882-modify]//
#ifdef SEG_SNAP_SCU_SUPPORT
#include <Protocol/H2OMessageFilter.h>
#endif
//[-end-170629-IB07400882-modify]//
//[-end-170316-IB07400848-add]//

BOOLEAN                                      mRefreshFormSet       = FALSE;
EFI_EVENT                                    *mInputEventList      = NULL;
H2O_INPUT_EVENT_DESCRIPTION                  *mInputEventDescList  = NULL;
UINT32                                        mInputEventListCount = 0;
LIST_ENTRY      mEventQueue           = INITIALIZE_LIST_HEAD_VARIABLE (mEventQueue);
EFI_LOCK        mEventQueueLock       = {TPL_CALLBACK, 4, 1};
EFI_GUID        mScuFormSetGuid       = {0x9f85453e, 0x2f03, 0x4989, 0xad, 0x3b, 0x4a, 0x84, 0x07, 0x91, 0xaf, 0x3a};
EFI_GUID        mSecureBootMgrFormSetGuid = {0xaa1305b9, 0x1f3, 0x4afb, 0x92, 0xe, 0xc9, 0xb9, 0x79, 0xa8, 0x52, 0xfd};
extern EFI_HII_HANDLE     mCurrentHiiHandle;
extern EFI_GUID           mCurrentFormSetGuid;
extern UINT16             mCurrentFormId;
extern BOOLEAN            mHiiPackageListUpdated;

extern UINT16                   mCurFakeQestId;
extern BOOLEAN                  mFinishRetrieveCall;
STATIC FORM_DISPLAY_ENGINE_STATEMENT mDisplayStatement;

USER_INPUT                    *gUserInput;
FORM_DISPLAY_ENGINE_FORM      *gFormData;

#define TICKS_PER_MS            10000U


/**
 Execute all of the Submit service functions.

 @param[out] Request             A pointer to the request from the Submit service functions.
                                 Related definition can refer to "Browser actions" of FormBrowserEx.h.
 @param[out] ShowSubmitDialog    A pointer to the value if needing to show the original submit dialog.

 @retval EFI_SUCCESS             Execute the Submit service functions successfully.
 @retval EFI_Status              Otherwise.
*/
EFI_STATUS
EFIAPI
H2OSubmitSvcManager (
  OUT   UINT32                              *Request,
  OUT   BOOLEAN                             *ShowSubmitDialog
  )
{
  EFI_STATUS                         Status;
  UINTN                              HandleNum;
  EFI_HANDLE                         *HandleBuf;
  UINTN                              Index;
  H2O_SUBMIT_SVC_PROTOCOL            *SubmitSvc;
  UINT32                             ChildRequest;
  BOOLEAN                            ChildShowSubmitDialog;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gH2OSubmitSvcProtocolGuid,
                  NULL,
                  &HandleNum,
                  &HandleBuf
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  DEBUG ((EFI_D_INFO, "HandleNum: %d \n", HandleNum));

  for (Index = 0; Index < HandleNum; Index++) {
    ChildRequest = 0;
    ChildShowSubmitDialog = TRUE;
    Status = gBS->HandleProtocol (
                    HandleBuf[Index],
                    &gH2OSubmitSvcProtocolGuid,
                    (VOID **)&SubmitSvc
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = SubmitSvc->ExecuteSvc (SubmitSvc, &ChildRequest, &ChildShowSubmitDialog);
    if (!EFI_ERROR (Status)) {
      *Request |= ChildRequest;

      if (*ShowSubmitDialog != FALSE) {
        // Ask FormBrowser not to show the dialog for submit.
        *ShowSubmitDialog = ChildShowSubmitDialog;
      }
    }
  }


  gBS->FreePool (HandleBuf);
  return EFI_SUCCESS;
}

FORM_BROWSER_FORM *
GetScuLoadDefaultForm (
  IN FORM_BROWSER_FORMSET                *FormSet
  )
{
  LIST_ENTRY                             *FormLink;
  FORM_BROWSER_FORM                      *Form;

  FormLink = GetFirstNode (&FormSet->FormListHead);
  Form     = FORM_BROWSER_FORM_FROM_LINK (FormLink);

  while (!IsNull (&FormSet->FormListHead, FormLink)) {
    Form = FORM_BROWSER_FORM_FROM_LINK (FormLink);
    if (Form->FormId == 0xFFFF) {
      break;
    }
    FormLink = GetNextNode (&FormSet->FormListHead, FormLink);
  }

  return Form;
}

EFI_STATUS
FBNotifyEventToTarget (
  IN H2O_FORM_BROWSER_PRIVATE_DATA       *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT        *Event
  )
{
  UINT32                                  Index;
  H2O_DISPLAY_ENGINE_PROTOCOL             *TargetDE;

  if (Event->Target == H2O_FORM_BROWSER_CONSOLE_SYSTEM) {
    for (Index = 0; Index < Private->EngineListCount; Index++) {
      TargetDE = Private->EngineList[Index];
      TargetDE->Notify (TargetDE, Event);
    }
  } else if (Event->Target != H2O_FORM_BROWSER_CONSOLE_NOT_ASSIGNED) {
//[-start-190708-IB16530035-modify]//
//
//We use the "Event->Target(Console Id)" to recognize different node of the console out device,
//Console Redirection is Console In/Out device, cause we should not use "Event->Target(Console Id)"
//to choose the Engine(Text Mode Engine or Graphic Mode Engine)
//Default value is zero, it determine to the value at H2O_FORM_BROWSER_TEXT_SUPPORT and H2O_FORM_BROWSER_METRO_SUPPORT at Project.env
//If both of the H2O_FORM_BROWSER_TEXT_SUPPORT and H2O_FORM_BROWSER_METRO_SUPPORT at Project.env is "YES"
//,EngineList[0] would choose the Text Mode as the ConsoleIn Engine
//,EngineList[1] would choose the Graphic Mode as the ConsoleIn Engine
//
    TargetDE = Private->EngineList[0];
//[-end-190708-IB16530035-modify]//
    TargetDE->Notify (TargetDE, Event);
  }

  return EFI_SUCCESS;
}

/**
 Detect key press.

 @param [in] Private         Formbrowser private data
 @param [in] Event           Display engine event type

 @retval EFI_SUCCESS         Detect key press success.

**/
EFI_STATUS
FBKeyPress (
  IN H2O_FORM_BROWSER_PRIVATE_DATA             *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT              *Event
  )
{

  H2O_DISPLAY_ENGINE_EVT_KEYPRESS              *KeyPress;
  UINT32                                       Index;
  INT32                                        NewActivatedEngine;
  H2O_FORM_BROWSER_CONSOLE_DEV                 *ConsoleDev;

  KeyPress = (H2O_DISPLAY_ENGINE_EVT_KEYPRESS *)Event;

  //
  // change engine by ` key
  //
  if (KeyPress->KeyData.Key.UnicodeChar == '`') {

    //
    // Get new NewActivatedEngine
    //
    NewActivatedEngine = Private->ActivatedEngine;
    for (Index = 0; Index < Private->EngineListCount; Index++) {
      if (Private->ActivatedEngine == (INT32)(Index + 1)) {
        NewActivatedEngine = (INT32)((Index + 1) % Private->EngineListCount + 1);
        break;
      }
    }
    if (NewActivatedEngine == Private->ActivatedEngine) {
      return EFI_SUCCESS;
    }

    //
    // Check has one or more DE is new actived DE and not SerialDevPath
    //
    for (Index = 0; Index < mConsoleDevListCount; Index++) {
      ConsoleDev = mConsoleDevList[Index];

      if (IsConInDeviceType (ConsoleDev->DeviceType) || !IsConOutDeviceType (ConsoleDev->DeviceType)) {
        continue;
      }

      if (ConsoleDev->ConsoleId == NewActivatedEngine) {
        break;
      }
    }
    if (Index >= mConsoleDevListCount) {
      return EFI_SUCCESS;
    }

    //
    // Change actived DE
    //
    FBSetActivedEngine (Private, NewActivatedEngine);

    for (Index = 0; Index < mConsoleDevListCount; Index++) {
      ConsoleDev = mConsoleDevList[Index];

      if (!IsConInDeviceType (ConsoleDev->DeviceType) || IsConOutDeviceType (ConsoleDev->DeviceType)) {
        continue;
      }

      //
      // Set ConsoleId to actived DE
      //
      ConsoleDev->ConsoleId = (H2O_CONSOLE_ID)Private->ActivatedEngine;
    }
    return EFI_SUCCESS;
  }

  FBNotifyEventToTarget (Private, Event);

  return EFI_SUCCESS;
}

/**
 Formbrowser select question

 @param [in] Private         Formbrowser private data
 @param [in] Event           Display engine event type

 @retval Status              Formbrowser select question status

**/
STATIC
EFI_STATUS
FBSelectQuestion (
  IN H2O_FORM_BROWSER_PRIVATE_DATA       *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT        *Event
  )
{
  EFI_STATUS                             Status;
  H2O_DISPLAY_ENGINE_EVT_SELECT_Q        *SelectQ;
  FORM_BROWSER_FORM                      *Form;
  FORM_BROWSER_STATEMENT                 *Statement;
  FORM_DISPLAY_ENGINE_STATEMENT          DisplayStatement;

  SelectQ = (H2O_DISPLAY_ENGINE_EVT_SELECT_Q *)Event;
  Form = FBPageIdToForm (Private, SelectQ->PageId);

  if (Form == NULL) {
    return EFI_NOT_FOUND;
  }

  if (SelectQ->QuestionId != 0) {
    Statement = IdToQuestion2 (Form, SelectQ->QuestionId);
  } else {
    DisplayStatement.OpCode = SelectQ->IfrOpCode;
    Statement = GetBrowserStatement (&DisplayStatement);
  }

  if (Statement == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // BUGBUG: Differenet page,
  //         need follow ProcessGotoOpCode to change formset, form, and question
  //
  ASSERT (SelectQ->PageId == Private->FB.CurrentP->PageId);
  if (SelectQ->PageId != Private->FB.CurrentP->PageId) {
    return EFI_NOT_FOUND;
  }

  Private->FB.CurrentQ = &Statement->Statement;
  gCurrentSelection->Statement  = Statement;
  gCurrentSelection->QuestionId = Statement->QuestionId;
  gCurrentSelection->CurrentMenu->QuestionId = Statement->QuestionId;

  Status = FBBroadcastEvent (Event);

  return Status;
}

/**
 Refresh question.

 @param [in] Private         Formbrowser private data
 @param [in] Event           Display engine event type

 @retval Status              Refresh question status
**/
STATIC
EFI_STATUS
RefreshQuestion (
  IN H2O_FORM_BROWSER_PRIVATE_DATA       *Private,
  IN FORM_BROWSER_STATEMENT              *Question
  )
{
  EFI_STATUS                      Status;
  UI_MENU_SELECTION               *Selection;

  Selection = gCurrentSelection;
  GetQuestionValue (Selection->FormSet, Selection->Form, Question, GetSetValueWithHiiDriver);

  //
  // Reset FormPackage update flag
  //
  mHiiPackageListUpdated = FALSE;

  //
  // Question value may be changed, need invoke its Callback()
  //
  Status = ProcessCallBackFunction(Selection, Selection->FormSet, Selection->Form, Question, EFI_BROWSER_ACTION_RETRIEVE, FALSE);

  if (mHiiPackageListUpdated) {
    //
    // Package list is updated, force to reparse IFR binary of target Formset
    //
    mHiiPackageListUpdated = FALSE;
    gCurrentSelection->Action = UI_ACTION_REFRESH_FORMSET;
    gUserInput->Action        = BROWSER_ACTION_NONE;
  }

  CopyMem (&Question->Statement.HiiValue, &Question->HiiValue, sizeof (EFI_HII_VALUE));
  if (Question->TextTwo != 0) {
    Question->Statement.TextTwo = GetString (Question->TextTwo, Selection->FormSet->HiiHandle);
  }

  return Status;
}


/**
 Formbrowser timer refresh entry list

 @param [in] Private         Formbrowser private data
 @param [in] Event           Display engine event type

 @retval EFI_SUCCESS         Formbrowser timer refresh entry list success
**/
EFI_STATUS
FBTimer (
  IN H2O_FORM_BROWSER_PRIVATE_DATA       *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT        *Event
  )
{
  H2O_DISPLAY_ENGINE_EVT_TIMER           *Timer;
  STATEMENT_REFRESH_ENTRY                *RefreshEntry;
  LIST_ENTRY                             *Link;
  H2O_DISPLAY_ENGINE_PROTOCOL            *TargetDE;

  Timer = (H2O_DISPLAY_ENGINE_EVT_TIMER *)Event;

  ASSERT (Timer->Hdr.Target != H2O_DISPLAY_ENGINE_EVT_TARGET_BROADCAST);

  //
  //Not yet implement; it should send to DE
  //
  if (Timer->Hdr.Target != H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER) {
    if (Event->Target > (INT32) Private->EngineListCount) {
      return EFI_NOT_FOUND;
    }

    TargetDE = Private->EngineList[Event->Target - 1];
    TargetDE->Notify (TargetDE, Event);
    return EFI_SUCCESS;
  }

  switch (Timer->TimerId) {

  case H2O_FORM_BROWSER_TIMER_ID_REFRESH_LIST:
    //
    // refresh entry list
    //
    Link = GetFirstNode (&Private->RefreshList);
    while (!IsNull (&Private->RefreshList, Link)) {
      RefreshEntry = (STATEMENT_REFRESH_ENTRY *) Link;
      Link = GetNextNode (&Private->RefreshList, Link);

      RefreshQuestion (Private, RefreshEntry->Statement);
    }

    mDisplayStatement.OpCode      = Private->FB.CurrentQ->IfrOpCode;
    gUserInput->SelectedStatement = &mDisplayStatement;
    gUserInput->Action            = BROWSER_ACTION_NONE;
    break;

  default:
    ASSERT (FALSE);
  }

  return EFI_SUCCESS;
}

/**
 Get current time in nano second

 @return current time in nano second or 0 if get fail
**/
UINT64
EFIAPI
GetElapsedTimeInNanoSec (
  VOID
  )
{
  EFI_STATUS             Status;
  STATIC EFI_CPU_ARCH_PROTOCOL  *Cpu = NULL;
  UINT64                 CurrentTick;
  UINT64                 TimerPeriod;
  STATIC UINT64          TimerPeriodInPs = 0; // 1e-12 second

  if (Cpu == NULL) {
    Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **) &Cpu);
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      return 0;
    }
  }

  Status = Cpu->GetTimerValue (Cpu, 0, &CurrentTick, NULL);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "H2O form browser get tick fail\n"));
    return 0;
  }

  if (TimerPeriodInPs == 0) {
    Status = Cpu->GetTimerValue (Cpu, 0, &CurrentTick, &TimerPeriod);
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "H2O form browser get tick fail\n"));
      return 0;
    }
    TimerPeriodInPs = DivU64x32 (TimerPeriod, 1000);
  }

  return MultU64x32 (DivU64x32 (CurrentTick, 1000), (UINT32) TimerPeriodInPs);
}

/**
 Formbrowser set timer.

 @param [in] Target           Target of killtimer
 @param [in] TimerId          A id to the function to be notified when the time-out value elapses
 @param [in] Context          Context of time event
 @param [in] TriggerTime      trigger time

 @retval EFI_SUCCESS          Formbrowser set timer successfully
**/
EFI_STATUS
FBSetTimer (
  IN INT32                                 Target,
  IN UINT32                                TimerId,
  IN H2O_FORM_BROWSER_TIMER_TYPE           Type,
  IN CONST H2O_DISPLAY_ENGINE_EVT          *NotifyEvent,
  IN UINT64                                TimeoutInNanoSec
  )
{
  TIMERINFO                                *TimerInfo;
  H2O_DISPLAY_ENGINE_EVT                   *Event;
  LIST_ENTRY                               *Node;

  if (NotifyEvent == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // check current timer lsit to avoid register timer repeatedly
  //
  Node = GetFirstNode (&mFBPrivate.TimerList);
  while (!IsNull (&mFBPrivate.TimerList, Node)) {
    TimerInfo = TIMERINFO_FROM_LINK (Node);
    if ((TimerInfo->Target == Target) &&
        (TimerInfo->TimerId == TimerId) &&
        (TimerInfo->Type == Type) &&
        (TimerInfo->TimeoutInNanoSec == TimeoutInNanoSec)) {
      return EFI_SUCCESS;
    }
    Node = GetNextNode (&mFBPrivate.TimerList, Node);
  }

  TimerInfo = AllocateZeroPool (sizeof (TIMERINFO));
  if (TimerInfo == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Event = AllocateCopyPool (NotifyEvent->Size, NotifyEvent);
  if (Event == NULL) {
    FreePool (TimerInfo);
    return EFI_OUT_OF_RESOURCES;
  }

  TimerInfo->Signature             = H2O_FORM_BROWSER_TIMER_INFO_SIGNATURE;
  TimerInfo->Target                = Target;
  TimerInfo->TimerId               = TimerId;
  TimerInfo->Type                  = Type;
  TimerInfo->NotifyEvent           = Event;
  TimerInfo->TimeoutInNanoSec      = TimeoutInNanoSec;
  TimerInfo->ClockExpiresInNanoSec = GetElapsedTimeInNanoSec () + TimeoutInNanoSec;

  InsertTailList (&mFBPrivate.TimerList, &TimerInfo->Link);

  return EFI_SUCCESS;
}

/**
 Formbrowser kill timer.

 @param [in] Target           Target of killtimer
 @param [in] TimerId          A id to the function to be notified when the time-out value elapses

 @retval TRUE                 Formbrowser kill timer success
 @retval FALSE                Formbrowser kill timer fail
**/
BOOLEAN
FBKillTimer (
  IN  INT32                    Target,
  IN  UINT32                   TimerId
  )
{
  LIST_ENTRY                             *Link;
  H2O_FORM_BROWSER_PRIVATE_DATA          *Private;
  TIMERINFO                              *TimerInfo;

  Private = &mFBPrivate;

  Link = GetFirstNode (&Private->TimerList);
  while (!IsNull (&Private->TimerList, Link)) {
    TimerInfo = TIMERINFO_FROM_LINK (Link);
    Link      = GetNextNode (&Private->TimerList, Link);

    if (TimerInfo->Target == Target && TimerInfo->TimerId == TimerId) {
      RemoveEntryList (&TimerInfo->Link);
      FreePool (TimerInfo->NotifyEvent);
      FreePool (TimerInfo);
      return TRUE;
    }
  }
  return FALSE;
}


/**
 Formbrowser broadcast event.

 @param [in] Event           Display engine event type

 @retval Status              Formbrowser broadcast event status
**/
EFI_STATUS
FBBroadcastEvent (
  IN CONST H2O_DISPLAY_ENGINE_EVT        *Event
  )
{
  EFI_STATUS                             Status;
  H2O_FORM_BROWSER_PRIVATE_DATA          *Private;
  UINT32                                 Index;

  Private = &mFBPrivate;

  Status = EFI_SUCCESS;
  for (Index = 0; Index < Private->EngineListCount; Index++) {
    Status = Private->EngineList[Index]->Notify (Private->EngineList[Index], Event);
  }

  return Status;
}

/**
 Queue of event.

 @param [in] Event           Display engine event type

 @retval EFI_SUCCESS         Queue of event success
**/
EFI_STATUS
QueueEvent (
  H2O_DISPLAY_ENGINE_EVT        *Event
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA  *Private;
  H2O_DISPLAY_ENGINE_EVENT_NODE  *Node;

  Private = &mFBPrivate;

  Node = (H2O_DISPLAY_ENGINE_EVENT_NODE*)AllocateZeroPool (sizeof (H2O_DISPLAY_ENGINE_EVENT_NODE));
  if (Node == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  Node->Signature = H2O_DISPLAY_ENGINE_EVENT_NODE_SIGNATURE;
  Node->Event     = (H2O_DISPLAY_ENGINE_EVT *) AllocatePool (Event->Size);
  if (Node->Event == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (Node->Event, Event, Event->Size);

  EfiAcquireLock (&mEventQueueLock);
  InsertTailList (&mEventQueue, &Node->Link);
  EfiReleaseLock (&mEventQueueLock);

  return EFI_SUCCESS;
}

/**
 Get next queue event.

 @param [in] Event           Display engine event type

 @retval EFI_SUCCESS         Get next queue event successfully
**/
STATIC
BOOLEAN
GetNextQueuedEvent (
  H2O_DISPLAY_ENGINE_EVT        **Event
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA  *Private;
  H2O_DISPLAY_ENGINE_EVENT_NODE  *Node;

  Private = &mFBPrivate;

  ASSERT (Event != NULL);

  EfiAcquireLock (&mEventQueueLock);
  if (IsListEmpty (&mEventQueue)) {
    EfiReleaseLock (&mEventQueueLock);
    return FALSE;
  }

  Node = H2O_DISPLAY_ENGINE_EVENT_NODE_FROM_LINK (
           (LIST_ENTRY *)GetFirstNode (&mEventQueue)
           );
  RemoveEntryList (&Node->Link);
  EfiReleaseLock (&mEventQueueLock);

  *Event = Node->Event;
  FreePool (Node);

  return TRUE;
}

/**
 Queue of event.

 @param [in] NumberOfEvents       Number Of events
 @param [in] UserEvents           Event list
 @param [out] UserIndex           Event index

 @retval EFI_SUCCESS              Queue of event success
**/
STATIC
EFI_STATUS
EFIAPI
FBWaitForEvent (
  IN UINTN        NumberOfEvents,
  IN EFI_EVENT    *UserEvents,
  OUT UINTN       *UserIndex
  )
{
  EFI_STATUS      Status;
  UINTN           Index;

  for (Index = 0; Index < NumberOfEvents; Index++) {

    Status = gBS->CheckEvent (UserEvents[Index]);

    //
    // provide index of event that caused problem
    //
    if (!EFI_ERROR (Status)) {
      *UserIndex = Index;
      return Status;
    }
  }

  return EFI_NOT_READY;
}

STATIC
EFI_STATUS
GetTargetInfo (
  IN  EFI_GUID                                *TargetFormSetGuid,
  IN  UINT16                                  TargetFormId,
  IN  UINT16                                  TargetQuestionId,
  OUT FORM_BROWSER_FORMSET                    **TargetFormSet,
  OUT FORM_BROWSER_FORM                       **TargetForm,
  OUT FORM_BROWSER_STATEMENT                  **TargetStatement
  )
{
  FORM_BROWSER_FORMSET                        *FormSet;
  FORM_BROWSER_FORM                           *Form;
  FORM_BROWSER_STATEMENT                      *Statement;
  LIST_ENTRY                                  *LinkFormSet;
  LIST_ENTRY                                  *LinkForm;
  LIST_ENTRY                                  *LinkStatement;

  if (TargetFormSetGuid == NULL || CompareGuid (TargetFormSetGuid, &gZeroGuid)) {
    return EFI_NOT_FOUND;
  }

  LinkFormSet = GetFirstNode (&mFBPrivate.FormSetList);
  while (!IsNull (&mFBPrivate.FormSetList, LinkFormSet)) {
    FormSet     = FORM_BROWSER_FORMSET_FROM_DISPLAY_LINK (LinkFormSet);
    LinkFormSet = GetNextNode (&mFBPrivate.FormSetList, LinkFormSet);
    if (!CompareGuid (&FormSet->Guid, TargetFormSetGuid)) {
      continue;
    }

    if (TargetFormId == 0 && TargetQuestionId == 0) {
      *TargetFormSet   = FormSet;
      *TargetForm      = NULL;
      *TargetStatement = NULL;
      return EFI_SUCCESS;
    }

    LinkForm = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, LinkForm)) {
      Form     = FORM_BROWSER_FORM_FROM_LINK (LinkForm);
      LinkForm = GetNextNode (&FormSet->FormListHead, LinkForm);

      if (TargetFormId != 0) {
        if (Form->FormId == TargetFormId) {
          if (TargetQuestionId == 0) {
            *TargetFormSet   = FormSet;
            *TargetForm      = Form;
            *TargetStatement = NULL;
            return EFI_SUCCESS;
          }
        } else {
          continue;
        }
      }

      LinkStatement = GetFirstNode (&Form->StatementListHead);
      while (!IsNull (&Form->StatementListHead, LinkStatement)) {
        Statement = FORM_BROWSER_STATEMENT_FROM_LINK (LinkStatement);
        LinkStatement = GetNextNode (&Form->StatementListHead, LinkStatement);
        if (Statement->QuestionId == TargetQuestionId) {
          *TargetFormSet   = FormSet;
          *TargetForm      = Form;
          *TargetStatement = Statement;
          return EFI_SUCCESS;
        }
      }
    }
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
DiscardQuestion (
  IN FORM_BROWSER_FORMSET                *FormSet,
  IN FORM_BROWSER_FORM                   *Form,
  IN FORM_BROWSER_STATEMENT              *Statement
  )
{
  if (FormSet == NULL || Form == NULL || Statement == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Statement->ValueChanged) {
    GetQuestionValue (FormSet, Form, Statement, GetSetValueWithBuffer);
    SetQuestionValue (FormSet, Form, Statement, GetSetValueWithEditBuffer);
    IsQuestionValueChanged(FormSet, Form, Statement, GetSetValueWithBuffer);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
LoadQuestionDefault (
  IN FORM_BROWSER_FORMSET                *FormSet,
  IN FORM_BROWSER_FORM                   *Form,
  IN FORM_BROWSER_STATEMENT              *Statement,
  IN UINT16                              DefaultId
  )
{
  EFI_STATUS                             Status;

  if (FormSet == NULL || Form == NULL || Statement == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetQuestionDefault (FormSet, Form, Statement, DefaultId);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Synchronize Buffer storage's Edit buffer
  //
  if ((Statement->Storage != NULL) &&
      (Statement->Storage->Type != EFI_HII_VARSTORE_EFI_VARIABLE)) {
    SetQuestionValue (FormSet, Form, Statement, GetSetValueWithEditBuffer);
  }

  IsQuestionValueChanged(FormSet, Form, Statement, GetSetValueWithBuffer);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SubmitQuestion (
  IN FORM_BROWSER_STATEMENT              *Statement
  )
{
  EFI_STATUS                             Status;
  CHAR16                                 *ConfigRequest;
  EFI_STRING                             ConfigResp;
  EFI_STRING                             Progress;

  if (Statement == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Statement->Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE ||
      Statement->Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
    return EFI_UNSUPPORTED;
  }

  //
  // 1. Prepare <ConfigResp>
  //
  ConfigRequest = CatSPrint (NULL, L"%s%s", Statement->Storage->ConfigHdr, Statement->BlockName);
  if (ConfigRequest == NULL) {
    return EFI_UNSUPPORTED;
  }

  Status = StorageToConfigResp (Statement->Storage, &ConfigResp, ConfigRequest, TRUE);
  FreePool (ConfigRequest);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // 2. Set value to hii config routine protocol.
  //
  Status = gHiiConfigRouting->RouteConfig (gHiiConfigRouting, ConfigResp, &Progress);
  FreePool (ConfigResp);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // 3. Config success, update storage shadow Buffer
  //
  if (Statement->Storage->Type == EFI_HII_VARSTORE_BUFFER ||
      Statement->Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
    CopyMem (
      Statement->Storage->Buffer     + Statement->VarStoreInfo.VarOffset,
      Statement->Storage->EditBuffer + Statement->VarStoreInfo.VarOffset,
      Statement->StorageWidth
      );
//} else if (Statement->Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
  }
  //
  // 4. Update the NV flag.
  //
  if (Statement->ValueChanged && ((Statement->QuestionFlags & EFI_IFR_FLAG_RESET_REQUIRED) != 0)) {
    gResetRequired = TRUE;
  }
  if (Statement->ValueChanged) {
    Statement->ValueChanged = FALSE;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
FBHotKeyDiscard (
  IN H2O_DISPLAY_ENGINE_EVT_HOT_KEY      *HotKeyEvent
  )
{
  EFI_STATUS                             Status;
  FORM_BROWSER_FORMSET                   *FormSet;
  FORM_BROWSER_FORM                      *Form;
  FORM_BROWSER_STATEMENT                 *Statement;

  if (HotKeyEvent == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (CompareGuid (&HotKeyEvent->HotKeyTargetFormSetGuid, &gZeroGuid)) {
    return DiscardForm (gCurrentSelection->FormSet, gCurrentSelection->Form, gBrowserSettingScope);
  }

  Status = GetTargetInfo (
             &HotKeyEvent->HotKeyTargetFormSetGuid,
             HotKeyEvent->HotKeyTargetFormId,
             HotKeyEvent->HotKeyTargetQuestionId,
             &FormSet,
             &Form,
             &Statement
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (HotKeyEvent->HotKeyTargetFormId == 0 && HotKeyEvent->HotKeyTargetQuestionId == 0) {
    return DiscardForm (FormSet, NULL, FormSetLevel);
  } else if (HotKeyEvent->HotKeyTargetFormId != 0 && HotKeyEvent->HotKeyTargetQuestionId == 0) {
    return DiscardForm (FormSet, Form, FormLevel);
  } else {
    return DiscardQuestion (FormSet, Form, Statement);
  }
}

STATIC
EFI_STATUS
FBHotKeyLoadDefault (
  IN H2O_DISPLAY_ENGINE_EVT_HOT_KEY      *HotKeyEvent
  )
{
  EFI_STATUS                             Status;
  FORM_BROWSER_FORMSET                   *FormSet;
  FORM_BROWSER_FORM                      *Form;
  FORM_BROWSER_STATEMENT                 *Statement;

  if (HotKeyEvent == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (CompareGuid (&HotKeyEvent->HotKeyTargetFormSetGuid, &gZeroGuid)) {
    if (GetScuFormset () != NULL) {
      Status = BroadcastOpenDByHotKeyEvt (H2O_DISPLAY_ENGINE_EVT_TYPE_DEFAULT);
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }

    Status = ExtractDefault (gCurrentSelection->FormSet, gCurrentSelection->Form, HotKeyEvent->HotKeyDefaultId, gBrowserSettingScope, GetDefaultForAll, NULL, FALSE);
    UpdateStatementStatus (gCurrentSelection->FormSet, gCurrentSelection->Form, gBrowserSettingScope);
    return Status;
  }

  Status = GetTargetInfo (
             &HotKeyEvent->HotKeyTargetFormSetGuid,
             HotKeyEvent->HotKeyTargetFormId,
             HotKeyEvent->HotKeyTargetQuestionId,
             &FormSet,
             &Form,
             &Statement
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (HotKeyEvent->HotKeyTargetFormId == 0 && HotKeyEvent->HotKeyTargetQuestionId == 0) {
    Status = ExtractDefault (FormSet, NULL, HotKeyEvent->HotKeyDefaultId, FormSetLevel, GetDefaultForAll, NULL, FALSE);
    UpdateStatementStatus (FormSet, NULL, FormSetLevel);
    return Status;
  } else if (HotKeyEvent->HotKeyTargetFormId != 0 && HotKeyEvent->HotKeyTargetQuestionId == 0) {
    Status = ExtractDefault (FormSet, Form, HotKeyEvent->HotKeyDefaultId, FormLevel, GetDefaultForAll, NULL, FALSE);
    UpdateStatementStatus (FormSet, Form, FormLevel);
    return Status;
  } else {
    return LoadQuestionDefault (FormSet, Form, Statement, HotKeyEvent->HotKeyDefaultId);
  }
}

STATIC
EFI_STATUS
FBHotKeySave (
  IN H2O_DISPLAY_ENGINE_EVT_HOT_KEY      *HotKeyEvent
  )
{
  EFI_STATUS                             Status;
  FORM_BROWSER_FORMSET                   *FormSet;
  FORM_BROWSER_FORM                      *Form;
  FORM_BROWSER_STATEMENT                 *Statement;
  USER_INPUT                             *OrgUserInputData;

  if (HotKeyEvent == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (CompareGuid (&HotKeyEvent->HotKeyTargetFormSetGuid, &gZeroGuid)) {
    OrgUserInputData = gUserInput;
    Status = SubmitForm (gCurrentSelection->FormSet, gCurrentSelection->Form, gBrowserSettingScope);
    gUserInput = OrgUserInputData;
    return Status;
  }

  Status = GetTargetInfo (
             &HotKeyEvent->HotKeyTargetFormSetGuid,
             HotKeyEvent->HotKeyTargetFormId,
             HotKeyEvent->HotKeyTargetQuestionId,
             &FormSet,
             &Form,
             &Statement
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  OrgUserInputData = gUserInput;
  if (HotKeyEvent->HotKeyTargetFormId == 0 && HotKeyEvent->HotKeyTargetQuestionId == 0) {
    Status = SubmitForm (FormSet, NULL, FormSetLevel);
  } else if (HotKeyEvent->HotKeyTargetFormId != 0 && HotKeyEvent->HotKeyTargetQuestionId == 0) {
    Status = SubmitForm (FormSet, Form, FormLevel);
  } else {
    Status = SubmitQuestion (Statement);
  }
  gUserInput = OrgUserInputData;

  return Status;
}

STATIC
EFI_STATUS
FBHotKeyGoTo (
  IN H2O_DISPLAY_ENGINE_EVT_HOT_KEY      *HotKeyEvent
  )
{
  FORM_BROWSER_STATEMENT                 Statement;

  if (HotKeyEvent == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem (&Statement, sizeof (FORM_BROWSER_STATEMENT));
  CopyGuid (&Statement.HiiValue.Value.ref.FormSetGuid, &HotKeyEvent->HotKeyTargetFormSetGuid);
  Statement.HiiValue.Value.ref.FormId     = HotKeyEvent->HotKeyTargetFormId;
  Statement.HiiValue.Value.ref.QuestionId = HotKeyEvent->HotKeyTargetQuestionId;

  return ProcessGotoOpCode (&Statement, gCurrentSelection);
}

STATIC
EFI_STATUS
FBHotKeyCallback (
  IN H2O_DISPLAY_ENGINE_EVT_HOT_KEY      *HotKeyEvent
  )
{
  EFI_HII_HANDLE                         HiiHandle;
  EFI_STATUS                             Status;
  EFI_HANDLE                             DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL         *ConfigAccess;
  EFI_BROWSER_ACTION_REQUEST             ActionRequest;

  if (HotKeyEvent == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (CompareGuid (&HotKeyEvent->HotKeyTargetFormSetGuid, &gZeroGuid)) {
    HiiHandle = gCurrentSelection->Handle;
  } else {
    HiiHandle = FormSetGuidToHiiHandle (&HotKeyEvent->HotKeyTargetFormSetGuid);
  }

  Status = gHiiDatabase->GetPackageListHandle (gHiiDatabase, HiiHandle, &DriverHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->HandleProtocol (DriverHandle, &gEfiHiiConfigAccessProtocolGuid, (VOID **) &ConfigAccess);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return ConfigAccess->Callback (
                         ConfigAccess,
                         H2O_BROWSER_ACTION_HOT_KEY_CALLBACK,
                         HotKeyEvent->HotKeyTargetQuestionId,
                         HotKeyEvent->HotKeyHiiValue.Type,
                         &HotKeyEvent->HotKeyHiiValue.Value,
                         &ActionRequest
                         );
}

STATIC
EFI_STATUS
FBHotKeySetQuestionValue (
  IN H2O_DISPLAY_ENGINE_EVT_HOT_KEY      *HotKeyEvent
  )
{
  EFI_STATUS                             Status;
  FORM_BROWSER_FORMSET                   *FormSet;
  FORM_BROWSER_FORM                      *Form;
  FORM_BROWSER_STATEMENT                 *Statement;
  UINTN                                  StringLength;
  FORM_BROWSER_FORMSET                   *BackupFormSet;
  FORM_BROWSER_FORM                      *BackupForm;
  UINTN                                  Index;

  if (HotKeyEvent == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetTargetInfo (
             &HotKeyEvent->HotKeyTargetFormSetGuid,
             HotKeyEvent->HotKeyTargetFormId,
             HotKeyEvent->HotKeyTargetQuestionId,
             &FormSet,
             &Form,
             &Statement
             );
  if (EFI_ERROR (Status) || Statement == NULL) {
    return Status;
  }

  if (!Statement->Statement.Selectable) {
    return EFI_ABORTED;
  }

  switch (Statement->Operand) {

  case EFI_IFR_STRING_OP:
    if (HotKeyEvent->HotKeyHiiValue.Buffer == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    StringLength = StrLen ((CHAR16 *) HotKeyEvent->HotKeyHiiValue.Buffer);
    if (StringLength < Statement->Minimum || StringLength > Statement->Maximum) {
      return EFI_ABORTED;
    }

    HiiSetString (FormSet->HiiHandle, Statement->HiiValue.Value.string, (CHAR16 *) HotKeyEvent->HotKeyHiiValue.Buffer, NULL);
    CopyMem (Statement->BufferValue, HotKeyEvent->HotKeyHiiValue.Buffer, (StringLength + 1) * sizeof (CHAR16));
    break;

  case EFI_IFR_PASSWORD_OP:
    if (HotKeyEvent->HotKeyHiiValue.Buffer == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    StringLength = StrLen ((CHAR16 *) HotKeyEvent->HotKeyHiiValue.Buffer);
    if (StringLength != 0 && (StringLength < Statement->Minimum || StringLength > Statement->Maximum)) {
      return EFI_ABORTED;
    }

    HiiSetString (FormSet->HiiHandle, Statement->HiiValue.Value.string, (CHAR16 *) HotKeyEvent->HotKeyHiiValue.Buffer, NULL);
    CopyMem (Statement->BufferValue, HotKeyEvent->HotKeyHiiValue.Buffer, (StringLength + 1) * sizeof (CHAR16));

    //
    // Two password match, send it to Configuration Driver
    //
    if ((Statement->QuestionFlags & EFI_IFR_FLAG_CALLBACK) != 0) {
      BackupFormSet              = gCurrentSelection->FormSet;
      BackupForm                 = gCurrentSelection->Form;
      gCurrentSelection->FormSet = FormSet;
      gCurrentSelection->Form    = Form;
      mDisplayStatement.OpCode = Statement->OpCode;
      Status = PasswordCheck (NULL, &mDisplayStatement, (CHAR16 *) Statement->BufferValue);
      ShowPwdStatusMessage (0, Status);

      gCurrentSelection->FormSet = BackupFormSet;
      gCurrentSelection->Form    = BackupForm;
      //
      // Clean the value after saved it.
      //
      ZeroMem (Statement->BufferValue, (StringLength + 1) * sizeof (CHAR16));
      HiiSetString (FormSet->HiiHandle, Statement->HiiValue.Value.string, (CHAR16*) Statement->BufferValue, NULL);
    } else {
      SetQuestionValue (FormSet, Form, Statement, GetSetValueWithHiiDriver);
    }
    break;

  case EFI_IFR_ORDERED_LIST_OP:
    if (HotKeyEvent->HotKeyHiiValue.Buffer == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    if (HotKeyEvent->HotKeyHiiValue.BufferLen > Statement->StorageWidth) {
      return EFI_ABORTED;
    }

    CopyMem (Statement->BufferValue, HotKeyEvent->HotKeyHiiValue.Buffer, HotKeyEvent->HotKeyHiiValue.BufferLen);
    break;

  case EFI_IFR_NUMERIC_OP:
    if (HotKeyEvent->HotKeyHiiValue.Value.u64 > Statement->Maximum ||
        HotKeyEvent->HotKeyHiiValue.Value.u64 < Statement->Minimum) {
      return EFI_ABORTED;
    }
    CopyMem (&Statement->HiiValue, &HotKeyEvent->HotKeyHiiValue, sizeof (EFI_HII_VALUE));
    break;

  case EFI_IFR_ONE_OF_OP:
    for (Index = 0; Index < Statement->Statement.NumberOfOptions; Index++) {
      if (HotKeyEvent->HotKeyHiiValue.Value.u64 == Statement->Statement.Options[Index].HiiValue.Value.u64) {
        break;
      }
    }
    if (Index == Statement->Statement.NumberOfOptions) {
      return EFI_ABORTED;
    }
    CopyMem (&Statement->HiiValue, &HotKeyEvent->HotKeyHiiValue, sizeof (EFI_HII_VALUE));
    break;


  default:
    CopyMem (&Statement->HiiValue, &HotKeyEvent->HotKeyHiiValue, sizeof (EFI_HII_VALUE));
    break;
  }

  //
  // Do the question validation.
  //
  Status = ValueChangedValidation (FormSet, Form, Statement);
  if (!EFI_ERROR (Status) && (Statement->Operand != EFI_IFR_PASSWORD_OP)) {
    SetQuestionValue (FormSet, Form, Statement, GetSetValueWithEditBuffer);
    //
    // Verify whether question value has checked, update the ValueChanged flag in Question.
    //
    IsQuestionValueChanged(FormSet, Form, Statement, GetSetValueWithBuffer);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
FBHotKey (
  IN CONST H2O_DISPLAY_ENGINE_EVT        *Event,
  OUT      BOOLEAN                       *ExitFlag
  )
{
  EFI_STATUS                             Status;
  H2O_DISPLAY_ENGINE_EVT_HOT_KEY         *HotKeyEvent;

  HotKeyEvent = (H2O_DISPLAY_ENGINE_EVT_HOT_KEY *) Event;

  switch (HotKeyEvent->HotKeyAction) {

  case HotKeyDiscard:
    Status = FBHotKeyDiscard (HotKeyEvent);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    gUserInput->Action = BROWSER_ACTION_NONE;
    *ExitFlag          = TRUE;
    break;

  case HotKeyLoadDefault:
    Status = FBHotKeyLoadDefault (HotKeyEvent);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    gUserInput->Action = BROWSER_ACTION_NONE;
    *ExitFlag          = TRUE;
    break;

  case HotKeySave:
    Status = FBHotKeySave (HotKeyEvent);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    gUserInput->Action = BROWSER_ACTION_NONE;
    *ExitFlag          = TRUE;
    break;

  case HotKeyGoTo:
    Status = FBHotKeyGoTo (HotKeyEvent);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    gUserInput->Action = BROWSER_ACTION_GOTO;
    *ExitFlag          = TRUE;
    break;

  case HotKeyCallback:
    Status = FBHotKeyCallback (HotKeyEvent);
    break;

  case HotKeySetQuestionValue:
    Status = FBHotKeySetQuestionValue (HotKeyEvent);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    gCurrentSelection->Action = UI_ACTION_REFRESH_FORM;
    gUserInput->Action        = BROWSER_ACTION_NONE;
    *ExitFlag                 = TRUE;
    break;

  default:
    Status = EFI_UNSUPPORTED;
    break;
  }

  return Status;
}

BOOLEAN
IsValidKey (
  IN EFI_INPUT_KEY                       *Key
  )
{
  if (Key == NULL) {
    return FALSE;
  }

  if (Key->UnicodeChar == CHAR_LINEFEED &&
      Key->ScanCode    == SCAN_NULL) {
    return FALSE;
  }

  return TRUE;
}

/**
 Get next event according to device type.

 @param [in] Event           Display engine event type

 @retval TRUE                Get next event success
 @retval FALSE               Get next event fail

**/
STATIC
BOOLEAN
GetNextEventTimeout (
  H2O_DISPLAY_ENGINE_EVT        **Event,
  UINTN                         TimeOut
  )
{

  EFI_STATUS                          Status;
  UINTN                               Index;
  EFI_INPUT_KEY                       Key;
  EFI_KEY_DATA                        KeyData;
  EFI_SIMPLE_POINTER_STATE            MouseState;
  H2O_DISPLAY_ENGINE_EVT_KEYPRESS     *KeyEvent;
  H2O_DISPLAY_ENGINE_EVT_REL_PTR_MOVE *MouseEvent;
  H2O_DISPLAY_ENGINE_EVT_ABS_PTR_MOVE *AbsPtrEvent;
  H2O_FORM_BROWSER_PRIVATE_DATA       *Private;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL      *SimpleTextIn;
  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL   *SimpleTextInEx;
  EFI_SIMPLE_POINTER_PROTOCOL         *SimplePointer;
  H2O_INPUT_EVENT_DESCRIPTION         *InputEventDesc;
  UINTN                               X;
  UINTN                               Y;
  BOOLEAN                             LeftButton;
  BOOLEAN                             RightButton;

  Private = &mFBPrivate;

  if (Private->SetupMouse != NULL) {
    Status = Private->SetupMouse->QueryState (Private->SetupMouse, &X, &Y, &LeftButton, &RightButton);
    if (!EFI_ERROR (Status)) {
      AbsPtrEvent = (H2O_DISPLAY_ENGINE_EVT_ABS_PTR_MOVE *)AllocateZeroPool (sizeof (H2O_DISPLAY_ENGINE_EVT_ABS_PTR_MOVE));
      if (AbsPtrEvent == NULL) {
        return FALSE;
      }
      AbsPtrEvent->Hdr.Size = sizeof (H2O_DISPLAY_ENGINE_EVT_ABS_PTR_MOVE);
      AbsPtrEvent->Hdr.Type = H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE;
      //
      // Set Target to actived DE
      //
      AbsPtrEvent->Hdr.Target = Private->ActivatedEngine;

      AbsPtrEvent->AbsPtrState.CurrentX = (UINT64)(X);
      AbsPtrEvent->AbsPtrState.CurrentY = (UINT64)(Y);
      if (LeftButton) {
        AbsPtrEvent->AbsPtrState.ActiveButtons |= EFI_ABSP_TouchActive;
      }
      if (RightButton) {
        AbsPtrEvent->AbsPtrState.ActiveButtons |= EFI_ABS_AltActive;
      }
      *Event = (H2O_DISPLAY_ENGINE_EVT*) AbsPtrEvent;
      return TRUE;
    }
  }

  if (mInputEventListCount == 0) {
    return FALSE;
  }

  Index  = 0;
  Status = FBWaitForEvent (
             (UINTN)mInputEventListCount,
             mInputEventList,
             &Index
             );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  InputEventDesc = &mInputEventDescList[Index];

  switch (InputEventDesc->DeviceType) {

  case H2O_FORM_BROWSER_CONSOLE_STI:
    SimpleTextIn = (EFI_SIMPLE_TEXT_INPUT_PROTOCOL*)InputEventDesc->Protocol;
    Status = SimpleTextIn->ReadKeyStroke (SimpleTextIn, &Key);
    if (EFI_ERROR (Status) || !IsValidKey (&KeyData.Key)) {
      return FALSE;
    }
    KeyEvent = (H2O_DISPLAY_ENGINE_EVT_KEYPRESS *)AllocateZeroPool (sizeof (H2O_DISPLAY_ENGINE_EVT_KEYPRESS));
    if (KeyEvent == NULL) {
      return FALSE;
    }
    KeyEvent->Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_KEYPRESS);
    KeyEvent->Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS;
    KeyEvent->Hdr.Target = InputEventDesc->ConDev->ConsoleId;
    CopyMem (&KeyEvent->KeyData.Key, &Key, sizeof (EFI_INPUT_KEY));
    *Event = (H2O_DISPLAY_ENGINE_EVT*)KeyEvent;
    break;

  case H2O_FORM_BROWSER_CONSOLE_STI2:
    SimpleTextInEx = (EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL*)InputEventDesc->Protocol;
    Status = SimpleTextInEx->ReadKeyStrokeEx (SimpleTextInEx, &KeyData);
    if (EFI_ERROR (Status) || !IsValidKey (&KeyData.Key)) {
      return FALSE;
    }
    KeyEvent = (H2O_DISPLAY_ENGINE_EVT_KEYPRESS *)AllocateZeroPool (sizeof (H2O_DISPLAY_ENGINE_EVT_KEYPRESS));
    if (KeyEvent == NULL) {
      return FALSE;
    }
    KeyEvent->Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_KEYPRESS);
    KeyEvent->Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS;
    KeyEvent->Hdr.Target = InputEventDesc->ConDev->ConsoleId;
    CopyMem (&KeyEvent->KeyData, &KeyData, sizeof (EFI_KEY_DATA));
    *Event = (H2O_DISPLAY_ENGINE_EVT*)KeyEvent;
    break;

  case H2O_FORM_BROWSER_CONSOLE_SP:
    SimplePointer = (EFI_SIMPLE_POINTER_PROTOCOL*)InputEventDesc->Protocol;
    Status = SimplePointer->GetState (SimplePointer, &MouseState);
    if (EFI_ERROR (Status)) {
      return FALSE;
    }
    MouseEvent = (H2O_DISPLAY_ENGINE_EVT_REL_PTR_MOVE *)AllocateZeroPool (sizeof (H2O_DISPLAY_ENGINE_EVT_REL_PTR_MOVE));
    if (MouseEvent == NULL) {
      return FALSE;
    }
    MouseEvent->Hdr.Size = sizeof (H2O_DISPLAY_ENGINE_EVT_REL_PTR_MOVE);
    MouseEvent->Hdr.Type = H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE;
    MouseEvent->Hdr.Target = InputEventDesc->ConDev->ConsoleId;
    CopyMem (&MouseEvent->State, &MouseState, sizeof (EFI_SIMPLE_POINTER_STATE));
    *Event = (H2O_DISPLAY_ENGINE_EVT*)MouseEvent;
    break;

  default:
    ASSERT (FALSE);
    return FALSE;
  }

  return TRUE;
}

/**
 Check timer event function.
 If expired time came, add the notify event in queue.
**/
VOID
CheckTimerEvent (
  VOID
  )
{
  LIST_ENTRY                               *StartLink;
  LIST_ENTRY                               *CurrentLink;
  TIMERINFO                                *TimerInfo;

  if (IsListEmpty (&mFBPrivate.TimerList)) {
    return;
  }

  StartLink   = &mFBPrivate.TimerList;
  CurrentLink = StartLink->ForwardLink;

  while (CurrentLink != StartLink) {
    TimerInfo   = TIMERINFO_FROM_LINK (CurrentLink);
    CurrentLink = CurrentLink->ForwardLink;

    if (GetElapsedTimeInNanoSec () >= TimerInfo->ClockExpiresInNanoSec) {
      QueueEvent (TimerInfo->NotifyEvent);

      if (TimerInfo->Type != H2O_FORM_BROWSER_TIMER_TYPE_PERIODIC) {
        FBKillTimer (TimerInfo->Target, TimerInfo->TimerId);
      } else {
        TimerInfo->ClockExpiresInNanoSec = GetElapsedTimeInNanoSec () + TimerInfo->TimeoutInNanoSec;
      }
    }
  }
}

//[-start-170316-IB07400848-add]//
//[-start-170629-IB07400882-modify]//
#ifdef SEG_SNAP_SCU_SUPPORT
VOID
Idle (
  VOID
  )
{
  EFI_STATUS                      Status;
  H2O_MESSAGE_FILTER_PROTOCOL     *MessageFilter;

  Status = gBS->LocateProtocol ( &gH2OMessageFilterProtocolGuid, NULL, &MessageFilter);
  if (!EFI_ERROR(Status)) {
    MessageFilter->Idle ( MessageFilter);
  }
}
#endif
//[-end-170629-IB07400882-modify]//
//[-end-170316-IB07400848-add]//

/**
 Get next event.

 @param [in] Event           Display engine event type

 @retval TRUE                Get next event success
 @retval FALSE               Get next event fail

**/
BOOLEAN
GetNextEvent (
  IN H2O_DISPLAY_ENGINE_EVT        **Event
  )
{
  ASSERT (Event != NULL);

  FBHotPlugEventFunc ();

  if (GetNextQueuedEvent (Event)) {
    return TRUE;
  }

  if (GetNextEventTimeout (Event, 0L)) {
    return TRUE;
  }

//[-start-170316-IB07400848-add]//
//[-start-170629-IB07400882-modify]//
#ifdef SEG_SNAP_SCU_SUPPORT
  Idle ();
#endif
//[-end-170629-IB07400882-modify]//
//[-end-170316-IB07400848-add]//

  CheckTimerEvent ();

  return FALSE;
}

/**
 Destroy event queue.

 @param [in] Private         Formbrowser private data

**/
VOID
DestroyEventQueue (
  IN H2O_FORM_BROWSER_PRIVATE_DATA       *Private
  )
{
  H2O_DISPLAY_ENGINE_EVT               *Event;
  BOOLEAN                              IsEmpty;

  for (;;) {
    EfiAcquireLock (&mEventQueueLock);
    IsEmpty = IsListEmpty (&mEventQueue);
    EfiReleaseLock (&mEventQueueLock);
    if (IsEmpty) {
      break;
    }

    GetNextQueuedEvent (&Event);
    FreePool (Event);
  }
}

/**
 Convert event type to string.

 @param [in] Event           Display engine event type

 @return Event string        Get Event string

**/
CHAR16 *EventToStr(IN CONST H2O_EVT_TYPE EvtType)
{
#ifdef _MSC_VER
#define EVENTSTR(x) if(EvtType==x) return (L#x)
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_TIMER);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_L);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_L);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_P);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_P);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_Q);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_Q);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_REFRESH);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_REFRESH_Q);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_Q);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_P);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGE_Q);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_DEFAULT);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_DEFAULT_Q);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_DEFAULT_P);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_EXIT);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT_EXIT);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD_EXIT);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_EXIT_P);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT_EXIT_P);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD_EXIT_P);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT_P);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD_P);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_D);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_D);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_TIMER);
  EVENTSTR(H2O_DISPLAY_ENGINE_EVT_TYPE_SHOW_HELP);
#endif
  return L"Unknown";
}


/**
 Formbrowser event callback function.

 @param [in] Event           Display engine event type

 @retval Status              Formbrowser event callback function status
**/
EFI_STATUS
FBEventCallback (
  IN CONST H2O_DISPLAY_ENGINE_EVT        *Event,
  OUT      BOOLEAN                       *ExitFlag
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA          *Private;
  EFI_STATUS                             Status;
  H2O_DISPLAY_ENGINE_EVT_CHANGE_Q        *ChangeQ;
  H2O_DISPLAY_ENGINE_EVT_SELECT_P        *SelectP;
  UINT16                                 HiiHandleIndex;
  UINT16                                 FormId;
  UINT8                                  Operand;
  FORM_BROWSER_STATEMENT                 *Statement;
  FORM_ENTRY_INFO                        *MenuList;
  UINT32                                 Request;
  BOOLEAN                                ShowSubmitDialog;
  SETUP_DATE_CHANGE                      SetupDateChange;

  Private = &mFBPrivate;
  *ExitFlag = FALSE;

  if (Event == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem (gUserInput, sizeof (USER_INPUT));

  Status    = EFI_SUCCESS;

  switch (Event->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS:
    Status = FBKeyPress (Private, Event);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE:
    Status = FBNotifyEventToTarget (Private, Event);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_L:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_L:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_P:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_P:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_REFRESH_Q:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_D:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_D:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_Q:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_REFRESH:
    Status = FBBroadcastEvent (Event);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_Q:
    Operand = Private->FB.CurrentQ->Operand;

    if (Operand == EFI_IFR_REF_OP ||
        Operand == EFI_IFR_ACTION_OP ||
        Operand == EFI_IFR_RESET_BUTTON_OP) {
      mDisplayStatement.OpCode      = Private->FB.CurrentQ->IfrOpCode;
      gUserInput->SelectedStatement = &mDisplayStatement;
      *ExitFlag = TRUE;
    } else if (Operand != EFI_IFR_CHECKBOX_OP) {
      //
      // Process checkbox in ChangeQ directly, not OpenQ
      //
      BroadcastOpenDByQ (Private->FB.CurrentQ);
    }
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGE_Q:
    if (Private->FB.CurrentQ == NULL || !Private->FB.CurrentQ->Selectable) {
      break;
    }
    ChangeQ = (H2O_DISPLAY_ENGINE_EVT_CHANGE_Q *) Event;
    mDisplayStatement.OpCode = Private->FB.CurrentQ->IfrOpCode;
    gUserInput->SelectedStatement = &mDisplayStatement;
    CopyHiiValue (&gUserInput->InputValue, &ChangeQ->HiiValue);

    Statement = GetBrowserStatement(gUserInput->SelectedStatement);
    if (Statement != NULL && (Statement->Operand == EFI_IFR_STRING_OP || Statement->Operand == EFI_IFR_PASSWORD_OP)) {
      gUserInput->InputValue.Value.string = HiiSetString (gCurrentSelection->FormSet->HiiHandle, 0, (EFI_STRING) ChangeQ->HiiValue.Buffer, NULL);
    }
    *ExitFlag = TRUE;
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_Q:
    Status = FBSelectQuestion (Private, Event);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_P:
    SelectP = (H2O_DISPLAY_ENGINE_EVT_SELECT_P *) Event;
    HiiHandleIndex = (UINT16) (SelectP->PageId >> 16);
    FormId         = (UINT16) (SelectP->PageId & 0xFFFF);

    if (HiiHandleIndex > Private->HiiHandleCount) {
      break;
    }

    if (HiiHandleIndex == Private->HiiHandleIndex) {
      if (FormId == gCurrentSelection->FormId) {
        //
        // do nothing
        //
        break;
      }
      gCurrentSelection->FormId     = FormId;
      gCurrentSelection->QuestionId = 0;
      gCurrentSelection->Action     = UI_ACTION_REFRESH_FORM;
    } else {
      Private->HiiHandleIndex        = (UINTN) HiiHandleIndex;
      gCurrentSelection->Handle      = Private->HiiHandleList[Private->HiiHandleIndex];
      gCurrentSelection->FormId      = FormId;
      gCurrentSelection->QuestionId  = 0;
      gCurrentSelection->CurrentMenu = NULL;
      gCurrentSelection->Action      = UI_ACTION_REFRESH_FORMSET;
      CopyGuid (&gCurrentSelection->FormSetGuid, &gZeroGuid);

      //
      // Clear the menu history data.
      //
      while (!IsListEmpty (&mFBPrivate.FormBrowserEx2.FormViewHistoryHead)) {
        MenuList = FORM_ENTRY_INFO_FROM_LINK (mFBPrivate.FormBrowserEx2.FormViewHistoryHead.ForwardLink);
        RemoveEntryList (&MenuList->Link);
        FreePool (MenuList);
      }
    }
    gUserInput->Action = BROWSER_ACTION_NONE;
    *ExitFlag = TRUE;
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_EXIT:
    FBBroadcastEvent (Event);
    gUserInput->Action = BROWSER_ACTION_NONE;
    gCurrentSelection->Action = UI_ACTION_EXIT;
    *ExitFlag = TRUE;
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_DEFAULT:
    if (GetScuFormset () != NULL) {
      Status = BroadcastOpenDByHotKeyEvt (H2O_DISPLAY_ENGINE_EVT_TYPE_DEFAULT);
      if (EFI_ERROR (Status)) {
        break;
      }
    }

    if (FeaturePcdGet(PcdH2OSetupChangeDisplaySupported)) {
      SetupDateChange.SetTime = FALSE;
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
    gUserInput->Action    = BROWSER_ACTION_DEFAULT;
    gUserInput->DefaultId = EFI_HII_DEFAULT_CLASS_STANDARD;
    *ExitFlag = TRUE;
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT_EXIT:
    if (GetScuFormset () != NULL ||
        CompareGuid (gCurrentSelection->FormSet->ClassGuid, &mSecureBootMgrFormSetGuid)) {

      gUserInput->Action = BROWSER_ACTION_NONE;
      Request = BROWSER_ACTION_NONE;
      ShowSubmitDialog = TRUE;
      Status = H2OSubmitSvcManager (&Request, &ShowSubmitDialog);
      if (!EFI_ERROR (Status)) {
        gUserInput->Action = Request;

        if ((Request & BROWSER_ACTION_RESET) == BROWSER_ACTION_RESET) {
          gCurrentSelection->Action = UI_ACTION_EXIT;
        }
      }

      if (Status == EFI_NOT_FOUND || ShowSubmitDialog) {
        //
        // There is no H2OSubmitSvc protocol installed.
        //

        //
        // Show original submit dialog if it is not hooked.
        //
        Status = BroadcastOpenDByHotKeyEvt (H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT_EXIT);
        if (EFI_ERROR (Status)) {
          break;
        }

        gUserInput->Action |= BROWSER_ACTION_SUBMIT | BROWSER_ACTION_RESET;
        gCurrentSelection->Action = UI_ACTION_EXIT;
      }
    } else {
      gUserInput->Action = BROWSER_ACTION_SUBMIT;
    }
    *ExitFlag = TRUE;
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD_EXIT:
    if (!FormExitPolicy()) {
      break;
    }
    gUserInput->Action = BROWSER_ACTION_FORM_EXIT;
    *ExitFlag = TRUE;
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHOW_HELP:
    Status = BroadcastOpenDByHotKeyEvt (Event->Type);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_TIMER:
    Status = FBTimer (Private, Event);
    if (gUserInput->Action != 0) {
      *ExitFlag = TRUE;
    }
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_HOT_KEY:
    Status = FBHotKey (Event, ExitFlag);
    break;

  default:
    DEBUG ((EFI_D_INFO, "Invalid H2O Display Engine Event: %d\n", Event->Type));
    ASSERT (FALSE);
    break;
  }

  return Status;
}

/**
  OEM specifies whether Setup exits Page by ESC key.

  This function customized the behavior that whether Setup exits Page so that
  system able to boot when configuration is not changed.

  @retval  TRUE     Exits FrontPage
  @retval  FALSE    Don't exit FrontPage.
**/
BOOLEAN
EFIAPI
FormExitPolicy (
  VOID
  )
{
  if (CompareGuid (&gCurrentSelection->FormSet->Guid, &gFrontPageFormSetGuid)) {
    return FALSE;
  }
  return TRUE;
}


/**
  Confirm how to handle the changed data.

  @return Action BROWSER_ACTION_SUBMIT, BROWSER_ACTION_DISCARD or other values.
**/
UINTN
EFIAPI
ConfirmDataChange (
  VOID
  )
{
  EFI_STATUS                    Status;
  SETUP_DATE_CHANGE             SetupDateChange;

  if (GetScuFormset () != NULL ||
      CompareGuid (gCurrentSelection->FormSet->ClassGuid, &mSecureBootMgrFormSetGuid)) {
    Status = BroadcastOpenDByHotKeyEvt (H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD_EXIT);
    if (!EFI_ERROR (Status)) {
      gCurrentSelection->Action = UI_ACTION_EXIT;
      if (FeaturePcdGet(PcdH2OSetupChangeDisplaySupported)) {
        SetupDateChange.SetTime = FALSE;
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
      return BROWSER_ACTION_DISCARD;
    } else {
      return BROWSER_ACTION_NONE;
    }
  } else {
    Status = BroadcastOpenDByHotKeyEvt (H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD_EXIT);
    if (!EFI_ERROR (Status)) {
      return BROWSER_ACTION_DISCARD;
    } else {
      return BROWSER_ACTION_NONE;
    }
  }
}

VOID
BrowserStatusProcess (
  VOID
  )
{
  CHAR16                        *ErrorInfo;
  UINT8                         TimeOut;
  EFI_STRING_ID                 StringToken;
  H2O_FORM_BROWSER_D            Dialog;
  EFI_IFR_OP_HEADER             *OpCodeBuf;

  if (gFormData->BrowserStatus == BROWSER_SUCCESS) {
    return;
  }

  StringToken          = 0;
  OpCodeBuf            = NULL;

  if (gFormData->HighLightedStatement != NULL) {
    OpCodeBuf = gFormData->HighLightedStatement->OpCode;
  }

  if (gFormData->BrowserStatus == (BROWSER_WARNING_IF)) {
    ASSERT (OpCodeBuf != NULL && OpCodeBuf->OpCode == EFI_IFR_WARNING_IF_OP);
    if (OpCodeBuf != NULL && OpCodeBuf->OpCode == EFI_IFR_WARNING_IF_OP) {
      TimeOut     = ((EFI_IFR_WARNING_IF *) OpCodeBuf)->TimeOut;
      StringToken = ((EFI_IFR_WARNING_IF *) OpCodeBuf)->Warning;
    }
  } else {
    TimeOut = 0;
    if ((gFormData->BrowserStatus == (BROWSER_NO_SUBMIT_IF)) &&
        (OpCodeBuf != NULL && OpCodeBuf->OpCode == EFI_IFR_NO_SUBMIT_IF_OP)) {
      StringToken = ((EFI_IFR_NO_SUBMIT_IF *) OpCodeBuf)->Error;
    } else if ((gFormData->BrowserStatus == (BROWSER_INCONSISTENT_IF)) &&
               (OpCodeBuf != NULL && OpCodeBuf->OpCode == EFI_IFR_INCONSISTENT_IF_OP)) {
      StringToken = ((EFI_IFR_INCONSISTENT_IF *) OpCodeBuf)->Error;
    }
  }

  if (StringToken != 0) {
    ErrorInfo = GetString (StringToken, gCurrentSelection->FormSet->HiiHandle);
  } else if (gFormData->ErrorString != NULL) {
    //
    // Only used to compatible with old setup browser.
    // Not use this field in new browser core.
    //
    ErrorInfo = gFormData->ErrorString;
  } else {
    switch (gFormData->BrowserStatus) {
    case BROWSER_SUBMIT_FAIL:
      ErrorInfo = gSaveFailed;
      break;

    case BROWSER_FORM_NOT_FOUND:
      ErrorInfo = gFormNotFound;
      break;

    case BROWSER_FORM_SUPPRESS:
      ErrorInfo = gFormSuppress;
      break;

    case BROWSER_PROTOCOL_NOT_FOUND:
      ErrorInfo = gProtocolNotFound;
      break;

    default:
      ErrorInfo = gBrwoserError;
      break;
    }
  }

  if (ErrorInfo != NULL) {
    CreateSimpleDialog (H2O_FORM_BROWSER_D_TYPE_SELECTION, 0, NULL, 1, &ErrorInfo, 1, &Dialog);
  }

  if (StringToken != 0) {
    FreePool (ErrorInfo);
  }
}

/**
  Check if the question value is valid or not

  @param[in] Question            The pointer to check question

  @retval EFI_SUCCESS            Question value is valid
  @retval EFI_NOT_FOUND          Question value is mismatch
**/
EFI_STATUS
CheckQuestionValue (
  IN FORM_BROWSER_STATEMENT       *Question
  )
{
  EFI_STATUS                      Status;
  EFI_HII_VALUE                   HiiValue;
  UINTN                           Index;
  UINTN                           Index2;
  UINT8                           *ValueArray;
  UINT8                           ValueType;
  EFI_IFR_ORDERED_LIST            *OrderList;
  BOOLEAN                         ValueInvalid;
  QUESTION_OPTION                 *OneOfOption;
  EFI_HII_VALUE                   *QuestionHiiValue;
  H2O_FORM_BROWSER_O              *Option;
  H2O_FORM_BROWSER_S              *Statement;
  CHAR16                          *ErrorMsg;
  CHAR16                          *MismatchInfo;
  CHAR16                          *StrArray[2];
  H2O_FORM_BROWSER_D              Dialog;

  Status           = EFI_SUCCESS;
  QuestionHiiValue = &Question->HiiValue;
  Statement        = &Question->Statement;

  switch (Question->OpCode->OpCode) {

  case EFI_IFR_ONE_OF_OP:
    if (Statement->NumberOfOptions == 0) {
      break;
    }

    OneOfOption = ValueToOption (Question, QuestionHiiValue);
    if (OneOfOption != NULL) {
      break;
    }

    ErrorMsg = GetString (STRING_TOKEN (OPTION_MISMATCH), mHiiHandle);
    MismatchInfo = CatSPrint (NULL, L"(%s: mismatch value is %d.)", Statement->Prompt, QuestionHiiValue->Value.u64);
    if (ErrorMsg != NULL && MismatchInfo != NULL) {
      DestroyEventQueue (&mFBPrivate);
      StrArray[0]  = ErrorMsg;
      StrArray[1]  = MismatchInfo;
      ZeroMem (&Dialog, sizeof (H2O_FORM_BROWSER_D));
      CreateSimpleDialog (H2O_FORM_BROWSER_D_TYPE_MSG, 0, NULL, 2, StrArray, 1, &Dialog);
      FreePool ((VOID *) ErrorMsg);
      FreePool ((VOID *) MismatchInfo);
    }

    //
    // Force the Question value to be valid
    // Exit current DisplayForm with new value.
    //
    Option                        = &Statement->Options[0];
    mDisplayStatement.OpCode      = Question->OpCode;
    gUserInput->SelectedStatement = &mDisplayStatement;
    gUserInput->InputValue.Type   = Option->HiiValue.Type;

    switch (gUserInput->InputValue.Type) {

    case EFI_IFR_TYPE_NUM_SIZE_8:
      gUserInput->InputValue.Value.u8 = Option->HiiValue.Value.u8;
      break;

    case EFI_IFR_TYPE_NUM_SIZE_16:
      CopyMem (&gUserInput->InputValue.Value.u16, &Option->HiiValue.Value.u16, sizeof (UINT16));
      break;

    case EFI_IFR_TYPE_NUM_SIZE_32:
      CopyMem (&gUserInput->InputValue.Value.u32, &Option->HiiValue.Value.u32, sizeof (UINT32));
      break;

    case EFI_IFR_TYPE_NUM_SIZE_64:
      CopyMem (&gUserInput->InputValue.Value.u64, &Option->HiiValue.Value.u64, sizeof (UINT64));
      break;

    default:
      ASSERT (FALSE);
      break;
    }
    return EFI_NOT_FOUND;

  case EFI_IFR_ORDERED_LIST_OP:
    if (Statement->NumberOfOptions == 0) {
      break;
    }

    OrderList  = (EFI_IFR_ORDERED_LIST *) Question->OpCode;
    Option     = &Statement->Options[0];
    ValueType  = Option->HiiValue.Type;
    ValueArray = QuestionHiiValue->Buffer;

    HiiValue.Type      = ValueType;
    HiiValue.Value.u64 = 0;
    for (Index = 0; Index < OrderList->MaxContainers; Index++) {
      HiiValue.Value.u64 = GetArrayData (ValueArray, ValueType, Index);
      if (HiiValue.Value.u64 == 0) {
        //
        // Values for the options in ordered lists should never be a 0
        //
        break;
      }

      OneOfOption = ValueToOption (Question, &HiiValue);
      if (OneOfOption != NULL) {
        continue;
      }

      ErrorMsg = GetString (STRING_TOKEN (OPTION_MISMATCH), mHiiHandle);
      MismatchInfo = CatSPrint (NULL, L"(%s: mismatch value is %d.)", Statement->Prompt, HiiValue.Value.u64);
      if (ErrorMsg != NULL && MismatchInfo != NULL) {
        DestroyEventQueue (&mFBPrivate);
        StrArray[0]  = ErrorMsg;
        StrArray[1]  = MismatchInfo;
        ZeroMem (&Dialog, sizeof (H2O_FORM_BROWSER_D));
        CreateSimpleDialog (H2O_FORM_BROWSER_D_TYPE_MSG, 0, NULL, 2, StrArray, 1, &Dialog);
        FreePool ((VOID *) ErrorMsg);
        FreePool ((VOID *) MismatchInfo);
      }

      //
      // The initial value of the orderedlist is invalid, force to be valid value
      // Exit current DisplayForm with new value.
      //
      mDisplayStatement.OpCode      = Question->OpCode;
      gUserInput->SelectedStatement = &mDisplayStatement;

      ValueArray = AllocateZeroPool (QuestionHiiValue->BufferLen);
      ASSERT (ValueArray != NULL);
      if (ValueArray == NULL) {
        break;
      }
      gUserInput->InputValue.Buffer    = ValueArray;
      gUserInput->InputValue.BufferLen = QuestionHiiValue->BufferLen;
      gUserInput->InputValue.Type      = QuestionHiiValue->Type;

      for (Index2 = 0; Index2 < Statement->NumberOfOptions && Index2 < OrderList->MaxContainers; Index2++) {
        Option = &Statement->Options[Index2];
        SetArrayData (ValueArray, ValueType, Index2, Option->HiiValue.Value.u64);
      }
      if (Index2 < OrderList->MaxContainers) {
        SetArrayData (ValueArray, ValueType, Index2, 0);
      }
      return EFI_NOT_FOUND;
    }

    //
    // If valid option more than the max container, skip these options.
    //
    if (Index >= OrderList->MaxContainers) {
      break;
    }

    //
    // Search the other options, try to find the one not in the container.
    //
    ValueArray = QuestionHiiValue->Buffer;
    ValueInvalid = FALSE;
    for (Index2 = 0; Index2 < Statement->NumberOfOptions; Index2++) {
      Option = &Statement->Options[Index2];
      if (FindArrayData (ValueArray, ValueType, Option->HiiValue.Value.u64, NULL)) {
        continue;
      }

      if (!ValueInvalid) {
        ValueInvalid = TRUE;

        ErrorMsg = GetString (STRING_TOKEN (OPTION_MISMATCH), mHiiHandle);
        if (ErrorMsg != NULL) {
          DestroyEventQueue (&mFBPrivate);
          ZeroMem (&Dialog, sizeof (H2O_FORM_BROWSER_D));
          CreateSimpleDialog (H2O_FORM_BROWSER_D_TYPE_MSG, 0, NULL, 1, &ErrorMsg, 1, &Dialog);
          FreePool ((VOID *) ErrorMsg);
        }

        //
        // The initial value of the orderedlist is invalid, force to be valid value
        // Exit current DisplayForm with new value.
        //
        mDisplayStatement.OpCode      = Question->OpCode;
        gUserInput->SelectedStatement = &mDisplayStatement;

        ValueArray = AllocateCopyPool (QuestionHiiValue->BufferLen, QuestionHiiValue->Buffer);
        ASSERT (ValueArray != NULL);
        if (ValueArray == NULL) {
          break;
        }
        gUserInput->InputValue.Buffer    = ValueArray;
        gUserInput->InputValue.BufferLen = QuestionHiiValue->BufferLen;
        gUserInput->InputValue.Type      = QuestionHiiValue->Type;
      }

      SetArrayData (ValueArray, ValueType, Index++, Option->HiiValue.Value.u64);
    }

    if (ValueInvalid) {
      return EFI_NOT_FOUND;
    }
    break;

  default:
    break;
  }

  return Status;
}

/**
 Check if there is a question value is mismatch

 @param[in] Form      The pointer of form which contains Questions

 @retval TRUE         There is a question value is mismatch
 @retval FALSE        There is not a question value is mismatch
**/
BOOLEAN
IsQuestionValueMismatch (
  IN FORM_BROWSER_FORM                        *Form
  )
{
  FORM_BROWSER_STATEMENT                      *Statement;
  LIST_ENTRY                                  *Link;

  if (Form == NULL) {
    return FALSE;
  }

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Statement = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
    Link      = GetNextNode (&Form->StatementListHead, Link);
    if (EvaluateExpressionList(Statement->Expression, FALSE, NULL, NULL) >= ExpressSuppress) {
      continue;
    }

    if (CheckQuestionValue (Statement) == EFI_NOT_FOUND) {
      return TRUE;
    }
  }

  return FALSE;
}

EFI_STATUS
EFIAPI
FormDisplay (
  IN  FORM_DISPLAY_ENGINE_FORM  *FormData,
  OUT USER_INPUT                *UserInputData
  )
{
  H2O_DISPLAY_ENGINE_EVT        *Event;
  BOOLEAN                       ExitFlag;
  H2O_DISPLAY_ENGINE_EVT_TIMER  TimerEvent;
  STATEMENT_REFRESH_ENTRY       *RefreshEntry;
  UINT64                        MinTime;
  LIST_ENTRY                    *Link;
  H2O_DISPLAY_ENGINE_EVT_OPEN_P OpenP;

  ASSERT (FormData != NULL);
  if (FormData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  gUserInput = UserInputData;
  gFormData  = FormData;

  //
  // Process the status info first.
  //
  BrowserStatusProcess();
  if (UserInputData == NULL) {
    //
    // UserInputData == NULL, means only need to print the error info, return here.
    //
    return EFI_SUCCESS;
  }

  //
  // SetTimer  300ms
  // Because system timer isn't on time,
  // so Time / Date refresh rate should be littler than 1s;
  //
  if (!IsListEmpty (&mFBPrivate.RefreshList)) {
    MinTime = (UINT64) (-1);
    Link = GetFirstNode (&mFBPrivate.RefreshList);
    while (!IsNull (&mFBPrivate.RefreshList, Link)) {
      RefreshEntry = (STATEMENT_REFRESH_ENTRY *) Link;
      Link         = GetNextNode (&mFBPrivate.RefreshList, Link);
      MinTime = (RefreshEntry->Statement->RefreshInterval < MinTime) ? RefreshEntry->Statement->RefreshInterval : MinTime;
    }
    //
    // It should set timer for each item in refresh list. Temporarily set 300ms to refresh.
    //
    TimerEvent.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_TIMER);
    TimerEvent.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_TIMER;
    TimerEvent.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;
    TimerEvent.TimerId    = H2O_FORM_BROWSER_TIMER_ID_REFRESH_LIST;
    TimerEvent.Time       = MultU64x32(MinTime, 1000000000); // 1 second

    FBSetTimer (
      H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER,
      H2O_FORM_BROWSER_TIMER_ID_REFRESH_LIST,
      H2O_FORM_BROWSER_TIMER_TYPE_PERIODIC,
      (CONST H2O_DISPLAY_ENGINE_EVT *) &TimerEvent,
      TimerEvent.Time
      );
  }

  FBUpdateSMInfo (&mFBPrivate);
  if (mFBPrivate.Repaint) {
    FBRepaint (&mFBPrivate);
    mFBPrivate.Repaint = FALSE;
  } else {
    ZeroMem (&OpenP, sizeof (H2O_DISPLAY_ENGINE_EVT_OPEN_P));
    OpenP.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_OPEN_P);
    OpenP.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;
    OpenP.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_P;
    FBBroadcastEvent ((H2O_DISPLAY_ENGINE_EVT*)&OpenP);
  }
  if (IsQuestionValueMismatch (gCurrentSelection->Form)) {
    return EFI_NOT_FOUND;
  }

  //
  // event loop
  //
  Event = NULL;
  while (1) {
    if (!GetNextEvent (&Event)) {
      if (mRefreshFormSet) {
        gUserInput->Action        = BROWSER_ACTION_NONE;
        gCurrentSelection->Action = UI_ACTION_REFRESH_FORMSET;
        break;
      }
      continue;
    }
    FBEventCallback (Event, &ExitFlag);
    if (ExitFlag) {
      FreePool (Event);
      break;
    }
    FreePool (Event);
  }

  mRefreshFormSet = FALSE;
  DestroyEventQueue (&mFBPrivate);

  return EFI_SUCCESS;
}


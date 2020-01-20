/** @file
  Initialization and display related functions for setup page panel.

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

#include "LTDEPanels.h"
#include "LTDEQuestions.h"
#include "LTDEPrint.h"
#include "LTDESupport.h"
#include "LTDEMisc.h"

#define PROMPT_VALUE_SEPARATOR_WIDTH          1
#define ARROW_TEXT                            (EFI_RED | EFI_BRIGHT)
CHAR16                                        mPageUpStr[2]   = {ARROW_UP, CHAR_NULL};
CHAR16                                        mPageDownStr[2] = {ARROW_DOWN, CHAR_NULL};

typedef enum {
  LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_PAGE_UP = 0,
  LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_PAGE_DOWN,
  LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_HOME,
  LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_END,
  LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_MAX,
} LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION;

typedef struct {
  UINT32                                      Action;
  EFI_INPUT_KEY                               InputKey;
  CHAR16                                      *String;
} LTDE_SETUP_PAGE_PANEL_HOT_KEY_INFO;

LTDE_SETUP_PAGE_PANEL_HOT_KEY_INFO            mSetupPagePanelHotKeyInfoList[] = {
  {LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_PAGE_UP  , {SCAN_PAGE_UP  , CHAR_NULL}, mPageUpStr},
  {LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_PAGE_DOWN, {SCAN_PAGE_DOWN, CHAR_NULL}, mPageDownStr},
  {LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_HOME     , {SCAN_HOME     , CHAR_NULL}, L""},
  {LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_END      , {SCAN_END      , CHAR_NULL}, L""},
  };

/**
  Check if input question is valid highlight or not

  @param[in] Control             A pointer to control instance

  @retval TRUE                   Query question is valid highlight control
  @retval FALSE                  Query question is not valid highlight control

**/
STATIC
BOOLEAN
IsValidHighlightControl (
  IN H2O_LTDE_CONTROL                         *Control
  )
{
  if (Control == NULL || Control->ControlStyle.PseudoClass == H2O_STYLE_PSEUDO_CLASS_GRAYOUT) {
    return FALSE;
  }

  return Control->Selectable;
}

/**
  Check if input question ID is current highlight or not

  @param[in] PageId              Paeg ID
  @param[in] QuestionId          Query question ID
  @param[in] IfrOpCode           IFR opcode pointer of Query question

  @retval TRUE                   Query question is current highlight
  @retval FALSE                  Query question is not current highlight

**/
BOOLEAN
IsCurrentHighlight (
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  )
{
  H2O_LTDE_PANEL                              *SetupPagePanel;

  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);

  if (SetupPagePanel != NULL &&
      SetupPagePanel->SelectedControl != NULL &&
      SetupPagePanel->SelectedControl->PageId == PageId &&
      ((QuestionId != 0    && SetupPagePanel->SelectedControl->QuestionId == QuestionId) ||
       (IfrOpCode  != NULL && SetupPagePanel->SelectedControl->IfrOpCode  == IfrOpCode))) {
    return TRUE;
  }

  return FALSE;
}

/**
  Get valid highlight control in current display position of content item.

  @param[in] ContentItem         A pointer to content item

  @return The pointer of valid highlight control or NULL if not found

**/
STATIC
H2O_LTDE_CONTROL *
GetValidHighlightControl (
  IN H2O_LTDE_PANEL_ITEM                      *ContentItem
  )
{
  INT32                                       StartY;
  INT32                                       EndY;
  UINT32                                      Index;
  H2O_LTDE_CONTROL                            *Control;

  if (ContentItem == NULL) {
    return NULL;
  }

  StartY = ContentItem->CurrentPos;
  EndY   = ContentItem->CurrentPos + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - 1;

  for (Index = 0; Index < ContentItem->ControlCount; Index++) {
    Control = &ContentItem->ControlList[Index];
    if (IS_OVERLAP (Control->ControlField.top, Control->ControlField.bottom, StartY, EndY) &&
        IsValidHighlightControl (Control)) {
      return Control;
    }
  }

  return NULL;
}

/**
  Get hot key info of setup page panel by hot key action.

  @param[in] HotKeyAction         Hot key action.

  @return The pointer of hot key info or NULL if not found

**/
STATIC
LTDE_SETUP_PAGE_PANEL_HOT_KEY_INFO *
GetSetupPageHotKeyInfo (
  IN UINT32                                   HotKeyAction
  )
{
  UINT32                                      Index;

  for (Index = 0; Index < sizeof (mSetupPagePanelHotKeyInfoList) / sizeof (LTDE_SETUP_PAGE_PANEL_HOT_KEY_INFO); Index++) {
    if (mSetupPagePanelHotKeyInfoList[Index].Action == HotKeyAction) {
      return &mSetupPagePanelHotKeyInfoList[Index];
    }
  }

  return NULL;
}

/**
  Get prompt string of question

  @param[in] Question           A pointer to question

  @return The pointer of prompt string or NULL if not found

**/
STATIC
CHAR16 *
GetQuestionPromptStr (
  IN H2O_FORM_BROWSER_Q                       *Question
  )
{
  CHAR16                                      *QuestionPromptStr;
  UINTN                                       QuestionPromptStrSize;
  CHAR16                                      *PromptStr;

  if (Question == NULL) {
    return NULL;
  }

  QuestionPromptStr     = (Question->Prompt != NULL && *Question->Prompt != CHAR_NULL) ? Question->Prompt : L" ";
  QuestionPromptStrSize = StrSize (QuestionPromptStr);

  switch (Question->Operand) {

  case EFI_IFR_ORDERED_LIST_OP:
    PromptStr = OrderListOpCodeCreatePromptStr (Question);
    break;

  case EFI_IFR_REF_OP:
    PromptStr = AllocateZeroPool (QuestionPromptStrSize + sizeof (CHAR16));
    if (PromptStr != NULL) {
      PromptStr[0] = REF_OP_DELIMITER;
      CopyMem (&PromptStr[1], QuestionPromptStr, QuestionPromptStrSize);
    }
    break;

  default:
    PromptStr = AllocateCopyPool (QuestionPromptStrSize, QuestionPromptStr);
  }

  return PromptStr;
}

/**
  Get value string of question

  @param[in] Question           A pointer to question

  @return The pointer of value string or NULL if not found

**/
CHAR16 *
GetQuestionValueStr (
  IN H2O_FORM_BROWSER_Q                       *Question
  )
{
  CHAR16                                      *ValueStr;
  H2O_FORM_BROWSER_O                          *Option;
  UINT32                                      Index;
  CHAR16                                      *TempStr;

  if (Question == NULL) {
    return NULL;
  }

  switch (Question->Operand) {

  case EFI_IFR_SUBTITLE_OP:
  case EFI_IFR_REF_OP:
  case EFI_IFR_RESET_BUTTON_OP:
    return NULL;

  case EFI_IFR_TEXT_OP:
  case EFI_IFR_ACTION_OP:
    return (Question->TextTwo != NULL) ? AllocateCopyPool (StrSize (Question->TextTwo), Question->TextTwo) : NULL;

  case EFI_IFR_ONE_OF_OP:
    for (Index = 0; Index < Question->NumberOfOptions; Index++) {
      Option = &(Question->Options[Index]);
      if ((Option->HiiValue.Value.u64 == Question->HiiValue.Value.u64)) {
        return CatSPrint (NULL, L"%c%s%c", LEFT_ONEOF_DELIMITER, Option->Text, RIGHT_ONEOF_DELIMITER);
      }
    }
    break;

  case EFI_IFR_ORDERED_LIST_OP:
    return NULL;

  case EFI_IFR_NUMERIC_OP:
    TempStr = PrintFormattedNumber (Question);
    if (TempStr == NULL) {
      break;
    }
    ValueStr = CatSPrint (NULL, L"%c%s%c", LEFT_NUMERIC_DELIMITER, TempStr, RIGHT_NUMERIC_DELIMITER);
    FreePool (TempStr);
    return ValueStr;

  case EFI_IFR_CHECKBOX_OP:
    return CatSPrint (NULL, L"%c%c%c", LEFT_CHECKBOX_DELIMITER, (Question->HiiValue.Value.b) ? CHECK_ON : CHECK_OFF, RIGHT_CHECKBOX_DELIMITER);

  case EFI_IFR_TIME_OP:
  case EFI_IFR_DATE_OP:
    return DateTimeOpCodeCreateValueStr (Question);

  case EFI_IFR_STRING_OP:
  case EFI_IFR_PASSWORD_OP:
    return (Question->HiiValue.Buffer != NULL) ? AllocateCopyPool (StrSize ((CHAR16 *) Question->HiiValue.Buffer), Question->HiiValue.Buffer) : NULL;

  default:
    break;
  }

  return NULL;
}

/**
  Update control in content item of seutp page panel

  @param[in]  SetupPagePanel     A pointer to setup page panel
  @param[in]  ContentItem        A pointer to content item
  @param[in]  Statement          A pointer to statement
  @param[out] Control            A pointer to control

  @retval EFI_SUCCESS            Update control successful
  @retval EFI_INVALID_PARAMETER  Input parameter is NULL

**/
EFI_STATUS
UpdateSetupPageControl (
  IN  H2O_LTDE_PANEL                          *SetupPagePanel,
  IN  H2O_LTDE_PANEL_ITEM                     *ContentItem,
  IN  H2O_FORM_BROWSER_S                      *Statement,
  OUT H2O_LTDE_CONTROL                        *Control
  )
{
  UINT32                                      PseudoClass;

  if (SetupPagePanel == NULL || ContentItem == NULL || Statement == NULL || Control == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Statement->GrayedOut || Statement->Locked) {
    PseudoClass = H2O_STYLE_PSEUDO_CLASS_GRAYOUT;
  } else if (Statement->Selectable) {
    PseudoClass = H2O_STYLE_PSEUDO_CLASS_SELECTABLE;
  } else {
    PseudoClass = H2O_STYLE_PSEUDO_CLASS_NORMAL;
  }

  CopyHiiValue (&Control->HiiValue, &Statement->HiiValue);
  Control->Text.String              = GetQuestionPromptStr (Statement);
  Control->ValueStrInfo.String      = GetQuestionValueStr (Statement);
  Control->PageId                   = Statement->PageId;
  Control->StatementId              = Statement->StatementId;
  Control->QuestionId               = Statement->QuestionId;
  Control->Operand                  = Statement->Operand;
  Control->IfrOpCode                = Statement->IfrOpCode;
  Control->Selectable               = Statement->Selectable;
  Control->ControlStyle.StyleType   = GetStyleTypeByOpCode (Statement->Operand);
  Control->ControlStyle.PseudoClass = PseudoClass;

  return EFI_SUCCESS;
}

/**
  Update page up and page down items in seutp page panel

  @param[in] SetupPagePanel      A pointer to setup page panel

  @retval EFI_SUCCESS            Update page up and page down items successful
  @retval EFI_INVALID_PARAMETER  SetupPagePanel is NULL
  @retval EFI_NOT_FOUND          Panel item is not found

**/
STATIC
EFI_STATUS
UpdatePageUpPageDownItems (
  IN H2O_LTDE_PANEL                           *SetupPagePanel
  )
{
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_PANEL_ITEM                         *PageUpItem;
  H2O_LTDE_PANEL_ITEM                         *PageDownItem;
  BOOLEAN                                     ErasePageUp;
  BOOLEAN                                     ErasePageDown;
  EFI_STATUS                                  Status;
  RECT                                        *BorderLineFieldList;
  UINT32                                      BorderLineFieldCount;
  LTDE_PANEL_BORDER_LINE_INFO                 BorderLineInfo;

  if (SetupPagePanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem  = GetPanelItem (SetupPagePanel, LTDE_PANEL_ITEM_ID_CONTENT);
  PageUpItem   = GetPanelItem (SetupPagePanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP);
  PageDownItem = GetPanelItem (SetupPagePanel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN);
  if (ContentItem == NULL || PageUpItem == NULL || PageDownItem == NULL) {
    return EFI_NOT_FOUND;
  }

  ErasePageUp        = FALSE;
  PageUpItem->Hidden = (SetupPagePanel->BorderLineWidth == 0 || ContentItem->CurrentPos == 0) ? TRUE : FALSE;
  if (PageUpItem->Hidden) {
    ErasePageUp = TRUE;
  } else {
    DEConOutSetAttribute ((SetupPagePanel->ColorAttribute & 0xF0) | ARROW_TEXT);
    DisplayString (
      PageUpItem->ItemField.left,
      PageUpItem->ItemField.top,
      PageUpItem->ControlList->Text.String
      );
  }

  ErasePageDown = FALSE;
  if (SetupPagePanel->BorderLineWidth == 0 ||
      (ContentItem->CurrentPos + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - 1) >= ContentItem->MaxPos) {
    PageDownItem->Hidden = TRUE;
  } else {
    PageDownItem->Hidden = FALSE;
  }
  if (PageDownItem->Hidden) {
    ErasePageDown = TRUE;
  } else {
    DEConOutSetAttribute ((SetupPagePanel->ColorAttribute & 0xF0) | ARROW_TEXT);
    DisplayString (
      PageDownItem->ItemField.left,
      PageDownItem->ItemField.top,
      PageDownItem->ControlList->Text.String
      );;
  }

  if ((ErasePageUp || ErasePageDown) && SetupPagePanel->BorderLineWidth != 0) {
    DEConOutSetAttribute (SetupPagePanel->ColorAttribute);
    Status = GetAllBorderLineField (&BorderLineFieldList, &BorderLineFieldCount);
    if (!EFI_ERROR (Status)) {
      Status = GetBorderLineInfo (&SetupPagePanel->PanelField, BorderLineFieldList, BorderLineFieldCount, &BorderLineInfo);
      if (!EFI_ERROR (Status)) {
        if (ErasePageUp) {
          DisplayString (SetupPagePanel->PanelField.left, SetupPagePanel->PanelField.top, BorderLineInfo.TopHorizonalLine);
        }
        if (ErasePageDown) {
          DisplayString (SetupPagePanel->PanelField.left, SetupPagePanel->PanelField.bottom, BorderLineInfo.BottomHorizonalLine);
        }
        SafeFreePool ((VOID **) &BorderLineInfo.TopHorizonalLine);
        SafeFreePool ((VOID **) &BorderLineInfo.BottomHorizonalLine);
        SafeFreePool ((VOID **) &BorderLineInfo.LeftVerticalLine);
        SafeFreePool ((VOID **) &BorderLineInfo.RightVerticalLine);
      }
      SafeFreePool ((VOID **) &BorderLineFieldList);
    }
  }

  return EFI_SUCCESS;
}

/**
  Set field of all controls in content item of seutp page panel

  @param[in] SetupPagePanel      A pointer to setup page panel

  @retval EFI_SUCCESS            Set field successful
  @retval EFI_INVALID_PARAMETER  ContentItem is NULL

**/
STATIC
EFI_STATUS
SetContentItemControlField (
  IN H2O_LTDE_PANEL_ITEM                      *ContentItem
  )
{
  UINT32                                      Index;
  INT32                                       ClientX;
  INT32                                       ClientY;
  UINT32                                      ContentItemWidth;
  UINT32                                      PromptWidth;
  UINT32                                      ValueWidth;
  H2O_LTDE_CONTROL                            *Control;
  INT32                                       ControlPromptWidth;
  INT32                                       ControlPromptHeight;
  INT32                                       ControlValueWidth;
  INT32                                       ControlValueHeight;

  if (ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItemWidth = (UINT32) H2O_LTDE_FIELD_WIDTH (&ContentItem->ItemField);
  PromptWidth      = (ContentItemWidth - PROMPT_VALUE_SEPARATOR_WIDTH) / 2;
  ValueWidth       = ContentItemWidth - PromptWidth - PROMPT_VALUE_SEPARATOR_WIDTH;

  ClientX = 0;
  ClientY = 0;
  for (Index = 0; Index < ContentItem->ControlCount; Index++) {
    Control = &ContentItem->ControlList[Index];

    if (Control->ValueStrInfo.String == NULL) {
      ControlPromptWidth = ContentItemWidth;
      ControlValueWidth  = 0;
    } else {
      ControlPromptWidth = PromptWidth;
      ControlValueWidth  = ValueWidth;
    }

    ControlPromptHeight = GetStringHeight (Control->Text.String, ControlPromptWidth);
    SetRect (
      &Control->Text.StringField,
      ClientX,
      ClientY,
      ClientX + ControlPromptWidth - 1,
      ClientY + ControlPromptHeight - 1
      );

    if (Control->ValueStrInfo.String != NULL) {
      ControlValueHeight = GetStringHeight (Control->ValueStrInfo.String, ControlValueWidth);
      SetRect (
        &Control->ValueStrInfo.StringField,
        ClientX + PromptWidth + PROMPT_VALUE_SEPARATOR_WIDTH,
        ClientY,
        ClientX + ContentItemWidth - 1,
        ClientY + ControlValueHeight - 1
        );
    } else {
      ControlValueHeight = 0;
    }

    SetRect (
      &Control->ControlField,
      ClientX,
      ClientY,
      ClientX + ContentItemWidth - 1,
      ClientY + MAX (ControlPromptHeight, ControlValueHeight) - 1
      );

    ClientY += MAX (ControlPromptHeight, ControlValueHeight);
  }

  ContentItem->MaxPos = (ClientY > 0) ? (ClientY - 1) : 0;

  return EFI_SUCCESS;
}

/**
  Initialize all controls in content item of seutp page panel

  @param[in]      SetupPagePanel   A pointer to setup page panel
  @param[in, out] ContentItem      A pointer to content item

  @retval EFI_SUCCESS            Initialize all controls successful
  @retval EFI_INVALID_PARAMETER  SetupPagePanel or ContentItem is NULL
  @retval EFI_OUT_OF_RESOURCES   Allocate pool fail

**/
STATIC
EFI_STATUS
InitSetupPagePanelContentItemControlList (
  IN     H2O_LTDE_PANEL                       *SetupPagePanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  )
{
  EFI_STATUS                                  Status;
  UINT32                                      Index;
  UINT32                                      Count;
  UINT32                                      ControlCount;
  H2O_LTDE_CONTROL                            *ControlList;
  H2O_FORM_BROWSER_S                          *Statement;

  if (SetupPagePanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ControlCount = mDEPrivate->FBProtocol->CurrentP->NumberOfStatementIds;
  ControlList  = AllocateZeroPool (sizeof (H2O_LTDE_CONTROL) * ControlCount);
  if (ControlList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Count = 0;
  for (Index = 0; Index < ControlCount; Index++) {
    Status = mDEPrivate->FBProtocol->GetSInfo (
                                       mDEPrivate->FBProtocol,
                                       mDEPrivate->FBProtocol->CurrentP->PageId,
                                       mDEPrivate->FBProtocol->CurrentP->StatementIds[Index],
                                       &Statement
                                       );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = UpdateSetupPageControl (SetupPagePanel, ContentItem, Statement, &ControlList[Count]);
    SafeFreePool ((VOID **) &Statement);
    if (EFI_ERROR (Status)) {
      continue;
    }

    Count++;
  }

  ContentItem->Vertical     = SetupPagePanel->Vertical;
  ContentItem->ControlCount = Count;
  ContentItem->ControlList  = ControlList;

  SetContentItemControlField (ContentItem);

  return EFI_SUCCESS;
}

/**
  Initialize content item of seutp page panel

  @param[in]      SetupPagePanel   A pointer to setup page panel
  @param[in, out] ContentItem      A pointer to content item

  @retval EFI_SUCCESS            Initialize content item successful
  @retval EFI_INVALID_PARAMETER  SetupPagePanel or ContentItem is NULL

**/
STATIC
EFI_STATUS
InitSetupPagePanelContentItem (
  IN     H2O_LTDE_PANEL                       *SetupPagePanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  )
{
  H2O_LTDE_PANEL_ITEM                         *OrgContentItem;
  H2O_LTDE_CONTROL                            *Control;
  H2O_FORM_BROWSER_Q                          *CurrentQ;
  INT32                                       StartY;
  INT32                                       EndY;

  if (SetupPagePanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem->ItemId      = LTDE_PANEL_ITEM_ID_CONTENT;
  ContentItem->Hidden      = FALSE;

  CopyRect (&ContentItem->ItemField, &SetupPagePanel->PanelField);
  InflateRect (&ContentItem->ItemField, -SetupPagePanel->BorderLineWidth, -SetupPagePanel->BorderLineWidth);

  InitSetupPagePanelContentItemControlList (SetupPagePanel, ContentItem);

  //
  // Update CurrentPos and SelectedControl
  //
  OrgContentItem = GetPanelItem (SetupPagePanel, LTDE_PANEL_ITEM_ID_CONTENT);
  CurrentQ       = mDEPrivate->FBProtocol->CurrentQ;
  if (CurrentQ != NULL &&
      IsCurrentHighlight (CurrentQ->PageId, CurrentQ->QuestionId, CurrentQ->IfrOpCode)) {
    Control = GetControlByQuestionId (
                ContentItem->ControlList,
                ContentItem->ControlCount,
                CurrentQ->QuestionId,
                CurrentQ->IfrOpCode
                );
    if (Control != NULL && IsValidHighlightControl (Control)) {
      Control->Sequence = SetupPagePanel->SelectedControl->Sequence;
      SetupPagePanel->SelectedControl = Control;
      if (OrgContentItem != NULL) {
        ContentItem->CurrentPos = OrgContentItem->CurrentPos;
      }
      return EFI_SUCCESS;
    }
  }

  SetupPagePanel->SelectedControl = NULL;
  Control = NULL;
  if (CurrentQ != NULL) {
    Control = GetControlByQuestionId (
                ContentItem->ControlList,
                ContentItem->ControlCount,
                CurrentQ->QuestionId,
                CurrentQ->IfrOpCode
                );
  }
  if (Control == NULL || !IsValidHighlightControl (Control)) {
    Control = GetValidHighlightControl (ContentItem);;
  }
  if (Control != NULL) {
    SetupPagePanel->SelectedControl = Control;
    StartY = ContentItem->CurrentPos;
    EndY   = StartY + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - 1;
    if (!IN_RANGE (Control->ControlField.top, StartY, EndY) ||
        !IN_RANGE (Control->ControlField.bottom, StartY, EndY)) {
      if (Control->ControlField.top <= ContentItem->CurrentPos) {
        ContentItem->CurrentPos = Control->ControlField.top;
      } else {
        ContentItem->CurrentPos = Control->ControlField.top - H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) + 1;
      }
    }

    SendSelectQNotify (Control->PageId, Control->QuestionId, Control->IfrOpCode);
  }

  return EFI_SUCCESS;
}

/**
  Initialize control list in page up or page down item of seutp page panel

  @param[in]      SetupPagePanel   A pointer to setup page panel
  @param[in, out] PageUpDownItem   A pointer to page up or page down item

  @retval EFI_SUCCESS            Initialize control list successful
  @retval EFI_INVALID_PARAMETER  SetupPagePanel or PageUpDownItem is NULL
  @retval EFI_NOT_FOUND          Hot key info is not found
  @retval EFI_OUT_OF_RESOURCES   Allocate pool fail

**/
STATIC
EFI_STATUS
InitSetupPagePanelContentPageUpDownItemControlList (
  IN     H2O_LTDE_PANEL                       *SetupPagePanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *PageUpDownItem
  )
{
  H2O_LTDE_CONTROL                            *Control;
  UINT32                                      HotKeyAction;
  LTDE_SETUP_PAGE_PANEL_HOT_KEY_INFO          *HotKey;

  if (SetupPagePanel == NULL || PageUpDownItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HotKeyAction = (PageUpDownItem->ItemId == LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP) ? LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_PAGE_UP :
                                                                                  LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_PAGE_DOWN;
  HotKey = GetSetupPageHotKeyInfo (HotKeyAction);
  if (HotKey == NULL) {
    return EFI_NOT_FOUND;
  }

  Control = AllocateZeroPool (sizeof (H2O_LTDE_CONTROL));
  if (Control == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Control->Selectable  = TRUE;
  Control->Text.String = AllocateCopyPool (StrSize(HotKey->String), HotKey->String);

  PageUpDownItem->ControlCount = 1;
  PageUpDownItem->ControlList  = Control;

  return EFI_SUCCESS;
}

/**
  Initialize page up item of seutp page panel

  @param[in]      SetupPagePanel   A pointer to setup page panel
  @param[in, out] PageUpItem       A pointer to page up item

  @retval EFI_SUCCESS            Initialize page up item successful
  @retval EFI_INVALID_PARAMETER  SetupPagePanel or PageUpItem is NULL

**/
STATIC
EFI_STATUS
InitSetupPagePanelContentPageUpItem (
  IN     H2O_LTDE_PANEL                       *SetupPagePanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *PageUpItem
  )
{
  if (SetupPagePanel == NULL || PageUpItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PageUpItem->ItemId = LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP;
  PageUpItem->Hidden = TRUE;

  CopyRect (&PageUpItem->ItemField, &SetupPagePanel->PanelField);
  PageUpItem->ItemField.left   = PageUpItem->ItemField.right;
  PageUpItem->ItemField.bottom = PageUpItem->ItemField.top;

  InitSetupPagePanelContentPageUpDownItemControlList (SetupPagePanel, PageUpItem);

  return EFI_SUCCESS;
}

/**
  Initialize page down item of seutp page panel

  @param[in]      SetupPagePanel   A pointer to setup page panel
  @param[in, out] PageUpItem       A pointer to page down item

  @retval EFI_SUCCESS            Initialize page down item successful
  @retval EFI_INVALID_PARAMETER  SetupPagePanel or PageDownItem is NULL

**/
STATIC
EFI_STATUS
InitSetupPagePanelContentPageDownItem (
  IN     H2O_LTDE_PANEL                       *SetupPagePanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *PageDownItem
  )
{
  if (SetupPagePanel == NULL || PageDownItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PageDownItem->ItemId      = LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN;
  PageDownItem->Hidden      = TRUE;

  CopyRect (&PageDownItem->ItemField, &SetupPagePanel->PanelField);
  PageDownItem->ItemField.left = PageDownItem->ItemField.right;
  PageDownItem->ItemField.top  = PageDownItem->ItemField.bottom;

  InitSetupPagePanelContentPageUpDownItemControlList (SetupPagePanel, PageDownItem);

  return EFI_SUCCESS;
}

/**
  Initialize seutp page panel

  @retval EFI_SUCCESS            Initialize page down item successful
  @retval EFI_NOT_FOUND          Setup page panel is not found
  @retval EFI_OUT_OF_RESOURCES   Allocate pool fail

**/
EFI_STATUS
InitSetupPagePanel (
  VOID
  )
{
  H2O_LTDE_PANEL                              *SetupPagePanel;
  UINT32                                      ItemCount;
  H2O_LTDE_PANEL_ITEM                         *ItemList;

  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel == NULL) {
    return EFI_NOT_FOUND;
  }

  ItemCount = 3;
  ItemList  = AllocateZeroPool (ItemCount * sizeof (H2O_LTDE_PANEL_ITEM));
  if (ItemList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  InitSetupPagePanelContentItem (SetupPagePanel, &ItemList[0]);
  InitSetupPagePanelContentPageUpItem (SetupPagePanel, &ItemList[1]);
  InitSetupPagePanelContentPageDownItem (SetupPagePanel, &ItemList[2]);

  FreePanelItemList (SetupPagePanel->ItemList, SetupPagePanel->ItemCount);
  SetupPagePanel->ItemCount = ItemCount;
  SetupPagePanel->ItemList  = ItemList;

  return EFI_SUCCESS;
}

/**
  Display control in seutp page panel

  @param[in] SetupPagePanel      A pointer to setup page panel
  @param[in] ContentItem         A pointer to content item
  @param[in] Control             A pointer to control

  @retval EFI_SUCCESS            Display control successful
  @retval EFI_INVALID_PARAMETER  Input parameter is NULL

**/
EFI_STATUS
DisplaySetupPagePanelContentItemControl (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_PANEL_ITEM                      *ContentItem,
  IN H2O_LTDE_CONTROL                         *Control
  )
{
  EFI_STATUS                                  Status;
  UINT32                                      Attribute;
  UINT32                                      HighlightAttribute;
  UINT32                                      Index;
  INT32                                       StartY;
  INT32                                       EndY;
  BOOLEAN                                     IsHighlight;
  UINT32                                      ControlHeight;
  UINT32                                      PromptWidth;
  UINT32                                      PrmoptStrNum;
  CHAR16                                      **PrmoptStrArray;
  CHAR16                                      *PrmoptStr;
  UINT32                                      ValueWidth;
  UINT32                                      ValueStrNum;
  CHAR16                                      **ValueStrArray;
  CHAR16                                      *ValueStr;
  CHAR16                                      *SeparationStr;
  CHAR16                                      *DisplayStr;

  if (SetupPagePanel == NULL || ContentItem == NULL || Control == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  StartY = ContentItem->CurrentPos;
  EndY   = StartY + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - 1;
  if (!IS_OVERLAP (Control->ControlField.top, Control->ControlField.bottom, StartY, EndY)) {
    return EFI_SUCCESS;
  }

  IsHighlight = IsCurrentHighlight (Control->PageId, Control->QuestionId, Control->IfrOpCode);
  GetPanelColorAttribute (SetupPagePanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, Control->ControlStyle.PseudoClass, &Attribute);
  GetPanelColorAttribute (SetupPagePanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_HIGHLIGHT , &HighlightAttribute);

  switch (Control->Operand) {

  case EFI_IFR_TIME_OP:
  case EFI_IFR_DATE_OP:
    return DateTimeOpCodeDisplayControl (SetupPagePanel, Control);

  case EFI_IFR_ORDERED_LIST_OP:
    return OrderListOpCodeDisplayValueStr (SetupPagePanel, ContentItem, Control);

  default:
    break;
  }

  PromptWidth = H2O_LTDE_FIELD_WIDTH (&Control->Text.StringField);
  Status = GetStringArrayByWidth (Control->Text.String, PromptWidth, &PrmoptStrNum, &PrmoptStrArray);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Control->ValueStrInfo.String != NULL) {
    ValueWidth = H2O_LTDE_FIELD_WIDTH (&Control->ValueStrInfo.StringField);
    Status = GetStringArrayByWidth (Control->ValueStrInfo.String, ValueWidth, &ValueStrNum, &ValueStrArray);
    if (EFI_ERROR (Status)) {
      ValueStrNum   = 0;
      ValueStrArray = NULL;
    }
  } else {
    ValueWidth    = 0;
    ValueStrNum   = 0;
    ValueStrArray = NULL;
  }

  SeparationStr = CreateString (PROMPT_VALUE_SEPARATOR_WIDTH, ' ');
  ControlHeight = H2O_LTDE_FIELD_HEIGHT (&Control->ControlField);

  DEConOutSetAttribute (IsHighlight ? HighlightAttribute : Attribute);
  for (Index = 0; Index < ControlHeight; Index++) {
    if (!IN_RANGE ((INT32) (Control->ControlField.top + Index), StartY, EndY)) {
      continue;
    }

    if (Index < PrmoptStrNum) {
      PrmoptStr = GetAlignmentString (PrmoptStrArray[Index], PromptWidth, LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT);
    } else {
      PrmoptStr = CreateString (PromptWidth, ' ');
    }

    if (Control->ValueStrInfo.String != NULL) {
      if (Index < ValueStrNum) {
        ValueStr = GetAlignmentString (ValueStrArray[Index], ValueWidth, LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT);
      } else {
        ValueStr = CreateString (ValueWidth, ' ');
      }

      DisplayStr = CatSPrint (NULL, L"%s%s%s", PrmoptStr, SeparationStr, ValueStr);
      FreePool (PrmoptStr);
      FreePool (ValueStr);
    } else {
      DisplayStr = PrmoptStr;
    }

    DisplayString (
      ContentItem->ItemField.left,
      ContentItem->ItemField.top + (Control->ControlField.top - ContentItem->CurrentPos) + Index,
      DisplayStr
      );

    FreePool (DisplayStr);
  }

  if (PrmoptStrArray != NULL) {
    for (Index = 0; Index < PrmoptStrNum; Index++) {
      FreePool (PrmoptStrArray[Index]);
    }
    FreePool (PrmoptStrArray);
  }
  if (ValueStrArray != NULL) {
    for (Index = 0; Index < ValueStrNum; Index++) {
      FreePool (ValueStrArray[Index]);
    }
    FreePool (ValueStrArray);
  }
  SafeFreePool ((VOID **) &SeparationStr);

  return EFI_SUCCESS;
}

/**
  Display content item in seutp page panel

  @param[in] SetupPagePanel      A pointer to setup page panel
  @param[in] ContentItem         A pointer to content item

  @retval EFI_SUCCESS            Display content item successful
  @retval EFI_INVALID_PARAMETER  Input parameter is NULL

**/
EFI_STATUS
DisplaySetupPagePanelContentItem (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_PANEL_ITEM                      *ContentItem
  )
{
  H2O_LTDE_CONTROL                            *Control;
  UINT32                                      Index;
  UINT32                                      EmptyHeight;
  RECT                                        Field;

  if (SetupPagePanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ContentItem->ControlCount == 0) {
    ClearField (SetupPagePanel->ColorAttribute, &ContentItem->ItemField);
    return EFI_SUCCESS;
  }

  for (Index = 0; Index < ContentItem->ControlCount; Index++) {
    Control = &ContentItem->ControlList[Index];
    DisplaySetupPagePanelContentItemControl (SetupPagePanel, ContentItem, Control);
  }

  if (ContentItem->CurrentPos + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - 1 > ContentItem->MaxPos) {
    EmptyHeight = H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - (ContentItem->MaxPos - ContentItem->CurrentPos + 1);
    CopyRect (&Field, &ContentItem->ItemField);
    Field.top = Field.bottom - EmptyHeight + 1;
    ClearField (SetupPagePanel->ColorAttribute, &Field);
  }

  return EFI_SUCCESS;
}

/**
  Display seutp page panel

  @retval EFI_SUCCESS            Display seutp page panel successful

**/
EFI_STATUS
DisplaySetupPagePanel (
  VOID
  )
{
  H2O_LTDE_PANEL                              *SetupPagePanel;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;

  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel == NULL || !SetupPagePanel->Visible) {
    return EFI_SUCCESS;
  }

  ContentItem = GetPanelItem (SetupPagePanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem != NULL) {
    DisplaySetupPagePanelContentItem (SetupPagePanel, ContentItem);
  }

  UpdatePageUpPageDownItems (SetupPagePanel);

  return EFI_SUCCESS;
}

/**
  Process select question event in setup page panel.

  @param[in] SetupPagePanel      Pointer to setup page panel
  @param[in] SelectQ             Pointer to select question event

  @retval EFI_SUCCESS            Process select question event successful.
  @retval EFI_INVALID_PARAMETER  SetupPagePanel or SelectQ is NULL.
  @retval EFI_NOT_FOUND          Content item or control is not found.

**/
EFI_STATUS
SetupPagePanelProcessSelectQEvt (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_DISPLAY_ENGINE_EVT_SELECT_Q          *SelectQ
  )
{
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_CONTROL                            *OrgControl;
  H2O_LTDE_CONTROL                            *SelectQControl;
  INT32                                       StartY;
  INT32                                       EndY;

  if (SetupPagePanel == NULL || SelectQ == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem = GetPanelItem (SetupPagePanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem == NULL) {
    return EFI_NOT_FOUND;
  }

  SelectQControl = GetControlByQuestionId (
                     ContentItem->ControlList,
                     ContentItem->ControlCount,
                     SelectQ->QuestionId,
                     SelectQ->IfrOpCode
                     );
  if (SelectQControl == NULL) {
    return EFI_NOT_FOUND;
  }
  if (SelectQControl == SetupPagePanel->SelectedControl) {
    return EFI_SUCCESS;
  }

  OrgControl = SetupPagePanel->SelectedControl;
  if (OrgControl != NULL) {
    OrgControl->Sequence = 0;
  }
  SetupPagePanel->SelectedControl = SelectQControl;

  //
  // Update current position of ContentItem and refresh setup page if need.
  //
  StartY = ContentItem->CurrentPos;
  EndY   = StartY + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - 1;
  if (IN_RANGE (SelectQControl->ControlField.top, StartY, EndY) &&
      IN_RANGE (SelectQControl->ControlField.bottom, StartY, EndY)) {
    DisplaySetupPagePanelContentItemControl (SetupPagePanel, ContentItem, OrgControl);
    DisplaySetupPagePanelContentItemControl (SetupPagePanel, ContentItem, SelectQControl);
    return EFI_SUCCESS;
  }

  if (SelectQControl->ControlField.top <= ContentItem->CurrentPos) {
    ContentItem->CurrentPos = SelectQControl->ControlField.top;
  } else {
    ContentItem->CurrentPos = SelectQControl->ControlField.bottom - H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) + 1;
  }

  DisplaySetupPagePanel ();

  return EFI_SUCCESS;
}

/**
  Process select question hot key in setup page panel.

  @param[in] SetupPagePanel      Pointer to setup page panel
  @param[in] GoNext              TRUE if go to next control

  @retval EFI_SUCCESS            Process select question hot key successful.
  @retval EFI_INVALID_PARAMETER  SetupPagePanel is NULL.
  @retval EFI_NOT_FOUND          Content item is not found.

**/
EFI_STATUS
SetupPagePanelProcessSelectQHotKey (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN BOOLEAN                                  GoNext
  )
{
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_CONTROL                            *Control;
  H2O_LTDE_CONTROL                            *NextControl;
  EFI_STATUS                                  Status;
  INT32                                       StartY;
  INT32                                       EndY;
  UINT32                                      Index;
  BOOLEAN                                     Loop;

  if (SetupPagePanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem = GetPanelItem (SetupPagePanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem == NULL) {
    return EFI_NOT_FOUND;
  }

  StartY = ContentItem->CurrentPos;
  EndY   = StartY + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - 1;

  //
  // If it is ordered list, jump to next otion value.
  //
  if (SetupPagePanel->SelectedControl != NULL && SetupPagePanel->SelectedControl->Operand == EFI_IFR_ORDERED_LIST_OP) {
    Status = OrderListOpCodeProcessSelectQHotKey (SetupPagePanel, ContentItem, GoNext);
    if (!EFI_ERROR(Status)) {
      return Status;
    }
  }

  //
  // If there is a next selectable control, jump to it.
  //
  Loop = FALSE;
  if (SetupPagePanel->SelectedControl == NULL) {
    NextControl = GoNext ? GetNextSelectableControlByCurrentPos (ContentItem, ContentItem->CurrentPos, Loop) :
                           GetPreviousSelectableControlByCurrentPos (ContentItem, ContentItem->CurrentPos, Loop);

  } else {
    NextControl = GoNext ? GetNextSelectableControl (ContentItem, SetupPagePanel->SelectedControl, Loop) :
                           GetPreviousSelectableControl (ContentItem, SetupPagePanel->SelectedControl, Loop);
  }
  if (NextControl != NULL &&
      ((IS_OVERLAP (NextControl->ControlField.top, NextControl->ControlField.bottom, StartY, EndY)) ||
       (GoNext && NextControl->ControlField.top == EndY + 1) ||
       ((!GoNext) && (NextControl->ControlField.bottom == (StartY - 1))))) {
    SendSelectQNotify (NextControl->PageId, NextControl->QuestionId, NextControl->IfrOpCode);
    return EFI_SUCCESS;
  }

  if (GoNext && EndY >= ContentItem->MaxPos ||
      !GoNext && StartY == 0) {
    return EFI_SUCCESS;
  }

  //
  // Set Current position and refresh setup page.
  //
  NextControl = NULL;
  for (Index = 0; Index < ContentItem->ControlCount; Index++) {
    Control = &ContentItem->ControlList[Index];
    if (GoNext && IN_RANGE(EndY, Control->ControlField.top, Control->ControlField.bottom)) {
      NextControl = (Index + 1 < ContentItem->ControlCount) ? &ContentItem->ControlList[Index + 1] : Control;
      break;
    }

    if (!GoNext && IN_RANGE(StartY, Control->ControlField.top, Control->ControlField.bottom)) {
      NextControl = Index > 0 ? &ContentItem->ControlList[Index - 1] : Control;
      break;
    }
  }
  if (NextControl == NULL) {
    return EFI_NOT_FOUND;
  }

  if (GoNext) {
    StartY = NextControl->ControlField.bottom - H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) + 1;
  } else {
    StartY = NextControl->ControlField.top;
  }
  ContentItem->CurrentPos = StartY;
  EndY = StartY + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - 1;

  if (SetupPagePanel->SelectedControl != NULL &&
      !IS_OVERLAP (SetupPagePanel->SelectedControl->ControlField.top, SetupPagePanel->SelectedControl->ControlField.bottom, StartY, EndY)) {
    SetupPagePanel->SelectedControl = NULL;
  }

  DisplaySetupPagePanel ();

  return EFI_SUCCESS;
}

/**
  Process open question hot key in setup page panel.

  @param[in] SetupPagePanel      Pointer to setup page panel
  @param[in] SelectedControl     Pointer to selected control

  @retval EFI_SUCCESS            Process open question hot key successful.
  @retval EFI_INVALID_PARAMETER  SetupPagePanel is NULL.
  @retval EFI_NOT_FOUND          Selected control is not found.

**/
EFI_STATUS
SetupPagePanelProcessOpenQHotKey (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_CONTROL                         *SelectedControl
  )
{
  EFI_HII_VALUE                               HiiValue;

  if (SetupPagePanel == NULL || SelectedControl == NULL || !IsValidHighlightControl (SelectedControl)) {
    return EFI_INVALID_PARAMETER;
  }

  switch (SelectedControl->Operand) {

  case EFI_IFR_ORDERED_LIST_OP:
    //
    // In original text UI design, don't pop dialog for these opcodes.
    //
    break;

  case EFI_IFR_DATE_OP:
  case EFI_IFR_TIME_OP:
    SelectedControl->Sequence = (SelectedControl->Sequence == 2) ? 0 : SelectedControl->Sequence + 1;
    DateTimeOpCodeDisplayControl (
      SetupPagePanel,
      SelectedControl
      );
    break;

  case EFI_IFR_CHECKBOX_OP:
    ZeroMem (&HiiValue, sizeof (EFI_HII_VALUE));
    HiiValue.Type    = EFI_IFR_TYPE_BOOLEAN;
    HiiValue.Value.b = (BOOLEAN) (SelectedControl->HiiValue.Value.b ? FALSE : TRUE);
    return SendChangeQNotify (SelectedControl->PageId, SelectedControl->QuestionId, &HiiValue);

  default:
    return SendOpenQNotify (SelectedControl->PageId, SelectedControl->QuestionId, SelectedControl->IfrOpCode);
  }

  return EFI_SUCCESS;
}

/**
  Process change question hot key in setup page panel.

  @param[in] SetupPagePanel      Pointer to setup page panel
  @param[in] Increase            TRUE if increase question value

  @retval EFI_SUCCESS            Process change question hot key successful.
  @retval EFI_INVALID_PARAMETER  SetupPagePanel is NULL.
  @retval EFI_NOT_FOUND          Selected control is not found.

**/
EFI_STATUS
SetupPagePanelProcessChangeQHotKey (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN BOOLEAN                                  Increase
  )
{
  EFI_STATUS                                  Status;
  H2O_LTDE_CONTROL                            *SelectedControl;
  EFI_TIME                                    EfiTime;
  EFI_HII_VALUE                               HiiValue;

  if (SetupPagePanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (SetupPagePanel->SelectedControl == NULL) {
    return EFI_NOT_FOUND;
  }

  SelectedControl = SetupPagePanel->SelectedControl;

  switch (SelectedControl->Operand) {

  case EFI_IFR_DATE_OP:
  case EFI_IFR_TIME_OP:
    TransferHiiValueToEfiTime (&SelectedControl->HiiValue, &EfiTime);
    Status = GetNextDateTimeValue (
               DateTimeOpCodeGetItemValue (SelectedControl),
               Increase,
               &EfiTime
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    TransferEfiTimeToHiiValue (SelectedControl->Operand == EFI_IFR_DATE_OP, &EfiTime, &HiiValue);
    SendChangeQNotify (SelectedControl->PageId, SelectedControl->QuestionId, &HiiValue);
    break;

  case EFI_IFR_ORDERED_LIST_OP:
    return OrderListOpCodeShiftOrder (SelectedControl, !Increase);

  default:
    Status = GetNextQuestionValue (mDEPrivate->FBProtocol->CurrentQ, Increase, &HiiValue);
    if (EFI_ERROR (Status)) {
      break;
    }

    return SendChangeQNotify (SelectedControl->PageId, SelectedControl->QuestionId, &HiiValue);
  }

  return Status;
}

/**
  Process hot key in setup page panel.

  @param[in] UserInputKey        Pointer to user input data

  @retval EFI_SUCCESS            Process hot key successful.
  @retval EFI_INVALID_PARAMETER  UserInputKey is NULL.
  @retval EFI_NOT_FOUND          setup page panel is not found or user input key is not hot key of setup page panel.

**/
EFI_STATUS
SetupPagePanelProcessHotKey (
  IN EFI_INPUT_KEY                            *UserInputKey
  )
{
  H2O_LTDE_PANEL                              *SetupPagePanel;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  EFI_INPUT_KEY                               InputKey;
  UINT32                                      Index;
  UINT32                                      Count;
  UINT32                                      HotKeyAction;
  INT32                                       ContentItemHeight;
  BOOLEAN                                     Refresh;
  INT32                                       StartY;
  INT32                                       EndY;
  H2O_LTDE_CONTROL                            *Control;

  if (UserInputKey == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel == NULL || !SetupPagePanel->Visible) {
    return EFI_NOT_FOUND;
  }

  ContentItem = GetPanelItem (SetupPagePanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem == NULL) {
    return EFI_NOT_FOUND;
  }

  CopyMem (&InputKey, UserInputKey, sizeof (EFI_INPUT_KEY));
  InputKey.UnicodeChar = TO_LOWER_UNICODE_CHAR (InputKey.UnicodeChar);

  HotKeyAction = LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_MAX;
  Count        = sizeof (mSetupPagePanelHotKeyInfoList) / sizeof (LTDE_SETUP_PAGE_PANEL_HOT_KEY_INFO);
  for (Index = 0; Index < Count; Index++) {
    if (mSetupPagePanelHotKeyInfoList[Index].InputKey.ScanCode    == InputKey.ScanCode &&
        mSetupPagePanelHotKeyInfoList[Index].InputKey.UnicodeChar == InputKey.UnicodeChar) {
      HotKeyAction = mSetupPagePanelHotKeyInfoList[Index].Action;
      break;
    }
  }
  if (Index == Count) {
    return EFI_NOT_FOUND;
  }

  Refresh           = FALSE;
  ContentItemHeight = H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField);
  switch (HotKeyAction) {

  case LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_PAGE_UP:
    if (ContentItem->CurrentPos == 0) {
      break;
    }

    Refresh = TRUE;
    if (ContentItem->CurrentPos > ContentItemHeight) {
      ContentItem->CurrentPos -= ContentItemHeight;
    } else {
      ContentItem->CurrentPos = 0;
    }
    break;

  case LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_PAGE_DOWN:
    if (ContentItem->MaxPos <= ContentItem->CurrentPos + ContentItemHeight - 1) {
      break;
    }

    Refresh = TRUE;
    ContentItem->CurrentPos += ContentItemHeight;
    break;

  case LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_HOME:
    if (ContentItem->CurrentPos == 0) {
      break;
    }
    Refresh = TRUE;
    ContentItem->CurrentPos = 0;
    break;

  case LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_END:
    if (IN_RANGE (ContentItem->MaxPos, ContentItem->CurrentPos, (ContentItem->CurrentPos + H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) - 1))) {
      break;
    }
    Refresh = TRUE;
    ContentItem->CurrentPos = ContentItem->MaxPos - H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField) + 1;
    break;
  }

  if (Refresh) {
    Control = NULL;
    StartY  = ContentItem->CurrentPos;
    EndY    = StartY + ContentItemHeight - 1;
    for (Index = 0; Index < ContentItem->ControlCount; Index++) {
      Control = &ContentItem->ControlList[Index];
      if (IS_OVERLAP (Control->ControlField.top, Control->ControlField.bottom, StartY, EndY) &&
          IsValidHighlightControl (Control)) {
        break;
      }
    }
    if (Index == ContentItem->ControlCount) {
      SetupPagePanel->SelectedControl = NULL;
    } else {
      SetupPagePanel->SelectedControl = Control;
      SendSelectQNotify (Control->PageId, Control->QuestionId, Control->IfrOpCode);
    }

    DisplaySetupPagePanel ();
  }

  return EFI_SUCCESS;
}

/**
  Process mouse click in setup page panel.

  @param[in] SetupPagePanel      Pointer to setup page panel
  @param[in] SelectedPanelItem   Pointer to selected pane litem
  @param[in] SelectedControl     Pointer to selected control
  @param[in] UserInputData       Pointer to user input data

  @retval EFI_SUCCESS            Process hot key successful.
  @retval EFI_INVALID_PARAMETER  Input parameter is NULL.
  @retval EFI_UNSUPPORTED        Selected control is not selectable.

**/
EFI_STATUS
SetupPagePanelProcessMouseClick (
  IN H2O_LTDE_PANEL                           *SetupPagePanel,
  IN H2O_LTDE_PANEL_ITEM                      *SelectedPanelItem,
  IN H2O_LTDE_CONTROL                         *SelectedControl,
  IN H2O_DISPLAY_ENGINE_USER_INPUT_DATA       *UserInputData
  )
{
  UINT32                                      Index;
  UINT32                                      Action;
  HOT_KEY_INFO                                HotKey;

  if (SetupPagePanel == NULL || SelectedPanelItem == NULL || SelectedControl == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!SelectedControl->Selectable) {
    return EFI_UNSUPPORTED;
  }

  switch (SelectedPanelItem->ItemId) {

  case LTDE_PANEL_ITEM_ID_CONTENT:
    if (SelectedControl->Operand == EFI_IFR_ORDERED_LIST_OP) {
      OrderListOpCodeProcessMouseClick (SetupPagePanel, SelectedPanelItem, SelectedControl, UserInputData);
    }

    if (SelectedControl->PageId     == SetupPagePanel->SelectedControl->PageId &&
        SelectedControl->QuestionId == SetupPagePanel->SelectedControl->QuestionId &&
        SelectedControl->IfrOpCode  == SetupPagePanel->SelectedControl->IfrOpCode) {
      ZeroMem (&HotKey, sizeof (HOT_KEY_INFO));
      HotKey.HotKeyAction = HotKeyEnter;
      return SendEvtByHotKey (&HotKey);
    } else {
      return SendSelectQNotify (SelectedControl->PageId, SelectedControl->QuestionId, SelectedControl->IfrOpCode);
    }
    break;

  case LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP:
  case LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN:
    Action = (SelectedPanelItem->ItemId == LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP) ? LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_PAGE_UP :
                                                                                 LTDE_SETUP_PAGE_PANEL_HOT_KEY_ACTION_PAGE_DOWN;
    for (Index = 0; Index < sizeof (mSetupPagePanelHotKeyInfoList) / sizeof (LTDE_SETUP_PAGE_PANEL_HOT_KEY_INFO); Index++) {
      if (mSetupPagePanelHotKeyInfoList[Index].Action == Action) {
        SetupPagePanelProcessHotKey (&mSetupPagePanelHotKeyInfoList[Index].InputKey);
        break;
      }
    }
    break;

  default:
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}


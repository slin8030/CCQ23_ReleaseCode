/** @file
  Initialization and display related functions for hot key panel.

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
#include "LTDEPrint.h"
#include "LTDEMisc.h"

#define LTDE_HOT_KEY_PANEL_CONTROL_INTERVAL_WIDTH             1
#define LTDE_HOT_KEY_PANEL_TEXT_VALUE_INTERVAL_WIDTH          1
#define LTDE_HOT_KEY_PANEL_GROUP_SEPARATED_STRING             L"/"

/**
  Converts unicode string from uppercase to lowercase.

  @param[in, out] Str        String to be converted
**/
STATIC
VOID
ToLowerStr (
  IN OUT CHAR16                               *Str
  )
{
  CHAR16                                      *Ptr;

  if (Str == NULL) {
    return;
  }

  for (Ptr = Str; *Ptr != L'\0'; Ptr++) {
    if (*Ptr >= L'A' && *Ptr <= L'Z') {
      *Ptr = (CHAR16) (*Ptr - L'A' + L'a');
    }
  }
}

/**
  Get hot key string from hot key info.
  It will convert to image string if hot key string is up, down, left and right.

  @param[in] HotKey        A pointer to hot key info instance

  @return The hot key string pointer or NULL if HotKey is NULL or allocate pool fail.
**/
STATIC
CHAR16 *
GetHotKeyString (
  IN HOT_KEY_INFO                             *HotKey
  )
{
  UINTN                                       StringSize;
  CHAR16                                      *String;
  CHAR16                                      CharString[2];

  if (HotKey == NULL) {
    return NULL;
  }

  StringSize = StrSize (HotKey->Mark);
  String     = AllocateCopyPool (StringSize, HotKey->Mark);
  if (String == NULL) {
    return NULL;
  }

  ToLowerStr (String);
  ZeroMem (CharString, sizeof (CharString));

  if (StrCmp (String, L"up") == 0) {
    CharString[0] = ARROW_UP;
  } else if (StrCmp (String, L"down") == 0) {
    CharString[0] = ARROW_DOWN;
  } else if (StrCmp (String, L"left") == 0) {
    CharString[0] = ARROW_LEFT;
  } else if (StrCmp (String, L"right") == 0) {
    CharString[0] = ARROW_RIGHT;
  } else {
    CopyMem (String, HotKey->Mark, StringSize);
    return String;
  }

  FreePool (String);
  return AllocateCopyPool (sizeof(CharString), CharString);
}

/**
  Get key data of hot key control which is selected by mouse click.

  @param[in]  HotKeyPanel        A pointer to hot key panel instance
  @param[in]  SelectedPanelItem  A pointer to selected panel item instance
  @param[in]  SelectedControl    A pointer to selected control instance
  @param[in]  UserInputData      A pointer to user input data
  @param[out] KeyData            A pointer to return key data of hot key

  @retval EFI_SUCCESS            Get key data successful.
  @retval EFI_INVALID_PARAMETER  Input parameter is NULL or it is not mouse input.
  @retval EFI_NOT_FOUND          User is not click on hot key action field.
  @retval EFI_OUT_OF_RESOURCES   Allocate pool fail.
**/
STATIC
EFI_STATUS
GetKeyDataByMouseClick (
  IN  H2O_LTDE_PANEL                          *HotKeyPanel,
  IN  H2O_LTDE_PANEL_ITEM                     *SelectedPanelItem,
  IN  H2O_LTDE_CONTROL                        *SelectedControl,
  IN  H2O_DISPLAY_ENGINE_USER_INPUT_DATA      *UserInputData,
  OUT EFI_KEY_DATA                            *KeyData
  )
{
  RECT                                         HotKeyActionField;
  HOT_KEY_INFO                                 *HotKeyInfo;
  UINT32                                       HotKeyInfoCount;
  CHAR16                                       *HotKeyString;
  CHAR16                                       *HotKeyStringPtr;
  UINT32                                       HotKeyStartX;
  UINT32                                       HotKeyEndX;
  CHAR16                                       *SignatureStrPtr;
  UINT32                                       SignatureStrLen;

  if (UserInputData == NULL || SelectedControl == NULL || KeyData == NULL ||
      UserInputData->IsKeyboard || SelectedControl->HiiValue.Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check if user is click on the prompt string field of hot ket control.
  //
  CopyRect (&HotKeyActionField, &SelectedControl->Text.StringField);
  OffsetRect (&HotKeyActionField, SelectedPanelItem->ItemField.left, SelectedPanelItem->ItemField.top);
  if (!IsPointOnField (&HotKeyActionField, UserInputData->CursorX, UserInputData->CursorY)) {
    return EFI_NOT_FOUND;
  }

  HotKeyInfo      = (HOT_KEY_INFO *) SelectedControl->HiiValue.Buffer;
  HotKeyInfoCount = SelectedControl->HiiValue.BufferLen / sizeof (HOT_KEY_INFO);
  if (HotKeyInfoCount == 1) {
    CopyMem (KeyData, &HotKeyInfo->KeyData, sizeof (EFI_KEY_DATA));
    return EFI_SUCCESS;
  }

  //
  // User click on the prompt string field of hot ket control which contain multiple hot keys.
  // Get the key data of user selected hot key.
  //
  HotKeyString = AllocateCopyPool (StrSize (SelectedControl->Text.String), SelectedControl->Text.String);
  if (HotKeyString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  HotKeyStartX    = HotKeyActionField.left;
  HotKeyStringPtr = HotKeyString;
  SignatureStrLen = sizeof (LTDE_HOT_KEY_PANEL_GROUP_SEPARATED_STRING) / sizeof (CHAR16) - 1;
  while (UserInputData->CursorX >= HotKeyStartX) {
    SignatureStrPtr = StrStr (HotKeyStringPtr, LTDE_HOT_KEY_PANEL_GROUP_SEPARATED_STRING);
    if (SignatureStrPtr != NULL) {
      *SignatureStrPtr = CHAR_NULL;
    }

    HotKeyEndX = HotKeyStartX + (UINT32) GetStringDisplayWidth (HotKeyStringPtr) - 1;
    if (IN_RANGE (UserInputData->CursorX, HotKeyStartX, HotKeyEndX)) {
      CopyMem (KeyData, &HotKeyInfo->KeyData, sizeof (EFI_KEY_DATA));
      FreePool (HotKeyString);
      return EFI_SUCCESS;
    }
    HotKeyStartX = HotKeyEndX + SignatureStrLen + 1;

    if (SignatureStrPtr == NULL) {
      break;
    }

    HotKeyInfo++;
    HotKeyStringPtr = SignatureStrPtr + SignatureStrLen;
  }

  FreePool (HotKeyString);
  return EFI_NOT_FOUND;
}

/**
  Check if input group ID is exist in input control list or not.

  @param[in]  ControlList        A pointer to control list
  @param[in]  ControlCount       Control count
  @param[in]  GroupId            Check group ID

  @retval TRUE                   Input group ID is exist in input control list.
  @retval FALSE                  Input group ID is not exist in input control list or ControlList is NULL.
**/
STATIC
BOOLEAN
IsGroupIdExist (
  IN H2O_LTDE_CONTROL                         *ControlList,
  IN UINT32                                   ControlCount,
  IN UINT8                                    GroupId
  )
{
  UINT32                                      Index;

  if (ControlList == NULL) {
    return FALSE;
  }

  for (Index = 0; Index < ControlCount; Index++) {
    if (((HOT_KEY_INFO *) ControlList[Index].HiiValue.Buffer)->GroupId == GroupId) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
  Initialize the control list in content item of hot key panel.

  @param[in]      HotKeyPanel    A pointer to the hot key panel instance.
  @param[in, out] ContentItem    A pointer to the content item instance.

  @retval EFI_SUCCESS            Initialize the control list successful.
  @retval EFI_INVALID_PARAMETER  HotKeyPanel or ContentItem is NULL.
  @retval EFI_OUT_OF_RESOURCES   Allocate pool fail.
**/
STATIC
EFI_STATUS
InitHotKeyPanelContentItemControlList (
  IN     H2O_LTDE_PANEL                       *HotKeyPanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  )
{
  HOT_KEY_INFO                                *HotKey;
  HOT_KEY_INFO                                *HotKeyList;
  UINT32                                      HotKeyCount;
  UINT32                                      HotKeyIndex;
  UINT32                                      ControlCount;
  UINT32                                      Index;
  H2O_LTDE_CONTROL                            *ControlList;
  H2O_LTDE_CONTROL                            *Control;
  CHAR16                                      *OrgStr;
  CHAR16                                      *CurrentStr;

  if (HotKeyPanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!HotKeyPanel->Vertical) {
    return EFI_UNSUPPORTED;
  }

  if (mDEPrivate->FBProtocol->CurrentP == NULL ||
      mDEPrivate->FBProtocol->CurrentP->HotKeyInfo == NULL) {
    return EFI_SUCCESS;
  }

  HotKeyList  = mDEPrivate->FBProtocol->CurrentP->HotKeyInfo;
  HotKeyCount = 0;
  while (!IS_END_OF_HOT_KEY_INFO(&HotKeyList[HotKeyCount])) {
    HotKeyCount++;
  }
  if (HotKeyCount == 0) {
    return EFI_SUCCESS;
  }

  ControlCount = HotKeyCount;
  ControlList  = AllocateZeroPool (ControlCount * sizeof (H2O_LTDE_CONTROL));
  if (ControlList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (HotKeyIndex = 0, ControlCount = 0; HotKeyIndex < HotKeyCount; HotKeyIndex++) {
    HotKey = &HotKeyList[HotKeyIndex];
    if (HotKey->GroupId != 0 && IsGroupIdExist (ControlList, ControlCount, HotKey->GroupId) ||
        !HotKey->Display) {
      continue;
    }

    Control                      = &ControlList[ControlCount++];
    Control->Selectable          = TRUE;
    Control->Text.String         = GetHotKeyString (HotKey);
    Control->ValueStrInfo.String = AllocateCopyPool (StrSize (HotKey->String), HotKey->String);
    Control->HiiValue.BufferLen  = sizeof (HOT_KEY_INFO);
    Control->HiiValue.Buffer     = AllocateCopyPool (Control->HiiValue.BufferLen, HotKey);

    if (HotKey->GroupId != 0) {
      for (Index = HotKeyIndex + 1; Index < HotKeyCount; Index++) {
        if (HotKeyList[Index].GroupId != HotKey->GroupId) {
          continue;
        }

        OrgStr               = Control->Text.String;
        CurrentStr           = GetHotKeyString (&HotKeyList[Index]);
        Control->Text.String = CatSPrint (NULL, L"%s%s%s", OrgStr, LTDE_HOT_KEY_PANEL_GROUP_SEPARATED_STRING, CurrentStr);
        FreePool (OrgStr);
        FreePool (CurrentStr);

        Control->HiiValue.Buffer = ReallocatePool (
                                     Control->HiiValue.BufferLen,
                                     Control->HiiValue.BufferLen + sizeof (HOT_KEY_INFO),
                                     Control->HiiValue.Buffer
                                     );
        CopyMem (Control->HiiValue.Buffer + Control->HiiValue.BufferLen, &HotKeyList[Index], sizeof (HOT_KEY_INFO));
        Control->HiiValue.BufferLen += sizeof (HOT_KEY_INFO);
      }
    }
  }

  ContentItem->Vertical     = HotKeyPanel->Vertical;
  ContentItem->ControlCount = ControlCount;
  ContentItem->ControlList  = ControlList;

  return EFI_SUCCESS;
}

/**
  Initialize the content item of hot key panel.

  @param[in]      HotKeyPanel    A pointer to the hot key panel instance.
  @param[in, out] ContentItem    A pointer to the content item instance.

  @retval EFI_SUCCESS            Initialize the content item successful.
  @retval EFI_INVALID_PARAMETER  HotKeyPanel or ContentItem is NULL.
**/
STATIC
EFI_STATUS
InitHotKeyPanelContentItem (
  IN     H2O_LTDE_PANEL                       *HotKeyPanel,
  IN OUT H2O_LTDE_PANEL_ITEM                  *ContentItem
  )
{
  if (HotKeyPanel == NULL || ContentItem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ContentItem->ItemId = LTDE_PANEL_ITEM_ID_CONTENT;

  CopyRect (&ContentItem->ItemField, &HotKeyPanel->PanelField);
  InflateRect (&ContentItem->ItemField, -HotKeyPanel->BorderLineWidth, -HotKeyPanel->BorderLineWidth);

  InitHotKeyPanelContentItemControlList (HotKeyPanel, ContentItem);

  return EFI_SUCCESS;
}

/**
  Initialize hot key panel.

  @retval EFI_SUCCESS            Initialize hot key panel successful.
  @retval EFI_NOT_FOUND          Hot key panel is not found.
  @retval EFI_OUT_OF_RESOURCES   Allocate pool fail.
**/
EFI_STATUS
InitHotKeyPanel (
  VOID
  )
{
  H2O_LTDE_PANEL                              *HotKeyPanel;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;

  HotKeyPanel = GetPanel (H2O_PANEL_TYPE_HOTKEY);
  if (HotKeyPanel == NULL) {
    return EFI_NOT_FOUND;
  }

  ContentItem = AllocateZeroPool (sizeof (H2O_LTDE_PANEL_ITEM));
  if (ContentItem == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  InitHotKeyPanelContentItem (HotKeyPanel, ContentItem);

  FreePanelItemList (HotKeyPanel->ItemList, HotKeyPanel->ItemCount);
  HotKeyPanel->ItemCount = 1;
  HotKeyPanel->ItemList  = ContentItem;

  return EFI_SUCCESS;
}

/**
  Display hot key panel.

  @retval EFI_SUCCESS            Display help text panel successful.
  @retval EFI_OUT_OF_RESOURCES   Allocate pool fail.
**/
EFI_STATUS
DisplayHotKeyPanel (
  VOID
  )
{
  H2O_LTDE_PANEL                                 *HotKeyPanel;
  H2O_LTDE_PANEL_ITEM                            *ContentItem;
  UINT32                                         HotKeyIndex;
  H2O_LTDE_CONTROL                               *Control;
  INT32                                          BackupX;
  INT32                                          AbsX;
  INT32                                          AbsY;
  INT32                                          ClientX;
  INT32                                          ClientY;
  UINT32                                         ContentItemHeight;
  UINT32                                         ContentItemWidth;
  UINT32                                         ControlNumPerLine;
  UINT32                                         ControlWidth;
  UINT32                                         RemainderWidth;
  UINT32                                         MaxTextStrWidth;
  UINT32                                         MaxValueStrWidth;
  UINT32                                         *TextStrWidth;
  UINT32                                         *ValueStrWidth;
  UINT32                                         ColorAttribute;
  UINT32                                         HighlightColorAttribute;
  UINT32                                         TextValueInterval;
  CHAR16                                         *ControlIntervalStr;
  CHAR16                                         *String;
  CHAR16                                         *TempString;
  BOOLEAN                                        IsLast;

  HotKeyPanel = GetPanel (H2O_PANEL_TYPE_HOTKEY);
  if (HotKeyPanel == NULL || !HotKeyPanel->Visible) {
    return EFI_SUCCESS;
  }

  ContentItem = GetPanelItem (HotKeyPanel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem == NULL) {
    return EFI_SUCCESS;
  }

  if (ContentItem->ControlCount == 0) {
    ClearField (HotKeyPanel->ColorAttribute, &ContentItem->ItemField);
    return EFI_SUCCESS;
  }

  AbsX               = ContentItem->ItemField.left;
  AbsY               = ContentItem->ItemField.top;
  ContentItemHeight  = H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField);
  ContentItemWidth   = H2O_LTDE_FIELD_WIDTH (&ContentItem->ItemField);
  ControlNumPerLine  = ContentItem->ControlCount / ContentItemHeight;
  ControlNumPerLine += (ContentItem->ControlCount % ContentItemHeight != 0) ? 1 : 0;
  ControlWidth       = (ContentItemWidth - (ControlNumPerLine + 1) * LTDE_HOT_KEY_PANEL_CONTROL_INTERVAL_WIDTH) / ControlNumPerLine;
  RemainderWidth     = (ContentItemWidth - (ControlNumPerLine + 1) * LTDE_HOT_KEY_PANEL_CONTROL_INTERVAL_WIDTH) % ControlNumPerLine;
  TextValueInterval  = LTDE_HOT_KEY_PANEL_TEXT_VALUE_INTERVAL_WIDTH;
  GetPanelColorAttribute (HotKeyPanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL   , &ColorAttribute);
  GetPanelColorAttribute (HotKeyPanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_HIGHLIGHT, &HighlightColorAttribute);

  //
  // Set the location of hot key control and print it.
  //
  ClientX            = 0;
  ClientY            = 0;
  MaxTextStrWidth    = 0;
  MaxValueStrWidth   = 0;
  IsLast             = FALSE;
  TextStrWidth       = AllocateZeroPool (sizeof (UINT32) * ContentItemHeight);
  ValueStrWidth      = AllocateZeroPool (sizeof (UINT32) * ContentItemHeight);
  ControlIntervalStr = CreateString (LTDE_HOT_KEY_PANEL_CONTROL_INTERVAL_WIDTH, ' ');
  if (TextStrWidth == NULL || ValueStrWidth == NULL || ControlIntervalStr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (HotKeyIndex = 0; HotKeyIndex < ContentItem->ControlCount; HotKeyIndex++) {
    ClientY = HotKeyIndex % ContentItemHeight;
    if (ClientY == 0) {
      if (HotKeyIndex != 0) {
        ClientX += (ControlWidth + LTDE_HOT_KEY_PANEL_CONTROL_INTERVAL_WIDTH);
      }

      if (HotKeyIndex + ContentItemHeight >= ContentItem->ControlCount) {
        IsLast        = TRUE;
        ControlWidth += RemainderWidth;
      }

      MaxTextStrWidth  = 0;
      MaxValueStrWidth = 0;
      for (ClientY = 0; ClientY < (INT32) ContentItemHeight; ClientY++) {
        if (HotKeyIndex + ClientY >= ContentItem->ControlCount) {
          break;
        }

        TextStrWidth[ClientY]  = (UINT32) GetStringDisplayWidth (ContentItem->ControlList[HotKeyIndex + ClientY].Text.String);
        ValueStrWidth[ClientY] = (UINT32) GetStringDisplayWidth (ContentItem->ControlList[HotKeyIndex + ClientY].ValueStrInfo.String);
        MaxTextStrWidth  = MAX (MaxTextStrWidth , TextStrWidth[ClientY]);
        MaxValueStrWidth = MAX (MaxValueStrWidth, ValueStrWidth[ClientY]);
      }
      ClientY = 0;

      if (MaxTextStrWidth + MaxValueStrWidth + TextValueInterval > ControlWidth) {
        MaxTextStrWidth  = ControlWidth / 2;
        MaxValueStrWidth = ControlWidth - MaxTextStrWidth - TextValueInterval;
      }
    }

    BackupX = ClientX;
    Control = &ContentItem->ControlList[HotKeyIndex];

    if (LTDE_HOT_KEY_PANEL_CONTROL_INTERVAL_WIDTH > 0) {
      DEConOutSetAttribute (ColorAttribute);
      DisplayString (AbsX + ClientX, AbsY + ClientY, ControlIntervalStr);
      ClientX += LTDE_HOT_KEY_PANEL_CONTROL_INTERVAL_WIDTH;
    }

    SetRect (
      &Control->ControlField,
      ClientX,
      ClientY,
      ClientX + ControlWidth - 1,
      ClientY
      );

    //
    // Process text string
    //
    SetRect (
      &Control->Text.StringField,
      ClientX,
      ClientY,
      ClientX + TextStrWidth[ClientY]  - 1,
      ClientY
      );
    DEConOutSetAttribute (HighlightColorAttribute);
    String = GetAlignmentString (Control->Text.String, MaxTextStrWidth, LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT);
    if (String != NULL) {
      if (TextValueInterval > 0) {
        TempString = String;
        String = GetAlignmentString (TempString, MaxTextStrWidth + TextValueInterval, LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT);
        FreePool (TempString);
      }
      DisplayString (AbsX + ClientX, AbsY + ClientY, String);
      FreePool (String);
    }
    ClientX += (MaxTextStrWidth + TextValueInterval);

    //
    // Process value string
    //
    SetRect (
      &Control->ValueStrInfo.StringField,
      ClientX,
      ClientY,
      ClientX + ValueStrWidth[ClientY] - 1,
      ClientY
      );
    DEConOutSetAttribute (ColorAttribute);
    String = GetAlignmentString (Control->ValueStrInfo.String, ControlWidth - MaxTextStrWidth - TextValueInterval, LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT);
    if (String != NULL) {
      DisplayString (AbsX + ClientX, AbsY + ClientY, String);
      FreePool (String);
    }

    if (IsLast && LTDE_HOT_KEY_PANEL_CONTROL_INTERVAL_WIDTH > 0) {
      DisplayString (AbsX + ClientX + (ControlWidth - MaxTextStrWidth - TextValueInterval), AbsY + ClientY, ControlIntervalStr);
    }

    ClientX = BackupX;
  }

  if ((HotKeyIndex % ContentItemHeight) != 0) {
    String = CreateString (ControlWidth + LTDE_HOT_KEY_PANEL_CONTROL_INTERVAL_WIDTH * 2, ' ');
    if (String == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    while ((HotKeyIndex % ContentItemHeight) != 0) {
      ClientY = HotKeyIndex % ContentItemHeight;
      DisplayString (AbsX + ClientX, AbsY + ClientY, String);
      HotKeyIndex++;
    }
    FreePool (String);
  }

  FreePool (TextStrWidth);
  FreePool (ValueStrWidth);
  FreePool (ControlIntervalStr);

  return EFI_SUCCESS;
}

/**
  Process mouse click selected control in hot key panel.

  @param[in] HotKeyPanel         A pointer to the hot key panel instance.
  @param[in] SelectedPanelItem   A pointer to the mouse selected panel item.
  @param[in] SelectedControl     A pointer to the mouse selected control.
  @param[in] UserInputData       A pointer to the user input data.

  @retval EFI_SUCCESS            Process user input successful.
  @retval EFI_INVALID_PARAMETER  Input parameter is NULL.
  @retval Other                  User is not click on hot key control or send hot key event fail.
**/
EFI_STATUS
HotKeyPanelProcessMouseClick (
  IN H2O_LTDE_PANEL                           *HotKeyPanel,
  IN H2O_LTDE_PANEL_ITEM                      *SelectedPanelItem,
  IN H2O_LTDE_CONTROL                         *SelectedControl,
  IN H2O_DISPLAY_ENGINE_USER_INPUT_DATA       *UserInputData
  )
{
  EFI_STATUS                                  Status;
  H2O_DISPLAY_ENGINE_USER_INPUT_DATA          HotKeyInputData;
  HOT_KEY_INFO                                HotKey;

  if (HotKeyPanel == NULL || SelectedPanelItem == NULL || SelectedControl == NULL || UserInputData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem (&HotKeyInputData, sizeof (HotKeyInputData));
  HotKeyInputData.IsKeyboard = TRUE;

  Status = GetKeyDataByMouseClick (HotKeyPanel, SelectedPanelItem, SelectedControl, UserInputData, &HotKeyInputData.KeyData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = CheckFBHotKey (&HotKeyInputData, &HotKey);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return SendEvtByHotKey (&HotKey);
}


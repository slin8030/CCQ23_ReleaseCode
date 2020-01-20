/** @file
  Control (which make up panel) related Functions for H2O display engine driver.

;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "LTDEControl.h"
#include "LTDEPanels.h"
#include "LTDEPrint.h"
#include "LTDEMisc.h"

H2O_LTDE_CONTROL *
GetControlById (
  IN H2O_LTDE_PANEL                           *Panel,
  IN UINT32                                   ItemId,
  IN UINT32                                   ControlId
  )
{
  H2O_LTDE_PANEL_ITEM                         *PanelItem;

  PanelItem = GetPanelItem (Panel, ItemId);
  if (PanelItem == NULL) {
    return NULL;
  }

  return FindControlByControlId (PanelItem->ControlList, PanelItem->ControlCount, ControlId, 0);
}

H2O_LTDE_CONTROL *
FindControlByControlId (
  IN H2O_LTDE_CONTROL                         *ControlArray,
  IN UINT32                                   ControlArrayCount,
  IN UINT32                                   ControlId,
  IN UINT32                                   SequenceIndex
  )
{
  UINT32                                      Index;
  UINT32                                      Count;

  if (ControlArray == NULL) {
    return NULL;
  }

  Count = 0;

  for (Index = 0; Index < ControlArrayCount; Index++) {
    if (ControlArray[Index].ControlId != ControlId) {
      continue;
    }

    if (Count == SequenceIndex) {
      return &ControlArray[Index];
    }

    Count++;
  }

  return NULL;
}

H2O_LTDE_CONTROL *
GetControlByQuestionId (
  IN H2O_LTDE_CONTROL                         *ControlList,
  IN UINT32                                   ControlCount,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  )
{
  UINT32                                      Index;

  if (ControlList == NULL || ControlCount == 0) {
    return NULL;
  }

  for (Index = 0; Index < ControlCount; Index++) {
    if ((QuestionId != 0    && ControlList[Index].QuestionId == QuestionId) ||
        (IfrOpCode  != NULL && ControlList[Index].IfrOpCode  == IfrOpCode)) {
      return &ControlList[Index];
    }
  }

  return NULL;
}

/**
  Will copy LineWidth amount of a string in the OutputString buffer and return the
  number of CHAR16 characters that were copied into the OutputString buffer.
  The output string format is:
    Glyph Info + String info + '\0'.

  In the code, it deals \r,\n,\r\n same as \n\r, also it not process the \r or \g.

  @param  InputString            String description for this option.
  @param  LineWidth              Width of the desired string to extract in CHAR16
                                 characters
  @param  GlyphWidth             The glyph width of the begin of the char in the string.
  @param  Index                  Where in InputString to start the copy process
  @param  OutputString           Buffer to copy the string into

  @return Returns the number of CHAR16 characters that were copied into the OutputString
  buffer, include extra glyph info and '\0' info.

**/
STATIC
UINT32
GetLineByWidth (
  IN     CHAR16                               *InputString,
  IN     UINT32                               LineWidth,
  IN OUT UINT32                               *GlyphWidth,
  IN OUT UINTN                                *Index,
  OUT    CHAR16                               **OutputString OPTIONAL
  )
{
  UINT32                                      StrOffset;
  UINT32                                      GlyphOffset;
  UINT32                                      OriginalGlyphWidth;
  BOOLEAN                                     ReturnFlag;
  UINT32                                      LastSpaceOffset;
  UINT32                                      LastGlyphWidth;

  if (InputString == NULL || Index == NULL) {
    return 0;
  }

  if (LineWidth == 0 || *GlyphWidth == 0) {
    return 0;
  }

  //
  // Save original glyph width.
  //
  OriginalGlyphWidth = *GlyphWidth;
  LastGlyphWidth     = OriginalGlyphWidth;
  ReturnFlag         = FALSE;
  LastSpaceOffset    = 0;

  //
  // NARROW_CHAR can not be printed in screen, so if a line only contain  the two CHARs: 'NARROW_CHAR + CHAR_CARRIAGE_RETURN' , it is a empty line  in Screen.
  // To avoid displaying this  empty line in screen,  just skip  the two CHARs here.
  //
  if ((InputString[*Index] == NARROW_CHAR) && (InputString[*Index + 1] == CHAR_CARRIAGE_RETURN)) {
    *Index = *Index + 2;
  }

  //
  // Fast-forward the string and see if there is a carriage-return in the string
  //
  for (StrOffset = 0, GlyphOffset = 0; GlyphOffset <= LineWidth; StrOffset++) {
    switch (InputString[*Index + StrOffset]) {
      case NARROW_CHAR:
        *GlyphWidth = 1;
        break;

      case WIDE_CHAR:
        *GlyphWidth = 2;
        break;

      case CHAR_CARRIAGE_RETURN:
      case CHAR_LINEFEED:
      case CHAR_NULL:
        ReturnFlag = TRUE;
        break;

      default:
        GlyphOffset = GlyphOffset + *GlyphWidth;

        //
        // Record the last space info in this line. Will be used in rewind.
        //
        if ((InputString[*Index + StrOffset] == CHAR_SPACE) && (GlyphOffset <= LineWidth)) {
          LastSpaceOffset = StrOffset;
          LastGlyphWidth  = *GlyphWidth;
        }
        break;
    }

    if (ReturnFlag) {
      break;
    }
  }

  //
  // Rewind the string from the maximum size until we see a space to break the line
  //
  if (GlyphOffset > LineWidth) {
    //
    // Rewind the string to last space char in this line.
    //
    if (LastSpaceOffset != 0) {
      StrOffset   = LastSpaceOffset;
      *GlyphWidth = LastGlyphWidth;
    } else {
      //
      // Roll back to last char in the line width.
      //
      StrOffset--;
    }
  }

  //
  // The CHAR_NULL has process last time, this time just return 0 to stand for the end.
  //
  if (StrOffset == 0 && (InputString[*Index + StrOffset] == CHAR_NULL)) {
    return 0;
  }

  if (OutputString != NULL) {
    //
    // Need extra glyph info and '\0' info, so +2.
    //
    *OutputString = AllocateZeroPool (((UINTN) (StrOffset + 2) * sizeof(CHAR16)));
    if (*OutputString == NULL) {
      return 0;
    }

    //
    // Save the glyph info at the begin of the string, will used by Print function.
    //
    if (OriginalGlyphWidth == 1) {
      *(*OutputString) = NARROW_CHAR;
    } else  {
      *(*OutputString) = WIDE_CHAR;
    }

    CopyMem ((*OutputString) + 1, &InputString[*Index], StrOffset * sizeof(CHAR16));
  }

  if (InputString[*Index + StrOffset] == CHAR_SPACE) {
    //
    // Skip the space info at the begin of next line.
    //
    *Index = (UINT16) (*Index + StrOffset + 1);
  } else if (InputString[*Index + StrOffset] == CHAR_LINEFEED) {
    //
    // Skip the /n or /n/r info.
    //
    if (InputString[*Index + StrOffset + 1] == CHAR_CARRIAGE_RETURN) {
      *Index = (UINT16) (*Index + StrOffset + 2);
    } else {
      *Index = (UINT16) (*Index + StrOffset + 1);
    }
  } else if (InputString[*Index + StrOffset] == CHAR_CARRIAGE_RETURN) {
    //
    // Skip the /r or /r/n info.
    //
    if (InputString[*Index + StrOffset + 1] == CHAR_LINEFEED) {
      *Index = (UINT16) (*Index + StrOffset + 2);
    } else {
      *Index = (UINT16) (*Index + StrOffset + 1);
    }
  } else {
    *Index = (UINT16) (*Index + StrOffset);
  }

  //
  // Include extra glyph info and '\0' info, so +2.
  //
  return StrOffset + 2;
}

UINT32
GetStringHeight (
  IN CHAR16                                   *String,
  IN UINT32                                   LineWidth
  )
{
  UINT32                                      LineCount;
  UINT32                                      GlyphWidth;
  UINTN                                       StringIndex;

  if (String == NULL || LineWidth == 0) {
    return 0;
  }

  GlyphWidth  = 1;
  StringIndex = 0;
  LineCount   = 0;
  while (GetLineByWidth (String, LineWidth, &GlyphWidth, &StringIndex, NULL) != 0) {
    LineCount++;
  }

  return LineCount;
}

EFI_STATUS
GetStringArrayByWidth (
  IN  CHAR16                                  *String,
  IN  UINT32                                  LineWidth,
  OUT UINT32                                  *StringArrayNum,
  OUT CHAR16                                  ***StringArray
  )
{
  UINTN                                       StrPtrIndex;
  UINTN                                       StrPtrCount;
  CHAR16                                      **StrPtrArray;
  UINT32                                      GlyphWidth;
  UINTN                                       LineIndex;
  CHAR16                                      *LineString;

  if (String == NULL || *String == CHAR_NULL || LineWidth == 0 || StringArrayNum == NULL || StringArray == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  StrPtrIndex = 0;
  StrPtrCount = 10;
  StrPtrArray = AllocateZeroPool (StrPtrCount * sizeof (CHAR16 *));
  if (StrPtrArray == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  GlyphWidth = 1;
  LineIndex  = 0;
  LineString = NULL;
  while (GetLineByWidth (String, LineWidth, &GlyphWidth, &LineIndex, &LineString) != 0) {
    if (LineString == NULL) {
      continue;
    }

    if (StrPtrIndex >= StrPtrCount) {
      StrPtrArray = ReallocatePool (
                   StrPtrCount * sizeof (CHAR16 *),
                   (StrPtrCount + 10) * sizeof (CHAR16 *),
                   StrPtrArray
                   );
      if (StrPtrArray == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      StrPtrCount += 10;
    }

    StrPtrArray[StrPtrIndex] = LineString;
    StrPtrIndex++;
  }

  *StringArrayNum = (UINT32) StrPtrIndex;
  *StringArray    = StrPtrArray;

  return EFI_SUCCESS;
}

EFI_STATUS
CalculateRequireSize (
  IN  CHAR16                                  *DisplayString,
  IN  UINT32                                  LimitLineWidth,
  OUT UINT32                                  *RequireWidth,
  OUT UINT32                                  *RequireHeight
  )
{
  EFI_STATUS                                  Status;
  UINTN                                       MaxStringWidth;
  UINT32                                      Index;
  UINT32                                      SeparateStringNum;
  CHAR16                                      **SeparateString;

  if (DisplayString == NULL || RequireWidth == NULL || RequireHeight == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (*DisplayString == CHAR_NULL) {
    *RequireWidth  = 0;
    *RequireHeight = 0;
    return EFI_SUCCESS;
  }

  Status = GetStringArrayByWidth (DisplayString, LimitLineWidth, &SeparateStringNum, &SeparateString);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MaxStringWidth = 0;
  for (Index = 0; Index < SeparateStringNum; Index++) {
    MaxStringWidth = MAX (MaxStringWidth, GetStringDisplayWidth (SeparateString[Index]));
    SafeFreePool ((VOID **) &SeparateString[Index]);
  }
  SafeFreePool ((VOID **) &SeparateString);

  *RequireWidth  = (UINT32) MaxStringWidth;
  *RequireHeight = SeparateStringNum;

  return EFI_SUCCESS;
}

CHAR16 *
GetAlignmentString (
  IN CHAR16                                   *String,
  IN UINT32                                   LineWidth,
  IN UINT32                                   AlignmentAction
  )
{
  UINT32                                      DisplayWidth;
  UINT32                                      StringLen;
  UINT32                                      Difference;
  UINT32                                      PrefixCount;
  UINT32                                      SuffixCount;
  CHAR16                                      *ResultString;
  CHAR16                                      *ResultStringPtr;
  UINT32                                      Index;

  if (String == NULL || LineWidth == 0 || AlignmentAction >= LTDE_STRING_ALIGNMENT_ACTION_MAX) {
    return NULL;
  }

  DisplayWidth = (UINT32) GetStringDisplayWidth (String);
  StringLen    = (UINT32) StrLen (String);
  if (DisplayWidth >= LineWidth) {
    ResultString = AllocateCopyPool ((StringLen + 1) * sizeof (CHAR16), String);
    if (ResultString == NULL) {
      return NULL;
    }

    while (DisplayWidth > LineWidth) {
      StringLen--;
      DisplayWidth -= ConsoleLibGetGlyphWidth (ResultString[StringLen]);
      ResultString[StringLen] = CHAR_NULL;
    }
    return ResultString;
  }

  Difference   = LineWidth - DisplayWidth;
  ResultString = AllocateZeroPool ((StringLen + 1 + Difference + 2) * sizeof (CHAR16)); // 2 for extra glyph info
  if (ResultString == NULL) {
    return NULL;
  }

  ResultStringPtr = ResultString;

  switch (AlignmentAction) {

  case LTDE_STRING_ALIGNMENT_ACTION_FLUSH_LEFT:
    CopyMem (ResultStringPtr, String, (StringLen + 1) * sizeof (CHAR16));
    ResultStringPtr += StringLen;

    *ResultStringPtr = NARROW_CHAR;
    ResultStringPtr++;

    for (Index = 0; Index < Difference; Index++, ResultStringPtr++) {
      *ResultStringPtr = ' ';
    }
    break;

  case LTDE_STRING_ALIGNMENT_ACTION_FLUSH_RIGHT:
    *ResultStringPtr = NARROW_CHAR;
    ResultStringPtr++;

    for (Index = 0; Index < Difference; Index++, ResultStringPtr++) {
      *ResultStringPtr = ' ';
    }

    CopyMem (ResultStringPtr, String, (StringLen + 1) * sizeof (CHAR16));
    break;

  case LTDE_STRING_ALIGNMENT_ACTION_CENTERED:
    PrefixCount = Difference / 2;
    SuffixCount = Difference - PrefixCount;

    *ResultStringPtr = NARROW_CHAR;
    ResultStringPtr++;

    for (Index = 0; Index < PrefixCount; Index++, ResultStringPtr++) {
      *ResultStringPtr = ' ';
    }

    CopyMem (ResultStringPtr, String, (StringLen + 1) * sizeof (CHAR16));
    ResultStringPtr += StringLen;

    for (Index = 0; Index < SuffixCount; Index++, ResultStringPtr++) {
      *ResultStringPtr = ' ';
    }
    break;

  default:
    FreePool (ResultString);
    return NULL;
  }

  return ResultString;
}

/**
 Clear field with specific color attribute

 @param[in] Attribute             Color attribute for clear field
 @param[in] Field                 Pointer to clear field

 @retval EFI_SUCCESS              Clear field successfully
 @retval EFI_OUT_OF_RESOURCES     Fail to allocate pool
 @retval Other                    Fail to get screen resolution
**/
EFI_STATUS
ClearField (
  IN CONST UINT32                             Attribute,
  IN       RECT                               *Field
  )
{
  EFI_STATUS                                  Status;
  INT32                                       Index;
  INT32                                       ClearWidth;
  CHAR16                                      *ClearString;
  INT32                                       ScreenRow;
  INT32                                       ScreenColumn;
  BOOLEAN                                     SkipLastChar;

  Status = DEConOutQueryModeWithoutModeNumer ((UINT32 *) &ScreenColumn, (UINT32 *) &ScreenRow);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ScreenColumn--;
  ScreenRow--;

  ClearWidth  = H2O_LTDE_FIELD_WIDTH (Field);
  ClearString = CreateString (ClearWidth, ' ');
  if (ClearString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (Field->bottom                == ScreenRow &&
      Field->left + ClearWidth - 1 == ScreenColumn) {
    SkipLastChar = TRUE;
  } else {
    SkipLastChar = FALSE;
  }

  DEConOutSetAttribute ((UINTN) Attribute);

  for (Index = Field->top; Index <= Field->bottom; Index++) {
    if (SkipLastChar && (Index == ScreenRow)) {
      ClearString[ClearWidth - 1] = CHAR_NULL;
    }

    DisplayString (Field->left, Index, ClearString);
  }

  SafeFreePool ((VOID **) &ClearString);

  return EFI_SUCCESS;
}

VOID
FreeControlInfo (
  IN H2O_LTDE_CONTROL                         *Control
  )
{
  if (Control == NULL) {
    return;
  }

  SafeFreePool ((VOID **) &Control->Text.String);
  SafeFreePool ((VOID **) &Control->ValueStrInfo.String);
  SafeFreePool ((VOID **) &Control->HiiValue.Buffer);
}

VOID
FreePanelItemList (
  IN H2O_LTDE_PANEL_ITEM                      *ItemList,
  IN UINT32                                   ItemCount
  )
{
  UINT32                                      ItemIndex;
  UINT32                                      ControlIndex;

  if (ItemList == NULL || ItemCount == 0) {
    return;
  }

  for (ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++) {
    if (ItemList[ItemIndex].ControlList  == NULL ||
        ItemList[ItemIndex].ControlCount == 0) {
      continue;
    }

    for (ControlIndex = 0; ControlIndex < ItemList[ItemIndex].ControlCount; ControlIndex++) {
      FreeControlInfo (&ItemList[ItemIndex].ControlList[ControlIndex]);
    }
    FreePool (ItemList[ItemIndex].ControlList);
  }
  FreePool (ItemList);
}

H2O_LTDE_PANEL *
CreatePanel (
  VOID
  )
{
  H2O_LTDE_PANEL                             *Panel;

  Panel = AllocateZeroPool (sizeof (H2O_LTDE_PANEL));
  if (Panel == NULL) {
    return NULL;
  }

  Panel->Signature = H2O_LTDE_PANEL_SIGNATURE;
  InitializeListHead (&Panel->Link);
  InsertTailList (&mDEPrivate->PanelListHead, &Panel->Link);

  return Panel;
}

VOID
FreePanel (
  IN H2O_LTDE_PANEL                           *Panel
  )
{
  if (Panel == NULL) {
    return;
  }

  FreePanelItemList (Panel->ItemList, Panel->ItemCount);
  SafeFreePool ((VOID **) &Panel->VfcfPanelInfo->ContentsImage.CurrentBlt);
}

H2O_LTDE_PANEL *
GetPanel (
  IN UINT32                                   PanelType
  )
{
  LIST_ENTRY                                  *Link;
  H2O_LTDE_PANEL                              *Panel;

  if (IsListEmpty (&mDEPrivate->PanelListHead)) {
    return NULL;
  }

  Link = GetFirstNode (&mDEPrivate->PanelListHead);
  while (!IsNull (&mDEPrivate->PanelListHead, Link)) {
    Panel = H2O_LTDE_PANEL_FROM_LINK (Link);

    if (Panel->VfcfPanelInfo != NULL &&
        Panel->VfcfPanelInfo->PanelType == PanelType) {
      return Panel;
    }

    Link = GetNextNode (&mDEPrivate->PanelListHead, Link);
  }

  return NULL;
}

H2O_LTDE_PANEL_ITEM *
GetPanelItem (
  IN H2O_LTDE_PANEL                           *Panel,
  IN UINT32                                   ItemId
  )
{
  UINT32                                      Index;

  if (Panel == NULL || Panel->ItemList == NULL) {
    return NULL;
  }

  for (Index = 0; Index < Panel->ItemCount; Index++) {
    if (Panel->ItemList[Index].ItemId == ItemId) {
      return &Panel->ItemList[Index];
    }
  }

  return NULL;
}

H2O_LTDE_PANEL_ITEM *
GetPanelItemByControl (
  IN H2O_LTDE_PANEL                           *Panel,
  IN H2O_LTDE_CONTROL                         *Control
  )
{
  UINT32                                      Index;
  UINT32                                      ControlIndex;
  H2O_LTDE_PANEL_ITEM                         *PanelItem;

  if (Panel == NULL || Control == NULL || Panel->ItemList == NULL) {
    return NULL;
  }

  for (Index = 0; Index < Panel->ItemCount; Index++) {
    PanelItem = &Panel->ItemList[Index];

    for (ControlIndex = 0; ControlIndex < PanelItem->ControlCount; ControlIndex++) {
      if (&PanelItem->ControlList[ControlIndex] == Control) {
        return PanelItem;
      }
    }
  }

  return NULL;
}

STATIC
H2O_LTDE_PANEL_ITEM *
GetPanelItemByMouse (
  IN H2O_LTDE_PANEL                           *Panel,
  IN INT32                                    MouseX,
  IN INT32                                    MouseY
  )
{
  UINT32                                      Index;
  H2O_LTDE_PANEL_ITEM                         *PanelItem;

  if (Panel == NULL) {
    return NULL;
  }

  for (Index = 0; Index < Panel->ItemCount; Index++) {
    PanelItem = &Panel->ItemList[Index];
    if (!PanelItem->Hidden && IsPointOnField (&PanelItem->ItemField, MouseX, MouseY)) {
      return PanelItem;
    }
  }

  return NULL;
}

EFI_STATUS
GetControlByMouse (
  IN  H2O_LTDE_PANEL                          *Panel,
  IN  INT32                                   MouseX,
  IN  INT32                                   MouseY,
  OUT H2O_LTDE_PANEL_ITEM                     **SelectedPanelItem,
  OUT H2O_LTDE_CONTROL                        **SelectedControl
  )
{
  H2O_LTDE_PANEL_ITEM                         *PanelItem;
  INT32                                       MouseClientX;
  INT32                                       MouseClientY;
  UINT32                                      Index;
  H2O_LTDE_CONTROL                            *Control;

  if (Panel == NULL || SelectedPanelItem == NULL || SelectedControl == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PanelItem = GetPanelItemByMouse (Panel, MouseX, MouseY);
  if (PanelItem == NULL) {
    return EFI_NOT_FOUND;
  }

  if (PanelItem->Vertical) {
    MouseClientX = MouseX - PanelItem->ItemField.left;
    MouseClientY = MouseY - PanelItem->ItemField.top + PanelItem->CurrentPos;
  } else {
    MouseClientX = MouseX - PanelItem->ItemField.left + PanelItem->CurrentPos;
    MouseClientY = MouseY - PanelItem->ItemField.top;
  }

  for (Index = 0; Index < PanelItem->ControlCount; Index++) {
    Control = &PanelItem->ControlList[Index];
    if (IsPointOnField (&Control->ControlField, MouseClientX, MouseClientY)) {
      *SelectedPanelItem = PanelItem;
      *SelectedControl   = Control;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

H2O_LTDE_PANEL_ITEM *
GetNextSelectablePanelItem (
  IN H2O_LTDE_PANEL                           *Panel,
  IN H2O_LTDE_PANEL_ITEM                      *CurrentPanelItem,
  IN BOOLEAN                                  IsLoop
  )
{
  UINT32                                      Index;
  H2O_LTDE_PANEL_ITEM                         *PanelItem;
  H2O_LTDE_PANEL_ITEM                         *NextControl;
  H2O_LTDE_PANEL_ITEM                         *FirstControl;

  if (Panel == NULL) {
    return NULL;
  }

  NextControl  = NULL;
  FirstControl = NULL;

  for (Index = 0; Index < Panel->ItemCount; Index++) {
    PanelItem = &Panel->ItemList[Index];

    if (PanelItem->Hidden || !PanelItem->Selectable || PanelItem == CurrentPanelItem) {
      continue;
    }

    if (CurrentPanelItem == NULL || IsLoop) {
      if (FirstControl == NULL) {
        FirstControl = PanelItem;
      } else if ((PanelItem->ItemField.top  < FirstControl->ItemField.top) ||
                 (PanelItem->ItemField.top == FirstControl->ItemField.top &&
                  PanelItem->ItemField.left < FirstControl->ItemField.left)) {
        FirstControl = PanelItem;
      }

      if (CurrentPanelItem == NULL) {
        continue;
      }
    }

    if ((PanelItem->ItemField.top  < CurrentPanelItem->ItemField.top) ||
        (PanelItem->ItemField.top == CurrentPanelItem->ItemField.top &&
         PanelItem->ItemField.left < CurrentPanelItem->ItemField.left)) {
      continue;
    }

    if (NextControl == NULL) {
      NextControl = PanelItem;
      continue;
    }

    if ((PanelItem->ItemField.top  < NextControl->ItemField.top) ||
        (PanelItem->ItemField.top == NextControl->ItemField.top &&
         PanelItem->ItemField.left < NextControl->ItemField.left)) {
      NextControl = PanelItem;
    }
  }

  if (CurrentPanelItem == NULL || (NextControl == NULL && IsLoop)) {
    NextControl = FirstControl;
  }

  return NextControl;
}

H2O_LTDE_CONTROL *
GetNextSelectableControl (
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem,
  IN H2O_LTDE_CONTROL                         *CurrentControl,
  IN BOOLEAN                                  IsLoop
  )
{
  UINT32                                      Index;
  H2O_LTDE_CONTROL                            *FirstControl;
  H2O_LTDE_CONTROL                            *Control;
  BOOLEAN                                     IsCurrentControlFound;

  if (PanelItem == NULL || PanelItem->ControlList == NULL) {
    return NULL;
  }

  FirstControl          = NULL;
  IsCurrentControlFound = (CurrentControl == NULL) ? TRUE : FALSE;

  for (Index = 0; Index < PanelItem->ControlCount; Index++) {
    Control = &PanelItem->ControlList[Index];

    if (!Control->Selectable) {
      continue;
    }

    if (IsLoop && FirstControl == NULL) {
      FirstControl = Control;
    }

    if (!IsCurrentControlFound) {
      if (Control == CurrentControl) {
        IsCurrentControlFound = TRUE;
      }
      continue;
    }

    return Control;
  }

  if (IsLoop && FirstControl != NULL && FirstControl != CurrentControl) {
    return FirstControl;
  }

  return NULL;
}

H2O_LTDE_CONTROL *
GetNextSelectableControlByCurrentPos (
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem,
  IN INT32                                    CurrentPosition,
  IN BOOLEAN                                  IsLoop
  )
{
  UINT32                                      Index;
  H2O_LTDE_CONTROL                            *FirstControl;
  H2O_LTDE_CONTROL                            *Control;
  BOOLEAN                                     IsCurrentControlFound;
  INT32                                       Start;
  INT32                                       End;

  if (PanelItem == NULL || PanelItem->ControlList == NULL) {
    return NULL;
  }

  FirstControl          = NULL;
  IsCurrentControlFound = FALSE;

  for (Index = 0; Index < PanelItem->ControlCount; Index++) {
    Control = &PanelItem->ControlList[Index];

    if (!IsCurrentControlFound) {
      Start = PanelItem->Vertical ? Control->ControlField.top    : Control->ControlField.left;
      End   = PanelItem->Vertical ? Control->ControlField.bottom : Control->ControlField.right;
      if (IN_RANGE (CurrentPosition, Start, End)) {
        IsCurrentControlFound = TRUE;
        continue;
      }
    }

    if (!Control->Selectable) {
      continue;
    }

    if (IsLoop && FirstControl == NULL) {
      FirstControl = Control;
    }

    if (IsCurrentControlFound) {
      return Control;
    }
  }

  if (IsLoop && FirstControl != NULL) {
    Start = PanelItem->Vertical ? FirstControl->ControlField.top    : FirstControl->ControlField.left;
    End   = PanelItem->Vertical ? FirstControl->ControlField.bottom : FirstControl->ControlField.right;
    if (!IN_RANGE (CurrentPosition, Start, End)) {
      return FirstControl;
    }
  }

  return NULL;
}

H2O_LTDE_CONTROL *
GetPreviousSelectableControl (
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem,
  IN H2O_LTDE_CONTROL                         *CurrentControl,
  IN BOOLEAN                                  IsLoop
  )
{
  UINT32                                      Index;
  H2O_LTDE_CONTROL                            *PreviousControl;
  H2O_LTDE_CONTROL                            *Control;

  if (PanelItem == NULL || PanelItem->ControlList == NULL) {
    return NULL;
  }

  PreviousControl = NULL;

  for (Index = 0; Index < PanelItem->ControlCount; Index++) {
    Control = &PanelItem->ControlList[Index];

    if (!Control->Selectable) {
      continue;
    }

    if (Control == CurrentControl) {
      if (PreviousControl != NULL || !IsLoop) {
        break;
      }
      continue;
    }

    PreviousControl = Control;
  }

  return PreviousControl;
}

H2O_LTDE_CONTROL *
GetPreviousSelectableControlByCurrentPos (
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem,
  IN INT32                                    CurrentPosition,
  IN BOOLEAN                                  IsLoop
  )
{
  UINT32                                      Index;
  H2O_LTDE_CONTROL                            *PreviousControl;
  H2O_LTDE_CONTROL                            *Control;
  INT32                                       Start;
  INT32                                       End;

  if (PanelItem == NULL || PanelItem->ControlList == NULL) {
    return NULL;
  }

  PreviousControl = NULL;

  for (Index = 0; Index < PanelItem->ControlCount; Index++) {
    Control = &PanelItem->ControlList[Index];

    Start = PanelItem->Vertical ? Control->ControlField.top    : Control->ControlField.left;
    End   = PanelItem->Vertical ? Control->ControlField.bottom : Control->ControlField.right;
    if (IN_RANGE (CurrentPosition, Start, End)) {
      if (PreviousControl != NULL || !IsLoop) {
        break;
      }
      continue;
    }

    if (!Control->Selectable) {
      continue;
    }

    PreviousControl = Control;
  }

  return PreviousControl;
}


BOOLEAN
UpdatePanelContentItemPos (
  IN H2O_LTDE_PANEL                           *Panel
  )
{
  BOOLEAN                                     Updated;
  INT32                                       EndPos;
  INT32                                       ContentItemHeight;
  H2O_LTDE_PANEL_ITEM                         *ContentItem;
  H2O_LTDE_PANEL_ITEM                         *PanelItem;
  H2O_LTDE_CONTROL                            *SelectedControl;

  Updated = FALSE;

  if (Panel == NULL || Panel->SelectedControl == NULL) {
    return Updated;
  }

  ContentItem = GetPanelItem (Panel, LTDE_PANEL_ITEM_ID_CONTENT);
  if (ContentItem == NULL) {
    return Updated;
  }

  if (ContentItem != GetPanelItemByControl (Panel, Panel->SelectedControl)) {
    return Updated;
  }

  ContentItemHeight = H2O_LTDE_FIELD_HEIGHT (&ContentItem->ItemField);
  EndPos            = ContentItem->CurrentPos + ContentItemHeight - 1;
  SelectedControl   = Panel->SelectedControl;

  if ((!IN_RANGE (SelectedControl->ControlField.top   , ContentItem->CurrentPos, EndPos) ||
       !IN_RANGE (SelectedControl->ControlField.bottom, ContentItem->CurrentPos, EndPos))) {
    if (SelectedControl->ControlField.top < ContentItem->CurrentPos) {
      ContentItem->CurrentPos = SelectedControl->ControlField.top;
    } else {
      ContentItem->CurrentPos = SelectedControl->ControlField.bottom - ContentItemHeight + 1;
    }

    PanelItem = GetPanelItem (Panel, LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_UP);
    if (PanelItem != NULL) {
      PanelItem->Hidden = (ContentItem->CurrentPos == 0) ? TRUE : FALSE;
    }

    PanelItem = GetPanelItem (Panel, LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_DOWN);
    if (PanelItem != NULL) {
      PanelItem->Hidden = (ContentItem->CurrentPos + ContentItemHeight - 1 >= ContentItem->MaxPos) ? TRUE : FALSE;
    }

    PanelItem = GetPanelItem (Panel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP);
    if (PanelItem != NULL) {
      PanelItem->Hidden = (ContentItem->CurrentPos == 0) ? TRUE : FALSE;
    }

    PanelItem = GetPanelItem (Panel, LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN);
    if (PanelItem != NULL) {
      PanelItem->Hidden = (ContentItem->CurrentPos + ContentItemHeight - 1 >= ContentItem->MaxPos) ? TRUE : FALSE;
    }

    Updated = TRUE;
  }

  return Updated;
}


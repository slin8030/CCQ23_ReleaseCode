/** @file
  Display related functions for H2O display engine driver.
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

#include "H2ODisplayEngineLocalText.h"
#include "LTDESupport.h"
#include "LTDEControl.h"
#include "LTDEPanels.h"
#include "LTDEPrint.h"

#define BORDER_LINE_CHAR_ADD_DOWN             BIT0
#define BORDER_LINE_CHAR_ADD_UP               BIT1
#define BORDER_LINE_CHAR_ADD_RIGHT            BIT2
#define BORDER_LINE_CHAR_ADD_LEFT             BIT3

UINT32                                        mScreenMaxY = (UINT32) -1;
UINT32                                        mScreenMaxX = (UINT32) -1;

/**
 Create value in UINT64 type.

 @param [in] TargetHiiValue     HII value which set type as UINT64
 @param [in] ValueUint64        Value for UINT64 type

**/
EFI_STATUS
CreateValueAsUint64 (
  IN EFI_HII_VALUE  *TargetHiiValue,
  IN UINT64         ValueUint64
  )
{
  if (TargetHiiValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TargetHiiValue->Type      = EFI_IFR_TYPE_NUM_SIZE_64;
  TargetHiiValue->Value.u64 = ValueUint64;

  return EFI_SUCCESS;
}

/**
 Create value in string type.

 @param [in] TargetHiiValue        HII value which set type as string
 @param [in] Buffer                String buffer

**/
EFI_STATUS
CreateValueAsString (
  IN EFI_HII_VALUE   *TargetHiiValue,
  IN UINT16          BufferLen,
  IN UINT8           *Buffer
  )
{
  if (TargetHiiValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TargetHiiValue->Type      = EFI_IFR_TYPE_STRING;
  TargetHiiValue->BufferLen = BufferLen;
  TargetHiiValue->Buffer    = Buffer;

  return EFI_SUCCESS;
}

EFI_STATUS
CompareHiiValue (
  IN  EFI_HII_VALUE                        *Value1,
  IN  EFI_HII_VALUE                        *Value2,
  OUT INTN                                 *Result
  )
{
  INT64   Temp64;
  UINTN   Len;

  if (Value1->Type >= EFI_IFR_TYPE_OTHER || Value2->Type >= EFI_IFR_TYPE_OTHER ) {
    if (Value1->Type != EFI_IFR_TYPE_BUFFER && Value2->Type != EFI_IFR_TYPE_BUFFER) {
      return EFI_UNSUPPORTED;
    }
  }

  if (Value1->Type == EFI_IFR_TYPE_STRING || Value2->Type == EFI_IFR_TYPE_STRING ) {
    if (Value1->Type != Value2->Type) {
      //
      // Both Operator should be type of String
      //
      return EFI_UNSUPPORTED;
    }

    if (Value1->Value.string == 0 || Value2->Value.string == 0) {
      //
      // StringId 0 is reserved
      //
      return EFI_INVALID_PARAMETER;
    }

    if (Value1->Value.string == Value2->Value.string) {
      *Result = 0;
      return EFI_SUCCESS;
    }

    *Result = 1;
    return EFI_SUCCESS;
  }

  if (Value1->Type == EFI_IFR_TYPE_BUFFER || Value2->Type == EFI_IFR_TYPE_BUFFER) {
    if (Value1->Type != Value2->Type) {
      //
      // Both Operator should be type of Buffer.
      //
      return EFI_UNSUPPORTED;
    }
    Len = Value1->BufferLen > Value2->BufferLen ? Value2->BufferLen : Value1->BufferLen;
    *Result = CompareMem (Value1->Buffer, Value2->Buffer, Len);
    if ((*Result == 0) && (Value1->BufferLen != Value2->BufferLen)) {
      //
      // In this case, means base on samll number buffer, the data is same
      // So which value has more data, which value is bigger.
      //
      *Result = Value1->BufferLen > Value2->BufferLen ? 1 : -1;
    }
    return EFI_SUCCESS;
  }

  //
  // Take remain types(integer, boolean, date/time) as integer
  //
  Temp64 = (INT64) (Value1->Value.u64 - Value2->Value.u64);
  if (Temp64 > 0) {
    *Result = 1;
  } else if (Temp64 < 0) {
    *Result = -1;
  } else {
    *Result = 0;
  }

  return EFI_SUCCESS;
}

/**
 Display last char on the screen by specific color which defined in panel info

 @param[in] SimpleTextOut        The pointer of simple text out protocol for sepcific console device

 @retval EFI_SUCCESS             Success to display last char on the screen or the position of last char is not
                                 defined in any panel info
 @retval EFI_INVALID_PARAMETER   Input parameter is NULL
**/
STATIC
EFI_STATUS
DisplayScreenLastChar (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL             *SimpleTextOut
  )
{
  EFI_STATUS                                     Status;
  LIST_ENTRY                                     *Link;
  H2O_LTDE_PANEL                                 *Panel;
  H2O_LTDE_PANEL                                 *TargetPanel;
  INT32                                          ScreenLastCharXValue;
  INT32                                          ScreenLastCharYValue;
  UINT32                                         PanelColorAttribute;
  INT32                                          OrgAttribute;
  INT32                                          OrgCursorColumn;
  INT32                                          OrgCursorRow;

  if (SimpleTextOut == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = DEConOutQueryModeWithoutModeNumer ((UINT32 *) &ScreenLastCharXValue, (UINT32 *) &ScreenLastCharYValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ScreenLastCharXValue--;
  ScreenLastCharYValue--;

  TargetPanel = NULL;
  Link = GetFirstNode (&mDEPrivate->PanelListHead);
  while (!IsNull (&mDEPrivate->PanelListHead, Link)) {
    Panel = H2O_LTDE_PANEL_FROM_LINK (Link);

    if (Panel != NULL &&
        Panel->Visible &&
        Panel->PanelField.right  == ScreenLastCharXValue &&
        Panel->PanelField.bottom == ScreenLastCharYValue) {
      TargetPanel = Panel;
      break;
    }

    Link = GetNextNode (&mDEPrivate->PanelListHead, Link);
  }
  if (TargetPanel == NULL) {
    return EFI_SUCCESS;
  }

  GetPanelColorAttribute (TargetPanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL, &PanelColorAttribute);
  OrgAttribute    = SimpleTextOut->Mode->Attribute;
  OrgCursorColumn = SimpleTextOut->Mode->CursorColumn;
  OrgCursorRow    = SimpleTextOut->Mode->CursorRow;

  SimpleTextOut->SetCursorPosition (SimpleTextOut, ScreenLastCharXValue, ScreenLastCharYValue);
  SimpleTextOut->SetAttribute (SimpleTextOut, PanelColorAttribute);
  SimpleTextOut->OutputString (SimpleTextOut, L" ");
  SimpleTextOut->SetAttribute (SimpleTextOut, OrgAttribute);
  SimpleTextOut->SetCursorPosition (SimpleTextOut, OrgCursorColumn, OrgCursorRow);

  return EFI_SUCCESS;
}

/**
 Add up line on border line char

 @param[in, out] BorderLineChar  Pointer to border line char
**/
STATIC
VOID
BorderLineCharAddUp (
  IN OUT CHAR16                                  *BorderLineChar
  )
{
  if (BorderLineChar == NULL) {
    return;
  }

  switch (*BorderLineChar) {

  case BOXDRAW_HORIZONTAL:
    *BorderLineChar = BOXDRAW_UP_HORIZONTAL;
    break;

  case BOXDRAW_DOWN_RIGHT:
    *BorderLineChar = BOXDRAW_VERTICAL_RIGHT;
    break;

  case BOXDRAW_DOWN_LEFT:
    *BorderLineChar = BOXDRAW_VERTICAL_LEFT;
    break;

  case BOXDRAW_DOWN_HORIZONTAL:
    *BorderLineChar = BOXDRAW_VERTICAL_HORIZONTAL;
    break;
  }
}

/**
 Add down line on border line char

 @param[in, out] BorderLineChar  Pointer to border line char
**/
STATIC
VOID
BorderLineCharAddDown (
  IN OUT CHAR16                                  *BorderLineChar
  )
{
  if (BorderLineChar == NULL) {
    return;
  }

  switch (*BorderLineChar) {

  case BOXDRAW_HORIZONTAL:
    *BorderLineChar = BOXDRAW_DOWN_HORIZONTAL;
    break;

  case BOXDRAW_UP_RIGHT:
    *BorderLineChar = BOXDRAW_VERTICAL_RIGHT;
    break;

  case BOXDRAW_UP_LEFT:
    *BorderLineChar = BOXDRAW_VERTICAL_LEFT;
    break;

  case BOXDRAW_UP_HORIZONTAL:
    *BorderLineChar = BOXDRAW_VERTICAL_HORIZONTAL;
    break;
  }
}

/**
 Add left line on border line char

 @param[in, out] BorderLineChar  Pointer to border line char
**/
STATIC
VOID
BorderLineCharAddLeft (
  IN OUT CHAR16                                  *BorderLineChar
  )
{
  if (BorderLineChar == NULL) {
    return;
  }

  switch (*BorderLineChar) {

  case BOXDRAW_VERTICAL:
    *BorderLineChar = BOXDRAW_VERTICAL_LEFT;
    break;

  case BOXDRAW_UP_RIGHT:
    *BorderLineChar = BOXDRAW_UP_HORIZONTAL;
    break;

  case BOXDRAW_DOWN_RIGHT:
    *BorderLineChar = BOXDRAW_DOWN_HORIZONTAL;
    break;

  case BOXDRAW_VERTICAL_RIGHT:
    *BorderLineChar = BOXDRAW_VERTICAL_HORIZONTAL;
    break;
  }
}

/**
 Add right line on border line char

 @param[in, out] BorderLineChar  Pointer to border line char
**/
STATIC
VOID
BorderLineCharAddRight (
  IN OUT CHAR16                                  *BorderLineChar
  )
{
  if (BorderLineChar == NULL) {
    return;
  }

  switch (*BorderLineChar) {

  case BOXDRAW_VERTICAL:
    *BorderLineChar = BOXDRAW_VERTICAL_RIGHT;
    break;

  case BOXDRAW_UP_LEFT:
    *BorderLineChar = BOXDRAW_UP_HORIZONTAL;
    break;

  case BOXDRAW_DOWN_LEFT:
    *BorderLineChar = BOXDRAW_DOWN_HORIZONTAL;
    break;

  case BOXDRAW_VERTICAL_LEFT:
    *BorderLineChar = BOXDRAW_VERTICAL_HORIZONTAL;
    break;
  }
}

/**
 Update border line char by action

 @param[in]      BorderLineCharAction  Add action of border line char
 @param[in, out] BorderLineChar        Pointer to border line char
**/
STATIC
VOID
UpdateBorderLineChar (
  IN     UINT32                                  BorderLineCharAction,
  IN OUT CHAR16                                  *BorderLineChar
  )
{
  if ((BorderLineCharAction & BORDER_LINE_CHAR_ADD_DOWN) != 0) {
    BorderLineCharAddDown (BorderLineChar);
  }
  if ((BorderLineCharAction & BORDER_LINE_CHAR_ADD_UP) != 0) {
    BorderLineCharAddUp (BorderLineChar);
  }
  if ((BorderLineCharAction & BORDER_LINE_CHAR_ADD_RIGHT) != 0) {
    BorderLineCharAddRight (BorderLineChar);
  }
  if ((BorderLineCharAction & BORDER_LINE_CHAR_ADD_LEFT) != 0) {
    BorderLineCharAddLeft (BorderLineChar);
  }
}

/**
 Check if two fields are overlap.

 @param[in] Field1        Pointer to first field
 @param[in] Field2        Pointer to second field

 @retval TRUE             Two fields are overlap
 @retval FALSE            Two fields are not overlap
**/
STATIC
BOOLEAN
IsFieldOverlap (
  IN RECT                                        *Field1,
  IN RECT                                        *Field2
  )
{
  if (((Field1->right  == Field2->left  ) && (IS_OVERLAP (Field1->top , Field1->bottom, Field2->top , Field2->bottom))) ||
      ((Field1->left   == Field2->right ) && (IS_OVERLAP (Field1->top , Field1->bottom, Field2->top , Field2->bottom))) ||
      ((Field1->bottom == Field2->top   ) && (IS_OVERLAP (Field1->left, Field1->right , Field2->left, Field2->right ))) ||
      ((Field1->top    == Field2->bottom) && (IS_OVERLAP (Field1->left, Field1->right , Field2->left, Field2->right )))) {
    return TRUE;
  }

  return FALSE;
}

/**
 Update border line strings

 @param[in]      CurrentPanelField     Current panel field
 @param[in]      AdjacentPanelField    Adjacent panel field
 @param[in]      HorizonalLineWidth    Horizonal line width
 @param[in, out] TopHorizonalLine      Top horizonal border line string
 @param[in, out] BorderLineChar        Bottom horizonal border line string
 @param[in, out] BorderLineChar        Left vertical border line string
 @param[in, out] BorderLineChar        Right vertical border line string
**/
STATIC
EFI_STATUS
UpdateBorderLineStr (
  IN     RECT                                    *CurrentPanelField,
  IN     RECT                                    *AdjacentPanelField,
  IN     UINT32                                  HorizonalLineWidth,
  IN OUT CHAR16                                  *TopHorizonalLine,
  IN OUT CHAR16                                  *BottomHorizonalLine,
  IN OUT CHAR16                                  *LeftVerticalLine,
  IN OUT CHAR16                                  *RightVerticalLine
  )
{
  if (CurrentPanelField->right == AdjacentPanelField->left) {
    if (IN_RANGE (CurrentPanelField->top, AdjacentPanelField->top, AdjacentPanelField->bottom)) {
      if (CurrentPanelField->top == AdjacentPanelField->top) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_DOWN | BORDER_LINE_CHAR_ADD_RIGHT, &TopHorizonalLine[HorizonalLineWidth - 1]);
      } else if (CurrentPanelField->top == AdjacentPanelField->bottom) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_UP | BORDER_LINE_CHAR_ADD_RIGHT, &TopHorizonalLine[HorizonalLineWidth - 1]);
      } else {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_UP | BORDER_LINE_CHAR_ADD_DOWN, &TopHorizonalLine[HorizonalLineWidth - 1]);
      }
    } else {
      if (CurrentPanelField->bottom > AdjacentPanelField->top) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_RIGHT, &RightVerticalLine[AdjacentPanelField->top - CurrentPanelField->top - 1]);
      }
    }

    if (IN_RANGE (CurrentPanelField->bottom, AdjacentPanelField->top, AdjacentPanelField->bottom)) {
      if (CurrentPanelField->bottom == AdjacentPanelField->top) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_DOWN | BORDER_LINE_CHAR_ADD_RIGHT, &BottomHorizonalLine[HorizonalLineWidth - 1]);
      } else if (CurrentPanelField->bottom == AdjacentPanelField->bottom) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_UP | BORDER_LINE_CHAR_ADD_RIGHT, &BottomHorizonalLine[HorizonalLineWidth - 1]);
      } else {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_UP | BORDER_LINE_CHAR_ADD_DOWN, &BottomHorizonalLine[HorizonalLineWidth - 1]);
      }
    } else {
      if (CurrentPanelField->top < AdjacentPanelField->bottom) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_RIGHT, &RightVerticalLine[AdjacentPanelField->bottom - CurrentPanelField->top - 1]);
      }
    }

    return EFI_SUCCESS;
  }

  if (CurrentPanelField->left == AdjacentPanelField->right) {
    if (IN_RANGE (CurrentPanelField->top, AdjacentPanelField->top, AdjacentPanelField->bottom)) {
      if (CurrentPanelField->top == AdjacentPanelField->top) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_DOWN | BORDER_LINE_CHAR_ADD_LEFT, &TopHorizonalLine[0]);
      } else if (CurrentPanelField->top == AdjacentPanelField->bottom) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_UP | BORDER_LINE_CHAR_ADD_LEFT, &TopHorizonalLine[0]);
      } else {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_UP | BORDER_LINE_CHAR_ADD_DOWN, &TopHorizonalLine[0]);
      }
    } else {
      if (CurrentPanelField->bottom > AdjacentPanelField->top) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_LEFT, &LeftVerticalLine[AdjacentPanelField->top - CurrentPanelField->top - 1]);
      }
    }

    if (IN_RANGE (CurrentPanelField->bottom, AdjacentPanelField->top, AdjacentPanelField->bottom)) {
      if (CurrentPanelField->bottom == AdjacentPanelField->top) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_DOWN | BORDER_LINE_CHAR_ADD_LEFT, &BottomHorizonalLine[0]);
      } else if (CurrentPanelField->bottom == AdjacentPanelField->bottom) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_UP | BORDER_LINE_CHAR_ADD_LEFT, &BottomHorizonalLine[0]);
      } else {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_UP | BORDER_LINE_CHAR_ADD_DOWN, &BottomHorizonalLine[0]);
      }
    } else {
      if (CurrentPanelField->top < AdjacentPanelField->bottom) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_LEFT, &LeftVerticalLine[AdjacentPanelField->bottom - CurrentPanelField->top - 1]);
      }
    }

    return EFI_SUCCESS;
  }

  if (CurrentPanelField->bottom == AdjacentPanelField->top) {
    if (IN_RANGE (CurrentPanelField->left, AdjacentPanelField->left, AdjacentPanelField->right)) {
      if (CurrentPanelField->left == AdjacentPanelField->left) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_RIGHT | BORDER_LINE_CHAR_ADD_DOWN, &BottomHorizonalLine[0]);
      } else if (CurrentPanelField->left == AdjacentPanelField->right) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_LEFT | BORDER_LINE_CHAR_ADD_DOWN, &BottomHorizonalLine[0]);
      } else {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_RIGHT | BORDER_LINE_CHAR_ADD_LEFT, &BottomHorizonalLine[0]);
      }
    } else {
      if (CurrentPanelField->right > AdjacentPanelField->left) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_DOWN, &BottomHorizonalLine[AdjacentPanelField->left - CurrentPanelField->left]);
      }
    }

    if (IN_RANGE (CurrentPanelField->right, AdjacentPanelField->left, AdjacentPanelField->right)) {
      if (CurrentPanelField->right == AdjacentPanelField->left) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_RIGHT | BORDER_LINE_CHAR_ADD_DOWN, &BottomHorizonalLine[HorizonalLineWidth - 1]);
      } else if (CurrentPanelField->right == AdjacentPanelField->right) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_LEFT | BORDER_LINE_CHAR_ADD_DOWN, &BottomHorizonalLine[HorizonalLineWidth - 1]);
      } else {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_RIGHT | BORDER_LINE_CHAR_ADD_LEFT, &BottomHorizonalLine[HorizonalLineWidth - 1]);
      }
    } else {
      if (CurrentPanelField->left < AdjacentPanelField->right) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_DOWN, &BottomHorizonalLine[AdjacentPanelField->right - CurrentPanelField->left]);
      }
    }

    return EFI_SUCCESS;
  }

  if (CurrentPanelField->top == AdjacentPanelField->bottom) {
    if (IN_RANGE (CurrentPanelField->left, AdjacentPanelField->left, AdjacentPanelField->right)) {
      if (CurrentPanelField->left == AdjacentPanelField->left) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_RIGHT | BORDER_LINE_CHAR_ADD_UP, &TopHorizonalLine[0]);
      } else if (CurrentPanelField->left == AdjacentPanelField->right) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_LEFT | BORDER_LINE_CHAR_ADD_UP, &TopHorizonalLine[0]);
      } else {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_RIGHT | BORDER_LINE_CHAR_ADD_LEFT, &TopHorizonalLine[0]);
      }
    } else {
      if (CurrentPanelField->right > AdjacentPanelField->left) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_UP, &TopHorizonalLine[AdjacentPanelField->left - CurrentPanelField->left]);
      }
    }

    if (IN_RANGE (CurrentPanelField->right, AdjacentPanelField->left, AdjacentPanelField->right)) {
      if (CurrentPanelField->right == AdjacentPanelField->left) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_RIGHT | BORDER_LINE_CHAR_ADD_UP, &TopHorizonalLine[HorizonalLineWidth - 1]);
      } else if (CurrentPanelField->right == AdjacentPanelField->right) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_LEFT | BORDER_LINE_CHAR_ADD_UP, &TopHorizonalLine[HorizonalLineWidth - 1]);
      } else {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_RIGHT | BORDER_LINE_CHAR_ADD_LEFT, &TopHorizonalLine[HorizonalLineWidth - 1]);
      }
    } else {
      if (CurrentPanelField->left < AdjacentPanelField->right) {
        UpdateBorderLineChar (BORDER_LINE_CHAR_ADD_UP, &TopHorizonalLine[AdjacentPanelField->right - CurrentPanelField->left]);
      }
    }

    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;;
}

EFI_STATUS
GetAllBorderLineField (
  OUT RECT                                       **BorderLineFieldList,
  OUT UINT32                                     *BorderLineFieldCount
  )
{
  LIST_ENTRY                                     *Link;
  H2O_LTDE_PANEL                                 *Panel;
  UINT32                                         FieldCount;
  RECT                                           *FieldList;

  if (BorderLineFieldList == NULL || BorderLineFieldCount == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FieldCount = 0;
  Link  = GetFirstNode (&mDEPrivate->PanelListHead);
  while (!IsNull (&mDEPrivate->PanelListHead, Link)) {
    Panel = H2O_LTDE_PANEL_FROM_LINK (Link);
    Link  = GetNextNode (&mDEPrivate->PanelListHead, Link);
    if (Panel == NULL || Panel->BorderLineWidth == 0 || !Panel->Visible) {
      continue;
    }
    FieldCount++;
  }
  if (FieldCount == 0) {
    return EFI_NOT_FOUND;
  }

  FieldList = AllocateZeroPool (FieldCount * sizeof (RECT));
  if (FieldList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  FieldCount = 0;
  Link       = GetFirstNode (&mDEPrivate->PanelListHead);
  while (!IsNull (&mDEPrivate->PanelListHead, Link)) {
    Panel = H2O_LTDE_PANEL_FROM_LINK (Link);
    Link  = GetNextNode (&mDEPrivate->PanelListHead, Link);
    if (Panel == NULL || Panel->BorderLineWidth == 0 || !Panel->Visible) {
      continue;
    }
    CopyRect (&FieldList[FieldCount], &Panel->PanelField);
    FieldCount++;
  }

  *BorderLineFieldList  = FieldList;
  *BorderLineFieldCount = FieldCount;

  return EFI_SUCCESS;
}

/**
 Print border line for current panel field.
 It will also deal with the border line char which is overlapped with previous panel field.

 @param[in] CurrentPanelField          Current panel field
 @param[in] PreviousPanelField         Previous panel field list
 @param[in] PreviousPanelFieldCount    Previous panel field list count
**/
EFI_STATUS
GetBorderLineInfo (
  IN  RECT                                       *CurrentPanelField,
  IN  RECT                                       *BorderLineFieldList,
  IN  UINT32                                     BorderLineFieldCount,
  OUT LTDE_PANEL_BORDER_LINE_INFO                *BorderLineInfo
  )
{
  UINT32                                         Index;
  UINT32                                         HorizonalLineWidth;
  UINT32                                         VerticalLineHeight;
  CHAR16                                         *TopHorizonalLine;
  CHAR16                                         *BottomHorizonalLine;
  CHAR16                                         *LeftVerticalLine;
  CHAR16                                         *RightVerticalLine;
  EFI_STATUS                                     Status;

  if (CurrentPanelField == NULL || BorderLineInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize horizonal and vetical border line data for current field
  //
  HorizonalLineWidth = (UINT32) (CurrentPanelField->right  - CurrentPanelField->left + 1);
  VerticalLineHeight = (UINT32) (CurrentPanelField->bottom - CurrentPanelField->top  + 1) - 2;

  TopHorizonalLine    = AllocatePool ((HorizonalLineWidth + 1) * sizeof (CHAR16));
  BottomHorizonalLine = AllocatePool ((HorizonalLineWidth + 1) * sizeof (CHAR16));
  LeftVerticalLine    = AllocatePool (VerticalLineHeight * sizeof (CHAR16));
  RightVerticalLine   = AllocatePool (VerticalLineHeight * sizeof (CHAR16));
  if (TopHorizonalLine == NULL || BottomHorizonalLine == NULL || LeftVerticalLine == NULL || RightVerticalLine == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  TopHorizonalLine[0]                         = BOXDRAW_DOWN_RIGHT;
  TopHorizonalLine[HorizonalLineWidth - 1]    = BOXDRAW_DOWN_LEFT;
  BottomHorizonalLine[0]                      = BOXDRAW_UP_RIGHT;
  BottomHorizonalLine[HorizonalLineWidth - 1] = BOXDRAW_UP_LEFT;
  for (Index = 1; Index < HorizonalLineWidth - 1; Index++) {
    TopHorizonalLine[Index]    = BOXDRAW_HORIZONTAL;
    BottomHorizonalLine[Index] = BOXDRAW_HORIZONTAL;
  }
  TopHorizonalLine[HorizonalLineWidth]    = CHAR_NULL;
  BottomHorizonalLine[HorizonalLineWidth] = CHAR_NULL;

  for (Index = 0; Index < VerticalLineHeight; Index++) {
    LeftVerticalLine[Index]  = BOXDRAW_VERTICAL;
    RightVerticalLine[Index] = BOXDRAW_VERTICAL;
  }

  //
  // Update line data when current field cover to previous fields
  //
  if (BorderLineFieldList != NULL) {
    for (Index = 0; Index < BorderLineFieldCount; Index++) {
      if (!IsFieldOverlap (CurrentPanelField, &BorderLineFieldList[Index]) ||
          EqualRect(CurrentPanelField, &BorderLineFieldList[Index])) {
        continue;
      }
      Status = UpdateBorderLineStr (
                 CurrentPanelField,
                 &BorderLineFieldList[Index],
                 HorizonalLineWidth,
                 TopHorizonalLine,
                 BottomHorizonalLine,
                 LeftVerticalLine,
                 RightVerticalLine
                 );
      ASSERT (Status == EFI_SUCCESS);
    }
  }

  BorderLineInfo->HorizonalLineWidth  = HorizonalLineWidth;
  BorderLineInfo->VerticalLineHeight  = VerticalLineHeight;
  BorderLineInfo->TopHorizonalLine    = TopHorizonalLine;
  BorderLineInfo->BottomHorizonalLine = BottomHorizonalLine;
  BorderLineInfo->LeftVerticalLine    = LeftVerticalLine;
  BorderLineInfo->RightVerticalLine   = RightVerticalLine;

  return EFI_SUCCESS;
}

VOID
FreeBorderLineInfo (
  IN LTDE_PANEL_BORDER_LINE_INFO              *BorderLineInfo
  )
{
  if (BorderLineInfo != NULL) {
    SafeFreePool ((VOID **) &BorderLineInfo->TopHorizonalLine);
    SafeFreePool ((VOID **) &BorderLineInfo->BottomHorizonalLine);
    SafeFreePool ((VOID **) &BorderLineInfo->LeftVerticalLine);
    SafeFreePool ((VOID **) &BorderLineInfo->RightVerticalLine);
  }
}

/**
 Print border line for current panel field.
 It will also deal with the border line char which is overlapped with previous panel field.

 @param[in] CurrentPanelField          Current panel field
 @param[in] PreviousPanelField         Previous panel field list
 @param[in] PreviousPanelFieldCount    Previous panel field list count
**/
EFI_STATUS
PrintBorderLineFunc (
  IN RECT                                     *CurrentPanelField,
  IN LTDE_PANEL_BORDER_LINE_INFO              *BorderLineInfo     OPTIONAL
  )
{
  EFI_STATUS                                  Status;
  LTDE_PANEL_BORDER_LINE_INFO                 *BorderLineInfoPtr;
  LTDE_PANEL_BORDER_LINE_INFO                 BorderLineInfoInstance;
  UINT32                                      Index;
  CHAR16                                      Character[2];

  if (CurrentPanelField == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BorderLineInfo == NULL) {
    Status = GetBorderLineInfo (CurrentPanelField, NULL, 0, &BorderLineInfoInstance);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    BorderLineInfoPtr = &BorderLineInfoInstance;
  } else {
    BorderLineInfoPtr = BorderLineInfo;
  }

  //
  // Print border line of current field
  //
  DisplayString (CurrentPanelField->left, CurrentPanelField->bottom, BorderLineInfoPtr->BottomHorizonalLine);
  DisplayString (CurrentPanelField->left, CurrentPanelField->top   , BorderLineInfoPtr->TopHorizonalLine);

  Character[1] = CHAR_NULL;
  for (Index = 0; Index < BorderLineInfoPtr->VerticalLineHeight; Index++) {
    Character[0] = BorderLineInfoPtr->LeftVerticalLine[Index];
    DisplayString (CurrentPanelField->left , CurrentPanelField->top + 1 + Index, Character);

    Character[0] = BorderLineInfoPtr->RightVerticalLine[Index];
    DisplayString (CurrentPanelField->right, CurrentPanelField->top + 1 + Index, Character);
  }

  if (BorderLineInfoPtr != BorderLineInfo) {
    FreeBorderLineInfo (BorderLineInfoPtr);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ConDevInit (
  IN H2O_FORM_BROWSER_CONSOLE_DEV_NODE           *ConDevNode
  )
{
  H2O_LTDE_PANEL                                 *ScreenPanel;
  H2O_LTDE_PANEL                                 *SetupPagePanel;
  UINT32                                         Attribute;
  UINT32                                         HorizontalResolution;
  UINT32                                         VerticalResolution;
  UINTN                                          DeltaX;
  UINTN                                          DeltaY;
  UINT32                                         Interval;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL                  Background;
  UINTN                                          Column;
  UINTN                                          Row;

  SetupPagePanel = GetPanel (H2O_PANEL_TYPE_SETUP_PAGE);
  GetPanelColorAttribute (SetupPagePanel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL, &Attribute);
  ConDevNode->SimpleTextOut->SetAttribute (ConDevNode->SimpleTextOut, Attribute);
  ConDevNode->SimpleTextOut->ClearScreen (ConDevNode->SimpleTextOut);

  if (ConDevNode->GraphicsOut != NULL) {
    ScreenPanel = GetPanel (H2O_PANEL_TYPE_SCREEN);

    ConDevNode->SimpleTextOut->QueryMode (ConDevNode->SimpleTextOut, ConDevNode->SimpleTextOut->Mode->Mode, &Column, &Row);
    HorizontalResolution = ConDevNode->GraphicsOut->Mode->Info->HorizontalResolution;
    VerticalResolution   = ConDevNode->GraphicsOut->Mode->Info->VerticalResolution;

    DeltaX = (HorizontalResolution - Column * EFI_GLYPH_WIDTH) / 2;
    DeltaY = (VerticalResolution - Row * EFI_GLYPH_HEIGHT) / 2;
    ZeroMem (&Background, sizeof(Background));

    if (DeltaX != 0 || ScreenPanel->PanelField.left != 0) {
      Interval = ScreenPanel->PanelField.left * EFI_GLYPH_WIDTH;
      ConDevNode->GraphicsOut->Blt (
                                 ConDevNode->GraphicsOut,
                                 &Background,
                                 EfiBltVideoFill,
                                 0,
                                 0,
                                 0,
                                 0,
                                 DeltaX + Interval,
                                 VerticalResolution,
                                 0
                                 );
    }

    if (DeltaX != 0 || ScreenPanel->PanelField.right < (INT32) Column - 1) {
      Interval = (UINT32) ((INT32) Column - 1 - ScreenPanel->PanelField.right) * EFI_GLYPH_WIDTH;
      ConDevNode->GraphicsOut->Blt (
                                 ConDevNode->GraphicsOut,
                                 &Background,
                                 EfiBltVideoFill,
                                 HorizontalResolution - DeltaX - Interval,
                                 0,
                                 HorizontalResolution - DeltaX - Interval,
                                 0,
                                 DeltaX + Interval,
                                 VerticalResolution,
                                 0
                                 );
    }


    if (DeltaY != 0 || ScreenPanel->PanelField.top != 0) {
      Interval = ScreenPanel->PanelField.top * EFI_GLYPH_HEIGHT;
      ConDevNode->GraphicsOut->Blt (
                                 ConDevNode->GraphicsOut,
                                 &Background,
                                 EfiBltVideoFill,
                                 0,
                                 0,
                                 0,
                                 0,
                                 HorizontalResolution,
                                 DeltaY + Interval,
                                 0
                                 );
    }
    if (DeltaY != 0 || ScreenPanel->PanelField.bottom < (INT32) Row - 1) {
      Interval = (UINT32) ((INT32) Row - 1 - ScreenPanel->PanelField.bottom) * EFI_GLYPH_HEIGHT;
      ConDevNode->GraphicsOut->Blt (
                                 ConDevNode->GraphicsOut,
                                 &Background,
                                 EfiBltVideoFill,
                                 0,
                                 VerticalResolution - DeltaY - Interval,
                                 0,
                                 VerticalResolution - DeltaY - Interval,
                                 HorizontalResolution,
                                 DeltaY + Interval,
                                 0
                                 );
    }
  }

  DisplayScreenLastChar (ConDevNode->SimpleTextOut);

  return EFI_SUCCESS;
}

/**
 Display layout which includes screen last character and the border line for each panel.

 @retval EFI_SUCCESS                Display layout successfully
 @retval EFI_OUT_OF_RESOURCES       Allocate pool fail
**/
EFI_STATUS
DisplayLayout (
  VOID
  )
{
  UINT32                                         PanelColorAttribute;
  LIST_ENTRY                                     *Link;
  H2O_LTDE_PANEL                                 *Panel;
  RECT                                           *BorderLineFieldList;
  UINT32                                         BorderLineFieldCount;
  H2O_FORM_BROWSER_CONSOLE_DEV_NODE              *ConDevNode;
  EFI_STATUS                                     Status;
  LTDE_PANEL_BORDER_LINE_INFO                    BorderLineInfo;

  //
  // Display screen last character when console device is in initial status.
  //
  Link = GetFirstNode (&mDEPrivate->ConsoleDevListHead);
  while (!IsNull (&mDEPrivate->ConsoleDevListHead, Link)) {
    ConDevNode = H2O_FORM_BROWSER_CONSOLE_DEV_NODE_FROM_LINK (Link);

    if (ConDevNode != NULL && ConDevNode->ConDevStatus == CONSOLE_DEVICE_STATUS_INIT) {
      ConDevInit (ConDevNode);
      ConDevNode->ConDevStatus = CONSOLE_DEVICE_STATUS_READY;
    }

    Link = GetNextNode (&mDEPrivate->ConsoleDevListHead, Link);
  }

  //
  // Display border line for each panel
  //
  Status = GetAllBorderLineField (&BorderLineFieldList, &BorderLineFieldCount);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Link = GetFirstNode (&mDEPrivate->PanelListHead);
  while (!IsNull (&mDEPrivate->PanelListHead, Link)) {
    Panel = H2O_LTDE_PANEL_FROM_LINK (Link);
    Link  = GetNextNode (&mDEPrivate->PanelListHead, Link);

    if (Panel == NULL || Panel->BorderLineWidth == 0 || !Panel->Visible) {
      continue;
    }

    GetPanelColorAttribute (Panel->VfcfPanelInfo, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL, &PanelColorAttribute);
    DEConOutSetAttribute (PanelColorAttribute);
    Status = GetBorderLineInfo (&Panel->PanelField, BorderLineFieldList, BorderLineFieldCount, &BorderLineInfo);
    if (!EFI_ERROR (Status)) {
      PrintBorderLineFunc (&Panel->PanelField, &BorderLineInfo);
      FreeBorderLineInfo (&BorderLineInfo);
    }
  }

  FreePool ((VOID *) BorderLineFieldList);

  return EFI_SUCCESS;
}

/**
 Display string

 @param [in] StartX                Target x-axis of string
 @param [in] StartY                Target y-axis of string
 @param [in] Fmt                   Format string

 @retval EFI_SUCCESS               Display string successfully
 @retval Other                     Output dstring fail

**/
EFI_STATUS
DisplayString (
  IN UINT32                                   StartX,
  IN UINT32                                   StartY,
  IN CHAR16                                   *String
  )
{
  CHAR16                                      *ModifiedStr;
  UINTN                                       ModifiedStrLen;

  if (StartX > mScreenMaxX ||
      StartY > mScreenMaxY) {
    return EFI_INVALID_PARAMETER;
  }

  DEConOutSetCursorPosition (StartX, StartY);

  //
  // Prevent display string on screen right bottom character.
  //
  if (StartY == mScreenMaxY) {
    if (StartX == mScreenMaxX) {
      return EFI_SUCCESS;
    } else if (StartX + GetStringDisplayWidth (String) >= mScreenMaxX) {
      ModifiedStrLen = StrLen (String);
      ModifiedStr    = AllocateCopyPool ((ModifiedStrLen + 1) * sizeof (CHAR16), String);
      if (ModifiedStr == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      while (ModifiedStrLen > 0) {
        ModifiedStrLen--;
        ModifiedStr[ModifiedStrLen] = CHAR_NULL;

        if (StartX + GetStringDisplayWidth (ModifiedStr) < mScreenMaxX) {
          DEConOutOutputString (ModifiedStr);
          break;
        }
      }

      FreePool (ModifiedStr);
      return EFI_SUCCESS;
    }
  }

  return DEConOutOutputString (String);
}


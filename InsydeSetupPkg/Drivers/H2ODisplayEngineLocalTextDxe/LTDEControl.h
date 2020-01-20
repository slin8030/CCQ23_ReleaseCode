/** @file

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

#ifndef _LTDE_CONTROL_H_
#define _LTDE_CONTROL_H_

#include "H2ODisplayEngineLocalText.h"

typedef struct _H2O_LTDE_PANEL                H2O_LTDE_PANEL;
typedef struct _H2O_LTDE_PANEL_ITEM           H2O_LTDE_PANEL_ITEM;
typedef struct _H2O_LTDE_CONTROL              H2O_LTDE_CONTROL;

#define H2O_LTDE_FIELD_WIDTH(Field)           (((RECT *) (Field))->right  - ((RECT *) (Field))->left + 1)
#define H2O_LTDE_FIELD_HEIGHT(Field)          (((RECT *) (Field))->bottom - ((RECT *) (Field))->top  + 1)
#define IS_DISPLAYED_IN_HEX(Control)          ((BOOLEAN) ((Control->Flags & EFI_IFR_DISPLAY_UINT_HEX) != 0))

typedef enum {
  LTDE_CONTROL_ID_DIALOG_TITLE,
  LTDE_CONTROL_ID_DIALOG_BODY,
  LTDE_CONTROL_ID_DIALOG_BODY_INPUT,
  LTDE_CONTROL_ID_DIALOG_BUTTON,
  LTDE_CONTROL_ID_DIALOG_HOT_KEY_PAGE_UP,
  LTDE_CONTROL_ID_DIALOG_HOT_KEY_PAGE_DOWN,
  LTDE_CONTROL_ID_DIALOG_HOT_KEY_SCROLL_UP,
  LTDE_CONTROL_ID_DIALOG_HOT_KEY_SCROLL_DOWN,
  LTDE_CONTROL_ID_MAX
} LTDE_CONTROL_ID;

typedef enum {
  LTDE_PANEL_ITEM_ID_TITLE,
  LTDE_PANEL_ITEM_ID_CONTENT,
  LTDE_PANEL_ITEM_ID_CONTENT_PAGE_UP,
  LTDE_PANEL_ITEM_ID_CONTENT_PAGE_DOWN,
  LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_UP,
  LTDE_PANEL_ITEM_ID_CONTENT_SCROLL_DOWN,
  LTDE_PANEL_ITEM_ID_BUTTON,
  LTDE_PANEL_ITEM_ID_MAX
} LTDE_PANEL_ITEM_ID;

typedef struct _H2O_LTDE_CONTROL {
  UINT32                                      ControlId;
  H2O_STYLE_INFO                              ControlStyle;
  RECT                                        ControlField;
  H2O_STRING_INFO                             Text;
  H2O_STRING_INFO                             ValueStrInfo;
  EFI_HII_VALUE                               HiiValue;

  //
  // Statement
  //
  H2O_PAGE_ID                                 PageId;
  H2O_STATEMENT_ID                            StatementId;
  EFI_QUESTION_ID                             QuestionId;
  UINT8                                       Operand;
  EFI_IFR_OP_HEADER                           *IfrOpCode;
  BOOLEAN                                     Selectable;

  UINT64                                      Minimum;
  UINT64                                      Maximum;
  UINT64                                      Step;
  UINT8                                       Flags;   ///< Flag to determine display format for EFI_IFR_NUMERIC

  //
  // Display item sequence for date/time
  //  Date:      Month/Day/Year
  //  Sequence:  0     1   2
  //
  //  Time:      Hour : Minute : Second
  //  Sequence:  0      1        2
  //
  UINT8                                       Sequence;
} H2O_LTDE_CONTROL;

typedef struct _H2O_LTDE_PANEL_ITEM {
  UINT32                                      ItemId;
  RECT                                        ItemField;
  INT32                                       CurrentPos;
  INT32                                       MaxPos;
  BOOLEAN                                     Vertical;
  BOOLEAN                                     Hidden;
  BOOLEAN                                     Selectable;
  UINT32                                      ControlCount;
  H2O_LTDE_CONTROL                            *ControlList;
} H2O_LTDE_PANEL_ITEM;

#define H2O_LTDE_PANEL_SIGNATURE SIGNATURE_32 ('H', 'L', 'T', 'P')

typedef struct _H2O_LTDE_PANEL {
  UINT32                                      Signature;
  LIST_ENTRY                                  Link;
  H2O_PANEL_INFO                              *VfcfPanelInfo;
  //
  // Panel related data
  //
  RECT                                        PanelField;
  INT32                                       BorderLineWidth;
  BOOLEAN                                     Visible;
  BOOLEAN                                     Vertical;
  UINT32                                      ColorAttribute;
  UINT32                                      ItemCount;
  H2O_LTDE_PANEL_ITEM                         *ItemList;
  H2O_LTDE_CONTROL                            *SelectedControl;
} H2O_LTDE_PANEL;

#define H2O_LTDE_PANEL_FROM_LINK(a) CR (a, H2O_LTDE_PANEL, Link, H2O_LTDE_PANEL_SIGNATURE)

H2O_LTDE_CONTROL *
GetControlById (
  IN H2O_LTDE_PANEL                           *Panel,
  IN UINT32                                   ItemId,
  IN UINT32                                   ControlId
  );

H2O_LTDE_CONTROL *
FindControlByControlId (
  IN H2O_LTDE_CONTROL                         *ControlArray,
  IN UINT32                                   ControlArrayCount,
  IN UINT32                                   ControlId,
  IN UINT32                                   SequenceIndex
  );

H2O_LTDE_CONTROL *
GetControlByQuestionId (
  IN H2O_LTDE_CONTROL                         *ControlList,
  IN UINT32                                   ControlCount,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  );

EFI_STATUS
DisplaySetupPageControls (
  IN H2O_LTDE_PANEL                           *SetupPagePanel
  );

EFI_STATUS
ClearField (
  IN CONST UINT32                             Attribute,
  IN       RECT                               *Field
  );

EFI_STATUS
CalculateRequireSize (
  IN  CHAR16                                  *DisplayString,
  IN  UINT32                                  LimitLineWidth,
  OUT UINT32                                  *RequireWidth,
  OUT UINT32                                  *RequireHeight
  );

EFI_STATUS
DisplayString (
  IN UINT32                                   StartX,
  IN UINT32                                   StartY,
  IN CHAR16                                   *String
  );

UINT32
GetStringHeight (
  IN CHAR16                                   *String,
  IN UINT32                                   LineWidth
  );

EFI_STATUS
GetStringArrayByWidth (
  IN  CHAR16                                  *String,
  IN  UINT32                                  LineWidth,
  OUT UINT32                                  *StringArrayNum,
  OUT CHAR16                                  ***StringArray
  );

CHAR16 *
GetAlignmentString (
  IN CHAR16                                   *String,
  IN UINT32                                   LineWidth,
  IN UINT32                                   AlignmentAction
  );

VOID
FreeControlInfo (
  IN H2O_LTDE_CONTROL                         *Control
  );

VOID
FreePanelItemList (
  IN H2O_LTDE_PANEL_ITEM                      *ItemList,
  IN UINT32                                   ItemCount
  );

H2O_LTDE_PANEL *
CreatePanel (
  VOID
  );

VOID
FreePanel (
  IN H2O_LTDE_PANEL                           *Panel
  );

H2O_LTDE_PANEL *
GetPanel (
  IN UINT32                                   PanelType
  );

H2O_LTDE_PANEL_ITEM *
GetPanelItem (
  IN H2O_LTDE_PANEL                           *Panel,
  IN UINT32                                   ItemId
  );

H2O_LTDE_PANEL_ITEM *
GetPanelItemByControl (
  IN H2O_LTDE_PANEL                           *Panel,
  IN H2O_LTDE_CONTROL                         *Control
  );

EFI_STATUS
GetControlByMouse (
  IN  H2O_LTDE_PANEL                          *Panel,
  IN  INT32                                   MouseX,
  IN  INT32                                   MouseY,
  OUT H2O_LTDE_PANEL_ITEM                     **SelectedPanelItem,
  OUT H2O_LTDE_CONTROL                        **SelectedControl
  );

H2O_LTDE_PANEL_ITEM *
GetNextSelectablePanelItem (
  IN H2O_LTDE_PANEL                           *Panel,
  IN H2O_LTDE_PANEL_ITEM                      *CurrentPanelItem,
  IN BOOLEAN                                  IsLoop
  );

H2O_LTDE_CONTROL *
GetNextSelectableControl (
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem,
  IN H2O_LTDE_CONTROL                         *CurrentControl,
  IN BOOLEAN                                  IsLoop
  );

H2O_LTDE_CONTROL *
GetNextSelectableControlByCurrentPos (
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem,
  IN INT32                                    CurrentPosition,
  IN BOOLEAN                                  IsLoop
  );

H2O_LTDE_CONTROL *
GetPreviousSelectableControl (
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem,
  IN H2O_LTDE_CONTROL                         *CurrentControl,
  IN BOOLEAN                                  IsLoop
  );

H2O_LTDE_CONTROL *
GetPreviousSelectableControlByCurrentPos (
  IN H2O_LTDE_PANEL_ITEM                      *PanelItem,
  IN INT32                                    CurrentPosition,
  IN BOOLEAN                                  IsLoop
  );

BOOLEAN
UpdatePanelContentItemPos (
  IN H2O_LTDE_PANEL                           *Panel
  );

#endif

/** @file

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#ifndef _INSYDEMODULE_HII_H
#define _INSYDEMODULE_HII_H

///
/// GUIDed opcodes defined for H2O implementation.
///
#define H2O_IFR_EXT_GUID \
  { \
    0x38237648, 0x09cc, 0x47c4, {0x8b, 0x5f, 0xb0, 0x9f, 0x06, 0x89, 0x0d, 0xf7} \
  }

#pragma pack(1)

//
// For H2O_VALUE
//
#define H2O_VALUE_TYPE_UNDEFINED             0x0000
#define H2O_VALUE_TYPE_BOOLEAN               0x0001
#define H2O_VALUE_TYPE_UINT32                0x0002
#define H2O_VALUE_TYPE_INT32                 0x0003
#define H2O_VALUE_TYPE_PERCENTAGE            0x0004
#define H2O_VALUE_TYPE_UINT8                 0x0005
#define H2O_VALUE_TYPE_UINT16                0x0006
#define H2O_VALUE_TYPE_UINT64                0x0007
#define H2O_VALUE_TYPE_INT8                  0x0008
#define H2O_VALUE_TYPE_INT16                 0x0009
#define H2O_VALUE_TYPE_INT64                 0x000A
#define H2O_VALUE_TYPE_STR16                 0x000B
#define H2O_VALUE_TYPE_GUID                  0x000C
#define H2O_VALUE_TYPE_STRID                 0x000D
#define H2O_VALUE_TYPE_IMGID                 0x000E
#define H2O_VALUE_TYPE_BUFFER                0x000F
#define H2O_VALUE_TYPE_DATE                  0x0010
#define H2O_VALUE_TYPE_TIME                  0x0011

typedef struct {
  UINT32                                     BufferSize;
  UINT8                                      *Buffer;
} H2O_BUFFER;

typedef struct {
  UINT16                                     Year;
  UINT8                                      Month;
  UINT8                                      Day;
} H2O_DATE;

typedef struct {
  UINT8                                      Hour;
  UINT8                                      Minute;
  UINT8                                      Second;
} H2O_TIME;

typedef union {
  VOID                                       *Ptr;
  BOOLEAN                                    Bool;
  UINT8                                      Pct;
  CHAR8                                      C8;
  CHAR16                                     C16;
  UINT8                                      U8;
  UINT16                                     U16;
  UINT32                                     U32;
  UINT64                                     U64;
  INT8                                       I8;
  INT16                                      I16;
  INT32                                      I32;
  INT64                                      I64;
  CHAR8                                      *Str8;
  CHAR16                                     *Str16;
  EFI_GUID                                   Guid;
  H2O_DATE                                   Date;
  H2O_TIME                                   Time;
  H2O_BUFFER                                 Buffer;
} H2O_VALUE_TYPE_VALUE;

typedef struct {
  UINT32                                     Type;
  H2O_VALUE_TYPE_VALUE                       Value;
} H2O_VALUE;

//
// bugbug: For backward compatibility
//
typedef union {
  BOOLEAN                                    Bool;
  INT32                                      Int;
  UINT32                                     Uint;
} OLD_H2O_VALUE_TYPE_VALUE;

typedef struct {
  UINT32                                     Type;
  OLD_H2O_VALUE_TYPE_VALUE                   Value;
} OLD_H2O_VALUE;

///
/// H2O implementation extension opcodes, new extension can be added here later.
///
#define H2O_IFR_EXT_HOTKEY            0x0000
#define H2O_IFR_EXT_LAYOUT            0x0001
#define H2O_IFR_EXT_STYLE             0x0002
#define H2O_IFR_EXT_HELPIMAGE         0x0003
#define H2O_IFR_EXT_PROPERTY          0x0004
#define H2O_IFR_EXT_STYLE_REF         0x0005
#define H2O_IFR_EXT_TEXT              0x0006
#define H2O_IFR_EXT_LINK              0x0007
#define H2O_IFR_EXT_EXT_FLAGS         0x000C
#define H2O_IFR_EXT_INCLUDE_FORM_SET  0x000D
#define H2O_IFR_EXT_EXCLUDE_FORM_SET  0x000E

//
// Link definitions
//
typedef struct _H2O_IFR_GUID_LINK {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_LINK
  ///
  UINT16              Function;
  UINT32              IncludeItemCount;
  UINT32              ExcludeItemCount;
} H2O_IFR_GUID_LINK;

///
/// HotKey opcode.
///
typedef struct _H2O_IFR_GUID_HOTKEY {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_HOTKEY
  ///
  UINT16              Function;
  UINT8               HotKeyOffset;
} H2O_IFR_GUID_HOTKEY;

///
/// Layout opcode.
///
typedef struct _H2O_IFR_GUID_LAYOUT {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_LAYOUT
  ///
  UINT16              Function;
  UINT16              LayoutId;
} H2O_IFR_GUID_LAYOUT;

///
/// Style opcode.
///
typedef struct _H2O_IFR_GUID_STYLE {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_STYLE
  ///
  UINT16              Function;
  UINT8               Type;
  UINT8               ClassNameOffset;
  UINT8               PseudoClassOffset;
//CHAR8               Selector[1];
} H2O_IFR_GUID_STYLE;

///
/// Style type
///
#define H2O_IFR_STYLE_TYPE_LAYOUT      0x00
#define H2O_IFR_STYLE_TYPE_PANEL       0x01
#define H2O_IFR_STYLE_TYPE_FORM        0x02
#define H2O_IFR_STYLE_TYPE_FORMSET     0x03
#define H2O_IFR_STYLE_TYPE_ACTION      0x04
#define H2O_IFR_STYLE_TYPE_CHECKBOX    0x05
#define H2O_IFR_STYLE_TYPE_DATE        0x06
#define H2O_IFR_STYLE_TYPE_GOTO        0x07
#define H2O_IFR_STYLE_TYPE_ONEOF       0x08
#define H2O_IFR_STYLE_TYPE_NUMERIC     0x09
#define H2O_IFR_STYLE_TYPE_ORDEREDLIST 0x0a
#define H2O_IFR_STYLE_TYPE_PASSWORD    0x0b
#define H2O_IFR_STYLE_TYPE_STRING      0x0c
#define H2O_IFR_STYLE_TYPE_TIME        0x0d
#define H2O_IFR_STYLE_TYPE_TEXT        0x0e
#define H2O_IFR_STYLE_TYPE_SUBTITLE    0x0f
#define H2O_IFR_STYLE_TYPE_RESETBUTTON 0x10
#define H2O_IFR_STYLE_TYPE_HOTKEY      0x11
#define H2O_IFR_STYLE_TYPE_DIALOG      0x12
#define H2O_IFR_STYLE_TYPE_STATEMENT   0x13
#define H2O_IFR_STYLE_TYPE_SHEET       0xff

///
/// HelpImage opcode.
///
typedef struct _H2O_IFR_GUID_HELPIMAGE {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_HELPIMAGE
  ///
  UINT16              Function;
  EFI_IMAGE_ID        HelpImage;
} H2O_IFR_GUID_HELPIMAGE;

///
/// Property opcode.
///
typedef struct _H2O_IFR_GUID_PROPERTY {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_PROPERTY
  ///
  UINT16              Function;
  UINT32              ValueType;
  H2O_VALUE           H2OValue;
  UINT8               IdentifierOffset;
  UINT8               ValueOffset;
//CHAR8               Property[1];
} H2O_IFR_GUID_PROPERTY;

///
/// Property value type
///
#define H2O_PROPERTY_VALUE_TYPE_ANIMATION    0x0001
#define H2O_PROPERTY_VALUE_TYPE_BOOLEAN      0x0002
#define H2O_PROPERTY_VALUE_TYPE_COLOR        0x0003
#define H2O_PROPERTY_VALUE_TYPE_COORD        0x0004
#define H2O_PROPERTY_VALUE_TYPE_ENUM         0x0005
#define H2O_PROPERTY_VALUE_TYPE_ENUMS        0x0006
#define H2O_PROPERTY_VALUE_TYPE_GUID         0x0007
#define H2O_PROPERTY_VALUE_TYPE_ID           0x0008
#define H2O_PROPERTY_VALUE_TYPE_IMAGE        0x0009
#define H2O_PROPERTY_VALUE_TYPE_KEYWORD      0x000A
#define H2O_PROPERTY_VALUE_TYPE_KEYWORDS     0x000B
#define H2O_PROPERTY_VALUE_TYPE_SIZE         0x000C
#define H2O_PROPERTY_VALUE_TYPE_STRING       0x000D
#define H2O_PROPERTY_VALUE_TYPE_TOKEN        0x000E
#define H2O_PROPERTY_VALUE_TYPE_BUFFER       0x000F
#define H2O_PROPERTY_VALUE_TYPE_DATE         0x0010
#define H2O_PROPERTY_VALUE_TYPE_TIME         0x0011

#define H2O_PROPERTY_VALUE_TYPE_HIIVALUE     0x00FF
#define H2O_PROPERTY_VALUE_TYPE_OVERRIDE     0xFFFF

///
/// Style Ref opcode.
///
typedef struct _H2O_IFR_GUID_STYLE_REF {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_STYLE_REF
  ///
  UINT16              Function;
  UINT8               Type;
  UINT8               ClassNameOffset;
  UINT8               PseudoClassOffset;
//CHAR8               Selector[1];
} H2O_IFR_GUID_STYLE_REF;

///
/// Extended Text opcode.
///
typedef struct _H2O_IFR_GUID_TEXT {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_TEXT
  ///
  UINT16              Function;
  EFI_STRING_ID       Text;
} H2O_IFR_GUID_TEXT;

///
/// Extended Flags opcode.
///
typedef struct _H2O_IFR_GUID_EXT_FLAGS {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_EXT_FLAGS
  ///
  UINT16              Function;
  UINT32              ExtFlags;
} H2O_IFR_GUID_EXT_FLAGS;

//
// Extended Flags
//
#define H2O_IFR_EXT_FLAG_SETUP_CHANGE_LIST_IGNORE 0x00000001

///
/// Include Guid opcode for Link opcode.
///
typedef struct _H2O_IFR_GUID_INCLUDE_FORM_SET {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_INCLUDE_FORM_SET
  ///
  UINT16              Function;
  EFI_GUID            IncludeGuid;
} H2O_IFR_GUID_INCLUDE_FORM_SET;

///
/// Exclude Guid opcode for Link opcode.
///
typedef struct _H2O_IFR_GUID_EXCLUDE_FORM_SET {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_EXCLUDE_FORM_SET
  ///
  UINT16              Function;
  EFI_GUID            ExcludeGuid;
} H2O_IFR_GUID_EXCLUDE_FORM_SET;

typedef UINT16 H2O_PANEL_ID;
typedef UINT16 H2O_LAYOUT_ID;

//
// Hii Layout Package
//
#define H2O_HII_PACKAGE_LAYOUTS    0xe0

typedef struct _H2O_LAYOUT_PACKAGE_HDR {
  EFI_HII_PACKAGE_HEADER   Header;
  UINT32                   HdrSize;
  UINT32                   LayoutBlockOffset;
} H2O_LAYOUT_PACKAGE_HDR;

typedef struct _H2O_HII_LAYOUT_BLOCK {
  UINT8                    BlockType;
  UINT8                    BlockSize;
//UINT8                    BlockData[1];
} H2O_HII_LAYOUT_BLOCK;

#define H2O_HII_LIBT_LAYOUT_BEGIN  0x01
#define H2O_HII_LIBT_LAYOUT_END    0x02
#define H2O_HII_LIBT_PANEL_BEGIN   0x03
#define H2O_HII_LIBT_PANEL_END     0x04
#define H2O_HII_LIBT_STYLE_BEGIN   0x05
#define H2O_HII_LIBT_STYLE_END     0x06
#define H2O_HII_LIBT_PROPERTY      0x07
#define H2O_HII_LIBT_LAYOUT_DUP    0x08
#define H2O_HII_LIBT_PANEL_DUP     0x09
#define H2O_HII_LIBT_EXT2          0x10
#define H2O_HII_LIBT_EXT4          0x11
#define H2O_HII_LIBT_FORMAT        0x12
#define H2O_HII_LIBT_ENUM          0x13
#define H2O_HII_LIBT_KEYWORD       0x14

typedef struct _H2O_HII_LIBT_LAYOUT_BEGIN_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  H2O_LAYOUT_ID            LayoutId;
  EFI_GUID                 DisplayEngineGuid[3];
} H2O_HII_LIBT_LAYOUT_BEGIN_BLOCK;

typedef struct _H2O_HII_LIBT_LAYOUT_END_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
} H2O_HII_LIBT_LAYOUT_END_BLOCK;

typedef UINT32 H2O_PANEL_TYPE;

#define H2O_PANEL_TYPE_MENU       0x0000
#define H2O_PANEL_TYPE_NAVIGATION 0x0001
#define H2O_PANEL_TYPE_FORM       0x0002
#define H2O_PANEL_TYPE_INFO       0x0003
#define H2O_PANEL_TYPE_TITLE      0x0004
#define H2O_PANEL_TYPE_HELP       0x0005
#define H2O_PANEL_TYPE_FORM2      0x0006
#define H2O_PANEL_TYPE_GUID       0xffff

typedef struct _H2O_HII_LIBT_PANEL_BEGIN_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  H2O_PANEL_ID             PanelId;
  H2O_PANEL_TYPE           PanelType;
  EFI_GUID                 PanelGuid;
} H2O_HII_LIBT_PANEL_BEGIN_BLOCK;

typedef struct _H2O_HII_LIBT_PANEL_END_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
} H2O_HII_LIBT_PANEL_END_BLOCK;

typedef struct _H2O_HII_LIBT_STYLE_BEGIN_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT32                   Type;
  UINT8                    ClassNameOffset;
  UINT8                    PseudoClassOffset;
//CHAR8                    Selector[1];
} H2O_HII_LIBT_STYLE_BEGIN_BLOCK;

typedef struct _H2O_HII_LIBT_STYLE_END_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
} H2O_HII_LIBT_STYLE_END_BLOCK;

typedef struct _H2O_HII_LIBT_PROPERTY_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT32                   ValueType;
  OLD_H2O_VALUE            OldH2OValue;
  UINT8                    IdentifierOffset;
  UINT8                    ValueOffset;
  H2O_VALUE                H2OValue;
//CHAR8                    Property[1];
} H2O_HII_LIBT_PROPERTY_BLOCK;

typedef struct _H2O_HII_LIBT_FORMAT_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT32                   StyleType;
  UINT32                   ValueType;
  UINT8                    IdentifierOffset;
} H2O_HII_LIBT_FORMAT_BLOCK;

typedef struct _H2O_HII_LIBT_ENUM_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT32                   ValueNumber;
  UINT8                    ValueOffset;
} H2O_HII_LIBT_ENUM_BLOCK;

typedef struct _H2O_HII_LIBT_KEYWORD_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT8                    ValueOffset;
} H2O_HII_LIBT_KEYWORD_BLOCK;

typedef struct _H2O_HII_LIBT_LAYOUT_DUP_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  H2O_LAYOUT_ID            LayoutId;
  H2O_LAYOUT_ID            OldLayoutId;
} H2O_HII_LIBT_LAYOUT_DUP_BLOCK;

typedef struct _H2O_HII_LIBT_PANEL_DUP_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  H2O_PANEL_ID             PanelId;
  H2O_LAYOUT_ID            OldLayoutId;
  H2O_PANEL_ID             OldPanelId;
} H2O_HII_LIBT_PANEL_DUP_BLOCK;

typedef struct _H2O_HII_LIBT_EXT2_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT8                    BlockType;
  UINT16                   BlockSize;
//UINT8                    BlockData[1];
} H2O_HII_LIBT_EXT2_BLOCK;

typedef struct _H2O_HII_LIBT_EXT4_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT8                    BlockType;
  UINT32                   BlockSize;
//UINT8                    BlockData[1];
} H2O_HII_LIBT_EXT4_BLOCK;

//
// ======= Vfr Tree =========
//
typedef UINT16 H2O_VFR_ID;
typedef UINT16 H2O_FORM_ID;
typedef UINT16 H2O_HOTKEY_ID;

#define H2O_HII_LIBT_EXT_VFR         0x81
#define H2O_HII_LIBT_EXT_VFR_END     0x82
#define H2O_HII_LIBT_EXT_FORMSET     0x83
#define H2O_HII_LIBT_EXT_FORMSET_END 0x84
#define H2O_HII_LIBT_EXT_FORM        0x85
#define H2O_HII_LIBT_EXT_FORM_END    0x86
#define H2O_HII_LIBT_EXT_HOTKEY      0x87
#define H2O_HII_LIBT_EXT_HOTKEY_END  0x88
#define H2O_HII_LIBT_EXT_IMPORT      0x89
#define H2O_HII_LIBT_EXT_IMPORT_END  0x8A
#define H2O_HII_LIBT_EXT_STATEMENT      0x8B
#define H2O_HII_LIBT_EXT_STATEMENT_END  0x8C

typedef struct _H2O_HII_LIBT_EXT_VFR_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  H2O_VFR_ID               VfrId;
  UINT8                    SourceOffset;
} H2O_HII_LIBT_EXT_VFR_BLOCK;

typedef struct _H2O_HII_LIBT_EXT_VFR_END_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
} H2O_HII_LIBT_EXT_VFR_END_BLOCK;

typedef struct _H2O_HII_LIBT_EXT_FORMSET_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  EFI_GUID                 FormsetGuid;
} H2O_HII_LIBT_EXT_FORMSET_BLOCK;

typedef struct _H2O_HII_LIBT_EXT_FORMSET_END_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
} H2O_HII_LIBT_EXT_FORMSET_END_BLOCK;

typedef struct _H2O_HII_LIBT_EXT_FORM_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  H2O_FORM_ID              FormId;
} H2O_HII_LIBT_EXT_FORM_BLOCK;

typedef struct _H2O_HII_LIBT_EXT_FORM_END_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
} H2O_HII_LIBT_EXT_FORM_END_BLOCK;

typedef struct _H2O_HII_LIBT_EXT_HOTKEY_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  H2O_HOTKEY_ID            HotkeyId;
} H2O_HII_LIBT_EXT_HOTKEY_BLOCK;

typedef struct _H2O_HII_LIBT_EXT_HOTKEY_END_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
} H2O_HII_LIBT_EXT_HOTKEY_END_BLOCK;

#define HII_H2O_HII_IMPORT_QUESTION          BIT0

typedef struct _H2O_HII_LIBT_EXT_IMPORT_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT16                   Flags;
  EFI_GUID                 TargetFormsetGuid;
  EFI_FORM_ID              TargetFormId;
  EFI_QUESTION_ID          TargetId;
  EFI_QUESTION_ID          LocalId;
  UINT16                   TargetVarstoreSize;
  EFI_GUID                 TargetVarstoreGuid;
  //CHAR8                  TargetVarstoreName[1];
} H2O_HII_LIBT_EXT_IMPORT_BLOCK;

#define H2O_HII_STATEMENT_QUESTION      BIT0

typedef struct _H2O_HII_LIBT_EXT_STATEMENT_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT16                   Flags;
  EFI_QUESTION_ID          TargetId;
} H2O_HII_LIBT_EXT_STATEMENT_BLOCK;

typedef struct _H2O_HII_LIBT_EXT_STATEMENT_END_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
} H2O_HII_LIBT_EXT_STATEMENT_END_BLOCK;

#pragma pack()

extern EFI_GUID gH2OIfrExtGuid;

#endif


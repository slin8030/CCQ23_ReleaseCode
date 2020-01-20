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

#include "LayoutSupportLib.h"

STATIC
BOOLEAN
IsValueExist (
  IN UINT32                                    ValueArrayCount,
  IN UINT32                                    *ValueArray,
  IN UINT32                                    Value
  )
{
  UINT32                                       Index;

  if (ValueArray != NULL) {
    for (Index = 0; Index < ValueArrayCount; Index++) {
      if (ValueArray[Index] == Value) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

STATIC
UINT32
GetListCount (
  IN LIST_ENTRY                                *LinkHead
  )
{
  LIST_ENTRY                                   *Link;
  UINT32                                       Count;

  if (LinkHead == NULL || IsListEmpty (LinkHead)) {
    return 0;
  }

  Link  = LinkHead;
  Count = 0;
  do {
    Link = Link->ForwardLink;
    Count++;
  } while (!IsNodeAtEnd (LinkHead, Link));

  return Count;
}

STATIC
VOID
BubbleSort (
  IN     UINT32                               ArrayCount,
  IN OUT UINT32                               *Array
  )
{
  UINT32                                      Index;
  UINT32                                      ArrayIndex;
  UINT32                                      Temp;

  if (Array == NULL) {
    return;
  }

  for (Index = 0; Index < ArrayCount - 1; Index++) {
    for (ArrayIndex = 1; ArrayIndex < ArrayCount; ArrayIndex++) {
      if (Array[ArrayIndex - 1] > Array[ArrayIndex]) {
        Temp                  = Array[ArrayIndex - 1];
        Array[ArrayIndex - 1] = Array[ArrayIndex];
        Array[ArrayIndex]     = Temp;
      }
    }
  }
}

STATIC
EFI_STATUS
ConvertToHiiValue (
  IN  H2O_VALUE                               *H2OValue,
  OUT EFI_HII_VALUE                           *HiiValue
  )
{
  EFI_STATUS                                   Status;
  CHAR16                                       *String;
  UINTN                                        StringSize;
  LAYOUT_DATABASE_PROTOCOL                     *LayoutDatabase;

  if (H2OValue == NULL || HiiValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem (HiiValue, sizeof(EFI_HII_VALUE));

  switch (H2OValue->Type) {

  case H2O_VALUE_TYPE_BOOLEAN:
    HiiValue->Type    = EFI_IFR_TYPE_BOOLEAN;
    HiiValue->Value.b = H2OValue->Value.Bool;
    break;

  case H2O_VALUE_TYPE_UINT8:
    HiiValue->Type     = EFI_IFR_TYPE_NUM_SIZE_8;
    HiiValue->Value.u8 = H2OValue->Value.U8;
    break;

  case H2O_VALUE_TYPE_UINT16:
    HiiValue->Type      = EFI_IFR_TYPE_NUM_SIZE_16;
    HiiValue->Value.u16 = H2OValue->Value.U16;
    break;

  case H2O_VALUE_TYPE_UINT32:
    HiiValue->Type      = EFI_IFR_TYPE_NUM_SIZE_32;
    HiiValue->Value.u32 = H2OValue->Value.U32;
    break;

  case H2O_VALUE_TYPE_UINT64:
    HiiValue->Type      = EFI_IFR_TYPE_NUM_SIZE_64;
    HiiValue->Value.u64 = H2OValue->Value.U64;
    break;

  case H2O_VALUE_TYPE_STR16:
    HiiValue->Type      = EFI_IFR_TYPE_STRING;
    HiiValue->BufferLen = (UINT16) StrSize (H2OValue->Value.Str16);
    HiiValue->Buffer    = AllocateCopyPool ((UINTN) HiiValue->BufferLen, H2OValue->Value.Str16);
    break;

  case H2O_VALUE_TYPE_STRID:
    Status = gBS->LocateProtocol (&gLayoutDatabaseProtocolGuid, NULL, (VOID **) &LayoutDatabase);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    HiiValue->Type      = EFI_IFR_TYPE_STRING;
    String = HiiGetString (LayoutDatabase->ImagePkgHiiHandle, H2OValue->Value.U16, NULL);
    if (String == NULL) {
      return EFI_NOT_FOUND;
    }
    StringSize = StrSize (String);

    HiiValue->Value.string = (EFI_STRING_ID) H2OValue->Value.U16;
    HiiValue->BufferLen    = (UINT16) StringSize + sizeof (EFI_HII_HANDLE);
    HiiValue->Buffer       = AllocateZeroPool (HiiValue->BufferLen);
    if (HiiValue->Buffer == NULL) {
      FreePool (String);
      return EFI_OUT_OF_RESOURCES;
    }
    CopyMem (HiiValue->Buffer, String, StringSize);
    CopyMem (&HiiValue->Buffer[StringSize], &LayoutDatabase->ImagePkgHiiHandle, sizeof (EFI_HII_HANDLE));
    FreePool (String);
    break;

  case H2O_VALUE_TYPE_BUFFER:
    HiiValue->Type      = EFI_IFR_TYPE_BUFFER;
    HiiValue->BufferLen = (UINT16) H2OValue->Value.Buffer.BufferSize;
    HiiValue->Buffer    = AllocateCopyPool ((UINTN) HiiValue->BufferLen, H2OValue->Value.Buffer.Buffer);
    break;

  case H2O_VALUE_TYPE_TIME:
    HiiValue->Type              = EFI_IFR_TYPE_TIME;
    HiiValue->Value.time.Hour   = H2OValue->Value.Time.Hour;
    HiiValue->Value.time.Minute = H2OValue->Value.Time.Minute;
    HiiValue->Value.time.Second = H2OValue->Value.Time.Second;
    break;

  case H2O_VALUE_TYPE_DATE:
    HiiValue->Type             = EFI_IFR_TYPE_DATE;
    HiiValue->Value.date.Year  = H2OValue->Value.Date.Year;
    HiiValue->Value.date.Month = H2OValue->Value.Date.Month;
    HiiValue->Value.date.Day   = H2OValue->Value.Date.Day;
    break;

  default:
    ASSERT (FALSE);
    break;
  }

  return EFI_SUCCESS;
}

STATIC
UINT32
Abs (
  IN     UINT32                              First,
  IN     UINT32                              Second
  )
{
  UINT32                                     Result;

  Result = 0;
  if (First >= Second) {
    Result = First - Second;
  } else {
    Result = Second - First;
  }

  return Result;
}

STATIC
UINT32
ColorGap (
  IN     EFI_GRAPHICS_OUTPUT_BLT_PIXEL       *Color1,
  IN     EFI_GRAPHICS_OUTPUT_BLT_PIXEL       *Color2
  )
{
  UINT32                                     Result;
  UINT32                                     GapR;
  UINT32                                     GapG;
  UINT32                                     GapB;

  GapR = Abs ((UINT32)Color1->Red, (UINT32)Color2->Red);
  GapG = Abs ((UINT32)Color1->Green, (UINT32)Color2->Green);
  GapB = Abs ((UINT32)Color1->Blue, (UINT32)Color2->Blue);
  Result = GapR * GapR + GapG * GapG + GapB * GapB;

  return Result;
}

STATIC
UINT32
RgbToAttribute (
  IN     EFI_GRAPHICS_OUTPUT_BLT_PIXEL       *Rgb
  )
{
  UINT32                                     Index;
  UINT32                                     Gap;
  UINT32                                     MinGap;
  UINT32                                     MinIndex;
  UINT32                                     Attribute;
  RGB_ATTR                                   RgbAttr[] = {
                                               //
                                               // B    G    R   reserved
                                               //
                                               {{0x00, 0x00, 0x00, 0xff}, EFI_BLACK       },
                                               {{0x98, 0x00, 0x00, 0xff}, EFI_LIGHTBLUE   },
                                               {{0x00, 0x98, 0x00, 0xff}, EFI_LIGHTGREEN  },
                                               {{0x98, 0x98, 0x00, 0xff}, EFI_LIGHTCYAN   },
                                               {{0x00, 0x00, 0x98, 0xff}, EFI_LIGHTRED    },
                                               {{0x98, 0x00, 0x98, 0xff}, EFI_MAGENTA     },
                                               {{0x00, 0x98, 0x98, 0xff}, EFI_BROWN       },
                                               {{0x98, 0x98, 0x98, 0xff}, EFI_LIGHTGRAY   },
                                               {{0x30, 0x30, 0x30, 0xff}, EFI_DARKGRAY    },
                                               {{0xff, 0x00, 0x00, 0xff}, EFI_BLUE        },
                                               {{0x00, 0xff, 0x00, 0xff}, EFI_GREEN       },
                                               {{0xff, 0xff, 0x00, 0xff}, EFI_CYAN        },
                                               {{0x00, 0x00, 0xff, 0xff}, EFI_RED         },
                                               {{0xff, 0x00, 0xff, 0xff}, EFI_LIGHTMAGENTA},
                                               {{0x00, 0xff, 0xff, 0xff}, EFI_YELLOW      },
                                               {{0xff, 0xff, 0xff, 0xff}, EFI_WHITE       },
                                               {{0x00, 0x00, 0x00, 0x00}, EFI_BLACK       }
                                             };

  MinGap = 0;
  MinIndex = 0;
  for (Index = 0; RgbAttr[Index].Rgb.Reserved != 0; Index ++) {
    Gap = ColorGap (Rgb, &RgbAttr[Index].Rgb);
    if (Gap <= MinGap) {
      //
      // Get Most Closest Attribute
      //
      MinGap = Gap;
      MinIndex = Index;
      if (MinGap == 0) {
        //
        // Same Color
        //
        break;
      }
    }
  }
  Attribute = RgbAttr[MinIndex].Attribute;

  return Attribute;
}

STATIC
EFI_STATUS
ParseH2OValue (
  IN     H2O_PROPERTY_INFO                     *Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  CopyMem (&PropValue->H2OValue, &Property->H2OValue, sizeof (H2O_VALUE));

  switch (Property->H2OValue.Type) {

  case H2O_VALUE_TYPE_STR16:
    PropValue->H2OValue.Value.Str16 = (CHAR16 *)Property->ValueStr;
    break;

  case H2O_VALUE_TYPE_BUFFER:
    PropValue->H2OValue.Value.Buffer.Buffer = (UINT8 *)Property->ValueStr;
    break;

  default:
    break;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ParseSizeValue (
  IN     INT32                                 RelativeSize,
  IN     H2O_PROPERTY_INFO                     *Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  if (Property->H2OValue.Type == H2O_VALUE_TYPE_PERCENTAGE) {
    PropValue->H2OValue.Type = H2O_VALUE_TYPE_UINT32;
    PropValue->H2OValue.Value.U32 = (UINT32)RelativeSize * Property->H2OValue.Value.U32 / 100;
  } else if (Property->H2OValue.Type == H2O_VALUE_TYPE_INT32 || Property->H2OValue.Type == H2O_VALUE_TYPE_INT64) {
    PropValue->H2OValue.Type = H2O_VALUE_TYPE_UINT32;
    if (Property->H2OValue.Value.I32 < 0) {
      PropValue->H2OValue.Value.U32 = (UINT32)(RelativeSize + Property->H2OValue.Value.I32);
    } else {
      PropValue->H2OValue.Value.U32 = (UINT32)(Property->H2OValue.Value.I32);
    }
  } else {
    CopyMem (&PropValue->H2OValue, &Property->H2OValue, sizeof (H2O_VALUE));
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ParseCoordValue (
  IN     RECT                                  *RelativeField,
  IN     H2O_PROPERTY_INFO                     *Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  INT32                                        RelativeSize;

  RelativeSize = 1;
  if (RelativeField != NULL) {
    if (AsciiStrStr (Property->IdentifierStr, "left") != NULL ||
      AsciiStrStr (Property->IdentifierStr, "right") != NULL) {
      RelativeSize = RelativeField->right - RelativeField->left;
    } else if (AsciiStrStr (Property->IdentifierStr, "top") != NULL ||
      AsciiStrStr (Property->IdentifierStr, "bottom") != NULL) {
      RelativeSize = RelativeField->bottom - RelativeField->top;
    }
  }

  ParseSizeValue (RelativeSize, Property, PropValue);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ParseColorValue (
  IN     H2O_PROPERTY_INFO                     *Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL                Rgb;
  UINT32                                       Attribute;
  UINT32                                       RgbNumber;
  UINT32                                       TempAttribute;
  CHAR8                                        *StrPtr;
  BOOLEAN                                      Background;

  StrPtr = Property->ValueStr;
  TempAttribute = 0;

  Background = FALSE;
  if (AsciiStrStr (Property->IdentifierStr, "background") != NULL) {
    Background = TRUE;
  }

  if (Property->H2OValue.Type == H2O_VALUE_TYPE_UINT32) {
    RgbNumber = (UINT32)Property->H2OValue.Value.U32;
    CopyMem (&Rgb, &RgbNumber, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    TempAttribute = RgbToAttribute (&Rgb);
  } else {
    return EFI_NOT_FOUND;
  }

  Attribute = 0;
  if (Background) {
    if (TempAttribute == EFI_WHITE) {
      TempAttribute = EFI_LIGHTGRAY;
    }
    TempAttribute <<= 4;
    Attribute &= 0x0f;
    Attribute |= TempAttribute;
  } else {
    Attribute &= 0xf0;
    Attribute |= TempAttribute;
  }

  CopyMem (&PropValue->Value.Color.Rgb, &Rgb, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  PropValue->Value.Color.Attribute = Attribute;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ParseImageValue (
  IN     H2O_PROPERTY_INFO                     *Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  if (Property->H2OValue.Type == H2O_VALUE_TYPE_STR16) {
    PropValue->Value.Image.ImageMapStr = (CHAR16 *)Property->ValueStr;
  } else {
    CopyMem (&PropValue->H2OValue, &Property->H2OValue, sizeof (H2O_VALUE));
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ParseProperty (
  IN     RECT                                  *RelativeField OPTIONAL,
  IN     H2O_PROPERTY_INFO                     *Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  ZeroMem (PropValue, sizeof (H2O_PROPERTY_VALUE));

  PropValue->Type = Property->ValueType;
  ParseH2OValue (Property, PropValue);

  switch (Property->ValueType){

  case H2O_PROPERTY_VALUE_TYPE_SIZE:
    ParseSizeValue (1, Property, PropValue);
    break;

  case H2O_PROPERTY_VALUE_TYPE_COORD:
    ParseCoordValue (RelativeField, Property, PropValue);
    break;

  case H2O_PROPERTY_VALUE_TYPE_COLOR:
    ParseColorValue (Property, PropValue);
    break;

  case H2O_PROPERTY_VALUE_TYPE_IMAGE:
  case H2O_PROPERTY_VALUE_TYPE_ANIMATION:
    ParseImageValue (Property, PropValue);
    break;

  case H2O_PROPERTY_VALUE_TYPE_BOOLEAN:
  case H2O_PROPERTY_VALUE_TYPE_ID:
  case H2O_PROPERTY_VALUE_TYPE_TOKEN:
  case H2O_PROPERTY_VALUE_TYPE_ENUM:
  case H2O_PROPERTY_VALUE_TYPE_ENUMS:
  case H2O_PROPERTY_VALUE_TYPE_KEYWORD:
  case H2O_PROPERTY_VALUE_TYPE_KEYWORDS:
  case H2O_PROPERTY_VALUE_TYPE_STRING:
  case H2O_PROPERTY_VALUE_TYPE_GUID:
  case H2O_PROPERTY_VALUE_TYPE_DATE:
  case H2O_PROPERTY_VALUE_TYPE_TIME:
  case H2O_PROPERTY_VALUE_TYPE_BUFFER:
  case H2O_PROPERTY_VALUE_TYPE_HIIVALUE:
  case H2O_PROPERTY_VALUE_TYPE_OVERRIDE:
    break;

  default:
    //
    // Mismatched IdentifierStr String
    //
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetPropertyFromPropertyList (
  IN     LIST_ENTRY                            *PropertyListHead,
  IN     CHAR8                                 *IdentifierStr,
  IN     RECT                                  *RelativeField OPTIONAL,
  OUT    H2O_PROPERTY_INFO                     **Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  EFI_STATUS                                   Status;
  LIST_ENTRY                                   *Link;
  H2O_PROPERTY_INFO                            *CurrentProperty;


  if (PropertyListHead == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IdentifierStr == NULL || Property == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Link = PropertyListHead;
  if (IsNull (Link, Link->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  Status = EFI_NOT_FOUND;

  *Property = NULL;
  CurrentProperty = NULL;
  do {
    Link = Link->ForwardLink;
    CurrentProperty = H2O_PROPERTY_INFO_NODE_FROM_LINK (Link);

    if (AsciiStrCmp (CurrentProperty->IdentifierStr, IdentifierStr) == 0) {
      ParseProperty (RelativeField, CurrentProperty, PropValue);
      *Property = CurrentProperty;
      Status = EFI_SUCCESS;
    }
  } while (!IsNodeAtEnd (PropertyListHead, Link));

  return Status;
}

EFI_STATUS
GetPropertyFromStyleList (
  IN     LIST_ENTRY                            *StyleListHead,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     CHAR8                                 *IdentifierStr,
  IN     RECT                                  *RelativeField OPTIONAL,
  OUT    H2O_PROPERTY_INFO                     **Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  EFI_STATUS                                   Status;
  LIST_ENTRY                                   *Link;
  H2O_STYLE_INFO                               *CurrentStyle;

  H2O_PROPERTY_INFO                            *TempProperty;
  UINT8                                        Priority;
  UINT8                                        PriorityOld;
  BOOLEAN                                      Changed;


  if (StyleListHead == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IdentifierStr == NULL || Property == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Link = StyleListHead;
  if (IsNull (Link, Link->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  Status = EFI_NOT_FOUND;
  Priority = 0xff;
  PriorityOld = Priority;
  Changed = FALSE;
  *Property = NULL;

  CurrentStyle = NULL;
  do {
    Link = Link->ForwardLink;
    CurrentStyle = H2O_STYLE_INFO_NODE_FROM_LINK (Link);

    if (CurrentStyle->StyleType == H2O_IFR_STYLE_TYPE_HOTKEY ||
      CurrentStyle->StyleType == H2O_IFR_STYLE_TYPE_FORM ||
      CurrentStyle->StyleType == H2O_IFR_STYLE_TYPE_FORMSET ||
      CurrentStyle->StyleType == H2O_IFR_STYLE_TYPE_STATEMENT) {
      //
      // If StyleType is from vfr...endvfr, the formsetguid and formid and hotkeyid are already checked.
      //
      Priority = 0;
      Changed = TRUE;
    } else if (StyleType != 0 && CurrentStyle->StyleType != 0 && StyleType != CurrentStyle->StyleType) {
      continue;
    }

    if (PseudoClass != 0 && CurrentStyle->PseudoClass != 0 && PseudoClass != CurrentStyle->PseudoClass) {
      continue;
    }

    if (StyleType != 0 && CurrentStyle->StyleType != 0 && PseudoClass != 0 && CurrentStyle->PseudoClass != 0) {
      if (StyleType == CurrentStyle->StyleType && (PseudoClass == CurrentStyle->PseudoClass)) {
        //
        // ".ClassName StyleType : PseudoClass" priority is the highest
        //
        if (Priority >= 1) {
          Priority = 1;
          Changed = TRUE;
        }
      }
    } else if (StyleType != 0 && CurrentStyle->StyleType != 0) {
      if (StyleType == CurrentStyle->StyleType) {
        //
        // ".ClassName StyleType" priority is the second
        //
        if (Priority >= 2) {
          Priority = 2;
          Changed = TRUE;
        }
      }
    } else if (PseudoClass != 0 && CurrentStyle->PseudoClass != 0) {
      if (PseudoClass == CurrentStyle->PseudoClass) {
        //
        // ".ClassName : PseudoClass" priority is the third
        //
        if (Priority >= 3) {
          Priority = 3;
          Changed = TRUE;
        }
      }
    } else if (CurrentStyle->StyleType == 0 && CurrentStyle->PseudoClass == 0) {
      //
      // ".ClassName" priority is the last
      //
      if (Priority >= 4) {
        Priority = 4;
        Changed = TRUE;
      }
    }

    if (Changed) {
      Status = GetPropertyFromPropertyList (&CurrentStyle->PropertyListHead, IdentifierStr, RelativeField, &TempProperty, PropValue);
      if (Status == EFI_SUCCESS) {
        *Property = TempProperty;
        PriorityOld = Priority;
      } else {
        Priority = PriorityOld;
      }
      Changed = FALSE;
    }
  } while (!IsNodeAtEnd (StyleListHead, Link));

  if (*Property == NULL) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetPropertyFromHotkeyList (
  IN     LIST_ENTRY                            *HotkeyListHead,
  IN     UINT32                                HotkeyId,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     CHAR8                                 *IdentifierStr,
  IN     RECT                                  *RelativeField OPTIONAL,
  OUT    H2O_PROPERTY_INFO                     **Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  EFI_STATUS                                   Status;
  LIST_ENTRY                                   *HotkeyLink;
  H2O_HOTKEY_INFO                              *CurrentHotkey;

  if (HotkeyId == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (IdentifierStr == NULL || Property == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HotkeyLink = HotkeyListHead;
  if (IsNull (HotkeyLink, HotkeyLink->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  do {
    HotkeyLink = HotkeyLink->ForwardLink;
    CurrentHotkey = H2O_HOTKEY_INFO_NODE_FROM_LINK (HotkeyLink);

    if (CurrentHotkey->HotkeyId == HotkeyId) {
      //
      // Style of hotkey
      //
      Status = GetPropertyFromStyleList (&CurrentHotkey->StyleListHead, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);
      if (Status == EFI_SUCCESS) {
        return EFI_SUCCESS;
      }
    }
  } while (!IsNodeAtEnd (HotkeyListHead, HotkeyLink));

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
GetPropertyFromFormInVfrTree (
  IN     LIST_ENTRY                            *FormListHead,
  IN     UINT32                                FormId,
  IN     UINT32                                HotkeyId,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     CHAR8                                 *IdentifierStr,
  IN     RECT                                  *RelativeField OPTIONAL,
  OUT    H2O_PROPERTY_INFO                     **Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  EFI_STATUS                                   Status;
  LIST_ENTRY                                   *FormLink;
  H2O_FORM_INFO                                *CurrentForm;

  if (FormId == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (IdentifierStr == NULL || Property == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FormLink = FormListHead;
  if (IsNull (FormLink, FormLink->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  do {
    FormLink = FormLink->ForwardLink;
    CurrentForm = H2O_FORM_INFO_NODE_FROM_LINK (FormLink);

    if (CurrentForm->FormId == FormId) {
      //
      // Hotkey of form
      //
      Status = GetPropertyFromHotkeyList (&CurrentForm->HotkeyListHead, HotkeyId, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);
      if (Status == EFI_SUCCESS) {
        return EFI_SUCCESS;
      }
      //
      // Style of form
      //
      Status = GetPropertyFromStyleList (&CurrentForm->StyleListHead, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);
      if (Status == EFI_SUCCESS) {
        return EFI_SUCCESS;
      }
    }
  } while (!IsNodeAtEnd (FormListHead, FormLink));

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
GetPropertyFromFormsetInVfrTree (
  IN     LIST_ENTRY                            *FormsetListHead,
  IN     EFI_GUID                              *FormsetGuid,
  IN     UINT32                                FormId,
  IN     UINT32                                HotkeyId,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     CHAR8                                 *IdentifierStr,
  IN     RECT                                  *RelativeField OPTIONAL,
  OUT    H2O_PROPERTY_INFO                     **Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  EFI_STATUS                                   Status;
  LIST_ENTRY                                   *FormsetLink;
  H2O_FORMSET_INFO                             *CurrentFormset;

  if (FormsetGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IdentifierStr == NULL || Property == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FormsetLink = FormsetListHead;
  if (IsNull (FormsetLink, FormsetLink->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  do {
    FormsetLink = FormsetLink->ForwardLink;
    CurrentFormset = H2O_FORMSET_INFO_NODE_FROM_LINK (FormsetLink);

    if (CompareGuid(&CurrentFormset->FormsetGuid, FormsetGuid)) {
      //
      // Form
      //
      Status = GetPropertyFromFormInVfrTree (&CurrentFormset->FormListHead, FormId, HotkeyId, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);
      if (Status == EFI_SUCCESS) {
        return EFI_SUCCESS;
      }
      //
      // Hotkey of formset
      //
      Status = GetPropertyFromHotkeyList (&CurrentFormset->HotkeyListHead, HotkeyId, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);
      if (Status == EFI_SUCCESS) {
        return EFI_SUCCESS;
      }
      //
      // Style of formset
      //
      Status = GetPropertyFromStyleList (&CurrentFormset->StyleListHead, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);
      if (Status == EFI_SUCCESS) {
        return EFI_SUCCESS;
      }
    }
  } while (!IsNodeAtEnd (FormsetListHead, FormsetLink));

  return EFI_NOT_FOUND;
}

EFI_STATUS
GetPropertyFromVfrInVfrTree (
  IN     LIST_ENTRY                            *VfrListHead,
  IN     EFI_GUID                              *FormsetGuid,
  IN     UINT32                                FormId,
  IN     UINT32                                HotkeyId,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     CHAR8                                 *IdentifierStr,
  IN     RECT                                  *RelativeField OPTIONAL,
  OUT    H2O_PROPERTY_INFO                     **Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  EFI_STATUS                                   Status;
  LIST_ENTRY                                   *VfrLink;
  H2O_VFR_INFO                                 *CurrentVfr;

  if (FormsetGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IdentifierStr == NULL || Property == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  VfrLink = VfrListHead;
  if (IsNull (VfrLink, VfrLink->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  do {
    VfrLink = VfrLink->ForwardLink;
    CurrentVfr = H2O_VFR_INFO_NODE_FROM_LINK (VfrLink);

    //
    // Formset
    //
    Status = GetPropertyFromFormsetInVfrTree (&CurrentVfr->FormsetListHead, FormsetGuid, FormId, HotkeyId, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);
    if (Status == EFI_SUCCESS) {
      return EFI_SUCCESS;
    }
  } while (!IsNodeAtEnd (VfrListHead, VfrLink));

  return EFI_NOT_FOUND;
}

EFI_STATUS
GetPropertyFromVfrInVfcf (
  IN     EFI_GUID                              *FormsetGuid,
  IN     UINT32                                FormId,
  IN     UINT32                                HotkeyId,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     CHAR8                                 *IdentifierStr,
  IN     RECT                                  *RelativeField OPTIONAL,
  OUT    H2O_PROPERTY_INFO                     **Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  EFI_STATUS                                   Status;
  LAYOUT_DATABASE_PROTOCOL                     *LayoutDatabase;

  if (FormsetGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IdentifierStr == NULL || Property == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_NOT_FOUND;

  Status = gBS->LocateProtocol (&gLayoutDatabaseProtocolGuid, NULL, (VOID **) &LayoutDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Vfr
  //
  Status = GetPropertyFromVfrInVfrTree (LayoutDatabase->VfrListHead, FormsetGuid, FormId, HotkeyId, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);
  if (Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }

  return Status;
}

STATIC
EFI_STATUS
GetPropertyFromPanel (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     UINT32                                HotkeyId,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     CHAR8                                 *IdentifierStr,
  IN     RECT                                  *RelativeField OPTIONAL,
  OUT    H2O_PROPERTY_INFO                     **Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  EFI_STATUS                                   Status;

  if (Panel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IdentifierStr == NULL || Property == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_NOT_FOUND;

  //
  // Panel itself style
  //
  Status = GetPropertyFromStyleList (&Panel->StyleListHead, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);
  if (Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }

  //
  // Panel's parent layout style
  //
  Status = GetPropertyFromStyleList (&Panel->ParentLayout->StyleListHead, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);

  //
  // Hotkey of panel's parent layout
  //
  Status = GetPropertyFromHotkeyList (&Panel->ParentLayout->HotkeyListHead, HotkeyId, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);
  if (Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }

  return Status;
}

STATIC
EFI_STATUS
GetPropertyFromVfrAndPanel (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     CHAR8                                 *IdentifierStr,
  IN     RECT                                  *RelativeField OPTIONAL,
  OUT    H2O_PROPERTY_INFO                     **Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  )
{
  EFI_STATUS                                   Status;
  H2O_VFR_PROPERTY                             EmptyVfrProperty;

  if (Panel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IdentifierStr == NULL || Property == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (VfrProperty == NULL) {
    ZeroMem (&EmptyVfrProperty, sizeof (H2O_VFR_PROPERTY));
    VfrProperty = &EmptyVfrProperty;
  }

  Status = EFI_NOT_FOUND;

  //
  // Priority: Vfr Option > Vfr Statement > Vfr Form > Vfr Formset > Vfr in Vfcf > Vfcf
  //
  Status = GetPropertyFromPropertyList (VfrProperty->OptionPropList, IdentifierStr, RelativeField, Property, PropValue);
  if (Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }

  Status = GetPropertyFromPropertyList (VfrProperty->StatementPropList, IdentifierStr, RelativeField, Property, PropValue);
  if (Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }

  Status = GetPropertyFromPropertyList (VfrProperty->FormPropList, IdentifierStr, RelativeField, Property, PropValue);
  if (Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }

  Status = GetPropertyFromPropertyList (VfrProperty->FormsetPropList, IdentifierStr, RelativeField, Property, PropValue);
  if (Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }

  Status = GetPropertyFromVfrInVfcf (&VfrProperty->FormsetGuid, VfrProperty->FormId, VfrProperty->HotkeyId, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);
  if (Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }

  Status = GetPropertyFromPanel (Panel, VfrProperty->HotkeyId, StyleType, PseudoClass, IdentifierStr, RelativeField, Property, PropValue);

  return Status;
}

BOOLEAN
IsVisibility (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL) {
    return FALSE;
  }

  Status = EFI_NOT_FOUND;

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "visibility", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    //
    // Not found, default visibility is true
    //
    return TRUE;
  }

  return PropValue.H2OValue.Value.Bool;
}

EFI_STATUS
GetPanelField (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     RECT                                  *RelativeField,
  OUT    RECT                                  *PanelField
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;
  RECT                                         ResultRect;

  if (Panel == NULL || PanelField == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "left", RelativeField, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ResultRect.left = PropValue.H2OValue.Value.U32;

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "top", RelativeField, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ResultRect.top = PropValue.H2OValue.Value.U32;

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "right", RelativeField, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ResultRect.right = PropValue.H2OValue.Value.U32;

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "bottom", RelativeField, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ResultRect.bottom = PropValue.H2OValue.Value.U32;

  CopyRect (PanelField, &ResultRect);

  return EFI_SUCCESS;
}

EFI_STATUS
GetPaddingRect(
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     RECT                                  *RelativeField,
  OUT    RECT                                  *Rect
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;
  RECT                                         ResultRect;

  if (Panel == NULL || Rect == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "padding-left", RelativeField, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ResultRect.left = PropValue.H2OValue.Value.U32;

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "padding-top", RelativeField, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ResultRect.top = PropValue.H2OValue.Value.U32;

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "padding-right", RelativeField, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ResultRect.right = PropValue.H2OValue.Value.U32;

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "padding-bottom", RelativeField, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ResultRect.bottom = PropValue.H2OValue.Value.U32;

  CopyRect (Rect, &ResultRect);

  return EFI_SUCCESS;
}

EFI_STATUS
GetForegroundColor (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    H2O_COLOR_INFO                        *Color
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Color == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "foreground-color", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CopyMem (Color, &PropValue.Value.Color, sizeof (H2O_COLOR_INFO));

  return EFI_SUCCESS;
}

EFI_STATUS
GetBackgroundColor (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    H2O_COLOR_INFO                        *Color
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Color == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "background-color", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CopyMem (Color, &PropValue.Value.Color, sizeof (H2O_COLOR_INFO));

  return EFI_SUCCESS;
}

EFI_STATUS
GetPanelColorAttribute (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    UINT32                                *PanelColorAttribute
  )
{
  EFI_STATUS                                   Status;
  H2O_COLOR_INFO                               Color;
  UINT32                                       Attribute;

  Attribute = 0;

  Status = GetForegroundColor (Panel, VfrProperty, StyleType, PseudoClass, &Color);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Attribute |= Color.Attribute;

  Status = GetBackgroundColor (Panel, VfrProperty, StyleType, PseudoClass, &Color);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Attribute |= Color.Attribute;

  *PanelColorAttribute = Attribute;

  return EFI_SUCCESS;
}

EFI_STATUS
GetBackgroundImage (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    H2O_IMAGE_INFO                        *Image
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Image == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "background-image", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CopyMem (Image, &PropValue.Value.Image, sizeof (H2O_IMAGE_INFO));

  return EFI_SUCCESS;
}

UINT32
GetBorderWidth (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL) {
    return 0;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "border-width", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    //
    // Not found, default border width is 0
    //
    return 0;
  }

  return PropValue.H2OValue.Value.U32;
}

UINT32
GetFontSize (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL) {
    return 0;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "font-size", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    //
    // Not found, default border width is 0
    //
    return 0;
  }

  return PropValue.H2OValue.Value.U32;
}

CHAR16 *
GetFontName (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;
  CHAR16                                       *FontName;

  if (Panel == NULL) {
    return NULL;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "font-name", NULL, &Property, &PropValue);
  if (Status != EFI_SUCCESS || Property == NULL) {
    //
    // Not found, default border width is 0
    //
    return NULL;
  }

  FontName = AllocateZeroPool (sizeof (CHAR16) * (AsciiStrLen (Property->ValueStr) + 1));
  if (FontName != NULL) {
    AsciiStrToUnicodeStr (Property->ValueStr, FontName);
  }

  return FontName;
}

UINT32
GetResolution (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL) {
    return 0;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "resolution", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    //
    // Not found, default border width is 0
    //
    return 0;
  }

  return PropValue.H2OValue.Value.U32;
}

UINT32
GetHelpImageDisplay (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL) {
    return 0;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "help-image-display", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    //
    // Not found, default border width is 0
    //
    return 0;
  }

  return PropValue.H2OValue.Value.U32;
}

BOOLEAN
IsHelpTextShow (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL) {
    return FALSE;
  }

  Status = EFI_NOT_FOUND;

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "help-text-show", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    //
    // Not found, default help-text-show is false
    //
    return FALSE;
  }

  return PropValue.H2OValue.Value.Bool;
}

UINT32
GetHelpTextFontSize (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL) {
    return 0;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "help-text-font-size", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    //
    // Not found, default border width is 0
    //
    return 0;
  }

  return PropValue.H2OValue.Value.U32;
}

EFI_STATUS
GetScrollbarColor (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    H2O_COLOR_INFO                        *Color
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Color == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "scrollbar-color", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CopyMem (Color, &PropValue.Value.Color, sizeof (H2O_COLOR_INFO));

  return EFI_SUCCESS;
}

UINT32
GetScrollbarWidth (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL) {
    return 0;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "scrollbar-width", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    //
    // Not found, default scrollbar width is 0
    //
    return 0;
  }

  return PropValue.H2OValue.Value.U32;
}

EFI_STATUS
GetHeight(
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     RECT                                  *RelativeField,
  OUT    INT32                                 *Height
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Height == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "height", RelativeField, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *Height = PropValue.H2OValue.Value.U32;

  return EFI_SUCCESS;
}

EFI_STATUS
GetWidth(
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     RECT                                  *RelativeField,
  OUT    INT32                                 *Width
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Width == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "width", RelativeField, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *Width = PropValue.H2OValue.Value.U32;

  return EFI_SUCCESS;
}

EFI_STATUS
GetChildPadding (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    UINT32                                *ChildPadding
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || ChildPadding == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "child-padding", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ChildPadding = PropValue.H2OValue.Value.U32;

  return EFI_SUCCESS;
}

//
// Prompt and Value
//
EFI_STATUS
GetPromptIconImage(
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    H2O_IMAGE_INFO                        *Image
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Image == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "prompt-icon-image", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CopyMem (Image, &PropValue.Value.Image, sizeof (H2O_IMAGE_INFO));

  return EFI_SUCCESS;
}

EFI_STATUS
GetPromptIconWidth (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    UINT32                                *Width
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Width == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "prompt-icon-width", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *Width = PropValue.H2OValue.Value.U32;

  return EFI_SUCCESS;
}

EFI_STATUS
GetPromptBackgroundImage(
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    H2O_IMAGE_INFO                        *Image
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Image == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "prompt-background-image", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CopyMem (Image, &PropValue.Value.Image, sizeof (H2O_IMAGE_INFO));

  return EFI_SUCCESS;
}

EFI_STATUS
GetPromptFontSize (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    UINT32                                *FontSize
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || FontSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "prompt-font-size", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *FontSize = PropValue.H2OValue.Value.U32;

  return EFI_SUCCESS;
}


EFI_STATUS
GetPromptFontColor (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    H2O_COLOR_INFO                        *Color
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Color == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "prompt-font-color", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CopyMem (Color, &PropValue.Value.Color, sizeof (H2O_COLOR_INFO));

  return EFI_SUCCESS;
}

EFI_STATUS
GetPromptControlType (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    CHAR16                                **TypeStr
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;
  CHAR16                                       *ControlType;

  if (Panel == NULL || TypeStr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "prompt-control-type", NULL, &Property, &PropValue);
  if (Status != EFI_SUCCESS || Property == NULL) {
    return Status;
  }

  ControlType = AllocateZeroPool (sizeof (CHAR16) * (AsciiStrLen (Property->ValueStr) + 1));
  if (ControlType != NULL) {
    AsciiStrToUnicodeStr (Property->ValueStr, ControlType);
    *TypeStr = ControlType;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
GetPromptWidth (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    UINT32                                *Width
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Width == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "prompt-width", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *Width = PropValue.H2OValue.Value.U32;

  return EFI_SUCCESS;
}

EFI_STATUS
GetValueIconImage(
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    H2O_IMAGE_INFO                        *Image
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Image == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "value-icon-image", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CopyMem (Image, &PropValue.Value.Image, sizeof (H2O_IMAGE_INFO));

  return EFI_SUCCESS;
}

EFI_STATUS
GetValueIconWidth (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    UINT32                                *Width
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Width == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "value-icon-width", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *Width = PropValue.H2OValue.Value.U32;

  return EFI_SUCCESS;
}

EFI_STATUS
GetValueBackgroundImage(
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    H2O_IMAGE_INFO                        *Image
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Image == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "value-background-image", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CopyMem (Image, &PropValue.Value.Image, sizeof (H2O_IMAGE_INFO));

  return EFI_SUCCESS;
}

EFI_STATUS
GetValueFontSize (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    UINT32                                *FontSize
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || FontSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "value-font-size", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *FontSize = PropValue.H2OValue.Value.U32;

  return EFI_SUCCESS;
}

EFI_STATUS
GetValueFontColor (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    H2O_COLOR_INFO                        *Color
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;

  if (Panel == NULL || Color == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "value-font-color", NULL, &Property, &PropValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CopyMem (Color, &PropValue.Value.Color, sizeof (H2O_COLOR_INFO));

  return EFI_SUCCESS;
}

EFI_STATUS
GetValueControlType (
  IN     H2O_PANEL_INFO                        *Panel,
  IN     H2O_VFR_PROPERTY                      *VfrProperty,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  OUT    CHAR16                                **TypeStr
  )
{
  EFI_STATUS                                   Status;
  H2O_PROPERTY_INFO                            *Property;
  H2O_PROPERTY_VALUE                           PropValue;
  CHAR16                                       *ControlType;

  if (Panel == NULL || TypeStr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPropertyFromVfrAndPanel (Panel, VfrProperty, StyleType, PseudoClass, "value-control-type", NULL, &Property, &PropValue);
  if (Status != EFI_SUCCESS || Property == NULL) {
    return Status;
  }

  ControlType = AllocateZeroPool (sizeof (CHAR16) * (AsciiStrLen (Property->ValueStr) + 1));
  if (ControlType != NULL) {
    AsciiStrToUnicodeStr (Property->ValueStr, ControlType);
    *TypeStr = ControlType;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetPropertyOfHotkey (
  IN  H2O_LAYOUT_INFO                          *LayoutInfo,
  IN  EFI_GUID                                 *FormsetGuid,
  IN  H2O_FORM_ID                              FormId,
  IN  UINT32                                   HotkeyId,
  IN  CHAR8                                    *IdentifierStr,
  IN  UINT32                                   BufferSize,
  OUT UINT8                                    *Buffer
  )
{
  UINT32                                       StyleType;
  UINT32                                       PseudoClass;
  EFI_STATUS                                   Status;
  CHAR16                                       *Str;
  EFI_IMAGE_INPUT                              *Image;
  EFI_HII_VALUE                                HiiValue;
  H2O_PROPERTY_INFO                            *PropertyInfo;
  H2O_PROPERTY_VALUE                           PropertyValue;
  LAYOUT_DATABASE_PROTOCOL                     *LayoutDatabase;
  UINT32                                       ResultSize;
  UINT8                                        *ResultPtr;

  if ((LayoutInfo == NULL && FormsetGuid == NULL) || IdentifierStr == NULL || BufferSize == 0 || Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  StyleType   = H2O_IFR_STYLE_TYPE_HOTKEY;
  PseudoClass = H2O_STYLE_PSEUDO_CLASS_NORMAL;

  if (LayoutInfo != NULL) {
    Status = GetPropertyFromHotkeyList (&LayoutInfo->HotkeyListHead, HotkeyId, StyleType, PseudoClass, IdentifierStr, NULL, &PropertyInfo, &PropertyValue);
  } else {
    Status = GetPropertyFromVfrInVfcf (FormsetGuid, FormId, HotkeyId, StyleType, PseudoClass, IdentifierStr, NULL, &PropertyInfo, &PropertyValue);
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Str = NULL;

  switch (PropertyValue.Type) {

  case H2O_PROPERTY_VALUE_TYPE_BOOLEAN:
    ResultSize = sizeof (BOOLEAN);
    ResultPtr  = (UINT8 *) &PropertyValue.H2OValue.Value.Bool;
    break;

  case H2O_PROPERTY_VALUE_TYPE_STRING:
    Str = AllocateCopyPool (StrSize (PropertyValue.H2OValue.Value.Str16), PropertyValue.H2OValue.Value.Str16);
    if (Str == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    ResultSize = sizeof (CHAR16 *);
    ResultPtr  = (UINT8 *) &Str;
    break;

  case H2O_PROPERTY_VALUE_TYPE_TOKEN:
    Status = gBS->LocateProtocol (&gLayoutDatabaseProtocolGuid, NULL, (VOID **) &LayoutDatabase);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (PropertyValue.H2OValue.Type == H2O_VALUE_TYPE_STRID) {
      Str = HiiGetString (LayoutDatabase->ImagePkgHiiHandle, PropertyValue.H2OValue.Value.U16, NULL);
      if (Str == NULL) {
        return EFI_NOT_FOUND;
      }
      ResultSize = sizeof (CHAR16 *);
      ResultPtr  = (UINT8 *) &Str;
    } else if (PropertyValue.H2OValue.Type == H2O_VALUE_TYPE_IMGID) {
      Image = AllocateZeroPool (sizeof(EFI_IMAGE_INPUT));
      if (Image == NULL) {
        return EFI_NOT_FOUND;
      }
      gHiiImage->GetImage (gHiiImage, LayoutDatabase->ImagePkgHiiHandle, PropertyValue.H2OValue.Value.U16, Image);

      ResultSize = sizeof (EFI_IMAGE_INPUT *);
      ResultPtr  = (UINT8 *) &Image;
    } else {
      return EFI_UNSUPPORTED;
    }
    break;

  case H2O_PROPERTY_VALUE_TYPE_GUID:
    ResultSize = sizeof (EFI_GUID);
    ResultPtr  = (UINT8 *) &PropertyValue.H2OValue.Value.Guid;
    break;

  case H2O_PROPERTY_VALUE_TYPE_ID:
    switch (PropertyValue.H2OValue.Type) {

    case H2O_VALUE_TYPE_UINT8:
      ResultSize = MIN(sizeof (UINT8), BufferSize);
      ResultPtr  = (UINT8 *) &PropertyValue.H2OValue.Value.U8;
      break;

    case H2O_VALUE_TYPE_UINT16:
      ResultSize = MIN(sizeof (UINT16), BufferSize);
      ResultPtr  = (UINT8 *) &PropertyValue.H2OValue.Value.U16;
      break;

    case H2O_VALUE_TYPE_UINT32:
      ResultSize = MIN(sizeof (UINT32), BufferSize);
      ResultPtr  = (UINT8 *) &PropertyValue.H2OValue.Value.U32;
      break;

    case H2O_VALUE_TYPE_UINT64:
      ResultSize = MIN(sizeof (UINT64), BufferSize);
      ResultPtr  = (UINT8 *) &PropertyValue.H2OValue.Value.U64;
      break;

    default:
      return EFI_UNSUPPORTED;
    }
    break;

  case H2O_PROPERTY_VALUE_TYPE_HIIVALUE:
    Status = ConvertToHiiValue  (&PropertyValue.H2OValue, &HiiValue);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    ResultSize = sizeof (EFI_HII_VALUE);
    ResultPtr  = (UINT8 *) &HiiValue;
    break;

  default:
     return EFI_UNSUPPORTED;
  }


  if (BufferSize < ResultSize) {
    return EFI_BUFFER_TOO_SMALL;
  }

  ZeroMem (Buffer, BufferSize);
  CopyMem (Buffer, ResultPtr, ResultSize);

  return EFI_SUCCESS;
}

STATIC
VOID
FreeHotkey (
  OUT H2O_HOTKEY                               *Hotkey
  )
{
  if (Hotkey == NULL) {
    return;
  }

  if (Hotkey->KeyStr != NULL) {
    FreePool (Hotkey->KeyStr);
  }
  if (Hotkey->PromptStrTokenName != NULL) {
    FreePool (Hotkey->PromptStrTokenName);
  }
  if (Hotkey->HelpStrTokenName != NULL) {
    FreePool (Hotkey->HelpStrTokenName);
  }
}

STATIC
EFI_STATUS
GetHotkey (
  IN  H2O_LAYOUT_INFO                          *LayoutInfo,
  IN  EFI_GUID                                 *FormsetGuid,
  IN  H2O_FORM_ID                              FormId,
  IN  UINT32                                   HotkeyId,
  OUT H2O_HOTKEY                               *Hotkey
  )
{
  BOOLEAN                                      IsHotkeyEnable;

  if ((LayoutInfo == NULL && FormsetGuid == NULL) || Hotkey == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  IsHotkeyEnable = TRUE;
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "enable", sizeof (IsHotkeyEnable), (UINT8 *) &IsHotkeyEnable);
  if (!IsHotkeyEnable) {
    return EFI_NOT_FOUND;
  }

  ZeroMem (Hotkey, sizeof (H2O_HOTKEY));
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "key-string"                 , sizeof(CHAR16 *)         , (UINT8 *) &Hotkey->KeyStr);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "prompt-string-token"        , sizeof(CHAR16 *)         , (UINT8 *) &Hotkey->PromptStrTokenName);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "help-string-token"          , sizeof(CHAR16 *)         , (UINT8 *) &Hotkey->HelpStrTokenName);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "prompt-image-token"         , sizeof(EFI_IMAGE_INPUT *), (UINT8 *) &Hotkey->Image);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "visibility"                 , sizeof(BOOLEAN)          , (UINT8 *) &Hotkey->Display);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "group-id"                   , sizeof(UINT8)            , (UINT8 *) &Hotkey->GroupId);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "key-action"                 , sizeof(UINT32)           , (UINT8 *) &Hotkey->HotKeyAction);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "action-formsetguid"         , sizeof(EFI_GUID)         , (UINT8 *) &Hotkey->FormSetGuid);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "action-formid"              , sizeof(UINT16)           , (UINT8 *) &Hotkey->FormId);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "action-questionid"          , sizeof(UINT16)           , (UINT8 *) &Hotkey->QuestionId);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "action-defaultid"           , sizeof(UINT16)           , (UINT8 *) &Hotkey->HotKeyDefaultId);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "action-target-formsetguid"  , sizeof(EFI_GUID)         , (UINT8 *) &Hotkey->HotKeyTargetFormSetGuid);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "action-target-formid"       , sizeof(UINT16)           , (UINT8 *) &Hotkey->HotKeyTargetFormId);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "action-target-questionid"   , sizeof(UINT16)           , (UINT8 *) &Hotkey->HotKeyTargetQuestionId);
  GetPropertyOfHotkey (LayoutInfo, FormsetGuid, FormId, HotkeyId, "action-target-questionvalue", sizeof(EFI_HII_VALUE)    , (UINT8 *) &Hotkey->HotKeyHiiValue);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetHotKeyIdList (
  IN  EFI_GUID                                *FormsetGuid,
  IN  H2O_FORM_ID                             FormId,
  OUT UINT32                                  *HotKeyIdCount,
  OUT UINT32                                  **HotKeyId
  )
{
  EFI_STATUS                                  Status;
  H2O_FORMSET_INFO                            *FormsetInfo;
  H2O_FORM_INFO                               *FormInfo;
  UINT32                                      IdListNum;
  UINT32                                      IdListCount;
  UINT32                                      *IdList;
  LIST_ENTRY                                  *Link;
  H2O_HOTKEY_INFO                             *HotKeyInfo;

  if (FormsetGuid == NULL || HotKeyIdCount == NULL || HotKeyId == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  IdList      = NULL;
  IdListCount = 0;
  IdListNum   = 0;

  //
  // Get all hotkey ID in formset level
  //
  Status = GetFormsetLayoutByGuid (FormsetGuid, &FormsetInfo);
  if (EFI_ERROR (Status) || FormsetInfo == NULL) {
    return Status;
  }

  if (!IsListEmpty (&FormsetInfo->HotkeyListHead)) {
    Link = &FormsetInfo->HotkeyListHead;
    do {
      Link       = Link->ForwardLink;
      HotKeyInfo = H2O_HOTKEY_INFO_NODE_FROM_LINK (Link);

      if (IsValueExist (IdListCount, IdList, HotKeyInfo->HotkeyId)) {
        continue;
      }

      if (IdListCount == IdListNum) {
        IdListNum += 10;
        IdList     = ReallocatePool (IdListCount * sizeof (UINT32), IdListNum * sizeof (UINT32), IdList);
        if (IdList == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }
      }

      IdList[IdListCount++] = HotKeyInfo->HotkeyId;
    } while (!IsNodeAtEnd (&FormsetInfo->HotkeyListHead, Link));
  }

  //
  // Check hotkey in form level
  //
  FormInfo = GetFormLayoutByFormId (FormsetInfo, FormId);
  if (FormInfo != NULL && !IsListEmpty (&FormInfo->HotkeyListHead)) {
    Link = &FormInfo->HotkeyListHead;
    do {
      Link       = Link->ForwardLink;
      HotKeyInfo = H2O_HOTKEY_INFO_NODE_FROM_LINK (Link);

      if (IsValueExist (IdListCount, IdList, HotKeyInfo->HotkeyId)) {
        continue;
      }

      if (IdListCount == IdListNum) {
        IdListNum += 10;
        IdList     = ReallocatePool (IdListCount * sizeof (UINT32), IdListNum * sizeof (UINT32), IdList);
        if (IdList == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }
      }

      IdList[IdListCount++] = HotKeyInfo->HotkeyId;
    } while (!IsNodeAtEnd (&FormInfo->HotkeyListHead, Link));
  }

  if (IdList == NULL) {
    return EFI_NOT_FOUND;
  }

  BubbleSort (IdListCount, IdList);
  *HotKeyId      = IdList;
  *HotKeyIdCount = IdListCount;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetHotKeyInfo (
  IN  UINT32                                  HotKeyCount,
  IN  H2O_HOTKEY                              *HotKey,
  OUT UINT32                                  *HotKeyInfoCount,
  OUT HOT_KEY_INFO                            **HotKeyInfo
  )
{
  EFI_STATUS                                  Status;
  H2O_KEY_DESC_PROTOCOL                       *KeyDesc;
  UINT32                                      Index;
  UINT32                                      InfoCount;
  HOT_KEY_INFO                                *Info;
  HOT_KEY_INFO                                *CurrInfo;
  H2O_HOTKEY                                  *CurrHotKey;

  if (HotKeyCount == 0 || HotKey == NULL || HotKeyInfoCount == NULL || HotKeyInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gH2OKeyDescProtocolGuid, NULL, (VOID **) &KeyDesc);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Info = AllocateZeroPool ((HotKeyCount + 1) * sizeof (HOT_KEY_INFO));
  if (Info == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0, InfoCount = 0; Index < HotKeyCount; Index++) {
    CurrHotKey = &HotKey[Index];
    if (CurrHotKey->KeyStr == NULL || CurrHotKey->HotKeyAction >= HotKeyMax) {
      continue;
    }

    CurrInfo = &Info[InfoCount];

    Status = KeyDesc->KeyFromString (KeyDesc, CurrHotKey->KeyStr, &CurrInfo->KeyData);
    if (EFI_ERROR (Status)) {
      continue;
    }
    CurrInfo->Mark            = (CurrHotKey->PromptStrTokenName != NULL) ? AllocateCopyPool (StrSize (CurrHotKey->PromptStrTokenName), CurrHotKey->PromptStrTokenName) : NULL;
    CurrInfo->String          = (CurrHotKey->HelpStrTokenName   != NULL) ? AllocateCopyPool (StrSize (CurrHotKey->HelpStrTokenName)  , CurrHotKey->HelpStrTokenName)   : NULL;
    CurrInfo->HotKeyAction    = CurrHotKey->HotKeyAction;
    CurrInfo->GroupId         = CurrHotKey->GroupId;
    CurrInfo->ImageBuffer     = CurrHotKey->Image;
    CurrInfo->Display         = CurrHotKey->Display;
    CurrInfo->HotKeyDefaultId = CurrHotKey->HotKeyDefaultId;
    CopyGuid (&CurrInfo->HotKeyTargetFormSetGuid, &CurrHotKey->HotKeyTargetFormSetGuid);
    CurrInfo->HotKeyTargetFormId     = CurrHotKey->HotKeyTargetFormId;
    CurrInfo->HotKeyTargetQuestionId = CurrHotKey->HotKeyTargetQuestionId;
    CopyMem (&CurrInfo->HotKeyHiiValue, &CurrHotKey->HotKeyHiiValue, sizeof (EFI_HII_VALUE));

    InfoCount++;
  }

  if (InfoCount == 0) {
    FreePool (Info);
    return EFI_NOT_FOUND;
  }

  *HotKeyInfoCount = InfoCount;
  *HotKeyInfo      = Info;

  return EFI_SUCCESS;
}


EFI_STATUS
GetHotKeyList (
  IN  EFI_GUID                                *FormsetGuid,
  IN  H2O_FORM_ID                             FormId,
  OUT UINT32                                  *HotKeyInfoCount,
  OUT HOT_KEY_INFO                            **HotKeyInfo
  )
{
  EFI_STATUS                                  Status;
  UINT32                                      *HotKeyId;
  UINT32                                      HotKeyIdCount;
  UINT32                                      Index;
  UINT32                                      HotKeyCount;
  H2O_HOTKEY                                  *HotKey;

  if (FormsetGuid == NULL || HotKeyInfoCount == NULL || HotKeyInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get H2O_HOTKEY list from layout
  //
  Status = GetHotKeyIdList (FormsetGuid, FormId, &HotKeyIdCount, &HotKeyId);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  HotKey = AllocateZeroPool (HotKeyIdCount * sizeof (H2O_HOTKEY));
  if (HotKey == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0, HotKeyCount = 0; Index < HotKeyIdCount; Index++) {
    if (GetHotkey (NULL, FormsetGuid, FormId, HotKeyId[Index], &HotKey[HotKeyCount]) == EFI_SUCCESS) {
      HotKeyCount++;
    }
  }
  if (HotKeyCount == 0) {
    FreePool (HotKey);
    return EFI_NOT_FOUND;
  }

  Status = GetHotKeyInfo (HotKeyCount, HotKey, HotKeyInfoCount, HotKeyInfo);

  for (Index = 0; Index < HotKeyCount; Index++) {
    FreeHotkey (&HotKey[Index]);
  }
  FreePool (HotKey);
  FreePool (HotKeyId);

  return Status;
}


EFI_STATUS
GetHotKeyListByLayoutInfo (
  IN  H2O_LAYOUT_INFO                          *LayoutInfo,
  IN  EFI_GUID                                 *FormsetGuid,
  IN  H2O_FORM_ID                              FormId,
  IN  EFI_QUESTION_ID                          QuestionId,
  OUT UINT32                                   *HotKeyInfoCount,
  OUT HOT_KEY_INFO                             **HotKeyInfo
  )
{
  EFI_STATUS                                   Status;
  LIST_ENTRY                                   *Link;
  UINT32                                       Index;
  UINT32                                       LayoutHotKeyCount;
  H2O_HOTKEY                                   *LayoutHotKey;
  UINT32                                       HotKeyCount;
  H2O_HOTKEY                                   *HotKey;
  H2O_HOTKEY_INFO                              *H2OHotKeyInfo;
  EFI_GUID                                     ZeroGuid;
  BOOLEAN                                      IsFormsetMatch;
  BOOLEAN                                      IsFormMatch;
  BOOLEAN                                      IsQuestionMatch;
  UINT32                                       PreviousHotkeyId;
  UINT32                                       HotkeyId;

  if (LayoutInfo == NULL || FormsetGuid == NULL || HotKeyInfoCount == NULL || HotKeyInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get all layout hot keys in the hotkey ID order
  //
  LayoutHotKeyCount = GetListCount (&LayoutInfo->HotkeyListHead);
  if (LayoutHotKeyCount == 0) {
    return EFI_NOT_FOUND;
  }

  LayoutHotKey = AllocateZeroPool (LayoutHotKeyCount * sizeof(H2O_HOTKEY));
  HotKey       = AllocateZeroPool (LayoutHotKeyCount * sizeof(H2O_HOTKEY));
  if (LayoutHotKey == NULL || HotKey == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0, PreviousHotkeyId = 0; Index < LayoutHotKeyCount; Index++) {
    HotkeyId = (UINT32) -1;
    Link     = &LayoutInfo->HotkeyListHead;
    do {
      Link          = Link->ForwardLink;
      H2OHotKeyInfo = H2O_HOTKEY_INFO_NODE_FROM_LINK (Link);
      if (H2OHotKeyInfo->HotkeyId > PreviousHotkeyId) {
        HotkeyId = MIN (HotkeyId, H2OHotKeyInfo->HotkeyId);
      }
    } while (!IsNodeAtEnd (&LayoutInfo->HotkeyListHead, Link));

    GetHotkey (LayoutInfo, NULL, 0, HotkeyId, &LayoutHotKey[Index]);
    PreviousHotkeyId = HotkeyId;
  }

  //
  // Filter hot keys by input formset GUID, form ID and statement ID
  //
  ZeroMem (&ZeroGuid, sizeof (EFI_GUID));
  for (Index = 0, HotKeyCount = 0; Index < LayoutHotKeyCount; Index++) {
    IsFormsetMatch  = (BOOLEAN) (CompareGuid (FormsetGuid, &ZeroGuid) ||
                                 CompareGuid (&LayoutHotKey[Index].FormSetGuid, &ZeroGuid) ||
                                 CompareGuid (&LayoutHotKey[Index].FormSetGuid, FormsetGuid));
    IsFormMatch     = (BOOLEAN) (FormId == 0 ||
                                 LayoutHotKey[Index].FormId == 0 ||
                                 LayoutHotKey[Index].FormId == FormId);
    IsQuestionMatch = (BOOLEAN) (QuestionId == 0 ||
                                 LayoutHotKey[Index].QuestionId == 0 ||
                                 LayoutHotKey[Index].QuestionId == QuestionId);
    if (IsFormsetMatch && IsFormMatch && IsQuestionMatch) {
      CopyMem (&HotKey[HotKeyCount++], &LayoutHotKey[Index], sizeof(H2O_HOTKEY));
    }
    FreeHotkey (&LayoutHotKey[Index]);
  }
  FreePool (LayoutHotKey);

  if (HotKeyCount == 0) {
    FreePool (HotKey);
    return EFI_NOT_FOUND;
  }

  Status = GetHotKeyInfo (HotKeyCount, HotKey, HotKeyInfoCount, HotKeyInfo);
  FreePool (HotKey);

  return Status;
}

H2O_PANEL_INFO *
GetPanelInfo (
  IN H2O_LAYOUT_INFO                           *LayoutInfo,
  IN UINT32                                    PanelType
  )
{
  LIST_ENTRY                                   *PanelLink;
  H2O_PANEL_INFO                               *Panel;

  ASSERT (LayoutInfo != NULL);

  PanelLink = &LayoutInfo->PanelListHead;
  if (IsNull (PanelLink, PanelLink->ForwardLink)) {
    return NULL;
  }

  do {
    PanelLink = PanelLink->ForwardLink;
    Panel     = H2O_PANEL_INFO_NODE_FROM_LINK (PanelLink);

    if (Panel->PanelType == PanelType) {
      return Panel;
    }
  } while (!IsNodeAtEnd (&LayoutInfo->PanelListHead, PanelLink));

  return NULL;
}

UINT32
GetStyleTypeByOperand (
  IN UINT8                                     Operand
  )
{
  switch (Operand) {
  case EFI_IFR_ACTION_OP      : return H2O_IFR_STYLE_TYPE_ACTION;
  case EFI_IFR_CHECKBOX_OP    : return H2O_IFR_STYLE_TYPE_CHECKBOX;
  case EFI_IFR_DATE_OP        : return H2O_IFR_STYLE_TYPE_DATE;
  case EFI_IFR_REF_OP         : return H2O_IFR_STYLE_TYPE_GOTO;
  case EFI_IFR_ONE_OF_OP      : return H2O_IFR_STYLE_TYPE_ONEOF;
  case EFI_IFR_NUMERIC_OP     : return H2O_IFR_STYLE_TYPE_NUMERIC;
  case EFI_IFR_ORDERED_LIST_OP: return H2O_IFR_STYLE_TYPE_ORDEREDLIST;
  case EFI_IFR_PASSWORD_OP    : return H2O_IFR_STYLE_TYPE_PASSWORD;
  case EFI_IFR_STRING_OP      : return H2O_IFR_STYLE_TYPE_STRING;
  case EFI_IFR_TIME_OP        : return H2O_IFR_STYLE_TYPE_TIME;
  case EFI_IFR_TEXT_OP        : return H2O_IFR_STYLE_TYPE_TEXT;
  case EFI_IFR_SUBTITLE_OP    : return H2O_IFR_STYLE_TYPE_SUBTITLE;
  case EFI_IFR_RESET_BUTTON_OP: return H2O_IFR_STYLE_TYPE_RESETBUTTON;
  default : return 0;
  }
}

UINT32
GetPseudoClassByStatement (
  IN H2O_FORM_BROWSER_S                        *Statement
  )
{
  UINT32                                       PseudoClass;

  PseudoClass = H2O_STYLE_PSEUDO_CLASS_NORMAL;
  if (Statement->Selectable) {
    PseudoClass = H2O_STYLE_PSEUDO_CLASS_SELECTABLE;
  }
  if (Statement->GrayedOut) {
    PseudoClass = H2O_STYLE_PSEUDO_CLASS_GRAYOUT;
  }
  if (Statement->Locked) {
    PseudoClass = H2O_STYLE_PSEUDO_CLASS_GRAYOUT;
  }
  if (Statement->ReadOnly) {
    PseudoClass = H2O_STYLE_PSEUDO_CLASS_GRAYOUT;
  }
  return PseudoClass;
}


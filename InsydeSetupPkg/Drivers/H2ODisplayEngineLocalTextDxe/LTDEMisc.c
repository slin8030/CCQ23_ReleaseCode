/** @file
  Common functions for H2O display engine driver.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <H2ODisplayEngineLocalText.h>

/**
 Copy HII value.

 @param[out] DstHiiValue         Pointer to destination HII value
 @param[in]  SrcHiiValue         Pointer to source HII value
**/
VOID
CopyHiiValue (
  OUT EFI_HII_VALUE                           *DstHiiValue,
  IN  EFI_HII_VALUE                           *SrcHiiValue
  )
{
  if (DstHiiValue == NULL || SrcHiiValue == NULL) {
    return;
  }

  CopyMem (DstHiiValue, SrcHiiValue, sizeof (EFI_HII_VALUE));
  if (SrcHiiValue->Buffer != NULL && SrcHiiValue->BufferLen != 0) {
    DstHiiValue->Buffer = AllocateCopyPool (SrcHiiValue->BufferLen, SrcHiiValue->Buffer);
  }
}

CHAR16 *
PrintFormattedNumber (
  IN H2O_FORM_BROWSER_Q                       *Question
  )
{
  INT64                                       Value;
  EFI_HII_VALUE                               *QuestionValue;
  CHAR16                                      FormattedNumber[21];

  QuestionValue = &Question->HiiValue;

  Value = (INT64) QuestionValue->Value.u64;
  switch (Question->Flags & EFI_IFR_DISPLAY) {

  case EFI_IFR_DISPLAY_INT_DEC:
    switch (QuestionValue->Type) {

    case EFI_IFR_NUMERIC_SIZE_1:
      Value = (INT64) ((INT8) QuestionValue->Value.u8);
      break;

    case EFI_IFR_NUMERIC_SIZE_2:
      Value = (INT64) ((INT16) QuestionValue->Value.u16);
      break;

    case EFI_IFR_NUMERIC_SIZE_4:
      Value = (INT64) ((INT32) QuestionValue->Value.u32);
      break;

    case EFI_IFR_NUMERIC_SIZE_8:
    default:
      break;
    }

    if (Value < 0) {
      Value = -Value;
      UnicodeSPrint (FormattedNumber, sizeof (FormattedNumber), L"-%ld", Value);
    } else {
      UnicodeSPrint (FormattedNumber, sizeof (FormattedNumber), L"%ld", Value);
    }
    break;

  case EFI_IFR_DISPLAY_UINT_DEC:
    UnicodeSPrint (FormattedNumber, sizeof (FormattedNumber), L"%ld", Value);

    break;

  case EFI_IFR_DISPLAY_UINT_HEX:
    UnicodeSPrint (FormattedNumber, sizeof (FormattedNumber), L"%s%lx", HEX_NUMBER_PREFIX_STRING, Value);
    break;

  default:
    return NULL;
  }

  return AllocateCopyPool (StrSize (FormattedNumber), FormattedNumber);
}

CHAR16 *
CreateString (
  IN UINT32                                   StringLength,
  IN CHAR16                                   StringChar
  )
{
  CHAR16                                      *String;
  UINT32                                      Index;

  String = AllocateZeroPool ((StringLength + 1) * sizeof (CHAR16));
  if (String == NULL) {
    return NULL;
  }

  for (Index = 0; Index < StringLength; Index++) {
    String[Index] = StringChar;
  }

  return String;
}

BOOLEAN
IsValidValueStr (
  IN CHAR16                                   *ValueStr,
  IN BOOLEAN                                  IsHex
  )
{
  if (ValueStr == NULL) {
    return FALSE;
  }

  while (*ValueStr != CHAR_NULL) {
    if ((IsHex  && !IsHexChar (*ValueStr)) ||
        (!IsHex && !IsDecChar (*ValueStr))) {
      return FALSE;
    }
    ValueStr++;
  }

  return TRUE;
}

BOOLEAN
IsPointOnField (
  IN RECT                                     *ControlAbsField,
  IN INT32                                    Column,
  IN INT32                                    Row
  )
{
  if (ControlAbsField != NULL &&
      IN_RANGE (Row   , ControlAbsField->top , ControlAbsField->bottom) &&
      IN_RANGE (Column, ControlAbsField->left, ControlAbsField->right)) {
    return TRUE;
  }

  return FALSE;
}


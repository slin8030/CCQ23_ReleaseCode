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

#include <H2OKeyDesc.h>

typedef struct {
  CHAR16                                      *KeyShiftStr;
  UINT32                                      KeyShiftState;
} H2O_KEY_SHIFT_STATE_INFO;

typedef struct {
  CHAR16                                      *ScanCodeStr;
  UINT16                                      ScanCode;
} H2O_SCAN_CODE_INFO;

typedef struct {
  CHAR16                                      *UnicodeCharStr;
  CHAR16                                      UnicodeChar;
} H2O_UNICODE_CHAR_INFO;

H2O_KEY_SHIFT_STATE_INFO                      mKeyShiftStateInfo[] = {
                                                {L"ctrl"  , EFI_RIGHT_CONTROL_PRESSED | EFI_LEFT_CONTROL_PRESSED},
                                                {L"shift" , EFI_RIGHT_SHIFT_PRESSED   | EFI_LEFT_SHIFT_PRESSED  },
                                                {L"alt"   , EFI_RIGHT_ALT_PRESSED     | EFI_LEFT_ALT_PRESSED    },
                                                {L"logo"  , EFI_RIGHT_LOGO_PRESSED    | EFI_LEFT_LOGO_PRESSED   },
                                                {L"menu"  , EFI_MENU_KEY_PRESSED                                },
                                                {L"sysreq", EFI_SYS_REQ_PRESSED                                 },
                                                };

H2O_SCAN_CODE_INFO                            mScanCodeInfo[] = {
                                                {L"esc"  , SCAN_ESC      },
                                                {L"ins"  , SCAN_INSERT   },
                                                {L"del"  , SCAN_DELETE   },
                                                {L"up"   , SCAN_UP       },
                                                {L"down" , SCAN_DOWN     },
                                                {L"right", SCAN_RIGHT    },
                                                {L"left" , SCAN_LEFT     },
                                                {L"home" , SCAN_HOME     },
                                                {L"end"  , SCAN_END      },
                                                {L"pgup" , SCAN_PAGE_UP  },
                                                {L"pgdn" , SCAN_PAGE_DOWN},
                                                {L"f1"   , SCAN_F1       },
                                                {L"f2"   , SCAN_F2       },
                                                {L"f3"   , SCAN_F3       },
                                                {L"f4"   , SCAN_F4       },
                                                {L"f5"   , SCAN_F5       },
                                                {L"f6"   , SCAN_F6       },
                                                {L"f7"   , SCAN_F7       },
                                                {L"f8"   , SCAN_F8       },
                                                {L"f9"   , SCAN_F9       },
                                                {L"f10"  , SCAN_F10      },
                                                {L"f11"  , SCAN_F11      },
                                                {L"f12"  , SCAN_F12      },
                                                };

H2O_UNICODE_CHAR_INFO                         mUnicodeCharInfo[] = {
                                                {L"enter"    , CHAR_CARRIAGE_RETURN},
                                                {L"backspace", CHAR_BACKSPACE      },
                                                {L"tab"      , CHAR_TAB            },
                                                {L"space"    , ' '                 },
                                                };

/**
  Check if the input character is a visible character

  @param[in] UnicodeChar     Unicode character

  @retval TRUE               The input character is a visible character
  @retval FALSE              The input character is not a visible character
**/
STATIC
BOOLEAN
IsVisibleChar (
  IN CHAR16                                   UnicodeChar
  )
{
  return (BOOLEAN) (UnicodeChar >= ' ' && UnicodeChar <= '~');
}

/**
 Converts the unicode character of the string from uppercase to lowercase.

 @param[in, out] Str                     String to be converted
**/
STATIC
VOID
ToLower (
  IN OUT CHAR16                               *Str
  )
{
  CHAR16      *Ptr;

  for (Ptr = Str; *Ptr != L'\0'; Ptr++) {
    if (*Ptr >= L'A' && *Ptr <= L'Z') {
      *Ptr = (CHAR16) (*Ptr - L'A' + L'a');
    }
  }
}

/**
 Append key string into target string.

 @param[in]      KeyStr         Pointer to key string
 @param[in, out] Str            Double pointer to appended string

 @retval EFI_SUCCESS            Append string successfully.
 @retval EFI_INVALID_PARAMETER  KeyStr or AppendedStr is NULL.
**/
STATIC
EFI_STATUS
AppendKeyStr (
  IN     CHAR16                               *KeyStr,
  IN OUT CHAR16                               **AppendedStr
  )
{
  CHAR16                                      *OrgStr;
  CHAR16                                      *NewStr;

  if (KeyStr == NULL || AppendedStr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  OrgStr = *AppendedStr;
  if (OrgStr == NULL) {
    NewStr = AllocateCopyPool (StrSize (KeyStr), KeyStr);
  } else {
    NewStr = CatSPrint (NULL, L"%s-%s", OrgStr, KeyStr);
    FreePool (OrgStr);
  }
  *AppendedStr = NewStr;

  return EFI_SUCCESS;
}

/**
  Converts from standard EFI key data structures to strings.

  @param[in]      This             Pointer to current instance of this protocol.
  @param[out]     String           The key data user want to change.
  @param[in]      KeyData          The string has been changed.

  @retval EFI_SUCCESS            Function completed successfully.
  @retval EFI_OUT_OF_RESOURCES   Unable to allocate required resources.
*/
EFI_STATUS
EFIAPI
H2OKeyDescKeyToString (
  IN  H2O_KEY_DESC_PROTOCOL                   *This,
  IN  EFI_KEY_DATA                            *KeyData,
  OUT CHAR16                                  **String
  )
{
  CHAR16                                      *ResultStr;
  CHAR16                                      CharStr[2];
  UINT32                                      KeyShiftState;
  UINTN                                       Index;

  if (This == NULL || KeyData == NULL || String == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (KeyData->Key.ScanCode != SCAN_NULL && KeyData->Key.UnicodeChar != CHAR_NULL) {
    return EFI_NO_MAPPING;
  }

  ResultStr     = NULL;
  KeyShiftState = KeyData->KeyState.KeyShiftState;

  if (((KeyShiftState & EFI_SHIFT_STATE_VALID) == EFI_SHIFT_STATE_VALID) && (KeyShiftState != EFI_SHIFT_STATE_VALID)) {
    for (Index = 0; Index < sizeof (mKeyShiftStateInfo) / sizeof (H2O_KEY_SHIFT_STATE_INFO); Index++) {
      if ((mKeyShiftStateInfo[Index].KeyShiftState & KeyShiftState) != 0) {
        AppendKeyStr (mKeyShiftStateInfo[Index].KeyShiftStr, &ResultStr);
      }
    }
  }

  if (KeyData->Key.ScanCode != SCAN_NULL) {
    for (Index = 0; Index < sizeof (mScanCodeInfo) / sizeof (H2O_SCAN_CODE_INFO); Index++) {
      if (mScanCodeInfo[Index].ScanCode == KeyData->Key.ScanCode) {
        AppendKeyStr (mScanCodeInfo[Index].ScanCodeStr, &ResultStr);
        break;
      }
    }
    if (Index == sizeof (mScanCodeInfo) / sizeof (H2O_SCAN_CODE_INFO)) {
      if (ResultStr != NULL) {
        FreePool (ResultStr);
      }
      return EFI_NO_MAPPING;
    }
  } else if (KeyData->Key.UnicodeChar != CHAR_NULL) {
    for (Index = 0; Index < sizeof (mUnicodeCharInfo) / sizeof (H2O_UNICODE_CHAR_INFO); Index++) {
      if (mUnicodeCharInfo[Index].UnicodeChar == KeyData->Key.UnicodeChar) {
        AppendKeyStr (mUnicodeCharInfo[Index].UnicodeCharStr, &ResultStr);
        break;
      }
    }
    if (Index == sizeof (mUnicodeCharInfo) / sizeof (H2O_UNICODE_CHAR_INFO)) {
      if (IsVisibleChar (KeyData->Key.UnicodeChar)) {
        CharStr[0] = KeyData->Key.UnicodeChar;
        CharStr[1] = CHAR_NULL;
        AppendKeyStr (CharStr, &ResultStr);
      } else {
        if (ResultStr != NULL) {
          FreePool (ResultStr);
        }
        return EFI_NO_MAPPING;
      }
    }
  }

  if (ResultStr == NULL) {
    return EFI_NO_MAPPING;
  }

  *String = ResultStr;
  return EFI_SUCCESS;
}

/**
  Converts from strings to standard EFI key data structures.

  @param[in]      This             Pointer to current instance of this protocol.
  @param[in]      String           The string user want to change.
  @param[out]     KeyData          The key data has been changed.

  @retval EFI_SUCCESS            Function completed successfully.
  @retval EFI_INVALID_PARAMETER  Invalid input string.
*/
EFI_STATUS
EFIAPI
H2OKeyDescKeyFromString (
  IN  H2O_KEY_DESC_PROTOCOL                   *This,
  IN  CHAR16                                  *String,
  OUT EFI_KEY_DATA                            *KeyData
  )
{

  EFI_KEY_DATA                                ResultKeyData;
  CHAR16                                      *StringPtr;
  CHAR16                                      *DashCharPtr;
  CHAR16                                      KeyStr[30];
  UINTN                                       Index;

  if (This == NULL || String == NULL || KeyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (*String == CHAR_NULL) {
    return EFI_NO_MAPPING;
  }

  ZeroMem (&ResultKeyData, sizeof (ResultKeyData));
  ResultKeyData.KeyState.KeyShiftState  = EFI_SHIFT_STATE_VALID;
  ResultKeyData.KeyState.KeyToggleState = EFI_TOGGLE_STATE_VALID;

  //
  // Check the key string before '-' character to parsing key shift state.
  //
  StringPtr = String;
  while (TRUE) {
    DashCharPtr = StrStr (StringPtr, L"-");
    if (DashCharPtr == NULL || DashCharPtr == StringPtr) {
      break;
    }

    ZeroMem (KeyStr, sizeof (KeyStr));
    CopyMem (KeyStr, StringPtr, ((UINTN) (DashCharPtr - StringPtr)) * sizeof (CHAR16));
    ToLower (KeyStr);
    StringPtr = DashCharPtr + 1;

    for (Index = 0; Index < sizeof (mKeyShiftStateInfo) / sizeof (H2O_KEY_SHIFT_STATE_INFO); Index++) {
      if (StrCmp (mKeyShiftStateInfo[Index].KeyShiftStr, KeyStr) == 0) {
        ResultKeyData.KeyState.KeyShiftState |= mKeyShiftStateInfo[Index].KeyShiftState;
        break;
      }
    }
    if (Index == sizeof (mKeyShiftStateInfo) / sizeof (H2O_KEY_SHIFT_STATE_INFO)) {
      return EFI_NO_MAPPING;
    }
  }
  if (*StringPtr == CHAR_NULL) {
    return EFI_NO_MAPPING;
  }

  //
  // Check last key string
  //
  if (StrLen (StringPtr) == 1) {
    if (IsVisibleChar (*StringPtr)) {
      ResultKeyData.Key.UnicodeChar = *StringPtr;
      goto Done;
    }
  } else {
    StrCpy (KeyStr, StringPtr);
    ToLower (KeyStr);

    for (Index = 0; Index < sizeof (mKeyShiftStateInfo) / sizeof (H2O_KEY_SHIFT_STATE_INFO); Index++) {
      if (StrCmp (mKeyShiftStateInfo[Index].KeyShiftStr, KeyStr) == 0) {
        ResultKeyData.KeyState.KeyShiftState |= mKeyShiftStateInfo[Index].KeyShiftState;
        goto Done;
      }
    }

    for (Index = 0; Index < sizeof (mScanCodeInfo) / sizeof (H2O_SCAN_CODE_INFO); Index++) {
      if (StrCmp (mScanCodeInfo[Index].ScanCodeStr, KeyStr) == 0) {
        ResultKeyData.Key.ScanCode = mScanCodeInfo[Index].ScanCode;
        goto Done;
      }
    }

    for (Index = 0; Index < sizeof (mUnicodeCharInfo) / sizeof (H2O_UNICODE_CHAR_INFO); Index++) {
      if (StrCmp (mUnicodeCharInfo[Index].UnicodeCharStr, KeyStr) == 0) {
        ResultKeyData.Key.UnicodeChar = mUnicodeCharInfo[Index].UnicodeChar;
        goto Done;
      }
    }
  }
  return EFI_NO_MAPPING;

Done:
  CopyMem (KeyData, &ResultKeyData, sizeof(EFI_KEY_DATA));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
H2OKeyDescEntryPoint (
  IN     EFI_HANDLE                            ImageHandle,
  IN     EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  EFI_STATUS                                   Status;
  H2O_KEY_DESC_PROTOCOL                        *H2OKeyDescProtocol;

  //
  // Install H2O_KEY_DESC_PROTOCOL
  //
  H2OKeyDescProtocol = (H2O_KEY_DESC_PROTOCOL*) AllocateZeroPool (sizeof (H2O_KEY_DESC_PROTOCOL));
  if (H2OKeyDescProtocol == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  H2OKeyDescProtocol->Size           = (UINT32) sizeof (H2O_KEY_DESC_PROTOCOL);
  H2OKeyDescProtocol->KeyToString    = H2OKeyDescKeyToString;
  H2OKeyDescProtocol->KeyFromString  = H2OKeyDescKeyFromString;

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gH2OKeyDescProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  H2OKeyDescProtocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}


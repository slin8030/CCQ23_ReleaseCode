/** @file

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

#ifndef _LTDE_MISC_H_
#define _LTDE_MISC_H_

VOID
CopyHiiValue (
  IN EFI_HII_VALUE                            *DstHiiValue,
  IN EFI_HII_VALUE                            *SrcHiiValue
  );

CHAR16 *
PrintFormattedNumber (
  IN H2O_FORM_BROWSER_Q                       *Question
  );

CHAR16 *
CreateString (
  IN UINT32                                   StringLength,
  IN CHAR16                                   StringChar
  );

BOOLEAN
IsValidValueStr (
  IN CHAR16                                   *ValueStr,
  IN BOOLEAN                                  IsHex
  );

BOOLEAN
IsPointOnField (
  IN RECT                                     *ControlAbsField,
  IN INT32                                    Column,
  IN INT32                                    Row
  );

#endif

/** @file
  Interface definition for ConsoleLib.

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CONSOLE_LIBRARY_H_
#define _CONSOLE_LIBRARY_H_

//
// Console Library API
//
UINT32
EFIAPI
ConsoleLibGetGlyphWidth (
  IN CHAR16                               UnicodeChar
  );

UINTN
GetStringDisplayWidth (
  IN CHAR16                               *String
  );

UINTN
GetStringWidth (
  CHAR16                                  *String
  );

#endif

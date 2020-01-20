/** @file
  Self Debug Library

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SELF_DEBUG_LIB_H_
#define _SELF_DEBUG_LIB_H_

#include <Library/DebugLib.h>

BOOLEAN
EFIAPI
DbgAssertEnabled (
  VOID
  );

VOID
EFIAPI
DbgAssert (
  IN CONST CHAR8  *FileName,
  IN UINTN        LineNumber,
  IN CONST CHAR8  *Description
  );

VOID
EFIAPI
DbgPrint (
  IN  UINTN        ErrorLevel,
  IN  CONST CHAR8  *Format,
  ...
  );

VOID
DbgDumpHex (
  IN UINTN        Indent,
  IN UINTN        Offset,
  IN UINTN        DataSize,
  IN VOID         *UserData
  );

// #define SELF_DEBUG

#ifdef SELF_DEBUG

#ifdef MDEPKG_NDEBUG

#undef MDEPKG_NDEBUG

#ifdef DEBUG
#undef DEBUG
#endif

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef _ASSERT
#undef _ASSERT
#endif

#ifdef ASSERT_EFI_ERROR
#undef ASSERT_EFI_ERROR
#endif

#define _ASSERT(Expression)  DbgAssert (__FILE__, __LINE__, #Expression)

#define DEBUG(Expression)    DbgPrint Expression

#define ASSERT(Expression)        \
  do {                            \
    if (DbgAssertEnabled ()) {  \
      if (!(Expression)) {        \
        _ASSERT (Expression);     \
      }                           \
    }                             \
  } while (FALSE)

#define ASSERT_EFI_ERROR(StatusParameter)                                                    \
        do {                                                                                 \
          if (DebugAssertEnabled ()) {                                                       \
            if (EFI_ERROR (StatusParameter)) {                                               \
              DEBUG ((EFI_D_ERROR, "\nASSERT_EFI_ERROR (Status = %r)\n", StatusParameter));  \
              _ASSERT (!EFI_ERROR (StatusParameter));                                        \
            }                                                                                \
          }                                                                                  \
        } while (FALSE)

#endif // MDEPKG_NDEBUG

#define DEBUG_DUMP_HEX(Indent, Offset, DataSize, UserData)  DbgDumpHex (Indent, Offset, DataSize, UserData)

#else  // SELF_DEBUG

#define DEBUG_DUMP_HEX(Indent, Offset, DataSize, UserData)

#endif  // SELF_DEBUG

#endif // _SELF_DEBUG_LIB_H_



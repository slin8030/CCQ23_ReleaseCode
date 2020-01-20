/** @file
  Monitor header file.

  Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/
#ifndef __PRINT_UEFIRW_H__
#define __PRINT_UEFIRW_H__

#include <Protocol/SimpleFileSystem.h>

#define NARROW_CHAR        0xFFF0
#define WIDE_CHAR          0xFFF1
#define NON_BREAKING_CHAR  0xFFF2

#define CHARACTER_NUMBER_FOR_VALUE  30

#define LEFT_JUSTIFY   0x01
#define PREFIX_SIGN    0x02
#define PREFIX_BLANK   0x04
#define COMMA_TYPE     0x08
#define LONG_TYPE      0x10
#define PREFIX_ZERO    0x20

typedef CHAR16  CHAR_W;
#define STRING_W(_s)   L##_s

UINTN
EfiValueToHexStr (
  IN  OUT CHAR16  *Buffer,
  IN  UINT64      Value,
  IN  UINTN       Flags,
  IN  UINTN       Width
  );


STATIC
CHAR_W *
GetFlagsAndWidth (
  IN  CHAR_W      *Format, 
  OUT UINTN       *Flags, 
  OUT UINTN       *Width,
  IN OUT  VA_LIST *Marker
  );


UINTN
SPrint (
  OUT CHAR_W        *Buffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR_W  *Format,
  ...
  );

UINTN
EFIAPI
VSPrint (
  OUT CHAR_W        *StartOfBuffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR_W  *FormatString,
  IN  VA_LIST       Marker
  );

UINTN
EfiStatusToString (
  IN EFI_STATUS   Status,
  OUT CHAR_W      *Buffer,
  IN  UINTN       BufferSize
  );

STATIC
UINTN
GuidToString (
  IN  EFI_GUID  *Guid,
  IN  CHAR_W    *Buffer,
  IN  UINTN     BufferSize
  );

STATIC
UINTN
TimeToString (
  IN EFI_TIME   *Time,
  OUT CHAR_W    *Buffer,
  IN  UINTN     BufferSize
  );

UINTN
EfiValueToString (
  IN  OUT CHAR16  *Buffer,
  IN  INT64       Value,
  IN  UINTN       Flags,
  IN  UINTN       Width
  );

UINTN
PrintInternal (
  IN UINTN                            Column,
  IN UINTN                            Row,
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *Out,
  IN CHAR16                           *Fmt,
  IN VA_LIST                          Args
  );

UINTN
EFIAPI
ConsolePrint (
  IN CHAR16   *Fmt,
  ...
  );

UINTN
PrintString (
  IN CHAR16       *String
  );

UINTN
PrintChar (
  CHAR16       Character
  );

UINTN
EFIAPI
PrintAt (
  IN UINTN     Column,
  IN UINTN     Row,
  IN CHAR16    *Fmt,
  ...
  );

UINTN
PrintStringAt (
  IN UINTN     Column,
  IN UINTN     Row,
  IN CHAR16    *String
  );

UINTN
PrintCharAt (
  IN UINTN     Column,
  IN UINTN     Row,
  IN CHAR16       Character
  );

#endif

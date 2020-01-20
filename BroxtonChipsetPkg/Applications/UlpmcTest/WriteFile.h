/** @file
  Screen header file.

  Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/
#ifndef __WRITE_FILE_H__
#define __WRITE_FILE_H__

#include <Protocol/SimpleFileSystem.h>

EFI_STATUS 
InitFileSystem(
  IN  EFI_HANDLE   ImageHandle,
  IN  CHAR16       *name,
  IN  UINT64       MODE,  
  OUT EFI_FILE     **FileHandle
  );

EFI_STATUS
WriteString2File(
  IN  EFI_FILE     *Fp,
  IN  CHAR8        *String
  );

VOID
MyUnicodeStrToAsciiStr (
  IN  CONST CHAR16  *Source,
  OUT       CHAR8   *Destination
  );

VOID
MyAsciiToUnicodeStrStr (
  IN  CONST CHAR8   *Source,
  OUT       CHAR16  *Destination
  );

EFI_STATUS
ReadStringLine(
  IN  EFI_FILE     *Fp,
  IN  UINT16       MaxNum,  
  OUT CHAR8        *String
  );

#endif

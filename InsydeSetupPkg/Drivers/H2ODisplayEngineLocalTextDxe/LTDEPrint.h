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

#ifndef _LTDE_PRINT_H_
#define _LTDE_PRINT_H_

EFI_STATUS
DEConOutOutputString (
  IN CHAR16                                 *String
  );

EFI_STATUS
DEConOutQueryModeWithoutModeNumer (
  OUT UINT32                                *Columns,
  OUT UINT32                                *Rows
  );

EFI_STATUS
DEConOutSetAttribute (
  IN UINTN                                  Attribute
  );

EFI_STATUS
DEConOutSetNarrowAttribute (
  VOID
  );

EFI_STATUS
DEConOutSetCursorPosition (
  IN UINTN                                  Column,
  IN UINTN                                  Row
  );

EFI_STATUS
DEConOutEnableCursor (
  IN BOOLEAN                                Visible
  );

EFI_STATUS
InitializeVirtualKb (
  RECT                                        *ControlAbsField
  );

EFI_STATUS
ShutdownVirtualKb (
  VOID
  );


BOOLEAN
IsVirtualKbVisibled (
  );

EFI_STATUS
GetScreenField (
  OUT RECT                                    *ScreenField
  );

#endif

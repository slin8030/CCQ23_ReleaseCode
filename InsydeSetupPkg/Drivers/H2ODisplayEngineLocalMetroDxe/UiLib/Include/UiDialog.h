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

#ifndef _UI_DIALOG_H
#define _UI_DIALOG_H

typedef struct _UI_DIALOG UI_DIALOG;
typedef struct _UI_DIALOG_CLASS UI_DIALOG_CLASS;

UI_DIALOG_CLASS *
EFIAPI
GetDialogClass (
  VOID
  );

struct _UI_DIALOG {
  UI_FRAME                      Frame;
  HINSTANCE                     Instance;
  HWND                          ParentWnd;
  CHAR16                        *XmlBuffer;
  WNDPROC                       Proc;
  LPARAM                        Param;
  BOOLEAN                       Running;
  INTN                          Ret;
  BOOLEAN                       CloseDlgWhenTouchOutside;
};

struct _UI_DIALOG_CLASS {
  UI_CONTROL_CLASS              ParentClass;
};

#endif

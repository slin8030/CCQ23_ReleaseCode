/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    SmmScriptSave.h
    
Abstract:

  This is an implementation of the BootScript at run time.  

--*/

#ifndef _RUNTIME_SCRIPT_SAVE_H
#define _RUNTIME_SCRIPT_SAVE_H

#include "Efi.h"
#include "EfiBootScript.h"


typedef EFI_PHYSICAL_ADDRESS     EFI_SMM_SCRIPT_TABLE;

EFI_STATUS 
SmmBootScriptCreateTable (
  IN OUT EFI_SMM_SCRIPT_TABLE    *ScriptTable,
  IN UINTN                       Type
  );

EFI_STATUS 
SmmBootScriptWrite (
  IN OUT EFI_SMM_SCRIPT_TABLE    *ScriptTable,
  IN UINTN                       Type,
  IN UINT16                      OpCode,
  ...
  );

EFI_STATUS
SmmBootScriptCloseTable (
  IN EFI_SMM_SCRIPT_TABLE        ScriptTableBase,
  IN EFI_SMM_SCRIPT_TABLE        ScriptTablePtr,
  IN UINTN                       Type
  );

#endif

/** @file
  Debug Print Error Level library instance that retrieves the current error
  level from PcdDebugPrintErrorLevel.  This generic library instance does not
  support the setting of the global debug print error level mask for the platform.

  Copyright (c) 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Library/DebugPrintErrorLevelLib.h>
#include <Library/PcdLib.h>

#include <Library/PeiServicesTablePointerLib.h>
#include <Library/VariableLib.h>
#include <Guid/DebugMask.h>

/**
  Returns the debug print error level mask for the current module.

  @return  Debug print error level mask for the current module.

**/
UINT32
EFIAPI
GetDebugPrintErrorLevel (
  VOID
  )
{
//[-start-160923-IB11270164-remove]//
//  EFI_STATUS                      Status;
//  UINTN                           Size;
//  UINT64                          GlobalErrorLevel;
//[-end-160923-IB11270164-remove]//
  UINT32                          DebugPrintErrorLevel;
  CONST EFI_PEI_SERVICES          **PeiServices;

  PeiServices = GetPeiServicesTablePointer ();

  //
  // Retrieve the current debug print error level mask from PcdDebugPrintErrorLevel.
  //
  DebugPrintErrorLevel = PcdGet32 (PcdDebugPrintErrorLevel);

//[-start-160923-IB11270164-remove]//
//  Size = sizeof (GlobalErrorLevel);
//  Status = CommonGetVariable (
//             DEBUG_MASK_VARIABLE_NAME,
//             &gEfiGenericVariableGuid,
//             &Size,
//             &GlobalErrorLevel
//             );
//  if (!EFI_ERROR (Status)) {
//    DebugPrintErrorLevel = (UINT32)GlobalErrorLevel;
//  }
//[-end-160923-IB11270164-remove]//

  return DebugPrintErrorLevel;
}

/**
  Sets the global debug print error level mask fpr the entire platform.

  @param   ErrorLevel     Global debug print error level.

  @retval  TRUE           The debug print error level mask was sucessfully set.
  @retval  FALSE          The debug print error level mask could not be set.

**/
BOOLEAN
EFIAPI
SetDebugPrintErrorLevel (
  UINT32  ErrorLevel
  )
{
  //
  // This library uinstance does not support setting the global debug print error
  // level mask.
  //
  return FALSE;
}


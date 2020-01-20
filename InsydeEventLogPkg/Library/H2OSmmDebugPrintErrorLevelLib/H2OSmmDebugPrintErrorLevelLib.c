/** @file
 H2OSmmDebugPrintErrorLevelLib Lib functions which relate with connect the device

;******************************************************************************
;* Copyright (c) 2016 - 2018, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

*/

#include <Library/DebugPrintErrorLevelLib.h>
#include <Library/PcdLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DebugCfgToErrLevelLib.h>
#include <Protocol/SmmVariable.h>
#include <Guid/DebugMask.h>
#include <Guid/DebugMessageConfigUtilHii.h>


///
/// Global variable that is set to TRUE after the first attempt is made to 
/// retrieve the global error level mask through the EFI Varibale Services.
/// This variable prevents the EFI Variable Services from being called for
/// every DEBUG() macro.
///
BOOLEAN           mGlobalErrorLevelInitialized = FALSE;
UINT32            mDebugPrintErrorLevel        = 0;

/**
  
  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS   The constructor completed successfully.
  @retval Other value   The constructor did not complete successfully.

**/
EFI_STATUS
EFIAPI
SmmDebugPrintErrorLevelLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                                 Status;
  EFI_TPL                                    CurrentTpl;
  UINTN                                      Size;
  DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION    DebugMsgCfg;

//[-start-180518-IB08400596-modify]//
  mDebugPrintErrorLevel = PcdGet32 (PcdDebugPrintErrorLevel);
//[-end-180518-IB08400596-modify]//

  //
  // Make sure the TPL Level is low enough for EFI Variable Services to be called
  //
  CurrentTpl = SystemTable->BootServices->RaiseTPL (TPL_HIGH_LEVEL);
  SystemTable->BootServices->RestoreTPL (CurrentTpl);
  if (CurrentTpl <= TPL_CALLBACK) {
    //
    // Attempt to retrieve the global debug print error level mask from the 
    // EFI Variable
    //
    Size = sizeof (DebugMsgCfg);
    Status = SystemTable->RuntimeServices->GetVariable (
               H2O_DEBUG_MESSAGE_CONFIG_UTIL_VARSTORE_NAME, 
               &gH2ODebugMessageConfigUtilVarstoreGuid,
               NULL, 
               &Size, 
               &DebugMsgCfg
               );
    if (Status != EFI_NOT_AVAILABLE_YET) {
      //
      // If EFI Variable Services are available, then set a flag so the EFI
      // Variable will not be read again by this module.
      //
      mGlobalErrorLevelInitialized = TRUE;
      if (!EFI_ERROR (Status)) {
        //
        // If the EFI Varible exists, then set this module's module's mask to
        // the global debug print error level mask value.
        //
//[-start-160219-IB08400332-modify]//
        mDebugPrintErrorLevel = DebugCfgToErrLevel((VOID*)&DebugMsgCfg);  
//[-end-160219-IB08400332-modify]//
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  
  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS   The destructor completed successfully.
  @retval Other value   The destructor did not complete successfully.

**/
EFI_STATUS
EFIAPI
SmmDebugPrintErrorLevelLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}

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
  EFI_STATUS                                 Status;
  UINTN                                      Size;
  DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION    DebugMsgCfg;
  EFI_SMM_VARIABLE_PROTOCOL                  *SmmVariable = NULL;
  
//[-start-180206-IB08400567-modify]//
  if (!mGlobalErrorLevelInitialized && (gSmst != NULL)) {
//[-end-180206-IB08400567-modify]//
    // Attempt to retrieve the global debug print error level mask from the 
    // EFI Variable
    //  
    Status = gSmst->SmmLocateProtocol (&gEfiSmmVariableProtocolGuid, NULL, (VOID **)&SmmVariable);
    if (EFI_ERROR (Status)) {
//[-start-180518-IB08400596-modify]//
      return PcdGet32 (PcdDebugPrintErrorLevel);
//[-end-180518-IB08400596-modify]//
    }
    
    Size = sizeof (DebugMsgCfg);
    Status = SmmVariable->SmmGetVariable (
                            H2O_DEBUG_MESSAGE_CONFIG_UTIL_VARSTORE_NAME, 
                            &gH2ODebugMessageConfigUtilVarstoreGuid, 
                            NULL,
                            &Size, 
                            &DebugMsgCfg
                            );
    if (!EFI_ERROR (Status)) {
      //
      // If EFI Variable Services are available, then set a flag so the EFI
      // Variable will not be read again by this module.
      //
      mGlobalErrorLevelInitialized = TRUE;
//[-start-160219-IB08400332-modify]//
      mDebugPrintErrorLevel = DebugCfgToErrLevel((VOID*)&DebugMsgCfg);  
//[-end-160219-IB08400332-modify]//
    }
  }

  //
  // Return the current mask value for this module.
  //
  return mDebugPrintErrorLevel;
}

/**
  Sets the global debug print error level mask fpr the entire platform.
  
  @param   ErrorLevel     Global debug print error level
  
  @retval  TRUE           The debug print error level mask was sucessfully set.
  @retval  FALSE          The debug print error level mask could not be set.

**/
BOOLEAN
EFIAPI
SetDebugPrintErrorLevel (
  UINT32  ErrorLevel
  )
{
  EFI_STATUS                                 Status;
  UINTN                                      Size;
  DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION    DebugMsgCfg;
  EFI_SMM_VARIABLE_PROTOCOL                  *SmmVariable = NULL;
  
  if (gSmst != NULL) {
    Status = gSmst->SmmLocateProtocol (&gEfiSmmVariableProtocolGuid, NULL, (VOID **)&SmmVariable);
    if (EFI_ERROR (Status)) {
      return FALSE;
    }
    //
    // Attempt to store the Debug Message Configuration in an EFI Variable
    //
    Size = sizeof (DebugMsgCfg);
    Status = SmmVariable->SmmGetVariable (
                            H2O_DEBUG_MESSAGE_CONFIG_UTIL_VARSTORE_NAME, 
                            &gH2ODebugMessageConfigUtilVarstoreGuid,  
                            NULL,
                            &Size, 
                            &DebugMsgCfg
                            );
    if (!EFI_ERROR (Status)) { 
      //
      // If the EFI Varible exists, then set this module's module's mask to
      // the global debug print error level mask value.
      //
//[-start-160219-IB08400332-modify]//
      ErrLevelToDebugCfg (ErrorLevel, (VOID*)&DebugMsgCfg);
//[-end-160219-IB08400332-modify]//

      Status = SmmVariable->SmmSetVariable(
                              H2O_DEBUG_MESSAGE_CONFIG_UTIL_VARSTORE_NAME, 
                              &gH2ODebugMessageConfigUtilVarstoreGuid, 
                              (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS),
                              Size,
                              &DebugMsgCfg
                              );
        if (!EFI_ERROR (Status)) {
          //
          // If the EFI Variable was updated, then update the mask value for this 
          // module and return TRUE.
          //
          mGlobalErrorLevelInitialized = TRUE;    
          mDebugPrintErrorLevel = ErrorLevel;
          return TRUE;
        }
    }
  }
  
  //
  // Return FALSE since the EFI Variable could not be updated.
  //
  return FALSE;
}

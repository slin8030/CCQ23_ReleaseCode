/** @file
 SMM Chipset Services Library.

 This file contains only one function that is SmmCsSvcResetSystem().
 The function SmmCsSvcResetSystem() use chipset services to reset system.
  
***************************************************************************
* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************

*/

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <ScAccess.h>
//[-start-160914-IB07400784-add]//
#include <Protocol/SmmVariable.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SmmServicesTableLib.h>
#include <ChipsetSetupConfig.h>
//[-end-160914-IB07400784-add]//

/**
 Resets the system.

 @param[in]         ResetType           The type of reset to perform.
                                
 @retval            EFI_UNSUPPORTED     ResetType is invalid.
 @retval            others              Error occurs
*/
EFI_STATUS
EFIAPI
ResetSystem (
  IN EFI_RESET_TYPE   ResetType
  )
{
  UINT8       InitialData;
  UINT8       OutputData;
//[-start-160914-IB07400784-add]//
  UINT8                         FullSystemConfig[FixedPcdGet32(PcdSetupConfigSize)];
  UINTN                         VarSize;
  EFI_SMM_VARIABLE_PROTOCOL     *SmmVariable;
  EFI_STATUS                    Status;
  CHIPSET_CONFIGURATION         SetupVariable;
  UINT8                         ResetSelect;

  ResetSelect = 0xFF;
  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmVariableProtocolGuid,
                    NULL,
                    (VOID **)&SmmVariable
                    );
  if (!EFI_ERROR (Status)) {
    VarSize = PcdGet32(PcdSetupConfigSize);
    Status = SmmVariable->SmmGetVariable(
      SETUP_VARIABLE_NAME,
      &gSystemConfigurationGuid,
      NULL,
      &VarSize,
      FullSystemConfig
      );
    if (!EFI_ERROR (Status)) {
      CopyMem (&SetupVariable, FullSystemConfig, sizeof(CHIPSET_CONFIGURATION));
      ResetSelect = SetupVariable.ResetSelect;
    }
  }
//[-end-160914-IB07400784-add]//

  switch (ResetType) {
  case EfiResetWarm:
    InitialData  = V_PCH_RST_CNT_SOFTSTARTSTATE;
    OutputData   = V_PCH_RST_CNT_HARDRESET;
//[-start-160914-IB07400784-add]//
    if (ResetSelect != 0xFF) {
      OutputData   = ResetSelect;
    }
//[-end-160914-IB07400784-add]//
    break;

  case EfiResetCold:
    InitialData  = V_PCH_RST_CNT_HARDSTARTSTATE;
//[-start-160914-IB07400784-modify]//
    //
    // APL BIOS spec (Doc#559811, Vol2, Rev1.1, Section 2.1.1)
    //
    // Triggers for this event include, but not limited to, 
    // writing a value of 0xE to the Reset Control Register at IO port 0xCF9, or 
    // entering certain system power management states
    //
    OutputData   = V_PCH_RST_CNT_FULLRESET;
//[-end-160914-IB07400784-modify]//
    break;

  default:
    return EFI_UNSUPPORTED;
  }

  IoWrite8 ((UINTN)R_PCH_RST_CNT, (UINT8)InitialData);
  IoWrite8 ((UINTN)R_PCH_RST_CNT, (UINT8)OutputData);

  //
  // Given we should have reset getting here would be bad
  //
  CpuDeadLoop ();
  return EFI_DEVICE_ERROR;
}

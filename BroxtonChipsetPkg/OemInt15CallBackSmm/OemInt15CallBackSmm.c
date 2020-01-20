/** @file
  Provide INT 15 SMM Callback function

;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <OemInt15CallBackSmm.h>
#include <Library/SmmOemSvcChipsetLib.h>
//[-start-160914-IB07400784-add]//
#include <ScAccess.h>
#include <Protocol/SmmVariable.h>
#include <Library/BaseMemoryLib.h>
#include <ChipsetSetupConfig.h>
//[-end-160914-IB07400784-add]//

//
// Sample for install multiple Int15 function
//
UINT16 mINT15FunctionNumber[] = {
  0x9999,
  0xFFFF   // End of table
};

EFI_STATUS
EFIAPI
InstallOemInt15CallBackFunction (
  IN CONST EFI_GUID                       *Protocol,
  IN VOID                                 *Interface,
  IN EFI_HANDLE                           Handle
  )
{
  EFI_STATUS                            Status;
  EFI_SMM_INT15_SERVICE_PROTOCOL        *SmmInt15Service = NULL;  
  UINT32                                Index = 0;
  UINT16                                *OemInt15VbiosFunctionHook;
  UINT16                                Size;

  OemInt15VbiosFunctionHook = NULL;
  Size = 0;

  Status = gSmst->SmmLocateProtocol (&gEfiSmmInt15ServiceProtocolGuid, NULL, (VOID **)&SmmInt15Service);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR(Status)) { 
    return Status;
  }
  
  //
  // Install multiple Int15 function to OemInt15CallBack
  //
  for (Index = 0; mINT15FunctionNumber[Index] !=0xFFFF; Index++) {
    
    Status = SmmInt15Service->InstallInt15ProtocolInterface (
                                SmmInt15Service,
                                mINT15FunctionNumber[Index],
                                OemInt15CallBack,
                                NULL
                                );

    if (Status == EFI_ALREADY_STARTED) { 
      //
      // use new callback function to replace original callback function
      //
      Status = SmmInt15Service->ReinstallInt15ProtocolInterface (
                                  SmmInt15Service,
                                  mINT15FunctionNumber[Index],
                                  OemInt15CallBack,
                                  NULL
                                  );
      return Status;
    }
  }

  Status = OemSvcGetOemInt15VbiosFunctionlist (&OemInt15VbiosFunctionHook, &Size);
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcChipsetLib OemSvcGetOemInt15VbiosFunctionlist, Status : %r\n", Status));
  if (Status == EFI_MEDIA_CHANGED){
    for (Index = 0; Index < Size; Index++) {
      Status = SmmInt15Service->InstallInt15ProtocolInterface (
                                  SmmInt15Service,
                                  OemInt15VbiosFunctionHook[Index],
                                  OemInt15CallBack,
                                  NULL
                                  );
      if (Status == EFI_ALREADY_STARTED) {
        //
        // use new callback function to replace original one
        //
        Status = SmmInt15Service->ReinstallInt15ProtocolInterface (
                                    SmmInt15Service,
                                    OemInt15VbiosFunctionHook[Index],
                                    OemInt15CallBack,
                                    NULL
                                    );
        return Status;
      }
    }
  }
  
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
OemINT15EntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Initializes the OEM INT15 SMM Dispatcher

Arguments:
  
  ImageHandle   - Pointer to the loaded image protocol for this driver
  SystemTable   - Pointer to the EFI System Table

Returns:
  Status        - EFI_SUCCESS

--*/
{
  EFI_STATUS                            Status;
  EFI_SMM_INT15_SERVICE_PROTOCOL        *SmmInt15Service = NULL;  
  EFI_EVENT                             InstallOemInt15CallBackFunctionEvent;
  
  Status = gSmst->SmmLocateProtocol (&gEfiSmmInt15ServiceProtocolGuid, NULL, (VOID **)&SmmInt15Service);

  if (!EFI_ERROR(Status)) {
    InstallOemInt15CallBackFunction (&gEfiSmmInt15ServiceProtocolGuid, NULL, NULL);
  } else {
    Status = gSmst->SmmRegisterProtocolNotify (
      &gEfiSmmInt15ServiceProtocolGuid,
      InstallOemInt15CallBackFunction,
      &InstallOemInt15CallBackFunctionEvent
      );
    ASSERT_EFI_ERROR (Status);
  }
  
  return EFI_SUCCESS;
}

//[-start-160914-IB07400784-add]//
#ifdef WARM_RESET_HOOK_BY_SCU_OS_RESET
#define CpuDeadLoop() {volatile int i = 1; while(i);}
  
EFI_STATUS
EFIAPI
ResetSystem (
  IN EFI_RESET_TYPE   ResetType
  )
{
  UINT8                         InitialData;
  UINT8                         OutputData;
  UINT8                         FullSystemConfig[FixedPcdGet32(PcdSetupConfigSize)];
  UINTN                         VarSize;
  EFI_SMM_VARIABLE_PROTOCOL     *SmmVariable;
  EFI_STATUS                    Status;
  CHIPSET_CONFIGURATION         SetupVariable;
  UINT8                         ResetSelect;
  UINT8                         Data8;

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

  switch (ResetType) {
  case EfiResetWarm:
    InitialData  = V_PCH_RST_CNT_SOFTSTARTSTATE;
    OutputData   = V_PCH_RST_CNT_HARDRESET;
    if (ResetSelect != 0xFF) {
      OutputData   = ResetSelect;
    }
    break;

  case EfiResetCold:
    InitialData  = V_PCH_RST_CNT_HARDSTARTSTATE;
    //
    // APL BIOS spec (Doc#559811, Vol2, Rev1.1, Section 2.1.1)
    //
    // Triggers for this event include, but not limited to, 
    // writing a value of 0xE to the Reset Control Register at IO port 0xCF9, or 
    // entering certain system power management states
    //
    OutputData   = V_PCH_RST_CNT_FULLRESET;
    break;

  default:
    return EFI_UNSUPPORTED;
  }
  
  Data8 = InitialData;
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, R_PCH_RST_CNT, 1, &Data8);
  Data8 = OutputData;
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, R_PCH_RST_CNT, 1, &Data8);

  //
  // Given we should have reset getting here would be bad
  //
  CpuDeadLoop ();
  return EFI_DEVICE_ERROR;
}
#endif
//[-end-160914-IB07400784-add]//

STATIC
VOID
OemInt15CallBack (
  IN  OUT EFI_IA32_REGISTER_SET   *CpuRegs, 
  IN      VOID                    *Context
  )
/*++

Routine Description:

  Oem INT15 callback 
  
Arguments:

  Handle      - Image handle
  Context     - Context

Returns:

  None

--*/
{
  EFI_STATUS                      Status;
  UINT8                           Data8;

  if (!CpuRegs->X.AX) {
    return;
  }

  Status = OemSvcVbiosHookCallBack (CpuRegs->X.AX, CpuRegs, Context);
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcChipsetLib OemSvcVbiosHookCallBack, Status : %r\n", Status));
  if (Status == EFI_SUCCESS) {
    return;
  }

  switch (CpuRegs->X.AX) {
  case 0x9999:
    if (CpuRegs->X.BX == 0xdead) {
      //
      // Hook CSM reboot
      //
//[-start-160914-IB07400784-modify]//
#ifdef WARM_RESET_HOOK_BY_SCU_OS_RESET
      ResetSystem(EfiResetWarm);
#endif
//[-end-160914-IB07400784-modify]//
      Data8 = 0x6;
      gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, 0xCF9, 1, &Data8);
      CpuRegs->X.AX = 1;
    }
    break;

  default:
    //
    // to do 
    //
    break;
  }

  return;
}

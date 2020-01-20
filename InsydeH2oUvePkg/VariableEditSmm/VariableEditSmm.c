/** @file
  Content file contains function definitions for Variable Edit Smm driver

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
**/

#include "VariableEditSmm.h"

//
// Factory default Support Function
//
STATIC
IHISI_REGISTER_TABLE
H2OUVE_SIMSERVICE_REGISTER_TABLE[] = {
  //
  // AH=52h, H2O UEFI variable edit SMI service - Confirm the legality of the variables.
  //
  { UveVariableConfirm, "S52OemH2oUveSmiSvc0", H2oUveSmiService}, \

  //
  // AH=53h, H2O UEFI variable edit SMI service - Boot information-related services.
  //
  { UveBootInfoService, "S53OemH2oUveSmiSvc1", H2oUveSmiService}
};

H2OUVE_SERVICE_FUNCTION_MAP_TABLE       mFunctionMapTable[] = {
//    mRax        vRax        mRcx        vRcx        mRsi        vRsi        mRdi        vRdi        pFunc
  { _AH__MASK_, _UVE__52H_, DONT__CARE, DONT__CARE, DONT__CARE, DONT__CARE, DONT__CARE, DONT__CARE, SetupCallbackSyncCallbackIhisi },

  { _AH__MASK_, _UVE__53H_, _ECX_MASK_, _ECX__01H_, DONT__CARE, DONT__CARE, DONT__CARE, DONT__CARE, GetBootTypeOrderInformation },
  { _AH__MASK_, _UVE__53H_, _ECX_MASK_, _ECX__02H_, DONT__CARE, DONT__CARE, DONT__CARE, DONT__CARE, SetBootTypeOrderInformation },
  { _AH__MASK_, _UVE__53H_, _ECX_MASK_, _ECX__03H_, DONT__CARE, DONT__CARE, DONT__CARE, DONT__CARE, GetCurrentBootTypeInformation },
  { _AH__MASK_, _UVE__53H_, _ECX_MASK_, _ECX__04H_, DONT__CARE, DONT__CARE, DONT__CARE, DONT__CARE, SetCurrentBootTypeInformation },
  { _AH__MASK_, _UVE__53H_, _ECX_MASK_, _ECX__05H_, DONT__CARE, DONT__CARE, DONT__CARE, DONT__CARE, GetCurrentBootTypeName },
};

H2O_IHISI_PROTOCOL                      *mH2OIhisi           = NULL;

/**
  Implement IHISI SPEC. AH=52h, H2O UEFI variable edit SMI service - Confirm the legality of the variables.
  Implement IHISI SPEC. AH=53h, H2O UEFI variable edit SMI service - Boot information-related services.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
H2oUveSmiService (
  VOID
)
{
  UINT64                          Rax     = 0;
  UINT64                          Rcx     = 0;
  UINT64                          Rsi     = 0;
  UINT64                          Rdi     = 0;


  UINTN                           Index = 0;
  H2OUVE_SERVICE_FUNCTION_POINT   FuncPtr = NULL;

  Rax = (UINT64) (UINTN) mH2OIhisi->ReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RAX);
  Rcx = (UINT64) (UINTN) mH2OIhisi->ReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  Rsi = (UINT64) (UINTN) mH2OIhisi->ReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  Rdi = (UINT64) (UINTN) mH2OIhisi->ReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);

  for (Index = 0; Index < ARRAY_SIZE (mFunctionMapTable); ++Index) {

    if ( (CHECK_CONDITION (Rax, mFunctionMapTable[Index].MaskRax, mFunctionMapTable[Index].ValueRax)) &&
         (CHECK_CONDITION (Rcx, mFunctionMapTable[Index].MaskRcx, mFunctionMapTable[Index].ValueRcx)) &&
         (CHECK_CONDITION (Rsi, mFunctionMapTable[Index].MaskRsi, mFunctionMapTable[Index].ValueRsi)) &&
         (CHECK_CONDITION (Rdi, mFunctionMapTable[Index].MaskRdi, mFunctionMapTable[Index].ValueRdi))
       ) {
      FuncPtr = mFunctionMapTable[Index].FuncPtr;
      break;
    }

  }

  if (FuncPtr == NULL) {
    return EFI_UNSUPPORTED;
  }

  return FuncPtr (Rax, Rcx, Rsi, Rdi);
}

/**
  Register IHISI sub function if SubFuncTable CmdNumber/AsciiFuncGuid define in PcdIhisiRegisterTable list.

  @param[out] SubFuncTable        Pointer to ihisi register table.
  @param[out] TableCount          SubFuncTable count

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
RegisterIhisiSubFunction (
  IN IHISI_REGISTER_TABLE         *SubFuncTable,
  IN UINT16                        TableCount
  )
{
  EFI_STATUS                    Status;
  PCD_IHISI_REGISTER_TABLE     *PcdRegisterTable;
  UINT16                        PcdRegisterTableCount;
  UINT16                        PcdRegisterTableIndex;
  UINT16                        SubFuncTableIndex;
  UINT8                         PcdCommand;
  CHAR8                         String[20];
  Status = EFI_UNSUPPORTED;
  PcdRegisterTable = (PCD_IHISI_REGISTER_TABLE *)PcdGetPtr (PcdH2oUveIhisiRegisterTable);
  PcdRegisterTableCount = (UINT16)FixedPcdGetPtrSize (PcdH2oUveIhisiRegisterTable) / sizeof (PcdRegisterTable[0]);
  //
  //  Register IHISI sub function if subfunction CmdNumber/AsciiFuncGuid define in PcdIhisiRegisterTable list.
  //
  for (SubFuncTableIndex = 0; SubFuncTableIndex < TableCount; SubFuncTableIndex++) {
    for (PcdRegisterTableIndex = 0; PcdRegisterTableIndex < PcdRegisterTableCount; PcdRegisterTableIndex++) {
      PcdCommand = PcdRegisterTable[PcdRegisterTableIndex].CmdNumber;
      CopyMem (String, PcdRegisterTable[PcdRegisterTableIndex].FuncSignature, 20);
      if (SubFuncTable[SubFuncTableIndex].CmdNumber == PcdRegisterTable[PcdRegisterTableIndex].CmdNumber) {
        if (AsciiStrCmp (SubFuncTable[SubFuncTableIndex].FuncSignature,PcdRegisterTable[PcdRegisterTableIndex].FuncSignature) == 0) {
          Status = mH2OIhisi->RegisterCommand (SubFuncTable[SubFuncTableIndex].CmdNumber ,
                                               SubFuncTable[SubFuncTableIndex].IhisiFunction,
                                               PcdRegisterTable[PcdRegisterTableIndex].Priority);
          if (EFI_ERROR(Status)) {
            DEBUG ((EFI_D_ERROR, "IHISI command :0x%X, priority : 0x%X, that already has a registered function\n", SubFuncTable[SubFuncTableIndex].CmdNumber,PcdRegisterTable[PcdRegisterTableIndex].Priority));
          }
          break;
        }
      }
    }
  }
  return Status;
}

/**
  Register IHISI sub function for H2oUvePkg.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
InstallH2oUveIhisiServices (
  VOID
  )
{
  EFI_STATUS                            Status;
  IHISI_REGISTER_TABLE                  *SubFuncTable;
  UINT16                                TableCount;

  Status = gSmst->SmmLocateProtocol (
                    &gH2OIhisiProtocolGuid,
                    NULL,
                    (VOID **) &mH2OIhisi
                    );

  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_INFO, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return Status;
  }


  SubFuncTable = H2OUVE_SIMSERVICE_REGISTER_TABLE;
  TableCount = sizeof(H2OUVE_SIMSERVICE_REGISTER_TABLE)/sizeof(H2OUVE_SIMSERVICE_REGISTER_TABLE[0]);
  Status = RegisterIhisiSubFunction (SubFuncTable, TableCount);
  if (EFI_ERROR(Status)) {
    DEBUG ( (EFI_D_INFO, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
  }

  return Status;
}

/**
  Check status of function disabled variable, install IHISI sub function and initialize SysPassword Driver.

 @param [in]   ImageHandle      Pointer to the loaded image protocol for this driver
 @param [in]   SystemTable      Pointer to the EFI System Table

 @retval EFI_SUCCESS
 @retval EFI_UNSUPPORTED        VarEditFunDis variable is exist and has been seted TRUE.
 @return Other                  Error occurred in this function.

**/
EFI_STATUS
EFIAPI
SetupCallbackSyncEntryPoint (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                                Status;
  VARIABLE_EDIT_FUNCTION_DISABLE_SETTING    VarEditFunDis = {0};
  UINTN                                     VarEditFunDisSize = sizeof (VARIABLE_EDIT_FUNCTION_DISABLE_SETTING);
  EFI_SMM_VARIABLE_PROTOCOL                 *SmmVariable;

  DEBUG ((EFI_D_INFO, "->SetupCallbackSyncEntryPoint\n"));

  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmVariableProtocolGuid,
                    NULL,
                    (VOID **)&SmmVariable
                    );

  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_INFO, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return Status;
  }
  Status = SmmVariable->SmmGetVariable (
                          H2OUVE_VARSTORE_NAME,
                          &gH2oUveVarstoreGuid,
                          NULL,
                          &VarEditFunDisSize,
                          &VarEditFunDis
                          );

  if ( VarEditFunDisSize != sizeof (VARIABLE_EDIT_FUNCTION_DISABLE_SETTING)) {
    DEBUG ( (EFI_D_INFO, "Please check sizeof (VARIABLE_EDIT_FUNCTION_DISABLE_SETTING) in Variable Store."));
  }
  if ( (!EFI_ERROR (Status)) &&
       (VarEditFunDis.VariableEditFunDis == TRUE) &&
       (VarEditFunDisSize == sizeof (VARIABLE_EDIT_FUNCTION_DISABLE_SETTING))
     ) {
    DEBUG ( (EFI_D_INFO, "[%a:%d] VariableEditor Function Disabled.\n", __FUNCTION__, __LINE__));
    return EFI_UNSUPPORTED;
  }

  Status = InstallH2oUveIhisiServices ();

  return Status;
}


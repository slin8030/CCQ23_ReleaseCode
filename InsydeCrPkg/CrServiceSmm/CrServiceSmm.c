/** @file
  Console redirection SMM drvier
;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/IoLib.h>

#include <Protocol/SmmCpu.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmReadyToLock.h>

#include "CrServiceSmm.h"

//
// Driver
//
#define SMI_PORT         PcdGet16 (PcdSoftwareSmiPort)
#define SW_SMI_CRS       PcdGet8 (PcdH2OCrSoftwareSmi)
#define REG_COUNT        5

EFI_SMM_CPU_PROTOCOL     *mSmmCpu = NULL;
CR_REG_ARRAY             SaveReg[REG_COUNT] = {0};


/**
  This fucntion uses to read saved CPU double word register by CPU index

  @param[In]   RegisterNum   Register number which want to get
  @param[In]   CpuIndex      CPU index number to get register.
  @param[Out]  RegisterData  pointer to output register data

  @retval EFI_SUCCESS   Read double word register successfully
  @return Other         Any error occured while disabling all secure boot SMI functions successful.

**/
EFI_STATUS
GetDwordRegisterByCpuIndex (
  IN  EFI_SMM_SAVE_STATE_REGISTER       RegisterNum,
  IN  UINTN                             CpuIndex,
  OUT UINT32                            *RegisterData
  )
{
  UINTN                       Width;

  Width = sizeof (UINT32);
  *RegisterData = 0;

  return mSmmCpu->ReadSaveState (
                    mSmmCpu,
                    Width,
                    RegisterNum,
                    CpuIndex,
                    RegisterData
                    );
}

/**
  This fucntion uses to write saved CPU double word register by CPU index

  @param[In]   RegisterNum   Register number which want to get
  @param[In]   CpuIndex      CPU index number to get register.
  @param[Out]  RegisterData  pointer to output register data

  @retval EFI_SUCCESS   Read double word register successfully
  @return Other         Any error occured while disabling all secure boot SMI functions successful.

**/
EFI_STATUS
SetDwordRegisterByCpuIndex (
  IN  EFI_SMM_SAVE_STATE_REGISTER       RegisterNum,
  IN  UINTN                             CpuIndex,
  IN  UINT32                            *RegisterData
  )
{
  UINTN                       Width;

  Width = sizeof (UINT32);

  return mSmmCpu->WriteSaveState (
                    mSmmCpu,
                    Width,
                    RegisterNum,
                    CpuIndex,
                    RegisterData
                    );
}

EFI_STATUS
DoCrSmmFunction (
  CR_REG_ARRAY  *Reg,
  UINTN         CmdCount,
  UINTN         CpuIndex
  )
{
  while (CmdCount != 0) {
    switch (Reg->Ecx.Reg8) {
    case MEM_W:
      MmioWrite8((UINTN)Reg->Ebx.Reg32 , (UINT8)Reg->Ecx.Reg.Data);
      break;

    case MEM_R:
      Reg->Eax.Reg8 = MmioRead8((UINTN)Reg->Ebx.Reg32);
      break;

    case IO_W:
      IoWrite8((UINTN)Reg->Ebx.Reg32, (UINT8)Reg->Ecx.Reg.Data);
      break;

    case IO_R:
      Reg->Eax.Reg8 = IoRead8((UINTN)Reg->Ebx.Reg32);
      break;

    default:
      break;
    }

    if (Reg->Ecx.Reg8 == MEM_R || Reg->Ecx.Reg8 == IO_R) {
      SetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RAX, CpuIndex, &Reg->Eax.Reg32);
    }
    CmdCount--;
  }

return EFI_SUCCESS;
}

EFI_STATUS
CrServiceSmiCallback (
  IN EFI_HANDLE            DispatchHandle,
  IN CONST VOID            *Context         OPTIONAL,
  IN OUT VOID              *CommBuffer      OPTIONAL,
  IN OUT UINTN             *CommBufferSize  OPTIONAL
  )
{
  EFI_STATUS               Status;
  UINTN                    Index;
  UINTN                    CpuIndex;
  CR_REG_ARRAY             *Reg;
  static BOOLEAN           SaveMode  = FALSE;
  static UINTN             SaveCount = 0;

  //
  // Select the variable to save current REG.
  //
  if (SaveMode) {
    if (SaveCount >= REG_COUNT) {
      //
      // replace last cmd.
      //
      SaveCount = REG_COUNT - 1;
      ASSERT (SaveCount < REG_COUNT);
    }
    Reg = &SaveReg[SaveCount];
  } else {
    Reg = &SaveReg[0];
  }

  for (Index = 0; Index < gSmst->NumberOfCpus; Index++) {
    Status = GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RAX, Index, &Reg->Eax.Reg32);
    ASSERT_EFI_ERROR(Status);
    Status = GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RDX, Index, &Reg->Edx.Reg32);
    ASSERT_EFI_ERROR(Status);
    //
    // Find out which CPU triggered PnP SMI
    //
    if ((Reg->Eax.CrsSmi == SW_SMI_CRS) && (Reg->Edx.SmiPort == PcdGet16 (PcdSoftwareSmiPort))) {
      //
      // Cpu found!
      //
      break;
    }
  }
  if (Index == gSmst->NumberOfCpus) {
    //
    // Error out due to CPU not found
    //
    return EFI_NOT_FOUND;
  }

  //
  // ESI : Signatures
  // AL  : Soft SMI vale
  // EBX : MMIO Address
  // CL  : Function (see. enum SMI_FUNC)
  // CH  : Data
  // DX  : SmiPort = 0xB2
  //
  CpuIndex = Index;
  Status = GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RAX, CpuIndex, &Reg->Eax.Reg32);
  ASSERT_EFI_ERROR(Status);
  Status = GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RBX, CpuIndex, &Reg->Ebx.Reg32);
  ASSERT_EFI_ERROR(Status);
  Status = GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RCX, CpuIndex, &Reg->Ecx.Reg32);
  ASSERT_EFI_ERROR(Status);
  Status = GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RSI, CpuIndex, &Reg->Esi.Reg32);
  ASSERT_EFI_ERROR(Status);

  if (Reg->Esi.Signature  != SIGNATURE_32 ('$', 'C', 'R', 'S')) {
    return EFI_INVALID_PARAMETER;
  }

  if (SaveMode == FALSE) {

    if (Reg->Ecx.Reg8 == QUEUE_CMD) {
      //
      // Start Queue MODE.
      //
      SaveMode  = TRUE;
      SaveCount = 0;
      return EFI_SUCCESS;
    } else {
      //
      //  Do one CMD.
      //
      return DoCrSmmFunction (Reg, 1, CpuIndex);
    }

  } else if (SaveMode == TRUE) {

    if (Reg->Ecx.Reg8 == QUEUE_CMD) {
      //
      // Do the all of CMDs in the Queue.
      //
      SaveMode = FALSE;
      return DoCrSmmFunction (Reg, SaveCount, CpuIndex);

    } else {
      //
      // Store the CMD into the Queue.
      //
      SaveCount++;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CrServiceSmmEntryPoint (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE    *SystemTable
  )
{
  EFI_STATUS                      Status;
  EFI_HANDLE                      Handle;
  EFI_SMM_SW_REGISTER_CONTEXT     SwContext;
  EFI_SMM_SW_DISPATCH2_PROTOCOL   *SwDispatch;

  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmSwDispatch2ProtocolGuid,
                    NULL,
                    (VOID **) &SwDispatch
                    );
  if (EFI_ERROR (Status)) {
     ASSERT_EFI_ERROR (Status);
    return Status;
  }

  //
  // Software SMI for CR services callback function
  //
  Handle = NULL;
  SwContext.SwSmiInputValue = SW_SMI_CRS;
  Status = SwDispatch->Register (
                         SwDispatch,
                         CrServiceSmiCallback,
                         &SwContext,
                         &Handle
                         );
  if (EFI_ERROR (Status)) {
     ASSERT_EFI_ERROR (Status);
     return Status;
  }

  //
  // Locate Smm Cpu protocol for Cpu save state manipulation
  //
  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmCpuProtocolGuid,
                    NULL,
                    (VOID **)&mSmmCpu
                    );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}


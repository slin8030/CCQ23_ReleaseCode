/** @file
  TCO Watchdog Function.

@copyright
  Copyright (c) 1999 - 2015 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by the
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
  This file contains an 'Intel Peripheral Driver' and is uniquely
  identified as "Intel Reference Module" and is licensed for Intel
  CPUs and chipsets under the terms of your license agreement with
  Intel or your vendor. This file may be modified by the user, subject
  to additional terms of the license agreement.

@par Specification Reference:
**/

//[-start-151229-IB03090424-modify]//
#include "Watchdog.h"
#include "SmmPlatform.h"

EFI_SMM_CPU_PROTOCOL        *mSmmCpu;
EFI_SMM_VARIABLE_PROTOCOL   *mSmmVariable;

UINT32
ReadCpuReg32 (
  EFI_SMM_SAVE_STATE_REGISTER  Register,
  UINTN                        CpuIndex
  )
{
  EFI_STATUS                  Status;
  UINT32                      Uint32;

  Status = mSmmCpu->ReadSaveState (
                      mSmmCpu,
                      sizeof (UINT32),
                      Register,
                      CpuIndex,
                      &Uint32
                      );

  return Uint32;
}

VOID
EnableWatchdogCallback (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
  )
/*++

Routine Description:

  When a watchdog event happens, install a dummy handler to allow 2nd TCO event after SMI.

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                     Status;
  OFFLINE_CPU_CONTEXT_LIST       CpuContextList;
  UINT32                         Data32;
  UINTN                          CpuIndex = 0;
  UINTN                          CpuSaveMemAddr = 0;
  UINT8                          Index;

  DEBUG((EFI_D_INFO, "EnableWatchdogCallback - Start\n"));

  Status = gSmst->SmmLocateProtocol (&gEfiSmmCpuProtocolGuid, NULL, (VOID **) &mSmmCpu);
  if(EFI_ERROR(Status)){
    return;
  }

  SetMem(&CpuContextList, sizeof(CpuContextList), 0x0);
  CpuContextList.NumOfCpus = (UINT32)gSmst->NumberOfCpus;
  CpuContextList.CpuArchitecture = CONTEXT_i386;
  //
  //Get CPU context
  //
  for(CpuIndex = 0; CpuIndex < CpuContextList.NumOfCpus; CpuIndex ++){
    //Flags
    CpuContextList.CpuContext[CpuIndex].Flag = CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS;

    //SEG context
    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_GS,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].SegGs = Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_FS,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].SegFs = Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_ES,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].SegEs = Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_DS,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].SegDs = Data32;

    //Integer context
    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RDI,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Edi= Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RSI,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Esi= Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RBX,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Ebx= Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RDX,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Edx= Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RCX,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Ecx= Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RAX,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Eax= Data32;

    //Control context
    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RBP,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Ebp= Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RIP,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Eip= Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_CS,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].SegCs= Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RFLAGS,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Eflags= Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RSP,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Esp = Data32;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_SS,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].SegSs = Data32;
  }

  for (Index = 0; Index < 4; Index++) {
    IoWrite8(CmosIo_72, EFI_CMOS_CRASHDUMP_ADDR_0 + Index);
    CpuSaveMemAddr |= (UINTN)(IoRead8(CmosIo_73) << (8*Index));
  }
  DEBUG ((EFI_D_INFO, "CpuSaveMemAddr: %X\n", CpuSaveMemAddr));

  CopyMem((VOID *)CpuSaveMemAddr, &CpuContextList, sizeof(CpuContextList));

  IoWrite8(CmosIo_72, EFI_CMOS_CRASHDUMP_TRIGGERED);
  IoWrite8(CmosIo_73, 0x5A);

  AsmWbinvd();
}
//[-end-151229-IB03090424-modify]//

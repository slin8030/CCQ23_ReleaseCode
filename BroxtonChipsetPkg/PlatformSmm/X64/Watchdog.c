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


UINT16
ReadCpuReg16 (
  EFI_SMM_SAVE_STATE_REGISTER  Register,
  UINTN                        CpuIndex
  )
{
  EFI_STATUS                  Status;
  UINT16                      Uint16;

  Status = mSmmCpu->ReadSaveState (
                      mSmmCpu,
                      sizeof (UINT16),
                      Register,
                      CpuIndex,
                      &Uint16
                      );

  return Uint16;
}

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

UINT64
ReadCpuReg64 (
  EFI_SMM_SAVE_STATE_REGISTER  Register,
  UINTN                        CpuIndex
  )
{
  EFI_STATUS                  Status;
  UINT64                      Uint64;

  Status = mSmmCpu->ReadSaveState (
                      mSmmCpu,
                      sizeof (UINT64),
                      Register,
                      CpuIndex,
                      &Uint64
                      );

  return Uint64;
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
  UINT16                         Data16;
  UINT32                         Data32;
  UINT64                         Data64;
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
  CpuContextList.CpuArchitecture = CONTEXT_AMD64;

  //
  //Get CPU context
  //
  for (CpuIndex = 0; CpuIndex < CpuContextList.NumOfCpus; CpuIndex ++) {
    //Flags
    CpuContextList.CpuContext[CpuIndex].Flag = CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS;

    //SEG context
    Data16 = ReadCpuReg16(EFI_SMM_SAVE_STATE_REGISTER_GS,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].SegGs = Data16;

    Data16 = ReadCpuReg16(EFI_SMM_SAVE_STATE_REGISTER_FS,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].SegFs = Data16;

    Data16 = ReadCpuReg16(EFI_SMM_SAVE_STATE_REGISTER_ES,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].SegEs = Data16;

    Data16 = ReadCpuReg16(EFI_SMM_SAVE_STATE_REGISTER_DS,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].SegDs = Data16;

    //Integer context
    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_RDI,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Rdi= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_RSI,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Rsi= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_RBX,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Rbx= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_RDX,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Rdx= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_RCX,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Rcx= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_RAX,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Rax= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_R8,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].R8= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_R9,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].R9= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_R10,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].R10= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_R11,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].R11= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_R12,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].R12= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_R13,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].R13= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_R14,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].R14= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_R15,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].R15= Data64;

    //Control context
    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_RBP,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Rbp= Data64;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_RIP,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Rip= Data64;

    Data16 = ReadCpuReg16(EFI_SMM_SAVE_STATE_REGISTER_CS,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].SegCs= Data16;

    Data32 = ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RFLAGS,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].EFlags= Data32;

    Data64 = ReadCpuReg64(EFI_SMM_SAVE_STATE_REGISTER_RSP,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].Rsp = Data64;

    Data16 = ReadCpuReg16(EFI_SMM_SAVE_STATE_REGISTER_SS,CpuIndex);
    CpuContextList.CpuContext[CpuIndex].SegSs = Data16;
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
//[-end-151229-IB03090424-modify]//
}

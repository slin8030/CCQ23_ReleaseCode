/*++

Copyright (c)  1999 - 2014 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    SmmStm.c

Abstract:

    Smm/Stm Interface implementation file

--*/
#include "SmmPlatform.h"
#include "SmmStm.h"
#include "SaAccess.h"
//[-start-151229-IB03090424-modify]//
#if defined(TXT_SUPPORT) && (TXT_SUPPORT) && 0

#define SMM_FIRST_SEGMENT         0xA0000

extern  CHIPSET_CONFIGURATION                    mSystemConfiguration;
//[-end-151229-IB03090424-modify]//

VOID
GetMsegStatusCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  );

VOID
UpdateMsegSizeCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  );

VOID
LoadStmCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  );

VOID
UnloadStmCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  );

VOID
GetSmramRangesCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  );

VOID
SmmCopyMem (
  IN VOID   *Destination,
  IN VOID   *Source,
  IN UINTN  Length
  );

VOID
SmmZeroMem (
  IN VOID   *Buffer,
  IN UINTN  Size
  );


VOID
CpuEnableMseg (
VOID
  );

VOID
CpuDisableMseg (
VOID
  );


LT_MSEG_STATUS LtMsegStatus;


VOID
SmmZeroMem (
  IN VOID   *Buffer,
  IN UINTN  Size
  )
/*++

Routine Description:

  Set Buffer to Value for Size bytes.

Arguments:

  Buffer  - Memory to set.

  Size    - Number of bytes to set

Returns:

  None

--*/
{
  INT8  *Ptr;

  Ptr = Buffer;
  while (Size--) {
    *(Ptr++) = 0;
  }
}

VOID
InitSmmStm (
  EFI_SMM_SW_DISPATCH_PROTOCOL              *SwDispatch
)
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                                Status;
  EFI_HANDLE                                Handle;
  EFI_SMM_SW_DISPATCH_CONTEXT               SwContext;
  UINT32                                    MsegBase;

  Handle = NULL;


  Status = mSmst->SmmAllocatePool(
             EfiRuntimeServicesData,
             MAX_MSEG_SIZE + 1024*4,
             (VOID**)&MsegBase
             );
  ASSERT_EFI_ERROR(Status);
  ZeroMem((UINT8 *)MsegBase, MAX_MSEG_SIZE + 1024*4);

  if (MsegBase & 0x00000FFF) {
    MsegBase = MsegBase + 0x00001000;
    MsegBase = MsegBase & (~(0x00000FFF));
  }
  LtMsegStatus.MsegBase = MsegBase;
  LtMsegStatus.MsegSize = MAX_MSEG_SIZE;
  LtMsegStatus.MaxMsegSize = MAX_MSEG_SIZE;

  if ((mSystemConfiguration.LtTechnology) && (McMmio32(MC_MMIO_POC) & BIT14)) {
    Mmio32 (LT_PUB_BASE, LPB_MSEG_BAR) = MsegBase;
    Mmio32 (LT_PUB_BASE, LPB_MSEG_SIZE) = MAX_MSEG_SIZE;
  }


  SwContext.SwSmiInputValue = SMI_CMD_GET_MSEG_STATUS;
  Status = SwDispatch->Register(
                             SwDispatch,
                             GetMsegStatusCallback,
                             &SwContext,
                             &Handle
                             );
  ASSERT_EFI_ERROR(Status);

  SwContext.SwSmiInputValue = SMI_CMD_UPDATE_MSEG_SIZE;
  Status = SwDispatch->Register(
                           SwDispatch,
                           UpdateMsegSizeCallback,
                           &SwContext,
                           &Handle
                           );
  ASSERT_EFI_ERROR(Status);

  SwContext.SwSmiInputValue = SMI_CMD_LOAD_STM;
  Status = SwDispatch->Register(
                             SwDispatch,
                             LoadStmCallback,
                             &SwContext,
                             &Handle
                             );
  ASSERT_EFI_ERROR(Status);

  SwContext.SwSmiInputValue = SMI_CMD_UNLOAD_STM;
  Status = SwDispatch->Register(
                             SwDispatch,
                             UnloadStmCallback,
                             &SwContext,
                             &Handle
                             );
  ASSERT_EFI_ERROR(Status);

  SwContext.SwSmiInputValue = SMI_CMD_GET_SMRAM_RANGES;
  Status = SwDispatch->Register(
                             SwDispatch,
                             GetSmramRangesCallback,
                             &SwContext,
                             &Handle
                             );
  ASSERT_EFI_ERROR(Status);
}

VOID
GetMsegStatusCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  SmmCopyMem((UINTN*)(mSmst->CpuSaveState->ECX), &LtMsegStatus, sizeof(LT_MSEG_STATUS));

  // return success
  mSmst->CpuSaveState->EAX = LT_SMM_SUCCESS;
  mSmst->CpuSaveState->EFLAGS &= ~BIT0;
}

VOID
UpdateMsegSizeCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  // return error
  mSmst->CpuSaveState->EAX = ERROR_LT_SMM_STM_SIZE_TOO_LARGE;
  mSmst->CpuSaveState->EFLAGS |= BIT0;
}

VOID
LoadStmCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  LT_SMM_STM_DESCRIPTOR   *StmPackage;
  LT_STM_OS_SECTION       *SectionPtr;
  LT_STM_METADATA_ENTRY   *MetaDataPtr;
  UINTN                   *StmDataSrcPtr;
  UINTN                   *StmDataDestPtr;
  UINTN                   *SectionEnd;
  UINTN                   StmDataSize;
  UINTN                   Index;
  UINT8                   NullGuid[LT_STM_METADATA_GUID_SIZE];
  BOOLEAN                 NullGuidFound = FALSE;

  StmPackage = (LT_SMM_STM_DESCRIPTOR*)( mSmst->CpuSaveState->ECX );
  SectionPtr = (LT_STM_OS_SECTION*)( (UINTN)StmPackage->StmOsSectionPtr );
  MetaDataPtr = (LT_STM_METADATA_ENTRY*)( (UINTN)SectionPtr->BaseAddress );

  SmmZeroMem(&NullGuid, sizeof(NullGuid));

  // validate inputs
  // TODO: should improve the error-checking here
  //if ( SectionPtr->TerminalSection ) {
  //  mSmst->CpuSaveState->EAX = ERROR_LT_SMM_STM_REJECTED;
  //  mSmst->CpuSaveState->EFLAGS |= BIT0;
  //  return;
  //}

  // The first OS section contains metadata prepended to the actul STM
  // data. So, skip the meta-data and point to the start of STM data
  // before copying STM data from first OS section.

  // while we haven't reached the end of the OS section, keep looking for NULL GUID
  SectionEnd = (UINTN*)( (UINT8*)MetaDataPtr + SectionPtr->StmSectionLength );
  while ( (UINT8*)MetaDataPtr < (UINT8*)SectionEnd ) {
    if ( EfiCompareMem((UINT8*)MetaDataPtr->Guid, &NullGuid, sizeof(NullGuid)) == 0 ) {
      NullGuidFound = TRUE;
      break;
    }

    MetaDataPtr = (LT_STM_METADATA_ENTRY*)( (UINT8*)MetaDataPtr + MetaDataPtr->MetaDataLength );
  }

  // if we reached end of first OS section without finding NULL GUID, exit with error
  if ( !NullGuidFound ) {
    mSmst->CpuSaveState->EAX = ERROR_LT_SMM_STM_REJECTED;
    mSmst->CpuSaveState->EFLAGS |= BIT0;
    return;
  }

  // advance past NULL GUID
  StmDataSrcPtr = (UINTN*)( (UINT8*)MetaDataPtr + sizeof(NullGuid) );

  // copy the STM data
  StmDataDestPtr = (UINTN*)(UINTN)LtMsegStatus.MsegBase;
  // for the first OS section, subtract length of metadata and NULL GUID to get actual STM data size
  StmDataSize = (UINT8*)SectionEnd - (UINT8*)StmDataSrcPtr;

  while(TRUE) {
    if (StmDataSize > 0) {
      SmmCopyMem(StmDataDestPtr, StmDataSrcPtr, StmDataSize);
    }
    if(SectionPtr->TerminalSection) {
      break;
    }
    StmDataDestPtr = (UINTN*)( (UINT8*)StmDataDestPtr + StmDataSize );
    SectionPtr = (LT_STM_OS_SECTION*)( (UINT8*)SectionPtr + sizeof(LT_STM_OS_SECTION) );
    StmDataSrcPtr = (UINTN*)(UINTN)SectionPtr->BaseAddress;
    StmDataSize = SectionPtr->StmSectionLength;
  }

  // copy STM Info
  SmmCopyMem( (UINT8*)&LtMsegStatus.StmInfo, (UINT8*)&StmPackage->StmInfo, sizeof(STM_INFO) );
  LtMsegStatus.StmIsPresent = 1;

  //Enable MSEG in CPU threads.
  for (Index = 0; Index < mSmst->NumberOfCpus; Index++) {
    if(Index == 0) {
      CpuEnableMseg();
    } else {
      mSmst->SmmStartupThisAp (
                (EFI_AP_PROCEDURE) CpuEnableMseg,
                Index,
                NULL
                );
    }
  }

  // return success
  mSmst->CpuSaveState->EAX = LT_SMM_SUCCESS;
  mSmst->CpuSaveState->EFLAGS &= ~BIT0;
}

VOID
UnloadStmCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINTN                   Index;

  SmmZeroMem((UINTN*)(UINTN)LtMsegStatus.MsegBase, LtMsegStatus.MsegSize);

  //Disable MSEG in CPU threads.
  for (Index = 0; Index < mSmst->NumberOfCpus; Index++) {
    if(Index == 0) {
      CpuDisableMseg();
    } else {
      mSmst->SmmStartupThisAp (
                (EFI_AP_PROCEDURE) CpuDisableMseg,
                Index,
                NULL
                );
    }
  }

  // return success
  mSmst->CpuSaveState->EAX = LT_SMM_SUCCESS;
  mSmst->CpuSaveState->EFLAGS &= ~BIT0;
}

VOID
GetSmramRangesCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  LT_SMRAM_RANGE            *RangePtr;
  SMM_HANDLER_PRIVATE_DATA      *SmmPrivateData;

  // clear 4 entries in buffer
  RangePtr = (LT_SMRAM_RANGE*)(mSmst->CpuSaveState->ECX);
  SmmZeroMem((UINTN*)RangePtr, 4*sizeof(LT_SMRAM_RANGE));

  // first entry is TSEG
  SmmPrivateData  = (SMM_HANDLER_PRIVATE_DATA*)(UINTN)( SMM_FIRST_SEGMENT + SMM_PRIVATE_DATA_OFFSET);
  RangePtr->RangeBase = SmmPrivateData->TsegBase;
  RangePtr->RangeSize = (UINT32)(SmmPrivateData->TsegSize);

  // second entry is CSEG
  RangePtr++;
  RangePtr->RangeBase = (0x0A000 << 4); // A000 segment
  RangePtr->RangeSize = 0x20000;        // 128K

  // third entry is RUN_CSEG
  RangePtr++;
  RangePtr->RangeBase = (0x0F000 << 4); // F000 segment
  RangePtr->RangeSize = 0x10000;        // 64K

  // return success
  mSmst->CpuSaveState->EAX = LT_SMM_SUCCESS;
  mSmst->CpuSaveState->EFLAGS &= ~BIT0;
}

VOID
CpuEnableMseg (
VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINT64            MsegMsr;

  MsegMsr = LtMsegStatus.MsegBase | BIT0;
  EfiWriteMsr(EFI_MSR_IA32_MSEG, MsegMsr);
}


VOID
CpuDisableMseg (
VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINT64            MsegMsr;

  MsegMsr = 0x0000000000000000;
  EfiWriteMsr(EFI_MSR_IA32_MSEG, MsegMsr);
}


#endif //TXT_SUPPORT

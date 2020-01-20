//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/
#include <Uefi.h>
#include <Library/PcdLib.h>

#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Protocol/EfiShellInterface.h>
#include <Protocol/EfiShellParameters.h>
#include <Library/HeciMsgLib.h>

#define _FULL_OP_

#define _PERF_CHECK_

UINT8*  mFileBuffer;
UINT8*  PartitionBuffer;

#pragma pack(1)
typedef struct _FPTHdrV2{
  UINT32 ReservedQ[4];   //4 DWORD filled with all 0s
  UINT32 Signature;     //$FPT
  UINT32 NumFptEntries;  //Number of FPT entries
  UINT8  HdrVer;
  UINT8  EntryVer;
  UINT8  HdrLen;
  UINT8  CheckSum;
  UINT16 FlashLifeTime;
  UINT16 FlashCycleLmt;
  UINT32 DRAMSize;
  UINT32 FPTFlags;
  UINT32 ReservedS[2];   //2 DWORD reserved.
}FPTHdrV2;

typedef struct _FPTEntryHdrV1{
  UINT32 PartitionName;       //Unique name of the partition
  UINT32 PartitionOwner;  
  UINT32 Offset;              //Offset of the partition from beginning of FPT
  UINT32 Length;              //Length of the partition
  UINT32 TokensOnStart;
  UINT32 MaximumTokens;
  UINT32 ScratchSectors;
  UINT32 Attributes;
}FPTEntryHdrV1;

#pragma pack()


EFI_STATUS
ExtractCodePartitions(
  IN UINT8* kFileBuffer,
  OUT UINT32* mDestSize,
  OUT UINT8* ReUseSrcBuf
)
{
  EFI_STATUS          Status;
  FPTHdrV2            *fpt_hdr;
  UINT32              NParts;
  UINT32              PartIdx;
  UINT8               *BufPtr;
  FPTEntryHdrV1       *fpt_entry;
  UINT32               FTPREnd; 
  UINT32               FTPRStart;
  UINT32               NFTPEnd;
  UINT32               NFTPStart;
  UINT32               MDMVEnd;
  UINT32               MDMVStart;

  Status = EFI_SUCCESS;
  NParts = 0;
  PartIdx = 0;
  BufPtr = kFileBuffer;
  *mDestSize = 0;
  *ReUseSrcBuf = 1;
  FTPREnd = 0;
  NFTPEnd = 0;
  MDMVEnd = 0;
  FTPRStart = 0;
  NFTPStart = 0;
  MDMVStart = 0;

  fpt_hdr = (FPTHdrV2*)kFileBuffer;
  // check kfilebuffer is not NULL
  if(NULL == fpt_hdr) {
    Print(L"FPT is NULL. Aborting.\n");
    return EFI_ABORTED;
  }

  if(fpt_hdr->Signature != SIGNATURE_32('$','F','P','T')){
    Print(L"Invalid FPT header. Aborting.\n");
    return EFI_ABORTED;
  }
  NParts = fpt_hdr->NumFptEntries;
  if(NParts <2){
    Print(L"Not enough partitions for the update.\n");
    return EFI_ABORTED;
  }
  //
  //Parse the FPT entries, and find FTPR, NFTP and MDMV partitions, and check if they're continuous blocks
  //
  BufPtr = kFileBuffer + fpt_hdr->HdrLen;

  for(PartIdx = 0; PartIdx < NParts; PartIdx++){
    fpt_entry = (FPTEntryHdrV1*)BufPtr;
    if(fpt_entry->PartitionName == SIGNATURE_32('F','T','P','R')){
      FTPRStart = fpt_entry->Offset;
      PartitionBuffer += fpt_entry->Offset;
      *mDestSize += fpt_entry->Length;
      FTPREnd = fpt_entry->Offset + fpt_entry->Length -1;
      Print(L"FTPR Size: %x:\n", *mDestSize);
    }else if(fpt_entry->PartitionName == SIGNATURE_32('N','F','T','P')){
      NFTPStart = fpt_entry->Offset;
      NFTPEnd = fpt_entry->Offset + fpt_entry->Length -1;
      *mDestSize += fpt_entry->Length;
      Print(L"NFTP Size: %x\n", fpt_entry->Length);
    }else if(fpt_entry->PartitionName == SIGNATURE_32('M','D','M','V')){
      MDMVStart = fpt_entry->Offset;
      MDMVEnd = fpt_entry->Offset + fpt_entry->Length -1;
      *mDestSize += fpt_entry->Length;
      Print(L"MDMV Size: %x\n", fpt_entry->Length);
    }
    BufPtr += sizeof(FPTEntryHdrV1);
  }
  
  if(FTPRStart<NFTPStart && NFTPStart <MDMVStart && (FTPREnd+1 == NFTPStart) && (NFTPEnd+1 == MDMVStart)){
  //
    //No need to copy buffer. Just point to the start of FTPR and provide the length.
  //
  Print(L"----Reuse the buffer allocated previously. Destination buffer size:%x, Destination buffer offset:%x\n", *mDestSize, FTPRStart);
  return EFI_SUCCESS;
  }else{
  //
  //We need to allocate buffer for this
  //
  if( (FTPRStart == 0) || (NFTPStart == 0) || (MDMVStart == 0)){
    Print(L"Missing required partitions for a full update.\n");
    return EFI_ABORTED;
  }
  else{
    PartitionBuffer = AllocateZeroPool(*mDestSize);
    CopyMem(PartitionBuffer, mFileBuffer+FTPRStart, FTPREnd-FTPRStart +1);
    
    CopyMem(PartitionBuffer+(FTPREnd-FTPRStart+1), mFileBuffer+NFTPStart, NFTPEnd-NFTPStart+1);

    CopyMem(PartitionBuffer+(FTPREnd-FTPRStart+1)+(NFTPEnd-NFTPStart+1), mFileBuffer+MDMVStart, MDMVEnd-MDMVStart+1);
    *ReUseSrcBuf = 0;

    Print(L"Allocated new buffer to pass to SEC.\n");

    return EFI_SUCCESS;

  }

  }

}


EFI_STATUS
LoadSecImage(
  IN   CHAR16                *FileName,
  OUT UINTN                   *mSize
)
{
  SHELL_FILE_HANDLE      FileHandle;
  EFI_STATUS                  Status = EFI_SUCCESS;

  FileHandle = NULL;
  
  Status = ShellOpenFileByName(FileName, &FileHandle, EFI_FILE_MODE_READ, 0); 
  if (EFI_ERROR(Status)) {
    Print(L"Load File %s Error....!!!\n", FileName);
    Status = EFI_NOT_FOUND;
    goto LOADERROR;
  }
  Status = ShellGetFileSize(FileHandle, (UINT64 *)mSize);
  if (EFI_ERROR(Status)) {
    Print(L"Can't Get File %s Size, Return\n", FileName);
    Status = EFI_NOT_FOUND;
    goto LOADERROR;
  }

  mFileBuffer = AllocatePool((UINTN)*mSize);
  if (mFileBuffer == NULL) {
    Print(L"Can't Allocate Memory, Return\n");    
    Status = EFI_OUT_OF_RESOURCES;
    goto LOADERROR;  
  }
  
  Print(L"Loading %s to Memory......\n", FileName);    
  
  Status = ShellReadFile(FileHandle, (UINTN *)mSize, mFileBuffer);
  
  if (EFI_ERROR(Status)) {
    Print(L"Can't Read %s, Return\n", FileName);
    Status = EFI_OUT_OF_RESOURCES;
    goto LOADERROR;
  }
  

LOADERROR:  
  if(FileHandle != NULL) {
    ShellCloseFile(&FileHandle);
  }

  return Status;
}


UINT8
AnalyzeArgument(
  CHAR16     **Argv
)
{
  UINT8 Action = 0xFE;
  switch(Argv[1][1])
  {
    case 'F':
    case 'f':
      Action = 0;
      break;
    
    case 'V':
    case 'v':
      Action = 1;
      break;
        
  }
  return Action;
}

EFI_STATUS
EFIAPI 
SecFwuTestAppMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
{
  
   EFI_STATUS Status = EFI_SUCCESS;
   UINTN                                             FileSize = 0;
   UINTN                                              Argc;
   CHAR16                                           **Argv;
   EFI_SHELL_PARAMETERS_PROTOCOL *EfiShellParametersProtocol;
   EFI_SHELL_INTERFACE                      *EfiShellInterface;

   UINT8                                               Action;
   UINT8                                               LoopCount = 0;

  UINT32 Percentage = 0;
  UINT32 CurrentStage = 0; 
  UINT32 TotalStages = 0;
  UINT32 LastUpdateStatus = 0;
  UINT32 LastResetType = 0;

  UINT8  ReUseSrcBuf = 0;
  UINT32  PartitionsSize = 0;

#ifdef _FULL_OP_
  FWU_GET_VERSION_MSG_REPLY     VersionStatus;
   UINT32    RuleData = 0;
   UINT32    Index = 0;
#endif 

#ifdef _PERF_CHECK_
  UINT64                         TickerStart = 0;
  UINT64                         TickerStop = 0;
  UINT64                         Interval = 0;
#endif

   UINT32   MaxBufferSize = 0;
   OEM_UUID SecOemId;

   UINT32 ResetType =4;


   UINT8  SecAddress = 0;


   UINT8  InProgress = 0;


   mFileBuffer = NULL;
   PartitionBuffer = NULL;

   Status = gBS->OpenProtocol(ImageHandle,
                  &gEfiShellParametersProtocolGuid,
                  (VOID **)&EfiShellParametersProtocol,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
   //Step 1: Get Argument 

   if (!EFI_ERROR(Status)) {
    //
    // use shell 2.0 interface
    //
     Argc = EfiShellParametersProtocol->Argc;
     Argv = EfiShellParametersProtocol->Argv;
   } else {
    //
    // try to get shell 1.0 interface instead.
    //
     Status = gBS->OpenProtocol(ImageHandle,
                    &gEfiShellInterfaceGuid,
                    (VOID **)&EfiShellInterface,
                    ImageHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
     if (!EFI_ERROR(Status)) {
       //
       // use shell 1.0 interface
       //
       Argc = EfiShellInterface->Argc;
       Argv = EfiShellInterface->Argv;
     } else {
       Print(L"Can't Find Shell Interface, Return!\n");
       return EFI_NOT_FOUND;
    }
  }

//Step 2:  Analyze Argument 
   if (Argc != 3) {
     //ShowHelp();
     return EFI_NOT_FOUND;
   }

   Action = AnalyzeArgument(Argv);
   if(Action == 0xFE)
   {
     Print(L"------Input Argument error...!!!\n");
     //ShowHelp();
     return EFI_NOT_FOUND;
   }

   switch(Action){
   case 0:
     Status = LoadSecImage(Argv[2], &FileSize);
     if(EFI_ERROR(Status)){
       Print(L"------Failed to load Sec UPD image.\n");

       goto _noconnect_exit;
     }
     break;

   case 1:
     break;  //not supporting version command by this utlitlity yet.
   default:
     break;
   }


   Print(L"------File size is:%d.\n", FileSize);

   PartitionBuffer = mFileBuffer;

   Status = ExtractCodePartitions(PartitionBuffer, &PartitionsSize, &ReUseSrcBuf);
   if(EFI_ERROR(Status)){
     Print(L"------Failed to extract partitions from UPD image.\n");
     goto _noconnect_exit;
   }


#ifdef _FULL_OP_
   Status = HeciGetLocalFwUpdate(&RuleData);
   if(EFI_ERROR(Status)){
     Print(L"------Failed to get FWU Status from Sec.\n");
     goto _noconnect_exit;
   }


   if(RuleData == 1){
     Print(L"-------------Local firmware update enabled.\n");
   }else{
     Print(L"-------------Local firmware update disabled. Not able to perform SEC firmware update\n");
     Status = EFI_ABORTED;
     goto _noconnect_exit;
   }

   Status = HeciFwuWaitOnLastUpdate();
   if(EFI_ERROR(Status)){
     Print(L"------Failed due to get sec into normal state.\n");
     goto _noconnect_exit;
   }
#endif


#ifdef _PERF_CHECK_
  TickerStart = AsmReadTsc();
#endif

   Status = HeciConnectFwuInterface(&SecAddress, &MaxBufferSize);
   if(EFI_ERROR(Status)){
     Print(L"------Failed to get FWU connection from Sec.\n");
     goto _exit;
   }
#ifdef _PERF_CHECK_
  TickerStop = AsmReadTsc();
  Interval = DivU64x64Remainder(TickerStop-TickerStart, 1600000, NULL);
  Print(L"######Performance: FWU_Connect cost %d mseconds.\n", Interval);
  TickerStop = TickerStart = 0;
#endif

   Print(L"------Connection setup. Max buffer size is:%d\n", MaxBufferSize);

#ifdef _FULL_OP_
   ZeroMem(&VersionStatus, sizeof(FWU_GET_VERSION_MSG_REPLY)); 
   
   Status = HeciSendFwuGetVersionMsg(&VersionStatus, SecAddress);
   if(EFI_ERROR(Status)){
     Print(L"------Failed to get Version from  Sec.\n");
     goto _exit;
   }

   Print(L"------Code Version now is: %d.%d.%d.%d\n", VersionStatus.CodeVersion.Major, VersionStatus.CodeVersion.Minor, VersionStatus.CodeVersion.Hotfix, VersionStatus.CodeVersion.Build);
#endif


   ZeroMem(&SecOemId, sizeof(OEM_UUID));

   Status = HeciVerifyOemId(&SecOemId, SecAddress);
   if(EFI_ERROR(Status)){
     Print(L"------OEM id verification failed.\n");
     goto _exit;
   }

#ifdef _FULL_OP_
   Status = HeciSendFwuGetOemIdMsg(&SecOemId,SecAddress);

   if(EFI_ERROR(Status)){
     Print(L"------Failed to get OemID from  Sec.\n");
     goto _exit;
   }


   Print(L"------The OemId is:%08x-%04x-%04x-", SecOemId.Data1, SecOemId.Data2, SecOemId.Data3);
   for(Index = 0; Index < 8; Index++){
     Print(L"%02x", SecOemId.Data4[Index]);
   }
   Print(L"\n");
   
#endif



   //Status = HeciSendFwuStartMsg((UINT32)FileSize-0x33000, &SecOemId,SecAddress, FileSize -0x33000 +4);
   Status = HeciSendFwuStartMsg(PartitionsSize, &SecOemId,SecAddress, MaxBufferSize);
   if(EFI_ERROR(Status)){
     Print(L"------Failed to send FwuStart messag to  Sec.\n");

     Status = HeciCheckFwuInProgress(&InProgress);
     Print(L"------FWU progress check after FWU start message. In progress: %d.\n", InProgress);

     goto _fwu_end_exit;
   }


#ifdef _PERF_CHECK_
  TickerStart = AsmReadTsc();
#endif

   //Status = HeciSendFwuDataMsg(mFileBuffer+0x33000, (UINT32)FileSize-0x33000, SecAddress, MaxBufferSize);
   Status = HeciSendFwuDataMsg(PartitionBuffer, PartitionsSize, SecAddress, MaxBufferSize);
   if(EFI_ERROR(Status)){
     Print(L"------Failed to send Fwu Data messag to  Sec.\n");
     goto _exit;
   }

#ifdef _PERF_CHECK_
  TickerStop = AsmReadTsc();
  Interval = DivU64x64Remainder(TickerStop-TickerStart, 1600000, NULL);
  Print(L"######Performance: FWU_Data cost %d mseconds.\n", Interval);
  TickerStop = TickerStart = 0;
#endif



#ifdef _PERF_CHECK_
  TickerStart = AsmReadTsc();
#endif

_fwu_end_exit:
   Status = HeciSendFwuEndMsg(&ResetType,SecAddress);
   if(EFI_ERROR(Status)){
     Print(L"------Timeout waiting for FWU_END reply. Continue to query the status for 90 seconds\n");
     goto _exit;
   }


_exit:
  Status = HeciDisconnectFwuInterface(SecAddress,MaxBufferSize);
  if(EFI_ERROR(Status)){
    Print(L"------Failed to disconnet from Sec.\n");
  }

  //By theory, the update won't take longer than 90 seconds~
  do{
    Status = HeciFwuQueryUpdateStatus(&Percentage, &CurrentStage, &TotalStages, &LastUpdateStatus, &LastResetType, &InProgress);

    if(EFI_ERROR(Status)){
     Print(L"Failed to update status");
     goto _noconnect_exit;
    }
    Print(L"Fw update in progress- Percentage:%d%%, Stage:%d/%d, LastUpdateStatus:%d.\n", Percentage, CurrentStage, TotalStages, LastUpdateStatus);
    gBS->Stall(500*1000);
    LoopCount++;
  }while(Percentage<100 &&  LoopCount<180);

#ifdef _PERF_CHECK_
  TickerStop = AsmReadTsc();
  Interval = DivU64x64Remainder(TickerStop-TickerStart, 1600000, NULL);
  Print(L"######Performance: FWU_End cost %d mseconds.\n", Interval);
  TickerStop = TickerStart = 0;
#endif

_noconnect_exit:
  if(mFileBuffer != NULL){
  FreePool(mFileBuffer);
    mFileBuffer = NULL;
  }
  if(!ReUseSrcBuf && (PartitionBuffer!=NULL)){
  FreePool(PartitionBuffer);
  PartitionBuffer = NULL;
  }
  return Status;
}

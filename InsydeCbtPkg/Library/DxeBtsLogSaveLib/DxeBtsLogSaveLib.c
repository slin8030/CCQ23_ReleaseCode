 /** @file
  Dxe Bts Log Save Library
  
;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <PiDxe.h>

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>

#include <Protocol/BtsInitDxe.h>
#include <Protocol/SimpleFileSystem.h>

#define  MSR_IA32_DS_AREA            0x0600

CHAR16   mFileName[11] ={L"BtsLog.BTS"};


#pragma pack(1)

typedef struct {
  UINT32   Signature;
  UINT32   Version;
  UINT32   BtsOffset;
  UINT32   BtsSize;
  UINT32   ImginfoOffset;
  UINT32   ImginfoSize;
} Bts_Log_Header, *PBts_Log_Header;

typedef struct {
  UINT64   pBtsBufferBase;
  UINT64   pBtsBufferIndex;
  UINT64   pBtsMaxSize;
  UINT64   pBtsIntThresold;
  UINT64   pPebsBufferBase;
  UINT64   pPebsIndex;
  UINT64   pPebsMaxSize;
  UINT64   pPebsIntThresold;
  UINT64   dwPebsCounterReset;
  UINT64   Reserved[3];
} DS_BUFFER_MGR_BLOCK, *PDS_BUFFER_MGR_BLOCK;

#pragma pack()

/**
  Do Dxe Bts Log Save.
  
**/
VOID
EFIAPI
DxeBtsLogSave (
  )
{
  EFI_STATUS                        Status;
  EFI_BTS_INIT_PROTOCOL             *BtsInitProtocol;
  PDS_BUFFER_MGR_BLOCK              pBtsBuffer;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *FileSystemProtocol;
  EFI_FILE                          *RootHandle;
  EFI_FILE                          *File;
  Bts_Log_Header                    Btsheader;
  UINTN                             Size;
  UINT64                            *Buffer;

  //
  // Locate gEfiBtsInitProtocolGuid Protocol interface.
  //
  Status = gBS->LocateProtocol (
                  &gEfiBtsInitProtocolGuid, 
                  NULL, 
                  (VOID **)&BtsInitProtocol
                  );
  if (Status != EFI_SUCCESS) {
    return; 
  }

  //
  // Stop Bts log.
  //
  BtsInitProtocol->StopBts ();

  //
  // Get Bts buffer and BufferSize
  //
  pBtsBuffer = (PDS_BUFFER_MGR_BLOCK)(UINTN)AsmReadMsr64(MSR_IA32_DS_AREA);

  if (pBtsBuffer != NULL) {
    Btsheader.Signature = 'HSTB';
    Btsheader.Version = 0x10001;
    Btsheader.BtsOffset = (UINT32)sizeof (Bts_Log_Header);
    Size = (UINTN)((pBtsBuffer->pBtsMaxSize - pBtsBuffer->pBtsBufferIndex) / 24) * 24;
    Size += (UINTN)(pBtsBuffer->pBtsBufferIndex - pBtsBuffer->pBtsBufferBase);
    Btsheader.BtsSize = (UINT32)Size;
    Btsheader.ImginfoOffset = (UINT32)Btsheader.BtsOffset + Btsheader.BtsSize;
    Btsheader.ImginfoSize = (UINT32)(pBtsBuffer->pPebsIndex - pBtsBuffer->pPebsBufferBase);
  } else {
    goto Exit;
  }
  
  //
  // Locate gEfiSimpleFileSystemProtocolGuid Protocol interface
  //
  Status = gBS->LocateProtocol (
                  &gEfiSimpleFileSystemProtocolGuid, 
                  NULL, 
                  (VOID **)&FileSystemProtocol
                  );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = FileSystemProtocol->OpenVolume (
                               FileSystemProtocol, 
                               &RootHandle
                               );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // if file exist, delect it.
  //
  Status = RootHandle->Open (
                       RootHandle, 
                       &File, 
                       mFileName,
                       EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                       0
                       );  
  if (Status == EFI_SUCCESS) {
    File->Delete (File);
  }

  //
  // Create file to store log.
  //
  Status = RootHandle->Open (
                         RootHandle, 
                         &File, 
                         mFileName,
                         EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                         EFI_FILE_ARCHIVE
                         );  
  if (EFI_ERROR (Status)) {
    RootHandle->Close (RootHandle);
    goto Exit;
  }
  
  Size = sizeof (Bts_Log_Header);
  Status = File->Write (File, &Size, (VOID *) &Btsheader);

  //
  // First part of bts records
  //
  Buffer = (UINT64 *)(UINTN)(pBtsBuffer->pBtsBufferIndex);
  if (*Buffer != 0) {
    Size = (UINTN)((pBtsBuffer->pBtsMaxSize - pBtsBuffer->pBtsBufferIndex) / 24) * 24;
    Status = File->Write (File, &Size, (VOID *)(UINTN)pBtsBuffer->pBtsBufferIndex);
  }

  //
  // Last part of bts records
  //
  Size = (UINTN)(pBtsBuffer->pBtsBufferIndex - pBtsBuffer->pBtsBufferBase);
  Status = File->Write (File, &Size, (VOID *)(UINTN)pBtsBuffer->pBtsBufferBase);

  //
  // Image info
  //
  Size = (UINT32)(pBtsBuffer->pPebsIndex - pBtsBuffer->pPebsBufferBase);
  Status = File->Write (File, &Size, (VOID *)(UINTN)pBtsBuffer->pPebsBufferBase);

  File->Close (File);
  RootHandle->Close (RootHandle);

  Exit:  
  //
  // Start Bts log.
  //
  BtsInitProtocol->StartBts ();
  return;
}
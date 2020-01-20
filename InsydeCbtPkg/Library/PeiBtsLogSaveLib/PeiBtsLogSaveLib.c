 /** @file
  Pei Bts Log Save Library
  
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
#include <PiPei.h>

#include <Ppi/BtsInitPei.h>
#include <Ppi/FileAccessPei.h>

#include <Library/PeiServicesLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>

#define  MSR_IA32_DS_AREA            0x0600

CHAR16   mFileName[11] = {L"BtsLog.bts"};

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
  Do Pei Bts Log Save.
  
**/
VOID
EFIAPI
PeiBtsLogSave (
  )
{
  EFI_STATUS              Status;
  PEI_FILE_ACCESS_PPI     *FilePpi;
  PEI_FILE_HANDLE         FileHandle;
  PEI_BTS_INIT_PPI        *BtsInitPpi;
  PDS_BUFFER_MGR_BLOCK    pBtsBuffer;
  Bts_Log_Header          Btsheader;
  UINTN                   Size;
  UINT64                  *Buffer;

  //
  // Locate gPeiBtsInitPpiGuid interface
  //
  Status = PeiServicesLocatePpi (&gPeiBtsInitPpiGuid, 0, NULL, (VOID **)&BtsInitPpi);
  if (Status != EFI_SUCCESS) {
    return; 
  }
  
  //
  // Stop Bts log.
  //
  BtsInitPpi->StopBts ();

  //
  // Get Bts buffer and BufferSize
  //
  pBtsBuffer = (PDS_BUFFER_MGR_BLOCK)(UINTN)AsmReadMsr64(MSR_IA32_DS_AREA);
  if (pBtsBuffer != NULL) {
    Btsheader.Signature = 'HSTB';
    Btsheader.Version = 0x10001;
    Btsheader.BtsOffset = (UINT32)sizeof (Bts_Log_Header);
    Size = (UINTN)((((UINT32)(pBtsBuffer->pBtsMaxSize - pBtsBuffer->pBtsBufferIndex)) / 24) * 24);
    Size += (UINTN)(pBtsBuffer->pBtsBufferIndex - pBtsBuffer->pBtsBufferBase);
    Btsheader.BtsSize = (UINT32)Size;
    Btsheader.ImginfoOffset = (UINT32)Btsheader.BtsOffset + Btsheader.BtsSize;
    Btsheader.ImginfoSize = (UINT32)(pBtsBuffer->pPebsIndex - pBtsBuffer->pPebsBufferBase);
  } else {
    goto Exit;
  }
  
  //
  // Locate gPeiFileAccessPpiGuid interface
  //
  Status = PeiServicesLocatePpi (&gPeiFileAccessPpiGuid, 0, NULL, (VOID **) &FilePpi);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Storage BTS info
  //
  Status = FilePpi->OpenFile (
                      FilePpi, 
                      mFileName, 
                      &FileHandle, 
                      EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 
                      EFI_FILE_ARCHIVE,
                      0
                      );  
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Size = sizeof (Bts_Log_Header);
  Status = FilePpi->WriteFile (FilePpi, FileHandle, &Size, (VOID *) &Btsheader);

  //
  // First part of bts records
  //
  Buffer = (UINT64 *)(UINTN)(pBtsBuffer->pBtsBufferIndex);
  if (*Buffer != 0) {
    Size = (UINTN)((((UINT32)(pBtsBuffer->pBtsMaxSize - pBtsBuffer->pBtsBufferIndex)) / 24) * 24);
    Status = FilePpi->WriteFile (FilePpi, FileHandle, &Size, (VOID *)(UINTN)pBtsBuffer->pBtsBufferIndex);
  }
  //
  // Last part of bts records
  //
  Size = (UINTN)(pBtsBuffer->pBtsBufferIndex - pBtsBuffer->pBtsBufferBase);
  Status = FilePpi->WriteFile (FilePpi, FileHandle, &Size, (VOID *)(UINTN)pBtsBuffer->pBtsBufferBase);

  //
  // Image info
  //
  Size = (UINT32)(pBtsBuffer->pPebsIndex - pBtsBuffer->pPebsBufferBase);
  Status = FilePpi->WriteFile (FilePpi, FileHandle, &Size, (VOID *)(UINTN)pBtsBuffer->pPebsBufferBase);

  FilePpi->CloseFile (FilePpi, FileHandle);

  Exit:  
  //
  // Start Bts log.
  //
  BtsInitPpi->StartBts ();
  return;
 
}
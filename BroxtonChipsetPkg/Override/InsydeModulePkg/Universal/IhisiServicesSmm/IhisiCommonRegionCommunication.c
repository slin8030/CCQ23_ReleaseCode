/** @file
  FactoryDefaultLib Library Instance implementation

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "IhisiCommonRegionCommunication.h"
#include <Library/MemoryAllocationLib.h>
#include <Library/VariableSupportLib.h>
#include <Library/FlashRegionLib.h>
#include <Guid/ImageAuthentication.h>
//[-start-161215-IB10860217-add]//
#include <Library/VariableLib.h>
#include <Guid/AdmiSecureBoot.h>
#include <Guid/DebugMask.h>
//[-end-161215-IB10860217-add]//
STATIC COMMON_REGION_READ_STRUCTURE  mCommonRegionReadControl = {0};

//
//Data communcation sub function
//
STATIC COMMON_REGION_COMMUNICATION_FUNCTION_TABLE DataCommunicationSubFun []= {
  {FACTORY_COPY_EVENT,                                  WriteDefaultSettingsToFactoryCopy},
  {FACTORY_COPY_READ,                                   ReadDefaultSettingsToFactoryCopy},
  {FACTORY_COPY_RESTORE_WITH_CLEARING_ALL_SETTINGS,     RestoreFactoryCopyWithClearingAllSettings},
  {FACTORY_COPY_RESTORE_WITH_RESERVING_OTHER_SETTINGS,  RestoreFactoryCopyWithReservingOtherSettings}
  };

//
// Factory default Support Function
//
STATIC
IHISI_REGISTER_TABLE
COMMON_REGION_REGISTER_TABLE[] = {
  //
  // AH=49h
  //
  { FBTSCommonCommunication, "S49Kn_ComDataCommun", CommonRegionDataCommunication}, \

  //
  // AH=4Bh
  //
  { FBTSCommonRead, "S4BKn_ComDataRead00", CommonRegionDataRead}
};

/**
  Get all of secure boot settings from factory copy region.

  @param[in, out] BufferSize     On input, Total buffer size.
                                 On output, The size of all secure boot settings.
  @param[out]     Buffer         Buffer to save secure boot settings.

  @retval EFI_SUCCESS            Collect all of secure boot settings successful.
  @retval EFI_INVALID_PARAMETER  Any input parameter is invalid.
  @retval EFI_OUT_OF_RESOURCES   Allocate memory failed.
**/
EFI_STATUS
CollectSecureBootSettings (
  IN OUT UINTN            *BufferSize,
  OUT    UINT8            *Buffer
  )
{
  UINTN               Index;
  UINTN               CopySize;
  UINTN               TotalSize;
  UINTN               HeaderSize;
  UINTN               FunctionCnt;
  BOOLEAN             VariableFound;
  UINTN               LastVariableIndex;
  UINT8              *ReadBuffer;
  UINT64              FactoryCopySize;
  VARIABLE_HEADER    *VariableHeader;
  VARIABLE_HEADER    *InDeletedVariable;
  SPECIFIC_VARIABLE   GetVariable [] = {IsPkVariable, IsKekVariable, IsDbVariable, IsDbxVariable};

  FactoryCopySize = 0;

  HeaderSize = sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY) + GetVariableStoreHeaderSize ();;
  if (BufferSize == NULL || Buffer == NULL || *BufferSize < HeaderSize) {
    return EFI_INVALID_PARAMETER;
  }

  FactoryCopySize = FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);
  ReadBuffer = AllocatePool ( ((UINTN) FactoryCopySize) - HeaderSize);
  if (ReadBuffer == NULL) {
    ASSERT (ReadBuffer != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  TotalSize = *BufferSize;
  LastVariableIndex = 0;
  FunctionCnt = sizeof (GetVariable) / sizeof (SPECIFIC_VARIABLE);
  for (Index = 0; Index < FunctionCnt; Index++) {
    VariableFound = FALSE;
    VariableHeader = (VARIABLE_HEADER *) (Buffer + HeaderSize);
    InDeletedVariable = NULL;
    while (IsValidVariableHeader (VariableHeader) && (UINTN) VariableHeader - (UINTN) Buffer < TotalSize) {
      if (VariableHeader->State == VAR_ADDED &&
          GetVariable[Index] ((CHAR16 *) (VariableHeader + 1), &VariableHeader->VendorGuid)) {
        CopySize = (UINTN) GetNextVariablePtr (VariableHeader) - (UINTN) VariableHeader;
        if (CopySize + LastVariableIndex + HeaderSize > ((UINTN) FactoryCopySize) ) {
          FreePool (ReadBuffer);
          ASSERT (FALSE);
          return EFI_OUT_OF_RESOURCES;
        }
        CopyMem (&ReadBuffer[LastVariableIndex], VariableHeader, CopySize);
        LastVariableIndex += CopySize;
        VariableFound = TRUE;
        break;
      } else if (VariableHeader->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION) &&
                 GetVariable[Index] ((CHAR16 *) (VariableHeader + 1), &VariableHeader->VendorGuid)) {
        InDeletedVariable = VariableHeader;
      }
      VariableHeader = GetNextVariablePtr (VariableHeader);
    }
    if (!VariableFound && InDeletedVariable != NULL) {
      CopySize = (UINTN) GetNextVariablePtr (InDeletedVariable) - (UINTN) InDeletedVariable;
      if (CopySize + LastVariableIndex + HeaderSize > (UINTN) FactoryCopySize) {
        FreePool (ReadBuffer);
        ASSERT (FALSE);
        return EFI_OUT_OF_RESOURCES;
      }
      CopyMem (&ReadBuffer[LastVariableIndex], InDeletedVariable, CopySize);
      LastVariableIndex += CopySize;
      VariableFound = TRUE;
    }
    //
    // if factory copy doesn't have dbx variable, it still a legal factory default copy.
    //
    if (!VariableFound && Index != FunctionCnt - 1)  {
      FreePool (ReadBuffer);
      return EFI_NOT_FOUND;
    }
  }
  //
  // copy collected secure boot settings to input buffer
  //
  SetMem (Buffer + HeaderSize, TotalSize - HeaderSize, 0xFF);
  CopyMem (Buffer + HeaderSize, ReadBuffer, LastVariableIndex);
  FreePool (ReadBuffer);
  //
  // Setting output buffer size
  //
  *BufferSize = LastVariableIndex + HeaderSize;
  return EFI_SUCCESS;
}

/**
  The implementation of IHISI function 49h type 2. return default factory default settings
  information to application.

  @retval EFI_SUCCESS            The function completed successfully.
**/
STATIC
EFI_STATUS
ReadDefaultSettingsToFactoryCopy (
  VOID
  )
{
  OUTPUT_DATA_STRUCTURE          *OutputData;
  UINT64                         FactoryCopySize;

  OutputData = (OUTPUT_DATA_STRUCTURE *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  OutputData->BlockSize = COMMON_REGION_BLOCK_SIZE_4K;

  FactoryCopySize = FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);

  if (FactoryCopySize == 0x10000) {
    OutputData->DataSize = COMMON_REGION_SIZE_64K;
  } else {
    OutputData->DataSize = COMMON_REGION_REPORT_READ_SIZE;
    OutputData->PysicalDataSize = (UINT32) FactoryCopySize;
  }
  mCommonRegionReadControl.WantToReadData = TRUE;
  mCommonRegionReadControl.InCommonReadProcess = FALSE;
  mCommonRegionReadControl.BlockSize = 0x1000;
  mCommonRegionReadControl.DataSize = (UINTN) FactoryCopySize;
  mCommonRegionReadControl.RemainDataSize = (UINTN) FactoryCopySize;
  return IHISI_SUCCESS;
}

/**
  The implementation of IHISI function 49h type 1. Write secure boot settings from
  variable store to factory copy region.

  @retval EFI_SUCCESS           The function completed successfully.
  @retval EFI_OUT_OF_RESOURCES  Allocate pool failed.
  @retval Others                Other error occurred in this function.
**/
STATIC
EFI_STATUS
WriteDefaultSettingsToFactoryCopy (
  VOID
  )
{
  EFI_STATUS                              Status;
  UINTN                                   EraseSize;
  UINTN                                   WriteSize;
  UINTN                                   WriteCount;
  BOOLEAN                                 WriteSuccess;
  UINTN                                   ReadBufferSize;
  UINTN                                   WriteBufferSize;
  UINT8                                  *Buffer;
  UINT64                                  FactoryCopyAddr;
  UINT64                                  FactoryCopySize;

  FactoryCopySize = FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);
  FactoryCopyAddr = FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);

  ReadBufferSize = (UINTN) FactoryCopySize;
  Buffer = AllocatePool (ReadBufferSize);
  if (Buffer == NULL) {
    ASSERT (Buffer != NULL);
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  Status = mSmmFwBlockService->Read (
                                mSmmFwBlockService,
                                (UINTN) FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_WORKING, 1),
                                0,
                                &ReadBufferSize,
                                Buffer
                                );
  if (EFI_ERROR(Status)) {
    FreePool (Buffer);
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  WriteBufferSize = ReadBufferSize;
  Status = CollectSecureBootSettings (&WriteBufferSize, Buffer);
  if (EFI_ERROR(Status) || ReadBufferSize < WriteBufferSize + FACTORY_COPY_HEADER_SIZE) {
    FreePool (Buffer);
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  //
  // Combine factory copy header and foctory copy content
  //
  CopyMem (Buffer + FACTORY_COPY_HEADER_SIZE, Buffer, WriteBufferSize);
  ReadBufferSize = FACTORY_COPY_HEADER_SIZE;
  Status = mSmmFwBlockService->Read (
                                mSmmFwBlockService,
                                (UINTN) FactoryCopyAddr,
                                0,
                                &ReadBufferSize,
                                Buffer
                                );
  WriteBufferSize += FACTORY_COPY_HEADER_SIZE;
  if (EFI_ERROR(Status)) {
    FreePool (Buffer);
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  WriteCount = 0;
  WriteSuccess = FALSE;
  do {
    EraseSize = (UINTN) FactoryCopySize;
    Status = mSmmFwBlockService->EraseBlocks (
                                  mSmmFwBlockService,
                                  (UINTN) FactoryCopyAddr,
                                  (UINTN *) &EraseSize
                                  );
    if (!EFI_ERROR (Status)) {
      WriteSize = WriteBufferSize;
      Status = mSmmFwBlockService->Write (
                                    mSmmFwBlockService,
                                    (UINTN) FactoryCopyAddr,
                                    (UINTN *) &WriteSize,
                                    Buffer
                                    );
      if (!EFI_ERROR (Status)) {
        WriteSuccess = TRUE;
      }
    }
    WriteCount++;
  } while (WriteCount < 100 && !WriteSuccess);

  FreePool (Buffer);
  return WriteSuccess ? EFI_SUCCESS : IHISI_ACCESS_PROHIBITED;
}

/**
  Initialize output data for IHISI function 49h

  @param[out] OutputData        Pointer to OUTPUT_DATA_STRUCTURE structure.

  @retval EFI_SUCCESS           Initailize output data successful.
  @retval EFI_INVALID_PARAMETER OutputData is NULL.
**/
EFI_STATUS
InitializeOutputData (
  OUT OUTPUT_DATA_STRUCTURE       *OutputData
  )
{
  if (OutputData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  OutputData->Signature = COMMON_REGION_OUTPUT_SIGNATURE;
  OutputData->StructureSize = sizeof (OUTPUT_DATA_STRUCTURE);
  OutputData->Status = 0;

  return EFI_SUCCESS;
}

/**
  The entry point of IHISI function 4Bh.

  @retval EFI_SUCCESS           The function completed successfully
  @retval EFI_BUFFER_TOO_SMALL  The size of input buffer is too small to save read data.
  @return Others                Other error causes this function cannot work properly.
**/
EFI_STATUS
EFIAPI
CommonRegionDataRead (
  VOID
  )
{
  EFI_STATUS                              Status;
  UINTN                                   ReadBufferSize;
  UINT32                                  OutputBufferSize;
  UINT8                                  *OutputBuffer;
  UINT64                                  FactoryCopyAddr;

  Status = EFI_UNSUPPORTED;
  if (mCommonRegionReadControl.WantToReadData) {
    OutputBuffer = (UINT8 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
    CopyMem (&OutputBufferSize, (VOID *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI), sizeof (UINT32));
    if (OutputBuffer == NULL || OutputBufferSize != mCommonRegionReadControl.BlockSize) {
      return IHISI_UNSUPPORTED_FUNCTION;
    }
    if (BufferOverlapSmram ((VOID *) OutputBuffer, OutputBufferSize)) {
      return IHISI_ACCESS_PROHIBITED;
    }

    mCommonRegionReadControl.InCommonReadProcess = TRUE;
    ReadBufferSize = (UINTN) OutputBufferSize;
    
    FactoryCopyAddr = FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);
    Status = mSmmFwBlockService->Read (
                                  mSmmFwBlockService,
                                  (UINTN) FactoryCopyAddr,
                                  mCommonRegionReadControl.DataSize - mCommonRegionReadControl.RemainDataSize,
                                  &ReadBufferSize,
                                  OutputBuffer
                                  );

    if (mCommonRegionReadControl.RemainDataSize > OutputBufferSize) {
      mCommonRegionReadControl.RemainDataSize -= OutputBufferSize;
    } else {
    //
    // The factory copy size must be integer multiple of flash blocks. If RemainDataSize smaller than
    // output buffer size indicates this read is inavlid
    //
      if (mCommonRegionReadControl.RemainDataSize < OutputBufferSize) {
        Status = EFI_OUT_OF_RESOURCES;
      }
      mCommonRegionReadControl.WantToReadData = FALSE;
      mCommonRegionReadControl.RemainDataSize = 0;
      mCommonRegionReadControl.InCommonReadProcess = FALSE;
    }

    if (!EFI_ERROR(Status)) {
      OutputBufferSize = (UINT32) ReadBufferSize;
      CopyMem ((VOID *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI), &OutputBufferSize, sizeof (UINT32));
      Status = IHISI_SUCCESS;
    } else {
      Status = IHISI_UNSUPPORTED_FUNCTION;
    }
  } else {
    Status = IHISI_UNSUPPORTED_FUNCTION;
  }

  return Status;
}

/**
  This funciotn uses to export merged factory default setting. The merged factory copy
  default setting means keep all of original variable data and then merge variable data
  in factory copy region with original data. If finding the same variable, system will
  use the variable in factory default to replace oringnal variable.

  @param[in, out] DataSize       On input, the size in bytes of the return Data buffer.
                                 On output the size of data returned in Data buffer.
  @param[out]     Data           The buffer to return the contents of the variable.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_INVALID_PARAMETER  Some input parameter is invalid.
  @retval EFI_BUFFER_TOO_SMALL   The VariableStoreLength is too small for the result.  DataSize has
                                 been updated with the size needed to complete the request.
  @return Others                 Other error causes merge factory default setting failed.
**/
EFI_STATUS
ExportMergedFactoryDefaultSetting (
  IN OUT UINTN                         *DataSize,
  OUT    UINT8                         *Data
  )
{
  EFI_STATUS                            Status;
  UINTN                                 WorkingBufferLength;
  UINTN                                 VariableBufferLength;
  UINTN                                 FactoryCopyRegionSize;
  UINT8                                *WorkingBuffer;
  UINT8                                *VariableBuffer;
  UINT64                                VariableAddr;
  UINT64                                VariableSize;
  UINT64                                FactoryCopyAddr;
  UINT64                                FactoryCopySize;
  

  if (DataSize == NULL || Data == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  WorkingBuffer       = NULL;
  VariableBuffer      = NULL;
  WorkingBufferLength = 0;

    FactoryCopySize = 0;
    FactoryCopyAddr = 0;
    VariableAddr = 0;
    VariableSize = 0;

  VariableSize = FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_WORKING, 1);
  FactoryCopySize = FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);

  ASSERT ( VariableSize  >= FactoryCopySize);
  //
  // Read whole factory copy sub-region data
  //
  WorkingBufferLength = (UINTN) VariableSize;
  WorkingBuffer = AllocatePool (WorkingBufferLength);
  if (WorkingBuffer == NULL) {
    ASSERT (WorkingBuffer != NULL);
    Status = EFI_OUT_OF_RESOURCES;
  }

  SetMem (WorkingBuffer, WorkingBufferLength, 0xFF);

  FactoryCopyRegionSize = (UINTN) FactoryCopySize - FACTORY_COPY_HEADER_SIZE;
  Status = mSmmFwBlockService->Read (
                                mSmmFwBlockService,
                                (UINTN) FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1),
                                FACTORY_COPY_HEADER_SIZE,
                                &FactoryCopyRegionSize,
                                WorkingBuffer
                                );
  if (EFI_ERROR (Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  //
  // Read whole variable store data
  //
  VariableBufferLength = (UINTN) VariableSize;
  VariableBuffer = AllocatePool (VariableBufferLength);
  if (VariableBuffer == NULL) {
    ASSERT (VariableBuffer != NULL);
    Status = EFI_OUT_OF_RESOURCES;
  }
  SetMem (VariableBuffer, VariableBufferLength, 0xFF);
  Status = mSmmFwBlockService->Read (
                                mSmmFwBlockService,
                                (UINTN)FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_WORKING, 1),
                                0,
                                &VariableBufferLength,
                                VariableBuffer
                                );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = MergeVariableToFactoryCopy (WorkingBuffer, &WorkingBufferLength, VariableBuffer, VariableBufferLength);

Done:

  if (!EFI_ERROR (Status)) {
    if (WorkingBufferLength <= *DataSize) {
      CopyMem (Data, WorkingBuffer, WorkingBufferLength);
    } else {
      Status = EFI_BUFFER_TOO_SMALL;
    }
    *DataSize = WorkingBufferLength;
  }

  if (WorkingBuffer != NULL) {
    FreePool (WorkingBuffer);
  }

  if (VariableBuffer != NULL) {
    FreePool (VariableBuffer);
  }

  return Status;
}

/**
  Checks the contian of factory copy region is valid or not

  @retval TRUE   The contain in factory copy region is valid.
  @retval FALSE  The contain in factory copy region is invalid.
**/
STATIC
BOOLEAN
IsFacotryCopyValid (
  VOID
  )
{
  EFI_STATUS                            Status;
  UINTN                                 Index;
  BOOLEAN                               IsValid;
  UINTN                                 ReadBufferSize;
  UINTN                                 NumberOfInteger;
  UINTN                                *ReadBuffer;
  UINT64                                FactoryCopySize;

  IsValid = FALSE;
  FactoryCopySize = FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);
  ReadBufferSize = (UINTN) FactoryCopySize - FACTORY_COPY_HEADER_SIZE;
  ReadBuffer = AllocatePool (ReadBufferSize);
  if (ReadBuffer == NULL) {
    ASSERT (ReadBuffer != NULL);
    return FALSE;
  }

  Status = mSmmFwBlockService->Read (
                                mSmmFwBlockService,
                                (UINTN) FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1),
                                FACTORY_COPY_HEADER_SIZE,
                                &ReadBufferSize,
                                (UINT8 *) ReadBuffer
                                );
  if (!EFI_ERROR (Status)) {
    NumberOfInteger = ReadBufferSize / sizeof (UINTN);
    for (Index = 0; Index < NumberOfInteger; Index++) {
      if (ReadBuffer[Index] != (UINTN) (-1)) {
        IsValid = TRUE;
        break;
      }
    }
  }

  FreePool (ReadBuffer);
  return IsValid;
}

/**
  This funciotn uses to export factory default setting to input buffer.

  @param[in, out] DataSize       On input, the size in bytes of the return Data buffer.
                                 On output the size of data returned in Data buffer.
  @param[out]     Data           The buffer to return the contents of the variable.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_INVALID_PARAMETER  Some input parameter is invalid.
  @retval EFI_BUFFER_TOO_SMALL   The VariableStoreLength is too small for the result.  DataSize has
                                 been updated with the size needed to complete the request.
  @return Others                 Other error causes merge factory default setting failed.
**/
EFI_STATUS
ExportFactoryDefaultSetting (
  IN OUT UINTN                         *DataSize,
  OUT    UINT8                         *Data
  )
{
  EFI_STATUS                            Status;
  UINTN                                 HeaderSize;
  UINTN                                 ReadDataSize;
  UINTN                                 RequiredDataSize;
  UINT8                                *HeaderBuffer;
  UINT8                                *FactoryDefaultBuffer;
  UINT64                                FactoryCopySize;

  FactoryDefaultBuffer = NULL;
  HeaderBuffer = NULL;

  if (DataSize == NULL || Data == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FactoryCopySize = FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);

  RequiredDataSize = (UINTN) FactoryCopySize - FACTORY_COPY_HEADER_SIZE;
  if (*DataSize < RequiredDataSize) {
    *DataSize = RequiredDataSize;
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Read original firmware volume header and variable store header
  //
  HeaderSize = sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY) + GetVariableStoreHeaderSize ();
  HeaderBuffer = AllocatePool (HeaderSize);
  if (HeaderBuffer == NULL) {
    ASSERT (HeaderBuffer != NULL);
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  Status = mSmmFwBlockService->Read (
                                mSmmFwBlockService,
                                (UINTN) FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_WORKING, 1),
                                0,
                                &HeaderSize,
                                HeaderBuffer
                                );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Read whole factory store data except for factory default header, firmware volume header and variable store header.
  //
  ReadDataSize = RequiredDataSize - HeaderSize;
  FactoryDefaultBuffer = AllocatePool (ReadDataSize);
  if (FactoryDefaultBuffer == NULL) {
    ASSERT (FactoryDefaultBuffer != NULL);
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Status = mSmmFwBlockService->Read (
                                mSmmFwBlockService,
                                (UINTN) FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1),
                                FACTORY_COPY_HEADER_SIZE + HeaderSize,
                                &ReadDataSize,
                                FactoryDefaultBuffer
                                );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  CopyMem (Data, HeaderBuffer, HeaderSize);
  CopyMem (Data + HeaderSize, FactoryDefaultBuffer, RequiredDataSize - HeaderSize);
  *DataSize = RequiredDataSize;

Done:

  if (FactoryDefaultBuffer != NULL) {
    FreePool (FactoryDefaultBuffer);
  }

  if (HeaderBuffer != NULL) {
    FreePool (HeaderBuffer);
  }

  return Status;
}

/**
  According to restore type to collect factory default settings and restore these settings
  in output buffer.

  @param[in]      RestoreType           Factory copy restore type.
  @param[in, out] BufferSize            On input, the size in bytes of the return BufferSize.
                                        On output the size of data returned in BufferSize.
  @param[out]     FactoryDefaultBuffer  The buffer to return the contents of the factory default settings.

  @retval EFI_SUCCESS                   The function completed successfully.
  @retval EFI_INVALID_PARAMETER         Some input parameter is invalid.
  @retval EFI_BUFFER_TOO_SMALL          The BufferSize is too small for the result.  BufferSize has
                                        been updated with the size needed to complete the request.
  @retval EFI_NOT_READY                 The contain of factory copy region is invalid.
  @return Others                        Other error cause collect factory default setting failed.
**/
EFI_STATUS
CollectFactoryDefaultSetting (
  IN     UINTN                      RestoreType,
  IN OUT UINTN                     *BufferSize,
  OUT    UINT8                     *FactoryDefaultBuffer
  )
{
  EFI_STATUS                        Status;

  if (RestoreType != FACTORY_COPY_RESTORE_WITH_CLEARING_ALL_SETTINGS &&
      RestoreType != FACTORY_COPY_RESTORE_WITH_RESERVING_OTHER_SETTINGS) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == NULL || FactoryDefaultBuffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsFacotryCopyValid ()) {
    return EFI_NOT_READY;
  }

  Status = EFI_INVALID_PARAMETER;
  switch (RestoreType) {

  case FACTORY_COPY_RESTORE_WITH_CLEARING_ALL_SETTINGS:
    Status = ExportFactoryDefaultSetting (BufferSize, FactoryDefaultBuffer);
    break;

  case FACTORY_COPY_RESTORE_WITH_RESERVING_OTHER_SETTINGS:
    Status = ExportMergedFactoryDefaultSetting (BufferSize, FactoryDefaultBuffer);
    break;

  }

  return Status;
}

/**
  According to restore type to restore factory default.

  @param[in] RestoreType          Input restore type. this funtion only supports FACTORY_COPY_RESTORE_WITH_CLEARING_ALL_SETTINGS
                                  and FACTORY_COPY_RESTORE_WITH_RESERVING_OTHER_SETTINGS. Other restore type will return
                                  EFI_INVALId_PARAMETER.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Any input parameter is invalid.
  @return Others                  Other error cause collect factory default setting failed.

**/
//[-start-161215-IB10860217-modify]//
EFI_STATUS
RestoreFactory (
  IN  UINTN     RestoreType
  )
{
  EFI_STATUS                              Status;
//  UINTN                                   EraseSize;
//  UINTN                                   WriteSize;
  UINTN                                   BufferSize;
//  UINTN                                   WriteCount;
//  BOOLEAN                                 WriteSuccess;
//  UINT8                                  *Buffer;
//  UINT64                                  FactoryCopySize;
//  UINT64                                  VariableAddr;
//  UINT64                                  VariableSize;
  UINT8                                   DefaultAthenData;
  
  if (RestoreType != FACTORY_COPY_RESTORE_WITH_CLEARING_ALL_SETTINGS &&
      RestoreType != FACTORY_COPY_RESTORE_WITH_RESERVING_OTHER_SETTINGS) {
    return EFI_INVALID_PARAMETER;
  }

  BufferSize = 1;
  DefaultAthenData = 1;
  
  Status = CommonSetVariable  (
             EFI_RESTORE_FACOTRY_DEFAULT_NAME,
             &gEfiGenericVariableGuid,
             EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS |
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS,
             BufferSize,
             &DefaultAthenData
             );

//  VariableSize = FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_WORKING, 1);
//  VariableAddr = FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_WORKING, 1) ;
//
//  FactoryCopySize = FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);
//
//  ASSERT (FactoryCopySize <= VariableSize);
//  BufferSize = (UINTN) VariableSize;
//  Buffer = AllocatePool (BufferSize);
//  if (Buffer == NULL) {
//    ASSERT (Buffer != NULL);
//    return EFI_OUT_OF_RESOURCES;
//  }
//  SetMem (Buffer, BufferSize, 0xFF);
//
//  Status = CollectFactoryDefaultSetting (RestoreType, &BufferSize, Buffer);
//  if (EFI_ERROR (Status)) {
//    FreePool (Buffer);
//    return Status;
//  }
//
//  WriteSuccess = FALSE;
//  WriteCount   = 0;
//  do {
//    EraseSize = (UINTN) VariableSize;
//    Status = mSmmFwBlockService->EraseBlocks (
//                                  mSmmFwBlockService,
//                                  (UINTN) VariableAddr,
//                                  (UINTN *) &EraseSize
//                                  );
//    if (!EFI_ERROR (Status)) {
//      WriteSize = BufferSize;
//      Status = mSmmFwBlockService->Write (
//                                    mSmmFwBlockService,
//                                    (UINTN) VariableAddr,
//                                    (UINTN *) &WriteSize,
//                                    Buffer
//                                    );
//      if (!EFI_ERROR (Status)) {
//        WriteSuccess = TRUE;
//      }
//    }
//    WriteCount++;
//  } while (WriteCount < 100 && !WriteSuccess);
//
//  FreePool (Buffer);

  return Status;
}
//[-end-161215-IB10860217-modify]//

/**
  The implementation of IHISI function 49h type 3. Restore variable data from factory copy region and
  clear all of variable data in variable store.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval Others                  Other error cause restoring factory copy failed.
**/
STATIC
EFI_STATUS
RestoreFactoryCopyWithClearingAllSettings (
  VOID
  )
{
  EFI_STATUS              Status;

  Status = RestoreFactory (FACTORY_COPY_RESTORE_WITH_CLEARING_ALL_SETTINGS);

  if (EFI_ERROR (Status)) {
    Status = IHISI_ACCESS_PROHIBITED;
  }

  return Status;
}

/**
  The implementation of IHISI function 49h type 4. Clearing all of secure boot settings in variable store and then using
  then using variable data contained in factory copy region to update variable. All of other variables (not secure boot
  relative variables or variables are located in factory copy region) will not be cleared.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval Others                  Other error cause restoring factory copy failed.
**/
STATIC
EFI_STATUS
RestoreFactoryCopyWithReservingOtherSettings (
  VOID
  )
{
  EFI_STATUS     Status;

  Status = RestoreFactory (FACTORY_COPY_RESTORE_WITH_RESERVING_OTHER_SETTINGS);

  if (EFI_ERROR (Status)) {
    Status = IHISI_ACCESS_PROHIBITED;
  }

  return Status;
}

EFI_STATUS
CommonRegionDataCommunication (
  VOID
  )
/*++

Routine Description:

  The entry point of IHISI function 49h.

Arguments:

  None

Returns:

  EFI_SUCCESS               - The function completed successfully
  EFI_UNSUPPORTED           - The type of function 49H is unsupported.
  Other                     - Other error cause in this function.

--*/
{
  EFI_STATUS                     Status;
  UINTN                          Index;
  UINT8                          DataType;
  INPUT_DATA_STRUCTURE          *InputData;
  OUTPUT_DATA_STRUCTURE         *OutputData;

  InputData  = (INPUT_DATA_STRUCTURE *) (UINTN) IhisiReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RCX);
  OutputData = (OUTPUT_DATA_STRUCTURE *) (UINTN) IhisiReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RCX);
  if (BufferOverlapSmram ((VOID *) InputData, sizeof (INPUT_DATA_STRUCTURE)) ||
      BufferOverlapSmram ((VOID *) OutputData, sizeof (OUTPUT_DATA_STRUCTURE))) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }
  DataType = InputData->DataType;
  Status = InitializeOutputData (OutputData);
  ASSERT_EFI_ERROR (Status);

  for (Index = 0; Index < sizeof (DataCommunicationSubFun) / sizeof (COMMON_REGION_COMMUNICATION_FUNCTION_TABLE); Index++) {
    if (DataType == DataCommunicationSubFun[Index].DataType) {
      Status = DataCommunicationSubFun[Index].Function ();
      break;
    }
  }

  if (Index == sizeof (DataCommunicationSubFun) / sizeof (COMMON_REGION_COMMUNICATION_FUNCTION_TABLE) || EFI_ERROR (Status)) {
    OutputData->Status = UNSUPPORTED_TYPE;
    Status = IHISI_UNSUPPORTED_FUNCTION;
  }

  return Status;
}

EFI_STATUS
InstallCommonRegionCommunicationServices (
  VOID
  )
{
  EFI_STATUS              Status;
  IHISI_REGISTER_TABLE   *SubFuncTable;
  UINT16                  TableCount;

  if (!FeaturePcdGet (PcdH2OSecureBootSupported)) {
    return EFI_SUCCESS;
  }

  SubFuncTable = COMMON_REGION_REGISTER_TABLE;
  TableCount = sizeof(COMMON_REGION_REGISTER_TABLE)/sizeof(COMMON_REGION_REGISTER_TABLE[0]);
  Status = RegisterIhisiSubFunction (SubFuncTable, TableCount);
  if (EFI_ERROR(Status)) {
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}

/** @file
  Implements CSE Variable Storage Services and installs
  an instance of the VariableStorage Runtime DXE protocol.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2017 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#include "CseVariableStorage.h"
//[-start-170620-IB07400876-add]//
#ifdef AUTO_RUN_DATA_CLEAR_WORKAROUND
#include <Library/CmosLib.h>
#include <ChipsetCmos.h>
#endif
//[-end-170620-IB07400876-add]//
//
// Module globals
//
CSE_VARIABLE_FILE_INFO        *mCseVariableFileInfo[CseVariableFileTypeMax];

//[-start-160816-IB07220129-add]//
MBP_SMM_TRUSTED_KEY            mSmmTrustedKeyInternal;
//[-end-160816-IB07220129-add]//
//[-start-160812-IB07220128-modify]//
VARIABLE_STORAGE_PROTOCOL_EX   mCseVariableStorageProtocol = {
  CseVariableStorageGetId,
  CseVariableStorageGetVariable,
  CseVariableStorageGetAuthenticatedVariable,
  CseVariableStorageGetNextVariableName,
  CseVariableStorageGetStorageUsage,
  CseVariableStorageGetAuthenticatedSupport,
  CseVariableStorageSetVariable,
  CseVariableStorageWriteServiceIsReady,
  CseVariableStorageRegisterWriteServiceReadyCallback,
  CseVariableStorageGarbageCollect,
  NULL,
//[-start-160816-IB07220129-add]//
  &mSmmTrustedKeyInternal
//[-end-160816-IB07220129-add]//
};
//[-end-160812-IB07220128-modify]//

BOOLEAN mIsEmmcBoot = FALSE;

/**
  Performs common initialization needed for this module.

  @param  None

  @retval EFI_SUCCESS  The module was initialized successfully.
  @retval Others       The module could not be initialized.
**/
EFI_STATUS
EFIAPI
CseVariableStorageCommonInitialize (
  VOID
  )
{
  EFI_STATUS              Status;
  UINT8                   BootMedia;
  EFI_HOB_GUID_TYPE       *GuidHobPtr;
  MBP_CURRENT_BOOT_MEDIA  *BootMediaData;
//[-start-160812-IB07220128-add]//
  VOID                    *Interface;
//[-end-160812-IB07220128-add]//
  CSE_VARIABLE_FILE_INFO  *CseVariableFileInfo;
  UINTN                   Index;

//[-start-160812-IB07220128-add]//
  Status = gBS->LocateProtocol (&gVariableStorageProtocolGuid, NULL, &Interface);
  if (!EFI_ERROR (Status)) {
    //
    // CseVariableFileInfo had been created before
    //
    CopyMem (mCseVariableFileInfo, ((VARIABLE_STORAGE_PROTOCOL_EX*) Interface)->CseVariableFileInfoPtr, sizeof(mCseVariableFileInfo));
    CseVariableFileInfo = AllocateRuntimeZeroPool (sizeof (CSE_VARIABLE_FILE_INFO) * CseVariableFileTypeMax);
    for (Index = 0; Index < CseVariableFileTypeMax; Index++) {
      CopyMem (CseVariableFileInfo + Index, mCseVariableFileInfo[Index], sizeof (CSE_VARIABLE_FILE_INFO));
      mCseVariableFileInfo[Index] = CseVariableFileInfo + Index;
    }
    mCseVariableStorageProtocol.CseVariableFileInfoPtr = mCseVariableFileInfo;

    //
    // Point to RT instance
    //
    mSmmTrustedKey = ((VARIABLE_STORAGE_PROTOCOL_EX*) Interface)->SmmTrustedKey;
    return EFI_SUCCESS;
  }
//[-end-160812-IB07220128-add]//
  
  //
  // Allocate and initialize the global variable structure
  //
  Status = InitializeCseStorageGlobalVariableStructures (mCseVariableFileInfo, TRUE);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

//[-start-160812-IB07220128-add]//
  mCseVariableStorageProtocol.CseVariableFileInfoPtr = mCseVariableFileInfo;
//[-end-160812-IB07220128-add]//

//[-start-160816-IB07220129-add]//
  Status = HeciGetSMMTrustedKey (&mSmmTrustedKeyInternal);
  if (!EFI_ERROR (Status)) {
    mSmmTrustedKey = &mSmmTrustedKeyInternal;
  } else {
    DEBUG ((EFI_D_ERROR, "Unable to get the SMM trusted key. Cannot send HECI2 transactions.\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
//[-end-160816-IB07220129-add]//
  
  //
  // Establish the CSE variable stores
  //  - Create an enabled variable store if it does not exist
  //  - Load the variable header data regions needed to find variables
  //
  Status = EstablishAndLoadCseVariableStores (mCseVariableFileInfo, TRUE);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Error returned from EstablishAndLoadCseVariableStores()\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  DEBUG ((
    EFI_D_INFO,
    "  CSE Pre-memory File Enabled: %c\n  CSE Pre-memory Store Established:  %c\n  CSE NVM File Enabled: %c\n  CSE NVM Store Established: %c\n\n",
    (mCseVariableFileInfo[CseVariableFileTypePreMemoryFile]->FileEnabled ? 'T' : 'F'),
    (mCseVariableFileInfo[CseVariableFileTypePreMemoryFile]->FileStoreEstablished ? 'T' : 'F'),
    (mCseVariableFileInfo[CseVariableFileTypePrimaryIndexFile]->FileEnabled ? 'T' : 'F'),
    (mCseVariableFileInfo[CseVariableFileTypePrimaryIndexFile]->FileStoreEstablished ? 'T' : 'F')
    ));

  GuidHobPtr  = GetFirstGuidHob (&gEfiBootMediaHobGuid);
  if (GuidHobPtr == NULL) {
    DEBUG ((DEBUG_ERROR, "CseVariableStorage Error: Boot Media HOB does not exist!\n"));
    ASSERT (GuidHobPtr != NULL);
    return FALSE;
  }

  BootMediaData = (MBP_CURRENT_BOOT_MEDIA*) GET_GUID_HOB_DATA (GuidHobPtr);
  BootMedia = (UINT8) BootMediaData->PhysicalData;
  if (BootMedia != BOOT_FROM_SPI) {
    mIsEmmcBoot = TRUE;
  }

  return EFI_SUCCESS;
}

/**
  Reclaims the variable store in the pre-memory file.

  @param  None

  @retval EFI_SUCCESS             The pre-memory file reclaim operation was successful.
  @retval Others                  The pre-memory file reclaim operation failed.
**/
EFI_STATUS
ReclaimPreMemoryFileVariableStore (
  VOID
  )
{
  EFI_STATUS                     Status;
  UINTN                          PreMemoryFileSize  = 0;
  UINT32                         TotalBufferSize    = 0;
  UINT32                         VariableStoreBaseOffset   = 0;
  UINTN                          VariableHeaderTotalSize   = 0;

  CHAR16                         *VariableName             = NULL;
  UINT8                          *ReclaimBuffer            = NULL;
  UINT8                          *SourceBuffer             = NULL;
  UINT8                          *VariableDestPtr          = NULL;
  UINT8                          *VariableDestDataPtr      = NULL;
  UINT8                          *VariableSourceDataPtr    = NULL;
  VARIABLE_NVM_HEADER            *NextVariable             = NULL;
  VARIABLE_NVM_HEADER            *VariableSourcePtr        = NULL;
  VARIABLE_NVM_STORE_HEADER      *ReclaimBufferStoreHeader = NULL;

  VARIABLE_NVM_STORE_INFO        SourceVariableStoreInfo;

  if (!mCseVariableFileInfo[CseVariableFileTypePreMemoryFile]->FileEnabled ||
    !mCseVariableFileInfo[CseVariableFileTypePreMemoryFile]->FileStoreEstablished) {
    return EFI_NOT_READY;
  }

  TotalBufferSize = mCseVariableFileInfo[CseVariableFileTypePreMemoryFile]->FileStoreMaximumSize;
  ASSERT (TotalBufferSize > 0);

  //
  // Allocate a source and reclaim buffer as the working reclaim area
  //
  SourceBuffer = AllocateZeroPool (TotalBufferSize);
  if (SourceBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ReclaimBuffer = AllocateZeroPool (TotalBufferSize);
  if (ReclaimBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  GetCseVariableStoreFileOffset (CseVariableFileTypePreMemoryFile, &VariableStoreBaseOffset);

  Status = GetCseNvmFileSize (mCseVariableFileInfo[CseVariableFileTypePreMemoryFile]->FileName, &PreMemoryFileSize, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Error obtaining the pre-memory file size. Cannot reclaim the pre-memory file. Status = %r\n", Status));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  //
  // Populate the source buffer with the pre-memory file contents
  //
  Status = ReadCseNvmFile (
             mCseVariableFileInfo[CseVariableFileTypePreMemoryFile]->FileName,
             VariableStoreBaseOffset,
             SourceBuffer,
             &PreMemoryFileSize,
             NULL
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Error occurred reading the pre-memory file for reclaim. Cannot reclaim the pre-memory file. Status = %r\n", Status));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  // Note: The actual file size might be slightly less than the maximum size due to variable data alignment
  ASSERT (PreMemoryFileSize <= TotalBufferSize);

  SourceVariableStoreInfo.VariableStoreHeader = (VARIABLE_NVM_STORE_HEADER *) SourceBuffer;

  Status = IsAuthenticatedVariableStore (SourceVariableStoreInfo.VariableStoreHeader, &SourceVariableStoreInfo.AuthFlag);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Copy the variable store header
  //
  CopyMem (ReclaimBuffer, SourceVariableStoreInfo.VariableStoreHeader, sizeof (VARIABLE_NVM_STORE_HEADER));
  ReclaimBufferStoreHeader = (VARIABLE_NVM_STORE_HEADER *) ReclaimBuffer;
  VariableDestPtr = (UINT8 *) GetStartPointer (ReclaimBufferStoreHeader);

  //
  // Initialize new variable store properties subject to change
  //
  ReclaimBufferStoreHeader->VariableDataTotalLength    = 0;
  ReclaimBufferStoreHeader->VariableHeaderTotalEntries = 0;
  ReclaimBufferStoreHeader->VariableHeaderTotalLength  = 0;

  //
  // Reinstall all valid (active) variables
  //
  VariableSourcePtr = GetStartPointer (SourceVariableStoreInfo.VariableStoreHeader);
  VariableDestDataPtr = (UINT8 *) (
                          (UINTN) ReclaimBuffer +
                          (UINTN) mCseVariableFileInfo[CseVariableFileTypePreMemoryFile]->FileStoreMaximumSize -
                          1
                          );

  while (IsValidVariableHeader (VariableSourcePtr, GetEndPointer (SourceVariableStoreInfo.VariableStoreHeader))) {
    NextVariable = GetNextVariablePtr (&SourceVariableStoreInfo, VariableSourcePtr);

    if (VariableSourcePtr->State == VAR_ADDED || VariableSourcePtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
      VariableHeaderTotalSize = (UINTN) NextVariable - (UINTN) VariableSourcePtr;

      VariableSourceDataPtr = (UINT8 *) ((UINTN) SourceVariableStoreInfo.VariableStoreHeader + (UINTN) VariableSourcePtr->DataOffset);
      VariableDestDataPtr -= VariableSourcePtr->DataSize;
      VariableDestDataPtr -= NVM_GET_PAD_SIZE_N ((UINTN) VariableDestDataPtr);

      // Copy variable header
      CopyMem (VariableDestPtr, VariableSourcePtr, VariableHeaderTotalSize);
      ((VARIABLE_NVM_HEADER *) VariableDestPtr)->DataOffset = (UINT32) (
                                                                (UINTN) VariableDestDataPtr -
                                                                (UINTN) ReclaimBufferStoreHeader
                                                                );

      VariableDestPtr += VariableHeaderTotalSize;

      // Copy variable data
      CopyMem (VariableDestDataPtr, VariableSourceDataPtr, VariableSourcePtr->DataSize);

      // Update variable store properties
      VariableName = GetVariableNamePtr (VariableSourcePtr, SourceVariableStoreInfo.AuthFlag);

      ReclaimBufferStoreHeader->VariableHeaderTotalEntries++;
      ReclaimBufferStoreHeader->VariableDataTotalLength   += (UINT32) (
                                                               VariableSourcePtr->DataSize +
                                                               NVM_GET_PAD_SIZE_N (VariableSourcePtr->DataSize)
                                                               );
      ReclaimBufferStoreHeader->VariableHeaderTotalLength += (UINT32) (
                                                               GetVariableHeaderSize (SourceVariableStoreInfo.AuthFlag) +
                                                               StrSize (VariableName) +
                                                               NVM_GET_PAD_SIZE (StrSize (VariableName))
                                                               );
    }
    VariableSourcePtr = NextVariable;
  }

  CopyMem (
    (UINT8 *) (UINTN) mCseVariableFileInfo[CseVariableFileTypePreMemoryFile]->HeaderRegionBase,
    ReclaimBuffer,
    TotalBufferSize
    );

  Status = UpdateCseNvmFile (
             mCseVariableFileInfo[CseVariableFileTypePreMemoryFile]->FileName,
             VariableStoreBaseOffset,
             ReclaimBuffer,
             PreMemoryFileSize,
             TRUE,
             NULL
             );

  FreePool (ReclaimBuffer);
  FreePool (SourceBuffer);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Error occurred updating the pre-memory data file after reclaim. Status = %r.\n", Status));
  }

  return Status;
}

/**
  Updates the variable header in the index file.

  @param[in]  IndexFileWriteOffset           The offset in bytes in the index file of the variable header.
  @param[in]  TotalVariableHeaderUpdateSize  Total size in bytes that should be updated.
  @param[in]  VariableCseFileType            The CSE NVM variable file type.
  @param[in]  VariableHeader                 Pointer to the variable header that is being updated.

  @retval     EFI_SUCCESS                    The CSE NVM index file was updated successfully.
  @retval     Others                         The CSE NVM index file update operation failed.
**/
EFI_STATUS
EFIAPI
UpdateIndexFile (
  IN         UINT32                    IndexFileWriteOffset,
  IN         UINT32                    TotalVariableHeaderUpdateSize,
  IN         CSE_VARIABLE_FILE_TYPE    VariableCseFileType,
  IN  CONST  VARIABLE_NVM_HEADER       *VariableHeader
  )
{
  EFI_STATUS Status;
  UINT32     TotalUpdateSize;
  UINT32     VariableStoreBaseOffset;
  UINT8      *VariableStoreHeader;

  TotalUpdateSize         = 0;
  VariableStoreBaseOffset = 0;
  VariableStoreHeader     = NULL;

  if (VariableHeader == NULL || VariableCseFileType >= CseVariableFileTypeMax) {
    return EFI_INVALID_PARAMETER;
  }

  VariableStoreHeader = (UINT8 *) (UINTN) mCseVariableFileInfo[VariableCseFileType]->HeaderRegionBase;
  GetCseVariableStoreFileOffset (VariableCseFileType, &VariableStoreBaseOffset);

  if (PcdGetBool (PcdMonolithicCseNvmIndexFileUpdate)) {
    TotalUpdateSize = ((UINT32) ((UINTN) VariableHeader - (UINTN) VariableStoreHeader)) + TotalVariableHeaderUpdateSize;

    if (TotalUpdateSize <= HeciGetHeci2WriteBufferSize ()) {
      //
      // Update both the variable store header and the variable header in one write transaction (monolithic update)
      //
      return UpdateCseNvmFile (
               mCseVariableFileInfo[VariableCseFileType]->FileName,
               VariableStoreBaseOffset,
               VariableStoreHeader,
               TotalUpdateSize,
               FALSE,
               NULL
               );
    }
  }

  //
  // Update the variable store header (micro update #1)
  //
  Status = UpdateCseNvmFile (
             mCseVariableFileInfo[VariableCseFileType]->FileName,
             VariableStoreBaseOffset,
             VariableStoreHeader,
             sizeof (VARIABLE_NVM_STORE_HEADER),
             FALSE,
             NULL
             );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Update the variable header (micro update #2)
  //
  return UpdateCseNvmFile (
           mCseVariableFileInfo[VariableCseFileType]->FileName,
           IndexFileWriteOffset,
           (UINT8 *) VariableHeader,
           TotalVariableHeaderUpdateSize,
           FALSE,
           NULL
           );
}

/**
  Writes the CSE NVM variable with the supplied data. These are the same arguments as the EFI Variable services.

  @param[in]  CseFileType             The CSE data file type.
  @param[in]  CseDataFileName         Pointer to a caller allocated buffer for the CSE data file name.
                                      The buffer size must be of at least CSE_MAX_NVM_FILE_NAME_LENGTH bytes.
  @param[in]  VariableDataFileNumber  The data file number for the CSE NVM variable.

  @retval     EFI_SUCCESS             The name was copied to the buffer successfully.
  @retval     Others                  The name construction operation failed.
**/
EFI_STATUS
EFIAPI
ConstructDataFileName (
  IN    CSE_VARIABLE_FILE_TYPE  CseFileType,
  IN    CHAR8                   *CseDataFileName,
  IN    UINT32                  VariableDataFileNumber
  ) {
  EFI_STATUS    Status;

  if (CseDataFileName == NULL || CseFileType >= CseVariableFileTypeMax) {
    return EFI_INVALID_PARAMETER;
  }
  ZeroMem (CseDataFileName, CSE_MAX_NVM_FILE_NAME_LENGTH);

  if (CseFileType == CseVariableFileTypePreMemoryFile) {
    //
    // The pre-memory file is a single file that serves as both the index file
    // and the data file. Therefore, the data filename is the index filename.
    //
    AsciiStrCpyS (CseDataFileName, CSE_MAX_NVM_FILE_NAME_LENGTH, mCseVariableFileInfo[CseFileType]->FileName);
  } else {
    Status = BuildCseDataFileName (VariableDataFileNumber, CseDataFileName);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

/**
  Deletes a variable on CSE NVM.

  @param[in]  CseFileType          The CSE data file type.
  @param[in]  VariableHeader       Pointer to the variable header that should be deleted.
  @param[in]  VariableStoreInfo    Pointer to a structure that maintains variable store information.

  @retval     EFI_SUCCESS          The CSE NVM variable was deleted successfully.
  @retval     Others               The CSE NVM variable deletion operation failed.
**/
EFI_STATUS
EFIAPI
DeleteCseNvmFileVariable (
  IN         CSE_VARIABLE_FILE_TYPE    CseFileType,
  IN         VARIABLE_NVM_HEADER       *VariableHeader,
  IN  CONST  VARIABLE_NVM_STORE_INFO   *VariableStoreInfo
  )
{
  EFI_STATUS Status;
  UINT8      EmptyBuffer               = 0;
  UINT32     VariableIndexWriteOffset  = 0;
  UINT32     VariableStoreBaseOffset   = 0;

  CHAR8      VariableDataFileName[CSE_MAX_NVM_FILE_NAME_LENGTH];

  if (VariableHeader == NULL || VariableStoreInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  GetCseVariableStoreFileOffset (CseFileType, &VariableStoreBaseOffset);
  VariableIndexWriteOffset = NVM_HEADER_ALIGN ((UINT32) (VariableStoreBaseOffset +
                               ((UINTN) (VariableHeader) -
                               (UINTN) VariableStoreInfo->VariableStoreHeader))
                               );

  DEBUG ((
  EFI_D_INFO,
    "Deleting CSE NVM variable in data file. Total data length before delete = %d bytes. Data size of deleted variable = %d bytes.\n",
    VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength,
    VariableHeader->DataSize
    ));

  Status = ConstructDataFileName (CseFileType, &VariableDataFileName[0], VariableHeader->DataOffset);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  } else if (&VariableDataFileName[0] == NULL) {
    ASSERT (&VariableDataFileName[0] != NULL);
    return EFI_OUT_OF_RESOURCES;
  }
  DEBUG ((EFI_D_INFO, "CSE NVM Data Filename: %a\n", &VariableDataFileName[0]));

  //
  // Update variable store properties
  //
  // Note: VariableHeaderTotalEntries is not decremented. Newly written variables must be given unique file names.
  // Note: Variable headers occupy space even after deletion until a potential reclaim operation.
  //
  VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength -= VariableHeader->DataSize;
  VariableStoreInfo->VariableStoreHeader->Size = sizeof (VARIABLE_NVM_STORE_HEADER) +
                                                   VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength +
                                                   VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalLength;
  VariableHeader->State &= VAR_DELETED;

  //
  // Update the variable store header and variable header on CSE NVM
  //
  Status = UpdateIndexFile (
             VariableIndexWriteOffset,
             (UINT32) GetVariableHeaderSize (VariableStoreInfo->AuthFlag),
             CseVariableFileTypePrimaryIndexFile,
             VariableHeader
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Error occurred updating the index file. Status = %r.\n", Status));
    return Status;
  }

  //
  // Delete the data file
  //
  Status = UpdateCseNvmFile (
             &VariableDataFileName[0],
             0,
             (UINT8 *) &EmptyBuffer,
             0,
             TRUE,
             NULL
             );
  if (!EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Successfully deleted the CSE NVM file.\n"));
  } else {
    DEBUG ((EFI_D_ERROR, "Error occurred deleting the CSE NVM data file.\n"));
  }

  return Status;
}

/**
  Deletes a variable the CSE NVM pre-memory file.

  @param[in]  CseFileType          The CSE data file type.
  @param[in]  VariableHeader       Pointer to the variable header that should be deleted.
  @param[in]  VariableStoreInfo    Pointer to a structure that maintains variable store information.

  @retval     EFI_SUCCESS          The pre-memory file variable was deleted successfully.
  @retval     Others               The pre-memory file variable deletion operation failed.
**/
EFI_STATUS
EFIAPI
DeleteCsePreMemoryFileVariable (
  IN         CSE_VARIABLE_FILE_TYPE    CseFileType,
  IN         VARIABLE_NVM_HEADER       *VariableHeader,
  IN  CONST  VARIABLE_NVM_STORE_INFO   *VariableStoreInfo
  )
{
  UINT32    VariableIndexWriteOffset   = 0;
  UINT32    VariableStoreBaseOffset    = 0;

  if (VariableHeader == NULL || VariableStoreInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  GetCseVariableStoreFileOffset (CseFileType, &VariableStoreBaseOffset);
  VariableIndexWriteOffset = NVM_HEADER_ALIGN ((UINT32) (VariableStoreBaseOffset +
                               ((UINTN) (VariableHeader) -
                               (UINTN) VariableStoreInfo->VariableStoreHeader))
                               );

  DEBUG ((
    EFI_D_INFO,
    "Deleting CSE NVM variable in pre-memory file. Total data length before delete = %d bytes. Data size of deleted variable = %d bytes.\n",
    VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength,
    VariableHeader->DataSize
    ));

  // Note: Variable store data size properties are not updated in the pre-memory file until the reclaim operation.
  // Note: VariableHeaderTotalEntries is not readjusted until the reclaim operation as the variable headers are still
  //       present in the variable store after deletion.
  VariableHeader->State &= VAR_DELETED;

  //
  // Update the state in the variable header
  //
  return UpdateCseNvmFile (
           mCseVariableFileInfo[CseVariableFileTypePreMemoryFile]->FileName,
           VariableIndexWriteOffset,
           (UINT8 *) VariableHeader,
           GetVariableHeaderSize (VariableStoreInfo->AuthFlag),
           FALSE,
           NULL
           );
  // Note: In the pre-memory file, deleted data is marked and actually removed during the reclaim operation.
}

/**
  Sets authenticated fields in a CSE NVM variable header.

  @param[in]  AuthenticatedVariableHeader      Pointer to an authenticated CSE NVM variable header.
  @param[in]  AuthenticatedVariableFieldTrack  Pointer to a structure which maintains authenticated field values.

  @retval     EFI_SUCCESS                      The authenticated fields in the variable header were set successfully.
  @retval     EFI_INVALID_PARAMETER            A required pointer is invalid.

**/
EFI_STATUS
EFIAPI
SetAuthenticatedFields (
  IN         AUTHENTICATED_VARIABLE_NVM_HEADER      *AuthenticatedVariableHeader,
  IN  CONST  CSE_VARIABLE_AUTHENTICATED_FIELD_TRACK *AuthenticatedVariableFieldTrack
  ) {
  if (AuthenticatedVariableHeader   == NULL ||
    AuthenticatedVariableFieldTrack == NULL ||
    AuthenticatedVariableFieldTrack->TimeStamp == NULL
    )  {
    return EFI_INVALID_PARAMETER;
  }

  AuthenticatedVariableHeader->MonotonicCount = AuthenticatedVariableFieldTrack->MonotonicCount;
  AuthenticatedVariableHeader->PubKeyIndex    = AuthenticatedVariableFieldTrack->PubKeyIndex;

  ZeroMem (&AuthenticatedVariableHeader->TimeStamp, sizeof (EFI_TIME));
  CopyMem (&AuthenticatedVariableHeader->TimeStamp, AuthenticatedVariableFieldTrack->TimeStamp, sizeof (EFI_TIME));

  return EFI_SUCCESS;
}

/**
  Writes a new variable to CSE NVM.

  @param[in]  CseFileType                      The CSE data file type.
  @param[in]  VariableInformationTrack         Pointer to a structure that maintains common variable information.
  @param[in]  VariableStoreInfo                Pointer to a structure that maintains variable store information.
  @param[in]  AuthenticatedVariableFieldTrack  Pointer to a structure which maintains authenticated field values.

  @retval     EFI_SUCCESS                      The variable write operation was successful.
  @retval     Others                           The variable write operation failed.
**/
EFI_STATUS
EFIAPI
WriteNewVariable (
  IN         CSE_VARIABLE_FILE_TYPE                  CseFileType,
  IN  CONST  CSE_VARIABLE_INFORMATION_TRACK          *VariableInformationTrack,
  IN         VARIABLE_NVM_STORE_INFO                 *VariableStoreInfo,
  IN  CONST  CSE_VARIABLE_AUTHENTICATED_FIELD_TRACK  *VariableAuthenticatedFieldTrack OPTIONAL
  ) {
  EFI_STATUS Status;
  UINT32     NewVariableTotalSize     = 0; ///< The total variable size (header + data)
  UINT32     VariableDataWriteOffset  = 0; ///< The offset from the beginning of the data file to the data
  UINT32     VariableDataFileNumber   = 0; ///< The variable data file number (if data is stored in individual files)
  UINT32     VariableDataTotalSize    = 0; ///< The total variable data size including alignment padding if applicable
  UINT32     VariableHeaderTotalSize  = 0; ///< The total variable header size including the variable name
  UINT32     VariableIndexWriteOffset = 0; ///< The offset from the beginning of the index file to the variable header
  UINT32     VariablePaddingSize      = 0; ///< Size in bytes of padding needed to align variable data to a dword boundary.
                                           ///< This is only needed in the pre-memory file.
  UINT32     VariableStoreBaseOffset  = 0; ///< The offset from the beginning of the file to the variable store

  AUTHENTICATED_VARIABLE_NVM_HEADER   *AuthenticatedVariableHeader     = NULL;
  VARIABLE_NVM_HEADER                 *NewVariableHeader               = NULL;

  INT32                               TotalVariableHeaderRemainingSize = 0;
  INT32                               TotalVariableStoreRemainingSize  = 0;

  CHAR8                               VariableDataFileName[CSE_MAX_NVM_FILE_NAME_LENGTH];
//[-start-170620-IB07400876-add]//
#ifdef AUTO_RUN_DATA_CLEAR_WORKAROUND
  UINT8                               CmosData;
#endif
//[-end-170620-IB07400876-add]//

  if (VariableInformationTrack  == NULL || VariableStoreInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Make sure a new variable does not contain invalid attributes
  //
  if (VariableInformationTrack->DataSize == 0 ||
    (VariableInformationTrack->Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == 0
    ) {
    DEBUG ((EFI_D_ERROR, "Invalid attributes specified for a new variable.\n"));
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((EFI_D_INFO, "A new variable is being written.\n"));

  GetCseVariableStoreFileOffset (CseFileType, &VariableStoreBaseOffset);

  VariableDataTotalSize = (UINT32) (VariableInformationTrack->DataSize);

  VariableHeaderTotalSize = (UINT32) (
                              GetVariableHeaderSize (VariableStoreInfo->AuthFlag) +
                              StrSize (VariableInformationTrack->VariableName) +
                              NVM_GET_PAD_SIZE (StrSize (VariableInformationTrack->VariableName))
                              );
  NewVariableTotalSize = VariableHeaderTotalSize + VariableDataTotalSize;

  VariableIndexWriteOffset = NVM_HEADER_ALIGN (
                               (UINT32) (
                                 VariableStoreBaseOffset +
                                 ((UINTN) GetEndPointer (VariableStoreInfo->VariableStoreHeader) -
                                 (UINTN) VariableStoreInfo->VariableStoreHeader)
                                 )
                               );

  //
  // Calculate the space remaining in the variable header region
  //
  TotalVariableStoreRemainingSize = mCseVariableFileInfo[CseFileType]->FileStoreMaximumSize -
                                      sizeof (VARIABLE_NVM_STORE_HEADER) -
                                      VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalLength -
                                      VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength;

  if (CseFileType == CseVariableFileTypePreMemoryFile) {
    TotalVariableHeaderRemainingSize = TotalVariableStoreRemainingSize;
  } else {
    TotalVariableHeaderRemainingSize = (INT32) (
                                         PcdGet32 (PcdCseNvmDataFileMaximumSize) -
                                         sizeof (VARIABLE_NVM_STORE_HEADER) -
                                         VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalLength
                                         );
  }

  //
  // Check for pre-memory file reclaim conditions
  //
  if (((mCseVariableFileInfo[CseFileType]->FileStoreMaximumSize - VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength) < VariableInformationTrack->DataSize ||
    TotalVariableHeaderRemainingSize < (INT32) VariableHeaderTotalSize ||
    TotalVariableStoreRemainingSize < (INT32) NewVariableTotalSize)
  ) {
    if (CseFileType == CseVariableFileTypePreMemoryFile) {
      DEBUG ((EFI_D_WARN, "Variable data exceeds available pre-memory file capacity. Attempting to recover space...\n"));

      DEBUG ((EFI_D_INFO, "Variable Store Info Before Reclaim:\n"));
      DEBUG ((EFI_D_INFO, "  Total Header Entries: %d\n", VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalEntries));
      DEBUG ((EFI_D_INFO, "  Total Header Length: %d\n",  VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalLength));
      DEBUG ((EFI_D_INFO, "  Total Data Length: %d\n",    VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength));

      // Perform a pre-memory file reclaim operation to attempt to recover space
      Status = ReclaimPreMemoryFileVariableStore ();

      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "An error occurred reclaiming the pre-memory file. The variable could not be updated.\n"));
        ASSERT_EFI_ERROR (Status);
        return Status;
      }

      DEBUG ((EFI_D_INFO, "Variable Store Info After Reclaim:\n"));
      DEBUG ((EFI_D_INFO, "  Total Header Entries: %d\n", VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalEntries));
      DEBUG ((EFI_D_INFO, "  Total Header Length: %d\n",  VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalLength));
      DEBUG ((EFI_D_INFO, "  Total Data Length: %d\n",    VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength));

      // Check if enough space is available after the reclaim
      TotalVariableStoreRemainingSize = mCseVariableFileInfo[CseFileType]->FileStoreMaximumSize -
                                          sizeof (VARIABLE_NVM_STORE_HEADER) -
                                          VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalLength -
                                          VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength;

      TotalVariableHeaderRemainingSize = TotalVariableStoreRemainingSize;

      if (((mCseVariableFileInfo[CseFileType]->FileStoreMaximumSize - VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength) < VariableInformationTrack->DataSize ||
        TotalVariableHeaderRemainingSize < (INT32) VariableHeaderTotalSize ||
        TotalVariableStoreRemainingSize < (INT32) NewVariableTotalSize)
        ) {
          DEBUG ((EFI_D_ERROR, "Cannot save the variable. CSE NVM file space is at maximum capacity.\n"));
          return EFI_OUT_OF_RESOURCES;
      }
    } else {
      DEBUG ((EFI_D_ERROR, "Cannot save the variable. CSE NVM file space is at maximum capacity.\n"));
      return EFI_OUT_OF_RESOURCES;
    }
  }

  if (CseFileType == CseVariableFileTypePreMemoryFile) {
    VariableDataWriteOffset = (UINT32) (
                                mCseVariableFileInfo[CseFileType]->FileStoreMaximumSize -
                                VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength -
                                VariableInformationTrack->DataSize
                                );
    if (VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength == 0) {
      VariableDataWriteOffset--;
    }

    VariablePaddingSize = NVM_GET_PAD_SIZE_N (VariableDataWriteOffset);

    VariableDataWriteOffset -= VariablePaddingSize;
    VariableDataTotalSize   += VariablePaddingSize;
  } else {
    VariableDataFileNumber = VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalEntries;

    if (CSE_PRIMARY_NVM_MAX_DATA_FILES < 1 || VariableDataFileNumber >= CSE_PRIMARY_NVM_MAX_DATA_FILES) {
      DEBUG ((EFI_D_ERROR, "Cannot save new variable - maximum number of CSE NVM files reached.\n"));
//[-start-170620-IB07400876-add]//
#ifdef AUTO_RUN_DATA_CLEAR_WORKAROUND
      CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2); 
      CmosData |= (B_CMOS_VAR_DATA_CLEAR_WORKAROUND);
      WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2, CmosData); 
#endif
//[-end-170620-IB07400876-add]//
      return EFI_OUT_OF_RESOURCES;
    }
  }

  DEBUG ((EFI_D_INFO, "New variable total size (header + data): %d bytes.\n", NewVariableTotalSize));
  DEBUG ((EFI_D_INFO, "Size available BEFORE adding the variable:\n"));
  DEBUG ((
  EFI_D_INFO,
    "  Total available in store: %d bytes of maximum %d bytes.\n",
    TotalVariableStoreRemainingSize,
    mCseVariableFileInfo[CseFileType]->FileStoreMaximumSize
    ));

  Status = ConstructDataFileName (CseFileType, &VariableDataFileName[0], VariableDataFileNumber);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  } else if (&VariableDataFileName[0] == NULL) {
    ASSERT (&VariableDataFileName[0] != NULL);
    return EFI_OUT_OF_RESOURCES;
  }
  DEBUG ((EFI_D_INFO, "CSE NVM Data Filename: %a\n", &VariableDataFileName[0]));

  NewVariableHeader = GetEndPointer (VariableStoreInfo->VariableStoreHeader);
  VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalEntries++;

  DEBUG ((EFI_D_INFO, "New variable header at memory address 0x%x\n", NewVariableHeader));

  NewVariableHeader->StartId    = VARIABLE_DATA;
  NewVariableHeader->State      = VAR_ADDED;
  NewVariableHeader->Attributes = VariableInformationTrack->Attributes;

  NewVariableHeader->DataSize   = (UINT32) VariableInformationTrack->DataSize;
  NewVariableHeader->NameSize   = (UINT32) StrSize (VariableInformationTrack->VariableName);

  if (CseFileType == CseVariableFileTypePreMemoryFile) {
    NewVariableHeader->DataOffset = VariableDataWriteOffset;
  } else {
    NewVariableHeader->DataOffset = VariableDataFileNumber;
  }

  StrCpyS (
    GetVariableNamePtr (NewVariableHeader, VariableStoreInfo->AuthFlag),
    NewVariableHeader->NameSize,
    VariableInformationTrack->VariableName
    );
  CopyMem (&NewVariableHeader->VendorGuid, VariableInformationTrack->VendorGuid, sizeof (EFI_GUID));

  if (VariableStoreInfo->AuthFlag) {
    AuthenticatedVariableHeader = (AUTHENTICATED_VARIABLE_NVM_HEADER *) NewVariableHeader;
  }

  if (VariableStoreInfo->AuthFlag && AuthenticatedVariableHeader != NULL) {
    if (VariableAuthenticatedFieldTrack == NULL) {
      ASSERT (VariableAuthenticatedFieldTrack != NULL);
      return EFI_INVALID_PARAMETER;
    }

    SetAuthenticatedFields (AuthenticatedVariableHeader, VariableAuthenticatedFieldTrack);
  }

  //
  // The EFI_VARIABLE_APPEND_WRITE attribute will never be set in the returned Attributes bitmask.
  //
  NewVariableHeader->Attributes = VariableInformationTrack->Attributes & (~EFI_VARIABLE_APPEND_WRITE);

  //
  // Update the variable store properties in the variable store header
  //
  VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalLength += VariableHeaderTotalSize;
  VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength   += VariableDataTotalSize;

  VariableStoreInfo->VariableStoreHeader->Size = sizeof (VARIABLE_NVM_STORE_HEADER) +
                                                   VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength +
                                                   VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalLength;

  DEBUG ((EFI_D_INFO, "Updated the local (memory) variable store for the new variable. Current data:\n"));

  DEBUG ((EFI_D_INFO, "  Total size of all variable headers: %d bytes\n", VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalLength));
  DEBUG ((EFI_D_INFO, "  Total size of variable data: %d bytes\n",         VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength));
  DEBUG ((EFI_D_INFO, "  Total number of variable entries: %d\n",         VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalEntries));

  DEBUG ((EFI_D_INFO, "Writing the CSE NVM data file...\n"));
  Status =  UpdateCseNvmFile (
              &VariableDataFileName[0],
              VariableDataWriteOffset,
              (UINT8 *) VariableInformationTrack->Data,
              VariableInformationTrack->DataSize,
              CseFileType != CseVariableFileTypePreMemoryFile,
              NULL
              );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Error occurred updating the data file. Variable not written.\n"));
    return Status;
  }

  DEBUG ((
  EFI_D_INFO,
    "Writing the new variable header to offset 0x%x in the CSE NVM file header region.\n",
    VariableIndexWriteOffset
    ));

  DEBUG ((EFI_D_INFO, "Updating the index file...\n"));
  return UpdateIndexFile (
           VariableIndexWriteOffset,
           VariableHeaderTotalSize,
           CseFileType,
           NewVariableHeader
           );
}

/**
  Updates an existing variable on CSE NVM.

  @param[in]  CseFileType                      The CSE data file type.
  @param[in]  VariableInformationTrack         Pointer to a structure that maintains common variable information.
  @param[in]  VariablePtrTrack                 Pointer to a structure that maintains variable header pointers.
  @param[in]  VariableStoreInfo                Pointer to a structure that maintains variable store information.
  @param[in]  AuthenticatedVariableFieldTrack  Pointer to a structure which maintains authenticated field values.

  @retval     EFI_SUCCESS                      The variable update operation was successful.
  @retval     Others                           The variable update operation failed.
**/
EFI_STATUS
EFIAPI
UpdateVariable (
  IN         CSE_VARIABLE_FILE_TYPE                  CseFileType,
  IN  CONST  CSE_VARIABLE_INFORMATION_TRACK          *VariableInformationTrack,
  IN  CONST  VARIABLE_NVM_POINTER_TRACK              *VariablePtrTrack,
  IN         VARIABLE_NVM_STORE_INFO                 *VariableStoreInfo,
  IN  CONST  CSE_VARIABLE_AUTHENTICATED_FIELD_TRACK  *VariableAuthenticatedFieldTrack OPTIONAL
  ) {
  EFI_STATUS Status;
  UINT32     VariableDataWriteOffset  = 0; ///< The offset from the beginning of the data file to the data
  UINT32     VariableDataFileNumber   = 0; ///< The variable data file number (if data is stored in individual files)
  UINT32     VariableHeaderTotalSize  = 0; ///< The total variable header size including the variable name
  UINT32     VariableIndexWriteOffset = 0; ///< The offset from the beginning of the index file to the variable header
  UINT32     VariableStoreBaseOffset  = 0; ///< The offset from the beginning of the file to the variable store

  AUTHENTICATED_VARIABLE_NVM_HEADER *AuthenticatedVariableHeader = NULL;
  VARIABLE_NVM_HEADER               *VariableHeader              = NULL;

  CHAR8                             VariableDataFileName[CSE_MAX_NVM_FILE_NAME_LENGTH];

  if (VariableInformationTrack  == NULL ||
    VariablePtrTrack            == NULL ||
    VariablePtrTrack->CurrPtr   == NULL ||
    VariablePtrTrack->EndPtr    == NULL ||
    VariableStoreInfo           == NULL
    ) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((EFI_D_INFO, "An existing variable is being updated.\n"));

  VariableHeader = VariablePtrTrack->CurrPtr;
  if (!IsValidVariableHeader (VariableHeader, VariablePtrTrack->EndPtr)) {
    DEBUG ((EFI_D_ERROR, "The existing variable header is not valid.\n"));
    return EFI_INVALID_PARAMETER;
  }
  ASSERT ((UINTN) VariableHeader > (UINTN) VariableStoreInfo->VariableStoreHeader);

  GetCseVariableStoreFileOffset (CseFileType, &VariableStoreBaseOffset);

  if (CseFileType == CseVariableFileTypePreMemoryFile) {
    VariableDataWriteOffset = VariableHeader->DataOffset;
  } else {
    VariableDataFileNumber = VariableHeader->DataOffset;
  }

  VariableIndexWriteOffset = NVM_HEADER_ALIGN (
                               (UINT32) (
                                 VariableStoreBaseOffset +
                                 ((UINTN) (VariableHeader) - (UINTN) VariableStoreInfo->VariableStoreHeader)
                                 )
                               );
  VariableHeaderTotalSize = (UINT32) (
                              GetVariableHeaderSize (VariableStoreInfo->AuthFlag) +
                              StrSize (VariableInformationTrack->VariableName) +
                              NVM_GET_PAD_SIZE (StrSize (VariableInformationTrack->VariableName))
                              );

  Status = ConstructDataFileName (CseFileType, &VariableDataFileName[0], VariableDataFileNumber);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  } else if (&VariableDataFileName[0] == NULL) {
    ASSERT (&VariableDataFileName[0] != NULL);
    return EFI_OUT_OF_RESOURCES;
  }
  DEBUG ((EFI_D_INFO, "CSE NVM Data Filename: %a\n", &VariableDataFileName[0]));

  // Note: Do not check if data already exists on NVM to prevent re-write as HECI I/O is the bottleneck.

  if (VariableStoreInfo->AuthFlag) {
    AuthenticatedVariableHeader = (AUTHENTICATED_VARIABLE_NVM_HEADER *) VariableHeader;
  }

  if (VariableStoreInfo->AuthFlag && AuthenticatedVariableHeader != NULL) {
    if (VariableAuthenticatedFieldTrack == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    SetAuthenticatedFields (AuthenticatedVariableHeader, VariableAuthenticatedFieldTrack);
  }

  //
  // Update the variable's data size to the new size if needed
  //
  if (VariableHeader->DataSize != VariableInformationTrack->DataSize) {
    // Note: Pre-memory file updates with size changes should be handled prior to
    //       this function in the flow: variable delete, reclaim, and new variable addition.
    if (CseFileType == CseVariableFileTypePreMemoryFile) {
      ASSERT (CseFileType != CseVariableFileTypePreMemoryFile);
      return EFI_NOT_READY;
    }

    DEBUG ((
      EFI_D_INFO,
      "Need to update variable data size. Updating from %d bytes to %d bytes...\n",
      VariableHeader->DataSize, VariableInformationTrack->DataSize
      ));

    if (VariableHeader->DataSize < VariableInformationTrack->DataSize) {
      VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength += (UINT32) (
                                                                           VariableInformationTrack->DataSize -
                                                                           VariableHeader->DataSize
                                                                           );
    } else {
      VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength -= (UINT32) (
                                                                           VariableHeader->DataSize -
                                                                           VariableInformationTrack->DataSize
                                                                           );
    }

    VariableStoreInfo->VariableStoreHeader->Size = sizeof (VARIABLE_NVM_STORE_HEADER) +
                                                     VariableStoreInfo->VariableStoreHeader->VariableDataTotalLength +
                                                     VariableStoreInfo->VariableStoreHeader->VariableHeaderTotalLength;

    VariableHeader->DataSize   = (UINT32) VariableInformationTrack->DataSize;
  }
  //
  // The EFI_VARIABLE_APPEND_WRITE attribute will never be set in the returned Attributes bitmask.
  //
  VariableHeader->Attributes = VariableInformationTrack->Attributes & (~EFI_VARIABLE_APPEND_WRITE);

  //
  // Update the CSE NVM store file with the updates to the local variable store.
  //
  DEBUG ((EFI_D_INFO, "Writing the variable data to the CSE NVM data file...\n"));

  //
  // Write out the variable data to the data file
  //
  Status = UpdateCseNvmFile (
             &VariableDataFileName[0],
             VariableDataWriteOffset,
             (UINT8 *) VariableInformationTrack->Data,
             VariableInformationTrack->DataSize,
             CseFileType != CseVariableFileTypePreMemoryFile,
             NULL
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((EFI_D_INFO, "Updating the index file...\n"));
  Status = UpdateIndexFile (
             VariableIndexWriteOffset,
             VariableHeaderTotalSize,
             CseFileType,
             VariableHeader
             );

  DEBUG ((EFI_D_INFO, "Variable update flow complete.\n"));
  return Status;
}

/**
  Deletes a pre-memory file variable and performs a reclaim operation.

  This is needed when a variable in the pre-memory file is updated with a new size. Due to the packing format
  in the pre-memory file, if the size changes, the old data is removed and the new data added.

  @param[in]  VariableHeader     Pointer to the variable header to be deleted and reclaimed.
  @param[in]  VariableStoreInfo  Pointer to a structure that maintains variable store information.

  @retval     EFI_SUCCESS        The delete and reclaim operation was successful.
  @retval     Others             The delete and reclaim operation failed.
**/
EFI_STATUS
EFIAPI
DeleteAndReclaimPreMemoryVariable (
  IN          VARIABLE_NVM_HEADER      *VariableHeader,
  IN  CONST   VARIABLE_NVM_STORE_INFO  *VariableStoreInfo
  )
{
  EFI_STATUS  Status;

  if (VariableHeader == NULL || VariableStoreInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  DEBUG ((EFI_D_WARN, "Performing a pre-memory file reclaim operation due to variable size change...\n"));

  Status = DeleteCsePreMemoryFileVariable (
             CseVariableFileTypePreMemoryFile,
             VariableHeader,
             VariableStoreInfo
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = ReclaimPreMemoryFileVariableStore ();
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "An error occurred reclaiming the pre-memory file. The variable could not be updated.\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  return EFI_SUCCESS;
}

/**
  Recycles a variable.

  A variable is recycled when it is added while in the deleted state.

  In the case of a variable per CSE NVM file, this allows the same variable header and data file to be used
  which reduces redundancy of data files and necessity for a reclaim operation.

  @param[in]  CseFileType                      The CSE data file type.
  @param[in]  VariableInformationTrack         Pointer to a structure that maintains common variable information.
  @param[in]  RecycledVariablePtrTrack         Pointer to a structure that maintains variable header pointers.
  @param[in]  VariableStoreInfo                Pointer to a structure that maintains variable store information.
  @param[in]  AuthenticatedVariableFieldTrack  Pointer to a structure which maintains authenticated field values.

  @retval     EFI_SUCCESS                      The recycle operation was successful.
  @retval     Others                           The recycle operation failed.
**/
EFI_STATUS
EFIAPI
RecycleVariable (
  IN         CSE_VARIABLE_FILE_TYPE                  CseFileType,
  IN  CONST  CSE_VARIABLE_INFORMATION_TRACK          *VariableInformationTrack,
  IN  CONST  VARIABLE_NVM_POINTER_TRACK              *RecycledVariablePtrTrack,
  IN         VARIABLE_NVM_STORE_INFO                 *VariableStoreInfo,
  IN  CONST  CSE_VARIABLE_AUTHENTICATED_FIELD_TRACK  *VariableAuthenticatedFieldTrack OPTIONAL
  ) {
  EFI_STATUS  Status;

  if (VariableInformationTrack        == NULL ||
    RecycledVariablePtrTrack          == NULL ||
    RecycledVariablePtrTrack->CurrPtr == NULL ||
    RecycledVariablePtrTrack->EndPtr  == NULL ||
    VariableStoreInfo                 == NULL
    ) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((EFI_D_INFO, "Variable is being recycled.\n"));

  if (CseFileType == CseVariableFileTypePreMemoryFile &&
    VariableInformationTrack->DataSize != RecycledVariablePtrTrack->CurrPtr->DataSize
    ) {
    Status = DeleteAndReclaimPreMemoryVariable (
               RecycledVariablePtrTrack->CurrPtr,
               VariableStoreInfo
               );

    if (!EFI_ERROR (Status)) {
      Status = WriteNewVariable (
                 CseFileType,
                 VariableInformationTrack,
                 VariableStoreInfo,
                 VariableAuthenticatedFieldTrack
                 );
    }
  } else {
    RecycledVariablePtrTrack->CurrPtr->State = VAR_ADDED;
//[-start-161003-IB07220139-add]//
    //
    // Attribute may change when setting new variable so update attribute here
    //
    RecycledVariablePtrTrack->CurrPtr->Attributes = VariableInformationTrack->Attributes & (~EFI_VARIABLE_APPEND_WRITE);
    if (VariableAuthenticatedFieldTrack != NULL) {
      ((AUTHENTICATED_VARIABLE_NVM_HEADER *) RecycledVariablePtrTrack->CurrPtr)->MonotonicCount = VariableAuthenticatedFieldTrack->MonotonicCount;
      ((AUTHENTICATED_VARIABLE_NVM_HEADER *) RecycledVariablePtrTrack->CurrPtr)->PubKeyIndex    = VariableAuthenticatedFieldTrack->PubKeyIndex;
      CopyMem (
        &(((AUTHENTICATED_VARIABLE_NVM_HEADER *) RecycledVariablePtrTrack->CurrPtr)->TimeStamp),
        VariableAuthenticatedFieldTrack->TimeStamp,
        sizeof (EFI_TIME)
        );
    }
//[-end-161003-IB07220139-add]//
    Status = UpdateVariable (
               CseFileType,
               VariableInformationTrack,
               RecycledVariablePtrTrack,
               VariableStoreInfo,
               VariableAuthenticatedFieldTrack
               );
  }

  return Status;
}

/**
  Sets the CSE NVM variable with the supplied data. These arguments are similar to those in the EFI Variable services.

  @param[in]      VariableName    Name of variable.
  @param[in]      VendorGuid      Guid of variable.
  @param[in]      Data            Variable data.
  @param[in]      DataSize        Size of data. 0 means delete.
  @param[in]      Attributes      Attributes of the variable. No runtime or bootservice attribute set means delete.
  @param[in,out]  IndexVariable   The variable found in the header region corresponding to this variable.
  @param[in]      KeyIndex        Index of associated public key in database
  @param[in]      MonotonicCount  Associated monotonic count value to protect against replay attack
  @param[in]      TimeStamp       Associated TimeStamp value to protect against replay attack

  @retval         EFI_SUCCESS     The set operation was successful.
  @retval         Others          The set operation failed.
**/
EFI_STATUS
EFIAPI
SetVariable (
  IN       CHAR16                      *VariableName,
  IN       EFI_GUID                    *VendorGuid,
  IN       VOID                        *Data,
  IN       UINTN                       DataSize,
  IN       UINT32                      Attributes,
  IN OUT   VARIABLE_NVM_POINTER_TRACK  *IndexVariable,
  IN       UINT32                      KeyIndex        OPTIONAL,
  IN       UINT64                      MonotonicCount  OPTIONAL,
  IN       EFI_TIME                    *TimeStamp      OPTIONAL
  )
{
  BOOLEAN                                 UpdatingExistingVariable;
  CSE_VARIABLE_FILE_TYPE                  VariableCseFileType;
  CSE_VARIABLE_AUTHENTICATED_FIELD_TRACK  VariableAuthenticatedFieldTrack;
  CSE_VARIABLE_INFORMATION_TRACK          VariableInformationTrack;
  EFI_STATUS                              Status;
  VARIABLE_NVM_STORE_INFO                 VariableStoreInfo;
  VARIABLE_NVM_POINTER_TRACK              VariableNvmDeletedVariablePtrTrack;

  VariableNvmDeletedVariablePtrTrack.StartPtr               = NULL;
  VariableNvmDeletedVariablePtrTrack.EndPtr                 = NULL;
  VariableNvmDeletedVariablePtrTrack.CurrPtr                = NULL;
  VariableNvmDeletedVariablePtrTrack.InDeletedTransitionPtr = NULL;

  UpdatingExistingVariable = (BOOLEAN) (IndexVariable->CurrPtr != NULL && IsValidVariableHeader (IndexVariable->CurrPtr, IndexVariable->EndPtr));
  VariableCseFileType      = GetCseVariableStoreFileType (VariableName, VendorGuid, mCseVariableFileInfo);

  if (VariableCseFileType >= CseVariableFileTypeMax) {
    return EFI_UNSUPPORTED;
  }

  if (mCseVariableFileInfo[VariableCseFileType] == NULL) {
    DEBUG ((EFI_D_ERROR, "The global CSE file info variable was not populated for this file type.\n"));
    return EFI_NOT_FOUND;
  }

  VariableStoreInfo.VariableStoreHeader = (VARIABLE_NVM_STORE_HEADER *) (UINTN) mCseVariableFileInfo[VariableCseFileType]->HeaderRegionBase;

  Status = IsAuthenticatedVariableStore (VariableStoreInfo.VariableStoreHeader, &VariableStoreInfo.AuthFlag);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  if (VariableStoreInfo.VariableStoreHeader == NULL) {
    DEBUG ((EFI_D_ERROR, "The variable store could not be found for this file type.\n"));
    return EFI_NOT_FOUND;
  }

  if (!mCseVariableFileInfo[VariableCseFileType]->FileEnabled || !mCseVariableFileInfo[VariableCseFileType]->FileStoreEstablished) {
    DEBUG ((EFI_D_ERROR, "Error: Attempted to set a variable with a CSE file type not enabled.\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Populate structures consumed by helper functions
  //
  if (VariableStoreInfo.AuthFlag) {
    VariableAuthenticatedFieldTrack.MonotonicCount = MonotonicCount;
    VariableAuthenticatedFieldTrack.PubKeyIndex    = KeyIndex;
    VariableAuthenticatedFieldTrack.TimeStamp      = TimeStamp;
  }

  VariableInformationTrack.VariableName = VariableName;
  VariableInformationTrack.VendorGuid   = VendorGuid;
  VariableInformationTrack.Data         = Data;
  VariableInformationTrack.DataSize     = DataSize;
  VariableInformationTrack.Attributes   = Attributes;

  if (UpdatingExistingVariable) {
    //
    // Existing Variable
    //
    if (DataSize == 0 || (Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == 0) {
      // Note: Setting a variable with no access or zero DataSize attributes causes it to be deleted.
      DEBUG ((EFI_D_INFO, "Variable has been requested for delete.\n"));

      if (VariableCseFileType == CseVariableFileTypePreMemoryFile) {
        Status = DeleteCsePreMemoryFileVariable (
                   VariableCseFileType,
                   IndexVariable->CurrPtr,
                   &VariableStoreInfo
                   );
      } else {
        Status =  DeleteCseNvmFileVariable (
                    VariableCseFileType,
                    IndexVariable->CurrPtr,
                    &VariableStoreInfo
                    );
      }
    } else {
      if (VariableCseFileType == CseVariableFileTypePreMemoryFile && IndexVariable->CurrPtr->DataSize != DataSize) {
        Status = DeleteAndReclaimPreMemoryVariable (IndexVariable->CurrPtr, &VariableStoreInfo);
        ASSERT_EFI_ERROR (Status);

        Status = WriteNewVariable (
                   VariableCseFileType,
                   &VariableInformationTrack,
                   &VariableStoreInfo,
                   (VariableStoreInfo.AuthFlag) ? &VariableAuthenticatedFieldTrack : NULL
                   );
      } else {
        Status = UpdateVariable (
                   VariableCseFileType,
                   &VariableInformationTrack,
                   IndexVariable,
                   &VariableStoreInfo,
                   (VariableStoreInfo.AuthFlag) ? &VariableAuthenticatedFieldTrack : NULL
                   );
      }
    }
  } else {
    //
    // Non-Existing Variable
    //
    VariableNvmDeletedVariablePtrTrack.StartPtr = GetStartPointer (VariableStoreInfo.VariableStoreHeader);
    VariableNvmDeletedVariablePtrTrack.EndPtr   = GetEndPointer (VariableStoreInfo.VariableStoreHeader);

    //
    // Check if variable is recycled (added back after deletion)
    //
    Status = FindDeletedVariable (VariableName, VendorGuid, VariableStoreInfo.VariableStoreHeader, &VariableNvmDeletedVariablePtrTrack);

    if (EFI_ERROR (Status)) {
      VariableNvmDeletedVariablePtrTrack.CurrPtr = NULL;
    }

    if (VariableNvmDeletedVariablePtrTrack.CurrPtr != NULL) {
      Status = RecycleVariable (
                 VariableCseFileType,
                 &VariableInformationTrack,
                 &VariableNvmDeletedVariablePtrTrack,
                 &VariableStoreInfo,
                 (VariableStoreInfo.AuthFlag) ? &VariableAuthenticatedFieldTrack : NULL
                 );
    } else {
      Status = WriteNewVariable (
                 VariableCseFileType,
                 &VariableInformationTrack,
                 &VariableStoreInfo,
                 (VariableStoreInfo.AuthFlag) ? &VariableAuthenticatedFieldTrack : NULL
                 );
    }
  }

  return Status;
}

/**
  Retrieves a protocol instance-specific GUID.

  Returns a unique GUID per VARIABLE_STORAGE_PROTOCOL instance.

  @param[out]      VariableGuid           A pointer to an EFI_GUID that is this protocol instance's GUID.

  @retval          EFI_SUCCESS            The data was returned successfully.
  @retval          EFI_INVALID_PARAMETER  A required parameter is NULL.
**/
EFI_STATUS
EFIAPI
CseVariableStorageGetId (
  OUT       EFI_GUID                        *InstanceGuid
  )
{
  if (InstanceGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (InstanceGuid, &gCseVariableStorageProtocolInstanceGuid, sizeof (EFI_GUID));

  return EFI_SUCCESS;
}

/**
  This service retrieves a variable's value using its name and GUID.

  Read the specified variable from the CSE NVM variable store. If the Data
  buffer is too small to hold the contents of the variable,
  the error EFI_BUFFER_TOO_SMALL is returned and DataSize is set to the
  required buffer size to obtain the data.

  @param[in]       This                   A pointer to this instance of the VARIABLE_STORAGE_PROTOCOL.
  @param[in]       VariableName           A pointer to a null-terminated string that is the variable's name.
  @param[in]       VariableGuid           A pointer to an EFI_GUID that is the variable's GUID. The combination of
                                          VariableGuid and VariableName must be unique.
  @param[out]      Attributes             If non-NULL, on return, points to the variable's attributes.
  @param[in, out]  DataSize               On entry, points to the size in bytes of the Data buffer.
                                          On return, points to the size of the data returned in Data.
  @param[out]      Data                   Points to the buffer which will hold the returned variable value.

  @retval          EFI_SUCCESS            The variable was read successfully.
  @retval          EFI_NOT_FOUND          The variable could not be found.
  @retval          EFI_BUFFER_TOO_SMALL   The DataSize is too small for the resulting data.
                                          DataSize is updated with the size required for
                                          the specified variable.
  @retval          EFI_INVALID_PARAMETER  VariableName, VariableGuid, DataSize or Data is NULL.
  @retval          EFI_DEVICE_ERROR       The variable could not be retrieved because of a device error.
**/
EFI_STATUS
EFIAPI
CseVariableStorageGetVariable (
  IN CONST  VARIABLE_STORAGE_PROTOCOL  *This,
  IN CONST  CHAR16                     *VariableName,
  IN CONST  EFI_GUID                   *VariableGuid,
  OUT       UINT32                     *Attributes OPTIONAL,
  IN OUT    UINTN                      *DataSize,
  OUT       VOID                       *Data
  )
{
  //
  // Check input parameters
  //
  if (VariableName == NULL || VariableGuid == NULL || DataSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((EFI_D_INFO, "Attempting to read CSE NVM variable.\n  Name=\'%ls\'\n  GUID=%g\n\n", \
    VariableName, VariableGuid));

  DEBUG ((EFI_D_INFO, "Preparing to call GetCseVariable()\n"));

  return GetCseVariable (mCseVariableFileInfo, VariableName, VariableGuid, Attributes, DataSize, Data, NULL);
}

/**
  This service retrieves an authenticated variable's value using its name and GUID.

  Read the specified authenticated variable from the UEFI variable store. If the Data
  buffer is too small to hold the contents of the variable,
  the error EFI_BUFFER_TOO_SMALL is returned and DataSize is set to the
  required buffer size to obtain the data.

  @param[in]       This                   A pointer to this instance of the VARIABLE_STORAGE_PROTOCOL.
  @param[in]       VariableName           A pointer to a null-terminated string that is the variable's name.
  @param[in]       VariableGuid           A pointer to an EFI_GUID that is the variable's GUID. The combination of
                                          VariableGuid and VariableName must be unique.
  @param[out]      Attributes             If non-NULL, on return, points to the variable's attributes.
  @param[in, out]  DataSize               On entry, points to the size in bytes of the Data buffer.
                                          On return, points to the size of the data returned in Data.
  @param[out]      Data                   Points to the buffer which will hold the returned variable value.
  @param[out]      KeyIndex               Index of associated public key in database
  @param[out]      MonotonicCount         Associated monotonic count value to protect against replay attack
  @param[out]      TimeStamp              Associated TimeStamp value to protect against replay attack

  @retval          EFI_SUCCESS            The variable was read successfully.
  @retval          EFI_NOT_FOUND          The variable could not be found.
  @retval          EFI_BUFFER_TOO_SMALL   The DataSize is too small for the resulting data.
                                          DataSize is updated with the size required for
                                          the specified variable.
  @retval          EFI_INVALID_PARAMETER  VariableName, VariableGuid, DataSize or Data is NULL.
  @retval          EFI_DEVICE_ERROR       The variable could not be retrieved because of a device error.
**/
EFI_STATUS
EFIAPI
CseVariableStorageGetAuthenticatedVariable (
  IN CONST  VARIABLE_STORAGE_PROTOCOL    *This,
  IN CONST  CHAR16                       *VariableName,
  IN CONST  EFI_GUID                     *VariableGuid,
  OUT       UINT32                       *Attributes,
  IN OUT    UINTN                        *DataSize,
  OUT       VOID                         *Data,
  OUT       UINT32                       *KeyIndex,
  OUT       UINT64                       *MonotonicCount,
  OUT       EFI_TIME                     *TimeStamp
  )
{
  EFI_STATUS                        Status;
  AUTHENTICATED_VARIABLE_NVM_HEADER *AuthenticatedVariableHeader = NULL;

  //
  // Check input parameters
  //
  if (VariableName == NULL || VariableGuid == NULL || DataSize == NULL ||
      KeyIndex == NULL || MonotonicCount == NULL || TimeStamp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((EFI_D_INFO, "Attempting to read CSE NVM authenticated variable.\n  Name=\'%ls\'\n  GUID=%g\n\n", \
    VariableName, VariableGuid));

  DEBUG ((EFI_D_INFO, "Preparing to call GetCseVariable()\n"));

  Status = GetCseVariable (mCseVariableFileInfo, VariableName, VariableGuid, Attributes, DataSize, Data, (VARIABLE_NVM_HEADER **) &AuthenticatedVariableHeader);

  if (!EFI_ERROR (Status) && AuthenticatedVariableHeader != NULL) {
    *KeyIndex       = AuthenticatedVariableHeader->PubKeyIndex;
    *MonotonicCount = AuthenticatedVariableHeader->MonotonicCount;

    CopyMem (TimeStamp, &AuthenticatedVariableHeader->TimeStamp, sizeof (EFI_TIME));
  }

  return Status;
}

/**
  Return the next variable name and GUID.

  This function is called multiple times to retrieve the VariableName
  and VariableGuid of all variables currently available in the system.
  On each call, the previous results are passed into the interface,
  and, on return, the interface returns the data for the next
  interface. When the entire variable list has been returned,
  EFI_NOT_FOUND is returned.

  @param[in]      This                   A pointer to this instance of the VARIABLE_STORAGE_PROTOCOL.

  @param[in, out] VariableNameSize       On entry, points to the size of the buffer pointed to by
                                         VariableName. On return, the size of the variable name buffer.
  @param[in, out] VariableName           On entry, a pointer to a null-terminated string that is the
                                         variable's name. On return, points to the next variable's
                                         null-terminated name string.
  @param[in, out] VariableGuid           On entry, a pointer to an EFI_GUID that is the variable's GUID.
                                         On return, a pointer to the next variable's GUID.
  @param[out]     VariableAttributes     A pointer to the variable attributes.

  @retval         EFI_SUCCESS            The variable was read successfully.
  @retval         EFI_NOT_FOUND          The variable could not be found.
  @retval         EFI_BUFFER_TOO_SMALL   The VariableNameSize is too small for the resulting
                                         data. VariableNameSize is updated with the size
                                         required for the specified variable.
  @retval         EFI_INVALID_PARAMETER  VariableName, VariableGuid or
                                         VariableNameSize is NULL.
  @retval         EFI_DEVICE_ERROR       The variable could not be retrieved because of a device error.
**/
EFI_STATUS
EFIAPI
CseVariableStorageGetNextVariableName (
  IN CONST  VARIABLE_STORAGE_PROTOCOL       *This,
  IN OUT    UINTN                           *VariableNameSize,
  IN OUT    CHAR16                          *VariableName,
  IN OUT    EFI_GUID                        *VariableGuid,
  OUT       UINT32                          *VariableAttributes
  )
{
  EFI_STATUS           Status;
  BOOLEAN              IsAuthVariable;
  UINTN                VarNameSize;
  VARIABLE_NVM_HEADER  *VariablePtr;

  if (VariableNameSize == NULL || VariableName == NULL || VariableGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetNextCseVariableName (VariableName, VariableGuid, mCseVariableFileInfo, &VariablePtr, &IsAuthVariable);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  VarNameSize = NameSizeOfVariable (VariablePtr, IsAuthVariable);
  ASSERT (VarNameSize != 0);

  if (VarNameSize <= *VariableNameSize) {
    CopyMem (VariableName, GetVariableNamePtr (VariablePtr, IsAuthVariable), VarNameSize);
    CopyMem (VariableGuid, GetVendorGuidPtr (VariablePtr, IsAuthVariable), sizeof (EFI_GUID));
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_BUFFER_TOO_SMALL;
  }
  *VariableNameSize = VarNameSize;
  *VariableAttributes = VariablePtr->Attributes;

  return Status;
}

/**
  Returns information on the amount of space available in the variable store. If the amount of data that can be written
  depends on if the platform is in Pre-OS stage or OS stage, the AtRuntime parameter should be used to compute usage.

  @param[in]  This                        A pointer to this instance of the VARIABLE_STORAGE_PROTOCOL.
  @param[in]  AtRuntime                   TRUE is the platform is in OS Runtime, FALSE if still in Pre-OS stage
  @param[out] VariableStoreSize           The total size of the NV storage. Indicates the maximum amount
                                          of data that can be stored in this NV storage area.
  @param[out] CommonVariablesTotalSize    The total combined size of all the common UEFI variables that are
                                          stored in this NV storage area. Excludes variables with the
                                          EFI_VARIABLE_HARDWARE_ERROR_RECORD attribute set.
  @param[out] HwErrVariablesTotalSize     The total combined size of all the UEFI variables that have the
                                          EFI_VARIABLE_HARDWARE_ERROR_RECORD attribute set and which are
                                          stored in this NV storage area. Excludes all other variables.

  @retval     EFI_INVALID_PARAMETER       Any of the given parameters are NULL
  @retval     EFI_SUCCESS                 Space information returned successfully.
**/
EFI_STATUS
EFIAPI
CseVariableStorageGetStorageUsage (
  IN CONST    VARIABLE_STORAGE_PROTOCOL   *This,
  IN          BOOLEAN                     AtRuntime,
  OUT         UINT32                      *VariableStoreSize,
  OUT         UINT32                      *CommonVariablesTotalSize,
  OUT         UINT32                      *HwErrVariablesTotalSize
  )
{
  BOOLEAN                    IsAuthVariable;
  CSE_VARIABLE_FILE_TYPE     Type;
  EFI_GUID                   Guid;
  EFI_STATUS                 Status;
  UINT32                     Attributes;
  VARIABLE_NVM_POINTER_TRACK VariablePtrTrack;

  CHAR16  *VariableName         = NULL;

  UINTN   VariableNameSize      = 0;
  UINTN   NewVariableNameSize   = 0;

  UINTN  CseVariableTotalStoreSizeInternal   = 0;
  UINTN  CseCommonVariablesTotalSizeInternal = 0;
  UINTN  CseHwErrVariablesTotalSizeInternal  = 0;

  for (Type = (CSE_VARIABLE_FILE_TYPE) 0; Type < CseVariableFileTypeMax; Type++) {
    if (mCseVariableFileInfo[Type] != NULL && mCseVariableFileInfo[Type]->FileStoreEstablished) {
      CseVariableTotalStoreSizeInternal += (mCseVariableFileInfo[Type]->FileStoreMaximumSize - sizeof (VARIABLE_NVM_STORE_HEADER));
      DEBUG ((EFI_D_INFO, "Current store maximum size = %d bytes.\n", mCseVariableFileInfo[Type]->FileStoreMaximumSize));
    }

  }
  ASSERT (CseVariableTotalStoreSizeInternal > 0);

  VariableNameSize = sizeof (CHAR16);
  VariableName     = AllocateZeroPool (VariableNameSize);
  if (VariableName == NULL) {
    ASSERT (VariableName != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  while (TRUE) {
    NewVariableNameSize = VariableNameSize;
    Status = CseVariableStorageGetNextVariableName (This, &NewVariableNameSize, VariableName, &Guid, &Attributes);

    if (Status == EFI_BUFFER_TOO_SMALL) {
      VariableName = ReallocatePool (VariableNameSize, NewVariableNameSize, VariableName);
      if (VariableName == NULL) {
        ASSERT (VariableName != NULL);
        return EFI_OUT_OF_RESOURCES;
      }
      Status = CseVariableStorageGetNextVariableName (This, &NewVariableNameSize, VariableName, &Guid, &Attributes);
      VariableNameSize = NewVariableNameSize;
    }

    if (Status == EFI_NOT_FOUND) {
      break;
    }
    ASSERT_EFI_ERROR (Status);

    //
    // Get the variable header for this variable
    //
    Status = FindVariable (VariableName, &Guid, mCseVariableFileInfo, &VariablePtrTrack);

    if (EFI_ERROR (Status) || VariablePtrTrack.CurrPtr == NULL) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }

    Type = GetCseVariableStoreFileType (VariableName, &Guid, mCseVariableFileInfo);

    Status = IsAuthenticatedVariableStore ((VARIABLE_NVM_STORE_HEADER *) (UINTN) mCseVariableFileInfo[Type]->HeaderRegionBase, &IsAuthVariable);

    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }

    //
    // When the reclaim operation is implemented, need to do a runtime check.
    // At runtime, don't worry about the variable state. At boot time, the
    // state needs to be taken into account because the variables can be reclaimed.
    //
    if ((VariablePtrTrack.CurrPtr->Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
      CseHwErrVariablesTotalSizeInternal += GetVariableHeaderSize (IsAuthVariable);
      CseHwErrVariablesTotalSizeInternal += VariablePtrTrack.CurrPtr->NameSize;
      CseHwErrVariablesTotalSizeInternal += VariablePtrTrack.CurrPtr->DataSize;
    } else {
      CseCommonVariablesTotalSizeInternal += GetVariableHeaderSize (IsAuthVariable);
      CseCommonVariablesTotalSizeInternal += VariablePtrTrack.CurrPtr->NameSize;
      CseCommonVariablesTotalSizeInternal += VariablePtrTrack.CurrPtr->DataSize;
    }
  }

  FreePool (VariableName);

  *VariableStoreSize        = (UINT32) CseVariableTotalStoreSizeInternal;
  *CommonVariablesTotalSize = (UINT32) CseCommonVariablesTotalSizeInternal;
  *HwErrVariablesTotalSize  = (UINT32) CseHwErrVariablesTotalSizeInternal;

  return EFI_SUCCESS;
}

/**
  Returns whether this NV storage area supports storing authenticated variables or not

  @param[in]  This             A pointer to this instance of the VARIABLE_STORAGE_PROTOCOL.
  @param[out] AuthSupported    TRUE if this NV storage area can store authenticated variables,
                               FALSE otherwise

  @retval     EFI_SUCCESS      AuthSupported was returned successfully.
**/
EFI_STATUS
EFIAPI
CseVariableStorageGetAuthenticatedSupport (
  IN CONST    VARIABLE_STORAGE_PROTOCOL   *This,
  OUT         BOOLEAN                     *AuthSupported
  )
{
  *AuthSupported = TRUE;

  return EFI_SUCCESS;
}

/**
  This code sets a variable's value using its name and GUID.

  Caution: This function may receive untrusted input.
  This function may be invoked in SMM mode, and datasize and data are external input.
  This function will do basic validation, before parsing the data.
  This function will parse the authentication carefully to avoid security issues, like
  buffer overflow, integer overflow.
  This function will check attribute carefully to avoid authentication bypass.

  @param[in]  This                     A pointer to this instance of the VARIABLE_STORAGE_PROTOCOL.
  @param[in]  VariableName             Name of Variable to be found.
  @param[in]  VendorGuid               Variable vendor GUID.
  @param[in]  Attributes               Attribute value of the variable found
  @param[in]  DataSize                 Size of Data found. If size is less than the
                                       data, this value contains the required size.
  @param[in]  Data                     Data pointer.
  @param[in]  AtRuntime                TRUE is the platform is in OS Runtime, FALSE if still in Pre-OS stage
  @param[in]  KeyIndex                 If writing an authenticated variable, the public key index
  @param[in]  MonotonicCount           If writing a monotonic counter authenticated variable, the counter value
  @param[in]  TimeStamp                If writing a timestamp authenticated variable, the timestamp value

  @retval     EFI_INVALID_PARAMETER    Invalid parameter.
  @retval     EFI_SUCCESS              Set successfully.
  @retval     EFI_OUT_OF_RESOURCES     Resource not enough to set variable.
  @retval     EFI_NOT_FOUND            Not found.
  @retval     EFI_WRITE_PROTECTED      Variable is read-only.
**/
EFI_STATUS
EFIAPI
CseVariableStorageSetVariable (
  IN CONST    VARIABLE_STORAGE_PROTOCOL   *This,
  IN          CHAR16                      *VariableName,
  IN          EFI_GUID                    *VendorGuid,
  IN          UINT32                      Attributes,
  IN          UINTN                       DataSize,
  IN          VOID                        *Data,
  IN          BOOLEAN                     AtRuntime,
  IN          UINT32                      KeyIndex       OPTIONAL,
  IN          UINT64                      MonotonicCount OPTIONAL,
  IN          EFI_TIME                    *TimeStamp     OPTIONAL
  )
{
  EFI_STATUS                  Status;
  VARIABLE_NVM_POINTER_TRACK  VariablePtrTrack;

  //
  // Check input parameters.
  //
  if (VariableName == NULL || VariableName[0] == 0 || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (DataSize != 0 && Data == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (((Attributes & EFI_VARIABLE_NON_VOLATILE) != EFI_VARIABLE_NON_VOLATILE) && (DataSize != 0)) {
    DEBUG ((EFI_D_ERROR, "A volatile variable was passed to the CSE storage protocol.\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  //
  if (AtRuntime && mIsEmmcBoot) {
    DEBUG ((EFI_D_ERROR, "Performing workaround for eMMC boot issue\n"));
    return EFI_SUCCESS;
  }

  DEBUG ((EFI_D_INFO, "Attempting to set CSE NVM variable.\n  Name=\'%ls\'\n  GUID=%g\n  Attributes=0x%x\n\n", \
                      VariableName, *VendorGuid, Attributes));

  if (PcdGet32 (PcdMaxVariableSize) - sizeof (VARIABLE_NVM_HEADER) <= StrSize (VariableName) + DataSize) {
    DEBUG ((EFI_D_ERROR, "The variable being written is larger than the maximum specified variable size.\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check if the variable already exists
  //
  Status = FindVariable (VariableName, VendorGuid, mCseVariableFileInfo, &VariablePtrTrack);
  if (!EFI_ERROR (Status)) {
    if (Attributes != 0 && (Attributes & (~EFI_VARIABLE_APPEND_WRITE)) != VariablePtrTrack.CurrPtr->Attributes) {
      DEBUG ((EFI_D_ERROR, "Error: Attributes given do not match the existing variable.\n"));
      DEBUG ((EFI_D_ERROR, "       Attributes given = 0x%x. Existing attributes = 0x%x.\n", Attributes, VariablePtrTrack.CurrPtr->Attributes));
      return EFI_INVALID_PARAMETER;
    }
  } else if (Status != EFI_NOT_FOUND) {
    DEBUG ((EFI_D_ERROR, "Error occurred finding the variable.\n"));
    return Status;
  }

  DEBUG ((EFI_D_INFO, "VariablePtrTrack->CurrPtr is null = %c\n", (VariablePtrTrack.CurrPtr == NULL ? 'T' : 'F')));

  Status = SetVariable (VariableName, VendorGuid, Data, DataSize, Attributes, &VariablePtrTrack, KeyIndex, MonotonicCount, TimeStamp);
  if (!EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "The variable was updated successfully.\n"));
  }

  return Status;
}

/**
  Returns whether this NV storage area is ready to accept calls to SetVariable() or not

  @param[in]  This     A pointer to this instance of the VARIABLE_STORAGE_PROTOCOL.

  @retval     TRUE     The NV storage area is ready to accept calls to SetVariable()
  @retval     FALSE    The NV storage area is not ready to accept calls to SetVariable()
**/
BOOLEAN
EFIAPI
CseVariableStorageWriteServiceIsReady (
  IN CONST    VARIABLE_STORAGE_PROTOCOL   *This
  )
{
  //
  // CSE should be ready to service write commands when this driver is loaded
  //
  return TRUE;
}

/**
  Sets the callback to be invoked when the VARIABLE_STORAGE_PROTOCOL is ready to accept calls to SetVariable()

  The VARIABLE_STORAGE_PROTOCOL is required to invoke the callback as quickly as possible after the core
  variable driver invokes RegisterWriteServiceReadyCallback() to set the callback.

  @param[in]  This                A pointer to this instance of the VARIABLE_STORAGE_PROTOCOL.
  @param[in]  CallbackFunction    The callback function

  @retval     EFI_SUCCESS         The callback function was sucessfully registered
**/
EFI_STATUS
EFIAPI
CseVariableStorageRegisterWriteServiceReadyCallback (
  IN CONST    VARIABLE_STORAGE_PROTOCOL             *This,
  IN VARIABLE_STORAGE_WRITE_SERVICE_READY_CALLBACK  CallbackFunction
  )
{
  //
  // CSE should be ready to service write commands when this driver is loaded
  // immediately invoke the callback function
  //
  return CallbackFunction ();
}

/**
  Performs variable store garbage collection/reclaim operation.

  @param[in]  This                             A pointer to this instance of the VARIABLE_STORAGE_PROTOCOL.

  @retval     EFI_INVALID_PARAMETER            Invalid parameter.
  @retval     EFI_SUCCESS                      Garbage Collection Successful.
  @retval     EFI_OUT_OF_RESOURCES             Insufficient resource to complete garbage collection.
  @retval     EFI_WRITE_PROTECTED              Write services are not yet ready.

**/
EFI_STATUS
EFIAPI
CseVariableStorageGarbageCollect (
  IN CONST    VARIABLE_STORAGE_PROTOCOL   *This
  )
{
  //
  // @todo: Need to Implement
  //
  return EFI_SUCCESS;
}


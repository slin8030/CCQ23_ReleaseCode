/** @file
  Provide support functions for variable services.

;******************************************************************************
;* Copyright (c) 2012 - 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include "Variable.h"
#include "AuthService.h"
#include "InsydeSecureVariable.h"
#include "VariableLock.h"
#include "VarCheck.h"
#include "GlobalVariable.h"
#include "SensitiveVariable.h"
#include <PostCode.h>

#include <Protocol/NonVolatileVariable.h>
//#include <Protocol/VariableDefaultUpdate.h>
#include <Protocol/Bds.h>

#include <Library/FlashRegionLib.h>
#include <Guid/BiosFirstBootHobGuid.h>

#define VARIABLE_WORKING_REGION_HOB_GUID {0x92888eba, 0xe125, 0x4c41, 0xbc, 0x9d, 0x68, 0x3e, 0x8f, 0x7e, 0x61, 0x18}
STATIC EFI_GUID  mVariableWorkingRegionHobGuid = VARIABLE_WORKING_REGION_HOB_GUID;


typedef struct {
  EFI_PHYSICAL_ADDRESS              VariableWorkingRegionStart;
  EFI_PHYSICAL_ADDRESS              VariableWorkingRegionLength;
} VARIALBE_WORKING_REGION_INFO;


typedef struct {
  CHAR16            *VariableName;
  EFI_GUID          *VendorGuid;
} VARIALBE_NAME_GUID;

BOOLEAN                               mReadyToBootEventSignaled = FALSE;
SMM_VAR_BUFFER                        *mSmmVarBuf;
SMM_VAR_BUFFER                        *mSmmPhyVarBuf;
EFI_BDS_ENTRY                         mOriginalBdsEntry;
UINTN                                 mSmramRangeCount;
EFI_SMRAM_DESCRIPTOR                  *mSmramRanges;
VAR_ERROR_FLAG                        mCurrentBootVarErrFlag = VAR_ERROR_FLAG_NO_ERROR;
CHAR16                                mVariableNameBuffer[MAX_VARIABLE_NAME_SIZE];
VOID                                  *mVariableDataBuffer;

SMI_SUB_FUNCTION_MAP mNonSecureBootFunctionsTable [] = {
  { SMM_GET_VARIABLE_SMI_FUN_NUM,             SmmInternalGetVariable        , TRUE}, \
  { SMM_GET_NEXT_VARIABLE_SMI_FUN_NUM,        SmmInternalGetNextVariableName, TRUE}, \
  { SET_SENSITIVE_VARIABLE_FUN_NUM,           SmmSetSensitiveVariable       , TRUE}, \
  { SMM_VARIABLE_CHECK_FUN_NUM,               SmmCreateVariableCheckList    , TRUE}, \
  { SMM_VARIABLE_LOCK_FUN_NUM,                SmmCreateVariableLockList     , TRUE}, \
  { LEGACY_BOOT_SMI_FUN_NUM,                  SmmLegacyBootEvent            , TRUE}, \
  { SMM_SET_VARIABLE_SMI_FUN_NUM,             SmmInternalSetVariable        , TRUE}, \
  { DISABLE_SECURE_BOOT_SMI_FUN_NUM,          SmmDisableSecureBootSmi       , TRUE}, \
  { UPDATE_VARIABLE_PROPERTY_FUN_NUM,         SmmUpdateVariablePropertySmi  , TRUE}, \
//[-start-170418-IB15590029-add]//
  { SMM_QUERY_VARIABLE_INFO_FUN_NUM,          SmmInternalQueryVariableInfo  , TRUE}, \
//[-end-170418-IB15590029-add]//
  { 0,                                        NULL                          , TRUE}
  };

//[-start-161215-IB10860217-add]//
BOOLEAN
IsRestoreFactoryDefault (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid,
  IN UINT32                  Attributes
  ) 
{
  if (VariableName != NULL && VendorGuid != NULL) {
    if (StrCmp (VariableName, EFI_RESTORE_FACOTRY_DEFAULT_NAME) == 0 && CompareGuid (VendorGuid, &gEfiGenericVariableGuid) && (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) == Attributes ) {
      return TRUE;
    }
  }
  return FALSE;

}
//[-end-161215-IB10860217-add]//
/**
  This code gets the size of non-volatile variable store.

  @retunr UINTN           The size of non-volatile variable store.

**/
UINTN
GetNonVolatileVariableStoreSize (
  VOID
  )
{
  return mVariableModuleGlobal->NonVolatileVariableCacheSize;
}

/**
 This code gets the pointer to the last variable memory pointer byte

 @param [in]   VarStoreHeader   Pointer to the Variable Store Header.

 @retval VARIABLE_HEADER*       Pointer to last unavailable Variable Header.

**/
VARIABLE_HEADER *
EFIAPI
GetNonVolatileEndPointer (
  IN VARIABLE_STORE_HEADER       *VarStoreHeader
  )
{
  //
  // The end of variable store
  //
  return (VARIABLE_HEADER *) ((UINTN) VarStoreHeader + GetNonVolatileVariableStoreSize ());
}

/**
  This code checks if variable header is valid or not and is whether whole data in variable region.

  @param[in] Variable          Pointer to the Variable Header.
  @param[in] VariableStoreEnd  Pointer to the Variable store end.

  @retval TRUE                 Variable header is valid and in variable region.
  @retval FALSE                Variable header is not valid or isn't in variable region.
**/
STATIC
BOOLEAN
IsValidVariableHeaderInVarRegion (
  IN CONST VARIABLE_HEADER   *Variable,
  IN CONST VARIABLE_HEADER   *VariableStoreEnd
  )
{
  if (Variable == NULL || VariableStoreEnd == NULL) {
    return FALSE;
  }
  if (IsValidVariableHeader (Variable) && (UINTN) GetNextVariablePtr (Variable) <= (UINTN) VariableStoreEnd) {
    return TRUE;
  }
  return FALSE;
}


/**
  Check if exist newer variable when doing reclaim.

  @param  Variable    Pointer to start position

  @retval TRUE        Exists another variable, which is newer than the current one.
  @retval FALSE       Doesn't exist another vairable which is newer than the current one.

**/
BOOLEAN
ExistNewerVariable (
  IN  VARIABLE_HEADER         *Variable
  )
{
  VARIABLE_HEADER       *NextVariable;
  CHAR16                *VariableName;
  EFI_GUID              *VendorGuid;

  VendorGuid   = &Variable->VendorGuid;
  VariableName = GET_VARIABLE_NAME_PTR(Variable);

  NextVariable = GetNextVariablePtr (Variable);
  while (IsValidVariableHeader (NextVariable)) {
    if ((NextVariable->State == VAR_ADDED) || (NextVariable->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION))) {
      //
      // If match Guid and Name
      //
      if (CompareGuid (VendorGuid, &NextVariable->VendorGuid)) {
         if (CompareMem (VariableName, GET_VARIABLE_NAME_PTR (NextVariable), StrSize (VariableName)) == 0) {
           return TRUE;
         }
       }
    }
    NextVariable = GetNextVariablePtr (NextVariable);
  }
  return FALSE;
}


/**
  This function writes data to the FWH at the correct LBA even if the LBAs
  are fragmented.

  @param Global                  Pointer to VARAIBLE_GLOBAL structure
  @param Volatile                If the Variable is Volatile or Non-Volatile
  @param SetByIndex              TRUE: Target pointer is given as index
                                 FALSE: Target pointer is absolute
  @param DataPtrIndex            Pointer to the Data from the end of VARIABLE_STORE_HEADER
                                 structure
  @param DataSize                Size of data to be written.
  @param Buffer                  Pointer to the buffer from which data is written

  @retval EFI_INVALID_PARAMETER  Parameters not valid
  @retval EFI_SUCCESS            Variable store successfully updated

--*/
EFI_STATUS
UpdateVariableStore (
  IN  VARIABLE_GLOBAL         *Global,
  IN  BOOLEAN                 Volatile,
  IN  BOOLEAN                 SetByIndex,
  IN  UINTN                   DataPtrIndex,
  IN  UINT32                  DataSize,
  IN  UINT8                   *Buffer
  )
{
  VARIABLE_STORE_HEADER       *VolatileBase;
  VARIABLE_STORE_HEADER       *NonVolatileBase;
  EFI_PHYSICAL_ADDRESS        DataPtr;

  DataPtr     = DataPtrIndex;
  //
  // Check if the Data is Volatile.
  //
  if (!Volatile) {
    //
    // Data Pointer should point to the actual Address where data is to be
    // written.
    //
    NonVolatileBase = mVariableModuleGlobal->NonVolatileVariableCache;
    if (SetByIndex) {
      DataPtr += (UINTN) NonVolatileBase;
    }

    if ((DataPtr + DataSize) >= ((UINTN) ((UINT8 *) NonVolatileBase + GetNonVolatileVariableStoreSize ()))) {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    // Data Pointer should point to the actual Address where data is to be
    // written.
    //
    VolatileBase = (VARIABLE_STORE_HEADER *) ((UINTN) Global->VolatileVariableBase);
    if (SetByIndex) {
      DataPtr += Global->VolatileVariableBase;
    }

    if ((DataPtr + DataSize) >= ((UINTN) ((UINT8 *) VolatileBase + GetVariableStoreSize (VolatileBase)))) {
      return EFI_INVALID_PARAMETER;
    }
  }
  CopyMem ((UINT8 *)(UINTN)DataPtr, Buffer, DataSize);
  return EFI_SUCCESS;
}

/**
  Find the variable in the specified variable store.

  @param[in]       VariableName        Name of the variable to be found
  @param[in]       VendorGuid          Vendor GUID to be found.
  @param[in, out]  PtrTrack            Variable Track Pointer structure that contains Variable Information.
  @param[out]      VariableCount       The number of found variabl.

  @retval          EFI_SUCCESS         Variable found successfully
  @retval          EFI_NOT_FOUND       Variable not found
**/
EFI_STATUS
FindVariableEx (
  IN CONST  CHAR16                  *VariableName,
  IN CONST  EFI_GUID                *VendorGuid,
  IN OUT    VARIABLE_POINTER_TRACK  *PtrTrack,
  OUT       UINTN                   *VariableCount
  )
{
  VARIABLE_HEADER                *InDeletedVariable;
  VOID                           *Point;

  PtrTrack->InDeletedTransitionPtr = NULL;
  *VariableCount                   = 0;
  //
  // Find the variable by walk through HOB, volatile and non-volatile variable store.
  //
  InDeletedVariable  = NULL;

  for ( PtrTrack->CurrPtr = PtrTrack->StartPtr
      ; IsValidVariableHeaderInVarRegion (PtrTrack->CurrPtr, PtrTrack->EndPtr)
      ; PtrTrack->CurrPtr = GetNextVariablePtr (PtrTrack->CurrPtr)
      ) {
    if (PtrTrack->CurrPtr->State == VAR_ADDED ||
        PtrTrack->CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)
       ) {
      if (VariableName[0] == 0) {
        if (PtrTrack->CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
          InDeletedVariable   = PtrTrack->CurrPtr;
        } else {
          PtrTrack->InDeletedTransitionPtr = InDeletedVariable;
          return EFI_SUCCESS;
        }
      } else {
        if (CompareGuid (VendorGuid, &PtrTrack->CurrPtr->VendorGuid)) {
          Point = (VOID *) GET_VARIABLE_NAME_PTR (PtrTrack->CurrPtr);

          ASSERT (NameSizeOfVariable (PtrTrack->CurrPtr) != 0);
          if (CompareMem (VariableName, Point, NameSizeOfVariable (PtrTrack->CurrPtr)) == 0) {
            (*VariableCount)++;
            if (PtrTrack->CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
              InDeletedVariable     = PtrTrack->CurrPtr;
            } else {
              PtrTrack->InDeletedTransitionPtr = InDeletedVariable;
              return EFI_SUCCESS;
            }
          }
        }
      }
     }
  }

  PtrTrack->CurrPtr = InDeletedVariable;
  return (PtrTrack->CurrPtr  == NULL) ? EFI_NOT_FOUND : EFI_SUCCESS;
}

/**
  Finds the first variable from the first NV storage

  @param[out]  PtrTrack               VARIABLE_POINTER_TRACK structure for output,
                                      including the range searched and the target position.
  @param[in]   Global                 Pointer to VARIABLE_GLOBAL structure, including
                                      base of volatile variable storage area, base of
                                      NV variable storage area, and a lock.

  @retval EFI_INVALID_PARAMETER       If VariableName is not an empty string, while
                                      VendorGuid is NULL.
  @retval EFI_SUCCESS                 Variable successfully found.
  @retval EFI_NOT_FOUND               Variable not found

**/
EFI_STATUS
FindFirstNvVariable (
  OUT VARIABLE_POINTER_TRACK  *PtrTrack,
  IN  VARIABLE_GLOBAL         *Global
  )
{
  VARIABLE_STORAGE_PROTOCOL   *VariableStorage;
  UINTN                       VariableNameBufferSize;
  UINT32                      VariableAttributes;
  EFI_GUID                    TempGuid;
  EFI_STATUS                  Status;
  UINTN                       VariableCount;

  if (mVariableModuleGlobal->VariableBase.VariableStoresCount <= 0) {
    return EFI_NOT_FOUND;
  }

  VariableNameBufferSize = sizeof (mVariableNameBuffer);
  ZeroMem ((VOID *) &mVariableNameBuffer[0], VariableNameBufferSize);
  ZeroMem ((VOID *) &TempGuid, sizeof (EFI_GUID));
  VariableStorage = mVariableModuleGlobal->VariableBase.VariableStores[0];
  Status = VariableStorage->GetNextVariableName (
                              VariableStorage,
                              &VariableNameBufferSize,
                              &mVariableNameBuffer[0],
                              &TempGuid,
                              &VariableAttributes
                              );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "VariableStorageProtocol->GetNextVariableName status %r\n", Status));
    ASSERT (Status != EFI_BUFFER_TOO_SMALL);
    return Status;
  }
  return FindVariable (&mVariableNameBuffer[0], &TempGuid, PtrTrack, &VariableCount, Global);
}

/**
  This code finds variable in storage blocks (Volatile or Non-Volatile)

  @param VariableName                Name of the variable to be found
  @param VendorGuid                  Vendor GUID to be found.
  @param PtrTrack                    Variable Track Pointer structure that contains Variable Information.
  @param VariableCount               The number of found variabl.
  @param Global                      VARIABLE_GLOBAL pointer.

  @retval EFI_INVALID_PARAMETER      If VariableName is not an empty string, while
                                     VendorGuid is NULL.
  @retval EFI_SUCCESS                Variable successfully found.
  @retval EFI_NOT_FOUND              Variable not found

**/
EFI_STATUS
FindVariable (
  IN        CHAR16                  *VariableName,
  IN        EFI_GUID                *VendorGuid,
  OUT       VARIABLE_POINTER_TRACK  *PtrTrack,
  OUT       UINTN                   *VariableCount,
  IN        VARIABLE_GLOBAL         *Global
  )
{
  VARIABLE_HEADER                    *Variable[2];
  VARIABLE_STORE_HEADER              *VariableStoreHeader[2];
  UINTN                              Index;
  UINTN                              IndexMax;
  EFI_STATUS                         Status;
  EFI_STATUS                         Status2;
  EFI_GUID                           VariableStorageId;
  EFI_GUID                           InstanceGuid;
  VARIABLE_STORAGE_PROTOCOL          *VariableStorageProtocol;
  VARIABLE_STORAGE_PROTOCOL          *CorrectVariableStorageProtocol;
  VARIABLE_STORAGE_SELECTOR_PROTOCOL *VariableStorageSelectorProtocol;
  UINTN                              DataSize;
  UINT32                             Attributes;
  UINT32                             KeyIndex;
  UINT64                             MonotonicCount;
  EFI_TIME                           TimeStamp;

  *VariableCount = 0;
  //
  // 0: Non-Volatile, 1: Volatile
  //
  IndexMax = 2;

  VariableStoreHeader[0]  = mVariableModuleGlobal->NonVolatileVariableCache;
  VariableStoreHeader[1]  = (VARIABLE_STORE_HEADER *) ((UINTN) Global->VolatileVariableBase);
  //
  // Start Pointers for the variable.
  // Actual Data Pointer where data can be written.
  //
  Variable[0] = GetStartPointer (VariableStoreHeader[0]);
  Variable[1] = GetStartPointer (VariableStoreHeader[1]);


  if (VariableName[0] != 0 && VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Find the variable by walk through non-volatile and volatile variable store
  //
  for (Index = 0; Index < IndexMax; Index++) {
    PtrTrack->StartPtr  = GetStartPointer (VariableStoreHeader[Index]);
    PtrTrack->Volatile  = (BOOLEAN) Index;
    if (Index == 0) {
      PtrTrack->EndPtr = GetNonVolatileEndPointer (VariableStoreHeader[Index]);
    } else {
      PtrTrack->EndPtr = GetEndPointer (VariableStoreHeader[Index]);
    }

    Status = FindVariableEx (VariableName, VendorGuid, PtrTrack, VariableCount);
    if (!EFI_ERROR (Status)) {
      return Status;
    }
  }
  //
  // If VariableName is an empty string get the first variable from the first NV storage
  //
  if (VariableName[0] == 0) {
    return FindFirstNvVariable (PtrTrack, Global);
  }

  //
  // Search the VARIABLE_STORAGE_PROTOCOLs
  // first, try the protocol instance which the VariableStorageSelectorLib suggests
  //
  CorrectVariableStorageProtocol = NULL;
  ZeroMem ((VOID *) &VariableStorageId, sizeof (EFI_GUID));
  VariableStorageSelectorProtocol = mVariableModuleGlobal->VariableBase.VariableStorageSelectorProtocol;
   if (VariableStorageSelectorProtocol == NULL) {
     return EFI_NOT_FOUND;
   }

  Status = VariableStorageSelectorProtocol->GetId (VariableName, VendorGuid, &VariableStorageId);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0;
       Index < mVariableModuleGlobal->VariableBase.VariableStoresCount;
       Index++) {
    VariableStorageProtocol = mVariableModuleGlobal->VariableBase.VariableStores[Index];
    ZeroMem ((VOID *) &InstanceGuid, sizeof (EFI_GUID));
    Status = VariableStorageProtocol->GetId (&InstanceGuid);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if (CompareGuid (&VariableStorageId, &InstanceGuid)) {
      CorrectVariableStorageProtocol = VariableStorageProtocol;
      DataSize = SCRATCH_SIZE;
      ZeroMem (mVariableDataBuffer, DataSize);
      Status = VariableStorageProtocol->GetAuthenticatedVariable (
                                          VariableStorageProtocol,
                                          VariableName,
                                          VendorGuid,
                                          &Attributes,
                                          &DataSize,
                                          mVariableDataBuffer,
                                          &KeyIndex,
                                          &MonotonicCount,
                                          &TimeStamp
                                          );
      if (!EFI_ERROR (Status)) {
        goto UpdateNvCache;
      }
      if (Status != EFI_NOT_FOUND) {
        return Status;
      }
      break;
    }
  }
  //
  // As a fallback, try searching the remaining VARIABLE_STORAGE_PROTOCOLs even
  // though the variable shouldn't actually be stored in there
  //
  for (Index = 0;
       Index < mVariableModuleGlobal->VariableBase.VariableStoresCount;
       Index++) {
    VariableStorageProtocol = mVariableModuleGlobal->VariableBase.VariableStores[Index];
    ZeroMem ((VOID *) &InstanceGuid, sizeof (EFI_GUID));
    Status = VariableStorageProtocol->GetId (&InstanceGuid);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if (!CompareGuid (&VariableStorageId, &InstanceGuid)) {
      DataSize = SCRATCH_SIZE;
      ZeroMem (mVariableDataBuffer, DataSize);
      Status = VariableStorageProtocol->GetAuthenticatedVariable (
                                          VariableStorageProtocol,
                                          VariableName,
                                          VendorGuid,
                                          &Attributes,
                                          &DataSize,
                                          mVariableDataBuffer,
                                          &KeyIndex,
                                          &MonotonicCount,
                                          &TimeStamp
                                          );
      if (!EFI_ERROR (Status)) {
        if (CorrectVariableStorageProtocol != NULL) {
          if (CorrectVariableStorageProtocol->WriteServiceIsReady (
                                                CorrectVariableStorageProtocol) &&
              VariableStorageProtocol->WriteServiceIsReady (
                                        VariableStorageProtocol)) {
            Status2 = CorrectVariableStorageProtocol->SetVariable (
                                                        CorrectVariableStorageProtocol,
                                                        VariableName,
                                                        VendorGuid,
                                                        Attributes,
                                                        DataSize,
                                                        mVariableDataBuffer,
                                                        VariableAtRuntime (),
                                                        KeyIndex,
                                                        MonotonicCount,
                                                        &TimeStamp
                                                        );
            if (EFI_ERROR (Status2)) {
              goto UpdateNvCache;
            }
            //
            // Delete the redundant copy that is incorrectly stored
            //
            Status2 = VariableStorageProtocol->SetVariable (
                                                 VariableStorageProtocol,
                                                 VariableName,
                                                 VendorGuid,
                                                 Attributes,
                                                 0,
                                                 NULL,
                                                 VariableAtRuntime (),
                                                 0,
                                                 0,
                                                 &TimeStamp
                                                 );
          }
        }
        goto UpdateNvCache;
      }
      if (Status != EFI_NOT_FOUND) {
        return Status;
      }
    }
  }
  return EFI_NOT_FOUND;

UpdateNvCache:
  PtrTrack->CurrPtr                 = NULL;
  PtrTrack->InDeletedTransitionPtr  = NULL;
  PtrTrack->StartPtr                = GetStartPointer (mVariableModuleGlobal->NonVolatileVariableCache);
  PtrTrack->EndPtr                  = GetEndPointer   (mVariableModuleGlobal->NonVolatileVariableCache);
  PtrTrack->Volatile                = FALSE;
  Status = UpdateVariableInternal (
             VariableName,
             VendorGuid,
             mVariableDataBuffer,
             DataSize,
             Attributes,
             KeyIndex,
             MonotonicCount,
             PtrTrack,
             &TimeStamp,
             TRUE
             );
  if (!EFI_ERROR (Status)) {
    PtrTrack->StartPtr = GetStartPointer (mVariableModuleGlobal->NonVolatileVariableCache);
    PtrTrack->EndPtr   = GetEndPointer   (mVariableModuleGlobal->NonVolatileVariableCache);
    PtrTrack->Volatile = FALSE;
    Status = FindVariableEx (VariableName, VendorGuid, PtrTrack, VariableCount);
  }
  return Status;
}

STATIC
UINTN
GetMaxUserVariableSpace (
  VOID
  )
{
  if (mSmst == NULL) {
    return mVariableModuleGlobal->CommonMaxUserVariableSpace;
  }
  return mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->CommonMaxUserVariableSpace;
}

STATIC
UINTN
GetUserVariableTotalSize (
  VOID
  )
{
  if (mSmst == NULL) {
    return mVariableModuleGlobal->CommonUserVariableTotalSize;
  }
  return mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->CommonUserVariableTotalSize;
}

/**
  Gets the VARIABLE_STORAGE_PROTOCOL for the NV Storage that should store
  a variable with the given name and GUID

  @param[in]   VariableName             Name of the variable to be found.
  @param[in]   VendorGuid               Vendor GUID to be found.
  @param[out]  VariableStorageProtocol  The found VARIABLE_STORAGE_PROTOCOL

  @retval EFI_INVALID_PARAMETER       If VariableName is an empty string, or
                                      VendorGuid is NULL.
  @retval EFI_SUCCESS                 VARIABLE_STORAGE_PROTOCOL successfully found.
  @retval EFI_NOT_FOUND               VARIABLE_STORAGE_PROTOCOL not found

**/
EFI_STATUS
GetVariableStorageProtocol (
  IN  CHAR16                      *VariableName,
  IN  EFI_GUID                    *VendorGuid,
  OUT VARIABLE_STORAGE_PROTOCOL   **VariableStorageProtocol
  )
{
  EFI_GUID                           VariableStorageId;
  EFI_GUID                           InstanceGuid;
  EFI_STATUS                         Status;
  UINTN                              Index;
  VARIABLE_STORAGE_SELECTOR_PROTOCOL *VariableStorageSelectorProtocol;
  VARIABLE_STORAGE_PROTOCOL          *CurrentInstance;

  if (VariableName[0] == 0 || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  VariableStorageSelectorProtocol = mVariableModuleGlobal->VariableBase.VariableStorageSelectorProtocol;
  if (VariableStorageSelectorProtocol == NULL) {
    return EFI_NOT_FOUND;
  }
  ZeroMem ((VOID *) &VariableStorageId, sizeof (EFI_GUID));
  Status = VariableStorageSelectorProtocol->GetId (VariableName, VendorGuid, &VariableStorageId);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  for (Index = 0;
       Index < mVariableModuleGlobal->VariableBase.VariableStoresCount;
       Index++) {
    CurrentInstance = mVariableModuleGlobal->VariableBase.VariableStores[Index];
    ZeroMem ((VOID *) &InstanceGuid, sizeof (EFI_GUID));
    Status = CurrentInstance->GetId (&InstanceGuid);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if (CompareGuid (&VariableStorageId, &InstanceGuid)) {
      *VariableStorageProtocol = CurrentInstance;
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}


/**
  Record variable error flag.

  @param[in] Flag               Variable error flag to record.
  @param[in] VariableName       Name of variable.
  @param[in] VendorGuid         Guid of variable.
  @param[in] Attributes         Attributes of the variable.
  @param[in] VariableSize       Size of the variable.

**/
VOID
RecordVarErrorFlag (
  IN VAR_ERROR_FLAG         Flag,
  IN CHAR16                 *VariableName,
  IN EFI_GUID               *VendorGuid,
  IN UINT32                 Attributes,
  IN UINTN                  VariableSize
  )
{
  EFI_STATUS                Status;
  VARIABLE_POINTER_TRACK    Variable;
  VAR_ERROR_FLAG            *VarErrFlag;
  VAR_ERROR_FLAG            TempFlag;
  UINTN                     VariableCount;

  if (!AfterEndOfDxe ()) {
    //
    // Before EndOfDxe, just record the current boot variable error flag to local variable,
    // and leave the variable error flag in NV flash as the last boot variable error flag.
    // After EndOfDxe in InitializeVarErrorFlag (), the variable error flag in NV flash
    // will be initialized to this local current boot variable error flag.
    //
    mCurrentBootVarErrFlag &= Flag;
    return;
  }


  //
  // Record error flag (it should have be initialized).
  //
  VariableCount = 0;
  Status = FindVariable (
             VAR_ERROR_FLAG_NAME,
             &gEdkiiVarErrorFlagGuid,
             &Variable,
             &VariableCount,
             &mVariableModuleGlobal->VariableBase
             );
  if (!EFI_ERROR (Status)) {
    VarErrFlag = (VAR_ERROR_FLAG *) GetVariableDataPtr (Variable.CurrPtr);
    TempFlag = *VarErrFlag;
    TempFlag &= Flag;
    if (TempFlag == *VarErrFlag) {
      return;
    }
    Status = UpdateVariableStore (
               &mVariableModuleGlobal->VariableBase,
               FALSE,
               FALSE,
               (UINTN) VarErrFlag,
               sizeof (TempFlag),
               &TempFlag
               );
    if (!EFI_ERROR (Status)) {
      //
      // Update the data in NV cache.
      //
      *VarErrFlag = Flag;
    }
  }
}

/**
  Initialize variable error flag.

  Before EndOfDxe, the variable indicates the last boot variable error flag,
  then it means the last boot variable error flag must be got before EndOfDxe.
  After EndOfDxe, the variable indicates the current boot variable error flag,
  then it means the current boot variable error flag must be got after EndOfDxe.

**/
VOID
InitializeVarErrorFlag (
  VOID
  )
{
  EFI_STATUS                Status;
  VARIABLE_POINTER_TRACK    Variable;
  VAR_ERROR_FLAG            Flag;
  VAR_ERROR_FLAG            VarErrFlag;
  UINTN                     VariableCount;

  if (!AfterEndOfDxe ()) {
    return;
  }

  Flag = mCurrentBootVarErrFlag;
  VariableCount = 0;
  Status = FindVariable (
             VAR_ERROR_FLAG_NAME,
             &gEdkiiVarErrorFlagGuid,
             &Variable,
             &VariableCount,
             &mVariableModuleGlobal->VariableBase
             );
  if (!EFI_ERROR (Status)) {
    VarErrFlag = *((VAR_ERROR_FLAG *) GetVariableDataPtr (Variable.CurrPtr));
    if (VarErrFlag == Flag) {
      return;
    }
  }

  UpdateVariable (
    VAR_ERROR_FLAG_NAME,
    &gEdkiiVarErrorFlagGuid,
    &Flag,
    sizeof (Flag),
    VARIABLE_ATTRIBUTE_NV_BS_RT,
    0,
    0,
    &Variable,
    NULL,
    &mVariableModuleGlobal->VariableBase
    );
}

/**
  Is user variable?

  @param VariableName   Name of Variable to be found.
  @param VendorGuid     Variable vendor GUID.

  @retval TRUE          User variable.
  @retval FALSE         System variable.

**/
BOOLEAN
IsUserVariable (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid
  )
{
  VAR_CHECK_VARIABLE_PROPERTY   Property;

  //
  // Only after End Of Dxe, the variables belong to system variable are fixed.
  // If PcdMaxUserNvStorageVariableSize is 0, it means user variable share the same NV storage with system variable,
  // then no need to check if the variable is user variable or not specially.
  //
  if (AfterEndOfDxe () && (GetMaxUserVariableSpace () != 0)) {
    if (InternalVarCheckVariablePropertyGet (VariableName, VendorGuid, &Property) == EFI_NOT_FOUND) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Calculate common user variable total size.

**/
VOID
CalculateCommonUserVariableTotalSize (
  VOID
  )
{
  VARIABLE_HEADER               *Variable;
  VARIABLE_HEADER               *NextVariable;
  UINTN                         VariableSize;
  VAR_CHECK_VARIABLE_PROPERTY   Property;

  //
  // Only after End Of Dxe, the variables belong to system variable are fixed.
  // If PcdMaxUserNvStorageVariableSize is 0, it means user variable share the same NV storage with system variable,
  // then no need to calculate the common user variable total size specially.
  //
  if (AfterEndOfDxe ()  && (GetMaxUserVariableSpace () != 0)) {
    Variable = GetStartPointer (mVariableModuleGlobal->NonVolatileVariableCache);
    while (IsValidVariableHeaderInVarRegion (Variable, GetNonVolatileEndPointer (mVariableModuleGlobal->NonVolatileVariableCache))) {
      NextVariable = GetNextVariablePtr (Variable);
      VariableSize = (UINTN) NextVariable - (UINTN) Variable;
      if ((Variable->Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) != EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
        if (InternalVarCheckVariablePropertyGet ((CHAR16 *) (Variable + 1), &Variable->VendorGuid, &Property) == EFI_NOT_FOUND) {
          //
          // No property, it is user variable.
          //
          if (mSmst == NULL) {
            mVariableModuleGlobal->CommonUserVariableTotalSize += VariableSize;
          } else {
            mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->CommonUserVariableTotalSize += VariableSize;
          }
        }
      }

      Variable = NextVariable;
    }
  }
}

/**
  Initialize variable quota.

**/
VOID
InitializeVariableQuota (
  VOID
  )
{
  STATIC BOOLEAN    Initialized = FALSE;

  if (!AfterEndOfDxe () || Initialized) {
    return;
  }
  Initialized = TRUE;

  InitializeVarErrorFlag ();
  CalculateCommonUserVariableTotalSize ();
}

/**
  Delete all variables which the state is VAR_ADDED & VAR_IN_DELETED_TRANSITION and
  attribute is non-volative.

  @param  VariableName      Name of the variable which will be deleted
  @param  VendorGuid        GUID of the variable which will be deleted
  @param  Global            VARIABLE_GLOBAL pointer

  @retval EFI_SUCCESS       Delete successfully
  @return others            At least one variable delete fail

**/
EFI_STATUS
DeleteAllOldVariable (
  IN  CHAR16                  *VariableName,
  IN  EFI_GUID                *VendorGuid,
  IN  VARIABLE_GLOBAL         *Global
  )

{
  VARIABLE_HEADER       *Variable;
  VARIABLE_STORE_HEADER *VariableStoreHeader;
  UINT8                 State;
  EFI_STATUS            Status;
  EFI_STATUS            DeleteStatus;

  if (VariableName == NULL || VariableName[0] == 0 || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  VariableStoreHeader = mVariableModuleGlobal->NonVolatileVariableCache;
  Variable = GetStartPointer (VariableStoreHeader);

  DeleteStatus = EFI_SUCCESS;

  while (IsValidVariableHeaderInVarRegion (Variable, GetNonVolatileEndPointer (VariableStoreHeader))) {
    if ((Variable->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION)) &&
        ExistNewerVariable (Variable)) {
      if (!(VariableAtRuntime () && !(Variable->Attributes & EFI_VARIABLE_RUNTIME_ACCESS))) {
        if (CompareGuid (VendorGuid, &Variable->VendorGuid) &&
            !StrCmp (VariableName, GET_VARIABLE_NAME_PTR (Variable))) {
          State = Variable->State;
          State &= VAR_DELETED;
          Status = UpdateVariableStore (
                    Global,
                    FALSE,
                    FALSE,
                    (UINTN) &Variable->State,
                    sizeof (UINT8),
                    &State
                    );
          if (EFI_ERROR (Status)) {
            DeleteStatus = Status;
          }
        }
      }
    }

    Variable = GetNextVariablePtr (Variable);
  }

  return DeleteStatus;
}

/**
  non-volatile variable store garbage collection and reclaim operation

  @param  Variable                Pointer to new added variable.
  @param  VarSize                 The size of new added variable size.
  @param  VariableBase            Base address of variable store
  @param  LastVariableOffset      Offset of last variable

  @retval EFI_INVALID_PARAMETER   Any input parameter is invalid
  @retval EFI_OUT_OF_RESOURCES    Allocate pool failed or total variable size is large than variable store size.
  @retval EFI_SUCCESS             Reclaim non-volatile

--*/
EFI_STATUS
EFIAPI
ReclaimNonVolatileVariable (
  IN   VARIABLE_HEADER        *NewVariable,
  IN   UINTN                  NewVarSize,
  IN   EFI_PHYSICAL_ADDRESS   VariableBase,
  OUT  UINTN                  *LastVariableOffset
  )
{
  VARIABLE_HEADER             *NextVariable;
  VARIABLE_STORE_HEADER       *VariableStoreHeader;
  UINTN                       VariableSize;
  EFI_STATUS                  Status;
  UINTN                       CommonUserVariableTotalSize;
  VARIABLE_POINTER_TRACK      VarErrFlagPtr;
  VARIABLE_POINTER_TRACK      VariableTrack;
  UINTN                       VariableCount;
  VARIABLE_STORAGE_PROTOCOL   *VariableStorageProtocol;
  UINT8                       *CurrPtr;
  UINTN                       Index;

  POST_CODE (DXE_VARIABLE_RECLAIM);
  if (LastVariableOffset == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  CommonUserVariableTotalSize = 0;
  VariableStoreHeader = (VARIABLE_STORE_HEADER *) ((UINTN) VariableBase);

  //
  // Compute the changes to NV storage usage
  //
  if (mSmst == NULL) {
    CommonUserVariableTotalSize = mVariableModuleGlobal->CommonUserVariableTotalSize;
  } else {
    CommonUserVariableTotalSize = mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->CommonUserVariableTotalSize;
  }

  if (NewVariable != NULL) {
    VariableTrack.StartPtr = GetStartPointer (VariableStoreHeader);
    VariableTrack.EndPtr   = GetEndPointer   (VariableStoreHeader);
    VariableTrack.CurrPtr  = NULL;
    VariableTrack.Volatile = FALSE;
    VariableCount = 0;
    Status = FindVariableEx (
               GET_VARIABLE_NAME_PTR (NewVariable),
               &NewVariable->VendorGuid,
               &VariableTrack,
               &VariableCount
               );
    if (Status == EFI_SUCCESS) {
      CommonUserVariableTotalSize -= DataSizeOfVariable (VariableTrack.CurrPtr);
    }
    CommonUserVariableTotalSize += NewVariable->DataSize;
  }

  //
  // Retain VarErrorFlag in the NV cache
  //
  VarErrFlagPtr.CurrPtr = NULL;
  VariableSize = 0;
  if (mVariableModuleGlobal->EndOfDxe) {
    VarErrFlagPtr.StartPtr = GetStartPointer (VariableStoreHeader);
    VarErrFlagPtr.EndPtr   = GetEndPointer   (VariableStoreHeader);
    VarErrFlagPtr.Volatile = FALSE;
    if (IsValidVariableHeaderInVarRegion (VarErrFlagPtr.StartPtr, VarErrFlagPtr.EndPtr)) {
      Status = FindVariableEx (
                VAR_ERROR_FLAG_NAME,
                &gEdkiiVarErrorFlagGuid,
                &VarErrFlagPtr,
                &VariableCount
                );
      if (!EFI_ERROR (Status) && VarErrFlagPtr.CurrPtr != NULL) {
        NextVariable = GetNextVariablePtr (VarErrFlagPtr.CurrPtr);
        VariableSize = (UINTN) NextVariable - (UINTN) VarErrFlagPtr.CurrPtr;
      } else {
        VarErrFlagPtr.CurrPtr = NULL;
      }
    }
  }

  CurrPtr = (UINT8 *) GetStartPointer (VariableStoreHeader);
  SetMem (CurrPtr, GetNonVolatileVariableStoreSize (), 0xff);
  //
  // Delete all existing data
  //
  if (VarErrFlagPtr.CurrPtr != NULL) {
    CopyMem (CurrPtr, VarErrFlagPtr.CurrPtr, VariableSize);
    CurrPtr += VariableSize;
  }

  for ( Index = 0;
        Index < mVariableModuleGlobal->VariableBase.VariableStoresCount;
        Index++) {
    VariableStorageProtocol = mVariableModuleGlobal->VariableBase.VariableStores[Index];
    if (VariableStorageProtocol->WriteServiceIsReady (VariableStorageProtocol)) {
      Status = VariableStorageProtocol->GarbageCollect (VariableStorageProtocol);
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }
  }

  if (NewVariable != NULL) {
    VariableStorageProtocol = NULL;
    Status = GetVariableStorageProtocol (
              GET_VARIABLE_NAME_PTR (NewVariable),
              &NewVariable->VendorGuid,
              &VariableStorageProtocol
              );
    if (!EFI_ERROR (Status) && VariableStorageProtocol != NULL &&
        VariableStorageProtocol->WriteServiceIsReady (VariableStorageProtocol)) {
        Status = VariableStorageProtocol->SetVariable (
                                            VariableStorageProtocol,
                                            GET_VARIABLE_NAME_PTR (NewVariable),
                                            &NewVariable->VendorGuid,
                                            NewVariable->Attributes,
                                            NewVariable->DataSize,
                                            GetVariableDataPtr (NewVariable),
                                            VariableAtRuntime (),
                                            NewVariable->PubKeyIndex,
                                            ReadUnaligned64 (&(NewVariable->MonotonicCount)),
                                            &NewVariable->TimeStamp
                                            );
    }
    if (Status != EFI_SUCCESS) {
      return Status;
    }
    CopyMem (CurrPtr, (UINT8 *) NewVariable, NewVarSize);
    ((VARIABLE_HEADER *) CurrPtr)->State = VAR_ADDED;
    CurrPtr += NewVarSize;
  }
  if (mSmst == NULL) {
    mVariableModuleGlobal->CommonUserVariableTotalSize = CommonUserVariableTotalSize;
  } else {
    mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->CommonUserVariableTotalSize = CommonUserVariableTotalSize;
  }

  *LastVariableOffset = (UINTN) (CurrPtr - (UINT8 *) VariableStoreHeader);
  return EFI_SUCCESS;
}

/**
  Variable store garbage collection and reclaim operation.

  @param[in]      VariableBase           Base address of variable store
  @param[out]     LastVariableOffset     Offset of last variable
  @param[in, out] CurrentVairable        If it is not NULL, it means not to process
                                         current variable for Reclaim.

  @return EFI_OUT_OF_RESOURCES
  @return EFI_SUCCESS
  @return Others
**/
EFI_STATUS
EFIAPI
Reclaim (
  IN       EFI_PHYSICAL_ADDRESS  VariableBase,
  OUT      UINTN                 *LastVariableOffset,
  IN OUT   VARIABLE_HEADER       **CurrentVariable OPTIONAL
  )
{
  VARIABLE_HEADER             *Variable;
  VARIABLE_HEADER             *NextVariable;
  VARIABLE_STORE_HEADER       *VariableStoreHeader;
  UINT8                       *ValidBuffer;
  UINTN                       VariableSize;
  UINT8                       *CurrPtr;
  EFI_STATUS                  Status;
  VARIABLE_HEADER             *WorkingVariable;
  UINTN                       TotalBufferSize;
  UINTN                       CommonUserVariableTotalSize;


  POST_CODE (DXE_VARIABLE_RECLAIM);
  if (VariableAtRuntime ()) {
    return EFI_UNSUPPORTED;
  }

  VariableStoreHeader         = (VARIABLE_STORE_HEADER *) ((UINTN) VariableBase);
  Variable                    = GetStartPointer (VariableStoreHeader);
  CommonUserVariableTotalSize = 0;
  //
  // To make the reclaim, here we just allocate a memory that equal to the original memory
  //
  TotalBufferSize = GetVariableStoreSize (VariableStoreHeader);
  ValidBuffer = VariableAllocateZeroBuffer (TotalBufferSize, FALSE);
  if (ValidBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  WorkingVariable = NULL;
  SetMem (ValidBuffer, TotalBufferSize, 0xff);

  CurrPtr = ValidBuffer;

  //
  // Copy variable store header
  //
  CopyMem (CurrPtr, VariableStoreHeader, GetVariableStoreHeaderSize ());
  CurrPtr += GetVariableStoreHeaderSize ();
  for (Variable = GetStartPointer (VariableStoreHeader); IsValidVariableHeader (Variable); Variable = NextVariable) {
    NextVariable = GetNextVariablePtr (Variable);
    //
    // Check the contents of this variable is in variable range.
    //
    if ((UINTN) NextVariable - (UINTN) VariableBase > TotalBufferSize) {
      ASSERT (FALSE);
      break;
    }
    //
    // check variable name size is correct.
    //
    if (Variable->NameSize != StrSize (GET_VARIABLE_NAME_PTR (Variable))) {
      ASSERT (FALSE);
      continue;
    }
    //
    // State VAR_ADDED or VAR_IN_DELETED_TRANSITION are to kept,
    // The CurrentVariable, is also saved, as SetVariable may fail duo to lack of space
    //
    if (Variable->State == VAR_ADDED) {
      VariableSize = (UINTN) NextVariable - (UINTN) Variable;
      CopyMem (CurrPtr, (UINT8 *) Variable, VariableSize);
      CurrPtr += VariableSize;
      if (IsUserVariable ((CHAR16 *) (Variable + 1), &Variable->VendorGuid)) {
        CommonUserVariableTotalSize += VariableSize;
      }
    } else if (Variable->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION)) {
      //
      // As variables that with the same guid and name may exist in NV due to power failure during SetVariable,
      // we will only save the latest valid one
      //
      if (!ExistNewerVariable(Variable)) {
        VariableSize = (UINTN) NextVariable - (UINTN) Variable;
        CopyMem (CurrPtr, (UINT8 *) Variable, VariableSize);
        if (IsUserVariable ((CHAR16 *) (Variable + 1), &Variable->VendorGuid)) {
          CommonUserVariableTotalSize += VariableSize;
        }
        //
        // If CurrentVariable == Variable, mark as VAR_IN_DELETED_TRANSITION
        //
        if (CurrentVariable != NULL) {
          if (Variable != *CurrentVariable){
            ((VARIABLE_HEADER *)CurrPtr)->State = VAR_ADDED;
          } else  {
            WorkingVariable = (VARIABLE_HEADER *) ((UINTN) VariableBase + (UINTN) (CurrPtr - ValidBuffer));
          }
        }
        CurrPtr += VariableSize;
      }
    }
  }

  //
  // If volatile variable store, just copy valid buffer
  //
  SetMem ((UINT8 *) (UINTN) VariableBase, GetVariableStoreSize (VariableStoreHeader), 0xff);
  CopyMem ((UINT8 *) (UINTN) VariableBase, ValidBuffer, (UINTN) (CurrPtr - ValidBuffer));
  *LastVariableOffset = (UINTN) (CurrPtr - ValidBuffer);
  Status              = EFI_SUCCESS;

  if (EFI_ERROR (Status)) {
    *LastVariableOffset = 0;
  }
  if (!EFI_ERROR (Status) && CurrentVariable != NULL) {
    *CurrentVariable = WorkingVariable;
  }

  EFI_FREE_POOL (ValidBuffer);
  return Status;
}


/**
  This code finds variable in storage blocks (Volatile or Non-Volatile)

  @param VariableName                Name of the variable to be found
  @param VendorGuid                  Vendor GUID to be found.
  @param PtrTrack                    Variable Track Pointer structure that contains Variable Information.
  @param VariableCount               The number of found variabl.
  @param Global                      VARIABLE_GLOBAL pointer

  @retval EFI_INVALID_PARAMETER       If VariableName is not an empty string, while
                                      VendorGuid is NULL.
  @retval EFI_SUCCESS                 Variable successfully found.
  @retval EFI_NOT_FOUND               Variable not found

**/
EFI_STATUS
FindVariableByLifetime (
  IN        CHAR16                  *VariableName,
  IN        EFI_GUID                *VendorGuid,
  OUT       VARIABLE_POINTER_TRACK  *PtrTrack,
  OUT       UINTN                   *VariableCount,
  IN        VARIABLE_GLOBAL         *Global
  )
{
  EFI_STATUS         Status;

  Status = FindVariable (
             VariableName,
             VendorGuid,
             PtrTrack,
             VariableCount,
             Global
             );
  if (!EFI_ERROR (Status)) {
    if (VariableAtRuntime () && (PtrTrack->CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS) != EFI_VARIABLE_RUNTIME_ACCESS) {
      PtrTrack->CurrPtr = NULL;
      Status = EFI_NOT_FOUND;
    }
  }

  return Status;
}


/**
  Internal function to get current nonvolatile offset from physical hardware device.

  @return UINTN       The offset of the first free nonvolatile variable space.ata into this region.
**/
STATIC
UINTN
GetCurrentNonVolatileOffset (
  VOID
  )
{
  UINT8                   *CurrPtr;
  VARIABLE_HEADER         *LastVariable;

  CurrPtr   = (UINT8 *) ((UINTN) mVariableModuleGlobal->NonVolatileVariableCache);
  //
  // Update NonVolatileOffset to make sure the the NonVolatileOffset is correct
  //
  LastVariable = (VARIABLE_HEADER *) (CurrPtr + GetVariableStoreHeaderSize ());
  while (IsValidVariableHeader (LastVariable)) {
    LastVariable = GetNextVariablePtr (LastVariable);
  }
  return ((UINTN) LastVariable - (UINTN) CurrPtr);
}

/**
  Internal function to check VARIABLE_RECLAIM_THRESHOLD or set NEED_DO_RECLAIM_NAME variable to make sure system will
  do reclaim in next boot.

  @retval EFI_SUCCESS    Nonvolatile variable free space is smaller than VARIABLE_RECLAIM_THRESHOLD or
                         set NEED_DO_RECLAIM_NAME variable successfully.
  @return Other          Any error occurred while writing NEED_DO_RECLAIM_NAME variable.
**/
STATIC
EFI_STATUS
SetDoReclaimNextBoot (
  VOID
  )
{
  EFI_STATUS              Status;
  VARIABLE_POINTER_TRACK  Variable;
  UINTN                   VariableCount;
  UINT8                   Data;
  VARIABLE_GLOBAL         *Global;

  //
  // System will do reclaim next boot if free area is below a threshold, so just return EFI_SUCCESS if free is below
  // a threshold.
  //
  if ((GetNonVolatileVariableStoreSize () - GetCurrentNonVolatileOffset ()) < VARIABLE_RECLAIM_THRESHOLD) {
    return EFI_SUCCESS;
  }

  //
  // return EFI_SUCCESS directly if Need do reclaim variable is set
  //
  Global = &mVariableModuleGlobal->VariableBase;
  Status = FindVariableByLifetime (
             NEED_DO_RECLAIM_NAME,
             &gEfiGenericVariableGuid,
             &Variable,
             &VariableCount,
             Global
             );
  if (!EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }
  //
  // Write NEED_DO_RECLAIM_NAME variable to indicate system need do reclaim next POST.
  //
  Data = 1;
  return UpdateVariable (
           NEED_DO_RECLAIM_NAME,
           &gEfiGenericVariableGuid,
           &Data,
           sizeof (UINT8),
           EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
           0,
           0,
           &Variable,
           NULL,
           Global
           );
}

/**
  Internal function to check the existence of VARIABLE_RECLAIM_THRESHOLD variable.

  @retval TRUE    VARIABLE_RECLAIM_THRESHOLD variable exists.
  @retval FALSE   VARIABLE_RECLAIM_THRESHOLD variable doesn't exist.
**/
STATIC
BOOLEAN
DoesNeedDoReclaimVariableExist (
  VOID
  )
{
  EFI_STATUS              Status;
  VARIABLE_POINTER_TRACK  Variable;
  UINTN                   VariableCount;

  if (PcdGetBool(PcdRuntimeReclaimSupported)) {
    return FALSE;
  }

  Status = FindVariableByLifetime (
             NEED_DO_RECLAIM_NAME,
             &gEfiGenericVariableGuid,
             &Variable,
             &VariableCount,
             &mVariableModuleGlobal->VariableBase
             );

  return EFI_ERROR (Status) ? FALSE : TRUE;

}

STATIC
EFI_STATUS
DeleteNeedDoReclaimVariable (
  VOID
  )
/*++

Routine Description:

  Internal function to delete VARIABLE_RECLAIM_THRESHOLD variable.

Arguments:

  None

Returns:

  EFI_SUCCESS      - Delete VARIABLE_RECLAIM_THRESHOLD variable successfully.
  Other            - Any error occurred while deleting VARIABLE_RECLAIM_THRESHOLD variable.

--*/
{
  EFI_STATUS              Status;
  VARIABLE_POINTER_TRACK  Variable;
  UINTN                   VariableCount;

  if (PcdGetBool(PcdRuntimeReclaimSupported)) {
    return EFI_SUCCESS;
  }

  Status = FindVariableByLifetime (
             NEED_DO_RECLAIM_NAME,
             &gEfiGenericVariableGuid,
             &Variable,
             &VariableCount,
             &mVariableModuleGlobal->VariableBase
             );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  return UpdateVariable (
           NEED_DO_RECLAIM_NAME,
           &gEfiGenericVariableGuid,
           NULL,
           0,
           0,
           0,
           0,
           &Variable,
           NULL,
           &mVariableModuleGlobal->VariableBase
           );
}


/**
  Update the variable region with Variable information. If EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS is set,
  index of associated public key is needed.

  @param VariableName       Name of variable.
  @param VendorGuid         Guid of variable.
  @param Data               Variable data.
  @param DataSize           Size of data. 0 means delete.
  @param Attributes         Attributes of the variable.
  @param KeyIndex           Index of associated public key.
  @param MonotonicCount     Value of associated monotonic count.
  @param Variable           The variable information which is used to keep track of variable usage.
  @param TimeStamp          Value of associated TimeStamp.
  @param Global             Ponter to VARIABLE_GLOBAL.

  @retval EFI_SUCCESS           The update operation is success.
  @retval EFI_OUT_OF_RESOURCES  Variable region is full, can not write other data into this region.

**/
EFI_STATUS
UpdateVariable (
  IN       CHAR16                 *VariableName,
  IN       EFI_GUID               *VendorGuid,
  IN       VOID                   *Data,
  IN       UINTN                  DataSize,
  IN       UINT32                 Attributes,
  IN       UINT32                 KeyIndex        OPTIONAL,
  IN       UINT64                 MonotonicCount  OPTIONAL,
  IN       VARIABLE_POINTER_TRACK *Variable,
  IN       EFI_TIME               *TimeStamp      OPTIONAL,
  IN       VARIABLE_GLOBAL        *Global
  )
 {
//[-start-161020-IB07400800-add]//
   if (PcdGetBool (PcdFDOState) == 1) {
     return UpdateVariableInternal (
              VariableName,
              VendorGuid,
              Data,
              DataSize,
              Attributes,
              KeyIndex,
              MonotonicCount,
              Variable,
              TimeStamp,
//[-start-161202-IB07400821-modify]//
////[-start-161128-IB11270169-modify]//
//              PcdGetBool (PcdForceVolatileVariable)
////[-end-161128-IB11270169-modify]//
              TRUE
//[-end-161202-IB07400821-modify]//
              );
   }
//[-end-161020-IB07400800-add]//
   return UpdateVariableInternal (
            VariableName,
            VendorGuid,
            Data,
            DataSize,
            Attributes,
            KeyIndex,
            MonotonicCount,
            Variable,
            TimeStamp,
            FALSE
            );
 }

/**
  Routine to confirm the existence of factory default.

  @retval TRUE      The factory default data exists
  @retval FALSE     The factory default data doesn't exist.

**/
STATIC
BOOLEAN
DoesFactoryDefaultExist (
  VOID
  )
{
  EFI_STATUS       Status;
  UINTN            DataSize;

  DataSize = 0;
  Status = GetSecureDatabaseDefaultVariables (EFI_PLATFORM_KEY_DEFAULT_NAME, &gEfiGlobalVariableGuid, NULL, &DataSize, NULL);
  return Status == EFI_BUFFER_TOO_SMALL;
}

/**
  This code sets authenticated variable in storage blocks (Volatile or Non-Volatile).

  @param VariableName                     Name of Variable to be found.
  @param VendorGuid                       Variable vendor GUID.
  @param Attributes                       Attribute value of the variable found
  @param DataSize                         Size of Data found. If size is less than the
                                          data, this value contains the required size.
  @param Data                             Data pointer.
  @param Global                           Pointer to VARIABLE_GLOBAL structure

  @retval EFI_INVALID_PARAMETER           Invalid parameter.
  @retval EFI_SUCCESS                     Set successfully.
  @retval EFI_OUT_OF_RESOURCES            Resource not enough to set variable.
  @retval EFI_NOT_FOUND                   Not found.
  @retval EFI_WRITE_PROTECTED             Variable is read-only.

**/
EFI_STATUS
EFIAPI
SetAuthVariable (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid,
  IN UINT32                  Attributes,
  IN UINTN                   DataSize,
  IN VOID                    *Data,
  IN VARIABLE_GLOBAL         *Global
  )
{
  VARIABLE_POINTER_TRACK              Variable;
  EFI_STATUS                          Status;
  UINTN                               PayloadSize;
  UINTN                               VariableCount;

  if (!FeaturePcdGet (PcdH2OSecureBootSupported) && IsSecureDatabaseVariable (VariableName, VendorGuid)) {
    return EFI_WRITE_PROTECTED;
  }
  //
  // Check input parameters.
  //
  if (VariableName == NULL || VariableName[0] == 0 || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (DataSize != 0 && Data == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // UEFI 2.3.1 - check for incompatible attributes
  //
  if ((Attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) &&
    (Attributes &  EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  //  Make sure if runtime bit is set, boot service bit is set also.
  //
  if ((Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == EFI_VARIABLE_RUNTIME_ACCESS) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsInsydeSecureVariable (Attributes, DataSize, Data)) {
    if (!IsValidInsydeSecureVariable (VariableName, VendorGuid, Attributes, DataSize, Data)) {
      return EFI_SECURITY_VIOLATION;
    }
    PayloadSize = DataSize;
  } else if ((Attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) == EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) {
    if (DataSize < AUTHINFO_SIZE) {
      //
      // Try to write Authencated Variable without AuthInfo.
      //
      return EFI_SECURITY_VIOLATION;
    }
    PayloadSize = DataSize - AUTHINFO_SIZE;

  } else if ((Attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) == EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) {
    if (DataSize < AUTHINFO2_SIZE (Data)) {
      //
      // Try to write Authencated Variable without AuthInfo.
      //
      return EFI_SECURITY_VIOLATION;
    }
    PayloadSize = DataSize - AUTHINFO2_SIZE (Data);
  } else {
    PayloadSize = DataSize;
  }

  //
  //  The size of the VariableName, including the Unicode Null in bytes plus
  //  the DataSize is limited to maximum size of PcdGet32 (PcdMaxHardwareErrorVariableSize)
  //  bytes for HwErrRec, and PcdGet32 (PcdMaxVariableSize) bytes for the others.
  //
  if ((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
    if ((PayloadSize > MAX_HARDWARE_ERROR_VARIABLE_SIZE) ||
        (sizeof (VARIABLE_HEADER) + StrSize (VariableName) + PayloadSize > MAX_HARDWARE_ERROR_VARIABLE_SIZE)) {
      return EFI_INVALID_PARAMETER;
    }
    //
    // According to UEFI spec, HARDWARE_ERROR_RECORD variable name convention should be L"HwErrRecXXXX".
    //
    if (StrnCmp (VariableName, L"HwErrRec", StrLen (L"HwErrRec")) != 0) {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    //  The size of the VariableName, including the Unicode Null in bytes plus
    //  the DataSize is limited to maximum size of PcdGet32 (PcdMaxVariableSize) bytes.
    //
    if ((PayloadSize > MAX_VARIABLE_SIZE) ||
        (sizeof (VARIABLE_HEADER) + StrSize (VariableName) + PayloadSize > MAX_VARIABLE_SIZE)) {
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Check whether the input variable is already existed.
  //
  VariableCount = 0;
  Status = FindVariableByLifetime (VariableName, VendorGuid, &Variable, &VariableCount, Global);

  //
  // Process PK, KEK, Sigdb seperately.
  //
  if (IsPkVariable (VariableName, VendorGuid)){
    Status = ProcessVarWithPk (VariableName, VendorGuid, Data, DataSize, &Variable, Attributes, TRUE, Global);
    if (!EFI_ERROR (Status) && DoesFactoryDefaultExist ()) {
      //
      // Set "CustomSecurity" varialbe to 1 indicates secure boot database has been modified by user and machine isn't in factory.
      //
      Status = UpdateCustomSecurityStatus (1);
    }

  } else if (IsKekVariable (VariableName, VendorGuid)) {
    Status = ProcessVarWithPk (VariableName, VendorGuid, Data, DataSize, &Variable, Attributes, FALSE, Global);
  } else if (IsImageSecureDatabaseVariable (VariableName, VendorGuid)) {
    if (IsDbtVariable (VariableName, VendorGuid) && (PcdGet64 (PcdOsIndicationsSupported) & EFI_OS_INDICATIONS_TIMESTAMP_REVOCATION) == 0) {
      return EFI_WRITE_PROTECTED;
    }
    Status = ProcessVarWithPk (VariableName, VendorGuid, Data, DataSize, &Variable, Attributes, FALSE, Global);
    if (EFI_ERROR (Status)) {
      Status = ProcessVarWithKek (VariableName, VendorGuid, Data, DataSize, &Variable, Attributes, Global);
    }
  } else if (IsInsydeSecureVariable (Attributes, DataSize, Data)) {
    Status = ProcessInsydeSecureVariable (VariableName, VendorGuid, Data, DataSize, &Variable, Attributes, Global);
  } else {
    Status = ProcessVariable (VariableName, VendorGuid, Data, DataSize, &Variable, Attributes, Global);
  }

  return Status;
}

/**
  Calculate the sum of all elements in a buffer in unit of UINT8.
  During calculation, the carry bits are dropped.

  @param  Buffer   Pointer to the buffer to carry out the sum operation.
  @param  Length   The size, in bytes, of Buffer.

  @return Sum      The sum of Buffer with carry bits dropped during additions.

**/
UINT16
EFIAPI
InternalCalculateSum16 (
  IN      CONST UINT8         *Buffer,
  IN      UINTN               Length
  )
{
  UINT32    Sum;
  UINTN     Count;

  for (Sum = 0, Count = 0; Count < Length; Count++) {
    Sum = (UINT32) (Sum + *(Buffer + Count));
  }

  Sum = 0x10000 - (Sum & 0x0000ffff);

  return (UINT16) Sum;
}


/**
  Internal function for set variable through SMI.

  @param VariableName              Name of Variable to be found
  @param VendorGuid                Variable vendor GUID
  @param Attributes                Attribute value of the variable found
  @param DataSize                  Size of Data found. If size is less than the
                                   data, this value contains the required size.
  @param Data                      Data pointer

  @retval EFI_INVALID_PARAMETER    Invalid parameter
  @retval EFI_SUCCESS              Set successfully
  @retval EFI_OUT_OF_RESOURCES     Resource not enough to set variable
  @retval EFI_NOT_FOUND            Not found
  @retval EFI_SECURITY_VIOLATION   The variable could not be written due to EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS
                                   set but the AuthInfo does NOT pass the validation check carried out by the firmware.

**/
EFI_STATUS
EFIAPI
SetVariableThroughSmi (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid,
  IN UINT32                  Attributes,
  IN UINTN                   DataSize,
  IN VOID                    *Data
  )
{
  EFI_STATUS                  Status;
  UINT8                       *WorkingBuf;
  UINTN                       SmmBufSize;

  SmmBufSize = sizeof (SMM_VAR_BUFFER) + MAX_VARIABLE_NAME_SIZE + MAX_VARIABLE_SIZE;
  SetMem (mSmmVarBuf, SmmBufSize, 0x0);
  mSmmVarBuf->Signature = SMM_VARIABLE_SIGNATURE;
  CopyMem (&mSmmVarBuf->VarGuid, VendorGuid, sizeof (EFI_GUID));
  mSmmVarBuf->Attributes = Attributes;
  mSmmVarBuf->VariableNameSize = StrSize (VariableName);
  mSmmVarBuf->DataSize = DataSize;
  mSmmVarBuf->VarChecksum = InternalCalculateSum16 ((UINT8 *)Data, DataSize);
  WorkingBuf = (UINT8 *) (mSmmVarBuf + 1);

  StrCpy ((CHAR16 *) WorkingBuf, VariableName);
  WorkingBuf += mSmmVarBuf->VariableNameSize;
  CopyMem (WorkingBuf, Data, DataSize);
  mSmmVarBuf->Status = EFI_UNSUPPORTED;
  SmmSecureBootCall ((UINT8 *) mSmmPhyVarBuf, SmmBufSize, SMM_SET_VARIABLE_SMI_FUN_NUM, SW_SMI_PORT);
  Status = mSmmVarBuf->Status;

  return Status;
}

/**
  Function uses to check BootOrder variable hook mechanism is whether enabled.

  @retval TRUE     BootOrder variable hook mechanism is enabled.
  @return FALSE    BootOrder variable hook mechanism is disabled.
**/
STATIC
BOOLEAN
IsBootOrderHookEnabled (
  VOID
  )
{
  return mSmst ? mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->BootOrderVariableHook : mVariableModuleGlobal->BootOrderVariableHook;
}

/**
  According to variable name and GUID to Determine the variable is BoorOrder or not.

  @param[in] VariableName  Name of Variable to be found.
  @param[in] VendorGuid    Variable vendor GUID.

  @retval TRUE     This is BoorOrder variable.
  @return FALSE    This isn't BoorOrder variable.
**/
STATIC
BOOLEAN
IsBootOrderVariable (
  IN     CHAR16                             *VariableName,
  IN     EFI_GUID                           *VendorGuid
  )
{
  if (VariableName != NULL && VendorGuid != NULL) {
    if (StrCmp (VariableName, L"BootOrder") == 0 && CompareGuid (VendorGuid, &gEfiGlobalVariableGuid)) {
      return TRUE;
    }
  }
  return FALSE;
}


/**
  Get the specific config data from MultiConfig region.

  @param  RequireKind           Find Setup Setting for SETUP_FOR_BIOS_POST or SETUP_FOR_LOAD_DEFAULT.
  @param  VariableName          A pointer to a null-terminated string that is the variable's name.
  @param  VariableGuid          A pointer to an EFI_GUID that is the variable's GUID. The combination of
                                VariableGuid and VariableName must be unique.
  @param  Attributes            If non-NULL, on return, points to the variable's attributes.
  @param  DataSize              On entry, points to the size in bytes of the Data buffer.
                                On return, points to the size of the data returned in Data.
  @param  Data                  Points to the buffer which will hold the returned variable value.

  @retval  EFI_SUCCESS               Config data found successfully
  @retval  EFI_NOT_FOUND          Config data not found
  @retval  EFI_INVALID_PARAMETER  Data is NULL
  @retval EFI_BUFFER_TOO_SMALL  The DataSize is too small for the resulting data.
                                                  DataSize is updated with the size required for
                                                  the specified variable.

**/
EFI_STATUS
GetConfigData (
  IN CONST  UINT8                   RequireKind,
  IN        CHAR16                  *VariableName,
  IN        EFI_GUID                *VariableGuid,
  OUT       UINT32                  *Attributes,
  IN OUT    UINTN                   *DataSize,
  OUT       VOID                    *Data
  )
{
  EFI_STATUS            Status;
  UINT16                ConfigCount;
  UINT16                Index;
  VOID                  *NamePtr;
  VOID                  *DataPtr;
  UINTN                 VarDataSize;

  Status = EFI_NOT_FOUND;
  Index = 0;
  ConfigCount = GetConfigCount();
  DataPtr = NULL;

  for ( ; Index < ConfigCount; Index ++) {
    //
    // Get Active (Attribute: ACTIVE & BIOS_POST) Full Setup Setting from Multi Config Region
    //
    Status = GetFullSetupSetting (
                  RequireKind,
                  &Index,
                  *DataSize,
                  &DataPtr,
                  NULL,
                  NULL
                  );
    if (!EFI_ERROR (Status) && DataPtr != NULL) {
      NamePtr = (VOID *) GET_VARIABLE_NAME_PTR (DataPtr);
      if (CompareMem (VariableName, NamePtr, NameSizeOfVariable (DataPtr)) == 0) {
        //
        // Get data size
        //
        VarDataSize = DataSizeOfVariable (DataPtr);
        if (*DataSize >= VarDataSize) {
          if (Data == NULL) {
            return EFI_INVALID_PARAMETER;
          }

          CopyMem (Data, GetVariableDataPtr (DataPtr), VarDataSize);

          if (Attributes != NULL) {
            *Attributes = ((VARIABLE_HEADER *)DataPtr)->Attributes;
          }

          *DataSize = VarDataSize;

          Status = VariableServicesSetVariable (
                       VariableName,
                       VariableGuid,
                       ((VARIABLE_HEADER *)DataPtr)->Attributes,
                       *DataSize,
                       Data
                       );
          return Status;
        } else {
          *DataSize = VarDataSize;
          return EFI_BUFFER_TOO_SMALL;
        }
      }
    }
  }
  return EFI_NOT_FOUND;
}

EFI_STATUS
GetVariableThroughSmi (
  IN      CHAR16            *VariableName,
  IN      EFI_GUID          *VendorGuid,
  OUT     UINT32            *Attributes OPTIONAL,
  IN OUT  UINTN             *DataSize,
  OUT     VOID              *Data
  )
{
  EFI_STATUS                  Status;
  UINT8                       *WorkingBuf;
  UINTN                       SmmBufSize;

  SmmBufSize = sizeof (SMM_VAR_BUFFER) + MAX_VARIABLE_NAME_SIZE + MAX_VARIABLE_SIZE;
  SetMem (mSmmVarBuf, SmmBufSize, 0x0);
  mSmmVarBuf->Signature = SMM_VARIABLE_SIGNATURE;
  CopyMem (&mSmmVarBuf->VarGuid, VendorGuid, sizeof (EFI_GUID));
  mSmmVarBuf->Attributes = 0;
  mSmmVarBuf->VariableNameSize = StrSize (VariableName);
  mSmmVarBuf->DataSize = *DataSize;
  mSmmVarBuf->VarChecksum = 0;
  WorkingBuf = (UINT8 *) (mSmmVarBuf + 1);
  StrCpy ((CHAR16 *) WorkingBuf, VariableName);
  SmmSecureBootCall ((UINT8 *) mSmmPhyVarBuf, SmmBufSize, SMM_GET_VARIABLE_SMI_FUN_NUM, SW_SMI_PORT);
  WorkingBuf += mSmmVarBuf->VariableNameSize;
  Status = mSmmVarBuf->Status;
  if (!EFI_ERROR (Status)) {
    CopyMem (Data, WorkingBuf, mSmmVarBuf->DataSize);
    *DataSize = mSmmVarBuf->DataSize;
    if (Attributes != NULL) {
      *Attributes = mSmmVarBuf->Attributes;
    }
  } else if (Status == EFI_BUFFER_TOO_SMALL) {
    *DataSize = mSmmVarBuf->DataSize;
  }

  return Status;
}

EFI_STATUS
GetNextVariableThroughSmi (
  IN OUT  UINTN             *VariableNameSize,
  IN OUT  CHAR16            *VariableName,
  IN OUT  EFI_GUID          *VendorGuid
  )
{
  EFI_STATUS                  Status;
  UINT8                       *WorkingBuf;
  UINTN                       SmmBufSize;

  SmmBufSize = sizeof (SMM_VAR_BUFFER) + MAX_VARIABLE_NAME_SIZE + MAX_VARIABLE_SIZE;
  SetMem (mSmmVarBuf, SmmBufSize, 0x0);
  mSmmVarBuf->Signature = SMM_VARIABLE_SIGNATURE;
  CopyMem (&mSmmVarBuf->VarGuid, VendorGuid, sizeof (EFI_GUID));
  mSmmVarBuf->Attributes = 0;
  mSmmVarBuf->VariableNameSize = *VariableNameSize;
  mSmmVarBuf->VarChecksum      = 0;
  WorkingBuf = (UINT8 *) (mSmmVarBuf + 1);
  StrCpy ((CHAR16 *) WorkingBuf, VariableName);
  SmmSecureBootCall ((UINT8 *) mSmmPhyVarBuf, SmmBufSize, SMM_GET_NEXT_VARIABLE_SMI_FUN_NUM, SW_SMI_PORT);
  Status = mSmmVarBuf->Status;
  if (!EFI_ERROR (Status)) {
    CopyMem (VariableName, WorkingBuf, mSmmVarBuf->VariableNameSize);
    CopyMem (VendorGuid, &mSmmVarBuf->VarGuid, sizeof (EFI_GUID));
    *VariableNameSize = mSmmVarBuf->VariableNameSize;
  } else if (Status == EFI_BUFFER_TOO_SMALL) {
    *VariableNameSize = mSmmVarBuf->VariableNameSize;
  }

  return Status;
}

/**
  This code finds variable in storage blocks (Volatile or Non-Volatile).

  @param VariableName               Name of Variable to be found.
  @param VendorGuid                 Variable vendor GUID.
  @param Attributes                 Attribute value of the variable found.
  @param DataSize                   Size of Data found. If size is less than the
                                    data, this value contains the required size.
  @param Data                       Data pointer.

  @return EFI_INVALID_PARAMETER     Invalid parameter.
  @return EFI_SUCCESS               Find the specified variable.
  @return EFI_NOT_FOUND             Not found.
  @return EFI_BUFFER_TO_SMALL       DataSize is too small for the result.

**/
EFI_STATUS
EFIAPI
VariableServicesGetVariable (
  IN      CHAR16            *VariableName,
  IN      EFI_GUID          * VendorGuid,
  OUT     UINT32            *Attributes OPTIONAL,
  IN OUT  UINTN             *DataSize,
  OUT     VOID              *Data
  )
{
  VARIABLE_POINTER_TRACK  Variable;
  UINTN                   VarDataSize;
  EFI_STATUS              Status;
  UINTN                   VariableCount;
  VARIABLE_GLOBAL         *Global;

  if (IsBootOrderHookEnabled () && IsBootOrderVariable (VariableName, VendorGuid)) {
   return VariableServicesGetVariable (
            L"PhysicalBootOrder",
            &gEfiGenericVariableGuid,
            Attributes,
            DataSize,
            Data
            );
  }


  if (VariableName == NULL || VendorGuid == NULL || DataSize == NULL || VariableName[0] == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (VariableAtRuntime () && mSmst == NULL) {
    return GetVariableThroughSmi (VariableName, VendorGuid, Attributes, DataSize, Data);
  }

  if (VariableAtRuntime ()) {
    WriteBackInvalidateDataCache ();
  }
//[-start-170406-IB07250279-add]//
  AcquireLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
//[-end-170406-IB07250279-add]//
  //
  // Cehck Secure Database Default variable first.
  //
  if (FeaturePcdGet (PcdH2OSecureBootSupported) && IsSecureDatabaseDefaultVariable (VariableName, VendorGuid)) {
     Status = GetSecureDatabaseDefaultVariables (VariableName, VendorGuid, Attributes, DataSize, Data);
     goto Done;
  }
  //
  // Find existing variable
  //

  Global = &mVariableModuleGlobal->VariableBase;
  VariableCount = 0;
  Status = FindVariableByLifetime (VariableName, VendorGuid, &Variable, &VariableCount, Global);
  if (!(VariableAtRuntime ()) && FeaturePcdGet (PcdMultiConfigSupported) && (Status == EFI_NOT_FOUND)) {
    Status = GetConfigData (SETUP_FOR_BIOS_POST, VariableName, VendorGuid, Attributes, DataSize, Data);
    if (Status == EFI_NOT_FOUND) {
      Status = GetConfigData (SETUP_FOR_LOAD_DEFAULT, VariableName, VendorGuid, Attributes, DataSize, Data);
    }
    if (!EFI_ERROR (Status)) {
      goto Done;
    }
  }
  if (Variable.CurrPtr == NULL || EFI_ERROR (Status)) {
    goto Done;
  }

  if (IsAdminPasswordVariable (VariableName, VendorGuid)) {
    //
    // Set data size to 1 and contents of data to 1 to indicate the existence of admin password.
    //
    VarDataSize = sizeof (UINT8);
  } else {
    VarDataSize = Variable.CurrPtr->DataSize;
  }
  if (*DataSize >= VarDataSize) {
    if (Data == NULL) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
    if (IsAdminPasswordVariable (VariableName, VendorGuid)) {
      //
      // Set data size to 1 and contents of data to 1 to indicate the existence of admin password.
      //
      SetMem (Data, sizeof (UINT8), 0x01);
    } else {
      CopyMem (Data, GetVariableDataPtr (Variable.CurrPtr), VarDataSize);
    }

    if (Attributes != NULL) {
      //
      // According to AuditMode and DeployedMode should return EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS.
      // But they stored in non-volatile storage to make sure them can be still available after reset system.
      //
      if (IsReadOnlyVariable (VariableName, VendorGuid) ||
          (FeaturePcdGet (PcdH2OCustomizedSecureBootSupported) &&
          (IsAuditModeVariable (VariableName, VendorGuid) || IsDeployedModeVariable (VariableName, VendorGuid)))) {
        //
        // According EFI spec 3.2 Globally defined variables, convert all of read-only variables to
        // fit specification definition.
        //
        *Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
      } else {
        *Attributes = Variable.CurrPtr->Attributes;
      }

    }
    *DataSize = VarDataSize;
    Status = EFI_SUCCESS;
  } else {
    *DataSize = VarDataSize;
    Status = EFI_BUFFER_TOO_SMALL;
  }

Done:
//[-start-170406-IB07250279-add]//
  ReleaseLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
//[-end-170406-IB07250279-add]//
  return Status;
}

/**
  This code Finds the Next available secure boot database default variable.

  @param[in, out] VariableNameSize Size of the variable.
  @param[in, out] VariableName     Pointer to variable name.
  @param[in, out] VendorGuid       Variable Vendor Guid.

  @retval EFI_SUCCESS              Invalid parameter.
  @retval EFI_BUFFER_TOO_SMALL     Find the specified variable.
  @retval EFI_NOT_FOUND            Not found.
**/
STATIC
EFI_STATUS
GetNextDefaultVariableName (
  IN OUT  UINTN             *VariableNameSize,
  IN OUT  CHAR16            *VariableName,
  IN OUT  EFI_GUID          *VendorGuid
  )
{
  EFI_STATUS        Status;
  UINTN             Index;
  UINTN             CurrentIndex;
  CHAR16            *DefaultVariableName[DEFAULT_VARIALBE_NUM] = {
                       EFI_PLATFORM_KEY_DEFAULT_NAME,
                       EFI_KEY_EXCHANGE_KEY_DEFAULT_NAME,
                       EFI_IMAGE_SECURITY_DEFAULT_DATABASE,
                       EFI_IMAGE_SECURITY_DEFAULT_DATABASE1,
                       EFI_IMAGE_SECURITY_DEFAULT_DATABASE2
                       };

  Index = 0;
  if (VariableName[0] != 0) {
    for (Index = 0; Index < DEFAULT_VARIALBE_NUM; Index++) {
      if (StrCmp (DefaultVariableName[Index], VariableName) == 0) {
        Index++;
        break;
      }
    }
  }

  for (CurrentIndex = Index; CurrentIndex < DEFAULT_VARIALBE_NUM; CurrentIndex++) {
    Status = FindSecureDatabaseDefaultVariables (DefaultVariableName[CurrentIndex], &gEfiGlobalVariableGuid, NULL, NULL);
    if (!EFI_ERROR (Status)) {
      if (StrSize (DefaultVariableName[CurrentIndex]) > *VariableNameSize) {
        *VariableNameSize = StrSize (DefaultVariableName[CurrentIndex]);
        return EFI_BUFFER_TOO_SMALL;
      } else {
        *VariableNameSize = StrSize (DefaultVariableName[CurrentIndex]);
        CopyMem (VariableName, DefaultVariableName[CurrentIndex], *VariableNameSize);
        CopyMem (VendorGuid, &gEfiGlobalVariableGuid, sizeof (EFI_GUID));
        return EFI_SUCCESS;
      }
    }
  }

  return EFI_NOT_FOUND;
}

/**
  This code Finds the Next available variable.

  Caution: This function may receive untrusted input.
  This function may be invoked in SMM mode. This function will do basic validation, before parse the data.

  @param[in]  VariableName  Pointer to variable name.
  @param[in]  VendorGuid    Variable Vendor Guid.
  @param[out] VariablePtr   Pointer to variable header address.

  @return EFI_SUCCESS       Find the specified variable.
  @return EFI_NOT_FOUND     Not found.

**/
EFI_STATUS
EFIAPI
VariableServiceGetNextInMemoryVariableInternal (
  IN  CHAR16                *VariableName,
  IN  EFI_GUID              *VendorGuid,
  OUT VARIABLE_HEADER       **VariablePtr,
  OUT BOOLEAN               *CurrentVariableInMemory
  )
{
  VARIABLE_POINTER_TRACK  Variable;
  VARIABLE_POINTER_TRACK  VariablePtrTrack;
  EFI_STATUS              Status;
  VARIABLE_STORE_HEADER   *VariableStoreHeader;
  UINTN                   VariableCount;

  *CurrentVariableInMemory = FALSE;
  if (VariableName[0] != 0 && VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  VariableStoreHeader = (VARIABLE_STORE_HEADER *) (UINTN) mVariableModuleGlobal->VariableBase.VolatileVariableBase;
  //
  // Find the current variable by walk through HOB and volatile variable store.
  //
  Variable.CurrPtr  = NULL;
  Variable.StartPtr = NULL;
  Variable.EndPtr   = NULL;
  Status            = EFI_NOT_FOUND;

  if (VariableStoreHeader == NULL) {
    return EFI_NOT_FOUND;
  }

  Variable.StartPtr = GetStartPointer (VariableStoreHeader);
  Variable.EndPtr   = GetEndPointer   (VariableStoreHeader);
  Variable.Volatile = TRUE;

  Status = FindVariableEx (VariableName, VendorGuid, &Variable, &VariableCount);
  if (EFI_ERROR (Status) || Variable.CurrPtr == NULL ||
     (VariableAtRuntime () && (Variable.CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS) == 0)) {
    goto Done;
  }

  *CurrentVariableInMemory = TRUE;

  if (VariableName[0] != 0) {
    //
    // If variable name is not NULL, get next variable.
    //
    Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
  }

  while (TRUE) {
    //
    // Switch from Volatile to HOB
    //
    if (!IsValidVariableHeaderInVarRegion (Variable.CurrPtr, Variable.EndPtr)) {
      return EFI_NOT_FOUND;
    }

    //
    // Variable is found
    //
    if (Variable.CurrPtr->State == VAR_ADDED || Variable.CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
      if (!VariableAtRuntime () || ((Variable.CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS) != 0)) {
        if (Variable.CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
          //
          // If it is a IN_DELETED_TRANSITION variable,
          // and there is also a same ADDED one at the same time,
          // don't return it.
          //
          VariablePtrTrack.StartPtr = Variable.StartPtr;
          VariablePtrTrack.EndPtr = Variable.EndPtr;
          Status = FindVariableEx (
                     GET_VARIABLE_NAME_PTR (Variable.CurrPtr),
                     &Variable.CurrPtr->VendorGuid,
                     &VariablePtrTrack,
                     &VariableCount
                     );
          if (!EFI_ERROR (Status) && VariablePtrTrack.CurrPtr->State == VAR_ADDED) {
            Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
            continue;
          }
        }

        *VariablePtr = Variable.CurrPtr;
        Status = EFI_SUCCESS;
        goto Done;
      }
    }

    Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
  }

Done:
  return Status;
}

/**
  This code Finds the Next available variable.

  Caution: This function may receive untrusted input.
  This function may be invoked in SMM mode. This function will do basic validation, before parse the data.

  @param VariableNameSize   Size of the variable name.
  @param VariableName       Pointer to variable name.
  @param VendorGuid         Variable Vendor Guid.

  @return EFI_SUCCESS       Find the specified variable.
  @return EFI_NOT_FOUND     Not found.

**/
EFI_STATUS
EFIAPI
VariableServiceGetNextVariableInternal (
  IN OUT UINTN              *VariableNameSize,
  IN OUT CHAR16             *VariableName,
  IN OUT EFI_GUID           *VariableGuid
  )
{
  EFI_GUID                           VariableStorageId;
  EFI_GUID                           InstanceGuid;
  VARIABLE_HEADER                    *VariablePtr;
  UINTN                              Index;
  UINTN                              VarNameSize;
  UINT32                             VarAttributes;
  UINTN                              CallerVariableNameBufferSize;
  EFI_STATUS                         Status;
  BOOLEAN                            SearchComplete;
  BOOLEAN                            CurrentVariableInMemory;
  CHAR16                             SavedChar;
  VARIABLE_STORAGE_PROTOCOL          *VariableStorage;
  VARIABLE_STORAGE_SELECTOR_PROTOCOL *VariableStorageSelectorProtocol;

  CallerVariableNameBufferSize =  *VariableNameSize;

  //
  // Check the volatile and HOB variables first
  //
  Status = VariableServiceGetNextInMemoryVariableInternal (
             VariableName,
             VariableGuid,
             &VariablePtr,
             &CurrentVariableInMemory
             );
  if (!EFI_ERROR (Status)) {
    VarNameSize = NameSizeOfVariable (VariablePtr);
    ASSERT (VarNameSize != 0);
    if (VarNameSize <= *VariableNameSize) {
      CopyMem (VariableName, GET_VARIABLE_NAME_PTR (VariablePtr), VarNameSize);
      CopyMem (VariableGuid, &VariablePtr->VendorGuid, sizeof (EFI_GUID));
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_BUFFER_TOO_SMALL;
    }

    *VariableNameSize = VarNameSize;
    return Status;
  } else if (Status != EFI_NOT_FOUND) {
    return Status;
  }

  //
  // If VariableName is an empty string or we reached the end of the volatile
  // and HOB variables, get the first variable from the first NV storage
  //
  if (VariableName[0] == 0 || (Status == EFI_NOT_FOUND && CurrentVariableInMemory)) {
    if (mVariableModuleGlobal->VariableBase.VariableStoresCount <= 0) {
      return EFI_NOT_FOUND;
    }
    ZeroMem ((VOID *) VariableName, *VariableNameSize);
    ZeroMem ((VOID *) VariableGuid, sizeof (VariableGuid));
    VariableStorage = mVariableModuleGlobal->VariableBase.VariableStores[0];
    Status = VariableStorage->GetNextVariableName (
                                VariableStorage,
                                VariableNameSize,
                                VariableName,
                                VariableGuid,
                                &VarAttributes
                                );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // Don't return this variable if we are at runtime and the variable's attributes do not include
    // the EFI_VARIABLE_RUNTIME_ACCESS flag. If this is true, advance to the next variable
    //
    if (((VarAttributes & EFI_VARIABLE_RUNTIME_ACCESS) != 0) || !VariableAtRuntime ()) {
      return Status;
    }
  }
  VariableStorageSelectorProtocol = mVariableModuleGlobal->VariableBase.VariableStorageSelectorProtocol;
  if (VariableStorageSelectorProtocol == NULL) {
    return EFI_NOT_FOUND;
  }

  SearchComplete = FALSE;
  while (!SearchComplete) {
    Status = VariableStorageSelectorProtocol->GetId (VariableName, VariableGuid, &VariableStorageId);
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
    VarNameSize = CallerVariableNameBufferSize;
    for ( Index = 0;
          Index < mVariableModuleGlobal->VariableBase.VariableStoresCount;
          Index++) {
      VariableStorage = mVariableModuleGlobal->VariableBase.VariableStores[Index];
      ZeroMem ((VOID *) &InstanceGuid, sizeof (EFI_GUID));
      Status = VariableStorage->GetId (&InstanceGuid);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (!CompareGuid (&VariableStorageId, &InstanceGuid)) {
        continue;
      }
      Status = VariableStorage->GetNextVariableName (
                                  VariableStorage,
                                  &VarNameSize,
                                  VariableName,
                                  VariableGuid,
                                  &VarAttributes
                                  );
      if (!EFI_ERROR (Status)) {
        if (((VarAttributes & EFI_VARIABLE_RUNTIME_ACCESS) == 0) && VariableAtRuntime ()) {
          //
          // Don't return this variable if we are at runtime and the variable's attributes do not include
          // the EFI_VARIABLE_RUNTIME_ACCESS flag. If this is true, advance to the next variable
          //
          break;
        }
        goto Done;
      }

      if (Status != EFI_NOT_FOUND) {
        return Status;
      }
      //
      // If we reached the end of the variables in the current NV storage
      // get the first variable in the next NV storage
      //
      if ((Index + 1) < mVariableModuleGlobal->VariableBase.VariableStoresCount) {
        VarNameSize = CallerVariableNameBufferSize;
        VariableStorage = mVariableModuleGlobal->VariableBase.VariableStores[Index + 1];

        ZeroMem ((VOID *) VariableGuid, sizeof (VariableGuid));
        Status = VariableStorage->GetNextVariableName (
                                    VariableStorage,
                                    &VarNameSize,
                                    VariableName,
                                    VariableGuid,
                                    &VarAttributes
                                    );
        if (!EFI_ERROR (Status)) {
          if (((VarAttributes & EFI_VARIABLE_RUNTIME_ACCESS) == 0) && VariableAtRuntime ()) {
            //
            // Don't return this variable if we are at runtime and the variable's attributes do not include
            // the EFI_VARIABLE_RUNTIME_ACCESS flag. If this is true, advance to the next variable
            //
            break;
          }
        }
        goto Done;
      }

      if (FeaturePcdGet (PcdH2OSecureBootSupported)) {
        SavedChar = VariableName[0];
        VariableName[0] = 0;
        *VariableNameSize = CallerVariableNameBufferSize;
        Status = GetNextDefaultVariableName (VariableNameSize, VariableName, VariableGuid);
        if (EFI_ERROR (Status)) {
          //
          // If cannot get next variable next, restore first character.
          //
          VariableName[0] = SavedChar;
        }
        return Status;
      }
      // This is the last variable
      SearchComplete = TRUE;
      break;
    }
  }

  return EFI_NOT_FOUND;

Done:
  *VariableNameSize = VarNameSize;

  if (CallerVariableNameBufferSize < VarNameSize) {
    return EFI_BUFFER_TOO_SMALL;
  }

  return EFI_SUCCESS;
}

//[-start-161020-IB07400800-add]//
EFI_STATUS
EFIAPI
VariableServicesGetNextVariableName2 (
  IN OUT  UINTN             *VariableNameSize,
  IN OUT  CHAR16            *VariableName,
  IN OUT  EFI_GUID          *VendorGuid
  )
{
  VARIABLE_POINTER_TRACK  Variable;
  UINTN                   VarNameSize;
  EFI_STATUS              Status;
  UINTN                   VariableCount;
  VARIABLE_GLOBAL         *Global;
  CHAR16                  SavedChar;
  BOOLEAN                 InteruptEnabled;


  if (VariableNameSize == NULL || VariableName == NULL || VendorGuid == NULL) {

    return EFI_INVALID_PARAMETER;
  }

  InteruptEnabled = GetInterruptState ();
  if (VariableAtRuntime () && InteruptEnabled) {
    DisableInterrupts ();
  }
  if (VariableAtRuntime () ) {
    WriteBackInvalidateDataCache ();
  }

  if (FeaturePcdGet (PcdH2OSecureBootSupported) && IsSecureDatabaseDefaultVariable (VariableName, VendorGuid)) {
    Status = GetNextDefaultVariableName (VariableNameSize, VariableName, VendorGuid);
    goto Done;
  }

  Global = &mVariableModuleGlobal->VariableBase;
  VariableCount = 0;
  Status = FindVariableByLifetime (VariableName, VendorGuid, &Variable, &VariableCount, Global);

  if (Variable.CurrPtr == NULL || EFI_ERROR (Status)) {
    goto Done;
  }

  if (VariableName[0] != 0) {
    //
    // If variable name is not NULL, get next variable
    //
    Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
  }

  while (TRUE) {
    //
    // If both volatile and non-volatile variable store are parsed,
    // return not found
    //
    if (Variable.CurrPtr >= Variable.EndPtr || Variable.CurrPtr == NULL) {
      Variable.Volatile = (BOOLEAN) (Variable.Volatile ^ ((BOOLEAN) 0x1));
      if (Variable.Volatile) {
        Variable.StartPtr = GetStartPointer ((VARIABLE_STORE_HEADER *) (UINTN) (Global->VolatileVariableBase));
        Variable.EndPtr   = GetEndPointer ((VARIABLE_STORE_HEADER *) (UINTN) (Global->VolatileVariableBase));
      } else {
        //
        // Skip finding security default variables if PcdH2OSecureBootSupported PCD is FALSE.
        //
        if (!FeaturePcdGet (PcdH2OSecureBootSupported)) {
          Status = EFI_NOT_FOUND;
          goto Done;
        }
        //
        // Assume secure boot database default variable is at the end of whole data base,
        // so start to get these variables after all variable.
        //
        SavedChar = VariableName[0];
        VariableName[0] = 0;
        Status = GetNextDefaultVariableName (VariableNameSize, VariableName, VendorGuid);
        if (EFI_ERROR (Status)) {
          //
          // If cannot get next variable next, restore first character.
          //
          VariableName[0] = SavedChar;
        }
        goto Done;
      }

      Variable.CurrPtr = Variable.StartPtr;
      if (!IsValidVariableHeaderInVarRegion (Variable.CurrPtr, Variable.EndPtr)) {

        continue;
      }
    }
    //
    // Variable is found
    //

    if (IsValidVariableHeaderInVarRegion (Variable.CurrPtr, Variable.EndPtr) &&
        ((Variable.CurrPtr->State == VAR_ADDED) ||
         ((Variable.CurrPtr->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION)) &&
          !ExistNewerVariable (Variable.CurrPtr)))) {
      ASSERT (Variable.CurrPtr->NameSize == StrSize (GET_VARIABLE_NAME_PTR (Variable.CurrPtr)));
      if (!(VariableAtRuntime () && !(Variable.CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS)) &&
           Variable.CurrPtr->NameSize == StrSize (GET_VARIABLE_NAME_PTR (Variable.CurrPtr))) {
        VarNameSize = Variable.CurrPtr->NameSize;
        if (VarNameSize <= *VariableNameSize) {
          CopyMem (
            VariableName,
            GET_VARIABLE_NAME_PTR (Variable.CurrPtr),
            VarNameSize
            );
          CopyMem (
            VendorGuid,
            &Variable.CurrPtr->VendorGuid,
            sizeof (EFI_GUID)
            );
          Status = EFI_SUCCESS;
        } else {
          Status = EFI_BUFFER_TOO_SMALL;
        }

        *VariableNameSize = VarNameSize;
        goto Done;
      }
    }

    Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
  }
  Status = EFI_NOT_FOUND;

Done:
  if (VariableAtRuntime () && InteruptEnabled) {
    EnableInterrupts ();
  }
  return Status;
}
//[-end-161020-IB07400800-add]//

/**
  This code Finds the Next available variable.

  @param VariableNameSize           Size of the variable name.
  @param VariableName               Pointer to variable name.
  @param VendorGuid                 Variable Vendor Guid.

  @retval EFI_INVALID_PARAMETER     Invalid parameter.
  @retval EFI_SUCCESS               Find the specified variable.
  @retval EFI_NOT_FOUND             Not found.
  @retval EFI_BUFFER_TO_SMALL       DataSize is too small for the result.
**/
EFI_STATUS
EFIAPI
VariableServicesGetNextVariableName (
  IN OUT  UINTN             *VariableNameSize,
  IN OUT  CHAR16            *VariableName,
  IN OUT  EFI_GUID          *VendorGuid
  )
{
  EFI_STATUS              Status;
  UINTN                   VariableNameBufferSize;
  EFI_GUID                WorkingVendorGuid;

  if (VariableNameSize == NULL || VariableName == NULL || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (VariableAtRuntime () && mSmst == NULL) {
    return GetNextVariableThroughSmi (VariableNameSize, VariableName, VendorGuid);
  }

  if (VariableAtRuntime ()) {
    WriteBackInvalidateDataCache ();
  }
//[-start-170406-IB07250279-add]//
  AcquireLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
//[-end-170406-IB07250279-add]//

  if (FeaturePcdGet (PcdH2OSecureBootSupported) && IsSecureDatabaseDefaultVariable (VariableName, VendorGuid)) {
    Status = GetNextDefaultVariableName (VariableNameSize, VariableName, VendorGuid);
    goto Done;
  }
  
//[-start-161020-IB07400800-add]//
//[-start-170412-IB07400858-modify]//
  if (PcdGetBool (PcdFDOState) == 1) {
    Status = VariableServicesGetNextVariableName2 (VariableNameSize, VariableName, VendorGuid);
    goto Done;
  }
//[-end-170412-IB07400858-modify]//
//[-end-161020-IB07400800-add]//

  VariableNameBufferSize = sizeof (mVariableNameBuffer);
  ZeroMem ((VOID *) &mVariableNameBuffer[0], VariableNameBufferSize);
  StrCpyS (&mVariableNameBuffer[0], VariableNameBufferSize, VariableName);
  CopyGuid (&WorkingVendorGuid, VendorGuid);
  Status = VariableServiceGetNextVariableInternal (&VariableNameBufferSize, &mVariableNameBuffer[0], &WorkingVendorGuid);
  if (!EFI_ERROR (Status)) {
    if (VariableNameBufferSize > *VariableNameSize) {
      *VariableNameSize = VariableNameBufferSize;
      Status = EFI_BUFFER_TOO_SMALL;
    } else {
      StrCpyS (VariableName, *VariableNameSize, &mVariableNameBuffer[0]);
      CopyGuid (VendorGuid, &WorkingVendorGuid);
      *VariableNameSize = VariableNameBufferSize;
    }
  }

Done:
//[-start-170406-IB07250279-add]//
  ReleaseLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
//[-end-170406-IB07250279-add]//
  return Status;
}

/**
  Check if a Unicode character is a hexadecimal character.

  This function checks if a Unicode character is a
  hexadecimal character.  The valid hexadecimal character is
  L'0' to L'9', L'a' to L'f', or L'A' to L'F'.

  @param[in] Char       The character to check against.

  @retval TRUE          If the Char is a hexadecmial character.
  @retval FALSE         If the Char is not a hexadecmial character.
**/
BOOLEAN
EFIAPI
IsHexaDecimalDigitCharacter (
  IN CHAR16             Char
  )
{
  return (BOOLEAN) ((Char >= L'0' && Char <= L'9') || (Char >= L'A' && Char <= L'F') || (Char >= L'a' && Char <= L'f'));
}

/**
  No access attribute (0) is used to delete variable. This function is used update no access
  attribute to correct attribute if variable exists and is authenticated variable and the
  variable data format is correct.

  @param VariableName                     Name of Variable to be found.
  @param VendorGuid                       Variable vendor GUID.
  @param DataSize                         Size of Data found. If size is less than the
                                          data, this value contains the required size.
  @param Data                             Data pointer.
  @param Attributes                       Pointer for attributes.
                                          [in]: must be 0 to indicate this is no access attributes.
                                          [out]: the attributes of pre-exist variable.

  @retval EFI_SUCCESS                     Update attribute successful.
  @retval EFI_INVALID_PARAMETER           Any input parameter is invalid.
  @retval EFI_NOT_FOUND                   Cannot find pre-exist variable.

**/
EFI_STATUS
UpdateNoAccessAttribute (
  IN     CHAR16          *VariableName,
  IN     EFI_GUID        *VendorGuid,
  IN     UINTN           DataSize,
  IN     VOID            *Data,
  IN OUT UINT32          *Attributes
  )
{
  UINTN                      VariableCount;
  VARIABLE_POINTER_TRACK     Variable;
  EFI_STATUS                 Status;
  BOOLEAN                    IsAuthenVariable;

  if (VariableName == NULL || VariableName[0] == 0 || VendorGuid == NULL ||
      Attributes == NULL || *Attributes != 0) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_NOT_FOUND;
  VariableCount = 0;
  Status = FindVariableByLifetime (VariableName, VendorGuid, &Variable, &VariableCount, &mVariableModuleGlobal->VariableBase);
  if (!EFI_ERROR (Status)) {
    Status = EFI_SUCCESS;
    IsAuthenVariable = FALSE;
    if ((Variable.CurrPtr->Attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) == EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) {
      IsAuthenVariable = TRUE;
      if (DataSize != AUTHINFO_SIZE) {
        Status = EFI_INVALID_PARAMETER;
      }
    } else if ((Variable.CurrPtr->Attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) == EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) {
      IsAuthenVariable = TRUE;
      //
      // Check datasize to prevent from reading data from no access address. If we don't do this check, any access
      // this no access address in runtime will cause blue screen. (ex: WHCK8324 SecureBoot manual test 05-ClearTestConfiguration)
      //
      if (DataSize == 0 || Data == NULL || DataSize != AUTHINFO2_SIZE (Data)) {
        Status = EFI_INVALID_PARAMETER;
      }
    }

    if (Status == EFI_SUCCESS && IsAuthenVariable) {
      *Attributes = Variable.CurrPtr->Attributes;
    }
  }

  return Status;
}

/**
  Internal function to check system is whether executing runtime code in SMM.

  @retval TRUE      System executes runtime code in SMM.
  @retval FALSE     System isn't in SMM or run SMM code in SMM.

**/
STATIC
BOOLEAN
DoesRunRuntimeCodeInSmm (
  VOID
  )
{
  EFI_SMM_BASE2_PROTOCOL     *SmmBase;
  BOOLEAN                    InSmm;
  EFI_STATUS                 Status;

  if (mSmst != NULL) {
    return FALSE;
  }
  if (VariableAtRuntime ()) {
    return FALSE;
  }
  Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **) &SmmBase
                  );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }
  InSmm = FALSE;
  SmmBase->InSmm (SmmBase, &InSmm);

  return InSmm;
}

/**
  Internal function to check is whether PK variable exist.

  @retval TRUE    PK variable exist.
  @retval FALSE   PK variable doesn't exist.
**/
BOOLEAN
DoesPkExist (
  VOID
  )
{
  VARIABLE_POINTER_TRACK  Variable;
  EFI_STATUS              Status;
  UINTN                   VariableCount;

  Status = FindVariable (
             EFI_PLATFORM_KEY_NAME,
             &gEfiGlobalVariableGuid,
             &Variable,
             &VariableCount,
             &mVariableModuleGlobal->VariableBase
             );
  return Status == EFI_SUCCESS;
}

/**
  Update related variables if any of secure modes variable is changed.

  @param[in] VariableName        A Null-terminated string that is the name of the vendor¡¦s variable.
  @param[in] VendorGuid          A unique identifier for the vendor.
**/
VOID
UpdateStateForModeVariablesChanged (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid
  )
{
  VARIABLE_POINTER_TRACK  Variable;
  EFI_STATUS              Status;
  UINTN                   VariableCount;
  UINT8                   *AuditMode;
  UINT8                   *DeployedMode;
  UINT8                   *SetupMode;

  if ((!FeaturePcdGet (PcdH2OSecureBootSupported)) ||
      (!IsAuditModeVariable (VariableName, VendorGuid) && !IsDeployedModeVariable (VariableName, VendorGuid))) {
    return;
  }
  //
  // Needn't do anything if user deletes AuditMode or DeployedMode variable.
  // (Not allow to delete AuditMode or DeployedMode even if the value of these variables is 0).
  //
  Status = FindVariableByLifetime (VariableName, VendorGuid, &Variable, &VariableCount, &mVariableModuleGlobal->VariableBase);
  if (Status != EFI_SUCCESS) {
    return;
  }

  if (IsAuditModeVariable (VariableName, VendorGuid)) {
    //
    // Needn't do anything if user creates new AuditMode variable with value 0.
    // It means the secure boot modes isn't changed.
    //
    AuditMode = GetVariableDataPtr (Variable.CurrPtr);
    if (*AuditMode == 0) {
      return;
    }
    //
    // Change related variable if system wants to operate in audit mode.
    //
    Status = FindVariableByLifetime (
               EFI_SETUP_MODE_NAME,
               &gEfiGlobalVariableGuid,
               &Variable,
               &VariableCount,
               &mVariableModuleGlobal->VariableBase
               );
    if (Status != EFI_SUCCESS) {
      return;
    }
    SetupMode = GetVariableDataPtr (Variable.CurrPtr);
    //
    // 1. Update AuditMode and DeployedMode to read-only if system wants to operate in audit mode.
    //
    UpdateAuditModeProperty (VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY);
    UpdateDeployedModeProperty (VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY);
    if (*SetupMode == 0) {
      //
      // 2.If system operates in user mode previously, need delete PK variable, change SetupMode
      // to 1 and SecureBoot to 0.
      //
      Status = FindVariableByLifetime (
                 EFI_PLATFORM_KEY_NAME,
                 &gEfiGlobalVariableGuid,
                 &Variable,
                 &VariableCount,
                 &mVariableModuleGlobal->VariableBase
                 );
      ASSERT (Status == EFI_SUCCESS);
      Status = UpdateVariable (
                 EFI_PLATFORM_KEY_NAME,
                 &gEfiGlobalVariableGuid,
                 NULL,
                 0,
                 0,
                 0,
                 0,
                 &Variable,
                 NULL,
                 &mVariableModuleGlobal->VariableBase
                 );
      ASSERT (Status == EFI_SUCCESS);
      UpdatePlatformMode (SETUP_MODE, &mVariableModuleGlobal->VariableBase);
      UpdatePlatformBootMode (SECURE_BOOT_MODE_DISABLE, &mVariableModuleGlobal->VariableBase);
      //
      // Update status to indicate security signature database is modified.
      //
      UpdateCustomSecurityStatus (1);
    }
  } else if (IsDeployedModeVariable (VariableName, VendorGuid)) {
    //
    // Needn't do anything if user creates new Deployed variable with value 0.
    // It means the secure boot modes isn't changed.
    //
    DeployedMode = GetVariableDataPtr (Variable.CurrPtr);
    if (*DeployedMode == 0) {
      return;
    }
    //
    // Update AuditMode and DeployedMode to read-only if system wants to operate in deployed mode.
    //
    UpdateAuditModeProperty (VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY);
    UpdateDeployedModeProperty (VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY);
  }
}
/**
  This code sets variable in storage blocks (Volatile or Non-Volatile).

  @param VariableName                     Name of Variable to be found.
  @param VendorGuid                       Variable vendor GUID.
  @param Attributes                       Attribute value of the variable found
  @param DataSize                         Size of Data found. If size is less than the
                                          data, this value contains the required size.
  @param Data                             Data pointer.

  @retval EFI_INVALID_PARAMETER           Invalid parameter.
  @retval EFI_SUCCESS                     Set successfully.
  @retval EFI_OUT_OF_RESOURCES            Resource not enough to set variable.
  @retval EFI_NOT_FOUND                   Not found.
  @retval EFI_WRITE_PROTECTED             Variable is read-only.

**/
EFI_STATUS
EFIAPI
VariableServicesSetVariable (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid,
  IN UINT32                  Attributes,
  IN UINTN                   DataSize,
  IN VOID                    *Data
  )
{
  VARIABLE_POINTER_TRACK  Variable;
  EFI_STATUS              Status;
  UINTN                   VariableCount;
  VARIABLE_GLOBAL         *Global;
  UINTN                   PayloadSize;

//[-start-161215-IB10860217-add]//
  if (mSmst != NULL && !mEnableLocking  && IsRestoreFactoryDefault(VariableName, VendorGuid, Attributes) && DataSize == 1) {
    Global = &mVariableModuleGlobal->VariableBase;	
  
    if (Global->ReentrantState != 0) {
      return EFI_ACCESS_DENIED;
    }

    Global->ReentrantState++;

    VariableCount = 0;
    Status = FindVariable (
               VariableName,
               VendorGuid,
               &Variable,
               &VariableCount,
               Global
               );

    Status = UpdateVariable (
             VariableName,
             VendorGuid,
             Data,
             DataSize,
             Attributes,
             0,
             0,
             &Variable,
             0,
             Global
             );

	goto Done;		 
  }
//[-end-161215-IB10860217-add]//
           
  if (IsBootOrderHookEnabled () && IsBootOrderVariable (VariableName, VendorGuid)) {
   return VariableServicesSetVariable (
            L"PhysicalBootOrder",
            &gEfiGenericVariableGuid,
            Attributes,
            DataSize,
            Data
            );
  }


  if (VariableName == NULL || VariableName[0] == 0 || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check for reserverd bit in variable attribute.
  //
  if ((Attributes & (~EFI_VARIABLE_ATTRIBUTES_MASK)) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  //  Make sure if runtime bit is set, boot service bit is set also
  //
  if ((Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == EFI_VARIABLE_RUNTIME_ACCESS) {
    return EFI_INVALID_PARAMETER;
  }
  PayloadSize = DataSize;
  //
  //  The size of the VariableName, including the Unicode Null in bytes plus
  //  the DataSize is limited to maximum size of MAX_HARDWARE_ERROR_VARIABLE_SIZE
  //  bytes for HwErrRec, and MAX_VARIABLE_SIZE bytes for the others.
  //
  if ((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
    if ((DataSize > MAX_HARDWARE_ERROR_VARIABLE_SIZE) ||
        (sizeof (VARIABLE_HEADER) + StrSize (VariableName) + DataSize > MAX_HARDWARE_ERROR_VARIABLE_SIZE)) {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    if ((Attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) == EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) {
      if (!IsInsydeSecureVariable (Attributes, DataSize, Data)) {
        if (DataSize < AUTHINFO_SIZE) {
          //
          // Try to write Authenticated Variable without AuthInfo.
          //
          return EFI_SECURITY_VIOLATION;
        }
        PayloadSize = DataSize - AUTHINFO_SIZE;
      } else {
        if (DataSize < sizeof (WIN_CERTIFICATE_UEFI_GUID) - sizeof (UINT8)) {
          return EFI_SECURITY_VIOLATION;
        }
        PayloadSize = DataSize - (sizeof (WIN_CERTIFICATE_UEFI_GUID) - sizeof (UINT8));
      }
    } else if ((Attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) == EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) {
      //
      // Sanity check for EFI_VARIABLE_AUTHENTICATION_2 descriptor.
      //
      if (DataSize < OFFSET_OF_AUTHINFO2_CERT_DATA ||
        ((EFI_VARIABLE_AUTHENTICATION_2 *) Data)->AuthInfo.Hdr.dwLength > DataSize - (OFFSET_OF (EFI_VARIABLE_AUTHENTICATION_2, AuthInfo)) ||
        ((EFI_VARIABLE_AUTHENTICATION_2 *) Data)->AuthInfo.Hdr.dwLength < OFFSET_OF (WIN_CERTIFICATE_UEFI_GUID, CertData)) {
        return EFI_SECURITY_VIOLATION;
      }
      PayloadSize = DataSize - AUTHINFO2_SIZE (Data);
    }
    if ((UINTN)(~0) - PayloadSize < StrSize(VariableName)){
      //
      // Prevent whole variable size overflow
      //
      return EFI_INVALID_PARAMETER;
    }
    //
    //  The size of the VariableName, including the Unicode Null in bytes plus
    //  the DataSize is limited to maximum size of MAX_VARIABLE_SIZE bytes.
    //
    if (sizeof (VARIABLE_HEADER) + StrSize (VariableName) + PayloadSize > MAX_VARIABLE_SIZE) {
      return EFI_INVALID_PARAMETER;
    }

    if (GetMaxUserVariableSpace () != 0 && (Attributes & EFI_VARIABLE_NON_VOLATILE) == EFI_VARIABLE_NON_VOLATILE &&
        IsUserVariable (VariableName, VendorGuid)) {
      if (sizeof (VARIABLE_HEADER) + StrSize (VariableName) + PayloadSize + GetUserVariableTotalSize () > GetMaxUserVariableSpace ()) {
        RecordVarErrorFlag (VAR_ERROR_FLAG_SYSTEM_ERROR, VariableName, VendorGuid, Attributes, PayloadSize);
        return EFI_OUT_OF_RESOURCES;
      }
    }

  }
  Global = &mVariableModuleGlobal->VariableBase;


  if (IsVariableLocked (VariableName, VendorGuid)) {
    return EFI_WRITE_PROTECTED;
  }

  Status = InternalVarCheckSetVariableCheck (VariableName, VendorGuid, Attributes, PayloadSize, (VOID *) ((UINTN) Data + DataSize - PayloadSize));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (VariableAtRuntime ()) {
    WriteBackInvalidateDataCache ();
  }

  if (Global->ReentrantState != 0) {
    return EFI_ACCESS_DENIED;
  }

  Global->ReentrantState++;
//[-start-170406-IB07250279-add]//
  AcquireLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
//[-end-170406-IB07250279-add]//

  if (FeaturePcdGet (PcdH2OCustomizedSecureBootSupported) &&
      (IsAuditModeVariable (VariableName, VendorGuid) || IsDeployedModeVariable (VariableName, VendorGuid))) {
    //
    // For AuditMode and DeployedMode variables the value should be 0 or 1
    //
    if ((Attributes == 0 || DataSize == 0) || (*((UINT8 *) Data) != 0 && *((UINT8 *) Data) != 1)) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
    Attributes = VARIABLE_ATTRIBUTE_NV_BS_RT;
  }

  //
  // Some CPU spends much more time to access normal memory region if system is in SMM. Therefore, we check
  // normal nonvolatile variable data in protected mode and don't call SetupVariable in SMM if variable data
  // and attributes are all the same.
  //
//[-start-170418-IB15590029-remove]//
//  if (mReadyToBootEventSignaled && mSmst == NULL && mVariableModuleGlobal->SmmCodeReady &&
//      (Attributes == (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS) ||
//       Attributes == (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS))) {
//    Status = FindVariable (
//               VariableName,
//               VendorGuid,
//               &Variable,
//               &VariableCount,
//               Global
//               );
//    if (!EFI_ERROR (Status) && Variable.CurrPtr->Attributes == Attributes && DataSizeOfVariable (Variable.CurrPtr) == DataSize &&
//        (CompareMem (Data, GetVariableDataPtr (Variable.CurrPtr), DataSize) == 0)) {
//      Status = EFI_SUCCESS;
//      goto Done;
//    }
//  }
//[-end-170418-IB15590029-remove]//
  //
  // Some CPU spends much more time to access normal memory region if system is in SMM. Therefore, we always set
  // volatile variable in protected mode to enhance performance.
  //
  if ((mReadyToBootEventSignaled || IsInsydeSecureVariable (Attributes, DataSize, Data) || IsAdminPasswordVariable (VariableName, VendorGuid) || IsAnyMorVariable (VariableName, VendorGuid))
      && mSmst == NULL && mVariableModuleGlobal->SmmCodeReady && ((Attributes & EFI_VARIABLE_NON_VOLATILE) == EFI_VARIABLE_NON_VOLATILE || Attributes == 0)) {
    if (Attributes != 0 && (IsAuditModeVariable (VariableName, VendorGuid) || IsDeployedModeVariable (VariableName, VendorGuid))) {
      Attributes = VARIABLE_ATTRIBUTE_BS_RT;
    }
    if (!DoesRunRuntimeCodeInSmm ()) {
      Status = SetVariableThroughSmi (
                 VariableName,
                 VendorGuid,
                 Attributes,
                 DataSize,
                 Data
                 );
    } else {
      Status = mVariableModuleGlobal->SmmSetVariable (
                                        VariableName,
                                        VendorGuid,
                                        Attributes,
                                        DataSize,
                                        Data
                                        );

    }
    goto Done;
  }

  SyncAuthData (Global);
  //
  // Special Handling for MOR Lock variable.
  //
  Status = SetVariableCheckHandlerMor (VariableName, VendorGuid, Attributes, DataSize, Data);
  if (Status == EFI_ALREADY_STARTED) {
    //
    // EFI_ALREADY_STARTED means the SetVariable() action is handled inside of SetVariableCheckHandlerMor().
    // Variable driver can just return SUCCESS.
    //
    Status = EFI_SUCCESS;
    goto Done;
  }
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if (VariableAtRuntime () || Attributes != 0) {
    VariableCount = 0;
    Status = FindVariable (
               VariableName,
               VendorGuid,
               &Variable,
               &VariableCount,
               Global
               );
    if (!EFI_ERROR (Status)) {
      //
      // Add policy to make sure we cannot update any variable which attribute doesn't have
      // EFI_VARIABLE_RUNTIME_ACCESS at runtime.
      //
      if (VariableAtRuntime () && (Variable.CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS) == 0) {
        Status = EFI_WRITE_PROTECTED;
        goto Done;
      }
      //
      // If a preexisting variable is rewritten with different attributes, SetVariable() shall not
      // modify the variable and shall return EFI_INVALID_PARAMETER. Two exceptions to this rule:
      // 1. No access attributes specified
      // 2. The only attribute differing is EFI_VARIABLE_APPEND_WRITE
      //
      if (Attributes != 0 && (Attributes & (~EFI_VARIABLE_APPEND_WRITE)) != Variable.CurrPtr->Attributes) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  }

  if (Attributes == 0) {
    Status = UpdateNoAccessAttribute (
               VariableName,
               VendorGuid,
               DataSize,
               Data,
               &Attributes
               );
    if (EFI_ERROR (Status)) {
      goto Done;
    }
  }

  VariableCount = 0;
  Status = FindVariableByLifetime (VariableName, VendorGuid, &Variable, &VariableCount, Global);
  if (Status == EFI_INVALID_PARAMETER) {
    goto Done;
  }

  if ((VariableCount > 1) && (Attributes & EFI_VARIABLE_NON_VOLATILE)) {
    DeleteAllOldVariable (VariableName, VendorGuid, Global);
  }

  if (VariableAtRuntime () && Attributes && (!(Attributes & EFI_VARIABLE_RUNTIME_ACCESS) || !(Attributes & EFI_VARIABLE_NON_VOLATILE))) {
    //
    // Runtime but Attribute is not Runtime or is volatile
    //
    Status = EFI_INVALID_PARAMETER;
  } else if ((Attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) || (Attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) ||
             IsSecureDatabaseVariable (VariableName, VendorGuid)) {
    Status = SetAuthVariable (
               VariableName,
               VendorGuid,
               Attributes,
               DataSize,
               Data,
               Global
               );
  } else {
    //
    // Only can update variable directly when variable doesn't exist or exist variable doens't authenticated variable
    //
    Status = EFI_WRITE_PROTECTED;
    if ((Variable.CurrPtr == NULL) || (Variable.CurrPtr != NULL && (Variable.CurrPtr->Attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) == 0 &&
        (Variable.CurrPtr->Attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) == 0)) {
      Status = UpdateVariable (
                 VariableName,
                 VendorGuid,
                 Data,
                 DataSize,
                 Attributes,
                 0,
                 0,
                 &Variable,
                 0,
                 Global
                 );
      if (FeaturePcdGet (PcdH2OCustomizedSecureBootSupported) && Status == EFI_SUCCESS) {
        UpdateStateForModeVariablesChanged (VariableName, VendorGuid);
      }
    }
  }
Done:
  Global->ReentrantState--;
//[-start-170406-IB07250279-add]//
  ReleaseLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
//[-end-170406-IB07250279-add]//
  return Status;
}

//[-start-170418-IB15590029-add]//
EFI_STATUS
QueryVariableInfoThroughSmi (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  )
{
  EFI_STATUS                  Status;
  UINTN                       SmmBufSize;

  SmmBufSize = sizeof (SMM_VAR_BUFFER) + MAX_VARIABLE_NAME_SIZE + MAX_VARIABLE_SIZE;
  SetMem (mSmmVarBuf, SmmBufSize, 0x0);
  mSmmVarBuf->Signature = SMM_VARIABLE_SIGNATURE;
  mSmmVarBuf->Attributes = Attributes;
  mSmmVarBuf->MaximumVariableStorageSize = 0;
  mSmmVarBuf->RemainingVariableStorageSize = 0;
  mSmmVarBuf->MaximumVariableSize = 0;
  SmmSecureBootCall ((UINT8 *) mSmmPhyVarBuf, SmmBufSize, SMM_QUERY_VARIABLE_INFO_FUN_NUM, SW_SMI_PORT);
  Status = mSmmVarBuf->Status;
  if (!EFI_ERROR (Status)) {
    *MaximumVariableStorageSize = mSmmVarBuf->MaximumVariableStorageSize;
    *RemainingVariableStorageSize = mSmmVarBuf->RemainingVariableStorageSize;
    *MaximumVariableSize = mSmmVarBuf->MaximumVariableSize;
  } 

  return Status;
}
//[-end-170418-IB15590029-add]//

/**
  This code returns information about the EFI variables.

  @param Attributes                     Attributes bitmask to specify the type of variables
                                        on which to return information.
  @param MaximumVariableStorageSize     Pointer to the maximum size of the storage space available
                                        for the EFI variables associated with the attributes specified.
  @param RemainingVariableStorageSize   Pointer to the remaining size of the storage space available
                                        for EFI variables associated with the attributes specified.
  @param MaximumVariableSize            Pointer to the maximum size of an individual EFI variables


  @retval EFI_INVALID_PARAMETER         An invalid combination of attribute bits was supplied.
  @retval EFI_SUCCESS                   Query successfully.
  @retval EFI_UNSUPPORTED               The attribute is not supported on this platform.

**/
EFI_STATUS
EFIAPI
VariableServicesQueryVariableInfo (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  )
{
  VARIABLE_HEADER             *Variable;
  VARIABLE_HEADER             *NextVariable;
  VARIABLE_HEADER             *VariableEndPointer;
  UINT64                      VariableSize;
  VARIABLE_STORE_HEADER       *VariableStoreHeader;
  VARIABLE_GLOBAL             *Global;
  UINTN                       VariableStoreSize;
  UINT32                      NvVariableStoreSize;
  BOOLEAN                     InteruptEnabled;
  VARIABLE_STORAGE_PROTOCOL   *VariableStorageProtocol;
  UINTN                       Index;
  UINT32                      VspCommonVariablesTotalSize;
  UINT32                      VspHwErrVariablesTotalSize;
  EFI_STATUS                  Status;

  if(MaximumVariableStorageSize == NULL || RemainingVariableStorageSize == NULL || MaximumVariableSize == NULL || Attributes == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if((Attributes & (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_HARDWARE_ERROR_RECORD)) == 0) {
    //
    // Make sure the Attributes combination is supported by the platform.
    //
    return EFI_UNSUPPORTED;
  } else if ((Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == EFI_VARIABLE_RUNTIME_ACCESS) {
    //
    // Make sure if runtime bit is set, boot service bit is set also.
    //
    return EFI_INVALID_PARAMETER;
  } else if (VariableAtRuntime () && !(Attributes & EFI_VARIABLE_RUNTIME_ACCESS)) {
    //
    // Make sure RT Attribute is set if we are in Runtime phase.
    //
    return EFI_INVALID_PARAMETER;
  }
  //
  // In current code doesn't support EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS.
  //
  else if ((Attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) != 0) {
    return EFI_INVALID_PARAMETER;
  }

//[-start-170302-IB15590023-add]//
  //
  // For Ubuntu 16.10 installing a blank HDD.
  //
  if (VariableAtRuntime () && mSmst == NULL) {
    return QueryVariableInfoThroughSmi (Attributes, MaximumVariableStorageSize, RemainingVariableStorageSize, MaximumVariableSize);
  }
//[-end-170302-IB15590023-add]//

  InteruptEnabled = GetInterruptState ();
  if (VariableAtRuntime () && InteruptEnabled) {
    DisableInterrupts ();
  }
  if (VariableAtRuntime ()) {
    WriteBackInvalidateDataCache ();
  }
//[-start-170406-IB07250279-add]//
  AcquireLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
//[-end-170406-IB07250279-add]//
  Global   = &mVariableModuleGlobal->VariableBase;
  Variable = NULL;
  VariableEndPointer = NULL;
  VariableStoreSize = 0;
  if((Attributes & EFI_VARIABLE_NON_VOLATILE) == 0) {
    //
    // Query is Volatile related.
    //
    VariableStoreHeader = (VARIABLE_STORE_HEADER *) ((UINTN) Global->VolatileVariableBase);
    VariableEndPointer = GetEndPointer (VariableStoreHeader);
    VariableStoreSize = GetVariableStoreSize (VariableStoreHeader);
    //
    // Point to the starting address of the variables.
    //
    Variable = GetStartPointer (VariableStoreHeader);
  }
//[-start-170330-IB07400855-add]//
  else {
    VariableStoreSize = GetNonVolatileVariableStoreSize ();
  }
//[-end-170330-IB07400855-add]//
  //
  // Now let's fill *MaximumVariableStorageSize *RemainingVariableStorageSize
  // with the storage size (excluding the storage header size).
  //
  *MaximumVariableStorageSize   = VariableStoreSize - GetVariableStoreHeaderSize ();
  *RemainingVariableStorageSize = VariableStoreSize - GetVariableStoreHeaderSize ();

  //
  // Let *MaximumVariableSize be MAX_VARIABLE_SIZE with the exception of the variable header size.
  //
  *MaximumVariableSize = MAX_VARIABLE_SIZE - sizeof (VARIABLE_HEADER);
  //
  // Harware error record variable needs larger size.
  //
  if ((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
    *MaximumVariableSize = MAX_HARDWARE_ERROR_VARIABLE_SIZE - sizeof (VARIABLE_HEADER);
  }



  if ((Attributes & EFI_VARIABLE_NON_VOLATILE) == 0) {
    //
    // For Volatile related, walk through the variable store.
    //
    while (IsValidVariableHeaderInVarRegion (Variable, VariableEndPointer)) {
      NextVariable = GetNextVariablePtr (Variable);
      VariableSize = (UINT64) (UINTN) NextVariable - (UINT64) (UINTN) Variable;

      if (VariableAtRuntime ()) {
        //
        // we don't take the state of the variables in mind
        // when calculating RemainingVariableStorageSize,
        // since the space occupied by variables not marked with
        // VAR_ADDED is not allowed to be reclaimed in Runtime.
        //
        *RemainingVariableStorageSize -= VariableSize;
      } else {
        //
        // Only care about Variables with State VAR_ADDED,because
        // the space not marked as VAR_ADDED is reclaimable now.
        //
        if ((Variable->State == VAR_ADDED) ||
            (Variable->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION))) {
          *RemainingVariableStorageSize -= VariableSize;
        }
      }

      //
      // Go to the next one
      //
      Variable = NextVariable;
    }
  } else {
    //
    // For Non Volatile related, call GetStorageUsage() on the VARIABLE_STORAGE_PROTOCOLs
    //
    for ( Index = 0;
          Index < mVariableModuleGlobal->VariableBase.VariableStoresCount;
          Index++) {
      VariableStorageProtocol = mVariableModuleGlobal->VariableBase.VariableStores[Index];
      Status = VariableStorageProtocol->GetStorageUsage (
                                          VariableStorageProtocol,
                                          VariableAtRuntime (),
                                          &NvVariableStoreSize,
                                          &VspCommonVariablesTotalSize,
                                          &VspHwErrVariablesTotalSize
                                          );
      ASSERT_EFI_ERROR (Status);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      *RemainingVariableStorageSize  -= VspCommonVariablesTotalSize;
    }
  }

  if (*RemainingVariableStorageSize < sizeof (VARIABLE_HEADER)) {
    *MaximumVariableSize = 0;
  } else if ((*RemainingVariableStorageSize - sizeof (VARIABLE_HEADER)) < *MaximumVariableSize) {
    *MaximumVariableSize = *RemainingVariableStorageSize - sizeof (VARIABLE_HEADER);
  }
  if (VariableAtRuntime () && InteruptEnabled) {
    EnableInterrupts ();
  }
//[-start-170406-IB07250279-add]//
  ReleaseLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
//[-end-170406-IB07250279-add]//
  return EFI_SUCCESS;
}


/**
  Initialize authenticated services.

  @param Event    Event whose notification function is being invoked.
  @param Context  Pointer to the notification function's context.

**/
VOID
EFIAPI
CryptoCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS                     Status;
  STATIC BOOLEAN                 Initialized = FALSE;

  if (!Initialized) {
    Status = AutenticatedVariableServiceInitialize (&mVariableModuleGlobal->VariableBase);
    Initialized = TRUE;
    ASSERT_EFI_ERROR (Status);
  }

  if (Event != NULL) {
    if (mSmst == NULL) {
      gBS->CloseEvent (Event);
    } else if (mSmmVariableGlobal->SmmRT != NULL) {
      mSmmVariableGlobal->SmmRT->DisableProtocolNotify (Event);
    }
  }
  return;

}

/**
  The notification of gEfiCryptoServiceProtocolGuid protocol is installed

  @param  Protocol              Points to the protocol's unique identifier.
  @param  Interface             Points to the interface instance.
  @param  Handle                The handle on which the interface was installed.

**/
EFI_STATUS
EFIAPI
CryptoServicesInstalled (
  IN     CONST EFI_GUID                *Protocol,
  IN     VOID                          *Interface,
  IN     EFI_HANDLE                    Handle
  )
{
  CryptoCallback (NULL, NULL);
  return EFI_SUCCESS;
}

/**
  Check is whether need retore factory default.

  @return TRUE   System is in restore factory default mode.
  @return FALSE  System isn't in restore factory default mode.

--*/
BOOLEAN
NeedRestoreFactoryDefault (
  VOID
  )
{
  UINT8                          RestoreFactoryDefault;
  EFI_STATUS                     Status;
  BOOLEAN                        NeedRestoreDefault;
  UINT8                          *Buffer;
  UINTN                          StrSize;
  VARIABLE_POINTER_TRACK         Variable;
  UINTN                          VariableCount;

  EFI_PEI_HOB_POINTERS          GuidHob;
  BOOLEAN                       *BiosFirstBootFlagBuffer = NULL;
  BIOS_FIRST_BOOT_HOB           *BiosFirstBootHob = NULL;

  NeedRestoreDefault = FALSE;

  Status = FindVariableByLifetime (
             EFI_RESTORE_FACOTRY_DEFAULT_NAME,
             &gEfiGenericVariableGuid,
             &Variable,
             &VariableCount,
             &mVariableModuleGlobal->VariableBase
             );
  if (!EFI_ERROR (Status) && Variable.CurrPtr->DataSize == sizeof (UINT8)) {
    StrSize = Variable.CurrPtr->NameSize + GET_PAD_SIZE (Variable.CurrPtr->NameSize);
    Buffer = (UINT8 *) Variable.CurrPtr;
    Buffer = Buffer + StrSize + sizeof (VARIABLE_HEADER);
    RestoreFactoryDefault = *Buffer;
    if (RestoreFactoryDefault == 1) {
      NeedRestoreDefault = TRUE;
    }
  }

  if (PcdGetBool (PcdBuildActivatesSecureBoot)) {
    GuidHob.Raw = GetHobList ();
 
    if (GuidHob.Raw != NULL) {
      if ((GuidHob.Raw = GetNextGuidHob (&gBiosFirstBootHobGuid, GuidHob.Raw)) != NULL) {
        BiosFirstBootHob = GET_GUID_HOB_DATA (GuidHob.Guid);
//[-start-171123-IB07400931-modify]//
        BiosFirstBootFlagBuffer = (BOOLEAN *) (UINTN)BiosFirstBootHob->BufferAddress;
//[-end-171123-IB07400931-modify]//
        if (*BiosFirstBootFlagBuffer == TRUE) {
          NeedRestoreDefault = TRUE;
        }
      }
    }
  }

  return NeedRestoreDefault;
}

/**
  Resotre NV_VARIABLE_STORE to facotry default.

  @return EFI_INVALID_PARAMETER Any input parameter is invalid.
  @return EFI_NOT_FOUND         gEfiVariableDefaultUpdateProtocolGuid doesn't exit.
  @return EFI_UNSUPPORTED       Factory default copy isn't initialized.
  @return EFI_SUCCESS           Restore NV_VARIABLE_STORE to default successful.

--*/
EFI_STATUS
RestoreFactoryDefault (
  VOID
  )
{
  VARIALBE_NAME_GUID             SecureBootVariables[]        = {
                                   {EFI_PLATFORM_KEY_NAME,                &gEfiGlobalVariableGuid},
                                   {EFI_KEY_EXCHANGE_KEY_NAME,            &gEfiGlobalVariableGuid},
                                   {EFI_IMAGE_SECURITY_DATABASE,          &gEfiImageSecurityDatabaseGuid},
                                   {EFI_IMAGE_SECURITY_DATABASE1,         &gEfiImageSecurityDatabaseGuid},
                                   {EFI_IMAGE_SECURITY_DATABASE2,         &gEfiImageSecurityDatabaseGuid}
                                 };
  VARIALBE_NAME_GUID             SecureBootDefaultVariables[] = {
                                   {EFI_PLATFORM_KEY_DEFAULT_NAME,        &gEfiGlobalVariableGuid},
                                   {EFI_KEY_EXCHANGE_KEY_DEFAULT_NAME,    &gEfiGlobalVariableGuid},
                                   {EFI_IMAGE_SECURITY_DEFAULT_DATABASE,  &gEfiGlobalVariableGuid},
                                   {EFI_IMAGE_SECURITY_DEFAULT_DATABASE1, &gEfiGlobalVariableGuid},
                                   {EFI_IMAGE_SECURITY_DEFAULT_DATABASE2, &gEfiGlobalVariableGuid}
                                 };
  UINTN                          Index;
  UINTN                          MaxVariableNum;
  VARIABLE_HEADER                *VariableHeader;
  EFI_STATUS                     Status;
  VARIABLE_STORAGE_PROTOCOL      *VariableStorageProtocol;

  MaxVariableNum = sizeof (SecureBootVariables) / sizeof (VARIALBE_NAME_GUID);

  for (Index = 0; Index < MaxVariableNum; Index++) {
    Status = GetVariableStorageProtocol (
              SecureBootVariables[Index].VariableName,
              SecureBootVariables[Index].VendorGuid,
              &VariableStorageProtocol
              );
    if (!EFI_ERROR (Status)) {
      VariableHeader = FindSecureDatabaseDefaultVariableHeader (
                         SecureBootDefaultVariables[Index].VariableName,
                         SecureBootDefaultVariables[Index].VendorGuid
                         );
      if (VariableHeader == NULL) {
        Status = VariableStorageProtocol->SetVariable (
                                            VariableStorageProtocol,
                                            SecureBootVariables[Index].VariableName,
                                            SecureBootVariables[Index].VendorGuid,
                                            VARIABLE_ATTRIBUTE_NV_BS_RT_AT,
                                            0,
                                            NULL,
                                            VariableAtRuntime (),
                                            0,
                                            0,
                                            NULL
                                            );
      } else {
        Status = VariableStorageProtocol->SetVariable (
                                            VariableStorageProtocol,
                                            SecureBootVariables[Index].VariableName,
                                            SecureBootVariables[Index].VendorGuid,
                                            VARIABLE_ATTRIBUTE_NV_BS_RT_AT,
                                            VariableHeader->DataSize,
                                            GetVariableDataPtr (VariableHeader),
                                            VariableAtRuntime (),
                                            VariableHeader->PubKeyIndex,
                                            ReadUnaligned64 (&(VariableHeader->MonotonicCount)),
                                            &VariableHeader->TimeStamp
                                            );
      }
    }
  }

  //
  // Clear "RestoreFactoryDefault" to 0 to prevent from system always restoring factory default during POST.
  //
  UpdateRestoreFactoryDefaultVariable (0);

  gST->RuntimeServices->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);

  return EFI_SUCCESS;
}

/**
  Callback routine to enable hook BootOrder variable mechanic.

  @param Event    Event whose notification function is being invoked.
  @param Context  Pointer to the notification function's context.
**/
VOID
EFIAPI
BootOrderHookEnableCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  mVariableModuleGlobal->BootOrderVariableHook = TRUE;
  return;
}

/**
  Callback routine to disable hook BootOrder variable mechanic.

  @param Event    Event whose notification function is being invoked.
  @param Context  Pointer to the notification function's context.
**/
VOID
EFIAPI
BootOrderHookDisableCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  mVariableModuleGlobal->BootOrderVariableHook = FALSE;
  return;
}

/**
  Initialize SMM RAM information.

  @retval EFI_SUCCESS           Initialize SMM RAM information successfully.
  @retval EFI_OUT_OF_RESOURCES  Allocate memory to save SMM RAM informaiton failed.
  @retval Other                 Any error occurred during initialize SMM RAM information.
**/
EFI_STATUS
EFIAPI
SmmRamInfoInitialize (
  VOID
  )
{

  EFI_STATUS                  Status;
  UINTN                       Size;
  EFI_SMM_ACCESS2_PROTOCOL   *SmmAccess;

  Status = gBS->LocateProtocol (&gEfiSmmAccess2ProtocolGuid, NULL, (VOID **)&SmmAccess);
  ASSERT (Status == EFI_SUCCESS);
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  Size = 0;
  Status = SmmAccess->GetCapabilities (SmmAccess, &Size, NULL);
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return Status;
  }
  mSmramRanges = VariableAllocateZeroBuffer (Size, TRUE);
  ASSERT (mSmramRanges != NULL);
  if (mSmramRanges == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = SmmAccess->GetCapabilities (SmmAccess, &Size, mSmramRanges);
  ASSERT_EFI_ERROR (Status);
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  mSmramRangeCount = Size / sizeof (EFI_SMRAM_DESCRIPTOR);
  return EFI_SUCCESS;
}

/**
  Initialize SMM mode NV data

  @param  SmmBase        Pointer to EFI_SMM_BASE2_PROTOCOL

  @retval EFI_SUCCESS    Initialize SMM mode NV data successful.
  @retval Other          Any error occured during initialize SMM NV data.

**/
EFI_STATUS
EFIAPI
SmmNvsInitialize (
  IN  EFI_SMM_BASE2_PROTOCOL                 *SmmBase
  )
{
  EFI_STATUS                      Status;

  Status = SmmRamInfoInitialize ();
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  mSmmVariableGlobal->Signature                        = SMM_VARIABLE_SIGNATURE;
  Status = gBS->LocateProtocol (
                  &gEfiNonVolatileVariableProtocolGuid,
                  NULL,
                  (VOID **)&mSmmVariableGlobal->ProtectedModeVariableModuleGlobal
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->SmmCodeReady = TRUE;
  //
  // Since we use SMI to trigger secure boot callback functions, we needn't use
  // protected mode callback functions.
  //
  mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->SecureBootCallbackEnabled = FALSE;

  //
  // We want to support volatile variable both in SMM mode and protected mode, so we copy volatile base address
  // from protected mode. This address will be physical address and will not be converted to virutal address
  // in virtual address change event.
  //
  CopyMem (
    &mVariableModuleGlobal->VariableBase.VolatileVariableBase,
    &mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->VariableBase.VolatileVariableBase,
    sizeof (EFI_PHYSICAL_ADDRESS)
    );
  //
  // NOTE: We don't copy VolatileLastOffset to SMM mode. Instead, we use
  // mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->VolatileLastVariableOffset and then SMM mode and
  // protected mode can use the same address to save volatile last variable offset.
  //
  CopyMem (
    &mVariableModuleGlobal->NonVolatileLastVariableOffset,
    &mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->NonVolatileLastVariableOffset,
    sizeof (EFI_PHYSICAL_ADDRESS)
    );

  CopyMem (
    &mVariableModuleGlobal->NonVolatileVariableCacheSize,
    &mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->NonVolatileVariableCacheSize,
    sizeof (mVariableModuleGlobal->NonVolatileVariableCacheSize)
    );

  CopyMem (
    &mVariableModuleGlobal->NonVolatileVariableCache,
    &mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->NonVolatileVariableCache,
    sizeof (mVariableModuleGlobal->NonVolatileVariableCache)
    );

  CopyMem (
    &mVariableModuleGlobal->FactoryDefaultBase,
    &mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->FactoryDefaultBase,
    sizeof (mVariableModuleGlobal->FactoryDefaultBase)
    );

  CopyMem (
    &mVariableModuleGlobal->FactoryDefaultSize,
    &mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->FactoryDefaultSize,
    sizeof (mVariableModuleGlobal->FactoryDefaultSize)
    );

  CopyMem (
    &mVariableModuleGlobal->GlobalVariableList,
    &mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->GlobalVariableList,
    sizeof (mVariableModuleGlobal->GlobalVariableList)
    );

  return Status;
}

/**
  Make sure the remaing non-volatile variable space is larger than
  VARIABLE_RECLAIM_THRESHOLD before boot to OS

  @param Event    - Event whose notification function is being invoked.
  @param Context  - Pointer to the notification function's context.

**/
VOID
EFIAPI
ReclaimForOS (
  VOID
  )
{
  EFI_STATUS                      Status;
  STATIC BOOLEAN                  Reclaimed;

  //
  // This function will be called only once at EndOfDxe or ReadyToBoot event.
  //
  if (Reclaimed) {
    return;
  }
  Reclaimed = TRUE;
  mVariableModuleGlobal->NonVolatileLastVariableOffset = GetCurrentNonVolatileOffset ();
  //
  // Check if the free area is blow a threshold
  //
  if ((GetNonVolatileVariableStoreSize () - mVariableModuleGlobal->NonVolatileLastVariableOffset) < VARIABLE_RECLAIM_THRESHOLD ||
      DoesNeedDoReclaimVariableExist ()) {
      Status = ReclaimNonVolatileVariable (
                 NULL,
                 0,
                 (EFI_PHYSICAL_ADDRESS) (UINTN) mVariableModuleGlobal->NonVolatileVariableCache,
                 &mVariableModuleGlobal->NonVolatileLastVariableOffset
                 );
    ASSERT(!EFI_ERROR(Status));
    if (!EFI_ERROR (Status) && DoesNeedDoReclaimVariableExist ()) {
      DeleteNeedDoReclaimVariable ();
    }
  }

}

/**
  Callback function for ready to boot event.

  @param Event    - Event whose notification function is being invoked.
  @param Context  - Pointer to the notification function's context.

**/
VOID
EFIAPI
ReadyToBootCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  if (!mReadyToBootEventSignaled) {
    if (!mVariableModuleGlobal->EndOfDxe) {
      CreateVariableLockListThroughSmi ();
      CreateVariableCheckListThroughSmi ();
    }
    mVariableModuleGlobal->EndOfDxe = TRUE;
    mReadyToBootEventSignaled = TRUE;
    InitializeVariableQuota ();
    ReclaimForOS ();
  }
}

/**
  Notification function of EFI_END_OF_DXE_EVENT_GROUP_GUID event group.

  This is a notification function registered on EFI_END_OF_DXE_EVENT_GROUP_GUID event group.

  @param[in]  Event        Event whose notification function is being invoked.
  @param[in]  Context      Pointer to the notification function's context.
**/
VOID
EFIAPI
OnEndOfDxe (
  IN EFI_EVENT                               Event,
  IN VOID                                    *Context
  )
{
  if (!mVariableModuleGlobal->EndOfDxe) {
    CreateVariableLockListThroughSmi ();
    CreateVariableCheckListThroughSmi ();
  }
  mVariableModuleGlobal->EndOfDxe = TRUE;
  if (PcdGetBool (PcdReclaimVariableSpaceAtEndOfDxe)) {
    ReclaimForOS ();
  }
  InitializeVariableQuota ();
}

/**
  Callback function for legacy boot event.

  @param Event   Event whose notification function is being invoked.
  @param Context Pointer to the notification function's context.

**/
VOID
EFIAPI
LegacyBootCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  UINT32       Signature;

  if (mVariableModuleGlobal->SmmCodeReady && mSmst == NULL) {
    Signature = SMM_LEGACY_BOOT_SIGNATURE;
    CopyMem (mSmmPhyVarBuf, &Signature, sizeof (Signature));
    SmmSecureBootCall ((UINT8 *) mSmmPhyVarBuf, sizeof (Signature), LEGACY_BOOT_SMI_FUN_NUM, SW_SMI_PORT);
  }
  gBS->CloseEvent (Event);
  return;
}

/**
  This function uses to initialize volatile variable store header

  @param  VolatileVariableStore   pointer to volatile variable store which want to initialize

  @retval EFI_SUCCESS             Initialize voaltile variable store successful.
  @retval EFI_INVALID_PARAMETER   VolatileVariableStore pointer to NULL.

**/
STATIC
EFI_STATUS
InitializeVolatileVariableStoreHeader (
  IN OUT VARIABLE_STORE_HEADER                 *VolatileVariableStore
  )
{
  ECP_VARIABLE_STORE_HEADER     *EcpVarStoreHeader;

  if (VolatileVariableStore == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (PcdGetBool (PcdUseEcpVariableStoreHeader)) {
    EcpVarStoreHeader = (ECP_VARIABLE_STORE_HEADER *) VolatileVariableStore;
    EcpVarStoreHeader->Signature                  = ECP_VARIABLE_STORE_SIGNATURE;
    EcpVarStoreHeader->Size                       = VARIABLE_STORE_SIZE;
    EcpVarStoreHeader->Format                     = VARIABLE_STORE_FORMATTED;
    EcpVarStoreHeader->State                      = VARIABLE_STORE_HEALTHY;
    EcpVarStoreHeader->Reserved                   = 0;
    EcpVarStoreHeader->Reserved1                  = 0;
  } else {
    CopyGuid (&VolatileVariableStore->Signature, &gEfiAuthenticatedVariableGuid);
    VolatileVariableStore->Size                   = VARIABLE_STORE_SIZE;
    VolatileVariableStore->Format                 = VARIABLE_STORE_FORMATTED;
    VolatileVariableStore->State                  = VARIABLE_STORE_HEALTHY;
    VolatileVariableStore->Reserved               = 0;
    VolatileVariableStore->Reserved1              = 0;
  }
  mVariableModuleGlobal->VariableBase.VolatileVariableBase = (EFI_PHYSICAL_ADDRESS) (UINTN) VolatileVariableStore;
  mVariableModuleGlobal->VolatileLastVariableOffset = GetVariableStoreHeaderSize ();

  return EFI_SUCCESS;
}

/**
  This function uses to disable all of secure boot relative callbacks

  @param Event      Event whose notification function is being invoked.
  @param Context    Pointer to the notification function's context.

**/
VOID
EFIAPI
DisableSecureBootCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  STATIC  BOOLEAN     SmiDisabled = FALSE;

  if (!SmiDisabled) {
    if (mVariableModuleGlobal->SmmCodeReady) {
      SmmSecureBootCall (NULL, 0, DISABLE_SECURE_BOOT_SMI_FUN_NUM, SW_SMI_PORT);
      SmiDisabled = TRUE;
    }
  }
  mVariableModuleGlobal->SecureBootCallbackEnabled = FALSE;
  gBS->CloseEvent (Event);
  return;
}

/**
  Register call back function (on ReadytoBoot event and an platform specific event) to disable
  all of secure boot functions.

  @return EFI_SUCCESS   Register callback function successful.

--*/
EFI_STATUS
RegisterEventToDisableSecureBoot (
  VOID
  )
{
  EFI_STATUS       Status;
  EFI_EVENT        Event;
  VOID             *Registration;


  Event = VariableCreateProtocolNotifyEvent (
            &gPlatformDisableSecureBootGuid,
            TPL_CALLBACK,
            DisableSecureBootCallback,
            NULL,
            &Registration
            );

  Status = EfiCreateEventReadyToBootEx (
             TPL_NOTIFY,
             DisableSecureBootCallback,
             NULL,
             &Event
             );

  return Status;

}

/**
  Build the variable working region record for use of other drivers.

  @param[in] Address
  @param[in] Length

  @retval EFI_SUCCESS           Build variable working region record successful.
  @retval EFI_OUT_OF_RESOURCES  Allocate memory failed.
  @retval Other                 Install configuration table failed.
**/
EFI_STATUS
BuildVariableWorkingRegionRecord (
  IN EFI_PHYSICAL_ADDRESS              Address,
  IN EFI_PHYSICAL_ADDRESS              Length
  )
{
  VARIALBE_WORKING_REGION_INFO        *VariableWorkingRegionInfo;
  EFI_STATUS                          Status;

  VariableWorkingRegionInfo = VariableAllocateZeroBuffer (sizeof(VARIALBE_WORKING_REGION_INFO), TRUE);
  if (VariableWorkingRegionInfo == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  VariableWorkingRegionInfo->VariableWorkingRegionStart  = Address;
  VariableWorkingRegionInfo->VariableWorkingRegionLength = Length;
  Status = gBS->InstallConfigurationTable (&mVariableWorkingRegionHobGuid, VariableWorkingRegionInfo);
  return Status;
}

/**
  BdsEntry hook function. This function uses to set Insyde variable state to locked
  and then enter original BdsEntry ().

  @param[in] This      The EFI_BDS_ARCH_PROTOCOL instance.
**/
STATIC
VOID
EFIAPI
BdsEntry (
  IN EFI_BDS_ARCH_PROTOCOL  *This
  )
{
  mVariableModuleGlobal->InsydeSecureVariableLocked = TRUE;
  mOriginalBdsEntry (This);
}

/**
  This function uses to hook original BdsEntry ().

  @param[in] Event      Event whose notification function is being invoked.
  @param[in] Event      Pointer to the notification function's context.
**/
STATIC
VOID
EFIAPI
BdsCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS                        Status;
  EFI_BDS_ARCH_PROTOCOL             *Bds;

  Status = gBS->LocateProtocol (
                  &gEfiBdsArchProtocolGuid,
                  NULL,
                  (VOID **) &Bds
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return;
  }
  mOriginalBdsEntry = Bds->Entry;
  Bds->Entry = BdsEntry;
  return;
}

/**
  Initializes variable store area for non-volatile and volatile variable.

  @retval EFI_SUCCESS           Hook BdsEntry () successful or register callback () successful.
  @retval EFI_OUT_OF_RESOURCES  Register callback() for gEfiBdsArchProtocolGuid failed.
**/
STATIC
EFI_STATUS
InitializeInsydeVariableLockedState (
  VOID
  )
{
  EFI_STATUS                        Status;
  EFI_BDS_ARCH_PROTOCOL             *Bds;
  EFI_EVENT                         Event;
  VOID                              *Registration;

  Status = gBS->LocateProtocol (
                  &gEfiBdsArchProtocolGuid,
                  NULL,
                  (VOID **) &Bds
                  );
  if (!EFI_ERROR (Status)) {
    mOriginalBdsEntry = Bds->Entry;
    Bds->Entry = BdsEntry;
  } else {
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    BdsCallback,
                    NULL,
                    &Event
                    );
    ASSERT_EFI_ERROR (Status);

    //
    // Register for protocol notifications on this event
    //
    Status = gBS->RegisterProtocolNotify (
                    &gEfiBdsArchProtocolGuid,
                    Event,
                    &Registration
                    );
    ASSERT_EFI_ERROR (Status);
  }
  return Status;
}

/**
  Add the EFI_MEMORY_RUNTIME memory attribute to input memory region.

  @param[in] BaseAddress    Input memory base address.
  @param[in] Length         Input memory size.

  @retval    EFI_SUCCESS    Add EFI_MEMORY_RUNTIME memory attribute successfully.
  @retval    other          Any other occurred while adding EFI_MEMORY_RUNTIME memory attribute.
**/
STATIC
EFI_STATUS
SetRuntimeMemoryAttribute (
  IN EFI_PHYSICAL_ADDRESS                BaseAddress,
  IN UINT64                              Length
  )
{
  EFI_STATUS                            Status;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR       GcdDescriptor;

  //
  // Mark the Flash part memory space as EFI_MEMORY_RUNTIME
  //
  BaseAddress = BaseAddress & (~EFI_PAGE_MASK);
  Length = (Length + EFI_PAGE_SIZE - 1) & (~EFI_PAGE_MASK);

  Status      = gDS->GetMemorySpaceDescriptor (BaseAddress, &GcdDescriptor);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gDS->SetMemorySpaceAttributes (
                  BaseAddress,
                  Length,
                  GcdDescriptor.Attributes | EFI_MEMORY_RUNTIME
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Initialize the L"OsIndicationsSupported" variable.

  According to EFI SPEC, this variable is read-only variable, so we should
  initialize this variable by project setting in variable driver.

  @retval EFI_SUCCESS   Initialize L"OsIndicationsSupported" variable successfully.
  @retval Other         Any error occurred while initializing L"OsIndicationsSupported" variable.
**/
STATIC
EFI_STATUS
InitializeOsIndicationsSupportedVariable (
  VOID
  )
{
  UINTN                   VariableCount;
  VARIABLE_POINTER_TRACK  Variable;
  UINT64                  OsIndicationsSupported;
  EFI_STATUS              Status;

  Status = FindVariableByLifetime (
             OS_INDICATIONS_SUPPORTED_NAME,
             &gEfiGlobalVariableGuid,
             &Variable,
             &VariableCount,
             &mVariableModuleGlobal->VariableBase
             );

  OsIndicationsSupported = PcdGet64 (PcdOsIndicationsSupported);
  Status  = UpdateVariable (
              OS_INDICATIONS_SUPPORTED_NAME,
              &gEfiGlobalVariableGuid,
              &OsIndicationsSupported,
              sizeof(OsIndicationsSupported),
              EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
              0,
              0,
              &Variable,
              NULL,
              &mVariableModuleGlobal->VariableBase
              );

  return Status;
}


/**
  Initializes variable store area for non-volatile and volatile variable.

  @retval EFI_SUCCESS           Function successfully executed.
  @retval EFI_OUT_OF_RESOURCES  Fail to allocate enough memory resource.

**/
EFI_STATUS
VariableCommonInitialize (
  VOID
  )
{
  EFI_STATUS                            Status;
  VARIABLE_STORE_HEADER                 *VolatileVariableStore;
  EFI_HANDLE                            NewHandle;
  EFI_EVENT                             Event;
  UINT32                                 MaxUserNvVariableSpaceSize;

  if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
    return EFI_UNSUPPORTED;
  }

  VolatileVariableStore  = NULL;
  mVariableModuleGlobal = VariableAllocateZeroBuffer (sizeof (ESAL_VARIABLE_GLOBAL), TRUE);

  if (mVariableModuleGlobal == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Shutdown;
  }

  mVariableDataBuffer = VariableAllocateZeroBuffer (MAX_VARIABLE_SIZE, TRUE);
  if (mVariableDataBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Shutdown;
  }

  EfiInitializeLock (&mVariableModuleGlobal->VariableBase.VariableServicesLock, TPL_NOTIFY);

  mSmmVarBuf = VariableAllocateZeroBuffer (sizeof (SMM_VAR_BUFFER) + MAX_VARIABLE_NAME_SIZE + MAX_VARIABLE_SIZE, TRUE);
  if (mSmmVarBuf == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Shutdown;
  }
  mSmmPhyVarBuf = mSmmVarBuf;
  //
  // Reserved MAX_VARIABLE_SIZE runtime buffer for "Append" operation in virtual mode.
  //
  mStorageArea  = VariableAllocateZeroBuffer (APPEND_BUFF_SIZE, TRUE);
  if (mStorageArea == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Shutdown;
  }
  //
  // Reserved MAX_VARIABLE_SIZE runtime buffer for certificated database list (normal time based authenticated variable)
  // operation in virtual mode.
  //
  mCertDbList  = VariableAllocateZeroBuffer (MAX_VARIABLE_SIZE, TRUE);
  if (mCertDbList == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Shutdown;
  }

  //
  // Allocate memory for volatile variable store
  //
  POST_CODE (DXE_VARIABLE_INIT);

  mVariableModuleGlobal->VarCheckVariableList = &mVarCheckVariableList;

  mVariableModuleGlobal->FactoryDefaultBase = FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);
  mVariableModuleGlobal->FactoryDefaultSize = (UINTN) FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);
  Status = SetRuntimeMemoryAttribute (mVariableModuleGlobal->FactoryDefaultBase, (UINT64) mVariableModuleGlobal->FactoryDefaultSize);
  if (EFI_ERROR (Status)) {
    goto Shutdown;
  }

  InitializeInsydeVariableLockedState ();
  //
  // Since memory for non-volatile variable cache and volatile variable may be modified after start image, so combine these two
  // memory and save the start address and length in configuration table for other driver use.
  //
  VolatileVariableStore = VariableAllocateZeroBuffer (VARIABLE_STORE_SIZE + SCRATCH_SIZE + DEFAULT_NV_STORE_SIZE, TRUE);
  if (VolatileVariableStore == NULL) {
    POST_CODE (DXE_VARIABLE_INIT_FAIL);
    Status = EFI_OUT_OF_RESOURCES;
    goto Shutdown;
  }
  Status = BuildVariableWorkingRegionRecord (
           (EFI_PHYSICAL_ADDRESS) (UINTN) VolatileVariableStore,
           (EFI_PHYSICAL_ADDRESS) (UINTN) (VARIABLE_STORE_SIZE + SCRATCH_SIZE + DEFAULT_NV_STORE_SIZE)
           );
  if (EFI_ERROR (Status)) {
    goto Shutdown;
  }
  SetMem (VolatileVariableStore, VARIABLE_STORE_SIZE + SCRATCH_SIZE, 0xff);
  Status = InitializeVolatileVariableStoreHeader (VolatileVariableStore);
  ASSERT_EFI_ERROR (Status);


  //
  // Allocate EfiRuntimeServicesData memory type for variable cache, and then we can use this memory as variable cache in
  // POST time and runtime. system will pass this variable cache address to SMM mode driver. Finally, system can access this
  // variable cache in SMM mode and protected mode. But we must disable cache mechanism if LegacyBoot event is signaled, due to
  // EfiRuntimeServicesData memory is used by legacy OS.
  //
  mVariableModuleGlobal->NonVolatileVariableCache = (VARIABLE_STORE_HEADER *) ((UINT8 *) VolatileVariableStore + VARIABLE_STORE_SIZE + SCRATCH_SIZE);
  mVariableModuleGlobal->NonVolatileVariableCacheSize = DEFAULT_NV_STORE_SIZE;
  SetMem32 ((VOID *) mVariableModuleGlobal->NonVolatileVariableCache, DEFAULT_NV_STORE_SIZE, (UINT32) 0xFFFFFFFF);
  //
  // Initialize the VARIABLE_STORE_HEADER for the NV Storage Cache
  //

  CopyMem (
    &mVariableModuleGlobal->NonVolatileVariableCache->Signature,
    &gEfiAuthenticatedVariableGuid,
    sizeof (EFI_GUID)
    );
  mVariableModuleGlobal->NonVolatileVariableCache->Size    = DEFAULT_NV_STORE_SIZE;
  mVariableModuleGlobal->NonVolatileVariableCache->Format  = VARIABLE_STORE_FORMATTED;
  mVariableModuleGlobal->NonVolatileVariableCache->State   = VARIABLE_STORE_HEALTHY;

  MaxUserNvVariableSpaceSize = PcdGet32 (PcdMaxUserNvVariableSpaceSize);
  ASSERT (MaxUserNvVariableSpaceSize < (DEFAULT_NV_STORE_SIZE - sizeof (VARIABLE_STORE_HEADER)));
  mVariableModuleGlobal->CommonMaxUserVariableSpace = MaxUserNvVariableSpaceSize;

  mVariableModuleGlobal->GlobalVariableList = (VOID *) mGlobalVariableList;
  //
  // Install gEfiNonVolatileVariableProtocolGuid to let platform know support
  // non-volatile variable and also passes some information to SMM variable driver.
  //
  NewHandle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &NewHandle,
                  &gEfiNonVolatileVariableProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  mVariableModuleGlobal
                  );
  if (EFI_ERROR (Status)) {
    goto Shutdown;
  }
  RegisterEventToDisableSecureBoot ();
  Status = EfiCreateEventReadyToBootEx (
             TPL_NOTIFY,
             ReadyToBootCallback,
             NULL,
             &Event
             );
  if (EFI_ERROR (Status)) {
    goto Shutdown;
  }
  Status = EfiCreateEventLegacyBootEx (
             TPL_NOTIFY,
             LegacyBootCallback,
             NULL,
             &Event
             );
  if (EFI_ERROR (Status)) {
    goto Shutdown;
  }

  InitVariableStorages ();

  if (NeedRestoreFactoryDefault ()) {
    RestoreFactoryDefault ();
  }

  InitializeOsIndicationsSupportedVariable ();
  MorLockInit ();
  //
  // Register the event handling function to set the End Of DXE flag.
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  OnEndOfDxe,
                  NULL,
                  &gEfiEndOfDxeEventGroupGuid,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;

Shutdown:

  if (mVariableModuleGlobal != NULL) {
    if (mVariableModuleGlobal->NonVolatileVariableCache != NULL) {
      EFI_FREE_POOL (mVariableModuleGlobal->NonVolatileVariableCache);
    }
    EFI_FREE_POOL (mVariableModuleGlobal);
  }

  if (mSmmVarBuf != NULL) {
    EFI_FREE_POOL (mSmmVarBuf);
  }
  if (mStorageArea != NULL) {
    EFI_FREE_POOL (mStorageArea);
  }

  return Status;
}

/**
  According to system mode to allocate pool.
  Allocate BootServicesData pool in protect mode if Runtime is FALSE.
  Allocate EfiRuntimeServicesData pool in protect mode if Runtime is TRUE.
  Allocate memory from SMM ram if system in SMM mode.

  @param[in] Size    The size of buffer to allocate
  @param[in] Runtime Runtime Data or not.

  @return NULL       Buffer unsuccessfully allocated.
  @return Other      Buffer successfully allocated.
**/
VOID *
VariableAllocateZeroBuffer (
  IN UINTN     Size,
  IN BOOLEAN   Runtime
  )
{
  VOID           *Buffer;
  EFI_STATUS     Status;

  if (mSmst == NULL) {
    Status = gBS->AllocatePool (
                    Runtime ? EfiRuntimeServicesData : EfiBootServicesData,
                    Size,
                    &Buffer
                    );
  } else {
    Status = mSmst->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      Size,
                      &Buffer
                      );
  }

  if (!EFI_ERROR (Status)) {
    ZeroMem (Buffer, Size);
  } else {
    Buffer = NULL;
  }
  return Buffer;
}

/**
  This fucntion uses to read saved CPU double word register by CPU index

  @param  RegisterNum   Register number which want to get
  @param  CpuIndex      CPU index number to get register.
  @param  RegisterData  pointer to output register data

  @retval EFI_SUCCESS   Read double word register successful
  @return Other         Any error occured while disabling all secure boot SMI functions successful.

**/
EFI_STATUS
GetDwordRegisterByCpuIndex (
  IN  EFI_SMM_SAVE_STATE_REGISTER       RegisterNum,
  IN  UINTN                             CpuIndex,
  OUT UINT32                            *RegisterData
  )
{
  return mSmmVariableGlobal->SmmCpu->ReadSaveState (
                                       mSmmVariableGlobal->SmmCpu,
                                       sizeof (UINT32),
                                       RegisterNum,
                                       CpuIndex,
                                       RegisterData
                                       );
}

/**
  This fucntion uses to update current executing CPU to SMM_VARIABLE_INSTANCE

  @retval EFI_SUCCESS     Update current executing CPU successful.

  @retval EFI_NOT_FOUND   Cannot find current executing CPU.

**/
EFI_STATUS
UpdateCurrentExecutingCpu (
  VOID
  )
{
  UINTN           Index;
  UINT32          RegisterEax;
  UINT32          RegisterEdx;
  EFI_STATUS      Status;

  Status = EFI_NOT_FOUND;
  for (Index = 0; Index < mSmst->NumberOfCpus; Index++) {
    GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RAX, Index, &RegisterEax);
    GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RDX, Index, &RegisterEdx);
    if ((RegisterEax & 0xff) == SECURE_BOOT_SW_SMI && (RegisterEdx & 0xffff) == SW_SMI_PORT) {
      mSmmVariableGlobal->CurrentlyExecutingCpu = Index;
      Status = EFI_SUCCESS;
      break;
    }
  }
  return Status;

}


/**
  Callback function for service administer secure boot

  @param[in]     DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]     Context         Points to an optional handler context which was specified when the
                                 handler was registered.
  @param[in,out] CommBuffer      A pointer to a collection of data in memory that will
                                 be conveyed from a non-SMM environment into an SMM environment.
  @param[in,out] CommBufferSize  The size of the CommBuffer.

  @retval EFI_SUCCESS                         The interrupt was handled and quiesced. No other handlers
                                              should still be called.
  @retval EFI_WARN_INTERRUPT_SOURCE_QUIESCED  The interrupt has been quiesced but other handlers should
                                              still be called.
  @retval EFI_WARN_INTERRUPT_SOURCE_PENDING   The interrupt is still pending and other handlers should still
                                              be called.
  @retval EFI_INTERRUPT_PENDING               The interrupt could not be quiesced.

**/
EFI_STATUS
EFIAPI
SecureBootCallback (
  IN EFI_HANDLE     DispatchHandle,
  IN CONST VOID     *Context         OPTIONAL,
  IN OUT VOID       *CommBuffer      OPTIONAL,
  IN OUT UINTN      *CommBufferSize  OPTIONAL
  )
{
  UINTN                          Cmd;
  UINTN                          Index;
  EFI_STATUS                     Status;
  UINT32                         RegisterEax;

  //
  // Since the mSmst->CurrentlyExecutingCpu may be incorrect, so we
  // need update CurrentlyExecutingCpu in our private data and then
  // we can get register directly
  //
  Status = UpdateCurrentExecutingCpu ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RAX ,&RegisterEax);
  Status = EFI_UNSUPPORTED;
  Cmd = (UINTN)((RegisterEax >> 8) & 0xFF);
  for (Index = 0; mSecureBootFunctionsTable[Index].SmiSubFunction != NULL ; Index++) {
    if (Cmd == mSecureBootFunctionsTable[Index].FunNum) {
      if (mReadyToBootEventSignaled  && !mSecureBootFunctionsTable[Index].SupportedAfterReadyToBoot) {
        break;
      }
      Status = mSecureBootFunctionsTable[Index].SmiSubFunction ();
      break;
    }
  }

  //
  // Convert Status to al register
  //
  Status &= 0xff;
  RegisterEax = (UINT32)((RegisterEax & 0xffffff00) | Status);
  SetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RAX, RegisterEax);
  return EFI_SUCCESS;
}


/**
  Callback function only for variable service

  @param[in]     DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]     Context         Points to an optional handler context which was specified when the
                                 handler was registered.
  @param[in,out] CommBuffer      A pointer to a collection of data in memory that will
                                 be conveyed from a non-SMM environment into an SMM environment.
  @param[in,out] CommBufferSize  The size of the CommBuffer.

  @retval EFI_SUCCESS                         The interrupt was handled and quiesced. No other handlers
                                              should still be called.
  @retval EFI_WARN_INTERRUPT_SOURCE_QUIESCED  The interrupt has been quiesced but other handlers should
                                              still be called.
  @retval EFI_WARN_INTERRUPT_SOURCE_PENDING   The interrupt is still pending and other handlers should still
                                              be called.
  @retval EFI_INTERRUPT_PENDING               The interrupt could not be quiesced.

**/
EFI_STATUS
EFIAPI
NonSecureBootCallback (
  IN EFI_HANDLE     DispatchHandle,
  IN CONST VOID     *Context         OPTIONAL,
  IN OUT VOID       *CommBuffer      OPTIONAL,
  IN OUT UINTN      *CommBufferSize  OPTIONAL
  )
{
  UINTN                          Cmd;
  UINTN                          Index;
  EFI_STATUS                     Status;
  UINT32                         RegisterEax;

  //
  // Since the mSmst->CurrentlyExecutingCpu may be incorrect, so we
  // need update CurrentlyExecutingCpu in our private data and then
  // we can get register directly
  //
  Status = UpdateCurrentExecutingCpu ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RAX ,&RegisterEax);
  Status = EFI_UNSUPPORTED;
  Cmd = (UINTN)((RegisterEax >> 8) & 0xFF);
  for (Index = 0; mNonSecureBootFunctionsTable[Index].SmiSubFunction != NULL ; Index++) {
    if (Cmd == mNonSecureBootFunctionsTable[Index].FunNum) {
      Status = mNonSecureBootFunctionsTable[Index].SmiSubFunction ();
      break;
    }
  }

  //
  // Convert Status to al register
  //
  Status &= 0xff;
  RegisterEax = (UINT32)((RegisterEax & 0xffffff00) | Status);
  SetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RAX, RegisterEax);
  return EFI_SUCCESS;
}

/**
  This fucnitons uses to read saved CPU double word register

  @param  RegisterNum   Register number which want to get
   @param RegisterData  pointer to output register data

  @retval EFI_SUCCESS   Read double word register successful
  @return Other         Any error occured while disabling all secure boot SMI functions successful.

**/
EFI_STATUS
GetDwordRegister (
  IN EFI_SMM_SAVE_STATE_REGISTER       RegisterNum,
  OUT UINT32                            *RegisterData
  )
{
  return mSmmVariableGlobal->SmmCpu->ReadSaveState (
                                       mSmmVariableGlobal->SmmCpu,
                                       sizeof (UINT32),
                                       RegisterNum,
                                       mSmmVariableGlobal->CurrentlyExecutingCpu,
                                       RegisterData
                                       );

}

/**
  This fucnitons uses to write saved CPU double word register

  @param  RegisterNum   Register number which want to get
   @param RegisterData  pointer to output register data

  @retval EFI_SUCCESS   Read double word register successful
  @return Other         Any error occured while disabling all secure boot SMI functions successful.

**/
EFI_STATUS
SetDwordRegister (
  IN EFI_SMM_SAVE_STATE_REGISTER       RegisterNum,
  OUT UINT32                           RegisterData
  )
{
  return mSmmVariableGlobal->SmmCpu->WriteSaveState (
                                       mSmmVariableGlobal->SmmCpu,
                                       sizeof (UINT32),
                                       RegisterNum,
                                       mSmmVariableGlobal->CurrentlyExecutingCpu,
                                       &RegisterData
                                       );

}

/**
  Check is whether support administer secure boot or not.

  @return TRUE  Administer Secure boot is enabled.
  @return FALSE Administer Secure boot is disabled.

--*/
BOOLEAN
IsAdministerSecureBootSupport (
  VOID
  )
{
  STATIC UINT8                   AdmiSecureBoot = 0xFF;
  EFI_STATUS                     Status;
  BOOLEAN                        AdmiSecureBootEnable;
  UINT8                          *Buffer;
  UINTN                          StrSize;
  VARIABLE_POINTER_TRACK         Variable;
  UINTN                          VariableCount;


  if (!FeaturePcdGet (PcdH2OSecureBootSupported)) {
    return FALSE;
  }

  AdmiSecureBootEnable = FALSE;

  if (AdmiSecureBoot == 0xFF) {
    AdmiSecureBoot = 0;
    Status = FindVariableByLifetime (
               EFI_ADMINISTER_SECURE_BOOT_NAME,
               &gEfiGenericVariableGuid,
               &Variable,
               &VariableCount,
               &mVariableModuleGlobal->VariableBase
               );
    if (!EFI_ERROR (Status) && Variable.CurrPtr->DataSize == sizeof (UINT8)) {
      StrSize = Variable.CurrPtr->NameSize + GET_PAD_SIZE (Variable.CurrPtr->NameSize);
      Buffer = (UINT8 *) Variable.CurrPtr;
      Buffer = Buffer + StrSize + sizeof (VARIABLE_HEADER);
      AdmiSecureBoot = *Buffer;
      if (AdmiSecureBoot == 1) {
        AdmiSecureBootEnable = TRUE;
      }
    }
  } else {
    if (AdmiSecureBoot == 1) {
      AdmiSecureBootEnable = TRUE;
    }
  }
  return AdmiSecureBootEnable;

}

/**
  Copy protected mode information from EfiRuntimeServicesData to SMM Ram.
  We only need invoke this function when Legacy Boot event is signaled and system is in SMM mode.

  EfiRuntimeServicesData memory is used by legacy OS, so we must copy memory to SMM Ram
  to prevent from data corrupted by OS.

  @retval EFI_SUCCESS           Restore information to SMM Ram successful.
  @retval EFI_UNSUPPORTED       System isn't in SMM mode.
  @retval EFI_OUT_OF_RESOURCES  Allcoate memory failed.
--*/
STATIC
EFI_STATUS
RestoreProtectedModeInfoToSmmRam (
  VOID
  )
{
  ESAL_VARIABLE_GLOBAL            *VariableGlobal;

  if (mSmst == NULL) {
    return EFI_UNSUPPORTED;
  }

  VariableGlobal = VariableAllocateZeroBuffer (sizeof (ESAL_VARIABLE_GLOBAL), TRUE);
  if (VariableGlobal == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (VariableGlobal, mSmmVariableGlobal->ProtectedModeVariableModuleGlobal, sizeof (ESAL_VARIABLE_GLOBAL));
  mSmmVariableGlobal->ProtectedModeVariableModuleGlobal = VariableGlobal;
  return EFI_SUCCESS;
}

/**
  This function uses to do specific action when legacy boot event is signaled.

  @retval EFI_SUCCESS      All of action for legacy boot event in SMM mode is successful.
  @retval Other            Any error occurred.
--*/
EFI_STATUS
SmmLegacyBootEvent (
  VOID
  )
{

  UINT32         BufferSize;
  UINT8          *VariableBuffer;
  UINT32         Signature;
  STATIC BOOLEAN Initialized = FALSE;

  if (Initialized) {
    return EFI_ALREADY_STARTED;
  }
  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RBX ,&BufferSize);
  VariableBuffer = NULL;
  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RSI ,(UINT32 *) &VariableBuffer);
  Signature = SMM_LEGACY_BOOT_SIGNATURE;

  if (BufferOverlapSmram (VariableBuffer, BufferSize) || BufferSize != sizeof (UINT32) ||
      CompareMem (VariableBuffer, &Signature, sizeof (UINT32))!= 0) {
    return EFI_UNSUPPORTED;
  }
  Initialized = TRUE;
  return RestoreProtectedModeInfoToSmmRam ();
}

//[-start-170418-IB15590029-add]//
EFI_STATUS
SmmInternalQueryVariableInfo (
  VOID
  )
{
  UINT32             BufferSize;
  SMM_VAR_BUFFER    *SmmVarBuffer;

  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RBX ,&BufferSize);
  SmmVarBuffer = NULL;
  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RSI ,(UINT32 *) &SmmVarBuffer);

  SmmVarBuffer->Status = VariableServicesQueryVariableInfo (
                           SmmVarBuffer->Attributes,
                           &SmmVarBuffer->MaximumVariableStorageSize,
                           &SmmVarBuffer->RemainingVariableStorageSize,
                           &SmmVarBuffer->MaximumVariableSize
                           );

  return EFI_SUCCESS;
}
//[-end-170418-IB15590029-add]//

EFI_STATUS
SmmInternalGetVariable (
  VOID
  )
{
  UINT32             BufferSize;
  SMM_VAR_BUFFER    *SmmVarBuffer;
  CHAR16            *VariableName;
  UINT8             *VariableBuffer;


  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RBX ,&BufferSize);
  SmmVarBuffer = NULL;
  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RSI ,(UINT32 *) &SmmVarBuffer);


  VariableName = (CHAR16 *) (SmmVarBuffer + 1);
  VariableBuffer = ((CHAR8 *) (SmmVarBuffer + 1)) + SmmVarBuffer->VariableNameSize;

  SmmVarBuffer->Status = VariableServicesGetVariable (
                           VariableName,
                           &SmmVarBuffer->VarGuid,
                           &SmmVarBuffer->Attributes,
                           &SmmVarBuffer->DataSize,
                           VariableBuffer
                           );
  return EFI_SUCCESS;
}

EFI_STATUS
SmmInternalGetNextVariableName (
  VOID
  )
{
  UINT32             BufferSize;
  SMM_VAR_BUFFER    *SmmVarBuffer;
  CHAR16            *VariableName;
  UINT8             *VariableBuffer;


  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RBX ,&BufferSize);
  SmmVarBuffer = NULL;
  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RSI ,(UINT32 *) &SmmVarBuffer);

  VariableName = (CHAR16 *) (SmmVarBuffer + 1);
  VariableBuffer = ((CHAR8 *) (SmmVarBuffer + 1)) + SmmVarBuffer->VariableNameSize;

  SmmVarBuffer->Status = VariableServicesGetNextVariableName (
                           &SmmVarBuffer->VariableNameSize,
                           VariableName,
                           &SmmVarBuffer->VarGuid
                           );
  return EFI_SUCCESS;
}

/**
  This fucnitons uses to invoke SMM mode SetVariable ()

  @retval EFI_SUCCESS     Disable all secure boot SMI functions successful.
  @return Other           Any erro occured while disabling all secure boot SMI functions successful.

**/
EFI_STATUS
SmmInternalSetVariable (
  VOID
  )
{
  UINT32             BufferSize;
  SMM_VAR_BUFFER    *SmmVarBuffer;
  CHAR16            *VariableName;
  UINT8             *VariableBuffer;


  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RBX ,&BufferSize);
  SmmVarBuffer = NULL;
  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RSI ,(UINT32 *) &SmmVarBuffer);

  if (BufferOverlapSmram (SmmVarBuffer, BufferSize) ||
      BufferSize != sizeof (SMM_VAR_BUFFER) + MAX_VARIABLE_NAME_SIZE + MAX_VARIABLE_SIZE ||
      SmmVarBuffer->Signature != SMM_VARIABLE_SIGNATURE) {
    return EFI_UNSUPPORTED;
  }
  VariableName = (CHAR16 *) (SmmVarBuffer + 1);
  VariableBuffer = ((UINT8 *) (SmmVarBuffer + 1)) + SmmVarBuffer->VariableNameSize;
  if (InternalCalculateSum16 (VariableBuffer, SmmVarBuffer->DataSize) != SmmVarBuffer->VarChecksum) {
    return EFI_UNSUPPORTED;
  }
  SmmVarBuffer->Status = VariableServicesSetVariable (
                           VariableName,
                           &SmmVarBuffer->VarGuid,
                           SmmVarBuffer->Attributes,
                           SmmVarBuffer->DataSize,
                           VariableBuffer
                           );
  return EFI_SUCCESS;
}


/**
  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It convers pointer to new virtual address.

  @param  Event        Event whose notification function is being invoked.
  @param  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
VariableAddressChangeEvent (
  IN EFI_EVENT                            Event,
  IN VOID                                 *Context
  )
{
  UINTN                       Index;
  VARIABLE_STORAGE_PROTOCOL   *VariableStorageProtocol;


  //
  // Setup the callback to determine when to enable variable writes
  //
  for ( Index = 0;
        Index < mVariableModuleGlobal->VariableBase.VariableStoresCount;
        Index++) {
    VariableStorageProtocol = mVariableModuleGlobal->VariableBase.VariableStores[Index];
    gRT->ConvertPointer (0x0, (VOID **) &VariableStorageProtocol->GetId);
    gRT->ConvertPointer (0x0, (VOID **) &VariableStorageProtocol->GetVariable);
    gRT->ConvertPointer (0x0, (VOID **) &VariableStorageProtocol->GetAuthenticatedVariable);
    gRT->ConvertPointer (0x0, (VOID **) &VariableStorageProtocol->GetNextVariableName);
    gRT->ConvertPointer (0x0, (VOID **) &VariableStorageProtocol->GetStorageUsage);
    gRT->ConvertPointer (0x0, (VOID **) &VariableStorageProtocol->GetAuthenticatedSupport);
    gRT->ConvertPointer (0x0, (VOID **) &VariableStorageProtocol->SetVariable);
    gRT->ConvertPointer (0x0, (VOID **) &VariableStorageProtocol->WriteServiceIsReady);
    gRT->ConvertPointer (0x0, (VOID **) &VariableStorageProtocol->RegisterWriteServiceReadyCallback);
    gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->VariableBase.VariableStores[Index]);
  }
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->VariableBase.VariableStores);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->VariableBase.VariableStorageSelectorProtocol->GetId);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->VariableBase.VariableStorageSelectorProtocol);
  //
  // convert relative base addresses or pointers
  //
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->VariableBase.VolatileVariableBase);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->NonVolatileVariableCache);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->FactoryDefaultBase);

  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->GlobalVariableList);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal);

  gRT->ConvertPointer (0x0, (VOID **) &mSmmVarBuf);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableDataBuffer);

  ConvertVariablLockList ();
  ConvertVariableCheckInfo ();

  AuthVariableClassAddressChange ();
}

/**
  This function allows the caller to determine if UEFI ExitBootServices() has been called.

  This function returns TRUE after all the EVT_SIGNAL_EXIT_BOOT_SERVICES functions have
  executed as a result of the OS calling ExitBootServices().  Prior to this time FALSE
  is returned. This function is used by runtime code to decide it is legal to access
  services that go away after ExitBootServices().

  @retval  TRUE  The system has finished executing the EVT_SIGNAL_EXIT_BOOT_SERVICES event.
  @retval  FALSE The system has not finished executing the EVT_SIGNAL_EXIT_BOOT_SERVICES event.

**/
BOOLEAN
EFIAPI
VariableAtRuntime (
  VOID
  )
{
  return mSmst == NULL ? mVariableModuleGlobal->AtRuntime : mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->AtRuntime;
}

/**
  Notification function of gEfiEventExitBootServicesGuid.

  This is a notification function registered on gEfiEventExitBootServicesGuid event.
  It convers pointer to new virtual address.

  @param  Event        Event whose notification function is being invoked.
  @param  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
ExitBootServicesEvent (
  IN EFI_EVENT                            Event,
  IN VOID                                 *Context
  )
{
  mVariableModuleGlobal->AtRuntime = TRUE;
}


/**
  Creates and returns a notification event and registers that event with all the protocol
  instances specified by ProtocolGuid.

  This function causes the notification function to be executed for every protocol of type
  ProtocolGuid instance that exists in the system when this function is invoked. In addition,
  every time a protocol of type ProtocolGuid instance is installed or reinstalled, the notification
  function is also executed. This function returns the notification event that was created.
  If ProtocolGuid is NULL, then ASSERT().
  If NotifyTpl is not a legal TPL value, then ASSERT().
  If NotifyFunction is NULL, then ASSERT().
  If Registration is NULL, then ASSERT().


  @param  ProtocolGuid    Supplies GUID of the protocol upon whose installation the event is fired.
  @param  NotifyTpl       Supplies the task priority level of the event notifications.
  @param  NotifyFunction  Supplies the function to notify when the event is signaled.
  @param  NotifyContext   The context parameter to pass to NotifyFunction.
  @param  Registration    A pointer to a memory location to receive the registration value.
                          This value is passed to LocateHandle() to obtain new handles that
                          have been added that support the ProtocolGuid-specified protocol.

  @return The notification event that was created.

**/
EFI_EVENT
EFIAPI
VariableCreateProtocolNotifyEvent(
  IN  EFI_GUID          *ProtocolGuid,
  IN  EFI_TPL           NotifyTpl,
  IN  EFI_EVENT_NOTIFY  NotifyFunction,
  IN  VOID              *NotifyContext,  OPTIONAL
  OUT VOID              **Registration
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;

  ASSERT (ProtocolGuid != NULL);
  ASSERT (NotifyFunction != NULL);
  ASSERT (Registration != NULL);

  //
  // Create the event
  //

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  NotifyTpl,
                  NotifyFunction,
                  NotifyContext,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Register for protocol notifications on this event
  //

  Status = gBS->RegisterProtocolNotify (
                  ProtocolGuid,
                  Event,
                  Registration
                  );

  ASSERT_EFI_ERROR (Status);

  return Event;
}


/**
  Check the variable is whether authenticated or not.

  @param[In]    Variable        pointer to the variable

  @retval TRUE  the variable is authenticated
  @retval FALSE the variable is not authenticated

**/
BOOLEAN
IsAuthenticatedVariable (
  VARIABLE_HEADER               *Variable
  )
{

  if (((Variable->Attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) == EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) ||
      ((Variable->Attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) == EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)) {
    return TRUE;
  }

  return FALSE;
}

/**
  Check the input memory buffer is whether overlap the SMRAM ranges.

  @param[in] Buffer       The pointer to the buffer to be checked.
  @param[in] BufferSize   The size in bytes of the input buffer

  @retval  TURE        The buffer overlaps SMRAM ranges.
  @retval  FALSE       The buffer doesn't overlap SMRAM ranges.
**/
BOOLEAN
EFIAPI
BufferOverlapSmram (
  IN VOID              *Buffer,
  IN UINTN              BufferSize
  )
{
  UINTN                 Index;
  EFI_PHYSICAL_ADDRESS  BufferEnd;
  EFI_PHYSICAL_ADDRESS  BufferStart;

  if (mSmst == NULL || Buffer == NULL || BufferSize == 0) {
    return FALSE;
  }

  BufferStart = (EFI_PHYSICAL_ADDRESS) (UINTN) Buffer;
  BufferEnd =   (EFI_PHYSICAL_ADDRESS) ((UINTN) Buffer + BufferSize - 1);

  //
  // Integer overflow check
  //
  if (BufferEnd < BufferStart) {
    return TRUE;
  }

  for (Index = 0; Index < mSmramRangeCount; Index ++) {
    //
    // The condition for two ranges doesn't overlap is:
    // Buffer End is smaller than the range start or Buffer start is larger than the range end.
    // so the overlap condition is above condition isn't satisfied.
    //
    if (!(BufferEnd < mSmramRanges[Index].CpuStart ||
        BufferStart >= (mSmramRanges[Index].CpuStart + mSmramRanges[Index].PhysicalSize))) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
  This function allows the caller to determine if UEFI ExitBootServices() has been called.

  This function returns TRUE after all the EVT_SIGNAL_EXIT_BOOT_SERVICES functions have
  executed as a result of the OS calling ExitBootServices().  Prior to this time FALSE
  is returned. This function is used by runtime code to decide it is legal to access
  services that go away after ExitBootServices().

  @retval  TRUE  The system has finished executing the EVT_SIGNAL_EXIT_BOOT_SERVICES event.
  @retval  FALSE The system has not finished executing the EVT_SIGNAL_EXIT_BOOT_SERVICES event.

**/
BOOLEAN
EFIAPI
AfterEndOfDxe (
  VOID
  )
{
  return mSmst == NULL ? mVariableModuleGlobal->EndOfDxe : mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->EndOfDxe;
}

/**
  Update the variable region with Variable information. If EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS is set,
  index of associated public key is needed.

  @param[in] VariableName       Name of variable.
  @param[in] VendorGuid         Guid of variable.
  @param[in] Data               Variable data.
  @param[in] DataSize           Size of data. 0 means delete.
  @param[in] Attributes         Attributes of the variable.
  @param[in] KeyIndex           Index of associated public key.
  @param[in] MonotonicCount     Value of associated monotonic count.
  @param[in, out] CacheVariable The variable information which is used to keep track of variable usage.
  @param[in] TimeStamp          Value of associated TimeStamp.
  @param[in] OnlyUpdateNvCache  TRUE if only the NV cache should be written to, not the VARIABLE_STORAGE_PROTOCOLs

  @retval EFI_SUCCESS           The update operation is success.
  @retval EFI_OUT_OF_RESOURCES  Variable region is full, can not write other data into this region.

**/
EFI_STATUS
UpdateVariableInternal (
  IN      CHAR16                      *VariableName,
  IN      EFI_GUID                    *VendorGuid,
  IN      VOID                        *Data,
  IN      UINTN                       DataSize,
  IN      UINT32                      Attributes      OPTIONAL,
  IN      UINT32                      KeyIndex        OPTIONAL,
  IN      UINT64                      MonotonicCount  OPTIONAL,
  IN OUT  VARIABLE_POINTER_TRACK      *Variable,
  IN      EFI_TIME                    *TimeStamp      OPTIONAL,
  IN      BOOLEAN                     OnlyUpdateNvCache
  )
{
  EFI_STATUS                          Status;
  VARIABLE_HEADER                     *NextVariable;
  UINTN                               NonVolatileVarableStoreSize;
  UINTN                               VarNameOffset;
  UINTN                               VarDataOffset;
  UINTN                               VarNameSize;
  UINTN                               VarSize;
  BOOLEAN                             Volatile;
  UINT8                               State;
  UINTN                               BufSize;
  UINTN                               DataOffset;
  UINT8                               *WriteBuffer;
  UINTN                               *VolatileLastVariableOffset;
  VARIABLE_GLOBAL                     *Global;
  VARIABLE_STORAGE_PROTOCOL           *VariableStorageProtocol;


  WriteBuffer       = NULL;
  Global            = &mVariableModuleGlobal->VariableBase;
  // for Secure Boot variables, perform sanity check of incoming data since the structure is known to firmware
  //
  if (!OnlyUpdateNvCache && IsSecureDatabaseVariable (VariableName, VendorGuid)) {
      Status = CheckSecureBootVarData (VariableName, VendorGuid, Data, DataSize);
      if (EFI_ERROR (Status))
      return Status;
  }

  if (Variable->CurrPtr != NULL) {
    //
    // Update/Delete existing variable.
    //
    Volatile = Variable->Volatile;

    if (VariableAtRuntime () && !IsReadOnlyVariable (VariableName, VendorGuid)) {
      //
      // If AtRuntime and the variable is Volatile and Runtime Access,
      // the volatile is ReadOnly, and SetVariable should be aborted and
      // return EFI_WRITE_PROTECTED.
      //
      if (Variable->Volatile) {
        Status = EFI_WRITE_PROTECTED;
        goto Done;
      }
      //
      // Only variable that have NV attributes can be updated/deleted in Runtime.
      //
      if ((Variable->CurrPtr->Attributes & EFI_VARIABLE_NON_VOLATILE) == 0) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }

    //
    // Special handling for VarErrorFlag
    //
    if (CompareGuid (VendorGuid, &gEdkiiVarErrorFlagGuid) &&
        (StrCmp (VariableName, VAR_ERROR_FLAG_NAME) == 0) &&
        (DataSize == sizeof (VAR_ERROR_FLAG)) && !OnlyUpdateNvCache) {
      RecordVarErrorFlag (*((VAR_ERROR_FLAG *) Data), VariableName, VendorGuid, Attributes, DataSize);
      return EFI_SUCCESS;
    }

    //
    // Setting a data variable with no access, or zero DataSize attributes
    // specified causes it to be deleted.
    //
    if ((((Attributes & EFI_VARIABLE_APPEND_WRITE) == 0) && (DataSize == 0)) || ((Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == 0)) {
      Variable->CurrPtr->State &= VAR_DELETED;
      Status                    = EFI_SUCCESS;

      if (!Variable->Volatile && !OnlyUpdateNvCache) {
        //
        // Delete the variable from the NV storage
        //
        Status = GetVariableStorageProtocol (
                  VariableName,
                  VendorGuid,
                  &VariableStorageProtocol
                  );
        if (!EFI_ERROR (Status)) {
          if (VariableStorageProtocol == NULL) {
            ASSERT (VariableStorageProtocol != NULL);
            return EFI_NOT_FOUND;
          }
          Status = VariableStorageProtocol->SetVariable (
                                              VariableStorageProtocol,
                                              VariableName,
                                              VendorGuid,
                                              Attributes,
                                              0,
                                              NULL,
                                              VariableAtRuntime (),
                                              0,
                                              0,
                                              TimeStamp
                                              );
        }
        if (Status == EFI_SUCCESS && IsUserVariable ((CHAR16 *) (Variable->CurrPtr + 1), &Variable->CurrPtr->VendorGuid)) {
          VarSize = ((UINTN) GetNextVariablePtr (Variable->CurrPtr) - (UINTN) Variable->CurrPtr);
          if (mSmst == NULL) {
            mVariableModuleGlobal->CommonUserVariableTotalSize -= VarSize;
          } else {
            mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->CommonUserVariableTotalSize -= VarSize;
          }
        }
      }

      goto Done;
    }
    //
    // If the variable is marked valid, and the same data has been passed in,
    // then return to the caller immediately.
    //
    if (DataSizeOfVariable (Variable->CurrPtr) == DataSize &&
        (CompareMem (Data, GetVariableDataPtr (Variable->CurrPtr), DataSize) == 0) &&
        ((Attributes & EFI_VARIABLE_APPEND_WRITE) == 0) && (TimeStamp == NULL || CompareTimeStamp (TimeStamp, &Variable->CurrPtr->TimeStamp))) {
      Status = EFI_SUCCESS;
      goto Done;
    }
    //
    // EFI_VARIABLE_APPEND_WRITE attribute only effects for existing variable
    // FIXUP - need to check for duplicate data before appending data, also need to append to correct
    // list, assuming this is a cert, or a PubKey!
    //
    if ((Attributes & EFI_VARIABLE_APPEND_WRITE) != 0) {

      BufSize = Variable->CurrPtr->DataSize + DataSize;

      SetMem (mStorageArea, APPEND_BUFF_SIZE, 0xff);
      //
      // Cache the previous variable data into StorageArea.
      //
      DataOffset = sizeof (VARIABLE_HEADER) + Variable->CurrPtr->NameSize + GET_PAD_SIZE (Variable->CurrPtr->NameSize);
      CopyMem (mStorageArea, (UINT8*)((UINTN)Variable->CurrPtr + DataOffset), Variable->CurrPtr->DataSize);

      //
      // Append the new data to the end of previous data.
      // for special secure boot variables, need to check for duplicate data before append
      //
      if ((DataSize > 0) && ((IsKekVariable (VariableName, VendorGuid)) ||
        (IsImageSecureDatabaseVariable (VariableName, VendorGuid)))) {
        BufSize = Variable->CurrPtr->DataSize;
        Status = AppendSignatureList (DataSize, Data, MAX_VARIABLE_SIZE, &BufSize, mStorageArea);
        if (EFI_ERROR (Status)) {
          goto Done;
        }
        //
        // If appended signature is already existence, just return SUCCESS;
        //
        if ((BufSize == Variable->CurrPtr->DataSize) &&
            (TimeStamp == NULL || CompareTimeStamp (TimeStamp, &Variable->CurrPtr->TimeStamp))) {
          Status = EFI_SUCCESS;
          goto Done;
        }
      } else {
        //
        // If appended data size is 0, just return EFI_SUCCESS
        //
        if ((DataSize == 0) &&
            (TimeStamp == NULL || CompareTimeStamp (TimeStamp, &Variable->CurrPtr->TimeStamp))) {
          Status = EFI_SUCCESS;
          goto Done;
        }

      if (BufSize > APPEND_BUFF_SIZE) {
        //
        // If variable size (previous + current) is bigger than 4K that was reserved in runtime,
        // return EFI_OUT_OF_RESOURCES.
        //
        return EFI_OUT_OF_RESOURCES;
      }
        CopyMem ((UINT8*)((UINTN)mStorageArea + Variable->CurrPtr->DataSize), Data, DataSize);
      }
      //
      // Override Data and DataSize which are used for combined data area including previous and new data.
      //
      Data     = mStorageArea;
      DataSize = BufSize;
    }

    if ((Variable->CurrPtr->State == VAR_ADDED) ||
       (Variable->CurrPtr->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION))) {

      //
      // Mark the old variable as in delete transition.
      //
      Variable->CurrPtr->State &= VAR_IN_DELETED_TRANSITION;
    }
  //
  // Not found existing variable. Create a new variable.
  //
  } else {
    //
    // Make sure we are trying to create a new variable.
    // Setting a data variable with no access, or zero DataSize attributes means to delete it.
    //
    if (DataSize == 0 || (Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == 0) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }

    //
    // Only variable have NV|RT attribute can be created in Runtime.
    //
    if (VariableAtRuntime () &&
        (((Attributes & EFI_VARIABLE_RUNTIME_ACCESS) == 0) || ((Attributes & EFI_VARIABLE_NON_VOLATILE) == 0))) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  //
  // Tricky part: Use scratch data area at the end of volatile variable store
  // as a temporary storage.
  //
  if (mSmst == NULL) {
    NextVariable = GetEndPointer ((VARIABLE_STORE_HEADER *) ((UINTN) Global->VolatileVariableBase));
  } else {
    WriteBuffer = VariableAllocateZeroBuffer (SCRATCH_SIZE, FALSE);
    if (WriteBuffer == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    NextVariable = (VARIABLE_HEADER *) WriteBuffer;
  }

  SetMem (NextVariable, SCRATCH_SIZE, 0xff);

  NextVariable->StartId     = VARIABLE_DATA;
  NextVariable->Attributes  = Attributes;
  //
  // NextVariable->State = VAR_ADDED;
  //
  NextVariable->Reserved        = 0;
  NextVariable->PubKeyIndex     = KeyIndex;
  NextVariable->MonotonicCount  = MonotonicCount;
  SetMem (&NextVariable->TimeStamp, sizeof (EFI_TIME), 0);
  //
  // if creating a new variable, use the passed in timestamp, if available
  //
  if ((TimeStamp != NULL) && (Variable->CurrPtr == NULL)) {
    CopyMem (&NextVariable->TimeStamp, TimeStamp, sizeof (EFI_TIME));
  }
  //
  // for UEFI 2.3.1
  //
  if (((Attributes & EFI_VARIABLE_APPEND_WRITE) == 0) &&
  		((Attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) ==
		EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)) {
    ASSERT (TimeStamp != NULL);
    if (TimeStamp != NULL) {
      CopyMem (&NextVariable->TimeStamp, TimeStamp, sizeof (EFI_TIME));
    }
  } else if (((Attributes & EFI_VARIABLE_APPEND_WRITE) != 0) &&
             ((Attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) != 0) &&
             (Variable->CurrPtr != NULL))  {
      //
      // In the case when the EFI_VARIABLE_APPEND_WRITE attribute is set, only
      // when the new TimeStamp value is later than the current timestamp associated
      // with the variable, we need associate the new timestamp with the updated value.
      //
    CopyMem (&NextVariable->TimeStamp, &Variable->CurrPtr->TimeStamp, sizeof (EFI_TIME));
    if (TimeStamp != NULL && !CompareTimeStamp (TimeStamp, &Variable->CurrPtr->TimeStamp)) {
      CopyMem (&NextVariable->TimeStamp, TimeStamp, sizeof (EFI_TIME));
    }
  }

  //
  // The EFI_VARIABLE_APPEND_WRITE attribute will never be set in the returned
  // Attributes bitmask parameter of a GetVariable() call.
  //
  NextVariable->Attributes  = Attributes & (~EFI_VARIABLE_APPEND_WRITE);

  VarNameOffset                 = sizeof (VARIABLE_HEADER);
  VarNameSize                   = StrSize (VariableName);
  CopyMem (
    (UINT8 *) ((UINTN) NextVariable + VarNameOffset),
    VariableName,
    VarNameSize
    );
  VarDataOffset = VarNameOffset + VarNameSize + GET_PAD_SIZE (VarNameSize);
  CopyMem (
    (UINT8 *) ((UINTN) NextVariable + VarDataOffset),
    Data,
    DataSize
    );
  CopyMem (&NextVariable->VendorGuid, VendorGuid, sizeof (EFI_GUID));
  //
  // There will be pad bytes after Data, the NextVariable->NameSize and
  // NextVariable->DataSize should not include pad size so that variable
  // service can get actual size in GetVariable.
  //
  NextVariable->NameSize  = (UINT32)VarNameSize;
  NextVariable->DataSize  = (UINT32)DataSize;

  //
  // The actual size of the variable that stores in storage should
  // include pad size.
  //
  VarSize = VarDataOffset + DataSize + GET_PAD_SIZE (DataSize);
  if ((Attributes & EFI_VARIABLE_NON_VOLATILE) != 0) {
    //
    // Sync mVariableModuleGlobal->NonVolatileLastVariableOffset first. This is caused by we
    // may invoke this function between two different modes (protected mode and SMM mode).
    //
    mVariableModuleGlobal->NonVolatileLastVariableOffset = GetCurrentNonVolatileOffset ();
    //
    // Create a nonvolatile variable.
    //
    Volatile = FALSE;
    NonVolatileVarableStoreSize = GetNonVolatileVariableStoreSize ();
    if ((UINT32) (VarSize + mVariableModuleGlobal->NonVolatileLastVariableOffset) >
          NonVolatileVarableStoreSize
          ) {
      if (VariableAtRuntime () && !PcdGetBool(PcdRuntimeReclaimSupported)) {
        SetDoReclaimNextBoot ();
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      //
      // Perform garbage collection & reclaim operation.
      //
      Status = ReclaimNonVolatileVariable (
                 NextVariable,
                 VarSize,
                 (EFI_PHYSICAL_ADDRESS) (UINTN) mVariableModuleGlobal->NonVolatileVariableCache,
                 &mVariableModuleGlobal->NonVolatileLastVariableOffset
                 );
      goto Done;
    }

    if (!OnlyUpdateNvCache) {
      //
      // Write the variable to NV
      //
      Status = GetVariableStorageProtocol (
                VariableName,
                VendorGuid,
                &VariableStorageProtocol
                );
      if (EFI_ERROR (Status) || VariableStorageProtocol == NULL) {
        goto Done;
      }
      Status = VariableStorageProtocol->SetVariable (
                                          VariableStorageProtocol,
                                          VariableName,
                                          VendorGuid,
                                          Attributes,
                                          DataSize,
                                          Data,
                                          VariableAtRuntime (),
                                          KeyIndex,
                                          MonotonicCount,
                                          TimeStamp
                                          );
      if (EFI_ERROR (Status)) {
        goto Done;
      }

      if (IsUserVariable ((CHAR16 *) (NextVariable + 1), &NextVariable->VendorGuid)) {
        if (mSmst == NULL) {
          mVariableModuleGlobal->CommonUserVariableTotalSize += VarSize;
        } else {
          mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->CommonUserVariableTotalSize += VarSize;
        }
      }
    }
    //
    // Update the NV Cache
    //
    NextVariable->State = VAR_ADDED;
    Status = UpdateVariableStore (
               Global,
               FALSE,
               TRUE,
                mVariableModuleGlobal->NonVolatileLastVariableOffset,
               (UINT32) VarSize,
               (UINT8 *) NextVariable
               );
//[-start-161023-IB07400803-modify]//
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "  Variable Driver: Wrote the variable to the NV cache in UpdateVariable(). Status = %r\n", Status));
//[-end-161023-IB07400803-modify]//
      goto Done;
    }
    mVariableModuleGlobal->NonVolatileLastVariableOffset += (VarSize);
  } else {
    //
    // Create a volatile variable.
    //
    Volatile = TRUE;
    VolatileLastVariableOffset = NULL;
    if (mSmst == NULL) {
      VolatileLastVariableOffset = &mVariableModuleGlobal->VolatileLastVariableOffset;
    } else {
      VolatileLastVariableOffset = &mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->VolatileLastVariableOffset;
    }

    if ((UINT32) (VarSize + *VolatileLastVariableOffset) >
        GetVariableStoreSize ((VARIABLE_STORE_HEADER *) (UINTN) (Global->VolatileVariableBase))) {
      //
      // Perform garbage collection & reclaim operation.
      //
      Status = Reclaim (
                 Global->VolatileVariableBase,
                 VolatileLastVariableOffset,
                 &Variable->CurrPtr
                 );
      if (EFI_ERROR (Status)) {
        goto Done;
      }
      //
      // If still no enough space, return out of resources.
      //
      if ((UINT32) (VarSize + *VolatileLastVariableOffset) >
            GetVariableStoreSize ((VARIABLE_STORE_HEADER *) (UINTN) (Global->VolatileVariableBase))
            ) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }
    }

    NextVariable->State = VAR_ADDED;
    Status = UpdateVariableStore (
               Global,
               TRUE,
               TRUE,
               *VolatileLastVariableOffset,
               (UINT32) VarSize,
               (UINT8 *) NextVariable
               );

    if (EFI_ERROR (Status)) {
      goto Done;
    }

    *VolatileLastVariableOffset += (VarSize);
  }

  //
  // Mark the old variable as deleted.
  //
  if (!EFI_ERROR (Status) && Variable->CurrPtr != NULL) {
    State = Variable->CurrPtr->State;
    State &= VAR_DELETED;

    Status = UpdateVariableStore (
             Global,
             Variable->Volatile,
             FALSE,
             (UINTN) &Variable->CurrPtr->State,
             sizeof (UINT8),
             &State
             );
    if (Status == EFI_SUCCESS && !OnlyUpdateNvCache && !Variable->Volatile &&
        IsUserVariable ((CHAR16 *) (Variable->CurrPtr + 1), &Variable->CurrPtr->VendorGuid)) {
      VarSize = ((UINTN) GetNextVariablePtr (Variable->CurrPtr) - (UINTN) Variable->CurrPtr);
      if (mSmst == NULL) {
        mVariableModuleGlobal->CommonUserVariableTotalSize -= VarSize;
      } else {
        mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->CommonUserVariableTotalSize -= VarSize;
      }
    }
  }

Done:
  if (mSmst != NULL && WriteBuffer != NULL) {
    EFI_FREE_POOL (WriteBuffer);
  }
  return Status;
}
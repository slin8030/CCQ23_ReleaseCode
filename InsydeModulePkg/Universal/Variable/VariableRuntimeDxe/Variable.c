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
#include "Reclaim.h"
#include "AuthService.h"
#include "InsydeSecureVariable.h"
#include "VariableLock.h"
#include "VarCheck.h"
#include "GlobalVariable.h"
#include "SensitiveVariable.h"
#include <PostCode.h>

#include <Protocol/NonVolatileVariable.h>
#include <Protocol/VariableDefaultUpdate.h>
#include <Protocol/Bds.h>

#include <Library/FlashRegionLib.h>

#define VARIABLE_WORKING_REGION_HOB_GUID {0x92888eba, 0xe125, 0x4c41, 0xbc, 0x9d, 0x68, 0x3e, 0x8f, 0x7e, 0x61, 0x18}
STATIC EFI_GUID  mVariableWorkingRegionHobGuid = VARIABLE_WORKING_REGION_HOB_GUID;
extern EFI_GUID        gEfiAlternateFvBlockGuid;


typedef struct {
  EFI_PHYSICAL_ADDRESS              VariableWorkingRegionStart;
  EFI_PHYSICAL_ADDRESS              VariableWorkingRegionLength;
} VARIALBE_WORKING_REGION_INFO;


BOOLEAN                               mReadyToBootEventSignaled = FALSE;
SMM_VAR_BUFFER                        *mSmmVarBuf;
SMM_VAR_BUFFER                        *mSmmPhyVarBuf;
VARIALBE_RECLAIM_INFO                 *mVariableReclaimInfo;
EFI_BDS_ENTRY                         mOriginalBdsEntry;
UINTN                                 mSmramRangeCount;
EFI_SMRAM_DESCRIPTOR                  *mSmramRanges;
VAR_ERROR_FLAG                        mCurrentBootVarErrFlag = VAR_ERROR_FLAG_NO_ERROR;

SMI_SUB_FUNCTION_MAP mNonSecureBootFunctionsTable [] = {
  { SET_SENSITIVE_VARIABLE_FUN_NUM,           SmmSetSensitiveVariable       , TRUE}, \
  { SMM_VARIABLE_CHECK_FUN_NUM,               SmmCreateVariableCheckList    , TRUE}, \
  { SMM_VARIABLE_LOCK_FUN_NUM,                SmmCreateVariableLockList     , TRUE}, \
  { LEGACY_BOOT_SMI_FUN_NUM,                  SmmLegacyBootEvent            , TRUE}, \
  { SMM_SET_VARIABLE_SMI_FUN_NUM,             SmmInternalSetVariable        , TRUE}, \
  { DISABLE_VARIABLE_CACHE_SMI_FUN_NUM,       SmmDisableVariableCache       , TRUE}, \
  { DISABLE_SECURE_BOOT_SMI_FUN_NUM,          SmmDisableSecureBootSmi       , TRUE}, \
  { UPDATE_VARIABLE_PROPERTY_FUN_NUM,         SmmUpdateVariablePropertySmi  , TRUE}, \
  { 0,                                        NULL                          , TRUE}
  };

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
  Check the saved data and the input variable data is whether the same.

  @param  VariableBase    Base address of variable store
  @param  VariableOffset  The offset of current non-volatile variable
  @param  Variable        Pointer to the Variable Store Header.

  @retval TRUE            This variable is correct.
  @retval FALSE           This varaible is incorrect.

**/
BOOLEAN
EFIAPI
IsCorrectVariable (
  IN   EFI_PHYSICAL_ADDRESS  VariableBase,
  IN   UINTN                 VariableOffset,
  IN   VARIABLE_HEADER       *Variable

  )
{
  VARIABLE_HEADER        *CurrentVariable;
  EFI_PHYSICAL_ADDRESS   ReadPtr;
  UINT32                 WholeVariableSize;
  BOOLEAN                VariableIsCorrect;

  //
  // Initialize local variable
  //
  CurrentVariable = Variable;
  ReadPtr = VariableBase + VariableOffset;

  //
  // Calculate variable size from current variable
  //
  WholeVariableSize = 0;
  WholeVariableSize = sizeof (VARIABLE_HEADER) + CurrentVariable->NameSize + CurrentVariable->DataSize;

  //
  // Check Vriable is whether correct
  //
  VariableIsCorrect = (CompareMem ((UINT8 *) ((UINTN) ReadPtr), Variable, WholeVariableSize) == 0) ? TRUE : FALSE;

  return VariableIsCorrect;
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
  This funciotn is used to disable variable SMM mode cache mechanism in protected mode or SMM mode.

**/
VOID
DisableSmmModeVariableCache (
  VOID
  )
{
  UINT32            Signature;

  //
  // Needn't disable cache in runtime if support SMM mode, due to always set variable through SMI in runtime.
  //
  if (!VariableAtRuntime () && mVariableModuleGlobal->SmmCodeReady && mSmst == NULL) {
    Signature = DISABLE_VARIABLE_CACHE_SIGNATURE;
    CopyMem (mSmmPhyVarBuf, &Signature, sizeof (Signature));
    SmmSecureBootCall ((UINT8 *) mSmmPhyVarBuf, sizeof (Signature), DISABLE_VARIABLE_CACHE_SMI_FUN_NUM, SW_SMI_PORT);
  } else if (mSmst != NULL) {
    mVariableModuleGlobal->NonVolatileVariableCache = NULL;
  }
  return;
}

/**
  This funciotn is used to disable variable proteced mode cache mechanism in protected mode or SMM mode.

**/
VOID
DisableProtectedModeVariableCache (
  VOID
  )
{
  if (mSmst != NULL) {
    if (mSmmVariableGlobal->ProtectedModeVariableModuleGlobal != NULL) {
      mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->NonVolatileVariableCache = NULL;
    }
  } else {
    mVariableModuleGlobal->NonVolatileVariableCache = NULL;
  }
  return;
}

/**
  This funciotn is used to disable variable whole cache mechanism in protected mode or SMM mode.

**/
VOID
DisableVariableCache (
  VOID
  )
{
  DisableProtectedModeVariableCache ();
  DisableSmmModeVariableCache ();
  return;
}


/**
  This fucnitons uses to disable variable cache address

  @return EFI_SUCCESS    Disable all secure boot SMI functions successful.
  @return Other          Any erro occured while disabling all secure boot SMI functions successful.

**/
EFI_STATUS
SmmDisableVariableCache (
  VOID
  )
{

  UINT32         BufferSize;
  UINT8          *VariableBuffer;
  UINT32         Signature;

  //
  // Check signature to prevent from other application disables cache.
  // If this situation occured, it may cause cache data isn't the same between
  // protected mode and SMM mode. the worst case, system may write incorrect data to
  // variable store.
  //
  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RBX ,&BufferSize);
  VariableBuffer = NULL;
  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RSI ,(UINT32 *) &VariableBuffer);
  Signature = DISABLE_VARIABLE_CACHE_SIGNATURE;

  if (BufferOverlapSmram (VariableBuffer, BufferSize) || BufferSize != sizeof (UINT32) ||
      CompareMem (VariableBuffer, &Signature, sizeof (UINT32))!= 0) {
    return EFI_UNSUPPORTED;
  }

  mVariableModuleGlobal->NonVolatileVariableCache = NULL;
  return EFI_SUCCESS;
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
  EFI_FV_BLOCK_MAP_ENTRY              *PtrBlockMapEntry;
  UINTN                               BlockIndex2;
  UINTN                               LinearOffset;
  UINTN                               CurrWriteSize;
  UINTN                               CurrWritePtr;
  UINT8                               *CurrBuffer;
  EFI_LBA                             LbaNumber;
  UINTN                               Size;
  EFI_FIRMWARE_VOLUME_HEADER          *FwVolHeader;
  VARIABLE_STORE_HEADER               *VolatileBase;
  EFI_PHYSICAL_ADDRESS                FvVolHdr;
  EFI_PHYSICAL_ADDRESS                DataPtr;
  EFI_STATUS                          Status;
  UINTN                               WriteDataSize;
  BOOLEAN                             FvFound;
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  *Fvb;

  Status      = EFI_SUCCESS;
  DataPtr     = DataPtrIndex;

  if (Volatile) {
    //
    // Update volatile varialbe in SMM mode or protected mode
    //
    Status = EFI_INVALID_PARAMETER;
    VolatileBase = (VARIABLE_STORE_HEADER *) ((UINTN) Global->VolatileVariableBase);
    if (SetByIndex) {
      DataPtr += Global->VolatileVariableBase;
    }
    if ((DataPtr + DataSize) < ((UINTN) ((UINT8 *) VolatileBase + GetVariableStoreSize (VolatileBase)))) {
      //
      // If Volatile Variable just do a simple mem copy.
      //
      CopyMem ((UINT8 *) ((UINTN) DataPtr), Buffer, DataSize);
      Status =  EFI_SUCCESS;
    }
  } else if (mSmst == NULL) {
    //
    // update non-volatile varialbe in protected mode
    //
    Fvb         = mVariableModuleGlobal->FvbInstance;
    Status = Fvb->GetPhysicalAddress(Fvb, &FvVolHdr);
    ASSERT_EFI_ERROR (Status);
    FwVolHeader = (EFI_FIRMWARE_VOLUME_HEADER *) ((UINTN) FvVolHdr);

    //
    // Data Pointer should point to the actual Address where data is to be
    // written
    //
    if (SetByIndex) {
      DataPtr += Global->NonVolatileVariableBase;
    }

    if ((DataPtr + DataSize) >= ((EFI_PHYSICAL_ADDRESS) (UINTN) ((UINT8 *) FwVolHeader + FwVolHeader->FvLength))) {
      return EFI_INVALID_PARAMETER;
    }
    //
    // If we are here we are dealing with Non-Volatile Variables
    //
    LinearOffset  = (UINTN) FwVolHeader;
    CurrWritePtr  = (UINTN) DataPtr;
    CurrWriteSize = DataSize;
    CurrBuffer    = Buffer;
    LbaNumber     = 0;

    if (CurrWritePtr < LinearOffset) {
      return EFI_INVALID_PARAMETER;
    }

    FvFound = FALSE;
    for (PtrBlockMapEntry = FwVolHeader->BlockMap; PtrBlockMapEntry->NumBlocks != 0; PtrBlockMapEntry++) {
      for (BlockIndex2 = 0; BlockIndex2 < PtrBlockMapEntry->NumBlocks; BlockIndex2++) {
        //
        // Check to see if the Variable Writes are spanning through multiple
        // blocks.
        //
        if ((CurrWritePtr >= LinearOffset) && (CurrWritePtr < LinearOffset + PtrBlockMapEntry->Length)) {
          FvFound = TRUE;
          if ((CurrWritePtr + CurrWriteSize) <= (LinearOffset + PtrBlockMapEntry->Length)) {
            Status = Fvb->Write (
                            Fvb,
                            LbaNumber,
                            (UINTN) (CurrWritePtr - LinearOffset),
                            &CurrWriteSize,
                            CurrBuffer
                            );
            break;
          } else {
            Size = (UINT32) (LinearOffset + PtrBlockMapEntry->Length - CurrWritePtr);
            Status = Fvb->Write (
                           Fvb,
                           LbaNumber,
                           (UINTN) (CurrWritePtr - LinearOffset),
                           &Size,
                           CurrBuffer
                           );
            if (EFI_ERROR (Status)) {
              break;
            }
            CurrWritePtr  = LinearOffset + PtrBlockMapEntry->Length;
            CurrBuffer    = CurrBuffer + Size;
            CurrWriteSize = CurrWriteSize - Size;
          }
        }

        LinearOffset += PtrBlockMapEntry->Length;
        LbaNumber++;
      }
      if (FvFound) {
        break;
      }
    }
  } else {
    //
    // update non-volatile varialbe in SMM mode
    //
    if (SetByIndex) {
      DataPtr += Global->NonVolatileVariableBase;
    }
    WriteDataSize = (UINTN) DataSize;
    Status = mSmmVariableGlobal->SmmFwbService->Write (
                                                  mSmmVariableGlobal->SmmFwbService,
                                                  (UINTN ) DataPtr,
                                                  &WriteDataSize,
                                                  Buffer
                                                  );
  }

  if (mVariableModuleGlobal->NonVolatileVariableCache != NULL && !Volatile) {
    if (!EFI_ERROR (Status)) {
      CopyMem (
        (VOID *) ((UINTN) DataPtr - (UINTN) Global->NonVolatileVariableBase + (UINTN) mVariableModuleGlobal->NonVolatileVariableCache),
         Buffer,
         DataSize
         );
    } else {
      //
      // If updating NV data failed, turn off variable chache to prevent from data corrupted
      //
      DisableVariableCache ();
    }
  }

  return Status;
}

/**
  Convert memory address saves non-volatile data to real non-volatile data address.

  @param Global                  VARIABLE_GLOBAL pointer
  @param PtrTrack                [in]: Current pointer to Variable track pointer structure in cache or non-volatile that contains variable information.
                                 [out]: Current pointer to Variable track pointer structure in non-volatile that contains variable information.
  @retval EFI_INVALID_PARAMETER  Input parameter is invalid.
  @retval EFI_SUCCESS            Convert address successful.

**/
EFI_STATUS
ConvertCacheAddressToPhysicalAddress (
  IN     VARIABLE_GLOBAL          *Global,
  IN OUT VARIABLE_POINTER_TRACK   *PtrTrack
  )
{
  if (PtrTrack == NULL || Global == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Needn't convert volatile or address has been converted. Just return success.
  //
  if (PtrTrack->Volatile ||
      (UINTN) PtrTrack->StartPtr == Global->NonVolatileVariableBase + GetVariableStoreHeaderSize ()) {
    return EFI_SUCCESS;
  }

  ASSERT (mVariableModuleGlobal->NonVolatileVariableCache != NULL);
  PtrTrack->StartPtr = (VARIABLE_HEADER *) ((UINTN) Global->NonVolatileVariableBase + GetVariableStoreHeaderSize ());
  PtrTrack->EndPtr   = GetNonVolatileEndPointer ((VARIABLE_STORE_HEADER *) (UINTN) Global->NonVolatileVariableBase);
  if (PtrTrack->CurrPtr != NULL) {
    PtrTrack->CurrPtr = (VARIABLE_HEADER *) ((UINTN) Global->NonVolatileVariableBase +
                        (UINTN) PtrTrack->CurrPtr - (UINTN) mVariableModuleGlobal->NonVolatileVariableCache);
  }

  return EFI_SUCCESS;
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
  IN  CONST CHAR16                  *VariableName,
  IN  CONST EFI_GUID                *VendorGuid,
  OUT       VARIABLE_POINTER_TRACK  *PtrTrack,
  OUT       UINTN                   *VariableCount,
  IN        VARIABLE_GLOBAL         *Global
  )
{
  VARIABLE_HEADER       *Variable[2];
  VARIABLE_STORE_HEADER *VariableStoreHeader[2];
  UINTN                 Index;
  UINTN                 IndexMax;
  VARIABLE_HEADER       *InDeletedVariable;
  UINTN                 InDeletedIndex;
  BOOLEAN               VariableFound;
  EFI_STATUS            Status;


  InDeletedVariable = NULL;
  InDeletedIndex    = (UINTN)-1;
  *VariableCount = 0;

  //
  // 0: Non-Volatile, 1: Volatile
  //
  IndexMax = 2;
  if (mVariableModuleGlobal->NonVolatileVariableCache == NULL) {
    VariableStoreHeader[0]  = (VARIABLE_STORE_HEADER *) ((UINTN) Global->NonVolatileVariableBase);
  } else {
    VariableStoreHeader[0]  = (VARIABLE_STORE_HEADER *) mVariableModuleGlobal->NonVolatileVariableCache;
  }
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

  VariableFound = FALSE;
  //
  // Find the variable by walk through non-volatile and volatile variable store
  //
  for (Index = 0; Index < IndexMax; Index++) {
    PtrTrack->StartPtr  = GetStartPointer (VariableStoreHeader[Index]);
    if (Index == 0) {
      PtrTrack->EndPtr = GetNonVolatileEndPointer (VariableStoreHeader[Index]);
    } else {
      PtrTrack->EndPtr = GetEndPointer (VariableStoreHeader[Index]);
    }

    while (IsValidVariableHeaderInVarRegion (Variable[Index], PtrTrack->EndPtr)) {
      if (Variable[Index]->State == VAR_ADDED) {
        if (VariableName[0] == 0) {
          VariableFound = TRUE;
          break;
        } else if (CompareGuid (VendorGuid, &Variable[Index]->VendorGuid) &&
                   !StrCmp (VariableName, GET_VARIABLE_NAME_PTR (Variable[Index]))) {
          VariableFound = TRUE;
          break;
        }
      } else if (Variable[Index]->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION)) {
        //
        // VAR_IN_DELETED_TRANSITION should also be checked.
        //
        if (VariableName[0] == 0) {
          InDeletedVariable = Variable[Index];
          InDeletedIndex    = Index;
          (*VariableCount)++;
        } else if (CompareGuid (VendorGuid, &Variable[Index]->VendorGuid) &&
                   !StrCmp (VariableName, GET_VARIABLE_NAME_PTR (Variable[Index]))) {
          InDeletedVariable = Variable[Index];
          InDeletedIndex    = Index;
          (*VariableCount)++;
        }
      }
      Variable[Index] = GetNextVariablePtr (Variable[Index]);
    }
    if (VariableFound) {
      break;
    }
  }

  if (VariableFound) {
    PtrTrack->CurrPtr   = Variable[Index];
    PtrTrack->Volatile  = (BOOLEAN) (Index);
    (*VariableCount)++;
  } else if (InDeletedVariable != NULL) {
    PtrTrack->CurrPtr   = InDeletedVariable;
    PtrTrack->Volatile  = (BOOLEAN) InDeletedIndex;
    VariableFound = TRUE;
  }

  Status = EFI_SUCCESS;
  if (VariableFound) {
    ConvertCacheAddressToPhysicalAddress (Global, PtrTrack);
    //
    // If non-volatile variable data is invalid, we should flush variable cache to synchronize non-volatile variable data.
    //
    if (!IsValidVariableHeader (PtrTrack->CurrPtr)) {
      ASSERT (FALSE);
      Status = FlushVariableCache (
                 mVariableModuleGlobal->NonVolatileVariableCache,
                 (UINT8 *) (UINTN) mVariableModuleGlobal->VariableBase.NonVolatileVariableBase,
                 mVariableModuleGlobal->NonVolatileVariableCacheSize,
                 &mVariableModuleGlobal->NonVolatileLastVariableOffset
                 );
      if (!EFI_ERROR (Status)) {
        Status = FindVariableByLifetime (
                   VariableName,
                   VendorGuid,
                   PtrTrack,
                   VariableCount,
                   Global
                   );
      }
    }
  } else {
    PtrTrack->CurrPtr = NULL;
    Status = EFI_NOT_FOUND;
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
    Variable = GetStartPointer ((VARIABLE_STORE_HEADER *) mVariableModuleGlobal->NonVolatileVariableCache);
    while (IsValidVariableHeaderInVarRegion (Variable, GetNonVolatileEndPointer ((VARIABLE_STORE_HEADER *) mVariableModuleGlobal->NonVolatileVariableCache))) {
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

  VariableStoreHeader = (VARIABLE_STORE_HEADER *) ((UINTN) Global->NonVolatileVariableBase);
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
  VARIABLE_HEADER             *Variable;
  VARIABLE_HEADER             *NextVariable;
  VARIABLE_STORE_HEADER       *VariableStoreHeader;
  UINT8                       *ValidBuffer;
  UINTN                       ValidBufferSize;
  UINTN                       VariableSize;
  UINTN                       TotalBufferSize;
  UINT8                       *CurrPtr;
  EFI_STATUS                  Status;
  UINTN                       CommonUserVariableTotalSize;

  POST_CODE (DXE_VARIABLE_RECLAIM);
  if (NewVariable == NULL || LastVariableOffset == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  CommonUserVariableTotalSize = 0;
  VariableStoreHeader = (VARIABLE_STORE_HEADER *) ((UINTN) VariableBase);
  Variable = GetStartPointer (VariableStoreHeader);

  //
  // To make the reclaim, here we just allocate a memory that equal to the original memory
  //
  TotalBufferSize = GetNonVolatileVariableStoreSize ();
  ValidBuffer = NULL;
  if (mSmst == NULL) {
    ValidBuffer = (UINT8 *) mVariableReclaimInfo->BackupBuffer;
  } else {
    ValidBuffer = VariableAllocateZeroBuffer (TotalBufferSize, FALSE);
  }

  if (ValidBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
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
    // The CurrentVariable, is also saved, as SetVariable may fail due to lack of space
    //
    if (Variable->State == VAR_ADDED &&
        !(CompareGuid (&NewVariable->VendorGuid, &Variable->VendorGuid) &&
        !StrCmp (GET_VARIABLE_NAME_PTR (NewVariable), GET_VARIABLE_NAME_PTR (Variable)))) {
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
      if (!ExistNewerVariable(Variable) &&
          !(CompareGuid (&NewVariable->VendorGuid, &Variable->VendorGuid) &&
           !StrCmp (GET_VARIABLE_NAME_PTR (NewVariable), GET_VARIABLE_NAME_PTR (Variable)))) {
        VariableSize = (UINTN) NextVariable - (UINTN) Variable;
        CopyMem (CurrPtr, (UINT8 *) Variable, VariableSize);
        ((VARIABLE_HEADER *) CurrPtr)->State = VAR_ADDED;
        CurrPtr += VariableSize;
        if (IsUserVariable ((CHAR16 *) (Variable + 1), &Variable->VendorGuid)) {
          CommonUserVariableTotalSize += VariableSize;
        }
      }
    }
  }
  ValidBufferSize = (UINTN) (CurrPtr - ValidBuffer);
  if (ValidBufferSize + NewVarSize > TotalBufferSize) {
    ElimateVariableWhenRegionFull (ValidBuffer, TotalBufferSize, &ValidBufferSize);
    if (ValidBufferSize + NewVarSize > TotalBufferSize) {
      if (mSmst != NULL) {
        EFI_FREE_POOL (ValidBuffer);
      }
      return EFI_OUT_OF_RESOURCES;
    }

    CurrPtr = ValidBuffer + ValidBufferSize;
  }


  CopyMem (CurrPtr, NewVariable, NewVarSize);
  ((VARIABLE_HEADER *) CurrPtr)->State &= VAR_ADDED;
  CurrPtr += NewVarSize;
  //
  // If non-volatile variable store, perform FTW here.
  //
  Status = FtwVariableSpace (VariableBase, ValidBuffer);
  *LastVariableOffset = 0;
  if (!EFI_ERROR (Status)) {
    if (mSmst == NULL) {
      mVariableModuleGlobal->CommonUserVariableTotalSize = CommonUserVariableTotalSize;
    } else {
      mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->CommonUserVariableTotalSize = CommonUserVariableTotalSize;
    }
    *LastVariableOffset = (UINTN) (CurrPtr - ValidBuffer);
  }

  if (!EFI_ERROR (Status) && mVariableModuleGlobal->NonVolatileVariableCache != NULL) {
    SetMem (mVariableModuleGlobal->NonVolatileVariableCache, TotalBufferSize, 0xff);
    CopyMem (
      mVariableModuleGlobal->NonVolatileVariableCache,
      (VOID *) (UINTN) VariableBase,
      (UINTN) (CurrPtr - ValidBuffer)
      );
  } else if (mVariableModuleGlobal->NonVolatileVariableCache != NULL){
    //
    // reclaim failed, turn off variable chache to prevent from data corrupted
    //
    DisableVariableCache ();
  }
  if (mSmst != NULL) {
    EFI_FREE_POOL (ValidBuffer);
  }
  return Status;
}

/**
  Variable store garbage collection and reclaim operation.

  @param  VariableBase           Base address of variable store
  @param  LastVariableOffset     Offset of last variable
  @param  IsVolatile             The variable store is volatile or not,
                                 if it is non-volatile, need FTW
  @param  CurrentVairable        If it is not NULL, it means not to process
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
  IN       BOOLEAN               IsVolatile,
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

  VariableStoreHeader         = (VARIABLE_STORE_HEADER *) ((UINTN) VariableBase);
  Variable                    = GetStartPointer (VariableStoreHeader);
  CommonUserVariableTotalSize = 0;
  //
  // To make the reclaim, here we just allocate a memory that equal to the original memory
  //
  if (IsVolatile) {
    TotalBufferSize = GetVariableStoreSize (VariableStoreHeader);
  } else {
    TotalBufferSize = GetNonVolatileVariableStoreSize ();
  }
  ValidBuffer = NULL;
  if (mSmst == NULL) {
    ValidBuffer = (UINT8 *) mVariableReclaimInfo->BackupBuffer;
  } else {
    ValidBuffer = VariableAllocateZeroBuffer (TotalBufferSize, FALSE);
  }
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

  if (IsVolatile) {
    //
    // If volatile variable store, just copy valid buffer
    //
    SetMem ((UINT8 *) (UINTN) VariableBase, GetVariableStoreSize (VariableStoreHeader), 0xff);
    CopyMem ((UINT8 *) (UINTN) VariableBase, ValidBuffer, (UINTN) (CurrPtr - ValidBuffer));
    *LastVariableOffset = (UINTN) (CurrPtr - ValidBuffer);
    Status              = EFI_SUCCESS;
  } else {
    //
    // If non-volatile variable store, perform FTW here.
    //
    Status = FtwVariableSpace (VariableBase, ValidBuffer);
    if (!EFI_ERROR (Status)) {
      if (mSmst == NULL) {
        mVariableModuleGlobal->CommonUserVariableTotalSize = CommonUserVariableTotalSize;
      } else {
        mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->CommonUserVariableTotalSize = CommonUserVariableTotalSize;
      }
      *LastVariableOffset = (UINTN) (CurrPtr - ValidBuffer);
    }
  }

  if (EFI_ERROR (Status)) {
    *LastVariableOffset = 0;
  }
  if (!EFI_ERROR (Status) && CurrentVariable != NULL) {
    *CurrentVariable = WorkingVariable;
  }
  if (!EFI_ERROR (Status) && !IsVolatile && mVariableModuleGlobal->NonVolatileVariableCache != NULL) {
    SetMem (mVariableModuleGlobal->NonVolatileVariableCache, TotalBufferSize, 0xff);
    CopyMem (
      mVariableModuleGlobal->NonVolatileVariableCache,
      (VOID *) (UINTN) VariableBase,
      (UINTN) (CurrPtr - ValidBuffer)
      );
  } else if (!IsVolatile && mVariableModuleGlobal->NonVolatileVariableCache != NULL) {
    //
    // Turn off varialbe cache, if reclaim NV data failed
    //
    DisableVariableCache ();
  }

  if (mSmst != NULL) {
    EFI_FREE_POOL (ValidBuffer);
  }
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
  IN  CONST CHAR16                  *VariableName,
  IN  CONST EFI_GUID                *VendorGuid,
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

  CurrPtr   = (UINT8 *) ((UINTN) mVariableModuleGlobal->VariableBase.NonVolatileVariableBase);
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
  IN CONST CHAR16                 *VariableName,
  IN CONST EFI_GUID               *VendorGuid,
  IN CONST VOID                   *Data,
  IN       UINTN                  DataSize,
  IN       UINT32                 Attributes,
  IN       UINT32                 KeyIndex        OPTIONAL,
  IN       UINT64                 MonotonicCount  OPTIONAL,
  IN       VARIABLE_POINTER_TRACK *Variable,
  IN       EFI_TIME               *TimeStamp      OPTIONAL,
  IN       VARIABLE_GLOBAL        *Global
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
  CHAR8                               *CurrPtr;
  VARIABLE_HEADER                     *LastVariable;
  UINT8                               *WriteBuffer;
  UINTN                               TrySetVriableTime;
  BOOLEAN                             SetVariableSuccess;
  UINTN                               VariableCount;
  UINTN                               *VolatileLastVariableOffset;


  WriteBuffer       = NULL;

  // for Secure Boot variables, perform sanity check of incoming data since the structure is known to firmware
  //
  if (IsSecureDatabaseVariable (VariableName, VendorGuid)) {
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
    // Setting a data variable with no access, or zero DataSize attributes
    // specified causes it to be deleted.
    //
    if ((((Attributes & EFI_VARIABLE_APPEND_WRITE) == 0) && (DataSize == 0)) || ((Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == 0)) {
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
      if (Status == EFI_SUCCESS && !Volatile && IsUserVariable ((CHAR16 *) (Variable->CurrPtr + 1), &Variable->CurrPtr->VendorGuid)) {
        VarSize = ((UINTN) GetNextVariablePtr (Variable->CurrPtr) - (UINTN) Variable->CurrPtr);
        if (mSmst == NULL) {
          mVariableModuleGlobal->CommonUserVariableTotalSize -= VarSize;
        } else {
          mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->CommonUserVariableTotalSize -= VarSize;
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
      State = Variable->CurrPtr->State;
      State &= VAR_IN_DELETED_TRANSITION;

      Status = UpdateVariableStore (
                 Global,
                 Variable->Volatile,
                 FALSE,
                 (UINTN) &Variable->CurrPtr->State,
                 sizeof (UINT8),
                 &State
                 );
      if (EFI_ERROR (Status)) {
        goto Done;
      }
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
    // If variable cache is enabled, we should check varible data and variable variale is consistent before we write
    // to non-volatile storage.
    //
    if (mVariableModuleGlobal->NonVolatileVariableCache != NULL) {
      CurrPtr = (CHAR8*) ((UINTN) mVariableModuleGlobal->NonVolatileVariableCache);
      LastVariable = (VARIABLE_HEADER*) (CurrPtr + GetVariableStoreHeaderSize ());
      while (IsValidVariableHeader (LastVariable)) {
        LastVariable = GetNextVariablePtr (LastVariable);
      }
      //
      // Sync varaible cache and non-volatile variable, if the last variable offset in these two space isn't the same.
      //
      if (mVariableModuleGlobal->NonVolatileLastVariableOffset != (UINTN) LastVariable - (UINTN) CurrPtr) {
        ASSERT (FALSE);
        Status = FlushVariableCache (
                   mVariableModuleGlobal->NonVolatileVariableCache,
                   (UINT8 *) (UINTN) mVariableModuleGlobal->VariableBase.NonVolatileVariableBase,
                   mVariableModuleGlobal->NonVolatileVariableCacheSize,
                   &mVariableModuleGlobal->NonVolatileLastVariableOffset
                   );
        //
        // Try to find exist variable after flush variable cache to mark the exist variable
        //
        VariableCount = 0;
        Status = FindVariableByLifetime (VariableName, VendorGuid, Variable, &VariableCount, Global);
        if (Variable->CurrPtr != NULL && Variable->CurrPtr->State == VAR_ADDED) {
          State = (VAR_ADDED & VAR_IN_DELETED_TRANSITION);
          Status = UpdateVariableStore (
                     Global,
                     Variable->Volatile,
                     FALSE,
                     (UINTN) &Variable->CurrPtr->State,
                     sizeof (UINT8),
                     &State
                     );
          if (EFI_ERROR (Status)) {
            goto Done;
          }
        }
      }
    }
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
                 Global->NonVolatileVariableBase,
                 &mVariableModuleGlobal->NonVolatileLastVariableOffset
                 );
      goto Done;
    }
    //
    // Start try to set non-volatile variable and use do-while to
    // make sure at least set variable once.
    //
    TrySetVriableTime = 0;
    SetVariableSuccess = FALSE;

    //
    // Four steps
    // 1. Write variable header
    // 2. Set variable state to header valid
    // 3. Write variable data
    // 4. Set variable state to valid
    //
    //
    // Step 1:

    do {
      NextVariable->State = 0xFF;
      Status = UpdateVariableStore (
                 Global,
                 FALSE,
                 TRUE,
                 mVariableModuleGlobal->NonVolatileLastVariableOffset,
                 sizeof (VARIABLE_HEADER),
                 (UINT8 *) NextVariable
                 );

      if (EFI_ERROR (Status)) {
        goto Done;
      }

      //
      // Step 2:
      //
      NextVariable->State = VAR_HEADER_VALID_ONLY;
      Status = UpdateVariableStore (
                 Global,
                 FALSE,
                 TRUE,
                 mVariableModuleGlobal->NonVolatileLastVariableOffset + OFFSET_OF (VARIABLE_HEADER, State),
                 sizeof (UINT8),
                 &NextVariable->State
                 );

      if (EFI_ERROR (Status)) {
        goto Done;
      }
      //
      // Step 3:
      //
      Status = UpdateVariableStore (
                 Global,
                 FALSE,
                 TRUE,
                 mVariableModuleGlobal->NonVolatileLastVariableOffset + sizeof (VARIABLE_HEADER),
                 (UINT32) VarSize - sizeof (VARIABLE_HEADER),
                 (UINT8 *) NextVariable + sizeof (VARIABLE_HEADER)
                 );

      if (EFI_ERROR (Status)) {
        goto Done;
      }
      TrySetVriableTime++;
      //
      //if update SetVariableSuccess to TRUE
      //
      if (IsCorrectVariable (Global->NonVolatileVariableBase, mVariableModuleGlobal->NonVolatileLastVariableOffset, NextVariable)) {
        SetVariableSuccess = TRUE;
      } else {
        if (VariableAtRuntime () && !PcdGetBool(PcdRuntimeReclaimSupported)) {
          Status = EFI_OUT_OF_RESOURCES;
          goto Done;
        }
        NextVariable->State &= VAR_DELETED;
        Status = UpdateVariableStore (
                   Global,
                   FALSE,
                   TRUE,
                   mVariableModuleGlobal->NonVolatileLastVariableOffset + OFFSET_OF (VARIABLE_HEADER, State),
                   sizeof (UINT8),
                   &NextVariable->State
                   );
        //
        // reclaim non-volatile variable : skip this setting variable
        //
        Status = Reclaim (
                   Global->NonVolatileVariableBase,
                   &mVariableModuleGlobal->NonVolatileLastVariableOffset,
                   FALSE,
                   &Variable->CurrPtr
                   );
        if (EFI_ERROR (Status)) {
          goto Done;
        }
      }
    } while (!SetVariableSuccess  && (TrySetVriableTime < MAX_TRY_SET_VARIABLE_TIMES));

    if (!SetVariableSuccess) {
      Status = EFI_DEVICE_ERROR;
      goto Done;
    }
    //
    // Step 4:write the VAR_ADDED, until make sure data is correct
    // Set variable state to valid
    //
    NextVariable->State = VAR_ADDED;
    Status = UpdateVariableStore (
               Global,
               FALSE,
               TRUE,
               mVariableModuleGlobal->NonVolatileLastVariableOffset + OFFSET_OF (VARIABLE_HEADER, State),
               sizeof (UINT8),
               &NextVariable->State
               );

    if (EFI_ERROR (Status)) {
      goto Done;
    }

    mVariableModuleGlobal->NonVolatileLastVariableOffset += (VarSize);
    if (IsUserVariable ((CHAR16 *) (NextVariable + 1), &NextVariable->VendorGuid)) {
      if (mSmst == NULL) {
        mVariableModuleGlobal->CommonUserVariableTotalSize += VarSize;
      } else {
        mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->CommonUserVariableTotalSize += VarSize;
      }
    }
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
                 TRUE,
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
    if (Status == EFI_SUCCESS && !Variable->Volatile && IsUserVariable ((CHAR16 *) (Variable->CurrPtr + 1), &Variable->CurrPtr->VendorGuid)) {
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
  BOOLEAN                 InteruptEnabled;

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

  InteruptEnabled = GetInterruptState ();
  if (VariableAtRuntime () && InteruptEnabled) {
    DisableInterrupts ();
  }
  if (VariableAtRuntime ()) {
    WriteBackInvalidateDataCache ();
  }
  AcquireLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);

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
  ReleaseLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
  if (VariableAtRuntime () && InteruptEnabled) {
    EnableInterrupts ();
  }
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
  AcquireLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);

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
  ReleaseLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
  if (VariableAtRuntime () && InteruptEnabled) {
    EnableInterrupts ();
  }
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

  AcquireLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
  if (VariableAtRuntime ()) {
    WriteBackInvalidateDataCache ();
  }

  if (Global->ReentrantState != 0) {
    ReleaseLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
    return EFI_ACCESS_DENIED;
  }

  Global->ReentrantState++;

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
  if (mReadyToBootEventSignaled && mSmst == NULL && mVariableModuleGlobal->SmmCodeReady &&
      (Attributes == (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS) ||
       Attributes == (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS))) {
    Status = FindVariable (
               VariableName,
               VendorGuid,
               &Variable,
               &VariableCount,
               Global
               );
    if (!EFI_ERROR (Status) && Variable.CurrPtr->Attributes == Attributes && DataSizeOfVariable (Variable.CurrPtr) == DataSize &&
        (CompareMem (Data, GetVariableDataPtr (Variable.CurrPtr), DataSize) == 0)) {
      Status = EFI_SUCCESS;
      goto Done;
    }
  }
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
  ReleaseLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
  return Status;
}


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
  VARIABLE_HEADER        *Variable;
  VARIABLE_HEADER        *NextVariable;
  VARIABLE_HEADER        *VariableEndPointer;
  UINT64                 VariableSize;
  VARIABLE_STORE_HEADER  *VariableStoreHeader;
  VARIABLE_GLOBAL        *Global;
  UINTN                  VariableStoreSize;
  BOOLEAN                InteruptEnabled;

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

  InteruptEnabled = GetInterruptState ();
  if (VariableAtRuntime () && InteruptEnabled) {
    DisableInterrupts ();
  }
  if (VariableAtRuntime ()) {
    WriteBackInvalidateDataCache ();
  }
  AcquireLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);

  Global = &mVariableModuleGlobal->VariableBase;
  if((Attributes & EFI_VARIABLE_NON_VOLATILE) == 0) {
    //
    // Query is Volatile related.
    //
    VariableStoreHeader = (VARIABLE_STORE_HEADER *) ((UINTN) Global->VolatileVariableBase);
    VariableEndPointer = GetEndPointer (VariableStoreHeader);
    VariableStoreSize = GetVariableStoreSize (VariableStoreHeader);
  } else {
    //
    // Query is Non-Volatile related.
    //
    VariableStoreHeader = (VARIABLE_STORE_HEADER *) ((UINTN) Global->NonVolatileVariableBase);
    VariableEndPointer = GetNonVolatileEndPointer (VariableStoreHeader);
    VariableStoreSize = GetNonVolatileVariableStoreSize ();
  }

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

  //
  // Point to the starting address of the variables.
  //
  Variable = GetStartPointer (VariableStoreHeader);

  //
  // Now walk through the related variable store.
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

  if (*RemainingVariableStorageSize < sizeof (VARIABLE_HEADER)) {
    *MaximumVariableSize = 0;
  } else if ((*RemainingVariableStorageSize - sizeof (VARIABLE_HEADER)) < *MaximumVariableSize) {
    *MaximumVariableSize = *RemainingVariableStorageSize - sizeof (VARIABLE_HEADER);
  }
  ReleaseLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
  if (VariableAtRuntime () && InteruptEnabled) {
    EnableInterrupts ();
  }
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
  EFI_VARIABLE_DEFAULT_UPDATE_PROTOCOL      *VariableDefaultUpdateProtocol;
  EFI_STATUS                                Status;

  Status = gBS->LocateProtocol (
                  &gEfiVariableDefaultUpdateProtocolGuid,
                  NULL,
                  (VOID **)&VariableDefaultUpdateProtocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = VariableDefaultUpdateProtocol->UpdateFactorySetting (VariableDefaultUpdateProtocol, RESTORE_WITH_RESERVING_OTHER_SETTINGS);
  //
  // Update factory setting will cause variable data and variable inconsistent, so disable variable cache.
  //
  DisableVariableCache ();
  //
  // Clear "RestoreFactoryDefault" to 0 to prevent from system always restoring factory default during POST.
  //
  UpdateRestoreFactoryDefaultVariable (0);

  if (!EFI_ERROR (Status)) {
    gST->RuntimeServices->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
  }

  return Status;
}


EFI_STATUS
EFIAPI
DummyUpdateFactorySetting (
  IN      EFI_VARIABLE_DEFAULT_UPDATE_PROTOCOL     *This,
  IN      UINT32                                   RestoreType
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
DummyUpdateBackupSetting (
  IN      EFI_VARIABLE_DEFAULT_UPDATE_PROTOCOL     *This,
  IN      EFI_DEVICE_PATH_PROTOCOL                 *DevicePath,
  IN      UINT32                                   RestoreType
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Replace all of restore default functions which doesn't use to dummy function.
  This action can prevent from malware uses these functions.

  @return EFI_NOT_FOUND   gEfiVariableDefaultUpdateProtocolGuid doesn't exit.
  @return EFI_SUCCESS     Udpate restore default functions successful.

--*/
EFI_STATUS
UpdateRestoreVariableDefaultProtocol (
  VOID
  )
{
  EFI_STATUS                                  Status;
  UINTN                                       NumberOfHandles;
  EFI_HANDLE                                  *Handles;
  EFI_VARIABLE_DEFAULT_UPDATE_PROTOCOL        *OldVariableDefaultUpdateProtocol;
  EFI_VARIABLE_DEFAULT_UPDATE_PROTOCOL        *NewVariableDefaultUpdateProtocol;


  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiVariableDefaultUpdateProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &Handles
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  ASSERT (NumberOfHandles == 1);

  Status = gBS->HandleProtocol (
                  Handles[0],
                  &gEfiVariableDefaultUpdateProtocolGuid,
                  (VOID **)&OldVariableDefaultUpdateProtocol
                  );

  ASSERT_EFI_ERROR (Status);

  NewVariableDefaultUpdateProtocol = VariableAllocateZeroBuffer (sizeof (EFI_VARIABLE_DEFAULT_UPDATE_PROTOCOL), FALSE);
  if (NewVariableDefaultUpdateProtocol == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (NewVariableDefaultUpdateProtocol, OldVariableDefaultUpdateProtocol, sizeof (EFI_VARIABLE_DEFAULT_UPDATE_PROTOCOL));
  NewVariableDefaultUpdateProtocol->UpdateFactorySetting = DummyUpdateFactorySetting;
  if (!IsAdministerSecureBootSupport ()) {
    NewVariableDefaultUpdateProtocol->UpdateBackupSetting = DummyUpdateBackupSetting;
  }

  Status = gBS->ReinstallProtocolInterface (
                  Handles[0],
                  &gEfiVariableDefaultUpdateProtocolGuid,
                  OldVariableDefaultUpdateProtocol,
                  NewVariableDefaultUpdateProtocol
                  );

  return Status;

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
  Callback function for gEfiVariableDefaultUpdateProtocolGuid protocol

  @param Event    Event whose notification function is being invoked.
  @param Context  Pointer to the notification function's context.

**/
VOID
EFIAPI
DefaultUpdateCallbackFunction (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )

{
  if (Event != NULL) {
    gBS->CloseEvent (Event);
  }

  if (NeedRestoreFactoryDefault ()) {
    RestoreFactoryDefault ();
  }

  UpdateRestoreVariableDefaultProtocol ();

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
    &mVariableModuleGlobal->VariableBase.NonVolatileVariableBase,
    &mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->VariableBase.NonVolatileVariableBase,
    sizeof (EFI_PHYSICAL_ADDRESS)
    );
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
  Status = GetVariableStoreInfo (
             mVariableModuleGlobal->VariableBase.NonVolatileVariableBase,
             &(mSmmVariableGlobal->VariableStoreInfo.FvbBaseAddress),
             &(mSmmVariableGlobal->VariableStoreInfo.Lba),
             &(mSmmVariableGlobal->VariableStoreInfo.Offset)
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
    Status = Reclaim (
              mVariableModuleGlobal->VariableBase.NonVolatileVariableBase,
              &mVariableModuleGlobal->NonVolatileLastVariableOffset,
              FALSE,
              NULL
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

  DisableVariableCache ();
  if (mVariableModuleGlobal->SmmCodeReady && mSmst == NULL) {
    Signature = SMM_LEGACY_BOOT_SIGNATURE;
    CopyMem (mSmmPhyVarBuf, &Signature, sizeof (Signature));
    SmmSecureBootCall ((UINT8 *) mSmmPhyVarBuf, sizeof (Signature), LEGACY_BOOT_SMI_FUN_NUM, SW_SMI_PORT);
  }
  gBS->CloseEvent (Event);
  return;
}



/**
  This function uses to restore default variaboe store header to non-volatile sotrage

  @param VariableStoreHeader     pointer the variable store header in non-volatile sotrage.

  @retval EFI_SUCCESS            The restore process is success.
  @retval EFI_INVALID_PARAMETER  VariableStoreHeader parameter is NULL.
  @return other                  Write data to non-volatile storage failed.

--*/
EFI_STATUS
RestoreDefaultVariableStoreHeader (
  VARIABLE_STORE_HEADER                 *VariableStoreHeader
  )
{
  VARIABLE_STORE_HEADER         WorkingStoreHeader;
  ECP_VARIABLE_STORE_HEADER     EcpVarStoreHeader;

  if (VariableStoreHeader == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (PcdGetBool (PcdUseEcpVariableStoreHeader)) {
    SetMem (&EcpVarStoreHeader, sizeof (EcpVarStoreHeader), 0xFF);
    EcpVarStoreHeader.Signature = ECP_VARIABLE_STORE_SIGNATURE;
    EcpVarStoreHeader.Format    = VARIABLE_STORE_FORMATTED;
    EcpVarStoreHeader.State     = VARIABLE_STORE_HEALTHY;
    EcpVarStoreHeader.Reserved  = 0;
    EcpVarStoreHeader.Reserved1 = 0;
    return UpdateVariableStore (
             &mVariableModuleGlobal->VariableBase,
             FALSE,
             FALSE,
             (UINTN) VariableStoreHeader,
             sizeof (EcpVarStoreHeader),
             (UINT8 *) &EcpVarStoreHeader
             );
  } else {
    SetMem (&WorkingStoreHeader, sizeof (WorkingStoreHeader), 0xFF);
    CopyMem (
      &WorkingStoreHeader.Signature,
      &gEfiAuthenticatedVariableGuid,
      sizeof (EFI_GUID)
      );
    WorkingStoreHeader.Format    = VARIABLE_STORE_FORMATTED;
    WorkingStoreHeader.State     = VARIABLE_STORE_HEALTHY;
    WorkingStoreHeader.Reserved  = 0;
    WorkingStoreHeader.Reserved1 = 0;
    return UpdateVariableStore (
             &mVariableModuleGlobal->VariableBase,
             FALSE,
             FALSE,
             (UINTN) VariableStoreHeader,
             sizeof (WorkingStoreHeader),
             (UINT8 *) &WorkingStoreHeader
             );
  }
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
 Check the integrity of firmware volume header.

 @param[in] FwVolHeader  A pointer to a firmware volume header.

 @retval TRUE     The firmware volume is consistent.
 @retval FALSE    The firmware volume has corrupted. So it is not an FV.
**/
STATIC
BOOLEAN
ValidateFvHeader (
  IN EFI_FIRMWARE_VOLUME_HEADER            *FwVolHeader
  )
{
  //
  // Verify the header revision, header signature, length
  // Length of FvBlock cannot be 2**64-1
  // HeaderLength cannot be an odd number
  //
  if ((FwVolHeader->Revision != EFI_FVH_REVISION) ||
      (FwVolHeader->Signature != EFI_FVH_SIGNATURE) ||
      (FwVolHeader->FvLength == ((UINTN) -1)) ||
      ((FwVolHeader->HeaderLength & 0x01) != 0)) {
    return FALSE;
  }

  //
  // Verify the header checksum
  //
  if (CalculateCheckSum16 ((UINT16 *) FwVolHeader, FwVolHeader->HeaderLength) != 0) {
    return FALSE;
  }

  return TRUE;
}


/**
 Internal function to initialize variable information base address.

 @retval EFI_SUCCESS           Init variable information related base address successfully.
 @retval EFI_NOT_READY         firmware volume to store variable data is corrupted.
 @retval EFI_BUFFER_TOO_SMALL  Unable flush variable data to variable cache because
                               all of variable data size is larger than variable cache size.
 @retval EFI_OUT_OF_RESOURCES  There are not enough memory to allocate.
 @return Other                 Other error occurred in this function.
**/
EFI_STATUS
InitVariableBaseAddress (
  VOID
  )
{
  EFI_FIRMWARE_VOLUME_HEADER            *FvHeader;
  VARIABLE_STORE_HEADER                 *VariableStoreHeader;
  UINT64                                VariableStoreLength;
  EFI_STATUS                            Status;
  VARIABLE_STORE_HEADER                 *VolatileVariableStore;

  //
  // memory address has been initialized so return directly.
  //
  if (mVariableModuleGlobal->VariableBase.NonVolatileVariableBase != 0) {
    return EFI_SUCCESS;
  }
  //
  // firmware volume header is invalid return directly. expected this function will be invoked
  // after restoring correct firmware volume header.
  //
  FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_WORKING, 1);
  if (!ValidateFvHeader (FvHeader)) {
    return EFI_NOT_READY;
  }

  VariableStoreHeader = (VARIABLE_STORE_HEADER *) ((UINT8 *) FvHeader + FvHeader->HeaderLength);

  mVariableModuleGlobal->VariableBase.NonVolatileVariableBase = (EFI_PHYSICAL_ADDRESS) (UINTN) VariableStoreHeader;
  VariableStoreLength =  FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_WORKING, 1) - FvHeader->HeaderLength;

  Status = SetRuntimeMemoryAttribute (mVariableModuleGlobal->VariableBase.NonVolatileVariableBase, VariableStoreLength);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Since memory for non-volatile variable cache and volatile variable may be modified after start image, so combine these two
  // memory and save the start address and length in configuration table for other driver use.
  //
  VolatileVariableStore = VariableAllocateZeroBuffer (VARIABLE_STORE_SIZE + SCRATCH_SIZE + (UINTN) VariableStoreLength, TRUE);
  if (VolatileVariableStore == NULL) {
    POST_CODE (DXE_VARIABLE_INIT_FAIL);
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  Status = BuildVariableWorkingRegionRecord (
           (EFI_PHYSICAL_ADDRESS) (UINTN) VolatileVariableStore,
           (EFI_PHYSICAL_ADDRESS) (UINTN) (VARIABLE_STORE_SIZE + SCRATCH_SIZE + (UINTN) VariableStoreLength)
           );
  if (EFI_ERROR (Status)) {
    goto Done;
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
  mVariableModuleGlobal->NonVolatileVariableCache = (UINT8 *) VolatileVariableStore + VARIABLE_STORE_SIZE + SCRATCH_SIZE;
  mVariableModuleGlobal->NonVolatileVariableCacheSize = (UINTN) VariableStoreLength;

  Status = FlushVariableCache (
             mVariableModuleGlobal->NonVolatileVariableCache,
             (UINT8 *) (UINTN) mVariableModuleGlobal->VariableBase.NonVolatileVariableBase,
             mVariableModuleGlobal->NonVolatileVariableCacheSize,
             &mVariableModuleGlobal->NonVolatileLastVariableOffset
             );
Done:
  if (Status != EFI_SUCCESS && Status != EFI_BUFFER_TOO_SMALL) {
    mVariableModuleGlobal->VariableBase.NonVolatileVariableBase = 0;
  }
  return Status;
}

/**
  Initializes read-only variable services.

  @retval EFI_SUCCESS           Function successfully executed.
  @retval EFI_OUT_OF_RESOURCES  Fail to allocate enough memory resource.
  @return Others                Ohter error occurred in this function.
**/
EFI_STATUS
VariableReadyOnlyInitialize (
  VOID
  )
{
  EFI_STATUS               Status;

  if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
    return EFI_UNSUPPORTED;
  }

  mVariableModuleGlobal = VariableAllocateZeroBuffer (sizeof (ESAL_VARIABLE_GLOBAL), TRUE);
  if (mVariableModuleGlobal == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  EfiInitializeLock (&mVariableModuleGlobal->VariableBase.VariableServicesLock, TPL_NOTIFY);

  mVariableModuleGlobal->FactoryDefaultBase = FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);
  mVariableModuleGlobal->FactoryDefaultSize = (UINTN) FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);
  Status = SetRuntimeMemoryAttribute (mVariableModuleGlobal->FactoryDefaultBase, (UINT64) mVariableModuleGlobal->FactoryDefaultSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  InitVariableBaseAddress ();

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
  EFI_STATUS                            InfoStatus;
  VARIABLE_STORE_HEADER                 *VariableStoreHeader;
  UINT64                                VariableStoreLength;
  EFI_HANDLE                            NewHandle;
  EFI_EVENT                             Event;
  EFI_VARIABLE_DEFAULT_UPDATE_PROTOCOL  *VariableDefaultUpdateProtocol;
  VOID                                  *Registration;
  EFI_FIRMWARE_VOLUME_HEADER            *FvHeader;
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL     *FvbProtocol;
  EFI_PHYSICAL_ADDRESS                   FvBaseAddress;
  UINT32                                 MaxUserNvVariableSpaceSize;

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

  FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_WORKING, 1);
  VariableStoreHeader = (VARIABLE_STORE_HEADER *) ((UINT8 *) FvHeader + FvHeader->HeaderLength);
  VariableStoreLength =  FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_WORKING, 1) - FvHeader->HeaderLength;
  Status = GetFvbInfoByAddress ((EFI_PHYSICAL_ADDRESS) (UINTN) FvHeader, NULL, &FvbProtocol);
  if (EFI_ERROR (Status)) {
    goto Shutdown;
  }
  mVariableModuleGlobal->FvbInstance = FvbProtocol;

  //
  // To prevent from variable store header is corrupted (may be caused by other tool),
  // restore to default variable store header to make sure variable service can work
  // properly.
  //
  if (GetVariableStoreStatus (VariableStoreHeader) != EfiValid) {
    Status = RestoreDefaultVariableStoreHeader (VariableStoreHeader);
    if (EFI_ERROR (Status) || GetVariableStoreStatus (VariableStoreHeader) != EfiValid) {
      goto Shutdown;
    }
  }

  if (~GetVariableStoreSize (VariableStoreHeader) == 0) {
    Status = UpdateVariableStore (
              &mVariableModuleGlobal->VariableBase,
              FALSE,
              FALSE,
              PcdGetBool (PcdUseEcpVariableStoreHeader) ? (UINTN) &((ECP_VARIABLE_STORE_HEADER *) VariableStoreHeader)->Size : (UINTN) &VariableStoreHeader->Size,
              sizeof (UINT32),
              (UINT8 *) &VariableStoreLength
              );

    if (EFI_ERROR (Status)) {
      goto Shutdown;
    }
  }

  InitializeInsydeVariableLockedState ();

  MaxUserNvVariableSpaceSize = PcdGet32 (PcdMaxUserNvVariableSpaceSize);
  ASSERT (MaxUserNvVariableSpaceSize < (VariableStoreLength - sizeof (VARIABLE_STORE_HEADER)));
  mVariableModuleGlobal->CommonMaxUserVariableSpace = MaxUserNvVariableSpaceSize;

  //
  // Allocate reclaim related memory before doing reclaim
  //
  mVariableReclaimInfo = VariableAllocateZeroBuffer (sizeof (VARIALBE_RECLAIM_INFO), TRUE);
  if (mVariableReclaimInfo == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Shutdown;
  }
  Status = gBS->LocateProtocol (
                  &gEfiFaultTolerantWriteLiteProtocolGuid,
                  NULL,
                  (VOID **)&mVariableReclaimInfo->FtwLiteProtocol
                  );
  if (EFI_ERROR (Status)) {
    goto Shutdown;
  }

  Status = InitVariableBaseAddress ();
  if (Status != EFI_SUCCESS && Status != EFI_BUFFER_TOO_SMALL) {
    goto Shutdown;
  }

  mVariableReclaimInfo->BackupBuffer = VariableAllocateZeroBuffer (GetNonVolatileVariableStoreSize () + GetVariableStoreHeaderSize (), TRUE);
  if (mVariableReclaimInfo->BackupBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Shutdown;
  }

  InfoStatus = GetVariableStoreInfo (
                 mVariableModuleGlobal->VariableBase.NonVolatileVariableBase,
                 &FvBaseAddress,
                 &mVariableReclaimInfo->VariableLba,
                 &mVariableReclaimInfo->VariableOffset
                 );
  if (InfoStatus != EFI_SUCCESS) {
    goto Shutdown;
  }
  if (Status == EFI_BUFFER_TOO_SMALL) {
    ASSERT (FALSE);

    //
    // If variable data size is large than cache size, there are something wrong in variable data.
    // Try to use reclaim mechanism to fix this error.
    //
    Status = Reclaim (
               mVariableModuleGlobal->VariableBase.NonVolatileVariableBase,
               &mVariableModuleGlobal->NonVolatileLastVariableOffset,
               FALSE,
               NULL
               );
    if (EFI_ERROR (Status)) {
      goto Shutdown;
    }
    Status = FlushVariableCache (
               mVariableModuleGlobal->NonVolatileVariableCache,
               (UINT8 *) (UINTN) mVariableModuleGlobal->VariableBase.NonVolatileVariableBase,
               mVariableModuleGlobal->NonVolatileVariableCacheSize,
               &mVariableModuleGlobal->NonVolatileLastVariableOffset
               );
  }

  if (EFI_ERROR (Status)) {
    goto Shutdown;
  }

  //
  // Reclaim if the free area is blow a threshold to release some non-volatile space
  //
  if (GetNonVolatileVariableStoreSize () - mVariableModuleGlobal->NonVolatileLastVariableOffset < VARIABLE_RECLAIM_THRESHOLD) {
    Status = Reclaim (
               mVariableModuleGlobal->VariableBase.NonVolatileVariableBase,
               &mVariableModuleGlobal->NonVolatileLastVariableOffset,
               FALSE,
               NULL
               );
    if (EFI_ERROR (Status)) {
      goto Shutdown;
    }
  }
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

  Status = gBS->LocateProtocol (&gEfiVariableDefaultUpdateProtocolGuid, NULL, (VOID **)&VariableDefaultUpdateProtocol);
  if (!EFI_ERROR (Status)) {
    DefaultUpdateCallbackFunction (NULL, NULL);
  } else {
    Event = VariableCreateProtocolNotifyEvent (
              &gEfiVariableDefaultUpdateProtocolGuid,
              TPL_CALLBACK,
              DefaultUpdateCallbackFunction,
              NULL,
              &Registration
              );
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

  if (mVariableReclaimInfo != NULL) {
    if (mVariableReclaimInfo->BackupBuffer != NULL) {
      EFI_FREE_POOL (mVariableReclaimInfo->BackupBuffer);
    }
    EFI_FREE_POOL (mVariableReclaimInfo);
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
  This fucnitons uses to flush current non-volatile data to variable cache

  @param  CacheBaseAdress         pointer to variable cache base address.
  @param  NonVolatileBaseAddress  pointer to non-volatile base address.
  @param  CacheSize               Variable cache size.
  @param  LastVariableOffset      Pointer to save last variable offset.

  @return EFI_SUCCESS             Flush non-volatile data to variable cache successful.
  @return EFI_INVALID_PARAMETER   CacheBaseAdress or NonVolatileBaseAddress is NULL.
  @return EFI_BUFFER_TOO_SMALL    CacheSize is too small.

**/
EFI_STATUS
FlushVariableCache (
  IN  UINT8    *CacheBaseAdress,
  IN  UINT8    *NonVolatileBaseAddress,
  IN  UINTN    CacheSize,
  OUT UINTN    *LastVariableOffset
  )
{
  VARIABLE_HEADER                       *NextVariable;

  if (CacheBaseAdress == NULL || NonVolatileBaseAddress == NULL || LastVariableOffset == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  NextVariable  = (VARIABLE_HEADER *) (NonVolatileBaseAddress + GetVariableStoreHeaderSize ());

  while (IsValidVariableHeader (NextVariable)) {
    NextVariable = GetNextVariablePtr (NextVariable);
  }

  *LastVariableOffset = (UINTN) NextVariable - (UINTN) NonVolatileBaseAddress;
  if (CacheSize < *LastVariableOffset) {

    return  EFI_BUFFER_TOO_SMALL;
  }

  SetMem (CacheBaseAdress, CacheSize, 0xff);
  CopyMem (CacheBaseAdress, NonVolatileBaseAddress, *LastVariableOffset);
  return EFI_SUCCESS;
}

/**
  Check this input handle is whether a NV store FVB handle.

  @param[in] Handle    Input EFI_HANDLE instance

  @retval TRUE         This is NV storage FVB handle.
  @retval FALSE        This isn't NV storage FVB handle.
**/
STATIC
BOOLEAN
IsNvStorageHandle (
  EFI_HANDLE      Handle
  )
{
  EFI_STATUS   Status;

  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiFirmwareVolumeBlockProtocolGuid,
                  NULL,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }
  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiAlternateFvBlockGuid,
                  NULL,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  return (BOOLEAN) (Status == EFI_SUCCESS);
}

/**
  Get the proper fvb handle and/or fvb protocol by the given Flash address.

  @param  Address        The Flash address.
  @param  FvbHandle      In output, if it is not NULL, it points to the proper FVB handle.
  @param  FvbProtocol    In output, if it is not NULL, it points to the proper FVB protocol.

**/
EFI_STATUS
GetFvbInfoByAddress (
  IN  EFI_PHYSICAL_ADDRESS                Address,
  OUT EFI_HANDLE                          *FvbHandle OPTIONAL,
  OUT EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  **FvbProtocol OPTIONAL
  )
{
  EFI_STATUS                              Status;
  EFI_HANDLE                              *HandleBuffer;
  UINTN                                   HandleCount;
  UINTN                                   Index;
  EFI_PHYSICAL_ADDRESS                    FvbBaseAddress;
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL      *Fvb;
  EFI_FVB_ATTRIBUTES_2                    Attributes;
  UINTN                                   BlockSize;
  UINTN                                   NumberOfBlocks;


  //
  // Get all FVB handles.
  //
  Status = GetFvbCountAndBuffer (&HandleCount, &HandleBuffer);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  //
  // Get the FVB to access variable store.
  //
  Fvb = NULL;
  for (Index = 0; Index < HandleCount; Index += 1, Status = EFI_NOT_FOUND, Fvb = NULL) {
    Status = GetFvbByHandle (HandleBuffer[Index], &Fvb);
    if (EFI_ERROR (Status)) {
      Status = EFI_NOT_FOUND;
      break;
    }

    //
    // Ensure this FVB protocol supported Write operation.
    //
    Status = Fvb->GetAttributes (Fvb, &Attributes);
    if (EFI_ERROR (Status) || ((Attributes & EFI_FVB2_WRITE_STATUS) == 0)) {
      continue;
    }

    //
    // Compare the address and select the right one.
    //
    Status = Fvb->GetPhysicalAddress (Fvb, &FvbBaseAddress);
    if (EFI_ERROR (Status)) {
      continue;
    }
    //
    // Assume one FVB has one type of BlockSize.
    //
    Status = Fvb->GetBlockSize (Fvb, 0, &BlockSize, &NumberOfBlocks);
    if (EFI_ERROR (Status)) {
      continue;
    }

    if ((Address >= FvbBaseAddress) && (Address < (FvbBaseAddress + BlockSize * NumberOfBlocks)) && IsNvStorageHandle (HandleBuffer[Index])) {
      if (FvbHandle != NULL) {
        *FvbHandle  = HandleBuffer[Index];
      }
      if (FvbProtocol != NULL) {
        *FvbProtocol = Fvb;
      }
      Status = EFI_SUCCESS;
      break;
    }
  }
  FreePool (HandleBuffer);

  if (Fvb == NULL) {
    Status = EFI_NOT_FOUND;
  }

  return Status;
}

/**
  Function returns an array of handles that support the FVB protocol
  in a buffer allocated from pool.

  @param  NumberHandles         The number of handles returned in Buffer.
  @param  Buffer                A pointer to the buffer to return the requested
                                array of  handles that support FVB protocol.

  @retval EFI_SUCCESS           The array of handles was returned in Buffer, and the number of
                                handles in Buffer was returned in NumberHandles.
  @retval EFI_NOT_FOUND         No FVB handle was found.
  @retval EFI_OUT_OF_RESOURCES  There is not enough pool memory to store the matching results.
  @retval EFI_INVALID_PARAMETER NumberHandles is NULL or Buffer is NULL.

**/
EFI_STATUS
GetFvbCountAndBuffer (
  OUT UINTN                               *NumberHandles,
  OUT EFI_HANDLE                          **Buffer
  )
{
  EFI_STATUS                              Status;

  //
  // Locate all handles of Fvb protocol
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolumeBlockProtocolGuid,
                  NULL,
                  NumberHandles,
                  Buffer
                  );
  return Status;
}

/**
  Retrive the FVB protocol interface by HANDLE.

  @param  FvBlockHandle         The handle of FVB protocol that provides services for
                                reading, writing, and erasing the target block.
  @param  FvBlock               The interface of FVB protocol

  @retval EFI_SUCCESS           The interface information for the specified protocol was returned.
  @retval EFI_UNSUPPORTED       The device does not support the FVB protocol.
  @retval EFI_INVALID_PARAMETER FvBlockHandle is not a valid EFI_HANDLE or FvBlock is NULL.

**/
EFI_STATUS
GetFvbByHandle (
  IN  EFI_HANDLE                          FvBlockHandle,
  OUT EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  **FvBlock
  )
{
  //
  // To get the FVB protocol interface on the handle
  //
  return gBS->HandleProtocol (
                FvBlockHandle,
                &gEfiFirmwareVolumeBlockProtocolGuid,
                (VOID **) FvBlock
                );
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
  //
  // convert relative base addresses or pointers
  //
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->VariableBase.NonVolatileVariableBase);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->VariableBase.VolatileVariableBase);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->NonVolatileVariableCache);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->FactoryDefaultBase);
  //
  // Convert Fvb relative function pointers
  //
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->FvbInstance->GetBlockSize);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->FvbInstance->GetPhysicalAddress);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->FvbInstance->GetAttributes);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->FvbInstance->SetAttributes);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->FvbInstance->Read);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->FvbInstance->Write);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->FvbInstance);

  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->GlobalVariableList);
  PreservedTableAddressChange ();
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal);
  //
  // convert reaclaim relative pointer
  //
  gRT->ConvertPointer (0x0, (VOID **) &mVariableReclaimInfo->BackupBuffer);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableReclaimInfo->FtwLiteProtocol);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableReclaimInfo);

  gRT->ConvertPointer (0x0, (VOID **) &mSmmVarBuf);

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
  gBS->CloseEvent (Event);
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
  Convert all of preserved variable table relative pointers to virtual address.

**/
VOID
PreservedTableAddressChange (
  VOID
  )
{
  UINTN Index;

  if (mVariableModuleGlobal->PreservedVariableTable == NULL) {
    return;
  }

  for (Index = 0; mVariableModuleGlobal->PreservedVariableTable[Index].VariableName != NULL; Index++) {
    gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->PreservedVariableTable[Index].VariableName);
  }

  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->PreservedVariableTable);

}

/**
  Check the variable is whether in the preserved variable table or not.

  @param[In]    Variable        pointer to the variable

  @retval TRUE   the variable is in the preserved variable table
  @retval FALSE  the variable is not in the preserved variable table

**/
BOOLEAN
IsVarialbeInKeepList (
  VARIABLE_HEADER               *Variable
  )
{
  UINTN Index;

  for (Index = 0; mVariableModuleGlobal->PreservedVariableTable[Index].VariableName != NULL; Index++) {
    if ((CompareGuid (&Variable->VendorGuid, &mVariableModuleGlobal->PreservedVariableTable[Index].VendorGuid)) &&
        (StrCmp (GET_VARIABLE_NAME_PTR (Variable), mVariableModuleGlobal->PreservedVariableTable[Index].VariableName) == 0)) {
      return TRUE;
    }
  }

  return FALSE;
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
  This fucniton elimates all the variables except authenticated variables and
  the variables form mVariableModuleGlobal->PreservedVariableTable.

  @param[In]   ValidBuffer        pointer to the valid variable buffer
  @param[In]   TotalBufferSize    the total size of the varible region
  @param[Out]  ValidVariableSize  pointer to the size of the valid variable

**/
VOID
ElimateVariableWhenRegionFull (
  IN  UINT8                     *ValidBuffer,
  IN  UINTN                     TotalBufferSize,
  OUT UINTN                     *ValidVariableSize
  )
{
  VARIABLE_HEADER               *Variable;
  VARIABLE_HEADER               *NextVariable;
  UINTN                         VariableSize;
  UINT8                         *ValidVariablePtr;


  if (mVariableModuleGlobal->PreservedVariableTable == NULL) {
    return;
  }

  Variable = (VARIABLE_HEADER *) (ValidBuffer + GetVariableStoreHeaderSize ());

  ValidVariablePtr = (UINT8 *) Variable;
  VariableSize = 0;
  while (IsValidVariableHeader (Variable)) {
    NextVariable = GetNextVariablePtr (Variable);
    VariableSize = (UINTN) NextVariable - (UINTN) Variable;

    if (IsVarialbeInKeepList (Variable) || IsAuthenticatedVariable (Variable)) {
      if (ValidVariablePtr != (UINT8 *) Variable) {
        CopyMem (ValidVariablePtr, Variable, VariableSize);
      }

      ValidVariablePtr = ValidVariablePtr + VariableSize;
    }

    Variable = NextVariable;
  }

  *ValidVariableSize = (UINTN) (ValidVariablePtr - ValidBuffer);
  SetMem (ValidVariablePtr, TotalBufferSize - *ValidVariableSize, 0xFF);
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
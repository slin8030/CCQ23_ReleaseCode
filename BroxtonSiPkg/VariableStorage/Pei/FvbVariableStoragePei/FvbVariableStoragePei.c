/** @file
  This PEIM implements Firmware Volume Block Variable Storage Services
  and installs an instance of the VariableStorage PPI.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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

#include "FvbVariableStoragePei.h"

//
// Module globals
//
static VARIABLE_STORAGE_PPI mFvbVariableStoragePpi = {
  PeiFvbVariableStorageGetId,
  PeiFvbVariableStorageGetVariable,
  PeiFvbVariableStorageGetNextVariableName
};

static EFI_PEI_PPI_DESCRIPTOR     mPpiListVariable = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gVariableStoragePpiGuid,
  &mFvbVariableStoragePpi
};

/**
  Provide the functionality of FVB variable storage services.

  @param  FileHandle   Handle of the file being invoked.
                       Type EFI_PEI_FILE_HANDLE is defined in FfsFindNextFile().
  @param  PeiServices  General purpose services available to every PEIM.

  @retval EFI_SUCCESS  If the interface could be successfully installed
  @retval Others       Returned from PeiServicesInstallPpi()
**/
EFI_STATUS
EFIAPI
PeimInitializeFvbVariableStorageServices (
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
{
  BOOLEAN                 ImageInPermanentMemory;
  UINTN                   Instance;
  EFI_GUID                DiscoveredInstanceGuid;
  EFI_STATUS              Status;
  EFI_PEI_PPI_DESCRIPTOR  *VariableStoragePpiDescriptor;
  VARIABLE_STORAGE_PPI    *VariableStoragePpi;

  ImageInPermanentMemory = FALSE;

  if (PcdGetBool (PcdForceVolatileVariable) || !PcdGetBool (PcdEnableFvbVariableStorage)) {
    DEBUG ((DEBUG_INFO, "FVB Variable Storage PPI is disabled.\n"));
    return EFI_SUCCESS;
  }

  if (PcdGetBool (PcdReinstallFvbVariablePpi)) {
    Status = PeiServicesRegisterForShadow (FileHandle);
    if (EFI_ERROR (Status)) {
      if (Status == EFI_ALREADY_STARTED) {
        ImageInPermanentMemory = TRUE;
      } else {
        ASSERT_EFI_ERROR (Status);
        return Status;
      }
    }
  }

  if (!ImageInPermanentMemory) {
    PeiServicesInstallPpi (&mPpiListVariable);
    ASSERT_EFI_ERROR (Status);
    return Status;
  } else {
    //
    // Search for any instances of VARIABLE_STORAGE_PPI installed
    // by this PEIM before permanent memory
    // If discovered, reinstall the PPI for post memory
    //
    for (Instance = 0; Status != EFI_NOT_FOUND; Instance++) {
      Status = PeiServicesLocatePpi (
                 &gVariableStoragePpiGuid,
                 Instance,
                 &VariableStoragePpiDescriptor,
                 (VOID **) &VariableStoragePpi
                 );

      if (!EFI_ERROR (Status) && !EFI_ERROR (VariableStoragePpi->GetId (&DiscoveredInstanceGuid))) {
        if (CompareGuid (&gFvbVariableStoragePpiInstanceGuid, &DiscoveredInstanceGuid)) {
          Status = PeiServicesReInstallPpi (VariableStoragePpiDescriptor, &mPpiListVariable);
          ASSERT_EFI_ERROR (Status);
          return Status;
        }
      } else if (Status != EFI_NOT_FOUND) {
        ASSERT_EFI_ERROR (Status);
        return Status;
      }
    }

    //
    // This PEIM did not install an instance of VARIABLE_STORAGE_PPI
    // Install the instance for the first time
    //
    return PeiServicesInstallPpi (&mPpiListVariable);
  }
}

/**

  Gets the pointer to the first variable header in given variable store area.

  @param VarStoreHeader  Pointer to the Variable Store Header.

  @return Pointer to the first variable header

**/
VARIABLE_HEADER *
GetStartPointer (
  IN VARIABLE_STORE_HEADER       *VarStoreHeader
  )
{
  //
  // The end of variable store
  //
  return (VARIABLE_HEADER *) HEADER_ALIGN (VarStoreHeader + 1);
}


/**
  This code gets the pointer to the last variable memory pointer byte.

  @param  VarStoreHeader  Pointer to the Variable Store Header.

  @return VARIABLE_HEADER* pointer to last unavailable Variable Header.

**/
VARIABLE_HEADER *
GetEndPointer (
  IN VARIABLE_STORE_HEADER       *VarStoreHeader
  )
{
  //
  // The end of variable store
  //
  return (VARIABLE_HEADER *) HEADER_ALIGN ((UINTN) VarStoreHeader + VarStoreHeader->Size);
}


/**
  This code checks if variable header is valid or not.

  @param  Variable  Pointer to the Variable Header.

  @retval TRUE      Variable header is valid.
  @retval FALSE     Variable header is not valid.

**/
BOOLEAN
IsValidVariableHeader (
  IN  VARIABLE_HEADER   *Variable
  )
{
  if (Variable == NULL || Variable->StartId != VARIABLE_DATA ) {
    return FALSE;
  }

  return TRUE;
}

/**
  This code gets the size of variable header.

  @param AuthFlag   Authenticated variable flag.

  @return Size of variable header in bytes in type UINTN.

**/
UINTN
GetVariableHeaderSize (
  IN  BOOLEAN       AuthFlag
  )
{
  UINTN Value;

  if (AuthFlag) {
    Value = sizeof (AUTHENTICATED_VARIABLE_HEADER);
  } else {
    Value = sizeof (VARIABLE_HEADER);
  }

  return Value;
}

/**
  This code gets the size of name of variable.

  @param  Variable  Pointer to the Variable Header.
  @param  AuthFlag  Authenticated variable flag.

  @return Size of variable in bytes in type UINTN.

**/
UINTN
NameSizeOfVariable (
  IN  VARIABLE_HEADER   *Variable,
  IN  BOOLEAN           AuthFlag
  )
{
  AUTHENTICATED_VARIABLE_HEADER *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *) Variable;
  if (AuthFlag) {
    if (AuthVariable->State == (UINT8) (-1) ||
       AuthVariable->DataSize == (UINT32) (-1) ||
       AuthVariable->NameSize == (UINT32) (-1) ||
       AuthVariable->Attributes == (UINT32) (-1)) {
      return 0;
    }
    return (UINTN) AuthVariable->NameSize;
  } else {
    if (Variable->State == (UINT8) (-1) ||
       Variable->DataSize == (UINT32) (-1) ||
       Variable->NameSize == (UINT32) (-1) ||
       Variable->Attributes == (UINT32) (-1)) {
      return 0;
    }
    return (UINTN) Variable->NameSize;
  }
}


/**
  This code gets the size of data of variable.

  @param  Variable  Pointer to the Variable Header.
  @param  AuthFlag  Authenticated variable flag.

  @return Size of variable in bytes in type UINTN.

**/
UINTN
DataSizeOfVariable (
  IN  VARIABLE_HEADER   *Variable,
  IN  BOOLEAN           AuthFlag
  )
{
  AUTHENTICATED_VARIABLE_HEADER *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *) Variable;
  if (AuthFlag) {
    if (AuthVariable->State == (UINT8) (-1) ||
       AuthVariable->DataSize == (UINT32) (-1) ||
       AuthVariable->NameSize == (UINT32) (-1) ||
       AuthVariable->Attributes == (UINT32) (-1)) {
      return 0;
    }
    return (UINTN) AuthVariable->DataSize;
  } else {
    if (Variable->State == (UINT8) (-1) ||
       Variable->DataSize == (UINT32) (-1) ||
       Variable->NameSize == (UINT32) (-1) ||
       Variable->Attributes == (UINT32) (-1)) {
      return 0;
    }
    return (UINTN) Variable->DataSize;
  }
}

/**
  This code gets the pointer to the variable name.

  @param   Variable  Pointer to the Variable Header.
  @param   AuthFlag  Authenticated variable flag.

  @return  A CHAR16* pointer to Variable Name.

**/
CHAR16 *
GetVariableNamePtr (
  IN VARIABLE_HEADER    *Variable,
  IN BOOLEAN            AuthFlag
  )
{
  return (CHAR16 *) ((UINTN) Variable + GetVariableHeaderSize (AuthFlag));
}

/**
  This code gets the pointer to the variable guid.

  @param Variable   Pointer to the Variable Header.
  @param AuthFlag   Authenticated variable flag.

  @return A EFI_GUID* pointer to Vendor Guid.

**/
EFI_GUID *
GetVendorGuidPtr (
  IN VARIABLE_HEADER    *Variable,
  IN BOOLEAN            AuthFlag
  )
{
  AUTHENTICATED_VARIABLE_HEADER *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *) Variable;
  if (AuthFlag) {
    return &AuthVariable->VendorGuid;
  } else {
    return &Variable->VendorGuid;
  }
}

/**
  This code gets the pointer to the variable data.

  @param   Variable         Pointer to the Variable Header.
  @param   VariableHeader   Pointer to the Variable Header that has consecutive content.
  @param   AuthFlag         Authenticated variable flag.

  @return  A UINT8* pointer to Variable Data.

**/
UINT8 *
GetVariableDataPtr (
  IN  VARIABLE_HEADER   *Variable,
  IN  VARIABLE_HEADER   *VariableHeader,
  IN  BOOLEAN           AuthFlag
  )
{
  UINTN Value;

  //
  // Be careful about pad size for alignment
  //
  Value =  (UINTN) GetVariableNamePtr (Variable, AuthFlag);
  Value += NameSizeOfVariable (VariableHeader, AuthFlag);
  Value += GET_PAD_SIZE (NameSizeOfVariable (VariableHeader, AuthFlag));

  return (UINT8 *) Value;
}


/**
  This code gets the pointer to the next variable header.

  @param  StoreInfo         Pointer to variable store info structure.
  @param  Variable          Pointer to the Variable Header.
  @param  VariableHeader    Pointer to the Variable Header that has consecutive content.

  @return  A VARIABLE_HEADER* pointer to next variable header.

**/
VARIABLE_HEADER *
GetNextVariablePtr (
  IN  VARIABLE_STORE_INFO   *StoreInfo,
  IN  VARIABLE_HEADER       *Variable,
  IN  VARIABLE_HEADER       *VariableHeader
  )
{
  EFI_PHYSICAL_ADDRESS  TargetAddress;
  EFI_PHYSICAL_ADDRESS  SpareAddress;
  UINTN                 Value;

  Value =  (UINTN) GetVariableDataPtr (Variable, VariableHeader, StoreInfo->AuthFlag);
  Value += DataSizeOfVariable (VariableHeader, StoreInfo->AuthFlag);
  Value += GET_PAD_SIZE (DataSizeOfVariable (VariableHeader, StoreInfo->AuthFlag));
  //
  // Be careful about pad size for alignment
  //
  Value = HEADER_ALIGN (Value);

  if (StoreInfo->FtwLastWriteData != NULL) {
    TargetAddress = StoreInfo->FtwLastWriteData->TargetAddress;
    SpareAddress = StoreInfo->FtwLastWriteData->SpareAddress;
    if (((UINTN) Variable < (UINTN) TargetAddress) && (Value >= (UINTN) TargetAddress)) {
      //
      // Next variable is in spare block.
      //
      Value = (UINTN) SpareAddress + (Value - (UINTN) TargetAddress);
    }
  }

  return (VARIABLE_HEADER *) Value;
}

/**
  Retrieves a PPI instance-specific GUID.

  Returns a unique GUID per VARIABLE_STORAGE_PPI instance.

  @param[out]      VariableGuid           A pointer to an EFI_GUID that is this PPI instance's GUID.

  @retval          EFI_SUCCESS            The data was returned successfully.
  @retval          EFI_INVALID_PARAMETER  A required parameter is NULL.

**/
EFI_STATUS
EFIAPI
PeiFvbVariableStorageGetId (
  OUT       EFI_GUID                        *InstanceGuid
  )
{
  if (InstanceGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (InstanceGuid, &gFvbVariableStoragePpiInstanceGuid, sizeof (EFI_GUID));

  return EFI_SUCCESS;
}

/**
  Get variable store status.

  @param  VarStoreHeader  Pointer to the Variable Store Header.

  @retval  EfiRaw      Variable store is raw
  @retval  EfiValid    Variable store is valid
  @retval  EfiInvalid  Variable store is invalid

**/
VARIABLE_STORE_STATUS
GetVariableStoreStatus (
  IN VARIABLE_STORE_HEADER *VarStoreHeader
  )
{
  if ((CompareGuid (&VarStoreHeader->Signature, &gEfiAuthenticatedVariableGuid) ||
       CompareGuid (&VarStoreHeader->Signature, &gEfiVariableGuid)) &&
      VarStoreHeader->Format == VARIABLE_STORE_FORMATTED &&
      VarStoreHeader->State == VARIABLE_STORE_HEALTHY
      ) {

    return EfiValid;
  }

  if (((UINT32 *)(&VarStoreHeader->Signature))[0] == 0xffffffff &&
      ((UINT32 *)(&VarStoreHeader->Signature))[1] == 0xffffffff &&
      ((UINT32 *)(&VarStoreHeader->Signature))[2] == 0xffffffff &&
      ((UINT32 *)(&VarStoreHeader->Signature))[3] == 0xffffffff &&
      VarStoreHeader->Size == 0xffffffff &&
      VarStoreHeader->Format == 0xff &&
      VarStoreHeader->State == 0xff
      ) {

    return EfiRaw;
  } else {
    return EfiInvalid;
  }
}

/**
  Get variable header that has consecutive content.

  @param StoreInfo      Pointer to variable store info structure.
  @param Variable       Pointer to the Variable Header.
  @param VariableHeader Pointer to Pointer to the Variable Header that has consecutive content.

  @retval TRUE          Variable header is valid.
  @retval FALSE         Variable header is not valid.

**/
BOOLEAN
GetVariableHeader (
  IN VARIABLE_STORE_INFO    *StoreInfo,
  IN VARIABLE_HEADER        *Variable,
  OUT VARIABLE_HEADER       **VariableHeader
  )
{
  EFI_PHYSICAL_ADDRESS  TargetAddress;
  EFI_PHYSICAL_ADDRESS  SpareAddress;
  EFI_HOB_GUID_TYPE     *GuidHob;
  UINTN                 PartialHeaderSize;

  if (Variable == NULL) {
    return FALSE;
  }

  //
  // First assume variable header pointed by Variable is consecutive.
  //
  *VariableHeader = Variable;

  if (StoreInfo->FtwLastWriteData != NULL) {
    TargetAddress = StoreInfo->FtwLastWriteData->TargetAddress;
    SpareAddress = StoreInfo->FtwLastWriteData->SpareAddress;
    if (((UINTN) Variable > (UINTN) SpareAddress) &&
        (((UINTN) Variable - (UINTN) SpareAddress + (UINTN) TargetAddress) >= (UINTN) GetEndPointer (StoreInfo->VariableStoreHeader))) {
      //
      // Reach the end of variable store.
      //
      return FALSE;
    }
    if (((UINTN) Variable < (UINTN) TargetAddress) && (((UINTN) Variable + GetVariableHeaderSize (StoreInfo->AuthFlag)) > (UINTN) TargetAddress)) {
      //
      // Variable header pointed by Variable is inconsecutive,
      // create a guid hob to combine the two partial variable header content together.
      //
      GuidHob = GetFirstGuidHob (&gEfiCallerIdGuid);
      if (GuidHob != NULL) {
        *VariableHeader = (VARIABLE_HEADER *) GET_GUID_HOB_DATA (GuidHob);
      } else {
        *VariableHeader = (VARIABLE_HEADER *) BuildGuidHob (&gEfiCallerIdGuid, GetVariableHeaderSize (StoreInfo->AuthFlag));
        PartialHeaderSize = (UINTN) TargetAddress - (UINTN) Variable;
        //
        // Partial content is in NV storage.
        //
        CopyMem ((UINT8 *) *VariableHeader, (UINT8 *) Variable, PartialHeaderSize);
        //
        // Another partial content is in spare block.
        //
        CopyMem ((UINT8 *) *VariableHeader + PartialHeaderSize, (UINT8 *) (UINTN) SpareAddress, GetVariableHeaderSize (StoreInfo->AuthFlag) - PartialHeaderSize);
      }
    }
  } else {
    if (Variable >= GetEndPointer (StoreInfo->VariableStoreHeader)) {
      //
      // Reach the end of variable store.
      //
      return FALSE;
    }
  }

  return IsValidVariableHeader (*VariableHeader);
}

/**
  Get variable name or data to output buffer.

  @param  StoreInfo     Pointer to variable store info structure.
  @param  NameOrData    Pointer to the variable name/data that may be inconsecutive.
  @param  Size          Variable name/data size.
  @param  Buffer        Pointer to output buffer to hold the variable name/data.

**/
VOID
GetVariableNameOrData (
  IN VARIABLE_STORE_INFO    *StoreInfo,
  IN UINT8                  *NameOrData,
  IN UINTN                  Size,
  OUT UINT8                 *Buffer
  )
{
  EFI_PHYSICAL_ADDRESS  TargetAddress;
  EFI_PHYSICAL_ADDRESS  SpareAddress;
  UINTN                 PartialSize;

  if (StoreInfo->FtwLastWriteData != NULL) {
    TargetAddress = StoreInfo->FtwLastWriteData->TargetAddress;
    SpareAddress = StoreInfo->FtwLastWriteData->SpareAddress;
    if (((UINTN) NameOrData < (UINTN) TargetAddress) && (((UINTN) NameOrData + Size) > (UINTN) TargetAddress)) {
      //
      // Variable name/data is inconsecutive.
      //
      PartialSize = (UINTN) TargetAddress - (UINTN) NameOrData;
      //
      // Partial content is in NV storage.
      //
      CopyMem (Buffer, NameOrData, PartialSize);
      //
      // Another partial content is in spare block.
      //
      CopyMem (Buffer + PartialSize, (UINT8 *) (UINTN) SpareAddress, Size - PartialSize);
      return;
    }
  }

  //
  // Variable name/data is consecutive.
  //
  CopyMem (Buffer, NameOrData, Size);
}

/**
  Compare two variable names, one of them may be inconsecutive.

  @param StoreInfo      Pointer to variable store info structure.
  @param Name1          Pointer to one variable name.
  @param Name2          Pointer to another variable name.
  @param NameSize       Variable name size.

  @retval TRUE          Name1 and Name2 are identical.
  @retval FALSE         Name1 and Name2 are not identical.

**/
BOOLEAN
CompareVariableName (
  IN VARIABLE_STORE_INFO    *StoreInfo,
  IN CONST CHAR16           *Name1,
  IN CONST CHAR16           *Name2,
  IN UINTN                  NameSize
  )
{
  EFI_PHYSICAL_ADDRESS  TargetAddress;
  EFI_PHYSICAL_ADDRESS  SpareAddress;
  UINTN                 PartialNameSize;

  if (StoreInfo->FtwLastWriteData != NULL) {
    TargetAddress = StoreInfo->FtwLastWriteData->TargetAddress;
    SpareAddress = StoreInfo->FtwLastWriteData->SpareAddress;
    if (((UINTN) Name1 < (UINTN) TargetAddress) && (((UINTN) Name1 + NameSize) > (UINTN) TargetAddress)) {
      //
      // Name1 is inconsecutive.
      //
      PartialNameSize = (UINTN) TargetAddress - (UINTN) Name1;
      //
      // Partial content is in NV storage.
      //
      if (CompareMem ((UINT8 *) Name1, (UINT8 *) Name2, PartialNameSize) == 0) {
        //
        // Another partial content is in spare block.
        //
        if (CompareMem ((UINT8 *) (UINTN) SpareAddress, (UINT8 *) Name2 + PartialNameSize, NameSize - PartialNameSize) == 0) {
          return TRUE;
        }
      }
      return FALSE;
    } else if (((UINTN) Name2 < (UINTN) TargetAddress) && (((UINTN) Name2 + NameSize) > (UINTN) TargetAddress)) {
      //
      // Name2 is inconsecutive.
      //
      PartialNameSize = (UINTN) TargetAddress - (UINTN) Name2;
      //
      // Partial content is in NV storage.
      //
      if (CompareMem ((UINT8 *) Name2, (UINT8 *) Name1, PartialNameSize) == 0) {
        //
        // Another partial content is in spare block.
        //
        if (CompareMem ((UINT8 *) (UINTN) SpareAddress, (UINT8 *) Name1 + PartialNameSize, NameSize - PartialNameSize) == 0) {
          return TRUE;
        }
      }
      return FALSE;
    }
  }

  //
  // Both Name1 and Name2 are consecutive.
  //
  if (CompareMem ((UINT8 *) Name1, (UINT8 *) Name2, NameSize) == 0) {
    return TRUE;
  }
  return FALSE;
}

/**
  This function compares a variable with variable entries in database.

  @param  StoreInfo     Pointer to variable store info structure.
  @param  Variable      Pointer to the variable in our database
  @param  VariableHeader Pointer to the Variable Header that has consecutive content.
  @param  VariableName  Name of the variable to compare to 'Variable'
  @param  VendorGuid    GUID of the variable to compare to 'Variable'
  @param  PtrTrack      Variable Track Pointer structure that contains Variable Information.

  @retval EFI_SUCCESS    Found match variable
  @retval EFI_NOT_FOUND  Variable not found

**/
EFI_STATUS
CompareWithValidVariable (
  IN  VARIABLE_STORE_INFO           *StoreInfo,
  IN  VARIABLE_HEADER               *Variable,
  IN  VARIABLE_HEADER               *VariableHeader,
  IN  CONST CHAR16                  *VariableName,
  IN  CONST EFI_GUID                *VendorGuid,
  OUT VARIABLE_POINTER_TRACK        *PtrTrack
  )
{
  VOID      *Point;
  EFI_GUID  *TempVendorGuid;

  TempVendorGuid = GetVendorGuidPtr (VariableHeader, StoreInfo->AuthFlag);

  if (VariableName[0] == 0) {
    PtrTrack->CurrPtr = Variable;
    return EFI_SUCCESS;
  } else {
    //
    // Don't use CompareGuid function here for performance reasons.
    // Instead we compare the GUID a UINT32 at a time and branch
    // on the first failed comparison.
    //
    if ((((INT32 *) VendorGuid)[0] == ((INT32 *) TempVendorGuid)[0]) &&
        (((INT32 *) VendorGuid)[1] == ((INT32 *) TempVendorGuid)[1]) &&
        (((INT32 *) VendorGuid)[2] == ((INT32 *) TempVendorGuid)[2]) &&
        (((INT32 *) VendorGuid)[3] == ((INT32 *) TempVendorGuid)[3])
        ) {
      ASSERT (NameSizeOfVariable (VariableHeader, StoreInfo->AuthFlag) != 0);
      Point = (VOID *) GetVariableNamePtr (Variable, StoreInfo->AuthFlag);
      if (CompareVariableName (StoreInfo, VariableName, Point, NameSizeOfVariable (VariableHeader, StoreInfo->AuthFlag))) {
        PtrTrack->CurrPtr = Variable;
        return EFI_SUCCESS;
      }
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Return the variable store header and the store info based on the Index.

  @param Type       The type of the variable store.
  @param StoreInfo  Return the store info.

  @return  Pointer to the variable store header.
**/
VARIABLE_STORE_HEADER *
GetVariableStore (
  IN VARIABLE_STORE_TYPE         Type,
  OUT VARIABLE_STORE_INFO        *StoreInfo
  )
{
  EFI_HOB_GUID_TYPE                     *GuidHob;
  EFI_FIRMWARE_VOLUME_HEADER            *FvHeader;
  VARIABLE_STORE_HEADER                 *VariableStoreHeader;
  EFI_PHYSICAL_ADDRESS                  NvStorageBase;
  UINT32                                NvStorageSize;
  FAULT_TOLERANT_WRITE_LAST_WRITE_DATA  *FtwLastWriteData;
  UINT32                                BackUpOffset;

  StoreInfo->IndexTable = NULL;
  StoreInfo->FtwLastWriteData = NULL;
  StoreInfo->AuthFlag = FALSE;
  VariableStoreHeader = NULL;
  switch (Type) {
    case VariableStoreTypeNv:
      if (GetBootModeHob () != BOOT_IN_RECOVERY_MODE) {
        //
        // The content of NV storage for variable is not reliable in recovery boot mode.
        //

        NvStorageSize = PcdGet32 (PcdFlashNvStorageVariableSize);
        NvStorageBase = (EFI_PHYSICAL_ADDRESS) (PcdGet64 (PcdFlashNvStorageVariableBase64) != 0 ?
                                                PcdGet64 (PcdFlashNvStorageVariableBase64) :
                                                PcdGet32 (PcdFlashNvStorageVariableBase)
                                               );
        if (NvStorageBase == 0) {
          DEBUG ((EFI_D_ERROR, "Firmware Volume for Variable Store location is not set!\n"));
          break;
        }
        //
        // First let FvHeader point to NV storage base.
        //
        FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) NvStorageBase;

        //
        // Check the FTW last write data hob.
        //
        BackUpOffset = 0;
        GuidHob = GetFirstGuidHob (&gEdkiiFaultTolerantWriteGuid);
        if (GuidHob != NULL) {
          FtwLastWriteData = (FAULT_TOLERANT_WRITE_LAST_WRITE_DATA *) GET_GUID_HOB_DATA (GuidHob);
          if (FtwLastWriteData->TargetAddress == NvStorageBase) {
            //
            // Let FvHeader point to spare block.
            //
            FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) FtwLastWriteData->SpareAddress;
            DEBUG ((EFI_D_INFO, "PeiVariable: NV storage is backed up in spare block: 0x%x\n", (UINTN) FtwLastWriteData->SpareAddress));
          } else if ((FtwLastWriteData->TargetAddress > NvStorageBase) && (FtwLastWriteData->TargetAddress < (NvStorageBase + NvStorageSize))) {
            StoreInfo->FtwLastWriteData = FtwLastWriteData;
            //
            // Flash NV storage from the offset is backed up in spare block.
            //
            BackUpOffset = (UINT32) (FtwLastWriteData->TargetAddress - NvStorageBase);
            DEBUG ((EFI_D_INFO, "PeiVariable: High partial NV storage from offset: %x is backed up in spare block: 0x%x\n", BackUpOffset, (UINTN) FtwLastWriteData->SpareAddress));
            //
            // At least one block data in flash NV storage is still valid, so still leave FvHeader point to NV storage base.
            //
          }
        }

        //
        // Check if the Firmware Volume is not corrupted
        //
        if ((FvHeader->Signature != EFI_FVH_SIGNATURE) || (!CompareGuid (&gEfiSystemNvDataFvGuid, &FvHeader->FileSystemGuid))) {
          DEBUG ((EFI_D_ERROR, "Firmware Volume for Variable Store is corrupted\n"));
          break;
        }

        VariableStoreHeader = (VARIABLE_STORE_HEADER *) ((UINT8 *) FvHeader + FvHeader->HeaderLength);

        StoreInfo->AuthFlag = (BOOLEAN) (CompareGuid (&VariableStoreHeader->Signature, &gEfiAuthenticatedVariableGuid));

        GuidHob = GetFirstGuidHob (&gEfiVariableIndexTableGuid);
        if (GuidHob != NULL) {
          StoreInfo->IndexTable = GET_GUID_HOB_DATA (GuidHob);
        } else {
          //
          // If it's the first time to access variable region in flash, create a guid hob to record
          // VAR_ADDED type variable info.
          // Note that as the resource of PEI phase is limited, only store the limited number of
          // VAR_ADDED type variables to reduce access time.
          //
          StoreInfo->IndexTable = (VARIABLE_INDEX_TABLE *) BuildGuidHob (&gEfiVariableIndexTableGuid, sizeof (VARIABLE_INDEX_TABLE));
          StoreInfo->IndexTable->Length      = 0;
          StoreInfo->IndexTable->StartPtr    = GetStartPointer (VariableStoreHeader);
          StoreInfo->IndexTable->EndPtr      = GetEndPointer   (VariableStoreHeader);
          StoreInfo->IndexTable->GoneThrough = 0;
        }
      }
      break;

    default:
      ASSERT (FALSE);
      break;
  }

  StoreInfo->VariableStoreHeader = VariableStoreHeader;
  return VariableStoreHeader;
}

/**
  Find the variable in the specified variable store.

  @param  StoreInfo           Pointer to the store info structure.
  @param  VariableName        Name of the variable to be found
  @param  VendorGuid          Vendor GUID to be found.
  @param  PtrTrack            Variable Track Pointer structure that contains Variable Information.

  @retval  EFI_SUCCESS            Variable found successfully
  @retval  EFI_NOT_FOUND          Variable not found
  @retval  EFI_INVALID_PARAMETER  Invalid variable name

**/
EFI_STATUS
FindVariableEx (
  IN VARIABLE_STORE_INFO         *StoreInfo,
  IN CONST CHAR16                *VariableName,
  IN CONST EFI_GUID              *VendorGuid,
  OUT VARIABLE_POINTER_TRACK     *PtrTrack
  )
{
  VARIABLE_HEADER         *Variable;
  VARIABLE_HEADER         *LastVariable;
  VARIABLE_HEADER         *MaxIndex;
  UINTN                   Index;
  UINTN                   Offset;
  BOOLEAN                 StopRecord;
  VARIABLE_HEADER         *InDeletedVariable;
  VARIABLE_STORE_HEADER   *VariableStoreHeader;
  VARIABLE_INDEX_TABLE    *IndexTable;
  VARIABLE_HEADER         *VariableHeader;

  VariableStoreHeader = StoreInfo->VariableStoreHeader;

  if (VariableStoreHeader == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (GetVariableStoreStatus (VariableStoreHeader) != EfiValid) {
    return EFI_UNSUPPORTED;
  }

  if (~VariableStoreHeader->Size == 0) {
    return EFI_NOT_FOUND;
  }

  IndexTable = StoreInfo->IndexTable;
  PtrTrack->StartPtr = GetStartPointer (VariableStoreHeader);
  PtrTrack->EndPtr   = GetEndPointer   (VariableStoreHeader);

  InDeletedVariable = NULL;

  //
  // No Variable Address equals zero, so 0 as initial value is safe.
  //
  MaxIndex   = NULL;
  VariableHeader = NULL;

  if (IndexTable != NULL) {
    //
    // traverse the variable index table to look for varible.
    // The IndexTable->Index[Index] records the distance of two neighbouring VAR_ADDED type variables.
    //
    for (Offset = 0, Index = 0; Index < IndexTable->Length; Index++) {
      ASSERT (Index < sizeof (IndexTable->Index) / sizeof (IndexTable->Index[0]));
      Offset   += IndexTable->Index[Index];
      MaxIndex  = (VARIABLE_HEADER *) ((UINT8 *) IndexTable->StartPtr + Offset);
      GetVariableHeader (StoreInfo, MaxIndex, &VariableHeader);
      if (CompareWithValidVariable (StoreInfo, MaxIndex, VariableHeader, VariableName, VendorGuid, PtrTrack) == EFI_SUCCESS) {
        if (VariableHeader->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
          InDeletedVariable = PtrTrack->CurrPtr;
        } else {
          return EFI_SUCCESS;
        }
      }
    }

    if (IndexTable->GoneThrough != 0) {
      //
      // If the table has all the existing variables indexed, return.
      //
      PtrTrack->CurrPtr = InDeletedVariable;
      return (PtrTrack->CurrPtr == NULL) ? EFI_NOT_FOUND : EFI_SUCCESS;
    }
  }

  if (MaxIndex != NULL) {
    //
    // HOB exists but the variable cannot be found in HOB
    // If not found in HOB, then let's start from the MaxIndex we've found.
    //
    Variable     = GetNextVariablePtr (StoreInfo, MaxIndex, VariableHeader);
    LastVariable = MaxIndex;
  } else {
    //
    // Start Pointers for the variable.
    // Actual Data Pointer where data can be written.
    //
    Variable     = PtrTrack->StartPtr;
    LastVariable = PtrTrack->StartPtr;
  }

  //
  // Find the variable by walk through variable store
  //
  StopRecord = FALSE;
  while (GetVariableHeader (StoreInfo, Variable, &VariableHeader)) {
    if (VariableHeader->State == VAR_ADDED || VariableHeader->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
      //
      // Record Variable in VariableIndex HOB
      //
      if ((IndexTable != NULL) && !StopRecord) {
        Offset = (UINTN) Variable - (UINTN) LastVariable;
        if ((Offset > 0x0FFFF) || (IndexTable->Length == sizeof (IndexTable->Index) / sizeof (IndexTable->Index[0]))) {
          //
          // Stop to record if the distance of two neighbouring VAR_ADDED variable is larger than the allowable scope(UINT16),
          // or the record buffer is full.
          //
          StopRecord = TRUE;
        } else {
          IndexTable->Index[IndexTable->Length++] = (UINT16) Offset;
          LastVariable = Variable;
        }
      }

      if (CompareWithValidVariable (StoreInfo, Variable, VariableHeader, VariableName, VendorGuid, PtrTrack) == EFI_SUCCESS) {
        if (VariableHeader->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
          InDeletedVariable = PtrTrack->CurrPtr;
        } else {
          return EFI_SUCCESS;
        }
      }
    }

    Variable = GetNextVariablePtr (StoreInfo, Variable, VariableHeader);
  }
  //
  // The end of the NV Variable Store has been reached
  // If StopRecord was not set, then all variables are now in the IndexTable
  // NV variables can now be completely enumerated using the IndexTable in the future
  // This substantially reduces the amount of time needed for subsequent enumerations
  //
  if ((IndexTable != NULL) && !StopRecord) {
    IndexTable->GoneThrough = 1;
  }

  PtrTrack->CurrPtr = InDeletedVariable;

  return (PtrTrack->CurrPtr == NULL) ? EFI_NOT_FOUND : EFI_SUCCESS;
}

/**
  Find the variable in HOB and Non-Volatile variable storages.

  @param  VariableName  Name of the variable to be found
  @param  VendorGuid    Vendor GUID to be found.
  @param  PtrTrack      Variable Track Pointer structure that contains Variable Information.
  @param  StoreInfo     Return the store info.

  @retval  EFI_SUCCESS            Variable found successfully
  @retval  EFI_NOT_FOUND          Variable not found
  @retval  EFI_INVALID_PARAMETER  Invalid variable name
**/
EFI_STATUS
FindVariable (
  IN CONST  CHAR16            *VariableName,
  IN CONST  EFI_GUID          *VendorGuid,
  OUT VARIABLE_POINTER_TRACK  *PtrTrack,
  OUT VARIABLE_STORE_INFO     *StoreInfo
  )
{
  EFI_STATUS                  Status;
  VARIABLE_STORE_TYPE         Type;

  if (VariableName[0] != 0 && VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  for (Type = (VARIABLE_STORE_TYPE) 0; Type < VariableStoreTypeMax; Type++) {
    GetVariableStore (Type, StoreInfo);
    Status = FindVariableEx (
               StoreInfo,
               VariableName,
               VendorGuid,
               PtrTrack
               );
    if (!EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  This service retrieves a variable's value using its name and GUID.

  Read the specified variable from the NV variable store FV. If the Data
  buffer is too small to hold the contents of the variable,
  the error EFI_BUFFER_TOO_SMALL is returned and DataSize is set to the
  required buffer size to obtain the data.

  @param[in]       This                   A pointer to this instance of the VARIABLE_STORAGE_PPI.
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
PeiFvbVariableStorageGetVariable (
  IN CONST  VARIABLE_STORAGE_PPI            *This,
  IN CONST  CHAR16                          *VariableName,
  IN CONST  EFI_GUID                        *VariableGuid,
  OUT       UINT32                          *Attributes,
  IN OUT    UINTN                           *DataSize,
  OUT       VOID                            *Data
  )
{
  VARIABLE_POINTER_TRACK  Variable;
  UINTN                   VarDataSize;
  EFI_STATUS              Status;
  VARIABLE_STORE_INFO     StoreInfo;
  VARIABLE_HEADER         *VariableHeader;

  if (VariableName == NULL || VariableGuid == NULL || DataSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  VariableHeader = NULL;

  //
  // Find existing variable
  //
  Status = FindVariable (VariableName, VariableGuid, &Variable, &StoreInfo);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  GetVariableHeader (&StoreInfo, Variable.CurrPtr, &VariableHeader);

  //
  // Get data size
  //
  VarDataSize = DataSizeOfVariable (VariableHeader, StoreInfo.AuthFlag);
  if (*DataSize >= VarDataSize) {
    if (Data == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    GetVariableNameOrData (&StoreInfo, GetVariableDataPtr (Variable.CurrPtr, VariableHeader, StoreInfo.AuthFlag), VarDataSize, Data);

    if (Attributes != NULL) {
      *Attributes = VariableHeader->Attributes;
    }

    *DataSize = VarDataSize;
    return EFI_SUCCESS;
  } else {
    *DataSize = VarDataSize;
    return EFI_BUFFER_TOO_SMALL;
  }
}

/**
  Return the next variable name and GUID.

  This function is called multiple times to retrieve the VariableName
  and VariableGuid of all variables currently available in the system.
  On each call, the previous results are passed into the interface,
  and, on return, the interface returns the data for the next
  interface. When the entire variable list has been returned,
  EFI_NOT_FOUND is returned.

  @param[in]      This                   A pointer to this instance of the VARIABLE_STORAGE_PPI.

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
PeiFvbVariableStorageGetNextVariableName (
  IN CONST  VARIABLE_STORAGE_PPI            *This,
  IN OUT    UINTN                           *VariableNameSize,
  IN OUT    CHAR16                          *VariableName,
  IN OUT    EFI_GUID                        *VariableGuid,
  OUT       UINT32                          *VariableAttributes
  )
{
  //
  // Temporary dummy stub implementation
  //

  return EFI_SUCCESS;
}

/** @file
  Implement ReadOnly Variable Services required by PEIM and install
  PEI ReadOnly Varaiable2 PPI. These services operates the non volatile storage space.

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/**


Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
Module Name:

**/


#include "Variable.h"
#include <Library/FlashRegionLib.h>

extern BOOLEAN mVariablePpiInstalledInMemory;

EFI_STATUS
ReadOnlyVariable2HookCallback (
  IN CONST EFI_PEI_SERVICES           **PeiServices
//  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
//  IN VOID                       *Ppi
  );

//
// Module globals
//
EFI_PEI_READ_ONLY_VARIABLE2_PPI mVariable2Ppi = {
  PeiGetVariable,
  PeiGetNextVariableName
};

EFI_PEI_PPI_DESCRIPTOR     mPpiListVariable2 = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiReadOnlyVariable2PpiGuid,
  &mVariable2Ppi
};

/**
  Provide the functionality of the variable services.

  @param  FileHandle   Handle of the file being invoked.
                       Type EFI_PEI_FILE_HANDLE is defined in FfsFindNextFile().
  @param  PeiServices  General purpose services available to every PEIM.

  @retval EFI_SUCCESS  If the interface could be successfully installed
  @retval Others       Returned from PeiServicesInstallPpi()
**/
EFI_STATUS
EFIAPI
PeimInitializeVariableServices (
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
{
  EFI_STATUS                        Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI   *VariablePpi;
  EFI_PEI_PPI_DESCRIPTOR            *VariablePeiPpiDescriptor;

  //
  // Register so it will be automatically shadowed to memory
  //
  Status = PeiServicesRegisterForShadow (FileHandle);

  if (!EFI_ERROR (Status)) {

    Status = PeiServicesInstallPpi (&mPpiListVariable2);
    Status = ReadOnlyVariable2HookCallback (PeiServices);

  } else if (Status == EFI_ALREADY_STARTED) {

    //
    // Now that module in memory, update the
    // PPI that describes the Stall to other modules
    //
    Status = (**PeiServices).LocatePpi (
                               PeiServices,
                               &gEfiPeiReadOnlyVariable2PpiGuid,
                               0,
                               &VariablePeiPpiDescriptor,
                               (VOID **)&VariablePpi
                               );

    //
    // Reinstall the Interface using the memory-based descriptor
    //
    if (!EFI_ERROR (Status)) {
      //
      // Reinstall the PPI
      //
      mVariablePpiInstalledInMemory = TRUE;
      Status = (**PeiServices).ReInstallPpi (
                                 PeiServices,
                                 VariablePeiPpiDescriptor,
                                 &mPpiListVariable2
                                 );
      Status = ReadOnlyVariable2HookCallback (PeiServices);
    }

  }

  return Status;
}

/**
  This function compares a variable with variable entries in database.

  @param  Variable      Pointer to the variable in our database
  @param  VariableName  Name of the variable to compare to 'Variable'
  @param  VendorGuid    GUID of the variable to compare to 'Variable'
  @param  PtrTrack      Variable Track Pointer structure that contains Variable Information.

  @retval EFI_SUCCESS    Found match variable
  @retval EFI_NOT_FOUND  Variable not found

**/
EFI_STATUS
CompareWithValidVariable (
  IN  VARIABLE_HEADER               *Variable,
  IN  CONST CHAR16                  *VariableName,
  IN  CONST EFI_GUID                *VendorGuid,
  OUT VARIABLE_POINTER_TRACK        *PtrTrack
  )
{
  VOID  *Point;

  if (VariableName[0] == 0) {
    PtrTrack->CurrPtr = Variable;
    return EFI_SUCCESS;
  } else {
    //
    // Don't use CompareGuid function here for performance reasons.
    // Instead we compare the GUID a UINT32 at a time and branch
    // on the first failed comparison.
    //
    if ((((INT32 *) VendorGuid)[0] == ((INT32 *) &Variable->VendorGuid)[0]) &&
        (((INT32 *) VendorGuid)[1] == ((INT32 *) &Variable->VendorGuid)[1]) &&
        (((INT32 *) VendorGuid)[2] == ((INT32 *) &Variable->VendorGuid)[2]) &&
        (((INT32 *) VendorGuid)[3] == ((INT32 *) &Variable->VendorGuid)[3])
        ) {
      ASSERT (NameSizeOfVariable (Variable) != 0);
      Point = (VOID *) GET_VARIABLE_NAME_PTR (Variable);
      if (CompareMem (VariableName, Point, NameSizeOfVariable (Variable)) == 0) {
        PtrTrack->CurrPtr = Variable;
        return EFI_SUCCESS;
      }
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Return the variable HOB header and the store info for the given HOB type

  @param Type       The type of the variable HOB.
  @param StoreInfo  Return the store info.

  @return  Pointer to the variable HOB header.
**/
VARIABLE_STORE_HEADER *
GetHobVariableStore (
  IN VARIABLE_HOB_TYPE           Type,
  OUT VARIABLE_STORE_INFO        *StoreInfo
  )
{
  EFI_HOB_GUID_TYPE              *GuidHob;

  switch (Type) {
    case VariableHobTypeCache:
      GuidHob = GetFirstGuidHob (&gPeiVariableCacheHobGuid);
      if (GuidHob != NULL) {
        StoreInfo->VariableStoreHeader = (VARIABLE_STORE_HEADER *) GET_GUID_HOB_DATA (GuidHob);
        return StoreInfo->VariableStoreHeader;
      }
      break;

    case VariableHobTypeDefault:
      GuidHob = GetFirstGuidHob (&gEfiAuthenticatedVariableGuid);
      if (GuidHob != NULL) {
        StoreInfo->VariableStoreHeader = (VARIABLE_STORE_HEADER *) GET_GUID_HOB_DATA (GuidHob);
        return StoreInfo->VariableStoreHeader;
      }
      break;
  }
  return NULL;
}

/**
  Get the specific config data from MultiConfig region.

  @param  RequireKind         Find Setup Setting for SETUP_FOR_BIOS_POST or SETUP_FOR_LOAD_DEFAULT.
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
  IN CONST  CHAR16                  *VariableName,
  IN CONST  EFI_GUID                *VariableGuid,
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
  DataPtr = NULL;
  ConfigCount = GetConfigCount();

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
          return EFI_SUCCESS;
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
  This code gets the current status of Variable Store.

  @param[in] VarStoreHeader  Pointer to the Variable Store Header.

  @retval EfiRaw             Variable store status is raw.
  @retval EfiValid           Variable store status is valid.
  @retval EfiInvalid         Variable store status is invalid.
**/
VARIABLE_STORE_STATUS
PeiGetVariableStoreStatus (
  IN CONST VARIABLE_STORE_HEADER *VarStoreHeader
  )
{
  ECP_VARIABLE_STORE_HEADER        *EcpVarStoreHeader;

  if (PcdGetBool (PcdUseEcpVariableStoreHeader)) {
    EcpVarStoreHeader = (ECP_VARIABLE_STORE_HEADER *) VarStoreHeader;
    if (EcpVarStoreHeader->Signature == ECP_VARIABLE_STORE_SIGNATURE &&
        EcpVarStoreHeader->Format == VARIABLE_STORE_FORMATTED &&
        EcpVarStoreHeader->State == VARIABLE_STORE_HEALTHY
        ) {

      return EfiValid;
    }

    if (EcpVarStoreHeader->Signature == 0xffffffff &&
        EcpVarStoreHeader->Size == 0xffffffff &&
        EcpVarStoreHeader->Format == 0xff &&
        EcpVarStoreHeader->State == 0xff
        ) {

      return EfiRaw;
    } else {

      return EfiInvalid;
    }
  } else {
    if (( CompareGuid (&VarStoreHeader->Signature, &gPeiVariableCacheHobGuid) ||
        CompareGuid (&VarStoreHeader->Signature, &gEfiAuthenticatedVariableGuid) ||
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
FindVariableInHobsInternal (
  IN VARIABLE_STORE_INFO         *StoreInfo,
  IN CONST CHAR16                *VariableName,
  IN CONST EFI_GUID              *VendorGuid,
  OUT VARIABLE_POINTER_TRACK     *PtrTrack
  )
{
  VARIABLE_HEADER         *Variable;
  VARIABLE_HEADER         *InDeletedVariable;
  VARIABLE_STORE_HEADER   *VariableStoreHeader;
  VARIABLE_HEADER         *VariableHeader;

  VariableStoreHeader = StoreInfo->VariableStoreHeader;

  if (VariableStoreHeader == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (PeiGetVariableStoreStatus (VariableStoreHeader) != EfiValid) {
    return EFI_UNSUPPORTED;
  }

  if (~VariableStoreHeader->Size == 0) {
    return EFI_NOT_FOUND;
  }

  PtrTrack->StartPtr = GetStartPointer (VariableStoreHeader);
  PtrTrack->EndPtr   = GetEndPointer   (VariableStoreHeader);

  InDeletedVariable = NULL;

  //
  // No Variable Address equals zero, so 0 as initial value is safe.
  //
  VariableHeader = NULL;

  //
  // Start at the beginning of the HOB
  //
  Variable = PtrTrack->StartPtr;

  //
  // Find the variable by walk through variable store
  //
  while ((GetNextVariablePtr (Variable) <= PtrTrack->EndPtr) && IsValidVariableHeader (Variable)) {
    if (Variable->State == VAR_ADDED || Variable->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
      if (CompareWithValidVariable (Variable, VariableName, VendorGuid, PtrTrack) == EFI_SUCCESS) {
        if (PtrTrack->CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
          InDeletedVariable = PtrTrack->CurrPtr;
        } else {
          return EFI_SUCCESS;
        }
      }
    }
    Variable = GetNextVariablePtr (Variable);
  }
  PtrTrack->CurrPtr = InDeletedVariable;

  return (PtrTrack->CurrPtr == NULL) ? EFI_NOT_FOUND : EFI_SUCCESS;
}

/**
  Find the variable in HOB variable storages.

  @param  VariableName  Name of the variable to be found
  @param  VendorGuid    Vendor GUID to be found.
  @param  PtrTrack      Variable Track Pointer structure that contains Variable Information.
  @param  StoreInfo     Return the store info.

  @retval  EFI_SUCCESS            Variable found successfully
  @retval  EFI_NOT_FOUND          Variable not found
  @retval  EFI_INVALID_PARAMETER  Invalid variable name
**/
EFI_STATUS
FindVariableInHobs (
  IN CONST  CHAR16            *VariableName,
  IN CONST  EFI_GUID          *VendorGuid,
  OUT VARIABLE_POINTER_TRACK  *PtrTrack,
  OUT VARIABLE_STORE_INFO     *StoreInfo
  )
{
  EFI_STATUS                  Status;
  VARIABLE_HOB_TYPE           Type;

  if (VariableName[0] != 0 && VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  for (Type = (VARIABLE_HOB_TYPE) 0; Type < VariableHobTypeMax; Type++) {
    if (GetHobVariableStore (Type, StoreInfo) != NULL) {
      Status = FindVariableInHobsInternal (
                 StoreInfo,
                 VariableName,
                 VendorGuid,
                 PtrTrack
                 );
      if (!EFI_ERROR (Status)) {
        return Status;
      }
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Retrieves a variable value from HOB resources.

  Read the specified variable from the UEFI variable store in the HOB. If the Data
  buffer is too small to hold the contents of the variable, the error
  EFI_BUFFER_TOO_SMALL is returned and DataSize is set to the required buffer
  size to obtain the data.

  @param  VariableName          A pointer to a null-terminated string that is the variable's name.
  @param  VariableGuid          A pointer to an EFI_GUID that is the variable's GUID. The combination of
                                VariableGuid and VariableName must be unique.
  @param  Attributes            If non-NULL, on return, points to the variable's attributes.
  @param  DataSize              On entry, points to the size in bytes of the Data buffer.
                                On return, points to the size of the data returned in Data.
  @param  Data                  Points to the buffer which will hold the returned variable value.

  @retval EFI_SUCCESS           The variable was read successfully.
  @retval EFI_NOT_FOUND         The variable could not be found.
  @retval EFI_BUFFER_TOO_SMALL  The DataSize is too small for the resulting data.
                                DataSize is updated with the size required for
                                the specified variable.
  @retval EFI_INVALID_PARAMETER VariableName, VariableGuid, DataSize or Data is NULL.

**/
EFI_STATUS
EFIAPI
GetHobVariable (
  IN CONST  CHAR16                          *VariableName,
  IN CONST  EFI_GUID                        *VariableGuid,
  OUT       UINT32                          *Attributes,
  IN OUT    UINTN                           *DataSize,
  OUT       VOID                            *Data
  )
{
  EFI_STATUS              Status;
  UINTN                   HobVariableDataSize;
  VARIABLE_HOB_TYPE       Type;
  VARIABLE_POINTER_TRACK  HobVariable;
  VARIABLE_STORE_INFO     HobStoreInfo;

  //
  // Check the HOB variable stores if they exist
  //
  for (Type = (VARIABLE_HOB_TYPE) 0; Type < VariableHobTypeMax; Type++) {
    if (GetHobVariableStore (Type, &HobStoreInfo) != NULL) {
      DEBUG ((EFI_D_INFO, "Temp Debug: Found a HOB variable store in PeiGetVariable()\n"));
      Status = FindVariableInHobsInternal (
                 &HobStoreInfo,
                 VariableName,
                 VariableGuid,
                 &HobVariable
                 );
      if (HobVariable.CurrPtr == NULL || EFI_ERROR (Status)) {
        if (Status == EFI_NOT_FOUND) {
          //
          // Check the next HOB
          //
          continue;
        }
        DEBUG ((EFI_D_ERROR, "Temp Debug: HobVariable found was null or had an error\n"));
        return Status;
      }

      //
      // Get data size
      //
      HobVariableDataSize = DataSizeOfVariable (HobVariable.CurrPtr);
      if (*DataSize >= HobVariableDataSize) {
        DEBUG ((EFI_D_INFO, "Temp Debug: Data buffer passed for variable data is large enough\n"));
        if (Data == NULL) {
          DEBUG ((EFI_D_ERROR, "Temp Debug: Data in the variable is NULL (invalid)\n"));
          return EFI_INVALID_PARAMETER;
        }

        CopyMem (Data, GetVariableDataPtr (HobVariable.CurrPtr), HobVariableDataSize);
        if (Attributes != NULL) {
          *Attributes = HobVariable.CurrPtr->Attributes;
        }

        *DataSize = HobVariableDataSize;

        DEBUG ((EFI_D_INFO, "Temp Debug: Variable was found in the HOB. No need to check storage PPIs. Returning success.\n\n"));

        //
        // Variable was found in the HOB
        //
        return EFI_SUCCESS;
      } else {
        DEBUG ((EFI_D_ERROR, "Temp Debug: Data buffer provided for variable data is too small!\n"));
        *DataSize = HobVariableDataSize;
        return EFI_BUFFER_TOO_SMALL;
      }
    }
  }

  return EFI_NOT_FOUND;
}

/**
  This service retrieves a variable's value using its name and GUID.

  Read the specified variable from the UEFI variable store. If the Data
  buffer is too small to hold the contents of the variable, the error
  EFI_BUFFER_TOO_SMALL is returned and DataSize is set to the required buffer
  size to obtain the data.

  @param  This                  A pointer to this instance of the EFI_PEI_READ_ONLY_VARIABLE2_PPI.
  @param  VariableName          A pointer to a null-terminated string that is the variable's name.
  @param  VariableGuid          A pointer to an EFI_GUID that is the variable's GUID. The combination of
                                VariableGuid and VariableName must be unique.
  @param  Attributes            If non-NULL, on return, points to the variable's attributes.
  @param  DataSize              On entry, points to the size in bytes of the Data buffer.
                                On return, points to the size of the data returned in Data.
  @param  Data                  Points to the buffer which will hold the returned variable value.

  @retval EFI_SUCCESS           The variable was read successfully.
  @retval EFI_NOT_FOUND         The variable could not be found.
  @retval EFI_BUFFER_TOO_SMALL  The DataSize is too small for the resulting data.
                                DataSize is updated with the size required for
                                the specified variable.
  @retval EFI_INVALID_PARAMETER VariableName, VariableGuid, DataSize or Data is NULL.
  @retval EFI_DEVICE_ERROR      The variable could not be retrieved because of a device error.

**/
EFI_STATUS
EFIAPI
PeiGetVariable (
  IN CONST  EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
  IN CONST  CHAR16                          *VariableName,
  IN CONST  EFI_GUID                        *VariableGuid,
  OUT       UINT32                          *Attributes,
  IN OUT    UINTN                           *DataSize,
  OUT       VOID                            *Data
  )
{

  EFI_GUID                       DiscoveredInstanceGuid;
  EFI_GUID                       VariableStorageInstanceId;
  EFI_PEI_PPI_DESCRIPTOR         *VariableStoragePpiDescriptor;
  EFI_STATUS                     Status;
  UINTN                          Instance;
  VARIABLE_STORAGE_PPI           *VariableStoragePpi;
  VARIABLE_STORAGE_SELECTOR_PPI  *VariableStorageSelectorPpi;

  if (VariableName == NULL || VariableGuid == NULL || DataSize == NULL || VariableName[0] == 0) {
    DEBUG ((EFI_D_ERROR, "Temp Debug: Invalid parameter passed to PeiGetVariable()\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check if the variable can be found in a HOB store
  //
  Status = GetHobVariable (VariableName, VariableGuid, Attributes, DataSize, Data);

  if (Status == EFI_NOT_FOUND) {
    DEBUG ((EFI_D_INFO, "Temp Debug: Could not find the variable in the HOBs. Checking storage PPIs...\n"));
  } else if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Temp Debug: Error occurred checking the HOBs for the variable.\n"));
  } else {
    return Status;
  }

  //
  // Determine which PPI instance should be used for this variable
  //
  Status = PeiServicesLocatePpi (
             &gVariableStorageSelectorPpiGuid,
             0,
             NULL,
             (VOID **) &VariableStorageSelectorPpi
             );
  if (Status == EFI_NOT_FOUND) {
    return Status;
  } else if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  Status = VariableStorageSelectorPpi->GetId (VariableName, VariableGuid, &VariableStorageInstanceId);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Temp Debug: Could not find the Variable Storage PPI ID for this variable!\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  for (Instance = 0; Status != EFI_NOT_FOUND; Instance++) {
    Status = PeiServicesLocatePpi (
               &gVariableStoragePpiGuid,
               Instance,
               &VariableStoragePpiDescriptor,
               (VOID **)&VariableStoragePpi
               );

    if (!EFI_ERROR (Status) && !EFI_ERROR (VariableStoragePpi->GetId (&DiscoveredInstanceGuid))) {
      DEBUG ((EFI_D_INFO, "Correct VariableStorageId GUID = %g\nDiscoveredGuid = %g\n", &VariableStorageInstanceId, &DiscoveredInstanceGuid));

      if (CompareGuid (&VariableStorageInstanceId, &DiscoveredInstanceGuid)) {
        //
        // Found the appropriate Variable Storage PPI. Now use it to get the variable.
        //
        Status = VariableStoragePpi->GetVariable (VariableStoragePpi, VariableName, VariableGuid, Attributes, DataSize, Data);
        if (Status == EFI_NOT_FOUND) {
          break;
        }
        return Status;
      }
    } else if (Status != EFI_NOT_FOUND) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
  }
  //
  // As a fallback, try searching the remaining VARIABLE_STORAGE_PPIs even
  // though the variable shouldn't actually be stored in there
  //
  Status = EFI_SUCCESS;
  for (Instance = 0; Status != EFI_NOT_FOUND; Instance++) {
    Status = PeiServicesLocatePpi (
               &gVariableStoragePpiGuid,
               Instance,
               &VariableStoragePpiDescriptor,
               (VOID **)&VariableStoragePpi
               );
    if (!EFI_ERROR (Status) && !EFI_ERROR (VariableStoragePpi->GetId (&DiscoveredInstanceGuid))) {
      if (!CompareGuid (&VariableStorageInstanceId, &DiscoveredInstanceGuid)) {
        Status = VariableStoragePpi->GetVariable (VariableStoragePpi, VariableName, VariableGuid, Attributes, DataSize, Data);
        DEBUG ((EFI_D_INFO, "Correct VariableStorageId GUID = %g\nDiscoveredGuid = %g\nStatus = %r\n", &VariableStorageInstanceId, &DiscoveredInstanceGuid, Status));
        if (!EFI_ERROR (Status)) {
          DEBUG ((EFI_D_INFO, "Name[%s] Guid[%g] not stored in correct storage PPI!\n", VariableName, VariableGuid));
        }
        if (Status != EFI_NOT_FOUND) {
          return Status;
        }
      }
    } else if (Status != EFI_NOT_FOUND) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
  }
  if (FeaturePcdGet (PcdMultiConfigSupported)) {
    Status = GetConfigData (SETUP_FOR_BIOS_POST, VariableName, VariableGuid, Attributes, DataSize, Data);
    if (Status == EFI_NOT_FOUND) {
      Status = GetConfigData (SETUP_FOR_LOAD_DEFAULT, VariableName, VariableGuid, Attributes, DataSize, Data);
    }
    if (!EFI_ERROR (Status)) {
      return Status;
    }
  }
  return EFI_NOT_FOUND;
}

/**
  Return the next variable name and GUID.

  This function is called multiple times to retrieve the VariableName
  and VariableGuid of all variables currently available in the system.
  On each call, the previous results are passed into the interface,
  and, on return, the interface returns the data for the next
  interface. When the entire variable list has been returned,
  EFI_NOT_FOUND is returned.

  @param  VariableNameSize  On entry, points to the size of the buffer pointed to by VariableName.
                            On return, the size of the variable name buffer.
  @param  VariableName      On entry, a pointer to a null-terminated string that is the variable's name.
                            On return, points to the next variable's null-terminated name string.
  @param  VariableGuid      On entry, a pointer to an EFI_GUID that is the variable's GUID.
                            On return, a pointer to the next variable's GUID.

  @retval EFI_SUCCESS           The variable was read successfully.
  @retval EFI_NOT_FOUND         The variable could not be found.
  @retval EFI_BUFFER_TOO_SMALL  The VariableNameSize is too small for the resulting
                                data. VariableNameSize is updated with the size
                                required for the specified variable.
  @retval EFI_INVALID_PARAMETER VariableName, VariableGuid or
                                VariableNameSize is NULL.
  @retval EFI_DEVICE_ERROR      The variable could not be retrieved because of a device error.

**/
EFI_STATUS
EFIAPI
PeiGetHobNextVariableName (
  IN OUT UINTN                              *VariableNameSize,
  IN OUT CHAR16                             *VariableName,
  IN OUT EFI_GUID                           *VariableGuid
  )
{
  VARIABLE_HOB_TYPE       Type;
  VARIABLE_POINTER_TRACK  Variable;
  VARIABLE_POINTER_TRACK  VariableInHob;
  VARIABLE_POINTER_TRACK  VariablePtrTrack;
  VARIABLE_STORE_INFO     StoreInfo;
  UINTN                   VarNameSize;
  EFI_STATUS              Status;
  VARIABLE_STORE_HEADER   *VariableStoreHeader[VariableHobTypeMax];

  if (VariableName == NULL || VariableGuid == NULL || VariableNameSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = FindVariableInHobs (VariableName, VariableGuid, &Variable, &StoreInfo);
  if (Variable.CurrPtr == NULL || Status != EFI_SUCCESS) {
    return Status;
  }

  if (VariableName[0] != 0) {
    //
    // If variable name is not NULL, get next variable
    //
    Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
  }

  VariableStoreHeader[VariableHobTypeDefault] = GetHobVariableStore (VariableHobTypeDefault, NULL);
  VariableStoreHeader[VariableHobTypeCache]   = GetHobVariableStore (VariableHobTypeCache, NULL);

  while (TRUE) {
    //
    // Switch from HOB to Non-Volatile.
    //
    while ((Variable.CurrPtr >= Variable.EndPtr) ||
           (Variable.CurrPtr == NULL)            ||
           !IsValidVariableHeader (Variable.CurrPtr)
          ) {
      //
      // Find current storage index
      //
      for (Type = (VARIABLE_HOB_TYPE) 0; Type < VariableHobTypeMax; Type++) {
        if ((VariableStoreHeader[Type] != NULL) && (Variable.StartPtr == GetStartPointer (VariableStoreHeader[Type]))) {
          break;
        }
      }
      ASSERT (Type < VariableHobTypeMax);
      //
      // Switch to next storage
      //
      for (Type++; Type < VariableHobTypeMax; Type++) {
        if (VariableStoreHeader[Type] != NULL) {
          break;
        }
      }
      //
      // Capture the case that
      // 1. current storage is the last one, or
      // 2. no further storage
      //
      if (Type >= VariableHobTypeMax) {
        return EFI_NOT_FOUND;
      }
      Variable.StartPtr = GetStartPointer (VariableStoreHeader[Type]);
      Variable.EndPtr   = GetEndPointer   (VariableStoreHeader[Type]);
      Variable.CurrPtr  = Variable.StartPtr;
    }

    if (Variable.CurrPtr->State == VAR_ADDED || Variable.CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
      if (Variable.CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
        //
        // If it is a IN_DELETED_TRANSITION variable,
        // and there is also a same ADDED one at the same time,
        // don't return it.
        //
        for (Type = (VARIABLE_HOB_TYPE) 0; Type < VariableHobTypeMax; Type++) {
          if ((VariableStoreHeader[Type] != NULL) && (Variable.StartPtr == GetStartPointer (VariableStoreHeader[Type]))) {
            break;
          }
        }
        ASSERT (Type < VariableHobTypeMax);
        if (Type == VariableHobTypeMax) {
          return EFI_NOT_FOUND;
        }
        GetHobVariableStore (Type, &StoreInfo);
        Status = FindVariableInHobsInternal  (
                   &StoreInfo,
                   GET_VARIABLE_NAME_PTR (Variable.CurrPtr),
                   &Variable.CurrPtr->VendorGuid,
                   &VariablePtrTrack
                   );
        if (!EFI_ERROR (Status) && VariablePtrTrack.CurrPtr->State == VAR_ADDED) {
          Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
          continue;
        }
      }

      //
      // Don't return NV variable when HOB overrides it
      //
      if ((VariableStoreHeader[VariableHobTypeDefault] != NULL) && (VariableStoreHeader[VariableHobTypeCache] != NULL) &&
          (Variable.StartPtr == GetStartPointer (VariableStoreHeader[VariableHobTypeCache]))
         ) {
        Status = FindVariableInHobsInternal  (
                   &StoreInfo,
                   GET_VARIABLE_NAME_PTR (Variable.CurrPtr),
                   &Variable.CurrPtr->VendorGuid,
                   &VariableInHob
                   );
        if (!EFI_ERROR (Status)) {
          Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
          continue;
        }
      }

      VarNameSize = NameSizeOfVariable (Variable.CurrPtr);
      ASSERT (VarNameSize != 0);

      if (VarNameSize <= *VariableNameSize) {
        CopyMem (VariableName, GET_VARIABLE_NAME_PTR (Variable.CurrPtr), VarNameSize);

        CopyMem (VariableGuid, &Variable.CurrPtr->VendorGuid, sizeof (EFI_GUID));

        Status = EFI_SUCCESS;
      } else {
        Status = EFI_BUFFER_TOO_SMALL;
      }

      *VariableNameSize = VarNameSize;
      //
      // Variable is found
      //
      return Status;
    } else {
      Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
    }
  }
}



/**
  Determines if a variable exists in the variable store HOBs

  @param  VariableName          A pointer to a null-terminated string that is the variable's name.
  @param  VariableGuid          A pointer to an EFI_GUID that is the variable's GUID. The combination of
                                VariableGuid and VariableName must be unique.

  @retval TRUE                  The variable exists in the HOBs
  @retval FALSE                 The variable does not exist in the HOBs

**/
BOOLEAN
EFIAPI
VariableExistsInHob (
  IN CONST  CHAR16                          *VariableName,
  IN CONST  EFI_GUID                        *VariableGuid
  )
{
  EFI_STATUS              Status;
  VARIABLE_POINTER_TRACK  HobVariable;
  VARIABLE_STORE_INFO     HobStoreInfo;

  Status = FindVariableInHobs (VariableName, VariableGuid, &HobVariable, &HobStoreInfo);
  if ((HobVariable.CurrPtr != NULL) && (!EFI_ERROR (Status))) {
    return TRUE;
  } else {
    if (Status == EFI_NOT_FOUND) {
      return FALSE;
    }
    ASSERT_EFI_ERROR (Status);
    return FALSE;
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

  @param  This              A pointer to this instance of the EFI_PEI_READ_ONLY_VARIABLE2_PPI.

  @param  VariableNameSize  On entry, points to the size of the buffer pointed to by VariableName.
                            On return, the size of the variable name buffer.
  @param  VariableName      On entry, a pointer to a null-terminated string that is the variable's name.
                            On return, points to the next variable's null-terminated name string.
  @param  VariableGuid      On entry, a pointer to an EFI_GUID that is the variable's GUID.
                            On return, a pointer to the next variable's GUID.

  @retval EFI_SUCCESS           The variable was read successfully.
  @retval EFI_NOT_FOUND         The variable could not be found.
  @retval EFI_BUFFER_TOO_SMALL  The VariableNameSize is too small for the resulting
                                data. VariableNameSize is updated with the size
                                required for the specified variable.
  @retval EFI_INVALID_PARAMETER VariableName, VariableGuid or
                                VariableNameSize is NULL.
  @retval EFI_DEVICE_ERROR      The variable could not be retrieved because of a device error.

**/
EFI_STATUS
EFIAPI
PeiGetNextVariableName (
  IN CONST  EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
  IN OUT UINTN                              *VariableNameSize,
  IN OUT CHAR16                             *VariableName,
  IN OUT EFI_GUID                           *VariableGuid
  )
{
  EFI_GUID                      DiscoveredInstanceGuid;
  EFI_GUID                      VariableStorageInstanceId;
  EFI_PEI_PPI_DESCRIPTOR        *VariableStoragePpiDescriptor;
  EFI_STATUS                    Status;
  VARIABLE_STORAGE_PPI          *VariableStoragePpi;
  VARIABLE_STORAGE_SELECTOR_PPI *VariableStorageSelectorPpi;
  BOOLEAN                       SearchComplete;
  UINTN                         Instance;
  UINT32                        VarAttributes;

  if (VariableName == NULL || VariableGuid == NULL || VariableNameSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check the HOBs first
  //
  Status = PeiGetHobNextVariableName (VariableNameSize, VariableName, VariableGuid);
  if (!EFI_ERROR (Status)) {
    return Status;
  } else if (Status != EFI_NOT_FOUND) {
    return Status;
  }
  //
  // If VariableName is an empty string or we reached the end of the HOBs,
  // get the first variable from the first Variable Storage PPI
  //
  if (VariableName[0] == 0 || (Status == EFI_NOT_FOUND && VariableExistsInHob (VariableName, VariableGuid))) {
    ZeroMem ((VOID *) VariableName, *VariableNameSize);
    ZeroMem ((VOID *) VariableGuid, sizeof (VariableGuid));
    Status = PeiServicesLocatePpi (
               &gVariableStoragePpiGuid,
               0,
               &VariableStoragePpiDescriptor,
               (VOID **)&VariableStoragePpi
               );
    if (!EFI_ERROR (Status)) {
        Status = VariableStoragePpi->GetNextVariableName (
                  VariableStoragePpi,
                  VariableNameSize,
                  VariableName,
                  VariableGuid,
                  &VarAttributes
                  );
        if (EFI_ERROR (Status)) {
          DEBUG ((DEBUG_INFO, "VariableStoragePpi->GetNextVariableName status %r\n", Status));
        }
    }
    return Status;
  }

  Status = PeiServicesLocatePpi (
             &gVariableStorageSelectorPpiGuid,
             0,
             NULL,
             (VOID **) &VariableStorageSelectorPpi
             );
  if (Status == EFI_NOT_FOUND) {
    return Status;
  } else if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  //
  // All of the HOB variables have been already enumerated, enumerate the Variable Storage PPIs
  //
  SearchComplete = FALSE;
  while (!SearchComplete) {
    Status = VariableStorageSelectorPpi->GetId (VariableName, VariableGuid, &VariableStorageInstanceId);
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }

    for (Instance = 0; Status != EFI_NOT_FOUND; Instance++) {
      Status = PeiServicesLocatePpi (
                 &gVariableStoragePpiGuid,
                 Instance,
                 &VariableStoragePpiDescriptor,
                 (VOID **)&VariableStoragePpi
                 );
      if (!EFI_ERROR (Status)) {
        Status = VariableStoragePpi->GetId (&DiscoveredInstanceGuid);
        if (!EFI_ERROR (Status)) {
          if (CompareGuid (&VariableStorageInstanceId, &DiscoveredInstanceGuid)) {
            Status = VariableStoragePpi->GetNextVariableName (
                      VariableStoragePpi,
                      VariableNameSize,
                      VariableName,
                      VariableGuid,
                      &VarAttributes
                      );
            if (!EFI_ERROR (Status)) {
              if (VariableExistsInHob (VariableName, VariableGuid)) {
                //
                // Don't return this variable if there is a HOB variable that overrides it
                // advance to the next variable
                //
                break;
              }
              return Status;
            } else if (Status == EFI_NOT_FOUND) {
              //
              // If we reached the end of the variables in the current variable
              // storage PPI, get the first variable in the next variable storage PPI
              //
              ZeroMem ((VOID *) VariableName, *VariableNameSize);
              ZeroMem ((VOID *) VariableGuid, sizeof (VariableGuid));
              Status = PeiServicesLocatePpi (
                         &gVariableStoragePpiGuid,
                         Instance + 1,
                         &VariableStoragePpiDescriptor,
                         (VOID **)&VariableStoragePpi
                         );
              if (!EFI_ERROR (Status)) {
                Status = VariableStoragePpi->GetNextVariableName (
                          VariableStoragePpi,
                          VariableNameSize,
                          VariableName,
                          VariableGuid,
                          &VarAttributes
                          );
                if (!EFI_ERROR (Status)) {
                  if (VariableExistsInHob (VariableName, VariableGuid)) {
                    //
                    // Don't return this variable if there is a HOB variable that overrides it
                    // advance to the next variable
                    //
                    break;
                  }
                }
                return Status;
              } else if (Status == EFI_NOT_FOUND) {
                // This is the last variable
                SearchComplete = TRUE;
                break;
              } else {
                ASSERT_EFI_ERROR (Status);
                return Status;
              }
            } else {
              return Status;
            }
          }
        } else {
          ASSERT_EFI_ERROR (Status);
          return Status;
        }
      } else if (Status == EFI_NOT_FOUND) {
        SearchComplete = TRUE;
      } else {
        ASSERT_EFI_ERROR (Status);
        return Status;
      }
    }
  }
  return EFI_NOT_FOUND;
}

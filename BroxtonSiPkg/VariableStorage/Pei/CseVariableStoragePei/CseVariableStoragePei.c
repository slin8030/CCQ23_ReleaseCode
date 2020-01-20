/** @file
  This PEIM implements CSE Variable Storage Services and installs
  an instance of the VariableStorage PPI.

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

#include "CseVariableStoragePei.h"

//
// Module globals
//
static VARIABLE_STORAGE_PPI mCseVariableStoragePpi = {
  PeiCseVariableStorageGetId,
  PeiCseVariableStorageGetVariable,
  PeiCseVariableStorageGetNextVariableName
};

static EFI_PEI_PPI_DESCRIPTOR     mPpiListVariable = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gVariableStoragePpiGuid,
  &mCseVariableStoragePpi
};

/**
  Initializes data structures used by the PEIM.

  @param  None

  @retval EFI_SUCCESS  The data structures were initialized successfully.
  @retval Others       An error occurred initializing the data structures.
**/
EFI_STATUS
EFIAPI
PeimInitializeDataStructures (
  VOID
  )
{
  EFI_STATUS              Status;
  UINTN                   CseVariableFileInfoHobSize    = 0;
  EFI_HOB_GUID_TYPE       *CseVariableFileInfoHob       = NULL;
  CSE_VARIABLE_FILE_INFO  **CseVariableFileInfo         = NULL;

  CseVariableFileInfoHob = GetFirstGuidHob (&gCseVariableFileInfoHobGuid);

  if (CseVariableFileInfoHob == NULL) {
    //
    // Create the variable file info HOB for the first time
    //
    DEBUG ((EFI_D_INFO, "Initializing CSE Variable PEI storage structures\n"));

    CseVariableFileInfoHobSize = sizeof (CSE_VARIABLE_FILE_INFO *) * CseVariableFileTypeMax;

    CseVariableFileInfo = (CSE_VARIABLE_FILE_INFO  **) BuildGuidHob (&gCseVariableFileInfoHobGuid, CseVariableFileInfoHobSize);
  } else {
    CseVariableFileInfo = GET_GUID_HOB_DATA (CseVariableFileInfoHob);
  }

  if (CseVariableFileInfo == NULL) {
    ASSERT (CseVariableFileInfo != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem (CseVariableFileInfo, CseVariableFileInfoHobSize);

  Status = InitializeCseStorageGlobalVariableStructures (CseVariableFileInfo, FALSE);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  //
  // Load existing variable stores if they exist
  //
  Status = EstablishAndLoadCseVariableStores (CseVariableFileInfo, FALSE);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
  }

  return Status;
}

/**
  Provide the functionality of CSE NVM variable storage services.

  @param  FileHandle   Handle of the file being invoked.
                       Type EFI_PEI_FILE_HANDLE is defined in FfsFindNextFile().
  @param  PeiServices  General purpose services available to every PEIM.

  @retval EFI_SUCCESS  If the interface could be successfully installed
  @retval Others       Returned from PeiServicesInstallPpi()
**/
EFI_STATUS
EFIAPI
PeimInitializeCseVariableStorageServices (
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
{
  BOOLEAN                 ImageInPermanentMemory;
  UINTN                   Instance;
  EFI_GUID                DiscoveredInstanceGuid;
//[-start-160812-IB07220128-modify]//
  EFI_STATUS              Status = EFI_SUCCESS;
//[-end-160812-IB07220128-modify]//
  EFI_PEI_PPI_DESCRIPTOR  *VariableStoragePpiDescriptor = NULL;
  VARIABLE_STORAGE_PPI    *VariableStoragePpi           = NULL;

  ImageInPermanentMemory = FALSE;

//[-start-161020-IB07400800-modify]//
//  if (PcdGetBool (PcdForceVolatileVariable) || !PcdGetBool (PcdEnableCseVariableStorage)) {
  if (!PcdGetBool (PcdEnableCseVariableStorage)) {
//[-end-161020-IB07400800-modify]//
    return EFI_SUCCESS;
  }

  if (PcdGetBool (PcdReinstallCseVariablePpi)) {
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
    Status = PeiServicesInstallPpi (&mPpiListVariable);
    ASSERT_EFI_ERROR (Status);
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
        if (CompareGuid (&gCseVariableStoragePpiInstanceGuid, &DiscoveredInstanceGuid)) {
          Status = PeiServicesReInstallPpi (VariableStoragePpiDescriptor, &mPpiListVariable);

          if (EFI_ERROR (Status)) {
            ASSERT_EFI_ERROR (Status);
            return Status;
          }

          break;
        }
      } else if (Status != EFI_NOT_FOUND) {
        ASSERT_EFI_ERROR (Status);
        return Status;
      }
    }

    if (Status == EFI_NOT_FOUND) {
      //
      // This PEIM did not install an instance of VARIABLE_STORAGE_PPI
      // Install the instance for the first time
      //
      Status = PeiServicesInstallPpi (&mPpiListVariable);

      if (EFI_ERROR (Status)) {
        ASSERT_EFI_ERROR (Status);
        return Status;
      }
    }
  }

  Status = PeimInitializeDataStructures ();
  ASSERT_EFI_ERROR (Status);

  return Status;
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
PeiCseVariableStorageGetId (
  OUT       EFI_GUID                        *InstanceGuid
  )
{
  if (InstanceGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (InstanceGuid, &gCseVariableStoragePpiInstanceGuid, sizeof (EFI_GUID));

  return EFI_SUCCESS;
}

/**
  This service retrieves a variable's value using its name and GUID.

  Read the specified variable from the CSE NVM variable store. If the Data
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
PeiCseVariableStorageGetVariable (
  IN CONST  VARIABLE_STORAGE_PPI            *This,
  IN CONST  CHAR16                          *VariableName,
  IN CONST  EFI_GUID                        *VariableGuid,
  OUT       UINT32                          *Attributes,
  IN OUT    UINTN                           *DataSize,
  OUT       VOID                            *Data
  )
{
  EFI_BOOT_MODE          BootMode;
  EFI_STATUS             Status;
  CSE_VARIABLE_FILE_TYPE VariableCseFileType;

  EFI_HOB_GUID_TYPE      *CseVariableFileInfoHob = NULL;
  CSE_VARIABLE_FILE_INFO **CseVariableFileInfo   = NULL;

  //
  // Check input parameters
  //
  if (VariableName == NULL || VariableGuid == NULL || DataSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CseVariableFileInfoHob = GetFirstGuidHob (&gCseVariableFileInfoHobGuid);
  ASSERT (CseVariableFileInfoHob != NULL);

  CseVariableFileInfo = GET_GUID_HOB_DATA (CseVariableFileInfoHob);

  VariableCseFileType = GetCseVariableStoreFileType (VariableName, VariableGuid, CseVariableFileInfo);

  Status = PeiServicesGetBootMode (&BootMode);
  if (!EFI_ERROR (Status) && BootMode == BOOT_ON_S3_RESUME && VariableCseFileType == CseVariableFileTypePreMemoryFile) {
    //
    // All variables read in S3 resume must be in the pre-memory file.
    // The pre-memory file contents are read from CSE SRAM and cached in PEI.
    // CSE variable services are not guaranteed in post-memory PEI on S3 resume.
    //
    ASSERT (BootMode == BOOT_ON_S3_RESUME && VariableCseFileType != CseVariableFileTypePreMemoryFile);
  }

  DEBUG ((EFI_D_INFO, "Attempting to read CSE NVM variable.\n  Name=\'%ls\'\n  GUID=%g\n", \
          VariableName, VariableGuid));

  DEBUG ((EFI_D_INFO, "CseVariableFileInfoHob = 0x%x\n", CseVariableFileInfoHob));
  DEBUG ((EFI_D_INFO, "CseVariableFileInfo = 0x%x\n", CseVariableFileInfo));

  return GetCseVariable (CseVariableFileInfo, VariableName, VariableGuid, Attributes, DataSize, Data, NULL);
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
PeiCseVariableStorageGetNextVariableName (
  IN CONST  VARIABLE_STORAGE_PPI            *This,
  IN OUT    UINTN                           *VariableNameSize,
  IN OUT    CHAR16                          *VariableName,
  IN OUT    EFI_GUID                        *VariableGuid,
  OUT       UINT32                          *VariableAttributes
  )
{
  EFI_STATUS             Status;
  BOOLEAN                IsAuthVariable;
  UINTN                  VarNameSize;
  VARIABLE_NVM_HEADER    *VariablePtr;

  EFI_HOB_GUID_TYPE      *CseVariableFileInfoHob = NULL;
  CSE_VARIABLE_FILE_INFO **CseVariableFileInfo   = NULL;

  if (VariableNameSize == NULL || VariableName == NULL || VariableGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CseVariableFileInfoHob = GetFirstGuidHob (&gCseVariableFileInfoHobGuid);
  ASSERT (CseVariableFileInfoHob != NULL);

  CseVariableFileInfo = GET_GUID_HOB_DATA (CseVariableFileInfoHob);

  Status = GetNextCseVariableName (VariableName, VariableGuid, CseVariableFileInfo, &VariablePtr, &IsAuthVariable);

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

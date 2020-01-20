/** @file
  Variable storage routines used to interface with variable storage drivers.

  Caution: This module requires additional review when modified.
  This driver will have external input - variable data. They may be input in SMM mode.
  This external input must be validated carefully to avoid security issue like
  buffer overflow, integer overflow.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "VariableStorage.h"

extern VARIABLE_MODULE_GLOBAL   *mVariableModuleGlobal;

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
  *VariableStorageProtocol = NULL;

  VariableStorageSelectorProtocol = mVariableModuleGlobal->VariableGlobal.VariableStorageSelectorProtocol;
  if (VariableStorageSelectorProtocol == NULL) {
    return EFI_NOT_FOUND;
  }

  ZeroMem ((VOID *) &VariableStorageId, sizeof (EFI_GUID));

  Status = VariableStorageSelectorProtocol->GetId (VariableName, VendorGuid, &VariableStorageId);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for ( Index = 0;
        Index < mVariableModuleGlobal->VariableGlobal.VariableStoresCount;
        Index++) {
    CurrentInstance = mVariableModuleGlobal->VariableGlobal.VariableStores[Index];
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


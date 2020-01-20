/** @file
    Implementation functions and structures for var check protocol.

;******************************************************************************
;* Copyright (c) 2015 - 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/** @file
  Implementation functions and structures for var check protocol.

Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Variable.h"
#include "VariableLock.h"
#include "VarCheck.h"
#include <Library/DevicePathLib.h>

extern LIST_ENTRY mLockedVariableList;
#define VAR_CHECK_HANDLER_TABLE_SIZE    0x8

UINT32                                  mNumberOfHandler = 0;
UINT32                                  mMaxNumberOfHandler = 0;
VAR_CHECK_SET_VARIABLE_CHECK_HANDLER    *mHandlerTable = NULL;
LIST_ENTRY                              mVarCheckVariableList = INITIALIZE_LIST_HEAD_VARIABLE (mVarCheckVariableList);

/**
  Internal check for load option.

  @param[in] VariablePropery    Pointer to variable property.
  @param[in] DataSize           Data size.
  @param[in] Data               Pointer to data buffer.

  @retval EFI_SUCCESS           The SetVariable check result was success.
  @retval EFI_INVALID_PARAMETER The data buffer is not a valid load option.

**/
EFI_STATUS
EFIAPI
InternalVarCheckLoadOption (
  IN VAR_CHECK_VARIABLE_PROPERTY    *VariablePropery,
  IN UINTN                          DataSize,
  IN VOID                           *Data
  )
{

  UINT16                    FilePathListLength;
  CHAR16                    *Description;
  EFI_DEVICE_PATH_PROTOCOL  *FilePathList;

  FilePathListLength = *((UINT16 *) ((UINTN) Data + sizeof (UINT32)));

  //
  // Check Description
  //
  Description = (CHAR16 *) ((UINTN) Data + sizeof (UINT32) + sizeof (UINT16));
  while (Description < (CHAR16 *) ((UINTN) Data + DataSize)) {
    if (*Description == L'\0') {
      break;
    }
    Description++;
  }
  if ((UINTN) Description >= ((UINTN) Data + DataSize)) {
    return EFI_INVALID_PARAMETER;
  }
  Description++;

  //
  // Check FilePathList
  //
  FilePathList = (EFI_DEVICE_PATH_PROTOCOL *) Description;
  if ((UINTN) FilePathList > (MAX_ADDRESS - FilePathListLength)) {
    return EFI_INVALID_PARAMETER;
  }
  if (((UINTN) FilePathList + FilePathListLength) > ((UINTN) Data + DataSize)) {
    return EFI_INVALID_PARAMETER;
  }
  if (FilePathListLength < sizeof (EFI_DEVICE_PATH_PROTOCOL)) {
    return EFI_INVALID_PARAMETER;
  }
  if (!IsDevicePathValid (FilePathList, FilePathListLength)) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

/**
  Internal check for key option.

  @param[in] VariablePropery    Pointer to variable property.
  @param[in] DataSize           Data size.
  @param[in] Data               Pointer to data buffer.

  @retval EFI_SUCCESS           The SetVariable check result was success.
  @retval EFI_INVALID_PARAMETER The data buffer is not a valid key option.

**/
EFI_STATUS
EFIAPI
InternalVarCheckKeyOption (
  IN VAR_CHECK_VARIABLE_PROPERTY    *VariablePropery,
  IN UINTN                          DataSize,
  IN VOID                           *Data
  )
{
  if (((DataSize - sizeof (EFI_KEY_OPTION)) % sizeof (EFI_INPUT_KEY)) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

/**
  Internal check for device path.

  @param[in] VariablePropery    Pointer to variable property.
  @param[in] DataSize           Data size.
  @param[in] Data               Pointer to data buffer.

  @retval EFI_SUCCESS           The SetVariable check result was success.
  @retval EFI_INVALID_PARAMETER The data buffer is not a valid device path.

**/
EFI_STATUS
EFIAPI
InternalVarCheckDevicePath (
  IN VAR_CHECK_VARIABLE_PROPERTY    *VariablePropery,
  IN UINTN                          DataSize,
  IN VOID                           *Data
  )
{
  if (!IsDevicePathValid ((EFI_DEVICE_PATH_PROTOCOL *) Data, DataSize)) {
    return EFI_INVALID_PARAMETER;
  }
  return EFI_SUCCESS;
}

/**
  Internal check for ASCII string.

  @param[in] VariablePropery    Pointer to variable property.
  @param[in] DataSize           Data size.
  @param[in] Data               Pointer to data buffer.

  @retval EFI_SUCCESS           The SetVariable check result was success.
  @retval EFI_INVALID_PARAMETER The data buffer is not a Null-terminated ASCII string.

**/
EFI_STATUS
EFIAPI
InternalVarCheckAsciiString (
  IN VAR_CHECK_VARIABLE_PROPERTY    *VariablePropery,
  IN UINTN                          DataSize,
  IN VOID                           *Data
  )
{
  CHAR8     *String;
  UINTN     Index;

  String = (CHAR8 *) Data;
  if (String[DataSize - 1] == '\0') {
    return EFI_SUCCESS;
  } else {
    for (Index = 1; Index < DataSize && (String[DataSize - 1 - Index] != '\0'); Index++);
    if (Index == DataSize) {
      return EFI_INVALID_PARAMETER;
    }
  }
  return EFI_SUCCESS;
}

/**
  Internal check for size array.

  @param[in] VariablePropery    Pointer to variable property.
  @param[in] DataSize           Data size.
  @param[in] Data               Pointer to data buffer.

  @retval EFI_SUCCESS           The SetVariable check result was success.
  @retval EFI_INVALID_PARAMETER The DataSize is not size array.

**/
EFI_STATUS
EFIAPI
InternalVarCheckSizeArray (
  IN VAR_CHECK_VARIABLE_PROPERTY    *VariablePropery,
  IN UINTN                          DataSize,
  IN VOID                           *Data
  )
{
  if ((DataSize % VariablePropery->MinSize) != 0) {
    return EFI_INVALID_PARAMETER;
  }
  return EFI_SUCCESS;
}

//
// To prevent name collisions with possible future globally defined variables,
// other internal firmware data variables that are not defined here must be
// saved with a unique VendorGuid other than EFI_GLOBAL_VARIABLE or
// any other GUID defined by the UEFI Specification. Implementations must
// only permit the creation of variables with a UEFI Specification-defined
// VendorGuid when these variables are documented in the UEFI Specification.
//
UEFI_DEFINED_VARIABLE_ENTRY mGlobalVariableList[] = {
  {
    EFI_LANG_CODES_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_BS_RT,
      1,
      MAX_UINTN
    },
    InternalVarCheckAsciiString
  },
  {
    EFI_LANG_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      1,
      MAX_UINTN
    },
    InternalVarCheckAsciiString
  },
  {
    EFI_TIME_OUT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (UINT16),
      sizeof (UINT16)
    },
    NULL
  },
  {
    EFI_PLATFORM_LANG_CODES_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_BS_RT,
      1,
      MAX_UINTN
    },
    InternalVarCheckAsciiString
  },
  {
    EFI_PLATFORM_LANG_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      1,
      MAX_UINTN
    },
    InternalVarCheckAsciiString
  },
  {
    EFI_CON_IN_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (EFI_DEVICE_PATH_PROTOCOL),
      MAX_UINTN
    },
    InternalVarCheckDevicePath
  },
  {
    EFI_CON_OUT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (EFI_DEVICE_PATH_PROTOCOL),
      MAX_UINTN
    },
    InternalVarCheckDevicePath
  },
  {
    EFI_ERR_OUT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (EFI_DEVICE_PATH_PROTOCOL),
      MAX_UINTN
    },
    InternalVarCheckDevicePath
  },
  {
    EFI_CON_IN_DEV_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_BS_RT,
      sizeof (EFI_DEVICE_PATH_PROTOCOL),
      MAX_UINTN
    },
    InternalVarCheckDevicePath
  },
  {
    EFI_CON_OUT_DEV_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_BS_RT,
      sizeof (EFI_DEVICE_PATH_PROTOCOL),
      MAX_UINTN
    },
    InternalVarCheckDevicePath
  },
  {
    EFI_ERR_OUT_DEV_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_BS_RT,
      sizeof (EFI_DEVICE_PATH_PROTOCOL),
      MAX_UINTN
    },
    InternalVarCheckDevicePath
  },
  {
    EFI_BOOT_ORDER_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (UINT16),
      MAX_UINTN
    },
    InternalVarCheckSizeArray
  },
  {
    EFI_BOOT_NEXT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (UINT16),
      sizeof (UINT16)
    },
    NULL
  },
  {
    EFI_BOOT_CURRENT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_BS_RT,
      sizeof (UINT16),
      sizeof (UINT16)
    },
    NULL
  },
  {
    EFI_BOOT_OPTION_SUPPORT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_BS_RT,
      sizeof (UINT32),
      sizeof (UINT32)
    },
    NULL
  },
  {
    EFI_DRIVER_ORDER_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (UINT16),
      MAX_UINTN
    },
    InternalVarCheckSizeArray
  },
  {
    EFI_HW_ERR_REC_SUPPORT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (UINT16),
      sizeof (UINT16)
    },
    NULL
  },
  {
    EFI_SETUP_MODE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_BS_RT,
      sizeof (UINT8),
      sizeof (UINT8)
    },
    NULL
  },
  {
    EFI_KEY_EXCHANGE_KEY_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT_AT,
      1,
      MAX_UINTN
    },
    NULL
  },
  {
    EFI_PLATFORM_KEY_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT_AT,
      1,
      MAX_UINTN
    },
    NULL
  },
  {
    EFI_SIGNATURE_SUPPORT_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_BS_RT,
      sizeof (EFI_GUID),
      MAX_UINTN
    },
    InternalVarCheckSizeArray
  },
  {
    EFI_SECURE_BOOT_MODE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_BS_RT,
      sizeof (UINT8),
      sizeof (UINT8)
    },
    NULL
  },
  {
    EFI_KEK_DEFAULT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_BS_RT,
      1,
      MAX_UINTN
    },
    NULL
  },
  {
    EFI_PK_DEFAULT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_BS_RT,
      1,
      MAX_UINTN
    },
    NULL
  },
  {
    EFI_DB_DEFAULT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_BS_RT,
      1,
      MAX_UINTN
    },
    NULL
  },
  {
    EFI_DBX_DEFAULT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_BS_RT,
      1,
      MAX_UINTN
    },
    NULL
  },
  {
    EFI_DBT_DEFAULT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_BS_RT,
      1,
      MAX_UINTN
    },
    NULL
  },
  {
    EFI_OS_INDICATIONS_SUPPORT_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_BS_RT,
      sizeof (UINT64),
      sizeof (UINT64)
    },
    NULL
  },
  {
    EFI_OS_INDICATIONS_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (UINT64),
      sizeof (UINT64)
    },
    NULL
  },
  {
    EFI_VENDOR_KEYS_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_BS_RT,
      sizeof (UINT8),
      sizeof (UINT8)
    },
    NULL
  },
  {
    EFI_AUDIT_MODE_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_BS_RT,
      sizeof (UINT8),
      sizeof (UINT8)
    },
    NULL
  },
  {
    EFI_DEPLOYED_MODE_VARIABLE_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_BS_RT,
      sizeof (UINT8),
      sizeof (UINT8)
    },
    NULL
  },
};
UEFI_DEFINED_VARIABLE_ENTRY mGlobalVariableList2[] = {
  {
    L"Boot####",
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (UINT32) + sizeof (UINT16),
      MAX_UINTN
    },
    InternalVarCheckLoadOption
  },
  {
    L"Driver####",
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (UINT32) + sizeof (UINT16),
      MAX_UINTN
    },
    InternalVarCheckLoadOption
  },
  {
    L"Key####",
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (EFI_KEY_OPTION),
      sizeof (EFI_KEY_OPTION) + 3 * sizeof (EFI_INPUT_KEY)
    },
    InternalVarCheckKeyOption
  },
};

//
// EFI_IMAGE_SECURITY_DATABASE_GUID
//
UEFI_DEFINED_VARIABLE_ENTRY mImageSecurityVariableList[] = {
  {
    EFI_IMAGE_SECURITY_DATABASE,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT_AT,
      1,
      MAX_UINTN
    },
    NULL
  },
  {
    EFI_IMAGE_SECURITY_DATABASE1,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT_AT,
      1,
      MAX_UINTN
    },
    NULL
  },
  {
    EFI_IMAGE_SECURITY_DATABASE2,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT_AT,
      1,
      MAX_UINTN
    },
    NULL
  },
};

typedef struct {
  EFI_GUID                      *Guid;
  CHAR16                        *Name;
  VAR_CHECK_VARIABLE_PROPERTY   VariableProperty;
  INTERNAL_VAR_CHECK_FUNCTION   CheckFunction;
} VARIABLE_DRIVER_VARIABLE_ENTRY;

VARIABLE_DRIVER_VARIABLE_ENTRY mVariableDriverVariableList[] = {
//  {
//    &gEfiSecureBootEnableDisableGuid,
//    EFI_SECURE_BOOT_ENABLE_NAME,
//    {
//      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
//      0,
//      VARIABLE_ATTRIBUTE_NV_BS,
//      sizeof (UINT8),
//      sizeof (UINT8)
//    },
//    NULL
//  },
//  {
//    &gEfiCustomModeEnableGuid,
//    EFI_CUSTOM_MODE_NAME,
//    {
//      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
//      0,
//      VARIABLE_ATTRIBUTE_NV_BS,
//      sizeof (UINT8),
//      sizeof (UINT8)
//    },
//    NULL
//  },
//  {
//    &gEfiVendorKeysNvGuid,
//    EFI_VENDOR_KEYS_NV_VARIABLE_NAME,
//    {
//      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
//      0,
//      VARIABLE_ATTRIBUTE_NV_BS_RT_AT,
//      sizeof (UINT8),
//      sizeof (UINT8)
//    },
//    NULL
//  },
  {
    &gEfiAuthenticatedVariableGuid,
    L"AuthVarKeyDatabase",
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT_AW,
      sizeof (UINT8),
      MAX_UINTN
    },
    NULL
  },
  {
    &gEfiGenericVariableGuid,
    L"certdb",
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      0,
      VARIABLE_ATTRIBUTE_NV_BS_RT_AT,
      sizeof (UINT32),
      MAX_UINTN
    },
    NULL
  },
  {
    &gEdkiiVarErrorFlagGuid,
    VAR_ERROR_FLAG_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (VAR_ERROR_FLAG),
      sizeof (VAR_ERROR_FLAG)
    },
    NULL
  },
};

/**
  Get UEFI defined global variable or image security database variable property.
  The code will check if variable guid is global variable or image security database guid first.
  If yes, further check if variable name is in mGlobalVariableList, mGlobalVariableList2 or mImageSecurityVariableList.

  @param[in]  VariableName      Pointer to variable name.
  @param[in]  VendorGuid        Variable Vendor Guid.
  @param[in]  WildcardMatch     Try wildcard match or not.
  @param[out] VariableProperty  Pointer to variable property.
  @param[out] VarCheckFunction  Pointer to check function.

  @retval EFI_SUCCESS           Variable is not global variable or image security database variable.
  @retval EFI_INVALID_PARAMETER Variable is global variable or image security database variable, but variable name is not in the lists.

**/
EFI_STATUS
GetUefiDefinedVariableProperty (
  IN CHAR16                         *VariableName,
  IN EFI_GUID                       *VendorGuid,
  IN BOOLEAN                        WildcardMatch,
  OUT VAR_CHECK_VARIABLE_PROPERTY   **VariableProperty,
  OUT INTERNAL_VAR_CHECK_FUNCTION   *VarCheckFunction OPTIONAL
  )
{
  UINTN     Index;
  UINTN     NameLength;

  if (CompareGuid (VendorGuid, &gEfiGlobalVariableGuid)){
    //
    // Try list 1, exactly match.
    //
    for (Index = 0; Index < sizeof (mGlobalVariableList)/sizeof (mGlobalVariableList[0]); Index++) {
      if (StrCmp (mGlobalVariableList[Index].Name, VariableName) == 0) {
        if (VarCheckFunction != NULL) {
          *VarCheckFunction = mGlobalVariableList[Index].CheckFunction;
        }
        *VariableProperty = &mGlobalVariableList[Index].VariableProperty;
        return EFI_SUCCESS;
      }
    }

    //
    // Try list 2.
    //
    NameLength = StrLen (VariableName) - 4;
    for (Index = 0; Index < sizeof (mGlobalVariableList2)/sizeof (mGlobalVariableList2[0]); Index++) {
      if (WildcardMatch) {
        if ((StrLen (VariableName) == StrLen (mGlobalVariableList2[Index].Name)) &&
            (StrnCmp (mGlobalVariableList2[Index].Name, VariableName, NameLength) == 0) &&
            IsHexaDecimalDigitCharacter (VariableName[NameLength]) &&
            IsHexaDecimalDigitCharacter (VariableName[NameLength + 1]) &&
            IsHexaDecimalDigitCharacter (VariableName[NameLength + 2]) &&
            IsHexaDecimalDigitCharacter (VariableName[NameLength + 3])) {
          if (VarCheckFunction != NULL) {
            *VarCheckFunction = mGlobalVariableList2[Index].CheckFunction;
          }
          *VariableProperty = &mGlobalVariableList2[Index].VariableProperty;
          return EFI_SUCCESS;
        }
      }
      if (StrCmp (mGlobalVariableList2[Index].Name, VariableName) == 0) {
        if (VarCheckFunction != NULL) {
          *VarCheckFunction = mGlobalVariableList2[Index].CheckFunction;
        }
        *VariableProperty = &mGlobalVariableList2[Index].VariableProperty;
        return EFI_SUCCESS;
      }
    }

    //
    // The variable name is not in the lists.
    //
    return EFI_INVALID_PARAMETER;
  }

  if (CompareGuid (VendorGuid, &gEfiImageSecurityDatabaseGuid)){
    for (Index = 0; Index < sizeof (mImageSecurityVariableList)/sizeof (mImageSecurityVariableList[0]); Index++) {
      if (StrCmp (mImageSecurityVariableList[Index].Name, VariableName) == 0) {
        if (VarCheckFunction != NULL) {
          *VarCheckFunction = mImageSecurityVariableList[Index].CheckFunction;
        }
        *VariableProperty = &mImageSecurityVariableList[Index].VariableProperty;
        return EFI_SUCCESS;
      }
    }

    return EFI_INVALID_PARAMETER;
  }

  //
  // It is not global variable or image security database variable.
  //
  return EFI_SUCCESS;
}

/**
  Get variable property for variables managed by Varaible driver.

  @param[in]  VariableName      Pointer to variable name.
  @param[in]  VendorGuid        Variable Vendor Guid.

  @return Pointer to variable property.

**/
VAR_CHECK_VARIABLE_PROPERTY *
GetVariableDriverVariableProperty (
  IN CHAR16                         *VariableName,
  IN EFI_GUID                       *VendorGuid
  )
{
  UINTN     Index;

  for (Index = 0; Index < sizeof (mVariableDriverVariableList)/sizeof (mVariableDriverVariableList[0]); Index++) {
    if ((CompareGuid (mVariableDriverVariableList[Index].Guid, VendorGuid)) && (StrCmp (mVariableDriverVariableList[Index].Name, VariableName) == 0)) {
      return &mVariableDriverVariableList[Index].VariableProperty;
    }
  }

  return NULL;
}

/**
  Internal SetVariable check.

  @param[in] VariableName       Name of Variable to set.
  @param[in] VendorGuid         Variable vendor GUID.
  @param[in] Attributes         Attribute value of the variable.
  @param[in] DataSize           Size of Data to set.
  @param[in] Data               Data pointer.

  @retval EFI_SUCCESS           The SetVariable check result was success.
  @retval EFI_INVALID_PARAMETER An invalid combination of attribute bits, name, and GUID was supplied,
                                or the DataSize exceeds the minimum or maximum allowed,
                                or the Data value is not following UEFI spec for UEFI defined variables.
  @retval EFI_WRITE_PROTECTED   The variable in question is read-only.
  @retval Others                The return status from check handler.

**/
EFI_STATUS
EFIAPI
InternalVarCheckSetVariableCheck (
  IN CHAR16     *VariableName,
  IN EFI_GUID   *VendorGuid,
  IN UINT32     Attributes,
  IN UINTN      DataSize,
  IN VOID       *Data
  )
{
  EFI_STATUS                    Status;
  UINTN                         Index;
  LIST_ENTRY                    *Link;
  VAR_CHECK_VARIABLE_ENTRY      *Entry;
  CHAR16                        *Name;
  VAR_CHECK_VARIABLE_PROPERTY   *Property;
  INTERNAL_VAR_CHECK_FUNCTION   VarCheckFunction;

  if (!AfterEndOfDxe () &&
     (!IsAuditModeVariable (VariableName, VendorGuid) && !IsDeployedModeVariable (VariableName, VendorGuid))) {
    //
    // Only do check after End Of Dxe.
    //
    return EFI_SUCCESS;
  }
  if (!FeaturePcdGet (PcdH2OCustomizedSecureBootSupported) &&
      (IsAuditModeVariable (VariableName, VendorGuid) || IsDeployedModeVariable (VariableName, VendorGuid))) {
    return EFI_INVALID_PARAMETER;
  }

  Property = NULL;
  VarCheckFunction = NULL;
  for ( Link = GetFirstNode (&mVarCheckVariableList)
      ; !IsNull (&mVarCheckVariableList, Link)
      ; Link = GetNextNode (&mVarCheckVariableList, Link)
      ) {
    Entry = BASE_CR (Link, VAR_CHECK_VARIABLE_ENTRY, Link);
    Name = (CHAR16 *) ((UINTN) Entry + sizeof (*Entry));
    if (CompareGuid (&Entry->Guid, VendorGuid) && (StrCmp (Name, VariableName) == 0)) {
      Property = &Entry->VariableProperty;
      break;
    }
  }

  if (Property == NULL) {
    Property = GetVariableDriverVariableProperty (VariableName, VendorGuid);
  }
  if (Property == NULL) {
    Status = GetUefiDefinedVariableProperty (VariableName, VendorGuid, TRUE, &Property, &VarCheckFunction);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (Property != NULL) {
    if (mEnableLocking && ((Property->Property & VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY) != 0)) {
      return EFI_WRITE_PROTECTED;
    }
    if ((((Attributes & EFI_VARIABLE_APPEND_WRITE) == 0) && (DataSize == 0)) || (Attributes == 0)) {
      //
      // Do not check delete variable.
      //
      return EFI_SUCCESS;
    }
    if (!((((Attributes & EFI_VARIABLE_APPEND_WRITE) == 0) && (DataSize == 0)) || (Attributes == 0))) {
      //
      // Not to delete variable.
      //
      if ((Attributes & (~EFI_VARIABLE_APPEND_WRITE)) != Property->Attributes) {
        return EFI_INVALID_PARAMETER;
      }
      if (DataSize != 0) {
        if ((DataSize < Property->MinSize) || (DataSize > Property->MaxSize)) {
          return EFI_INVALID_PARAMETER;
        }
        if (VarCheckFunction != NULL) {
          Status = VarCheckFunction (
                     Property,
                     DataSize,
                     Data
                     );
          if (EFI_ERROR (Status)) {
            return Status;
          }
        }
      }
    }
  }

  for (Index = 0; Index < mNumberOfHandler; Index ++) {
    Status = mHandlerTable[Index] (
               VariableName,
               VendorGuid,
               Attributes,
               DataSize,
               Data
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  return EFI_SUCCESS;
}

/**
  Reallocates more global memory to store the registered handler list.

  @retval EFI_SUCCESS            Reallocate memory successfully.
  @retval EFI_OUT_OF_RESOURCES   No enough memory to allocate.
**/
RETURN_STATUS
EFIAPI
ReallocateHandlerTable (
  VOID
  )
{
  VAR_CHECK_SET_VARIABLE_CHECK_HANDLER  *HandlerTable;

  //
  // Reallocate memory for check handler table.
  //
  HandlerTable = VariableAllocateZeroBuffer (
                   (mMaxNumberOfHandler + VAR_CHECK_HANDLER_TABLE_SIZE) * sizeof (VAR_CHECK_SET_VARIABLE_CHECK_HANDLER),
                   TRUE
                   );
  if (HandlerTable == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (HandlerTable, mHandlerTable, mMaxNumberOfHandler * sizeof (VAR_CHECK_SET_VARIABLE_CHECK_HANDLER));
  EFI_FREE_POOL (mHandlerTable);

  mHandlerTable = HandlerTable;
  //
  // Increase max handler number.
  //
  mMaxNumberOfHandler = mMaxNumberOfHandler + VAR_CHECK_HANDLER_TABLE_SIZE;
  return EFI_SUCCESS;
}

/**
  Register SetVariable check handler.

  @param[in] Handler            Pointer to check handler.

  @retval EFI_SUCCESS           The SetVariable check handler was registered successfully.
  @retval EFI_INVALID_PARAMETER Handler is NULL.
  @retval EFI_ACCESS_DENIED     EFI_END_OF_DXE_EVENT_GROUP_GUID or EFI_EVENT_GROUP_READY_TO_BOOT has
                                already been signaled.
  @retval EFI_OUT_OF_RESOURCES  There is not enough resource for the SetVariable check handler register request.
  @retval EFI_UNSUPPORTED       This interface is not implemented.
                                For example, it is unsupported in VarCheck protocol if both VarCheck and SmmVarCheck protocols are present.

**/
EFI_STATUS
EFIAPI
VarCheckRegisterSetVariableCheckHandler (
  IN VAR_CHECK_SET_VARIABLE_CHECK_HANDLER   Handler
  )
{
  EFI_STATUS    Status;

  if (Handler == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (AfterEndOfDxe ()) {
    return EFI_ACCESS_DENIED;
  }

  //
  // Check whether the handler list is enough to store new handler.
  //
  if (mNumberOfHandler == mMaxNumberOfHandler) {
    //
    // Allocate more resources for new handler.
    //
    Status = ReallocateHandlerTable();
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Register new handler into the handler list.
  //
  mHandlerTable[mNumberOfHandler] = Handler;
  mNumberOfHandler ++;

  return EFI_SUCCESS;
}

/**
  Variable property get function.

  @param[in] Name   Pointer to the variable name.
  @param[in] Guid   Pointer to the vendor GUID.

  @return Pointer to the property of variable specified by the Name and Guid.

**/
VAR_CHECK_VARIABLE_PROPERTY *
VariablePropertyGetFunction (
  IN CHAR16                 *Name,
  IN EFI_GUID               *Guid,
  IN BOOLEAN                WildcardMatch
  )
{
  LIST_ENTRY                    *Link;
  VAR_CHECK_VARIABLE_ENTRY      *Entry;
  CHAR16                        *VariableName;
  VAR_CHECK_VARIABLE_PROPERTY   *Property;
  LIST_ENTRY                    *VarCheckList;

  if (mSmst != NULL && !AfterEndOfDxe ()) {
    VarCheckList = mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->VarCheckVariableList;
  } else {
    VarCheckList = &mVarCheckVariableList;
  }

  for ( Link = GetFirstNode (VarCheckList)
      ; !IsNull (VarCheckList, Link)
      ; Link = GetNextNode (VarCheckList, Link)
      ) {
    Entry = BASE_CR (Link, VAR_CHECK_VARIABLE_ENTRY, Link);
    VariableName = (CHAR16 *) ((UINTN) Entry + sizeof (*Entry));
    if (CompareGuid (&Entry->Guid, Guid) && (StrCmp (VariableName, Name) == 0)) {
      return &Entry->VariableProperty;
    }
  }

  Property = GetVariableDriverVariableProperty (Name, Guid);
  if (Property == NULL) {
    GetUefiDefinedVariableProperty (Name, Guid, WildcardMatch, &Property, NULL);
  }

  return Property;
}


/**
  Internal function to set variable property.

  @param[in] Name               Pointer to the variable name.
  @param[in] Guid               Pointer to the vendor GUID.
  @param[in] VariableProperty   Pointer to the input variable property.

  @retval EFI_SUCCESS           The property of variable specified by the Name and Guid was set successfully.
  @retval EFI_INVALID_PARAMETER Name, Guid or VariableProperty is NULL, or Name is an empty string,
                                or the fields of VariableProperty are not valid.
  @retval EFI_ACCESS_DENIED     EFI_EVENT_GROUP_EXIT_BOOT_SERVICES has already been signaled.
  @retval EFI_OUT_OF_RESOURCES  There is not enough resource for the variable property set request.
**/
EFI_STATUS
InternalVariablePropertySet (
  IN CHAR16                         *Name,
  IN EFI_GUID                       *Guid,
  IN VAR_CHECK_VARIABLE_PROPERTY    *VariableProperty
  )
{
  EFI_STATUS                    Status;
  VAR_CHECK_VARIABLE_ENTRY      *Entry;
  CHAR16                        *VariableName;
  VAR_CHECK_VARIABLE_PROPERTY   *Property;
  LIST_ENTRY                    *CheckList;

  if (Name == NULL || Name[0] == 0 || Guid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (VariableProperty == NULL || VariableProperty->Revision != VAR_CHECK_VARIABLE_PROPERTY_REVISION) {
    return EFI_INVALID_PARAMETER;
  }

  if (VariableAtRuntime ()) {
    return EFI_ACCESS_DENIED;
  }

  //
  // Get the pointer of property data for set.
  //
  Property = VariablePropertyGetFunction (Name, Guid, FALSE);
  if (Property != NULL) {
    CopyMem (Property, VariableProperty, sizeof (*VariableProperty));
  } else {
    Status = gBS->AllocatePool (
                    EfiRuntimeServicesData,
                    sizeof (*Entry) + StrSize (Name),
                    (VOID **) &Entry
                    );
    if (Status != EFI_SUCCESS) {
      return Status;
    }
    ZeroMem (Entry, sizeof (*Entry) + StrSize (Name));
    VariableName = (CHAR16 *) ((UINTN) Entry + sizeof (*Entry));
    StrnCpy (VariableName, Name, StrLen (Name));
    CopyGuid (&Entry->Guid, Guid);
    CopyMem (&Entry->VariableProperty, VariableProperty, sizeof (*VariableProperty));
    CheckList = mSmst == NULL ? &mVarCheckVariableList : mSmmVariableGlobal->ProtectedModeVariableModuleGlobal->VarCheckVariableList;
    InsertTailList (CheckList, &Entry->Link);
  }

  return EFI_SUCCESS;
}


/**
  Variable property set.

  @param[in] Name               Pointer to the variable name.
  @param[in] Guid               Pointer to the vendor GUID.
  @param[in] VariableProperty   Pointer to the input variable property.

  @retval EFI_SUCCESS           The property of variable specified by the Name and Guid was set successfully.
  @retval EFI_INVALID_PARAMETER Name, Guid or VariableProperty is NULL, or Name is an empty string,
                                or the fields of VariableProperty are not valid.
  @retval EFI_ACCESS_DENIED     EFI_END_OF_DXE_EVENT_GROUP_GUID or EFI_EVENT_GROUP_READY_TO_BOOT has
                                already been signaled.
  @retval EFI_OUT_OF_RESOURCES  There is not enough resource for the variable property set request.
**/
EFI_STATUS
EFIAPI
VarCheckVariablePropertySet (
  IN CHAR16                         *Name,
  IN EFI_GUID                       *Guid,
  IN VAR_CHECK_VARIABLE_PROPERTY    *VariableProperty
  )
{
  EFI_STATUS                    Status;

  if (AfterEndOfDxe ()) {
    return EFI_ACCESS_DENIED;
  }

  AcquireLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
  Status = InternalVariablePropertySet (Name, Guid, VariableProperty);
  ReleaseLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);

  return Status;
}

/**
  Internal variable property get.

  @param[in]  Name              Pointer to the variable name.
  @param[in]  Guid              Pointer to the vendor GUID.
  @param[out] VariableProperty  Pointer to the output variable property.

  @retval EFI_SUCCESS           The property of variable specified by the Name and Guid was got successfully.
  @retval EFI_NOT_FOUND         The property of variable specified by the Name and Guid was not found.

**/
EFI_STATUS
EFIAPI
InternalVarCheckVariablePropertyGet (
  IN  CHAR16                         *Name,
  IN  EFI_GUID                       *Guid,
  OUT VAR_CHECK_VARIABLE_PROPERTY   *VariableProperty
  )
{
  LIST_ENTRY                    *Link;
  VARIABLE_ENTRY                *Entry;
  CHAR16                        *VariableName;
  BOOLEAN                       Found;
  VAR_CHECK_VARIABLE_PROPERTY   *Property;

  Found = FALSE;
  Property = VariablePropertyGetFunction (Name, Guid, TRUE);
  if (Property != NULL) {
    CopyMem (VariableProperty, Property, sizeof (*VariableProperty));
    Found = TRUE;
  }

  for ( Link = GetFirstNode (&mLockedVariableList)
      ; !IsNull (&mLockedVariableList, Link)
      ; Link = GetNextNode (&mLockedVariableList, Link)
      ) {
    Entry = BASE_CR (Link, VARIABLE_ENTRY, Link);
    VariableName = (CHAR16 *) ((UINTN) Entry + sizeof (*Entry));
    if (CompareGuid (&Entry->Guid, Guid) && (StrCmp (VariableName, Name) == 0)) {
      VariableProperty->Property |= VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY;
      if (!Found) {
        VariableProperty->Revision = VAR_CHECK_VARIABLE_PROPERTY_REVISION;
        Found = TRUE;
      }
    }
  }

  return (Found ? EFI_SUCCESS : EFI_NOT_FOUND);
}


/**
  Variable property get.

  @param[in]  Name              Pointer to the variable name.
  @param[in]  Guid              Pointer to the vendor GUID.
  @param[out] VariableProperty  Pointer to the output variable property.

  @retval EFI_SUCCESS           The property of variable specified by the Name and Guid was got successfully.
  @retval EFI_INVALID_PARAMETER Name, Guid or VariableProperty is NULL, or Name is an empty string.
  @retval EFI_NOT_FOUND         The property of variable specified by the Name and Guid was not found.

**/
EFI_STATUS
EFIAPI
VarCheckVariablePropertyGet (
  IN CHAR16                         *Name,
  IN EFI_GUID                       *Guid,
  OUT VAR_CHECK_VARIABLE_PROPERTY   *VariableProperty
  )
{
  EFI_STATUS    Status;

  if (Name == NULL || Name[0] == 0 || Guid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (VariableProperty == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  AcquireLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
  Status = InternalVarCheckVariablePropertyGet (Name, Guid, VariableProperty);
  ReleaseLockOnlyAtBootTime (&mVariableModuleGlobal->VariableBase.VariableServicesLock);
  return Status;
}


/**
  Convert all of pointer to variable check information to virtual address.
**/
VOID
ConvertVariableCheckInfo (
  VOID
  )
{
  UINTN         Index;

  for (Index = 0; Index < mNumberOfHandler; Index++) {
    gRT->ConvertPointer (0x0, (VOID **) &mHandlerTable[Index]);
  }
  gRT->ConvertPointer (0x0, (VOID **) &mHandlerTable);
  ConvertList (&mVarCheckVariableList);
}


/**
  Create variable check list in SMM RAM.

  @param[in] ListHead      Head of linked list from protected mode.

  @retval EFI_SUCCESS           Create variable check list in SMM RAM successfully.
  @retval EFI_USUPPORTED        System isn't in SMM mode.
  @retval EFI_OUT_OF_RESOURCES  There is not enough resource to hold the variable entry.
**/
STATIC
EFI_STATUS
CreateVariableCheckListInSmm (
  LIST_ENTRY         *ListHead
  )
{
  LIST_ENTRY               *Link;
  VAR_CHECK_VARIABLE_ENTRY *Entry;
  UINTN                    EntrySize;
  VAR_CHECK_VARIABLE_ENTRY *WorkingEntry;
  CHAR16                   *Name;

  if (mSmst == NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // in the list of locked variables, convert the name pointers first
  //
  for ( Link = GetFirstNode (ListHead)
      ; !IsNull (ListHead, Link)
      ; Link = GetNextNode (ListHead, Link)
      ) {
    Entry     = BASE_CR (Link, VAR_CHECK_VARIABLE_ENTRY, Link);
    Name      = (CHAR16 *) ((UINTN) Entry + sizeof (*Entry));
    EntrySize = sizeof (VARIABLE_ENTRY) + StrSize (Name);
    WorkingEntry = VariableAllocateZeroBuffer (EntrySize, TRUE);
    ASSERT (WorkingEntry != NULL);
    if (WorkingEntry == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    CopyMem (WorkingEntry, Entry, EntrySize);
    InsertTailList (&mLockedVariableList, &WorkingEntry->Link);
  }

  return EFI_SUCCESS;
}

/**
  This function uses to send SMI to create vriable check list in SMM mode.
**/
VOID
CreateVariableCheckListThroughSmi (
  VOID
  )
{
  UINT32            Signature;
  LIST_ENTRY        *ListHead;

  if (mVariableModuleGlobal->SmmCodeReady && mSmst == NULL) {
    Signature = SMM_VARIABLE_CHECK_SIGNATURE;
    CopyMem (mSmmPhyVarBuf, &Signature, sizeof (Signature));
    ListHead = &mVarCheckVariableList;
    CopyMem (mSmmPhyVarBuf + 1, &ListHead, sizeof (LIST_ENTRY *));
    SmmSecureBootCall ((UINT8 *) mSmmPhyVarBuf, sizeof (SMM_VAR_BUFFER) + sizeof (LIST_ENTRY *), SMM_VARIABLE_CHECK_FUN_NUM, SW_SMI_PORT);
  }
}

/**
  This function uses to create variable check list.

  @return EFI_SUCCESS    Create variable check list in SMM RAM successfully.
  @return Other          Any error occured while creating variable check list in SMM RAM.
**/
EFI_STATUS
SmmCreateVariableCheckList (
  VOID
  )
{
  UINT32         BufferSize;
  UINT8          *VariableBuffer;
  UINT32         Signature;
  LIST_ENTRY     *ListHead;
  STATIC BOOLEAN Initialized = FALSE;

  if (Initialized) {
    return EFI_ALREADY_STARTED;
  }
  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RBX ,&BufferSize);
  VariableBuffer = NULL;
  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RSI ,(UINT32 *) &VariableBuffer);
  Signature = SMM_VARIABLE_CHECK_SIGNATURE;

  if (BufferOverlapSmram (VariableBuffer, BufferSize) || BufferSize != sizeof (SMM_VAR_BUFFER) + sizeof (LIST_ENTRY *) ||
      CompareMem (VariableBuffer, &Signature, sizeof (UINT32))!= 0) {
    return EFI_UNSUPPORTED;
  }
  Initialized = TRUE;
  ListHead = *((LIST_ENTRY **) (VariableBuffer + sizeof (SMM_VAR_BUFFER)));
  return CreateVariableCheckListInSmm (ListHead);
}

/**
  Internal function to updated UEFI defined global variable property in portected mode memory.

  @param[in] VariableName        A Null-terminated string that is the name of the vendor!|s variable.
  @param[in] Property            Input variable property.

  @return EFI_SUCCESS            Update variable check property of UEFI defined global variable successfully.
  @return EFI_INVALID_PARAMETER  Input variable check propery is unsupported.
  @return EFI_NOT_FOUND          Cannot find specific UEFI defined global variable in variable check list.
**/
STATIC
EFI_STATUS
UpdateProtectedModeUefiVariableProperty (
  IN CONST CHAR16      *VariableName,
  IN CONST UINT8       Property
  )
{
  UINTN                        Index;
  UINTN                        Entries;
  UEFI_DEFINED_VARIABLE_ENTRY  *GlobalVariableList;

  if (Property != 0 && Property != VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY) {
    return EFI_INVALID_PARAMETER;
  }

  Entries = sizeof (mGlobalVariableList) / sizeof (mGlobalVariableList[0]);
  GlobalVariableList = mGlobalVariableList;
  for (Index = 0; Index < Entries; Index++) {
    if (StrCmp (GlobalVariableList[Index].Name, VariableName) == 0) {
      if (mSmst != NULL) {
        //
        // We need take care the case accessing normal memory at runtime and the below code
        // compare variable name with the variable list in SMM RAM and then update the property to
        // the normal memory through physical address in mVariableModuleGlobal->GlobalVariableList.
        // Note: If protected mode runs in 64 bit mode, the UINTN in SMM mode is also considered 8 bytes.
        // We needn't do extra conversion for UINTN.
        //
        GlobalVariableList = mVariableModuleGlobal->GlobalVariableList;
      }
      GlobalVariableList[Index].VariableProperty.Property = Property;
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}

/**
  This fucnitons uses to invoke SMM mode update variable property.

  After invoking this function. the return status is put in Status member in SMM_VAR_BUFFER.

  @retval EFI_SUCCESS     Update variable property in SMM mode successful.
**/
EFI_STATUS
SmmUpdateVariablePropertySmi (
  VOID
  )
{
  UINT32             BufferSize;
  SMM_VAR_BUFFER    *SmmVarBuffer;
  CHAR16            *VariableName;
  UINT8             *VariableBuffer;
  UINTN             Entries;
  UINTN             Index;


  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RBX ,&BufferSize);
  SmmVarBuffer = NULL;
  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RSI ,(UINT32 *) &SmmVarBuffer);

  if (BufferOverlapSmram (SmmVarBuffer, BufferSize) ||
      BufferSize > sizeof (SMM_VAR_BUFFER) + MAX_VARIABLE_NAME_SIZE + MAX_VARIABLE_SIZE ||
      BufferSize != sizeof (SMM_VAR_BUFFER) + SmmVarBuffer->VariableNameSize +  SmmVarBuffer->DataSize ||
      SmmVarBuffer->DataSize != sizeof (UINT8) || SmmVarBuffer->Signature != UPDATE_VARIABLE_PROPERTY_SIGNATURE) {
    return EFI_UNSUPPORTED;
  }

  VariableName = (CHAR16 *) (SmmVarBuffer + 1);
  VariableBuffer = ((UINT8 *) (SmmVarBuffer + 1)) + SmmVarBuffer->VariableNameSize;

  Entries = sizeof (mGlobalVariableList) / sizeof (mGlobalVariableList[0]);
  for (Index = 0; Index < Entries; Index++) {
    if (StrCmp (mGlobalVariableList[Index].Name, VariableName) == 0) {
      mGlobalVariableList[Index].VariableProperty.Property = *VariableBuffer;
      SmmVarBuffer->Status = EFI_SUCCESS;
      return EFI_SUCCESS;
    }
  }
  SmmVarBuffer->Status = EFI_NOT_FOUND;
  return EFI_SUCCESS;
}

/**
  Internal function to updated UEFI defined global variable property in SMM memory.

  @param[in] VariableName        A Null-terminated string that is the name of the vendor!|s variable.
  @param[in] Property            Input variable property.

  @return EFI_SUCCESS            Update variable check property of UEFI defined global variable successfully.
  @return EFI_INVALID_PARAMETER  Input variable check propery is unsupported.
  @return EFI_NOT_FOUND          Cannot find specific UEFI defined global variable in variable check list.
**/
STATIC
EFI_STATUS
UpdateSmmModeUefiVariableProperty (
  IN CONST CHAR16      *VariableName,
  IN CONST UINT8       Property
  )
{
  UINTN                        Index;
  UINTN                        Entries;
  UINT8                       *WorkingBuf;
  UINTN                       SmmBufSize;

  if (Property != 0 && Property != VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY) {
    return EFI_INVALID_PARAMETER;
  }

  if (mSmst != NULL) {
    Entries = sizeof (mGlobalVariableList) / sizeof (mGlobalVariableList[0]);
    for (Index = 0; Index < Entries; Index++) {
      if (StrCmp (mGlobalVariableList[Index].Name, VariableName) == 0) {
        mGlobalVariableList[Index].VariableProperty.Property = Property;
        return EFI_SUCCESS;
      }
    }
    return EFI_NOT_FOUND;
  }
  if (!mVariableModuleGlobal->SmmCodeReady) {
    return EFI_SUCCESS;
  }


  SmmBufSize = sizeof (SMM_VAR_BUFFER) + StrSize (VariableName) + sizeof (Property);
  mSmmVarBuf->Signature = UPDATE_VARIABLE_PROPERTY_SIGNATURE;
  mSmmVarBuf->Attributes = 0;
  mSmmVarBuf->VariableNameSize = StrSize (VariableName);
  mSmmVarBuf->DataSize = sizeof (Property);

  WorkingBuf = (UINT8 *) (mSmmVarBuf + 1);
  StrCpy ((CHAR16 *) WorkingBuf, VariableName);
  WorkingBuf += mSmmVarBuf->VariableNameSize;
  CopyMem (WorkingBuf, &Property, sizeof (Property));
  mSmmVarBuf->Status = EFI_UNSUPPORTED;
  SmmSecureBootCall ((UINT8 *) mSmmPhyVarBuf, SmmBufSize, UPDATE_VARIABLE_PROPERTY_FUN_NUM, SW_SMI_PORT);
  return mSmmVarBuf->Status;
}

/**
  Internal function to update UEFI defined global variable property

  @param[in] VariableName        A Null-terminated string that is the name of the vendor!|s variable.
  @param[in] Property            Input variable property.

  @return EFI_SUCCESS            Update variable check property of UEFI defined global variable successfully.
  @return EFI_INVALID_PARAMETER  Input variable check propery is unsupported.
  @return EFI_NOT_FOUND          Cannot find specific UEFI defined global variable in variable check list.
**/
STATIC
EFI_STATUS
UpdateUefiVariableProperty (
  IN CONST CHAR16      *VariableName,
  IN CONST UINT8       Property
  )
{
  EFI_STATUS         Status;

  if (VariableAtRuntime ()) {
    return EFI_INVALID_PARAMETER;
  }
  Status = UpdateProtectedModeUefiVariableProperty (VariableName, Property);
  if (Status != EFI_SUCCESS) {
    return Status;
  }
  return UpdateSmmModeUefiVariableProperty (VariableName, Property);
}

/**
  Update the variable check property of AuditMode variable.

  @param[in] Property            Input variable property.

  @return EFI_SUCCESS            Update variable check property of AuditMode variable successfully.
  @return EFI_INVALID_PARAMETER  Input variable check propery is unsupported.
  @return EFI_NOT_FOUND          Cannot find AuditMode variable in variable check list.
**/
EFI_STATUS
UpdateAuditModeProperty (
  IN CONST UINT8      Property
  )
{
  return UpdateUefiVariableProperty (EFI_AUDIT_MODE_VARIABLE_NAME, Property);
}

/**
  Update the variable check property of DeployedMode variable.

  @param[in] Property            Input variable property.

  @return EFI_SUCCESS            Update variable check property of DeployedMode variable successfully.
  @return EFI_INVALID_PARAMETER  Input variable check propery is unsupported.
  @return EFI_NOT_FOUND          Cannot find DeployedMode variable in variable check list.
**/
EFI_STATUS
UpdateDeployedModeProperty (
  IN CONST UINT8      Property
  )
{
  return UpdateUefiVariableProperty (EFI_DEPLOYED_MODE_VARIABLE_NAME, Property);
}

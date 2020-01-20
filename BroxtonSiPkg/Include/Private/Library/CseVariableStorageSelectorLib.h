/** @file
  CSE Variable Storage Selector Library

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
#ifndef _CSE_VARIABLE_STORAGE_SELECTOR_LIB_H_
#define _CSE_VARIABLE_STORAGE_SELECTOR_LIB_H_

#include <Uefi.h>

#include <Library/CseVariableStorageLib.h>
#include <Protocol/Heci.h>

#define CSE_PRIMARY_NVM_FILE_STORE_OFFSET   0

//
// Variable store options provided by CSE (platform specific)
//
typedef enum {
  VariableStoreTypeHostMemoryBuffer,     ///< A variable store that resides in host memory
  VariableStoreTypeNv,                   ///< General NVM provided by CSE
  VariableStoreTypeMax                   ///< End of CSE variable store types
} CSE_VARIABLE_STORE_TYPE;

/**
  Returns the CSE NVM file used for the requested variable.

  @param[in] VariableName         Name of the variable.
  @param[in] VendorGuid           Guid of the variable.
  @param[in] CseVariableFileInfo  An array of pointers to CSE
                                  variable file information.

  @return  The type of CSE NVM file used for this variable.
**/
CSE_VARIABLE_FILE_TYPE
EFIAPI
GetCseVariableStoreFileType (
  IN CONST CHAR16                 *VariableName,
  IN CONST EFI_GUID               *VendorGuid,
  IN CONST CSE_VARIABLE_FILE_INFO **CseVariableFileInfo
  );

/**
  Returns the base offset for variable services in the CSE NVM file
  specified for this file type.

  @param[in]  CseVariableFileType  The type of the CSE NVM file.

  @param[out] CseFileOffset        The offset to base a variable store
                                   in the CSE file for this variable.

  @return     EFI_SUCCESS          Offset successfully returned for the
                                   variable.
  @return     Others               An error occurred.
**/
EFI_STATUS
EFIAPI
GetCseVariableStoreFileOffset (
  IN  CSE_VARIABLE_FILE_TYPE  CseVariableFileType,
  OUT UINT32                  *CseFileOffset
  );

/**
  Returns the HECI protocol used to access the CSE NVM file based on the current point in the boot.

  @param    HeciProtocol                A pointer to the HECI2 protocol if it is active.
                                        If HECI1 is active, the value is NULL.

  @return   CSE_VARIABLE_HECI_PROTOCOL  The HECI protocol to currently use.
**/
CSE_VARIABLE_HECI_PROTOCOL
EFIAPI
GetCseHeciProtocol (
  OUT      EFI_HECI_PROTOCOL  **Heci2Protocol
  );

/**
  Returns the HECI protocol used to access the CSE NVM file based on a given varaible.

  @param[in] VariableName         Name of the variable.
  @param[in] VendorGuid           Guid of the variable.

  @return   CSE_VARIABLE_HECI_PROTOCOL  The HECI protocol to currently use.
**/
CSE_VARIABLE_HECI_PROTOCOL
EFIAPI
GetCseVariableHeciProtocol (
  IN CONST CHAR16             *VariableName,
  IN CONST EFI_GUID           *VendorGuid
  );

#endif

/** @file
  Defines common functions used in CseVariableStorage.

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

#ifndef _CSE_VARIABLE_STORAGE_H_
#define _CSE_VARIABLE_STORAGE_H_

#include "CseVariableStorageSmmRuntimeDxe.h"

#include <Library/BaseLib.h>
#include <Library/HobLib.h>
#include <Library/VariableNvmStorageLib.h>
//[-start-160812-IB07220128-add]//
#include <Library/UefiBootServicesTableLib.h>
//[-end-160812-IB07220128-add]//
//[-start-161021-IB07400801-add]//
#ifdef EFI_DEBUG
#ifndef EFI_DEBUG_CSE_VARIABLE_MESSAGE
#undef DEBUG
#define DEBUG(Expression)
#endif
#endif
//[-end-161021-IB07400801-add]//
//[-start-160812-IB07220128-add]//
typedef struct {
  VARIABLE_STORAGE_GET_ID                                 GetId;
  VARIABLE_STORAGE_GET_VARIABLE                           GetVariable;
  VARIABLE_STORAGE_GET_AUTHENTICATED_VARIABLE             GetAuthenticatedVariable;
  VARIABLE_STORAGE_GET_NEXT_VARIABLE_NAME                 GetNextVariableName;
  VARIABLE_STORAGE_GET_STORAGE_USAGE                      GetStorageUsage;
  VARIABLE_STORAGE_GET_AUTHENTICATED_SUPPORT              GetAuthenticatedSupport;
  VARIABLE_STORAGE_SET_VARIABLE                           SetVariable;
  VARIABLE_STORAGE_WRITE_SERVICE_IS_READY                 WriteServiceIsReady;
  VARIABLE_STORAGE_REGISTER_WRITE_SERVICE_READY_CALLBACK  RegisterWriteServiceReadyCallback;
  VARIABLE_STORAGE_GARBAGE_COLLECT                        GarbageCollect;
  CSE_VARIABLE_FILE_INFO                                  **CseVariableFileInfoPtr;
//[-start-160816-IB07220129-add]//
  MBP_SMM_TRUSTED_KEY                                     *SmmTrustedKey;
//[-end-160816-IB07220129-add]//
} VARIABLE_STORAGE_PROTOCOL_EX;

extern EFI_GUID                      gEfiBootMediaHobGuid;
//[-end-160812-IB07220128-add]//
//[-start-160816-IB07220129-add]//
extern MBP_SMM_TRUSTED_KEY           *mSmmTrustedKey;
//[-end-160816-IB07220129-add]//
extern CSE_VARIABLE_FILE_INFO        *mCseVariableFileInfo[];
//[-start-160812-IB07220128-modify]//
extern VARIABLE_STORAGE_PROTOCOL_EX   mCseVariableStorageProtocol;
//[-end-160812-IB07220128-modify]//

#pragma pack(push, 1)

//
// Maintains common variable information
//
typedef struct {
  CHAR16      *VariableName;
  EFI_GUID    *VendorGuid;
  VOID        *Data;
  UINTN       DataSize;
  UINT32      Attributes;
} CSE_VARIABLE_INFORMATION_TRACK;

//
// Maintains authenticated variable information
//
typedef struct {
  UINT64      MonotonicCount;  ///< Associated monotonic count value to protect against replay attack
  UINT32      PubKeyIndex;     ///< Index of associated public key in database
  EFI_TIME    *TimeStamp;      ///< Associated TimeStamp value to protect against replay attack
} CSE_VARIABLE_AUTHENTICATED_FIELD_TRACK;

#pragma pack(pop)

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
  );

/**
  Sets the CSE NVM variable with the supplied data.

  These arguments are similar to those in the EFI Variable services.

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
  );

#endif

/** @file
  PEI/DXE/SMM Shared Code for the CSE Variable Storage Selector Library

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

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CseVariableStorageLib.h>
#include <Library/DebugLib.h>
//[-start-161003-IB07220139-add]//
#include <KernelSetupConfig.h>
//[-end-161003-IB07220139-add]//
//[-start-161227-IB07250260-add]//
#include <Guid/DebugMask.h>
//[-end-161227-IB07250260-add]//
//[-start-161021-IB07400801-add]//
#ifdef EFI_DEBUG
#ifndef EFI_DEBUG_CSE_VARIABLE_MESSAGE
#undef DEBUG
#define DEBUG(Expression)
#endif
#endif
//[-end-161021-IB07400801-add]//

typedef struct {
  CONST CHAR16       *VariableName;
  EFI_GUID           *VendorGuid;
} VARIABLE_NAME_TABLE_ENTRY;

//[-start-160818-IB03090432-add]//
extern EFI_GUID gEfiMemoryConfigVariableGuid;
extern EFI_GUID gEfiMemoryTypeInformationGuid;
//[-end-160818-IB03090432-add]//

//[-start-160818-IB03090432-modify]//
//[-start-161227-IB07250260-modify]//
VARIABLE_NAME_TABLE_ENTRY mPreMemoryFileStoredVariables[] = {
  { L"Setup", &gSystemConfigurationGuid },
  { L"MemoryConfig", &gEfiMemoryConfigVariableGuid },
  { L"MemoryBootData", &gEfiMemoryConfigVariableGuid },
  { L"MemoryTypeInformation", &gEfiMemoryTypeInformationGuid },
  { L"PerfDataMemAddr", &gEfiGenericVariableGuid }
};
//[-end-161227-IB07250260-modify]//
//[-end-160818-IB03090432-modify]//

BOOLEAN
EFIAPI
IsPreMemoryVariable (
  IN  CONST  CHAR16       *VariableName,
  IN  CONST  EFI_GUID     *VendorGuid
  )
{
  UINTN   Index;

  for (Index = 0;
       Index < (sizeof (mPreMemoryFileStoredVariables) / sizeof (mPreMemoryFileStoredVariables[0]));
       Index++) {
    if (CompareGuid (VendorGuid, mPreMemoryFileStoredVariables[Index].VendorGuid) &&
        (StrCmp (VariableName, mPreMemoryFileStoredVariables[Index].VariableName) == 0)) {
      return TRUE;
    }
  }
  return FALSE;
}

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
  )
{
  CSE_VARIABLE_FILE_TYPE Type;

  if (VariableName[0] == 0) {
    //
    // Return the first available CSE file store
    //
    DEBUG ((EFI_D_INFO, "CseVariableStorageSelectorLib - Variable name is NULL\n"));
    for (Type = (CSE_VARIABLE_FILE_TYPE) 0; Type < CseVariableFileTypeMax; Type++) {
      if (CseVariableFileInfo[Type]->FileEnabled) {
        return Type;
      }
    }
    //
    // There should always be at least one CSE file store enabled
    //
//[-start-161020-IB07400800-remove]//
//    ASSERT (FALSE);
//[-end-161020-IB07400800-remove]//
    return (CSE_VARIABLE_FILE_TYPE) 0;
  } else if (IsPreMemoryVariable (VariableName, VendorGuid)) {
    //
    // The variable is preferred to be stored in the MRC training data file.
    //
    DEBUG ((DEBUG_INFO, "CseVariableStorageSelectorLib - PreMemory Variable - %s\n", VariableName));
    return (CseVariableFileInfo[CseVariableFileTypePreMemoryFile]->FileEnabled ?
                                                                   CseVariableFileTypePreMemoryFile :
                                                                   CseVariableFileTypePrimaryIndexFile
                                                                   );
  } else {
    //
    // All other variables are stored in individual CSE files
    //
    DEBUG ((EFI_D_INFO, "CseVariableStorageSelectorLib - Variable is not Setup\n"));
    return CseVariableFileTypePrimaryIndexFile;
  }
}

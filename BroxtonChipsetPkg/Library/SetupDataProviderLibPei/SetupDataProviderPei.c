/** @file
  This module provides default Setup variable data if Setupvariable is not found
  and also provides a Setup variable cache mechanism in PEI phase

;******************************************************************************
;* Copyright (c) 2013 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <PiPei.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/KernelConfigLib.h>
#include <Library/SetupUtilityLib.h>
#include <Library/PeiOemSvcKernelLib.h>
#include <Library/BaseSetupDefaultLib.h>


#include <Ppi/ReadOnlyVariable2.h>
#include <Guid/PeiDefaultSetupProviderHob.h>
#include <Guid/SetupDefaultHob.h>
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
#include <Ppi/EmuPei.h>
#endif
//[-end-161123-IB07250310-modify]//
#include <FastRecoveryData.h>
#include "SetupDataProvider.h"
#include <Library/SeamlessRecoveryLib.h>

//[-start-151123-IB11270134-remove]//
// #ifndef __GNUC__
// #pragma pack(push, 16)
// IFR_SETUP_DEFAULT IfrSetupDefault = {
//                     {'$', 'I', 'F', 'R', 'D', 'F', 'T'},
//                     0x02,
//                     0x05,
//                     { (UINT64) (UINTN) AdvanceVfrSystemConfigDefault0000,
//                       (UINT64) (UINTN) PowerVfrSystemConfigDefault0000,
//                       (UINT64) (UINTN) MainVfrSystemConfigDefault0000,
//                       (UINT64) (UINTN) SecurityVfrSystemConfigDefault0000,
//                       (UINT64) (UINTN) BootVfrSystemConfigDefault0000,
//                       (UINT64) (UINTN) 0
//                     }
// };
// #pragma pack(pop)
// #endif
//[-end-151123-IB11270134-remove]//

CHAR16 mSetupVariableName[] = SETUP_VARIABLE_NAME;
CHAR16 mSetupOrgVariableName[] = L"SetupOrg";
BOOLEAN mVariablePpiInstalledInMemory = FALSE;
BOOLEAN mVariablePpiIsHooked = FALSE;

EFI_PEI_READ_ONLY_VARIABLE2_PPI mSetupDataProviderVariablePpi = {
  NewPeiGetVariable,
  NewPeiNextVariableName
};

EFI_PEI_PPI_DESCRIPTOR     mSetupDataProviderVariablePpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiReadOnlyVariable2PpiGuid,
  &mSetupDataProviderVariablePpi
};

EFI_STATUS
ReadOnlyVariable2HookCallback (
  IN CONST EFI_PEI_SERVICES           **PeiServices
//  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
//  IN VOID                       *Ppi
  );

//EFI_PEI_NOTIFY_DESCRIPTOR      mVariableNotifyPpiList[] = {
//  {
//    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
//    &gEfiPeiReadOnlyVariable2PpiGuid,
//    ReadOnlyVariable2HookCallback
//  }
//};

EFI_STATUS
FindGuidHob(
  IN  EFI_GUID                         *Guid,
  OUT EFI_PEI_HOB_POINTERS             *GuidHob
  ) 
{
  GuidHob->Raw = GetHobList ();
  GuidHob->Raw = GetNextGuidHob (Guid, GuidHob->Raw);

  if (GuidHob->Raw == NULL) {
    return EFI_NOT_FOUND;
  }


  return EFI_SUCCESS;
}

/**
  Provide the get next variable functionality of the variable services.

  @param  This              A pointer to this instance of the EFI_PEI_READ_ONLY_VARIABLE2_PPI.

  @param  VariableNameSize  On entry, points to the size of the buffer pointed to by VariableName.
  @param  VariableName      On entry, a pointer to a null-terminated string that is the variable's name.
                            On return, points to the next variable's null-terminated name string.

  @param  VariableGuid      On entry, a pointer to an EFI_GUID that is the variable's GUID.
                            On return, a pointer to the next variable's GUID.

  @retval EFI_SUCCESS       The interface could be successfully installed
  @retval EFI_NOT_FOUND     The variable could not be discovered

**/
EFI_STATUS
EFIAPI
NewPeiNextVariableName (
  IN CONST  EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
  IN OUT UINTN                              *VariableNameSize,
  IN OUT CHAR16                             *VariableName,
  IN OUT EFI_GUID                           *VendorGuid
  )
{
  EFI_STATUS                                Status;
  EFI_PEI_HOB_POINTERS                      GuidHob;
  SETUP_DATA_PROVIDER_CONTEXT               *SetupDataProviderContext;

  FindGuidHob (&gPeiDefaultSetupProviderGuid, &GuidHob);
  if (GuidHob.Raw == NULL) {
    return EFI_NOT_FOUND;
  }

  if (FeaturePcdGet(PcdUseEmuVariable)){
    Status = EFI_NOT_FOUND;
  } else {
    SetupDataProviderContext = (SETUP_DATA_PROVIDER_CONTEXT *)GET_GUID_HOB_DATA (GuidHob.Guid);
    Status = SetupDataProviderContext->VariablePpi->NextVariableName (
                                                      This,
                                                      VariableNameSize,
                                                      VariableName,
                                                      VendorGuid
                                                      );
  }
  
  
  return Status;
}

/**
  Provide the revised read variable functionality of the variable services.

  @param  This                  A pointer to this instance of the EFI_PEI_READ_ONLY_VARIABLE2_PPI.
  @param  VariableName          A pointer to a null-terminated string that is the variable's name.
  @param  VariableGuid          A pointer to an EFI_GUID that is the variable's GUID. The combination of
                                VariableGuid and VariableName must be unique.
  @param  Attributes            If non-NULL, on return, points to the variable's attributes.
  @param  DataSize              On entry, points to the size in bytes of the Data buffer.
                                On return, points to the size of the data returned in Data.
  @param  Data                  Points to the buffer which will hold the returned variable value.

  @retval EFI_SUCCESS           The interface could be successfully installed
  @retval EFI_NOT_FOUND         The variable could not be discovered
  @retval EFI_BUFFER_TOO_SMALL  The caller buffer is not large enough

**/
EFI_STATUS
EFIAPI
NewPeiGetVariable (
  IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI  *This,
  IN CONST CHAR16                           *VariableName,
  IN CONST EFI_GUID                         *VendorGuid,
  OUT UINT32                                *Attributes OPTIONAL,
  IN OUT UINTN                              *DataSize,
  OUT VOID                                  *Data
  )
{
  EFI_PEI_HOB_POINTERS                      GuidHob;
  SETUP_DATA_PROVIDER_CONTEXT               *SetupDataProviderContext;
  UINTN                                     SetupVariableSize;
  VOID                                      *SetupData;
  EFI_STATUS                                Status;

  if (VariableName == NULL || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FindGuidHob (&gPeiDefaultSetupProviderGuid, &GuidHob);
  if (GuidHob.Raw == NULL) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  SetupDataProviderContext = (SETUP_DATA_PROVIDER_CONTEXT *)GET_GUID_HOB_DATA (GuidHob.Guid);
  if (SetupDataProviderContext == NULL) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  FindGuidHob (&gSetupDefaultHobGuid, &GuidHob);  
  if (GuidHob.Raw == NULL) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  SetupData = GET_GUID_HOB_DATA (GuidHob.Guid);
  if (SetupData == NULL) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  SetupVariableSize = SetupDataProviderContext->SetupDataSize;

//[-start-160308-IB10860193-modify]//
  if(!PcdGetBool(PcdNvStorageHaveVariable)) {
      
    if (CompareGuid ((EFI_GUID *)VendorGuid, &gSystemConfigurationGuid) &&
      (StrCmp (VariableName, mSetupVariableName) == 0)) {
 
      if (*DataSize < SetupVariableSize) {
        *DataSize = SetupVariableSize;
//        return EFI_BUFFER_TOO_SMALL;
      } else if (Data == NULL) {
        return EFI_INVALID_PARAMETER;
      }

//      if (Attributes != NULL) {
//        *Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
//      }
//      CopyMem (Data, SetupData, SetupVariableSize);
//      return EFI_SUCCESS;
    }
  }
//[-end-160308-IB10860193-modify]//

  if (FeaturePcdGet(PcdUseEmuVariable)) {
    Status = EFI_NOT_FOUND;
  } else {
    if (CompareGuid ((EFI_GUID *)VendorGuid, &gSystemConfigurationGuid) &&
        (StrCmp (VariableName, mSetupOrgVariableName) == 0)) {
      return SetupDataProviderContext->VariablePpi->GetVariable (
                                                      This,
                                                      mSetupVariableName,
                                                      VendorGuid,
                                                      Attributes,
                                                      DataSize,
                                                      Data
                                                      );

    }
    Status = SetupDataProviderContext->VariablePpi->GetVariable (
                                                    This,
                                                    VariableName,
                                                    VendorGuid,
                                                    Attributes,
                                                    DataSize,
                                                    Data
                                                    );
//[-start-160905-IB07220136-add]//
    if (EFI_ERROR (Status) && CompareGuid ((EFI_GUID *)VendorGuid, &gSystemConfigurationGuid) &&
      (StrCmp (VariableName, mSetupVariableName) == 0)) {
      
      if (*DataSize < SetupVariableSize) {
        *DataSize = SetupVariableSize;
        return EFI_BUFFER_TOO_SMALL;
      } else if (Data == NULL) {
        return EFI_INVALID_PARAMETER;
      }
  
      if (Attributes != NULL) {
        *Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
      }
  
      CopyMem (Data, SetupData, SetupVariableSize);
      return EFI_SUCCESS;
    }
//[-end-160905-IB07220136-add]//
  }
  return Status;
  
}

/**
  Setup setup default data and hook variable PPI in PEI phase.

  @param FileHandle             Handle of the file being invoked.
  @param PeiServices            Describes the list of possible PEI Services.

 @retval EFI_STATUS

**/
EFI_STATUS
ReadOnlyVariable2HookCallback (
  IN CONST EFI_PEI_SERVICES           **PeiServices
//  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
//  IN VOID                       *Ppi
  )
{
  EFI_STATUS                                Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI           *VariablePpi;
  EFI_PEI_PPI_DESCRIPTOR                    *VariablePpiDescriptor;
  SETUP_DATA_PROVIDER_CONTEXT               *SetupDataProviderContext;
  UINTN                                     VariableSize;
  BOOLEAN                                   IsRecovery;
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
  UINT8                                     *EmuPeiPpi;
#endif
//[-end-161123-IB07250310-modify]//

  EFI_PEI_HOB_POINTERS                      GuidHob;
  VOID                                      *SetupData;

//  if (mVariablePpiIsHooked) {
//    return EFI_SUCCESS;
//  }

  IsRecovery = FALSE;

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             &VariablePpiDescriptor,
             (VOID **)&VariablePpi
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Detect if the system is in recovery mode
  // 
  Status = OemSvcDetectRecoveryRequest (&IsRecovery);

//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
  if (FeaturePcdGet(PcdUseFastCrisisRecovery)) {
    Status = PeiServicesLocatePpi (
               &gEmuPeiPpiGuid,
               0,
               NULL,
               (VOID **)&EmuPeiPpi
               );

    if (!EFI_ERROR (Status)) {
      IsRecovery = TRUE;
    }
  }
  if ((!IsRecovery) && IsFirmwareFailureRecovery ()) {
    IsRecovery = TRUE;
  }
#endif
//[-end-161123-IB07250310-modify]//

  if (IsRecovery) {
//[-start-151216-IB07220025-remove]//
//     Status = PeiServicesSetBootMode (BOOT_IN_RECOVERY_MODE);
//[-end-151216-IB07220025-remove]//
  }
  
  if (FeaturePcdGet(PcdUseEmuVariable)) {
    Status = EFI_NOT_FOUND;
  } else {
    VariableSize = 0;
    Status = VariablePpi->GetVariable (
                          VariablePpi,
                          mSetupVariableName,
                          &gSystemConfigurationGuid,
                          NULL,
                          &VariableSize,
                          NULL
                          );
  }
  if (Status == EFI_BUFFER_TOO_SMALL && !IsRecovery ) {
      return EFI_SUCCESS;
  } else {
    VariableSize = (UINTN)PcdGet32 (PcdSetupConfigSize);
  }

  FindGuidHob (&gPeiDefaultSetupProviderGuid, &GuidHob);
  
  if (GuidHob.Raw != NULL) {
    if (mVariablePpiInstalledInMemory) {
      SetupDataProviderContext = (SETUP_DATA_PROVIDER_CONTEXT *)GET_GUID_HOB_DATA (GuidHob.Guid);
      if (SetupDataProviderContext == NULL) {
        ASSERT (FALSE);
        return EFI_NOT_FOUND;
      }
      SetupDataProviderContext->VariablePpi = VariablePpi;
      Status = (*PeiServices)->ReInstallPpi (
                             PeiServices, 
                             VariablePpiDescriptor, 
                             &mSetupDataProviderVariablePpiList
                             );
      if (!EFI_ERROR(Status)) {
        mVariablePpiIsHooked = TRUE;
      }
    }
    return EFI_SUCCESS;
  }

  SetupDataProviderContext = (SETUP_DATA_PROVIDER_CONTEXT *) BuildGuidHob (
                               &gPeiDefaultSetupProviderGuid,
                               sizeof (SETUP_DATA_PROVIDER_CONTEXT)
                               );
  if (SetupDataProviderContext == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  
  SetupDataProviderContext->VariablePpi = VariablePpi;
  SetupDataProviderContext->SetupDataSize = VariableSize;

  SetupData = BuildGuidHob (
                &gSetupDefaultHobGuid,
                VariableSize
                );

  if (Status == EFI_BUFFER_TOO_SMALL && !IsRecovery) {
    Status = VariablePpi->GetVariable (
                            VariablePpi,
                            mSetupVariableName,
                            &gSystemConfigurationGuid,
                            NULL,
                            &VariableSize,
                            SetupData
                            );
    
  } else {
    ZeroMem (SetupData, VariableSize);
    ExtractSetupDefault (SetupData);
    
  } 
  
  mVariablePpiIsHooked = TRUE;
  Status = (*PeiServices)->ReInstallPpi (
                             PeiServices, 
                             VariablePpiDescriptor, 
                             &mSetupDataProviderVariablePpiList
                             );
  
  return Status;
}


/**
  The constructor register a ReadOnlyVariable2 PPI callback to hook
	GetVariable().
  

  @param  FileHandle   The handle of FFS header the loaded driver.
  @param  PeiServices  The pointer to the PEI services.

  @retval EFI_SUCCESS  The constructor always returns EFI_SUCCESS.

**/
//EFI_STATUS
//EFIAPI
//InitSetupDataProvider (
//  IN EFI_PEI_FILE_HANDLE        FileHandle,
//  IN CONST EFI_PEI_SERVICES     **PeiServices
//  )
//{
//#ifndef __GNUC__
//    UINT8 EZH2OVersion;
//  
//    //
//    // Dummy code to avoid IfrSetupDefault data being removed due to linker optimization
//    // Do not remove this line
//    //
//    EZH2OVersion = IfrSetupDefault.EZH2OVersion;
//#endif
////  PeiServicesNotifyPpi(mVariableNotifyPpiList);
//  return (*PeiServices)->NotifyPpi (PeiServices, mVariableNotifyPpiList);
//
////  return EFI_SUCCESS;
//}


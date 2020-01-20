/** @file
  Implementation of Fsp PCH Policy Initialization.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

#include <PeiFspPolicyInitLib.h>
#include <Ppi/CpuPolicy.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Library/CpuPolicyLib.h>

/**
  Performs FSP CPU PEI Policy initialization.

  @param[in][out]  FspmUpd             Pointer to FSP UPD Data.

  @retval          EFI_SUCCESS         FSP UPD Data is updated.
  @retval          EFI_NOT_FOUND       Fail to locate required PPI.
  @retval          Other               FSP UPD Data update process fail.
**/
EFI_STATUS
EFIAPI
PeiFspCpuPolicyInitPreMem (
  IN OUT FSPM_UPD    *FspmUpd
  )
{
  EFI_STATUS                Status;
  SI_CPU_POLICY_PPI         *SiCpuPolicyPpi;

  //
  // Locate SiCpuPolicyPpi
  //
  SiCpuPolicyPpi = NULL;
  Status = PeiServicesLocatePpi (
             &gSiCpuPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiCpuPolicyPpi
             );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

/**
  Performs FSP CPU PEI Policy post memory initialization.

  @param[in][out]  FspsUpd             Pointer to FSP UPD Data.

  @retval          EFI_SUCCESS         FSP UPD Data is updated.
  @retval          EFI_NOT_FOUND       Fail to locate required PPI.
  @retval          Other               FSP UPD Data update process fail.
**/
EFI_STATUS
EFIAPI
PeiFspCpuPolicyInit (
  IN OUT FSPS_UPD    *FspsUpd
  )
{
//[-start-161123-IB07250310-modify]//
  CHIPSET_CONFIGURATION            *SystemConfiguration;
//[-end-161123-IB07250310-modify]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *VariableServices;
  UINTN                            VariableSize = 0;
  EFI_STATUS                       Status;

  DEBUG ((DEBUG_INFO, "Update FspsUpd from setup option...\n"));

  Status = PeiServicesLocatePpi (
               &gEfiPeiReadOnlyVariable2PpiGuid,
               0,
               NULL,
               (VOID **) &VariableServices
               );
  ASSERT_EFI_ERROR (Status);

//[-start-161123-IB07250310-modify]//
  VariableSize = sizeof (CHIPSET_CONFIGURATION);
//[-end-161123-IB07250310-modify]//
  SystemConfiguration = AllocateZeroPool (VariableSize);

//[-start-161123-IB07250310-modify]//
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               SystemConfiguration
                               );
//[-end-161123-IB07250310-modify]//

  if (Status == EFI_SUCCESS) {
    FspsUpd->FspsConfig.ActiveProcessorCores  = SystemConfiguration->ActiveProcessorCores;
    FspsUpd->FspsConfig.DisableCore1          = SystemConfiguration->Core1;
    FspsUpd->FspsConfig.DisableCore2          = SystemConfiguration->Core2;
    FspsUpd->FspsConfig.DisableCore3          = SystemConfiguration->Core3;
    FspsUpd->FspsConfig.VmxEnable             = SystemConfiguration->ProcessorVmxEnable;
    FspsUpd->FspsConfig.ProcTraceMemSize      = SystemConfiguration->ProcTraceMemSize;
    FspsUpd->FspsConfig.ProcTraceEnable       = SystemConfiguration->ProcTraceEnable;
    FspsUpd->FspsConfig.Eist                  = SystemConfiguration->EnableGv;
    FspsUpd->FspsConfig.BootPState            = SystemConfiguration->BootPState;
    FspsUpd->FspsConfig.EnableCx              = SystemConfiguration->EnableCx;
    FspsUpd->FspsConfig.C1e                   = SystemConfiguration->EnableCxe;
    FspsUpd->FspsConfig.BiProcHot             = SystemConfiguration->EnableProcHot;
    FspsUpd->FspsConfig.PkgCStateLimit        = (MAX_PKG_C_STATE)SystemConfiguration->MaxPkgCState;
    FspsUpd->FspsConfig.EnableCx              = SystemConfiguration->EnableCx;
    FspsUpd->FspsConfig.CStateAutoDemotion    = SystemConfiguration->CStateAutoDemotion;
    FspsUpd->FspsConfig.CStateUnDemotion      = SystemConfiguration->CStateUnDemotion;
    FspsUpd->FspsConfig.MaxCoreCState         = SystemConfiguration->MaxCoreCState;
    FspsUpd->FspsConfig.PkgCStateDemotion     = SystemConfiguration->PkgCStateDemotion;
    FspsUpd->FspsConfig.PkgCStateUnDemotion   = SystemConfiguration->PkgCStateUnDemotion;
    FspsUpd->FspsConfig.TurboMode             = SystemConfiguration->TurboModeEnable;
  }
  return EFI_SUCCESS;
}

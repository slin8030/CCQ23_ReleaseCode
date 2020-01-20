/** @file
  Implementation of Fsp SA Policy Initialization.

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

#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/SaPolicy.h>
#include <Ppi/SaMiscConfig.h>
#include <Ppi/MemoryConfig.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SmbusLib.h>
#include <Library/MmPciLib.h>
#include <Library/ConfigBlockLib.h>

#include <IndustryStandard/Pci.h>
#include <Ppi/DramPolicyPpi.h>
#include <ScAccess.h>
#include <Ppi/FirmwareVolume.h>
#include <Pi/PiFirmwareFile.h>
#include <Pi/PiPeiCis.h>
#include <Core/Pei/PeiMain.h>

//[-start-161123-IB07250310-add]//
extern EFI_GUID gPeiLogoGuid;
extern EFI_GUID gPeiDefaultVbtGuid;
extern EFI_GUID gVbtMipiAuoGuid;
extern EFI_GUID gVbtMipiSharpGuid;
extern EFI_GUID gVbtMipiJdiGuid;
extern EFI_GUID gVbtEdpTypeCGuid;

BOOLEAN
IsRunCrisisRecoveryMode (
  VOID
  );
//[-end-161123-IB07250310-add]//

/**
  PeiGetSectionFromFv finds the file in FV and gets file Address and Size

  @param[in] NameGuid              - File GUID
  @param[out] Address              - Pointer to the File Address
  @param[out] Size                 - Pointer to File Size

  @retval EFI_SUCCESS                Successfull in reading the section from FV
**/
EFI_STATUS
EFIAPI
PeiGetSectionFromFv (
  IN CONST  EFI_GUID        NameGuid,
  OUT VOID                  **Address,
  OUT UINT32               *Size
  )
{
  EFI_STATUS  Status;
  EFI_PEI_FIRMWARE_VOLUME_PPI          *FvPpi;
  EFI_FV_FILE_INFO                     FvFileInfo;
  PEI_CORE_INSTANCE                    *PrivateData;
  UINTN                                CurrentFv;
  PEI_CORE_FV_HANDLE                   *CoreFvHandle;
  EFI_PEI_FILE_HANDLE                  VbtFileHandle;
  EFI_GUID                             *VbtGuid;
  EFI_COMMON_SECTION_HEADER            *Section;
  CONST EFI_PEI_SERVICES               **PeiServices;

  PeiServices = GetPeiServicesTablePointer ();

  PrivateData = PEI_CORE_INSTANCE_FROM_PS_THIS(PeiServices);

  Status = PeiServicesLocatePpi (
            &gEfiFirmwareFileSystem2Guid,
            0,
            NULL,
            (VOID **) &FvPpi
            );
  ASSERT_EFI_ERROR (Status);

  CurrentFv = PrivateData->CurrentPeimFvCount;
  CoreFvHandle = &(PrivateData->Fv[CurrentFv]);

  Status = FvPpi->FindFileByName (FvPpi, &NameGuid, &CoreFvHandle->FvHandle, &VbtFileHandle);
  if (!EFI_ERROR(Status) && VbtFileHandle != NULL) {

  DEBUG ((DEBUG_INFO, "Find SectionByType \n"));

    Status = FvPpi->FindSectionByType (FvPpi, EFI_SECTION_RAW, VbtFileHandle, (VOID **) &VbtGuid);
    if (!EFI_ERROR (Status)) {

    DEBUG ((DEBUG_INFO, "GetFileInfo \n"));

      Status = FvPpi->GetFileInfo (FvPpi, VbtFileHandle, &FvFileInfo);
      Section = (EFI_COMMON_SECTION_HEADER *)FvFileInfo.Buffer;

      if (IS_SECTION2 (Section)) {
        ASSERT (SECTION2_SIZE (Section) > 0x00FFFFFF);
        *Size = SECTION2_SIZE (Section) - sizeof (EFI_COMMON_SECTION_HEADER2);
        *Address = ((UINT8 *)Section + sizeof (EFI_COMMON_SECTION_HEADER2));
      } else {
        *Size = SECTION_SIZE (Section) - sizeof (EFI_COMMON_SECTION_HEADER);
        *Address = ((UINT8 *)Section + sizeof (EFI_COMMON_SECTION_HEADER));
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  Performs FSP SA PEI Policy initialization in pre-memory.

  @param[in][out]  FspmUpd             Pointer to FSP UPD Data.

  @retval          EFI_SUCCESS         FSP UPD Data is updated.
  @retval          EFI_NOT_FOUND       Fail to locate required PPI.
  @retval          Other               FSP UPD Data update process fail.
**/
EFI_STATUS
EFIAPI
PeiFspSaPolicyInitPreMem (
  IN OUT FSPM_UPD    *FspmUpd
  )
{
//[-start-161123-IB07250310-modify]//
  CHIPSET_CONFIGURATION            *SystemConfiguration;
//[-end-161123-IB07250310-modify]//
  UINTN                            VariableSize = 0;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *VariableServices;
  EFI_STATUS                       Status;

  DEBUG ((DEBUG_INFO, "Wrapper-PeiFspSaPolicyInitPreMem - Start\n"));

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
    FspmUpd->FspmConfig.IgdDvmt50PreAlloc = SystemConfiguration->IgdDvmt50PreAlloc;
    FspmUpd->FspmConfig.IgdApertureSize = SystemConfiguration->IgdApertureSize;
    FspmUpd->FspmConfig.GttSize = SystemConfiguration->GTTSize;
    FspmUpd->FspmConfig.Igd = SystemConfiguration->Igd;
    FspmUpd->FspmConfig.PrimaryVideoAdaptor = SystemConfiguration->PrimaryVideoAdaptor;

    if (SystemConfiguration->NpkEn == 3) { // Auto
      //
      // For Auto , enable NPK for Android and disable for Windows
      //
      if (SystemConfiguration->OsSelection == 1) { // Android
        FspmUpd->FspmConfig.NpkEn          = 1;
      } else {
        FspmUpd->FspmConfig.NpkEn          = 0;
      }
    } else { // use setup value
      FspmUpd->FspmConfig.NpkEn            = SystemConfiguration->NpkEn;              ///< 0-disabled, 1-enabled, 2-debugger, 3-auto
    }
    FspmUpd->FspmConfig.FwTraceEn          = SystemConfiguration->FwTraceEn;
    FspmUpd->FspmConfig.FwTraceDestination = SystemConfiguration->FwTraceDestination;
    FspmUpd->FspmConfig.RecoverDump        = SystemConfiguration->RecoverDump;
    FspmUpd->FspmConfig.Msc0Size           = SystemConfiguration->Msc0Size;
    FspmUpd->FspmConfig.Msc0Wrap           = SystemConfiguration->Msc0Wrap;
    FspmUpd->FspmConfig.Msc1Size           = SystemConfiguration->Msc1Size;
    FspmUpd->FspmConfig.Msc1Wrap           = SystemConfiguration->Msc1Wrap;
    FspmUpd->FspmConfig.PtiMode            = SystemConfiguration->PtiMode;
    FspmUpd->FspmConfig.PtiTraining        = SystemConfiguration->PtiTraining;
    FspmUpd->FspmConfig.PtiSpeed           = SystemConfiguration->PtiSpeed;
    FspmUpd->FspmConfig.PunitMlvl          = SystemConfiguration->PunitMlvl;
    FspmUpd->FspmConfig.PmcMlvl            = SystemConfiguration->PmcMlvl;
    FspmUpd->FspmConfig.SwTraceEn          = SystemConfiguration->SwTraceEn;
  }

  DEBUG((DEBUG_INFO, "Wrapper-PeiFspSaPolicyInitPreMem - End\n"));
  return EFI_SUCCESS;
}


/**
  Performs FSP SA PEI Policy initialization.

  @param[in][out]  FspsUpd             Pointer to FSP UPD Data.

  @retval          EFI_SUCCESS         FSP UPD Data is updated.
  @retval          EFI_NOT_FOUND       Fail to locate required PPI.
  @retval          Other               FSP UPD Data update process fail.
**/
EFI_STATUS
EFIAPI
PeiFspSaPolicyInit (
  IN OUT FSPS_UPD    *FspsUpd
  )
{
  EFI_STATUS                       Status;
  SA_MISC_CONFIG                   *MiscConfig;
  SI_SA_POLICY_PPI                 *SiSaPolicyPpi;
  VOID                             *Buffer;
  UINT32                           Size;
  EFI_GUID                         PeiLogoGuid = gPeiLogoGuid;
  EFI_GUID                         PeiVbtGuid;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *VariableServices;
//[-start-161123-IB07250310-modify]//
  CHIPSET_CONFIGURATION             *SystemConfiguration;
//[-end-161123-IB07250310-modify]//
  UINTN                            VariableSize = 0;
  EFI_BOOT_MODE                    BootMode;

//[-start-161123-IB07250310-add]//
  Buffer = NULL;
  Size = 0;
  PeiVbtGuid = gPeiDefaultVbtGuid;
//[-end-161123-IB07250310-add]//
  //
  // Locate SiSaPolicyPpi
  //
  SiSaPolicyPpi = NULL;
  MiscConfig = NULL;
  Status = PeiServicesLocatePpi(
             &gSiSaPolicyPpiGuid,
             0,
             NULL,
             (VOID **)&SiSaPolicyPpi
             );
  if (EFI_ERROR (Status) == FALSE) {
    Status = GetConfigBlock((VOID *)SiSaPolicyPpi, &gSaMiscConfigGuid, (VOID *)&MiscConfig);
  }

  Status = PeiServicesGetBootMode(&BootMode);

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             (VOID **) &VariableServices
             );
  ASSERT_EFI_ERROR (Status);

  DEBUG((DEBUG_INFO, "Update North Cluster FspsUpd from setup option...\n"));

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
    FspsUpd->FspsConfig.PmSupport = SystemConfiguration->PmSupport;
    FspsUpd->FspsConfig.EnableRenderStandby = SystemConfiguration->EnableRenderStandby;
    FspsUpd->FspsConfig.CdClock = SystemConfiguration->CdClock;
    FspsUpd->FspsConfig.PavpEnable = SystemConfiguration->PavpEnable;
//[-start-161123-IB07250310-modify]//
    FspsUpd->FspsConfig.PeiGraphicsPeimInit = IsRunCrisisRecoveryMode() ? TRUE : SystemConfiguration->PeiGraphicsPeimInit;
//[-end-161123-IB07250310-modify]//

    FspsUpd->FspsConfig.IpuEn = SystemConfiguration->IpuEn;
    FspsUpd->FspsConfig.IpuAcpiMode = SystemConfiguration->IpuAcpiMode;
  }
  //
  // Update VbtGuid based on VbtSelect opion from setup
  //
  if (SystemConfiguration->VbtSelect == 0) {
    PeiVbtGuid = gPeiDefaultVbtGuid;
  } else if (SystemConfiguration->VbtSelect == 1) {
    PeiVbtGuid = gVbtMipiAuoGuid;
  } else if (SystemConfiguration->VbtSelect == 2) {
    PeiVbtGuid = gVbtMipiSharpGuid;
  } else if (SystemConfiguration->VbtSelect == 3) {
    PeiVbtGuid = gVbtMipiJdiGuid;
  } else if (SystemConfiguration->VbtSelect == 4) {
    PeiVbtGuid = gVbtEdpTypeCGuid;
  }
  //
  // Update UPD:LogoPtr
  //
  PeiGetSectionFromFv(PeiVbtGuid, &Buffer, &Size);
  if (Buffer == NULL) {
    DEBUG(( DEBUG_ERROR, "Could not locate VBT"));
  }

  if (BootMode == BOOT_ON_S3_RESUME) {
    FspsUpd->FspsConfig.GraphicsConfigPtr = (UINT32) NULL;
  } else {
    FspsUpd->FspsConfig.GraphicsConfigPtr = (UINT32) Buffer;
  }
  DEBUG(( DEBUG_INFO, "VbtPtr from PeiGetSectionFromFv is 0x%x\n", FspsUpd->FspsConfig.GraphicsConfigPtr));
  DEBUG(( DEBUG_INFO, "VbtSize from PeiGetSectionFromFv is 0x%x\n", Size));

  PeiGetSectionFromFv(PeiLogoGuid, &Buffer, &Size);
  if (Buffer == NULL) {
     DEBUG(( DEBUG_ERROR, "Could not locate Logo"));
  }
  FspsUpd->FspsConfig.LogoPtr  = (UINT32) Buffer;
  FspsUpd->FspsConfig.LogoSize          = Size;
  DEBUG(( DEBUG_INFO, "LogoPtr from PeiFspSaPolicyInit PeiGetSectionFromFv is 0x%x\n", Buffer));
  DEBUG(( DEBUG_INFO, "LogoSize from PeiFspSaPolicyInit PeiGetSectionFromFv is 0x%x\n", Size));

  return EFI_SUCCESS;
}
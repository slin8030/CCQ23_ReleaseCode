/** @file
  Do Platform Stage System Agent initialization.

@copyright
  Copyright (c) 2013 - 2016 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by the
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor. This file may be modified by the user, subject to
  the additional terms of the license agreement.

@par Specification Reference:
**/

#include <PeiSaPolicyUpdate.h>
#include <Library/DebugLib.h>
#include <Library/PeiSaPolicyLib.h>
#include <Library/PlatformConfigDataLib.h>
#include <Ppi/FirmwareVolume.h>
#include <Pi/PiFirmwareFile.h>
#include <Pi/PiPeiCis.h>
#include <Core/Pei/PeiMain.h>
//[-start-160226-IB03090424-add]//
#include <Library/SteppingLib.h>
//[-end-160226-IB03090424-add]//

//[-start-151228-IB03090424-modify]//

/**
  UpdatePeiSaPolicy performs SA PEI Policy initialzation

  @param[in out] SiSaPolicyPpi     - SI_SA_POLICY PPI 

  @retval EFI_SUCCESS              The policy is installed and initialized.
**/
EFI_STATUS
UpdatePeiSaPolicy (
  IN OUT   SI_SA_POLICY_PPI      *SiSaPolicyPpi
  )
{
  EFI_STATUS                      Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariableServices;
  CHIPSET_CONFIGURATION            SystemConfiguration;
  UINTN                           VarSize;
  SA_MISC_CONFIG                  *MiscConfig = NULL;
  GRAPHICS_CONFIG                 *GtConfig = NULL;
  IPU_CONFIG                      *IpuPolicy = NULL;
  VOID*                           Buffer;
  UINT32                          Size;
  EFI_GUID                        PeiLogoGuid        = { 0x7BB28B99, 0x61BB, 0x11D5, 0x9A, 0x5D, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D };
  EFI_GUID                        VbtGuid            = { 0xE08CA6D5, 0x8D02, 0x43ae, 0xAB, 0xB1, 0x95, 0x2C, 0xC7, 0x87, 0xC9, 0x33 };
  VBT_INFO                        VbtInfo;
  SC_POLICY_PPI                   *ScPolicyPpi;
  SC_VTD_CONFIG                   *VtdConfig;
//[-start-160616-IB06720412-add]//
  BXT_SERIES                      SocSeries;
  BXT_STEPPING                    SocStepping;
//[-end-160616-IB06720412-add]//
  

  DEBUG ((DEBUG_INFO, "Entering Get Config Block function call from UpdatePeiSaPolicy\n"));

  Status = GetConfigBlock ((VOID *)SiSaPolicyPpi, &gSaMiscConfigGuid , (VOID *)&MiscConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *)SiSaPolicyPpi, &gGraphicsConfigGuid, (VOID *)&GtConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *)SiSaPolicyPpi, &gIpuConfigGuid, (VOID *)&IpuPolicy);
  ASSERT_EFI_ERROR (Status);


  //
  // Locate system configuration variable
  //
  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid, // GUID
             0,                            // INSTANCE
             NULL,                         // EFI_PEI_PPI_DESCRIPTOR
             &VariableServices             // PPI
             );
  ASSERT_EFI_ERROR ( Status);

  //
  // Locate SC Policy
  //
  Status = PeiServicesLocatePpi (
             &gScPolicyPpiGuid,
             0,
             NULL,
             (VOID **)&ScPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gVtdConfigGuid, (VOID *) &VtdConfig);
  ASSERT_EFI_ERROR (Status);

  //
  // Get System configuration variables
  //
  VarSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160821-IB07400772-modify]//
//  Status = GetSystemConfigData(&SystemConfiguration, &VarSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );
//[-end-160821-IB07400772-modify]//

  if (!EFI_ERROR(Status)) {
    //
    // Get the Platform Configuration from SetupData
    //
    GtConfig->GttMmAdr = GTTMM_BASE_ADDRESS;
    GtConfig->GmAdr = GMADR_BASE_ADDRESS;
    GtConfig->PeiGraphicsPeimInit = SystemConfiguration.PeiGraphicsPeimInit;
    GtConfig->PmSupport = SystemConfiguration.PmSupport;
    GtConfig->EnableRenderStandby = SystemConfiguration.EnableRenderStandby;
    GtConfig->CdClock = SystemConfiguration.CdClock;
    GtConfig->PavpEnable = SystemConfiguration.PavpEnable;


    MiscConfig->FastBoot = 1;
    MiscConfig->DynSR = 1;

    IpuPolicy->SaIpuEnable = SystemConfiguration.IpuEn;

//[-start-160302-IB03090424-modify]//
  //
  // Workaround for [1504141196][[APL RCR] IPU and VT-D dependency implementation] -fix in C0 stepping
  //
//[-start-160317-IB03090425-modify]//
//[-start-160616-IB06720412-modify]//
  SocSeries   = GetBxtSeries ();
  SocStepping = BxtStepping ();
//[-start-160826-IB03090433-modify]//
  if ((SocSeries == BxtP && SocStepping <= BxtPB1) || (SocSeries != BxtP && SocStepping < BxtC0)) {
//[-end-160826-IB03090433-modify]//
//[-end-160616-IB06720412-modify]//
//[-end-160317-IB03090425-modify]//
    if (IpuPolicy->SaIpuEnble == 1) {
      VtdConfig->VtdEnable = 0;
    }
  }
//[-end-160302-IB03090424-modify]//

    IpuPolicy->IpuAcpiMode = SystemConfiguration.IpuAcpiMode;
    IpuPolicy->IpuMmAdr = IPUMM_BASE_ADDRESS;
  }
  
  PeiGetSectionFromFv(PeiLogoGuid, &Buffer, &Size);
  if (Buffer == NULL) {
    DEBUG(( DEBUG_ERROR, "Could not locate Pei Logo"));
  }
  GtConfig->LogoPtr           = Buffer;
  GtConfig->LogoSize          = Size;
  DEBUG(( DEBUG_INFO, "LogoPtr from PeiGetSectionFromFv is 0x%x\n", Buffer));
  DEBUG(( DEBUG_INFO, "LogoSize from PeiGetSectionFromFv is 0x%x\n", Size));   
  
  PeiGetSectionFromFv(VbtGuid, &Buffer, &Size);
  if (Buffer == NULL) {
    DEBUG(( DEBUG_ERROR, "Could not locate VBT"));
  }
  GtConfig->GraphicsConfigPtr = Buffer;

  //Build the VBT data into HOB for DXE GOP  
  VbtInfo.VbtAddress = (EFI_PHYSICAL_ADDRESS)Buffer;
  VbtInfo.VbtSize = Size;
  DEBUG(( DEBUG_INFO, "VbtInfo VbtAddress is 0x%x\n", Buffer));
  DEBUG(( DEBUG_INFO, "VbtInfo VbtSize is 0x%x\n", Size));
  
  BuildGuidDataHob (
  &gVbtInfoGuid,
  &VbtInfo,
  sizeof (VbtInfo)
  );

            
  return EFI_SUCCESS;
}
//[-end-151228-IB03090424-modify]//

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


/** @file
 PEI Chipset Services Library.

 This file contains only one function that is PeiCsSvcPlatformStage2Init().
 The function PeiCsSvcPlatformStage2Init() use chipset services to initialization 
 chipset in stage 2 of PEi phase.

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

#include <Uefi/UefiBaseType.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Guid/PlatformInfo.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <ChipsetSetupConfig.h>
#include <Guid/BxtVariable.h>
#include <Guid/TpmInstance.h>
//[-start-160107-IB08450333-add//
#include <Guid/PttPTPInstanceGuid.h>
//[-end-160107-IB08450333-add//
#include <Guid/GlobalVariable.h>
#include <Library/IoLib.h>
#include <Library/I2CLib.h>
#ifdef TABLET_PF_ENABLE  
#include <Library/PmicLib.h>
#endif
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/VariableLib.h>
//[-start-160421-IB08450341-add]//
#include <Library/HeciMsgLib.h>
//[-end-160421-IB08450341-add]//

EFI_STATUS
FtpmSupportInit (
  IN EFI_PEI_SERVICES               **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR      *NotifyDescriptor,
  IN VOID                           *Ppi
  );

EFI_PEI_NOTIFY_DESCRIPTOR mTpmDeviceSeleted[] = {
  (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiTpmDeviceSelectedGuid,
  FtpmSupportInit
};

//[-start-160421-IB08450341-add]//
/**
  Depends on setup config to switching fTPM/dTPM 
 
  @param[in] PeiServices       An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation

  @retval EFI_SUCCESS          Operation completed successfully.
  @others                      Failed on cooperative functions.
**/
EFI_STATUS
PlatformTargetTpmSwitch (
  IN CONST EFI_PEI_SERVICES                  **PeiServices
)
{

  EFI_STATUS                           Status;
  BOOLEAN                              CurrentPttEnabled;
  BOOLEAN                              TargetFTpm;
  UINTN                                VariableSize;  
  EFI_PEI_READ_ONLY_VARIABLE2_PPI      *Variable;
  CHIPSET_CONFIGURATION                *SystemConfiguration;
//[-start-160701-IB07220101-add]//
  EFI_BOOT_MODE                         BootMode;

  //
  // Skip to switch target Tpm during S3 resume
  //
  Status = (*PeiServices)->GetBootMode(PeiServices, &BootMode);
//[-start-160701-IB06740505-modify]//
  if ((BootMode == BOOT_ON_S3_RESUME) || (BootMode == BOOT_IN_RECOVERY_MODE)) {
    return EFI_SUCCESS;
  }
//[-end-160701-IB06740505-modify]//
//[-end-160701-IB07220101-add]//
  
  if (!FeaturePcdGet(PcdRuntimeFtpmDtpmSwitch) || (!FeaturePcdGet(FtpmSupport))) {
//[-start-171002-IB07400914-add]//
    if (!FeaturePcdGet(FtpmSupport)) {
      //
      // fTPM is disabled, send HECI cmd to disable fTPM if already enabled.
      //
      CurrentPttEnabled = FALSE;
      Status = PttHeciGetState (&CurrentPttEnabled);
      ASSERT_EFI_ERROR(Status);
      
      if (CurrentPttEnabled != FALSE) {
        Status = PttHeciSetState (FALSE);
        ASSERT_EFI_ERROR(Status);
        
        Status = PeiServicesResetSystem();
        ASSERT_EFI_ERROR(Status);
        CpuDeadLoop();
      }
    }
//[-end-171002-IB07400914-add]//
    return EFI_SUCCESS;
  }
  
  TargetFTpm = TRUE;
   
  //
  // Get tar get fTPM type from SystemConfiguration
  //
  Status = (*PeiServices)->LocatePpi (
                             PeiServices,
                             &gEfiPeiReadOnlyVariable2PpiGuid,
                             0,
                             NULL,
                             (VOID **)&Variable
                             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  VariableSize = PcdGet32 (PcdSetupConfigSize);
  SystemConfiguration = (CHIPSET_CONFIGURATION *)AllocatePool (VariableSize);
  ASSERT (SystemConfiguration != NULL);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = Variable->GetVariable (
                       Variable,
                       SETUP_VARIABLE_NAME,
                       &gSystemConfigurationGuid,
                       NULL,
                       &VariableSize,
                       SystemConfiguration
                       );
  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status)) {
    TargetFTpm = TRUE;
  } else {
    TargetFTpm = SystemConfiguration->TargetTPM == TARGET_TPM_FTPM ? TRUE : FALSE;
  }
  FreePool (SystemConfiguration); 

  //
  // Ask HECI about current target TPM is TPM or dTPM.
  //
  Status = PttHeciGetState (&CurrentPttEnabled);
//[-start-160428-IB08450344-modify]//
  if (EFI_ERROR(Status)) {
    return Status;
  }
//[-end-160428-IB08450344-modify]//

  //
  // Do target TPM switching, if target TPM is not synchorized.
  //  
  if (CurrentPttEnabled != TargetFTpm) {
//[-start-160428-IB08450344-modify]//
    Status = PttHeciSetState (TargetFTpm);
    if (EFI_ERROR(Status)) {
      return Status;
    }
//[-end-160428-IB08450344-modify]//
    Status = PeiServicesResetSystem();
    ASSERT_EFI_ERROR(Status);
    CpuDeadLoop();
  }
  
  return EFI_SUCCESS;
  
}
//[-end-160421-IB08450341-add]//

EFI_STATUS
FtpmSupportInit (
  IN EFI_PEI_SERVICES                  **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR         *NotifyDescriptor,
  IN VOID                              *Ppi
  )
{
  UINTN                                Size;
//[-start-160421-IB08450341-add]//
  EFI_STATUS                           Status;
  BOOLEAN                              CurrentPttEnabled;
  VOID                                 *Dummy;
//[-end-160421-IB08450341-add]//
//[-start-160701-IB07220101-add]//
  EFI_BOOT_MODE                        BootMode;
  UINTN                                VariableSize;  
  EFI_PEI_READ_ONLY_VARIABLE2_PPI      *Variable;
  CHIPSET_CONFIGURATION                *SystemConfiguration;
  BOOLEAN                              TargetFTpm;
//[-end-160701-IB07220101-add]//

//[-start-160421-IB08450341-modify]//
  //
  // Check TPM is selected.
  // Do not execute follow code if TPM is not selected.
  //
  Status = (*PeiServices)->LocatePpi (
                             PeiServices,
                             &gEfiTpmDeviceSelectedGuid,
                             0,
                             NULL,
                             (VOID **)&Dummy
                             );
  if (EFI_ERROR(Status)) {
    return Status;
  }

//[-start-160701-IB07220101-add]//
  Status = (*PeiServices)->GetBootMode(PeiServices, &BootMode);
//[-end-160701-IB07220101-add]//
//[-start-160701-IB06740505-add]//
  if (BootMode == BOOT_IN_RECOVERY_MODE) {
    return EFI_UNSUPPORTED;
  }
//[-end-160701-IB06740505-add]//

  //
  // Change PcdTpmInstanceGuid to PTT instance GUID. Otherwise this value is
  // configured by TreeConfigPeim
  //  
//[-start-160701-IB07220101-modify]//
  if (FeaturePcdGet(FtpmSupport)) {

    if (BootMode == BOOT_ON_S3_RESUME) {
      //
      // Get tar get fTPM type from SystemConfiguration
      //
      Status = (*PeiServices)->LocatePpi (
                                 PeiServices,
                                 &gEfiPeiReadOnlyVariable2PpiGuid,
                                 0,
                                 NULL,
                                 (VOID **)&Variable
                                 );
      ASSERT_EFI_ERROR (Status);
      if (EFI_ERROR(Status)) {
        return Status;
      }
      
      VariableSize = PcdGet32 (PcdSetupConfigSize);
      SystemConfiguration = (CHIPSET_CONFIGURATION *)AllocatePool (VariableSize);
      ASSERT (SystemConfiguration != NULL);
      if (EFI_ERROR(Status)) {
        return Status;
      }
      
      Status = Variable->GetVariable (
                           Variable,
                           SETUP_VARIABLE_NAME,
                           &gSystemConfigurationGuid,
                           NULL,
                           &VariableSize,
                           SystemConfiguration
                           );
      ASSERT_EFI_ERROR(Status);
      if (EFI_ERROR(Status)) {
        TargetFTpm = TRUE;
      } else {
        TargetFTpm = SystemConfiguration->TargetTPM == TARGET_TPM_FTPM ? TRUE : FALSE;
      }
      if (TargetFTpm) {
        Size = sizeof (EFI_GUID);
        PcdSetPtr (PcdTpmInstanceGuid, &Size, &gTpmDeviceInstanceTpm20PttPtpGuid);
      }
    } else {
  
      //
      // Ask HECI about current target TPM is TPM or dTPM.
      //
      Status = PttHeciGetState (&CurrentPttEnabled);
      if (EFI_ERROR(Status)) {
        CurrentPttEnabled = FALSE;
      }
  
      if (CurrentPttEnabled) {
        Size = sizeof (EFI_GUID);
        PcdSetPtr (PcdTpmInstanceGuid, &Size, &gTpmDeviceInstanceTpm20PttPtpGuid);
      }    
    }
  }
//[-end-160701-IB07220101-modify]//
//[-end-160421-IB08450341-modify]//

  return EFI_SUCCESS;
}

#if TABLET_PF_ENABLE
VOID
ForceOffModem(PMIC_TYPE PmicId) {
  UINT8 Value;
  EFI_STATUS Status;
  if (PmicId == PMIC_TYPE_CRC_PLUS) {
    Value = 0;
    Status = ByteWriteI2C(PMIC_I2C_BUSNO, PMIC_PAGE_1_I2C_ADDR, PMIC_REG_MODEMCTRL_CRCP, 1, &Value);
    if (EFI_SUCCESS != Status)  {
      DEBUG ((DEBUG_INFO, "CrcPlusPmicWrite8 for Modem failed :%r\n", Status));
    }
  }
 
}
#endif

/**
 Platform initialization in PEI phase stage 2.

 @param[in]         None

 @retval            EFI_SUCCESS         This function alway return successfully
*/
EFI_STATUS
PlatformStage2Init(
  VOID
  )
{
//[-start-160421-IB08450341-remove]//
  //UINTN                           VariableSize;  
//[-end-160421-IB08450341-remove]//
  EFI_STATUS                      Status;
//[-start-160421-IB08450341-remove]//
  //EFI_PEI_READ_ONLY_VARIABLE2_PPI *Variable;
  //CHIPSET_CONFIGURATION           *SystemConfiguration;
//[-end-160421-IB08450341-remmove]//
  CONST EFI_PEI_SERVICES          **PeiServices;
#ifdef TABLET_PF_ENABLE  
  EFI_PLATFORM_INFO_HOB           *PlatformInfo;
  EFI_PEI_HOB_POINTERS            GuidHob;
#endif
  EFI_BOOT_MODE                   BootMode;

  PeiServices = GetPeiServicesTablePointer ();

  Status = (*PeiServices)->GetBootMode(PeiServices, &BootMode);

//  if (BootMode != BOOT_ON_S3_RESUME && BootMode != BOOT_IN_RECOVERY_MODE) {
//    if (FeaturePcdGet (PcdUnsignedFvSupported)) {
//      BuildFvHob (
//        FixedPcdGet32 (PcdFlashUnsignedFvRegionBase),
//        FixedPcdGet32 (PcdFlashUnsignedFvRegionSize)
//      );
//    }
//
//    BuildFvHob (
//      FixedPcdGet32 (PcdFlashNvStorageVariableBase),
//      (FixedPcdGet32 (PcdFlashNvStorageVariableSize)
//       + FixedPcdGet32 (PcdFlashNvStorageFtwWorkingSize)
//       + FixedPcdGet32 (PcdFlashNvStorageFtwSpareSize)
//       + FixedPcdGet32 (PcdFlashNvStorageFactoryCopySize))
//    );
//
//  }

//[-start-160421-IB08450341-modify]//
  PlatformTargetTpmSwitch (PeiServices);
  Status = PeiServicesNotifyPpi (mTpmDeviceSeleted); 
//[-end-160421-IB08450341-modify]//


  return EFI_SUCCESS;
}


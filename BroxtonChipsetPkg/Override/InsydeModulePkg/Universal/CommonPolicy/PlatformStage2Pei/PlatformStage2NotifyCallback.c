/** @file

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
#include <FastRecoveryData.h>
#include <Ppi/EmuPei.h>
#endif
//[-end-161123-IB07250310-modify]//
#include <Library/PeiOemSvcKernelLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <PostCode.h>
#include <Library/PeiChipsetSvcLib.h>
#include <Library/FlashRegionLib.h>

//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
EFI_STATUS
EFIAPI
PeimInitializeRecovery (
  );
#endif
//[-end-161123-IB07250310-modify]//

/**
  Install Firmware Volume Hob's once there is main memory.It installs
  firmware volume Hobs when main memory is available.

  @param PeiServices            General purpose services available to every PEIM.
  @param NotifyDescriptor       Pointer to EFI_PEI_NOTIFY_DESCRIPTOR
  @param Ppi                    EFI_PEI_FV_FILE_LOADER_PPI

  @retval   EFI_SUCCESS         if the interface could be successfully installed.

**/
EFI_STATUS
EFIAPI
MemoryDiscoveredPpiNotifyPlatformStage2Callback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  EFI_STATUS                          Status;
  EFI_BOOT_MODE                       BootMode;
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
  FAST_RECOVERY_DXE_TO_PEI_DATA       *DxeToPeiData = NULL;

  if (FeaturePcdGet (PcdUseFastCrisisRecovery)) {
    Status = PeiServicesLocatePpi (
               &gEmuPeiPpiGuid,
               0,
               NULL,
               (VOID *)&DxeToPeiData
               );
    if (EFI_ERROR (Status)) {
      //
      // We are not in EmuPei Phase.
      //
      DxeToPeiData = NULL;
    }
  }

  POST_CODE (PEI_MEMORY_CALLBACK);
  DEBUG ((EFI_D_INFO, "Enter MemoryDiscoveredPpiNotifyCallback\n"));
#endif
//[-end-161123-IB07250310-modify]//

  //
  // Get boot mode
  //
  Status = PeiServicesGetBootMode (&BootMode);
  ASSERT_EFI_ERROR (Status);

  if (BootMode == BOOT_ON_S3_RESUME) {
    POST_CODE (S3_MEMORY_CALLBACK);
  }

  PeiCsSvcStage2MemoryDiscoverCallback (&BootMode);

  //
  // OemServices
  //
  OemSvcInitMemoryDiscoverCallback (&BootMode);

//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
  //
  // Create the firmware volume HOB's
  //
  DEBUG ((EFI_D_INFO, "Create the firmware volume HOB\n"));
  if (BootMode == BOOT_IN_RECOVERY_MODE) {
    //
    // Prepare the recovery service
    //
    DEBUG ((EFI_D_INFO, "Prepare the recovery service\n"));
    Status = PeimInitializeRecovery ();
    ASSERT_EFI_ERROR (Status);
  } else if (BootMode != BOOT_ON_S3_RESUME) {
//    if (FeaturePcdGet (PcdUnsignedFvSupported)) {
//      BuildFvHob (
//        FixedPcdGet32 (PcdFlashUnsignedFvRegionBase),
//        FixedPcdGet32 (PcdEndOfFlashUnsignedFvRegionTotalSize)
//      );
//    }
//    BuildFvHob (
//      FixedPcdGet32 (PcdFlashNvStorageVariableBase),
//      (FixedPcdGet32 (PcdFlashNvStorageVariableSize) 
//       + FixedPcdGet32 (PcdFlashNvStorageFtwWorkingSize) 
//       + FixedPcdGet32 (PcdFlashNvStorageFtwSpareSize) 
//       + FixedPcdGet32 (PcdFlashNvStorageFactoryCopySize))
//    );
  }
#endif
//[-end-161123-IB07250310-modify]//
  BuildResourceDescriptorHob (
    EFI_RESOURCE_FIRMWARE_DEVICE,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    FdmGetBaseAddr (),
    FdmGetFlashAreaSize ()
    );

  //
  // Create a CPU hand-off information
  //
  BuildCpuHob (FixedPcdGet8 (PcdMemorySpaceSize), FixedPcdGet8 (PcdIoSpaceSize));
  DEBUG ((EFI_D_INFO, "MemoryDiscoveredPpiNotifyCallback Exit\n"));

  return Status;
}

/**
  To force system into Recovery mode.

  @param PeiServices            General purpose services available to every PEIM.
  @param NotifyDescriptor       Pointer to EFI_PEI_NOTIFY_DESCRIPTOR
  @param Ppi                    EFI_PEI_FV_FILE_LOADER_PPI

  @retval EFI_SUCCESS           Enter Recovery mode OK.

**/
EFI_STATUS
ForceEnterRecoveryEntry (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  //
  // Due to there is no common method to set a recovery request, so let OEM to decide the method.
  // (OemServices)
  //
  OemSvcSetRecoveryRequest ();

  //
  // Trigger a reset to force system enter recovery mode.
  //
  PeiServicesResetSystem ();

  return EFI_SUCCESS;
}

/**
  Use the PEI_READ_ONLY_VARIABLE_ACCESS_PPI to notify and set platform hardware switch value.

  @param  **PeiServices         General purpose services available to every PEIM.
  @param  *NotifyDescriptor     Pointer to EFI_PEI_NOTIFY_DESCRIPTOR.
  @param  *Ppi                  EFI_PEI_FV_FILE_LOADER_PPI
  
  @retval EFI_SUCCESS           if the interface could be successfully installed.
**/
EFI_STATUS
EFIAPI
SetPlatformHardwareSwitchNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
//[-start-161228-IB04530802-remove]//
//   EFI_STATUS                     Status;
//   PLATFORM_HARDWARE_SWITCH       PlatformHardwareSwitch;
//   PLATFORM_HARDWARE_SWITCH       EmptyPlatformHardwareSwitch;
//   PLATFORM_HARDWARE_SWITCH       *PlatformHardwareSwitchDptr;
// 
//   ZeroMem (&PlatformHardwareSwitch, sizeof (PLATFORM_HARDWARE_SWITCH));
//   ZeroMem (&EmptyPlatformHardwareSwitch, sizeof (PLATFORM_HARDWARE_SWITCH));
// 
//   PlatformHardwareSwitchDptr = &PlatformHardwareSwitch;
// 
//   PeiCsSvcSetPlatformHardwareSwitch(&PlatformHardwareSwitchDptr);
//   //
//   // OemServices
//   //
//   Status = OemSvcSetPlatformHardwareSwitch (
//              &PlatformHardwareSwitchDptr
//              );
// 
//   if (!EFI_ERROR (Status)) {
//     return Status;
//   }
// 
//   if (CompareMem (
// 	&EmptyPlatformHardwareSwitch, 
// 	&PlatformHardwareSwitch, 
// 	sizeof (PLATFORM_HARDWARE_SWITCH)) == 0) {
//     return Status;
//   }
// 
//   //
//   // Set PlatformHardwareSwitch data to HOB.
//   //
//   BuildGuidDataHob (
//     &gPlatformHardwareSwitchGuid, 
//     PlatformHardwareSwitchDptr, 
//     sizeof (PLATFORM_HARDWARE_SWITCH)
//     );
// 
//   return Status;
//[-end-161228-IB04530802-remove]//

//[-start-161228-IB04530802-add]//
  return EFI_SUCCESS;
//[-end-161228-IB04530802-add]//
}


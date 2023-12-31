/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/** @file
  Firmware Volume Block Driver for Lakeport Platform.

  Firmware volume block driver for FWH or SPI device. 
  It depends on which Flash Device Library to be linked with this driver.

Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#include <PiDxe.h>
#include "FvbService.h"
#include <Library/UefiRuntimeLib.h>
//[-start-151224-IB07220029-add]//
#include <Library/HobLib.h>
//[-end-151224-IB07220029-add]//
extern FWB_GLOBAL              mFvbModuleGlobal;
extern EFI_GUID        gEfiAlternateFvBlockGuid;
/**
  Call back function on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.

  Fixup internal data so that the driver is callable in EFI  runtime 
  in virtual mode. Convert the mFvbModuleGlobal date items to there 
  virtual address.

  @param  Event     Event whose notification function is being invoked.
  @param  Context   The context of the Notification context. Not used in
                    this call back function.

**/
VOID
EFIAPI
FvbVirtualddressChangeEvent (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  )
{
  EFI_FW_VOL_INSTANCE                   *FwhInstance;
  UINTN                                 Index;

  //
  // Convert the base address of all the instances
  //
  for (Index = 0; Index < mFvbModuleGlobal.NumFv; Index++) {
    FwhInstance = GetFvbInstance (Index);
    EfiConvertPointer (0, (VOID **) &FwhInstance->FvBase);
  }

  EfiConvertPointer (0, (VOID **) &mFvbModuleGlobal.FvInstance); 
}


/**
  The function installs EFI_FIRMWARE_VOLUME_BLOCK protocol
  for each FV in the system.

  @param[in]  FwhInstance   The pointer to a FW volume instance structure,
                            which contains the information about one FV.
  @param[in]  InstanceNum   The instance number which can be used as a ID 
                            to locate this FwhInstance in other functions.

  @retval     VOID

**/
VOID
InstallFvbProtocol (
  IN  EFI_FW_VOL_INSTANCE               *FwhInstance,
  IN  UINTN                             InstanceNum
  )
{
  EFI_FW_VOL_BLOCK_DEVICE               *FvbDevice;
  EFI_FIRMWARE_VOLUME_HEADER            *FwVolHeader;
  EFI_STATUS                            Status;
  EFI_HANDLE                            FwbHandle;
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL    *OldFwbInterface;

  FvbDevice = (EFI_FW_VOL_BLOCK_DEVICE *) AllocateRuntimeCopyPool (
                                            sizeof (EFI_FW_VOL_BLOCK_DEVICE), 
                                            &mFvbDeviceTemplate
                                            );
  ASSERT (FvbDevice != NULL);

  FvbDevice->Instance = InstanceNum;
  FwVolHeader = &FwhInstance->VolumeHeader;
  
  //
  // Set up the devicepath
  //  
  //DEBUG ((EFI_D_INFO, "FwBlockService.c: Setting up DevicePath for 0x%lx:\n", FwhInstance->FvBase));
  if (FwVolHeader->ExtHeaderOffset == 0) {
      //
      // FV does not contains extension header, then produce MEMMAP_DEVICE_PATH
      //
    FvbDevice->DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) AllocateRuntimeCopyPool (sizeof (FV_MEMMAP_DEVICE_PATH), &mFvMemmapDevicePathTemplate);
    ((FV_MEMMAP_DEVICE_PATH *) FvbDevice->DevicePath)->MemMapDevPath.StartingAddress = FwhInstance->FvBase;
    ((FV_MEMMAP_DEVICE_PATH *) FvbDevice->DevicePath)->MemMapDevPath.EndingAddress   = FwhInstance->FvBase + FwVolHeader->FvLength - 1;
  } else {
    FvbDevice->DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) AllocateRuntimeCopyPool (sizeof (FV_PIWG_DEVICE_PATH), &mFvPIWGDevicePathTemplate);
    CopyGuid (
      &((FV_PIWG_DEVICE_PATH *)FvbDevice->DevicePath)->FvDevPath.FvName, 
      (GUID *)(UINTN)(FwhInstance->FvBase + FwVolHeader->ExtHeaderOffset)
      );
  }  

  //
  // Find a handle with a matching device path that has supports FW Block protocol
  //
  Status = gBS->LocateDevicePath (&gEfiFirmwareVolumeBlockProtocolGuid, &FvbDevice->DevicePath, &FwbHandle);
  if (EFI_ERROR (Status) ) {
    //
    // LocateDevicePath fails so install a new interface and device path
    //
    //DEBUG ((EFI_D_INFO, "FwBlockService.c: LocateDevicePath failed, install new interface 0x%lx:\n", FwhInstance->FvBase));
    FwbHandle = NULL;
    Status =  gBS->InstallMultipleProtocolInterfaces (
                     &FwbHandle,
                     &gEfiFirmwareVolumeBlockProtocolGuid,   
                     &FvbDevice->FwVolBlockInstance,
                     &gEfiDevicePathProtocolGuid,            
                     FvbDevice->DevicePath,
                     NULL
                     );
    ASSERT_EFI_ERROR (Status);
    //DEBUG ((EFI_D_INFO, "FwBlockService.c: IMPI FirmwareVolBlockProt, DevPath 0x%lx: %r\n", FwhInstance->FvBase, Status));

    //
    // Install Alternate FVB on the same handle
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &FwbHandle,
                    &gEfiAlternateFvBlockGuid,
                    NULL,
                    NULL
                    );
    ASSERT_EFI_ERROR (Status);
  } else if (IsDevicePathEnd (FvbDevice->DevicePath)) {
    //
    // Device allready exists, so reinstall the FVB protocol
    //
    //DEBUG ((EFI_D_ERROR, "FwBlockService.c: LocateDevicePath succeeded, reinstall interface 0x%lx:\n", FwhInstance->FvBase));
    Status = gBS->HandleProtocol (
                    FwbHandle,
                    &gEfiFirmwareVolumeBlockProtocolGuid,
                    (VOID **) &OldFwbInterface
                    );
    ASSERT_EFI_ERROR (Status);

    Status =  gBS->ReinstallProtocolInterface (
                     FwbHandle,
                     &gEfiFirmwareVolumeBlockProtocolGuid,
                     OldFwbInterface,
                     &FvbDevice->FwVolBlockInstance
                     );
    ASSERT_EFI_ERROR (Status);

  } else {
    //
    // There was a FVB protocol on an End Device Path node
    //
    ASSERT (FALSE);
  }

}


/**
  The driver entry point for Firmware Volume Block Driver.

  The function does the necessary initialization work for 
  Firmware Volume Block Driver.

  @param[in]  ImageHandle       The firmware allocated handle for the UEFI image.
  @param[in]  SystemTable       A pointer to the EFI system table.

  @retval     EFI_SUCCESS       This funtion always return EFI_SUCCESS.
                                It will ASSERT on errors.

**/
EFI_STATUS
EFIAPI
DxeFvbInitialize (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_EVENT                             Event;

//[-start-151224-IB07220029-add]//
  if (PcdGetBool (PcdUseFastCrisisRecovery)) {
    if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
      return EFI_UNSUPPORTED;
    }
  }
//[-end-151224-IB07220029-add]//

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  FvbVirtualddressChangeEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  FvbInitialize (); 
  
  return EFI_SUCCESS;
}


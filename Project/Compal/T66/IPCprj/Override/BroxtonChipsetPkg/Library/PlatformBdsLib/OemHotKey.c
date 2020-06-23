/** @file
  PlatformBdsLib

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "OemHotKey.h"

/**
  Platform Oem HotKey Callback Function

  @param  Selection       HotKey Selection
  @param  Timeout         
  @param  BootMode        
  @param  NoBootDevices   

  @retval EFI_SUCCESS   

**/
EFI_STATUS
OemHotKeyCallback (
  IN UINT16                                    Selection,
  IN UINT16                                    Timeout,
  IN EFI_BOOT_MODE                             BootMode,
  IN BOOLEAN                                   NoBootDevices
  )
{
  EFI_STATUS                            Status;
  LIST_ENTRY                            BdsBootOptionList;

  Status = OemSvcHookAfterHotkeyDetect (
             Selection,
             BootMode,
             NoBootDevices
             );
  if (EFI_SUCCESS == Status) {
    return Status;
  }

  if (Selection == FRONT_PAGE_HOT_KEY && BdsLibIsWin8FastBootActive ()) {
    BdsLibConnectUsbHID ();
    BdsLibConnectAll ();
    if (BdsLibGetBootType () != LEGACY_BOOT_TYPE) {
      BdsLibRestoreBootOrderFromPhysicalBootOrder ();
    }
  }

  Status = BdsLibStartSetupUtility (TRUE);

  switch (Selection) {
  case SETUP_HOT_KEY:
    //
    // Display SetupUtility
    //
    BdsLibStartSetupUtility (FALSE);

    break;

  case DEVICE_MANAGER_HOT_KEY:
    //
    // Display the Device Manager
    //
    do {
      gCallbackKey = CallDeviceManager ();
    } while (gCallbackKey == FRONT_PAGE_KEY_DEVICE_MANAGER);
    
    break;

  case BOOT_MANAGER_HOT_KEY:
    //
    // User chose to run the Boot Manager
    //
    CallBootManager ();
//[-start-190822-IB16530054-add]//
//  if (FeaturePcdGet(PcdFrontPageSupported)) {
//PRJ+>>>>no need insyde GUI console mode
//      PlatformBdsEnterFrontPage (0xffff, TRUE);
//PRJ+<<<<no need insyde GUI console mode
//  } else {
//    BdsLibStartSetupUtility (FALSE);
//  }
//[-end-190822-IB16530054-add]//
    break;

  case BOOT_MAINTAIN_HOT_KEY:
    if (BdsLibGetBootType () != LEGACY_BOOT_TYPE) {
      //
      // Display the Boot Maintenance Manager
      //
      BdsStartBootMaint ();
    } else {
      return EFI_UNSUPPORTED;
    }
    break;

  case SECURE_BOOT_HOT_KEY:
    if (BdsLibGetBootType () != LEGACY_BOOT_TYPE) {
      //
      // Display SetupUtility
      //
      CallSecureBootMgr ();
    } else {
      return EFI_UNSUPPORTED;
    }
    break;
  case FRONT_PAGE_HOT_KEY:
    if (FeaturePcdGet(PcdFrontPageSupported)) {
      Timeout = 0xFFFF;
      InitializeListHead (&BdsBootOptionList);
      BdsLibEnumerateAllBootOption (TRUE, &BdsBootOptionList);
      PlatformBdsEnterFrontPage (Timeout, TRUE);
      break;
    }

  }
  
  return EFI_SUCCESS;
}  


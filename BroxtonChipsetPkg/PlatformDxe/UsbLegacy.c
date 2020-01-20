/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/*++

Copyright (c)  2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  UsbLegacy.c
    
Abstract:

  The Usb Legacy Protocol is used provide a standard interface to the
  UsbLegacy code for platform modifications of operation.
  
  
 

--*/
#include "PlatformDxe.h"
#include <Protocol/UsbLegacyPlatform.h>
#include <Library/DxeOemSvcChipsetLib.h>
#include "UsbLegacy.h"
#include "BoardUsbLegacy.h"

//
// Local  prototypes
//
EFI_USB_LEGACY_PLATFORM_PROTOCOL    mUsbLegacyPlatform;


EFI_STATUS
GetUsbPlatformOptions (
  IN   EFI_USB_LEGACY_PLATFORM_PROTOCOL *This,
  OUT  USB_LEGACY_MODIFIERS           *UsbLegacyModifiers
  )
/*++

  Routine Description:
    Return all platform information that can modify USB legacy operation

  Arguments:
    This                - Protocol instance pointer.
    UsbLegacyModifiers  - List of keys to monitor from. This includes both

  Returns:
    EFI_SUCCESS   - Modifiers exist.
    EFI_NOT_FOUND - Modifiers not not exist.

--*/

{
  UsbLegacyModifiers->UsbLegacyEnable           = 0x00;
  UsbLegacyModifiers->UsbZip                    = 0x00;
  UsbLegacyModifiers->UsbZipEmulation           = 0x00;
  UsbLegacyModifiers->UsbFixedDiskWpBootSector  = 0x00;
  UsbLegacyModifiers->UsbBoot                   = 0x00;
  //
  // Check SETUP for behavior modifications
  //
//[-start-160413-IB04190071-modify]//
#if defined (TABLET_PF_ENABLE) && (TABLET_PF_ENABLE == 1)
  //
  // mSystemConfiguration.UsbLegacy uses negative logic. 0 = enabled.
  //
  UsbLegacyModifiers->UsbLegacyEnable = 
       mSystemConfiguration.UsbLegacy + 1; 
#else
  //
  // mSystemConfiguration.LegacyUsbSupport = 0 : Disabled, 1:Enalbe, 2:UsbUefiOnly
  // 
  if (mSystemConfiguration.LegacyUsbSupport == 0x01) {
    UsbLegacyModifiers->UsbLegacyEnable = 0x01;
  }
  if (mSystemConfiguration.LegacyUsbSupport == 0x02) {
    UsbLegacyModifiers->UsbUefiOnly = 0x01;
  }		 
#endif  
//[-end-160413-IB04190071-modify]//

  UsbLegacyModifiers->UsbZip = 0x02;

  UsbLegacyModifiers->UsbZipEmulation = 
       mSystemConfiguration.UsbZipEmulation;

  UsbLegacyModifiers->UsbFixedDiskWpBootSector = 
       mSystemConfiguration.Fdbs +1;

//  UsbLegacyModifiers->UsbFixedDiskWpBootSector = 2;
//[-start-160302-IB07220045-modify]//
//   UsbLegacyModifiers->UsbBoot =
//        mSystemConfiguration.UsbBoot +1;
  if (mSystemConfiguration.UsbBoot == 0) {
    UsbLegacyModifiers->UsbBoot = 0x02; //DEFINE_USB_BOOT_ENABLE;
  } else if (mSystemConfiguration.UsbBoot == 1) {
    UsbLegacyModifiers->UsbBoot = 0x01; //DEFINE_USB_BOOT_DISABLED;
  }
//[-end-160302-IB07220045-modify]//
//UsbLegacyPlatform.165_001 Add Start
  /*++
  UsbLegacyModifiers->UsbEhciEnable = 
              mSystemConfiguration.UsbEhciEnable;
  --*/
         
 //-  UsbLegacyModifiers->UsbMassEmulation = mSystemConfiguration.UsbBIOSINT13DeviceEmulation;
//-  UsbLegacyModifiers->UsbMassEmulationSizeLimit = mSystemConfiguration.UsbBIOSINT13DeviceEmulationSize;

  OemSvcSetUsbLegacyPlatformOptions (UsbLegacyModifiers);

  return EFI_SUCCESS;
}

EFI_STATUS
GetPlatformMonitorKeyOptions (
  IN   EFI_USB_LEGACY_PLATFORM_PROTOCOL *This,
  OUT  KEY_ELEMENT                    **KeyList,
  OUT  UINTN                          *KeyListSize
  )
/*++

  Routine Description:
    Return all platform information that can modify USB legacy operation

  Arguments:
    This          - Protocol instance pointer.
    KeyList       - List of keys to monitor from. This includes both
                    USB & PS2 keyboard inputs.
    KeyListSize   - Size of KeyList in bytes

  Returns:
    EFI_SUCCESS   - Keys are to be monitored.
    EFI_NOT_FOUND - No keys are to be monitored.

--*/
 {
  UINT8                    Count;
  KEY_ELEMENT              EndEntry;

  Count =0;

  ZeroMem(&EndEntry, sizeof(KEY_ELEMENT));
  * KeyList = (KEY_ELEMENT *)PcdGetPtr (PcdPlatformKeyList);
  while (CompareMem (&EndEntry, &(*KeyList)[Count],sizeof(KEY_ELEMENT)) != 0) {
    Count++;
  }
  *KeyListSize = Count * sizeof(KEY_ELEMENT);

  return EFI_SUCCESS;
}
 
EFI_STATUS
UsbLegacyPlatformInstall (
  )
/*++

Routine Description:
  Install Driver to produce USB Legacy platform protocol. 

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 

  EFI_SUCCESS - USB Legacy Platform protocol installed

  Other       - No protocol installed, unload driver.

--*/
{
  EFI_STATUS                           Status;
  EFI_HANDLE                           Handle;
  //
  // Grab a copy of all the protocols we depend on. Any error would
  // be a dispatcher bug!.
  //


  mUsbLegacyPlatform.GetUsbPlatformOptions  = GetUsbPlatformOptions;
  mUsbLegacyPlatform.GetPlatformMonitorKeyOptions  = GetPlatformMonitorKeyOptions;


  //
  // Make a new handle and install the protocol
  //
  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEfiUsbLegacyPlatformProtocolGuid, 
                  EFI_NATIVE_INTERFACE,
                  &mUsbLegacyPlatform
                  );
  return Status;
}

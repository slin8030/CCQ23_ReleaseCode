/** @file
  This function provides an interface to hook after POST hotkey is detected.
;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/DxeOemSvcChipsetLib.h>

/**
 This function provides an interface to hook after POST hotkey is detected.

 @param[in]     Selection           The detected hotkey value.
 @param[in]     BootMode            Current Boot mode.
 @param[in]     NoBootDevices       TRUE : No boot device exist.
                                    FALSE: Bootable device is detected.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcHookAfterHotkeyDetect (
  IN UINT16                             Selection,
  IN EFI_BOOT_MODE                      BootMode,
  IN BOOLEAN                            NoBootDevices
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/

  return EFI_UNSUPPORTED;
}

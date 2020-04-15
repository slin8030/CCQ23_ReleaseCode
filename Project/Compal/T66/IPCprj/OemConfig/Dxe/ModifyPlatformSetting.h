/** @file
  Private function to update devices config

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

#ifndef __OEM_MODIFY_PLATFORM_SETTING_DXE_H__
#define __OEM_MODIFY_PLATFORM_SETTING_DXE_H__

/**
 Modify platform setting

 @retval EFI_SUCCESS            The function is executed successfully.
**/
EFI_STATUS
EFIAPI
ModifyPlatformSetting (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

#endif

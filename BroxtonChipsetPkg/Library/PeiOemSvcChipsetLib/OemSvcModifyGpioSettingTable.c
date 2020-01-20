/** @file
    This function offers an interface to Dynamically modify gChipsetPkgTokenSpaceGuid.PcdPeiGpioTable.

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/PeiOemSvcChipsetLib.h>
#include <Library/PcdLib.h>

/**
 This function offers an interface to Dynamically change GPIO setting procedure.
 Note:The default in memory space 

 @param[in, out]    PlatformGpioSetting_N     On entry, points to a structure that specifies the GPIO setting. 
                                              On exit, points to the updated structure. 
                                              The default value is decided in BoardGpios.c.

 @param[in, out]    PlatformGpioSetting_NW    On entry, points to a structure of that specifies the GPIO setting. 
                                              On exit, points to the updated structure. 
                                              The default value is decided in BoardGpios.c.

 @param[in, out]    PlatformGpioSetting_W     On entry, points to a structure that specifies the GPIO setting. 
                                              On exit, points to the updated structure. 
                                              The default value is decided in BoardGpios.c.
                                        
 @param[in, out]    PlatformGpioSetting_SW    On entry, points to a structure that specifies the GPIO setting. 
                                              On exit, points to the updated structure. 
                                              The default value is decided in BoardGpios.c.

 @retval            EFI_UNSUPPORTED       Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED     Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS           The function performs the same operation as caller.
                                          The caller will skip the specified behavior and assuming
                                          that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcModifyGpioSettingTable (
  IN OUT BXT_GPIO_PAD_INIT     **PlatformGpioSetting_N,
  IN OUT BXT_GPIO_PAD_INIT     **PlatformGpioSetting_NW,
  IN OUT BXT_GPIO_PAD_INIT     **PlatformGpioSetting_W,
  IN OUT BXT_GPIO_PAD_INIT     **PlatformGpioSetting_SW
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/

  return EFI_UNSUPPORTED;

  
}

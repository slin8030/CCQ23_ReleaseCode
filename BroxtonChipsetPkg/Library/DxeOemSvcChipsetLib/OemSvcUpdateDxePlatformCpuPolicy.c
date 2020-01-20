/** @file
  This function offers an interface to modify EFI_PLATFORM_CPU_PROTOCOL Data before the system
  installs EFI_PLATFORM_CPU_PROTOCOL.
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
 This function offers an interface to modify EFI_PLATFORM_CPU_PROTOCOL Data before the system
 installs EFI_PLATFORM_CPU_PROTOCOL.

 @param[in, out]    *PlatformCpuPolicy  On entry, points to EFI_PLATFORM_CPU_PROTOCOL structure.
                                        On exit, points to updated EFI_PLATFORM_CPU_PROTOCOL structure.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
//EFI_STATUS
//OemSvcUpdateDxePlatformCpuPolicy (
//  IN OUT EFI_PLATFORM_CPU_PROTOCOL   *PlatformCpuPolicy
//  )
//{
//  /*++
//    Todo:
//      Add project specific code in here.
//  --*/
// 
//  return EFI_UNSUPPORTED;
//}

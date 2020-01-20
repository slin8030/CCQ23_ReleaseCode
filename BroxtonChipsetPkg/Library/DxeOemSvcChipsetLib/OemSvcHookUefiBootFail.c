/** @file
  This function provides an interface to hook when boot to UEFI boot option failed.
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
#include <Library/GenericBdsLib.h>

/**
 This function provides an interface to hook when boot to UEFI boot option failed..

 @param[in]     *InputOption        Content of BDS_COMMON_OPTION
 @param[in]     Status              Return status from BdsLibBootViaBootOption function
 @param[in]     *ExitData           The string data return from boot image
 @param[in]     ExitDataSize        Size of ExitData
 
 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcHookUefiBootFail (
  IN  VOID                *InputOption,
  IN  EFI_STATUS          Status,
  IN  CHAR16              *ExitData,
  IN  UINTN               ExitDataSize
  )
{
  BDS_COMMON_OPTION       *Option;

  Option = (BDS_COMMON_OPTION*)InputOption;
  /*++
    Todo:
      Add project specific code in here.
  --*/

  return EFI_UNSUPPORTED;
}

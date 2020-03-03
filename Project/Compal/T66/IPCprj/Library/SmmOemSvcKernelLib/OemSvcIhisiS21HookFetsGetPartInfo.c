/** @file
 This function offers an interface to Hook IHISI Sub function AH=21h,function "FetsGetPartInfo
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

#include <Library/SmmOemSvcKernelLib.h>

/**
 This function offers an interface to Hook IHISI Sub function AH=21h,function "FetsGetPartInfo"

 @param[in, out]    IhisiStatus         On entry, pointer to IhisiStatus Address.
                                        On exit, points to updated IhisiStatus Address.
 @param[in, out]    EcPartSize          On entry, pointer to EcPartSize Address.
                                        On exit, points to updated EcPartSize Address.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcIhisiS21HookFetsGetPartInfo (
  IN OUT UINT32     *IhisiStatus,
  IN OUT UINT32     *EcPartSize
  )
{
/*++
  Todo:
  Add project specific code in here.
--*/
  *EcPartSize = EC128K;
  IhisiStatus = IHISI_SUCCESS;

  return EFI_MEDIA_CHANGED;
}

/** @file
 This function offers an interface to Hook IHISI Sub function AH=20h,function "FetsDoAfterFlashing"
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

#include <Library/SmmOemSvcChipsetLib.h>

/**
 This function provides a hook for IHISI sub-function AH=20h, function "FetsDoAfterFlashing". 
 This function is called after flashing EC part. Please refer to Insyde H2O internal Soft-SMI 
 interface (IHISI) Specification.
 
 @param[in]         ActionAfterFlashing Input action flag.
 
 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcIhisiS20HookFetsDoAfterFlashing (
  IN UINT8          ActionAfterFlashing
  )
{
/*++
  Todo:
  Add project specific code in here.
--*/

  return EFI_UNSUPPORTED;
}

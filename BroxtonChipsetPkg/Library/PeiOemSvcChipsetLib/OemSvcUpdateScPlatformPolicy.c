/** @file
  
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



/**
 This function offers an interface to modify SC_POLICY_PPI data before the system 
 installs SC_POLICY_PPI.

 @param[in, out]    *ScPolicyPpi                  On entry, points to SC_POLICY_PPI structure.
                                                  On exit, points to updated SC_POLICY_PPI structure.

 @retval            EFI_UNSUPPORTED               Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED             Alter the Configuration Parameter.
 @retval            EFI_SUCCESS                   The function performs the same operation as caller.
                                                  The caller will skip the specified behavior and assuming
                                                  that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcUpdateScPlatformPolicy (
  IN OUT SC_POLICY_PPI                         *ScPolicyPpi
  )
{

  return EFI_UNSUPPORTED;
}
//[-end-140909-IB10820533-modify]//
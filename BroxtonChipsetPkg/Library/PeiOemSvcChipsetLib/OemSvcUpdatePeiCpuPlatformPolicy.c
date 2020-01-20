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
  This function offers an interface for OEM code to change the SI_CPU_POLICY_PPI.

 @param[in]        **PerServices                  General purpose services available to every PEIM.
 @param[in, out]   *SiCpuPolicyPpi                On entry, points to SI_CPU_POLICY_PPI structure.
                                                  On exit, points to updated SI_CPU_POLICY_PPI structure.

 @retval            EFI_UNSUPPORTED               Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED             Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS                   The function performs the same operation as caller.

**/
EFI_STATUS
OemSvcUpdatePeiCpuPlatformPolicy (
  IN CONST    EFI_PEI_SERVICES     **PeiServices,
  IN OUT      SI_CPU_POLICY_PPI    *SiCpuPolicyPpi
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/

  return EFI_UNSUPPORTED;
} 
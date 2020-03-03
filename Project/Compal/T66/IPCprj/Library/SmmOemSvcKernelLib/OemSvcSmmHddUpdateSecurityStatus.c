/** @file
  Provide OEM to execute specific process, after update HDD security status.

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
  Provide OEM to execute specific process, after update HDD security status.

  @param[in]        HddPasswordService  EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.
  @param[in]        HddInfo             The HDD information used in HDD Password.
  @param[in]        Index               Use to know which index.

  @retval           EFI_UNSUPPORTED     Returns unsupported by default.
  @retval           EFI_SUCCESS         The service is customized in the project.
  @retval           EFI_MEDIA_CHANGED   The value of IN OUT parameter is changed.
  @retval           Others              Base on OEM design.
**/
EFI_STATUS
OemSvcSmmHddUpdateSecurityStatus (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *HddPasswordService,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo,
  IN  UINTN                             Index
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/

  return EFI_UNSUPPORTED;
}

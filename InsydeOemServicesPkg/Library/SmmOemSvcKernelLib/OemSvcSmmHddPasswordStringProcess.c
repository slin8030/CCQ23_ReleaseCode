/** @file
  Provide OEM to execute specific process, before user inputs the password.

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
  Provide OEM to execute specific string process, after user inputs the password.

  @param[in]        RawPasswordPtr      Password string address.
  @param[in]        RawPasswordLength   Password string length.
  @param[OUT]       PasswordToHdd       Encode password address.
  @param[OUT]       PasswordToHddLength Encode string length.

  @retval           EFI_UNSUPPORTED     Returns unsupported by default.
  @retval           EFI_SUCCESS         The service is customized in the project.
  @retval           EFI_MEDIA_CHANGED   The value of IN OUT parameter is changed.
  @retval           Others              Base on OEM design.
**/
EFI_STATUS
OemSvcSmmHddPasswordStringProcess (
  IN UINT8                                PasswordType,
  IN   VOID                               *RawPasswordPtr,
  IN   UINTN                              RawPasswordLength,
  OUT  UINT8                              **PasswordPtr,
  OUT  UINTN                              *PasswordLength
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/

  return EFI_UNSUPPORTED;
}

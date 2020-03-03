/** @file
  Provide OEM to execute specific process, unlock all of the HDD password when S3/S4 resume.

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

#include <Guid/HddPasswordVariable.h>

/**
  Provide OEM to execute specific process, unlock all of the HDD password when S3/S4 resume.

  @param[in]        HddPasswordService  EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.
  @param[in]        HddInfoArray        The array of HDD information used in HDD Password.
  @param[in]        HddPasswordTable    User or Master flag.
  @param[in]        NumOfHdd            Number of HDD

  @retval           EFI_UNSUPPORTED     Returns unsupported by default.
  @retval           EFI_SUCCESS         The service is customized in the project.
  @retval           EFI_MEDIA_CHANGED   The value of IN OUT parameter is changed.
  @retval           Others              Base on OEM design.
**/
EFI_STATUS
OemSvcSmmHddPasswordUnlockAllHdd (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *HddPasswordServiceProtocol,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfoArray,
  IN  UINTN                              NumOfHdd,
  IN  VOID                               *HddPasswordTablePtr,
  IN  UINTN                              HddPasswordTableSize
  )
{
  HDD_PASSWORD_TABLE                     *HddPasswordTable;

  HddPasswordTable = (HDD_PASSWORD_TABLE *)HddPasswordTablePtr;

  /*++
    Todo:
      Add project specific code in here.
  --*/

  return EFI_UNSUPPORTED;
}

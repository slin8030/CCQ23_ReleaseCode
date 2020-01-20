/** @file
  Provide OEM to execute specific process, reset specific device with password.

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

#include <Library/DxeOemSvcKernelLib.h>

#include <Library/BaseMemoryLib.h>
#include <Guid/HddPasswordVariable.h>

/**
  Provide OEM to execute specific process, reset specific device with password.

  @param[in]        HddPasswordService  EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.
  @param[in]        HddInfo             The HDD information used in HDD Password.
  @param[in]        HddPasswordTablePtr The Pointer to Hdd Password Table.
  @param[in]        HddPasswordTableSize The size of Hdd Password Table.

  @retval           EFI_UNSUPPORTED     Returns unsupported by default.
  @retval           EFI_SUCCESS         The service is customized in the project.
  @retval           EFI_MEDIA_CHANGED   The value of IN OUT parameter is changed.
  @retval           Others              Base on OEM design.
**/
EFI_STATUS
OemSvcDxeHddResetSecurityStatus (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL  *HddPasswordService,
  IN  HDD_PASSWORD_HDD_INFO              *HddInfo,
  IN  VOID                               *HddPasswordTablePtr,
  IN  UINTN                              HddPasswordTableSize
  )
{
  HDD_PASSWORD_TABLE                     *HddPasswordTable;

//  EFI_STATUS                             Status;
//  UINT8                                  PasswordToHdd[HDD_PASSWORD_MAX_NUMBER + 1];
//  UINTN                                  PasswordToHddLength;
//  UINTN                                  HddPasswordTableIndex;
//  UINTN                                  HddPasswordTableUnitSize;
//  HDD_PASSWORD_TABLE                     *HddPasswordTableUnitPtr;

  HddPasswordTable = (HDD_PASSWORD_TABLE *)HddPasswordTablePtr;

  /*++
    Todo:
      Add project specific code in here.
  --*/

//  if (HddPasswordTable != NULL) {
//    ZeroMem (&PasswordToHdd, (HDD_PASSWORD_MAX_NUMBER + 1));
//
//    PasswordToHddLength = HddInfo->MaxPasswordLengthSupport;
//
//    HddPasswordTableIndex = 0;
//    HddPasswordTableUnitPtr = HddPasswordTable;
//    HddPasswordTableUnitSize = sizeof (HDD_PASSWORD_TABLE) + HddPasswordTable->ExtDataSize;
//    while((HddPasswordTableUnitPtr->ControllerNumber != HddInfo->ControllerNumber &&
//          HddPasswordTableUnitPtr->PortNumber != HddInfo->PortNumber &&
//          HddPasswordTableUnitPtr->PortMulNumber != HddInfo->PortMulNumber) &&
//          HddPasswordTableIndex < (HddPasswordTableSize / HddPasswordTableUnitSize)) {
//      HddPasswordTableIndex++;
//      HddPasswordTableUnitPtr = HddPasswordTable + HddPasswordTableIndex * HddPasswordTableUnitSize;
//    }
//
//    if (HddPasswordTableIndex != (HddPasswordTableSize / HddPasswordTableUnitSize)) {
//
//      Status = HddPasswordService->PasswordStringProcess (
//                                     HddPasswordService,
//                                     HddPasswordTableUnitPtr->PasswordType,
//                                     HddPasswordTableUnitPtr->PasswordStr,
//                                     HddInfo->MaxPasswordLengthSupport,
//                                     (VOID **)&PasswordToHdd,
//                                     &PasswordToHddLength
//                                     );
//
//      Status = HddPasswordService->SendHddSecurityCmd (
//                                     HddPasswordService,
//                                     H2O_HDD_PASSWORD_CMD_COMRESET,
//                                     HddInfo,
//                                     HddPasswordTableUnitPtr->PasswordType,
//                                     PasswordToHdd,
//                                     PasswordToHddLength
//                                     );
//    }
//  }
//
//  return EFI_SUCCESS;
  return EFI_UNSUPPORTED;
}

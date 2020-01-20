/** @file
  Header file for Password

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SETUP_PASSWORD_H_
#define _SETUP_PASSWORD_H_

#include "SetupUtilityLibCommon.h"
#include <Uefi/UefiSpec.h>
#include <Library/PrintLib.h>
#include <Library/HobLib.h>

#include <Protocol/SetupMouse.h>
#include <Protocol/CpuIo.h>
#include <Guid/HobList.h>
#include <Protocol/HddPasswordService.h>
#include <Protocol/SysPasswordService.h>
#include <Guid/HddPasswordVariable.h>
#include <Library/PcdLib.h>

#define MIN_PASSWORD_LENGTH       1
#define SETUP_PASSWORD            0x01
#define BROWSER_STATE_VALIDATE_PASSWORD  0
#define BROWSER_STATE_SET_PASSWORD       1

#define bit(a)   (1 << (a))
#define HDD_SECURITY_SUPPORT bit(0)
#define HDD_SECURITY_ENABLE  bit(1)
#define HDD_SECURITY_LOCK    bit(2)
#define HDD_SECURITY_FROZEN  bit(3)
#define HDD_SECURITY_EXPIRED bit(4)
#define ATAPI_DEVICE_BIT     bit(15)

#define INIT_ITEM_MAPPING(item,value) (item==0?(item=value):(value=value))


//
// The following functions are internal use in security page
//
EFI_STATUS
UpdateSupervisorTags (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  BOOLEAN                           Installed,
  IN  CHAR16                            *NewString,
  IN  KERNEL_CONFIGURATION              *KernelConfig
  );

EFI_STATUS
UpdateUserTags (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  BOOLEAN                           Installed,
  IN  CHAR16                            *NewString,
  IN  KERNEL_CONFIGURATION              *KernelConfig
  );

EFI_STATUS
AllHddPasswordCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *FormCallback,
  IN  BOOLEAN                                UserOrMaster,
  IN  EFI_HII_HANDLE                         HiiHandle,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest,
  OUT BOOLEAN                                *PState,
  IN  HDD_PASSWORD_SCU_DATA                  *HddPasswordScuData,
  IN  UINTN                                  NumOfHdd
  );


UINTN
SysPasswordMaxNumber (
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL    *SysPasswordService  OPTIONAL
  );

#endif

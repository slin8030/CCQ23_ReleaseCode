/** @file
  Header file for Security menu

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SECURITY_CALLBACK_H_
#define _SECURITY_CALLBACK_H_

#include "SetupUtilityLibCommon.h"
#include "Password.h"

EFI_STATUS
SecurityCallbackRoutineByAction (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  );

EFI_STATUS
EFIAPI
InitStoragePasswordForSCU (
  IN  EFI_HII_HANDLE                         HiiHandle
  );

extern EFI_CALLBACK_INFO                     *mSecurityCallBackInfo;
#endif

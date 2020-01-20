/** @file

 Initial and callback functions for Advance page of SetupUtilityDxe Driver

;******************************************************************************
;* Copyright (c) 2012-2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#ifndef _ADVANCE_CALLBACK_H
#define _ADVANCE_CALLBACK_H

#include "SetupUtility.h"

EFI_STATUS
AdvanceCallbackRoutine (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  );

EFI_STATUS
AdvanceCallbackRoutineByAction (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  );

EFI_STATUS
InstallAdvanceCallbackRoutine (
  IN EFI_HANDLE                             DriverHandle,
  IN EFI_HII_HANDLE                         HiiHandle
  );

EFI_STATUS
UninstallAdvanceCallbackRoutine (
  IN EFI_HANDLE                             DriverHandle
  );

EFI_STATUS
InitAdvanceMenu (
  IN EFI_HII_HANDLE                         HiiHandle
  );

#if ENBDT_PF_ENABLE      
extern  IDE_CONFIG                      *mIdeConfig;
//[-start-151123-IB11270134-add]//
 extern ADVANCE_CONFIGURATION            mAdvConfig;       
//[-end-151123-IB11270134-add]//
EFI_STATUS
UpdateHDCConfigure (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  CHIPSET_CONFIGURATION              *Buffer
  );
EFI_STATUS  
InitIdeConfig (
  IDE_CONFIG                             *IdeConfig
  );
#endif
  
#endif

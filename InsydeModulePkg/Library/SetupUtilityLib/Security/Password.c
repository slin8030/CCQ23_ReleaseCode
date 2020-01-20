/** @file
  Password relaitve code, including HDD password and System password

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

#include "Password.h"
#include "Security.h"
#include <Protocol/DiskInfo.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <Library/OemGraphicsLib.h>
#include <Library/DxeChipsetSvcLib.h>

PASSWORD_CONFIGURATION             mPasswordConfig;

STATIC
BOOLEAN
CheckAnyHddLocked (
  IN  HDD_PASSWORD_SCU_DATA     *HddPasswordScuData,
  IN  UINTN                      NumOfHdd
  );

STATIC
EFI_STATUS
CheckSetupPassword (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  EFI_SYS_PASSWORD_SERVICE_PROTOCOL *SysPasswordService,
  IN  SETUP_UTILITY_CONFIGURATION       *ScuInfo,
  IN  KERNEL_CONFIGURATION              *KernelConfig
  );


BOOLEAN
HaveSysPasswordCheck (
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL     *SysPasswordService  OPTIONAL
  );

EFI_STATUS
CleanSysPasswordCheck (
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL     *SysPasswordService  OPTIONAL
  );

CURRENT_USER_TYPE
GetSysPasswordCurrentUser (
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL     *SysPasswordService  OPTIONAL
  );

EFI_STATUS
SetSysPasswordCurrentUser (
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL     *SysPasswordService, OPTIONAL
  CURRENT_USER_TYPE                     CurrentUserType
  );


EFI_STATUS
UpdateAllHddPasswordFlag (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  HDD_PASSWORD_SCU_DATA             *HddPasswordScuData,
  IN  UINTN                             NumOfHdd
  )
{
  UINTN                                 Index;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;
  EFI_STATUS                            Status;


  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (HddPasswordScuData == NULL) {
    mPasswordConfig.SetAllHddPasswordFlag = 0;
    return EFI_SUCCESS;
  }

  if (NumOfHdd == 0) {
    mPasswordConfig.SetAllHddPasswordFlag = 0;
    return EFI_SUCCESS;
  }


  if (CheckAnyHddLocked (HddPasswordScuData, NumOfHdd)) {
    mPasswordConfig.SetAllHddPasswordFlag = 0;
  } else {
    mPasswordConfig.SetAllHddPasswordFlag = 1;
    mPasswordConfig.SetAllMasterHddPasswordFlag = 1;
    for (Index = 0; Index < NumOfHdd; Index++) {
      if (HddPasswordScuData[Index].Flag == CHANGE_PASSWORD ||
          HddPasswordScuData[Index].Flag == ENABLE_PASSWORD) {
        mPasswordConfig.SetAllHddPasswordFlag = 0;
        mPasswordConfig.SetAllMasterHddPasswordFlag = 0;
        break;
      }
    }
  }

  return EFI_SUCCESS;
}


EFI_STATUS
ValidateAllHddPassword (
  IN  BOOLEAN                           UserOrMaster,
  IN  HDD_PASSWORD_SCU_DATA             *HddPasswordScuData,
  IN  UINTN                             NumOfHdd
  )
{

  //
  //  Always return EFI_SUCCESS to display two password items for new password and confirm new password.
  //
  return EFI_SUCCESS;
}

EFI_STATUS
SetAllHddPassword (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  HDD_PASSWORD_SCU_DATA             *HddPasswordScuData,
  IN  BOOLEAN                           UserOrMaster,
  IN  UINTN                             NumOfHdd,
  IN  CHAR16                            *Password
  )
{
  UINTN                              Index;
  UINTN                              SetSecurityFlag;
  SETUP_UTILITY_BROWSER_DATA         *SuBrowser;
  EFI_STATUS                         Status;

  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    //
    //  Return EFI_NOT_AVAILABLE_YET or EFI_UNSUPPORTED to let browser terminate password flow.
    //
    return EFI_NOT_AVAILABLE_YET;
  }

  SetSecurityFlag = DISABLE_PASSWORD;
  mPasswordConfig.SetAllMasterHddPasswordFlag = FALSE;
  if ((Password[0] == 0)) {
    SetSecurityFlag = DISABLE_PASSWORD;
    mPasswordConfig.SetAllMasterHddPasswordFlag = FALSE;
  } else {
    SetSecurityFlag = CHANGE_PASSWORD;
  }

  if (UserOrMaster == USER_PSW) {
    for (Index = 0; Index < NumOfHdd; Index++) {
      HddPasswordScuData[Index].Flag = SetSecurityFlag;

      ZeroMem (
        HddPasswordScuData[Index].InputString,
        (HddPasswordScuData[Index].StringLength * sizeof (CHAR16))
        );

      if ((Password[0] == 0)) {
        HddPasswordScuData[Index].MasterFlag = SetSecurityFlag;

        HddPasswordScuData[Index].StringLength = 0;
      } else {
        mPasswordConfig.SetAllMasterHddPasswordFlag = FALSE;
        HddPasswordScuData[Index].MasterFlag = MASTER_PASSWORD_GRAYOUT;

        HddPasswordScuData[Index].StringLength = StrLen (Password);

        StrCpy (
          HddPasswordScuData[Index].InputString,
          Password
          );
      }


    }
  } else {
    for (Index = 0; Index < NumOfHdd; Index++) {
      HddPasswordScuData[Index].MasterFlag = SetSecurityFlag;

      ZeroMem (
        HddPasswordScuData[Index].MasterInputString,
        (HddPasswordScuData[Index].StringLength * sizeof (CHAR16))
        );
      if ((Password[0] == 0)) {
        HddPasswordScuData[Index].Flag = SetSecurityFlag;

        HddPasswordScuData[Index].StringLength = 0;
      } else {
        HddPasswordScuData[Index].StringLength = StrLen (Password);

        StrCpy (
          HddPasswordScuData[Index].MasterInputString,
          Password
          );
      }

    }
  }

  UpdateAllHddPasswordFlag (
    HiiHandle,
    HddPasswordScuData,
    NumOfHdd
    );

  //
  //  Return EFI_SUCCESS if password was accepted and saved.
  //
  return EFI_SUCCESS;
}

/**
  Set password to all harddisk

  @param  FormCallback           The form call back protocol.
  @param  MasterOrUser           HDD password or master HDD password.
  @param  HiiHandle              Return string token of device status.
  @param  Type                   Return string token of device status.
  @param  Value
  @param  ActionRequest
  @param  PState                 Password access is success or not, if access success then return TRUE.
  @param  HddPasswordDataInfo    Pointer to the HDD password buffer
  @param  NumOfHddPswd           Number of Harddisk

  @retval EFI_SUCCESS            Password callback success.
  @retval EFI_ALREADY_STARTED    Already password in SetupUtility.
  @retval EFI_NOT_READY          Password confirm error.

**/
EFI_STATUS
AllHddPasswordCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL    *FormCallback,
  IN  BOOLEAN                                 UserOrMaster,
  IN  EFI_HII_HANDLE                          HiiHandle,
  IN  UINT8                                   Type,
  IN  EFI_IFR_TYPE_VALUE                      *Value,
  OUT EFI_BROWSER_ACTION_REQUEST              *ActionRequest,
  OUT BOOLEAN                                 *PState,
  IN  HDD_PASSWORD_SCU_DATA                   *HddPasswordScuData,
  IN  UINTN                                   NumOfHdd
  )
{

  STATIC UINTN             PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
  EFI_STATUS               Status;
  CHAR16                   *Password;

  *PState = FALSE;
  if (Type != EFI_IFR_TYPE_STRING) {
    return  EFI_INVALID_PARAMETER;
  }

  if (Value->string == 0) {
    //
    // If Value->String == 0, only need reset the state machine to BROWSER_STATE_VALIDATE_PASSWORD
    //
    PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
    return EFI_SUCCESS;
  }


  Password = HiiGetString (HiiHandle, Value->string, NULL);
  //
  // When try to set a new password, user will be chanlleged with old password.
  // The Callback is responsible for validating old password input by user,
  // If Callback return EFI_SUCCESS, it indicates validation pass.
  //
  switch (PasswordState) {

  case BROWSER_STATE_VALIDATE_PASSWORD:
    Status = ValidateAllHddPassword (
               UserOrMaster,
               HddPasswordScuData,
               NumOfHdd
               );
    if (Status == EFI_SUCCESS) {
      PasswordState = BROWSER_STATE_SET_PASSWORD;
    }
    break;

  case BROWSER_STATE_SET_PASSWORD:
    Status = SetAllHddPassword (
               HiiHandle,
               HddPasswordScuData,
               UserOrMaster,
               NumOfHdd,
               Password
               );
    *PState = TRUE;
    PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
    break;

  default:
    Status = EFI_NOT_AVAILABLE_YET;
    break;
  }

  if (Password != NULL) {
    gBS->FreePool (Password);
  }

  return Status;
}

/**
  Check system pasword status.

  @param  ScuInfo                Point to SETUP_UTILITY_CONFIGURATION instance.
  @param  KernelConfig           The kernel configuration data.

  @retval EFI_SUCCESS            Check system password success.
  @retval EFI_NOT_FOUND          Not found system password.

**/
EFI_STATUS
PasswordCheck (
  IN  SETUP_UTILITY_CONFIGURATION       *ScuInfo,
  IN  KERNEL_CONFIGURATION              *KernelConfig
  )
{
  EFI_STATUS                            Status;
  EFI_HII_HANDLE                        HiiHandle;
  CHAR16                                *NewString;
  UINTN                                 NumOfSupervisorPswdInfo;
  UINTN                                 NumOfUserPswdInfo;
  SYS_PASSWORD_INFO_DATA                *SysPasswordInfoData;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;
  EFI_BOOT_MODE                         BootMode;


  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  HiiHandle               = ScuInfo->MapTable[SecurityHiiHandle].HiiHandle;
  NumOfSupervisorPswdInfo = 0;
  NumOfUserPswdInfo       = 0;

  if (ScuInfo->SysPasswordService == NULL) {
    return EFI_UNSUPPORTED;
  }
  if (mSecurityCallBackInfo == NULL) {
    //
    // When not in SCU, skip password check if boot mode is S4.
    //
    BootMode = GetBootModeHob ();
    if (EFI_ERROR (Status)) {
      return EFI_NOT_FOUND;
    }
    if (BootMode == BOOT_ON_S4_RESUME) {
      return EFI_SUCCESS;
    }
  }

  SysPasswordInfoData = NULL;
  //
  // Get Supervisor password
  //
  Status = ScuInfo->SysPasswordService->GetStatus (
                                          ScuInfo->SysPasswordService,
                                          SystemSupervisor
                                          );
  if (!EFI_ERROR (Status)) {
    NumOfSupervisorPswdInfo = 1;
  }

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (SYS_PASSWORD_INFO_DATA),
                  (VOID **) &SysPasswordInfoData
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  SysPasswordInfoData->Flag           = NO_ACCESS_PASSWORD;
  SysPasswordInfoData->StringLength   = 0x00;
  SysPasswordInfoData->NumOfEntry     = NumOfSupervisorPswdInfo;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  ((SysPasswordMaxNumber (ScuInfo->SysPasswordService) + 1) * sizeof (CHAR16)),
                  (VOID **)&(SysPasswordInfoData->InputString)
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

   ZeroMem (
     SysPasswordInfoData->InputString,
     ((SysPasswordMaxNumber (ScuInfo->SysPasswordService) + 1) * sizeof (CHAR16))
     );

  ScuInfo->SupervisorPassword = SysPasswordInfoData;

  //
  // Get User password
  //
  if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
    Status = ScuInfo->SysPasswordService->GetStatus (
                                            ScuInfo->SysPasswordService,
                                            SystemUser
                                            );
    SysPasswordInfoData = NULL;

    if (!EFI_ERROR (Status)) {
      NumOfUserPswdInfo = 1;
    }


    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    sizeof (SYS_PASSWORD_INFO_DATA),
                    (VOID **)&SysPasswordInfoData
                    );

    if (EFI_ERROR (Status)) {
      gBS->FreePool (ScuInfo->SupervisorPassword);
      return Status;
    }

    SysPasswordInfoData->Flag           = NO_ACCESS_PASSWORD;
    SysPasswordInfoData->StringLength   = 0x00;
    SysPasswordInfoData->NumOfEntry     = NumOfUserPswdInfo;

    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    ((SysPasswordMaxNumber (ScuInfo->SysPasswordService) + 1) * sizeof (CHAR16)),
                    (VOID **)&(SysPasswordInfoData->InputString)
                    );

    if (EFI_ERROR (Status)) {
      return Status;
    }

     ZeroMem (
       SysPasswordInfoData->InputString,
       ((SysPasswordMaxNumber (ScuInfo->SysPasswordService) + 1) * sizeof (CHAR16))
       );

    ScuInfo->UserPassword = SysPasswordInfoData;

    if ((NumOfUserPswdInfo != 0) || (NumOfSupervisorPswdInfo != 0)) {
      //
      // OemServices
      //
      Status = OemSvcPasswordEntryCallBack ();
    }
  } else {
    if (NumOfSupervisorPswdInfo != 0) {
      //
      // OemServices
      //
      Status = OemSvcPasswordEntryCallBack ();
    }
  }

  if (ScuInfo->SupervisorPassword->NumOfEntry == 0) {
    //
    // No set Supervisor password
    //
    NewString = HiiGetString (ScuInfo->MapTable[SecurityHiiHandle].HiiHandle, STRING_TOKEN (STR_NOT_INSTALLED_TEXT), NULL);
    UpdateSupervisorTags (
      HiiHandle,
      FALSE,
      NewString,
      KernelConfig
      );
    ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->SupervisorFlag = 0;
    ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->UserFlag = 0;
    SuBrowser->SUCInfo->SupervisorPwdFlag = FALSE;
    SuBrowser->SUCInfo->UserPwdFlag       = FALSE;
    gBS->FreePool (NewString);
    Status = ScuInfo->SysPasswordService->UnlockPassword (ScuInfo->SysPasswordService, NULL, 0);
    return EFI_SUCCESS;
  }

  if (HaveSysPasswordCheck (ScuInfo->SysPasswordService)) {
    DisableQuietBoot ();
    gST->ConOut->EnableCursor (gST->ConOut, TRUE);
    Status = CheckSetupPassword (
               ScuInfo->MapTable[SecurityHiiHandle].HiiHandle,
               ScuInfo->SysPasswordService,
               ScuInfo,
               KernelConfig
               );
    CleanSysPasswordCheck (ScuInfo->SysPasswordService);
  } else {
    if (GetSysPasswordCurrentUser (ScuInfo->SysPasswordService) == SystemSupervisorType) {
      KernelConfig->SetUserPass = FALSE;
    } else {
      KernelConfig->SetUserPass = TRUE;
    }
  }

  NewString = HiiGetString (ScuInfo->MapTable[SecurityHiiHandle].HiiHandle, STRING_TOKEN (STR_INSTALLED_TEXT), NULL);
  UpdateSupervisorTags (
    HiiHandle,
    TRUE,
    NewString,
    KernelConfig
    );
  ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->SupervisorFlag = 1;
  SuBrowser->SUCInfo->SupervisorPwdFlag = TRUE;

  if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
    if (ScuInfo->UserPassword->NumOfEntry == 0) {
      ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->UserFlag = 0;
      SuBrowser->SUCInfo->UserPwdFlag = FALSE;
    } else {
      ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->UserFlag = 1;
      SuBrowser->SUCInfo->UserPwdFlag = TRUE;
    }

    if (ScuInfo->UserPassword->NumOfEntry != 0) {
      UpdateUserTags (
        HiiHandle,
        TRUE,
        NewString,
        KernelConfig
        );
    }
  }
  gBS->FreePool (NewString);

  return EFI_SUCCESS;
}

/**
  Update supervisor tag.

  @param  HiiHandle              Return string token of device status.
  @param  Installed              TRUE installed supervisor password.
  @param  NewString              String buffer.
  @param  KernelConfig           The kernel configuration data.

  @retval EFI_SUCCESS            Always return.

**/
EFI_STATUS
UpdateSupervisorTags (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  BOOLEAN                           Installed,
  IN  CHAR16                            *NewString,
  IN  KERNEL_CONFIGURATION              *KernelConfig
  )
{
  STRING_REF                            TokenToUpdate;

  if (Installed) {
    KernelConfig->SupervisorFlag = 1;
  } else {
    KernelConfig->SupervisorFlag = 0;
    if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
      UpdateUserTags (HiiHandle, FALSE, NewString, KernelConfig);
    }
  }

  TokenToUpdate = STRING_TOKEN (STR_SUPERVISOR_PASSWORD_STRING2);
  HiiSetString (HiiHandle, TokenToUpdate, NewString, NULL);

  return EFI_SUCCESS;
}

/**
  Update user password tag.

  @param  HiiHandle              Return string token of device status.
  @param  Installed              TRUE installed supervisor password.
  @param  NewString              String buffer.
  @param  KernelConfig           The kernel configuration data.

  @retval EFI_SUCCESS            Always return.

**/
EFI_STATUS
UpdateUserTags (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  BOOLEAN                           Installed,
  IN  CHAR16                            *NewString,
  IN  KERNEL_CONFIGURATION              *KernelConfig
  )
{
  STRING_REF                            TokenToUpdate;

  if (Installed) {
    KernelConfig->UserFlag = TRUE;
  } else {
    KernelConfig->UserFlag = FALSE;
  }

  TokenToUpdate = STRING_TOKEN (STR_USER_PASSWORD_STRING2);
  HiiSetString (HiiHandle, TokenToUpdate, NewString, NULL);

  return EFI_SUCCESS;
}

/**
  Check system pasword status.

  @param  HiiHandle              Return string token of device status.
  @param  SysPasswordService     System Password protocol service.
  @param  ScuInfo
  @param  SetupVariable          The Setup NV data.

  @retval EFI_SUCCESS            Check system password success.
  @retval EFI_NOT_FOUND          Not found system password.

**/
STATIC
EFI_STATUS
CheckSetupPassword (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  EFI_SYS_PASSWORD_SERVICE_PROTOCOL *SysPasswordService,
  IN  SETUP_UTILITY_CONFIGURATION       *ScuInfo,
  IN  KERNEL_CONFIGURATION              *KernelConfig
  )
{
  EFI_STATUS                            Status;
  EFI_INPUT_KEY                         Key;
  CHAR16                                *PasswordString;
  CHAR16                                *StringPtr;
  CHAR16                                *ErrorStatusMsg;
  CHAR16                                *ThreeIncorrectMsg;
  CHAR16                                *RebootMsg;
  UINTN                                 TryPasswordTimes;
  BOOLEAN                               DrawFlag;
  BOOLEAN                               ReadChar;
  UINT16                                Password;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;
  CHAR16                                *SysInvalidPassword;
  CURRENT_USER_TYPE                     CurrentUserType;

  if (SysPasswordService == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (ScuInfo->SupervisorPassword->NumOfEntry == 0) {
    return EFI_SUCCESS;
  }

  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }


  Password = 0;
  DrawFlag = FALSE;
  ReadChar = TRUE;
  TryPasswordTimes = (UINTN)PcdGet8 (PcdMaxCheckPasswordCount);
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
				  ((SysPasswordMaxNumber (SysPasswordService) + 1) * sizeof (CHAR16)),
                  (VOID **)&StringPtr
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // OemServices
  //
  Status = OemSvcGetMaxCheckPasswordCount (
             &TryPasswordTimes
             );

  PasswordString = HiiGetString (HiiHandle, STRING_TOKEN (STR_CHECK_PASSWORD_STRING), NULL);

  SysInvalidPassword = HiiGetString (HiiHandle, STRING_TOKEN (STR_SYS_DIALOG_INVALID_PASSWORD), NULL);
  //
  // Clear screen before print Harddisk passwork prompt message
  //
  gST->ConOut->ClearScreen (gST->ConOut);
  do {
    ZeroMem (StringPtr, (SysPasswordMaxNumber (SysPasswordService) + 1) * sizeof (CHAR16));
    SuBrowser->H2ODialog->PasswordDialog (
                              0,
                              FALSE,
                              (UINT32) (SysPasswordMaxNumber (SysPasswordService) + 1),
                              StringPtr,
                              &Key,
                              PasswordString
                              );

    Status = SysPasswordService->GetStatus (
                                   SysPasswordService,
                                   SystemSupervisor
                                   );
    if (Status == EFI_SUCCESS) {
      //
      // Check Supervisor password
      //
      Status = SysPasswordService->CheckPassword (
                                     SysPasswordService,
                                     StringPtr,
                                     (SysPasswordMaxNumber (SysPasswordService) + 1) * sizeof(CHAR16),
                                     SystemSupervisor
                                     );

      if (Status == EFI_SUCCESS) {
        KernelConfig->SetUserPass = FALSE;
        CurrentUserType = SystemSupervisorType;
        gBS->FreePool (PasswordString);
        gBS->FreePool (SysInvalidPassword);
        Status = SysPasswordService->UnlockPassword (
                                       SysPasswordService,
                                       StringPtr,
                                       (SysPasswordMaxNumber (SysPasswordService) + 1) * sizeof(CHAR16)
                                       );

        SetSysPasswordCurrentUser (
          SysPasswordService,
          CurrentUserType
          );
        gBS->FreePool (StringPtr);
        return EFI_SUCCESS;
      }
    }
    if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
      if (ScuInfo->UserPassword->NumOfEntry != 0) {
        Status = SysPasswordService->GetStatus(
                                       SysPasswordService,
                                       SystemUser
                                       );

        if (Status == EFI_SUCCESS) {
          //
          // Check User password
          //
          Status = SysPasswordService->CheckPassword (
                                         SysPasswordService,
                                         StringPtr,
                                         (SysPasswordMaxNumber (SysPasswordService) + 1) * sizeof(CHAR16),
                                         SystemUser
                                         );

          if (Status == EFI_SUCCESS) {
            KernelConfig->SetUserPass = TRUE;
            CurrentUserType = SystemUserType;
            gBS->FreePool (PasswordString);
            gBS->FreePool (SysInvalidPassword);
            SetSysPasswordCurrentUser (
              SysPasswordService,
              CurrentUserType
              );
            gBS->FreePool (StringPtr);
            return EFI_SUCCESS;
          }
        }
      }
    }

    TryPasswordTimes--;
    ReadChar = TRUE;
    if (TryPasswordTimes > 0) {
      SuBrowser->H2ODialog->ConfirmDialog (
                                2,
                                FALSE,
                                40,
                                NULL,
                                &Key,
                                SysInvalidPassword
                                );
    }
  } while (TryPasswordTimes > 0);

  //
  // OemServices
  //
  Status = OemSvcCheckPasswordFailCallBack ();

  if (Status == EFI_UNSUPPORTED) {
    ErrorStatusMsg    = HiiGetString (HiiHandle, STRING_TOKEN (STR_SYS_DIALOG_ERROR_STATUS), NULL);
    ThreeIncorrectMsg = HiiGetString (HiiHandle, STRING_TOKEN (STR_SYS_DIALOG_THREE_ERRORS), NULL);
    RebootMsg         = HiiGetString (HiiHandle, STRING_TOKEN (STR_SYS_DIALOG_RESTART_MSG), NULL);
    SuBrowser->H2ODialog->CreateMsgPopUp (
                            40,
                            3,
                            ErrorStatusMsg,
                            ThreeIncorrectMsg,
                            RebootMsg
                            );

    CpuDeadLoop ();
  }

  gBS->FreePool (PasswordString);
  gBS->FreePool (StringPtr);

  return EFI_SECURITY_VIOLATION;
}

/**
  Check Harddisk security ststus.

  @param  HddPasswordScuData     The Harddisk information array.
  @param  NumOfHdd               Number of simple password protocol of harddisk.

  @retval TRUE                   Harddisk security status is locked
  @retval EFI_NOT_FOUND          It could not found any harddisk security status is locked.

**/
BOOLEAN
CheckAnyHddLocked (
  IN  HDD_PASSWORD_SCU_DATA      *HddPasswordScuData,
  IN  UINTN                      NumOfHdd
  )
{
  UINT32  Index;
  UINT32  StatusFlag;

  if ((HddPasswordScuData == NULL) || (NumOfHdd == 0)) {
    return FALSE;
  }

  StatusFlag = 0;
  for (Index = 0; Index < NumOfHdd; Index++) {
    StatusFlag |= HddPasswordScuData[Index].Flag;
  }

  if ((StatusFlag & SECURITY_LOCK) != 0) {
    return TRUE;
  }

  return FALSE;
}

/**
  Check hard disk password status.

  @param  HiiHandle              Specific HII handle for Security menu.
  @param  BootOrSCU              TRUE: In to SCU
                                 FALSE: want to boot
  @param  KernelConfig           Point to KERNEL_CONFIGURATION instance.
  @param  SavePasswordPoint      Point to security harddisk password instance

  @retval EFI_SUCCESS            Check hard disk password success.
  @retval EFI_NOT_FOUND          Not found system password.

**/
EFI_STATUS
FrozenHddResetStatus (
  VOID
  )
{
  EFI_STATUS                            Status;
  UINTN                                 Index;
  EFI_HDD_PASSWORD_SERVICE_PROTOCOL     *HddPasswordService;
  HDD_PASSWORD_HDD_INFO                 *HddInfoArray;
  UINTN                                 NumOfHdd;
  SETUP_UTILITY_CONFIGURATION           *SUCInfo;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;

  HddPasswordService = NULL;
  HddInfoArray       = NULL;
  NumOfHdd           = 0;

  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }


  SUCInfo = SuBrowser->SUCInfo;

  HddPasswordService = SUCInfo->HddPasswordService;

  Status = HddPasswordService->GetHddInfo (
                                 HddPasswordService,
                                 &HddInfoArray,
                                 &NumOfHdd
                                 );
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  if (NumOfHdd == 0) {
    return EFI_SUCCESS;
  }

  for (Index = 0; Index < NumOfHdd; Index++) {
    if ((HddInfoArray[Index].HddSecurityStatus & HDD_SECURITY_FROZEN) == HDD_SECURITY_FROZEN) {

        HddPasswordService->ResetSecuirtyStatus (
                              HddPasswordService,
                              &HddInfoArray[Index]
                              );
    }
  }

  if (HddInfoArray != NULL) {
    gBS->FreePool (HddInfoArray);
  }

  return EFI_SUCCESS;
}

/**
  Set user set password status. This founction could set password status
  when press 'F10' and set 'yes'.

  @retval EFI_SUCCESS            Set system password and harddisk password success.

**/
EFI_STATUS
SetSecurityStatus (
  VOID
  )
{
  UINTN                               Index;
  SETUP_UTILITY_CONFIGURATION         *SUCInfo;
  EFI_STATUS                          Status;
  SETUP_UTILITY_BROWSER_DATA          *SuBrowser;
  UINT8                               PasswordToHdd[HDD_PASSWORD_MAX_NUMBER + 1];
  UINTN                               PasswordToHddLength;
  UINTN                               DisabledNumOfHdd;
  HDD_PASSWORD_TABLE                  *HddPasswordTable;
  UINTN                               HddPasswordTableSize;

  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SUCInfo = SuBrowser->SUCInfo;

  if (SUCInfo->SysPasswordService == NULL) {
    return EFI_NOT_READY;
  }

  if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
    //
    // Set User Password
    //
    if (SUCInfo->UserPassword->Flag != NO_ACCESS_PASSWORD) {

      SUCInfo->SysPasswordService->DisablePassword (
                                     SUCInfo->SysPasswordService,
                                     SystemUser
                                     );

      if (SUCInfo->UserPassword->Flag == CHANGE_PASSWORD) {
        SUCInfo->SysPasswordService->SetPassword (
                                       SUCInfo->SysPasswordService,
                                       SUCInfo->UserPassword->InputString,
                                       SUCInfo->UserPassword->StringLength,
                                       SystemUser
                                       );
      }
    }
  }

  //
  // Set Supervisor Password
  //
  if (SUCInfo->SupervisorPassword->Flag != NO_ACCESS_PASSWORD) {
    SUCInfo->SysPasswordService->DisablePassword (
                                   SUCInfo->SysPasswordService,
                                   SystemSupervisor
                                   );

    if (SUCInfo->SupervisorPassword->Flag == CHANGE_PASSWORD) {
      SUCInfo->SysPasswordService->SetPassword (
                                     SUCInfo->SysPasswordService,
                                     SUCInfo->SupervisorPassword->InputString,
                                     SUCInfo->SupervisorPassword->StringLength,
                                     SystemSupervisor
                                     );
    }
  }

  //
  // HDD Password
  //
  DisabledNumOfHdd = 0;
  Status = CommonGetVariableDataAndSize (
             SAVE_HDD_PASSWORD_VARIABLE_NAME,
             &gSaveHddPasswordGuid,
             &HddPasswordTableSize,
             (VOID **) &HddPasswordTable
             );

  if (Status == EFI_SUCCESS) {
    //
    // May get old table
    //

    if ((HddPasswordTableSize / sizeof (HDD_PASSWORD_TABLE)) != (SUCInfo->NumOfHdd + 1) ) {
      Status = EFI_NOT_FOUND;
      gBS->FreePool (HddPasswordTable);
    }
  }

  if (Status != EFI_SUCCESS) {
    //
    // Create new table
    //

    HddPasswordTable = NULL;
    HddPasswordTableSize = sizeof (HDD_PASSWORD_TABLE) * (SUCInfo->NumOfHdd + 1);
    HddPasswordTable = AllocateZeroPool (HddPasswordTableSize);
    if (HddPasswordTable == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  for (Index = 0; Index < SUCInfo->NumOfHdd; Index++) {
    if ((SUCInfo->HddPasswordScuData[Index].Flag == NO_ACCESS_PASSWORD) &&
        (SUCInfo->HddPasswordScuData[Index].MasterFlag == NO_ACCESS_PASSWORD)) {
      DisabledNumOfHdd++;
      continue;
    }

    if ((SUCInfo->HddPasswordScuData[Index].Flag != CHANGE_PASSWORD) &&
        (SUCInfo->HddPasswordScuData[Index].Flag != DISABLE_PASSWORD) &&
        (SUCInfo->HddPasswordScuData[Index].MasterFlag != CHANGE_PASSWORD) &&
        (SUCInfo->HddPasswordScuData[Index].MasterFlag != DISABLE_PASSWORD)) {
      continue;
    }

    if (SUCInfo->HddPasswordScuData[Index].Flag == SECURITY_LOCK) {
      continue;
    }

    //
    // Change HDD user password
    //
    if (SUCInfo->HddPasswordScuData[Index].Flag == CHANGE_PASSWORD) {
      ZeroMem (PasswordToHdd, (HDD_PASSWORD_MAX_NUMBER + 1));
      PasswordToHddLength = 0;

      Status = SUCInfo->HddPasswordService->PasswordStringProcess (
                                     SUCInfo->HddPasswordService,
                                     USER_PSW,
                                     SUCInfo->HddPasswordScuData[Index].InputString,
                                     StrLen (SUCInfo->HddPasswordScuData[Index].InputString),
                                     (VOID **)&PasswordToHdd,
                                     &PasswordToHddLength
                                     );

      if (EFI_ERROR(Status)) {
        continue;
      }

      Status = SUCInfo->HddPasswordService->SetHddPassword (
                                     SUCInfo->HddPasswordService,
                                     SUCInfo->HddPasswordScuData[Index].HddInfo,
                                     USER_PSW,
                                     PasswordToHdd,
                                     PasswordToHddLength
                                     );
      if (Status == EFI_SUCCESS) {
        CopyMem (
          HddPasswordTable[Index].PasswordStr,
          SUCInfo->HddPasswordScuData[Index].InputString,
          StrSize (SUCInfo->HddPasswordScuData[Index].InputString)
          );
        HddPasswordTable[Index].PasswordType     = USER_PSW;
        HddPasswordTable[Index].ControllerNumber = SUCInfo->HddPasswordScuData[Index].HddInfo->ControllerNumber;
        HddPasswordTable[Index].PortNumber       = SUCInfo->HddPasswordScuData[Index].HddInfo->PortNumber;
        HddPasswordTable[Index].PortMulNumber    = SUCInfo->HddPasswordScuData[Index].HddInfo->PortMulNumber;
      }
    }

    //
    // Change HDD Master password
    //
    if (SUCInfo->HddPasswordScuData[Index].MasterFlag == CHANGE_PASSWORD ||
        (SUCInfo->HddPasswordScuData[Index].Flag == CHANGE_PASSWORD &&
         SUCInfo->HddPasswordScuData[Index].MasterFlag == MASTER_PASSWORD_GRAYOUT &&
         SUCInfo->HddPasswordScuData[Index].MasterInputString[0] != 0) ) {
      //
      // Change Password
      //   Condition 1: Master HDD password is changed
      //   Condition 2: MasterFlag is not CHANGE_PASSWORD but Master HDD Password should be set.
      //                When supporting "Set Master HDD Password" related item to grayout
      //                by HDD security status changing from disabled to enabled,
      //                there are more check rules to determine to set master HDD password.
      //

      ZeroMem (PasswordToHdd, (HDD_PASSWORD_MAX_NUMBER + 1));
      PasswordToHddLength = 0;

      Status = SUCInfo->HddPasswordService->PasswordStringProcess (
                                              SUCInfo->HddPasswordService,
                                              MASTER_PSW,
                                              SUCInfo->HddPasswordScuData[Index].MasterInputString,
                                              StrLen (SUCInfo->HddPasswordScuData[Index].MasterInputString),
                                              (VOID **)&PasswordToHdd,
                                              &PasswordToHddLength
                                              );

      if (EFI_ERROR(Status)) {
        continue;
      }

      SUCInfo->HddPasswordService->SetHddPassword (
                                     SUCInfo->HddPasswordService,
                                     SUCInfo->HddPasswordScuData[Index].HddInfo,
                                     MASTER_PSW,
                                     PasswordToHdd,
                                     PasswordToHddLength
                                     );
    }
    //
    // Disable User Password
    //
    if (SUCInfo->HddPasswordScuData[Index].Flag == DISABLE_PASSWORD) {
      ZeroMem (PasswordToHdd, (HDD_PASSWORD_MAX_NUMBER + 1));
      PasswordToHddLength = 0;

      Status = SUCInfo->HddPasswordService->PasswordStringProcess (
                                              SUCInfo->HddPasswordService,
                                              USER_PSW,
                                              SUCInfo->HddPasswordScuData[Index].DisableInputString,
                                              SUCInfo->HddPasswordScuData[Index].DisableStringLength,
                                              (VOID **)&PasswordToHdd,
                                              &PasswordToHddLength
                                              );

      if (EFI_ERROR(Status)) {
        continue;
      }


      Status = SUCInfo->HddPasswordService->DisableHddPassword (
                                              SUCInfo->HddPasswordService,
                                              SUCInfo->HddPasswordScuData[Index].HddInfo,
                                              USER_PSW,
                                              PasswordToHdd,
                                              PasswordToHddLength
                                              );
      if (Status == EFI_SUCCESS) {
        DisabledNumOfHdd++;
        continue;
      }
    }
    //
    // Disable Master Password
    //
    if (SUCInfo->HddPasswordScuData[Index].MasterFlag == DISABLE_PASSWORD) {
      ZeroMem (PasswordToHdd, (HDD_PASSWORD_MAX_NUMBER + 1));
      PasswordToHddLength = 0;

      Status = SUCInfo->HddPasswordService->PasswordStringProcess (
                                              SUCInfo->HddPasswordService,
                                              MASTER_PSW,
                                              SUCInfo->HddPasswordScuData[Index].DisableInputString,
                                              SUCInfo->HddPasswordScuData[Index].DisableStringLength,
                                              (VOID **)&PasswordToHdd,
                                              &PasswordToHddLength
                                              );

      if (EFI_ERROR(Status)) {
        continue;
      }


      Status = SUCInfo->HddPasswordService->DisableHddPassword (
                                              SUCInfo->HddPasswordService,
                                              SUCInfo->HddPasswordScuData[Index].HddInfo,
                                              MASTER_PSW,
                                              PasswordToHdd,
                                              PasswordToHddLength
                                              );
      if (Status == EFI_SUCCESS) {
        DisabledNumOfHdd++;
        continue;
      }
      if (Status != EFI_SUCCESS) {
        ZeroMem (PasswordToHdd, (HDD_PASSWORD_MAX_NUMBER + 1));
        PasswordToHddLength = 0;

        Status = SUCInfo->HddPasswordService->PasswordStringProcess (
                                                SUCInfo->HddPasswordService,
                                                SUCInfo->HddPasswordScuData[Index].DisableAllType,
                                                SUCInfo->HddPasswordScuData[Index].DisableAllInputString,
                                                SUCInfo->HddPasswordScuData[Index].DisableStringLength,
                                                (VOID **)&PasswordToHdd,
                                                &PasswordToHddLength
                                                );

        if (EFI_ERROR(Status)) {
          continue;
        }

        Status = SUCInfo->HddPasswordService->DisableHddPassword (
                                                SUCInfo->HddPasswordService,
                                                SUCInfo->HddPasswordScuData[Index].HddInfo,
                                                SUCInfo->HddPasswordScuData[Index].DisableAllType,
                                                PasswordToHdd,
                                                PasswordToHddLength
                                                );
        if (Status == EFI_SUCCESS) {
          DisabledNumOfHdd++;
        }
      }
    }
  }

  if (DisabledNumOfHdd == SUCInfo->NumOfHdd) {
      Status = CommonSetVariable (
         SAVE_HDD_PASSWORD_VARIABLE_NAME,
         &gSaveHddPasswordGuid,
         EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
         0,
         NULL
         );
  } else {
    Status = CommonSetVariable (
       SAVE_HDD_PASSWORD_VARIABLE_NAME,
       &gSaveHddPasswordGuid,
       EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
       HddPasswordTableSize,
       HddPasswordTable
       );

    //
    //  Reset all storage device security status to locked if security enabled
    //
    for (Index = 0; Index < SUCInfo->NumOfHdd; Index++) {
      if ((SUCInfo->HddPasswordScuData[Index].HddInfo->HddSecurityStatus & HDD_ENABLE_BIT) == HDD_ENABLE_BIT) {
        Status = SUCInfo->HddPasswordService->ResetSecuirtyStatus (
                                                SUCInfo->HddPasswordService,
                                                SUCInfo->HddPasswordScuData[Index].HddInfo
                                                );
      }
    }
  }

  if (HddPasswordTable != NULL) {
    FreePool(HddPasswordTable);
  }
  return EFI_SUCCESS;
}



/**
  Check the system password checking flag to decide to ask password or not.

  @param  SysPasswordService     system password service protocol

  @retval TRUE                   the system password checking flag is enable.
  @retval FALSE                  the system password checking flag is disable.

**/
BOOLEAN
HaveSysPasswordCheck (
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL    *SysPasswordService  OPTIONAL
  )
{
  EFI_STATUS                              Status;
  SYS_PASSWORD_SETUP_INFO_DATA            SetupInfoBuffer;


  if (SysPasswordService == NULL) {
    Status = gBS->LocateProtocol (
                    &gEfiSysPasswordServiceProtocolGuid,
                    NULL,
                    (VOID **) &SysPasswordService
                    );
    if (EFI_ERROR (Status)) {
      return FALSE;
    }
  }


  Status = SysPasswordService->GetSysPswdSetupInfoData(
                                 SysPasswordService,
                                 &SetupInfoBuffer
                                 );

  return SetupInfoBuffer.HaveSysPasswordCheck;
}

/**
  Set system password checking to disable.

  @param  SysPasswordService     system password service protocol

  @retval EFI_SUCCESS            set successful

**/
EFI_STATUS
CleanSysPasswordCheck (
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL       *SysPasswordService OPTIONAL
  )
{
  EFI_STATUS                              Status;
  SYS_PASSWORD_SETUP_INFO_DATA            SetupInfoBuffer;
  UINT32                                  SetupBits;

  SetupBits = 0;

  if (SysPasswordService == NULL) {
    Status = gBS->LocateProtocol (
                    &gEfiSysPasswordServiceProtocolGuid,
                    NULL,
                    (VOID **) &SysPasswordService
                    );
    if (EFI_ERROR (Status)) {
      return FALSE;
    }
  }

  Status = SysPasswordService->GetSysPswdSetupInfoData(
                                 SysPasswordService,
                                 &SetupInfoBuffer
                                 );

  SetupInfoBuffer.HaveSysPasswordCheck = FALSE;

  SetupBits = SYS_PASSWORD_CHECK_BIT;

  Status = SysPasswordService->SetSysPswdSetupInfoData(
                                 SysPasswordService,
                                 &SetupInfoBuffer,
                                 SetupBits
                                 );

  return Status;
}

/**
  Get previous checking password is supervior password or user password.

  @param  SysPasswordService     system password service protocol

  @retval SystemSupervisorType   current user is supervisor.
  @retval SystemUserType         current user is user.

**/
CURRENT_USER_TYPE
GetSysPasswordCurrentUser (
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL       *SysPasswordService OPTIONAL
  )
{
  EFI_STATUS                              Status;
  SYS_PASSWORD_SETUP_INFO_DATA            SetupInfoBuffer;


  if (SysPasswordService == NULL) {
    Status = gBS->LocateProtocol (
                    &gEfiSysPasswordServiceProtocolGuid,
                    NULL,
                    (VOID **) &SysPasswordService
                    );
    if (EFI_ERROR (Status)) {
      return FALSE;
    }
  }

  Status = SysPasswordService->GetSysPswdSetupInfoData(
                                 SysPasswordService,
                                 &SetupInfoBuffer
                                 );

  return SetupInfoBuffer.CurrentUser;
}

/**
  Set current password input is supervior password or user password.

  @param  SysPasswordService     system password service protocol
  @param  CurrentUserType        systemSupervisorType or systemUserType

  @retval EFI_SUCCESS            set current user successful.

**/
EFI_STATUS
SetSysPasswordCurrentUser (
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL       *SysPasswordService, OPTIONAL
  CURRENT_USER_TYPE       CurrentUserType
  )
{
  EFI_STATUS                              Status;
  SYS_PASSWORD_SETUP_INFO_DATA            SetupInfoBuffer;
  UINT32                                  SetupBits;

  SetupBits = 0;

  if (SysPasswordService == NULL) {
    Status = gBS->LocateProtocol (
                    &gEfiSysPasswordServiceProtocolGuid,
                    NULL,
                    (VOID **) &SysPasswordService
                    );
    if (EFI_ERROR (Status)) {
      return FALSE;
    }
  }

  Status = SysPasswordService->GetSysPswdSetupInfoData(
                                 SysPasswordService,
                                 &SetupInfoBuffer
                                 );

  SetupInfoBuffer.CurrentUser = CurrentUserType;

  SetupBits = CURRENT_USER_BIT;

  Status = SysPasswordService->SetSysPswdSetupInfoData(
                                 SysPasswordService,
                                 &SetupInfoBuffer,
                                 SetupBits
                                 );

  return Status;
}

/**
  Get the length of system password support.

  @param  SysPasswordService     system password service protocol

  @retval                        return the length of system password support.

**/
UINTN
SysPasswordMaxNumber (
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL    *SysPasswordService  OPTIONAL
  )
{
  EFI_STATUS                              Status;
  SYS_PASSWORD_SETUP_INFO_DATA            SetupInfoBuffer;


  if (SysPasswordService == NULL) {
    Status = gBS->LocateProtocol (
                    &gEfiSysPasswordServiceProtocolGuid,
                    NULL,
                    (VOID **) &SysPasswordService
                    );
    if (EFI_ERROR (Status)) {
      return FALSE;
    }
  }


  Status = SysPasswordService->GetSysPswdSetupInfoData(
                                 SysPasswordService,
                                 &SetupInfoBuffer
                                 );

  if (EFI_ERROR(Status)) {
    return 0;
  }

  return SetupInfoBuffer.MaxSysPasswordLength;
}

/**
 Initialize HDD Password information

 @retval EFI_SUCCESS            Initialize setup utility private data success
 @retval EFI_OUT_OF_RESOURCES   Allocate pool fail
 @return other                  Get setup utility private data fail or locate HDD Password service protocol fail or
                                get HDD security info fail
**/
EFI_STATUS
InitHddPasswordScuData (
  VOID
  )
{
  EFI_HDD_PASSWORD_SERVICE_PROTOCOL     *HddPasswordService;
  HDD_PASSWORD_SCU_DATA                 *HddPasswordScuData;
  UINTN                                 Index;
  EFI_STATUS                            Status;
  UINTN                                 NumOfHdd;
  HDD_PASSWORD_HDD_INFO                 *HddInfoArray;
  SETUP_UTILITY_CONFIGURATION           *SUCInfo;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;
  HDD_PASSWORD_TABLE                    *HddPasswordTable;
  UINTN                                 DataSize;

  SuBrowser = NULL;
  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (Status != EFI_SUCCESS || SuBrowser == NULL) {
    return Status;
  }


  HddPasswordScuData          = NULL;
  HddInfoArray                = NULL;
  SUCInfo                     = SuBrowser->SUCInfo;
  HddPasswordService          = SUCInfo->HddPasswordService;
  SUCInfo->HddPasswordScuData = NULL;
  SUCInfo->NumOfHdd           = 0;
  SuBrowser->IdeConfig        = NULL;
  NumOfHdd                    = 0;
  HddPasswordTable            = NULL;
  DataSize                    = 0;

  ZeroMem (&mPasswordConfig, sizeof (PASSWORD_CONFIGURATION));

  if (HddPasswordService == NULL) {
    return EFI_UNSUPPORTED;
  }

  Status = HddPasswordService->GetHddInfo (
                                 HddPasswordService,
                                 &HddInfoArray,
                                 &NumOfHdd
                                 );
  if (Status != EFI_SUCCESS || NumOfHdd == 0) {
    return Status;
  }

  HddPasswordScuData = (HDD_PASSWORD_SCU_DATA *)AllocateZeroPool (((NumOfHdd) * sizeof(HDD_PASSWORD_SCU_DATA)));

  if (HddPasswordScuData == NULL) {
    return Status;
  }

  SUCInfo->NumOfHdd = NumOfHdd;
  SUCInfo->HddPasswordScuData  = HddPasswordScuData;

  if (NumOfHdd != 0) {
    mPasswordConfig.HddPasswordSupport = 1;

    Status = CommonGetVariableDataAndSize (
               SAVE_HDD_PASSWORD_VARIABLE_NAME,
               &gSaveHddPasswordGuid,
               &DataSize,
               (VOID **) &HddPasswordTable
               );

    for (Index = 0; Index < NumOfHdd; Index++) {
      if ((HddInfoArray[Index].HddSecurityStatus & HDD_SECURITY_LOCK) == HDD_SECURITY_LOCK) {
        HddPasswordScuData[Index].Flag = SECURITY_LOCK;
      } else if ((HddInfoArray[Index].HddSecurityStatus & HDD_SECURITY_ENABLE) == HDD_SECURITY_ENABLE) {
        HddPasswordScuData[Index].Flag = ENABLE_PASSWORD;

        if (HddPasswordTable != NULL) {
          if (HddPasswordTable[Index].PasswordType != MASTER_PSW) {
            HddPasswordScuData[Index].MasterFlag = MASTER_PASSWORD_GRAYOUT;
          }

          ZeroMem (
            HddPasswordScuData[Index].DisableInputString,
            (HDD_PASSWORD_MAX_NUMBER + 1) * sizeof (CHAR16)
            );

          StrCpy (
            HddPasswordScuData[Index].DisableInputString,
            HddPasswordTable[Index].PasswordStr
            );

          HddPasswordScuData[Index].DisableAllType = HddPasswordTable[Index].PasswordType;
          HddPasswordScuData[Index].DisableStringLength = StrLen (HddPasswordTable[Index].PasswordStr);
        }

      } else {
        HddPasswordScuData[Index].Flag           = NO_ACCESS_PASSWORD;
        HddPasswordScuData[Index].MasterFlag     = NO_ACCESS_PASSWORD;
        HddPasswordScuData[Index].DisableAllType = NO_ACCESS_PSW;
      }
      HddPasswordScuData[Index].HddInfo = &HddInfoArray[Index];
      HddPasswordScuData[Index].NumOfEntry  = 0x01;
    }

    if (HddPasswordTable != NULL) {
      FreePool (HddPasswordTable);
    }
  }

  return EFI_SUCCESS;
}

/**
 Reset System Password information for SCU

 @retval EFI_SUCCESS            Initialize setup utility private data success
 @retval EFI_OUT_OF_RESOURCES   Allocate pool fail
 @return other                  Get setup utility private data fail or locate System Password service protocol fail or
                                get System password info fail
**/
EFI_STATUS
ResetSysPasswordInfoForSCU (
  VOID
  )
{
  EFI_STATUS                                Status;
  SETUP_UTILITY_CONFIGURATION               *SUCInfo;
  SETUP_UTILITY_BROWSER_DATA                *SuBrowser;

  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SUCInfo = SuBrowser->SUCInfo;

  if (SUCInfo->SysPasswordService != NULL) {
    if (SUCInfo->SupervisorPassword  != NULL) {
      if (SUCInfo->SupervisorPassword->InputString != NULL) {
        gBS->FreePool (SUCInfo->SupervisorPassword->InputString);
      }
      gBS->FreePool (SUCInfo->SupervisorPassword);
      SUCInfo->SupervisorPassword = NULL;
    }
    if (SUCInfo->UserPassword  != NULL) {
      if (SUCInfo->UserPassword->InputString != NULL) {
        gBS->FreePool (SUCInfo->UserPassword->InputString);
      }
      gBS->FreePool (SUCInfo->UserPassword);
      SUCInfo->UserPassword = NULL;
    }

    Status = PasswordCheck (
               SUCInfo,
               (KERNEL_CONFIGURATION *) SuBrowser->SCBuffer
               );
  }

  return Status;
}

/**
 Reset HDD Password information for SCU

 @retval EFI_SUCCESS            Initialize setup utility private data success
 @retval EFI_OUT_OF_RESOURCES   Allocate pool fail
 @return other                  Get setup utility private data fail or locate HDD Password service protocol fail or
                                get HDD security info fail
**/
EFI_STATUS
ResetHddPasswordInfoForSCU (
  VOID
  )
{
  EFI_STATUS                                Status;
  SETUP_UTILITY_CONFIGURATION               *SUCInfo;
  SETUP_UTILITY_BROWSER_DATA                *SuBrowser;

  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SUCInfo = SuBrowser->SUCInfo;

  if (SUCInfo->HddPasswordService!= NULL) {
    ZeroMem (&mPasswordConfig, sizeof (PASSWORD_CONFIGURATION));

    if (SUCInfo->HddPasswordScuData != NULL) {
      if (SUCInfo->HddPasswordScuData[0].HddInfo != NULL) {
        gBS->FreePool (SUCInfo->HddPasswordScuData[0].HddInfo);
      }
      gBS->FreePool (SUCInfo->HddPasswordScuData);
      SUCInfo->HddPasswordScuData = NULL;
      SUCInfo->NumOfHdd = 0;
    }

    Status = InitHddPasswordScuData ();
  }

  return Status;
}

EFI_STATUS
EFIAPI
InitStoragePasswordForSCU (
  IN  EFI_HII_HANDLE                    HiiHandle
  )
{
  EFI_STATUS                            Status;
  UINTN                                 Index;
  SETUP_UTILITY_CONFIGURATION           *SUCInfo;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;
  VOID                                  *StartOpCodeHandle;
  VOID                                  *EndOpCodeHandle;
  EFI_IFR_GUID_LABEL                    *StartLabel;
  EFI_IFR_GUID_LABEL                    *EndLabel;
  EFI_HDD_PASSWORD_SERVICE_PROTOCOL     *HddPasswordService;
  HDD_PASSWORD_SCU_DATA                 *HddPasswordScuData;
  UINTN                                 NumOfHdd;
  HDD_PASSWORD_HDD_INFO                 *HddInfo;
  CHAR16                                HddModelString[DEVICE_MODEL_NAME_STRING_LENGTH + 1];
  EFI_STRING_ID                         DeviceName;
  UINTN                                 StringIndex;

  SuBrowser = NULL;
  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (Status != EFI_SUCCESS || SuBrowser == NULL) {
    return Status;
  }

  SUCInfo            = SuBrowser->SUCInfo;
  HddPasswordService = SUCInfo->HddPasswordService;
  HddPasswordScuData = SUCInfo->HddPasswordScuData;
  NumOfHdd           = SUCInfo->NumOfHdd;


  if (HddPasswordService == NULL || HddPasswordScuData == NULL || NumOfHdd == 0) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate space for creation of UpdateData Buffer
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);

  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = LABEL_STORAGE_PASSWORD_OPTION;

  //
  // Create Hii Extend Label OpCode as the end opcode
  //
  EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = LABEL_STORAGE_PASSWORD_OPTION_END;





  for (Index = 0; Index < NumOfHdd; Index++) {

    HddInfo = HddPasswordScuData[Index].HddInfo;

    //
    // Copy Model String
    //
    ZeroMem (HddModelString, (DEVICE_MODEL_NAME_STRING_SIZE + sizeof(CHAR16)));
    CopyMem (HddModelString, HddInfo->HddModelString, DEVICE_MODEL_NAME_STRING_SIZE);

    //
    // Clean space from tail
    //
    StringIndex = DEVICE_MODEL_NAME_STRING_LENGTH - 1;
    while (HddModelString[StringIndex] == 0x0020) {
      SetMem (&HddModelString[StringIndex], sizeof(CHAR16), 0x0000);
      StringIndex--;
    }

    //
    // Add user name selection.
    //
    DeviceName = HiiSetString (HiiHandle, 0, (EFI_STRING)HddModelString, NULL);

    HiiCreateGotoOpCode (
      StartOpCodeHandle,                                // Container for dynamic created opcodes
      STORAGE_PASSWORD_DEVICE_FORM_ID,                  // Target Form ID
      DeviceName,                                       // Prompt text
      STRING_TOKEN (STR_STORAGE_PASSWORD_DEVICE_HELP),  // Help text
      EFI_IFR_FLAG_CALLBACK,                            // Question flag
      (UINT16)(LABEL_STORAGE_PASSWORD_OPTION + Index)   // Question ID
      );

  }

  HiiCreateEndOpCode (EndOpCodeHandle);

  HiiUpdateForm (
    HiiHandle,
    NULL,
    STORAGE_PASSWORD_FORM_ID,
    StartOpCodeHandle,
    EndOpCodeHandle
    );

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);

  mPasswordConfig.StoragePasswordCallbackState = BROWSER_STATE_VALIDATE_PASSWORD;

  return EFI_SUCCESS;
}

EFI_STATUS
StoragePasswordUpdateForm (
  IN  EFI_HII_HANDLE                      HiiHandle,
  IN  UINT16                              HddIndex
  )
{
  EFI_STATUS                            Status;
  EFI_IFR_GUID_LABEL                    *StartUserPasswordLabel;
  EFI_IFR_GUID_LABEL                    *EndUserPasswordLabel;
  EFI_IFR_GUID_LABEL                    *StartMasterPasswordLabel;
  EFI_IFR_GUID_LABEL                    *EndMasterPasswordLabel;
  HDD_PASSWORD_HDD_INFO                 *HddInfo;
  CHAR16                                HddModelString[DEVICE_MODEL_NAME_STRING_LENGTH + 1];
  CHAR16                                *NewString;
  CHAR16                                ModelNameToken[DEVICE_MODEL_NAME_STRING_LENGTH + 3];
  EFI_STRING_ID                         StorageStatusToken;
  VOID                                  *StartUserPasswordOpCodeHandle;
  VOID                                  *EndUserPasswordOpCodeHandle;
  VOID                                  *StartMasterPasswordOpCodeHandle;
  VOID                                  *EndMasterPasswordOpCodeHandle;
  SETUP_UTILITY_CONFIGURATION           *SUCInfo;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;
  EFI_HDD_PASSWORD_SERVICE_PROTOCOL     *HddPasswordService;
  HDD_PASSWORD_SCU_DATA                 *HddPasswordScuData;
  UINTN                                 NumOfHdd;
  UINTN                                 StringIndex;

  SuBrowser = NULL;
  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (Status != EFI_SUCCESS || SuBrowser == NULL) {
    return Status;
  }

  SUCInfo            = SuBrowser->SUCInfo;
  HddPasswordService = SUCInfo->HddPasswordService;
  HddPasswordScuData = SUCInfo->HddPasswordScuData;
  NumOfHdd           = SUCInfo->NumOfHdd;

  if (HddPasswordService == NULL || HddPasswordScuData == NULL || NumOfHdd == 0) {
    return EFI_UNSUPPORTED;
  }

  StartUserPasswordOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartUserPasswordOpCodeHandle != NULL);

  EndUserPasswordOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndUserPasswordOpCodeHandle != NULL);

  StartMasterPasswordOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartMasterPasswordOpCodeHandle != NULL);

  EndMasterPasswordOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndMasterPasswordOpCodeHandle != NULL);

  //
  // Create Hii Extend Label OpCode as the start storage user password opcode
  //
  StartUserPasswordLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartUserPasswordOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartUserPasswordLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartUserPasswordLabel->Number       = LABEL_STORAGE_PASSWORD_DEVICE_USER_PASSWORD_OPTION;

  //
  // Create Hii Extend Label OpCode as the end storage user password opcode
  //
  EndUserPasswordLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndUserPasswordOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndUserPasswordLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndUserPasswordLabel->Number       = LABEL_STORAGE_PASSWORD_DEVICE_USER_PASSWORD_OPTION_END;

  //
  // Create Hii Extend Label OpCode as the start storage master password opcode
  //
  StartMasterPasswordLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartMasterPasswordOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartMasterPasswordLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartMasterPasswordLabel->Number       = LABEL_STORAGE_PASSWORD_DEVICE_MASTER_PASSWORD_OPTION;

  //
  // Create Hii Extend Label OpCode as the end storage master password opcode
  //
  EndMasterPasswordLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndMasterPasswordOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndMasterPasswordLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndMasterPasswordLabel->Number       = LABEL_STORAGE_PASSWORD_DEVICE_MASTER_PASSWORD_OPTION_END;

  mPasswordConfig.SelectedScuDataIndex = (UINT8)HddIndex;

  HddInfo = HddPasswordScuData[HddIndex].HddInfo;

  //
  // Copy Model String
  //
  ZeroMem (HddModelString, (DEVICE_MODEL_NAME_STRING_SIZE + sizeof(CHAR16)));
  CopyMem (HddModelString, HddInfo->HddModelString, DEVICE_MODEL_NAME_STRING_SIZE);

  //
  // Clean space from tail
  //
  StringIndex = DEVICE_MODEL_NAME_STRING_LENGTH - 1;
  while (HddModelString[StringIndex] == 0x0020) {
    SetMem (&HddModelString[StringIndex], sizeof(CHAR16), 0x0000);
    StringIndex--;
  }

  ZeroMem (ModelNameToken, (DEVICE_MODEL_NAME_STRING_SIZE + sizeof(CHAR16) * 3));


  StrCat (ModelNameToken, L"[");
  StrCat (ModelNameToken, HddModelString);
  StrCat (ModelNameToken, L"]");

  HiiSetString (
    HiiHandle,
    STRING_TOKEN (STR_STORAGE_DEVICE_NAME_STRING),
    ModelNameToken,
    NULL
    );


  //
  // Update Information
  //
  if (HddPasswordScuData[HddIndex].Flag == SECURITY_LOCK) {
    //
    // Harddisk Security Enable [Unlock]
    //
    StorageStatusToken = STRING_TOKEN (STR_HDD_PSW_LOCK);
  } else if (HddPasswordScuData[HddIndex].Flag == CHANGE_PASSWORD ||
             ((HddPasswordScuData[HddIndex].Flag != DISABLE_PASSWORD &&
               HddPasswordScuData[HddIndex].Flag != NO_ACCESS_PASSWORD) &&
              HddPasswordScuData[HddIndex].MasterFlag == CHANGE_PASSWORD)) {
    //
    // Password Changed
    // 1. User Password changed.
    // 2. Security enabled, only master password changed.
    //
    StorageStatusToken = STRING_TOKEN (STR_HDD_PSW_CHANGE);
  } else if (HddPasswordScuData[HddIndex].Flag == DISABLE_PASSWORD ||
             HddPasswordScuData[HddIndex].MasterFlag == DISABLE_PASSWORD ) {
    //
    // Harddisk Security Disable
    //
    StorageStatusToken = STRING_TOKEN (STR_HDD_PSW_DISABLE);
  } else if (HddPasswordScuData[HddIndex].Flag == ENABLE_PASSWORD) {
    //
    // Harddisk Security Unlock
    //
    StorageStatusToken = STRING_TOKEN (STR_HDD_PSW_UNLOCK);
  } else {
    //
    //Harddisk don't access password
    //
    StorageStatusToken = STRING_TOKEN (STR_HDD_PSW_NONACCESS);
  }

  NewString = HiiGetString (
                HiiHandle,
                StorageStatusToken,
                NULL
                );
  HiiSetString (
    HiiHandle,
    STRING_TOKEN (STR_STORAGE_SECURITY_MODE_STATUS),
    NewString,
    NULL
    );


  if (HddPasswordScuData[HddIndex].Flag == SECURITY_LOCK) {
    mPasswordConfig.SetHddPasswordFlag = 0;
    mPasswordConfig.UnlockHddPasswordFlag = 1;

    HiiCreatePasswordOpCodeEx (
      StartUserPasswordOpCodeHandle,
      (EFI_QUESTION_ID)(KEY_CHECK_STORAGE_PASSWORD),
      PASSWORD_NAME_VALUE_VARSTORE_ID,
      STRING_TOKEN (STR_HDD_PASSWORD_VAR_NAME),
      STRING_TOKEN (STR_STORAGE_CHECK_PASSWORD_STRING),
      STRING_TOKEN (STR_LOCK_ONE_STORAGE_SECURITY_TITLE_HELP),
      0x0,
      EFI_IFR_FLAG_CALLBACK,
      PcdGet16 (PcdH2OHddPasswordMinLength),
      MIN (PcdGet16 (PcdH2OHddPasswordMaxLength), HddInfo->MaxPasswordLengthSupport)
      );
  } else {
    mPasswordConfig.SetHddPasswordFlag = 1;
    mPasswordConfig.UnlockHddPasswordFlag = 0;

    HiiCreatePasswordOpCodeEx (
      StartUserPasswordOpCodeHandle,
      (EFI_QUESTION_ID)(KEY_SET_STORAGE_PASSWORD),
      PASSWORD_NAME_VALUE_VARSTORE_ID,
      STRING_TOKEN (STR_HDD_PASSWORD_VAR_NAME),
      STRING_TOKEN (STR_STORAGE_PASSWORD_PROMPT),
      STRING_TOKEN (STR_ONE_STORAGE_SECURITY_TITLE_HELP),
      0x0,
      EFI_IFR_FLAG_CALLBACK,
      PcdGet16 (PcdH2OHddPasswordMinLength),
      MIN (PcdGet16 (PcdH2OHddPasswordMaxLength), HddInfo->MaxPasswordLengthSupport)
      );
  }

  //
  // According to ATA8-ACS-4 spec, section 4.17.4
  // When security is enabled, master password is indicated by master passwrod capability
  // High - user and master password are interchageable
  // Maximum - master password cannot be used for security unlock, security disable
  // Only security erase unit command is available. Thus when the capability bit
  // is set maximum, we will hide the master password option in SCU
  //
  if (HddInfo->HddSecurityStatus & HDD_MASTER_CAPABILITY) {
    mPasswordConfig.UseMasterPassword = 0;
  } else {
    mPasswordConfig.UseMasterPassword = 1;

    if (HddPasswordScuData[HddIndex].MasterFlag == MASTER_PASSWORD_GRAYOUT) {
      mPasswordConfig.MasterHddPasswordFlag = 0;
      HiiCreateGrayOutIfOpCodeEx (StartMasterPasswordOpCodeHandle, 1);
      HiiCreateTrueOpCode (StartMasterPasswordOpCodeHandle);
    } else {
      mPasswordConfig.MasterHddPasswordFlag = 1;
    }

    HiiCreatePasswordOpCodeEx (
      StartMasterPasswordOpCodeHandle,
      (EFI_QUESTION_ID)(KEY_MASTER_STORAGE_PASSWORD),
      PASSWORD_NAME_VALUE_VARSTORE_ID,
      STRING_TOKEN (STR_MASTER_HDD_PASSWORD_VAR_NAME),
      STRING_TOKEN (STR_MASTER_STORAGE_PASSWORD_PROMPT),
      STRING_TOKEN (STR_ONE_STORAGE_MASTER_SECURITY_TITLE_HELP),
      0x0,
      EFI_IFR_FLAG_CALLBACK,
      PcdGet16 (PcdH2OHddPasswordMinLength),
      MIN (PcdGet16 (PcdH2OHddPasswordMaxLength), HddInfo->MaxPasswordLengthSupport)
      );

    if (HddPasswordScuData[HddIndex].MasterFlag == MASTER_PASSWORD_GRAYOUT) {
      HiiCreateEndOpCode (EndMasterPasswordOpCodeHandle);
    }

  }

  HiiUpdateForm (
    HiiHandle,
    NULL,
    STORAGE_PASSWORD_DEVICE_FORM_ID,
    StartUserPasswordOpCodeHandle,
    EndUserPasswordOpCodeHandle
    );

  HiiUpdateForm (
    HiiHandle,
    NULL,
    STORAGE_PASSWORD_DEVICE_FORM_ID,
    StartMasterPasswordOpCodeHandle,
    EndMasterPasswordOpCodeHandle
    );

  HiiFreeOpCodeHandle (StartUserPasswordOpCodeHandle);
  HiiFreeOpCodeHandle (EndUserPasswordOpCodeHandle);

  HiiFreeOpCodeHandle (StartMasterPasswordOpCodeHandle);
  HiiFreeOpCodeHandle (EndMasterPasswordOpCodeHandle);

  return EFI_SUCCESS;

}

EFI_STATUS
ValidateStoragePassword (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  UINT16                            HddIndex,
  IN  BOOLEAN                           UserOrMaster,
  IN  CHAR16                            *Password
  )
{
  EFI_STATUS                            Status;
  KERNEL_CONFIGURATION                  *MyIfrNVData;
  UINT8                                 PasswordToHdd[HDD_PASSWORD_MAX_NUMBER + 1];
  UINTN                                 PasswordToHddLength;
  HDD_PASSWORD_TABLE                    *HddPasswordTable;
  UINTN                                 DataSize;
  SETUP_UTILITY_CONFIGURATION           *SUCInfo;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;
  EFI_HDD_PASSWORD_SERVICE_PROTOCOL     *HddPasswordService;
  HDD_PASSWORD_SCU_DATA                 *HddPasswordScuData;
  UINTN                                 NumOfHdd;

  SuBrowser = NULL;
  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (Status != EFI_SUCCESS || SuBrowser == NULL) {
    //
    //  Return EFI_NOT_AVAILABLE_YET or EFI_UNSUPPORTED to let browser terminate password flow.
    //  If callback function return other status, the password flow won't terminate.
    //
    return EFI_NOT_AVAILABLE_YET;
  }

  SUCInfo            = SuBrowser->SUCInfo;
  HddPasswordService = SUCInfo->HddPasswordService;
  HddPasswordScuData = SUCInfo->HddPasswordScuData;
  NumOfHdd           = SUCInfo->NumOfHdd;

  if (HddPasswordService == NULL || HddPasswordScuData == NULL || NumOfHdd == 0) {
    //
    //  Return EFI_NOT_AVAILABLE_YET or EFI_UNSUPPORTED to let browser terminate password flow.
    //  If callback function return other status, the password flow won't terminate.
    //
    return EFI_NOT_AVAILABLE_YET;
  }

  MyIfrNVData = (KERNEL_CONFIGURATION *) SuBrowser->SCBuffer;
  HddPasswordTable = NULL;
  DataSize = 0;

  if (HddPasswordScuData[HddIndex].Flag == ENABLE_PASSWORD) {
  }
  if (HddPasswordScuData[HddIndex].Flag == DISABLE_PASSWORD) {
    //
    //  return EFI_SUCCESS to display two password items for new password and confirm new password
    //
    return EFI_SUCCESS;
  }
  if (HddPasswordScuData[HddIndex].Flag == NO_ACCESS_PASSWORD) {
    //
    //  return EFI_SUCCESS to display two password items for new password and confirm new password
    //
    return EFI_SUCCESS;
  }

  if (Password[0] == 0x00 && HddPasswordScuData[HddIndex].Flag == SECURITY_LOCK) {
    //
    // If First time enter here, HDD is LOCK and not yet ask user input HDD password
    //

    //
    //  return EFI_CRC_ERROR to display one password item for unlock HDD first by correct password
    //
    return EFI_CRC_ERROR;
  }

  if (Password[0] == 0) {
    //
    //  Check for an existing password by sending a NULL string value.
    //  Returns an error, indicating that there is a preexisting password.
    //
    return EFI_NOT_READY;
  }
  if (HddPasswordScuData[HddIndex].Flag != SECURITY_LOCK) {
    //
    //  To check previous password for change password or disable password
    //

    if (HddPasswordScuData[HddIndex].Flag == CHANGE_PASSWORD ||
        HddPasswordScuData[HddIndex].MasterFlag == CHANGE_PASSWORD) {
      //
      //  User has changed new password, but the new password is not set to HDD.
      //  The new password is stored in buffer until "Save and Exit" to set to HDD by SetSecurityStatus()
      //
      if (UserOrMaster == USER_PSW && HddPasswordScuData[HddIndex].InputString[0] != 0) {
        if (StrCmp (HddPasswordScuData[HddIndex].InputString, Password) == 0) {
          //
          //  input password is match current set User HDD password
          //  return EFI_SUCCESS to display another two password items for new password and confirm new password
          //
          return EFI_SUCCESS;
        } else {
          //
          //  input password is  NOT  match current set User HDD password
          //  return EFI_NOT_READY to clean dialog and let browser terminate password flow
          //
          return EFI_NOT_READY;
        }
      } else if (UserOrMaster == MASTER_PSW && HddPasswordScuData[HddIndex].MasterInputString[0] != 0) {
        if (StrCmp (HddPasswordScuData[HddIndex].MasterInputString, Password) == 0) {
          //
          //  input password is match current set Master password
          //  return EFI_SUCCESS to display another two password items for new password and confirm new password
          //
          return EFI_SUCCESS;
        } else {
          //
          //  input password is  NOT  match current set Master HDD password
          //  return EFI_NOT_READY to clean dialog and let browser terminate password flow.
          //
          return EFI_NOT_READY;
        }
      } else if (HddPasswordScuData[HddIndex].InputString[0] != 0 &&
                 HddPasswordScuData[HddIndex].MasterInputString[0] != 0) {
        //
        //  input password is  NOT  match current set password
        //  return EFI_NOT_READY to clean dialog and let browser terminate password flow.
        //
        return EFI_NOT_READY;
      }
    }

    if (HddPasswordScuData[HddIndex].DisableAllType == UserOrMaster &&
        StrCmp (HddPasswordScuData[HddIndex].DisableInputString, Password) == 0) {
      //
      //  Check previos HDD password stored in variable.
      //

      Status = StoragePasswordUpdateForm (
                 HiiHandle,
                 HddIndex
                 );

      //
      //  input password is match current set HDD password
      //  return EFI_SUCCESS to display another two password items for new password and confirm new password
      //
      return EFI_SUCCESS;
    } else {
      //
      //  Directly checking  password which is stored in HDD by using Unlock command.
      //

      ZeroMem (PasswordToHdd, (HDD_PASSWORD_MAX_NUMBER + 1));
      PasswordToHddLength = 0;

      Status = HddPasswordService->PasswordStringProcess (
                                     HddPasswordService,
                                     UserOrMaster,
                                     Password,
                                     StrLen (Password),
                                     (VOID **)&PasswordToHdd,
                                     &PasswordToHddLength
                                     );

      if (Status != EFI_SUCCESS) {
        //
        //  Return EFI_NOT_AVAILABLE_YET or EFI_UNSUPPORTED to let browser terminate password flow.
        //
        return EFI_NOT_AVAILABLE_YET;
      }

      Status = HddPasswordService->UnlockHddPassword (
                                     HddPasswordService,
                                     HddPasswordScuData[HddIndex].HddInfo,
                                     UserOrMaster,
                                     PasswordToHdd,
                                     PasswordToHddLength
                                     );
      if (EFI_ERROR (Status)) {
        //
        //  user input password is  NOT  match the password in HDD
        //  return EFI_NOT_READY to clean dialog and let browser terminate password flow.
        //
        return EFI_NOT_READY;
      }

      Status = StoragePasswordUpdateForm (
                 HiiHandle,
                 HddIndex
                 );

      //
      //  input password is match current set HDD password
      //  return EFI_SUCCESS to display another two password items for new password and confirm new password
      //
      return EFI_SUCCESS;
    }
  }

  //
  //  Unlock HDD in SCU
  //
  ZeroMem (PasswordToHdd, (HDD_PASSWORD_MAX_NUMBER + 1));
  PasswordToHddLength = 0;

  Status = HddPasswordService->PasswordStringProcess (
                                 HddPasswordService,
                                 UserOrMaster,
                                 Password,
                                 StrLen (Password),
                                 (VOID **)&PasswordToHdd,
                                 &PasswordToHddLength
                                 );

  if (EFI_ERROR(Status)) {
    //
    //  Return EFI_NOT_AVAILABLE_YET or EFI_UNSUPPORTED to let browser terminate password flow.
    //
    return EFI_NOT_AVAILABLE_YET;
  }

  Status = HddPasswordService->UnlockHddPassword (
                                 HddPasswordService,
                                 HddPasswordScuData[HddIndex].HddInfo,
                                 UserOrMaster,
                                 PasswordToHdd,
                                 PasswordToHddLength
                                 );
  if (EFI_ERROR (Status)) {
    //
    //  Unlock HDD in SCU fail
    //  return EFI_NOT_READY to clean dialog
    //
    return EFI_NOT_READY;
  }

  ZeroMem (
    HddPasswordScuData[HddIndex].DisableInputString,
    ((HDD_PASSWORD_MAX_NUMBER + 1) * sizeof (CHAR16))
    );
  StrCpy (
    HddPasswordScuData[HddIndex].DisableInputString,
    Password
    );
  HddPasswordScuData[HddIndex].DisableStringLength = StrLen (Password);
  HddPasswordScuData[HddIndex].DisableAllType = UserOrMaster;

  if (UserOrMaster != MASTER_PSW) {
    //
    //  If input HDD User Password,
    //  let Set HDD Master Password grayout
    //

    HddPasswordScuData[HddIndex].MasterFlag = MASTER_PASSWORD_GRAYOUT;
  }

  if ((UserOrMaster == MASTER_PSW) && (HddPasswordScuData[HddIndex].Flag != SECURITY_LOCK)) {
    HddPasswordScuData[HddIndex].MasterFlag = ENABLE_PASSWORD;
    Status = StoragePasswordUpdateForm (
               HiiHandle,
               HddIndex
               );

    //
    //  input password is match current set HDD password
    //  return EFI_SUCCESS to display another two password items for new password and confirm new password
    //
    return EFI_SUCCESS;
  }
  HddPasswordScuData[HddIndex].Flag = ENABLE_PASSWORD;


  Status = StoragePasswordUpdateForm (
             HiiHandle,
             HddIndex
             );

  //
  //  If unlock successfully, return EFI_UNSUPPORTED to not change StoragePasswordCallbackState
  //
  return EFI_UNSUPPORTED;

}

EFI_STATUS
SetStoragePassword (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  UINT16                            HddIndex,
  IN  BOOLEAN                           UserOrMaster,
  IN  CHAR16                            *Password
  )
{
  EFI_STATUS                            Status;
  SETUP_UTILITY_CONFIGURATION           *SUCInfo;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;
  EFI_HDD_PASSWORD_SERVICE_PROTOCOL     *HddPasswordService;
  HDD_PASSWORD_SCU_DATA                 *HddPasswordScuData;
  UINTN                                 NumOfHdd;

  SuBrowser = NULL;
  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (Status != EFI_SUCCESS || SuBrowser == NULL) {

    //
    //  Return EFI_NOT_AVAILABLE_YET or EFI_UNSUPPORTED to let browser terminate password flow.
    //
    return EFI_NOT_AVAILABLE_YET;
  }

  SUCInfo            = SuBrowser->SUCInfo;
  HddPasswordService = SUCInfo->HddPasswordService;
  HddPasswordScuData = SUCInfo->HddPasswordScuData;
  NumOfHdd           = SUCInfo->NumOfHdd;

  if (HddPasswordService == NULL || HddPasswordScuData == NULL || NumOfHdd == 0) {
    //
    //  Return EFI_NOT_AVAILABLE_YET or EFI_UNSUPPORTED to let browser terminate password flow.
    //
    return EFI_NOT_AVAILABLE_YET;
  }

  if (Password[0] != 0x00) {
    //
    // Password String is not NULL => Change New Password
    //

    if (UserOrMaster == USER_PSW) {
      //
      // Set flag relative to User password
      //

      HddPasswordScuData[HddIndex].Flag = CHANGE_PASSWORD;
      if (HddPasswordScuData[HddIndex].DisableAllType != MASTER_PSW) {
        HddPasswordScuData[HddIndex].MasterFlag = MASTER_PASSWORD_GRAYOUT;
      }

    } else {
      //
      // Set flag relative to Master password
      //

      HddPasswordScuData[HddIndex].MasterFlag = CHANGE_PASSWORD;

    }
    if (UserOrMaster == USER_PSW) {
      //
      // Store password string to User password buffer
      //

      ZeroMem (
        HddPasswordScuData[HddIndex].InputString,
        ((HDD_PASSWORD_MAX_NUMBER + 1) * sizeof (CHAR16))
        );

      StrCpy (
        HddPasswordScuData[HddIndex].InputString,
        Password
        );

      HddPasswordScuData[HddIndex].StringLength = StrLen (Password);
    } else {
      //
      // Store password string to Master password buffer
      //

      ZeroMem (
        HddPasswordScuData[HddIndex].MasterInputString,
        ((HDD_PASSWORD_MAX_NUMBER + 1) * sizeof (CHAR16))
        );

      StrCpy (
        HddPasswordScuData[HddIndex].MasterInputString,
        Password
        );
    }
  } else {
    //
    // Password String is NULL => Disable password
    //

    HddPasswordScuData[HddIndex].Flag = DISABLE_PASSWORD;
    HddPasswordScuData[HddIndex].MasterFlag = DISABLE_PASSWORD;
    HddPasswordScuData[HddIndex].DisableAllType = NO_ACCESS_PSW;

    ZeroMem (
      HddPasswordScuData[HddIndex].InputString,
      ((HDD_PASSWORD_MAX_NUMBER + 1) * sizeof (CHAR16))
      );

    ZeroMem (
      HddPasswordScuData[HddIndex].MasterInputString,
      ((HDD_PASSWORD_MAX_NUMBER + 1) * sizeof (CHAR16))
      );

  }

  UpdateAllHddPasswordFlag (
    HiiHandle,
    HddPasswordScuData,
    NumOfHdd
    );

  Status = StoragePasswordUpdateForm (
             HiiHandle,
             HddIndex
             );

  //
  //  Return EFI_SUCCESS if password was accepted and saved.
  //
  return EFI_SUCCESS;
}

EFI_STATUS
StoragePasswordCallback (
  IN  EFI_HII_HANDLE                      HiiHandle,
  IN  UINT8                               Type,
  IN  EFI_IFR_TYPE_VALUE                  *Value,
  IN  UINT16                              HddIndex,
  IN  BOOLEAN                             UserOrMaster
  )
{
  EFI_STATUS               Status;
  CHAR16                   *Password;

  if (Type != EFI_IFR_TYPE_STRING) {
    return  EFI_INVALID_PARAMETER;
  }

  if (Value->string == 0) {
    //
    // If Value->String == 0, only need reset the state machine to BROWSER_STATE_VALIDATE_PASSWORD
    //
    mPasswordConfig.StoragePasswordCallbackState = BROWSER_STATE_VALIDATE_PASSWORD;
    return EFI_SUCCESS;
  }

  Password = NULL;
  Password = HiiGetString (HiiHandle, Value->string, NULL);
  //
  // When try to set a new password, user will be chanlleged with old password.
  // The Callback is responsible for validating old password input by user,
  // If Callback return EFI_SUCCESS, it indicates validation pass.
  //
  switch (mPasswordConfig.StoragePasswordCallbackState) {

  case BROWSER_STATE_VALIDATE_PASSWORD:
    Status = ValidateStoragePassword (
               HiiHandle,
               HddIndex,
               UserOrMaster,
               Password
               );
    if (Status == EFI_SUCCESS) {
      mPasswordConfig.StoragePasswordCallbackState = BROWSER_STATE_SET_PASSWORD;
    }
    break;

  case BROWSER_STATE_SET_PASSWORD:
    Status = SetStoragePassword (
               HiiHandle,
               HddIndex,
               UserOrMaster,
               Password
               );
    mPasswordConfig.StoragePasswordCallbackState = BROWSER_STATE_VALIDATE_PASSWORD;
    break;

  default:
    Status = EFI_NOT_AVAILABLE_YET;
    break;
  }

  if (Password != NULL) {
    gBS->FreePool (Password);
  }

  return Status;
}


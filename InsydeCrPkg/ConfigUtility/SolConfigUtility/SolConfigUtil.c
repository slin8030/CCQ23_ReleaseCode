/** @file

  Initial and callback functions for SOL Configuration

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

#include <SolConfigUtil.h>

EFI_HANDLE                                mDriverHandle = NULL;
SOL_CONFIG_UTIL_PRIVATE_DATA              *mPrivateData = NULL;

//[-start-160407-IB14090058-remove]//
//BOOLEAN                                   mFirstBoot = FALSE;
//[-end-160407-IB14090058-remove]//
UINT8                                     mMyNameValue = 0;
CHAR16                  mVariableName[] = SOL_VARSTORE_NAME;
HII_VENDOR_DEVICE_PATH  mHiiVendorDevicePath0 = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    SOL_FORMSET_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Request                A null-terminated Unicode string in
                                 <ConfigRequest> format.
  @param  Progress               On return, points to a character in the Request
                                 string. Points to the string's null terminator if
                                 request was successful. Points to the most recent
                                 '&' before the first failing name/value pair (or
                                 the beginning of the string if the failure is in
                                 the first name/value pair) if the request was not
                                 successful.
  @param  Results                A null-terminated Unicode string in
                                 <ConfigAltResp> format which has all values filled
                                 in for the names in the Request string. String to
                                 be allocated by the called function.

  @retval EFI_SUCCESS            The Results is filled with the requested values.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this
                                 driver.

**/
EFI_STATUS
EFIAPI
ExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                       Status;
  UINTN                            BufferSize;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;
  EFI_STRING                       ConfigRequest;
  EFI_STRING                       ConfigRequestHdr;
  UINTN                            Size;
  BOOLEAN                          AllocatedRequest;
  EFI_STRING                       Value;
  UINTN                            ValueStrLen;
  UINT8                            MyNameValue0;
  CHAR16                           BackupChar;

  DEBUG ((EFI_D_INFO, "SolExtract: Start\n"));

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize the local variables.
  //
  ConfigRequestHdr  = NULL;
  ConfigRequest     = NULL;
  Size              = 0;
  *Progress         = Request;
  AllocatedRequest  = FALSE;

  HiiConfigRouting = mPrivateData->HiiConfigRouting;

  //
  // Get Buffer Storage data from EFI variable.
  // Try to get the current setting from variable.
  //
  BufferSize = sizeof (SOL_CONFIGURATION);
  Status = gRT->GetVariable (
                  mVariableName,
                  &gSolConfigUtilVarstoreGuid,
                  NULL,
                  &BufferSize,
                  &mPrivateData->Configuration
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  if (Request == NULL) {
    //
    // Request is set to NULL, construct full request string.
    //

    //
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    ConfigRequestHdr = HiiConstructConfigHdr (&gSolConfigUtilVarstoreGuid, mVariableName, mPrivateData->DriverHandle);
    Size = (StrLen (ConfigRequestHdr) + 32 + 1) * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (Size);
    ASSERT (ConfigRequest != NULL);
    AllocatedRequest = TRUE;
    UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, (UINT64)BufferSize);
    FreePool (ConfigRequestHdr);
    ConfigRequestHdr = NULL;
  } else {
    //
    // Check routing data in <ConfigHdr>.
    // Note: if only one Storage is used, then this checking could be skipped.
    //
    if (!HiiIsConfigHdrMatch (Request, &gSolConfigUtilVarstoreGuid, NULL)) {
      return EFI_NOT_FOUND;
    }
    //
    // Check whether request for EFI Varstore. EFI varstore get data
    // through hii database, not support in this path.
    //
    if (!HiiIsConfigHdrMatch (Request, &gSolConfigUtilVarstoreGuid, mVariableName)) {
      return EFI_NOT_FOUND;
    }
    //
    // Set Request to the unified request string.
    //
    ConfigRequest = Request;
  }

  if (StrStr (ConfigRequest, L"OFFSET") == NULL) {
    //
    // If requesting Name/Value storage, return value 0.
    //
    //
    // Allocate memory for <ConfigResp>, e.g. Name0=0x11, Name1=0x1234, Name2="ABCD"
    // <Request>   ::=<ConfigHdr>&Name0&Name1&Name2
    // <ConfigResp>::=<ConfigHdr>&Name0=11&Name1=1234&Name2=0041004200430044
    //
    BufferSize = (StrLen (ConfigRequest) + 1 + sizeof (MyNameValue0) * 2 + 1) * sizeof (CHAR16);
    *Results = AllocateZeroPool (BufferSize);
    ASSERT (*Results != NULL);
    StrCpy (*Results, ConfigRequest);
    Value = *Results;

    //
    // Append value of NameValueVar0, type is UINT8
    //
    if ((Value = StrStr (*Results, L"MyNameValue0")) != NULL) {
      MyNameValue0 = 0;

      Value += StrLen (L"MyNameValue0");
      ValueStrLen = ((sizeof (MyNameValue0) * 2) + 1);
      CopyMem (Value + ValueStrLen, Value, StrSize (Value));

      BackupChar = Value[ValueStrLen];
      *Value++   = L'=';
      Value += UnicodeValueToString (
                 Value,
                 PREFIX_ZERO | RADIX_HEX,
                 MyNameValue0,
                 sizeof (MyNameValue0) * 2
                 );
      *Value = BackupChar;
    }
    return EFI_SUCCESS;
  }

  Status = HiiConfigRouting->BlockToConfig (
                               HiiConfigRouting,
                               ConfigRequest,
                               (UINT8 *)&mPrivateData->Configuration,
                               sizeof(SOL_CONFIGURATION),
                               Results,
                               Progress
                               );
  if (!EFI_ERROR (Status)) {
    ConfigRequestHdr = HiiConstructConfigHdr (&gSolConfigUtilVarstoreGuid, mVariableName, mPrivateData->DriverHandle);
  }

  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    FreePool (ConfigRequest);
  }

  if (ConfigRequestHdr != NULL) {
    FreePool (ConfigRequestHdr);
  }
  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  } else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }
  DEBUG ((EFI_D_INFO, "SolExtract: End\n"));
  return EFI_SUCCESS;
}

/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Configuration          A null-terminated Unicode string in <ConfigResp>
                                 format.
  @param  Progress               A pointer to a string filled in with the offset of
                                 the most recent '&' before the first failing
                                 name/value pair (or the beginning of the string if
                                 the failure is in the first name/value pair) or
                                 the terminating NULL if all was successful.

  @retval EFI_SUCCESS            The Results is processed successfully.
  @retval EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this
                                 driver.
**/
EFI_STATUS
EFIAPI
RouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  SOL_CONFIGURATION                              SolConfig = {0};
  EFI_STATUS                                    Status;
  EFI_HII_CONFIG_ROUTING_PROTOCOL               *HiiConfigRouting;
  UINTN                                         BufferSize;

  DEBUG ((EFI_D_INFO, "SolRoutet: Start\n"));

  if (Configuration == NULL || Progress == NULL || This == NULL) {
   return EFI_INVALID_PARAMETER;
  }

  if (!HiiIsConfigHdrMatch(Configuration, &gSolConfigUtilVarstoreGuid, mVariableName)) {
    return EFI_SUCCESS;
  }

  HiiConfigRouting = mPrivateData->HiiConfigRouting;
  BufferSize = sizeof (SOL_CONFIGURATION);
  Status = HiiConfigRouting->ConfigToBlock (
                               HiiConfigRouting,
                               Configuration,
                               (UINT8 *) &SolConfig,
                               &BufferSize,
                               Progress
                               );
  if (EFI_ERROR (Status)) {
    return Status;
  }
//[-start-160104-IB04930000-add]//
  mPrivateData->Configuration.SolEnable = SolConfig.SolEnable;
  mPrivateData->Configuration.UseDhcp   = SolConfig.UseDhcp;
//[-end-160104-IB04930000-add]//
  //
  // Store broswer data Buffer Storage to EFI variable
  //
  Status = CommonSetVariable (
             mVariableName,
             &gSolConfigUtilVarstoreGuid,
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
             sizeof (SOL_CONFIGURATION),
//[-start-160104-IB04930000-modify]//
             &mPrivateData->Configuration
//[-end-160104-IB04930000-modify]//
             );
  DEBUG ((EFI_D_INFO, "SolRoute: End\n"));

  return Status;
}

/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Action                 Specifies the type of action taken by the browser.
  @param  QuestionId             A unique value which is sent to the original
                                 exporting driver so that it can identify the type
                                 of data to expect.
  @param  Type                   The type of value for the question.
  @param  Value                  A pointer to the data being sent to the original
                                 exporting driver.
  @param  ActionRequest          On return, points to the action requested by the
                                 callback function.

  @retval EFI_SUCCESS            The callback successfully handled the action.
  @retval EFI_OUT_OF_RESOURCES   Not enough storage is available to hold the
                                 variable and its data.
  @retval EFI_DEVICE_ERROR       The variable could not be saved.
  @retval EFI_UNSUPPORTED        The specified Action is not supported by the
                                 callback.

**/
EFI_STATUS
EFIAPI
DriverCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_STATUS                                 Status;
  H2O_DIALOG_PROTOCOL                        *H2ODialog;
  EFI_FORM_BROWSER2_PROTOCOL                 *FormBrowser2;
  CHAR16                                     *ConfigRequest;
  UINTN                                      ConfigRequestSize;
  UINT8                                      MyNameValue0;

  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) && (Action != EFI_BROWSER_ACTION_FORM_CLOSE))||
    (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  MyNameValue0 = mMyNameValue + 1;
  mMyNameValue ++;
  Status = gBS->LocateProtocol (&gEfiFormBrowser2ProtocolGuid, NULL, (VOID **) &FormBrowser2);
  if (!EFI_ERROR (Status)) {
    ConfigRequestSize = 0x100;
    ConfigRequest     = AllocateZeroPool (ConfigRequestSize);
    UnicodeSPrint (ConfigRequest, ConfigRequestSize, L"MyNameValue0=%02x", MyNameValue0);
    FormBrowser2->BrowserCallback (
                    FormBrowser2,
                    &ConfigRequestSize,
                    ConfigRequest,
                    FALSE,
                    &gSolFormsetGuid,
                    L"MyNameValue0"
                    );
    FreePool (ConfigRequest);
  }

  Status = EFI_SUCCESS;

  switch (Action) {
  case EFI_BROWSER_ACTION_FORM_OPEN:
    DEBUG ((EFI_D_INFO, "SolCallBack->FORM_OPEN              Qid:0x%x  Typ:0x%x\n",QuestionId, Type));
    Status = EFI_UNSUPPORTED;
    break;

  case EFI_BROWSER_ACTION_CHANGING:
    DEBUG ((EFI_D_INFO, "SolCallBack->CHANGING               Qid:0x%x  Typ:0x%x\n",QuestionId, Type));
    {
      Status = gBS->LocateProtocol (
                      &gH2ODialogProtocolGuid,
                      NULL,
                      (VOID **)&H2ODialog
                      );
      if (EFI_ERROR (Status)) {
        return EFI_UNSUPPORTED;
      }

      switch (QuestionId) {
      //
      // For SOL Callbacks.
      //

      default:
        //
        // Dynamical Label callback.
        //
        SolUpdateEntrys (&mPrivateData->Configuration);
        SolConfigCallback (QuestionId, &mPrivateData->Configuration);
        break;
      }
    }
    break;

  case EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING:
    DEBUG ((EFI_D_INFO, "SolCallBack->DEFAULT_MANUFACTURING  Qid:0x%x  Typ:0x%x\n",QuestionId, Type));
    Status = EFI_UNSUPPORTED;
    break;

  case EFI_BROWSER_ACTION_DEFAULT_STANDARD:
    DEBUG ((EFI_D_INFO, "SolCallBack->DEFAULT_STANDARD       Qid:0x%x  Typ:0x%x\n",QuestionId, Type));
    if (QuestionId == KEY_LOAD_DEFAULT) {
      //
      // Workaround for SCU init stage calling this function.
      //
      SolConfigSetToDefault (&mPrivateData->Configuration);
      SolUpdateEntrys (&mPrivateData->Configuration);
    } else {
      Status = EFI_UNSUPPORTED;
    }
    break;

  case EFI_BROWSER_ACTION_RETRIEVE:
    DEBUG ((EFI_D_INFO, "SolCallBack->RETRIEVE               Qid:0x%x  Typ:0x%x\n",QuestionId, Type));
    if (QuestionId == KEY_LOAD_DEFAULT) {
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_UNSUPPORTED;
    }
    break;

  default:
    DEBUG ((EFI_D_INFO, "SolCallBack->default:0x%x           Qid:0x%x  Typ:0x%x\n",Action ,QuestionId, Type));
    Status = EFI_UNSUPPORTED;
    break;
  }

  return Status;
}

/**
  Unloads the application and its installed protocol.

  @param[in]  ImageHandle       Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
**/
EFI_STATUS
EFIAPI
SetupUnload (
  IN EFI_HANDLE  ImageHandle
  )
{

  ASSERT (mPrivateData != NULL);

  if (mDriverHandle != NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           mDriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mHiiVendorDevicePath0,
           &gEfiHiiConfigAccessProtocolGuid,
           &mPrivateData->ConfigAccess,
           NULL
           );
    mDriverHandle = NULL;
  }

  if (mPrivateData->HiiHandle != NULL) {
    HiiRemovePackages (mPrivateData->HiiHandle);
  }

  FreePool (mPrivateData);
  mPrivateData = NULL;

  return EFI_SUCCESS;
}

/**
  Entering SetupUtility notify function.

  @param  Event                 Event whose notification function is being invoked.
  @param  Context               The pointer to the notification function's context,
                                which is implementation-dependent.

  @retval EFI_SUCESS     This function always complete successfully.

**/
VOID
EFIAPI
SetupUtilityNotify (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  EFI_STATUS    Status;

  //
  // Init Sol scu setting
  //
//[-start-160407-IB14090058-remove]//
//  if (mFirstBoot) {
//    SolConfigSetToDefault (&mPrivateData->Configuration);
//  }
//[-end-160407-IB14090058-remove]//
  Status = SolConfigFormInit (mPrivateData->HiiHandle, &mPrivateData->Configuration);
  Status = CommonSetVariable (
             mVariableName,
             &gSolConfigUtilVarstoreGuid,
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
             sizeof (SOL_CONFIGURATION),
             &mPrivateData->Configuration
             );
  gBS->CloseEvent (Event);
}

/**
  Initialization for the Setup related functions.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to SystemTable.

  @retval EFI_SUCESS     This function always complete successfully.

**/
EFI_STATUS
EFIAPI
SolConfigUtilEntry (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_HII_HANDLE                        HiiHandle;
  EFI_HII_STRING_PROTOCOL               *HiiString;
  EFI_HII_DATABASE_PROTOCOL             *HiiDatabase;
  EFI_HII_CONFIG_ROUTING_PROTOCOL       *HiiConfigRouting;
  CHAR16                                *NewString;
  SOL_CONFIGURATION                     *SolConfig;
  VOID                                  *Registration;
  EFI_EVENT                             Event;
  UINTN                                 BufferSize;
  BOOLEAN                               ActionFlag;
  EFI_STRING                            ConfigRequestHdr;

  //
  // Initialize the local variables.
  //
  ConfigRequestHdr = NULL;
  NewString        = NULL;

  //
  // Initialize driver private data
  //
  mPrivateData = AllocateZeroPool (sizeof (SOL_CONFIG_UTIL_PRIVATE_DATA));
  if (mPrivateData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->Signature = SOL_CONFIG_UTIL_PRIVATE_SIGNATURE;

  mPrivateData->ConfigAccess.ExtractConfig = ExtractConfig;
  mPrivateData->ConfigAccess.RouteConfig = RouteConfig;
  mPrivateData->ConfigAccess.Callback = DriverCallback;

  //
  // Locate Hii Database protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiDatabase = HiiDatabase;

  //
  // Locate HiiString protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiStringProtocolGuid, NULL, (VOID **) &HiiString);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiString = HiiString;

  //
  // Locate ConfigRouting protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &HiiConfigRouting);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiConfigRouting = HiiConfigRouting;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mDriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath0,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mPrivateData->ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  mPrivateData->DriverHandle = mDriverHandle;

  //
  // Publish our HII data
  //
  HiiHandle = HiiAddPackages (
                &gSolFormsetGuid,
                mDriverHandle,
                SolConfigUtilStrings,
                SolConfigUtilVfrBin,
                NULL
                );
  if (HiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->HiiHandle = HiiHandle;

  //
  // Initialize configuration data
  //
  SolConfig = &mPrivateData->Configuration;
  ZeroMem (SolConfig, sizeof (SOL_CONFIGURATION));

  //
  // Try to read NV config EFI variable first
  //
  ConfigRequestHdr = HiiConstructConfigHdr (&gSolConfigUtilVarstoreGuid, mVariableName, mDriverHandle);
  ASSERT (ConfigRequestHdr != NULL);
  if (ConfigRequestHdr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  DEBUG ((EFI_D_ERROR, "ConfigRequestHdr: %s\n", ConfigRequestHdr));

  BufferSize = sizeof (SOL_CONFIGURATION);
  Status = gRT->GetVariable (mVariableName, &gSolConfigUtilVarstoreGuid, NULL, &BufferSize, SolConfig);
  if (EFI_ERROR (Status)) {
//[-start-160407-IB14090058-modify]//
    SolConfigSetToDefault (SolConfig);
//[-end-160407-IB14090058-modify]//
    //
    // Store zero data Buffer Storage to EFI variable
    //
    Status = CommonSetVariable (
               mVariableName,
               &gSolConfigUtilVarstoreGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
               sizeof (SOL_CONFIGURATION),
               SolConfig
               );
    if (EFI_ERROR (Status)) {
      SetupUnload (ImageHandle);
      return Status;
    }
    //
    // EFI variable for NV config doesn't exit, we should build this variable
    // based on default values stored in IFR
    //

    ActionFlag = HiiSetToDefaults (ConfigRequestHdr, EFI_HII_DEFAULT_CLASS_STANDARD);
    if (!ActionFlag) {
      SetupUnload (ImageHandle);
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    // EFI variable does exist and Validate Current Setting
    //

    ActionFlag = HiiValidateSettings (ConfigRequestHdr);
    if (!ActionFlag) {
      SetupUnload (ImageHandle);
      return EFI_INVALID_PARAMETER;
    }
  }
  FreePool (ConfigRequestHdr);

  //
  // When execute Setup Utility application, install HII data
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK - 1,
                  SetupUtilityNotify,
                  NULL,
                  &Event
                  );
  if (!EFI_ERROR (Status)) {
    Status = gBS->RegisterProtocolNotify (
                    &gEfiSetupUtilityApplicationProtocolGuid,
                    Event,
                    &Registration
                    );
  }

  return Status;
}

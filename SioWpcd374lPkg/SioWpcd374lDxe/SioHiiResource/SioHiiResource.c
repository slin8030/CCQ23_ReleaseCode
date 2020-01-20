/** @file

Create variable and SIO page

;******************************************************************************
;* Copyright (c) 2014-2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <SioHiiResource.h>

extern UINT8                      *mDmiTablePtr;
extern SIO_DEVICE_LIST_TABLE      *mTablePtr;
extern EFI_SIO_RESOURCE_FUNCTION  mSioResourceFunction[];

CHAR16            *mSioVariableName = (CHAR16*) PcdGetPtr (PcdSioWpcd374lSetupStr);
EFI_GUID          mSioFormSetGuid   = SIO_WPCD374L_SETUP00_FORMSET_GUID;
BOOLEAN           mFirstBoot        = TRUE;
SIO_PRIVATE_DATA  *mSioPrivateData  = NULL;

HII_VENDOR_DEVICE_PATH  mHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    SIO_WPCD374L_SETUP00_FORMSET_GUID
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

  @param[in]   This              Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]   Request           A null-terminated Unicode string in <ConfigRequest> format.
  @param[out]  Progress          On return, points to a character in the Request
                                 string. Points to the string's null terminator if
                                 request was successful. Points to the most recent
                                 '&' before the first failing name/value pair (or
                                 the beginning of the string if the failure is in
                                 the first name/value pair) if the request was not
                                 successful.
  @param[out]  Results           A null-terminated Unicode string in
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
  EFI_STATUS                          Status;
  UINTN                               BufferSize;
  EFI_STRING                          ConfigRequestHdr;
  EFI_STRING                          ConfigRequest;
  BOOLEAN                             AllocatedRequest;
  UINTN                               Size;
  EFI_HII_CONFIG_ROUTING_PROTOCOL     *HiiConfigRouting;
  SIO_PRIVATE_DATA                    *PrivateData;

  PrivateData = SIO_PRIVATE_FROM_THIS (This);

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Progress = Request;
  if ((Request != NULL) && !HiiIsConfigHdrMatch (Request, &mSioFormSetGuid, mSioVariableName)) {
    return EFI_NOT_FOUND;
  }
  ConfigRequestHdr = NULL;
  ConfigRequest    = NULL;
  AllocatedRequest = FALSE;
  Size             = 0;

  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //
  BufferSize = sizeof (SIO_CONFIGURATION);
  ConfigRequest = Request;
  if ((Request == NULL) || (StrStr (Request, L"OFFSET") == NULL)) {
    //
    // Request has no request element, construct full request string.
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    ConfigRequestHdr = HiiConstructConfigHdr (
                         &mSioFormSetGuid,
                         mSioVariableName,
                         mSioPrivateData->DriverHandle
                         );
    if (ConfigRequestHdr == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    Size = (StrLen (ConfigRequestHdr) + 32 + 1) * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (Size);
    ASSERT (ConfigRequest != NULL);
    AllocatedRequest = TRUE;
    UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, (UINT64)BufferSize);
    gBS->FreePool (ConfigRequestHdr);
  }
  Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &HiiConfigRouting);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = HiiConfigRouting->BlockToConfig (
                               HiiConfigRouting,
                               ConfigRequest,
                               (UINT8 *) &PrivateData->Configuration,
                               BufferSize,
                               Results,
                               Progress
                               );
  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    gBS->FreePool (ConfigRequest);
    ConfigRequest = NULL;
  }
  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  } else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

  return Status;
}

/**
  This function processes the results of changes in configuration.

  @param[in]   This              Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]   Configuration     A null-terminated Unicode string in <ConfigResp>
                                 format.
  @param[out]  Progress          A pointer to a string filled in with the offset of
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
  EFI_STATUS                          Status;
  SIO_PRIVATE_DATA                    *PrivateData;
  UINTN                               BufferSize;

  PrivateData = SIO_PRIVATE_FROM_THIS (This);

  if (Configuration == NULL || Progress == NULL || This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Progress = Configuration;
  if (!HiiIsConfigHdrMatch (Configuration, &mSioFormSetGuid, mSioVariableName)) {
    return EFI_NOT_FOUND;
  }

  *Progress = Configuration + StrLen (Configuration);

  //
  // Convert <ConfigResp> to buffer data by helper function ConfigToBlock()
  //
  BufferSize = sizeof (SIO_CONFIGURATION);
  Status = PrivateData->HiiConfigRouting->ConfigToBlock (
                               PrivateData->HiiConfigRouting,
                               Configuration,
                               (UINT8 *) &PrivateData->Configuration,
                               &BufferSize,
                               Progress
                               );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Store Buffer Storage back to EFI variable
  //
  Status = gRT->SetVariable(
                  mSioVariableName,
                  &mSioFormSetGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  sizeof (SIO_CONFIGURATION),
                  &PrivateData->Configuration
                  );

  return EFI_SUCCESS;
}

/**
  This function processes the results of changes in configuration.

  @param[in]  This               Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  Action             Specifies the type of action taken by the browser.
  @param[in]  QuestionId         A unique value which is sent to the original
                                 exporting driver so that it can identify the type
                                 of data to expect.
  @param[in]  Type               The type of value for the question.
  @param[in]  Value              A pointer to the data being sent to the original
                                 exporting driver.
  @param[out] ActionRequest      On return, points to the action requested by the
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
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN  EFI_BROWSER_ACTION                    Action,
  IN  EFI_QUESTION_ID                       QuestionId,
  IN  UINT8                                 Type,
  IN  EFI_IFR_TYPE_VALUE                    *Value,
  OUT EFI_BROWSER_ACTION_REQUEST            *ActionRequest
  )
{
  EFI_STATUS        Status;
  SIO_PRIVATE_DATA  *PrivateData;

  PrivateData = SIO_PRIVATE_FROM_THIS (This);

  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) && (Action != EFI_BROWSER_ACTION_FORM_CLOSE)) || (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }


  Status = EFI_SUCCESS;
  switch (Action) {
  case EFI_BROWSER_ACTION_RETRIEVE:
  case EFI_BROWSER_ACTION_CHANGING:
    break;

  default:
    Status = EFI_UNSUPPORTED;
    break;
  }

  //
  // All other action return unsupported.
  //
  return Status;
}

/**
  Add dynamic COM items for extensive function.

  @retval EFI_SUCCESS                The function completed successfully.

**/
EFI_STATUS
InitCom (
  )
{
  EFI_STATUS                      Status;
  SIO_DEVICE_LIST_TABLE           *GetByConfiguration;
  UINT8                           Index;
  UINT8                           *DmiTable;
  SIO_EXTENSIVE_TABLE_TYPE2       *Type2Function;

  Index = 0;

  //
  // To define "Device" and "DeviceInstance" for display items
  //    
  GetByConfiguration = (SIO_DEVICE_LIST_TABLE*) &mSioPrivateData->Configuration;

  while (Index < mSioPrivateData->Configuration.AutoUpdateNum) {
    if (GetByConfiguration->Device == COM) {
      DmiTable = mDmiTablePtr;
      FindExtensiveDevice (
        &DmiTable,
        mSioResourceFunction->Instance,
        COM,
        GetByConfiguration->DeviceInstance
        );
      if (DmiTable != NULL) {
        do {
          if (*DmiTable == SUBTYPE2) {
            Type2Function = (SIO_EXTENSIVE_TABLE_TYPE2*)DmiTable;
            if (Type2Function->Enable) {
              Status = CreateComTypeOptionOpCode (
                         SIO_COM1_LABEL + GetByConfiguration->DeviceInstance,
                         Type2Function->Setting
                         );
              break;
            }
          }
          DmiTable += *(UINT8*)(DmiTable + 1);
        } while ((*DmiTable != 0) && (*DmiTable != 0xFF));
      }
    }
    GetByConfiguration++;
    Index++;
  }

  return EFI_SUCCESS;
}

/**
  Create OP code for COM type.

  @param[in]   Label             Update information starting immediately after this label in the IFR

**/
EFI_STATUS
CreateComTypeOptionOpCode (
  UINT16 Label,
  UINT8  SettingDefault
  )
{
  UINT8                           Rs232TypeDefault;
  UINT8                           Rs485TypeDefault;
  EFI_STATUS                      Status;
  EFI_IFR_GUID_LABEL              *StartLabel;
  VOID                            *StartOpCodeHandle;
  VOID                            *OptionsOpCodeHandle;
  UINT16                          Offset;
  EFI_QUESTION_ID                 QuestionId;

  Offset = 0;
  QuestionId = 0;
  Rs232TypeDefault = 0;
  Rs485TypeDefault = 0;
  Status = EFI_UNSUPPORTED;

  //
  // Initialize the container for dynamic opcodes
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);

  OptionsOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT ( OptionsOpCodeHandle != NULL);

  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = Label;

  if (SettingDefault == 0) {
    Rs232TypeDefault = EFI_IFR_OPTION_DEFAULT;
  } else {
    Rs485TypeDefault = EFI_IFR_OPTION_DEFAULT;
  }

  HiiCreateOneOfOptionOpCode (
    OptionsOpCodeHandle,
    STRING_TOKEN(STR_SIO_COM_RS232),
    Rs232TypeDefault,
    EFI_IFR_NUMERIC_SIZE_1,
    0
    );

  HiiCreateOneOfOptionOpCode (
    OptionsOpCodeHandle,
    STRING_TOKEN(STR_SIO_COM_RS485),
    Rs485TypeDefault,
    EFI_IFR_NUMERIC_SIZE_1,
    1
    );

  switch (Label) {
  case SIO_COM1_LABEL:
    Offset = VAR_OFFSET(Com1Type);
    QuestionId = SIO_COM1_UPDATE_QUESTION_ID;
    break;
  case SIO_COM2_LABEL:
    Offset = VAR_OFFSET(Com2Type);
    QuestionId = SIO_COM2_UPDATE_QUESTION_ID;
    break;
  case SIO_COM3_LABEL:
    Offset = VAR_OFFSET(Com3Type);
    QuestionId = SIO_COM3_UPDATE_QUESTION_ID;
    break;
  case SIO_COM4_LABEL:
    Offset = VAR_OFFSET(Com4Type);
    QuestionId = SIO_COM4_UPDATE_QUESTION_ID;
    break;
  case SIO_COM5_LABEL:
    Offset = VAR_OFFSET(Com5Type);
    QuestionId = SIO_COM5_UPDATE_QUESTION_ID;
    break;
  case SIO_COM6_LABEL:
    Offset = VAR_OFFSET(Com6Type);
    QuestionId = SIO_COM6_UPDATE_QUESTION_ID;
    break;
  default:
    break;
  }

  if (Offset != 0) {
    HiiCreateOneOfOpCode (
      StartOpCodeHandle,
      QuestionId,
      SIO_CONFIGURATION_VARSTORE_ID,
      Offset,
      STRING_TOKEN (STR_SIO_COM_TYPE),
      STRING_TOKEN (STR_SIO_COM_TYPE_HELP),
      EFI_IFR_FLAG_CALLBACK,
      EFI_IFR_NUMERIC_SIZE_1,
      OptionsOpCodeHandle,
      NULL
      );

    Status = HiiUpdateForm (
               mSioPrivateData->HiiHandle,         // HII handle
               &mSioFormSetGuid,                   // Formset GUID
               SIO_CONFIGURATION_FORM_ID,          // Form ID
               StartOpCodeHandle,                  // Label for where to insert opcodes
               NULL                                // Replace data
               );
    ASSERT_EFI_ERROR(Status);
  }

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (OptionsOpCodeHandle);

  return Status;
}

/**
  Add packlist to Create Setup page.

  @retval EFI_SUCCESS                The function completed successfully.
**/
EFI_STATUS
EFIAPI
CreateSetupPage (
  VOID
  )
{
  EFI_STATUS                          Status;
  BOOLEAN                             ActionFlag;
  UINTN                               BufferSize;
  EFI_STRING                          ConfigRequestHdr;
  SIO_DEVICE_LIST_TABLE               *PcdPointer;
  SIO_DEVICE_LIST_TABLE               *GetByConfiguration;

  ConfigRequestHdr = NULL;

  //
  // Initialize driver private data
  //
  mSioPrivateData = AllocateZeroPool (sizeof (SIO_PRIVATE_DATA));
  if (mSioPrivateData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mSioPrivateData->Signature = SIO_PRIVATE_SIGNATURE;
  mSioPrivateData->ConfigAccess.ExtractConfig = ExtractConfig;
  mSioPrivateData->ConfigAccess.RouteConfig = RouteConfig;
  mSioPrivateData->ConfigAccess.Callback = DriverCallback;

  //
  // Locate ConfigRouting protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiConfigRoutingProtocolGuid,
                  NULL,
                  (VOID **)&mSioPrivateData->HiiConfigRouting
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Install Device Path Protocol and Config Access protocol to driver handle
  // Install Platform Driver Override Protocol to driver handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mSioPrivateData->DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mSioPrivateData->ConfigAccess,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mSioPrivateData->HiiHandle = HiiAddPackages (
                                 &mSioFormSetGuid,
                                 mSioPrivateData->DriverHandle,
                                 SioHiiResourceVfrBin,
                                 SioWpcd374lDxeStrings,
                                 NULL
                                 );
  if (mSioPrivateData->HiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize configuration data
  //
  ZeroMem (&mSioPrivateData->Configuration, sizeof (SIO_CONFIGURATION));

  //
  // Try to read NV config EFI variable first
  //
  ConfigRequestHdr = HiiConstructConfigHdr (
                       &mSioFormSetGuid,
                       mSioVariableName,
                       mSioPrivateData->DriverHandle
                       );
  ASSERT (ConfigRequestHdr != NULL);

  BufferSize = sizeof (SIO_CONFIGURATION);
  Status = gRT->GetVariable (
                  mSioVariableName,
                  &mSioFormSetGuid,
                  NULL,
                  &BufferSize,
                  &mSioPrivateData->Configuration
                  );
  if (EFI_ERROR (Status)) {

    //
    // Store zero data Buffer Storage to EFI variable
    //
    Status = gRT->SetVariable(
                    mSioVariableName,
                    &mSioFormSetGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    sizeof (SIO_CONFIGURATION),
                    &mSioPrivateData->Configuration
                    );
    ASSERT_EFI_ERROR (Status);

    //
    // EFI variable for NV config doesn't exit, we should build this variable
    // based on default values stored in IFR
    //
    ActionFlag = HiiSetToDefaults (ConfigRequestHdr, EFI_HII_DEFAULT_CLASS_STANDARD);
    ASSERT (ActionFlag);

    //
    // To define "Device" and "DeviceInstance" for display items
    //
    PcdPointer = mTablePtr;
    GetByConfiguration = (SIO_DEVICE_LIST_TABLE*)&mSioPrivateData->Configuration;
    while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
      if ((PcdPointer->Device == COM) || (PcdPointer->Device == LPT) || (PcdPointer->Device == WDT) ||
          (PcdPointer->Device == CIR) || (PcdPointer->Device == IR)) {
        if (PcdPointer->DeviceEnable == TRUE) {
          GetByConfiguration->Device = PcdPointer->Device;
          GetByConfiguration->DeviceInstance = PcdPointer->DeviceInstance;
        }
        GetByConfiguration++;
        mSioPrivateData->Configuration.AutoUpdateNum++;
      }

      PcdPointer++;
    }

    //
    // Store items that we want to display
    //
    Status = gRT->SetVariable(
                    mSioVariableName,
                    &mSioFormSetGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    sizeof (SIO_CONFIGURATION),
                    &mSioPrivateData->Configuration
                    );
    ASSERT_EFI_ERROR (Status);
  } else {
    mFirstBoot = FALSE;
    //
    // EFI variable does exist and Validate Current Setting
    //
    ActionFlag = HiiValidateSettings (ConfigRequestHdr);
    ASSERT (ActionFlag);
  }

  InitCom ();

  FreePool (ConfigRequestHdr);

  return Status;
}


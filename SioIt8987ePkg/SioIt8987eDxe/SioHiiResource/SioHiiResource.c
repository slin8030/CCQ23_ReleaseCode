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

CHAR16            *mSioVariableName = (CHAR16*) PCD_SIO_VARIABLE_NAME;
EFI_GUID          mSioFormSetGuid   = SIO_IT8987E_SETUP00_FORMSET_GUID;
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
    SIO_IT8987E_SETUP00_FORMSET_GUID
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

UINT8 *
EFIAPI
InternalHiiCreateOpCodeExtended (
  IN VOID   *OpCodeHandle,
  IN VOID   *OpCodeTemplate,
  IN UINT8  OpCode,
  IN UINTN  OpCodeSize,
  IN UINTN  ExtensionSize,
  IN UINT8  Scope
  );

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
  if ((Request != NULL) && !HiiIsConfigHdrMatch (Request, &mSioFormSetGuid, mSioVariableName)){
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

  if (HiiIsConfigHdrMatch (Request, &mSioFormSetGuid, mSioVariableName) || (Request == NULL)) {
    BufferSize = sizeof (SIO_CONFIGURATION);
    Status = HiiConfigRouting->BlockToConfig (
                                 HiiConfigRouting,
                                 ConfigRequest,
                                 (UINT8 *) &PrivateData->Configuration,
                                 BufferSize,
                                 Results,
                                 Progress
                                 );
  }

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
  UINT8                               Index, Value;

  PrivateData = SIO_PRIVATE_FROM_THIS (This);
  Index         = 0;
  Value         = 0;

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
  EFI_STATUS             Status;
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

VOID
InitDynamicSetupOptions (
  )
{
  UINT8                      *ExtensiveTable;
  SIO_DEVICE_LIST_TABLE      *DeviceResource;
  SIO_DEVICE_FIFO_SETTING    *FifoSetting;
  SIO_ACPI_IRQ_INFORMATION   *AcpiIrqInfo;
  SIO_DEVICE_MODE_SELECTION  *DeviceModeSelection;
#if FixedPcdGetBool(PcdSioIt8987eEnableLedSetup)
  SIO_IDENTIFY_DEVICE        *IdentifyDevice = NULL;
  SIO_LED_SETTING            *LedSetting;
  SIO_LED_ADDITIONAL_FEATURE_SETTING *AdditionalFeatureSetting;
  SIO_FADING_LED_SETTING     *FadingLedSetting;
#endif

  DeviceResource = (SIO_DEVICE_LIST_TABLE*)mTablePtr;

  while ((DeviceResource->TypeH != NONE_ID) || (DeviceResource->TypeL != NONE_ID)) {
    ExtensiveTable = mExtensiveTablePtr;
    FindExtensiveDevice (
      &ExtensiveTable,
      mSioResourceFunction->Instance,
      DeviceResource->Device,
      DeviceResource->DeviceInstance
      );

    if (ExtensiveTable != NULL) {
      switch (DeviceResource->Device) {
      case COM:
        do {
          ExtensiveTable += *(ExtensiveTable + 1);
          switch (*ExtensiveTable) {
          case MODE_SELECTION_TYPE:
            DeviceModeSelection = (SIO_DEVICE_MODE_SELECTION*)ExtensiveTable;
            if (DeviceModeSelection->Enable) {
              CreateComModeOptionOpCode (
                COM1_MODE_LABEL + DeviceResource->DeviceInstance,
                DeviceModeSelection
                );
            }
            break;

          case FIFO_SETTING_TYPE:
            FifoSetting = (SIO_DEVICE_FIFO_SETTING*)ExtensiveTable;
            if (FifoSetting->Enable) {
              CreateComFifoOptionOpCode (
                COM1_FIFO_LABEL + DeviceResource->DeviceInstance,
                FifoSetting
                );
            }
            break;

          case ACPI_IRQ_INFO_TYPE:
            AcpiIrqInfo = (SIO_ACPI_IRQ_INFORMATION*)ExtensiveTable;
            if (AcpiIrqInfo->Enable) {
              CreateComAcpiIrqOptionOpCode (
                COM1_ACPI_IRQ_LABEL + DeviceResource->DeviceInstance,
                DeviceResource->DeviceInstance,
                AcpiIrqInfo
                );
            }
            break;

          default:
            break;
          }
        } while ((*ExtensiveTable != DEVICE_EXTENSIVE_RESOURCE) && (*(UINT32*)ExtensiveTable != EXTENSIVE_TABLE_ENDING));
        break;

      default:
        break;
      }
    }

    DeviceResource++;
  }
}

/**
  Create OP code for COM type.

  @param[in]   Label             Update information starting immediately after this label in the IFR

**/
VOID
CreateComModeOptionOpCode (
  UINT16                     Label,
  SIO_DEVICE_MODE_SELECTION  *DeviceModeSelection
  )
{
  UINT8               Index;
  UINT8               DefaultValue;
  UINT16              Offset;
  UINT16              StringId;
  BOOLEAN             OwnedModeSupport;
  EFI_STATUS          Status;
  EFI_QUESTION_ID     QuestionId;
  EFI_IFR_GUID_LABEL  *StartLabel;
  VOID                *StartOpCodeHandle;
  VOID                *OptionsOpCodeHandle;

  StringId   = 0;
  Offset     = 0;
  QuestionId = 0;
  Status     = EFI_UNSUPPORTED;

  //
  // Initialize the container for dynamic opcodes
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);

  OptionsOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (OptionsOpCodeHandle != NULL);

  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartLabel = (EFI_IFR_GUID_LABEL *)HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = Label;

  for (Index = 0; Index < 3; Index++) {
    DefaultValue = 0;
    OwnedModeSupport   = FALSE;
    if (DeviceModeSelection->OwnedMode & (BIT0 << Index)) {
      OwnedModeSupport = TRUE;
      switch (Index) {
      case 0:
        StringId  = STRING_TOKEN(STR_SIO_COM_RS232);
        break;
      case 1:
        StringId = STRING_TOKEN(STR_SIO_COM_RS485);
        break;
      case 2:
        StringId = STRING_TOKEN(STR_SIO_COM_RS422);
        break;
      default:
        break;
      }
      if (DeviceModeSelection->Setting & (BIT0 << Index)) {
        DefaultValue = EFI_IFR_OPTION_DEFAULT;
      }
      if (OwnedModeSupport) {
        HiiCreateOneOfOptionOpCode (
          OptionsOpCodeHandle,
          StringId,
          DefaultValue,
          EFI_IFR_NUMERIC_SIZE_1,
          (UINT64)Index
          );
      }
    }
  }

  switch (Label) {
  case COM1_MODE_LABEL:
    Offset = VAR_OFFSET (Com1Mode);
    QuestionId = COM_MODE_QUESTION_ID;
    break;
  case COM2_MODE_LABEL:
    Offset = VAR_OFFSET (Com2Mode);
    QuestionId = COM_MODE_QUESTION_ID + 1;
    break;
  default:
    break;
  }

  ASSERT (Offset != 0);
  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    QuestionId,
    SIO_CONFIGURATION_VARSTORE_ID,
    Offset,
    STRING_TOKEN (STR_SIO_MODE_SELECTION),
    STRING_TOKEN (STR_SIO_COM_MODE_HELP),
    0,
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
  ASSERT_EFI_ERROR (Status);

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (OptionsOpCodeHandle);
}

/**
  Create OP code for COM FIFO.

  @param[in]   Label             Update information starting immediately after this label in the IFR

**/
VOID
CreateComFifoOptionOpCode (
  UINT16                   Label,
  SIO_DEVICE_FIFO_SETTING  *FifoSetting
  )
{
  UINT8                           Index;
  UINT8                           DefaultValue;
  UINT16                          StringId;
  UINT16                          EqualList;
  UINT16                          Offset;
  EFI_STATUS                      Status;
  EFI_QUESTION_ID                 QuestionId;
  EFI_IFR_GUID_LABEL              *StartLabel;
  VOID                            *StartOpCodeHandle;
  VOID                            *OptionsOpCodeHandle;

  StringId   = 0;
  EqualList  = 0;
  QuestionId = 0;
  Offset     = 0;
  Status     = EFI_UNSUPPORTED;

  //
  // Initialize the container for dynamic opcodes
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);

  OptionsOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (OptionsOpCodeHandle != NULL);

  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartLabel = (EFI_IFR_GUID_LABEL *)HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = Label;

  for (Index = 0; Index < 2; Index++) {
    DefaultValue = EFI_IFR_OPTION_DEFAULT;
    if (Index == 0) {
      StringId = STRING_TOKEN(STR_SIO_SELECT_DISABLE);
      if (FifoSetting->SetupDefault == 1) DefaultValue = 0;
    }
    if (Index == 1) {
      StringId = STRING_TOKEN(STR_SIO_SELECT_ENABLE);
      if (FifoSetting->SetupDefault == 0) DefaultValue = 0;
    }
    HiiCreateOneOfOptionOpCode (
      OptionsOpCodeHandle,
      StringId,
      DefaultValue,
      EFI_IFR_NUMERIC_SIZE_1,
      (UINT64)Index
      );
  }

  switch (Label) {
  case COM1_FIFO_LABEL:
    Offset     = VAR_OFFSET (Com1Fifo);
    QuestionId = COM_FIFO_QUESTION_ID;
    break;
  case COM2_FIFO_LABEL:
    Offset     = VAR_OFFSET (Com2Fifo);
    QuestionId = COM_FIFO_QUESTION_ID + 1;
    break;
  default:
    break;
  }

  ASSERT (Offset != 0);
  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    QuestionId,
    SIO_CONFIGURATION_VARSTORE_ID,
    (Offset + FIFO_SETTING_OFFSET (Enable)),
    STRING_TOKEN (STR_SIO_FIFO),
    STRING_TOKEN (STR_SIO_FIFO_HELP),
    0,
    EFI_IFR_NUMERIC_SIZE_1,
    OptionsOpCodeHandle,
    NULL
    );

  HiiFreeOpCodeHandle (OptionsOpCodeHandle);
  OptionsOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (OptionsOpCodeHandle != NULL);
  HiiCreateSuppressIfOpCodeEx (StartOpCodeHandle, 1);
  HiiCreateIdEqualListOpCodeEx (StartOpCodeHandle, QuestionId, 1, &EqualList);

  for (Index = 0; Index < 16; Index++) {
    if (((FifoSetting->LevelSupported & 0x01) == 0) && (Index >= 4)) break;
    DefaultValue = 0;

    switch (Index) {
    case 0:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_1_BYTE);
      break;
    case 1:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_4_BYTE);
      break;
    case 2:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_8_BYTE);
      break;
    case 3:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_14_BYTE);
      break;
    case 4:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_16_BYTE);
      break;
    case 5:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_32_BYTE);
      break;
    case 6:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_48_BYTE);
      break;
    case 7:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_64_BYTE);
      break;
    case 8:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_80_BYTE);
      break;
    case 9:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_88_BYTE);
      break;
    case 10:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_96_BYTE);
      break;
    case 11:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_104_BYTE);
      break;
    case 12:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_112_BYTE);
      break;
    case 13:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_116_BYTE);
      break;
    case 14:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_120_BYTE);
      break;
    case 15:
      StringId = STRING_TOKEN(STR_SIO_FIFO_LEVEL_124_BYTE);
      break;
    default:
      break;
    }

    if (FifoSetting->Level == Index) {
      DefaultValue = EFI_IFR_OPTION_DEFAULT;
    }

    HiiCreateOneOfOptionOpCode (
      OptionsOpCodeHandle,
      StringId,
      DefaultValue,
      EFI_IFR_NUMERIC_SIZE_1,
      (UINT64)Index
      );
  }

  if ((FifoSetting->LevelSupported & 0x02) != 0) {
    if (FifoSetting->Level == 0x10) {
      DefaultValue = EFI_IFR_OPTION_DEFAULT;
    } else {
      DefaultValue = 0;
    }
    HiiCreateOneOfOptionOpCode (
      OptionsOpCodeHandle,
      STRING_TOKEN(STR_SIO_FIFO_LEVEL_128_BYTE),
      DefaultValue,
      EFI_IFR_NUMERIC_SIZE_1,
      16
      );
  }

  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    (QuestionId + 6),
    SIO_CONFIGURATION_VARSTORE_ID,
    (Offset + FIFO_SETTING_OFFSET (Level)),
    STRING_TOKEN (STR_SIO_FIFO_LEVEL),
    STRING_TOKEN (STR_SIO_FIFO_LEVEL_HELP),
    0,
    EFI_IFR_NUMERIC_SIZE_1,
    OptionsOpCodeHandle,
    NULL
    );

  HiiCreateEndOpCode (StartOpCodeHandle);

  Status = HiiUpdateForm (
             mSioPrivateData->HiiHandle,         // HII handle
             &mSioFormSetGuid,                   // Formset GUID
             SIO_CONFIGURATION_FORM_ID,          // Form ID
             StartOpCodeHandle,                  // Label for where to insert opcodes
             NULL                                // Replace data
             );
  ASSERT_EFI_ERROR(Status);

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (OptionsOpCodeHandle);
}

/**
  Create OP code for ACPI IRQ.

  @param[in]   Label           Update information starting immediately after this label in the IFR

**/
VOID
CreateComAcpiIrqOptionOpCode (
  UINT16                    Label,
  UINT8                     DeviceInstance,
  SIO_ACPI_IRQ_INFORMATION  *AcpiIrqInfo
  )
{
  UINT8               Index;
  UINT8               DefaultValue;
  UINT16              Offset;
  UINT16              StringId;
  EFI_STATUS          Status;
  EFI_QUESTION_ID     QuestionId;
  EFI_IFR_GUID_LABEL  *StartLabel;
  VOID                *StartOpCodeHandle;
  VOID                *OptionsOpCodeHandle;

  StringId   = 0;
  Offset     = 0;
  QuestionId = 0;
  Status     = EFI_UNSUPPORTED;

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
  StartLabel = (EFI_IFR_GUID_LABEL *)HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = Label;

  for (Index = 0; Index < 2; Index++) {
    DefaultValue = 0;
    switch (Index) {
    case 0:
      StringId  = STRING_TOKEN(STR_SIO_LEVEL_TRIGGERED);
      break;
    case 1:
      StringId = STRING_TOKEN(STR_SIO_EDGE_TRIGGERED);
      break;
    default:
      break;
    }
    if ((((AcpiIrqInfo->IrqInfo & BIT0) != 0) && Index == 1) || (((AcpiIrqInfo->IrqInfo & BIT0) == 0) && Index == 0)) {
      DefaultValue = EFI_IFR_OPTION_DEFAULT;
    }
    HiiCreateOneOfOptionOpCode (
      OptionsOpCodeHandle,
      StringId,
      DefaultValue,
      EFI_IFR_NUMERIC_SIZE_1,
      (UINT64)Index
      );
  }

  Offset = VAR_OFFSET (ComIrq[DeviceInstance]);
  QuestionId = COM_ACPI_IRQ_QUESTION_ID + DeviceInstance;

  ASSERT (Offset != 0);
  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    QuestionId,
    SIO_CONFIGURATION_VARSTORE_ID,
    (Offset + IRQ_SETTING_OFFSET (IntMode)),
    STRING_TOKEN (STR_SIO_INTERRUPT_MODE_SELECTION),
    STRING_TOKEN (STR_SIO_INTERRUPT_MODE_HELP),
    0,
    EFI_IFR_NUMERIC_SIZE_1,
    OptionsOpCodeHandle,
    NULL
    );

  HiiFreeOpCodeHandle (OptionsOpCodeHandle);
  OptionsOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (OptionsOpCodeHandle != NULL);

  for (Index = 0; Index < 2; Index++) {
    DefaultValue = 0;
    switch (Index) {
    case 0:
      StringId = STRING_TOKEN(STR_SIO_ACTIVE_HIGH);
      break;
    case 1:
      StringId = STRING_TOKEN(STR_SIO_ACTIVE_LOW);
      break;
    default:
      break;
    }
    if ((((AcpiIrqInfo->IrqInfo & BIT3) != 0) && Index == 1) || (((AcpiIrqInfo->IrqInfo & BIT3) == 0) && Index == 0)) {
      DefaultValue = EFI_IFR_OPTION_DEFAULT;
    }
    HiiCreateOneOfOptionOpCode (
      OptionsOpCodeHandle,
      StringId,
      DefaultValue,
      EFI_IFR_NUMERIC_SIZE_1,
      (UINT64)Index
      );
  }

  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    (QuestionId + 6),
    SIO_CONFIGURATION_VARSTORE_ID,
    (Offset + IRQ_SETTING_OFFSET (InterruptPolarity)),
    STRING_TOKEN (STR_SIO_INTERRUPT_POLARITY_SELECTION),
    STRING_TOKEN (STR_SIO_INTERRUPT_POLARITY_HELP),
    0,
    EFI_IFR_NUMERIC_SIZE_1,
    OptionsOpCodeHandle,
    NULL
    );

  HiiFreeOpCodeHandle (OptionsOpCodeHandle);
  OptionsOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (OptionsOpCodeHandle != NULL);

  for (Index = 0; Index < 2; Index++) {
    DefaultValue = 0;
    switch (Index) {
    case 0:
      StringId  = STRING_TOKEN(STR_SIO_INTERRUPT_EXCLUSIVE);
      break;
    case 1:
      StringId = STRING_TOKEN(STR_SIO_INTERRUPT_SHARED);
      break;
    default:
      break;
    }
    if ((((AcpiIrqInfo->IrqInfo & BIT4) != 0) && Index == 1) || (((AcpiIrqInfo->IrqInfo & BIT4) == 0) && Index == 0)) {
      DefaultValue = EFI_IFR_OPTION_DEFAULT;
    }
    HiiCreateOneOfOptionOpCode (
      OptionsOpCodeHandle,
      StringId,
      DefaultValue,
      EFI_IFR_NUMERIC_SIZE_1,
      (UINT64)Index
      );
  }

  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    (QuestionId + 12),
    SIO_CONFIGURATION_VARSTORE_ID,
    (Offset + IRQ_SETTING_OFFSET (IntShare)),
    STRING_TOKEN (STR_SIO_INTERRUPT_SHARING_SELECTION),
    STRING_TOKEN (STR_SIO_INTERRUPT_SHARING_HELP),
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
  ASSERT_EFI_ERROR (Status);

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (OptionsOpCodeHandle);
}

UINT8 *
HiiCreateIdEqualOpCodeEx2 (
  IN VOID                    *OpCodeHandle,
  IN EFI_QUESTION_ID         QuestionId,
  IN UINT16                  Value,
  IN UINT8                   Scope
  )
{
  EFI_IFR_EQ_ID_VAL          OpCode;

  OpCode.QuestionId = QuestionId;
  OpCode.Value = Value;

  return InternalHiiCreateOpCodeExtended (
           OpCodeHandle,
           &OpCode,
           EFI_IFR_EQ_ID_VAL_OP,
           sizeof (OpCode),
           0,
           Scope
           );
}

UINT8 *
HiiCreateIdEqualListOpCodeEx2 (
  IN VOID                    *OpCodeHandle,
  IN EFI_QUESTION_ID         QuestionId,
  IN UINT8                   ListLength,
  IN UINT16                  *ValueList,
  IN UINT8                   Scope
  )
{
  EFI_IFR_EQ_ID_VAL_LIST     *OpCode;
  UINTN                      OpCodeSize;
  UINT8                      *OpCodePtr;
  UINT8                      Index;

  OpCodeSize = sizeof (EFI_IFR_EQ_ID_VAL_LIST) + sizeof (UINT16) * (ListLength - 1);
  OpCode = AllocatePool (OpCodeSize);
  if (OpCode == NULL) {
    return NULL;
  }

  OpCode->QuestionId = QuestionId;
  OpCode->ListLength = ListLength;
  for (Index = 0; Index < ListLength; Index++) {
    OpCode->ValueList[Index] = ValueList[Index];
  }

  OpCodePtr = InternalHiiCreateOpCodeExtended (
             OpCodeHandle,
             OpCode,
             EFI_IFR_EQ_ID_VAL_LIST_OP,
             OpCodeSize,
             0,
             Scope
             );
  FreePool (OpCode);

  return OpCodePtr;
}

/**
  Create EFI_IFR_REFRESH_OP opcode.

  @param[in]  StartOpCodeHandle  Handle to the buffer of opcodes.

  @retval NULL   There is not enough space left in Buffer to add the opcode.
  @retval Other  A pointer to the created opcode.

**/
EFI_STATUS
CreateRefreshOpCode (
  IN VOID             *StartOpCodeHandle
  )
{
  EFI_IFR_REFRESH            Refresh;
  HII_LIB_OPCODE_BUFFER      *OpCodeHandleBuffer;
  UINT8                      *LocalBuffer;

  if (StartOpCodeHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Refresh.Header.Length   = sizeof (EFI_IFR_REFRESH);
  Refresh.Header.OpCode   = EFI_IFR_REFRESH_OP;
  Refresh.Header.Scope    = 0;
  Refresh.RefreshInterval = 1;

  OpCodeHandleBuffer = (HII_LIB_OPCODE_BUFFER *)StartOpCodeHandle;
  LocalBuffer = (UINT8 *) OpCodeHandleBuffer->Buffer + OpCodeHandleBuffer->Position;
  CopyMem (LocalBuffer, &Refresh, sizeof (EFI_IFR_REFRESH));

  OpCodeHandleBuffer->Position+= sizeof (EFI_IFR_REFRESH);

  return EFI_SUCCESS;
}

VOID
SetupValueInit (
  SIO_DEVICE_LIST_TABLE  *PcdResource
  )
{
  UINT8                      *ExtensiveTable;
  UINT8                      *VariableAddress;
  SIO_FIFO_SETTING           *FifoVariable;
  SIO_ACPI_IRQ_INFORMATION   *AcpiIrqInfo;
  SIO_DEVICE_FIFO_SETTING    *FifoSetting;
  SIO_DEVICE_MODE_SELECTION  *DeviceModeSelection;

  ExtensiveTable  = mExtensiveTablePtr;
  VariableAddress = NULL;
  FifoVariable    = NULL;

  FindExtensiveDevice (
    &ExtensiveTable,
    PcdResource->TypeInstance,
    PcdResource->Device,
    PcdResource->DeviceInstance
    );
  if (ExtensiveTable == NULL) return;

  switch (PcdResource->Device) {
  case COM:
    switch (PcdResource->DeviceInstance) {
    case 0:
      VariableAddress = &mSioPrivateData->Configuration.Com1Mode;
      FifoVariable    = &mSioPrivateData->Configuration.Com1Fifo;
      break;
    case 1:
      VariableAddress = &mSioPrivateData->Configuration.Com2Mode;
      FifoVariable    = &mSioPrivateData->Configuration.Com2Fifo;
      break;
    default:
      break;
    }
    do {
      ExtensiveTable += *(ExtensiveTable + 1);
      switch (*ExtensiveTable) {
      case MODE_SELECTION_TYPE:
        DeviceModeSelection = (SIO_DEVICE_MODE_SELECTION*)ExtensiveTable;
        switch (DeviceModeSelection->Setting) {
        case BIT0:
          *VariableAddress = 0;
          break;
        case BIT1:
          *VariableAddress = 1;
          break;
        case BIT2:
          *VariableAddress = 2;
          break;
        default:
          break;
        }
        break;

      case ACPI_IRQ_INFO_TYPE:
        AcpiIrqInfo = (SIO_ACPI_IRQ_INFORMATION*)ExtensiveTable;
        if ((AcpiIrqInfo->IrqInfo & BIT3) != 0) {
          mSioPrivateData->Configuration.ComIrq[PcdResource->DeviceInstance].InterruptPolarity = 1;
        }
        if ((AcpiIrqInfo->IrqInfo & BIT0) != 0) {
          mSioPrivateData->Configuration.ComIrq[PcdResource->DeviceInstance].IntMode = 1;
        }
        if ((AcpiIrqInfo->IrqInfo & BIT4) != 0) {
          mSioPrivateData->Configuration.ComIrq[PcdResource->DeviceInstance].IntShare = 1;
        }
        break;

      case FIFO_SETTING_TYPE:
        FifoSetting = (SIO_DEVICE_FIFO_SETTING*)ExtensiveTable;
        FifoVariable->Enable = FifoSetting->SetupDefault;
        FifoVariable->Level  = FifoSetting->Level;
        break;

      default:
        break;
      }
    } while ((*ExtensiveTable != DEVICE_EXTENSIVE_RESOURCE) && (*(UINT32*)ExtensiveTable != EXTENSIVE_TABLE_ENDING));
    break;

  default:
    break;
  }
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
                                 SioIt8987eDxeStrings,
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
  if (EFI_ERROR (Status) || ((!EFI_ERROR (Status)) && (mSioPrivateData->Configuration.NotFirstBoot == 0))) {

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
      if ((PcdPointer->Device == COM) || (PcdPointer->Device == CIR) || (PcdPointer->Device == IR)) {
        if (PcdPointer->DeviceEnable == TRUE) {
          GetByConfiguration->Device = PcdPointer->Device;
          GetByConfiguration->DeviceInstance = PcdPointer->DeviceInstance;
        }

        GetByConfiguration++;
        mSioPrivateData->Configuration.AutoUpdateNum++;
      }

      SetupValueInit (PcdPointer);

      PcdPointer++;
    }

    mSioPrivateData->Configuration.NotFirstBoot = 1;

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

  InitDynamicSetupOptions ();

  FreePool (ConfigRequestHdr);

  return Status;
}


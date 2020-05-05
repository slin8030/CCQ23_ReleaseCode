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

SIO_MAP_QUESTION_ID mItemList = {{(LIST_ENTRY *) NULL, (LIST_ENTRY *) NULL}, 0, 0};

CHAR16            *mSioVariableName = (CHAR16*) PCD_SIO_VARIABLE_NAME;
EFI_GUID          mSioFormSetGuid   = SIO_NCT5104D_SETUP00_FORMSET_GUID;
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
    SIO_NCT5104D_SETUP00_FORMSET_GUID
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
  BufferSize = sizeof (SIO_NCT5104D_CONFIGURATION);
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
    BufferSize = sizeof (SIO_NCT5104D_CONFIGURATION);
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
  SIO_NCT5104D_GPIO_CONFIGURATION     *SioGpioConfig;
  UINT8                               Index, Value;

  SioGpioConfig = NULL;
  Index         = 0;
  Value         = 0;

  if (Configuration == NULL || Progress == NULL || This == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  PrivateData = SIO_PRIVATE_FROM_THIS (This);

  *Progress = Configuration;
  if (!HiiIsConfigHdrMatch (Configuration, &mSioFormSetGuid, mSioVariableName)) {
    return EFI_NOT_FOUND;
  }

  *Progress = Configuration + StrLen (Configuration);

  //
  // Convert <ConfigResp> to buffer data by helper function ConfigToBlock()
  //
  BufferSize = sizeof (SIO_NCT5104D_CONFIGURATION);
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
  // Set RT variable to NV variable
  //
  SioGpioConfig = &PrivateData->Configuration.GpioVariable;
  for (Index = 0; Index < MAX_SIO_GPIO; Index++) {
    Value = PrivateData->Configuration.Gpio[Index] & 0xC0;
    Value |= (SioGpioConfig->GpVariable[Index].Resistance & 1);
    Value |= ((SioGpioConfig->GpVariable[Index].TriState & 1) << 1);
    Value |= ((SioGpioConfig->GpVariable[Index].Invert & 1) << 2);
    Value |= ((SioGpioConfig->GpVariable[Index].InOut & 1) << 3);
    Value |= ((SioGpioConfig->GpVariable[Index].Output & 1) << 4);
    Value |= ((SioGpioConfig->GpVariable[Index].Output2 & 1) << 5);
    PrivateData->Configuration.Gpio[Index] = Value;
  }

  //
  // Store Buffer Storage back to EFI variable
  //
  Status = gRT->SetVariable(
                  mSioVariableName,
                  &mSioFormSetGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  sizeof (SIO_NCT5104D_CONFIGURATION),
                  &PrivateData->Configuration
                  );

  return Status;
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
  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) && (Action != EFI_BROWSER_ACTION_FORM_CLOSE)) || (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  switch (Action) {
  case EFI_BROWSER_ACTION_RETRIEVE:
  case EFI_BROWSER_ACTION_CHANGING:
    break;

  default:
    return EFI_UNSUPPORTED;
    break;
  }

  return EFI_SUCCESS;
}

VOID
InitDynamicSetupOptions (
  )
{
  UINT8                               *ExtensiveTable;
  SIO_DEVICE_LIST_TABLE               *DeviceResource;
  SIO_DEVICE_FIFO_SETTING             *FifoSetting;
  SIO_NCT5104D_ACPI_IRQ_INFORMATION   *AcpiIrqInfo;
  SIO_DEVICE_MODE_SELECTION           *DeviceModeSelection;

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
            AcpiIrqInfo = (SIO_NCT5104D_ACPI_IRQ_INFORMATION*)ExtensiveTable;
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

      case LPT:
        do {
          ExtensiveTable += *(ExtensiveTable + 1);
          switch (*ExtensiveTable) {
          case MODE_SELECTION_TYPE:
            DeviceModeSelection = (SIO_DEVICE_MODE_SELECTION*)ExtensiveTable;
            if (DeviceModeSelection->Enable) {
              CreateLptModeOptionOpCode (
                LPT1_MODE_LABEL + DeviceResource->DeviceInstance,
                DeviceModeSelection
                );
            }
            break;

          default:
            break;
          }
        } while ((*ExtensiveTable != DEVICE_EXTENSIVE_RESOURCE) && (*(UINT32*)ExtensiveTable != EXTENSIVE_TABLE_ENDING));
        break;

      case WDT:
        do {
          ExtensiveTable += *(ExtensiveTable + 1);
          switch (*ExtensiveTable) {
          case WATCH_DOG_SETTING_TYPE:
            if (DeviceResource->DeviceEnable) {
              CreateWdtOptionOpCode (
                SIO_WDT_LABEL,
                (SIO_WATCH_DOG_SETTING*)ExtensiveTable
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
  Add dynamic GPIO items for extensive function.

  @retval EFI_SUCCESS                The function completed successfully.
                                
**/          
EFI_STATUS
InitGpio (
  )
{
  UINT8                       OldGpioGroup;
  UINT8                       Index, DeviceHandle;
  UINT8                       *ExtensiveTable;
  EFI_STATUS                  Status;
  SIO_DEVICE_LIST_TABLE       *DeviceResource;
  SIO_EXTENSIVE_TABLE_TYPE17  *GpioSelection;
  VOID                        *StartOpCodeHandle;

  OldGpioGroup      = 0xFF;
  Index             = 0;
  DeviceHandle      = 0;
  GpioSelection     = NULL;
  StartOpCodeHandle = NULL;
  Status            = EFI_SUCCESS;
  DeviceResource    = (SIO_DEVICE_LIST_TABLE*)&mSioPrivateData->Configuration;

  for (Index = 0; Index < mSioPrivateData->Configuration.AutoUpdateNum; Index++, DeviceResource++) {
    if (DeviceResource->Device == SGPIO) {
      ExtensiveTable = mExtensiveTablePtr;
      
      while (DeviceHandle < MAX_SIO_GPIO) { 
        FindExtensiveDevice (
          &ExtensiveTable,
          DeviceResource->TypeInstance,
          DeviceResource->Device,
          DeviceHandle
          );

        if (ExtensiveTable != NULL) {
          GpioSelection = (SIO_EXTENSIVE_TABLE_TYPE17 *)(ExtensiveTable + *(ExtensiveTable + 1));
          if (OldGpioGroup == 0xFF) {
            OldGpioGroup = GpioSelection->GpioNum.Group;
            StartOpCodeHandle = HiiAllocateOpCodeHandle ();
            ASSERT (StartOpCodeHandle != NULL);
          } else if (OldGpioGroup != GpioSelection->GpioNum.Group) {
            Status = HiiUpdateForm (
                       mSioPrivateData->HiiHandle,                         // HII handle
                       &mSioFormSetGuid,                                   // Formset GUID
                       SIO_GPIO0X_FORM_ID + OldGpioGroup,                  // Form ID
                       StartOpCodeHandle,                                  // Label for where to insert opcodes
                       NULL                                                // Replace data
                       );
            OldGpioGroup = GpioSelection->GpioNum.Group;

            ASSERT_EFI_ERROR (Status);
            HiiFreeOpCodeHandle (StartOpCodeHandle);
            StartOpCodeHandle = HiiAllocateOpCodeHandle ();
            ASSERT (StartOpCodeHandle != NULL);
          }
          do {
            ExtensiveTable += *(ExtensiveTable + 1);
            switch (*ExtensiveTable) {
            case TYPE17:
              GpioSelection = (SIO_EXTENSIVE_TABLE_TYPE17 *)ExtensiveTable;
              if (GpioSelection->Enable) {
                Status = CreateGpioTypeOptionOpCode (
                           SIO_GPIO0X_LABEL + GpioSelection->GpioNum.Group,
                           DeviceHandle,
                           GpioSelection,
                           StartOpCodeHandle
                           );
                ASSERT_EFI_ERROR (Status);
                break;
              }
              break;
            default:
              break;
            }
          } while ((*ExtensiveTable != DEVICE_EXTENSIVE_RESOURCE) && (*(UINT32*)ExtensiveTable != EXTENSIVE_TABLE_ENDING));
        }
        DeviceHandle++;
      }
      if (DeviceHandle == MAX_SIO_GPIO) {
        Status = HiiUpdateForm (
                   mSioPrivateData->HiiHandle,                         // HII handle
                   &mSioFormSetGuid,                                   // Formset GUID
                   SIO_GPIO0X_FORM_ID + GpioSelection->GpioNum.Group,  // Form ID
                   StartOpCodeHandle,                                  // Label for where to insert opcodes
                   NULL                                                // Replace data
                   );

        ASSERT_EFI_ERROR (Status);
        HiiFreeOpCodeHandle (StartOpCodeHandle);
      }
    }
  }

  return Status;
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
  case COM3_MODE_LABEL:
    Offset = VAR_OFFSET (Com3Mode);
    QuestionId = COM_MODE_QUESTION_ID + 2;
    break;
  case COM4_MODE_LABEL:
    Offset = VAR_OFFSET (Com4Mode);
    QuestionId = COM_MODE_QUESTION_ID + 3;
    break;
  default:
    break;
  }

  ASSERT (Offset != 0);
  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    QuestionId,
    SIO_NCT5104D_CONFIGURATION_VARSTORE_ID,
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
             SIO_NCT5104D_CONFIGURATION_FORM_ID, // Form ID
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
  case COM3_FIFO_LABEL:
    Offset     = VAR_OFFSET (Com3Fifo);
    QuestionId = COM_FIFO_QUESTION_ID + 2;
    break;
  case COM4_FIFO_LABEL:
    Offset     = VAR_OFFSET (Com4Fifo);
    QuestionId = COM_FIFO_QUESTION_ID + 3;
    break;
  default:
    break;
  }

  ASSERT (Offset != 0);
  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    QuestionId,
    SIO_NCT5104D_CONFIGURATION_VARSTORE_ID,
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
    SIO_NCT5104D_CONFIGURATION_VARSTORE_ID,
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
             SIO_NCT5104D_CONFIGURATION_FORM_ID, // Form ID
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
  UINT16                              Label,
  UINT8                               DeviceInstance,
  SIO_NCT5104D_ACPI_IRQ_INFORMATION   *AcpiIrqInfo
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
    SIO_NCT5104D_CONFIGURATION_VARSTORE_ID,
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
    SIO_NCT5104D_CONFIGURATION_VARSTORE_ID,
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
    SIO_NCT5104D_CONFIGURATION_VARSTORE_ID,
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
             SIO_NCT5104D_CONFIGURATION_FORM_ID, // Form ID
             StartOpCodeHandle,                  // Label for where to insert opcodes
             NULL                                // Replace data
             );
  ASSERT_EFI_ERROR (Status);

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (OptionsOpCodeHandle);
}

/**
  Create OP code for COM type.

  @param[in]   Label             Update information starting immediately after this label in the IFR

**/
VOID
CreateLptModeOptionOpCode (
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

  for (Index = 0; Index < 7; Index++) {
    DefaultValue = 0;
    OwnedModeSupport   = FALSE;
    if (DeviceModeSelection->OwnedMode & (BIT0 << Index)) {
      OwnedModeSupport = TRUE;
      switch (Index) {
      case 0:
        StringId  = STRING_TOKEN(STR_SIO_LPT_SPP);
        break;
      case 1:
        StringId = STRING_TOKEN(STR_SIO_LPT_EPP);
        break;
      case 2:
        StringId = STRING_TOKEN(STR_SIO_LPT_ECP);
        break;
      case 3:
        StringId = STRING_TOKEN(STR_SIO_LPT_ECP_EPP);
        break;
      case 4:
        StringId = STRING_TOKEN(STR_SIO_LPT_ECP_SPP);
        break;
      case 5:
        StringId = STRING_TOKEN(STR_SIO_LPT_EPP_SPP);
        break;
      case 6:
        StringId = STRING_TOKEN(STR_SIO_LPT_PRINT);
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
  default:
    break;
  }

  ASSERT (Offset != 0);
  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    QuestionId,
    SIO_NCT5104D_CONFIGURATION_VARSTORE_ID,
    Offset,
    STRING_TOKEN (STR_SIO_MODE_SELECTION),
    STRING_TOKEN (STR_SIO_LPT_MODE_HELP),
    0,
    EFI_IFR_NUMERIC_SIZE_1,
    OptionsOpCodeHandle,
    NULL
    );

  Status = HiiUpdateForm (
             mSioPrivateData->HiiHandle,         // HII handle
             &mSioFormSetGuid,                   // Formset GUID
             SIO_NCT5104D_CONFIGURATION_FORM_ID, // Form ID
             StartOpCodeHandle,                  // Label for where to insert opcodes
             NULL                                // Replace data
             );
  ASSERT_EFI_ERROR (Status);

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (OptionsOpCodeHandle);
}

/**
  Create OP code for WDT Counter mode and Timer.

  @param[in]   Label           Update information starting immediately after this label in the IFR

**/
VOID
CreateWdtOptionOpCode (
  UINT16                 Label,
  SIO_WATCH_DOG_SETTING  *WdtSetting
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

  StringId    = 0;
  Offset      = 0;
  QuestionId  = 0;
  Status      = EFI_UNSUPPORTED;

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

  for (Index = 0; Index < 4; Index++) {
    if ((WdtSetting->UnitSupported & (BIT0 << Index)) == 0) {
      continue;
    }
    DefaultValue = 0;
    switch (Index) {
    case 0:
      StringId = STRING_TOKEN(STR_SIO_SELECT_SECOND);
      break;
    case 1:
      StringId = STRING_TOKEN(STR_SIO_SELECT_MINUTE);
      break;
    case 2:
      StringId = STRING_TOKEN(STR_SIO_SELECT_MILLISECOND);
      break;
    case 3:
      StringId = STRING_TOKEN(STR_SIO_SELECT_MILLIMINUTE);
      break;
    default:
      break;
    }
    if (Index == WdtSetting->UnitSelection) {
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

  Offset = VAR_OFFSET (WatchDogConfig);
  QuestionId = SIO_WDT_QUESTION_ID;

  ASSERT (Offset != 0);
  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    QuestionId,
    SIO_NCT5104D_CONFIGURATION_VARSTORE_ID,
    (Offset + WDT_SETTING_OFFSET (TimerCountMode)),
    STRING_TOKEN (STR_SIO_WDT_COUNT_MODE),
    STRING_TOKEN (STR_SIO_WDT_COUNT_MODE_HELP),
    0,
    EFI_IFR_NUMERIC_SIZE_1,
    OptionsOpCodeHandle,
    NULL
    );

  HiiFreeOpCodeHandle (OptionsOpCodeHandle);
  OptionsOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (OptionsOpCodeHandle != NULL);

  HiiCreateDefaultOpCode (
    OptionsOpCodeHandle,
    EFI_HII_DEFAULT_CLASS_STANDARD,
    EFI_IFR_NUMERIC_SIZE_2,
    WdtSetting->TimeOut
    );
  HiiCreateNumericOpCode (
    StartOpCodeHandle,
    (QuestionId + 1),
    SIO_NCT5104D_CONFIGURATION_VARSTORE_ID,
    (Offset + WDT_SETTING_OFFSET (TimeOutValue)),
    STRING_TOKEN(STR_SIO_WDT_COUNTER),
    STRING_TOKEN(STR_SIO_WDT_COUNTER_HELP),
    0,
    EFI_IFR_NUMERIC_SIZE_2 | EFI_IFR_DISPLAY_UINT_DEC,
    0,
    WdtSetting->MaxTimeOut,
    1,
    OptionsOpCodeHandle
    );

  Status = HiiUpdateForm (
             mSioPrivateData->HiiHandle,         // HII handle
             &mSioFormSetGuid,                   // Formset GUID
             SIO_NCT5104D_CONFIGURATION_FORM_ID, // Form ID
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

EFI_STATUS
CreateGpioTypeOptionOpCode (
  IN UINT16                       Label,
  IN UINT8                        DeviceHandle,
  IN SIO_EXTENSIVE_TABLE_TYPE17   *GpioSelection,
  IN VOID                         *StartOpCodeHandle
  )
{
  EFI_STATUS           Status;
  UINT8                Index;
  UINT8                DefaultType0, DefaultType1;
  UINT16               Offset, OffsetTmp;
  UINT16               StringId, StringId0, StringId1;
  UINT16               OptionId, HelpId;
  EFI_QUESTION_ID      QuestionId;
  EFI_IFR_GUID_LABEL   *StartLabel;
  VOID                 *OptionsOpCodeHandle;
  EFI_STRING           NewStringTitle, OutValString;
  BOOLEAN              NeedCreate, FindItem;
  LIST_ENTRY           *Link;
  SIO_MAP_QUESTION_ID  *Item;
  UINT8                ItemCount;

  Status               = EFI_SUCCESS;
  StringId             = 0;
  StringId0            = 0;
  StringId1            = 0;
  Offset               = 0xFFFF;
  OffsetTmp            = 0;
  OptionId             = 0;
  HelpId               = 0;
  QuestionId           = 0;
  Index                = 0;
  DefaultType0         = 0;
  DefaultType1         = 0;
  NewStringTitle       = NULL;
  OutValString         = NULL;
  OptionsOpCodeHandle  = NULL;
  NeedCreate           = FALSE;
  FindItem             = FALSE;
  Item                 = NULL;

  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = Label;

  OffsetTmp = SIO_GPIO00_OFFSET + DeviceHandle * 6;

  NewStringTitle = AllocateZeroPool (14);

  OutValString = AllocateZeroPool (26);

  UnicodeSPrint (NewStringTitle, 14, L"GPIO%x%x", GpioSelection->GpioNum.Group, GpioSelection->GpioNum.Num);

  StringId = HiiSetString (mSioPrivateData->HiiHandle, 0, NewStringTitle, NULL);

  HiiCreateSuppressIfOpCodeEx (StartOpCodeHandle, 1);

  ItemCount = 0;
  for (Link = GetFirstNode (&mItemList.List); !IsNull (&mItemList.List, Link); Link = GetNextNode (&mItemList.List, Link)) {
    Item = (SIO_MAP_QUESTION_ID*) Link;
    if (DeviceHandle == Item->GpHandle) {
      if (!FindItem) {
        HiiCreateIdEqualOpCodeEx2 (StartOpCodeHandle, Item->FeatureId, 0, 1);
        FindItem = TRUE;
      } else {
        HiiCreateIdEqualOpCodeEx2 (StartOpCodeHandle, Item->FeatureId, 0, 0);
      }
      HiiCreateNotOpCode (StartOpCodeHandle);
      ItemCount++;
    }
  }

  if (!FindItem) {
    HiiCreateFalseOpCode (StartOpCodeHandle);
  } else {
    for (Index = 0; Index < ItemCount - 1; Index++) {
      HiiCreateAndOpCode (StartOpCodeHandle);
    }
    HiiCreateEndOpCode (StartOpCodeHandle);  // Not equal
  }

  HiiCreateSubTitleOpCode (StartOpCodeHandle, StringId, 0, 0, 1);

  for (Index = 0; Index < 6; Index++) {
    OptionsOpCodeHandle = HiiAllocateOpCodeHandle ();
    ASSERT (OptionsOpCodeHandle != NULL);
    DefaultType0 = 0;
    DefaultType1 = 0;
    StringId0    = 0;
    StringId1    = 0;
    NeedCreate   = FALSE;
    Offset       = OffsetTmp + Index;
    switch (Index) {
    //
    //  Resistance
    //
    case 0:
      if (GpioSelection->Capability.Pull != 0) {
        OptionId     = STRING_TOKEN (STR_GPIO_INTERNAL_RESISTANCE);
        HelpId       = STRING_TOKEN (STR_GPIO_INTERNAL_HELP_STRING);
        QuestionId   = SIO_GPIO_RESISTANCE_QUESTION_ID + DeviceHandle;
        StringId0    = STRING_TOKEN (STR_GPIO_PULL_DOWN);
        StringId1    = STRING_TOKEN (STR_GPIO_PULL_UP);
        NeedCreate   = TRUE;
        if (GpioSelection->Setting.Pull) {
          DefaultType1 = EFI_IFR_OPTION_DEFAULT;
        } else {
          DefaultType0 = EFI_IFR_OPTION_DEFAULT;
        }
      }
      break;
    case 1:
      if (GpioSelection->Capability.TriState != 0) {
        OptionId     = STRING_TOKEN (STR_GPIO_INTERNAL_RESISTANCE);
        HelpId       = STRING_TOKEN (STR_GPIO_INTERNAL_HELP_STRING2);
        QuestionId   = SIO_GPIO_RESISTANCE2_QUESTION_ID + DeviceHandle;
        StringId0    = STRING_TOKEN (STR_GPIO_PUSH_PULL);
        StringId1    = STRING_TOKEN (STR_GPIO_OPEN_DRAIN);
        NeedCreate   = TRUE;
        if (GpioSelection->Setting.TriState) {
          DefaultType1 = EFI_IFR_OPTION_DEFAULT;
        } else {
          DefaultType0 = EFI_IFR_OPTION_DEFAULT;
        }
      }
      break;
    //
    //  Invert
    //
    case 2:
      if (GpioSelection->Capability.IsInv) {
        OptionId     = STRING_TOKEN (STR_GPIO_POLARITY);
        HelpId       = STRING_TOKEN (STR_GPIO_POLARITY_HELP_STRING);
        QuestionId   = SIO_GPIO_POLARITY_QUESTION_ID + DeviceHandle;
        StringId0    = STRING_TOKEN (STR_SIO_SELECT_DISABLE);
        StringId1    = STRING_TOKEN (STR_SIO_SELECT_ENABLE);
        NeedCreate   = TRUE;
        if (GpioSelection->Setting.Invert) {
          DefaultType1 = EFI_IFR_OPTION_DEFAULT;
        } else {
          DefaultType0 = EFI_IFR_OPTION_DEFAULT;
        }
      }
      break;
    //
    // In or Out
    //
    case 3:
      if (GpioSelection->Capability.InOut != 0) {
        OptionId     = STRING_TOKEN (STR_GPIO_IN_OUT);
        HelpId       = STRING_TOKEN (STR_GPIO_IN_OUT_HELP_STRING);
        QuestionId   = SIO_GPIO_IN_OUT_QUESTION_ID + DeviceHandle;
        StringId0    = STRING_TOKEN (STR_GPIO_INPUT);
        StringId1    = STRING_TOKEN (STR_GPIO_OUTPUT);
        NeedCreate   = TRUE;
        if (GpioSelection->Setting.InOut) {
          DefaultType1 = EFI_IFR_OPTION_DEFAULT;
        } else {
          DefaultType0 = EFI_IFR_OPTION_DEFAULT;
        }
      } 
      break;
    case 4:
      if (GpioSelection->Capability.InOut & 2) {
        OptionId     = STRING_TOKEN (STR_GPIO_OUT_VALUE);
        HelpId       = STRING_TOKEN (STR_GPIO_OUT_VALUE_HELP_STRING);
        QuestionId   = SIO_GPIO_OUT_VAL_QUESTION_ID + DeviceHandle;
        StringId0    = STRING_TOKEN (STR_GPIO_LOW);
        StringId1    = STRING_TOKEN (STR_GPIO_HIGH);
        NeedCreate   = TRUE;
        if (GpioSelection->Setting.PeiOut) {
          DefaultType1 = EFI_IFR_OPTION_DEFAULT;
        } else {
          DefaultType0 = EFI_IFR_OPTION_DEFAULT;
        }
        if (!PCD_SIO_ENABLE_GPIO_PEI) {
          UnicodeSPrint (OutValString, 26, L"Output Value");
          HiiSetString (mSioPrivateData->HiiHandle, OptionId, OutValString, NULL);
        }
        HiiCreateSuppressIfOpCodeEx (StartOpCodeHandle, 1);
        HiiCreateIdEqualOpCodeEx (StartOpCodeHandle, SIO_GPIO_IN_OUT_QUESTION_ID + DeviceHandle, 0);
      }
      break;
    case 5:
      if ((GpioSelection->Capability.InOut & 2) && PCD_SIO_ENABLE_GPIO_PEI) {
        OptionId     = STRING_TOKEN (STR_GPIO_OUT2_VALUE);
        HelpId       = STRING_TOKEN (STR_GPIO_OUT_VALUE_HELP_STRING);
        QuestionId   = SIO_GPIO_OUT2_VAL_QUESTION_ID + DeviceHandle;
        StringId0    = STRING_TOKEN (STR_GPIO_LOW);
        StringId1    = STRING_TOKEN (STR_GPIO_HIGH);
        NeedCreate   = TRUE;
        if (GpioSelection->Setting.DxeOut) {
          DefaultType1 = EFI_IFR_OPTION_DEFAULT;
        } else {
          DefaultType0 = EFI_IFR_OPTION_DEFAULT;
        }
        HiiCreateSuppressIfOpCodeEx (StartOpCodeHandle, 1);
        HiiCreateIdEqualOpCodeEx (StartOpCodeHandle, SIO_GPIO_IN_OUT_QUESTION_ID + DeviceHandle, 0);
      }
      break;
    default:
      break;
    }

    if (NeedCreate) {
      HiiCreateOneOfOptionOpCode (
        OptionsOpCodeHandle,
        StringId0,
        DefaultType0,
        EFI_IFR_NUMERIC_SIZE_1,
        0
        );    

      HiiCreateOneOfOptionOpCode (
        OptionsOpCodeHandle,
        StringId1,
        DefaultType1,
        EFI_IFR_NUMERIC_SIZE_1,
        1
        );    
    }

    if (Offset != 0xFFFF && NeedCreate) {
      HiiCreateOneOfOpCode (
        StartOpCodeHandle,
        QuestionId,
        SIO_NCT5104D_CONFIGURATION_VARSTORE_ID,
        Offset,
        OptionId,
        HelpId,
        EFI_IFR_FLAG_CALLBACK,
        EFI_IFR_NUMERIC_SIZE_1,
        OptionsOpCodeHandle,
        NULL
        );
    }
    if ((Index == 5 || Index == 4) && NeedCreate) {
      HiiCreateEndOpCode (StartOpCodeHandle);
    }
    HiiFreeOpCodeHandle (OptionsOpCodeHandle);
  }

  HiiCreateEndOpCode (StartOpCodeHandle);  //label

  HiiCreateEndOpCode (StartOpCodeHandle);  //suppressif

  FreePool (OutValString);

  FreePool (NewStringTitle);

  return Status;
}

VOID
SetupValueInit (
  SIO_DEVICE_LIST_TABLE  *PcdResource
  )
{
  UINT8                             *ExtensiveTable;
  UINT8                             *VariableAddress;
  SIO_NCT5104D_FIFO_SETTING         *FifoVariable;
  SIO_WATCH_DOG_SETTING             *WdtSetting;
  SIO_NCT5104D_ACPI_IRQ_INFORMATION *AcpiIrqInfo;
  SIO_DEVICE_FIFO_SETTING           *FifoSetting;
  SIO_DEVICE_MODE_SELECTION         *DeviceModeSelection;
  UINTN                             GpioIndex;
  SIO_EXTENSIVE_TABLE_TYPE17        *GpioSetting;
  SIO_NCT5104D_GPIO_CONFIGURATION   *GpioConfiguration;

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
    case 2:
      VariableAddress = &mSioPrivateData->Configuration.Com3Mode;
      FifoVariable    = &mSioPrivateData->Configuration.Com3Fifo;
      break;
    case 3:
      VariableAddress = &mSioPrivateData->Configuration.Com4Mode;
      FifoVariable    = &mSioPrivateData->Configuration.Com4Fifo;
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
        AcpiIrqInfo = (SIO_NCT5104D_ACPI_IRQ_INFORMATION*)ExtensiveTable;
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

  case LPT:
    switch (PcdResource->DeviceInstance) {
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
        case BIT3:
          *VariableAddress = 3;
          break;
        case BIT4:
          *VariableAddress = 4;
          break;
        case BIT5:
          *VariableAddress = 5;
          break;
        case BIT6:
          *VariableAddress = 6;
          break;
        }
        break;

      default:
        break;
      }
    } while ((*ExtensiveTable != DEVICE_EXTENSIVE_RESOURCE) && (*(UINT32*)ExtensiveTable != EXTENSIVE_TABLE_ENDING));
    break;

  case WDT:
    do {
      ExtensiveTable += *(ExtensiveTable + 1);
      switch (*ExtensiveTable) {
      case WATCH_DOG_SETTING_TYPE:
        WdtSetting = (SIO_WATCH_DOG_SETTING*)ExtensiveTable;
        mSioPrivateData->Configuration.WatchDogConfig.TimerCountMode = WdtSetting->UnitSelection;
        mSioPrivateData->Configuration.WatchDogConfig.TimeOutValue = WdtSetting->TimeOut;
        break;

      default:
        break;
      }
    } while ((*ExtensiveTable != DEVICE_EXTENSIVE_RESOURCE) && (*(UINT32*)ExtensiveTable != EXTENSIVE_TABLE_ENDING));
    break;
  
  case SGPIO:
    GpioIndex = 0;
    GpioConfiguration = &mSioPrivateData->Configuration.GpioVariable;
    do {
      ExtensiveTable += *(ExtensiveTable + 1);
      switch (*ExtensiveTable) {
      case TYPE17:
        GpioSetting = (SIO_EXTENSIVE_TABLE_TYPE17 *)ExtensiveTable;
        if (GpioSetting->Enable) {
          GpioConfiguration->GpVariable[GpioIndex].Resistance  = GpioSetting->Setting.Pull;
          GpioConfiguration->GpVariable[GpioIndex].TriState    = GpioSetting->Setting.TriState;
          GpioConfiguration->GpVariable[GpioIndex].Invert      = GpioSetting->Setting.Invert;
          GpioConfiguration->GpVariable[GpioIndex].InOut       = GpioSetting->Setting.InOut;
          GpioConfiguration->GpVariable[GpioIndex].Output      = GpioSetting->Setting.PeiOut;
          GpioConfiguration->GpVariable[GpioIndex].Output2     = GpioSetting->Setting.DxeOut;
        }
        GpioIndex++;
        break;
      }
    } while ((*(UINT32*)ExtensiveTable != EXTENSIVE_TABLE_ENDING));
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
                                 SioNct5104dDxeStrings,
                                 NULL
                                 );
  if (mSioPrivateData->HiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize configuration data
  //
  ZeroMem (&mSioPrivateData->Configuration, sizeof (SIO_NCT5104D_CONFIGURATION));

  //
  // Try to read NV config EFI variable first
  //
  ConfigRequestHdr = HiiConstructConfigHdr (
                       &mSioFormSetGuid,
                       mSioVariableName,
                       mSioPrivateData->DriverHandle
                       );
  ASSERT (ConfigRequestHdr != NULL);

  BufferSize = sizeof (SIO_NCT5104D_CONFIGURATION);
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
                    sizeof (SIO_NCT5104D_CONFIGURATION),
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
          (PcdPointer->Device == CIR) || (PcdPointer->Device == IR)  || (PcdPointer->Device == SGPIO)) {
        if (PcdPointer->DeviceEnable == TRUE) {
          GetByConfiguration->Device = PcdPointer->Device;
          GetByConfiguration->DeviceInstance = PcdPointer->DeviceInstance;
        }

        if (PcdPointer->Device == SGPIO) {
          GetByConfiguration->DeviceEnable = PcdPointer->DeviceEnable;
          GetByConfiguration->DeviceBaseAdr = PcdPointer->DeviceBaseAdr;
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
                    sizeof (SIO_NCT5104D_CONFIGURATION),
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

  //
  // Reserved for feature Gpio subtype programing
  //
  InitializeListHead (&mItemList.List);
  InitGpio ();
  HandleListRemoveAll (&mItemList);

  FreePool (ConfigRequestHdr);

  return Status;
}


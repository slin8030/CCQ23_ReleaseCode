/** @file

  Callback Functions for Console Redirection Configuration

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "CrConfigUtil.h"
#include "CrDeviceMenu.h"
#include "CrConfig.h"

extern CR_CONFIG_MANAGER_PRIVATE_DATA         *mPrivateData;
extern CHAR16                                 *mVariableName;

CR_DEV_NODE                                   *mCurrentDevNode = NULL;

STATIC
EFI_STATUS
ForceSubmit (
  VOID
  )
{
  EFI_STATUS                                  Status;
  EFI_FORM_BROWSER2_PROTOCOL                  *FormBrowser2;
  CHAR16                                      *ConfigRequest;
  UINTN                                       ConfigRequestSize;
  EFI_GUID                                    *NameValueGuid;
  CHAR16                                      *NameValueName;
  STATIC UINT8                                NameValueValue = 0;

  //
  // Change name value data to make form brower will call RouteConfig () when SubmitForm ()
  //
  Status = gBS->LocateProtocol (&gEfiFormBrowser2ProtocolGuid, NULL, (VOID **) &FormBrowser2);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return Status;
  }

  NameValueGuid = &gCrConfigurationGuid;
  NameValueName = HiiGetString (mPrivateData->HiiHandle, STRING_TOKEN(STR_NAME_VALUE_VAR_NAME0), NULL);
  if (NameValueValue < 0xFF) {
    NameValueValue ++;
  } else {
    NameValueValue = 0;
  }

  ConfigRequestSize = 0x100;
  ConfigRequest     = AllocateZeroPool (ConfigRequestSize);
  UnicodeSPrint (ConfigRequest, ConfigRequestSize, L"%s=%02x", NameValueName, NameValueValue);
  Status = FormBrowser2->BrowserCallback (
                           FormBrowser2,
                           &ConfigRequestSize,
                           ConfigRequest,
                           FALSE,
                           NameValueGuid,
                           NameValueName
                           );
  FreePool (NameValueName);
  FreePool (ConfigRequest);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return Status;
  }

  return EFI_SUCCESS;
}

STATIC
BOOLEAN
IsGlobalSetting (
  IN EFI_QUESTION_ID                          QuestionId
  )
{
  switch (QuestionId) {

  case KEY_GLOBAL_TERMINAL_TYPE:
  case KEY_GLOBAL_BAUD_RATE:
  case KEY_GLOBAL_DATA_BITS:
  case KEY_GLOBAL_PARITY:
  case KEY_GLOBAL_STOP_BITS:
  case KEY_GLOBAL_FLOW_CONTROL:
    return TRUE;

  default:
    break;
  }

  return FALSE;
}

STATIC
BOOLEAN
IsLocalSetting (
  IN EFI_QUESTION_ID                          QuestionId
  )
{
  switch (QuestionId) {

  case KEY_PORT_ENABLE:
  case KEY_USE_GLOBAL_SETTING:
  case KEY_TERMINAL_TYPE:
  case KEY_BAUD_RATE:
  case KEY_DATA_BITS:
  case KEY_PARITY:
  case KEY_STOP_BITS:
  case KEY_FLOW_CONTROL:
    return TRUE;

  default:
    break;
  }

  return FALSE;
}

STATIC
EFI_STATUS
UpdateStatusStr (
  IN CR_DEV_NODE                              *DevNode
  )
{
  CHAR16                                      *Str;

  if (DevNode == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Str = GetStatusStr (&DevNode->Dev);
  HiiSetString (mPrivateData->HiiHandle, DevNode->StatusStrId, Str, NULL);
  FreePool (Str);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
UpdateGlobalToLocal (
  IN CR_CONFIGURATION                         *CrConfig
  )
{
  if (CrConfig == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CrConfig->CrPortConfig.TerminalType = CrConfig->GlobalTerminalType;
  CrConfig->CrPortConfig.BaudRate     = CrConfig->GlobalBaudRate;
  CrConfig->CrPortConfig.Parity       = CrConfig->GlobalParity;
  CrConfig->CrPortConfig.DataBits     = CrConfig->GlobalDataBits;
  CrConfig->CrPortConfig.StopBits     = CrConfig->GlobalStopBits;
  CrConfig->CrPortConfig.FlowControl  = CrConfig->GlobalFlowControl;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
UpdateDeviceToCrConfig (
  IN CR_DEVICE_SETTING                        *Device,
  IN CR_CONFIGURATION                         *CrConfig
  )
{
  if (Device == NULL || CrConfig == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CrConfig->CrPortConfig.PortEnable       = Device->Attribute.PortEnable;
  CrConfig->CrPortConfig.UseGlobalSetting = Device->Attribute.UseGlobalSetting;

  CrConfig->CrPortConfig.TerminalType     = Device->Attribute.TerminalType;
  CrConfig->CrPortConfig.BaudRate         = Device->Attribute.BaudRate;
  CrConfig->CrPortConfig.Parity           = Device->Attribute.Parity;
  CrConfig->CrPortConfig.DataBits         = Device->Attribute.DataBits;
  CrConfig->CrPortConfig.StopBits         = Device->Attribute.StopBits;
  CrConfig->CrPortConfig.FlowControl      = Device->Attribute.FlowControl;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
UpdateDeviceFromCrConfig (
  IN CR_DEVICE_SETTING                        *Device,
  IN CR_CONFIGURATION                         *CrConfig
  )
{
  if (Device == NULL || CrConfig == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Device->Attribute.PortEnable       = CrConfig->CrPortConfig.PortEnable;
  Device->Attribute.UseGlobalSetting = CrConfig->CrPortConfig.UseGlobalSetting;

  Device->Attribute.TerminalType     = CrConfig->CrPortConfig.TerminalType;
  Device->Attribute.BaudRate         = CrConfig->CrPortConfig.BaudRate;
  Device->Attribute.Parity           = CrConfig->CrPortConfig.Parity;
  Device->Attribute.DataBits         = CrConfig->CrPortConfig.DataBits;
  Device->Attribute.StopBits         = CrConfig->CrPortConfig.StopBits;
  Device->Attribute.FlowControl      = CrConfig->CrPortConfig.FlowControl;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
UpdateAllDevFromConfig (
  IN CR_CONFIGURATION                         *CrConfig
  )
{
  LIST_ENTRY                                  *Link;
  CR_DEV_NODE                                 *CrDevNode;

  if (IsListEmpty (&mPrivateData->CrDevListHead)) {
    return EFI_NOT_FOUND;
  }

  Link = GetFirstNode (&mPrivateData->CrDevListHead);
  while (!IsNull (&mPrivateData->CrDevListHead, Link)) {

    CrDevNode = CR_DEV_NODE_FROM_LINK (Link);
    Link = GetNextNode (&mPrivateData->CrDevListHead, Link);

    if (CrDevNode->Dev.Attribute.UseGlobalSetting) {
      CrDevNode->Dev.Attribute.TerminalType = CrConfig->GlobalTerminalType;
      CrDevNode->Dev.Attribute.BaudRate     = CrConfig->GlobalBaudRate;
      CrDevNode->Dev.Attribute.Parity       = CrConfig->GlobalParity;
      CrDevNode->Dev.Attribute.DataBits     = CrConfig->GlobalDataBits;
      CrDevNode->Dev.Attribute.StopBits     = CrConfig->GlobalStopBits;
      CrDevNode->Dev.Attribute.FlowControl  = CrConfig->GlobalFlowControl;
    }
    UpdateStatusStr (CrDevNode);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
UpdateDevPage (
  VOID
  )
{
  EFI_STATUS                                  Status;
  CR_DEVICE_SETTING                           *Device;
  CR_CONFIGURATION                            CrConfig;
  BOOLEAN                                     IsSuccess;

  if (mCurrentDevNode == NULL) {
    return EFI_UNSUPPORTED;
  }

  Device = &mCurrentDevNode->Dev;

  Status = HiiSetString (mPrivateData->HiiHandle, STRING_TOKEN (STR_STR_CR_DEVICE_TITLE), Device->DevName, NULL);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return Status;
  }

  IsSuccess = HiiGetBrowserData (&gCrConfigurationGuid, mVariableName, sizeof (CR_CONFIGURATION), (UINT8 *)&CrConfig);
  if (!IsSuccess) {
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  UpdateDeviceToCrConfig (Device, &CrConfig);

  IsSuccess = HiiSetBrowserData (&gCrConfigurationGuid, mVariableName, sizeof (CR_CONFIGURATION), (UINT8 *)&CrConfig, NULL);
  if (!IsSuccess) {
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  return Status;
}

STATIC
EFI_STATUS
GotoDevCallback (
  IN EFI_QUESTION_ID                          QuestionId
  )
{
  EFI_STATUS                                  Status;
  CR_DEV_NODE                                 *DevNode;

  //
  // Current question is the "created goto question"
  //
  Status = GetDevNodeByQuestionId (QuestionId, &DevNode);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  mCurrentDevNode = DevNode;

  UpdateDevPage ();

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GlobalSettingCallback (
  VOID
  )
{
  CR_CONFIGURATION                            CrConfig;
  BOOLEAN                                     IsSuccess;

  IsSuccess = HiiGetBrowserData (&gCrConfigurationGuid, mVariableName, sizeof (CR_CONFIGURATION), (UINT8 *)&CrConfig);
  if (!IsSuccess) {
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  UpdateAllDevFromConfig (&CrConfig);

  IsSuccess = HiiSetBrowserData (&gCrConfigurationGuid, mVariableName, sizeof (CR_CONFIGURATION), (UINT8 *)&CrConfig, NULL);
  if (!IsSuccess) {
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
LocalSettingCallback (
  VOID
  )
{
  CR_DEVICE_SETTING                           *Device;
  CR_CONFIGURATION                            CrConfig;
  BOOLEAN                                     IsSuccess;

  if (mCurrentDevNode == NULL) {
    return EFI_UNSUPPORTED;
  }

  IsSuccess = HiiGetBrowserData (&gCrConfigurationGuid, mVariableName, sizeof (CR_CONFIGURATION), (UINT8 *)&CrConfig);
  if (!IsSuccess) {
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  Device = &mCurrentDevNode->Dev;

  if (CrConfig.CrPortConfig.UseGlobalSetting) {
    UpdateGlobalToLocal (&CrConfig);
  }
  UpdateDeviceFromCrConfig (Device, &CrConfig);

  UpdateStatusStr (mCurrentDevNode);

  IsSuccess = HiiSetBrowserData (&gCrConfigurationGuid, mVariableName, sizeof (CR_CONFIGURATION), (UINT8 *)&CrConfig, NULL);
  if (!IsSuccess) {
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
AfterLoadDefault (
  VOID
  )
{
  //
  // Update CR devices
  //
  UpdateDevices (TRUE);

  //
  // Check use global setting
  //
  GlobalSettingCallback ();

  //
  // Update device page
  //
  UpdateDevPage ();

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CrSubmit (
  VOID
  )
{
  return SetDevVariables ();
}

EFI_STATUS
EFIAPI
CrDriverCallback (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL     *This,
  IN EFI_BROWSER_ACTION                       Action,
  IN EFI_QUESTION_ID                          QuestionId,
  IN UINT8                                    Type,
  IN EFI_IFR_TYPE_VALUE                       *Value,
  OUT EFI_BROWSER_ACTION_REQUEST              *ActionRequest
  )
{
  EFI_STATUS                                  Status;

  //
  // Ensure call RoutConfig () when submit even if no changing anything
  //
  if (IsListEmpty (&mPrivateData->CrDevListHead)) {
    return EFI_UNSUPPORTED;
  } else {
    ForceSubmit ();
  }

  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) && (Action != EFI_BROWSER_ACTION_FORM_CLOSE))||
    (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;

  switch (Action) {

  case EFI_BROWSER_ACTION_CHANGING:
    DEBUG ((EFI_D_INFO, "CRCallBack->CHANGING               Qid:0x%x  Typ:0x%x\n", QuestionId, Type));
    Status = GotoDevCallback (QuestionId);
    break;

  case EFI_BROWSER_ACTION_CHANGED:
    DEBUG ((EFI_D_INFO, "CRCallBack->CHANGED                Qid:0x%x  Typ:0x%x\n", QuestionId, Type));
    if (IsGlobalSetting (QuestionId)) {
      Status = GlobalSettingCallback ();
    } else if (IsLocalSetting (QuestionId)) {
      Status = LocalSettingCallback ();
    }
    break;

  case EFI_BROWSER_ACTION_DEFAULT_STANDARD:
    DEBUG ((EFI_D_INFO, "CRCallBack->DEFAULT_STANDARD       Qid:0x%x  Typ:0x%x\n", QuestionId, Type));
    if (QuestionId == KEY_SCAN_F9) {
        Status = AfterLoadDefault ();
    } else {
      Status = EFI_UNSUPPORTED;
    }
    break;

  case EFI_BROWSER_ACTION_RETRIEVE:
    DEBUG ((EFI_D_INFO, "CRCallBack->RETRIEVE               Qid:0x%x  Typ:0x%x\n", QuestionId, Type));
    if (QuestionId == KEY_SCAN_F9) {
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_UNSUPPORTED;
    }
    break;

  default:
    DEBUG ((EFI_D_INFO, "CRCallBack->default:Action=0x%x    Qid:0x%x  Typ:0x%x\n", Action , QuestionId, Type));
    Status = EFI_UNSUPPORTED;
    break;
  }

  return Status;
}


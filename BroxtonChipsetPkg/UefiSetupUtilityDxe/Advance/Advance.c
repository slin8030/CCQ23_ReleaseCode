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

#include "Advance.h"
//[-start-161027-IB07400806-add]//
#include <Protocol/CpuGlobalNvsArea.h>
//[-end-161027-IB07400806-add]//

#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)
IDE_CONFIG                                *mIdeConfig;
//[-start-151123-IB11270134-add]//
ADVANCE_CONFIGURATION                      mAdvConfig;
//[-end-151123-IB11270134-add]//
#endif
EFI_CALLBACK_INFO                         *mAdvCallBackInfo;

/**
 This is the callback function for the Advance Menu.

 @param [in]   This
 @param [in]   Action
 @param [in]   QuestionId
 @param [in]   Type
 @param [in]   Value
 @param [out]  ActionRequest


**/
EFI_STATUS
AdvanceCallbackRoutine (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  UINTN                                 BufferSize;
  EFI_STATUS                            Status;
  EFI_HII_HANDLE                        HiiHandle;
  EFI_CALLBACK_INFO                     *CallbackInfo;
  CHIPSET_CONFIGURATION                 *MyIfrNVData;
  SETUP_UTILITY_CONFIGURATION           *SUCInfo;
  EFI_SETUP_UTILITY_BROWSER_PROTOCOL    *Interface;
  EFI_GUID                              VarStoreGuid = SYSTEM_CONFIGURATION_GUID;

  Interface = NULL;

  if (!FeaturePcdGet (PcdH2OFormBrowserSupported) && Action == EFI_BROWSER_ACTION_CHANGING) {
    //
    // For compatible to old form browser which only use EFI_BROWSER_ACTION_CHANGING action,
    // change action to EFI_BROWSER_ACTION_CHANGED to make it workable.
    //
    Action = EFI_BROWSER_ACTION_CHANGED;
  }

  if (Action != EFI_BROWSER_ACTION_CHANGED) {
    return AdvanceCallbackRoutineByAction (This, Action, QuestionId, Type, Value, ActionRequest);
  }
  
  *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
  CallbackInfo   = EFI_CALLBACK_INFO_FROM_THIS (This);

  Interface = NULL;
  BufferSize   = GetVarStoreSize (CallbackInfo->HiiHandle, &CallbackInfo->FormsetGuid, &VarStoreGuid, "SystemConfig");
  if (!FeaturePcdGet (PcdH2OFormBrowserSupported)) {
    if (QuestionId == GET_SETUP_CONFIG || QuestionId == SET_SETUP_CONFIG) {
      Status = SetupVariableConfig (
                 &VarStoreGuid,
                 L"SystemConfig",
                 BufferSize,
                 (UINT8 *) gSUBrowser->SCBuffer,
                 (BOOLEAN)(QuestionId == GET_SETUP_CONFIG)
                 );
      return Status;
    }
    Status = gBS->LocateProtocol (
                   &gEfiSetupUtilityBrowserProtocolGuid,
                   NULL,
                   (VOID **)&Interface
                   );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = SetupVariableConfig (
             &VarStoreGuid,
             L"SystemConfig",
             BufferSize,
             (UINT8 *) gSUBrowser->SCBuffer,
             TRUE
             );
  MyIfrNVData = (CHIPSET_CONFIGURATION *)gSUBrowser->SCBuffer;
  Status    = EFI_SUCCESS;
  SUCInfo   = gSUBrowser->SUCInfo;

  HiiHandle = CallbackInfo->HiiHandle;

  switch (QuestionId) {

  default:
    Status = HotKeyCallBack (
               This,
               Action,
               QuestionId,
               Type,
               Value,
               ActionRequest
               );
    break;
  }

  SetupVariableConfig (
    &VarStoreGuid,
    L"SystemConfig",
    BufferSize,
    (UINT8 *) gSUBrowser->SCBuffer,
    FALSE
    );

  return Status;
}

EFI_STATUS
AdvanceCallbackRoutineByAction (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_STATUS                            Status;
  EFI_CALLBACK_INFO                     *CallbackInfo;
  UINTN                                 BufferSize;
  EFI_GUID                              VarStoreGuid = SYSTEM_CONFIGURATION_GUID;

  if ((This == NULL) ||
      ((Value == NULL) &&
       (Action != EFI_BROWSER_ACTION_FORM_OPEN) &&
       (Action != EFI_BROWSER_ACTION_FORM_CLOSE))||
      (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
  CallbackInfo   = EFI_CALLBACK_INFO_FROM_THIS (This);
  BufferSize     = GetVarStoreSize (CallbackInfo->HiiHandle, &CallbackInfo->FormsetGuid, &VarStoreGuid, "SystemConfig");
  Status         = EFI_UNSUPPORTED;

  switch (Action) {

  case EFI_BROWSER_ACTION_FORM_OPEN:
    if (QuestionId == 0) {
      Status = SetupVariableConfig (
                 &VarStoreGuid,
                 L"SystemConfig",
                 BufferSize,
                 (UINT8 *) gSUBrowser->SCBuffer,
                 FALSE
                 );
    }
    break;

  case EFI_BROWSER_ACTION_FORM_CLOSE:
    if (QuestionId == 0) {
      Status = SetupVariableConfig (
                 &VarStoreGuid,
                 L"SystemConfig",
                 BufferSize,
                 (UINT8 *) gSUBrowser->SCBuffer,
                 TRUE
                 );
    }
    break;

  case EFI_BROWSER_ACTION_CHANGING:
    Status = EFI_SUCCESS;
    break;

  case EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING:
    if (QuestionId == KEY_SCAN_F9) {
      Status = HotKeyCallBack (
                This,
                Action,
                QuestionId,
                Type,
                Value,
                ActionRequest
                );
      Status = SetupVariableConfig (
                  &VarStoreGuid,
                  L"SystemConfig",
                  BufferSize,
                  (UINT8 *) gSUBrowser->SCBuffer,
                  FALSE
                  );
    }
    //
    // avoid GetQuestionDefault execute ExtractConfig
    //
    return EFI_SUCCESS;

  default:
    break;
  }

  return Status;
}

EFI_STATUS
InstallAdvanceCallbackRoutine (
  IN EFI_HANDLE                             DriverHandle,
  IN EFI_HII_HANDLE                         HiiHandle
  )
{
  EFI_STATUS                                Status;
  EFI_GUID                                  FormsetGuid = FORMSET_ID_GUID_ADVANCE;
  EFI_GUID                                  OldFormsetGuid = SYSTEM_CONFIGURATION_GUID;
  
  mAdvCallBackInfo = AllocatePool (sizeof(EFI_CALLBACK_INFO));
  if (mAdvCallBackInfo == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  mAdvCallBackInfo->Signature                    = EFI_CALLBACK_INFO_SIGNATURE;
  mAdvCallBackInfo->DriverCallback.ExtractConfig = gSUBrowser->ExtractConfig;
  mAdvCallBackInfo->DriverCallback.RouteConfig   = gSUBrowser->RouteConfig;
  mAdvCallBackInfo->DriverCallback.Callback      = AdvanceCallbackRoutine;
  mAdvCallBackInfo->HiiHandle                    = HiiHandle;
  if (FeaturePcdGet (PcdH2OFormBrowserSupported)) {
    CopyGuid (&mAdvCallBackInfo->FormsetGuid, &FormsetGuid);
  } else {
    CopyGuid (&mAdvCallBackInfo->FormsetGuid, &OldFormsetGuid);
  }

  //
  // Install protocol interface
  //
  Status = gBS->InstallProtocolInterface (
                  &DriverHandle,
                  &gEfiHiiConfigAccessProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mAdvCallBackInfo->DriverCallback
                  );
  ASSERT_EFI_ERROR (Status);
  Status = InitAdvanceMenu (HiiHandle);

  return Status;
}

EFI_STATUS
UninstallAdvanceCallbackRoutine (
  IN EFI_HANDLE                             DriverHandle
  )
{
  EFI_STATUS     Status;

  if (mAdvCallBackInfo == NULL) {
    return EFI_SUCCESS;
  }
  Status = gBS->UninstallProtocolInterface (
                  DriverHandle,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mAdvCallBackInfo->DriverCallback
                  );
  ASSERT_EFI_ERROR (Status);
  
  gBS->FreePool (mAdvCallBackInfo);
  mAdvCallBackInfo = NULL;
  
  return Status;
}

//[-start-161027-IB07400806-add]//
VOID
UpdateAutoThermalReporting (
  IN EFI_HII_HANDLE                         HiiHandle
  )
{
  CHAR16                        Buffer[40];

  EFI_STATUS                        Status;
  CPU_GLOBAL_NVS_AREA_PROTOCOL      *CpuGlobalNvsAreaProtocol;
  
  DEBUG ((EFI_D_ERROR, "UpdateAutoThermalReporting!\n"));
  Status = gBS->LocateProtocol (&gCpuGlobalNvsAreaProtocolGuid, NULL, (VOID **) &CpuGlobalNvsAreaProtocol);
  DEBUG ((EFI_D_ERROR, "Status = %r, CpuGlobalNvsArea = %x\n", Status, CpuGlobalNvsAreaProtocol->Area));
  ASSERT_EFI_ERROR (Status);
  
//  if (((CHIPSET_CONFIGURATION *)gSUBrowser->SCBuffer)->AutoThermalReporting) {
    UnicodeSPrint (Buffer, sizeof (Buffer), L"%d", (UINT32)CpuGlobalNvsAreaProtocol->Area->AutoCriticalTripPoint);
    HiiSetString(HiiHandle, (EFI_STRING_ID)STR_AUTO_CRITICAL_THERMAL_TRIP_POINT_VALUE, Buffer, NULL);
    
    UnicodeSPrint (Buffer, sizeof (Buffer), L"%d", (UINT32)CpuGlobalNvsAreaProtocol->Area->AutoPassiveTripPoint);
    HiiSetString(HiiHandle, (EFI_STRING_ID)STR_AUTO_PASSIVE_THERMAL_TRIP_POINT_VALUE, Buffer, NULL);
    
    UnicodeSPrint (Buffer, sizeof (Buffer), L"%d", (UINT32)1);
    HiiSetString(HiiHandle, (EFI_STRING_ID)STR_AUTO_PASSIVE_TC1_VALUE, Buffer, NULL);
    
    UnicodeSPrint (Buffer, sizeof (Buffer), L"%d", (UINT32)5);
    HiiSetString(HiiHandle, (EFI_STRING_ID)STR_AUTO_PASSIVE_TC2_VALUE, Buffer, NULL);
    
    UnicodeSPrint (Buffer, sizeof (Buffer), L"%d", (UINT32)10);
    HiiSetString(HiiHandle, (EFI_STRING_ID)STR_AUTO_PASSIVE_TSP_VALUE, Buffer, NULL);
    
    UnicodeSPrint (Buffer, sizeof (Buffer), L"%d", (UINT32)CpuGlobalNvsAreaProtocol->Area->AutoActiveTripPoint);
    HiiSetString(HiiHandle, (EFI_STRING_ID)STR_AUTO_ACTIVE_THERMAL_TRIP_POINT_VALUE, Buffer, NULL);
//  }
}
//[-end-161027-IB07400806-add]//

VOID
UpdateSeCInformation(
  IN EFI_HII_HANDLE                         HiiHandle
)
{

  CHAR16                  TempString[80];
  EFI_STATUS              Status;
  SEC_OPERATION_PROTOCOL  *SeCOp;
  SEC_INFOMATION          SeCInfo;

  DEBUG ((EFI_D_ERROR, "UpdateSeCInformation --\n"));

  Status = gBS->LocateProtocol (
                &gEfiSeCOperationProtocolGuid,
                NULL,
                (VOID **)&SeCOp
                );
  if (EFI_ERROR(Status)) {
    return;
  }

  Status = SeCOp->GetPlatformSeCInfo(
                    &SeCInfo
                    );
  if (EFI_ERROR(Status)) {
    return;
  }
  if(SeCInfo.SeCVerValid) {
    UnicodeSPrint (
      TempString, 
      sizeof(TempString), 
      L"%d.%d.%d.%d", 
      SeCInfo.SeCVer.CodeMajor,
      SeCInfo.SeCVer.CodeMinor,
      SeCInfo.SeCVer.CodeHotFix,
      SeCInfo.SeCVer.CodeBuildNo
      );
  } else {
    StrCpy (TempString, L"0.0.0.0");
  }
  
//[-start-160105-IB03090424-modify]//
  DEBUG ((EFI_D_ERROR, "update STR_TXE_FW_VALUE\n"));
  HiiSetString(HiiHandle, (STRING_REF)STR_TXE_FW_VALUE, TempString, NULL);
//[-end-160105-IB03090424-modify]//

  if (SeCInfo.SeCCapabilityValid) {
    UnicodeSPrint (
      TempString, 
      sizeof(TempString), 
      L"%08x",
      SeCInfo.SeCCapability 
      );
  } else {
    StrCpy (TempString, L"N/A");
  }

  DEBUG ((EFI_D_ERROR, "update STR_SEC_CAPABILITY_VALUE\n"));
  HiiSetString(HiiHandle, (STRING_REF)STR_SEC_CAPABILITY_VALUE, TempString, NULL);

  if (SeCInfo.SeCFeatureValid) {
    UnicodeSPrint (
      TempString, 
      sizeof(TempString), 
      L"%08x",
      SeCInfo.SeCFeature
      );
  } else{
    StrCpy (TempString, L"N/A");
  }
  DEBUG ((EFI_D_ERROR, "update STR_SEC_FEATURE_VALUE\n"));
  HiiSetString(HiiHandle, (STRING_REF)STR_SEC_FEATURE_VALUE, TempString, NULL);
  
  if (SeCInfo.SeCOEMTagValid) {
    UnicodeSPrint (
              TempString, 
              sizeof(TempString), 
              L"%08x", 
              SeCInfo.SeCOEMTag
              );
  } else {
    StrCpy (TempString, L"N/A");
  }
  
  DEBUG ((EFI_D_ERROR, "update STR_SEC_OEMTAG_VALUE\n"));
  HiiSetString(HiiHandle, (STRING_REF)STR_SEC_OEMTAG_VALUE, TempString, NULL);

  switch (SeCInfo.SeCOpMode) {
  
  case 0:
    StrCpy (TempString, L"Normal");
    break;
    
  case 1:
    StrCpy (TempString, L"N/A");
    break;

  case 2:
    StrCpy (TempString, L"Debug Mode");
    break;

  case 3:
    StrCpy (TempString, L"Temporary Disabled");
    break;

  case 4:
    StrCpy (TempString, L"SECOVR_JMPR");
    break;

  case 5:
    StrCpy (TempString, L"SECOVR_HECI_MSG");
    break;
    
  default:
    StrCpy (TempString, L"Unknown");
    break;
  
  }

  DEBUG ((EFI_D_ERROR, "update STR_SEC_TEMP_DISABLE_PROMPT\n"));
  HiiSetString(HiiHandle, (STRING_REF)STR_SEC_TEMP_DISABLE_PROMPT, TempString, NULL);
  
  ((CHIPSET_CONFIGURATION *)gSUBrowser->SCBuffer)->SeCEOPDone = (UINT8)SeCInfo.SeCEOPDone;

}


EFI_STATUS
InitAdvanceMenu (
  IN EFI_HII_HANDLE                         HiiHandle
  )
{
  EFI_STATUS                                Status;

  Status = EFI_SUCCESS;
  //
  // Check and update IDE configuration.
  //
  gSUBrowser->SUCInfo->PrevSataCnfigure = ((CHIPSET_CONFIGURATION *)gSUBrowser->SCBuffer)->SataInterfaceMode;
#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)
  mIdeConfig = AllocateZeroPool (sizeof(IDE_CONFIG)*8);
  InitIdeConfig (mIdeConfig);
  gSUBrowser->IdeConfig = mIdeConfig;
  Status = CheckIde (
             HiiHandle,
             (CHIPSET_CONFIGURATION *)gSUBrowser->SCBuffer,
             FALSE
             );
//[-start-160816-IB10860206-add]//
  SataDeviceCallBack (HiiHandle);
//[-end-160816-IB10860206-add]//
#endif
  UpdateSeCInformation (HiiHandle);
//[-start-161027-IB07400806-add]//
  UpdateAutoThermalReporting (HiiHandle);
//[-end-161027-IB07400806-add]//
  return Status;
}

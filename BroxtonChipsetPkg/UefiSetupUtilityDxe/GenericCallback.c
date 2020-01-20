/** @file

 GenericCallback function for SetupUtilityDxe Driver

;******************************************************************************
;* Copyright (c) 2012-2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#include "SetupUtility.h"
#include <ScPolicyCommon.h>
#include <Library/VariableSupportLib.h>
#include <Protocol/FormBrowserEx2.h>
#include <Protocol/H2OFormBrowser.h>
#include <Library/DxeOemSvcChipsetLib.h>
#include <ScRegs/RegsPcu.h>
//[-start-160218-IB03090424-add]//
#include <Library/HeciMsgLib.h>
//[-end-160218-IB03090424-add]//
#include <Library/DxeInsydeChipsetLib.h>
//[-start-160923-IB07400789-add]//
#include <Library/CmosLib.h>
#include <ChipsetCmos.h>
//[-end-160923-IB07400789-add]//
//[-start-161114-IB07400814-add]//
#include <Library/IoLib.h>
#include <ScAccess.h>
//[-end-161114-IB07400814-add]//

//[-start-161008-IB07400794-add]//
VOID
LoadOsDefaultValues (
  CHIPSET_CONFIGURATION *FakeNvData
  );
//[-end-161008-IB07400794-add]//
//[-start-161022-IB07400802-add]//
//#if BXTI_PF_ENABLE
VOID
LoadRTDefaultValues (
  CHIPSET_CONFIGURATION  *FakeNvData
  );
//#endif
//[-end-161022-IB07400802-add]//

UINT16                          gSCUSystemHealth;
EFI_EVENT                       gSCUTimerEvent;
BOOLEAN                         mIsSaveWithoutExit;
BOOLEAN                         mPopLoadDefaultDialog = TRUE;
UINT32                          mScuRecord = 0;

EFI_STATUS
EFIAPI
SendSubmitExitNotify (
  VOID
)
{
  EFI_STATUS                         Status;
  H2O_DISPLAY_ENGINE_EVT_SUBMIT_EXIT SubmitExitNotify;
  H2O_FORM_BROWSER_PROTOCOL          *FBProtocol;

  Status = gBS->LocateProtocol (
                   &gH2OFormBrowserProtocolGuid,
                   NULL,
                   (VOID **)&FBProtocol
                   );
  if (!EFI_ERROR (Status)){    
    ZeroMem (&SubmitExitNotify, sizeof (SubmitExitNotify));
    
    SubmitExitNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_SUBMIT_EXIT);
    SubmitExitNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT_EXIT;
    SubmitExitNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;
    
    Status = FBProtocol->Notify (FBProtocol, &SubmitExitNotify.Hdr);
  }

  return Status;
}

STATIC
EFI_STATUS
FullReset (

)
{
  IoWrite8 ((UINTN)R_RST_CNT, (UINT8)V_RST_CNT_HARDSTARTSTATE);
  IoWrite8 ((UINTN)R_RST_CNT, (UINT8)V_RST_CNT_FULLRESET);
  CpuDeadLoop();
  
  return EFI_SUCCESS;
}

/**
  Execute form browser action by EDK2 form browser extension 2 protocol

  @param[in] Action     Execute the request action.
  @param[in] DefaultId  The default Id info when need to load default value.

  @retval EFI_SUCCESS   Successfully execute form browser action
  @retval Other         Locate protocol fail
**/
EFI_STATUS
ExecuteFormBrowserAction (
  IN UINT32                                    Action,
  IN UINT16                                    DefaultId
  )
{
  EFI_STATUS                                   Status;
  EDKII_FORM_BROWSER_EXTENSION2_PROTOCOL       *FormBrowserEx2;

  Status = gBS->LocateProtocol (
                  &gEdkiiFormBrowserEx2ProtocolGuid,
                  NULL,
                  (VOID **)&FormBrowserEx2
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return FormBrowserEx2->ExecuteAction (Action, DefaultId);
}

EFI_STATUS
HotKeyCallBackByAction (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN  EFI_BROWSER_ACTION                    Action,
  IN  EFI_QUESTION_ID                       QuestionId,
  IN  UINT8                                 Type,
  IN  EFI_IFR_TYPE_VALUE                    *Value,
  OUT EFI_BROWSER_ACTION_REQUEST            *ActionRequest
  )
{
  EFI_STATUS                     Status;

  Status = EFI_UNSUPPORTED;

  switch (Action) {

  case EFI_BROWSER_ACTION_CHANGING:
    return EFI_SUCCESS;

  case EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING:
    if (QuestionId == KEY_SCAN_F9) {
      mPopLoadDefaultDialog = FALSE;
      Status = HotKeyCallBack (
                 This,
                 EFI_BROWSER_ACTION_CHANGED,
                 QuestionId,
                 Type,
                 Value,
                 ActionRequest
                 );
      mPopLoadDefaultDialog = TRUE;
      BrowserRefreshFormSet ();
    }
    //
    // avoid GetQuestionDefault execute ExtractConfig, so always return success.
    //
    return EFI_SUCCESS;

  default:
    break;
  }

  return Status;
}

/**
 Check Sec(TXE) SystemConfig variables and do corresponding response.

 @param[in]    SystemConfigPtr   Pointer to SYSTEM_CONFIGURATAION
 
 @retval NONE              
*/
VOID
CheckSecSystemConfig (
  IN  CHIPSET_CONFIGURATION    *SystemConfigPtr
)
{
  EFI_STATUS              Status;
  SEC_OPERATION_PROTOCOL  *SeCOp;
  SEC_INFOMATION          SeCInfo;

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
  SeCInfo.FwUpdate = SystemConfigPtr->SecFirmwareUpdate;
  SeCInfo.HmrfpoEnable = SystemConfigPtr->SecFlashUpdate;

  Status = SeCOp->SetPlatformSeCInfo(
                    &SeCInfo
                    );
}

EFI_STATUS
RestoreVariables(
  VOID
  )
{
  UINTN                                     BufferSize;
  UINT8                                     *BackupLang = NULL;
  EFI_STATUS                                Status      = EFI_NOT_READY;
  UINT8                                     SetupNvData[FixedPcdGet32(PcdSetupConfigSize)];
  SETUP_UTILITY_CONFIGURATION               *SUCInfo    = NULL;

  SUCInfo = gSUBrowser->SUCInfo;

  CopyMem (&SetupNvData, gSUBrowser->SCBuffer, PcdGet32 (PcdSetupConfigSize));
  
  Status = gRT->SetVariable (
                  L"BootOrder",
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  SUCInfo->AdvBootDeviceNum * sizeof (UINT16),
                  (UINT16 *) SUCInfo->BootOrder
                  );

  BackupLang = GetVariableAndSize (
                  L"PlatformLang",
                  &gEfiGlobalVariableGuid,
                  &BufferSize
                  );
  
  Status = gRT->SetVariable (
                  L"BackupPlatformLang",
                  &gEfiGenericVariableGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  BufferSize,
                  BackupLang
                  );
          
  Status = gRT->SetVariable (
                L"Timeout",
                &gEfiGlobalVariableGuid,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                sizeof (UINT16),
                (UINT16 *) &(((CHIPSET_CONFIGURATION *)&SetupNvData)->Timeout)
                );
        
  gBS->FreePool (BackupLang);

  SetSecurityStatus (); 
  CheckSecSystemConfig ((CHIPSET_CONFIGURATION *)&SetupNvData);

  BufferSize = PcdGet32 (PcdSetupConfigSize);
  
  Status = SaveSetupConfig (
             SETUP_VARIABLE_NAME,
             &mFormSetGuid,
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
             BufferSize,
             &SetupNvData
             );

//[-start-160803-IB07220122-remove]//
//   SetSystemConfigData(&SetupNvData, BufferSize);     
//[-end-160803-IB07220122-remove]//

  return Status;
}

/**
 Proccess HotKey function

 @param [in]   This
 @param [in]   HiiHandle
 @param [in]   Action
 @param [in]   QuestionId
 @param [in]   Type
 @param [in]   Value
 @param [out]  ActionRequest


**/
EFI_STATUS
EFIAPI
HotKeyCallBack (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN  EFI_BROWSER_ACTION                    Action,
  IN  EFI_QUESTION_ID                       QuestionId,
  IN  UINT8                                 Type,
  IN  EFI_IFR_TYPE_VALUE                    *Value,
  OUT EFI_BROWSER_ACTION_REQUEST            *ActionRequest
  )
{
  UINTN                                     BufferSize     = 0;
  CHAR16                                    *StringPtr     = NULL;
  UINT16                                    *BootOrderList = NULL;
  EFI_STATUS                                Status = EFI_NOT_READY;
  EFI_INPUT_KEY                             Key;
  CHIPSET_CONFIGURATION                     *MyIfrNVData;
  SETUP_UTILITY_CONFIGURATION               *SUCInfo       = NULL;
  EFI_SETUP_UTILITY_BROWSER_PROTOCOL        *Interface     = NULL;
  UINTN                                     VariableSize;
  VOID                                      *FullSystemConfiguration;
  CHIPSET_CONFIGURATION                     SystemConfiguration;
  EFI_HII_HANDLE                            HiiHandle;
  UINT32                                    VariableAttributes;
//[-start-160923-IB07400789-add]//
  UINT8                                     CmosData;
//[-end-160923-IB07400789-add]//
  
  if (Action != EFI_BROWSER_ACTION_CHANGED) {
    return HotKeyCallBackByAction (This, Action, QuestionId, Type, Value, ActionRequest);
  }
  
  SUCInfo             = gSUBrowser->SUCInfo;
  MyIfrNVData         = (CHIPSET_CONFIGURATION *) gSUBrowser->SCBuffer;
  VariableSize = PcdGet32 (PcdSetupConfigSize);
  FullSystemConfiguration = AllocatePool (VariableSize);
  ASSERT (FullSystemConfiguration != NULL);

  Status              = EFI_SUCCESS;
  HiiHandle           = mDriverHiiHandle;
  *ActionRequest      = EFI_BROWSER_ACTION_REQUEST_NONE;

//[-start-160720-IB03090429-modify]//
  Status = gRT->GetVariable(
                  SETUP_VARIABLE_NAME,
                  &mFormSetGuid,
                  &VariableAttributes,
                  &VariableSize,
                  FullSystemConfiguration
                  );
//[-end-160720-IB03090429-modify]//
  ASSERT_EFI_ERROR(Status);

  if (EFI_ERROR (Status)) {
    Status = GetChipsetSetupVariableDxe ((CHIPSET_CONFIGURATION *)FullSystemConfiguration, sizeof (CHIPSET_CONFIGURATION));  
    if (EFI_ERROR (Status)) {
      return Status;  
    }
  }
   
  ASSERT (sizeof(CHIPSET_CONFIGURATION) <= VariableSize);
  CopyMem (&SystemConfiguration, FullSystemConfiguration, sizeof(CHIPSET_CONFIGURATION));
  FreePool (FullSystemConfiguration);
  switch (QuestionId) {

  case KEY_SCAN_ESC :    
    //
    // Discard setup and exit
    //
    StringPtr = HiiGetString (HiiHandle, STRING_TOKEN (STR_EXIT_DISCARDING_CHANGES_STRING), NULL);
    gSUBrowser->H2ODialog->ConfirmDialog (
                               0,
                               FALSE,
                               0,
                               NULL,
                               &Key,
                               StringPtr
                               );
    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      if (mSetupOptionStatusForFullReset == ChangedWithoutSaved) {
        mSetupOptionStatusForFullReset = NoChanged;
      } else if (mSetupOptionStatusForFullReset == ChangedWithSaved) {
        FullReset ();
      }
      CheckLanguage ();
      *ActionRequest = EFI_BROWSER_ACTION_REQUEST_EXIT;
    }

    gBS->FreePool (StringPtr);
    break;

  case KEY_SCAN_F9 :
    //
    // Load Optimal
    //
    // If user Accesslevel = USER_PASSWORD_VIEW_ONLY(0x02) or USER_PASSWORD_LIMITED(0x03)
    // and use user password into SCU,the user don't use load optimal function.
    //
    if ((MyIfrNVData->SetUserPass == TRUE) &&
        ((MyIfrNVData->UserAccessLevel == 2) ||
        (MyIfrNVData->UserAccessLevel == 3))) {
      return EFI_ABORTED;
    }

    EventTimerControl(0);

    //
    // Load optimal setup.
    //
    Key.UnicodeChar = CHAR_CARRIAGE_RETURN;
    if (mPopLoadDefaultDialog) {    
      StringPtr = HiiGetString (HiiHandle, STRING_TOKEN (STR_LOAD_OPTIMAL_DEFAULTS_STRING), NULL);
      gSUBrowser->H2ODialog->ConfirmDialog (
                                 0,
                                 FALSE,
                                 0,
                                 NULL,
                                 &Key,
                                 StringPtr
                                 );
      gBS->FreePool (StringPtr);
      if (Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
        EventTimerControl(TIMEOUT_OF_EVENT);
        return EFI_UNSUPPORTED;
      }
      if (FeaturePcdGet (PcdH2OFormBrowserSupported)) {
        return ExecuteFormBrowserAction (BROWSER_ACTION_DEFAULT, EFI_HII_DEFAULT_CLASS_STANDARD);
      }
    }


    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      mScuRecord |= SCU_ACTION_LOAD_DEFAULT;
      mSetupOptionStatusForFullReset = ChangedWithoutSaved;  
      MyIfrNVData->Language = 0;
      Status = UpdateLangItem (
                 This,
                 (UINT8 *) &MyIfrNVData->Language
                 );
      if (FeaturePcdGet (PcdH2OFormBrowserSupported)) {
        BrowserRefreshFormSet ();
      } else {
        Status = gBS->LocateProtocol (
                        &gEfiSetupUtilityBrowserProtocolGuid,
                        NULL,
                        (VOID **)&Interface
                        );
        if (EFI_ERROR (Status)) {
          return Status;
        }
        Interface->Firstin = TRUE;
      }
      //
      //Remove this condition, because no loneger use Changelanguage Flag to determine chagelanguage
      //
      UpdatePasswordState (gSUBrowser->SUCInfo->MapTable[SecurityHiiHandle].HiiHandle);

      BootOrderList = GetVariableAndSize (
                        L"BootOrder",
                        &gEfiGlobalVariableGuid,
                        &BufferSize
                        );

      if (BufferSize != 0) {
         gBS->FreePool (SUCInfo->BootOrder);
         SUCInfo->BootOrder = BootOrderList;
         SUCInfo->AdvBootDeviceNum  = (UINT16) (BufferSize / sizeof(UINT16));
       }

      MyIfrNVData->BootMenuType         = NORMAL_MENU;
//[-start-160923-IB07400788-modify]//
//      MyIfrNVData->BootNormalPriority   = EFI_FIRST;
      MyIfrNVData->BootNormalPriority   = 2;
//[-end-160923-IB07400788-modify]//
      MyIfrNVData->LegacyNormalMenuType = NORMAL_MENU;
      //
      //  reset the flag of showing from.
      //  
      DefaultSetup (MyIfrNVData);

#if ENBDT_PF_ENABLE                
      UpdateHDCConfigure (
        gSUBrowser->SUCInfo->MapTable[AdvanceHiiHandle].HiiHandle,
        MyIfrNVData
        );
#endif    

      UpdateStringToken ((KERNEL_CONFIGURATION *)MyIfrNVData);
    } 

    EventTimerControl(TIMEOUT_OF_EVENT);

    break;

  case KEY_SCAN_F10 :  
    //
    // If user Accesslevel = USER_PASSWORD_VIEW_ONLY(0x02) or USER_PASSWORD_LIMITED(0x03)
    // and use user password into SCU,the user don't use load optimal function.
    //
    if ((MyIfrNVData->SetUserPass == TRUE) &&
        (MyIfrNVData->UserAccessLevel == 2)) {
      return EFI_ABORTED;
    }
    EventTimerControl (0);

    if (FeaturePcdGet (PcdH2OFormBrowserSupported)){
      SendSubmitExitNotify ();
      break;
    }
    
    //
    // Save setup and exit.
    //
    StringPtr = HiiGetString (HiiHandle, STRING_TOKEN (STR_EXIT_SAVING_CHANGES_STRING), NULL);
    gSUBrowser->H2ODialog->ConfirmDialog (
                               0,
                               FALSE,
                               0,
                               NULL,
                               &Key,
                               StringPtr
                               );
    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      GenericRouteConfig ((EFI_HII_CONFIG_ACCESS_PROTOCOL *) This, NULL, NULL);
      //
      // set the request action to save all value and then exit
      //
      gBS->RaiseTPL (TPL_NOTIFY);
      gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    }

    gBS->FreePool (StringPtr);
    EventTimerControl (TIMEOUT_OF_EVENT);
    break;

  case KEY_SAVE_WITHOUT_EXIT:      
    StringPtr = HiiGetString (HiiHandle, STRING_TOKEN (STR_SAVE_CHANGE_WITHOUT_EXIT_STRING), NULL);
    gSUBrowser->H2ODialog->ConfirmDialog (
                               0,
                               FALSE,
                               0,
                               NULL,
                               &Key,
                               StringPtr
                               );
    gBS->FreePool (StringPtr);
    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      mIsSaveWithoutExit = TRUE;
      if (MyIfrNVData->ProcessorVmxEnable != SystemConfiguration.ProcessorVmxEnable) {
        mSetupOptionStatusForFullReset = ChangedWithSaved;
      }
      if (FeaturePcdGet (PcdH2OFormBrowserSupported)) {
        ExecuteFormBrowserAction (BROWSER_ACTION_SUBMIT, 0);
      } else {
        GenericRouteConfig ((EFI_HII_CONFIG_ACCESS_PROTOCOL *) This, NULL, NULL);
      }
    } else {
      Status = EFI_UNSUPPORTED;
    }
    break;

  case KEY_DISCARD_CHANGE:   
    //
    // discard setup change.
    //
    StringPtr = HiiGetString (HiiHandle, STRING_TOKEN (STR_DISCARD_CHANGES_STRING), NULL);
    gSUBrowser->H2ODialog->ConfirmDialog (
                               0,
                               FALSE,
                               0,
                               NULL,
                               &Key,
                               StringPtr
                               );
    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      mScuRecord &= (~SCU_ACTION_LOAD_DEFAULT);
      if (FeaturePcdGet (PcdH2OFormBrowserSupported)) {
        BrowserRefreshFormSet ();
        ExecuteFormBrowserAction (BROWSER_ACTION_DISCARD, 0);
      } else {        
        Status = gBS->LocateProtocol (
                        &gEfiSetupUtilityBrowserProtocolGuid,
                        NULL,
                        (VOID**)&Interface
                        );   
        if (EFI_ERROR (Status)) {
          return Status;
        }
        Interface->Firstin = TRUE;
      }
      if (mSetupOptionStatusForFullReset == ChangedWithoutSaved) {
        mSetupOptionStatusForFullReset = NoChanged;
      }
      Status = DiscardChange (This);
      UpdateHDCConfigure (
        gSUBrowser->SUCInfo->MapTable[AdvanceHiiHandle].HiiHandle,
        MyIfrNVData
        );
      SUCInfo->DoRefresh = TRUE;
    }

    gBS->FreePool (StringPtr);
    break;

    case KEY_VT_CONFIG :
      //
      // If VT (ProcessorVmxEnable) has been select, let it FullReset.
      //
      mSetupOptionStatusForFullReset = ChangedWithoutSaved;
      break;
      
//[-start-170321-IB07400848-remove]//
//  case KEY_XHCI_PREBOOT_SUPPORT :
//    //
//    // When using XHCI controller, set XHCI mode to "Enable". Otherwise, set to "Disable".
//    //
//    if (MyIfrNVData->UsbXhciSupport) {
//      MyIfrNVData->ScUsb30Mode = XHCI_MODE_ON;
//    } else {
//      MyIfrNVData->ScUsb30Mode = XHCI_MODE_OFF ;
//    }
//    break;
//[-end-170321-IB07400848-remove]//

//[-start-160420-IB07220069-add]//
  case KEY_BOOT_MODE_TYPE :
    //
    // If it is non-uefi boot type, set 8254 gate to disable.
    //
    switch (MyIfrNVData->BootType) {

    case DUAL_BOOT_TYPE:
    case LEGACY_BOOT_TYPE:
      if (MyIfrNVData->Cg8254 == 1) {
        MyIfrNVData->Cg8254 = 0;
      }
      break;

//[-start-160923-IB07400789-remove]//
    //
    // Default Cg8254 = 0, do not switch back
    //
//    case EFI_BOOT_TYPE :
//      if (MyIfrNVData->Cg8254 == 0) {
//        MyIfrNVData->Cg8254 = 1;
//      }
//      break;
//[-end-160923-IB07400789-remove]//

    default :
      break;
    }
    break;
//[-end-160420-IB07220069-add]//

//[-start-160218-IB03090424-add]//
//[-start-161008-IB07400794-modify]//
  case KEY_DATA_CLEAR_KEYS:
//[-end-161008-IB07400794-modify]//
    //
    // Popup a notification menu
    //
    StringPtr = HiiGetString (HiiHandle, STRING_TOKEN (STR_CLEAR_CSE_DATA_REGION_QUESTION), NULL);
    gSUBrowser->H2ODialog->ConfirmDialog (
                               0,
                               FALSE,
                               0,
                               NULL,
                               &Key,
                               StringPtr
                               );

    //
    // If the user hits the YES Response key
    //
    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
//[-start-160720-IB03090429-modify]//
      //
      // Set a variable and do System restart
      //

//[-start-160803-IB07220122-modify]//
//       Status = GetSystemConfigData (&SystemConfiguration, &VariableSize);

			MyIfrNVData->RpmbDataClear = 1;

// 			Status = SetSystemConfigData (&SystemConfiguration, VariableSize);
      Status = GenericRouteConfig ((EFI_HII_CONFIG_ACCESS_PROTOCOL *) This, NULL, NULL);

      //
      // Reset the system (EfiResetCold)
      // 
      if (Status == EFI_SUCCESS) {
        //
        // set the request action to save all value and then exit
        //
        gBS->RaiseTPL (TPL_NOTIFY);
        gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
      } 
//[-end-160803-IB07220122-modify]//
//      else {
//        StrCpyS(StringBuffer1, 200, L"NVM / RPMB Data Clear - Not able to set flag and issue Reset");
//        StrCpyS(StringBuffer2, 200, L"Press Enter to Continue");
//        do {
//            CreatePopUp(EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer1, StringBuffer2, NULL);
//        } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
//      }
    }
//[-end-160720-IB03090429-modify]//
    
    break;
//[-end-160218-IB03090424-add]//
//[-start-160923-IB07400789-add]//
  case KEY_WIN7_KB_MS_SUPPORT:
    CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature);
    if (MyIfrNVData->Win7KbMsSupport) {
      CmosData |= (B_CMOS_WIN7_VIRTUAL_KBC_SUPPORT);
    } else {
      CmosData &= (~B_CMOS_WIN7_VIRTUAL_KBC_SUPPORT);
    }
    WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature, CmosData);
    break;
//[-end-160923-IB07400789-add]//
//[-start-161008-IB07400794-add]//
  case KEY_OS_SELECTION:
    LoadOsDefaultValues (MyIfrNVData);
    break;
//[-end-161008-IB07400794-add]//
//[-start-161022-IB07400802-add]//
  case KEY_IOTG_RTEN:  //Real Time - RTEn callback
    DEBUG((DEBUG_INFO,"RTEn Callback.\n"));
    LoadRTDefaultValues (MyIfrNVData);
    break;
//[-end-161022-IB07400802-add]//
    
  default :
    break;
  }

  return EFI_SUCCESS;
}

/**
 This function allows a caller to extract the current configuration for one
 or more named elements from the target driver.

 @param [in]   This             Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
 @param [in]   Request          A null-terminated Unicode string in <ConfigRequest> format.
 @param [out]  Progress         On return, points to a character in the Request string.
                                Points to the string's null terminator if request was successful.
                                Points to the most recent '&' before the first failing name/value
                                pair (or the beginning of the string if the failure is in the
                                first name/value pair) if the request was not successful.
 @param [out]  Results          A null-terminated Unicode string in <ConfigAltResp> format which
                                has all values filled in for the names in the Request string.
                                String to be allocated by the called function.

 @retval EFI_SUCCESS            The Results is filled with the requested values.
 @retval EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
 @retval EFI_INVALID_PARAMETER  Request is NULL, illegal syntax, or unknown name.
 @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
GenericExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                    Status;
  UINTN                         BufferSize;
  EFI_STRING                    ConfigRequestHdr;
  EFI_STRING                    ConfigRequest;
  BOOLEAN                       AllocatedRequest;
  UINTN                         Size;
  CHAR16                        *StrPointer;
  EFI_CALLBACK_INFO             *CallbackInfo;
  EFI_HANDLE                    DriverHandle;
  EFI_GUID                      VarStoreGuid = SYSTEM_CONFIGURATION_GUID;


  if (This == NULL || Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Progress        = Request;
  CallbackInfo     = EFI_CALLBACK_INFO_FROM_THIS (This);
  BufferSize       = GetVarStoreSize (CallbackInfo->HiiHandle, &CallbackInfo->FormsetGuid, &VarStoreGuid, "SystemConfig");
  ConfigRequestHdr = NULL;
  ConfigRequest    = NULL;
  AllocatedRequest = FALSE;
  Size             = 0;

  if (Request == NULL) {
    //
    // Request is set to NULL, construct full request string.
    //
    //
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    Status = gSUBrowser->HiiDatabase->GetPackageListHandle (gSUBrowser->HiiDatabase, CallbackInfo->HiiHandle, &DriverHandle);
    if (EFI_ERROR (Status)) {
      return EFI_NOT_FOUND;
    }
    ConfigRequestHdr = HiiConstructConfigHdr (&mFormSetGuid, mVariableName, DriverHandle);
    Size = (StrLen (ConfigRequestHdr) + 32 + 1) * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (Size);
    ASSERT (ConfigRequest != NULL);
    AllocatedRequest = TRUE;
    UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, (UINT64) BufferSize);
    FreePool (ConfigRequestHdr);
    ConfigRequestHdr = NULL;
  } else {
    //
    // Check routing data in <ConfigHdr>.
    // Note: if only one Storage is used, then this checking could be skipped.
    //
    if (!HiiIsConfigHdrMatch (Request, &mFormSetGuid, mVariableName)) {
      return EFI_NOT_FOUND;
    }
    //
    // Set Request to the unified request string.
    //
    ConfigRequest = Request;
    //
    // Check whether Request includes Request Element.
    //
    if (StrStr (Request, L"OFFSET") == NULL) {
      //
      // Check Request Element does exist in Reques String
      //
      StrPointer = StrStr (Request, L"PATH");
      if (StrPointer == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      if (StrStr (StrPointer, L"&") == NULL) {
        Size = (StrLen (Request) + 32 + 1) * sizeof (CHAR16);
        ConfigRequest = AllocateZeroPool (Size);
        ASSERT (ConfigRequest != NULL);
        AllocatedRequest = TRUE;
        UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", Request, (UINT64) BufferSize);
      }
    }
  }

  if (StrStr (ConfigRequest, L"OFFSET") == NULL) {
    //
    // If requesting Name/Value storage, return not found.
    //
    return EFI_NOT_FOUND;
  }

  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //
  Status = gSUBrowser->HiiConfigRouting->BlockToConfig (
                                           gSUBrowser->HiiConfigRouting,
                                           ConfigRequest,
                                           (UINT8 *) gSUBrowser->SCBuffer,
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
//[-start-170125-IB07400837-add]//
// extern EFI_GUID gCrConfigurationGuid;
#define CR_CONFIGURATION_GUID \
{ \
  0x7EC07B9F, 0x66E3, 0x43d4, {0x9B, 0x52, 0x38, 0xFB, 0xB4, 0x63, 0x90, 0xCC}  \
}

#define CONSOLE_REDIRECTION_VARSTORE_NAME  L"CrConfig"

//[-start-180502-IB07400963-remove]//
//#pragma pack(1)
//
//typedef struct {
//  UINT8         PortEnable;
//  UINT8         UseGlobalSetting;
//  UINT8         TerminalType;
//  UINT8         BaudRate;
//  UINT8         Parity;
//  UINT8         DataBits;
//  UINT8         StopBits;
//  UINT8         FlowControl;
//} CR_SERIAL_PORT_CONFIG;
//
//typedef struct {
//  UINT8                  CREnable;
//  UINT8                  CRInfoWaitTime;
//  UINT8                  CRAfterPost;
//  UINT8                  CRTextModeResolution;
//  UINT8                  GlobalTerminalType;
//  UINT8                  GlobalBaudRate;
//  UINT8                  GlobalParity;
//  UINT8                  GlobalDataBits;
//  UINT8                  GlobalStopBits;
//  UINT8                  GlobalFlowControl;
//  UINT8                  AutoRefresh;
//  UINT8                  FailSaveBaudRate;
//  UINT8                  SolEnable;
//  UINT8                  UseDhcp;
//  CR_SERIAL_PORT_CONFIG  CrPortConfig;
//} CR_CONFIGURATION;
//
//#pragma pack()
//[-end-180502-IB07400963-remove]//

EFI_STATUS
LinkPackageSetupSync (
  IN OUT  CHIPSET_CONFIGURATION             *ScBuffer
  ) 
{
  CHAR16              *CrVarName = CONSOLE_REDIRECTION_VARSTORE_NAME;
  EFI_GUID            CrVarGuid = CR_CONFIGURATION_GUID;
  UINTN               BufferSize;
//[-start-180502-IB07400963-modify]//
  UINT8               *CrConfig;
//[-end-180502-IB07400963-modify]//
  EFI_STATUS          Status;

  if (ScBuffer->RotateScreenConfig != 0) {
    //
    // Rotate Screen is enabled, disable console redirection.
    //
//[-start-180502-IB07400963-modify]//
    Status = CommonGetVariableDataAndSize (
               CrVarName,
               &CrVarGuid,
               &BufferSize,
               (VOID **) &CrConfig
               );
    if (Status != EFI_SUCCESS) {
      return Status;
    }
    
    CrConfig[0] = FALSE;
    
    Status = gRT->SetVariable (
      CrVarName,
      &CrVarGuid,
      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
      BufferSize,
      CrConfig
      );
//[-end-180502-IB07400963-modify]//
  }
  return EFI_SUCCESS;
}
//[-end-170125-IB07400837-add]//

//[-start-161114-IB07400814-add]//
EFI_STATUS
CheckDeviceStatus (
  IN OUT  CHIPSET_CONFIGURATION             *ScBuffer
) 
{

  UINTN                                       VariableSize;
  VOID                                        *FullSystemConfiguration;
  CHIPSET_CONFIGURATION                       SystemConfiguration;
  EFI_STATUS                                  Status;
  UINT32                                      FunctionDisable1;
  UINT32                                      FunctionDisable2;
  BOOLEAN                                     TxeGlobalReset = FALSE;

   
  Status = EFI_SUCCESS;
  VariableSize = PcdGet32 (PcdSetupConfigSize);
  FullSystemConfiguration = AllocatePool (VariableSize);
  ASSERT (FullSystemConfiguration != NULL);

  Status = gRT->GetVariable(
                  SETUP_VARIABLE_NAME,
                  &mFormSetGuid,
                  NULL,
                  &VariableSize,
                  FullSystemConfiguration
                  );
  ASSERT_EFI_ERROR(Status);
   
  ASSERT (sizeof(CHIPSET_CONFIGURATION) <= VariableSize);
  CopyMem (&SystemConfiguration, FullSystemConfiguration, sizeof(CHIPSET_CONFIGURATION));
  FreePool (FullSystemConfiguration);

  FunctionDisable1 = MmioRead32 (PcdGet32(PcdPmcIpc1BaseAddress0) + R_PMC_FUNC_DIS);
  FunctionDisable2 = MmioRead32 (PcdGet32(PcdPmcIpc1BaseAddress0) + R_PMC_FUNC_DIS_1);

  if (ScBuffer->PcieRootPortEn[0] != SystemConfiguration.PcieRootPortEn[0]) { // Lane 4, 0/0x14/0
    FunctionDisable1 &= (~B_PMC_FUNC_DIS_PCIE0_P0);
    TxeGlobalReset = TRUE;
  }
  if (ScBuffer->PcieRootPortEn[1] != SystemConfiguration.PcieRootPortEn[1]) { // Lane 5, 0/0x14/1
    FunctionDisable1 &= (~B_PMC_FUNC_DIS_PCIE0_P1);
    TxeGlobalReset = TRUE;
  }
  if (ScBuffer->PcieRootPortEn[2] != SystemConfiguration.PcieRootPortEn[2]) { // Lane 0, 0/0x13/0
    FunctionDisable2 &= (~B_PMC_FUNC_DIS_1_PCIE1_P0);
    TxeGlobalReset = TRUE;
  }
  if (ScBuffer->PcieRootPortEn[3] != SystemConfiguration.PcieRootPortEn[3]) { // Lane 1, 0/0x13/1
    FunctionDisable2 &= (~B_PMC_FUNC_DIS_1_PCIE1_P1);
    TxeGlobalReset = TRUE;
  }
  if (ScBuffer->PcieRootPortEn[4] != SystemConfiguration.PcieRootPortEn[4]) { // Lane 2, 0/0x13/2
    FunctionDisable2 &= (~B_PMC_FUNC_DIS_1_PCIE1_P2);
    TxeGlobalReset = TRUE;
  }
  if (ScBuffer->PcieRootPortEn[5] != SystemConfiguration.PcieRootPortEn[5]) { // Lane 3, 0/0x13/3
    FunctionDisable2 &= (~B_PMC_FUNC_DIS_1_PCIE1_P3);
    TxeGlobalReset = TRUE;
  }
  
  MmioWrite32 (PcdGet32(PcdPmcIpc1BaseAddress0) + R_PMC_FUNC_DIS, FunctionDisable1);
  MmioWrite32 (PcdGet32(PcdPmcIpc1BaseAddress0) + R_PMC_FUNC_DIS_1, FunctionDisable2);

  if (TxeGlobalReset) {
    MmioOr32 (PcdGet32(PcdPmcIpc1BaseAddress0) + R_PMC_ETR, B_PMC_ETR_CF9GR);
  }
  
  return Status;
}
//[-end-161114-IB07400814-add]//

/**
 This function processes the results of changes in configuration.

 @param [in]   This             Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
 @param [in]   Configuration    A null-terminated Unicode string in <ConfigResp> format.
 @param [out]  Progress         A pointer to a string filled in with the offset of the most
                                recent '&' before the first failing name/value pair (or the
                                beginning of the string if the failure is in the first
                                name/value pair) or the terminating NULL if all was successful.

 @retval EFI_SUCCESS            The Results is processed successfully.
 @retval EFI_INVALID_PARAMETER  Configuration is NULL.
 @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
GenericRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  EFI_STATUS  Status;
  UINTN                                     BufferSize;
  EFI_GUID                                  VarStoreGuid = SYSTEM_CONFIGURATION_GUID;
  

  if (Configuration != NULL && !HiiIsConfigHdrMatch (Configuration, &mFormSetGuid, mVariableName)) {
    return EFI_SUCCESS;
  }

  BufferSize = PcdGet32 (PcdSetupConfigSize);
  SetupVariableConfig (
    &VarStoreGuid,
    L"SystemConfig",
    BufferSize,
    (UINT8 *) gSUBrowser->SCBuffer,
    TRUE
    );

//
// Oem hook when F10 or Exit Saving Changes or Save Change Without Exit submit.
// And system will set current setuputility setting to browser.
//
//[-start-161114-IB07400814-add]//
  CheckDeviceStatus ((VOID*)gSUBrowser->SCBuffer);
//[-end-161114-IB07400814-add]//
//[-start-170125-IB07400837-add]//
  LinkPackageSetupSync ((VOID*)gSUBrowser->SCBuffer);
//[-end-170125-IB07400837-add]//
  OemSvcHookRouteConfig ((VOID *)gSUBrowser->SCBuffer, (UINT32)BufferSize, mScuRecord);

  SetupVariableConfig (
    &VarStoreGuid,
    L"SystemConfig",
    BufferSize,
    (UINT8 *) gSUBrowser->SCBuffer,
    FALSE
    );

  Status = RestoreVariables();
  if (mSetupOptionStatusForFullReset != NoChanged) {
    if (!mIsSaveWithoutExit) {
      FullReset();
    }
    mIsSaveWithoutExit = FALSE;
  } else {
    if (!mIsSaveWithoutExit) {
      do {
        gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
      } while(1);
    }
    mIsSaveWithoutExit = FALSE;
  }

  if(Configuration == NULL || Progress == NULL)
    return EFI_INVALID_PARAMETER;

  *Progress = Configuration;

  *Progress = Configuration + StrLen (Configuration);

  return  EFI_SUCCESS;
}

//[-start-161008-IB07400794-add]//
//
// from RC PlatformSetupDxe.c
//
VOID
LoadOsDefaultValues (
  CHIPSET_CONFIGURATION  *FakeNvData
  )
{
//#if !BXTI_PF_ENABLE
  //
  // CCG Board IDs
  //
  if (!FakeNvData->IsIOTGBoardIds) {
    if (FakeNvData->OsSelection == 0) {        // 0 : Default: Windows
//      FakeNvData->GOPEnable                  = TRUE;
//      FakeNvData->SecureBoot                 = TRUE; //Temporary don't touch it for now for WOS
//      FakeNvData->CsmControl                 = FALSE;
      FakeNvData->LowPowerS0Idle             = FALSE;
      FakeNvData->LpssI2C0Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C1Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C2Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C3Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C4Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C5Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C6Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C7Enabled            = 1; //PCI Mode
      FakeNvData->LpssHsuart0Enabled         = 1; //PCI Mode
      FakeNvData->LpssHsuart1Enabled         = 1; //PCI Mode
      FakeNvData->LpssHsuart2Enabled         = 1; //PCI Mode
      FakeNvData->LpssHsuart3Enabled         = 1; //PCI Mode
      FakeNvData->LpssSpi0Enabled            = 1; //PCI Mode
      FakeNvData->LpssSpi1Enabled            = 1; //PCI Mode
      FakeNvData->LpssSpi2Enabled            = 1; //PCI Mode
      FakeNvData->I2s343A                    = 1; //PCI Mode
      FakeNvData->I2s34C1                    = 1; //PCI Mode
      FakeNvData->I2cNfc                     = 1; //PCI Mode
      FakeNvData->I2cPss                     = 1; //PCI Mode
      FakeNvData->UartGps                    = 1; //PCI Mode
      FakeNvData->UartBt                     = 1; //PCI Mode
      FakeNvData->MonitorMwaitEnable         = 2;
    } else if (FakeNvData->OsSelection == 1) { // 1 : Android
//      FakeNvData->GOPEnable                  = TRUE;
//      FakeNvData->SecureBoot                 = FALSE;
//      FakeNvData->CsmControl                 = FALSE;
      FakeNvData->LpssI2C0Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C1Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C2Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C3Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C4Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C5Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C6Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C7Enabled            = 1; //PCI Mode
      FakeNvData->LpssHsuart0Enabled         = 1; //PCI Mode
      FakeNvData->LpssHsuart1Enabled         = 1; //PCI Mode
      FakeNvData->LpssHsuart2Enabled         = 1; //PCI Mode
      FakeNvData->LpssHsuart3Enabled         = 1; //PCI Mode
      FakeNvData->LpssSpi0Enabled            = 1; //PCI Mode
      FakeNvData->LpssSpi1Enabled            = 1; //PCI Mode
      FakeNvData->LpssSpi2Enabled            = 1; //PCI Mode
      FakeNvData->I2s343A                    = 1; //PCI Mode
      FakeNvData->I2s34C1                    = 1; //PCI Mode
      FakeNvData->I2cNfc                     = 1; //PCI Mode
      FakeNvData->I2cPss                     = 1; //PCI Mode
      FakeNvData->UartGps                    = 1; //PCI Mode
      FakeNvData->UartBt                     = 1; //PCI Mode
      FakeNvData->MonitorMwaitEnable         = 2;
    } else if (FakeNvData->OsSelection == 2 || FakeNvData->OsSelection == 4) { // 2 and 3: Win7 and DOS Legacy OS
      DEBUG((DEBUG_INFO,"\n\nCsmControl setting to TRUE\n\n"));
//      FakeNvData->GOPEnable                  = FALSE;
//      FakeNvData->SecureBoot                 = FALSE;
//      FakeNvData->CsmControl                 = TRUE;
      FakeNvData->LowPowerS0Idle             = FALSE;
      FakeNvData->LpssI2C0Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C1Enabled            = 0; //Disable
      FakeNvData->LpssI2C2Enabled            = 0; //Disable
      FakeNvData->LpssI2C3Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C4Enabled            = 1; //PCI Mode
      FakeNvData->LpssI2C5Enabled            = 0; //Disable
      FakeNvData->LpssI2C6Enabled            = 0; //Disable
      FakeNvData->LpssI2C7Enabled            = 0; //Disable
      FakeNvData->LpssHsuart0Enabled         = 0; //Disable
      FakeNvData->LpssHsuart1Enabled         = 0; //Disable
      FakeNvData->LpssHsuart2Enabled         = 0; //Disable
      FakeNvData->LpssHsuart3Enabled         = 0; //Disable
      FakeNvData->LpssSpi0Enabled            = 0; //Disable
      FakeNvData->LpssSpi1Enabled            = 0; //Disable
      FakeNvData->LpssSpi2Enabled            = 0; //Disable
      FakeNvData->I2s343A                    = 0; //Disable
      FakeNvData->I2s34C1                    = 0; //Disable
      FakeNvData->I2cNfc                     = 0; //Disable
      FakeNvData->I2cPss                     = 0; //Disable
      FakeNvData->UartGps                    = 0; //Disable
      FakeNvData->UartBt                     = 0; //Disable
      FakeNvData->MonitorMwaitEnable         = 2;
    }
//#else
  } else {
    //
    // IOTG Board IDs
    //
    if (FakeNvData->OsSelection == 0) {        // 0 : Default: Windows
//      FakeNvData->SecureBoot                 = TRUE;
//      FakeNvData->Cg8254                     = TRUE;
      FakeNvData->Cg8254                     = FALSE; //default disable to support legacy boot
//      FakeNvData->MonitorMwaitEnable         = 1;
      FakeNvData->MonitorMwaitEnable         = 2; //defulat auto
  	  FakeNvData->TcoLock                    = 0;
  	  FakeNvData->Ipc1Enable                 = 0;
      FakeNvData->ScHdAudioNhltEndpointDmic  = 1;
      FakeNvData->ScHdAudioNhltEndpointBt    = 1;
      FakeNvData->ScHdAudioIoBufferOwnership = 0;
      FakeNvData->LowPowerS0Idle             = FALSE;
      FakeNvData->PmSupport                  = TRUE;
  	  FakeNvData->ScUsbOtg                   = 0;
//[-start-171031-IB07400922-remove]//
//      FakeNvData->IpuAcpiMode                = 1;  // SA IPU ACPI mode set to IGFX Child
//      FakeNvData->WorldCameraSel             = 2;  // Rear Camera set to STR_OV13858
//      FakeNvData->UserCameraSel              = 1;  // Front Camera set to OV2740
//[-end-171031-IB07400922-remove]//
    } else if (FakeNvData->OsSelection == 1) { // 1 : Android
//      FakeNvData->SecureBoot                 = FALSE;
      FakeNvData->Cg8254                     = FALSE;
      FakeNvData->MonitorMwaitEnable         = 0;
  	  FakeNvData->TcoLock                    = 0;
  	  FakeNvData->Ipc1Enable                 = 1;
      FakeNvData->ScHdAudioNhltEndpointDmic  = 1;
      FakeNvData->ScHdAudioNhltEndpointBt    = 0;
      FakeNvData->ScHdAudioIoBufferOwnership = 0;
      FakeNvData->LowPowerS0Idle             = TRUE;
      FakeNvData->PmSupport                  = TRUE;
  	  FakeNvData->ScUsbOtg                   = 0;
//[-start-171031-IB07400922-remove]//
//      FakeNvData->IpuAcpiMode                = 1;  // SA IPU ACPI mode set to IGFX Child
//      FakeNvData->WorldCameraSel             = 2;  // Rear Camera set to STR_OV13858
//      FakeNvData->UserCameraSel              = 1;  // Front Camera set to OV2740
//[-end-171031-IB07400922-remove]//
    } else if (FakeNvData->OsSelection == 2) { // 2: Win7 and DOS Legacy OS
//      FakeNvData->SecureBoot                 = FALSE;
//      FakeNvData->Cg8254                     = FALSE;
//      FakeNvData->MonitorMwaitEnable         = 1;
//  	  FakeNvData->TcoLock                    = 0;
//  	  FakeNvData->Ipc1Enable                 = 0;
//      FakeNvData->ScHdAudioNhltEndpointDmic  = 1;
//      FakeNvData->ScHdAudioNhltEndpointBt    = 1;
//      FakeNvData->ScHdAudioIoBufferOwnership = 0;
//      FakeNvData->LowPowerS0Idle             = FALSE;
//      FakeNvData->PmSupport                  = TRUE;
//  	  FakeNvData->ScUsbOtg                   = 0;
//[-start-171031-IB07400922-remove]//
//      FakeNvData->IpuAcpiMode                = 1;  // SA IPU ACPI mode set to IGFX Child
//      FakeNvData->WorldCameraSel             = 2;  // Rear Camera set to STR_OV13858
//      FakeNvData->UserCameraSel              = 1;  // Front Camera set to OV2740
//[-end-171031-IB07400922-remove]//
    } else if (FakeNvData->OsSelection == 3) { // 3 : Intel Linux
//      FakeNvData->SecureBoot                 = FALSE;
      FakeNvData->Cg8254                     = FALSE;
      FakeNvData->MonitorMwaitEnable         = 0;
  	  FakeNvData->TcoLock                    = 0;
  	  FakeNvData->Ipc1Enable                 = 1;
      FakeNvData->ScHdAudioNhltEndpointDmic  = 0;
      FakeNvData->ScHdAudioNhltEndpointBt    = 0;
      FakeNvData->ScHdAudioIoBufferOwnership = 3;
      FakeNvData->LowPowerS0Idle             = TRUE;
      FakeNvData->PmSupport                  = TRUE;
  	  FakeNvData->ScUsbOtg                   = 0;
//[-start-171031-IB07400922-remove]//
//      FakeNvData->IpuAcpiMode                = 0;  // SA IPU ACPI mode set to disable
//      FakeNvData->WorldCameraSel             = 0;  // Rear Camera set to disable
//      FakeNvData->UserCameraSel              = 0;  // Front Camera set to disable
//[-end-171031-IB07400922-remove]//
    }
    //
    // Default disable CCG devices
    //
    FakeNvData->I2s343A                    = 0; //Disable
    FakeNvData->I2s34C1                    = 0; //Disable
    FakeNvData->I2cNfc                     = 0; //Disable
    FakeNvData->I2cPss                     = 0; //Disable
    FakeNvData->UartGps                    = 0; //Disable
    FakeNvData->UartBt                     = 0; //Disable
//#endif
  }
}
//[-end-161008-IB07400794-add]//
//[-start-161022-IB07400802-add]//
//[-start-190314-IB07401090-modify]//
//
// Doc#570618 Rev 1.4, Section 2.5, Sample Code to Enable RT Mode in BIOS  
//
//#if BXTI_PF_ENABLE
VOID
LoadRTDefaultValues (
  CHIPSET_CONFIGURATION  *FakeNvData
  )
{
  UINT8  PortIndex;

  if (FakeNvData->RTEn == 0) { //Real Time Disabled
    FakeNvData->EnableGv = 1;  //enable SpeedStep
    FakeNvData->EnableCx = 1;  //enable C-States
    for (PortIndex = 0; PortIndex < SC_MAX_PCIE_ROOT_PORTS; PortIndex++) {
      FakeNvData->PcieRootPortAspm[PortIndex] = 4; // Auto
    }
    //
    // Root Port Enable/Disable by OEM.
    //
//    switch (FakeNvData->BoardId) { 
//      case BOARD_ID_BFH_IVI: 
//      case BOARD_ID_MRB_IVI: 
//      case BOARD_ID_MNH_RVP: 
//        FakeNvData->PcieRootPortEn[0] = 1; //Enable pcie root port 1 
//        FakeNvData->PcieRootPortEn[1] = 1; //Enable pcie root port 2 
//        break; 
//      case BOARD_ID_OXH_CRB: 
//      case BOARD_ID_JNH_CRB: 
//      case BOARD_ID_LFH_CRB: 
//        FakeNvData->PcieRootPortEn[0] = 2;  //Auto pcie root port 1 
//        FakeNvData->PcieRootPortEn[1] = 2;  //Auto pcie root port 2 
//      default: 
//        break; 
//    } 
  } else { //Real Time Enable
    FakeNvData->EnableGv = 0;  //disable SpeedStep
    FakeNvData->EnableCx = 0;  //disable C-States
    for (PortIndex = 0; PortIndex < SC_MAX_PCIE_ROOT_PORTS; PortIndex++) {
      FakeNvData->PcieRootPortAspm[PortIndex] = 0; // disable ASPM
    }
    //
    // Root Port Enable/Disable by OEM.
    //
//    FakeNvData->PcieRootPortEn[0] = 1; //Enable pcie root port 1 
//    FakeNvData->PcieRootPortEn[1] = 1; //Enable pcie root port 2 
  }
}
//#endif
//[-end-190314-IB07401090-modify]//
//[-end-161022-IB07400802-add]//

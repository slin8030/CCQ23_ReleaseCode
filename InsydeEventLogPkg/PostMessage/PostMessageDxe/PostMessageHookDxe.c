/** @file

  POST Message Hook Dxe implementation.

;******************************************************************************
;* Copyright (c) 2014 - 2018, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/ 

#include "PostMessageHookDxe.h"

//
// Extern Variables
//
extern POST_MESSAGE_LIST                     gPostMessageList[];
extern OEM_POST_MESSAGE_STRING               gOemPostMessageStr[];
extern OEM_POST_MESSAGE_LOG                  gOemPostMessageLog[];
extern OEM_POST_MESSAGE_BEEP                 gOemPostMessageBeep[];

extern UINTN                                 gPostMessageListSize;
extern UINTN                                 gOemPostMessageStrSize;
extern UINTN                                 gOemPostMessageLogSize;
extern UINTN                                 gOemPostMessageBeepSize;

extern EFI_PHYSICAL_ADDRESS                  gPostMessageBuffer;  
extern EFI_PHYSICAL_ADDRESS                  gPostMsgStatusCodeBuffer;  
extern UINT8                                 gPostMessageEn;
extern BOOLEAN                               gProgressCodeEn;
extern BOOLEAN                               gErrorCodeEn;
extern BOOLEAN                               gDebugCodeEn;
extern BOOLEAN                               gLogPostMsg;
extern BOOLEAN                               gShowPostMsg;
extern BOOLEAN                               gBeepPostMsg;
extern BOOLEAN                               gStalltoShow;
extern BOOLEAN                               gSupportPostMessageFun;
extern BOOLEAN                               gOemHookOnly;

/**
  Call Setup Utility by loading Setup Utility application.

  @retval EFI_SUCCESS    Call Setup Utility application successful
  @retval EFI_NOT_FOUND  Can not find Setup Utility application or load image fail
**/
EFI_STATUS
CallScuApp (
  VOID
  )
{
  EFI_STATUS                          Status;
  UINTN                               HandleCount;
  EFI_HANDLE                          *HandleBuffer;
  EFI_GUID                            FileNameGuid = {0x3935B0A1, 0xA182, 0x4887, {0xbc, 0x56, 0x67, 0x55, 0x28, 0xe7, 0x88, 0x77}};
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH   FvFilePath;
  UINTN                               Index;
  EFI_FIRMWARE_VOLUME2_PROTOCOL       *Fv;
  UINTN                               Size;
  EFI_FV_FILETYPE                     Type;
  EFI_FV_FILE_ATTRIBUTES              Attributes;
  UINT32                              AuthenticationStatus;
  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL            *SetupAppDevicePath;
  EFI_HANDLE                          ImageHandle;
  UINTN                               ExitDataSize;
  CHAR16                              *ExitData;

  HandleCount  = 0;
  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolume2ProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (Status != EFI_SUCCESS || HandleBuffer == NULL) {
    return EFI_NOT_FOUND;
  }

  EfiInitializeFwVolDevicepathNode (&FvFilePath, &FileNameGuid);

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiFirmwareVolume2ProtocolGuid,
                    (VOID **) &Fv
                    );
    if (!EFI_ERROR (Status)) {
      Status = Fv->ReadFile (
                     Fv,
                     &FileNameGuid,
                     NULL,
                     &Size,
                     &Type,
                     &Attributes,
                     &AuthenticationStatus
                     );
    }
    if (EFI_ERROR (Status)) {
      continue;
    }

    //
    // Create device path of setup utility application
    //
    DevicePath = DevicePathFromHandle (HandleBuffer[Index]);
    if (DevicePath == NULL) {
      continue;
    }
    SetupAppDevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *) &FvFilePath);
    if (SetupAppDevicePath == NULL) {
      continue;
    }

    Status = gBS->LoadImage (
                    TRUE,
                    gImageHandle,
                    SetupAppDevicePath,
                    NULL,
                    0,
                    &ImageHandle
                    );
    gBS->FreePool (SetupAppDevicePath);
    if (EFI_ERROR (Status)) {
      continue;
    }

    gBS->StartImage (ImageHandle, &ExitDataSize, &ExitData);
    break;
  }

  gBS->FreePool (HandleBuffer);

  return Status;
}


/**
 

 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
PostMessageHobHandleHook (
  IN OUT PEI_POST_MESSAGE_DATA_HOB           *PeiPostMessageHob  
  )
{
  //
  // Default sample is just log Code Type and Code Value
  //
  PostMessageStatusCode (PeiPostMessageHob->CodeType, PeiPostMessageHob->CodeValue, 0, NULL, NULL);

  return EFI_SUCCESS;
}

/**
 This function is a hnadler for OEM/ODM to create a specific function to handle post message.         
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
PostMessageStatusCodeHook (
  IN EFI_STATUS_CODE_TYPE           CodeType,
  IN EFI_STATUS_CODE_VALUE          CodeValue,
  IN UINT32                         Instance    OPTIONAL,
  IN EFI_GUID                       * CallerId  OPTIONAL,
  IN EFI_STATUS_CODE_DATA           * Data      OPTIONAL  
  ) 
{
  UINTN                           Index;

  if (((CodeType == EFI_PROGRESS_CODE) && (gProgressCodeEn == FALSE))  ||
    ((CodeType == EFI_ERROR_CODE)      && (gErrorCodeEn    == FALSE))  ||
    ((CodeType == EFI_DEBUG_CODE)      && (gDebugCodeEn    == FALSE))) {  
    return EFI_UNSUPPORTED;
  }

  for (Index = 0; Index < gPostMessageListSize; Index++) {
    if ((gPostMessageList[Index].CodeType==CodeType) && (gPostMessageList[Index].CodeValue==CodeValue)) {      
      
//[-start-171212-IB08400542-modify]//
      if (FeaturePcdGet(PcdH2OEventLogSupported) != 0) {    
        if (gLogPostMsg==TRUE) {
          //
          // Log Post Message
          //
          if ((gSupportPostMessageFun==TRUE)&&(gPostMessageEn==TRUE)) {
            if ((gPostMessageList[Index].BehaviorBitMap&BEHAVIOR_LOG_TO_STORAGE)==BEHAVIOR_LOG_TO_STORAGE) {
              EventLogHandler (CodeType, CodeValue, Instance, CallerId, Data);
            }
          }
        }
      }        

      if (FeaturePcdGet(PcdH2OPostBeepSupported) != 0) {
        if (gBeepPostMsg==TRUE) {
          //
          // Event Beep
          //
          if ((gSupportPostMessageFun==TRUE)&&(gPostMessageEn==TRUE)) {
            if ((gPostMessageList[Index].BehaviorBitMap&BEHAVIOR_EVENT_BEEP)==BEHAVIOR_EVENT_BEEP) {
              PostBeepHandler (CodeType, CodeValue, Instance, CallerId, Data);
            }
          }
        }
      }      
    }
  }

  //
  // Check if OEM hook function only.
  //
  if (PcdGetBool (PcdH2OPostMessageOemHookEnable) == TRUE) {
    return EFI_ABORTED;
  } else {
    return EFI_SUCCESS;
  }
//[-end-171212-IB08400542-modify]//
  
}

/**
 This function is a hnadler for OEM/ODM to create a specific function to handle Post message before boot to OS.           
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
PublishPostMessageTableHook (
  VOID
  )   
{

  //
  // Example - Show Post Message on screen before boot to OS
  //
  if (gShowPostMsg==TRUE) {
    ShowPostMessage ();
  }
  return EFI_SUCCESS;
}

/**
 
 ScanOtherDevices() provide to scan special deivce state.
   Example: Keyboard populated?? SATA PORT populated?? IDE OR AHCI mode?? and more device work state.        
 
 @retval EFI Status                  
*/
VOID
EFIAPI
ScanOtherDevices (
  VOID
  )    
{  
  //
  // Please Modulize your function.
  //


}

/**
 
 Do a while loop to show attention and Post messages on screen.
 F1 key: exit loop and then continue to POST.           
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
ShowPostMessage (
  VOID
  )
{
  EFI_STATUS                                  Status;
  EFI_INPUT_KEY                               InputKey;
  UINTN                                       Index;
  UINT32                                      TmpAttribute;
  EFI_TPL                                     OldTpl;

  OldTpl = 0;

  TmpAttribute = gST->ConOut->Mode->Attribute;
  gST->ConOut->ClearScreen (gST->ConOut);  
  
  ZeroMem (&InputKey, sizeof (EFI_INPUT_KEY));

  Status = PopPostMessage ();

  if (Status == EFI_UNSUPPORTED) {
    //
    // No Event Data will Show.
    //
    goto  SHOW_EXIT;
  }
  
  if (gStalltoShow==TRUE) {
    //
    // Just stall 3 sec and then continune to BIOS POST
    //
    gBS->Stall(3000000);
    gST->ConOut->ClearScreen (gST->ConOut);
    Status = EFI_SUCCESS;
    goto  SHOW_EXIT;
  }
  gST->ConOut->OutputString(
                         gST->ConOut,
                         L"POST Message - Please press F1 key to continue!!\r\n"
                         );

  gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);

  OldTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL); 
  gBS->RestoreTPL (TPL_DRIVER); 

  //
  // Entry Post Message loop until pressed F1 key
  //
  do {

    if ((InputKey.ScanCode != 0) || (InputKey.UnicodeChar != (CHAR16)0)) {

      gST->ConOut->OutputString(
                             gST->ConOut,
                             L"POST Message - Please press F1 key to continue!!\r\n"
                             );

    }

//    if (InputKey.ScanCode == SCAN_F2) {
//      //
//      // Go to System Setup Program
//      //
//      CallScuApp ();
//      PopPostMessage ();
//    } 

    gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index);
    gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);
  } while (InputKey.ScanCode != SCAN_F1);


SHOW_EXIT:
  //
  // Restore previous console attribute
  //
  gST->ConOut->SetAttribute (gST->ConOut, TmpAttribute);
  gST->ConOut->ClearScreen (gST->ConOut);
  
  if (OldTpl != 0) {
    gBS->RaiseTPL (TPL_HIGH_LEVEL); 
    gBS->RestoreTPL (OldTpl);
  }
  
  return Status;
}

/**
 Show all of POST messages from linking list to screen.        
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
PopPostMessage (
  VOID
  )  
{  
  POST_MESSAGE_HEAD_STRUCTURE            *PostMessageHead = NULL;
  CHAR16                                  *StoreAddr;
  
  PostMessageHead = (POST_MESSAGE_HEAD_STRUCTURE *)(UINTN)gPostMessageBuffer;
  
  if (PostMessageHead->UpmEndAddr > PostMessageHead->UpmStartAddr) {
    StoreAddr = (CHAR16 *)(UINTN)PostMessageHead->UpmStartAddr;
    gST->ConOut->OutputString( gST->ConOut, StoreAddr); 
    return EFI_SUCCESS;
  }
  
  return EFI_UNSUPPORTED;
}

/**
 Log event data to the Event Storages(E.g. BIOS, BMC SEL, Memory, DCMI SEL,.....).     
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EventLogHandler (
  IN EFI_STATUS_CODE_TYPE           CodeType,
  IN EFI_STATUS_CODE_VALUE          CodeValue,
  IN UINT32                         Instance    OPTIONAL,
  IN EFI_GUID                       * CallerId  OPTIONAL,
  IN EFI_STATUS_CODE_DATA           * Data      OPTIONAL  
  )
{
  EFI_STATUS                         Status;
  EVENT_LOG_PROTOCOL                 *EventHandler;
  UINTN                              Index;
  BOOLEAN                            LogEvent=FALSE;
  EVENT_TYPE_ID                      EventTypeId;
  UINT8                              Data8[3]={0};
//[-start-171006-IB08400522-add]//
  UINT8                              LogEventTo;
//[-end-171006-IB08400522-add]//
  
  for (Index = 0; Index < gOemPostMessageLogSize; Index++) {
    if ((gOemPostMessageLog[Index].CodeType == CodeType) && (gOemPostMessageLog[Index].CodeValue == CodeValue)) {
      //
      // We got it. Log it.
      //
      LogEvent = TRUE;
      break;
    }
  }

  if (LogEvent == FALSE) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (&gH2OEventLogProtocolGuid, NULL, (VOID **)&EventHandler);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Arrange data and log it.
  //
  EventTypeId.SensorType = gOemPostMessageLog[Index].EventID.SensorType;
  EventTypeId.SensorNum  = gOemPostMessageLog[Index].EventID.SensorNum;
  EventTypeId.EventType  = gOemPostMessageLog[Index].EventID.EventType;
  Data8[0] = gOemPostMessageLog[Index].EventData[0];
  Data8[1] = gOemPostMessageLog[Index].EventData[1];
  Data8[2] = gOemPostMessageLog[Index].EventData[2];
  
//[-start-171006-IB08400522-modify]//
//[-start-171212-IB08400542-modify]//
  LogEventTo = PcdGet8 (PcdH2OEventLogTarget);
//[-end-171212-IB08400542-modify]//
  Status = EventHandler->LogEvent (LogEventTo, &EventTypeId, Data8, 3);  
  if (EFI_ERROR (Status)) {
    return Status;
  }
//[-end-171006-IB08400522-modify]//
  return EFI_SUCCESS;

}

/**
 

 @param[in]         This                
 @param[in]         SelId               
 @param[out]        SelData             
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
PostBeepHandler (
  IN EFI_STATUS_CODE_TYPE           CodeType,
  IN EFI_STATUS_CODE_VALUE          CodeValue,
  IN UINT32                         Instance    OPTIONAL,
  IN EFI_GUID                       * CallerId  OPTIONAL,
  IN EFI_STATUS_CODE_DATA           * Data      OPTIONAL  
  )
{
  EFI_STATUS                         Status=EFI_UNSUPPORTED;
  UINTN                              Index;
  EFI_SPEAKER_IF_PROTOCOL            *Speaker = NULL;
  UINT8                              BeepDataCount=0;
  UINT8                              BeepLoopIndex=0;

  Status = gBS->LocateProtocol (
                &gEfiSpeakerInterfaceProtocolGuid,
                NULL,
                 (VOID **)&Speaker
                );
  if(EFI_ERROR(Status)) {
    return Status;
  }

   for (Index = 0; Index < gOemPostMessageBeepSize; Index++) {
    if ((gOemPostMessageBeep[Index].CodeValue == CodeValue)) {
      //
      // We got it. Beep it.
      //
      for (BeepLoopIndex=0; BeepLoopIndex<gOemPostMessageBeep[Index].BeepLoop; BeepLoopIndex++) {
        BeepDataCount=0;
        while (gOemPostMessageBeep[Index].BeepData[BeepDataCount].BeepDuration != 0x0){
          Status = Speaker->GenerateBeep (Speaker, 
                                          0x1, 
                                          gOemPostMessageBeep[Index].BeepData[BeepDataCount].BeepDuration,
                                          gOemPostMessageBeep[Index].BeepData[BeepDataCount].TimerInterval );

          BeepDataCount++;
        }
      }
      return Status;
    }
  }
   
  return Status;

}


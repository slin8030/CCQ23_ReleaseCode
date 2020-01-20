/** @file

  BIOS Storage kernel Dxe implementation.

;******************************************************************************
;* Copyright (c) 2016-2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/ 


#include <BiosStorageKernelDxe.h>

EFI_EVENT_LOG_INSTANCE                *mELPrivate;
EFI_EVENT_LOG_INSTANCE                *SmmELPrivate;
EFI_SMM_SYSTEM_TABLE2                 *gSmst=NULL;

LIST_ENTRY                            mBiosStorageKernelList;
BOOLEAN                               mClearBiosStorageKernel;
BOOLEAN                               mSwitchToSmm = FALSE; // Switch the GPNV region access to SMM driver.

extern EFI_RUNTIME_SERVICES           *gRT;

EFI_EVENT                             mVarWriteEvent;
/**
 
 Write event log emulation function which is used before gEfiNonVolatileVariableProtocolGuid is installed.
 Event log will temporarily be stored in allocated pool and be recorded in our temp event log list.          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EmuWriteEventLog (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This,
  IN  UINT8                             EventLogType,
  IN  UINT32                            PostBitmap1,
  IN  UINT32                            PostBitmap2,
  IN  UINTN                             OptionDataSize,
  IN  UINT8                             *OptionLogData
  )
{
  EFI_STATUS                            Status;
  EFI_EVENT_LOG_DATA                    *EventLog;
  UINTN                                 DataFormatTypeLength;
  UINTN                                 NumBytes;
  EL_TIME                               ElTime;
  
  Status = gBS->AllocatePool (
                  EfiBootServicesData, 
                  sizeof (EFI_EVENT_LOG_DATA) ,
                  (VOID **)&EventLog
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  EventLog->Signature = EVENT_LOG_DATA_SIGNATURE;

  DataFormatTypeLength = 0;
  switch (EventLogType) {

  case EfiEventLogTypePostError:
    DataFormatTypeLength = BIOS_EVENT_LOG_DATA_FORMAT_POST_RESULT_BITMAP_LENGTH;
    break;

  default:
    DataFormatTypeLength = BIOS_EVENT_LOG_DATA_FORMAT_NONE_LENGTH;
    break;
  }
  NumBytes = BIOS_EVENT_LOG_BASE_LENGTH + DataFormatTypeLength + OptionDataSize;

  Status = gBS->AllocatePool (
                  EfiBootServicesData, 
                  NumBytes,
                  (VOID **)&(EventLog->Buffer)
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (EventLog);
    return Status;
  }

  ZeroMem (EventLog->Buffer, NumBytes);

  Status = EventLogGetTimeFromCmos (&ElTime);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  EventLog->Buffer->Year   = ElTime.Year;
  EventLog->Buffer->Month  = ElTime.Month;
  EventLog->Buffer->Day    = ElTime.Day;
  EventLog->Buffer->Hour   = ElTime.Hour;
  EventLog->Buffer->Minute = ElTime.Minute;
  EventLog->Buffer->Second = ElTime.Second;
  EventLog->Buffer->Type   = EventLogType;
  EventLog->Buffer->Length = (UINT8) NumBytes;

  if (EventLogType == EfiEventLogTypePostError) {
    *(UINT32 *) (EventLog->Buffer->Data)             = PostBitmap1;
    *(UINT32 *) (UINTN) (&EventLog->Buffer->Data[4]) = PostBitmap2;
  }

  if (OptionLogData != NULL) {
    CopyMem ((UINT8*) ((UINTN) EventLog->Buffer + NumBytes - OptionDataSize), OptionLogData, OptionDataSize);
  }

  InsertTailList (&mBiosStorageKernelList, &EventLog->Link);
  
  return EFI_SUCCESS;
}


/**
 
 Read next event log emulation function which is used before gEfiNonVolatileVariableProtocolGuid is installed.
 This function will search next event log from Hob list and our temp event log list.

 @param[in]         This                
 @param[in]         EventListAddress           
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EmuReadNextEventLog (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This,
  IN OUT VOID                           **EventListAddress
  )
{
  BOOLEAN                              GetFirstEventLog;
  EFI_STATUS                           Status;
  EFI_EVENT_LOG_DATA                   *EventLog;
  VOID                                 *HobList;
  BIOS_EVENT_LOG_ORGANIZATION          *HobLog;
  LIST_ENTRY                           *Link;

  GetFirstEventLog = FALSE;

  //
  // First, read event log in Hob list which is created during PEI phase.
  // But if perform clear event log at this POST time, there is no need to read event log from Hob list.
  //
  if (!mClearBiosStorageKernel) {
    Status = EfiGetSystemConfigurationTable (&gEfiHobListGuid, (VOID **)&HobList);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (*EventListAddress == NULL) {
//[-start-171212-IB08400542-modify]//
      HobLog = GetNextGuidHob (&gH2OBiosEventLogHobGuid, HobList);
//[-end-171212-IB08400542-modify]//
      if (HobLog != NULL) {
        *EventListAddress = (VOID *)((UINT8 *)HobLog + sizeof (EFI_HOB_GUID_TYPE));
        return EFI_SUCCESS;
      }
    }

    for (;;) {
//[-start-171212-IB08400542-modify]//
      HobLog = GetNextGuidHob (&gH2OBiosEventLogHobGuid, HobList);
//[-end-171212-IB08400542-modify]//
      if (HobLog == NULL) {
        break;
      }
      HobLog = (BIOS_EVENT_LOG_ORGANIZATION *)((UINT8 *)HobLog + sizeof (EFI_HOB_GUID_TYPE));
      if (HobLog == (BIOS_EVENT_LOG_ORGANIZATION *) *EventListAddress) {
//[-start-171212-IB08400542-modify]//
        HobLog = GetNextGuidHob (&gH2OBiosEventLogHobGuid, HobList);
//[-end-171212-IB08400542-modify]//
        if (HobLog == NULL) {
          GetFirstEventLog = TRUE;
          break;
        }
        *EventListAddress = (VOID *)((UINT8 *)HobLog + sizeof (EFI_HOB_GUID_TYPE));
        return EFI_SUCCESS;
      }
    }
  }

  if (IsListEmpty (&mBiosStorageKernelList)) {
    return EFI_NOT_FOUND;
  }

  //
  // Get first event log.
  //
  if (*EventListAddress == NULL || GetFirstEventLog) {
    Link = mBiosStorageKernelList.ForwardLink;
    EventLog = DATA_FROM_EFI_EVENT_LOG_THIS(Link);
    *EventListAddress = (VOID *) EventLog->Buffer;
    return EFI_SUCCESS;
  }

  //
  // Keep searching until match the input event log, then return next event log.
  //
  Link = mBiosStorageKernelList.ForwardLink;
  while (Link != &mBiosStorageKernelList) {
    EventLog = DATA_FROM_EFI_EVENT_LOG_THIS(Link);
    if (EventLog->Buffer == *EventListAddress) {
      Link = Link->ForwardLink;
      if (Link == &mBiosStorageKernelList) {
        return EFI_NOT_FOUND;
      }
      
      EventLog = DATA_FROM_EFI_EVENT_LOG_THIS(Link);
      *EventListAddress = (VOID *) EventLog->Buffer;
      return EFI_SUCCESS;
    }
    
    Link = Link->ForwardLink;
  }

  return EFI_NOT_FOUND;
}

/**
 
 Clear event log emulation function which is used before gEfiNonVolatileVariableProtocolGuid is installed.
 It will set event logs, which are stored in Hob list, are invalid and clear our temp event log list.

 @param[in]         This                          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EmuClearEventLog (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL      *This
  )
{
  EFI_STATUS                           Status;
  LIST_ENTRY                           *Link;
  EFI_EVENT_LOG_DATA                   *EventLog;
  
  mClearBiosStorageKernel = TRUE;
  
  Link = mBiosStorageKernelList.ForwardLink;
  while (Link != &mBiosStorageKernelList) {
    EventLog = DATA_FROM_EFI_EVENT_LOG_THIS(Link);
    Link = Link->ForwardLink;
    gBS->FreePool (EventLog->Buffer);
    gBS->FreePool (EventLog);
  }
  InitializeListHead (&mBiosStorageKernelList);
  
  Status = This->Write (
                   This,
                   EfiEventLogTypeLogAreaResetCleared,
                   0,
                   0,
                   0,
                   NULL
                   );
  return Status;
}

/**
 Smart detect current flash device type

 @param[in]         This                
 @param[in]         Buffer           
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
FlashFdDetectDevice (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This,
  OUT UINT8                             *Buffer
  )
{
  EFI_STATUS                            Status;
//[-start-180724-IB08400617-remove]//
//  EFI_EVENT_LOG_INSTANCE                *ELPrivate;
//[-end-180724-IB08400617-remove]//

  mELPrivate = INSTANCE_FROM_EFI_EVENT_LOG_THIS (This);

  Status = GetFlashDevice (&(mELPrivate->DevicePtr));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Buffer != NULL) {
    CopyMem (Buffer, mELPrivate->DevicePtr, sizeof (FLASH_DEVICE));
  }
  
  return Status;
}

/**
 Get the HOB of Event log.

 @param[in]         This                          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
GetEventLogHob (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL      *This
  )
{
  VOID                                 *HobList;
  VOID                                 *Buffer;
  EFI_STATUS                           Status;
  BIOS_EVENT_LOG_ORGANIZATION          *HobLog;
  BIOS_EVENT_LOG_ORGANIZATION          *CurrentLog;
  EFI_EVENT_LOG_INSTANCE               *EPrivate;
  UINTN                                 Length;

  EPrivate = INSTANCE_FROM_EFI_EVENT_LOG_THIS (This);
  CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)(UINTN)EPrivate->GPNVBase;

  while ((CurrentLog->Type != 0xFF) && (CurrentLog->Length != 0xFF)) {
    CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)( (UINT8*)CurrentLog + CurrentLog->Length);

    if (((UINTN)CurrentLog > (UINTN)(EPrivate->GPNVBase + EPrivate->GPNVLength) ||
        (UINTN)CurrentLog< (UINTN)(EPrivate->GPNVBase)) ||
        (CurrentLog->Length == 0)) {

      return EFI_SUCCESS;
    }
  }

  Status = EfiGetSystemConfigurationTable (&gEfiHobListGuid, (VOID **)&HobList);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (;;) {
//[-start-171212-IB08400542-modify]//
    Buffer = GetNextGuidHob (&gH2OBiosEventLogHobGuid, HobList);
//[-end-171212-IB08400542-modify]//
    if (Buffer == NULL) {
      break;
    }
    HobLog = (BIOS_EVENT_LOG_ORGANIZATION *)((UINT8 *)Buffer + sizeof (EFI_HOB_GUID_TYPE));
    Length = (UINTN)HobLog->Length;
    Status = FlashProgram (
                          (UINT8 *)(UINTN)CurrentLog,
                          Buffer,
                          &Length,
                          (((UINTN)CurrentLog) & ~(0xFFFF))
                          );

    if (EFI_ERROR (Status) ) {
      return Status;
    }
    
    HobList = GET_NEXT_HOB (Buffer);

    CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *) ((UINT8*) CurrentLog + Length);
    if (((UINTN)CurrentLog > (UINTN)(EPrivate->GPNVBase + EPrivate->GPNVLength) ||
        (UINTN)CurrentLog  < (UINTN)(EPrivate->GPNVBase)) ||
        (CurrentLog->Length == 0)) {
      return EFI_SUCCESS;
    }
  }

  return EFI_SUCCESS;
}

/**
 Fixup internal data pointers so that the services can be called in virtual mode.
          
 @param[in] Event     The event registered.
 @param[in] Context   Event context.
  
 @retval EFI Status                  
*/
VOID
EFIAPI
EventLogVirtualAddressChangeEvent (
  IN EFI_EVENT                         Event,
  IN VOID                              *Context
  )
{
//[-start-171212-IB08400542-modify]//
  gBS->UninstallProtocolInterface (
                  mELPrivate->Handle,
                  &gH2OBiosEventLogProtocolGuid,
                  &mELPrivate->EventLogService
                  );
//[-end-171212-IB08400542-modify]//

}

/**
 Initialize Flash device if flash device hasn't been initialized         
 
 @retval EFI Status                  
*/
VOID
EFIAPI
NonVolatileVariableNotifyFunction (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS                           Status;
  LIST_ENTRY                           *Link;
  BIOS_EVENT_LOG_ORGANIZATION          *CurrentLog;
  EFI_EVENT_LOG_DATA                   *EventLog;
  UINTN                                NumBytes;
  
  if (mELPrivate->DevicePtr == NULL) {
    Status = FlashFdDetectDevice (&mELPrivate->EventLogService, NULL);
    ASSERT_EFI_ERROR (Status);

    //
    // Before gEfiNonVolatileVariableProtocolGuid is installed, some drivers may write or clear event log.
    // It should write those event logs into flash device in here.
    // First write event log from Hob list, then write event log from out temp event log list.
    //
    if (mClearBiosStorageKernel) {
      EfiClearEventLog (&mELPrivate->EventLogService);
    } else {
      GetEventLogHob (&mELPrivate->EventLogService);
    }
    mClearBiosStorageKernel = FALSE;

    if (!IsListEmpty (&mBiosStorageKernelList)) {
      Link = mBiosStorageKernelList.ForwardLink;

      CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *) (UINTN) mELPrivate->GPNVBase;
      while ((CurrentLog->Type != 0xFF) && (CurrentLog->Length != 0xFF)) {
        if (((UINTN)CurrentLog + CurrentLog->Length) >= (UINTN)(mELPrivate->GPNVBase + mELPrivate->GPNVLength) ||
            (UINTN)CurrentLog< (UINTN)(mELPrivate->GPNVBase) ||
            (CurrentLog->Length == 0)){
          return;
        }
        CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)( (UINT8*)CurrentLog + CurrentLog->Length);
      }
      
      while (Link != &mBiosStorageKernelList) {
        EventLog = DATA_FROM_EFI_EVENT_LOG_THIS(Link);
        NumBytes = (UINTN) EventLog->Buffer->Length;
        
        Status = FlashProgram (
                              (UINT8 *)(UINTN)CurrentLog,
                              (UINT8 *) (UINTN) EventLog->Buffer,
                              &NumBytes,
                              (((UINTN)CurrentLog) & ~(0xFFFF))
                              );

        CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *) ((UINT8*) CurrentLog + NumBytes);
        if ((((UINTN)CurrentLog + CurrentLog->Length) >= (UINTN)(mELPrivate->GPNVBase + mELPrivate->GPNVLength)) ||
            ((UINTN)CurrentLog < (UINTN)(mELPrivate->GPNVBase)) ||
            (CurrentLog->Length == 0)) {
          return;
        }
        
        Link = Link->ForwardLink;
        gBS->FreePool (EventLog->Buffer);
        gBS->FreePool (EventLog);
      }      

      InitializeListHead (&mBiosStorageKernelList);
    }
  }

  return;
}

/**
 Entry point of this driver. Install Event Log protocol into DXE.

 @param[in] ImageHandle       Image handle of this driver.
 @param[in] SystemTable       Global system service table.          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosStorageKernelEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                          Status;
  EFI_EVENT                           Event;
  VOID                                *Registration;
  VOID                                *ProtocolInterface;
  EFI_EVENT                           BiosProtectReadyToBootEvent;
  VOID                                *VariableWriteEventRegistration; 

  //
  // Make sure the NV space is available for BIOS Storage.
  // 
//[-start-171212-IB08400542-modify]//
  if ((!PcdGet32 (PcdH2OBiosEventStorageBase)) || (!PcdGet32 (PcdH2OBiosEventStorageSize))) {
    return EFI_NOT_FOUND;
  }
//[-end-171212-IB08400542-modify]//
  
  Event = 0;
  //
  // Make sure these global variables are initialized in both DXE and SMM.
  //
  mClearBiosStorageKernel = FALSE;
  InitializeListHead (&mBiosStorageKernelList);

  gRT = gST->RuntimeServices;
  
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (EFI_EVENT_LOG_INSTANCE),
                  (VOID **)&mELPrivate
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mELPrivate->Signature                      = EVENT_LOG_SIGNATURE;
  mELPrivate->DevicePtr                      = NULL;
//[-start-171212-IB08400542-modify]//
  mELPrivate->GPNVBase                       = PcdGet32 (PcdH2OBiosEventStorageBase);
  mELPrivate->GPNVLength                     = PcdGet32 (PcdH2OBiosEventStorageSize);
//[-end-171212-IB08400542-modify]//
  mELPrivate->EventLogService.Write          = EfiWriteEventLog;
  mELPrivate->EventLogService.Clear          = EfiClearEventLog;
  mELPrivate->EventLogService.ReadNext       = EfiReadNextEventLog;
  mELPrivate->EventLogService.DetectDevice   = FlashFdDetectDevice;
  mELPrivate->EventLogService.OverWrite      = EfiOverWriteEventLog;

  mELPrivate->Handle = NULL;
//[-start-171212-IB08400542-modify]//
  Status = gBS->InstallProtocolInterface (
                                      &mELPrivate->Handle,
                                      &gH2OBiosEventLogProtocolGuid,
                                      EFI_NATIVE_INTERFACE,
                                      &(mELPrivate->EventLogService)
                                      );
//[-end-171212-IB08400542-modify]//
  if (!EFI_ERROR (Status)) {
    if (mELPrivate->DevicePtr == NULL) {
      Status = FlashFdDetectDevice (&mELPrivate->EventLogService, NULL);
      ASSERT_EFI_ERROR (Status);
    }
    Status = gBS->LocateProtocol (&gEfiNonVolatileVariableProtocolGuid, NULL, (VOID **)&ProtocolInterface);
    if (EFI_ERROR (Status)) {
      Status = gBS->CreateEvent (
                      EVT_NOTIFY_SIGNAL,
                      TPL_CALLBACK,
                      NonVolatileVariableNotifyFunction,
                      NULL,
                      &Event
                      );
      ASSERT_EFI_ERROR(Status);

      Status = gBS->RegisterProtocolNotify (
                      &gEfiNonVolatileVariableProtocolGuid,
                      Event,
                      &Registration
                      );
      ASSERT_EFI_ERROR(Status);
    } else {
      NonVolatileVariableNotifyFunction (Event, NULL);
    }

    //
    // Use ReadyToBootEvent to make sure BiosProtectEvent()
    // performed whether BIOS protection is enabled.
    //
    Status = EfiCreateEventReadyToBootEx (
                                          TPL_CALLBACK,
                                           BiosProtectEvent,
                                           NULL,
                                           &BiosProtectReadyToBootEvent
                                           );
    ASSERT_EFI_ERROR (Status);

    //
    // Create an evnet - Wait for Variable Write Architecture Protocol
    //
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    InstalledVarWriteNotificationFunction,
                    NULL,
                    &mVarWriteEvent
                    );

    if (!EFI_ERROR (Status)) {
      gBS->RegisterProtocolNotify (
             &gEfiVariableWriteArchProtocolGuid,
             mVarWriteEvent,
             &VariableWriteEventRegistration
             );
    }
  }

  Status = GetEventLogHob (&mELPrivate->EventLogService);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Event Log Get Event Log Hob failed.\n"));
  }

  return EFI_SUCCESS;
}

/**
 Read function of Event log.s 

 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EfiReadNextEventLog (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL         *This,
  IN OUT VOID                             **EventListAddress
  )
{
  EFI_EVENT_LOG_INSTANCE                *EPrivate;
  BIOS_EVENT_LOG_ORGANIZATION           *CurrentLog;


  EPrivate = INSTANCE_FROM_EFI_EVENT_LOG_THIS (This);
  if (EPrivate->DevicePtr == NULL) {
    return EmuReadNextEventLog (This, EventListAddress);
  }

  //
  //Current Log is first log
  //
  CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)(UINTN)EPrivate->GPNVBase;

  //
  //Check EventListAddress.
  //If it is NULL, and the first log exist, return first Log
  //
  if (*EventListAddress == NULL) {
    if ((CurrentLog->Type == 0xFF) && (CurrentLog->Length == 0xFF)) {

      return EFI_NOT_FOUND;
    }
    *EventListAddress = (UINT8 *)(UINTN)EPrivate->GPNVBase;
    return EFI_SUCCESS;
  }

  //
  //Check current log
  //
  CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)*EventListAddress;

  //
  //If the EventListAddress out of range, return error
  //
  if (((UINTN)CurrentLog > (UINTN)(EPrivate->GPNVBase + EPrivate->GPNVLength) ||
      (UINTN)CurrentLog< (UINTN)(EPrivate->GPNVBase)) ||
      (CurrentLog->Length == 0)){

      return EFI_INVALID_PARAMETER;
  }

  //
  //If the EventListAddress point to the NULL, return
  //
  if ((CurrentLog->Type == 0xFF) && (CurrentLog->Length == 0xFF)) {

    return EFI_NOT_FOUND;
  }

  //
  //Find out and point to the next Event Log
  //
  *EventListAddress = (UINT8 *)( (UINT8*)CurrentLog + CurrentLog->Length);

  return EFI_SUCCESS;

}

/**
  Write event to BIOS Storage.

  @param [in] This                 This PPI interface.
  @param [in] BiosStorageType      Event log type.
  @param [in] PostBitmap1          Post bitmap 1 which will be stored in data area of POST error type log.
  @param [in] PostBitmap2          Post bitmap 2 which will be stored in data area of POST error type log.
  @param [in] OptionDataSize       Optional data size.
  @param [in] OptionLogData        Pointer to optional data.
  
  @retval EFI Status            
**/
EFI_STATUS
EFIAPI
EfiWriteEventLog (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This,
  IN  UINT8                             EventLogType,
  IN  UINT32                            PostBitmap1,
  IN  UINT32                            PostBitmap2,
  IN  UINTN                             OptionDataSize,
  IN  UINT8                             *OptionLogData
  )
{
  UINT8                                 *Src;
  UINT8                                 *Dest;
  EFI_STATUS                            Status;
  EFI_EVENT_LOG_INSTANCE                *EPrivate;
  UINTN                                 WriteAddress;
  BIOS_EVENT_LOG_ORGANIZATION           *CurrentLog;
  BIOS_EVENT_LOG_ORGANIZATION           *Buffer;
  UINTN                                 NumBytes;
  UINTN                                 DataFormatTypeLength;
  EFI_EVENT_LOG_BUF                     *EventLogBuf = NULL;
  EFI_SMM_COMMUNICATE_HEADER            *SmmCommBufferHeader;
  EFI_SMM_COMMUNICATION_PROTOCOL        *Communication;
//[-start-180417-IB08400580-modify]//
  UINT8                                 *SmmEventLogBuffer;
//[-end-180417-IB08400580-modify]//
  UINTN                                 CommSize;
  EL_TIME                               ElTime;

  if ((OptionDataSize == 0) || (OptionLogData == NULL)) {
    OptionDataSize = 0;
    OptionLogData = NULL;
  }

  EPrivate = INSTANCE_FROM_EFI_EVENT_LOG_THIS (This);
  if (EPrivate->DevicePtr == NULL) {
    return EmuWriteEventLog (This, EventLogType, PostBitmap1, PostBitmap2, OptionDataSize, OptionLogData);
  }

  if (SwitchToSmm()) {
    Status = gBS->LocateProtocol (&gEfiSmmCommunicationProtocolGuid, NULL, (VOID **)&Communication);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "BiosStorageKernelDxe locate SMM Communication protocol: %r\n", Status));
      return Status;
    }

//[-start-180417-IB08400580-add]//
    Status = gBS->AllocatePool (EfiRuntimeServicesData, SMM_EVENT_LOG_COMMUNICATION_BUFFER_SIZE, &SmmEventLogBuffer);
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
//[-end-180417-IB08400580-add]//
    
    //
    // Use to notify SMM driver that the BIOS Storage Kernel service is not available after exiting boot services.
    //
    SmmCommBufferHeader = (EFI_SMM_COMMUNICATE_HEADER *)SmmEventLogBuffer;
    EventLogBuf = (EFI_EVENT_LOG_BUF *)SmmCommBufferHeader->Data;

    EventLogBuf->ActionType     = EVENT_LOG_WRITE_LOG;
    EventLogBuf->This           = This;
    EventLogBuf->EventLogType   = EventLogType;
    EventLogBuf->PostBitmap1    = PostBitmap1;
    EventLogBuf->PostBitmap2    = PostBitmap2;
    EventLogBuf->OptionDataSize = OptionDataSize;
    EventLogBuf->OptionLogData  = OptionLogData;
    EventLogBuf->ReturnStatus   = EFI_UNSUPPORTED;

    CopyGuid (&SmmCommBufferHeader->HeaderGuid, &gH2OBiosStorageCommunicationGuid);
    SmmCommBufferHeader->MessageLength = sizeof(EFI_EVENT_LOG_BUF);
    CommSize = OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data)  + sizeof(EFI_EVENT_LOG_BUF);
    
    Status = Communication->Communicate (Communication, SmmEventLogBuffer, &CommSize);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "BiosStorageKernelDxe logs event by switching to SMM Handler: %r\n", Status));
      return Status;
    }
    
    Status = EventLogBuf->ReturnStatus;
    DEBUG((EFI_D_INFO, "Switching to SMM Handler return status: %r\n", Status));
//[-start-180419-IB08400580-add]//
    gBS->FreePool (SmmEventLogBuffer);
//[-end-180419-IB08400580-add]//
    return Status;
  }

  CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)(UINTN)EPrivate->GPNVBase;

  while ((CurrentLog->Type != 0xFF) && (CurrentLog->Length != 0xFF)) {
    if (((UINTN)CurrentLog + CurrentLog->Length + BIOS_EVENT_LOG_BASE_LENGTH + OptionDataSize) >= 
         (UINTN)(EPrivate->GPNVBase + EPrivate->GPNVLength)) {
      //
      // Event Full
      //
      return EFI_OUT_OF_RESOURCES;
    }

    if ((UINTN)CurrentLog< (UINTN)(EPrivate->GPNVBase) ||
        (CurrentLog->Length == 0)) {

      return EFI_UNSUPPORTED;
    }
    CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)( (UINT8*)CurrentLog + CurrentLog->Length);
  }

  DataFormatTypeLength = 0;
  switch (EventLogType) {

  case EfiEventLogTypePostError:
    DataFormatTypeLength = BIOS_EVENT_LOG_DATA_FORMAT_POST_RESULT_BITMAP_LENGTH;
    break;

  default:
    DataFormatTypeLength = BIOS_EVENT_LOG_DATA_FORMAT_NONE_LENGTH;
    break;
  }
  NumBytes = BIOS_EVENT_LOG_BASE_LENGTH + DataFormatTypeLength + OptionDataSize;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  NumBytes,
                  (VOID **)&Buffer
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Event Log allocate pool failed: %r\n", Status));
    return Status;
  }

  ZeroMem (Buffer, NumBytes);

  Status = EventLogGetTimeFromCmos (&ElTime);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  Buffer->Year   = ElTime.Year;
  Buffer->Month  = ElTime.Month;
  Buffer->Day    = ElTime.Day;
  Buffer->Hour   = ElTime.Hour;
  Buffer->Minute = ElTime.Minute;
  Buffer->Second = ElTime.Second;

  Buffer->Type   = EventLogType;
  Buffer->Length = (UINT8)NumBytes;

  if (EventLogType == EfiEventLogTypePostError) {
    *(UINT32 *)(Buffer->Data)            = PostBitmap1;
    *(UINT32 *)(UINTN)(&Buffer->Data[4]) = PostBitmap2;
  }

  if (OptionLogData!= NULL) {
    CopyMem ((UINT8*)((UINTN)Buffer+NumBytes-OptionDataSize), OptionLogData, OptionDataSize);
  }

  WriteAddress = (UINTN)CurrentLog;
  Dest = (UINT8 *)(UINTN)WriteAddress;
  Src = (UINT8 *)(UINTN)Buffer;

  Status = FlashProgram (
                        Dest,
                        Src,
                        &NumBytes,
                        (((UINTN)CurrentLog) & ~(0xFFFF))
                        );

  //
  //  !!WORKARUOUD!! - Force Cache update
  //
  *Dest = 0;
  
  gBS->FreePool(Buffer);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Write Event Event failed: %r\n", Status));
  }

  return Status;

}

/**
 Clear function of Event log.           
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EfiClearEventLog (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This
  )
{
  EFI_STATUS                            Status;
  EFI_EVENT_LOG_INSTANCE                *EPrivate;
  UINT8                                 *Buffer;
  UINT32                                i;
  UINTN                                 BlockSize;
  UINTN                                 FdSupportEraseSize;
  UINTN                                 EraseCount;
  UINTN                                 EraseStartAddress;
  EFI_EVENT_LOG_BUF                     *EventLogBuf = NULL;
  EFI_SMM_COMMUNICATE_HEADER            *SmmCommBufferHeader;
  EFI_SMM_COMMUNICATION_PROTOCOL        *Communication;
//[-start-180417-IB08400580-modify]//
  UINT8                                 *SmmEventLogBuffer;
//[-end-180417-IB08400580-modify]//
  UINTN                                 CommSize;

  EPrivate  = INSTANCE_FROM_EFI_EVENT_LOG_THIS (This);
  if (EPrivate->DevicePtr == NULL) {
    return EmuClearEventLog (This);
  }

  if (SwitchToSmm ()) {
    Status = gBS->LocateProtocol (&gEfiSmmCommunicationProtocolGuid, NULL, (VOID **)&Communication);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "BiosStorageKernelDxe locate SMM Communication protocol: %r\n", Status));
      return Status;
    }

//[-start-180417-IB08400580-add]//
    Status = gBS->AllocatePool (EfiRuntimeServicesData, SMM_EVENT_LOG_COMMUNICATION_BUFFER_SIZE, &SmmEventLogBuffer);
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
//[-end-180417-IB08400580-add]//

    //
    // Use to notify SMM driver that the BIOS Storage Kernel service is not available after exiting boot services.
    //
    SmmCommBufferHeader = (EFI_SMM_COMMUNICATE_HEADER *)SmmEventLogBuffer;
    EventLogBuf = (EFI_EVENT_LOG_BUF *)SmmCommBufferHeader->Data;

    EventLogBuf->ActionType = EVENT_LOG_CLEAR_LOG;
    EventLogBuf->This = This;
    EventLogBuf->ReturnStatus = EFI_UNSUPPORTED;

    CopyGuid (&SmmCommBufferHeader->HeaderGuid, &gH2OBiosStorageCommunicationGuid);
    SmmCommBufferHeader->MessageLength = sizeof(EFI_EVENT_LOG_BUF);
    CommSize = OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data)  + sizeof(EFI_EVENT_LOG_BUF);
    
    Status = Communication->Communicate (Communication, SmmEventLogBuffer, &CommSize);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "BiosStorageKernelDxe logs event by switching to SMM Handler: %r\n", Status));
      return Status;
    }
    
    Status = EventLogBuf->ReturnStatus;
    DEBUG((EFI_D_INFO, "Switching to SMM Handler return status: %r\n", Status));

//[-start-180419-IB08400580-add]//
    gBS->FreePool (SmmEventLogBuffer);
//[-end-180419-IB08400580-add]//
    return Status;
  }
  
  FdSupportEraseSize = GetFlashBlockSize();  
  EraseCount = GET_ERASE_SECTOR_NUM ( 
                                 EPrivate->GPNVBase,
                                 EPrivate->GPNVLength,
                                 EPrivate->GPNVBase & ~(FdSupportEraseSize - 1),
                                 FdSupportEraseSize
                                 );
  EraseStartAddress = EPrivate->GPNVBase & ~(FdSupportEraseSize - 1);
  
  BlockSize = FdSupportEraseSize * EraseCount;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  BlockSize,
                  (VOID **)&Buffer
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Event Log allocate pool failed: %r\n", Status));
    return Status;
  }

  //
  //Copy whole block data to buffer
  //
  CopyMem(
    Buffer,
    (VOID *)(UINTN)(EPrivate->GPNVBase & (~(FdSupportEraseSize - 1))),
    BlockSize
    );

  //
  //Copy modified GPNV data to buffer
  //
  for (i=0;i<EPrivate->GPNVLength;i++) {
    *(UINT8 *)((Buffer + (EPrivate->GPNVBase) - (EPrivate->GPNVBase & ((UINT32)(~0)) & (~(FdSupportEraseSize - 1))))+i) = 0xFF;
  }

  //
  //Flash GPNV
  //
//[-start-161109-IB04560703-modify]//
  for (i = 0; i < EraseCount; i++) {
    
    Status = FlashErase (
                         (EPrivate->GPNVBase & ((UINT32)(~0)) & (~(FdSupportEraseSize - 1))) + FdSupportEraseSize * i,
                         FdSupportEraseSize
                         );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Event Log FlashErase() failed: %r\n", Status));
      return Status;
    }
  }

  Status = FlashProgram (
                        (UINT8 *)(UINTN)(EPrivate->GPNVBase & ((~(FdSupportEraseSize - 1)))),
                        Buffer,
                        &BlockSize,
                        ((EPrivate->GPNVBase & (UINT32)(~0)) & ~(FdSupportEraseSize - 1))
                        );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Event Log FlashProgram() failed: %r\n", Status));
    return Status;
  }
//[-end-161109-IB04560703-modify]//
  //
  //  !!WORKARUOUD!! - Force Cache update
  //
  AsmWbinvd ();
  gBS->FreePool(Buffer);

  if (!mClearBiosStorageKernel) {
    Status = EPrivate->EventLogService.Write(
                                         &EPrivate->EventLogService,
                                         EfiEventLogTypeLogAreaResetCleared,
                                         0,
                                         0,
                                         0,
                                         NULL
                                         );
  }
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Event Log write data failed: %r\n", Status));
  }
  
  return Status;
}

/**
  Write the BIOS Event Log with overwrite method.

  @param [in] This                 This PPI interface.
  @param [in] InputBuffer          The event log data to logging..
  
  @retval EFI Status            
**/
EFI_STATUS
EFIAPI
EfiOverWriteEventLog (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This,
  IN  BIOS_EVENT_LOG_ORGANIZATION       *InputBuffer
  )
{
  UINT8                                 *Src;
  UINT8                                 *Dest;
  EFI_STATUS                            Status;
  EFI_EVENT_LOG_INSTANCE                *EPrivate;
  UINTN                                 WriteAddress;
  BIOS_EVENT_LOG_ORGANIZATION           *CurrentLog;
  BIOS_EVENT_LOG_ORGANIZATION           *Buffer;
  UINTN                                 NumBytes;
  UINTN                                 BaseLenght;
  EFI_SMM_COMMUNICATE_HEADER            *SmmCommBufferHeader;
  EFI_SMM_COMMUNICATION_PROTOCOL        *Communication;
//[-start-180417-IB08400580-modify]//
  UINT8                                 *SmmEventLogBuffer;
//[-end-180417-IB08400580-modify]//
  UINTN                                 CommSize;  
  EFI_EVENT_LOG_BUF                     *EventLogBuf = NULL;

  if ((This == NULL) || (InputBuffer == NULL)) {
    return EFI_UNSUPPORTED;
  }
  if (SwitchToSmm()) {
    Status = gBS->LocateProtocol (&gEfiSmmCommunicationProtocolGuid, NULL, (VOID **)&Communication);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "BiosStorageKernelDxe locate SMM Communication protocol: %r\n", Status));
      return Status;
    }

//[-start-180417-IB08400580-add]//
    Status = gBS->AllocatePool (EfiRuntimeServicesData, SMM_EVENT_LOG_COMMUNICATION_BUFFER_SIZE, &SmmEventLogBuffer);
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
//[-end-180417-IB08400580-add]//

    //
    // Use to notify SMM driver that the BIOS Storage Kernel service is not available after exiting boot services.
    //
    SmmCommBufferHeader = (EFI_SMM_COMMUNICATE_HEADER *)SmmEventLogBuffer;
    EventLogBuf = (EFI_EVENT_LOG_BUF *)SmmCommBufferHeader->Data;

    EventLogBuf->ActionType     = EVENT_LOG_OVERWRITE_LOG;
    EventLogBuf->This           = This;
    EventLogBuf->RawData        = (UINT8 *)InputBuffer;
    EventLogBuf->ReturnStatus   = EFI_UNSUPPORTED;

    CopyGuid (&SmmCommBufferHeader->HeaderGuid, &gH2OBiosStorageCommunicationGuid);
    SmmCommBufferHeader->MessageLength = sizeof(EFI_EVENT_LOG_BUF);
    CommSize = OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data)  + sizeof(EFI_EVENT_LOG_BUF);
    
    Status = Communication->Communicate (Communication, SmmEventLogBuffer, &CommSize);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "BiosStorageKernelDxe Overwrite: logs event by switching to SMM Handler: %r\n", Status));
      return Status;
    }
    
    Status = EventLogBuf->ReturnStatus;
//[-start-180419-IB08400580-add]//
    gBS->FreePool (SmmEventLogBuffer);
//[-end-180419-IB08400580-add]//    

    return Status;
  }

  EPrivate = INSTANCE_FROM_EFI_EVENT_LOG_THIS (This);

  CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)(UINTN)EPrivate->GPNVBase;

  while ((CurrentLog->Type != 0xFF) && (CurrentLog->Length != 0xFF)) {
    if (((UINTN)CurrentLog + CurrentLog->Length + InputBuffer->Length) >= 
        (UINTN)(EPrivate->GPNVBase + EPrivate->GPNVLength)) {
      //
      // Event Full
      //
      return EFI_OUT_OF_RESOURCES;
    }

    if ((UINTN)CurrentLog< (UINTN)(EPrivate->GPNVBase) ||
        (CurrentLog->Length == 0)) {

      return EFI_UNSUPPORTED;
    }
    CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)( (UINT8*)CurrentLog + CurrentLog->Length);
  }

  NumBytes = InputBuffer->Length;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  NumBytes,
                  (VOID **)&Buffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (Buffer, NumBytes);

  Buffer->Year   = InputBuffer->Year;
  Buffer->Month  = InputBuffer->Month;
  Buffer->Day    = InputBuffer->Day;
  Buffer->Hour   = InputBuffer->Hour;
  Buffer->Minute = InputBuffer->Minute;
  Buffer->Second = InputBuffer->Second;

  Buffer->Type   = InputBuffer->Type;;
  Buffer->Length = InputBuffer->Length;;

  BaseLenght = BIOS_EVENT_LOG_BASE_LENGTH;
  
  if (InputBuffer->Length > BIOS_EVENT_LOG_BASE_LENGTH) {
    CopyMem ((UINT8*)((UINTN)Buffer+NumBytes-(InputBuffer->Length - BaseLenght)), 
                &InputBuffer->Data[0], 
                InputBuffer->Length - BaseLenght);
  }

  WriteAddress = (UINTN)CurrentLog;
  Dest = (UINT8 *)(UINTN)WriteAddress;
  Src = (UINT8 *)(UINTN)Buffer;

  Status = FlashProgram (
                      Dest,
                      Src,
                      &NumBytes,
                      (WriteAddress & ~(0xFFFF))
                      );
  //
  //  !!WORKARUOUD!! - Force Cache update
  //
  *Dest = 0;
  
  gBS->FreePool(Buffer);

  return Status;

}

/**
  Check if BIOS protection is enabled. 
         
 @param[in]  Event             A pointer to the Event that triggered the callback.
 @param[in]  Context           A pointer to private data registered with the callback function.
  
 @retval EFI Status                  
*/
VOID
EFIAPI
BiosProtectEvent (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  mSwitchToSmm = TRUE;
}

/**
  Check if accessing BIOS GPNV region need to switch to SMM driver. 
           
  @retval TRUE  Switch the access BIOS GPNV region by SMM driver.
          FALSE Don't switch.
*/
BOOLEAN
EFIAPI
SwitchToSmm (
  VOID
  )
{  
  if (mSwitchToSmm == TRUE) {
    return TRUE;
  }

  if (PcdGetBool(PcdH2OSmmBiosStorageSupportReady)) {
    mSwitchToSmm = TRUE;
  } else {
    mSwitchToSmm = FALSE;
  }

  return mSwitchToSmm;
  
}

/**
 
 Notification function when Variable Architecture Protocol Installed
            
 @param[in] Event   - The Event that is being processed
 @param[in] Context - Event Context
  
 @retval EFI Status                  
*/
VOID
EFIAPI
InstalledVarWriteNotificationFunction (
  IN  EFI_EVENT       Event,
  IN  VOID            *Context
  )
{
  EFI_STATUS                              Status;
  H2O_EVENT_STORAGE_VARIABLE              H2OEventStorageVar;  
  UINTN                                   VarSize;
  CHAR16                                  *VarName = H2O_EVENT_STORAGE_VARIABLE_NAME;

  //
  // Set EventStorage variable to store the BIOS Storege address.
  //
  VarSize = sizeof(H2O_EVENT_STORAGE_VARIABLE);
  gRT->GetVariable (
                H2O_EVENT_STORAGE_VARIABLE_NAME,
                &gH2OEventStorageVariableGuid,
                NULL,
                &VarSize,
                &H2OEventStorageVar
                );

  H2OEventStorageVar.BiosEventStorageExist = TRUE;
  H2OEventStorageVar.BiosEventStorageBaseAddress = (UINT32) (UINTN)(mELPrivate->GPNVBase);
  H2OEventStorageVar.BiosEventStorageLength = (UINT32) (UINTN)(mELPrivate->GPNVLength);

  VarSize = sizeof(H2O_EVENT_STORAGE_VARIABLE);
  Status = gRT->SetVariable (
                         VarName,
                         &gH2OEventStorageVariableGuid,
                         EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                         VarSize,
                         &H2OEventStorageVar
                         );   
  if (EFI_ERROR (Status)) {
    return;
  }
  
  gBS->CloseEvent (mVarWriteEvent);

  return;

}



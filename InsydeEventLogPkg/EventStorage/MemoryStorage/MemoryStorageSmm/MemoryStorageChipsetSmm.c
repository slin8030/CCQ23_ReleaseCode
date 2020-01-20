/** @file

  Memory Event Storage Chipset implementation.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/ 


#include "MemoryStorageChipsetSmm.h"

STORAGE_EVENT_LOG_INFO                *mMemoryStorageLog;
UINTN                                 mMemoryStorageLogNum = 0;
CHAR16                                *mMemoryStorageNameString = L"MEMORY";
EVENT_LOG_FULL_OPTION                 mEventLogFullOp = EVENT_LOG_FULL_OVERWRITE;                
EVENT_STORAGE_PROTOCOL                gEventStorageProtocol = {
                                                         MemoryWriteEvent,
                                                         MemoryReadEvent,
                                                         MemoryGetStorageNameString,
                                                         MemoryGetEventCount,
                                                         MemoryClearEvent,
                                                         MemoryRefreshDatabase,
                                                         &mEventLogFullOp
                                                         };

/**
 Entry point of this driver. Install Memory Event Storage protocol into DXE.

 @param[in] ImageHandle       Image handle of this driver.
 @param[in] SystemTable       Global system service table.          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
MemoryStorageChipsetEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            SmmHandle;

  SmmHandle = NULL;
  
  //
  // First time entry. Initial driver
  //
  Status = gSmst->SmmInstallProtocolInterface (
                                           &SmmHandle,
                                           &gH2OSmmEventStorageProtocolGuid,
                                           EFI_NATIVE_INTERFACE,
                                           &gEventStorageProtocol
                                           );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;

}

/**
 Locate event log protocol to log data.           

 @param[in] EventID  ID for specific event.
 @param[in] Data     Data that will log into MEMORY storage.
 @param[in] DataSize Size of Data.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
MemoryWriteEvent (
  IN  EVENT_TYPE_ID                    *EventID,
  IN  UINT8                            *Data,
  IN  UINTN                            DataSize
  )
{
  EFI_STATUS                         Status;
  EFI_MEMORY_STORAGE_PROTOCOL        *MemEventLog;  
  UINT8                              *Buffer=NULL;

  MemEventLog = NULL;  

  Status = gSmst->SmmLocateProtocol (&gH2OSmmMemoryStorageProtocolGuid, NULL, (VOID **)&MemEventLog);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  Status = gSmst->SmmAllocatePool (
                             EfiRuntimeServicesData, 
                             DataSize, 
                             (VOID **)&Buffer
                             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  ZeroMem (Buffer, DataSize);

  //
  // Loop for even fulled state
  //
  do {
    CopyMem (Buffer, Data, DataSize);
    
    Status = MemEventLog->Write (
                             EventID->SensorType,
                             EventID->SensorNum,
                             EventID->EventType,
                             DataSize,
                             Buffer
                             );    
    if (EFI_ERROR (Status) && (Status == EFI_OUT_OF_RESOURCES)) {
      //
      // It means the storage is full of data, 
      // we need to adjust the storage base on the policy of 
      // "Event Log Full option" setting in SCU.
      //

      if (mEventLogFullOp == EVENT_LOG_FULL_STOP_LOGGING) {
        //
        // Not log anymore.
        //
        return EFI_OUT_OF_RESOURCES;
      } else {
        //
        // Storage is full, and we still want to log event into it.
        // Need to arrange the data in the storage.
        //
        Status = ArrangeEventStorage ();
        if (EFI_ERROR (Status)) {
          return Status;
        } 
        
      }
    } else {
      //
      // After MemEventLog->Write(),
      // return if Status == EFI_SUCCESS or OTHER ERROR status beside EFI_OUT_OF_RESOURCE.
      //
      return Status;
    } 
    
  } while (TRUE);

}

/**
 Locate Event log protocol to get logged data in MEMORY.          

 @param[in]  Data        Logged data of MEMORY.
 @param[out] DataSize    Size of Logged data.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
MemoryReadEvent (
  IN      UINTN                            Index,
  IN OUT  UINT8                            **Data
  )
{

  if (mMemoryStorageLogNum == 0) {
    return EFI_NOT_FOUND;
  }
  
  if (Index >= mMemoryStorageLogNum) {
    return EFI_OUT_OF_RESOURCES;
  }

  *Data = (UINT8 *)&mMemoryStorageLog[Index];
  return EFI_SUCCESS;
}

/**
 Return storage name string.

 @param[in]         String   name string of the MEMORY storage.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
MemoryGetStorageNameString (
  CHAR16                                  **String
)
{

  *String = mMemoryStorageNameString;

  return EFI_SUCCESS;
}

/**
 Return the count of event count.

 @param[in]         Count     Event count of MEMORY storage.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
MemoryGetEventCount (
  IN  OUT UINTN                           *Count
  )
{
  *Count = mMemoryStorageLogNum;
  
  return EFI_SUCCESS;
}

/**
 Locate Event log protocol to get logged data in MEMORY.

 @param[in]          DataCount   Number of data count in MEMORY storage.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
MemoryRefreshDatabase (
  IN OUT  UINTN                   *DataCount
  )
{
  EFI_STATUS                         Status;
  EFI_MEMORY_STORAGE_PROTOCOL        *MemEventLog;  
  MEMORY_STORAGE_ORGANIZATION        *EventList = NULL;
  VE_MEMORY_EVENT_LOG_STRUCTURE      MemoryEvent;
  UINTN                              EventCount;
  UINTN                              Index;

  FreeMemoryEventDatabase ();
  
  Status = gSmst->SmmLocateProtocol (&gH2OSmmMemoryStorageProtocolGuid, NULL, (VOID **)&MemEventLog);
  if (EFI_ERROR (Status)) {
    return Status;
  } 

  //
  // Prepare Event Log Data From Memory.
  //
  EventCount = 0;
  while (TRUE) {
    Status = MemEventLog->ReadNext ((VOID **)&EventList);
    if (EFI_ERROR (Status)) {
      break; 
    }

    if ((EventList->SensorType == 0x00) &&
        (EventList->SensorNum  == 0x00) &&
        (EventList->EventType  == 0x00)) {
      break;
    }
    EventCount++;
  }

  mMemoryStorageLogNum = EventCount;
  //
  // If there are no logs exist, return to main menu
  //
  if (mMemoryStorageLogNum == 0) {
    *DataCount = 0;
    return EFI_SUCCESS;
  }
  
  //
  // Fill in the MemoryEventLog with Memory Evet
  //  
  Status = gSmst->SmmAllocatePool (
                               EfiRuntimeServicesData, 
                               sizeof(STORAGE_EVENT_LOG_INFO)*mMemoryStorageLogNum, 
                               (VOID **)&mMemoryStorageLog
                               );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ZeroMem (mMemoryStorageLog, sizeof(STORAGE_EVENT_LOG_INFO)*mMemoryStorageLogNum);
  
  EventList = NULL;
  Index = 0;
  while (TRUE) {
    Status = MemEventLog->ReadNext ((VOID **)&EventList);
    if (EFI_ERROR (Status)) {
      break;
    }
    if ((EventList->SensorType == 0x00) &&
        (EventList->SensorNum  == 0x00) &&
        (EventList->EventType  == 0x00)) {
      break;
    }
    MemoryEvent.EventList = EventList;

    Status = gSmst->SmmAllocatePool ( 
                                 EfiRuntimeServicesData, 
                                 MemoryEvent.EventList->Length, 
                                 (VOID **)&(mMemoryStorageLog[Index].EventRawData)
                                 );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    ZeroMem (mMemoryStorageLog[Index].EventRawData, MemoryEvent.EventList->Length);
    
    mMemoryStorageLog[Index].NextRecordId = 0;
    mMemoryStorageLog[Index].RecordId     = 0;
    mMemoryStorageLog[Index].RecordType   = 0;

    
    mMemoryStorageLog[Index].Year    = 2000 + TimeTranslator (MemoryEvent.EventList->Year);
    mMemoryStorageLog[Index].Month   = TimeTranslator (MemoryEvent.EventList->Month);
    mMemoryStorageLog[Index].Date    = TimeTranslator (MemoryEvent.EventList->Day);
    mMemoryStorageLog[Index].Hour    = TimeTranslator (MemoryEvent.EventList->Hour);
    mMemoryStorageLog[Index].Minute  = TimeTranslator (MemoryEvent.EventList->Minute);
    mMemoryStorageLog[Index].Second  = TimeTranslator (MemoryEvent.EventList->Second);

    mMemoryStorageLog[Index].GeneratorId       = 0;
    mMemoryStorageLog[Index].GeneratorIdEnable = FALSE;
    mMemoryStorageLog[Index].EvMRev            = 0;
    
    mMemoryStorageLog[Index].EventID.SensorType = MemoryEvent.EventList->SensorType;
    mMemoryStorageLog[Index].EventID.SensorNum  = MemoryEvent.EventList->SensorNum;
    mMemoryStorageLog[Index].EventID.EventType  = MemoryEvent.EventList->EventType;
    mMemoryStorageLog[Index].EventIdEnable      = TRUE;
    
    mMemoryStorageLog[Index].Data               = &(mMemoryStorageLog[Index].EventRawData[MEMORY_STORAGE_BASE_LENGTH]);

    mMemoryStorageLog[Index].EventTypeID       = 0;
    mMemoryStorageLog[Index].EventTypeIdEnable = FALSE;
    
    mMemoryStorageLog[Index].EventRawDataSize  = MemoryEvent.EventList->Length;
    CopyMem (
         (UINT8 *)mMemoryStorageLog[Index].EventRawData, 
         (UINT8 *)(MemoryEvent.EventList), 
         mMemoryStorageLog[Index].EventRawDataSize
         );
    
    Index++;
  }

  *DataCount = mMemoryStorageLogNum;
  return EFI_SUCCESS;
}

/**
 Clear log data.
          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
MemoryClearEvent (
  VOID
)
{
  EFI_STATUS                         Status;
  EFI_MEMORY_STORAGE_PROTOCOL        *MemEventLog;  

  Status = gSmst->SmmLocateProtocol (
                                 &gH2OSmmMemoryStorageProtocolGuid, 
                                 NULL,
                                 (VOID **)&MemEventLog
                                 );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Clear MEMORY logged data.
  //
  Status = MemEventLog->Clear ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (mMemoryStorageLogNum == 0) {
    return EFI_SUCCESS;
  }
  //
  // Clear MEMORY event log buffer.
  //
  FreeMemoryEventDatabase ();

  return EFI_SUCCESS;
}

/**
 Translate time format to numerical value.

 @param[in]         OrgTime   Original time format.
             
 
 @retval Value of time.                 
*/
UINT8
EFIAPI
TimeTranslator (
  IN  UINT8             OrgTime
)
{
  UINT8       Time;

  Time = ((OrgTime >> 4) * 10) + (OrgTime & 0x0f);

  return Time;

}

/**
 Free Memory event data buffer.        
 
 @retval EFI Status                  
*/
VOID
EFIAPI
FreeMemoryEventDatabase (
  VOID
  )
{
  UINTN                     Index;
  
  if (mMemoryStorageLogNum != 0) { 
    //
    // Clear data of event data buffer.
    //
    for (Index = 0; Index < mMemoryStorageLogNum; Index++) {
      if (mMemoryStorageLog[Index].EventRawData != NULL) {
        gSmst->SmmFreePool (mMemoryStorageLog[Index].EventRawData);
        mMemoryStorageLog[Index].EventRawData = NULL;
      }
    }

    if (mMemoryStorageLog != NULL) {
      gSmst->SmmFreePool (mMemoryStorageLog);
      mMemoryStorageLog = NULL;
    }
    
    mMemoryStorageLogNum = 0;
  }
}

/**
 
 Adding an event data after arranging the storage.

 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
AddEventAfterArrangeEventStorage (
  VOID
  )
{
  EFI_STATUS                         Status;
  EFI_MEMORY_STORAGE_PROTOCOL        *MemEventLog;  
  UINT8                              LogData[EVENT_LOG_DATA_SIZE];

  Status = gSmst->SmmLocateProtocol (&gH2OSmmMemoryStorageProtocolGuid, NULL, (VOID **)&MemEventLog);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  LogData[0] = OEM_SENSOR_OVERWRITE;
  LogData[1] = 0xFF;
  LogData[2] = CHANGE_MEMORY_STORAGE_DATA3;

  
  Status = MemEventLog->Write (
                           OEM_SENSOR_TYPE_EVENT_STORAGE,
                           BIOS_DEFINED_SENSOR_NUM,
                           EVENT_TYPE_OEM,
                           0x3,
                           LogData
                           );  
    
  return Status;
}

/**
 Arrange the data in the storage.         
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
ArrangeEventStorage (
  VOID
  )
{
  EFI_STATUS               Status;

  switch (mEventLogFullOp) {
  case EVENT_LOG_FULL_OVERWRITE:
    Status = ShiftDataOfStorage();
    break;

  case EVENT_LOG_FULL_CLEAR_ALL:
    Status = MemoryClearEvent();
    break;

  default:
    Status = EFI_UNSUPPORTED;
    break;
  }

  if (!EFI_ERROR (Status)) {
    //
    // Logging an event after arranging the storage.
    //
    Status = AddEventAfterArrangeEventStorage ();
  }
  
  return Status;
}

/**
 Shift event data of the storage.           
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
ShiftDataOfStorage (
  VOID
  )
{
  EFI_STATUS                      Status;
  UINTN                           MemoryEventLogCount;
  UINTN                           Index;
  EFI_MEMORY_STORAGE_PROTOCOL     *MemEventLog;  

  Status = gSmst->SmmLocateProtocol (&gH2OSmmMemoryStorageProtocolGuid, NULL, (VOID **)&MemEventLog);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  Status = MemoryRefreshDatabase (&MemoryEventLogCount);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  if (MemoryEventLogCount <= EVENT_LOG_FULL_ADJUST_EVENT_NUM) {
    //
    // If Current count of logged event is equal or less than the adjusting count,
    // Clear all event data immediately.
    //
    Status= MemoryClearEvent();
    return Status;
  }
  
  //
  // clear original event data of the storage.
  //
  Status = MemEventLog->Clear ();
  if (EFI_ERROR (Status)) {
    // 
    // Can not do overwrite action.
    //
  } else {
    for (Index = EVENT_LOG_FULL_ADJUST_EVENT_NUM; Index < MemoryEventLogCount; Index++) {   
      //
      // Restore the adjusted event data into the storage.
      //
      Status = MemEventLog->OverWrite ((MEMORY_STORAGE_ORGANIZATION *)(mMemoryStorageLog[Index].EventRawData));
      if (EFI_ERROR (Status)) { 
        break;
      }
    }
  }

  FreeMemoryEventDatabase ();
  AsmWbinvd ();
  return Status;
}


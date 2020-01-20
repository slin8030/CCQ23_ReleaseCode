/** @file

  BIOS Storage Chipset Dxe implementation.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/ 

#include <BiosStorageChipsetDxe.h>

STORAGE_EVENT_LOG_INFO                *mBiosStorage;
UINTN                                 mBiosStorageNum = 0;
CHAR16                                *mBiosStorageNameString = L"BIOS";
EVENT_LOG_FULL_OPTION                 mEventLogFullOp = EVENT_LOG_FULL_OVERWRITE;
BOOLEAN                               mClearActionWithOverwrite = FALSE;
EVENT_STORAGE_PROTOCOL                gEventStorageProtocol = {
                                                         BiosWriteEvent,
                                                         BiosReadEvent,
                                                         BiosGetStorageNameString,
                                                         BiosGetEventCount,
                                                         BiosClearEvent,
                                                         BiosRefreshDatabase,
                                                         &mEventLogFullOp
                                                         };

H2O_EVENT_LOG_STRING_PROTOCOL         gEventLogString = {
  EventLogStringHandler
  };

BIOS_EVENT_LOG_STRING                 gBiosStorageChipsetString[] = {
                                        {0x80, 0x8, {0x0C, 0x80,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0}, L"Total memory size has been changed."},
                                        {0x80, 0x6, {OEM_SENSOR_TYPE_EVENT_STORAGE, BIOS_DEFINED_SENSOR_NUM, EVENT_TYPE_OEM, OEM_SENSOR_OVERWRITE, 0xFF, CHANGE_BIOS_STORAGE_DATA3}, L"BIOS Event Storage has been changed."}
                                        };
/**
 Sort the current BIOS Storage by time.          

 @param[in] Entry1      Entry of BIOS Storage.
 @param[in] Entry2      Entry of BIOS Storage.
 
 @retval    TRUE        Entry1 is Older than Entry2.
            FALSE       Entry1 is NOT Older than Entry2.
*/
EFI_STATUS
EFIAPI
IsOlderEvent (
  IN  STORAGE_EVENT_LOG_INFO       *Entry1,
  IN  STORAGE_EVENT_LOG_INFO       *Entry2
  )
{
  if (Entry1->Year < Entry2->Year) {
    DEBUG ((EFI_D_INFO, "Year: %d < %d\n", Entry1->Year,Entry2->Year));
    return TRUE;
  } else if (Entry1->Year > Entry2->Year) {
    DEBUG ((EFI_D_INFO, "Year: %d > %d\n", Entry1->Year,Entry2->Year));
    return FALSE;
  }

  if (Entry1->Month < Entry2->Month) {
    DEBUG ((EFI_D_INFO, "Month: %d < %d\n", Entry1->Month,Entry2->Month));
    return TRUE;
  } else if (Entry1->Month > Entry2->Month) {
    DEBUG ((EFI_D_INFO, "Month: %d > %d\n", Entry1->Month,Entry2->Month));
    return FALSE;
  } 

  if (Entry1->Date < Entry2->Date) {
    DEBUG ((EFI_D_INFO, "Date: %d < %d\n", Entry1->Date,Entry2->Date));
    return TRUE;
  } else if (Entry1->Date > Entry2->Date) {
    DEBUG ((EFI_D_INFO, "Date: %d > %d\n", Entry1->Date,Entry2->Date));
    return FALSE;
  }

  if (Entry1->Hour < Entry2->Hour) {
    DEBUG ((EFI_D_INFO, "Hour: %d < %d\n", Entry1->Hour,Entry2->Hour));
    return TRUE;
  } else if (Entry1->Hour > Entry2->Hour) {
    DEBUG ((EFI_D_INFO, "Hour: %d > %d\n", Entry1->Hour,Entry2->Hour));
    return FALSE;
  }

  if (Entry1->Minute < Entry2->Minute) {
    DEBUG ((EFI_D_INFO, "Minute: %d < %d\n", Entry1->Minute,Entry2->Minute));
    return TRUE;
  } else if (Entry1->Minute > Entry2->Minute) {
    DEBUG ((EFI_D_INFO, "Minute: %d > %d\n", Entry1->Minute,Entry2->Minute));
    return FALSE;
  }

  if (Entry1->Second < Entry2->Second) {
    DEBUG ((EFI_D_INFO, "Second: %d < %d\n", Entry1->Second,Entry2->Second));
    return TRUE;
  } else if (Entry1->Second > Entry2->Second) {
    DEBUG ((EFI_D_INFO, "Second: %d > %d\n", Entry1->Second,Entry2->Second));
    return FALSE;
  }

  return FALSE;

}

/**
 Sort the current BIOS Storage by time.          

 VOID
 
 @retval VOID                 
*/
VOID
SortBiosStorage (
  VOID
  )
{
  UINTN                                  Index1;
  UINTN                                  Index2;
  UINTN                                  Temp;
  STORAGE_EVENT_LOG_INFO                 TempEntry;
  
  for (Index1 = 0; Index1 < (mBiosStorageNum - 1); Index1++) {
    Temp = Index1;

    for (Index2 = (Index1 + 1); Index2 < mBiosStorageNum; Index2++) {
      if (IsOlderEvent (&mBiosStorage[Index2], &mBiosStorage[Temp]) == TRUE) {
        Temp = Index2;
      }
    }

    if (Index1 != Temp) {
      //
      // Swap the entries.
      //
      CopyMem (&TempEntry, &mBiosStorage[Index1], sizeof(STORAGE_EVENT_LOG_INFO));
      CopyMem (&mBiosStorage[Index1], &mBiosStorage[Temp], sizeof(STORAGE_EVENT_LOG_INFO));
      CopyMem (&mBiosStorage[Temp], &TempEntry, sizeof(STORAGE_EVENT_LOG_INFO));
    }
  }

}

/**
 Locate event log protocol to log data.

 @param[in]         EventID       ID for specific event.         
 @param[in]         Data          Data that will log into BIOS storage.     
 @param[in]         DataSize      Size of Data.       
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosWriteEvent (
  IN  EVENT_TYPE_ID                    *EventID,
  IN  UINT8                            *Data,
  IN  UINTN                            DataSize
  )
{
  EFI_STATUS                         Status = EFI_UNSUPPORTED;
  EFI_BIOS_EVENT_LOG_PROTOCOL        *EventLog;  
  UINTN                              LocalDataSize=0;
  UINT8                              *Buffer=NULL;
  UINTN                              EventIdNum;
  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL  *EventLogUseVariable;  

  if (FeaturePcdGet(PcdH2OBiosEventStorageUseVariableStorageSupported)) {
    //
    // BIOS Event Log Use Variable protocol: use EFI Variable.
    //
    Status = gBS->LocateProtocol (&gH2OBiosEventLogUseVariableProtocolGuid, NULL, (VOID **)&EventLogUseVariable);
    if (EFI_ERROR (Status)) {
      return Status;
    } 
    
    EventIdNum = sizeof(EVENT_TYPE_ID);
    
    //
    // "3" bytes are for SensorType, SensorNum and EventType information.
    //
    LocalDataSize = DataSize + EventIdNum;
    Buffer = NULL;
    Buffer = AllocateZeroPool (LocalDataSize);
    if (Buffer == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      
      //
      // Loop for even fulled state
      //
      do {
        Buffer[0] = EventID->SensorType;
        Buffer[1] = EventID->SensorNum;
        Buffer[2] = EventID->EventType;
        
        CopyMem (&Buffer[3], Data, DataSize);
        Status = EventLogUseVariable->Write (
                                        EventLogUseVariable,
                                        0x80,
                                        0,
                                        0,
                                        LocalDataSize,
                                        Buffer
                                        );    
        if (Status == EFI_OUT_OF_RESOURCES) {
          //
          // It means the storage is full of data, 
          // we need to adjust the storage base on the policy of 
          // "Event Log Full option" setting in PCD.
          //
        
          if (mEventLogFullOp == EVENT_LOG_FULL_STOP_LOGGING) {
            //
            // Not log anymore.
            //
            Status = EFI_OUT_OF_RESOURCES;
            break;
          } else {
            //
            // Storage is full, and we still want to log event into it.
            // Need to arrange the data in the storage.
            //
            Status = ArrangeEventStorage ();
            if (EFI_ERROR (Status)) {
              break;
            } 
          }
        } else {
          //
          // After EventLogUseVariable->Write(),
          // return if Status == EFI_SUCCESS or OTHER ERROR status beside EFI_OUT_OF_RESOURCE.
          //
          break;
        }
      } while (TRUE);
    }

    //
    // Delete the log number of variable.
    // 
    CommonSetVariable (
      BIOS_STORAGE_LOG_NUMBER_VAR_NAME,
      &gH2OBiosStorageVariableGuid,
      0,
      0,
      NULL
      );

  } else {
    //
    // Use Bios Event Log protocol.
    //
    EventLog = NULL;  
    EventIdNum = sizeof(EVENT_TYPE_ID);
    
//[-start-171212-IB08400542-modify]//
    Status = gBS->LocateProtocol (&gH2OBiosEventLogProtocolGuid, NULL, (VOID **)&EventLog);
//[-end-171212-IB08400542-modify]//
    if (EFI_ERROR (Status)) {
      return Status;
    }
    
    //
    // "3" bytes are for SensorType, SensorNum and EventType information.
    //
    LocalDataSize = DataSize + EventIdNum;
    
    Buffer = NULL;
    Buffer = AllocateZeroPool (LocalDataSize);
    if (Buffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    
    //
    // Loop for even fulled state
    //
    do {
      Buffer[0] = EventID->SensorType;
      Buffer[1] = EventID->SensorNum;
      Buffer[2] = EventID->EventType;
      
      CopyMem (&Buffer[3], Data, DataSize);
      
      Status = EventLog->Write (
                            EventLog,
                            0x80,
                            0,
                            0,
                            LocalDataSize,
                            Buffer
                            );    
      if (Status == EFI_OUT_OF_RESOURCES) {
        //
        // It means the storage is full of data, 
        // we need to adjust the storage base on the policy of 
        // "Event Log Full option" setting in PCD.
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
        // After EventLog->Write(),
        // return if Status == EFI_SUCCESS or OTHER ERROR status beside EFI_OUT_OF_RESOURCE.
        //
        return Status;
      } 
    } while (TRUE);
  }
    
  return Status;
}

/**
 Locate Event log protocol to get logged data in BIOS.

 @param[in]         Index       Logged data of BIOS.                      
 @param[out]        Data        Size of Logged data.     
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosReadEvent (
  IN      UINTN                            Index,
  IN OUT  UINT8                            **Data
  )
{

  if (mBiosStorageNum == 0) {
    return EFI_NOT_FOUND;
  }
  
  if (Index >= mBiosStorageNum) {
    return EFI_OUT_OF_RESOURCES;
  }

  *Data = (UINT8 *)&mBiosStorage[Index];
  return EFI_SUCCESS;
}


/**
 Return storage name string.
             
 @param[out]        String     Name string of the BIOS storage.        
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosGetStorageNameString (
  CHAR16                                  **String
)
{

  *String = mBiosStorageNameString;

  return EFI_SUCCESS;
}


/**
 Return the count of event count.
            
 @param[out]        Count     Event count of BIOS storage.        
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosGetEventCount (
  IN  OUT UINTN                           *Count
  )
{
  *Count = mBiosStorageNum;
  
  return EFI_SUCCESS;
}

/**
 Locate Event log protocol to get logged data in BIOS.
              
 @param[out]        DataCount     Number of data count in BIOS storage.        
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosRefreshDatabase (
  IN OUT  UINTN                   *DataCount
  )
{
  EFI_STATUS                                Status;
  EFI_BIOS_EVENT_LOG_PROTOCOL               *EventLog;  
  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL  *EventLogUseVariable;  
  UINT8                                     *EventBuffer = NULL;
  BIOS_EVENT_LOG_ORGANIZATION               *BiosEvent;
  UINTN                                     EventCount;
  UINTN                                     EventIndex;
  UINTN                                     Index;

  FreeBiosEventDatabase ();
  *DataCount = 0;

  if (FeaturePcdGet(PcdH2OBiosEventStorageUseVariableStorageSupported)) {
    //
    // BIOS Event Log Use Variable protocol: use EFI Variable.
    //
    Status = gBS->LocateProtocol (&gH2OBiosEventLogUseVariableProtocolGuid, NULL, (VOID **)&EventLogUseVariable);
    if (EFI_ERROR (Status)) {
      return Status;
    } 

    EventCount = 0;
    Status = EventLogUseVariable->Read (EventLogUseVariable, (VOID **)&EventBuffer, &EventCount);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    
    DEBUG ((EFI_D_INFO, "BIOS Event Buffer: 0x%x\n", EventBuffer));
    DEBUG ((EFI_D_INFO, "BIOS Event Count: %d\n", EventCount));
    if (EventCount == 0) {
      return EFI_SUCCESS;
    }
    
    mBiosStorageNum = EventCount;
  
    mBiosStorage = AllocateZeroPool (sizeof(STORAGE_EVENT_LOG_INFO) * mBiosStorageNum);
    if (mBiosStorage == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      Index = 0;
      EventIndex = 0;
      while (EventIndex < (mBiosStorageNum)) {      
        BiosEvent = (BIOS_EVENT_LOG_ORGANIZATION *) ((UINT8*)EventBuffer + Index);
        DEBUG ((EFI_D_INFO, "BiosEvent[%d]: 0x%x\n", EventIndex, BiosEvent));
        mBiosStorage[EventIndex].EventRawData = AllocateZeroPool (BiosEvent->Length);
        if (mBiosStorage[EventIndex].EventRawData == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          break;
        }
  
        mBiosStorage[EventIndex].NextRecordId = 0;
        mBiosStorage[EventIndex].RecordId     = 0;
        mBiosStorage[EventIndex].RecordType   = 0;
  
        
        mBiosStorage[EventIndex].Year    = 2000 + TimeTranslator (BiosEvent->Year);
        mBiosStorage[EventIndex].Month   = TimeTranslator (BiosEvent->Month);
        mBiosStorage[EventIndex].Date    = TimeTranslator (BiosEvent->Day);
        mBiosStorage[EventIndex].Hour    = TimeTranslator (BiosEvent->Hour);
        mBiosStorage[EventIndex].Minute  = TimeTranslator (BiosEvent->Minute);
        mBiosStorage[EventIndex].Second  = TimeTranslator (BiosEvent->Second);
  
        mBiosStorage[EventIndex].GeneratorId       = 0;
        mBiosStorage[EventIndex].GeneratorIdEnable = FALSE;
        mBiosStorage[EventIndex].EvMRev            = 0;
  
        if (BiosEvent->Length > (sizeof(BIOS_EVENT_LOG_ORGANIZATION) - EVENT_LOG_DATA_SIZE)) {
          mBiosStorage[EventIndex].EventID.SensorType = BiosEvent->Data[0];
          mBiosStorage[EventIndex].EventID.SensorNum  = BiosEvent->Data[1];
          mBiosStorage[EventIndex].EventID.EventType  = BiosEvent->Data[2];
          mBiosStorage[EventIndex].Data               = &(mBiosStorage[EventIndex].EventRawData[11]);
        } else {
          mBiosStorage[EventIndex].EventID.SensorType = 0xFF;
          mBiosStorage[EventIndex].EventID.SensorNum  = 0xFF;
          mBiosStorage[EventIndex].EventID.EventType  = 0xFF;
          mBiosStorage[EventIndex].Data               = NULL;
        }
       
        mBiosStorage[EventIndex].EventIdEnable      = TRUE;
        mBiosStorage[EventIndex].EventTypeID       = BiosEvent->Type;
        mBiosStorage[EventIndex].EventTypeIdEnable = TRUE;
        mBiosStorage[EventIndex].EventRawDataSize  = BiosEvent->Length;
        CopyMem (
          (UINT8 *)mBiosStorage[EventIndex].EventRawData, 
          (UINT8 *)&(BiosEvent->Type), 
          mBiosStorage[EventIndex].EventRawDataSize
          );
        
        Index += BiosEvent->Length;
        EventIndex++;
      }
  
      mBiosStorageNum = EventIndex;
    }
    
    *DataCount = mBiosStorageNum;
    FreePool (EventBuffer);

    if (mBiosStorageNum > 1) {    
      SortBiosStorage ();
    }
  } else {
    //
    // BIOS Event Log protocol: use GPNV region.
    //
//[-start-171212-IB08400542-modify]//
    Status = gBS->LocateProtocol (&gH2OBiosEventLogProtocolGuid, NULL, (VOID **)&EventLog);
//[-end-171212-IB08400542-modify]//
    if (EFI_ERROR (Status)) {
      return Status;
    } 

    //
    // Prepare Event Log Data From BIOS ROM Part
    //
    EventCount = 0;
    BiosEvent = NULL;
    while (TRUE) {
      Status = EventLog->ReadNext (EventLog, (VOID **)&BiosEvent);
      if (EFI_ERROR (Status) || (BiosEvent->Type == 0xFF) && (BiosEvent->Length == 0xFF)) {
        break; 
      }
      EventCount++;
    }

    mBiosStorageNum = EventCount;
    //
    // If there are no logs exist, return to main menu
    //
    if (mBiosStorageNum == 0) {
      *DataCount = 0;
      return EFI_SUCCESS;
    }

    //
    // Fill in the BiosStorageChipset with BiosEvet
    //
    mBiosStorage = AllocateZeroPool (sizeof(STORAGE_EVENT_LOG_INFO)*mBiosStorageNum);
    if (mBiosStorage == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  
    BiosEvent = NULL;
    EventIndex = 0;
    while (TRUE) {
      Status = EventLog->ReadNext (EventLog, (VOID **)&BiosEvent);
      if (EFI_ERROR (Status) || (BiosEvent->Type == 0xFF) && (BiosEvent->Length == 0xFF)) {
        break;
      }
      
      mBiosStorage[EventIndex].EventRawData = AllocateZeroPool (BiosEvent->Length);
      if (mBiosStorage[EventIndex].EventRawData == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }        
      mBiosStorage[EventIndex].NextRecordId = 0;
      mBiosStorage[EventIndex].RecordId     = 0;
      mBiosStorage[EventIndex].RecordType   = 0;
      
      mBiosStorage[EventIndex].Year    = 2000 + TimeTranslator (BiosEvent->Year);
      mBiosStorage[EventIndex].Month   = TimeTranslator (BiosEvent->Month);
      mBiosStorage[EventIndex].Date    = TimeTranslator (BiosEvent->Day);
      mBiosStorage[EventIndex].Hour    = TimeTranslator (BiosEvent->Hour);
      mBiosStorage[EventIndex].Minute  = TimeTranslator (BiosEvent->Minute);
      mBiosStorage[EventIndex].Second  = TimeTranslator (BiosEvent->Second);
      
      mBiosStorage[EventIndex].GeneratorId       = 0;
      mBiosStorage[EventIndex].GeneratorIdEnable = FALSE;
      mBiosStorage[EventIndex].EvMRev            = 0;
          
      if (BiosEvent->Length > (sizeof(BIOS_EVENT_LOG_ORGANIZATION) - EVENT_LOG_DATA_SIZE)) {
        mBiosStorage[EventIndex].EventID.SensorType = BiosEvent->Data[0];
        mBiosStorage[EventIndex].EventID.SensorNum  = BiosEvent->Data[1];
        mBiosStorage[EventIndex].EventID.EventType  = BiosEvent->Data[2];
        mBiosStorage[EventIndex].Data               = &(mBiosStorage[EventIndex].EventRawData[11]);
      } else {
        mBiosStorage[EventIndex].EventID.SensorType = 0xFF;
        mBiosStorage[EventIndex].EventID.SensorNum  = 0xFF;
        mBiosStorage[EventIndex].EventID.EventType  = 0xFF;
        mBiosStorage[EventIndex].Data               = NULL;
      }
      
      mBiosStorage[EventIndex].EventIdEnable      = TRUE;
      mBiosStorage[EventIndex].EventTypeID       = BiosEvent->Type;
      mBiosStorage[EventIndex].EventTypeIdEnable = TRUE;
      mBiosStorage[EventIndex].EventRawDataSize  = BiosEvent->Length;
      if (mBiosStorage[EventIndex].EventRawData != NULL) {
        CopyMem (
         (UINT8 *)mBiosStorage[EventIndex].EventRawData, 
         (UINT8 *)&(BiosEvent->Type), 
         mBiosStorage[EventIndex].EventRawDataSize
         );
      }
      EventIndex++;
      mBiosStorageNum = EventIndex;
    }

    *DataCount = mBiosStorageNum;
  }
  if (mBiosStorageNum != 0) {
    return EFI_SUCCESS;
  } else {
    return Status;
  }
}

/**
 Clear logged data.

 Void           
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosClearEvent (
  VOID
)
{
  EFI_STATUS                         Status;
  EFI_BIOS_EVENT_LOG_PROTOCOL        *EventLog;  
  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL  *EventLogUseVariable;  

  if (FeaturePcdGet(PcdH2OBiosEventStorageUseVariableStorageSupported)) {
    //
    // BIOS Event Log Use Variable protocol: use EFI Variable.
    //
    Status = gBS->LocateProtocol (&gH2OBiosEventLogUseVariableProtocolGuid, NULL, (VOID **)&EventLogUseVariable);
    if (EFI_ERROR (Status)) {
      return Status;
    } 

    //
    // Clear BIOS logged data.
    //
    Status = EventLogUseVariable->Clear(EventLogUseVariable);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  } else {
//[-start-171212-IB08400542-modify]//
    Status = gBS->LocateProtocol (&gH2OBiosEventLogProtocolGuid, NULL, (VOID **)&EventLog);
//[-end-171212-IB08400542-modify]//
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Clear BIOS logged data.
    //
    Status = EventLog->Clear (EventLog);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (mBiosStorageNum == 0) {
    return EFI_SUCCESS;
  }
  //
  // Clear BIOS event log buffer.
  //
  if (mClearActionWithOverwrite == FALSE) {
    FreeBiosEventDatabase ();
  } else {
    //
    // Overwrite action this time, and need to reset the global parameter mClearActionWithOverwrite.
    //
    mClearActionWithOverwrite = FALSE;
  }
  return EFI_SUCCESS;
}

/**
 Write the BIOS Event Log with overwrite method.

 @param [in] InputBuffer          The event log data to logging..            
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosOverwriteEvent (
  IN  BIOS_EVENT_LOG_ORGANIZATION                    *InputBuffer
)
{
  EFI_STATUS                                Status;
  EFI_BIOS_EVENT_LOG_PROTOCOL               *EventLog;    
  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL  *EventLogUseVariable;  

  if (FeaturePcdGet(PcdH2OBiosEventStorageUseVariableStorageSupported)) {
    //
    // BIOS Event Log Use Variable protocol: use EFI Variable.
    //
    Status = gBS->LocateProtocol (&gH2OBiosEventLogUseVariableProtocolGuid, NULL, (VOID **)&EventLogUseVariable);
    if (EFI_ERROR (Status)) {
      return Status;
    } 

    Status = EventLogUseVariable->OverWrite (EventLogUseVariable, InputBuffer);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    
  } else {  
    //
    // BIOS Event Log protocol: use GPNV region.
    //
//[-start-171212-IB08400542-modify]//
    Status = gBS->LocateProtocol (&gH2OBiosEventLogProtocolGuid, NULL, (VOID **)&EventLog);
//[-end-171212-IB08400542-modify]//
    if (EFI_ERROR (Status)) {
      return Status;
    }
    
    Status = EventLog->OverWrite (EventLog, InputBuffer);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return Status;
}

/**
 Translate time format to numerical value.

 @param[in]         OrgTime    Original time format.                       
 
 @retval UINT8      Time                  
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
 Free BIOS event data buffer.

 VOID           
 
 @retval VOID            
*/
VOID
EFIAPI
FreeBiosEventDatabase (
  VOID
  )
{
  UINTN                     Index;
  
  if (mBiosStorageNum != 0) { 
    //
    // Clear data of event data buffer.
    //
    for (Index = 0; Index < mBiosStorageNum; Index++) {
      if (mBiosStorage[Index].EventRawData != NULL) {
        FreePool (mBiosStorage[Index].EventRawData);
        mBiosStorage[Index].EventRawData = NULL;
      }
    }

    if (mBiosStorage != NULL) {
      FreePool (mBiosStorage);
      mBiosStorage = NULL;
    }
    mBiosStorageNum = 0;
  }
}

/**
 Adding an event data after arranging the storage.

 VOID
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
AddEventAfterArrangeEventStorage (
  VOID
  )
{
  EFI_STATUS                         Status;
  EFI_BIOS_EVENT_LOG_PROTOCOL        *EventLog;  
  UINT8                              LogData[EVENT_LOG_DATA_SIZE];
  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL  *EventLogUseVariable;  

  if (FeaturePcdGet(PcdH2OBiosEventStorageUseVariableStorageSupported)) {
    //
    // BIOS Event Log Use Variable protocol: use EFI Variable.
    //
    Status = gBS->LocateProtocol (&gH2OBiosEventLogUseVariableProtocolGuid, NULL, (VOID **)&EventLogUseVariable);
    if (EFI_ERROR (Status)) {
      return Status;
    } 
    LogData[0] = OEM_SENSOR_TYPE_EVENT_STORAGE;
    LogData[1] = BIOS_DEFINED_SENSOR_NUM;
    LogData[2] = EVENT_TYPE_OEM;

    LogData[3] = OEM_SENSOR_OVERWRITE;
    LogData[4] = 0xFF;
    LogData[5] = CHANGE_BIOS_STORAGE_DATA3;
    
    Status = EventLogUseVariable->Write (
                                    EventLogUseVariable,
                                    0x80,
                                    0,
                                    0,
                                    EVENT_LOG_DATA_SIZE,
                                    LogData
                                    ); 
  } else {
//[-start-171212-IB08400542-modify]//
    Status = gBS->LocateProtocol (&gH2OBiosEventLogProtocolGuid, NULL, (VOID **)&EventLog);
//[-end-171212-IB08400542-modify]//
    if (EFI_ERROR (Status)) {
      return Status;
    }
    
    LogData[0] = OEM_SENSOR_TYPE_EVENT_STORAGE;
    LogData[1] = BIOS_DEFINED_SENSOR_NUM;
    LogData[2] = EVENT_TYPE_OEM;

    LogData[3] = OEM_SENSOR_OVERWRITE;
    LogData[4] = 0xFF;
    LogData[5] = CHANGE_BIOS_STORAGE_DATA3;
    
    Status = EventLog->Write (
                         EventLog,
                         0x80,
                         0,
                         0,
                         EVENT_LOG_DATA_SIZE,
                         LogData
                         ); 
  }

  return Status;
}

/**
 Arrange the data in the storage.

 VOID           
 
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
    Status = BiosClearEvent();
    break;

  default:
    Status = EFI_UNSUPPORTED;
    break;
  }

  if (!EFI_ERROR (Status)) {
    //
    // Logging an event after arranging the storage.
    //
    AddEventAfterArrangeEventStorage ();
  }
  
  return Status;
}

/**
 Shift event data of the storage.

 VOID          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
ShiftDataOfStorage (
  VOID
  )
{
  EFI_STATUS                      Status;
  UINTN                           BiosStorageChipsetCount;
  UINTN                           Index;

  Status = BiosRefreshDatabase (&BiosStorageChipsetCount);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  if (BiosStorageChipsetCount <= EVENT_LOG_FULL_ADJUST_EVENT_NUM) {
    //
    // If Current count of logged event is equal or less than the adjusting count,
    // Clear all event data immediately.
    //
    Status= BiosClearEvent();
    return Status;
  }

  //
  // clear original event data of the storage.
  //
  mClearActionWithOverwrite = TRUE;
  Status = BiosClearEvent();
  if (EFI_ERROR (Status)) { 
    DEBUG ((EFI_D_ERROR, "ShiftDataOfStorage() BiosClearEvent() Status: %r\n", Status));    
  } else {
    for (Index = EVENT_LOG_FULL_ADJUST_EVENT_NUM; Index < BiosStorageChipsetCount; Index++) {   
      //
      // Restore the adjusted event data into the storage.
      //
      Status = BiosOverwriteEvent((BIOS_EVENT_LOG_ORGANIZATION *)(mBiosStorage[Index].EventRawData));
      if (EFI_ERROR (Status)) { 
        DEBUG ((EFI_D_ERROR, "mBiosStorage[%d] BiosOverwriteEvent() Status: %r\n", Index, Status));
        break;
      }
    } 
  }

  FreeBiosEventDatabase ();
  return Status;
}

/**
 Translate string of BIOS event log.

 @param[in]         EventInfo        Information of BIOS event log data.        
 @param[out]        EventString      Returned string of the input BIOS event log data.      
 @param[out]        StringSize       Size of EventString.      
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EventLogStringHandler (
  IN VOID                                   *EventInfo,
  IN OUT CHAR16                             **EventString,
  IN OUT UINTN                              *StringSize
  )
{
  EFI_STATUS                            Status = EFI_NOT_FOUND;
  STORAGE_EVENT_LOG_INFO                *StorageEventInfo;
  
  StorageEventInfo = (STORAGE_EVENT_LOG_INFO*) EventInfo;
  if ((StorageEventInfo->EventTypeIdEnable != TRUE)) {
    //
    // Not BIOS event log event, just return not found.
    //
    return EFI_NOT_FOUND;
  }

  switch (StorageEventInfo->EventTypeID) {
  case EfiEventLogTypeLogAreaResetCleared:
    Status = BiosEventLogStringType16 (EventInfo, EventString, StringSize);
    break;
    
  default:
    Status = BiosEventLogStringType80 (EventInfo, EventString, StringSize);
    break;
  }
  
  return Status;
}

/**
 Translate string of BIOS event log for Event Type is 0x16.          

 @param[in]  EventInfo    Information of BIOS event log data.
 @param[out] EventString  Returned string of the input BIOS event log data.
 @param[out] StringSize   Size of EventString.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosEventLogStringType16 (
  IN VOID                                   *EventInfo OPTIONAL,
  IN OUT CHAR16                             **EventString,
  IN OUT UINTN                              *StringSize
  )
{
  VOID                       *TempString = NULL;
  CHAR16                     *EventStr = L"Event Log has been Cleared.";

  *StringSize = StrLen (EventStr) * 2 + 2;
  TempString = AllocateZeroPool (*StringSize);
  if (TempString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
//[-start-180724-IB08400617-modify]//
  StrCpyS(TempString, MAX_ELV_STR_NUM, EventStr);
//[-end-180724-IB08400617-modify]//

  *EventString = TempString;
  return EFI_SUCCESS;
}

/**
 Translate string of BIOS event log for Event Type is 0x80.           

 @param[in]   EventInfo     Information of BIOS event log data.
 @param[out]  EventString   Returned string of the input BIOS event log data.
 @param[out]  StringSize    Size of EventString.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosEventLogStringType80 (
  IN VOID                                   *EventInfo OPTIONAL,
  IN OUT CHAR16                             **EventString,
  IN OUT UINTN                              *StringSize
  )
{
  VOID                       *TempString=NULL;
  UINTN                      Index;
  UINTN                      NumOfBiosEventStr;
  STORAGE_EVENT_LOG_INFO     *StorageEventInfo;
  EFI_STATUS                 Status = EFI_NOT_FOUND;

  NumOfBiosEventStr = sizeof(gBiosStorageChipsetString) / sizeof(BIOS_EVENT_LOG_STRING);
  StorageEventInfo = (STORAGE_EVENT_LOG_INFO *)EventInfo; 

  if (StorageEventInfo->EventTypeID != 0x80) {
    return EFI_NOT_FOUND;
  }
  
  for (Index = 0; Index < NumOfBiosEventStr; Index++) {
    if (CompareMem(&(StorageEventInfo->EventRawData[8]), gBiosStorageChipsetString[Index].Data, gBiosStorageChipsetString[Index].ValidDataLength) == 0) {
      *StringSize = StrLen (gBiosStorageChipsetString[Index].EventStr) * 2 + 2;
      TempString = AllocateZeroPool (*StringSize);
      if (TempString == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
//[-start-180724-IB08400617-modify]//
      StrCpyS(TempString, MAX_ELV_STR_NUM, gBiosStorageChipsetString[Index].EventStr);
//[-end-180724-IB08400617-modify]//
      *EventString = TempString;  
      return EFI_SUCCESS;
    }
  } 

  return Status;
}


/**
 Entry point of this driver. Install Bios Event Storage protocol into DXE.

 @param[in] ImageHandle       Image handle of this driver.
 @param[in] SystemTable       Global system service table.          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosStorageChipsetDxeEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                            Status;  
  EFI_HANDLE                            Handle=NULL;

  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                                      &Handle,
                                      &gH2OEventStorageProtocolGuid,
                                      EFI_NATIVE_INTERFACE,
                                      &gEventStorageProtocol
                                      );
  if (EFI_ERROR (Status)) {
    return Status;
  } 

  //
  // Install Event String Protocol for user to log Event into BIOS ROM.
  //
  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gH2OEventLogStringProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &gEventLogString
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
     
  return EFI_SUCCESS;

}


/** @file

  BIOS Storage kernel Use Variable Dxe implementation.

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


#include <BiosStorageKernelUseVariableDxe.h>

CHAR16                                   *mBiosElOrderVarName = BIOS_STORAGE_LOG_ORDER_VAR_NAME;
CHAR16                                   *mBiosElViewOrderVarName = BIOS_STORAGE_LOG_VIEW_ORDER_VAR_NAME;
H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL mBiosEventLogUseVariableProtocl = {
                                           WriteEventLog,
                                           ClearEventLog,
                                           ReadEventLog,
                                           OverWriteEventLog
                                           };
EVENT_LOG_FULL_OPTION                    mEventLogFullOption = EVENT_LOG_FULL_OVERWRITE;
BOOLEAN                                  mOverWriteAction = FALSE;


/**
 Get a buffer that contains the related logging data.
 
 @param [in] This                 This protocol interface.
 @param [in] BiosStorageType      Event log type.
 @param [in] PostBitmap1          Post bitmap 1 which will be stored in data area of POST error type log.
 @param [in] PostBitmap2          Post bitmap 2 which will be stored in data area of POST error type log.
 @param [in] OptionDataSize       Optional data size.
 @param [in] OptionLogData        Pointer to optional data.

 @retval EFI Status                  
*/
BIOS_EVENT_LOG_ORGANIZATION *
EFIAPI
GetEventLogBuffer (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL        *This,
  IN  UINT8                                           EventLogType,
  IN  UINT32                                          PostBitmap1,
  IN  UINT32                                          PostBitmap2,
  IN  UINTN                                           OptionDataSize,
  IN  UINT8                                           *OptionLogData
  )
{
  BIOS_EVENT_LOG_ORGANIZATION           *Buffer = NULL;
  UINTN                                 NumBytes;
  UINTN                                 DataFormatTypeLength;
  EFI_STATUS                            Status;
  EL_TIME                               ElTime;

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
  Buffer = AllocateZeroPool (NumBytes);
  if (Buffer == NULL) {
    return NULL;
  } 

  Status = EventLogGetTimeFromCmos (&ElTime);
  if (EFI_ERROR (Status)) {
    return NULL;
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

  return Buffer;
}

/**
 Update the BiosElOrder variable with new coming "BiosElXXXX" variable number.

 @param [in] NewVarNum                 The number to add into the Bios EventLog order.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
UpdateBiosELOrderVariable (
  IN  UINT16                            NewVarNum
  )
{
  EFI_STATUS                        Status;
  UINT16                            *BiosElOrder;
  UINTN                             BiosElOrderSize;
  UINT16                            *NewBiosElOrder;
  UINTN                             NewBiosElOrderSize;

  Status = CommonGetVariableDataAndSize (
             mBiosElOrderVarName,
             &gH2OBiosStorageVariableGuid,
             &BiosElOrderSize,
             (VOID **) &BiosElOrder
             );
  if (EFI_ERROR (Status) || (BiosElOrder == NULL)) {
    //
    // No any record, just save the new variable number as the first one.
    //  
    BiosElOrderSize = sizeof(UINT16);
    BiosElOrder = AllocateZeroPool (BiosElOrderSize);
    if (BiosElOrder == NULL) {
      return EFI_OUT_OF_RESOURCES;
    } else {
      BiosElOrder[0] = NewVarNum;
    }

    Status = CommonSetVariable (
               mBiosElOrderVarName,
               &gH2OBiosStorageVariableGuid,
               EFI_VARIABLE_NON_VOLATILE  | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
               BiosElOrderSize,
               BiosElOrder
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "SetVariable %s failed: %r\n", mBiosElOrderVarName, Status));
    }
    
    FreePool (BiosElOrder);
    return Status;
  } else {
    //
    // Get original BiosElOrder content, and update new variable number to it.
    //
    NewBiosElOrderSize = BiosElOrderSize + sizeof(UINT16);
    NewBiosElOrder = AllocateZeroPool (NewBiosElOrderSize);
    if (NewBiosElOrder == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      CopyMem (NewBiosElOrder, BiosElOrder, BiosElOrderSize);
      NewBiosElOrder[BiosElOrderSize/2] = NewVarNum;

      Status = CommonSetVariable (
                 mBiosElOrderVarName,
                 &gH2OBiosStorageVariableGuid,
                 EFI_VARIABLE_NON_VOLATILE  | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 NewBiosElOrderSize,
                 NewBiosElOrder
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "SetVariable %s failed: %r\n", mBiosElOrderVarName, Status));
      }
      FreePool (NewBiosElOrder);
    }
    
    FreePool (BiosElOrder);
  }
  
  return Status;
}

/**
 Log the data inot the Bios EventLog variable.

 @param [in] CurVarNum                 The variable number for the logging data.
 @param [in] Buffer                    The buffer of logging data.
 @param [in] BufferSize                The size of the logging data buffer.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
LogToVariable (
  IN  UINT16                            CurVarNum,
  IN  BIOS_EVENT_LOG_ORGANIZATION       *Buffer,
  IN  UINTN                             BufferSize
  )
{
  CHAR16                       CurVarName[20];
  EFI_STATUS                   Status;

  UnicodeSPrint (CurVarName, sizeof (CurVarName), L"BiosEl%04x", CurVarNum);

  Status = CommonSetVariable (
             CurVarName,
             &gH2OBiosStorageVariableGuid,
             EFI_VARIABLE_NON_VOLATILE  | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
             BufferSize,
             Buffer
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "SetVariable %s failed: %r\n", CurVarName, Status));
  }
  
  return Status;
}

/**
 Read the data from the Bios EventLog variable.

 @param [out] Buffer                    The buffer of logging data which will be returned.
 @param [out] EventCount                The Event number for the logging data.

 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
ReadEventLogUseVariable (
  OUT VOID                                **Buffer,
  OUT UINTN                               *EventCount
  )
{
  EFI_STATUS                              Status = EFI_SUCCESS;
  UINTN                                   TotalCount;
  UINTN                                   TotalSize;
  BIOS_EVENT_LOG_ORGANIZATION             *EventLogEntry;
  UINT8                                   *EventLogBuffer;
  UINT16                                  *BiosElViewOrder;
  UINTN                                   BiosElViewOrderSize;
  UINT8                                   *BiosEl;
  UINTN                                   BiosElSize;
  UINTN                                   BiosElCount;
  UINTN                                   Index;
  UINTN                                   BiosElIndex;
  UINT16                                  CurrentVarNum;
  CHAR16                                  CurVarName[20];
  
  Status = CommonGetVariableDataAndSize (
             mBiosElViewOrderVarName,
             &gH2OBiosStorageVariableGuid,
             &BiosElViewOrderSize,
             (VOID **) &BiosElViewOrder
             );
  if (EFI_ERROR (Status) || (BiosElViewOrder == NULL)) {
    //
    // Can't get Bios Event Log Order variable. 
    // Should not have any event log record.
    //
    *EventCount = 0;
    return EFI_SUCCESS;
  } else {
    TotalCount = 0;
    TotalSize = 0;
    for (Index = 0; Index < (BiosElViewOrderSize / sizeof(UINT16)); Index++) {
      //
      // Get the current number for Bios Event Log variable name.
      //
      CurrentVarNum = BiosElViewOrder[Index];
      UnicodeSPrint (CurVarName, sizeof (CurVarName), L"BiosEl%04x", CurrentVarNum);
      DEBUG ((EFI_D_INFO, "ReadEventLog >> GetVariable %s: %r\n", CurVarName, Status));

      Status = CommonGetVariableDataAndSize (
                 CurVarName,
                 &gH2OBiosStorageVariableGuid,
                 &BiosElSize,
                 (VOID **) &BiosEl
                 );
      if (EFI_ERROR (Status)) {
        continue;
      }        
      //
      // First, try to get the total count of the Setup Change history
      //
      TotalSize += BiosElSize;
      BiosElIndex = 0;
      BiosElCount = 0;
      while (BiosElIndex <= (BiosElSize -1)) {
        EventLogEntry = (BIOS_EVENT_LOG_ORGANIZATION *)((UINT8*)(BiosEl) + BiosElIndex);
        
        BiosElIndex += EventLogEntry->Length;
        BiosElCount++;
      }
      DEBUG ((EFI_D_INFO, "Variable %s: BiosElCount: %d\n", CurVarName, BiosElCount));
      FreePool (BiosEl);
    }

    //
    // Base on the total size to allocate the enough buffer,
    // and fill up the event log data into the buffer.
    //
    EventLogBuffer = AllocateZeroPool (TotalSize);
    if (EventLogBuffer == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      DEBUG ((EFI_D_INFO, "BIOS Event Buffer: 0x%x\n", EventLogBuffer));
      //
      // Second, fill the event log data to the buffer.
      //
      TotalSize = 0;
      for (Index = 0; Index < (BiosElViewOrderSize / sizeof(UINT16)); Index++) {
        //
        // Get the current number for Bios Event Log variable name.
        //
        CurrentVarNum = BiosElViewOrder[Index];
        UnicodeSPrint (CurVarName, sizeof (CurVarName), L"BiosEl%04x", CurrentVarNum);
        DEBUG ((EFI_D_INFO, "ReadEventLog >> GetVariable %s: %r\n", CurVarName, Status));
      
        Status = CommonGetVariableDataAndSize (
                   CurVarName,
                   &gH2OBiosStorageVariableGuid,
                   &BiosElSize,
                   (VOID **) &BiosEl
                   );
        if (EFI_ERROR (Status)) {
          continue;
        }        
        //
        // First, try to get the total count of the Setup Change history
        //
        BiosElIndex = 0;
        BiosElCount = 0;
        while (BiosElIndex <= (BiosElSize -1)) {
          EventLogEntry = (BIOS_EVENT_LOG_ORGANIZATION *)((UINT8*)(BiosEl) + BiosElIndex);
          //
          // copy event data.
          //
          CopyMem (&(EventLogBuffer[TotalSize]), EventLogEntry, EventLogEntry->Length);
          DEBUG ((EFI_D_INFO, "EventLogEntry[%d]: 0x%x\n", BiosElCount, EventLogEntry));
          DEBUG ((EFI_D_INFO, "EventLogBuffer[%d] : 0x%x\n", BiosElCount, &EventLogBuffer[TotalSize]));
          
          BiosElIndex += EventLogEntry->Length;
          TotalSize += EventLogEntry->Length;
          BiosElCount++;
        }
      
        TotalCount += BiosElCount;
      
        DEBUG ((EFI_D_INFO, "Variable %s: BiosElCount: %d\n", CurVarName, BiosElCount));
        FreePool (BiosEl);
      }
    }
  }

  *EventCount = TotalCount;
  *Buffer = EventLogBuffer;
  FreePool (BiosElViewOrder);
  return EFI_SUCCESS;
}

/**
 Write the data to the Bios EventLog variable.

 @param [in] Buffer       The buffer of logging data.

 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
WriteEventLogUseVariable (
  IN  BIOS_EVENT_LOG_ORGANIZATION       *Buffer,
  IN  UINT16                            CurrentVarNum
  )
{
  EFI_STATUS                    Status = EFI_SUCCESS;
  UINT8                         *BiosEl;
  UINTN                         BiosElSize;
  UINT8                         *NewBiosEl;
  UINTN                         NewBiosElSize;
  CHAR16                        CurVarName[20];
  
  //
  // Read current variable.
  //
  UnicodeSPrint (CurVarName, sizeof (CurVarName), L"BiosEl%04x", CurrentVarNum);
  Status = CommonGetVariableDataAndSize (
             CurVarName,
             &gH2OBiosStorageVariableGuid,
             &BiosElSize,
             (VOID **) &BiosEl
             );
  if (EFI_ERROR (Status)) {
    //
    // Can't get the current variable.
    //
    DEBUG ((EFI_D_ERROR, "GetVariable %s failed: %r\n", CurVarName, Status));

    //
    // To avoid losing data, Just log the data to variable.      
    //
    UnicodeSPrint (CurVarName, sizeof (CurVarName), L"BiosEl%04x", CurrentVarNum);
    Status = LogToVariable (CurrentVarNum, Buffer, Buffer->Length);
    DEBUG ((EFI_D_INFO, "Save the event data to variable %s: %r\n", CurVarName, Status));

    //
    // Update BiosELOrder with new variable number.
    //
    UpdateBiosELOrderVariable (CurrentVarNum);
  } else {
    if ((BiosElSize + Buffer->Length) <= MAX_BIOS_STORAGE_VARIABLE_SIZE) {
      //
      // Available: update the current variable.
      //
      NewBiosElSize = BiosElSize + Buffer->Length;
      NewBiosEl = AllocateZeroPool (NewBiosElSize);
      if (NewBiosEl == NULL) {
        Status = EFI_OUT_OF_RESOURCES;
      } else {
        //
        // Update current variable content.
        //
        CopyMem (NewBiosEl, BiosEl, BiosElSize);
        CopyMem (&(NewBiosEl[BiosElSize]), Buffer, Buffer->Length);
        
        Status = CommonSetVariable (
                   CurVarName,
                   &gH2OBiosStorageVariableGuid,
                   EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                   NewBiosElSize,
                   NewBiosEl
                   );
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_ERROR, "SetVariable %s failed: %r\n", CurVarName, Status));
        }
        FreePool (NewBiosEl);
      }
    } else {
      //
      // Variable space is full.
      //
      Status = EFI_OUT_OF_RESOURCES;
    }
    FreePool (BiosEl);
  }

  if ((Status == EFI_OUT_OF_RESOURCES) && (mEventLogFullOption != EVENT_LOG_FULL_STOP_LOGGING)) {
    //
    // Event Log space is full, and need to overwrite or clear the space, 
    // so don't delete the variable of BIOS Storage log number.
    //
    mOverWriteAction = TRUE;
   
  }
  
  return Status;
}

/**
 
 Clear the data from the Bios EventLog variable.

 @param void
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
ClearEventLogUseVariable (
  IN   CLEAR_EVT_TYPE                    ClearEvtType,
  IN   UINT16                            ClearVarNum
  )
{
  EFI_STATUS                    Status;
  UINT16                        *BiosElOrder;
  UINTN                         BiosElOrderSize;
  UINTN                         Index;
  UINT16                        CurrentVarNum;
  CHAR16                        CurVarName[20];
  CHAR16                        *BiosElOrderName;
  UINT16                        *BiosElOrder2;
  UINTN                         BiosElOrder2Size;
  UINTN                         Index2;
  UINTN                         Count;

  if (ClearEvtType == CLEAR_EVT_BY_NUM) {
    //
    // Clear Event Type: Clear by Number.
    //   - Used by Overwrite action.
    //   - Get the current number for Bios Event Log variable name.
    //
    CurrentVarNum = ClearVarNum;
    UnicodeSPrint (CurVarName, sizeof (CurVarName), L"BiosEl%04x", CurrentVarNum);

    Status = CommonSetVariable (
               CurVarName,
               &gH2OBiosStorageVariableGuid,
               0,
               0,
               NULL
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "ClearEventLog >> Delete variable %s failed: %r\n", CurVarName, Status));
    } 
    //
    // Clear complete, so return EFI_SUCCESS.
    //
    Status = EFI_SUCCESS;
  } else {
    //
    // Clear Event Type: Clear all.
    //
    if (FixedPcdGetBool(PcdH2OBiosEventStorageUseVariableClearEventByAllOrders) == TRUE) {
      BiosElOrderName = mBiosElOrderVarName;
    } else {
      BiosElOrderName = mBiosElViewOrderVarName;
    }
    
    Status = CommonGetVariableDataAndSize (
               BiosElOrderName,
               &gH2OBiosStorageVariableGuid,
               &BiosElOrderSize,
               (VOID **) &BiosElOrder
               );
    if (EFI_ERROR (Status) || (BiosElOrder == NULL)) {
      //
      // Can't get Bios Event Log Order variable. 
      // Should not have any event log record.
      //
      return EFI_SUCCESS;
    } else {
      for (Index = 0; Index < (BiosElOrderSize / sizeof(UINT16)); Index++) {
        //
        // Get the current number for Bios Event Log variable name.
        //
        CurrentVarNum = BiosElOrder[Index];
        UnicodeSPrint (CurVarName, sizeof (CurVarName), L"BiosEl%04x", CurrentVarNum);
        DEBUG ((EFI_D_INFO, "ClearEventLog >> GetVariable %s: %r\n", CurVarName, Status));
    
        Status = CommonSetVariable (
                   CurVarName,
                   &gH2OBiosStorageVariableGuid,
                   0,
                   0,
                   NULL
                   );
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_ERROR, "ClearEventLog >> Delete variable %s failed: %r\n", CurVarName, Status));
        }        
      }
      if (FixedPcdGetBool(PcdH2OBiosEventStorageUseVariableClearEventByAllOrders) == TRUE) {
        BiosElOrderName = mBiosElOrderVarName;
        Status = CommonSetVariable (
                   BiosElOrderName,
                   &gH2OBiosStorageVariableGuid,
                   0,
                   0,
                   NULL
                   );
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_ERROR, "ClearEventLog >> Delete variable %s failed: %r\n", mBiosElOrderVarName, Status));
        }
    
        BiosElOrderName = mBiosElViewOrderVarName;
        Status = CommonSetVariable (
                   BiosElOrderName,
                   &gH2OBiosStorageVariableGuid,
                   0,
                   0,
                   NULL
                   );
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_ERROR, "ClearEventLog >> Delete variable %s failed: %r\n", mBiosElOrderVarName, Status));
        }

        //
        // Clear complete, so return EFI_SUCCESS.
        //
        Status = EFI_SUCCESS;
      } else {
        BiosElOrderName = mBiosElViewOrderVarName;
        //
        // Get original BiosElOrder
        //
        Status = CommonGetVariableDataAndSize (
                   mBiosElOrderVarName,
                   &gH2OBiosStorageVariableGuid,
                   &BiosElOrder2Size,
                   (VOID **) &BiosElOrder2
                   );
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_ERROR, "ClearEventLog >> Get variable %s failed: %r\n", mBiosElOrderVarName, Status));
        } else {
          //
          // Update BiosElOrder by remove the number in the BiosElViewOrder.
          //
          for (Index = 0; Index < (BiosElOrderSize/sizeof(UINT16)); Index++) {
            for (Index2 = 0; Index2 < (BiosElOrder2Size/sizeof(UINT16)); Index2++) {
              if (BiosElOrder[Index] == BiosElOrder2[Index2]) {
                BiosElOrder2[Index2] = RESERVED_VAR_NUMBER;
                break;
              }
            }
          }
    
          Count = 0;
          for (Index2 = 0; Index2 < (BiosElOrder2Size/sizeof(UINT16)); Index2++) {
            if (BiosElOrder2[Index2] != RESERVED_VAR_NUMBER) {
              BiosElOrder2[Count] = BiosElOrder2[Index2];
              Count++;
            }
          }
          Status = CommonSetVariable (
                     mBiosElOrderVarName,
                     &gH2OBiosStorageVariableGuid,
                     EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                     Count*sizeof(UINT16),
                     BiosElOrder2
                     );
          if (EFI_ERROR (Status)) {
            DEBUG ((EFI_D_ERROR, "ClearEventLog >> Get variable %s failed: %r\n", mBiosElOrderVarName, Status));
          }
    
          FreePool (BiosElOrder2);
          Status = CommonSetVariable (
                     mBiosElViewOrderVarName,
                     &gH2OBiosStorageVariableGuid,
                     0,
                     0,
                     NULL
                     );
          if (EFI_ERROR (Status)) {
            DEBUG ((EFI_D_ERROR, "ClearEventLog >> Set variable %s failed: %r\n", mBiosElViewOrderVarName, Status));
          }      
        }
        //
        // Clear complete, so return EFI_SUCCESS.
        //
        Status = EFI_SUCCESS;
      }
    }
  }

  return Status;
}

/**
 Read the data from the Bios EventLog variable.

 @param [in]  This                      This protocol interface.
 @param [out] Buffer                    The buffer of logging data which will be returned.
 @param [out] EventCount                The Event number for the logging data. 

 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
ReadEventLog (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL         *This,
  OUT VOID                                             **Buffer,
  OUT UINTN                                            *EventCount
  )
{
  EFI_STATUS                            Status = EFI_UNSUPPORTED;
  
  Status = ReadEventLogUseVariable (Buffer, EventCount);
  return Status;
}

/**
  Write event to BIOS Storage.

  @param [in] This                 This protocol interface.
  @param [in] BiosStorageType      Event log type.
  @param [in] PostBitmap1          Post bitmap 1 which will be stored in data area of POST error type log.
  @param [in] PostBitmap2          Post bitmap 2 which will be stored in data area of POST error type log.
  @param [in] OptionDataSize       Optional data size.
  @param [in] OptionLogData        Pointer to optional data.
  
  @retval EFI Status            
**/
EFI_STATUS
EFIAPI
WriteEventLog (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL        *This,
  IN  UINT8                                           EventLogType,
  IN  UINT32                                          PostBitmap1,
  IN  UINT32                                          PostBitmap2,
  IN  UINTN                                           OptionDataSize,
  IN  UINT8                                           *OptionLogData
  )
{
  EFI_STATUS                            Status = EFI_UNSUPPORTED;
  BIOS_EVENT_LOG_ORGANIZATION           *Buffer;  
  UINTN                                 LogVarSize;
  UINT16                                *LogVarNum;  
  UINT16                                CurrentVarNum;

  if ((OptionDataSize == 0) || (OptionLogData == NULL)) {
    OptionDataSize = 0;
    OptionLogData = NULL;
  }

  Buffer = NULL;
  Buffer = GetEventLogBuffer (This, EventLogType, PostBitmap1, PostBitmap2, OptionDataSize, OptionLogData);
  if (Buffer == NULL) {    
    DEBUG ((EFI_D_ERROR, "WriteEventLog(): GetEventLogBuffer failed.\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Get the current number for Bios Event Log variable name.
  //
  LogVarSize = sizeof(UINT16);
  Status = CommonGetVariableDataAndSize (
             BIOS_STORAGE_LOG_NUMBER_VAR_NAME,
             &gH2OBiosStorageVariableGuid,
             &LogVarSize,
             (VOID **) &LogVarNum
             );
  if (EFI_ERROR (Status)) {
    CurrentVarNum = 0;
  } else {
    CurrentVarNum = *LogVarNum;
    FreePool (LogVarNum);
  }
  
  Status = WriteEventLogUseVariable (Buffer, CurrentVarNum);
  
  FreePool (Buffer);
  
  return Status;
}

/**
 Clear function of Event log.           
 
 @param [in] This                 This protocol interface.

 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
ClearEventLog (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL       *This
  )
{
  EFI_STATUS                            Status = EFI_UNSUPPORTED;
  UINT16                                BiosElNum;
  UINTN                                 LogVarSize;
  UINT16                                *LogVarNum;  
  UINT16                                CurrentVarNum;
  CLEAR_EVT_TYPE                        ClearEvtType;

  if (mOverWriteAction == TRUE) {
    //
    // Get the current number for Bios Event Log variable name.
    //
    LogVarSize = sizeof(UINT16);
    Status = CommonGetVariableDataAndSize (
               BIOS_STORAGE_LOG_NUMBER_VAR_NAME,
               &gH2OBiosStorageVariableGuid,
               &LogVarSize,
               (VOID **) &LogVarNum
               );
    if (EFI_ERROR (Status)) {
      CurrentVarNum = 0;
    } else {
      CurrentVarNum = *LogVarNum;
    }
    
    ClearEvtType = CLEAR_EVT_BY_NUM;    
    Status = ClearEventLogUseVariable (ClearEvtType, CurrentVarNum);

  } else {
    ClearEvtType = CLEAR_EVT_ALL;
    CurrentVarNum = 0;
    Status = ClearEventLogUseVariable (ClearEvtType, CurrentVarNum);
    if (!EFI_ERROR (Status)) {
      //
      // Set BIOS Event Log view number with 0 as default.
      //
      BiosElNum = 0;
      Status = CommonSetVariable (
                 mBiosElViewOrderVarName,
                 &gH2OBiosStorageVariableGuid,
                 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 sizeof(UINT16),
                 &BiosElNum
                 );
    }  
  }
  
  //
  // Log an event as cleared event.
  //
  WriteEventLog (This, 0x16, 0, 0, 0, NULL);
  
  return Status;
}

/**
  Write the BIOS Event Log with overwrite method.

  @param [in] This                 This protocol interface.
  @param [in] InputBuffer          The event log data to logging.
  
  @retval EFI Status            
**/
EFI_STATUS
EFIAPI
OverWriteEventLog (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL       *This,
  IN  BIOS_EVENT_LOG_ORGANIZATION                    *InputBuffer
  )
{
  EFI_STATUS                            Status;
  UINTN                                 LogVarSize;
  UINT16                                *LogVarNum;  
  UINT16                                CurrentVarNum;

  //
  // Get the current number for Bios Event Log variable name.
  //
  LogVarSize = sizeof(UINT16);
  Status = CommonGetVariableDataAndSize (
             BIOS_STORAGE_LOG_NUMBER_VAR_NAME,
             &gH2OBiosStorageVariableGuid,
             &LogVarSize,
             (VOID **) &LogVarNum
             );
  if (EFI_ERROR (Status)) {
    CurrentVarNum = 0;
  } else {
    CurrentVarNum = *LogVarNum;
    FreePool (LogVarNum);
  }
  
  Status = WriteEventLogUseVariable (InputBuffer, CurrentVarNum);
  
  mOverWriteAction = FALSE;
  
  return Status;
}

/**
  Callback function for register Efi Variable Arch Protocol.

  @param[in] Event    Event whose notification function is being invoked.
  @param[in] Context  Pointer to the notification function's context.
**/
VOID
EFIAPI
EfiVariableWriteArchProtocolRegisterCallback (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  )
{
  SetBiosStorageLogLib (0, TRUE);
}

/**
 Entry point of this driver. Install Event Log protocol into DXE.

 @param[in] ImageHandle       Image handle of this driver.
 @param[in] SystemTable       Global system service table.          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosStorageKernelUseVariableEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                          Status;
  EFI_HANDLE                          Handle = NULL;
  EFI_EVENT                           EfiVariableArchProtocolRegisterEvent;
  VOID                                *Interface;
  VOID                                *Registration;

//[-start-171212-IB08400542-modify]//
  mEventLogFullOption = PcdGet8(PcdH2OEventLogFullOption);
//[-end-171212-IB08400542-modify]//

  Status = gBS->LocateProtocol (
                  &gEfiVariableWriteArchProtocolGuid,
                  NULL,
                  (VOID **) &Interface
                  );
  if (EFI_ERROR (Status)) {
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    EfiVariableWriteArchProtocolRegisterCallback,
                    NULL,
                    &EfiVariableArchProtocolRegisterEvent
                    );
    if (!EFI_ERROR (Status)) {
      Status = gBS->RegisterProtocolNotify (
                      &gEfiVariableWriteArchProtocolGuid,
                      EfiVariableArchProtocolRegisterEvent,
                      &Registration
                      );
    }
  } else {
    SetBiosStorageLogLib (0, TRUE);
  }
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gH2OBiosEventLogUseVariableProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mBiosEventLogUseVariableProtocl
                  );
  return Status;
}


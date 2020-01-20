/** @file

  Initial and callback functions for BIOS Event Log Config utility DXE

;******************************************************************************
;* Copyright (c) 2016 - 2018, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <BiosEventLogConfigUtil.h>

EVENT_LOG_PROTOCOL                        *mEventLog;
STORAGE_EVENT_LOG_INFO                    *mEventLogData;
UINTN                                     mEventLogNum = 0;
STORAGE_EVENT_LOG_INFO                    mFilteredEventLogData;
STORAGE_EVENT_LOG_INFO                    mBackupFilteredEventLogData;
UINTN                                     mFilterBitMap = 0;
UINTN                                     mBackupFilterBitMap = 0;
CHAR16                                    *mFilterString = NULL;
BOOLEAN                                   mCaseSensitive = FALSE;

CHAR16                                    mStorageName[MAX_STORAGE_NAME_NUM] = {0};
UINTN                                     mCurrentListStartIndex;
UINTN                                     mCurrentListEndIndex;
UINTN                                     mDaysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
EFI_HANDLE                                mDriverHandle[2] = {0};
BIOS_EVENT_LOG_CONFIG_UTIL_PRIVATE_DATA   *mPrivateData = NULL;
CHAR16                                    mVariableName[] = H2O_BIOS_EVENT_LOG_CONFIG_UTIL_VARSTORE_NAME;
CHAR16                                    mVariableNameForFilter[] = H2O_BIOS_EVENT_LOG_CONFIG_UTIL_FILTER_EFI_VARSTORE_NAME;
HII_VENDOR_DEVICE_PATH                    mHiiVendorDevicePath0 = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    H2O_BIOS_EVENT_LOG_CONFIG_UTIL_CONFIG_PAGE_FORMSET_GUID
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

HII_VENDOR_DEVICE_PATH                    mHiiVendorDevicePath1 = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    H2O_BIOS_EVENT_LOG_CONFIG_UTIL_EVENT_AND_MESSAGE_PAGE_FORMSET_GUID
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

//[-start-180213-IB08400569-add]//
EFI_STRING_ID                 mSaveEventStringId = 0;
EFI_STRING_ID                 mShowEventStringId[MAX_SHOW_ELV_LIST_NUM] = {0};
EFI_STRING_ID                 mShowEventHelpStringId[MAX_SHOW_ELV_LIST_NUM] = {0};
EFI_STRING_ID                 mTotalCountStringId = 0;
EFI_STRING_ID                 mCurrentCountStringId = 0;
//[-end-180213-IB08400569-add]//

/**
 Get the Event String.    

 Param[in]  EventInfo     The information of the event to translate string message.
 Param[out] EventString   String returned for the event.
 Param[out] StringSize    Size of EventString.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
GetEventString (
  IN     VOID                           *EventInfo,
  IN OUT CHAR16                         **EventString,
  IN OUT UINTN                          *StringSize
  )
{
  EFI_STATUS                              Status;
  UINTN                                   HandleCount;
  EFI_HANDLE                              *HandleBuffer;
  UINTN                                   Index;
  H2O_EVENT_LOG_STRING_PROTOCOL           *EventLogString;
  H2O_EVENT_LOG_STRING_OVERWRITE_PROTOCOL *EventLogStringOverwrite;

  Status = gBS->LocateProtocol (
                  &gH2OEventLogStringOverwriteProtocolGuid,
                  NULL,
                  (VOID **)&EventLogStringOverwrite
                  );
  if (!EFI_ERROR(Status)) {
    //
    // There exist RAS String Overwrite protocol. So, chech it firstly.
    //
    Status = EventLogStringOverwrite->EventLogStringOverwriteHandler (EventInfo, EventString, StringSize);
    if (!EFI_ERROR(Status)) {
      //
      // We Got it.
      //
      return EFI_SUCCESS;
    }
  }
  
  HandleCount  = 0;
  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gH2OEventLogStringProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    FreePool (HandleBuffer);
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    //
    // Double check which device connected on PCI(1D|0). Is keyboard??
    //
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gH2OEventLogStringProtocolGuid,
                    (VOID **)&EventLogString
                    );  
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = EventLogString->EventLogStringHandler (EventInfo, EventString, StringSize);

    if (!EFI_ERROR (Status)) {
      //
      // We Got It
      //
      return Status;
    }
  }

  FreePool (HandleBuffer);

  return EFI_NOT_FOUND;
}

/**
 Set the Event Storage type for filter function.    

 Param[in] VOID
 
 @retval EFI Status                  
*/
EFI_STATUS 
EFIAPI
SetStorageTypeForFilter (
  VOID
  ) 
{
  BIOS_EVENT_LOG_CONFIG_UTIL_FILTER_CONFIGURATION    Config = {0};
  BOOLEAN                                            GetBrowserData;
  BOOLEAN                                            SetBrowserData;

  GetBrowserData = HiiGetBrowserData (
                     &gH2OBiosEventLogConfigUtilVarstoreGuid, 
                     mVariableNameForFilter, 
                     sizeof(BIOS_EVENT_LOG_CONFIG_UTIL_FILTER_CONFIGURATION), 
                     (UINT8 *)&Config
                     );
  if (GetBrowserData == FALSE) {
    return EFI_UNSUPPORTED;
  }

  if (StrCmp(mStorageName, L"BIOS")==0) {
    Config.StorageType = EVENT_STORAGE_BIOS;
  } else if (StrCmp(mStorageName, L"MEMORY")==0) {
    Config.StorageType = EVENT_STORAGE_MEMORY; 
  } else if (StrCmp(mStorageName, L"BMC SEL")==0) {
    Config.StorageType = EVENT_STORAGE_BMC_SEL;
  } else {
    Config.StorageType = EVENT_STORAGE_DISABLED;
  }

  SetBrowserData = HiiSetBrowserData (
                     &gH2OBiosEventLogConfigUtilVarstoreGuid, 
                     mVariableNameForFilter, 
                     sizeof(BIOS_EVENT_LOG_CONFIG_UTIL_FILTER_CONFIGURATION), 
                     (UINT8 *)&Config, 
                     NULL
                     );
  if (SetBrowserData == TRUE) {
    return EFI_SUCCESS;
  } else {
    return EFI_UNSUPPORTED;
  }
}

CHAR16
EFIAPI  
ToUpper (
  CHAR16  a
  )
{
  if ('a' <= a && a <= 'z') {
    return (CHAR16) (a - 0x20);
  } else {
    return a;
  }
}

VOID
EFIAPI
StrUpr (
  IN CHAR16                           *Str
  )
{
  for(;*Str != 0; Str ++) {
    *Str = ToUpper(*Str);
  }
}

/**
 Check if the event data matches the filter condition.    

 Param[in]  EventData       Data of the Event.
 
 @retval    TRUE            Event matches the filter condition.
            FALSE           Event doesn't match the filter condition.
*/
BOOLEAN 
EFIAPI
DoesMatchFilterCondition (
  IN STORAGE_EVENT_LOG_INFO                *EventData
  ) 
{
  UINTN                                    LocalFilterBitMap=0;
  UINTN                                    StringSize;  
  CHAR16                                   *EventString;
  CHAR16                                   *TempString;
//[-start-170116-IB01780851-add]//
  EFI_STATUS                               Status;
//[-end-170116-IB01780851-add]//

  StringSize  = 0;
  EventString = NULL;
  TempString  = NULL;
  
  if (mFilterBitMap == 0) {
    //
    // No filter existence
    //
    return FALSE;
  }
  
  if ((mFilteredEventLogData.Year != 0xFFFF) &&
      (mFilteredEventLogData.Year == EventData->Year)) {
    LocalFilterBitMap |= B_FILTERED_BY_YEAR;
  }
  if ((mFilteredEventLogData.Month != 0xFF) &&
      (mFilteredEventLogData.Month == EventData->Month)) {
    LocalFilterBitMap |= B_FILTERED_BY_MONTH;
  }
  if ((mFilteredEventLogData.Date != 0xFF) &&
      (mFilteredEventLogData.Date == EventData->Date)) {   
    LocalFilterBitMap |= B_FILTERED_BY_DATE;
  }
  if ((mFilteredEventLogData.Hour != 0xFF) &&
      (mFilteredEventLogData.Hour == EventData->Hour)) {
    LocalFilterBitMap |= B_FILTERED_BY_HOUR;
  }
  if ((mFilteredEventLogData.Minute != 0xFF) &&
      (mFilteredEventLogData.Minute == EventData->Minute)) {
    LocalFilterBitMap |= B_FILTERED_BY_MINUTE;
  }
  if ((mFilteredEventLogData.Second != 0xFF) &&
      (mFilteredEventLogData.Second == EventData->Second)) {
    LocalFilterBitMap |= B_FILTERED_BY_SECOND;
  }
  if ((mFilteredEventLogData.GeneratorId != 0xFFFF) &&
      (mFilteredEventLogData.GeneratorId == EventData->GeneratorId)) {
    LocalFilterBitMap |= B_FILTERED_BY_GENERATOR_ID;
  }
  if ((mFilteredEventLogData.EventID.SensorType != 0xFF) &&
      (mFilteredEventLogData.EventID.SensorType == EventData->EventID.SensorType)) {
    LocalFilterBitMap |= B_FILTERED_BY_SENSOR_TYPE;
  }
  if ((mFilteredEventLogData.EventID.SensorNum != 0xFF) &&
      (mFilteredEventLogData.EventID.SensorNum == EventData->EventID.SensorNum)) {
    LocalFilterBitMap |= B_FILTERED_BY_SENSOR_NUM;
  }
  if ((mFilteredEventLogData.EventID.EventType != 0xFF) &&
      (mFilteredEventLogData.EventID.EventType == EventData->EventID.EventType)) {
    LocalFilterBitMap |= B_FILTERED_BY_EVENT_TYPE;
  }
  if ((mFilteredEventLogData.EventTypeID != 0xFF) &&
      (mFilteredEventLogData.EventTypeID == EventData->EventTypeID)) {
    LocalFilterBitMap |= B_FILTERED_BY_EVENT_TYPE_ID;
  }
  if (mFilterString != NULL) {     
//[-start-170116-IB01780851-modify]//
    Status = GetEventString ((VOID *)EventData, &EventString, &StringSize);
    if (EFI_ERROR (Status)) return FALSE;
//[-end-170116-IB01780851-modify]//
//[-start-160317-IB08400337-modify]//
    TempString = AllocateZeroPool (StrSize (mFilterString));
//[-end-160317-IB08400337-modify]//
    if (TempString == NULL) {
      FreePool (EventString);
      return FALSE;
    }
//[-start-180718-IB08400617-modify]//
    StrCpyS (TempString, StrSize (mFilterString) / sizeof(CHAR16) ,mFilterString);
//[-end-180718-IB08400617-modify]//
    
    if (!mCaseSensitive) {
      StrUpr (EventString);
      StrUpr (TempString);
    }
    
    if (StrStr (EventString, TempString) != NULL) {
      LocalFilterBitMap |= B_FILTERED_BY_STRING;
    }    
    FreePool (EventString);
    FreePool (TempString);
  }

  if (mFilterBitMap == LocalFilterBitMap) {
    //
    // Match all of filtered condition
    //
    return TRUE;
  }
  return FALSE;
}

/**
 Set the filter condition.    

 Param[in]  ResetFilter    TRUE: Reset the filter condition.
                           False: Set the filter condition.
 Param[in]  Data           Data of the Event.
 
 @retval    None
 
*/
VOID 
EFIAPI
ArrangeFilterCondition (
  IN  BOOLEAN                                       ResetFilter,
  IN  FILTER_EVENT_INFO                             *Data
  ) 
{
  BIOS_EVENT_LOG_CONFIG_UTIL_FILTER_CONFIGURATION    Config = {0};
  
  if (ResetFilter == TRUE) {   
    //
    // Make sue the event log data base has no filtered condition.
    //
    SetMem(&mFilteredEventLogData, sizeof(STORAGE_EVENT_LOG_INFO), 0xFF);
    SetMem(&mBackupFilteredEventLogData, sizeof(STORAGE_EVENT_LOG_INFO), 0xFF);
    mFilterBitMap = 0;
    mBackupFilterBitMap = 0;
    
    HiiSetBrowserData (
      &gH2OBiosEventLogConfigUtilVarstoreGuid, 
      mVariableNameForFilter, 
      sizeof(BIOS_EVENT_LOG_CONFIG_UTIL_FILTER_CONFIGURATION), 
      (UINT8 *)&Config, 
      NULL
      );

    SetStorageTypeForFilter ();
    return;
  }

  if (Data==NULL) {    
    return;
  }

  if (Data->Year != 0xFFFF) {
    mFilteredEventLogData.Year = Data->Year;
    mFilterBitMap |= B_FILTERED_BY_YEAR;
  }
  if (Data->Month!= 0xFF) {
    mFilteredEventLogData.Month = Data->Month;
    mFilterBitMap |= B_FILTERED_BY_MONTH;
  }
  if (Data->Date != 0xFF) {
    mFilteredEventLogData.Date = Data->Date;    
    mFilterBitMap |= B_FILTERED_BY_DATE;
  }
  if (Data->Hour != 0xFF) {
    mFilteredEventLogData.Hour = Data->Hour;
    mFilterBitMap |= B_FILTERED_BY_HOUR;
  }
  if (Data->Minute != 0xFF) {
    mFilteredEventLogData.Minute = Data->Minute;
    mFilterBitMap |= B_FILTERED_BY_MINUTE;
  }
  if (Data->Second != 0xFF) {
    mFilteredEventLogData.Second = Data->Second;
    mFilterBitMap |= B_FILTERED_BY_SECOND;
  }
  if (Data->GeneratorId != 0xFFFF) {
    mFilteredEventLogData.GeneratorId = Data->GeneratorId;
    mFilterBitMap |= B_FILTERED_BY_GENERATOR_ID;
  }
  if (Data->EventID.SensorType != 0xFF) {
    mFilteredEventLogData.EventID.SensorType = Data->EventID.SensorType;
    mFilterBitMap |= B_FILTERED_BY_SENSOR_TYPE;
  }
  if (Data->EventID.SensorNum != 0xFF) {
    mFilteredEventLogData.EventID.SensorNum = Data->EventID.SensorNum;
    mFilterBitMap |= B_FILTERED_BY_SENSOR_NUM;
  }
  if (Data->EventID.EventType != 0xFF) {
    mFilteredEventLogData.EventID.EventType = Data->EventID.EventType;
    mFilterBitMap |= B_FILTERED_BY_EVENT_TYPE;
  }
  if (Data->EventTypeID != 0xFF) {
    mFilteredEventLogData.EventTypeID = Data->EventTypeID;
    mFilterBitMap |= B_FILTERED_BY_EVENT_TYPE_ID;
  }
  if (mFilterString != NULL) {
    mFilterBitMap |= B_FILTERED_BY_STRING;
  }
  
  return;
}

/**
 Get the Event Log data base on the filter condition.    

 Param[in]  VOID
 
 @retval    None
 
*/
EFI_STATUS 
EFIAPI
GetEventLogDataBaseByFilterCondition (
  VOID
  ) 
{
  EFI_STATUS                            Status;
  UINTN                                 LocalEventCount;
  UINTN                                 Index;
  STORAGE_EVENT_LOG_INFO                *EventData;
  UINTN                                 MatchFiterCount=0;
  UINTN                                 FilterIndex=0;

  mEventLogNum = 0;
  mCurrentListStartIndex = 0;
  mCurrentListEndIndex = 0;
  
  Status = mEventLog->RefreshDatabase (mStorageName, &LocalEventCount);
  if (EFI_ERROR (Status)) {
    //
    // If returned status is not EFI_SUCCESS, show the error messsage.
    //
    DEBUG ((EFI_D_ERROR, "Get %s storage data fail: %r\n", mStorageName, Status));
    return Status;
  }

  //
  // If there are no logs exist, return to main menu
  //
  if (LocalEventCount == 0) {
    DEBUG ((EFI_D_ERROR, "%s storage has no event.\n", mStorageName));
    return Status;
  }

  //
  // Count event which are matched filter condition
  //
  if (mFilterBitMap != 0) {
    //
    // Filter event data
    //
    for (Index = 0; Index < LocalEventCount; Index++) {
      Status = mEventLog->GetEvent (mStorageName, Index, (UINT8 **)&EventData);
      if (EFI_ERROR (Status)) {
        break;
      }
      if (DoesMatchFilterCondition(EventData)) {
        MatchFiterCount++;
      }
    }
  } else {
    //
    // Don't filter any event data. Show all event data
    //
    MatchFiterCount = LocalEventCount;
  }
  if (MatchFiterCount != 0) {
    if (mEventLogData != NULL) {
      //
      // Free old Event Log Data base for to create new filtered Event Log Data base
      //
      FreePool (mEventLogData);
    }
    //
    // Reset event log data base by filter condition
    //
    Status = gBS->AllocatePool (
                    EfiBootServicesData, 
                    sizeof (STORAGE_EVENT_LOG_INFO) * MatchFiterCount, 
                    (VOID **)&mEventLogData
                    );
    ASSERT_EFI_ERROR (Status);
    ZeroMem (mEventLogData, sizeof (STORAGE_EVENT_LOG_INFO) * MatchFiterCount);

    for (Index = 0; Index < LocalEventCount; Index++) {
      Status = mEventLog->GetEvent (mStorageName, Index, (UINT8 **)&EventData);
      if (EFI_ERROR (Status)) {
        break;
      }
      if (DoesMatchFilterCondition (EventData) && (mFilterBitMap != 0)) {
        CopyMem (&mEventLogData[MatchFiterCount - FilterIndex - 1], EventData, sizeof (STORAGE_EVENT_LOG_INFO));
        FilterIndex++;
      } else if (mFilterBitMap == 0) { 
        CopyMem (&mEventLogData[MatchFiterCount - FilterIndex - 1], EventData, sizeof (STORAGE_EVENT_LOG_INFO));
        FilterIndex++;
      }
    }
    mEventLogNum = MatchFiterCount;
    return EFI_SUCCESS;
  } else {
    //
    // Not found matched event. Don't do anthing.
    //
    DEBUG ((EFI_D_ERROR, "%s storage has no any matched event\n", mStorageName));
  }
  
  return EFI_NOT_FOUND;
  
}

EFI_STATUS
EFIAPI
CheckDecember (
  IN      CHAR16                                        *String,
  OUT     UINT32                                        *Number
  ) 
{
  CHAR16                                                *Temp = String;
  
  if (String == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  // skip preceeding white space
  while (*Temp && *Temp == L' ') {
    Temp += 1;
  }

  // skip preceeding zeros
  while (*Temp && *Temp == L'0') {
    Temp += 1;
  }
  
  *Number = 0;
  while (*Temp) {
    if ( (*Temp >= L'0'  &&  *Temp <= L'9')) {
      *Number = *Number * 10 + *Temp -  L'0';
    } else {
      return EFI_INVALID_PARAMETER;
    }
    Temp += 1;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CheckHex (
  IN      CHAR16                                        *String,
  OUT     UINT32                                        *Number
  ) 
{
  CHAR16                                                *Temp = String;
  CHAR16                                                Char16 = 0;
  
  // skip preceeding white space
  while (*Temp && *Temp == L' ') {
    Temp += 1;
  }

  if (StrLen (Temp) == 0) {
    return EFI_INVALID_PARAMETER;
  }

  // skip preceeding zeros
  while (*Temp && *Temp == L'0') {
    Temp += 1;
  }
  
  *Number = 0;
  Char16 = * (String++);
  while (Char16) {
    if (Char16 >= L'a'  &&  Char16 <= L'f') {
      Char16 -= L'a' - L'A';
    }

    if ( (Char16 >= L'0'  &&  Char16 <= L'9')  || (Char16 >= L'A'  &&  Char16 <= L'F')) {
      *Number = *Number << 4  |  Char16 - (Char16 >= L'A' ? L'A' - 10 : L'0');
    } else {
      return EFI_INVALID_PARAMETER;
    }
   
    Char16 = * (String++);
  }
  return EFI_SUCCESS;
}

EFI_STATUS 
EFIAPI
TransferStringToDate (
  IN      CHAR16                                        *String,
  OUT     FILTER_EVENT_INFO                             *Data
  ) 
{
  EFI_STATUS                                            Status = EFI_SUCCESS;
  CHAR16                                                *TempStr = NULL;
  UINT32                                                Index = 0;
  UINT32                                                NumberIndex = 0;
  UINT32                                                ArgIndex = 0;
  UINT32                                                Number = 0;
  UINTN                                                 TempStrLen;
  
  if ((String == NULL) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  TempStr = AllocateZeroPool ((StrLen (String) + 2) * sizeof (CHAR16));
  if (TempStr == NULL) {
    return EFI_UNSUPPORTED;
  }

//[-start-180718-IB08400617-modify]//  
  StrCpyS (TempStr, StrLen (String) + 2,String);
//[-end-180718-IB08400617-modify]//

  TempStrLen = StrLen(TempStr); 
  TempStr [TempStrLen] = L'/';
  TempStr [TempStrLen + 1] = L'\0';
  
  for (Index = 0, NumberIndex = 0, ArgIndex = 1; TempStr[Index]; Index++) {
    if (TempStr[Index] == L'/') {
      TempStr[Index] = L'\0';
      Status = CheckDecember (&TempStr[NumberIndex], &Number);
      if (Status != EFI_SUCCESS) {
        Status = EFI_INVALID_PARAMETER;
        goto TransferStringToDateExit;
      }
      
      switch (ArgIndex) {

      case 1: // year
        if (Number > 0xffff) {
          Status = EFI_INVALID_PARAMETER;
          goto TransferStringToDateExit;
        } else {
          Data->Year = (UINT16)Number;
        }
        break;

      case 2: // month
        if (Number > 12) {
          Status = EFI_INVALID_PARAMETER;
          goto TransferStringToDateExit;
        } else {
          Data->Month = (UINT8)Number;
        }
        break;

      case 3: // date
        if (Number > 31) {
          Status = EFI_INVALID_PARAMETER;
          goto TransferStringToDateExit;
        } else {
          Data->Date = (UINT8)Number;
        }
        break;

      default:
        Status = EFI_INVALID_PARAMETER;
        goto TransferStringToDateExit;
        break;
      }
      
      NumberIndex = Index + 1;
      ArgIndex++;
    }
  }

  if (ArgIndex != 4) {
    Status = EFI_INVALID_PARAMETER;
  } else {
    Status = EFI_SUCCESS;
  } 

TransferStringToDateExit:
  FreePool (TempStr);
  return Status;
}

EFI_STATUS 
EFIAPI
TransferStringToTime (
  IN      CHAR16                                        *String,
  OUT     FILTER_EVENT_INFO                             *Data
  ) 
{
  EFI_STATUS                                            Status = EFI_SUCCESS;
  CHAR16                                                *TempStr = NULL;
  UINT32                                                Index = 0;
  UINT32                                                NumberIndex = 0;
  UINT32                                                ArgIndex = 0;
  UINT32                                                Number = 0;
  
  if ((String == NULL) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  TempStr = AllocateZeroPool ((StrLen (String) + 2) * sizeof (CHAR16));
  if (TempStr == NULL) {
    return EFI_UNSUPPORTED;
  }

//[-start-180718-IB08400617-modify]//
  StrCpyS (TempStr, StrLen (String) + 2, String);
//[-end-180718-IB08400617-modify]//

  TempStr [StrLen(TempStr)] = L':';
  TempStr [StrLen(TempStr) + 1] = L'\0';
  
  for (Index = 0, NumberIndex = 0, ArgIndex = 1; TempStr[Index]; Index++) {
    if (TempStr[Index] == L':') {
      TempStr[Index] = L'\0';
      Status = CheckDecember (&TempStr[NumberIndex], &Number);
      if (Status != EFI_SUCCESS) {
        Status = EFI_INVALID_PARAMETER;
        goto TransferStringToTimeExit;
      }
      
      switch (ArgIndex) {

      case 1: // Hour
        if (Number > 24) {
          Status = EFI_INVALID_PARAMETER;
          goto TransferStringToTimeExit;
        } else {
          Data->Hour = (UINT8)Number;
        }
        break;

      case 2: // minute
        if (Number > 60) {
          Status = EFI_INVALID_PARAMETER;
          goto TransferStringToTimeExit;
        } else {
          Data->Minute = (UINT8)Number;
        }
        break;

      case 3: // second
        if (Number > 60) {
          Status = EFI_INVALID_PARAMETER;
          goto TransferStringToTimeExit;
        } else {
          Data->Second = (UINT8)Number;
        }
        break;

      default:
        Status = EFI_INVALID_PARAMETER;
        goto TransferStringToTimeExit;
        break;
      }
      
      NumberIndex = Index + 1;
      ArgIndex++;
    }
  }

  if (ArgIndex != 4) {
    Status = EFI_INVALID_PARAMETER;
  } else {
    Status = EFI_SUCCESS;
  } 

TransferStringToTimeExit:
  FreePool (TempStr);
  return Status;
}

EFI_STATUS 
EFIAPI
TransferStringToNumber (
  IN      CHAR16                                        *String,
  OUT     UINT32                                        *NumberOut
  ) 
{
  EFI_STATUS                                            Status = EFI_SUCCESS;
  UINT32                                                Number = 0;
  CHAR16                                                *Temp = NULL;
  
  if (String == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Temp = String;
  while (*Temp && *Temp == L' ') {
    Temp += 1;
  }
  
  if (StrLen (Temp) == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*Temp == L'0') && (*(Temp + 1) == L'x')) {
    Status = CheckHex ((Temp + 2), &Number);
  } else {
    Status = CheckDecember (Temp, &Number);
  }

  if (Status != EFI_SUCCESS) {
    return EFI_INVALID_PARAMETER;
  }

  *NumberOut = Number;

  return EFI_SUCCESS;
}

EFI_STATUS 
EFIAPI
TransferStringToEventID (
  IN      CHAR16                                        *String,
  OUT     FILTER_EVENT_INFO                             *Data
  ) 
{
  EFI_STATUS                                            Status = EFI_SUCCESS;
  CHAR16                                                *TempStr = NULL;
  UINT32                                                Index = 0;
  UINT32                                                NumberIndex = 0;
  UINT32                                                ArgIndex = 0;
  UINT32                                                Number = 0;
  
  if ((String == NULL) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  TempStr = AllocateZeroPool ((StrLen (String) + 2) * sizeof (CHAR16));
  if (TempStr == NULL) {
    return EFI_UNSUPPORTED;
  }
//[-start-180718-IB08400617-modify]//
  StrCpyS (TempStr, (StrLen (String) + 2),String);
//[-end-180718-IB08400617-modify]//

  TempStr [StrLen(TempStr)] = L' ';
  TempStr [StrLen(TempStr) + 1] = L'\0';
  
  for (Index = 0, NumberIndex = 0, ArgIndex = 1; TempStr[Index]; Index++) {

    //
    // skip preceeding white space, and Get Number start Index
    //
    while ((TempStr[Index]) && (TempStr[Index] == L' ')) {
      Index += 1;
    }
    NumberIndex = Index;

    //
    // Get Number End Index
    //
    while ((TempStr[Index]) && (TempStr[Index] != L' ')) {
      Index += 1;
    }

    if ((TempStr[Index] == L'\0') || (NumberIndex >= Index)) {
      break;
    }
    
    TempStr[Index] = L'\0';
    Status = TransferStringToNumber (&TempStr[NumberIndex], &Number);
    if ((Status != EFI_SUCCESS) || (Number > 0xff)) {
      Status = EFI_INVALID_PARAMETER;
      goto TransferStringToEventIDExit;
    }
    
    switch (ArgIndex) {

    case 1: // SensorType
      Data->EventID.SensorType = (UINT8)Number;
      break;

    case 2: // SensorNum
      Data->EventID.SensorNum = (UINT8)Number;
      break;

    case 3: // EventType
      Data->EventID.EventType = (UINT8)Number;
      break;

    default:
      Status = EFI_INVALID_PARAMETER;
      goto TransferStringToEventIDExit;
      break;
    }
    
//[-start-161109-IB04560706-remove]//
//    NumberIndex = Index + 1;
//[-end-161109-IB04560706-remove]//
    ArgIndex++;
    
  }

  if (ArgIndex != 4) {
    Status = EFI_INVALID_PARAMETER;
  } else {
    Status = EFI_SUCCESS;
  } 

TransferStringToEventIDExit:
  FreePool (TempStr);
  return Status;
}

EFI_STATUS 
EFIAPI
TransferStringToData (
  IN      EFI_HII_HANDLE      HiiHandle,
  IN      EFI_IFR_TYPE_VALUE  *Value,
  IN OUT  FILTER_EVENT_INFO   *Data
  ) 
{
  EFI_STATUS                                            Status = EFI_SUCCESS;
  UINT32                                                Number = 0;
  CHAR16                                                *FilterStr = NULL;
  
  FilterStr = HiiGetString (HiiHandle, Value->string, NULL);
  if (FilterStr == NULL) {
    DEBUG ((EFI_D_ERROR, "Can't get BIOS Event Log Filter condition string\n"));
    DEBUG ((EFI_D_ERROR, "Value->string: 0x%x\n", Value->string));
    return EFI_NOT_FOUND;
  }

  if (StrLen (FilterStr) == 0) {
    // Means that the user clear the condition.
    FreePool (FilterStr);
    return EFI_SUCCESS;
  }

  switch (Data->FilterType) {

  case TypeDate:
    DEBUG ((EFI_D_INFO, "FilterType: TypeDate\n"));
    DEBUG ((EFI_D_INFO, "FilterStr: %s\n", FilterStr));
    Status = TransferStringToDate (FilterStr, Data);
    break;

  case TypeTime:
    DEBUG ((EFI_D_INFO, "FilterType: TypeTime\n"));
    Status = TransferStringToTime (FilterStr, Data);
    break;

  case TypeGeneratorID:
    DEBUG ((EFI_D_INFO, "FilterType: TypeGeneratorID\n"));
    Status = TransferStringToNumber (FilterStr, &Number);
    if ((Status == EFI_SUCCESS) && (Number <= 0xffff)) {
      Data->GeneratorId = (UINT16)Number;
    } else {
      Status = EFI_INVALID_PARAMETER;
    }
    break;

  case TypeEventID:
    DEBUG ((EFI_D_INFO, "FilterType: TypeEventID\n"));
    Status = TransferStringToEventID (FilterStr, Data);
    break;

  case TypeSensorType:
    DEBUG ((EFI_D_INFO, "FilterType: TypeSensorType\n"));
    Status = TransferStringToNumber (FilterStr, &Number);
    if ((Status == EFI_SUCCESS) && (Number <= 0xff)) {
      Data->EventID.SensorType = (UINT8)Number;
    } else {
      Status = EFI_INVALID_PARAMETER;
    }
    break;

  case TypeSensorNum:
    DEBUG ((EFI_D_INFO, "FilterType: TypeSensorNum\n"));
    Status = TransferStringToNumber (FilterStr, &Number);
    if ((Status == EFI_SUCCESS) && (Number <= 0xff)) {
      Data->EventID.SensorNum = (UINT8)Number;
    } else {
      Status = EFI_INVALID_PARAMETER;
    }
    break;

  case TypeEventType:
    DEBUG ((EFI_D_INFO, "FilterType: TypeEventType\n"));
    Status = TransferStringToNumber (FilterStr, &Number);
    if ((Status == EFI_SUCCESS) && (Number <= 0xff)) {
      Data->EventID.EventType = (UINT8)Number;
    } else {
      Status = EFI_INVALID_PARAMETER;
    }
    break;

  case TypeEventTypeID:
    DEBUG ((EFI_D_INFO, "FilterType: TypeEventTypeID\n"));
    Status = TransferStringToNumber (FilterStr, &Number);
    if ((Status == EFI_SUCCESS) && (Number <= 0xff)) {
      Data->EventTypeID = (UINT8)Number;
    } else {
      Status = EFI_INVALID_PARAMETER;
    }
    break;    

  case TypeString:
    DEBUG ((EFI_D_INFO, "FilterType: TypeString\n"));
//[-start-160315-IB08400337-modify]//
    mFilterString = AllocateZeroPool (StrSize (FilterStr));
//[-end-160315-IB08400337-modify]//    
    if (mFilterString == NULL) {
      return EFI_INVALID_PARAMETER;
    }
//[-start-180718-IB08400617-modify]//
    StrCpyS (mFilterString, StrSize (FilterStr) / sizeof(CHAR16),FilterStr);
//[-end-180718-IB08400617-modify]//
    break;

  default:
    Status = EFI_INVALID_PARAMETER;
    break;
  }

  FreePool (FilterStr);
  return Status;
}

/**
 Search first "Simple File System" protocol.    

 Param[in]  Removable   Is device of the "Simple File System" removable.
 
 @retval    Return find out "Simple File System" protoco, 
            Return NULL if not found                 
*/
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *
EFIAPI
SearchFileSystem (
  BOOLEAN Removable
  )
{
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL    *Volume;
  EFI_STATUS                          Status;
  EFI_HANDLE                         *HandleBuffer;
  UINTN                               NumberOfHandles;
  UINTN                               Index;
  EFI_BLOCK_IO_PROTOCOL              *BlkIo;


  Volume       = NULL;
  HandleBuffer = NULL;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  for (Index = 0; Index < NumberOfHandles; Index++) {

    BlkIo  = NULL;
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiBlockIoProtocolGuid, (VOID **)&BlkIo);

    if (BlkIo != NULL) {
      if (BlkIo->Media->ReadOnly) {
        continue;
      }
      if (Removable && !BlkIo->Media->RemovableMedia) {
        continue;
      }
    }
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiSimpleFileSystemProtocolGuid, (VOID **)&Volume);
    if (!EFI_ERROR (Status)) {
      break;
    }
  }

  if (HandleBuffer != NULL) {
    FreePool (HandleBuffer);
  }

  return Volume;
}

VOID
EFIAPI
ValueToString (
  UINTN     Value,
  UINTN     Digitals,
  CHAR16    *Buffer,
  UINTN     Flags
  )
{
  CHAR8   Str[30];
  UINTN   mod;
  CHAR8   *p1;
  CHAR16  *p2;
  CHAR16  StuffChar = 0;
  UINTN   count = 0;
  UINTN   Padding;

  p1  = Str;
  p2  = Buffer;

  if (Flags == VTS_RIGHT_ALIGN)
    StuffChar = L' ';

  if (Flags == VTS_LEAD_0)
    StuffChar = L'0';

  if (Value == 0) {

    // Stuff specify char to buffer
    if (StuffChar != 0) {
      Padding = Digitals - 1;
      
      while (Padding > 0) {
        *p2 = StuffChar;
        p2++;
        Padding--;      
      }
    }
    
    *p2++ = '0';
    *p2 = 0;
    return ;
  }
  
  while (Value) {
    mod = Value % 10;
    *p1 = (CHAR8)(mod + '0');
    Value = Value / 10;
    p1++;
    count++;
  }

  if ((count < Digitals) && (StuffChar != 0)) {
    Padding = Digitals - count;
    while (Padding) {
      *p2 = StuffChar;
      p2++;
      Padding--;
    }
  }

  while (count > 0) {
    p1--;
    *p2 = *p1;
    p2++;

    count--;
  }

  *p2 = 0;
}

EFI_STATUS
EFIAPI
ObtainSavingFileNameByTime (
  IN CHAR16                                 *StorageName,
  IN OUT CHAR16                             **FileName
  )
{
  EFI_STATUS                           Status;
  EFI_TIME                             EfiTime;
  CHAR16                               Buffer[5];
  UINTN                                FileNameSize=0;
  VOID                                 *TempString=NULL;

  //
  // File name will be defined as "YYYYMMDDHHMMSS"+"storage name"+".log"
  // ex: file name = 20091111171800+StorageName+.log
  //
//[-start-160315-IB08400337-modify]//
  FileNameSize = (StrLen(StorageName) + StrLen(L"20091111171800.log") + 1) * sizeof(CHAR16) ;
//[-end-160315-IB08400337-modify]//  

  Status = gBS->AllocatePool (EfiBootServicesData, FileNameSize, (VOID **)&TempString);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = gRT->GetTime (&EfiTime, NULL);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  ValueToString (EfiTime.Year, 4, Buffer, VTS_LEAD_0);
//[-start-180718-IB08400617-modify]//
  StrCpyS (TempString, 5, Buffer);
  
  ValueToString (EfiTime.Month, 2, Buffer, VTS_LEAD_0);
  StrCatS (TempString, 5, Buffer);

  ValueToString (EfiTime.Day, 2, Buffer, VTS_LEAD_0);
  StrCatS (TempString, 5, Buffer);

  ValueToString (EfiTime.Hour, 2, Buffer, VTS_LEAD_0);
  StrCatS (TempString, 5, Buffer);

  ValueToString (EfiTime.Minute, 2, Buffer, VTS_LEAD_0);
  StrCatS (TempString, 5, Buffer);

  ValueToString (EfiTime.Second, 2, Buffer, VTS_LEAD_0);
  StrCatS (TempString, 5, Buffer);

  StrCatS (TempString, MAX_STORAGE_NAME_NUM, StorageName);

  StrCatS (TempString, 5, L".log");
//[-end-180718-IB08400617-modify]//

  *FileName = TempString;

  return EFI_SUCCESS;
}

/**
 This function verifies the leap year

 @param[in]         Year  - year in YYYY format                
 
 @retval BOOLEAN    TRUE  - The year is a leap year
                    FALSE - The year is not a leap year              
*/
BOOLEAN
EFIAPI
IsLeapYear (
  IN UINT16   Year
  )
{
  if (Year % 4 == 0) {
    if (Year % 100 == 0) {
      if (Year % 400 == 0) {
        return TRUE;
      } else {
        return FALSE;
      }
    } else {
      return TRUE;
    }
  } else {
    return FALSE;
  }
}


/**
 Count the number of the leap years between 1970 and CurYear

 @param[in]         CurYear The Current year

 @retval UINTN      Count   The count of leapyears            
*/
UINTN
EFIAPI
CountNumOfLeapYears (
  IN UINT16           CurYear
  )
{
  UINT16  NumOfYear;
  UINT16  BaseYear;
  UINT16  Index;
  UINTN   Count;
  Count     = 0;
  BaseYear  = 1970;
  NumOfYear = (UINT16) (CurYear - 1970);
  for (Index = 0; Index <= NumOfYear; Index++) {
    if (IsLeapYear ((UINT16) (BaseYear + Index))) {
      Count += 1;
    }
  }

  return Count;
}

/**
 Count time stamp to Efi time.

 @param[in]         TimeStamp                
 @param[in]         EfiTime                          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
Ts2et (
  IN UINT32             TimeStamp,
  IN EFI_TIME           *EfiTime
  )
{
  UINT16                NumOfYears;
  UINTN                 NumOfLeapYears;
  UINTN                 NumOfDays;
  UINTN                 DayOfYear;
  UINT16                Years;
  UINT32                Mouths;

  // Caculate second;
  EfiTime->Second = (UINT8) (TimeStamp % 60);

  // Caculate minitunes;
  TimeStamp /= 60;
  EfiTime->Minute = (UINT8) (TimeStamp % 60);

  // Caculate Hour;
  TimeStamp /= 60;
  EfiTime->Hour = (UINT8) (TimeStamp % 24);

  NumOfDays = TimeStamp / 24;

  // caculate Year
  NumOfYears = (UINT16) (NumOfDays / 365);
  Years = 1970 + NumOfYears;

  if (Years != 1970) {
    NumOfLeapYears = CountNumOfLeapYears (Years-1);
  } else {
    NumOfLeapYears = 0;
  }

  NumOfDays -= NumOfLeapYears;

  // Year = 1970 + NumOfYear
  EfiTime->Year = (UINT16) (NumOfDays / 365 + 1970);

  DayOfYear = NumOfDays % 365 + 1;

  if (IsLeapYear(EfiTime->Year)) {
    mDaysOfMonth[1] = 29;
  }
  else {
    mDaysOfMonth[1] = 28;
  }

  for (Mouths = 0; DayOfYear > mDaysOfMonth[Mouths];  Mouths++) {
      DayOfYear -= mDaysOfMonth[Mouths];
  }

  EfiTime->Month = (UINT8) Mouths + 1;
  EfiTime->Day = (UINT8) DayOfYear;

  return EFI_SUCCESS;
}

/**
 Display Dialog for User Requirement which dilag type is Yes/No, Yes/No/Cancel, Ok or Ok/Cancel

 @param[in]  DialogOperation           Dialog type (Yes/No, Yes/No/Cancel, Ok or Ok/Cancel)
 @param[in]  HotKey                    HotKey information
 @param[in]  MaximumStringSize         Maximum string length
 @param[out] StringBuffer              String buffer
 @param[out] KeyValue                  Ptr to returned structure that indicates the key the user selected.
 @param[in]  String                    Ptr to null-terminated string that specifies the dialog prompt

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
ElvConfirmDialog (
  IN  UINT32                           DialogOperation,
  IN  BOOLEAN                          HotKey,
  IN  UINT32                           MaximumStringSize,
  OUT CHAR16                           *StringBuffer,
  OUT EFI_INPUT_KEY                    *KeyValue,
  IN  CHAR16                           *String
  )
{
  EFI_STATUS                                 Status;
  H2O_DIALOG_PROTOCOL                        *H2ODialog;
  
  Status = gBS->LocateProtocol (
                  &gH2ODialogProtocolGuid,
                  NULL,
                  (VOID **)&H2ODialog
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  return H2ODialog->ConfirmDialog (
                      DialogOperation,
                      HotKey,
                      MaximumStringSize,
                      StringBuffer,
                      KeyValue,
                      String
                      );
  
}

/**
 Return the detailed information of the input event index.

 @param[in]  EventIndex           The Index for the event to get the detailed information.
 @param[out] RetStr               Detailed information of the event.

 @retval EFI_SUCCESS              Process successfully.
**/
EFI_STATUS
EFIAPI
GetDetailedEventInfo(
  IN  UINTN                               EventIndex,
  OUT CHAR16                              *RetStr
  )
{
  CHAR16                        StringPtr[MAX_ELV_STR_NUM];
  UINTN                         Index;
  BOOLEAN                       HaveEvent = FALSE;
  UINTN                         EventStringSize;  
  CHAR16                        *EventString;
  
  UnicodeSPrint (
    StringPtr, 
    sizeof (StringPtr), 
    L"Entry Num: %04d\n",
    (EventIndex + 1)
    );
//[-start-180718-IB08400617-modify]//
//[-start-180824-IB07400999-modify]//
  StrCpyS(RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
  ZeroMem (StringPtr, sizeof(StringPtr));
  
  if (mEventLogData[EventIndex].EventTypeIdEnable) {
    UnicodeSPrint (
      StringPtr, 
      sizeof (StringPtr), 
      L"Event Type ID: 0x%02x\n",
      mEventLogData[EventIndex].EventTypeID
      );
    StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
    ZeroMem (StringPtr, sizeof(StringPtr));
  }
    
  if (mEventLogData[EventIndex].GeneratorIdEnable) {    
    UnicodeSPrint (
      StringPtr, 
      sizeof (StringPtr), 
      L"Generator ID: 0x%04x\n",
      mEventLogData[EventIndex].GeneratorId
      );
    StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
    ZeroMem (StringPtr, sizeof(StringPtr));
  }

  if (mEventLogData[EventIndex].EventIdEnable) {    
    if ((mEventLogData[EventIndex].Data == NULL) && ((mEventLogData[EventIndex].EventID.SensorType == 0xFF) && (mEventLogData[EventIndex].EventID.SensorNum == 0xFF) && (mEventLogData[EventIndex].EventID.EventType == 0xFF))) {
      UnicodeSPrint (
        StringPtr, 
        sizeof (StringPtr), 
        L"Event ID: NULL\n"      
        );
      StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
      ZeroMem (StringPtr, sizeof(StringPtr));
    } else {
      UnicodeSPrint (
        StringPtr, 
        sizeof (StringPtr), 
        L"Event ID: 0x%02x 0x%02x 0x%02x\n",
        mEventLogData[EventIndex].EventID.SensorType,
        mEventLogData[EventIndex].EventID.SensorNum,
        mEventLogData[EventIndex].EventID.EventType
        );
      StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
      ZeroMem (StringPtr, sizeof(StringPtr));
    }
  } else {
    UnicodeSPrint (
      StringPtr, 
      sizeof (StringPtr), 
      L"Sensor Type: 0x%02x\n",
      mEventLogData[EventIndex].EventID.SensorType
      );
    StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
    ZeroMem (StringPtr, sizeof(StringPtr));

    UnicodeSPrint (
      StringPtr, 
      sizeof (StringPtr), 
      L"Sensor Num: 0x%02x\n",
      mEventLogData[EventIndex].EventID.SensorNum
      );
    StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
    ZeroMem (StringPtr, sizeof(StringPtr));

    UnicodeSPrint (
      StringPtr, 
      sizeof (StringPtr), 
      L"Event Type: 0x%02x\n",
      mEventLogData[EventIndex].EventID.EventType
      );
    StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
    ZeroMem (StringPtr, sizeof(StringPtr));
    
  }

  if (mEventLogData[EventIndex].Data == NULL) {
    UnicodeSPrint (
    StringPtr, 
    sizeof (StringPtr), 
    L"Event Data: NULL\n"
    );
  } else {
    UnicodeSPrint (
    StringPtr, 
    sizeof (StringPtr), 
    L"Event Data: 0x%02x 0x%02x 0x%02x\n",
    mEventLogData[EventIndex].Data[0],
    mEventLogData[EventIndex].Data[1],
    mEventLogData[EventIndex].Data[2]
    );
  }
  StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
  ZeroMem (StringPtr, sizeof(StringPtr));
    
  UnicodeSPrint (
    StringPtr, 
    sizeof (StringPtr), 
    L"Event Raw Data: "
    );
  StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
  ZeroMem (StringPtr, sizeof(StringPtr));
  
  for (Index=0; Index < (mEventLogData[EventIndex].EventRawDataSize); Index++) {   
    HaveEvent = TRUE;
    UnicodeSPrint (
      StringPtr, 
      sizeof (StringPtr), 
      L"0x%02x ",
      mEventLogData[EventIndex].EventRawData[Index]
      );
    StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
    ZeroMem (StringPtr, sizeof(StringPtr));
  }  

  if (!HaveEvent) {
    UnicodeSPrint (
      StringPtr, 
      sizeof (StringPtr), 
      L"NULL\n"
      );
    StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
    ZeroMem (StringPtr, sizeof(StringPtr));
  } else {
    StrCatS (RetStr, MAX_ELV_STR_NUM * 2, L"\n");
  }

  if (!GetEventString ((VOID *)&(mEventLogData[EventIndex]), &EventString, &EventStringSize)) {
    UnicodeSPrint (
      StringPtr, 
      sizeof (StringPtr), 
      L"Event String: %s.\n",
      EventString
      );
    StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
    ZeroMem (StringPtr, sizeof(StringPtr));
    FreePool (EventString);
  } else {
    UnicodeSPrint (
      StringPtr, 
      sizeof (StringPtr), 
      L"Event String: Undefined.\n"
      );
    StrCatS (RetStr, MAX_ELV_STR_NUM * 2, StringPtr);
    ZeroMem (StringPtr, sizeof(StringPtr));
  }
//[-end-180824-IB07400999-modify]//
//[-end-180718-IB08400617-modify]//
  
  return EFI_SUCCESS;
  
}

/**
 Update the items for the Event Storage page in the Event Log Viewer.

 @param[in]  HiiHandle                 EFI HII handle.
 @param[in]  ShowAction                Determine the show policy when update the item of Event Storage page.    

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
UpdateElvStoragePage (
  IN  EFI_HII_HANDLE      HiiHandle,
  IN  UINT8               ShowAction
  )
{
//[-start-180214-IB08400569-modify]//
  VOID                          *StartOpCodeHandle = NULL;
  VOID                          *EndOpCodeHandle = NULL;
  VOID                          *SaveEventOpCodeHandle = NULL;
  EFI_IFR_GUID_LABEL            *StartLabel;
  EFI_IFR_GUID_LABEL            *EndLabel;
  EFI_STATUS                    Status;
  CHAR16                        StringPtr[MAX_ELV_STR_NUM] = {0};
  CHAR16                        StringPtr2[MAX_ELV_STR_NUM * 2] = {0};
  UINTN                         EventCount;
  UINTN                         EventStringIndex = 0;
  UINT8                         QuestionFlags;
  UINTN                         Index;
  CHAR16                        *EventString;
  UINTN                         EventStringSize;
  CHAR16                        ShowStringPtr[80] = {0};
  EFI_STRING_ID                 TempStringId;
//[-end-180214-IB08400569-modify]//

  if (ShowAction == SHOW_FIRST_PAGE) {
    EventCount = 0;
    
    // Get the all event data.
    Status = GetEventLogDataBaseByFilterCondition ();
    if (EFI_ERROR (Status)) {
      mEventLogNum = 0;
      mCurrentListStartIndex = 0;
      mCurrentListEndIndex   = 0;
      //return EFI_NOT_FOUND;
    } else {
      if (mEventLogNum == 0) {
        mCurrentListStartIndex = 0;
        mCurrentListEndIndex   = 0;
        //return EFI_NOT_FOUND;
      } else {
        mCurrentListStartIndex = 1;
        if ((mCurrentListStartIndex + MAX_SHOW_ELV_LIST_NUM - 1) <= mEventLogNum) {
          mCurrentListEndIndex = mCurrentListStartIndex + MAX_SHOW_ELV_LIST_NUM - 1;
        } else {
          mCurrentListEndIndex = mEventLogNum;
        }
      }
    }
  } else {
    //
    // Show action: Show Previous or Next page of Events.
    //
    if (ShowAction == SHOW_PREVIOUS_PAGE) {
      if (mCurrentListStartIndex <= MAX_SHOW_ELV_LIST_NUM) {
        //
        // No more previours events, skip.
        //
        return EFI_SUCCESS;
      } else {
        mCurrentListStartIndex = mCurrentListStartIndex - MAX_SHOW_ELV_LIST_NUM;
        mCurrentListEndIndex = mCurrentListStartIndex + MAX_SHOW_ELV_LIST_NUM - 1;
      }
    } else if (ShowAction == SHOW_NEXT_PAGE) {
      if (mCurrentListEndIndex == mEventLogNum) {
        //
        // No more next events, skip.
        //
        return EFI_SUCCESS;
      } 
      
      if ((mCurrentListEndIndex + MAX_SHOW_ELV_LIST_NUM) <= mEventLogNum) {
        mCurrentListStartIndex = mCurrentListStartIndex + MAX_SHOW_ELV_LIST_NUM;
        mCurrentListEndIndex = mCurrentListStartIndex + MAX_SHOW_ELV_LIST_NUM - 1;
      } else {
        mCurrentListStartIndex = mCurrentListStartIndex + MAX_SHOW_ELV_LIST_NUM;
        mCurrentListEndIndex = mEventLogNum;
      }
    } else {
      return EFI_INVALID_PARAMETER;
    }
  }

  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);
  
  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  StartLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = ELV_STORAGE_HEADER_START_LABEL; 

  EndLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = ELV_STORAGE_HEADER_END_LABEL; 
    
  //
  // For "Save XXXX Event Storage."
  //
  UnicodeSPrint (
    StringPtr, 
    sizeof (StringPtr), 
    L"Save %s Event Storage",
    mStorageName
    );

  SaveEventOpCodeHandle = HiiAllocateOpCodeHandle ();
  HiiCreateOneOfOptionOpCode (
          SaveEventOpCodeHandle,
          STRING_TOKEN (STR_SAVE_ALL_EVENTS),
          EFI_IFR_OPTION_DEFAULT,
          EFI_IFR_TYPE_NUM_SIZE_8,
          (UINT8)0
          );
  HiiCreateOneOfOptionOpCode (
          SaveEventOpCodeHandle,
          STRING_TOKEN (STR_SAVE_CURRENT_EVENTS),
          0,
          EFI_IFR_TYPE_NUM_SIZE_8,
          (UINT8)1
          );

//[-start-180213-IB08400569-modify]//
  if (mSaveEventStringId == 0) {    
    mSaveEventStringId = HiiSetString (HiiHandle, 0, StringPtr, NULL);
  } else {    
    TempStringId = HiiSetString (HiiHandle, mSaveEventStringId, StringPtr, NULL);
    if (TempStringId != mSaveEventStringId) {
      mSaveEventStringId = TempStringId;
      DEBUG ((EFI_D_ERROR, "String ID changed. New mSaveEventStringId = 0x%x\n", mSaveEventStringId));      
    }
  }

  QuestionFlags = EFI_IFR_FLAG_CALLBACK;
  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    (EFI_QUESTION_ID) KEY_ELV_SAVE_EVENT_STORAGE,
    0,
    0,
    mSaveEventStringId,
    STRING_TOKEN (STR_SAVE_EVENT_HELP),
    QuestionFlags,
    EFI_IFR_NUMERIC_SIZE_1,
    SaveEventOpCodeHandle,
    NULL
    );
//[-end-180213-IB08400569-modify]//

  if ((mCurrentListStartIndex != 1) && (mCurrentListStartIndex != 0)) {
    HiiCreateActionOpCode (
    StartOpCodeHandle,                                 // Container for dynamic created opcodes
    (EFI_QUESTION_ID) KEY_SHOW_EVENT_PREVIOUS_PAGE,    // Question ID
    STRING_TOKEN(STR_SHOW_EVENT_PREVIOUS_PAGE),        // String ID for Prompt
    STRING_TOKEN(STR_SHOW_EVENT_PREVIOUS_PAGE),        // String ID for Help
    EFI_IFR_FLAG_CALLBACK,                             // Flags in Question Header
    0
    );
  }
  
  if (mCurrentListEndIndex != mEventLogNum) {
    HiiCreateActionOpCode (
    StartOpCodeHandle,                                 // Container for dynamic created opcodes
    (EFI_QUESTION_ID) KEY_SHOW_EVENT_NEXT_PAGE,        // Question ID
    STRING_TOKEN(STR_SHOW_EVENT_NEXT_PAGE),            // String ID for Prompt
    STRING_TOKEN(STR_SHOW_EVENT_NEXT_PAGE),            // String ID for Help
    EFI_IFR_FLAG_CALLBACK,                             // Flags in Question Header
    0
    );
  }

  //
  // For "XXXX Event Storage"
  //
  UnicodeSPrint (
    StringPtr, 
    sizeof (StringPtr), 
    L"- Total Event Count     : %d",
    mEventLogNum
    );
//[-start-180213-IB08400569-modify]//
  if (mTotalCountStringId == 0) {
    mTotalCountStringId = HiiSetString (HiiHandle, 0, StringPtr, NULL);
  } else {
    TempStringId = HiiSetString (HiiHandle, mTotalCountStringId, StringPtr, NULL);
    if (TempStringId != mTotalCountStringId) {
      mTotalCountStringId = TempStringId;
      DEBUG ((EFI_D_ERROR, "String ID changed. New mTotalCountStringId = 0x%x\n", mTotalCountStringId));      
    }
  }
  HiiCreateSubTitleOpCode (
    StartOpCodeHandle,
    mTotalCountStringId,
    0,
    0,
    0
    );
//[-end-180213-IB08400569-modify]//

  UnicodeSPrint (
    StringPtr, 
    sizeof (StringPtr), 
    L"- Current Event Number  : %d to %d",
    mCurrentListStartIndex,
    mCurrentListEndIndex
    );
//[-start-180213-IB08400569-modify]//
  if (mCurrentCountStringId == 0) {
    mCurrentCountStringId = HiiSetString (HiiHandle, 0, StringPtr, NULL);
  } else {
    TempStringId = HiiSetString (HiiHandle, mCurrentCountStringId, StringPtr, NULL);
    if (TempStringId != mCurrentCountStringId) {
      mCurrentCountStringId = TempStringId;
      DEBUG ((EFI_D_ERROR, "String ID changed. New mCurrentCountStringId = 0x%x\n", mCurrentCountStringId));      
    }
  }
  HiiCreateSubTitleOpCode (
    StartOpCodeHandle,
    mCurrentCountStringId,
    0,
    0,
    0
    );
//[-end-180213-IB08400569-modify]//
  
  Status = HiiUpdateForm (
             HiiHandle,
             &gH2OBiosEventLogConfigUtilEventAndMessageFormsetGuid, 
             (EFI_FORM_ID)VFR_FORMID_BIOS_EVENT_LOG_VIEWER_STORAGE,
             StartOpCodeHandle,
             EndOpCodeHandle
             );

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);

  //
  // Add Event List.
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);
  
  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  StartLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = ELV_STORAGE_EVENTS_START_LABEL; 

  EndLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = ELV_STORAGE_EVENTS_END_LABEL; 

  if (mEventLogNum == 0) {
    //
    // Clear the Event List.
    //
    Status = HiiUpdateForm (
               HiiHandle,
               &gH2OBiosEventLogConfigUtilEventAndMessageFormsetGuid, 
               (EFI_FORM_ID)VFR_FORMID_BIOS_EVENT_LOG_VIEWER_STORAGE,
               StartOpCodeHandle,
               EndOpCodeHandle
               );
  } else {
//[-start-180214-IB08400569-modify]//
    // Print the Event List.
    for (Index = (mCurrentListStartIndex - 1), EventStringIndex = 0; Index < mCurrentListEndIndex; Index++, EventStringIndex++) {
      if (!GetEventString ((VOID *)&(mEventLogData[Index]), &EventString, &EventStringSize)) {
        UnicodeSPrint (
          StringPtr, 
          sizeof (StringPtr), 
          L"[%04d] %d/%02d/%02d %02d:%02d:%02d %s",
          (Index + 1),
          mEventLogData[Index].Year, 
          mEventLogData[Index].Month, 
          mEventLogData[Index].Date, 
          mEventLogData[Index].Hour, 
          mEventLogData[Index].Minute, 
          mEventLogData[Index].Second,
          EventString
          );
        FreePool (EventString);
      } else {
        UnicodeSPrint (
          StringPtr, 
          sizeof (StringPtr), 
          L"[%04d] %d/%02d/%02d %02d:%02d:%02d Undefined.",
          (Index + 1),
          mEventLogData[Index].Year, 
          mEventLogData[Index].Month, 
          mEventLogData[Index].Date, 
          mEventLogData[Index].Hour, 
          mEventLogData[Index].Minute, 
          mEventLogData[Index].Second
          );
      }

      ZeroMem (ShowStringPtr, sizeof(ShowStringPtr));
//[-start-180718-IB08400617-modify]//
//[-start-180824-IB07400999-modify]//
      if (StrSize(StringPtr) >= sizeof(CHAR16)*80) {
//        StringPtr[74] = L'\0';
//        StrCatS (ShowStringPtr, MAX_ELV_STR_NUM, StringPtr);
        CopyMem(ShowStringPtr, StringPtr, sizeof(CHAR16)*74);
        StrCatS (ShowStringPtr, MAX_ELV_STR_NUM, L".....");
      } else {
//        StrCatS (ShowStringPtr, MAX_ELV_STR_NUM, StringPtr);
        CopyMem(ShowStringPtr, StringPtr, StrSize (StringPtr));
      }
//[-end-180824-IB07400999-modify]//
//[-end-180718-IB08400617-modify]//
      
      if (mShowEventStringId[EventStringIndex] == 0) {
        DEBUG ((EFI_D_ERROR, "ShowStringPtr = %s\n", ShowStringPtr));   
        DEBUG ((EFI_D_ERROR, "StringPtr = %s\n", StringPtr));  
        mShowEventStringId[EventStringIndex] = HiiSetString (HiiHandle, 0, ShowStringPtr, NULL);
      } else {
        TempStringId = HiiSetString (HiiHandle, mShowEventStringId[EventStringIndex], ShowStringPtr, NULL);
        if (TempStringId != mShowEventStringId[EventStringIndex]) {
          mShowEventStringId[EventStringIndex] = TempStringId;
          DEBUG ((EFI_D_ERROR, "String ID changed. New mShowEventStringId[%x] = 0x%x\n", EventStringIndex, mShowEventStringId[EventStringIndex]));      
        }
      }
      DEBUG ((EFI_D_ERROR, "mShowEventStringId[%x]: 0x%x\n", EventStringIndex, mShowEventStringId[EventStringIndex]));
      if (mShowEventStringId[EventStringIndex] == 0) {
        DEBUG ((EFI_D_ERROR, "Cannot Set String ID. mShowEventStringId[%x] = 0\n", EventStringIndex));
      }

      GetDetailedEventInfo (Index, StringPtr2);
      if (mShowEventHelpStringId[EventStringIndex] == 0) {
        mShowEventHelpStringId[EventStringIndex] = HiiSetString (HiiHandle, 0, StringPtr2, NULL);
      } else {
        TempStringId = HiiSetString (HiiHandle, mShowEventHelpStringId[EventStringIndex], StringPtr2, NULL);
        if (TempStringId != mShowEventHelpStringId[EventStringIndex]) {
          mShowEventHelpStringId[EventStringIndex] = TempStringId;
          DEBUG ((EFI_D_ERROR, "String ID changed. New mShowEventHelpStringId[%x] = 0x%x\n", EventStringIndex, mShowEventHelpStringId[EventStringIndex]));      
        }
      }
      DEBUG ((EFI_D_ERROR, "mShowEventHelpStringId[%x]: 0x%x\n", EventStringIndex, mShowEventHelpStringId[EventStringIndex]));
      if (mShowEventHelpStringId[EventStringIndex] == 0) {
        DEBUG ((EFI_D_ERROR, "Cannot Set String ID. mShowEventHelpStringId[%x] = 0\n", EventStringIndex));
      }

      HiiCreateActionOpCode (
        StartOpCodeHandle,             // Container for dynamic created opcodes
        (EFI_QUESTION_ID) (ELV_LIST_BASE + Index),    // Question ID
        mShowEventStringId[EventStringIndex],             // String ID for Prompt
        mShowEventHelpStringId[EventStringIndex],         // String ID for Help
        EFI_IFR_FLAG_CALLBACK,         // Flags in Question Header
        0
        );      
    }
//[-end-180214-IB08400569-modify]//

    if (mCurrentListStartIndex != 1) {
      HiiCreateActionOpCode (
      StartOpCodeHandle,                                 // Container for dynamic created opcodes
      (EFI_QUESTION_ID) KEY_SHOW_EVENT_PREVIOUS_PAGE2,    // Question ID
      STRING_TOKEN(STR_SHOW_EVENT_PREVIOUS_PAGE),        // String ID for Prompt
      STRING_TOKEN(STR_SHOW_EVENT_PREVIOUS_PAGE),        // String ID for Help
      EFI_IFR_FLAG_CALLBACK,                             // Flags in Question Header
      0
      );
    }
  
    if (mCurrentListEndIndex != mEventLogNum) {
      HiiCreateActionOpCode (
      StartOpCodeHandle,                                 // Container for dynamic created opcodes
      (EFI_QUESTION_ID) KEY_SHOW_EVENT_NEXT_PAGE2,        // Question ID
      STRING_TOKEN(STR_SHOW_EVENT_NEXT_PAGE),            // String ID for Prompt
      STRING_TOKEN(STR_SHOW_EVENT_NEXT_PAGE),            // String ID for Help
      EFI_IFR_FLAG_CALLBACK,                             // Flags in Question Header
      0
      );
    }
  
    Status = HiiUpdateForm (
               HiiHandle,
               &gH2OBiosEventLogConfigUtilEventAndMessageFormsetGuid, 
               (EFI_FORM_ID)VFR_FORMID_BIOS_EVENT_LOG_VIEWER_STORAGE,
               StartOpCodeHandle,
               EndOpCodeHandle
               );
    
  }

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);
  
  return EFI_SUCCESS;
}

/**
 Get the available Event Storages.

 @param[in,out] StorageNum             Return the Event Storage number.

 @retval EFI_SUCCESS                   Process successfully.
**/
CHAR16 **
EFIAPI
GetStorageNameList (
  IN OUT  UINT32             *StorageNum
  )
{
  EFI_STATUS                      Status;
  CHAR16                          **StorageNameList;
  UINTN                           TempStorageNum;
  UINT32                          Index;
  EVENT_STORAGE_INFO              *StorageName;

  TempStorageNum = 0;
  Status = mEventLog->GetStorageName (&StorageName, &TempStorageNum);
  if (EFI_ERROR(Status)) {
    *StorageNum = 0;
    return NULL;
  }
  
  StorageNameList = AllocateZeroPool (sizeof (CHAR16*) * TempStorageNum);
  if (StorageNameList == NULL) {
    return NULL;
  } 

  for (Index = 0; Index < TempStorageNum; Index++) {
    StorageNameList[Index] = AllocateZeroPool (sizeof (CHAR16) * MAX_STORAGE_NAME_NUM);  
//[-start-180718-IB08400617-modify]//
    StrCpyS(StorageNameList[Index], MAX_STORAGE_NAME_NUM, StorageName[Index].EventStorageName);
//[-end-180718-IB08400617-modify]//
  }

  *StorageNum = (UINT32)TempStorageNum;
  
  return StorageNameList;
}

/**
 Save the data buffer to file system.

 @param[in]  *Volume                   Simple File System protocol.  
 @param[in]  *FileName                 File name.    
 @param[in]  *SavingDataBuffer         Date buffer to be stored. 
 @param[in]  SavingDataSize            Size of data buffer.

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
CreateSavingFile (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL    *Volume,  
  IN CHAR16                             *FileName,
  IN VOID                               *SavingDataBuffer,
  IN UINTN                              SavingDataSize
  )
{
  EFI_FILE                                *Root;
  EFI_FILE                                *File;
  EFI_STATUS                              Status;

  File = NULL;

  Status = Volume->OpenVolume (
                     Volume,
                     &Root
                     );
  if (EFI_ERROR (Status)) {
    return EFI_NO_MEDIA;
  }

  //
  // Confirm the FileName is existence or not.
  //
  Status = Root->Open (
                   Root,
                   &File,
                   FileName,
                   EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                   0
                   );
  if (EFI_ERROR (Status)) {
    //
    // No the same FileName be found. Create it and write event to it.
    //
    Status = Root->Open (
                     Root,
                     &File,
                     FileName,
                     EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                     0
                     );
    if (EFI_ERROR (Status)) {      
      Root->Close (Root);
      return EFI_NO_MEDIA;
    }
  }  
  //
  // The same FileName is existence. Overwrite event to the same file
  //
  Status = Root->Write (File, &SavingDataSize, SavingDataBuffer);
  Status = Root->Open (
                   Root,
                   &File,
                   FileName,
                   EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                   0
                   );
  Root->Close (Root);

  return Status;
}

/**
 Save the event log date to file system.
 
 @param[in]  HiiHandle                 EFI HII Handle.
 @param[in]  SaveAllEvent              Save all events or current events.

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
SaveEventLogToFile (
  IN  EFI_HII_HANDLE  HiiHandle,
  IN  BOOLEAN         SaveAllEvent
  )
{
  UINTN                                 Index;  
  UINTN                                 Index2;
  CHAR16                                *EventString = NULL;
  UINTN                                 StringSize = 0;  
  UINTN                                 TotalStringSize = 0;    
  VOID                                  *FinalString = NULL;
  VOID                                  *TempString = NULL;
  UINTN                                 TempStringSize = 0;
  BOOLEAN                               HaveEvent = FALSE;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *Volume = NULL;
  CHAR16                                *FileName = NULL;
  CHAR8                                 *AsciiString = NULL;
  BOOLEAN                               SaveEvent = FALSE;
  CHAR16                                *StringPtr;
  CHAR16                                StringPtr2[MAX_ELV_STR_NUM];
  EFI_STATUS                            Status = EFI_UNSUPPORTED;
  EFI_INPUT_KEY                         Key;
  UINTN                                 SaveStartIndex;
  UINTN                                 SaveEndIndex;

  if (mEventLogNum == 0) {
    //
    // There is no Event existed.
    //
    StringPtr = HiiGetString (
                  HiiHandle,
                  STRING_TOKEN (STR_NO_EVENT_EXISTS),
                  NULL
                  );
    Status = ElvConfirmDialog (
               DlgOk,
               FALSE,
               0,
               NULL,
               &Key,
               StringPtr
               );
    FreePool (StringPtr);

    return EFI_SUCCESS;
  }
    
  if (SaveAllEvent == TRUE) {
    UnicodeSPrint (
        StringPtr2, 
        sizeof (StringPtr2), 
        L"Save All Events?"
        ); 
    Status = ElvConfirmDialog (
               DlgYesNo,
               FALSE,
               0,
               NULL,
               &Key,
               StringPtr2
               );
    if (Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
      //
      // User skips saving action.
      //
      return EFI_SUCCESS;
    }
  } else {
    UnicodeSPrint (
        StringPtr2, 
        sizeof (StringPtr2), 
        L"Save Current Page Events from %d to %d ?",
        mCurrentListStartIndex,
        mCurrentListEndIndex
        ); 
    Status = ElvConfirmDialog (
               DlgYesNo,
               FALSE,
               0,
               NULL,
               &Key,
               StringPtr2
               );
    if (Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
      //
      // User skips saving action.
      //
      return EFI_SUCCESS;
    }
  }

  //
  // Get removable or non-removable device for saving event(s)
  //

  // Firstly, search removable device
  Volume = SearchFileSystem (TRUE);
  if (Volume == NULL) {
    // Search non-removable device
    Volume = SearchFileSystem (FALSE);
  }

  if (Volume == NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // Create a file name. ex: 20150101030201BIOS.log
  //
  ObtainSavingFileNameByTime (mStorageName, &FileName);

  //
  // Arrange data format as below.
  //
  //--------------------------------------------------------------------------------
  // Entry Num       : XXXX
  // Event Type ID   : 0xXX
  // Time Stamp      : Year/Month/Day Hour:Minute:Second
  // Event Data      : 0xXX 0xXX 0xXX 0xXX
  // Event Raw Data  : 0xXX 0xXX 0xXX 0xXX
  // Event String    : String
  //--------------------------------------------------------------------------------
  //  
  if (SaveAllEvent == FALSE) {
    //
    // save current page events
    //
    SaveStartIndex = mCurrentListStartIndex;
    SaveEndIndex = mCurrentListEndIndex;
  } else {
    //
    // save all events
    //
    SaveStartIndex = 1;
    SaveEndIndex = mEventLogNum;
  }

  //
  // Reserve 32M bytes to collect events
  //
  FinalString = AllocateZeroPool (0x2000000);
  if (FinalString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  for (Index = (SaveStartIndex - 1); Index < SaveEndIndex; Index++) {    
    TempStringSize += StrLen (L"Entry Num        : DDDD ") * 2;
    TempString = AllocateZeroPool (TempStringSize);
    if (TempString == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    UnicodeSPrint (
      TempString, 
      TempStringSize, 
      L"Entry Num        : %04d",
      Index + 1      
    );  
//[-start-180718-IB08400617-modify]//
    StrCatS (FinalString, MAX_ELV_STR_NUM, TempString);
    FreePool (TempString);
    TotalStringSize += TempStringSize;    
    StrCatS (FinalString, MAX_ELV_STR_NUM, L"\r\n");

    if (mEventLogData[Index].EventTypeIdEnable) {
      TempStringSize = StrLen (L"Event Type ID    : 0xXX ") * 2;
      TempString = AllocateZeroPool (TempStringSize);
      if (TempString == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      UnicodeSPrint (
        TempString, 
        TempStringSize, 
        L"Event Type ID    : 0x%02x",
        mEventLogData[Index].EventTypeID     
      );  
      StrCatS (FinalString, MAX_ELV_STR_NUM, TempString);
      FreePool (TempString);
      TotalStringSize += TempStringSize;
      StrCatS (FinalString, MAX_ELV_STR_NUM, L"\r\n");
    }

    TempStringSize = StrLen (L"Time Stamp       : DDDD/DD/DD DD:DD:DD ") * 2;
    TempString = AllocateZeroPool (TempStringSize);
    if (TempString == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    UnicodeSPrint (
      TempString, 
      TempStringSize, 
      L"Time Stamp       : %04d/%02d/%02d %02d:%02d:%02d",
      mEventLogData[Index].Year,
      mEventLogData[Index].Month,
      mEventLogData[Index].Date,
      mEventLogData[Index].Hour,
      mEventLogData[Index].Minute,
      mEventLogData[Index].Second
    );  
    StrCatS (FinalString, MAX_ELV_STR_NUM, TempString);
    FreePool (TempString);
    TotalStringSize += TempStringSize;
    StrCatS (FinalString, MAX_ELV_STR_NUM, L"\r\n");

    if (mEventLogData[Index].GeneratorIdEnable) {    
      TempStringSize = StrLen (L"Generator ID     : 0xXXXX ") * 2;
      TempString = AllocateZeroPool (TempStringSize);
      if (TempString == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      UnicodeSPrint (
        TempString, 
        TempStringSize, 
        L"Generator ID     : 0x%04x",
        mEventLogData[Index].GeneratorId
      );  
      StrCatS (FinalString, MAX_ELV_STR_NUM, TempString);
      FreePool (TempString);
      TotalStringSize += TempStringSize;
      StrCatS (FinalString, MAX_ELV_STR_NUM, L"\r\n");
    }

    if (mEventLogData[Index].EventIdEnable) {
      //
      // For storage driver that don't want to show Sensor Type, Sensor Num, Event Type.
      // Just want to show Event ID.
      // Like BIOS =>  Event ID     : 0xXX 0xXX 0xXX.
      //
      TempStringSize = StrLen (L"Event ID         : ")*2;
      if ((mEventLogData[Index].Data == NULL) && ((mEventLogData[Index].EventID.SensorType == 0xFF) && (mEventLogData[Index].EventID.SensorNum == 0xFF) && (mEventLogData[Index].EventID.EventType == 0xFF))) {
        TempStringSize += StrLen (L"NULL \n")*2;
        StrCatS (FinalString, MAX_ELV_STR_NUM, L"Event ID         : ");        
        StrCatS (FinalString, MAX_ELV_STR_NUM, L"NULL \n");
      } else {
        TempStringSize += StrLen (L"0xXX 0xXX 0xXX ")*2;
        TempString = AllocateZeroPool (TempStringSize);
        if (TempString == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }
        UnicodeSPrint (
          TempString, 
          TempStringSize, 
          L"Event ID         : 0x%02x 0x%02x 0x%02x",
          mEventLogData[Index].EventID.SensorType,
          mEventLogData[Index].EventID.SensorNum,
          mEventLogData[Index].EventID.EventType
        );  
        StrCatS (FinalString, MAX_ELV_STR_NUM, TempString);
        FreePool (TempString);
        StrCatS (FinalString, MAX_ELV_STR_NUM, L"\r\n");
      }      
      TotalStringSize += TempStringSize;
    } else {
      //
      // For storage driver that want to show Sensor Type, Sensor Num, Event Type.
      // Like BMC SEL =>  Sensor Type     : 0xXX
      //                  Sensor Num      : 0xXX
      //                  Event  Type     : 0xXX
      //
      TempStringSize =  StrLen (L"Sensor Type      : 0xXX ")*2;
      TempString = AllocateZeroPool (TempStringSize);
      if (TempString == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      UnicodeSPrint (
        TempString, 
        TempStringSize, 
        L"Sensor Type      : 0x%02x",
        mEventLogData[Index].EventID.SensorType
      );  
      StrCatS (FinalString, MAX_ELV_STR_NUM, TempString);
      FreePool (TempString);
      TotalStringSize += TempStringSize;
      StrCatS (FinalString, MAX_ELV_STR_NUM, L"\r\n");

      TempStringSize = StrLen (L"Sensor Num       : 0xXX ")*2;
      TempString = AllocateZeroPool (TempStringSize);
      if (TempString == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      UnicodeSPrint (
        TempString, 
        TempStringSize, 
        L"Sensor Num       : 0x%02x",
        mEventLogData[Index].EventID.SensorNum
      );  
      StrCatS (FinalString, MAX_ELV_STR_NUM, TempString);
      FreePool (TempString);
      TotalStringSize += TempStringSize;
      StrCatS (FinalString, MAX_ELV_STR_NUM, L"\r\n");

      TempStringSize = StrLen (L"Event  Type      : 0xXX ") * 2;
      TempString = AllocateZeroPool (TempStringSize);
      if (TempString == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      UnicodeSPrint (
        TempString, 
        TempStringSize, 
        L"Event  Type      : 0x%02x",
        mEventLogData[Index].EventID.EventType
      );  
      StrCatS (FinalString, MAX_ELV_STR_NUM, TempString);
      FreePool (TempString);
      TotalStringSize += TempStringSize;
      StrCatS (FinalString, MAX_ELV_STR_NUM, L"\r\n");
    }

    TempStringSize = StrLen (L"Event Data       : ") * 2;
    if (mEventLogData[Index].Data != NULL) {
      TempStringSize += StrLen (L"0xXX 0xXX 0xXX ") * 2;
      TempString = AllocateZeroPool (TempStringSize);
      if (TempString == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      UnicodeSPrint (
        TempString, 
        TempStringSize, 
        L"Event Data       : 0x%02x 0x%02x 0x%02x",
        mEventLogData[Index].Data[0],
        mEventLogData[Index].Data[1],
        mEventLogData[Index].Data[2]);  
      StrCatS (FinalString, MAX_ELV_STR_NUM, TempString);
      FreePool (TempString);
      StrCatS (FinalString, MAX_ELV_STR_NUM, L"\r\n");
    } else {
      TempStringSize += StrLen (L"NULL                \n") * 2;
      StrCatS (FinalString, MAX_ELV_STR_NUM, L"Event Data       : ");        
      StrCatS (FinalString, MAX_ELV_STR_NUM, L"NULL                \n");        
    }    
    TotalStringSize += TempStringSize;
    
    TempStringSize = StrLen (L"Event Raw Data   : ") * 2;
    StrCatS (FinalString, MAX_ELV_STR_NUM, L"Event Raw Data   : ");      
    TotalStringSize += TempStringSize;
    for (Index2=0; Index2 < (mEventLogData[Index].EventRawDataSize); Index2++) {   
      TempStringSize = StrLen (L"0xXX  ") * 2;
      TempString = AllocateZeroPool (TempStringSize);
      if (TempString == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      UnicodeSPrint (
        TempString, 
        TempStringSize, 
        L"0x%02x ",
        mEventLogData[Index].EventRawData[Index2]);  
      StrCatS (FinalString, MAX_ELV_STR_NUM, TempString);
      FreePool (TempString);      
      TotalStringSize += (TempStringSize);
      HaveEvent = TRUE;
//[-start-161109-IB04560700-remove]//
//      if (!HaveEvent) {
//        TempStringSize = StrLen (L"NULL") * 2;
//        StrCatS (FinalString, MAX_ELV_STR_NUM, L"NULL");      
//        TotalStringSize += TempStringSize;
//      }
//[-end-161109-IB04560700-remove]//
    }
//[-start-161109-IB04560700-add]//
    if (!HaveEvent) {
      TempStringSize = StrLen (L"NULL") * 2;
      StrCatS (FinalString, MAX_ELV_STR_NUM, L"NULL");      
      TotalStringSize += TempStringSize;
    }
//[-end-161109-IB04560700-add]//
    StrCatS (FinalString, MAX_ELV_STR_NUM, L"\r\n");      
    TotalStringSize += TempStringSize;

    TempStringSize = StrLen (L"Event String     : ") * 2;    
    StrCatS (FinalString, MAX_ELV_STR_NUM, L"Event String     : ");
    if (!GetEventString ((VOID *)&(mEventLogData[Index]), &EventString, &StringSize)) {  
      TempStringSize += StringSize;
      StrCatS (FinalString, MAX_ELV_STR_NUM, EventString);
      FreePool (EventString);
    } else {
      TempStringSize += StrLen (L"Undefined") * 2;
      StrCatS (FinalString, MAX_ELV_STR_NUM, L"Undefined");
    }
    TotalStringSize += TempStringSize;
    
    TempStringSize = StrLen (L"\r\n\r\n")*2;
    StrCatS (FinalString, MAX_ELV_STR_NUM, L"\r\n\r\n");
    TotalStringSize += TempStringSize;
//[-end-180718-IB08400617-modify]//
    
    SaveEvent = TRUE;
  }

  if (SaveEvent) {
    
    //
    // Transfer format from unicode to ascii and save event to file.
    //
    AsciiString = AllocateZeroPool (StrLen (FinalString));
    if (AsciiString == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
//[-start-180719-IB08400617-modify]//
//[-start-180823-IB07400999-modify]//
    UnicodeStrToAsciiStr (FinalString, AsciiString);
//[-end-180823-IB07400999-modify]//
//[-end-180719-IB08400617-modify]//
    Status = CreateSavingFile (Volume, FileName, AsciiString, StrLen (FinalString) + 1);
    if (!EFI_ERROR (Status)) {
      UnicodeSPrint (
        StringPtr2, 
        sizeof (StringPtr2), 
        L"Save Events to file %s",
        FileName
        ); 
      Status = ElvConfirmDialog (
                 DlgOk,
                 FALSE,
                 0,
                 NULL,
                 &Key,
                 StringPtr2
                 );
    }

    FreePool (FinalString);
    FreePool (AsciiString);
  }

  return Status;
}

/**
 Show the Event Log data on the Event Storage page.

 @param[in]  HiiHandle                 EFI HII Handle.

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
ShowEventLog (
  IN  EFI_HII_HANDLE  HiiHandle
  )
{
  EFI_STATUS                      Status;
  CHAR16                          **StorageNameList;
  UINT32                          StorageNameNum;
  UINT32                          SelectedIndex;
  EFI_INPUT_KEY                   Key;
  CHAR16                          *StringPtr;
  UINT32                          Index;
  H2O_DIALOG_PROTOCOL             *H2ODialog;  
  CHAR16                          StringPtr2[MAX_ELV_STR_NUM] = {0};
  
  Status = gBS->LocateProtocol (
                  &gH2ODialogProtocolGuid,
                  NULL,
                  (VOID **)&H2ODialog
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateProtocol (&gH2OEventLogProtocolGuid, NULL, (VOID **)&mEventLog);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  StorageNameNum = 0;
  SelectedIndex = 0;
  StorageNameList = NULL;
  StorageNameList = GetStorageNameList (&StorageNameNum);
  if ((StorageNameNum == 0) || (StorageNameList == NULL)) {
    //
    // There is no storage existed.
    //
    StringPtr = HiiGetString (
                  HiiHandle,
                  STRING_TOKEN (STR_NO_STORAGE_EXISTS),
                  NULL
                  );
    Status = ElvConfirmDialog (
               DlgOk,
               FALSE,
               0,
               NULL,
               &Key,
               StringPtr
               );
    FreePool (StringPtr);
  } else {
    StringPtr = HiiGetString (
                    HiiHandle,
                    STRING_TOKEN (STR_SELECT_EVENT_STORAGE),
                    NULL
                    );
    Status = H2ODialog->OneOfOptionDialog (
                          (UINT32)StorageNameNum, // Storage Number
                          FALSE,
                          NULL,
                          &Key,
                          MAX_STORAGE_NAME_NUM,
                          StringPtr,
                          &SelectedIndex,
                          (CHAR16 **) (StorageNameList),
                          0
                          );
    FreePool (StringPtr);

//[-start-171006-IB08400521-modify]//
    if (!EFI_ERROR (Status) && (Key.UnicodeChar == CHAR_CARRIAGE_RETURN)) {
      //
      // Update the event data to related labels in Event Log Viewer for Storage page.
      //
      ZeroMem (mStorageName, sizeof (mStorageName));
//[-start-180718-IB08400617-modify]//
      StrCpyS (mStorageName, MAX_STORAGE_NAME_NUM, StorageNameList[SelectedIndex]);
//[-end-180718-IB08400617-modify]//
      SetStorageTypeForFilter ();
      Status = UpdateElvStoragePage (HiiHandle, SHOW_FIRST_PAGE);
      if (EFI_ERROR (Status)) {
        UnicodeSPrint (
        StringPtr2, 
        sizeof (StringPtr2), 
        L"Can't get the Event Log Data."
        );
        ElvConfirmDialog (
          DlgOk,
          FALSE,
          0,
          NULL,
          &Key,
          StringPtr2
          );
      }
    } else {
      Status = EFI_INVALID_PARAMETER;
    }
//[-end-171006-IB08400521-modify]//

    for (Index = 0; Index < StorageNameNum; Index++) {
      FreePool (StorageNameList[Index]);  
    }
    FreePool (StorageNameList);
  }
  
  return Status;
}

/**
 Clear the Event Log data on the Event Storage page.

 @param[in]  HiiHandle                 EFI HII Handle.

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
ClearEventLog (
  IN  EFI_HII_HANDLE  HiiHandle
  )
{
  
  EFI_STATUS                      Status;
  CHAR16                          **StorageNameList;
  UINT32                          StorageNameNum;
  UINT32                          SelectedIndex;
  EFI_INPUT_KEY                   Key;
  CHAR16                          *StringPtr;
  CHAR16                          StringPtr2[MAX_ELV_STR_NUM] = {0};
  UINT32                          Index;
  H2O_DIALOG_PROTOCOL             *H2ODialog;
    
  Status = gBS->LocateProtocol (
                  &gH2ODialogProtocolGuid,
                  NULL,
                  (VOID **)&H2ODialog
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateProtocol (&gH2OEventLogProtocolGuid, NULL, (VOID **)&mEventLog);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  StorageNameNum = 0;
  SelectedIndex = 0;
  StorageNameList = NULL;
  StorageNameList = GetStorageNameList (&StorageNameNum);
  if ((StorageNameNum == 0) || (StorageNameList == NULL)) {
    //
    // There is no storage existed.
    //
    StringPtr = HiiGetString (
                  HiiHandle,
                  STRING_TOKEN (STR_NO_STORAGE_EXISTS),
                  NULL
                  );
    Status = ElvConfirmDialog (
               DlgOk,
               FALSE,
               0,
               NULL,
               &Key,
               StringPtr
               );
    FreePool (StringPtr);
  } else {
    StringPtr = HiiGetString (
                    HiiHandle,
                    STRING_TOKEN (STR_SELECT_EVENT_STORAGE),
                    NULL
                    );
    Status = H2ODialog->OneOfOptionDialog (
                          (UINT32)StorageNameNum, // Storage Number
                          FALSE,
                          NULL,
                          &Key,
                          MAX_STORAGE_NAME_NUM,
                          StringPtr,
                          &SelectedIndex,
                          (CHAR16 **) (StorageNameList),
                          0
                          );
    FreePool (StringPtr);
    if (!EFI_ERROR (Status) && (Key.UnicodeChar == CHAR_CARRIAGE_RETURN)){
      ZeroMem (StringPtr2, sizeof(StringPtr2));
      UnicodeSPrint (
        StringPtr2, 
        sizeof (StringPtr2), 
        L"Do you want to clear %s Event Logs",
        StorageNameList[SelectedIndex]
        );      
      Status = ElvConfirmDialog (
                 DlgYesNo,
                 FALSE,
                 0,
                 NULL,
                 &Key,
                 StringPtr2
                 );
      if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
        Status = mEventLog->ClearStorage (StorageNameList[SelectedIndex]);
        if (!EFI_ERROR (Status)) { 
          ZeroMem (StringPtr2, sizeof(StringPtr2));
          UnicodeSPrint (
            StringPtr2, 
            sizeof (StringPtr2), 
            L"%s event logs were cleared!",
            StorageNameList[SelectedIndex]
            );
          Status = ElvConfirmDialog (
                     DlgOk,
                     FALSE,
                     0,
                     NULL,
                     &Key,
                     StringPtr2
                     );
        } else {
          ZeroMem (StringPtr2, sizeof(StringPtr2));
          UnicodeSPrint (
            StringPtr2, 
            sizeof (StringPtr2), 
            L"WARNING!! Please try again later."
            );
          Status = ElvConfirmDialog (
                     DlgOk,
                     FALSE,
                     0,
                     NULL,
                     &Key,
                     StringPtr2
                     );
        }
      }
    }

    //
    // Free Storage name buffer.
    //
    for (Index = 0; Index < StorageNameNum; Index++) {
      FreePool (StorageNameList[Index]);  
    }
    FreePool (StorageNameList);
  }

  return EFI_SUCCESS;
}

/**
 Save the Event Log data on the Event Storage page.

 @param[in]  HiiHandle                 EFI HII Handle.
 @param[in]  SaveAction                Saving policy.

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
SaveEventLog (
  IN  EFI_HII_HANDLE  HiiHandle,
  IN  UINT8           SaveAction
  )
{
  EFI_STATUS                            Status;
  CHAR16                                *StringPtr;
  EFI_INPUT_KEY                         Key;

  
  switch (SaveAction) {
  case SAVE_ALL_EVENT_LOG:
    Status = SaveEventLogToFile (HiiHandle, TRUE);
    break;

  case SAVE_CURRENT_PAGE_EVENT_LOG:
    Status = SaveEventLogToFile (HiiHandle, FALSE);
    break;

  default:
    Status = EFI_UNSUPPORTED;
    break;
  }

  if (EFI_ERROR (Status)) {
    StringPtr = HiiGetString (
                  HiiHandle,
                  STRING_TOKEN (STR_SAVE_EVENT_FAILURE),
                  NULL
                  );
    Status = ElvConfirmDialog (
               DlgOk,
               FALSE,
               0,
               NULL,
               &Key,
               StringPtr
               );
    FreePool (StringPtr);
  }
  return Status;
}

/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Request                A null-terminated Unicode string in
                                 <ConfigRequest> format.
  @param  Progress               On return, points to a character in the Request
                                 string. Points to the string's null terminator if
                                 request was successful. Points to the most recent
                                 '&' before the first failing name/value pair (or
                                 the beginning of the string if the failure is in
                                 the first name/value pair) if the request was not
                                 successful.
  @param  Results                A null-terminated Unicode string in
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
  EFI_STATUS                       Status;
  UINTN                            BufferSize;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;
  EFI_STRING                       ConfigRequest;
  EFI_STRING                       ConfigRequestHdr;
  UINTN                            Size;
  BOOLEAN                          AllocatedRequest;
  EFI_STRING                       Value;
  UINTN                            ValueStrLen;
  UINT8                            MyNameValue0;
  CHAR16                           BackupChar;
//[-start-180507-IB08400587-add]//
  CHAR16                           *StrPointer;
//[-end-180507-IB08400587-add]//

  DEBUG ((EFI_D_INFO, "BiosEventLogConfigUtil ExtractConfig()\n"));
  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize the local variables.
  //
  ConfigRequestHdr  = NULL;
  ConfigRequest     = NULL;
  Size              = 0;
  *Progress         = Request;
  AllocatedRequest  = FALSE;

  HiiConfigRouting = mPrivateData->HiiConfigRouting;

  //
  // Get Buffer Storage data from EFI variable.
  // Try to get the current setting from variable.
  //
  BufferSize = sizeof (BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION);
  Status = gRT->GetVariable (
                  mVariableName,
                  &gH2OBiosEventLogConfigUtilVarstoreGuid,
                  NULL,
                  &BufferSize,
                  &mPrivateData->Configuration
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  if (Request == NULL) {
    //
    // Request is set to NULL, construct full request string.
    //

    //
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    ConfigRequestHdr = HiiConstructConfigHdr (&gH2OBiosEventLogConfigUtilVarstoreGuid, mVariableName, mPrivateData->DriverHandle[0]);
    if (ConfigRequestHdr == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    Size = (StrLen (ConfigRequestHdr) + 32 + 1) * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (Size);
    if (ConfigRequest == NULL) {
      ASSERT (ConfigRequest != NULL);
      FreePool (ConfigRequestHdr);
      ConfigRequestHdr = NULL;
      return EFI_OUT_OF_RESOURCES;
    }
    AllocatedRequest = TRUE;
    UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, (UINT64)BufferSize);
    FreePool (ConfigRequestHdr);
    ConfigRequestHdr = NULL;
  } else {
    //
    // Check routing data in <ConfigHdr>.
    // Note: if only one Storage is used, then this checking could be skipped.
    //
    if (!HiiIsConfigHdrMatch (Request, &gH2OBiosEventLogConfigUtilVarstoreGuid, NULL)) {
      return EFI_NOT_FOUND;
    }
    //
    // Check whether request for EFI Varstore. EFI varstore get data
    // through hii database, not support in this path.
    //
    if (!HiiIsConfigHdrMatch (Request, &gH2OBiosEventLogConfigUtilVarstoreGuid, mVariableName)) {
      return EFI_NOT_FOUND;
    }
    //
    // Set Request to the unified request string.
    //
    ConfigRequest = Request;
//[-start-180507-IB08400587-add]//
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
        ConfigRequest    = AllocateZeroPool (Size);
        ASSERT (ConfigRequest != NULL);
        AllocatedRequest = TRUE;
        UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", Request, (UINT64)BufferSize);
      }
    }    
//[-end-180507-IB08400587-add]//
  }

  if (StrStr (ConfigRequest, L"OFFSET") == NULL) {
    //
    // If requesting Name/Value storage, return value 0.
    //
    //
    // Allocate memory for <ConfigResp>, e.g. Name0=0x11, Name1=0x1234, Name2="ABCD"
    // <Request>   ::=<ConfigHdr>&Name0&Name1&Name2
    // <ConfigResp>::=<ConfigHdr>&Name0=11&Name1=1234&Name2=0041004200430044
    //
    BufferSize = (StrLen (ConfigRequest) + 1 + sizeof (MyNameValue0) * 2 + 1) * sizeof (CHAR16);
    *Results = AllocateZeroPool (BufferSize);
    if (*Results == NULL) {
      ASSERT (*Results != NULL);
      return EFI_OUT_OF_RESOURCES;
    }
//[-start-180718-IB08400617-modify]//
    StrCpyS (*Results, (BufferSize / sizeof(CHAR16)) + 1,ConfigRequest);
//[-end-180718-IB08400617-modify]//
    Value = *Results;

    //
    // Append value of NameValueVar0, type is UINT8
    //
    if ((Value = StrStr (*Results, L"MyNameValue0")) != NULL) {
      MyNameValue0 = 0;

      Value += StrLen (L"MyNameValue0");
      ValueStrLen = ((sizeof (MyNameValue0) * 2) + 1);
//[-start-180719-IB08400617-modify]//
      CopyMem (&Value[ValueStrLen], Value, StrSize (Value));

      BackupChar = Value[ValueStrLen];
      *Value++   = L'=';
//[-start-180823-IB07400999-modify]//
      Value += UnicodeValueToString (
                 Value,
                 PREFIX_ZERO | RADIX_HEX,
                 MyNameValue0,
                 sizeof (MyNameValue0) * 2
                 );
//[-end-180823-IB07400999-modify]//
//[-end-180719-IB08400617-modify]//
      *Value = BackupChar;
    }
    return EFI_SUCCESS;
  }

//[-start-180507-IB08400587-modify]//
  Status = HiiConfigRouting->BlockToConfig (
                               HiiConfigRouting,
                               ConfigRequest,
                               (UINT8 *)&mPrivateData->Configuration,
                               sizeof(BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION),
                               Results,
                               Progress
                               );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ConfigRequestHdr = HiiConstructConfigHdr (&gH2OBiosEventLogConfigUtilVarstoreGuid, mVariableName, mPrivateData->DriverHandle[0]);  
//[-end-180507-IB08400587-modify]//
  
  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    FreePool (ConfigRequest);
  }

  if (ConfigRequestHdr != NULL) {
    FreePool (ConfigRequestHdr);
  }
  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  } else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

  return EFI_SUCCESS;
}

/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Configuration          A null-terminated Unicode string in <ConfigResp>
                                 format.
  @param  Progress               A pointer to a string filled in with the offset of
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
  BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION      BiosEventLogConfig = {0};
  EFI_STATUS                                    Status;
  EFI_HII_CONFIG_ROUTING_PROTOCOL               *HiiConfigRouting;
  UINTN                                         BufferSize;
  
  DEBUG ((EFI_D_INFO, "BiosEventLogConfigUtil RouteConfig()\n"));

  if (Configuration == NULL || Progress == NULL || This == NULL) {
   return EFI_INVALID_PARAMETER;
  }

//[-start-180510-IB08400591-add]//
  *Progress = Configuration;
//[-end-180510-IB08400591-add]//  

//[-start-180507-IB08400587-modify]//
  if (!HiiIsConfigHdrMatch(Configuration, &gH2OBiosEventLogConfigUtilVarstoreGuid, mVariableName)) {
    return EFI_NOT_FOUND;
  }
//[-end-180507-IB08400587-modify]//

  HiiConfigRouting = mPrivateData->HiiConfigRouting;
  BufferSize = sizeof (BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION);
  Status = HiiConfigRouting->ConfigToBlock (
                               HiiConfigRouting,
                               Configuration,
                               (UINT8 *) &BiosEventLogConfig,
                               &BufferSize,
                               Progress
                               );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Store broswer data Buffer Storage to EFI variable
  //
  Status = CommonSetVariable (
             mVariableName,
             &gH2OBiosEventLogConfigUtilVarstoreGuid,
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
             sizeof (BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION),
             &BiosEventLogConfig
             );
  return Status;
  
}

/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Action                 Specifies the type of action taken by the browser.
  @param  QuestionId             A unique value which is sent to the original
                                 exporting driver so that it can identify the type
                                 of data to expect.
  @param  Type                   The type of value for the question.
  @param  Value                  A pointer to the data being sent to the original
                                 exporting driver.
  @param  ActionRequest          On return, points to the action requested by the
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
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_STATUS                                 Status;
  H2O_DIALOG_PROTOCOL                        *H2ODialog;
  FILTER_EVENT_INFO                          FilterEventInfo;
  EFI_INPUT_KEY                              Key;
  CHAR16                                     StringPtr[MAX_ELV_STR_NUM] = {0};
  
  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) && (Action != EFI_BROWSER_ACTION_FORM_CLOSE))||
    (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;

  switch (Action) {
  case EFI_BROWSER_ACTION_FORM_OPEN:
    DEBUG ((EFI_D_INFO, "BiosEventLogConfigUtil EFI_BROWSER_ACTION_FORM_OPEN. \n"));
    break;

  case EFI_BROWSER_ACTION_CHANGING:
    DEBUG ((EFI_D_INFO, "BiosEventLogConfigUtil EFI_BROWSER_ACTION_CHANGING. \n"));
    {
      Status = gBS->LocateProtocol (
                      &gH2ODialogProtocolGuid,
                      NULL,
                      (VOID **)&H2ODialog
                      );
      if (EFI_ERROR (Status)) {
        return Status;
      }

      switch (QuestionId) {
      //
      // For BIOS Event Log
      //
      case KEY_ELV_SHOW_EVENT_LOG:
        DEBUG ((EFI_D_INFO, "KEY_ELV_SHOW_EVENT_LOG!!\n"));
        ArrangeFilterCondition (TRUE, &FilterEventInfo);
        Status = ShowEventLog (mPrivateData->HiiHandle[1]);
        break;  
        
      case KEY_ELV_CLEAR_EVENT_LOG:
        DEBUG ((EFI_D_INFO, "KEY_ELV_CLEAR_EVENT_LOG!!\n"));
        Status = ClearEventLog (mPrivateData->HiiHandle[1]);    
        break;

      case KEY_ELV_SAVE_EVENT_STORAGE:
        DEBUG ((EFI_D_INFO, "KEY_ELV_SAVE_EVENT_STORAGE!!\n"));
        Status = SaveEventLog (mPrivateData->HiiHandle[1], Value->u8); 
        break;

      case KEY_SHOW_EVENT_PREVIOUS_PAGE:
      case KEY_SHOW_EVENT_PREVIOUS_PAGE2:  
        DEBUG ((EFI_D_INFO, "KEY_SHOW_EVENT_PREVIOUS_PAGE!!\n"));
        Status = UpdateElvStoragePage (mPrivateData->HiiHandle[1], SHOW_PREVIOUS_PAGE);
        break;

      case KEY_SHOW_EVENT_NEXT_PAGE:
      case KEY_SHOW_EVENT_NEXT_PAGE2:  
        DEBUG ((EFI_D_INFO, "KEY_SHOW_EVENT_NEXT_PAGE!!\n"));
        Status = UpdateElvStoragePage (mPrivateData->HiiHandle[1], SHOW_NEXT_PAGE);   
        break;

      case KEY_FILTER_ENABLE:
        DEBUG ((EFI_D_INFO, "KEY_FILTER_ENABLE!!\n"));
        switch (Value->u8) {
        case 0: // Disabled
          //
          // 1. Filter setting is Disabled, back up the original setting.
          // 2. Reset the all of the conditions.
          //
          CopyMem (&mBackupFilteredEventLogData, &mFilteredEventLogData, sizeof(STORAGE_EVENT_LOG_INFO));
          mBackupFilterBitMap = mFilterBitMap;
          mFilterBitMap = 0;
          break;

        case 1: // Enabled
          //
          // Filter setting is Enabled, restore the original setting.
          //
          CopyMem (&mFilteredEventLogData, &mBackupFilteredEventLogData, sizeof(STORAGE_EVENT_LOG_INFO));
          mFilterBitMap = mBackupFilterBitMap;
          break;

        case 2: // Clear the condition
          // Check Clear the condition?
          UnicodeSPrint (StringPtr, sizeof (StringPtr), L"Clear all of the filter conditions?");
          ElvConfirmDialog (DlgYesNo, FALSE, 0, NULL, &Key, StringPtr);
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            ArrangeFilterCondition (TRUE, &FilterEventInfo);
            Value->u8 = 0; // reset the setting to "Disabled"
          } else {
            // Return error status to restore the setting.
            return EFI_INVALID_PARAMETER;
          }
          
          break;
        default:
          break;
          
        }
        
        Status = UpdateElvStoragePage (mPrivateData->HiiHandle[1], SHOW_FIRST_PAGE);   
        break;
        
      case KEY_FILTER_DATE:
        DEBUG ((EFI_D_INFO, "KEY_FILTER_DATE!!\n"));
        SetMem (
          &mFilteredEventLogData.Year, 
          sizeof(mFilteredEventLogData.Year) + sizeof(mFilteredEventLogData.Month) + sizeof(mFilteredEventLogData.Date), 
          0xFF
          );
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_YEAR);
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_MONTH);
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_DATE);
        SetMem (&FilterEventInfo, sizeof(FILTER_EVENT_INFO), 0xFF);
        FilterEventInfo.FilterType = TypeDate;
        Status = TransferStringToData (mPrivateData->HiiHandle[1], Value, &FilterEventInfo);
        if (Status == EFI_INVALID_PARAMETER) {
          DEBUG ((EFI_D_ERROR, "Invalid Date condition.\n"));
          UnicodeSPrint (StringPtr, sizeof (StringPtr), L"Invalid Date condition.");
          ElvConfirmDialog (DlgOk, FALSE, 0, NULL, &Key, StringPtr);
          return Status;
        }
        ArrangeFilterCondition (FALSE, &FilterEventInfo);
        Status = UpdateElvStoragePage (mPrivateData->HiiHandle[1], SHOW_FIRST_PAGE);   
        break;

      case KEY_FILTER_TIME:
        DEBUG ((EFI_D_INFO, "KEY_FILTER_TIME!!\n"));
        SetMem (
          &mFilteredEventLogData.Hour, 
          sizeof(mFilteredEventLogData.Hour) + sizeof(mFilteredEventLogData.Minute) + sizeof(mFilteredEventLogData.Second), 
          0xFF
          );
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_HOUR);
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_MINUTE);
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_SECOND);
        SetMem (&FilterEventInfo, sizeof(FILTER_EVENT_INFO), 0xFF);
        FilterEventInfo.FilterType = TypeTime;
        Status = TransferStringToData (mPrivateData->HiiHandle[1], Value, &FilterEventInfo);
        if (Status == EFI_INVALID_PARAMETER) {
          DEBUG ((EFI_D_ERROR, "Invalid Time condition.\n"));
          UnicodeSPrint (StringPtr, sizeof (StringPtr), L"Invalid Time condition.");
          ElvConfirmDialog (DlgOk, FALSE, 0, NULL, &Key, StringPtr);
          return Status;
        }
        ArrangeFilterCondition (FALSE, &FilterEventInfo);
        Status = UpdateElvStoragePage (mPrivateData->HiiHandle[1], SHOW_FIRST_PAGE);   
        break;

      case KEY_FILTER_GENERATOR_ID:
        DEBUG ((EFI_D_INFO, "KEY_FILTER_GENERATOR_ID!!\n"));
        SetMem (
          &mFilteredEventLogData.GeneratorId, 
          sizeof(mFilteredEventLogData.GeneratorId), 
          0xFF
          );
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_GENERATOR_ID);
        SetMem (&FilterEventInfo, sizeof(FILTER_EVENT_INFO), 0xFF);
        FilterEventInfo.FilterType = TypeGeneratorID;
        Status = TransferStringToData (mPrivateData->HiiHandle[1], Value, &FilterEventInfo);
        if (Status == EFI_INVALID_PARAMETER) {
          DEBUG ((EFI_D_ERROR, "Invalid Generator ID condition.\n"));
          UnicodeSPrint (StringPtr, sizeof (StringPtr), L"Invalid Generator ID condition.");
          ElvConfirmDialog (DlgOk, FALSE, 0, NULL, &Key, StringPtr);
          return Status;
        }
        ArrangeFilterCondition (FALSE, &FilterEventInfo);
        Status = UpdateElvStoragePage (mPrivateData->HiiHandle[1], SHOW_FIRST_PAGE);   
        break; 

      case KEY_FILTER_EVENT_ID:
        DEBUG ((EFI_D_INFO, "KEY_FILTER_EVENT_ID!!\n"));
        SetMem (
          &mFilteredEventLogData.EventID, 
          sizeof(mFilteredEventLogData.EventID), 
          0xFF
          );
        
        //
        // Use Sensor Type, Sensor Num, and Event Type to instead Event ID,
        // because Event ID content is stored in "Sensor Type, Sensor Num, and Event Type".
        // This case is used by BIOS Event Storage and Memory Event Storage.
        //
        //mFilterBitMap &= ~B_FILTERED_BY_EVENT_ID;
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_SENSOR_TYPE);
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_SENSOR_NUM);
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_EVENT_TYPE);
        SetMem (&FilterEventInfo, sizeof(FILTER_EVENT_INFO), 0xFF);
        FilterEventInfo.FilterType = TypeEventID;
        Status = TransferStringToData (mPrivateData->HiiHandle[1], Value, &FilterEventInfo);
        if (Status == EFI_INVALID_PARAMETER) {
          DEBUG ((EFI_D_ERROR, "Invalid Event ID condition.\n"));
          UnicodeSPrint (StringPtr, sizeof (StringPtr), L"Invalid Event ID condition.");
          ElvConfirmDialog (DlgOk, FALSE, 0, NULL, &Key, StringPtr);
          return Status;
        }
        ArrangeFilterCondition (FALSE, &FilterEventInfo);
        Status = UpdateElvStoragePage (mPrivateData->HiiHandle[1], SHOW_FIRST_PAGE);   
        break;

      case KEY_FILTER_SENSOR_TYPE:
        DEBUG ((EFI_D_INFO, "KEY_FILTER_SENSOR_TYPE!!\n"));
        SetMem (
          &mFilteredEventLogData.EventID.SensorType, 
          sizeof(mFilteredEventLogData.EventID.SensorType), 
          0xFF
          );
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_SENSOR_TYPE);
        SetMem (&FilterEventInfo, sizeof(FILTER_EVENT_INFO), 0xFF);
        FilterEventInfo.FilterType = TypeSensorType;
        Status = TransferStringToData (mPrivateData->HiiHandle[1], Value, &FilterEventInfo);
        if (Status == EFI_INVALID_PARAMETER) {
          DEBUG ((EFI_D_ERROR, "Invalid Sensor Type condition.\n"));
          UnicodeSPrint (StringPtr, sizeof (StringPtr), L"Invalid Sensor Type condition.");
          ElvConfirmDialog (DlgOk, FALSE, 0, NULL, &Key, StringPtr);
          return Status;
        }
        ArrangeFilterCondition (FALSE, &FilterEventInfo);
        Status = UpdateElvStoragePage (mPrivateData->HiiHandle[1], SHOW_FIRST_PAGE);   
        break;
        
      case KEY_FILTER_SENSOR_NUMBER:
        DEBUG ((EFI_D_INFO, "KEY_FILTER_SENSOR_NUMBER!!\n"));
        SetMem (
          &mFilteredEventLogData.EventID.SensorNum, 
          sizeof(mFilteredEventLogData.EventID.SensorNum), 
          0xFF
          );
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_SENSOR_NUM);
        SetMem (&FilterEventInfo, sizeof(FILTER_EVENT_INFO), 0xFF);
        FilterEventInfo.FilterType = TypeSensorNum;
        Status = TransferStringToData (mPrivateData->HiiHandle[1], Value, &FilterEventInfo);
        if (Status == EFI_INVALID_PARAMETER) {
          DEBUG ((EFI_D_ERROR, "Invalid Sensor Number condition.\n"));
          UnicodeSPrint (StringPtr, sizeof (StringPtr), L"Invalid Sensor Number condition.");
          ElvConfirmDialog (DlgOk, FALSE, 0, NULL, &Key, StringPtr);
          return Status;
        }
        ArrangeFilterCondition (FALSE, &FilterEventInfo);
        Status = UpdateElvStoragePage (mPrivateData->HiiHandle[1], SHOW_FIRST_PAGE);   
        break;

      case KEY_FILTER_EVENT_TYPE:
        DEBUG ((EFI_D_INFO, "KEY_FILTER_EVENT_TYPE!!\n"));
        SetMem (
          &mFilteredEventLogData.EventID.EventType, 
          sizeof(mFilteredEventLogData.EventID.EventType), 
          0xFF
          );
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_EVENT_TYPE);
        SetMem (&FilterEventInfo, sizeof(FILTER_EVENT_INFO), 0xFF);
        FilterEventInfo.FilterType = TypeEventType;
        Status = TransferStringToData (mPrivateData->HiiHandle[1], Value, &FilterEventInfo);
        if (Status == EFI_INVALID_PARAMETER) {
          DEBUG ((EFI_D_ERROR, "Invalid Event Type condition.\n"));
          UnicodeSPrint (StringPtr, sizeof (StringPtr), L"Invalid Event Type condition.");
          ElvConfirmDialog (DlgOk, FALSE, 0, NULL, &Key, StringPtr);
          return Status;
        }
        ArrangeFilterCondition (FALSE, &FilterEventInfo);
        Status = UpdateElvStoragePage (mPrivateData->HiiHandle[1], SHOW_FIRST_PAGE);   
        break;

      case KEY_FILTER_EVENT_TYPE_ID:
        DEBUG ((EFI_D_INFO, "KEY_FILTER_EVENT_TYPE_ID!!\n"));
        SetMem (
          &mFilteredEventLogData.EventTypeID, 
          sizeof(mFilteredEventLogData.EventTypeID), 
          0xFF
          );
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_EVENT_TYPE_ID);
        SetMem (&FilterEventInfo, sizeof(FILTER_EVENT_INFO), 0xFF);
        FilterEventInfo.FilterType = TypeEventTypeID;
        Status = TransferStringToData (mPrivateData->HiiHandle[1], Value, &FilterEventInfo);
        if (Status == EFI_INVALID_PARAMETER) {
          DEBUG ((EFI_D_ERROR, "Invalid Event Type ID condition.\n"));
          UnicodeSPrint (StringPtr, sizeof (StringPtr), L"Invalid Event Type ID condition.");
          ElvConfirmDialog (DlgOk, FALSE, 0, NULL, &Key, StringPtr);
          return Status;
        }
        ArrangeFilterCondition (FALSE, &FilterEventInfo);
        Status = UpdateElvStoragePage (mPrivateData->HiiHandle[1], SHOW_FIRST_PAGE);   
        break;  

      case KEY_FILTER_STRING:
        DEBUG ((EFI_D_INFO, "KEY_FILTER_STRING!!\n"));
        if (mFilterString != NULL) {
          FreePool (mFilterString);
          mFilterString = NULL;
        }
        mFilterBitMap &= ~((UINTN)B_FILTERED_BY_STRING);
        SetMem (&FilterEventInfo, sizeof(FILTER_EVENT_INFO), 0xFF);
        FilterEventInfo.FilterType = TypeString;
        Status = TransferStringToData (mPrivateData->HiiHandle[1], Value, &FilterEventInfo);
        if (Status == EFI_INVALID_PARAMETER) {
          DEBUG ((EFI_D_ERROR, "Invalid String condition.\n"));
          UnicodeSPrint (StringPtr, sizeof (StringPtr), L"Invalid String condition.");
          ElvConfirmDialog (DlgOk, FALSE, 0, NULL, &Key, StringPtr);
          return Status;
        }
        // Check the case-sensitive setting.
        UnicodeSPrint (StringPtr, sizeof (StringPtr), L"Case Sensitive?");
        ElvConfirmDialog (DlgYesNo, FALSE, 0, NULL, &Key, StringPtr);
        if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
          mCaseSensitive = TRUE;
        } else {
          mCaseSensitive = FALSE;  
        }
        
        ArrangeFilterCondition (FALSE, &FilterEventInfo);
        Status = UpdateElvStoragePage (mPrivateData->HiiHandle[1], SHOW_FIRST_PAGE);   
        break;
        
      default:
        break;
      }

    }
    break;

  case EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING:
    DEBUG ((EFI_D_INFO, "BiosEventLogConfigUtil EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING. \n"));
    break;

  case EFI_BROWSER_ACTION_DEFAULT_STANDARD:
    DEBUG ((EFI_D_INFO, "BiosEventLogConfigUtil EFI_BROWSER_ACTION_DEFAULT_STANDARD. \n"));
    break;

  case EFI_BROWSER_ACTION_RETRIEVE:
    DEBUG ((EFI_D_INFO, "BiosEventLogConfigUtil EFI_BROWSER_ACTION_RETRIEVE. \n"));
    break;

  default:
    Status = EFI_UNSUPPORTED;
    break;
  }

  return Status;
}

//[-start-180507-IB08400587-add]//
/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Request                A null-terminated Unicode string in
                                 <ConfigRequest> format.
  @param  Progress               On return, points to a character in the Request
                                 string. Points to the string's null terminator if
                                 request was successful. Points to the most recent
                                 '&' before the first failing name/value pair (or
                                 the beginning of the string if the failure is in
                                 the first name/value pair) if the request was not
                                 successful.
  @param  Results                A null-terminated Unicode string in
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
ExtractConfig2 (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  if (Progress == NULL || Results == NULL || Request == NULL) {    
    return EFI_INVALID_PARAMETER;
  }
  
  *Progress = Request;
  
  return EFI_NOT_FOUND;
}

/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Configuration          A null-terminated Unicode string in <ConfigResp>
                                 format.
  @param  Progress               A pointer to a string filled in with the offset of
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
RouteConfig2 (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

//[-start-180510-IB08400591-add]//
  *Progress = Configuration;
//[-end-180510-IB08400591-add]//

  return EFI_NOT_FOUND;
}
//[-end-180507-IB08400587-add]//

/**
  Unloads the application and its installed protocol.

  @param[in]  ImageHandle       Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
**/
EFI_STATUS
EFIAPI
SetupUnload (
  IN EFI_HANDLE  ImageHandle
  )
{

  ASSERT (mPrivateData != NULL);

  if (mEventLog != NULL) {
    FreePool (mEventLog);
    mEventLog = NULL;
  }
  if (mEventLogData != NULL) {
    FreePool  (mEventLogData);
    mEventLogData = NULL;
  }

  if (mDriverHandle[0] != NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           mDriverHandle[0],
           &gEfiDevicePathProtocolGuid,
           &mHiiVendorDevicePath0,
           &gEfiHiiConfigAccessProtocolGuid,
           &mPrivateData->ConfigAccess,
           NULL
           );
    mDriverHandle[0] = NULL;
  }

//[-start-180507-IB08400587-add]//
  if (mDriverHandle[1] != NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           mDriverHandle[1],
           &gEfiDevicePathProtocolGuid,
           &mHiiVendorDevicePath0,
           &gEfiHiiConfigAccessProtocolGuid,
           &mPrivateData->ConfigAccess2,
           NULL
           );
    mDriverHandle[1] = NULL;
  }
//[-end-180507-IB08400587-add]//

  if (mPrivateData->HiiHandle[0] != NULL) {
    HiiRemovePackages (mPrivateData->HiiHandle[0]);
  }
  
  if (mPrivateData->HiiHandle[1] != NULL) {
    HiiRemovePackages (mPrivateData->HiiHandle[1]);
  }

  FreePool (mPrivateData);
  mPrivateData = NULL;

  return EFI_SUCCESS;
}

/**
  Initialization for the Setup related functions.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to SystemTable.

  @retval EFI_SUCESS     This function always complete successfully.

**/
EFI_STATUS
EFIAPI
BiosEventLogConfigUtilDxeInitialize (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_HII_HANDLE                        HiiHandle[2];
  EFI_HII_DATABASE_PROTOCOL             *HiiDatabase;
  EFI_HII_STRING_PROTOCOL               *HiiString;
  EFI_HII_CONFIG_ROUTING_PROTOCOL       *HiiConfigRouting;
  CHAR16                                *NewString;
  UINTN                                 BufferSize;
  BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION *Configuration;
  BOOLEAN                               ActionFlag;
  EFI_STRING                            ConfigRequestHdr;

  //
  // Initialize the local variables.
  //
  ConfigRequestHdr = NULL;
  NewString        = NULL;

  //
  // Initialize driver private data
  //
  mPrivateData = AllocateZeroPool (sizeof (BIOS_EVENT_LOG_CONFIG_UTIL_PRIVATE_DATA));
  if (mPrivateData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->Signature = BIOS_EVENT_LOG_CONFIG_UTIL_PRIVATE_SIGNATURE;

  mPrivateData->ConfigAccess.ExtractConfig = ExtractConfig;
  mPrivateData->ConfigAccess.RouteConfig = RouteConfig;
  mPrivateData->ConfigAccess.Callback = DriverCallback;
//[-start-180507-IB08400587-add]//
  mPrivateData->ConfigAccess2.ExtractConfig = ExtractConfig2;
  mPrivateData->ConfigAccess2.RouteConfig = RouteConfig2;
  mPrivateData->ConfigAccess2.Callback = DriverCallback;
//[-end-180507-IB08400587-add]//

  //
  // Locate Hii Database protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiDatabase = HiiDatabase;

  //
  // Locate HiiString protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiStringProtocolGuid, NULL, (VOID **) &HiiString);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiString = HiiString;

  //
  // Locate ConfigRouting protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &HiiConfigRouting);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiConfigRouting = HiiConfigRouting;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mDriverHandle[0],
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath0,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mPrivateData->ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  mPrivateData->DriverHandle[0] = mDriverHandle[0];

//[-start-180507-IB08400587-modify]//
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mDriverHandle[1],
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath1,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mPrivateData->ConfigAccess2,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  mPrivateData->DriverHandle[1] = mDriverHandle[1];
//[-end-180507-IB08400587-modify]//
  
  //
  // Publish our HII data
  //
  HiiHandle[0] = HiiAddPackages (
                   &gH2OBiosEventLogConfigUtilConfigFormsetGuid,
                   mDriverHandle[0],
                   BiosEventLogConfigUtilDxeStrings,
                   BiosEventLogConfigUtilConfigPageVfrBin,
                   NULL
                   );
  if (HiiHandle[0] == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->HiiHandle[0] = HiiHandle[0];

  //
  // Publish our HII data
  //
  HiiHandle[1] = HiiAddPackages (
                   &gH2OBiosEventLogConfigUtilEventAndMessageFormsetGuid,
                   mDriverHandle[1],
                   BiosEventLogConfigUtilDxeStrings,
                   BiosEventLogConfigUtilEventAndMessagePageVfrBin,
                   NULL
                   );
  if (HiiHandle[1] == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->HiiHandle[1] = HiiHandle[1];
  
  //
  // Initialize configuration data
  //
  Configuration = &mPrivateData->Configuration;
  ZeroMem (Configuration, sizeof (BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION));

  //
  // Try to read NV config EFI variable first
  //
  ConfigRequestHdr = HiiConstructConfigHdr (&gH2OBiosEventLogConfigUtilVarstoreGuid, mVariableName, mDriverHandle[0]);
  ASSERT (ConfigRequestHdr != NULL);
  if (ConfigRequestHdr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  DEBUG ((EFI_D_INFO, "ConfigRequestHdr: %s\n", ConfigRequestHdr));

  BufferSize = sizeof (BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION);
  Status = gRT->GetVariable (mVariableName, &gH2OBiosEventLogConfigUtilVarstoreGuid, NULL, &BufferSize, Configuration);
  if (EFI_ERROR (Status)) {
    //
    // Store zero data Buffer Storage to EFI variable
    //
    Status = CommonSetVariable (
               mVariableName,
               &gH2OBiosEventLogConfigUtilVarstoreGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
               sizeof (BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION),
               Configuration
               );
    if (EFI_ERROR (Status)) {
      SetupUnload (ImageHandle);
      return Status;
    }
    //
    // EFI variable for NV config doesn't exit, we should build this variable
    // based on default values stored in IFR
    //

    ActionFlag = HiiSetToDefaults (ConfigRequestHdr, EFI_HII_DEFAULT_CLASS_STANDARD);
    if (!ActionFlag) {
      SetupUnload (ImageHandle);
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    // EFI variable does exist and Validate Current Setting
    //

    ActionFlag = HiiValidateSettings (ConfigRequestHdr);
    if (!ActionFlag) {
      SetupUnload (ImageHandle);
      return EFI_INVALID_PARAMETER;
    }
  }
  FreePool (ConfigRequestHdr);

  ConfigRequestHdr = HiiConstructConfigHdr (&gH2OBiosEventLogConfigUtilVarstoreGuid, mVariableNameForFilter, mDriverHandle[1]);
  ASSERT (ConfigRequestHdr != NULL);

  HiiSetToDefaults (ConfigRequestHdr, EFI_HII_DEFAULT_CLASS_STANDARD);
  FreePool (ConfigRequestHdr);
  
  return EFI_SUCCESS;
}

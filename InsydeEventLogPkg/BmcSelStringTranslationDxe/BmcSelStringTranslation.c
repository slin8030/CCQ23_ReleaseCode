/** @file

  BMC SEL Event String implementation.

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

#include <BmcSelStringTranslation.h>

H2O_EVENT_LOG_STRING_PROTOCOL           gEventLogString = {
  EventLogStringHandler
  };

BMC_SEL_TRANSLATION_SKIP_TABLE          gBmcSelTranslationSkipTable[] = {
//                                              0x0C, 0x00, 0x70, TRUE,     // Memory ECC Event
//                                              0x07, 0x00, 0x70, TRUE,     // Machine Check Architecture Event
//                                              0x13, 0x00, 0x70, TRUE,     // PCIe Event
//                                              0xC0, 0x00, 0x70, TRUE      // IIO Event
                                                0xff, 0xff, 0xff, TRUE
                                                        };

/**
 Entry point of this driver. Install H2O Event Log String protocol into DXE.

 @param[in] ImageHandle       Image handle of this driver.
 @param[in] SystemTable       Global system service table.          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BmcSelStringTranslationInitEntry (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                                    Status;  
  EFI_HANDLE                                    Handle;

  Handle = NULL;
  //
  // Install Event String Protocol for user to translate event message
  //
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gH2OEventLogStringProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &gEventLogString
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS ;
}

/**
 Handler of Event Log String.

 @param[in]   EventInfo     Information of BMC SEL event log data.
 @param[out]  EventString   Returned string of the input BMC SEL event log data.
 @param[out]  StringSize    Size of EventString.
 
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
  EFI_STATUS                             Status=EFI_NOT_FOUND;
  STORAGE_EVENT_LOG_INFO                 *StorageEventInfo;

  StorageEventInfo = (STORAGE_EVENT_LOG_INFO *) EventInfo;
  Status = GetBmcSelString (StorageEventInfo, EventString, StringSize);

  return Status;
    
}

/**
 Get the string form BMC SEL
 
 @param[in]   EventInfo     Information of BMC SEL event log data.
 @param[out]  EventString   Returned string of the input BMC SEL event log data.
 @param[out]  StringSize    Size of EventString.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
GetBmcSelString (
  IN STORAGE_EVENT_LOG_INFO                 *EventInfo,
  IN OUT CHAR16                             **EventString,
  IN OUT UINTN                              *StringSize
  )
{

  EFI_STATUS                    Status;
//[-start-170929-IB08400459-remove]//
//  H2O_IPMI_SEL_INFO_PROTOCOL    *SelInfo;
//[-end-170929-IB08400459-remove]//
  SEL_INFO_STRUCT               SelInfoData;
  VOID                          *TempString=NULL;
  BOOLEAN                       GetSelString=FALSE;
//[-start-170929-IB08400459-modify]//
  H2O_IPMI_SEL_ENTRY_INPUT      SelEntry;
//[-end-170929-IB08400459-modify]//

  if (SkipTranslationSel (EventInfo)) {
    //
    // Skip this SEL type let other handler to translate.
    //
    return EFI_NOT_FOUND;
  }
    
//[-start-170929-IB08400459-remove]//
//  Status = gBS->LocateProtocol (&gH2OIpmiSelInfoProtocolGuid, NULL, (VOID **)&SelInfo);
//  if (EFI_ERROR (Status)) {
//    return Status;
//  }
//[-end-170929-IB08400459-remove]//

  SelEntry.Data.RecordId     = EventInfo->RecordId;
  SelEntry.Data.RecordType   = EventInfo->RecordType;
  
  SelEntry.Data.SensorType   = EventInfo->EventID.SensorType;
  SelEntry.Data.SensorNum    = EventInfo->EventID.SensorNum;
  SelEntry.Data.EventType    = (EventInfo->EventID.EventType & 0x7F);
  SelEntry.Data.EventDir     = (EventInfo->EventID.EventType >> 7) & 0x01;
  SelEntry.Data.EventData1   = EventInfo->Data[0];
  SelEntry.Data.EventData2   = EventInfo->Data[1];
  SelEntry.Data.EventData3   = EventInfo->Data[2];

//[-start-170929-IB08400459-modify]//
  Status = IpmiLibGetSelInfo ((VOID*)(UINTN)&(SelEntry.Data.RecordId), (VOID*)(UINTN)&SelInfoData);  
//[-end-170929-IB08400459-modify]//
  if (!EFI_ERROR (Status)) {
    TempString = AllocateZeroPool (MAX_UPDATE_DATA_SIZE);
    if (TempString==NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    
//[-start-161109-IB04560701-modify]//
//[-start-180724-IB08400617-modify]//
    if (StrLen (SelInfoData.SensorName)!=0) {      
      StrCatS (TempString, MAX_UPDATE_DATA_SIZE, SelInfoData.SensorName);       
      StrCatS (TempString, MAX_UPDATE_DATA_SIZE, L" ");
      *StringSize = (StrLen (SelInfoData.SensorName) * 2)+1;
      GetSelString = TRUE;
    }
  
    if (StrLen (SelInfoData.Type)!=0) {
      StrCatS (TempString, MAX_UPDATE_DATA_SIZE, SelInfoData.Type);      
      StrCatS (TempString, MAX_UPDATE_DATA_SIZE, L" ");            
      *StringSize = (StrLen (SelInfoData.Type) * 2)+1;
      GetSelString = TRUE;
    }
  
    if (StrLen (SelInfoData.Desc)!=0) {
      StrCatS (TempString, MAX_UPDATE_DATA_SIZE, SelInfoData.Desc);      
      StrCatS (TempString, MAX_UPDATE_DATA_SIZE, L" ");      
      *StringSize = (StrLen (SelInfoData.Desc) * 2)+1;
      GetSelString = TRUE;
    }    
//[-end-180724-IB08400617-modify]//
//[-end-161109-IB04560701-modify]//
    
    *EventString = TempString;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

/**
 Skip this SEL type let other handler to translate.

 @param[in]         EventInfo                           
 
 @retval BOOLEAN                 
*/
BOOLEAN
EFIAPI
SkipTranslationSel (
  IN STORAGE_EVENT_LOG_INFO                     *EventInfo
  )   
{
  UINTN                                     Index;
  
  for (Index = 0; Index < sizeof (gBmcSelTranslationSkipTable)/sizeof(BMC_SEL_TRANSLATION_SKIP_TABLE); Index++) {
    if ((EventInfo->EventID.SensorType   == gBmcSelTranslationSkipTable[Index].BmcSensorType) &&
        (EventInfo->EventID.SensorNum    == gBmcSelTranslationSkipTable[Index].BmcSensorNum) &&
        (EventInfo->EventID.EventType    == gBmcSelTranslationSkipTable[Index].BmcEventType)) {
        if (gBmcSelTranslationSkipTable[Index].SkipTranslation) {
          //
          // Skip this SEL type let other handler to translate.
          //
          return TRUE;
        }
    }
  }

  return FALSE;
}


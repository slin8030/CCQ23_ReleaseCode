/** @file
 EventLogSmm Lib functions which relate with connect the device

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

*/

/**
  This constructor function caches the EVENT_LOG_PROTOCOL pointer.

  @param[in] ImageHandle The firmware allocated handle for the EFI image.
  @param[in] SystemTable A pointer to the EFI System Table.

  @retval EFI_SUCCESS The constructor always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
EventLogSmmConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}

/**
 Log event data and event ID to the specific storage.           

 @param[in] EventStorageType        Storage type which will log event data into.
 @param[in] EventID                 ID for specific event. Prototype: EVENT_TYPE_ID
 @param[in] ErrorData               Event data.
 @param[in] ErrorDataSize           Event Data Size.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
LogEventLib (
  IN  UINT8                            EventStorageType,
  IN  VOID                             *EventID,
  IN  VOID                             *ErrorData,  
  IN  UINTN                            ErrorDataSize
  )
{  
  return EFI_UNSUPPORTED;
}

/**
 Get event data from the specific storage.
            
 @param[in] StorageString           Storage type which will get event data from.
 @param[in] Index                   Index of data that in selected storage.
 @param[in] Data                    Event data in the selected storage.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
GetEventLib (
  IN      CHAR16                           *StorageString,
  IN      UINTN                            DataIndex,
  IN OUT  UINT8                            **Data
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Get event count from the selected storage.
            
 @param[in]  StorageString        Storage type which will log event data into.
 @param[out] EventLogCount        Event count of the selected storage.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
GetEventCountLib (
  IN      CHAR16                           *StorageString,
  IN OUT  UINTN                            *EventLogCount  
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Clear event log data from the selected storage.

 @param[in]         
 StorageString        Storage type which logged event data into.          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
ClearEventLib (
  IN      CHAR16                           *StorageString
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Update event data of the selected storage.
            
 @param[in]  StorageString        Storage type which will log event data into.
 @param[out] EventLogCount        Event count of the selected storage.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
RefreshDatabaseLib (
  IN      CHAR16                           *StorageString,
  IN OUT  UINTN                            *EventLogCount  
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Get all the names of event storage drivers.
          
 @param[out] StorageName        Storage names.
 @param[out] StorageNum         Number of storage drivers.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
GetEventStorageNameLib (
  IN OUT  CHAR16                           **StorageName,
  IN OUT  UINTN                            *StorageNum
  )
{
  return EFI_UNSUPPORTED;
}

//[-start-150915-IB08400295-add]//
/**
 Get all the names of event storage drivers.
          
 @param[in] Number        Variable number for logging.
 @param[in] SetToView     Set if the log needs to show in Viewer. 
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
SetBiosStorageLogLib (
  IN    UINT16                           Number,
  IN    BOOLEAN                          SetToView
  )
{
  return EFI_UNSUPPORTED;
}
//[-end-150915-IB08400295-add]//

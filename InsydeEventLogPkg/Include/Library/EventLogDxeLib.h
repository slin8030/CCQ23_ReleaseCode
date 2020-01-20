/** @file

  Header file of Event Log DXE Lib implementation.

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

#ifndef _EVENT_LOG_DXE_LIB_H_
#define _EVENT_LOG_DXE_LIB_H_

/**
 Log event data and event ID to the specific storage.           

 @param[in] EventStorageType        Storage type which will log event data into.
 @param[in] EventID                 ID for specific event.
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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

#endif

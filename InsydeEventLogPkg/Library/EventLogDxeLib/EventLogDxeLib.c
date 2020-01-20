/** @file
 EventLogDxe Lib functions which relate with connect the device

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
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


#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/EventLogDxeLib.h>
#include <Library/BaseLib.h>
#include <Library/VariableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/EventLog.h>
#include <Guid/BiosStorageVariable.h>

EVENT_LOG_PROTOCOL  *mEventLogDxe=NULL;

/**
  This constructor function caches the EVENT_LOG_PROTOCOL pointer.

  @param[in] ImageHandle The firmware allocated handle for the EFI image.
  @param[in] SystemTable A pointer to the EFI System Table.

  @retval EFI_SUCCESS The constructor always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
EventLogDxeConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status=EFI_UNSUPPORTED;

  if (gBS==NULL) {
    return EFI_UNSUPPORTED;
  }
  
  Status = gBS->LocateProtocol (&gH2OEventLogProtocolGuid, NULL, (VOID **)&mEventLogDxe);
//  if (EFI_ERROR (Status)) {
//    return Status;
//  }

  if (FeaturePcdGet (PcdH2OBiosEventStorageUseVariableStorageSupported) == TRUE) {
    SetBiosStorageLogLib (0, TRUE);
  }
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
  EFI_STATUS                   Status=EFI_UNSUPPORTED;

  if (mEventLogDxe==NULL) {
    //
    // Locate Protocol again to aviod dependence problem.
    //    
    Status = gBS->LocateProtocol (&gH2OEventLogProtocolGuid, NULL, (VOID **)&mEventLogDxe);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = mEventLogDxe->LogEvent(EventStorageType, EventID, ErrorData, ErrorDataSize);
  
  return Status;
  
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
  EFI_STATUS                   Status=EFI_UNSUPPORTED;

  if (mEventLogDxe==NULL) {
    //
    // Locate Protocol again to aviod dependence problem.
    //    
    Status = gBS->LocateProtocol (&gH2OEventLogProtocolGuid, NULL, (VOID **)&mEventLogDxe);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = mEventLogDxe->GetEvent(StorageString, DataIndex, Data);
  
  return Status;
  
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
  EFI_STATUS                   Status=EFI_UNSUPPORTED;

  if (mEventLogDxe==NULL) {
    //
    // Locate Protocol again to aviod dependence problem.
    //    
    Status = gBS->LocateProtocol (&gH2OEventLogProtocolGuid, NULL, (VOID **)&mEventLogDxe);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = mEventLogDxe->GetEventCount(StorageString, EventLogCount);
  
  return Status;
  
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
  EFI_STATUS                   Status=EFI_UNSUPPORTED;

  if (mEventLogDxe==NULL) {
    //
    // Locate Protocol again to aviod dependence problem.
    //    
    Status = gBS->LocateProtocol (&gH2OEventLogProtocolGuid, NULL, (VOID **)&mEventLogDxe);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = mEventLogDxe->ClearStorage(StorageString);
  
  if (FeaturePcdGet (PcdH2OBiosEventStorageUseVariableStorageSupported) == TRUE) {
    SetBiosStorageLogLib (0, TRUE);
  }
  return Status;
  
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
  EFI_STATUS                   Status=EFI_UNSUPPORTED;

  if (mEventLogDxe==NULL) {
    //
    // Locate Protocol again to aviod dependence problem.
    //    
    Status = gBS->LocateProtocol (&gH2OEventLogProtocolGuid, NULL, (VOID **)&mEventLogDxe);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = mEventLogDxe->RefreshDatabase(StorageString, EventLogCount);
  
  return Status;
  
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
  EFI_STATUS                   Status=EFI_UNSUPPORTED;

  if (mEventLogDxe==NULL) {
    //
    // Locate Protocol again to aviod dependence problem.
    //    
    Status = gBS->LocateProtocol (&gH2OEventLogProtocolGuid, NULL, (VOID **)&mEventLogDxe);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = mEventLogDxe->GetStorageName((EVENT_STORAGE_INFO**)StorageName, StorageNum);
  
  return Status;
  
}

/**
 Get all the names of event storage drivers.
          
 @param[in] NewVarNum        Variable number for logging.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
SetNumberToViewer (
  IN    UINT16                           NewVarNum
  )
{
  EFI_STATUS                             Status;
  UINT16                                 *BiosElViewOrder;
  UINTN                                  BiosElViewOrderSize;
  UINT16                                 *NewBiosElViewOrder;
  UINTN                                  NewBiosElViewOrderSize;
  UINTN                                  Index;

  Status = CommonGetVariableDataAndSize (
             BIOS_STORAGE_LOG_VIEW_ORDER_VAR_NAME,
             &gH2OBiosStorageVariableGuid,
             &BiosElViewOrderSize,
             (VOID **) &BiosElViewOrder
             );
  if (EFI_ERROR (Status) || (BiosElViewOrder == NULL)) {
    //
    // No any record, just save the new variable number as the first one.
    //  
    BiosElViewOrderSize = sizeof(UINT16);
    BiosElViewOrder = AllocateZeroPool (BiosElViewOrderSize);
    if (BiosElViewOrder == NULL) {
      return EFI_OUT_OF_RESOURCES;
    } else {
      BiosElViewOrder[0] = NewVarNum;
    }

    Status = CommonSetVariable (
               BIOS_STORAGE_LOG_VIEW_ORDER_VAR_NAME,
               &gH2OBiosStorageVariableGuid,
               EFI_VARIABLE_NON_VOLATILE  | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
               BiosElViewOrderSize,
               BiosElViewOrder
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "SetVariable %s failed: %r\n", BIOS_STORAGE_LOG_VIEW_ORDER_VAR_NAME, Status));
    }
    
    gBS->FreePool (BiosElViewOrder);
    return Status;
  } else {
    //
    // Check if the number is already in the order.
    //
    for (Index = 0; Index < (BiosElViewOrderSize/sizeof (UINT16)); Index++) {
      if (BiosElViewOrder[Index] == NewVarNum) {
        //
        // Variable number has already been added.
        //
        gBS->FreePool (BiosElViewOrder);
        return Status;
      }
    }

    //
    // Get original BiosElViewOrder content, and update new variable number to it.
    //
    NewBiosElViewOrderSize = BiosElViewOrderSize + sizeof(UINT16);
    NewBiosElViewOrder = AllocateZeroPool (NewBiosElViewOrderSize);
    if (NewBiosElViewOrder == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      CopyMem (NewBiosElViewOrder, BiosElViewOrder, BiosElViewOrderSize);
      NewBiosElViewOrder[BiosElViewOrderSize/2] = NewVarNum;

      Status = CommonSetVariable (
                 BIOS_STORAGE_LOG_VIEW_ORDER_VAR_NAME,
                 &gH2OBiosStorageVariableGuid,
                 EFI_VARIABLE_NON_VOLATILE  | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 NewBiosElViewOrderSize,
                 NewBiosElViewOrder
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "SetVariable %s failed: %r\n", BIOS_STORAGE_LOG_VIEW_ORDER_VAR_NAME, Status));
      }
      gBS->FreePool (NewBiosElViewOrder);
    }
    gBS->FreePool (BiosElViewOrder);
  }
  
  return Status;
}

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
  EFI_STATUS                   Status = EFI_UNSUPPORTED;
  UINT16                       LogNumber;

  if (!FeaturePcdGet (PcdH2OBiosEventStorageUseVariableStorageSupported)) {
    Status = EFI_UNSUPPORTED; 
  } else {
    LogNumber = Number;
    Status = CommonSetVariable (
               BIOS_STORAGE_LOG_NUMBER_VAR_NAME,
               &gH2OBiosStorageVariableGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
               sizeof(LogNumber),
               &LogNumber
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "SetVariable %s failed: %r\n", BIOS_STORAGE_LOG_NUMBER_VAR_NAME, Status));
    } else {
      if (SetToView == TRUE) {
        SetNumberToViewer (Number);
      }
      AsmWbinvd ();
    }
  }
  
  return Status;
  
}

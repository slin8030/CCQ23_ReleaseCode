/** @file
  Implementation of EventLogPei module.

;******************************************************************************
;* Copyright (c) 2014-2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <EventLogPei.h>

static H2O_PEI_EVENT_LOG_PPI mPeiEventLogPpi = {
  PeiLogEvent
};

//[-start-171212-IB08400542-modify]//
EFI_PEI_PPI_DESCRIPTOR mPpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gH2OEventLogPpiGuid,
  &mPeiEventLogPpi
};
//[-end-171212-IB08400542-modify]//

EFI_PEI_NOTIFY_DESCRIPTOR mReadOnlyVariable2PpiNotify = {
  (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_DISPATCH | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiReadOnlyVariable2PpiGuid,
  ReadOnlyVariable2Callback
};

/**
 Log event data in PEI phase by Pei Event Log PPI. 
         
 @param[in] PeiServices     Efi Pei services.
 @param[in] EventID         Event ID of the event data.
 @param[in] DataSize        Size of event data.
 @param[in] *LogData        Event data which will be logged.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
PeiLogEvent (
  IN  CONST EFI_PEI_SERVICES                  **PeiServices,
  IN  EVENT_TYPE_ID                           EventID,
  IN  UINTN                                   DataSize,
  IN  UINT8                                   *LogData
  )
{
  EFI_STATUS                          Status;
  H2O_PEI_BMC_SEL_STORAGE_PPI         *PeiBmcSelStorage;

//[-start-171212-IB08400542-modify]//
  if (PcdGet8(PcdH2OEventLogTarget) == EVENT_STORAGE_DISABLED) {
    return EFI_UNSUPPORTED;
  }   
//[-end-171212-IB08400542-modify]//

  //
  // Log to BMC SEL.
  //
//[-start-171212-IB08400542-modify]//
  Status = (**PeiServices).LocatePpi(
                                  (CONST EFI_PEI_SERVICES **)PeiServices,
                                  &gH2OBmcSelStoragePpiGuid,
                                  0,
                                  NULL,
                                  (VOID **)&PeiBmcSelStorage
                                  );
//[-end-171212-IB08400542-modify]//
  if (!EFI_ERROR(Status)) {
    Status = PeiBmcSelStorage->LogEvent (PeiServices, EventID, DataSize, LogData);
//[-start-161109-IB04560705-add]//
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Event Log PEI PeiBmcSelStorage->LogEvent() failed: %r\n", Status));
    }  
//[-end-161109-IB04560705-add]//
  }
  
  //
  // Create a GUID hob(PeiEventLogHob) with the event data.
  //
  Status = PeiEventLogCreateHob (PeiServices, EventID, DataSize, LogData);

  return Status;
}

/**
 Log event data to BMC SEL.           

 @param[in] PeiServices     Efi Pei services.
 @param[in] EventID         Event ID of logged data.
 @param[in] DataSize        Size of event data.
 @param[in] *LogData        Event data which will be logged.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
PeiEventLogCreateHob (
  IN  CONST EFI_PEI_SERVICES                  **PeiServices,
  IN  EVENT_TYPE_ID                           EventID,
  IN  UINTN                                   DataSize,
  IN  UINT8                                   *Data
  )
{
  EFI_STATUS                          Status;
  PEI_EVENT_LOG_ORGANIZATION          Log;
  UINT32                              NumBytes;
  UINTN                               Index;
  VOID                                *Hob;
  EL_TIME                             ElTime;

  Status = EFI_SUCCESS;
  
  if (DataSize > (sizeof(Log.Data) - sizeof(EVENT_TYPE_ID))) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  NumBytes = PEI_EVENT_LOG_BASE_LENGTH + sizeof(EVENT_TYPE_ID) + DataSize;

  Status = EventLogGetTimeFromCmos (&ElTime);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  Log.Year   = ElTime.Year;
  Log.Month  = ElTime.Month;
  Log.Day    = ElTime.Day;
  Log.Hour   = ElTime.Hour;
  Log.Minute = ElTime.Minute;
  Log.Second = ElTime.Second;

  Log.Type   = 0x80;
  Log.Length = (UINT8)NumBytes;

  Log.Data[0] = EventID.SensorType;
  Log.Data[1] = EventID.SensorNum;
  Log.Data[2] = EventID.EventType;

  for (Index = 0; Index < DataSize; Index++) {
    Log.Data[3 + Index] = Data[Index];
  }

  Hob = BuildGuidDataHob (
                      &gH2OPeiEventLogHobGuid,
                      &Log,
                      NumBytes
                      );
  if (Hob == NULL) {
    Status = EFI_UNSUPPORTED;
  }
  return Status;

}

/**
 Update BIOS Event Log config to related PCD.

 @param[in]  VariablepPpi         A pointer to Variable service struct pointer.

 @retval EFI_SUCCESS              Update Policy success.
 @return EFI_ERROR                Get Variable error.
*/
EFI_STATUS
UpdateBiosEventLogConfig (
  IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI       *VariablePpi
  )
{
  EFI_STATUS                                Status;
  EFI_GUID                                  BiosEventLogConfigGuid = H2O_BIOS_EVENT_LOG_CONFIG_UTIL_VARSTORE_GUID;
  UINTN                                     VariableSize;
  CHAR16                                    *VariableName = H2O_BIOS_EVENT_LOG_CONFIG_UTIL_VARSTORE_NAME;
  BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION  BiosEventLogConfig;
    
  VariableSize = sizeof(BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION);
  Status = VariablePpi->GetVariable (
                          VariablePpi,
                          VariableName,
                          &BiosEventLogConfigGuid,
                          NULL,
                          &VariableSize,
                          &BiosEventLogConfig
                          );
  if (!EFI_ERROR (Status)) {
//[-start-171212-IB08400542-modify]//
//[-start-180718-IB08400617-modify]//
//[-start-180823-IB07400999-modify]//
    Status = PcdSet8 (PcdH2OEventLogTarget,         BiosEventLogConfig.LogEventTo);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "Setu PcdH2OEventLogTarget failure: %s : %r\n", Status));
      return Status;
    }
    Status = PcdSet8 (PcdH2OEventLogFullOption,     BiosEventLogConfig.EventLogFullOption);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "Setu PcdH2OEventLogFullOption failure: %s : %r\n", Status));
      return Status;
    }
//[-end-180823-IB07400999-modify]//
//[-end-180718-IB08400617-modify]//	
//[-end-171212-IB08400542-modify]//
  } else {
    DEBUG((EFI_D_ERROR, "UpdateBiosEventLogConfig() GetVariable %s : %r\n", VariableName, Status));   
  }

  return Status;
}

/**
  Callback function for the notification of ReadOnlyVariable2 PPI.

  @param[in]         PeiServices         A pointer to EFI_PEI_SERVICES struct pointer.

  @retval EFI_SUCCESS                    Update Policy success.
  @return EFI_ERROR                      Locate gEfiPeiReadOnlyVariable2PpiGuid or execute GetVariable error.
*/
EFI_STATUS
ReadOnlyVariable2Callback (
  IN EFI_PEI_SERVICES               **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR      *NotifyDescriptor,
  IN VOID                           *Ppi
  )
{
  EFI_STATUS                            Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI       *VariablePpi;
  
  Status = (*PeiServices)->LocatePpi (PeiServices, &gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **)&VariablePpi);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = UpdateBiosEventLogConfig (VariablePpi);

  return Status;
}

/**
  Perform PEI Event Log Init.

  @param [in] FileHandle           Handle of the file being invoked. 
  @param [in] PeiServices          General purpose services available to every PEIM.

  @retval EFI Status            
**/
EFI_STATUS
EFIAPI
EventLogPeiEntryPoint ( 
  IN EFI_PEI_FILE_HANDLE        FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                            Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI       *VariablePpi;

  Status = (*PeiServices)->LocatePpi (PeiServices, &gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **)&VariablePpi);
  if (EFI_ERROR (Status)) {
    //
    // Register callback function for Read Only Variable2 PPI.
    //
    Status = (**PeiServices).NotifyPpi (PeiServices, &mReadOnlyVariable2PpiNotify); 
    if (EFI_ERROR (Status)) {      
      DEBUG((EFI_D_ERROR, "Can not register callback function in Event Log PEI: %r\n", Status));   
    }
  } else {
    Status = UpdateBiosEventLogConfig (VariablePpi);
    if (EFI_ERROR (Status)) {      
      DEBUG((EFI_D_ERROR, "UpdateBiosEventLogConfig: %r\n", Status));   
    }
  }

  Status = (**PeiServices).InstallPpi (PeiServices, &mPpiList);

  return Status;
}


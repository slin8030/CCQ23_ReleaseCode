/** @file
  Implementation of BmcSelStoragePei module.

;******************************************************************************
;* Copyright (c) 2014 - 2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <BmcSelStoragePei.h>

static H2O_PEI_BMC_SEL_STORAGE_PPI mPeiBmcSelStoragePpi = {
                                                     LogEvent
                                                     };

//[-start-171212-IB08400542-modify]//
static EFI_PEI_PPI_DESCRIPTOR mPpiList = {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gH2OBmcSelStoragePpiGuid,
    &mPeiBmcSelStoragePpi
};
//[-end-171212-IB08400542-modify]//

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
LogEvent (
  IN  CONST EFI_PEI_SERVICES                  **PeiServices,
  IN  EVENT_TYPE_ID                           EventID,
  IN  UINTN                                   DataSize,
  IN  UINT8                                   *LogData
  )
{
  EFI_STATUS                                Status;
//[-start-170929-IB08400459-remove]//
//  H2O_IPMI_INTERFACE_PPI                    *IpmiPpi;
//[-end-170929-IB08400459-remove]//
  BOOLEAN                                   LogFull = FALSE;
  BOOLEAN                                   DeleteEntrySupport = FALSE;

  if (DataSize > MAX_BMC_SEL_LOGGED_DATA_SIZE) {
    return EFI_OUT_OF_RESOURCES;
  }
  
//[-start-170929-IB08400459-remove]//
//  Status = (**PeiServices).LocatePpi (
//                                  PeiServices,
//                                  &gH2OIpmiInterfacePpiGuid,
//                                  0,
//                                  NULL,
//                                  (VOID **)&IpmiPpi
//                                  );
//
//  if (EFI_ERROR (Status)) {
//    return Status;
//  }
//[-end-170929-IB08400459-remove]//

  Status = CheckBmcSelEventLogFull (PeiServices, &LogFull, &DeleteEntrySupport);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((LogFull == TRUE)) {
    if (DeleteEntrySupport == TRUE) {
      ShiftDataOfBmcSel (PeiServices);
    } else {
      //
      // BMC SEL is full and not support delete entry,
      // so that we can not log event anymore.
      //
      return EFI_OUT_OF_RESOURCES;
    }

  }
  
  Status = LogDataToBmcSel (PeiServices, EventID, DataSize, LogData);

  return EFI_SUCCESS;

}

/**
 Check if the storage is full.
 
 @param[in]  PeiServices     Efi Pei services.
 @param[out] LogFull         Status of the storage.
 @param[out] DelEntrySupport Support of "Delete Entry" cmd.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
CheckBmcSelEventLogFull (
  IN CONST EFI_PEI_SERVICES         **PeiServices,
  IN OUT BOOLEAN                    *LogFull,
  IN OUT BOOLEAN                    *DelEntrySupport
  )
{
  EFI_STATUS                            Status = EFI_SUCCESS;
//[-start-170929-IB08400459-remove]//
//  H2O_IPMI_INTERFACE_PPI                *IpmiPpi;
//[-end-170929-IB08400459-remove]//
  H2O_IPMI_SEL_INFO                     *IpmiSelInfo;
  UINT8                                 RecvSize = MAX_BUFFER_SIZE;
  UINT8                                 RecvBuf[MAX_BUFFER_SIZE];

//[-start-170929-IB08400459-remove]//
//  Status = (**PeiServices).LocatePpi (
//                                  PeiServices,
//                                  &gH2OIpmiInterfacePpiGuid,
//                                  0,
//                                  NULL,
//                                  (VOID **)&IpmiPpi
//                                  );
//  if(EFI_ERROR (Status)) {
//    return Status;
//  }
//[-end-170929-IB08400459-remove]//
  
  RecvSize = MAX_BUFFER_SIZE;
//[-start-170929-IB08400459-modify]//
  Status = GetBmcSelInfo (PeiServices, RecvBuf, &RecvSize);
//[-end-170929-IB08400459-modify]//
  if (EFI_ERROR (Status)) {
    return Status;
  }
  IpmiSelInfo = (H2O_IPMI_SEL_INFO *)RecvBuf;

  //
  // Use the free space to check if the storage is full.
  //
  if (IpmiSelInfo->FreeSpace < 16) {
    *LogFull = TRUE;
  } else {
    *LogFull = FALSE;
  }

  
  if ((IpmiSelInfo->OperationSupport & DELETE_SEL_CMD_SUPPORTED) == 0) {
    //
    // Delete SEL cmd not supported.
    //
    *DelEntrySupport = FALSE;
  } else {
    *DelEntrySupport = TRUE;
  }

  return EFI_SUCCESS;
  
}

//[-start-170929-IB08400459-modify]//
/**
 Excute IPMI CMD to get BMC SEL information. 
            
 @param[in] PeiServices     Efi Pei services.
 @param[in] RecvBuf         BmcSel information.
 @param[in] RecvSize
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
GetBmcSelInfo (
  IN CONST EFI_PEI_SERVICES       **PeiServices,
  IN UINT8                        *RecvBuf,
  IN UINT8                        *RecvSize
)
//[-end-170929-IB08400459-modify]//
{
  EFI_STATUS                      Status;
//[-start-170929-IB08400459-remove]//
//  H2O_IPMI_CMD_HEADER             Request = { H2O_IPMI_BMC_LUN,
//                                              H2O_IPMI_NETFN_STORAGE,
//                                              H2O_IPMI_CMD_GET_SEL_INFO
//                                              };
//[-end-170929-IB08400459-remove]//

//[-start-170929-IB08400459-modify]//
  Status = IpmiLibExecuteIpmiCmd (
             H2O_IPMI_NETFN_STORAGE,
             H2O_IPMI_CMD_GET_SEL_INFO,
             NULL,
             0,
             (VOID*)(UINTN)RecvBuf,
             RecvSize
             );
//[-end-170929-IB08400459-modify]//
  return Status;
}

/**
 Shift event data of the storage.

 @param[in]         PeiServices     Efi Pei services.           
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
ShiftDataOfBmcSel (
  IN CONST EFI_PEI_SERVICES             **PeiServices
  )
{
  EFI_STATUS                            Status;
  UINT8                                 RecvSize = MAX_BUFFER_SIZE;
  UINT8                                 RecvBuf[MAX_BUFFER_SIZE];
  UINT8                                 ResvId[2];
//[-start-170929-IB08400459-remove]//
//  H2O_IPMI_INTERFACE_PPI                *IpmiPpi;
//[-end-170929-IB08400459-remove]//
//[-start-170929-IB08400459-remove]//
//  H2O_IPMI_CMD_HEADER                   Request = {H2O_IPMI_BMC_LUN,
//                                                   H2O_IPMI_NETFN_STORAGE,
//                                                   H2O_IPMI_CMD_RESERVE_SEL_ENTRY
//                                                   };
//[-end-170929-IB08400459-remove]//
  UINT8                                 CmdBuf[4];
  UINTN                                 Index;

//[-start-170929-IB08400459-remove]//
//  Status = (**PeiServices).LocatePpi (
//                                  PeiServices,
//                                  &gH2OIpmiInterfacePpiGuid,
//                                  0,
//                                  NULL,
//                                  (VOID **)&IpmiPpi
//                                  );
//  if (EFI_ERROR (Status)) {
//    return Status;
//  }
//[-end-170929-IB08400459-remove]//

  RecvSize = MAX_BUFFER_SIZE;

  for (Index = 0; Index < EVENT_LOG_FULL_ADJUST_EVENT_NUM; Index++) {    
    //
    // Delete SEL Entry
    //
//[-start-170929-IB08400459-modify]//
    Status = IpmiLibExecuteIpmiCmd (
               H2O_IPMI_NETFN_STORAGE,
               H2O_IPMI_CMD_RESERVE_SEL_ENTRY,
               NULL,
               0,
               ResvId,
               &RecvSize
               );
//[-end-170929-IB08400459-modify]//
    if (EFI_ERROR(Status)) {
      return Status;
    }
//[-start-170929-IB08400459-remove]//
//    Request.Cmd = H2O_IPMI_CMD_DELETE_SEL_ENTRY;
//[-end-170929-IB08400459-remove]//
    CmdBuf[0] = ResvId[0];
    CmdBuf[1] = ResvId[1];
    CmdBuf[2] = 0;
    CmdBuf[3] = 0;

//[-start-170929-IB08400459-modify]//
//[-start-171228-IB08400552-add]//
    RecvSize = MAX_BUFFER_SIZE;
//[-end-171228-IB08400552-add]//
    Status = IpmiLibExecuteIpmiCmd (
               H2O_IPMI_NETFN_STORAGE,
               H2O_IPMI_CMD_DELETE_SEL_ENTRY,
               CmdBuf,
               4,
               RecvBuf,
               &RecvSize
               );
//[-end-170929-IB08400459-modify]//
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }

  AddEventAfterArrangeEventStorage (PeiServices);

  return EFI_SUCCESS;
}

/**
 Adding an event data after arranging the storage.

 @param[in]         PeiServices     Efi Pei services.            
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
AddEventAfterArrangeEventStorage (
  IN  CONST EFI_PEI_SERVICES         **PeiServices
  )
{
  EFI_STATUS                         Status;
//[-start-170929-IB08400459-remove]//
//  H2O_IPMI_INTERFACE_PPI             *IpmiPpi;
//[-end-170929-IB08400459-remove]//
//[-start-170929-IB08400459-remove]//
//  H2O_IPMI_CMD_HEADER                Request = {
//                                             H2O_IPMI_BMC_LUN,
//                                             H2O_IPMI_NETFN_SENSOR_EVENT,
//                                             H2O_IPMI_CMD_EVENT_MESSAGE
//                                             };
//[-end-170929-IB08400459-remove]//
  UINT8                              RecvBuf[MAX_BUFFER_SIZE];
  UINT8                              RecvSize;    
  UINT8                              LogData[EVENT_LOG_DATA_SIZE];

//[-start-170929-IB08400459-remove]//
//  Status = (**PeiServices).LocatePpi (
//                                  PeiServices,
//                                  &gH2OIpmiInterfacePpiGuid,
//                                  0,
//                                  NULL,
//                                  (VOID **)&IpmiPpi
//                                  );
//  if (EFI_ERROR (Status)) {
//    return Status;
//  }
//[-end-170929-IB08400459-remove]//

  LogData[0] = BIOS_SOFTWARE_ID;
  LogData[1] = EVENT_REV;
  LogData[2] = ADJUST_STORAGE_EVENT_ID1;
  LogData[3] = ADJUST_STORAGE_EVENT_ID2;
  LogData[4] = ADJUST_STORAGE_EVENT_ID3;
  LogData[5] = ADJUST_STORAGE_DATA1;
  LogData[6] = ADJUST_STORAGE_DATA2;
  LogData[7] = ADJUST_STORAGE_DATA3;
  
//[-start-170929-IB08400459-modify]//
//[-start-171228-IB08400552-add]//
  RecvSize = MAX_BUFFER_SIZE;
//[-end-171228-IB08400552-add]//
  Status = IpmiLibExecuteIpmiCmd (
             H2O_IPMI_NETFN_SENSOR_EVENT,
             H2O_IPMI_CMD_EVENT_MESSAGE,
             LogData,
             EVENT_LOG_DATA_SIZE,
             RecvBuf,
             &RecvSize
             );
//[-end-170929-IB08400459-modify]//
    
  return Status;
}

/**
 Log data into BMC SEL.
            
 @param[in] PeiServices     Efi Pei services.
 @param[in] EventID         Event ID of logged data.
 @param[in] DataSize        Size of the logged data.
 @param[in] Data            Data which will be logged into the BMC SEL.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
LogDataToBmcSel (
  IN  CONST EFI_PEI_SERVICES                  **PeiServices,
  IN  EVENT_TYPE_ID                           EventID,
  IN  UINTN                                   DataSize,
  IN  UINT8                                   *Data
  )
{
  EFI_STATUS                         Status;
//[-start-170929-IB08400459-remove]//
//  H2O_IPMI_INTERFACE_PPI             *IpmiPpi;
//[-end-170929-IB08400459-remove]//
//[-start-170929-IB08400459-remove]//
//  H2O_IPMI_CMD_HEADER                Request = {
//                                             H2O_IPMI_BMC_LUN,
//                                             H2O_IPMI_NETFN_SENSOR_EVENT,
//                                             H2O_IPMI_CMD_EVENT_MESSAGE
//                                             };
//[-end-170929-IB08400459-remove]//
  UINT8                              RecvBuf[MAX_BUFFER_SIZE];
  UINT8                              RecvSize;    
  UINT8                              LogData[EVENT_LOG_DATA_SIZE];
  UINTN                              Index;

  if (DataSize > MAX_BMC_SEL_LOGGED_DATA_SIZE) {
    return EFI_OUT_OF_RESOURCES;
  }
  
//[-start-170929-IB08400459-remove]//
//  Status = (**PeiServices).LocatePpi (
//                                  PeiServices,
//                                  &gH2OIpmiInterfacePpiGuid,
//                                  0,
//                                  NULL,
//                                  (VOID **)&IpmiPpi
//                                  );
//  if (EFI_ERROR (Status)) {
//    return Status;
//  }
//[-end-170929-IB08400459-remove]//

  LogData[0] = BIOS_SOFTWARE_ID;
  LogData[1] = EVENT_REV;
  LogData[2] = EventID.SensorType;
  LogData[3] = EventID.SensorNum;
  LogData[4] = EventID.EventType;

  
  for (Index = 0; Index < DataSize; Index++) {
    LogData[5 + Index] = Data[Index];
  }
  
//[-start-170929-IB08400459-modify]//
//[-start-171228-IB08400552-add]//
  RecvSize = MAX_BUFFER_SIZE;
//[-end-171228-IB08400552-add]//
  Status = IpmiLibExecuteIpmiCmd (
             H2O_IPMI_NETFN_SENSOR_EVENT,
             H2O_IPMI_CMD_EVENT_MESSAGE,
             LogData,
             EVENT_LOG_DATA_SIZE,
             RecvBuf,
             &RecvSize
             );
//[-end-170929-IB08400459-modify]//
    
  return Status;
}

/**
  Perform PEI Event Handler Init.

  @param [in] FileHandle           Handle of the file being invoked. 
  @param [in] PeiServices          General purpose services available to every PEIM.

  @retval EFI Status            
**/
EFI_STATUS
EFIAPI
BmcSelStoragePeiEntryPoint ( 
  IN EFI_PEI_FILE_HANDLE        FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS            Status;

  Status = (**PeiServices).InstallPpi (PeiServices, &mPpiList);

  return Status;
}


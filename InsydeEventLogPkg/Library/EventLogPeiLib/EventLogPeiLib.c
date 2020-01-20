/** @file
 EventLogPei Lib functions which relate with connect the device

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
#include <Library/EventLogPeiLib.h>

#include <Ppi/EventLogPei.h>

//EVENT_LOG_PROTOCOL  *mEventLogPei=NULL;

/**
  This constructor function caches the H2O_PEI_EVENT_LOG_PPI pointer.

  @param[in] ImageHandle The firmware allocated handle for the EFI image.
  @param[in] SystemTable A pointer to the EFI System Table.

  @retval EFI_SUCCESS The constructor always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
EventLogPeiConstructor (
  IN EFI_PEI_FILE_HANDLE        FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{

  return EFI_SUCCESS;
  
}

/**
 Log event data in PEI phase by Pei Event Log PPI. 
         
 @param[in] PeiServices     Efi Pei services.
 @param[in] EventID         Event ID of the event data. Prototype: EVENT_TYPE_ID
 @param[in] DataSize        Size of event data. 
 @param[in] *LogData        Event data which will be logged.
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
PeiLogEventLib (
  IN  CONST EFI_PEI_SERVICES                  **PeiServices,
  IN  VOID                                    *EventID,
  IN  UINTN                                   DataSize,
  IN  UINT8                                   *LogData
  )
{
  EFI_STATUS                              Status;
  H2O_PEI_EVENT_LOG_PPI                   *PeiEventLogPpi;
  EVENT_TYPE_ID                           LogEventID;  

 (*PeiServices)->CopyMem (&LogEventID, EventID, sizeof (EVENT_TYPE_ID));
  
//[-start-171212-IB08400542-modify]//
  Status = (*PeiServices)->LocatePpi (
                                  PeiServices,
                                  &gH2OEventLogPpiGuid,
                                  0,                      
                                  NULL,                    
                                  (VOID **)&PeiEventLogPpi    
                                  );
//[-end-171212-IB08400542-modify]//
  if (!EFI_ERROR (Status)) {
    Status = PeiEventLogPpi->WriteEvent (
                                     PeiServices,
                                     LogEventID,
                                     DataSize,
                                     LogData
                                     );

  }

  return Status;
}



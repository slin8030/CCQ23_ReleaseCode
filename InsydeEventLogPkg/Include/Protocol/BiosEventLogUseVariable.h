/** @file

  Definition of Bios Event Log Use Variable protocol.
    
;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _BIOS_EVENT_LOG_USE_VARIABLE_PROTO_H_
#define _BIOS_EVENT_LOG_USE_VARIABLE_PROTO_H_

#include <Guid/BiosEventLogHob.h>
    

typedef struct _H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL;

/**
  Write event to BIOS Storage.

  @param [in] This                 This PPI interface.
  @param [in] BiosStorageType      Event log type.
  @param [in] PostBitmap1          Post bitmap 1 which will be stored in data area of POST error type log.
  @param [in] PostBitmap2          Post bitmap 2 which will be stored in data area of POST error type log.
  @param [in] OptionDataSize       Optional data size.
  @param [in] OptionLogData        Pointer to optional data.
  
  @retval EFI Status            
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BIOS_EVENT_LOG_USE_VARIABLE_WRITE) (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL       *This,
  IN  UINT8                                          EventLogType,
  IN  UINT32                                         PostBitmap1,
  IN  UINT32                                         PostBitmap2,
  IN  UINTN                                          OptionDataSize,
  IN  UINT8                                          *OptionLogData
  );

/**
  Clear the BIOS Event Log.

  @param [in] This                 This PPI interface.
  
  @retval EFI Status            
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BIOS_EVENT_LOG_USE_VARIABLE_CLEAR) (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL       *This
  );

/**
  Read the BIOS Event Log.

  @param [in] This                 This PPI interface.
  @param [in] EventListAddress     Address for reading Event Log in BIOS storage.
  
  @retval EFI Status            
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BIOS_EVENT_LOG_USE_VARIABLE_READ) (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL       *This,
  OUT VOID                                           **Buffer,
  OUT UINTN                                          *EventCount
  );

/** 
  Write the BIOS Event Log with overwrite method.

  @param [in] This                 This PPI interface.
  @param [in] InputBuffer          The event log data to logging..
  
  @retval EFI Status            
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BIOS_EVENT_LOG_USE_VARIABLE_OVERWRITE) (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL       *This,
  IN  BIOS_EVENT_LOG_ORGANIZATION                    *InputBuffer
  );

//
// SMM RUNTIME PROTOCOL
//
typedef struct _H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL {
  H2O_BIOS_EVENT_LOG_USE_VARIABLE_WRITE                     Write;
  H2O_BIOS_EVENT_LOG_USE_VARIABLE_CLEAR                     Clear;
  H2O_BIOS_EVENT_LOG_USE_VARIABLE_READ                      Read;  
  H2O_BIOS_EVENT_LOG_USE_VARIABLE_OVERWRITE                 OverWrite;
} H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL;

extern EFI_GUID gH2OBiosEventLogUseVariableProtocolGuid;
extern EFI_GUID gH2OSmmBiosEventLogUseVariableProtocolGuid;

#endif

/** @file

  Definition of Bios Event Log protocol.
    
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

#ifndef _BIOS_EVENT_LOG_PROTO_H_
#define _BIOS_EVENT_LOG_PROTO_H_

#include <Guid/BiosEventLogHob.h>
    
//
// EventLog input register EBX, for security check
//
#define EVENT_LOG_EBX_SIGNATURE      0x24454C47      //$ELG (EventLog)

typedef struct _EFI_BIOS_EVENT_LOG_PROTOCOL  EFI_BIOS_EVENT_LOG_PROTOCOL;

typedef enum {
  EVENT_LOG_CLEAR_LOG,
  EVENT_LOG_WRITE_LOG,
  EVENT_LOG_OVERWRITE_LOG
} EVENT_LOG_ACTION_TYPE;

typedef struct _EFI_EVENT_LOG_BUF {
  EFI_BIOS_EVENT_LOG_PROTOCOL       *This;
  EVENT_LOG_ACTION_TYPE             ActionType;
  UINT8                             EventLogType;
  UINT32                            PostBitmap1;
  UINT32                            PostBitmap2;
  UINTN                             OptionDataSize;
  UINT8                             *OptionLogData;
  EFI_STATUS                        ReturnStatus;
  UINT8                             *RawData;
} EFI_EVENT_LOG_BUF;


/**
  Smart detect current flash device type

  @param [in] This                 This PPI interface.
  @param [in] Buffer               For Flash device interface.
  
  @retval EFI Status            
**/
typedef
EFI_STATUS
(EFIAPI *EFI_BIOS_EVENT_LOG_DETECT_DEVICE) (
  IN EFI_BIOS_EVENT_LOG_PROTOCOL        *This,
  OUT UINT8                             *Buffer
  )
;

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
(EFIAPI *EFI_BIOS_EVENT_LOG_WRITE) (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This,
  IN  UINT8                             EventLogType,
  IN  UINT32                            PostBitmap1,
  IN  UINT32                            PostBitmap2,
  IN  UINTN                             OptionDataSize,
  IN  UINT8                             *OptionLogData
  );

/**
  Clear the BIOS Event Log.

  @param [in] This                 This PPI interface.
  
  @retval EFI Status            
**/
typedef
EFI_STATUS
(EFIAPI *EFI_BIOS_EVENT_LOG_CLEAR) (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This
  );

/**
  Read the BIOS Event Log.

  @param [in] This                 This PPI interface.
  @param [in] EventListAddress     Address for reading Event Log in BIOS storage.
  
  @retval EFI Status            
**/
typedef
EFI_STATUS
(EFIAPI *EFI_BIOS_EVENT_LOG_READ_NEXT) (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This,
  IN OUT VOID                           **EventListAddress
  );

/**
  Write the BIOS Event Log with overwrite method.

  @param [in] This                 This PPI interface.
  @param [in] InputBuffer          The event log data to logging..
  
  @retval EFI Status            
**/
typedef
EFI_STATUS
(EFIAPI *EFI_BIOS_EVENT_LOG_OVERWRITE) (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This,
  IN  BIOS_EVENT_LOG_ORGANIZATION       *InputBuffer
  );

//
// SMM RUNTIME PROTOCOL
//
typedef struct _EFI_BIOS_EVENT_LOG_PROTOCOL {
  EFI_BIOS_EVENT_LOG_DETECT_DEVICE             DetectDevice;
  EFI_BIOS_EVENT_LOG_WRITE                     Write;
  EFI_BIOS_EVENT_LOG_CLEAR                     Clear;
  EFI_BIOS_EVENT_LOG_READ_NEXT                 ReadNext;  
  EFI_BIOS_EVENT_LOG_OVERWRITE                 OverWrite;
} EFI_BIOS_EVENT_LOG_PROTOCOL;

//[-start-171212-IB08400542-modify]//
extern EFI_GUID gH2OBiosEventLogProtocolGuid;
extern EFI_GUID gH2OBiosSmmEventLogProtocolGuid;
//[-end-171212-IB08400542-modify]//
extern EFI_GUID gH2OBiosStorageCommunicationGuid;

#endif

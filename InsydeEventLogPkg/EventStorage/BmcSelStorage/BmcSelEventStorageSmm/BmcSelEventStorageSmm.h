/** @file

  Header file of BMC SEL Event Storage SMM implementation.

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


#ifndef _BMC_SEL_EVENT_STORAGE_SMM_H_
#define _BMC_SEL_EVENT_STORAGE_SMM_H_

//[-start-170929-IB08400459-remove]//
//#include <Protocol/H2OIpmiInterfaceProtocol.h>
//[-end-170929-IB08400459-remove]//
//[-start-170929-IB08400459-add]//
#include <Guid/IpmiDefinition.h>
#include <Library/IpmiLibDefine.h>
#include <Library/IpmiInterfaceLib.h>
#include <Library/IpmiSelDataLib.h>
//[-end-170929-IB08400459-add]//
#include <Protocol/EventLog.h>
#include <Protocol/EventStorage.h>
#include <Protocol/SmmBase2.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/IoLib.h>

#define MAX_BUFFER_SIZE           100
#define BIOS_SOFTWARE_ID          0x01
#define EVENT_REV                 0x04
#define MAX_RETRY_COUNT           0x3
#define ONE_SECOND                1000000
#define MAX_CLEAR_RETRY_COUNT     0x3
#define REQ_DATA_INITIATE_ERASE   0xAA
#define REQ_DATA_GET_ERASE_STS    0x00
#define ERASURE_IN_PROGRESS       0x00
#define ERASURE_COMPLETED         0x01

#define EVENT_LOG_FULL_ADJUST_EVENT_NUM    2
#define DELETE_SEL_CMD_SUPPORTED           BIT3
#define CHANGE_BMC_SEL_STORAGE_DATA3       0x03


typedef struct {
  UINT8                        RecvBuf[MAX_BUFFER_SIZE];
} VE_BMC_EVENT_LOG_STRUCTURE;

EFI_STATUS
EFIAPI
BmcSelWriteEvent (
  IN  EVENT_TYPE_ID                    *EventID,
  IN  UINT8                            *Data,
  IN  UINTN                            DataSize
  );

EFI_STATUS
EFIAPI
BmcSelReadEvent (
  IN      UINTN                        Index,
  IN OUT  UINT8                        **Data
  );

//[-start-170929-IB08400459-modify]//
EFI_STATUS
EFIAPI
GetBmcSelInfo (
  IN UINT8                             *RecvBuf,
  IN UINT8                             *RecvSize
  );
//[-end-170929-IB08400459-modify]//

EFI_STATUS
EFIAPI
BmcSelGetStorageNameString (
  CHAR16                               **String
);

EFI_STATUS
EFIAPI
BmcSelGetEventCount (
  IN  OUT UINTN                        *Count
  );

EFI_STATUS
EFIAPI
BmcSelRefreshDatabase (
  IN OUT  UINTN                        *DataCount
  );

EFI_STATUS
EFIAPI
BmcSelClearEvent (
  VOID
);

BOOLEAN
EFIAPI
IsLeapYear (
  IN UINT16                            Year
  );

UINTN
EFIAPI
CountNumOfLeapYears (
  IN UINT16                            CurYear
  );

EFI_STATUS
EFIAPI
Ts2et (
  IN UINT32                            TimeStamp,
  IN EFI_TIME                          *EfiTime
  );

VOID
EFIAPI
StallForRetry (
  IN UINTN                              MicroSecond
  );

VOID
EFIAPI
FreeBmcSelEventDatabase (
  VOID
  );

EFI_STATUS
EFIAPI
CheckEventLogFull (
  IN OUT BOOLEAN                    *LogFull
  );

EFI_STATUS
EFIAPI
AddEventAfterArrangeEventStorage (
  VOID
  );

EFI_STATUS
EFIAPI
ArrangeEventStorage (
  VOID
  );

EFI_STATUS
EFIAPI
ShiftDataOfStorage (
  VOID
  );

#endif

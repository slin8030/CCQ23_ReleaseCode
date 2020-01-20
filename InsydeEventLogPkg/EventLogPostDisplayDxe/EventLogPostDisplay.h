/** @file

  Header file of Event Log POST Display DXE implementation.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _EVENT_LOG_POST_DISPLAY_DXE_H_
#define _EVENT_LOG_POST_DISPLAY_DXE_H_

//
// Statements that include other header files
//
#include <EventLogPostDisplaySetting.h>
#include <EventLogPostDisplayCommonFunc.h>

#include <Protocol/EventLog.h>
#include <Protocol/EventLogString.h>
#include <Protocol/EventLogStringOverwrite.h>
#include <Protocol/LoadedImage.h>

#define MAX_BUFFER_SIZE                         100
#define DEFAULT_SCROLLBAR_WIDTH                 1
#define MAX_STORAGE_SUPPORT                     3
#define MAX_WAIT_SECOND                         10
#define ONE_SECOND                              10000000
//[-start-180724-IB08400617-add]//
#define MAX_STORAGE_NAME_NUM                    10
//[-end-180724-IB08400617-add]//

#define CURRENT_POSX    (UINTN)gST->ConOut->Mode->CursorColumn
#define CURRENT_POSY    (UINTN)gST->ConOut->Mode->CursorRow

#pragma pack(1)
typedef struct {
  CHAR16                  *StorageString;
  CHAR16                  *ShowString;
  UINTN                   Id;
  UINTN                   FilteredEventAddress;
  UINTN                   FilteredEventNum;
} VIEW_EVENT_LOG_MENU;

typedef struct {
  UINT8  Address; ///< RTC offset address.
  UINT8  Data;    ///< Corresponding data value.
} RTC_COMPONENT;

//
// Register A
//
typedef struct {
  UINT8 Rs : 4;   ///< Rate Selection Bits
  UINT8 Dv : 3;   ///< Divisor
  UINT8 Uip : 1;  ///< Update in progress
} RTC_REGISTER_A_BITS;

typedef union {
  RTC_REGISTER_A_BITS Bits;
  UINT8               Data;
} RTC_REGISTER_A;

typedef struct {
  UINT8                   StorageBitMap;
  EVENT_TYPE_ID           EventID;
} EL_POST_DISP_FILTER;

typedef struct {
  BOOLEAN                 BiosEnable;
  BOOLEAN                 BmcSelEnable;
  BOOLEAN                 MemoryEnable;
} EL_POST_DISP_STORAGE;

#pragma pack()


EFI_STATUS
EFIAPI
ViewEventLogDriverUnload ( 
  IN EFI_HANDLE  ImageHandle 
  );

EFI_STATUS
EFIAPI
ExecuteFilterEventLogViewer (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  );

EFI_STATUS
EFIAPI
GetEventString (
  IN     VOID                           *EventInfo,
  IN OUT CHAR16                         **EventString,
  IN OUT UINTN                          *StringSize
  );

EFI_STATUS
EFIAPI
WaitForSingleEvent (
  EFI_EVENT  Event,
  UINT64     Timeout OPTIONAL
  );

//
// Global Variables
//
extern UINTN                               gColumn;
extern UINTN                               gTopRow;
extern UINTN                               gBottomRow;
extern UINTN                               gMaxItemPerPage;
extern VIEW_EVENT_LOG_MENU                 *MenuList;
extern EVENT_LOG_PROTOCOL                  *gEventLog;
extern CHAR16                              *gUtilityVersion;
extern BOOLEAN                             gShowUtilVer;
#endif  

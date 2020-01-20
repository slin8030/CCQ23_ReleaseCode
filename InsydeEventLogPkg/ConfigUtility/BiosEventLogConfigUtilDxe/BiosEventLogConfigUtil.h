/** @file
  Header file for BIOS Event Log Config utility

;******************************************************************************
;* Copyright (c) 2016 - 2018, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _BIOS_EVENT_LOG_CONFIG_UTIL_H_
#define _BIOS_EVENT_LOG_CONFIG_UTIL_H_

#include <BiosEventLogConfigUtilSetupConfig.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
//[-start-171208-IB08400539-modify]//
//#include <Library/SetupUtilityLib.h>
#include <Guid/MdeModuleHii.h>
//[-end-171208-IB08400539-modify]//
#include <Library/DevicePathLib.h>
#include <Library/VariableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiString.h>
#include <Protocol/SetupUtilityApplication.h>
//[-start-180518-IB08400594-modify]//
#include <Protocol/H2ODialog.h>
//[-end-180518-IB08400594-modify]//
#include <Protocol/EventLog.h>
#include <Protocol/EventLogString.h>
#include <Protocol/EventLogStringOverwrite.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/BlockIo.h>

//
// This is the generated IFR binary data for each formset defined in VFR.
// This data array is ready to be used as input of HiiAddPackages() to
// create a packagelist (which contains Form packages, String packages, etc).
//
extern UINT8 BiosEventLogConfigUtilConfigPageVfrBin[];
extern UINT8 BiosEventLogConfigUtilEventAndMessagePageVfrBin[];

#define BIOS_EVENT_LOG_CONFIG_UTIL_PRIVATE_SIGNATURE SIGNATURE_32 ('B', 'E', 'C', 'U')
#define MAX_STORAGE_NAME_NUM                    10
//[-start-180724-IB08400617-remove]//
//#define MAX_ELV_STR_NUM                         200
//[-end-180724-IB08400617-remove]//
#define MAX_FILTER_STR_NUM                      30
#define VTS_LEFT_ALIGN                          0
#define VTS_RIGHT_ALIGN                         1
#define VTS_LEAD_0                              2

#define NO_STORAGE_BE_SELECTED                  0xFFFF

//
// Filtered condition bitmap
//
#define B_FILTERED_BY_YEAR                      BIT0
#define B_FILTERED_BY_MONTH                     BIT1
#define B_FILTERED_BY_DATE                      BIT2
#define B_FILTERED_BY_HOUR                      BIT3
#define B_FILTERED_BY_MINUTE                    BIT4
#define B_FILTERED_BY_SECOND                    BIT5
#define B_FILTERED_BY_GENERATOR_ID              BIT6
#define B_FILTERED_BY_EVENT_ID                  BIT7
#define B_FILTERED_BY_SENSOR_TYPE               BIT8
#define B_FILTERED_BY_SENSOR_NUM                BIT9
#define B_FILTERED_BY_EVENT_TYPE                BIT10
#define B_FILTERED_BY_EVENT_TYPE_ID             BIT11
#define B_FILTERED_BY_STRING                    BIT12

#define ELV_LIST_BASE                           0x1200
#define MAX_SHOW_ELV_LIST_NUM                   100

typedef enum {
  SAVE_ALL_EVENT_LOG = 0,
  SAVE_CURRENT_PAGE_EVENT_LOG = 1  
} SAVE_EVENT_LOG_ACTION;

typedef enum {
  SHOW_FIRST_PAGE = 0,
  SHOW_PREVIOUS_PAGE = 1,
  SHOW_NEXT_PAGE = 2
} SHOW_EVENT_LOG_ACTION;

typedef enum {
  TypeDate,
  TypeTime,
  TypeGeneratorID,
  TypeEventID,
  TypeSensorType,
  TypeSensorNum,
  TypeEventType,
  TypeEventTypeID,
  TypeString,
  TypeClearFilteredCondition,
  TypeEnd
} TYPE_OF_FILTER_EVENT;

#pragma pack(1)
typedef struct {
  UINT32                                FilterType;
  UINT16                                Year;
  UINT8                                 Month;
  UINT8                                 Date;
  UINT8                                 Hour;
  UINT8                                 Minute;
  UINT8                                 Second;
  UINT16                                GeneratorId;
  EVENT_TYPE_ID                         EventID;     // 3-bytes ID:   
  UINT8                                 EventTypeID; // for bios
} FILTER_EVENT_INFO;

///
/// HII specific Vendor Device Path definition.
///
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;

typedef struct {
  UINTN                              Signature;

  EFI_HANDLE                         DriverHandle[2];
  EFI_HII_HANDLE                     HiiHandle[2];
  BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION Configuration;

  //
  // Consumed protocol
  //
  EFI_HII_DATABASE_PROTOCOL          *HiiDatabase;
  EFI_HII_STRING_PROTOCOL            *HiiString;
  EFI_HII_CONFIG_ROUTING_PROTOCOL    *HiiConfigRouting;
  EFI_FORM_BROWSER2_PROTOCOL         *FormBrowser2;

  //
  // Produced protocol
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL     ConfigAccess;
//[-start-180507-IB08400587-add]//
  EFI_HII_CONFIG_ACCESS_PROTOCOL     ConfigAccess2;
//[-end-180507-IB08400587-add]//
} BIOS_EVENT_LOG_CONFIG_UTIL_PRIVATE_DATA;

#pragma pack()

#define BIOS_EVENT_LOG_CONFIG_UTIL_PRIVATE_FROM_THIS(a)  CR (a, BIOS_EVENT_LOG_CONFIG_UTIL_PRIVATE_DATA, ConfigAccess, BIOS_EVENT_LOG_CONFIG_UTIL_PRIVATE_SIGNATURE)

#endif

/** @file

   The definition of BIOS Event Log Config utility HII.

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

#ifndef _BIOS_EVENT_LOG_CONFIG_UTIL_HII_H_
#define _BIOS_EVENT_LOG_CONFIG_UTIL_HII_H_

#define H2O_BIOS_EVENT_LOG_CONFIG_UTIL_CONFIG_PAGE_FORMSET_GUID \
  { \
    0xC71DE1B4, 0x25EF, 0x4F31, {0xA7, 0x26, 0xAA, 0x10, 0x50, 0x56, 0x8C, 0x23} \
  } 

#define H2O_BIOS_EVENT_LOG_CONFIG_UTIL_EVENT_AND_MESSAGE_PAGE_FORMSET_GUID \
  { \
    0xB24056AB, 0x8970, 0x427A, {0x8D, 0x7D, 0xA3, 0xC4, 0x9C, 0xB7, 0x04, 0xEE} \
  } 

#define H2O_BIOS_EVENT_LOG_CONFIG_UTIL_VARSTORE_GUID \
  { \
    0xC0E1B3E7, 0x7C82, 0x4A26, {0xA6, 0x03, 0xCB, 0x05, 0xCC, 0xB7, 0x05, 0xB2} \
  } 
#define H2O_BIOS_EVENT_LOG_CONFIG_UTIL_VARSTORE_NAME     L"BiosEventLogConfig"
#define H2O_BIOS_EVENT_LOG_CONFIG_UTIL_FILTER_EFI_VARSTORE_NAME L"BiosEventLogFilter"

#pragma pack(1)

//[-start-171207-IB08400539-remove]//
//#define  EVENT_STORAGE_ALL       0
//#define  EVENT_STORAGE_BIOS      1
//#define  EVENT_STORAGE_BMC_SEL   2
//#define  EVENT_STORAGE_DCMI_SEL  3
//#define  EVENT_STORAGE_MEMORY    4
//#define  EVENT_STORAGE_DISABLED  0xFF
//[-end-171207-IB08400539-remove]//

typedef struct {
  //
  // Bios Event Log Utility Configuration
  //
  UINT8         EventLogFullOption;                  
  UINT8         LogEventTo;
} BIOS_EVENT_LOG_CONFIG_UTIL_CONFIGURATION;

typedef struct {
  //
  // Bios Event Log Utility Filter Configuration
  //
  UINT8         StorageType;
  UINT8         FilterEn;                  
  UINT8         DateFilter[20]; // Ex. 2013/07/31
  UINT8         TimeFilter[16]; // Ex. 03:21:58
  UINT8         GeneratorIdFilter[12]; // Ex. 0x0020
  UINT8         EventIdFilter[30]; // Ex. 0x0F 0x00 0x6F
  UINT8         SensorTypeFilter[8]; // Ex. 0x02
  UINT8         SensorNumFilter[8]; // Ex. 0xEC
  UINT8         EventTypeFilter[8]; // Ex. 0x02
  UINT8         EventTypeIdFilter[8];// Ex. 0x02
  UINT8         StringFilter[30];// Ex. Test
} BIOS_EVENT_LOG_CONFIG_UTIL_FILTER_CONFIGURATION;

#pragma pack()

extern EFI_GUID gH2OBiosEventLogConfigUtilConfigFormsetGuid;
extern EFI_GUID gH2OBiosEventLogConfigUtilEventAndMessageFormsetGuid;
extern EFI_GUID gH2OBiosEventLogConfigUtilVarstoreGuid;
#endif

/** @file
  Header file for BIOS Event Log Config utility Setup Config

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

#include <Guid/BiosEventLogConfigUtilHii.h>
//[-start-171207-IB08400539-add]//
#include <EventLogDefine.h>
//[-end-171207-IB08400539-add]//

#ifndef _BIOS_EVENT_LOG_CONFIG_UTIL_SETUP_CONFIG_H_
#define _BIOS_EVENT_LOG_CONFIG_UTIL_SETUP_CONFIG_H_

//
// Variable store ID
//
#define BIOS_EVENT_LOG_CONFIG_UTIL_VARSTORE_ID           0x1001
#define BIOS_EVENT_LOG_CONFIG_UTIL_FILTER_VARSTORE_ID    0x1002


//
// Form ID definition
//
#define VFR_FORMID_BIOS_EVENT_LOG_CONFIG_PAGE            0x0011
#define VFR_FORMID_BIOS_EVENT_LOG_EVENT_AND_MESSAGE_PAGE 0x0012
#define VFR_FORMID_BIOS_EVENT_LOG_VIEWER_STORAGE         0x0013

//
// Labels definition
//
#define ELV_STORAGE_HEADER_START_LABEL            0x1011
#define ELV_STORAGE_HEADER_END_LABEL              0x1012
#define ELV_STORAGE_EVENTS_START_LABEL            0x1013
#define ELV_STORAGE_EVENTS_END_LABEL              0x1014

//
// Keys definition
//
#define KEY_LOAD_DEFAULT                          0x3001
#define KEY_LOG_EVENT_TO                          0x3002
#define KEY_EVENT_LOG_FULL_OPTION                 0x3003
#define KEY_FILTER_ENABLE                         0x3004
#define KEY_FILTER_DATE                           0x3005
#define KEY_FILTER_TIME                           0x3006
#define KEY_FILTER_GENERATOR_ID                   0x3007
#define KEY_FILTER_EVENT_ID                       0x3008
#define KEY_FILTER_SENSOR_TYPE                    0x3009
#define KEY_FILTER_SENSOR_NUMBER                  0x300A
#define KEY_FILTER_EVENT_TYPE                     0x300B
#define KEY_FILTER_EVENT_TYPE_ID                  0x300C
#define KEY_FILTER_STRING                         0x300D


#define KEY_ELV_SHOW_EVENT_LOG                    0x3101
#define KEY_ELV_CLEAR_EVENT_LOG                   0x3102
#define KEY_ELV_SAVE_EVENT_STORAGE                0x3103
#define KEY_SHOW_EVENT_PREVIOUS_PAGE              0x3104
#define KEY_SHOW_EVENT_NEXT_PAGE                  0x3105
#define KEY_SHOW_EVENT_PREVIOUS_PAGE2             0x3106
#define KEY_SHOW_EVENT_NEXT_PAGE2                 0x3107

#endif

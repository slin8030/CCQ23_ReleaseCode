/** @file
  Header file for H2O Event Log Config Manager Setup Config

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

#include <Guid/H2OEventLogConfigManagerHii.h>

#ifndef _H2O_EVENT_LOG_CONFIG_MANAGER_SETUP_CONFIG_H_
#define _H2O_EVENT_LOG_CONFIG_MANAGER_SETUP_CONFIG_H_

//
// Form ID definition
//
#define VFR_FORMID_H2O_EVENT_LOG_CONTROL_UTIL   0x0010
#define VFR_FORMID_SHOW_CONFIGURATION_PAGE      0x0011
#define VFR_FORMID_SHOW_EVENT_AND_MESSAGE_PAGE  0x0012

//
// Labels definition
//
#define CONFIGURATION_PAGE_LABEL                0x1011
#define CONFIGURATION_PAGE_END_LABEL            0x1012

#define EVENT_AND_MESSAGE_PAGE_LABEL            0x1013
#define EVENT_AND_MESSAGE_PAGE_END_LABEL        0x1014

//
// Keys definition
//
#define KEY_SHOW_CONFIGURATION_PAGE             0x3101
#define KEY_SHOW_EVENT_AND_MESSAGE_PAGE         0x3102

#endif

/** @file
  Header file for Setup Change Config utility Setup Config

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

#include <Guid/SetupChangeConfigUtilHii.h>

#ifndef _SETUP_CHANGE_CONFIG_UTIL_SETUP_CONFIG_H_
#define _SETUP_CHANGE_CONFIG_UTIL_SETUP_CONFIG_H_

//
// Variable store ID
//
#define SETUP_CHANGE_CONFIG_UTIL_VARSTORE_ID      0x3001

//
// Form ID definition
//
#define VFR_FORMID_SETUP_CHANGE_CONFIG_PAGE            0x0011
#define VFR_FORMID_SETUP_CHANGE_EVENT_AND_MESSAGE_PAGE 0x0012
#define VFR_FORMID_SETUP_CHANGE_SHOW_HISTORY           0x0013

//
// Labels definition
//
#define H2O_SETUP_CHANGE_HEADER_START_LABEL       0x1011
#define H2O_SETUP_CHANGE_HEADER_END_LABEL         0x1012
#define H2O_SETUP_CHANGE_EVENTS_START_LABEL       0x1013
#define H2O_SETUP_CHANGE_EVENTS_END_LABEL         0x1014

//
// Keys definition
//
#define KEY_LOAD_DEFAULT                          0x3001

#define KEY_ELV_SHOW_H2O_SETUP_CHANGE             0x3101
#define KEY_ELV_SAVE_H2O_SETUP_CHANGE             0x3102
#define KEY_SHOW_H2O_SETUP_CHANGE_PREVIOUS_PAGE   0x3103
#define KEY_SHOW_H2O_SETUP_CHANGE_NEXT_PAGE       0x3104
#define KEY_SHOW_H2O_SETUP_CHANGE_PREVIOUS_PAGE2  0x3105
#define KEY_SHOW_H2O_SETUP_CHANGE_NEXT_PAGE2      0x3106

#endif

/** @file
  Header file for Debug Message Config utility Setup Config

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

#include <Guid/DebugMessageConfigUtilHii.h>

#ifndef _DEBUG_MESSAGE_CONFIG_UTIL_SETUP_CONFIG_H_
#define _DEBUG_MESSAGE_CONFIG_UTIL_SETUP_CONFIG_H_

//
// Variable store ID
//
#define DEBUG_MESSAGE_CONFIG_UTIL_VARSTORE_ID     0x4001

//
// Form ID definition
//
#define VFR_FORMID_DEBUG_MESSAGE_CONFIG_PAGE            0x0011
#define VFR_FORMID_DEBUG_MESSAGE_EVENT_AND_MESSAGE_PAGE 0x0012
#define VFR_FORMID_SHOW_SERIAL_DEBUG_MESSAGE            0x0013

//
// Labels definition
//
#define ELV_SERIAL_DEBUG_HEADER_START_LABEL       0x1011
#define ELV_SERIAL_DEBUG_HEADER_END_LABEL         0x1012
#define ELV_SERIAL_DEBUG_EVENTS_START_LABEL       0x1013
#define ELV_SERIAL_DEBUG_EVENTS_END_LABEL         0x1014

//
// Keys definition
//
#define KEY_LOAD_DEFAULT                          0x3001
#define KEY_EFI_DEBUG_MSG_LEVEL                   0x3002
//[-start-160218-IB08400332-add]//
#define KEY_EFI_DEBUG_MSG_ADVANCED_MODE           0x3003
//[-end-160218-IB08400332-add]//

#define KEY_ELV_SHOW_EFI_DEBUG_MSG                0x3101
#define KEY_ELV_SAVE_EFI_DEBUG_MSG                0x3102
#define KEY_SHOW_SERIAL_DEBUG_MSG_PREVIOUS_PAGE   0x3103
#define KEY_SHOW_SERIAL_DEBUG_MSG_NEXT_PAGE       0x3104
#define KEY_SHOW_SERIAL_DEBUG_MSG_PREVIOUS_PAGE2  0x3105
#define KEY_SHOW_SERIAL_DEBUG_MSG_NEXT_PAGE2      0x3106

#endif

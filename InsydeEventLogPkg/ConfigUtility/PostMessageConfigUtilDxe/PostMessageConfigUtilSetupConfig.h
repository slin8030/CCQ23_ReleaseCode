/** @file
  Header file for POST Message Config utility Setup Config

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

#include <Guid/PostMessageConfigUtilHii.h>

#ifndef _POST_MESSAGE_CONFIG_UTIL_SETUP_CONFIG_H_
#define _POST_MESSAGE_CONFIG_UTIL_SETUP_CONFIG_H_

//
// Variable store ID
//
#define POST_MESSAGE_CONFIG_UTIL_VARSTORE_ID         0x2001

//
// Form ID definition
//
#define VFR_FORMID_POST_MESSAGE_CONFIG_PAGE          0x2011

//
// Keys definition
//
#define KEY_LOAD_DEFAULT                             0x2001
#define KEY_POST_MESSAGE_EN                          0x2002
#define KEY_PROGRESS_CODE_EN                         0x2003
#define KEY_ERROR_CODE_EN                            0x2004
#define KEY_DEBUG_CODE_EN                            0x2005
#define KEY_LOG_POST_MSG_EN                          0x2006
#define KEY_SHOW_POST_MSG_EN                         0x2007


#endif

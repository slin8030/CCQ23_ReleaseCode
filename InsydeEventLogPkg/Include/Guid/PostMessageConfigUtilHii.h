/** @file

   The definition of POST Message Config utility HII.

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

#ifndef _POST_MESSAGE_CONFIG_UTIL_HII_H_
#define _POST_MESSAGE_CONFIG_UTIL_HII_H_

#define H2O_POST_MESSAGE_CONFIG_UTIL_CONFIG_PAGE_FORMSET_GUID \
  { \
    0xDFD67CFE, 0xCEA9, 0x4777, {0x90, 0x0F, 0x04, 0x47, 0x87, 0x79, 0x8D, 0x43} \
  } 

#define H2O_POST_MESSAGE_CONFIG_UTIL_VARSTORE_GUID \
  { \
    0x73B169AF, 0xBB78, 0x40E4, {0xA6, 0x10, 0x24, 0xF5, 0x44, 0x8C, 0xF1, 0x4C} \
  } 

#define H2O_POST_MESSAGE_CONFIG_UTIL_VARSTORE_NAME L"PostMessageConfig"

#pragma pack(1)

typedef struct {
  //
  // POST Message Utility Configuration
  //
  UINT8         PostMessageEn;
  UINT8         ProgressCodeEn;
  UINT8         ErrorCodeEn;
  UINT8         DebugCodeEn;
  UINT8         LogPostMsgEn;                   
  UINT8         ShowPostMsgEn;
} POST_MESSAGE_CONFIG_UTIL_CONFIGURATION;

#pragma pack()

extern EFI_GUID gH2OPostMessageConfigUtilConfigFormsetGuid;
extern EFI_GUID gH2OPostMessageConfigUtilVarstoreGuid;

#endif

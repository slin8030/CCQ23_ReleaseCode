/** @file

   The definition of Debug Message Config utility HII.

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

#ifndef _DEBUG_MESSAGE_CONFIG_UTIL_HII_H_
#define _DEBUG_MESSAGE_CONFIG_UTIL_HII_H_

#define H2O_DEBUG_MESSAGE_CONFIG_UTIL_CONFIG_PAGE_FORMSET_GUID \
  { \
    0xFCCFE602, 0xE062, 0x482F, {0x93, 0xAB, 0x2D, 0xD3, 0xB3, 0x94, 0xF1, 0x5C} \
  } 

#define H2O_DEBUG_MESSAGE_CONFIG_UTIL_EVENT_AND_MESSAGE_PAGE_FORMSET_GUID \
  { \
    0x22CB6743, 0xC6AB, 0x4479, {0xBC, 0x72, 0x9F, 0x8E, 0x62, 0x67, 0xF3, 0xBB} \
  } 

#define H2O_DEBUG_MESSAGE_CONFIG_UTIL_VARSTORE_GUID \
  { \
    0x091AAE73, 0x6D75, 0x479E, {0x8D, 0x16, 0x8E, 0x38, 0xD8, 0xC8, 0x58, 0xC6} \
  } 

#define H2O_DEBUG_MESSAGE_CONFIG_UTIL_VARSTORE_NAME  L"DebugMessageConfig"

#pragma pack(1)

typedef struct {
  //
  // DEBUG Message Utility Configuration
  //
  UINT8         EfiDebugMsgLevel;
  UINT8         ShowProgressCode;    // Show Progress Code message of Status Code by serial port
  UINT8         ShowErrorCode;       // Show Error Code message of Status Code by serial port
  UINT8         ShowDebugCode;       // Show Debug Code message of Status Code by serial port
//[-start-160218-IB08400332-add]//
  UINT8         EfiDebugMsgAdvancedMode;
  UINT8         DebugInit;
  UINT8         DebugWarn;
  UINT8         DebugLoad;
  UINT8         DebugFs;
  UINT8         DebugInfo;
  UINT8         DebugDispatch;
  UINT8         DebugVariable;
  UINT8         DebugBm;
  UINT8         DebugBlkio;
  UINT8         DebugNet;
  UINT8         DebugUndi;
  UINT8         DebugLoadFile;
  UINT8         DebugEvent;
  UINT8         DebugGcd;
  UINT8         DebugCache;
  UINT8         DebugVerbose;
  UINT8         DebugError;
//[-end-160218-IB08400332-add]//
} DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION;

#pragma pack()

extern EFI_GUID gH2ODebugMessageConfigUtilConfigFormsetGuid;
extern EFI_GUID gH2ODebugMessageConfigUtilEventAndMessageFormsetGuid;
extern EFI_GUID gH2ODebugMessageConfigUtilVarstoreGuid;
#endif

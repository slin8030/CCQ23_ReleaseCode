/** @file
 Seg Platform support library header file.

 This file contains functions prototype that can easily access FUR data via
 using H2O IPMI FRU protocol.

;******************************************************************************
;* Copyright (c) 2018, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/
#ifndef _SEG_PLATFORM_SUPPORT_LIB_H_
#define _SEG_PLATFORM_SUPPORT_LIB_H_

typedef enum {
  BmcUsbDevicePath,
  PlatformInfoTypeEnd
} SEG_PLATFORM_INFO_TYPE;

/**
  This function returns information data belonging to DataType

  @param[in]  DataType                    The type which need info
  @param[out] ReturnData                  Return info data

  @return status of get info.

**/

EFI_STATUS
SegPlatformSupportLibGetInfo (
  IN  UINT8               DataType,
  OUT VOID                **ReturnData
  );

#endif


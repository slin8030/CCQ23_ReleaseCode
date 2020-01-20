/** @file
 Seg platform support library implement code.

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
  
#include <Uefi.h>
#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SegPlatformSupportLib.h>

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
  )
{ 
  CHAR16                  *BmcUsbHubDevicePath;
  CHAR8                   *GetStr;
  EFI_STATUS              Status;

  GetStr = NULL;
  Status = EFI_UNSUPPORTED;
  DEBUG((EFI_D_INFO, "[SegPlatformSupportLibGetInfo] DataType %d\n", DataType));
  switch (DataType) {
  case BmcUsbDevicePath:
    GetStr = PcdGetPtr (PcdH2OSegPlatformSupportBmcUsbDevicePath);
    BmcUsbHubDevicePath = AllocateZeroPool (PcdGetSize (PcdH2OSegPlatformSupportBmcUsbDevicePath) * 2);
    AsciiStrToUnicodeStr (GetStr, BmcUsbHubDevicePath);
    DEBUG((EFI_D_INFO, "[SegPlatformSupportLibGetInfo] BmcUsbHubDevicePath %s\n", BmcUsbHubDevicePath));
    if (GetStr != NULL) {
      *ReturnData = (VOID *)ConvertTextToDevicePath (BmcUsbHubDevicePath);
      FreePool (BmcUsbHubDevicePath);
    }
    Status = EFI_SUCCESS;
    break;

  case PlatformInfoTypeEnd:
  default:
    Status = EFI_UNSUPPORTED;
    break;
  }
  return Status;
}


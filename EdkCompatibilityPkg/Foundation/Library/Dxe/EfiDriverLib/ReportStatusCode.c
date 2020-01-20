/** @file

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/*++

Copyright (c) 2004 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ReportStatusCode.c

Abstract:

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)
#include EFI_ARCH_PROTOCOL_DEFINITION (StatusCode)

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
STATIC EFI_STATUS_CODE_PROTOCOL  *gStatusCode = NULL;
#endif

/**
 Report device path through status code.

 @param [in]   Type             Code type
 @param [in]   Value            Code value
 @param [in]   Instance         Instance number
 @param [in]   CallerId         Caller name
 @param [in]   Data

 @return Status code.
 @retval EFI_OUT_OF_RESOURCES   No enough buffer could be allocated

**/
EFI_STATUS
EfiLibReportStatusCode (
  IN EFI_STATUS_CODE_TYPE     Type,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN UINT32                   Instance,
  IN EFI_GUID                 *CallerId OPTIONAL,
  IN EFI_STATUS_CODE_DATA     *Data     OPTIONAL  
  )
{
  EFI_STATUS  Status;

#if (EFI_SPECIFICATION_VERSION >= 0x00020000) 
  if (gStatusCode == NULL) {
    if (gBS == NULL) {
      return EFI_UNSUPPORTED;
    }
    Status = gBS->LocateProtocol (&gEfiStatusCodeRuntimeProtocolGuid, NULL, (VOID **)&gStatusCode);
    if (EFI_ERROR (Status) || gStatusCode == NULL) {
      return EFI_UNSUPPORTED;
    }
  }
  Status = gStatusCode->ReportStatusCode (Type, Value, Instance, CallerId, Data);
  return Status;
#else
  if (gRT == NULL) {
    return EFI_UNSUPPORTED;
  }
  //
  // Check whether EFI_RUNTIME_SERVICES has Tiano Extension
  //
  Status = EFI_UNSUPPORTED;
  if (gRT->Hdr.Revision     == EFI_SPECIFICATION_VERSION     &&
      gRT->Hdr.HeaderSize   == sizeof (EFI_RUNTIME_SERVICES) &&
      gRT->ReportStatusCode != NULL) {
    Status = gRT->ReportStatusCode (Type, Value, Instance, CallerId, Data);
  }
  return Status;
#endif
}

/**
 Report device path through status code.

 @param [in]   Type             Code type
 @param [in]   Value            Code value
 @param [in]   Instance         Instance number
 @param [in]   CallerId         Caller name
 @param [in]   DevicePath       Device path that to be reported

 @return Status code.
 @retval EFI_OUT_OF_RESOURCES   No enough buffer could be allocated

**/
EFI_STATUS
ReportStatusCodeWithDevicePath (
  IN EFI_STATUS_CODE_TYPE     Type,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN UINT32                   Instance,
  IN EFI_GUID                 * CallerId OPTIONAL,
  IN EFI_DEVICE_PATH_PROTOCOL * DevicePath
  )
{
  UINT16                    Size;
  UINT16                    DevicePathSize;
  EFI_STATUS_CODE_DATA      *ExtendedData;
  EFI_DEVICE_PATH_PROTOCOL  *ExtendedDevicePath;
  EFI_STATUS                Status;

  DevicePathSize  = (UINT16) EfiDevicePathSize (DevicePath);
  Size            = (UINT16) (DevicePathSize + sizeof (EFI_STATUS_CODE_DATA));
  ExtendedData    = (EFI_STATUS_CODE_DATA *) EfiLibAllocatePool (Size);
  if (ExtendedData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ExtendedDevicePath = EfiConstructStatusCodeData (Size, &gEfiStatusCodeSpecificDataGuid, ExtendedData);
  EfiCopyMem (ExtendedDevicePath, DevicePath, DevicePathSize);

  Status = EfiLibReportStatusCode (Type, Value, Instance, CallerId, (EFI_STATUS_CODE_DATA *) ExtendedData);

  gBS->FreePool (ExtendedData);
  return Status;
}

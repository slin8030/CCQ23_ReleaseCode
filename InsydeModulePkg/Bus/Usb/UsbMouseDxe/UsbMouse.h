/** @file

;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _USB_MOUSE_H
#define _USB_MOUSE_H

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/HidDescriptorLib.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/UsbCore.h>
#include <Guid/StatusCodeDataTypeId.h>

#define STATE_BUFFER_SIZE       16

#define IMAGE_DRIVER_NAME       L"Usb Mouse Driver"
#define CONTROLLER_DRIVER_NAME  L"Generic Usb Mouse"
#define USB_MOUSE_DEV_SIGNATURE SIGNATURE_32 ('u', 'm', 'o', 'u')

typedef struct {
  BOOLEAN ButtonDetected;
  UINT8   ButtonMinIndex;
  UINT8   ButtonMaxIndex;
  UINT8   Reserved;
} PRIVATE_DATA;

typedef struct {
  UINTN                         Signature;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  EFI_USB_IO_PROTOCOL           *UsbIo;
  EFI_USB_CORE_PROTOCOL         *UsbCore;
  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR   IntEndpointDescriptor;
  EFI_SIMPLE_POINTER_PROTOCOL   SimplePointerProtocol;
  EFI_SIMPLE_POINTER_STATE      State[STATE_BUFFER_SIZE];
  EFI_SIMPLE_POINTER_MODE       Mode;
  UINT8                         ReportId;
  BOOLEAN                       StateChanged;
  PRIVATE_DATA                  PrivateData;
  REPORT_FIELD_INFO             ReportFieldInfo;
  EFI_UNICODE_STRING_TABLE      *ControllerNameTable;
  EFI_HANDLE                    RecoveryPollingHandle;
  UINTN                         BufferIn;
  UINTN                         BufferOut;
} USB_MOUSE_DEV;

#define USB_MOUSE_DEV_FROM_MOUSE_PROTOCOL(a) \
    CR(a, USB_MOUSE_DEV, SimplePointerProtocol, USB_MOUSE_DEV_SIGNATURE)

VOID
EFIAPI
UsbMouseRecoveryHandler (
  IN    UINTN        Event,
  IN    VOID         *Context
  );

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL      gUsbMouseDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL      gUsbMouseComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL     gUsbMouseComponentName2;

VOID
MouseReportStatusCode (
  IN USB_MOUSE_DEV             *UsbMouseDevice,
  IN EFI_STATUS_CODE_TYPE      CodeType,
  IN EFI_STATUS_CODE_VALUE     Value
  );

#endif

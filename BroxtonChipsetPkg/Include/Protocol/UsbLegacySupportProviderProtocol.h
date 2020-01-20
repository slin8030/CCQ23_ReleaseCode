/** @file
  Usb Legacy Support Provider Protocol Guid definition file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _USB_LEGACY_SUPPORT_PROVIDER_PROTOCOL_H_
#define _USB_LEGACY_SUPPORT_PROVIDER_PROTOCOL_H_

#define USB_LEGACY_SUPPORT_PROVIDER_PROTOCOL_GUID  \
  { 0x0d05c550, 0xac28, 0x46d8, { 0x8a, 0xe6, 0xda, 0x2b, 0xfb, 0x2c, 0xb5, 0x48 }}

extern EFI_GUID gUsbLegacySupportProviderProtocolGuid;

typedef struct _USB_LEGACY_SUPPORT_PROVIDER_PROTOCOL   EFI_USB_LEGACY_SUPPORT_PROVIDER_PROTOCOL;

struct _USB_LEGACY_SUPPORT_PROVIDER_PROTOCOL {
  VOID*   Private;
};

#endif

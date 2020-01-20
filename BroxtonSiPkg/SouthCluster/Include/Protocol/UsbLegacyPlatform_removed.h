/** @file
  Protocol used to get the platform policy on USB legacy operation.

@copyright
 Copyright (c) 2004 - 2015 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains an 'Intel Peripheral Driver' and is uniquely
 identified as "Intel Reference Module" and is licensed for Intel
 CPUs and chipsets under the terms of your license agreement with
 Intel or your vendor. This file may be modified by the user, subject
 to additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef _USB_LEGACY_PLATFORM_H_
#define _USB_LEGACY_PLATFORM_H_

EFI_FORWARD_DECLARATION(EFI_USB_LEGACY_PLATFORM_PROTOCOL);


#define EFI_USB_LEGACY_PLATFORM_PROTOCOL_GUID \
  {0x13f40f6e, 0x50c1, 0x4b73, 0xb1, 0xe2, 0x6e, 0x72, 0xd2, 0x1c, 0xb0, 0x4a}

typedef struct {
    UINT16            ShiftKey     :1;
    UINT16            AltKey       :1;
    UINT16            CtrlKey      :1;
    UINT16            Reserved     :13;
} KEY_ATTRIBUTE;

typedef struct {
    UINT8            ScanCode;
    KEY_ATTRIBUTE    Keyattribute;
} KEY_ELEMENT;

///
/// UsbLegacyEnable
///    0b = Disabled
///    1b = Enabled
/// UsbBoot
///    00b = Not supported
///    01b = Disabled
///    10b = Enabled
///    11b = Reserved
/// UsbZip
///    00b = Not supported
///    01b = Disabled
///    10b = Enabled
///    11b = Reserved
/// Usb Zip Emulation
///    00b = Floppy
///    01b = HDD
///    10b = Reserved
///    11b = Reserved
/// UsbFixedDiskWpBootSector
///    00b = Not supported
///    01b = Disabled
///    10b = Enabled
///    11b = Reserved
/// Usb Ehci Enable
///     0b = Disable
///     1b = Enable
/// Usb Mass Device Emulation
///    00b = Use default
///    01b = Floppy
///    10b = Hard Disk
///    11b = Size-based emulation
///
typedef struct {
    UINT32            UsbLegacyEnable          :1;
    UINT32            UsbBoot                  :2;
    UINT32            UsbZip                   :2;
    UINT32            UsbZipEmulation          :2;
    UINT32            UsbFixedDiskWpBootSector :2;
//UsbLegacyPlatform.165_001-- UINT32            Reserved                     :23;
    UINT32            UsbEhciEnable            :1;   ///<UsbLegacyPlatform.165_001++
    UINT32            UsbMassEmulation         :2;
    UINT32            Reserved                 :20;  ///<UsbLegacyPlatform.165_001++
    UINT32            UsbMassEmulationSizeLimit;
} USB_LEGACY_MODIFIERS;

///
/// Here is the protocol
///
/**
  Get SETUP/platform options for USB Legacy operation modification.

  @param[in]   This                - Protocol instance pointer.
  @param[out]  UsbLegacyModifiers  - List of keys to monitor from. This includes both
                                     PS2 and USB keyboards.

  @retval  EFI_SUCCESS   - Modifiers exist.
  @retval  EFI_NOT_FOUND - Modifiers not not exist.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_USB_PLATFORM_OPTIONS) (
  IN   EFI_USB_LEGACY_PLATFORM_PROTOCOL *This,
  OUT  USB_LEGACY_MODIFIERS           *UsbLegacyModifiers
  );

/**
  Return a list of keys to monitor for.

  @param[in]    This          - Protocol instance pointer.
  @param[out]   KeyList       - List of keys to monitor from. This includes both
                                USB & PS2 keyboard inputs.
  @param[out]   KeyListSize   - Size of KeyList in bytes

  @retval  EFI_SUCCESS   - Keys are to be monitored.
  @retval  EFI_NOT_FOUND - No keys are to be monitored.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_PLATFORM_MONITOR_KEY_OPTIONS) (
  IN   EFI_USB_LEGACY_PLATFORM_PROTOCOL *This,
  OUT  KEY_ELEMENT                    **KeyList,
  OUT  UINTN                          *KeyListSize
  );

/**
  This protocol implement the interface to get USB legacy platform options.
**/
typedef struct _EFI_USB_LEGACY_PLATFORM_PROTOCOL{
  EFI_GET_USB_PLATFORM_OPTIONS           GetUsbPlatformOptions;
  EFI_GET_PLATFORM_MONITOR_KEY_OPTIONS   GetPlatformMonitorKeyOptions;
} EFI_USB_LEGACY_PLATFORM_PROTOCOL;

extern EFI_GUID gEfiUsbLegacyPlatformProtocolGuid;

#endif

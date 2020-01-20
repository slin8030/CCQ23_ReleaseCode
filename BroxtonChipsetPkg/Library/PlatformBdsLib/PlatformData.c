/** @file
  PlatformBdsLib

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

/** 
  Defined the platform specific device path which will be used by
  platform Bbd to perform the platform policy connect.

Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "BdsPlatform.h"


//
// Predefined platform root bridge
//
PLATFORM_ROOT_BRIDGE_DEVICE_PATH gPlatformRootBridge0 = {
  gPciRootBridge,
  gEndEntire
};

EFI_DEVICE_PATH_PROTOCOL* gPlatformRootBridges [] = {
  (EFI_DEVICE_PATH_PROTOCOL*)&gPlatformRootBridge0,
  NULL
};

//
// Platform specific ISA keyboard device path
//
PLATFORM_ISA_KEYBOARD_DEVICE_PATH gIsaKeyboardDevicePath = {
  gPciRootBridge,
  gPciIsaBridge,
  gPnpPs2Keyboard,
  gEndEntire
};

//
// Platform specific ISA serial device path
//
PLATFORM_ISA_SERIAL_DEVICE_PATH gIsaSerialDevicePath = {
  gPciRootBridge,
  gPciIsaBridge,
  gPnp16550ComPort,
  gUart(115200, 8, 1, 1),
  gPcAnsiTerminal,
  gEndEntire
};

#if TABLET_PF_ENABLE
//
// Platform specific Button Array device path
//
HII_VENDOR_DEVICE_PATH  gHiiVendorDevicePath0 = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    //
    // {C8752FDE-B5C8-4528-897D-6920FE771E38}
    //
    { 0xC8752FDE, 0xB5C8, 0x4528, { 0x89, 0x7D, 0x69, 0x20, 0xFE, 0x77, 0x1E, 0x38 } }
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};
#endif

USB_CLASS_FORMAT_DEVICE_PATH gUsbClassKeyboardDevicePath = {
  gUsbKeyboardMouse,
  gEndEntire
};


//
// Predefined platform default console device path
//
BDS_CONSOLE_CONNECT_ENTRY gPlatformConsole [] = {
#if TABLET_PF_ENABLE
  {(EFI_DEVICE_PATH_PROTOCOL*)&gHiiVendorDevicePath0, CONSOLE_IN},
  {(EFI_DEVICE_PATH_PROTOCOL*)&gIsaSerialDevicePath, (CONSOLE_OUT | CONSOLE_IN)},
#endif  
  {(EFI_DEVICE_PATH_PROTOCOL*)&gIsaKeyboardDevicePath, CONSOLE_IN},
  {NULL, 0}
};

//
// Platform specific on chip PCI VGA device path
//
PLATFORM_ONBOARD_VGA_DEVICE_PATH gOnChipPciVgaDevicePath = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE(0, 0x2),
  gEndEntire
};

//
// Platform specific plug in PCI VGA device path
//
PLATFORM_OFFBOARD_VGA_DEVICE_PATH gPlugInPciVgaDevicePath = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE(0, 0x1),
  PCI_DEVICE_PATH_NODE(0, 0x0),
  gEndEntire
};

//
// Platform specific on chip internal graphics device path
//
PLATFORM_ONBOARD_VGA_DEVICE_PATH gOnChipIgfxDevicePath = {
  gPciRootBridge,
  {
    HARDWARE_DEVICE_PATH,
    HW_PCI_DP,
    (UINT8)(sizeof(PCI_DEVICE_PATH)),
    (UINT8)((sizeof(PCI_DEVICE_PATH)) >> 8),
    0,
    0x2
  },
  {
    ACPI_DEVICE_PATH,
    ACPI_ADR_DP,
    (UINT8) (sizeof (ACPI_ADR_DEVICE_PATH)),
    (UINT8) ((sizeof (ACPI_ADR_DEVICE_PATH)) >> 8),
    ACPI_DISPLAY_ADR (1, 0, 0, 1, 0, ACPI_ADR_DISPLAY_TYPE_VGA, 0, 0)
  },
  gEndEntire
};





//
// All the possible platform on chip internal graphics device path
//
EFI_DEVICE_PATH_PROTOCOL* gPlatformAllPossibleIgfxConsole [] = {
  (EFI_DEVICE_PATH_PROTOCOL*)&gOnChipIgfxDevicePath, 
  NULL
};

//
// All the possible platform PEG device path
//
EFI_DEVICE_PATH_PROTOCOL* gPlatformAllPossiblePegConsole [] = {
  (EFI_DEVICE_PATH_PROTOCOL*)&gPlugInPciVgaDevicePath, // PEG bridge (0x00/0x01/0x00) VGA device (D0/F0), Highest priority
  NULL
};

//
// All the possible platform PCIE graphics device path
//
EFI_DEVICE_PATH_PROTOCOL* gPlatformAllPossiblePcieConsole [] = {
  NULL
};

//
// All the possible platform PCI graphics device path
//
EFI_DEVICE_PATH_PROTOCOL* gPlatformAllPossiblePciConsole [] = {
  NULL
};


//
// Legacy hard disk boot option 
//
LEGACY_HD_DEVICE_PATH gLegacyHd = {
  {
    BBS_DEVICE_PATH,
    BBS_BBS_DP,
    (UINT8)(sizeof(BBS_BBS_DEVICE_PATH)),
    (UINT8)((sizeof(BBS_BBS_DEVICE_PATH)) >> 8),
    BBS_TYPE_HARDDRIVE,
    0,
    0
  },
  gEndEntire
};

//
// Legacy cdrom boot option
//
LEGACY_HD_DEVICE_PATH gLegacyCdrom = {
  {
    BBS_DEVICE_PATH,
    BBS_BBS_DP,
    (UINT8)(sizeof(BBS_BBS_DEVICE_PATH)),
    (UINT8)((sizeof(BBS_BBS_DEVICE_PATH)) >> 8),
    BBS_TYPE_CDROM,
    0,
    0
  },
  gEndEntire
};




//
// Predefined platform specific perdict boot option
//
EFI_DEVICE_PATH_PROTOCOL* gPlatformBootOption [] = {
  (EFI_DEVICE_PATH_PROTOCOL*)&gLegacyHd,
  (EFI_DEVICE_PATH_PROTOCOL*)&gLegacyCdrom,
  NULL
};


//
// Predefined platform specific driver option
//
EFI_DEVICE_PATH_PROTOCOL* gPlatformDriverOption [] = {
  NULL
};

//
// Predefined platform connect sequence
//
EFI_DEVICE_PATH_PROTOCOL* gPlatformConnectSequence [] = {
  (EFI_DEVICE_PATH_PROTOCOL *)&gPlatformRootBridge0,  // Force PCI enumer before Legacy OpROM shadow
  NULL
};


//
// Platform specific USB controller device path
//
PLATFORM_USB_DEVICE_PATH gUsbDevicePath0 = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE(0, 0x1D),
  gEndEntire
};

PLATFORM_USB_DEVICE_PATH gUsbDevicePath1 = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE(1, 0x1D),
  gEndEntire
};

PLATFORM_USB_DEVICE_PATH gUsbDevicePath2 = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE(2, 0x1D),
  gEndEntire
};

PLATFORM_USB_DEVICE_PATH gUsbDevicePath3 = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE(3, 0x1D),
  gEndEntire
};

//
// Predefined platform device path for user authtication
//
EFI_DEVICE_PATH_PROTOCOL* gUserAuthenticationDevice[] = {
  //
  // Predefined device path for secure card (USB disk).
  //
  (EFI_DEVICE_PATH_PROTOCOL*)&gUsbDevicePath0,
  (EFI_DEVICE_PATH_PROTOCOL*)&gUsbDevicePath1,
  (EFI_DEVICE_PATH_PROTOCOL*)&gUsbDevicePath2,
  (EFI_DEVICE_PATH_PROTOCOL*)&gUsbDevicePath3,
  NULL
};



//
// This file contains 'Framework Code' and is licensed as such 
// under the terms of your license agreement with Intel or your
// vendor.  This file may not be modified, except as allowed by
// additional terms of your license agreement.                 
//
/*++

Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  BdsPlatform.h

Abstract:

  Head file for BDS Platform specific code

--*/

#ifndef _BDS_PLATFORM_H
#define _BDS_PLATFORM_H

#include <FrameworkDxe.h>

#include <Protocol/FirmwareVolume2.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SimpleNetwork.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/LoadFile.h>
#include <Protocol/LegacyBios.h>
#include <Protocol/PciIo.h>
#include <Protocol/SmmAccess2.h>
#include <Protocol/DxeSmmReadyToLock.h>
#include <Protocol/UserManager.h>
#include <Protocol/DeferredImageLoad.h>
#include <Protocol/AcpiS3Save.h>
#include <Protocol/ExitPmAuth.h>
#include <Guid/CapsuleVendor.h>
#include <Guid/MemoryTypeInformation.h>
#include <Guid/GlobalVariable.h>


#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/GenericBdsLib.h>
#include <Library/PlatformBdsLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#include <Library/PrintLib.h>
#include <Library/PerformanceLib.h>
#include <Library/ReportStatusCodeLib.h>

#include <IndustryStandard/Pci.h>

extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformRootBridges [];
extern BDS_CONSOLE_CONNECT_ENTRY gPlatformConsole [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformAllPossiblePciVgaConsole [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformConnectSequence [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformDriverOption [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformBootOption [];
extern EFI_DEVICE_PATH_PROTOCOL  *gUserAuthenticationDevice[];
extern BDS_CONSOLE_CONNECT_ENTRY gPlatformSimpleConsole [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformSimpleBootOption [];

//
// the short form device path for Usb keyboard
//
#define CLASS_HID           3
#define SUBCLASS_BOOT       1
#define PROTOCOL_KEYBOARD   1

#define PCI_DEVICE_PATH_NODE(Func, Dev) \
  { \
    HARDWARE_DEVICE_PATH, \
    HW_PCI_DP, \
    { \
      (UINT8) (sizeof (PCI_DEVICE_PATH)), \
      (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8) \
    }, \
    (Func), \
    (Dev) \
  }

#define PNPID_DEVICE_PATH_NODE(PnpId) \
  { \
    { \
      ACPI_DEVICE_PATH, \
      ACPI_DP, \
      { \
        (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)), \
        (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8) \
      } \
    }, \
    EISA_PNP_ID((PnpId)), \
    0 \
  }

#define gUart(BaudRate, DataBits, Parity, StopBits) \
  { \
    { \
      MESSAGING_DEVICE_PATH, \
      MSG_UART_DP, \
      { \
        (UINT8) (sizeof (UART_DEVICE_PATH)), \
        (UINT8) ((sizeof (UART_DEVICE_PATH)) >> 8) \
      } \
    }, \
    0, \
    (BaudRate), \
    (DataBits), \
    (Parity), \
    (StopBits) \
  }

#define gPcAnsiTerminal \
  { \
    { \
      MESSAGING_DEVICE_PATH, \
      MSG_VENDOR_DP, \
      { \
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)), \
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8) \
      } \
    }, \
    DEVICE_PATH_MESSAGING_PC_ANSI \
  }

#define gUsbKeyboardMouse \
  { \
    { \
      MESSAGING_DEVICE_PATH, \
      MSG_USB_CLASS_DP, \
      (UINT8) (sizeof (USB_CLASS_DEVICE_PATH)), \
      (UINT8) ((sizeof (USB_CLASS_DEVICE_PATH)) >> 8) \
    }, \
    0xffff, \
    0xffff, \
    CLASS_HID, \
    SUBCLASS_BOOT, \
    PROTOCOL_KEYBOARD \
  }

#define gEndEntire \
  { \
    END_DEVICE_PATH_TYPE, \
    END_ENTIRE_DEVICE_PATH_SUBTYPE, \
    { \
      END_DEVICE_PATH_LENGTH, \
      0 \
    } \
  }

#define gPciRootBridge \
  PNPID_DEVICE_PATH_NODE(0x0A03)

#define gPnpPs2Keyboard \
  PNPID_DEVICE_PATH_NODE(0x0303)

#define gPnp16550ComPort \
  PNPID_DEVICE_PATH_NODE(0x0501)

#define gPciePort0Bridge \
  PCI_DEVICE_PATH_NODE(0, 0x1C)

#define gPciePort1Bridge \
  PCI_DEVICE_PATH_NODE(1, 0x1C)

#define gPciePort2Bridge \
  PCI_DEVICE_PATH_NODE(2, 0x1C)

#define gPciePort3Bridge \
  PCI_DEVICE_PATH_NODE(3, 0x1C)

#define gPciIsaBridge \
  PCI_DEVICE_PATH_NODE(0, 0x1f)

//
// Platform Root Bridge
//
typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_ROOT_BRIDGE_DEVICE_PATH;

//
// Below is the platform console device path
//
typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           IsaBridge;
  ACPI_HID_DEVICE_PATH      Keyboard;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_ISA_KEYBOARD_DEVICE_PATH;

typedef struct {
  USB_CLASS_DEVICE_PATH           UsbClass;
  EFI_DEVICE_PATH_PROTOCOL        End;
} USB_CLASS_FORMAT_DEVICE_PATH;  

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           OnboardVga;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_ONBOARD_VGA_DEVICE_PATH;

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           AgpBridge;
  PCI_DEVICE_PATH           AgpDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_OFFBOARD_VGA_DEVICE_PATH;

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           IsaBridge;
  ACPI_HID_DEVICE_PATH      IsaSerial;
  UART_DEVICE_PATH          Uart;
  VENDOR_DEVICE_PATH        TerminalType;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_ISA_SERIAL_DEVICE_PATH;

//
// Below is the boot option device path
//
typedef struct {
  BBS_BBS_DEVICE_PATH             LegacyHD;
  EFI_DEVICE_PATH_PROTOCOL        End;
} LEGACY_HD_DEVICE_PATH;

//
// Below is the platform IDE device path
//
typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           IsaBridge;
  ATAPI_DEVICE_PATH         Ide;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_IDE_DEVICE_PATH;

//
// Floppy device path definition
//
typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           IsaBridge;
  ACPI_HID_DEVICE_PATH      Floppy;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_FLOPPY_DEVICE_PATH;

//
// Below is the platform USB controller device path for 
// USB disk as user authentication device. 
//
typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           PciDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_USB_DEVICE_PATH;

//
// Below is the platform PCI device path
//
typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           PciDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_PCI_DEVICE_PATH;

//
// Platform BDS Functions
//
VOID
PlatformBdsGetDriverOption (
  IN LIST_ENTRY                   *BdsDriverLists
  );
  
VOID
PlatformBdsPredictBootOption (
  IN  LIST_ENTRY                     *BdsBootOptionList
  );

EFI_STATUS
PlatformBdsShowProgress (
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL TitleForeground,
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL TitleBackground,
  CHAR16                        *Title,
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL ProgressColor,
  UINTN                         Progress,
  UINTN                         PreviousValue
  );
  
VOID
PlatformBdsConnectSequence (
  VOID
  );

EFI_STATUS
PlatformBdsConnectConsole (
  IN BDS_CONSOLE_CONNECT_ENTRY   *PlatformConsole
  );

EFI_STATUS
PlatformBdsNoConsoleAction (
  VOID
  );

VOID
PlatformBdsEnterFrontPage (
  IN UINT16                 TimeoutDefault,
  IN BOOLEAN                ConnectAllHappened
  );

VOID
EFIAPI
PlatformBdsUserIdentify (
  OUT EFI_USER_PROFILE_HANDLE        *User,
  OUT BOOLEAN                        *DeferredImage
  );

VOID
EFIAPI
PlatformBdsConnectAuthDevice (
  VOID
  );

VOID
PlatformBdsEnterFrontPageWithHotKey (
  IN UINT16                       TimeoutDefault,
  IN BOOLEAN                      ConnectAllHappened
  );

 EFI_STATUS
 ShowProgress (
   IN UINT16					   TimeoutDefault
   );

 EFI_STATUS
 InitializeFrontPage (
   IN BOOLEAN						  InitializeHiiData
   );
 
 VOID
 UpdateFrontPageStrings (
   VOID
   );

 EFI_STATUS
 CallFrontPage (
   VOID
   );

 
 VOID
 CallBootManager (
   VOID
   );

VOID
CallDeviceManager (
  VOID
  );

VOID
BdsStartBootMaint (
  VOID
  );

CHAR16 *
GetStringById (
  IN  EFI_STRING_ID   Id
  );

EFI_STATUS
WaitForSingleEvent (
  IN EFI_EVENT                  Event,
  IN UINT64                     Timeout OPTIONAL
  );

// shoud be removed///////////////////////////
EFI_STATUS
EFIAPI
OsipLoadOsImage (
  IN    UINTN                           ImageNumber,
  OUT   EFI_PHYSICAL_ADDRESS            *ImageBase,
  OUT   UINTN                           *ImageSize,
  OUT   EFI_PHYSICAL_ADDRESS            *EntryPoint
  );

EFI_STATUS
EFIAPI
OsipLoadOsImageFromEmmc (
  IN    UINTN                           ImageNumber,
  OUT   EFI_PHYSICAL_ADDRESS            *ImageBase,
  OUT   UINTN                           *ImageSize,
  OUT   EFI_PHYSICAL_ADDRESS            *EntryPoint
  );
VOID
JumpToVector (
  IN VOID     *EntryPoint,
  IN UINTN    Parameter
  );
  
EFI_STATUS
InstallLegacyAcpi (
  );

EFI_STATUS
LegacyBiosBuildE820 (
//  IN  LEGACY_BIOS_INSTANCE    *Private,
//  OUT UINTN                   *Size
  );

//////////////////////////////////

EFI_STATUS
BdsBootAndroidFromUsb (
	VOID
  );

EFI_STATUS
BdsBootAndroidFromEmmc (
	UINT8	*FileBuffer
  );




#define ONE_SECOND  10000000
#define FRONT_PAGE_KEY_CONTINUE        0x1000
#define FRONT_PAGE_KEY_LANGUAGE        0x1234
#define FRONT_PAGE_KEY_BOOT_MANAGER    0x1064
#define FRONT_PAGE_KEY_DEVICE_MANAGER  0x8567
#define FRONT_PAGE_KEY_BOOT_MAINTAIN   0x9876

#define PORT_A_DVO                     0           // ; DVO A
#define PORT_B_DVO                     1           // ; DVO B
#define PORT_C_DVO                     2           // ; DVO C
#define PORT_D_DVO                     3           // ; DVO D
#define PORT_LVDS                      4           // ; Integrated LVDS port
#define PORT_ANALOG_TV                 5           // ; Integrated TV port
#define PORT_CRT                       6           // ; integrated Analog port
#define PORT_B_DP                      7           // ; DisplayPort B
#define PORT_C_DP                      8           // ; DisplayPort C
#define PORT_D_DP                      9           // ; DisplayPort D
#define PORT_A_DP                      10          // ; DisplayPort A (for eDP on ILK)
#define PORT_B_HDMI                    11          // ; HDMI B
#define PORT_C_HDMI                    12          // ; HDMI C
#define PORT_D_HDMI                    13          // ; HDMI D
#define PORT_B_DVI                     14          // ; DVI B
#define PORT_C_DVI                     15          // ; DVI C
#define PORT_D_DVI                     16          // ; DVI D
#define PORT_MIPI_A                    21          // ; MIPI
#define PORT_MIPI_B                    22   
#define PORT_MIPI_C                    23   

 
 extern BOOLEAN gConnectAllHappened;
 extern UINTN gCallbackKey;

VOID
BdsBootDeviceSelect (
  VOID
);
VOID FastBoot(VOID);

extern BOOLEAN    mModeInitialized;

//
// Boot video resolution and text mode.
//
extern UINT32     mBootHorizontalResolution    ;
extern UINT32     mBootVerticalResolution      ;
extern UINT32     mBootTextModeColumn          ;
extern UINT32     mBootTextModeRow             ;
//
// BIOS setup video resolution and text mode.
//
extern UINT32     mSetupTextModeColumn         ;
extern UINT32     mSetupTextModeRow            ;
extern UINT32     mSetupHorizontalResolution   ;
extern UINT32     mSetupVerticalResolution     ;


extern EFI_STATUS BdsSetConsoleMode (BOOLEAN);
#endif // _BDS_PLATFORM_H

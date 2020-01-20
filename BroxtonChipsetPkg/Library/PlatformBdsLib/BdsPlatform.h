/** @file
  PlatformBdsLib

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/** 
  Head file for BDS Platform specific code

Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _BDS_PLATFORM_H_
#define _BDS_PLATFORM_H_

#include <PiDxe.h>
#include <IndustryStandard/SmBios.h>
#include <IndustryStandard/Pci.h> 
#include <Uefi/UefiInternalFormRepresentation.h>

#include <Protocol/SetupUtility.h>
#include <Protocol/GenericMemoryTest.h>
#include <Protocol/ExitPmAuth.h>
#include <Protocol/ConOutDevStarted.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SetupUtility.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/LegacyBios.h>
#include <Protocol/LegacyRegion2.h>
#include <Protocol/OEMBadgingSupport.h>
#include <Protocol/UgaSplash.h>
#include <Protocol/ConsoleControl.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/H2ODialog.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/CpuIo.h>
#include <Protocol/SkipScanRemovableDev.h>
#include <Protocol/PlatformGopPolicy.h>
#include <Protocol/IsaAcpi.h>
#include <Protocol/SerialIo.h>
#include <Protocol/TerminalEscCode.h>
#include <Protocol/AcpiS3Save.h>
#include <Protocol/AlertStandardFormat.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/SimpleNetwork.h>
#include <Protocol/LoadFile.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DiskInfo.h>
#include <Protocol/ConsoleControl.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/Smbios.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/BootLogo.h>
#include <Protocol/SmmAccess2.h>
#include <Protocol/DxeSmmReadyToLock.h>
#include <Protocol/DeferredImageLoad.h>
#include <Protocol/ConsoleRedirectionService.h>

#include <Guid/GlobalVariable.h>
#include <Guid/DebugMask.h>
#include <Guid/CapsuleVendor.h>
#include <Guid/MdeModuleHii.h>
#include <Guid/BdsHii.h>
//[-start-151124-IB07220021-remove]//
// #include <Guid/ConsoleRedirectionSupport.h>
//[-end-151124-IB07220021-remove]//

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/HobLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/OemGraphicsLib.h>
#include <Library/PrintLib.h>
#include <Library/HiiLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PlatformBdsLib.h>
#include <Library/GenericBdsLib.h>
#include <Library/GenericUtilityLib.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <Library/DxeOemSvcChipsetLib.h>
//[-start-160920-IB07400786-add]//
#include <Library/GpioLib.h>
#include <Library/SideBandLib.h>
//[-end-160920-IB07400786-add]//

#include <PostCode.h>
#include <ChipsetSetupConfig.h>
#if ENBDT_PF_ENABLE  
#include <ScRegs/RegsPcu.h>
#include <Library/S3BootScriptLib.h>
#include "ScAccess.h"
#include "ScRegs/RegsSata.h"
#endif



extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformRootBridges [];
extern BDS_CONSOLE_CONNECT_ENTRY gPlatformConsole [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformAllPossibleIgfxConsole [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformAllPossiblePegConsole [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformAllPossiblePcieConsole [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformAllPossiblePciConsole [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformConnectSequence [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformDriverOption [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformBootOption [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformConnectSata[];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformConnectLastBoot[];
extern EFI_DEVICE_PATH_PROTOCOL  *gUserAuthenticationDevice[];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformConnectSd [];
extern EFI_DEVICE_PATH_PROTOCOL  *gPlatformConnectEmmc [];

typedef struct {
  UINTN  Seg;
  UINTN  Bus;
  UINTN  Dev;
  UINTN  Fun;
} PCI_DEV_DEF;

#ifndef PCICMD
#define PCICMD                          (0x4)
#endif

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

#define CONTROLLER_DEVICE_PATH_NODE(Ctrl) \
  { \
    HARDWARE_DEVICE_PATH, \
    HW_CONTROLLER_DP, \
    { \
      (UINT8) (sizeof (CONTROLLER_DEVICE_PATH)), \
      (UINT8) ((sizeof (CONTROLLER_DEVICE_PATH)) >> 8) \
    }, \
    (Ctrl) \
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

#define gPciRootBridge \
  {\
    ACPI_DEVICE_PATH,\
    ACPI_DP,\
    (UINT8)(sizeof(ACPI_HID_DEVICE_PATH)),\
    (UINT8)((sizeof(ACPI_HID_DEVICE_PATH)) >> 8),\
    EISA_PNP_ID(0x0A03),\
    0\
  }

#define gEndEntire \
  {\
    END_DEVICE_PATH_TYPE,\
    END_ENTIRE_DEVICE_PATH_SUBTYPE,\
    END_DEVICE_PATH_LENGTH,\
    0\
  }

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
// Temporary console variables used in the ConnectConsole
//
#define CON_OUT_CANDIDATE_NAME    L"ConOutCandidateDev"
#define CON_IN_CANDIDATE_NAME     L"ConInCandidateDev"
#define ERR_OUT_CANDIDATE_NAME    L"ErrOutCandidateDev"

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
  PCI_DEVICE_PATH           AgpBridge;
  PCI_DEVICE_PATH           AgpDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_OFFBOARD_VGA_DEVICE_PATH;

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           IsaBridge;
  ACPI_HID_DEVICE_PATH      Mouse;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_MOUSE_DEVICE_PATH;

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           OnboardVga;
  ACPI_ADR_DEVICE_PATH      DisplayDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_ONBOARD_VGA_DEVICE_PATH;

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           AgpBridge;
  PCI_DEVICE_PATH           AgpDevice;
  ACPI_ADR_DEVICE_PATH      DisplayDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_PLUG_IN_VGA_DEVICE_PATH;

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           IsaBridge;
  ACPI_HID_DEVICE_PATH      IsaSerial;
  UART_DEVICE_PATH          Uart;
  VENDOR_DEVICE_PATH        TerminalType;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_ISA_SERIAL_DEVICE_PATH;

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           MmcBridge;
  CONTROLLER_DEVICE_PATH    Controller;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_MMC_PARTITION_DEVICE_PATH;

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
// Below is the platform console device path
//

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           IsaBridge;
  SATA_DEVICE_PATH          SataBridge;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_SATA_DEVICE_PATH;

//
// Below is the boot option device path
//
typedef struct {
  BBS_BBS_DEVICE_PATH             LegacyHD;
  EFI_DEVICE_PATH_PROTOCOL        End;
} LEGACY_HD_DEVICE_PATH;



typedef enum {
  Igfx  = 0,
  Peg,
  Pcie,
  Pci,
  PossibleVgaTypeMax
} POSSIBLE_VGA_TYPE;

typedef enum {
  DisplayModeIgfx  = 0,
  DisplayModePeg,
  DisplayModePci,
  DisplayModeAuto,
  DisplayModeSg
} PRIMARY_DISPLAY;

typedef struct {
  EFI_HANDLE                   Handle;
  POSSIBLE_VGA_TYPE            VgaType;
  UINT8                        Priority;
} VGA_DEVICE_INFO;

typedef struct {
  UINTN                        VgaHandleConut;
  EFI_HANDLE                   PrimaryVgaHandle;
  EFI_HANDLE                   *VgaHandleBuffer;
} VGA_HANDLES_INFO;

#if TABLET_PF_ENABLE
typedef enum {
  PMIC_Equal         = 0, // =    0
  PMIC_Greater_Than,    // >    1
  PMIC_Smaller_Than,    // <    2
  PMIC_Greater_Equal,    // >=    3
  PMIC_Smaller_Equal,    // <=    4
  PMIC_Any          // don't care 5
} PMIC_Condition_list;

typedef enum {
  PMIC_White_List  = 0,  //White list
  PMIC_Black_List  = 1   //Black list
} PMIC_Compliance_mode;

typedef struct {
  UINT8    Cond_Choice;  // PMIC_Condition_list
  UINT8    Cond_Number;    // the number
}PMIC_Condition_Item;

typedef struct {
  PMIC_Condition_Item             PMIC_BoardID;
  PMIC_Condition_Item             PMIC_FabID;
  PMIC_Condition_Item             Soc_Stepping;//define PMIC type, 1:Dialog , 2:Rohm 
  PMIC_Condition_Item             PMIC_VendID;
  PMIC_Condition_Item             PMIC_RevID;
  PMIC_Compliance_mode                 mode;        //if 1, blacklist; if 0, white list.
} PMIC_Compliance_Item;
#endif

VOID
ChipsetPrepareReadyToBootEvent (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  );

EFI_STATUS
PlatformBdsNoConsoleAction (
  VOID
  );

EFI_STATUS
PlatformBdsBootDisplayDevice (
  IN CHIPSET_CONFIGURATION       *SetupNVRam
  );

EFI_STATUS
PlatformBdsBootSelection (
  UINT16                                    Selection,
  UINT16                                    Timeout
  );

EFI_STATUS
PlatformBdsConnectDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL    *HandleDevicePath,
  IN  BOOLEAN                     ConnectChildHandle,
  IN  BOOLEAN                     DispatchPossibleChild
  );

EFI_STATUS
UpdateLastBootDevicePath (
  IN   UINT16                     *BootPrevious,
  OUT  EFI_DEVICE_PATH_PROTOCOL   **LastBootDevicePath
  );

EFI_STATUS
PlatformBdsConnectConsoleMini (
  IN  BDS_CONSOLE_CONNECT_ENTRY   *PlatformConsole
  );

EFI_STATUS
ExitBootServiceSetVgaMode (
  IN CHIPSET_CONFIGURATION                  *SetupVariable,
  IN EFI_BOOT_MODE                          BootMode
  ); 

VOID
ExitBootServiceSetVgaModeCallback (
  EFI_EVENT  Event,
  VOID       *Context
  );

EFI_STATUS
EFIAPI
SetVgaCommandRegReadyToBootEvent (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  );

BOOLEAN
FoundUSBConsoleOutput (
  OUT EFI_DEVICE_PATH_PROTOCOL  **UsbConsoleOuputDevPath
  );

VOID
SetUsbConsoleOutToConOutVar (
  IN EFI_DEVICE_PATH_PROTOCOL  *UsbConsoleOuputDevPath
  ); 

EFI_STATUS
BdsMatchDevicePaths (
  IN  EFI_DEVICE_PATH_PROTOCOL  * Multi,
  IN  EFI_DEVICE_PATH_PROTOCOL  * Single,
  IN  EFI_DEVICE_PATH_PROTOCOL  **NewDevicePath OPTIONAL,
  IN  BOOLEAN                   Delete
  );

VOID
EFIAPI
PlatformBdsUserIdentify (
  OUT EFI_USER_PROFILE_HANDLE        *User,
  OUT BOOLEAN                        *DeferredImageExist
  );
  
VOID
InstallExitPmAuth (
  VOID
  );
    
VOID
EFIAPI
PlatformBdsConnectAuthDevice (
  VOID
  );
  
#if TABLET_PF_ENABLE  
EFI_STATUS
WaitForSingleEvent (
  IN EFI_EVENT                  Event,
  IN UINT64                     Timeout OPTIONAL
  );

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
  VOID
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
  VOID
  );
#endif

#if ENBDT_PF_ENABLE  
VOID
DisableAhciCtlr (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  );
#endif


#endif

/** @file
 BDS library definition, include the file and data structure

;******************************************************************************
;* Copyright (c) 2012 - 2018, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

Copyright (c) 2004 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

*/

#ifndef _INTERNAL_BDS_LIB_H_
#define _INTERNAL_BDS_LIB_H_

#include <FrameworkDxe.h>

#include <IndustryStandard/Pci.h>
#include <IndustryStandard/PeImage.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/SmBios.h>
#include <IndustryStandard/Bmp.h>

#include <Protocol/BlockIo.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/Cpu.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadFile.h>
#include <Protocol/DebugPort.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimpleNetwork.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/PciIo.h>
#include <Protocol/AcpiS3Save.h>
#include <Protocol/OEMBadging.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/UgaDraw.h>
#include <Protocol/HiiFont.h>
#include <Protocol/HiiImage.h>
#include <Protocol/UsbIo.h>
#include <Protocol/SetupUtility.h>
#include <Protocol/Ebc.h>
#include <Protocol/OEMBadgingSupport.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/LegacyBiosPlatform.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/StartOfBdsDiagnostics.h>
#include <Protocol/BootOptionPolicy.h>
#include <Protocol/EndOfBdsBootSelection.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/DiskInfo.h>
#include <Protocol/SkipScanRemovableDev.h>
#include <Protocol/H2ODialog.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/AcpiSupport.h>
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/ConsoleControl.h>
#include <Protocol/BootLogo.h>
#include <Protocol/Smbios.h>
#include <Protocol/BootLogo.h>
#include <Protocol/MonitorKeyFilter.h>
#include <Protocol/SysPasswordService.h>
#include <Protocol/UnicodeCollation.h>
#include <Protocol/HddPasswordService.h>
#include <Protocol/H2OBdsServices.h>
#include <Protocol/H2OBdsBootGroup.h>
#include <Protocol/I2cMaster.h>

#include <Guid/MemoryTypeInformation.h>
#include <Guid/FileInfo.h>
#include <Guid/GlobalVariable.h>
#include <Guid/PcAnsi.h>
#include <Guid/Performance.h>
#include <Guid/BdsLibHii.h>
#include <Guid/HdBootVariable.h>
#include <Guid/LastEnumLang.h>
#include <Guid/DebugMask.h>
#include <Guid/AdmiSecureBoot.h>
#include <Guid/RecoveryFileAddress.h>
#include <Guid/UsbEnumeration.h>
#include <Guid/FirmwarePerformance.h>
#include <Guid/BootOrderHook.h>
#include <Guid/H2OBdsCheckPoint.h>
#include <Guid/BdsBootDeviceGroup.h>
#include <Guid/HddPasswordVariable.h>

#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/BaseLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/TimerLib.h>
#include <Library/PcdLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/OemGraphicsLib.h>
#include <Library/CmosLib.h>
#include <Library/GenericBdsLib.h>
#include <Library/KernelConfigLib.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <Library/BdsCpLib.h>
#include <Library/VariableLib.h>

#include <SmiTable.h>
#include <PostCode.h>

#include "PerformanceTokens.h"
#include "String.h"
#include "Checkpoint.h"

#include <PortNumberMap.h>

#if !defined (EFI_REMOVABLE_MEDIA_FILE_NAME)
    #if defined (MDE_CPU_EBC)
        //
        // Uefi specification only defines the default boot file name for IA32, X64
        // and IPF processor, so need define boot file name for EBC architecture here.
        //
        #define EFI_REMOVABLE_MEDIA_FILE_NAME L"\\EFI\\BOOT\\BOOTEBC.EFI"
    #else
        #error "Can not determine the default boot file name for unknown processor type!"
    #endif
#endif

#if   defined (MDE_CPU_IA32)
  #define ARCHITECTURE_NAME     L"ia32"
#elif defined (MDE_CPU_X64)
  #define ARCHITECTURE_NAME     L"x64"
#elif defined (MDE_CPU_ARM)
  #define ARCHITECTURE_NAME     L"arm"
#elif defined (MDE_CPU_AARCH64)
  #define ARCHITECTURE_NAME     L"aarch64"
#else
  #error Unknown Processor Type
#endif

typedef struct {
  CHAR16            *FilePathString;
  CHAR16            *Description;
}BDS_GENERAL_UEFI_BOOT_OS;

#define RESOURCE_SECTION_RESOURCE_TYPE_VERSION               16
#define VS_FIXEDFILEINFO_SIGNATURE                           0xFEEF04BD
#define MICROSOFT_OS_VERSION_WINDOWS_8                       0x00060002

typedef struct {
  UINT32             Signature;
  UINT32             StrucVersion;
  UINT32             FileVersionMS;
  UINT32             FileVersionLS;
  UINT32             ProductVersionMS;
  UINT32             ProductVersionLS;
  UINT32             FileFlagsMask;
  UINT32             FileFlags;
  UINT32             FileOS;
  UINT32             FileType;
  UINT32             FileSubtype;
  UINT32             FileDateMS;
  UINT32             FileDateLS;
} VS_FIXEDFILEINFO;

#define SHELL_OPTIONAL_DATA_SIZE 6

typedef enum {
  NonRemovableHdd,
  UsbHdd,
  MaxMediaType
} BDS_MEDIA_TYPE;

#define ATTR_REMOVABLE_DEV           BIT0
#define ATTR_UNREMOVABLE_DEV         BIT1
#define ATTR_ALL                     ((UINT32) (-1))

typedef struct {
  UINTN         DevicePathType;
  EFI_STRING_ID StringToken;
  UINTN         DeviceNum;
  UINT32        Attribute;
} DEVICE_TYPE_INFORMATION;

/**
 Allocates a block of memory and writes performance data of booting into it.
 OS can processing these record.
**/
VOID
WriteBootToOsPerformanceData (
  VOID
  );

/**
 Get the headers (dos, image, optional header) from an image

 @param[in]  Device            SimpleFileSystem device handle
 @param[in]  FileName          File name for the image
 @param[out] DosHeader         Pointer to dos header
 @param[out] Hdr               The buffer in which to return the PE32, PE32+, or TE header.

 @retval EFI_SUCCESS           Successfully get the machine type.
 @retval EFI_NOT_FOUND         The file is not found.
 @retval EFI_LOAD_ERROR        File is not a valid image file.
**/
EFI_STATUS
EFIAPI
BdsLibGetImageHeader (
  IN  EFI_HANDLE                  Device,
  IN  CHAR16                      *FileName,
  OUT EFI_IMAGE_DOS_HEADER        *DosHeader,
  OUT EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION   Hdr
  );

/**
 This routine adjust the memory information for different memory type and
 save them into the variables for next boot.
**/
VOID
EFIAPI
BdsSetMemoryTypeInformationVariable (
  IN  EFI_EVENT                Event,
  IN  VOID                     *Context
  );

/**
 Validate the EFI Boot#### or Driver#### variable (VendorGuid/Name)

 @param[in] Variable           Boot#### variable data.
 @param[in] VariableSize       Returns the size of the EFI variable that was read

 @retval TRUE                  The variable data is correct.
 @retval FALSE                 The variable data is corrupted.
**/
BOOLEAN
ValidateOption (
  IN UINT8                     *Variable,
  IN UINTN                     VariableSize
  );

EFI_STATUS
BdsLibOpenFileFromDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL   *DevicePath,
  IN  UINT64                     OpenMode,
  IN  UINT64                     Attributes,
  OUT EFI_FILE_HANDLE            *NewHandle
  );

EFI_STATUS
BuildNetworkBootOption (
  IN     LIST_ENTRY  *BdsBootOptionList
  );

VOID
SetNativeResolution (
  IN EFI_HANDLE                         *PrimaryVgaHandle
  );

EFI_STATUS
VgaHandleSelect (
  IN  EFI_DEVICE_PATH_PROTOCOL         *VgaDevicePath,
  OUT EFI_HANDLE                       **SelectedHandle
  );

VOID
UpdateBgrt (
  IN EFI_HANDLE                       *PrimaryVgaHandle
  );

EFI_STATUS
BgrtUpdateImageInfo (
  IN  EFI_HANDLE                        *PrimaryVgaHandle,
  OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL     **BltBuffer,
  OUT UINTN                             *DestinationX,
  OUT UINTN                             *DestinationY,
  OUT UINTN                             *Width,
  OUT UINTN                             *Height
  );

VOID
UpdateBvdtToHii (
  IN EFI_HII_HANDLE     HiiHandle
  );

EFI_STATUS
SetBbsPriority (
  IN  EFI_LEGACY_BIOS_PROTOCOL          *LegacyBios,
  IN  BDS_COMMON_OPTION                 *Option
  );

UINT8
BdsLibGetMessagingDevicePathSubType (
  IN  BDS_COMMON_OPTION          * Option,
  IN  HARDDRIVE_DEVICE_PATH      * HardDriveDevicePath
  );

EFI_STATUS
BdsLibUpdateOptionVar (
  IN  CHAR16                         *OptionName,
  IN  EFI_DEVICE_PATH_PROTOCOL       *DevicePath,
  IN  CHAR16                         *Description,
  IN  UINT8                          *OptionalData,   OPTIONAL
  IN  UINT32                         OptionalDataSize
  );

EFI_STATUS
GetUnnecessaryEfiBootOption (
   IN     UINT16       *BootOrder,
   IN     UINT16       BootOrderCount,
   IN     UINTN        BootOrderIndex,
   IN OUT UINTN        *EfiOsCreatedIndex,
   IN OUT UINTN        *BiosCreatedIndex
  );

BOOLEAN
IsSinglePhysicalGop (
  EFI_HANDLE  *SinglePhyGop
  );

VOID
EnableOptimalTextMode (
  VOID
  );

EFI_STATUS
ChkTextModeNum (
  IN UINTN     RequestedColumns,
  IN UINTN     RequestedRows,
  OUT UINTN    *TextModeNum
  );

BOOLEAN
FoundTextBasedConsole (
  VOID
  );

VOID *
GetAllActiveConOutDevPath (
  VOID
  );

VOID
GetComboVideoOptimalResolution (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput,
  OUT UINT32                        *XResolution,
  OUT UINT32                        *YResoulution
  );

BOOLEAN
IsOnBoardPciDevice (
  IN     UINT32                              Bus,
  IN     UINT32                              Device,
  IN     UINT32                              Function
  );


EFI_STATUS
GetResourceSectionData (
  IN  EFI_HANDLE                        Device,
  IN  CHAR16                            *FileName,
  OUT EFI_IMAGE_DATA_DIRECTORY          *ResourceDataDir,
  OUT UINT8                             **ResourceData,
  OUT UINT32                            *ResourceDataSize
  );

EFI_STATUS
GetWindowsOsVer (
  IN  EFI_IMAGE_DATA_DIRECTORY          *ResourceDir,
  IN  UINT8                             *ResourceData,
  OUT UINT32                            *VersionMS,
  OUT UINT32                            *VersionLS
  );

/**
  Set target HDD in boot list as connected.
**/
VOID
SetTargetHddConnected (
  VOID
  );

/**
  Set all boot options in boot list as connected.
**/
VOID
SetAllBootOptionsConnected (
  VOID
  );

/**
  Update boot group boot options to boot list.

  @retval EFI_SUCCESS           Update boot group to boot list successfully.
  @retval EFI_OUT_OF_RESOURCES  Allocate memory failed.
**/
EFI_STATUS
UpdateBootGroup (
  VOID
  );

extern H2O_BDS_SERVICES_PROTOCOL  *gBdsServices;
#endif // _BDS_LIB_H_


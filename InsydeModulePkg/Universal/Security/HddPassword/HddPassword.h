/** @file
  EFI HDD Password Protocol

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _HDD_PASSWORD_H_
#define _HDD_PASSWORD_H_

#include <Pi/PiSmmCis.h>

#include <FrameworkDxe.h>
#include <PortNumberMap.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeChipsetSvcLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/OemGraphicsLib.h>
#include <Library/PciLib.h>
#include <Library/PostCodeLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/VariableLib.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <Library/SmmOemSvcKernelLib.h>
#include <Library/BdsCpLib.h>

#include <Protocol/AcpiRestoreCallbackDone.h>
#include <Protocol/AdapterInformation.h>
#include <Protocol/AtaPassThru.h>
#include <Protocol/DevicePath.h>
#include <Protocol/DiskInfo.h>
#include <Protocol/HddPasswordService.h>
#include <Protocol/LegacyBios.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/Metronome.h>
#include <Protocol/H2ODialog.h>
#include <Protocol/PciIo.h>
#include <Protocol/SmmBase.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmRuntime.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/UnicodeCollation.h>
#include <Protocol/BlockIo.h>
#include <Protocol/StorageSecurityCommand.h>
#include <Protocol/SmmCommunication.h>

#include <Guid/HddPasswordVariable.h>
#include <Guid/HobList.h>
#include <Guid/MdeModuleHii.h>
#include <Guid/AdmiSecureBoot.h>
#include <Guid/DebugMask.h>

#include <Guid/H2OBdsCheckPoint.h>

#include <IndustryStandard/Pci.h>
#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/TcgStorageCore.h>

#include <Uefi/UefiInternalFormRepresentation.h>
#include <Uefi/UefiBaseType.h>

#include <MmioAccess.h>

#include <H2OStorageSecurityCommand.h>

#include "HddPasswordStrDefs.h"


#define EBDA(a)                           (*(UINT8*)(UINTN)(((*(UINT16*)(UINTN)0x40e) << 4) + (a)))
#define EBDA_HDD_LOCKED_FLAG              0x3d9
#define BIT(a)                            (1 << (a))
#define ATAPI_DEVICE_BIT                  BIT15

//
//  Status Reg
//
#define PCI_CLASS_CODE                    0x09
#define PCI_AHCI_BAR                      0x24
#define PCI_CLASS_MASS_STORAGE            0x01
#define PCI_SUB_CLASS_IDE                 0x01
#define PCI_SUB_CLASS_AHCI                0x06
#define PCI_SUB_CLASS_RAID                0x04
#define PCI_CLASSC_PI_AHCI                0x01
#define PCI_CLASSC_PI_RAID                0x00

#define PCI_AHCI_BAR                      0x24

#define HBA_PORTS_START                   0x0100
#define HBA_PORTS_REG_WIDTH               0x0080
#define HBA_PORTS_SCTL                    0x002C
#define   HBA_PORTS_SCTL_DET_COMRESET     0x01
#define   HBA_PORTS_SCTL_DET_OFFLINE      0x04


#define MAX_HDD_PASSWORD_LENGTH           32
#define MODEL_NUMBER_LENGTH               40
#define SUPPORT_SATA_PORTS_NUM             8

#define HDD_TITLE_STRING_LENGTH           (MODEL_NUMBER_LENGTH + 30)

#define BUFFER_SIZE                      512

//
// Hot Key for skipping dialog
//
#define DEFAULT_SKIP_DIALOG_KEY_INDEX     2
#define DEFAULT_SKIP_DIALOG_KEY_SCANCODE  SCAN_ESC
#define DEFAULT_SKIP_DIALOG_KEY_UNICHAR   CHAR_NULL

//
// Character definitions
//
#define CHAR_SPACE                        0x0020

//
//  Option Icon
//
#define OPTION_ICON_MAX_STR_SIZE          64

//
//  Memory Alignment
//
#define ALIGN_SIZEOF_UINTN(a)             ((((a) - 1) | (sizeof (UINTN) - 1)) + 1)

//
//  Status flag definition of HddDataInfo
//
#define DEFAULT_HDD_PASSWORD_MAX_NUMBER   HDD_PASSWORD_MAX_NUMBER

EFI_HII_HANDLE                            gStringPackHandle;

EFI_HII_DATABASE_PROTOCOL                 *HiiDatabase;
EFI_HII_STRING_PROTOCOL                   *IfrLibHiiString;


#define STR_TOKEN_NUMBERS                  10



typedef struct _HDD_SECURITY_COMMAND_PACKET {
  UINT16                                  SecurityCmdType;
  UINT8                                   HddPassword[32];
  UINT16                                  MasterPasswordIdentifier;
  UINT16                                  Reserved[238];
} HDD_SECURITY_COMMAND_PACKET;

typedef struct {
  UINT8                                   PI;
  UINT8                                   SubClassCode;
  UINT8                                   BaseCode;
} SATA_CLASS_CODE;

typedef struct {
  EFI_LEGACY_BIOS_INSTALL_ROM             OriginalInstallPciRom;
  EFI_HANDLE                              RaidController;
  EFI_PCI_IO_PROTOCOL                     *PciIo;
  UINT32                                  *AhciAbarIoSave;
  UINTN                                   NumOfBackupPorts;
  UINTN                                   BackupPortStart;
  UINT8                                   DiskStart;
  UINT8                                   DiskEnd;
} RAID_SETUP_INFO;

typedef struct {
  CHAR16                                  **StrTokenArray;
} STR_TOKEN_INFO;

typedef struct {
  LIST_ENTRY                              Link;
  EFI_HANDLE                              AtaControllerHandle;
  UINT8                                   AtaMode;
  UINTN                                   PciSeg;
  UINTN                                   PciBus;
  UINTN                                   PciDevice;
  UINTN                                   PciFunction;
  UINT8                                   PI;
  UINT8                                   SubClassCode;
  UINT8                                   BaseCode;
} ATA_CONTROLLER_INFO;

typedef struct {
  EFI_HANDLE                              DiskInfoHandle;
  EFI_DISK_INFO_PROTOCOL                  *DiskInfo;
  ATA_IDENTIFY_DATA                       IdentifyData;
} DISKINFO_DATA;

typedef struct {
  UINT8                                   NumOfController;
  BOOLEAN                                 AtaControllerSearched;
  BOOLEAN                                 HddInfoCollected;
  RAID_SETUP_INFO                         RaidSetupInfo;
  STR_TOKEN_INFO                          StrTokenInfo;
  EFI_LEGACY_BIOS_PROTOCOL                *LegacyBios;
  LIST_ENTRY                              AtaControllerInfoListHead;
} DRIVER_INSTALL_INFO;

typedef struct {
  EFI_PHYSICAL_ADDRESS                    BaseAddr;
  UINTN                                   AllocatedSize;
  EFI_PHYSICAL_ADDRESS                    RemainMemAddr;
  UINTN                                   RemainSize;
} HDD_PASSWORD_MEM_RECORD;

typedef struct {
  UINT8                                   NumOfController;
  BOOLEAN                                 SmmMode;
} HDD_PASSWORD_S3_DATA;

typedef struct {
  UINTN                                   Signature;
  LIST_ENTRY                              Link;
  UINT32                                  BlockIoMediaId;
  EFI_HANDLE                              DeviceHandleInDxe;
  EFI_HANDLE                              DeviceHandleInSmm;
  EFI_STORAGE_SECURITY_COMMAND_PROTOCOL   *StorageSecurityCommandInDxe;
  EFI_STORAGE_SECURITY_COMMAND_PROTOCOL   *StorageSecurityCommandInSmm;
  HDD_PASSWORD_HDD_INFO                   HddInfo;
} HDD_PASSWORD_HDD_INFO_PRIVATE;

#define HDD_PASSWORD_HDD_INFO_PRIVATE_SIGNATURE SIGNATURE_32 ('h', 'd', 'i', 'p')
#define GET_HDD_INFO_PRIVATE_FROM_LINK(a)  CR (a, HDD_PASSWORD_HDD_INFO_PRIVATE, Link, HDD_PASSWORD_HDD_INFO_PRIVATE_SIGNATURE)

typedef struct {
  UINTN                                   NumOfHdd;
  LIST_ENTRY                              HddInfoPrivateListHead;
} HDD_INFO_MANAGER;

typedef struct {
  UINTN                                   Signature;
  EFI_HDD_PASSWORD_SERVICE_PROTOCOL       HddPasswordProtocol;
  HDD_INFO_MANAGER                        *HddInfoManager;
  HDD_PASSWORD_S3_DATA                    HddPasswordS3Data;
  HDD_PASSWORD_MEM_RECORD                 *MemRecord;
  BOOLEAN                                 HddInfoCollected;

  BOOLEAN                                 HddInfoAllCollected;
  BOOLEAN                                 UefiOsFastBootSupported;
} HDD_PASSWORD_PRIVATE;

#define HDD_PASSWORD_SIGNATURE            SIGNATURE_32 ('h', 'd', 'p', 'w')
#define GET_PRIVATE_FROM_HDD_PASSWORD(a)  CR (a, HDD_PASSWORD_PRIVATE, HddPasswordProtocol, HDD_PASSWORD_SIGNATURE)
#define HDD_PASSWORD_REQUEST_MEM_PAGE     2
#define HDD_PASSWORD_REQUEST_MEM_SIZE     (HDD_PASSWORD_REQUEST_MEM_PAGE * 4096)

typedef struct {
  UINTN                                   Signature;
  EFI_HDD_PASSWORD_DIALOG_PROTOCOL        HddPasswordDialogProtocol;
  EFI_HDD_PASSWORD_SERVICE_PROTOCOL       *HddPasswordService;
  H2O_DIALOG_PROTOCOL                     *H2oDialogProtocol;
  BOOLEAN                                 PostUnlocked;
} HDD_PASSWORD_POST_DIALOG_PRIVATE;

#define HDD_PASSWORD_POST_DIALOG_SIGNATURE SIGNATURE_32 ('h', 'd', 'p', 'd')
#define GET_PRIVATE_FROM_HDD_PASSWORD_POST_DIALOG(a)  CR (a, HDD_PASSWORD_POST_DIALOG_PRIVATE, HddPasswordDialogProtocol, HDD_PASSWORD_POST_DIALOG_SIGNATURE)

//
// SMM communicate header
//
typedef struct {
  UINTN                         Function;
  EFI_STATUS                    ReturnStatus;
  UINT8                         Data[1];
} HDD_PASSWORD_SMM_PARAMETER_HEADER;

//
// SMM function
//
#define HDD_PASSWORD_SMM_FUNCTION_INSTALL_INSTANCE          1
#define HDD_PASSWORD_ACPI_RESTORE_CALLBACK_NOTIFY_INSTANCE  2
#define HDD_PASSWORD_SMM_COLLECT_DEVICE_INSTANCE            3

//
// Size of SMM communicate header, without including the payload.
//
#define SMM_COMMUNICATE_HEADER_SIZE  (OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data))


CHAR16 *
GetStringById (
  IN STRING_REF                           Id
  );

EFI_STATUS
InitializeStringSupport  (
  VOID
  );

EFI_STATUS
InitDialogStringTokenArray (
  VOID
  );


EFI_STATUS
ReleaseStringTokenArray (
  VOID
  );



EFI_STATUS
GetModelNumber (
  IN VOID                                 *DescBufferPtr,
  IN VOID                                 *SourceBufferPtr
  );

EFI_STATUS
ChangeChannelDevice2PortNum (
  IN UINT32                               Bus,
  IN UINT32                               Device,
  IN UINT32                               Function,
  IN UINT8                                PrimarySecondary,
  IN UINT8                                SlaveMaster,
  OUT UINTN                               *PortNum
  );

EFI_STATUS
CheckLegacyRaidSupport (
  VOID
  );

EFI_STATUS
InitLegacyRaidSupport (
  VOID
  );


BOOLEAN
IsOnBoardPciDevice (
  IN UINT32                               Bus,
  IN UINT32                               Device,
  IN UINT32                               Function
  );

EFI_STATUS
GetMem (
  IN  UINTN                               RequestSize,
  IN  OUT  VOID                           **AllocAddr
  );

EFI_STATUS
EFIAPI
CollectStorageController (
  IN BOOLEAN                             NeedConnectController
  );

UINT16
EFIAPI
FindMappedController (
  IN UINTN                               Seg,
  IN UINTN                               Bus,
  IN UINTN                               Device,
  IN UINTN                               Function
  );

EFI_STATUS
HddUnlockDialogInit (
  IN EFI_HDD_PASSWORD_SERVICE_PROTOCOL  *HddPasswordService
  );
#endif


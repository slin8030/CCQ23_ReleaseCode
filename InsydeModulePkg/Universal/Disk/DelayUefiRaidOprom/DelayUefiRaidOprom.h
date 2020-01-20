/** @file
  Delay UEFI RAID OpROM Driver

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

#ifndef _DELAY_UEFI_RAID_OPROM_H_
#define _DELAY_UEFI_RAID_OPROM_H_

#include <PiDxe.h>

#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/Pci.h>
#include <IndustryStandard/Scsi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/KernelConfigLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PostCodeLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/AtaPassThru.h>
#include <Protocol/DevicePath.h>
#include <Protocol/DiskInfo.h>
#include <Protocol/HddPasswordService.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PlatformDriverOverride.h>
#include <Protocol/ScsiPassThruExt.h>
#include <Protocol/ScsiIo.h>
#include <Protocol/UefiRaidOpromReady.h>
#include <Protocol/NvmExpressPassthru.h>

#include <Guid/HobList.h>
#include <Guid/SetupDefaultHob.h>

#include <PortNumberMap.h>

#define ATA_IDENTIFY_CMD                  0xEC
#define IDENTIFY_TABLE_SIZE               0x200
#define ATA_BLOCK_UNIT                    0x200

#define bit(a)                            (1 << (a))
#define ATAPI_DEVICE_BIT                  bit(15)

  //
  // bit5..7 are for Logical unit number
  // 11100000b (0xe0)
  //
#define EFI_SCSI_LOGICAL_UNIT_NUMBER_MASK 0xe0

#define NVME_ADMIN_IDENTIFY_CMD           0x06

#pragma pack(1)

typedef union {
  UINT32  Scsi;
  UINT8   ExtScsi[4];
} SCSI_ID;

typedef struct _SCSI_TARGET_ID {
  SCSI_ID ScsiId;
  UINT8   ExtScsiId[12];
}SCSI_TARGET_ID;

//
// SCSI Command Descriptor Block 6
//
typedef struct {
  UINT8 OpCode;
  UINT8 Evpd : 1;
  UINT8 Reserved_16 : 7;
  UINT8 PageCode;
  UINT8 AllocationLengthMsb;
  UINT8 AllocationLengthLsb;
  UINT8 Control;
} EFI_SCSI_CDB_6;

typedef struct {
  UINT8 Error_Code : 7;
  UINT8 Valid : 1;
  UINT8 Segment_Number;
  UINT8 Sense_Key : 4;
  UINT8 Reserved_21 : 1;
  UINT8 ILI : 1;
  UINT8 Reserved_22 : 2;
  UINT8 Information_3_6[4];
  UINT8 Addnl_Sense_Length;           // n - 7
  UINT8 Vendor_Specific_8_11[4];
  UINT8 Addnl_Sense_Code;             // mandatory
  UINT8 Addnl_Sense_Code_Qualifier;   // mandatory
  UINT8 Field_Replaceable_Unit_Code;  // optional
  UINT8 Reserved_15_254[240];
} EFI_SCSI_SENSE_DATA_EXT;

typedef struct {
  UINT8 PeripheralDeviceType : 5;
  UINT8 PeripheralQualifier : 3;
  UINT8 PageCode;
  UINT8 PageLengthMsb;
  UINT8 PageLengthLsb;
  UINT8 Reserved_4_7[4];
  UINT8 SatVendorIdentification[8];
  UINT8 SatProductIdentification[16];
  UINT8 SatProductRevisionLevel[4];
  UINT8 AtaDeviceSignature[20];
  UINT8 CommandCode;
  UINT8 Reserved_57_59[3];
  UINT8 IdentifyData[512];
} EFI_SCSI_ATA_VPD_PAGE;

typedef struct {
  UINT16 Mp;                /* Maximum Power */
  UINT8  Rsvd1;             /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Mps:1;             /* Max Power Scale */
  UINT8  Nops:1;            /* Non-Operational State */
  UINT8  Rsvd2:6;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT32 Enlat;             /* Entry Latency */
  UINT32 Exlat;             /* Exit Latency */
  UINT8  Rrt:5;             /* Relative Read Throughput */
  UINT8  Rsvd3:3;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Rrl:5;             /* Relative Read Leatency */
  UINT8  Rsvd4:3;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Rwt:5;             /* Relative Write Throughput */
  UINT8  Rsvd5:3;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Rwl:5;             /* Relative Write Leatency */
  UINT8  Rsvd6:3;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Rsvd7[16];         /* Reserved as of Nvm Express 1.1 Spec */
} NVME_PSDESCRIPTOR;

//
//  Identify Controller Data
//
typedef struct {
  //
  // Controller Capabilities and Features 0-255
  //
  UINT16 Vid;                 /* PCI Vendor ID */
  UINT16 Ssvid;               /* PCI sub-system vendor ID */
  UINT8  Sn[20];              /* Product serial number */

  UINT8  Mn[40];              /* Proeduct model number */
  UINT8  Fr[8];               /* Firmware Revision */
  UINT8  Rab;                 /* Recommended Arbitration Burst */
  UINT8  Ieee_oui[3];         /* Organization Unique Identifier */
  UINT8  Cmic;                /* Multi-interface Capabilities */
  UINT8  Mdts;                /* Maximum Data Transfer Size */
  UINT8  Cntlid[2];           /* Controller ID */
  UINT8  Rsvd1[176];          /* Reserved as of Nvm Express 1.1 Spec */
  //
  // Admin Command Set Attributes
  //
  UINT16 Oacs;                /* Optional Admin Command Support */
  UINT8  Acl;                 /* Abort Command Limit */
  UINT8  Aerl;                /* Async Event Request Limit */
  UINT8  Frmw;                /* Firmware updates */
  UINT8  Lpa;                 /* Log Page Attributes */
  UINT8  Elpe;                /* Error Log Page Entries */
  UINT8  Npss;                /* Number of Power States Support */
  UINT8  Avscc;               /* Admin Vendor Specific Command Configuration */
  UINT8  Apsta;               /* Autonomous Power State Transition Attributes */
  UINT8  Rsvd2[246];          /* Reserved as of Nvm Express 1.1 Spec */
  //
  // NVM Command Set Attributes
  //
  UINT8  Sqes;                /* Submission Queue Entry Size */
  UINT8  Cqes;                /* Completion Queue Entry Size */
  UINT16 Rsvd3;               /* Reserved as of Nvm Express 1.1 Spec */
  UINT32 Nn;                  /* Number of Namespaces */
  UINT16 Oncs;                /* Optional NVM Command Support */
  UINT16 Fuses;               /* Fused Operation Support */
  UINT8  Fna;                 /* Format NVM Attributes */
  UINT8  Vwc;                 /* Volatile Write Cache */
  UINT16 Awun;                /* Atomic Write Unit Normal */
  UINT16 Awupf;               /* Atomic Write Unit Power Fail */
  UINT8  Nvscc;               /* NVM Vendor Specific Command Configuration */
  UINT8  Rsvd4;               /* Reserved as of Nvm Express 1.1 Spec */
  UINT16 Acwu;                /* Atomic Compare & Write Unit */
  UINT16 Rsvd5;               /* Reserved as of Nvm Express 1.1 Spec */
  UINT32 Sgls;                /* SGL Support  */
  UINT8  Rsvd6[164];          /* Reserved as of Nvm Express 1.1 Spec */
  //
  // I/O Command set Attributes
  //
  UINT8 Rsvd7[1344];          /* Reserved as of Nvm Express 1.1 Spec */
  //
  // Power State Descriptors
  //
  NVME_PSDESCRIPTOR PsDescriptor[32];

  UINT8  VendorData[1024];    /* Vendor specific data */
} NVME_ADMIN_CONTROLLER_DATA;

typedef struct {
  UINT16 Ms;                /* Metadata Size */
  UINT8  Lbads;             /* LBA Data Size */
  UINT8  Rp:2;              /* Relative Performance */
    #define LBAF_RP_BEST      00b
    #define LBAF_RP_BETTER    01b
    #define LBAF_RP_GOOD      10b
    #define LBAF_RP_DEGRADED  11b
  UINT8  Rsvd1:6;           /* Reserved as of Nvm Express 1.1 Spec */
} NVME_LBAFORMAT;

//
// Identify Namespace Data for NVMe
//
typedef struct {
  //
  // NVM Command Set Specific
  //
  UINT64 Nsze;                /* Namespace Size (total number of blocks in formatted namespace) */
  UINT64 Ncap;                /* Namespace Capacity (max number of logical blocks) */
  UINT64 Nuse;                /* Namespace Utilization */
  UINT8  Nsfeat;              /* Namespace Features */
  UINT8  Nlbaf;               /* Number of LBA Formats */
  UINT8  Flbas;               /* Formatted LBA size */
  UINT8  Mc;                  /* Metadata Capabilities */
  UINT8  Dpc;                 /* End-to-end Data Protection capabilities */
  UINT8  Dps;                 /* End-to-end Data Protection Type Settings */
  UINT8  Nmic;                /* Namespace Multi-path I/O and Namespace Sharing Capabilities */
  UINT8  Rescap;              /* Reservation Capabilities */
  UINT8  Rsvd1[88];           /* Reserved as of Nvm Express 1.1 Spec */
  UINT64 Eui64;               /* IEEE Extended Unique Identifier */
  //
  // LBA Format
  //
  NVME_LBAFORMAT LbaFormat[16];

  UINT8 Rsvd2[192];           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8 VendorData[3712];     /* Vendor specific data */
} NVME_ADMIN_NAMESPACE_DATA;

typedef struct {
  UINTN                                   Signature;
  EFI_UEFI_RAID_OPROM_READY_PROTOCOL      UefiRaidOpromReadyProtocol;
  EFI_DRIVER_BINDING_PROTOCOL             *DriverBindingProtocol;
  EFI_HANDLE                              ControllerHandle;
  EFI_HANDLE                              ThisImageHandle;
  BOOLEAN                                 StartFinish;
  BOOLEAN                                 AhciBusSupport;
  BOOLEAN                                 RaidControllerInfoGet;
  BOOLEAN                                 RaidControllerAllConnected;
  UINT32                                  RaidControllerRemain;
  EFI_EXT_SCSI_PASS_THRU_PROTOCOL         *ExtScsiPassThruProtocol;
  LIST_ENTRY                              DeviceInfoListHead;
  LIST_ENTRY                              RaidControllerInfoListHead;
} DELAY_UEFI_RAID_OPROM_PRIVATE;

#define DELAY_UEFI_RAID_OPROM_SIGNATURE   SIGNATURE_32 ('D', 'U', 'R', 'O')
#define GET_PRIVATE_FROM_DELAY_UEFI_RAID_OPROM_SIGNATURE(a)  CR (a, DELAY_UEFI_RAID_OPROM_PRIVATE, UefiRaidOpromReadyProtocol, DELAY_UEFI_RAID_OPROM_SIGNATURE)

#pragma pack()

typedef struct {
  UINTN                                   Signature;
  LIST_ENTRY                              Link;
  EFI_DISK_INFO_PROTOCOL                  DiskInfo;
  EFI_HANDLE                              DeviceHandle;
  EFI_HANDLE                              ControllerHandle;
  EFI_DEVICE_PATH_PROTOCOL                *ControllerDevicePath;
  EFI_ATA_PASS_THRU_PROTOCOL              *AtaPassThruPtr;
  EFI_EXT_SCSI_PASS_THRU_PROTOCOL         *ExtScsiPassThruPtr;
  UINT16                                  Port;
  UINT16                                  PortMultiplierPort;
  UINT16                                  DeviceIdentifyDataRaw[IDENTIFY_TABLE_SIZE];
  EFI_SCSI_SENSE_DATA_EXT                 ScsiSenseDataExt;
  EFI_SCSI_INQUIRY_DATA                   InquiryData;
  EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL      *NvmePassThruProtocol;
  NVME_ADMIN_CONTROLLER_DATA              *NvmeAdminControllerData;
} DEVICE_PRIVATE_DATA;

typedef struct {
  LIST_ENTRY                              Link;
  EFI_HANDLE                              RaidControllerHandle;
  UINTN                                   PciBus;
  UINTN                                   PciDevice;
  UINTN                                   PciFunction;
  BOOLEAN                                 Checked;
} RAID_CONTROLLER_PRIVATE_DATA;

#define DEVICE_PRIVATE_DATA_SIGNATURE     SIGNATURE_32 ('D', 'P', 'D', 'S')
#define DEVICE_PRIVATE_FROM_LINK(a)       CR (a, DEVICE_PRIVATE_DATA, Link, DEVICE_PRIVATE_DATA_SIGNATURE)
#define DEVICE_PRIVATE_FROM_DISKINFO(a)   CR (a, DEVICE_PRIVATE_DATA, DiskInfo, DEVICE_PRIVATE_DATA_SIGNATURE)

EFI_STATUS
EFIAPI
DelayUefiRaidOpromDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL          *This,
  IN EFI_HANDLE                           ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL             *RemainingDevicePath OPTIONAL
  );

EFI_STATUS
EFIAPI
DelayUefiRaidOpromDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL          *This,
  IN EFI_HANDLE                           ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL             *RemainingDevicePath OPTIONAL
  );

EFI_STATUS
EFIAPI
DelayUefiRaidOpromDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL         *This,
  IN  EFI_HANDLE                          ControllerHandle,
  IN  UINTN                               NumberOfChildren,
  IN  EFI_HANDLE                          *ChildHandleBuffer
  );

EFI_STATUS
EFIAPI
AtaDiskInfoInquiry (
  IN     EFI_DISK_INFO_PROTOCOL           *This,
  IN OUT VOID                             *InquiryData,
  IN OUT UINT32                           *InquiryDataSize
  );

EFI_STATUS
EFIAPI
AtaDiskInfoIdentify (
  IN     EFI_DISK_INFO_PROTOCOL           *This,
  IN OUT VOID                             *IdentifyData,
  IN OUT UINT32                           *IdentifyDataSize
  );

EFI_STATUS
EFIAPI
AtaDiskInfoSenseData (
  IN     EFI_DISK_INFO_PROTOCOL           *This,
  IN OUT VOID                             *SenseData,
  IN OUT UINT32                           *SenseDataSize,
  OUT    UINT8                            *SenseDataNumber
  );

EFI_STATUS
EFIAPI
AtaDiskInfoWhichIde (
  IN  EFI_DISK_INFO_PROTOCOL              *This,
  OUT UINT32                              *IdeChannel,
  OUT UINT32                              *IdeDevice
  );

EFI_STATUS
EFIAPI
ScsiDiskInfoInquiry (
  IN     EFI_DISK_INFO_PROTOCOL           *This,
  IN OUT VOID                             *InquiryData,
  IN OUT UINT32                           *InquiryDataSize
  );

EFI_STATUS
EFIAPI
ScsiDiskInfoIdentify (
  IN     EFI_DISK_INFO_PROTOCOL           *This,
  IN OUT VOID                             *IdentifyData,
  IN OUT UINT32                           *IdentifyDataSize
  );

EFI_STATUS
EFIAPI
ScsiDiskInfoSenseData (
  IN     EFI_DISK_INFO_PROTOCOL           *This,
  IN OUT VOID                             *SenseData,
  IN OUT UINT32                           *SenseDataSize,
  OUT    UINT8                            *SenseDataNumber
  );

EFI_STATUS
EFIAPI
ScsiDiskInfoWhichIde (
  IN  EFI_DISK_INFO_PROTOCOL              *This,
  OUT UINT32                              *IdeChannel,
  OUT UINT32                              *IdeDevice
  );

EFI_STATUS
EFIAPI
NvmeDiskInfoInquiry (
  IN     EFI_DISK_INFO_PROTOCOL           *This,
  IN OUT VOID                             *InquiryData,
  IN OUT UINT32                           *InquiryDataSize
  );

EFI_STATUS
EFIAPI
NvmeDiskInfoIdentify (
  IN     EFI_DISK_INFO_PROTOCOL           *This,
  IN OUT VOID                             *IdentifyData,
  IN OUT UINT32                           *IdentifyDataSize
  );

EFI_STATUS
EFIAPI
NvmeDiskInfoSenseData (
  IN     EFI_DISK_INFO_PROTOCOL           *This,
  IN OUT VOID                             *SenseData,
  IN OUT UINT32                           *SenseDataSize,
  OUT    UINT8                            *SenseDataNumber
  );

EFI_STATUS
EFIAPI
NvmeDiskInfoWhichIde (
  IN  EFI_DISK_INFO_PROTOCOL           *This,
  OUT UINT32                           *IdeChannel,
  OUT UINT32                           *IdeDevice
  );

EFI_STATUS
EFIAPI
ScanAllSataController (
  VOID
  );

#endif // _DELAY_UEFI_RAID_OPROM_H_


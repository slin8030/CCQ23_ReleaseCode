/*++

Copyright (c)  1999 - 2016, Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PlatformInfo.h

Abstract:

  GUID used for Platform Info Data entries in the HOB list.

--*/

#ifndef _PLATFORM_INFO_GUID_H_
#define _PLATFORM_INFO_GUID_H_

#include <PiPei.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/GpioLib.h>
#include <Protocol/DevicePath.h>

#include <SmipGenerated.h>


#define SMIP_GPIO_ENABLE        FALSE
#define PLATFORM_INFO_REVISION  1      // revision id for current platform information struct
//
// Start::Broxton Board Defines
//
#define BOARD_REVISION_DEFAULT  0xff
#define UNKNOWN_FABID       0x0F
#define FAB_ID_MASK         0x0F
#define BOARD_ID_2          0x01
#define BOARD_ID_1          0x40
#define BOARD_ID_0          0x04

#define BOARD_ID_DT_CRB     0x0

#define DC_ID_DDR3L      0x00
#define DC_ID_DDR3       0x04
#define DC_ID_LPDDR3     0x02
#define DC_ID_LPDDR2     0x06
#define DC_ID_DDR4       0x01
#define DC_ID_DDR3L_ECC  0x05
#define DC_ID_NO_MEM     0x07
//
// End::Broxton Board Defines
//

#define MAX_FAB_ID_RETRY_COUNT  100
#define MAX_FAB_ID_CHECK_COUNT  3

#define PLATFORM_INFO_HOB_REVISION  0x1

#define EFI_PLATFORM_INFO_GUID \
  { \
    0x1e2acc41, 0xe26a, 0x483d, 0xaf, 0xc7, 0xa0, 0x56, 0xc3, 0x4e, 0x8, 0x7b \
  }

#define GetCommOffset(Community, Offset) (((UINT32)Community) << 16) + GPIO_PADBAR + Offset
#define EFI_TOUCH_DEVICE_GUID \
  { \
    0x3829d234, 0xecff, 0x4adb, { 0xb4, 0x50, 0xcb, 0xc0, 0xe1, 0x7a, 0x15, 0xff } \
  }

//
// Setup option for multi-BOM selection
//
#define BOM_MANUALLY 0x80
#define BOM_FROM_PSS 0xFF

extern EFI_GUID gEfiPlatformInfoGuid;
extern EFI_GUID gIFWIVersionHobGuid;


typedef enum {
  FlavorUnknown = 0,
  //
  // Mobile
  //
  FlavorMobile = 1,
  //
  // Desktop
  //
  FlavorDesktop = 2,
  //
  // Tablet
  //
  FlavorTablet = 3
} PLATFORM_FLAVOR;

#pragma pack(1)

typedef struct {
  UINT16  PciResourceIoBase;
  UINT16  PciResourceIoLimit;
  UINT32  PciResourceMem32Base;
  UINT32  PciResourceMem32Limit;
  UINT64  PciResourceMem64Base;
  UINT64  PciResourceMem64Limit;
  UINT64  PciExpressBase;
  UINT32  PciExpressSize;
  UINT8   PciHostAddressWidth;
  UINT8   PciResourceMinSecBus;
} EFI_PLATFORM_PCI_DATA;

typedef struct {
  UINT8 CpuAddressWidth;
  UINT32 CpuFamilyStepping;
} EFI_PLATFORM_CPU_DATA;

typedef struct {
  UINT8 SysIoApicEnable;
  UINT8 SysSioExist;
} EFI_PLATFORM_SYS_DATA;

typedef struct {
  UINT32  MemTolm;
  UINT32  MemMaxTolm;
  UINT32  MemTsegSize;
  UINT32  MemTsegBase;
  UINT32  MemIedSize;
  UINT32  MemIgdSize;
  UINT32  MemIgdBase;
  UINT32  MemIgdGttSize;
  UINT32  MemIgdGttBase;
  UINT64  MemMir0;
  UINT64  MemMir1;
  UINT32  MemConfigSize;
  UINT16  MmioSize;
} EFI_PLATFORM_MEM_DATA;

typedef struct {
  UINT32 IgdOpRegionAddress;    // IGD OpRegion Starting Address
  UINT8  IgdBootType;           // IGD Boot Display Device
  UINT8  IgdPanelType;          // IGD Panel Type CMOs option
  UINT8  IgdTvFormat;           // IGD TV Format CMOS option
  UINT8  IgdTvMinor;            // IGD TV Minor Format CMOS option
  UINT8  IgdPanelScaling;       // IGD Panel Scaling
  UINT8  IgdBlcConfig;          // IGD BLC Configuration
  UINT8  IgdBiaConfig;          // IGD BIA Configuration
  UINT8  IgdSscConfig;          // IGD SSC Configuration
  UINT8  IgdDvmtMemSize;        // IGD DVMT Memory Size
  UINT8  IgdFunc1Enable;        // IGD Function 1 Enable
  UINT8  IgdHpllVco;            // HPLL VCO
  UINT8  IgdSciSmiMode;         // GMCH SMI/SCI mode (0=SCI)
  UINT8  IgdPAVP;               // IGD PAVP data
} EFI_PLATFORM_IGD_DATA;

/*
//
// Read Board ID and FAB ID from PSS chip that prefix 2 additional characters to IPN string,
// This design start from FAB-D board. (e.g. RDH28336-201, RD for Broxton RVP FAB-D)
// The first character can be used for identify different board (RVP, PR, FFD...),
// and second character can be used for different Board revision (FAB-A, FAB-B...)
//
Fro BXTM, Following Board_ID/FAB_ID PSS chip is used to identify the board

Board Type        Board_ID in PSS   FAB_ID in PSS       Board_ID                   FAB_ID
RVP FABB/C       NULL                  NULL                   BOARD_ID_BXT_RVP    FAB_ID_RVP_B_C   
RVP FABD          R                       D                        BOARD_ID_BXT_RVP    FAB_ID_D   
EVT/PR0           F                       A                        BOARD_ID_BXT_FFD    FAB_ID_A
PR0.5               F                       C                        BOARD_ID_BXT_FFD    FAB_ID_C
  
*/

typedef enum {
  BOARD_ID_BXT_RVP  = 0x0,      // Broxton RVP Board
  BOARD_ID_BXT_PR   = 0x1,      // Broxton PR Board
  BOARD_ID_BXT_FFD  = 0x2,      // Broxton FFD Board
  BOARD_ID_BXT_RVV  = 0x3,      // Broxton RVV Board
  BOARD_ID_BXT_RVVP = 0xA,      // Broxton RVVP Board
  UNKNOWN_BOARD     = 0xFF      // Unknown Board

} BOARD_ID_LIST;

//[-start-151228-IB03090424-modify]//
typedef enum {
  BOARD_ID_APL_ERBP = 0x0,        // APL ERB
  BOARD_ID_APL_RVP_1A = 0x1,      // APL RVP 1A
  BOARD_ID_APL_RVP_2A = 0x2,      // APL RVP 2A
  BOARD_ID_APL_RVP_1C_LITE = 0x3, // APL RVP 1C Lite
  BOARD_ID_MNH_RVP = 0x5,         // Mineral Hill RVP
  BOARD_ID_OXH_CRB = 0x6,         // Oxbow Hill
  BOARD_ID_LFH_CRB = 0x7,         // Leaf Hill
  BOARD_ID_JNH_CRB = 0x8,         // Juniper Hill
  //Gordon Ridge HS Measurement Board = 0xC
  BOARD_ID_BFH_IVI = 0xD,         // Gordon Ridge BMP Interposer
  //Gordon Ridge Test Board = 0xE
  BOARD_ID_MRB_IVI = 0xF,         // Gordon Ridge BMP MRB
  BOARD_ID_APL_UNKNOWN = 0xff

} APL_BOARD_ID_LIST;
//[-end-151228-IB03090424-modify]//


//[-start-160317-IB03090425-modify]//
typedef enum {
  FAB_ID_RVP_B_C   = 0x0,       // For Broxton FAB B/C, special define
  FAB_ID_A         = 0x1,       // FAB A
  FAB_ID_B         = 0x2,       // FAB B
  FAB_ID_C         = 0x3,       // FAB C
  FAB_ID_D         = 0x4,       // FAB D
  FAB_ID_E         = 0x5,       // FAB E
  FAB_ID_F         = 0x6,       // FAB F
  UNKNOWN_FAB      = 0xFF       // Unknown FAB
} FAB_ID_LIST;
//[-end-160317-IB03090425-modify]//

typedef enum {
  PSS_WOS          = 0x0,       // WOS in PSS
  PSS_AOS          = 0x1,       // AOS in PSS
  UNKNOWN_OS       = 0xFF       // Unknown OS
} PSS_OS_LIST;

typedef enum {
  PSS_BOM_0        = 0x0,       // Multi-BOM: BOM0
  PSS_BOM_1        = 0x1,       // Multi-BOM: BOM1
  PSS_BOM_2        = 0x2,       // Multi-BOM: BOM2
  PSS_BOM_3        = 0x3,       // Multi-BOM: BOM3
  PSS_BOM_4        = 0x4,       // Multi-BOM: BOM4
  UNKNOWN_PSS_BOM  = 0xFF       // Unknown Mulit-BOM
} PSS_BOM_LIST;


typedef enum {
  TPM_DISABLE      = 0,         // TPM Disable
  TPM_PTT          = 1,         // PTT (fTPM)
  TPM_DTPM_1_2     = 2,         // Discrete TPM 1.2
  TPM_DTPM_2_0     = 3          // Discrete TPM 2.0
} TPM_DEVICE;

//[-start-160317-IB03090425-modify]//
typedef struct _EFI_PLATFORM_INFO_HOB {
  UINT16                      PlatformType; // Platform Type
  UINT8                       BoardId;             // Board ID
  UINT8                       BoardRev;            // Board Revision
  UINT8                       DockId;              // Dock ID
  UINT8                       OsSelPss;            // PSS OS Selection
  UINT8                       BomIdPss;            // PSS BOM ID
  PLATFORM_FLAVOR             PlatformFlavor;      // Platform Flavor
  UINT8                       ECOId;               // ECO applied on platform
  UINT16                      IohSku;
  UINT8                       IohRevision;
  UINT16                      IchSku;
  UINT8                       IchRevision;
  EFI_PLATFORM_PCI_DATA       PciData;
  EFI_PLATFORM_CPU_DATA       CpuData;
  EFI_PLATFORM_MEM_DATA       MemData;
  EFI_PLATFORM_SYS_DATA       SysData;
  EFI_PLATFORM_IGD_DATA       IgdData;
  UINT8                       RevisonId;           // Structure Revision ID
//  BOARD_FEATURES              BoardFeatures;
  UINT32                      SsidSvid;
  UINT16                      AudioSubsystemDeviceId;
  UINT64                      AcpiOemId;
  UINT64                      AcpiOemTableId;
  UINT16                      FABID;
  UINT8                       CfioEnabled;
  UINT8                       OsSelection;         // OS Selection 0: WOS 1:AOS 2:Win7
  UINT32                      FvMain3Base;
  UINT32                      FvMain3Length;
//[-start-160406-IB07400715-modify]//
//#if BXTI_PF_ENABLE
  UINT8                       ECPresent;
  UINT8                       PmicVersion;
//[-start-160422-IB03090426-add]//
  UINT8                       TxeMeasurementHash[32];
//[-end-160422-IB03090426-add]//
//[-start-170516-IB08450375-add]//
  UINT8                       RTEn;
//[-end-170516-IB08450375-add]//
//#endif
//[-end-160406-IB07400715-modify]//
//[-start-151228-IB03090424-add]//
  UINT8                       EcMajorRevision;
  UINT8                       EcMinorRevision;
//[-end-151228-IB03090424-add]//
  PlatfromMultiBOMConf        PlatformBOMValue;
  PlatfromFeatureConf         PlatformFeatureValue;
  UINT32                      SmipGpioBaseTableLength;
  UINT32                      SmipGpioDiffTableLength;
//[-start-160406-IB07400715-modify]//
//#if !BXTI_PF_ENABLE
  GPIO_CONFIG_SMIP            SmipGpioBaseTable[250];    // Use these two fields for SMIP GPIO configuration
  GPIO_CONFIG_SMIP            SmipGpioDiffTable[100];    //the maximum gpio diff table length is 100.
//#else
//  GPIO_CONFIG_SMIP            SmipGpioBaseTable[16];     // Temp solution MRC stack overflow
//  GPIO_CONFIG_SMIP            SmipGpioDiffTable[16];  
//#endif
//[-start-170516-IB08450375-add]//
//#if BXTI_PF_ENABLE
  UINT32                      BootGuard;
//#endif
//[-end-170516-IB08450375-add]//
//[-end-160406-IB07400715-modify]//
  //
  // Caveat: Must place pointer entries at the end of the structure.
  // Pointer in structure will result in size difference between 32bit and 64bit compilation.
  //
} EFI_PLATFORM_INFO_HOB;
//[-end-160317-IB03090425-modify]//

typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} TOUCH_DEVICE_PATH;

typedef enum {
    TOUCH_DISABLE  = 0,  
    TOUCH_DISCRETE  = 1, 
    TOUCH_INTEGRATED  = 2 
} TOUCH_ID_LIST;

#pragma pack()

EFI_STATUS
GetPlatformInfoHob (
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  OUT EFI_PLATFORM_INFO_HOB           **PlatformInfoHob
  );

#endif

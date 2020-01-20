/** @file
 GUID used for Platform Info Data entries in the HOB list.

@copyright
 Copyright (c) 1999 - 2016 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef _PLATFORM_INFO_GUID_H_
#define _PLATFORM_INFO_GUID_H_

#include <PiPei.h>

#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/GpioLib.h>
#include <Library/SmbusLib.h>
#include <IndustryStandard/SmBus.h>

#define PLATFORM_INFO_REVISION 1      // revision id for current platform information structure

// BXT RVV BOARD ID
#define BOARD_ID_BXT_RVV = 0x0F,       // Broxton RVV Board
//
// Start::BayLake Board Defines
//
#define BOARD_REVISION_DEFAULT 0xff
#define UNKNOWN_FABID       0x0F
#define FAB_ID_MASK         0x0F
#define BOARD_ID_2          0x01
#define BOARD_ID_1          0x40
#define BOARD_ID_0          0x04

#define BOARD_ID_DT_CRB     0x0
#define BOARD_ID_DT_VLVR    0x1
#define BOARD_ID_SVP_VLV    0xC
#define BOARD_ID_SVP_EV_VLV 0xD
//
// End::BayLake Board Defines
//

//
// Start::Alpine Valley Board Defines
//
//#define BOARD_ID_AV_SVP     1492 //0x5D4, AlpineValley Board id
#define DC_ID_DDR3L      0x00
#define DC_ID_DDR3       0x04
#define DC_ID_LPDDR3     0x02
#define DC_ID_LPDDR2     0x06
#define DC_ID_DDR4       0x01
#define DC_ID_DDR3L_ECC  0x05
#define DC_ID_NO_MEM     0x07
//
// End::Alpine Valley Board Defines
//

#define MAX_FAB_ID_RETRY_COUNT  100
#define MAX_FAB_ID_CHECK_COUNT  3

#define PLATFORM_INFO_HOB_REVISION  0x1

#define EFI_PLATFORM_INFO_GUID \
  { \
    0x1e2acc41, 0xe26a, 0x483d, 0xaf, 0xc7, 0xa0, 0x56, 0xc3, 0x4e, 0x8, 0x7b \
  }

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
  UINT8   CpuAddressWidth;
  UINT32  CpuFamilyStepping;
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

typedef enum {
  BOARD_ID_AV_SVP       = 0x0,    // Alpine Valley Board
  BOARD_ID_BL_RVP       = 0x2,    // BayLake Board
  BOARD_ID_BL_FFRD8     = 0x3,    // FFRD8 b'0011
  BOARD_ID_BL_FFRD      = 0x4,    // BayLake Board (FFRD10)
  BOARD_ID_BL_RVP_DDR3L = 0x5,    // BayLake Board (RVP DDR3L)
  BOARD_ID_BL_STHI      = 0x7,    // PPV- STHI Board
  BOARD_ID_BB_RVP       = 0x20,   // Bayley Bay Board
  BOARD_ID_BXT_RVP      = 0x30,   // Broxton Board
  BOARD_ID_BS_RVP       = 0x30,   // Bakersport Board
  BOARD_ID_CVH          = 0x90    // Crestview Hills
} BOARD_ID_LIST;

//[-start-151228-IB03090424-modify]//
typedef enum {
  BOARD_ID_APL_ERBP        = 0x0, // APL ERB
  BOARD_ID_APL_RVP_1A      = 0x1, // APL RVP 1A
  BOARD_ID_APL_RVP_2A      = 0x2, // APL RVP 2A
  BOARD_ID_APL_RVP_1C_LITE = 0x3, // APL RVP 1C Lite
  BOARD_ID_MNH_RVP         = 0x5, // Mineral Hill RVP
  BOARD_ID_OXH_CRB         = 0x6, // Oxbow Hill
  BOARD_ID_LFH_CRB         = 0x7, // Leaf Hill
  BOARD_ID_JNH_CRB         = 0x8, // Juniper Hill
  //Gordon Ridge HS Measurement Board = 0xC
  BOARD_ID_BFH_IVI         = 0xD, // Gordon Ridge BMP Interposer
  //Gordon Ridge Test Board = 0xE
  BOARD_ID_MRB_IVI         = 0xF, // Gordon Ridge BMP MRB
  BOARD_ID_BXT_RVVP        = 0xA, // Broxton RVV Board
  BOARD_ID_APL_UNKNOWN     = 0xff
} APL_BOARD_ID_LIST;
//[-end-151228-IB03090424-modify]//


typedef enum {
  FAB1 = 0,
  FAB2 = 1,
  FAB3 = 2
} FAB_ID_LIST;

typedef enum {
  PR0  = 0,   // FFRD10 PR0
  PR05 = 1,   // FFRD10 PR0.3 and PR 0.5
  PR1  = 2,   // FFRD10 PR1
  PR11 = 3    // FFRD10 PR1.1
} FFRD_ID_LIST;

typedef enum {
  FFRD_8_PR0  = 1,  // FFRD8 PR0
  FFRD_8_PR1  = 2   // FFRD8 PR1
} FFRD_8_ID_LIST;

typedef union {
  UINT32 famCnf;
  struct {
    UINT32 CurrSrcStr:3;
    UINT32 reserve:14;
    UINT32 odpullEn:1;
    UINT32 odpulldir:1;
    UINT32 hsMode:1;
    UINT32 Reseve3:1;
    UINT32 vpMode:1;
    UINT32 Reseve2:2;
    UINT32 HYSCTL:2;
    UINT32 parkMode:1;
    UINT32 reseve1:5;
    } r;
} CONF_FAMILY;

typedef union {
  UINT32 mics1;
  struct {
    UINT32 RCOMP_ENABLE:1;        // 0  set if rcomp is enable.
    } r;
} BXT_FAMILY_MISC;

typedef struct {
  CONF_FAMILY      confg;
  CONF_FAMILY      confg_change;
  BXT_FAMILY_MISC  family_misc;
  UINT32           mmioAddr;
//#ifdef EFI_DEBUG
  CHAR16*          family_name; // GPIO Family Name for debug purpose
//#endif
} GPIO_CONF_FAMILY_INIT;

typedef enum {
  TPM_DISABLE      = 0,         // TPM Disable
  TPM_PTT          = 1,         // PTT (fTPM)
  TPM_DTPM_1_2     = 2,         // Discrete TPM 1.2
  TPM_DTPM_2_0     = 3          // Discrete TPM 2.0
} TPM_DEVICE;

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
//[-start-160429-IB03090426-add]//
//[-start-160506-IB07400723-modify]//
//#if BXTI_PF_ENABLE
  UINT8                       ECPresent;
  UINT8                       PmicVersion;
  UINT8                       TxeMeasurementHash[32];
//[-start-160603-IB06720411-add]//
  UINT8                       RTEn;
//[-end-160603-IB06720411-add]//
//#endif
//[-end-160506-IB07400723-modify]//
//[-end-160429-IB03090426-add]//
  //
  // Caveat: Must place pointer entries at the end of the structure.
  // Pointer in structure will result in size difference between 32bit and 64bit compilation.
  //
  GPIO_CONF_FAMILY_INIT*      PlatfornGpioFamiliesSetting;
  BXT_GPIO_PAD_INIT*          PlatformGpioSetting_S;   // south Community
  BXT_GPIO_PAD_INIT*          PlatformGpioSetting_SW;   // South west Community
  BXT_GPIO_PAD_INIT*          PlatformGpioSetting_W;  // West  Community
  BXT_GPIO_PAD_INIT*          PlatformGpioSetting_NW;  // NorthWest west Community
  BXT_GPIO_PAD_INIT*          PlatformGpioSetting_N;  // North Community
  BXT_GPIO_PAD_INIT*          PlatformGpioSetting_S0ix;  // S0ix Based Config
  BXT_GPIO_PAD_INIT*          PlatformGpioSetting_S3;   // S3 Based Config
  BXT_GPIO_PAD_INIT*          PlatformGpioSetting_SetupConfig;  // Setup Based Config
} EFI_PLATFORM_INFO_HOB;

#pragma pack()

EFI_STATUS
GetPlatformInfoHob (
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  OUT EFI_PLATFORM_INFO_HOB           **PlatformInfoHob
  );


EFI_STATUS
InstallPlatformClocksNotify (
  IN CONST EFI_PEI_SERVICES           **PeiServices
  );

EFI_STATUS
InstallPlatformSysCtrlGPIONotify (
  IN CONST EFI_PEI_SERVICES           **PeiServices
  );

#endif

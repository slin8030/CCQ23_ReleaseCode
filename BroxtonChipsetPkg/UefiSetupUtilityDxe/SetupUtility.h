/** @file

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _UEFI_SETUP_UTILITY_H_
#define _UEFI_SETUP_UTILITY_H_

#include <Uefi.h>
#include <Pi/PiHob.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/PostCodeLib.h>
#include <Library/HiiLib.h>
#include <Library/PrintLib.h>
#include <Library/SetupUtilityLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/VariableLib.h>
#include <Library/OemGraphicsLib.h>
#include <Protocol/HiiDataBase.h>
#include <Protocol/HiiString.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/H2ODialog.h>
#include <Protocol/SetupUtility.h>
#include <Protocol/IdeControllerInit.h>
#include <Protocol/DiskInfo.h>
#include <Protocol/SeCOperation.h>
//[-start-161215-IB08450359-remove]//
//#include <Protocol/VariableEdit.h>
//[-end-161215-IB08450359-remove]//
#include <Protocol/SetupUtilityBrowser.h>
#include <Protocol/AlertStandardFormat.h>
#include <Protocol/SetupMouse.h>
#include <Protocol/HddPasswordService.h>
#include <Protocol/SetupUtilityApplication.h>
#include <Guid/GlobalVariable.h>
#include <Guid/SystemPasswordVariable.h>
#include <Guid/DebugMask.h>
#include <Guid/HobList.h>
#include <PostCode.h>
#include "ChipsetSetupConfig.h"
#include "SetupFuncs.h"
#include "IdeConfig.h"
#include "Advance.h"
#include "Power.h"
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
//[-start-160608-IB05400680-add]//
#include <Library/HeciMsgLib.h>
//[-end-160608-IB05400680-add]//
#include <Protocol/NvmExpressPassthru.h>

#define EFI_SETUP_UTILITY_SIGNATURE SIGNATURE_32('S','e','t','u')

#define STRING_PACK_GUID  { 0x8160a85f, 0x934d, 0x468b, 0xa2, 0x35, 0x72, 0x89, 0x59, 0x14, 0xf6, 0xfc }
#define HAVE_CREATE_SYSTEM_HEALTH_EVENT   0xCF8F
#define CUSTOM_BOOT_ORDER_SIZE            256
#define TIMEOUT_OF_EVENT                  5000000

#define SETUP_UTILITY_ADVANCE_FORMSET_GUID \
  { \
  0xa2db9298, 0x28bf, 0x47e5, {0x8f, 0xbc, 0x6f, 0xd6, 0xef, 0xff, 0xa9, 0xf5} \
  }
#define SETUP_UTILITY_POWER_FORMSET_GUID \
  { \
  0xcd765e9c, 0x137c, 0x4053, {0xbd, 0xff, 0x63, 0x14, 0xbd, 0x2e, 0xe2, 0xaf} \
  }
#define SETUP_UTILITY_POWER_ON_SECURITY_GUID \
  { \
  0xf05e86c1, 0xaca2, 0x4c96, {0x95, 0x2f, 0x10, 0x2b, 0xfa, 0xe, 0x1, 0x57}   \
  }
typedef enum {
  NoChanged  = 0,      ///< The SCU items are not changed yet.
  ChangedWithoutSaved, ///< The SCU items are changed but not saved.
  ChangedWithSaved     ///< The SCU items are changed and saved.
} SCU_OPTION_STATUS;
//
// UINT64 workaround
//
// The MS compiler doesn't handle QWORDs very well.  I'm breaking
// them into DWORDs to circumvent the problems.  Converting back
// shouldn't be a big deal.
//
//#pragma pack(1)
//typedef union _MSR_REG {
//  UINT64  Qword;
//
//  struct _DWORD {
//    UINT32  Low;
//    UINT32  High;
//  } Dwords;
//
//  struct _BYTE {
//    UINT8 FirstByte;
//    UINT8 SecondByte;
//    UINT8 ThirdByte;
//    UINT8 ForthByte;
//    UINT8 FifthByte;
//    UINT8 SixthByte;
//    UINT8 SeventhByte;
//    UINT8 EighthByte;
//  } Bytes;
//
//} MSR_REG;
//#pragma pack()

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include <SetupUtilityStrDefs.h>

//
// Global externs
//
extern UINT8                            SetupUtilityStrings[];
extern UINT32                           SetupUtilityStringsTotalSize;
extern UINT32                           SetupUtilityLibStringsTotalSize;
extern UINT8                            AdvanceVfrBin[];
extern UINT8                            PowerVfrBin[];

extern UINT8                            AdvanceVfrSystemConfigDefault0000[];
extern UINT8                            PowerVfrSystemConfigDefault0000[];

extern EFI_GUID                         mFormSetGuid;
extern CHAR16                           mVariableName[];

extern UINT16                           gSaveItemMapping[][2];
extern UINT16                           gSCUSystemHealth;
extern EFI_EVENT                        gSCUTimerEvent;
extern SETUP_UTILITY_BROWSER_DATA       *gSUBrowser;
extern BOOLEAN                          mSetupOptionStatusForFullReset;

extern EFI_HII_HANDLE                   mDriverHiiHandle;

//[-start-160608-IB05400680-add]//
//[-start-161002-IB07400791-remove]//
//extern EFI_HECI_PROTOCOL                *mPlatformConfigDataLibHeci2Protocol;
//[-end-161002-IB07400791-remove]//
//[-end-160608-IB05400680-add]//

//
// HII specific Vendor Device Path definition.
//
#pragma pack(1)
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  UINT32                         Reserved;
  UINT64                         UniqueId;
} HII_VENDOR_DEVICE_PATH_NODE;
#pragma pack()

typedef struct {
  HII_VENDOR_DEVICE_PATH_NODE    Node;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_TEMP_DEVICE_PATH;

typedef struct {
  UINTN                                 Signature;
  EFI_HANDLE                            Handle;
  EFI_SETUP_UTILITY_PROTOCOL            SetupUtility;
} SETUP_UTILITY_DATA;

typedef struct _NEW_PACKAGE_INFO {
  USER_INSTALL_CALLBACK_ROUTINE         CallbackRoutine;
  UINT8                                 *IfrPack;
  UINT8                                 *StringPack;
  UINT8                                 *ImagePack;
} NEW_PACKAGE_INFO;

EFI_STATUS
PowerOnSecurity (
  IN  EFI_SETUP_UTILITY_PROTOCOL        *SetupUtility
  );

EFI_STATUS
CheckIde (
  IN EFI_HII_HANDLE                     HiiHandle,
  IN CHIPSET_CONFIGURATION               *SetupVariable,
  IN BOOLEAN                            UpdateIde
  );

//[-start-160816-IB10860206-add]//
VOID
SataDeviceCallBack (
  IN EFI_HII_HANDLE HiiHandle
  );
//[-end-160816-IB10860206-add]//

EFI_STATUS
InstallHiiData (
  VOID
  );

EFI_STATUS
RemoveHiiData (
  IN VOID     *StringPack,
  ...
  );

EFI_STATUS
GetSystemConfigurationVar (
  IN SETUP_UTILITY_DATA                  *SetupData
  );

EFI_STATUS
InstallSetupUtilityBrowserProtocol (
  IN  EFI_SETUP_UTILITY_PROTOCOL         *This
  );

EFI_STATUS
UninstallSetupUtilityBrowserProtocol (
  VOID
  );

EFI_STATUS
CallSetupUtilityBrowser (
  VOID
  );

EFI_STATUS
EFIAPI
HotKeyCallBack (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN  EFI_BROWSER_ACTION                    Action,
  IN  EFI_QUESTION_ID                       QuestionId,
  IN  UINT8                                 Type,
  IN  EFI_IFR_TYPE_VALUE                    *Value,
  OUT EFI_BROWSER_ACTION_REQUEST            *ActionRequest
  );

EFI_STATUS
EFIAPI
GenericExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  );

EFI_STATUS
EFIAPI
GenericRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL         *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  );

EFI_STATUS
EFIAPI
InstallSetupHii (
  IN  EFI_SETUP_UTILITY_PROTOCOL            *This,
  IN  BOOLEAN                               InstallHii
  );

VOID
EFIAPI
SetupUtilityNotifyFn (
  IN EFI_EVENT                             Event,
  IN VOID                                  *Context
  );


#pragma pack(1)

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

#pragma pack()
#endif


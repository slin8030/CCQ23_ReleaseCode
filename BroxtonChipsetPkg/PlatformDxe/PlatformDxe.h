/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PlatformDxe.h

Abstract:

  Header file for Platform Initialization Driver.

 

++*/

#ifndef _PLATFORM_DRIVER_H
#define _PLATFORM_DRIVER_H

#include <PiDxe.h>
#include <IndustryStandard/Pci22.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/PcdLib.h>
#include <Library/PciLib.h>
#include <Library/I2clib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Protocol/IsaAcpi.h>
#include <Library/S3BootScriptLib.h>
#include <Library/DxeOemSvcChipsetLib.h>
#include <Library/PostCodeLib.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/CpuIo2.h>
#include <Protocol/BootScriptSave.h>
#include <Protocol/GlobalNvsArea.h>
#include <ScPolicyCommon.h>
#include <Protocol/SeCPlatformPolicy.h>
#include <Protocol/SeCOperation.h>
#include <Protocol/PciIo.h>
#include <Guid/GlobalVariable.h>
#include <Guid/BoardFeatures.h>
#include <Guid/EventGroup.h>
#include <Guid/PciLanInfo.h>
#include <Guid/PlatformInfo.h>
//[-start-160704-IB07220105-add]//
#include <Guid/EventLegacyBios.h>
//[-end-160704-IB07220105-add]//
#include <Framework/BootScript.h>
#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)
#include <KscLib.h>
#include <Protocol/ExitPmAuth.h>
#endif
#if (defined (TABLET_PF_ENABLE) && (TABLET_PF_ENABLE == 1))
#include <Library/PmicLib.h>
#include <Library/TimerLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/PmicLib/PmicWhiskeyCove.h>
#endif
#include <PostCode.h>
#include <ChipsetSetupConfig.h>
#include "Configuration.h"
#include <ScAccess.h>
#include <SaAccess.h>
#include <ChipsetCmos.h>
#include <PlatformBaseAddresses.h>
#include <PlatformBootMode.h>

#define ENABLE_USB_DEVICE_MODE_WITH_SW_VBUS_VALID   2
#define PCAT_RTC_ADDRESS_REGISTER 0x70
#define PCAT_RTC_DATA_REGISTER    0x71

#ifndef B_SMI_EN_LEGACY_USB2 
#define B_SMI_EN_LEGACY_USB2 B_SMI_STS_LEGACY_USB 
#endif
//
// Dallas DS12C887 Real Time Clock
//
#define RTC_ADDRESS_SECONDS           0   // R/W  Range 0..59
#define RTC_ADDRESS_SECONDS_ALARM     1   // R/W  Range 0..59
#define RTC_ADDRESS_MINUTES           2   // R/W  Range 0..59
#define RTC_ADDRESS_MINUTES_ALARM     3   // R/W  Range 0..59
#define RTC_ADDRESS_HOURS             4   // R/W  Range 1..12 or 0..23 Bit 7 is AM/PM
#define RTC_ADDRESS_HOURS_ALARM       5   // R/W  Range 1..12 or 0..23 Bit 7 is AM/PM
#define RTC_ADDRESS_DAY_OF_THE_WEEK   6   // R/W  Range 1..7
#define RTC_ADDRESS_DAY_OF_THE_MONTH  7   // R/W  Range 1..31
#define RTC_ADDRESS_MONTH             8   // R/W  Range 1..12
#define RTC_ADDRESS_YEAR              9   // R/W  Range 0..99
#define RTC_ADDRESS_CENTURY           50  // R/W  Range 19..20 Bit 8 is R/W
#define RTC_ADDRESS_REGISTER_A        10  // R/W[0..6]  R0[7]
#define RTC_ADDRESS_REGISTER_B        11  // R/W
#define RTC_ADDRESS_REGISTER_C        12  // RO
#define RTC_ADDRESS_REGISTER_D        13  // RO
#define   B_RTC_ALARM_INT_ENABLE      0x20
#define   B_RTC_ALARM_INT_STATUS      0x20
#define B_RTC_DATE_ALARM_MASK       0x3F

#define EFI_OEM_SPECIFIC                              0x8000
#define EFI_CU_PLATFORM_DXE_INIT                     (EFI_OEM_SPECIFIC | 0x00000011)
#define EFI_CU_PLATFORM_DXE_STEP1                    (EFI_OEM_SPECIFIC | 0x00000012)
#define EFI_CU_PLATFORM_DXE_STEP2                    (EFI_OEM_SPECIFIC | 0x00000013)
#define EFI_CU_PLATFORM_DXE_STEP3                    (EFI_OEM_SPECIFIC | 0x00000014)
#define EFI_CU_PLATFORM_DXE_STEP4                    (EFI_OEM_SPECIFIC | 0x00000015)
#define EFI_CU_PLATFORM_DXE_INIT_DONE                (EFI_OEM_SPECIFIC | 0x00000016)

typedef struct _GOP_DISPLAY_BRIGHTNESS_PROTOCOL GOP_DISPLAY_BRIGHTNESS_PROTOCOL;

typedef 
EFI_STATUS
(EFIAPI *GET_MAXIMUM_BRIGHTNESS_LEVEL) (
  IN  GOP_DISPLAY_BRIGHTNESS_PROTOCOL *This,
  OUT UINT32 *MaxBrightnessLevel
  );


typedef 
EFI_STATUS
(EFIAPI *GET_CURRENT_BRIGHTNESS_LEVEL) (
  IN  GOP_DISPLAY_BRIGHTNESS_PROTOCOL *This,
  OUT UINT32 *MaxBrightnessLevel
  );

typedef
EFI_STATUS
(EFIAPI *SET_BRIGHTNESS_LEVEL) (
  IN  GOP_DISPLAY_BRIGHTNESS_PROTOCOL *This,
  IN  UINT32  BrightnessLevel
  );

struct _GOP_DISPLAY_BRIGHTNESS_PROTOCOL {
  UINT32  Revision;
  GET_MAXIMUM_BRIGHTNESS_LEVEL GetMaxBrightnessLevel;
  GET_CURRENT_BRIGHTNESS_LEVEL GetCurrentBrightnessLevel;
  SET_BRIGHTNESS_LEVEL SetBrightnessLevel;
};

//
// Prototypes
//
VOID
ReadyToBootFunction (
  EFI_EVENT  Event,
  VOID       *Context
  );

VOID
EFIAPI
PciBusEvent (
  IN EFI_EVENT    Event,
  IN VOID*        Context
  );

EFI_STATUS
PciBusDriverHook(
  VOID
  );

//[-start-160216-IB03090424-add]//
/** Prototypes defined in SiPolicy.c **/
VOID
InitSiPolicy(
VOID
);
//[-end-160216-IB03090424-add]//
//[-start-170329-IB07400852-add]//
EFI_STATUS
EFIAPI
Chipset2InitializePlatform (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );
//[-end-170329-IB07400852-add]//

// Global externs
//
extern CHIPSET_CONFIGURATION mSystemConfiguration;
#endif

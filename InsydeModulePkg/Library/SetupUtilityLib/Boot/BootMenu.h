/** @file
  Header file for Boot Menu relative function

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

#ifndef _SETUP_BOOT_MENU_H_
#define _SETUP_BOOT_MENU_H_

#include <Library/SetupUtilityLib.h>
#include <Library/PrintLib.h>
#include "BbsType.h"
#include "Boot.h"

#define MAX_STRING_SIZE                 0x100
#define MAX_BOOT_COUNT                  100

#define MIN_ALIGNMENT_SIZE  4
#define ALIGN_SIZE(a)       ((a % MIN_ALIGNMENT_SIZE) ? MIN_ALIGNMENT_SIZE - (a % MIN_ALIGNMENT_SIZE) : 0)

typedef struct {
  EFI_DEVICE_PATH_PROTOCOL        *BlockIoDevicePath;
  CHAR16                          *HwDeviceName;
} HARDWARE_BOOT_DEVICE_INFO;

//
// The following functions are used in Boot menu internal
//
EFI_STATUS
Shiftitem (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN UINTN                                  LastTokenRecord,
  IN UINTN                                  DeviceNum,
  IN     BOOLEAN                            UpShift,
  IN OUT KERNEL_CONFIGURATION               *KernelConfig
  );

EFI_STATUS
GetLegacyBootDevType (
  OUT UINT16                                 *LegacyBootDevTypeCount,
  OUT UINT16                                 **LegacyBootDevType
  );

EFI_STATUS
UpdateBootOrderToActionOpcode (
  IN     EFI_HII_HANDLE                      BootHiiHandle,
  IN OUT KERNEL_CONFIGURATION                *KernelConfig
  );

EFI_STATUS
AdjustBootOrder (
  IN      BOOLEAN      EfiDeviceFirst,
  IN      UINTN        DeviceCount,
  IN OUT  UINT16       *BootOrder
  );

EFI_STATUS
ChangeToAutoBootOrder (
  IN      UINTN        DeviceCount,
  IN OUT  UINT16       *BootOrder
  );

BOOLEAN
IsUsbDevice (
  IN BBS_TABLE              *CurrentBbsTable
  );

BOOLEAN
IsEfiDevice (
  IN     UINT16    NumberOfDevice
  );

#endif

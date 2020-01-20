//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PcdLib.h>
//#include <Library/i2clib.h>
//#include <Library/PmicLib.h>
#include <Library/PrintLib.h>


#define CHARGER_BUSNO                0x02
#define FG_SLAVEADDRESS              0x55
#define FG_SLAVEOFFSET               0x2c
#define BUTTON_ARRAY_TIMER_INTERVAL         (2000000*5*2)             // 200ms
/*
//
// PIMIC Register
//
#define PMIC_MASK_RESETIRQ1_FCO              BIT5
#define PMIC_MASK_PWRSRCINT_ADPHVDET         BIT4
#define PMIC_MASK_CHIPCNTRL_COLD_OFF         BIT3
#define PMIC_REG_CHIPCNTRL                   0x100
#define PMIC_REG_PWRSRCINT                   0x005
#define PMIC_REG_RESETIRQ1                   0x010
#define PMIC_MASK_RESETIRQ1_CR               BIT0
#define PMIC_MASK_RESETIRQ1_WR               BIT1
#define PMIC_MASK_RESETIRQ1_CB               BIT2

*/

extern EFI_GUID                     gEfiNormalSetupGuid;

EFI_STATUS
BatteryIndicatorDrawEntry (
  VOID
  );

EFI_STATUS
CallBatteryIndicator (
  VOID
  );

typedef struct _BATTERY_DEV {
  EFI_HANDLE                              Handle;
  UINT8									  CurrentStateofCharge;
  UINT8									  gStatus;
  UINT8									  gCriticalBatteryLimit;
  EFI_EVENT                               TimerEvent;
} BATTERY_DEV;

VOID
EFIAPI
BatteryTimerHandler (
  IN EFI_EVENT    Event,
  IN VOID         *Context
 // IN VOID         *Context1
  );

/*
UINT8
EFIAPI
GetCurrentStateofCharge (
  IN BATTERY_DEV	*BatteryDevice
  );
  */

/*
VOID
EFIAPI
SimulateSoCHandler (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  );*/
/**@file

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

@par Specification
**/
/*++
Module Name:

  PupdrPlatformLib.h

Abstract:

  This library provides SMM functions for Stall.
  These can be used to save size and simplify code.
  All contents must be runtime and SMM safe.

--*/

//[-start-160517-IB03090427-modify]//
#ifndef _PLATFORM_INFO_LIB_H_
#define _PLATFORM_INFO_LIB_H_
#include <Pidv.h>
#include <Emt1.h>
#include "Pdr.h"

///
///for ShutdownSrc bit definition in platform PmicGetShutdownCause(UINT32 * ShutdownCause)
///and
///for ShutdownSrc bit definition in platform PmicGetResetSrc(UINT32 * ResetSrc)
///
#define  PUPDR_SHUTDOWN_CAUSE_RPMICWDTEXP        BIT15 ///<Previous shutdown was due to PMIC watchdog timer expiring
#define  PUPDR_SHUTDOWN_CAUSE_RBATTEMP           BIT14 ///<Previous shutdown was due to a critical battery over temperature event
#define  PUPDR_SHUTDOWN_CAUSE_RVSYSUVP           BIT13 ///<Previous shutdown was due to a VSYS under voltage event
#define  PUPDR_SHUTDOWN_CAUSE_RBATTRM            BIT12 ///<Previous shutdown was due to a battery removal event detected by VBAT comparator
#define  PUPDR_SHUTDOWN_CAUSE_RVCRIT             BIT11 ///<Previous shutdown was due to a violation of the VCRIT threshold
#define  PUPDR_SHUTDOWN_CAUSE_RSYSTEMP           BIT10 ///<Previous shutdown was due to a critical system over temperature event
#define  PUPDR_SHUTDOWN_CAUSE_RPMICTEMP          BIT9  ///<Previous shutdown was due to a critical PMIC internal over temperature event
#define  PUPDR_SHUTDOWN_CAUSE_RTHERMTRIP         BIT8  ///<Previous shutdown was due to a SoC thermal trip assertion


#define  PUPDR_SHUTDOWN_CAUSE_RPLATFORMRST       BIT5  ///<Reset was due to a platform reset
#define  PUPDR_SHUTDOWN_CAUSE_RSOCWDT            BIT4  ///<Reset was due to a SoC watchdog reset
#define  PUPDR_SHUTDOWN_CAUSE_RFCO               BIT3  ///<Reset was due to a Forced Cold Off
#define  PUPDR_SHUTDOWN_CAUSE_RWARMRESET         BIT2  ///<Reset was due to a WARM reset
#define  PUPDR_SHUTDOWN_CAUSE_RCOLDRESET         BIT1  ///<Reset was due to a COLD reset
#define  PUPDR_SHUTDOWN_CAUSE_RCOLDOFF           BIT0  ///<Reset was due to a SoC initiated cold off (normal shutdown)

///
/// for WakeCause bit definition in platform PmicGetWakeCause (IN OUT UINT32 *WakeCause)
///
#define  PUPDR_WAKE_CAUSE_WAKESPL                BIT7
#define  PUPDR_WAKE_CAUSE_WAKEPWRON              BIT6
#define  PUPDR_WAKE_CAUSE_WAKEWA                 BIT5
#define  PUPDR_WAKE_CAUSE_WAKEUSBID              BIT4
#define  PUPDR_WAKE_CAUSE_WAKEVDCIN              BIT3
#define  PUPDR_WAKE_CAUSE_WAKEVBUS               BIT2
#define  PUPDR_WAKE_CAUSE_WAKEBAT                BIT1
#define  PUPDR_WAKE_CAUSE_WAKEPBTN               BIT0

///
/// for PowerSrcIrq bit definition in Platform PmicGetPwrSrcIrq (IN OUT UINT32 *PowerSrcIrq)
///
#define  PUPDR_PWRSRCIRQ_BATDET                  BIT2
#define  PUPDR_PWRSRCIRQ_DCINDET                 BIT1
#define  PUPDR_PWRSRCIRQ_VBUSDET                 BIT0

///
/// SSRAM firmware logs offsets
///
#define PMC_RST_RECORD_0_OFFSET                  0x700
#define PMC_GLOBAL_RST_CAUSE_OFFSET              0x18


EFI_STATUS
EFIAPI
GetPlatformSpid (
  OUT SOFTWARE_PLATFORM_ID * Spid,
  OUT UINT8 *Ssn,
  OUT UINT8 *ExtID,
  OUT UINT8 *SystemUuid
  );

/*++

Routine Description:

  Get the information about SPID.

Arguments:

  The Address of SPID info.

Returns:

  EFI_SUCCESS                  Get platform SPID successfully
  EFI_INVALID_PARAMETER        Invalid pointer input
  EFI_DEVICE_ERROR             Cannot get platform SPID as expected

--*/

EFI_STATUS
EFIAPI
GetPlatformEM1Data (
  OUT EFI_ACPI_ENERGY_MANAGEMENT_1_TABLE * EM1
  );

/*++

Routine Description:

  Get the information about EM1.

Arguments:

  The Address of EM1 table.

Returns:

  EFI_SUCCESS                  Get platform SPID successfully
  EFI_INVALID_PARAMETER        Invalid pointer input
  EFI_DEVICE_ERROR             Cannot get platform SPID as expected

--*/

EFI_STATUS
EFIAPI
SetBatteryValid (
  IN UINT8     BatteryValid
  );

/*++

Routine Description:


Arguments:


Returns:

  EFI_SUCCESS                  Get platform SPID successfully
  EFI_INVALID_PARAMETER        Invalid pointer input
  EFI_DEVICE_ERROR             Cannot get platform SPID as expected

--*/

EFI_STATUS
EFIAPI
GetPlatformEM0Data (
  OUT PDR_ENERGE_MANAGEMENT * PdrEM
  );

EFI_STATUS
EFIAPI
GetPlatformOEM0Data (
  OUT VOID * OEM0
  );

EFI_STATUS
EFIAPI
GetPlatformOEM1Data (
  OUT EFI_ACPI_EM_OEM_1_TABLE * OEM1
  );

UINT8
EFIAPI
GetBatteryValid (
  VOID
  );

EFI_STATUS
EFIAPI
GetShutdownCause (
  IN OUT UINT32 *ShutdownCause
  );


EFI_STATUS
EFIAPI
GetWakeCause (
  IN OUT UINT32 *WakeCause
  );

EFI_STATUS
EFIAPI
GetPwrSrcIrq (
  IN OUT UINT32 *PowerSrcIrq
  );

VOID
EFIAPI
ClearWakeCause (
  VOID
  );

VOID
EFIAPI
ClearShutdownCause (
  VOID
  );

VOID
EFIAPI
RecordTcoStatus (
  VOID
  );

BOOLEAN
CheckTcoStatus (
  VOID
  );

EFI_STATUS
EFIAPI
GetBatteryCapacity (
  UINT8 * BatteryCapacity
  );

EFI_STATUS
EFIAPI
GetBatVoltage (
  IN OUT UINT16 *Voltage
  );

EFI_STATUS
EFIAPI
GetUsbChargerType (
  IN OUT UINTN *UsbChargerType
  );

EFI_STATUS
EFIAPI
GetPupdrSourceInit (
  );

EFI_STATUS
EFIAPI
GetPupdrWakeSource (
  OUT UINT8 *WakeSource
  );

EFI_STATUS
EFIAPI
GetPupdrResetSource (
  OUT UINT8 *ResetSource,
  OUT UINT8 *ResetType,
  OUT UINT32 *ResetExtraInfo
  );

EFI_STATUS
EFIAPI
GetPupdrShutdownSource (
  OUT UINT8 *ShutdownSource
  );

EFI_STATUS
GetPupdrSourceCleanUp (
  VOID
  );

EFI_STATUS
EFIAPI
GetPlatformBatteryConfigData (
  IN  EFI_GUID *ConfigFileGuid,
  OUT UINT8    *ConfigData,
  OUT UINTN    *ConigfSize
  );
#endif
//[-end-160517-IB03090427-modify]//

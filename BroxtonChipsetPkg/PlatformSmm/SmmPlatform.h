/** @file

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
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

  SmmPlatform.h

Abstract:

  Header file for
 
++*/

#ifndef _PLATFORM_H
#define _PLATFORM_H

//[-start-151229-IB03090424-modify]//
#include <PiSmm.h>



#include <Protocol/SmmBase2.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/SmmPowerButtonDispatch2.h>
#include <Protocol/SmmSxDispatch2.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmIchnDispatch.h>
#include <Protocol/SmmAccess2.h>
#include <Protocol/SmmVariable.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/LoadedImage.h>
#include "Protocol/GlobalNvsArea.h"
#include <Guid/AcpiVariableCompatibility.h>
#include <ChipsetSetupConfig.h>
#include <Guid/EfiVpdData.h>
#include <Guid/PciLanInfo.h>
#include <IndustryStandard/Pci22.h>

#include "ScAccess.h"
#include "CpuRegs.h"
//#include "CMOSMap.h"
//[-start-160824-IB07220130-add]//
#include <Library/CmosLib.h>
//[-end-160824-IB07220130-add]//
#include "ChipsetCmos.h"
#include "PlatformBaseAddresses.h"
#include "SaRegs.h"
#include <Library/UefiBootServicesTableLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>
#include <Library/SmmOemSvcKernelLib.h>
#include <Library/MemoryAllocationLib.h>
#include <ChipsetSmiTable.h>
#include <Library/SmmServicesTableLib.h>
#include <Protocol/SmmCpu.h>
#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)   
#include <Protocol/AcpiRestoreCallbackDone.h> 
#include <Protocol/OverrideAspm.h>
#include <Protocol/AcpiEnableCallbackDone.h>
#include <Protocol/AcpiDisableCallbackDone.h>
#endif
#include <Library/GpioLib.h>

typedef struct {
  UINT8     Register;
  UINT8     Function;
  UINT8     Device;
  UINT8     Bus;
  UINT32    ExtendedRegister;
} SMM_PCI_IO_ADDRESS;

typedef struct {
  CHAR8     BoardAaNumber[7];
  UINTN     BoardFabNumber;
} BOARD_AA_NUMBER_DECODE;

#define SMI_CMD_GET_MSEG_STATUS     0x70
#define SMI_CMD_UPDATE_MSEG_SIZE    0x71
#define SMI_CMD_LOAD_STM            0x72
#define SMI_CMD_UNLOAD_STM          0x73
#define SMI_CMD_GET_SMRAM_RANGES    0x74


#define PCAT_RTC_ADDRESS_REGISTER   0x74
#define PCAT_RTC_DATA_REGISTER      0x75

#define RTC_ADDRESS_SECOND          0x00
#define RTC_ADDRESS_SECOND_ALARM    0x01
#define RTC_ADDRESS_MINUTE          0x02
#define RTC_ADDRESS_MINUTE_ALARM    0x03
#define RTC_ADDRESS_HOUR            0x04
#define RTC_ADDRESS_HOUR_ALARM      0x05

#define RTC_ADDRESS_REGISTER_A      0x0A
#define RTC_ADDRESS_REGISTER_B      0x0B
#define RTC_ADDRESS_REGISTER_C      0x0C
#define RTC_ADDRESS_REGISTER_D      0x0D

#define B_RTC_ALARM_INT_ENABLE      0x20
#define B_RTC_ALARM_INT_STATUS      0x20

#define B_RTC_DATE_ALARM_MASK       0x3F

#define PCAT_CMOS_2_ADDRESS_REGISTER  0x72
#define PCAT_CMOS_2_DATA_REGISTER     0x73

#define EC_C_PORT                     0x66
#define SMC_SMI_DISABLE               0xBC
#define SMC_ENABLE_ACPI_MODE          0xAA  // Enable ACPI mode


//
// #defines for APM 1.2 functions
//
#define APM_INSTALLATION_CHECK                         0x00
#define APM_REAL_MODE_INTERFACE_CONNECT                0x01
#define APM_16_BIT_PROTECTED_MODE_INTERFACE_CONNECT    0x02
#define APM_32_BIT_PROTECTED_MODE_INTERFACE_CONNECT    0x03
#define APM_INTERFACE_DISCONNECT                       0x04
#define APM_CPU_IDLE                                   0x05
#define APM_CPU_BUSY                                   0x06
#define APM_SET_POWER_STATE                            0x07
#define APM_ENABLE_DISABLE_POWER_MANAGEMENT            0x08
#define APM_RESTORE_APM_BIOS_POWER_ON_DEFAULTS         0x09
#define APM_GET_POWER_STATUS                           0x0A
#define APM_GET_PM_EVENT                               0x0B
#define APM_GET_POWER_STATE                            0x0C
#define APM_ENABLE_DISABLE_DEVICE_POWER_MANAGEMENT     0x0D
#define APM_DRIVER_VERSION                             0x0E
#define APM_ENGAGE_DISENGAGE_POWER_MANAGEMENT          0x0F
#define APM_GET_CAPABILITIES                           0x10
#define APM_GET_SET_DISABLE_RESUME_TIMER               0x11
#define APM_ENABLE_DISABLE_RESUME_ON_RING_INDICATOR    0x12
#define APM_ENABLE_DISABLE_TIMER_BASED_REQUESTS        0x13
#define APM_OEM_DEFINED_FUNCTIONS                      0x80

//
// #defines for APM 1.2 variable mAPMInterfaceConnectState (also double as error codes)
//
#define APM_REAL_MODE                        0x02
#define APM_NOT_CONNECTED                    0x03
#define APM_16BIT_PROTECTED                  0x05
#define APM_32BIT_PROTECTED                  0x07
//
// #defines for additional APM 1.2 error codes
//
#define APM_FUNCTIONALITY_DISABLED                0x01
#define APM_UNRECOGNIZED_DEVICE_ID                0x09
#define APM_INTERFACE_NOT_ENGAGED                 0x0B
#define APM_FUNCTION_NOT_SUPPORTED                0x0C
#define APM_NO_POWER_MANAGEMENT_EVENTS_PENDING    0x80
//
// #defines for APM 1.2 miscellaneous support
//
#define APM_FLAG_16BIT_PROTECTED_SUPPORT     0x01
#define APM_FLAG_32BIT_PROTECTED_SUPPORT     0x02
#define APM_FLAG_DISABLED                    0x04
#define APM_FLAG_DISENGAGED                  0x08
#define APM_SET_POWER_STATE_OFF              0x03
#define APM_AC_LINE_STATUS_UNKNOWN           0xFF00
#define APM_BATTERY_STATUS_UNKNOWN           0x00FF
#define APM_BATTERY_FLAG_UNKNOWN             0xFF00
#define APM_BATTERY_LIFE_TIME_UNITS_UNKNOWN  0xFFFF


#define MAXIMUM_NUMBER_OF_PSTATES           12
#define  ICH_SMM_DATA_PORT                  0xB3

#define EFI_IA32_PMG_CST_CONFIG               0x000000E2
#define   B_EFI_CST_CONTROL_LOCK                BIT15
#define   B_EFI_IO_MWAIT_REDIRECTION_ENABLE     BIT10
#define EFI_IA32_PMG_IO_CAPTURE_ADDR          0x000000E4

extern EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *mPciRootBridgeIo;

//
// Callback function prototypes
//
EFI_STATUS
ChipsetPowerButtonCallback (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST  VOID                  *DispatchContext,
  IN  OUT VOID                                *CommBuffer  OPTIONAL,
  IN  UINTN                                   *CommBufferSize  OPTIONAL
  );

EFI_STATUS
S5SleepWakeOnLanCallBack (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  CONST VOID                              *DispatchContext,
  IN  OUT VOID                                *CommBuffer  OPTIONAL,
  IN  UINTN                                   *CommBufferSize  OPTIONAL
  );

EFI_STATUS
S5SleepAcLossCallBack (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  CONST VOID                              *DispatchContext,
  IN  OUT VOID                                *CommBuffer  OPTIONAL,
  IN  UINTN                                   *CommBufferSize  OPTIONAL
  );

EFI_STATUS
S5SleepCapsuleCallBack (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  CONST VOID                              *DispatchContext,
  IN  OUT VOID                                *CommBuffer  OPTIONAL,
  IN  UINTN                                   *CommBufferSize  OPTIONAL
  );

EFI_STATUS
S5SleepEntryCallBack (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST  VOID                  *DispatchContext,
  IN  OUT VOID                                *CommBuffer  OPTIONAL,
  IN  UINTN                                   *CommBufferSize  OPTIONAL
  );
  
EFI_STATUS
S4SleepEntryCallBack (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST VOID                              *DispatchContext,
  IN  OUT VOID                                *CommBuffer  OPTIONAL,
  IN  UINTN                                   *CommBufferSize  OPTIONAL
  );

EFI_STATUS
EnableAcpiCallback (
  IN  CONST VOID                    *DispatchContext,
  IN  VOID                          *Interface,
  IN  EFI_HANDLE                    Handle
  );
  
EFI_STATUS
DisableAcpiCallback (
  IN  CONST VOID                    *DispatchContext,
  IN  VOID                          *Interface,
  IN  EFI_HANDLE                    Handle
  );

EFI_STATUS
Apm12Callback (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST  VOID                  *DispatchContext,
  IN  OUT  VOID                    *CommBuffer,
  IN  OUT  UINTN                   *CommBufferSize
  );

EFI_STATUS
SmmReadyToBootCallback (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  CONST VOID                              *DispatchContext,
  IN  OUT VOID                                *CommBuffer  OPTIONAL,
  IN  UINTN                                   *CommBufferSize  OPTIONAL
  );

VOID
DummyTco1Callback (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
  );

VOID
PmeCallback (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
  );

VOID
EccCallback (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
  );

VOID
WatchdogCallback (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
  );

//VOID
//EnableWatchdogCallback (
//  IN  EFI_HANDLE                              DispatchHandle,
//  IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
//  );

VOID
SetAfterG3On (
  BOOLEAN Enable
  );

//[-start-160824-IB07220130-add]//
VOID
WakeToProcessPendingCapsule (
  IN UINT16                         AcpiBaseAddr,
  IN UINT8                          WakeAfter
  );
//[-end-160824-IB07220130-add]//

EFI_STATUS
S3SleepEntryCallBack (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST VOID                    *DispatchContext,
  IN  OUT VOID                      *CommBuffer  OPTIONAL,
  IN  UINTN                         *CommBufferSize  OPTIONAL
  );

EFI_STATUS
SaveRuntimeScriptTable (
  IN EFI_SMM_SYSTEM_TABLE2       *Smst
  );

/*+
VOID
WaitForMdiTransactionComplete(
  IN       UINT32   Data32,
  IN OUT   BOOLEAN  *ReadyBitSet
  );
-*/

EFI_STATUS
TpmPtsSmbsCallback (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST VOID                    *DispatchContext,
  IN  OUT VOID                      *CommBuffer  OPTIONAL,
  IN  UINTN                         *CommBufferSize  OPTIONAL
  );

EFI_STATUS
EFIAPI
PlatformRestoreAcpiCallbackDone (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  );

EFI_STATUS
EFIAPI
PlatformRestoreAcpiCallbackStart (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  );
//[-start-160421-IB10860195-remove]//
//[-start-160411-IB10860194-add]//
//EFI_STATUS
//SaveRestoreSmiEnable (
//  IN  BOOLEAN        SaveRestoreFlag
//  );
//[-end-160411-IB10860194-add]//
//[-end-160421-IB10860195-remove]//
EFI_STATUS
SaveRestorePMC (
  IN  BOOLEAN        SaveRestoreFlag
  );

#if ENBDT_PF_ENABLE
EFI_STATUS
SaveRestorePci (
  IN  BOOLEAN        SaveRestoreFlag
  );
#endif

EFI_STATUS
EFIAPI
SaveRestoreMtrr (
  IN  VOID        *FlagBuffer
  );
//[-end-151229-IB03090424-modify]//
//[-start-170626-IB07400880-add]//
VOID
WakeOnUsbFromS5 (
  IN UINT8  ConfigType // 1: Prepare to Sleep, 2: Ready to Shutdown   
  );

EFI_STATUS
EFIAPI
PrepareToSleepS5Callback (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST  VOID                  *DispatchContext,
  IN  OUT  VOID                    *CommBuffer,
  IN  OUT  UINTN                   *CommBufferSize
  );
//[-end-170626-IB07400880-add]//

#endif


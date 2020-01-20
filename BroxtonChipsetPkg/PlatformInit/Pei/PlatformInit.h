/*++

Copyright (c)  1999 - 2016 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PlatformEarlyInit.h

Abstract:

  Platform Early Stage header file

--*/

/*++
 This file contains an 'Intel Peripheral Driver' and is
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may
 be modified by the user, subject to additional terms of the
 license agreement
--*/
#ifndef _EFI_PLATFORM_INIT_H_
#define _EFI_PLATFORM_INIT_H_

#define EFI_FORWARD_DECLARATION(x) typedef struct _##x x

//-----------------------------------------------------


#include <CpuRegs.h>
#include <CpuType.h>
#include <FrameworkPei.h>
#include <PeiSaPolicyUpdate.h>
#include <Scregs/RegsPsf.h>
#include <Core/Pei/PeiMain.h>

#include <Pi/PiFirmwareFile.h>
#include <Pi/PiPeiCis.h>

#include <Guid/Capsule.h>
#include <Guid/EfiVpdData.h>
#include <Guid/FirmwareFileSystem.h>
#include <Guid/GlobalVariable.h>
#include <Guid/MemoryTypeInformation.h>
#include <Guid/PlatformInfo.h>

#include <Guid/RecoveryDevice.h>
//[-start-151127-IB02950555-modify]//
//#include <Guid/SetupVariable.h>
#include <ChipsetSetupConfig.h>
//[-end-151127-IB02950555-modify]//
#include <Guid/TpmInstance.h>
#include <Guid/PlatformCpuInfo.h>

#include <Ppi/AtaController.h>
#include <Ppi/BlockIo.h>
#include <Ppi/BootInRecoveryMode.h>
//[-start-160317-IB03090425-remove]//
//#include <Ppi/Cache.h>
//[-end-160317-IB03090425-remove]//
#include <Ppi/Capsule.h>
#include <Ppi/DeviceRecoveryModule.h>
#include <Ppi/EndOfPeiPhase.h>
//[-start-160429-IB03090426-remove]//
//#include <Ppi/FindFv.h>
//[-end-160429-IB03090426-remove]//
#include <Ppi/MasterBootMode.h>
#include <Ppi/MemoryDiscovered.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/RecoveryModule.h>
#include <Ppi/Reset.h>
#include <Ppi/SaPolicy.h>
#include <Ppi/Smbus.h>
#include <Ppi/Speaker.h>
#include <Ppi/Stall.h>
#include <Ppi/fTPMPolicy.h>
#include <Ppi/CpuPolicy.h>
#include <Ppi/FirmwareVolume.h>
#include <Ppi/DramPolicyPpi.h>
//[-start-160105-IB03090424-add]//
#include <Ppi/SmmAccess.h>
//[-end-160105-IB03090424-add]//
//[-start-151224-IB07220029-add]//
#include <Ppi/EmuPei.h>
//[-end-151224-IB07220029-add]//
//[-start-151229-IB03090424-modify]//
#include <Library/PeiPolicyInitLib.h>
#include <Library/PeiCpuPolicyUpdateLib.h>
//[-end-151229-IB03090424-modify]//
#include <Library/PeiSaPolicyLib.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
#include <Library/SideBandLib.h>
#include <Library/DebugLib.h>
#include <Library/TimerLib.h>
#include <Library/GpioLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PreSiliconLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <IndustryStandard/Pci22.h>
#include <Library/PmicLib.h>
//[-start-151216-IB07220025-add]//
#include <Library/PeiOemSvcKernelLib.h>
//[-end-151216-IB07220025-add]//
//[-start-160509-IB03090427-modify]//
#include <PmicReg_WhiskeyCove.h>
//[-end-160509-IB03090427-modify]//

#include "CMOSMap.h"
#include "LegacySpeaker.h"
#include "Platform.h"
#include <PlatformBaseAddresses.h>
#include <PlatformBootMode.h>
#include <SaAccess.h>
#include <ScAccess.h>
//[-start-151127-IB02950555-remove]//
//#include "SetupMode.h"
//[-start-151127-IB02950555-remove]//
#include <CpuAccess.h>

#define SMC_LAN_ON      0x46
#define SMC_LAN_OFF     0x47
#define SMC_DEEP_S3_STS 0xB2

//----------------------------------------
//
// Wake Event Types
//
#define SMBIOS_WAKEUP_TYPE_RESERVED           0x00
#define SMBIOS_WAKEUP_TYPE_OTHERS             0x01
#define SMBIOS_WAKEUP_TYPE_UNKNOWN            0x02
#define SMBIOS_WAKEUP_TYPE_APM_TIMER          0x03
#define SMBIOS_WAKEUP_TYPE_MODEM_RING         0x04
#define SMBIOS_WAKEUP_TYPE_LAN_REMOTE         0x05
#define SMBIOS_WAKEUP_TYPE_POWER_SWITCH       0x06
#define SMBIOS_WAKEUP_TYPE_PCI_PME            0x07
#define SMBIOS_WAKEUP_TYPE_AC_POWER_RESTORED  0x08

//
// Defines for stall PPI
//
#define PEI_STALL_RESOLUTION  1

//
// Define for Vibra
//
#define  PWM_DUTY_CYCLE            0x55  // zero duty cycle is 0x32/0xff
#define  PWM_BASE_UNIT             0x3   // DRV2603 work on 10k-250k while PWM base is 19.2M, 0x3/0xFF*19.2MHz ~ 200KHz
#define  PWM_TIMEOUT_MAX           100
//[-start-160317-IB03090425-add]//
#define  PWM_DELAY                 1000000
//[-end-160317-IB03090425-add]//

extern EFI_GUID gVbtInfoGuid;

typedef struct {
  EFI_PHYSICAL_ADDRESS    VbtAddress;
  UINT32                  VbtSize;
} VBT_INFO;

#pragma pack(push, 1)
typedef union {
  UINT32 Raw;
  struct {
    UINT32 Pwm_On_Time_Divisor    :8;  /*[7:0]*/
    UINT32 Pwm_Base_Unit          :16; /*[23:8]*/
    UINT32 Pwm_Rsvd               :6;  /*[29:24]*/
    UINT32 Pwm_Sw_Update          :1;  /*[30]*/
    UINT32 Pwm_Enable             :1;  /*[31]*/
  } BITS; /*Bits*/
} PWMCTRL;
#pragma pack(pop)

//
// Function Prototypes
//
//[-start-160429-IB03090426-remove]//
//VOID
//PlatformCpuInit (
//  IN CONST EFI_PEI_SERVICES      **PeiServices,
//  IN CHIPSET_CONFIGURATION       *SystemConfiguration
//  );
//[-end-160429-IB03090426-remove]//


EFI_STATUS
PlatformScInit (
  IN CHIPSET_CONFIGURATION       *SystemConfiguration,
  IN CONST EFI_PEI_SERVICES      **PeiServices,
  IN UINT16                      PlatformType
  );

//[-start-160429-IB03090426-remove]//
//EFI_STATUS
//PeimInitializeFlashMap (
//  IN EFI_FFS_FILE_HEADER        *FfsHeader,
//  IN CONST EFI_PEI_SERVICES           **PeiServices
//  );
//
//EFI_STATUS
//PeimInstallFlashMapPpi (
//  IN EFI_FFS_FILE_HEADER        *FfsHeader,
//  IN CONST EFI_PEI_SERVICES           **PeiServices
//  );
//[-end-160429-IB03090426-remove]//

EFI_STATUS
EFIAPI
IchReset (
  IN CONST EFI_PEI_SERVICES           **PeiServices
  );

BOOLEAN
GetSleepTypeAfterWakeup (
  IN  CONST EFI_PEI_SERVICES          **PeiServices,
  OUT UINT16                    *SleepType
  );

//[-start-160429-IB03090426-remove]//
//EFI_STATUS
//EFIAPI
//MemoryDiscoveredPpiNotifyCallback (
//  IN EFI_PEI_SERVICES           **PeiServices,
//  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
//  IN VOID                       *Ppi
//  )
//;
//
//EFI_STATUS
//EFIAPI
//PeiGetVariable (
//  IN CONST EFI_PEI_SERVICES       **PeiServices,
//  IN CHAR16                       *VariableName,
//  IN EFI_GUID                     * VendorGuid,
//  OUT UINT32                      *Attributes OPTIONAL,
//  IN OUT UINTN                    *DataSize,
//  OUT VOID                        *Data
//  );
//
//EFI_STATUS
//EFIAPI
//PeiGetNextVariableName (
//  IN CONST EFI_PEI_SERVICES       **PeiServices,
//  IN OUT UINTN                    *VariableNameSize,
//  IN OUT CHAR16                   *VariableName,
//  IN OUT EFI_GUID                 *VendorGuid
//  );
//[-end-160429-IB03090426-remove]//

EFI_STATUS
EndOfPeiPpiNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  );

//[-start-151229-IB03090424-add]//
//[-start-160330-IB07400715-remove]//
//#if (BXTI_PF_ENABLE == 1)
//VOID
//ConfigurePlatformPmic (
//  IN EFI_PEI_SERVICES           **PeiServices
//  );
//#endif
//[-end-160330-IB07400715-remove]//
//[-end-151229-IB03090424-add]//

EFI_STATUS
EFIAPI
PeimInitializeRecovery (
  IN CONST EFI_PEI_SERVICES     **PeiServices
  );

VOID
CheckPowerOffNow (
  VOID
  );

//[-start-160429-IB03090426-remove]//
//EFI_STATUS
//PcieSecondaryBusReset (
//  IN CONST EFI_PEI_SERVICES  **PeiServices,
//  IN UINT8             Bus,
//  IN UINT8             Dev,
//  IN UINT8             Fun
//  );
//[-end-160429-IB03090426-remove]//

VOID
SetPlatformBootMode (
  IN CONST EFI_PEI_SERVICES             **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB *PlatformInfoHob
  );

//[-start-160429-IB03090426-remove]//
//EFI_STATUS
//FindFv (
//  IN EFI_PEI_FIND_FV_PPI          *This,
//  IN EFI_PEI_SERVICES             **PeiServices,
//  IN OUT UINT8                    *FvNumber,
//  OUT EFI_FIRMWARE_VOLUME_HEADER  **FVAddress
//  );
//[-end-160429-IB03090426-remove]//

EFI_STATUS
EFIAPI
CpuOnlyReset (
  IN CONST EFI_PEI_SERVICES   **PeiServices
  );

EFI_STATUS
EFIAPI
Stall (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN CONST EFI_PEI_STALL_PPI    *This,
  IN UINTN                      Microseconds
  );

EFI_STATUS
MultiPlatformInfoInit (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB *PlatformInfoHob
  );

BOOLEAN
IsRecoveryJumper (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB *PlatformInfoHob
);

EFI_STATUS
PlatformInfoUpdate (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB *PlatformInfoHob,
  IN CHIPSET_CONFIGURATION     *SystemConfiguration
  );

//[-start-160413-IB03090426-remove]//
//VOID
//PlatformSaInit (
//IN CHIPSET_CONFIGURATION       *SystemConfiguration,
//IN CONST EFI_PEI_SERVICES      **PeiServices
//  );
//[-end-160413-IB03090426-remove]//

EFI_STATUS
InitializePlatform (
  IN CONST EFI_PEI_SERVICES             **PeiServices,
  IN EFI_PLATFORM_INFO_HOB        *PlatformInfoHob,
  IN CHIPSET_CONFIGURATION        *SystemConfiguration
);

//[-start-160317-IB03090425-remove]//
//EFI_STATUS
//EFIAPI
//SetPeiCacheMode (
//  IN  CONST EFI_PEI_SERVICES    **PeiServices
//  );
//[-end-160317-IB03090425-remove]//

EFI_STATUS
EFIAPI
SetDxeCacheMode (
  IN  CONST EFI_PEI_SERVICES    **PeiServices
  );

EFI_STATUS
GeneralPowerFailureHandler (
  IN CONST EFI_PEI_SERVICES       **PeiServices
  );

BOOLEAN
IsRtcUipAlwaysSet (
  IN CONST EFI_PEI_SERVICES       **PeiServices
  );

EFI_STATUS
RtcPowerFailureHandler (
  IN CONST EFI_PEI_SERVICES       **PeiServices
  );

EFI_STATUS
EFIAPI
PublishMemoryTypeInfo (
  VOID
  );

EFI_STATUS
EFIAPI
PeiGetSectionFromFv (
  IN CONST  EFI_GUID        NameGuid,
  OUT VOID                  **Address,
  OUT UINT32               *Size
  );

EFI_STATUS
EFIAPI
ConfigClockTrunk (
  IN CONST EFI_PEI_SERVICES  **PeiServices
  );

EFI_STATUS
EFIAPI
PlatformInitFinalConfig (
  IN CONST EFI_PEI_SERVICES           **PeiServices
  );

EFI_STATUS
ValidateFvHeader (
  IN EFI_FIRMWARE_VOLUME_HEADER            *FwVolHeader
  );

EFI_STATUS
UpdateBootMode (
  IN CONST EFI_PEI_SERVICES                       **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB                    *PlatformInfoHob
  );


//[-start-160216-IB03090424-add]//
//[-start-160406-IB07400715-remove]//
//#if (BXTI_PF_ENABLE == 1)
//[-end-160406-IB07400715-remove]//
//[-start-160317-IB03090425-modify]//
UINT8
ConfigurePlatformPmic (
  VOID
  );
//[-end-160317-IB03090425-modify]//
//[-start-161018-IB06740518-add]//
VOID
ConfigurePmicIMON (
  VOID
  );
//[-end-161018-IB06740518-add]//

EFI_STATUS
ScPlatformSccDllOverride (
  VOID
  );

//[-start-160509-IB03090427-add]//
VOID 
ScUsb2PhyOverride (
  VOID
  );
//[-end-160509-IB03090427-add]//
//[-start-160406-IB07400715-remove]//
//#endif
//[-end-160406-IB07400715-remove]//
//[-end-160216-IB03090424-add]//
#endif // _EFI_PLATFORM_INIT_H_

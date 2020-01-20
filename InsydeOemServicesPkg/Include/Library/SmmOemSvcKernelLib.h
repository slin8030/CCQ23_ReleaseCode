/** @file
  Definition for Oem Services Default Lib.

;******************************************************************************
;* Copyright (c) 2012- 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SMM_OEM_SVC_KERNEL_LIB_H_
#define _SMM_OEM_SVC_KERNEL_LIB_H_

#include <Uefi.h>
#include <DmiStringInformation.h>
#include <BeepStatusCode.h>
#include <H2OIhisi.h>
#include <Protocol/HddPasswordService.h>

#define PCIE_ASPM_DEV_END_OF_TABLE  0xFFFF

#pragma pack (1)
//
// Device List Structure
//
typedef struct _PCIE_ASPM_DEV_INFO {
  UINT16    VendorId;           // offset 0
  UINT16    DeviceId;           // offset 2
  UINT8     RevId;              // offset 8; 0xFF means all steppings
  UINT8     RootAspm;
  UINT8     EndpointAspm;
} PCIE_ASPM_DEV_INFO;

#pragma pack ()


//
// OemSvc function prototypes
//
EFI_STATUS
OemSvcDisableAcpiCallback (
  VOID
  );

EFI_STATUS
OemSvcEnableAcpiCallback (
  VOID
  );

EFI_STATUS
OemSvcGetAspmOverrideInfo (
  OUT PCIE_ASPM_DEV_INFO                    **PcieAspmDevs
  );

EFI_STATUS
OemSvcSmmInstallPnpGpnvTable (
  OUT UINTN                                 *UpdateableGpnvCount,
  OUT OEM_GPNV_MAP                          **GetOemGPNVMap
  );

EFI_STATUS
OemSvcSmmInstallPnpStringTable (
  OUT UINTN                                 *UpdateableStringCount,
  OUT DMI_UPDATABLE_STRING                  **mUpdatableStrings
  );

EFI_STATUS
OemSvcPowerButtonCallback (
  VOID
  );

EFI_STATUS
OemSvcRestoreAcpiCallback (
  VOID
  );

EFI_STATUS
OemSvcS1Callback (
  VOID
  );

EFI_STATUS
OemSvcS3Callback (
  VOID
  );

EFI_STATUS
OemSvcS4Callback (
  VOID
  );

EFI_STATUS
OemSvcS5AcLossCallback (
  VOID
  );

EFI_STATUS
OemSvcS5Callback (
  VOID
  );

EFI_STATUS
OemSvcOsResetCallback (
  VOID
  );

EFI_STATUS
OemSvcVariablePreservedTable (
  IN OUT PRESERVED_VARIABLE_TABLE              **VariablePreservedTable,
  IN OUT BOOLEAN                               *IsKeepVariableInList
  );

EFI_STATUS
OemSvcVariablePreservedTable2 (
  IN OUT PRESERVED_VARIABLE_TABLE_2            **VariablePreservedTable
  );

EFI_STATUS
OemSvcIhisiGetWholeBiosRomMap (
  OUT VOID                                     **BiosRomMap,
  OUT UINTN                                    *NumberOfRegions
  );

EFI_STATUS
OemSvcUpdateStatusCodeBeep (
  OUT STATUS_CODE_BEEP_ENTRY           **StatusCodeBeepList,
  OUT BEEP_TYPE                        **BeepTypeList
  );

EFI_STATUS
OemSvcIhisiS1FHookFbtsApHookForBios (
  IN UINT8          ApState
  );

EFI_STATUS
OemSvcIhisiS10HookGetAcStatus (
  IN OUT UINT8      *AcStatus
  );

EFI_STATUS
OemSvcIhisiS10HookGetBatterylife (
  IN OUT UINT8      *BattLife
  );

EFI_STATUS
OemSvcIhisiS10HookGetPermission (
  IN OUT UINT16     *Permission
  );

EFI_STATUS
OemSvcIhisiS11HookFbtsApCheck (
  IN OUT UINT8      *ApStatus
  );

EFI_STATUS
OemSvcIhisiS12HookGetOemFlashMap (
  IN OUT UINTN                          *Media_mOemRomMapSize,
  IN OUT FBTS_PLATFORM_ROM_MAP          **Media_mOemRomMap,
  IN OUT UINTN                          *Media_mOemPrivateRomMapSize,
  IN OUT FBTS_PLATFORM_PRIVATE_ROM      **Media_mOemPrivateRomMap
  );

EFI_STATUS
OemSvcIhisiS20HookFetsDoAfterFlashing (
  IN UINT8          ActionAfterFlashing
  );

EFI_STATUS
OemSvcIhisiS20HookFetsDoBeforeFlashing (
  IN OUT UINT8      **FlashingDataBuffer,
  IN OUT UINTN      *SizeToFlash,
  IN OUT UINT8      *DestBlockNo
  );

EFI_STATUS
OemSvcIhisiS20HookFetsEcFlash (
  IN UINT8          *FlashingDataBuffer,
  IN UINTN          SizeToFlash,
  IN UINT8          DestBlockNo
  );

EFI_STATUS
OemSvcIhisiS20HookFetsEcIdle (
  IN BOOLEAN        Idle
  );

EFI_STATUS
OemSvcIhisiS21HookFetsGetPartInfo (
  IN OUT UINT32     *IhisiStatus,
  IN OUT UINT32     *EcPartSize
  );

EFI_STATUS
OemSvcIhisiS41CommonCommuniction (
  IN AP_COMMUNICATION_DATA_TABLE        *ApCommDataBuffer,
  IN OUT BIOS_COMMUNICATION_DATA_TABLE  *BiosCommDataBuffer
  );

EFI_STATUS
OemSvcIhisiS41ReservedFunction (
  IN AP_COMMUNICATION_DATA_TABLE        *ApCommDataBuffer,
  IN OUT BIOS_COMMUNICATION_DATA_TABLE  *BiosCommDataBuffer
  );

EFI_STATUS
OemSvcIhisiS41T1VbiosFunction (
  IN AP_COMMUNICATION_DATA_TABLE        *ApCommDataBuffer,
  IN OUT BIOS_COMMUNICATION_DATA_TABLE  *BiosCommDataBuffer
  );

EFI_STATUS
OemSvcIhisiS41T50HookOa30ReadWriteFunction (
  IN AP_COMMUNICATION_DATA_TABLE        *ApCommDataBuffer,
  IN OUT BIOS_COMMUNICATION_DATA_TABLE  *BiosCommDataBuffer
  );

EFI_STATUS
OemSvcIhisiS41T54LogoUpdateFunction (
  IN AP_COMMUNICATION_DATA_TABLE        *ApCommDataBuffer,
  IN OUT BIOS_COMMUNICATION_DATA_TABLE  *BiosCommDataBuffer
  );

EFI_STATUS
OemSvcIhisiS41T55CheckBiosSignBySystemBiosFunction (
  IN AP_COMMUNICATION_DATA_TABLE        *ApCommDataBuffer,
  IN OUT BIOS_COMMUNICATION_DATA_TABLE  *BiosCommDataBuffer
  );

EFI_STATUS
OemSvcSmmHddPasswordUnlockAllHdd (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL  *HddPasswordService,
  IN  HDD_PASSWORD_HDD_INFO              *HddInfoArray,
  IN  UINTN                              NumOfHdd,
  IN  VOID                               *HddPasswordTablePtr,
  IN  UINTN                              HddPasswordTableSize
  );

EFI_STATUS
OemSvcSmmHddPasswordStringProcess (
  IN UINT8                                PasswordType,
  IN   VOID                               *RawPasswordPtr,
  IN   UINTN                              RawPasswordLength,
  OUT  UINT8                              **PasswordPtr,
  OUT  UINTN                              *PasswordLength
  );

EFI_STATUS
OemSvcSmmHddSetPassword (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL  *HddPasswordService,
  IN  HDD_PASSWORD_HDD_INFO              *HddInfo,
  IN  BOOLEAN                            UserOrMaster,
  IN  UINT8                              *PasswordPtr,
  IN  UINTN                              PasswordLength
  );

EFI_STATUS
OemSvcSmmHddUnlockPassword (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL  *HddPasswordService,
  IN  HDD_PASSWORD_HDD_INFO              *HddInfo,
  IN  BOOLEAN                            UserOrMaster,
  IN  UINT8                              *PasswordPtr,
  IN  UINTN                              PasswordLength
  );

EFI_STATUS
OemSvcSmmHddDisablePassword (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL  *HddPasswordService,
  IN  HDD_PASSWORD_HDD_INFO              *HddInfo,
  IN  BOOLEAN                            UserOrMaster,
  IN  UINT8                              *PasswordPtr,
  IN  UINTN                              PasswordLength
  );

EFI_STATUS
OemSvcSmmHddUpdateSecurityStatus (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *HddPasswordService,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo,
  IN  UINTN                             Index
  );

EFI_STATUS
OemSvcSmmHddResetSecurityStatus (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL  *HddPasswordService,
  IN  HDD_PASSWORD_HDD_INFO              *HddInfo,
  IN  VOID                               *HddPasswordTablePtr,
  IN  UINTN                              HddPasswordTableSize
  );

#endif

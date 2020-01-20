/** @file
  Definition for Dxe OEM Services Chipset Lib.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _DXE_OEM_SVC_CHIPSET_LIB_H_
#define _DXE_OEM_SVC_CHIPSET_LIB_H_

#include <PiDxe.h>
//[-start-151220-IB1127138-add]//
#include <Protocol/SaPolicy.h>
//[-end-151220-IB1127138-add]//
#include <Library/BadgingSupportLib.h>
#include <Protocol/PlatformGopPolicy.h>
//#include <Protocol/PpmPlatformPolicy.h>
#include <Protocol/SeCPlatformPolicy.h>
#include <Protocol/UsbLegacyPlatform.h>
#include <ScPolicyCommon.h>
#include <Protocol/BiosRegionLock.h>
#include <IndustryStandard/Acpi10.h>
#include <ChipsetFota.h>
#include <ChipsetSetupConfig.h>
#include <Protocol/PciRootBridgeIo.h>
//#include <Protocol/CpuPlatformPolicyProtocol.h>
#include <Protocol/IgdOpRegion.h>
#include <Protocol/GlobalNvsArea.h>

#define SCU_ACTION_LOAD_DEFAULT BIT0

/**
 This function provides an interface to modify the ACPI description table header.

 @param[in out]     *Table              On entry, points to ACPI description table header.
                                        On exit , points to updated ACPI description table header.
 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
*/
EFI_STATUS
OemSvcModifyAcpiDescriptionHeader (
  IN OUT  EFI_ACPI_DESCRIPTION_HEADER            *Table
  );

/**
 This function provides an interface to modify OEM Logo and POST String.

 @param[in out]     *EFI_OEM_BADGING_LOGO_DATA                   On entry, points to a structure that specifies image data.
                                                                 On exit , points to updated structure.
 @param[in out]     *BadgingDataSize    On entry, the size of EFI_OEM_BADGING_LOGO_DATA matrix.
                                        On exit , the size of updated EFI_OEM_BADGING_LOGO_DATA matrix.
 @param[in out]     *OemBadgingString   On entry, points to OEM_BADGING_STRING matrix.
                                        On exit , points to updated OEM_BADGING_STRING matrix.
 @param[in out]     *OemBadgingStringInTextMode                  On entry, points to OEM_BADGING_STRING matrix in text mode.
                                                                 On exit , points to updated OEM_BADGING_STRING matrix in text mode.
 @param[in out]     *StringCount        The number is POST string count.
                                        On entry, base on SetupVariable->QuietBoot 
                                                  1 : The number of entries in OemBadgingString,
                                                  0 : The number of entries in OemBadgingStringInTextMode.
                                        On exit , base on SetupVariable->QuietBoot 
                                                  1 : The number of entries in updated OemBadgingString,
                                                  0 : The number of entries in updated OemBadgingStringInTextMode.
 @param[in out]     *OemBadgingStringAfterSelect                 On entry, points to OEM_BADGING_STRING matrix after selected.
                                                                 On exit , points to updated OEM_BADGING_STRING matrix after selected.
 @param[in out]     *OemBadgingStringAfterSelectInTextMode       On entry, points to OEM_BADGING_STRINGmatrix after selected  in text mode.
                                                                 On exit , points to updated OEM_BADGING_STRING matrix after selected  in text mode.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/

EFI_STATUS
OemSvcUpdateOemBadgingLogoData (
  IN OUT EFI_OEM_BADGING_LOGO_DATA            **EfiOemBadgingLogoData,
  IN OUT UINTN                                *BadgingDataSize,
  IN OUT OEM_BADGING_STRING                   **OemBadgingString,
  IN OUT OEM_BADGING_STRING                   **OemBadgingStringInTextMode,
  IN OUT UINTN                                *StringCount,
  IN OUT OEM_BADGING_STRING                   **OemBadgingStringAfterSelect,
  IN OUT OEM_BADGING_STRING                   **OemBadgingStringAfterSelectInTextMode
  );

/**
 This function offers an interface to modify EFI_PLATFORM_CPU_PROTOCOL Data before the system
 installs EFI_PLATFORM_CPU_PROTOCOL.

 @param[in, out]    *PlatformCpuPolicy  On entry, points to EFI_PLATFORM_CPU_PROTOCOL structure.
                                        On exit, points to updated EFI_PLATFORM_CPU_PROTOCOL structure.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
//EFI_STATUS
//OemSvcUpdateDxePlatformCpuPolicy (
//  IN OUT EFI_PLATFORM_CPU_PROTOCOL   *PlatformCpuPolicy
//  );

/**
 This function offers an interface to modify PLATFORM_GOP_POLICY_PROTOCOL Data before the system
 installs PLATFORM_GOP_POLICY_PROTOCOL.

@param[in, out]    *PlatformGopPolicy   On entry, points to PLATFORM_GOP_POLICY_PROTOCOL structure.
                                        On exit, points to updated PLATFORM_GOP_POLICY_PROTOCOL structure.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcUpdateDxePlatformGopPolicy (
  IN OUT PLATFORM_GOP_POLICY_PROTOCOL   *PlatformGopPolicy
  );

/**
 This function offers an interface to modify PPM_PLATFORM_POLICY_PROTOCOL Data before the system
 installs PPM_PLATFORM_POLICY_PROTOCOL.

@param[in, out]    *PpmPlatformPolicy   On entry, points to PPM_PLATFORM_POLICY_PROTOCOL structure.
                                        On exit, points to updated PPM_PLATFORM_POLICY_PROTOCOL structure.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
//EFI_STATUS
//OemSvcUpdateDxePpmPlatformPolicy (
//  IN OUT PPM_PLATFORM_POLICY_PROTOCOL   *PpmPlatformPolicy
//  );

/**
 This function offers an interface to modify DXE_SEC_POLICY_PROTOCOL Data before the system
 installs DXE_SEC_POLICY_PROTOCOL.

@param[in, out]    *SecPlatformPolicy   On entry, points to DXE_SEC_POLICY_PROTOCOL structure.
                                        On exit, points to updated DXE_SEC_POLICY_PROTOCOL structure.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcUpdateDxeSecPlatformPolicy (
  IN OUT DXE_SEC_POLICY_PROTOCOL   *SecPlatformPolicy
  );
  
/**
 This function provides an interface to modify USB legacy options.

 @param[in out]     *UsbLegacyModifiers On entry, points to USB_LEGACY_MODIFIERS instance.
                                        On exit , points to updated USB_LEGACY_MODIFIERS instance.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
*/
EFI_STATUS
OemSvcSetUsbLegacyPlatformOptions (
  IN OUT USB_LEGACY_MODIFIERS  *UsbLegacyModifiers
  );
  
/**
  This service is called, when the status S5 or S4 resumes to OS. 
  Provide OEM to add project characteristic code.

  @param  Base on OEM design.

  @retval EFI_UNSUPPORTED       Returns unsupported by default.
  @retval EFI_SUCCESS           The service is customized in the project.
  @retval EFI_MEDIA_CHANGED     The value of IN OUT parameter is changed. 
  @retval Others                Depends on customization.
**/
EFI_STATUS
OemSvcEnableAcpiCallback (
  VOID
  );

/**
 This function offers an interface to modify DXE_PCH_PLATFORM_POLICY_PROTOCOL Data before the system
 installs DXE_PCH_PLATFORM_POLICY_PROTOCOL.

 @param[in, out]    *PchPolicy          On entry, points to DXE_PCH_PLATFORM_POLICY_PROTOCOL structure.
                                        On exit, points to updated DXE_PCH_PLATFORM_POLICY_PROTOCOL structure.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
//EFI_STATUS
//OemSvcUpdatDxePchPlatformPolicy (
//  IN OUT DXE_PCH_PLATFORM_POLICY_PROTOCOL         *PchPolicy
//  );

/**
  Customize BIOS protect region before boot to OS. 

  @param[out]        *BiosRegionTable    Pointer to BiosRegion Table.
  @param[out]        ProtectRegionNum    The number of Bios protect region instances.
  
  @retval            EFI_UNSUPPORTED     Returns unsupported by default, OEM did not implement this function.
  @retval            EFI_MEDIA_CHANGED   Provide table for kernel to set protect region and lock flash program registers. 
**/
EFI_STATUS
OemSvcUpdateBiosProtectTable (
  OUT BIOS_PROTECT_REGION           **BiosRegionTable,
  OUT UINT8                         *ProtectRegionNum
  );

EFI_STATUS
OemSvcFotaVariablePreservedTable (
  IN OUT FOTA_PRESERVED_VARIABLE_TABLE         **VariablePreservedTable,
  IN OUT BOOLEAN                               *IsKeepVariableInList
  );

EFI_STATUS
OemSvcFotaBiosProtectTable (
  OUT BIOS_PROTECT_REGION           **BiosRegionTable,
  OUT UINT8                         *ProtectRegionNum
  );

/**
 This function provides an interface to hook before and after DxePlatformEntryPoint (PlatformDxe.inf).PlatformDxe.inf
 is responsible for setting up any platform specific policy or initialization information.

 @param[in]     *SetupVariable      On entry, points to CHIPSET_CONFIGURATION instance.
                                    On exit , points to updated CHIPSET_CONFIGURATION instance.
 @param[in]     *PciRootBridgeIo    On entry, points to EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL instance.
                                    On exit , points to updated EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL instance.
 @param[in]     Flag                TRUE : Before DxePlatformEntryPoint.
                                    FALSE: After DxePlatformEntryPoint.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcHookPlatformDxe (
  IN  CHIPSET_CONFIGURATION            *SetupVariable,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *PciRootBridgeIo,
  IN  BOOLEAN                          Flag
  );

/**
 This function provides an interface to update GlobalNvs table content.

 @param[in, out]    mGlobalNvsArea      On entry, points to EFI_GLOBAL_NVS_AREA  instance.
                                        On exit , points to updated EFI_GLOBAL_NVS_AREA  instance.
 @param[in, out]    mOemGlobalNvsArea   On entry, points to EFI_OEM_GLOBAL_NVS_AREA  instance.
                                        On exit , points to updated EFI_OEM_GLOBAL_NVS_AREA  instance.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcUpdateGlobalNvs (
  EFI_GLOBAL_NVS_AREA                   *GlobalNvsArea
  );

/**
 This function provides an interface to update IGD OpRegion content.

 @param [in out]    *IgdOpRegion        On entry, points to memorys buffer for Internal graphics device, 
                                        this buffer set aside comminocate between ACPI code and POST.
                                        On exit , points to updated memory buffer for Internal Graphics Device.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter.
*/
EFI_STATUS
OemSvcSetIgdOpRegion (
  IN OUT IGD_OPREGION_PROTOCOL          *IgdOpRegion
  );

/**
 This function provides an interface to hook PlatformResetSystem 

 @param[in]         ResetType           Warm or cold.
 @param[in]         ResetStatus         Possible cause of reset.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcHookPlatformReset (
  IN EFI_RESET_TYPE                     ResetType,
  IN EFI_STATUS                         ResetStatus
  );

/**

 This function provides an interface to set Boot Display Device

  @param             SetupNVRam         Points to CHIPSET_CONFIGURATION instance

  @retval            EFI_UNSUPPORTED    Returns unsupported by default.
  @retval            EFI_MEDIA_CHANGED  Alter the Configuration Parameter.
  @retval            EFI_SUCCESS        The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcSetBootDisplayDevice (
  IN  CHIPSET_CONFIGURATION             *SetupNVRam
  );

/**
 This function provides an interface to hook before display Logo.

 @param             None

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcHookBeforeLogoDisplay (
  VOID
  );

/**
 This function provides an interface to hook after display Logo.

 @param             None

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcHookAfterLogoDisplay (
  VOID
  );

/**
 This function provides an interface to hook after POST hotkey is detected.

 @param[in]     Selection           The detected hotkey value.
 @param[in]     BootMode            Current Boot mode.
 @param[in]     NoBootDevices       TRUE : No boot device exist.
                                    FALSE: Bootable device is detected.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcHookAfterHotkeyDetect (
  IN UINT16                             Selection,
  IN EFI_BOOT_MODE                      BootMode,
  IN BOOLEAN                            NoBootDevices
  );

/**
 This function provides an interface to hook when boot to UEFI boot option failed..

 @param[in]     *InputOption        Content of BDS_COMMON_OPTION
 @param[in]     Status              Return status from BdsLibBootViaBootOption function
 @param[in]     *ExitData           The string data return from boot image
 @param[in]     ExitDataSize        Size of ExitData
 
 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcHookUefiBootFail (
  IN  VOID                              *InputOption,
  IN  EFI_STATUS                        Status,
  IN  CHAR16                            *ExitData,
  IN  UINTN                             ExitDataSize
  );

/**
 This function provides an interface to hook when there is no bootable device been detect.

 @param             None

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcHookNoBootableDevice (
  VOID
  );

/**
 This function provides an interface to implement platform PMIC initializaion.

 @param             None

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcHookPlatformPmicInitialization (
  VOID
  );

/**
  This function provides an interface to hook GenericRouteConfig.

  @param[in,out]     ScBuffer            A pointer to CHIPSET_CONFIGURATION struct.
  @param[in]         BufferSize          System configuration size.
  @param[in]         ScuRecord           The bit mask of the currently SCU record.
                                           Bit 0 = 1 (SCU_ACTION_LOAD_DEFAULT), It indicates system do load default action.

  @retval            EFI_UNSUPPORTED     This function is a pure hook; Chipset code don't care return status.
  @retval            EFI_SUCCESS         This function is a pure hook; Chipset code don't care return status. 
**/
EFI_STATUS
OemSvcHookRouteConfig (
  IN OUT CHIPSET_CONFIGURATION          *ScBuffer, 
  IN     UINT32                         BufferSize,
  IN     UINT32                         ScuRecord
  );
//[-start-151220-IB1127138-add]//
/**
 This function offers an interface to modify SA_POLICY_PROTOCOL Data before the system
 installs SA_POLICY_PROTOCOL.

 @param[in, out]    *SaPlatformPolicy   On entry, points to SA_POLICY_PROTOCOL structure.
                                        On exit, points to updated SA_POLICY_PROTOCOL structure.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcUpdateDxePlatformSaPolicy (
  IN OUT SA_POLICY_PROTOCOL          *SaPlatformPolicy
  );
//[-end-151220-IB1127138-add]//
#endif

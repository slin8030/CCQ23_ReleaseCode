## @file
#  Platform Package Description file
#
#******************************************************************************
#* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************
## @file
# EFI/Framework Emulation Platform with UEFI HII interface supported.
#
# The Emulation Platform can be used to debug individual modules, prior to creating
#    a real platform. This also provides an example for how an DSC is created.
#
# Copyright (c) 2006 - 2013, Intel Corporation. All rights reserved.<BR>
#
#    This program and the accompanying materials
#    are licensed and made available under the terms and conditions of the BSD License
#    which accompanies this distribution. The full text of the license may be found at
#    http://opensource.org/licenses/bsd-license.php
#
#    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]

################################################################################
#
# SKU Identification section - list of all SKU IDs supported by this
#                              Platform.
#
################################################################################
[SkuIds]
  0|DEFAULT              # The entry: 0|DEFAULT is reserved and always required.

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses]
  #
  # Entry point
  #
  PeiCoreEntryPoint|MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  #
  # Basic
  #
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf {
    <SOURCE_OVERRIDE_PATH>
      MdePkg/Override/Library/BaseLib
      NtEmulatorPkg/Override/MdePkg/Library/BaseLib
  }
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf {
    <SOURCE_OVERRIDE_PATH>
      NtEmulatorPkg/Override/MdePkg/Library/BaseIoLibIntrinsic
  }
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  CacheMaintenanceLib|MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  #
  # UEFI & PI
  #
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
# HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  HiiLib|InsydeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiDecompressLib|IntelFrameworkModulePkg/Library/BaseUefiTianoCustomDecompressLib/BaseUefiTianoCustomDecompressLib.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf

  #
  # Generic Modules
  #
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
  NetLib|MdeModulePkg/Library/DxeNetLib/DxeNetLib.inf
  IpIoLib|MdeModulePkg/Library/DxeIpIoLib/DxeIpIoLib.inf
  UdpIoLib|MdeModulePkg/Library/DxeUdpIoLib/DxeUdpIoLib.inf
  DpcLib|MdeModulePkg/Library/DxeDpcLib/DxeDpcLib.inf
  #OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  GenericBdsLib|IntelFrameworkModulePkg/Library/GenericBdsLib/GenericBdsLib.inf {
    <SOURCE_OVERRIDE_PATH>
      NtEmulatorPkg/Override/IntelFrameworkModulePkg/Library/GenericBdsLib
  }
  #CustomizedDisplayLib|MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  #TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  TimerLib|InsydeModulePkg/Library/CpuTimerLib/CpuTimerLib.inf

  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  #
  # Platform
  #
  PlatformBdsLib|NtEmulatorPkg/Library/NtEmulatorBdsLib/NtEmulatorBdsLib.inf
  #
  # Misc
  #
!if $(EFI_DEBUG) == YES
  DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
!else
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
!endif
  DebugPrintErrorLevelLib|MdeModulePkg/Library/DxeDebugPrintErrorLevelLib/DxeDebugPrintErrorLevelLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  CpuExceptionHandlerLib|MdeModulePkg/Library/CpuExceptionHandlerLibNull/CpuExceptionHandlerLibNull.inf
  LockBoxLib|MdeModulePkg/Library/LockBoxNullLib/LockBoxNullLib.inf

!if $(SECURE_BOOT_ENABLE) == TRUE
  PlatformSecureLib|NtEmulatorPkg/Library/PlatformSecureLib/PlatformSecureLib.inf
  IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
  TpmMeasurementLib|SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf
!endif

  NULL|MdeModulePkg/Override/Library/CompilerStubLib/CompilerStubLib.inf
  PostCodeLib|MdePkg/Library/PeiDxePostCodeLibReportStatusCode/PeiDxePostCodeLibReportStatusCode.inf

  #
  # InsydeModulePkg
  #
  AcpiPlatformLib|InsydeModulePkg/Library/CommonPlatformLib/AcpiPlatformLib/AcpiPlatformLib.inf
  BitBltLib|InsydeModulePkg/Library/BitBltLib/BitBltLib.inf
  BvdtLib|InsydeModulePkg/Library/BvdtLib/BvdtLib.inf {
    <SOURCE_OVERRIDE_PATH>
      NtEmulatorPkg/Override/InsydeModulePkg/Library/BvdtLib
  }
  CmosLib|InsydeModulePkg/Library/CmosLibNull/CmosLibNull.inf
  CommonEcLib|InsydeModulePkg/Library/CommonEcLib/CommonEcLib.inf
  CommonPciLib|InsydeModulePkg/Library/CommonPlatformLib/CommonPciLib/CommonPciLib.inf
  ConsoleLib|InsydeModulePkg/Library/ConsoleLib/ConsoleLib.inf
  HiiExLib|InsydeModulePkg/Library/UefiHiiExLib/UefiHiiExLib.inf
  IrsiRegistrationLib|InsydeModulePkg/Library/Irsi/IrsiRegistrationLib/IrsiRegistrationLib.inf
  MultiConfigBaseLib|InsydeModulePkg/Library/MultiConfigBaseLib/MultiConfigBaseLib.inf
  OemGraphicsLib|InsydeModulePkg/Library/OemGraphicsLib/OemGraphicsLib.inf
  S3BootScriptLib|MdePkg/Library/BaseS3BootScriptLibNull/BaseS3BootScriptLibNull.inf
  VariableLib|InsydeModulePkg/Library/DxeVariableLib/DxeVariableLib.inf
  VariableSupportLib|InsydeModulePkg/Library/VariableSupportLib/VariableSupportLib.inf
  BdsCpLib|InsydeModulePkg/Library/BdsCpLib/BdsCpLib.inf

  StdLib|InsydeModulePkg/Library/StdLib/StdLib.inf
  RectLib|InsydeModulePkg/Library/RectLib/RectLib.inf
  Win32Lib|InsydeModulePkg/Library/Win32Lib/Win32Lib.inf

  BaseOemSvcKernelLib|InsydeOemServicesPkg/Library/BaseOemSvcKernelLib/BaseOemSvcKernelLib.inf
  BaseOemSvcKernelLibDefault|InsydeOemServicesPkg/Library/BaseOemSvcKernelLib/BaseOemSvcKernelLibDefault.inf
  DxeOemSvcKernelLibDefault|InsydeOemServicesPkg/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLibDefault.inf
  DxeChipsetSvcLib|InsydeModulePkg/Library/DxeChipsetSvcLib/DxeChipsetSvcLib.inf

  SetupUtilityLib|InsydeModulePkg/Library/SetupUtilityLib/SetupUtilityLib.inf
  KernelConfigLib|InsydeModulePkg/Library/DxeKernelConfigLib/DxeKernelConfigLib.inf
  KernelVfrLib|InsydeModulePkg/Library/SetupUtilityLib/KernelVfrLib.inf
  BaseSetupDefaultLib|InsydeModulePkg/Library/SetupDefaultLib/SetupDefaultLib.inf

  GenericUtilityLib|InsydeModulePkg/Library/GenericUtilityLib/GenericUtilityLib.inf {
    <SOURCE_OVERRIDE_PATH>
      NtEmulatorPkg/Override/InsydeModulePkg/Library/GenericUtilityLib
  }

!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserSupported
  LayoutLib|InsydeSetupPkg/Library/LayoutSupportLib/LayoutSupportLib.inf
  H2ODisplayEngineLib|InsydeSetupPkg/Library/H2ODisplayEngineLib/H2ODisplayEngineLib.inf
!endif


[LibraryClasses.common.USER_DEFINED]
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  OemHookStatusCodeLib|NtEmulatorPkg/Library/PeiNtEmulatorOemHookStatusCodeLib/PeiNtEmulatorOemHookStatusCodeLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

[LibraryClasses.common.PEIM,LibraryClasses.common.PEI_CORE]
  #
  # PEI phase common
  #
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibOptPei/BaseMemoryLibOptPei.inf
  #IoLib|MdePkg/Library/PeiIoLibCpuIo/PeiIoLibCpuIo.inf
  PeCoffGetEntryPointLib|NtEmulatorPkg/Library/NtEmulatorPeiPeCoffGetEntryPointLib/NtEmulatorPeiPeCoffGetEntryPointLib.inf
  PeCoffExtraActionLib|NtEmulatorPkg/Library/PeiNtEmulatorPeCoffExtraActionLib/PeiNtEmulatorPeCoffExtraActionLib.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf

[LibraryClasses.common.PEI_CORE]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf

[LibraryClasses.common.PEIM]
  PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  OemHookStatusCodeLib|NtEmulatorPkg/Library/PeiNtEmulatorOemHookStatusCodeLib/PeiNtEmulatorOemHookStatusCodeLib.inf
!if $(SECURE_BOOT_ENABLE) == TRUE
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/PeiCryptLib.inf
!endif

  SeamlessRecoveryLib|InsydeModulePkg/Library/PeiSeamlessRecoveryLib/SeamlessRecoveryLib.inf
  VariableLib|InsydeModulePkg/Library/PeiVariableLib/PeiVariableLib.inf
  FdSupportLib|InsydeModulePkg/Library/FlashDeviceSupport/FdSupportLib/FdSupportLib.inf
  FlashDevicesLib|InsydeFlashDevicePkg/Library/FlashDevicesLib/FlashDevicesLib.inf
  SpiAccessLib|InsydeModulePkg/Library/FlashDeviceSupport/SpiAccessLibNull/SpiAccessLibNull.inf

  PeiOemSvcKernelLibDefault|InsydeOemServicesPkg/Library/PeiOemSvcKernelLib/PeiOemSvcKernelLibDefault.inf

[LibraryClasses.common]
  #
  # DXE phase common
  #
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibOptDxe/BaseMemoryLibOptDxe.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  OemHookStatusCodeLib|NtEmulatorPkg/Library/DxeNtEmulatorOemHookStatusCodeLib/DxeNtEmulatorOemHookStatusCodeLib.inf
  PeCoffExtraActionLib|NtEmulatorPkg/Library/DxeNtEmulatorPeCoffExtraActionLib/DxeNtEmulatorPeCoffExtraActionLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  WinNtLib|NtEmulatorPkg/Library/DxeWinNtLib/DxeWinNtLib.inf
!if $(SECURE_BOOT_ENABLE) == TRUE
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
!endif

[LibraryClasses.common.DXE_CORE]
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

[LibraryClasses.common.DXE_SMM_DRIVER]
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf

[LibraryClasses.common.UEFI_DRIVER]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf


[LibraryClasses.common.UEFI_APPLICATION]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  #
  # Runtime
  #
!if $(SECURE_BOOT_ENABLE) == TRUE
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/RuntimeCryptLib.inf
!endif

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplSwitchToLongMode|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplBuildPageTables|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdPeiCoreImageLoaderSearchTeSectionFirst|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdVariableCollectStatistics|TRUE

  gInsydeTokenSpaceGuid.PcdSysPasswordInCmos|FALSE

[PcdsFixedAtBuild]
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxSizeNonPopulateCapsule|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxSizePopulateCapsule|0x0
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000040
  gEfiNtEmulatorPkgTokenSpaceGuid.PcdWinNtFirmwareFdSize|0x2a0000
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x1f
  gEfiNtEmulatorPkgTokenSpaceGuid.PcdWinNtFirmwareVolume|L"..\\Fv\\NtEmulator.fd"
  gEfiNtEmulatorPkgTokenSpaceGuid.PcdWinNtFirmwareBlockSize|0x10000
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x0f
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|FALSE
#!if $(SECURE_BOOT_ENABLE) == TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x2000
#!endif

!if $(SECURE_BOOT_ENABLE) == TRUE
  # override the default values from SecurityPkg to ensure images from all sources are verified in secure boot
  gEfiSecurityPkgTokenSpaceGuid.PcdOptionRomImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdFixedMediaImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdRemovableMediaImageVerificationPolicy|0x04
!endif

  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultPlatformLangCodes|$(RFC_LANGUAGES)

  # @PcdTypeStruct OEM_LOGO_RESOLUTION_DEFINITION {UINT32 LogoResolutionX; UINT32 LogoResolutionY; UINT32 ScuResolutionX; UINT32 ScuResolutionY;}
  # @PcdValueType OEM_LOGO_RESOLUTION_DEFINITION
  gInsydeTokenSpaceGuid.PcdDefaultLogoResolution|{0x20, 0x03, 0x00, 0x00, 0x58, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00}

################################################################################
#
# Pcd Dynamic Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsDynamicDefault.common.DEFAULT]
  gEfiNtEmulatorPkgTokenSpaceGuid.PcdWinNtSerialPort|L""|VOID*|20
  gEfiNtEmulatorPkgTokenSpaceGuid.PcdWinNtGop|L"UGA Window 1"|VOID*|52
  gEfiNtEmulatorPkgTokenSpaceGuid.PcdWinNtConsole|L"Bus Driver Console Window"|VOID*|52
  gEfiNtEmulatorPkgTokenSpaceGuid.PcdWinNtVirtualDisk|L"FW;40960;512"|VOID*|26
  gEfiNtEmulatorPkgTokenSpaceGuid.PcdWinNtMemorySize|L"64!64"|VOID*|12
  gEfiNtEmulatorPkgTokenSpaceGuid.PcdWinNtPhysicalDisk|L"a:RW;2880;512!d:RO;307200;2048!j:RW;262144;512"|VOID*|100
# gEfiNtEmulatorPkgTokenSpaceGuid.PcdWinNtUga|L"UGA Window 1!UGA Window 2"|VOID*|52

  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase|0

  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|40
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|128
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1024
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|768
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1024
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|768
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdSetupConOutColumn|128
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdSetupConOutRow|40

[PcdsDynamicDefault.Ia32]
  gEfiNtEmulatorPkgTokenSpaceGuid.PcdWinNtFileSystem|L".!..\..\..\..\EdkShellBinPkg\Bin\Ia32\Apps"|VOID*|106

[PcdsDynamicDefault.x64]
  gEfiNtEmulatorPkgTokenSpaceGuid.PcdWinNtFileSystem|L".!..\..\..\..\EdkShellBinPkg\Bin\X64\Apps"|VOID*|106
[PcdsDynamicHii.common.DEFAULT]
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdSetupConOutColumn|L"SetupConsoleConfig"|gEfiNtEmulatorPkgTokenSpaceGuid|0x0|80
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdSetupConOutRow|L"SetupConsoleConfig"|gEfiNtEmulatorPkgTokenSpaceGuid|0x4|25
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdPlatformBootTimeOut|L"Timeout"|gEfiGlobalVariableGuid|0x0|10
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdHardwareErrorRecordLevel|L"HwErrRecSupport"|gEfiGlobalVariableGuid|0x0|1

###################################################################################################
#
# Components Section - list of the modules and components that will be processed by compilation
#                      tools and the EDK II tools to generate PE32/PE32+/Coff image files.
#
# Note: The EDK II DSC file is not used to specify how compiled binary images get placed
#       into firmware volume images. This section is just a list of modules to compile from
#       source into UEFI-compliant binaries.
#       It is the FDF file that contains information on combining binary files into firmware
#       volume images, whose concept is beyond UEFI and is described in PI specification.
#       Binary modules do not need to be listed in this section, as they should be
#       specified in the FDF file. For example: Shell binary (Shell_Full.efi), FAT binary (Fat.efi),
#       Logo (Logo.bmp), and etc.
#       There may also be modules listed in this section that are not required in the FDF file,
#       When a module listed here is excluded from FDF file, then UEFI-compliant binary will be
#       generated for it, but the binary will not be put into any firmware volume.
#
###################################################################################################
[Components]


  ##
  #  SEC Phase modules
  ##
  NtEmulatorPkg/Sec/SecMain.inf

  ##
  #  PEI Phase modules
  ##
  MdeModulePkg/Core/Pei/PeiMain.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  MdeModulePkg/Universal/ReportStatusCodeRouter/Pei/ReportStatusCodeRouterPei.inf
  MdeModulePkg/Universal/StatusCodeHandler/Pei/StatusCodeHandlerPei.inf
  NtEmulatorPkg/WinNtOemHookStatusCodeHandlerPei/WinNtOemHookStatusCodeHandlerPei.inf
  NtEmulatorPkg/BootModePei/BootModePei.inf
  NtEmulatorPkg/StallPei/StallPei.inf
  NtEmulatorPkg/WinNtFlashMapPei/WinNtFlashMapPei.inf
  MdeModulePkg/Universal/FaultTolerantWritePei/FaultTolerantWritePei.inf
!if $(SECURE_BOOT_ENABLE) == TRUE
  SecurityPkg/VariableAuthenticated/Pei/VariablePei.inf
!else
  MdeModulePkg/Universal/Variable/Pei/VariablePei.inf
!endif



  NtEmulatorPkg/WinNtAutoScanPei/WinNtAutoScanPei.inf
  NtEmulatorPkg/WinNtFirmwareVolumePei/WinNtFirmwareVolumePei.inf
  NtEmulatorPkg/WinNtThunkPPIToProtocolPei/WinNtThunkPPIToProtocolPei.inf
  MdeModulePkg/Core/DxeIplPeim/DxeIpl.inf
  ##
  #  DXE Phase modules
  ##
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <SOURCE_OVERRIDE_PATH>
      MdeModulePkg/Override/Core/Dxe
    <LibraryClasses>
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
  }

  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  NtEmulatorPkg/MetronomeDxe/MetronomeDxe.inf
  NtEmulatorPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
  NtEmulatorPkg/ResetRuntimeDxe/ResetRuntimeDxe.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  NtEmulatorPkg/FvbServicesRuntimeDxe/FvbServicesRuntimeDxe.inf
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf  {
    <LibraryClasses>
!if $(SECURE_BOOT_ENABLE) == TRUE
      NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
!endif
  }
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  MdeModulePkg/Universal/EbcDxe/EbcDxe.inf
  MdeModulePkg/Universal/MemoryTest/NullMemoryTestDxe/NullMemoryTestDxe.inf
  NtEmulatorPkg/WinNtThunkDxe/WinNtThunkDxe.inf
  NtEmulatorPkg/CpuRuntimeDxe/CpuRuntimeDxe.inf
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
  NtEmulatorPkg/MiscSubClassPlatformDxe/MiscSubClassPlatformDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      NtEmulatorPkg/Override/NtEmulatorPkg/MiscSubClassPlatformDxe
  }
  NtEmulatorPkg/TimerDxe/TimerDxe.inf
  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
  NtEmulatorPkg/WinNtOemHookStatusCodeHandlerDxe/WinNtOemHookStatusCodeHandlerDxe.inf
!if $(SECURE_BOOT_ENABLE) == TRUE
  SecurityPkg/VariableAuthenticated/RuntimeDxe/VariableRuntimeDxe.inf
  SecurityPkg/VariableAuthenticated/SecureBootConfigDxe/SecureBootConfigDxe.inf
!else
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
!endif
  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  #MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf {
  #  <LibraryClasses>
  #    PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  #}
  #MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf {
  #  <LibraryClasses>
  #    PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  #}
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  #MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf     ##This driver follows UEFI specification definition
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf    ##This driver follows UEFI specification definition
  IntelFrameworkModulePkg/Bus/Pci/IdeBusDxe/IdeBusDxe.inf
  NtEmulatorPkg/WinNtBusDriverDxe/WinNtBusDriverDxe.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }
  NtEmulatorPkg/WinNtBlockIoDxe/WinNtBlockIoDxe.inf
  NtEmulatorPkg/WinNtSerialIoDxe/WinNtSerialIoDxe.inf
  NtEmulatorPkg/WinNtGopDxe/WinNtGopDxe.inf
  NtEmulatorPkg/WinNtSimpleFileSystemDxe/WinNtSimpleFileSystemDxe.inf
  MdeModulePkg/Application/HelloWorld/HelloWorld.inf

  #
  # Network stack drivers
  # To test network drivers, need network Io driver(SnpNtEmulatorIo.dll), please refer to NETWORK-IO Subproject.
  #
!if gInsydeTokenSpaceGuid.PcdH2ONetworkSupported
  MdeModulePkg/Universal/Network/DpcDxe/DpcDxe.inf
  MdeModulePkg/Universal/Network/ArpDxe/ArpDxe.inf
  MdeModulePkg/Universal/Network/Dhcp4Dxe/Dhcp4Dxe.inf
  MdeModulePkg/Universal/Network/Ip4ConfigDxe/Ip4ConfigDxe.inf
  MdeModulePkg/Universal/Network/Ip4Dxe/Ip4Dxe.inf
  MdeModulePkg/Universal/Network/MnpDxe/MnpDxe.inf
  MdeModulePkg/Universal/Network/VlanConfigDxe/VlanConfigDxe.inf
  MdeModulePkg/Universal/Network/Mtftp4Dxe/Mtftp4Dxe.inf
  MdeModulePkg/Universal/Network/Tcp4Dxe/Tcp4Dxe.inf
  MdeModulePkg/Universal/Network/Udp4Dxe/Udp4Dxe.inf
  MdeModulePkg/Universal/Network/UefiPxeBcDxe/UefiPxeBcDxe.inf
  NtEmulatorPkg/SnpNtEmulatorDxe/SnpNtEmulatorDxe.inf {
    <LibraryClasses>
      NULL|EdkCompatibilityPkg/Foundation/Library/CompilerStub/CompilerStubLib.inf
  }

  MdeModulePkg/Universal/Network/IScsiDxe/IScsiDxe.inf
!endif

  IntelFrameworkModulePkg/Universal/BdsDxe/BdsDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      NtEmulatorPkg/Override/IntelFrameworkModulePkg/Universal/BdsDxe
  }
  # MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  #MdeModulePkg/Universal/DisplayEngineDxe/DisplayEngineDxe.inf
  #MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
  MdeModulePkg/Universal/DriverSampleDxe/DriverSampleDxe.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  MdeModulePkg/Application/VariableInfo/VariableInfo.inf

  MdeModulePkg/Universal/PlatformDriOverrideDxe/PlatformDriOverrideDxe.inf

  #
  # PEI driver
  #
  InsydeModulePkg/Universal/Variable/SetupDataProviderPei/SetupDataProviderPei.inf

  #
  # DXE driver
  #
  InsydeModulePkg/Library/SetupUtilityLib/SetupUtilityLib.inf
  InsydeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }
  InsydeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  InsydeModulePkg/Universal/UserInterface/UnicodeCollationDxe/UnicodeCollationDxe.inf
  InsydeModulePkg/Universal/UserInterface/HiiDatabaseDxe/HiiDatabaseDxe.inf

  InsydeModulePkg/Tools/GenBvdt/GenBvdt.inf {
    <BuildOptions>
       *_*_IA32_DLINK_FLAGS == /out:"$(BIN_DIR)\$(BASE_NAME)" /base:0x10000000 /NOLOGO /SUBSYSTEM:CONSOLE /NODEFAULTLIB /IGNORE:4086 /MAP /OPT:REF /MACHINE:I386 /LTCG Kernel32.lib LIBCMT.lib
       *_*_X64_DLINK_FLAGS ==  /out:"$(BIN_DIR)\$(BASE_NAME)" /base:0x10000000 /NOLOGO /SUBSYSTEM:CONSOLE /NODEFAULTLIB /IGNORE:4086 /MAP /OPT:REF /MACHINE:AMD64 /LTCG Kernel32.lib LIBCMT.lib
  }

  InsydeModulePkg/Universal/Console/JpegDecoderDxe/JpegDecoderDxe.inf
  InsydeModulePkg/Universal/Console/GifDecoderDxe/GifDecoderDxe.inf
  InsydeModulePkg/Universal/Console/PngDecoderDxe/PngDecoderDxe.inf

  InsydeModulePkg/Universal/UserInterface/SetupUtilityApp/SetupUtilityApp.inf
  InsydeModulePkg/Universal/UserInterface/SetupMouseDxe/SetupMouseDxe.inf
  InsydeModulePkg/Universal/Security/SysPasswordDxe/SysPasswordDxe.inf
  InsydeModulePkg/Universal/UserInterface/Fonts/TrueTypeDecoderDxe/TrueTypeDecoderDxe.inf

  #
  # Form Browser / Display Engine
  #
!if gEfiNtEmulatorPkgTokenSpaceGuid.PcdEdk2FormBrowserSupported
  MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  MdeModulePkg/Universal/DisplayEngineDxe/DisplayEngineDxe.inf {
    <LibraryClasses>
      CustomizedDisplayLib|MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
  }
!else
!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserSupported
  InsydeModulePkg/Universal/UserInterface/H2OKeyDescDxe/H2OKeyDescDxe.inf
  InsydeSetupPkg/Drivers/DisplayTypeDxe/DisplayTypeDxe.inf
  InsydeSetupPkg/Drivers/HiiLayoutPkgDxe/HiiLayoutPkgDxe.inf
  InsydeSetupPkg/Drivers/H2OFormBrowserDxe/H2OFormBrowserDxe.inf
!if gInsydeTokenSpaceGuid.PcdH2OSetupChangeDisplaySupported
  InsydeSetupPkg/Drivers/H2OSetupChangeDxe/H2OSetupChangeDxe.inf
!endif

!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalTextDESupported
  InsydeSetupPkg/Drivers/H2ODisplayEngineLocalTextDxe/H2ODisplayEngineLocalTextDxe.inf
!endif

!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalMetroDESupported
  InsydeModulePkg/Universal/UserInterface/MicrowindowsDxe/MicrowindowsDxe.inf
  InsydeSetupPkg/Drivers/H2ODisplayEngineLocalMetroDxe/H2ODisplayEngineLocalMetroDxe.inf
!endif

  #
  # For tune performance, this driver should be removed after final version.
  #
  InsydeSetupPkg/Drivers/PerfDxe/PerfDxe.inf
!else
!if gInsydeTokenSpaceGuid.PcdGraphicsSetupSupported
  InsydeModulePkg/Universal/UserInterface/MicrowindowsDxe/MicrowindowsDxe.inf
  InsydeModulePkg/Universal/UserInterface/SetupBrowserDxe/GraphicsSetupBrowserDxe.inf
!else
  InsydeModulePkg/Universal/UserInterface/SetupBrowserDxe/SetupBrowserDxe.inf
!endif
!endif
!endif

###################################################################################################
#
# BuildOptions Section - Define the module specific tool chain flags that should be used as
#                        the default flags for a module. These flags are appended to any
#                        standard flags that are defined by the build process. They can be
#                        applied for any modules or only those modules with the specific
#                        module style (EDK or EDKII) specified in [Components] section.
#
###################################################################################################
[BuildOptions]
  DEBUG_*_IA32_DLINK_FLAGS =  /EXPORT:InitializeDriver=$(IMAGE_ENTRY_POINT) /BASE:0x10000 /ALIGN:4096 /FILEALIGN:4096 /SUBSYSTEM:CONSOLE
  RELEASE_*_IA32_DLINK_FLAGS = /ALIGN:4096 /FILEALIGN:4096
  *_*_IA32_CC_FLAGS = /Od
  *_*_IA32_DLINK_FLAGS = /OPT:NOICF

  DEBUG_*_X64_DLINK_FLAGS = /EXPORT:InitializeDriver=$(IMAGE_ENTRY_POINT) /BASE:0x10000 /ALIGN:4096 /FILEALIGN:4096 /SUBSYSTEM:CONSOLE
  RELEASE_*_X64_DLINK_FLAGS = /ALIGN:4096 /FILEALIGN:4096
  *_*_X64_CC_FLAGS = /Od
  *_*_X64_DLINK_FLAGS = /OPT:NOICF

#############################################################################################################
# NOTE:
# The following [Libraries.X64] section is for building EDK module under the EDKII tool chain.
# If you want build EDK module for NtEmulator platform, please uncomment [Libraries.X64] section and
# libraries used by that EDK module.
# Currently, NtEmulator platform do not have any EDK style module
#
#
#[Libraries]
  #
  # Libraries common to PEI and DXE
  #
  #  EdkCompatibilityPkg/Foundation/Efi/Guid/EfiGuidLib.inf
  #  EdkCompatibilityPkg/Foundation/Framework/Guid/EdkFrameworkGuidLib.inf
  #  EdkCompatibilityPkg/Foundation/Guid/EdkGuidLib.inf
  #  EdkCompatibilityPkg/Foundation/Library/EfiCommonLib/EfiCommonLib.inf
  #  EdkCompatibilityPkg/Foundation/Cpu/Pentium/CpuIA32Lib/CpuIA32Lib.inf
  #  EdkCompatibilityPkg/Foundation/Cpu/Itanium/CpuIa64Lib/CpuIA64Lib.inf
  #  EdkCompatibilityPkg/Foundation/Library/CustomizedDecompress/CustomizedDecompress.inf
  #  EdkCompatibilityPkg/Foundation/Library/CompilerStub/CompilerStubLib.inf
  #  EdkCompatibilityPkg/Foundation/Library/Dxe/Hob/HobLib.inf

  #
  # PEI libraries
  #
  #  EdkCompatibilityPkg/Foundation/Framework/Ppi/EdkFrameworkPpiLib.inf
  #  EdkCompatibilityPkg/Foundation/Ppi/EdkPpiLib.inf
  #  EdkCompatibilityPkg/Foundation/Library/Pei/PeiLib/PeiLib.inf
  #  EdkCompatibilityPkg/Foundation/Library/Pei/Hob/PeiHobLib.inf

  #
  # DXE libraries
  #
  #  EdkCompatibilityPkg/Foundation/Core/Dxe/ArchProtocol/ArchProtocolLib.inf
  #  EdkCompatibilityPkg/Foundation/Efi/Protocol/EfiProtocolLib.inf
  #  EdkCompatibilityPkg/Foundation/Framework/Protocol/EdkFrameworkProtocolLib.inf
  #  EdkCompatibilityPkg/Foundation/Protocol/EdkProtocolLib.inf
  #  EdkCompatibilityPkg/Foundation/Library/Dxe/EfiDriverLib/EfiDriverLib.inf
  #  EdkCompatibilityPkg/Foundation/Library/RuntimeDxe/EfiRuntimeLib/EfiRuntimeLib.inf
  #  EdkCompatibilityPkg/Foundation/Library/Dxe/Graphics/Graphics.inf
  #  EdkCompatibilityPkg/Foundation/Library/Dxe/EfiIfrSupportLib/EfiIfrSupportLib.inf
  #  EdkCompatibilityPkg/Foundation/Library/Dxe/Print/PrintLib.inf
  #  EdkCompatibilityPkg/Foundation/Library/Dxe/EfiScriptLib/EfiScriptLib.inf
  #  EdkCompatibilityPkg/Foundation/Library/Dxe/EfiUiLib/EfiUiLib.inf

  #
  # Print/Graphics Library consume SetupBrowser Print Protocol
  #
  #  EdkCompatibilityPkg/Foundation/Library/Dxe/PrintLite/PrintLib.inf
  #  EdkCompatibilityPkg/Foundation/Library/Dxe/GraphicsLite/Graphics.inf


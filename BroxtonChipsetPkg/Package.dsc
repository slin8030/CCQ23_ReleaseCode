## @file
#  Platform Package Description file
#
#******************************************************************************
#* Copyright (c) 2013 - 2018, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************

[Defines]
  DEFINE    CHIPSET_PKG = BroxtonChipsetPkg

  DEFINE    CHIPSET_OVERRIDE =$(CHIPSET_PKG)/Override
  DEFINE    PCH_INIT_ROOT    =$(PLATFORM_RC_PACKAGE)/SouthCluster/PchInit

  !include $(CHIPSET_PKG)/Package.env

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################

[LibraryClasses]
  #
  # Basic
  #
  BaseOemSvcChipsetLib|$(CHIPSET_PKG)/Library/BaseOemSvcChipsetLib/BaseOemSvcChipsetLib.inf
  BaseOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/BaseOemSvcChipsetLib/BaseOemSvcChipsetLibDefault.inf
  !disable GenericBdsLib|InsydeModulePkg/Library/GenericBdsLib/GenericBdsLib.inf
  GenericBdsLib|InsydeModulePkg/Library/GenericBdsLib/GenericBdsLib.inf{
  <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/GenericBdsLib/
  }
  UefiDecompressLib|IntelFrameworkModulePkg/Library/BaseUefiTianoCustomDecompressLib/BaseUefiTianoCustomDecompressLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/IntelFrameworkModulePkg/Library/BaseUefiTianoCustomDecompressLib
  }
  #
  # Related to Platform
  #
  BiosIdLib|$(CHIPSET_PKG)/Library/BiosIdLib/BiosIdLib.inf
#[-start-160803-IB07220122-remove]#
#   PlatformConfigDataLib|$(CHIPSET_PKG)/Library/PlatformConfigDataLib/PlatformConfigDataLib.inf
#[-end-160803-IB07220122-remove]#
  #
  # Related to  PCH
  #
#[-start-170407-IB07400856-modify]#
  SmbusLib|$(CHIPSET_PKG)/Library/SmbusLibNull/SmbusLib.inf
#[-end-170407-IB07400856-modify]#
  SmmLib|$(CHIPSET_PKG)/Library/PchSmmLib/PchSmmLib.inf
  #ResetSystemLib|$(CHIPSET_PKG)/Library/ResetSystemLib/ResetSystemLib.inf
  ResetSystemLib|MdeModulePkg/Library/BaseResetSystemLibNull/BaseResetSystemLibNull.inf
  I2cDriverLib|$(CHIPSET_PKG)/Library/I2cStackLib/I2cDriverLib/I2cDriverLib.inf
#[-start-161116-IB05400732-add]#
  IoDecodeLib|$(CHIPSET_PKG)/Library/IoDecodeLib/IoDecodeLib.inf
#[-end-161116-IB05400732-add]#

#[-start-151123-IB11270133-remove]#
#   !disable SetupUtilityLib|InsydeModulePkg/Library/SetupUtilityLib/SetupUtilityLib.inf
#   SetupUtilityLib|InsydeModulePkg/Library/SetupUtilityLib/SetupUtilityLib.inf{
#     <SOURCE_OVERRIDE_PATH>
#       $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/SetupUtilityLib
#   }
#[-end-151123-IB11270133-remove]#

  !disable FlashRegionLib|InsydeModulePkg/Library/FlashRegionLib/DxeFlashRegionLib/DxeFlashRegionLib.inf
  FlashRegionLib|InsydeModulePkg/Library/FlashRegionLib/DxeFlashRegionLib/DxeFlashRegionLib.inf{
  <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/FlashRegionLib/DxeFlashRegionLib/
  }
  #
  # End of PCH
  #

  PlatformBdsLib|$(CHIPSET_PKG)/Library/PlatformBdsLib/PlatformBdsLib.inf
  CapsuleUpdateCriteriaLib|InsydeModulePkg/Library/CapsuleUpdateCriteriaLib/CapsuleUpdateCriteriaLib.inf

  #
  # Generic Modules
  #
  SmmIoLib|$(CHIPSET_PKG)/Library/SmmIo/SmmIoLib.inf
  IoApicLib|PcAtChipsetPkg/Library/BaseIoApicLib/BaseIoApicLib.inf

  #
  # BUGBUG: 2012/10/23 Cmos access functions collosion with CmosLib in InsydeModulePkg
  #                    Must remove it and use Insyde style library.
  #

  #
  # Platform
  #
  PeiPolicyInitLib|$(CHIPSET_PKG)/Library/PeiPolicyInitLib/PeiPolicyInitLib.inf
  PeiPolicyUpdateLib|$(CHIPSET_PKG)/Library/PeiPolicyUpdateLib/PeiPolicyUpdateLib.inf
  Stage2HashLib|$(CHIPSET_PKG)/Library/Stage2HashLib/Stage2HashLib.inf
  PlatformVfrLib|$(CHIPSET_PKG)/UefiSetupUtilityDxe/PlatformVfrLib.inf
  DxePolicyUpdateLib|$(CHIPSET_PKG)/Library/DxePolicyUpdateLib/DxePolicyUpdateLib.inf
  PlatformCmosLib|$(CHIPSET_PKG)/Library/PlatformCmosLib/PlatformCmosLib.inf
#[-start-161202-IB11270171-add]#
  BaseForceRangeAccessLib|$(CHIPSET_PKG)/Library/BaseForceRangeAccess/BaseForceRangeAccess.inf 
#[-end-161202-IB11270171-add]#
  #
  # DXE phase common
  #
  UsbDebugPortLib|$(CHIPSET_PKG)/Library/UsbDebugPortLib/UsbDebugPortLibDxe.inf

  FlashWriteEnableLib|$(CHIPSET_PKG)/Library/FlashDeviceSupport/FlashWriteEnableLib/FlashWriteEnableLib.inf
  SpiAccessLib|$(CHIPSET_PKG)/Library/FlashDeviceSupport/SpiAccessLib/SpiAccessLib.inf
  SpiAccessInitLib|$(CHIPSET_PKG)/Library/FlashDeviceSupport/DxeSpiAccessInitLib/DxeSpiAccessInitLib.inf

!if ( ($(VP_BIOS_ENABLE) == YES) || ($(CSLE_ENABLE) == YES) )
  TimerLib|$(CHIPSET_PKG)/Library/IntelScAcpiPreSiTimerLib/IntelScAcpiPreSiTimerLib.inf
!else
  TimerLib|$(CHIPSET_PKG)/Library/IntelScAcpiTimerLib/IntelScAcpiTimerLib.inf
!endif
  EcLib|$(CHIPSET_PKG)/Library/BaseEcLib/BaseEcLib.inf
  EcHwLib|$(CHIPSET_PKG)/Library/BaseEcHwLib/BaseEcHwLib.inf
  EcMiscLib|$(CHIPSET_PKG)/Library/BaseEcMiscLib/BaseEcMiscLib.inf

  SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf

!if $(NPK_ENABLE) == YES
  TraceHubLib|$(CHIPSET_PKG)/Library/BaseTraceHubLib/BaseTraceHubLib.inf
  BaseTraceHubDebugLib|$(CHIPSET_PKG)/Library/BaseTraceHubDebugLib/BaseTraceHubDebugLib.inf
  PostCodeLib|InsydeModulePkg/Library/BasePostCodeLib/BasePostCodeLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/InsydeModulePkg/Library/BasePostCodeLib
  }
!else
  TraceHubLib|$(CHIPSET_PKG)/Library/BaseTraceHubLibNull/BaseTraceHubLibNull.inf
  BaseTraceHubDebugLib|$(CHIPSET_PKG)/Library/BaseTraceHubDebugLibNull/BaseTraceHubDebugLibNull.inf
!endif
#[-start-160107-IB08450333-add]#
!if gChipsetPkgTokenSpaceGuid.FtpmSupport
  PttHciLib|$(PLATFORM_RC_PACKAGE)/Txe/Library/PeiDxePttHciLib/PeiDxePttHciLib.inf
  PttPtpLib|$(PLATFORM_RC_PACKAGE)/Txe/Library/PeiDxePttPtpLib/PeiDxePttPtpLib.inf
!endif
  SetupUtilityLib|InsydeModulePkg/Library/SetupUtilityLib/SetupUtilityLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/InsydeModulePkg/Library/SetupUtilityLib
  }
#[-end-160107-IB08450333-add]#

#[-start-151229-IB03090424-add]#
#[-start-160317-IB03090425-modify]#
  SmbiosMemoryLib|$(PLATFORM_SI_PACKAGE)/NorthCluster/Library/Private/DxeSmbiosMemoryLib/DxeSmbiosMemoryLib.inf
#[-end-160317-IB03090425-modify]#
  WheaPlatformHooksLib|$(CHIPSET_PKG)/ApeiBertPkg/Library/WheaPlatformHooksLib/WheaPlatformHooksLib.inf
#[-end-151229-IB03090424-add]#

#[-start-160317-IB03090425-add]#
  MtrrLib|UefiCpuPkg/Library/MtrrLib/MtrrLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/UefiCpuPkg/Library/MtrrLib
  }
#[-end-160317-IB03090425-add]#
#[-start-160412-IB11270151-add]#
  !disable CapsuleLib|InsydeModulePkg/Library/DxeCapsuleLib/DxeCapsuleLib.inf
  CapsuleLib|InsydeModulePkg/Library/DxeCapsuleLib/DxeCapsuleLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/DxeCapsuleLib/
  }
#[-end-160412-IB11270151-add]#
#[-start-160510-IB03090427-add]#
  BpdtLib|$(PLATFORM_SI_PACKAGE)/Library/BpdtLib/BpdtLib.inf
  PmicLib|$(CHIPSET_PKG)/Library/PmicLib/PmicLibNull.inf
#[-end-160510-IB03090427-add]#
#[-start-161123-IB07250310-add]#
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  FspWrapperApiLib|$(CHIPSET_OVERRIDE)/IntelFsp2WrapperPkg/Library/BaseFspWrapperApiLib/BaseFspWrapperApiLib.inf
  FspWrapperApiTestLib|IntelFsp2WrapperPkg/Library/PeiFspWrapperApiTestLib/PeiFspWrapperApiTestLib.inf
  FspWrapperPlatformResetLib|$(CHIPSET_PKG)/FspSupport/Library/DxeFspWrapperPlatformResetLib/DxeFspWrapperPlatformResetLib.inf
!endif
#[-end-161123-IB07250310-add]#

#[-start-160802-IB03090430-add]#
  Heci2PowerManagementLib|$(PLATFORM_SI_PACKAGE)/Txe/Library/BaseHeci2PowerManagementNullLib/BaseHeci2PowerManagementNullLib.inf
#[-end-160802-IB03090430-add]#
#[-start-160826-IB03090433-add]#
  VariableStorageSelectorLib|$(CHIPSET_PKG)/Library/BaseVariableStorageSelectorLib/BaseVariableStorageSelectorLib.inf
#[-end-160826-IB03090433-add]#

  !disable KernelConfigLib|InsydeModulePkg/Library/DxeKernelConfigLib/DxeKernelConfigLib.inf
  KernelConfigLib|InsydeModulePkg/Library/DxeKernelConfigLib/DxeKernelConfigLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/DxeKernelConfigLib/
  }


[LibraryClasses.IA32]
  #
  # Usb_Device_Mode PEI LIB
  #
  UsbDeviceModePeiLib|$(CHIPSET_PKG)/Library/UsbDeviceModePeiLib/UsbDeviceModePeiLib.inf
  UdmWrapperPeiLib|$(CHIPSET_PKG)/Library/UdmWarpperPeiLib/UdmWrapperPeiLib.inf

  #
  # Library for EfiFastBoot PEI module
  #
  EfiFastBootPeiLib|$(CHIPSET_PKG)/Library/EfiFastBootPeiLib/EfiFastBootPeiLib.inf #Library

#[-start-160517-IB03090427-add]#
#[-start-160629-IB07400749-modify]#
!if $(TABLET_PF_BUILD) == YES  
  PmicLibPei|$(CHIPSET_PKG)/Library/PmicLibPei/PmicLibPei.inf
!else  
  PmicLibPei|$(CHIPSET_PKG)/Library/PmicLibPei/PmicLibPeiNull.inf
!endif
#[-end-160629-IB07400749-modify]#
#[-end-160517-IB03090427-add]#
#[-start-160803-IB07220122-add]#
  PeiVariableCacheLib|$(CHIPSET_PKG)/Library/PeiVariableCacheLib/PeiVariableCacheLib.inf
#[-end-160803-IB07220122-add]#


[LibraryClasses.common.SEC]
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  UsbDebugPortLib|$(CHIPSET_PKG)/Library/UsbDebugPortLib/UsbDebugPortLibPei.inf
  SetPcuUartLib|$(CHIPSET_PKG)/Library/SetPcuUartLib/SetPcuUartLib.inf
!if ( ($(VP_BIOS_ENABLE) == YES) || ($(CSLE_ENABLE) == YES) )
  TimerLib|$(CHIPSET_PKG)/Library/IntelScAcpiPreSiTimerLib/IntelScAcpiPreSiTimerLib.inf
!else
  TimerLib|$(CHIPSET_PKG)/Library/IntelScAcpiTimerLib/IntelScAcpiTimerLib.inf
!endif

[LibraryClasses.common.PEIM]
  !disable FlashWriteEnableLib|InsydeModulePkg/Library/FlashDeviceSupport/PeiFlashWriteEnableLib/PeiFlashWriteEnableLib.inf

!if gInsydeTokenSpaceGuid.PcdH2ODdtSupported AND $(H2O_DDT_DEBUG_IO) == Com
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
!else
  SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf
!endif

  KernelConfigLib|$(CHIPSET_PKG)/Library/PeiKernelConfigLib/PeiKernelConfigLib.inf
  UsbDebugPortLib|$(CHIPSET_PKG)/Library/UsbDebugPortLib/UsbDebugPortLibPei.inf
  MultiPlatformLib|$(CHIPSET_PKG)/Library/MultiPlatformLib/MultiPlatformLibBB.inf
  EcLib|$(CHIPSET_PKG)/Library/BaseEcLib/BaseEcLib.inf
  EcHwLib|$(CHIPSET_PKG)/Library/BaseEcHwLib/BaseEcHwLib.inf
  EcMiscLib|$(CHIPSET_PKG)/Library/BaseEcMiscLib/BaseEcMiscLib.inf

!if $(ENBDT_PF_BUILD) == YES
  PeiKscLib|$(CHIPSET_PKG)/Library/Ksc/Pei/PeiKscLib.inf
!endif
  PeiOemSvcChipsetLib|$(CHIPSET_PKG)/Library/PeiOemSvcChipsetLib/PeiOemSvcChipsetLib.inf
  PeiOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/PeiOemSvcChipsetLib/PeiOemSvcChipsetLibDefault.inf

#[-start-160111-IB08450334-modify]#
  TimerLib|PerformancePkg/Library/TscTimerLib/PeiTscTimerLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/PerformancePkg/Library/TscTimerLib/
  }
#[-end-160111-IB08450334-modify]#

  !disable FlashRegionLib|InsydeModulePkg/Library/FlashRegionLib/PeiFlashRegionLib/PeiFlashRegionLib.inf
  FlashRegionLib|InsydeModulePkg/Library/FlashRegionLib/PeiFlashRegionLib/PeiFlashRegionLib.inf{
  <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/FlashRegionLib/PeiFlashRegionLib/
  }
  PeiPlatformConfigUpdateLib|$(CHIPSET_PKG)/Library/PeiPlatformConfigUpdateLib/PeiPlatformConfigUpdateLib.inf
#[-start-160923-IB11200164-add]#
  !disable DebugPrintErrorLevelLib|InsydeModulePkg/Library/PeiDebugPrintErrorLevelLib/PeiDebugPrintErrorLevelLib.inf
  DebugPrintErrorLevelLib|InsydeModulePkg/Library/PeiDebugPrintErrorLevelLib/PeiDebugPrintErrorLevelLib.inf {
  <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/PeiDebugPrintErrorLevelLib/
  }
#[-end-160923-IB11200164-add]#
#[-start-161123-IB07250310-add]#
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  FspCommonLib|IntelFsp2Pkg/Library/BaseFspCommonLib/BaseFspCommonLib.inf
  FspSwitchStackLib|IntelFsp2Pkg/Library/BaseFspSwitchStackLib/BaseFspSwitchStackLib.inf
# FSP platform sample
  FspWrapperPlatformLib|$(CHIPSET_PKG)/FspSupport/Library/BaseFspPlatformInfoLibSample/BaseFspWrapperPlatformLibSample.inf
  FspPlatformSecLib|$(CHIPSET_PKG)/FspSupport/Library/SecFspWrapperPlatformSecLib/SecPeiFspPlatformSecLibSample.inf
  FspWrapperHobProcessLib|$(CHIPSET_PKG)/FspSupport/Library/PeiFspHobProcessLib/PeiFspHobProcessLib.inf
  FspPolicyInitLib|$(CHIPSET_PKG)/Library/PeiFspPolicyInitLib/PeiFspPolicyInitLib.inf
  FspWrapperPlatformResetLib|$(CHIPSET_PKG)/FspSupport/Library/PeiFspWrapperPlatformResetLib/PeiFspWrapperPlatformResetLib.inf
!endif
#[-end-161123-IB07250310-add]#
#[-start-170310-IB15550025-add]#
!disable PeiChipsetSvcLib|InsydeModulePkg/Library/PeiChipsetSvcLib/PeiChipsetSvcLib.inf
  PeiChipsetSvcLib|InsydeModulePkg/Library/PeiChipsetSvcLib/PeiChipsetSvcLib.inf {
  <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/PeiChipsetSvcLib/
  }
#[-end-170310-IB15550025-add]#

[LibraryClasses.common.PEI_CORE]
#[-start-170310-IB15550025-add]#
!disable PeiOemSvcKernelLibDefault|InsydeOemServicesPkg/Library/PeiOemSvcKernelLib/PeiOemSvcKernelLibDefault.inf
  PeiOemSvcKernelLibDefault|InsydeOemServicesPkg/Library/PeiOemSvcKernelLib/PeiOemSvcKernelLibDefault.inf {
  <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_OVERRIDE)/InsydeOemServicesPkg/Library/PeiOemSvcKernelLib/
  }
#[-end-170310-IB15550025-add]#
  UsbDebugPortLib|$(CHIPSET_PKG)/Library/UsbDebugPortLib/UsbDebugPortLibPei.inf
#[-start-160111-IB08450334-modify]#
  TimerLib|PerformancePkg/Library/TscTimerLib/PeiTscTimerLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/PerformancePkg/Library/TscTimerLib/
  }
#[-end-160111-IB08450334-modify]#

  !disable FlashRegionLib|InsydeModulePkg/Library/FlashRegionLib/PeiFlashRegionLib/PeiFlashRegionLib.inf
  FlashRegionLib|InsydeModulePkg/Library/FlashRegionLib/PeiFlashRegionLib/PeiFlashRegionLib.inf{
  <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/FlashRegionLib/PeiFlashRegionLib/
  }
#[-start-160923-IB11200164-add]# 
  !disable DebugPrintErrorLevelLib|InsydeModulePkg/Library/PeiDebugPrintErrorLevelLib/PeiDebugPrintErrorLevelLib.inf
  DebugPrintErrorLevelLib|InsydeModulePkg/Library/PeiDebugPrintErrorLevelLib/PeiDebugPrintErrorLevelLib.inf {
  <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/PeiDebugPrintErrorLevelLib/
  }
#[-end-160923-IB11200164-add]#
[LibraryClasses.common.DXE_CORE]
#[-start-160111-IB08450334-modify]#
  TimerLib|PerformancePkg/Library/TscTimerLib/DxeTscTimerLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/PerformancePkg/Library/TscTimerLib/
  }
#[-end-160111-IB08450334-modify]#

[LibraryClasses.common.DXE_SMM_DRIVER]
  SmmOemSvcChipsetLib|$(CHIPSET_PKG)/Library/SmmOemSvcChipsetLib/SmmOemSvcChipsetLib.inf
  SmmOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/SmmOemSvcChipsetLib/SmmOemSvcChipsetLibDefault.inf

  PerformanceLib|MdeModulePkg/Library/SmmPerformanceLib/SmmPerformanceLib.inf
  SpiAccessInitLib|$(CHIPSET_PKG)/Library/FlashDeviceSupport/SmmSpiAccessInitLib/SmmSpiAccessInitLib.inf

  StallSmmLib|$(CHIPSET_PKG)/Library/StallSmmLib/StallSmmLib.inf
  SmmKscLib|$(CHIPSET_PKG)/Library/Ksc/Smm/SmmKscLib.inf
  SmmCpuPlatformHookLib|$(CHIPSET_OVERRIDE)/IA32FamilyCpuPkg/Library/SmmCpuPlatformHookLibNull/SmmCpuPlatformHookLibNull.inf
#[-start-160111-IB08450334-add]#
  TimerLib|PerformancePkg/Library/TscTimerLib/DxeTscTimerLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/PerformancePkg/Library/TscTimerLib/
  }
#[-end-160111-IB08450334-add]#
#[-start-160808-IB07220123-add]#
  Heci2PowerManagementLib|$(PLATFORM_SI_PACKAGE)/Txe/Library/SmmHeci2PowerManagementLib/SmmHeci2PowerManagementLib.inf
#[-end-160808-IB07220123-add]#

  DxeInsydeChipsetLib|$(CHIPSET_PKG)/Library/DxeInsydeChipsetLib/DxeInsydeChipsetLib.inf
  SmmMemLib|MdePkg/Library/SmmMemLib/SmmMemLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/MdePkg/Override/Library/SmmMemLib
  }

[LibraryClasses.common.COMBINED_SMM_DXE]
  SmmOemSvcChipsetLib|$(CHIPSET_PKG)/Library/SmmOemSvcChipsetLib/SmmOemSvcChipsetLib.inf
  SmmOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/SmmOemSvcChipsetLib/SmmOemSvcChipsetLibDefault.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  DxeInsydeChipsetLib|$(CHIPSET_PKG)/Library/DxeInsydeChipsetLib/DxeInsydeChipsetLib.inf
  SmmMemLib|MdePkg/Library/SmmMemLib/SmmMemLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/MdePkg/Override/Library/SmmMemLib
  }

[LibraryClasses.common.SMM_CORE]
  SmmMemLib|MdePkg/Library/SmmMemLib/SmmMemLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/MdePkg/Override/Library/SmmMemLib
  }

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  DxeOemSvcChipsetLib|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLib.inf
  DxeOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLibDefault.inf
  DxeInsydeChipsetLib|$(CHIPSET_PKG)/Library/DxeInsydeChipsetLib/DxeInsydeChipsetLib.inf
#[-start-160111-IB08450334-modify]#
  TimerLib|PerformancePkg/Library/TscTimerLib/DxeTscTimerLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/PerformancePkg/Library/TscTimerLib/
  }
#[-end-160111-IB08450334-modify]#

#[-start-161123-IB07250310-add]#
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  !disable PcdLib|InsydeModulePkg/Library/SmmDxePcdLib/SmmDxePcdLib.inf
  PcdLib|InsydeModulePkg/Library/SmmDxePcdLib/SmmDxePcdLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/SmmDxePcdLib/
  }
!endif

[LibraryClasses.$(DXE_ARCH).DXE_SMM_DRIVER]
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  !disable PcdLib|InsydeModulePkg/Library/SmmDxePcdLib/SmmDxePcdLib.inf
  PcdLib|InsydeModulePkg/Library/SmmDxePcdLib/SmmDxePcdLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/SmmDxePcdLib/
  }
!endif

[LibraryClasses.$(DXE_ARCH).COMBINED_SMM_DXE]
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  !disable PcdLib|InsydeModulePkg/Library/SmmDxePcdLib/SmmDxePcdLib.inf
  PcdLib|InsydeModulePkg/Library/SmmDxePcdLib/SmmDxePcdLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/SmmDxePcdLib/
  }
!endif
#[-end-161123-IB07250310-add]#

[LibraryClasses.common.DXE_DRIVER]
!if $(ENBDT_PF_BUILD) == YES
  DxeKscLib|$(CHIPSET_PKG)/Library/Ksc/Dxe/DxeKscLib.inf
!endif

  SpiAccessInitLib|$(CHIPSET_PKG)/Library/FlashDeviceSupport/DxeSpiAccessInitLib/DxeSpiAccessInitLib.inf

  DxeOemSvcChipsetLib|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLib.inf
  DxeOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLibDefault.inf
  DxeInsydeChipsetLib|$(CHIPSET_PKG)/Library/DxeInsydeChipsetLib/DxeInsydeChipsetLib.inf

#[-start-160111-IB08450334-modify]#
  TimerLib|PerformancePkg/Library/TscTimerLib/DxeTscTimerLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/PerformancePkg/Library/TscTimerLib/
  }
#[-end-160111-IB08450334-modify]#

[LibraryClasses.common.UEFI_DRIVER]
  DxeOemSvcChipsetLib|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLib.inf
  DxeOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLibDefault.inf
  DxeInsydeChipsetLib|$(CHIPSET_PKG)/Library/DxeInsydeChipsetLib/DxeInsydeChipsetLib.inf
#[-start-160111-IB08450334-modify]#
  TimerLib|PerformancePkg/Library/TscTimerLib/DxeTscTimerLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/PerformancePkg/Library/TscTimerLib/
  }
#[-end-160111-IB08450334-modify]#

[LibraryClasses.common.UEFI_APPLICATION]

[PcdsFixedAtBuild]
  ## Stack size in the temporary RAM.
  #   0 means half of PcdTemporaryRamSizeWhenRecovery.
  #
  #  Refer gEfiCpuTokenSpaceGuid.PcdPeiTemporaryRamStackSize
  #
  gInsydeTokenSpaceGuid.PcdPeiTemporaryRamStackSizeWhenRecovery|0x3C00

  gEfiMdeModulePkgTokenSpaceGuid.PcdShadowPeimOnS3Boot|TRUE

[PcdsDynamicExDefault]
  #
  # Port number mapping table Define
  #
  gInsydeTokenSpaceGuid.PcdPortNumberMapTable|{ \
    0x00, 0x12, 0x00, 0, 0, 0x00, \
    0x00, 0x12, 0x00, 0, 1, 0x02, \
    0x00, 0x12, 0x00, 1, 0, 0x01, \
    0x00, 0x12, 0x00, 1, 1, 0x03, \
    0x00, 0x00, 0x00, 0, 0, 0x00} #EndEntry
#[-start-160714-IB07400759-remove]#
#  # PIRQ link value                                PIRQA, PIRQB, PIRQC, PIRQD, PIRQE, PIRQF, PIRQG, PIRQH (ILB_BASE_ADDRESS + x)
#  gChipsetPkgTokenSpaceGuid.PcdPirqLinkValueArray|{0x08,  0x09,  0x0A,  0x0B,  0x0C,  0x0D,  0x0E,  0x0F} #EndEntry
#
#  gChipsetPkgTokenSpaceGuid.PcdVirtualBusTable|{  0x00, 0x1c, 0x00, 0x04, \
#                                                  0x00, 0x1c, 0x01, 0x05, \
#                                                  0x00, 0x1c, 0x02, 0x06, \
#                                                  0x00, 0x1c, 0x03, 0x07} #EndEntry
#  # Dev 0x02 => 0x10, Mobile IGFX,        PIRQA, PIRQN, PIRQN, PIRQN
#  # Dev 0x03 => 0x18, IUNIT               PIRQB, PIRQN, PIRQN, PIRQN
#  # Dev 0x0A => 0x50, ISH                 PIRQE, PIRQN, PIRQN, PIRQN
#  # Dev 0x0B => 0x58, PUNIT               PIRQF, PIRQN, PIRQN, PIRQN
#  # Dev 0x0C => 0x60, GMM                 PIRQG, PIRQN, PIRQN, PIRQN
#  # Dev 0x0D => 0x60, UFS                 PIRQH, PIRQN, PIRQN, PIRQN
#  # Dev 0x10 => 0x80, SD Host#0-eMMC,     PIRQA, PIRQN, PIRQN, PIRQN
#  # Dev 0x11 => 0x88, SD Host#1-SDIO,     PIRQB, PIRQN, PIRQN, PIRQN
#  # Dev 0x12 => 0x90, SD Host#2-SD Card,  PIRQC, PIRQN, PIRQN, PIRQN
#  # Dev 0x13 => 0x98, SATA Controller,    PIRQD, PIRQN, PIRQN, PIRQN
#  # Dev 0x14 => 0xA0, xHCI Host,          PIRQE, PIRQN, PIRQN, PIRQN
#  # Dev 0x15 => 0xA8, LPE Audio,          PIRQF, PIRQN, PIRQN, PIRQN
#  # Dev 0x16 => 0xB0, USB OTG,            PIRQG, PIRQN, PIRQN, PIRQN
#  # Dev 0x18 => 0xC0, LPSS2 I2C,          PIRQB, PIRQA, PIRQD, PIRQC
#  # Dev 0x1A => 0xD0, SeC,                PIRQF, PIRQN, PIRQN, PIRQN
#  # Dev 0x1B => 0xD8, HD Audio,           PIRQG, PIRQN, PIRQN, PIRQN
#  # Dev 0x1C => 0xE0, PCIE Root Port,     PIRQA, PIRQB, PIRQC, PIRQD
#  # Dev 0x1E => 0xF0, LPSS,               PIRQD, PIRQB, PIRQC, PIRQA
#  # Dev 0x1F => 0xF8, SMBus Controller,   PIRQN, PIRQC, PIRQN, PIRQN
#  #
#  # Bus, Dev,  INT#A,IrqMask,   INT#B,IrqMask,   INT#C,IrqMask,   INT#D,IrqMask,   Slot, Reserved, DevIpRegValue,ProgrammableIrq.
#  #
#  # ProgrammableIrq
#  # This byte is provided for H2OIDE to identify is this device is a programmable device
#  # 0x00: Enable,
#  # 0xFF: Disable,
#  # 0x01~0xFE: Reserved
#  #
#  gChipsetPkgTokenSpaceGuid.PcdControllerDeviceIrqRoutingEntry| { \
#  0x00, 0x10, 0x08, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0x18, 0x09, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0x50, 0x0C, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0x58, 0x0D, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0x60, 0x0E, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0x68, 0x0F, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0x80, 0x08, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0x88, 0x09, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0x90, 0x0A, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0x98, 0x0B, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0xA0, 0x0C, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0xA8, 0x0D, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0xB0, 0x0F, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0xC0, 0x09, UINT16(0xDEB8), 0x08, UINT16(0xDEB8), 0x0B, UINT16(0xDEB8), 0x0A, UINT16(0xDEB8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0xD0, 0x0D, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0xD8, 0x0E, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0xE0, 0x08, UINT16(0xDEB8), 0x09, UINT16(0xDEB8), 0x0A, UINT16(0xDEB8), 0x0B, UINT16(0xDEB8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0xF0, 0x0B, UINT16(0xDEB8), 0x09, UINT16(0xDEB8), 0x0A, UINT16(0xDEB8), 0x08, UINT16(0xDEB8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x00, 0xF8, 0x00, UINT16(0xDEF8), 0x0A, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, \
#  0x04, 0x00, 0x08, UINT16(0xDEB8), 0x09, UINT16(0xDEB8), 0x0A, UINT16(0xDEB8), 0x0B, UINT16(0xDEB8), 0x01, 0xFF, UINT32(0xFFFFFFFF), 0x00, \
#  0x05, 0x00, 0x09, UINT16(0xDEB8), 0x0A, UINT16(0xDEB8), 0x0B, UINT16(0xDEB8), 0x08, UINT16(0xDEB8), 0x02, 0xFF, UINT32(0xFFFFFFFF), 0x00, \
#  0x06, 0x00, 0x0A, UINT16(0xDEB8), 0x0B, UINT16(0xDEB8), 0x08, UINT16(0xDEB8), 0x09, UINT16(0xDEB8), 0x03, 0xFF, UINT32(0xFFFFFFFF), 0x00, \
#  0x07, 0x00, 0x0B, UINT16(0xDEB8), 0x08, UINT16(0xDEB8), 0x09, UINT16(0xDEB8), 0x0A, UINT16(0xDEB8), 0x04, 0xFF, UINT32(0xFFFFFFFF), 0x00} #EndEntry
#
#  gChipsetPkgTokenSpaceGuid.PcdIrqPoolTable|{07, 0x00, \ #IRQ7
#                                             10, 0x00, \ #IRQ10
#                                             11, 0x00, \ #IRQ11
#                                             11, 0x00, \ #IRQ11
#                                             07, 0x00, \ #IRQ07
#                                             10, 0x00, \ #IRQ10
#                                             11, 0x00}   #IRQ11
#  gChipsetPkgTokenSpaceGuid.PcdPirqPriorityTable|{7,  \# PIRQ A
#                                                  0,  \# PIRQ B
#                                                  0,  \# PIRQ C
#                                                  10, \# PIRQ D
#                                                  0,  \# PIRQ E
#                                                  0,  \# PIRQ F
#                                                  0,  \# PIRQ G
#                                                  0}  #EndEntry
#[-end-160714-IB07400759-remove]#
#[-start-151124-IB07220020-remove]#
#   #
#   # Add Winbond W25Q64DW Spi config
#   #
#   gH2OFlashDeviceMfrNameGuid.PcdWinbondW25q64dwSpiMfrName|"WINBOND"
#   gH2OFlashDevicePartNameGuid.PcdWinbondW25q64dwSpiPartName|"W25Q64DW"
#   gH2OFlashDeviceGuid.PcdWinbondW25q64dwSpi|{ \ # Winbond  W25Q64DW
#     0x03, 0x00, 0x00, 0x00, 0xef, 0x60, 0x17, 0x00,  \  # DeviceType = 03  Id = 001760EF
#     0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x08   \  # ExtId = 00000000  BlockSize = 0010  BlockCount = 0800
#   }
# 
#   gH2OFlashDeviceConfigGuid.PcdWinbondW25q64dwSpiConfig|{ \ # Winbond  W25Q64DW
#     0x28, 0x00, 0x00, 0x00, 0x9f, 0x20, 0x02, 0x01,  \  # Size = 00000028  ReadIdOp = 9F  EraseOp = 20  WriteOp = 02  WriteStatusOp = 01
#     0x03, 0x05, 0x00, 0x00, 0x7c, 0xf2, 0x06, 0x50,  \  # ReadOp = 03  ReadStatusOp = 05  OpType = F27C  WriteEnablePrefix = 06  WriteStatusEnablePrefix = 50
#     0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,  \  # GlobalProtectAvailable = 00  BLockProtectAvailable = 00  BlockProtectCodeRequired = 00  MultiByteProgramAvailable = 01  BytesOfId = 03  MinBytePerProgRead = 01  NoVolStatusAvailable = 01
#     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,  \  # GlobalProtectCode = 00  GlobalUnprotectCode = 00  BlockProtectCode = 00  BlockUnprotectCode = 00  DeviceSize = 00800000
#     0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   \  # BlockEraseSize = 00001000  BlockProtectSize = 00000000
#   }
#[-end-151124-IB07220020-remove]#

  #
  # Winbond W25Q16DW Spi config
  #
  gH2OFlashDeviceMfrNameGuid.PcdWinbondW25q16dwSpiMfrName|"WINBOND"
  gH2OFlashDevicePartNameGuid.PcdWinbondW25q16dwSpiPartName|"W25Q16DW"
  gH2OFlashDeviceGuid.PcdWinbondW25q16dwSpi|{ \ # Winbond  W25Q16DW
    0x03, 0x00, 0x00, 0x00, 0xef, 0x60, 0x15, 0x00,  \  # DeviceType = 03  Id = 001560EF
    0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x02   \  # ExtId = 00000000  BlockSize = 0010  BlockCount = 0200
  }
  gH2OFlashDeviceConfigGuid.PcdWinbondW25q16dwSpiConfig|{ \ # Winbond  W25Q16DW
    0x28, 0x00, 0x00, 0x00, 0x9f, 0x20, 0x02, 0x01,  \  # Size = 00000028  ReadIdOp = 9F  EraseOp = 20  WriteOp = 02  WriteStatusOp = 01
    0x03, 0x05, 0x00, 0x00, 0x7c, 0xf2, 0x06, 0x50,  \  # ReadOp = 03  ReadStatusOp = 05  OpType = F27C  WriteEnablePrefix = 06  WriteStatusEnablePrefix = 50
    0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,  \  # GlobalProtectAvailable = 00  BLockProtectAvailable = 00  BlockProtectCodeRequired = 00  MultiByteProgramAvailable = 01  BytesOfId = 03  MinBytePerProgRead = 01  NoVolStatusAvailable = 01
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00,  \  # GlobalProtectCode = 00  GlobalUnprotectCode = 00  BlockProtectCode = 00  BlockUnprotectCode = 00  DeviceSize = 00800000
    0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   \  # BlockEraseSize = 00001000  BlockProtectSize = 00000000
  }

#[-start-151124-IB08450330-add]#
#[-start-161118-IB07400816-remove]#
#  #
#  # Winbond W25Q128FW Spi config
#  #
#  gH2OFlashDeviceMfrNameGuid.PcdWinbondW25q128fwSpiMfrName|"WINBOND"
#  gH2OFlashDevicePartNameGuid.PcdWinbondW25q128fwSpiPartName|"W25Q128FW"
#  gH2OFlashDeviceGuid.PcdWinbondW25q128fwSpi|{ \ # WINBOND  W25Q128BV
#    0x03, 0x00, 0x00, 0x00, 0xef, 0x60, 0x18, 0x00,  \  # DeviceType = 03  Id = 001860EF
#    0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10   \  # ExtId = 00000000  BlockSize = 0010  Multiple = 1000
#  }
#
#  gH2OFlashDeviceConfigGuid.PcdWinbondW25q128fwSpiConfig|{ \ # WINBOND  W25Q128FW
#    0x28, 0x00, 0x00, 0x00, 0x9f, 0x20, 0x02, 0x01,  \  # Size = 00000028  ReadIdOp = 9F  EraseOp = 20  WriteOp = 02  WriteStatusOp = 01
#    0x03, 0x05, 0x00, 0x00, 0x7c, 0xf2, 0x06, 0x06,  \  # ReadOp = 03  ReadStatusOp = 05  OpType = F27C  WriteEnablePrefix = 06  WriteStatusEnablePrefix = 06
#    0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,  \  # GlobalProtectAvailable = 00  BLockProtectAvailable = 00  BlockProtectCodeRequired = 00  MultiByteProgramAvailable = 01  BytesOfId = 03  MinBytePerProgRead = 01  NoVolStatusAvailable = 01
#    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,  \  # GlobalProtectCode = 00  GlobalUnprotectCode = 00  BlockProtectCode = 00  BlockUnprotectCode = 00  DeviceSize = 01000000
#    0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   \  # BlockEraseSize = 00001000  BlockProtectSize = 00000000
#  }
#[-end-161118-IB07400816-remove]#
#[-end-151124-IB08450330-add]#

#[-start-160112-IB07220031-remove]#
#   #
#   #
#   # Add MXIC 25U6435F Spi config
#   #
#   gH2OFlashDeviceGuid.PcdMxic25u6435fSpi|{ \ # Mxic MX25u6435F
#     0x03, 0x00, 0x00, 0x00, 0xC2, 0x25, 0x37, 0x00,  \  # DeviceType = 03  Id = 0x003725C2
#     0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x08   \  # ExtId = 00000000  BlockSize = 0010  BlockCount = 0800
#   }
# 
#   gH2OFlashDeviceConfigGuid.PcdMxic25u6435fSpiConfig|{ \ # Mxic MX25u6435F
#     0x28, 0x00, 0x00, 0x00, 0x9f, 0x20, 0x02, 0x01,  \  # Size = 00000028  ReadIdOp = 9F  EraseOp = 20  WriteOp = 02  WriteStatusOp = 01
#     0x03, 0x05, 0x00, 0x00, 0x7c, 0xf2, 0x06, 0x50,  \  # ReadOp = 03  ReadStatusOp = 05  OpType = F27C  WriteEnablePrefix = 06  WriteStatusEnablePrefix = 50
#     0x01, 0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x00,  \  # GlobalProtectAvailable = 01  BlockProtectAvailable = 00  BlockProtectCodeRequired = 00  MultiByteProgramAvailable = 01  BytesOfId = 03  MinBytePerProgRead = 01  NoVolStatusAvailable = 00
#     0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,  \  # GlobalProtectCode = 1C  GlobalUnprotectCode = 00  BlockProtectCode = 00  BlockUnprotectCode = 00  DeviceSize = 00200000
#     0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   \  # BlockEraseSize = 00001000  BlockProtectSize = 00000000
#   }
#[-end-160112-IB07220031-remove]#

  gH2OBdsDefaultBootListGenericOsTokenSpaceGuid.AndroidLoader|L"\\Loader.efi\tAndroid"
#[-start-151124-IB07220021-remove]#
#   gChipsetPkgTokenSpaceGuid.PcdH2OConsoleRedirectionClassGuid|{GUID("CB63151A-D635-49f6-8DE9-CDC992FF7477")}
#[-end-151124-IB07220021-remove]#
#[-start-160114-IB08450336-remove]#
#  gH2OBdsDefaultBootListGenericOsTokenSpaceGuid.MicrosoftOSfromeMMC|L"\\EFI\\Boot\\bootx64.efi\tWindows Boot Manager (eMMC)"
#[-end-160114-IB08450336-remove]#

[PcdsFixedAtBuild]
  gInsydeTokenSpaceGuid.Pcd8254Counter1OperatingMode|2
  gInsydeTokenSpaceGuid.PcdTemporaryRamSizeWhenRecovery|0x10000
  gInsydeTokenSpaceGuid.PcdDefaultSsidSvid|0x80867270
#[-start-160121-IB06740461-modify]#
  gInsydeTokenSpaceGuid.PcdI2cControllerTable|{  \ # The definition of I2C host controller number lookup table
    UINT64(0x0000000000001600),                  \ # Number 0 I2C controller is located on dev:0x16 fun:0x00
    UINT64(0x0000000000001601),                  \ # Number 1 I2C controller is located on dev:0x16 fun:0x01
    UINT64(0x0000000000001602),                  \ # Number 2 I2C controller is located on dev:0x16 fun:0x02
    UINT64(0x0000000000001603),                  \ # Number 3 I2C controller is located on dev:0x16 fun:0x03
    UINT64(0x0000000000001700),                  \ # Number 4 I2C controller is located on dev:0x17 fun:0x00
    UINT64(0x0000000000001701),                  \ # Number 5 I2C controller is located on dev:0x17 fun:0x01
    UINT64(0x0000000000001702),                  \ # Number 6 I2C controller is located on dev:0x17 fun:0x02
    UINT64(0x0000000000001703),                  \ # Number 7 I2C controller is located on dev:0x17 fun:0x03
    UINT64(0x0000000000000000)                   \ # End of table
  }
#[-end-160121-IB06740461-modify]#

  gInsydeTokenSpaceGuid.PcdI2cBusSpeedTable|{                                 \  # The definition of I2C bus configuration lookup table
    UINT16(100000),                                                           \  # Number 0 stands for 100 Khz
    UINT16(400000),                                                           \  # Number 0 stands for 400 khz
    UINT16(1000000),                                                          \  # Number 0 stands for 1  Mhz
    UINT64(00)                                                                \  # End of table
  }

#[-start-160801-IB03090430-modify]#
  gEfiBxtTokenSpaceGuid.PcdPmcIpc1BaseAddress0|0xFE042000
#[-end-160801-IB03090430-modify]#
  !if $(VP_BIOS_ENABLE) == YES
    #Override default for Simics
    gEfiBxtTokenSpaceGuid.PcdPlatformIdRegisterOffset|0x2C
    gEfiBxtTokenSpaceGuid.PcdPmcSsramBaseAddress0|0xFF03A000
  !endif
################################################################################
#
# Platform related components
#
################################################################################
#
# To Enable EFI_DEBUG on indivial module, add following override.
#
#  XXX.inf {
#    <LibraryClasses>
#      SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf
#      DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
#      DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
#      UefiBootServicesTableLib|$(CHIPSET_OVERRIDE2)/MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
#      UefiRuntimeServicesTableLib|$(CHIPSET_OVERRIDE2)/MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
#    <PcdsFixedAtBuild>
#      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
#    <PcdsPatchableInModule>
#      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000042
#    <BuildOptions>
#      MSFT:*_*_*_CC_FLAGS             = /UMDEPKG_NDEBUG
#      GCC:*_*_*_CC_FLAGS             = -UMDEPKG_NDEBUG
#  }
#
#
#[-start-161123-IB07250310-add]#
[Components]
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf {
    <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_PKG)/Override/MdePkg/Library/BasePcdLibNull
  }
  MdePkg/Library/PeiPcdLib/PeiPcdLib.inf {
    <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_PKG)/Override/MdePkg/Library/PeiPcdLib
  }
  MdePkg/Library/DxePcdLib/DxePcdLib.inf {
    <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_PKG)/Override/MdePkg/Library/DxePcdLib
  }
!endif
#[-end-161123-IB07250310-add]#
  MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/MdePkg/Library/BaseUefiDecompressLib
  }

[Components.$(PEI_ARCH)]
#[-start-1703017-IB15550025-add]#
  !disable InsydeModulePkg/Core/DxeIplPeim/DxeIpl.inf
  InsydeModulePkg/Core/DxeIplPeim/DxeIpl.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/InsydeModulePkg/Core/DxeIplPeim/
    <LibraryClasses>
      NULL|IntelFrameworkModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  } 
#[-end-1703017-IB15550025-add]#

  !disable InsydeModulePkg/Universal/Recovery/FatPei/FatPei.inf
  InsydeModulePkg/Universal/Recovery/FatPei/FatPei.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/InsydeModulePkg/Universal/Recovery/FatPei
  }

  $(CHIPSET_PKG)/VariableStorageSelector/Pei/VariableStorageSelectorPei.inf
  !disable InsydeModulePkg/Universal/Variable/VariablePei/VariablePei.inf
  InsydeModulePkg/Universal/Variable/VariablePei/VariablePei.inf {
   <SOURCE_OVERRIDE_PATH>
     $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/Variable/VariablePei
   <LibraryClasses>
     SetupDataProviderLibPei|$(CHIPSET_PKG)/Library/SetupDataProviderLibPei/SetupDataProviderPei.inf
#[-start-160803-IB07220122-add]#
     PlatformConfigDataLib|$(CHIPSET_PKG)/Library/PlatformConfigDataLib/PlatformConfigDataLib.inf
#[-end-160803-IB07220122-add]#
#[-start-160216-IB03090424-add]#
#[-start-170511-IB07400866-add]#
!if $(INSYDE_DEBUGGER) == YES AND $(EFI_DEBUG) == YES
     DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
!endif
#[-end-170511-IB07400866-add]#
   <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2E
#[-end-160216-IB03090424-add]#
  }

  #
  #  @{ Related Chipset  Start
  #
#  !disable IA32FamilyCpuPkg/SecCore/SecCore.inf
#  $(CHIPSET_OVERRIDE)/IA32FamilyCpuPkg/SecCore/SecCore.inf {
#    <PcdsFixedAtBuild>
#      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x0
#      gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x0
#  }
  $(CHIPSET_OVERRIDE)/IA32FamilyCpuPkg/SecCore/Vtf0SecCore.inf {
    <LibraryClasses>
      DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
      PlatformSecLib|$(CHIPSET_PKG)/Library/PlatformSecLib/Vtf0PlatformSecLib.inf
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x00
      gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x0
  }

  $(CHIPSET_PKG)/ChipsetSvcPei/ChipsetSvcPeiBB.inf{
    <LibraryClasses>
#[-start-160526-IB06720411-remove]#
#      MultiPlatformLib|$(CHIPSET_PKG)/Library/MultiPlatformLib/MultiPlatformLibBB.inf
#[-end-160526-IB06720411-remove]#
!if $(ENBDT_PF_BUILD) == YES
#[-start-161206-IB07400822-modify]#
!if gChipsetPkgTokenSpaceGuid.PcdKscSupport == 1
      PeiKscLib|$(CHIPSET_PKG)/Library/Ksc/Pei/PeiKscLib.inf
!else
      PeiKscLib|$(CHIPSET_PKG)/Library/Ksc/Pei/PeiKscLibNull.inf
!endif
#[-end-161206-IB07400822-modify]#
!endif
  }

  $(CHIPSET_PKG)/ChipsetSvcPei/ChipsetSvcPei.inf {
    <LibraryClasses>
#[-start-160526-IB06720411-remove]#
#      MultiPlatformLib|$(CHIPSET_PKG)/Library/MultiPlatformLib/MultiPlatformLibBB.inf
#[-end-160526-IB06720411-remove]#
!if $(ENBDT_PF_BUILD) == YES
#[-start-161206-IB07400822-modify]#
!if gChipsetPkgTokenSpaceGuid.PcdKscSupport == 1
      PeiKscLib|$(CHIPSET_PKG)/Library/Ksc/Pei/PeiKscLib.inf
!else
      PeiKscLib|$(CHIPSET_PKG)/Library/Ksc/Pei/PeiKscLibNull.inf
!endif
#[-end-161206-IB07400822-modify]#
!endif
  }

#  $(CHIPSET_PKG)/PolicyInit/Pei/PolicyInitPei.inf {
#    <LibraryClasses>
#      CpuPolicyLib|$(PLATFORM_RC_PACKAGE)/Cpu/Library/PeiCpuPolicyLib/PeiCpuPolicyLib.inf
#  }

#[-start-161123-IB07250310-modify]#
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  $(CHIPSET_PKG)/PlatformInit/Pei/PlatformInitPreMemWrapper.inf {
    <LibraryClasses>
      CpuPolicyLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiCpuPolicyLibPreMem/PeiCpuPolicyLibPreMem.inf
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
  }
  $(CHIPSET_PKG)/Override/IntelFsp2WrapperPkg/FspmWrapperPeim/FspmWrapperPeim.inf {
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
      # this is just the FSP-M s base, need cleanup soon
      gIntelFsp2WrapperTokenSpaceGuid.PcdFspmBaseAddress|0xFEF7B000
      gIntelFsp2WrapperTokenSpaceGuid.PcdFlashFvFspBase|0xFEF7B000
      gIntelFsp2WrapperTokenSpaceGuid.PcdFlashFvFspSize|0x59000    
      gIntelFsp2WrapperTokenSpaceGuid.PcdTemporaryRamBase|0xFEF00000
      gIntelFsp2WrapperTokenSpaceGuid.PcdFspStackBase|0xFEF22000
      gIntelFsp2WrapperTokenSpaceGuid.PcdFspStackSize|0x2A000
      gIntelFsp2PkgTokenSpaceGuid.PcdGlobalDataPointerAddress|0xFED00148
  }
  $(CHIPSET_PKG)/Override/IntelFsp2WrapperPkg/FspsWrapperPeim/FspsWrapperPeim.inf {
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
      gIntelFsp2WrapperTokenSpaceGuid.PcdTemporaryRamBase|0xFEF00000
      gIntelFsp2WrapperTokenSpaceGuid.PcdGlobalDataPointerAddress|0xFED00148
  }
!else
  $(CHIPSET_PKG)/PlatformInit/Pei/PlatformInitPreMem.inf {
#[-start-160216-IB03090424-modify]#
    <LibraryClasses>
      CpuPolicyLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiCpuPolicyLibPreMem/PeiCpuPolicyLibPreMem.inf
      PlatformSecLib|$(CHIPSET_PKG)/Library/PlatformSecLib/Vtf0PlatformSecLib.inf
#[-start-170222-IB07400843-add]#
#[-start-170801-IB07400897-modify]#
!if $(MRC_EV_RMT_BUILD) == YES OR $(MRC_DEBUG_DISABLE) == NO
      DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
!endif
#[-end-170801-IB07400897-modify]#
#[-end-170222-IB07400843-add]#
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2E
#[-end-160216-IB03090424-modify]#
  }
!endif
#[-end-161123-IB07250310-modify]#
  $(CHIPSET_PKG)/PlatformInit/Pei/PlatformInit.inf
  #
  #  @{ Related Chipset end
  #
  !disable MdeModulePkg/Universal/CapsulePei/CapsulePei.inf
  !disable InsydeModulePkg/Universal/StatusCode/DebugMaskPei/DebugMaskPei.inf
  !disable InsydeModulePkg/Universal/Variable/SetupDataProviderPei/SetupDataProviderPei.inf
  !disable InsydeModulePkg/Bus/Smbus/ProgClkGenPeim/ProgClkGenPeim.inf
  !disable InsydeModulePkg/Universal/CommonPolicy/PlatformStage1Pei/PlatformStage1Pei.inf
#  $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/CommonPolicy/PlatformStage1Pei/PlatformStage1Pei.inf

  !disable InsydeModulePkg/Universal/CommonPolicy/PlatformStage2Pei/PlatformStage2Pei.inf
  InsydeModulePkg/Universal/CommonPolicy/PlatformStage2Pei/PlatformStage2Pei.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/CommonPolicy/PlatformStage2Pei/
    <PcdsPatchableInModule>
#[-start-160614-IB07400744-modify]#
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000040
#[-end-160614-IB07400744-modify]#
  }
!if $(ENBDT_PF_BUILD) == YES
  $(CHIPSET_PKG)/PchAhciPei/PchAhciPei.inf

#  $(CHIPSET_PKG)/PlatformAzaliaPolicy/AzaliaPolicyPei.inf
#  $(CHIPSET_PKG)/CommonChipset/InstallVerbTablePei/InstallVerbTablePei.inf
#  $(CHIPSET_PKG)/AzaliaControllerPei/AzaliaInitPei.inf
!endif

!if gChipsetPkgTokenSpaceGuid.PcdSwitchableGraphicsSupported
  # Switchable Graphics / Hybrid Graphics Support
  $(CHIPSET_PKG)/SwitchableGraphicsPei/SwitchableGraphicsPei.inf
!endif



#[-start-180504-IB07400964-remove]#
#!if gChipsetPkgTokenSpaceGuid.SecureBootStage1Check == 1 AND gInsydeTokenSpaceGuid.PcdCrisisRecoverySupported == 1
#  $(CHIPSET_PKG)/TxeRecoveryFvLoader/TxeRecoveryFvLoader.inf
#!endif
#[-end-180504-IB07400964-remove]#



!if gChipsetPkgTokenSpaceGuid.PcdSpi2mSupport
#[-start-160905-IB07400778-remove]#
#  $(CHIPSET_PKG)/MmcBiosLoaderPei/MmcBiosLoaderPei.inf
#[-end-160905-IB07400778-remove]#

!if gInsydeTokenSpaceGuid.PcdCrisisRecoverySupported
  $(CHIPSET_PKG)/UsbDeviceModePei/UsbDeviceModePei.inf
  $(CHIPSET_PKG)/EfiFastBootPei/EfiFastBootPei.inf
!endif
!endif

#[-start-151229-IB06740454-modify]#
#[-start-151216-IB07220025-remove]#
!disable InsydeModulePkg/Universal/Recovery/CrisisRecoveryPei/CrisisRecoveryPei.inf
#[-start-161123-IB07250310-add]#
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
!disable InsydeModulePkg/Universal/Recovery/EmuSecPei/EmuSecPei.inf
!disable InsydeModulePkg/Universal/Recovery/EmuPeiHelperPei/EmuPeiHelperPei.inf
!endif
#[-end-161123-IB07250310-add]#
#[-start-161123-IB07250310-modify]#
!if gInsydeTokenSpaceGuid.PcdCrisisRecoverySupported
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
InsydeModulePkg/Universal/Recovery/CrisisRecoveryPei/CrisisRecoveryPei.inf {
  <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_PKG)/Override/InsydeModulePkg/Universal/Recovery/CrisisRecoveryPei
}
!else
!if gInsydeTokenSpaceGuid.PcdUseFastCrisisRecovery
!else
InsydeModulePkg/Universal/Recovery/CrisisRecoveryPei/CrisisRecoveryPei.inf {
  <SOURCE_OVERRIDE_PATH>
    $(CHIPSET_PKG)/Override/InsydeModulePkg/Universal/Recovery/CrisisRecoveryPei
}
!endif
!endif
#[-end-161123-IB07250310-modify]#
!endif
#[-end-151216-IB07220025-remove]#
#[-end-151229-IB06740454-modify]#

!if gInsydeTokenSpaceGuid.PcdCrisisRecoverySupported
  $(CHIPSET_PKG)/PchUfsPei/PchUfsPei.inf
!endif

#[-start-151216-IB07220025-add]#
!if gInsydeTokenSpaceGuid.PcdH2OUsbPeiSupported
  $(CHIPSET_PKG)/UsbPei/UsbPei.inf
!endif
#[-end-151216-IB07220025-add]#

#[-start-180504-IB07400964-remove]#
#  $(CHIPSET_PKG)/GenTxeConfigData/GenTxeConfigData.inf
#[-end-180504-IB07400964-remove]#

#
# Override core code for cache-based peims
#
  !disable MdeModulePkg/Core/Pei/PeiMain.inf
  MdeModulePkg/Core/Pei/PeiMain.inf {
    <BuildOptions>
      *_*_*_CC_FLAGS = -D MDEPKG_NDEBUG
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/MdeModulePkg/Core/Pei
#[-start-160216-IB03090424-add]#
    <PcdsFixedAtBuild>
        gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2E
    <PcdsPatchableInModule>
        gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000046
#[-end-160216-IB03090424-add]#
  }

  !disable MdeModulePkg/Universal/PCD/Pei/Pcd.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
    <SOURCE_OVERRIDE_PATH>
      MdeModulePkg/Override/Universal/PCD/Pei
      $(CHIPSET_PKG)/Override/MdeModulePkg/Universal/PCD/Pei
#[-start-160216-IB03090424-add]#
    <PcdsPatchableInModule>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000046
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2E
#[-end-160216-IB03090424-add]#
  }

  !disable MdeModulePkg/Universal/ReportStatusCodeRouter/Pei/ReportStatusCodeRouterPei.inf
  MdeModulePkg/Universal/ReportStatusCodeRouter/Pei/ReportStatusCodeRouterPei.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/MdeModulePkg/Universal/ReportStatusCodeRouter/Pei
#[-start-160216-IB03090424-add]#
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2E
#[-end-160216-IB03090424-add]#
  }


!if gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial == 1 || gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseMemory == 1
  !disable MdeModulePkg/Universal/StatusCodeHandler/Pei/StatusCodeHandlerPei.inf
  MdeModulePkg/Universal/StatusCodeHandler/Pei/StatusCodeHandlerPei.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/MdeModulePkg/Universal/StatusCodeHandler/Pei
#[-start-160216-IB03090424-add]#
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2E
#[-end-160216-IB03090424-add]#
  }
!endif

#[-start-160107-IB08450333-add]#
!disable InsydeModulePkg/Universal/Security/Tcg/TisPei/TisPei.inf
!disable InsydeModulePkg/Universal/Security/Tcg/Tcg2Pei/Tcg2Pei.inf
 
!if gInsydeTokenSpaceGuid.PcdH2OTpm2Supported
!if gChipsetPkgTokenSpaceGuid.FtpmSupport
  InsydeModulePkg/Universal/Security/Tcg/Tcg2Pei/Tcg2Pei.inf {
    <LibraryClasses>
      NULL|SecurityPkg/Library/HashInstanceLibSha1/HashInstanceLibSha1.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterPei.inf
      NULL|$(CHIPSET_PKG)/Library/Tpm2DeviceLibPtp/Tpm2InstanceLibPtt.inf
#[-start-160421-IB08450341-add]#
!if gChipsetPkgTokenSpaceGuid.PcdRuntimeFtpmDtpmSwitch
      NULL|$(CHIPSET_OVERRIDE)/SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpm.inf
!endif
#[-end-160421-IB08450341-add]#
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/InsydeModulePkg/Universal/Security/Tcg/Tcg2Pei
  }
!else
  InsydeModulePkg/Universal/Security/Tcg/TisPei/TisPei.inf
  InsydeModulePkg/Universal/Security/Tcg/Tcg2Pei/Tcg2Pei.inf {
    <LibraryClasses>
      NULL|SecurityPkg/Library/HashInstanceLibSha1/HashInstanceLibSha1.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/InsydeModulePkg/Universal/Security/Tcg/Tcg2Pei
  }
!endif

!endif
#[-end-160107-IB08450333-add]#



#[-start-160531-IB08450347-add]#
#[-start-160715-IB08450350-remove]#
#!disable InsydeModulePkg/Bus/Isa/Ps2KeyboardPei/Ps2KeyboardPei.inf
#[-end-160715-IB08450350-remove]#
#[-end-160531-IB08450347-add]#

#
# Please put DXE Modules in Components.X64 whatever target is 64 or 32bit.
# The futher build process will change this groupt to [Components.IA32] if
# target is 32 bit
#
[Components.$(DXE_ARCH)]
  $(CHIPSET_PKG)/RestoreMtrrDxe/RestoreMtrrDxe.inf
  $(CHIPSET_PKG)/OemAcpiPlatformDxe/OemAcpiPlatformDxe.inf

!if gInsydeTokenSpaceGuid.PcdH2OCsmSupported
  $(CHIPSET_PKG)/OemInt15CallBackSmm/OemInt15CallBackSmm.inf
  $(CHIPSET_PKG)/BbstableHookDxe/BbstableHookDxe.inf
!endif
  !disable MdeModulePkg/Core/Dxe/DxeMain.inf {
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <SOURCE_OVERRIDE_PATH>
      MdeModulePkg/Override/Core/Dxe
      $(CHIPSET_OVERRIDE)/MdeModulePkg/Core/Dxe
      $(CHIPSET_OVERRIDE)/MdeModulePkg/Override/Core/Dxe
  }

  !disable MdeModulePkg/Universal/EbcDxe/EbcDxe.inf
  !disable MdeModulePkg/Universal/Acpi/SmmS3SaveState/SmmS3SaveState.inf
  !disable MdeModulePkg/Universal/PrintDxe/PrintDxe.inf

  !disable IntelFrameworkModulePkg/Universal/SectionExtractionDxe/SectionExtractionDxe.inf
  !disable IntelFrameworkModulePkg/Universal/DataHubStdErrDxe/DataHubStdErrDxe.inf
  !disable InsydeModulePkg/Universal/Console/TgaDecoderDxe/TgaDecoderDxe.inf
  !disable InsydeModulePkg/Universal/Console/PcxDecoderDxe/PcxDecoderDxe.inf

  !disable InsydeModulePkg/Universal/Acpi/AcpiPlatformDxe/AcpiPlatformDxe.inf
  !disable InsydeModulePkg/Universal/Acpi/WpbtDxe/WpbtDxe.inf
#  !disable InsydeModulePkg/Bus/Isa/Ps2KeyboardDxe/Ps2keyboardDxe.inf
#!if $(ENBDT_PF_BUILD) == YES
#  InsydeModulePkg/Bus/Isa/Ps2KeyboardDxe/Ps2keyboardDxe.inf {
#    <SOURCE_OVERRIDE_PATH>
#      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Bus/Isa/Ps2KeyboardDxe
#  }
#!endif
#[-start-161123-IB07250310-add]#
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  !disable InsydeModulePkg/Universal/Recovery/EmuPeiGateDxe/EmuPeiGateDxe.inf
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  !disable InsydeModulePkg/Universal/PcdSmmDxe/Pcd.inf
  InsydeModulePkg/Universal/PcdSmmDxe/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/PcdSmmDxe
  }
!endif
!endif
#[-end-161123-IB07250310-add]#
#[-start-170208-IB15590021-add]#
!if gInsydeTokenSpaceGuid.PcdH2OAcpiBgrtSupported
  !disable InsydeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf
  InsydeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf {
    <SOURCE_OVERRIDE_PATH>
	  $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe
  }
!endif
#[-end-170208-IB15590021-add]#

  !disable InsydeModulePkg/Universal/CommonPolicy/PciHotPlugDxe/PciHotPlugDxe.inf
  !disable InsydeModulePkg/Universal/CommonPolicy/CommonSmiCallBackSmm/CommonSmiCallBackSmm.inf
#[-start-160302-IB11270148-remove]#
#   !disable InsydeModulePkg/Universal/Security/StorageSecurityCommandDxe/StorageSecurityCommandDxe.inf
#[-end-160302-IB11270148-remove]#
  !disable InsydeModulePkg/Universal/Console/VgaDriverPolicyDxe/VgaDriverPolicyDxe.inf
  !disable InsydeModulePkg/Universal/Console/EdidOverrideDxe/EdidOverrideDxe.inf
  !disable InsydeModulePkg/Universal/MemoryTest/GenericMemoryTestDxe/GenericMemoryTestDxe.inf
  !disable InsydeModulePkg/Universal/Irsi/IrsiRuntimeDxe/IrsiRuntimeDxe.inf
  !disable InsydeModulePkg/Universal/FirmwareVolume/FirmwareManagementRuntimeDxe/FirmwareManagementRuntimeDxe.inf
  !disable InsydeModulePkg/Universal/Disk/DelayUefiRaidOprom/DelayUefiRaidOprom.inf

  !disable InsydeModulePkg/Universal/Acpi/AcpiS3SaveDxe/AcpiS3SaveDxe.inf
  $(CHIPSET_OVERRIDE)/IntelFrameworkModulePkg/Universal/Acpi/AcpiS3SaveDxe/AcpiS3SaveDxe.inf
  !disable InsydeModulePkg/Universal/BdsDxe/BdsDxe.inf
  InsydeModulePkg/Universal/BdsDxe/BdsDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/BdsDxe/
  }
#[-start-190703-IB16530033-add]#
  !disable InsydeModulePkg/Bus/Pci/AhciBusDxe/AhciBusDxe.inf
  $(CHIPSET_PKG)/Override/InsydeModulePkg/Bus/Pci/AhciBusDxe/AhciBusDxe.inf
#[-end-190703-IB16530033-add]#

  
#
#----------------------------------------------------------------------
#  PLATFORM_RC_PACKAGE : $(PLATFORM_SOC) DXE drivers Start
#----------------------------------------------------------------------
#
   $(CHIPSET_PKG)/PlatformEmmcDxe/PlatformEmmcDxe.inf

#
#----------------------------------------------------------------------
#  PLATFORM_RC_PACKAGE : $(PLATFORM_SOC) DEX drivers End
#----------------------------------------------------------------------
#
!if gInsydeTokenSpaceGuid.PcdDynamicHotKeySupported
  $(CHIPSET_PKG)/DynamicHotKeyDxe/DynamicHotKeyDxe.inf
!endif
  $(CHIPSET_PKG)/CommonChipset/SpeakerDxe/LegacySpeakerDxe.inf

  $(CHIPSET_PKG)/SetupDataProviderDxe/SetupDataProviderDxe.inf
  $(CHIPSET_PKG)/VariableStorageSelector/Dxe/VariableStorageSelectorRuntimeDxe.inf
  $(CHIPSET_PKG)/VariableStorageSelector/Smm/VariableStorageSelectorSmm.inf
  !disable InsydeModulePkg/Universal/Variable/VariableRuntimeDxe/VariableRuntimeDxe.inf
  !disable InsydeModulePkg/Universal/Variable/EmuVariableRuntimeDxe/EmuVariableRuntimeDxe.inf
  !disable InsydeModulePkg/Universal/Variable/VariableDefaultUpdateDxe/VariableDefaultUpdateDxe.inf
#[-start-160426-IB10860196-add]#
  !disable InsydeModulePkg/Universal/FirmwareVolume/FtwLiteRuntimeDxe/FtwLiteRuntimeDxe.inf

#[-end-160426-IB10860196-add]#
!if gChipsetPkgTokenSpaceGuid.PcdUseEmuVariable == 0
  InsydeModulePkg/Universal/Variable/VariableRuntimeDxe/VariableRuntimeDxe.inf {
!if $(ENBDT_PF_BUILD) == YES
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/Variable/VariableRuntimeDxe
!endif
  }
!if gInsydeTokenSpaceGuid.PcdUseFastCrisisRecovery == 1
  InsydeModulePkg/Universal/Variable/EmuVariableRuntimeDxe/EmuVariableRuntimeDxe.inf
!endif

!else
  MdeModulePkg/Universal/Variable/EmuRuntimeDxe/EmuVariableRuntimeDxe.inf
!endif

  $(CHIPSET_PKG)/UefiSetupUtilityDxe/SetupUtilityDxe.inf
  $(CHIPSET_PKG)/OEMBadgingSupportDxe/OEMBadgingSupportDxe.inf
  $(CHIPSET_PKG)/IhisiSmm/IhisiSmm.inf
  $(CHIPSET_PKG)/ChipsetSvcDxe/ChipsetSvcDxe.inf
  $(CHIPSET_PKG)/ChipsetSvcSmm/ChipsetSvcSmm.inf
  $(CHIPSET_PKG)/SeCPolicyInitDxe/SeCPolicyInitDxe.inf

!if gChipsetPkgTokenSpaceGuid.PcdSwitchableGraphicsSupported
  # Switchable Graphics / Hybrid Graphics Support
  $(CHIPSET_PKG)/SwitchableGraphicsDxe/SwitchableGraphicsDxe.inf
  $(CHIPSET_PKG)/SwitchableGraphicsAcpi/AmdPowerXpressSsdt.inf
  $(CHIPSET_PKG)/SwitchableGraphicsAcpi/NvidiaOptimusSsdt.inf
!endif

!if gChipsetPkgTokenSpaceGuid.PcdSupportUnLockedBarHandle
 $(CHIPSET_PKG)/UnLockedBarHandleSmm/UnLockedBarHandleSmm.inf
!endif

  IntelFrameworkModulePkg/Universal/StatusCode/DatahubStatusCodeHandlerDxe/DatahubStatusCodeHandlerDxe.inf
  MdeModulePkg/Universal/MemoryTest/NullMemoryTestDxe/NullMemoryTestDxe.inf

  #
  # Compile only. Prevent Graphic setup error.
  #



  #
  # EDK II Related Platform codes
  #
  $(CHIPSET_PKG)/PlatformSmm/Platform.inf
  $(CHIPSET_PKG)/PlatformInfoDxe/PlatformInfoDxe.inf
  $(CHIPSET_PKG)/PlatformCpuInfo/PlatformCpuInfoDxe.inf
  $(CHIPSET_PKG)/PlatformDxe/Platform.inf
  $(CHIPSET_PKG)/PciPlatform/PciPlatform.inf
#[-start-160803-IB07220122-modify]#
  $(CHIPSET_PKG)/SaveMemoryConfig/SaveMemoryConfig.inf
#[-end-160803-IB07220122-modify]#
#[-start-151230-IB03090424-remove]#
#  $(CHIPSET_PKG)/IBBpreload/IBBpreloadDxe.inf
#[-end-151230-IB03090424-remove]#

  $(CHIPSET_PKG)/SmramSaveInfoHandlerSmm/SmramSaveInfoHandlerSmm.inf

!if $(GOP_DRIVER_ENABLE) == YES
  $(CHIPSET_PKG)/PlatformGopPolicy/PlatformGopPolicy.inf
!endif

!if $(HYBRID_ENABLE) == NO && $(SIMICS_ENABLE) == NO && $(VP_BIOS_ENABLE) == NO
!if $(ENBDT_PF_BUILD) == NO
  $(CHIPSET_PKG)/PlatformPmic/PlatformPmic.inf
!endif
!endif


  #
  # Application without built into ROM
  #
!if $(ENBDT_PF_BUILD) == NO
  $(CHIPSET_PKG)/Applications/PmicTest/PmicTest.inf
  $(CHIPSET_PKG)/Applications/UlpmcTest/UlpmcTest.inf
  $(CHIPSET_PKG)/Applications/UlpmcDbg/UlpmcDbg.inf
!endif

  $(CHIPSET_PKG)/Applications/Crc32/Crc32.inf
  $(CHIPSET_PKG)/Applications/Emmc/Emmc.inf
  $(CHIPSET_PKG)/Applications/I2cTest/I2cTest.inf
  $(CHIPSET_PKG)/Applications/Iosf/Iosf.inf
  $(CHIPSET_PKG)/Applications/LegacyBoot/LegacyBoot.inf
#  $(CHIPSET_PKG)/Applications/SecFwUpdate/SecFwUpdate.inf
  $(CHIPSET_PKG)/Applications/TalkI2C/TalkI2C.inf
  $(CHIPSET_PKG)/Applications/UlpiTest/UlpiTest.inf
  $(CHIPSET_PKG)/Applications/VkTestApp/VkTestApp.inf

#[-staru-161215-IB10860217-add]#
  !disable InsydeModulePkg/Universal/IhisiServicesSmm/IhisiServicesSmm.inf
  InsydeModulePkg/Universal/IhisiServicesSmm/IhisiServicesSmm.inf  {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/IhisiServicesSmm/
  }
#[-end-161215-IB10860217-add]#
#[-start-151214-IB03090424-remove]#
#!if $(CHARGING_ENABLE) == YES
#  $(CHIPSET_PKG)/S5Charging/S5Charging.inf{
#       <BuildOptions>
#        MSFT:*_*_*_CC_FLAGS             =  /D CHARGING_DEBUG
#        GCC:*_*_*_CC_FLAGS             =  -D CHARGING_DEBUG
#  }
#   $(CHIPSET_PKG)/S5Charging/S5ChargingInit.inf
#!endif
#[-end-151214-IB03090424-remove]#
!if $(LEGACY_BOOT_ENABLE) == YES
  $(CHIPSET_PKG)/ChipsetSmmThunkSmm/ChipsetSmmThunkSmm.inf
!endif
  #
  #Override: Add CedarView / ValleyView / CherryView CPU information for smm
  #
  !disable IA32FamilyCpuPkg/PiSmmCpuDxeSmm/PiSmmCpuDxeSmm.inf
  $(CHIPSET_OVERRIDE)/IA32FamilyCpuPkg/PiSmmCpuDxeSmm/PiSmmCpuDxeSmm.inf

  #
  # ACPI
  #
#[-start-170510-IB07400866-modify]#
  $(CHIPSET_PKG)/AcpiTablesPCAT/AcpiTables.inf {
    <SOURCE_OVERRIDE_PATH>
      $(PROJECT_PKG)/AcpiTablesDxe
  }
#[-end-170510-IB07400866-modify]#

  $(CHIPSET_PKG)/AcpiTablesPCAT/PlatformSsdt/PlatformSsdt.inf
  !disable InsydeModulePkg/Universal/Acpi/AcpiSupportDxe/AcpiSupportDxe.inf
  InsydeModulePkg/Universal/Acpi/AcpiSupportDxe/AcpiSupportDxe.inf {
!if $(ENBDT_PF_BUILD) == YES
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/Acpi/AcpiSupportDxe/
!endif
    <Depex>
      TRUE
  }

  #
  # TODO:Not ready
  #

#[-start-151124-IB07220021-remove]#
# !if gInsydeTokenSpaceGuid.PcdH2OConsoleRedirectionSupported == 1
# #   $(CHIPSET_PKG)/AcpiSpcr/AcpiSpcr.inf
# !endif
#[-end-151124-IB07220021-remove]#

  $(CHIPSET_PKG)/AcpiPlatform/AcpiPlatform.inf

  #
  # PCI
  #
  !disable InsydeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  InsydeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Bus/Pci/PciBusDxe
  }

#Use new XHCI module in kernel 05.05.11, should be removed after kernel 05.05.11 update.
#[-start-170508-IB15590030-modify]#
#[-start-180124-IB07400947-modify]#
!disable InsydeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
!if gInsydeTokenSpaceGuid.PcdH2OUsbSupported
!if $(KERNEL_XHCI_DXE_DRIVER_OVERRIDE) == YES
  $(CHIPSET_PKG)/Override/InsydeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
!else
  InsydeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
!endif
!endif
#[-end-180124-IB07400947-modify]#
#[-end-170508-IB15590030-modify]#

#[-start-151220-IB1127138-add]#
  $(CHIPSET_PKG)/CommonChipset/SetSsidSvidDxe/SetSsidSvidDxe.inf
#[-end-151220-IB1127138-add]#
!if $(ENBDT_PF_BUILD) == YES
  $(CHIPSET_PKG)/CommonChipset/AspmOverrideDxe/AspmOverrideDxe.inf
!endif

  #
  # ISA
  #
#[-start-160420-IB07400719-remove]#
#  $(CHIPSET_PKG)/Wpce791/SiO791.inf
#[-end-160420-IB07400719-remove]#
#[-start-151124-IB07220021-remove]#
# !if gInsydeTokenSpaceGuid.PcdH2OConsoleRedirectionSupported == 1
# IntelFrameworkModulePkg/Bus/Isa/IsaIoDxe/IsaIoDxe.inf
# !endif
#[-end-151124-IB07220021-remove]#
#[-start-161121-IB10860215-remove]#
##[-start-161026-IB10860213-add]#
#  !disable InsydeModulePkg/Bus/Isa/Ps2KeyboardDxe/Ps2keyboardDxe.inf
#  InsydeModulePkg/Bus/Isa/Ps2KeyboardDxe/Ps2keyboardDxe.inf {
#    <SOURCE_OVERRIDE_PATH>
#      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Bus/Isa/Ps2KeyboardDxe
#  }
##[-end-161026-IB10860213-add]#
#[-end-161121-IB10860215-remove]#
  #
  # Console
  #
#  $(CHIPSET_PKG)/ButtonArrayControl/ButtonArrayControl.inf

  #
  #  ECP
  #
#[-start-160218-IB08450338-add]#
  $(CHIPSET_PKG)/SmbiosUpdateDxe/SmbiosUpdateDxe.inf
#[-end-160218-IB08450338-add]#
#[-start-160120-IB07220035-remove]#
# !if $(SECURE_FLASH_SUPPORT) == YES
#[-end-160120-IB07220035-remove]#
  $(CHIPSET_PKG)/BiosRegionLock/BiosRegionLock.inf
#[-start-160120-IB07220035-remove]#
# !endif
#[-end-160120-IB07220035-remove]#

#[-start-160121-IB06740461-add]#
  $(CHIPSET_PKG)/I2cBus/Dxe/I2cMasterDxe.inf
#[-end-160121-IB06740461-add]#

  !disable InsydeModulePkg/Universal/CapsuleUpdate/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  InsydeModulePkg/Universal/CapsuleUpdate/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf {
!if $(TABLET_PF_BUILD) == YES
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/CapsuleUpdate/CapsuleRuntimeDxe
!endif
    <PcdsFeatureFlag>
      gEfiMdeModulePkgTokenSpaceGuid.PcdSupportUpdateCapsuleReset|TRUE
    <LibraryClasses>
      CapsuleUpdateCriteriaLib|InsydeModulePkg/Library/CapsuleUpdateCriteriaLib/CapsuleUpdateCriteriaLib.inf
  }

#[-start-160412-IB11270151-add]#
!if gInsydeTokenSpaceGuid.PcdH2OCapsuleUpdateSupported
  !disable InsydeModulePkg/Universal/CapsuleUpdate/CapsuleProcessorDxe/CapsuleProcessorDxe.inf
  InsydeModulePkg/Universal/CapsuleUpdate/CapsuleProcessorDxe/CapsuleProcessorDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/CapsuleUpdate/CapsuleProcessorDxe/
  }
!endif
#[-end-160412-IB11270151-add]#
  !disable InsydeModulePkg/Bus/I2c/I2cTouchPanelDxe/I2cTouchPanelDxe.inf
!if gInsydeTokenSpaceGuid.PcdH2OI2cSupported
  InsydeModulePkg/Bus/I2c/I2cTouchPanelDxe/I2cTouchPanelDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Bus/I2c/I2cTouchPanelDxe
  }
!endif

#[-start-161215-IB08450359-remove]#
#  #
#  # Variable Edit
#  #
#  $(CHIPSET_PKG)/VariableEditDxe/VariableEditDxe.inf
#  $(CHIPSET_PKG)/VariableEditSmm/VariableEditSmm.inf
#[-end-161215-IB08450359-remove]#
$(CHIPSET_PKG)/PlatformResetRunTimeDxe/PlatformResetRunTimeDxe.inf
!if $(ENBDT_PF_BUILD) == YES
  $(CHIPSET_PKG)/OemModifyOpRegionDxe/OemModifyOpRegionDxe.inf
!endif

#[-start-161206-IB07400822-modify]#
!if gChipsetPkgTokenSpaceGuid.PcdKscSupport == 1
  $(CHIPSET_PKG)/EC/EC.inf
!endif  
#[-end-161206-IB07400822-modify]#
  $(CHIPSET_PKG)/Dptf/Dxe/Dptf.inf

  !disable InsydeModulePkg/Universal/FirmwareVolume/FvbServicesRuntimeDxe/FvbServicesRuntimeDxe.inf
  InsydeModulePkg/Universal/FirmwareVolume/FvbServicesRuntimeDxe/FvbServicesRuntimeDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/FirmwareVolume/FvbServicesRuntimeDxe/
  }

  !disable InsydeModulePkg/Universal/FirmwareVolume/FlashDeviceFvbRuntimeDxe/FlashDeviceFvbRuntimeDxe.inf
  InsydeModulePkg/Universal/FirmwareVolume/FlashDeviceFvbRuntimeDxe/FlashDeviceFvbRuntimeDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/FirmwareVolume/FlashDeviceFvbRuntimeDxe/
  }

  #
  # USB TypeC
  #
  $(CHIPSET_PKG)/UsbTypeC/UsbTypeC.inf

#[-start-160111-IB03090424-add]#
  $(CHIPSET_PKG)/ApeiBertPkg/Whea/WheaSupport/WheaSupport.inf
#[-end-160111-IB03090424-add]#

#[-start-160429-IB03090426-add]#
!if $(HSTI_ENABLE) == TRUE
  $(CHIPSET_PKG)/HstiResultDxe/HstiResultDxe.inf
!endif
#[-end-160429-IB03090426-add]#
  
#[-start-151021-IB08450328-add]#
#[-start-151210-IB08450331-remove]#
#  EdkCompatibilityPkg/Compatibility/PiSmbiosRecordOnDataHubSmbiosRecordThunk/PiSmbiosRecordOnDataHubSmbiosRecordThunk.inf {
#    <SOURCE_OVERRIDE_PATH>
#      $(CHIPSET_PKG)/Override/EdkCompatibilityPkg/Compatibility/PiSmbiosRecordOnDataHubSmbiosRecordThunk/
#  }
#[-end-151210-IB08450331-remove]#
#[-end-151021-IB08450328-add]#

#[-start-160107-IB08450333-add]#
#[-start-160803-IB07220122-modify]#
#[-end-160803-IB07220122-modify]#

!disable InsydeModulePkg/Universal/Security/Tcg/TisDxe/TisDxe.inf
!disable InsydeModulePkg/Universal/Security/Tcg/Tcg2Dxe/Tcg2Dxe.inf
!disable InsydeModulePkg/Universal/Security/Tcg/Tcg2PhysicalPresenceDxe/Tcg2PhysicalPresenceDxe.inf
!disable InsydeModulePkg/Universal/Security/Tcg/Tcg2Smm/Tcg2Smm.inf

!if gInsydeTokenSpaceGuid.PcdH2OTpm2Supported
!if gChipsetPkgTokenSpaceGuid.FtpmSupport
  InsydeModulePkg/Universal/Security/Tcg/Tcg2Dxe/Tcg2Dxe.inf {
    <LibraryClasses>
      NULL|SecurityPkg/Library/HashInstanceLibSha1/HashInstanceLibSha1.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterDxe.inf
      NULL|$(CHIPSET_PKG)/Library/Tpm2DeviceLibPtp/Tpm2InstanceLibPtt.inf
#[-start-160421-IB08450341-add]#
!if gChipsetPkgTokenSpaceGuid.PcdRuntimeFtpmDtpmSwitch
      NULL|$(CHIPSET_OVERRIDE)/SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpm.inf
!endif
#[-end-160421-IB08450341-add]#
  }
  InsydeModulePkg/Universal/Security/Tcg/Tcg2PhysicalPresenceDxe/Tcg2PhysicalPresenceDxe.inf {
    <LibraryClasses>
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterDxe.inf
      NULL|$(CHIPSET_PKG)/Library/Tpm2DeviceLibPtp/Tpm2InstanceLibPtt.inf
#[-start-160421-IB08450341-add]#
!if gChipsetPkgTokenSpaceGuid.PcdRuntimeFtpmDtpmSwitch
      NULL|$(CHIPSET_OVERRIDE)/SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpm.inf
!endif
#[-end-160421-IB08450341-add]#
  }
  InsydeModulePkg/Universal/Security/Tcg/Tcg2Smm/Tcg2Smm.inf {
    <SOURCE_OVERRIDE_PATH>
#[-start-160912-IB07400782-modify]#
      $(CHIPSET_PKG)/Override/InsydeModulePkg/Universal/Security/Tcg/Tcg2Smm/
      $(CHIPSET_PKG)/Override2/InsydeModulePkg/Universal/Security/Tcg/Tcg2Smm/
#[-end-160912-IB07400782-modify]#
    <LibraryClasses>
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterDxe.inf
      NULL|$(CHIPSET_PKG)/Library/Tpm2DeviceLibPtp/Tpm2InstanceLibPtt.inf
#[-start-160421-IB08450341-add]#
!if gChipsetPkgTokenSpaceGuid.PcdRuntimeFtpmDtpmSwitch
      NULL|$(CHIPSET_OVERRIDE)/SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpm.inf
!endif
#[-end-160421-IB08450341-add]#
  }
!else # !FtpmSupport

  InsydeModulePkg/Universal/Security/Tcg/TisDxe/TisDxe.inf
  InsydeModulePkg/Universal/Security/Tcg/Tcg2Dxe/Tcg2Dxe.inf {
    <LibraryClasses>
      NULL|SecurityPkg/Library/HashInstanceLibSha1/HashInstanceLibSha1.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }
  InsydeModulePkg/Universal/Security/Tcg/Tcg2PhysicalPresenceDxe/Tcg2PhysicalPresenceDxe.inf
  InsydeModulePkg/Universal/Security/Tcg/Tcg2Smm/Tcg2Smm.inf

!endif
!endif

#[-start-160810-IB07220125-add]#
#[-start-160824-IB07220133-remove]#
# !if gInsydeTokenSpaceGuid.PcdH2OTpmSupported == 1 || gInsydeTokenSpaceGuid.PcdH2OTpm2Supported == 1
#   !disable InsydeModulePkg/Universal/Security/Tcg/MemoryOverwriteControl/TcgMor.inf
#   InsydeModulePkg/Universal/Security/Tcg/MemoryOverwriteControl/TcgMor.inf {
#     <SOURCE_OVERRIDE_PATH>
#       $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/Security/Tcg/MemoryOverwriteControl
#   }
# !endif
#[-end-160824-IB07220133-remove]#
#[-end-160810-IB07220125-add]#

!if gInsydeTokenSpaceGuid.PcdH2OTpmSupported == 1 || gInsydeTokenSpaceGuid.PcdH2OTpm2Supported == 1
  !disable InsydeModulePkg/Universal/Security/Tcg/TrEEConfigDxe/TrEEConfigDxe.inf
  InsydeModulePkg/Universal/Security/Tcg/TrEEConfigDxe/TrEEConfigDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/Security/Tcg/TrEEConfigDxe
    <LibraryClasses>
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf
  }
!endif


#[-start-160824-IB07220130-add]#
!if gInsydeTokenSpaceGuid.PcdSecureFlashSupported
  !disable InsydeModulePkg/Universal/Security/SecureFlash/SecureFlashDxe/SecureFlashDxe.inf
  InsydeModulePkg/Universal/Security/SecureFlash/SecureFlashDxe/SecureFlashDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/Security/SecureFlash/SecureFlashDxe
  }
!endif
#[-end-160824-IB07220130-add]#

#[-start-160824-IB07220133-remove]#
# #[-start-160531-IB08450347-add]#
# !disable InsydeModulePkg/Bus/Isa/Ps2KeyboardDxe/Ps2keyboardDxe.inf
# #[-start-160715-IB08450350-modify]#
#   InsydeModulePkg/Bus/Isa/Ps2KeyboardDxe/Ps2keyboardDxe.inf {
#     <SOURCE_OVERRIDE_PATH>
#       $(CHIPSET_OVERRIDE)/InsydeModulePkg/Bus/Isa/Ps2KeyboardDxe
#   }
# 
# #[-end-160715-IB08450350-modify]#
# #[-end-160531-IB08450347-add]#
#[-end-160824-IB07220133-remove]#
#[-start-161123-IB07250310-add]#
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  $(CHIPSET_PKG)/Override/IntelFsp2WrapperPkg/FspWrapperNotifyDxe/FspWrapperNotifyDxe.inf
!endif
#[-end-161123-IB07250310-add]#
#[-end-160107-IB08450333-add]#

!disable InsydeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  InsydeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/Disk/PartitionDxe
  }
!disable InsydeModulePkg/Universal/UserInterface/HiiDatabaseDxe/HiiDatabaseDxe.inf
  InsydeModulePkg/Universal/UserInterface/HiiDatabaseDxe/HiiDatabaseDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Universal/UserInterface/HiiDatabaseDxe
  }

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
#
# Define Build Options both for EDK and EDKII drivers.
#
#
# Define token for different Platform
#
!if $(TABLET_PF_BUILD) == YES
  DEFINE TABLET_PF_ENABLE     = /DTABLET_PF_ENABLE=1
  DEFINE TABLET_PF_ENABLE_GCC = -DTABLET_PF_ENABLE=1
!else
  DEFINE TABLET_PF_ENABLE     =
  DEFINE TABLET_PF_ENABLE_GCC =
!endif

!if $(ENBDT_PF_BUILD) == YES
  DEFINE ENBDT_PF_ENABLE     = /DENBDT_PF_ENABLE=1
  DEFINE ENBDT_PF_ENABLE_GCC = -DENBDT_PF_ENABLE=1
!else
  DEFINE ENBDT_PF_ENABLE     =
  DEFINE ENBDT_PF_ENABLE_GCC =
!endif

  DEFINE VP_BUILD_OPTIONS =

!if $(SOC_MODE_ENABLE) == YES
  DEFINE DSC_SOC_BUILD_OPTIONS      = /DSOC_MODE=1
  DEFINE DSC_SOC_BUILD_OPTIONS_GCC  = -DSOC_MODE=1
!else
  DEFINE DSC_SOC_BUILD_OPTIONS      =
  DEFINE DSC_SOC_BUILD_OPTIONS_GCC  =
!endif

!if $(MICROCODE_FREE) == YES
  DEFINE MICROCODE_FREE_BUILD_OPTIONS     = /D_MICROCODE_FREE_=1
  DEFINE MICROCODE_FREE_BUILD_OPTIONS_GCC = -D_MICROCODE_FREE_=1
!else
  DEFINE MICROCODE_FREE_BUILD_OPTIONS     = /D_MICROCODE_FREE_=0
  DEFINE MICROCODE_FREE_BUILD_OPTIONS_GCC = -D_MICROCODE_FREE_=0
!endif

!if $(SIMICS_ENABLE) == YES
  DEFINE SIMICS_BUILD_OPTIONS     = /D_SIMIC_=1
  DEFINE SIMICS_BUILD_OPTIONS_GCC = -D_SIMIC_=1
!else
  DEFINE SIMICS_BUILD_OPTIONS     = /D_SIMIC_=0
  DEFINE SIMICS_BUILD_OPTIONS_GCC = -D_SIMIC_=0
!endif

!if $(HYBRID_ENABLE) == YES
  DEFINE HYBRID_BUILD_OPTIONS     = /D_SLE_HYB_=1 /D_PSSD_FIX_=1
  DEFINE HYBRID_BUILD_OPTIONS_GCC = -D_SLE_HYB_=1 -D_PSSD_FIX_=0
!else
  DEFINE HYBRID_BUILD_OPTIONS     = /D_SLE_HYB_=0
  DEFINE HYBRID_BUILD_OPTIONS_GCC = -D_SLE_HYB_=0
!endif

!if $(COMPACT_ENABLE) == YES
  DEFINE COMPACT_BUILD_OPTIONS     = /D_SLE_COMP_=1
  DEFINE COMPACT_BUILD_OPTIONS_GCC = -D_SLE_COMP_=1
!else
  DEFINE COMPACT_BUILD_OPTIONS     = /D_SLE_COMP_=0
  DEFINE COMPACT_BUILD_OPTIONS_GCC = -D_SLE_COMP_=0
!endif

!if $(EXTERNAL_VGA_ENABLE) == YES
  DEFINE EXTERNAL_VGA_BUILD_OPTION     = /DEXTERNAL_VGA=1
  DEFINE EXTERNAL_VGA_BUILD_OPTION_GCC = -DEXTERNAL_VGA=1
!else
  DEFINE EXTERNAL_VGA_BUILD_OPTION     =
  DEFINE EXTERNAL_VGA_BUILD_OPTION_GCC =
!endif

!if $(SUPPORT_USER_PASSWORD) == YES
  DEFINE USERPASSWORD_OPTION      = /D SUPPORT_USER_PASSWORD
  DEFINE USERPASSWORD_OPTION_GCC  = -D SUPPORT_USER_PASSWORD
!else
  DEFINE USERPASSWORD_OPTION      =
  DEFINE USERPASSWORD_OPTION_GCC  =
!endif

!if $(ENBDT_S3_SUPPORT) == YES
  DEFINE ENBDT_S3_SUPPORT_OPTIONS     = /DNOCS_S3_SUPPORT
  DEFINE ENBDT_S3_SUPPORT_OPTIONS_GCC = -DNOCS_S3_SUPPORT
!else
  DEFINE ENBDT_S3_SUPPORT_OPTIONS     =
  DEFINE ENBDT_S3_SUPPORT_OPTIONS_GCC =
!endif

#
# Related TPM
#
!if $(FTPM_ENABLE) == YES
  DEFINE FTPM_SUPPORT_OPTION      = /D FTPM_ENABLE=1
  DEFINE FTPM_SUPPORT_OPTION_GCC  = -D FTPM_ENABLE=1
!else
  DEFINE FTPM_SUPPORT_OPTION      =
  DEFINE FTPM_SUPPORT_OPTION_GCC  =
!endif

!if $(SEC_ENABLE) == YES
  DEFINE DSC_SEC_BUILD_OPTIONS = /D SEC_SUPPORT_FLAG=1
  DEFINE DSC_SEC_BUILD_OPTIONS_GCC = -D SEC_SUPPORT_FLAG=1
!else
  DEFINE DSC_SEC_BUILD_OPTIONS =
  DEFINE DSC_SEC_BUILD_OPTIONS_GCC =
!endif

!if $(PCIESC_ENABLE) == YES
  DEFINE PCIESC_SUPPORT_BUILD_OPTION     = /D PCIESC_SUPPORT=1
  DEFINE PCIESC_SUPPORT_BUILD_OPTION_GCC = -D PCIESC_SUPPORT=1
!else
  DEFINE PCIESC_SUPPORT_BUILD_OPTION     =
  DEFINE PCIESC_SUPPORT_BUILD_OPTION_GCC =
!endif

!if $(ISH_ENABLE) == YES
  DEFINE ISH_ENABLE_OPTIONS     = /DISH_ENABLE=1
  DEFINE ISH_ENABLE_OPTIONS_GCC = -DISH_ENABLE=1
!else
  DEFINE ISH_ENABLE_OPTIONS     =
  DEFINE ISH_ENABLE_OPTIONS_GCC =
!endif

!if $(SATA_ENABLE) == YES
  DEFINE SATA_SUPPORT     = /DSATA_SUPPORT=1
  DEFINE SATA_SUPPORT_GCC = -DSATA_SUPPORT=1
!else
  DEFINE SATA_SUPPORT     =
  DEFINE SATA_SUPPORT_GCC =
!endif

#!if $(MRC_EV_RMT_BUILD) == YES
#  DEFINE MRC_EV_RMT_BUILD_OPTIONS     = /D RMT_ENABLE_TURNAROUNDS_TEST=1 /D RMT_JUMP_POSTCODES=1 /D BIT_ERROR_LOGGING_ENABLE=1 /D DEBUG_MSG=1
#  DEFINE MRC_EV_RMT_BUILD_OPTIONS_GCC = -D RMT_ENABLE_TURNAROUNDS_TEST=1 -D RMT_JUMP_POSTCODES=1 -D BIT_ERROR_LOGGING_ENABLE=1 -D DEBUG_MSG=1
#!else
#  DEFINE MRC_EV_RMT_BUILD_OPTIONS     = /DRMT_ENABLE_TURNAROUNDS_TEST=0 /DRMT_JUMP_POSTCODES=0 /DBIT_ERROR_LOGGING_ENABLE=0 /DDEBUG_MSG=0
#  DEFINE MRC_EV_RMT_BUILD_OPTIONS_GCC = -DRMT_ENABLE_TURNAROUNDS_TEST=0 /DRMT_JUMP_POSTCODES=0 /DBIT_ERROR_LOGGING_ENABLE=0 /DDEBUG_MSG=0
#!endif

!if $(BIOS_2MB_BUILD) == YES
  DEFINE BIOS_2MB_BUILDFLAG              = /D BIOS_2MB_BUILD
  DEFINE BIOS_2MB_BUILDFLAG_GCC          = -D BIOS_2MB_BUILD
!else
  DEFINE BIOS_2MB_BUILDFLAG              =
  DEFINE BIOS_2MB_BUILDFLAG_GCC          =
!endif

!if $(SILENT_LAKE_ENABLE) == YES
  DEFINE SILENT_LAKE_ENABLE_BUILDFLAG              = /D SILENT_LAKE_ENABLE
  DEFINE SILENT_LAKE_ENABLE_BUILDFLAG_GCC          = -D SILENT_LAKE_ENABLE
!else
  DEFINE SILENT_LAKE_ENABLE_BUILDFLAG              =
  DEFINE SILENT_LAKE_ENABLE_BUILDFLAG_GCC          =
!endif

!if $(S3_ENABLE) == YES
  DEFINE S3_BUILD_OPTIONS     = /D EFI_S3_RESUME=1
  DEFINE S3_BUILD_OPTIONS_GCC = -D EFI_S3_RESUME=1
!else
  DEFINE S3_BUILD_OPTIONS     =
  DEFINE S3_BUILD_OPTIONS_GCC =
!endif

!if $(EFI_DEBUG) == YES
  DEFINE EFI_DEBUG_OPTIONS              = /D EFI_DEBUG
  DEFINE EFI_DEBUG_OPTIONS_GCC          = -D EFI_DEBUG
!else
  DEFINE EFI_DEBUG_OPTIONS              =
  DEFINE EFI_DEBUG_OPTIONS_GCC          =
!endif

!if $(X64_CONFIG) == YES
 DEFINE EFI_X64_CONFIG_OPTIONS              = /D X64_CONFIG_BUILD=1
 DEFINE EFI_X64_CONFIG_OPTIONS_GCC          = /D X64_CONFIG_BUILD=1
!else
 DEFINE EFI_X64_CONFIG_OPTIONS              =
 DEFINE EFI_X64_CONFIG_OPTIONS_GCC          =
!endif
  DEFINE MERGED_RAIL_DDR_WA_BUILD_OPTION     = /D MERGED_RAIL_DDR_WA_ENABLE=1
  DEFINE MERGED_RAIL_DDR_WA_BUILD_OPTION_GCC = -D MERGED_RAIL_DDR_WA_ENABLE=1

  DEFINE HDAUDIO_SUPPORT_BUILD_OPTIONS      = /D HDAUDIO_SUPPORT
  DEFINE HDAUDIO_SUPPORT_BUILD_OPTIONS_GCC  = -D HDAUDIO_SUPPORT

!if $(PRAM_ENABLE) == YES
  DEFINE PRAM_SUPPORT_BUILD_OPTION     = /D PRAM_SUPPORT=1
  DEFINE PRAM_SUPPORT_BUILD_OPTION_GCC =  /D PRAM_SUPPORT=1
!else
  DEFINE PRAM_SUPPORT_BUILD_OPTION     =
  DEFINE PRAM_SUPPORT_BUILD_OPTION_GCC =
!endif
!if $(PEI_DISPLAY_ENABLE) == YES
  DEFINE PEI_DISPLAY_ENABLE_BUILD_OPTION     =  /D PEI_DISPLAY_ENABLE
  DEFINE PEI_DISPLAY_ENABLE_BUILD_OPTION_GCC =  /D PEI_DISPLAY_ENABLE
!else
  DEFINE PEI_DISPLAY_ENABLE_BUILD_OPTION     =
  DEFINE PEI_DISPLAY_ENABLE_BUILD_OPTION_GCC =
!endif
!if $(WIN7_SUPPORT) == YES
  DEFINE WIN7_SUPPORT_BUILD_OPTION     = /D WIN7_SUPPORT
  DEFINE WIN7_SUPPORT_BUILD_OPTION_GCC =  /D WIN7_SUPPORT
!else
  DEFINE WIN7_SUPPORT_BUILD_OPTION     =
  DEFINE WIN7_SUPPORT_BUILD_OPTION_GCC =
!endif
!if $(VP_BIOS_ENABLE) == YES
  DEFINE VP_BIOS_ENABLE_OPTION     = -DVP_BIOS_ENABLE=1
  DEFINE VP_BIOS_ENABLE_OPTION_GCC = -DVP_BIOS_ENABLE=1
!else
  DEFINE VP_BIOS_ENABLE_OPTION     =
  DEFINE VP_BIOS_ENABLE_OPTION_GCC =
!endif
!if $(SMM_FEATURE_LOCK_WA_ENABLE) == YES
  DEFINE SMM_FEATURE_LOCK_WA_OPTION     = -DSMM_FEATURE_LOCK_WA=1
  DEFINE SMM_FEATURE_LOCK_WA_OPTION_GCC = -DSMM_FEATURE_LOCK_WA=1
!else
  DEFINE SMM_FEATURE_LOCK_WA_OPTION =
  DEFINE SMM_FEATURE_LOCK_WA_OPTION_GCC =
!endif

!if $(NVM_VARIABLE_ENABLE) == YES
  DEFINE NVM_VARIABLE_BUILD_OPTION     = /D NVM_VARIABLE_ENABLE=1
  DEFINE NVM_VARIABLE_BUILD_OPTION_GCC = -D NVM_VARIABLE_ENABLE=1
!endif

!if $(SGX_ENABLE) == YES
  DEFINE SGX_ENABLE_OPTIONS     = /D SGX_SUPPORT=1
  DEFINE SGX_ENABLE_OPTIONS_GCC = -D SGX_SUPPORT=1
!else
  DEFINE SGX_ENABLE_OPTIONS     =
  DEFINE SGX_ENABLE_OPTIONS_GCC =
!endif

!if $(RVVP_BIOS_ENABLE) == YES
  DEFINE RVVPBIOS_BUILD_OPTIONS       = -DRVVP_ENABLE=1
  DEFINE RVVPBIOS_BUILD_OPTIONS_GCC   = -DRVVP_ENABLE=1
!else 
  DEFINE RVVPBIOS_BUILD_OPTIONS       = 
  DEFINE RVVPBIOS_BUILD_OPTIONS_GCC   =
!endif
#[-start-160406-IB07400715-remove]#
#!if $(BXTI_PF_BUILD) == TRUE
#  DEFINE BXTI_PF_ENABLE               = -D BXTI_PF_ENABLE=1
#  DEFINE BXTI_PF_ENABLE_GCC           = -D BXTI_PF_ENABLE=1
#!else
#  DEFINE BXTI_PF_ENABLE               =
#  DEFINE BXTI_PF_ENABLE_GCC           =  
#!endif
#[-end-160406-IB07400715-remove]#
!if $(NPK_ENABLE) == YES
  DEFINE NPK_ENABLE_OPTIONS     = /D NPK_ENABLE=1
  DEFINE NPK_ENABLE_OPTIONS_GCC = -D NPK_ENABLE=1
!else
  DEFINE NPK_ENABLE_OPTIONS     =
  DEFINE NPK_ENABLE_OPTIONS_GCC =
!endif

#[-start-151211-IB03090424-add]#
  DEFINE MEMORY_BUILD_OPTIONS   = -DEFI_MEMORY_INIT=1

!if $(BDAT_SUPPORT_ENABLED) == YES
  DEFINE BDAT_SUPPORT_BUILD_OPTION = -DBDAT_SUPPORT=1
!else
  DEFINE BDAT_SUPPORT_BUILD_OPTION = -DBDAT_SUPPORT=0
!endif
#[-end-151211-IB03090424-add]#
#[-start-161123-IB07250310-add]#
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  DEFINE FSP_WRAPPER_BUILD_OPTION     = /D FSP_WRAPPER_FLAG=1
  DEFINE FSP_WRAPPER_BUILD_OPTION_GCC = -DFSP_WRAPPER_FLAG=1
!else
  DEFINE FSP_WRAPPER_BUILD_OPTION     = 
  DEFINE FSP_WRAPPER_BUILD_OPTION_GCC = 
!endif
#[-end-161123-IB07250310-add]#

#[-start-161123-IB07250310-modify]#
#[-start-151211-IB03090424-modify]#
  DEFINE EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS = $(TABLET_PF_ENABLE) \
                                               $(ENBDT_PF_ENABLE) \
                                               $(EXTERNAL_VGA_BUILD_OPTION) \
                                               $(MICROCODE_FREE_BUILD_OPTIONS) \
                                               $(SIMICS_BUILD_OPTIONS) \
                                               $(HYBRID_BUILD_OPTIONS) \
                                               $(COMPACT_BUILD_OPTIONS) \
                                               $(USERPASSWORD_OPTION) \
                                               $(ENBDT_S3_SUPPORT_OPTIONS) \
                                               $(PCIESC_SUPPORT_BUILD_OPTION) \
                                               $(ISH_ENABLE_OPTIONS) \
                                               $(DSC_SEC_BUILD_OPTIONS) \
                                               $(FTPM_SUPPORT_OPTION) \
                                               $(SATA_SUPPORT) \
                                               $(BIOS_2MB_BUILDFLAG) \
                                               $(SILENT_LAKE_ENABLE_BUILDFLAG) \
                                               $(S3_BUILD_OPTIONS) \
                                               $(EFI_DEBUG_OPTIONS) \
                                               $(MERGED_RAIL_DDR_WA_BUILD_OPTION) \
                                               $(EFI_X64_CONFIG_OPTIONS) \
                                               $(HDAUDIO_SUPPORT_BUILD_OPTIONS) \
                                               $(PRAM_SUPPORT_BUILD_OPTION) \
                                               $(PEI_DISPLAY_ENABLE_BUILD_OPTION) \
                                               $(WIN7_SUPPORT_BUILD_OPTION) \
                                               $(VP_BIOS_ENABLE_OPTION) \
                                               $(SMM_FEATURE_LOCK_WA_OPTION) \
                                               $(NVM_VARIABLE_BUILD_OPTION) \
                                               $(SGX_ENABLE_OPTIONS) \
                                               $(RVVPBIOS_BUILD_OPTIONS) \
#[-start-160406-IB07400715-remove]#
#                                               $(BXTI_PF_ENABLE) \
#[-end-160406-IB07400715-remove]#
                                               $(NPK_ENABLE_OPTIONS) \
                                               $(MEMORY_BUILD_OPTIONS) \
                                               $(BDAT_SUPPORT_BUILD_OPTION) \
                                               $(FSP_WRAPPER_BUILD_OPTION)
#[-end-151211-IB03090424-modify]#

  DEFINE EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC = $(TABLET_PF_ENABLE_GCC) \
                                                   $(ENBDT_PF_ENABLE_GCC) \
                                                   $(EXTERNAL_VGA_BUILD_OPTION_GCC) \
                                                   $(MICROCODE_FREE_BUILD_OPTIONS_GCC) \
                                                   $(SIMICS_BUILD_OPTIONS_GCC) \
                                                   $(HYBRID_BUILD_OPTIONS_GCC) \
                                                   $(COMPACT_BUILD_OPTIONS_GCC) \
                                                   $(USERPASSWORD_OPTION_GCC) \
                                                   $(ENBDT_S3_SUPPORT_OPTIONS_GCC) \
                                                   $(PCIESC_SUPPORT_BUILD_OPTION_GCC) \
                                                   $(ISH_ENABLE_OPTIONS_GCC) \
                                                   $(DSC_SEC_BUILD_OPTIONS_GCC) \
                                                   $(FTPM_SUPPORT_OPTION_GCC) \
                                                   $(MRC_EV_RMT_BUILD_OPTIONS_GCC) \
                                                   $(SATA_SUPPORT_GCC) \
                                                   $(BIOS_2MB_BUILDFLAG_GCC) \
                                                   $(SILENT_LAKE_ENABLE_BUILDFLAG_GCC) \
                                                   $(S3_BUILD_OPTIONS_GCC) \
                                                   $(EFI_OPTIONS_DEBUG_GCC) \
                                                   $(MERGED_RAIL_DDR_WA_BUILD_OPTION_GCC) \
                                                   $(EFI_X64_CONFIG_OPTIONS_GCC) \
                                                   $(HDAUDIO_SUPPORT_BUILD_OPTIONS_GCC) \
                                                   $(PRAM_SUPPORT_BUILD_OPTION_GCC) \
                                                   $(PEI_DISPLAY_ENABLE_BUILD_OPTION_GCC) \
                                                   $(WIN7_SUPPORT_BUILD_OPTION_GCC) \
                                                   $(VP_BIOS_ENABLE_OPTION_GCC) \
                                                   $(SMM_FEATURE_LOCK_WA_OPTION_GCC) \
                                                   $(NVM_VARIABLE_BUILD_OPTION_GCC) \
                                                   $(SGX_ENABLE_OPTIONS_GCC) \
                                                   $(RVVPBIOS_BUILD_OPTIONS_GCC) \
#[-start-160406-IB07400715-remove]#
#                                                   $(BXTI_PF_ENABLE_GCC) \
#[-end-160406-IB07400715-remove]#
                                                   $(NPK_ENABLE_OPTIONS_GCC) \
                                                   $(FSP_WRAPPER_BUILD_OPTION_GCC)
#[-end-161123-IB07250310-modify]#

[BuildOptions.Common.EDKII]
#[-start-161123-IB07250310-add]#
  MSFT:*_*_IA32_ASM_FLAGS     = $(FSP_WRAPPER_BUILD_OPTION)
#[-end-161123-IB07250310-add]#
#[-start-160216-IB03090424-modify]#
  MSFT:*_*_IA32_CC_FLAGS      = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS) /Gs81920
#[-end-160216-IB03090424-modify]#
  MSFT:*_*_IA32_VFRPP_FLAGS   = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  MSFT:*_*_IA32_APP_FLAGS     = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  MSFT:*_*_IA32_PP_FLAGS      = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  MSFT:*_*_IA32_ASLPP_FLAGS   = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)

  GCC:*_*_IA32_CC_FLAGS      = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC)
  GCC:*_*_IA32_VFRPP_FLAGS   = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC)
  GCC:*_*_IA32_APP_FLAGS     = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC)
  GCC:*_*_IA32_PP_FLAGS      = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC)
  GCC:*_*_IA32_ASLPP_FLAGS   = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC)

  MSFT:*_*_X64_CC_FLAGS       = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  MSFT:*_*_X64_VFRPP_FLAGS    = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  MSFT:*_*_X64_APP_FLAGS      = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  MSFT:*_*_X64_PP_FLAGS       = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  MSFT:*_*_X64_ASLPP_FLAGS    = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)

  GCC:*_*_X64_CC_FLAGS       = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC)
  GCC:*_*_X64_VFRPP_FLAGS    = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC)
  GCC:*_*_X64_APP_FLAGS      = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC)
  GCC:*_*_X64_PP_FLAGS       = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC)
  GCC:*_*_X64_ASLPP_FLAGS    = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC)

  GCC:*_*_*_CC_FLAGS             = -DMDEPKG_NDEBUG
  GCC:*_*_IA32_JWASM_FLAGS       =
  GCC:*_*_X64_JWASM_FLAGS        =
  INTEL:*_*_*_CC_FLAGS           = /D MDEPKG_NDEBUG
!if $(EFI_DEBUG) == NO
  MSFT:RELEASE_*_*_CC_FLAGS      = /D MDEPKG_NDEBUG
  MSFT:DEBUG_*_*_CC_FLAGS        = /D MDEPKG_NDEBUG
  GCC:RELEASE_*_*_CC_FLAGS       = -D MDEPKG_NDEBUG
  GCC:DEBUG_*_*_CC_FLAGS         = -D MDEPKG_NDEBUG
!endif
[PcdsFeatureFlag]
!if $(AT_ENABLE) == YES
  gChipsetPkgTokenSpaceGuid.PcdAntiTheftSupported|TRUE
!else
  gChipsetPkgTokenSpaceGuid.PcdAntiTheftSupported|FALSE
!endif

!if $(MRC_EV_RMT_BUILD) == YES
  gChipsetPkgTokenSpaceGuid.MrcRmtEfiDebug|TRUE
!else
  gChipsetPkgTokenSpaceGuid.MrcRmtEfiDebug|FALSE
!endif
!if $(USB_DEBUG_SUPPORT) == YES
  gChipsetPkgTokenSpaceGuid.PcdUsbResetHc|FALSE
!endif
!if $(INSYDE_DEBUGGER) == YES
!if $(H2O_DDT_DEBUG_IO) == Usb
  gChipsetPkgTokenSpaceGuid.PcdUsbResetHc|FALSE
!endif
!if $(H2O_DDT_DEBUG_IO) == Xhc
  gChipsetPkgTokenSpaceGuid.PcdUsbResetHc|FALSE
!endif
!endif

  #
  # Override InsydeFlashDevicesPkg.dec Pcds
  #

#[-start-160307-IB07400708-remove]#
#  #
#  # Manufacturer group enable/disable PCDs
#  #
#  gH2OFlashDeviceGuid.PcdSstEnable|FALSE
#  gH2OFlashDeviceGuid.PcdAtmelEnable|FALSE
#  gH2OFlashDeviceGuid.PcdWinbondEnable|TRUE
#  gH2OFlashDeviceGuid.PcdMxicEnable|FALSE
#  gH2OFlashDeviceGuid.PcdIntelEnable|FALSE
#  gH2OFlashDeviceGuid.PcdStEnable|FALSE
#  gH2OFlashDeviceGuid.PcdPmcEnable|FALSE
#  gH2OFlashDeviceGuid.PcdGdEnable|FALSE
#  gH2OFlashDeviceGuid.PcdNumonyxEnable|FALSE
#
#  #
#  # Individual Flash Device enable/disable PCDs
#  #
#  gH2OFlashDeviceEnableGuid.PcdSst25lf040SpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdSst25lf040aSpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdSst25lf080aSpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdSst25vf080bSpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdSst25vf016bSpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdAtmel25df041SpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdAtmel25df081aSpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdAtmel26df161SpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdAtmel26df321SpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdWinbondW25q64bvSpiEnable|FALSE
##[-start-151124-IB07220020-modify]#
#  gH2OFlashDeviceEnableGuid.PcdWinbondW25q64fwSpiEnable|TRUE
##[-end-151124-IB07220020-modify]#
#  gH2OFlashDeviceEnableGuid.PcdWinbondW25q16dwSpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdAtmel25df641SpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdMxic25l8005SpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdMxic25l1605aSpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdStM25pe80SpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdPmc25lv040SpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdSst49lf008aLpcEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdSst39vf080LpcEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdIntel82802acLpcEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdSst49lf004aLpcEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdPmc49fl004tLpcEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdWinbond39v040fpaLpcEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdSt50flw040aLpcEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdCommonvidCommondidSpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdMxic25u6435fSpiEnable|TRUE
#[-end-160307-IB07400708-remove]#

  gEfiCpuTokenSpaceGuid.PcdCpuSmmBlockStartupThisAp|TRUE

#[-start-160307-IB07400708-add]#
#===============================================================================#
#                                                                               #
# SEG Defines/LibraryClasses/Pcds/Components/BuildOptions                       #
#                                                                               #
#===============================================================================#

#
# Defines
#
[Defines]
  DEFINE CHIPSET_OVERRIDE2     = $(CHIPSET_PKG)/Override2

#
# LibraryClasses
#
[LibraryClasses]
  OemGraphicsLib|InsydeModulePkg/Library/OemGraphicsLib/OemGraphicsLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Library/OemGraphicsLib
  }
!if gChipsetPkgTokenSpaceGuid.PcdUartPostCodeSupport == 1
  PostCodeLib|$(CHIPSET_PKG)/Library/UartPostCodeLib/UartPostCodeLib.inf
!endif

!if $(EFI_DEBUG) == YES || $(USB_DEBUG_SUPPORT) == YES
  UefiBootServicesTableLib|$(CHIPSET_OVERRIDE2)/MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|$(CHIPSET_OVERRIDE2)/MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
!endif

#[-start-160504-IB07400722-add]#
  KernelVfrLib|InsydeModulePkg/Library/SetupUtilityLib/KernelVfrLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/InsydeModulePkg/Library/SetupUtilityLib
      $(CHIPSET_PKG)/Override2/InsydeModulePkg/Library/SetupUtilityLib
  }
  
  SetupUtilityLib|InsydeModulePkg/Library/SetupUtilityLib/SetupUtilityLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/InsydeModulePkg/Library/SetupUtilityLib
      $(CHIPSET_PKG)/Override2/InsydeModulePkg/Library/SetupUtilityLib
  }
#[-end-160504-IB07400722-add]#
#[-start-160604-IB07400740-add]#
  MultiPlatformBaseLib|$(CHIPSET_PKG)/Library/MultiPlatformBaseLib/MultiPlatformBaseLib.inf
#[-end-160604-IB07400740-add]#
#[-start-160615-IB07400744-add]#
#[-start-180914-IB07401008-modify]#
  GenericBdsLib|InsydeModulePkg/Library/GenericBdsLib/GenericBdsLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE)/InsydeModulePkg/Library/GenericBdsLib
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Library/GenericBdsLib
  }
#[-end-180914-IB07401008-modify]#
#[-end-160615-IB07400744-add]#
#[-start-160828-IB07400775-add]#
  SioLib|InsydeModulePkg/Library/SioLib/SioLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Library/SioLib
  }
#[-end-160828-IB07400775-add]#
#[-start-160913-IB07400784-add]#
  !disable BaseSetupDefaultLib|InsydeModulePkg/Library/SetupDefaultLib/SetupDefaultLib.inf
  BaseSetupDefaultLib|InsydeModulePkg/Library/SetupDefaultLib/SetupDefaultLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Library/SetupDefaultLib
  }
#[-end-160913-IB07400784-add]#

#[-start-160921-IB07400787-add]#
  !disable BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf {
    <SOURCE_OVERRIDE_PATH>
      MdePkg/Override/Library/BaseLib
      $(CHIPSET_OVERRIDE2)/MdePkg/Library/BaseLib
  }
#[-end-160921-IB07400787-add]#
#[-start-161206-IB07400822-add]#
!if gChipsetPkgTokenSpaceGuid.PcdKscSupport == 0
  EcLib|$(CHIPSET_PKG)/Library/BaseEcLib/BaseEcLibNull.inf
  EcHwLib|$(CHIPSET_PKG)/Library/BaseEcHwLib/BaseEcHwLibNull.inf
  EcMiscLib|$(CHIPSET_PKG)/Library/BaseEcMiscLib/BaseEcMiscLibNull.inf
!endif
#[-end-161206-IB07400822-add]#
#[-start-190321-IB07401093-add]#
  BaseCryptLib|$(CHIPSET_OVERRIDE2)/InsydeModulePkg/Library/BaseCryptLib/RuntimeCryptLib.inf
#[-end-190321-IB07401093-add]#

[LibraryClasses.IA32]

[LibraryClasses.common.SEC]

[LibraryClasses.common.PEIM]
#[-start-161206-IB07400822-add]#
!if gChipsetPkgTokenSpaceGuid.PcdKscSupport == 0
  EcLib|$(CHIPSET_PKG)/Library/BaseEcLib/BaseEcLibNull.inf
  EcHwLib|$(CHIPSET_PKG)/Library/BaseEcHwLib/BaseEcHwLibNull.inf
  EcMiscLib|$(CHIPSET_PKG)/Library/BaseEcMiscLib/BaseEcMiscLibNull.inf  
  PeiKscLib|$(CHIPSET_PKG)/Library/Ksc/Pei/PeiKscLibNull.inf
!endif
#[-end-161206-IB07400822-add]#
#[-start-190321-IB07401093-add]#
  BaseCryptLib|$(CHIPSET_OVERRIDE2)/InsydeModulePkg/Library/BaseCryptLib/PeiCryptLib.inf
#[-end-190321-IB07401093-add]#

[LibraryClasses.common.PEI_CORE]

[LibraryClasses.common.DXE_CORE]
!if $(EFI_DEBUG) == YES || $(USB_DEBUG_SUPPORT) == YES
!if $(USE_EDKII_DEBUG_LIB_DXE) == YES
  SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
!else  
  DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
!endif
!endif

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
!if $(EFI_DEBUG) == YES || $(USB_DEBUG_SUPPORT) == YES
!if $(USE_EDKII_DEBUG_LIB_DXE) == YES
  SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
!else  
  DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
!endif
!endif
#[-start-170421-IB07400874-add]#
  #
  # Link Null SmmServicesTableLib for DXE RUNTIME driver
  #
  SmmServicesTableLib|$(CHIPSET_PKG)\Library\BaseSmmServicesTableLibNull\BaseSmmServicesTableLibNull.inf
#[-end-170421-IB07400874-add]#

[LibraryClasses.common.DXE_DRIVER]
!if $(EFI_DEBUG) == YES || $(USB_DEBUG_SUPPORT) == YES
!if $(USE_EDKII_DEBUG_LIB_DXE) == YES
  SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
!else  
  DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
!endif
!endif
#[-start-161206-IB07400822-add]#
!if gChipsetPkgTokenSpaceGuid.PcdKscSupport == 0
  DxeKscLib|$(CHIPSET_PKG)/Library/Ksc/Dxe/DxeKscLibNull.inf
!endif
#[-end-161206-IB07400822-add]#
#[-start-170421-IB07400874-add]#
  #
  # Link Null SmmServicesTableLib for DXE driver
  #
  SmmServicesTableLib|$(CHIPSET_PKG)\Library\BaseSmmServicesTableLibNull\BaseSmmServicesTableLibNull.inf
#[-end-170421-IB07400874-add]#

[LibraryClasses.common.SMM_CORE]
!if $(EFI_DEBUG) == YES || $(USB_DEBUG_SUPPORT) == YES
!if $(USE_EDKII_DEBUG_LIB_SMM) == YES
  SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
!else  
  DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
!endif
!endif

[LibraryClasses.common.DXE_SMM_DRIVER]
!if $(EFI_DEBUG) == YES || $(USB_DEBUG_SUPPORT) == YES
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
#[-start-160928-IB07400789-add]#
  CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/SmmCpuExceptionHandlerLib.inf
#[-end-160928-IB07400789-add]#
!if $(USE_EDKII_DEBUG_LIB_SMM) == YES
  SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
!else  
  DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
!endif
!endif

#[-start-161206-IB07400822-add]#
!if gChipsetPkgTokenSpaceGuid.PcdKscSupport == 0
  SmmKscLib|$(CHIPSET_PKG)/Library/Ksc/Smm/SmmKscLibNull.inf
!endif
#[-end-161206-IB07400822-add]#

[LibraryClasses.common.COMBINED_SMM_DXE]
!if $(EFI_DEBUG) == YES || $(USB_DEBUG_SUPPORT) == YES
!if $(USE_EDKII_DEBUG_LIB_SMM) == YES
  SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
!else  
  DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
!endif
!endif

[LibraryClasses.common.UEFI_DRIVER]
!if $(EFI_DEBUG) == YES || $(USB_DEBUG_SUPPORT) == YES
!if $(USE_EDKII_DEBUG_LIB_DXE) == YES
  SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
!else  
  DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
!endif
!endif

[LibraryClasses.common.UEFI_APPLICATION]
!if $(EFI_DEBUG) == YES || $(USB_DEBUG_SUPPORT) == YES
!if $(USE_EDKII_DEBUG_LIB_DXE) == YES
  SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
!else  
  DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
!endif
!endif

#[-start-190321-IB07401093-add]#
[LibraryClasses.common.USER_DEFINED]
  BaseCryptLib|$(CHIPSET_OVERRIDE2)/InsydeModulePkg/Library/BaseCryptLib/PeiCryptLib.inf
#[-end-190321-IB07401093-add]#

#
# Pcds
#
[PcdsFeatureFlag]
[PcdsFixedAtBuild]
#[-start-160411-IB07400715-add]#
  gInsydeCrTokenSpaceGuid.PcdH2OCrDevice|{  \
  #-------------------------------------------------------------------------------------------------------------
  #|  Enable   || DevName        || Device Path String
  #-------------------------------------------------------------------------------------------------------------
    UINT8(0x01), L"COM_A"         , L"PciRoot(0x0)/Pci(0x1F,0x0)/Serial(0x0)",  \
    UINT8(0x00), L"COM_B"         , L"PciRoot(0x0)/Pci(0x1F,0x0)/Serial(0x1)",  \
    UINT8(0x01), L"HSUART-2"      , L"PciRoot(0x0)/Pci(0x18,0x2)",  \
    UINT8(0x00), L"OTHER"         , L"PciRoot(0x0)/*",  \
    UINT8(0xFF), UINT8(0xFF)      , UINT8(0xFF) \
    }
    
#[-start-161023-IB07400803-modify]#
!if $(INSYDE_DEBUGGER) == YES AND $(H2O_DDT_DEBUG_IO) == Com
#[-end-161023-IB07400803-modify]#
  gInsydeCrTokenSpaceGuid.PcdH2OCrDevice|{  \
    UINT8(0xFF), UINT8(0xFF)      , UINT8(0xFF) \
    }
!endif

  #                                                      Bus          Device      Function       Vendor ID       Device ID
  #                                                  ===========   ===========   ===========   =============   =============
  gInsydeCrTokenSpaceGuid.PcdH2OPciHsUartDeviceList|{UINT32(0x00), UINT32(0x18), UINT32(0x00), UINT32(0x0000), UINT32(0x0000),  \
                                                     UINT32(0x00), UINT32(0x18), UINT32(0x01), UINT32(0x0000), UINT32(0x0000),  \
                                                     UINT32(0x00), UINT32(0x18), UINT32(0x02), UINT32(0x0000), UINT32(0x0000),  \
                                                     UINT32(0x00), UINT32(0x18), UINT32(0x03), UINT32(0x0000), UINT32(0x0000),  \
                                                     UINT32(0xFF), UINT32(0xFF), UINT32(0xFF), UINT32(0xFFFF), UINT32(0xFFFF)} # All 0xFF indicates end of list.

  #                                                      Enable          Address
  #                                                    =========    =================
  gInsydeCrTokenSpaceGuid.PcdH2OPchHsUartMmioAddress|{ UINT8(0xFF), UINT32(0xFFFFFFFF)} # All 0xFF indicates end of list.
#[-end-160411-IB07400715-add]#
#[-start-160929-IB05400712-add]#
  #
  # Beep table for different kinds of Beep sounds.
  # {BeepTypeId, Sound1, Sound2, Sound3, Sound4, Sound5, Sound6, Sound7, Sound8, Sound9}
  #
  #  Sound type:
  #  - BEEP_NONE : 0x00
  #  - BEEP_LONG : 0x01
  #  - BEEP_SHORT: 0x02
  #
  gInsydeTokenSpaceGuid.PcdBeepTypeList|{ \
    UINT8(0x00), UINT8(0x1), UINT8(0x1), UINT8(0x1), UINT8(0x2), UINT8(0x2), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), \ # 0: Long, Long, Long, Short, Short.
    UINT8(0x01), UINT8(0x2), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), \ # 1: Short.
    UINT8(0x02), UINT8(0x2), UINT8(0x1), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), \ # 2: Short, Long.
    UINT8(0x03), UINT8(0x1), UINT8(0x2), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), \ # 3: Long, Short.
    UINT8(0x04), UINT8(0x2), UINT8(0x2), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), \ # 4: Short, Short.
    UINT8(0x05), UINT8(0x1), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), \ # 5: Long
    UINT8(0xFF), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0)  \ # End Entry
  }
  #
  # Beep List for different kinds of Beep event for Status Code.
  # {CodeType, CodeValue, BeepTypeId, BeepLoopCount, Reserved, Reserved}
  #
  #  Code Type: Refer to PiStatusCode.h
  #  - EFI_PROGRESS_CODE             0x00000001
  #  - EFI_ERROR_CODE                0x00000002
  #  - EFI_DEBUG_CODE                0x00000003
  #
  #  Code Value: Refer to PiStatusCode.h
  #  - EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_PC_INIT             0x00051005
  #  - EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_EC_NONE_DETECTED    0x00051009
  #
  #  BeepTypeId:
  #  - Refer to the Index number of PcdBeepTypeList
  #
  #  BeepLoopCount:
  #  - Indicate the loop count for this Beep event.
  #
  gInsydeTokenSpaceGuid.PcdStatusCodeBeepList|{ \
    UINT32(0x80000002), UINT32(0x00051009), UINT8(0x5), UINT8(0xF0), UINT8(0), UINT8(0),    \ # (EFI_ERROR_CODE|EFI_ERROR_MAJOR), (EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_EC_NONE_DETECTED)
    UINT32(0x0),UINT32(0x0),UINT8(0x0),UINT8(0x0),UINT8(0),UINT8(0)                         \ # End Entry
  }
#[-end-160929-IB05400712-add]#
[PcdsDynamicDefault]

[PcdsDynamicExDefault]

  #
  # Add cFeon/EON EN25S64 Spi config
  #
  gH2OFlashDeviceMfrNameGuid.PcdcFeonEonEn25s64SpiMfrName|"cFeon/EON"
  gH2OFlashDevicePartNameGuid.PcdcFeonEonEn25s64SpiPartName|"EN25S64"
  gH2OFlashDeviceGuid.PcdcFeonEonEn25s64Spi|{ \ # cFeon/EON  EN25S64
    0x03, 0x00, 0x00, 0x00, 0x1C, 0x38, 0x17, 0x00,  \  # DeviceType = 03  Id =17381C
    0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x08   \  # ExtId = 00000000  BlockSize = 0010  BlockCount = 0800
  }

  gH2OFlashDeviceConfigGuid.PcdcFeonEonEn25s64SpiConfig|{ \ # cFeon/EON  EN25S64
    0x28, 0x00, 0x00, 0x00, 0x9f, 0x20, 0x02, 0x01,  \  # Size = 00000028  ReadIdOp = 9F  EraseOp = 20  WriteOp = 02  WriteStatusOp = 01
    0x03, 0x05, 0x00, 0x00, 0x7c, 0xf2, 0x06, 0x06,  \  # ReadOp = 03  ReadStatusOp = 05  OpType = F27C  WriteEnablePrefix = 06  WriteStatusEnablePrefix = 06
    0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,  \  # GlobalProtectAvailable = 01  BLockProtectAvailable = 00  BlockProtectCodeRequired = 00  MultiByteProgramAvailable = 00  BytesOfId = 00  MinBytePerProgRead = 01  NoVolStatusAvailable = 00
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,  \  # GlobalProtectCode = 00  GlobalUnprotectCode = 00  BlockProtectCode = 00  BlockUnprotectCode = 00  DeviceSize = 00800000
    0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   \  # BlockEraseSize = 00001000  BlockProtectSize = 00000000
  }

  #
  # Add cFeon/EON EN25S32 Spi config
  #
  gH2OFlashDeviceMfrNameGuid.PcdcFeonEonEn25s32SpiMfrName|"cFeon/EON"
  gH2OFlashDevicePartNameGuid.PcdcFeonEonEn25s32SpiPartName|"EN25S32"
  gH2OFlashDeviceGuid.PcdcFeonEonEn25s32Spi|{ \ # cFeon/EON  EN25S32
    0x03, 0x00, 0x00, 0x00, 0x1C, 0x38, 0x16, 0x00,  \  # DeviceType = 03  Id = 16381C
    0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x04   \  # ExtId = 00000000  BlockSize = 0010  BlockCount = 0400
  }

  gH2OFlashDeviceConfigGuid.PcdcFeonEonEn25s32SpiConfig|{ \ # cFeon/EON  EN25S32
    0x28, 0x00, 0x00, 0x00, 0x9f, 0x20, 0x02, 0x01,  \  # Size = 00000028  ReadIdOp = 9F  EraseOp = 20  WriteOp = 02  WriteStatusOp = 01
    0x03, 0x05, 0x00, 0x00, 0x7c, 0xf2, 0x06, 0x06,  \  # ReadOp = 03  ReadStatusOp = 05  OpType = F27C  WriteEnablePrefix = 06  WriteStatusEnablePrefix = 06
    0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,  \  # GlobalProtectAvailable = 00  BLockProtectAvailable = 00  BlockProtectCodeRequired = 00  MultiByteProgramAvailable = 01  BytesOfId = 03  MinBytePerProgRead = 01  NoVolStatusAvailable = 01   Reserve = 00
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00,  \  # GlobalProtectCode = 00  GlobalUnprotectCode = 00  BlockProtectCode = 00  BlockUnprotectCode = 00  DeviceSize = 00400000
    0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   \  # BlockEraseSize = 00001000  BlockProtectSize = 00000000
  }

  #
  # Add MXIC MX25U3235F Spi config
  #
  gH2OFlashDeviceMfrNameGuid.PcdMxic25u3235fSpiMfrName|"MXIC"
  gH2OFlashDevicePartNameGuid.PcdMxic25u3235fSpiPartName|"MX25U3235F"
  gH2OFlashDeviceGuid.PcdMxic25u3235fSpi|{ \ # MXIC  MX25U3235F
    0x03, 0x00, 0x00, 0x00, 0xc2, 0x25, 0x36, 0x00,  \  # DeviceType = 03  Id = 003625c2
    0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x04   \  # ExtId = 00000000  BlockSize = 0010  BlockCount = 0400
  }

  gH2OFlashDeviceConfigGuid.PcdMxic25u3235fSpiConfig|{ \ # MXIC  MX25U3235F
    0x28, 0x00, 0x00, 0x00, 0x9f, 0x20, 0x02, 0x01,  \  # Size = 00000028  ReadIdOp = 9F  EraseOp = 20  WriteOp = 02  WriteStatusOp = 01
    0x03, 0x05, 0x00, 0x00, 0x7c, 0xf2, 0x06, 0x06,  \  # ReadOp = 03  ReadStatusOp = 05  OpType = F27C  WriteEnablePrefix = 06  WriteStatusEnablePrefix = 06
    0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,  \  # GlobalProtectAvailable = 00  BLockProtectAvailable = 00  BlockProtectCodeRequired = 00  MultiByteProgramAvailable = 01  BytesOfId = 03  MinBytePerProgRead = 01  NoVolStatusAvailable = 01
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00,  \  # GlobalProtectCode = 00  GlobalUnprotectCode = 00  BlockProtectCode = 00  BlockUnprotectCode = 00  DeviceSize = 00400000
    0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   \  # BlockEraseSize = 00001000  BlockProtectSize = 00000000
  }
#[-start-160330-IB07400715-add]#
#[-start-161118-IB07400816-remove]#
#  #
#  # NUMONYXN N25Q128A11 Spi config
#  #
#  gH2OFlashDeviceMfrNameGuid.PcdNumonyxn25q128a11SpiMfrName|"NUMONYXN"
#  gH2OFlashDevicePartNameGuid.PcdNumonyxn25q128a11SpiPartName|"N25Q128A11"
#  gH2OFlashDeviceGuid.PcdNumonyxn25q128a11Spi|{ \ # MICRON N25Q128A11
#    0x03, 0x00, 0x00, 0x00, 0x20, 0xbb, 0x18, 0x00,  \  # DeviceType = 03  Id = 0018BB20
#    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01   \  # ExtId = 00000000  BlockSize = 0100  BlockCount = 0100
#  }
#
#  gH2OFlashDeviceConfigGuid.PcdNumonyxn25q128a11SpiConfig|{ \ # MICRON  N25Q128A11
#    0x28, 0x00, 0x00, 0x00, 0x9f, 0xd8, 0x02, 0x01,  \  # Size = 00000028  ReadIdOp = 9F  EraseOp = D8  WriteOp = 02  WriteStatusOp = 01
#    0x03, 0x05, 0x00, 0x00, 0x7c, 0xf2, 0x06, 0x06,  \  # ReadOp = 03  ReadStatusOp = 05  OpType = F27C  WriteEnablePrefix = 06  WriteStatusEnablePrefix = 06
#    0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,  \  # GlobalProtectAvailable = 00  BlockProtectAvailable = 00  BlockProtectCodeRequired = 00  MultiByteProgramAvailable = 01  BytesOfId = 03  MinBytePerProgRead = 01  NoVolStatusAvailable = 01
#    0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,  \  # GlobalProtectCode = 1C  GlobalUnprotectCode = 00  BlockProtectCode = 00  BlockUnprotectCode = 00  DeviceSize = 01000000
#    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00   \  # BlockEraseSize = 00010000  BlockProtectSize = 00000000
#  }
#[-end-161118-IB07400816-remove]#
#[-end-160330-IB07400715-add]#
#[-start-160704-IB07400752-add]#
!if $(LEGACY_IRQ_SUPPORT) == YES

  # PIRQ link value                               PIRQA, PIRQB, PIRQC, PIRQD, PIRQE, PIRQF, PIRQG, PIRQH (x)
  gChipsetPkgTokenSpaceGuid.PcdPirqLinkValueArray|{0x01,  0x02,  0x03,  0x04,  0x05,  0x06,  0x07,  0x08} #EndEntry

  gChipsetPkgTokenSpaceGuid.PcdVirtualBusTable|{  0x00, 0x13, 0x00, 0x04, \
                                                  0x00, 0x13, 0x01, 0x05, \
                                                  0x00, 0x13, 0x02, 0x06, \
                                                  0x00, 0x13, 0x03, 0x07, \
                                                  0x00, 0x14, 0x00, 0x08, \
                                                  0x00, 0x14, 0x01, 0x09}

  # =========================================================================================
  #
  # Refer FRC PciTree.asl, PeiScPolicyLib.c:mDirectIrqTable
  #
  # X:Dev 0x00 => 0x00, NPK,                PIRQA, PIRQN, PIRQN, PIRQN ==> APIC -> 16,24, N, N
  # V:Dev 0x02 => 0x10, Mobile IGFX,        PIRQD, PIRQN, PIRQN, PIRQN
  # V:Dev 0x03 => 0x18, IUNIT               PIRQF, PIRQN, PIRQN, PIRQN
  # X:Dev 0x0D => 0x68, PMC                 PIRQN, PIRQN, PIRQN, PIRQN ==> APIC ->  N,40, N, N
  # X:Dev 0x0E => 0x70, LPE Audio           PIRQA, PIRQN, PIRQN, PIRQN ==> APIC -> 25, N, N, N
  # V:Dev 0x0F => 0x78, CSE                 PIRQE, PIRQN, PIRQN, PIRQN
  # C:Dev 0x11 => 0x88, ISH                 PIRQA, PIRQN, PIRQN, PIRQN ==> APIC -> 26, N, N, N
  # V:Dev 0x12 => 0x90, SATA                PIRQD, PIRQN, PIRQN, PIRQN
  # V:Dev 0x13 => 0x98, PCIe 4 Lane,        PIRQG, PIRQH, PIRQE, PIRQF
  # V:Dev 0x14 => 0xA0, PCIe 2 Lane,        PIRQG, PIRQH, PIRQN, PIRQN
  # C:Dev 0x15 => 0xA8, XHCI/XDCI,          PIRQB, PIRQB, PIRQN, PIRQN ==> APIC -> 17,13, N, N
  # C:Dev 0x16 => 0xB0, I2C 0~3,            PIRQA, PIRQB, PIRQC, PIRQD ==> APIC -> 27,28,29,30
  # C:Dev 0x17 => 0xB8, I2C 4~7,            PIRQA, PIRQB, PIRQC, PIRQD ==> APIC -> 31,32,33,34
  # C:Dev 0x18 => 0xC0, UART 1~4,           PIRQA, PIRQB, PIRQC, PIRQD ==> APIC ->  4, 5, 6, 7
  # C:Dev 0x19 => 0xC8, SPI 1~3,            PIRQA, PIRQB, PIRQC, PIRQN ==> APIC -> 35,36,37, N
  # C:Dev 0x1B => 0xD8, SD Card,            PIRQA, PIRQN, PIRQN, PIRQN ==> APIC ->  3, N, N, N
  # C:Dev 0x1C => 0xE0, eMMC,               PIRQA, PIRQN, PIRQN, PIRQN ==> APIC -> 39, N, N, N
  # C:Dev 0x1E => 0xF0, SDIO,               PIRQA, PIRQN, PIRQN, PIRQN ==> APIC -> 42, N, N, N
  # V:Dev 0x1F => 0xF8, SMBus Controller,   PIRQE, PIRQN, PIRQN, PIRQN
  #
  # V:PCIe Root Port 0/0x13/0 => Bus 4,     PIRQG, PIRQH, PIRQE, PIRQF 
  # V:PCIe Root Port 0/0x13/1 => Bus 5,     PIRQH, PIRQE, PIRQF, PIRQG
  # V:PCIe Root Port 0/0x13/2 => Bus 6,     PIRQE, PIRQF, PIRQG, PIRQH
  # V:PCIe Root Port 0/0x13/3 => Bus 7,     PIRQF, PIRQG, PIRQH, PIRQE
  # V:PCIe Root Port 0/0x14/0 => Bus 8,     PIRQG, PIRQH, PIRQE, PIRQF 
  # V:PCIe Root Port 0/0x14/1 => Bus 9,     PIRQH, PIRQE, PIRQF, PIRQG
  #
  # =========================================================================================
  #
  # Bus, Dev,  INT#A,IrqMask,   INT#B,IrqMask,   INT#C,IrqMask,   INT#D,IrqMask,   Slot, Reserved, DevIpRegValue,ProgrammableIrq.
  #
  # ProgrammableIrq
  # This byte is provided for H2OIDE to identify is this device is a programmable device
  # 0x00: Enable,
  # 0xFF: Disable,
  # 0x01~0xFE: Reserved
  #

#[-start-170510-IB07400866-modify]#
  gChipsetPkgTokenSpaceGuid.PcdControllerDeviceIrqRoutingEntry| { \
  0x00, 0x10, 0x04, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, 0x00, \
  0x00, 0x18, 0x06, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, 0x00, \
  0x00, 0x78, 0x05, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, 0x00, \
  0x00, 0x90, 0x04, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, 0x00, \
  0x00, 0x98, 0x07, UINT16(0xDEB8), 0x08, UINT16(0xDEF8), 0x05, UINT16(0xDEF8), 0x06, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, 0x00, \
  0x00, 0xA0, 0x07, UINT16(0xDEB8), 0x08, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, 0x00, \
  0x00, 0xF8, 0x05, UINT16(0xDEF8), 0x00, UINT16(0xDEB8), 0x00, UINT16(0xDEF8), 0x00, UINT16(0xDEF8), 0x00, 0x00, UINT32(0xFFFFFFFF), 0x00, 0x00, \
  0x04, 0x00, 0x07, UINT16(0xDEB8), 0x08, UINT16(0xDEB8), 0x05, UINT16(0xDEB8), 0x06, UINT16(0xDEB8), 0x01, 0xFF, UINT32(0xFFFFFFFF), 0x00, 0x00, \
  0x05, 0x00, 0x08, UINT16(0xDEB8), 0x05, UINT16(0xDEB8), 0x06, UINT16(0xDEB8), 0x07, UINT16(0xDEB8), 0x02, 0xFF, UINT32(0xFFFFFFFF), 0x00, 0x00, \
  0x06, 0x00, 0x05, UINT16(0xDEB8), 0x06, UINT16(0xDEB8), 0x07, UINT16(0xDEB8), 0x08, UINT16(0xDEB8), 0x03, 0xFF, UINT32(0xFFFFFFFF), 0x00, 0x00, \
  0x07, 0x00, 0x06, UINT16(0xDEB8), 0x07, UINT16(0xDEB8), 0x08, UINT16(0xDEB8), 0x05, UINT16(0xDEB8), 0x04, 0xFF, UINT32(0xFFFFFFFF), 0x00, 0x00, \
  0x08, 0x00, 0x07, UINT16(0xDEB8), 0x08, UINT16(0xDEB8), 0x05, UINT16(0xDEB8), 0x06, UINT16(0xDEB8), 0x05, 0xFF, UINT32(0xFFFFFFFF), 0x00, 0x00, \
  0x09, 0x00, 0x08, UINT16(0xDEB8), 0x05, UINT16(0xDEB8), 0x06, UINT16(0xDEB8), 0x07, UINT16(0xDEB8), 0x06, 0xFF, UINT32(0xFFFFFFFF), 0x00, 0x00} #EndEntry
#[-end-170510-IB07400866-modify]#

#[-start-160714-IB07400759-modify]#
#[-start-160719-IB07400761-modify]#
  #
  # IRQ1 reseverd for Keyboard
  # IRQ9 reseverd for SCI
  # IRQ12 reseved for Mouse
  #
  gChipsetPkgTokenSpaceGuid.PcdIrqPoolTable|{10, 0x00, \ #IRQ10
                                             11, 0x00, \ #IRQ11
                                             11, 0x00, \ #IRQ11
                                             10, 0x00, \ #IRQ10
                                             11, 0x00}   #IRQ11
  gChipsetPkgTokenSpaceGuid.PcdPirqPriorityTable|{11, \# PIRQ A
                                                  0,  \# PIRQ B
                                                  0,  \# PIRQ C
                                                  10, \# PIRQ D
                                                  0,  \# PIRQ E
                                                  0,  \# PIRQ F
                                                  0,  \# PIRQ G
                                                  0}  #EndEntry
#[-end-160719-IB07400761-modify]#
#[-end-160714-IB07400759-modify]#
!endif                                                  
#[-end-160704-IB07400752-add]#  

#
# Components
#
[Components.$(PEI_ARCH)]
#[-start-190220-IB07401083-add]#
  #
  # Gen OBB Layout
  #
!if $(EFI_DEBUG) == NO  
  $(CHIPSET_PKG)/ObbLayout/ObbLayout.inf
!endif
#[-end-190220-IB07401083-add]#
#[-start-160329-IB07400711-add]#
!disable InsydeModulePkg/Universal/StatusCode/DdtStatusCodePei/DdtStatusCodePei.inf
!if gInsydeTokenSpaceGuid.PcdH2ODdtSupported
!if gInsydeTokenSpaceGuid.PcdStatusCodeUseDdt
  InsydeModulePkg/Universal/StatusCode/DdtStatusCodePei/DdtStatusCodePei.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Universal/StatusCode/DdtStatusCodePei
  }
!endif
!endif
#[-end-160329-IB07400711-add]#
#[-start-160614-IB07400744-add]#
!disable InsydeModulePkg/Bus/Pci/EhciPei/EhciPei.inf

# Crisis Recovery for SATA 
!disable InsydeModulePkg/Bus/Pci/IdeBusPei/IdeBusPei.inf
!disable InsydeModulePkg/Bus/Pci/AhciBusPei/AhciBusPei.inf
!if gChipsetPkgTokenSpaceGuid.PcdCrisisRecoverySataSupport
  InsydeModulePkg/Bus/Pci/AhciBusPei/AhciBusPei.inf
!endif

# Crisis Recovery for SDHC 
!disable InsydeModulePkg/Bus/Pci/SdhcPei/SdHostDriverPei.inf
!disable InsydeModulePkg/Bus/Sdhc/SdMmcPei/SdMmcDevicePei.inf
!if gInsydeTokenSpaceGuid.PcdH2OSdhcSupported
!if gChipsetPkgTokenSpaceGuid.PcdCrisisRecoverySdEmmcSupport
#[-start-170209-IB07400839-modify]#
  $(CHIPSET_PKG)/PchSdhcPei/PchSdhcPei.inf
#  InsydeModulePkg/Bus/Pci/SdhcPei/SdHostDriverPei.inf
#  InsydeModulePkg/Bus/Sdhc/SdMmcPei/SdMmcDevicePei.inf
  $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Bus/Pci/SdhcPei/SdHostDriverPei.inf
  $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Bus/Sdhc/SdMmcPei/SdMmcDevicePei.inf
#[-end-170209-IB07400839-modify]#
!endif
!endif

# Crisis Recovery for NVMe 
!disable InsydeModulePkg/Bus/Pci/NvmExpressPei/NvmExpressPei.inf
!if gInsydeTokenSpaceGuid.PcdH2ONvmeSupported
!if gChipsetPkgTokenSpaceGuid.PcdCrisisRecoveryNvmeSupport
  InsydeModulePkg/Bus/Pci/NvmExpressPei/NvmExpressPei.inf
!endif
!endif

# Crisis Recovery for CD/DVD
!disable InsydeModulePkg/Universal/Recovery/CdExpressPei/CdExpressPei.inf
!if gChipsetPkgTokenSpaceGuid.PcdCrisisRecoveryCdDvdSupport
#[-start-170208-IB07400839-modify]#
  InsydeModulePkg/Universal/Recovery/CdExpressPei/CdExpressPei.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Universal/Recovery/CdExpressPei
  }
#[-end-170208-IB07400839-modify]#
!endif
#[-end-160614-IB07400744-add]#
#[-start-160929-IB05400712-add]#
!if gInsydeTokenSpaceGuid.PcdH2OStatusCodeBeepSupported
  !disable InsydeModulePkg/Universal/StatusCode/BeepStatusCodePei/BeepStatusCodePei.inf
  InsydeModulePkg/Universal/StatusCode/BeepStatusCodePei/BeepStatusCodePei.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Universal/StatusCode/BeepStatusCodePei
  }
  $(CHIPSET_PKG)/CommonChipset/SpeakerPei/LegacySpeakerPei.inf
!endif
#[-end-160929-IB05400712-add]#
#[-start-170210-IB07400839-add]#
  !disable MdeModulePkg/Universal/PcatSingleSegmentPciCfg2Pei/PcatSingleSegmentPciCfg2Pei.inf
  MdeModulePkg/Universal/PcatSingleSegmentPciCfg2Pei/PcatSingleSegmentPciCfg2Pei.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/MdeModulePkg/Universal/PcatSingleSegmentPciCfg2Pei
  }
#[-end-170210-IB07400839-add]#

[Components.$(DXE_ARCH)]
#[-start-160329-IB07400711-add]#
  !disable MdeModulePkg/Core/Dxe/DxeMain.inf
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <SOURCE_OVERRIDE_PATH>
      MdeModulePkg/Override/Core/Dxe
#[-start-160820-IB07400772-add]#
#[-start-180914-IB07401008-modify]#
      $(CHIPSET_OVERRIDE)/MdeModulePkg/Core/Dxe
      $(CHIPSET_OVERRIDE)/MdeModulePkg/Override/Core/Dxe
      $(CHIPSET_OVERRIDE2)/MdeModulePkg/Core/Dxe
      $(CHIPSET_OVERRIDE2)/MdeModulePkg/Override/Core/Dxe
#[-end-180914-IB07401008-modify]#
#[-end-160820-IB07400772-add]#
    <PcdsPatchableInModule>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000046
#[-start-170106-IB07400830-add]#
!if $(PEI_MEMORY_CONSISTENCY_CHECK) == YES
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
!endif      
#[-end-170106-IB07400830-add]#
  }
  !disable MdeModulePkg/Core/PiSmmCore/PiSmmCore.inf
  MdeModulePkg/Core/PiSmmCore/PiSmmCore.inf {
    <PcdsPatchableInModule>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000046
  }
#[-start-160630-IB07400750-add]#
  !if gInsydeTokenSpaceGuid.PcdH2OCsmSupported
    $(CHIPSET_PKG)/UsbLegacyControlSmm/UsbLegacyControlSmm.inf # for legacy free suppport (no KBC)
  !endif
#[-end-160630-IB07400750-add]#
#[-end-160318-IB07400711-add]#
#[-start-160710-IB07400752-add]#
  !disable InsydeModulePkg/Csm/LegacyBiosDxe/LegacyBiosDxe.inf
  InsydeModulePkg/Csm/LegacyBiosDxe/LegacyBiosDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Csm/LegacyBiosDxe
  }
#[-start-160808-IB07400769-add]#
  $(CHIPSET_PKG)/ChipsetFeatureSmm/ChipsetFeatureSmm.inf {
#[-start-160820-IB07400772-modify]#
!if $(INSYDE_DEBUGGER) == YES AND $(H2O_DDT_DEBUG_IO) == Com
!else
#[-start-160924-IB07400789-modify]#
#[-start-161207-IB07400823-modify]#
!if $(APOLLOLAKE_CRB) == YES AND $(CHIPSET_DEBUG_FOR_RELEASE_BUILD) == YES
    #
    # Output debug message for CRB debug only
    #
    <LibraryClasses>
      SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf
      DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
      DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
      UefiBootServicesTableLib|$(CHIPSET_OVERRIDE2)/MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
      UefiRuntimeServicesTableLib|$(CHIPSET_OVERRIDE2)/MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
    <PcdsPatchableInModule>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000042
    <BuildOptions>
      MSFT:*_*_*_CC_FLAGS             = /UMDEPKG_NDEBUG
      GCC:*_*_*_CC_FLAGS             = -UMDEPKG_NDEBUG
!endif
#[-end-161207-IB07400823-modify]#
#[-end-160924-IB07400789-modify]#
!endif
#[-end-160820-IB07400772-modify]#
  }
#[-end-160808-IB07400769-add]#
#[-start-160923-IB07400788-add]#
  !disable InsydeModulePkg/Universal/CommonPolicy/BootOptionPolicyDxe/BootOptionPolicyDxe.inf
  InsydeModulePkg/Universal/CommonPolicy/BootOptionPolicyDxe/BootOptionPolicyDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Universal/CommonPolicy/BootOptionPolicyDxe
  }
#[-end-160923-IB07400788-add]#
#[-start-160923-IB07400789-add]#
  !disable InsydeModulePkg/Bus/Usb/UsbMouseDxe/UsbMouseDxe.inf
  !if gInsydeTokenSpaceGuid.PcdH2OUsbSupported
    InsydeModulePkg/Bus/Usb/UsbMouseDxe/UsbMouseDxe.inf {
      <SOURCE_OVERRIDE_PATH>
        $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Bus/Usb/UsbMouseDxe
    }
  !endif
  
  !disable InsydeModulePkg/Bus/Usb/UsbCoreDxe/UsbCoreDxe.inf
  InsydeModulePkg/Bus/Usb/UsbCoreDxe/UsbCoreDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Bus/Usb/UsbCoreDxe
  }
#[-end-160923-IB07400789-add]#
#[-start-160929-IB05400712-add]#
!if gInsydeTokenSpaceGuid.PcdH2OStatusCodeBeepSupported
  !disable InsydeModulePkg/Universal/StatusCode/BeepStatusCodeDxe/BeepStatusCodeDxe.inf
  InsydeModulePkg/Universal/StatusCode/BeepStatusCodeDxe/BeepStatusCodeDxe.inf {
    <BuildOptions>
      MSFT:*_*_*_CC_FLAGS             = /GL-
  }
  !disable InsydeModulePkg/Universal/StatusCode/BeepStatusCodeSmm/BeepStatusCodeSmm.inf
  InsydeModulePkg/Universal/StatusCode/BeepStatusCodeSmm/BeepStatusCodeSmm.inf {
    <BuildOptions>
      MSFT:*_*_*_CC_FLAGS             = /GL-
  }
!endif
#[-end-160929-IB05400712-add]#
#[-start-161015-IB07400797-add]#
  !disable InsydeModulePkg/Universal/CommonPolicy/LegacyBiosPlatformDxe/LegacyBiosPlatformDxe.inf
  InsydeModulePkg/Universal/CommonPolicy/LegacyBiosPlatformDxe/LegacyBiosPlatformDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Universal/CommonPolicy/LegacyBiosPlatformDxe
  }
#[-end-161015-IB07400797-add]#
#[-start-170124-IB07400837-add]#
  !disable InsydeModulePkg/Universal/Console/RotateScreenDxe/RotateScreenDxe.inf
!if gInsydeTokenSpaceGuid.PcdH2ORotateScreenSupported
  InsydeModulePkg/Universal/Console/RotateScreenDxe/RotateScreenDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Universal/Console/RotateScreenDxe
  }
!endif
#[-end-170124-IB07400837-add]#
#[-start-170316-IB07400848-add]#
#[-start-170629-IB07400882-modify]#
  !disable InsydeSetupPkg/Drivers/H2OFormBrowserDxe/H2OFormBrowserDxe.inf
!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserSupported
  InsydeSetupPkg/Drivers/H2OFormBrowserDxe/H2OFormBrowserDxe.inf {
#!if $(SEG_SNAP_SCU_SUPPORT) == YES
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeSetupPkg/Drivers/H2OFormBrowserDxe
#!endif
  }
!endif
#[-end-170629-IB07400882-modify]#
#[-end-170316-IB07400848-add]#
#[-start-170428-IB07400864-add]#
  !disable InsydeModulePkg/Bus/Isa/Ps2KeyboardDxe/Ps2keyboardDxe.inf
  InsydeModulePkg/Bus/Isa/Ps2KeyboardDxe/Ps2keyboardDxe.inf {
!if $(KSC_SIO_SUPPORT) == YES  
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Bus/Isa/Ps2KeyboardDxe
!endif
  }
#[-end-170428-IB07400864-add]#
#[-start-170504-IB07400864-add]#
!if $(GOP_DEBUG_MESSAGE_SUPPORT) == YES
  $(CHIPSET_PKG)\GopDebugDxe\GopDebugDxe.inf
!endif
#[-end-170504-IB07400864-add]#
#[-start-170608-IB07400873-add]#
  !disable InsydeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  InsydeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Universal/Console/GraphicsConsoleDxe
  }
#[-end-170608-IB07400873-add]#
#[-start-170626-IB07400880-add]#
  !disable InsydeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  InsydeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Bus/Usb/UsbBusDxe
  }
#[-end-170626-IB07400880-add]#
#[-start-170629-IB07400882-add]#
  !disable InsydeSetupPkg/Drivers/H2ODisplayEngineLocalMetroDxe/H2ODisplayEngineLocalMetroDxe.inf
!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalMetroDESupported
  InsydeSetupPkg/Drivers/H2ODisplayEngineLocalMetroDxe/H2ODisplayEngineLocalMetroDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeSetupPkg/Drivers/H2ODisplayEngineLocalMetroDxe
  }
!endif

  !disable InsydeSetupPkg/Drivers/H2ODisplayEngineLocalTextDxe/H2ODisplayEngineLocalTextDxe.inf
!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalTextDESupported
  InsydeSetupPkg/Drivers/H2ODisplayEngineLocalTextDxe/H2ODisplayEngineLocalTextDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeSetupPkg/Drivers/H2ODisplayEngineLocalTextDxe
  }
!endif
#[-end-170629-IB07400882-add]#
#[-start-170901-IB07400905-add]#
  !disable InsydeModulePkg/Universal/Security/StorageSecurityCommandDxe/StorageSecurityCommandDxe.inf
!if gInsydeTokenSpaceGuid.PcdH2OHddPasswordSupported
  InsydeModulePkg/Universal/Security/StorageSecurityCommandDxe/StorageSecurityCommandDxe.inf
!endif
#[-end-170901-IB07400905-add]#
#[-start-180821-IB16530003-add]#
  !disable InsydeModulePkg/Universal/CommonPolicy/CommonPciPlatformDxe/CommonPciPlatformDxe.inf
  InsydeModulePkg/Universal/CommonPolicy/CommonPciPlatformDxe/CommonPciPlatformDxe.inf {
    <SOURCE_OVERRIDE_PATH>
#[-start-180920-IB16530005-modify]#
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Universal/CommonPolicy/CommonPciPlatformDxe
#[-end-180920-IB16530005-modify]#
  }
#[-end-180821-IB16530003-add]#
#[-start-181001-IB07401020-add]#
  !disable InsydeModulePkg/Universal/Smbios/PnpSmm/PnpSmm.inf
  InsydeModulePkg/Universal/Smbios/PnpSmm/PnpSmm.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Universal/Smbios/PnpSmm
  }
#[-end-181001-IB07401020-add]#
#[-start-190321-IB07401093-add]#
  !disable InsydeModulePkg/Universal/Security/CryptoServiceRuntimeDxe/CryptoServiceRuntimeDxe.inf
  InsydeModulePkg/Universal/Security/CryptoServiceRuntimeDxe/CryptoServiceRuntimeDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_OVERRIDE2)/InsydeModulePkg/Universal/Security/CryptoServiceRuntimeDxe
  }
#[-end-190321-IB07401093-add]#
#[-start-190321-IB07401094-add]#
  $(CHIPSET_PKG)/SeedSupport/BLSeedTest/BLSeedTest.inf
  $(CHIPSET_PKG)/SeedSupport/SeedProtocolDxe/SeedProtocolDxe.inf
#[-end-190321-IB07401094-add]#

#
# BuildOptions
#
[BuildOptions]

[BuildOptions.Common.EDKII]

#[-end-160307-IB07400708-add]#

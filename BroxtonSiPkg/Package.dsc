## @file
#  RC Package Description file
#
#******************************************************************************
#* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************

[Defines]
DEFINE   PLATFORM_SI_PACKAGE        = $(PLATFORM_RC_PACKAGE)
#[-start-151126-IB02950555-modify]#
DEFINE   CLIENT_SI_PACKAGE          = ClientSiliconPkg
#[-end-151126-IB02950555-modify]#

[BuildOptions.common]

[PcdsFixedAtBuild]


[LibraryClasses]
#
# RC Common Library
#
!include $(PLATFORM_RC_PACKAGE)/SiPkgCommonLib.dsc

#[-start-160518-IB06720411-add]#
PchSerialIoUartLib|$(PLATFORM_RC_PACKAGE)/SouthCluster/Library/PeiDxeSmmPchSerialIoUartLib/PeiDxeSmmPchSerialIoUartLib.inf
PchSerialIoLib|$(PLATFORM_RC_PACKAGE)/SouthCluster/Library/PeiDxeSmmPchSerialIoLib/PeiDxeSmmPchSerialIoLib.inf
#[-end-160518-IB06720411-add]#

#[-start-160518-IB06720411-modify]#
[LibraryClasses.IA32.PEIM, LibraryClasses.IA32.PEI_CORE, LibraryClasses.IA32.SEC]
#[-end-160518-IB06720411-modify]#
#
# RC Pei Library
#
!include $(PLATFORM_RC_PACKAGE)/SiPkgPeiLib.dsc

#[-start-160518-IB06720411-modify]#
[LibraryClasses.Common.DXE_DRIVER, LibraryClasses.Common.DXE_CORE, LibraryClasses.Common.DXE_SMM_DRIVER, LibraryClasses.Common.SMM_CORE, LibraryClasses.Common.DXE_RUNTIME_DRIVER, LibraryClasses.Common.COMBINED_SMM_DXE]
#[-end-160518-IB06720411-modify]#
# 
# RC Dxe Library
#
!include $(PLATFORM_RC_PACKAGE)/SiPkgDxeLib.dsc

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  CseVariableStorageSelectorLib|$(PLATFORM_SI_PACKAGE)/Library/Private/DxeSmmCseVariableStorageSelectorLib/DxeCseVariableStorageSelectorLib.inf


[LibraryClasses.common.DXE_SMM_DRIVER]
#
# RC Smm Library
#
#!include $(PLATFORM_RC_PACKAGE)/ClientRefCodePkgSmmLib.dsc
#
# Libraries for build reference
#
#  SmmIoLib|$(PLATFORM_RC_PACKAGE)/$(PLATFORM_SOC)/CPU/SampleCode/Library/SmmIo/SmmIo.inf
#  SmmKscLib|$(PLATFORM_RC_PACKAGE)/$(PLATFORM_SOC)/CPU/SampleCode/Library/Ksc/Smm/SmmKscLib.inf
#  DTSHookLib|$(PLATFORM_RC_PACKAGE)/$(PLATFORM_SOC)/CPU/SampleCode/Library/DTSHookLib/Smm/DTSHookLib.inf
  CseVariableStorageSelectorLib|$(PLATFORM_SI_PACKAGE)/Library/Private/DxeSmmCseVariableStorageSelectorLib/SmmCseVariableStorageSelectorLib.inf

[Components.$(PEI_ARCH)]
  $(PLATFORM_RC_PACKAGE)/Cpu/SmmAccess/Pei/SmmAccess.inf
#[-start-170222-IB07400843-modify]#
  $(PLATFORM_RC_PACKAGE)/SiInit/Pei/SiInitPreMem.inf {
#[-start-170801-IB07400897-modify]#
!if $(MRC_EV_RMT_BUILD) == YES OR $(MRC_DEBUG_DISABLE) == NO
#[-end-170801-IB07400897-modify]#
    <LibraryClasses>
      DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
!endif
  }
#[-end-170222-IB07400843-modify]#
#[-start-170412-IB07400858-modify]#
  $(PLATFORM_RC_PACKAGE)/NorthCluster/BiosReservedMemoryInit/Pei/BiosReservedMemoryInit.inf {
    <LibraryClasses>
      DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  } 
#[-end-170412-IB07400858-modify]#
#[-start-161123-IB07250310-modify]#
!if (!gChipsetPkgTokenSpaceGuid.PcdFspEnabled) 
  $(PLATFORM_RC_PACKAGE)/NorthCluster/MemoryInit/MemoryInitAplk.inf {
    <PcdsPatchableInModule>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000046
    <BuildOptions>
    !if $(VP_BIOS_ENABLE) == YES
      *_*_IA32_CC_FLAGS       = -D_VP_=1
    !endif
    !if $(MRC_PMIXOR_ENABLE) == YES
      *_*_IA32_CC_FLAGS = -DMRC_PMIXOR_ENABLE=1
    !endif    
    !if $(MRC_DEBUG_DISABLE) == YES
      *_*_IA32_CC_FLAGS = -DMRC_DEBUG_DISABLE=1
    !else
#      MSFT:*_*_IA32_CC_FLAGS  = /GL-
#      INTEL:*_*_IA32_CC_FLAGS = /GL-
      GCC:*_*_IA32_CC_FLAGS   = -O0
      *_*_IA32_CC_FLAGS = -DMRC_DEBUG_DISABLE=0
    !endif
    !if $(SEC_ENABLE)==YES
      *_*_IA32_CC_FLAGS = -D SEC_SUPPORT_FLAG
    !endif
    !if $(MRC_EV_RMT_BUILD) == YES
      *_*_*_CC_FLAGS = -D MRC_EV_RMT_BUILD=1 -D DEBUG_MSG=1
    !endif  
#[-start-160324-IB07400711-add]#
#[-start-170801-IB07400897-modify]#
    <LibraryClasses>
      !if $(MRC_EV_RMT_BUILD) == NO AND $(MRC_DEBUG_DISABLE) == YES
        DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
      !endif  
#[-end-170801-IB07400897-modify]#
#[-end-160324-IB07400711-add]#
#[-start-160929-IB05400712-add]#
    <PcdsFixedAtBuild>
      !if gInsydeTokenSpaceGuid.PcdH2OStatusCodeBeepSupported
        gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x7
      !endif
#[-end-160929-IB05400712-add]#
  }
!endif
#[-end-161123-IB07250310-modify]#
     
  $(PLATFORM_RC_PACKAGE)/SiInit/Pei/SiInit.inf {
  <LibraryClasses>
     S3BootScriptLib|MdePkg/Library/BaseS3BootScriptLibNull/BaseS3BootScriptLibNull.inf
#[-start-160328-IB07400711-add]# 
!if $(EFI_DEBUG) == YES
!if $(H2O_DDT_DEBUG_IO) == Com
!else
     SerialPortLib|$(CHIPSET_PKG)/Library/BaseSerialPortLib/BaseSerialPortLib.inf
     DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
     DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
!endif     
#[-start-180824-IB07400999-add]#
!if $(H2O_EVENT_LOG_SUPPORTED) == YES
     DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
!endif
#[-end-180824-IB07400999-add]#
!endif     
#[-end-160328-IB07400711-add]#
  <PcdsFixedAtBuild>
     gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27     
  }
#[-start-160308-IB03090425-remove]#
#!if $(NPK_ENABLE) == YES
#  $(PLATFORM_RC_PACKAGE)/NorthCluster/NpkInit/Pei/NpkInitPei.inf
#!endif
#[-end-160308-IB03090425-remove]#
  #$(PLATFORM_RC_PACKAGE)/SouthCluster/Spi/ScSpiPeim.inf
  $(PLATFORM_RC_PACKAGE)/SouthCluster/SmmControl/Pei/SmmControl.inf
  #$(PLATFORM_RC_PACKAGE)/SouthCluster/Usb/Pei/ScUsb.inf
#[-start-170612-IB07400873-modify]#
!if $(GET_STAGE2_FROM_EMMC) == YES
#[-end-170612-IB07400873-modify]#
  $(PLATFORM_RC_PACKAGE)/SouthCluster/MmcMediaDevicePei/MmcMediaDevicePei.inf {
  !if $(TOOL_CHAIN_TAG) == GCC47
    <BuildOptions>
      GCC:*_*_*_CC_FLAGS = -DMDEPKG_NDEBUG
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0
    <PcdsPatchableInModule>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0
  !endif
    <LibraryClasses>
      DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  }
!endif
#[-start-171002-IB07400914-remove]#
#!if $(FTPM_ENABLE) == TRUE
#   $(PLATFORM_RC_PACKAGE)/Txe/fTPM/Pei/fTPMInitPei.inf
#!endif
#[-end-171002-IB07400914-remove]#

#[-start-160826-IB03090433-modify]#
#[-start-160812-IB07220128-add]#
!if gBxtRefCodePkgTokenSpaceGuid.PcdHostManagedVariables == TRUE
  $(PLATFORM_SI_PACKAGE)/VariableStorage/Pei/FvbVariableStoragePei/FvbVariableStoragePei.inf
!endif
  $(PLATFORM_SI_PACKAGE)/VariableStorage/Pei/CseVariableStoragePei/CseVariableStoragePei.inf {
    <PcdsPatchableInModule>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000103
    <LibraryClasses>
      DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
      HeciMsgLib|$(PLATFORM_SI_PACKAGE)/Txe/Library/HeciMsgLib/PeiHeciMsgLib.inf
#[-start-161205-IB11270171-add]#
      BaseForceRangeAccessLib|$(CHIPSET_PKG)/Library/BaseForceRangeAccess/BaseForceRangeAccess.inf
#[-end-161205-IB11270171-add]#
  }
#[-end-160812-IB07220128-add]#
#[-end-160826-IB03090433-modify]#

[Components.$(DXE_ARCH)]
#[-start-160510-IB08450345-modify]#
  $(PLATFORM_RC_PACKAGE)/SiInit/Dxe/SiInitDxe.inf {
    <LibraryClasses>
      SmbiosCpuLib|$(PLATFORM_RC_PACKAGE)/Cpu/Library/Private/DxeSmbiosCpuLib/DxeSmbiosCpuLib.inf
  }
#[-end-160510-IB08450345-modify]#
  $(PLATFORM_RC_PACKAGE)/Cpu/CpuInit/Dxe/CpuInitDxe.inf
  $(PLATFORM_RC_PACKAGE)/SouthCluster/ScInit/Dxe/ScInitDxeAplk.inf
  $(PLATFORM_RC_PACKAGE)/SouthCluster/ActiveBios/Dxe/ActiveBios.inf
#[-start-160302-IB07220045-modify]#
# !if $(USE_HPET_TIMER) == YES
  PcAtChipsetPkg/HpetTimerDxe/HpetTimerDxe.inf {
    <SOURCE_OVERRIDE_PATH>
      $(CHIPSET_PKG)/Override/PcAtChipsetPkg/HpetTimerDxe
  }
# !else
  $(PLATFORM_RC_PACKAGE)/SouthCluster/SmartTimer/Dxe/SmartTimer.inf
# !endif
#[-end-160302-IB07220045-modify]#
  $(PLATFORM_RC_PACKAGE)/SouthCluster/SmmControl/RuntimeDxe/SmmControl.inf
#[-start-151124-IB10860187-remove]#
# EdkCompatibilityPkg/Compatibility/SmmControl2OnSmmControlThunk/SmmControl2OnSmmControlThunk.inf #Remove when SmmControl.inf use gEfiSmmControl2ProtocolGuid to instead of gEfiSmmControlProtocolGuid
#[-end-151124-IB10860187-remove]#  
  $(PLATFORM_RC_PACKAGE)/SouthCluster/LegacyInterrupt/Dxe/LegacyInterrupt.inf
  $(PLATFORM_RC_PACKAGE)/SouthCluster/Reset/RuntimeDxe/ScReset.inf
  $(CHIPSET_PKG)/SmmSwDispatch2OnSmmSwDispatchThunk/SmmSwDispatch2OnSmmSwDispatchThunk.inf
  $(CHIPSET_PKG)/SmmSxDispatch2OnSmmSxDispatchThunk/SmmSxDispatch2OnSmmSxDispatchThunk.inf
  $(PLATFORM_RC_PACKAGE)/SouthCluster/ScSmiDispatcher/Smm/ScSmiDispatcher.inf
  $(PLATFORM_RC_PACKAGE)/SouthCluster/ScInit/Smm/ScInitSmm.inf
  $(PLATFORM_RC_PACKAGE)/SouthCluster/Spi/RuntimeDxe/ScSpiRuntime.inf {
    <LibraryClasses>
      PciLib|$(PLATFORM_RC_PACKAGE)/SouthCluster/Library/DxeRuntimePciLibPciExpress/DxeRuntimePciLibPciExpress.inf
  }
   $(PLATFORM_RC_PACKAGE)/SouthCluster/SampleCode/BiosWriteProtect/Smm/ScBiosWriteProtect.inf
   $(PLATFORM_RC_PACKAGE)/SouthCluster/Smbus/Dxe/PchSmbusDxe.inf
   $(PLATFORM_RC_PACKAGE)/SouthCluster/Smbus/Smm/PchSmbusSmm.inf
   $(PLATFORM_RC_PACKAGE)/NorthCluster/PciHostBridge/Dxe/PciHostBridge.inf
#[-start-151020-IB02950553-remove]#
#   $(PLATFORM_RC_PACKAGE)/NorthCluster/SystemAgent/SaInit/Dxe/SaInit.inf
#[-end-151020-IB02950553-remove]#
   $(PLATFORM_RC_PACKAGE)/NorthCluster/SaInit/Dxe/SaInit.inf   
   $(PLATFORM_RC_PACKAGE)/NorthCluster/AcpiTables/SaAcpiTables.inf

#[-start-160111-IB03090424-add]#
!if $(BDAT_SUPPORT_ENABLED) == YES
  $(PLATFORM_RC_PACKAGE)/NorthCluster/BdatDxe/BdatDxe.inf
!endif
#[-end-160111-IB03090424-add]#
#[-start-160218-IB03090424-modify]#
#[-start-160628-IB07400748-modify]#
 !if $(HG_ENABLE) == YES
   $(PLATFORM_SI_PACKAGE)/NorthCluster/AcpiTables/HybridGraphics/HgAcpiTables.inf
 !endif   
#[-end-160628-IB07400748-modify]#
#[-end-160218-IB03090424-modify]#
!if $(SEC_ENABLE) == YES
  $(PLATFORM_RC_PACKAGE)/Txe/Heci/Dxe/Hecidrv.inf {
    <BuildOptions>
    !if $(SEC_DEBUG_INFO_ENABLE) == YES
      *_*_*_CC_FLAGS      = -DSEC_DEBUG_INFO=1
    !else
      *_*_*_CC_FLAGS      = -DSEC_DEBUG_INFO=0
    !endif
  }
!endif
#[-start-171002-IB07400914-remove]#
#!if $(FTPM_ENABLE) == YES
#  //
#  //
# disabled due to collision of EFI_TREE_PHYSICAL_PRESENCE struct def
#  $(PLATFORM_RC_PACKAGE)/Txe/fTPM/Smm/FtpmSmm.inf
#!endif
#[-end-171002-IB07400914-remove]#

!if $(NVM_VARIABLE_ENABLE) == YES
#[-start-160714-IB07400759-modify]#
#[-start-160602-IB07220091-modify]#
  $(PLATFORM_RC_PACKAGE)/Txe/Heci/Smm/HeciSmm.inf
#[-end-160602-IB07220091-modify]#
#[-end-160714-IB07400759-modify]#
#[-start-161123-IB07250310-modify]#
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  $(PLATFORM_SI_PACKAGE)/Txe/Heci/Smm/HeciSmmRuntimeDxeFsp.inf
!else
  $(PLATFORM_RC_PACKAGE)/Txe/Heci/Smm/HeciSmmRuntimeDxe.inf
!endif
#[-end-161123-IB07250310-modify]#
!endif

  $(PLATFORM_RC_PACKAGE)/Cpu/PowerManagement/Dxe/PowerMgmtDxe.inf
  $(PLATFORM_RC_PACKAGE)/Cpu/PowerManagement/Smm/PowerMgmtSmm.inf
  $(PLATFORM_RC_PACKAGE)/Cpu/AcpiTables/CpuAcpiTables.inf
#!if $(HW_REDUCED_MODE_ENABLE) == YES
#  $(PLATFORM_RC_PACKAGE)/AcpiTables/AcpiTables.inf
#!else
#  $(PLATFORM_RC_PACKAGE)/AcpiTablesPCAT/AcpiTables.inf
#  $(PLATFORM_RC_PACKAGE)/AcpiTablesPCAT/PlatformSsdt/PlatformSsdt.inf
#!endif
#  $(PLATFORM_RC_PACKAGE)/NorthCluster/ISPDxe/ISPDxe.inf
#[-start-160901-IB07400777-modify]#
!if $(EMMC_DRIVER_ENABLE) == YES
  $(PLATFORM_RC_PACKAGE)/SouthCluster/SDControllerDxe/MmcHostDxe.inf
  $(PLATFORM_RC_PACKAGE)/SouthCluster/SDMediaDeviceDxe/MmcMediaDeviceDxe.inf
!endif

!if gInsydeTokenSpaceGuid.PcdH2OAhciSupported == 1 || gInsydeTokenSpaceGuid.PcdH2OIdeSupported == 1
  $(PLATFORM_RC_PACKAGE)/SouthCluster/SataController/Dxe/SataController.inf
!endif
#[-end-160901-IB07400777-modify]#
  $(PLATFORM_RC_PACKAGE)/Cpu/SmmAccess/Dxe/SmmAccess.inf
#  $(PLATFORM_RC_PACKAGE)/NorthCluster/SmBiosMemory/Dxe/SmBiosMemory.inf
#  $(PLATFORM_RC_PACKAGE)/SouthCluster/FotaDxe/FotaFwUpdateCapsule.inf

#
# HSTI
#
!if $(HSTI_ENABLE) == TRUE
$(PLATFORM_SI_PACKAGE)/Hsti/Dxe/HstiSiliconDxe.inf {
    <LibraryClasses>
      HstiLib | MdePkg/Library/DxeHstiLib/DxeHstiLib.inf
      Tpm2CommandLib | SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  }
!endif

#[-start-160812-IB07220128-add]#
!if gBxtRefCodePkgTokenSpaceGuid.PcdHostManagedVariables == TRUE
#[-start-160826-IB03090433-modify]#
    !if $(SMM_VARIABLE_ENABLE)
      $(PLATFORM_SI_PACKAGE)/VariableStorage/Dxe/FvbVariableStorageSmmRuntimeDxe/FvbVariableStorageSmm.inf {
        <PcdsPatchableInModule>
          gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000103
      }
    !else
      $(PLATFORM_SI_PACKAGE)/VariableStorage/Dxe/FvbVariableStorageSmmRuntimeDxe/FvbVariableStorageRuntimeDxe.inf {
        <PcdsPatchableInModule>
          gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000103
      }
    !endif
#[-end-160826-IB03090433-modify]#
!else

  $(PLATFORM_SI_PACKAGE)/VariableStorage/Dxe/CseVariableStorageSmmRuntimeDxe/CseVariableStorageSmmRuntimeDxe.inf {
    <LibraryClasses>
#[-start-161007-IB07400793-modify]#
!if $(X64_CONFIG) == YES
      BaseCryptLib|$(CHIPSET_PKG)/Override/InsydeModulePkg/Library/BaseCryptLib/SmmCryptLib.inf
!endif      
#[-end-161007-IB07400793-modify]#
  }

  $(PLATFORM_SI_PACKAGE)/VariableStorage/Dxe/CseVariableStorageSmmRuntimeDxe/CseVariableStorageRuntimeDxe.inf
!endif
#[-end-160812-IB07220128-add]#

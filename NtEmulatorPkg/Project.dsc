## @file
#  Platform Package Description file
#
#******************************************************************************
#* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************
!import NtEmulatorPkg/Package.dsc
!import ShellPkg/Package.dsc

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = NtEmulatorPkg
  PLATFORM_GUID                  = 7f48118c-2f97-4d39-972a-e836358b7433
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/$(PROJECT_PKG)
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Build/$(PROJECT_PKG)/Project.fdf

  #
  # Defines for default states.  These can be changed on the command line.
  # -D FLAG=VALUE
  #
  !include $(PROJECT_PKG)/Project.env
  DEFINE SECURE_BOOT_ENABLE      = FALSE
  RFC_LANGUAGES                  = "en-US;fr-FR;zh-TW;ja-JP"


################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses]
  #
  # ChipsetPkg OemSvc
  #
  BaseOemSvcChipsetLib|$(CHIPSET_PKG)/Library/BaseOemSvcChipsetLib/BaseOemSvcChipsetLib.inf
  BaseOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/BaseOemSvcChipsetLib/BaseOemSvcChipsetLibDefault.inf
  DxeOemSvcChipsetLib|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLib.inf
  DxeOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLibDefault.inf

  #
  # Project.dsc override OemSvc, need change $(PROJECT_PKG) => $(BOARD_PKG)
  # PROJECT_PKG mean NtEmulatorPkg
  #

  # override InsydeOemServicesPkg
  BaseOemSvcKernelLib|$(BOARD_PKG)/Library/BaseOemSvcKernelLib/BaseOemSvcKernelLib.inf
  DxeOemSvcKernelLib|$(BOARD_PKG)/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLib.inf

  # override ChipsetPkg
  BaseOemSvcChipsetLib|$(BOARD_PKG)/Library/BaseOemSvcChipsetLib/BaseOemSvcChipsetLib.inf

  #
  # Console Redirection library
  #
  CrVfrConfigLib|$(CHIPSET_PKG)/Library/CrVfrConfigLibNull/CrVfrConfigLibNull.inf
  CrConfigDefaultLib|$(CHIPSET_PKG)/Library/CrConfigDefaultLibNull/CrConfigDefaultLibNull.inf

  #
  # SetupDataProvider / ChipsetSvcDxe / SetupUtilityDxe relative library
  #
  PlatformVfrLib|$(CHIPSET_PKG)/UefiSetupUtilityDxe/PlatformVfrLib.inf
  TbtLib|$(CHIPSET_PKG)/Library/TbtLib/TbtLib.inf

  BaseInsydeChipsetLib|$(CHIPSET_PKG)/Library/BaseInsydeChipsetLib/BaseInsydeChipsetLib.inf

  MeDxeLib|$(PLATFORM_RC_PACKAGE)/Me/Library/MeLib/Dxe/MeDxeLib.inf
  MeChipsetLib|$(PLATFORM_RC_PACKAGE)/Me/Library/MeChipsetLib/MeChipsetLib.inf
  MmPciBaseLib|$(PLATFORM_RC_PACKAGE)/Library/MmPciBaseLib/MmPciBaseLib.inf
  PchPlatformLib|$(PLATFORM_RC_PACKAGE)/Pch/Library/PchPlatformLib/PchPlatformLib.inf
  RcKscLib|$(PLATFORM_RC_PACKAGE)/SampleCode/Library/RcKscLib/RcKscLib.inf
  AcpiGnvsInitLib|$(PLATFORM_RC_PACKAGE)/AcpiTables/SampleCode/Library/PlatformAcpiLib/AcpiGnvsInitLib.inf
  RapidStartDxeLib|$(PLATFORM_RC_PACKAGE)/RapidStart/SampleCode/Library/RapidStartPlatformLib/Dxe/RapidStartDxeLib.inf
  PttHeciDxeLib|$(PLATFORM_RC_PACKAGE)/Me/Library/PttHeciLib/Dxe/PttHeciDxeLib.inf
  CpuPlatformLib|$(PLATFORM_RC_PACKAGE)/Cpu/Library/CpuPlatformLib/CpuPlatformLib.inf

[LibraryClasses.common.PEIM]
  #
  # Project.dsc override OemSvc, need change $(PROJECT_PKG) => $(BOARD_PKG)
  # PROJECT_PKG mean NtEmulatorPkg
  #
  # override InsydeOemServicesPkg
  PeiOemSvcKernelLib|$(BOARD_PKG)/Library/PeiOemSvcKernelLib/PeiOemSvcKernelLib.inf

  #
  # SetupDataProvider relative library
  #
  KernelConfigLib|$(CHIPSET_PKG)/Library/PeiKernelConfigLib/PeiKernelConfigLib.inf

[PcdsFeatureFlag]
  gEfiNtEmulatorPkgTokenSpaceGuid.PcdEdk2FormBrowserSupported|FALSE
  gInsydeTokenSpaceGuid.PcdH2ONetworkSupported|FALSE

  gInsydeTokenSpaceGuid.PcdGraphicsSetupSupported|FALSE
  gInsydeTokenSpaceGuid.PcdH2OFormBrowserSupported|TRUE
  gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalTextDESupported|FALSE
  gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalMetroDESupported|TRUE
  gInsydeTokenSpaceGuid.PcdH2OSetupChangeDisplaySupported|TRUE

!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserSupported == TRUE
  gInsydeTokenSpaceGuid.PcdGraphicsSetupSupported|FALSE
!else
  gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalTextDESupported|FALSE
  gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalMetroDESupported|FALSE
!endif

  gInsydeTokenSpaceGuid.PcdEdkShellBinSupported|FALSE
  gInsydeTokenSpaceGuid.PcdShellBinSupported|TRUE
  gInsydeTokenSpaceGuid.PcdShellBuildSupported|FALSE
!errif (gInsydeTokenSpaceGuid.PcdEdkShellBinSupported == TRUE) and (gInsydeTokenSpaceGuid.PcdShellBinSupported == TRUE or gInsydeTokenSpaceGuid.PcdShellBuildSupported == TRUE), "Must NOT set both EdkShell and one of Shell PCDs as TRUE, otherwise BDS can not determine to use EDK Shell or EDK2 Shell."
!errif (gInsydeTokenSpaceGuid.PcdShellBinSupported == TRUE) and (gInsydeTokenSpaceGuid.PcdShellBuildSupported == TRUE), "Must NOT set both ShellBin and ShellBuild PCDs as TRUE, otherwise it will build fail by duplicated file GUID."

[PcdsFixedAtBuild]

  gInsydeTokenSpaceGuid.PcdScuFormsetGuidList|{ \
    GUID("C1E0B01A-607E-4B75-B8BB-0631ECFAACF2"), \ # Main
    GUID("C6D4769E-7F48-4D2A-98E9-87ADCCF35CCC"), \ # Avance
    GUID("5204F764-DF25-48A2-B337-9EC122B85E0D"), \ # Security
    GUID("A6712873-925F-46C6-90B4-A40F86A0917B"), \ # Power
    GUID("2D068309-12AC-45AB-9600-9187513CCDD8"), \ # Boot
    GUID("B6936426-FB04-4A7B-AA51-FD49397CDC01"), \ # Exit
    GUID("00000000-0000-0000-0000-000000000000")}
#[-start-151123-IB11270134-add]#
  gInsydeTokenSpaceGuid.PcdScuFormsetFlagList|{ \
    UINT8(0), \ # Main
    UINT8(0), \ # Avance
    UINT8(0), \ # Security
    UINT8(0), \ # Power
    UINT8(0), \ # Boot
    UINT8(0), \ # Exit
    UINT8(0xFF)}
#[-end-151123-IB11270134-add]#
!if gInsydeTokenSpaceGuid.PcdEdkShellBinSupported == FALSE
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdShellFile|{ 0x83, 0xA5, 0x04, 0x7C, 0x3E, 0x9E, 0x1C, 0x4F, 0xAD, 0x65, 0xE0, 0x52, 0x68, 0xD0, 0xB4, 0xD1 }
!endif

[Components]
  #
  # Below driver will provide some dummy protocol to solve driver dependency.
  #
  $(PROJECT_PKG)/Override/ChipsetWinNtPei/ChipsetWinNtPei.inf
  $(PROJECT_PKG)/Override/ChipsetWinNtDxe/ChipsetWinNtDxe.inf

  #
  # When library use $(PROJECT_PKG)\Project.dec in INF file, PROJECT_PKG is NtEmulatorPkg
  # it can't find header files in $(PROJECT_PKG)\Include.
  # it need add below line
  # *_*_*_CC_FLAGS = -I$(WORKSPACE)\$(BOARD_PKG)\Include
  #
  #

  #
  # If need override .h header file, please add the header file to below folder
  # *_*_*_CC_FLAGS = -I$(WORKSPACE)\$(PROJECT_PKG)\Override\$(CHIPSET_PKG)\Include
  #

  $(CHIPSET_PKG)/UefiSetupUtilityDxe/SetupUtilityDxe.inf {
    <BuildOptions>
       *_*_*_CC_FLAGS = -I$(WORKSPACE)\$(PROJECT_PKG)\Override\$(CHIPSET_PKG)\Include \
                        -I$(WORKSPACE)\$(PROJECT_PKG)\Override\$(PLATFORM_RC_PACKAGE)\Include \
                        -I$(WORKSPACE)\$(BOARD_PKG)\Include
  }

  $(CHIPSET_PKG)/ChipsetSvcDxe/ChipsetSvcDxe.inf {
    <BuildOptions>
       *_*_*_CC_FLAGS = -I$(WORKSPACE)\$(BOARD_PKG)\Include
    <LibraryClasses>
      NULL|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  }

  $(BOARD_PKG)/Library/BaseOemSvcKernelLib/BaseOemSvcKernelLib.inf {
    <BuildOptions>
       *_*_*_CC_FLAGS = -I$(WORKSPACE)\$(BOARD_PKG)\Include
  }

  $(BOARD_PKG)/Library/PeiOemSvcKernelLib/PeiOemSvcKernelLib.inf {
    <BuildOptions>
       *_*_*_CC_FLAGS = -I$(WORKSPACE)\$(BOARD_PKG)\Include
    <SOURCE_OVERRIDE_PATH>
       $(PROJECT_PKG)/Override/$(BOARD_PKG)/Library/PeiOemSvcKernelLib
  }

  $(BOARD_PKG)/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLib.inf {
    <BuildOptions>
       *_*_*_CC_FLAGS = -I$(WORKSPACE)\$(BOARD_PKG)\Include
    <SOURCE_OVERRIDE_PATH>
      $(PROJECT_PKG)/Override/$(BOARD_PKG)/Library/DxeOemSvcKernelLib
  }

  $(BOARD_PKG)/Library/BaseOemSvcChipsetLib/BaseOemSvcChipsetLib.inf {
    <BuildOptions>
       *_*_*_CC_FLAGS = -I$(WORKSPACE)\$(BOARD_PKG)\Include
    <SOURCE_OVERRIDE_PATH>
      $(PROJECT_PKG)/Override/$(BOARD_PKG)/Library/BaseOemSvcChipsetLib
  }

  $(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLib.inf {
    <BuildOptions>
       *_*_*_CC_FLAGS = -I$(WORKSPACE)\$(BOARD_PKG)\Include
  }

  $(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLibDefault.inf {
    <BuildOptions>
       *_*_*_CC_FLAGS = -I$(WORKSPACE)\$(BOARD_PKG)\Include
  }

  $(PLATFORM_RC_PACKAGE)/AcpiTables/SampleCode/Library/PlatformAcpiLib/AcpiGnvsInitLib.inf {
    <BuildOptions>
       *_*_*_CC_FLAGS = -I$(WORKSPACE)\$(BOARD_PKG)\Include
  }

  $(PLATFORM_RC_PACKAGE)/Pch/Library/PchPlatformLib/PchPlatformLib.inf {
    <BuildOptions>
      *_*_*_CC_FLAGS = -I$(WORKSPACE)\$(PROJECT_PKG)\Override\$(PLATFORM_RC_PACKAGE)\Include \
                       -I$(WORKSPACE)\$(PROJECT_PKG)\Override\$(PLATFORM_RC_PACKAGE)\Pch\Include
  }

[BuildOptions]
  MSFT:RELEASE_*_*_CC_FLAGS      = -D MDEPKG_NDEBUG
!if $(EFI_DEBUG) == NO
  MSFT:DEBUG_*_*_CC_FLAGS        = -D MDEPKG_NDEBUG
!endif


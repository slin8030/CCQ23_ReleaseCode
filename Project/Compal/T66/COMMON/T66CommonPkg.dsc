## @file
#  Platform Package Description file
#
#******************************************************************************
#* Copyright (c)  2016, Compal Electronics, Inc.. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[BuildOptions]
  !if $(WMI_WMAB_SUPPORT) == YES
    DEFINE WMI_WMAB_SUPPORT_OPTION    = /D WMI_WMAB_SUPPORT=1
  !else
    DEFINE WMI_WMAB_SUPPORT_OPTION    =
  !endif

  !if $(WMI_WMAC_SUPPORT) == YES
    DEFINE WMI_WMAC_SUPPORT_OPTION    = /D WMI_WMAC_SUPPORT=1
    DEFINE WMI_WMAC_SUPPORT_DIOP_PATH = /D DIOP_ASL=$(WMAC_DIO_PATH)
  !else
    DEFINE WMI_WMAC_SUPPORT_OPTION    =
    DEFINE WMI_WMAC_SUPPORT_DIOP_PATH =
  !endif

  !if $(WMI_WMAD_SUPPORT) == YES
    DEFINE WMI_WMAD_SUPPORT_OPTION    = /D WMI_WMAD_SUPPORT=1
  !else
    DEFINE WMI_WMAD_SUPPORT_OPTION    =
  !endif
  
  !if $(WMI_WMAE_SUPPORT) == YES
    DEFINE WMI_WMAE_SUPPORT_OPTION    = /D WMI_WMAE_SUPPORT=1
  !else
    DEFINE WMI_WMAE_SUPPORT_OPTION    =
  !endif  

  !if $(DEBUG_GPIO_SUPPORT) == YES
    DEFINE T66_DEBUG_GPIO_OPTION    = /D T66_DEBUG_GPIO=1
  !else
    DEFINE T66_DEBUG_GPIO_OPTION    = /D T66_DEBUG_GPIO=0
  !endif

  DEFINE EDK_EDKII_DSC_T66_BUILD_OPTIONS= $(WMI_WMAB_SUPPORT_OPTION) \
                                          $(WMI_WMAC_SUPPORT_OPTION) \
                                          $(WMI_WMAD_SUPPORT_OPTION) \
					  $(WMI_WMAE_SUPPORT_OPTION) \
                                          $(WMI_WMAC_SUPPORT_DIOP_PATH)

  *_*_X64_ASLPP_FLAGS = $(EDK_EDKII_DSC_T66_BUILD_OPTIONS)
  *_*_X64_ASLCC_FLAGS = $(EDK_EDKII_DSC_T66_BUILD_OPTIONS)
  *_*_*_CC_FLAGS = $(T66_DEBUG_GPIO_OPTION)

[LibraryClasses]
  EcSpiLib|$(T66_COMMON_PATH)/Library/EcLib/$(EC_CHIPSET_VENDOR)/EcSpiLib/EcSpiLib.inf

[LibraryClasses.common]

[LibraryClasses.common.SEC]

[LibraryClasses.common.PEIM]

  !disable PeiOemSvcChipsetLib|$(CHIPSET_PKG)/Library/PeiOemSvcChipsetLib/PeiOemSvcChipsetLib.inf
  PeiOemSvcChipsetLib|$(CHIPSET_PKG)/Library/PeiOemSvcChipsetLib/PeiOemSvcChipsetLib.inf {
  <SOURCE_OVERRIDE_PATH>
    $(T66_COMMON_PATH)/Override/$(CHIPSET_PKG)/Library/PeiOemSvcChipsetLib/
  }

  !disable PeiOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/PeiOemSvcChipsetLib/PeiOemSvcChipsetLibDefault.inf
  PeiOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/PeiOemSvcChipsetLib/PeiOemSvcChipsetLibDefault.inf {
  <SOURCE_OVERRIDE_PATH>
    $(T66_COMMON_PATH)/Override/$(CHIPSET_PKG)/Library/PeiOemSvcChipsetLib/
  }

[LibraryClasses.common.DXE_CORE]
  DxeProjectSvcLib|$(T66_COMMON_PATH)/Library/DxeProjectSvcLib/DxeProjectSvcLib.inf

[LibraryClasses.common.DXE_SMM_DRIVER]
  DxeProjectSvcLib|$(T66_COMMON_PATH)/Library/DxeProjectSvcLib/DxeProjectSvcLib.inf

[LibraryClasses.common.COMBINED_SMM_DXE]
  DxeProjectSvcLib|$(T66_COMMON_PATH)/Library/DxeProjectSvcLib/DxeProjectSvcLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  DxeProjectSvcLib|$(T66_COMMON_PATH)/Library/DxeProjectSvcLib/DxeProjectSvcLib.inf


  !disable DxeOemSvcChipsetLib|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLib.inf
  DxeOemSvcChipsetLib|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(T66_COMMON_PATH)/Override/$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/
  }

  !disable DxeOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLibDefault.inf
  DxeOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLibDefault.inf {
    <SOURCE_OVERRIDE_PATH>
      $(T66_COMMON_PATH)/Override/$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/
  }
#[PRJ]+ <<<< Modify PlatformGopPolicy GetPlatformLidStatus function (No Lid on HW design, alway return on).  
[LibraryClasses.common.DXE_DRIVER]
  DxeProjectSvcLib|$(T66_COMMON_PATH)/Library/DxeProjectSvcLib/DxeProjectSvcLib.inf


#[PRJ]+ >>>> Modify PlatformGopPolicy GetPlatformLidStatus function (No Lid on HW design, alway return on).
  !disable DxeOemSvcChipsetLib|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLib.inf
  DxeOemSvcChipsetLib|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(T66_COMMON_PATH)/Override/$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/
  }

  !disable DxeOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLibDefault.inf
  DxeOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLibDefault.inf {
    <SOURCE_OVERRIDE_PATH>
      $(T66_COMMON_PATH)/Override/$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/
  }
#[PRJ]+ <<<< Modify PlatformGopPolicy GetPlatformLidStatus function (No Lid on HW design, alway return on).  
[LibraryClasses.common.UEFI_DRIVER]
  DxeProjectSvcLib|$(T66_COMMON_PATH)/Library/DxeProjectSvcLib/DxeProjectSvcLib.inf


#[PRJ]+ >>>> Modify PlatformGopPolicy GetPlatformLidStatus function (No Lid on HW design, alway return on).
  !disable DxeOemSvcChipsetLib|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLib.inf
  DxeOemSvcChipsetLib|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(T66_COMMON_PATH)/Override/$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/
  }

  !disable DxeOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLibDefault.inf
  DxeOemSvcChipsetLibDefault|$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/DxeOemSvcChipsetLibDefault.inf {
    <SOURCE_OVERRIDE_PATH>
      $(T66_COMMON_PATH)/Override/$(CHIPSET_PKG)/Library/DxeOemSvcChipsetLib/
  }
#[PRJ]+ <<<< Modify PlatformGopPolicy GetPlatformLidStatus function (No Lid on HW design, alway return on).  
[LibraryClasses.common.UEFI_APPLICATION]
  DxeProjectSvcLib|$(T66_COMMON_PATH)/Library/DxeProjectSvcLib/DxeProjectSvcLib.inf

################################################################################
#
# Library Section - list of all EDK/Framework libraries
#
################################################################################
[Libraries.common]

[Libraries.IA32]

################################################################################
#
# Platform related components
#
################################################################################
[Components.$(PEI_ARCH)]

[Components.$(DXE_ARCH)]
#[PRJ]+ >>>> Modify for support  VirtualEEPROMVerifyTool and CMFCVerify 
  !disable $(CHIPSET_PKG)/AcpiTablesPCAT/AcpiTables.inf
  $(CHIPSET_PKG)/AcpiTablesPCAT/AcpiTables.inf{
     <SOURCE_OVERRIDE_PATH>
      $(PROJECT_PKG)/AcpiTablesDxe
     <SOURCE_OVERRIDE_PATH>
      $(T66_COMMON_PATH)/AcpiTablesPCAT/
     <BuildOptions>
          *_*_*_CC_FLAGS = -D COMPAL_ASL_SUPPORT
          *_*_*_ASLPP_FLAGS  = -D COMPAL_ASL_SUPPORT
  !if $(EC_PLATFORM_SETTING) == I2C
              *_*_*_CC_FLAGS      = /D EC_PLATFORM_SETTING=1
              *_*_*_ASLPP_FLAGS   = /D EC_PLATFORM_SETTING=1
  !endif
  !if $(EC_PLATFORM_SETTING) == LPC
              *_*_*_CC_FLAGS      = /D EC_PLATFORM_SETTING=0
              *_*_*_ASLPP_FLAGS   = /D EC_PLATFORM_SETTING=0
  !endif
  !if $(PROJECT_I2C_TOUCHPAD_ENABLE) == YES
          *_*_*_ASLPP_FLAGS  = -D PROJECT_I2C_TOUCHPAD_ENABLE
  !endif
  }
   !disable $(CHIPSET_PKG)/AcpiPlatform/AcpiPlatform.inf
   $(CHIPSET_PKG)/AcpiPlatform/AcpiPlatform.inf {
     <SOURCE_OVERRIDE_PATH>
      $(T66_COMMON_PATH)/Override/$(CHIPSET_PKG)/AcpiPlatform/
   }
#[PRJ]+ <<<< Modify for support  VirtualEEPROMVerifyTool and CMFCVerify 
  $(T66_COMMON_PATH)/Asl/T66WMI/T66WMISsdt.inf
  $(T66_COMMON_PATH)/T66Common/Dxe/WMI/T66WMIInit.inf

#[PRJ]+ >>>> Add T66 common code:T66ConfigDxe to update Smbios from CompalEEprom.
  $(T66_COMMON_PATH)/T66Common/T66Config/Dxe/T66ConfigDxe.inf
  $(T66_COMMON_PATH)/T66Common/T66Config/Smm/T66ConfigSmm.inf
#[PRJ]+ <<<< Add T66 common code:T66ConfigDxe to update Smbios from CompalEEprom. 

 
#[PRJ]+ >>>> Add T66 common utility
  !disable $(CHIPSET_PKG)/PlatformSmm/Platform.inf
  $(CHIPSET_PKG)/PlatformSmm/Platform.inf {
    <SOURCE_OVERRIDE_PATH>
      $(T66_COMMON_PATH)/Override/$(CHIPSET_PKG)/PlatformSmm/
  }
  $(T66_COMMON_PATH)/Tool/SideBand/SideBand.inf
  $(T66_COMMON_PATH)/Tool/BiosSet/BiosSet.inf
# Do ME Unlock for ME update
   $(T66_COMMON_PATH)/Tool/MeUnlock/MeUnlock.inf
  
#[PRJ]+ <<<< Add T66 common utility
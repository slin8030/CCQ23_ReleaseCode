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

[LibraryClasses.common.PEI_CORE]

[LibraryClasses.common.DXE_CORE]

[LibraryClasses.common.DXE_SMM_DRIVER]

[LibraryClasses.common.COMBINED_SMM_DXE]

[LibraryClasses.common.DXE_RUNTIME_DRIVER]

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

#[PRJ]+ >>>> Add T66 common code:T66ConfigDxe to update Smbios from CompalEEprom.
  $(T66_COMMON_PATH)/T66Common/T66Config/Dxe/T66ConfigDxe.inf
#[PRJ]+ <<<< Add T66 common code:T66ConfigDxe to update Smbios from CompalEEprom.  
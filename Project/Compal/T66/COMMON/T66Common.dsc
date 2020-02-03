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

[LibraryClasses.common.DXE_DRIVER]

[LibraryClasses.common.UEFI_DRIVER]

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

## @file
# Insyde H2oUve package project build description file.
#
#******************************************************************************
#* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************
#

[Defines]
#[-start-170110-IB07400831-modify]#
  DEFINE  INSYDE_H2OUVE_PKG          = InsydeH2oUvePkg
#[-end-170110-IB07400831-modify]#

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses]

[LibraryClasses.common.PEIM]

[LibraryClasses.common.DXE_DRIVER]

[LibraryClasses.common.DXE_SMM_DRIVER]
  DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
  SmmOemSvcH2oUveLib|$(INSYDE_H2OUVE_PKG)/Library/SmmOemSvcH2oUveLib/SmmOemSvcH2oUveLib.inf

[LibraryClasses.common.COMBINED_SMM_DXE]
  SmmOemSvcH2oUveLib|$(INSYDE_H2OUVE_PKG)/Library/SmmOemSvcH2oUveLib/SmmOemSvcH2oUveLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]

[LibraryClasses.common.UEFI_DRIVER]

[LibraryClasses.common.UEFI_APPLICATION]

[Components.$(PEI_ARCH)]

[Components.$(DXE_ARCH)]
  $(INSYDE_H2OUVE_PKG)/ConfigUtility/H2oUveConfigUtilDxe/H2oUveConfigUtilDxe.inf
!if gH2oUvePkgTokenSpaceGuid.PcdSampleFormSupported
  $(INSYDE_H2OUVE_PKG)/ConfigUtility/H2oUveSampleFormUtility/H2oUveSampleFormUtil.inf
!endif  
  $(INSYDE_H2OUVE_PKG)/VariableEditDxe/VariableEditDxe.inf
  $(INSYDE_H2OUVE_PKG)/VariableEditSmm/VariableEditSmm.inf

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]

[PcdsFixedAtBuild]
#
#Register Ihisi sub function table list.
#Table struct define {CmdNumber, AsciiFuncGuid, Priority}
# UINT8(CmdNumber), Char8[20](AsciiFuncGuid), UINT8(Priority)
##================  ========================  ===============
gH2oUvePkgTokenSpaceGuid.PcdH2oUveIhisiRegisterTable|{ \
  # Register IHISI AH=52h ()
  UINT8(0x52),      "S52OemH2oUveSmiSvc0",    UINT8(0x80), \

  # Register IHISI AH=53h ()
  UINT8(0x53),      "S53OemH2oUveSmiSvc1",    UINT8(0x80)  }


[PcdsDynamicDefault]
  
[PcdsDynamicExDefault]
  #
  # Use Advanced formset GUID as default so that the H2oUve Config Utility page could be put under Advanced menu.
  #
  gH2oUvePkgTokenSpaceGuid.PcdH2oUveClassGuid|{GUID("C6D4769E-7F48-4D2A-98E9-87ADCCF35CCC")}

[PcdsFixedAtBuild.IPF]

[PcdsPatchableInModule]


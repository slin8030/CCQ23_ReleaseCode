## @file
#  Platform Package Description file
#
#******************************************************************************
#* Copyright (c) 2012 - 2015, Insyde Software Corporation. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************

[Defines]

[Libraries]

[Libraries.IA32]

[Libraries.X64]

[LibraryClasses]
!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserSupported
  LayoutLib|InsydeSetupPkg/Library/LayoutSupportLib/LayoutSupportLib.inf
  H2ODisplayEngineLib|InsydeSetupPkg/Library/H2ODisplayEngineLib/H2ODisplayEngineLib.inf
!endif

[LibraryClasses.common.SEC]

[LibraryClasses.common.PEI_CORE]

[LibraryClasses.common.PEIM]

[LibraryClasses.common.DXE_CORE]

[LibraryClasses.common.DXE_RUNTIME_DRIVER]

[LibraryClasses.common.UEFI_DRIVER]

[LibraryClasses.common.DXE_DRIVER]

[LibraryClasses.common.DXE_SMM_DRIVER]

[LibraryClasses.common.COMBINED_SMM_DXE]

[LibraryClasses.common.SMM_CORE]

[LibraryClasses.common.UEFI_APPLICATION]

[PcdsFeatureFlag]

[PcdsFixedAtBuild]

[PcdsDynamicDefault]

[Components.$(PEI_ARCH)]

[Components.$(DXE_ARCH)]

!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserSupported

  InsydeSetupPkg/Drivers/DisplayTypeDxe/DisplayTypeDxe.inf
  InsydeSetupPkg/Drivers/HiiLayoutPkgDxe/HiiLayoutPkgDxe.inf
  InsydeSetupPkg/Drivers/H2OFormBrowserDxe/H2OFormBrowserDxe.inf
!if gInsydeTokenSpaceGuid.PcdH2OSetupChangeDisplaySupported
  InsydeSetupPkg/Drivers/H2OSetupChangeDxe/H2OSetupChangeDxe.inf
  InsydeSetupPkg/Drivers/H2OSetupChangeSmm/H2OSetupChangeSmm.inf
!endif

!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalTextDESupported
  InsydeSetupPkg/Drivers/H2ODisplayEngineLocalTextDxe/H2ODisplayEngineLocalTextDxe.inf
!endif

!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalMetroDESupported
  InsydeModulePkg/Universal/UserInterface/MicrowindowsDxe/MicrowindowsDxe.inf {
    <LibraryClasses>
       TimerLib|InsydeModulePkg/Library/CpuTimerLib/CpuTimerLib.inf
  }
  InsydeSetupPkg/Drivers/H2ODisplayEngineLocalMetroDxe/H2ODisplayEngineLocalMetroDxe.inf
!endif

!endif

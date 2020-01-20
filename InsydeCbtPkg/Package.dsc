## @file
# H2O CPU Branch Trace package project build description file.
#
#******************************************************************************
#* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************
#

[Defines]

[LibraryClasses]

[LibraryClasses.common.PEIM]
#[-start-150729-IB10130224-add]#
  PeiBtsLogSaveLib|InsydeCbtPkg/Library/PeiBtsLogSaveLib/PeiBtsLogSaveLib.inf
  PeiBtsLib|InsydeCbtPkg/Library/PeiBtsLib/PeiBtsLib.inf
#[-end-150729-IB10130224-add]#

[LibraryClasses.common.DXE_DRIVER]
#[-start-150729-IB10130224-add]#
  DxeBtsLogSaveLib|InsydeCbtPkg/Library/DxeBtsLogSaveLib/DxeBtsLogSaveLib.inf
  DxeBtsLib|InsydeCbtPkg/Library/DxeBtsLib/DxeBtsLib.inf
#[-end-150729-IB10130224-add]#

[LibraryClasses.common.DXE_SMM_DRIVER]

[LibraryClasses.common.DXE_RUNTIME_DRIVER]

[LibraryClasses.common.UEFI_DRIVER]

[LibraryClasses.common.UEFI_APPLICATION]

[PcdsFeatureFlag]
  
[PcdsFixedAtBuild]

[PcdsFixedAtBuild.IPF]

[PcdsPatchableInModule]

[PcdsDynamicExDefault]
  gInsydeCbtTokenSpaceGuid.PcdH2OBtsImageInfoBufferSize|0x3800
  gInsydeCbtTokenSpaceGuid.PcdH2OBtsPeiBufferPageSize|0x200
  gInsydeCbtTokenSpaceGuid.PcdH2OBtsDxeBufferPageSize|0x400

[Components.$(PEI_ARCH)]
!if gInsydeCbtTokenSpaceGuid.PcdH2OCBTSupported
  InsydeCbtPkg/Btsinit/BtsInitPei/BtsInitPei.inf
!endif

[Components.$(DXE_ARCH)]
!if gInsydeCbtTokenSpaceGuid.PcdH2OCBTSupported
  InsydeCbtPkg/Btsinit/BtsInitDxe/BtsInitDxe.inf
  InsydeCbtPkg/Btsinit/BtsinitSmm/BtsinitSmm.inf
!endif


[BuildOptions.Common.EDKII]
!if gInsydeCbtTokenSpaceGuid.PcdH2OCBTSupported
#[-start-170406-IB07400855-modify]#
  #
  # Please add Od to your debug module/driver
  #
  #MSFT:*_*_*_CC_FLAGS = $(CC_FLAGS) /Od
#[-end-170406-IB07400855-modify]#
!endif
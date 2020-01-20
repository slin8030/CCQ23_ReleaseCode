## @file
#  Platform Package Description file
#
#******************************************************************************
#* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************

[Defines]

[LibraryClasses]
  LanguageLib|EdkCompatibilityPkg/Compatibility/Library/UefiLanguageLib/UefiLanguageLib.inf

[Components.$(PEI_ARCH)]
  EdkCompatibilityPkg/Compatibility/AcpiVariableHobOnSmramReserveHobThunk/AcpiVariableHobOnSmramReserveHobThunk.inf

[Components.$(DXE_ARCH)]
  EdkCompatibilityPkg/Compatibility/BootScriptSaveOnS3SaveStateThunk/BootScriptSaveOnS3SaveStateThunk.inf
#   EdkCompatibilityPkg/Compatibility/FrameworkHiiOnUefiHiiThunk/FrameworkHiiOnUefiHiiThunk.inf
  EdkCompatibilityPkg/Compatibility/FvOnFv2Thunk/FvOnFv2Thunk.inf

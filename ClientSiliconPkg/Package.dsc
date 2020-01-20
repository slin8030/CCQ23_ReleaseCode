## @file
#  Platform Package Description file
#
#******************************************************************************
#* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************

[Defines]


################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################

[Defines]
  PLATFORM_NAME = ClientSiliconPkg
  PLATFORM_GUID = E7419982-BFA6-48e7-9986-7C554133CC6B
  PLATFORM_VERSION = 0.4
  DSC_SPECIFICATION = 0x00010005
  OUTPUT_DIRECTORY = Build/ClientSiliconPkg
  SUPPORTED_ARCHITECTURES = IA32|X64
  BUILD_TARGETS = DEBUG|RELEASE
  SKUID_IDENTIFIER = DEFAULT
[LibraryClasses.common]
ConfigBlockLib|ClientSiliconPkg/Library/BaseConfigBlockLib/BaseConfigBlockLib.inf
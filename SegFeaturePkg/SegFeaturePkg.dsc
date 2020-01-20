## @file
#  Package dscription file for SegFeaturePkg
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

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  !include SegFeaturePkg/Package.env
  PLATFORM_NAME                  = SegFeaturePkg
  PLATFORM_GUID                  = 061A861E-4C00-49d2-BA60-C0BD6EA14EB7  
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/SegFeaturePkg
  SUPPORTED_ARCHITECTURES        = IA32|X64|ARM|AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[BuildOptions]
  GCC:*_*_*_CC_FLAGS             = -DMDEPKG_NDEBUG
  INTEL:*_*_*_CC_FLAGS           = /D MDEPKG_NDEBUG
  MSFT:RELEASE_*_*_CC_FLAGS      = /D MDEPKG_NDEBUG

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses.ARM, LibraryClasses.AARCH64]

[LibraryClasses.ARM.PEIM]

[LibraryClasses.IA32, LibraryClasses.X64]

[LibraryClasses]

[LibraryClasses.common.PEIM]

[LibraryClasses.common.DXE_DRIVER]

[LibraryClasses.common.DXE_RUNTIME_DRIVER]

[LibraryClasses.common.UEFI_DRIVER]

[LibraryClasses.common.DXE_SMM_DRIVER]

[LibraryClasses.common.COMBINED_SMM_DXE]

[LibraryClasses.common.UEFI_APPLICATION]

[LibraryClasses.common.USER_DEFINED]

[PcdsFeatureFlag]

[PcdsFixedAtBuild]

[PcdsFixedAtBuild.IPF]

[PcdsPatchableInModule]

[PcdsDynamicDefault]

[PcdsDynamicExDefault]

[Components]

[Components.X64]

[Components.IA32]

[Components.ARM, Components.AARCH64]


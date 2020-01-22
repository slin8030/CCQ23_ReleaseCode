## @file
#  Project Makefile
#
#******************************************************************************
#* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************

#
# Build mode customization
# -q: Quiet mode   - disable all messages except fatal errors, module based messages.
#                    this is default build mode if BUILD_MODE is not specified
# -s: Silent mode  - file based messages
# -v: Verbose mode - Turn on verbose output with informational messages printed
#
#BUILD_MODE     = -s

#
# Build report customization
# Below is the default build report setting if BUILD_REPORT is not specified
#BUILD_REPORT = -Y PCD -Y DEPEX -Y LIBRARY -y $(WORKSPACE)/Build/$(PROJECT_PKG)/BuildReport.txt

#
# Add additional project build options here
#
#PROJECT_BUILD_OPTIONS =

BUILD_REPORT = -Y PCD -Y DEPEX -Y LIBRARY -y $(WORKSPACE)/Build/$(PROJECT_PKG)/BuildReport.txt

!include $(WORKSPACE)\BaseTools\Conf\Makefile

#[-start-160711-IB07220111-modify]#
#
# There are errors in BaseTools' Makefile. The following targets are replacement
# for those target.
#
uefi64debug:
  @$(MAKE) debug BUILD_TARGET=$@ BUILD_TARGET_OPTIONS="$(EFI_DEBUG)"

efidebugddt64:
  @$(MAKE) debug BUILD_TARGET=$@ BUILD_TARGET_OPTIONS="$(EFI_DEBUG_DDT)"

uefi32rmt:
    @$(MAKE) debug BUILD_TARGET=$@ BUILD_TARGET_OPTIONS="$(EFI_DEBUG)"

uefi64rmt:
    @$(MAKE) debug BUILD_TARGET=$@ BUILD_TARGET_OPTIONS="$(EFI_DEBUG)"
#[-end-160711-IB07220111-modify]#

#[-start-151122-IB07220012-add]#
rvp2:
  set RVP2=YES
#[-end-151122-IB07220012-add]#
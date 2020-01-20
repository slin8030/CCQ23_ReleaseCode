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
#[-start-170331-IB07400855-add]#
CBT_BUILD    = -D SYMBOLIC_DEBUG=YES -D H2OCBT_SUPPORT=YES
#[-end-170331-IB07400855-add]#

#[-start-161123-IB07250310-modify]#
#[-start-170915-IB07400910-modify]#
!include $(WORKSPACE)\BaseTools\Conf\Makefile
#[-end-170915-IB07400910-modify]#
#[-end-161123-IB07250310-modify]#

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
#[-start-160223-IB07400702-remove]#
#rvp2:
#  set RVP2=YES
#[-end-160223-IB07400702-remove]#
#[-end-151122-IB07220012-add]#

#[-start-160328-IB07400711-add]#
## @BuildTarget "IA32", DEBUG, Debug build with EFI debug messages via serial port for x86 32-bit CPU Arch. 
uefi32edbg:
  @$(MAKE) debug BUILD_TARGET=$@ BUILD_TARGET_OPTIONS="$(EFI_DEBUG)"
 
## @BuildTarget "IA32 X64", DEBUG, Debug build with EFI debug messages via serial port for x86 64-bit CPU Arch.
uefi64edbg:
  @$(MAKE) debug BUILD_TARGET=$@ BUILD_TARGET_OPTIONS="$(EFI_DEBUG)"
  
## @BuildTarget "IA32", DEBUG, Debug build with Insyde Debugger and EFI debug messages via ddt and serial port for x86 32-bit CPU Arch.
uefi32ddtedbg:
  @$(MAKE) debug BUILD_TARGET=$@ BUILD_TARGET_OPTIONS="$(EFI_DEBUG_DDT)"

## @BuildTarget "IA32 X64", DEBUG, Debug build with Insyde Debugger and EFI debug messages via ddt and serial port for x86 64-bit CPU Arch.
uefi64ddtedbg:
  @$(MAKE) debug BUILD_TARGET=$@ BUILD_TARGET_OPTIONS="$(EFI_DEBUG_DDT)"
  
## @BuildTarget "IA32", RELEASE, Release build for x86 32-bit CPU Arch. (without time stamp)
uefi32nots:
  @$(MAKE) release BUILD_TARGET=$@
## @BuildTarget "IA32 X64", RELEASE, Release build for x86 64-bit CPU Arch. (without time stamp)
uefi64nots:
  @$(MAKE) release BUILD_TARGET=$@
#[-end-160328-IB07400711-add]#
#[-start-160822-IB07400773-add]#
## @BuildTarget "IA32", RELEASE, Release build with secure flash enabled for x86 32-bit CPU Arch. (without time stamp)
uefi32sf:
  @$(MAKE) release BUILD_TARGET=$@
## @BuildTarget "IA32 X64", RELEASE, Release build with secure flash enabled for x86 64-bit CPU Arch. (without time stamp)
uefi64sf:
  @$(MAKE) release BUILD_TARGET=$@
#[-start-170407-IB07400856-remove]#
### @BuildTarget "IA32", RELEASE, Release build with secure flash enabled with ESP support for x86 32-bit CPU Arch. (without time stamp)
#uefi32sfesp:
#  @$(MAKE) release BUILD_TARGET=$@
### @BuildTarget "IA32 X64", RELEASE, Release build with secure flash enabled with ESP support for x86 64-bit CPU Arch. (without time stamp)
#uefi64sfesp:
#  @$(MAKE) release BUILD_TARGET=$@
##[-end-160822-IB07400773-add]#
#[-end-170407-IB07400856-remove]#
#[-start-170331-IB07400855-add]#
uefi32cbt:
  @$(MAKE) debug BUILD_TARGET=$@ BUILD_TARGET_OPTIONS="$(CBT_BUILD)"

uefi64cbt:
  @$(MAKE) debug BUILD_TARGET=$@ BUILD_TARGET_OPTIONS="$(CBT_BUILD)"
#[-end-170331-IB07400855-add]#
#[-start-181120-IB07401039-add]#
uefi32rvs:
  @$(MAKE) release BUILD_TARGET=$@
  
uefi64rvs:
  @$(MAKE) release BUILD_TARGET=$@
#[-end-181120-IB07401039-add]#
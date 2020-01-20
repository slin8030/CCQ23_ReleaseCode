## @file
#  Component description file for the Broxton RC PEI drivers.
#
# @copyright
#  INTEL CONFIDENTIAL
#  Copyright 2004 - 2016 Intel Corporation.
#
#  The source code contained or described herein and all documents related to the
#  source code ("Material") are owned by Intel Corporation or its suppliers or
#  licensors. Title to the Material remains with Intel Corporation or its suppliers
#  and licensors. The Material may contain trade secrets and proprietary and
#  confidential information of Intel Corporation and its suppliers and licensors,
#  and is protected by worldwide copyright and trade secret laws and treaty
#  provisions. No part of the Material may be used, copied, reproduced, modified,
#  published, uploaded, posted, transmitted, distributed, or disclosed in any way
#  without Intel's prior express written permission.
#
#  No license under any patent, copyright, trade secret or other intellectual
#  property right is granted to or conferred upon you by disclosure or delivery
#  of the Materials, either expressly, by implication, inducement, estoppel or
#  otherwise. Any license under such intellectual property rights must be
#  express and approved by Intel in writing.
#
#  Unless otherwise agreed by Intel in writing, you may not remove or alter
#  this notice or any other notice embedded in Materials by Intel or
#  Intel's suppliers or licensors in any way.
#
#  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
#  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
#  the terms of your license agreement with Intel or your vendor. This file may
#  be modified by the user, subject to additional terms of the license agreement.
#
# @par Specification Reference:
#
# @par Glossary:
###



#
# Common
#

#
# Sinit
#
!if ($(TABLET_PF_ENABLE) == 1)
!if ($(VP_BIOS_ENABLE) == NO)
  $(PLATFORM_SI_PACKAGE)/SiInit/Pei/SiInit.inf {
    <LibraryClasses>
  !if $(TABLET_PF_BUILD) == YES
      CpuS3Lib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuS3LibNull/PeiCpuS3LibNull.inf
  !endif
  }
!else
  $(PLATFORM_SI_PACKAGE)/SiInit/Pei/SiInitSimics.inf {
    <LibraryClasses>
  !if $(TABLET_PF_BUILD) == YES
      CpuS3Lib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuS3LibNull/PeiCpuS3LibNull.inf
  !endif
  }
!endif
!else
  $(PLATFORM_SI_PACKAGE)/SiInit/Pei/SiInit.inf {
    <LibraryClasses>
  !if $(TABLET_PF_BUILD) == YES
      CpuS3Lib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuS3LibNull/PeiCpuS3LibNull.inf
  !endif

  !if $(TARGET) != RELEASE
      DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
  !endif
  }
!endif
#
# NC
#

#
# SC
#

#
# CPU
#

  $(PLATFORM_SI_PACKAGE)/Cpu/SmmAccess/Pei/SmmAccess.inf


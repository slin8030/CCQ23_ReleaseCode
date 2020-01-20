## @file
#  Component description file for the Broxton RC PEI libraries.
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
# RC Pei Library
#


#
# Common
#
  CseVariableStorageSelectorLib|$(PLATFORM_SI_PACKAGE)/Library/Private/PeiCseVariableStorageSelectorLib/PeiCseVariableStorageSelectorLib.inf
  BootMediaLib|$(PLATFORM_SI_PACKAGE)/Library/PeiBootMediaLib/PeiBootMediaLib.inf
#[-start-151126-IB02950555-add]#
PeiSiPolicyInit|$(PLATFORM_RC_PACKAGE)/Library/PeiSiPolicyLib/PeiSiPolicyLib.inf
#[-end-151126-IB02950555-add]#

#
# SC
#
  PeiScPolicyLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/PeiScPolicyLib/PeiScPolicyLib.inf
!if $(FSP_BUILD) == TRUE
  ScInitLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/Private/PeiScInitLib/PeiScInitLibFsp.inf
!else
  ScInitLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/Private/PeiScInitLib/PeiScInitLib.inf
!endif
  PeiPciExpressInitLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/Private/PeiPciExpressInitLib/PeiPciExpressInitLib.inf
  PeiSpiInitLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/PeiSpiInitLib/PeiSpiInitLib.inf
  PeiVtdLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/PeiVtdLib/PeiVtdLib.inf

#
# CPU
#
  BiosGuardLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiBiosGuardLibNull/PeiBiosGuardLibNull.inf
  CpuInitLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuInitLib/PeiCpuInitLib.inf
  CpuOcInitLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuOcInitLibNull/PeiCpuOcInitLibNull.inf
  CpuOcLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuOcLibNull/PeiCpuOcLibNull.inf
  CpuPlatformLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiDxeSmmCpuPlatformLib/PeiDxeSmmCpuPlatformLib.inf
  CpuPolicyLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiCpuPolicyLibPreMem/PeiCpuPolicyLibPreMem.inf
  CpuPolicyLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiCpuPolicyLib/PeiCpuPolicyLib.inf
  CpuPowerMgmtLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuPowerMgmtLib/PeiCpuPowerMgmtLib.inf
  CpuPowerOnConfigLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuPowerOnConfigLib/PeiCpuPowerOnConfigLib.inf
  MpServiceLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiMpServiceLib/PeiMpServiceLib.inf
  SecCpuLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/SecCpuLib/SecCpuLib.inf
  CpuS3Lib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuS3Lib/PeiCpuS3Lib.inf


#
# NC
#
  GraphicsInitLib|$(PLATFORM_SI_PACKAGE)/NorthCluster/Library/Private/PeiGraphicsInitLib/PeiGraphicsInitLib.inf
  NpkInitLib|$(PLATFORM_SI_PACKAGE)/NorthCluster/Library/Private/PeiNpkInitLib/PeiNpkInitLib.inf
  PeiSaPolicyLib|$(PLATFORM_SI_PACKAGE)/NorthCluster/Library/PeiSaPolicyLib/PeiSaPolicyLib.inf
  IpuInitLib|$(PLATFORM_SI_PACKAGE)/NorthCluster/Library/Private/PeiIpuInitLib/PeiIpuInitLib.inf
  SaInitLib|$(PLATFORM_SI_PACKAGE)/NorthCluster/Library/Private/PeiSaInitLib/PeiSaInitLib.inf

#[-start-160628-IB07400748-modify]#
  !if $(HG_ENABLE) == YES
    HybridGraphicsInitLib|$(PLATFORM_SI_PACKAGE)/NorthCluster/Library/Private/PeiHybridGraphicsInitLib/PeiHybridGraphicsInitLib.inf
#  !else
#    HybridGraphicsInitLib|$(PLATFORM_SI_PACKAGE)/NorthCluster/Library/Private/PeiHybridGraphicsInitLibNull/PeiHybridGraphicsInitLibNull.inf
  !endif
#[-end-160628-IB07400748-modify]#

#[-start-150708-IB03090000-add]#
#
# Txe
#
SeCUmaLib|$(PLATFORM_RC_PACKAGE)/Txe/Library/Private/PeiSeCUma/SeCUma.inf
#[-end-150708-IB03090000-add]#

  !if $(PEI_DISPLAY_ENABLE) == YES
    DisplayInitLib|$(PLATFORM_SI_PACKAGE)/NorthCluster/Library/Private/PeiDisplayInitLib/PeiDisplayInitLib.inf
  !else
    DisplayInitLib|$(PLATFORM_SI_PACKAGE)/NorthCluster/Library/Private/PeiDisplayInitLibNull/PeiDisplayInitLibNull.inf
  !endif

#
# TXE
#
  HeciMsgLib|$(PLATFORM_SI_PACKAGE)/Txe/Library/HeciMsgLib/PeiHeciMsgLib.inf
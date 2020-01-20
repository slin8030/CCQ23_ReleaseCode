# @file
#  Component description file for the Broxton RC DXE libraries.
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
# RC Dxe Library
#

#[-start-160525-IB06720411-remove]#
# RC 0.9.0
##
## Common
##
#  !if $(SMM_VARIABLE_ENABLE) == YES
#    CseVariableStorageSelectorLib|$(PLATFORM_SI_PACKAGE)/Library/Private/DxeSmmCseVariableStorageSelectorLib/SmmCseVariableStorageSelectorLib.inf
#  !else
#    CseVariableStorageSelectorLib|$(PLATFORM_SI_PACKAGE)/Library/Private/DxeSmmCseVariableStorageSelectorLib/DxeCseVariableStorageSelectorLib.inf
#  !endif
  SmmHeciMsgLib|$(PLATFORM_SI_PACKAGE)/Txe/Library/SmmHeciMsgLib/SmmHeciMsgLib.inf
#[-end-160525-IB06720411-remove]#
#
# CPU
#
#[-start-160414-IB03090426-remove]#
##[-start-151126-IB02950555-add]#
#SmbiosCpuLib|$(PLATFORM_RC_PACKAGE)/Cpu/Library/Private/DxeSmbiosCpuLib/DxeSmbiosCpuLib.inf
##[-end-151126-IB02950555-add]#
#[-end-160414-IB03090426-remove]#

#
# SC
#
#[-start-151126-IB02950555-add]#
#[-start-160518-IB06720411-remove]#
#PchSerialIoUartLib|$(PLATFORM_RC_PACKAGE)/SouthCluster/Library/PeiDxeSmmPchSerialIoUartLib/PeiDxeSmmPchSerialIoUartLib.inf
#PchSerialIoLib|$(PLATFORM_RC_PACKAGE)/SouthCluster/Library/PeiDxeSmmPchSerialIoLib/PeiDxeSmmPchSerialIoLib.inf
#[-end-160518-IB06720411-remove]#
ScSmbusCommonLib|$(PLATFORM_RC_PACKAGE)/SouthCluster/Library/Private/PeiDxeSmmScSmbusCommonLib/PeiDxeSmmScSmbusCommonLib.inf
#[-start-160510-IB03090427-remove]#
#PmicLib|$(PLATFORM_RC_PACKAGE)/SouthCluster/Library/PmicLib/PmicLib.inf
#[-end-160510-IB03090427-remove]#
#[-end-151126-IB02950555-add]#

#
# NC
#

#
# TXE
#
  HeciMsgLib|$(PLATFORM_SI_PACKAGE)/Txe/Library/HeciMsgLib/DxeSmmHeciMsgLib.inf
#[-start-151126-IB02950555-add]#
!if $(SEC_ENABLE) == YES
  SeCLib|$(PLATFORM_SI_PACKAGE)/Txe/Library/SeCLib/SeCLib.inf
!endif
#[-end-151126-IB02950555-add]#
  BootMediaLib|$(PLATFORM_SI_PACKAGE)/Library/DxeSmmBootMediaLib/DxeSmmBootMediaLib.inf
#
# HSTI
#
!if $(HSTI_ENABLE) == TRUE
     PttPtpLib|$(PLATFORM_SI_PACKAGE)/Txe/Library/PeiDxePttPtpLib/PeiDxePttPtpLib.inf
!endif

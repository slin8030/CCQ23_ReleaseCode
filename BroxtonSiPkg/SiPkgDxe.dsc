## @file
#  Component description file for the Broxton RC DXE drivers.
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
# CPU
#
  $(PLATFORM_SI_PACKAGE)/Cpu/CpuInit/Dxe/CpuInitDxe.inf
  !if $(PPM_ENABLE) == YES
    $(PLATFORM_SI_PACKAGE)/Cpu/PowerManagement/Dxe/PowerMgmtDxe.inf
 #   $(PLATFORM_SI_PACKAGE)/Cpu/PowerManagement/Smm/PowerMgmtSmm.inf
    $(PLATFORM_SI_PACKAGE)/Cpu/AcpiTables/CpuAcpiTables.inf
  !endif
    $(PLATFORM_SI_PACKAGE)/Cpu/SmmAccess/Dxe/SmmAccess.inf

#
# TXE
#
  !if $(SEC_ENABLE) == YES
    $(PLATFORM_SI_PACKAGE)/Txe/Heci/Dxe/Hecidrv.inf
  !endif

#
# HSTI
#
!if $(HSTI_ENABLE) == TRUE
$(PLATFORM_SI_PACKAGE)/Hsti/Dxe/HstiSiliconDxe.inf {
    <LibraryClasses>
      HstiLib | MdePkg/Library/DxeHstiLib/DxeHstiLib.inf
      Tpm2CommandLib | SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
      Tpm2DeviceLib | SecurityPkg/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterDxe.inf
      NULL | SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpm.inf
      NULL | $(PLATFORM_SI_PACKAGE)/SampleCode/Library/Tpm2DeviceLibPtp/Tpm2InstanceLibPtt.inf
  }
!endif

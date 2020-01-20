## @file
#
#
# @copyright
#  INTEL CONFIDENTIAL
#  Copyright 2016 Intel Corporation.
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
#@par Specification Reference:
#
##
#
# Define Platform specific pre-processor tokens
#
!if $(TABLET_PF_BUILD) == TRUE
  DEFINE TABLET_PF_ENABLE = -DTABLET_PF_ENABLE=1
!else
  DEFINE TABLET_PF_ENABLE = -DTABLET_PF_ENABLE=0
!endif

!if $(ENBDT_PF_BUILD) == TRUE
  DEFINE ENBDT_PF_ENABLE = -DENBDT_PF_ENABLE=1
!else
  DEFINE ENBDT_PF_ENABLE = -DENBDT_PF_ENABLE=0
!endif

!if $(BXTI_PF_BUILD) == TRUE
  DEFINE BXTI_PF_ENABLE = -DBXTI_PF_ENABLE=1
!else
  DEFINE BXTI_PF_ENABLE = -DBXTI_PF_ENABLE=0
!endif
#
# Define common pre-processor tokens.
#
!if $(PCIESC_ENABLE) == TRUE
  DEFINE PCIESC_SUPPORT_BUILD_OPTION = -DPCIESC_SUPPORT=1
!endif

!if $(SATA_ENABLE) == TRUE
  DEFINE SATA_SUPPORT_BUILD_OPTION = -DSATA_SUPPORT=1
!endif

!if $(FSP_WRAPPER_ENABLE) == TRUE
  DEFINE FSP_WRAPPER_BUILD_OPTIONS = -DFSP_WRAPPER_FLAG=1
!else
  DEFINE FSP_WRAPPER_BUILD_OPTIONS =
!endif # $(FSP_WRAPPER_ENABLE) == TRUE

!if $(NVM_VARIABLE_ENABLE) == TRUE
  DEFINE NVM_VARIABLE_BUILD_OPTION = -DNVM_VARIABLE_ENABLE=1
!endif

!if $(NPK_ENABLE) == TRUE
  DEFINE NPK_ENABLE_BUILD_OPTIONS = -DNPK_ENABLE=1
!endif

!if $(FTPM_ENABLE) == TRUE
  DEFINE FTPM_SUPPORT_BUILD_OPTIONS = -DFTPM_SUPPORT=1
!endif

  DEFINE EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS = $(FSP_WRAPPER_BUILD_OPTIONS) $(TABLET_PF_ENABLE) $(ENBDT_PF_ENABLE) $(BXTI_PF_ENABLE) $(NVM_VARIABLE_BUILD_OPTION) $(NPK_ENABLE_BUILD_OPTIONS) $(FTPM_SUPPORT_BUILD_OPTIONS) $(PCIESC_SUPPORT_BUILD_OPTION) $(SATA_SUPPORT_BUILD_OPTION) $(HDAUDIO_SUPPORT_BUILD_OPTION)


#
# Compiler flags
#
  *_*_IA32_ASM_FLAGS     = $(FSP_WRAPPER_BUILD_OPTIONS) $(VP_BUILD_OPTIONS) -DEDKII_GLUE_PciExpressBaseAddress=$(PLATFORM_PCIEXPRESS_BASE)h

  *_*_IA32_CC_FLAGS      = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  *_*_IA32_VFRPP_FLAGS   = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  *_*_IA32_APP_FLAGS     = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  *_*_IA32_PP_FLAGS      = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  *_*_IA32_ASLPP_FLAGS   = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)

  *_*_X64_CC_FLAGS       = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  *_*_X64_VFRPP_FLAGS    = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  *_*_X64_APP_FLAGS      = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  *_*_X64_PP_FLAGS       = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)
  *_*_X64_ASLPP_FLAGS    = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS)

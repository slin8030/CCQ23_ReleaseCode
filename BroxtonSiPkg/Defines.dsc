##@file
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

###############################################
## All Fixed Defines should be here
###############################################

  DEFINE S3_ENABLE = TRUE
  DEFINE USB_ENABLE = TRUE
  DEFINE USB_NATIVE_ENABLE = TRUE
  DEFINE ATA_ENABLE = TRUE
  DEFINE AHCI_ENABLE = TRUE
  DEFINE IGD_ENABLE = TRUE
  DEFINE DTS_ENABLE = FALSE
  DEFINE PPM_ENABLE = TRUE
  DEFINE NPK_ENABLE = FALSE
  DEFINE PRAM_ENABLE = TRUE
  DEFINE MRCDEBUG_ENABLE = FALSE          # May not have any effect on Release builds
  DEFINE NVM_VARIABLE_ENABLE = TRUE
  #
  # Do not use 0x prefix, pass prefix 0x or postfix h through macro for C or ASM
  #
  DEFINE   PLATFORM_PCIEXPRESS_BASE   = 0E0000000
  DEFINE   PLATFORM_PCIEXPRESS_LEN    = 10000000

################################################
## All Dependent (if) Defines should be here
################################################

  #
  # MCG/PCCG/ISG dependent flags
  #
  !if $(TABLET_PF_BUILD) == TRUE
    DEFINE I2C_DRIVER_ENABLE  = FALSE
    DEFINE EMMC_DRIVER_ENABLE = TRUE
    DEFINE SATA_ENABLE        = FALSE
    DEFINE PCIESC_ENABLE      = FALSE
    DEFINE HG_ENABLE          = FALSE
  !elseif $(ENBDT_PF_BUILD) == TRUE
    DEFINE I2C_DRIVER_ENABLE  = FALSE
    DEFINE EMMC_DRIVER_ENABLE = TRUE
    DEFINE SATA_ENABLE        = TRUE
    DEFINE PCIESC_ENABLE      = TRUE
    DEFINE HG_ENABLE          = TRUE
    DEFINE HSTI_ENABLE        = FALSE
  !else
    # ISG
  !endif

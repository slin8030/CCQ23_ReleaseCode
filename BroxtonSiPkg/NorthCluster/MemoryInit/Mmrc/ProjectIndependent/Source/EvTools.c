/** @file
  EvTools.c
  Rank Margin EV Tools, these file is independent of the project.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2005 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/
#include "MmrcLibraries.h"
#include "MmrcHooks.h"


#if SIM || JTAG
MMRC_DATA *MyMrcData;
#endif

/**
  Executes the Rank Margin Tool

  @param[in, out]  MrcData             Host structure for all data related to MMRC.
  @param[in]       CapsuleStartIndex   Starting point within the CapsuleData structure
                                       where the Init should begin.
  @param[in]       StringIndex         When provided, this is the index into the string
                                       table to show the register names.
  @param[in]       Channel             Current Channel being examined

  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
EvToolsMain (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            CapsuleStartIndex,
  IN      UINT16            StringIndex,
  IN      UINT8             Channel
  )
{
  return MMRC_SUCCESS;
}

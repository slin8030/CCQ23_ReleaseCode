/** @file
  NULL Library for SATA Init

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

#include  "ScSataInit.h"

/**
  Function to configure SATA HBA in PEI phase
  CAP register in AHCI BAR will be set based on policy
  eSATA speed limit, per port, is also handled based on policy

  @param[in] ScPolicy                   The SC Policy instance
  @param[in] PciSataRegBase             SATA Register Base, Device 31 Function 2
  @param[in] SataPortToBeEnabledBitmap  Per port settings in AHCI will be based on this
  @param[in] TempMemBaseAddr            Temporary Memory Base Address for PCI
                                        devices to be used to initialize MMIO
                                        registers.

  @retval None
**/
VOID
ScSataInitAhci (
  IN  SC_POLICY_PPI               *ScPolicy,
  IN  UINTN                       PciSataRegBase,
  IN  UINT8                       SataPortToBeEnabledBitmap,
  IN  UINT32                      AhciBarAddress
  )
{

}

/**
  Internal function performing SATA init needed in PEI phase

  @param[in] ScPolicy            The SC Policy instance
  @param[in] AhciBarAddress      Temporary Memory Base Address for PCI
                                 devices to be used to initialize MMIO
                                 registers.
  @param[out] FuncDisable1Reg    To be used to update function disable register

  @retval None
**/
VOID
ScSataInit (
  IN  SC_POLICY_PPI  *ScPolicy,
  IN  UINT32         AhciBarAddress,
  OUT UINT32         *FuncDisable1Reg
  )
{

}



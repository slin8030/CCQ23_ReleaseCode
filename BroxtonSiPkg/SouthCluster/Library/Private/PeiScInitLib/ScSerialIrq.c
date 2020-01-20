/** @file
  Initializes serial IRQ

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

#include "ScInitPei.h"

/**
  The function performs Serial IRQ specific programming.

  @param[in] ScPolicyPpi  The SC Policy PPI instance
**/
VOID
EFIAPI
ScConfigureSerialIrq (
  IN  SC_POLICY_PPI *ScPolicyPpi
  )
{
  UINTN              PciLpcRegBase;
  UINT8              RegData8;
  EFI_STATUS         Status;
  SC_LPC_SIRQ_CONFIG *SerialIrqConfig;

  DEBUG ((DEBUG_INFO, "ScConfigureSerialIrq() - Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gSerialIrqConfigGuid, (VOID *) &SerialIrqConfig);
  ASSERT_EFI_ERROR (Status);

  if (SerialIrqConfig->SirqEnable == FALSE) {
    return;
  }

  PciLpcRegBase = MmPciBase (
                    DEFAULT_PCI_BUS_NUMBER_SC,
                    PCI_DEVICE_NUMBER_PCH_LPC,
                    PCI_FUNCTION_NUMBER_PCH_LPC
                    );
  RegData8      = 0;

  ///
  /// PCH BIOS Spec Section 6.3 Serial IRQs
  /// The only System BIOS requirement to use IRQs as a serial IRQ is to enable the function in LPC PCI offset 64h[7] and
  /// select continuous or quiet mode, LPC PCI offset 64h[6].
  /// PCH requires that the System BIOS first set the SERIRQ logic to continuous mode operation for at least one frame
  /// before switching it into quiet mode operation. This operation should be performed during the normal boot sequence
  /// as well as a resume from STR (S3).
  ///
  RegData8  = MmioRead8 (PciLpcRegBase + R_PCH_LPC_SERIRQ_CNT);
  RegData8  &= (UINT8) ~(B_PCH_LPC_SERIRQ_CNT_SIRQEN | B_PCH_LPC_SERIRQ_CNT_SFPW);

  switch (SerialIrqConfig->StartFramePulse) {
    case ScSfpw8Clk:
      RegData8 |= V_PCH_LPC_SERIRQ_CNT_SFPW_8CLK;
      break;
    case ScSfpw6Clk:
      RegData8 |= V_PCH_LPC_SERIRQ_CNT_SFPW_6CLK;
      break;
    case ScSfpw4Clk:
    default:
      RegData8 |= V_PCH_LPC_SERIRQ_CNT_SFPW_4CLK;
      break;
  }
  ///
  /// Set the SERIRQ logic to continuous mode
  ///
  RegData8 |= (UINT8) (B_PCH_LPC_SERIRQ_CNT_SIRQEN | B_PCH_LPC_SERIRQ_CNT_SIRQMD);
  MmioWrite8 (PciLpcRegBase + R_PCH_LPC_SERIRQ_CNT, RegData8);

  ///
  /// PCH BIOS Spec Section 6.3 Serial IRQs
  /// The only System BIOS requirement to use IRQs as a serial IRQ is to enable the function
  /// in LPC PCI offset 64h[7] and select continuous or quiet mode, LPC PCI offset 64h[6].
  ///
  if (SerialIrqConfig->SirqMode == ScQuietMode) {
    MmioAnd8 (PciLpcRegBase + R_PCH_LPC_SERIRQ_CNT, (UINT8)~B_PCH_LPC_SERIRQ_CNT_SIRQMD);
  }
  DEBUG ((DEBUG_INFO, "ScConfigureSerialIrq() - End\n"));
}



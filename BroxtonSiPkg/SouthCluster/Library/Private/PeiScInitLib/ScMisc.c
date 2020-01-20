/** @file
  Initializes Miscellaneous SC configurations

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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
#include <Ppi/ReadOnlyVariable2.h>
#include <Library/PeiServicesLib.h>

/**
  Internal function performing miscellaneous init needed in early PEI phase

  @param[in] ScPolicyPpi  The SC Platform Policy PPI instance

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_DEVICE_ERROR        Device error, aborts abnormally.
**/
EFI_STATUS
ScMiscInit (
  IN  SC_POLICY_PPI      *ScPolicyPpi
  )
{


  UINTN           P2sbBase;
  UINT16          RegData16;
  EFI_STATUS      Status;
  SC_HPET_CONFIG  *HpetConfig;

  DEBUG ((DEBUG_INFO, "ScMiscInit() - Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gHpetConfigGuid, (VOID *) &HpetConfig);
  ASSERT_EFI_ERROR (Status);

  P2sbBase = MmPciBase (
       DEFAULT_PCI_BUS_NUMBER_SC,
       PCI_DEVICE_NUMBER_P2SB,
       PCI_FUNCTION_NUMBER_P2SB
     );
  RegData16 = (
    (DEFAULT_PCI_BUS_NUMBER_SC << 8) +
    (PCI_DEVICE_NUMBER_P2SB << 3) +
    PCI_FUNCTION_NUMBER_P2SB
    );
  //
  // P2SB PCI CFG Offset 0x50 = VBDF (VLW Bus:Device:Function)
  //
  MmioWrite16((UINTN)(P2sbBase + R_PCH_P2SB_VBDF), RegData16);
  //
  // P2SB PCI CFG Offset 0xA0 = DISPBDF (Display Bus:Device:Function)
  // Must be set to P2SB BDF - used by CSE for PAVP flows
  //
  MmioWrite16((UINTN)(P2sbBase + R_PCH_P2SB_DISPBDF), RegData16);

  ///
  /// Initial and enable HPET High Precision Timer memory address for basic usage
  ///
  if (HpetConfig->Enable == TRUE) {
    ///
    /// Set HPET Timer enable to start counter spinning
    ///
    MmioOr32 (HpetConfig->Base + R_HPET_GCFG, B_HPET_GCFG_EN);
    //
    // Read back Posted Writes Register
    //
    MmioRead32 (HpetConfig->Base + R_HPET_GCFG);

    ///
    /// P2SB PCI CFG Offset 0x70 = HBDF (HPET Bus:Device:Function)
    ///
    if (HpetConfig->BdfValid) {
      RegData16 = ((UINT16) (HpetConfig->BusNumber) << 8) & B_PCH_P2SB_HBDF_BUF;
      RegData16 |= ((UINT16) (HpetConfig->DeviceNumber) << 3) & B_PCH_P2SB_HBDF_DEV;
      RegData16 |= (UINT16) (HpetConfig->FunctionNumber) & B_PCH_P2SB_HBDF_FUNC;
      MmioWrite16 ((UINTN) (P2sbBase + R_PCH_P2SB_HBDF), RegData16);
    }
  } else {
    MmioAnd32 (HpetConfig->Base + R_HPET_GCFG, (UINT32) ~B_HPET_GCFG_EN);
  }


  DEBUG ((DEBUG_INFO, "ScMiscInit() - End\n"));

  return EFI_SUCCESS;
}


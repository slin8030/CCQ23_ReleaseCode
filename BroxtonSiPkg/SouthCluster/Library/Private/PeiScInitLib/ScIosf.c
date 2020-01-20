/** @file
  Initializes SC IOSF Devices

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

@par Specification
**/
#include "ScInitPei.h"

/**
  Configure Iosf devices.

  @param[in]  None

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureIosf (
  )
{
  UINTN             P2sbBase;
  UINTN             Stepping;

  P2sbBase = 0;
  Stepping = BxtStepping();

  DEBUG ((DEBUG_INFO, "ConfigureIosf() Start\n"));

#ifdef SC_PM_ENABLE
if ((PLATFORM_ID == VALUE_REAL_PLATFORM) ||
    (PLATFORM_ID == VALUE_SLE_UPF)
       ){
  DEBUG ((DEBUG_INFO, "Start SC_PM_ENABLE\n"));
  //
  //
  if (SideBandRead32 (SB_PORTID_PSF0, R_PCH_PCR_PSF_GLOBAL_CONFIG) & (BIT3 | BIT4)) {
    DEBUG ((DEBUG_INFO, "PSF clock gate has been set earlier, temporarily disable clock gate before setting PSF_CLKREQ_HYS\n"));
    SideBandAndThenOr32 (SB_PORTID_PSF0, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFE4, 0);
    SideBandAndThenOr32 (SB_PORTID_PSF1, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFE4, 0);
    SideBandAndThenOr32 (SB_PORTID_PSF2, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFE4, 0);
    SideBandAndThenOr32 (SB_PORTID_PSF3, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFE4, 0);
    SideBandAndThenOr32 (SB_PORTID_PSF4, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFE4, 0);
  }

  SideBandAndThenOr32 (SB_PORTID_PSF0, R_PSF_CLKREQ_HYS, 0xFFFF0000, BIT2);
  SideBandAndThenOr32 (SB_PORTID_PSF1, R_PSF_CLKREQ_HYS, 0xFFFF0000, BIT2);
  SideBandAndThenOr32 (SB_PORTID_PSF2, R_PSF_CLKREQ_HYS, 0xFFFF0000, BIT2);
  SideBandAndThenOr32 (SB_PORTID_PSF3, R_PSF_CLKREQ_HYS, 0xFFFF0000, BIT2);
  SideBandAndThenOr32 (SB_PORTID_PSF4, R_PSF_CLKREQ_HYS, 0xFFFF0000, BIT2);
  //
  // PSF Low Power Programming
  //
  SideBandAndThenOr32 (SB_PORTID_PSF1, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFC, BIT7 | BIT6 | BIT4 | BIT3);
  SideBandAndThenOr32 (SB_PORTID_PSF2, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFC, BIT7 | BIT6 | BIT4 | BIT3);
  SideBandAndThenOr32 (SB_PORTID_PSF4, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFC, BIT7 | BIT6 | BIT4 | BIT3);
  switch (Stepping) {
    case BxtB0:
    case BxtB1:
      SideBandAndThenOr32 (SB_PORTID_PSF0, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFC, BIT7 | BIT6 | BIT4 | BIT3);
      SideBandAndThenOr32 (SB_PORTID_PSF3, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFC, BIT7 | BIT6 | BIT4);
      break;

//[-start-160828-IB07400775-add]//
    case BxtPA0:
      SideBandAndThenOr32 (SB_PORTID_PSF0, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFC, BIT7 | BIT6 | BIT4);
      SideBandAndThenOr32 (SB_PORTID_PSF3, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFC, BIT7 | BIT6 | BIT4);
      break;
//[-end-160828-IB07400775-add]//

    default:
      SideBandAndThenOr32 (SB_PORTID_PSF0, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFC, BIT7 | BIT6 | BIT4 | BIT3);
      SideBandAndThenOr32 (SB_PORTID_PSF3, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFC, BIT7 | BIT6 | BIT4 | BIT3);
  }
  //
  // P2SB Low Power Programming
  //
  P2sbBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_P2SB, PCI_FUNCTION_NUMBER_P2SB);
  MmioOr32 (P2sbBase + R_P2SB_P2SBC, B_P2SB_P2SBC_PGCBCGE);
  MmioOr32 (P2sbBase + R_P2SB_PCE, B_P2SB_PCE_HAE | B_P2SB_PCE_SE | B_P2SB_PCE_PMCRE);
  MmioAnd32 (P2sbBase + R_P2SB_PDOMAIN, (UINT32)(~0x00030003));
  DEBUG ((DEBUG_INFO, "End SC_PM_ENABLE\n"));
}
#endif

  DEBUG ((DEBUG_INFO, "ConfigureIosf() End\n"));

  return EFI_SUCCESS;
}


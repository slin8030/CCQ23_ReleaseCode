/** @file
  Initializes RTC.

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

/**
  The function performs RTC specific programming.

  @param[in] ScPolicyPpi  The SC Policy PPI instance

  @retval EFI_SUCCESS     The required settings programmed successfully
**/
EFI_STATUS
EFIAPI
ConfigureRtc (
  )
{
#if (TABLET_PF_ENABLE == 1)
  UINTN P2sbBase;
#ifdef SC_PM_ENABLE
  UINTN PmcPciMmBase;

  PmcPciMmBase = 0;
#endif
  DEBUG ((DEBUG_INFO, "ConfigureRtc() Start\n"));
  ///
  ///Set P2SB RCFG RTC Shadow Enable bit (RSE).
  ///
  P2sbBase = MmPciBase (
               DEFAULT_PCI_BUS_NUMBER_SC,
               PCI_DEVICE_NUMBER_P2SB,
               PCI_FUNCTION_NUMBER_P2SB
               );
  MmioOr32 (P2sbBase + R_PCH_P2SB_RCFG, B_PCH_P2SB_RCFG_RSE);
#endif

#ifdef SC_PM_ENABLE
  //
  // Program RTCDCG (RTC Dynamic Clock Gating Control). RTCDCG[2:0] = 3'b111
  //
  SideBandAndThenOr32(
    PID_RTC,
    R_PCH_PCR_RTC_RTCDCG,
    0xFFFFFFFF,
    B_PCH_PCR_RTC_RTCDCG_RTCROSIDEDCGEN |
    B_PCH_PCR_RTC_RTCDCG_RTCPCICLKDCGEN |
    B_PCH_PCR_RTC_RTCDCG_RTCPGCBDCGEN
    );
#endif

  DEBUG ((DEBUG_INFO, "ConfigureRtc() End\n"));
  return EFI_SUCCESS;
}



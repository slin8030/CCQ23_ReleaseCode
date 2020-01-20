/** @file
  Hybrid Graphics Pei driver.
  This Pei driver initialize GPIO programming
  for the platform.

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
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/MmPciLib.h>
#include <CpuRegs.h>
#include <SaAccess.h>
#include <PcieRegs.h>
#include <Library/PeiHybridGraphicsInitLib.h>
#include <Library/PeiServicesLib.h>
#include <Guid/SaDataHob.h>
#include <Library/PeiSaPolicyLib.h>
#include <Ppi/Stall.h>

/**
  Initialize the Hybrid Graphics support (PEI).

  @param[in]
  IN   HYBRID_GRAPHICS_CONFIG        *HgConfig
**/
VOID
HybridGraphicsInit (
  IN   HYBRID_GRAPHICS_CONFIG        *HgConfig
  )
{
  SA_DATA_HOB             *SaDataHob;

  DEBUG ((DEBUG_INFO, "HG::Hybrid Graphics PEI Initialization Starts\n"));
  ///
  /// Get SaDataHob HOB
  ///
  SaDataHob = NULL;
  SaDataHob = (SA_DATA_HOB *) GetFirstGuidHob (&gSaDataHobGuid);

  if (SaDataHob != NULL) {
    SaDataHob->HgInfo.HgEnabled              = HgConfig->HgEnabled;
    SaDataHob->HgInfo.HgDelayAfterHoldReset  = HgConfig->HgDelayAfterHoldReset;
    SaDataHob->HgInfo.HgDelayAfterPwrEn      = HgConfig->HgDelayAfterPwrEn;
    ///
    /// GPIO for PCIe Port
    ///
    if (HgConfig->HgEnabled == 1) {
      DEBUG ((DEBUG_INFO, "HG::Enabled: 0x%x\n", HgConfig->HgEnabled));
      SaDataHob->HgInfo.HgDgpuHoldRst.CommunityOffset      = HgConfig->HgDgpuHoldRst.CommunityOffset;
      SaDataHob->HgInfo.HgDgpuHoldRst.PinOffset            = HgConfig->HgDgpuHoldRst.PinOffset;
      SaDataHob->HgInfo.HgDgpuHoldRst.Active               = HgConfig->HgDgpuHoldRst.Active;
      SaDataHob->HgInfo.HgDgpuPwrEnable.CommunityOffset    = HgConfig->HgDgpuPwrEnable.CommunityOffset;
      SaDataHob->HgInfo.HgDgpuPwrEnable.PinOffset          = HgConfig->HgDgpuPwrEnable.PinOffset;
      SaDataHob->HgInfo.HgDgpuPwrEnable.Active             = HgConfig->HgDgpuPwrEnable.Active;
      SaDataHob->HgInfo.RootPortDev                        = HgConfig->RootPortDev;
      SaDataHob->HgInfo.RootPortFun                        = HgConfig->RootPortFun;
    }
  }
  ///
  /// Program SubsystemID for IGFX
  ///
  DEBUG ((DEBUG_INFO, "HG::Program SDID [Subsystem ID] for IGFX: 0x%x\n", HgConfig->HgSubSystemId));
  MmioOr16 (MmPciBase (SA_MC_BUS, 2, 0) + PCI_SID_OFFSET, HgConfig->HgSubSystemId);

  DEBUG ((DEBUG_INFO, "HG::Hybrid Graphics PEI Initialization Ends\n"));
}

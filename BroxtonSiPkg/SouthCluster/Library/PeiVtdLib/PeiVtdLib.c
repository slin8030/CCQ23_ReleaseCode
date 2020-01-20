/** @file
  This code provides a initialization of intel VT-d (Virtualization Technology for Directed I/O).

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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
#include <PiPei.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <SaRegs.h>
#include <Library/ScPlatformLib.h>
#include <Library/PeiDxeSmmMmPciLib.h>
#include <Library/ConfigBlockLib.h>
#include <Ppi/ScPolicy.h>
#include <Library/PeiVtdLib.h>
#include <PlatformBaseAddresses.h>

/**
  Enable VTd support in PSF.

  @retval EFI_SUCCESS   Successfully completed.
**/
EFI_STATUS
PchPsfEnableVtd (
  VOID
  );

/**
  Configure VT-d Base and capabilities.

  @param[in] ScPolicyPpi     - The SC Policy PPI instance

  @retval EFI_SUCCESS        - VT-d initialization complete
  @retval EFI_UNSUPPORTED    - VT-d is disabled by policy or not supported
**/
EFI_STATUS
VtdInit (
  IN SC_POLICY_PPI     *ScPolicyPpi
  )
{
  UINTN                McD0BaseAddress;
  UINTN                MchBar;
  UINT32               Data32Or;
  UINT32               VtdBase;
  UINT32               VtdGfxBase;
  EFI_STATUS           Status;
  SC_VTD_CONFIG        *VtdConfig;

  McD0BaseAddress = MmPciBase (SA_MC_BUS, 0, 0);
  MchBar          = MmioRead64 (McD0BaseAddress + R_SA_MCHBAR) &~BIT0;
  VtdGfxBase      = PcdGet32 (PcdVtdGfxBaseAddress);

  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gVtdConfigGuid, (VOID *) &VtdConfig);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Check SA supports VT-d and VT-d is enabled
  ///
  if ((VtdConfig->VtdEnable == 0) || (MmioRead32 (McD0BaseAddress + R_SA_MC_CAPID0_A) & BIT23)) {
    MmioWrite32 (MchBar + R_SA_MCHBAR_VTD2_OFFSET, DEF_VTD_BASE_ADDRESS);
    MmioWrite32 (MchBar + R_SA_MCHBAR_VTD1_OFFSET, VtdGfxBase);
    DEBUG ((DEBUG_INFO, "VTd disabled or no capability!\n"));
    return EFI_UNSUPPORTED;
  }

  ///
  /// Enable VTd in PSF
  ///
  Status = PchPsfEnableVtd ();
  ASSERT_EFI_ERROR (Status);

  ///
  ///  Configure VTD1 BAR, Skip GFXVTBAR if IGD is disabled
  ///
  if (MmioRead16 (MmPciBase (SA_MC_BUS, 2, 0) + R_SA_IGD_VID) != 0xFFFF) {
    Data32Or = VtdGfxBase;
    Data32Or |= 0x1;
    MmioWrite32 (MchBar + R_SA_MCHBAR_VTD1_OFFSET, Data32Or);
  }

  ///
  /// Configure VTD2 BAR
  ///
  Data32Or = DEF_VTD_BASE_ADDRESS;
  Data32Or |= 0x1;
  MmioWrite32 (MchBar + R_SA_MCHBAR_VTD2_OFFSET, Data32Or);

  ///
  /// Read back VTD BAR register ensure that the write completes
  ///
  VtdBase = MmioRead32 (MchBar + R_SA_MCHBAR_VTD1_OFFSET);
  if(VtdBase != (VtdGfxBase | BIT0)){
    DEBUG ((DEBUG_INFO, "GFX_VTD_BASE_ADDRESS set failed!\n"));
    return EFI_UNSUPPORTED;
  }

  VtdBase = MmioRead32 (MchBar + R_SA_MCHBAR_VTD2_OFFSET);
  if(VtdBase != (DEF_VTD_BASE_ADDRESS | BIT0)){
    DEBUG ((DEBUG_INFO, "DEF_VTD_BASE_ADDRESS set failed!\n"));
    return EFI_UNSUPPORTED;
  }

  ///
  /// Lock policy registers
  ///
  MmioWrite32 (MchBar + R_SA_MCHBAR_VTD1_OFFSET,MmioRead32 (MchBar + R_SA_MCHBAR_VTD1_OFFSET) | BIT1);
  MmioWrite32 (MchBar + R_SA_MCHBAR_VTD2_OFFSET,MmioRead32 (MchBar + R_SA_MCHBAR_VTD2_OFFSET) | BIT1);
  DEBUG ((DEBUG_INFO, "VTd enabled\n"));
  return EFI_SUCCESS;
}

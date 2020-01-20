/** @file
  Source code file for Silicon Init Pre Memory module.

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

@par Specification
**/
#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <Ppi/SeCUma.h>
#include "UfsMphyInit.h"
#include <UfsMphyDataDef.h>
#include <Library/HeciMsgLib.h>
#include <Library/HobLib.h>
#include <Library/PmcIpcLib.h>

extern EFI_GUID gEfiBootMediaHobGuid;
extern EFI_GUID gUfsPhyOverrideHobGuid;
extern UINT8    mIpcWbuffer[4];


UINT32  gMphyOverrideMagic[4] = {0xFEEDFACE, 0xA5A5A5A5, 0xDEADF00D, 0x5A5A5A5A};

UFS_MPHY_REG_CONFIG ScUFSMphyPwmG1Reg[] = {
  { 0x1010, 0x00000700, 0x00000100, 0 },   ///< Lane 0 RMMI width
  { 0x1020, 0x00000003, 0x00000001, 0 },   ///< Lane 0 UFS comm protocol
  { 0x1054, 0x07000000, 0x07000000, 0 },   ///< Lane 0 Config update
};


/**
  This function is used to program the UFS MPHY Registers

  @param[in]  pRegConfig  UFS MPHY Registers Table pointer
  @param[in]  length      UFS MPHY Registers Table length

  @retval None
**/

void
UfsProgramMphyRegs (
  IN      UFS_MPHY_REG_CONFIG *pRegConfig,
  IN      UINT32              length
  )
{
  UINT32 index = 0;
  UINT32 Data32;
  UINT64 Delay;

  if (pRegConfig == NULL) {
    return;
  }

  for (index = 0; index < length; index++) {
    SideBandAndThenOr32(
      SB_UFS_PORT,
      (UINT16)pRegConfig[index].RegOff,
      ~(pRegConfig[index].RegBitMask),
      pRegConfig[index].RegVal
      );

    DEBUG ((EFI_D_INFO, "UFS PortId: 0x%X, RegOff: 0x%04X, RegBitMask: 0x%08X, RegVal: 0x%08X\n",
      SB_UFS_PORT,
      pRegConfig[index].RegOff,
      pRegConfig[index].RegBitMask,
      pRegConfig[index].RegVal)
      );

    if (pRegConfig[index].RegPoll == 1) {
      Delay = DivU64x32 (UFS_MPHY_TIMEOUT, 10) + 1;
      do {
        ///
        /// Read back to see if the value being read as zero
        ///
        Data32 = SideBandRead32(SB_UFS_PORT, (UINT16)pRegConfig[index].RegOff);

        if ((Data32 & pRegConfig[index].RegBitMask) == 0 ) {
          break;
        }
        ///
        /// Stall for 1 microseconds.
        ///
        MicroSecondDelay (1);
        Delay--;
      } while (Delay > 0);
    }
  }

  return;

}

/**
  UfsConfigureMphyOverrides

  @param[in]  None

  @retval EFI_STATUS
**/
EFI_STATUS
EFIAPI
UfsConfigureMphyOverrides (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS           Hob;
  UFS_MPHY_OVERRIDE_CONFIG      *UfsOverrideCfg;

  Hob.Raw = GetFirstGuidHob (&gUfsPhyOverrideHobGuid);
  ASSERT (Hob.Raw != NULL);
  UfsOverrideCfg = (UFS_MPHY_OVERRIDE_CONFIG*)GET_GUID_HOB_DATA(Hob.Raw);

  DEBUG ((EFI_D_INFO, "UfsConfigureMphyOverrides() - Start \n"));
  if (UfsOverrideCfg == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Sanity check: validate magic data and num of regs to configure
  //
  if (CompareMem(UfsOverrideCfg->Magic,gMphyOverrideMagic, sizeof(gMphyOverrideMagic)) != 0 ) {
    DEBUG ((EFI_D_ERROR, "UFS MPHY Overrides block - invalid magic data\n"));
    return EFI_INVALID_PARAMETER;
  }
  //
  // Clear pll_en_ctrl bit, masking the MPHY PLL enable signal: RMW GPPRVRW6 @ 0x614, clear bit 31
  //
  SideBandAndThenOr32(SCC_EP_PORT_ID, R_SCC_SB_GPPRVRW6, 0x7FFFFFFF, 0x0);
  //
  // Configure MPHY to HS
  //
  UfsProgramMphyRegs(UfsOverrideCfg->MphyRegVals, UfsOverrideCfg->RegCount);
  //
  // Set pll_en_ctrl bit, unmasking the MPHY PLL enable signal: RMW GPPRVRW6 @ 0x614, set bit 31 to: 1`b1
  //
  SideBandAndThenOr32(SCC_EP_PORT_ID, R_SCC_SB_GPPRVRW6, 0x7FFFFFFF, 0x80000000);
  //
  // Delay of 100uS needed after above pll_en_ctrl unmasking write to allow PLL to lock
  //
  MicroSecondDelay (100);

  DEBUG ((EFI_D_INFO, "UfsConfigureMphyOverrides() - End\n"));

  return EFI_SUCCESS;
}

/**
  UfsConfigureMphyToPwmG1

  @param[in]  None

  @retval EFI_STATUS
**/
EFI_STATUS
EFIAPI
UfsConfigureMphyToPwmG1 (
  VOID
  )
{
  UINT32 length;

  DEBUG ((EFI_D_INFO, "UfsConfigureMphyToPwmG1() - Start \n"));

  length = sizeof(ScUFSMphyPwmG1Reg)/sizeof(UFS_MPHY_REG_CONFIG);
  ///
  /// 1, Configure rmmi_width for 16 bits:
  /// RMW IMPREG5 @ 0x1010, set bits 10,9,8 to:3`b001
  /// 2, Configure comm_protocol for UFS:
  /// RMW IMPREG9 @ 0x1020, set bits 1,0 to:2`b01
  /// 3, Apply above configuration by setting cfg_update:
  /// RMW IMPREG22 @ 0x1054, set bits 26,25,24 to:3`b111
  ///
  UfsProgramMphyRegs(ScUFSMphyPwmG1Reg, length);

  //
  // Set pll_en_ctrl bit, unmasking the MPHY PLL enable signal: RMW GPPRVRW6 @ 0x614, set bit 31 to: 1`b1
  //
  SideBandAndThenOr32(SCC_EP_PORT_ID, R_SCC_SB_GPPRVRW6, 0x7FFFFFFF, 0x80000000);

  //
  // Delay of 100uS needed after above pll_en_ctrl unmasking write to allow PLL to lock
  //
  MicroSecondDelay (100);

  DEBUG ((EFI_D_INFO, "UfsConfigureMphyToPwmG1() - End \n"));

  return EFI_SUCCESS;
}

/**
  The function perform the UFS PHY programming that required by silicon
  in early PEI phase before devices get initialized.

  @param[in] None

  @retval None
**/

VOID
ScUFSPhyInit (
  VOID
  )
{
  DEBUG ((EFI_D_INFO, "ScUFSPhyInit() - Start\n"));
  //
  // Set bit 22 in SCC register GPPRVRW1 to turn off chicken bit
  // Set dyn_mphy_pwr_en bit, allowing MPHY to operate: RMW GPPRVRW1 @ 0x600, set bit 22 to:1`b1
  //
  SideBandAndThenOr32(SCC_EP_PORT_ID, R_SCC_SB_GPPRVRW1, 0xFFDFFFFF, 0x200000);
  //
  // NOTE: Delay of 100uS after above dyn_mphy_pwr_en write (paranoia).
  //
  MicroSecondDelay (100);
  //
  // Minimal programming of MPHY to get to PWM-G1, set three registers in Intel MPHY Private Registers (IMPREGx)
  //
  UfsConfigureMphyToPwmG1();
  //
  // TODO: Read Over-ride MPHY settings
  //
  // Over-ride MPHY programming, Configure MPHY to High Speed
  //
  UfsConfigureMphyOverrides();


  DEBUG ((EFI_D_INFO, "ScUFSPhyInit() - End\n"));

  return;
}

/**
  ScUFSPhyPromgram

  @param[in] PeiServices

  @retval None
**/
EFI_STATUS
EFIAPI
ScUFSPhyPromgram (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS                  HobPtr;
  MBP_CURRENT_BOOT_MEDIA                *BootMediaData;

  //
  // Get Boot Media Hob
  //
  HobPtr.Guid   = GetFirstGuidHob (&gEfiBootMediaHobGuid);
  ASSERT (HobPtr.Guid != NULL);
  BootMediaData = (MBP_CURRENT_BOOT_MEDIA*) GET_GUID_HOB_DATA (HobPtr.Guid);

  if (BootMediaData->PhysicalData != BOOT_FROM_UFS) {  //Boot from UFS
    //
    // gCseUfsSelectPpiGuid is not found, means not boot from UFS.
    //
    ScUFSPhyInit ();
  }
  mIpcWbuffer[0] |= BIT1;
  return EFI_SUCCESS;
}


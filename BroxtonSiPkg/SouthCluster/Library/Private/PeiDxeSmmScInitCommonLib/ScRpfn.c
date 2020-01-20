/** @file
  This file contains functions that configures PCI Express Root Ports function swapping.

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
#include <Uefi/UefiBaseType.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MmPciLib.h>
#include <Library/ScPlatformLib.h>
#include <Private/Library/PeiDxeSmmScPciExpressHelpersLib.h>
#include <Private/Library/ScInitCommonLib.h>
#include <Library/S3BootScriptLib.h>

/**
  Configure root port function number mapping

  @retval EFI_SUCCESS  The function completed successfully
**/
EFI_STATUS
PcieConfigureRpfnMapping (
  VOID
  )
{
  UINT8      PortIndex;
  UINT8      MaxPciePortNum;
  UINT32     Data32;
  UINT16     Data16;
  UINTN      DevNum;
  UINTN      FuncNum;
  UINTN      RpBase;
  SC_SBI_PID RpPid;
  UINT8      RpPresence;

  DEBUG((DEBUG_INFO,"ScConfigureRpfnMapping () Start\n"));

  MaxPciePortNum = GetScMaxPciePortNum ();
  RpPresence     = 0;
  ///
  /// Configure root port function number mapping
  ///
  for (PortIndex = 0; PortIndex < MaxPciePortNum; ) {
    GetScPcieRpDevFun (PortIndex, &DevNum, &FuncNum);
    RpBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, (UINT32)DevNum, (UINT32)FuncNum);
    if (MmioRead16 (RpBase) != 0xFFFF) {
      RpPresence |= (BIT0 << PortIndex);
      if (FuncNum == 0) {
        //
        // If function number ZERO is present, no function swapping needed.
        // Just skip this device.
        //
        if (PortIndex == 0) {
          PortIndex = 2;
        } else if (PortIndex == 2) {
          PortIndex = 0xF;
        } else {
          PortIndex++;
        }
        continue;
      } else {
        //
        // If function number ZERO is not present, but others is. Need to do function swapping.
        // RP PCD register must sync with PSF RP function config register
        //
        RpPid = GetRpSbiPid (PortIndex);
        if (DevNum == PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1) {
          //
          // Set RP PCD function swapping register
          //
          PchPcrAndThenOr16 (
            RpPid, R_PCH_PCR_SPX_PCD,
            (UINT16)~(B_PCH_PCR_SPX_PCD_RP1FN << (PortIndex * S_PCH_PCR_SPX_PCD_RP_FIELD)),
            (UINT16)FuncNum
            );
          PchPcrRead16 (RpPid, R_PCH_PCR_SPX_PCD, &Data16);
          PCR_BOOT_SCRIPT_WRITE (
            S3BootScriptWidthUint16,
            RpPid, R_PCH_PCR_SPX_PCD,
            1,
            &Data16
            );
          //
          // Program PSF1 RP function config register.
          //
          PchPcrAndThenOr32 (
            PID_PSF1, R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D20_F0 - 4 * PortIndex,
            (UINT32)~B_PCH_PCR_PSFX_TX_AGENT_FUNCTION_CONFIG_FUNCTION,
            0
            );
          PchPcrRead32 (PID_PSF1, R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D20_F0 - 4 * PortIndex, &Data32);
          PCR_BOOT_SCRIPT_WRITE (
            S3BootScriptWidthUint32,
            PID_PSF1, R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D20_F0 - 4 * PortIndex,
            1,
            &Data32
            );
          PchPcrAndThenOr32 (
            PID_PSF1, R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D20_F0,
            (UINT32) ~B_PCH_PCR_PSFX_TX_AGENT_FUNCTION_CONFIG_FUNCTION,
            (UINT32) (FuncNum << N_PCH_PCR_PSFX_TX_AGENT_FUNCTION_CONFIG_FUNCTION)
            );
          PchPcrRead32 (PID_PSF1, R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D20_F0, &Data32);
          PCR_BOOT_SCRIPT_WRITE (
            S3BootScriptWidthUint32,
            PID_PSF1, R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D20_F0,
            1,
            &Data32
            );
        } else if (DevNum == PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2) {
          //
          // Set RP PCD function swapping register
          //
          PchPcrAndThenOr16 (
            RpPid, R_PCH_PCR_SPX_PCD,
            (UINT16)~(B_PCH_PCR_SPX_PCD_RP1FN << ((PortIndex - 2) * S_PCH_PCR_SPX_PCD_RP_FIELD)),
            (UINT16)FuncNum
            );
          PchPcrRead16 (RpPid, R_PCH_PCR_SPX_PCD, &Data16);
          PCR_BOOT_SCRIPT_WRITE (
            S3BootScriptWidthUint16,
            RpPid, R_PCH_PCR_SPX_PCD,
            1,
            &Data16
            );
          //
          // Program PSF1 RP function config register.
          //
          PchPcrAndThenOr32 (
            PID_PSF1, R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D19_F0 - (4 * (PortIndex - 2)),
            (UINT32)~B_PCH_PCR_PSFX_TX_AGENT_FUNCTION_CONFIG_FUNCTION,
            0
            );
          PchPcrRead32 (PID_PSF1, R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D19_F0 - (4 * (PortIndex - 2)), &Data32);
          PCR_BOOT_SCRIPT_WRITE (
            S3BootScriptWidthUint32,
            PID_PSF1, R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D19_F0 - (4 * (PortIndex - 2)),
            1,
            &Data32
            );
          PchPcrAndThenOr32 (
            PID_PSF1, R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D19_F0,
            (UINT32) ~B_PCH_PCR_PSFX_TX_AGENT_FUNCTION_CONFIG_FUNCTION,
            (UINT32) (FuncNum << N_PCH_PCR_PSFX_TX_AGENT_FUNCTION_CONFIG_FUNCTION)
            );
          PchPcrRead32 (PID_PSF1, R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D19_F0, &Data32);
          PCR_BOOT_SCRIPT_WRITE (
            S3BootScriptWidthUint32,
            PID_PSF1, R_PCH_PCR_PSF1_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D19_F0,
            1,
            &Data32
            );
        }
        if (PortIndex < 2) {
          PortIndex = 2;
        } else {
          PortIndex = 0xF;
        }
        continue;
      }
    }
    PortIndex++;
  }

  //
  // Also lock the SRL bit to make PCD register read only
  //
  // Avoid read/write to PCD register if the device 20 is not presented
  if ((RpPresence & (BIT1 | BIT0)) != 0) {
    PchPcrAndThenOr32 (
      PID_PCIE0, R_PCH_PCR_SPX_PCD,
      (UINT32)~0,
      B_PCH_PCR_SPX_PCD_SRL
     );
    PchPcrRead32 (PID_PCIE0, R_PCH_PCR_SPX_PCD, &Data32);
    PCR_BOOT_SCRIPT_WRITE (
      S3BootScriptWidthUint32,
      PID_PCIE0, R_PCH_PCR_SPX_PCD,
      1,
      &Data32
      );
  }
  // Avoid read/write to PCD register if the device 19 is not presented
  if ((RpPresence & (BIT5 | BIT4 | BIT3 | BIT2)) != 0) {
    PchPcrAndThenOr32 (
      PID_PCIE1, R_PCH_PCR_SPX_PCD,
      (UINT32)~0,
      B_PCH_PCR_SPX_PCD_SRL
      );
    PchPcrRead32 (PID_PCIE1, R_PCH_PCR_SPX_PCD, &Data32);
    PCR_BOOT_SCRIPT_WRITE (
      S3BootScriptWidthUint32,
      PID_PCIE1, R_PCH_PCR_SPX_PCD,
      1,
      &Data32
      );
  }

  return EFI_SUCCESS;
}

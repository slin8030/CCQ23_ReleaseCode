/** @file
  This file contains functions that initializes PCI Express Root Ports of PCH.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2017 Intel Corporation.

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

#include <Library/SteppingLib.h>
#include <Uefi/UefiBaseType.h>
#include "PciExpressInit.h"

/**
  Device information structure
**/
typedef struct {
  UINT16  Vid;
  UINT16  Did;
} PCIE_DEVICE_INFO;


/**
  This function returns mask of ports to be disabled based on Lane Owner and Policy

  @param[in]    ScPolicyPpi   The PCH Policy protocol

  @retval UINT32         Returns 32 bit mask of ports to be disabled. 1b = Disable. 0b = Enable.
**/
UINT32 GetRpDisableMask (
  IN      SC_POLICY_PPI  *ScPolicyPpi
)
{
  UINT32                  RpDisableMask;
  UINT8                   PortIndex;
  UINT8                   MaxPciePortNum;
  SC_PCIE_CONFIG          *PciExpressConfig;
  UINTN                   RpBase;
  UINTN                   RpDevice;
  UINTN                   RpFunction;
  EFI_STATUS              Status;

  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gPcieRpConfigGuid, (VOID *) &PciExpressConfig);
  ASSERT_EFI_ERROR (Status);

  MaxPciePortNum = GetScMaxPciePortNum ();
  RpDisableMask = 0;
  ///
  /// Root ports can be disabled in PchInitEarlyPei.c PchEarlyDisabledDeviceHandling()
  /// Get Root Port Disabled Status before starting
  ///
  for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
    GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
    RpBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, RpDevice, RpFunction);
    if (MmioRead16 (RpBase) == 0xFFFF) {
      RpDisableMask |= (1 << PortIndex);
    }
  }

  for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
    if ((PciExpressConfig->RootPort[PortIndex].Enable == 0) &&
        (PciExpressConfig->RootPort[PortIndex].HotPlug == 0)) {
      RpDisableMask |= (1 << PortIndex);
    }
    //
    // Function disable unused root port
    // 1.  Disable PCIe Port 1 if lane 3 is not owned by PCIe, PCR[FIA] + 250h bit [15:12] != 0
    // 2.  Disable PCIe Port 2 if lane 2 is not owned by PCIe, PCR[FIA] + 250h bit [11:08] != 0
    // 3.  Disable PCIe Port 3 if lane 7 is not owned by PCIe, PCR[FIA] + 250h bit [31:28] != 0
    // 4.  Disable PCIe Port 4 if lane 6 is not owned by PCIe, PCR[FIA] + 250h bit [27:24] != 0
    // 5.  Disable PCIe Port 5 if lane 5 is not owned by PCIe, PCR[FIA] + 254h bit [23:20] != 0
    // 6.  Disable PCIe Port 6 if lane 4 is not owned by PCIe, PCR[FIA] + 254h bit [19:16] != 0
    // 7. Perform the function disable handling as stated in section 8.2.1.1
    //
    if (IsPortAvailable (PcieP1 + PortIndex) != EFI_SUCCESS) {
      RpDisableMask |= (1 << PortIndex);
    }
  }

  return RpDisableMask;
}

/**
  PCIe controller configuration.
**/
typedef enum {
  Pcie2x1      = 0,
  Pcie1x2      = 1,
  Pcie4x1      = 2,
  Pcie1x2_2x1  = 3,
  Pcie2x2      = 4,
  Pcie1x4      = 5,
  PcieUnknow   = 6
} PCIE_RP_CONFIG_STRAP;

/**
  Checks if a given rootport owns its PCIE lane(s).
  In controllers configured as 1x4, 2x2 etc some ports have more than 1 lane, so other ports own none

  @param[in] RpIndex   Root Port Number
  @return TRUE         This port owns 1 or more lane
  @return FALSE        This port doesn't own any lane
**/
BOOLEAN
IsStandaloneRp (
  IN UINT8 RpIndex,
  IN UINTN RpDevice
  )
{
  PCIE_RP_CONFIG_STRAP PcieRpStrpFuseCfg = PcieUnknow;
  UINT8                FirstRpIndex;
  UINT32               Data32;
  //
  // Get PCI regsiter from SBI in case it's disabled.
  //
  if (RpIndex < 2) {
    FirstRpIndex = 0;
    PchSbiRpPciRead32 (
      RpDevice,
      FirstRpIndex,
      R_PCH_PCIE_STRPFUSECFG,
      &Data32
      );
  } else {
    FirstRpIndex = 2;
    PchSbiRpPciRead32 (
      RpDevice,
      FirstRpIndex,
      R_PCH_PCIE_STRPFUSECFG,
      &Data32
      );
  }

  Data32 = (Data32 & B_PCH_PCIE_STRPFUSECFG_RPC) >> N_PCH_PCIE_STRPFUSECFG_RPC;
  if (RpIndex < 2) {
    switch (Data32) {
      case 0 :
        PcieRpStrpFuseCfg = Pcie2x1;
        break;

      case 1 :
        PcieRpStrpFuseCfg = Pcie1x2;
        break;
    }
  } else {
    switch (Data32) {
      case 0 :
        PcieRpStrpFuseCfg = Pcie4x1;
        break;

      case 1 :
        PcieRpStrpFuseCfg = Pcie1x2_2x1;
        break;

      case 2 :
        PcieRpStrpFuseCfg = Pcie2x2;
        break;

      case 3 :
        PcieRpStrpFuseCfg = Pcie1x4;
        break;
    }
    //
    // Update the RpIndex of the PCIe1 (D19:F0~F3) to 0 Base
    //
    RpIndex -= 2;
  }

  switch (PcieRpStrpFuseCfg) {
    case Pcie2x1 :
      DEBUG ((DEBUG_INFO, "Port%x of Pcie2x1\n", RpIndex));
      return TRUE;
    case Pcie1x2 :
      DEBUG ((DEBUG_INFO, "Port%x of Pcie1x2\n", RpIndex));
      return ((RpIndex % 2) != 1);
    case Pcie4x1 :
      DEBUG ((DEBUG_INFO, "Port%x of Pcie4x1\n", RpIndex));
      return TRUE;
    case Pcie1x2_2x1 :
      DEBUG ((DEBUG_INFO, "Port%x of Pcie1x2_2x1\n", RpIndex));
      return ((RpIndex % 4) != 1);
    case Pcie2x2 :
      DEBUG ((DEBUG_INFO, "Port%x of Pcie2x2", RpIndex));
      return ((RpIndex % 2) != 1);
    case Pcie1x4 :
      DEBUG ((DEBUG_INFO, "Port%x of Pcie1x4\n", RpIndex));
      return ((RpIndex % 4) == 0);
    default:
      ASSERT (FALSE);
      return FALSE;
  }
}

/**
  This function disables root ports

  @param[in] PortIndex              Root Port Number
  @param[in] IsCardDetected         True:  Link Programming will not be executed
                                    False: Link Programming will be executed
  @param[in, out] FuncDisableReg    The PMC FUNC_DIS_0 register to be updated
  @param[in, out] FuncDisable1Reg   The PMC FUNC_DIS_1 register to be updated

  @retval EFI_SUCCESS           Root Port Function Disable successful
**/
EFI_STATUS
ScDisableRootPort (
  IN UINT8                PortIndex,
  IN BOOLEAN              IsCardDetected,
  IN OUT UINT32           *FuncDisableReg,
  IN OUT UINT32           *FuncDisable1Reg
)
{
  UINTN       RpBase;
  UINTN       RpDevice;
  UINTN       RpFunction;
  EFI_STATUS  Status;
  UINT32      LoopTime;
  UINT32      Data32;
  UINT8       FirstRpIndex;
  UINT32      TargetState;

  Data32       = 0;
  RpDevice     = 0xFF;
  RpFunction   = 0xFF;
  FirstRpIndex = 0;
  GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);

  DEBUG ((DEBUG_INFO, "ScDisableRootPort() Start %0x\n", PortIndex + 1));
  if (!IsCardDetected) {
    ///
    ///Link programming
    ///1.  Set B0:Dxx:Fn:338h[26] = 1b
    ///2.  Poll B0:Dxx:Fn:328h[31:24] until 0x1 or 50ms timeout
    ///3.  Set B0:Dxx:Fn +408h[27] =1b
    ///
    Status = PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PCIEALC, (UINT32)(~(B_PCH_PCIE_PCIEALC_BLKDQDA)), (UINT32)B_PCH_PCIE_PCIEALC_BLKDQDA);

    //
    // For first lane of a port owned by PCIE, poll until DETRDY
    // For all others (remaining lanes of a x2/x4 port and for lanes not owned by PCIE), wait intil DETRDYECINP1CG
    //
    if (IsStandaloneRp (PortIndex, RpDevice) && (IsPortAvailable (PcieP1 + PortIndex) == EFI_SUCCESS)) {
      TargetState = V_PCH_PCIE_PCIESTS1_LTSMSTATE_DETRDY;
    } else {
      TargetState = V_PCH_PCIE_PCIESTS1_LTSMSTATE_DETRDYECINP1CG;
    }

    for (LoopTime = 0; LoopTime < 500; LoopTime++) {
      Status = PchSbiRpPciRead32 (RpDevice, PortIndex, R_PCH_PCIE_PCIESTS1, &Data32);
      if (((Data32 & B_PCH_PCIE_PCIESTS1_LTSMSTATE) >> N_PCH_PCIE_PCIESTS1_LTSMSTATE) == TargetState) {
        break;
      } else {
        MicroSecondDelay (100);
      }
    }
    if (LoopTime == 500){
      ///What to do if timeout? Return Error?
      DEBUG((DEBUG_INFO, "Timeout waiting for LTSMSTATE to change to DETRDY. Data32 = %0x \n", Data32 ));
    }

    Status = PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PHYCTL4, (UINT32)(~(B_PCH_PCIE_PHYCTL4_SQDIS)),(UINT32) B_PCH_PCIE_PHYCTL4_SQDIS);
  }

  // Clear hotplug capable bits HPC and HPS
  RpBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, RpDevice, RpFunction);
  if (MmioRead16 (RpBase) == 0xFFFF) {
    Status = PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_SLCAP, (UINT32)(~(B_PCIE_SLCAP_HPC | B_PCIE_SLCAP_HPS)), 0);
  } else {
    MmioAnd32 (RpBase + R_PCH_PCIE_SLCAP, (UINT32)(~(B_PCIE_SLCAP_HPC | B_PCIE_SLCAP_HPS)));
  }

  if (PortIndex < 2) {
  ///
  ///PCIe RP IOSF Sideband register offset 0x00[19:16], depending on the port that is Function Disabled
  ///Access it by offset 0x02[4:0] to avoid RWO bit
  ///
    PchPcrAndThenOr8 (
        GetRpSbiPid(PortIndex),
        R_PCH_PCR_SPX_PCD + 0x02,
        0x0F,
        (1 << PortIndex)
    );
  ///
  ///Then the BIOS need to set the function disable register in PCIe RP and related PSF register, for example, root port 1:
  ///Set related PSF function disable bit for root port 1, PCR[PSF_1] + AGNT_T0_SHDW_PCIEN"[RP1][8] to 1
  ///
    if (GetBxtSeries() == BxtP) {
      if (PortIndex == 0) {
        PchPcrAndThenOr32 (
          PID_PSF1, (R_SC_BXTP_PCR_PSF1_T1_SHDW_PCIE01_REG_BASE + R_SC_PCR_PSF1_T1_SHDW_PCIEN),
          (UINT32)~0,
          B_SC_PCR_PSF1_T1_SHDW_PCIEN_FUNDIS
        );
      } else {
        PchPcrAndThenOr32 (
          PID_PSF1, (R_SC_BXTP_PCR_PSF1_T1_SHDW_PCIE02_REG_BASE + R_SC_PCR_PSF1_T1_SHDW_PCIEN),
          (UINT32)~0,
          B_SC_PCR_PSF1_T1_SHDW_PCIEN_FUNDIS
        );
      }
    } else {
      if (PortIndex == 0) {
        PchPcrAndThenOr32 (
          PID_PSF1, (R_SC_BXT_PCR_PSF1_T1_SHDW_PCIE01_REG_BASE + R_SC_PCR_PSF1_T1_SHDW_PCIEN),
          (UINT32)~0,
          B_SC_PCR_PSF1_T1_SHDW_PCIEN_FUNDIS
        );
      } else {
        PchPcrAndThenOr32 (
          PID_PSF1, (R_SC_BXT_PCR_PSF1_T1_SHDW_PCIE02_REG_BASE + R_SC_PCR_PSF1_T1_SHDW_PCIEN),
          (UINT32)~0,
          B_SC_PCR_PSF1_T1_SHDW_PCIEN_FUNDIS
        );
      }
    }
    FirstRpIndex = 0;
  } else {
    PchPcrAndThenOr8 (
        GetRpSbiPid(PortIndex),
        R_PCH_PCR_SPX_PCD + 0x02,
        0x0F,
        (1 << (PortIndex - 2))
    );
    PchPcrAndThenOr32 (
      PID_PSF1, ((R_SC_PCR_PSF1_T1_SHDW_PCIE03_REG_BASE + (0x100 * (PortIndex - 2))) + R_SC_PCR_PSF1_T1_SHDW_PCIEN),
      (UINT32)~0,
      B_SC_PCR_PSF1_T1_SHDW_PCIEN_FUNDIS
    );
    FirstRpIndex = 2;
  }

  ///
  /// Some power management steps needed along with function disable programming above:
  /// 1.  PCI offset E1h[6] to 1 (first root port of the controller)
  /// 2.  PCI offset E2h[4] to 1 (first root port of the controller)
  ///
  Status = PchSbiRpPciAndThenOr32 (RpDevice, FirstRpIndex , 0xE0, ~0u, 0x00104000);
  ///
  /// 3.  PCI offset 420h[31] to 1
  /// 4.  PCI offset 424h[8] to 1
  ///
  Status = PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PCIEPMECTL, (UINT32)(~(B_PCH_PCIE_PCIEPMECTL_FDPPGE)), (UINT32)B_PCH_PCIE_PCIEPMECTL_FDPPGE);
  Status = PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PCIEPMECTL2, (UINT32)(~(B_PCH_PCIE_PCIEPMECTL2_FDCPGE)), (UINT32)B_PCH_PCIE_PCIEPMECTL2_FDCPGE);

  if (PortIndex < 2) {
    *FuncDisableReg |= (B_PMC_FUNC_DIS_PCIE0_P0 >> PortIndex);
  } else {
    *FuncDisable1Reg |= (B_PMC_FUNC_DIS_1_PCIE1_P0 << (PortIndex - 2));
  }
  DEBUG ((DEBUG_INFO, "ScDisableRootPort() End\n"));
  return Status;
}

/**
  This function creates Capability and Extended Capability List

  @param[in] PortIndex    Root Port Number
  @param[in] *ScPolicyPpi   The Sc Policy protocol

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
InitCapabilityList (
  IN      UINT8             PortIndex,
  IN      SC_POLICY_PPI     *ScPolicyPpi
)
{
  UINTN          RpDevice;
  UINTN          RpFunction;
  UINTN          RpBase;
  UINT32         Data32;
  UINT16         Data16;
  UINT8          Data8;
  BOOLEAN        Gen3Enabled;
  UINT8          RpLinkSpeed;
  SC_PCIE_CONFIG *PcieRpConfig;
  EFI_STATUS     Status;

  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gPcieRpConfigGuid, (VOID *) &PcieRpConfig);
  ASSERT_EFI_ERROR (Status);

  Gen3Enabled = FALSE;
  RpDevice    = 0xFF;
  RpFunction  = 0xFF;

  ///
  /// Build Capability Linked List and Extended PCIe Capability Linked List
  ///
  GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
  RpBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, RpDevice, RpFunction);

  ///
  /// Check PCIe Gen3 Fuse and Policy Bits
  ///
  RpLinkSpeed   = MmioRead8 (RpBase + R_PCH_PCIE_LCAP) & B_PCIE_LCAP_MLS;
  if ((RpLinkSpeed == V_PCIE_LCAP_MLS_GEN3) && ((PcieRpConfig->RootPort[PortIndex].PcieSpeed == ScPcieAuto) ||
    (PcieRpConfig->RootPort[PortIndex].PcieSpeed == ScPcieGen3))) {
    Gen3Enabled = TRUE;
  }

  ///
  /// Build Capability linked list
  /// 1.  Read and write back to capability registers 34h, 41h, 81h and 91h using byte access.
  /// 2.  Program NSR, A4h[3] = 0b
  ///
  Data8 = MmioRead8 (RpBase + PCI_CAPBILITY_POINTER_OFFSET);
  MmioWrite8 (RpBase + PCI_CAPBILITY_POINTER_OFFSET, Data8);

  Data16 = MmioRead16 (RpBase + R_PCH_PCIE_CLIST);
  MmioWrite16 (RpBase + R_PCH_PCIE_CLIST, Data16);

  Data16 = MmioRead16 (RpBase + R_PCH_PCIE_MID);
  MmioWrite16 (RpBase + R_PCH_PCIE_MID, Data16);

  Data16 = MmioRead16 (RpBase + R_PCH_PCIE_SVCAP);
  MmioWrite16 (RpBase + R_PCH_PCIE_SVCAP, Data16);

  Data32 = MmioRead32 (RpBase + R_PCH_PCIE_PMCS);
  Data32 &= (UINT32)~(B_PCIE_PMCS_NSR);
  MmioWrite32 (RpBase + R_PCH_PCIE_PMCS, Data32);
  ///
  /// Build Extended PCIe Capability linked list
  /// 1.  To support Advanced Error Reporting
  /// i.  Set Next Capability Offset, Dxx:Fn + 100h[31:20] = 140h
  /// ii. Set Capability Version, Dxx:Fn + 100h[19:16] = 1h
  /// iii.  Set Capability ID, Dxx:Fn + 100h[15:0] = 0001h
  /// ELSE
  /// iv. Set Next Capability Offset, Dxx:Fn + 100h[31:20] = 140h
  /// v.  Set Capability Version, Dxx:Fn + 100h[19:16]  = 0h
  /// vi. Set Capability ID, Dxx:Fn + 100h[15:10]  = 0000h
  ///
  if (PcieRpConfig->RootPort[PortIndex].AdvancedErrorReporting) {
    Data32 = (V_PCH_PCIE_EX_AEC_CV << N_PCIE_EXCAP_CV) | V_PCIE_EX_AEC_CID;
  } else {
    Data32 = 0;
  }
  Data32 |= (R_PCH_PCIE_EX_ACSECH << N_PCIE_EXCAP_NCO);
  MmioWrite32 (RpBase + R_PCH_PCIE_EX_AECH, Data32);
  ///
  /// 2.  To support ACS
  /// i.  Set Next Capability Offset, Dxx:Fn + 140h[31:20] = 150h
  /// ii. Set Capability Version, Dxx:Fn + 140h[19:16] = 1h
  /// iii.  Set Capability ID, Dxx:Fn + 140h[15:0] = 000Dh
  /// iv. Read and write back to Dxx:Fn + 144h
  /// ELSE
  /// v.  Set Next Capability Offset, Dxx:Fn + 140h[31:20] = 150h
  /// vi. Set Capability Version, Dxx:Fn + 140h[19:16]  = 0h
  /// vii.  Set Capability ID, Dxx:Fn + 140h[15:10]  = 0000h
  ///
  if (PcieRpConfig->RootPort[PortIndex].AcsEnabled == TRUE) {
    Data32 = MmioRead32 (RpBase + R_PCH_PCIE_EX_ACSCAPR);
    MmioWrite32 (RpBase + R_PCH_PCIE_EX_ACSCAPR, Data32);

    Data32 = (V_PCH_PCIE_EX_ACS_CV << N_PCIE_EXCAP_CV) | V_PCIE_EX_ACS_CID;
  } else {
    Data32 = 0;
  }
  Data32 |= (R_PCH_PCIE_EX_PTMECH << N_PCIE_EXCAP_NCO);
  MmioWrite32 (RpBase + R_PCH_PCIE_EX_ACSECH, Data32);

  ///
  /// 3.  To support PTM
  /// i.  Set Next Capability Offset, Dxx:Fn + 150h[31:20] = 200h
  /// ii. Set Capability Version, Dxx:Fn + 150h[19:16] = 1h
  /// iii.  Set Capability ID, Dxx:Fn + 150h[15:0] = 001Fh
  /// iv. Read and write back to Dxx:Fn + 154h
  /// ELSE
  /// v.  Set Next Capability Offset, Dxx:Fn + 150h[31:20] = 200h
  /// vi. Set Capability Version, Dxx:Fn + 150h[19:16]  = 0h
  /// vii.  Set Capability ID, Dxx:Fn + 150h[15:10]  = 0000h
  ///
  Data32 = 0;
  if (PcieRpConfig->RootPort[PortIndex].PtmEnable == TRUE) {
    Data32 = MmioRead32 (RpBase + R_PCH_PCIE_EX_PTMCAPR);
    Data32 |= (B_PCIE_EX_PTMCAPR_PTMRSPC +  B_PCIE_EX_PTMCAPR_PTMRC);
    MmioWrite32 (RpBase + R_PCH_PCIE_EX_PTMCAPR, Data32);
    Data32 = (V_PCH_PCIE_EX_PTM_CV << N_PCIE_EXCAP_CV) | V_PCIE_EX_PTM_CID;
  }
  Data32 |= (R_PCH_PCIE_EX_L1SECH << N_PCIE_EXCAP_NCO);
  MmioWrite32 (RpBase + R_PCH_PCIE_EX_PTMECH, Data32);

  ///
  /// 4.  To support L1 Sub-State
  /// i.  Set Next Capability Offset, Dxx:Fn + 200h[31:20] = 220h
  /// ii. Set Capability Version, Dxx:Fn + 200h[19:16] = 1h
  /// iii.  Set Capability ID, Dxx:Fn + 200h[15:0] = 001Eh
  /// iv. Refer to section 8.3.1 for other requirements
  /// ELSE
  /// v.  Set Next Capability Offset, Dxx:Fn + 200h[31:20] = 220h
  /// vi. Set Capability Version, Dxx:Fn + 200h[19:16]  = 0h
  /// vii.  Set Capability ID, Dxx:Fn + 200h[15:10]  = 0000h
  ///
  if (PcieRpConfig->RootPort[PortIndex].L1Substates != ScPcieL1SubstatesDisabled) {
    Data32 = (V_PCH_PCIE_EX_L1S_CV << N_PCIE_EXCAP_CV) | V_PCIE_EX_L1S_CID;
  } else {
    Data32 = 0;
  }
  Data32 |= (R_PCH_PCIE_EX_SPEECH << N_PCIE_EXCAP_NCO);
  MmioWrite32 (RpBase + R_PCH_PCIE_EX_L1SECH, Data32);
  ///
  /// 5.  If the RP is GEN3 capable (by fuse and BIOS policy), enable Secondary PCI Express Extended Capability
  /// i.  Set Next Capability Offset, Dxx:Fn + 220h[31:20] = 000h
  /// ii. Set Capability Version, Dxx:Fn + 220h[19:16] = 1h
  /// iii.  Set Capability ID, Dxx:Fn + 220h[15:0] = 0019h
  /// ELSE
  /// iv. Set Next Capability Offset, Dxx:Fn + 220h[31:20] = 000h
  /// v.  Set Capability Version, Dxx:Fn + 220h[19:16]  = 0h
  /// vi. Set Capability ID, Dxx:Fn + 220h[15:10]  = 0000h
  ///
  if (Gen3Enabled) {
    Data32 = (V_PCH_PCIE_EX_SPEECH_CV << N_PCIE_EXCAP_CV) | V_PCIE_EX_SPE_CID;
  } else {
    Data32 = 0;
  }
  Data32 |= (V_PCIE_EXCAP_NCO_LISTEND << N_PCIE_EXCAP_NCO);
  MmioWrite32 (RpBase + R_PCH_PCIE_EX_SPEECH, Data32);

  return EFI_SUCCESS;
}


/**
  Initialize R/WO Registers

  @param[in] ScPolicyPpi    The SC Policy instance

  @retval EFI_SUCCESS             The function completed successfully
**/

EFI_STATUS
PciERWORegInit (
  IN      SC_POLICY_PPI        *ScPolicyPpi
  )
{

  UINTN          PortIndex;
  UINTN          RpDevice;
  UINTN          RpFunction;
  UINTN          RpBase;
  UINT32         Data32;
  UINT16         Data16;
  UINT8          MaxPciePortNum;
  SC_PCIE_CONFIG *PcieRpConfig;
  EFI_STATUS     Status;

  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gPcieRpConfigGuid, (VOID *) &PcieRpConfig);
  ASSERT_EFI_ERROR (Status);

  MaxPciePortNum = GetScMaxPciePortNum ();
  DEBUG ((DEBUG_INFO, "PciERWORegInit() Start\n"));

  ///
  /// System BIOS must read the register and write the same value back to the register
  /// before passing control to the operating system.
  /// Dev:Func/Type Register Offset   Register Name                                     Bits
  /// Dxx:F0-F7     034h              Capabilities Pointer                              7:0
  /// Dxx:F0-F7     040h              Capabilities List                                 15:8
  /// Dxx:F0-F7     080h              Message Signaled Interrupt Capability ID          15:8
  /// Dxx:F0-F7     090h              Subsystem Vendor Capability                       15:8
  /// Dxx:F0-F7     100h              Advanced Error Extended Reporting                 31:0
  /// Dxx:F0-F7     140h              ACS Extended Capability Header                    31:0
  /// Dxx:F0-F7     144h              ACS Capability Register                           3:0
  /// Dxx:F0-F7     200h              L1 Sub-States Extended Capability Header          31:0
  /// Dxx:F0-F7     220h              Secondary PCI Express Extended Capability Header  31:0
  /// Dxx:F0-F7     0A4h              PCI Power Management Control And Status           3
  /// ---- Above are done in InitCapabilityList()
  /// Dev:Func/Type Register Offset   Register Name                                     Bits
  /// Dxx:F0-F7     042h              PCI Express Capabilities                          8
  /// Dxx:F0-F7     050h              Link Control                                      3
  /// ---- Above are done in PchInitRootPorts()
  /// Dev:Func/Type Register Offset   Register Name                                     Bits
  /// Dxx:F0-F7     064h              Device Capabilities 2                             19:18, 11
  /// ---- Above are done in ScInitSingleRootPort()
  /// Dev:Func/Type Register Offset   Register Name                                     Bits
  /// Dxx:F0-F7     404h              Latency Tolerance Reporting Override 2            2
  /// ---- Above are done in PchPciExpressHelpersLibrary.c PcieSetPm ()
  /// Dev:Func/Type Register Offset   Register Name                                     Bits
  /// Dxx:F0~F7     044h              Device Capabilities                               2:0
  /// Dxx:F0-F7     04Ch              Link Capabilities                                 22, 17:15, 11:10
  /// Dxx:F0-F7     054h              Slot Capabilities                                 31:19, 16:5
  /// Dxx:F0-F7     094h              Subsystem Vendor ID                               31:0
  /// Dxx:F0-F7     0D8h              Miscellaneous Port Configuration                  23, 2
  /// Dxx:F0-F7     204h              L1 Sub-States Capabilities                        23:19, 17:16, 15:8, 4:0
  for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
    GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
    RpBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, RpDevice, RpFunction);

    if (MmioRead16 (RpBase + PCI_VENDOR_ID_OFFSET) != 0xFFFF) {
      Data16 = MmioRead16 (RpBase + R_PCH_PCIE_DCAP);
      MmioWrite16 (RpBase + R_PCH_PCIE_DCAP, Data16);

      Data32 = MmioRead32 (RpBase + R_PCH_PCIE_LCAP);
      MmioWrite32 (RpBase + R_PCH_PCIE_LCAP, Data32);

      Data32 = MmioRead32 (RpBase + R_PCH_PCIE_SLCAP);
      MmioWrite32 (RpBase + R_PCH_PCIE_SLCAP, Data32);

      Data32 = MmioRead32 (RpBase + R_PCH_PCIE_SVID);
      MmioWrite32 (RpBase + R_PCH_PCIE_SVID, Data32);
      //
      // Set
      // D8 [23] = 1 [RWO] Secure Register Lock
      // D8 [17:15] = 0x4 CCEL Common Clock Exit Latency
      //
      Data32 = MmioRead32 (RpBase + R_PCH_PCIE_MPC);
      Data32 &= (UINT32) ~(B_PCH_PCIE_MPC_CCEL);
      Data32 |= (B_PCH_PCIE_MPC_SRL | (4 << N_PCH_PCIE_MPC_CCEL));
      MmioWrite32 (RpBase + R_PCH_PCIE_MPC, Data32);

      Data32 = MmioRead32 (RpBase + R_PCH_PCIE_EX_L1SCAP);
      MmioWrite32 (RpBase + R_PCH_PCIE_EX_L1SCAP, Data32);
    }
  }

  DEBUG ((DEBUG_INFO, "PciERWORegInit() End\n"));

  return EFI_SUCCESS;
}

/**
  This is the function to enable the clock gating for PCI Express ports.

  @param[in] PortIndex            The root port number (zero based)
  @param[in] TrunkClockGateEn     Indicates whether trunk clock gating is to be enabled,
                                  requieres all controller ports to have dedicated CLKREQ#
                                  or to be disabled.

  @retval EFI_SUCCESS             Successfully completed.
**/
EFI_STATUS
PcieEnableClockGating (
  IN  UINT8    PortIndex,
  IN  BOOLEAN  TrunkClockGateEn
  )
{
  UINT32 Data32Or;
  UINT32 Data32And;
  UINT32 DataRead;
  UINTN  RpDevice;
  UINTN  RpFunction;
  UINT8  FirstRpIndex;

  DEBUG ((DEBUG_INFO, "PcieEnableClockGating () Start %0x\n", PortIndex + 1));
  Data32Or     = 0;
  Data32And    = ~0u;

  GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
  ///
  /// Program the settings which must apply to fisrt RP
  ///
  if (PortIndex == 0 || PortIndex == 2) {
    ///
    ///   Disable static clock gating by
    ///   Program first RP in each controller PCI offset E1h [5, 4, 2] = [1, 1, 1] , E2h [4] = [1] using sideband acces
    ///   Program Dxx:Fn + F7h [6] = 1
    ///
    Data32Or = (((B_PCH_PCIE_RPDCGEN_LCLKREQEN | B_PCH_PCIE_RPDCGEN_BBCLKREQEN |
                B_PCH_PCIE_RPDCGEN_SRDBCGEN) << 8) | (B_PCH_PCIE_RPPGEN_SEOSCGE << 16));
    Data32And = (UINT32) ~(B_PCH_PCIE_RPDCGEN_RPSCGEN << 8);
    PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, 0xE0, Data32And, Data32Or);
      PchSbiRpPciRead32(RpDevice, PortIndex, 0xE0, &DataRead);
    //
    //  CCFG - Channel Configuration
    //  PCI Offset: 0xD0~0xD3
    //  [15] 1b
    //
    PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_CCFG, (UINT32)~B_PCH_PCIE_CCFG_DCGEISMA, B_PCH_PCIE_CCFG_DCGEISMA);
  }
  if (TrunkClockGateEn) {
    if (PortIndex < 2) {
      FirstRpIndex = 0;
    } else {
      FirstRpIndex = 2;
    }
    Data32Or |= (B_PCH_PCIE_RPDCGEN_PTOCGE << 8);
    PchSbiRpPciAndThenOr32 (RpDevice, FirstRpIndex, 0xE0, ~0u, Data32Or);
  }

  ///
  /// Program the settings which apply to each port
  ///

  //
  //  RPDCGEN - Root Port Dynamic Clock Gate
  //  PCI Offset: 0xE1
  //  [1:0] 11b
  //
  Data32Or = (B_PCH_PCIE_RPDCGEN_RPDBCGEN | B_PCH_PCIE_RPDCGEN_RPDLCGEN) << 8;
  PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, 0xE0, Data32And, Data32Or);
  //
  //  IOSFSBCS: IOSF Sideband Control and Status
  //  PCI Offset: 0xF7
  //  [6] 1b
  //
  Data32And = (UINT32)~(B_PCH_PCIE_IOSFSBCS_SCPTCGE << 24);
  Data32Or = (B_PCH_PCIE_IOSFSBCS_SCPTCGE << 24);
  PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_F4, Data32And, Data32Or);

  return EFI_SUCCESS;
}

/**
  This is the function to enable the power gating for PCI Express ports.

  @param[in] PortIndex            The root port number (zero based)

  @retval EFI_SUCCESS             Successfully completed.
**/
EFI_STATUS
PcieEnablePowerGating (
  IN  UINT8                                     PortIndex
  )
{
  UINTN                    RpDevice;
  UINTN                    RpFunction;
  UINT32                   Data32Or;
  UINT32                   Data32And;

  DEBUG ((DEBUG_INFO, "PcieEnablePowerGating () Start %0x\n", PortIndex + 1));
  Data32Or = 0;
  Data32And = (UINT32)~(0);

  GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
  ///
  /// Program the settings which must apply to first RP
  ///
  if (PortIndex == 0 || PortIndex == 2) {
    //
    //  PWRCTL - Power Control
    //  PCI Offset: 0xE8~0xEB
    //  [17] 1b
    //
    Data32Or = (B_PCH_PCIE_PWRCTL_WPDMPGEP | B_PCH_PCIE_PWRCTL_DBUPI);
    PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PWRCTL, Data32And, Data32Or);
    //
    //  PCIEPMECTL2 - PCIe PM Extension Control
    //  PCI Offset: 0x424~0x427
    //  [11] 1b
    //
    Data32And =(UINT32)~B_PCH_PCIE_PCIEPMECTL2_PHYCLPGE;
    Data32Or = B_PCH_PCIE_PCIEPMECTL2_PHYCLPGE;
    PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PCIEPMECTL2, Data32And, Data32Or);
  }
  ///
  /// Program the settings which apply to each port
  ///
  GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
  //
  //  PCIEPMECTL - PCIe PM Extension Control
  //  PCI Offset: 0x420
  //  [31, 30, 29] 111b
  //
  Data32And = (UINT32)~(B_PCH_PCIE_PCIEPMECTL_FDPPGE | B_PCH_PCIE_PCIEPMECTL_DLSULPPGE | B_PCH_PCIE_PCIEPMECTL_DLSULDLSD | B_PCH_PCIE_PCIEPMECTL_L1LE);
  Data32Or = (B_PCH_PCIE_PCIEPMECTL_FDPPGE | B_PCH_PCIE_PCIEPMECTL_DLSULPPGE | B_PCH_PCIE_PCIEPMECTL_DLSULDLSD | B_PCH_PCIE_PCIEPMECTL_L1FSOE);
  PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PCIEPMECTL, Data32And, Data32Or);

  //
  //  PCIEPMECTL2 - PCIe PM Extension Control
  //  PCI Offset: 0x424~0x427
  //  [7:4] all 1's
  //
  Data32And = 0xFFFFFF0F;
  Data32Or = (B_PCH_PCIE_PCIEPMECTL2_DETSCPGE | B_PCH_PCIE_PCIEPMECTL2_L23RDYSCPGE | B_PCH_PCIE_PCIEPMECTL2_DISSCPGE | B_PCH_PCIE_PCIEPMECTL2_L1SCPGE);
  PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PCIEPMECTL2, Data32And, Data32Or);

  //
  //  Power Control Enable
  //  PCI Offset: 0x428
  //  [5, 1] = [1, 0]
  //
  Data32And = (UINT32)~(B_PCH_PCIE_PCE_PMCRE);
  Data32Or = B_PCH_PCIE_PCE_HAE;
  PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PCE, Data32And, Data32Or);

  return EFI_SUCCESS;
}

/**
  This is the function to enable Compliance Mode

  @param[in] PortIndex            The root port number (zero based)

  @retval EFI_SUCCESS             Successfully completed.
**/
EFI_STATUS
PcieEnableComplianceMode (
  IN  UINT8           PortIndex
  )
{
  UINTN                    RpDevice;
  UINTN                    RpFunction;
  UINT32                   Data32Or;
  UINT32                   Data32And;

  DEBUG ((DEBUG_INFO, "PcieEnableComplianceMode () %0x\n", PortIndex + 1));
  Data32Or  = 0;
  Data32And = ~0u;

  GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);

  // Clock force on
  SideBandAndThenOr32 (PID_PCLKD, 0x100C, ~0u, (BIT3|BIT2|BIT1|BIT0));

  // Disable Dynamic Clock Gating
  if (PortIndex == 0 || PortIndex == 2) {
    Data32And = (UINT32) ~(B_PCH_PCIE_RPDCGEN_SRDBCGEN << 8);
  }
  Data32And &= (UINT32) ~((B_PCH_PCIE_RPDCGEN_RPDBCGEN | B_PCH_PCIE_RPDCGEN_RPDLCGEN) << 8);
  PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, 0xE0, Data32And, Data32Or);

  // Disable Power management
  Data32And = (UINT32)~(B_PCH_PCIE_PCIEPMECTL_DLSULPPGE);
  PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PCIEPMECTL, Data32And, Data32Or);

  return EFI_SUCCESS;
}

/**
  This is the function to configure precision time measurement

  @param[in] PortIndex            The root port number (zero based)

  @retval EFI_SUCCESS             Successfully completed.
**/
EFI_STATUS
PcieConfigurePtm (
  IN  UINT8  PortIndex
  )
{
  UINT32 Data32Or;
  UINT32 Data32And;
  UINTN  RpDevice;
  UINTN  RpFunction;

  DEBUG ((DEBUG_INFO, "PcieConfigurePtm () Start %0x\n", PortIndex + 1));
  Data32Or = 0;
  Data32And = (UINT32)~(0);

  GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
  //
  // Program the settings which must apply to first RP
  //
  if (PortIndex == 0 || PortIndex == 2) {
    Data32Or = 0x20012001;
    PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PTMPSDC1, Data32And, Data32Or);
    Data32Or = 0x16031603;
    PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PTMPSDC2, Data32And, Data32Or);
    Data32Or = 0x16031603;
    PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PTMPSDC3, Data32And, Data32Or);
    Data32Or = 0x40050;
    PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_PTMECFG, Data32And, Data32Or);
  }

  return EFI_SUCCESS;
}

/**
  This is the function to configure PLL shutdown

  @param[in] PortIndex            The root port number (zero based)

  @retval EFI_SUCCESS             Successfully completed.
**/
EFI_STATUS
PcieConfigurePllShutdown (
  IN  UINT8                PortIndex
  )
{
  UINT32                   Data32Or;
  UINT32                   Data32And;
  UINTN                    RpDevice;
  UINTN                    RpFunction;

  Data32Or = 0;
  Data32And = (UINT32)~(0);

  //
  // Program the settings which must apply to first RP
  //
  if (PortIndex == 0 || PortIndex == 2) {
    GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
    //
    //  PHYCTL2 - Physical Layer and AFE Control 2
    //  PCI Offset: 0xF5
    //  [3:0] = 111b
    //

    //
    //  IOSFSBCS: IOSF Sideband Control and Status
    //  PCI Offset: 0xF7
    //  [3:2] = 00b
    //
    Data32And = (UINT32)~(B_PCH_PCIE_IOSFSBCS_SIID << 24);
    Data32Or = ((B_PCH_PCIE_PHYCTL2_PXPG3PLLOFFEN | B_PCH_PCIE_PHYCTL2_PXPG2PLLOFFEN | B_PCH_PCIE_PHYCTL2_BLKPLLEN) << 8);
    PchSbiRpPciAndThenOr32 (RpDevice, PortIndex, R_PCH_PCIE_F4, Data32And, Data32Or);
  }

  return EFI_SUCCESS;
}

/**
  The function to change the root port speed based on policy

  @param[in] ScPolicyPpi The PCH Policy PPI instance

  @retval EFI_SUCCESS             Succeeds.
**/
EFI_STATUS
ScPcieRpSpeedChange (
  IN  SC_POLICY_PPI           *ScPolicyPpi
  )
{
  EFI_STATUS            Status;
  UINTN                 PortIndex;
  UINTN                 MaxPciePortNum;
  UINTN                 PciRootPortRegBase[PCIE_MAX_ROOT_PORTS];
  UINTN                 RpDevice;
  UINTN                 RpFunction;
  UINTN                 LinkRetrainedBitmap;
  UINTN                 TimeoutCount;
  SC_PCIE_CONFIG        *PcieRpConfig;

  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gPcieRpConfigGuid, (VOID *) &PcieRpConfig);
  ASSERT_EFI_ERROR (Status);

  MaxPciePortNum = GetScMaxPciePortNum ();
  //
  // Since we are using the root port base many times, it is best to cache them.
  //
  for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
    GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
    PciRootPortRegBase[PortIndex] = MmPciBase (
                                      DEFAULT_PCI_BUS_NUMBER_SC,
                                      RpDevice,
                                      RpFunction
                                      );

  }

  ///
  /// NOTE: Detection of Non-Complaint PCI Express Devices
  ///
  LinkRetrainedBitmap = 0;
  for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
    if (MmioRead16 (PciRootPortRegBase[PortIndex] + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
      continue;
    }

    if (PcieRpConfig->RootPort[PortIndex].PcieSpeed == ScPcieGen1) {
      MmioAndThenOr16 (PciRootPortRegBase[PortIndex] + R_PCH_PCIE_LCTL2,
                       (UINT16) ~(B_PCIE_LCTL2_TLS),
                       V_PCIE_LCTL2_TLS_GEN1);
    } else if ((PcieRpConfig->RootPort[PortIndex].PcieSpeed >= ScPcieGen2) ||
               (PcieRpConfig->RootPort[PortIndex].PcieSpeed == ScPcieAuto)) {
      MmioAndThenOr16 (PciRootPortRegBase[PortIndex] + R_PCH_PCIE_LCTL2,
                       (UINT16) ~(B_PCIE_LCTL2_TLS),
                       V_PCIE_LCTL2_TLS_GEN2);
    } else {
      continue;
    }

    //
    // Retrain link
    //
    MmioOr16 (PciRootPortRegBase[PortIndex] + R_PCH_PCIE_LCTL, B_PCIE_LCTL_RL);
    LinkRetrainedBitmap |= (1u << PortIndex);
  }

  //
  // 15 ms timeout while checking for link active on retrained link
  //
  for (TimeoutCount = 0; ((LinkRetrainedBitmap != 0) && (TimeoutCount < 150)); TimeoutCount++) {
    //
    // Delay 100 us
    //
    MicroSecondDelay (100);
    //
    // Check for remaining root port which was link retrained
    //
    for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
      if ((LinkRetrainedBitmap & (1u << PortIndex)) != 0) {
        //
        // If the link is active, clear the bitmap
        //
        if (MmioRead16 (PciRootPortRegBase[PortIndex] + R_PCH_PCIE_LSTS) & B_PCIE_LSTS_LA) {
          LinkRetrainedBitmap &= ~(1u << PortIndex);
        }
      }
    }
  }

  //
  // If 15 ms has timeout, and some link are not active, train to gen1
  //
  if (LinkRetrainedBitmap != 0) {
    for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
      if ((LinkRetrainedBitmap & (1u << PortIndex)) != 0) {
        //
        // Set TLS to gen1
        //
        MmioAndThenOr16 (PciRootPortRegBase[PortIndex] + R_PCH_PCIE_LCTL2,
                         (UINT16) ~(B_PCIE_LCTL2_TLS),
                         V_PCIE_LCTL2_TLS_GEN1);
        //
        // Retrain link
        //
        MmioOr16 (PciRootPortRegBase[PortIndex] + R_PCH_PCIE_LCTL, B_PCIE_LCTL_RL);
      }
    }

    //
    // Wait for retrain completion or timeout in 15ms. Do not expect failure as
    // port was detected and trained as Gen1 earlier
    //
    for (TimeoutCount = 0; ((LinkRetrainedBitmap != 0) && (TimeoutCount < 150)); TimeoutCount++) {
      //
      // Delay 100 us
      //
      MicroSecondDelay (100);
      //
      // Check for remaining root port which was link retrained
      //
      for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
        if ((LinkRetrainedBitmap & (1u << PortIndex)) != 0) {
          //
          // If the link is active, clear the bitmap
          //
          if (MmioRead16 (PciRootPortRegBase[PortIndex] + R_PCH_PCIE_LSTS) & B_PCIE_LSTS_LA) {
            LinkRetrainedBitmap &= ~(1u << PortIndex);
          }
        }
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  Get information about the endpoint

  @param[in]  RpBase      Root port base address
  @param[in]  TempPciBus  Temporary bus number
  @param[out] DeviceInfo  Device information structure

  @raturn TRUE if device was found, FALSE otherwise
**/
BOOLEAN
GetDeviceInfo (
  IN  UINTN             RpBase,
  IN  UINT8             TempPciBus,
  OUT PCIE_DEVICE_INFO  *DeviceInfo
  )
{
  UINTN                   EpBase;
  UINT32                  Data32;

  DeviceInfo->Vid = 0xFFFF;
  DeviceInfo->Did = 0xFFFF;

  //
  // Check for device presence
  //
  if ((MmioRead16 (RpBase + R_PCH_PCIE_SLSTS) & B_PCIE_SLSTS_PDS) == 0) {
    return FALSE;
  }

  //
  // Assign temporary bus numbers to the root port
  //
  MmioAndThenOr32 (
    RpBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
    (UINT32)~B_PCI_BRIDGE_BNUM_SBBN_SCBN,
    ((UINT32) (TempPciBus << 8)) | ((UINT32) (TempPciBus << 16))
  );

  //
  // A config write is required in order for the device to re-capture the Bus number,
  // according to PCI Express Base Specification, 2.2.6.2
  // Write to a read-only register VendorID to not cause any side effects.
  //
  EpBase = MmPciBase (TempPciBus, 0, 0);
  MmioWrite16 (EpBase + PCI_VENDOR_ID_OFFSET, 0);

  Data32 = MmioRead32 (EpBase + PCI_VENDOR_ID_OFFSET);
  DeviceInfo->Vid = (UINT16) (Data32 & 0xFFFF);
  DeviceInfo->Did = (UINT16) (Data32 >> 16);

  //
  // Clear bus numbers
  //
  MmioAnd32 (RpBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET, (UINT32)~B_PCI_BRIDGE_BNUM_SBBN_SCBN);

  DEBUG ((DEBUG_INFO, "VID: %04X DID: %04X \n",
          DeviceInfo->Vid, DeviceInfo->Did));

  return (Data32 != 0xFFFFFFFF);
}


/**
  This function checks if de-emphasis needs to be changed from default for a given rootport

  @param[in] DevInfo      Information on device that is connected to rootport

  @retval TRUE            De-emphasis needs to be changed
  @retval FALSE           No need to change de-emphasis
**/
BOOLEAN
NeedDecreasedDeEmphasis (
  IN PCIE_DEVICE_INFO      DevInfo
  )
{
  //
  // Intel WiGig devices
  //
  if (DevInfo.Vid == V_INTEL_VENDOR_ID && DevInfo.Did == 0x097C) {
    return TRUE;
  }
  return FALSE;
}

/**
  Perform Initialization of the Downstream Root Ports.

  @param[in] ScPolicyPpi             The SC Policy protocol
  @param[in] TempPciBusMin           The temporary minimum Bus number for root port initialization
  @param[in] TempPciBusMax           The temporary maximum Bus number for root port initialization
  @param[in] OriginalFuncDisableReg  The PMC FUNC_DIS_0 register from PMC
  @param[in] OriginalFuncDisable1Reg The PMC FUNC_DIS_1 register from PMC
  @param[in, out] FuncDisableReg     The PMC FUNC_DIS_0 register to be updated
  @param[in, out] FuncDisable1Reg    The PMC FUNC_DIS_1 register to be updated

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ScInitRootPorts (
  IN SC_POLICY_PPI        *ScPolicyPpi,
  IN UINT8                TempPciBusMin,
  IN UINT8                TempPciBusMax,
  IN UINT32               *OriginalFuncDisableReg,
  IN UINT32               *OriginalFuncDisable1Reg,
  IN OUT UINT32           *FuncDisableReg,
  IN OUT UINT32           *FuncDisable1Reg
  )
{
  SC_PCIE_CONFIG          *PciExpressConfig;
  EFI_STATUS              Status;
  UINT32                  Data32And;
  UINT32                  Data32Or;
  UINT8                   PortIndex;
  UINTN                   RpBase;
  UINT8                   MaxPciePortNum;
  UINTN                   RpDevice;
  UINTN                   RpFunction;
  UINTN                   PmcBaseAddress;
  UINT16                  AcpiBaseAddr;
  UINT32                  RpDisableMask;
  BOOLEAN                 PortWithMaxPayload128;
  UINT8                   RpLinkSpeed;
  UINT16                  DeviceId;
//[-start-170622-IB07400878-add]//
#ifdef PCIE_GEN1_UNDOCUMENTED_WORKAROUND
  UINT8                   Data8And;
  UINT8                   Data8Or;
#endif  
//[-end-170622-IB07400878-add]//

  DEBUG ((DEBUG_INFO, "ScInitRootPorts() Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gPcieRpConfigGuid, (VOID *) &PciExpressConfig);
  ASSERT_EFI_ERROR (Status);

  Status            = EFI_SUCCESS;
  RpDisableMask     = 0;
  Data32And         = 0xFFFFFFFF;
  Data32Or          = 0;
  RpDevice          = 0xFF;
  RpFunction        = 0xFF;
  RpBase            = 0;
  PmcBaseAddress    = PMC_BASE_ADDRESS;
  AcpiBaseAddr      = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  MaxPciePortNum    = GetScMaxPciePortNum ();
  PortWithMaxPayload128 = FALSE;

  RpDisableMask = GetRpDisableMask (ScPolicyPpi);

  for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
    //
    // Skip the Root Port init if the PCIE controller is disabled by PMC.
    //
    if (PortIndex < 2) {
      if (((B_PMC_FUNC_DIS_PCIE0_P0 | B_PMC_FUNC_DIS_PCIE0_P1) == (*OriginalFuncDisableReg & (B_PMC_FUNC_DIS_PCIE0_P0 | B_PMC_FUNC_DIS_PCIE0_P1)))
          && (PciExpressConfig->RootPort[PortIndex].Enable != 1)) {
        *FuncDisableReg |= (B_PMC_FUNC_DIS_PCIE0_P0 >> PortIndex);
        continue;
      }
    } else {
      if ((B_PMC_FUNC_DIS_1_PCIE1_P0 | B_PMC_FUNC_DIS_1_PCIE1_P1 | B_PMC_FUNC_DIS_1_PCIE1_P2 | B_PMC_FUNC_DIS_1_PCIE1_P3) == (*OriginalFuncDisable1Reg & (B_PMC_FUNC_DIS_1_PCIE1_P0 | B_PMC_FUNC_DIS_1_PCIE1_P1 | B_PMC_FUNC_DIS_1_PCIE1_P2 | B_PMC_FUNC_DIS_1_PCIE1_P3))
          && (PciExpressConfig->RootPort[PortIndex].Enable != 1)) {
        *FuncDisable1Reg |= (B_PMC_FUNC_DIS_1_PCIE1_P0 << (PortIndex - 2));
        continue;
      }
    }
    GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
    RpBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, RpDevice, RpFunction);
    
//[-start-170622-IB07400878-add]//
#ifdef PCIE_GEN1_UNDOCUMENTED_WORKAROUND
    //
    // Force Training in Gen1/Gen2 speed
    //
    Data8And = (UINT8)(~((UINT8)(B_PCH_PCIE_MPC_PCIESD >> 8)));
    Data8Or = 0;
    switch (PciExpressConfig->RootPort[PortIndex].PcieSpeed) {
      case ScPcieGen1:
        Data8Or |= (V_PCH_PCIE_MPC_PCIESD_GEN1 << (N_PCH_PCIE_MPC_PCIESD - 8));
        MmioAndThenOr8 (RpBase + R_PCH_PCIE_MPC + 1, Data8And, Data8Or);
        MicroSecondDelay (100);
        break;
      case ScPcieGen2:
        Data8Or |= (V_PCH_PCIE_MPC_PCIESD_GEN2 << (N_PCH_PCIE_MPC_PCIESD - 8));
        MmioAndThenOr8 (RpBase + R_PCH_PCIE_MPC + 1, Data8And, Data8Or);
        break;
      case ScPcieGen3:
      case ScPcieAuto:
        break;
    }
#endif    
//[-end-170622-IB07400878-add]//

    ///
    /// set CLKREQ to Root Port Mapping
    ///
    Status = SetPortClkReqNumber (PciExpressConfig, PortIndex);

    if ((RpDisableMask & (BIT0 << PortIndex)) == 0) {
      ///
      /// Set the Slot Implemented Bit.  Note that this must be set before
      /// presence is valid.
      /// initialize the "Slot Implemented" bit of the PCI Express* Capabilities Register,
      /// XCAP Dxx:Fn:42h[8] of each available and enabled downstream root port.
      /// Setting this bit will indicate that the PCI Express* link associated with this
      /// port is connected to a slot (as compared to being connected to an integrated
      /// device component).
      ///
      MmioOr16 (RpBase + R_PCH_PCIE_XCAP, B_PCIE_XCAP_SI);
    }
    ///
    /// If the port PCI config is hidden (cause by the early boot step, because of
    /// the Root Port Configuration soft-strap does not map this port to any
    /// PHY lanes or the port is function disabled by fused), function disable the port
    ///
    if (MmioRead16 (RpBase) == 0xFFFF && (PciExpressConfig->RootPort[PortIndex].Enable != 1)) {
      ScDisableRootPort(PortIndex, FALSE, FuncDisableReg, FuncDisable1Reg);
      ///
      /// Else if the port is not mapped to any PHY lanes
      /// (Lane Muxing soft-straps and fuses do not map this port to any PHY lanes),
      /// function disable the port
      ///
    } else if ((RpDisableMask & (BIT0 << PortIndex)) != 0 && (PciExpressConfig->RootPort[PortIndex].Enable != 1)) {
      ///
      /// If the port is not hot plug enable and a PCIe card is detected,
      /// and BIOS wants to disable the port,
      /// BIOS should set link disable at B0:Dxx:Fn:50h[4]
      /// followed by function disable the port
      ///
      if ((MmioRead16 (RpBase + R_PCH_PCIE_SLSTS) & B_PCIE_SLSTS_PDS) != 0) {
        MmioOr16 ((RpBase + R_PCH_PCIE_LCTL), (UINT16) B_PCIE_LCTL_LD);
        ScDisableRootPort(PortIndex, TRUE, FuncDisableReg, FuncDisable1Reg);
      } else {
        ScDisableRootPort(PortIndex, FALSE, FuncDisableReg, FuncDisable1Reg);
      }
      ///
      /// Else if the port is not hot plug enable
      /// and no PCIe card is detected
      /// or no more ClkReq signals are available
      ///
      /// If the user forces the port to be enabled, don't function disable the root port
    } else if ((((MmioRead16 (RpBase + R_PCH_PCIE_SLSTS) & B_PCIE_SLSTS_PDS) == 0) &&
              (PciExpressConfig->RootPort[PortIndex].HotPlug == 0) && (PciExpressConfig->RootPort[PortIndex].Enable != 1))) {
      RpDisableMask |= (BIT0 << PortIndex);
      ScDisableRootPort(PortIndex, FALSE, FuncDisableReg, FuncDisable1Reg);
    } else {
      /// Completion Retry Status Replay Enable
      /// Following reset it is possible for a device to terminate the
      /// configuration request but indicate that it is temporarily unable to process it,
      /// but in the future. The device will return the Configuration Request Retry Status.
      /// By setting the Completion Retry Status Replay Enable, Dxx:Fn + 320h[22],
      /// the RP will re-issue the request on receiving such status.
      /// The BIOS shall set this bit before first configuration access to the endpoint.
      MmioOr32 (RpBase + R_PCH_PCIE_PCIECFG2, B_PCH_PCIE_PCIECFG2_CRSREN);
      ///
      /// Configure the rootports
      ///
      Status = ScInitSingleRootPort (
                PortIndex,
                ScPolicyPpi,
                PmcBaseAddress,
                AcpiBaseAddr,
                &PortWithMaxPayload128,
                TempPciBusMin,
                TempPciBusMax
                );
      if (!EFI_ERROR (Status)) {
        DEBUG ((DEBUG_INFO, " Root Port %x device enabled. RpDisableMask: 0x%x ClkReq:%0x \n",
          PortIndex + 1,
          RpDisableMask,
          PciExpressConfig->RootPort[PortIndex].ClkReqNumber));
      }
      ///
      /// Additional PCI Express* Programming Steps
      ///
      /// Set "Link Speed Training Policy", Dxx:Fn + D4h[6] to 1.
      /// Make sure this is after mod-PHY related programming is completed.
      MmioOr32 (RpBase + R_PCH_PCIE_MPC2, B_PCH_PCIE_MPC2_LSTP);

      ///
      /// If Transmitter Half Swing is enabled, program the following sequence
      /// a. Ensure that the link is in L0.
      /// b. Program the Link Disable bit (0x50[4]) to 1b.
      /// c. Program the Analog PHY Transmitter Voltage Swing bit (0xE8[13]) to set the transmitter swing to half/full swing
      /// d. Program the Link Disable bit (0x50[4]) to 0b.
      /// BIOS can only enable this on SKU where GEN3 capability is fused disabled on that port
      RpLinkSpeed   = MmioRead8 (RpBase + R_PCH_PCIE_LCAP) & B_PCIE_LCAP_MLS;
      if ((RpLinkSpeed < V_PCIE_LCAP_MLS_GEN3) &&
        ((PciExpressConfig->RootPort[PortIndex].TransmitterHalfSwing) &&
        (((MmioRead32 (RpBase + R_PCH_PCIE_PCIESTS1) & (B_PCH_PCIE_PCIESTS1_LNKSTAT)) >> N_PCH_PCIE_PCIESTS1_LNKSTAT) == V_PCH_PCIE_PCIESTS1_LNKSTAT_L0))) {
        MmioOr8 (RpBase + R_PCH_PCIE_LCTL, B_PCIE_LCTL_LD);

        MmioOr16 (RpBase + R_PCH_PCIE_PWRCTL, B_PCH_PCIE_PWRCTL_TXSWING);

        MmioAnd8 (RpBase + R_PCH_PCIE_LCTL, (UINT8) ~(B_PCIE_LCTL_LD));
      }

    }
    //
    // Program Power Management Settings
    //
    if (!PciExpressConfig->DisableRootPortClockGating) {
      // TrunkClockGateEn depends on each of the controller ports supporting CLKREQ# or being disabled
      PcieEnableClockGating (
      PortIndex,
      ((((RpDisableMask >> PortIndex) & BIT0) | PciExpressConfig->RootPort[PortIndex].ClkReqSupported) == TRUE)
      );
    }
    //
    // Program Power gating
    //
    PcieEnablePowerGating (PortIndex);
    //
    // Compliance Mode
    //
    if (PciExpressConfig->ComplianceTestMode == TRUE) {
      PcieEnableComplianceMode (PortIndex);
    }
    //
    // Program Precision Time Measurement (PTM)
    //
    if (GetBxtSeries() == BxtP) {
      PcieConfigurePtm (PortIndex);
    }
    //
    // Proram PLL shutdown
    //
    PcieConfigurePllShutdown (PortIndex);
  }

  ///
  /// set CLKREQ to Root Port Mapping
  /// After the configuration done, set PCR[FIA] + 0h bit [31, 17, 16, 15] to [1, 1, 1, 1]
  ///
  PchPcrAndThenOr32 (PID_FIA, R_SC_PCR_FIA_CC,(UINT32) ~0, (BIT31 | BIT17 | BIT16 | BIT15));

  for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
    GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
    RpBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, RpDevice, RpFunction);
    if (MmioRead16 (RpBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
      continue;
    }
    /// If peer memory writes are supported in the platform and there is at least one device connected
    /// to any of the PCH Root Port that supports only 128B max payload size, BIOS must program all
    /// the PCH Root Ports such that upstream posted writes are split at 128B boundary by setting
    /// UNSD and CCFG.UPSD to 0 and CCFG.UPRS to 000b
    Data32And = ~0u;
    Data32Or  = 0;
    DeviceId = MmioRead16 (RpBase + PCI_DEVICE_ID_OFFSET);
    if (!(IS_BXT_P_PCIE_DEVICE_ID (DeviceId) || IS_BXT_PCIE_DEVICE_ID (DeviceId)) ) {
      if ((PciExpressConfig->EnablePeerMemoryWrite) && (PortWithMaxPayload128)) {
       Data32And &= ~(B_PCH_PCIE_CCFG_UPSD | B_PCH_PCIE_CCFG_UPRS);
      }
    } else {
      //Since no DMI on BXT, need to set 64Byte Split for all Pcie ports
      // UNSD = 0, UPSD = 0, UNRS = 111b, UPRS = 111b
      Data32And &= (UINT32)~(B_PCH_PCIE_CCFG_UPSD | B_PCH_PCIE_CCFG_UPRS);
      Data32Or  |= (UINT32)(B_PCH_PCIE_CCFG_UNRS | B_PCH_PCIE_CCFG_UPRS);
    }
    //
    // Set Peer Disable
    // Set B0:Dy:Fn + D0h [26:25] to 00b
    //
    Data32And &= (UINT32) ~(B_PCH_PCIE_CCFG_UMRPD | B_PCH_PCIE_CCFG_UPMWPD);
    MmioAnd32 (RpBase + R_PCH_PCIE_CCFG, Data32And);
    //
    // Set B0:Dy:Fn + D0h [13:12] to 01b
    //
    Data32And &= (UINT32) ~B_PCH_PCIE_CCFG_UNRD;
    Data32Or  |=  (1u << N_PCH_PCIE_CCFG_UNRD);
    MmioAndThenOr32 (RpBase + R_PCH_PCIE_CCFG, Data32And, Data32Or);
    ///
    /// Enable PCIe Relaxed Order to always allow downstream completions to pass posted writes. To enable feature set the following registers:
    /// 1)Set B0:Dxx:Fn:320h[24] = 1b
    Data32Or = (B_PCH_PCIE_PCIECFG2_CROAOV | B_PCH_PCIE_PCIECFG2_LBWSSTE);
    MmioOr32 (RpBase + R_PCH_PCIE_PCIECFG2, Data32Or);
    ///
    /// PCI Bus Emulation & Port80 Decode Support
    /// The I/O cycles within the 80h-8Fh range can be explicitly claimed
    /// by the PCIe RP by setting MPC.P8XDE, PCI offset D8h[26] = 1 (using byte access)
    ///
    if ((PciExpressConfig->EnablePort8xhDecode) &&
      (PciExpressConfig->ScPciePort8xhDecodePortIndex == PortIndex)) {
      MmioOr8 (RpBase + R_PCH_PCIE_MPC + 3, (UINT8) (B_PCH_PCIE_MPC_P8XDE >> 24));
      ///
      /// Set PCIE RP PCI offset ECh[2] = 1b
      ///
      Data32Or = B_PCH_PCIE_DC_PCIBEM;
      MmioOr32 (RpBase + R_PCH_PCIE_DC, Data32Or);
      ///
      /// PCI Bus Emulation & Port80 Decode Support
      /// BIOS must also configure the corresponding DMI registers GCS.RPR and GCS.RPRDID
      /// to enable DMI to forward the Port8x cycles to the corresponding PCIe RP
      ///
      //TODO PchIoPort80DecodeSet (PciExpressConfig->PciePort8xhDecodePortIndex);
    }
  }
  ///
  /// Enable PCIe Relaxed Order to always allow downstream completions to pass posted writes. To enable feature set the following registers:
  /// 2)PSF_1_PSF_PORT_CONFIG_PG0_PORT0 bit [1] = [1b]
  SideBandAndThenOr32 (PID_PSF1, R_SC_PCR_PSF1_PORT_CONFIG_PG0_PORT0, ~0u, BIT1);


  //
  // Program the root port target link speed based on policy
  //
  Status = ScPcieRpSpeedChange (ScPolicyPpi);
  ASSERT_EFI_ERROR (Status);

  //
  // Program R/WO Registers
  //
#if RVVP_ENABLE == 0
  PciERWORegInit(ScPolicyPpi);
#else
  DEBUG ((DEBUG_ERROR, "PciERWOReg not Init because of RVVP\n"));
#endif

  DEBUG ((DEBUG_INFO, "ScInitRootPorts() End\n"));

  return EFI_SUCCESS;
}

/**
  Perform Root Port Initialization.

  @param[in] PortIndex              The root port to be initialized (zero based)
  @param[in] ScPolicyPpi            The ScPolicy PPI
  @param[in] PmcBaseAddress         The Power Management Controller Base Address
  @param[in] AcpiBaseAddr           The PM I/O Base address of the SC
  @param[out] PortWithMaxPayload128 At least one Port with MaxPayload set to 128 bits
  @param[in] TempPciBusMin          The temporary minimum Bus number for root port initialization
  @param[in] TempPciBusMax          The temporary maximum Bus number for root port initialization

  @retval EFI_SUCCESS               Device found. The root port must be enabled.
  @retval EFI_NOT_FOUND             No device is found on the root port. It may be disabled.
  @exception EFI_UNSUPPORTED        Unsupported operation.
**/
EFI_STATUS
ScInitSingleRootPort (
  IN  UINT8                                     PortIndex,
  IN  SC_POLICY_PPI                             *ScPolicyPpi,
  IN  UINTN                                     PmcBaseAddress,
  IN  UINT16                                    AcpiBaseAddr,
  OUT BOOLEAN                                   *PortWithMaxPayload128,
  IN  UINT8                                     TempPciBusMin,
  IN  UINT8                                     TempPciBusMax
  )
{
  EFI_STATUS                        Status;
  UINTN                             RpDevice;
  UINTN                             RpFunction;
  UINTN                             RpBase;
  UINT32                            CapOffset;
  UINT8                             BusNumber;
  UINT32                            Data32;
  UINT32                            Data32Or;
  UINT32                            Data32And;
  UINT16                            Data16;
  UINT16                            Data16Or;
  UINT16                            Data16And;
  UINT8                             Data8Or;
  UINT8                             Data8And;
  SC_PCIE_CONFIG                    *PcieRpConfig;
  CONST SC_PCIE_ROOT_PORT_CONFIG    *RootPortConfig;
  BOOLEAN                           DeviceFound;
  UINT16                            DeviceId;
  PCIE_DEVICE_INFO                  DevInfo;

  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gPcieRpConfigGuid, (VOID *) &PcieRpConfig);
  ASSERT_EFI_ERROR (Status);

  DeviceFound     = FALSE;
  RootPortConfig  = &PcieRpConfig->RootPort[PortIndex];
  BusNumber       = DEFAULT_PCI_BUS_NUMBER_SC;

  RpFunction   = 0xFF;
  RpDevice     = 0xFF;
  DEBUG ((DEBUG_INFO, "ScInitSingleRootPort () Start %0x\n", PortIndex + 1));

  GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
  RpBase = MmPciBase (BusNumber, RpDevice, RpFunction);

  CapOffset = PcieFindCapId (
                BusNumber,
                (UINT8) RpDevice,
                (UINT8) RpFunction,
                EFI_PCI_CAPABILITY_ID_PCIEXP
                );

  if (CapOffset == 0) {
    return EFI_UNSUPPORTED;
  }

  GetDeviceInfo (RpBase, TempPciBusMin, &DevInfo);
  if (NeedDecreasedDeEmphasis (DevInfo) || (RootPortConfig->SelectableDeemphasis == TRUE)) {
    MmioOr32 (RpBase + R_PCH_PCIE_LCTL2, B_PCIE_LCTL2_LSTS2_SD);
  }

  ///
  /// Optimizer Configuration
  ///
  /// If B0:Dxx:Fn + 400h is programmed, BIOS will also program B0:Dxx:Fn + 404h [1:0] = 11b,
  /// to enable these override values.
  /// - Fn refers to the function number of the root port that has a device attached to it.
  /// - Default override value for B0:Dxx:Fn + 400h should be 880F880Fh
  /// - Also set 404h[2] to lock down the configuration
  /// - Refer to table below for the 404h[3] policy bit behavior.
  /// Done in PcieSetPm()
  ///
  /// Power Optimizer Configuration
  /// Program B0:Dxx:Fn + 64h [11] = 1b
  ///
  Data32Or = 0;
  Data32And = ~0u;
  if (RootPortConfig->LtrEnable == TRUE) {
    Data32Or |= B_PCIE_DCAP2_LTRMS;
  } else {
    Data32And &= (UINT32) ~(B_PCIE_DCAP2_LTRMS);
  }
  ///
  /// Optimized Buffer Flush/Fill (OBFF) is not supported.
  /// Program B0:Dxx:Fn + 64h [19:18] = 0h
  ///
  Data32And &= (UINT32) ~B_PCIE_DCAP2_OBFFS;
  MmioAndThenOr32 (RpBase + R_PCH_PCIE_DCAP2, Data32And, Data32Or);
  ///
  /// Program B0:Dxx:Fn + 68h [10] = 1b
  ///
  Data16 = MmioRead16 (RpBase + R_PCH_PCIE_DCTL2);
  if (RootPortConfig->LtrEnable == TRUE) {
    Data16 |= B_PCIE_DCTL2_LTREN;
  } else {
    Data16 &= (UINT16) ~(B_PCIE_DCTL2_LTREN);
  }
  MmioWrite16 (RpBase + R_PCH_PCIE_DCTL2, Data16);
  ///
  /// Step 3 Done in PchPciExpressHelpersLibrary.c ConfigureLtr ()
  ///


  ///
  /// Set Dxx:Fn + 478h[31:24] = 28h (Gen3 Active State L0s Preparation Latency)
  ///
  Data32Or = 0x28000000;
  Data32And = (UINT32) (~ 0xFF000000);
  MmioAndThenOr32 (RpBase + R_PCH_PCIE_G3L0SCTL, Data32And, Data32Or);
  ///
  /// Set Dxx:Fn + 318h [31:16] = 1414h (Gen2 and Gen1 Active State L0s Preparation Latency)
  ///
  Data32Or  = 0x14140000;
  Data32And = (UINT32) ~(B_PCH_PCIE_PCIEL0SC_G2ASL0SPL | B_PCH_PCIE_PCIEL0SC_G1ASL0SPL);
  MmioAndThenOr32 (RpBase + R_PCH_PCIE_PCIEL0SC, Data32And, Data32Or);

  //
  //  PCIERTP1 - PCI Express Replay Timer Policy 1
  //  PCI Offset: 0x300~0x303
  //  [23:20, 19:16, 15:12, 11:8, 7:4, 3:0] = [0xB, 0x7, 0x5, 0xF, 0xA, 0x7]
  //
  Data32And = (UINT32)~0xffffff;
  Data32Or = 0xB75FA7;
  MmioAndThenOr32 (RpBase + R_PCH_PCIE_PCIERTP1, Data32And,  Data32Or);

  //
  //  PCIERTP2 - PCI Express Replay Timer Policy 2
  //  PCI Offset: 0x304~307
  //  [11:8, 7:4, 3:0] = [0xC, 0x9, 0x7]
  //
  Data32And = (UINT32)~0xfff;
  Data32Or = 0xC97;
  MmioAndThenOr32 (RpBase + R_PCH_PCIE_PCIERTP2, Data32And,  Data32Or);

  ///
  /// Enable squelch by
  /// 1.  Program Dxx:Fn + 324h[7, 5] = [0, 1]
  /// 2.  Program Dxx:Fn + E8h[1, 0] = [1, 0]
  /// Step 26
  /// Set Link Clock Domain Squelch Exit Debounce Timers, Dxx:Fn + 324[25:24] = 00b. Can combine with step 9
  ///
  Data32And = (UINT32) ~(B_PCH_PCIE_PCIEDBG_LGCLKSQEXITDBTIMERS | B_PCH_PCIE_PCIEDBG_SQOL0);
  Data32Or = B_PCH_PCIE_PCIEDBG_SPCE;
  MmioAndThenOr32 (RpBase + R_PCH_PCIE_PCIEDBG, Data32And, Data32Or);

  Data32Or = B_PCH_PCIE_PWRCTL_RPL1SQPOL | B_PCH_PCIE_PWRCTL_DLP;
  MmioOr32 (RpBase + R_PCH_PCIE_PWRCTL, Data32Or);

  ///
  /// If Dxx:Fn + F5h[0] = 1b or step 3 is TRUE, set Dxx:Fn + 4Ch[17:15] = 100b
  /// Else set Dxx:Fn + 4Ch[17:15] = 010b
  /// Set ASPM Optionality Compliance, Dxx:Fn + 4Ch[22] = 1b, done as part of the step 12, default value is being used.
  ///
  Data32 = 0x00;
  PchSbiRpPciRead32 (RpDevice, PortIndex, R_PCH_PCIE_PHYCTL2, &Data32);
  if (Data32 & B_PCH_PCIE_PHYCTL2_PXPG2PLLOFFEN) {
    Data32Or = BIT17;
  } else {
    Data32Or = BIT16;
  }
  Data32And = (UINT32) (~B_PCIE_LCAP_EL1);

  ///
  /// Set LCAP APMS according to platform policy.
  ///
  if (RootPortConfig->Aspm < ScPcieAspmAutoConfig) {
    Data32And &= (UINT32) ~B_PCIE_LCAP_APMS;
    Data32Or  |= RootPortConfig->Aspm << N_PCIE_LCAP_APMS;
  } else if (RootPortConfig->Aspm == ScPcieAspmAutoConfig) {
    Data32And &= (UINT32) ~B_PCIE_LCAP_APMS;
    Data32Or  |= 0x3 << N_PCIE_LCAP_APMS;
  }
  //
  // The EL1, ASPMOC and APMS of LCAP are RWO, must program all together.
  //
  MmioAndThenOr32 (RpBase + R_PCH_PCIE_LCAP, Data32And, Data32Or);

  ///
  /// Configure PCI Express Number of Fast Training Sequence for each port
  /// 1.  Set Dxx:Fn + 314h [31:24, 23:16, 15:8, 7:0] to [7Eh, 70h, 3Fh, 38h]
  /// 2.  Set Dxx:Fn + 478h [15:8, 7:0] to [36h, 1Bh] (Gen3 Unique Clock N_FTS, Gen3 Common Clock N_FTS
  ///
  Data32  = 0x7E703F38;
  MmioWrite32 (RpBase + R_PCH_PCIE_PCIENFTS, Data32);

  Data32And = (UINT32) ~(B_PCH_PCIE_G3L0SCTL_G3UCNFTS | B_PCH_PCIE_G3L0SCTL_G3CCNFTS);
  Data32Or = 0x361B;
  MmioAndThenOr32 (RpBase + R_PCH_PCIE_G3L0SCTL, Data32And, Data32Or);

  ///
  /// Set Common Clock Exit Latency,  Dxx:Fn + D8h[17:15] = 3h
  ///
  Data32And = (UINT32) (~B_PCIE_MPC_CCEL);
  Data32Or  = 3 << N_PCIE_MPC_CCEL;
  switch (PcieRpConfig->RootPort[PortIndex].PcieSpeed) {
  case ScPcieGen1:
    Data32Or |= (V_PCIE_MPC_PCIESD_GEN1 << N_PCIE_MPC_PCIESD);
    break;
  case ScPcieGen2:
    Data32Or |= (V_PCIE_MPC_PCIESD_GEN2 << N_PCIE_MPC_PCIESD);
    break;
  case ScPcieGen3:
  case ScPcieAuto:
    break;
  }
  MmioAndThenOr32 (RpBase + R_PCH_PCIE_MPC, Data32And, Data32Or);

  ///
  /// Set MPC.IRRCE, Dxx:Fn + D8h[25] = 1b using byte access
  /// For system that support MCTP over PCIE set
  /// Set PCIE RP PCI offset D8h[27] = 0b
  /// Set PCIE RP PCI offset D8h[3] = 1b
  ///
  Data8And = (UINT8) ~((B_PCH_PCIE_MPC_IRRCE | B_PCH_PCIE_MPC_MMBNCE) >> 24);
  Data8Or = B_PCH_PCIE_MPC_IRRCE >> 24;
  MmioAndThenOr8 (RpBase + R_PCH_PCIE_MPC + 3, Data8And, Data8Or);
  DeviceId = MmioRead16 (RpBase + PCI_DEVICE_ID_OFFSET);
  if (!(IS_BXT_P_PCIE_DEVICE_ID (DeviceId) || IS_BXT_PCIE_DEVICE_ID (DeviceId)) ) {
    Data8And = (UINT8) ~(B_PCH_PCIE_MPC_MCTPSE);
    Data8Or  = B_PCH_PCIE_MPC_MCTPSE ;
    MmioAndThenOr8 (RpBase + R_PCH_PCIE_MPC, Data8And, Data8Or);
  }

  ///
  /// Set PCIE RP PCI offset F5h[7:4] = 0000b
  ///
  MmioAnd8 (RpBase + R_PCH_PCIE_PHYCTL2, (UINT8) ~(B_PCH_PCIE_PHYCTL2_TDFT | B_PCH_PCIE_PHYCTL2_TXCFGCHGWAIT));

  ///
  /// Enable PME_TO Time-Out Policy, Dxx:Fn + E2h[6] =1b
  ///
  Data8Or   = B_PCH_PCIE_RPPGEN_PTOTOP;
  Data8And  = (UINT8) ~B_PCH_PCIE_RPPGEN_PTOTOP;
  MmioAndThenOr8 (RpBase + R_PCH_PCIE_RPPGEN, Data8And, Data8Or);
  ///
  /// Enable Completion Time-Out Non-Fatal Advisory Error, Dxx:Fn + 324h[14] = 1b
  ///
  MmioOr32 (RpBase + R_PCH_PCIE_PCIEDBG, B_PCH_PCIE_PCIEDBG_CTONFAE);
  ///
  /// Configure Transmitter Preset for each Upstream and Downstream Port Lane:
  /// 1.  Set L01EC.DPL0TP, Dxx:Fn + 22Ch[3:0]    = 0001b
  /// 2.  Set L01EC.UPL0TP, Dxx:Fn + 22Ch[11:8]   = 0111b
  /// 3.  Set L01EC.DPL1TP, Dxx:Fn + 22Ch[19:16]  = 0010b
  /// 4.  Set L01EC.UPL1TP, Dxx:Fn + 22Ch[27:24]  = 0111b
  /// 5.  Set L23EC.DPL2TP, Dxx:Fn + 230h[3:0]    = 0111b
  /// 6.  Set L23EC.UPL2TP, Dxx:Fn + 230h[11:8]   = 0111b
  /// 7.  Set L23EC.DPL3TP, Dxx:Fn + 230h[19:16]  = 0111b
  /// 8.  Set L23EC.UPL3TP, Dxx:Fn + 230h[27:24]  = 0111b
  ///
  Data32And = (UINT32) ~(B_PCIE_EX_L01EC_UPL1TP | B_PCIE_EX_L01EC_DPL1TP | B_PCIE_EX_L01EC_UPL0TP | B_PCIE_EX_L01EC_DPL0TP);
  //
  // clear DP/UPL 0/1 RPH
  //
  Data32And &= (UINT32) ~(B_PCH_PCIE_EX_L01EC_DPL0RPH | B_PCH_PCIE_EX_L01EC_UPL0RPH | B_PCH_PCIE_EX_L01EC_DPL1RPH | B_PCH_PCIE_EX_L01EC_UPL1RPH);

  Data32Or = (V_PCH_PCIE_EX_L01EC_UPL1TP | V_PCH_PCIE_EX_L01EC_DPL1TP | V_PCH_PCIE_EX_L01EC_UPL0TP | V_PCH_PCIE_EX_L01EC_DPL0TP);
  MmioAndThenOr32 (RpBase + R_PCH_PCIE_EX_L01EC, Data32And, Data32Or);

  Data32And = (UINT32) ~(B_PCIE_EX_L23EC_UPL3TP | B_PCIE_EX_L23EC_DPL3TP | B_PCIE_EX_L23EC_UPL2TP | B_PCIE_EX_L23EC_DPL2TP);
  //
  // clear DP/UPL 2/3 RPH
  //
  Data32And &= (UINT32) ~(B_PCH_PCIE_EX_L23EC_UPL3RPH | B_PCH_PCIE_EX_L23EC_DPL3RPH | B_PCH_PCIE_EX_L23EC_UPL2RPH | B_PCH_PCIE_EX_L23EC_DPL2RPH);
  Data32Or = (V_PCH_PCIE_EX_L23EC_UPL3TP | V_PCH_PCIE_EX_L23EC_DPL3TP | V_PCH_PCIE_EX_L23EC_UPL2TP | V_PCH_PCIE_EX_L23EC_DPL2TP);
  MmioAndThenOr32 (RpBase + R_PCH_PCIE_EX_L23EC, Data32And, Data32Or);

  ///
  /// Enable EQ TS2 in Recovery Receiver Config, Dxx:Fn + 450h[7]= 1b
  ///
  MmioOr32 (RpBase + R_PCH_PCIE_EQCFG1, B_PCH_PCIE_EQCFG1_EQTS2IRRC);

  ///
  /// If there is no IOAPIC behind the root port, set EOI Forwarding Disable bit (PCIE RP PCI offset D4h[1]) to 1b.
  /// Done in PchPciExpressHelpersLibrary.c PcieSetEoiFwdDisable ()
  /// For Empty Hot Plug Slot, set is done in ScInitSingleRootPort ()
  ///

  ///
  /// System bios should initiate link retrain for all slots that has card populated after register restoration.
  /// Done in PchPciExpressHelpersLibrary.c PchPcieInitRootPortDownstreamDevices ()
  ///

  ///
  /// Configure Extended Synch
  ///
  if (RootPortConfig->ExtSync) {
    Data16And = (UINT16) (-1);
    Data16Or  = B_PCIE_LCTL_ES;
  } else {
    Data16And = (UINT16) (~B_PCIE_LCTL_ES);
    Data16Or  = 0;
  }
  MmioAndThenOr16 (RpBase + R_PCH_PCIE_LCTL, Data16And, Data16Or);

  ///
  /// Set PME timeout to 10ms
  ///
  Data32And = (UINT32) (~B_PCH_PCIE_PCIECFG2_PMET);
  Data32Or  = V_PCH_PCIE_PCIECFG2_PMET << N_PCH_PCIE_PCIECFG2_PMET;
  MmioAndThenOr32 (RpBase + R_PCH_PCIE_PCIECFG2, Data32And, Data32Or);

  ///
  /// Configure Completion Timeout
  ///
  Data16And = (UINT16)~(B_PCIE_DCTL2_CTD | B_PCIE_DCTL2_CTV);
  Data16Or  = 0;
  if (RootPortConfig->CompletionTimeout == ScPcieCompletionTO_Disabled) {
    Data16Or = B_PCIE_DCTL2_CTD;
  } else {
    switch (RootPortConfig->CompletionTimeout) {
    case ScPcieCompletionTO_Default:
      Data16Or = V_PCIE_DCTL2_CTV_DEFAULT;
      break;

    case ScPcieCompletionTO_16_55ms:
      Data16Or = V_PCIE_DCTL2_CTV_40MS_50MS;
      break;

    case ScPcieCompletionTO_65_210ms:
      Data16Or = V_PCIE_DCTL2_CTV_160MS_170MS;
      break;

    case ScPcieCompletionTO_260_900ms:
      Data16Or = V_PCIE_DCTL2_CTV_400MS_500MS;
      break;

    case ScPcieCompletionTO_1_3P5s:
      Data16Or = V_PCIE_DCTL2_CTV_1P6S_1P7S;
      break;

    default:
      Data16Or = 0;
      break;
    }
  }

  MmioAndThenOr16 (RpBase + R_PCH_PCIE_DCTL2, Data16And, Data16Or);

  if (RootPortConfig->SlotImplemented) {
    ///
    /// Slot Implemented enabled earlier. Here will only save this register for enabled ports
    ///
    Data16Or  = BIT8;
    Data16And = 0xFFFF;
    ///
    /// For Root Port Slots Numbering on the CRBs.
    ///
    Data32Or  = 0;
    Data32And = (UINT32) (~(B_PCIE_SLCAP_SLV | B_PCIE_SLCAP_SLS | B_PCIE_SLCAP_PSN));
    ///
    /// Note: If Hot Plug is supported, then write a 1 to the Hot Plug Capable (bit6) and Hot Plug
    /// Surprise (bit5) in the Slot Capabilities register, PCIE RP PCI offset 54h. Otherwise,
    /// write 0 to the bits PCIe Hot Plug SCI Enable
    ///
    Data32And &= (UINT32) (~(B_PCIE_SLCAP_HPC | B_PCIE_SLCAP_HPS));
    if (RootPortConfig->HotPlug) {
      Data32Or |= B_PCIE_SLCAP_HPC | B_PCIE_SLCAP_HPS;
    }
    ///
    /// Get the width from LCAP
    /// Slot Type  X1  X2/X4/X8/X16
    /// Default     10W   25W
    /// The slot power consumption and allocation is platform specific. Please refer to the
    /// "PCI Express* Card Electromechanical (CEM) Spec" for details.
    ///
    if (RootPortConfig->SlotPowerLimitValue != 0) {
        Data32Or |= (UINT32) (RootPortConfig->SlotPowerLimitValue << 7);
        Data32Or |= (UINT32) (RootPortConfig->SlotPowerLimitScale << 15);
    } else {
      if ((((MmioRead32 (RpBase + R_PCH_PCIE_LCAP)) & B_PCIE_LCAP_MLW) >> N_PCIE_LCAP_MLW) == 0x01) {
        Data32Or |= (UINT32) (100 << 7);
        Data32Or |= (UINT32) (1 << 15);
      } else if ((((MmioRead32 (RpBase + R_PCH_PCIE_LCAP)) & B_PCIE_LCAP_MLW) >> N_PCIE_LCAP_MLW) >= 0x02) {
        Data32Or |= (UINT32) (250 << 7);
        Data32Or |= (UINT32) (1 << 15);
      }
    }
    ///
    /// Initialize Physical Slot Number for Root Ports
    ///
    Data32Or |= (UINT32) (RootPortConfig->PhysicalSlotNumber << 19);
    MmioAndThenOr32 (RpBase + R_PCH_PCIE_SLCAP, Data32And, Data32Or);
  }
  InitCapabilityList (PortIndex, ScPolicyPpi);

  ///
  /// Maximum Payload Size
  /// Note: For hotplug supported ports,
  /// regardless of the capability of device attached during boot,
  /// set the DCTL.MPS as 128B
  ///
  if (RootPortConfig->HotPlug) {
    MmioAnd16 (RpBase + R_PCH_PCIE_DCAP, (UINT16) ~B_PCIE_DCAP_MPS);
  }
  ///
  /// Initialize downstream devices
  ///
  Status = PchPcieInitRootPortDownstreamDevices (
            BusNumber,
            (UINT8) RpDevice,
            (UINT8) RpFunction,
            TempPciBusMin,
            TempPciBusMax,
            PortWithMaxPayload128
            );
  if (Status == EFI_SUCCESS) {
    DeviceFound = TRUE;
  }
  ///
  /// Additional PCI Express* Programming Steps
  /// Set "Poisoned TLP Non-Fatal Advisory Error Enable", Dxx:Fn + D4h[12] to 1
  ///
  /// Disable the forwarding of EOI messages if no device is present.
  /// Set PCIE RP PCI offset D4h [1] = 1b
  ///
  Data32Or = B_PCH_PCIE_MPC2_PTNFAE;
//  if (!DeviceFound) {
  Data32Or |= B_PCH_PCIE_MPC2_EOIFD;
//  }
  /// Set PCI offset D4h [9] = 1b
  Data32Or |= B_PCH_PCIE_MPC2_TLPF;
  MmioOr32 (RpBase + R_PCH_PCIE_MPC2, Data32Or);
  ///
  /// Additional configurations
  ///
  ///
  /// Configure Error Reporting policy in the Device Control Register
  ///
  Data16And = (UINT16) (~(B_PCIE_DCTL_URE | B_PCIE_DCTL_FEE | B_PCIE_DCTL_NFE | B_PCIE_DCTL_CEE));
  Data16Or  = 0;

  if (RootPortConfig->UnsupportedRequestReport) {
    Data16Or |= B_PCIE_DCTL_URE;
  }

  if (RootPortConfig->FatalErrorReport) {
    Data16Or |= B_PCIE_DCTL_FEE;
  }

  if (RootPortConfig->NoFatalErrorReport) {
    Data16Or |= B_PCIE_DCTL_NFE;
  }

  if (RootPortConfig->CorrectableErrorReport) {
    Data16Or |= B_PCIE_DCTL_CEE;
  }
  //
  //   DCTL - Device Control
  //  PCI Offset: 0x48~0x49
  //  [7:5] = [001] 256B Max Payload Size
  //
  Data16Or |= (1 << N_PCIE_DCTL_MPS);
  MmioAndThenOr16 (RpBase + R_PCH_PCIE_DCTL, Data16And, Data16Or);

  ///
  /// Configure Interrupt / Error reporting in R_PCH_PCIE_RCTL
  ///
  Data16And = (UINT16) (~(B_PCIE_RCTL_PIE | B_PCIE_RCTL_SFE | B_PCIE_RCTL_SNE | B_PCIE_RCTL_SCE));
  Data16Or  = 0;

  if (RootPortConfig->PmeInterrupt) {
    Data16Or |= B_PCIE_RCTL_PIE;
  }

  if (RootPortConfig->SystemErrorOnFatalError) {
    Data16Or |= B_PCIE_RCTL_SFE;
  }

  if (RootPortConfig->SystemErrorOnNonFatalError) {
    Data16Or |= B_PCIE_RCTL_SNE;
  }

  if (RootPortConfig->SystemErrorOnCorrectableError) {
    Data16Or |= B_PCIE_RCTL_SCE;
  }

  MmioAndThenOr16 (RpBase + R_PCH_PCIE_RCTL, Data16And, Data16Or);

  ///
  /// Root PCI-E Powermanagement SCI Enable
  ///
  if (RootPortConfig->PmSci) {
    ///
    /// BIOS Enabling of Intel PCH PCI Express* PME SCI Generation
    ///
    /// Make sure that PME Interrupt Enable bit, Dxx:Fn:Reg 5Ch[3] is cleared
    ///
    Data16And = (UINT16) (~B_PCIE_RCTL_PIE);
    Data16Or  = 0;
    MmioAnd16 (RpBase + R_PCH_PCIE_RCTL, Data16And);

    ///
    /// Make sure GPE0 Register (AcpiBaseAddr+20h[9]), PCI_EXP_STS is 0, clear it if not zero
    ///
    Data32Or = IoRead32 (AcpiBaseAddr + R_ACPI_GPE0a_STS);
    if ((Data32Or & B_ACPI_GPE0a_STS_PCI_EXP) != 0) {
      Data32Or = B_ACPI_GPE0a_STS_PCI_EXP;
      IoWrite32 (AcpiBaseAddr + R_ACPI_GPE0a_STS, Data32Or);
    }
    ///
    /// Set BIOS_PCI_EXP_EN bit, PMC PCI offset A0h[10],
    /// to globally enable the setting of the PCI_EXP_STS bit by a PCI Express* PME event.
    ///
    Data32 = MmioRead32 (PmcBaseAddress + R_PMC_GEN_PMCON_2);
    if ((Data32 & B_PMC_GEN_PMCON_BIOS_PCI_EXP_EN) == 0) {
    MmioOr32 (PmcBaseAddress + R_PMC_GEN_PMCON_2, B_PMC_GEN_PMCON_BIOS_PCI_EXP_EN);
    }
  }
  if (RootPortConfig->HotPlug) {
    ///
    /// Clear following status bits, by writing 1b to them, in the Slot
    /// Status register at offset 1Ah of PCI Express Capability structure:
    /// Presence Detect Changed (bit3)
    ///
    Data16Or  = B_PCIE_SLSTS_PDC;
    MmioOr16 (RpBase + CapOffset + R_PCIE_SLSTS_OFFSET, Data16Or);
    ///
    /// Program the following bits in Slot Control register at offset 18h
    /// of PCI Express* Capability structure:
    /// Presence Detect Changed Enable (bit3) = 1b
    /// Hot Plug Interrupt Enable (bit5) = 0b
    ///
    Data16And = (UINT16) (~B_PCIE_SLCTL_HPE);
    Data16Or  = B_PCIE_SLCTL_PDE;
    MmioAndThenOr16 (RpBase + CapOffset + R_PCIE_SLCTL_OFFSET, Data16And, Data16Or);
    ///
    /// Program Misc Port Config (MPC) register at PCI config space offset
    /// D8h as follows:
    /// Hot Plug SCI Enable (HPCE, bit30) = 1b
    /// Hot Plug SMI Enable (HPME, bit1) = 0b (default)
    ///
    MmioOr8 (RpBase + R_PCH_PCIE_MPC + 3, (UINT8) (B_PCH_PCIE_MPC_HPCE >> 24));
    ///
    /// Clear GPE0 Register HOT_PLUG_STS by writing 1
    ///
    IoWrite32 (AcpiBaseAddr + R_ACPI_GPE0a_STS, (UINT32) B_ACPI_GPE0a_STS_HOT_PLUG);

    ///
    /// BIOS should mask the reporting of Completion timeout (CT) errors byerrors by setting
    /// the uncorrectable Error Mask register PCIE RP PCI offset 108h[14].
    ///
    Data32And = 0xFFFFFFFF;
    Data32Or  = B_PCIE_EX_UEM_CT;
    MmioOr32 (RpBase + R_PCH_PCIE_EX_UEM, Data32Or);
  }

  DEBUG ((DEBUG_INFO, "ScInitSingleRootPort () End \n"));
  if (DeviceFound == TRUE || (RootPortConfig->HotPlug == TRUE)) {
    return EFI_SUCCESS;
  } else {
    return EFI_NOT_FOUND;
  }
}

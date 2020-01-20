/** @file
  Library to initialize IPU Device.

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

#include <Library/PeiIpuInitLib.h>
#include <Library/DebugLib.h>
#include <ScAccess.h>
#include <Library/PeiSaPolicyLib.h>
#include <Guid/SaDataHob.h>
#include <Library/HobLib.h>
#include <Library/SteppingLib.h>
#include <Library/SideBandLib.h>
#include <Library/PreSiliconLib.h>

#define FLIS_PORT           0x31
#define CSI_RCOMP_CONTROL   0x08
#define CPHY0_RX_CONTROL1   0x28
#define CPHY2_RX_CONTROL1   0x68
#define CPHY0_DLL_OVRD      0x18
#define CPHY2_DLL_OVRD      0x60
#define DPHY0_RX_CNTRL      0xB0
#define DPHY0_DLL_OVRD      0xA4
#define DPHY1_DLL_OVRD      0xD0
#define AUTO                0xFF

/**
  IpuInit: Initialize the IPU Device

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] SaPolicyPpi          Instance of SI_SA_POLICY_PPI

  @retval  None
**/
VOID IpuInit(
  IN CONST EFI_PEI_SERVICES  **PeiServices,
  IN SI_SA_POLICY_PPI        *SaPolicyPpi
  )
{
  SA_DATA_HOB  *SaDataHob;
  UINT32       Data32;
  BOOLEAN      IpuDisabled;
  IPU_CONFIG   *IpuPolicy = NULL;
  EFI_STATUS   Status;
  UINT32       LoIpuBaseAddress;
  UINT32       HiIpuBaseAddress;
  UINT32       IpuMmAdr;
  UINTN        IpuPciMmBase;

  DEBUG((DEBUG_INFO, "IpuInit - Start\n"));
  //
  // Initialize specific policy into Hob for DXE phase use.
  //
  SaDataHob = NULL;
  SaDataHob = (SA_DATA_HOB *)GetFirstGuidHob(&gSaDataHobGuid);

  Status = GetConfigBlock((VOID *)SaPolicyPpi, &gIpuConfigGuid, (VOID *)&IpuPolicy);
  ASSERT_EFI_ERROR(Status);

  IpuPciMmBase = MmPciBase (SA_IPU_BUS_NUM, SA_IPU_DEV_NUM, SA_IPU_FUN_NUM);

  ///
  /// Check is Iunit being fused off by Reading Cunit PCI Config Space Offset 0xE8 [31] (ipu_dis)
  /// If ipu_dis = 0, Iunit is enabled
  /// else Iunit is disabled
  ///
  Data32 = MmioRead32(MmPciBase(SA_MC_BUS, SA_MC_DEV, SA_MC_FUN) + R_SA_MC_CAPID0_B);
  IpuDisabled = FALSE;
  if (Data32 & BIT31) {
    DEBUG ((DEBUG_INFO, "IPU Fused off\n"));
    IpuDisabled = TRUE;
  } else {
    //
    // IPU is not fused off
    //
    if ((MmioRead16 (IpuPciMmBase + PCI_VID)) == 0xFFFF) {
      DEBUG((DEBUG_ERROR, "IPU config space not accessible!\n"));
      IpuDisabled = TRUE;
    }

    if ((IpuPolicy->SaIpuEnable == DEVICE_ENABLE)
    ) {
      IpuDisabled = FALSE;
    } else {
      //
      // Policy decides to disable IPU
      //
      IpuDisabled = TRUE;
    }

    if (IpuDisabled) {
      ///
      /// If IPU is not fused off it is enabled anyway so here BIOS has to disable it if required
      /// Clear Cunit PCI Config Space Offset 0x54 [5] = 0
      ///
      DEBUG ((DEBUG_INFO, "Disable IPU\n"));
      MmioAnd32 (MmPciBase (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN) + R_SA_DEVEN, (UINT32)~BIT5);
      //
      // Update IPU ACPI mode depending on IGFX present or not
      //
      IpuPolicy->IpuAcpiMode = 0;  //Auto
    } else {
      ///
      /// If IPU is not fused off and it is enabled required, it should be enabled by default by
      /// reading Cunit PCI Config Space Offset 0x54 [5] = 1
      ///
      DEBUG ((DEBUG_INFO, "Enabled IPU\n"));
      MmioOr32 (MmPciBase(SA_MC_BUS, SA_MC_DEV, SA_MC_FUN) + R_SA_DEVEN, (UINT32)BIT5);
      //
      // Initialize IPU registers
      //
      MmioWrite32 (IpuPciMmBase + PCI_SVID, 0x00000000);

      if (PLATFORM_ID == VALUE_REAL_PLATFORM) {
        if (MmioRead16 (MmPciBase(SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0)) != 0xFFFF) {
          //
          // Set IPU ACPI mode as IGFX Child device
          //
          IpuPolicy->IpuAcpiMode = 1;  //AcpiIgfx
        } else {
          //
          // Set IPU ACPI mode as ACPI device
          //
          IpuPolicy->IpuAcpiMode = 2;  //AcpiNoIgfx
        }
      }
      DEBUG ((DEBUG_INFO, "IpuAcpiMode: %d\n", IpuPolicy->IpuAcpiMode));
      IpuMmAdr = IpuPolicy->IpuMmAdr;
      LoIpuBaseAddress = (UINT32) (IpuMmAdr & 0xFFFFFFFF);
      HiIpuBaseAddress = 0;
      DEBUG ((DEBUG_INFO, "Allocate temporary IpuMmAdr\n"));
      MmioWrite32 (IpuPciMmBase + PCI_BAR0, LoIpuBaseAddress);
      MmioWrite32 (IpuPciMmBase + PCI_BAR0 + 0x4, HiIpuBaseAddress);

      ///
      /// Enable Bus Master and Memory access on 0:2:0
      ///
      DEBUG((DEBUG_INFO, "Enable Bus Master and Memory access\n"));
      MmioOr16 (IpuPciMmBase + PCI_CMD, (BIT2 | BIT1));


      CsiPhyWorkaround();

      ///
      /// Disable Bus Master and Memory access on 0:2:0
      ///
      MmioAnd16 (IpuPciMmBase + PCI_CMD, (UINT16) ~(BIT2 | BIT1));

      ///
      /// Clear IPUMmAdr
      ///
      MmioWrite32 (IpuPciMmBase + PCI_BAR0, 0);
      MmioWrite32 (IpuPciMmBase + PCI_BAR0 + 0x4, 0);
    }
  }
  if (SaDataHob != NULL) {
    if (IpuPolicy->IpuAcpiMode != 0) {  //No equal to Auto
      SaDataHob->IpuAcpiMode = IpuPolicy->IpuAcpiMode;
      DEBUG((DEBUG_INFO, "Update SaDataHob->IpuAcpiMode: %d\n", SaDataHob->IpuAcpiMode));
    }
  }
  DEBUG((DEBUG_INFO, "IpuInit - End\n"));
}

/**
CsiPhyWorkaround: Initialize DRC, CRC and RCOMP PHY parameters

@param[in] None

@retval  None
**/
VOID CsiPhyWorkaround(
  )
{
  UINT8        CrcVal;    // Must be a value between 0 and 63. A value of 0xFF means AUTO
  UINT8        DrcVal0_1; // Must be a value between 0 and 63. A value of 0xFF means AUTO
  UINT8        DrcVal2_3; // Must be a value between 0 and 63. A value of 0xFF means AUTO
  UINT8        RcompVal;  // Must be a value between 0 and 31. A value of 0xFF means AUTO
  UINT8        CtleVal;   // Must be a value between 0 and 15. No support for AUTO!
  UINT32       Data;
  BXT_STEPPING SocStepping;

  DEBUG((DEBUG_INFO, "CsiPhyWorkaround - Start\n"));
  SocStepping = BxtStepping ();
  if (SocStepping == BxtB1) {
    CrcVal    = 23;
    DrcVal0_1 = 44;
    DrcVal2_3 = 44;
    RcompVal  = 20;
    CtleVal   = 4;
  } else if (SocStepping == BxtC0) {
    CrcVal    = 18;
    DrcVal0_1 = 29;
    DrcVal2_3 = 31;
    RcompVal  = AUTO;
    CtleVal   = 4;
  } else if (SocStepping >= BxtPB0) {
    CrcVal    = 18;
    DrcVal0_1 = 29;
    DrcVal2_3 = 31;
    RcompVal  = AUTO;
    CtleVal   = 4;
  } else {
    return;
  }

  // Set RCOMP
  // If RcompVal is AUTO, then bypass the below section and let HW use its HW power up defaults
  if (RcompVal != AUTO) {
    // csi_rcomp_control.csi_hs_rcomp_update_mode = 0x2    RegOffset=0x08 Bits 16:15
    // csi_rcomp_control.csi_hs_rcomp_ovr_code = RcompVal  RegOffset=0x08 Bits  5:1
    // csi_rcomp_control.cphy0_hs_rcomp_ovr_enable = 0x0   RegOffset=0x08 Bits  6:6
    Data = SideBandRead32(FLIS_PORT, CSI_RCOMP_CONTROL);
    Data = Data & (~((0x3 << 15) | (0x1 << 6) | (31 << 1))); // Zero the fields that we override
    Data = Data | (0x2 << 15) | (RcompVal << 1);
    SideBandWrite32(FLIS_PORT, CSI_RCOMP_CONTROL, Data);

    // csi_rcomp_control.cphy0_hs_rcomp_ovr_enable = 0x1  RegOffset=0x08  Bits  6:6
    Data = Data | (0x1 << 6);
    SideBandWrite32(FLIS_PORT, CSI_RCOMP_CONTROL, Data);
  }

  // Set CTLE: No AUTO option for CTLE
  // cphy0_rx_control1.eq_dlane0 = CtleVal  RegOffset=0x28  Bits 30:27
  Data = SideBandRead32(FLIS_PORT, CPHY0_RX_CONTROL1);
  Data = Data & (~(15 << 27));  // Zero the fields that we override
  Data = Data | (CtleVal << 27);
  SideBandWrite32(FLIS_PORT, CPHY0_RX_CONTROL1, Data);

  // cphy2_rx_control1.eq_dlane1 = CtleVal  RegOffset=0x68  Bits 30:27
  Data = SideBandRead32(FLIS_PORT, CPHY2_RX_CONTROL1);
  Data = Data & (~(15 << 27));  // Zero the fields that we override
  Data = Data | (CtleVal << 27);
  SideBandWrite32(FLIS_PORT, CPHY2_RX_CONTROL1, Data);

  // Set CRC
  if (CrcVal != AUTO) {
    // cphy0_dll_ovrd.crcdc_fsm_dlane0 = CrcVal RegOffset=0x18  Bits 6:1
    // cphy0_dll_ovrd.lden_crcdc_fsm_dlane0 = 1 RegOffset=0x18  Bits 0:0
    Data = SideBandRead32(FLIS_PORT, CPHY0_DLL_OVRD);
    Data = Data & (~((63 << 1) | (0x1 << 0))); // Zero the fields that we override
    Data = Data | (CrcVal << 1) | (0x1 << 0);
    SideBandWrite32(FLIS_PORT, CPHY0_DLL_OVRD, Data);

    // cphy2_dll_ovrd.crcdc_fsm_dlane1 = CrcVal RegOffset=0x60  Bits 6:1
    // cphy2_dll_ovrd.lden_crcdc_fsm_dlane1 = 1 RegOffset=0x60  Bits 0:0
    Data = SideBandRead32(FLIS_PORT, CPHY2_DLL_OVRD);
    Data = Data & (~((63 << 1) | (0x1 << 0))); // Zero the fields that we override
    Data = Data | (CrcVal << 1) | (0x1 << 0);
    SideBandWrite32(FLIS_PORT, CPHY2_DLL_OVRD, Data);
  }

  // Set DRC
  // Setting the below bits in DPHY0_RX_CNTRL will be done regardless if auto is required for either DrcVal0_1 or DrcVal2_3
  // dphy0_rx_cntrl.csi1_hs_skewcal_cr_sel_dlane3 = 1   RegOffset=0xb0  Bits 28:28
  // dphy0_rx_cntrl.csi1_hs_skewcal_cr_sel_dlane2 = 1   RegOffset=0xb0  Bits 26:26
  // dphy0_rx_cntrl.csi1_hs_skewcal_cr_sel_dlane1 = 1   RegOffset=0xb0  Bits 24:24
  // dphy0_rx_cntrl.csi1_hs_skewcal_cr_sel_dlane0 = 1   RegOffset=0xb0  Bits 22:22
  Data = SideBandRead32(FLIS_PORT, DPHY0_RX_CNTRL);
  Data = Data | (0x1 << 28) | (0x1 << 26) | (0x1 << 24) | (0x1 << 22);
  SideBandWrite32(FLIS_PORT, DPHY0_RX_CNTRL, Data);

  if (DrcVal0_1 != AUTO) {
    // dphy0_dll_ovrd.lden_drc_fsm = 0      RegOffset=0xa4   Bits 0:0
    // dphy0_dll_ovrd.drc_fsm_ovrd = DrcVal0_1  RegOffset=0xa4   Bits 6:1
    Data = SideBandRead32(FLIS_PORT, DPHY0_DLL_OVRD);
    Data = Data & (~((63 << 1) | (0x1 << 0))); // Zero the fields that we override
    Data = Data | (DrcVal0_1 << 1);
    SideBandWrite32(FLIS_PORT, DPHY0_DLL_OVRD, Data);
    // dphy0_dll_ovrd.lden_drc_fsm = 1    RegOffset=0xa4   Bits 0:0
    Data = Data | (0x1 << 0);
    SideBandWrite32(FLIS_PORT, DPHY0_DLL_OVRD, Data);
  }

  if (DrcVal2_3 != AUTO) {
    // dphy1_dll_ovrd.lden_drc_fsm = 0      RegOffset=0xd0   Bits 0:0
    // dphy1_dll_ovrd.drc_fsm_ovrd = DrcVal2_3  RegOffset=0xd0   Bits 6:1
    Data = SideBandRead32(FLIS_PORT, DPHY1_DLL_OVRD);
    Data = Data & (~((63 << 1) | (0x1 << 0))); // Zero the fields that we override
    Data = Data | (DrcVal2_3 << 1);
    SideBandWrite32(FLIS_PORT, DPHY1_DLL_OVRD, Data);
    // dphy1_dll_ovrd.lden_drc_fsm = 1      RegOffset=0xd0   Bits 0:0
    Data = Data | (0x1 << 0);
    SideBandWrite32(FLIS_PORT, DPHY1_DLL_OVRD, Data);
  }

  DEBUG((DEBUG_INFO, "CsiPhyWorkaround - End\n"));

  return;
}
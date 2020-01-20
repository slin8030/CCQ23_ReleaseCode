/** @file
  SC HSIO Initialization file

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

@par Specification Reference:
**/

#include "ScInitPei.h"
#include "UfsMphyInit.h"
#include <Library/SteppingLib.h>
#include <Library/HsioLib.h>
#include <Private/ScHsio.h>
//[-start-160216-IB07400702-add]//
#include <ChipsetPostCode.h>
//[-end-160216-IB07400702-add]//


GLOBAL_REMOVE_IF_UNREFERENCED UINT8 mIpcWbuffer[4] = {0, 0, 0, 0};

/**
  The function perform all the USB2 PHY programming that required by silicon
  in early PEI phase before devices get initialized.

  @param[in] XhciPciMmBase        XHCI PCI Base Address
  @param[in] XhciMmioBase         Memory base address of XHCI Controller

  @retval None
**/
VOID
ScUsb2PhyInit (
  SC_USB_CONFIG      *UsbConfig,
  UINTN              XhciPciMmBase,
  UINT32             XhciMmioBase
  )
{
  UINT16                      Index;
  SC_SIDEBAND_TABLE_STRUCT    *Usb2PhyCommonTable;
  UINT16                      Usb2PhyCommonTableSize;
  SC_SIDEBAND_TABLE_STRUCT    *Usb2PhyPerPortTable;
  UINT16                      Usb2PhyPerPortTableSize;
  UINT8                       PortIndex;
  UINT16                      Offset;
  BXT_SERIES                  BxtSeries;
  UINT32                      Data32Or;
//[-start-170116-IB07400834-add]//
  BXT_STEPPING                Stepping;
//[-end-170116-IB07400834-add]//

  BxtSeries               = GetBxtSeries();
//[-start-170116-IB07400834-add]//
  Stepping                = BxtStepping();
//[-end-170116-IB07400834-add]//
  Usb2PhyCommonTableSize  = 0;
  Usb2PhyCommonTable      = NULL;
  Usb2PhyPerPortTableSize = 0;
  Usb2PhyPerPortTable     = NULL;
  Data32Or                = 0;

  DEBUG ((DEBUG_INFO, "ScUsb2PhyInit() - Start\n"));

  if (UsbConfig  == NULL) {
    DEBUG ((DEBUG_ERROR, "ScUsbSsicPhyInit(): ScPolicyPpi or UsbConfig is NULL  - End\n"));
    return;
  }

  switch (BxtSeries) {
    case BxtP:
//[-start-170116-IB07400834-modify]//
      if (Stepping == BxtPA0) {
        Usb2PhyCommonTableSize  = (sizeof (ScBxtpUsb2PhyCommonTable_A0) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
        Usb2PhyCommonTable      = ScBxtpUsb2PhyCommonTable_A0;
        Usb2PhyPerPortTableSize = (sizeof (ScBxtpUsb2PhyPerPortTable_A0) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
        Usb2PhyPerPortTable     = ScBxtpUsb2PhyPerPortTable_A0;
      } else {
        Usb2PhyCommonTableSize  = (sizeof (ScBxtpUsb2PhyCommonTable_Bx) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
        Usb2PhyCommonTable      = ScBxtpUsb2PhyCommonTable_Bx;
        Usb2PhyPerPortTableSize = (sizeof (ScBxtpUsb2PhyPerPortTable_Bx) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
        Usb2PhyPerPortTable     = ScBxtpUsb2PhyPerPortTable_Bx;
      }
//[-end-170116-IB07400834-modify]//
      break;

    case Bxt1:
      Usb2PhyCommonTableSize  = (sizeof (ScBxtUsb2PhyCommonTable_E0) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
      Usb2PhyCommonTable      = ScBxtUsb2PhyCommonTable_E0;
      Usb2PhyPerPortTableSize = (sizeof (ScBxtUsb2PhyPerPortTable_E0) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
      Usb2PhyPerPortTable     = ScBxtUsb2PhyPerPortTable_E0;
      break;

    default:
      Usb2PhyCommonTableSize  = 0;
      Usb2PhyCommonTable      = NULL;
      Usb2PhyPerPortTableSize = 0;
      Usb2PhyPerPortTable     = NULL;
      DEBUG ((DEBUG_ERROR, "Unknown stepping and skip ScUsb2PhyInit()\n"));
      break;
  }

  for (Index = 0; Index < Usb2PhyCommonTableSize; Index++) {
    SideBandAndThenOr32(
      (UINT8)(USB2_PHY_PORT_ID & 0xff),
      (UINT16)(Usb2PhyCommonTable[Index].Address & 0xffff),
      Usb2PhyCommonTable[Index].AndMask,
      Usb2PhyCommonTable[Index].OrMask
      );
  }
  for (Index = 0; Index < Usb2PhyPerPortTableSize; Index++) {
    for (PortIndex = 1; PortIndex <= GetScXhciMaxUsb2PortNum (); PortIndex++) {
      Data32Or = Usb2PhyPerPortTable[Index].OrMask;
      if (BxtSeries == BxtP) {
        if (Usb2PhyPerPortTable[Index].Address == R_PCR_USB2_PPORT) {
          Data32Or &= 0xFFFF80FF;
          Data32Or |= (UINT32) ((UsbConfig->PortUsb20[PortIndex-1].AfeConfig.PerPortTxPeHalf << 14) |
                                (UsbConfig->PortUsb20[PortIndex-1].AfeConfig.PerPortPeTxiSet << 11) |
                                (UsbConfig->PortUsb20[PortIndex-1].AfeConfig.PerPortTxiSet   << 8 ));
        }
        if (Usb2PhyPerPortTable[Index].Address == R_PCR_USB2_PPORT2) {
          Data32Or &= 0xFC71FFFC;
          Data32Or |= (UINT32) ((UsbConfig->PortUsb20[PortIndex-1].AfeConfig.HsSkewSel        << 25) |
                                (UsbConfig->PortUsb20[PortIndex-1].AfeConfig.IUsbTxEmphasisEn << 23) |
                                (UsbConfig->PortUsb20[PortIndex-1].AfeConfig.PerPortRXISet    << 17) |
                                (UsbConfig->PortUsb20[PortIndex-1].AfeConfig.HsNpreDrvSel));
        }
      }

      Offset = R_PCR_USB2_PHY_LANE_BASE_ADDR | Usb2PhyPerPortTable[Index].Address | (PortIndex << 8);
      SideBandAndThenOr32(
        (UINT8)(USB2_PHY_PORT_ID & 0xff),
        Offset,
        Usb2PhyPerPortTable[Index].AndMask,
        Data32Or
        );
    }
  }
  mIpcWbuffer[0] |= BIT2;

  DEBUG ((DEBUG_INFO, "ScUsb2PhyInit() - End\n"));
}

/**
  The function perform the SSIC PHY programming that required by silicon
  in early PEI phase before devices get initialized.

  @param[in] UsbConfig            The SC Policy for USB configuration
  @param[in] XhciPciMmBase        XHCI PCI Base Address
  @param[in] XhciMmioBase         Memory base address of XHCI Controller

  @retval None
**/
VOID
ScUsbSsicPhyInit (
  SC_USB_CONFIG      *UsbConfig,
  UINTN              XhciPciMmBase,
  UINT32             XhciMmioBase
  )
{
  UINT16                   Index;
  UINT16                   TableSize;
  UINT32                   Value;
  SC_SIDEBAND_TABLE_STRUCT *ScHsioTable;
  BXT_STEPPING             Stepping;
  SC_SBI_PID               PortId;

  DEBUG ((DEBUG_INFO, "ScUsbSsicPhyInit() - Start\n"));

  if (UsbConfig  == NULL) {
    DEBUG ((DEBUG_ERROR, "ScUsbSsicPhyInit(): ScPolicyPpi or UsbConfig is NULL  - End\n"));
    return;
  }

  if (MmioRead32 ((UINTN) (XhciPciMmBase)) == 0xFFFFFFFF) {
    DEBUG ((DEBUG_ERROR, "ScUsbSsicPhyInit(): XHCI controller is disabled  - End\n"));
    return;
  }
  //
  // Open XHCI MMIO space with temp assigned address
  //
  XhciMemorySpaceOpen (
    UsbConfig,
    XhciMmioBase,
    XhciPciMmBase
  );

  Stepping = BxtStepping ();
  //
  // BXT & BXTP only have SSIC[0]
  //
  if (UsbConfig->SsicConfig.SsicPort[0].Rate == XhciSsicRateA) {
    if (UsbConfig->SsicConfig.DlanePwrGating == TRUE) {
      switch (Stepping) {
        case BxtB0:
        case BxtB1:
        case BxtB2:
        case BxtC0:
          TableSize = (sizeof (ScSsicHsioTable_Bx_RateA_Dlane_PG) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
          ScHsioTable = ScSsicHsioTable_Bx_RateA_Dlane_PG;
          break;
//[-start-160828-IB07400775-add]//
        case BxtPA0:
//[-end-160828-IB07400775-add]//
        case BxtPB0:
        case BxtPB1:
        case BxtPE0:
        case BxtPF0:          
          TableSize = (sizeof (ScBxtpSsicModphy_RateA_Dlane_PG) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
          ScHsioTable = ScBxtpSsicModphy_RateA_Dlane_PG;
          break;
        default:
          TableSize   = 0;
          ScHsioTable = NULL;
          DEBUG ((DEBUG_INFO, "Unsupported SC Stepping\n"));
          break;
      }
    } else {
      switch (Stepping) {
        case BxtB0:
        case BxtB1:
        case BxtB2:
        case BxtC0:
          TableSize = (sizeof (ScSsicHsioTable_RateA) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
          ScHsioTable = ScSsicHsioTable_RateA;
          break;
//[-start-160828-IB07400775-add]//
        case BxtPA0:
          TableSize = (sizeof (ScBxtpSsicModphy_Ax_RateA) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
          ScHsioTable = ScBxtpSsicModphy_Ax_RateA;
          break;
//[-end-160828-IB07400775-add]//
        case BxtPB0:
        case BxtPB1:
        case BxtPE0:
        case BxtPF0:          
          TableSize = (sizeof (ScBxtpSsicModphy_Bx_RateA) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
          ScHsioTable = ScBxtpSsicModphy_Bx_RateA;
          break;
        default:
          TableSize   = 0;
          ScHsioTable = NULL;
          DEBUG ((DEBUG_INFO, "Unsupported SC Stepping\n"));
          break;
      }
    }
  } else { // UsbConfig->SsicConfig.SsicPort[0].Rate == XhciSsicRateB
    if (UsbConfig->SsicConfig.DlanePwrGating == TRUE) {
      switch (Stepping) {
        case BxtB0:
        case BxtB1:
        case BxtB2:
        case BxtC0:
          TableSize = (sizeof (ScSsicHsioTable_Bx_RateB_Dlane_PG)/ sizeof (SC_SIDEBAND_TABLE_STRUCT));
          ScHsioTable = ScSsicHsioTable_Bx_RateB_Dlane_PG;
          break;
//[-start-160828-IB07400775-add]//
        case BxtPA0:
          TableSize = (sizeof (ScBxtpSsicModphy_Ax_RateB_Dlane_PG)/ sizeof (SC_SIDEBAND_TABLE_STRUCT));
          ScHsioTable = ScBxtpSsicModphy_Ax_RateB_Dlane_PG;
          break;
//[-end-160828-IB07400775-add]//
        case BxtPB0:
        case BxtPB1:
        case BxtPE0:
        case BxtPF0:          
          TableSize = (sizeof (ScBxtpSsicModphy_RateB_Dlane_PG)/ sizeof (SC_SIDEBAND_TABLE_STRUCT));
          ScHsioTable = ScBxtpSsicModphy_RateB_Dlane_PG;
          break;
        default:
          TableSize   = 0;
          ScHsioTable = NULL;
          DEBUG ((DEBUG_INFO, "Unsupported SC Stepping\n"));
          break;
      }
    } else {
      switch (Stepping) {
        case BxtB0:
        case BxtB1:
        case BxtB2:
        case BxtC0:
          TableSize = (sizeof (ScSsicHsioTable_RateB) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
          ScHsioTable = ScSsicHsioTable_RateB;
          break;
//[-start-160828-IB07400775-add]//
        case BxtPA0:
          TableSize = (sizeof (ScBxtpSsicModphy_Ax_RateB) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
          ScHsioTable = ScBxtpSsicModphy_Ax_RateB;
          break;
//[-end-160828-IB07400775-add]//
        case BxtPB0:
        case BxtPB1:
        case BxtPE0:
        case BxtPF0:          
          TableSize = (sizeof (ScBxtpSsicModphy_Bx_RateB) / sizeof (SC_SIDEBAND_TABLE_STRUCT));
          ScHsioTable = ScBxtpSsicModphy_Bx_RateB;
          break;
        default:
          TableSize   = 0;
          ScHsioTable = NULL;
          DEBUG ((DEBUG_INFO, "Unsupported SC Stepping\n"));
          break;
      }
    }
  }

  ///BXT 1 and BXT P use the same SSIC MPHY SideBand ID (0xAF)
  if (GetBxtSeries () == BxtP || GetBxtSeries () == Bxt1) {
    PortId = PID_MMP0_BXTP;
  } else {
    PortId = PID_MMP0_BXT;
  }
  for (Index = 0; Index < TableSize; Index++) {
    SideBandAndThenOr32(
      (UINT8) PortId,
      (UINT16)(ScHsioTable[Index].Address & 0xffff),
      ScHsioTable[Index].AndMask,
      ScHsioTable[Index].OrMask
      );

    DEBUG ((DEBUG_INFO, "PortId: 0x%X, Address: 0x%04X, AndMask: 0x%08X, OrMask: 0x%08X\n",
      PortId,
      (UINT16)(ScHsioTable[Index].Address & 0xffff),
      ScHsioTable[Index].AndMask,
      ScHsioTable[Index].OrMask)
      );
  }

  ///
  /// If SSIC initialization is not enabled,
  /// Set SSIC_PORT_UNUSED bits and SSIC_PROG_DONE bits and return
  ///
  //
  // if not port1 == 0:
  //   xhci.port1_register_access_control.rrap_reg_bank_valid = 1                           ;R_XHCI_SSIC_GLOBAL_CONFIG_CONTROL[25] = 1b
  //   xhci.port1_ssic_config_reg1.ssicrate = 1-rateb # 0: RateB                            ;R_XHCI_PORT1_SSIC_CONFIG_REG1[4] = 1b
  //   xhci.port1_profile_attributes_reg0 = 0x22c001 + rateb # Local TX HS Rate             ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG0 = 0x22c001
  //   xhci.port1_profile_attributes_reg1 = 0xa2c001 + rateb # Local RX HS Rate             ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG1 = 0xa2c001
  //   xhci.port1_profile_attributes_reg2 = 0x23c001 + gear2 # Local TX HS Gear             ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG2 = 0x23c001
  //   xhci.port1_profile_attributes_reg3 = 0xa3c001 + gear2 # Local RX HS Gear             ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG3 = 0xa3c001
  //   xhci.port1_profile_attributes_reg4 = 0x228001 + rateb # Remote TX HS Rate            ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG4 = 0x228001
  //   xhci.port1_profile_attributes_reg5 = 0xa28001 + rateb # Remote RX HS Rate            ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG5 = 0xa28001
  //   xhci.port1_profile_attributes_reg6 = 0x238001 + gear2 # Remote TX HS Gear            ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG6 = 0x238001
  //   xhci.port1_profile_attributes_reg7 = 0xa38001 + gear2 # Remote RX HS Gear            ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG7 = 0xa38001
  //   xhci.port1_profile_attributes_reg8 = 0x288000 + tx_synclen                           ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG8 = 0x288000 + 15
  //   xhci.port1_profile_attributes_reg9 = 0x298000 + tx_preparelen                        ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG9 = 0x298000 + 44
  //   xhci.port1_profile_attributes_reg10 = 0x338000 + tx_minact                           ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG10 = 0x338000 + 15
  //   xhci.port1_ssic_config_reg2.disable_scrambling = discramble                          ;R_XHCI_PORT1_SSIC_CONFIG_REG2[25] = 1
  //   xhci.port1_profile_attributes_reg11 = 0x4038000 + discramble                         ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG11 = 0x4038001
  //   xhci.port1_ssic_config_reg3.disable_u0_stall = 1                                     ;     R_XHCI_PORT1_SSIC_CONFIG_REG3[2] = 1
  //   xhci.port1_profile_attributes_reg12 = 0x4048001 # U0_STALL                           ;R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG12 = 0x4048001
  //   if not ls == 0:
  //     print 'Port 1 will be in LS mode, write port1_register_access_control.cpd = 1 to exit'
  //     xhci.port1_register_access_control.cpd = 0
  //     xhci.port1_register_access_control.read_write = 1
  //     xhci.port1_register_access_control.target_phy = 0 # 0: Remote
  //     xhci.port1_register_access_control.att_id = 0x1;
  //

  MmioAndThenOr32 ((XhciMmioBase + R_XHCI_PORT1_REGISTER_ACCESS_CONTROL), 0xfdffffff, 0x02000000);
  DEBUG ((EFI_D_INFO, "SSIC1 Rate %x\n",UsbConfig->SsicConfig.SsicPort[0].Rate));

  if (UsbConfig->SsicConfig.SsicPort[0].Rate == XhciSsicRateA){ ///<SSIC Rate A series
    MmioAndThenOr32 ((XhciMmioBase + R_XHCI_PORT1_SSIC_CONFIG_REG1), 0xffffffef, 0x00000010);
  } else { ///<SSIC Rate B series
    MmioAndThenOr32 ((XhciMmioBase + R_XHCI_PORT1_SSIC_CONFIG_REG1), 0xffffffef, 0x00000000);
  }

  if (UsbConfig->SsicConfig.SsicPort[0].Rate == XhciSsicRateA) { ///<SSIC Rate A series
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG0), 0x22c001); ///attribute0
  } else {///<SSIC Rate B series
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG0), 0x22c002);
  }
  if (UsbConfig->SsicConfig.SsicPort[0].Rate == XhciSsicRateA) { ///<SSIC Rate A series
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG1), 0xa2c001); ///attribute1
  } else {///<SSIC Rate B series
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG1), 0xa2c002);
  }
  MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG2), 0x23c001); ///attribute2
  MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG3), 0xa3c001); ///attribute3
  if (UsbConfig->SsicConfig.SsicPort[0].Rate == XhciSsicRateA){ ///<SSIC Rate A series
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG4), 0x228001); ///attribute4
  } else {///<SSIC Rate B series
  MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG4), 0x228002);
  }
  if (UsbConfig->SsicConfig.SsicPort[0].Rate == XhciSsicRateA){ ///<SSIC Rate A series
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG5), 0xa28001); ///attribute5
  } else {///<SSIC Rate B series
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG5), 0xa28002);
  }
  MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG6), 0x238001); ///attribute6
  MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG7), 0xa38001); ///attribute7
  MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG8), 0x288000 + 0x46); ///attribute8
  MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG9), 0x298000 + 0x0f); ///attribute9
  MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG10), 0x338000 + 0x01); ///attribute10

  MmioAndThenOr32 ((XhciMmioBase + R_XHCI_PORT1_SSIC_CONFIG_REG2), 0xfdffffff, 0x02000000);

  MmioAndThenOr32 ((XhciMmioBase + R_XHCI_PORT1_SSIC_CONFIG_REG2), 0xfe1fffff, 0x01e00000); /// SSIC Configuration Register 2.RETRAIN_TIME = 0xf. (1ms).

  MmioWrite32((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG11), 0x4038001); ///attribute11
  MmioAndThenOr32 ((XhciMmioBase + R_XHCI_PORT1_SSIC_CONFIG_REG3), 0xfffffffb, 0x00000004);
  MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG12), 0x4048001); ///attribute12

  DEBUG ((DEBUG_INFO, "XhciMmioBase 0x%x\n", XhciMmioBase));
  DEBUG ((DEBUG_INFO, "Initialize xHCI SSIC PORT1\n"));

  Value = MmioRead32(XhciMmioBase + R_XHCI_SSIC_CFG_2_PORT1);
  DEBUG((DEBUG_INFO, "SSIC Port1 Enable 0x%x:0x%x\n", (XhciMmioBase + R_XHCI_SSIC_CFG_2_PORT1), Value));

  // if not port2 == 0:
  //   xhci.port2_register_access_control.rrap_reg_bank_valid = 1
  //   xhci.port2_ssic_config_reg1.ssicrate = 1-rateb # 0: RateB
  //   xhci.port2_ssic_config_reg1.hsgear = gear2
  //   xhci.port2_profile_attributes_reg0 = 0x22c001 + rateb # Local TX HS Rate
  //   xhci.port2_profile_attributes_reg1 = 0xa2c001 + rateb # Local RX HS Rate
  //   xhci.port2_profile_attributes_reg2 = 0x23c001 + gear2 # Local TX HS Gear
  //   xhci.port2_profile_attributes_reg3 = 0xa3c001 + gear2 # Local RX HS Gear
  //   xhci.port2_profile_attributes_reg4 = 0x228001 + rateb # Remote TX HS Rate
  //   xhci.port2_profile_attributes_reg5 = 0xa28001 + rateb # Remote RX HS Rate
  //   xhci.port2_profile_attributes_reg6 = 0x238001 + gear2 # Remote TX HS Gear
  //   xhci.port2_profile_attributes_reg7 = 0xa38001 + gear2 # Remote RX HS Gear
  //   xhci.port2_profile_attributes_reg8 = 0x288000 + tx_synclen
  //   xhci.port2_profile_attributes_reg9 = 0x298000 + tx_preparelen
  //   xhci.port2_profile_attributes_reg10 = 0x338000 + tx_minact
  //   xhci.port2_ssic_config_reg2.disable_scrambling = discramble
  //   xhci.port2_profile_attributes_reg11 = 0x4038000 + discramble
  //   xhci.port2_ssic_config_reg3.disable_u0_stall = 1
  //   xhci.port2_profile_attributes_reg12 = 0x4048001 # U0_STALL
  //   if not ls == 0:
  //     print 'Port 2 will be in LS mode, write port2_register_access_control.cpd = 1 to exit'
  //     xhci.port2_register_access_control.cpd = 0
  //     xhci.port2_register_access_control.read_write = 1
  //     xhci.port2_register_access_control.target_phy = 0 # 0: Remote
  //     xhci.port2_register_access_control.att_id = 0x1;

  if (UsbConfig->SsicConfig.SsicPort[1].Enable == TRUE) {
    MmioAndThenOr32 ((XhciMmioBase + R_XHCI_PORT1_REGISTER_ACCESS_CONTROL + 0x110), 0xfdffffff, 0x02000000);

    DEBUG ((DEBUG_INFO, "SSIC2 Rate %x\n",UsbConfig->SsicConfig.SsicPort[1].Rate));

    if (UsbConfig->SsicConfig.SsicPort[1].Rate == XhciSsicRateA) { ///<SSIC Rate A series
      MmioAndThenOr32 ((XhciMmioBase + R_XHCI_PORT1_SSIC_CONFIG_REG1 + 0x30), 0xffffffef, 0x00000010);
    } else { //SSIC Rate B series
      MmioAndThenOr32 ((XhciMmioBase + R_XHCI_PORT1_SSIC_CONFIG_REG1 + 0x30), 0xffffffef, 0x00000000);
    }

    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG0 + 0x110), 0x22c001);
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG1 + 0x110), 0xa2c001);
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG2 + 0x110), 0x23c001);
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG3 + 0x110), 0xa3c001);
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG4 + 0x110), 0x228001);
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG5 + 0x110), 0xa28001);

    if (UsbConfig->SsicConfig.SsicPort[1].Rate == XhciSsicRateA) { ///<SSIC Rate A series
      MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG6 + 0x110), 0x23C001);
    } else { //SSIC Rate B series
      MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG6 + 0x110), 0x23C002);
    }
    //  MmioWrite32     ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG6 + 0x110), 0x238001);
    //  MmioWrite32     ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG6 + 0x110), 0x23C002);

    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG7 + 0x110), 0xa38001);
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG8 + 0x110), 0x288000 + 0x44);
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG9 + 0x110), 0x298000 + 15);
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG10 + 0x110), 0x338000 + 15);
    MmioAndThenOr32 ((XhciMmioBase + R_XHCI_PORT1_SSIC_CONFIG_REG2 + 0x30), 0xfdffffff, 0x02000000);
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG11 + 0x110), 0x4038001);
    MmioAndThenOr32 ((XhciMmioBase + R_XHCI_PORT1_SSIC_CONFIG_REG3 + 0x30), 0xfffffffb, 0x00000004);
    MmioWrite32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG12 + 0x110), 0x4048001);

    DEBUG ((DEBUG_INFO, "XhciMmioBase 0x%x\n", XhciMmioBase));
    DEBUG ((DEBUG_INFO, "Initialize xHCI SSIC PORT2\n"));

    Value = MmioRead32(XhciMmioBase + R_XHCI_SSIC_CFG_2_PORT2);
    DEBUG((DEBUG_INFO, "SSIC Port2 Enable 0x%x:0x%x\n", (XhciMmioBase + R_XHCI_SSIC_CFG_2_PORT2), Value));
  }
  //
  // --- Proceeds SSIC Link Training.
  // xhci.port1_ssic_config_reg2.prog_done=1
  // xhci.port2_ssic_config_reg2.prog_done=1
  //

  // MmioOr32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG0), BIT30);
  // MmioOr32 ((XhciMmioBase + R_XHCI_PORT1_PROFILE_ATTRIBUTES_REG0), BIT30);

  //
  // PMC/PMU overrides (for initial bring-up)
  //  1) Removes PHY power up overrides for clane and dane core power; and
  //  2) Resets the PHY
  //
  mIpcWbuffer[0] |= BIT0;

  // Clear memory resources
  XhciMemorySpaceClose (
    UsbConfig,
    XhciMmioBase,
    XhciPciMmBase
    );

  DEBUG ((DEBUG_INFO, "ScUsbSsicPhyInit() - End\n"));

  return;
}

/**
  The function indicates if at least one of the data lanes attached
  to a specific common lane is of a specific phymode.

  @param[in] Pid               Common Lane End Point ID
  @param[in] PhyMode           Phymode that needs to be checked
  @param[in] Los1              Lane Owner Status 1 Value
  @param[in] Los2              Lane Owner Status 2 Value
  @param[in] Los3              Lane Owner Status 3 Value
  @param[in] Los4              Lane Owner Status 4 Value

  @retval EFI_SUCCESS          Phymode exists.
          EFI_UNSUPPORTED      Phymode does not exist.
**/

EFI_STATUS
ScLosPhyModeCheck (
  IN SC_SBI_PID                        Pid,
  IN UINT8                             PhyMode,
  IN UINT32                            Los1,
  IN UINT32                            Los2,
  IN UINT32                            Los3,
  IN UINT32                            Los4
  )
{
  UINT8 LosPhyMode;
  UINT8 Shift;

  switch (PhyMode) {
    case SC_LANE_OWN_MIPI_A:
      LosPhyMode = V_SC_PCR_FIA_LANE_OWN_SSIC;
      break;
    case SC_LANE_OWN_MIPI_B:
      LosPhyMode = V_SC_PCR_FIA_LANE_OWN_SSIC;
      break;
    default:
      LosPhyMode = PhyMode;
      break;
  }

  switch (Pid) {
    case PID_MODPHY0:
      for(Shift = SC_MODPHY0_LOS1_LANE_START; Shift <= SC_MODPHY0_LOS1_LANE_END; Shift++) {
        if(((Los1 >> (Shift * 4)) & 0xF) == LosPhyMode) {
          return EFI_SUCCESS;
        }
      }
      for(Shift = SC_MODPHY0_LOS2_LANE_START; Shift <= SC_MODPHY0_LOS2_LANE_END; Shift++) {
        if(((Los2 >> (Shift * 4)) & 0xF) == LosPhyMode) {
          return EFI_SUCCESS;
        }
      }
      break;
   default:
      return EFI_SUCCESS;
      break;
  }
  return EFI_UNSUPPORTED;
}
/**
  The function program HSIO registers.

  @param[in] ScPolicy  The SC Policy PPI instance

  @retval EFI_SUCCESS  THe HSIO settings have been programmed correctly
**/
EFI_STATUS
ScHsioCommonBiosProg (
  IN  SC_POLICY_PPI      *ScPolicy
  )
{
  SC_SBI_HSIO_TABLE_STRUCT    *ScHsioTable;
  UINT16                      HsioSize;
  UINT8                       Entry;
  UINT32                      MipiRate;
  UINT32                      Los1;
  UINT32                      Los2;
  UINT32                      Los3;
  UINT32                      Los4;
  UINT8                       Index;
  UINT8                       LaneNum;
  SC_SBI_PID                  PortId;
  UINT16                      Offset;
  UINT16                      *Usb3LaneAddr;
  UINT16                      *PcieLaneAddr;
  UINT16                      *SataLaneAddr;
  UINT16                      Usb3LaneAddrSize;
  UINT16                      PcieLaneAddrSize;
  UINT16                      SataLaneAddrSize;

  ScHsioTable      = NULL;
  HsioSize         = 0;
  Usb3LaneAddr     = NULL;
  Usb3LaneAddrSize = 0;
  PcieLaneAddr     = NULL;
  PcieLaneAddrSize = 0;
  SataLaneAddr     = NULL;
  SataLaneAddrSize = 0;
  Entry            = 0;
  Los3             = 0;
  Los4             = 0;

  DEBUG ((DEBUG_INFO, "ScHsioCommonBiosProg () - Start\n"));
  ///
  /// Step 1
  /// Read the MIPI Rate Register and LOS Registers
  ///
  MipiRate = SideBandRead32 (PID_MODPHY0, R_SC_PCR_MODPHY0_COM0_FBDIVRATIO_7_0);
  DEBUG ((DEBUG_INFO, "FIA MIPI Rate = %08x\n", MipiRate));
  Los1 = SideBandRead32 (PID_FIA, R_SC_PCR_FIA_LOS1_REG_BASE);
  DEBUG ((DEBUG_INFO, "FIA LOS1 = %08x\n", Los1));
  Los2 = SideBandRead32 (PID_FIA, R_SC_PCR_FIA_LOS2_REG_BASE);
  DEBUG ((DEBUG_INFO, "FIA LOS2 = %08x\n", Los2));

  ///
  /// Step 2
  /// Assign the appropriate BIOS HSIO table
  ///
  switch (BxtStepping ()) {
//[-start-160828-IB07400775-add]//
    case BxtPA0:
      ScHsioTable = ScBxtpHsioTable_Ax;
      HsioSize = (UINT16) (sizeof (ScBxtpHsioTable_Ax) / sizeof (SC_SBI_HSIO_TABLE_STRUCT));
      Usb3LaneAddr     = ScBxtpUsb3HsioAddress;
      PcieLaneAddr     = ScBxtpPcieHsioAddress;
      SataLaneAddr     = ScBxtpSataHsioAddress;
      Usb3LaneAddrSize = (UINT16) (sizeof (ScBxtpUsb3HsioAddress) / sizeof (UINT16));
      PcieLaneAddrSize = (UINT16) (sizeof (ScBxtpPcieHsioAddress) / sizeof (UINT16));
      SataLaneAddrSize = (UINT16) (sizeof (ScBxtpSataHsioAddress) / sizeof (UINT16));
      break;
//[-end-160828-IB07400775-add]//
    case BxtPB0:
    case BxtPB1:
    case BxtPE0:
    case BxtPF0:      
      ScHsioTable = ScBxtpHsioTable_Bx;
      HsioSize = (UINT16) (sizeof (ScBxtpHsioTable_Bx) / sizeof (SC_SBI_HSIO_TABLE_STRUCT));
      Usb3LaneAddr     = ScBxtpUsb3HsioAddress;
      PcieLaneAddr     = ScBxtpPcieHsioAddress;
      SataLaneAddr     = ScBxtpSataHsioAddress;
      Usb3LaneAddrSize = (UINT16) (sizeof (ScBxtpUsb3HsioAddress) / sizeof (UINT16));
      PcieLaneAddrSize = (UINT16) (sizeof (ScBxtpPcieHsioAddress) / sizeof (UINT16));
      SataLaneAddrSize = (UINT16) (sizeof (ScBxtpSataHsioAddress) / sizeof (UINT16));
      break;
    case BxtB0:
    case BxtB1:
    case BxtB2:
    case BxtC0:
      ScHsioTable = ScBxtHsioTable_Bx;
      HsioSize = (UINT16) (sizeof (ScBxtHsioTable_Bx) / sizeof (SC_SBI_HSIO_TABLE_STRUCT));
      Usb3LaneAddr     = ScBxtBxUsb3HsioAddress;
      PcieLaneAddr     = ScBxtBxPcieHsioAddress;
      SataLaneAddr     = NULL;
      Usb3LaneAddrSize = (UINT16) (sizeof (ScBxtBxUsb3HsioAddress) / sizeof (UINT16));
      PcieLaneAddrSize = (UINT16) (sizeof (ScBxtBxPcieHsioAddress) / sizeof (UINT16));
      SataLaneAddrSize = 0;
      break;
    default:
      ScHsioTable = NULL;
      HsioSize = 0;
      DEBUG ((DEBUG_INFO, "Unsupported SC Stepping\n"));
      break;
  }
  ///
  /// Step 3
  /// Iterate through the HSIO table
  ///
  for (Entry = 0; Entry < HsioSize; Entry++){
    if (((ScHsioTable[Entry].Offset & B_SC_HSIO_ACCESS_TYPE) == V_SC_HSIO_ACCESS_TYPE_BDCAST) &&
      ((ScHsioTable[Entry].Offset & B_SC_HSIO_LANE_GROUP_NO) != SC_HSIO_LANE_GROUP_COMMON_LANE)) {
      if (ScLosPhyModeCheck ((SC_SBI_PID) ScHsioTable[Entry].PortId, ScHsioTable[Entry].LanePhyMode, Los1, Los2, Los3, Los4) == EFI_SUCCESS) {
        ///
        /// Step 3a
        /// If the HSIO offset is targeted to all the phymode specific data lanes attached to the common lane (BDCAST, Group accessing)
        /// then program the values only if the PhyModes actually exist on the platform based on FIA LOS
        ///
        DEBUG ((DEBUG_INFO, "Programming HSIO BDCAST : PortID = %02x, Offset = %04x, AndMask = %08x, Value = %08x, LanePhyMode = %02x\n", (SC_SBI_PID) ScHsioTable[Entry].PortId, (UINT16) ScHsioTable[Entry].Offset, (UINT32) ScHsioTable[Entry].AndMask, (UINT32) ScHsioTable[Entry].OrMask, ScHsioTable[Entry].LanePhyMode));
        SideBandAndThenOr32(
          (UINT8) ScHsioTable[Entry].PortId,
          (UINT16) ScHsioTable[Entry].Offset,
          ScHsioTable[Entry].AndMask,
          ScHsioTable[Entry].OrMask
          );
      } else {
        DEBUG ((DEBUG_INFO, "Skipping HSIO BDCAST : PortID = %02x, Offset = %04x, AndMask = %08x, Value = %08x, LanePhyMode = %02x\n", (SC_SBI_PID) ScHsioTable[Entry].PortId, (UINT16) ScHsioTable[Entry].Offset, (UINT32) ScHsioTable[Entry].AndMask, (UINT32) ScHsioTable[Entry].OrMask, ScHsioTable[Entry].LanePhyMode));
      }
    } else if ((ScHsioTable[Entry].Offset & B_SC_HSIO_ACCESS_TYPE) == V_SC_HSIO_ACCESS_TYPE_UCAST) {
      if (ScLosPhyModeCheck ((SC_SBI_PID) ScHsioTable[Entry].PortId, ScHsioTable[Entry].LanePhyMode, Los1, Los2, Los3, Los4) == EFI_SUCCESS) {
        if (ScHsioTable[Entry].LanePhyMode == V_SC_PCR_FIA_LANE_OWN_USB3) {
          for (Index = 0; Index < Usb3LaneAddrSize; Index++) {
            if (ScGetUsb3LaneNum (Index, &LaneNum) == EFI_SUCCESS) {
              PortId = (UINT8)ScHsioTable[Entry].PortId;
              Offset = ScHsioTable[Entry].Offset| Usb3LaneAddr[Index];
              DEBUG ((DEBUG_INFO, "Programming HSIO UNICAST : PortID = %02x, Offset = %04x, AndMask = %08x, Value = %08x, LanePhyMode = %02x\n", (SC_SBI_PID) PortId, (UINT16) Offset, (UINT32) ScHsioTable[Entry].AndMask, (UINT32) ScHsioTable[Entry].OrMask, ScHsioTable[Entry].LanePhyMode));
              SideBandAndThenOr32 (
                PortId,
                Offset,
                ScHsioTable[Entry].AndMask,
                ScHsioTable[Entry].OrMask
                );
            }
          }
        } else if (ScHsioTable[Entry].LanePhyMode == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          for (Index = 0; Index < PcieLaneAddrSize; Index++) {
            if (ScGetPcieLaneNum (Index, &LaneNum) == EFI_SUCCESS ) {
              Offset = ScHsioTable[Entry].Offset| PcieLaneAddr[Index];
              DEBUG ((DEBUG_INFO, "Programming HSIO UNICAST : PortID = %02x, Offset = %04x, AndMask = %08x, Value = %08x, LanePhyMode = %02x\n", (SC_SBI_PID) ScHsioTable[Entry].PortId, Offset, (UINT32) ScHsioTable[Entry].AndMask, (UINT32) ScHsioTable[Entry].OrMask, ScHsioTable[Entry].LanePhyMode));
              SideBandAndThenOr32 (
                (UINT8) ScHsioTable[Entry].PortId,
                Offset,
                ScHsioTable[Entry].AndMask,
                ScHsioTable[Entry].OrMask
                );
            }
          }
        } else {
          for (Index = 0; Index < SataLaneAddrSize; Index++) {
            if (ScGetSataLaneNum (Index, &LaneNum) == EFI_SUCCESS) {
              Offset = ScHsioTable[Entry].Offset| SataLaneAddr[Index];
              DEBUG ((DEBUG_INFO, "Programming HSIO UNICAST : PortID = %02x, Offset = %04x, AndMask = %08x, Value = %08x, LanePhyMode = %02x\n", (SC_SBI_PID) ScHsioTable[Entry].PortId, Offset, (UINT32) ScHsioTable[Entry].AndMask, (UINT32) ScHsioTable[Entry].OrMask, ScHsioTable[Entry].LanePhyMode));
              SideBandAndThenOr32 (
                (UINT8) ScHsioTable[Entry].PortId,
                Offset,
                ScHsioTable[Entry].AndMask,
                ScHsioTable[Entry].OrMask
                );
            }
          }
        }
      } else {
        DEBUG ((DEBUG_INFO, "Skipping HSIO UNICAST : PortID = %02x, Offset = %04x, AndMask = %08x, Value = %08x, LanePhyMode = %02x\n", (SC_SBI_PID) ScHsioTable[Entry].PortId, (UINT16) ScHsioTable[Entry].Offset, (UINT32) ScHsioTable[Entry].AndMask, (UINT32) ScHsioTable[Entry].OrMask, ScHsioTable[Entry].LanePhyMode));
      }
    } else {
      ///
      /// Step 3b
      /// If the HSIO offset is targeted to all the data lanes attached to the common lane (MULCAST)
      /// or the offset target is the common lane then program without any checks
      ///
      DEBUG ((DEBUG_INFO, "HSIO : PortID = %02x, Offset = %04x, AndMask = %08x, Value = %08x\n", (SC_SBI_PID) ScHsioTable[Entry].PortId, (UINT16) ScHsioTable[Entry].Offset, (UINT32) ScHsioTable[Entry].AndMask, (UINT32) ScHsioTable[Entry].OrMask));
      SideBandAndThenOr32(
        (UINT8) ScHsioTable[Entry].PortId,
        (UINT16) ScHsioTable[Entry].Offset,
        ScHsioTable[Entry].AndMask,
        ScHsioTable[Entry].OrMask
        );
    }
  }

  DEBUG ((DEBUG_INFO, "ScHsioCommonBiosProg () - End\n"));
  return EFI_SUCCESS;
}

/**
  The function perform all the ModPhy programming that required by silicon
  in early PEI phase before devices get initialized.

  @param[in] None           No parameter to be input.

  @retval EFI_SUCCESS       The function completed successfully
  @retval Others            All other error conditions encountered result in an ASSERT.
**/
EFI_STATUS
EFIAPI
ScHsioBiosProg (
  IN  SI_POLICY_PPI  *SiPolicy,
  IN  SC_POLICY_PPI  *ScPolicy
  )
{
  UINTN         XhciPciBase;
  UINT32        XhciMmioBase;
  BXT_SERIES    BxtSeries;
  SC_USB_CONFIG *UsbConfig;
  EFI_STATUS    Status;
//[-start-160419-IB07400718-add]//
  UINT8         PmcFwVerWbuffer[8];
//[-end-160419-IB07400718-add]//

  DEBUG ((DEBUG_INFO, "ScHsioBiosProg () - Start\n"));

  ScHsioCommonBiosProg (ScPolicy);

  Status = GetConfigBlock ((VOID *) ScPolicy, &gUsbConfigGuid, (VOID *) &UsbConfig);
  ASSERT_EFI_ERROR (Status);

  XhciPciBase  = MmPciBase (
                  DEFAULT_PCI_BUS_NUMBER_SC,
                  PCI_DEVICE_NUMBER_XHCI,
                  PCI_FUNCTION_NUMBER_XHCI
                  );

  XhciMmioBase = (UINT32) (SiPolicy->TempMemBaseAddr);
  ASSERT (MmioRead32 (XhciPciBase) != 0xFFFFFFFF);
  //
  // Enable MSE of XHCI PCI offset 04h[1] to enable memory decode
  //
  MmioAnd8 (XhciPciBase + PCI_COMMAND_OFFSET, (UINT8)~EFI_PCI_COMMAND_MEMORY_SPACE);
  MmioWrite32 (XhciPciBase + R_XHCI_MEM_BASE, XhciMmioBase);
  MmioOr8 (XhciPciBase + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE);
  BxtSeries = GetBxtSeries ();
  if (BxtSeries == Bxt1) {
    // UFS MPHY initialization
    ScUFSPhyPromgram ();
    ScUsbSsicPhyInit (UsbConfig, XhciPciBase, XhciMmioBase);
    ScUsb2PhyInit (UsbConfig, XhciPciBase, XhciMmioBase);
    DEBUG ((DEBUG_INFO, "USB USB3(ModPhy)/USB2/HSIC/SSIC Programming()\n"));
  } else if (BxtSeries == BxtP) {
    ScUsbSsicPhyInit (UsbConfig, XhciPciBase, XhciMmioBase);
    ScUsb2PhyInit (UsbConfig, XhciPciBase, XhciMmioBase);
  }

//[-start-160218-IB07400702-modify]//
  Status = ReadyForNewIpc();
  if (!EFI_ERROR (Status)) {
    IpcSendCommandEx (IPC_CMD_ID_PHY_CFG,IPC_SUBCMD_ID_PHY_CFG_COMPLETE, &mIpcWbuffer, 2);
  } else {
    CHIPSET_POST_CODE (PEI_FRC_SC_INIT_IPC_NOT_READY);
    DEBUG ((EFI_D_ERROR, "IPC not ready!! skip send command!!\n"));
  }
//[-end-160218-IB07400702-modify]//
//[-start-160419-IB07400718-add]//
  //
  // Get PMC FW Version
  //
  Status = ReadyForNewIpc();
  if (!EFI_ERROR (Status)) {
    Status = IpcSendCommandEx (IPC_CMD_ID_PMC_VER,IPC_SUBCMD_ID_PMC_VER_READ, &PmcFwVerWbuffer, 8);
    DEBUG ((EFI_D_ERROR, "IpcSendCommandEx = %r\n", Status));
    if (!EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "PmcFwVerWbuffer[0] = %x\n", PmcFwVerWbuffer[0]));
      DEBUG ((EFI_D_ERROR, "PmcFwVerWbuffer[1] = %x\n", PmcFwVerWbuffer[1]));
      DEBUG ((EFI_D_ERROR, "PmcFwVerWbuffer[2] = %x\n", PmcFwVerWbuffer[2]));
      DEBUG ((EFI_D_ERROR, "PmcFwVerWbuffer[3] = %x\n", PmcFwVerWbuffer[3]));
      DEBUG ((EFI_D_ERROR, "PmcFwVerWbuffer[4] = %x\n", PmcFwVerWbuffer[4]));
      DEBUG ((EFI_D_ERROR, "PmcFwVerWbuffer[5] = %x\n", PmcFwVerWbuffer[5]));
      DEBUG ((EFI_D_ERROR, "PmcFwVerWbuffer[6] = %x\n", PmcFwVerWbuffer[6]));
      DEBUG ((EFI_D_ERROR, "PmcFwVerWbuffer[7] = %x\n", PmcFwVerWbuffer[7]));
      PcdSet32 (PcdPmcFwVersion, (PmcFwVerWbuffer[3] << 24) | (PmcFwVerWbuffer[2] << 16) | (PmcFwVerWbuffer[1] << 8) | PmcFwVerWbuffer[0]);
    }
  }
//[-end-160419-IB07400718-add]//
  //
  // Clear BAR and disable memory decode
  //
  MmioAnd8 (XhciPciBase + PCI_COMMAND_OFFSET, (UINT8)~(EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER));
  MmioWrite32 ((UINTN) (XhciPciBase + R_XHCI_MEM_BASE), 0);

  DEBUG ((DEBUG_INFO, "ScHsioBiosProg() - End\n"));
  return EFI_SUCCESS;
}


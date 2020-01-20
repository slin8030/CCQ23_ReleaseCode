/** @file
  This file is PeiScPolicy library.

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

#include "PeiScPolicyLibrary.h"
#include <Library/SteppingLib.h>

GLOBAL_REMOVE_IF_UNREFERENCED UINT8 mSmbusRsvdAddresses[] = {
  0xA0,
  0xA2,
  0xA4,
  0xA6
};

//[-start-160813-IB07400770-modify]//
#ifdef LEGACY_IRQ_SUPPORT
GLOBAL_REMOVE_IF_UNREFERENCED PRIVATE_PCICFGCTRL mDirectIrqTable[] = {
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C0,  27, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C1,  28, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C2,  29, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C3,  30, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C4,  31, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C5,  32, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C6,  33, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C7,  34, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART0, 44, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART1, 45, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART2,  6, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART3, 47, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI0,  35, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI1,  36, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI2,  37, V_PCICFG_CTRL_INTC },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_SDCARD, 43, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_EMMC  , 39, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_UFS   , 38, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_SDIO  , 42, V_PCICFG_CTRL_INTA },
  { SB_ISH_BRIDGE_PORT,  R_ISH_SB_PCICFGCTRL_ISH   , 26, V_PCICFG_CTRL_INTA },
  { SB_USB_DEVICE_PORT,  R_USB_SB_PCICFGCTRL_XDCI  , 13, V_PCICFG_CTRL_INTB },
};
#else
GLOBAL_REMOVE_IF_UNREFERENCED PRIVATE_PCICFGCTRL mDirectIrqTable[] = {
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C0,  27, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C1,  28, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C2,  29, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C3,  30, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C4,  31, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C5,  32, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C6,  33, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C7,  34, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART0,  4, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART1,  5, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART2,  6, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART3,  7, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI0,  35, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI1,  36, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI2,  37, V_PCICFG_CTRL_INTC },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_SDCARD,  3, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_EMMC  , 39, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_UFS   , 38, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_SDIO  , 42, V_PCICFG_CTRL_INTA },
  { SB_ISH_BRIDGE_PORT,  R_ISH_SB_PCICFGCTRL_ISH   , 26, V_PCICFG_CTRL_INTA },
  { SB_USB_DEVICE_PORT,  R_USB_SB_PCICFGCTRL_XDCI  , 13, V_PCICFG_CTRL_INTB },
};
#endif
//[-end-160813-IB07400770-modify]//

/**
  mDevIntConfig[] table contains data on INTx and IRQ for each device.


  PCI Express Root Ports mapping should be programmed only with values as in below table (D19/20)
  otherwise _PRT methods in ACPI for RootPorts would require additional patching as
  PCIe Endpoint Device Interrupt is further subjected to INTx to PIRQy Mapping

  Configured IRQ values are not used if an OS chooses to be in PIC instead of APIC mode
**/
GLOBAL_REMOVE_IF_UNREFERENCED SC_DEVICE_INTERRUPT_CONFIG mDevIntConfig[] = {
  {19, 0, ScIntA, 16}, // PCI Express Port 3, INT is default, programmed in PciCfgSpace + FCh
  {19, 1, ScIntB, 17}, // PCI Express Port 4, INT is default, programmed in PciCfgSpace + FCh
  {19, 2, ScIntC, 18}, // PCI Express Port 5, INT is default, programmed in PciCfgSpace + FCh
  {19, 3, ScIntD, 19}, // PCI Express Port 6, INT is default, programmed in PciCfgSpace + FCh
  {20, 0, ScIntA, 16}, // PCI Express Port 1 (APL Only), INT is default, programmed in PciCfgSpace + FCh
  {20, 1, ScIntB, 17}, // PCI Express Port 2 (APL Only), INT is default, programmed in PciCfgSpace + FCh
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT8 mPortUsb20PerPortTxPeHalf[]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
GLOBAL_REMOVE_IF_UNREFERENCED UINT8 mPortUsb20PerPortPeTxiSet[]  = {0x07, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x01};
GLOBAL_REMOVE_IF_UNREFERENCED UINT8 mPortUsb20PerPortTxiSet[]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};
GLOBAL_REMOVE_IF_UNREFERENCED UINT8 mPortUsb20HsSkewSel[]        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
GLOBAL_REMOVE_IF_UNREFERENCED UINT8 mPortUsb20IUsbTxEmphasisEn[] = {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01};
GLOBAL_REMOVE_IF_UNREFERENCED UINT8 mPortUsb20PerPortRXISet[]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
GLOBAL_REMOVE_IF_UNREFERENCED UINT8 mPortUsb20HsNpreDrvSel[]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadScGeneralConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_GENERAL_CONFIG *ScGeneralConfig;

  ScGeneralConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "ScGeneralConfig->Header.Guid = %g\n", &ScGeneralConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "ScGeneralConfig->Header.Size = 0x%x\n", ScGeneralConfig->Header.Size));

  /********************************
    General Config Configuration
  ********************************/
  ScGeneralConfig->SubSystemVendorId = V_INTEL_VENDOR_ID;
  ScGeneralConfig->SubSystemId       = V_SC_DEFAULT_SID;
  ScGeneralConfig->AcpiBase          = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  ScGeneralConfig->PmcBase           = (UINT32)PcdGet32(PcdPmcIpc1BaseAddress0);
  ScGeneralConfig->P2sbBase          = (UINT32)PcdGet32(PcdP2SBBaseAddress);
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadSataConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_SATA_CONFIG  *SataConfig;
  UINT8           PortIndex;

  SataConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "SataConfig->Header.Guid = %g\n", &SataConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "SataConfig->Header.Size = 0x%x\n", SataConfig->Header.Size));

  /********************************
    SATA Configuration
  ********************************/
  SataConfig->Enable      = TRUE;
  SataConfig->SalpSupport = TRUE;
  SataConfig->SataMode    = ScSataModeAhci;
  SataConfig->SpeedLimit  = ScSataSpeedDefault;

  for (PortIndex = 0; PortIndex < SC_MAX_SATA_PORTS; PortIndex++) {
    SataConfig->PortSettings[PortIndex].Enable  = TRUE;
    SataConfig->PortSettings[PortIndex].DmVal   = 15;
    SataConfig->PortSettings[PortIndex].DitoVal = 625;
  }
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadPcieConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  SC_PCIE_CONFIG  *PcieConfig;
  UINT8           PortIndex;

  PcieConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "PcieConfig->Header.Guid = %g\n", &PcieConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "PcieConfig->Header.Size = 0x%x\n", PcieConfig->Header.Size));

  /********************************
    PCIe Configuration
  ********************************/
  for (PortIndex = 0; PortIndex < GetScMaxPciePortNum (); PortIndex++) {
    PcieConfig->RootPort[PortIndex].Aspm                 = ScPcieAspmAutoConfig;
    PcieConfig->RootPort[PortIndex].Enable               = TRUE;
    PcieConfig->RootPort[PortIndex].SlotImplemented      = TRUE;
    PcieConfig->RootPort[PortIndex].PmSci                = TRUE;
    PcieConfig->RootPort[PortIndex].AcsEnabled           = TRUE;
    PcieConfig->RootPort[PortIndex].PhysicalSlotNumber   = PortIndex;
    PcieConfig->RootPort[PortIndex].L1Substates          = ScPcieL1SubstatesL1_1_2;
    PcieConfig->RootPort[PortIndex].SelectableDeemphasis = TRUE;
    //
    // PCIe LTR Configuration.
    //
    PcieConfig->RootPort[PortIndex].LtrEnable                         = TRUE;
    PcieConfig->RootPort[PortIndex].LtrMaxSnoopLatency                = 0x1003;
    PcieConfig->RootPort[PortIndex].LtrMaxNoSnoopLatency              = 0x1003;
    PcieConfig->RootPort[PortIndex].SnoopLatencyOverrideMode          = 2;
    PcieConfig->RootPort[PortIndex].SnoopLatencyOverrideMultiplier    = 2;
    PcieConfig->RootPort[PortIndex].SnoopLatencyOverrideValue         = 60;
    PcieConfig->RootPort[PortIndex].NonSnoopLatencyOverrideMode       = 2;
    PcieConfig->RootPort[PortIndex].NonSnoopLatencyOverrideMultiplier = 2;
    PcieConfig->RootPort[PortIndex].NonSnoopLatencyOverrideValue      = 60;
  }
  PcieConfig->AspmSwSmiNumber = SW_SMI_PCIE_ASPM_OVERRIDE;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadUsbConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_USB_CONFIG  *UsbConfig;
  UINT32         PortIndex;

  UsbConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "UsbConfig->Header.Guid = %g\n", &UsbConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "UsbConfig->Header.Size = 0x%x\n", UsbConfig->Header.Size));

  /********************************
    USB Configuration
  ********************************/
  UsbConfig->Usb30Settings.Mode = XHCI_MODE_ON;
  for (PortIndex = 0; PortIndex < GetScXhciMaxUsb2PortNum (); PortIndex++) {
    UsbConfig->PortUsb20[PortIndex].Enable           = TRUE;
    UsbConfig->PortUsb20[PortIndex].AfeConfig.PerPortTxPeHalf  = (UINT32) mPortUsb20PerPortTxPeHalf[PortIndex];
    UsbConfig->PortUsb20[PortIndex].AfeConfig.PerPortPeTxiSet  = (UINT32) mPortUsb20PerPortPeTxiSet[PortIndex];
    UsbConfig->PortUsb20[PortIndex].AfeConfig.PerPortTxiSet    = (UINT32) mPortUsb20PerPortTxiSet[PortIndex];
    UsbConfig->PortUsb20[PortIndex].AfeConfig.IUsbTxEmphasisEn = (UINT32) mPortUsb20IUsbTxEmphasisEn[PortIndex];
    UsbConfig->PortUsb20[PortIndex].AfeConfig.PerPortRXISet    = (UINT32) mPortUsb20PerPortRXISet[PortIndex];
    UsbConfig->PortUsb20[PortIndex].AfeConfig.HsSkewSel        = (UINT32) mPortUsb20HsSkewSel[PortIndex];
    UsbConfig->PortUsb20[PortIndex].AfeConfig.HsNpreDrvSel     = (UINT32) mPortUsb20HsNpreDrvSel[PortIndex];
  }

  for (PortIndex = 0; PortIndex < GetScXhciMaxUsb3PortNum (); PortIndex++) {
    UsbConfig->PortUsb30[PortIndex].Enable = TRUE;
  }
  UsbConfig->XdciConfig.Enable = ScPciMode;
  UsbConfig->SsicConfig.SsicPort[0].Enable = FALSE;
  UsbConfig->SsicConfig.SsicPort[1].Enable = FALSE;
  UsbConfig->SsicConfig.SsicPort[0].Rate   = XhciSsicRateA;
  UsbConfig->SsicConfig.SsicPort[1].Rate   = XhciSsicRateA;
  UsbConfig->SsicConfig.DlanePwrGating     = TRUE;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadHpetConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_HPET_CONFIG  *HpetConfig;

  HpetConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "HpetConfig->Header.Guid = %g\n", &HpetConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "HpetConfig->Header.Size = 0x%x\n", HpetConfig->Header.Size));

  /********************************
    HPET Configuration
  ********************************/
  HpetConfig->Enable = TRUE;
  HpetConfig->Base   = SC_HPET_BASE_ADDRESS;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadSmbusConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_SMBUS_CONFIG  *SmbusConfig;

  SmbusConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "SmbusConfig->Header.Guid = %g\n", &SmbusConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "SmbusConfig->Header.Size = 0x%x\n", SmbusConfig->Header.Size));

  /********************************
    SMBus Configuration
  ********************************/
  SmbusConfig->Enable      = TRUE;
  SmbusConfig->SmbusIoBase = PcdGet16(PcdSmbusBaseAddress);
  ASSERT (sizeof (mSmbusRsvdAddresses) <= SC_MAX_SMBUS_RESERVED_ADDRESS);
  SmbusConfig->NumRsvdSmbusAddresses = sizeof (mSmbusRsvdAddresses);
  CopyMem (
    SmbusConfig->RsvdSmbusAddressTable,
    mSmbusRsvdAddresses,
    sizeof (mSmbusRsvdAddresses)
    );
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadIoApicConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_IOAPIC_CONFIG  *IoApicConfig;

  IoApicConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "IoApicConfig->Header.Guid = %g\n", &IoApicConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "IoApicConfig->Header.Size = 0x%x\n", IoApicConfig->Header.Size));

  /********************************
    IOAPIC Configuration
  ********************************/
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadHdaConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_HDAUDIO_CONFIG   *HdaConfig;

  HdaConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "HdaConfig->Header.Guid = %g\n", &HdaConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "HdaConfig->Header.Size = 0x%x\n", HdaConfig->Header.Size));

  /********************************
    HD-Audio Configuration
  ********************************/
  HdaConfig->Enable                = TRUE;
  HdaConfig->DspEnable             = TRUE;
  HdaConfig->Mmt                   = ScHdaVc0;
  HdaConfig->Hmt                   = ScHdaVc0;
  HdaConfig->PwrGate               = TRUE;
  HdaConfig->ClkGate               = TRUE;
  HdaConfig->Pme                   = TRUE;
  HdaConfig->IoBufferOwnership     = ScHdaIoBufOwnerI2sPort;
  HdaConfig->VcType                = ScHdaVc0;
  HdaConfig->HdAudioLinkFrequency  = ScHdaLinkFreq24MHz;
  HdaConfig->IDispLinkFrequency    = ScHdaLinkFreq96MHz;
  HdaConfig->IDispLinkTmode        = ScHdaIDispMode2T;
  HdaConfig->ResetWaitTimer        = 300;
  HdaConfig->I2sConfiguration      = 1;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadGmmConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_GMM_CONFIG  *GmmConfig;

  GmmConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "GmmConfig->Header.Guid = %g\n", &GmmConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "GmmConfig->Header.Size = 0x%x\n", GmmConfig->Header.Size));

  /********************************
    GMM Configuration
  ********************************/
  GmmConfig->ClkGatingPgcbClkTrunk   = TRUE;
  GmmConfig->ClkGatingSb             = TRUE;
  GmmConfig->ClkGatingSbClkTrunk     = TRUE;
  GmmConfig->ClkGatingSbClkPartition = TRUE;
  GmmConfig->ClkGatingCore           = TRUE;
  GmmConfig->ClkGatingDma            = TRUE;
  GmmConfig->ClkGatingRegAccess      = TRUE;
  GmmConfig->ClkGatingHost           = TRUE;
  GmmConfig->ClkGatingPartition      = TRUE;
  GmmConfig->ClkGatingTrunk          = TRUE;
  GmmConfig->SvPwrGatingHwAutoEnable = TRUE;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadPmConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_PM_CONFIG  *PmConfig;

  PmConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "PmConfig->Header.Guid = %g\n", &PmConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "PmConfig->Header.Size = 0x%x\n", PmConfig->Header.Size));

  /********************************
    PM Configuration
  ********************************/
  PmConfig->PciClockRun        = TRUE;
  PmConfig->Timer8254ClkGateEn = TRUE;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadLockDownConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_LOCK_DOWN_CONFIG *LockDownConfig;

  LockDownConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "LockDownConfig->Header.Guid = %g\n", &LockDownConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "LockDownConfig->Header.Size = 0x%x\n", LockDownConfig->Header.Size));

  /********************************
    Lock Down Configuration
  ********************************/

  LockDownConfig->GlobalSmi           = TRUE;
  //
  // Flash Security Recommendations,
  // Intel strongly recommends that BIOS sets the BIOS Interface Lock Down bit. Enabling this bit
  // will mitigate malicious software attempts to replace the system BIOS option ROM with its own code.
  // Here we always enable this as a Policy.
  //
  LockDownConfig->BiosInterface       = TRUE;
  LockDownConfig->RtcLock             = TRUE;
  LockDownConfig->BiosLockSwSmiNumber = SW_SMI_BIOS_LOCK;
  LockDownConfig->TcoLock             = TRUE;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadSerialIrqConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_LPC_SIRQ_CONFIG *SerialIrqConfig;

  SerialIrqConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "SerialIrqConfig->Header.Guid = %g\n", &SerialIrqConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "SerialIrqConfig->Header.Size = 0x%x\n", SerialIrqConfig->Header.Size));

  /********************************
    Serial IRQ Configuration
  ********************************/
  SerialIrqConfig->SirqEnable      = TRUE;
  SerialIrqConfig->SirqMode        = ScQuietMode;
  SerialIrqConfig->StartFramePulse = ScSfpw4Clk;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadLpssConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_LPSS_CONFIG  *LpssConfig;
  UINT8           Index;

  LpssConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "LpssConfig->Header.Guid = %g\n", &LpssConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "LpssConfig->Header.Size = 0x%x\n", LpssConfig->Header.Size));

  /********************************
    LPSS Configuration
  ********************************/
  LpssConfig->I2c0Enable    = ScPciMode;
  LpssConfig->I2c1Enable    = ScPciMode;
  LpssConfig->I2c2Enable    = ScPciMode;
  LpssConfig->I2c3Enable    = ScPciMode;
  LpssConfig->I2c4Enable    = ScPciMode;
  LpssConfig->I2c5Enable    = ScPciMode;
  LpssConfig->I2c6Enable    = ScPciMode;
  LpssConfig->I2c7Enable    = ScPciMode;
  LpssConfig->Hsuart0Enable = ScPciMode;
  LpssConfig->Hsuart1Enable = ScPciMode;
  LpssConfig->Hsuart2Enable = ScPciMode;
  LpssConfig->Hsuart3Enable = ScPciMode;
  LpssConfig->Spi0Enable    = ScPciMode;
  LpssConfig->Spi1Enable    = ScPciMode;
  LpssConfig->Spi2Enable    = ScPciMode;
  for (Index = 0; Index < LPSS_I2C_DEVICE_NUM; Index++) {
    LpssConfig->I2cClkGateCfg[Index] = TRUE;
  }
  for (Index = 0; Index < LPSS_HSUART_DEVICE_NUM; Index++) {
    LpssConfig->HsuartClkGateCfg[Index] = TRUE;
  }
  for (Index = 0; Index < LPSS_SPI_DEVICE_NUM; Index++) {
    LpssConfig->SpiClkGateCfg[Index] = TRUE;
  }
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadScsConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_SCS_CONFIG  *ScsConfig;

  ScsConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "ScsConfig->Header.Guid = %g\n", &ScsConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "ScsConfig->Header.Size = 0x%x\n", ScsConfig->Header.Size));

  /********************************
    SCS Configuration
  ********************************/
  ScsConfig->SdcardEnable     = ScPciMode;
  ScsConfig->SdioEnable       = ScPciMode;
  ScsConfig->EmmcEnable       = ScPciMode;
  ScsConfig->EmmcHostMaxSpeed = SccEmmcHs400;
  ScsConfig->GppLock          = 0;
  ScsConfig->SccEmmcTraceLength = SCC_EMMC_LONG_TRACE_LEN;
  ScsConfig->SdioRegDllConfig.TxCmdCntl      = 0x505;
  ScsConfig->SdioRegDllConfig.TxDataCntl1    = 0xE;
  ScsConfig->SdioRegDllConfig.TxDataCntl2    = 0x22272828;
  ScsConfig->SdioRegDllConfig.RxCmdDataCntl1 = 0x16161616;
  ScsConfig->SdioRegDllConfig.RxCmdDataCntl2 = 0x10000;
  ScsConfig->SdcardRegDllConfig.TxCmdCntl    = 0x505;
  if (GetBxtSeries () == Bxt1) {
    ScsConfig->SdcardRegDllConfig.TxDataCntl1    = 0xA12;
    ScsConfig->SdcardRegDllConfig.TxDataCntl2    = 0x26272727;
    ScsConfig->SdcardRegDllConfig.RxCmdDataCntl1 = 0x32483B3B;
  } else {
    ScsConfig->SdcardRegDllConfig.TxDataCntl1    = 0xA13;
    ScsConfig->SdcardRegDllConfig.TxDataCntl2    = 0x24242828;
    ScsConfig->SdcardRegDllConfig.RxCmdDataCntl1 = 0x73A3637;
  }
  ScsConfig->SdcardRegDllConfig.RxStrobeCntl   = 0x0;
  ScsConfig->SdcardRegDllConfig.RxCmdDataCntl2 = 0x10000;

  if (ScsConfig->SccEmmcTraceLength == SCC_EMMC_SHORT_TRACE_LEN) {
    // Configure DLL settings for short trace length
    ScsConfig->EmmcRegDllConfig.TxCmdCntl      = 0x505;
    ScsConfig->EmmcRegDllConfig.TxDataCntl1    = 0xC15;
    ScsConfig->EmmcRegDllConfig.TxDataCntl2    = 0x1C1C1C00;
    ScsConfig->EmmcRegDllConfig.RxCmdDataCntl1 = 0x1C1C1C00;
    ScsConfig->EmmcRegDllConfig.RxStrobeCntl   = 0x0a0a;
    ScsConfig->EmmcRegDllConfig.RxCmdDataCntl2 = 0x1001C;
    ScsConfig->EmmcRegDllConfig.MasterSwCntl   = 0x001;
  } else {
    // Configure DLL settings for long trace length
    ScsConfig->EmmcRegDllConfig.TxCmdCntl      = 0x505;
    ScsConfig->EmmcRegDllConfig.TxDataCntl1    = 0xC11;
    ScsConfig->EmmcRegDllConfig.TxDataCntl2    = 0x1C2A2927;
    ScsConfig->EmmcRegDllConfig.RxCmdDataCntl1 = 0x000D162F;
    ScsConfig->EmmcRegDllConfig.RxStrobeCntl   = 0x0a0a;
    ScsConfig->EmmcRegDllConfig.RxCmdDataCntl2 = 0x1003b;
    ScsConfig->EmmcRegDllConfig.MasterSwCntl   = 0x001;
  }
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadVtdConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_VTD_CONFIG  *VtdConfig;

  VtdConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "VtdConfig->Header.Guid = %g\n", &VtdConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "VtdConfig->Header.Size = 0x%x\n", VtdConfig->Header.Size));

  /********************************
    VT-d Configuration
  ********************************/
  VtdConfig->RmrrUsbBaseAddr = 0;
  VtdConfig->RmrrUsbLimit = 0;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadIshConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_ISH_CONFIG  *IshConfig;

  IshConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "IshConfig->Header.Guid = %g\n", &IshConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "IshConfig->Header.Size = 0x%x\n", IshConfig->Header.Size));

  /********************************
    ISH Configuration
  ********************************/
  IshConfig->Enable = TRUE;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadFlashProtectionConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  SC_FLASH_PROTECTION_CONFIG  *FlashProtectionConfig;

  FlashProtectionConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "FlashProtectionConfig->Header.Guid = %g\n", &FlashProtectionConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "FlashProtectionConfig->Header.Size = 0x%x\n", FlashProtectionConfig->Header.Size));

  /********************************
    Flash Protection Configuration
  ********************************/
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadDciConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_DCI_CONFIG  *DciConfig;

  DciConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "DciConfig->Header.Guid = %g\n", &DciConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "DciConfig->Header.Size = 0x%x\n", DciConfig->Header.Size));

  /********************************
    DCI Configuration
  ********************************/
  DciConfig->DciAutoDetect = TRUE;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/

VOID
LoadP2sbConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  SC_P2SB_CONFIG  *P2sbConfig;

  P2sbConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "P2sbConfig->Header.Guid = %g\n", &P2sbConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "P2sbConfig->Header.Size = 0x%x\n", P2sbConfig->Header.Size));

  /********************************
    P2SB Configuration
  ********************************/

  P2sbConfig->P2sbUnhide = 0;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadInterruptConfigDefault (
  IN VOID         *ConfigBlockPointer
  )
{
  UINT8                IntConfigTableEntries;
  UINT8                DirectIrqTableEntries;
  SC_INTERRUPT_CONFIG  *InterruptConfig;

  InterruptConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "InterruptConfig->Header.Guid = %g\n", &InterruptConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "InterruptConfig->Header.Size = 0x%x\n", InterruptConfig->Header.Size));

  /********************************
    Interrupt Configuration
  ********************************/

  DirectIrqTableEntries = sizeof (mDirectIrqTable) / sizeof (PRIVATE_PCICFGCTRL);
  ASSERT (DirectIrqTableEntries <= SC_MAX_DIRECT_IRQ_CONFIG);
  InterruptConfig->NumOfDirectIrqTable = DirectIrqTableEntries;
  CopyMem (
    InterruptConfig->DirectIrqTable,
    mDirectIrqTable,
    sizeof (mDirectIrqTable)
    );

  IntConfigTableEntries = sizeof (mDevIntConfig) / sizeof (SC_DEVICE_INTERRUPT_CONFIG);
  ASSERT (IntConfigTableEntries <= SC_MAX_DEVICE_INTERRUPT_CONFIG);
  InterruptConfig->NumOfDevIntConfig = IntConfigTableEntries;
  CopyMem (
    InterruptConfig->DevIntConfig,
    mDevIntConfig,
    sizeof (mDevIntConfig)
    );

//[-start-160704-IB07400752-modify]//
#ifndef LEGACY_IRQ_SUPPORT
  InterruptConfig->PxRcRouting[0] = V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ3;   // R_ITSS_SB_PARC    PIRQA->IRQx Routing Control
  InterruptConfig->PxRcRouting[1] = V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ4;   // R_ITSS_SB_PBRC    PIRQB->IRQx Routing Control
  InterruptConfig->PxRcRouting[2] = V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ5;   // R_ITSS_SB_PCRC    PIRQC->IRQx Routing Control
  InterruptConfig->PxRcRouting[3] = V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ6;   // R_ITSS_SB_PDRC    PIRQD->IRQx Routing Control
  InterruptConfig->PxRcRouting[4] = V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ7;   // R_ITSS_SB_PERC    PIRQE->IRQx Routing Control
  InterruptConfig->PxRcRouting[5] = V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ9;   // R_ITSS_SB_PFRC    PIRQF->IRQx Routing Control
  InterruptConfig->PxRcRouting[6] = V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ10;  // R_ITSS_SB_PGRC    PIRQG->IRQx Routing Control
  InterruptConfig->PxRcRouting[7] = V_ITSS_SB_REN_ENABLE + V_ITSS_SB_IR_IRQ11;  // R_ITSS_SB_PHRC    PIRQH->IRQx Routing Control
#endif
//[-end-160704-IB07400752-modify]//
  //
  // Configure Interrupt Polarity
  //
  InterruptConfig->IPC[0] = (UINT32)
    ((V_ITSS_SB_IPC_ACTIVE_LOW << 31) + // IRQ 31
    (V_ITSS_SB_IPC_ACTIVE_LOW << 30) +  // IRQ 30
    (V_ITSS_SB_IPC_ACTIVE_LOW << 29) +  // IRQ 29
    (V_ITSS_SB_IPC_ACTIVE_LOW << 28) +  // IRQ 28
    (V_ITSS_SB_IPC_ACTIVE_LOW << 27) +  // IRQ 27
    (V_ITSS_SB_IPC_ACTIVE_LOW << 26) +  // IRQ 26
    (V_ITSS_SB_IPC_ACTIVE_LOW << 25) +  // IRQ 25
    (V_ITSS_SB_IPC_ACTIVE_LOW << 24) +  // IRQ 24
    (V_ITSS_SB_IPC_ACTIVE_LOW << 23) +  // IRQ 23
    (V_ITSS_SB_IPC_ACTIVE_LOW << 22) +  // IRQ 22
    (V_ITSS_SB_IPC_ACTIVE_LOW << 21) +  // IRQ 21
    (V_ITSS_SB_IPC_ACTIVE_LOW << 20) +  // IRQ 20
    (V_ITSS_SB_IPC_ACTIVE_LOW << 19) +  // IRQ 19
    (V_ITSS_SB_IPC_ACTIVE_LOW << 18) +  // IRQ 18
    (V_ITSS_SB_IPC_ACTIVE_LOW << 17) +  // IRQ 17
    (V_ITSS_SB_IPC_ACTIVE_LOW << 16) +  // IRQ 16
    (V_ITSS_SB_IPC_ACTIVE_LOW << 15) +  // IRQ 15
    (V_ITSS_SB_IPC_ACTIVE_LOW << 14) +  // IRQ 14
    (V_ITSS_SB_IPC_ACTIVE_LOW << 13) +  // IRQ 13
    (V_ITSS_SB_IPC_ACTIVE_HIGH << 12) + // IRQ 12
    (V_ITSS_SB_IPC_ACTIVE_LOW << 11) +  // IRQ 11
    (V_ITSS_SB_IPC_ACTIVE_LOW << 10) +  // IRQ 10
    (V_ITSS_SB_IPC_ACTIVE_LOW << 9) +   // IRQ  9
    (V_ITSS_SB_IPC_ACTIVE_HIGH << 8) +  // IRQ  8
    (V_ITSS_SB_IPC_ACTIVE_LOW << 7) +   // IRQ  7
    (V_ITSS_SB_IPC_ACTIVE_LOW << 6) +   // IRQ  6
    (V_ITSS_SB_IPC_ACTIVE_LOW << 5) +   // IRQ  5
    (V_ITSS_SB_IPC_ACTIVE_LOW << 4) +   // IRQ  4
    (V_ITSS_SB_IPC_ACTIVE_LOW << 3) +   // IRQ  3
    (V_ITSS_SB_IPC_ACTIVE_HIGH << 2) +  // IRQ  2
    (V_ITSS_SB_IPC_ACTIVE_HIGH << 1) +  // IRQ  1
    (V_ITSS_SB_IPC_ACTIVE_HIGH << 0))   // IRQ  0
    ;

  InterruptConfig->IPC[1] = (UINT32)
    ((V_ITSS_SB_IPC_ACTIVE_LOW << 31) + // IRQ 63
    (V_ITSS_SB_IPC_ACTIVE_LOW << 30) +  // IRQ 62
    (V_ITSS_SB_IPC_ACTIVE_LOW << 29) +  // IRQ 61
    (V_ITSS_SB_IPC_ACTIVE_LOW << 28) +  // IRQ 60
    (V_ITSS_SB_IPC_ACTIVE_LOW << 27) +  // IRQ 59
    (V_ITSS_SB_IPC_ACTIVE_LOW << 26) +  // IRQ 58
    (V_ITSS_SB_IPC_ACTIVE_LOW << 25) +  // IRQ 57
    (V_ITSS_SB_IPC_ACTIVE_LOW << 24) +  // IRQ 56
    (V_ITSS_SB_IPC_ACTIVE_LOW << 23) +  // IRQ 55
    (V_ITSS_SB_IPC_ACTIVE_LOW << 22) +  // IRQ 54
    (V_ITSS_SB_IPC_ACTIVE_LOW << 21) +  // IRQ 53
    (V_ITSS_SB_IPC_ACTIVE_LOW << 20) +  // IRQ 52
    (V_ITSS_SB_IPC_ACTIVE_LOW << 19) +  // IRQ 51
    (V_ITSS_SB_IPC_ACTIVE_LOW << 18) +  // IRQ 50
    (V_ITSS_SB_IPC_ACTIVE_LOW << 17) +  // IRQ 49
    (V_ITSS_SB_IPC_ACTIVE_LOW << 16) +  // IRQ 48
    (V_ITSS_SB_IPC_ACTIVE_LOW << 15) +  // IRQ 47
    (V_ITSS_SB_IPC_ACTIVE_LOW << 14) +  // IRQ 46
    (V_ITSS_SB_IPC_ACTIVE_LOW << 13) +  // IRQ 45
    (V_ITSS_SB_IPC_ACTIVE_LOW << 12) +  // IRQ 44
    (V_ITSS_SB_IPC_ACTIVE_LOW << 11) +  // IRQ 43
    (V_ITSS_SB_IPC_ACTIVE_LOW << 10) +  // IRQ 42
    (V_ITSS_SB_IPC_ACTIVE_LOW << 9) +   // IRQ 41
    (V_ITSS_SB_IPC_ACTIVE_LOW << 8) +   // IRQ 40
    (V_ITSS_SB_IPC_ACTIVE_LOW << 7) +   // IRQ 39
    (V_ITSS_SB_IPC_ACTIVE_LOW << 6) +   // IRQ 38
    (V_ITSS_SB_IPC_ACTIVE_LOW << 5) +   // IRQ 37
    (V_ITSS_SB_IPC_ACTIVE_LOW << 4) +   // IRQ 36
    (V_ITSS_SB_IPC_ACTIVE_LOW << 3) +   // IRQ 35
    (V_ITSS_SB_IPC_ACTIVE_LOW << 2) +   // IRQ 34
    (V_ITSS_SB_IPC_ACTIVE_LOW << 1) +   // IRQ 33
    (V_ITSS_SB_IPC_ACTIVE_LOW << 0))    // IRQ 32
    ;
  InterruptConfig->IPC[2] = 0xFFFFFFFF;
  InterruptConfig->IPC[3] = 0xFFFFFFFF;
}

static IP_BLOCK_ENTRY  mScIpBlocks [] = {
  {&gScGeneralConfigGuid,         sizeof (SC_GENERAL_CONFIG),      SC_GENERAL_CONFIG_REVISION,             LoadScGeneralConfigDefault},
  {&gSataConfigGuid,              sizeof (SC_SATA_CONFIG),         SATA_CONFIG_REVISION,                   LoadSataConfigDefault},
  {&gPcieRpConfigGuid,            sizeof (SC_PCIE_CONFIG),         PCIE_RP_CONFIG_REVISION,                LoadPcieConfigDefault},
  {&gUsbConfigGuid,               sizeof (SC_USB_CONFIG),          USB_CONFIG_REVISION,                    LoadUsbConfigDefault},
  {&gHpetConfigGuid,              sizeof (SC_HPET_CONFIG),         HPET_CONFIG_REVISION,                   LoadHpetConfigDefault},
  {&gSmbusConfigGuid,             sizeof (SC_SMBUS_CONFIG),        SMBUS_CONFIG_REVISION,                  LoadSmbusConfigDefault},
  {&gIoApicConfigGuid,            sizeof (SC_IOAPIC_CONFIG),       IOAPIC_CONFIG_REVISION,                 LoadIoApicConfigDefault},
  {&gHdAudioConfigGuid,           sizeof (SC_HDAUDIO_CONFIG),      HDAUDIO_CONFIG_REVISION,                LoadHdaConfigDefault},
  {&gGmmConfigGuid,               sizeof (SC_GMM_CONFIG),          GMM_CONFIG_REVISION,                    LoadGmmConfigDefault},
  {&gPmConfigGuid,                sizeof (SC_PM_CONFIG),           PM_CONFIG_REVISION,                     LoadPmConfigDefault},
  {&gLockDownConfigGuid,          sizeof (SC_LOCK_DOWN_CONFIG),    LOCK_DOWN_CONFIG_REVISION,              LoadLockDownConfigDefault},
  {&gSerialIrqConfigGuid,         sizeof (SC_LPC_SIRQ_CONFIG),     SERIAL_IRQ_CONFIG_REVISION,             LoadSerialIrqConfigDefault},
  {&gLpssConfigGuid,              sizeof (SC_LPSS_CONFIG),         LPSS_CONFIG_REVISION,                   LoadLpssConfigDefault},
  {&gScsConfigGuid,               sizeof (SC_SCS_CONFIG),          SCS_CONFIG_REVISION,                    LoadScsConfigDefault},
  {&gVtdConfigGuid,               sizeof (SC_VTD_CONFIG),          VTD_CONFIG_REVISION,                    LoadVtdConfigDefault},
  {&gIshConfigGuid,               sizeof (SC_ISH_CONFIG),          ISH_CONFIG_REVISION,                    LoadIshConfigDefault},
  {&gFlashProtectionConfigGuid,   sizeof (SC_FLASH_PROTECTION_CONFIG),  FLASH_PROTECTION_CONFIG_REVISION,  LoadFlashProtectionConfigDefault},
  {&gDciConfigGuid,               sizeof (SC_DCI_CONFIG),          DCI_CONFIG_REVISION,                    LoadDciConfigDefault},
  {&gP2sbConfigGuid,              sizeof (SC_P2SB_CONFIG),         P2SB_CONFIG_REVISION,                   LoadP2sbConfigDefault},
  {&gInterruptConfigGuid,         sizeof (SC_INTERRUPT_CONFIG),    INTERRUPT_CONFIG_REVISION,              LoadInterruptConfigDefault},
};


/**
  Get SC config block table total size.

  @retval Size of SC config block table
**/
UINT32
EFIAPI
ScGetConfigBlockTotalSize (
  VOID
  )
{
  UINT32            TotalBlockCount;
  UINT32            TotalBlockSize;
  UINT32            ConfigBlockHdrSize;
  UINT32            BlockCount;

  TotalBlockCount = sizeof(mScIpBlocks) / sizeof (IP_BLOCK_ENTRY);
  TotalBlockSize = 0;
  for (BlockCount = 0 ; BlockCount < TotalBlockCount; BlockCount++) {
    TotalBlockSize += (UINT32)mScIpBlocks[BlockCount].Size;
    DEBUG ((DEBUG_INFO, "TotalBlockSize after adding Block[0x%x]= 0x%x\n", BlockCount, TotalBlockSize));
  }
  ConfigBlockHdrSize = GetSizeOfConfigBlockTableHeaders ((UINT16)TotalBlockCount);
  //
  // Because CreateConfigBlockTable has the padding for each config block,
  // we need extra size, which is TotalBlockCount * 3, to create the table
  //
  return ConfigBlockHdrSize + TotalBlockSize + (TotalBlockCount * 3);
}

/**
  CreateConfigBlocks generates the config blocks of SC Policy.
  It allocates and zero out buffer, and fills in the Intel default settings.

  @param[out] ScPolicyPpi        The pointer to get SC Policy PPI instance

  @retval EFI_SUCCESS            The policy default is initialized.
  @retval EFI_OUT_OF_RESOURCES   Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
ScCreateConfigBlocks (
  OUT  SC_POLICY_PPI **ScPolicyPpi
  )
{
  UINT16             TotalBlockCount;
  UINT16             BlockCount;
  VOID               *ConfigBlockPointer;
  CONFIG_BLOCK       ConfigBlockBuf;
  EFI_STATUS         Status;
  SC_POLICY_PPI      *InitPolicy;
  UINT32             RequiredSize;

  DEBUG ((DEBUG_INFO, "SC CreateConfigBlocks\n"));

  InitPolicy = NULL;
  TotalBlockCount = sizeof(mScIpBlocks) / sizeof (IP_BLOCK_ENTRY);
  DEBUG ((DEBUG_INFO, "TotalBlockCount = 0x%x\n", TotalBlockCount));

  RequiredSize = ScGetConfigBlockTotalSize ();

  Status = CreateConfigBlockTable ((VOID *)&InitPolicy, TotalBlockCount, RequiredSize);
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize Policy Revision
  //
  InitPolicy->TableHeader.Header.Revision = SC_POLICY_REVISION;
  //
  // Initialize ConfigBlockPointer to NULL
  //
  ConfigBlockPointer = NULL;
  //
  // Loop to identify each config block from mScIpBlocks[] Table and add each of them
  //
  for (BlockCount = 0 ; BlockCount < TotalBlockCount; BlockCount++) {
    CopyMem (&(ConfigBlockBuf.Header.Guid), mScIpBlocks[BlockCount].Guid, sizeof (EFI_GUID));
    ConfigBlockBuf.Header.Size     = mScIpBlocks[BlockCount].Size;
    ConfigBlockBuf.Header.Revision = mScIpBlocks[BlockCount].Revision;
    ConfigBlockPointer             = (VOID *)&ConfigBlockBuf;
    Status = AddConfigBlock ((VOID *)InitPolicy, (VOID *)&ConfigBlockPointer);
    ASSERT_EFI_ERROR (Status);
    mScIpBlocks[BlockCount].LoadDefault (ConfigBlockPointer);
  }
  //
  // Assignment for returning Policy config block base address
  //
  *ScPolicyPpi = InitPolicy;
  return EFI_SUCCESS;
}

/**
  ScInstallPolicyPpi installs ScPolicyPpi.
  While installed, RC assumes the Policy is ready and finalized. So please update and override
  any setting before calling this function.

  @param[in] ScPolicyPpi         The pointer to SC Policy PPI instance

  @retval EFI_SUCCESS            The policy is installed.
  @retval EFI_OUT_OF_RESOURCES   Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
ScInstallPolicyPpi (
  IN  SC_POLICY_PPI *ScPolicyPpi
  )
{
  EFI_STATUS              Status;
  EFI_PEI_PPI_DESCRIPTOR  *ScPolicyPpiDesc;

  ScPolicyPpiDesc = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
  if (ScPolicyPpiDesc == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }

  ScPolicyPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  ScPolicyPpiDesc->Guid  = &gScPolicyPpiGuid;
  ScPolicyPpiDesc->Ppi   = ScPolicyPpi;

  //
  // Print whole SC_POLICY_PPI and serial out.
  //
  ScPrintPolicyPpi (ScPolicyPpi);

  //
  // Install SC Policy PPI
  //
  Status = PeiServicesInstallPpi (ScPolicyPpiDesc);
  ASSERT_EFI_ERROR (Status);
  return Status;
}

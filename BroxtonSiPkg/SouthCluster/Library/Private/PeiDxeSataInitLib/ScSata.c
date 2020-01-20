/** @file
  The SC SATA Init in PEIM phase

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

#include  "ScSataInit.h"
#include <Library/SideBandLib.h>


/**
  Disable Sata Controller

  @param[in] AhciBarAddress       The Base Address for AHCI BAR
  @param[out] FuncDisable1Reg     To be used to update function disable register

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
DisableScSataController (
  IN  UINT32     AhciBarAddress,
  OUT UINT32     *FuncDisable1Reg
  )
{
  UINTN  PciSataRegBase;
  UINT32 AhciBar;
  UINT32 Data32And;
  UINT32 Data32Or;
  UINT8  Data8;

  PciSataRegBase  = MmPciAddress (0, PCI_BUS_NUMBER_SATA, PCI_DEVICE_NUMBER_SATA, PCI_FUNCTION_NUMBER_SATA, 0);

  DEBUG ((DEBUG_INFO, "DisableScSataController: DisableScSataController() Started\n"));

  ///
  /// Set the AHCI BAR
  ///
  AhciBar = AhciBarAddress;
  MmioWrite32 (PciSataRegBase + R_SATA_AHCI_BAR, AhciBar);

  ///
  /// Enable command register memory space decoding
  ///
  MmioOr16 (PciSataRegBase + PCI_COMMAND_OFFSET, (UINT16) EFI_PCI_COMMAND_MEMORY_SPACE);

  ///
  /// SC BIOS Spec Section 14.3 SATA Controller Disabling
  /// Step 1 - Disable all ports
  /// Set SATA PCI offset 94h[7:0] to all 0b
  ///
  MmioAnd16 (PciSataRegBase + R_SATA_PCS, (UINT16) ~(B_SATA_PCS_PXE_MASK));

  ///
  /// Step 2 - Disable all ports
  /// Clear PI register, ABAR + 0Ch
  ///
  Data8 = MmioRead8 (AhciBar + R_SATA_VS_CAP);
  MmioWrite8 (AhciBar + R_SATA_VS_CAP, Data8);
  Data8 = MmioRead8 (AhciBar + R_SATA_VS_CAP + 1);
  MmioWrite8 (AhciBar + R_SATA_VS_CAP + 1, Data8);

  Data8 = (UINT8) (~B_SATA_PORT_MASK);
  MmioAnd8 (AhciBar + R_SATA_AHCI_PI, Data8);

  ///
  /// After BIOS issues initial write to this register, BIOS is requested to
  /// issue two reads to this register.
  ///
  Data32Or = MmioRead32 (AhciBar + R_SATA_AHCI_PI);
  Data32Or = MmioRead32 (AhciBar + R_SATA_AHCI_PI);

  ///
  /// Step 3
  /// Clear MSE and IOE, SATA PCI offset 4h[1:0] = 00b
  ///
  MmioAnd16 (PciSataRegBase + PCI_COMMAND_OFFSET, (UINT16) ~(EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_IO_SPACE));

  ///
  /// Step 4
  /// Set Sata Port Clock Disable bits SATA PCI offset 90h[7:0] to all 1b
  ///
  MmioOr32 (PciSataRegBase + R_SATA_MAP, (UINT32) (B_SATA_MAP_PCD));

  ///
  /// Step 5 Enable all SATA dynamic clock gating and dynamic power gating features:
  ///  a. SATA SIR offset 9Ch bit[29, 23] = [1, 1]
  ///  b. SATA SIR offset 8Ch bit[23:16, 7:0] to all 1b
  ///  c. SATA SIR offset A0h bit[15] to [1]
  ///  d. SATA SIR offset 84h bit[23:16] to all 1b
  ///  e. SATA SIR offset A4h bit[26, 25, 24, 14, 5] = [0, 0, 0, 1, 1]
  ///  f. SATA SIR offset A8h bit[17, 16] = [1, 1]
  ///
  /// Step 5a
  ///  SATA SIR offset 9Ch bit[29, 23] = [1, 1]
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_9C);
  MmioOr32 (PciSataRegBase + R_SATA_STRD, (UINT32) (BIT29 | BIT23));


  ///
  /// Step 5b
  /// SATA SIR offset 8Ch bit[23:16, 7:0] to all 1b
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_8C);
  MmioOr32 (PciSataRegBase + R_SATA_STRD, 0x00FF00FF);

  ///
  /// Step 5c
  /// SATA SIR offset 54h bit [30, 26:24, 21] = [1, 1s, 1]
  ///
  /// SATA SIR offset A0h bit[15] to [1]
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_A0);
  MmioOr32 (PciSataRegBase + R_SATA_STRD, (UINT32) (BIT15));

  ///
  /// Step 5d
  /// SATA SIR offset 84h bit[23:16] to all 1b
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_84);
  MmioOr32 (PciSataRegBase + R_SATA_STRD, 0x00FF0000);

  ///
  /// Step 5e
  /// SATA SIR offset A4h bit[26, 25, 24, 14, 5] = [0, 0, 0, 1, 1]
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_A4);
  Data32And = (UINT32)~(BIT26 | BIT25 | BIT24 | BIT14 | BIT5);
  Data32Or  = (UINT32) (BIT14 | BIT5);
  MmioAndThenOr32 (
    PciSataRegBase + R_SATA_STRD,
    Data32And,
    Data32Or
    );

  ///
  /// Step 5f
  /// SATA SIR offset A8h bit[17, 16] = [1, 1]
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_A8);
  Data32And = (UINT32)~(BIT17 | BIT16);
  Data32Or  = (UINT32) (BIT17 | BIT16);
  MmioAndThenOr32 (
    PciSataRegBase + R_SATA_STRD,
    Data32And,
    Data32Or
    );
  ///
  /// Step 6
  /// Disabling SATA Device by programming SATA SCFD, SATA PCI offset 9Ch[10] = 1
  ///
  MmioOr32 (PciSataRegBase + R_SATA_SATAGC, (UINT32) (BIT10));


  ///
  /// Step 7
  /// Set PCR[PSF1] AUD PCIEN[8] to 1
  ///
  SideBandAndThenOr32 (
    PID_PSF1,
    R_SC_PCR_PSF1_T0_SHDW_SATA_REG_BASE + R_PCH_PCR_PSFX_T0_SHDW_PCIEN,
    (UINT32)~0,
    B_PCH_PCR_PSFX_T0_SHDW_PCIEN_FUNDIS);

  ///
  /// Set Ahci Bar to zero (Note: MSE and IO has been disabled)
  ///
  AhciBar = 0;
  MmioWrite32 (PciSataRegBase + R_SATA_AHCI_BAR, AhciBar);
  *FuncDisable1Reg |= B_PMC_FUNC_DIS_1_SATA;
  DEBUG ((DEBUG_INFO, "DisableScSataController: DisableScSataController() Ended\n"));

  return EFI_SUCCESS;
}

/**
  Function to configure SATA HBA in PEI phase
  CAP register in AHCI BAR will be set based on policy
  eSATA speed limit, per port, is also handled based on policy

  @param[in] SataConfig                 The SC Policy for SATA configuration
  @param[in] PciSataRegBase             SATA Register Base, Device 31 Function 2
  @param[in] SataPortToBeEnabledBitmap  Per port settings in AHCI will be based on this
  @param[in] AhciBarAddress             Temporary Memory Base Address for PCI
                                        devices to be used to initialize MMIO
                                        registers.

  @retval None
**/
VOID
ScSataInitAhci (
  IN  CONST SC_SATA_CONFIG *SataConfig,
  IN  UINTN                PciSataRegBase,
  IN  UINT8                SataPortToBeEnabledBitmap,
  IN  UINT32               AhciBarAddress
  )
{
  UINTN                     PortIndex;
  UINT8                     PortSataControl;
  UINT8                     PortxEnabled;
  UINT32                    AhciBar;
  UINT32                    CapRegister;
  UINT32                    Data32And;
  UINT32                    Data32Or;
  UINT32                    PxCMDRegister;

  //
  // Setup the AhciBar
  //
  MmioAnd16 (PciSataRegBase + PCI_COMMAND_OFFSET, (UINT16) ~EFI_PCI_COMMAND_MEMORY_SPACE);
  AhciBar = AhciBarAddress;
  MmioWrite32 (PciSataRegBase + R_SATA_AHCI_BAR, AhciBar);
  MmioOr16 (PciSataRegBase + PCI_COMMAND_OFFSET, (UINT16) EFI_PCI_COMMAND_MEMORY_SPACE);

  PortxEnabled = B_SATA_PCS_P0E;

  //
  // Read the default value of the Host Capabilites Register
  // NOTE: many of the bits in this register are R/WO (Read/Write Once)
  //
  CapRegister = MmioRead32 (AhciBar + R_SATA_AHCI_CAP);
  //
  // CAP - HBA Capabilities Register
  // ABAR Offset:   00h - 03h
  // Value: [31, 30, 24, 14, 13, 5] all 1's
  //        [29, 19:15, 12:6, 4:0]  all RO
  //        [28:25, 23:20, 5]       set by platform requirement.
  //
  CapRegister &= (UINT32)~(B_SATA_AHCI_CAP_SIS | B_SATA_AHCI_CAP_SSS | B_SATA_AHCI_CAP_SALP | \
                           B_SATA_AHCI_CAP_SAL | B_SATA_AHCI_CAP_ISS_MASK | B_SATA_AHCI_CAP_SXS);

  CapRegister |= (UINT32) (B_SATA_AHCI_CAP_S64A | B_SATA_AHCI_CAP_SCQA | B_SATA_AHCI_CAP_SCLO | \
                           B_SATA_AHCI_CAP_SSC | B_SATA_AHCI_CAP_PSC);

  for (PortIndex = 0; PortIndex < SC_MAX_SATA_PORTS; PortIndex++) {
    //
    // Check PCS.PxE to know if the SATA Port x is disabled.
    //
    if ((SataPortToBeEnabledBitmap & (PortxEnabled << PortIndex)) == 0) {
      continue;
    }

    if (SataConfig->PortSettings[PortIndex].InterlockSw == TRUE) {
      //
      // [28] Mechanical Presence Switch is Enabled for at least one of the ports
      //
      CapRegister |= B_SATA_AHCI_CAP_SIS;
    }

    if ((SataConfig->PortSettings[PortIndex].SpinUp == TRUE) ||
        (SataConfig->PortSettings[PortIndex].External == TRUE)) {
      //
      // SC BIOS Spec Section 14.1.4 Initialize Registers in AHCI Memory-Mapped Space
      // Step 1.4
      // Set SSS (ABAR + 00h[27]) to enable SATA controller supports staggered
      // spin-up on its ports, for use in balancing power spikes
      // SATA Port Spin up is supported at least one of the ports
      // If this is an extra eSATA port. It needs to be hotpluggable but it's usually empty anyway
      // so LPM is not available but Listen Mode is available, so it will have good power management.
      // If Sata Test Mode enabled, then uncoditonally clear SSS (ABAR + 00h[27])
      // to resolve Spin-donw issue with the test equiepment
      //
      if (SataConfig->TestMode == FALSE ) {
        CapRegister |= B_SATA_AHCI_CAP_SSS;
      }
    }

    if (SataConfig->PortSettings[PortIndex].External == TRUE) {
      //
      // [5] External SATA is supported at least one of the ports
      //
      CapRegister |= B_SATA_AHCI_CAP_SXS;
    }
  }

  //
  // Set SALP (ABAR + 00h[26]) to enable Aggressive Link Power Management (LPM) support.
  //
  if (SataConfig->SalpSupport == TRUE) {
    CapRegister |= B_SATA_AHCI_CAP_SALP;
  }

  //
  // Configure for the max speed support 1.5Gb/s, 3.0Gb/s and 6.0Gb/s.
  //
  switch (SataConfig->SpeedLimit) {
  case ScSataSpeedGen1:
    CapRegister |= (V_SATA_AHCI_CAP_ISS_1_5_G << N_SATA_AHCI_CAP_ISS);
    break;

  case ScSataSpeedGen2:
    CapRegister |= (V_SATA_AHCI_CAP_ISS_3_0_G << N_SATA_AHCI_CAP_ISS);
    break;

  case ScSataSpeedGen3:
  case ScSataSpeedDefault:
    CapRegister |= (V_SATA_AHCI_CAP_ISS_6_0_G << N_SATA_AHCI_CAP_ISS);
    break;
  }

  //
  // Update the Host Capabilites Register
  // NOTE: Many of the bits in this register are R/WO (Read/Write Once)
  //
  MmioWrite32 (AhciBar + R_SATA_AHCI_CAP, CapRegister);

  ///
  /// Enable Enabled SATA ports,
  /// If BIOS accesses any of the port specific AHCI address range before setting PI bit,
  /// BIOS is required to read the PI register before the initial write to the PI register.
  /// NOTE: The read before initial write to PI register is done by the MmioAndThenOr32 routine.
  /// NOTE: many of the bits in this register are R/WO (Read/Write Once)
  ///
  MmioAndThenOr32 (AhciBar + R_SATA_AHCI_PI, (UINT32)~(B_SATA_PORT_MASK), (UINT32)SataPortToBeEnabledBitmap);

  ///
  /// After BIOS issues initial write to this register, BIOS is requested to issue two
  /// reads to this register.
  ///
  Data32Or = MmioRead32 (AhciBar + R_SATA_AHCI_PI);
  Data32Or = MmioRead32 (AhciBar + R_SATA_AHCI_PI);

  ///
  /// Program all PCS "Port X Enabled" to all 0b,
  /// then program them again to all 1b in order to trigger COMRESET
  ///
  MmioAnd8 (PciSataRegBase + R_SATA_PCS, (UINT8) ~SataPortToBeEnabledBitmap);
  MmioOr16 (PciSataRegBase + R_SATA_PCS, (UINT8) SataPortToBeEnabledBitmap);


  //
  // SATA DevSleep Enabling
  //

  //
  // CAP2 - HBA Capabilities Register
  // ABAR Offset:   24h - 27h
  // Value: [5]          1b
  //        [31:6, 1:0]  all RO
  //        [4:2]        set by platform requirement. Only read available when SATA PHY PM Disable Fuse is '1'
  //
  Data32Or = B_SATA_AHCI_CAP2_DESO;
  Data32Or |= B_SATA_AHCI_CAP2_SDS | B_SATA_AHCI_CAP2_SADM | B_SATA_AHCI_CAP2_APST;
  MmioOr32 (AhciBar + R_SATA_AHCI_CAP2, Data32Or);

  //
  // PxCMD - HBA Port Command
  // ABAR Offset:   118h + PortIndex * 80h
  //
  /// NOTE: this register must be updated after Port Implemented and Capabilities register,
  /// Many of the bits in this register are R/WO (Read/Write Once)
  ///
  for (PortIndex = 0; PortIndex < SC_MAX_SATA_PORTS; PortIndex++) {
    ///
    /// Check PCS.PxE to know if the SATA Port x is disabled.
    ///
    if ((SataPortToBeEnabledBitmap & (B_SATA_PCS_P0E << PortIndex)) == 0) {
      continue;
    }
    ///
    /// Initial to zero first
    ///
    PxCMDRegister = 0;

    if (SataConfig->PortSettings[PortIndex].HotPlug == TRUE) {
      if (SataConfig->PortSettings[PortIndex].External == FALSE) {
        ///
        /// [18] Set Hot Plug Capable Port
        ///
        PxCMDRegister |= B_SATA_AHCI_PxCMD_HPCP;
        if (SataConfig->PortSettings[PortIndex].InterlockSw == TRUE) {
          ///
          /// [19] Set Mechanical Presense Switch Attached to Port
          ///
          PxCMDRegister |= B_SATA_AHCI_PxCMD_MPSP;
        }
      }
    } else {
      ///
      /// When "Mechanical Switch Attached to Port" (PxCMD[19]) is set, it is expected that HPCP (PxCMD[18]) is also set.
      ///
      if (SataConfig->PortSettings[PortIndex].InterlockSw == TRUE) {
        DEBUG ((DEBUG_ERROR, "Hot-Plug function of Port%d should be enabled while the Inter Lock Switch of it is enabled!\n", PortIndex));
      }
    }
    ///
    /// [21] Set External SATA Port
    ///
    if (SataConfig->PortSettings[PortIndex].External == TRUE) {
      PxCMDRegister |= B_SATA_AHCI_PxCMD_ESP;
    }
    ///
    /// [1] Set Spin-Up Device
    ///
    if (SataConfig->PortSettings[PortIndex].SpinUp == TRUE || (CapRegister & B_SATA_AHCI_CAP_SSS) != 0) {
      PxCMDRegister |= B_SATA_AHCI_PxCMD_SUD;
    }

    ///
    /// [26] Set Aggressive Link Power Management Enable
    ///
    if (SataConfig->SalpSupport == TRUE) {
      PxCMDRegister |= (B_SATA_AHCI_PxCMD_ALPE);
    }

    MmioAndThenOr32 (
      AhciBar + (R_SATA_AHCI_P0CMD + (0x80 * PortIndex)),
      (UINT32)~(B_SATA_AHCI_PxCMD_MASK),
      (UINT32) PxCMDRegister
      );

    ///
    /// DevSleep programming
    ///
    if (SataConfig->PortSettings[PortIndex].DevSlp == TRUE) {
      Data32And = (UINT32)~(B_SATA_AHCI_PxDEVSLP_DITO_MASK | B_SATA_AHCI_PxDEVSLP_DM_MASK);
      Data32Or  = (B_SATA_AHCI_PxDEVSLP_DSP | V_SATA_AHCI_PxDEVSLP_DM_16 | V_SATA_AHCI_PxDEVSLP_DITO_625);

      if (SataConfig->PortSettings[PortIndex].EnableDitoConfig == TRUE) {
        Data32Or &= Data32And;
        Data32Or |= ((SataConfig->PortSettings[PortIndex].DitoVal << 15) | (SataConfig->PortSettings[PortIndex].DmVal << 25));
      }
      MmioAndThenOr32 (
        AhciBar + (R_SATA_AHCI_P0DEVSLP + (0x80 * PortIndex)),
        Data32And,
        Data32Or
        );
    } else {
      MmioAnd32 (
        AhciBar + (R_SATA_AHCI_P0DEVSLP + (0x80 * PortIndex)),
        (UINT32) ~(B_SATA_AHCI_PxDEVSLP_DSP)
        );
    }
    ///
    /// eSATA port support only up to Gen2.
    /// When enabled, BIOS will configure the PxSCTL.SPD to 2 to limit the eSATA port speed.
    /// Please be noted, this setting could be cleared by HBA reset, which might be issued
    /// by EFI AHCI driver when POST time, or by SATA inbox driver/RST driver after POST.
    /// To support the Speed Limitation when POST, the EFI AHCI driver should preserve the
    /// setting before and after initialization. For support it after POST, it's dependent on
    /// driver's behavior.
    ///
    if ((SataConfig->eSATASpeedLimit == TRUE) &&
        SataConfig->PortSettings[PortIndex].External == TRUE) {
      PortSataControl = MmioRead8 (AhciBar + (R_SATA_AHCI_P0SCTL + (0x80 * PortIndex)));
      PortSataControl &= (UINT8) ~(B_SATA_AHCI_PXSCTL_SPD);
      PortSataControl |= (UINT8) V_SATA_AHCI_PXSCTL_SPD_2;
      MmioWrite8 (AhciBar + (R_SATA_AHCI_P0SCTL + (0x80 * PortIndex)), PortSataControl);
    }
  }

  //
  // Clear AhciBar
  //
  MmioAnd16 (PciSataRegBase + PCI_COMMAND_OFFSET, (UINT16) ~EFI_PCI_COMMAND_MEMORY_SPACE);
  MmioWrite32 (PciSataRegBase + R_SATA_AHCI_BAR, 0);
}

/**
  Internal function performing SATA init needed in PEI phase.

  @param[in] ScPolicyPpi         The SC Policy instance
  @param[in] AhciBarAddress      Temporary Memory Base Address for PCI
                                 devices to be used to initialize MMIO
                                 registers.
  @param[out] FuncDisable1Reg    To be used to update function disable register

  @retval None
**/
VOID
ScSataInit (
  IN  SC_POLICY_PPI *ScPolicyPpi,
  IN  UINT32        AhciBarAddress,
  OUT UINT32        *FuncDisable1Reg
  )
{
  UINT8          SataPortToBeEnabledBitmap;
  UINT32         Data32And;
  UINT32         Data32Or;
  UINTN          PciSataRegBase;
  UINT8          SataGcReg;
  EFI_STATUS     Status;
  SC_SATA_CONFIG *SataConfig;
  UINT8          Index;
  UINT32         DisableDynamicPowerGateBitMap;

  DEBUG ((DEBUG_INFO, "ScSataInit() - Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gSataConfigGuid, (VOID *) &SataConfig);
  ASSERT_EFI_ERROR (Status);

  PciSataRegBase  = MmPciBase (
                      DEFAULT_PCI_BUS_NUMBER_SC,
                      PCI_DEVICE_NUMBER_SATA,
                      PCI_FUNCTION_NUMBER_SATA
                      );

  ///
  /// SATA can be disabled by fuse or soft strap
  /// Skip SATA initialization whenever SATA device is not available.
  ///
  if (MmioRead16 (PciSataRegBase) == 0xFFFF) {
    if (SataConfig->Enable == FALSE) {
      *FuncDisable1Reg |= B_PMC_FUNC_DIS_1_SATA;
    }
    return;
  }
  ///
  /// If Sata is disabled, perform the disabling steps to function disable the SATA Controller
  ///
  if (SataConfig->Enable == FALSE) {
    DisableScSataController (AhciBarAddress, FuncDisable1Reg);
    return;
  }

  ///
  /// Set SIR Common Test Registers
  ///
  ///
  /// CTM1 - Common Test Mode 1
  /// SIR Offset:   98h - 9Bh
  /// Value: 0x00000183
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_98);
  MmioWrite32 (PciSataRegBase + R_SATA_STRD, 0x00000183);
  ///
  /// CTM2 - Common Test Mode 2
  /// SIR Offset:   9Ch - 9Fh
  /// Value: 0x20DC0224
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_9C);
  MmioWrite32 (PciSataRegBase + R_SATA_STRD, 0x20DC0224);
  ///
  /// CTM3 - Common Test Mode 3
  /// SIR Offset:   A0h - A3h
  /// Value: 0x004C8000
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_A0);
  MmioWrite32 (PciSataRegBase + R_SATA_STRD, 0x004C8000);
  ///
  /// CTM4 - Common Test Mode 4
  /// SIR Offset:   A4h - A7h
  /// Value: 0x00194060
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_A4);
  MmioWrite32 (PciSataRegBase + R_SATA_STRD, 0x00194060);

  //
  // When platform design has one of below condition,
  // BIOS has to set SATA PHYDPGE=0 and SQOFFIDLED=1 for specific the SATA port.
  //   - SATA hot-plug enabled port (PxCMD.HPCP = 1)
  //   - SATA external port (PxCMD.ESP = 1)
  //   - SATA slimline port with zero-power ODD (ZPODD) attached (or other AN capable ODD)
  //
  // Set PHYDPGE = 0 and SQOFFIDLED = 1 for selected ports.
  //
  //   [MPHY Dynamic Power gating] SIR offset 90h[7:0], BIT0 = Port0, ..., BIT7 = Port7
  //   [Slumber Squelch off] SIR offset 80h[23:16], BIT16 = Port0, ..., BIT23 = Port7
  //
  DisableDynamicPowerGateBitMap = 0;
  for (Index = 0; Index < SC_MAX_SATA_PORTS; Index++) {
    if (SataConfig->PortSettings[Index].External ||
        SataConfig->PortSettings[Index].HotPlug  ||
        SataConfig->PortSettings[Index].DisableDynamicPg) {
      DisableDynamicPowerGateBitMap |= 1u << Index;
    }
  }

  ///
  /// Set SIR Port-based Test Mode Registers
  ///
  /// Port-based Test Mode 1 Register
  /// SIR Offset:   80h - 83h
  /// Value: 0x00000000
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_80);
  MmioWrite32 (PciSataRegBase + R_SATA_STRD, (DisableDynamicPowerGateBitMap << 16));
  ///
  /// Port-based Test Mode 2 Register
  /// SIR Offset:   84h - 87h
  /// Value: 0x00030003
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_84);
  MmioWrite32 (PciSataRegBase + R_SATA_STRD, 0x00030003);
  ///
  /// Port-based Test Mode 3 Register
  /// SIR Offset:   88h - 8Bh
  /// Value: 0x00000000
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_88);
  MmioWrite32 (PciSataRegBase + R_SATA_STRD, 0x00000000);
  ///
  /// Port-based Test Mode 4 Register
  /// SIR Offset:   8Ch - 8Fh
  /// Value: 0x00030003
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_8C);
  MmioWrite32 (PciSataRegBase + R_SATA_STRD, 0x00030003);

  ///
  /// CPPM LTR Slumber
  /// SIR Offset:   CCh - CDh
  /// Value: [15:0] = 900Ah
  ///
  //
  /// CPPM LTR Partial
  /// SIR Offset:   CEh - CFh
  /// Value: [15:0] = 883Ch
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_CC);
  MmioWrite32 ( PciSataRegBase + R_SATA_STRD, 0x883C900A);
  ///
  /// CPPM LTR Active
  /// SIR Offset:   D0h - D1h
  /// Value: [15:0] = 880Ah
  ///
  Data32And = 0xFFFF0000;
  Data32Or  = 0x0000880A;
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_D0);
  MmioAndThenOr32 ( PciSataRegBase + R_SATA_STRD, Data32And, Data32Or);
  ///
  /// CPPM SATA Modes
  /// SIR Offset:   C8h - CBh
  /// Value: [3:0] 1011b
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_C8);
  Data32And = 0xFFFFFFFB;
  Data32Or  = 0x0000000B;
  MmioAndThenOr32 ( PciSataRegBase + R_SATA_STRD, Data32And, Data32Or);
  ///
  /// OOB Detection Margin Register
  /// SIR Offset:   D4h - D7h
  /// Value: 0x24190E07
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_D4);
  MmioWrite32 (PciSataRegBase + R_SATA_STRD, 0x24190E07);
  //
  // Enable all SATA ports
  //
  SataPortToBeEnabledBitmap = (UINT8)(B_SATA_PCS_P0E | B_SATA_PCS_P1E);
  ///
  /// PCS - Port Control and Status
  /// PCI Offset:   94h - 97h
  /// Value [7:0] Port 0~7 Enabled bit as per SataPortToBeEnabledBitmap value.
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_PCS, SataPortToBeEnabledBitmap);
  ///
  /// After configuring Port and Control Status (PCS) Register Port x Enabled (PxE) bits accordingly,
  /// wait 1.4 micro second
  ///
  MicroSecondDelay (0x02);
  ///
  /// SATAGC - SATA General Configuration
  /// PCI Offset:   9Ch - 9Fh
  ///
  ///
  /// write MSS/ASSEL[4:0] prior to others bits for PSF w/a
  ///
  SataGcReg = MmioRead8 (PciSataRegBase + R_SATA_SATAGC);
  SataGcReg &= ~B_SATA_SATAGC_ASSEL;
  SataGcReg |= V_SATA_SATAGC_MSS_8K << N_SATA_SATAGC_MSS;
  //
  // Set FLR Capability Selection[5] to 1b (Note: Bits 7:0 are RWO, perform byte write here. Bit 31 later)
  //
  SataGcReg |= BIT5;
  SataGcReg = MmioWrite8 (PciSataRegBase + R_SATA_SATAGC, SataGcReg);
  //
  // Set Write Request Size Select/Max Payload Size[14:12] = 111b to 64b
  //
  MmioOr8 (PciSataRegBase + R_SATA_SATAGC + 1, (BIT6 | BIT5 | BIT4));
  //
  // Set SATA mode Select[16] to AHCI
  //
  MmioAnd8 (PciSataRegBase + R_SATA_SATAGC + 2,(UINT8) ~(BIT0));

  ///
  /// Configure the ABAR's host capability register and speed limit per port for eSATA
  ///
  ScSataInitAhci (SataConfig, PciSataRegBase, SataPortToBeEnabledBitmap, AhciBarAddress);

  ///
  /// registers should be programmed after SATA device detection
  ///

  ///
  /// CTM5 - Common Test Mode 5
  /// SIR Offset:   A8h - ABh
  /// Value: 0x000F0020
  ///
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_A8);
  MmioWrite32 (PciSataRegBase + R_SATA_STRD, 0x000F0020);
  ///
  /// Port-based Test Mode 5 Register
  /// SIR Offset:   90h - 93h
  /// Value: 0x00000003
  ///
  Data32Or = 0x03;
  Data32Or &= (UINT32) ~DisableDynamicPowerGateBitMap;
  MmioWrite8 (PciSataRegBase + R_SATA_SIRI, R_SATA_SIR_90);
  MmioWrite32 (PciSataRegBase + R_SATA_STRD, Data32Or);

#ifdef FSP_FLAG
  ///
  /// Set SATAGC Register Lock [31]
  ///
  MmioOr32 ((UINTN) (PciSataRegBase + R_SATA_SATAGC), BIT31);
#endif

  DEBUG ((DEBUG_INFO, "ScSataInit() - End\n"));
}



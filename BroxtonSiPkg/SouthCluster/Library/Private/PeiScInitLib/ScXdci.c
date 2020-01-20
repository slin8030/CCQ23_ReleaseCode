/** @file
  Initializes SC xDCI controller

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

@par Specification
**/
#include "ScInitPei.h"

/**
  Hide PCI config space of xDCI device and do any final initialization.

  @param[in] UsbConfig         The SC Policy for USB configuration
  @param[in] TempMemBaseAddr   Temporary Memory Base Address for PCI
                               devices to be used to initialize MMIO
                               registers.

  @retval EFI_SUCCESS          The function completed successfully
**/
EFI_STATUS
DisableXdci (
  IN  SC_USB_CONFIG  *UsbConfig,
  IN  UINT32         TempMemBaseAddr
  )
{
  UINTN      XdciPciMmBase;
  UINT32     XdciMmioBase;
  EFI_STATUS Status;
  UINTN      XhciPciMmBase;
  UINT32     XhciMmioBase;
  UINT32     BitMask;
  UINT32     BitValue;
  BOOLEAN    CoreInResetState;

  XdciPciMmBase = MmPciBase (
                    DEFAULT_PCI_BUS_NUMBER_SC,
                    PCI_DEVICE_NUMBER_OTG,
                    PCI_FUNCTION_NUMBER_OTG
                    );
  XhciPciMmBase = MmPciBase (
                    DEFAULT_PCI_BUS_NUMBER_SC,
                    PCI_DEVICE_NUMBER_XHCI,
                    PCI_FUNCTION_NUMBER_XHCI
                    );

  DEBUG ((DEBUG_INFO, "  DisableXdci () Start\n"));

  //--------------------------------------------------------------
  // Step 1: xHCI and xDCI BARs and memory spaces need to be set up and enabled (BAR, BAR_HIGH, MSE=1'b1)
  //--------------------------------------------------------------
  //--------------------------------------------------------------
  // Step 2: Enable xDCI power gating and clk gating.
  //--------------------------------------------------------------
  // 2.1 PMCTL Program offset 0x1D0 [5:0] = 111111b in xDCI private config space.
  // 2.3 Device PG Config, D3-Hot Enable (D3HEN)
  // Done in ConfigureXdci ()
  //--------------------------------------------------------------
  // Step 3: Enable DRD SW ID mode and set to host mode
  //--------------------------------------------------------------
  DEBUG ((DEBUG_INFO, "  Step 3: Enable DRD SW ID mode and set to host mode\n"));
  // Assign memory resources for xHCI
  XhciMmioBase = TempMemBaseAddr;
  MmioAnd16 (XhciPciMmBase + PCI_COMMAND_OFFSET, (UINT16) (~EFI_PCI_COMMAND_MEMORY_SPACE));
  MmioWrite32 (XhciPciMmBase + R_XHCI_MEM_BASE, XhciMmioBase);
  MmioOr16 (XhciPciMmBase + PCI_COMMAND_OFFSET, (UINT16) EFI_PCI_COMMAND_MEMORY_SPACE);
  // Enable DRD SW ID mode and set to host mode
  MmioOr32 (
    (XhciMmioBase + R_XHCI_MEM_DUAL_ROLE_CFG0),
    (UINT32) BIT21
    );
  MmioAnd32 (
    (XhciMmioBase + R_XHCI_MEM_DUAL_ROLE_CFG0),
    (UINT32) ~BIT20
    );
  //--------------------------------------------------------------
  // Step 4: Wait until DRD mux has changed to host-owned
  //--------------------------------------------------------------
  DEBUG ((DEBUG_INFO, "  Step 4: Wait until DRD mux has changed to host-owned\n"));
  BitMask  = (UINT32) (BIT29);
  BitValue = (UINT32) (BIT29);
  Status = ScMmioPoll32 (
             (UINTN) (XhciMmioBase + R_XHCI_MEM_DUAL_ROLE_CFG1),
             BitMask,
             BitValue,
             500
             );
  if (Status == EFI_TIMEOUT) {
    DEBUG ((DEBUG_INFO, "  Timeout while polling on xHCI BAR + R_XHCI_MEM_DUAL_ROLE_CFG1 [29] for 1b\n"));
  }
  // Clear memory resources for xHCI
  MmioAnd16 ((UINTN) (XhciPciMmBase + PCI_COMMAND_OFFSET), (UINT16) (~EFI_PCI_COMMAND_MEMORY_SPACE));
  MmioWrite32 (XhciPciMmBase + R_XHCI_MEM_BASE, 0);
  //--------------------------------------------------------------
  // Step 5: xDCI intialization for hibernation support
  //--------------------------------------------------------------
  // Assign memory resources for xDCI
  XdciMmioBase = TempMemBaseAddr;
  MmioAnd16 (XdciPciMmBase + PCI_COMMAND_OFFSET, (UINT16) (~EFI_PCI_COMMAND_MEMORY_SPACE));
  MmioWrite32 (XdciPciMmBase + R_OTG_BAR0, XdciMmioBase);
  MmioOr16 (XdciPciMmBase + PCI_COMMAND_OFFSET, (UINT16)EFI_PCI_COMMAND_MEMORY_SPACE);
  DEBUG ((DEBUG_INFO, "  Step 5: xDCI intialization for hibernation support\n"));

  CoreInResetState = ((MmioRead8 (XdciMmioBase + R_OTG_MEM_APBFC_U3PMU_CFG4) & BIT3) == BIT3);
  if (CoreInResetState == FALSE) {
    //5.1 Set GCTL.GblHibernationEn to 1, xDCI.GCTRL[1] = 1
    MmioOr8 ( XdciMmioBase + R_XDCI_GCTL,
      B_XDCI_GCTL_GHIBEREN
      );
    // 2.2 Set xDCI BAR + C200h [6] to 1b to enable the USB2 PHY suspend if no xDCI driver is not loaded.
    MmioOr8 (XdciMmioBase + R_XDCI_GUSB2PHYCFG,
      B_XDCI_GUSB2PHYCFG_SUSPHY
      );
    //5.2 8. Ensure that GUSB2PHYCFG[6] (Suspend 2.0 PHY) and GUSB3PIPECTL[17] (Suspend SS PHY) are set to '1'.
    BitMask  = B_XDCI_GUSB2PHYCFG_SUSPHY;
    BitValue = B_XDCI_GUSB2PHYCFG_SUSPHY;
    Status = ScMmioPoll32 (
               (UINTN) (XdciMmioBase + R_XDCI_GUSB2PHYCFG),
               BitMask,
               BitValue,
               50
               );
    BitMask  = B_XDCI_GUSB3PIPECTL0_SUSPENDEN;
    BitValue = B_XDCI_GUSB3PIPECTL0_SUSPENDEN;
    Status = ScMmioPoll32 (
               (UINTN) (XdciMmioBase + R_XDCI_GUSB3PIPECTL0),
               BitMask,
               BitValue,
               50
               );
    if (Status == EFI_TIMEOUT) {
      DEBUG ((DEBUG_INFO, "  Timeout while polling on xHCI BAR + USB3PIPECTL[17] for 1b\n"));
    }
    //--------------------------------------------------------------
    // Step 6: Device-Initiated Disconnect and Low Power While Disconnected
    //--------------------------------------------------------------
    // Set DCTL.RunStop [31] and DCTL.KeepConnect [19] to 0.
    MmioAnd32 (
      (XdciMmioBase + R_XDCI_DCTL),
      ~(BIT31 | BIT19)
      );
    BitMask  = (UINT32) (BIT22);
    BitValue = (UINT32) (BIT22);
    Status = ScMmioPoll32 (
               (UINTN) (XdciMmioBase + 0xC70C),
               BitMask,
               BitValue,
               50
               );
    if (Status == EFI_TIMEOUT) {
      DEBUG ((DEBUG_INFO, "Timeout while polling on xDCI BAR + 0xC70C [22] for 1b\n"));
    }
    //--------------------------------------------------------------
    // Step 7: RTD3 Entry
    //--------------------------------------------------------------
    DEBUG ((DEBUG_INFO, "  Step 7: RTD3  Entry\n"));
    // 7.1 Requrest IP power gate
    MmioOr16 (
      (XdciMmioBase + R_OTG_MEM_APBFC_U3PMU_CFG4),
      (UINT16) (BIT1 | BIT0)
      );
    // 7.2 Poll IP power state until it's gated
    BitMask  = (UINT32) (BIT11 | BIT10 | BIT9 | BIT8);
    BitValue = (UINT32) (BIT11 | BIT10 | BIT9 | BIT8);
    Status = ScMmioPoll32 (
               (UINTN) (XdciMmioBase + R_OTG_MEM_APBFC_U3PMU_CFG2),
               BitMask,
               BitValue,
               50
               );
    if (Status == EFI_TIMEOUT) {
      DEBUG ((DEBUG_INFO, "  Timeout while polling on xDCI BAR + 0x10F810 [8:11] = 4'b1111b\n"));
    }
    // 7.3 Assert the core soft reset
    //     APBFC_U3PMU_CFG4[3] = 1'b1
    MmioOr16 (
      (XdciMmioBase + R_OTG_MEM_APBFC_U3PMU_CFG4),
      (UINT16) BIT3
      );
  }
  // 7.4 Enable internal PME_EN
  //     APBFC_U3PMU_CFG5.u3_pme_en[3] = 1 and/or APBFC_U3PMU_CFG5.u3_pme_en[4] = 1
  MmioOr16 (
    (XdciMmioBase + R_OTG_MEM_APBFC_U3PMU_CFG5),
    (UINT16) (BIT4 | BIT3)
    );
  // 7.6 set PMECTRLSTATUS[8] = 1
  //         PMECTRLSTATUS[15] = 0 (note: write 1 to clear)
  MmioOr32 ((UINTN) (XdciPciMmBase + R_OTG_PMECTLSTS), BIT8);
  MmioOr32 ((UINTN) (XdciPciMmBase + R_OTG_PMECTLSTS), BIT15);
  // 7.7 PCI Bus driver  Set PowerState to D3 in PMCSR.
  //     PCI Config Space: PMECTRLSTATUS. POWERSTATE [1:0] = 2'h3
  MmioOr32 ((UINTN) (XdciPciMmBase + R_OTG_PMECTLSTS), B_OTG_PMECTLSTS_POWERSTATE);
  MmioRead32 (XdciPciMmBase + R_OTG_PMECTLSTS);

  //--------------------------------------------------------------
  // Step 8: Put xDCI into ACPI mode and configure PSF for function disable
  //--------------------------------------------------------------
  DEBUG ((DEBUG_INFO, "  Step 8: Put xDCI into ACPI mode and configure PSF for function disable\n"));
  // 8.1: Put xDCI into ACPI mode, xDCI.PCICFGCTRL.PCI_CFG_DIS = 1'b1
  SideBandAndThenOr32(
    OTG_PORT_ID,
    R_OTG_PCICFGCTR1,
    ~0u,
    B_OTG_PCICFGCTR1_PCI_CFG_DIS1
    );

  // 8.2: Configure PSF for function disable, PSF_1_AGNT_T0_SHDW_PCIEN_USBDEVICE_RS1_D21_F1.FunDis = 1'b1
  SideBandAndThenOr32(
    SB_PORTID_PSF1,
    R_SC_PCR_PSF1_T0_SHDW_USB_XDCI_REG_BASE + R_PCH_PCR_PSFX_T0_SHDW_PCIEN,
    ~0u,
    B_SC_PCR_PSF1_T1_SHDW_PCIEN_FUNDIS
    );

  //--------------------------------------------------------------
  // Step 9: Tell PMC that xDCI has been disabled
  //--------------------------------------------------------------
  DEBUG ((DEBUG_INFO, "  Step 9: Tell PMC that xDCI has been disabled\n"));
  // GCR.FUNC_DIS_0[0] = 1'b1
  // Set in ConfigureXdci () and write to FUNC_DIS register in ScInit ()

  // Clear memory resources for xDCI
  MmioAnd16 (XdciPciMmBase + PCI_COMMAND_OFFSET, (UINT16) (~EFI_PCI_COMMAND_MEMORY_SPACE));
  MmioWrite32 (XdciPciMmBase + R_OTG_BAR0, 0);

  DEBUG ((DEBUG_INFO, "  DisableXdci () End\n"));

  return EFI_SUCCESS;
}

/**
  Configure xDCI devices.

  @param[in] SiPolicy                     The Silicon Policy PPI instance
  @param[in] ScPolicy                     The SC Policy Ppi instance
  @param[in, out] FuncDisableReg          The value of Function disable register

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureXdci (
  IN  SI_POLICY_PPI  *SiPolicy,
  IN  SC_POLICY_PPI  *ScPolicy,
  IN  OUT UINT32     *FuncDisableReg
  )
{
  UINTN          XdciPciMmBase;
  UINT32         XdciMmioBase;
  SC_USB_CONFIG  *UsbConfig;
  EFI_STATUS     Status;

  DEBUG ((DEBUG_INFO, "ConfigureXdci() Start\n"));
  XdciPciMmBase = MmPciBase (
                    DEFAULT_PCI_BUS_NUMBER_SC,
                    PCI_DEVICE_NUMBER_OTG,
                    PCI_FUNCTION_NUMBER_OTG
                    );

  if (MmioRead16 (XdciPciMmBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
    DEBUG ((DEBUG_INFO, "xDCI: Pci device NOT found\n"));
    return EFI_NOT_FOUND;
  }

  Status = GetConfigBlock ((VOID *) ScPolicy, &gUsbConfigGuid, (VOID *) &UsbConfig);
  ASSERT_EFI_ERROR (Status);

  // 2.1 Enabling Trunk Clock Gating
  // Set 0x1D0 [5:0]=6'b111111
  SideBandAndThenOr32 (
    OTG_PORT_ID,
    R_OTG_PMCTL,
    ~0u,
    (BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)
    );
  // 2.3 set xDCI cfg space 0xA0[18:17] = 2'b10 to enable the D3 hot enable
  MmioOr32 (XdciPciMmBase + R_XDCI_POW_PG_CONF, B_XDCI_POW_PG_CONF_D3HEN);

  if (UsbConfig->XdciConfig.Enable != ScDisabled) {
    // Assign memory resources for xDCI
    XdciMmioBase = SiPolicy->TempMemBaseAddr;
    MmioAnd16 (XdciPciMmBase + PCI_COMMAND_OFFSET, (UINT16) (~EFI_PCI_COMMAND_MEMORY_SPACE));
    MmioWrite32 (XdciPciMmBase + R_OTG_BAR0, XdciMmioBase);
    MmioOr16 (XdciPciMmBase + PCI_COMMAND_OFFSET, (UINT16)EFI_PCI_COMMAND_MEMORY_SPACE);

    //
    // Chassis Power Gate Enable
    // For Chassis compliance, if a platform support D3 hot, BIOS is required to set register 0xA2 D3-Hot Enable (D3HEN) to 1.
    // If a platform support D0i3, BIOS is required to set or DEVIDLE Enable (DEVIDLEN) to 1.
    //
    MmioOr32 (XdciPciMmBase + R_XDCI_POW_PG_CONF,
      B_XDCI_POW_PG_CONF_D3HEN | B_XDCI_POW_PG_CONF_DEVIDLEN
      );

    ///
    /// Check if device present
    ///
    if (MmioRead16 (XdciPciMmBase + PCI_VENDOR_ID_OFFSET) != 0xFFFF) {
      ///
      /// Broxton BIOS Spec, Section 29.3 Intel OTG Snoop Programming
      /// Set IOSFCTL.NSNPDIS, offset 0x00 [7] to 1b in OTG private space
      ///
      SideBandAndThenOr32(
        OTG_PORT_ID,
        R_OTG_IOSFCTL,
        0xFFFFFFFF,
        B_OTG_IOSFCTL_NSNPDIS
      );

      ///
      /// Disable BAR1 when device is working in PCI mode.
      ///
      if (UsbConfig->XdciConfig.Enable == ScPciMode) {
        SideBandAndThenOr32(
          OTG_PORT_ID,
          R_OTG_PCICFGCTR1,
          0xFFFFFFFF,
          (B_OTG_PCICFGCTR1_B1D1)
        );
      }
      ///
      /// Set GUSB2PHYCFG_0[6] = 1'b1
      ///
      MmioOr32 (XdciMmioBase + R_XDCI_GUSB2PHYCFG, B_XDCI_GUSB2PHYCFG_SUSPHY);
    }
    // Clear memory resources for xDCI
    MmioAnd16 (XdciPciMmBase + PCI_COMMAND_OFFSET, (UINT16) (~EFI_PCI_COMMAND_MEMORY_SPACE));
    MmioWrite32 (XdciPciMmBase + R_OTG_BAR0, 0);
  } else {
    DisableXdci (UsbConfig, SiPolicy->TempMemBaseAddr);
    *FuncDisableReg |= B_PMC_FUNC_DIS_USB_XDCI;
  }

  DEBUG ((DEBUG_INFO, "ConfigureXdci() End\n"));
  return EFI_SUCCESS;
}


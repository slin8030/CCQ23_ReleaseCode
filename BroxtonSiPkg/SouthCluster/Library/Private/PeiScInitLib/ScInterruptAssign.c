/** @file
  Interrupt Assign

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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

/**
  Configures interrupt pin assignment for standard PCI device

  @param[in] Device          Device number
  @param[in] Function        Function number
  @param[in] InterruptPin    INTx

  @retval Status
**/
EFI_STATUS
StandardIntSet (
  IN UINT8   Device,
  IN UINT8   Function,
  IN UINT8   InterruptPin
  )
{
  UINTN     Address;

  Address = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, Device, Function);

  MmioWrite8 (Address + PCI_INT_PIN_OFFSET, InterruptPin);

  return EFI_SUCCESS;
}

/**
  Configures PCI Express Port interrupt pin assignment

  @param[in] Device          Device number
  @param[in] Function        Function number
  @param[in] InterruptPin    INTx

  @retval Status
**/
EFI_STATUS
PciExpressIntSet (
  IN UINT8   Device,
  IN UINT8   Function,
  IN UINT8   InterruptPin
  )
{
  UINTN     Address;
  UINT32    Data32Or;
  UINT32    Data32And;

  //
  // Set Interrupt Pin
  //
  Address = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, Device, Function) + R_PCH_PCIE_STRPFUSECFG;
  Data32And = (UINT32) ~B_PCH_PCIE_STRPFUSECFG_PXIP;
  Data32Or = (UINT32) (InterruptPin) << N_PCH_PCIE_STRPFUSECFG_PXIP;
  MmioAndThenOr32 (Address, Data32And, Data32Or);
  return EFI_SUCCESS;
}

/**
  Configures SC devices interrupts

  @param[in] SchPolicy                  The SC Policy PPI instance

  @retval EFI_SUCCESS                   Succeeds.
**/
EFI_STATUS
ScConfigureInterrupts (
  IN  SC_POLICY_PPI *ScPolicyPpi
  )
{
  EFI_STATUS           Status;
  SC_INTERRUPT_CONFIG  *InterruptConfig;
  UINTN                P2sbPciMmBase = 0;
  UINTN                XhciPciMmBase = 0;
  UINT32               Index = 0;
  UINT32               Data = 0;
  //
  // General Interrupt Requirements for BXT South Cluster Devices
  //
  // 1.) Define device sideband endpoint port, PCICFGCTRL reg offset,
  //     and IRQ and interrupt pin in directIrqTable.
  //
  // 2.) Set device IRQ in PciTree.asl (IntPin should match the value in directIrqTable).
  //
  // 3.) Set IRQ polarity and ensure value is correct in ITSS.IPC0, ITSS.IPC1, etc.
  //     register programming below.
  //
  // 4.) Add BDF with IRQ for the device in IrqInitTable in ScInit.c.
  //     This value is written to the interrupt line register (offset 0x3C) in the device's
  //     PCI CFG space.
  //
  // 5.) Write the Master Message Enable bit of ITSS to enable the interrupt transaction
  //     to IOSF
  //
  // Note: MSIs are set by OS device drivers and typically require no IAFW configuration.
  //

  DEBUG ((DEBUG_INFO, "ScConfigureInterrupts() - Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gInterruptConfigGuid, (VOID *) &InterruptConfig);
  ASSERT_EFI_ERROR (Status);

  //
  // Loop through whole interrupt table and program devices registers
  //
  for (Index = 0; Index < InterruptConfig->NumOfDevIntConfig; Index++) {
    //
    // Do not configure devices with Dxx < 16
    //
    if (InterruptConfig->DevIntConfig[Index].Device < 16) {
      continue;
    }

    switch (InterruptConfig->DevIntConfig[Index].Device) {
      case PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1:
      case PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2:
        //
        // PCI Express Port
        //
        PciExpressIntSet (
          InterruptConfig->DevIntConfig[Index].Device,
          InterruptConfig->DevIntConfig[Index].Function,
          InterruptConfig->DevIntConfig[Index].IntX
          );
        break;
      default:
        //
        //Standard PCI Devices which have Interrupt Pin register under PciCfgSpace + 3Dh
        //
        StandardIntSet (
          InterruptConfig->DevIntConfig[Index].Device,
          InterruptConfig->DevIntConfig[Index].Function,
          InterruptConfig->DevIntConfig[Index].IntX
          );
        break;
    }
  }
  P2sbPciMmBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_P2SB, PCI_FUNCTION_NUMBER_P2SB);

  XhciPciMmBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_XHCI, PCI_FUNCTION_NUMBER_XHCI);
  //
  // Writing the Master Message Enable (bit 0) of ITSS Register 0x3334
  //
  SideBandAndThenOr32 (SB_ITSS_PORT, R_ITSS_SB_MMC, 0xFFFFFFFF, BIT0);

  //
  // Ensure HPET and IOAPIC are enabled in P2SB PCI CFG space
  // 1) HPET   -> Register 0x60 bit 7
  // 2) IOAPIC -> Register 0x64 bit 8
  //
  MmioOr16 ((UINTN)MmioAddress (P2sbPciMmBase, R_PCH_P2SB_HPTC), (UINT16)(B_PCH_P2SB_HPTC_AE));
  MmioOr16 ((UINTN)MmioAddress (P2sbPciMmBase, R_PCH_P2SB_IOAC), (UINT16)(B_PCH_P2SB_IOAC_AE));

  // Set Max Writes Pending (Max Number of Writes Allowed on IOSF-SB to SBREG_BAR)
  MmioWrite32 ((UINTN)MmioAddress (P2sbPciMmBase, R_P2SB_P2SBC), (UINT32)0x07);

  // Set MemSpaceEn and BusInitiate in P2SB Command Register
  MmioWrite32 ((UINTN)MmioAddress (P2sbPciMmBase, PCI_COMMAND_OFFSET), (UINT32)EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER);

  //
  // Set PCICFGCTRL register in corresponding SB port for each device in directIrqTable
  //
  for (Index = 0; Index < InterruptConfig->NumOfDirectIrqTable; Index++) {
    SideBandWrite32 (
      InterruptConfig->DirectIrqTable[Index].Port,
      InterruptConfig->DirectIrqTable[Index].PciCfgOffset,
      (InterruptConfig->DirectIrqTable[Index].PciIrqNumber << N_PCICFGCTRL_PCI_IRQ) +
      (InterruptConfig->DirectIrqTable[Index].IrqPin << N_PCICFGCTRL_INT_PIN)
      );
  }

  //
  // Set xHCI Interrupt Pin
  // Inaccessible after XHCC1.ACCTRL is set (lock down bit)
  //
  MmioWrite8 ((UINTN)(XhciPciMmBase + R_XHCI_INT_PN), (UINT8)V_XHCI_INT_PN);

  //
  // Set ACPIIN / ACPIIE For HD Audio Device
  //
  DEBUG((EFI_D_INFO, "Set ACPIIN / ACPIIE For HD Audio Device.\n"));
  SideBandAndThenOr32 (
    SB_AUDIO_PORT,
    R_HDA_PCICFGCTL,
    0xFFFFFFFF,
    (UINT32)((V_HDA_INTLN << 8) | B_HDA_PCICFGCTL_SPCBAD | B_HDA_PCICFGCTL_ACPIIE)
    );
  //
  // Program Legacy Interrupt Routing Registers
  // ITSS private registers are only accessible via DWORD aligned IOSF-SB CrWr messages
  //
//[-start-160704-IB07400752-modify]//
#ifndef LEGACY_IRQ_SUPPORT
  Data = 
    (((InterruptConfig->PxRcRouting[0]) << 0) +  // R_ITSS_SB_PARC    PIRQA->IRQx Routing Control
    ((InterruptConfig->PxRcRouting[1]) << 8) +   // R_ITSS_SB_PBRC    PIRQB->IRQx Routing Control
    ((InterruptConfig->PxRcRouting[2]) << 16) +  // R_ITSS_SB_PCRC    PIRQC->IRQx Routing Control
    ((InterruptConfig->PxRcRouting[3]) << 24));  // R_ITSS_SB_PDRC    PIRQD->IRQx Routing Control
  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PARC, Data);

  Data =
    (((InterruptConfig->PxRcRouting[4]) << 0) +  // R_ITSS_SB_PERC    PIRQE->IRQx Routing Control
    ((InterruptConfig->PxRcRouting[5]) << 8) +   // R_ITSS_SB_PFRC    PIRQF->IRQx Routing Control
    ((InterruptConfig->PxRcRouting[6]) << 16) +  // R_ITSS_SB_PGRC    PIRQG->IRQx Routing Control
    ((InterruptConfig->PxRcRouting[7]) << 24));  // R_ITSS_SB_PHRC    PIRQH->IRQx Routing Control
  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PERC, Data);
#endif
//[-end-160704-IB07400752-modify]//

  //
  //  PIR0[15:00] Dev 0x00: NPK        -> PIRQA
  //  PIR0[31:16] Dev 0x0C: GMM        -> PIRQH
  //  PIR2[15:00] Dev 0x02: IGD        -> PIRQD
  //  PIR2[31:16] Dev 0x03: IUNIT      -> PIRQF
  //  PIR4[15:00] Dev 0x0E: Audio      -> PIRQA
  //  PIR4[31:16] Dev 0x0F: CSE        -> PIRQE
  //  PIR6[15:00] Dev 0x13: PCIe       -> PIRQG, PIRQH, PIRQE, PIRQF
  //  PIR6[31:16] Dev 0x14: PCIe       -> PIRQG, PIRQH, PIRQE, PIRQF
  //  PIR8[15:00] Dev 0x15: XHCI(xDCI) -> PIRQB, PIRQC
  //  PIR8[31:16] Dev 0x12: SATA       -> PIRQD
  // PIR10[15:00] Dev 0x1F: LPC/SmBus  -> PIRQE
  //
  Data =
    (((V_ITSS_SB_IAR_PIRQA | V_ITSS_SB_IBR_PIRQA | V_ITSS_SB_ICR_PIRQA | V_ITSS_SB_IDR_PIRQA) << 0) +  //  R_ITSS_SB_PIR0 Pin->PIRQx Routing for NPK
    ((V_ITSS_SB_IAR_PIRQH | V_ITSS_SB_IBR_PIRQH | V_ITSS_SB_ICR_PIRQH | V_ITSS_SB_IDR_PIRQH) << 16))   //  R_ITSS_SB_PIR1 Pin->PIRQx Routing for GMM
    ;
  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PIR0, Data);

  Data =
    (((V_ITSS_SB_IAR_PIRQD | V_ITSS_SB_IBR_PIRQD | V_ITSS_SB_ICR_PIRQD | V_ITSS_SB_IDR_PIRQD) << 0) +  //  R_ITSS_SB_PIR2 Pin->PIRQx Routing for Display
    ((V_ITSS_SB_IAR_PIRQF | V_ITSS_SB_IBR_PIRQF | V_ITSS_SB_ICR_PIRQF | V_ITSS_SB_IDR_PIRQF) << 16))   //  R_ITSS_SB_PIR3 Pin->PIRQx Routing for IUNIT
    ;
  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PIR2, Data);

  Data =
    (((V_ITSS_SB_IAR_PIRQA | V_ITSS_SB_IBR_PIRQA | V_ITSS_SB_ICR_PIRQA | V_ITSS_SB_IDR_PIRQA) << 0) +  //  R_ITSS_SB_PIR4 Pin->PIRQx Routing for Audio
    ((V_ITSS_SB_IAR_PIRQE | V_ITSS_SB_IBR_PIRQE | V_ITSS_SB_ICR_PIRQE | V_ITSS_SB_IDR_PIRQE) << 16))   //  R_ITSS_SB_PIR5 Pin->PIRQx Routing for CSE
    ;
  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PIR4, Data);

  Data =
    // Adjust PCI Interrupt Routing configuration of PCIe in ITSS PIR6 & ACPI _PRT for PCI Express Graphic Card
    (((V_ITSS_SB_IAR_PIRQG | V_ITSS_SB_IBR_PIRQH | V_ITSS_SB_ICR_PIRQE | V_ITSS_SB_IDR_PIRQF) << 0) +  //  R_ITSS_SB_PIR6 Pin->PIRQx Routing for PCIE (Device 19/20)
    ((V_ITSS_SB_IAR_PIRQB | V_ITSS_SB_IBR_PIRQB | V_ITSS_SB_ICR_PIRQB | V_ITSS_SB_IDR_PIRQB) << 16))   //  R_ITSS_SB_PIR7 Pin->PIRQx Routing for xHCI
    ;
  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PIR6, Data);

  Data =
    (((V_ITSS_SB_IAR_PIRQC | V_ITSS_SB_IBR_PIRQC | V_ITSS_SB_ICR_PIRQC | V_ITSS_SB_IDR_PIRQC) << 0) +  //  R_ITSS_SB_PIR8 Pin->PIRQx Routing for xDCI
    ((V_ITSS_SB_IAR_PIRQD | V_ITSS_SB_IBR_PIRQD | V_ITSS_SB_ICR_PIRQD | V_ITSS_SB_IDR_PIRQD) << 16))   //  R_ITSS_SB_PIR9 Pin->PIRQx Routing for SATA
    ;
  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PIR8, Data);

  Data =
    (((V_ITSS_SB_IAR_PIRQE | V_ITSS_SB_IBR_PIRQE | V_ITSS_SB_ICR_PIRQE | V_ITSS_SB_IDR_PIRQE) << 0) +  //  R_ITSS_SB_PIR10 Pin->PIRQx Routing for SMBus
    (0))                                                                                               //  R_ITSS_SB_PIR11 Pin->PIRQx Routing Reserved
    ;
  SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PIR10, Data);

  //
  // Set IRQ Interrupt Polarity Control
  //
  for (Index = 0; Index < SC_NUM_IPC_REG; Index++) {
    SideBandWrite32 (SB_ITSS_PORT, (UINT16) (R_ITSS_SB_IPC0 + 4 * Index), InterruptConfig->IPC[Index]);
  }

  DEBUG ((DEBUG_INFO, "ScConfigureInterrupts() - End\n"));
  return EFI_SUCCESS;
}



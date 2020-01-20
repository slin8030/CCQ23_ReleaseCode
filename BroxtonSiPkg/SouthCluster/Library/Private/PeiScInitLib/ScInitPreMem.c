/** @file
  The SC Init PEIM implements the PCH PEI Init PPI.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2004 - 2018 Intel Corporation.

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
#ifdef FSP_FLAG
#include <FspmUpd.h>
#include <Library/FspCommonLib.h>
#endif

/**
  Build Memory Mapped IO Resource which is used to build E820 Table in LegacyBios,
  the resource range should be preserved in ACPI as well.

  @param[in]  None

  @retval EFI_SUCCESS             The function completed successfully.
**/
EFI_STATUS
ScPreservedMmioResource (
  VOID
  )
{
  //
  // This function builds a HOB that describes a chunk of system memory.
  //
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    SC_PRESERVED_BASE_ADDRESS,
    SC_PRESERVED_MMIO_SIZE
    );
  //
  // This function builds a HOB for the memory allocation.
  //
  BuildMemoryAllocationHob (
    SC_PRESERVED_BASE_ADDRESS,
    SC_PRESERVED_MMIO_SIZE,
    EFI_RESOURCE_MEMORY_MAPPED_IO
    );

  return EFI_SUCCESS;
}

/**
  Function disable controller under PSF1
**/
VOID
PsfT1FunctionDisable (
  IN  SC_SBI_PID  Pid,
  IN  UINT16      ShdwRegBase
  )
{
  PchPcrAndThenOr32 (
    Pid,
    ShdwRegBase + R_SC_PCR_PSF1_T1_SHDW_PCIEN,
    ~0u,
    B_SC_PCR_PSF1_T1_SHDW_PCIEN_FUNDIS
    );
}

/**
  This function performs PCIe power sequence to bring up the PCIe related device and initiate link training.

  @param[in] ScPreMemPolicyPpi               The SC PREMEM Policy PPI instance

  @retval  None
**/
VOID
ScConfigurePciePowerSequence (
  IN  SC_PREMEM_POLICY_PPI *ScPreMemPolicyPpi
  )
{
  SC_PCIE_PREMEM_CONFIG *PciePreMemConfig;
  UINT8                 MaxPciePortNum;
  UINT8                 RpIndex;
  UINTN                 RpBase;
  UINTN                 RpDev;
  UINTN                 RpFunc;
  EFI_STATUS            Status;
  UINTN                 PcieRp0StrpFuseCfg;
  UINTN                 PcieRp2StrpFuseCfg;
  UINT32                Data32;
  BXT_SERIES            BxtSeries;
  UINT8                 Index;
  UINT32                PerstDelayTime;
  UINT32                PadOffset;

  PostCode (0xB10); // PEI_FRC_SC_CONFIG_PCIE_POWER_SEQ
  DEBUG ((DEBUG_INFO, "ScConfigurePciePowerSequence ()\n"));
  ///
  ///  @todo: to control PFET and PERST#
  ///
  /// The suggested flow for BIOS is as follows:
  /// 1. BIOS must read the GPIO configuration from the SMIP to determine which GPIOs are mapped to which PCIe functionality,
  ///    PERST# and PFET. This is required to know which GPIOs BIOS will be writing to for the following flow.
  /// 2. As early in BIOS code as possible (well before enumeration), turn on power to the external devices through the platform
  ///    selected PFET GPIOs. This is due to a PCIe Spec defined 100ms delay that must be met before de-asserting PERST# to the device.
  ///    The earlier in BIOS it can be enabled, the more of that 100ms time can be hidden behind other BIOS activities.
  /// 3. 100ms after PFET enabled, BIOS can de-assert PERST# to the device to bring the device out of reset.
  ///    a. Note: REFCLK must be valid/stable at least 100us prior to PERST# de-assertion. This is guaranteed by design,
  ///       as internal PLL is up and RCOMP for REFCLK IO is complete well before BIOS brings device out of reset through PERST#.
  /// 4. Immediately after PERST# de-assertion, BIOS must program BLKDQDA *and* BLKPLLEN to '0' to allow the link to train.
  ///    a. BLKDQDA holds link in Detect as discussed above.
  ///    b. BLKPLLEN allows the controller to request the PLL from the MODPHY.

  Status = GetConfigBlock ((VOID *) ScPreMemPolicyPpi, &gPcieRpPreMemConfigGuid, (VOID *) &PciePreMemConfig);
  ASSERT_EFI_ERROR (Status);

  BxtSeries       = GetBxtSeries ();
  MaxPciePortNum  = (UINT8) GetScMaxPciePortNum ();

  //
  // Set GPIO RX/TX enable to Pad Mode for native PCIE_CLKREQ#
  //
  for (Index = 0; Index < MaxPciePortNum; Index++) {
    PadOffset = PciePreMemConfig->RootPort[Index].Clock;
    if (PadOffset != 0) {
      GpioPadWrite(PadOffset, GpioPadRead(PadOffset) & ~(BIT21 | BIT22));
    }
  }

  if(PciePreMemConfig->SkipPciePowerSequence == 0) {
    //
    // 100ms delay for deassert PERST# from start ticker to current ticker
    //
    if (PciePreMemConfig->StartTimerTickerOfPfetAssert == 0) {
      PerstDelayTime = STALL_100_MILLI_SECOND;
    } else {
      PerstDelayTime = STALL_100_MILLI_SECOND - (UINT32) DivU64x32 (GetTimeInNanoSecond (GetPerformanceCounter()) - GetTimeInNanoSecond ((UINT64) PciePreMemConfig->StartTimerTickerOfPfetAssert), 1000000);
    }
    DEBUG ((DEBUG_INFO, "PerstDelayTime = %d ms\n", PerstDelayTime));
    if (PerstDelayTime <= 100) {
      MicroSecondDelay (PerstDelayTime * STALL_ONE_MILLI_SECOND);
    }

    //
    // deassert PERST#
    //
    for (Index = 0; Index < MaxPciePortNum; Index++) {
      if (PciePreMemConfig->RootPort[Index].Perst != 0) {
        GpioSetOutputValue (PciePreMemConfig->RootPort[Index].Perst, 1);
      }
    }
  }
#if (TABLET_PF_ENABLE == 1)
    // For BXT E0:
    // De-assert to '1' - Used on both reset exit and RTD3 exit to release the device from reset and initiate link training
    // 1. Set term = 0xC This will configure the internal termination to be 20k PU
    // 2. Set gpiotxdis = 1  This will disabled the tx, which will allow the internal pull-up to pull the IO high to 1 to de-assert PERST# to the device.
    // WiFi
    Data32 = GpioPadRead (S_GPIO_191);
    Data32 &= 0xFFFFFCFF;
    Data32 |= 0x100;
    Status = GpioPadWrite (S_GPIO_191, Data32);
#endif

  //
  // Get PCIE Dev 20 PCI register from SBI in case it's disabled.
  //
//[-start-180917-IB07401020-add]//
  MicroSecondDelay (100 * STALL_ONE_MILLI_SECOND);
//[-end-180917-IB07401020-add]//
  Status = PchSbiRpPciRead32 (
             PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1,
             PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_1,
             R_PCH_PCIE_STRPFUSECFG,
             &Data32
             );
  if (Status == EFI_SUCCESS) {
    PcieRp0StrpFuseCfg = (Data32 & B_PCH_PCIE_STRPFUSECFG_RPC) >> N_PCH_PCIE_STRPFUSECFG_RPC;
    switch (PcieRp0StrpFuseCfg) {
      case 1:
        //
        // Disable RP 1 when it's set to 1 x2
        //
        DEBUG ((DEBUG_INFO, "Disabling PCIE RP 2\n"));
        if (BxtSeries == BxtP) {
          PsfT1FunctionDisable (PID_PSF1, R_SC_BXTP_PCR_PSF1_T1_SHDW_PCIE02_REG_BASE);
        } else {
          PsfT1FunctionDisable (PID_PSF1, R_SC_BXT_PCR_PSF1_T1_SHDW_PCIE02_REG_BASE);
        }
        break;
      case 0:
      default:
        break;
    }
  }

  if (BxtSeries == BxtP) {
    //
    // Get PCIE Dev 19 PCI register from SBI in case it's disabled.
    //
    Status = PchSbiRpPciRead32 (
               PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2,
               2,
               R_PCH_PCIE_STRPFUSECFG,
               &Data32
               );
    if (Status == EFI_SUCCESS) {
      PcieRp2StrpFuseCfg = (Data32 & B_PCH_PCIE_STRPFUSECFG_RPC) >> N_PCH_PCIE_STRPFUSECFG_RPC;
      switch (PcieRp2StrpFuseCfg) {
        case 3:
          //
          // Disable RP 3/4/5 when it's set to 1 x4
          //
          DEBUG ((DEBUG_INFO, "Disabling PCIE RP 3/4/5\n"));
          PsfT1FunctionDisable (PID_PSF1, R_SC_PCR_PSF1_T1_SHDW_PCIE04_REG_BASE);
          PsfT1FunctionDisable (PID_PSF1, R_SC_PCR_PSF1_T1_SHDW_PCIE05_REG_BASE);
          PsfT1FunctionDisable (PID_PSF1, R_SC_PCR_PSF1_T1_SHDW_PCIE06_REG_BASE);
          break;
        case 2:
          //
          // Disable RP 3/5 when it's set to 2 x2
          //
          DEBUG ((DEBUG_INFO, "Disabling PCIE RP 3/5\n"));
          PsfT1FunctionDisable (PID_PSF1, R_SC_PCR_PSF1_T1_SHDW_PCIE04_REG_BASE);
          PsfT1FunctionDisable (PID_PSF1, R_SC_PCR_PSF1_T1_SHDW_PCIE06_REG_BASE);
          break;
       case 1:
          //
          // Disable RP 3 when it's set to 1 x2
          //
          DEBUG ((DEBUG_INFO, "Disabling PCIE RP 3\n"));
          PsfT1FunctionDisable (PID_PSF1, R_SC_PCR_PSF1_T1_SHDW_PCIE04_REG_BASE);
          break;
        case 0:
        default:
          break;
      }
    }
  }

  for (RpIndex = 0; RpIndex < MaxPciePortNum; RpIndex++) {
    Status = GetScPcieRpDevFun (RpIndex, &RpDev, &RpFunc);
    ASSERT_EFI_ERROR (Status);

    RpBase = MmPciBase (
               DEFAULT_PCI_BUS_NUMBER_SC,
               (UINT32) RpDev,
               (UINT32) RpFunc
               );
    if (MmioRead16 (RpBase) != 0xFFFF) {
      MmioAnd32 (
        RpBase + R_PCH_PCIE_PCIEALC,
        (UINT32)~(B_PCH_PCIE_PCIEALC_BLKDQDA)
        );
      MmioAnd32 (
        RpBase + R_PCH_PCIE_F4,
        (UINT32)~(B_PCH_PCIE_PHYCTL2_BLKPLLEN)
        );
    }
  }
}

/**
  Internal function performing Smbus init needed in very early PEI phase

  @param[in]  None

  @retval  None
**/
VOID
SmbusInit(
  VOID
  )
{
  UINTN  SmbusBaseAddress;
  UINT16 SmbusIoBase;
#ifdef FSP_FLAG
  FSPM_UPD *FspmUpd;
#endif

  DEBUG ((DEBUG_INFO, "ScSmbusInit() Start\n"));

  SmbusBaseAddress = MmPciBase (
                       DEFAULT_PCI_BUS_NUMBER_SC,
                       PCI_DEVICE_NUMBER_SMBUS,
                       PCI_FUNCTION_NUMBER_SMBUS
                       );
  SmbusIoBase = PcdGet16 (PcdSmbusBaseAddress);
  ///
  /// Since PEI has no PCI enumerator, set the BAR & I/O space enable ourselves
  ///
  MmioAndThenOr32 (SmbusBaseAddress + R_SMBUS_BASE, B_SMBUS_BASE_BAR, SmbusIoBase);

  MmioOr8 (SmbusBaseAddress + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_IO_SPACE);

  ///
  /// PCH BIOS Spec section 5.16 Security Recommendations - Set SPD write disable bit, Smbus PCI offset 40h[4] = 1b
  ///
#ifdef FSP_FLAG
  FspmUpd = (FSPM_UPD *)GetFspMemoryInitUpdDataPointer ();
  if (!FspmUpd->FspmConfig.SpdWriteEnable)
    MmioOr8 (SmbusBaseAddress + R_SMBUS_HOSTC, B_SMBUS_HOSTC_SPD_WD);
#else
  MmioOr8 (SmbusBaseAddress + R_SMBUS_HOSTC, B_SMBUS_HOSTC_SPD_WD);
#endif

  ///
  /// Reset the SMBus host controller
  ///
  MmioOr8 (SmbusBaseAddress + R_SMBUS_HOSTC, B_SMBUS_HOSTC_SSRESET);

  ///
  /// Enable the SMBus host controller
  ///
  MmioAndThenOr8 (
          SmbusBaseAddress + R_SMBUS_HOSTC,
          (UINT8) (~(B_SMBUS_HOSTC_SMI_EN | B_SMBUS_HOSTC_I2C_EN)),
          B_SMBUS_HOSTC_HST_EN
          );

  ///
  /// Clear Status Register before anyone uses the interfaces
  ///
  IoWrite8 (SmbusIoBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_HSTS_ALL);

  DEBUG ((DEBUG_INFO, "ScSmbusInit() End\n"));
}

/**
  Internal function performing miscellaneous init needed in very early PEI phase

  @param[in]  None

  @retval  None
**/
VOID
ScEarlyInit (
  VOID
  )
{
  DEBUG ((DEBUG_INFO, "ScEarlyInit() - Start\n"));
  if (GetBxtSeries () == BxtP) {
    SmbusInit ();
    ConfigureLpcOnEarlyPei ();
  }
  //
  // Enable the upper 128-byte bank of RTC RAM.
  // PCR [RTC] + 0x3400 [2] = 1
  //
  SideBandAndThenOr32 (
    PID_RTC,
    R_PCH_PCR_RTC_CONF,
    ~0u,
    B_PCH_PCR_RTC_CONF_UCMOS_EN
    );

  // Ensure UART2 is Unhidden
  SideBandAndThenOr32 (
    SB_PORTID_PSF3,
    R_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2,
    ~(UINT32)(B_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2_CFGDIS),
    0
    );

  DEBUG ((DEBUG_INFO, "ScEarlyInit() - End\n"));

  return;
}

/**
  This function performs SC initialization stage after memory is available.
  Only the feature must be executed right after memory installed should be done here.

  @param[in] PeiServices   Pointer to PEI Services Table.
  @param[in] NotifyDesc    Pointer to the descriptor for the Notification event that
                           caused this function to execute.
  @param[in] Ppi           Pointer to the PPI data associated with this function.

  @retval EFI_SUCCESS      Always returns EFI_SUCCESS
**/
EFI_STATUS
ScOnMemoryInstalled (
  IN      EFI_PEI_SERVICES          **PeiServices,
  IN      EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
  IN      VOID                      *Ppi
  )
{
  EFI_STATUS       Status;
  SC_POLICY_PPI    *ScPolicy;
  SI_POLICY_PPI    *SiPolicy;

  DEBUG ((DEBUG_INFO, "ScOnMemoryInstalled () - Start\n"));

  ///
  /// Get RC Policy settings through the SiPolicy PPI
  ///
  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **)&SiPolicy
             );
  if (Status != EFI_SUCCESS) {
    //
    // SI_POLICY_PPI must be installed at this point
    //
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  Status = PeiServicesLocatePpi (
             &gScPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &ScPolicy
             );
  if (Status != EFI_SUCCESS) {
    //
    // SC_POLICY_PPI must be installed at this point
    //
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  DEBUG ((DEBUG_INFO, "ScOnMemoryInstalled () - End\n"));
  return EFI_SUCCESS;
}

/**
  Performing Sc early init before ScPolicy PPI produced

  @retval EFI_SUCCESS             The function completes successfully
  @retval EFI_OUT_OF_RESOURCES    Insufficient resources to create database
**/
EFI_STATUS
EFIAPI
ScInitPreMem (
  VOID
  )
{
  UINTN  P2sbBase;
  UINT32 PcrBase;

  PostCode (0xB00); // SC PreMem Entry // PEI_FRC_SC_PRE_MEM_ENTRY
  DEBUG ((DEBUG_INFO, "ScInitPreMem() - Start\n"));
  ///
  /// Check if SBREG has been set.
  /// If not, program it.
  ///
  P2sbBase = MmPciBase (
               DEFAULT_PCI_BUS_NUMBER_SC,
               PCI_DEVICE_NUMBER_P2SB,
               PCI_FUNCTION_NUMBER_P2SB
               );
  PcrBase  = MmioRead32 (P2sbBase + R_PCH_P2SB_SBREG_BAR);
  if ((PcrBase & B_PCH_P2SB_SBREG_RBA) == 0) {
    DEBUG ((DEBUG_INFO, "SBREG should be programmed before here\n"));
    //
    // Set SBREG base address.
    //
    MmioWrite32 (P2sbBase + R_PCH_P2SB_SBREG_BAR, SC_PCR_BASE_ADDRESS);
    //
    // Enable the MSE bit for MMIO decode.
    //
    MmioOr8 (P2sbBase + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE);
  }

  ///
  /// Perform miscellaneous init needed in very early PEI phase
  ///
  PostCode (0xB02); // PEI_FRC_SC_EARLY_INIT
  ScEarlyInit ();

  PostCode (0xB7F);  // SC PreMem Exit // PEI_FRC_SC_PRE_MEM_EXIT
  DEBUG ((DEBUG_INFO, "ScInitPreMem() - End\n"));

  return EFI_SUCCESS;
}

/**
  SC init pre-memory entry point

  @param[in]  None

  @retval  None
**/
VOID
EFIAPI
ScInitPrePolicy (
  VOID
  )
{
  DEBUG ((DEBUG_INFO, "ScInitPrePolicy() - Start\n"));

  //
  // Build Memory Mapped IO Resource which is used to build E820 Table in LegacyBios.
  // the resource range should be preserved in ACPI as well.
  //
  ScPreservedMmioResource ();

  //
  // Perform SC early init
  //
  ScInitPreMem ();

  DEBUG ((DEBUG_INFO, "ScInitPrePolicy() - End\n"));
}

/**
  This function performs basic initialization for SC in PEI phase after Policy produced.
  If any of the base address arguments is zero, this function will disable the corresponding
  decoding, otherwise this function will enable the decoding.
  This function locks down the AcpiBase.

  @param[in] SiPolicyPpi   The Silicon Policy PPI instance
  @param[in] ScPolicyPpi  The PCH Policy PPI instance

  @retval  None
**/
VOID
EFIAPI
ScOnPolicyInstalled (
  IN  SI_POLICY_PPI   *SiPolicyPpi,
  IN  SC_POLICY_PPI   *ScPolicyPpi
  )
{
  DEBUG ((DEBUG_INFO, "ScOnPolicyInstalled() - Start\n"));

  DEBUG ((DEBUG_INFO, "ScOnPolicyInstalled() - End\n"));
}

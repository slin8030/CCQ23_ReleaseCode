/** @file
  This file is SampleCode of the library for Intel CPU PEI Policy initialization.

@copyright
  Copyright (c) 2009 - 2015 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by the
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor. This file may be modified by the user, subject to
  the additional terms of the license agreement.

@par Specification Reference:
**/

//[-start-151228-IB03090424-modify]//
#include <PeiCpuPolicyUpdate.h>
#if 0
#include <CmosMap.h>


extern EFI_GUID gEpcOsDataGuid;

/**
  Perform a cold reset using PCH Reset PPI
**/
VOID
EFIAPI
ResetCold (
  VOID
  )
{
  EFI_STATUS     Status;
  PCH_RESET_PPI  *PchResetPpi;
  PCH_RESET_TYPE PchResetType;

  Status = PeiServicesLocatePpi (
                  &gPchResetPpiGuid,
                  0,
                  NULL,
                  &PchResetPpi
                  );
  ASSERT_EFI_ERROR (Status);

  PchResetType = PowerCycleReset;
  PchResetPpi->Reset (PchResetPpi, PchResetType);
}



#endif
/**
  This function performs CPU PEI Policy initialization in Pre-memory.

  @param[in] SiCpuPolicyPpi        The Cpu Policy PPI instance

  @retval EFI_SUCCESS              The PPI is installed and initialized.
  @retval EFI ERRORS               The PPI is not successfully installed.
  @retval EFI_OUT_OF_RESOURCES     Do not have enough resources to initialize the driver
**/
EFI_STATUS
EFIAPI
UpdatePeiCpuPolicyPreMem (
  IN OUT  SI_CPU_POLICY_PPI *SiCpuPolicyPpi
  )
{
#if 0
  EFI_STATUS                  Status;
  TXT_CONFIG                  *TxtConfig;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariableServices;
  UINTN                       VariableSize;
  CPU_SETUP                   CpuSetupBuffer;
  CPU_SETUP                   *CpuSetup;
  EFI_BOOT_MODE               BootMode;
  UINTN                       PchSpiBase;
  UINT8                       NumSpiComponents;
  UINT32                      FlashBase;
  UINT32                      TotalFlashSize;
  UINT32                      PmConA;
  UINT32                      PmConB;
  UINTN                       PmcBaseAddress;


  SiCpuPolicyPpi->SecurityConfig = (SECURITY_CONFIG *) AllocateZeroPool (sizeof (SECURITY_CONFIG));
  if (SiCpuPolicyPpi->SecurityConfig == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }


  TxtConfig  = SiCpuPolicyPpi->TxtConfig;

  //
  // Make sure ReadOnlyVariablePpi is available
  //
  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             &VariableServices
             );
  ASSERT_EFI_ERROR (Status);

  //
  // Get Setup Variable
  //
  VariableSize = sizeof (CPU_SETUP);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               L"CpuSetup",
                               &gCpuSetupVariableGuid,
                               NULL,
                               &VariableSize,
                               &CpuSetupBuffer
                               );
  DEBUG(( DEBUG_INFO, "Get Variable status = %r\n", Status));
  ASSERT_EFI_ERROR (Status);

  CpuSetup = &CpuSetupBuffer;
  SiCpuPolicyPpi->EnableC6Dram = CpuSetup->EnableC6Dram;

  //
  // Update BIST on Reset policy
  //
  SiCpuPolicyPpi->BistOnReset = CpuSetup->BistOnReset;

  ///
  /// Function will update PrmrrSize
  /// If Runtime Memory interface is enabled and OS variable exist, the PRMRR size will be defined by OS
  /// If Runtime Memory interface is not enabled or OS asked for not supported PRMRR size, PRMRR size will be updated by
  /// PRMRR size from setup options
  /// Function will issue a cold reset if such is needed, in order to enable/disable SGX
  ///
  UpdatePrmrrSize(VariableServices, CpuSetup, SiCpuPolicyPpi);

  //
  // Update Txt Platform Policy
  //
  if (TxtConfig != NULL) {
    SiCpuPolicyPpi->Txt = CpuSetup->Txt;
    TxtConfig->TxtDprMemorySize = CpuSetup->DprSize * 0x100000;
    TxtConfig->McuUpdateDataAddr = FLASH_REGION_MICROCODE_BASE +
                                   ((EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) FLASH_REGION_MICROCODE_BASE)->HeaderLength +
                                   sizeof (EFI_FFS_FILE_HEADER);
    IoWrite8 (CMOS_ADDR_PORT, (UINT8) CMOS_TXT_REG);
    TxtConfig->TxtAcheckRequest = IoRead8 (CMOS_DATA_PORT);
    //
    // ACheck Request needs to be reset since we do not want system to run ACheck in the same boot sequence
    //
    IoWrite8 (CMOS_DATA_PORT, (UINT8) 0);
    TxtConfig->ResetAux = CpuSetup->ResetAux;
  }

  //
  // Get below 3 bits from CPT GEN_PMCON 2 & 3 for system warm boot checking
  // (1)GEN_PMCON_2 (0:31:2 offset 0A2) bit[5] RO - Memory placed in Self-Refresh (MEM_SR)
  // (2)GEN_PMCON_2 (0:31:2 offset 0A2) bit[1] RWCiV SYS_PWROK Failure (SYSPWR_FLR)
  // (3)GEN_PMCON_3 (0:31:2 offset 0A4) bit[1] RWC-(PWR_FLR)
  //
  PmcBaseAddress = MmPciBase (
                     DEFAULT_PCI_BUS_NUMBER_PCH,
                     PCI_DEVICE_NUMBER_PCH_PMC,
                     PCI_FUNCTION_NUMBER_PCH_PMC
                     );
  PmConA  = MmioRead32 (PmcBaseAddress + R_PCH_PMC_GEN_PMCON_A);
  PmConB  = MmioRead32 (PmcBaseAddress + R_PCH_PMC_GEN_PMCON_B);

  //
  // Check GEN_PMCON_B (PMC PCI offset 0A4) bit[1] - PWR FLR
  //
  if (PmConB & B_PCH_PMC_GEN_PMCON_B_PWR_FLR) {
    MmioOr32 (
      PmcBaseAddress + R_PCH_PMC_GEN_PMCON_B,
      B_PCH_PMC_GEN_PMCON_B_PWR_FLR
      );
  }

  //
  // When the CPUPWR_FLR bit is set, it indicates VRMPWRGD signal from the CPU VRM went low.
  // Software must clear this bit if set.
  //
  if (PmConB & B_PCH_PMC_GEN_PMCON_B_SUS_PWR_FLR) {
    //
    // BIOS clears this bit by writing a '1' to it.
    //
    MmioOr32 (
      PmcBaseAddress + R_PCH_PMC_GEN_PMCON_B,
      B_PCH_PMC_GEN_PMCON_B_SUS_PWR_FLR
      );
  }

  if (CpuSetup->SkipStopPbet == 1) {
    SiCpuPolicyPpi->SkipStopPbet = TRUE;
  } else {
    SiCpuPolicyPpi->SkipStopPbet = FALSE;
  }

  PchSpiBase = MmioRead32 (MmPciBase (
                            DEFAULT_PCI_BUS_NUMBER_PCH,
                            PCI_DEVICE_NUMBER_PCH_SPI,
                            PCI_FUNCTION_NUMBER_PCH_SPI)
                            + R_PCH_SPI_BAR0) & ~B_PCH_SPI_BAR0_MASK;

  Status = PeiServicesGetBootMode (&BootMode);
  SiCpuPolicyPpi->BiosGuard = CpuSetup->BiosGuard;
  if (SiCpuPolicyPpi->BiosGuard == TRUE) {
    ///
    /// Select to Flash Map 0 Register to get the number of flash Component
    ///
    MmioAndThenOr32 (
      PchSpiBase + R_PCH_SPI_FDOC,
      (UINT32) (~(B_PCH_SPI_FDOC_FDSS_MASK | B_PCH_SPI_FDOC_FDSI_MASK)),
      (UINT32) (V_PCH_SPI_FDOC_FDSS_FSDM | R_PCH_SPI_FDBAR_FLASH_MAP0)
    );
    /**
     Copy Zero based Number Of Components
     Valid Bit Settings:
       - 00 : 1 Component
       - 01 : 2 Components
       - All other settings : Reserved
    **/
    NumSpiComponents = (UINT8) ((MmioRead16 (PchSpiBase + R_PCH_SPI_FDOD) & B_PCH_SPI_FDBAR_NC) >> N_PCH_SPI_FDBAR_NC);
    ///
    /// Select to Flash Components Register to get Components Density
    ///
    MmioAndThenOr32 (
      PchSpiBase + R_PCH_SPI_FDOC,
      (UINT32) (~(B_PCH_SPI_FDOC_FDSS_MASK | B_PCH_SPI_FDOC_FDSI_MASK)),
      (UINT32) (V_PCH_SPI_FDOC_FDSS_COMP | R_PCH_SPI_FCBA_FLCOMP)
    );
    ///
    /// Calculate TotalFlashSize from Descriptor information
    ///
    FlashBase = (UINT8) MmioRead32 (PchSpiBase + R_PCH_SPI_FDOD);
    TotalFlashSize = (SPI_SIZE_BASE_512KB << ((UINT8) (FlashBase & B_PCH_SPI_FLCOMP_COMP0_MASK)));
    if (NumSpiComponents == 1) {
      TotalFlashSize += (SPI_SIZE_BASE_512KB << ((UINT8) ((FlashBase & B_PCH_SPI_FLCOMP_COMP1_MASK) >> 4)));
    }
    FlashBase = TotalFlashSize - FLASH_SIZE;

    SiCpuPolicyPpi->TotalFlashSize        = (UINT16) RShiftU64(TotalFlashSize, 10);
    SiCpuPolicyPpi->BiosSize              = (UINT16) RShiftU64(FLASH_SIZE, 10);
  }
#endif  
  return EFI_SUCCESS;
}
//[-end-151228-IB03090424-modify]//

/** @file
  The PEIM implements the SA PEI Initialization.

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

@par Specification
**/
#include <Library/PeiSaInitLib.h>
#include <SaAccess.h>
#include <Ppi/SaPeiInit.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>

#include <PiPei.h>

#include <Library/PeiServicesLib.h>

#include <Library/MmPciLib.h>
#include <Guid/SaDataHob.h>
#include <Library/PcdLib.h>
#include <Library/SideBandLib.h>
#include <Library/SteppingLib.h>
#include <Library/PreSiliconLib.h>
#include <Library/PeiSaPolicyLib.h>
#include <Library/PeiGraphicsInitLib.h>
#if (ENBDT_PF_ENABLE == 1)
#include <Library/PeiHybridGraphicsInitLib.h>
#endif
#include <Library/PeiIpuInitLib.h>
#include <Library/DisplayInitLib.h>
#include <Library/TimerLib.h>
#include <Library/PostCodeLib.h>
#include <PlatformBaseAddresses.h>
#include <Npkt.h>
#include <ScRegs/RegsPsf.h>

static EFI_PEI_PPI_DESCRIPTOR mSaPeiInitPpi[] = {
  {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gSaPeiInitPpiGuid,
  NULL
  }
};

typedef struct {
  UINT8 DeviceNumber;
  UINT8 FunctionNumber;
  UINT8 SvidRegOffset;
} SA_SVID_SID_INIT_ENTRY;

/**
  This function prints the PEI phase platform policy.

  @param[in] SaPolicyPpi - Instance of _SA_POLICY_PPI
**/
VOID
SaPeiPolicyDump (
  IN SI_SA_POLICY_PPI   *SaPolicyPpi
  )
{
  INTN                            i;
  SA_MISC_CONFIG                  *MiscConfig = NULL;
  GRAPHICS_CONFIG                 *GtConfig = NULL;
  IPU_CONFIG                      *IpuPolicy = NULL;
//[-start-160628-IB07400748-modify]//
#if (ENBDT_PF_ENABLE == 1)
#ifdef HG_ENABLE
  HYBRID_GRAPHICS_CONFIG          *HgConfig = NULL;
#endif
#endif
//[-end-160628-IB07400748-modify]//
  EFI_STATUS                      Status;

  Status = GetConfigBlock((VOID *)SaPolicyPpi, &gSaMiscConfigGuid, (VOID *)&MiscConfig);
  ASSERT_EFI_ERROR(Status);

  Status = GetConfigBlock((VOID *)SaPolicyPpi, &gGraphicsConfigGuid, (VOID *)&GtConfig);
  ASSERT_EFI_ERROR(Status);

  Status = GetConfigBlock((VOID *)SaPolicyPpi, &gIpuConfigGuid, (VOID *)&IpuPolicy);
  ASSERT_EFI_ERROR(Status);

//[-start-160628-IB07400748-modify]//
#if (ENBDT_PF_ENABLE == 1)
#ifdef HG_ENABLE
  Status = GetConfigBlock((VOID *)SaPolicyPpi, &gHybridGraphicsConfigGuid, (VOID *)&HgConfig);
  ASSERT_EFI_ERROR(Status);
#endif
#endif
//[-end-160628-IB07400748-modify]//


  DEBUG ((DEBUG_INFO, "\n------------------------ SA Platform Policy (PEI) dump BEGIN -----------------\n"));
  DEBUG ((DEBUG_INFO, " Revision             : %x\n", SaPolicyPpi->TableHeader.Header.Revision));

  DEBUG ((DEBUG_INFO, "------------------------ SA_PLATFORM_DATA -----------------\n"));
  DEBUG ((DEBUG_INFO, " IPU Enable          : %x\n", IpuPolicy->SaIpuEnable));
  DEBUG ((DEBUG_INFO, " IPU Acpi Mode       : %x\n", IpuPolicy->IpuAcpiMode));
  DEBUG ((DEBUG_INFO, " SpdAddressTable[%d]  :", MAX_SOCKETS));
  for (i = 0; i < MAX_SOCKETS; i++) {
    DEBUG((DEBUG_INFO, " %x", MiscConfig->SpdAddressTable[i]));
  }
  DEBUG ((DEBUG_INFO, "\n"));

  DEBUG ((DEBUG_INFO, "\n------------------------ GT_CONFIGURATION ---------------\n"));
  DEBUG ((DEBUG_INFO, " GttMmAdr             : 0x%x\n", GtConfig->GttMmAdr));
  DEBUG ((DEBUG_INFO, " GmAdr : 0x%x\n", GtConfig->GmAdr));
  DEBUG ((DEBUG_INFO, " CdClock : 0x%x\n", GtConfig->CdClock));
  DEBUG ((DEBUG_INFO, " PeiGraphicsPeimInit : 0x%x\n", GtConfig->PeiGraphicsPeimInit));
  DEBUG ((DEBUG_INFO, " LogoPtr : 0x%x\n", GtConfig->LogoPtr));
  DEBUG ((DEBUG_INFO, " LogoSize : 0x%x\n", GtConfig->LogoSize));
  DEBUG ((DEBUG_INFO, " GraphicsConfigPtr : 0x%x\n", GtConfig->GraphicsConfigPtr));

  DEBUG ((DEBUG_INFO, " GT PM Support             : %x\n", GtConfig->PmSupport));
  DEBUG ((DEBUG_INFO, " PavpEnable                : %x\n", GtConfig->PavpEnable));
  DEBUG ((DEBUG_INFO, " EnableRenderStandby       : %x\n", GtConfig->EnableRenderStandby));
  DEBUG ((DEBUG_INFO, " PavpPr3                   : %x\n", GtConfig->PavpPr3));

//[-start-160628-IB07400748-modify]//
#if (ENBDT_PF_ENABLE == 1)
#ifdef HG_ENABLE
  DEBUG ((DEBUG_INFO, "\n------------------------ HG_CONFIGURATION ---------------\n"));
  DEBUG ((DEBUG_INFO, "RootPortDev  = %x\n", HgConfig->RootPortDev));
  DEBUG ((DEBUG_INFO, "RootPortFun  = %x\n", HgConfig->RootPortFun));
  DEBUG ((DEBUG_INFO, "HG Enabled = %d\n", HgConfig->HgEnabled));
  DEBUG ((DEBUG_INFO, "Delay after Power Enable = %d\n", HgConfig->HgDelayAfterPwrEn));
  DEBUG ((DEBUG_INFO, "Delay after Hold Reset   = %d\n", HgConfig->HgDelayAfterHoldReset));
  if ( HgConfig->HgEnabled == 1) {
    DEBUG ((DEBUG_INFO, "Hold Reset Community Offset     = %x\n", HgConfig->HgDgpuHoldRst.CommunityOffset));
    DEBUG ((DEBUG_INFO, "Hold Reset Pin Offset           = %x\n", HgConfig->HgDgpuHoldRst.PinOffset));
    DEBUG ((DEBUG_INFO, "Hold Reset Active Info          = %d\n", HgConfig->HgDgpuHoldRst.Active));
    DEBUG ((DEBUG_INFO, "Power Enable Community Offset   = %x\n", HgConfig->HgDgpuPwrEnable.CommunityOffset));
    DEBUG ((DEBUG_INFO, "Power Enable Pin Offset         = %x\n", HgConfig->HgDgpuPwrEnable.PinOffset));
    DEBUG ((DEBUG_INFO, "Power Enable Active Info        = %d\n", HgConfig->HgDgpuPwrEnable.Active));
  }
#endif
#endif
//[-end-160628-IB07400748-modify]//

  DEBUG ((DEBUG_INFO, "\n------------------------ SA Platform Policy (PEI) dump END -----------------\n"));
  return;
}

/**
  Init and Install SA Hob

  @param[in] SaPolicyPpi - Instance of _SA_POLICY_PPI

  @return EFI_SUCCESS
**/
EFI_STATUS
InstallSaHob (
  IN SI_SA_POLICY_PPI       *SaPolicyPpi
  )
{
  EFI_STATUS                  Status;
  SA_DATA_HOB                 *SaDataHob;

//[-start-160627-IB07400748-add]//
  SaDataHob = NULL;
  SaDataHob = (SA_DATA_HOB *)GetFirstGuidHob (&gSaDataHobGuid);
  if (SaDataHob != NULL) {
    DEBUG ((EFI_D_ERROR, "SaDataHob already exist %x!!\n", SaDataHob));
    return EFI_SUCCESS;
  }
//[-end-160627-IB07400748-add]//

  //
  // Create HOB for SA Data
  //
  Status = PeiServicesCreateHob (
                             EFI_HOB_TYPE_GUID_EXTENSION,
                             sizeof (SA_DATA_HOB),
                             (VOID **) &SaDataHob
                             );
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize default HOB data
  //
  SaDataHob->EfiHobGuidType.Name = gSaDataHobGuid;
  
//[-start-160627-IB07400748-add]//
  DEBUG ((EFI_D_ERROR, "SaDataHob = %x!!\n", SaDataHob));
//[-end-160627-IB07400748-add]//

  DEBUG ((DEBUG_INFO, "SA Data HOB installed\n"));

  return EFI_SUCCESS;
}

/**
Program North Cluster devices Subsystem Vendor Identifier (SVID) and Subsystem Identifier (SID).
**/
VOID
SaProgramSvidSid(
)
{
  UINT8                         Index;
  UINT8                         BusNumber;
  UINTN                         PciEAddressBase;
  UINT8                         DeviceNumber;
  UINT8                         FunctionNumber;
  UINT8                         SvidRegOffset;
  UINT16                        Data16;
  STATIC SA_SVID_SID_INIT_ENTRY SvidSidInitTable[] = {
    {
      0,
      0,
      PCI_SVID_OFFSET
    },
    {
      0,
      2,
      PCI_SVID_OFFSET
    },
    {
      2,
      0,
      PCI_SVID_OFFSET
    },
    {
      3,
      0,
      PCI_SVID_OFFSET
    }
  };

  for (Index = 0; Index < (sizeof(SvidSidInitTable) / sizeof(SA_SVID_SID_INIT_ENTRY)); Index++) {
    BusNumber = 0;
    DeviceNumber = SvidSidInitTable[Index].DeviceNumber;
    FunctionNumber = SvidSidInitTable[Index].FunctionNumber;
    SvidRegOffset = SvidSidInitTable[Index].SvidRegOffset;
    PciEAddressBase = (UINT32)MmPciBase(BusNumber, DeviceNumber, FunctionNumber);
    ///
    /// Skip if the device is disabled
    ///
    if (MmioRead16(PciEAddressBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
      continue;
    }
    ///
    /// Program Default Subsystem Vendor Identifier (SVID)
    ///
    Data16 = V_INTEL_VENDOR_ID;
    MmioWrite16(
      (UINTN)(PciEAddressBase + SvidRegOffset),
      Data16
      );

    ///
    /// Program Default Subsystem Identifier (SID)
    ///
    Data16 = V_SA_DEFAULT_SID;
    MmioWrite16(
      (UINTN)(PciEAddressBase + SvidRegOffset + 2),
      Data16
      );
  }

  return;
}

/**
  System Agent PEI Initialization.

  @param [in] FfsHeader    - Pointer to Firmware File System file header.
  @param [in] PeiServices  - General purpose services available to every PEIM.

  @return  EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
SaInit (
  IN EFI_PEI_FILE_HANDLE       FfsHeader,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  EFI_STATUS                    Status;
  SI_SA_POLICY_PPI              *SaPolicyPpi;
  GRAPHICS_CONFIG               *GtConfig = NULL;
//[-start-160628-IB07400748-modify]//
#if (ENBDT_PF_ENABLE == 1)
#ifdef HG_ENABLE
  HYBRID_GRAPHICS_CONFIG        *HgConfig = NULL;
#endif
#endif  
//[-end-160628-IB07400748-modify]//

  DEBUG ((DEBUG_INFO, "PostMem SaInit Entry\n"));
  PostCode (0xA00); // PEI_POST_MEM_SA_INIT_ENTRY

  //
  // Get platform policy settings through the SA Platform Policy PPI
  //
  Status = (**PeiServices).LocatePpi (
                             PeiServices,
                             &gSiSaPolicyPpiGuid,
                             0,
                             NULL,
                             (VOID **) &SaPolicyPpi
                             );
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock((VOID *)SaPolicyPpi, &gGraphicsConfigGuid, (VOID *)&GtConfig);
  ASSERT_EFI_ERROR(Status);

//[-start-160628-IB07400748-modify]//
#if (ENBDT_PF_ENABLE == 1)
#ifdef HG_ENABLE
  Status = GetConfigBlock((VOID *)SaPolicyPpi, &gHybridGraphicsConfigGuid, (VOID *)&HgConfig);
  ASSERT_EFI_ERROR(Status);
#endif  
#endif
//[-end-160628-IB07400748-modify]//

  //
  // Dump SA Platform Policy
  //
  DEBUG ((DEBUG_INFO, "SaPeiPolicyDump Start\n"));
  PostCode (0xA64); // PEI_SA_PEI_POLICY_DUMP_START
  SaPeiPolicyDump (SaPolicyPpi);

  //
  // Program EC Base
  //
  DEBUG ((DEBUG_INFO, "Program EC Base Start\n"));
  PostCode (0xA0A); // PEI_PROGRAM_EC_BASE_START
  ProgramEcBase ();
  DEBUG ((DEBUG_INFO, "ProgramEcBase Done....\n"));

  //
  // Install SA HOBs
  //
  DEBUG ((DEBUG_INFO, "InstallSaHob Start\n"));
  PostCode (0xA02); // PEI_INSTALL_SA_HOBS_START
  InstallSaHob (SaPolicyPpi);

  //
  // Initialize Aunit registers
  //
  DEBUG ((DEBUG_INFO, "SaAunitInit Start\n"));
  PostCode (0xA0B); // PEI_SA_AUNIT_INIT_START
  SaAunitInit (PeiServices, SaPolicyPpi);
  DEBUG ((DEBUG_INFO, "Aunit Config Done\n"));

//[-start-160628-IB07400748-modify]//
#if (ENBDT_PF_ENABLE == 1)
#ifdef HG_ENABLE
  //
  // Initialize Hybrid Graphics
  //
  DEBUG((DEBUG_INFO, "Initializing Hybrid Graphics Start\n"));
  PostCode (0xA0C); // PEI_INIT_HYBRID_GRAPHICS_START
  HybridGraphicsInit(HgConfig);
#endif  
#endif
//[-end-160628-IB07400748-modify]//

  //
  // Initialize IPU Device (Iunit)
  //
  DEBUG ((DEBUG_INFO, "Initializing IPU device Start\n"));
  PostCode (0xA14); // PEI_INIT_IPU_DEVICE_START
  IpuInit (PeiServices, SaPolicyPpi);

  ///
  /// Program SVID SID
  ///
  DEBUG ((DEBUG_INFO, "Program SVID SID Start\n"));
  PostCode (0xA18); // PEI_PROGRAM_SVID_SID_START
  SaProgramSvidSid();

  // NPK hack
  DEBUG ((DEBUG_INFO, "NPK hack Misc BAR 0x70 to 0xFE240000\n"));
  //#define R_PCH_NPK_FW_LBAR                         0x70
  //#define B_PCH_NPK_FW_RBAL                         0xFFFC0000
  //#define R_PCH_NPK_FW_UBAR                         0x74
  //#define B_PCH_NPK_FW_RBAU                         0xFFFFFFFF
  //#define V_PCH_NPK_FW_BARL                         0xFE240000  //Range : 0xFE240000 - 0xFE27FFFF
  //#define V_PCH_NPK_FW_BARU                         0x00000000
  SideBandWrite32(SB_PORTID_PSF1, R_SC_PCR_PSF1_T0_SHDW_NPK_ACPI_REG_BASE + R_PCH_PCR_PSFX_T0_SHDW_BAR0, (UINT32)FW_BASE_ADDRESS);
  MmioWrite32( MmPciBase(0, PCI_DEVICE_NUMBER_NPK, PCI_FUNCTION_NUMBER_NPK) + R_PCH_NPK_FW_LBAR, (UINT32)FW_BASE_ADDRESS);

  //
  // Initialize PEI Display
  //
  DEBUG ((DEBUG_INFO, "Initializing Pei Display Start\n"));
  PostCode (0xA03); // PEI_INIT_PEI_DISPLAY_START
//[-start-191206-IB16530064-modify]//
  PeiDisplayInit (PeiServices, GtConfig);
//[-end-191206-IB16530064-modify]//

  //
  // PAVP Initialization
  //
  DEBUG ((DEBUG_INFO, "Initializing PAVP Start\n"));
  PostCode (0xA32); // PEI_INIT_PAVP_START
  PavpInit (GtConfig);

  //
  // Graphics PM initialization after BIOS_RESET_CPL
  //
  DEBUG ((DEBUG_INFO, "Program GraphicsPmInit Start\n"));
  PostCode (0xA63); // PEI_PROGRAM_GRAPHICS_PM_INIT_START
  Status = GraphicsPmInit (GtConfig);
  ASSERT_EFI_ERROR (Status);

  //
  // Install Ppi
  //
  Status = (**PeiServices).InstallPpi (PeiServices, mSaPeiInitPpi);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "Post-Mem SaInit Exit\n"));
  PostCode (0xA7F); // PEI_POST_MEM_SA_INIT_EXIT

  return EFI_SUCCESS;
}

/**
  Initialize the SA Aunit

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] SaPolicyPpi          Instance of SI_SA_POLICY_PPI
**/
VOID
SaAunitInit (
  IN CONST EFI_PEI_SERVICES  **PeiServices,
  IN SI_SA_POLICY_PPI        *SaPolicyPpi
  )
{
  UINT32          MchBarBase;
  UINTN           McD0BaseAddress;

  McD0BaseAddress = MmPciBase (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN);
  MchBarBase  = MmioRead32 (McD0BaseAddress + 0x48) &~BIT0;

  MmioWrite32(MchBarBase+ SA_UPARB_GCNT_A2B_0, 0x01010101);
  MmioWrite32(MchBarBase+ SA_UPARB_GCNT_A2B_1, 0x04010101);
  MmioWrite32(MchBarBase+ SA_UPARB_GCNT_A2B_2, 0x08000100);
  MmioWrite32(MchBarBase+ SA_UPARB_GCNT_A2B_3, 0x08010101);
  MmioWrite32(MchBarBase+ SA_UPARB_GCNT_A2B_4, 0x04000100);
  MmioWrite32(MchBarBase+ SA_UPARB_GCNT_A2B_5, 0x0101);
  MmioWrite32(MchBarBase+ SA_UPARB_GCNT_A2B_6, 0x04000101);
  MmioWrite32(MchBarBase+ SA_UPARB_GCNT_A2B_7, 0x04000101);
  MmioWrite32(MchBarBase+ SA_UPARB_GCNT_A2T_0, 0x01000101);
}


/**
  ProgramEcBase: Program the EC Base
**/
VOID
ProgramEcBase (
  VOID
  )
{
  //
  //Setting up EC base
  //
  PciCfg32Write_CF8CFC (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, CUNIT_PCICFG_PCIEXBAR_REG, (PCIEX_BASE_ADDRESS|BIT0));
}


/**
  Initialize the SA Punit

  @param[in] PeiServices          General purpose services available to every PEIM.
**/
VOID
SaPunitInit (
  IN CONST EFI_PEI_SERVICES  **PeiServices
  )
{
  UINT32                                Data = 0;
#ifndef FSP_WRAPPER_FLAG
  UINTN                                 PunitPciMemBase = 0;
  UINTN                                 PciD0F0RegBase = 0;
  UINTN                                 MchBar = 0;
#endif
  //
  // P-unit bring up
  //
  if (MmioRead32 (MCH_BASE_ADDRESS + 0x7078) == 0xffffffff) {
    //
    // P-unit not found - skip this flow
    //
    DEBUG ((EFI_D_ERROR, "Warning!! -- Punit MMIO not available - Skipping set RST_CPL.\n"));
  } else {
#ifndef FSP_WRAPPER_FLAG
    PunitPciMemBase = MmPciBase (0, 0, 1);
    //
    // Set PUNIT Interrupt Pin
    // Inaccessible after PUINT_INTPIN_LOCK is set (lock down bit)
    //
    MmioWrite8 ((UINTN)(PunitPciMemBase + R_PUINT_INTR_LAT + 1), (UINT8)BIT1);
    DEBUG((DEBUG_INFO, "PunitPciMemBase = 0x%x.\n",PunitPciMemBase));
    DEBUG((DEBUG_INFO, "PUNIT Interrupt Pin = 0x%x.\n",MmioRead8((UINTN)(PunitPciMemBase + 0x3D))));
    //
    // Set PUINT IRQ to 24 and INTPIN LOCK
    //
    PciD0F0RegBase = MmPciBase (0, 0, 0);
    MchBar          = MmioRead32 (PciD0F0RegBase + 0x48) &~BIT0;
    MmioWrite32 (MchBar + R_PUNIT_THERMAL_DEVICE_IRQ, V_PUINT_THERMAL_DEVICE_IRQ | B_PUINT_THERMAL_DEVICE_IRQ_LOCK);
    DEBUG((DEBUG_INFO, "PciD0F0RegBase = 0x%x.\n", PciD0F0RegBase));
    DEBUG((DEBUG_INFO, "MchBar = 0x%x.\n", MchBar));
    DEBUG((DEBUG_INFO, "R_PUNIT_THERMAL_DEVICE_IRQ = 0x%x.\n", MmioRead32 (MchBar + R_PUNIT_THERMAL_DEVICE_IRQ)));
#endif
    Data = MmioRead32 ((UINTN)(MCH_BASE_ADDRESS + 0x7818));
    Data &= 0xFFFFE01F;
    Data |= (BIT5 | BIT9);
    MmioWrite32((UINTN)(MCH_BASE_ADDRESS + 0x7818), Data);

    //
    // Need IAFW to indicate BIOS done to Pcode to complete Phase 7 of P-unit bring up
    // Note: If P-unit patch is loaded, the issue with MCH MMIO regs getting erased here is resolved
    //
    DEBUG ((EFI_D_INFO, "Setting Pcode RST_CPL (0x%x = 0x%x)\n",\
           (MCH_BASE_ADDRESS + 0x7078),\
           MmioRead32(MCH_BASE_ADDRESS + 0x7078) ));
    PostCode (0xA61); // PEI_SETTING_PCODE_RST_CPL

    MmioOr32 ((MCH_BASE_ADDRESS + 0x7078), BIT0);
    //
    // Poll for bit 8 in same reg (RST_CPL). It will be set once Pcode observes bit 0 being set.
    //
    DEBUG ((EFI_D_INFO, "Waiting for Pcode to acknowledge RST_CPL being set...\n"));
    while ( !(MmioRead32 (MCH_BASE_ADDRESS + 0x7078) & BIT8) ) {
      MicroSecondDelay (100);
    }

    // Set the "MemValid" bit in the SRAM which will be used to indicate memory contents need to be preserved to the MRC
    // on warm/cold reset and S3.
    MmioOr32 ((PcdGet32(PcdPmcSsramBaseAddress0) + 0x1410), BIT0);
  }
}

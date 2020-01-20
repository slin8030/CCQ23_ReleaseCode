/** @file
  PEIM to initialize IGFX PM

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
#include <Library/PeiServicesLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MmPciLib.h>
#include <IndustryStandard/Pci22.h>
#include <Library/PeiGraphicsInitLib.h>
#include <Library/PciLib.h>
#include <Library/TimerLib.h>
#include <SaAccess.h>
#include <CpuRegs.h>
#include <Ppi/SiPolicyPpi.h>
#include <Pi/PiBootMode.h>

///
/// Driver Consumed PPI Prototypes
///
#include <Ppi/SaPolicy.h>

GLOBAL_REMOVE_IF_UNREFERENCED BOOT_SCRIPT_REGISTER_SETTING  gSaGtRC6Registers[] = {
  //
  // Render/Video/Blitter Idle Max Count
  //
  { 0x0, 0x2054,  0x0, 0xA},
  { 0x0, 0x12054, 0x0, 0xA},
  { 0x0, 0x22054, 0x0, 0xA},
  { 0x0, 0x1A054, 0x0, 0xA},
  { 0x0, 0x1C054, 0x0, 0xA},
  { 0x0, 0x1C054, 0x0, 0xA},
  { 0x0, 0xC3E4,  0x0, 0xA},
  //
  // Enable Idle Messages
  //
  {0x0, 0x2050,  0x0, 0x00010000},
  {0x0, 0x12050, 0x0, 0x00010000},
  {0x0, 0x22050, 0x0, 0x00010000},
  {0x0, 0x1a050, 0x0, 0x00010000},
  {0x0, 0x1c050, 0x0, 0x00010000},
};

/**
  InitializeCdClock: Initialize Cd Clock for display engine.

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_ABORTED             S3 boot - display already initialized
  @retval EFI_UNSUPPORTED         iGfx disabled, iDisplay Audio not present
  @retval EFI_NOT_FOUND           SaPolicy or temporary GTT base address not found
**/
EFI_STATUS
InitializeCdClock (
  IN UINTN           GttMmAdr
  )
{
  EFI_BOOT_MODE              BootMode;
  EFI_STATUS Status;
  DEBUG ((DEBUG_INFO, "InitializeCdClock() Start\n"));

  Status = PeiServicesGetBootMode (&BootMode);
  ASSERT_EFI_ERROR (Status);

  if (BootMode == BOOT_ON_S3_RESUME) {
    MmioAnd32 (GttMmAdr + 0x46408, (UINT32) ~(BIT4)); // Clear NDE_RSTWRN_OPT RST PCH Handshake En to 0b.
    PollGtReady (GttMmAdr, 0x42000, BIT27, BIT27);  // Poll Fuse PG0 distribution status

    MmioOr32 (GttMmAdr + 0x45400, BIT29);           // Enable PG1
    PollGtReady (GttMmAdr, 0x45400, BIT28, BIT28);  // Poll for PG1 status
    PollGtReady (GttMmAdr, 0x42000, BIT26, BIT26);  // Poll Fuse PG1 distribution status

    MmioOr32 (GttMmAdr + 0x45400, BIT31);           // Enable PG2
    PollGtReady (GttMmAdr, 0x45400, BIT30, BIT30);  // Poll for PG2 status
    PollGtReady (GttMmAdr, 0x42000, BIT25, BIT25);  // Poll Fuse PG2 distribution status

    //Inform Power controll of upcoming freq change
    PollGtReady (GttMmAdr, 0x138124, BIT31, 0);
    MmioWrite32 (GttMmAdr + 0x138128, 0x80000000);
    MmioWrite32 (GttMmAdr + 0x138124, 0x80000017);
    PollGtReady (GttMmAdr, 0x138124, BIT31, 0);

    //Enable DE_PLL
    MmioWrite32 (GttMmAdr + 0x6d000,0x41);
    MmioOr32 (GttMmAdr + 0x46070, BIT31);
    PollGtReady (GttMmAdr, 0x46070, BIT30, BIT30);
    MmioWrite32 (GttMmAdr + 0x46000, 0x3004DE);     // Program 624 MHz CD clock.

    //Inform Power controller of the selected freq
    MmioWrite32 (GttMmAdr + 0x138128, 0x00000019);
    MmioWrite32 (GttMmAdr + 0x138124, 0x80000017);

    //DBUF Power Well Control
    MmioOr32 (GttMmAdr + 0x45008, (BIT31));
    PollGtReady (GttMmAdr, 0x45008, BIT30, BIT30);
  }

  DEBUG ((DEBUG_INFO, "InitializeCdClock() End\n"));
  return EFI_SUCCESS;
}

/**
  Program the max Cd Clock supported by the platform

  @param[in] SaPolicy            Instance of SA policy
  @param[in] GttMmAdr            Base Address of IGFX MMIO BAR

  @retval EFI_SUCCESS            GT Power Management initialization complete
  @retval EFI_INVALID_PARAMETER  The input parameter is invalid
**/
EFI_STATUS
CdClkInit (
  IN GRAPHICS_CONFIG *GtConfig,
  IN UINTN           GttMmAdr
  )
{
  UINT32 Data32Or;
  ///
  /// CDCLK_CTL - GttMmAdr + 0x46000
  /// CdClock = 0; 144 Mhz - [10:0] = 0x11E
  /// CdClock = 1; 288 Mhz - [10:0] = 0x23E
  /// CdClock = 2; 384 Mhz - [10:0] = 0x2FE
  /// CdClock = 3; 576 Mhz - [10:0] = 0x47E
  /// CdClock = 4; 624 Mhz - [10:0] = 0x4DE
  ///
  switch (GtConfig->CdClock) {
    case 0 :
      Data32Or = 0x11E;
    break;
    case 1 :
      Data32Or = 0x23E;
    break;
    case 2 :
      Data32Or = 0x2FE;
    break;
    case 3 :
      Data32Or = 0x47E;
    break;
    case 4 :
      Data32Or = 0x4DE;
    break;
    default:
      return EFI_INVALID_PARAMETER;
  }
  DEBUG ((DEBUG_INFO, "CdClkInit Value[10:0] = 0x%x\n", Data32Or));
  MmioAndThenOr32 (GttMmAdr + 0x46000, 0xFFFFF800, Data32Or);
  return EFI_SUCCESS;
}


/**
  Initialize GT PowerManagement of SystemAgent.

  @param[in] GtConfig             Instance of GRAPHICS_CONFIG
  @param[in] GttMmAdr             Base Address of IGFX MMIO BAR
  @param[in] MchBarBase           Base Address of MCH_BAR

  @retval EFI_SUCCESS            GT Power Management initialization complete
  @retval EFI_INVALID_PARAMETER  The input parameter is invalid
**/
EFI_STATUS
PmInit (
  IN GRAPHICS_CONFIG *GtConfig,
  IN UINTN           GttMmAdr,
  IN UINT32          MchBarBase
  )
{
  UINT8                         i;
  UINT32                        Data32And;
  UINT32                        Data32Or;
  UINT32                        Data32;
  UINT32                        RegOffset;
  UINT32                        Data32Mask;
  UINT32                        Result;
  UINT32                        GMSSizeSelector;
  UINT32                        GMSSize;
  UINT32                        GMSBase;
  UINT32                        RC6CXTBASE;
  UINTN                         McD0BaseAddress;

  McD0BaseAddress   = MmPciBase (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN);
  //
  // PmInit Initialization
  //
  DEBUG ((EFI_D_INFO, "Initializing GT PowerManagement\n"));

  if ((GttMmAdr == 0) || (MchBarBase == 0) || (GtConfig == NULL)) {
    DEBUG ((DEBUG_ERROR, "Invalid parameters for PmInit\n"));
    return EFI_INVALID_PARAMETER;
  }

  ///
  /// 1a. Set RC6 Context Location
  ///
  RegOffset                     = 0xD40;
  Data32                        = 0x80000001;
  MmioWrite32 (GttMmAdr + RegOffset, Data32);

  ///
  /// 1b. Set Context Base
  /// Must set to a physical address within GT stolen Memory WOPCM, at least 24KB from the top.
  /// This range must be coordinated with other uses (like GuC and PAVP);
  /// It is expected that the upper 24KB of stolen memory is the proper location
  /// Also set bit 2:0 to 111b
  ///

  GMSSizeSelector = MmioRead32 (McD0BaseAddress + R_SA_GGC);
  GMSSizeSelector = (GMSSizeSelector & B_SA_GGC_GMS_MASK) >> N_SA_GGC_GMS_OFFSET;
  GMSSize = (UINT32) GMSSizeSelector * 32;
  DEBUG ((DEBUG_INFO, "GMSSize: %dMB\n",GMSSize));
  GMSBase = MmioRead32 (McD0BaseAddress + R_SA_BDSM) & B_SA_BDSM_BDSM_MASK;
  DEBUG ((DEBUG_INFO, "GMSBase read from R_SA_BDSM: 0x%x\n",GMSBase));

  //
  //RC6CXTBASE: the algorithm is BDSM+BDSM_SIZE-RC6CTXBASE_SIZE if WOPCM is not enabled.  If WOPCM is enabled, it should be WOPCMBASE+WOPCM_SIZE-RC6CTXBASE_SIZE.
  //In current design, WOPCM region : WOPCMbase (PAVPC bits 31:20) to DSMtop (BDSM + GMS) with PCME (PAVPC bit 0) = 1
  //so we can use RC6CXTBASE = DSMtop (BDSM + GMS) -RC6CTXBASE_SIZE in either case
  //

  RegOffset                     = 0xD48;
  RC6CXTBASE                    = GMSBase + GMSSize * 0x100000 - RC6CTXBASE_SIZE;
  Data32                        = RC6CXTBASE | BIT1 | BIT0;
  DEBUG ((DEBUG_INFO, "RC6 Context Base: 0x%x\n",RC6CXTBASE));
  MmioWrite32 (GttMmAdr + RegOffset, Data32);

  if (GtConfig->PmSupport){
    ///
    /// Following steps are following Gen9 GT PM Programming Guide
    ///
    ///
    /// Set chicken bits to enable GT B+ stepping workarounds
    ///
    RegOffset = 0xD00;
    Data32 = 0xE;
    MmioWrite32(GttMmAdr + RegOffset, Data32);

    ///
    /// 1aa. Enable all GTI-Uncore clock gating
    ///
    RegOffset                     = 0xD08;
    Data32                        = 0x00000000;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// 2a. Enable Force Wake
    ///
    RegOffset                     = 0xA188;
    Data32                        = 0x00010001;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// 2b. Poll to verify Force Wake Acknowledge Bit
    ///
    RegOffset                     = 0x130044;
    Data32Mask                    = BIT0;
    Result                        = 1;
    DEBUG ((DEBUG_INFO, "Polling Force Wake Acknowledge Bit 0x130044 bit0 to be 1...\n"));
    PollGtReady (GttMmAdr, RegOffset, Data32Mask, Result);

    ///
    /// 3a. Enabling Push Bus Metric Control
    ///
    RegOffset                     = 0xA250;
    Data32                        = 0x000001FF;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// 3b. Configuring Push Bus Shift
    ///
    RegOffset                     = 0xA25C;
    Data32                        = 0x00000010;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// 3c. Pushbus Metric Control
    ///
    RegOffset                     = 0xA248;
    Data32                        = 0x80000004;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// 4. Program GfxPause Register
    ///
    RegOffset                     = 0xA000;
    Data32                        = 0x00070020;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// 5a. GPM Control
    ///
    RegOffset                     = 0xA180;
    Data32                        = 0xC5200000;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    if ((GetBxtSeries() == BxtP && BxtStepping() >= BxtPB0) || (GetBxtSeries() != BxtP && BxtStepping() >= BxtC0)) {
      RegOffset                     = 0xA194;
      Data32Or                      = BIT10;
      Data32And                     = (UINT32) ~(BIT9 | BIT10);
      MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
    }

    ///
    /// 5b. Enable DOP clock gating.
    ///
    Data32 = 0x000007FD;


    RegOffset                     = 0x9424;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);
    DEBUG ((DEBUG_INFO, "Enabled DOP clock gating \n"));

    ///
    /// 5c-5g. Enable unit level clock gates
    ///
      RegOffset                     = 0x9400;
      Data32                        = 0x00000000;
      MmioWrite32 (GttMmAdr + RegOffset, Data32);

      RegOffset                     = 0x9404;
      Data32                        = 0x40401000;
      MmioWrite32 (GttMmAdr + RegOffset, Data32);

      RegOffset                     = 0x9408;
      Data32                        = 0x00000000;
      MmioWrite32 (GttMmAdr + RegOffset, Data32);

      RegOffset                     = 0x940C;
      Data32                        = 0x02000001;
      MmioWrite32 (GttMmAdr + RegOffset, Data32);
      //
      //0x9430 [28] = 1: HDCREQ Clock Gating Disable for all steppings
      //0x9430[14] = 1: SDE units Clock Gating Disable for A steppings
      //
      RegOffset                     = 0x9430;
      Data32 =  MmioRead32 (GttMmAdr + RegOffset);

      Data32 |= BIT28;

      MmioWrite32 (GttMmAdr + RegOffset, Data32);



    ///
    /// 6-7 SW RC6 Settings
    ///
    for (i = 0; i < sizeof (gSaGtRC6Registers) / sizeof (BOOT_SCRIPT_REGISTER_SETTING); ++i) {
      RegOffset                     = gSaGtRC6Registers[i].Offset;
      Data32And                     = gSaGtRC6Registers[i].AndMask;
      Data32Or                      = gSaGtRC6Registers[i].OrMask;

      MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
    }

    ///
    /// 8A. Set Normal frequency request:
    ///
    Data32 = 0x6000000;
    RegOffset = 0xA008;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// 8A. RP Control
    ///
    Data32 = 0x00000592;
    RegOffset = 0xA024;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// 9. Enabling to enter RC6 state in idle mode.
    ///
    if (GtConfig->EnableRenderStandby) {
      RegOffset                     = 0xA094;
      Data32                        = 0x00040000;
      MmioWrite32 (GttMmAdr + RegOffset, Data32);
      DEBUG ((DEBUG_INFO, "Entered RC6 state in idle mode\n"));
    }

      ///
      /// 10a. Clear offset 0xA188 [31:0] to clear the force wake enable
      ///
      RegOffset                     = 0xA188;
      Data32                        = 0x00010000;
      MmioWrite32 (GttMmAdr + RegOffset, Data32);

      ///
      /// 10b. Poll until clearing is cleared to verify the force wake acknowledge.
      ///
      RegOffset                     = 0x130044;
      Data32Mask                    = BIT0;
      Result                        = 0;
      DEBUG ((DEBUG_INFO, "Polling Force Wake acknowledge Bit 0x130044 bit0 to be 0...\n"));
      PollGtReady (GttMmAdr, RegOffset, Data32Mask, Result);
  }
  return EFI_SUCCESS;
}

/**
  Initialize PAVP feature of SystemAgent.

  @param[in] GRAPHICS_CONFIG  *GtConfig

  @retval EFI_SUCCESS     PAVP initialization complete
**/
EFI_STATUS
PavpInit (
  IN       GRAPHICS_CONFIG             *GtConfig
  )
{
  UINT32        PcmBase = 0;
  UINTN         McD0BaseAddress;
  UINT32        Pavpc;
  UINT32        GMSSizeSelector;
  UINT32        GMSSize;
  UINT32        GMSBase;
  SI_POLICY_PPI *SiPolicyPpi;
  EFI_STATUS    Status;

  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **)&SiPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);

  ///
  /// PAVP Initialization
  ///
  DEBUG ((EFI_D_INFO, "Initializing PAVP - Start\n"));
  McD0BaseAddress = MmPciBase (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN);
  Pavpc           = MmioRead32 (McD0BaseAddress + R_SA_PAVPC);
  Pavpc &= (UINT32) ~(B_SA_PAVPC_HVYMODSEL_MASK | B_SA_PAVPC_PCMBASE_MASK | B_SA_PAVPC_PAVPE_MASK | B_SA_PAVPC_PCME_MASK);

  //
  //WOPCM region : WOPCMbase (PAVPC bits 31:20) to DSMtop (BDSM + GMS) with PCME (PAVPC bit 0) = 1
  //
  GMSSizeSelector = MmioRead32 (McD0BaseAddress + R_SA_GGC);
  GMSSizeSelector = (GMSSizeSelector & B_SA_GGC_GMS_MASK) >> N_SA_GGC_GMS_OFFSET;
  GMSSize = (UINT32) GMSSizeSelector * 32;
  GMSBase = MmioRead32 (McD0BaseAddress + R_SA_BDSM) & B_SA_BDSM_BDSM_MASK;

  if (GtConfig->PavpEnable == 1) {
        PcmBase = GMSBase + (GMSSize - PAVP_PCM_SIZE_1_MB ) * 0x100000;

    Pavpc |= PcmBase;
    Pavpc |= B_SA_PAVPC_PCME_MASK | B_SA_PAVPC_PAVPE_MASK;
      Pavpc &= (UINT32) ~(BIT4);

    if ((GtConfig->PavpPr3 == 1) && (SiPolicyPpi->OsSelection == SiWindows)) {
      Pavpc |= BIT6;
    } else {
      Pavpc &= (UINT32) ~(BIT6);
    }
  }

  ///
  /// Lock PAVPC Register
  ///
    Pavpc |= B_SA_PAVPC_PAVPLCK_MASK;
  MmioWrite32 (McD0BaseAddress + R_SA_PAVPC, Pavpc);

  DEBUG ((EFI_D_INFO, "Initializing PAVP - End\n"));
  return EFI_SUCCESS;
}



/**
  Initialize GT Power management

  @param[in] GRAPHICS_CONFIG             GtConfig

  @retval EFI_SUCCESS           GT Power management initialization complete
**/
EFI_STATUS
GraphicsPmInit (
  IN       GRAPHICS_CONFIG             *GtConfig
  )
{
  UINT32                LoGTBaseAddress;
  UINT32                HiGTBaseAddress;
  UINTN                 McD2BaseAddress;
  UINTN                 McD0BaseAddress;
  UINT32                GttMmAdr;
  UINT32                MchBarBase;
  UINT32                Data32;
  UINT32                Data32And;

  DEBUG((DEBUG_INFO, " iGfx Power management start.\n"));

  GttMmAdr   = 0;
  MchBarBase = 0;
  MchBarBase = MmioRead64 (MmPciBase (SA_MC_BUS, 0, 0) + 0x48) &~BIT0;
  McD0BaseAddress = MmPciBase (SA_MC_BUS, 0, 0);

  ///
  /// If device 0:2:0 (Internal Graphics Device, or GT) is enabled, then Program GttMmAdr,
  ///
  McD2BaseAddress = MmPciBase (SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0);
  if (MmioRead16 (McD2BaseAddress + R_SA_IGD_VID) != 0xFFFF) {
    ///
    /// Program GT PM Settings if GttMmAdr allocation is Successful
    ///
    GttMmAdr                          = GtConfig->GttMmAdr;
    LoGTBaseAddress                   = (UINT32) (GttMmAdr & 0xFFFFFFFF);
    HiGTBaseAddress                   = 0;
    MmioWrite32 (McD2BaseAddress + R_SA_IGD_GTTMMADR, LoGTBaseAddress);
    MmioWrite32 (McD2BaseAddress + R_SA_IGD_GTTMMADR + 4, HiGTBaseAddress);

    ///
    /// Enable Bus Master and Memory access on 0:2:0
    ///
    MmioOr16 (McD2BaseAddress + R_SA_IGD_CMD, (BIT2 | BIT1));


    if (BxtStepping () >= BxtB0) {
      MmioOr32 (GttMmAdr + 0x101010, BIT1);
    }

    ///
    /// Enable VGA paging only when IGD has VGA decode enabled.
    ///
//[-start-160705-IB07400752-modify]//
//    Data32 = MmioRead32 (McD0BaseAddress + R_SA_GGC);
//    Data32 = (Data32 & 0x2) >> 1;
//    if (Data32 == 0) {
      MmioWrite32 (MchBarBase + 0x6800, 0x3);
//    }
//[-end-160705-IB07400752-modify]//

    ///
    /// PmInit Initialization
    ///
    DEBUG ((DEBUG_INFO, "Initializing GT PowerManagement\n"));
    PmInit (GtConfig, GttMmAdr, MchBarBase);

    ///
    /// Copy MSR_PLATFORM_INFO.B_PLATFORM_INFO_PRODUCTION bit to CONFIG0 Address D00h, bit 30 and Lock bit
    ///
    Data32 = (AsmReadMsr64 (MSR_PLATFORM_INFO) & B_PLATFORM_INFO_PRODUCTION);
    Data32 = (Data32 << 3) | BIT31;
    Data32And = (UINT32) ~(BIT31 | BIT30);
    MmioAndThenOr32 (GttMmAdr + 0xD00, Data32And, Data32);
    Data32 = MmioRead32 (GttMmAdr + 0xD00);
    DEBUG ((DEBUG_INFO, "Update CONFIG0 Address D00 : %x\n", Data32));

    ///
    ///Program the max Cd Clock supported by the platform
    ///
    if (PLATFORM_ID == VALUE_REAL_PLATFORM) {
      CdClkInit (GtConfig, GttMmAdr);
    }

    ///
    ///Initialize Display CD Clock.
    ///
    InitializeCdClock (GttMmAdr);

    ///
    /// Lock Programmable BDFs for the Audio functions and Touch Controller
    ///
    MmioWrite32 (GttMmAdr + 0x1300B0, 0x00100001);
    MmioOr32 (GttMmAdr + 0x1300B4, BIT0);

    ///
    /// Disable Bus Master and Memory access on 0:2:0
    ///
    MmioAnd16 (McD2BaseAddress + R_SA_IGD_CMD, (UINT16) ~(BIT2 | BIT1));

    ///
    /// Clear GttMmAdr
    ///
    MmioWrite32 (McD2BaseAddress + R_SA_IGD_GTTMMADR, 0);
    MmioWrite32 (McD2BaseAddress + R_SA_IGD_GTTMMADR + 0x4, 0);

  }
//[-start-161015-IB07400797-add]//
  else {
    Data32 = MmioRead32 (MchBarBase + 0x6800);
    Data32 &= (~BIT2); // IGD disabled, defaut disable ABSeginDRAM
    MmioWrite32 (MchBarBase + 0x6800, Data32);
  }
//[-end-161015-IB07400797-add]//

  DEBUG((DEBUG_INFO, "iGfx Power management end.\n"));
  DEBUG ((DEBUG_INFO, "Lock GGC, BDSM and BGSM registers\n"));

  //
  // Lock the following registers - GGC, BDSM, BGSM
  //
  MmioOr32 (McD0BaseAddress + R_SA_GGC, BIT0);
  MmioOr32 (McD0BaseAddress + R_SA_BDSM, BIT0);
  MmioOr32 (McD0BaseAddress + R_SA_BGSM, BIT0);

  return EFI_SUCCESS;
}

/**
  "Poll Status" for GT Readiness

  @param[in] Base             Base address of MMIO
  @param[in] Offset           MMIO Offset
  @param[in] Mask             Mask
  @param[in] Result           Value to wait for

  @retval  None
**/
VOID
PollGtReady (
  IN       UINT64                       Base,
  IN       UINT32                       Offset,
  IN       UINT32                       Mask,
  IN       UINT32                       Result
  )
{
  UINT32  GtStatus;
  UINT16  StallCount;

  StallCount = 0;

  ///
  /// Register read
  ///
  GtStatus = MmioRead32 ((UINTN) Base + Offset);

  while (((GtStatus & Mask) != Result) && (StallCount < GT_WAIT_TIMEOUT)) {

    GtStatus = MmioRead32 ((UINTN) Base + Offset);
    ///
    /// 1mSec wait
    ///
    MicroSecondDelay (1000);
    StallCount = StallCount + 1;
  }
  ASSERT ((StallCount != GT_WAIT_TIMEOUT));
}

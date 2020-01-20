/** @file
  Source code file for Silicon Init Pre Memory module.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2017 Intel Corporation.

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
#include <Ppi/ScPolicy.h>
#include <Ppi/SiPolicyPpi.h>
#include <Ppi/SeCUma.h>
#include <Ppi/CpuPolicy.h>
#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/SteppingLib.h>
#include <Library/PeiSaPolicyLib.h>
#include <Private/Library/CpuInitLib.h>
#include <Private/Library/PeiScInitLib.h>
#include <Library/PeiGraphicsInitLib.h>
#include <Library/PeiNpkInitLib.h>
#include <Library/PeiSaInitLib.h>
#include <Library/PostCodeLib.h>
#include <Library/Private/PeiCpuInitLib/CpuInitPeim.h>
#include <Library/PerformanceLib.h>
#include <Library/SideBandLib.h>
#include <Library/CpuMailboxLib.h>
//[-start-160308-IB03090425-add]//
#include <Ppi/MemoryDiscovered.h>
//[-end-160308-IB03090425-add]//

EFI_STATUS
EFIAPI
SeCUmaEntry (
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  );

static EFI_PEI_NOTIFY_DESCRIPTOR  mNpkInitNotifyList[] = {
  {
   (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiMemoryDiscoveredPpiGuid,
    NpkOnMemoryInstalled
  }
};

/**
  Configure Real Time enabling support.

  @param[in]  RtEn                The Real Time enable configuration type

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
SiInitRt (
  IN UINT8  RtEn
)
{
  UINTN   PciD0F0RegBase;
  UINTN   MchBar;
  UINT32  Data;
  UINT32  MailboxData;
  UINT32  MailboxCmd;
  UINT32  MailboxStatus;
  UINT32  MailboxType;

  DEBUG((EFI_D_INFO, "SiInitRt() - Start\n"));

  PciD0F0RegBase = MmPciBase (0, 0, 0);
  MchBar         = MmioRead32 (PciD0F0RegBase + R_SA_MCHBAR_REG) &~BIT0;

  // ISM disable
  Data = SideBandRead32 (SB_PORTID_PSF1, R_PCH_PCR_PSF_GLOBAL_CONFIG);
  Data &= ~BIT3;
  SideBandWrite32 (SB_PORTID_PSF1, R_PCH_PCR_PSF_GLOBAL_CONFIG, Data);
  Data = SideBandRead32 (SB_PORTID_PSF1, R_PCH_PCR_PSF_GLOBAL_CONFIG);

  Data = SideBandRead32 (SB_PORTID_PSF0, R_PCH_PCR_PSF_GLOBAL_CONFIG);
  Data |= (BIT7 | BIT2);
  SideBandWrite32 (SB_PORTID_PSF0, R_PCH_PCR_PSF_GLOBAL_CONFIG, Data);
  Data = SideBandRead32 (SB_PORTID_PSF0, R_PCH_PCR_PSF_GLOBAL_CONFIG);

  // Realtime settings
  Data = SideBandRead32 (SB_PORTID_PSF1, 0x405C);
  Data = Data & ~(BIT15 | BIT14 | BIT13 | BIT12);
  SideBandWrite32 (SB_PORTID_PSF1, 0x405C, Data);
  Data = SideBandRead32 (SB_PORTID_PSF1, 0x405C);

  // IDI Real-Time Feature Configuration Bits settings AGENT_DIDX
  Data = MmioRead32 (MchBar + R_BUNIT_RT_EN);
  DEBUG ((EFI_D_INFO, "Set AGENT Disabled \n\r"));
  Data |= B_BUNIT_RT_EN_RT_ENABLE; // RT_IDI_AGENT (bit 17:16) = 1'b00,  RT_ENABLE (bit 0) = 1

  MmioWrite32 (MchBar + R_BUNIT_RT_EN, Data);
  Data = MmioRead32 (MchBar + R_BUNIT_RT_EN);
  DEBUG ((EFI_D_INFO, "B_CR_RT_EN_0_0_0_MCHBAR = 0x%x\n\r", Data));

  // punit to stop all memory subsystem power management
  MailboxType = MAILBOX_TYPE_PCODE;
  MailboxCmd  = BIT31 | 0x21;
  MailboxData = 0x80008000;
  MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);

  MailboxCmd  = BIT31 | 0x38;
  MailboxData = 0x00000001;
  MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);

  DEBUG ((EFI_D_INFO, "SiInitRt() - End\n"));

  return EFI_SUCCESS;
}

/**
  Silicon Initializes before PlatformPolicy PPI produced

  @param[in] FileHandle           The file handle of the file, Not used.
  @param[in] PeiServices          General purpose services available to every PEIM.

  @retval EFI_SUCCESS             The function completes successfully
**/
EFI_STATUS
EFIAPI
SiInitPrePolicy (
  IN  EFI_PEI_FILE_HANDLE      FileHandle,
  IN CONST EFI_PEI_SERVICES  **PeiServices
  )
{
  EFI_STATUS           Status;
  BXT_SERIES           BxtSeries;
  BOOLEAN              ImageInMemory;
  SC_PREMEM_POLICY_PPI *ScPreMemPolicyPpi;
  SI_SA_POLICY_PPI     *SiSaPolicyPpi;
  SA_PRE_MEM_CONFIG    *SaPreMemConfig;

  ImageInMemory = FALSE;
  DEBUG ((DEBUG_INFO, "SiInitPrePolicy() Start\n"));

  Status = (*PeiServices)->RegisterForShadow (FileHandle);
  if (Status == EFI_ALREADY_STARTED) {
    ImageInMemory = TRUE;
  } else if (Status == EFI_NOT_FOUND) {
    ASSERT_EFI_ERROR (Status);
  }

  if (!ImageInMemory) {

    //
    // @todo: Initializes SC before Policy initialized
    //
    #if (ENBDT_PF_ENABLE == 1) || (FSP_FLAG == 1)
      ScInitPrePolicy ();
    #endif
    BxtSeries = GetBxtSeries ();
    if (BxtSeries == BxtP || (BxtSeries == Bxt1)) {
      Status = PeiServicesLocatePpi (
                 &gScPreMemPolicyPpiGuid,
                 0,
                 NULL,
                 (VOID **)&ScPreMemPolicyPpi
                 );
      ASSERT_EFI_ERROR (Status);

     PERF_START_EX (NULL, NULL, NULL, AsmReadTsc(), 0x5020);
     ScConfigurePciePowerSequence (ScPreMemPolicyPpi);
     PERF_END_EX (NULL, NULL, NULL, AsmReadTsc(), 0x5021);
    }

    DEBUG((DEBUG_INFO, "Pre-Mem North Cluster Entry\n"));
    PostCode (0xA00);

    //
    // NPK Pei Initialization
    //
    PeiNpkInit(PeiServices);

    Status = (*PeiServices)->LocatePpi (
                               PeiServices,
                               &gSiSaPreMemPolicyPpiGuid,
                               0,
                               NULL,
                               (void **)&SiSaPolicyPpi
                               );
    ASSERT_EFI_ERROR(Status);

    Status = GetConfigBlock ((VOID *) SiSaPolicyPpi, &gSaPreMemConfigGuid, (VOID *) &SaPreMemConfig);
    ASSERT_EFI_ERROR (Status);

    DEBUG((DEBUG_INFO, "SaPreMemConfig->RtEn %X\n", SaPreMemConfig->RtEn));
    if (SaPreMemConfig->RtEn != 0) {
      ///
      /// Real Time enabling
      ///
      Status = SiInitRt (SaPreMemConfig->RtEn);
    }

    ///
    /// Initialize Graphics (IGD)
    ///
    DEBUG ((DEBUG_INFO, "Initializing Pre-Mem Graphics\n"));
    PostCode (0xA50);
    PERF_START_EX (NULL, NULL, NULL, AsmReadTsc(), 0x5030);
    GraphicsPreMemInit (PeiServices);
    PERF_END_EX (NULL, NULL, NULL, AsmReadTsc() , 0x5031);

    DEBUG((DEBUG_INFO, "Pre-Mem North Cluster Exit\n"));
    PostCode (0xA7F);

    #ifndef FSP_FLAG
    PERF_START_EX (NULL, NULL, NULL, AsmReadTsc(), 0x5050);
    BuildBistHob(PeiServices);
    PERF_END_EX (NULL, NULL, NULL, AsmReadTsc(), 0x5051);
    #endif

  } else {

    //
    // Silicon init call back after PlatformPolicy PPI produced
    //
    CpuOnPolicyInstalled();

    //
    // Register NPK init call back after PlatformPolicy PPI produced
    //
    DEBUG ((DEBUG_INFO, " NPK install memory notify PPI\n"));
    Status = PeiServicesNotifyPpi (mNpkInitNotifyList);
  }

  DEBUG ((DEBUG_INFO, "SiInitPrePolicy() - End\n"));
  return EFI_SUCCESS;
}

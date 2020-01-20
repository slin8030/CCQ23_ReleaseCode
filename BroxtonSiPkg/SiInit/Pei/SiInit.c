/** @file
    Source code file for Silicon Init Post Memory module.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2017 Intel Corporation.

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

#include <Ppi/ScPolicy.h>
#include <SiPolicyHob.h>
#include <Ppi/EndOfPeiPhase.h>
//[-start-160414-IB03090426-add]//
#include <Ppi/SmmAccess.h>
//[-end-160414-IB03090426-add]//
#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <Private/Library/CpuInitLib.h>
#include <Private/Library/SmbiosCpuLib.h>
#include <Private/Library/MpServiceLib.h>
#include <Private/Library/CpuPowerOnConfigLib.h>

#ifndef FSP_FLAG
#include <Private/Library/CpuS3Lib.h>
#else
#include <Library/FspCommonLib.h>
#include <FspsUpd.h>
#endif

#include <Private/Library/PeiScInitLib.h>
#include <Library/PeiSaInitLib.h>
#include <Library/PerformanceLib.h>
//[-start-160217-IB07400702-add]//
#include <PostCode.h>
#include <ChipsetPostCode.h>
//[-end-160217-IB07400702-add]//
/**
  Slicon Initializes after PlatformPolicy PPI produced, All required polices must be installed before the callback

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] NotifyDescriptor     The notification structure this PEIM registered on install.
  @param[in] Ppi                  The memory discovered PPI.  Not used.

  @retval EFI_SUCCESS             Succeeds.
**/
EFI_STATUS
EFIAPI
SiInitAfterPolicyInstalled (
  IN  EFI_PEI_SERVICES             **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
  IN  VOID                         *Ppi
  );

#ifndef FSP_FLAG
EFI_STATUS
CpuS3SmmAccessNotifyCallback(
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  );
#endif
/**
  Silicon Init End of PEI callback function. This is the last change before entering DXE and OS when S3 resume.

  @param[in] PeiServices   - Pointer to PEI Services Table.
  @param[in] NotifyDesc    - Pointer to the descriptor for the Notification event that
                             caused this function to execute.
  @param[in] Ppi           - Pointer to the PPI data associated with this function.

  @retval EFI_STATUS       - Always return EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
SiInitOnEndOfPei (
  IN EFI_PEI_SERVICES                   **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR          *NotifyDesc,
  IN VOID                               *Ppi
  );

static EFI_PEI_NOTIFY_DESCRIPTOR  mSiInitNotifyList[] = {
#ifndef FSP_FLAG
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK,
    &gPeiSmmAccessPpiGuid,
    CpuS3SmmAccessNotifyCallback
  },
#endif
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gEfiEndOfPeiSignalPpiGuid,
    SiInitOnEndOfPei
  }
};

/**
  Silicon Init End of PEI callback function. This is the last change before entering DXE and OS when S3 resume.

  @param[in] PeiServices   - Pointer to PEI Services Table.
  @param[in] NotifyDesc    - Pointer to the descriptor for the Notification event that
                             caused this function to execute.
  @param[in] Ppi           - Pointer to the PPI data associated with this function.

  @retval EFI_STATUS       - Always return EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
SiInitOnEndOfPei (
  IN EFI_PEI_SERVICES                   **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR          *NotifyDesc,
  IN VOID                               *Ppi
  )
{
  DEBUG ((DEBUG_INFO, "SiInitOnEndOfPei - Start\n"));

  ///
  /// Initializes SC after End of Pei
  ///
  ScOnEndOfPei();

  InitializeSmbiosCpuHobs ();

  DEBUG ((DEBUG_INFO, "SiInitOnEndOfPei - End\n"));
  return EFI_SUCCESS;
}

/**
  Silicon Initializes after memory services initialized

  @param[in] FileHandle           The file handle of the file, Not used.
  @param[in] PeiServices          General purpose services available to every PEIM.

  @retval EFI_SUCCESS             The function completes successfully
**/
EFI_STATUS
EFIAPI
SiInit (
  IN  EFI_PEI_FILE_HANDLE               FileHandle,
  IN CONST EFI_PEI_SERVICES             **PeiServices
  )
{
  EFI_STATUS        Status;
  SI_POLICY_PPI     *SiPolicyPpi;
  SC_POLICY_PPI     *ScPolicyPpi;
  SI_CPU_POLICY_PPI *CpuPolicy;
  VOID              *HobPtr;
#ifdef FSP_FLAG
  FSPS_UPD          *FspsUpd;
#endif

  DEBUG ((DEBUG_INFO, "SiInit () Start\n"));
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SI_INIT_START);  
//[-end-160217-IB07400702-add]//

  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **)&SiPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);

  Status = PeiServicesLocatePpi (
             &gScPolicyPpiGuid,
             0,
             NULL,
             (VOID **)&ScPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);
  //
  // Locate installed CPU Policy PPI
  //
  Status = PeiServicesLocatePpi (
             &gSiCpuPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &CpuPolicy
             );
  if (Status != EFI_SUCCESS) {
    //
    // CPU_POLICY_PPI must be installed at this point
    //
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  HobPtr = BuildGuidDataHob (&gSiPolicyHobGuid, SiPolicyPpi, sizeof (SI_POLICY_PPI));
  ASSERT (HobPtr != 0);

#ifdef FSP_FLAG
  FspsUpd = (FSPS_UPD *)GetFspSiliconInitUpdDataPointer ();
#endif
  //
  // Program registers that needs to be done before RESET_CPL
  //
  PERF_START_EX (NULL, NULL, NULL, 0, 0x4010);
#ifdef FSP_FLAG
  if ( FspsUpd->FspsConfig.SkipPunitInit == 0) {
#endif
    CpuInitPreResetCpl (CpuPolicy);
#ifdef FSP_FLAG
  }
#endif
  PERF_END_EX (NULL, NULL, NULL, 0, 0x4011);

  //
  // Punit Init
  //
  DEBUG ((DEBUG_INFO, "SiInit () SaPunitInit\n"));
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SI_INIT_SA_PUINT_INIT);
//[-end-160217-IB07400702-add]//
  PERF_START_EX (NULL, NULL, NULL, 0, 0x4020);
#ifdef FSP_FLAG
  if ( FspsUpd->FspsConfig.SkipPunitInit == 0) {
#endif
    SaPunitInit(PeiServices);
#ifdef FSP_FLAG
  }
#endif
  PERF_END_EX (NULL, NULL, NULL, 0, 0x4021);

  //
  // Initialize SC after Policy PPI produced
  //
//[-start-160217-IB07400702-add]//
//  POST_CODE (PEI_SB_REG_INIT);
//[-end-160217-IB07400702-add]//
  PERF_START_EX (NULL, NULL, NULL, 0, 0x4030);
  ScInit (SiPolicyPpi, ScPolicyPpi);
  PERF_END_EX (NULL, NULL, NULL, 0, 0x4031);

  //
  // Initialize multi-processor service
  //
//[-start-160217-IB07400702-add]//
//  POST_CODE (PEI_CPU_AP_INIT);
  CHIPSET_POST_CODE (PEI_FRC_SI_INIT_INIT_MP);
//[-end-160217-IB07400702-add]//
  PERF_START_EX (NULL, NULL, NULL, 0, 0x4040);
#ifdef FSP_FLAG
  if ( FspsUpd->FspsConfig.SkipMpInit == 0) {
#endif
  Status = InitializeMpServices (CpuPolicy);
#ifdef FSP_FLAG
  }
#endif
  PERF_END_EX (NULL, NULL, NULL, 0, 0x4041);

  //
  // Initialize processor features, performance and power management features, BIOS GUARD, and overclocking etc features before RESET_CPL at post-memory phase.
  //
//[-start-160217-IB07400702-add]//
//  POST_CODE (PEI_CPU_REG_INIT);
  CHIPSET_POST_CODE (PEI_FRC_SI_INIT_CPU_INIT);
//[-end-160217-IB07400702-add]//
  PERF_START_EX (NULL, NULL, NULL, 0, 0x4050);
  Status = CpuInit (CpuPolicy);
  PERF_END_EX (NULL, NULL, NULL, 0, 0x4051);


  //
  // Initialize power management after RESET_CPL at post-memory phase.
  //
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SI_INIT_CPU_PM_INIT);
//[-end-160217-IB07400702-add]//
  PERF_START_EX (NULL, NULL, NULL, 0, 0x4070);
#ifdef FSP_FLAG
  if ( FspsUpd->FspsConfig.SkipMpInit == 0) {
#endif
  CpuPowerMgmtInit ();
#ifdef FSP_FLAG
  }
#endif
  PERF_END_EX (NULL, NULL, NULL, 0, 0x4071);

  //
  // Install EndOfPei callback function.
  //
  Status = PeiServicesNotifyPpi (mSiInitNotifyList);
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize System Agent
  //
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SI_INIT_SA_INIT);
//[-end-160217-IB07400702-add]//
  PERF_START_EX (NULL, NULL, NULL, 0, 0x4080);
  SaInit(FileHandle, PeiServices);
  PERF_END_EX (NULL, NULL, NULL, 0, 0x4081);

  DEBUG ((DEBUG_INFO, "SiInit () - End\n"));
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_SI_INIT_END);  
//[-end-160217-IB07400702-add]//
  return EFI_SUCCESS;
}

#ifndef FSP_FLAG
EFI_STATUS
CpuS3SmmAccessNotifyCallback(
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
)
{
  //
  // Restore Cpu settings only during S3 resume
  //
  S3InitializeCpu((const EFI_PEI_SERVICES **)PeiServices);
  return EFI_SUCCESS;
}
#endif

/** @file
  This file is PeiCpuPolicy library.

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

@par Specification
**/
#include "PeiCpuPolicyLibrary.h"
#include <Library/PreSiliconLib.h>

GLOBAL_REMOVE_IF_UNREFERENCED CONST  CHAR8 SocketNameString[] = "U3E1";
GLOBAL_REMOVE_IF_UNREFERENCED CONST  CHAR8 FillByOemString[]  = "To Be Filled By O.E.M.";

//
// Function call to Load defaults for Individial IP Blocks
//
EFI_STATUS
EFIAPI
LoadCpuPreMemDefault (
  IN   VOID         *ConfigBlockPointer
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
LoadBiosGuardDefault (
  IN   VOID         *ConfigBlockPointer
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
LoadCpuDefault (
  IN   VOID         *ConfigBlockPointer
  )
{
  CPU_CONFIG         *CpuConfig;
  SMBIOS_SOCKET_INFO *SmbiosSocketInfo;
  UINT64             MsrValue;

  CpuConfig     = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "CpuConfig->BlockSize = 0x%x\n", CpuConfig->Header.Size));
  //
  // Policy initialization commented out here is because it's the same with default 0 and no need to re-do again.
  //
  CpuConfig->SmmbaseSwSmiNumber       = SW_SMI_FROM_SMMBASE;
  CpuConfig->CpuRatio                 = 63;
  CpuConfig->HyperThreading           = CPU_FEATURE_DISABLE;
  CpuConfig->ProcTraceEnable          = CPU_FEATURE_DISABLE;
  CpuConfig->VmxEnable                = CPU_FEATURE_ENABLE;
  CpuConfig->SmxEnable                = CPU_FEATURE_ENABLE;
  CpuConfig->MonitorMwaitEnable       = CPU_FEATURE_ENABLE;
  CpuConfig->MachineCheckEnable       = CPU_FEATURE_ENABLE;
  CpuConfig->AesEnable                = CPU_FEATURE_ENABLE;
  CpuConfig->DebugInterfaceLockEnable = CPU_FEATURE_ENABLE;
  CpuConfig->ApIdleManner             = 1;
  CpuConfig->ApHandoffManner          = 1;
  CpuConfig->ActiveProcessorCores     = CPU_FEATURE_ALL_CORES_ENABLE;
  CpuConfig->DisableCore1             = CPU_FEATURE_DISABLE;
  CpuConfig->DisableCore2             = CPU_FEATURE_DISABLE;
  CpuConfig->DisableCore3             = CPU_FEATURE_DISABLE;
  CpuConfig->EnableDts                = CPU_FEATURE_DISABLE;
  CpuConfig->PackageDts               = CPU_FEATURE_DISABLE;

  ///
  /// Processor Trace
  ///
  CpuConfig->ProcTraceMemSize         = EnumProcTraceMemDisable;

  CpuConfig->SmbiosSocketInfo         = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocateZeroPool (sizeof (SMBIOS_SOCKET_INFO));
  SmbiosSocketInfo                        = (SMBIOS_SOCKET_INFO *)(UINTN)CpuConfig->SmbiosSocketInfo;
  if (SmbiosSocketInfo == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  MsrValue = AsmReadMsr64 (MSR_PLATFORM_INFO);
  SmbiosSocketInfo->MaxSpeed          = (100 * (((UINT32) MsrValue >> N_PLATFORM_INFO_MAX_RATIO) & B_PLATFORM_INFO_RATIO_MASK));

  SmbiosSocketInfo->ProcessorUpgrade = ProcessorUpgradeOther;
  CpuConfig->MlcStreamerPrefetcher   = CPU_FEATURE_ENABLE;
  CpuConfig->MlcSpatialPrefetcher    = CPU_FEATURE_ENABLE;


  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
LoadCpuOverclockingDefault (
  IN   VOID         *ConfigBlockPointer
  )
{
  CPU_OVERCLOCKING_CONFIG *CpuOverclockingConfig;

  CpuOverclockingConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "CpuOverclockingConfig->BlockSize = 0x%x\n", CpuOverclockingConfig->Header.Size));
  //
  // Policy initialization commented out here is because it's the same with default 0 and no need to re-do again.
  //

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
LoadPowerMgmtDefault (
  IN   VOID         *ConfigBlockPointer
  )
{
  POWER_MGMT_CONFIG *PowerMgmtConfig;

  PowerMgmtConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "PowerMgmtConfig->BlockSize = 0x%x\n", PowerMgmtConfig->Header.Size));

  ///
  /// Initialize RATL (Runtime Average Temperature Limit) Config
  ///
  PowerMgmtConfig->TccActivationOffset         = 10;

  PowerMgmtConfig->Eist                          = TRUE;
  PowerMgmtConfig->Cx                            = TRUE;
  PowerMgmtConfig->C1e                           = TRUE;
  PowerMgmtConfig->C1AutoDemotion                = TRUE;
  PowerMgmtConfig->C3AutoDemotion                = TRUE;
  PowerMgmtConfig->C1UnDemotion                  = TRUE;
  PowerMgmtConfig->C3UnDemotion                  = TRUE;
  PowerMgmtConfig->BiProcHot                     = TRUE;
  PowerMgmtConfig->DisableProcHotOut             = TRUE;
  PowerMgmtConfig->TurboMode                     = TRUE;
  PowerMgmtConfig->PowerLimit2Enable             = TRUE;
  PowerMgmtConfig->AutoThermalReporting          = TRUE;
  PowerMgmtConfig->ThermalMonitor                = TRUE;
  PowerMgmtConfig->CustomPowerUnit               = PowerUnit125MilliWatts;
  PowerMgmtConfig->PowerLimit1                   = AUTO;
  PowerMgmtConfig->PowerLimit2                   = AUTO;
  PowerMgmtConfig->PowerLimit1Time               = AUTO;
  PowerMgmtConfig->PowerLimit3                   = AUTO;
  PowerMgmtConfig->PowerLimit4                   = AUTO;
  PowerMgmtConfig->PowerLimit3Time               = AUTO;
  PowerMgmtConfig->PowerLimit3DutyCycle          = AUTO;
  PowerMgmtConfig->PowerLimit3Lock               = TRUE;
  PowerMgmtConfig->PlatformPowerLimit1Power      = AUTO;
  PowerMgmtConfig->PlatformPowerLimit1Time       = AUTO;
  PowerMgmtConfig->PlatformPowerLimit2Power      = AUTO;
  PowerMgmtConfig->DdrPowerLimit                 = AUTO;
  PowerMgmtConfig->PmgCstCfgCtrlLock             = TRUE;
  PowerMgmtConfig->S3RestoreMsrSwSmiNumber       = SW_SMI_S3_RESTORE_MSR;
  PowerMgmtConfig->PkgCStateLimit                = PkgCpuDefault;
  PowerMgmtConfig->TurboPowerLimitLock           = FALSE;
  PowerMgmtConfig->ProcHotLock                   = FALSE;
  PowerMgmtConfig->RatioLimitProgramable         = FALSE;
  PowerMgmtConfig->TStates                       = TRUE;
  PowerMgmtConfig->SkipSetBootPState             = CPU_FEATURE_DISABLE;
  PowerMgmtConfig->VrConfig                      = CPU_FEATURE_DISABLE;
  PowerMgmtConfig->PowerLimit1Enable             = TRUE;
  PowerMgmtConfig->PowerLimit1ClampEnable        = TRUE;
  PowerMgmtConfig->CustomPowerLimit1             = AUTO;
  PowerMgmtConfig->CustomPowerLimit1Time         = AUTO;
  PowerMgmtConfig->PmgCstCfgCtrIoMwaitRedirection  = CPU_FEATURE_DISABLE;
  return EFI_SUCCESS;
}



/**
  LoadCpuConfigBlockDefault - Initialize default settings for each CPU Config block

  @param[in] ConfigBlockPointer         The buffer pointer that will be initialized as specific config block
  @param[in] BlockId                    Request to initialize defaults of specified config block by given Block ID

  @retval EFI_SUCCESS                   The given buffer has contained the defaults of requested config block
  @retval EFI_NOT_FOUND                 Block ID is not defined so no default Config block will be initialized
**/
EFI_STATUS
EFIAPI
LoadCpuConfigBlockDefault (
  IN   VOID          *ConfigBlockPointer
  )
{
  CONFIG_BLOCK *ConfigBlkPtr;

  ConfigBlkPtr = (CONFIG_BLOCK *) ConfigBlockPointer;

 if (CompareGuid (&(ConfigBlkPtr->Header.Guid), &gCpuConfigPreMemGuid)) {
   LoadCpuPreMemDefault (ConfigBlockPointer);
 }
 if (CompareGuid (&(ConfigBlkPtr->Header.Guid), &gBiosGuardConfigGuid)) {
   LoadBiosGuardDefault(ConfigBlockPointer);
 }
 if (CompareGuid (&(ConfigBlkPtr->Header.Guid), &gCpuConfigGuid)) {
   LoadCpuDefault (ConfigBlockPointer);
 }
 if (CompareGuid (&(ConfigBlkPtr->Header.Guid), &gCpuOverclockingConfigGuid)) {
   LoadCpuOverclockingDefault (ConfigBlockPointer);
 }
 if (CompareGuid (&(ConfigBlkPtr->Header.Guid), &gPowerMgmtConfigGuid)) {
   LoadPowerMgmtDefault(ConfigBlockPointer);
 }


  return EFI_SUCCESS;
}

/**
  CreateCpuConfigBlocks creates the default setting of PEI Cpu Policy.
  It allocates and zero out buffer, and fills in the Intel default settings.

  @param[out] SiCpuPolicyPpi            The pointer to get PEI Cpu Policy PPI instance

  @retval EFI_SUCCESS                   The policy default is initialized.
  @retval EFI_OUT_OF_RESOURCES          Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
CreateCpuConfigBlocks (
  OUT  SI_CPU_POLICY_PPI      **SiCpuPolicyPpi
  )
{
  SI_CPU_POLICY_PPI           *SiCpuPolicy;
  UINT32                      ConfigBlockTableTotalSize;
  UINT8                       TotalBlockCount;
  EFI_STATUS                  Status;
  UINT8                       BlockCount;
  VOID                        *ConfigBlockPointer;
  CONFIG_BLOCK_HEADER mBxtCpuIpBlocksPostMem [4];

  PostCode (0xC00);
  CopyMem (&mBxtCpuIpBlocksPostMem[0].Guid, &gBiosGuardConfigGuid, sizeof (EFI_GUID));
  CopyMem (&mBxtCpuIpBlocksPostMem[1].Guid, &gCpuConfigGuid, sizeof (EFI_GUID));
  CopyMem (&mBxtCpuIpBlocksPostMem[2].Guid, &gCpuOverclockingConfigGuid, sizeof (EFI_GUID));
  CopyMem (&mBxtCpuIpBlocksPostMem[3].Guid, &gPowerMgmtConfigGuid, sizeof (EFI_GUID));


  mBxtCpuIpBlocksPostMem[0].Size = sizeof(BIOS_GUARD_CONFIG);
  mBxtCpuIpBlocksPostMem[1].Size = sizeof(CPU_CONFIG);
  mBxtCpuIpBlocksPostMem[2].Size = sizeof(CPU_OVERCLOCKING_CONFIG);
  mBxtCpuIpBlocksPostMem[3].Size = sizeof(POWER_MGMT_CONFIG);
  mBxtCpuIpBlocksPostMem[0].Revision = BIOS_GUARD_CONFIG_REVISION;
  mBxtCpuIpBlocksPostMem[1].Revision = CPU_CONFIG_REVISION;
  mBxtCpuIpBlocksPostMem[2].Revision = CPU_OVERCLOCKING_CONFIG_REVISION;
  mBxtCpuIpBlocksPostMem[3].Revision = POWER_MGMT_CONFIG_REVISION;

  TotalBlockCount = sizeof(mBxtCpuIpBlocksPostMem) / sizeof (CONFIG_BLOCK_HEADER);

  ConfigBlockTableTotalSize = sizeof(CONFIG_BLOCK_TABLE_HEADER) + TotalBlockCount * 4;
  for(BlockCount = 0; BlockCount < TotalBlockCount; BlockCount++){
    ConfigBlockTableTotalSize += (UINT32)mBxtCpuIpBlocksPostMem[BlockCount].Size;
  }

  ///
  /// Allocate memory for the CPU Policy Ppi and Descriptor
  ///
  DEBUG ((DEBUG_INFO, "CreateCpuConfigBlocksPostMem Start\n"));
  Status =  CreateConfigBlockTable ((VOID *)&SiCpuPolicy, TotalBlockCount, ConfigBlockTableTotalSize);
  if (SiCpuPolicy == NULL) {
    ASSERT (SiCpuPolicy != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize Policy Revision
  //
  SiCpuPolicy->ConfigBlockTableHeader.Header.Revision = SI_CPU_POLICY_PPI_REVISION;

  //
  // Initialize ConfigBlockPointer to NULL
  //
  ConfigBlockPointer = NULL;

  ///
  /// Put IP_BLOCK_STRUCT and target structure into the right place in SI_CPU_POLICY_PPI
  ///
  for(BlockCount = 0; BlockCount < TotalBlockCount; BlockCount++){
    ConfigBlockPointer = (VOID *) &mBxtCpuIpBlocksPostMem [BlockCount];
    Status = AddConfigBlock ((VOID *) SiCpuPolicy, (VOID *) &ConfigBlockPointer);
    ASSERT_EFI_ERROR (Status);
    LoadCpuConfigBlockDefault((VOID *)ConfigBlockPointer);
  }

  //
  // Assignment for returning CpuInitPolicy config block base address
  //
  *SiCpuPolicyPpi = SiCpuPolicy;
  PostCode (0xC0F);
  return EFI_SUCCESS;
}

/**
  CpuInstallPolicyPpi installs SiCpuPolicyPpi.
  While installed, RC assumes the Policy is ready and finalized. So please update and override
  any setting before calling this function.

  @param[in] SiCpuPolicyPpi             The pointer to PEI Cpu Policy PPI instance

  @retval EFI_SUCCESS                   The policy is installed.
  @retval EFI_OUT_OF_RESOURCES          Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
CpuInstallPolicyPpi (
  IN  SI_CPU_POLICY_PPI                 *SiCpuPolicyPpi
  )
{
  EFI_STATUS                  Status;
  EFI_PEI_PPI_DESCRIPTOR      *NewSiCpuPolicyPpiDesc;
  EFI_PEI_PPI_DESCRIPTOR      *OldSiCpuPolicyPpiDesc;
  SI_CPU_POLICY_PPI           *OldSiCpuPolicy;


  NewSiCpuPolicyPpiDesc = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
  if (NewSiCpuPolicyPpiDesc == NULL) {
    ASSERT (NewSiCpuPolicyPpiDesc != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  NewSiCpuPolicyPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  NewSiCpuPolicyPpiDesc->Guid  = &gSiCpuPolicyPpiGuid;
  NewSiCpuPolicyPpiDesc->Ppi   = SiCpuPolicyPpi;

  //
  // Print whole SI_CPU_POLICY_PPI and serial out in PostMem.
  //
  CpuPrintPolicyPpi (SiCpuPolicyPpi);
  //
  // Locate Cpu Policy Ppi
  //
  Status = PeiServicesLocatePpi (
             &gSiCpuPolicyPpiGuid,
             0,
             &OldSiCpuPolicyPpiDesc,
             (VOID **) &OldSiCpuPolicy
             );
  if (EFI_ERROR (Status)) {
    //
    // Install PEI Cpu Policy PPI
    //
    DEBUG ((DEBUG_ERROR, "Locate Old Si CPU Policy Ppi fail in Post-Memory\n"));
    Status = PeiServicesInstallPpi (NewSiCpuPolicyPpiDesc);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Install Si CPU Policy Ppi fail in Post-Memory\n"));
    }
    ASSERT_EFI_ERROR (Status);
  } else {
    //
    // ReInstall PEI Cpu Policy PPI
    //
    DEBUG ((EFI_D_INFO, "Re-Install Si CPU Policy Ppi in Post-Memory\n"));
    Status = PeiServicesReInstallPpi (OldSiCpuPolicyPpiDesc, NewSiCpuPolicyPpiDesc);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Re-Install Si CPU Policy Ppi fail in Post-Memory\n"));
    }
    ASSERT_EFI_ERROR (Status);
  }
  return Status;
}

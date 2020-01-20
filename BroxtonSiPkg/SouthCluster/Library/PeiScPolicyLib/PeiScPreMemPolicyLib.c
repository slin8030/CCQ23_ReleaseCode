/** @file
  This file is PeiScPreMemPolicy library.

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

#include "PeiScPolicyLibrary.h"

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadLpcPreMemConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  SC_LPC_PREMEM_CONFIG  *LpcPreMemConfig;
  LpcPreMemConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "LpcPreMemConfig->Header.Guid = %g\n", &LpcPreMemConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "LpcPreMemConfig->Header.Size = 0x%x\n", LpcPreMemConfig->Header.Size));

  /********************************
    LPC Configuration
  ********************************/
  LpcPreMemConfig->EnhancePort8xhDecoding = TRUE;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadPciePreMemConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  SC_PCIE_PREMEM_CONFIG  *PciePreMemConfig;
  PciePreMemConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "PciePreMemConfig->Header.Guid = %g\n", &PciePreMemConfig->Header.Guid));
  DEBUG ((DEBUG_INFO, "PciePreMemConfig->Header.Size = 0x%x\n", PciePreMemConfig->Header.Size));

  /********************************
    PCIe Configuration
  ********************************/
}

static IP_BLOCK_ENTRY  mScIpBlocks [] = {
  {&gPcieRpPreMemConfigGuid,     sizeof (SC_PCIE_PREMEM_CONFIG),      PCIE_RP_PREMEM_CONFIG_REVISION,       LoadPciePreMemConfigDefault},
  {&gLpcPreMemConfigGuid,        sizeof (SC_LPC_PREMEM_CONFIG),       LPC_PREMEM_CONFIG_REVISION,           LoadLpcPreMemConfigDefault},
};

/**
  Get SC PREMEM config block table total size.

  @retval                               Size of SC PREMEM config block table
**/
UINT32
EFIAPI
ScGetPreMemConfigBlockTotalSize (
  VOID
  )
{
  UINT32            TotalBlockCount;
  UINT32            TotalBlockSize;
  UINT32            ConfigBlockHdrSize;
  UINT32            BlockCount;

  TotalBlockCount = sizeof(mScIpBlocks) / sizeof (IP_BLOCK_ENTRY);
  TotalBlockSize = 0;
  for (BlockCount = 0 ; BlockCount < TotalBlockCount; BlockCount++) {
    TotalBlockSize += (UINT32)mScIpBlocks[BlockCount].Size;
    DEBUG ((DEBUG_INFO, "TotalBlockSize after adding Block[0x%x]= 0x%x\n", BlockCount, TotalBlockSize));
  }
  ConfigBlockHdrSize = GetSizeOfConfigBlockTableHeaders ((UINT16)TotalBlockCount);
  //
  // Because CreateConfigBlockTable has the padding for each config block,
  // we need extra size, which is TotalBlockCount * 3, to create the table
  //
  return ConfigBlockHdrSize + TotalBlockSize + (TotalBlockCount * 3);
}

/**
  CreatePreMemConfigBlocks generates the config blocks of SC Policy.
  It allocates and zero out buffer, and fills in the Intel default settings.

  @param[out] ScPreMemPolicyPpi        The pointer to get SC PREMEM Policy PPI instance

  @retval EFI_SUCCESS                   The policy default is initialized.
  @retval EFI_OUT_OF_RESOURCES          Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
ScCreatePreMemConfigBlocks (
  OUT  SC_PREMEM_POLICY_PPI            **ScPreMemPolicyPpi
  )
{
  UINT16                    TotalBlockCount;
  UINT16                    BlockCount;
  VOID                      *ConfigBlockPointer;
  CONFIG_BLOCK              ConfigBlockBuf;
  EFI_STATUS                Status;
  SC_PREMEM_POLICY_PPI      *InitPolicy;
  UINT32                    RequiredSize;

  DEBUG ((DEBUG_INFO, "SC CreateConfigBlocks\n"));

  InitPolicy = NULL;
  TotalBlockCount = sizeof(mScIpBlocks) / sizeof (IP_BLOCK_ENTRY);
  DEBUG ((DEBUG_INFO, "TotalBlockCount = 0x%x\n", TotalBlockCount));

  RequiredSize = ScGetPreMemConfigBlockTotalSize ();

  Status = CreateConfigBlockTable ((VOID *)&InitPolicy, TotalBlockCount, RequiredSize);
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize Policy Revision
  //
  InitPolicy->TableHeader.Header.Revision = SC_PREMEM_POLICY_REVISION;
  //
  // Initialize ConfigBlockPointer to NULL
  //
  ConfigBlockPointer = NULL;
  //
  // Loop to identify each config block from mScIpBlocks[] Table and add each of them
  //
  for (BlockCount = 0 ; BlockCount < TotalBlockCount; BlockCount++) {
    CopyMem (&(ConfigBlockBuf.Header.Guid), mScIpBlocks[BlockCount].Guid, sizeof (EFI_GUID));
    ConfigBlockBuf.Header.Size     = mScIpBlocks[BlockCount].Size;
    ConfigBlockBuf.Header.Revision = mScIpBlocks[BlockCount].Revision;
    ConfigBlockPointer             = (VOID *)&ConfigBlockBuf;
    Status = AddConfigBlock ((VOID *)InitPolicy, (VOID *)&ConfigBlockPointer);
    ASSERT_EFI_ERROR (Status);
    mScIpBlocks[BlockCount].LoadDefault (ConfigBlockPointer);
  }
  //
  // Assignment for returning Policy config block base address
  //
  *ScPreMemPolicyPpi = InitPolicy;
  return EFI_SUCCESS;
}

/**
  ScInstallPreMemPolicyPpi installs PchPolicyPpi.
  While installed, RC assumes the Policy is ready and finalized. So please update and override
  any setting before calling this function.

  @param[in] ScPreMemPolicyPpi         The pointer to SC PREMEM Policy PPI instance

  @retval EFI_SUCCESS                   The policy is installed.
  @retval EFI_OUT_OF_RESOURCES          Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
ScInstallPreMemPolicyPpi (
  IN  SC_PREMEM_POLICY_PPI             *ScPreMemPolicyPpi
  )
{
  EFI_STATUS                            Status;
  EFI_PEI_PPI_DESCRIPTOR                *ScPreMemPolicyPpiDesc;

  ScPreMemPolicyPpiDesc = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
  if (ScPreMemPolicyPpiDesc == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }

  ScPreMemPolicyPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  ScPreMemPolicyPpiDesc->Guid  = &gScPreMemPolicyPpiGuid;
  ScPreMemPolicyPpiDesc->Ppi   = ScPreMemPolicyPpi;

  //
  // Print whole SC_PREMEM_POLICY_PPI and serial out.
  //
  ScPreMemPrintPolicyPpi (ScPreMemPolicyPpi);

  //
  // Install PREMEM Policy PPI
  //
  Status = PeiServicesInstallPpi (ScPreMemPolicyPpiDesc);
  ASSERT_EFI_ERROR (Status);
  return Status;
}

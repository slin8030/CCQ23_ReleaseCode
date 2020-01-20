/** @file
  Framework PEIM to initialize memory on a Memory Controller.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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

#ifndef _MEMORY_INIT_H_
#define _MEMORY_INIT_H_

#include "Mrc.h"
#include "McFunc.h"
#include "OemHooks.h"
#include "Bdat.h"

#include "MmrcLibraries.h"
#include "SsaResults.h"

#define MRC_PEI_REPORT_PROGRESS_CODE(PeiServices, Code) \
        (*PeiServices)->ReportStatusCode(PeiServices, EFI_PROGRESS_CODE, Code, 0, NULL, NULL)
#define MRC_PEI_REPORT_ERROR_CODE(PeiServices, Code, Severity)\
        (*PeiServices)->ReportStatusCode(PeiServices, EFI_ERROR_CODE|(Severity), Code, 0, NULL, NULL)
//
// MRC Variable Attributes
//
#define MEM_DET_COMMON_MEM_ATTR \
          (EFI_RESOURCE_ATTRIBUTE_PRESENT                 | \
           EFI_RESOURCE_ATTRIBUTE_INITIALIZED             | \
           EFI_RESOURCE_ATTRIBUTE_TESTED                  | \
           EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             | \
           EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       | \
           EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | \
           EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE)

#define MEM_DET_COMMON_MEM_ATTR1 \
          (EFI_RESOURCE_ATTRIBUTE_PRESENT                 | \
           EFI_RESOURCE_ATTRIBUTE_INITIALIZED             | \
           EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             | \
           EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       | \
           EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | \
           EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE)

/**
  This function installs the EFI Memory

  @param[in]      PeiServices   PEI Services Table.
  @param[in]      BootMode      Indicates the boot mode selected
  @param[in,out]  MrcData       Pointer to MRC Output Data that contains MRC Parameters

  @retval  EFI_SUCCESS      When no errors when installing memory
  @retval  Others           Errors occurred while installing memory
**/
EFI_STATUS
InstallEfiMemory (
  IN        EFI_PEI_SERVICES      **PeiServices,
  IN        EFI_BOOT_MODE         BootMode,
  IN  OUT   MMRC_DATA   *MrcData
)
;

/**
  This function installs the EFI Memory when S3 path is executed

  @param[in]      PeiServices  PEI Services Table.
  @param[in,out]  MrcData      Pointer to MRC Output Data that contains MRC Parameters.

  @retval  EFI_SUCCESS         When no errors when installing memory
  @retval  Others              Errors occurred while installing memory
**/
EFI_STATUS
InstallS3Memory (
  IN        EFI_PEI_SERVICES      **PeiServices,
  IN  OUT   MMRC_DATA   *MrcData
)
;

/**
  This function restores the MRC parameters previously saved and configured in the Setup Options

  @param  PeiServices    PEI Services Table.
  @param  MrcData        Pointer to MRC Output Data that contains MRC Parameters.

  @retval EFI_SUCCESS    MRC parameters restored correctly
  @retval Others         Otherwise
**/
EFI_STATUS
MrcParamsRestore (
  IN        EFI_PEI_SERVICES      **PeiServices,
  IN  OUT   MMRC_DATA   *MrcData
)
;

/**
  This function saves the MRC Parameters to Hob for later use.

  @param[in]   PeiServices   PEI Services Table.
  @param[out]  MrcData       Pointer to MRC Output Data that contains MRC Parameters

  @retval  EFI_SUCCESS  Hob is successfully built.
  @retval  Others       Errors occur while creating new Hob
**/
EFI_STATUS
MrcParamsSave (
  IN        EFI_PEI_SERVICES      **PeiServices,
  IN  OUT   MMRC_DATA   *MrcData
)
;

/**
  This overrides the MRC Timing Parameters with setup values.

  @param[in]   PeiServices   PEI Services Table.
  @param[out]  MrcData       Pointer to MRC Output Data that contains MRC Parameters

  @retval  EFI_SUCCESS  Setup correctly done.
  @retval  Others       Otherwise
**/
EFI_STATUS
GetTimingOverrides (
  IN        EFI_PEI_SERVICES      **PeiServices,
  IN  OUT   MMRC_DATA   *MrcData
)
;

#if MEM_SV_HOOKS
/**
  This configures the checkpoints using the Port: 01h Offset: 4Bh Sticky Scratch Pad register 1 (SSKPD1)

  @param[in]      PeiServices   PEI Services Table.
  @param[in,out]  MrcData       Pointer to MRC Output Data that contains MRC Parameters.

  @retval  EFI_SUCCESS       Checkpoint configured correctly
  @retval  Others            Otherwise
**/
EFI_STATUS
CheckpointConfigure (
  IN        EFI_PEI_SERVICES      **PeiServices,
  IN        MMRC_DATA   *MrcData
)
;
#endif // MEM_SV_HOOKS

/**
  Configure the memory thru several training steps

  @param[in,out]  MrcData       Host structure for all MRC global data.

  @retval  EFI_SUCCESS
  @retval  Others
**/
MMRC_STATUS
ConfigureMemory (
  IN  OUT   MMRC_DATA           *MrcData
)
;


#endif

/**
  This function configures the Lakemore ODLA

  @param[in]  MrcData          Host structure for all MRC global data.
  @param[in]  LakemoreBarSize  Memory space reserved for Lakemore
  @param[in]  LakemoreBarAddr  Bar Address used for Lakemore

  @retval  EFI_SUCCESS
  @retval  Others
**/
MMRC_STATUS
LakemoreConfiguration (
  IN        MMRC_DATA   *MrcData,
  IN        UINT64                LakemoreBarSize,
  IN        UINT64                LakemoreBarAddr
)
;

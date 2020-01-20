/** @file
  CPU policy PPI produced by a platform driver specifying various
  expected CPU settings. This PPI is consumed by CPU PEI modules.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2009 - 2016 Intel Corporation.

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
#ifndef _CPU_POLICY_PPI_H_
#define _CPU_POLICY_PPI_H_

#include <ConfigBlock.h>
#include <CpuAccess.h>
#include <CpuDataStruct.h>
#include <Ppi/BiosGuardConfig.h>
#include <Ppi/CpuConfigPreMem.h>
#include <Ppi/CpuConfig.h>
#include <Ppi/CpuOverclockingConfig.h>
#include <Ppi/PowerMgmtConfig.h>
#include <Ppi/TxtConfig.h>
//
// Extern the GUID for protocol users.
//
extern EFI_GUID gSiCpuPolicyPpiGuid;

#pragma pack(1)


//
// SI_CPU_POLICY_PPI revisions
//
#define SI_CPU_POLICY_PPI_REVISION  1


/**
  The PPI allows the platform code to publish a set of configuration information that the
  CPU drivers will use to configure the processor in the PEI phase.
  This Policy PPI needs to be initialized for CPU configuration.
  @note The PPI has to be published before processor PEIMs are dispatched.
**/
typedef struct {
  CONFIG_BLOCK_TABLE_HEADER ConfigBlockTableHeader;
  //CONFIG_BLOCK_OFFSET_TABLE OffsetTable; ///< Offset 32 config Block Offset Table (N*4)
//
// Individual Config Block Structures are added here in memory as part of AddConfigBlock()
//

} SI_CPU_POLICY_PPI;

#pragma pack()
#endif

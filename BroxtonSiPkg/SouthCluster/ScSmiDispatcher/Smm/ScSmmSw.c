/** @file
@brief
  File to contain all the hardware specific stuff for the Smm Sw dispatch protocol.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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
#include "ScSmmHelpers.h"
#include "PlatformBaseAddresses.h"
#include <Protocol/SmmCpu.h>
//
// There is only one instance for SwCommBuffer.
// It's safe in SMM since there is no re-entry for the function.
//
EFI_SMM_SW_CONTEXT            mScSwCommBuffer;
EFI_SMM_CPU_PROTOCOL          *mSmmCpuProtocol;

CONST SC_SMM_SOURCE_DESC SW_SOURCE_DESC = {
  SC_SMM_NO_FLAGS,
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_SMI_EN
      },
      S_SMI_EN,
      N_SMI_EN_APMC
    },
    NULL_BIT_DESC_INITIALIZER
  },
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_SMI_STS
      },
      S_SMI_STS,
      N_SMI_STS_APM
    }
  }
};

/**

  @brief
  Get the Software Smi value

  @param[in] Record               No use
  @param[in] Context              The context that includes Software Smi value to be filled

  @retval None

**/
VOID
EFIAPI
SwGetContext (
  IN  DATABASE_RECORD    *Record,
  OUT SC_SMM_CONTEXT    *Context
  )
{
  UINT8 ApmCnt;

  ApmCnt                      = IoRead8 ((UINTN) R_APM_CNT);

  Context->Sw.SwSmiInputValue = ApmCnt;
}

/**

  @brief
  Check whether software SMI value of two contexts match

  @param[in] Context1             Context 1 that includes software SMI value 1
  @param[in] Context2             Context 2 that includes software SMI value 2

  @retval FALSE                   Software SMI value match
  @retval TRUE                    Software SMI value don't match

**/
BOOLEAN
EFIAPI
SwCmpContext (
  IN SC_SMM_CONTEXT     *Context1,
  IN SC_SMM_CONTEXT     *Context2
  )
{
  return (BOOLEAN) (Context1->Sw.SwSmiInputValue == Context2->Sw.SwSmiInputValue);
}
/**
  Gather the CommBuffer information of SmmSwDispatch2.

  @param[in]  Record              No use
  @param[out] CommBuffer          Point to the CommBuffer structure
  @param[out] CommBufferSize      Point to the Size of CommBuffer structure

**/
VOID
EFIAPI
SwGetCommBuffer (
  IN  DATABASE_RECORD    *Record,
  OUT VOID               **CommBuffer,
  OUT UINTN              *CommBufferSize
  )
{
  EFI_STATUS                            Status;
  EFI_SMM_SAVE_STATE_IO_INFO            SmiIoInfo;
  UINTN                                 Index;

  ASSERT (Record->ProtocolType == SwType);

  mScSwCommBuffer.CommandPort = IoRead8 (R_APM_CNT);
  mScSwCommBuffer.DataPort    = IoRead8 (R_APM_STS);

  //
  // Try to find which CPU trigger SWSMI
  //
  mScSwCommBuffer.SwSmiCpuIndex = 0;
  for (Index = 0; Index < gSmst->NumberOfCpus; Index++) {
    Status = mSmmCpuProtocol->ReadSaveState (
                                mSmmCpuProtocol,
                                sizeof(EFI_SMM_SAVE_STATE_IO_INFO),
                                EFI_SMM_SAVE_STATE_REGISTER_IO,
                                Index,
                                &SmiIoInfo
                                );
    if (EFI_ERROR (Status)) {
      continue;
    }
    if (SmiIoInfo.IoPort == R_APM_CNT) {
      //
      // Find matched CPU.
      //
      mScSwCommBuffer.SwSmiCpuIndex = Index;
      break;
    }
  }

  ///
  /// Return the CommBuffer
  ///
  *CommBuffer = (VOID *) &mScSwCommBuffer;
  *CommBufferSize  = sizeof (EFI_SMM_SW_CONTEXT);
}

/**
  Init required protocol for Sc Sw Dispatch protocol.


**/
VOID
ScSwDispatchInit (
  VOID
  )
{
  EFI_STATUS                            Status;
  //
  // Locate PI SMM CPU protocol
  //
  Status = gSmst->SmmLocateProtocol (&gEfiSmmCpuProtocolGuid, NULL, (VOID **)&mSmmCpuProtocol);
  ASSERT_EFI_ERROR (Status);
}

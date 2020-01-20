/** @file
  Some definitions for MP services Ppi.

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

@par Specification
**/
#ifndef _MP_SERVICE_H_
#define _MP_SERVICE_H_

#include <Library/ReportStatusCodeLib.h>
#include <Private/Library/MpServiceLib.h>

///
/// Combine f(FamilyId), m(Model), s(SteppingId) to a single 32 bit number
///
#define EfiMakeCpuVersion(f, m, s)  (((UINT32) (f) << 16) | ((UINT32) (m) << 8) | ((UINT32) (s)))

/**
  Get CPU platform features settings to fill MP data.

  @param[in] WakeUpBuffer             - The address of wakeup buffer.
  @param[in] StackAddressStart        - The start address of APs's stacks.
  @param[in] MaximumCPUsForThisSystem - Maximum CPUs in this system.

  @retval EFI_SUCCESS              - Function successfully executed.
  @retval Other                    - Error occurred while allocating memory.
**/
EFI_STATUS
EFIAPI
FillMpData (
  IN UINTN        WakeUpBuffer,
  IN VOID         *StackAddressStart,
  IN UINTN        MaximumCPUsForThisSystem
  );

/**
  This function is called by all processors (both BSP and AP) once and collects MP related data

  @param[in] BSP           - TRUE if the CPU is BSP
  @param[in] BistParam     - BIST (build-in self test) data for the processor. This data
                             is only valid for processors that are waked up for the 1st
                             time in this CPU DXE driver.

  @retval EFI_SUCCESS   - Data for the processor collected and filled in
**/
EFI_STATUS
FillInProcessorInformation (
  IN BOOLEAN        BSP,
  IN UINT32         BistParam
  );

/**
  Notification function that gets called once permanent memory installed to take care
  of MP CPU related activities in PEI phase

  @param[in] PeiServices   - Indirect reference to the PEI Services Table
  @param[in] NotifyDesc    - Pointer to the descriptor for the Notification event that
                             caused this function to execute.
  @param[in] Ppi           - Pointer to the PPI data associated with this function.

  @retval EFI_SUCCESS      - Multiple processors are intialized successfully
**/
EFI_STATUS
InitializeMpSupport (
  IN EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  );

/**
  Re-load microcode patch.

  @retval EFI_SUCCESS         - Multiple processors re-load microcode patch
**/
EFI_STATUS
ReloadMicrocode (
  IN MP_CPU_EXCHANGE_INFO     *ExchangeInfo
  );

/**
  Get processor feature

  @param[in] Features  - pointer to a buffer which stores feature information
**/
VOID
EFIAPI
GetProcessorFeatures (
  IN UINT32 *Features
  );

/**
  Switch BSP to the processor which has least features

  @retval EFI_STATUS  - status code returned from each sub-routines
**/
EFI_STATUS
EFIAPI
SwitchToLowestFeatureProcess (
  VOID
  );

/**
  Find out the common features supported by all core/threads
**/
VOID
EFIAPI
GetProcessorCommonFeature (
  VOID
  );

/**
  Get the processor data with least features
**/
VOID
EFIAPI
GetProcessorWithLeastFeature (
  VOID
  );

/**
  Extract CPU detail version infomation

  @param[in] FamilyId   - FamilyId, including ExtendedFamilyId
  @param[in] Model      - Model, including ExtendedModel
  @param[in] SteppingId - SteppingId
  @param[in] Processor  - Processor

**/
VOID
EFIAPI
EfiCpuVersion (
  IN  OUT UINT16  *FamilyId,    OPTIONAL
  IN  OUT UINT8   *Model,       OPTIONAL
  IN  OUT UINT8   *SteppingId,  OPTIONAL
  IN  OUT UINT8   *Processor    OPTIONAL
  );

/**
  Update some processor info into LEAST_FEATURE_PROC data structure.

  @param[in] Index                 - indicate which processor calling this routine
  @param[in] LeastFeatureProcessor - the data structure that will be updated
**/
VOID
EFIAPI
UpdateProcessorInfo (
  IN UINTN              Index,
  IN LEAST_FEATURE_PROC *LeastFeatureProcessor
  );

/**
  Get processor feature delta

  @param[in] FeaturesInput   - Supported features for input processor
  @param[in] CommonFeatures  - Supported features for processor (subset of FeaturesInput)

  @retval The least of processor features
**/
UINT32
EFIAPI
GetProcessorFeatureDelta (
  IN UINT32 *FeaturesInput,
  IN UINT32 *CommonFeatures
  );

/**
  Calculate how many bits are one from given number

  @param[in] Value - number that will be calculated bits

  @retval Number of bits
**/
UINT32
EFIAPI
GetBitsNumberOfOne (
  IN UINT32 Value
  );

/**
  Exchange 2 processors (BSP to AP or AP to BSP)

  @param[in] MyInfo      - CPU info for current processor
  @param[in] OthersInfo  - CPU info that will be exchanged with
**/
VOID
AsmExchangeRole (
  IN CPU_EXCHANGE_ROLE_INFO *MyInfo,
  IN CPU_EXCHANGE_ROLE_INFO *OthersInfo
  );
#endif

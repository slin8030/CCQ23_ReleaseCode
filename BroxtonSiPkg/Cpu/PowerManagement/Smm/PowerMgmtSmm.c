/** @file
  This is the SMM driver for saving and restoring the powermanagement related MSRs

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
#include "PowerMgmtSmm.h"

///
/// Global variables
///
GLOBAL_REMOVE_IF_UNREFERENCED CPU_CONFIG                       *mCpuConfig;
GLOBAL_REMOVE_IF_UNREFERENCED POWER_MGMT_CONFIG                *mPowerMgmtConfig;

/**
  Initialize the SMM power management Handler.

  @param[in] ImageHandle   - Pointer to the loaded image protocol for this driver
  @param[in] SystemTable   - Pointer to the EFI System Table

  @retval EFI_SUCCESS   The driver installes/initialized correctly.
  @retval EFI_NOT_FOUND CPU Data HOB not available.
**/
EFI_STATUS
EFIAPI
PowerMgmtSmmEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{

  EFI_STATUS                        Status;
  EFI_HANDLE                        SwHandle;
  EFI_SMM_SW_REGISTER_CONTEXT       SwContext;
  EFI_SMM_SW_DISPATCH2_PROTOCOL     *SwDispatch;
  VOID                              *Hob;
  CPU_INIT_DATA_HOB                 *mCpuInitDataHob;

  SwHandle = 0;
  DEBUG ((DEBUG_INFO, " PpmSmmEntryPoint Started\n"));

  ///
  /// Get CPU Init Data Hob
  ///
  Hob = GetFirstGuidHob (&gCpuInitDataHobGuid);
  if (Hob == NULL) {
    DEBUG ((DEBUG_ERROR, "CPU Data HOB not available\n"));
    return EFI_NOT_FOUND;
  }
  mCpuInitDataHob = (CPU_INIT_DATA_HOB *) ((UINTN) Hob + sizeof (EFI_HOB_GUID_TYPE));
  mCpuConfig       = (CPU_CONFIG *)(UINTN)mCpuInitDataHob->CpuConfig;
  mPowerMgmtConfig = (POWER_MGMT_CONFIG *) (UINTN)mCpuInitDataHob->PowerMgmtConfig;

  ///
  /// Locate the ICH SMM SW dispatch protocol
  ///
  Status = gSmst->SmmLocateProtocol (
                        &gEfiSmmSwDispatch2ProtocolGuid,
                        NULL,
                        (VOID**) &SwDispatch
                        );

  ///
  /// Register ACPI S3 MSR restore handler
  ///
  SwContext.SwSmiInputValue = PcdGet8(PcdS3RestoreMsrSwSmiNumber);

  Status = SwDispatch->Register (
                  SwDispatch,
                  (EFI_SMM_HANDLER_ENTRY_POINT2) S3RestoreMsr,
                  &SwContext,
                  &SwHandle
                  );
  ASSERT_EFI_ERROR (Status);

  ///
  ///  Install Digital Thermal Sensor
  ///
  if (mCpuConfig->EnableDts > 0) {
    DEBUG ((DEBUG_INFO, " Install Digital Thermal Sensor \n"));
    InstallDigitalThermalSensor();
  }
  ///
  ///  Save MSRs for S3 Resume.
  ///
  DEBUG ((DEBUG_INFO, " Saving Processor MSR for S3 Resume \n"));

  S3SaveMsr ();

  return EFI_SUCCESS;
}

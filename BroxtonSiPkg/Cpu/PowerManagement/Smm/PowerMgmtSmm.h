/** @file
  Header file for PowerMgmt Smm Driver.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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
#ifndef _POWER_MGMT_SMM_H_
#define _POWER_MGMT_SMM_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/CpuPlatformLib.h>
#include <Private/CpuInitDataHob.h>
#include <Private/Library/CpuCommonLib.h>
#include <CpuAccess.h>
#include <ScAccess.h>
#include <Private/PowerMgmtNvsStruct.h>

#define PPM_WAIT_PERIOD 15


extern POWER_MGMT_CONFIG                *PowerMgmtConfig;
extern UINT8                            mDtsValue;

typedef struct _EFI_MSR_VALUES {
  UINT16  Index;
  UINT64  Value;
  UINT64  BitMask;
  BOOLEAN RestoreFlag;
} EFI_MSR_VALUES;

/**
  Save processor MSR runtime settings for S3.

  @retval EFI_SUCCESS            Processor MSR setting is saved.
**/
EFI_STATUS
S3SaveMsr (
  VOID
  );

/**
  Runs the specified procedure on all logical processors, passing in the
  parameter buffer to the procedure.

  @param[in, out] Procedure     The function to be run.
  @param[in, out] Buffer        Pointer to a parameter buffer.

  @retval EFI_SUCCESS
**/
EFI_STATUS
RunOnAllLogicalProcessors (
  IN OUT EFI_AP_PROCEDURE Procedure,
  IN OUT VOID             *Buffer
  );

/**
  This function will restore MSR settings.

  This function must be MP safe.

  @param[in, out] Buffer        Unused
**/
VOID
EFIAPI
ApSafeRestoreMsr (
  IN OUT VOID *Buffer
  );

/**
  Restore processor MSR runtime settings for S3.

  @param[in] DispatchHandle  - The handle of this callback, obtained when registering
  @param[in] DispatchContex  - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT
**/
VOID
S3RestoreMsr (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_REGISTER_CONTEXT   *DispatchContext
  );

/**
  Digital Thermal Sensor (DTS) SMM driver function.

  @param[in] ImageHandle         Image handle for this driver image

  @retval EFI_SUCCESS            Driver initialization completed successfully
  @retval EFI_OUT_OF_RESOURCES   Error when allocating required memory buffer.
**/
EFI_STATUS
EFIAPI
InstallDigitalThermalSensor (
  VOID
  );

/**
Enable DTS back on S3 Resume
**/
VOID
EFIAPI
EnableDtsOnS3Resume (
  );

#endif

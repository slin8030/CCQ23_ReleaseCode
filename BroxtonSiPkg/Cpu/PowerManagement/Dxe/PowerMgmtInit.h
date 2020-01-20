/** @file
  This header file contains processors power management definitions.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

#ifndef _POWER_MANAGEMENT_H_
#define _POWER_MANAGEMENT_H_

#include "PowerMgmtCommon.h"

//
// Function prototypes
//
/**
  Configures PowerLimits and Config TDP values
**/
VOID
CtdpS3Save (
  VOID
  );
/**
  Initialize the power management support.
  This function will do boot time configuration:
    Install into SMRAM/SMM
    Detect HW capabilities and SW configuration
    Initialize HW and software state (primarily MSR and ACPI tables)
    Install SMI handlers for runtime interfacess

  @param[in] ImageHandle  Pointer to the loaded image protocol for this driver
  @param[in] SystemTable  Pointer to the EFI System Table

  @retval EFI_SUCCESS   The driver installed/initialized correctly.
  @retval Driver will ASSERT in debug builds on error. PPM functionality is considered critical for mobile systems.
**/
EFI_STATUS
EFIAPI
InitPowerManagement (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  );

/**
  Initializes the CPU power management global variable.
  This must be called prior to any of the functions being used.
**/
VOID
InitPowerManagementGlobalVariables (
  VOID
  );

/**
  Initialize the platform power management based on hardware capabilities
  and user configuration settings.

  This includes creating FVID table, updating ACPI tables,
  and updating processor and chipset hardware configuration.

  This should be called prior to any Px, Cx, Tx activity.

  @retval EFI_SUCCESS - on success
  @retval Appropiate failure code on error
**/
EFI_STATUS
InitPpmDxe (
  VOID
  );
#endif

/** @file
  File to contain all the hardware specific stuff for the Smm Power Button dispatch protocol.

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

CONST SC_SMM_SOURCE_DESC POWER_BUTTON_SOURCE_DESC = {
  SC_SMM_SCI_EN_DEPENDENT,
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_ACPI_PM1_EN
      },
      S_ACPI_PM1_EN,
      N_ACPI_PM1_EN_PWRBTN
    },
    NULL_BIT_DESC_INITIALIZER
  },
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_ACPI_PM1_STS
      },
      S_ACPI_PM1_STS,
      N_ACPI_PM1_STS_PWRBTN
    }
  }
};

/**

  Get the power button status.

  @param[in]  Record               The pointer to the DATABASE_RECORD.
  @param[out] Context              Calling context from the hardware, will be updated with the current power button status.

  @retval None

**/
VOID
EFIAPI
PowerButtonGetContext (
  IN  DATABASE_RECORD    *Record,
  OUT SC_SMM_CONTEXT    *Context
  )
{
  UINT16  GenPmCon2;
  UINT32  PmcBase;

  PmcBase = PMC_BASE_ADDRESS;

  GenPmCon2 = MmioRead16 (PmcBase + R_PMC_GEN_PMCON_2);

  if ((GenPmCon2 & B_PMC_GEN_PMCON_PWRBTN_LVL) != 0) {
    Context->PowerButton.Phase = EfiPowerButtonExit;
  } else {
    Context->PowerButton.Phase = EfiPowerButtonEntry;
  }
}

/**

  Check whether Power Button status of two contexts match

  @param[in] Context1             Context 1 that includes Power Button status 1
  @param[in] Context2             Context 2 that includes Power Button status 2

  @retval FALSE                   Power Button status match
  @retval TRUE                    Power Button status don't match

**/
BOOLEAN
EFIAPI
PowerButtonCmpContext (
  IN SC_SMM_CONTEXT     *Context1,
  IN SC_SMM_CONTEXT     *Context2
  )
{
  return (BOOLEAN) (Context1->PowerButton.Phase == Context2->PowerButton.Phase);
}

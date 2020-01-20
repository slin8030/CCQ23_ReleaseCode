/** @file
  File to contain all the hardware specific stuff for the Smm Sx dispatch protocol.

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

@par Specification Reference:
**/
#include "ScSmmHelpers.h"
#ifndef PROGRESS_CODE_S3_SUSPEND_END
#define PROGRESS_CODE_S3_SUSPEND_END (EFI_SOFTWARE_SMM_DRIVER | (EFI_OEM_SPECIFIC | 0x00000001))
#endif

const SC_SMM_SOURCE_DESC SX_SOURCE_DESC = {
  SC_SMM_NO_FLAGS,
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_SMI_EN
      },
      S_SMI_EN,
      N_SMI_EN_ON_SLP_EN
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
      N_SMI_STS_ON_SLP_EN
    }
  }
};
/**

  Get the Sleep type

  @param[in]  Record               No use
  @param[out] Context              The context that includes SLP_TYP bits to be filled

  @retval None

**/
VOID
EFIAPI
SxGetContext (
  IN  DATABASE_RECORD    *Record,
  OUT SC_SMM_CONTEXT     *Context
  )
{
  UINT32  Pm1Cnt;

  Pm1Cnt = IoRead32 ((UINTN) (AcpiBaseAddr + R_ACPI_PM1_CNT));

  //
  // By design, the context phase will always be ENTRY
  //
  Context->Sx.Phase = SxEntry;

  //
  // Map the PM1_CNT register's SLP_TYP bits to the context type
  //
  switch (Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) {
    case V_ACPI_PM1_CNT_S0:
      Context->Sx.Type = SxS0;
      break;

    case V_ACPI_PM1_CNT_S1:
      Context->Sx.Type = SxS1;
      break;

    case V_ACPI_PM1_CNT_S3:
      Context->Sx.Type = SxS3;
      break;

    case V_ACPI_PM1_CNT_S4:
      Context->Sx.Type = SxS4;
      break;

    case V_ACPI_PM1_CNT_S5:
      Context->Sx.Type = SxS5;
      break;

    default:
      ASSERT (FALSE);
      break;
  }
}
/**

  Check whether sleep type of two contexts match

  @param[in] Context1             Context 1 that includes sleep type 1
  @param[in] Context2             Context 2 that includes sleep type 2

  @retval FALSE                   Sleep types match
  @retval TRUE                    Sleep types don't match

**/
BOOLEAN
EFIAPI
SxCmpContext (
  IN SC_SMM_CONTEXT     *Context1,
  IN SC_SMM_CONTEXT     *Context2
  )
{
  return (BOOLEAN) (Context1->Sx.Type == Context2->Sx.Type);
}
typedef struct {
  UINT8 Device;
  UINT8 Function;
} USB_CONTROLLER;

/**

  Additional xHCI Controller Configurations Prior to Entering S3/S4/S5

  @param[in] None

  @retval None

**/
VOID
XhciSxWorkaround (
  VOID
  )
{
  UINT32      PmcBase;
  UINTN       XhciPciMmBase;

  PmcBase         = PMC_BASE_ADDRESS;
  ///
  /// Check if XHCI controller is enabled
  ///
  if ((MmioRead32 (PmcBase + R_PMC_FUNC_DIS) & (UINT32) B_PMC_FUNC_DIS_USB_XHCI) != 0) {
    return;
  }
  XhciPciMmBase = MmPciAddress (
                    0,
                    DEFAULT_PCI_BUS_NUMBER_SC,
                    PCI_DEVICE_NUMBER_XHCI,
                    PCI_FUNCTION_NUMBER_XHCI,
                    0
                    );
  ///
  /// Set "PME Enable" bit of PWR_CNTL_STS register, D20:F0:74h[8] = 1h
  ///
  MmioOr16 ((XhciPciMmBase + R_XHCI_PWR_CNTL_STS), (UINT16) (B_XHCI_PWR_CNTL_STS_PME_EN));
}

/**

  When we get an SMI that indicates that we are transitioning to a sleep state,
  we need to actually transition to that state.  We do this by disabling the
  "SMI on sleep enable" feature, which generates an SMI when the operating system
  tries to put the system to sleep, and then physically putting the system to sleep.

  @param[in] None

  @retval None.

**/
VOID
ScSmmSxGoToSleep (
  VOID
  )
{
  UINT32  Pm1Cnt;

  //
  // Flush cache into memory before we go to sleep. It is necessary for S3 sleep
  // because we may update memory in SMM Sx sleep handlers -- the updates are in cache now
  //
  AsmWbinvd ();

  //
  // Disable SMIs
  //
  ScSmmClearSource (&SX_SOURCE_DESC);
  ScSmmDisableSource (&SX_SOURCE_DESC);

  //
  // Get Power Management 1 Control Register Value
  //
  Pm1Cnt = IoRead32 ((UINTN) (AcpiBaseAddr + R_ACPI_PM1_CNT));

  if (((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S3) ||
      ((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S4) ||
      ((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S5)) {
    XhciSxWorkaround ();
  }
  if (((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S3) ||
      ((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S4) ||
      ((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S5)) {
  }
  ///
  /// Record S3 suspend performance data
  ///
  if ((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S3) {
    ///
    /// Report status code before goto S3 sleep
    ///
    REPORT_STATUS_CODE (EFI_PROGRESS_CODE, PROGRESS_CODE_S3_SUSPEND_END);

    ///
    /// Flush cache into memory before we go to sleep.
    ///
    AsmWbinvd ();
  }
  //
  // Now that SMIs are disabled, write to the SLP_EN bit again to trigger the sleep
  //
  Pm1Cnt |= B_ACPI_PM1_CNT_SLP_EN;

  IoWrite32 ((UINTN) (AcpiBaseAddr + R_ACPI_PM1_CNT), Pm1Cnt);

  //
  // Should only proceed if wake event is generated.
  //
  if ((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S1) {
    while (((IoRead16 ((UINTN) (AcpiBaseAddr + R_ACPI_PM1_STS))) & B_ACPI_PM1_STS_WAK) == 0x0);
  } else {
    CpuDeadLoop ();
  }
  //
  // The system just went to sleep. If the sleep state was S1, then code execution will resume
  // here when the system wakes up.
  //
  Pm1Cnt = IoRead32 ((UINTN) (AcpiBaseAddr + R_ACPI_PM1_CNT));

  if ((Pm1Cnt & B_ACPI_PM1_CNT_SCI_EN) == 0) {
    //
    // An ACPI OS isn't present, clear the sleep information
    //
    Pm1Cnt &= ~B_ACPI_PM1_CNT_SLP_TYP;
    Pm1Cnt |= V_ACPI_PM1_CNT_S0;

    IoWrite32 ((UINTN) (AcpiBaseAddr + R_ACPI_PM1_CNT), Pm1Cnt);
  }

  ScSmmClearSource (&SX_SOURCE_DESC);
  ScSmmEnableSource (&SX_SOURCE_DESC);
}

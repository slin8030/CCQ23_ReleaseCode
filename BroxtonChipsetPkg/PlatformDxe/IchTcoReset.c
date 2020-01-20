/** @file
 Implements the programming of events in TCO Reset

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

#include "PlatformDxe.h"
#include <Protocol/TcoReset.h>
#include <Protocol/HwWatchdogTimer.h>


EFI_STATUS
EFIAPI
EnableTcoReset (
  IN      UINT32            *RcrbGcsSaveValue
  );

EFI_STATUS
EFIAPI
DisableTcoReset (
  OUT     UINT32    RcrbGcsRestoreValue
  );

EFI_TCO_RESET_PROTOCOL  mTcoResetProtocol = {
  EnableTcoReset,
  DisableTcoReset
};


EFI_STATUS
EFIAPI
EnableTcoReset (
  IN      UINT32            *RcrbGcsSaveValue
  )
/*++

Routine Description:

  Enables the TCO timer to reset the system in case of a system hang.  This is
  used when writing the clock registers.

Arguments:

  RcrbGcsSaveValue  - This is the value of the RCRB GCS register before it is
                      changed by this procedure.  This will be used to restore
                      the settings of this register in PpiDisableTcoReset.

Returns:

  EFI_STATUS

--*/
{
  UINT16                              TmpWord;
  UINT16                              AcpiBaseAddr;
  EFI_WATCHDOG_TIMER_DRIVER_PROTOCOL  *WatchdogTimerProtocol;
  EFI_STATUS                          Status;
  UINTN                               PbtnDisableInterval = 4;  //Default value

  //
  // Get Watchdog Timer protocol.
  //
  Status = gBS->LocateProtocol (&gEfiWatchdogTimerDriverProtocolGuid, NULL, (VOID **)&WatchdogTimerProtocol);

  // If the protocol is present, shut off the Timer as we enter BDS
  if (!EFI_ERROR(Status)) {
    WatchdogTimerProtocol->RestartWatchdogTimer();
    WatchdogTimerProtocol->AllowKnownReset(TRUE);
  }

  if (*RcrbGcsSaveValue == 0) {
    PbtnDisableInterval = PcdGet32(PcdPBTNDisableInterval);
  } else {
    PbtnDisableInterval = *RcrbGcsSaveValue * 10 / 6;
  }

  //
  // Read ACPI Base Address
  //
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  // Stop TCO if not already stopped
//[-start-160817-IB03090432-modify]//
  TmpWord = IoRead16 (AcpiBaseAddr + R_TCO1_CNT);
  TmpWord |= B_TCO1_CNT_TMR_HLT;
  IoWrite16(AcpiBaseAddr + R_TCO1_CNT, TmpWord);
//[-end-160817-IB03090432-modify]//

  // Clear second TCO status
  IoWrite32(AcpiBaseAddr + R_TCO_STS, B_TCO_STS_SECOND_TO);

  // Enable reboot on TCO timeout
  *RcrbGcsSaveValue = MmioRead32 (PMC_BASE_ADDRESS + R_PMC_PM_CFG);
  MmioAnd8 (PMC_BASE_ADDRESS + R_PMC_PM_CFG, (UINT8) ~B_PMC_PM_CFG_NO_REBOOT);

  // Set TCO reload value (interval *.6s)
  IoWrite32(AcpiBaseAddr + R_TCO_TMR, (UINT32)(PbtnDisableInterval<<16));

  // Force TCO to load new value
  IoWrite8 (AcpiBaseAddr + R_TCO_RLD, 4);

  // Clear second TCO status
  IoWrite32 (AcpiBaseAddr + R_TCO_STS, B_TCO_STS_SECOND_TO);

  // Start TCO timer running
//[-start-160817-IB03090432-modify]//
  TmpWord = IoRead16 (AcpiBaseAddr + R_TCO1_CNT);
  TmpWord &= ~(B_TCO1_CNT_TMR_HLT);
  IoWrite16 (AcpiBaseAddr + R_TCO1_CNT, TmpWord);
//[-end-160817-IB03090432-modify]//

  //
  // Clear Sleep Type Enable
  //
  IoWrite16 (AcpiBaseAddr + R_SMI_EN,
    (UINT16)(IoRead16 (AcpiBaseAddr + R_SMI_EN) & (~B_SMI_EN_ON_SLP_EN)));

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
DisableTcoReset (
  OUT     UINT32    RcrbGcsRestoreValue
  )
/*++

Routine Description:

  Disables the TCO timer.  This is used after writing the clock registers.

Arguments:

  RcrbGcsRestoreValue - Value saved in PpiEnableTcoReset so that it can
                        restored.

Returns:

  EFI_STATUS

--*/
{
  UINT16                              TmpWord;
  UINT16                              AcpiBaseAddr;
  EFI_WATCHDOG_TIMER_DRIVER_PROTOCOL  *WatchdogTimerProtocol;
  EFI_STATUS                          Status;

  //
  // Read ACPI Base Address
  //
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  // Stop the TCO timer
//[-start-160817-IB03090432-modify]//
  TmpWord = IoRead16(AcpiBaseAddr + R_TCO1_CNT);
  TmpWord |= B_TCO1_CNT_TMR_HLT;
  IoWrite16 (AcpiBaseAddr + R_TCO1_CNT, TmpWord);
//[-end-160817-IB03090432-modify]//

  //
  // Get Watchdog Timer protocol.
  //
  Status = gBS->LocateProtocol (&gEfiWatchdogTimerDriverProtocolGuid, NULL, (VOID **)&WatchdogTimerProtocol);

  // If the protocol is present, shut off the Timer as we enter BDS
  if (!EFI_ERROR(Status)) {
    WatchdogTimerProtocol->AllowKnownReset(FALSE);
  }

  return EFI_SUCCESS;
}

VOID
InitTcoReset (
  )
/*++

Routine Description:
  Updates the feature policies according to the setup variable.

Arguments:

Returns:
  VOID

--*/
{
  EFI_HANDLE                        Handle;
  EFI_STATUS                        Status;

  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
             &Handle,
             &gEfiTcoResetProtocolGuid,
             EFI_NATIVE_INTERFACE,
             &mTcoResetProtocol
             );
  ASSERT_EFI_ERROR(Status);

}

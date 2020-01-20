/*++
 This file contains an 'Intel Peripheral Driver' and is
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may
 be modified by the user, subject to additional terms of the
 license agreement
--*/
/*++

Copyright (c)  1999 - 2014 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  StallSmm.c

Abstract:

  SMM Stall implementation

--*/

//
// Include files
//
#include "Library/StallSmmLib.h"
#include "Pi/PiSmmCis.h"
#include "PiDxe.h"
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include "ScAccess.h"

VOID
SmmStall (
  IN  UINTN   Microseconds
  )
/*++

Routine Description:

  Delay for at least the request number of microseconds.
  Timer used is ACPI time counter, which has 1us granularity.

Arguments:

  Microseconds - Number of microseconds to delay.

Returns:

  None

--*/
{
  UINTN   Ticks;
  UINTN   Counts;
  UINTN   CurrentTick;
  UINTN   OriginalTick;
  UINTN   RemainingTick;
  UINT16  AcpiBaseAddr;

  if (Microseconds == 0) {
    return;
  }
  
  ///
  /// Read ACPI Base Address
  ///
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

//[-start-180813-IB07400997-modify]//
  OriginalTick = IoRead32 (AcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
//[-end-180813-IB07400997-modify]//
  CurrentTick = OriginalTick;

  //
  // The timer frequency is 3.579545 MHz, so 1 ms corresponds 3.58 clocks
  //
  Ticks = Microseconds * 358 / 100 + OriginalTick + 1;

  //
  // The loops needed by timer overflow
  //
  Counts = Ticks / V_ACPI_PM1_TMR_MAX_VAL;

  //
  // Remaining clocks within one loop
  //
  RemainingTick = Ticks % V_ACPI_PM1_TMR_MAX_VAL;

  //
  // not intend to use TMROF_STS bit of register PM1_STS, because this adds extra
  // one I/O operation, and maybe generate SMI
  //
  while ((Counts != 0) || (RemainingTick > CurrentTick)) {
//[-start-180813-IB07400997-modify]//
    CurrentTick = IoRead32 (AcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
//[-end-180813-IB07400997-modify]//
    //
    // Check if timer overflow
    //
    if (CurrentTick < OriginalTick) {
      Counts--;
    }
    OriginalTick = CurrentTick;
  }
}

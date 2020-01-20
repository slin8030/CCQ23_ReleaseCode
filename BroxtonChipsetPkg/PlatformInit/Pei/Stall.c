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

  Stall.c

Abstract:

  Produce Stall Ppi.

--*/

/*++
 This file contains an 'Intel Peripheral Driver' and is
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may
 be modified by the user, subject to additional terms of the
 license agreement
--*/

#include <ScAccess.h>
#include <PlatformBaseAddresses.h>
#include <Library/PeiServicesLib.h>
#include <Library/PreSiliconLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
//[-start-160120-IB07220034-add]//
#include <Library/TimerLib.h>
//[-end-160120-IB07220034-add]//
#include <Ppi/Stall.h>

#include "Stall.h"  //Function Prototypes

//[-start-160120-IB07220034-modify]//
/**
  Waits for at least the given number of microseconds.

  @param  PeiServices     General purpose services available to every PEIM.
  @param  This            PPI instance structure.
  @param  Microseconds    Desired length of time to wait.

  @retval EFI_SUCCESS     If the desired amount of time was passed.
**/
EFI_STATUS
EFIAPI
Stall (
  IN CONST EFI_PEI_SERVICES   **PeiServices,
  IN CONST EFI_PEI_STALL_PPI  *This,
  IN UINTN                    Microseconds
  )
{
  MicroSecondDelay (Microseconds);
  return EFI_SUCCESS;
}

/**
  Waits for at least the given number of microseconds.

  @param  PeiServices     General purpose services available to every PEIM.
  @param  This            PPI instance structure.
  @param  Microseconds    Desired length of time to wait.

  @retval EFI_SUCCESS     If the desired amount of time was passed.
**/
EFI_STATUS
EFIAPI
Stall_org (
  IN CONST EFI_PEI_SERVICES   **PeiServices,
  IN CONST EFI_PEI_STALL_PPI  *This,
  IN UINTN                    Microseconds
  )
{
   //
   // !! This should be re-written to use TimerLib !!
   //

  if (PLATFORM_ID == VALUE_REAL_PLATFORM) {
    UINTN   Ticks;
    UINTN   Counts;
    UINT16  AcpiBaseAddr;
    UINT32  CurrentTick;
    UINT32  OriginalTick;
    UINT32  RemainingTick;

    if (Microseconds == 0) {
      return EFI_SUCCESS;
    }

    ///
    /// Read ACPI Base Address
    ///
    AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
    
    OriginalTick = IoRead32 (AcpiBaseAddr + R_ACPI_PM1_TMR);
    OriginalTick &= (V_ACPI_PM1_TMR_MAX_VAL - 1);
    CurrentTick = OriginalTick;

    //
    // The timer frequency is 3.579545MHz, so 1 ms corresponds to 3.58 clocks
    //
    Ticks = Microseconds * 358 / 100 + OriginalTick + 1;

    //
    // The loops needed for timer overflow
    //
    Counts = (UINTN)RShiftU64((UINT64)Ticks, 24);

    //
    // Remaining clocks within one loop
    //
    RemainingTick = Ticks & 0xFFFFFF;

    //
    // Do not intend to use TMROF_STS bit of register PM1_STS, because this add extra
    // one I/O operation, and may generate SMI
    //
    while (Counts != 0) {
      CurrentTick = IoRead32 (AcpiBaseAddr + R_ACPI_PM1_TMR) & B_ACPI_PM1_TMR_VAL;
//[-start-180813-IB07400997-modify]//
      if (CurrentTick < OriginalTick) {
//[-end-180813-IB07400997-modify]//
        Counts--;
      }
      OriginalTick = CurrentTick;
    }

    while ((RemainingTick > CurrentTick) && (OriginalTick <= CurrentTick)) {
      OriginalTick  = CurrentTick;
      CurrentTick   = IoRead32 (AcpiBaseAddr + R_ACPI_PM1_TMR) & B_ACPI_PM1_TMR_VAL;
    }
  } else {
    do {
      CpuPause();
    } while(Microseconds-- > 0);
  }

  return EFI_SUCCESS;
}
//[-end-160120-IB07220034-modify]//

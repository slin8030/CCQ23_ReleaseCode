/*++
This file contains an 'Intel Peripheral Driver' and is
licensed for Intel CPUs and chipsets under the terms of your
license agreement with Intel or your vendor.  This file may
be modified by the user, subject to additional terms of the
license agreement
--*/
/** @file
ICH9 ACPI Pre-Silicon Timer implements one instance of Timer Library.

Copyright (c) 2007-2014, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#include <Base.h>
#include <ScAccess.h>
#include <Library/TimerLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>


/**
The constructor function enables ACPI IO space.

If ACPI I/O space not enabled, this function will enable it.
It will always return RETURN_SUCCESS.

@retval EFI_SUCCESS   The constructor always returns RETURN_SUCCESS.

**/
RETURN_STATUS
EFIAPI
IntelScAcpiPreSiTimerLibConstructor(
VOID
)
{
  return RETURN_SUCCESS;
}

/**
Internal function to read the current tick counter of ACPI.

Internal function to read the current tick counter of ACPI.

@return The tick counter read.

**/
STATIC
UINT32
InternalAcpiGetTimerTick(
VOID
)
{
  return IoRead32(PcdGet16(PcdScAcpiIoPortBaseAddress) + R_ACPI_PM1_TMR);
}

/**
Stalls the CPU for at least the given number of ticks.

Stalls the CPU for at least the given number of ticks. It's invoked by
MicroSecondDelay() and NanoSecondDelay().

@param  Delay     A period of time to delay in ticks.

**/
STATIC
VOID
InternalAcpiDelay(
IN      UINT32                    Delay
)
{
    Delay = Delay / 300;
    do {
      CpuPause();
    } while (Delay-- > 0);
}

/**
Stalls the CPU for at least the given number of microseconds.

Stalls the CPU for the number of microseconds specified by MicroSeconds.

@param  MicroSeconds  The minimum number of microseconds to delay.

@return MicroSeconds

**/
UINTN
EFIAPI
MicroSecondDelay(
IN      UINTN                     MicroSeconds
)
{
  InternalAcpiDelay(
    (UINT32)MicroSeconds
    );

  return MicroSeconds;
}

/**
Stalls the CPU for at least the given number of nanoseconds.

Stalls the CPU for the number of nanoseconds specified by NanoSeconds.

@param  NanoSeconds The minimum number of nanoseconds to delay.

@return NanoSeconds

**/
UINTN
EFIAPI
NanoSecondDelay(
IN      UINTN                     NanoSeconds
)
{
  InternalAcpiDelay(
    (UINT32)NanoSeconds
    );
  return NanoSeconds;
}

/**
Retrieves the current value of a 64-bit free running performance counter.

Retrieves the current value of a 64-bit free running performance counter. The
counter can either count up by 1 or count down by 1. If the physical
performance counter counts by a larger increment, then the counter values
must be translated. The properties of the counter can be retrieved from
GetPerformanceCounterProperties().

@return The current value of the free running performance counter.

**/
UINT64
EFIAPI
GetPerformanceCounter(
VOID
)
{
  return (UINT64)InternalAcpiGetTimerTick();
}

/**
Retrieves the 64-bit frequency in Hz and the range of performance counter
values.

If StartValue is not NULL, then the value that the performance counter starts
with immediately after is it rolls over is returned in StartValue. If
EndValue is not NULL, then the value that the performance counter end with
immediately before it rolls over is returned in EndValue. The 64-bit
frequency of the performance counter in Hz is always returned. If StartValue
is less than EndValue, then the performance counter counts up. If StartValue
is greater than EndValue, then the performance counter counts down. For
example, a 64-bit free running counter that counts up would have a StartValue
of 0 and an EndValue of 0xFFFFFFFFFFFFFFFF. A 24-bit free running counter
that counts down would have a StartValue of 0xFFFFFF and an EndValue of 0.

@param  StartValue  The value the performance counter starts with when it
rolls over.
@param  EndValue    The value that the performance counter ends with before
it rolls over.

@return The frequency in Hz.

**/
UINT64
EFIAPI
GetPerformanceCounterProperties(
OUT      UINT64                    *StartValue, OPTIONAL
OUT      UINT64                    *EndValue     OPTIONAL
)
{
  if (StartValue != NULL) {
    *StartValue = 0;
  }

  if (EndValue != NULL) {
    *EndValue = V_ACPI_PM1_TMR_MAX_VAL - 1;
  }

  return V_ACPI_PM1_TMR_FREQUENCY;
}


UINT64
EFIAPI
GetTimeInNanoSecond (
  IN      UINT64                     Ticks
  )
{
  UINT64  Frequency;
  UINT64  NanoSeconds;
  UINT64  Remainder;
  INTN    Shift;
  Frequency = GetPerformanceCounterProperties (NULL, NULL);

  //
  //          Ticks
  // Time = --------- x 1,000,000,000
  //        Frequency
  //
  NanoSeconds = MultU64x32 (DivU64x64Remainder (Ticks, Frequency, &Remainder), 1000000000u);

  //
  // Ensure (Remainder * 1,000,000,000) will not overflow 64-bit.
  // Since 2^29 < 1,000,000,000 = 0x3B9ACA00 < 2^30, Remainder should < 2^(64-30) = 2^34,
  // i.e. highest bit set in Remainder should <= 33.
  //
  Shift = MAX (0, HighBitSet64 (Remainder) - 33);
  Remainder = RShiftU64 (Remainder, (UINTN) Shift);
  Frequency = RShiftU64 (Frequency, (UINTN) Shift);
  NanoSeconds += DivU64x64Remainder (MultU64x32 (Remainder, 1000000000u), Frequency, NULL);

  return NanoSeconds;
}

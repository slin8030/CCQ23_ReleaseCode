/** @file
  A Internal Timer Library implementation which uses the Time Stamp Counter in the processor.

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/TimerLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/PcdLib.h>

/**  Calculate TSC frequency.

  The TSC counting frequency is determined by comparing how far it counts
  during a 1ms period as determined by the ACPI timer. The ACPI timer is
  used because it counts at a known frequency.
  If ACPI I/O space not enabled, this function will enable it. Then the
  TSC is sampled, followed by waiting for 3579 clocks of the ACPI timer, or 1ms.
  The TSC is then sampled again. The difference multiplied by 1000 is the TSC
  frequency. There will be a small error because of the overhead of reading
  the ACPI timer. An attempt is made to determine and compensate for this error.

  @return The number of TSC counts per second.

**/
#define REG_TIMER_CONTROL_WORD            0x43
#define REG_COUNTER_0_STATUS_BYTE_FORMAT  0x40
#define REG_COUNTER_1_STATUS_BYTE_FORMAT  0x41
#define REG_COUNTER_2_STATUS_BYTE_FORMAT  0x42

UINT64
InternalCalculateTscFrequency (
  VOID
  )
{
  UINT64      StartTSC;
  UINT64      EndTSC;
  UINT32      Ticks;
  UINT64      TscFrequency;
  BOOLEAN     InterruptEnabled;
  UINT8       SavePort61;
  UINT8       CounterStatus;

  //
  // Check interrupt flag
  //
  InterruptEnabled = GetInterruptState ();
  if (InterruptEnabled) {
    DisableInterrupts ();
  }

  SavePort61 = IoRead8 (0x61);

  //
  // save origianl status
  //
  IoWrite8 (REG_TIMER_CONTROL_WORD, 0xE8);
  CounterStatus = IoRead8 (REG_COUNTER_2_STATUS_BYTE_FORMAT);

  //
  // enable counter 2 and disable speaker
  //
  IoWrite8 (0x61, (SavePort61 & ~0x02) | 0x01);

  //
  // Set counter 2 to mode 0 (one-shot), binary count
  // 8254 Timer Frequency: 1193182 Hz
  // 1 ms ticks => 1193182 * 0.001 = 1193
  //
  Ticks = 1193;
  IoWrite8 (REG_TIMER_CONTROL_WORD, 0xB0);
  IoWrite8 (REG_COUNTER_2_STATUS_BYTE_FORMAT, (UINT8)(Ticks & 0xFF));
  IoWrite8 (REG_COUNTER_2_STATUS_BYTE_FORMAT, (UINT8)((Ticks >> 8) & 0xFF));

  //
  // PIT starts counting at the next edge, we need to delay for a microsecond,
  //
  IoRead8 (0x61);

  //
  // port 0x61, bit #5 is counter 2 OUT Status.
  //
  StartTSC = AsmReadTsc();
  while ((IoRead8 (0x61) & 0x20) == 0) {
  }
  EndTSC = AsmReadTsc();
  TscFrequency = EndTSC - StartTSC;

  //
  // calcaute 1 second tsc frequency
  //
  TscFrequency = MultU64x32 (TscFrequency, 1000);

  //
  // restore 8254 status
  //
  IoWrite8 (REG_TIMER_CONTROL_WORD, ((CounterStatus & 0x3F) | 0x80));
  if (((CounterStatus & 0x3F) | 0x80) == 0xB6) {
    //
    // speaker is mode 3, default frequncy 0x500
    //
    Ticks = 0x500;
    IoWrite8 (REG_COUNTER_2_STATUS_BYTE_FORMAT, (UINT8)(Ticks & 0xFF));
    IoWrite8 (REG_COUNTER_2_STATUS_BYTE_FORMAT, (UINT8)((Ticks >> 8) & 0xFF));
  }

  //
  // restore
  //
  IoWrite8 (0x61, SavePort61);
  if (InterruptEnabled) {
    EnableInterrupts ();
  }

  return TscFrequency;
}

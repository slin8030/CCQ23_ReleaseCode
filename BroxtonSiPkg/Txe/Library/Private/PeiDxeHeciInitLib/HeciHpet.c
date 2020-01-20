/** @file
  Definitions for HECI driver

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

#include "HeciHpet.h"

//[-start-160629-IB07400749-add]//
#ifdef EFI_DEBUG
#ifndef EFI_DEBUG_HECI_MESSAGE
#undef DEBUG
#define DEBUG(Expression)
#endif
#endif
//[-end-160629-IB07400749-add]//

///
/// Extern for shared HECI data and protocols
///
volatile UINT32       mSaveHpetConfigReg;

/**
Enable Hpet function.

   @param[in] None

 @retval UINT32                  Return the High Precision Event Timer base address

**/
volatile
UINT32 *
EnableHpet(
VOID
)
{
  volatile UINT32         *HpetTimer;

  HpetTimer = (VOID *)(UINTN)(HPET_BASE_ADDRESS);

  //
  // Start the timer so it is up and running
  //
  HpetTimer[HPET_GEN_CONFIG_LOW] = HPET_START;

  DEBUG((EFI_D_INFO, "EnableHpet %x %x\n\n ", HPET_GEN_CONFIG_LOW, HpetTimer));
  return HpetTimer;

  /*
  VOLATILE UINT32 *HpetConfigReg;

  HpetConfigReg = NULL;
  //
  // Get the High Precision Event Timer base address and enable the memory range
  //
  HpetConfigReg = (UINT32 *) (UINTN) (SC_RCRB_BASE + R_PCH_RCRB_HPTC);
  switch (*HpetConfigReg & B_PCH_RCRB_HPTC_AS) {
  case 0:
  mHeciContext->HpetTimer = (VOID *) (UINTN) (HPET_ADDRESS_0);
  break;

  case 1:
  mHeciContext->HpetTimer = (VOID *) (UINTN) (HPET_ADDRESS_1);
  break;

  case 2:
  mHeciContext->HpetTimer = (VOID *) (UINTN) (HPET_ADDRESS_2);
  break;

  case 3:
  mHeciContext->HpetTimer = (VOID *) (UINTN) (HPET_ADDRESS_3);
  break;

  default:
  mHeciContext->HpetTimer = NULL;
  break;
  }
  //
  // Read this back to force the write-back.
  //
  *HpetConfigReg = *HpetConfigReg | B_PCH_RCRB_HPTC_AE;

  //
  // Start the timer so it is up and running
  //
  mHeciContext->HpetTimer[HPET_GEN_CONFIG_LOW]  = HPET_START;
  mHeciContext->HpetTimer[HPET_GEN_CONFIG_LOW]  = HPET_START;

  return;
  */
}
/**
  Store the value of High Performance Timer

  @param[in]  None

  @retval  None

**/
VOID
SaveHpet (
  VOID
  )
{
//  mSaveHpetConfigReg = MmioRead32 (SC_RCRB_BASE + R_PCH_RCRB_HPTC);
}

/**
  Restore the value of High Performance Timer

  @param[in]  None

  @retval  None

**/
VOID
RestoreHpet (
  VOID
  )
{
//  MmioWrite32 (SC_RCRB_BASE + R_PCH_RCRB_HPTC, mSaveHpetConfigReg);
}

/**
   Used for calculating timeouts

  @param[in]  Start - Snapshot of the HPET timer
  @param[in]  End   - Calculated time when timeout period will be done
  @param[in]  Time  - Timeout period in microseconds

  @retval  None

**/
volatile
UINT32 *
StartTimer (
  OUT UINT32 *Start,
  OUT UINT32 *End,
  IN  UINT32 Time
  )
{
  UINT32  Ticks;
  volatile UINT32 *HpetTimer;

  ///
  /// Make sure that HPET is enabled and running
  ///
  HpetTimer = EnableHpet();

  ///
  /// Read current timer value into start time from HPET
  ///
  *Start = HpetTimer[HPET_MAIN_COUNTER_LOW];

  ///
  /// Convert microseconds into 70ns timer ticks
  ///
  Ticks = Time * HPET_TICKS_PER_MICRO;

  ///
  /// Compute end time
  ///
  *End = *Start + Ticks;

  return HpetTimer;
}

/**
  Used to determine if a timeout has occured.

  @param[in]  Start - Snapshot of the HPET timer when the timeout period started.
  @param[in]  End   - Calculated time when timeout period will be done.

  @retval  EFI_STATUS

**/
EFI_STATUS
Timeout(
IN  UINT32                      Start,
IN  UINT32                      End,
IN  volatile UINT32             *HpetTimer
)
/*++

  Routine Description:
    Used to determine if a timeout has occured.

  Arguments:
    Start - Snapshot of the HPET timer when the timeout period started.
    End   - Calculated time when timeout period will be done.

  Returns:
    EFI_STATUS

--*/
{
  UINT32  Current;

  ///
  /// Read HPET and assign the value as the current time.
  ///
  Current = HpetTimer[HPET_MAIN_COUNTER_LOW];

  ///
  /// Test basic case (no overflow)
  ///
  if ((Start < End) && (End <= Current)) {
    return EFI_TIMEOUT;
  }
  ///
  /// Test basic start/end conditions with overflowed timer
  ///
  if ((Start < End) && (Current < Start)) {
    return EFI_TIMEOUT;
  }
  ///
  /// Test for overflowed start/end condition
  ///
  if ((Start > End) && ((Current < Start) && (Current > End))) {
    return EFI_TIMEOUT;
  }
  ///
  /// Catch corner case of broken arguments
  ///
  if (Start == End) {
    return EFI_TIMEOUT;
  }


  DEBUG ((EFI_D_INFO, "crnt %X start %X end %X\n", Current, Start, End));
  ///
  /// Else, we have not yet timed out
  ///
  return EFI_SUCCESS;
}

/**
  Delay for at least the request number of microseconds

  @param[in]  delayTime - Number of microseconds to delay.

  @retval  None

**/
VOID
IoDelay (
  UINT32 delayTime
  )
{
  // TODO: Provide either MicroSecondDelay or PPI Stall method. StallPpi->Stall (PeiServices, StallPpi, delayTime);
  MicroSecondDelay(delayTime);
//  gBS->Stall (delayTime);
}

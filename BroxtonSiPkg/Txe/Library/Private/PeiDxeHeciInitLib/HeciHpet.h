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

#ifndef _HECI_HPET_H
#define _HECI_HPET_H

#include "HeciCore.h"
#include <IndustryStandard/Pci22.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <HeciRegs.h>
#include <SeCAccess.h>


volatile
UINT32 *
EnableHpet(
VOID
)
/*++

Routine Description:

Enable Hpet function.

Arguments:

None.

Returns:

None.

--*/
;

VOID
SaveHpet (
  VOID
  )
;

/**
  Restore the value of High Performance Timer

  @param[in]  None

  @retval  None

**/
VOID
RestoreHpet (
  VOID
  )
;

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
;

/**
  Used to determine if a timeout has occured.

  @param[in]  Start - Snapshot of the HPET timer when the timeout period started.
  @param[in]  End   - Calculated time when timeout period will be done.

  @retval  EFI_STATUS

**/
EFI_STATUS
Timeout (
  IN  UINT32 Start,
  IN  UINT32                      End,
  IN  volatile UINT32             *HpetTimer
  )
;

/**
  Delay for at least the request number of microseconds

  @param[in]  delayTime - Number of microseconds to delay.

  @retval  None

**/
VOID
IoDelay (
  UINT32 delayTime
  )
;

#endif ///< _HECI_HPET_H

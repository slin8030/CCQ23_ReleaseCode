/** @file
  CpuPowerOnConfig PEI Library

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

#include <Private/Library/CpuPowerOnConfigLib.h>
#include <Library/DebugLib.h>
#include <Library/TimerLib.h>
#include <Library/CpuPlatformLib.h>
#include <Library/PcdLib.h>
#include <CpuAccess.h>
#include <Library/CpuPolicyLib.h>
#include <SaRegs.h>
#include <Library/IoLib.h>
#include <Library/PostCodeLib.h>

/**
  Calling this function causes a system-wide reset. This sets
  all circuitry within the system to its initial state. This type of reset
  is asynchronous to system operation and operates without regard to
  cycle boundaries.

  System reset should not return, if it returns, it means the system does
  not support cold reset.
**/
VOID
EFIAPI
ResetCold (
  VOID
  )
{
  IoWrite8 (RESET_GENERATOR_PORT, 0x2);
  IoWrite8 (RESET_GENERATOR_PORT, 0xE);
}

/**
  Check Active Cores and Reset if needed
**/
VOID
CheckAndResetActiveCoresAfterCpl (
  )
{
  PostCode (0xC31);
  if (PcdGet32 (PcdSetCoreCount)==1){
    DEBUG ((DEBUG_INFO, "Written Number of Active Cores back to PUNIT - Issuing Reset\n"));
//[-start-160819-IB03090432-modify]//
    PcdSet32 (PcdSetCoreCount, 0);
//[-end-160819-IB03090432-modify]//
    PostCode (0xC34);
    MicroSecondDelay (3000000);
    ResetCold();
  }
  PostCode (0xC36);
  return;
}


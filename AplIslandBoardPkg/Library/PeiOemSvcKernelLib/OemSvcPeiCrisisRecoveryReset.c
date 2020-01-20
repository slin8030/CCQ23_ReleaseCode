/** @file
  Provide OEM to reset system after crisis recovery complete.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/PeiOemSvcKernelLib.h>
#include <ScRegs/RegsPcu.h>
#include <Library/CmosLib.h>
#include <ChipsetCmos.h>
//[-start-160126-IB07220038-add]//
#include <Library/IoLib.h>
//[-end-160126-IB07220038-add]//

/**
  Provide OEM to reset system after crisis recovery complete.

  @retval EFI_UNSUPPORTED    Returns unsupported by default.
  @retval EFI_SUCCESS        The service is customized in the project.
  @retval EFI_MEDIA_CHANGED  The value of IN OUT parameter is changed.
  @retval Others             Depends on customization.
**/
EFI_STATUS
OemSvcPeiCrisisRecoveryReset (
  )
{
//[-start-160905-IB07400778-modify]//
  WriteExtCmos8(R_RTC_EXT_INDEX, R_RTC_EXT_TARGET, CmosRecoveryOnFlagAddress, V_CMOS_FAST_RECOVERY_DISABLED);
//[-end-160905-IB07400778-modify]//

//[-start-160126-IB07220038-add]//
  // Set the "MemValid" bit in the SRAM which will be used to indicate memory contents need to be preserved to the MRC
  // on warm/cold reset and S3.
  MmioAnd32 ((PcdGet32(PcdPmcSsramBaseAddress0) + 0x1410), (UINT32)~BIT0); 
//[-end-160126-IB07220038-add]//

  return EFI_UNSUPPORTED;
}

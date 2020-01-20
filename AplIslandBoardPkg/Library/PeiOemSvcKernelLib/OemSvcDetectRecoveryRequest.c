/** @file
  This OemService provides OEM to decide the method of recovery request. 
  When DXE loader found that the DXE-core of firmware volume is corrupt, it will force system to restart. 
  This service will be called to set the recovery requests before system restart. 
  To design the recovery requests according to OEM specification.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
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
//[-start-160907-IB07400780-add]//
#include <Library/PlatformCmosLib.h>
//[-end-160907-IB07400780-add]//

/**
  This OemService provides OEM to detect the recovery mode. 
  OEM designs the rule to detect that boot mode is recovery mode or not, 
  The rule bases on the recovery request which defined in OemService "OemSetRecoveryRequest".

  @param[in, out]  *IsRecovery        If service detects that the current system is recovery mode.
                                      This parameter will return TRUE, else return FALSE.

  @retval          EFI_UNSUPPORTED    Returns unsupported by default.
  @retval          EFI_SUCCESS        The service is customized in the project.
  @retval          EFI_MEDIA_CHANGED  The value of IN OUT parameter is changed. 
  @retval          Others             Depends on customization.
**/
EFI_STATUS
OemSvcDetectRecoveryRequest (
  IN OUT BOOLEAN                               *IsRecovery
  )
{
//[-start-160907-IB07400780-add]//
  if (!CheckCmosBatteryStatus()) { // CMOS data missing, disable recovery mode
    WriteExtCmos8(R_XCMOS_INDEX, R_XCMOS_DATA, CmosRecoveryOnFlagAddress, V_CMOS_FAST_RECOVERY_DISABLED);
  }
//[-end-160907-IB07400780-add]//
//[-start-151216-IB07220025-add]//
//[-start-160905-IB07400778-modify]//
  if ((ReadExtCmos8(R_RTC_EXT_INDEX, R_RTC_EXT_TARGET, CmosRecoveryOnFlagAddress)) == V_CMOS_FAST_RECOVERY_ENABLED) {
//[-end-160905-IB07400778-modify]//
    *IsRecovery = TRUE;
  }
//[-end-151216-IB07220025-add]//
  return EFI_UNSUPPORTED;
}

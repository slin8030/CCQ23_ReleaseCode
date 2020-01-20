/** @file
  This OemService provides OEM to decide the method of recovery request. 
  When DXE loader found that the DXE-core of firmware volume is corrupt, it will force system to restart. 
  This service will be called to set the recovery requests before system restart. 
  To design the recovery requests according to OEM specification.

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corporation. All Rights Reserved.
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

/**
  This OemService provides OEM to decide the method of recovery request. 
  When DXE loader found that the DXE-core of firmware volume is corrupt, it will force system to restart. 
  This service will be called to set the recovery requests before system restart. 
  To design the recovery requests according to OEM specification.

  @param  Based on OEM design.

  @retval EFI_UNSUPPORTED      Returns unsupported by default.
  @retval EFI_SUCCESS          The service is customized in the project.
  @retval EFI_MEDIA_CHANGED    The value of IN OUT parameter is changed. 
  @retval Others               Depends on customization.
**/
EFI_STATUS
OemSvcSetRecoveryRequest (
  VOID
  )
{
//[-start-160905-IB07400778-modify]//
  WriteExtCmos8(R_RTC_EXT_INDEX, R_RTC_EXT_TARGET, CmosRecoveryOnFlagAddress, V_CMOS_FAST_RECOVERY_ENABLED);
//[-end-160905-IB07400778-modify]//

  return EFI_UNSUPPORTED;
}


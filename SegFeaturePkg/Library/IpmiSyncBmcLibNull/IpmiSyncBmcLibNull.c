/** @file
 IPMI sync bmc Null code.

;******************************************************************************
;* Copyright (c) 2016 - 2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/

#include <Library/IpmiSyncBmcLib.h>

/**
 This function can sync power policy to and from BMC  

 @param[in]         PowerPolicy         Power policy.
 @param[in]         SetToBmc            Set to Bmc or not .

 @retval EFI_SUCCESS                    Update platform policy success.
 @return EFI_ERROR (Status)             Implement code execute status.
*/
EFI_STATUS
IpmiSyncBmcPowerPolicy (
  UINT8             *PowerPolicy,
  BOOLEAN           SetToBmc
  )
{
  return EFI_UNSUPPORTED;
}

//[-start-180309-IB09330437-add]//
EFI_STATUS
IpmiSyncBmcPchControlPowerPolicy (
  IN OUT BOOLEAN        *SetAfterG3On
  )
{
  return EFI_UNSUPPORTED;
}
//[-end-180309-IB09330437-add]//


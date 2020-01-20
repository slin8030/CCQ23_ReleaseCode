/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
  Subsystem IDs setting for multiplatform.

  Copyright (c) 2010-2016, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#include <BoardSsidSvid.h>
//
// Global module data
//

EFI_STATUS
InitializeBoardSsidSvid (
    IN CONST EFI_PEI_SERVICES       **PeiServices,
    IN EFI_PLATFORM_INFO_HOB        *PlatformInfoHob
  )
/*++

Routine Description:


Arguments:

Returns:

--*/
{
    UINT32  SsidSvidValue = 0;
    //
    // Set OEM ID according to Board ID.
    //
    switch (PlatformInfoHob->BoardId) {
      case BOARD_ID_BB_RVP:
      case BOARD_ID_BL_RVP:
      case BOARD_ID_BL_FFRD:
      case BOARD_ID_AV_SVP:
             default:
                  SsidSvidValue = SUBSYSTEM_SVID_SSID;//SUBSYSTEM_SVID_SSID_DEFAULT;
                  break;
      }
    PlatformInfoHob->SsidSvid = SsidSvidValue;
    return  EFI_SUCCESS;
}


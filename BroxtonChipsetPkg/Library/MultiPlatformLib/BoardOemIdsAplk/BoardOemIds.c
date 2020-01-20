/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
  ACPI oem ids setting for multiplatform.

  Copyright (c) 2010-2016, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#include <BoardOemIds.h>
//
// Global module data
//

EFI_STATUS
InitializeBoardOemId (
  IN CONST EFI_PEI_SERVICES       **PeiServices,
  IN EFI_PLATFORM_INFO_HOB        *PlatformInfoHob
  )
/*++

Routine Description:


Arguments:

Returns:

--*/
{
    UINT64  OemId;
    UINT64  OemTableId;
    //
    // Set OEM ID according to Board ID.
    //
    switch (PlatformInfoHob->BoardId) {
      // TODO: Need to assign different ID for different Board?
      case BOARD_ID_BB_RVP:
      case BOARD_ID_BL_RVP:	  
      case BOARD_ID_BL_FFRD:
      case BOARD_ID_AV_SVP:
      default:
        OemId = EFI_ACPI_OEM_ID_DEFAULT;
        OemTableId = EFI_ACPI_OEM_TABLE_ID_DEFAULT;
        break;
    }

    PlatformInfoHob->AcpiOemId = OemId;
    PlatformInfoHob->AcpiOemTableId = OemTableId;
    return  EFI_SUCCESS;
}


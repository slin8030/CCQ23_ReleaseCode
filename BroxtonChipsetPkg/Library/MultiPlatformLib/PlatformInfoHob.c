/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
This file does Multiplatform initialization

@copyright
Copyright (c) 2010 - 2016 Intel Corporation. All rights reserved.
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.
This file contains a 'Sample Driver' and is licensed as such
under the terms of your license agreement with Intel or your
vendor. This file may be modified by the user, subject to
the additional terms of the license agreement.

@par Specification Reference:
**/

#include <Library/MultiPlatformLib.h>

//[-start-160603-IB06720411-modify]//
/*++

Routine Description:

  Returns the Platform Info of the platform from the HOB.

Arguments:

  PeiServices               - General purpose services available to every PEIM.
  PlatformInfoHob           - Pointer to the PLATFORM_INFO_HOB Pointer

Returns:

  EFI_SUCCESS               - The function completed successfully.
  EFI_NOT_FOUND             - PlatformInfoHob data doesn't exist, use default instead.

--*/
EFI_STATUS
GetPlatformInfoHob (
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  OUT EFI_PLATFORM_INFO_HOB     **PlatformInfoHob
  )
{
  EFI_PEI_HOB_POINTERS        GuidHob;

  //
  // Find the PlatformInfo HOB
  //
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw == NULL) {
    return EFI_NOT_FOUND;
  }

  if ((GuidHob.Raw = GetNextGuidHob (&gEfiPlatformInfoGuid, GuidHob.Raw)) != NULL) {
    *PlatformInfoHob = GET_GUID_HOB_DATA (GuidHob.Guid);
  }

  //
  // PlatformInfo PEIM should provide this HOB data, if not ASSERT and return error.
  //
  ASSERT_EFI_ERROR (*PlatformInfoHob != NULL);
  if (!(*PlatformInfoHob)) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}
//[-end-160603-IB06720411-modify]//


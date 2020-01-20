/** @file
This file provides APIs for Board Identification

@copyright
Copyright (c) 1999 - 2016 Intel Corporation. All rights reserved.
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

//[-start-160413-IB03090426-modify]//
//[-start-160603-IB06720411-modify]//
#include <Library/MultiPlatformLib.h>
//[-end-160603-IB06720411-modify]//

/**
GetBoardIdFabId returns the Platform Board Id and Fab Id.

@param[out] BoardId             The Board Id of the platform
@param[out] FabId               The FAB Id of the platform

@retval EFI_SUCCESS             The policy default is initialized.

**/
EFI_STATUS
GetBoardIdFabId (
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  )
{
  UINT8       DataBuffer[2];
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;
  Status = GetBoardFabId (DataBuffer);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Error Reading Board Id and Fab Id from EC\n"));
    return Status;
  }

  // BoardId - Bits 2:0
  // FabId - Bits 10:8
  *BoardId = DataBuffer[0] & 0x7;
  *FabId   = DataBuffer[1] & 0x7;

  return Status;
}
//[-end-160413-IB03090426-modify]//

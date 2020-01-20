/** @file
  Get board ID.

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/PeiOemSvcKernelLib.h>

/**
  Get board ID.

  @param[in, out]  *BoardId           A pointer to board ID.

  @retval          EFI_UNSUPPORTED    Returns unsupported by default.
  @retval          EFI_SUCCESS        The service is customized in the project.
  @retval          EFI_MEDIA_CHANGED  The value of IN OUT parameter is changed.
  @retval          Others             Depends on customization.
**/
EFI_STATUS
OemSvcGetBoardId (
  IN OUT UINT32                                *BoardId
  )
{
  return EFI_UNSUPPORTED;
}


/** @file

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

#include <Library/PeiOemSvcChipsetLib.h>
/**
 This function offers an interface to Get Board/Fab Id.


 @param[out]        *BoardId            On entry, points to BoardId.
                                        On exit, points to updated BoardId.
 @param[out]        *FabId              On entry, points to FabId.
                                        On exit, points to updated FabId.
 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcGetBoardFabIds (
  OUT UINT8                    *BoardId,
  OUT UINT8                    *FabId
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/

  *BoardId = BOARD_ID_OXH_CRB;
  return EFI_MEDIA_CHANGED;
// return EFI_UNSUPPORTED;
}

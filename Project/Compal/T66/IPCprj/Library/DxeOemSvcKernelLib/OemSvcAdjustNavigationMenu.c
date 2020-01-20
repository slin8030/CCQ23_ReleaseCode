/** @file
  Adjust navigation menu which is displayed in Setup Utility.

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/



/**
  Adjust navigation menu which is displayed in Setup Utility

  @param[in, out] HiiHandle            Pointer to HII handle list
  @param[in, out] HiiHandleCount       Pointer to the number of HII handle list
  @param[in, out] FormSetGuid          Pointer to the formset GUID

  @retval EFI_UNSUPPORTED              Returns unsupported by default.
  @retval EFI_MEDIA_CHANGED            The value of IN OUT parameter is changed.
**/
EFI_STATUS
OemSvcAdjustNavigationMenu (
  IN OUT EFI_HII_HANDLE                *HiiHandle,
  IN OUT UINTN                         *HiiHandleCount,
  IN OUT EFI_GUID                      *FormSetGuid
  )
{

  return EFI_UNSUPPORTED;
}


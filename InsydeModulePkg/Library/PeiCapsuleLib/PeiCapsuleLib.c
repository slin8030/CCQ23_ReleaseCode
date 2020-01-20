/** @file
  Lite version of PeiCapsuleLib for PEIMs before memory initialization

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <SecureFlash.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Ppi/ReadOnlyVariable2.h>

/**
  Check secure flash image under S3 boot mode

  @param[in]  None

  @retval   Returns TRUE if pending cpasule existed.

**/
BOOLEAN
DetectPendingUpdateImage (
  VOID
  )
{
  EFI_STATUS                         Status;
  IMAGE_INFO                         ImageInfo;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI    *PpiVariableServices;
  UINTN                              Size;

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             (VOID **) &PpiVariableServices
             );
  if (!EFI_ERROR (Status)) {
    Size = sizeof (IMAGE_INFO);
    Status = PpiVariableServices->GetVariable (
                                    PpiVariableServices,
                                    SECURE_FLASH_INFORMATION_NAME,
                                    &gSecureFlashInfoGuid,
                                    NULL,
                                    &Size,
                                    (VOID *) &ImageInfo
                                    );
    if (!EFI_ERROR (Status) && (ImageInfo.FlashMode)) {
      return TRUE;
    }
  }

  return FALSE;
}
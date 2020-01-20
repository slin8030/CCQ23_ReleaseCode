/** @file
 DXE Chipset Services Library.

 Perform load default of setup menu. Chipset can use it to load default of
 setup related variables.

;***************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Protocol/H2ODxeChipsetServices.h>

//
// Global Variables (This Source File Only)
//
extern H2O_CHIPSET_SERVICES_PROTOCOL *mChipsetSvc;

/**
 Perform load default of setup menu. Chipset can use it to load the default of setup related variables.

 @retval EFI_SUCCESS         Function returns successfully.
 @retval EFI_UNSUPPORTED     Function is not implemented.
*/
EFI_STATUS
EFIAPI
DxeCsSvcLoadDefaultSetupMenu (
  VOID
  )
{
  //
  // Verify that the protocol interface structure contains the function
  // pointer and whether that function pointer is non-NULL. If not, return
  // an error.
  //
  if (mChipsetSvc == NULL ||
      mChipsetSvc->Size < (OFFSET_OF (H2O_CHIPSET_SERVICES_PROTOCOL, LoadDefaultSetupMenu) + sizeof (VOID*)) ||
      mChipsetSvc->LoadDefaultSetupMenu == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O DXE Chipset Services can not be found or member LoadDefaultSetupMenu() isn't implement!\n"));
    return EFI_UNSUPPORTED;
  }
  return mChipsetSvc->LoadDefaultSetupMenu ();
}

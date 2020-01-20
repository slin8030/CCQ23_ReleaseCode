/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/



#include <EfiFastBootPeiLib.h>


//
// Launches the OSIP image
//

EFI_STATUS
EFIAPI
FastbootStartImage (
  IN UINT8  *Buffer
  )
{
  //TODO -m InstallLegacyAcpi();
  //TODO -m BdsBootAndroidFromEmmc (Buffer);
  return EFI_SUCCESS;
}

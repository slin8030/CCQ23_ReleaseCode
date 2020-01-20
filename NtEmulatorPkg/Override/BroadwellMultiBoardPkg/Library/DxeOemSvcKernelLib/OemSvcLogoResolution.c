/** @file
  Provide OEM to customize resolution and SCU resolution. .

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/DxeOemSvcKernelLib.h>

#define  OEM_LOGO_RESOULTION              0x00


//
// module variables
//
OEM_LOGO_RESOLUTION_DEFINITION mOemLogoResolutionTable = {
  //
  // If set logo resolution to 0, the logo resolution will be get from GetLogoResolution() function.
  // Otherwise, it will use user prefer Resolution: 1024x768, 800x600, 640x480... .
  // OEM_LOGO_RESOULTION(resoultion depend on OEM logo)
  //
  OEM_LOGO_RESOULTION,       // LogoResolutionX;
  OEM_LOGO_RESOULTION,       // LogoResolutionY;
  //
  // SCU can support different resolutions ex:640x480, 800x600, 1024x768...
  // If user changes the resolution, please make sure
  // $(EFI_SOURCE)\$(PROJECT_FAMILY)\$(PROJECT_FAMILY)\OemServices\Dxe\UpdateFormLen\UpdateFormLen.c
  // also updating to correct value.
  //
  1024,                       // SCUResolutionX;
  768                        // SCUResolutionY;
};


/**
  Get OEM customization resolution and SCU resolution.
  Customization resolution provides OEM to define logo resolution directly or call GetLogoResolution () function to get logo resolution automatically.

  @param[out]  **OemLogoResolutionTable    A double pointer to OEM_LOGO_RESOLUTION_DEFINITION

  @retval      EFI_UNSUPPORTED             Returns unsupported by default.
  @retval      EFI_SUCCESS                 Get resolution information successful.
  @retval      EFI_MEDIA_CHANGED           The value of IN OUT parameter is changed.
  @retval      Others                      Base on OEM design.
**/
EFI_STATUS
OemSvcLogoResolution (
  IN OUT OEM_LOGO_RESOLUTION_DEFINITION    **OemLogoResolutionTable
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/
  if (*OemLogoResolutionTable != NULL) {

    if ((*OemLogoResolutionTable)->LogoResolutionX != 800 &&
        (*OemLogoResolutionTable)->LogoResolutionY != 600) {
      //
      // Please refer to gInsydeTokenSpaceGuid.PcdDefaultLogoResolution for default resolution
      //

      //
      // If the resolution is different from default vaules,
      // that means native resolution was found, and we only provides SCU resolution here
      //
      (*OemLogoResolutionTable)->ScuResolutionX = mOemLogoResolutionTable.ScuResolutionX;
      (*OemLogoResolutionTable)->ScuResolutionY = mOemLogoResolutionTable.ScuResolutionY;
      return EFI_MEDIA_CHANGED;
    }
  }

  *OemLogoResolutionTable = &mOemLogoResolutionTable;


  return EFI_SUCCESS;
}


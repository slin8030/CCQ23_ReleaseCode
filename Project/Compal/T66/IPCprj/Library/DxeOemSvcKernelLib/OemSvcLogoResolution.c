/** @file
  Provide OEM to customize resolution and SCU resolution. .

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <PiPei.h>                
#include <ChipsetSetupConfig.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/OEMBadgingSupport.h>
#include <Protocol/JpegDecoder.h>
#include <Protocol/PcxDecoder.h>
#include <Protocol/TgaDecoder.h>
#include <Protocol/GifDecoder.h>
#include <Protocol/BmpDecoder.h>
#include <Protocol/SetupUtility.h>
#include <Library/PcdLib.h>
#include <Library/HobLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <Library/BaseMemoryLib.h>

//
// data type definitions
//
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
//PRJ+>>>> Modify Logo and SCU resolution to 800x1280  
  800,                       // SCUResolutionX;
  1280                        // SCUResolutionY;
//PRJ+<<<< Modify Logo and SCU resolution to 800x1280    
};

BOOLEAN   mUpdateLogoResolution = FALSE;
STATIC OEM_LOGO_RESOLUTION_DEFINITION mReturnResolutionTable;

STATIC
VOID
SwapResolutionTableXY (
  OEM_LOGO_RESOLUTION_DEFINITION        *ResolutionTable
  )
{
  UINT32                                TmpX;
//[-start-170124-IB07400837-add]//
  EFI_STATUS                            Status;
  EFI_SETUP_UTILITY_PROTOCOL            *SetupUtility;
  CHIPSET_CONFIGURATION                 *SystemConfiguration = NULL;
//[-end-170124-IB07400837-add]//
  
//[-start-170124-IB07400837-add]//
  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityProtocolGuid,
                  NULL,
                  (VOID **)&SetupUtility
                  );
  SystemConfiguration = (CHIPSET_CONFIGURATION *) SetupUtility->SetupNvData;
  if ((EFI_ERROR (Status)) || 
      (!EFI_ERROR(Status) && (SystemConfiguration->RotateScreenConfig == 0))) {
    return;
  }
//[-end-170124-IB07400837-add]//


  TmpX = ResolutionTable->LogoResolutionX;
  ResolutionTable->LogoResolutionX = ResolutionTable->LogoResolutionY;
  ResolutionTable->LogoResolutionY = TmpX;

  TmpX = ResolutionTable->ScuResolutionX;
  ResolutionTable->ScuResolutionX = ResolutionTable->ScuResolutionY;
  ResolutionTable->ScuResolutionY = TmpX;
}




//
// helper function prototypes
//
EFI_STATUS
GetLogoResolution (
  OUT UINT32                            *LogoResolutionX,
  OUT UINT32                            *LogoResolutionY
  );

/**
  Initialize module variable mPostKeyToOperation.

  @param  LogoResolutionX       Horizontal Logo resolution. 
  @param  LogoResolutionY       Vertical Logo resolution. 

  @retval EFI_UNSUPPORTED       Unsupported logo format. 
**/
EFI_STATUS
GetLogoResolution (
  OUT UINT32                            *LogoResolutionX,
  OUT UINT32                            *LogoResolutionY
  )
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL                  *GraphicsOutput;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL                 *Blt;
  UINTN                                         BltSize;
  EFI_JPEG_DECODER_PROTOCOL                     *JpegDecoder;
  EFI_JPEG_DECODER_STATUS                       DecoderStatus;
  EFI_PCX_DECODER_PROTOCOL                      *PcxDecoder;
  EFI_TGA_DECODER_PROTOCOL                      *TgaDecoder;
  EFI_BMP_DECODER_PROTOCOL                      *BmpDecoder;
  TGA_FORMAT                                    TgaFormate;
  BOOLEAN                                       HasAlphaChannel;
  EFI_OEM_BADGING_SUPPORT_PROTOCOL              *Badging;
  EFI_BADGING_SUPPORT_DISPLAY_ATTRIBUTE         Attribute;
  EFI_BADGING_SUPPORT_FORMAT                    Format;
  EFI_BADGING_SUPPORT_IMAGE_TYPE                Type;
  UINT8                                         *ImageData;
  UINTN                                         ImageSize;
  UINTN                                         CoordinateX;
  UINTN                                         CoordinateY;
  UINT32                                        Instance;
  UINTN                                         Height;
  UINTN                                         Width;
  UINTN                                         LocX;
  UINTN                                         LocY;
  EFI_STATUS                                    Status;
  EFI_GIF_DECODER_PROTOCOL                      *GifDecoder; 
  ANIMATION                                     *Animation; 

  Status = gBS->HandleProtocol (gST->ConsoleOutHandle, &gEfiGraphicsOutputProtocolGuid, (VOID **)&GraphicsOutput);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Badging = NULL;
  Status  = gBS->LocateProtocol (&gEfiOEMBadgingSupportProtocolGuid, NULL, (VOID **)&Badging);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Set the request for the first image of type Logo
  //
  Instance = 0;
  Type = EfiBadgingSupportImageLogo;
  Format = EfiBadgingSupportFormatBMP;

  while (TRUE) {
    ImageData = NULL;
    ImageSize = 0;

    Status = Badging->GetImage (
                        Badging,
                        &Instance,
                        &Type,
                        &Format,
                        &ImageData,
                        &ImageSize,
                        &Attribute,
                        &CoordinateX,
                        &CoordinateY
                        );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // Determine and video resolution based on the logo
    //
    if (Type != EfiBadgingSupportImageLogo) {
      gBS->FreePool (ImageData);
      continue;
    }
    Blt             = NULL;
    BltSize         = 0;
    Height          = 0;
    Width           = 0;
    TgaFormate      = UnsupportedTgaFormat;
    HasAlphaChannel = FALSE;

    switch (Format) {

    case EfiBadgingSupportFormatBMP:
      Status = gBS->LocateProtocol (
                      &gEfiBmpDecoderProtocolGuid,
                      NULL,
                      (VOID **)&BmpDecoder
                      );
      if (!EFI_ERROR (Status)) {
        Status = BmpDecoder->DecodeImage (
                             BmpDecoder,
                             ImageData,
                             ImageSize,
                             (UINT8 **)&Blt,
                             &BltSize,
                             &Height,
                             &Width
                             );
      }
      break;

    case EfiBadgingSupportFormatTGA:
      Status = gBS->LocateProtocol (
                      &gEfiTgaDecoderProtocolGuid,
                      NULL,
                      (VOID **)&TgaDecoder
                      );
      if (!EFI_ERROR (Status)) {
        Status = TgaDecoder->DecodeImage (
                               TgaDecoder,
                               ImageData,
                               ImageSize,
                               (UINT8 **)&Blt,
                               &BltSize,
                               &Height,
                               &Width,
                               &TgaFormate,
                               &HasAlphaChannel
                               );
      }
      break;

    case EfiBadgingSupportFormatJPEG:
      Status = gBS->LocateProtocol (
                      &gEfiJpegDecoderProtocolGuid,
                      NULL,
                      (VOID **)&JpegDecoder
                      );
      if (!EFI_ERROR (Status)) {
        Status = JpegDecoder->DecodeImage (
                                JpegDecoder,
                                ImageData,
                                ImageSize,
                                (UINT8 **)&Blt,
                                &BltSize,
                                &Height,
                                &Width,
                                &DecoderStatus
                                );
      }
      break;

    case EfiBadgingSupportFormatPCX:
      Status = gBS->LocateProtocol (
                      &gEfiPcxDecoderProtocolGuid,
                      NULL,
                      (VOID **)&PcxDecoder
                      );
      if (!EFI_ERROR (Status)) {
        Status = PcxDecoder->DecodeImage (
                             PcxDecoder,
                             ImageData,
                             ImageSize,
                             (UINT8 **)&Blt,
                             &BltSize,
                             &Height,
                             &Width
                             );
      }
      break;
      
    case EfiBadgingSupportFormatGIF:
      Status = gBS->LocateProtocol (
                      &gEfiGifDecoderProtocolGuid,
                      NULL,
                      (VOID **)&GifDecoder
                      );
      if (!EFI_ERROR (Status)) {
        Status = GifDecoder->CreateAnimationFromMem (
                               GifDecoder, 
                               ImageData,
                               ImageSize, 
                               NULL, 
                               &Animation
                               ); 
        if (!EFI_ERROR (Status)) {
          Width  = Animation->Width; 
          Height = Animation->Height; 

          GifDecoder->DestroyAnimation (
                        GifDecoder, 
                        Animation
                        ); 
        }
      }
      break;

    default:
      Status = EFI_UNSUPPORTED;
      break;
    }

    if (EFI_ERROR (Status)) {
      gBS->FreePool (ImageData);
      continue;
    }

    gBS->FreePool (ImageData);
    gBS->FreePool (Blt);

    if (Badging->OemVideoModeScrStrXY (Badging, OemSupportedVideoMode, (UINT32)Width, (UINT32)Height, &LocX, &LocY)) {
      *LogoResolutionX = (UINT32) Width;
      *LogoResolutionY = (UINT32) Height;
      break;
    }
  }

  return Status;
}

/**
  Get OEM customization resolution and SCU resolution. 
  Customization resolution provides OEM to define logo resolution directly or call GetLogoResolution () function to get logo resolution automatically.

  @param[out]  **OemLogoResolutionTable    A double pointer to OEM_LOGO_RESOLUTION_DEFINITION

  @retval      EFI_MEDIA_CHANGED           Get resolution information successful.
  @retval      Others                      Base on OEM design.
**/
EFI_STATUS
OemSvcLogoResolution (
  IN OUT OEM_LOGO_RESOLUTION_DEFINITION    **OemLogoResolutionTable
  )
{
  EFI_SETUP_UTILITY_PROTOCOL            *SetupUtility;
  CHIPSET_CONFIGURATION                  *SystemConfiguration = NULL;
  EFI_STATUS                            Status;
  EFI_BOOT_MODE                         BootMode;
  UINT32                                OemLogoX;
  UINT32                                OemLogoY;
//[-start-170608-IB07400873-add]//
  UINT8                                 LogoScuResolution;
//[-end-170608-IB07400873-add]//

  SetupUtility = NULL;
  Status = EFI_SUCCESS;
  
//[-start-160720-IB07400761-add]//
  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityProtocolGuid,
                  NULL,
                  (VOID **)&SetupUtility
                  );
  SystemConfiguration = (CHIPSET_CONFIGURATION *) SetupUtility->SetupNvData;
  if (SystemConfiguration->LogoScuResolution != 0) {
//[-start-170608-IB07400873-modify]//
    //
    // Check this resolution is supported or not
    //
    LogoScuResolution = SystemConfiguration->LogoScuResolution;
    if (PcdGet32 (PcdSupportedLogoScuResolution) != 0) {
      for (LogoScuResolution = SystemConfiguration->LogoScuResolution;
           LogoScuResolution > 0; LogoScuResolution--) {
        if (PcdGet32 (PcdSupportedLogoScuResolution) & (1 << LogoScuResolution)) {
          break;
        }
      }
    }
    switch (LogoScuResolution) {  
    case 1: // 640 x 480 (80 x 25)
      OemLogoX = 640;
      OemLogoY = 480;
      break;
    case 2: // 800 x 600 (100 x 31)
      OemLogoX = 800;
      OemLogoY = 600;
      break;
    case 3: // 1024 x 768 (128 x 40)
      OemLogoX = 1024;
      OemLogoY = 768;
      break;
    default: // Default set to 640 x 480 for unsupported video card.
      OemLogoX = 640;
      OemLogoY = 480;
      break;
    }
//[-end-170608-IB07400873-modify]//
      
//[-start-160927-IB07400789-add]//
    if (*OemLogoResolutionTable == NULL) {
      (*OemLogoResolutionTable) = &mOemLogoResolutionTable;
    }
//[-end-160927-IB07400789-add]//
//PRJ+>>>> Modify Logo and SCU resolution to 800x1280 
    OemLogoX = 800;
    OemLogoY = 1280;
//PRJ+<<<< Modify Logo and SCU resolution to 800x1280  
    (*OemLogoResolutionTable)->LogoResolutionX = OemLogoX;
    (*OemLogoResolutionTable)->LogoResolutionY = OemLogoY;
    (*OemLogoResolutionTable)->ScuResolutionX = OemLogoX;
    (*OemLogoResolutionTable)->ScuResolutionY = OemLogoY;
    
//[-start-170124-IB07400837-add]//
    if (FeaturePcdGet (PcdH2ORotateScreenSupported) && PcdGetBool (PcdH2ORotateScreenRotateLogo)) {
      SwapResolutionTableXY (*OemLogoResolutionTable);
    }
//[-end-170124-IB07400837-add]//

    return EFI_MEDIA_CHANGED;
  }
//[-end-160720-IB07400761-add]//

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

      if (FeaturePcdGet (PcdH2ORotateScreenSupported) && PcdGetBool (PcdH2ORotateScreenRotateLogo)) {
        SwapResolutionTableXY (*OemLogoResolutionTable);
      }
      return EFI_MEDIA_CHANGED;
    } 
  }
  if (!mUpdateLogoResolution) {
    BootMode = GetBootModeHob ();
    Status = gBS->LocateProtocol (
                    &gEfiSetupUtilityProtocolGuid,
                    NULL,
                    (VOID **)&SetupUtility
                    );
    SystemConfiguration = (CHIPSET_CONFIGURATION *) SetupUtility->SetupNvData;

    if ( ( BootMode == BOOT_IN_RECOVERY_MODE ) || ( SystemConfiguration->QuietBoot == FALSE ) ) {
      mOemLogoResolutionTable.LogoResolutionX = mOemLogoResolutionTable.ScuResolutionX;
      mOemLogoResolutionTable.LogoResolutionY = mOemLogoResolutionTable.ScuResolutionY;
    } else if ( mOemLogoResolutionTable.LogoResolutionX == OEM_LOGO_RESOULTION ) {
      Status = GetLogoResolution (&OemLogoX, &OemLogoY);
      if (!EFI_ERROR (Status)) {
        mOemLogoResolutionTable.LogoResolutionX = OemLogoX;
        mOemLogoResolutionTable.LogoResolutionY = OemLogoY;
        if (*OemLogoResolutionTable != NULL) {
          (*OemLogoResolutionTable)->LogoResolutionX = mOemLogoResolutionTable.LogoResolutionX;
          (*OemLogoResolutionTable)->LogoResolutionY = mOemLogoResolutionTable.LogoResolutionY;
        }
      } else {
        mOemLogoResolutionTable.LogoResolutionX = 800;
        mOemLogoResolutionTable.LogoResolutionY = 600;
      }
    }
    mUpdateLogoResolution = TRUE;
  }

  if (*OemLogoResolutionTable == NULL) {
    //
    // The table is only NULL when the caller is an Ecp driver,
    // this case can be removed when there's no Ecp caller.
    //
    
    //
    // No native resolution is found
    // Use resolution table above
    //
    (*OemLogoResolutionTable) = &mOemLogoResolutionTable;
    if (FeaturePcdGet (PcdH2ORotateScreenSupported) && PcdGetBool (PcdH2ORotateScreenRotateLogo)) {
      CopyMem (&mReturnResolutionTable, &mOemLogoResolutionTable, sizeof (OEM_LOGO_RESOLUTION_DEFINITION));
      SwapResolutionTableXY (&mReturnResolutionTable);
      (*OemLogoResolutionTable) = &mReturnResolutionTable;
    }

  } else {
    (*OemLogoResolutionTable)->ScuResolutionX = mOemLogoResolutionTable.ScuResolutionX;
    (*OemLogoResolutionTable)->ScuResolutionY = mOemLogoResolutionTable.ScuResolutionY;
    if (FeaturePcdGet (PcdH2ORotateScreenSupported) && PcdGetBool (PcdH2ORotateScreenRotateLogo)) {
      SwapResolutionTableXY (*OemLogoResolutionTable);
    }
  }
  return EFI_MEDIA_CHANGED;
}


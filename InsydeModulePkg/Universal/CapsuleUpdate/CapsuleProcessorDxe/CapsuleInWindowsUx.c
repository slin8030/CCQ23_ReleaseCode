/** @file
  WindowsUx capsule processor on firmware management protocol implementation 

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "CapsuleProcessorDxe.h"

STATIC  CAPSULE_FMP_ENTRY       mCapsuleFmpEntry;

#pragma pack(1)
typedef struct {
  UINT8         Version;
  UINT8         Checksum;
  UINT8         ImageType;
  UINT8         Reserved;
  UINT32        Mode;
  UINT32        ImageOffsetX;
  UINT32        ImageOffsetY;
} WINDOWS_UX_CAPSULE_HEADER;
#pragma pack()

/**
  Dummy function for SetMode() of EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL

  @param  This       The protocol instance pointer.
  @param  ModeNumber The mode number to set.

  @retval EFI_SUCCESS      The requested text mode was set.

**/
STATIC
EFI_STATUS
EFIAPI
DummySetMode (
  IN     EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL        *This,
  IN     UINTN                                  ModeNumber
  )
{
  return EFI_SUCCESS;
}

/**
  Dummy function for SetMode() of EFI_GRAPHICS_OUTPUT_PROTOCOL

  @param  This       The protocol instance pointer.
  @param  ModeNumber The mode number to set.

  @retval EFI_SUCCESS      The requested text mode was set.

**/
STATIC
EFI_STATUS
EFIAPI
DummyGraphicsSetMode (
  IN     EFI_GRAPHICS_OUTPUT_PROTOCOL           *This,
  IN     UINT32                                 ModeNumber
  )
{
  return EFI_SUCCESS;
}

/**
  Dummy function for ClearScreen() of EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL

  @param  This              The protocol instance pointer.

  @retval  EFI_SUCCESS      The operation completed successfully.

**/
STATIC
EFI_STATUS
EFIAPI
DummyClearScreen (
  IN     EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL        *This
  )
{
  return EFI_SUCCESS;
}

/**
  Dummy function for OutputString() of EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL

  @param  This              The protocol instance pointer.
  @param  String            The NULL-terminated string to be displayed

  @retval  EFI_SUCCESS      The operation completed successfully.

**/
STATIC
EFI_STATUS
EFIAPI
DummyOutputString (
  IN     EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL        *This,
  IN     CHAR16                                 *String
  )
{
  return EFI_SUCCESS;
}

/**
  Disable the display of firmware update utility, the display will be controlled
  by Windows UX Capsule image if Windows Update is triggered

  @retval  EFI_SUCCESS      The operation completed successfully.

**/
STATIC
EFI_STATUS
DisableVendorUserInterface (
  VOID
  )
{
  EFI_STATUS                    Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;

  gST->ConOut->SetMode      = DummySetMode;
  gST->ConOut->ClearScreen  = DummyClearScreen;
  gST->ConOut->OutputString = DummyOutputString;
  Status = gBS->HandleProtocol (
                  gST->ConsoleOutHandle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **)&GraphicsOutput
                  );
  if (Status == EFI_SUCCESS) {
    GraphicsOutput->SetMode = DummyGraphicsSetMode;
  }
  return EFI_SUCCESS;
}

/**
  Display the boot logo

  @retval  EFI_SUCCESS      The operation completed successfully.
  @retval  !EFI_SUCCESS     Failed on displaying the boot logo

**/
STATIC
EFI_STATUS
DisplayBootLogo (
  VOID
  )
{
  EFI_STATUS                    Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer;
  UINTN                         DestinationX;
  UINTN                         DestinationY;
  UINTN                         Width;
  UINTN                         Height;

  BltBuffer    = NULL;
  DestinationX = 0;
  DestinationY = 0;
  Width        = 0;
  Height       = 0;

  Status = BgrtGetBootImageInfo (
             &BltBuffer,
             &DestinationX,
             &DestinationY,
             &Width,
             &Height
             );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->EnableCursor (gST->ConOut, FALSE);

  Status = gBS->HandleProtocol (
                  gST->ConsoleOutHandle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **)&GraphicsOutput
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = GraphicsOutput->Blt (
                             GraphicsOutput,
                             BltBuffer,
                             EfiBltBufferToVideo,
                             0,
                             0,
                             DestinationX,
                             DestinationY,
                             Width,
                             Height,
                             Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                             );

  if (BltBuffer != NULL) {
    gBS->FreePool (BltBuffer);
  }
  return Status;
}

/**
  Display Windows UX capsule image

  This function updates the hardware with the new firmware image.
  This function returns EFI_UNSUPPORTED if the firmware image is not updatable.
  If the firmware image is updatable, the function should perform the following minimal validations
  before proceeding to do the firmware image update.
  - Validate the image authentication if image has attribute 
    IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED.  The function returns 
    EFI_SECURITY_VIOLATION if the validation fails.
  - Validate the image is a supported image for this device.  The function returns EFI_ABORTED if 
    the image is unsupported.  The function can optionally provide more detailed information on 
    why the image is not a supported image.
  - Validate the data from VendorCode if not null.  Image validation must be performed before 
    VendorCode data validation.  VendorCode data is ignored or considered invalid if image 
    validation failed.  The function returns EFI_ABORTED if the data is invalid.
  
  VendorCode enables vendor to implement vendor-specific firmware image update policy.  Null if 
  the caller did not specify the policy or use the default policy.  As an example, vendor can implement 
  a policy to allow an option to force a firmware image update when the abort reason is due to the new 
  firmware image version is older than the current firmware image version or bad image checksum.  
  Sensitive operations such as those wiping the entire firmware image and render the device to be
  non-functional should be encoded in the image itself rather than passed with the VendorCode.
  AbortReason enables vendor to have the option to provide a more detailed description of the abort
  reason to the caller.

  @param[in]  This               A pointer to the EFI_FIRMWARE_MANAGEMENT_PROTOCOL instance.
  @param[in]  ImageIndex         A unique number identifying the firmware image(s) within the device.
                                 The number is between 1 and DescriptorCount.
  @param[in]  Image              Points to the new image.
  @param[in]  ImageSize          Size of the new image in bytes.
  @param[in]  VendorCode         This enables vendor to implement vendor-specific firmware image update policy.  
                                 Null indicates the caller did not specify the policy or use the default policy.
  @param[in]  Progress           A function used by the driver to report the progress of the firmware update.
  @param[out] AbortReason        A pointer to a pointer to a null-terminated string providing more
                                 details for the aborted operation. The buffer is allocated by this function 
                                 with AllocatePool(), and it is the caller's responsibility to free it with a
                                 call to FreePool().

  @retval EFI_SUCCESS            The device was successfully updated with the new image.
  @retval EFI_ABORTED            The operation is aborted.
  @retval EFI_INVALID_PARAMETER  The Image was NULL.
  @retval EFI_UNSUPPORTED        The operation is not supported.
  @retval EFI_SECURITY_VIOLATION The operation could not be performed due to an authentication failure.

**/
EFI_STATUS
EFIAPI 
WindowsUxSetImage (
  IN     EFI_FIRMWARE_MANAGEMENT_PROTOCOL               *This,
  IN     UINT8                                          ImageIndex,
  IN     CONST VOID                                     *Capsule,
  IN     UINTN                                          CapsuleSize,
  IN     CONST VOID                                     *VendorCode,
  IN     EFI_FIRMWARE_MANAGEMENT_UPDATE_IMAGE_PROGRESS  Progress,
  OUT    CHAR16                                         **AbortReason
  )
{
  EFI_STATUS                    Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;
  WINDOWS_UX_CAPSULE_HEADER     *WindowsUxCapsule;
  UINT8                         *ImageData;
  UINTN                         ImageSize;
  UINTN                         BltSize;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt;
  UINTN                         Height;
  UINTN                         Width;

  DisplayBootLogo ();
  WindowsUxCapsule = (WINDOWS_UX_CAPSULE_HEADER*)((UINT8*)Capsule + ((EFI_CAPSULE_HEADER*)Capsule)->HeaderSize);
  ImageData = (UINT8*)(WindowsUxCapsule + 1);
  ImageSize = CapsuleSize - (((EFI_CAPSULE_HEADER*)Capsule)->HeaderSize + sizeof (WINDOWS_UX_CAPSULE_HEADER));
  Blt       = NULL;
  Status = ConvertBmpToGopBlt (
             ImageData,
             ImageSize,
             (VOID **)&Blt,
             &BltSize,
             &Height,
             &Width
             );
  Status = gBS->HandleProtocol (
                  gST->ConsoleOutHandle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **)&GraphicsOutput
                  );
  if (!EFI_ERROR(Status)) {
    Status = GraphicsOutput->Blt (
                               GraphicsOutput,
                               Blt,
                               EfiBltBufferToVideo,
                               0,
                               0,
                               WindowsUxCapsule->ImageOffsetX,
                               WindowsUxCapsule->ImageOffsetY,
                               Width,
                               Height,
                               Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                               );
  }
  if (Blt != NULL) {
    gBS->FreePool (Blt);
  }
  if (!EFI_ERROR (Status)) {
    DisableVendorUserInterface();
  }
  return Status;
}

/**

  Installation of capsule in WindowsUx processor

**/
EFI_STATUS
InstallCapsuleInWindowsUx (
  VOID
  )
{
  mCapsuleFmp.CapsuleProcessorCount ++;
  mCapsuleFmpEntry.ImageIndex = (UINT8)mCapsuleFmp.CapsuleProcessorCount;
  CopyGuid (&mCapsuleFmpEntry.ImageTypeGuid, &gWindowsUxCapsuleGuid);
  mCapsuleFmpEntry.SetImage   = WindowsUxSetImage;
  mCapsuleFmpEntry.CheckImage = NULL;
  InsertTailList (&mCapsuleFmp.CapsuleProcessorListHead, (LIST_ENTRY*)&mCapsuleFmpEntry);
  return EFI_SUCCESS;
}
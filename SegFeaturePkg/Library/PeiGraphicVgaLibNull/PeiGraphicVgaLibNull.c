/** @file
  Null PeiGraphicVgaLib Library instance does nothing and returns unsupport status.
  
;******************************************************************************
;* Copyright (c) 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/GraphicVgaLib.h>
  
/**
  Output the JPEG to console device

 @param[in]          DenstinationX - where Row start to output the Bitmap
 @param[in]          DenstinationY - where column start to output the Bitmap
 @param[in]          ImageData - Image data
 @param[in]          ImageSize - Image Size
 @param[in]          BackGround - The Bitmap is background or not
 @param[in]          Center - Output the Bitmap at Center

 @retval               EFI_SUCCESS           - The function completed successfully.
*/
EFI_STATUS
GraphicVgaShowJpeg (
  IN UINT32                                 DenstinationX,
  IN UINT32                                 DenstinationY,
  IN UINT8                                  *ImageData,
  IN UINTN                                  ImageSize,
  IN PROGRESS_BAR_DATA                      ProgressBarData,
  IN BOOLEAN                                BackGround,
  IN BOOLEAN                                Center
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Output the PNG to console device

 @param[in]          DenstinationX   - where Row start to output the Bitmap
 @param[in]          DenstinationY   - where column start to output the Bitmap
 @param[in]          ImageData - Image data
 @param[in]          ImageSize - Image Size
 @param[in]          ProgressBarData - Settings of progress bar
 @param[in]          BackGround      - The Bitmap is background or not
 @param[in]          Center          - Output the Bitmap at Center

 @retval             EFI_SUCCESS     - The function completed successfully.
*/
EFI_STATUS
GraphicVgaShowPng (
  IN UINT32                                 DenstinationX,
  IN UINT32                                 DenstinationY,
  IN UINT8                                  *ImageData,
  IN UINTN                                  ImageSize,
  IN PROGRESS_BAR_DATA                      ProgressBarData,
  IN BOOLEAN                                BackGround,
  IN BOOLEAN                                Center
  )
{
  return EFI_UNSUPPORTED;
}
  
 /**
  Output the Bitmap to console device

 @param[in]          DenstinationX - where Row start to output the Bitmap
 @param[in]          DenstinationY - where column start to output the Bitmap
 @param[in]          ImageData - Image data
 @param[in]          ImageSize - Image Size
 @param[in]          BackGround - The Bitmap is background or not
 @param[in]          Center - Output the Bitmap at Center

 @retval               EFI_SUCCESS           - The function completed successfully.
*/
EFI_STATUS
GraphicVgaShowBitmap (
  IN UINT32                                 DenstinationX,
  IN UINT32                                 DenstinationY,
  IN UINT8                                  *ImageData,
  IN UINTN                                  ImageSize,
  IN BOOLEAN                                BackGround,
  IN BOOLEAN                                Center
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Output the Image to console device

 @param[in]          DenstinationX - where Row start to output the Bitmap
 @param[in]          DenstinationY - where column start to output the Bitmap
 @param[in]          ImageData - Image data
 @param[in]          ImageSize - Image Size
 @param[in]          BackGround - The Bitmap is background or not
 @param[in]          Center - Output the Bitmap at Center

 @retval               EFI_SUCCESS           - The function completed successfully.
*/
EFI_STATUS
GraphicVgaShowImage (
  IN UINT32                                 DenstinationX,
  IN UINT32                                 DenstinationY,
  IN UINT8                                  *ImageData,
  IN UINTN                                  ImageSize,
  IN BOOLEAN                                BackGround,
  IN BOOLEAN                                Center
  )
{
  return EFI_UNSUPPORTED;
}
  
/**
 This function shows the input string on the specified position of screen.

 @param [in] DenstinationX    The X coordinate of position
 @param [in] DenstinationY    The Y coordinate of position
 @param [in] WordColor        The string word color
 @param [in] BackGroundColor  The strgig background color
 @param [in] String           The string which will be shown
 
 @retval EFI_SUCCESS   String was successfully shown on the specified position
 @return Others        Show string error

**/
EFI_STATUS
EFIAPI
GraphicVgaShowString (
  IN UINT16 DenstinationX,
  IN UINT16 DenstinationY,
  IN UINT32 WordColor,
  IN UINT32 BackGroundColor,
  IN UINT8  *String
  )
{
  return EFI_UNSUPPORTED;
}


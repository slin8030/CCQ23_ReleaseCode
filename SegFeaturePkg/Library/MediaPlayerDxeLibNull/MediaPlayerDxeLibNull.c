/** @file
 Media Player Null Library.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/

#include <Uefi.h>
#include <Library/MediaPlayerDxeLib.h>

/**
 According to the policy to play media file.
 Play by GUID name or raw data.

 @param[in]  Info                            A struct contains the information of playing media file.
 @param[in]  *GuidName                   Point to the GUID name.
 @param[in]  *RawData                    Point to the raw data located address.
 @param[in]  RawDataSize                The WAV raw data size.
 @param[in]  *DecoderType              Current running decoder type.
  
 @retval EFI_UNSUPPORTED
**/
EFI_STATUS
MediaPlayerDxeLibPlay (
  IN   H2O_MEDIA_PLAYER_INFO_LIB                           Info,
  IN   EFI_GUID                                            *GuidName,
  IN   UINT8                                               *RawData,
  IN   UINT32                                              RawDataSize,
  OUT  H2O_MEDIA_PLAYER_DECODER_TYPE_LIB                   *DecoderType
  )
{
  return EFI_UNSUPPORTED;
}

/** 
 Pause current present media file by inputing decoder type.
            
 @param[in]  *DecoderType              Decoder type.
  
 @retval EFI_UNSUPPORTED
**/
EFI_STATUS
MediaPlayerDxeLibPause (
  IN  H2O_MEDIA_PLAYER_DECODER_TYPE_LIB   DecoderType
  )
{
  return EFI_UNSUPPORTED;
}

/** 
 Stop current present media file by inputing decoder type.
            
 @param[in]  *DecoderType              Decoder type.
  
 @retval EFI_UNSUPPORTED
**/
EFI_STATUS
MediaPlayerDxeLibStop (
  IN  H2O_MEDIA_PLAYER_DECODER_TYPE_LIB   DecoderType
  )
{
  return EFI_UNSUPPORTED;
}

/** 
 Continue current present media file by inputing decoder type.
            
 @param[in]  *DecoderType              Decoder type.
  
 @retval EFI_UNSUPPORTED
**/
EFI_STATUS
MediaPlayerDxeLibContinue (
  IN  H2O_MEDIA_PLAYER_DECODER_TYPE_LIB   DecoderType
  )
{
  return EFI_UNSUPPORTED;
}

/** 
  ZoomIn
            
  @param[in]  *DecoderType              Decoder type.
  
  @retval EFI_UNSUPPORTED
**/
EFI_STATUS
MediaPlayerDxeLibZoomIn (
  IN  H2O_MEDIA_PLAYER_DECODER_TYPE_LIB   DecoderType
  )
{
  return EFI_UNSUPPORTED;
}

/** 
  ZoomOut

  @param[in]  *DecoderType              Decoder type.
  
  @retval EFI_UNSUPPORTED                  
**/
EFI_STATUS
MediaPlayerDxeLibZoomOut (
  IN  H2O_MEDIA_PLAYER_DECODER_TYPE_LIB   DecoderType
  )
{
  return EFI_UNSUPPORTED;
}


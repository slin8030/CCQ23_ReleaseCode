/** @file
 Media Player library header file for SEG feature package.

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

//
// Server Feature - Insyde MediaPlayer Package V1.0
//

#ifndef _MEDIA_PLAYER_SEGPKG_LIB_H_
#define _MEDIA_PLAYER_SEGPKG_LIB_H_

//
//  Enumeration for the source of media file.
//
typedef enum {
  MediaPlayerByRawData,
  MediaPlayerByGuidName,
  MediaPlayerFileTypeMax
} H2O_MEDIA_PLAYER_FILE_LIB;

//
//  Enumeration for the play function of media player.
//
typedef enum {
  MediaPlayerPlayOnce,
  MediaPlayerRepeatPlay,
  MediaPlayerPlayMethodMax
} H2O_MEDIA_PLAYER_METHOD_LIB;

//
//  Enumeration for current supported decoder.
//
typedef enum {
  MediaPlayerGifLib,
  MediaPlayerMP3Lib,
  MediaPlayerWaveLib,
  MediaPlayerXvid,
  MediaPlayerH264,
  MediaPlayerTypeMax
} H2O_MEDIA_PLAYER_DECODER_TYPE_LIB;

//
//  Enumeration for the display position of AnimationGifDxe
//
typedef enum {
  MediaPlayerMiddleCentered,
  MediaPlayerTopCentered,
  MediaPlayerBottomCentered,
  MediaPlayerLeftCentered,
  MediaPlayerRightCentered,
  MediaPlayerAcpiBgrtPosition,
  MediaPlayerCustomizedPosition
} H2O_MEDIA_PLAYER_POSITION_LIB;

//
//  Enumeration for video resize function
//
typedef enum {
  MediaPlayerVideoZoomIn,
  MediaPlayerVideoZoomOut,
  MediaPlayerVideoResizeMax
} H2O_MEDIA_PLAYER_VIDEO_RESIZE_LIB;

typedef struct {
  H2O_MEDIA_PLAYER_FILE_LIB            Source;
  H2O_MEDIA_PLAYER_METHOD_LIB          Method;
  H2O_MEDIA_PLAYER_POSITION_LIB        Position;
  UINTN                                CoordinateX;
  UINTN                                CoordinateY;
  EFI_STATUS                           (*CallBackFunc) (); 
} H2O_MEDIA_PLAYER_INFO_LIB;

/**
 According to the policy to play media file.
 Play by GUID name or raw data.

 @param[in]  Info                            A struct contains the information of playing media file.
 @param[in]  *GuidName                   Point to the GUID name.
 @param[in]  *RawData                    Point to the raw data located address.
 @param[in]  RawDataSize                The WAV raw data size.
 @param[in]  *DecoderType              Current running decoder type.
  
 @retval EFI Status
**/
EFI_STATUS
MediaPlayerDxeLibPlay (
  IN   H2O_MEDIA_PLAYER_INFO_LIB                           Info,
  IN   EFI_GUID                                            *GuidName,
  IN   UINT8                                               *RawData,
  IN   UINT32                                              RawDataSize,
  OUT  H2O_MEDIA_PLAYER_DECODER_TYPE_LIB                   *DecoderType
  );

/** 
 Pause current present media file by inputing decoder type.
            
 @param[in]  *DecoderType              Decoder type.
  
 @retval EFI Status
**/
EFI_STATUS
MediaPlayerDxeLibPause (
  IN  H2O_MEDIA_PLAYER_DECODER_TYPE_LIB   DecoderType
  );

/** 
 Stop current present media file by inputing decoder type.
            
 @param[in]  *DecoderType              Decoder type.
  
 @retval EFI Status
**/
EFI_STATUS
MediaPlayerDxeLibStop (
  IN  H2O_MEDIA_PLAYER_DECODER_TYPE_LIB   DecoderType
  );

/** 
 Continue current present media file by inputing decoder type.
            
 @param[in]  *DecoderType              Decoder type.
  
 @retval EFI Status
**/
EFI_STATUS
MediaPlayerDxeLibContinue (
  IN  H2O_MEDIA_PLAYER_DECODER_TYPE_LIB   DecoderType
  );

/** 
  ZoomIn
            
  @param[in]  *DecoderType              Decoder type.
  
  @retval None
**/
EFI_STATUS
MediaPlayerDxeLibZoomIn (
  IN  H2O_MEDIA_PLAYER_DECODER_TYPE_LIB   DecoderType
  );

/** 
  ZoomOut

  @param[in]  *DecoderType              Decoder type.
  
  @retval None                  
**/
EFI_STATUS
MediaPlayerDxeLibZoomOut (
  IN  H2O_MEDIA_PLAYER_DECODER_TYPE_LIB   DecoderType
  );

#endif 


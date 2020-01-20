/** @file
  Definition for Stage 2 Hash Lib.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _STAGE_2_HASH_LIB_H_
#define _STAGE_2_HASH_LIB_H_

/**
 This function will return if manifest is available.
 @retval            TRUE                Manifest is available.
 @retval            FALSE               Manifest is not available.
*/
BOOLEAN ManifestAvailable(
);

/**
 This function will return stage 2 hash data size.
 @param[out]        *HashSize           Return the data size of stage 2 hash.
 @retval            EFI_SUCCESS         Operation successfully completed.
*/
EFI_STATUS
GetStage2HashSize (
  OUT UINTN                                   *HashSize
  );


/**
 This function will return stage 2 physical address.
 @param[in]         RecoveryFv          Pointer to recovery FV in current memory
 @param[out]        *Offset             Return hash offset.
 @retval            EFI_SUCCESS         Operation successfully completed.
*/
EFI_STATUS
GetStage2HashPhysicalAddress (
  IN  VOID *                                   RecoveryFv,
  OUT PHYSICAL_ADDRESS                         *Address
  );

/**
 This function will return stage 2 hash data offset in a fd file.
 @param[in]         RecoveryFv          Pointer to recovery FV in current memory
 @param[out]        *Offset             Return hash offset.
 @retval            EFI_SUCCESS         Operation successfully completed.
*/
EFI_STATUS
GetStage2HashFdOffset (
  IN  VOID *                                  RecoveryFv,
  OUT UINTN                                   *Offset
  );

/**
 This function will return physical address of stage 2 hash data.
 @param[out]        *Offset             Return hash offset.
 @retval            EFI_SUCCESS         Operation successfully completed.
*/
EFI_STATUS
GetStage2PhysicalAddress (
  OUT PHYSICAL_ADDRESS                        *Address
  );

/**
 This function will return stage 2 ffset in a fd file.
 @param[out]        *Offset             Return hash offset.
 @retval            EFI_SUCCESS         Operation successfully completed.
*/
EFI_STATUS
GetStage2FdOffset (
  OUT UINTN                                   *Offset
  );
  
/**
 This function will return stage 2 size in bytes.
 @param[out]        *Offset             Return size.
 @retval            EFI_SUCCESS         Operation successfully completed.
*/
EFI_STATUS
GetStage2Size (
  OUT UINTN                                   *Size
  );
  
/**
 This function will calculate hash data provides an interface to modify OEM Logo and POST String.
 @param[in]         Stage2              Pointer to stage 2 data.
 @param[in]         Stage2Size          Size of stage 2.
 @param[out]        HashData            Caller will allocate the buffer enough to fill hash data.
                                        On exit, this function will return the hash data of stage 2 data.
 @retval            EFI_SUCCESS         Operation successfully completed.
*/
EFI_STATUS
GetStage2HashData (
  IN     VOID                                  *Stage2,
  IN     UINTN                                 Stage2Size,              
  OUT    VOID                                  *HashData
  );
  

#endif
